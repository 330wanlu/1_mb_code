#define _bluetooth_c
#include "_bluetooth.h"

int bluetooth_add(char *seria_name, const _so_note    *note, const char *switch_name)
{
	struct _bluetooth *stream;
	int result;
	stream = malloc(sizeof(struct _bluetooth));
	if (stream == NULL)
		return DF_ERR_MEM_ERR;
	memset(stream, 0, sizeof(struct _bluetooth));
	memcpy(stream->switch_name, switch_name, strlen(switch_name));
	stream->note = (_so_note *)note;
	result = ty_file_add(DF_DEV_BLUETOOTH, stream, "bluetooth", (struct _file_fuc *)&bluetooth_fuc);
	if (result<0)
		free(stream);
	return result;
}

static int bluetooth_open(struct ty_file	*file)
{
	int j;
	struct _bluetooth *stream;
	stream = file->pro_data;
	if ((stream == NULL))
		return DF_ERR_PORT_CORE_FD_NULL;
	//out("打开bluetooth模块\n");
	if (stream->state == 0)
	{
		
		stream->machine_fd = ty_open("/dev/machine", 0, 0, NULL);
		if (stream->machine_fd<0)
		{
			out("bluetooth open machine_fd failed\n");
			return -6;
		}

		stream->deploy_fd = ty_open("/dev/deploy", 0, 0, NULL);
		if (stream->deploy_fd < 0)
		{
			out("bluetooth 打开deploy失败\n");
			return -1;
		}
		stream->ty_usb_fd = ty_open("/dev/ty_usb", 0, 0, NULL);
		if (stream->ty_usb_fd<0)
		{
			out("bluetooth打开ty_usb文件失败\n");
			return stream->ty_usb_fd;
		}
		stream->event_file_fd = ty_open("/dev/event", 0, 0, NULL);

		stream->ty_usb_m_fd = ty_open("/dev/ty_usb_m", 0, 0, NULL);
		if (stream->ty_usb_m_fd<0)
		{
			out("bluetooth打开ty_usb_m_fd文件失败\n");
			return stream->ty_usb_m_fd;
		}
		stream->get_net_fd = ty_open("/dev/get_net_state", 0, 0, NULL);
		if (stream->get_net_fd < 0)
		{
			out("bluetooth打开get_net_state文件失败\n");
			return -1;
		}
		stream->switch_fd = ty_open(stream->switch_name, 0, 0, NULL);
		if (stream->switch_fd < 0)
		{
			out("bluetooth打开switch_fd文件失败\n");
			return -1;
		}
		ty_ctl(stream->switch_fd, DF_SWITCH_CM_GET_SUPPORT, &stream->dev_support);
		if (stream->dev_support.support_ble == 0)//不支持蓝牙
			return 0;



		uint8 mode_ble[2] = { 0 };
		mode_ble[0] = 0x00;

		if (ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_BLUETOOTH_POWER, "\x00") < 0)
		{
			out("关闭蓝牙电源失败\n");
		}
		sleep(1);
		if (ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_BLUETOOTH_POWER, "\x01") < 0)
		{
			out("打开蓝牙电源失败\n");
		}
		
		if (ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_CTL_BLUETOOTH_MODE, mode_ble) < 0)
		{
			out("设置蓝牙AT模式失败\n");
		}


		struct _switch_dev_id   id;
		char machine_id[16] = { 0 };
		memset(&id, 0, sizeof(id));
		ty_ctl(stream->machine_fd, DF_MACHINE_INFOR_CM_GET_ID, &id);
		bcd2asc(id.id, (uint8 *)machine_id, 6);
		memcpy(stream->machine_id, machine_id, 12);
		if (memcmp(machine_id, "068", 3) != 0)
		{
			while (get_module_state() < 0)//等待ty_usb_m模块完成
			{
				//printf(" Wait for USB finished, continue!\n");
				sleep(1);
			}
		}
		stream->serial_fd = ty_open("/dev/seria1", 0, 0, NULL);
		if (stream->serial_fd<0)
		{
			out("打开串口[%s]错误\n", "/dev/seria1");
			return stream->serial_fd;
		}
		out("设置串口波特率[%d]\n", 115200);
		ty_ctl(stream->serial_fd, DF_SERIA_CM_BPS, 115200);
		_task_open("bluetooth_task", bluetooth_detect_task, stream, 300, NULL);
		_task_open("bluetooth_task", bluetooth_order_task, stream, 100, NULL);
	}
	for (j = 0; j<sizeof(stream->fd) / sizeof(stream->fd[0]); j++)
	{
		if (stream->fd[j].state == 0)
			break;
	}
	if (j == sizeof(stream->fd) / sizeof(stream->fd[0]))
	{
		return DF_ERR_PORT_CORE_SLAVE_SPACE;
	}
	memset(&stream->fd[j], 0, sizeof(stream->fd[0]));
	stream->fd[j].dev = stream;
	stream->fd[j].state = 1;
	stream->fd[j].mem_fd = &file->fd[j];
	stream->state++;
	out("打开bluetooth成功:j=%d\n", j);
	return j + 1;
}

static int bluetooth_ctl(void *data, int fd, int cm, va_list args)
{	
	struct _bluetooth  *stream;
	struct _bluetooth_fd   *id;
	int i;
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
			return ctl_fun[i].ctl(id, args);
	}
	return DF_ERR_PORT_CORE_CM;
}
static int bluetooth_close(void *data, int fd)
{
	return 0;
}



/*扫描wifi列表*/
//wpa_passphrase  234 12345678 | tee -a /etc/wpa_supplicant.conf  往配置文件中添加wifi配置
//wpa_supplicant -iwlan0 -Dnl80211 -c /etc/wpa_supplicant.conf & 开启wifi
static int scanning_wifi_list(struct _bluetooth *stream, uint8 *json_in, struct json_parm_bluetooth *parm)
{
	char *g_buf;
	char sendbuf[1024 * 5] = { 0 };
	char r_buf[8192] = { 0 };
	system("ifconfig wlan0 up");
	mb_system_read("iwlist wlan0 scan | grep \"ESSID\\|Quality\"", r_buf, sizeof(r_buf),"/tmp/iwlist_result.txt");

	out("iwlist : %s\n", r_buf);
	
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



	char wifi_ssid[50] = {0};
	char signal_level[20] = {0};
	signed int level;
	char *p_index, *p_start, *p_end;
	p_index = strstr(r_buf, "Signal level=");
	if (p_index == NULL)
	{
		out("scanning_wifi_list wifi搜索可用列表失败\n");
		return BLUETOOTH_ERR_WIFI_SCAN_WIFI_ERR;
	}
	for (;;)
	{
		p_start = strstr(p_index, "Signal level=");
		if (p_start == NULL)
			break;
		p_start += strlen("Signal level=");
		p_end = strstr(p_start, " dBm");
		if (p_end == NULL)
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
			out("ssid为空,直接过滤\n");
			continue;
		}

		cJSON *dir3;
		cJSON_AddItemToObject(dir2, "dira", dir3 = cJSON_CreateObject());
		cJSON_AddStringToObject(dir3, "ssid", wifi_ssid);
		cJSON_AddStringToObject(dir3, "signal_level", signal_level);
		continue;
	}
	g_buf = cJSON_PrintUnformatted(json);

	//out("返回信息：%s\n",g_buf);
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


static int set_wifi_config(struct _bluetooth *stream, uint8 *json_in, struct json_parm_bluetooth *parm)
{
	cJSON *root = NULL;
	cJSON *array_obj = NULL;
	cJSON *arr_item, *item;
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

	out("设置WIFIjson数据：%s\n", json_in);
	//int eth0_ok = get_socket_net_state("eth0");
	//if (eth0_ok == 1)
	//{
	//	out("设置wifi前请先拔插网线");
	//	return BLUETOOTH_ERR_WIFI_SET_LAN_ERR;
	//}


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
	//out("gbk:wifi ssid:%s\n", wifi_name);
	printf("utf8:wifi ssid:%s\n", wifi_name);

	if (strlen(wifi_name) > 32)
	{
		out("SSID长度有误\n");
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
		out("wifi长度有误\n");
		cJSON_Delete(root);
		return BLUETOOTH_ERR_PARM;
	}
	out("wifi passwd:%s\n", wifi_pwd);

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
			out("IP长度有误\n");
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
			out("netmask长度有误\n");
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
			out("gateway长度有误\n");
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
			out("dns长度有误\n");
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

	char s_buf[2048] = { 0 };
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
		sprintf(s_buf + strlen(s_buf), "route add default gw %s\n", gateway);

		
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

	//out("gbk:wifi ssid config:%s\n", wifi_config_path);
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
		out("wifi服务执行开始\n");

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
			out("读取临时文件：%s\n", cmd_buf);
			if (strstr(cmd_buf, "CTRL-EVENT-CONNECTED") != NULL)
			{
				break;
			}
			sleep(1);
		}
		//mb_system_read_sleep("wpa_supplicant -iwlan0 -Dnl80211 -c /etc/wpa_supplicant.conf", cmd_buf, sizeof(cmd_buf),20);

		out("wifi服务执行返回数据：%s\n", cmd_buf);

		if (strstr(cmd_buf, "CTRL-EVENT-CONNECTED") == NULL)
		{
			ty_ctl(stream->get_net_fd, DF_SET_NET_WIFI_USED, 0);
			delete_file(wifi_config_path);
			out("wifi服务执行失败\n");
			return BLUETOOTH_ERR_WIFI_SET_PASSWD_ERR;
		}
		if (net_type == 2)
		{
			system("killall udhcpc");
			memset(cmd, 0, sizeof(cmd));
			sprintf(cmd, "ifconfig wlan0 %s netmask %s", ip, netmask);
			system(cmd);

			memset(cmd, 0, sizeof(cmd));
			sprintf(cmd, "route add default gw %s", gateway);
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
	//out("返回信息：%s\n", g_buf);
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


	char now_ssid[50] = { 0 };
	find_now_wifi_ssid(now_ssid);

	int eth0_ok = get_socket_net_state("eth0");

	char wifi_ssid[50] = { 0 };

	
	DIR *dir = NULL;
	struct dirent *entry;
	if ((dir = opendir("/etc/wifi_config")) == NULL) 
	{
		out("打开/etc/wifi_config文件夹失败，不存在配置文件");
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
	//out("返回信息：%s\n", g_buf);
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
		out("SSID长度有误\n");
		cJSON_Delete(root);
		return BLUETOOTH_ERR_PARM;
	}

	char wifi_path[200] = {0};
	sprintf(wifi_path, "/etc/wifi_config/wifi_%s.sh", wifi_name);
	//printf("utf8 wifi_path :%s",wifi_path);
	delete_file(wifi_path);

	char now_ssid[50] = { 0 };
	find_now_wifi_ssid(now_ssid);
	if (strcmp(now_ssid, wifi_name)==0)
		system("killall wpa_supplicant");
	
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
	//out("返回信息：%s\n", g_buf);
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

	sleep(2);
	result = ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_RESTORE, NULL);
	if (result < 0)
	{
		out("***蓝牙命令回出厂失败\n");
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
	//out("蓝牙句柄:%d\n", stream->ble_attr.fd);
	if (ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_CTL_BLUETOOTH_MODE, mode_ble) < 0)
	{
		out("设置蓝牙AT模式失败\n");
	}
	usleep(2000);
	if (bluetooth_send_recv(stream, "AT+DISCONN\r\n", rbuf, sizeof(rbuf)) < 0)
	{
		out("蓝牙断开链接失败:%s\n", rbuf);
		return -1;
	}
	sprintf(sbuf, "AT+NAME=%s\r\n", ble_name);
	if (bluetooth_send_recv(stream, sbuf, rbuf, sizeof(rbuf)) < 0)
	{
		out("设置蓝牙名称结果失败:%s\n", rbuf);
		return -1;
	}
	out("设置蓝牙名称结果:%s\n", rbuf);

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
	//out("该终端一共有%d个USB端口\n");
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
		//return err_out_tcp(fd, -99);
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
	//out("该终端一共有%d个USB端口\n");
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
			//out("%d号端口没有设备\n", port_count + 1);
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
		out("获取唯一标识符失败\n");
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
	out("[set_id]:设置设备ID以及生产日期\n");
	memset(&id, 0, sizeof(id));
	asc2bcd((uint8*)ter_id, id.id, 12);
	asc2bcd((uint8*)date, id.date, 4);
	asc2bcd((uint8*)date + 4, id.date + 2, 2);
	asc2bcd((uint8*)date + 6, id.date + 3, 2);
	if ((result = ty_ctl(stream->machine_fd, DF_MACHINE_INFOR_CM_SET_ID, &id))<0)
	{
		out("设置机器编号失败\n");
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
			out("get ip err!\n");
			cJSON_Delete(root);
			return -1;
		}
		memcpy(ip_addr, item->valuestring, strlen(item->valuestring));
		
		item = cJSON_GetObjectItem(arr_item, "netmask");
		if (item == NULL)
		{
			out("get mask err!\n");
			cJSON_Delete(root);
			return -1;
		}
		memcpy(ip_msk, item->valuestring, strlen(item->valuestring));

		item = cJSON_GetObjectItem(arr_item, "gateway");
		if (item == NULL)
		{
			out("get  gw err!\n");
			cJSON_Delete(root);
			return -1;
		}
		memcpy(ip_gw, item->valuestring, strlen(item->valuestring));

		item = cJSON_GetObjectItem(arr_item, "dns");
		if (item == NULL)
		{
			out("get dns err!\n");
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
		out("***打包机器编号result = %d\n", result);
	}
	//out("打包生产日期\n");
	bcd2asc(id.id, (uint8 *)machine_id, 6);

	memcpy(mach_type, stream->note->type, strlen(stream->note->type));

	//out("打包软件版本\n");
	result = ty_ctl(stream->machine_fd, DF_MACHINE_INFOR_CM_GET_SOFT, soft_ver);
	if (result < 0)
	{
		out("打包软件版本result = %d\n", result);
	}


	result = ty_ctl(stream->machine_fd, DF_MACHINE_INFOR_CM_GET_MACHINE_S, &machine_s);
	if (result < 0)
	{
		out("DF_MACHINE_INFOR_CM_GET_MACHINE_S err:%d\n", result);
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

	fun_get_ter_all_data_json_without_port(data_layer);

	int data_len;
	g_buf = cJSON_PrintUnformatted(json);

	out("查询到的数据：%s\n", g_buf);

	data_len = strlen(g_buf);
	char *compress = calloc(1, data_len*2 + 2048);
	int compress_len = data_len * 2 + 2048;
	asc_compress_base64((const unsigned char *)g_buf, data_len, (unsigned char *)compress, &compress_len);

	out("压缩后的数据：%s\n", compress);

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
		out("Error before cJSON_Parse: [%s]\n", cJSON_GetErrorPtr());
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
			//out("获取协议版本\n");
			item = cJSON_GetObjectItem(object, "protocol");
			if (item != NULL)
			{
				//out("cJSON_GetObjectItem: type=%d, string is %s, valuestring=%s|\n", item->type, item->string, item->valuestring);
				memcpy(parm->protocol, item->valuestring, strlen(item->valuestring));
			}
			result = memcmp(parm->protocol, BLUETOOTH_PROTOCOL, strlen(parm->protocol));
			if (result != 0)
			{
				//out("协议版本不一致\n");
				cJSON_Delete(root);
				return -1;
			}
			//out("获取命令类型\n");
			item = cJSON_GetObjectItem(object, "code_type");
			if (item != NULL)
			{
				//out("cJSON_GetObjectItem: type=%d, string is %s, valuestring=%s\n", item->type, item->string, item->valuestring);
				memcpy(parm->code_type, item->valuestring, strlen(item->valuestring));
			}
			//out("parm->code_type = %s\n", parm->code_type);
			if (memcmp(parm->code_type, BLUETOOTH_SYNC_CODE_REQUEST, strlen(parm->code_type)) != 0)
			{
				out("消息命令错误\n");
				cJSON_Delete(root);
				return -1;
			}
			//out("获取源ID\n");
			item = cJSON_GetObjectItem(object, "source_topic");
			if (item != NULL)
			{
				//out("cJSON_GetObjectItem: type=%d, string is %s, valuestring=%s\n", item->type, item->string, item->valuestring);
				memcpy(parm->source_topic, item->valuestring, strlen(item->valuestring));
			}
			//out("获取源ID成功 ：%s\n", parm->source_topic);

			//out("获取命令字\n");
			item = cJSON_GetObjectItem(object, "cmd");
			if (item != NULL)
			{
				//out("cJSON_GetObjectItem: type=%d, string is %s, valueint=%d\n", item->type, item->string, item->valueint);
				memcpy(parm->cmd, item->valuestring, strlen(item->valuestring));

			}
			//out("获取本条命令的随机数\n");
			item = cJSON_GetObjectItem(object, "random");
			if (item != NULL)
			{
				memcpy(parm->random, item->valuestring, strlen(item->valuestring));
				//out("获取随机数成功 ： %s\n", parm->random);

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
//		//out("bluetooth_recv_head recv failed!\n");
//		return -2;
//	}
//	if (memcmp(head->ver, "0001", 4) != 0)
//	{
//		out("bluetooth recv version wrong :[%s]!\n\n", head->ver);
//		int i;
//		for (i = 0; i < 4; i++)
//		{
//			out(" %02x", head->ver[i]);
//		}
//		out("\n\n");
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
////	out("data_format:%s \t encryption:%s\n", data_format, encryption);
//	if (memcmp(head->data_format, "00", 2) == 0)//json
//	{
//		if (memcmp(head->encryption, "00", 2) == 0)//明文
//		{
//			if (memcmp(head->whole_packs_num,"0000", 4)== 0)
//			{
//				out("recv whole packs number is zero!\n");
//				return -1;
//			}
//			else//目前请求包只存在一包情况
//			{
//				memcpy(len_str, head->data_len, 6);
//				if (comport_recv(ble_attr, body, atoi(len_str), 0) < 0)
//				{
//					
//					out("recv body err is:%s\n", (char *)body);
//					return -1;
//				}
//				out("recv body :%s\n", (char *)body);
//				if (analysis_json_head_bluetooth(body,parm) < 0)
//				{
//					out("analysis_json_head_bluetooth failed!\n");
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
		out("错误代码中未能描述该代码意义\n");
		//return err_out_s(fd, 255, (uint8 *)"未知错误");
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
	out("蓝牙错误返回:\n%s\n", g_buf);
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
				printf("bluetooth name1 :%s\n", rbuf);
				return ;
			}
			//out("bluetooth AT+NAME :%s\n", rbuf);
			if (memcmp(rbuf, stream->machine_id, strlen((char *)stream->machine_id)) != 0)//蓝牙名称和机器编号不一致
			{
				memset(sbuf, 0, sizeof(sbuf));
				if (memcmp(stream->machine_id, "068", 3) != 0)//机器编号是初始化的????????????
				{
					char reg_ble_name[20] = { 0 };
					get_reg_ble_name(reg_ble_name);
					if (strlen(reg_ble_name) > 0)//获取到注册需要设置的蓝牙名称--1号端口的模拟盘盘号
					{
						memset(sbuf, 0, sizeof(sbuf));
						printf("reg_ble_name:%s\n", reg_ble_name);
						sprintf(sbuf, "AT+NAME=%s\r\n", reg_ble_name);
					}
					else
					{

						memset(sbuf, 0, sizeof(sbuf));
						sprintf(sbuf, "AT+NAME=%s\r\n", "000000000000");
					}

				}
				else//有机器编号了 如068180000001
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
			//out("bluetooth AT+PNAME :%s\n", rbuf);
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
			//printf("AT+PASEN=OFF :%s\n", rbuf);


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
			//printf("reset bluetooth result:%s\n", rbuf);

			stream->ble_name_init = 1;
		}

		if (bluetooth_send_recv(stream, "AT+BLESTA?\r\n", rbuf, sizeof(rbuf)) < 0)
		{
			//printf("bluetooth AT+BLESTA :%s\n", rbuf);
			return ;
		}
		if (memcmp(rbuf, "05", 2) == 0)
		{
			out("蓝牙连接成功\n");

			mode_ble[0] = 0x01;
			if (ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_CTL_BLUETOOTH_MODE, mode_ble) < 0)
			{
				out("设置蓝牙透传模式失败\n");
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
	out("设置蓝牙断开连接\n");
	stream->ble_sta = Ble_init;
	sleep(3);
	if (ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_CTL_BLUETOOTH_MODE, mode_ble) < 0)
	{
		out("设置蓝牙AT模式失败\n");
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
	int data_len = 0;
	int result;
	int err_no = 0;
	struct _bluetooth_head *head;
	struct _bluetooth *stream;
	struct json_parm_bluetooth parm;
	char *body = NULL;
	stream = (struct _bluetooth *)arg;
	
	int len;
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
	//out("蓝牙数据接收处理线程\n");
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
			//out("读取到的数据报文头：%s\n", head);
			continue;
		}
		out("读取到的数据报文头：%s\n", head_data);
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
			out("bluetooth recv version wrong :[%s]!\n\n", head->ver);
			int i;
			for (i = 0; i < 4; i++)
			{
				out(" %02x", head->ver[i]);
			}
			out("\n\n");
			//bluetooth_err_back(stream, BLUETOOTH_ERR_PARM, &parm);
			goto err_clean ;
		}

		data_len = atoi(head->data_len);
		if (data_len <= 0)
		{
			bluetooth_err_back(stream, BLUETOOTH_ERR_HEAD, &parm);
			goto err_clean;
		}
		body = malloc(data_len + 10);
		if (body == NULL)
		{
			bluetooth_err_back(stream, BLUETOOTH_ERR_SYSTEM, &parm);
			goto err_clean;
		}
		memset(body, 0, data_len + 10);
		usleep(100000);

		int body_time = 0;
		int body_len=0;
		for (body_time = 0; body_time < 1000; body_time++, usleep(1000))
		{
			body_len += ty_read(stream->serial_fd, (char *)body + body_len, data_len - body_len);
			if (body_len == data_len)
				break;
		}
		if (body_time == 1000)
		{
			out("body数据获取超时\n");
			bluetooth_err_back(stream, BLUETOOTH_ERR_RECV, &parm);
			free(body);
			goto err_clean;
		}
		out("获取到的body数据为：%s\n", body);

		if (analysis_json_head_bluetooth(body, &parm) < 0)
		{
			out("analysis_json_head_bluetooth failed!\n");
			bluetooth_err_back(stream, BLUETOOTH_ERR_PARM, &parm);
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
				out("执行:[%s]\n", bluetooth_orders[num].name);
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
		free(body);
		goto err_clean;
	}
	if (timer == 5000)
	{
		out("接收数据超时\n");
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
//	out("bluetooth AT+NAME :%s\n", rbuf);
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
//	out("bluetooth AT+PNAME :%s\n", rbuf);
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
	out("串口发送数据%s\r\n", sbuf);
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
				out("bluetooth get terminal information send err!\n");
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
				out("bluetooth get terminal information send err1!\n");
				free(sendbuf);
				return -1;
			}
			after_len -= after_len;
		}
		usleep(400000);//0.4S
		//sleep(1);//逻辑版本1s
	} while (after_len > 0);
	free(sendbuf);
	return 0;

}

static int bluetooth_send_recv(struct _bluetooth	*stream, char *sbuf, char *rbuf, int rbuf_len)
{

	char buf[65535] = { 0 };
	int result;
	// out("发送命令给STM32,并且等待返回,该部分单任务运行,命令字为:%02x\n",order);

	//_lock_set(stream->lock);
	ty_ctl(stream->serial_fd, DF_SERIA_CM_CLS);
	//printf("AT send:%s\r\n", sbuf);
	result = ty_write(stream->serial_fd, sbuf, strlen(sbuf));
	if (result<0)
	{
		//_lock_un(stream->lock);
		return result;
	}
	usleep(100000);
	int len = ty_read(stream->serial_fd, buf, sizeof(buf));
	if (len <= 0)
	{
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


