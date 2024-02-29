#include "../inc/bluetooth.h"

static int bluetooth_open(struct ty_file *file);
static int bluetooth_ctl(void *data,int fd,int cm,va_list args);
static int bluetooth_close(void *data,int fd);
static int set_bluetooth_name(struct _bluetooth_fd *id,va_list args);
static int ble_2at_disconnect(struct _bluetooth *stream);
static void bluetooth_order_task(void *arg,int timer);
static void bluetooth_detect_task(void *arg,int timer);
static int ble_test_conn(struct _bluetooth *stream,uint8 *json_in,struct json_parm_bluetooth *parm);
static int get_all_ports_status(struct _bluetooth *stream,uint8 *json_in,struct json_parm_bluetooth *parm);
static int get_all_ports_ad(struct _bluetooth *stream,uint8 *json_in,struct json_parm_bluetooth *parm);
static int get_key_status(struct _bluetooth *stream,uint8 *json_in,struct json_parm_bluetooth *parm);
static int set_ip_by_bluetooth(struct _bluetooth *stream,uint8 *json_in,struct json_parm_bluetooth *parm);
static int get_terminal_info_by_bluetooth(struct _bluetooth *stream,uint8 *json_in,struct json_parm_bluetooth *parm);
static int scanning_wifi_list(struct _bluetooth *stream,uint8 *json_in,struct json_parm_bluetooth *parm);
static int set_wifi_config(struct _bluetooth *stream,uint8 *json_in,struct json_parm_bluetooth *parm);
static int get_configured_wifi_list(struct _bluetooth *stream,uint8 *json_in,struct json_parm_bluetooth *parm);
static int delete_configured_wifi_list(struct _bluetooth *stream,uint8 *json_in,struct json_parm_bluetooth *parm);
static int ble_up_all_power(struct _bluetooth *stream,uint8 *json_in,struct json_parm_bluetooth *parm);
static int ble_down_all_power(struct _bluetooth *stream,uint8 *json_in,struct json_parm_bluetooth *parm);
static int get_hub_mcu_status(struct _bluetooth *stream,uint8 *json_in,struct json_parm_bluetooth *parm);
static int reg_ter_id(struct _bluetooth *stream,uint8 *json_in,struct json_parm_bluetooth *parm);
static int control_dev_restore(struct _bluetooth *stream,uint8 *json_in,struct json_parm_bluetooth *parm);
static int control_ble_disconn(struct _bluetooth *stream,uint8 *json_in,struct json_parm_bluetooth *parm);
static int bluetooth_send_recv(struct _bluetooth *stream,char *sbuf,char *rbuf,int rbuf_len);
static int comport_send(struct _bluetooth *stream,char *sbuf,int sbuf_len);
static int bluetooth_send_packets(struct _bluetooth *stream,char *sbuf,int len,char *encryption);

static int upgrade_by_bluetooth(struct _bluetooth *stream, uint8 *json_in, struct json_parm_bluetooth *parm);
static int download_by_bluetooth(struct _bluetooth *stream, uint8 *json_in, struct json_parm_bluetooth *parm);
static int get_upgarde_status_by_bluetooth(struct _bluetooth *stream, uint8 *json_in, struct json_parm_bluetooth *parm);

static const struct _bluetooth_ctl_fu ctl_fun[] =
{
	{DF_CORE_CM_SET_BLE_NAME ,set_bluetooth_name},
};

static const struct _file_fuc bluetooth_fuc =
{
	.open = bluetooth_open,
	.read = NULL,
	.write = NULL,
	.ctl = bluetooth_ctl,
	.close = bluetooth_close,
	.del = NULL
};

static const struct _bluetooth_errinfo ble_err[] =
{
	{ BLUETOOTH_ERR_SYSTEM,					"System Err"						},
	{ BLUETOOTH_ERR_SEND,					"Send Err"							},
	{ BLUETOOTH_ERR_RECV,					"Recv Err"							},
	{ BLUETOOTH_ERR_PARM,					"Parm Jason Err"					},
	{ BLUETOOTH_ERR_HEAD ,					"Parm Head Err"						},
	{ BLUETOOTH_ERR_WIFI_SET_LAN_ERR,		"Wired network is not turned off"	},
	{ BLUETOOTH_ERR_WIFI_SET_PASSWD_ERR,	"Wifi passwd error"					},
	{ BLUETOOTH_ERR_WIFI_SCAN_WIFI_ERR,		"WiFi device list search failed"	},
	{ BLUETOOTH_ERR_WIFI_NO_CONGIGUED_INFO, "No wifi local configuration"		}
};

static const struct _bluetooth_order bluetooth_orders[] =
{
	{ "test_conn",				"[动作类]：测试联机",				ble_test_conn					},
	{ "get_terinfo",			"[查询类]:查询机柜全部信息",		get_terminal_info_by_bluetooth	},
	{ "get_all_ports_status",	"[查询类]:获取机柜所有端口状态",	get_all_ports_status			},
	{ "get_all_ports_ad",		"[查询类]:获取机柜所有端口AD值",	get_all_ports_ad				},
	{ "get_key_status",			"[查询类]:获取机柜密钥状态",		get_key_status					},
	{ "get_hub_mcu_status",		"[查询类]:获取机柜hub及单片机状态", get_hub_mcu_status				},
	{ "set_ip",					"[设置类]:设置IP地址",				set_ip_by_bluetooth},
	{ "dev_restore",			"[动作类]:回出厂",					control_dev_restore},
	{ "ble_disconn",			"[动作类]:断开蓝牙",				control_ble_disconn},
	{ "reg_terminal",			"[动作类]:机器编号注册",			reg_ter_id						},
	{ "down_all_power",			"[动作类]:下电所有端口",			ble_down_all_power				},
	{ "up_all_power",			"[动作类]:上电所有端口",			ble_up_all_power				},
	{ "scan_wifi",				"[查询类]:搜索WLAN网络",			scanning_wifi_list},
	{ "get_wifi",				"[查询类]:查询当前已配置wifi",		get_configured_wifi_list },
	{ "set_wifi",				"[设置类]:设置wifi配置",			set_wifi_config },
	{ "del_wifi",				"[动作类]:删除已配置wifi信息",		delete_configured_wifi_list },
	{ "upgrade",				"[动作类]:通知升级",		upgrade_by_bluetooth },
	{ "download",				"[动作类]:下载升级包",		download_by_bluetooth },
	{ "get_upgarde_status",		"[动作类]:获取升级状态",		get_upgarde_status_by_bluetooth }
};

int bluetooth_add(char *seria_name,const _so_note *note,const char *switch_name)
{
	struct _bluetooth *stream = NULL;
	int result = 0;
	stream = malloc(sizeof(struct _bluetooth));
	if (stream == NULL)
		return DF_ERR_MEM_ERR;
	memset(stream,0,sizeof(struct _bluetooth));
	memcpy(stream->switch_name,switch_name,strlen(switch_name));
	stream->note = (_so_note *)note;
	result = ty_file_add(DF_DEV_BLUETOOTH,stream,"bluetooth",(struct _file_fuc *)&bluetooth_fuc);
	if(result<0)
		free(stream);
	return result;
}

static int bluetooth_open(struct ty_file *file)
{
	bluetooth_out("enter bluetooth open\n");
	int j = 0;
	struct _bluetooth *stream = NULL;
	uint8 mode_ble[2] = { 0 };
	mode_ble[0] = 0x00;
	struct _switch_dev_id id;
	char machine_id[16] = { 0 };
	stream = file->pro_data;
	if((stream == NULL))
		return DF_ERR_PORT_CORE_FD_NULL;
	bluetooth_out("enter bluetooth open	1111\n");
	if(stream->state == 0)
	{
		stream->machine_fd = ty_open("/dev/machine",0,0,NULL);
		if(stream->machine_fd < 0)
		{
			bluetooth_out("bluetooth open machine_fd failed\n");
			return -6;
		}
		stream->deploy_fd = ty_open("/dev/deploy",0,0,NULL);
		if (stream->deploy_fd < 0)
		{
			bluetooth_out("bluetooth 打开deploy失败\n");
			return -1;
		}
		stream->ty_usb_fd = ty_open("/dev/ty_usb",0,0,NULL);
		if (stream->ty_usb_fd < 0)
		{
			bluetooth_out("bluetooth打开ty_usb文件失败\n");
			return stream->ty_usb_fd;
		}
		stream->event_file_fd = ty_open("/dev/event",0,0,NULL);
		// stream->ty_usb_m_fd = ty_open("/dev/ty_usb_m",0,0,NULL);
		// if (stream->ty_usb_m_fd < 0)
		// {
		// 	bluetooth_out("bluetooth打开ty_usb_m_fd文件失败\n");
		// 	return stream->ty_usb_m_fd;
		// }
		stream->get_net_fd = ty_open("/dev/get_net_state",0,0,NULL);
		if (stream->get_net_fd < 0)
		{
			bluetooth_out("bluetooth打开get_net_state文件失败\n");
			return -1;
		}
		stream->switch_fd = ty_open(stream->switch_name,0,0,NULL);
		if (stream->switch_fd < 0)
		{
			bluetooth_out("bluetooth打开switch_fd文件失败\n");
			return -1;
		}
		ty_ctl(stream->switch_fd,DF_SWITCH_CM_GET_SUPPORT,&stream->dev_support);
		if (stream->dev_support.support_ble == 0)//不支持蓝牙
		{
			bluetooth_out("lbc stream->dev_support.support_ble == 0\n");
			return 0;
		}
		if (ty_ctl(stream->ty_usb_fd,DF_TY_USB_CM_BLUETOOTH_POWER,"\x00") < 0)
		{
			bluetooth_out("关闭蓝牙电源失败\n");
		}
		sleep(1);
		if (ty_ctl(stream->ty_usb_fd,DF_TY_USB_CM_BLUETOOTH_POWER,"\x01") < 0)
		{
			bluetooth_out("打开蓝牙电源失败\n");
		}
		if (ty_ctl(stream->ty_usb_fd,DF_TY_USB_CM_CTL_BLUETOOTH_MODE,mode_ble) < 0)
		{
			bluetooth_out("设置蓝牙AT模式失败\n");
		}
		memset(&id, 0, sizeof(id));
		ty_ctl(stream->machine_fd,DF_MACHINE_INFOR_CM_GET_ID,&id);
		bcd2asc(id.id,(uint8 *)machine_id,6);
		memcpy(stream->machine_id,machine_id,12);
		// if (memcmp(machine_id, "068", 3) != 0)
		// {
		// 	while (get_module_state() < 0)//等待ty_usb_m模块完成
		// 	{
		// 		sleep(1);
		// 	}
		// }
		stream->serial_fd = ty_open("/dev/seria1",0,0,NULL);
		if (stream->serial_fd < 0)
		{
			bluetooth_out("打开串口[%s]错误\n", "/dev/seria1");
			return stream->serial_fd;
		}
		bluetooth_out("设置串口波特率[%d]\n",115200);
		ty_ctl(stream->serial_fd,DF_SERIA_CM_BPS,115200);
		_scheduled_task_open("bluetooth_task",bluetooth_detect_task,stream,300,NULL);
		_scheduled_task_open("bluetooth_task",bluetooth_order_task,stream,100,NULL);
	}
	for(j = 0;j<sizeof(stream->fd)/sizeof(stream->fd[0]);j++)
	{
		if(stream->fd[j].state == 0)
			break;
	}
	if(j == sizeof(stream->fd)/sizeof(stream->fd[0]))
	{
		return DF_ERR_PORT_CORE_SLAVE_SPACE;
	}
	memset(&stream->fd[j],0,sizeof(stream->fd[0]));
	stream->fd[j].dev = stream;
	stream->fd[j].state = 1;
	stream->fd[j].mem_fd = &file->fd[j];
	stream->state++;
	//升级结构体初始化
	stream->upgrade.upgarde_status = NO_UPGRADE;
	stream->upgrade.file_data = NULL;
	stream->upgrade.file_data_size = 0;

	bluetooth_out("打开bluetooth成功:j=%d\n",j);
	return j + 1;
}

static int bluetooth_ctl(void *data,int fd,int cm,va_list args)
{
	struct _bluetooth *stream = NULL;
	struct _bluetooth_fd *id = NULL;
	int i = 0;
	stream = data;
	if ((stream == NULL))
		return DF_ERR_PORT_CORE_FD_NULL;
	if ((fd == 0) || (fd>sizeof(stream->fd)/sizeof(stream->fd[0])))
		return DF_ERR_PORT_CORE_FD_Z;
	fd--;
	id = &stream->fd[fd];
	if (id->state == 0)
		return DF_ERR_PORT_CORE_SLAVE_DEV;
	for (i = 0;i<sizeof(ctl_fun)/sizeof(ctl_fun[0]);i++)
	{
		if (cm == ctl_fun[i].cm)
			return ctl_fun[i].ctl(id, args);
	}
	return DF_ERR_PORT_CORE_CM;
}

static int bluetooth_close(void *data,int fd)
{
	return 0;
}

/*扫描wifi列表*/
//wpa_passphrase  234 12345678 | tee -a /etc/wpa_supplicant.conf  往配置文件中添加wifi配置
//wpa_supplicant -iwlan0 -Dnl80211 -c /etc/wpa_supplicant.conf & 开启wifi
static int scanning_wifi_list(struct _bluetooth *stream,uint8 *json_in,struct json_parm_bluetooth *parm)
{
	char *g_buf = NULL;
	char sendbuf[1024 * 5] = {0};
	char r_buf[8192] = {0};
	char wifi_ssid[64] = {0};
	char signal_level[32] = {0};
	signed int level = 0;
	char *p_index = NULL,*p_start = NULL,*p_end = NULL;
	system("ifconfig wlan0 up");
	mb_system_read("iwlist wlan0 scan | grep \"ESSID\\|Quality\"",r_buf,sizeof(r_buf),"/tmp/iwlist_result.txt");
	bluetooth_out("iwlist : %s\n",r_buf);
	cJSON *dir1 = NULL,*dir2 = NULL,*dir3 = NULL;		//组包使用
	cJSON *json = cJSON_CreateObject();
	cJSON_AddItemToObject(json,"head",dir1 = cJSON_CreateObject());
	cJSON_AddStringToObject(dir1,"protocol",BLUETOOTH_PROTOCOL);
	cJSON_AddStringToObject(dir1,"code_type",BLUETOOTH_SYNC_CODE_RESPONSE);
	cJSON_AddStringToObject(dir1,"cmd",parm->cmd);
	cJSON_AddStringToObject(dir1,"source_topic",parm->source_topic);
	cJSON_AddStringToObject(dir1,"random",parm->random);
	cJSON_AddStringToObject(dir1,"result","s");
	cJSON_AddItemToObject(json,"data",dir2 = cJSON_CreateArray());

	p_index = strstr(r_buf,"Signal level=");
	if(p_index == NULL)
	{
		bluetooth_out("scanning_wifi_list wifi搜索可用列表失败\n");
		return BLUETOOTH_ERR_WIFI_SCAN_WIFI_ERR;
	}
	for(;;)
	{
		p_start = strstr(p_index,"Signal level=");
		if(p_start == NULL)
			break;
		p_start += strlen("Signal level=");
		p_end = strstr(p_start," dBm");
		if(p_end == NULL)
			break;
		p_index = p_end;
		memset(signal_level, 0, sizeof(signal_level));
		memcpy(signal_level, p_start, p_end - p_start);
		level = atoi(signal_level);
		if (level < -70)
		{
			continue;
		}
		p_start = strstr(p_index, "ESSID:\"");
		if (p_start == NULL)
			break;
		p_start += strlen("ESSID:\"");
		p_end = strstr(p_start, "\"");
		if (p_end == NULL)
			break;
		p_index = p_end;
		memset(wifi_ssid, 0, sizeof(wifi_ssid));
		memcpy(wifi_ssid, p_start, p_end - p_start);
		if (strlen(wifi_ssid) == 0)
		{
			bluetooth_out("ssid为空,直接过滤\n");
			continue;
		}
		cJSON *dir3;
		cJSON_AddItemToObject(dir2, "dira", dir3 = cJSON_CreateObject());
		cJSON_AddStringToObject(dir3, "ssid", wifi_ssid);
		cJSON_AddStringToObject(dir3, "signal_level", signal_level);
		continue;
	}
	g_buf = cJSON_PrintUnformatted(json);
	//bluetooth_out("返回信息：%s\n",g_buf);
	//printf("%s\n", g_buf);
	memcpy(sendbuf, "0001", 4);//协议头
	sprintf(sendbuf + 4, "%06d", strlen(g_buf));
	memcpy(sendbuf + 10, "00000001000100000000", 20);
	memcpy(sendbuf + 30, g_buf, strlen(g_buf));
	comport_send(stream, sendbuf, 30 + strlen(g_buf));
	free(g_buf);
	cJSON_Delete(json);
	return 0;
}

static int set_wifi_config(struct _bluetooth *stream, uint8 *json_in, struct json_parm_bluetooth *parm)
{
	cJSON *root = NULL;
	cJSON *array_obj = NULL;
	cJSON *arr_item = NULL, *item = NULL;
	char *g_buf = NULL;
	char sendbuf[1024 * 5] = { 0 };
	char wifi_name[200] = { 0 };
	char wifi_pwd[50] = { 0 };
	uint8 net_type;
	char ip[20] = {0};
	char netmask[20] = { 0 };
	char gateway[20] = { 0 };
	char dns[20] = { 0 };
	char resolv[1024] = { 0 };
	char resolv_path[200] = { 0 };
	char s_buf[2048] = { 0 };
	bluetooth_out("设置WIFIjson数据：%s\n", json_in);
	root = cJSON_Parse((char*)json_in);
	if (root == NULL)
	{
		return BLUETOOTH_ERR_PARM;
	}
	array_obj = cJSON_GetObjectItem(root, "data");
	if (array_obj == NULL)
	{
		cJSON_Delete(root);
		return BLUETOOTH_ERR_PARM;
	}
	arr_item = cJSON_GetArrayItem(array_obj, 0);
	if (arr_item == NULL)
	{
		cJSON_Delete(root);
		return BLUETOOTH_ERR_PARM;
	}
	item = cJSON_GetObjectItem(arr_item, "ssid");
	if (item == NULL)
	{
		cJSON_Delete(root);
		return BLUETOOTH_ERR_PARM;
	}
	memcpy(wifi_name, item->valuestring, strlen(item->valuestring));
	printf("utf8:wifi ssid:%s\n", wifi_name);
	if (strlen(wifi_name) > 32)
	{
		bluetooth_out("SSID长度有误\n");
		cJSON_Delete(root);
		return BLUETOOTH_ERR_PARM;
	}
	item = cJSON_GetObjectItem(arr_item, "passwd");
	if (item == NULL)
	{
		cJSON_Delete(root);
		return BLUETOOTH_ERR_PARM;
	}
	memcpy(wifi_pwd, item->valuestring, strlen(item->valuestring));
	if (strlen(wifi_pwd) > 64)
	{
		bluetooth_out("wifi长度有误\n");
		cJSON_Delete(root);
		return BLUETOOTH_ERR_PARM;
	}
	bluetooth_out("wifi passwd:%s\n", wifi_pwd);
	item = cJSON_GetObjectItem(arr_item, "net_type");
	if (item == NULL)
	{
		cJSON_Delete(root);
		return BLUETOOTH_ERR_PARM;
	}
	net_type = atoi(item->valuestring);

	if (net_type == 2)
	{
		//设置为固定IP模式
		item = cJSON_GetObjectItem(arr_item, "ip");
		if (item == NULL)
		{
			cJSON_Delete(root);
			return BLUETOOTH_ERR_PARM;
		}
		memcpy(ip, item->valuestring, strlen(item->valuestring));
		cJSON_Delete(root);
		if (strlen(ip) > 17)
		{
			bluetooth_out("IP长度有误\n");
			cJSON_Delete(root);
			return BLUETOOTH_ERR_PARM;
		}
		item = cJSON_GetObjectItem(arr_item, "netmask");
		if (item == NULL)
		{
			cJSON_Delete(root);
			return BLUETOOTH_ERR_PARM;
		}
		memcpy(netmask, item->valuestring, strlen(item->valuestring));
		cJSON_Delete(root);
		if (strlen(netmask) > 17)
		{
			bluetooth_out("netmask长度有误\n");
			cJSON_Delete(root);
			return BLUETOOTH_ERR_PARM;
		}
		item = cJSON_GetObjectItem(arr_item, "gateway");
		if (item == NULL)
		{
			cJSON_Delete(root);
			return BLUETOOTH_ERR_PARM;
		}
		memcpy(gateway, item->valuestring, strlen(item->valuestring));
		if (strlen(gateway) > 17)
		{
			bluetooth_out("gateway长度有误\n");
			cJSON_Delete(root);
			return BLUETOOTH_ERR_PARM;
		}

		item = cJSON_GetObjectItem(arr_item, "dns");
		if (item == NULL)
		{
			cJSON_Delete(root);
			return BLUETOOTH_ERR_PARM;
		}
		memcpy(dns, item->valuestring, strlen(item->valuestring));
		if (strlen(dns) > 17)
		{
			bluetooth_out("dns长度有误\n");
			cJSON_Delete(root);
			return BLUETOOTH_ERR_PARM;
		}
		cJSON_Delete(root);
	}
	else
	{
		//设置为DHCP模式
		cJSON_Delete(root);
	}
	
	if (file_exists("/etc/wifi_config") != 0)
	{
		system("mkdir /etc/wifi_config");
	}
	sprintf(s_buf,"#!/bin/sh\n");
	sprintf(s_buf + strlen(s_buf), "rm -rf /var/run/wpa_supplicant\n");
	sprintf(s_buf + strlen(s_buf), "rm -rf /etc/wpa_supplicant.conf\n");
	sprintf(s_buf + strlen(s_buf), "killall wpa_supplicant\n");
	sprintf(s_buf + strlen(s_buf), "killall udhcpc\n");
	sprintf(s_buf + strlen(s_buf), "wpa_passphrase  %s %s | tee -a /etc/wpa_supplicant.conf\n", wifi_name, wifi_pwd);	
	sprintf(s_buf + strlen(s_buf), "wpa_supplicant -iwlan0 -Dnl80211 -c /etc/wpa_supplicant.conf &\n");
	sprintf(s_buf + strlen(s_buf), "sleep 5\n");
	if (net_type == 2)
	{
		sprintf(s_buf + strlen(s_buf), "ifconfig wlan0 %s netmask %s\n",ip,netmask);
		sprintf(s_buf + strlen(s_buf), "rbluetooth_oute add default gw %s\n", gateway);
		sprintf(resolv, "nameserver %s", gateway);
		sprintf(resolv + strlen(resolv), "nameserver %s", "114.114.114.114");
		sprintf(resolv_path, "/etc/wifi_config/resolv.conf_%s", wifi_name);
		write_file(resolv_path, resolv, strlen(resolv));
	}
	else
	{
		sprintf(s_buf + strlen(s_buf), "udhcpc -i wlan0 &\n");
	}

	char wifi_config_path[200] = { 0 };
	sprintf(wifi_config_path, "/etc/wifi_config/wifi_%s.sh", wifi_name);

	//bluetooth_out("gbk:wifi ssid config:%s\n", wifi_config_path);
	//printf("utf8:wifi ssid config:%s\n", wifi_config_path);


	write_file(wifi_config_path, s_buf, strlen(s_buf));

	chmod_file(wifi_config_path,"777");


	if (0)
	{
		ty_ctl(stream->get_net_fd, DF_SET_NET_WIFI_USED, 1);
		char cmd_buf[2048] = {0};
		char cmd[1024] = {0};
		system("rm -rf /var/run/wpa_supplicant");
		system("rm -rf /etc/wpa_supplicant.conf");
		system("killall wpa_supplicant");
		system("killall udhcpc");
		sprintf(cmd, "wpa_passphrase  %s %s | tee -a /etc/wpa_supplicant.conf", wifi_name, wifi_pwd);
		system(cmd);
		sleep(1);
		bluetooth_out("wifi服务执行开始\n");

		char order[200] = {0};
		char tmp_file[100] = {0};
		//int result;
		
		sprintf(tmp_file, "/tmp/system_cmd_sleep.log");
		delete_file(tmp_file);
		sprintf(order, "%s >> %s & ", "wpa_supplicant -iwlan0 -Dnl80211 -c /etc/wpa_supplicant.conf", tmp_file);

		//sprintf(order, "wpa_supplicant  -Dnl80211 -iwlan0 -f %s -c /etc/wpa_supplicant.conf -d -t -B &", tmp_file);


		printf("order:%s\n", order);
		system(order);
		sleep(1);
		int i = 0;
		for (i=0;i<30;i++)
		{
			copy_file(tmp_file,"/tmp/read_file.txt");
			read_file("/tmp/read_file.txt", cmd_buf, sizeof(cmd_buf));
			bluetooth_out("读取临时文件：%s\n", cmd_buf);
			if (strstr(cmd_buf, "CTRL-EVENT-CONNECTED") != NULL)
			{
				break;
			}
			sleep(1);
		}
		//mb_system_read_sleep("wpa_supplicant -iwlan0 -Dnl80211 -c /etc/wpa_supplicant.conf", cmd_buf, sizeof(cmd_buf),20);

		bluetooth_out("wifi服务执行返回数据：%s\n", cmd_buf);

		if (strstr(cmd_buf, "CTRL-EVENT-CONNECTED") == NULL)
		{
			ty_ctl(stream->get_net_fd, DF_SET_NET_WIFI_USED, 0);
			delete_file(wifi_config_path);
			bluetooth_out("wifi服务执行失败\n");
			return BLUETOOTH_ERR_WIFI_SET_PASSWD_ERR;
		}
		if (net_type == 2)
		{
			system("killall udhcpc");
			memset(cmd, 0, sizeof(cmd));
			sprintf(cmd, "ifconfig wlan0 %s netmask %s", ip, netmask);
			system(cmd);

			memset(cmd, 0, sizeof(cmd));
			sprintf(cmd, "rbluetooth_oute add default gw %s", gateway);
			system(cmd);

			copy_file_cmp(resolv_path, DF_LINUX_DNS);
		}
		else
		{
			system("killall udhcpc");
			system("udhcpc -i wlan0 &");
		}
		ty_ctl(stream->get_net_fd, DF_SET_NET_WIFI_USED, 0);
	}

	ty_ctl(stream->get_net_fd, DF_SET_NET_WIFI_USED, wifi_name);

	cJSON *dir1, *dir2;		//组包使用
	cJSON *json = cJSON_CreateObject();
	cJSON_AddItemToObject(json, "head", dir1 = cJSON_CreateObject());
	cJSON_AddStringToObject(dir1, "protocol", BLUETOOTH_PROTOCOL);
	cJSON_AddStringToObject(dir1, "code_type", BLUETOOTH_SYNC_CODE_RESPONSE);
	cJSON_AddStringToObject(dir1, "cmd", parm->cmd);
	cJSON_AddStringToObject(dir1, "source_topic", parm->source_topic);
	cJSON_AddStringToObject(dir1, "random", parm->random);
	cJSON_AddStringToObject(dir1, "result", "s");

	cJSON_AddItemToObject(json, "data", dir2 = cJSON_CreateArray());
	cJSON *dir3;
	cJSON_AddItemToObject(dir2, "dira", dir3 = cJSON_CreateObject());
	g_buf = cJSON_PrintUnformatted(json);
	//bluetooth_out("返回信息：%s\n", g_buf);
	printf("%s\n", g_buf);
	memcpy(sendbuf, "0001", 4);//协议头
	sprintf(sendbuf + 4, "%06d", strlen(g_buf));
	memcpy(sendbuf + 10, "00000001000100000000", 20);
	memcpy(sendbuf + 30, g_buf, strlen(g_buf));

	comport_send(stream, sendbuf, 30 + strlen(g_buf));
	free(g_buf);
	cJSON_Delete(json);
  	return 0;
}

static int get_configured_wifi_list(struct _bluetooth *stream, uint8 *json_in, struct json_parm_bluetooth *parm)
{
	char *g_buf;
	char sendbuf[1024 * 5] = { 0 };
	char now_ssid[50] = { 0 };
	DIR *dir = NULL;
	struct dirent *entry;
	int eth0_ok = get_socket_net_state("eth0");
	char wifi_ssid[50] = { 0 };

	cJSON *dir1, *dir2;		//组包使用
	cJSON *json = cJSON_CreateObject();
	cJSON_AddItemToObject(json, "head", dir1 = cJSON_CreateObject());
	cJSON_AddStringToObject(dir1, "protocol", BLUETOOTH_PROTOCOL);
	cJSON_AddStringToObject(dir1, "code_type", BLUETOOTH_SYNC_CODE_RESPONSE);
	cJSON_AddStringToObject(dir1, "cmd", parm->cmd);
	cJSON_AddStringToObject(dir1, "source_topic", parm->source_topic);
	cJSON_AddStringToObject(dir1, "random", parm->random);
	cJSON_AddStringToObject(dir1, "result", "s");

	cJSON_AddItemToObject(json, "data", dir2 = cJSON_CreateArray());

	
	find_now_wifi_ssid(now_ssid);

	if ((dir = opendir("/etc/wifi_config")) == NULL) 
	{
		bluetooth_out("打开/etc/wifi_config文件夹失败，不存在配置文件");
	}
	else 
	{
		while ((entry = readdir(dir))) 
		{
			//printf("filename = %s\n", entry->d_name);  //输出文件或者目录的名称
			if (memcmp(entry->d_name, "wifi_", 5) != 0)
			{
				continue;
			}
			memset(wifi_ssid, 0, sizeof(wifi_ssid));
			memcpy(wifi_ssid, entry->d_name + 5, strlen(entry->d_name)-8);
			cJSON *dir3;
			cJSON_AddItemToObject(dir2, "dira", dir3 = cJSON_CreateObject());
			if (eth0_ok == 1)
				cJSON_AddStringToObject(dir3, "ethernet", "1");
			else
				cJSON_AddStringToObject(dir3, "ethernet", "0");
			cJSON_AddStringToObject(dir3, "ssid", wifi_ssid);
			if ((strlen(now_ssid) == 0) || (strcmp(wifi_ssid, now_ssid) != 0))
				cJSON_AddStringToObject(dir3, "now_ssid", "0");
			else
				cJSON_AddStringToObject(dir3, "now_ssid", "1");
		}
		closedir(dir);
	}
	g_buf = cJSON_PrintUnformatted(json);
	//bluetooth_out("返回信息：%s\n", g_buf);
	printf("%s\n", g_buf);
	memcpy(sendbuf, "0001", 4);//协议头
	sprintf(sendbuf + 4, "%06d", strlen(g_buf));
	memcpy(sendbuf + 10, "00000001000100000000", 20);
	memcpy(sendbuf + 30, g_buf, strlen(g_buf));

	comport_send(stream, sendbuf, 30 + strlen(g_buf));
	free(g_buf);
	cJSON_Delete(json);
	return 0;
}

static int delete_configured_wifi_list(struct _bluetooth *stream, uint8 *json_in, struct json_parm_bluetooth *parm)
{
	cJSON *root = NULL;
	cJSON *array_obj = NULL;
	cJSON *arr_item, *item;
	char *g_buf = NULL;
	char sendbuf[1024 * 5] = { 0 };
	char wifi_name[200] = { 0 };
	char wifi_path[200] = {0};
	char now_ssid[50] = { 0 };
	cJSON *dir1, *dir2,*json,*dir3;		//组包使用

	root = cJSON_Parse((char*)json_in);
	if (root == NULL)
	{
		return BLUETOOTH_ERR_PARM;
	}
	array_obj = cJSON_GetObjectItem(root, "data");
	if (array_obj == NULL)
	{
		cJSON_Delete(root);
		return BLUETOOTH_ERR_PARM;
	}
	arr_item = cJSON_GetArrayItem(array_obj, 0);
	if (arr_item == NULL)
	{
		cJSON_Delete(root);
		return BLUETOOTH_ERR_PARM;
	}
	item = cJSON_GetObjectItem(arr_item, "ssid");
	if (item == NULL)
	{
		cJSON_Delete(root);
		return BLUETOOTH_ERR_PARM;
	}
	memcpy(wifi_name, item->valuestring, strlen(item->valuestring));
	//printf("utf8:wifi ssid %s\n", wifi_name);
	if (strlen(wifi_name) > 32)
	{
		bluetooth_out("SSID长度有误\n");
		cJSON_Delete(root);
		return BLUETOOTH_ERR_PARM;
	}

	
	sprintf(wifi_path, "/etc/wifi_config/wifi_%s.sh", wifi_name);
	//printf("utf8 wifi_path :%s",wifi_path);
	delete_file(wifi_path);

	
	find_now_wifi_ssid(now_ssid);
	if (strcmp(now_ssid, wifi_name)==0)
		system("killall wpa_supplicant");
	
	json = cJSON_CreateObject();
	cJSON_AddItemToObject(json, "head", dir1 = cJSON_CreateObject());
	cJSON_AddStringToObject(dir1, "protocol", BLUETOOTH_PROTOCOL);
	cJSON_AddStringToObject(dir1, "code_type", BLUETOOTH_SYNC_CODE_RESPONSE);
	cJSON_AddStringToObject(dir1, "cmd", parm->cmd);
	cJSON_AddStringToObject(dir1, "source_topic", parm->source_topic);
	cJSON_AddStringToObject(dir1, "random", parm->random);
	cJSON_AddStringToObject(dir1, "result", "s");

	cJSON_AddItemToObject(json, "data", dir2 = cJSON_CreateArray());
	cJSON_AddItemToObject(dir2, "dira", dir3 = cJSON_CreateObject());
	g_buf = cJSON_PrintUnformatted(json);
	//bluetooth_out("返回信息：%s\n", g_buf);
	printf("%s\n", g_buf);
	memcpy(sendbuf, "0001", 4);//协议头
	sprintf(sendbuf + 4, "%06d", strlen(g_buf));
	memcpy(sendbuf + 10, "00000001000100000000", 20);
	memcpy(sendbuf + 30, g_buf, strlen(g_buf));

	comport_send(stream, sendbuf, 30 + strlen(g_buf));
	free(g_buf);
	cJSON_Delete(json);
	return 0;
}

static int upgrade_by_bluetooth(struct _bluetooth *stream, uint8 *json_in, struct json_parm_bluetooth *parm)
{
	cJSON *root = NULL;
	cJSON *array_obj = NULL;
	cJSON *arr_item, *item;
	char *g_buf = NULL;
	char sendbuf[1024 * 5] = { 0 };
	char wifi_name[200] = { 0 };
	char wifi_path[200] = {0};
	char now_ssid[50] = { 0 };
	cJSON *dir1, *dir2,*json,*dir3;		//组包使用

	root = cJSON_Parse((char*)json_in);
	if (root == NULL)
	{
		return BLUETOOTH_ERR_PARM;
	}
	array_obj = cJSON_GetObjectItem(root, "data");
	if (array_obj == NULL)
	{
		cJSON_Delete(root);
		return BLUETOOTH_ERR_PARM;
	}
	arr_item = cJSON_GetArrayItem(array_obj, 0);
	if (arr_item == NULL)
	{
		cJSON_Delete(root);
		return BLUETOOTH_ERR_PARM;
	}
	item = cJSON_GetObjectItem(arr_item, "file_size");
	if (item == NULL)
	{
		cJSON_Delete(root);
		return BLUETOOTH_ERR_PARM;
	}
	stream->upgrade.file_size = atoi(item->valuestring);

	item = cJSON_GetObjectItem(arr_item, "pack_size");
	if (item == NULL)
	{
		cJSON_Delete(root);
		return BLUETOOTH_ERR_PARM;
	}
	stream->upgrade.pack_size = atoi(item->valuestring);

	item = cJSON_GetObjectItem(arr_item, "pack_total_num");
	if (item == NULL)
	{
		cJSON_Delete(root);
		return BLUETOOTH_ERR_PARM;
	}
	stream->upgrade.pack_total_num = atoi(item->valuestring);

	item = cJSON_GetObjectItem(arr_item, "file_md5");
	if (item == NULL)
	{
		cJSON_Delete(root);
		return BLUETOOTH_ERR_PARM;
	}
	strcpy(stream->upgrade.file_md5,item->valuestring);

	if(stream->upgrade.file_data != NULL)
	{
		free(stream->upgrade.file_data);
		stream->upgrade.file_data = NULL;
	}
	stream->upgrade.file_data_size = 0;
	stream->upgrade.cur_pack_num = 0;

	printf("****************enter upgrade_by_bluetooth****************\n");
	printf("file_size = %d,pack_size = %d,pack_total_num = %d,file_md5 = %s\n",stream->upgrade.file_size,stream->upgrade.pack_size,stream->upgrade.pack_total_num,stream->upgrade.file_md5);

	json = cJSON_CreateObject();
	cJSON_AddItemToObject(json, "head", dir1 = cJSON_CreateObject());
	cJSON_AddStringToObject(dir1, "protocol", BLUETOOTH_PROTOCOL);
	cJSON_AddStringToObject(dir1, "code_type", BLUETOOTH_SYNC_CODE_RESPONSE);
	cJSON_AddStringToObject(dir1, "cmd", parm->cmd);
	cJSON_AddStringToObject(dir1, "source_topic", parm->source_topic);
	cJSON_AddStringToObject(dir1, "random", parm->random);
	cJSON_AddStringToObject(dir1, "result", "s");

	cJSON_AddItemToObject(json, "data", dir2 = cJSON_CreateArray());
	cJSON_AddItemToObject(dir2, "dira", dir3 = cJSON_CreateObject());
	g_buf = cJSON_PrintUnformatted(json);
	//bluetooth_out("返回信息：%s\n", g_buf);
	printf("%s\n", g_buf);
	memcpy(sendbuf, "0001", 4);//协议头
	sprintf(sendbuf + 4, "%06d", strlen(g_buf));
	memcpy(sendbuf + 10, "00000001000100000000", 20);
	memcpy(sendbuf + 30, g_buf, strlen(g_buf));

	comport_send(stream, sendbuf, 30 + strlen(g_buf));
	free(g_buf);
	cJSON_Delete(root);
	cJSON_Delete(json);
	return 0;
}


static int mb_chmod_file_path(char *power, char *path)
{
	char cmd[1000] = { 0 };
	memset(cmd, 0, sizeof(cmd));
	sprintf(cmd, "chmod %s %s", power, path);
	system(cmd);
	sync();
	return 0;
}

static int mb_create_file_dir(char *file_path)
{
	char cmd[1000] = { 0 };
	sprintf(cmd, "mkdir %s", file_path);
	system(cmd);
	sync();
	return 0;
}

static int mb_tar_decompress(char *tar, char *path)
{
	char cmd[1000] = { 0 };
	memset(cmd, 0, sizeof(cmd));
	sprintf(cmd, "tar -xf %s -C %s/", tar, path);
	system(cmd);
	sync();
	return 0;
}

static int mb_copy_file_path(char *from, char *to)
{
	char cmd[1000] = { 0 };
	memset(cmd, 0, sizeof(cmd));
	sprintf(cmd, "cp %s %s", from, to);
	system(cmd);
	sync();
	return 0;
}

static int mb_delete_file_dir(char *file_path, int file_type)
{
	char cmd[1000] = { 0 };
	if (file_type == 0)
	{
		//文件删除
		sprintf(cmd, "rm %s", file_path);
	}
	else
	{
		//文件夹删除
		sprintf(cmd, "rm -rf %s/", file_path);
	}
	system(cmd);
	sync();
	return 0;
}

static int File_Compare_MD5(char *src_file, char *comp_file)
{
	int fds, fdc;
	int sizes = 0, sizec = 0, size_read = 0;
	struct stat statbufs;
	struct stat statbufc;
	char *buff_s = NULL, *buff_c = NULL;
	MD5_CTX ctx;
	char md5_s[16] = { 0 }, md5_c[16] = { 0 };
	if (src_file == NULL || comp_file == NULL)
		return -1;
	if (access(src_file, 0) != 0)
	{
		return -1;
	}
	if (access(comp_file, 0) != 0)
	{
		return 0;//此种情况可以说是MD5值不同
	}
	stat(src_file, &statbufs);
	sizes = statbufs.st_size;
	buff_s = boot_malloc(sizes + 10);
	if (buff_s == NULL)
	{
		return -1;
	}


	stat(comp_file, &statbufc);
	sizec = statbufc.st_size;
	buff_c = boot_malloc(sizec + 10);
	if (buff_c == NULL)
	{
		free(buff_s);
		return -1;
	}


	fds = open(src_file, O_RDONLY);
	if (fds < 0)
	{
		free(buff_c);
		free(buff_s);
		return -1;
	}
	size_read = read(fds, buff_s, sizes);
	if (size_read != sizes)
	{
		free(buff_c);
		free(buff_s);
		close(fds);
		return -1;
	}

	MD5_Init(&ctx);
	MD5_Update(&ctx, buff_s, sizes);
	MD5_Final((unsigned char *)md5_s, &ctx);

	close(fds);

	size_read = 0;
	fdc = open(comp_file, O_RDONLY);
	if (fdc < 0)
	{
		free(buff_c);
		free(buff_s);
		return 0;
	}
	size_read = read(fdc, buff_c, sizec);
	if (size_read != sizec)
	{
		//printf("read err ! not full!\n");
		free(buff_c);
		free(buff_s);
		close(fdc);
		return 0;
	}

	MD5_Init(&ctx);
	MD5_Update(&ctx, buff_c, sizec);
	MD5_Final((unsigned char *)md5_c, &ctx);

	close(fdc);

	if (memcmp(md5_s, md5_c, 16) != 0)
	{
		free(buff_c);
		free(buff_s);
		return 0;//MD5值不同
	}
	free(buff_c);
	free(buff_s);
	//printf("MD5值相同不进行替换!\n");
	return 1;//MD5值相同
}


static int parse_config_copy_file(unsigned char *configname, unsigned char *now_path)
{
	FILE *file;
	int result = 0;
	char lineBuff[256] = { 0 };// 存储读取一行的数据
	char* linePos = NULL;
	char file_name[100] = { 0 };
	char file_real_path[100] = { 0 };
	char tar_file_path[100] = { 0 };
	char real_file_path[100] = { 0 };

	if (access((char *)configname, 0) != 0)
	{
		return -1;
	}
	file = fopen((char *)configname, "rb");
	if (!file)
	{
		return -1;
	}
	fseek(file, 0, SEEK_END);
	ftell(file); //配置文件长度  
	fseek(file, 0, SEEK_SET);
	memset(lineBuff, 0, sizeof(lineBuff));
	//下面的循环是先判断config文件的格式是否正确
	while (fgets(lineBuff, 256, file) != NULL)
	{
		if ((lineBuff[0] == '#') || (lineBuff[0] == ';') || memcmp(lineBuff, "\r\n", 2) == 0 || lineBuff[0] == '\n')
			continue;
		linePos = strstr(lineBuff, "=");
		if (linePos == NULL)
		{
			memset(lineBuff, 0, sizeof(lineBuff));
			fclose(file);
			return -1;
		}
	}
	fseek(file, 0, SEEK_SET);
	memset(lineBuff, 0, sizeof(lineBuff));
	while (fgets(lineBuff, 256, file) != NULL)
	{
		if ((lineBuff[0] == '#') || (lineBuff[0] == ';') || memcmp(lineBuff, "\r\n", 2) == 0 || lineBuff[0] == '\n')
			continue;
		linePos = strstr(lineBuff, " = ");
		if (linePos == NULL)
		{
			memset(lineBuff, 0, sizeof(lineBuff));
			fclose(file);
			return -1;
		}
		str_replace(lineBuff, "\r\n", "\n");

		memset(file_name, 0, sizeof(file_name));
		memcpy(file_name, lineBuff, strlen(lineBuff) - strlen(linePos));
		//if (memcmp(file_name, "mb_boot", 7) == 0)//判断是否是本boot程序，如果是，则跳过.
		//	continue;
		memset(file_real_path, 0, sizeof(file_real_path));
		if (linePos[strlen(linePos) - 1] == '\n'&&linePos[strlen(linePos) - 2] == '\r')
			linePos[strlen(linePos) - 2] = '\0';
		else if (linePos[strlen(linePos) - 1] == '\n')
			linePos[strlen(linePos) - 1] = '\0';
		memcpy(file_real_path, linePos + 3, strlen(linePos) - 3);//减去一个" = "

		if (file_real_path[strlen(file_real_path) - 1] == '/')//统一格式(如路径中结尾有'/'，则去掉，后面统一加上)
		{
			file_real_path[strlen(file_real_path) - 1] = '\0';
		}

		memset(tar_file_path, 0, sizeof(tar_file_path));
		sprintf(tar_file_path, "%s/%s", now_path, file_name);
		//检测tar目录下是否有要更新的文件和是否存在真实复制替换的目录
		//printf("path1 = %s\npath2 = %s\n", file_real_path, tar_file_path);
		if (access(file_real_path, 0) != 0)
		{
			fclose(file);
			return -1;
		}
		if (access(tar_file_path, 0) != 0)
		{
			fclose(file);
			return -1;
		}
		memset(real_file_path, 0, sizeof(real_file_path));
		sprintf(real_file_path, "%s/%s", file_real_path, file_name);
		result = File_Compare_MD5(tar_file_path, real_file_path);
		if (result == 0)
		{
			//MD5值不同进行替换
			//原系统有该文件，先备份该文件为filename.back,用于检测失败回退使用
			/*if (access(real_file_path, 0) == 0)
			{
				char backpath[200] = { 0 };
				sprintf(backpath, "%s.back", real_file_path);
				mb_copy_file_path(real_file_path, backpath);
			}*/
			//用本次tar压缩包下的文件替换系统下文件
			mb_copy_file_path(tar_file_path, file_real_path);
		}
		else if (result < 0)
		{
			deal_cmd_newshare_out("err Compare MD5!\n");
			fclose(file);
			return result;
		}
		memset(lineBuff, 0, sizeof(lineBuff));
		continue;
	}
	return 0;
	fclose(file);
}

static int Write_Decompress_file(unsigned char *file, long file_len, unsigned char *filename, unsigned char * Dest_dir)
{
	int file_fd, result;
	char config_path[200] = { 0 };
	mb_delete_file_dir((char *)filename, 0);

	file_fd = open((char *)filename, O_CREAT | O_RDWR);
	if (file_fd < 0)
	{
		return -1;
	}
	result = write(file_fd, file, file_len);
	if (result < 0)
	{
		close(file_fd);
		return -1;
	}
	if (access((char *)Dest_dir, 0) != 0)
	{
		mb_create_file_dir((char *)Dest_dir);
	}
	mb_chmod_file_path("777", (char *)filename);
	mb_tar_decompress((char *)filename, (char *)Dest_dir);
	close(file_fd);
	sleep(1);//等待解压完成
	//进行解压文件的校验和复制到当前系统
	sprintf((char*)config_path, "%s/mb_config.cnf", Dest_dir);
	result = parse_config_copy_file((unsigned char *)config_path, Dest_dir);
	if (result < 0)
		return result;
	return 0;
}


/**********************************************************************
 *
 * File name    : base64.cpp / base64.c
 * Function     : base64 encoding and decoding of data or file.
 * Created time : 2020-08-04
 *
 *********************************************************************/

//base64 编码转换表，共64个
static const char base64_encode_table[] = {
    'A','B','C','D','E','F','G','H','I','J',
    'K','L','M','N','O','P','Q','R','S','T',
    'U','V','W','X','Y','Z','a','b','c','d',
    'e','f','g','h','i','j','k','l','m','n',
    'o','p','q','r','s','t','u','v','w','x',
    'y','z','0','1','2','3','4','5','6','7',
    '8','9','+','/'
};

//base64 解码表
static const unsigned char base64_decode_table[] = {
    //每行16个
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,                //1 - 16
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,                //17 - 32
    0,0,0,0,0,0,0,0,0,0,0,62,0,0,0,63,              //33 - 48
    52,53,54,55,56,57,58,59,60,61,0,0,0,0,0,0,      //49 - 64
    0,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,           //65 - 80
    15,16,17,18,19,20,21,22,23,24,25,0,0,0,0,0,     //81 - 96
    0,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40, //97 - 112
    41,42,43,44,45,46,47,48,49,50,51,0,0,0,0,0      //113 - 128
};

/**
 * @brief base64_encode     base64编码
 * @param indata            需编码的数据
 * @param inlen             需编码的数据大小
 * @param outdata           编码后输出的数据
 * @param outlen            编码后输出的数据大小
 * @return  int             0：成功    -1：无效参数
 */
static int my_base64_encode(const char *indata, int inlen, char *outdata, int *outlen)
{
    if(indata == NULL || inlen <= 0) {
        return -1;
    }
/*
    //方法一：
    int i, j;
    char ch;
    int add_len = (inlen % 3 == 0 ? 0 : 3 - inlen % 3); //原字符串需补齐的字符个数
    int in_len = inlen + add_len; //源字符串补齐字符后的长度，为3的倍数
    if(outdata != NULL) {
        //编码，长度为调整之后的长度，3字节一组
        for(i=0, j=0; i<in_len; i+=3, j+=4) {
            //将indata第一个字符向右移动2bit（丢弃2bit）
            ch = base64_encode_table[(unsigned char)indata[i] >> 2]; //对应base64转换表的字符
            outdata[j] = ch; //赋值

            //处理最后一组（最后3个字节）的数据
            if(i == in_len - 3 && add_len != 0) {
                if(add_len == 1) {
                    outdata[j + 1] = base64_encode_table[(((unsigned char)indata[i] & 0x03) << 4) | ((unsigned char)indata[i + 1] >> 4)];
                    outdata[j + 2] = base64_encode_table[((unsigned char)indata[i + 1] & 0x0f) << 2];
                    outdata[j + 3] = '=';
                }
                else if(add_len == 2) {
                    outdata[j + 1] = base64_encode_table[((unsigned char)indata[i] & 0x03) << 4];
                    outdata[j + 2] = '=';
                    outdata[j + 3] = '=';
                }
            }
            //处理正常的3字节数据
            else {
                outdata[j + 1] = base64_encode_table[(((unsigned char)indata[i] & 0x03) << 4) | ((unsigned char)indata[i + 1] >> 4)];
                outdata[j + 2] = base64_encode_table[(((unsigned char)indata[i + 1] & 0x0f) << 2) | ((unsigned char)indata[i + 2] >> 6)];
                outdata[j + 3] = base64_encode_table[(unsigned char)indata[i + 2] & 0x3f];
            }
        }
    }
    if(outlen != NULL) {
        *outlen = in_len * 4 / 3; //编码后的长度
    }
*/
    //方法二：
    int i, j;
    unsigned char num = inlen % 3;
    if(outdata != NULL) {
        //编码，3个字节一组，若数据总长度不是3的倍数，则跳过最后的 num 个字节数据
        for(i=0, j=0; i<inlen - num; i+=3, j+=4) {
            outdata[j] = base64_encode_table[(unsigned char)indata[i] >> 2];
            outdata[j + 1] = base64_encode_table[(((unsigned char)indata[i] & 0x03) << 4) | ((unsigned char)indata[i + 1] >> 4)];
            outdata[j + 2] = base64_encode_table[(((unsigned char)indata[i + 1] & 0x0f) << 2) | ((unsigned char)indata[i + 2] >> 6)];
            outdata[j + 3] = base64_encode_table[(unsigned char)indata[i + 2] & 0x3f];
        }
        //继续处理最后的 num 个字节的数据
        if(num == 1) { //余数为1，需补齐两个字节'='
            outdata[j] = base64_encode_table[(unsigned char)indata[inlen - 1] >> 2];
            outdata[j + 1] = base64_encode_table[((unsigned char)indata[inlen - 1] & 0x03) << 4];
            outdata[j + 2] = '=';
            outdata[j + 3] = '=';
        }
        else if(num == 2) { //余数为2，需补齐一个字节'='
            outdata[j] = base64_encode_table[(unsigned char)indata[inlen - 2] >> 2];
            outdata[j + 1] = base64_encode_table[(((unsigned char)indata[inlen - 2] & 0x03) << 4) | ((unsigned char)indata[inlen - 1] >> 4)];
            outdata[j + 2] = base64_encode_table[((unsigned char)indata[inlen - 1] & 0x0f) << 2];
            outdata[j + 3] = '=';
        }
    }
    if(outlen != NULL) {
        *outlen = (inlen + (num == 0 ? 0 : 3 - num)) * 4 / 3; //编码后的长度
    }

    return 0;
}

/**
 * @brief base64_decode     base64解码
 * @param indata            需解码的数据
 * @param inlen             需解码的数据大小
 * @param outdata           解码后输出的数据
 * @param outlen            解码后输出的数据大小
 * @return  int             0：成功    -1：无效参数
 * 注意：解码的数据的大小必须大于4，且是4的倍数
 */
static int  my_base64_decode(const char *indata, int inlen, char *outdata, int *outlen)
{
    if(indata == NULL || inlen <= 0 || (outdata == NULL && outlen == NULL)) {
        return -1;
    }
    if(inlen < 4 ||inlen % 4 != 0) { //需要解码的数据长度不是4的倍数  //inlen < 4 ||
        return -1;
    }

    int i, j;

    //计算解码后的字符串长度
    int len = inlen / 4 * 3;
    if(indata[inlen - 1] == '=') {
        len--;
    }
    if(indata[inlen - 2] == '=') {
        len--;
    }

    if(outdata != NULL) {
        for(i=0, j=0; i<inlen; i+=4, j+=3) {
            outdata[j] = (base64_decode_table[(unsigned char)indata[i]] << 2) | (base64_decode_table[(unsigned char)indata[i + 1]] >> 4);
            outdata[j + 1] = (base64_decode_table[(unsigned char)indata[i + 1]] << 4) | (base64_decode_table[(unsigned char)indata[i + 2]] >> 2);
            outdata[j + 2] = (base64_decode_table[(unsigned char)indata[i + 2]] << 6) | (base64_decode_table[(unsigned char)indata[i + 3]]);
        }
    }
    if(outlen != NULL) {
        *outlen = len;
    }
    return 0;
}


#define TAR_FILE_PATH_APP	"/home/share/exe/tar/app.tar"
static int download_by_bluetooth(struct _bluetooth *stream, uint8 *json_in, struct json_parm_bluetooth *parm)
{
	int result = 0;
	cJSON *root = NULL;
	cJSON *array_obj = NULL;
	cJSON *arr_item, *item;
	char *g_buf = NULL;
	char sendbuf[1024 * 5] = { 0 };
	char wifi_name[200] = { 0 };
	char wifi_path[200] = {0};
	char now_ssid[50] = { 0 };
	cJSON *dir1, *dir2,*json,*dir3;		//组包使用

	root = cJSON_Parse((char*)json_in);
	if (root == NULL)
	{
		return BLUETOOTH_ERR_PARM;
	}
	array_obj = cJSON_GetObjectItem(root, "data");
	if (array_obj == NULL)
	{
		cJSON_Delete(root);
		return BLUETOOTH_ERR_PARM;
	}
	arr_item = cJSON_GetArrayItem(array_obj, 0);
	if (arr_item == NULL)
	{
		cJSON_Delete(root);
		return BLUETOOTH_ERR_PARM;
	}

	item = cJSON_GetObjectItem(arr_item, "pack_num");
	if (item == NULL)
	{
		cJSON_Delete(root);
		return BLUETOOTH_ERR_PARM;
	}
	stream->upgrade.cur_pack_num = atoi(item->valuestring);

	if(stream->upgrade.cur_pack_num > stream->upgrade.pack_total_num)
	{
		result = -1;
	}

	item = cJSON_GetObjectItem(arr_item, "pack_data");
	if (item == NULL)
	{
		cJSON_Delete(root);
		return BLUETOOTH_ERR_PARM;
	}

	if(stream->upgrade.file_data == NULL)
	{
		stream->upgrade.file_data = calloc(1,stream->upgrade.file_size*3);   //多1k
	}

	// memcpy(stream->upgrade.file_data + stream->upgrade.cur_pack_num*stream->upgrade.pack_size,item->valuestring,strlen(item->valuestring));

	memcpy(stream->upgrade.file_data,item->valuestring,strlen(item->valuestring));
	stream->upgrade.file_data_size += strlen(item->valuestring);

	printf("***************stream->upgrade.cur_pack_num = %d,stream->upgrade.file_data_size = %d\n",stream->upgrade.cur_pack_num,stream->upgrade.file_data_size);

	json = cJSON_CreateObject();
	cJSON_AddItemToObject(json, "head", dir1 = cJSON_CreateObject());
	cJSON_AddStringToObject(dir1, "protocol", BLUETOOTH_PROTOCOL);
	cJSON_AddStringToObject(dir1, "code_type", BLUETOOTH_SYNC_CODE_RESPONSE);
	cJSON_AddStringToObject(dir1, "cmd", parm->cmd);
	cJSON_AddStringToObject(dir1, "source_topic", parm->source_topic);
	cJSON_AddStringToObject(dir1, "random", parm->random);
	if(result < 0)
		cJSON_AddStringToObject(dir1, "result", "f");
	else
		cJSON_AddStringToObject(dir1, "result", "s");

	cJSON_AddItemToObject(json, "data", dir2 = cJSON_CreateArray());
	cJSON_AddItemToObject(dir2, "dira", dir3 = cJSON_CreateObject());
	g_buf = cJSON_PrintUnformatted(json);
	//bluetooth_out("返回信息：%s\n", g_buf);
	printf("%s\n", g_buf);
	memcpy(sendbuf, "0001", 4);//协议头
	sprintf(sendbuf + 4, "%06d", strlen(g_buf));
	memcpy(sendbuf + 10, "00000001000100000000", 20);
	memcpy(sendbuf + 30, g_buf, strlen(g_buf));

	comport_send(stream, sendbuf, 30 + strlen(g_buf));
	free(g_buf);
	cJSON_Delete(root);
	cJSON_Delete(json);

	// int file_fd = -1,i = 0;
	// MD5_CTX ctx;
	// char md5_s[16] = { 0 },md5_data[64] = {0x00};
	// char *final_file_data = NULL;
	// int final_len = 0;
	// if(stream->upgrade.cur_pack_num == stream->upgrade.pack_total_num)   //包数达到最大数量
	// {
	// 	final_file_data = calloc(1,stream->upgrade.file_data_size + 1204);
	// 	my_base64_decode((const unsigned char *)stream->upgrade.file_data, stream->upgrade.file_data_size,final_file_data,&final_len);
		
	// 	// file_fd = open("/home/test_file", O_CREAT | O_RDWR);
	// 	// if (file_fd < 0)
	// 	// {
	// 	// 	return -1;
	// 	// }
	// 	// write(file_fd, final_file_data, final_len);
	// 	// close(file_fd);

	// 	printf("final_len = %d\n",final_len);
	// 	MD5_Init(&ctx);
	// 	MD5_Update(&ctx, final_file_data, final_len);
	// 	MD5_Final((unsigned char *)md5_s, &ctx);
		
	// 	for (i = 0; i < 16; i++)
	// 	{
	// 		sprintf(&md5_data[i * 2], "%02X", (unsigned char)md5_s[i]);
	// 	}
	// 	for(i = 0;i<strlen(md5_data);++i)
	// 	{
	// 		if(md5_data[i] > '9')
	// 		{
	// 			md5_data[i] = md5_data[i] + 32;
	// 		}
	// 	}
		
	// 	printf("file_md5 = %s,md5_data = %s\n",stream->upgrade.file_md5,md5_data);
	// 	if(strlen(stream->upgrade.file_md5) > 0 && 0 == memcmp(md5_data,stream->upgrade.file_md5,strlen(stream->upgrade.file_md5)))   //md5校验通过，反base64
	// 	{
	// 		printf("0 == memcmp\n");
	// 		// Write_Decompress_file(final_file_data, final_len, (unsigned char *)BULETOOTH_DOWNLOAD_FILENAME, BULETOOTH_UPGRADE_FILE_PATH);
	// 		//write_file("/etc/need_update_auth.log", "服务器要求系统更新升级文件", strlen("服务器要求系统更新升级文件"));

	// 		//if (access(TAR_FILE_PATH_APP, 0) == 0)
	// 		{
	// 			system("rm /home/share/exe/tar/app.tar");
	// 		}
	// 		//if (access("/home/share/exe/tar/tmpapp", 0) == 0)
	// 		{
	// 			system("rm  -rf /home/share/exe/tar/tmpapp");
	// 		}
	// 		file_fd = open(TAR_FILE_PATH_APP, O_CREAT | O_RDWR);
	// 		if (file_fd < 0)
	// 		{
	// 			printf("open error\n");
	// 			return -1;
	// 		}
	// 		write(file_fd, final_file_data, final_len);
	// 		close(file_fd);
	// 		stream->upgrade.upgarde_status = UPGRADED_SUCCESSED;
	// 		write_file("/etc/need_update_auth.log", "服务器要求系统更新升级文件", strlen("服务器要求系统更新升级文件"));
	// 	}
	// 	else
	// 	{
	// 		printf("0 != memcmp\n");
	// 		stream->upgrade.upgarde_status = UPGRADED_FIALED;
	// 	}
	// 	free(stream->upgrade.file_data);
	// 	stream->upgrade.file_data = NULL;
	// 	free(final_file_data);
	// }




	return 0;
}


//升级文件先转base64，再算md5
static int get_upgarde_status_by_bluetooth(struct _bluetooth *stream, uint8 *json_in, struct json_parm_bluetooth *parm)
{
	MD5_CTX ctx;
	cJSON *root = NULL;
	cJSON *array_obj = NULL;
	cJSON *arr_item, *item;
	char *g_buf = NULL;
	char sendbuf[1024 * 5] = { 0 };
	char tmp_data[64] = { 0 };
	char md5_s[16] = { 0 },md5_data[64] = {0x00};
	char app_update_path[128] = { 0 };
	char *final_file_data = NULL;
	int tmp_md5 = 0,final_len = 0;
	cJSON *dir1,*dir2,*json,*dir3;		//组包使用
	int i = 0;

	json = cJSON_CreateObject();
	cJSON_AddItemToObject(json, "head", dir1 = cJSON_CreateObject());
	cJSON_AddStringToObject(dir1, "protocol", BLUETOOTH_PROTOCOL);
	cJSON_AddStringToObject(dir1, "code_type", BLUETOOTH_SYNC_CODE_RESPONSE);
	cJSON_AddStringToObject(dir1, "cmd", parm->cmd);
	cJSON_AddStringToObject(dir1, "source_topic", parm->source_topic);
	cJSON_AddStringToObject(dir1, "random", parm->random);
	cJSON_AddStringToObject(dir1, "result", "s");

	cJSON_AddItemToObject(json, "data", dir2 = cJSON_CreateArray());
	cJSON_AddItemToObject(dir2, "dira", dir3 = cJSON_CreateObject());

	int file_fd;
	if(stream->upgrade.cur_pack_num == stream->upgrade.pack_total_num)   //包数达到最大数量
	{
		final_file_data = calloc(1,stream->upgrade.file_data_size + 1204);
		my_base64_decode((const unsigned char *)stream->upgrade.file_data, stream->upgrade.file_data_size,final_file_data,&final_len);
		
		// file_fd = open("/home/test_file", O_CREAT | O_RDWR);
		// if (file_fd < 0)
		// {
		// 	return -1;
		// }
		// write(file_fd, final_file_data, final_len);
		// close(file_fd);

		printf("final_len = %d\n",final_len);
		MD5_Init(&ctx);
		MD5_Update(&ctx, final_file_data, final_len);
		MD5_Final((unsigned char *)md5_s, &ctx);
		
		for (i = 0; i < 16; i++)
		{
			sprintf(&md5_data[i * 2], "%02X", (unsigned char)md5_s[i]);
		}
		for(i = 0;i<strlen(md5_data);++i)
		{
			if(md5_data[i] > '9')
			{
				md5_data[i] = md5_data[i] + 32;
			}
		}
		
		printf("file_md5 = %s,md5_data = %s\n",stream->upgrade.file_md5,md5_data);
		if(strlen(stream->upgrade.file_md5) > 0 && 0 == memcmp(md5_data,stream->upgrade.file_md5,strlen(stream->upgrade.file_md5)))   //md5校验通过，反base64
		{
			printf("0 == memcmp\n");
			// Write_Decompress_file(final_file_data, final_len, (unsigned char *)BULETOOTH_DOWNLOAD_FILENAME, BULETOOTH_UPGRADE_FILE_PATH);
			//write_file("/etc/need_update_auth.log", "服务器要求系统更新升级文件", strlen("服务器要求系统更新升级文件"));

			//if (access(TAR_FILE_PATH_APP, 0) == 0)
			{
				system("rm /home/share/exe/tar/app.tar");
			}
			//if (access("/home/share/exe/tar/tmpapp", 0) == 0)
			{
				system("rm  -rf /home/share/exe/tar/tmpapp");
			}
			file_fd = open(TAR_FILE_PATH_APP, O_CREAT | O_RDWR);
			if (file_fd < 0)
			{
				printf("open error\n");
				return -1;
			}
			write(file_fd, final_file_data, final_len);
			close(file_fd);
			stream->upgrade.upgarde_status = UPGRADED_SUCCESSED;
			write_file("/etc/need_update_auth.log", "服务器要求系统更新升级文件", strlen("服务器要求系统更新升级文件"));
		}
		else
		{
			printf("0 != memcmp\n");
			stream->upgrade.upgarde_status = UPGRADED_FIALED;
		}
		free(stream->upgrade.file_data);
		stream->upgrade.file_data = NULL;
		free(final_file_data);
	}

	memset(tmp_data,0x00,sizeof(tmp_data));
	sprintf(tmp_data,"%d",stream->upgrade.upgarde_status);
	printf("1111111111111111\n");
	//cJSON_AddItemToObject(dir3, "upgarde_status", tmp_data);
	printf("2222222222222222\n");

	g_buf = cJSON_PrintUnformatted(json);
	//bluetooth_out("返回信息：%s\n", g_buf);
	printf("%s\n", g_buf);
	memcpy(sendbuf, "0001", 4);//协议头
	sprintf(sendbuf + 4, "%06d", strlen(g_buf));
	memcpy(sendbuf + 10, "00000001000100000000", 20);
	memcpy(sendbuf + 30, g_buf, strlen(g_buf));

	comport_send(stream, sendbuf, 30 + strlen(g_buf));
	free(g_buf);
	cJSON_Delete(json);
	return 0;
}


static int control_dev_restore(struct _bluetooth *stream, uint8 *json_in, struct json_parm_bluetooth *parm)
{
	char *g_buf = NULL;
	char sendbuf[1024 * 5] = { 0 };
	int result;

	//DF_TY_USB_CM_RESTORE
	
	cJSON *dir1, *dir2,*dir3;		//组包使用
	cJSON *json = cJSON_CreateObject();
	cJSON_AddItemToObject(json, "head", dir1 = cJSON_CreateObject());
	cJSON_AddStringToObject(dir1, "protocol", BLUETOOTH_PROTOCOL);
	cJSON_AddStringToObject(dir1, "code_type", BLUETOOTH_SYNC_CODE_RESPONSE);
	cJSON_AddStringToObject(dir1, "cmd", parm->cmd);
	cJSON_AddStringToObject(dir1, "source_topic", parm->source_topic);
	cJSON_AddStringToObject(dir1, "random", parm->random);
	cJSON_AddStringToObject(dir1, "result", "s");

	cJSON_AddItemToObject(json, "data", dir2 = cJSON_CreateArray());
	cJSON_AddItemToObject(dir2, "dira", dir3 = cJSON_CreateObject());
	g_buf = cJSON_PrintUnformatted(json);

	memcpy(sendbuf, "0001", 4);//协议头
	sprintf(sendbuf + 4, "%06d", strlen(g_buf));
	memcpy(sendbuf + 10, "00000001000100000000", 20);
	memcpy(sendbuf + 30, g_buf, strlen(g_buf));

	comport_send(stream, sendbuf, 30 + strlen(g_buf));
	free(g_buf);
	cJSON_Delete(json);
	sleep(2);
	result = ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_RESTORE, NULL);
	if (result < 0)
	{
		bluetooth_out("***蓝牙命令回出厂失败\n");
	}
	return 0;
}
//蓝牙联机指令

static int ble_test_conn(struct _bluetooth *stream, uint8 *json_in, struct json_parm_bluetooth *parm)
{

	char *g_buf = NULL;
	char sendbuf[1024 * 5] = { 0 };
	cJSON *dir1, *dir2;		//组包使用
	cJSON *json = cJSON_CreateObject();
	cJSON_AddItemToObject(json, "head", dir1 = cJSON_CreateObject());
	cJSON_AddStringToObject(dir1, "protocol", BLUETOOTH_PROTOCOL);
	cJSON_AddStringToObject(dir1, "code_type", BLUETOOTH_SYNC_CODE_RESPONSE);
	cJSON_AddStringToObject(dir1, "cmd", parm->cmd);
	cJSON_AddStringToObject(dir1, "source_topic", parm->source_topic);
	cJSON_AddStringToObject(dir1, "random", parm->random);
	cJSON_AddStringToObject(dir1, "result", "s");

	cJSON_AddItemToObject(json, "data", dir2 = cJSON_CreateArray());
	cJSON *dir3;
	cJSON_AddItemToObject(dir2, "dira", dir3 = cJSON_CreateObject());
	g_buf = cJSON_PrintUnformatted(json);

	memcpy(sendbuf, "0001", 4);//协议头
	sprintf(sendbuf + 4, "%06d", strlen(g_buf));
	memcpy(sendbuf + 10, "00000001000100000000", 20);
	memcpy(sendbuf + 30, g_buf, strlen(g_buf));

	comport_send(stream, sendbuf, 30 + strlen(g_buf));
	free(g_buf);
	cJSON_Delete(json);
	return 0;
}

static int set_bluetooth_name(struct _bluetooth_fd   *id, va_list args)
{
	struct _bluetooth *stream;
	char mode_ble[2] = { 0 };
	char rbuf[1024] = { 0 };
	char sbuf[1024] = { 0 };
	char *ble_name = NULL;
	stream = id->dev;

	ble_name = va_arg(args, char *);
	if (ble_name == NULL)
		return -1;
	stream->ble_sta = Ble_init;
	sleep(5);
	//bluetooth_out("蓝牙句柄:%d\n", stream->ble_attr.fd);
	if (ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_CTL_BLUETOOTH_MODE, mode_ble) < 0)
	{
		bluetooth_out("设置蓝牙AT模式失败\n");
	}
	usleep(2000);
	if (bluetooth_send_recv(stream, "AT+DISCONN\r\n", rbuf, sizeof(rbuf)) < 0)
	{
		bluetooth_out("蓝牙断开链接失败:%s\n", rbuf);
		return -1;
	}
	sprintf(sbuf, "AT+NAME=%s\r\n", ble_name);
	if (bluetooth_send_recv(stream, sbuf, rbuf, sizeof(rbuf)) < 0)
	{
		bluetooth_out("设置蓝牙名称结果失败:%s\n", rbuf);
		return -1;
	}
	bluetooth_out("设置蓝牙名称结果:%s\n", rbuf);

	memset(sbuf, 0, sizeof(sbuf));
	sprintf(sbuf, "AT+SAVE\r\n");
	if (bluetooth_send_recv(stream, sbuf, rbuf, sizeof(rbuf)) < 0)
	{
		return -1;
	}
	printf("save at result:%s\n", rbuf);

	memset(sbuf, 0, sizeof(sbuf));
	sprintf(sbuf, "AT+RESET\r\n");
	if (bluetooth_send_recv(stream, sbuf, rbuf, sizeof(rbuf)) < 0)
	{
		return -1;
	}
	stream->ble_sta = Ble_broadcasting;
	return 0;
}

//单板检测获取端口AD状态
static int get_all_ports_ad(struct _bluetooth *stream, uint8 *json_in, struct json_parm_bluetooth *parm)
{
	int port_count;
	//char sendbuf[1024 * 5] = { 0 };
	char *g_buf = NULL;
	cJSON *dir1, *dir2;		//组包使用
	cJSON *json = cJSON_CreateObject();
	cJSON_AddItemToObject(json, "head", dir1 = cJSON_CreateObject());
	cJSON_AddStringToObject(dir1, "protocol", BLUETOOTH_PROTOCOL);
	cJSON_AddStringToObject(dir1, "code_type", BLUETOOTH_SYNC_CODE_RESPONSE);
	cJSON_AddStringToObject(dir1, "cmd", parm->cmd);
	cJSON_AddStringToObject(dir1, "source_topic", parm->source_topic);
	cJSON_AddStringToObject(dir1, "random", parm->random);
	cJSON_AddStringToObject(dir1, "result", "s");

	int port_num = ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_PORT_NUMBER);//获取终端端口数量
	//bluetooth_out("该终端一共有%d个USB端口\n");
	if (port_num == 68)
		port_num = 34;
	cJSON *port_array;
	cJSON_AddItemToObject(json, "data", dir2 = cJSON_CreateArray());
	cJSON *data_layer = cJSON_CreateObject();
	cJSON_AddItemToObject(dir2, "dirl", data_layer);

	cJSON_AddItemToObject(data_layer, "usb_port_info", port_array = cJSON_CreateArray());

	int len;
	char all_ad[1024];
	len = ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_GET_AD_STATUS, all_ad);
	if (len <= 0)
	{
		//return err_bluetooth_out_tcp(fd, -99);
		printf("err!\n");
		cJSON_Delete(json);
		return -1;
	}
	/*printf("len is :%d\n");
	int i;
	for (i = 0; i < 200; i++)
	{
	printf("%02x ", all_ad[i]);
	}
	printf("\n\n");*/
	char *point_ad = all_ad;
	//	point_ad += 2;
	int ad_value;
	char tmp_s[100] = { 0 };
	for (port_count = 0; port_count<port_num; port_count++)
	{
		ad_value = *point_ad * 256 + *(point_ad + 1);
		point_ad += 2;
		cJSON *port_layer;
		cJSON_AddItemToObject(port_array, "dira", port_layer = cJSON_CreateObject());
		memset(tmp_s, 0, sizeof(tmp_s)); sprintf(tmp_s, "%d", port_count + 1);
		cJSON_AddStringToObject(port_layer, "port_num", tmp_s);

		memset(tmp_s, 0, sizeof(tmp_s)); sprintf(tmp_s, "%d", ad_value);
		cJSON_AddStringToObject(port_layer, "port_ad", tmp_s);
	}

	g_buf = cJSON_PrintUnformatted(json);
	//printf("gbuf:\n%s\n", g_buf);
	//memcpy(sendbuf, "0001", 4);//协议头
	//sprintf(sendbuf + 4, "%06d", strlen(g_buf));
	//memcpy(sendbuf + 10, "00000001000100000000", 20);
	//memcpy(sendbuf + 30, g_buf, strlen(g_buf));

	//comport_send(ble_attr, sendbuf, 30 + strlen(g_buf));
	char encryption[3] = { 0 };
	memcpy(encryption, parm->head->encryption, 2);
	if (bluetooth_send_packets(stream, g_buf, strlen(g_buf), encryption) < 0)
	{
		free(g_buf);
		cJSON_Delete(json);
		return BLUETOOTH_ERR_SEND;
	}
	free(g_buf);
	cJSON_Delete(json);
	return 0;
}


//单板检测获取端口状态
static int get_all_ports_status(struct _bluetooth *stream, uint8 *json_in, struct json_parm_bluetooth *parm)
{
	int result,port_count;
	//char sendbuf[1024 * 5] = { 0 };
	char *g_buf = NULL;
	cJSON *dir1, *dir2;		//组包使用
	cJSON *json = cJSON_CreateObject();
	cJSON_AddItemToObject(json, "head", dir1 = cJSON_CreateObject());
	cJSON_AddStringToObject(dir1, "protocol", BLUETOOTH_PROTOCOL);
	cJSON_AddStringToObject(dir1, "code_type", BLUETOOTH_SYNC_CODE_RESPONSE);
	cJSON_AddStringToObject(dir1, "cmd", parm->cmd);
	cJSON_AddStringToObject(dir1, "source_topic", parm->source_topic);
	cJSON_AddStringToObject(dir1, "random", parm->random);
	cJSON_AddStringToObject(dir1, "result", "s");

	int port_num = ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_PORT_NUMBER);//获取终端端口数量
	//bluetooth_out("该终端一共有%d个USB端口\n");
	//if (port_num == 68)
		//port_num = 34;
	cJSON *port_array;
	cJSON_AddItemToObject(json, "data", dir2 = cJSON_CreateArray());
	cJSON *data_layer = cJSON_CreateObject();
	cJSON_AddItemToObject(dir2, "dirl", data_layer);

	cJSON_AddItemToObject(data_layer, "usb_port_info", port_array = cJSON_CreateArray());

	char tmp_s[100] = { 0 };
	for (port_count = 1; port_count <= port_num; port_count++)
	{
		result = ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_GET_PORT_FILE_EXIST, port_count);
		cJSON *port_layer;
		cJSON_AddItemToObject(port_array, "dira", port_layer = cJSON_CreateObject());
		memset(tmp_s, 0, sizeof(tmp_s)); sprintf(tmp_s, "%d", port_count);
		cJSON_AddStringToObject(port_layer, "port_num", tmp_s);
		//printf("port:%d busid :%s\n",port_count,)
		//	result = ty_ctl(stream->ty_usb_m_fd, DF_TY_USB_M_CM_GET_PORT_ERROR, port_count);
		if (result<0)
		{
			//bluetooth_out("%d号端口没有设备\n", port_count + 1);
			cJSON_AddStringToObject(port_layer, "port_status", "1");

		}
		else
		{
			cJSON_AddStringToObject(port_layer, "port_status", "0");
		}
	}

	g_buf = cJSON_PrintUnformatted(json);

	//memcpy(sendbuf, "0001", 4);//协议头
	//sprintf(sendbuf + 4, "%06d", strlen(g_buf));
	//memcpy(sendbuf + 10, "00000001000100000000", 20);
	//memcpy(sendbuf + 30, g_buf, strlen(g_buf));

	//comport_send(ble_attr, sendbuf, 30 + strlen(g_buf));
	//free(g_buf);
	//cJSON_Delete(json);


	char encryption[3] = { 0 };
	memcpy(encryption, parm->head->encryption, 2);
	if (bluetooth_send_packets(stream, g_buf, strlen(g_buf), encryption) < 0)
	{
		free(g_buf);
		cJSON_Delete(json);
		return BLUETOOTH_ERR_SEND;
	}
	free(g_buf);
	cJSON_Delete(json);
	return 0;
}

//上电所有端口

static int ble_up_all_power(struct _bluetooth *stream, uint8 *json_in, struct json_parm_bluetooth *parm)
{
	int result;
	int i, usb_sum;
	char *g_buf = NULL;
	char sendbuf[1024] = { 0 };
	usb_sum = ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_PORT_NUMBER);
	for (i = 0; i < usb_sum; i++)
	{
		result = ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_OPEN_PORT_POWER, i + 1);
		if (result < 0)
		{
			ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_OPEN_PORT_POWER, i + 1);
		}
		usleep(20000);
	}
	cJSON *dir1, *dir2;		//组包使用
	cJSON *json = cJSON_CreateObject();
	cJSON_AddItemToObject(json, "head", dir1 = cJSON_CreateObject());
	cJSON_AddStringToObject(dir1, "protocol", BLUETOOTH_PROTOCOL);
	cJSON_AddStringToObject(dir1, "code_type", BLUETOOTH_SYNC_CODE_RESPONSE);
	cJSON_AddStringToObject(dir1, "cmd", parm->cmd);
	cJSON_AddStringToObject(dir1, "source_topic", parm->source_topic);
	cJSON_AddStringToObject(dir1, "random", parm->random);
	if (result<0)
	{
		cJSON_AddStringToObject(dir1, "result", "f");
	}
	else
	{
		cJSON_AddStringToObject(dir1, "result", "s");
	}
	cJSON_AddItemToObject(json, "data", dir2 = cJSON_CreateArray());
	cJSON *dir3;
	cJSON_AddItemToObject(dir2, "dira", dir3 = cJSON_CreateObject());
	g_buf = cJSON_PrintUnformatted(json);

	memcpy(sendbuf, "0001", 4);//协议头
	sprintf(sendbuf + 4, "%06d", strlen(g_buf));
	memcpy(sendbuf + 10, "00000001000100000000", 20);
	memcpy(sendbuf + 30, g_buf, strlen(g_buf));

	comport_send(stream, sendbuf, 30 + strlen(g_buf));
	free(g_buf);
	cJSON_Delete(json);
	return 0;
}

//下电所有端口

static int ble_down_all_power(struct _bluetooth *stream, uint8 *json_in, struct json_parm_bluetooth *parm)
{
	//int result;
	int i, usb_sum;
	char *g_buf = NULL;
	char sendbuf[1024] = { 0 };
	usb_sum = ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_PORT_NUMBER);
	//if (usb_sum == 68)
		//usb_sum = 34;
	for (i = 0; i < usb_sum; i++)
	{
		ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_CLOSE_PORT_POWER, i + 1);
		usleep(20000);
	}
	cJSON *dir1, *dir2;		//组包使用
	cJSON *json = cJSON_CreateObject();
	cJSON_AddItemToObject(json, "head", dir1 = cJSON_CreateObject());
	cJSON_AddStringToObject(dir1, "protocol", BLUETOOTH_PROTOCOL);
	cJSON_AddStringToObject(dir1, "code_type", BLUETOOTH_SYNC_CODE_RESPONSE);
	cJSON_AddStringToObject(dir1, "cmd", parm->cmd);
	cJSON_AddStringToObject(dir1, "source_topic", parm->source_topic);
	cJSON_AddStringToObject(dir1, "random", parm->random);
	/*if (result<0)
	{
		cJSON_AddStringToObject(dir1, "result", "f");
	}
	else*/
	{
		cJSON_AddStringToObject(dir1, "result", "s");
	}
	cJSON_AddItemToObject(json, "data", dir2 = cJSON_CreateArray());
	cJSON *dir3;
	cJSON_AddItemToObject(dir2, "dira", dir3 = cJSON_CreateObject());
	g_buf = cJSON_PrintUnformatted(json);

	memcpy(sendbuf, "0001", 4);//协议头
	sprintf(sendbuf + 4, "%06d", strlen(g_buf));
	memcpy(sendbuf + 10, "00000001000100000000", 20);
	memcpy(sendbuf + 30, g_buf, strlen(g_buf));

	comport_send(stream, sendbuf, 30 + strlen(g_buf));
	free(g_buf);
	cJSON_Delete(json);
	return  0;
}

static int Key_Check_Http(struct http_parm * parm)
{
	int sock = 0;
	int result;
	struct timeval tv;
	result = http_socket_open(&sock, parm);
	if (result <0)
	{
		printf("socket create failed\n");//socket 创建失败直接返回

		return -1;
	}
	result = http_get_token(sock, parm);
	if (result < 0)
	{
		printf("http get token err!\n");
		shutdown(sock, SHUT_RDWR);
		close(sock);
		return -1;
	}

	gettimeofday(&tv, NULL);
	memset(parm->timestamp, 0, sizeof(parm->timestamp));
	sprintf((char*)parm->timestamp, "%ld", tv.tv_sec);


	result = http_send_sign_enc(sock, parm);
	if (result < 0)
	{
		printf("http_send_sign_enc err!\n");
		shutdown(sock, SHUT_RDWR);
		close(sock);
		return -1;
	}
	shutdown(sock, SHUT_RDWR);
	close(sock);
	return 0;
}

//获取密钥是否正确状态
static int get_key_status(struct _bluetooth *stream, uint8 *json_in, struct json_parm_bluetooth *parm)
{
	int result;
	char sendbuf[1024] = { 0 };
	char *g_buf = NULL;
	struct http_parm parm_h;
	char baseudid[100] = { 0 };
	struct _switch_dev_only 	udid;


	result = get_DNS_ip("www.njmbxx.com", parm_h.ip);
	if (result < 0)
	{
		;
	}
	ty_ctl(stream->ty_usb_fd, DF_TU_USB_CM_PORT_GET_KEY_VALUE, parm_h.key_s);
	result = ty_ctl(stream->switch_fd, DF_SWITCH_CM_GET_SERIAL_NUM, &udid);
	if (result<0)
	{
		bluetooth_out("获取唯一标识符失败\n");
		return -1;
	}
	memcpy(parm_h.ter_id, "TERREGENCCKECK", 14);
	memcpy(parm_h.appid, "TERREGENCCKECK", 14);
	memset(baseudid, 0, sizeof(baseudid));
	encode((char *)udid.serial_num, 12, (char *)baseudid);
	memcpy(parm_h.cpu_id, baseudid, 16);
	memcpy(parm_h.app_ver, stream->note->ver, strlen(stream->note->ver));

	result = Key_Check_Http(&parm_h);

	cJSON *dir1, *dir2;		//组包使用
	cJSON *json = cJSON_CreateObject();
	cJSON_AddItemToObject(json, "head", dir1 = cJSON_CreateObject());
	cJSON_AddStringToObject(dir1, "protocol", BLUETOOTH_PROTOCOL);
	cJSON_AddStringToObject(dir1, "code_type", BLUETOOTH_SYNC_CODE_RESPONSE);
	cJSON_AddStringToObject(dir1, "cmd", parm->cmd);
	cJSON_AddStringToObject(dir1, "source_topic", parm->source_topic);
	cJSON_AddStringToObject(dir1, "random", parm->random);
	if (result<0)
	{
		cJSON_AddStringToObject(dir1, "result", "f");
	}
	else
	{
		cJSON_AddStringToObject(dir1, "result", "s");
	}
	cJSON_AddItemToObject(json, "data", dir2 = cJSON_CreateArray());
	cJSON *dir3;
	cJSON_AddItemToObject(dir2, "dira", dir3 = cJSON_CreateObject());
	g_buf = cJSON_PrintUnformatted(json);

	memcpy(sendbuf, "0001", 4);//协议头
	sprintf(sendbuf + 4, "%06d", strlen(g_buf));
	memcpy(sendbuf + 10, "00000001000100000000", 20);
	memcpy(sendbuf + 30, g_buf, strlen(g_buf));

	comport_send(stream, sendbuf, 30 + strlen(g_buf));
	free(g_buf);
	cJSON_Delete(json);
	return 0;
}

static int judge_hub(int port_num)
{
	//char err_hub_id[2048];
	int err = 0;
	if (port_num == 32)
	{
		if (access("/sys/bus/usb/devices/1-1.1", 0) != 0)
		{
			err += 1;

		}
		if (access("/sys/bus/usb/devices/1-1.2", 0) != 0)
		{
			err += 1;

		}
		if (access("/sys/bus/usb/devices/1-1.3", 0) != 0)
		{
			err += 1;

		}
		if (access("/sys/bus/usb/devices/1-1.4", 0) != 0)
		{
			err += 1;

		}
		if (access("/sys/bus/usb/devices/1-1.5", 0) != 0)
		{
			err += 1;

		}
		if (err != 0)
			return -1;
		return 0;

	}
	else if (port_num == 4)
	{
		if (access("/sys/bus/usb/devices/1-1", 0) != 0)
		{
			err += 1;

		}
		if (err != 0)
			return -1;
		return 0;

	}
	else if (port_num == 20)
	{
		if (access("/sys/bus/usb/devices/1-1.1", 0) != 0)
		{
			err += 1;

		}
		if (access("/sys/bus/usb/devices/1-1.2", 0) != 0)
		{
			err += 1;

		}
		if (access("/sys/bus/usb/devices/1-1.3", 0) != 0)
		{
			err += 1;

		}
		if (err != 0)
			return -1;
		return 0;

	}
	else if (port_num == 68)
	{
		if (access("/sys/bus/usb/devices/1-1.1.1", 0) != 0)
		{
			err += 1;

		}
		if (access("/sys/bus/usb/devices/1-1.1.2", 0) != 0)
		{
			err += 1;

		}
		if (access("/sys/bus/usb/devices/1-1.1.3", 0) != 0)
		{
			err += 1;
		}
		if (access("/sys/bus/usb/devices/1-1.1.4", 0) != 0)
		{
			err += 1;
		}
		if (access("/sys/bus/usb/devices/1-1.1.5", 0) != 0)
		{
			err += 1;
		}
		if (access("/sys/bus/usb/devices/1-1.2.1", 0) != 0)
		{
			err += 1;
		}
		if (access("/sys/bus/usb/devices/1-1.2.2", 0) != 0)
		{
			err += 1;
		}
		if (access("/sys/bus/usb/devices/1-1.2.3", 0) != 0)
		{
			err += 1;
		}
		if (access("/sys/bus/usb/devices/1-1.2.4", 0) != 0)
		{
			err += 1;
		}
		if (access("/sys/bus/usb/devices/1-1.2.5", 0) != 0)
		{
			err += 1;
		}
		if (err != 0)
			return -1;
		return 0;
	}
	else if (port_num == 100)
	{

		if (access("/sys/bus/usb/devices/1-1.1.1", 0) != 0)
		{
			err += 1;
		}
		if (access("/sys/bus/usb/devices/1-1.1.2", 0) != 0)
		{
			err += 1;
		}

		if (access("/sys/bus/usb/devices/1-1.1.3", 0) != 0)
		{
			err += 1;
		}

		if (access("/sys/bus/usb/devices/1-1.2.1", 0) != 0)
		{
			err += 1;
		}
		if (access("/sys/bus/usb/devices/1-1.2.2", 0) != 0)
		{
			err += 1;
		}
		if (access("/sys/bus/usb/devices/1-1.2.3", 0) != 0)
		{
			err += 1;
		}
		if (access("/sys/bus/usb/devices/1-1.3.1", 0) != 0)
		{
			err += 1;
		}
		if (access("/sys/bus/usb/devices/1-1.3.2", 0) != 0)
		{
			err += 1;
		}
		if (access("/sys/bus/usb/devices/1-1.3.3", 0) != 0)
		{
			err += 1;
		}
		if (access("/sys/bus/usb/devices/1-1.4.1", 0) != 0)
		{
			err += 1;
		}
		if (access("/sys/bus/usb/devices/1-1.4.2", 0) != 0)
		{
			err += 1;
		}
		if (access("/sys/bus/usb/devices/1-1.4.3", 0) != 0)
		{
			err += 1;
		}

		if (access("/sys/bus/usb/devices/1-1.5.1", 0) != 0)
		{
			err += 1;
		}

		if (access("/sys/bus/usb/devices/1-1.5.2", 0) != 0)
		{
			err += 1;
		}
		if (access("/sys/bus/usb/devices/1-1.5.3", 0) != 0)
		{
			err += 1;
		}
		if (err != 0)
			return -1;
		return 0;
	}
	else
		return -1;
	return 0;
}
static int get_hub_mcu_status(struct _bluetooth *stream, uint8 *json_in, struct json_parm_bluetooth *parm)
{
	int usb_sum;
	//int result;
	char sendbuf[1024] = { 0 };
	char *g_buf = NULL;
	usb_sum = ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_PORT_NUMBER);


	cJSON *dir1, *dir2;		//组包使用
	cJSON *json = cJSON_CreateObject();
	cJSON_AddItemToObject(json, "head", dir1 = cJSON_CreateObject());
	cJSON_AddStringToObject(dir1, "protocol", BLUETOOTH_PROTOCOL);
	cJSON_AddStringToObject(dir1, "code_type", BLUETOOTH_SYNC_CODE_RESPONSE);
	cJSON_AddStringToObject(dir1, "cmd", parm->cmd);
	cJSON_AddStringToObject(dir1, "source_topic", parm->source_topic);
	cJSON_AddStringToObject(dir1, "random", parm->random);


	cJSON_AddStringToObject(dir1, "result", "s");

	cJSON_AddItemToObject(json, "data", dir2 = cJSON_CreateArray());
	cJSON *dir3;
	cJSON_AddItemToObject(dir2, "dira", dir3 = cJSON_CreateObject());


	if (usb_sum == 68)
	{
		if (judge_hub(68) < 0)
		{
			cJSON_AddStringToObject(dir3, "hub_status", "1");
		}
		else
		{
			cJSON_AddStringToObject(dir3, "hub_status", "0");
		}
	}
	else if (usb_sum == 4)
	{
		if (judge_hub(4) < 0)
		{
			cJSON_AddStringToObject(dir3, "hub_status", "1");
		}
		else
		{
			cJSON_AddStringToObject(dir3, "hub_status", "0");
		}
	}
	else if (usb_sum == 20)
	{
		if (judge_hub(20) < 0)
		{
			cJSON_AddStringToObject(dir3, "hub_status", "1");
		}
		else
		{
			cJSON_AddStringToObject(dir3, "hub_status", "0");
		}
	}
	else if (usb_sum == 32)
	{
		if (judge_hub(32) < 0)
		{
			cJSON_AddStringToObject(dir3, "hub_status", "1");
		}
		else
		{
			cJSON_AddStringToObject(dir3, "hub_status", "0");
		}
	}
	else if (usb_sum == 100)
	{
		if (judge_hub(100) < 0)
		{
			cJSON_AddStringToObject(dir3, "hub_status", "1");
		}
		else
		{
			cJSON_AddStringToObject(dir3, "hub_status", "0");
		}

	}
	if (ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_CONN, NULL) < 0)
	{
		cJSON_AddStringToObject(dir3, "mcu_status", "1");
	}
	else
	{
		cJSON_AddStringToObject(dir3, "mcu_status", "0");
	}
	g_buf = cJSON_PrintUnformatted(json);

	memcpy(sendbuf, "0001", 4);//协议头
	sprintf(sendbuf + 4, "%06d", strlen(g_buf));
	memcpy(sendbuf + 10, "00000001000100000000", 20);
	memcpy(sendbuf + 30, g_buf, strlen(g_buf));

	comport_send(stream, sendbuf, 30 + strlen(g_buf));
	free(g_buf);
	cJSON_Delete(json);
	return 0;

}

static int reg_ter_id(struct _bluetooth *stream, uint8 *json_in, struct json_parm_bluetooth *parm)
{
	char ter_id[13] = { 0 };
	char date[9] = { 0 };
	char err_info[1024] = { 0 };
	cJSON*root = NULL, *array_obj, *arr_item, *item;
	root = cJSON_Parse((char*)json_in);
	if (root == NULL)
	{
		return -1;
	}
	array_obj = cJSON_GetObjectItem(root, "data");
	if (array_obj == NULL)
	{
		cJSON_Delete(root);
		return -1;
	}
	arr_item = cJSON_GetArrayItem(array_obj, 0);
	if (arr_item == NULL)
	{
		cJSON_Delete(root);
		return -1;
	}
	item = cJSON_GetObjectItem(arr_item, "ter_id");
	if (item == NULL)
	{
		cJSON_Delete(root);
		return -1;
	}
	memcpy(ter_id, item->valuestring, strlen(item->valuestring));


	item = cJSON_GetObjectItem(arr_item, "date");
	if (item == NULL)
	{
		cJSON_Delete(root);
		return -1;
	}
	memcpy(date, item->valuestring, strlen(item->valuestring));

	struct _switch_dev_id   id;
	int result;
	bluetooth_out("[set_id]:设置设备ID以及生产日期\n");
	memset(&id, 0, sizeof(id));
	asc2bcd((uint8*)ter_id, id.id, 12);
	asc2bcd((uint8*)date, id.date, 4);
	asc2bcd((uint8*)date + 4, id.date + 2, 2);
	asc2bcd((uint8*)date + 6, id.date + 3, 2);
	if ((result = ty_ctl(stream->machine_fd, DF_MACHINE_INFOR_CM_SET_ID, &id))<0)
	{
		bluetooth_out("设置机器编号失败\n");
		write_log("设置机器编号失败,错误代码%d\n", result);
		get_reg_err_info(err_info);
		//
	}

	char *g_buf = NULL;
	char sendbuf[1024 * 5] = { 0 };
	//	int result;

	cJSON *dir1, *dir2;		//组包使用
	cJSON *json = cJSON_CreateObject();
	cJSON_AddItemToObject(json, "head", dir1 = cJSON_CreateObject());
	cJSON_AddStringToObject(dir1, "protocol", BLUETOOTH_PROTOCOL);
	cJSON_AddStringToObject(dir1, "code_type", BLUETOOTH_SYNC_CODE_RESPONSE);
	cJSON_AddStringToObject(dir1, "cmd", parm->cmd);
	cJSON_AddStringToObject(dir1, "source_topic", parm->source_topic);
	cJSON_AddStringToObject(dir1, "random", parm->random);
	if (result < 0 )
		cJSON_AddStringToObject(dir1, "result", "f");
	else
	
		cJSON_AddStringToObject(dir1, "result", "s");

	cJSON_AddItemToObject(json, "data", dir2 = cJSON_CreateArray());
	cJSON *dir3;
	cJSON_AddItemToObject(dir2, "dira", dir3 = cJSON_CreateObject());

	if (result < 0)
	{
		cJSON_AddStringToObject(dir3, "err_info", err_info);
	}
	g_buf = cJSON_PrintUnformatted(json);

	memcpy(sendbuf, "0001", 4);//协议头
	sprintf(sendbuf + 4, "%06d", strlen(g_buf));
	memcpy(sendbuf + 10, "00000001000100000000", 20);
	memcpy(sendbuf + 30, g_buf, strlen(g_buf));
	printf("g_buf:\n%s\n", g_buf);
	comport_send(stream, sendbuf, 30 + strlen(g_buf));
	free(g_buf);
	cJSON_Delete(root);
	cJSON_Delete(json);
	return 0;
}

static int set_ip_by_bluetooth(struct _bluetooth *stream, uint8 *json_in, struct json_parm_bluetooth *parm)
{
	cJSON *root = NULL;
	cJSON *array_obj = NULL;
	cJSON *arr_item, *item;
	int type = 0;
	char *g_buf = NULL;
	char sendbuf[1024 * 5] = { 0 };
	char ip_addr[16] = { 0 };
	char ip_msk[16] = { 0 };
	char ip_gw[16] = { 0 };
	char ip_dns[16] = { 0 };
	uint32 ip = 0, msk = 0, gw = 0, dns = 0;
	int result;


	root = cJSON_Parse((char*)json_in);
	if (root == NULL)
	{
		return -1;
	}
	array_obj = cJSON_GetObjectItem(root, "data");
	if (array_obj == NULL)
	{
		cJSON_Delete(root);
		return -1;
	}
	arr_item = cJSON_GetArrayItem(array_obj, 0);
	if (arr_item == NULL)
	{
		cJSON_Delete(root);
		return -1;
	}
	item = cJSON_GetObjectItem(arr_item, "ip_type");
	if (item == NULL)
	{
		cJSON_Delete(root);
		return -1;
	}
	type = atoi(item->valuestring);
	if (type == 0)
	{
		//dhcp

		result = ty_ctl(stream->deploy_fd, DF_DEPLOY_CM_SET_IP, ip, msk, gw, dns, type);
		if (result < 0)
		{
			cJSON_Delete(root);
			return -1;
		}
	}
	else if (type == 1)
	{
		//static
		item = cJSON_GetObjectItem(arr_item, "ip_address");
		if (item == NULL)
		{
			bluetooth_out("get ip err!\n");
			cJSON_Delete(root);
			return -1;
		}
		memcpy(ip_addr, item->valuestring, strlen(item->valuestring));
		
		item = cJSON_GetObjectItem(arr_item, "netmask");
		if (item == NULL)
		{
			bluetooth_out("get mask err!\n");
			cJSON_Delete(root);
			return -1;
		}
		memcpy(ip_msk, item->valuestring, strlen(item->valuestring));

		item = cJSON_GetObjectItem(arr_item, "gateway");
		if (item == NULL)
		{
			bluetooth_out("get  gw err!\n");
			cJSON_Delete(root);
			return -1;
		}
		memcpy(ip_gw, item->valuestring, strlen(item->valuestring));

		item = cJSON_GetObjectItem(arr_item, "dns");
		if (item == NULL)
		{
			bluetooth_out("get dns err!\n");
			cJSON_Delete(root);
			return -1;
		}
		
		memcpy(ip_dns, item->valuestring, strlen(item->valuestring));
		ip = ip_asc2ip(ip_addr, strlen(ip_addr));
		msk = ip_asc2ip(ip_msk, strlen(ip_msk));
		gw = ip_asc2ip(ip_gw, strlen(ip_gw));
		dns = ip_asc2ip(ip_dns, strlen(ip_dns));

		printf("get ip is:%s\n", ip_addr);
		printf("get msk is:%s\n", ip_msk);
		printf("get ip_gw is:%s\n", ip_gw);
		printf("get ip_dns is:%s\n", ip_dns);
		result = ty_ctl(stream->deploy_fd, DF_DEPLOY_CM_SET_IP, ip, msk, gw, dns, type);
		if (result < 0)
		{

			cJSON_Delete(root);
			return -1;
		}

	}

	cJSON *dir1, *dir2;		//组包使用
	cJSON *json = cJSON_CreateObject();
	cJSON_AddItemToObject(json, "head", dir1 = cJSON_CreateObject());
	cJSON_AddStringToObject(dir1, "protocol", BLUETOOTH_PROTOCOL);
	cJSON_AddStringToObject(dir1, "code_type", BLUETOOTH_SYNC_CODE_RESPONSE);
	cJSON_AddStringToObject(dir1, "cmd", parm->cmd);
	cJSON_AddStringToObject(dir1, "source_topic", parm->source_topic);
	cJSON_AddStringToObject(dir1, "random", parm->random);
	cJSON_AddStringToObject(dir1, "result", "s");

	cJSON_AddItemToObject(json, "data", dir2 = cJSON_CreateArray());
	cJSON *dir3;
	cJSON_AddItemToObject(dir2, "dira", dir3 = cJSON_CreateObject());
	g_buf = cJSON_PrintUnformatted(json);

	memcpy(sendbuf, "0001", 4);//协议头
	sprintf(sendbuf + 4, "%06d", strlen(g_buf));
	memcpy(sendbuf + 10, "00000001000100000000", 20);
	memcpy(sendbuf + 30, g_buf, strlen(g_buf));

	comport_send(stream, sendbuf, 30 + strlen(g_buf));
	free(g_buf);
	cJSON_Delete(json);
	cJSON_Delete(root);

	return 0;
}

static int get_terminal_info_by_bluetooth(struct _bluetooth *stream, uint8 *json_in, struct json_parm_bluetooth *parm)
{
	int result = 0;
	struct _switch_dev_id   id;
	char machine_id[13] = { 0 };
	char soft_ver[8] = { 0 };
	char mach_type[12] = { 0 };
	char *g_buf = NULL;
	struct _machine_s machine_s;
	char date[20] = { 0 };
	memset(&id, 0, sizeof(struct _switch_dev_id));
	memset(&machine_s, 0, sizeof(struct _machine_s));

	result = ty_ctl(stream->machine_fd, DF_MACHINE_INFOR_CM_GET_ID, &id);
	if (result < 0)
	{
		bluetooth_out("***打包机器编号result = %d\n", result);
	}
	//bluetooth_out("打包生产日期\n");
	bcd2asc(id.id, (uint8 *)machine_id, 6);

	memcpy(mach_type, stream->note->type, strlen(stream->note->type));

	//bluetooth_out("打包软件版本\n");
	result = ty_ctl(stream->machine_fd, DF_MACHINE_INFOR_CM_GET_SOFT, soft_ver);
	if (result < 0)
	{
		bluetooth_out("打包软件版本result = %d\n", result);
	}


	result = ty_ctl(stream->machine_fd, DF_MACHINE_INFOR_CM_GET_MACHINE_S, &machine_s);
	if (result < 0)
	{
		bluetooth_out("DF_MACHINE_INFOR_CM_GET_MACHINE_S err:%d\n", result);
	}

	memcpy(date, machine_s.r_date, 4);
	memcpy(date + 4, "-", 1);
	memcpy(date + 5, machine_s.r_date + 4, 2);
	memcpy(date + 7, "-", 1);
	memcpy(date + 8, machine_s.r_date + 6, 2);
	;
	cJSON *dir1, *dir2;		//组包使用
	cJSON *json = cJSON_CreateObject();
	cJSON_AddItemToObject(json, "head", dir1 = cJSON_CreateObject());
	cJSON_AddStringToObject(dir1, "protocol", BLUETOOTH_PROTOCOL);
	cJSON_AddStringToObject(dir1, "code_type", BLUETOOTH_SYNC_CODE_RESPONSE);
	cJSON_AddStringToObject(dir1, "cmd", parm->cmd);
	cJSON_AddStringToObject(dir1, "source_topic", parm->source_topic);
	cJSON_AddStringToObject(dir1, "random", parm->random);
	cJSON_AddStringToObject(dir1, "result", "s");

	struct _ter_info ter_info;
	memset(&ter_info, 0, sizeof(struct _ter_info));
	ty_ctl(stream->machine_fd, DF_MACHINE_INFOR_CM_GET_INFOMATION, &ter_info);

	cJSON_AddItemToObject(json, "data", dir2 = cJSON_CreateArray());

	cJSON *data_layer = cJSON_CreateObject();
	cJSON_AddItemToObject(dir2, "dirl", data_layer);

	(get_usb_port_class())->fun_get_ter_all_data_json_without_port(data_layer);

	int data_len;
	g_buf = cJSON_PrintUnformatted(json);

	bluetooth_out("查询到的数据：%s\n", g_buf);

	data_len = strlen(g_buf);
	char *compress = calloc(1, data_len*2 + 2048);
	int compress_len = data_len * 2 + 2048;
	asc_compress_base64((const unsigned char *)g_buf, data_len, (unsigned char *)compress, &compress_len);

	bluetooth_out("压缩后的数据：%s\n", compress);

	char encryption[3] = { 0 };
	strcpy(encryption, "0001");
	if (bluetooth_send_packets(stream, compress, strlen(compress), encryption) < 0)
	{
		free(g_buf);
		free(compress);
		cJSON_Delete(json);
		return BLUETOOTH_ERR_SEND;
	}
	free(g_buf);
	free(compress);
	cJSON_Delete(json);


	return 0;
}

static int control_ble_disconn(struct _bluetooth *stream, uint8 *json_in, struct json_parm_bluetooth *parm)
{
	char *g_buf = NULL;
	char sendbuf[1024 * 5] = { 0 };
//	int result;

	cJSON *dir1, *dir2;		//组包使用
	cJSON *json = cJSON_CreateObject();
	cJSON_AddItemToObject(json, "head", dir1 = cJSON_CreateObject());
	cJSON_AddStringToObject(dir1, "protocol", BLUETOOTH_PROTOCOL);
	cJSON_AddStringToObject(dir1, "code_type", BLUETOOTH_SYNC_CODE_RESPONSE);
	cJSON_AddStringToObject(dir1, "cmd", parm->cmd);
	cJSON_AddStringToObject(dir1, "source_topic", parm->source_topic);
	cJSON_AddStringToObject(dir1, "random", parm->random);
	cJSON_AddStringToObject(dir1, "result", "s");

	cJSON_AddItemToObject(json, "data", dir2 = cJSON_CreateArray());
	cJSON *dir3;
	cJSON_AddItemToObject(dir2, "dira", dir3 = cJSON_CreateObject());
	g_buf = cJSON_PrintUnformatted(json);

	memcpy(sendbuf, "0001", 4);//协议头
	sprintf(sendbuf + 4, "%06d", strlen(g_buf));
	memcpy(sendbuf + 10, "00000001000100000000", 20);
	memcpy(sendbuf + 30, g_buf, strlen(g_buf));

	comport_send(stream, sendbuf, 30 + strlen(g_buf));
	free(g_buf);
	cJSON_Delete(json);

	ble_2at_disconnect(stream);
	return 0;
}

/**********解析json 头********/
static int analysis_json_head_bluetooth(char *inbuf, struct json_parm_bluetooth *parm)
{
	int result;
	cJSON *item;
	cJSON *root = cJSON_Parse(inbuf);
	if (!root)
	{
		bluetooth_out("Error before cJSON_Parse: [%s]\n", cJSON_GetErrorPtr());
		return -1;
	}
	else
	{
		cJSON *object = cJSON_GetObjectItem(root, "head");
		if (object == NULL)
		{
			printf("Error before head: [%s]\n", cJSON_GetErrorPtr());
			cJSON_Delete(root);
			return -1;
		}
		if (object != NULL)
		{
			//bluetooth_out("获取协议版本\n");
			item = cJSON_GetObjectItem(object, "protocol");
			if (item != NULL)
			{
				//bluetooth_out("cJSON_GetObjectItem: type=%d, string is %s, valuestring=%s|\n", item->type, item->string, item->valuestring);
				memcpy(parm->protocol, item->valuestring, strlen(item->valuestring));
			}
			result = memcmp(parm->protocol, BLUETOOTH_PROTOCOL, strlen(parm->protocol));
			if (result != 0)
			{
				//bluetooth_out("协议版本不一致\n");
				cJSON_Delete(root);
				return -1;
			}
			//bluetooth_out("获取命令类型\n");
			item = cJSON_GetObjectItem(object, "code_type");
			if (item != NULL)
			{
				//bluetooth_out("cJSON_GetObjectItem: type=%d, string is %s, valuestring=%s\n", item->type, item->string, item->valuestring);
				memcpy(parm->code_type, item->valuestring, strlen(item->valuestring));
			}
			//bluetooth_out("parm->code_type = %s\n", parm->code_type);
			if (memcmp(parm->code_type, BLUETOOTH_SYNC_CODE_REQUEST, strlen(parm->code_type)) != 0)
			{
				bluetooth_out("消息命令错误\n");
				cJSON_Delete(root);
				return -1;
			}
			//bluetooth_out("获取源ID\n");
			item = cJSON_GetObjectItem(object, "source_topic");
			if (item != NULL)
			{
				//bluetooth_out("cJSON_GetObjectItem: type=%d, string is %s, valuestring=%s\n", item->type, item->string, item->valuestring);
				memcpy(parm->source_topic, item->valuestring, strlen(item->valuestring));
			}
			//bluetooth_out("获取源ID成功 ：%s\n", parm->source_topic);

			//bluetooth_out("获取命令字\n");
			item = cJSON_GetObjectItem(object, "cmd");
			if (item != NULL)
			{
				//bluetooth_out("cJSON_GetObjectItem: type=%d, string is %s, valueint=%d\n", item->type, item->string, item->valueint);
				memcpy(parm->cmd, item->valuestring, strlen(item->valuestring));

			}
			//bluetooth_out("获取本条命令的随机数\n");
			item = cJSON_GetObjectItem(object, "random");
			if (item != NULL)
			{
				memcpy(parm->random, item->valuestring, strlen(item->valuestring));
				//bluetooth_out("获取随机数成功 ： %s\n", parm->random);

			}
			else
			{
				cJSON_Delete(root);
				return -1;
			}
		}
		cJSON_Delete(root);
	}
	return 0;

}
//static int bluetooth_recv_head(struct _bluetooth *stream, struct _bluetooth_head *head)
//{
//	if (comport_recv(ble_attr,(char *)head, BULETOOTH_HEAD_LEN, 0) < 0)
//	{
//		//bluetooth_out("bluetooth_recv_head recv failed!\n");
//		return -2;
//	}
//	if (memcmp(head->ver, "0001", 4) != 0)
//	{
//		bluetooth_out("bluetooth recv version wrong :[%s]!\n\n", head->ver);
//		int i;
//		for (i = 0; i < 4; i++)
//		{
//			bluetooth_out(" %02x", head->ver[i]);
//		}
//		bluetooth_out("\n\n");
//		return -1;
//	}
//	//printf("recv head :%s\n", (char *)head);
//
//	return 0;
//}
//static int bluetooth_recv_parse_body(MyAttr *ble_attr, struct _bluetooth_head *head, struct json_parm_bluetooth *parm, char *body)
//{
//	char len_str[7] = { 0 };
//	char data_format[3] = { 0 }, encryption[3] = { 0 };
//	memcpy(data_format, head->data_format, 2);
//	memcpy(encryption, head->encryption, 2);
////	bluetooth_out("data_format:%s \t encryption:%s\n", data_format, encryption);
//	if (memcmp(head->data_format, "00", 2) == 0)//json
//	{
//		if (memcmp(head->encryption, "00", 2) == 0)//明文
//		{
//			if (memcmp(head->whole_packs_num,"0000", 4)== 0)
//			{
//				bluetooth_out("recv whole packs number is zero!\n");
//				return -1;
//			}
//			else//目前请求包只存在一包情况
//			{
//				memcpy(len_str, head->data_len, 6);
//				if (comport_recv(ble_attr, body, atoi(len_str), 0) < 0)
//				{
//					
//					bluetooth_out("recv body err is:%s\n", (char *)body);
//					return -1;
//				}
//				bluetooth_out("recv body :%s\n", (char *)body);
//				if (analysis_json_head_bluetooth(body,parm) < 0)
//				{
//					bluetooth_out("analysis_json_head_bluetooth failed!\n");
//					return -1;
//				}
//				return 0;
//			}
//		}
//		else
//		{
//			return -1;
//		}
//	}
//	else//目前只支持json格式
//	{
//		return -1;
//	}
//	return 0;
//}

/*********新接口错误返回*********/
int bluetooth_err_back(struct _bluetooth *stream, int err_no, struct json_parm_bluetooth *parm)
{
	int i;
	char errcode[10] = { 0 };
	char errinfo[100] = { 0 };
	for (i = 0; i<sizeof(ble_err) / sizeof(ble_err[0]); i++)
	{
		if (ble_err[i].err_no == err_no)
			break;
	}
	if (i == sizeof(ble_err) / sizeof(ble_err[0]))
	{
		bluetooth_out("错误代码中未能描述该代码意义\n");
		//return err_bluetooth_out_s(fd, 255, (uint8 *)"未知错误");
		sprintf(errinfo, "Unknown err!");
	}
	else
	{
		sprintf(errcode, "%d", err_no);
		sprintf(errinfo, "%s", ble_err[i].errinfo);
	}
	char *g_buf;
	cJSON *dir1, *dir2;		//组包使用
	cJSON *json = cJSON_CreateObject();
	cJSON_AddItemToObject(json, "head", dir1 = cJSON_CreateObject());
	cJSON_AddStringToObject(dir1, "protocol", BLUETOOTH_PROTOCOL);
	cJSON_AddStringToObject(dir1, "code_type", BLUETOOTH_SYNC_CODE_RESPONSE);
	if (err_no == BLUETOOTH_ERR_HEAD)
	{
	
		cJSON_AddStringToObject(dir1, "cmd", "");
		cJSON_AddStringToObject(dir1, "source_topic", "");
		cJSON_AddStringToObject(dir1, "random", "");
	}
	else
	{
		cJSON_AddStringToObject(dir1, "cmd", parm->cmd);
		cJSON_AddStringToObject(dir1, "source_topic", parm->source_topic);
		cJSON_AddStringToObject(dir1, "random", parm->random);
	}
	cJSON_AddStringToObject(dir1, "result", "f");

	cJSON_AddItemToObject(json, "data", dir2 = cJSON_CreateArray());
	cJSON *errlayer;
	cJSON_AddItemToObject(dir2, "dira", errlayer = cJSON_CreateObject());

	cJSON_AddStringToObject(errlayer, "err_code", errcode);
	cJSON_AddStringToObject(errlayer, "err_info", errinfo);
	g_buf = cJSON_PrintUnformatted(json);

	char encryption[3] = { 0 };
	memcpy(encryption, parm->head->encryption,2);
	//memcpy(encryption, "00", 2);
	bluetooth_out("蓝牙错误返回:\n%s\n", g_buf);
	bluetooth_send_packets(stream, g_buf, strlen(g_buf), encryption);
	free(g_buf);
	cJSON_Delete(json);

	return 0;
}

static void bluetooth_detect_task(void *arg, int timer)
{
	struct _bluetooth *stream;
	char sbuf[1024] = { 0 };
	char rbuf[1024] = { 0 };
	char mode_ble[2] = { 0 };
	stream = (struct _bluetooth *)arg;
	
	if (stream->ble_sta == Ble_init)//断开蓝牙时使用
	{
		bluetooth_out("蓝牙错误返回 stream->ble_sta == Ble_init\n");
		sleep(1);
		return;
	}
	if (stream->ble_sta != Ble_connect_success)
	{
		if (stream->ble_name_init != 1)
		{
			sleep(3);

			if (bluetooth_send_recv(stream, "AT+NAME?\r\n", rbuf, sizeof(rbuf)) < 0)
			{
				// printf("bluetooth name1 :%s\n", rbuf);
				return ;
			}
			bluetooth_out("bluetooth AT+NAME :%s\n", rbuf);
			if (memcmp(rbuf, stream->machine_id, strlen((char *)stream->machine_id)) != 0)//蓝牙名称和机器编号不一致
			{
				memset(sbuf, 0, sizeof(sbuf));
				// if (memcmp(stream->machine_id, "068", 3) != 0)//机器编号是初始化的????????????
				// {
				// 	char reg_ble_name[20] = { 0 };
				// 	get_reg_ble_name(reg_ble_name);
				// 	if (strlen(reg_ble_name) > 0)//获取到注册需要设置的蓝牙名称--1号端口的模拟盘盘号
				// 	{
				// 		memset(sbuf, 0, sizeof(sbuf));
				// 		printf("reg_ble_name:%s\n", reg_ble_name);
				// 		sprintf(sbuf, "AT+NAME=%s\r\n", reg_ble_name);
				// 	}
				// 	else
				// 	{

				// 		memset(sbuf, 0, sizeof(sbuf));
				// 		sprintf(sbuf, "AT+NAME=%s\r\n", "000000000000");
				// 	}

				// }
				// else//有机器编号了 如068180000001
				{
					sprintf(sbuf, "AT+NAME=%s\r\n", stream->machine_id);
				}

				if (bluetooth_send_recv(stream, sbuf, rbuf, sizeof(rbuf)) < 0)
				{
					return ;
				}
			}

			if (bluetooth_send_recv(stream, "AT+PNAME?\r\n", rbuf, sizeof(rbuf)) < 0)
			{
				return ;
			}
			bluetooth_out("bluetooth AT+PNAME :%s\n", rbuf);
			if (memcmp(rbuf, stream->machine_id, strlen((char *)stream->machine_id)) != 0)//有机器编号且蓝牙名不是机器编号
			{

				memset(sbuf, 0, sizeof(sbuf));
				memset(sbuf, 0, sizeof(sbuf));
				if (memcmp(stream->machine_id, "068", 3) != 0)
				{
					sprintf(sbuf, "AT+PNAME=%s\r\n", "000000000000");
				}
				else
					sprintf(sbuf, "AT+PNAME=%s\r\n", stream->machine_id);
				if (bluetooth_send_recv(stream, sbuf, rbuf, sizeof(rbuf)) < 0)
				{
					return ;
				}
			}


			memset(sbuf, 0, sizeof(sbuf));
			sprintf(sbuf, "AT+SYSID?\r\n");
			if (bluetooth_send_recv(stream, sbuf, rbuf, sizeof(rbuf)) < 0)
			{
				return ;
			}
			//printf("bluetooth sysid :%s\n", rbuf);
			if (memcmp(stream->machine_id, "068", 3) == 0 && memcmp(rbuf + 9, "48B", 3) != 0)//有机器编号且蓝牙SYSID是初始状态的
			{
				memset(sbuf, 0, sizeof(sbuf));
				char sysid[40] = { 0 };
				char sysid_t[40] = { 0 };
				char sysid_tt[40] = { 0 };
				int i, j;
				char tmp;//4F 77 0F 0000 E4 C2 84
				//
				//068184567234->48BB84 567234(print) ->          347256000084BB48   0 10 1 11 2 8 3 9
				memcpy(sysid_t, "48B", 3);
				for (i = 3, j = 3; i <= 11; i++)
				{
					if (stream->machine_id[i] == '1')
					{
						tmp = 'E';
					}
					else if (stream->machine_id[i] == '3')
					{
						tmp = 'B';
					}
					else if (stream->machine_id[i] == '0')
					{
						tmp = 'D';
					}
					else
					{
						tmp = stream->machine_id[i];
					}
					sysid_t[j] = tmp;
					j++;
				}
				memset(sbuf, 0, sizeof(sbuf));
				memcpy(sysid_tt, sysid_t, 6);
				memcpy(sysid_tt + 6, "0000", 4);
				memcpy(sysid_tt + 10, sysid_t + 6, 6);
				//068180000999
				//99D9DD00008DBE48
				//48BE8DDDD999
				for (i = 0, j = 1; i < 16; i += 2)
				{

					memcpy(sysid + (16 - (j * 2)), sysid_tt + i, 2);
					j++;

				}
				//printf("end id:%s\n", sysid);
				sprintf(sbuf, "AT+SYSID=%s\r\n", sysid);
				if (bluetooth_send_recv(stream, sbuf, rbuf, sizeof(rbuf)) < 0)
				{
					return ;
				}
			}

			//AT+PASEN=ON

			memset(sbuf, 0, sizeof(sbuf));
			sprintf(sbuf, "AT+PASEN=OFF\r\n");
			if (bluetooth_send_recv(stream, sbuf, rbuf, sizeof(rbuf)) < 0)
			{
				return ;
			}
			printf("AT+PASEN=OFF :%s\n", rbuf);


			memset(sbuf, 0, sizeof(sbuf));
			sprintf(sbuf, "AT+SAVE\r\n");
			if (bluetooth_send_recv(stream, sbuf, rbuf, sizeof(rbuf)) < 0)
			{
				return ;
			}
			//printf("save at result:%s\n", rbuf);

			memset(sbuf, 0, sizeof(sbuf));
			sprintf(sbuf, "AT+RESET\r\n");
			if (bluetooth_send_recv(stream, sbuf, rbuf, sizeof(rbuf)) < 0)
			{
				return ;
			}
			printf("reset bluetooth result:%s\n", rbuf);

			stream->ble_name_init = 1;
		}

		if (bluetooth_send_recv(stream, "AT+BLESTA?\r\n", rbuf, sizeof(rbuf)) < 0)
		{
			// printf("bluetooth AT+BLESTA :%s\n", rbuf);
			return ;
		}
		if (memcmp(rbuf, "05", 2) == 0)
		{
			bluetooth_out("蓝牙连接成功\n");

			mode_ble[0] = 0x01;
			if (ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_CTL_BLUETOOTH_MODE, mode_ble) < 0)
			{
				bluetooth_out("设置蓝牙透传模式失败\n");
			}
			usleep(2000);
			stream->ble_sta = Ble_connect_success;
			stream->ble_start_time = get_time_sec();
		}
		else
		{
			if (stream->ble_sta != Ble_init)//防止蓝牙断开不了
				stream->ble_sta = Ble_broadcasting;
		}
	}
	return;
}
static int ble_2at_disconnect(struct _bluetooth *stream)
{
	char mode_ble[2] = { 0 };
	//char sbuf[100] = { 0 };
	char rbuf[1024] = { 0 };
	mode_ble[0] = 0x00;
	bluetooth_out("设置蓝牙断开连接\n");
	stream->ble_sta = Ble_init;
	sleep(3);
	if (ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_CTL_BLUETOOTH_MODE, mode_ble) < 0)
	{
		bluetooth_out("设置蓝牙AT模式失败\n");
	}
	usleep(2000);
	if (bluetooth_send_recv(stream, "AT+DISCONN\r\n", rbuf, sizeof(rbuf)) < 0)
	{
		printf("bluetooth AT+DISCONN :%s\n", rbuf);
		return -1;
	}
	stream->ble_sta = Ble_broadcasting;
	return 0;
}
static void bluetooth_order_task(void *arg, int timer)
{
	int file_fd = -1;

	int data_len = 0;
	int result;
	int err_no = 0;
	struct _bluetooth_head *head;
	struct _bluetooth *stream;
	struct json_parm_bluetooth parm;
	char *body = NULL,*tmp_data = NULL;
	stream = (struct _bluetooth *)arg;
	
	int len,tmp_len = 0;
	char head_data[40] = {0};
	if (stream->ble_sta != Ble_connect_success)
	{
		return;
	}

	head = malloc(sizeof(struct _bluetooth_head));
	memset(head, 0, sizeof(struct _bluetooth_head));
	memset(&parm, 0, sizeof(struct json_parm_bluetooth));
	//uint8 buf[1024];
	//int datalen;
	//bluetooth_out("蓝牙数据接收处理线程\n");
	parm.head = head;
	for (len = 0, timer = 0; timer<5000; timer++, usleep(1000))
	{
		
		result = ty_ctl(stream->serial_fd, DF_SERIA_CM_GET);
		if (result<0)
		{
			continue;
		}
		len += ty_read(stream->serial_fd, (char *)head_data + len, BULETOOTH_HEAD_LEN - len);
		if (len<BULETOOTH_HEAD_LEN)
		{
			//bluetooth_out("读取到的数据报文头：%s\n", head);
			continue;
		}
		bluetooth_out("读取到的数据报文头：%s\n", head_data);
		int index = 0;
		memcpy(head->ver, head_data + index, 4);		index += 4;
		memcpy(head->data_len, head_data + index, 6);	index += 6;
		memcpy(head->data_format, head_data + index, 2);	index += 2;
		memcpy(head->encryption, head_data + index, 2);	index += 2;
		memcpy(head->whole_packs_num, head_data + index, 4);	index += 4;
		memcpy(head->pack_seria_num, head_data + index, 4);	index += 4;
		memcpy(head->reserved_bit, head_data + index, 8);	index += 8;

		if (memcmp(head->ver, "0001", 4) != 0)
		{
			bluetooth_out("bluetooth recv version wrong :[%s]!\n\n", head->ver);
			int i;
			for (i = 0; i < 4; i++)
			{
				bluetooth_out(" %02x", head->ver[i]);
			}
			bluetooth_out("\n\n");
			//bluetooth_err_back(stream, BLUETOOTH_ERR_PARM, &parm);
			goto err_clean ;
		}

		data_len = atoi(head->data_len);
		if (data_len <= 0)
		{
			bluetooth_err_back(stream, BLUETOOTH_ERR_HEAD, &parm);
			goto err_clean;
		}
		printf("-------------- data_len = %d\n",data_len);
		body = malloc(data_len + 1024);
		if (body == NULL)
		{
			bluetooth_err_back(stream, BLUETOOTH_ERR_SYSTEM, &parm);
			goto err_clean;
		}
		
		memset(body, 0, data_len + 1024);

		tmp_data = calloc(1,data_len + 10);
		if (tmp_data == NULL)
		{
			bluetooth_err_back(stream, BLUETOOTH_ERR_SYSTEM, &parm);
			goto err_clean;
		}
		usleep(100000);

		int body_time = 0;
		int body_len=0;
		for (body_time = 0; body_time < 10*50000*3; body_time++, usleep(1000))
		{
			memset(tmp_data,0x00,data_len + 10);
			tmp_len = ty_read(stream->serial_fd, tmp_data, data_len - body_len);
			if(tmp_len > 0)
			{
				if(tmp_len + body_len > data_len)
				{
					printf("---------------- error tmp_len = %d,body_len = %d,data_len = %d\n",tmp_len,body_len,data_len);
					free(body);
					free(tmp_data);
					goto err_clean;
				}
				else
				{
					memcpy((char *)body + body_len,tmp_data,tmp_len);
					body_len = body_len + tmp_len;
					printf("tmp_len = %d,body_len = %d,data_len = %d\ntmp_data = %s\n",tmp_len,body_len,data_len,tmp_data);
				}
			}

			// body_len += ty_read(stream->serial_fd, (char *)body + body_len, data_len - body_len);
			if (body_len >= data_len)
				break;
		}
		if (body_time == 10*50000*3)
		{
			bluetooth_out("body数据获取超时，body_len = %d,data_len = %d\n",body_len,data_len);
			bluetooth_err_back(stream, BLUETOOTH_ERR_RECV, &parm);
			free(tmp_data);
			free(body);
			goto err_clean;
		}
		printf("rec successed,body_len = %ld,tmp_len = %d,tmp_data = %s\n",body_len,tmp_len,tmp_data);
		// bluetooth_out("获取到的body数据为：%s\n", body);
		//printf("body = \n%s\n",body);

		// system("rm /home/body");
		// file_fd = open("/home/body", O_CREAT | O_RDWR);
		// if (file_fd < 0)
		// {
		// 	printf("open error\n");
		// 	return -1;
		// }
		// write(file_fd, body, body_len);
		// close(file_fd);

		printf("rec successed 111111111\n");
		if (analysis_json_head_bluetooth(body, &parm) < 0)
		{
			bluetooth_out("analysis_json_head_bluetooth failed!\n");
			bluetooth_err_back(stream, BLUETOOTH_ERR_PARM, &parm);
			free(tmp_data);
			free(body);
			goto err_clean ;
		}

		int num;
		for (num = 0; num < sizeof(bluetooth_orders) / sizeof(bluetooth_orders[0]); num++)
		{
			stream->ble_start_time = get_time_sec();//重置开始时间
			//printf("change start time:%d\n", stream->ble_start_time);
			if (memcmp(bluetooth_orders[num].order, parm.cmd, strlen(parm.cmd)) == 0)
			{
				bluetooth_out("执行:[%s]\n", bluetooth_orders[num].name);
				result = bluetooth_orders[num].answer(stream, (uint8 *)body, &parm);
				free(body);
				if (result < 0)
				{
					err_no = result;
					bluetooth_err_back(stream, err_no, &parm);
					ble_2at_disconnect(stream);
					goto  err_clean;
				}
				ble_2at_disconnect(stream);
				goto err_clean;
			}
		}
		free(tmp_data);
		free(body);
		goto err_clean;
	}
	if (timer == 5000)
	{
		bluetooth_out("接收数据超时\n");
		goto err_clean ;
	}

err_clean:

	free(head);
	int end_time = get_time_sec();
	if (end_time - stream->ble_start_time > 30)
	{
		ble_2at_disconnect(stream);
	}
	return;
}

//static int bluetooth_init(struct _bluetooth *stream, uint8 *machine_id)
//{
//	char sbuf[100] = { 0 };
//	char rbuf[1024] = { 0 };
//	printf("bluetooth_init\n");
//
//	
//	if (bluetooth_send_recv(stream, "AT+NAME?\r\n", rbuf, sizeof(rbuf)) < 0)
//	{
//		printf("bluetooth name1 :%s\n", rbuf);
//		return -1;
//	}
//	bluetooth_out("bluetooth AT+NAME :%s\n", rbuf);
//	if ( memcmp(rbuf, machine_id, strlen((char *)machine_id)) != 0)//蓝牙名称和机器编号不一致
//	{
//		memset(sbuf, 0, sizeof(sbuf));
//		if (memcmp(machine_id, "068", 3) != 0)//机器编号是初始化的????????????
//		{
//			char reg_ble_name[20] = { 0 };
//			get_reg_ble_name(reg_ble_name);
//			if (strlen(reg_ble_name) > 0)//获取到注册需要设置的蓝牙名称--1号端口的模拟盘盘号
//			{
//				memset(sbuf, 0, sizeof(sbuf));
//				printf("reg_ble_name:%s\n", reg_ble_name);
//				sprintf(sbuf, "AT+NAME=%s\r\n", reg_ble_name);
//			}
//			else
//			{
//			
//				memset(sbuf, 0, sizeof(sbuf));
//				sprintf(sbuf, "AT+NAME=%s\r\n", "000000000000");
//			}
//			
//		}
//		else//有机器编号了 如068180000001
//		{
//				sprintf(sbuf, "AT+NAME=%s\r\n", machine_id);
//		}
//			
//		if (bluetooth_send_recv(stream, sbuf, rbuf, sizeof(rbuf)) < 0)
//		{
//			return -1;
//		}
//	}
//
//	if (bluetooth_send_recv(stream, "AT+PNAME?\r\n", rbuf, sizeof(rbuf)) < 0)
//	{
//		return -1;
//	}
//	bluetooth_out("bluetooth AT+PNAME :%s\n", rbuf);
//	if (memcmp(rbuf, machine_id, strlen((char *)machine_id)) != 0)//有机器编号且蓝牙名不是机器编号
//	{
//
//		memset(sbuf, 0, sizeof(sbuf));
//		memset(sbuf, 0, sizeof(sbuf));
//		if (memcmp(machine_id, "068", 3) != 0)
//		{
//			sprintf(sbuf, "AT+PNAME=%s\r\n", "000000000000");
//		}
//		else
//			sprintf(sbuf, "AT+PNAME=%s\r\n", machine_id);
//		if (bluetooth_send_recv(stream, sbuf, rbuf, sizeof(rbuf)) < 0)
//		{
//			return -1;
//		}
//	}
//
//
//	memset(sbuf, 0, sizeof(sbuf));
//	sprintf(sbuf, "AT+SYSID?\r\n");
//	if (bluetooth_send_recv(stream, sbuf, rbuf, sizeof(rbuf)) < 0)
//	{
//		return -1;
//	}
//	printf("bluetooth sysid :%s\n", rbuf);
//	if (memcmp(machine_id, "068", 3) == 0 && memcmp(rbuf + 9, "48B", 3) != 0)//有机器编号且蓝牙SYSID是初始状态的
//	{
//		memset(sbuf, 0, sizeof(sbuf));
//		char sysid[40] = { 0 };
//		char sysid_t[40] = { 0 };
//		char sysid_tt[40] = { 0 };
//		int i, j;
//		char tmp;//4F 77 0F 0000 E4 C2 84
//		//
//		//068184567234->48BB84 567234(print) ->          347256000084BB48   0 10 1 11 2 8 3 9
//		memcpy(sysid_t, "48B", 3);
//		for (i = 3, j = 3; i <= 11; i++)
//		{
//			if (machine_id[i] == '1')
//			{
//				tmp = 'E';
//			}
//			else if (machine_id[i] == '3')
//			{
//				tmp = 'B';
//			}
//			else if (machine_id[i] == '0')
//			{
//				tmp = 'D';
//			}
//			else
//			{
//				tmp = machine_id[i];
//			}
//			sysid_t[j] = tmp;
//			j++;
//		}
//		memset(sbuf, 0, sizeof(sbuf));
//		memcpy(sysid_tt, sysid_t, 6);
//		memcpy(sysid_tt + 6, "0000", 4);
//		memcpy(sysid_tt + 10, sysid_t + 6, 6);
//		//068180000999
//		//99D9DD00008DBE48
//		//48BE8DDDD999
//		for (i = 0, j = 1; i < 16; i += 2)
//		{
//
//			memcpy(sysid + (16 - (j * 2)), sysid_tt + i, 2);
//			j++;
//
//		}
//		printf("end id:%s\n", sysid);
//		sprintf(sbuf, "AT+SYSID=%s\r\n", sysid);
//		if (bluetooth_send_recv(stream, sbuf, rbuf, sizeof(rbuf)) < 0)
//		{
//			return -1;
//		}
//	}
//
//	//AT+PASEN=ON
//
//	memset(sbuf, 0, sizeof(sbuf));
//	sprintf(sbuf, "AT+PASEN=OFF\r\n");
//	if (bluetooth_send_recv(stream, sbuf, rbuf, sizeof(rbuf)) < 0)
//	{
//		return -1;
//	}
//	printf("AT+PASEN=OFF :%s\n", rbuf);
//
//	/*memset(sbuf, 0, sizeof(sbuf));
//	sprintf(sbuf, "AT+PASS?\r\n");
//	if (bluetooth_send_recv(ble_attr, sbuf, rbuf, sizeof(rbuf)) < 0)
//	{
//	return -1;
//	}
//	printf("AT+PASS? :%s\n", rbuf);
//	if (memcmp(rbuf, "202108", 6) != 0)
//	{
//	memset(sbuf, 0, sizeof(sbuf));
//	sprintf(sbuf, "AT+PASS=202108\r\n");
//	if (bluetooth_send_recv(ble_attr, sbuf, rbuf, sizeof(rbuf)) < 0)
//	{
//	return -1;
//	}
//	printf("PASS RESULT :%s\n", rbuf);
//	}*/
//	memset(sbuf, 0, sizeof(sbuf));
//	sprintf(sbuf, "AT+SAVE\r\n");
//	if (bluetooth_send_recv(stream, sbuf, rbuf, sizeof(rbuf)) < 0)
//	{
//		return -1;
//	}
//	printf("save at result:%s\n", rbuf);
//
//	memset(sbuf, 0, sizeof(sbuf));
//	sprintf(sbuf, "AT+RESET\r\n");
//	if (bluetooth_send_recv(stream, sbuf, rbuf, sizeof(rbuf)) < 0)
//	{
//		return -1;
//	}
//	printf("reset bluetooth result:%s\n", rbuf);
//	return 0;
//}


static int comport_send(struct _bluetooth *stream, char *sbuf, int sbuf_len)
{
	bluetooth_out("串口发送数据%s\r\n", sbuf);
	ty_ctl(stream->serial_fd, DF_SERIA_CM_CLS);
	ty_write(stream->serial_fd, sbuf, sbuf_len);
	return 0;
}

static int bluetooth_send_packets(struct _bluetooth *stream, char *sbuf, int len, char *encryption)
{
	char *sendbuf = NULL;
	int after_len = len;
	int point = 0;
	int all_nums = 1;
	int now_nums = 1;
	int enc;
	enc = atoi(encryption);
	sendbuf = calloc(1, MAX_SINGLE_LEN + 128);
	if (sendbuf == NULL)
		return -1;

	if (len > MAX_SINGLE_LEN)
	{
		if (len % MAX_SINGLE_LEN)
			all_nums = (len / MAX_SINGLE_LEN) + 1;
		else
			all_nums = (len / MAX_SINGLE_LEN);
	}
	//是否分包发送
	do
	{
		if (after_len > MAX_SINGLE_LEN)
		{
			memset(sendbuf, 0, MAX_SINGLE_LEN + 128);
			memcpy(sendbuf, "0001", 4);//版本号
			sprintf((char *)(sendbuf + 4), "%06d", MAX_SINGLE_LEN);
			sprintf(sendbuf + 10, "00%02d%04d%04d00000000", enc, all_nums, now_nums);
			memcpy(sendbuf + 30, sbuf + point, MAX_SINGLE_LEN);

			if (comport_send(stream, (char *)sendbuf, MAX_SINGLE_LEN + 30) < 0)
			{
				bluetooth_out("bluetooth get terminal information send err!\n");
				free(sendbuf);
				return -1;
			}
			point += MAX_SINGLE_LEN;
			after_len -= MAX_SINGLE_LEN;
			now_nums++;
		}
		else
		{
			memset(sendbuf, 0, MAX_SINGLE_LEN + 128);
			memcpy(sendbuf, "0001", 4);//版本号
			sprintf((char *)(sendbuf + 4), "%06d", after_len);
			sprintf(sendbuf + 10, "00%02d%04d%04d00000000", enc, all_nums, now_nums);
			memcpy(sendbuf + 30, sbuf + point, after_len);
			if (comport_send(stream, (char *)sendbuf, after_len + 30) < 0)
			{
				bluetooth_out("bluetooth get terminal information send err1!\n");
				free(sendbuf);
				return -1;
			}
			after_len -= after_len;
		}
		usleep(400000);//0.4S
		//sleep(1);//裸机版本1s
	} while (after_len > 0);
	free(sendbuf);
	return 0;

}

static int bluetooth_send_recv(struct _bluetooth	*stream, char *sbuf, char *rbuf, int rbuf_len)
{

	char buf[65535] = { 0 };
	int result;
	// bluetooth_out("发送命令给STM32,并且等待返回,该部分单任务运行,命令字为:%02x\n",order);

	//_lock_set(stream->lock);
	ty_ctl(stream->serial_fd, DF_SERIA_CM_CLS);
	//printf("AT send:%s\r\n", sbuf);
	result = ty_write(stream->serial_fd, sbuf, strlen(sbuf));
	if (result<0)
	{
		printf("ty_write failed\r\n");
		//_lock_un(stream->lock);
		return result;
	}
	usleep(100000);
	int len = ty_read(stream->serial_fd, buf, sizeof(buf));
	if (len <= 0)
	{
		// printf("ty_read failed\r\n");
		//_lock_un(stream->lock);
		return -1;
	}
	//printf("AT recv:%s\r\n", buf);
	if (len < rbuf_len)
	{
		memcpy(rbuf, buf, len);
		//_lock_un(stream->lock);
		return 0;
	}
	//_lock_un(stream->lock);

	
	return -1;
}


