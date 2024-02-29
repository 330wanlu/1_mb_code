#include "../inc/get_net_state.h"

static void _get_ip_state_task(void *arg,int o_timer);
static int get_net_state_open(struct ty_file	*file);
static int get_net_state_ctl(void *data,int fd,int cm,va_list args);
static int get_net_state_close(void *data,int fd);
static int get_internet_infomation(struct _get_net_state *stream);
static int analysis_json_internet_info(struct _get_net_state *stream, char *inbuf);
static int clean_all_status(struct _get_net_state_fd *id, va_list args);
static int get_net_status(struct _get_net_state_fd *id, va_list args);
static int set_wifi_used_status(struct _get_net_state_fd *id, va_list args);//����wifi��ǰʹ���ĸ�ssid
static int check_net_internet(struct _get_net_state *stream);

static const struct _get_net_ctl_fu ctl_fun[]=
{
	{   DF_GET_NET_STATE_CM_CLEAN_STATUS,             clean_all_status             },
	{	DF_GET_NET_STATE_CM_GET_NET_STATUS,			  get_net_status               },
	{	DF_SET_NET_WIFI_USED,						  set_wifi_used_status           }
};
        
static const struct _file_fuc	get_net_state_fuc=
{
	.open=get_net_state_open,
	.read=NULL,
	.write=NULL,
	.ctl=get_net_state_ctl,
	.close=get_net_state_close,
	.del=NULL
};

int get_net_state_add(void)
{   struct _get_net_state *stream;
	int result;
	stream=malloc(sizeof(struct _get_net_state));
	if(stream==NULL)
		return DF_ERR_MEM_ERR;
	memset(stream,0,sizeof(struct _get_net_state));
    //get_net_state_out("����\"/dev/get_net_state\"�ļ����ļ�ϵͳ\n");
	result = ty_file_add(DF_DEV_GET_NET_STATE, stream, "get_net_state", (struct _file_fuc *)&get_net_state_fuc);
	if(result<0)
		free(stream);
	return result;
}


static int get_net_state_open(struct ty_file	*file)
{
	struct _get_net_state *stream;
	int j;
	stream=file->pro_data;
	if((stream==NULL))	
		return DF_ERR_PORT_CORE_FD_NULL;
   // get_net_state_out("��get_net_stateģ��\n");
    if(stream->state==0)
    {   //get_net_state_out("�״�ʹ��get_net_state_open\n");
        stream->switch_dev=ty_open("/dev/switch",0,0,NULL);
		//get_net_state_out("stream->switch_dev = %02x\n",stream->switch_dev);
		stream->deploy=ty_open("/dev/deploy",0,0,NULL);
        if(stream->switch_dev<0)
        {   get_net_state_out("��switch�豸ʧ��\n");
            return stream->switch_dev;
        }
		stream->machine_fd = ty_open("/dev/machine", 0, 0, NULL);	
		//ty_ctl(stream->machine_fd, DF_MACHINE_INFOR_CM_GET_MODE_SUPPORT, &stream->mode_support);
#ifdef MTK_OPENWRT
		get_net_mac("eth0.2", stream->eth_mac);
#else
		get_net_mac("eth0",stream->eth_mac);
		get_net_mac("wlan0", stream->wlan_mac);
#endif
		logout(INFO, "SYSTEM", "INIT", "����MAC��ַ��%s\r\n", stream->eth_mac);
		//get_net_state_out("��ȡ����eth0��mac��ַΪ%s\n", stream->eth_mac);
		stream->lcd_fd = ty_open("/dev/lcd_state", 0, 0, NULL);
#ifdef MTK_OPENWRT
		stream->ty_usb_fd = ty_open("/dev/ty_usb",0,0,NULL);
		if (stream->ty_usb_fd<0)
		{
			get_net_state_out("��USB[/dev/ty_usb]ģ��ʧ��\n");
			ty_close(stream->switch_dev);
			return stream->ty_usb_fd;
		}
#endif
		stream->task=_scheduled_task_open("_get_net_state",_get_ip_state_task,stream,DF_GET_IP_STATE_TASK_TIMER,NULL);
		if(stream->task<0)
		{
			get_net_state_out("������ѯ����״̬�߳�ʧ��\n");
			return stream->task;
		}
		// get_net_state_out("get_net_stateģ���ʼ���ɹ�\n");
    }
    for(j=0;j<sizeof(stream->fd)/sizeof(stream->fd[0]);j++)
	{	if(stream->fd[j].state==0)
			break;
	}
	if(j==sizeof(stream->fd)/sizeof(stream->fd[0]))
	{	return DF_ERR_PORT_CORE_SLAVE_SPACE;
	}
	memset(&stream->fd[j],0,sizeof(stream->fd[0]));
	stream->fd[j].dev=stream; 
	stream->fd[j].state=1; 
	stream->fd[j].mem_fd=&file->fd[j];
	stream->state++;	
    //get_net_state_out("��get_net_state�ɹ�:j=%d\n",j);
	return j+1;	 
}

static int _check_sdcard_exist(struct _get_net_state *stream)
{
	struct _lcd_print_info *lcd_print_info = NULL;
	lcd_print_info = get_lcd_printf_info();
	if (access("/dev/mmcblk1p1",0) < 0)
	{
		if (stream->sdcard_state != 10)
		{
			lcd_print_info->sd_state = 0;
			ty_ctl(stream->lcd_fd, DF_LCD_STATE_UPDATE_STATE);
			stream->sdcard_state = 10;
		}
		return 0;
	}
	if ((access("/sdcard/mysql", 0) < 0) && (access("/sdcard/sdcard.txt", 0) < 0))
	{
		if (stream->sdcard_state != 20)
		{
			lcd_print_info->sd_state = 1;
			ty_ctl(stream->lcd_fd, DF_LCD_STATE_UPDATE_STATE);
			stream->sdcard_state = 20;
		}
		return 0;
	}
	else
	{
		if (stream->sdcard_state != 30)
		{
			lcd_print_info->sd_state = 2;
			ty_ctl(stream->lcd_fd, DF_LCD_STATE_UPDATE_STATE);
			stream->sdcard_state = 30;
			return 0;
		}
		return 0;
	}

}

static int check_net_internet(struct _get_net_state *stream)
{
	char server_ip[20] = { 0 };
	uint32 ip, mask, getway, dns;
	int type;
	struct _net_status net_status;
	memset(&net_status, 0, sizeof(net_status));

	if (stream->net_mode == 0)
	{
		memcpy(net_status.mac, stream->eth_mac, strlen(stream->eth_mac));
		ty_ctl(stream->deploy, DF_DEPLOY_CM_GET_NET_PAR, &ip, &mask, &getway, &dns, &type);
		ip2ip_asc(ip, net_status.ip_address);
		ip2ip_asc(mask, net_status.netmask);
		ip2ip_asc(getway, net_status.gateway);
		ip2ip_asc(dns, net_status.dns);
		sprintf(net_status.ip_type, "%d", type);
		sprintf(net_status.first_net, "1");
		sprintf(net_status.eth_wlan, "eth0");
	}
	else
	{
		memcpy(net_status.mac, stream->wlan_mac, strlen(stream->wlan_mac));
		ty_ctl(stream->deploy, DF_DEPLOY_CM_GET_WIFI_PAR, &ip, &mask, &getway, &dns, &type);
		ip2ip_asc(ip, net_status.ip_address);
		ip2ip_asc(mask, net_status.netmask);
		ip2ip_asc(getway, net_status.gateway);
		ip2ip_asc(dns, net_status.dns);
		sprintf(net_status.ip_type, "%d", type);
		sprintf(net_status.first_net, "1");
		sprintf(net_status.eth_wlan, "wlan0");
		strcpy(net_status.wifi_ssid,stream->connect_ssid);
	}

	if (get_DNS_ip("www.baidu.com", server_ip) < 0)
	{
		//get_net_state_out("����www.baidu.com����ʧ��\n");
		sprintf(net_status.internet, "0");
		goto END;
	}

	//if (connect_ip_test(server_ip, 21, 3) < 0)
	//{
	//	get_net_state_out("����www.njmbxx.com ftp�˿�ʧ��\n");
	//	sprintf(net_status.internet, "0");
	//	goto END;
	//}
	sprintf(net_status.internet, "1");
END:

	if (strcmp(net_status.ip_address, stream->net_status.ip_address) != 0)
	{
		memset(stream->net_status.ip_address, 0, sizeof(stream->net_status.ip_address));
		memcpy(stream->net_status.ip_address, net_status.ip_address, strlen(net_status.ip_address));
		stream->net_state = 0;
	}
	if (strcmp(net_status.netmask, stream->net_status.netmask) != 0)
	{
		memset(stream->net_status.netmask, 0, sizeof(stream->net_status.netmask));
		memcpy(stream->net_status.netmask, net_status.netmask, strlen(net_status.netmask));
		stream->net_state = 0;
	}
	if (strcmp(net_status.gateway, stream->net_status.gateway) != 0)
	{
		memset(stream->net_status.gateway, 0, sizeof(stream->net_status.gateway));
		memcpy(stream->net_status.gateway, net_status.gateway, strlen(net_status.gateway));
		stream->net_state = 0;
	}
	if (strcmp(net_status.dns, stream->net_status.dns) != 0)
	{
		memset(stream->net_status.dns, 0, sizeof(stream->net_status.dns));
		memcpy(stream->net_status.dns, net_status.dns, strlen(net_status.dns));
		stream->net_state = 0;
	}
	if (strcmp(net_status.ip_type, stream->net_status.ip_type) != 0)
	{
		memset(stream->net_status.ip_type, 0, sizeof(stream->net_status.ip_type));
		memcpy(stream->net_status.ip_type, net_status.ip_type, strlen(net_status.ip_type));
		stream->net_state = 0;
	}
	if (strcmp(net_status.first_net, stream->net_status.first_net) != 0)
	{
		memset(stream->net_status.first_net, 0, sizeof(stream->net_status.first_net));
		memcpy(stream->net_status.first_net, net_status.first_net, strlen(net_status.first_net));
		stream->net_state = 0;
	}
	if (strcmp(net_status.internet, stream->net_status.internet) != 0)
	{
		memset(stream->net_status.internet, 0, sizeof(stream->net_status.internet));
		memcpy(stream->net_status.internet, net_status.internet, strlen(net_status.internet));
		stream->net_state = 0;
	}
	if (strcmp(net_status.mac, stream->net_status.mac) != 0)
	{
		memset(stream->net_status.mac, 0, sizeof(stream->net_status.mac));
		memcpy(stream->net_status.mac, net_status.mac, strlen(net_status.mac));
		stream->net_state = 0;
	}
	if (strcmp(net_status.eth_wlan, stream->net_status.eth_wlan) != 0)
	{
		memset(stream->net_status.eth_wlan, 0, sizeof(stream->net_status.eth_wlan));
		memcpy(stream->net_status.eth_wlan, net_status.eth_wlan, strlen(net_status.eth_wlan));
		stream->net_state = 0;
	}
	if (strcmp(net_status.wifi_ssid, stream->net_status.wifi_ssid) != 0)
	{
		memset(stream->net_status.wifi_ssid, 0, sizeof(stream->net_status.wifi_ssid));
		memcpy(stream->net_status.wifi_ssid, net_status.wifi_ssid, strlen(net_status.wifi_ssid));
		stream->net_state = 0;
	}
	return 0;
}


static void _get_ip_state_task(void *arg,int o_timer)
{	
	int result;
	int eth0_ok = -1;
	int wlan0_ok = -1;
	struct _get_net_state *stream;
	struct _lcd_print_info *lcd_print_info = NULL;
	int i = 0;
	USB_PORT_CLASS *usb_port_class = get_usb_port_class();  //��ȡusb_port�Ĳ���ָ��
	lcd_print_info = get_lcd_printf_info();
	stream = arg;
	get_net_state_out("\n\n����IP����߳�,stream->net_state= %d\n", stream->net_state);

	//get_net_state_out("���SD��\n");
	
	//_check_sdcard_exist(stream);
	eth0_ok = get_socket_net_state("eth0");

	if(usb_port_class->machine_type == TYPE_2202 || usb_port_class->machine_type == TYPE_2212)
	{
		wlan0_ok = get_socket_net_state("wlan0");
	}

	// if (atoi(stream->mode_support.wifi_support) == 1)
	// {
	// 	wlan0_ok = get_socket_net_state("wlan0");
	// }
	//logout(INFO, "SYSTEM", "������", "����״̬%d, wifi״̬%d\r\n", eth0_ok,wlan0_ok);
	if ((eth0_ok < 0 && wlan0_ok <0) || (eth0_ok < 0 && wlan0_ok == 0) || (eth0_ok < 0 && wlan0_ok ==1))
	{
		if (wlan0_ok < 0)
		{
			if (stream->wifi_state != DF_SWITCH_CM_PORT_RED_OFF_GREN_OFF)
			{	//��֧��wifi,��ȫ��
				get_net_state_out("1��֧��wifi,��ȫ��\n");
				ty_ctl(stream->switch_dev, DF_SWITCH_CM_PORT_RED_OFF_GREN_OFF, DF_WIFI_LED_SAME_USBPORT);
				stream->wifi_state = DF_SWITCH_CM_PORT_RED_OFF_GREN_OFF;
			}
		}

		logout(INFO, "SYSTEM", "������", "��⵽����δ����,������������\r\n");
		ty_ctl(stream->switch_dev, DF_SWITCH_CM_MAC_RESTART_POWER);
		stream->restart_mac_count += 1;
		get_net_state_out("��ȡIP״̬ʧ��\n");
		sleep(5 * stream->restart_mac_count);
		if (stream->restart_mac_count>3)
		{
			ty_ctl(stream->switch_dev, DF_SWITCH_CM_RESET_MA);//����
			sleep(10);
		}
		return;
	}
	else if (eth0_ok == 0 && wlan0_ok < 0)
	{
		if (stream->wifi_state != DF_SWITCH_CM_PORT_RED_OFF_GREN_OFF)
		{	//��֧��wifi,��ȫ��
			get_net_state_out("2��֧��wifi,��ȫ��\n");
			ty_ctl(stream->switch_dev, DF_SWITCH_CM_PORT_RED_OFF_GREN_OFF, DF_WIFI_LED_SAME_USBPORT);
			stream->wifi_state = DF_SWITCH_CM_PORT_RED_OFF_GREN_OFF;
		}
		ty_ctl(stream->deploy, DF_DEPLOY_CM_SET_IP_UPDATE);
		if (stream->net_state != 10)//��������
		{
			get_net_state_out("��ȡ��������״̬Ϊ����0\n");
			ty_ctl(stream->switch_dev, DF_SWITCH_CM_NO_NET);
			lcd_print_info->mqtt_state = 0;
			lcd_print_info->udp_state = 0;
			lcd_print_info->eth_state = 0;
			memset(&lcd_print_info->local_ip, 0, sizeof(lcd_print_info->local_ip));
			memcpy(&lcd_print_info->local_ip, stream->net_status.ip_address, strlen(stream->net_status.ip_address));
			ty_ctl(stream->lcd_fd, DF_LCD_STATE_UPDATE_STATE);
			stream->net_state = 10;//����״̬
			logout(INFO, "SYSTEM", "������", "��⵽���������쳣\r\n");
		}
	}
	else if (eth0_ok == 0 && wlan0_ok == 0)
	{
		if (stream->eth_off_count <= 5)//����ʱ�����5������
		{
			if (stream->wifi_state != DF_SWITCH_CM_PORT_RED_ON_GREN_OFF)
			{	//֧��wifi,δ����
				get_net_state_out("1֧��wifi,δ���������\n");
				ty_ctl(stream->switch_dev, DF_SWITCH_CM_PORT_RED_ON_GREN_OFF, DF_WIFI_LED_SAME_USBPORT);
				ty_ctl(stream->switch_dev, DF_SWITCH_CM_PORT_RED_ON_GREN_OFF, DF_WIFI_LED_SAME_USBPORT);
				stream->wifi_state = DF_SWITCH_CM_PORT_RED_ON_GREN_OFF;
			}
		}
		if (stream->eth_off_count > 5)//����ʱ�����5������
		{
			if (stream->wifi_state != DF_SWITCH_CM_PORT_RED_OFF_GREN_TWI)
			{	//֧��wifi,��������
				//get_net_state_out("1֧��wifi,���������̵���\n");
				ty_ctl(stream->switch_dev, DF_SWITCH_CM_PORT_RED_OFF_GREN_TWI, DF_WIFI_LED_SAME_USBPORT);
				stream->wifi_state = DF_SWITCH_CM_PORT_RED_OFF_GREN_TWI;
			}
		

			char r_buf[4096] = { 0 };
			mb_system_read("ls -l /etc/wifi_config/wifi_*.sh", r_buf, sizeof(r_buf), "/tmp/ls_l_etc_wifi_config.txt");

			if (strstr(r_buf, "No such file or directory") != NULL)
			{
				get_net_state_out("get_configured_wifi_list wifi���������б�ʧ��\n");
			}
			else
			{			
				get_net_state_out("�������������쳣�л���wifiģʽ\n");
				_mqtt_client_disconnect();
				//�л�����������
				system("ifconfig wlan0 up");


				if (strlen(stream->now_ssid) == 0)
				{
					struct _net_define	net;
					set_net("wlan0", &net);//������������
					stream->wifi_off_count = 0;
				}
				else
				{
					struct _net_define	net;
					result = set_net(stream->now_ssid, &net);//������������
					if (result < 0)
					{
						stream->wifi_off_count += 1;
					}
					else
					{
						stream->wifi_off_count = 0;
					}
					if (stream->wifi_off_count > 5)
					{
						get_net_state_out("��ǰwifi������,ɾ������\n");
						
						char wifi_config_path[100] = {0};
						sprintf(wifi_config_path, "/etc/wifi_config/wifi_%s.sh", stream->now_ssid);
						delete_file(wifi_config_path);
						memset(stream->now_ssid, 0, sizeof(stream->now_ssid));
					}
				}
			}				
			
		}
		else
		{
			ty_ctl(stream->deploy, DF_DEPLOY_CM_SET_IP_UPDATE);
			if (stream->net_state != 10)//��������
			{
				get_net_state_out("��ȡ��������״̬Ϊ����0\n");
				ty_ctl(stream->switch_dev, DF_SWITCH_CM_NO_NET);
				lcd_print_info->mqtt_state = 0;
				lcd_print_info->udp_state = 0;
				lcd_print_info->eth_state = 0;
				memset(&lcd_print_info->local_ip, 0, sizeof(lcd_print_info->local_ip));
				memcpy(&lcd_print_info->local_ip, stream->net_status.ip_address, strlen(stream->net_status.ip_address));
				ty_ctl(stream->lcd_fd, DF_LCD_STATE_UPDATE_STATE);
				stream->net_state = 10;//����״̬
				logout(INFO, "SYSTEM", "������", "��⵽���������쳣\r\n");
			}
			stream->eth_off_count += 1;
		}
	}
	else if (eth0_ok == 0 && wlan0_ok ==1)
	{
		char connect_ssid[50] = {0};
		memset(connect_ssid, 0, sizeof(connect_ssid));
		find_now_wifi_ssid(connect_ssid);
		if (strcmp(connect_ssid, stream->connect_ssid) != 0)
		{
			memset(stream->connect_ssid, 0, sizeof(stream->connect_ssid));
			strcpy(stream->connect_ssid, connect_ssid);
		}

		if ((strlen(stream->now_ssid) != 0) && (strlen(stream->connect_ssid) != 0))
		{
			if (strcmp(stream->now_ssid, stream->connect_ssid) != 0)
			{
				get_net_state_out("��ǰ���ӵ�wifi��Ҫ���õ�wifi��һ��,��Ҫ����\n");
				_mqtt_client_disconnect();
				system("killall wpa_supplicant");

				system("ifconfig wlan0 down");
				sleep(1);
				system("ifconfig wlan0 up");
				delete_file(DF_LINUX_DNS);

				return;//�˴�ֱ�ӷ��أ�Ŀ���ܿ����л�����;

			}

		}


		if (stream->wifi_state != DF_SWITCH_CM_PORT_RED_OFF_GREN_ON)
		{	//֧��wifi,������
			get_net_state_out("1֧��wifi,���ӳɹ�,���̵�\n");
			ty_ctl(stream->switch_dev, DF_SWITCH_CM_PORT_RED_OFF_GREN_ON, DF_WIFI_LED_SAME_USBPORT);
			stream->wifi_state = DF_SWITCH_CM_PORT_RED_OFF_GREN_ON;
		}
		stream->net_mode = 1;
		check_net_internet(stream);
		if (atoi(stream->net_status.internet) == 0)
		{
			if (stream->net_state != 20)
			{
				//get_net_state_out("��ȡ��������״̬Ϊ����1\n");
				ty_ctl(stream->switch_dev, DF_SWITCH_CM_GET_NET_OK);
				lcd_print_info->eth_state = 1;
				lcd_print_info->mqtt_state = 0;
				lcd_print_info->udp_state = 0;
				memset(&lcd_print_info->local_ip, 0, sizeof(lcd_print_info->local_ip));
				memcpy(&lcd_print_info->local_ip, stream->net_status.ip_address, strlen(stream->net_status.ip_address));
				ty_ctl(stream->lcd_fd, DF_LCD_STATE_UPDATE_STATE);
				stream->net_state = 20;//����״̬
				logout(INFO, "SYSTEM", "������", "��⵽ʹ�������쳣\r\n");
			}
		}
		else
		{
			if (stream->net_state != 30)
			{
				ty_ctl(stream->switch_dev, DF_SWITCH_CM_GET_NET_OK);
				lcd_print_info->mqtt_state = 1;
				lcd_print_info->udp_state = 1;
				lcd_print_info->eth_state = 2;
				memset(&lcd_print_info->local_ip, 0, sizeof(lcd_print_info->local_ip));
				memcpy(&lcd_print_info->local_ip, stream->net_status.ip_address, strlen(stream->net_status.ip_address));
				ty_ctl(stream->lcd_fd, DF_LCD_STATE_UPDATE_STATE);
				stream->net_state = 30;//����״̬
				logout(INFO, "SYSTEM", "������", "��⵽ʹ����������\r\n");
			}
		}
	}
	else if ((eth0_ok == 1 && wlan0_ok < 0) || (eth0_ok == 1 && wlan0_ok == 0))
	{
		if (wlan0_ok < 0)
		{
			if (stream->wifi_state != DF_SWITCH_CM_PORT_RED_OFF_GREN_OFF)
			{	//��֧��wifi,��ȫ��
				get_net_state_out("3��֧��wifi,��ȫ��\n");
				ty_ctl(stream->switch_dev, DF_SWITCH_CM_PORT_RED_OFF_GREN_OFF, DF_WIFI_LED_SAME_USBPORT);
				stream->wifi_state = DF_SWITCH_CM_PORT_RED_OFF_GREN_OFF;
			}
		}
		else
		{
			if (stream->wifi_state != DF_SWITCH_CM_PORT_RED_ON_GREN_OFF)
			{	//֧��wifi,δ����
				get_net_state_out("2֧��wifi,δ���������\n");
				ty_ctl(stream->switch_dev, DF_SWITCH_CM_PORT_RED_ON_GREN_OFF, DF_WIFI_LED_SAME_USBPORT);
				stream->wifi_state = DF_SWITCH_CM_PORT_RED_ON_GREN_OFF;
			}
		}

		stream->eth_off_count = 0;
		stream->net_mode = 0;		
		check_net_internet(stream);
		if (atoi(stream->net_status.internet) == 0)
		{
			if (stream->net_state != 20)
			{
				//get_net_state_out("��ȡ��������״̬Ϊ����1\n");
				ty_ctl(stream->switch_dev, DF_SWITCH_CM_GET_NET_OK);
				lcd_print_info->eth_state = 1;
				lcd_print_info->mqtt_state = 0;
				lcd_print_info->udp_state = 0;
				memset(&lcd_print_info->local_ip, 0, sizeof(lcd_print_info->local_ip));
				memcpy(&lcd_print_info->local_ip, stream->net_status.ip_address, strlen(stream->net_status.ip_address));
				ty_ctl(stream->lcd_fd, DF_LCD_STATE_UPDATE_STATE);
				stream->net_state = 20;//����״̬
				logout(INFO, "SYSTEM", "������", "��⵽ʹ�������쳣\r\n");
			}
		}
		else
		{
			if (stream->net_state != 30)
			{
				ty_ctl(stream->switch_dev, DF_SWITCH_CM_GET_NET_OK);
				lcd_print_info->mqtt_state = 1;
				lcd_print_info->udp_state = 1;
				lcd_print_info->eth_state = 2;
				memset(&lcd_print_info->local_ip, 0, sizeof(lcd_print_info->local_ip));
				memcpy(&lcd_print_info->local_ip, stream->net_status.ip_address, strlen(stream->net_status.ip_address));
				ty_ctl(stream->lcd_fd, DF_LCD_STATE_UPDATE_STATE);
				stream->net_state = 30;//����״̬
				logout(INFO, "SYSTEM", "������", "��⵽ʹ����������\r\n");
				usb_port_class->report_action.report_data.report_ter_flag = 0;
			}
		}
	}
	else if ((eth0_ok == 1) && (wlan0_ok == 1))
	{
		get_net_state_out("�����������������ر������������\n");
		system("killall wpa_supplicant");
		system("killall udhcpc -i wlan0");
		system("killall udhcpc -i eth0");
		system("ifconfig wlan0 down");
		delete_file(DF_LINUX_DNS);
		ty_ctl(stream->deploy, DF_DEPLOY_CM_SET_IP_UPDATE);
	}
	else
	{
		get_net_state_out("��ȡ����״̬����\n");
	}	

	if ((stream->net_state == 30) && (stream->internet_info_flag == 0) && (stream->internet_info_err_num < 10))
	{
		get_net_state_out("δ��ȡ������Ϣ,�˴�ȥ��ȡ������Ϣ\n");

		result = get_internet_infomation(stream);
		if (result < 0)
		{
			get_net_state_out("��ȡ������Ϣʧ��\n");
			stream->internet_info_err_num += 1;
		}
		else
		{
			stream->internet_info_flag = 1;
		}		
	}

	char time_now[20] = { 0 };
	get_net_state_out("���ʱ���Ƿ�Ϊ��ʼʱ��\n");
	timer_read_y_m_d_h_m_s(time_now);
	get_net_state_out("��ȡ����ϵͳʱ��Ϊ%s\n", time_now);

	ty_ctl(stream->switch_dev, DF_SWITCH_CM_HEART);//��ʱ�ɹ���ͬ����ʱ�䣬lcd��ʾ

	if (strcmp(time_now, SYSTEM_LEAST_TIME) > 0)
	{
		get_net_state_out("��ʱ�ɹ�\n");
		// ty_ctl(stream->switch_dev, DF_SWITCH_CM_HEART);//��ʱ�ɹ���ͬ����ʱ�䣬lcd��ʾ
		stream->net_status.nettime_ok = 1;
	}
	else
	{
		get_net_state_out("��ʱδ�ɹ�\n");
		stream->net_status.nettime_ok = 0;
	}
	
	sleep(2);
	return ;
}

static int get_internet_infomation(struct _get_net_state *stream)
{
	char tmp[100] = {0};
	int result;
	char *returndata = NULL;
	int returndata_len;
	char inter_info[5000] = {0};
	//get_net_state_out("���������������Ϣ\n");
	// result = socket_http_netPost("www.njmbxx.com", 20020, "/was.tersvr/device_location_for_ip", "application/x-www-form-urlencoded", tmp, 5, &returndata, &returndata_len);
	result = socket_http_netPost("ip.njmbxx.com", 20020, "/ping", "application/x-www-form-urlencoded", tmp, 5, &returndata, &returndata_len);
	printf("get_internet_infomation returndata = %s\n",returndata);
	//get_net_state_out("���������������Ϣ���\n");
	if (result < 0)
	{
		if (returndata != NULL)
		{
			//get_net_state_out("�ͷ�http���������ڴ�\n");
			free(returndata);
			returndata = NULL;
			//get_net_state_out("�ͷ��ڴ�ɹ�,ָ���\n");
		}
		get_net_state_out("��ȡ����IP��Ϣʧ��\n");
		return -1;
	}
	else
	{
		if (strlen(returndata) > sizeof(inter_info))
		{
			get_net_state_out("��ȡ������Ϣ�õ������ݳ��ȴ����쳣\n");
			free(returndata);
			returndata = NULL;
			return -2;
		}
		//get_net_state_out("��������Ϣ��UTF8����ת����GBK����\n");
		utf82gbk((uint8 *)returndata, (uint8 *)inter_info);
		free(returndata);
		returndata = NULL;
		//get_net_state_out("��ȡ����IP��Ϣ�ɹ�%s\n", inter_info);
		result = analysis_json_internet_info(stream, inter_info);
		if (result < 0)
		{
			get_net_state_out("��������JOSN����ʧ��,result = %d\n",result);

			return -3;
		}

		
	}
	return 0;
}

static int analysis_json_internet_info(struct _get_net_state *stream,char *inbuf)
{
	
	//char result[20] = { 0 };
	char result_code[10] = { 0 };
	char internet_ip[20] = { 0 };
	char longitude[20] = { 0 };
	char latitude[20] = { 0 };
	char area_info[200] = { 0 };
	char svrtime[20] = {0};
	cJSON *Jsp_Tmp;
	cJSON *root = cJSON_Parse(inbuf);
	if (!root)
	{
		get_net_state_out("Error before: [%s]\n", cJSON_GetErrorPtr());
		return -1;
	}
	

	//get_net_state_out("��ȡ��ѯ�����Ϣ\n");
	Jsp_Tmp = cJSON_GetObjectItem(root, "result");
	if (get_json_value_can_not_null(Jsp_Tmp, result_code, 0, 3) < 0) {
		get_net_state_out("��ȡ��ѯ�����Ϣ failed\n");
		cJSON_Delete(root);
		return -1;
	}
	if (strcmp(result_code, "s") != 0)
	{
		get_net_state_out("��ѯ������Ϣresult != s\n");
		cJSON_Delete(root);
		return -2;
	}

	//get_net_state_out("��ȡ������ʱ��\n");
	Jsp_Tmp = cJSON_GetObjectItem(root, "svrtime");
	if (get_json_value_can_not_null(Jsp_Tmp, svrtime, 0, 20) < 0) {
		get_net_state_out("��ȡ������ʱ�� failed\n");
		cJSON_Delete(root);
		return -1;
	}
	get_net_state_out("������ʱ��Ϊ%s\n",svrtime);


	long time_sec = get_second_from_date_time_turn((char *)svrtime);
	time_sec = time_sec - 8 * 3600;
	char settime[20] = { 0 };
	get_date_time_from_second_turn(time_sec, settime);

	logout(INFO, "system", "������", "��ȡ�����ʱ�ɹ���%s\r\n", settime);

	char cmd[200] = { 0 };
	sprintf(cmd, "date -s \"%s\" -u", settime);
	get_net_state_out("ʱ���������%s\n",cmd);
	system(cmd);
	//system("hwclock -w");

		

	cJSON *object = cJSON_GetObjectItem(root, "response");
	if (object == NULL)
	{
		get_net_state_out("Error before: [%s]\n", cJSON_GetErrorPtr());
		cJSON_Delete(root);
		return -3;
	}
	
	///////////////////////////////////////////////////////////////////
	Jsp_Tmp = cJSON_GetObjectItem(object, "internet_ip");
	if (get_json_value_can_not_null(Jsp_Tmp, internet_ip, 0, 20) < 0) {
		get_net_state_out("��ȡ��ѯ�����Ϣ failed\n");
		cJSON_Delete(root);
		return -4;
	}
	if (strlen(internet_ip) < 7)
	{
		get_net_state_out("��ȡ���Ĺ���IP���ȴ���,ip %s\n", internet_ip);
		cJSON_Delete(root);
		return -5;
	}

	write_file("/etc/ip_tmp.txt", internet_ip, strlen(internet_ip));

	///////////////////////////////////////////////////////////////////
	// Jsp_Tmp = cJSON_GetObjectItem(object, "longitude");
	// if (get_json_value_can_not_null(Jsp_Tmp, longitude, 0, 20) < 0) {
	// 	get_net_state_out("��ȡ������Ϣ failed\n");
	// 	cJSON_Delete(root);
	// 	return -6;
	// }
	// if (strlen(longitude) < 7)
	// {
	// 	get_net_state_out("��ȡ������Ϣ���ȴ���,ip %s\n", longitude);
	// 	cJSON_Delete(root);
	// 	return -7;
	// }
	// ///////////////////////////////////////////////////////////////////
	// Jsp_Tmp = cJSON_GetObjectItem(object, "latitude");
	// if (get_json_value_can_not_null(Jsp_Tmp, latitude, 0, 20) < 0) {
	// 	get_net_state_out("��ȡγ����Ϣ failed\n");
	// 	cJSON_Delete(root);
	// 	return -8;
	// }
	// if (strlen(latitude) < 7)
	// {
	// 	get_net_state_out("��ȡγ����Ϣ���ȴ���,ip %s\n", latitude);
	// 	cJSON_Delete(root);
	// 	return -9;
	// }

	///////////////////////////////////////////////////////////////////
	if (strcmp(internet_ip, stream->net_status.internet_ip) != 0)
	{
		memset(stream->net_status.internet_ip, 0, sizeof(stream->net_status.internet_ip));
		memcpy(stream->net_status.internet_ip, internet_ip, strlen(internet_ip));
	}

	Jsp_Tmp = cJSON_GetObjectItem(object, "area_info");
	if (get_json_value_can_not_null(Jsp_Tmp, area_info, 0, 200) < 0) {
		get_net_state_out("��ȡ������Ϣ failed\n");
		cJSON_Delete(root);
		return -10;
	}
	if (strlen(area_info) < 7)
	{
		get_net_state_out("��ȡ������Ϣ���ȴ���,ip %s\n", area_info);
		cJSON_Delete(root);
		return -11;
	}

	// logout(INFO, "SYSTEM", "������", "����IPΪ%s,������ϢΪ%s,γ����ϢΪ%s,������ϢΪ%s\r\n", internet_ip, longitude, latitude, area_info);

	logout(INFO, "SYSTEM", "������", "����IPΪ%s,������ϢΪ%s\r\n", internet_ip, area_info);


	
	// if (strcmp(longitude, stream->net_status.longitude) != 0)
	// {
	// 	memset(stream->net_status.longitude, 0, sizeof(stream->net_status.longitude));
	// 	memcpy(stream->net_status.longitude, longitude, strlen(longitude));
	// }
	// if (strcmp(latitude, stream->net_status.latitude) != 0)
	// {
	// 	memset(stream->net_status.latitude, 0, sizeof(stream->net_status.latitude));
	// 	memcpy(stream->net_status.latitude, latitude, strlen(latitude));
	// }
	if (strcmp(area_info, stream->net_status.area_info) != 0)
	{
		memset(stream->net_status.area_info, 0, sizeof(stream->net_status.area_info));
		memcpy(stream->net_status.area_info, area_info, strlen(area_info));
	}
	
	//stream->net_status.internet_ok = 1;
	cJSON_Delete(root);
	printf("1111111111111111111111111111111111111111111111\n");
	return 0;
}
static int get_net_state_ctl(void *data,int fd,int cm,va_list args)
{
	struct _get_net_state      *stream;
	struct _get_net_state_fd   *id;
	int i, result;
	stream = data;
	if ((stream == NULL))
		return DF_ERR_PORT_CORE_FD_NULL;
	if ((fd == 0) || (fd>sizeof(stream->fd) / sizeof(stream->fd[0])))
		return DF_ERR_PORT_CORE_FD_Z;
	fd--;
	id = &stream->fd[fd];
	if (id->state == 0)
		return DF_ERR_PORT_CORE_SLAVE_DEV;
	for (i = 0; i<sizeof(ctl_fun) / sizeof(ctl_fun[0]); i++)
	{
		if (cm == ctl_fun[i].cm)
		{
			result = ctl_fun[i].ctl(id, args);
			return result;
		}
	}
	return DF_ERR_PORT_CORE_CM;
}
static int get_net_state_close(void *data,int fd)
{
	return 0;
}

static int clean_all_status(struct _get_net_state_fd *id, va_list args)
{
	get_net_state_out("�������״̬\n");
	id->dev->net_state = 0;
	return 0;
}
static int get_net_status(struct _get_net_state_fd *id, va_list args)
{
	struct _get_net_state      *stream;
	struct _net_status *net_status;
	net_status = va_arg(args, struct _net_status  *);
	stream = id->dev;

	memcpy(net_status, &stream->net_status, sizeof(struct _net_status));
	return 0;
}

static int set_wifi_used_status(struct _get_net_state_fd *id, va_list args)
{
	struct _get_net_state      *stream;
	char *now_ssid;
	//int wifi_status;
	stream = id->dev;
	//wifi_status = va_arg(args, uint32);
	now_ssid = va_arg(args, char *);
	//stream->wifi_status = wifi_status;

	
	memset(stream->now_ssid, 0, sizeof(stream->now_ssid));
	strcpy(stream->now_ssid, now_ssid);
	get_net_state_out("����wifi�л�,Ҫ���õ�wifiΪ:%s,��ǰ���ӵ�wifiΪ��%s\n", stream->now_ssid, stream->connect_ssid);
	return 0;
}