#define _get_net_state_c
#include "_get_net_state.h"
int get_net_state_add(void)
{   struct _get_net_state *stream;
	int result;
	stream=malloc(sizeof(struct _get_net_state));
	if(stream==NULL)
		return DF_ERR_MEM_ERR;
	memset(stream,0,sizeof(struct _get_net_state));
    //out("增加\"/dev/get_net_state\"文件至文件系统\n");
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
   // out("打开get_net_state模块\n");
    if(stream->state==0)
    {   //out("首次使用get_net_state_open\n");
        stream->switch_dev=ty_open("/dev/switch",0,0,NULL);
		//out("stream->switch_dev = %02x\n",stream->switch_dev);
		stream->deploy=ty_open("/dev/deploy",0,0,NULL);
        if(stream->switch_dev<0)
        {   out("打开switch设备失败\n");
            return stream->switch_dev;
        }
		stream->machine_fd = ty_open("/dev/machine", 0, 0, NULL);	
		ty_ctl(stream->machine_fd, DF_MACHINE_INFOR_CM_GET_MODE_SUPPORT, &stream->mode_support);
#ifdef MTK_OPENWRT
		get_net_mac("eth0.2", stream->eth_mac);
#else
		get_net_mac("eth0",stream->eth_mac);
		get_net_mac("wlan0", stream->wlan_mac);
#endif
		logout(INFO, "SYSTEM", "INIT", "机柜MAC地址：%s\r\n", stream->eth_mac);
		out("获取到的eth0的mac地址为%s\n", stream->eth_mac);
		stream->lcd_fd = ty_open("/dev/lcd_state", 0, 0, NULL);
#ifdef MTK_OPENWRT
		stream->ty_usb_fd = ty_open("/dev/ty_usb",0,0,NULL);
		if (stream->ty_usb_fd<0)
		{
			out("打开USB[/dev/ty_usb]模块失败\n");
			ty_close(stream->switch_dev);
			return stream->ty_usb_fd;
		}
#endif
		stream->task=_task_open("_get_net_state",_get_ip_state_task,stream,DF_GET_IP_STATE_TASK_TIMER,NULL);
		if(stream->task<0)
		{
			out("开启查询网络状态线程失败\n");
			return stream->task;
		}
		// out("get_net_state模块初始化成功\n");
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
    //out("打开get_net_state成功:j=%d\n",j);
	return j+1;	 
}
#ifndef MTK_OPENWRT
static int _check_sdcard_exist(struct _get_net_state *stream)
{
	
	if (access("/dev/mmcblk1p1",0) < 0)
	{
		if (stream->sdcard_state != 10)
		{
			lcd_print_info.sd_state = 0;
			ty_ctl(stream->lcd_fd, DF_LCD_STATE_UPDATE_STATE);
			stream->sdcard_state = 10;
		}
		return 0;
	}
	if ((access("/sdcard/mysql", 0) < 0) && (access("/sdcard/sdcard.txt", 0) < 0))
	{
		if (stream->sdcard_state != 20)
		{
			lcd_print_info.sd_state = 1;
			ty_ctl(stream->lcd_fd, DF_LCD_STATE_UPDATE_STATE);
			stream->sdcard_state = 20;
		}
		return 0;
	}
	else
	{
		if (stream->sdcard_state != 30)
		{
			lcd_print_info.sd_state = 2;
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
		//out("解析www.baidu.com域名失败\n");
		sprintf(net_status.internet, "0");
		goto END;
	}

	//if (connect_ip_test(server_ip, 21, 3) < 0)
	//{
	//	out("解析www.njmbxx.com ftp端口失败\n");
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
#endif
#ifdef MTK_OPENWRT
static int get_net_state_mtk(struct _get_net_state *stream)
{
	char buf1[1024] = { 0 };
	char cmd[300] = { 0 };
	int result;
	char server_ip[30] = { 0 };

	uint32 ip, mask, getway, dns;
	int type;
	struct _net_status net_status;
	memset(&net_status, 0, sizeof(net_status));
	memcpy(net_status.mac, stream->eth_mac, strlen(stream->eth_mac));
	ty_ctl(stream->deploy, DF_DEPLOY_CM_GET_NET_PAR, &ip, &mask, &getway, &dns, &type);
	ip2ip_asc(ip, net_status.ip_address);
	ip2ip_asc(mask, net_status.netmask);
	ip2ip_asc(getway, net_status.gateway);
	ip2ip_asc(dns, net_status.dns);
	sprintf(net_status.ip_type, "%d", type);
	sprintf(net_status.first_net, "1");

	memcpy(stream->net_status.netmask, net_status.netmask, strlen(net_status.netmask));
	memcpy(stream->net_status.gateway, net_status.gateway, strlen(net_status.gateway));
	memcpy(stream->net_status.ip_address, net_status.ip_address, strlen(net_status.ip_address));
	memcpy(stream->net_status.ip_type, net_status.ip_type, strlen(net_status.ip_type));
	memcpy(stream->net_status.mac, net_status.mac, strlen(net_status.mac));
	memcpy(stream->net_status.dns, net_status.dns, strlen(net_status.dns));
	memcpy(stream->net_status.first_net, net_status.first_net, strlen(net_status.first_net));

	switch (lcd_print_info.now_net_mode)
	{
	case 0://LAN
		memset(cmd, 0, sizeof(cmd));
		sprintf(cmd, "swconfig dev rt305x port 0 get link | awk \'{print $2}\' | cut -d"":"" -f2");
		mb_system_read(cmd, buf1, sizeof(buf1),"/tmp/swconfig_dev_rt305x4.txt");
		if (strstr(buf1, "up") != NULL)//网线插入
		{
			result = get_socket_net_state("eth0.2");
			if (result < 0)
			{
				out("lan获取IP状态失败\n");
				logout(INFO, "SYSTEM", "网络检测","lan获取内网异常!\r\n");
				return NET_ERROR_NOT_FOUND_IP;
			}
			sleep(1);
			if (get_DNS_ip("www.baidu.com", server_ip) < 0)
			{
				out("lan解析www.baidu.com域名失败\n");
				//可能是DHCP指令出现异常
				uint32 ip, mask, getway, dns;
				int type;
				struct _net_status net_status;
				memset(&net_status, 0, sizeof(net_status));
				memcpy(net_status.mac, stream->eth_mac, strlen(stream->eth_mac));
				ty_ctl(stream->deploy, DF_DEPLOY_CM_GET_NET_PAR, &ip, &mask, &getway, &dns, &type);
				if(type != 0)
				{
					printf("############kill dhcp !!!!#########\n");
					memset(cmd, 0, sizeof(cmd));
					sprintf(cmd, "killall udhcpc");
					system(cmd);
				}
				//sprintf(cmd, "killall udhcpc");
				//system(cmd);
				logout(INFO, "SYSTEM", "网络检测","lan获取外网异常!\r\n");
				return NET_ERROR_NOT_LINK_NETWORK;
			}
			return 0;
		}
		else
		{
			logout(INFO, "SYSTEM", "网络检测","lan网线插入异常!\r\n");
			return NET_ERROR_NO_ETHERNET_CABEL;
		}
	case 1://WIFI
		result = get_socket_net_state("apcli0");
		if (result < 0)
		{
			out("wifi获取IP状态失败\n");
			return NET_ERROR_NOT_FOUND_IP;
		}
		memset(server_ip, 0, sizeof(server_ip));
		if (get_DNS_ip("www.baidu.com", server_ip) < 0)
		{
			out("wifi解析www.baidu.com域名失败\n");
			return NET_ERROR_NOT_LINK_NETWORK;
		}
		break;
	case 2://4G
		if (access("/dev/ttyUSB0", 0) != 0)
		{
			logout(INFO, "SYSTEM", "网络检测","4g外网异常，4g模块启动异常!\r\n");
			stream->net_status.last_status = 1;
			return NET_ERROR_NO_ETHERNET_CABEL;
		}
		result = get_socket_net_state("ppp0");
		if (result < 0)
		{
			out("4g获取IP状态失败\n");
			logout(INFO, "SYSTEM", "网络检测","4g外网异常，ppp0获取IP地址异常!\r\n");
			stream->net_status.last_status = 1;
			return NET_ERROR_NOT_FOUND_IP;
		}
		memset(server_ip, 0, sizeof(server_ip));
		if(stream->net_status.last_status == 0)
		{
			sleep(60);//一分钟检测一次
		}
		if (get_DNS_ip("www.baidu.com", server_ip) < 0)
		{
			out("4g解析www.baidu.com域名失败\n");
			logout(INFO, "SYSTEM", "网络检测","4g外网异常，域名解析失败!\r\n");
			stream->net_status.last_status = 1;
			return NET_ERROR_NOT_LINK_NETWORK;
		}
		stream->net_status.last_status = 0;
		break;

	default:
		break;
	}
	return 0;
}
static void _get_ip_state_task(void *arg,int o_timer)
{	

	
	int result;
	
	//out("进入IP检查线程\n");
	
#ifdef MTK_OPENWRT
	int inter_flag = 0;
	struct _get_net_state *stream;
	stream = (struct _get_net_state *)arg;
	//check_net_internet(stream);//49版本注释这个
	result = get_net_state_mtk(stream);
	switch (lcd_print_info.now_net_mode)
	{
	case 0://LAN
		if (result < 0)
		{
			if (result == NET_ERROR_NO_ETHERNET_CABEL)
			{
				printf("LAN NET NET_ERROR_NO_ETHERNET_CABEL!\n");
				ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_PORT_RED_OFF_GREN_OFF, 1002);
				ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_PORT_RED_OFF_GREN_OFF, 1003);
				ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_PORT_RED_OFF_GREN_OFF, 1004);
			}
			else if (result == NET_ERROR_NOT_FOUND_IP)
			{
				printf("LAN NET NET_ERROR_NOT_FOUND_IP!\n");
				ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_PORT_RED_TWI_GREN_TWI, 1002);
				ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_PORT_RED_OFF_GREN_OFF, 1003);
				ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_PORT_RED_OFF_GREN_OFF, 1004);
			}
			else if (result == NET_ERROR_NOT_LINK_NETWORK)
			{
				printf("LAN NET NET_ERROR_NOT_LINK_NETWORK!\n");
				ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_PORT_RED_ON_GREN_OFF, 1002);
				ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_PORT_RED_OFF_GREN_OFF, 1003);
				ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_PORT_RED_OFF_GREN_OFF, 1004);
			}
			
		}
		else
		{
			//printf("NORMAL LAN NET!\n");
			inter_flag = 1;
			ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_PORT_RED_OFF_GREN_ON, 1002);
			ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_PORT_RED_OFF_GREN_OFF, 1003);
			ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_PORT_RED_OFF_GREN_OFF, 1004);
		}
		break;
	case 1://WIFI
		if (result < 0)
		{
			if (result == NET_ERROR_NOT_FOUND_IP)
			{
				printf("WIFI NET NET_ERROR_NOT_FOUND_IP!\n");
				ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_PORT_RED_TWI_GREN_TWI, 1003);
				ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_PORT_RED_OFF_GREN_OFF, 1002);
				ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_PORT_RED_OFF_GREN_OFF, 1004);
			}
			else if (result == NET_ERROR_NOT_LINK_NETWORK)
			{
				printf("WIFI NET NET_ERROR_NOT_LINK_NETWORK!\n");
				ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_PORT_RED_ON_GREN_OFF,  1003);
				ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_PORT_RED_OFF_GREN_OFF, 1002);
				ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_PORT_RED_OFF_GREN_OFF, 1004);
			}
		}
		else
		{
		//	printf("NORMAL WIFI NET!\n");
			inter_flag = 1;
			ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_PORT_RED_OFF_GREN_ON,  1003);
			ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_PORT_RED_OFF_GREN_OFF, 1002);
			ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_PORT_RED_OFF_GREN_OFF, 1004);
		}
		break;
	case 2://4G
		if (result < 0)
		{
			
				printf("WIFI NET NET_ERROR_NOT_FOUND_IP!\n");
				ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_PORT_RED_OFF_GREN_OFF, 1003);
				ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_PORT_RED_OFF_GREN_OFF, 1002);
				ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_PORT_RED_ON_GREN_OFF, 1004);
			
		}
		else
		{
			//	printf("NORMAL WIFI NET!\n");
			inter_flag = 1;
			ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_PORT_RED_OFF_GREN_OFF,  1003);
			ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_PORT_RED_OFF_GREN_OFF, 1002);
			ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_PORT_RED_OFF_GREN_ON, 1004);
		}
		break;
	default:
		break;
	}
	
	if ((inter_flag == 1)&&(stream->internet_info_flag == 0) && (stream->internet_info_err_num < 10))
	{
		out("未获取公网信息,此处去获取公网信息\n");

		result = get_internet_infomation(stream);
		if (result < 0)
		{
			out("获取公网信息失败\n");
			stream->internet_info_err_num += 1;
		}
		else
		{
			stream->internet_info_flag = 1;
		}		
	}
	char time_now[20] = { 0 };
	//out("检测时间是否为初始时间\n");
	timer_read_y_m_d_h_m_s(time_now);
	//out("获取到的系统时间为%s\n", time_now);

	if (strcmp(time_now, SYSTEM_LEAST_TIME) > 0)
	{
		//out("对时成功\n");
		stream->net_status.nettime_ok = 1;
	}
	else
	{
		//out("对时未成功\n");
		stream->net_status.nettime_ok = 0;
	}

	sleep(5);
	//char buf1[1024] = { 0 };
	//char cmd[300] = { 0 };
	//sleep(5);
	//memset(cmd, 0, sizeof(cmd));
	//sprintf(cmd, "swconfig dev rt305x port 0 get link | awk \'{print $2}\' | cut -d"":"" -f2");
	//mb_system_read(cmd, buf1, sizeof(buf1));

	////char buf1[100] = { 0 };
	////FILE* file;
	////file = popen("swconfig dev rt305x port 0 get link | awk \'{print $2}\' | cut -d"":"" -f2", "r"); //将命令的输出 通过管道读取（“r”参数）到FILE* stream  

	//////out("进入newnwewnewnwIP检查线程\n");
	////fread(buf1, sizeof(char), sizeof(buf1), file); //将刚刚FILE* stream的数据流读取到buf中
	////pclose(file);
	////printf("status:%s\n", buf1);
	//if (strstr(buf1, "up") != NULL)//网线插入
	//{
	//	if (stream->net_state != 20)
	//	{
	//		out("获取到的网络状态为在线1\n");
	//		ty_ctl(stream->switch_dev, DF_SWITCH_CM_GET_NET_OK);
	//		lcd_print_info.eth_state = 1;
	//		lcd_print_info.mqtt_state = 0;
	//		lcd_print_info.udp_state = 0;
	//		memset(&lcd_print_info.local_ip, 0, sizeof(lcd_print_info.local_ip));
	//		memcpy(&lcd_print_info.local_ip, stream->net_status.ip_address, strlen(stream->net_status.ip_address));
	//		ty_ctl(stream->lcd_fd, DF_LCD_STATE_UPDATE_STATE);
	//		stream->net_state = 20;//在线状态
	//	}
	//}
	//else
	//{
	//	if (stream->net_state != 10)
	//	{
	//		out("获取到的网络状态为离线0\n");
	//		ty_ctl(stream->switch_dev, DF_SWITCH_CM_NO_NET);

	//		lcd_print_info.mqtt_state = 0;
	//		lcd_print_info.udp_state = 0;
	//		lcd_print_info.eth_state = 0;
	//		memset(&lcd_print_info.local_ip, 0, sizeof(lcd_print_info.local_ip));
	//		memcpy(&lcd_print_info.local_ip, stream->net_status.ip_address, strlen(stream->net_status.ip_address));
	//		ty_ctl(stream->lcd_fd, DF_LCD_STATE_UPDATE_STATE);
	//		stream->net_state = 10;//离线状态
	//	}
	//}

	return ;
#else

	int result;
	check_net_internet(stream);
	_check_sdcard_exist(stream);
	result = get_socket_net_state("eth0.2");
	if(result < 0)
	{
		out("获取IP状态失败\n");
		return ;
	}
	else if(result == 0)
	{	if(stream->net_state != 10)
		{
			out("获取到的网络状态为离线0\n");
			ty_ctl(stream->switch_dev,DF_SWITCH_CM_NO_NET);

			lcd_print_info.mqtt_state = 0;
			lcd_print_info.udp_state = 0;
			lcd_print_info.eth_state = 0;
			memset(&lcd_print_info.local_ip, 0, sizeof(lcd_print_info.local_ip));
			memcpy(&lcd_print_info.local_ip, stream->net_status.ip_address, strlen(stream->net_status.ip_address));
			ty_ctl(stream->lcd_fd, DF_LCD_STATE_UPDATE_STATE);
			stream->net_state = 10;//离线状态
			logout(INFO, "SYSTEM", "网络检测","检测到网线连接异常\n");
		}
		ty_ctl(stream->deploy,DF_DEPLOY_CM_SET_IP_UPDATE);
		//system("chmod 777 /etc/init.d/auto_config_network");
		//system("/etc/init.d/auto_config_network");
	}
	else if(result == 1)
	{	if(stream->net_state != 20)
		{
			out("获取到的网络状态为在线1\n");
			ty_ctl(stream->switch_dev, DF_SWITCH_CM_GET_NET_OK);
			lcd_print_info.eth_state = 1;
			lcd_print_info.mqtt_state = 0;
			lcd_print_info.udp_state = 0;
			memset(&lcd_print_info.local_ip, 0, sizeof(lcd_print_info.local_ip));
			memcpy(&lcd_print_info.local_ip, stream->net_status.ip_address, strlen(stream->net_status.ip_address));
			ty_ctl(stream->lcd_fd, DF_LCD_STATE_UPDATE_STATE);
			stream->net_state = 20;//在线状态
		}
	}							
	else
	{
		out("获取网络状态错误\n");
		if (stream->net_state != 30)
		{
			ty_ctl(stream->switch_dev, DF_SWITCH_CM_GET_NET_OK);
			lcd_print_info.mqtt_state = 1;
			lcd_print_info.udp_state = 1;
			lcd_print_info.eth_state = 2;
			memset(&lcd_print_info.local_ip, 0, sizeof(lcd_print_info.local_ip));
			memcpy(&lcd_print_info.local_ip, stream->net_status.ip_address, strlen(stream->net_status.ip_address));
			ty_ctl(stream->lcd_fd, DF_LCD_STATE_UPDATE_STATE);
			stream->net_state = 30;//在线状态
		}
	}
else
	{
		out("获取网络状态错误\n");
	}	

	if ((stream->net_state == 30) && (stream->internet_info_flag == 0) && (stream->internet_info_err_num < 10))
	{
		out("未获取公网信息,此处去获取公网信息\n");

		result = get_internet_infomation(stream);
		if (result < 0)
		{
			out("获取公网信息失败\n");
			stream->internet_info_err_num += 1;
		}
		else
		{
			stream->internet_info_flag = 1;
		}		
	}
	char time_now[20] = { 0 };
	//out("检测时间是否为初始时间\n");
	timer_read_y_m_d_h_m_s(time_now);
	//out("获取到的系统时间为%s\n", time_now);

	if (strcmp(time_now, SYSTEM_LEAST_TIME) > 0)
	{
		//out("对时成功\n");
		stream->net_status.nettime_ok = 1;
	}
	else
	{
		//out("对时未成功\n");
		stream->net_status.nettime_ok = 0;
	}

	sleep(5);
#endif	
	return ;
}
#else
static void _get_ip_state_task(void *arg,int o_timer)
{	
	int result;
	int eth0_ok = -1;
	int wlan0_ok = -1;
	struct _get_net_state *stream;
	stream = arg;
	//out("\n\n进入IP检查线程,stream->net_state= %d\n", stream->net_state);

	//out("检测SD卡\n");
	
	_check_sdcard_exist(stream);
	eth0_ok = get_socket_net_state("eth0");
	if (atoi(stream->mode_support.wifi_support) == 1)
	{
		wlan0_ok = get_socket_net_state("wlan0");
	}
	//logout(INFO, "SYSTEM", "网络检测", "网口状态%d, wifi状态%d\r\n", eth0_ok,wlan0_ok);
	if ((eth0_ok < 0 && wlan0_ok <0) || (eth0_ok < 0 && wlan0_ok == 0) || (eth0_ok < 0 && wlan0_ok ==1))
	{
		if (wlan0_ok < 0)
		{
			if (stream->wifi_state != DF_SWITCH_CM_PORT_RED_OFF_GREN_OFF)
			{	//不支持wifi,灯全灭
				out("1不支持wifi,灯全灭\n");
				ty_ctl(stream->switch_dev, DF_SWITCH_CM_PORT_RED_OFF_GREN_OFF, DF_WIFI_LED_SAME_USBPORT);
				stream->wifi_state = DF_SWITCH_CM_PORT_RED_OFF_GREN_OFF;
			}
		}

		logout(INFO, "SYSTEM", "网络检测", "检测到网卡未启动,即将重启网卡\r\n");
		ty_ctl(stream->switch_dev, DF_SWITCH_CM_MAC_RESTART_POWER);
		stream->restart_mac_count += 1;
		out("获取IP状态失败\n");
		sleep(5 * stream->restart_mac_count);
		if (stream->restart_mac_count>3)
		{
			ty_ctl(stream->switch_dev, DF_SWITCH_CM_RESET_MA);//重启
			sleep(10);
		}
		return;
	}
	else if (eth0_ok == 0 && wlan0_ok < 0)
	{
		if (stream->wifi_state != DF_SWITCH_CM_PORT_RED_OFF_GREN_OFF)
		{	//不支持wifi,灯全灭
			out("2不支持wifi,灯全灭\n");
			ty_ctl(stream->switch_dev, DF_SWITCH_CM_PORT_RED_OFF_GREN_OFF, DF_WIFI_LED_SAME_USBPORT);
			stream->wifi_state = DF_SWITCH_CM_PORT_RED_OFF_GREN_OFF;
		}
		ty_ctl(stream->deploy, DF_DEPLOY_CM_SET_IP_UPDATE);
		if (stream->net_state != 10)//网线离线
		{
			out("获取到的网络状态为离线0\n");
			ty_ctl(stream->switch_dev, DF_SWITCH_CM_NO_NET);
			lcd_print_info.mqtt_state = 0;
			lcd_print_info.udp_state = 0;
			lcd_print_info.eth_state = 0;
			memset(&lcd_print_info.local_ip, 0, sizeof(lcd_print_info.local_ip));
			memcpy(&lcd_print_info.local_ip, stream->net_status.ip_address, strlen(stream->net_status.ip_address));
			ty_ctl(stream->lcd_fd, DF_LCD_STATE_UPDATE_STATE);
			stream->net_state = 10;//离线状态
			logout(INFO, "SYSTEM", "网络检测", "检测到网线连接异常\r\n");
		}
	}
	else if (eth0_ok == 0 && wlan0_ok == 0)
	{
		if (stream->eth_off_count <= 5)//离线时间大于5个周期
		{
			if (stream->wifi_state != DF_SWITCH_CM_PORT_RED_ON_GREN_OFF)
			{	//支持wifi,未连接
				out("1支持wifi,未连接亮红灯\n");
				ty_ctl(stream->switch_dev, DF_SWITCH_CM_PORT_RED_ON_GREN_OFF, DF_WIFI_LED_SAME_USBPORT);
				ty_ctl(stream->switch_dev, DF_SWITCH_CM_PORT_RED_ON_GREN_OFF, DF_WIFI_LED_SAME_USBPORT);
				stream->wifi_state = DF_SWITCH_CM_PORT_RED_ON_GREN_OFF;
			}
		}
		if (stream->eth_off_count > 5)//离线时间大于5个周期
		{
			if (stream->wifi_state != DF_SWITCH_CM_PORT_RED_OFF_GREN_TWI)
			{	//支持wifi,正在连接
				//out("1支持wifi,正在连接绿灯闪\n");
				ty_ctl(stream->switch_dev, DF_SWITCH_CM_PORT_RED_OFF_GREN_TWI, DF_WIFI_LED_SAME_USBPORT);
				stream->wifi_state = DF_SWITCH_CM_PORT_RED_OFF_GREN_TWI;
			}
		

			char r_buf[4096] = { 0 };
			mb_system_read("ls -l /etc/wifi_config/wifi_*.sh", r_buf, sizeof(r_buf), "/tmp/ls_l_etc_wifi_config.txt");

			if (strstr(r_buf, "No such file or directory") != NULL)
			{
				out("get_configured_wifi_list wifi搜索可用列表失败\n");
			}
			else
			{			
				out("有线网络连接异常切换至wifi模式\n");
				_mqtt_client_disconnect();
				//切换至无线网络
				system("ifconfig wlan0 up");


				if (strlen(stream->now_ssid) == 0)
				{
					struct _net_define	net;
					set_net("wlan0", &net);//设置无线网络
					stream->wifi_off_count = 0;
				}
				else
				{
					struct _net_define	net;
					result = set_net(stream->now_ssid, &net);//设置无线网络
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
						out("当前wifi不可用,删除连接\n");
						
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
			if (stream->net_state != 10)//网线离线
			{
				out("获取到的网络状态为离线0\n");
				ty_ctl(stream->switch_dev, DF_SWITCH_CM_NO_NET);
				lcd_print_info.mqtt_state = 0;
				lcd_print_info.udp_state = 0;
				lcd_print_info.eth_state = 0;
				memset(&lcd_print_info.local_ip, 0, sizeof(lcd_print_info.local_ip));
				memcpy(&lcd_print_info.local_ip, stream->net_status.ip_address, strlen(stream->net_status.ip_address));
				ty_ctl(stream->lcd_fd, DF_LCD_STATE_UPDATE_STATE);
				stream->net_state = 10;//离线状态
				logout(INFO, "SYSTEM", "网络检测", "检测到网线连接异常\r\n");
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
				out("当前连接的wifi与要设置的wifi不一致,需要更换\n");
				_mqtt_client_disconnect();
				system("killall wpa_supplicant");

				system("ifconfig wlan0 down");
				sleep(1);
				system("ifconfig wlan0 up");
				delete_file(DF_LINUX_DNS);

				return;//此处直接返回，目的能快速切换连接;

			}

		}


		if (stream->wifi_state != DF_SWITCH_CM_PORT_RED_OFF_GREN_ON)
		{	//支持wifi,已连接
			out("1支持wifi,连接成功,亮绿灯\n");
			ty_ctl(stream->switch_dev, DF_SWITCH_CM_PORT_RED_OFF_GREN_ON, DF_WIFI_LED_SAME_USBPORT);
			stream->wifi_state = DF_SWITCH_CM_PORT_RED_OFF_GREN_ON;
		}
		stream->net_mode = 1;
		check_net_internet(stream);
		if (atoi(stream->net_status.internet) == 0)
		{
			if (stream->net_state != 20)
			{
				//out("获取到的网络状态为在线1\n");
				ty_ctl(stream->switch_dev, DF_SWITCH_CM_GET_NET_OK);
				lcd_print_info.eth_state = 1;
				lcd_print_info.mqtt_state = 0;
				lcd_print_info.udp_state = 0;
				memset(&lcd_print_info.local_ip, 0, sizeof(lcd_print_info.local_ip));
				memcpy(&lcd_print_info.local_ip, stream->net_status.ip_address, strlen(stream->net_status.ip_address));
				ty_ctl(stream->lcd_fd, DF_LCD_STATE_UPDATE_STATE);
				stream->net_state = 20;//在线状态
				logout(INFO, "SYSTEM", "网络检测", "检测到使用外网异常\r\n");
			}
		}
		else
		{
			if (stream->net_state != 30)
			{
				ty_ctl(stream->switch_dev, DF_SWITCH_CM_GET_NET_OK);
				lcd_print_info.mqtt_state = 1;
				lcd_print_info.udp_state = 1;
				lcd_print_info.eth_state = 2;
				memset(&lcd_print_info.local_ip, 0, sizeof(lcd_print_info.local_ip));
				memcpy(&lcd_print_info.local_ip, stream->net_status.ip_address, strlen(stream->net_status.ip_address));
				ty_ctl(stream->lcd_fd, DF_LCD_STATE_UPDATE_STATE);
				stream->net_state = 30;//在线状态
				logout(INFO, "SYSTEM", "网络检测", "检测到使用外网正常\r\n");
			}
		}
	}
	else if ((eth0_ok == 1 && wlan0_ok < 0) || (eth0_ok == 1 && wlan0_ok == 0))
	{
		if (wlan0_ok < 0)
		{
			if (stream->wifi_state != DF_SWITCH_CM_PORT_RED_OFF_GREN_OFF)
			{	//不支持wifi,灯全灭
				out("3不支持wifi,灯全灭\n");
				ty_ctl(stream->switch_dev, DF_SWITCH_CM_PORT_RED_OFF_GREN_OFF, DF_WIFI_LED_SAME_USBPORT);
				stream->wifi_state = DF_SWITCH_CM_PORT_RED_OFF_GREN_OFF;
			}
		}
		else
		{
			if (stream->wifi_state != DF_SWITCH_CM_PORT_RED_ON_GREN_OFF)
			{	//支持wifi,未连接
				out("2支持wifi,未连接亮红灯\n");
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
				//out("获取到的网络状态为在线1\n");
				ty_ctl(stream->switch_dev, DF_SWITCH_CM_GET_NET_OK);
				lcd_print_info.eth_state = 1;
				lcd_print_info.mqtt_state = 0;
				lcd_print_info.udp_state = 0;
				memset(&lcd_print_info.local_ip, 0, sizeof(lcd_print_info.local_ip));
				memcpy(&lcd_print_info.local_ip, stream->net_status.ip_address, strlen(stream->net_status.ip_address));
				ty_ctl(stream->lcd_fd, DF_LCD_STATE_UPDATE_STATE);
				stream->net_state = 20;//在线状态
				logout(INFO, "SYSTEM", "网络检测", "检测到使用外网异常\r\n");
			}
		}
		else
		{
			if (stream->net_state != 30)
			{
				ty_ctl(stream->switch_dev, DF_SWITCH_CM_GET_NET_OK);
				lcd_print_info.mqtt_state = 1;
				lcd_print_info.udp_state = 1;
				lcd_print_info.eth_state = 2;
				memset(&lcd_print_info.local_ip, 0, sizeof(lcd_print_info.local_ip));
				memcpy(&lcd_print_info.local_ip, stream->net_status.ip_address, strlen(stream->net_status.ip_address));
				ty_ctl(stream->lcd_fd, DF_LCD_STATE_UPDATE_STATE);
				stream->net_state = 30;//在线状态
				logout(INFO, "SYSTEM", "网络检测", "检测到使用外网正常\r\n");
			}
		}
	}
	else if ((eth0_ok == 1) && (wlan0_ok == 1))
	{
		out("有线网络正常情况需关闭无线网络服务\n");
		system("killall wpa_supplicant");
		system("killall udhcpc -i wlan0");
		system("killall udhcpc -i eth0");
		system("ifconfig wlan0 down");
		delete_file(DF_LINUX_DNS);
		ty_ctl(stream->deploy, DF_DEPLOY_CM_SET_IP_UPDATE);
	}
	else
	{
		out("获取网络状态错误\n");
	}	

	if ((stream->net_state == 30) && (stream->internet_info_flag == 0) && (stream->internet_info_err_num < 10))
	{
		out("未获取公网信息,此处去获取公网信息\n");

		result = get_internet_infomation(stream);
		if (result < 0)
		{
			out("获取公网信息失败\n");
			stream->internet_info_err_num += 1;
		}
		else
		{
			stream->internet_info_flag = 1;
		}		
	}

	char time_now[20] = { 0 };
	//out("检测时间是否为初始时间\n");
	timer_read_y_m_d_h_m_s(time_now);
	//out("获取到的系统时间为%s\n", time_now);

	if (strcmp(time_now, SYSTEM_LEAST_TIME) > 0)
	{
		//out("对时成功\n");
		ty_ctl(stream->switch_dev, DF_SWITCH_CM_HEART);//对时成功后同步下时间，lcd显示
		stream->net_status.nettime_ok = 1;
	}
	else
	{
		//out("对时未成功\n");
		stream->net_status.nettime_ok = 0;
	}
	
	sleep(2);
	return ;
}
#endif
static int get_internet_infomation(struct _get_net_state *stream)
{
	char tmp[100] = {0};
	int result;
	char *returndata = NULL;
	int returndata_len;
	char inter_info[5000] = {0};
	//out("向服务器请求公网信息\n");
	//result = socket_http_netPost("www.njmbxx.com", 20020, "/was.tersvr/device_location_for_ip", "application/x-www-form-urlencoded", tmp, 5, &returndata, &returndata_len);
	result = socket_http_netPost("ip.njmbxx.com", 20020, "/ping", "application/x-www-form-urlencoded", tmp, 5, &returndata, &returndata_len);
	//out("向服务器请求公网信息完成\n");
	if (result < 0)
	{
		if (returndata != NULL)
		{
			//out("释放http接收数据内存\n");
			free(returndata);
			returndata = NULL;
			//out("释放内存成功,指向空\n");
		}
		out("获取公网IP信息失败\n");
		return -1;
	}
	else
	{
		if (strlen(returndata) > sizeof(inter_info))
		{
			out("获取公网信息得到的数据长度存在异常\n");
			free(returndata);
			returndata = NULL;
			return -2;
		}
		//out("将公网信息的UTF8数据转换成GBK数据\n");
		utf82gbk((uint8 *)returndata, (uint8 *)inter_info);
		free(returndata);
		returndata = NULL;
		//out("获取公网IP信息成功%s\n", inter_info);
		result = analysis_json_internet_info(stream, inter_info);
		if (result < 0)
		{
			out("解析公网JOSN数据失败,result = %d\n",result);

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
		out("Error before: [%s]\n", cJSON_GetErrorPtr());
		return -1;
	}
	

	//out("获取查询结果信息\n");
	Jsp_Tmp = cJSON_GetObjectItem(root, "result");
	if (get_json_value_can_not_null(Jsp_Tmp, result_code, 0, 3) < 0) {
		out("获取查询结果信息 failed\n");
		cJSON_Delete(root);
		return -1;
	}
	if (strcmp(result_code, "s") != 0)
	{
		out("查询公网信息result != s\n");
		cJSON_Delete(root);
		return -2;
	}

	//out("获取服务器时间\n");
	Jsp_Tmp = cJSON_GetObjectItem(root, "svrtime");
	if (get_json_value_can_not_null(Jsp_Tmp, svrtime, 0, 20) < 0) {
		out("获取服务器时间 failed\n");
		cJSON_Delete(root);
		return -1;
	}
	out("服务器时间为%s\n",svrtime);


	long time_sec = get_second_from_date_time_turn((char *)svrtime);
	time_sec = time_sec - 8 * 3600;
	char settime[20] = { 0 };
	get_date_time_from_second_turn(time_sec, settime);

	logout(INFO, "system", "网络检测", "获取网络对时成功：%s\r\n", svrtime);

	char cmd[200] = { 0 };
	sprintf(cmd, "date -s \"%s\" -u", settime);
	out("时间设置命令：%s\n",cmd);
	system(cmd);
	//system("hwclock -w");
	
	cJSON *object = cJSON_GetObjectItem(root, "response");
	if (object == NULL)
	{
		out("Error before: [%s]\n", cJSON_GetErrorPtr());
		cJSON_Delete(root);
		return -3;
	}
	
	///////////////////////////////////////////////////////////////////
	Jsp_Tmp = cJSON_GetObjectItem(object, "internet_ip");
	if (get_json_value_can_not_null(Jsp_Tmp, internet_ip, 0, 20) < 0) {
		out("获取查询结果信息 failed\n");
		cJSON_Delete(root);
		return -4;
	}
	if (strlen(internet_ip) < 7)
	{
		out("获取到的公网IP长度错误,ip %s\n", internet_ip);
		cJSON_Delete(root);
		return -5;
	}

	write_file("/etc/ip_tmp.txt", internet_ip, strlen(internet_ip));

	// ///////////////////////////////////////////////////////////////////
	// Jsp_Tmp = cJSON_GetObjectItem(object, "longitude");
	// if (get_json_value_can_not_null(Jsp_Tmp, longitude, 0, 20) < 0) {
	// 	out("获取精度信息 failed\n");
	// 	cJSON_Delete(root);
	// 	return -6;
	// }
	// if (strlen(longitude) < 7)
	// {
	// 	out("获取精度信息长度错误,ip %s\n", longitude);
	// 	cJSON_Delete(root);
	// 	return -7;
	// }
	// ///////////////////////////////////////////////////////////////////
	// Jsp_Tmp = cJSON_GetObjectItem(object, "latitude");
	// if (get_json_value_can_not_null(Jsp_Tmp, latitude, 0, 20) < 0) {
	// 	out("获取纬度信息 failed\n");
	// 	cJSON_Delete(root);
	// 	return -8;
	// }
	// if (strlen(latitude) < 7)
	// {
	// 	out("获取纬度信息长度错误,ip %s\n", latitude);
	// 	cJSON_Delete(root);
	// 	return -9;
	// }

	///////////////////////////////////////////////////////////////////
	Jsp_Tmp = cJSON_GetObjectItem(object, "area_info");
	if (get_json_value_can_not_null(Jsp_Tmp, area_info, 0, 200) < 0) {
		out("获取区域信息 failed\n");
		cJSON_Delete(root);
		return -10;
	}
	if (strlen(area_info) < 7)
	{
		out("获取区域信息长度错误,ip %s\n", area_info);
		cJSON_Delete(root);
		return -11;
	}

	logout(INFO, "SYSTEM", "网络检测", "公网IP为%s,区域信息为%s\r\n", internet_ip, area_info);
	
	if (strcmp(internet_ip, stream->net_status.internet_ip) != 0)
	{
		memset(stream->net_status.internet_ip, 0, sizeof(stream->net_status.internet_ip));
		memcpy(stream->net_status.internet_ip, internet_ip, strlen(internet_ip));
	}
	if (strcmp(longitude, stream->net_status.longitude) != 0)
	{
		memset(stream->net_status.longitude, 0, sizeof(stream->net_status.longitude));
		memcpy(stream->net_status.longitude, longitude, strlen(longitude));
	}
	if (strcmp(latitude, stream->net_status.latitude) != 0)
	{
		memset(stream->net_status.latitude, 0, sizeof(stream->net_status.latitude));
		memcpy(stream->net_status.latitude, latitude, strlen(latitude));
	}
	if (strcmp(area_info, stream->net_status.area_info) != 0)
	{
		memset(stream->net_status.area_info, 0, sizeof(stream->net_status.area_info));
		memcpy(stream->net_status.area_info, area_info, strlen(area_info));
	}
	//stream->net_status.internet_ok = 1;
	cJSON_Delete(root);
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
	out("清除所有状态\n");
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
	out("设置wifi切换,要设置的wifi为:%s,当前连接的wifi为：%s\n", stream->now_ssid, stream->connect_ssid);
	return 0;
}