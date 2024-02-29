#define _deploy_c
#include "_deploy.h"
int deploy_add(_so_note    *note,const char *switch_name,const char *usb_name,int hard_type)
{   struct _deploy *stream;
	int result;
	if ((hard_type == 1) || (hard_type == 2) || (hard_type == 3))
	{
		out("A33��MTKʹ��Ĭ������");
	}
	else
	{
		#define HARD_TYPE_A20
		memset(soft_cut.busid,0,sizeof(soft_cut.busid));
		sprintf(soft_cut.busid,"2-1");
	}
	
	stream=malloc(sizeof(struct _deploy));
	if(stream==NULL)
		return DF_ERR_MEM_ERR;
	memset(stream,0,sizeof(struct _deploy));
	stream->note = note;
	stream->hardtype = hard_type;
    //out("����:[deploy_add]ģ��\n");
    memcpy(stream->switch_name,switch_name,strlen(switch_name));
    memcpy(stream->ty_usb_name,usb_name,strlen(usb_name));
	//memcpy(stream->net_time_name,net_time,strlen(net_time));
	result = ty_file_add(DF_DEV_DEPLOY, stream, "deploy", (struct _file_fuc *)&deploy_fuc);
	fd_stream = stream;
	if(result<0)
		free(stream);
    //out("����:[deploy_add]�ɹ�\n");
	return result;    
}

#ifdef MTK_OPENWRT
static int check_lan_ip_get(struct _deploy  *stream)
{
	char cmd[100] = { 0 };
	char buf1[1024] = { 0 };
	int count_detect = 0;
	while (1)
	{
		memset(cmd, 0, sizeof(cmd));
		memset(buf1, 0, sizeof(buf1));
		sprintf(cmd, "ifconfig eth0.2");
		mb_system_read(cmd, buf1, sizeof(buf1),"/tmp/ifconfig_eth0_2.txt");
		if (strstr(buf1, "inet addr:") != NULL)
		{
			ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_PORT_RED_ON_GREN_OFF, 1002);
			ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_PORT_RED_OFF_GREN_OFF, 1003);
			ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_PORT_RED_OFF_GREN_OFF, 1004);
			break;
		}
		if (count_detect == 10)
		{
			ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_PORT_RED_TWI_GREN_TWI, 1002);
			ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_PORT_RED_OFF_GREN_OFF, 1003);
			ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_PORT_RED_OFF_GREN_OFF, 1004);
			return -1;
		}
		count_detect++;
		sleep(1);
	}
	return 0;
}

static int check_wifi_ip_get(struct _deploy  *stream)
{
	char cmd[100] = { 0 };
	char buf1[1024] = { 0 };
	int count_detect = 0;
	while (1)
	{
		memset(cmd, 0, sizeof(cmd));
		memset(buf1, 0, sizeof(buf1));
		sprintf(cmd, "ifconfig apcli0");
		mb_system_read(cmd, buf1, sizeof(buf1),"/tmp/ifconfig_apcli0.txt");
		if (strstr(buf1, "inet addr:") != NULL)
		{
			ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_PORT_RED_ON_GREN_OFF, 1003);
			ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_PORT_RED_OFF_GREN_OFF, 1002);
			ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_PORT_RED_OFF_GREN_OFF, 1004);
			break;
		}
		if (count_detect == 10)
		{
			ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_PORT_RED_TWI_GREN_TWI, 1003);
			ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_PORT_RED_OFF_GREN_OFF, 1002);
			ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_PORT_RED_OFF_GREN_OFF, 1004);
			return -1;
		}
		count_detect++;
		sleep(1);
	}
	return 0;
}

static int check_4g_ip_get(struct _deploy  *stream)
{
	char cmd[100] = { 0 };
	char buf1[1024] = { 0 };
	int count_detect = 0;
	while (1)
	{
		memset(cmd, 0, sizeof(cmd));
		memset(buf1, 0, sizeof(buf1));
		sprintf(cmd, "ifconfig ppp0");
		mb_system_read(cmd, buf1, sizeof(buf1),"/tmp/ifconfig_ppp0.txt");
		if (strstr(buf1, "inet addr:") != NULL)
		{
			ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_PORT_RED_ON_GREN_OFF, 1004);
			ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_PORT_RED_OFF_GREN_OFF, 1002);
			ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_PORT_RED_OFF_GREN_OFF, 1003);
			break;
		}
		if (count_detect == 10)
		{
			ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_PORT_RED_TWI_GREN_TWI, 1004);
			ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_PORT_RED_OFF_GREN_OFF, 1002);
			ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_PORT_RED_OFF_GREN_OFF, 1003);
			return -1;
		}
		count_detect++;
		sleep(1);
	}
	return 0;
}
#endif



static int _mac_restart(struct _deploy *stream)
{
	int result;
	int i;

	system("ifconfig wlan0 up");

	out("��������Ƿ��Ѿ�����\n");
	result = get_socket_net_state("eth0");
	if (result < 0)
	{
		//eth0����δ����
		out("������ʧ��,������λMAC\n");
		goto restart;
	}
	else if (result == 0)
	{
		//eth0�������߻�ifconfigδ����
		out("����״̬Ϊ����,������λMAC\n");
		goto restart;
	}
	else if (result == 1)
	{
		out("��������,���踴λMAC\n");
		return 0;
	}

	out("��ʱ5��\n");
	sleep(5);

restart:
	//out("��ʱ5��\n");

	for (i = 0; i < 1000; i++)
	{
		if (access("/sys/bus/usb/devices/3-0:1.0", 0) == 0)
		{
			break;
		}
		usleep(10000);
		continue;
	}
	if (i == 1000)
	{
		logout(INFO, "SYSTEM", "INIT", "��⵽OTG����δ�������ؼ�����������\r\n");
		return -1;
	}
	//sleep(1);
	result = ty_ctl(stream->switch_fd, DF_SWITCH_CM_MAC_RESTART_POWER);
	if (result < 0)
	{
		logout(INFO, "SYSTEM", "INIT", "����MACоƬ��λʧ��\r\n");
		return result;
	}
	for (i = 0; i < 10000; i++)
	{
		result = get_socket_net_state("eth0");
		if (result >= 0)
		{
			out("eth0��������\n");
			break;
		}
		usleep(10000);
		continue;
	}
	if (i == 1000)
	{
		logout(INFO, "SYSTEM", "INIT", "������λ���ļ�ϵͳ��ѯeth0�����豸ʧ��\r\n");
		return -1;
	}

	out("MAC��λ���\n");
	return 0;
}

static int deploy_open(struct ty_file	*file)
{   struct _deploy  *stream;
    int result,j;
#ifdef MTK_OPENWRT
	char cmd[200] = { 0 };
#endif
	//int fd;
	//char name[DF_FILE_NAME_MAX_LEN];
	//uint8 gbuf[1024];
	stream=file->pro_data;
	if((stream==NULL))	
		return DF_ERR_PORT_CORE_FD_NULL;
    //out("��deploy_openģ��\n");
    if(stream->state==0)
    {   //out("��һ�δ������ļ�\n");
	
        stream->switch_fd=ty_open(stream->switch_name,0,0,NULL);
        if(stream->switch_fd<0)
        {   
			out("�򿪿���ʧ��\n");
            return stream->switch_fd;
        }
        stream->ty_usb_fd=ty_open(stream->ty_usb_name,0,0,NULL);
        if(stream->ty_usb_fd<0)
        {   
			out("��USB[%s]ģ��ʧ��\n",stream->ty_usb_name);
            ty_close(stream->switch_fd);
            return stream->ty_usb_fd;
        }
		result = ty_ctl(stream->switch_fd,DF_SWITCH_CM_READ_FLASH,&stream->file,sizeof(stream->file));
		if(result >= 0)
		{
			//logout(INFO, "SYSTEM", "INIT","��ȡ��Ƭ����FLASH�����ɹ���Э��汾:%d,IP����:%d,IP:%08x,MASK:%08x,GATE:%08x,DNS:%08x,UPDATEADDR:%s\r\n",stream->file.edit,stream->file.ip_type,stream->file.ip,stream->file.mask,stream->file.getway,stream->file.DNS,stream->file.net_soft);
		}
		else
		{	
			logout(INFO, "SYSTEM", "INIT", "������һ�λ�ȡ��Ƭ��FLASHʧ��,�����ٴλ�ȡ\r\n");
			sleep(1);
			result = ty_ctl(stream->switch_fd,DF_SWITCH_CM_READ_FLASH,&stream->file,sizeof(stream->file));
			
			if(result >= 0)
			{	
				logout(INFO, "SYSTEM", "INIT", "��ȡ��Ƭ����FLASH�����ɹ���Э��汾:%d,IP����:%d,IP:%08x,MASK:%08x,GATE:%08x,DNS:%08x,UPDATEADDR:%s\r\n", stream->file.edit, stream->file.ip_type, stream->file.ip, stream->file.mask, stream->file.getway, stream->file.DNS, stream->file.net_soft);
			}
			else
			{
				logout(INFO, "SYSTEM", "INIT", "�����ڶ��λ�ȡ��Ƭ��FLASHʧ��,�����ٴλ�ȡ\r\n");
				sleep(2);
				result = ty_ctl(stream->switch_fd,DF_SWITCH_CM_READ_FLASH,&stream->file,sizeof(stream->file));
				if(result >= 0)
				{
					logout(INFO, "SYSTEM", "INIT", "��ȡ��Ƭ����FLASH�����ɹ���Э��汾:%d,IP����:%d,IP:%08x,MASK:%08x,GATE:%08x,DNS:%08x,UPDATEADDR:%s\r\n", stream->file.edit, stream->file.ip_type, stream->file.ip, stream->file.mask, stream->file.getway, stream->file.DNS, stream->file.net_soft);
				}
				else
				{
					logout(INFO, "SYSTEM", "INIT", "���������λ�ȡ��Ƭ��FLASHʧ��,������������\r\n");
					return -1;
				}
			}
		}
		if (stream->file.edit == 0xffffffff)
		{
			logout(INFO, "SYSTEM", "INIT", "��ȡ��Ƭ��flash����Ϊ0xff,Ϊ�ָ��������ú������\r\n");
			sleep(1);
			struct _deploy_file     file;
			memset(&file, 0, sizeof(struct _deploy_file));
			ty_ctl(stream->switch_fd, DF_SWITCH_CM_READ_FLASH, &file, sizeof(file));

			sleep(2);
			memset(&file, 0, sizeof(struct _deploy_file));
			ty_ctl(stream->switch_fd, DF_SWITCH_CM_READ_FLASH, &file, sizeof(file));
		}
		else if (stream->file.edit == 0x00000000)
		{
			logout(INFO, "SYSTEM", "INIT", "��ȡ��Ƭ��flash����Ϊ0x00,����ΪͨѶ�쳣\r\n");
			sleep(1);
			struct _deploy_file     file;
			memset(&file, 0, sizeof(struct _deploy_file));
			ty_ctl(stream->switch_fd, DF_SWITCH_CM_READ_FLASH, &file, sizeof(file));

			sleep(2);
			memset(&file, 0, sizeof(struct _deploy_file));
			ty_ctl(stream->switch_fd, DF_SWITCH_CM_READ_FLASH, &file, sizeof(file));
		}
		else
		{
			logout(INFO, "SYSTEM", "INIT", "��ȡ��Ƭ��flash�����а汾��Ϊ��%02x\r\n", stream->file.edit);
		}


        //out("��ȡ�����ɹ�\n");
		int tmp_close_auto_update = stream->file.close_auto_update;//���һλ������Ϊ�Ƿ�ǿ�Ʒǽڵ��־
        if(stream->file.edit!=DF_DEPLOY_PAR_EDIT_2)
        {   
			out("�����汾��һ��\n");			
            if(stream->file.edit==DF_DEPLOY_PAR_EDIT_ODE)
            {   
				out("1.0�汾�Ĳ���\n");
				stream->file.edit = DF_DEPLOY_PAR_EDIT_2;
				stream->file.getway=0;
                stream->file.mask=0;
            }
			else if(stream->file.edit == DF_DEPLOY_PAR_EDIT)
			{
				out("2.0�汾�Ĳ���\n");
				stream->file.edit = DF_DEPLOY_PAR_EDIT_2;
				stream->file.vpn_enable = 0;
				stream->file.vpn_mode = 0;
				memset(stream->file.vpn_addr,0,sizeof(stream->file.vpn_addr));
				stream->file.vpn_port = 0;
				memset(stream->file.vpn_user,0,sizeof(stream->file.vpn_user));
				memset(stream->file.vpn_passwd,0,sizeof(stream->file.vpn_passwd));
				stream->file.vpn_ftp_port = 21;
				memset(stream->file.vpn_ftp_user,0,sizeof(stream->file.vpn_ftp_user));
				sprintf(stream->file.vpn_ftp_user,"vpnuser");
				memset(stream->file.vpn_ftp_passwd,0,sizeof(stream->file.vpn_ftp_passwd));
				sprintf(stream->file.vpn_ftp_passwd,"vpnuser123");
				stream->file.close_auto_update = 0;
			}
            else
            {   
				memcpy(&stream->file,&soft_cut,sizeof(stream->file));
            }      
        }
		stream->file.close_auto_update = tmp_close_auto_update;//���һλ������Ϊ�Ƿ�ǿ�Ʒǽڵ��־
        out("����ty_usb��BUSID[%s]\n",stream->file.busid);
        ty_ctl(stream->ty_usb_fd,DF_TY_USB_CM_SET_BUSID,stream->file.busid);
        out("���ö˿ڵ�����˳��:%d\n",stream->file.port_n);
        ty_ctl(stream->ty_usb_fd,DF_TY_USB_CM_SET_PORT_N,stream->file.port_n);
        out("�������ö��ʱ��:%d��\n",stream->file.enum_timer);
        ty_ctl(stream->ty_usb_fd,DF_TY_USB_CM_SET_ENUM_TIMER,stream->file.enum_timer*1000);

		int usb_port_numb = ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_PORT_NUMBER);
		int port;
		for (port = 1; port <= usb_port_numb; port++)
		{
			out("[ty_usb_m.c]���˿ںŴ򿪶˿� port = %d\n",port);
			ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_PORT_CLOSE_PORT_POWER, port);
			usleep(10000);

		}

		logout(INFO, "SYSTEM", "INIT","���ж˿ڵ�Դ�رճɹ�\r\n");
		sleep(2);

		if ((stream->hardtype == 1) || (stream->hardtype == 3))
		{
			_mac_restart(stream);
		}
		
        out("����ip��ַ\n");
		struct _net_define  net_par;
		memset(&net_par,0,sizeof(struct _net_define));
        net_par.ip=stream->file.ip;
        net_par.mask=stream->file.mask;		
		net_par.gateway[0]=stream->file.getway;
		net_par.DNS=stream->file.DNS;
		net_par.type=stream->file.ip_type;
#ifdef MTK_OPENWRT
		read_other_net_file(stream);
		net_par.net_mode = stream->file_o.ip_mode;
#endif

		//out("������Ƭ��FLASH��IPΪ��%08x \n",net_par.ip);
		//out("������Ƭ��FLASH��maskΪ��%08x \n",net_par.mask);
		//out("������Ƭ��FLASH��gatewayΪ��%08x \n",net_par.gateway[0]);
		//out("������Ƭ��FLASH��DNSΪ��%08x \n",net_par.DNS);
		//out("������Ƭ��FLASH��IP����Ϊ %d\n",net_par.type);
#ifdef MTK_OPENWRT
		read_other_net_file(stream);
		if(access("/etc/config/network",0) != 0)//û��network�ļ�
		{
			char cmdt[100] = {0};
			sprintf(cmdt,"touch /etc/config/network");
			system(cmdt);
			sync();
			logout(INFO, "SYSTEM", "INIT","donnot have network file! create it!\r\n");
			memset(cmdt,0,sizeof(cmdt));
			sprintf(cmdt,"chmod 644 /etc/config/network");
			system(cmdt);
			sync();
			memset(cmdt,0,sizeof(cmdt));
			sprintf(cmdt,"config_generate");
			system(cmdt);
			sync();
		}
		net_par.net_mode = stream->file_o.ip_mode;
		//out("������Ƭ��FLASH��IPΪ��%08x \n",net_par.ip);
		//out("������Ƭ��FLASH��maskΪ��%08x \n",net_par.mask);
		//out("������Ƭ��FLASH��gatewayΪ��%08x \n",net_par.gateway[0]);
		//out("������Ƭ��FLASH��DNSΪ��%08x \n",net_par.DNS);
		//out("������Ƭ��FLASH��IP����Ϊ %d\n",net_par.type);
		//out("�����ļ�����������Ϊ %d\n", net_par.net_mode);
		if (net_par.type == 0 && net_par.net_mode == 0)
		{
			out("IP��Ϊ��һ�ο���\n");
			//��������Ƿ����
			char buf1[500] = { 0 };

			memset(cmd, 0, sizeof(cmd));
			sprintf(cmd, "swconfig dev rt305x port 0 get link | awk \'{print $2}\' | cut -d"":"" -f2");
			mb_system_read(cmd, buf1, sizeof(buf1),"/tmp/swconfig_dev_rt305x.txt");
			printf("status:%s\n", buf1);
			if (strstr(buf1, "up") != NULL)//���߲���
			{
				logout(INFO, "SYSTEM", "INIT","��Ƭ��IP����δ����,����IPΪ����Ĭ��ֵdhcp\r\n");
				// 			if((net_par.ip==0)&&(net_par.mask==0)&&(net_par.gateway[0]==0)&&(net_par.DNS==0))
				// 			{	
				stream->file.ip = 0xC0A800FA;
				stream->file.mask = 0xFFFFFF00;
				stream->file.getway = 0xC0A80001;
				stream->file.DNS = 0xC0A80001;
				stream->file.ip_type = 1;
				stream->file_o.ip_mode = 0;
				//stream->file.flag_dhcp = 0;
				ty_ctl(stream->switch_fd, DF_SWITCH_CM_WRITE_FLASH, &stream->file, sizeof(stream->file));
				set_other_net_file(0, stream);
				
				_queue_add("set_ip", set_ip_q, stream, 100);
				//sleep(6);
				check_lan_ip_get(stream);
				lcd_print_info.now_net_mode = 0;
				/*while (1)
				{
					if (stream->file.flag_dhcp == 1)
						break;
					sleep(1);
				}*/
				/*memset(cmd, 0, sizeof(cmd));
				sprintf(cmd, "/etc/init.d/network restart");
				system(cmd);
				sleep(4);*/

			}
			else//�л���wifi Ĭ���˻�mb_machine ����:12345678
			{
				
				//����Ƿ���Ĭ��wifi
				char buf1[1024] = { 0 };

				memset(cmd, 0, sizeof(cmd));
				sprintf(cmd, "iwpriv ra0 set SiteSurvey = 1");
				system(cmd);
				memset(cmd, 0, sizeof(cmd));
				sprintf(cmd, "iwpriv ra0 get_site_survey");
				mb_system_read(cmd, buf1, sizeof(buf1),"/tmp/iwpriv_ra0.txt");
				char *p = NULL;
				//printf("wifi info:%s\n", buf1);
				p = strstr(buf1,"mb_machine");//�鿴�Ƿ���mb_machine���wifi��
				if (p == NULL)//�л���4g
				{
					/*stream->file.ip_mode = 2;*/
					ty_ctl(stream->switch_fd, DF_SWITCH_CM_WRITE_FLASH, &stream->file, sizeof(stream->file));
					set_other_net_file(2, stream);
					_queue_add("set_ip_4g", set_ip_4g, stream, 100);
					sleep(10);

					check_4g_ip_get(stream);
					lcd_print_info.now_net_mode = 2;

				}
				else
				{
					/*stream->file.ip_mode = 1;*/
					memcpy(stream->file_o.wifi_name, "mb_machine", 10);
					memcpy(stream->file_o.wifi_pwd, "12345678", 8);
					memcpy(stream->file_o.wifi_mode, "WPA2PSK",7);
					ty_ctl(stream->switch_fd, DF_SWITCH_CM_WRITE_FLASH, &stream->file, sizeof(stream->file));
					set_other_net_file(1, stream);
					_queue_add("set_ip_w", set_ip_w, stream, 100);
					sleep(10);
					check_wifi_ip_get(stream);
					lcd_print_info.now_net_mode = 1;
				}

			}
		}
		//DHCP
		else if (net_par.type == 1 && net_par.net_mode != 1 && net_par.net_mode != 2)
		{
			stream->file.ip_type = 1;
			stream->file_o.ip_mode = 0;
			ty_ctl(stream->switch_fd, DF_SWITCH_CM_WRITE_FLASH, &stream->file, sizeof(stream->file));
			set_other_net_file(0, stream);
			result = detect_process("udhcpc -i", "tmpu.txt");//����Ƿ��Ѿ�����dhcp����������򲻼��������Դﵽ����Ŀ����ٶ�
			if (result == 1)//��ǰֻ����һ��udhcpc -i eth0.2
			{
				if(check_lan_ip_get(stream) < 0)//������ʧ��
				{
					logout(INFO, "SYSTEM", "INIT","dhcp��һ���Ѿ����������������ȡʧ�ܣ�׼����������dhcp!\r\n");
					_queue_add("set_ip", set_ip_q, stream, 100);
					//sleep(7);
					check_lan_ip_get(stream);
				}
				else
				{
					out("dhcp�����������������������¿���!\n");
				}

			}
			else
			{
				logout(INFO, "SYSTEM", "INIT","dhcp��һ��δ������׼����������dhcp!\r\n");
				_queue_add("set_ip", set_ip_q, stream, 100);
				//sleep(7);
				check_lan_ip_get(stream);
			}
			lcd_print_info.now_net_mode = 0;
			//sleep(10);
		}
		//STATIC
		else if (net_par.type == 2 && net_par.net_mode != 1 && net_par.net_mode != 2)
		{
			out("��Ҫ���ù̶�IP\n");

			//��������Ƿ����

			char buf1[1024] = { 0 };

			memset(cmd, 0, sizeof(cmd));
			sprintf(cmd, "swconfig dev rt305x port 0 get link | awk \'{print $2}\' | cut -d\":\" -f2");
			mb_system_read(cmd, buf1, sizeof(buf1),"/tmp/swconfig_dev_rt305x2.txt");
			printf("status:%s\n", buf1);
			if (strstr(buf1, "up") != NULL)//���߲���
			{
				stream->file.lan_in = 1;
				if ((net_par.ip == 0) && (net_par.mask == 0) && (net_par.gateway[0] == 0) && (net_par.DNS == 0))
				{
					out("net_par.type==2 ��Ƭ��IP����δ���ã�����IPΪ����Ĭ��ֵ192.168.0.250\n");
					stream->file.ip = 0xC0A800FA;
					stream->file.mask = 0xFFFFFF00;
					stream->file.getway = 0xC0A80001;
					stream->file.DNS = 0xC0A80001;
					stream->file.ip_type = 2;
					//stream->file.ip_mode = 0;
					set_other_net_file(0,stream);
					ty_ctl(stream->switch_fd, DF_SWITCH_CM_WRITE_FLASH, &stream->file, sizeof(stream->file));
					_queue_add("set_ip", set_ip_q, stream, 100);
					sleep(1);
					check_lan_ip_get(stream);
					lcd_print_info.now_net_mode = 0;
					//sleep(5);
				}
				else
				{
					out("��Ƭ��FLASH�������ݣ�����ʹ��\n");
					net_par.net_mode = 0;
					set_other_net_file(0, stream);
					set_net(DF_NET_DEV, &net_par);
					sleep(1);
					check_lan_ip_get(stream);
					lcd_print_info.now_net_mode = 0;
					//sleep(5);
				}
			}
			else
			{

			}
		}
		else if (net_par.net_mode == 1)
		{
			
			{
				out("wifi����ʹ��\n");
				_queue_add("set_ip_w", set_ip_w, stream, 100);
				sleep(10);
				check_wifi_ip_get(stream);
				lcd_print_info.now_net_mode = 1;
			}
		}
		else if (net_par.net_mode == 2)//4g ��
		{
			//��������Ƿ����


			char buf1[1024] = { 0 };

			memset(cmd, 0, sizeof(cmd));
			sprintf(cmd, "swconfig dev rt305x port 0 get link | awk \'{print $2}\' | cut -d\":\" -f2");
			mb_system_read(cmd, buf1, sizeof(buf1),"/tmp/swconfig_dev_rt305x3.txt");
			printf("status:%s\n", buf1);
			if (strstr(buf1, "up") != NULL)//���߲���
			{
				if ((net_par.ip == 0) && (net_par.mask == 0) && (net_par.gateway[0] == 0) && (net_par.DNS == 0))
				{

					logout(INFO, "SYSTEM", "INIT","��Ƭ��IP����δ����,����IPΪ����Ĭ��ֵdhcp\r\n");
					stream->file.ip = 0xC0A800FA;
					stream->file.mask = 0xFFFFFF00;
					stream->file.getway = 0xC0A80001;
					stream->file.DNS = 0xC0A80001;
					stream->file.ip_type = 1;
					stream->file_o.ip_mode = 0;
					ty_ctl(stream->switch_fd, DF_SWITCH_CM_WRITE_FLASH, &stream->file, sizeof(stream->file));
					set_other_net_file(0, stream);
					_queue_add("set_ip", set_ip_q, stream, 100);
					sleep(6);
					check_lan_ip_get(stream);
					lcd_print_info.now_net_mode = 0;

				}
				else
				{
					set_other_net_file(0,stream);
					out("��Ƭ��FLASH�������ݣ�����ʹ��\n");
					set_net(DF_NET_DEV, &net_par);
					sleep(7);
					check_lan_ip_get(stream);
					lcd_print_info.now_net_mode = 0;
				}
			}
			else{
				if(access("/etc/ppp/peers",0) != 0)
				{
					system("mkdir /etc/ppp/peers");
					sync();
					system("cp /home/share/exe/peers/* /etc/ppp/peers/. -r");
					sync();
				}

				set_other_net_file(2,stream);
				_queue_add("set_ip_4g", set_ip_4g, stream, 100);
				sleep(10);

				check_4g_ip_get(stream);
				lcd_print_info.now_net_mode = 2;
			}

			
		}
		out("IP�������");
#else
		if(net_par.type==0)
		{
			//out("IP��Ϊ��һ�ο���\n");

			logout(INFO, "SYSTEM", "INIT", "��Ƭ��IP����δ����,��IP�ѻָ���������\r\n");

			if (file_exists("/etc/wifi_config") != 0)
			{
				delete_dir("/etc/wifi_config");
			}

// 			if((net_par.ip==0)&&(net_par.mask==0)&&(net_par.gateway[0]==0)&&(net_par.DNS==0))
// 			{	
			//out("net_par.type==0 ��Ƭ��IP����δ���ã�����IPΪ����Ĭ��ֵ192.168.0.250\n");
			stream->file.ip=0xC0A800FA;
			stream->file.mask=0xFFFFFF00;		
			stream->file.getway=0xC0A80001;
			stream->file.DNS=0x72727272;
			stream->file.ip_type=1;
			write_log("�س�����ɾ��mac_address�ļ�\n");
			delete_file(DF_TER_MAC_ADDR_FILE);
			sync();
			ty_ctl(stream->switch_fd,DF_SWITCH_CM_WRITE_FLASH,&stream->file,sizeof(stream->file));
			_queue_add("set_ip",set_ip_q,stream,100); 

		}
		else if (net_par.type == 1)
		{
			//out("��Ҫ���ó�DHCP\n");
			set_net(DF_NET_DEV, &net_par);
		}
		else if(net_par.type==2)
		{
			//out("��Ҫ���ù̶�IP\n");
			if((net_par.ip==0)&&(net_par.mask==0)&&(net_par.gateway[0]==0)&&(net_par.DNS==0))
			{
				//out("net_par.type==2 ��Ƭ��IP����δ���ã�����IPΪ����Ĭ��ֵ192.168.0.250\n");
				stream->file.ip=0xC0A800FA;
				stream->file.mask=0xFFFFFF00;		
				stream->file.getway=0xC0A80001;
				stream->file.DNS=0xC0A80001;
				stream->file.ip_type=2;
				ty_ctl(stream->switch_fd,DF_SWITCH_CM_WRITE_FLASH,&stream->file,sizeof(stream->file));
				_queue_add("set_ip",set_ip_q,stream,100); 				
			}
			else 
			{	//out("��Ƭ��FLASH�������ݣ�����ʹ��\n");
				set_net(DF_NET_DEV,&net_par);
			}
		}
		//out("IP�������\n");
		#endif

        stream->lock=_lock_open("deloy.c",NULL);
		//ftp�ĳɰ�����������Ҫ����������

		//stream->ftp_task=_task_open("_deploy.c",deploy_task,stream,DF_DEPLOY_TASK_TIMER,NULL);
		
    }
    for(j=0;j<sizeof(stream->fd)/sizeof(stream->fd[0]);j++)
	{	if(stream->fd[j].state==0)
			break;
	}
	if(j==sizeof(stream->fd)/sizeof(stream->fd[0]))
	{	
		return DF_ERR_PORT_CORE_SLAVE_SPACE;
	}
	memset(&stream->fd[j],0,sizeof(stream->fd[0]));
	stream->fd[j].dev=stream; 
	stream->fd[j].state=1; 
	stream->fd[j].mem_fd=&file->fd[j];
	stream->state++;	
    out("��deploy_open�ɹ�:j=%d\n",j);
	return j+1;    
}
#ifdef MTK_OPENWRT
static int set_other_net_file(int net_mode, struct _deploy  *stream)
{
	FILE * fd;
	//char *p = NULL;
	//int now_index = 0;
	char write_buf[1024] = { 0 };
	char line_buf[256] = { 0 };
	char tmp[256] = { 0 };
	char net_mod_str[3] = { 0 };
	sprintf(net_mod_str, "%d", net_mode);
	if (access(DF_OTHER_NET_FIlE_PATH, 0) != 0)//��������ļ�
	{
		fd = fopen(DF_OTHER_NET_FIlE_PATH, "w+");
		if (fd == NULL)
		{
			out("�򿪴�������������Ϣ�ļ�ʧ��\n");
			return -1;
		}
		sprintf(write_buf, "net_mode:%s\nwifi_name:mb_machine\nwifi_pwd:12345678\nwifi_mode:WPA2PSK\n", net_mod_str);
		//write(fd, write_buf, strlen(write_buf));
		fwrite(write_buf, strlen(write_buf), 1, fd);
		fclose(fd);
		return 0;
	}
	fd = fopen(DF_OTHER_NET_FIlE_PATH, "r");
	if (fd == NULL)
	{
		out("�򿪴�������������Ϣ�ļ�ʧ��\n");
		return -1;
	}
	if (net_mode == 1)//wifi
	{
		int index = 0;
		while (fgets(line_buf, 256, fd))
		{
			if (index == 0)
			{
			
				char *p = strstr(line_buf, "net_mode:");
				if (p == NULL)
				{
					strcat(write_buf, line_buf);
					continue;
				}
				sprintf(tmp, "net_mode:%s\n", net_mod_str);
				strcat(write_buf, tmp);
				index = 1;
				continue;
			}
			else if (index == 1)
			{
				char *p1 = strstr(line_buf, "wifi_name:");
				if (p1 == NULL)
				{
					strcat(write_buf, line_buf);
					continue;
				}
				memset(tmp, 0, sizeof(tmp));
				sprintf(tmp, "wifi_name:%s\n", stream->file_o.wifi_name);
				strcat(write_buf, tmp);
				index = 2;
				continue;
			}
			else if (index == 2)
			{
				char *p2 = strstr(line_buf, "wifi_pwd:");
				if (p2 == NULL)
				{
					strcat(write_buf, line_buf);
					continue;
				}
				memset(tmp, 0, sizeof(tmp));
				sprintf(tmp, "wifi_pwd:%s\n", stream->file_o.wifi_pwd);
				strcat(write_buf, tmp);
				index = 3;
				continue;
			}
			else if (index == 3)
			{
				char *p3 = strstr(line_buf, "wifi_mode:");
				if (p3 == NULL)
				{
					strcat(write_buf, line_buf);
					continue;
				}
				memset(tmp, 0, sizeof(tmp));
				sprintf(tmp, "wifi_mode:%s\n", stream->file_o.wifi_mode);
				strcat(write_buf, tmp);
				index = 4;
				continue;
			}
		}
	}
	else 
	{
		memset(write_buf, 0, sizeof(write_buf));
		while (fgets(line_buf, 256, fd))
		{
			char *p = strstr(line_buf, "net_mode:");
			if (p == NULL)
			{
				strcat(write_buf, line_buf);
			}
			else
			{
				sprintf(tmp, "net_mode:%s\n", net_mod_str);
				strcat(write_buf, tmp);
			}
			
		}
	}
	fclose(fd);
	remove(DF_OTHER_NET_FIlE_PATH);
	fd = fopen(DF_OTHER_NET_FIlE_PATH, "w+");
	if (fd == NULL)
	{
		out("�򿪴�������������Ϣ�ļ�ʧ�� read\n");
		return -1;
	}
	fputs(write_buf, fd);
	fclose(fd);
	sync();
	return 0;
	
}
static int read_other_net_file(struct _deploy *stream)
{
	FILE * fd;
	char *p = NULL;
	int now_index = 0;
	char net_mode[2] = { 0 };
	char write_buf[1024] = { 0 };
	char line_buf[256] = { 0 };
	//char line_buf1[256] = { 0 };
	if (access(DF_OTHER_NET_FIlE_PATH, 0) != 0)//��һ�� û�и��ļ�
	{
		fd = fopen(DF_OTHER_NET_FIlE_PATH, "w+");
		if (fd == NULL)
		{
			out("�򿪴�������������Ϣ�ļ�ʧ��\n");
			return -1;
		}
		stream->file_o.ip_mode = 0;
		memcpy(stream->file_o.wifi_name,"mb_machine",10);
		memcpy(stream->file_o.wifi_pwd, "12345678", 8);
		memcpy(stream->file_o.wifi_mode, "WPA2PSK", 7);
		sprintf(write_buf, "net_mode:0\nwifi_name:mb_machine\nwifi_pwd:12345678\nwifi_mode:WPA2PSK\n");
		//write(fd, write_buf, strlen(write_buf));
		fwrite(write_buf, strlen(write_buf), 1, fd);
		fclose(fd);
		return 0;
	}
	fd = fopen(DF_OTHER_NET_FIlE_PATH, "r");
	if (fd == NULL)
	{
		out("�򿪴�������������Ϣ�ļ�ʧ��\n");
		return -1;
	}
	while (fgets(line_buf, 256, fd))
	{
		if (now_index == 0)
		{
			p = strstr(line_buf, "net_mode:");
			if (p == NULL)
			{
				strcat(write_buf, line_buf);
				continue;
			}
			strcat(write_buf, line_buf);
			char *data = NULL;
			data = strstr(p + 9, "\n");
			memcpy(net_mode, p + 9, data - (p + 9));
			stream->file_o.ip_mode = atoi(net_mode);
			now_index = 1;
			continue;
		}
		else if (now_index == 1)
		{
			p = strstr(line_buf, "wifi_name:");
			if (p == NULL)
			{
				strcat(write_buf, line_buf);
				continue;
			}
			strcat(write_buf, line_buf);
			char *data = NULL;
			data = strstr(p + 10, "\n");
			memcpy(stream->file_o.wifi_name, p +10, data - (p + 10));
			
			now_index = 2;
			continue;
		}
		else if (now_index == 2)
		{
			p = strstr(line_buf, "wifi_pwd:");
			if (p == NULL)
			{
				strcat(write_buf, line_buf);
				continue;
			}
			strcat(write_buf, line_buf);
			char *data = NULL;
			data = strstr(p + 9, "\n");
			memcpy(stream->file_o.wifi_pwd, p + 9, data - (p + 9));
			now_index = 3;
			continue;
		}
		else if (now_index == 3)
		{
			p = strstr(line_buf, "wifi_mode:");
			if (p == NULL)
			{
				strcat(write_buf, line_buf);
				continue;
			}
			strcat(write_buf, line_buf);
			char *data = NULL;
			data = strstr(p + 10, "\n");
			memcpy(stream->file_o.wifi_mode, p + 10, data - (p + 10));
			now_index = 4;
			continue;
		}
		else
		{
			strcat(write_buf, line_buf);
		}
		//while (fgets(line_buf, 256, fd) != NULL)//��ȡ wifi_name
		//{
		//	p = strstr(line_buf1, "wifi_name:");
		//	if (p == NULL)
		//	{
		//		strcat(write_buf, line_buf1);
		//		continue;
		//	}
		//	char *data1 = NULL;
		//	data1 = strstr(p + 11, "\n");
		//	memcpy(net_mode, p + 11, data1 - (p +11));
		//}

	}
	out("............other file buf :%s\n", write_buf);
	fclose(fd);
	remove(DF_OTHER_NET_FIlE_PATH);
	fd = fopen(DF_OTHER_NET_FIlE_PATH, "w+");
	if (fd == NULL)
	{
		out("�򿪴�������������Ϣ�ļ�ʧ��\n");
		return -1;
	}
	fputs(write_buf, fd);
	fclose(fd);
	sync();
	return 0;
}
#endif

static int deploy_ctl(void *data,int fd,int cm,va_list args)
{   
	struct _deploy  *stream;
    struct _deploy_fd  *id;
    int result;
    int i;
    stream=data;
	if((stream==NULL))	
		return DF_ERR_PORT_CORE_FD_NULL;
	if((fd==0)||(fd>sizeof(stream->fd)/sizeof(stream->fd[0])))
		return DF_ERR_PORT_CORE_FD_Z;
	fd--;
	id=&stream->fd[fd];
	if(id->state==0)
		return DF_ERR_PORT_CORE_SLAVE_DEV;
    for(i=0;i<sizeof(ctl_fun)/sizeof(ctl_fun[0]);i++)
    {   
		if(cm==ctl_fun[i].cm)
        {   
			_lock_set(stream->lock);
            result=ctl_fun[i].ctl(id,args);
            _lock_un(stream->lock);
            return result;
        }    
    }
    return DF_ERR_PORT_CORE_CM;    
}

static int deploy_close(void *data,int fd)
{   
	struct _deploy  *stream;
    struct _deploy_fd  *id;
    //int j;
	stream=data;
	if((stream==NULL))	
		return DF_ERR_PORT_CORE_FD_NULL;
	if((fd==0)||(fd>sizeof(stream->fd)/sizeof(stream->fd[0])))
		return DF_ERR_PORT_CORE_FD_Z;
	fd--;
	id=&stream->fd[fd];
	if(id->state==0)
		return DF_ERR_PORT_CORE_SLAVE_DEV;
	id->state=0;
	stream->state--;
	memset(id,0,sizeof(stream->fd[0]));
    if(stream->state==0)
    {   _task_close(stream->ftp_task);
        ty_close(stream->switch_fd);
        ty_close(stream->ty_usb_fd);
        ty_close(stream->ftp_fd);
        _lock_close(stream->lock);
    }
    return 0;
}

/*==========================================ctlִ�к���==============================================*/
static int get_busid(struct _deploy_fd  *id,va_list args)
{   
	char *busid;
    busid=va_arg(args,char *);
    if(busid==NULL)
        return -1;
    memcpy(busid,id->dev->file.busid,strlen(id->dev->file.busid));
    //out("deploy:get_busid=[%s]\n",busid);
    return strlen(id->dev->file.busid);
}

static int get_port_n(struct _deploy_fd  *id,va_list args)
{   //out("deploy:get_port_n=[%d]\n",id->dev->file.port_n);
    return id->dev->file.port_n;    
}

static int get_enum_timer(struct _deploy_fd  *id,va_list args)
{   //out("deploy:get_enum_timer=[%d]\n",id->dev->file.enum_timer);
    return id->dev->file.enum_timer;    
}

static int get_ip_ip(struct _deploy_fd  *id,va_list args)
{   //out("deploy:get_ip_ip=[%d]\n",id->dev->file.ip);
	struct _net_define   net_par;
    //uint32 *ip;
	get_net(DF_NET_DEV,&net_par);
	return net_par.ip;    
}

static int get_start_port(struct _deploy_fd  *id,va_list args)
{   //out("deploy:get_start_port=[%d]\n",id->dev->file.start_port);
    return id->dev->file.start_port;    
}

static int get_vpn_par(struct _deploy_fd  *id,va_list args)
{	
	struct _vpn_deploy *vpn_deploy;

	vpn_deploy = va_arg(args,struct _vpn_deploy *);

	vpn_deploy->enable = id->dev->file.vpn_enable;
	vpn_deploy->mode = id->dev->file.vpn_mode;
	vpn_deploy->port = id->dev->file.vpn_port;
	vpn_deploy->ftp_port = id->dev->file.vpn_ftp_port;
	memcpy(vpn_deploy->addr,id->dev->file.vpn_addr,sizeof(vpn_deploy->addr));
	memcpy(vpn_deploy->user,id->dev->file.vpn_user,sizeof(vpn_deploy->user));
	memcpy(vpn_deploy->passwd,id->dev->file.vpn_passwd,sizeof(vpn_deploy->passwd));
	memcpy(vpn_deploy->ftp_user,id->dev->file.vpn_ftp_user,sizeof(vpn_deploy->ftp_user));
	memcpy(vpn_deploy->ftp_passwd,id->dev->file.vpn_ftp_passwd,sizeof(vpn_deploy->ftp_passwd));
	return 0;
}

static int get_close_update(struct _deploy_fd  *id,va_list args)
{   //out("deploy:get_start_port=[%d]\n",id->dev->file.start_port);
    return id->dev->file.close_auto_update;    
}

static int set_auth(struct _deploy_fd  *id, va_list args)
{
	struct _deploy  *stream;
	stream = id->dev;
	int port_n = 0;
	port_n = va_arg(args, int);
	stream->file.port_n = port_n;
	if (ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_SET_PORT_N, stream->file.port_n)<0)
		return -1;
	ty_ctl(stream->switch_fd, DF_SWITCH_CM_WRITE_FLASH, &stream->file, sizeof(stream->file));
	return 0;
}

static int get_net_soft(struct _deploy_fd  *id,va_list args)
{   
	char *net_soft;
    net_soft=va_arg(args,char *);
    if(net_soft==NULL)
        return -1;
    memcpy(net_soft,id->dev->file.net_soft,strlen(id->dev->file.net_soft));
    out("deploy:get_net_soft=[%s]\n",net_soft);
    return strlen(id->dev->file.net_soft);
}

static int set_busid(struct _deploy_fd  *id,va_list args)
{   char *busid;
    char name[DF_FILE_NAME_MAX_LEN];
    int len,fd;
    struct _deploy  *stream;
    busid=va_arg(args,char *);
    stream=id->dev;
    len=strlen(busid);
    out("deploy:set_busid=[%s]\n",busid);
    if(len==strlen(id->dev->file.busid))
    {   if(memcmp(busid,id->dev->file.busid,len)==0)
        {   out("deploy:��ԭ����һ��\n");
            return 0;
        }    
    }
    memset(stream->file.busid,0,sizeof(stream->file.busid));
    memcpy(stream->file.busid,busid,strlen(busid));
    if(ty_ctl(stream->ty_usb_fd,DF_TY_USB_CM_SET_BUSID,stream->file.busid)<0)
        return -1;
    out("deploy:set_busid=[%s] to switch_fd\n",busid);
    ty_ctl(stream->switch_fd,DF_SWITCH_CM_WRITE_FLASH,&stream->file,sizeof(stream->file));
    memset(name,0,sizeof(name));
    sprintf(name,"/dev/%s",DF_EVENT_FILE_NAME);
    fd=ty_open(name,0,0,NULL);       
    out("д�¼�\n");
    ty_ctl(fd,DF_EVENT_NAME_RESET,DF_EVENT_REASON_SET_BUSID);
    ty_close(fd);
    out("deploy:set_busid=[%s] to suc\n",busid);
    _queue_add("reset",reset_systerm,stream,10); 
    return 0;
}

static int update_ip_ip(struct _deploy_fd *id,va_list args)
{
	struct _deploy  *stream;	
	struct _net_define  net_par;
	stream=id->dev;
	memset(&net_par,0,sizeof(struct _net_define));
	net_par.ip=stream->file.ip;
	net_par.mask=stream->file.mask;		
	net_par.gateway[0]=stream->file.getway;
	net_par.DNS=stream->file.DNS;
	net_par.type=stream->file.ip_type;
	//out("������Ƭ��FLASH��IPΪ��%08x \n",net_par.ip);
	//out("������Ƭ��FLASH��maskΪ��%08x \n",net_par.mask);
	//out("������Ƭ��FLASH��gatewayΪ��%08x \n",net_par.gateway[0]);
	//out("������Ƭ��FLASH��DNSΪ��%08x \n",net_par.DNS);
	//out("������Ƭ��FLASH��IP����Ϊ %d\n",net_par.type);
	if(net_par.type==2)
	{	if((net_par.ip==0)&&(net_par.mask==0)&&(net_par.gateway[0]==0)&&(net_par.DNS==0))
		{
			out("�������Ϊ���ã�����������\n");
			return -1;
		}
		set_net(DF_NET_DEV,&net_par);
	}
	else if(net_par.type == 1)
		set_net(DF_NET_DEV, &net_par);
	//out("IP�������\n");
	return 0;
}
#ifdef MTK_OPENWRT
static int set_ip_wifi_wifi(struct _deploy_fd  *id, va_list args)
{
	struct _deploy  *stream;
	char *w_name,*w_pwd,*w_mode;
	//char w_name[30];
	stream = id->dev;
	w_name = (char *)va_arg(args, uint8 *);
	w_pwd = (char *)va_arg(args, uint8 *);
	w_mode = (char *)va_arg(args, uint8 *);
	memcpy(stream->file_o.wifi_name, w_name, strlen(w_name));
	memcpy(stream->file_o.wifi_pwd, w_pwd, strlen(w_pwd));
	memcpy(stream->file_o.wifi_mode, w_mode, strlen(w_mode));

	set_other_net_file(1, stream);
	_queue_add("set_ip_4", set_ip_w, stream, 100);
	return 0;

}
static int set_ip_ip(struct _deploy_fd  *id,va_list args)
{   struct _deploy  *stream;
    uint32 ip,mask,getway,DNS;
	int ip_type;
    stream=id->dev;
    ip=va_arg(args,uint32);
    mask=va_arg(args,uint32);
    getway=va_arg(args,uint32);
	DNS=va_arg(args,uint32);
	ip_type=va_arg(args,uint32);
	#if 1
	system("kill -9 $(pidof pppd)");//�����4g������ɱ���˽���
	if(ip_type==1)
	{	
		out("Ҫ���õ�IPΪ�̶�IP\n");
		lcd_print_info.now_net_mode = 0;
		if((stream->file.ip==ip)&&(stream->file.mask==mask)&&(stream->file.getway==getway)&&(stream->file.DNS==DNS)&&(stream->file.ip_type==2))
		{   
			out("��ԭ������һ��\n");
			//return 0;
		}
		else
		{
			stream->file.ip=ip;
			stream->file.mask=mask;
			stream->file.getway=getway;
			stream->file.DNS=DNS;	
			stream->file.ip_type=2;	//2Ϊ�̶�IP	

			//stream->file.ip_mode = 0;
			stream->file_o.ip_mode = 0;
			set_other_net_file(0, stream);
			out("дFLASH\n");
			ty_ctl(stream->switch_fd,DF_SWITCH_CM_WRITE_FLASH,&stream->file,sizeof(stream->file)-104);
			out("дFLASH�ɹ�\n");
		}
	}
	else if(ip_type==0)
	{	
		out("Ҫ���õ�IPΪDHCP\n");
		stream->file.ip_type=1; //1ΪDHCP
	//	stream->file.ip_mode = 0;
		stream->file_o.ip_mode = 0;
		out("дFLASH\n");
		set_other_net_file(0, stream);
		lcd_print_info.now_net_mode = 0;
		ty_ctl(stream->switch_fd,DF_SWITCH_CM_WRITE_FLASH,&stream->file,sizeof(stream->file));
	}
	
	else if (ip_type == 4)
	{
		//*if(access("/etc/ppp/peers",0) != 0)//�¸��汾����
		//{
			//system("mkdir /etc/ppp/peers");
			//sync();
			//system("cp /home/share/exe/peers/* /etc/ppp/peers/. -r");
			//sync();
	//	}*/
		_queue_add("set_ip_4", set_ip_4g, stream, 100);
		lcd_print_info.now_net_mode = 2;
		set_other_net_file(2,stream);
		return 0;
	}
	#endif
	//logout(INFO, "SYSTEM", "INIT","Set network\r\n",ip_type,ip,mask,getway,DNS);
    _queue_add("set_ip",set_ip_q,stream,100); 
    return 0;  
}
#else
static int set_ip_ip(struct _deploy_fd  *id,va_list args)
{   
	struct _deploy  *stream;
    uint32 ip,mask,getway,DNS;
	int ip_type;
    stream=id->dev;
    ip=va_arg(args,uint32);
    mask=va_arg(args,uint32);
    getway=va_arg(args,uint32);
	DNS=va_arg(args,uint32);
	ip_type=va_arg(args,uint32);
	#if 1
	
	if(ip_type==1)
	{	
		out("Ҫ���õ�IPΪ�̶�IP\n");
		
		if((stream->file.ip==ip)&&(stream->file.mask==mask)&&(stream->file.getway==getway)&&(stream->file.DNS==DNS)&&(stream->file.ip_type==2))
		{   
			out("��ԭ������һ��\n");
			//return 0;
		}
		else
		{
			stream->file.ip=ip;
			stream->file.mask=mask;
			stream->file.getway=getway;
			stream->file.DNS=DNS;	
			stream->file.ip_type=2;	//2Ϊ�̶�IP		
			out("дFLASH\n");
			ty_ctl(stream->switch_fd,DF_SWITCH_CM_WRITE_FLASH,&stream->file,sizeof(stream->file));
			out("дFLASH�ɹ�\n");
		}
	}
	else if(ip_type==0)
	{	
		out("Ҫ���õ�IPΪDHCP\n");
		stream->file.ip_type=1; //1ΪDHCP
		out("дFLASH\n");
		
		ty_ctl(stream->switch_fd,DF_SWITCH_CM_WRITE_FLASH,&stream->file,sizeof(stream->file));
		
	}
    
	#endif
	//logout(INFO, "SYSTEM", "INIT","Set network\r\n",ip_type,ip,mask,getway,DNS);
    _queue_add("set_ip",set_ip_q,stream,100); 
    return 0;  
}
#endif
//int set_ip_by_overall(uint32 ip,uint32 mask,uint32 getway,uint32 DNS,int ip_type)
//{   
//	struct _deploy  *stream;
//	
//    stream = fd_stream;
//
//	#if 1
//
//	if(ip_type==1)
//	{	
//		out("Ҫ���õ�IPΪ�̶�IP\n");
//		if((stream->file.ip==ip)&&(stream->file.mask==mask)&&(stream->file.getway==getway)&&(stream->file.DNS==DNS)&&(stream->file.ip_type==2))
//		{   
//			out("��ԭ������һ��\n");
//			//return 0;
//		}
//		else
//		{
//			stream->file.ip=ip;
//			stream->file.mask=mask;
//			stream->file.getway=getway;
//			stream->file.DNS=DNS;	
//			stream->file.ip_type=2;	//2Ϊ�̶�IP		
//			out("дFLASH\n");
//			ty_ctl(stream->switch_fd,DF_SWITCH_CM_WRITE_FLASH,&stream->file,sizeof(stream->file));
//		}
//	}
//	else if(ip_type==0)
//	{	
//		out("Ҫ���õ�IPΪDHCP\n");
//		stream->file.ip_type=1; //1ΪDHCP
//		out("дFLASH\n");
//		ty_ctl(stream->switch_fd,DF_SWITCH_CM_WRITE_FLASH,&stream->file,sizeof(stream->file));
//	}
//    
//	#endif
//    _queue_add("set_ip",set_ip_q,stream,100); 
//    return 0;  
//}

static int set_enum_timer(struct _deploy_fd  *id,va_list args)
{   
	struct _deploy  *stream;
    int enum_timer;
    stream=id->dev;
    enum_timer=va_arg(args,int);
    out("deploy:set_enum_timer=[%d]\n",enum_timer);
    if(stream->file.enum_timer==enum_timer)
    {   
		out("set_enum_timer:��ԭ����һ��\n");
        return 0;
    }
    stream->file.enum_timer=enum_timer;
    out("deploy:set_enum_timer=[%d] to switch_fd\n",enum_timer);
    ty_ctl(stream->switch_fd,DF_SWITCH_CM_WRITE_FLASH,&stream->file,sizeof(stream->file));
    return ty_ctl(stream->ty_usb_fd,DF_TY_USB_CM_SET_ENUM_TIMER,enum_timer*1000);     
}

static int set_port_n(struct _deploy_fd  *id,va_list args)
{   
	struct _deploy  *stream;
    int port_n;
    stream=id->dev;
    port_n=va_arg(args,int);
    out("deploy:set_port_n=[%d]\n",port_n);
    if(stream->file.port_n==port_n)
    {   
		out("set_port_n:��ԭ����һ��\n");
        return 0;
    }
    stream->file.port_n=port_n;
    ty_ctl(stream->switch_fd,DF_SWITCH_CM_WRITE_FLASH,&stream->file,sizeof(stream->file));
    out("deploy:set_port_n=[%d] to switch_fd\n",port_n);
    return ty_ctl(stream->ty_usb_fd,DF_TY_USB_CM_SET_PORT_N,port_n);
}

static int set_start_port(struct _deploy_fd  *id,va_list args)
{   
	struct _deploy  *stream;
    int start_port;
    start_port=va_arg(args,int);
    stream=id->dev;
    out("deploy:set_start_port=[%d]\n",start_port);
    if(stream->file.start_port==start_port)
    {   
		out("set_start_port:��ԭ����һ��\n");
        return 0;
    }       
    stream->file.start_port=start_port;
    out("deploy:start_port=[%d] to switch_fd\n",start_port);
	//ty_ctl(stream->switch_fd,DF_SWITCH_CM_WRITE_FLASH,&stream->file,sizeof(stream->file));
    return ty_ctl(stream->switch_fd,DF_SWITCH_CM_WRITE_FLASH,&stream->file,sizeof(stream->file));  
}

static int write_power_mode_flash(struct _deploy_fd  *id, va_list args)
{
	struct _deploy  *stream;
	int mode;
	mode = va_arg(args, int);
	stream = id->dev;

	if (stream->file.close_auto_update == mode)
	{
		out("write_power_mode_flash:��ԭ����һ��\n");
		return 0;
	}
	stream->file.close_auto_update = mode;
	printf(" write mode :%d\n", stream->file.close_auto_update);
	ty_ctl(stream->switch_fd, DF_SWITCH_CM_WRITE_FLASH, &stream->file, sizeof(stream->file));
	return 0;
}
static int read_power_mode_flash(struct _deploy_fd  *id, va_list args)
{
	struct _deploy  *stream;
	stream = id->dev;

	printf(" read mode :%d\n", stream->file.close_auto_update);
	return stream->file.close_auto_update;
}

static int set_vpn_par(struct _deploy_fd  *id,va_list args)
{   
	struct _deploy  *stream;
    struct _vpn_deploy *vpn_deploy;
	out("����deploy VPN���ú���\n");
	vpn_deploy = va_arg(args,struct _vpn_deploy *);
	out("�ж�Ҫ���õ�VPN�������ڴ����Ƿ�һ��\n");
    stream=id->dev;
	if(stream->file.vpn_enable == vpn_deploy->enable&&stream->file.vpn_mode == vpn_deploy->mode&&stream->file.vpn_port == vpn_deploy->port&&stream->file.vpn_ftp_port == vpn_deploy->ftp_port)
    {   
		if(memcmp(stream->file.vpn_addr,vpn_deploy->addr,sizeof(stream->file.vpn_addr))==0)
		{	
			if(memcmp(stream->file.vpn_user,vpn_deploy->user,sizeof(stream->file.vpn_user))==0)
			{	
				if(memcmp(stream->file.vpn_passwd,vpn_deploy->passwd,sizeof(stream->file.vpn_passwd))==0)
				{	
					if(memcmp(stream->file.vpn_ftp_user,vpn_deploy->ftp_user,sizeof(stream->file.vpn_ftp_user))==0)
					{	
						if(memcmp(stream->file.vpn_ftp_passwd,vpn_deploy->ftp_passwd,sizeof(stream->file.vpn_ftp_passwd))==0)
						{	
							out("Ҫ���õ�VPN������ԭ����һ��\n");
							return 0;
						}	
					}					
				}
			}
		}
    }       
	stream->file.vpn_enable=vpn_deploy->enable;
	stream->file.vpn_mode=vpn_deploy->mode;
	stream->file.vpn_port=vpn_deploy->port;
	stream->file.vpn_ftp_port=vpn_deploy->ftp_port;
	memcpy(stream->file.vpn_addr,vpn_deploy->addr,sizeof(stream->file.vpn_addr));
	memcpy(stream->file.vpn_user,vpn_deploy->user,sizeof(stream->file.vpn_user));
	memcpy(stream->file.vpn_passwd,vpn_deploy->passwd,sizeof(stream->file.vpn_passwd));
	memcpy(stream->file.vpn_ftp_user,vpn_deploy->ftp_user,sizeof(stream->file.vpn_ftp_user));
	memcpy(stream->file.vpn_ftp_passwd,vpn_deploy->ftp_passwd,sizeof(stream->file.vpn_ftp_passwd));
    return ty_ctl(stream->switch_fd,DF_SWITCH_CM_WRITE_FLASH,&stream->file,sizeof(stream->file));  
}
static int get_wifi_par(struct _deploy_fd  *id, va_list args)
{
	struct _deploy  *stream;
	struct _net_define   net_par;
	uint32 *ip, *mask, *getway, *dns;
	int *type;
	stream = id->dev;
	ip = va_arg(args, uint32 *);
	mask = va_arg(args, uint32 *);
	getway = va_arg(args, uint32 *);
	dns = va_arg(args, uint32 *);
	type = va_arg(args, int *);
	get_net("wlan0", &net_par);
	//out("��ȡ�������\n");
	if (ip != NULL)
	{
		*ip = net_par.ip;

	}
	if (mask != NULL)
	{
		*mask = net_par.mask;

	}
	if (getway != NULL)
	{
		*getway = net_par.gateway[0];

	}
	if (dns != NULL)
	{
		//*dns=stream->file.DNS;
		*dns = net_par.DNS;

	}
	if (type != NULL)
	{
		*type = stream->file.ip_type - 1;

	}
	char tmp[100] = {0};
	memset(tmp, 0, sizeof(tmp));
	ip2ip_asc(*ip, tmp);
	//out("��ȡ����wifi IP :%s\n", tmp);

	memset(tmp, 0, sizeof(tmp));
	ip2ip_asc(*mask, tmp);
	//out("��ȡ����wifi mask :%s\n", tmp);

	memset(tmp, 0, sizeof(tmp));
	ip2ip_asc(*getway, tmp);
	//out("��ȡ����wifi gateway :%s\n", tmp);

	memset(tmp, 0, sizeof(tmp));
	ip2ip_asc(*dns, tmp);
	//out("��ȡ����wifi DNS :%s\n", tmp);
	return 0;
}
static int get_net_par(struct _deploy_fd  *id,va_list args)
{   
	struct _deploy  *stream;  
	struct _net_define   net_par;
    uint32 *ip,*mask,*getway,*dns;
	int *type;
    stream=id->dev;
    ip=va_arg(args,uint32 *);
    mask=va_arg(args,uint32 *);
    getway=va_arg(args,uint32 *);
	dns=va_arg(args,uint32 *);
	type = va_arg(args,int *);
	get_net(DF_NET_DEV,&net_par);
    //out("��ȡ�������\n");
    if(ip!=NULL)
    {   
		*ip=net_par.ip;

    }
    if(mask!=NULL)
    {   
		*mask=net_par.mask;

    }
    if(getway!=NULL)
    {   
		*getway=net_par.gateway[0];

    }
	if(dns!=NULL)
    {   
		//*dns=stream->file.DNS;
		*dns = net_par.DNS;

    }
	if(type!=NULL)
    {   
		*type=stream->file.ip_type-1;

    }
    return 0;    
}




//static void deploy_task(void *arg,int timer)
//{   struct _deploy  *stream;
//    stream=arg;
//    if(ty_read(stream->ftp_fd,NULL,0)==1)
//    {   int fd;
//        char name[DF_FILE_NAME_MAX_LEN];
//        out("ftp���µ����,����3�����������\n");
//		sleep(3);
//        memset(name,0,sizeof(name));
//        sprintf(name,"/dev/%s",DF_EVENT_FILE_NAME);
//        fd=ty_open(name,0,0,NULL);
//        if(fd<0)
//            return ;        
//        out("д�¼�\n");
//        ty_ctl(fd,DF_EVENT_NAME_RESET,DF_EVENT_REASON_SOFT);
//        ty_close(fd);
//        memset(name,0,sizeof(name));
//        sprintf(name,"/dev/%s",DF_TY_PD_NAME);
//        fd=ty_open(name,0,0,NULL);
//        ty_ctl(fd,DF_TY_PD_CM_RESET);
//    }
//	else if(ty_read(stream->sd_fd,NULL,0)==1)
//    {   int fd;
//        char name[DF_FILE_NAME_MAX_LEN];
//        out("sd���µ����,����3�����������\n");
//		sleep(3);
//        memset(name,0,sizeof(name));
//        sprintf(name,"/dev/%s",DF_EVENT_FILE_NAME);
//        fd=ty_open(name,0,0,NULL);
//        if(fd<0)
//            return ;        
//        out("д�¼�\n");
//        ty_ctl(fd,DF_EVENT_NAME_RESET,DF_EVENT_REASON_SOFT);
//        ty_close(fd);
//        memset(name,0,sizeof(name));
//        sprintf(name,"/dev/%s",DF_TY_PD_NAME);
//        fd=ty_open(name,0,0,NULL);
//        ty_ctl(fd,DF_TY_PD_CM_RESET);
//    }
//}







static void reset_systerm(void *arg)
{   //struct _deploy  *stream;
    char name[DF_FILE_NAME_MAX_LEN];
    int fd;
    //stream=arg;
    printf("reset_systerm\n");
    memset(name,0,sizeof(name));
    sprintf(name,"/dev/%s",DF_TY_PD_NAME);
    fd=ty_open(name,0,0,NULL);
    ty_ctl(fd,DF_TY_PD_CM_RESET);
}
#ifdef MTK_OPENWRT
static void set_ip_4g(void *arg)//4g
{
	//struct _deploy *stream;
	struct _net_define net_par;
	//stream = arg;
	memset(&net_par, 0, sizeof(net_par));
	net_par.net_mode = 2;
	out("set 4g config!\n");
	set_net("4g", &net_par);
	return;
}
static void set_ip_w(void *arg)//wifi
{
	struct _deploy *stream;
	struct _net_define net_par;
	stream = arg;
	memset(&net_par, 0, sizeof(net_par));
	net_par.net_mode = 1;
	memcpy(net_par.wifi_name, stream->file_o.wifi_name, strlen(stream->file_o.wifi_name));
	memcpy(net_par.wifi_pwd, stream->file_o.wifi_pwd, strlen(stream->file_o.wifi_pwd));
	memcpy(net_par.wifi_mode, stream->file_o.wifi_mode, strlen(stream->file_o.wifi_mode));
	out("set wifi config!\n");
	set_net("wan", &net_par);
	return;
}
#endif
static void set_ip_q(void *arg)
{   
	struct _deploy  *stream;
    struct _net_define   net_par;
    stream=arg;
    memset(&net_par,0,sizeof(net_par));
    net_par.gateway[0]=stream->file.getway;
    net_par.ip=stream->file.ip;
    net_par.mask=stream->file.mask;
	net_par.type=stream->file.ip_type;
	net_par.DNS=stream->file.DNS;
	out("���û����������\n");
    set_net(DF_NET_DEV,&net_par);

    return ;
}






