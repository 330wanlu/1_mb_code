#define _deal_cmd_newshare_c
#include "_deal_cmd_newshare.h"

int inv_lock;
int init_parm(const _so_note    *note)
{
	memset(&ccomm_stream, 0, sizeof(struct json_parm_new_share));

	ccomm_stream.deal_cmd_fd.note = (_so_note *)note;

	ccomm_stream.deal_cmd_fd.machine_fd = ty_open("/dev/machine", 0, 0, NULL);
	if (ccomm_stream.deal_cmd_fd.machine_fd<0)
	{
		out("init_parm打开machine文件失败\n");
		return ccomm_stream.deal_cmd_fd.machine_fd;
	}
	ccomm_stream.deal_cmd_fd.ty_usb_fd = ty_open("/dev/ty_usb", 0, 0, NULL);
	if (ccomm_stream.deal_cmd_fd.ty_usb_fd<0)
	{
		out("init_parm打开ty_usb文件失败\n");
		return ccomm_stream.deal_cmd_fd.ty_usb_fd;
	}
	ccomm_stream.deal_cmd_fd.deploy_fd = ty_open("/dev/deploy", 0, 0, NULL);
	if (ccomm_stream.deal_cmd_fd.deploy_fd < 0)
	{
		out("init_parm打开配置文件失败\n");
		return -1;
	}

	ccomm_stream.deal_cmd_fd.get_net_fd = ty_open("/dev/get_net_state", 0, 0, NULL);
	if (ccomm_stream.deal_cmd_fd.get_net_fd < 0)
	{
		out("init_parm打开get_net_state文件失败\n");
		return -1;
	}
	ccomm_stream.deal_cmd_fd.inv_read_lock = _lock_open("deal_new_cmd.c", NULL);
	inv_lock = _lock_open("new_inv.c", NULL);
	ccomm_stream.deal_cmd_fd.event_file_fd = ty_open("/dev/event", 0, 0, NULL);
	ccomm_stream.deal_cmd_fd.state = 1;
	return 0;
}




/*****设置终端功能启用停用*******/
static int set_ter_func(int sockfd, uint8 *json_in, struct json_parm_new_share *parm, void *output)
{
	cJSON *root = NULL, *array_obj, *arr_item, *item;
	root = cJSON_Parse((char*)json_in);
	if (root == NULL)
	{
		return -1;
	}
	array_obj = cJSON_GetObjectItem(root, "data");
	if (array_obj == NULL)
	{

		return -1;
	}
	arr_item = cJSON_GetArrayItem(array_obj, 0);
	if (arr_item == NULL)
	{

		return -1;
	}
	item = cJSON_GetObjectItem(arr_item, "eth_enable");
	if (item == NULL)
	{

		return -1;
	}
	if (memcmp(item->valuestring, "1", 1) == 0)//网线开启
	{
		
	}
	else
	{

	}


	item = cJSON_GetObjectItem(arr_item, "wifi_enable");
	if (item == NULL)
	{

		return -1;
	}
	if (memcmp(item->valuestring, "1", 1) == 0)//无线开启
	{

	}
	else
	{

	}


	item = cJSON_GetObjectItem(arr_item, "4g_enable");
	if (item == NULL)
	{

		return -1;
	}
	if (memcmp(item->valuestring, "1", 1) == 0)//4g开启
	{

	}
	else
	{

	}

	item = cJSON_GetObjectItem(arr_item, "mysql_enable");
	if (item == NULL)
	{

		return -1;
	}
	if (memcmp(item->valuestring, "1", 1) == 0)//msql开启
	{

	}
	else
	{

	}

	item = cJSON_GetObjectItem(arr_item, "ftp_enable");
	if (item == NULL)
	{

		return -1;
	}
	if (memcmp(item->valuestring, "1", 1) == 0)//ftp开启
	{

	}
	else
	{

	}

	item = cJSON_GetObjectItem(arr_item, "ssh_enable");
	if (item == NULL)
	{

		return -1;
	}
	if (memcmp(item->valuestring, "1", 1) == 0)//ssh开启
	{

	}
	else
	{

	}


	item = cJSON_GetObjectItem(arr_item, "ntpdate_enable");
	if (item == NULL)
	{

		return -1;
	}
	if (memcmp(item->valuestring, "1", 1) == 0)//ntp开启
	{

	}
	else
	{

	}

	item = cJSON_GetObjectItem(arr_item, "tf_card_enable");
	if (item == NULL)
	{

		return -1;
	}
	if (memcmp(item->valuestring, "1", 1) == 0)//tfcard开启
	{

	}
	else
	{

	}

	item = cJSON_GetObjectItem(arr_item, "event_log_enable");
	if (item == NULL)
	{

		return -1;
	}
	if (memcmp(item->valuestring, "1", 1) == 0)//log开启
	{

	}
	else
	{

	}

	item = cJSON_GetObjectItem(arr_item, "usbshare_enable");
	if (item == NULL)
	{

		return -1;
	}
	if (memcmp(item->valuestring, "1", 1) == 0)//USBSHARE开启
	{

	}
	else
	{

	}

	item = cJSON_GetObjectItem(arr_item, "device_manage_enable");
	if (item == NULL)
	{

		return -1;
	}
	if (memcmp(item->valuestring, "1", 1) == 0)//设备管理开启
	{

	}
	else
	{

	}

	item = cJSON_GetObjectItem(arr_item, "mqtt_enable");
	if (item == NULL)
	{

		return -1;
	}
	if (memcmp(item->valuestring, "1", 1) == 0)//mqtt开启
	{

	}
	else
	{

	}

	item = cJSON_GetObjectItem(arr_item, "tax_invoice_enable");
	if (item == NULL)
	{

		return -1;
	}
	if (memcmp(item->valuestring, "1", 1) == 0)//税务开启
	{

	}
	else
	{

	}

	item = cJSON_GetObjectItem(arr_item, "extend_space_enable");
	if (item == NULL)
	{

		return -1;
	}
	if (memcmp(item->valuestring, "1", 1) == 0)//磁盘扩展支持开启
	{

	}
	else
	{

	}

	item = cJSON_GetObjectItem(arr_item, "careader_enable");
	if (item == NULL)
	{

		return -1;
	}
	if (memcmp(item->valuestring, "1", 1) == 0)//CA证书读取开启
	{

	}
	else
	{

	}

	item = cJSON_GetObjectItem(arr_item, "userload_enable");
	if (item == NULL)
	{

		return -1;
	}
	if (memcmp(item->valuestring, "1", 1) == 0)//用户登录管理系统开启
	{

	}
	else
	{

	}
	return 0;
}
/*****重启终端*******/
static int restart_ter(int sockfd, uint8 *json_in, struct json_parm_new_share *parm, void *output)
{

	unsigned char sendbuf[1024] = { 0 };
	char *g_buf = NULL;
	int len,result;
	ty_ctl(parm->deal_cmd_fd.machine_fd, DF_MACHINE_INFOR_CM_RESET_POWER, DF_EVENT_REASON_NET);


	cJSON *dir1, *dir2;		//组包使用
	cJSON *json = cJSON_CreateObject();
	cJSON_AddItemToObject(json, "head", dir1 = cJSON_CreateObject());
	cJSON_AddStringToObject(dir1, "protocol", NEW_USB_SHARE_PROTOCOL);
	cJSON_AddStringToObject(dir1, "code_type", NEW_USB_SHARE_SYNC_CODE_RESPONSE);
	cJSON_AddStringToObject(dir1, "cmd", parm->cmd);
	cJSON_AddStringToObject(dir1, "source_topic", parm->source_topic);
	cJSON_AddStringToObject(dir1, "random", parm->random);
	cJSON_AddStringToObject(dir1, "result", "s");

	cJSON_AddItemToObject(json, "data", dir2 = cJSON_CreateArray());
	cJSON *dir3;
	cJSON_AddItemToObject(dir2, "dira", dir3 = cJSON_CreateObject());
	g_buf = cJSON_Print(json);

	memcpy(sendbuf, "\x01\x06\x00\x07", 4);//协议头
	//长度位

	sendbuf[4] = strlen(g_buf) / 0x1000000; sendbuf[5] = strlen(g_buf) / 0x10000;
	sendbuf[6] = strlen(g_buf) / 256; sendbuf[7] = strlen(g_buf) % 256;
	memcpy(sendbuf + 8, g_buf, strlen(g_buf));
	len = strlen(g_buf) + 8;

	free(g_buf);
	cJSON_Delete(json);

	result = ty_socket_write(sockfd, sendbuf, len);
	if (result < 0)
		return result;
	return 0;
}
/*****申请端口使用权*****/
static int apply_port_use(int sockfd, uint8 *json_in, struct json_parm_new_share *parm, void *output)
{
	//struct _usb_pro usb;
	int result;
	//uint8 s_buf[200];
	//char client[64];
	cJSON *root, *array_obj, *arr_item, *item;
	int line_type = 0,trans_port = 0;
	//char trans_server[100] = { 0 };
	root = cJSON_Parse((char*)json_in);
	if (root == NULL)
	{
		return -1;
	}
	array_obj = cJSON_GetObjectItem(root, "data");
	if (array_obj == NULL)
	{

		return -1;
	}
	arr_item = cJSON_GetArrayItem(array_obj, 0);
	if (arr_item == NULL)
	{

		return -1;
	}
	item = cJSON_GetObjectItem(arr_item, "line_type");
	if (item == NULL)
	{

		return -1;
	}
	line_type = atoi(item->valuestring);
	item = cJSON_GetObjectItem(arr_item, "usb_port");
	if (item == NULL)
	{

		return -1;
	}
	memcpy((char *)output, item->valuestring, strlen(item->valuestring));
	trans_port = atoi(item->valuestring);

	
	if (line_type == 1)//进行外网转发
	{
		//out("line_type  == 1\n");
		//parm->deal_cmd_fd.deploy_fd = ty_open("/dev/deploy", 0, 0, NULL);
		//if (parm->deal_cmd_fd.deploy_fd<0)
		//{
		//	out("打开配置文件失败\n");
		//	return parm->deal_cmd_fd.deploy_fd;
		//}
		////printf("======depioy file open success======\n");
		//start = ty_ctl(parm->deal_cmd_fd.deploy_fd, DF_DEPLOY_CM_GET_START_PORT);//获取机柜的起始端口号
		//if (start<0)
		//{
		//	return -1;
		//}
		//
		//app_fd = 0;
		//out("[_app.c]按端口号打开端口\n");
		//memset(client, 0, sizeof(client));//客户端标识符GUID
		//parm->deal_cmd_fd.ty_usb_m_fd = ty_open("/dev/ty_usb_m", 0, 0, NULL);
		//if (parm->deal_cmd_fd.ty_usb_m_fd<0)
		//{
		//	out("打开ty_usb_m文件失败，result = %d\n", parm->deal_cmd_fd.ty_usb_m_fd);
		//	return parm->deal_cmd_fd.ty_usb_m_fd;
		//}
		//if ((result = ty_ctl(parm->deal_cmd_fd.ty_usb_m_fd, DF_TY_USB_M_CM_OPEN_PORT_IP, trans_port, 0xC0A800FA, client, &app_fd))<0)
		//{
		//	out("打开端口失败\n");
		//	return -1;
		//}
		logout(INFO, "tcp", "attach", "deal_cmd_newshare,加载USB端口,%d\r\n", trans_port);
		int app_fd;
		char client[128] = {0};
		result = open_port_attach(trans_port, 0x00, client, &app_fd);
		if (result < 0)
		{
			int err_no;
			if (result == DF_ERR_PORT_CORE_TY_USB_PORT_OVER || result == DF_ERR_PORT_CORE_TY_USB_NO_FIND_DEV)
				err_no = NEW_PROTOCOL_ERR_PORT_ERR;
			if (result == DF_ERR_PORT_CORE_TY_USB_NO_PERMISSION)
				err_no = NEW_PROTOCOL_ERR_NOT_AUTH;
			if (result == DF_ERR_PORT_CORE_TY_USB_PORT_MANAGE_PORT_USEED)
				err_no = NEW_PROTOCOL_ERR_PORT_BUSY;
			new_protocol_err_back(sockfd, err_no, parm);
			return result; 
		}
			
		char *g_buf = NULL;
		unsigned char sendbuf[1024] = { 0 };
		char handle[10] = { 0 };
		cJSON *json = NULL;
		cJSON *dir1, *dir2;		//组包使用
		cJSON *dir3;
		memcpy(sendbuf, "\x01\x06\x00\x07", 4);

		out("8007 返回包!\n");
		json = cJSON_CreateObject();
		cJSON_AddItemToObject(json, "head", dir1 = cJSON_CreateObject());
		cJSON_AddStringToObject(dir1, "protocol", "mb_usbshare_20191024");
		cJSON_AddStringToObject(dir1, "code_type", "response");
		cJSON_AddStringToObject(dir1, "cmd", "apply_use");
		cJSON_AddStringToObject(dir1, "source_topic", "");
		cJSON_AddStringToObject(dir1, "random", "987654321");
		cJSON_AddStringToObject(dir1, "result", "s");

		cJSON_AddItemToObject(json, "data", dir2 = cJSON_CreateArray());
		cJSON_AddItemToObject(dir2, "dira", dir3 = cJSON_CreateObject());

		cJSON_AddStringToObject(dir3, "usb_port", output);
		cJSON_AddStringToObject(dir3, "line_type", "1");
	
		/*cJSON_AddStringToObject(dir3, "tran_server", trans_server);
		cJSON_AddStringToObject(dir3, "tran_port", trans_port);*/
		sprintf(handle, "%d", sockfd);
		cJSON_AddStringToObject(dir3, "line_handle", handle);

		g_buf = cJSON_Print(json);
		sendbuf[4] = strlen(g_buf) / 0x1000000; sendbuf[5] = strlen(g_buf) / 0x10000;
		sendbuf[6] = strlen(g_buf) / 256; sendbuf[7] = strlen(g_buf) % 256;
		out("len ===== %02x %02x", sendbuf[6], sendbuf[7]);
		memcpy(sendbuf + 8, g_buf, strlen(g_buf));
		out("8007 返回包！:%s\n", g_buf);
		result = ty_socket_write(sockfd, sendbuf, strlen(g_buf) + 8);
		cJSON_Delete(json);
		free(g_buf);
		if (result < 0)
			return result;
		return RESULT_OK_FOR_APPLY_PORT_USE_GW;
	}
	else
	{
		return RESULT_OK_FOR_APPLY_PORT_USE_LOCAL;
	}
}



/**********获取终端全部信息*********/
static int get_ter_all_infos_new_share(int sockfd, uint8 *json_in, struct json_parm_new_share *parm, void *output)
{
	int result = 0;
	uint8 *sendbuf = NULL;
	char *g_buf = NULL;
	int len = 0;

	cJSON *dir1, *dir2;		//组包使用
	cJSON *json = cJSON_CreateObject();
	cJSON_AddItemToObject(json, "head", dir1 = cJSON_CreateObject());
	cJSON_AddStringToObject(dir1, "protocol", NEW_USB_SHARE_PROTOCOL);
	cJSON_AddStringToObject(dir1, "code_type", NEW_USB_SHARE_SYNC_CODE_RESPONSE);
	cJSON_AddStringToObject(dir1, "cmd", parm->cmd);
	cJSON_AddStringToObject(dir1, "source_topic", parm->source_topic);
	cJSON_AddStringToObject(dir1, "random", parm->random);
	cJSON_AddStringToObject(dir1, "result", "s");
	
	cJSON_AddItemToObject(json, "data", dir2 = cJSON_CreateArray());
		
	cJSON *data_layer = cJSON_CreateObject();
	cJSON_AddItemToObject(dir2, "dirl", data_layer);
	
	//fun_get_ter_all_data_json(data_layer);
	fun_get_ter_all_data_json_without_tax(data_layer);
		
	int data_len;
	g_buf = cJSON_PrintUnformatted(json);
	//out("g_buf len = %d", strlen(g_buf));
	//out("g_buf data = %s", g_buf);
	str_replace(g_buf, "\\\\", "\\");

	data_len = strlen(g_buf);
	sendbuf = calloc(1, data_len + 128);
	memcpy(sendbuf, "\x01\x06\x00\x07", 4);//协议头
	sendbuf[4] = strlen(g_buf) / 0x1000000; sendbuf[5] = strlen(g_buf) / 0x10000;
	sendbuf[6] = strlen(g_buf) / 256; sendbuf[7] = strlen(g_buf) % 256;
	//sprintf((char*)sendbuf + 4, "%04d", strlen(g_buf) + 4);//长度位
	memcpy(sendbuf + 8, g_buf, strlen(g_buf));
	len = strlen(g_buf) + 8;

	free(g_buf);
	cJSON_Delete(json);

	result = ty_socket_write(sockfd, sendbuf, len);
	if (result < 0)
	{
		free(sendbuf);
		return result;

	}
	free(sendbuf);
	return 0;


}


/**********设置终端机器编号及生产日期*********/
static int set_id_date_new_share(int sockfd, uint8 *json_in, struct json_parm_new_share *parm, void *output)
{

	cJSON *root = NULL;
	cJSON *array_obj = NULL;
	cJSON *arr_item = NULL, *item = NULL;
	char *g_buf = NULL;
	int len = 0;
	uint8 sendbuf[1024 * 5] = { 0 };
	struct _switch_dev_id   id;
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
	item = cJSON_GetObjectItem(arr_item, "ter_id");
	if (item == NULL)
	{
		cJSON_Delete(root);
		return -1;
	}
	uint8 valuetmp[100] = { 0 };
	memset(valuetmp, 0, sizeof(valuetmp));
	memcpy(valuetmp, item->valuestring, strlen(item->valuestring));
	asc2bcd(valuetmp, id.id, 12);
	item = cJSON_GetObjectItem(arr_item, "product_date");
	if (item == NULL)
	{
		cJSON_Delete(root);
		return -1;
	}
	memset(valuetmp, 0, sizeof(valuetmp));
	memcpy(valuetmp, item->valuestring, strlen(item->valuestring));
	asc2bcd(valuetmp, id.date, 4);
	asc2bcd(valuetmp + 5, id.date + 2, 2);
	asc2bcd(valuetmp + 8, id.date + 4, 2);

	/*parm->deal_cmd_fd.machine_fd = ty_open("/dev/machine", 0, 0, NULL);
	if (parm->deal_cmd_fd.machine_fd<0)
	{
		out("打开machine文件失败\n");
		return parm->deal_cmd_fd.machine_fd;
	}*/

	result = ty_ctl(parm->deal_cmd_fd.machine_fd, DF_MACHINE_INFOR_CM_SET_ID, &id);
	if (result < 0)
	{
		cJSON_Delete(root);
		return result;
	}



	cJSON *dir1, *dir2;		//组包使用
	cJSON *json = cJSON_CreateObject();
	cJSON_AddItemToObject(json, "head", dir1 = cJSON_CreateObject());
	cJSON_AddStringToObject(dir1, "protocol", NEW_USB_SHARE_PROTOCOL);
	cJSON_AddStringToObject(dir1, "code_type", NEW_USB_SHARE_SYNC_CODE_RESPONSE);
	cJSON_AddStringToObject(dir1, "cmd", parm->cmd);
	cJSON_AddStringToObject(dir1, "source_topic", parm->source_topic);
	cJSON_AddStringToObject(dir1, "random", parm->random);
	cJSON_AddStringToObject(dir1, "result", "s");

	cJSON_AddItemToObject(json, "data", dir2 = cJSON_CreateArray());
	cJSON *dir3;
	cJSON_AddItemToObject(dir2, "dira", dir3 = cJSON_CreateObject());
	g_buf = cJSON_Print(json);

	memcpy(sendbuf, "\x01\x06\x00\x07", 4);//协议头
	sendbuf[4] = strlen(g_buf) / 0x1000000; sendbuf[5] = strlen(g_buf) / 0x10000;
	sendbuf[6] = strlen(g_buf) / 256; sendbuf[7] = strlen(g_buf) % 256;
	memcpy(sendbuf + 8, g_buf, strlen(g_buf));
	len = strlen(g_buf) + 8;

	free(g_buf);
	cJSON_Delete(json);
	cJSON_Delete(root);
	result = ty_socket_write(sockfd, sendbuf, len);
	if (result < 0)
		return result;


	return 0;

}

/**********设置终端IP*********/
static int set_ip_new_share(int sockfd, uint8 *json_in, struct json_parm_new_share *parm, void *output)
{
	
	cJSON *root = NULL;
	cJSON *array_obj = NULL;
	cJSON *arr_item, *item;
	int type = 0, len = 0;
	char *g_buf = NULL;
	uint8 sendbuf[1024 * 5] = { 0 };
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

		result = ty_ctl(parm->deal_cmd_fd.deploy_fd, DF_DEPLOY_CM_SET_IP, ip, msk, gw, dns, type);
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
		result = ty_ctl(parm->deal_cmd_fd.deploy_fd, DF_DEPLOY_CM_SET_IP, ip, msk, gw, dns, type);
		if (result < 0)
		{

			cJSON_Delete(root);
			return -1;
		}
	
	}

	cJSON *dir1, *dir2;		//组包使用
	cJSON *json = cJSON_CreateObject();
	cJSON_AddItemToObject(json, "head", dir1 = cJSON_CreateObject());
	cJSON_AddStringToObject(dir1, "protocol", NEW_USB_SHARE_PROTOCOL);
	cJSON_AddStringToObject(dir1, "code_type", NEW_USB_SHARE_SYNC_CODE_RESPONSE);
	cJSON_AddStringToObject(dir1, "cmd", parm->cmd);
	cJSON_AddStringToObject(dir1, "source_topic", parm->source_topic);
	cJSON_AddStringToObject(dir1, "random", parm->random);
	cJSON_AddStringToObject(dir1, "result", "s");

	cJSON_AddItemToObject(json, "data", dir2 = cJSON_CreateArray());
	cJSON *dir3;
	cJSON_AddItemToObject(dir2, "dira", dir3 = cJSON_CreateObject());
	g_buf = cJSON_Print(json);

	memcpy(sendbuf, "\x01\x06\x00\x07", 4);//协议头
	sendbuf[4] = strlen(g_buf) / 0x1000000; sendbuf[5] = strlen(g_buf) / 0x10000;
	sendbuf[6] = strlen(g_buf) / 256; sendbuf[7] = strlen(g_buf) % 256;
	memcpy(sendbuf + 8, g_buf, strlen(g_buf));
	len = strlen(g_buf) + 8;

	free(g_buf);
	cJSON_Delete(json);
	cJSON_Delete(root);
	result = ty_socket_write(sockfd, sendbuf, len);
	if (result < 0)
		return result;



	return 0;

}

//tools func
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
static int mb_tar_decompress(char *tar, char *path)
{
	char cmd[1000] = { 0 };
	memset(cmd, 0, sizeof(cmd));
	sprintf(cmd, "tar -xf %s -C %s/", tar, path);
	system(cmd);
	sync();
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
static int mb_copy_file_path(char *from, char *to)
{
	char cmd[1000] = { 0 };
	memset(cmd, 0, sizeof(cmd));
	sprintf(cmd, "cp %s %s", from, to);
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
		out("原目录文件不存在[%s],MD5值默认为不同 !\n", comp_file);
		return 0;//此种情况可以说是MD5值不同
	}
	stat(src_file, &statbufs);
	sizes = statbufs.st_size;
	buff_s = boot_malloc(sizes + 10);
	if (buff_s == NULL)
	{
		out("err malloc!\n");
		return -1;
	}


	stat(comp_file, &statbufc);
	sizec = statbufc.st_size;
	buff_c = boot_malloc(sizec + 10);
	if (buff_c == NULL)
	{
		out("err malloc!\n");
		free(buff_s);
		return -1;
	}


	fds = open(src_file, O_RDONLY);
	if (fds < 0)
	{
		out("tar包下%s文件打开失败!\n", src_file);
		free(buff_c);
		free(buff_s);
		return -1;
	}
	size_read = read(fds, buff_s, sizes);
	if (size_read != sizes)
	{
		out("tar包%s文件读取大小和文件大小不一致!\n", src_file);
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
	//char cmd[100] = { 0 };
//	char update_filename[200] = { 0 };

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
			out("err format configfile!\n");
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
			out("err format configfile!\n");
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
			out("err format configfile Path1 not found!\n");
			fclose(file);
			return -1;
		}
		if (access(tar_file_path, 0) != 0)
		{
			out("err format configfile Path1 not found!\n");
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
			out("err Compare MD5!\n");
			fclose(file);
			return result;
		}
		memset(lineBuff, 0, sizeof(lineBuff));
		continue;
	}
	return 0;
	fclose(file);
}
//end tools

static int Write_Decompress_file(unsigned char *file, long file_len, unsigned char *filename, unsigned char * Dest_dir)
{
	int file_fd, result;
	char config_path[200] = { 0 };
	mb_delete_file_dir((char *)filename, 0);

	file_fd = open((char *)filename, O_CREAT | O_RDWR);
	if (file_fd < 0)
	{
		out("err open file\n");
		return -1;
	}
	result = write(file_fd, file, file_len);
	if (result < 0)
	{
		out("err write update file!\n");
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
/***********文件上传升级**************/
static int ter_update_new_share(int sockfd, uint8 *json_in, struct json_parm_new_share *parm, void *output)
{

	cJSON *root = NULL;
	cJSON *array_obj = NULL;
	cJSON *arr_item, *item;
	int len = 0;
	char *g_buf = NULL;
	uint8 sendbuf[1024 * 5] = { 0 };
	char filename[40] = { 0 };
	unsigned char *filedata = NULL;
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
	item = cJSON_GetObjectItem(arr_item, "file_name");
	if (item == NULL)
	{
		cJSON_Delete(root);
		return -1;
	}
	memcpy(filename, item->valuestring, strlen(item->valuestring));


	item = cJSON_GetObjectItem(arr_item, "file_len");
	if (item == NULL)
	{
		cJSON_Delete(root);
		return -1;
	}
	len = atol(item->valuestring);

	item = cJSON_GetObjectItem(arr_item, "file_data");
	if (item == NULL)
	{
		cJSON_Delete(root);
		return -1;
	}
	if (len  == 0)
	{
		cJSON_Delete(root);
		return -1;
	}
	filedata = calloc(1, len + 10);


	//printf("###############file_len:%d########decode file\n",len);
	decode(item->valuestring, strlen(item->valuestring), (char *)filedata);

	result = Write_Decompress_file(filedata, len, (unsigned char*)"/tmp/update.tar", (unsigned char*)"/tmp/update");

	free(filedata);

	cJSON *dir1, *dir2;		//组包使用
	int flag_ok = 0;
	cJSON *json = cJSON_CreateObject();
	cJSON_AddItemToObject(json, "head", dir1 = cJSON_CreateObject());
	cJSON_AddStringToObject(dir1, "protocol", NEW_USB_SHARE_PROTOCOL);
	cJSON_AddStringToObject(dir1, "code_type", NEW_USB_SHARE_SYNC_CODE_RESPONSE);
	cJSON_AddStringToObject(dir1, "cmd", parm->cmd);
	cJSON_AddStringToObject(dir1, "source_topic", parm->source_topic);
	cJSON_AddStringToObject(dir1, "random", parm->random);
	if (result < 0)//解压更新失败
		cJSON_AddStringToObject(dir1, "result", "f");
	else
	{
		flag_ok = 1;
		cJSON_AddStringToObject(dir1, "result", "s");
	}
		

	cJSON_AddItemToObject(json, "data", dir2 = cJSON_CreateArray());
	cJSON *dir3;
	cJSON_AddItemToObject(dir2, "dira", dir3 = cJSON_CreateObject());
	g_buf = cJSON_Print(json);

	memcpy(sendbuf, "\x01\x06\x00\x07", 4);//协议头
	sendbuf[4] = strlen(g_buf) / 0x1000000; sendbuf[5] = strlen(g_buf) / 0x10000;
	sendbuf[6] = strlen(g_buf) / 256; sendbuf[7] = strlen(g_buf) % 256;
	memcpy(sendbuf + 8, g_buf, strlen(g_buf));
	len = strlen(g_buf) + 8;

	free(g_buf);
	cJSON_Delete(json);
	cJSON_Delete(root);
	result = ty_socket_write(sockfd, sendbuf, len);
	if (flag_ok == 1)
	{
		sleep(3);
		char cmd[100] = { 0 };
		sprintf(cmd, "reboot");
		system(cmd);
	}
	if (result < 0)
	{
		return result;
	}
	return 0;

}

/**********解析json 头********/
static int analysis_json_head_new_share(char *inbuf, struct json_parm_new_share *parm,void *output)
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
			out("Error before head: [%s]\n", cJSON_GetErrorPtr());
			cJSON_Delete(root);
			return -1;
		}
		//printf("cJSON_GetObjectItem: type=%d, key is %s, value is %s\n",object->type,object->string,object->valuestring);  

		if (object != NULL)
		{
			out("analysis_json_head_new_share 获取协议版本\n");
			item = cJSON_GetObjectItem(object, "protocol");
			if (item != NULL)
			{
				out("cJSON_GetObjectItem: type=%d, string is %s, valuestring=%s\n", item->type, item->string, item->valuestring);
				memcpy(parm->protocol, item->valuestring, strlen(item->valuestring));
			}
			result = memcmp(parm->protocol, NEW_USB_SHARE_PROTOCOL, strlen(parm->protocol));
			if (result != 0)
			{
				out("协议版本不一致\n");
				cJSON_Delete(root);
				return -1;
			}
			///////////////////////////////////////////////////////////////////
			out("analysis_json_head_new_share 获取命令类型\n");
			item = cJSON_GetObjectItem(object, "code_type");
			if (item != NULL)
			{
				out("cJSON_GetObjectItem: type=%d, string is %s, valuestring=%s\n", item->type, item->string, item->valuestring);
				memcpy(parm->code_type, item->valuestring, strlen(item->valuestring));
			}
			out("parm->code_type = %s\n", parm->code_type);
			if (memcmp(parm->code_type, NEW_USB_SHARE_SYNC_CODE_REQUEST, strlen(parm->code_type)) != 0)
			{
				out("消息命令错误\n");
				cJSON_Delete(root);
				return -1;
			}
			/////////////////////////////////////////////////////////////////
			out("analysis_json_head_new_share 获取源ID\n");
			item = cJSON_GetObjectItem(object, "source_topic");
			if (item != NULL)
			{
				out("cJSON_GetObjectItem: type=%d, string is %s, valuestring=%s\n", item->type, item->string, item->valuestring);
				memcpy(parm->source_topic, item->valuestring, strlen(item->valuestring));
			}
			out("analysis_json_head_new_share 获取源ID成功 ：%s\n", parm->source_topic);
			////////////////////////////////////////////////////////////////

			out("analysis_json_head_new_share 获取命令字\n");
			item = cJSON_GetObjectItem(object, "cmd");
			if (item != NULL)
			{
				out("cJSON_GetObjectItem: type=%d, string is %s, valueint=%d\n", item->type, item->string, item->valueint);
				//memcpy(parm->cmd,item->valuestring,strlen(item->valuestring)); 
				memcpy(parm->cmd, item->valuestring, strlen(item->valuestring));

			}
			//*cmd = atoi(parm->cmd);

			out("analysis_json_head_new_share 获取本条命令的随机数\n");
			item = cJSON_GetObjectItem(object, "random");
			if (item != NULL)
			{
				//printf("cJSON_GetObjectItem: type=%d, string is %s, valueint=%d\n",item->type,item->string,item->valueint);  
				//memcpy(parm->random,item->valuestring,strlen(item->valuestring)); 
				memcpy(parm->random, item->valuestring, strlen(item->valuestring));
				out("获取随机数成功 ： %s\n", parm->random);

			}

			////////////////////////////////////////////////////////////////
			else
			{
				//printf("cJSON_GetObjectItem: get age failed\n");
				cJSON_Delete(root);
				return -1;
			}
		}
		cJSON_Delete(root);
	}
	return 0;

}


/*********新接口错误返回*********/
int new_protocol_err_back(int sockfd, int err_no, struct json_parm_new_share *parm)
{
	int i;
	char errcode[10] = { 0 };
	unsigned char sendbuf[500 * 2] = { 0 };
	char errinfo[100] = { 0 };
	memcpy(sendbuf, "\x01\x06\x00\x07", 4);//协议
	for (i = 0; i<sizeof(new_protocol_err_n) / sizeof(new_protocol_err_n[0]); i++)
	{
		if (new_protocol_err_n[i].err_no == err_no)
			break;
	}
	if (i == sizeof(new_protocol_err_n) / sizeof(new_protocol_err_n[0]))
	{
		out("错误代码中未能描述该代码意义\n");
		//return err_out_s(fd, 255, (uint8 *)"未知错误");
		sprintf(errinfo, "Unknown err!");
	}
	else
	{
		sprintf(errcode, "%d", err_no);
		sprintf(errinfo, "%s",new_protocol_err_n[i].errinfo);
	}
	char *g_buf;
	int len,result;
	cJSON *dir1, *dir2;		//组包使用
	cJSON *json = cJSON_CreateObject();
	cJSON_AddItemToObject(json, "head", dir1 = cJSON_CreateObject());
	cJSON_AddStringToObject(dir1, "protocol", NEW_USB_SHARE_PROTOCOL);
	cJSON_AddStringToObject(dir1, "code_type", NEW_USB_SHARE_SYNC_CODE_RESPONSE);
	cJSON_AddStringToObject(dir1, "cmd", parm->cmd);
	cJSON_AddStringToObject(dir1, "source_topic", parm->source_topic);
	cJSON_AddStringToObject(dir1, "random", parm->random);
	cJSON_AddStringToObject(dir1, "result", "f");

	cJSON_AddItemToObject(json, "data", dir2 = cJSON_CreateArray());
	cJSON *errlayer;
	cJSON_AddItemToObject(dir2, "dira", errlayer = cJSON_CreateObject());

	cJSON_AddStringToObject(errlayer, "err_code", errcode);
	cJSON_AddStringToObject(errlayer, "err_info", errinfo);
	g_buf = cJSON_Print(json);

	memcpy(sendbuf, "\x01\x06\x00\x07", 4);//协议头
	sendbuf[4] = strlen(g_buf) / 0x1000000; sendbuf[5] = strlen(g_buf) / 0x10000;
	sendbuf[6] = strlen(g_buf) / 256; sendbuf[7] = strlen(g_buf) % 256;
	memcpy(sendbuf + 8, g_buf, strlen(g_buf));
	len = strlen(g_buf) + 8;

	free(g_buf);
	cJSON_Delete(json);
	result = ty_socket_write(sockfd, sendbuf, len);
	if (result < 0)
		return result;

	return 0;
}

/*********根据不同指令处理响应请求*******/
int deal_cmd_newshare(int sockfd,unsigned char *json_in,void *output,int err_back_flag)
{
	int i = 0;
	struct json_parm_new_share parm;
	memcpy(&parm, &ccomm_stream, sizeof(struct json_parm_new_share));
	//struct _deal_cmd_new_fd;
	int result = 0;
	result = analysis_json_head_new_share((char*)json_in, &parm, output);
	if (result < 0)
	{
		return result;
	}
	//out("parm.cmd =%s\n", parm.cmd);
	for (i = 0; i<sizeof(new_shr_order) / sizeof(new_shr_order[0]); i++)
	{
		if (memcmp(new_shr_order[i].order,parm.cmd,strlen(parm.cmd) )== 0)
		{   //out("执行:[%s]\n",order_name[i].name);
			result = new_shr_order[i].answer(sockfd, json_in, &parm, output);
			
			if (result < 0)
			{
				if (err_back_flag == 1)//表示需要此处返回json的TCP错误返回
				{
					return new_protocol_err_back(sockfd, result, &parm);
				}
			}

			return result;//统一错误返回外部处理
		
		}
	}
	return 0;
}

/*******************socket 通讯***************************/

static int ty_socket_write(int sock, uint8 *buf, int buf_len)
{
	int i, result;
	for (i = 0; i<buf_len; i += result)
	{   //result=write(sock,buf+i,buf_len-i);
		result = send(sock, buf + i, buf_len - i, MSG_NOSIGNAL);
		if (result<0)
			return result;
	}
	return i;
}