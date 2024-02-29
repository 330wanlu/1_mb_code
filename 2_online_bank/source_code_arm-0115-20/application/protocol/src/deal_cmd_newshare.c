#include "../inc/deal_cmd_newshare.h"

int inv_lock;

static int ty_socket_write(int sock, uint8 *buf, int buf_len);
static int set_ip_new_share(int sockfd, uint8 *json_in, struct json_parm_new_share *parm, void *output);
static int set_id_date_new_share(int sockfd, uint8 *json_in, struct json_parm_new_share *parm, void *output);
static int apply_port_use(int sockfd, uint8 *json_in, struct json_parm_new_share *parm, void *output);
static int restart_ter(int sockfd, uint8 *json_in, struct json_parm_new_share *parm, void *output);
static int set_ter_func(int sockfd, uint8 *json_in, struct json_parm_new_share *parm, void *output);
static int get_ter_all_infos_new_share(int sockfd, uint8 *json_in, struct json_parm_new_share *parm, void *output);
static int ter_update_new_share(int sockfd, uint8 *json_in, struct json_parm_new_share *parm, void *output);
static int set_iptables(int sockfd, uint8 *json_in, struct json_parm_new_share *parm, void *output);
static int Write_Decompress_file(unsigned char *file, long file_len, unsigned char *filename, unsigned char * Dest_dir);
static int parse_config_copy_file(unsigned char *configname, unsigned char *now_path);
static int mb_delete_file_dir(char *file_path, int file_type);
static int mb_tar_decompress(char *tar, char *path);
static int mb_chmod_file_path(char *power, char *path);
static int mb_create_file_dir(char *file_path);
static int mb_copy_file_path(char *from, char *to);
static int File_Compare_MD5(char *src_file, char *comp_file);
static int restart_signle_port(int sockfd, uint8 *json_in, struct json_parm_new_share *parm, void *output);
static int start_robot_arm(int sockfd, uint8 *json_in, struct json_parm_new_share *parm, void *output);
static int check_net_delay(int sockfd, uint8 *json_in, struct json_parm_new_share *parm, void *output);
static int close_port(int sockfd, uint8 *json_in, struct json_parm_new_share *parm, void *output);
static int intranet_net_upgrade(int sockfd, uint8 *json_in, struct json_parm_new_share *parm, void *output);

struct json_parm_new_share ccomm_stream;

static const struct _new_share_errinfo new_protocol_err_n[] =
{
	{ NEW_PROTOCOL_ERR_SYSTEM, "System err!" },
	{ NEW_PROTOCOL_ERR_NOT_TAX, "This device is not Tax disc!" },
	{ NEW_PROTOCOL_ERR_JSON, "Json parm err!" },
	{ NEW_PROTOCOL_ERR_PORT_ERR, "Port err(device not exist)!" },
	{ NEW_PROTOCOL_ERR_NOT_AUTH, "Port unauthorized!" },
	{ NEW_PROTOCOL_ERR_PORT_BUSY, "Port in used!" },
	{ NEW_PROTOCOL_ERR_PORT_UPDATING, "Port in updating!" },
	{ NEW_PROTOCOL_ERR_PORT_IS_ROBOIARM, "Port is robotarm!" },
	{ NEW_PROTOCOL_ERR_NOT_WITH_ROBOIARM, "The machine is not with robotarm!"},
};
//
static const struct _new_share_order new_shr_order[] =
{
	{ "set_ip",						"[������]:����IP��ַ",						set_ip_new_share					},
	{ "set_id_date",				"[������]:���û�����ź���������",			set_id_date_new_share				},
	{ "ter_status",					"[��ѯ��]:��ѯ�ն�ȫ����Ϣ",				get_ter_all_infos_new_share			},
	{ "apply_use",					"[������]:����˿�ʹ��Ȩ",					apply_port_use						},
	{ "ter_restart",				"[������]:��������",				       restart_ter							},
	{ "set_ter_func",				"[������]:�����ն˹���ͣ����",			 	 set_ter_func						},
	{ "ter_update",					"[������]:�ļ��ϴ�������",					ter_update_new_share				},
	{ "setup_iptables",				"[������]:iptables����",				   set_iptables						},	
	{ "restart_port",				"[������]:���������˿�",					restart_signle_port					},
	{ "start_robot_arm",			"[������]:������е��",						start_robot_arm					},
	{ "check_net_delay",			"[��ѯ��]:��ѯ������ʱ",					check_net_delay					},
	{ "close_port",					"[������]:�رն˿�",						close_port					},
	{ "intranet_net_upgrade",		"[������]:��������",						intranet_net_upgrade				},
};

int init_parm(const _so_note    *note)
{
	memset(&ccomm_stream, 0, sizeof(struct json_parm_new_share));

	ccomm_stream.deal_cmd_fd.note = (_so_note *)note;

	ccomm_stream.deal_cmd_fd.machine_fd = ty_open("/dev/machine", 0, 0, NULL);
	if (ccomm_stream.deal_cmd_fd.machine_fd<0)
	{
		deal_cmd_newshare_out("init_parm��machine�ļ�ʧ��\n");
		return ccomm_stream.deal_cmd_fd.machine_fd;
	}
	ccomm_stream.deal_cmd_fd.ty_usb_fd = ty_open("/dev/ty_usb", 0, 0, NULL);
	if (ccomm_stream.deal_cmd_fd.ty_usb_fd<0)
	{
		deal_cmd_newshare_out("init_parm��ty_usb�ļ�ʧ��\n");
		return ccomm_stream.deal_cmd_fd.ty_usb_fd;
	}
	ccomm_stream.deal_cmd_fd.deploy_fd = ty_open("/dev/deploy", 0, 0, NULL);
	if (ccomm_stream.deal_cmd_fd.deploy_fd < 0)
	{
		deal_cmd_newshare_out("init_parm�������ļ�ʧ��\n");
		return -1;
	}

	ccomm_stream.deal_cmd_fd.get_net_fd = ty_open("/dev/get_net_state", 0, 0, NULL);
	if (ccomm_stream.deal_cmd_fd.get_net_fd < 0)
	{
		deal_cmd_newshare_out("init_parm��get_net_state�ļ�ʧ��\n");
		return -1;
	}
	ccomm_stream.deal_cmd_fd.inv_read_lock = _lock_open("deal_new_cmd.c", NULL);
	inv_lock = _lock_open("new_inv.c", NULL);
	ccomm_stream.deal_cmd_fd.event_file_fd = ty_open("/dev/event", 0, 0, NULL);
	ccomm_stream.deal_cmd_fd.state = 1;
	return 0;
}

/*****�����ն˹�������ͣ��*******/
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
	if (memcmp(item->valuestring, "1", 1) == 0)//���߿���
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
	if (memcmp(item->valuestring, "1", 1) == 0)//���߿���
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
	if (memcmp(item->valuestring, "1", 1) == 0)//4g����
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
	if (memcmp(item->valuestring, "1", 1) == 0)//msql����
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
	if (memcmp(item->valuestring, "1", 1) == 0)//ftp����
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
	if (memcmp(item->valuestring, "1", 1) == 0)//ssh����
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
	if (memcmp(item->valuestring, "1", 1) == 0)//ntp����
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
	if (memcmp(item->valuestring, "1", 1) == 0)//tfcard����
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
	if (memcmp(item->valuestring, "1", 1) == 0)//log����
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
	if (memcmp(item->valuestring, "1", 1) == 0)//USBSHARE����
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
	if (memcmp(item->valuestring, "1", 1) == 0)//�豸������
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
	if (memcmp(item->valuestring, "1", 1) == 0)//mqtt����
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
	if (memcmp(item->valuestring, "1", 1) == 0)//˰����
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
	if (memcmp(item->valuestring, "1", 1) == 0)//������չ֧�ֿ���
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
	if (memcmp(item->valuestring, "1", 1) == 0)//CA֤���ȡ����
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
	if (memcmp(item->valuestring, "1", 1) == 0)//�û���¼����ϵͳ����
	{

	}
	else
	{

	}
	return 0;
}
/*****�����ն�*******/
static int restart_ter(int sockfd, uint8 *json_in, struct json_parm_new_share *parm, void *output)
{

	unsigned char sendbuf[1024] = { 0 };
	char *g_buf = NULL;
	int len,result;
	ty_ctl(parm->deal_cmd_fd.machine_fd, DF_MACHINE_INFOR_CM_RESET_POWER, DF_EVENT_REASON_NET);

	cJSON *dir1, *dir2;		//���ʹ��
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

	memcpy(sendbuf, "\x01\x06\x00\x07", 4);//Э��ͷ
	//����λ

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
/*****����˿�ʹ��Ȩ*****/
static int apply_port_use(int sockfd, uint8 *json_in, struct json_parm_new_share *parm, void *output)
{
	//struct _usb_pro usb;
	int result;
	//uint8 s_buf[200];
	//char client[64];
	int app_fd;
	char user[128] = {0},oper_id[128] = {0x00};
	cJSON *root, *array_obj, *arr_item, *item;
	int line_type = 0,trans_port = 0;
	LOAD_DATA load_data = {0x00};
	USB_PORT_CLASS *usb_port_class = get_usb_port_class();  //��ȡusb_port�Ĳ���ָ��
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
	
	trans_port = atoi(item->valuestring);
	printf("lbc --------------- trans_port = %d\n",trans_port);

	item = cJSON_GetObjectItem(arr_item, "client_id");
	if (item != NULL)
	{
		memcpy((char *)user, item->valuestring, strlen(item->valuestring));
	}

	item = cJSON_GetObjectItem(arr_item, "oper_id");
	if (item != NULL)
	{
		memcpy((char *)oper_id, item->valuestring, strlen(item->valuestring));
	}

	if(*((int *)(usb_port_class->fun_check_with_robot_arm(NULL))))   //���ж��Ƿ��ǻ�е�۵Ļ���
		trans_port = trans_port*2;

	sprintf(output,"%d",trans_port);

	printf("lbc ---------------22222 trans_port = %d,output = %s\n",trans_port,output);

	result = *((int *)(usb_port_class->fun_check_with_robot_arm(NULL)));   //���ж��Ƿ��ǻ�е�۵Ļ���
	printf("apply_port_use result=%d\n",result);
	if(result)
	{
		if(0 > usb_port_class->online_bank_action.mechanical_arm_action.class->fun_arm_get_port_update_state(trans_port > 0?(trans_port - 1):0))  //�ж��Ƿ���������
		{
			result = NEW_PROTOCOL_ERR_PORT_UPDATING;
			logout(INFO, "tcp", "attach", "deal_cmd_newshare,����USB�˿�%dʧ�ܣ���е����������\r\n", trans_port);
			new_protocol_err_back(sockfd, result, parm);
			return result; 
		}
		logout(INFO, "tcp", "attach", "deal_cmd_newshare,����USB�˿�,%d\r\n", trans_port);
		if(0 == get_robot_arm_class()->fun_judge_robot_arm(trans_port > 0?(trans_port - 1):0))   //�ǻ�е��
		{
			result = NEW_PROTOCOL_ERR_PORT_IS_ROBOIARM;
			logout(INFO, "tcp", "attach", "deal_cmd_newshare,����USB�˿�%dʧ�ܣ��˶˿��ǻ�е��\r\n", trans_port);
			new_protocol_err_back(sockfd, result, parm);
			return result;
			
		}
	}

	if (line_type == 1)//��������ת��
	{
		//deal_cmd_newshare_out("line_type  == 1\n");
		//parm->deal_cmd_fd.deploy_fd = ty_open("/dev/deploy", 0, 0, NULL);
		//if (parm->deal_cmd_fd.deploy_fd<0)
		//{
		//	deal_cmd_newshare_out("�������ļ�ʧ��\n");
		//	return parm->deal_cmd_fd.deploy_fd;
		//}
		////printf("======depioy file open success======\n");
		//start = ty_ctl(parm->deal_cmd_fd.deploy_fd, DF_DEPLOY_CM_GET_START_PORT);//��ȡ�������ʼ�˿ں�
		//if (start<0)
		//{
		//	return -1;
		//}
		//
		//app_fd = 0;
		//deal_cmd_newshare_out("[_app.c]���˿ںŴ򿪶˿�\n");
		//memset(client, 0, sizeof(client));//�ͻ��˱�ʶ��GUID
		//parm->deal_cmd_fd.ty_usb_m_fd = ty_open("/dev/ty_usb_m", 0, 0, NULL);
		//if (parm->deal_cmd_fd.ty_usb_m_fd<0)
		//{
		//	deal_cmd_newshare_out("��ty_usb_m�ļ�ʧ�ܣ�result = %d\n", parm->deal_cmd_fd.ty_usb_m_fd);
		//	return parm->deal_cmd_fd.ty_usb_m_fd;
		//}
		//if ((result = ty_ctl(parm->deal_cmd_fd.ty_usb_m_fd, DF_TY_USB_M_CM_OPEN_PORT_IP, trans_port, 0xC0A800FA, client, &app_fd))<0)
		//{
		//	deal_cmd_newshare_out("�򿪶˿�ʧ��\n");
		//	return -1;
		//}

		
		// load_data.port = trans_port;
		load_data.port = (trans_port - 1) >= 0?(trans_port-1):0;
		printf("lbc --------- load_data.port=%d\n",load_data.port);
		memcpy(load_data.user,user,strlen(user));
		memcpy(load_data.oper_id,oper_id,strlen(oper_id));
		load_data.ip = 0x00;
		load_data.port_fd = &app_fd;
		load_data.net_type = EXTERNAL_NETWORK;
		result = *((int *)usb_port_class->load_action.fun_load_start((void *)(&load_data))); //ִ��һ�μ���
		//result = open_port_attach(trans_port, 0x00, client, &app_fd);
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
		cJSON *dir1, *dir2;		//���ʹ��
		cJSON *dir3;
		memcpy(sendbuf, "\x01\x06\x00\x07", 4);

		deal_cmd_newshare_out("8007 ���ذ�!\n");
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
		deal_cmd_newshare_out("len ===== %02x %02x", sendbuf[6], sendbuf[7]);
		memcpy(sendbuf + 8, g_buf, strlen(g_buf));
		deal_cmd_newshare_out("8007 ���ذ���:%s\n", g_buf);
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


// int main() {  
//     int listenfd, connfd;  
//     struct sockaddr_in servaddr;//�������󶨵ĵ�ַ  
//     struct sockaddr_in listendAddr, connectedAddr, peerAddr;//�ֱ��ʾ�����ĵ�ַ�����ӵı��ص�ַ�����ӵĶԶ˵�ַ  
//     int listendAddrLen, connectedAddrLen, peerLen;  
//     char ipAddr[INET_ADDRSTRLEN];//������ʮ���Ƶĵ�ַ  
//     listenfd = socket(AF_INET, SOCK_STREAM, 0);  
//     memset(&servaddr, 0, sizeof(servaddr));  
  
//     servaddr.sin_family = AF_INET;  
//     servaddr.sin_addr.s_addr = htonl(INADDR_ANY);  
//     servaddr.sin_port = htons(PORT);  
      
//     bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr));//�������˰󶨵�ַ  
  
//     listen(listenfd, LISTENQ);  
//     listendAddrLen = sizeof(listendAddr);  
//     getsockname(listenfd, (struct sockaddr *)&listendAddr, &listendAddrLen);//��ȡ�����ĵ�ַ�Ͷ˿�  
//     printf("listen address = %s:%d\n", inet_ntoa(listendAddr.sin_addr), ntohs(listendAddr.sin_port));  
  
//     while(1) {  
//         connfd = accept(listenfd, (struct sockaddr *)NULL, NULL);  
//         connectedAddrLen = sizeof(connectedAddr);  
//         getsockname(connfd, (struct sockaddr *)&connectedAddr, &connectedAddrLen);//��ȡconnfd��ʾ�������ϵı��ص�ַ  
//         printf("connected server address = %s:%d\n", inet_ntoa(connectedAddr.sin_addr), ntohs(connectedAddr.sin_port));  
//         getpeername(connfd, (struct sockaddr *)&peerAddr, &peerLen); //��ȡconnfd��ʾ�������ϵĶԶ˵�ַ  
//         printf("connected peer address = %s:%d\n", inet_ntop(AF_INET, &peerAddr.sin_addr, ipAddr, sizeof(ipAddr)), ntohs(peerAddr.sin_port));  
//     }  
//     return 0;  
// }  

// /**********��ȡ�ն�ȫ����Ϣ*********/
// static int get_ter_all_infos_new_share(int sockfd, uint8 *json_in, struct json_parm_new_share *parm, void *output)
// {
// 	int result = 0;
// 	uint8 *sendbuf = NULL;
// 	char *g_buf = NULL;
// 	int len = 0;
// 	cJSON *root, *array_obj,*arr_item,*item;
// 	char intranet_ip[64] = {0x00},external_ip[64] = {0x00},tmp[32] = {0x00};
// 	int intranet_delay_us = 0,external_delay_us = 0;
// 	float intranet_delay_ms = 0.0,external_delay_ms = 0.0;
// 	USB_PORT_CLASS *usb_port_class = get_usb_port_class();  //��ȡusb_port�Ĳ���ָ��

// 	struct sockaddr_in connectedAddr,peerAddr;
// 	int peerLen = 0,connectedAddrLen;
// 	char ipAddr[32];

// 	// connectedAddrLen = sizeof(connectedAddr);
// 	// getsockname(sockfd, (struct sockaddr *)&connectedAddr, &connectedAddrLen);//��ȡ�����ĵ�ַ�Ͷ˿�  
//     //  printf("connected server address = %s:%d\n", inet_ntoa(connectedAddr.sin_addr), ntohs(connectedAddr.sin_port));

// 	peerLen = sizeof(peerAddr);
// 	getpeername(sockfd, (struct sockaddr *)&peerAddr, &peerLen);
// 	inet_ntop(AF_INET, &peerAddr.sin_addr, ipAddr, sizeof(ipAddr));
// 	// printf("connected peer address = %s:%d\n", inet_ntop(AF_INET, &peerAddr.sin_addr, ipAddr, sizeof(ipAddr)), ntohs(peerAddr.sin_port));

// 	 //printf("lbc  ipAddr = %s\n",ipAddr);

// 	// root = cJSON_Parse((char*)json_in);
// 	// if(NULL != root)
// 	// {
// 	// 	array_obj = cJSON_GetObjectItem(root, "data");
// 	// 	if(NULL != array_obj)
// 	// 	{
// 	// 		arr_item = cJSON_GetArrayItem(array_obj, 0);
// 	// 		if (arr_item != NULL)
// 	// 		{
// 	// 			item = cJSON_GetObjectItem(arr_item, "intranet_ip");
// 	// 			if (item != NULL)
// 	// 			{
// 	// 				memcpy(intranet_ip,item->valuestring,strlen(item->valuestring));
// 	// 				item = cJSON_GetObjectItem(arr_item, "external_ip");
// 	// 				{
// 	// 					if (item != NULL)
// 	// 					{
// 	// 						memcpy(external_ip,item->valuestring,strlen(item->valuestring));
// 	// 						intranet_delay_us = get_net_delay(intranet_ip);
// 	// 						external_delay_us = get_net_delay(external_ip);
// 	// 						intranet_delay_ms = intranet_delay_us;
// 	// 						intranet_delay_ms = intranet_delay_ms/1000;
// 	// 						external_delay_ms = external_delay_us;
// 	// 						external_delay_ms = external_delay_ms/1000;
// 	// 						printf("intranet_ip:%s,intranet_delay_us=%d,external_ip:%s,external_delay_us=%d\n",intranet_ip,intranet_delay_us,external_ip,external_delay_us);
// 	// 					}
// 	// 				}
// 	// 			}
// 	// 		}
// 	// 	}
// 	// }


// 	char buffer_temp[10];
// 	int get_delay_time=0;
// 	memset(buffer_temp,0,sizeof(buffer_temp));
// 	root = cJSON_Parse((char*)json_in);
// 	if(NULL == root)
// 	{
// 		printf("get json data error");
// 	}

// 	array_obj = cJSON_GetObjectItem(root, "data");
// 	if (array_obj == NULL)
// 	{
// 		cJSON_Delete(root);
// 		printf("get data error!");
// 		return -1;
// 	}
// 	g_buf = cJSON_Print(array_obj);
// 	deal_cmd_newshare_out("g_buf data=%s",g_buf);

// 	cJSON *obj = cJSON_GetArrayItem(array_obj, 0);		// ��ȡ���������obj
// 	item = cJSON_GetObjectItem(obj, "_delay_time");
// 	if (item == NULL)
// 	{
// 		cJSON_Delete(root);
// 		// printf("net_delay_time  need to updata !\r\n");
// 		strcpy(buffer_temp,"0");
// 		get_delay_time = atoi(buffer_temp);
// 		// return -1;
// 	}
// 	else
// 	{
// 		strcpy(buffer_temp,item->valuestring);
// 		// printf("\r\n\n");
// 		// printf("_delay_time:%s\r\n",buffer_temp);  //eth_manage

// 		get_delay_time = atoi(buffer_temp);
// 		// printf("get_delay_time:%d\r\n",get_delay_time); 
// 	}

	
// 	intranet_delay_us = get_net_delay_external(ipAddr);//get_net_delay_intranet("eth0",ipAddr);//get_net_delay_intranet
// 	external_delay_us = get_net_delay_external("103.27.4.61");
// 	intranet_delay_ms = intranet_delay_us;
// 	intranet_delay_ms = intranet_delay_ms/1000;
// 	external_delay_ms = external_delay_us;
// 	external_delay_ms = external_delay_ms/1000;

// 	// printf("intranet_delay_ms = %f!\r\n",intranet_delay_ms);

// 	//ip_conflict_check("eth0","192.168.0.32");
	

// 	// usb_port_class->external_delay_ms = abs(external_delay_ms);
// 	if(0 != get_delay_time)
// 	{
// 		usb_port_class->intranet_delay_ms = get_delay_time;//����ǰ���������ʱʱ��
// 	}
// 	else
// 	{
// 		if(intranet_delay_ms < 0)
// 			usb_port_class->intranet_delay_ms = abs(intranet_delay_ms);
// 		else
// 			usb_port_class->intranet_delay_ms = intranet_delay_ms;
// 	}

// 	if(external_delay_ms < 0)
// 		usb_port_class->external_delay_ms = abs(external_delay_ms);
// 	else
// 		usb_port_class->external_delay_ms = external_delay_ms;

// 	// usb_port_class->intranet_delay_ms = get_delay_time;//����ǰ���������ʱʱ��
// 	//�����ϱ�������Ϣ
// 	usb_port_class->report_action.report_data.report_ter_flag_report = 0;

// 	//printf("intranet_ip:%s,intranet_delay_us=%d,external_ip:%s,external_delay_us=%d\n",ipAddr,intranet_delay_us,"103.27.4.61",external_delay_us);

// 	cJSON *dir1, *dir2;		//���ʹ��
// 	cJSON *json = cJSON_CreateObject();
// 	cJSON_AddItemToObject(json, "head", dir1 = cJSON_CreateObject());
// 	cJSON_AddStringToObject(dir1, "protocol", NEW_USB_SHARE_PROTOCOL);
// 	cJSON_AddStringToObject(dir1, "code_type", NEW_USB_SHARE_SYNC_CODE_RESPONSE);
// 	cJSON_AddStringToObject(dir1, "cmd", parm->cmd);
// 	cJSON_AddStringToObject(dir1, "source_topic", parm->source_topic);
// 	cJSON_AddStringToObject(dir1, "random", parm->random);
// 	cJSON_AddStringToObject(dir1, "result", "s");
	
// 	cJSON_AddItemToObject(json, "data", dir2 = cJSON_CreateArray());
		
// 	cJSON *data_layer = cJSON_CreateObject();
// 	cJSON_AddItemToObject(dir2, "dirl", data_layer);
	
// 	(get_usb_port_class())->fun_get_ter_all_data_json_without_tax(data_layer);

// 	memset(tmp,0x00,sizeof(tmp));
// 	sprintf(tmp,"%.02f ms",intranet_delay_ms);
// 	cJSON_AddStringToObject(data_layer, "intranet_delay", tmp);
// 	memset(tmp,0x00,sizeof(tmp));
// 	sprintf(tmp,"%.02f ms",external_delay_ms);
// 	cJSON_AddStringToObject(data_layer, "external_delay", tmp);
		
// 	int data_len;
// 	g_buf = cJSON_PrintUnformatted(json);
// 	//deal_cmd_newshare_out("g_buf len = %d", strlen(g_buf));
// 	//deal_cmd_newshare_out("g_buf data = %s", g_buf);
// 	str_replace(g_buf, "\\\\", "\\");

// 	data_len = strlen(g_buf);
// 	sendbuf = calloc(1, data_len + 128);
// 	memcpy(sendbuf, "\x01\x06\x00\x07", 4);//Э��ͷ
// 	sendbuf[4] = strlen(g_buf) / 0x1000000; sendbuf[5] = strlen(g_buf) / 0x10000;
// 	sendbuf[6] = strlen(g_buf) / 256; sendbuf[7] = strlen(g_buf) % 256;
// 	//sprintf((char*)sendbuf + 4, "%04d", strlen(g_buf) + 4);//����λ
// 	memcpy(sendbuf + 8, g_buf, strlen(g_buf));
// 	len = strlen(g_buf) + 8;

// 	free(g_buf);
// 	cJSON_Delete(json);

// 	result = ty_socket_write(sockfd, sendbuf, len);
// 	if (result < 0)
// 	{
// 		free(sendbuf);
// 		return result;

// 	}
// 	free(sendbuf);
// 	return 0;

// }

/**********��ȡ�ն�ȫ����Ϣ*********/
static int get_ter_all_infos_new_share(int sockfd, uint8 *json_in, struct json_parm_new_share *parm, void *output)
{
	int result = 0;
	uint8 *sendbuf = NULL;
	char *g_buf = NULL;
	int len = 0;
	cJSON *root, *array_obj,*arr_item,*item;
	char intranet_ip[64] = {0x00},external_ip[64] = {0x00},tmp[32] = {0x00};
	int intranet_delay_us = 0,external_delay_us = 0;
	float intranet_delay_ms = 0.0,external_delay_ms = 0.0;
	USB_PORT_CLASS *usb_port_class = get_usb_port_class();  //��ȡusb_port�Ĳ���ָ��

	struct sockaddr_in connectedAddr,peerAddr;
	int peerLen = 0,connectedAddrLen;
	char ipAddr[32];

	// connectedAddrLen = sizeof(connectedAddr);
	// getsockname(sockfd, (struct sockaddr *)&connectedAddr, &connectedAddrLen);//��ȡ�����ĵ�ַ�Ͷ˿�  
    //  printf("connected server address = %s:%d\n", inet_ntoa(connectedAddr.sin_addr), ntohs(connectedAddr.sin_port));

	peerLen = sizeof(peerAddr);
	getpeername(sockfd, (struct sockaddr *)&peerAddr, &peerLen);
	inet_ntop(AF_INET, &peerAddr.sin_addr, ipAddr, sizeof(ipAddr));
	// printf("connected peer address = %s:%d\n", inet_ntop(AF_INET, &peerAddr.sin_addr, ipAddr, sizeof(ipAddr)), ntohs(peerAddr.sin_port));

	 //printf("lbc  ipAddr = %s\n",ipAddr);

	// root = cJSON_Parse((char*)json_in);
	// if(NULL != root)
	// {
	// 	array_obj = cJSON_GetObjectItem(root, "data");
	// 	if(NULL != array_obj)
	// 	{
	// 		arr_item = cJSON_GetArrayItem(array_obj, 0);
	// 		if (arr_item != NULL)
	// 		{
	// 			item = cJSON_GetObjectItem(arr_item, "intranet_ip");
	// 			if (item != NULL)
	// 			{
	// 				memcpy(intranet_ip,item->valuestring,strlen(item->valuestring));
	// 				item = cJSON_GetObjectItem(arr_item, "external_ip");
	// 				{
	// 					if (item != NULL)
	// 					{
	// 						memcpy(external_ip,item->valuestring,strlen(item->valuestring));
	// 						intranet_delay_us = get_net_delay(intranet_ip);
	// 						external_delay_us = get_net_delay(external_ip);
	// 						intranet_delay_ms = intranet_delay_us;
	// 						intranet_delay_ms = intranet_delay_ms/1000;
	// 						external_delay_ms = external_delay_us;
	// 						external_delay_ms = external_delay_ms/1000;
	// 						printf("intranet_ip:%s,intranet_delay_us=%d,external_ip:%s,external_delay_us=%d\n",intranet_ip,intranet_delay_us,external_ip,external_delay_us);
	// 					}
	// 				}
	// 			}
	// 		}
	// 	}
	// }



	// char buffer_temp[10];
	// int get_delay_time=0;
	// memset(buffer_temp,0,sizeof(buffer_temp));
	// root = cJSON_Parse((char*)json_in);
	// if(NULL == root)
	// {
	// 	printf("get json data error");
	// }

	// array_obj = cJSON_GetObjectItem(root, "data");
	// if (array_obj == NULL)
	// {
	// 	cJSON_Delete(root);
	// 	printf("get data error!");
	// 	return -1;
	// }
	// g_buf = cJSON_Print(array_obj);
	// deal_cmd_newshare_out("g_buf data=%s",g_buf);

	// cJSON *obj = cJSON_GetArrayItem(array_obj, 0);		// ��ȡ���������obj
	// item = cJSON_GetObjectItem(obj, "_delay_time");
	// if (item == NULL)
	// {
	// 	cJSON_Delete(root);
	// 	// printf("net_delay_time  need to updata !\r\n");
	// 	strcpy(buffer_temp,"0");
	// 	get_delay_time = atoi(buffer_temp);
	// 	// return -1;
	// }
	// else
	// {
	// 	strcpy(buffer_temp,item->valuestring);
	// 	// printf("\r\n\n");
	// 	// printf("_delay_time:%s\r\n",buffer_temp);  //eth_manage

	// 	get_delay_time = atoi(buffer_temp);
	// 	// printf("get_delay_time:%d\r\n",get_delay_time); 
	// }

	
	intranet_delay_us = get_net_delay_external(ipAddr);//get_net_delay_intranet("eth0",ipAddr);//get_net_delay_intranet
	external_delay_us = get_net_delay_external("103.27.4.61");
	intranet_delay_ms = intranet_delay_us;
	intranet_delay_ms = intranet_delay_ms/1000;
	external_delay_ms = external_delay_us;
	external_delay_ms = external_delay_ms/1000;

	// printf("intranet_delay_ms = %f!\r\n",intranet_delay_ms);

	//ip_conflict_check("eth0","192.168.0.32");
	

	// usb_port_class->external_delay_ms = abs(external_delay_ms);
	if(intranet_delay_ms < 0)
		usb_port_class->intranet_delay_ms = abs(intranet_delay_ms);
	else
		usb_port_class->intranet_delay_ms = intranet_delay_ms;

	if(external_delay_ms < 0)
		usb_port_class->external_delay_ms = abs(external_delay_ms);
	else
		usb_port_class->external_delay_ms = external_delay_ms;

	// usb_port_class->intranet_delay_ms = get_delay_time;//����ǰ���������ʱʱ��
	//�����ϱ�������Ϣ
	usb_port_class->report_action.report_data.report_ter_flag_report = 0;

	//printf("intranet_ip:%s,intranet_delay_us=%d,external_ip:%s,external_delay_us=%d\n",ipAddr,intranet_delay_us,"103.27.4.61",external_delay_us);

	cJSON *dir1, *dir2;		//���ʹ��
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
	
	(get_usb_port_class())->fun_get_ter_all_data_json_without_tax(data_layer);

	// memset(tmp,0x00,sizeof(tmp));
	// sprintf(tmp,"%.02f ms",intranet_delay_ms);
	// cJSON_AddStringToObject(data_layer, "intranet_delay", tmp);
	// memset(tmp,0x00,sizeof(tmp));
	// sprintf(tmp,"%.02f ms",external_delay_ms);
	// cJSON_AddStringToObject(data_layer, "external_delay", tmp);
		
	int data_len;
	g_buf = cJSON_PrintUnformatted(json);
	//deal_cmd_newshare_out("g_buf len = %d", strlen(g_buf));
	//deal_cmd_newshare_out("g_buf data = %s", g_buf);
	str_replace(g_buf, "\\\\", "\\");

	data_len = strlen(g_buf);
	sendbuf = calloc(1, data_len + 128);
	memcpy(sendbuf, "\x01\x06\x00\x07", 4);//Э��ͷ
	sendbuf[4] = strlen(g_buf) / 0x1000000; sendbuf[5] = strlen(g_buf) / 0x10000;
	sendbuf[6] = strlen(g_buf) / 256; sendbuf[7] = strlen(g_buf) % 256;
	//sprintf((char*)sendbuf + 4, "%04d", strlen(g_buf) + 4);//����λ
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


/**********�����ն˻�����ż���������*********/
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
		deal_cmd_newshare_out("��machine�ļ�ʧ��\n");
		return parm->deal_cmd_fd.machine_fd;
	}*/

	result = ty_ctl(parm->deal_cmd_fd.machine_fd, DF_MACHINE_INFOR_CM_SET_ID, &id);
	if (result < 0)
	{
		cJSON_Delete(root);
		return result;
	}



	cJSON *dir1, *dir2;		//���ʹ��
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

	memcpy(sendbuf, "\x01\x06\x00\x07", 4);//Э��ͷ
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

/**********�����ն�IP*********/
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
	int result = 0;
	cJSON *dir1, *dir2;		//���ʹ��
	cJSON *dir3;
	cJSON *json = cJSON_CreateObject();


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
		cJSON_AddItemToObject(json, "head", dir1 = cJSON_CreateObject());
		cJSON_AddStringToObject(dir1, "protocol", NEW_USB_SHARE_PROTOCOL);
		cJSON_AddStringToObject(dir1, "code_type", NEW_USB_SHARE_SYNC_CODE_RESPONSE);
		cJSON_AddStringToObject(dir1, "cmd", parm->cmd);
		cJSON_AddStringToObject(dir1, "source_topic", parm->source_topic);
		cJSON_AddStringToObject(dir1, "random", parm->random);
		if(result == 0)
			cJSON_AddStringToObject(dir1, "result", "s");
		else if(result == -1)
			cJSON_AddStringToObject(dir1, "result", "f01");

		cJSON_AddItemToObject(json, "data", dir2 = cJSON_CreateArray());
		
		cJSON_AddItemToObject(dir2, "dira", dir3 = cJSON_CreateObject());
		g_buf = cJSON_Print(json);

		printf("lbc gbuf=%s\n",g_buf);

		memcpy(sendbuf, "\x01\x06\x00\x07", 4);//Э��ͷ
		sendbuf[4] = strlen(g_buf) / 0x1000000; sendbuf[5] = strlen(g_buf) / 0x10000;
		sendbuf[6] = strlen(g_buf) / 256; sendbuf[7] = strlen(g_buf) % 256;
		memcpy(sendbuf + 8, g_buf, strlen(g_buf));
		len = strlen(g_buf) + 8;

		free(g_buf);
		cJSON_Delete(json);
		cJSON_Delete(root);
		result = ty_socket_write(sockfd, sendbuf, len);

		result = ty_ctl(parm->deal_cmd_fd.deploy_fd, DF_DEPLOY_CM_SET_IP, ip, msk, gw, dns, type);
		if (result < 0)
		{
			cJSON_Delete(root);
			return -1;
		}
		return 0;
	}
	else if (type == 1)
	{
		//static
		item = cJSON_GetObjectItem(arr_item, "ip_address");
		if (item == NULL)
		{
			deal_cmd_newshare_out("get ip err!\n");
			cJSON_Delete(root);
			return -1;
		}
		memcpy(ip_addr, item->valuestring, strlen(item->valuestring));

		if(1 == ip_conflict_check("eth0",ip_addr)) // ����ip��ͻ
		{
			result = -1;
			goto End;
		}

		item = cJSON_GetObjectItem(arr_item, "netmask");
		if (item == NULL)
		{
			deal_cmd_newshare_out("get mask err!\n");
			cJSON_Delete(root);
			return -1;
		}
		memcpy(ip_msk, item->valuestring, strlen(item->valuestring));
		
		item = cJSON_GetObjectItem(arr_item, "gateway");
		if (item == NULL)
		{
			deal_cmd_newshare_out("get  gw err!\n");
			cJSON_Delete(root);
			return -1;
		}
		memcpy(ip_gw, item->valuestring, strlen(item->valuestring));

		item = cJSON_GetObjectItem(arr_item, "dns");
		if (item == NULL)
		{
			deal_cmd_newshare_out("get dns err!\n");
			cJSON_Delete(root);
			return -1;
		}

		memcpy(ip_dns, item->valuestring, strlen(item->valuestring));
		ip = ip_asc2ip(ip_addr, strlen(ip_addr));
		msk = ip_asc2ip(ip_msk, strlen(ip_msk));
		gw = ip_asc2ip(ip_gw, strlen(ip_gw));
		dns = ip_asc2ip(ip_dns, strlen(ip_dns));

		cJSON_AddItemToObject(json, "head", dir1 = cJSON_CreateObject());
		cJSON_AddStringToObject(dir1, "protocol", NEW_USB_SHARE_PROTOCOL);
		cJSON_AddStringToObject(dir1, "code_type", NEW_USB_SHARE_SYNC_CODE_RESPONSE);
		cJSON_AddStringToObject(dir1, "cmd", parm->cmd);
		cJSON_AddStringToObject(dir1, "source_topic", parm->source_topic);
		cJSON_AddStringToObject(dir1, "random", parm->random);
		if(result == 0)
			cJSON_AddStringToObject(dir1, "result", "s");
		else if(result == -1)
			cJSON_AddStringToObject(dir1, "result", "f01");

		cJSON_AddItemToObject(json, "data", dir2 = cJSON_CreateArray());
		
		cJSON_AddItemToObject(dir2, "dira", dir3 = cJSON_CreateObject());
		g_buf = cJSON_Print(json);

		printf("lbc gbuf=%s\n",g_buf);

		memcpy(sendbuf, "\x01\x06\x00\x07", 4);//Э��ͷ
		sendbuf[4] = strlen(g_buf) / 0x1000000; sendbuf[5] = strlen(g_buf) / 0x10000;
		sendbuf[6] = strlen(g_buf) / 256; sendbuf[7] = strlen(g_buf) % 256;
		memcpy(sendbuf + 8, g_buf, strlen(g_buf));
		len = strlen(g_buf) + 8;

		free(g_buf);
		cJSON_Delete(json);
		cJSON_Delete(root);
		result = ty_socket_write(sockfd, sendbuf, len);
		// if (result < 0)
		// 	return result;

		result = ty_ctl(parm->deal_cmd_fd.deploy_fd, DF_DEPLOY_CM_SET_IP, ip, msk, gw, dns, type);
		if (result < 0)
		{

			cJSON_Delete(root);
			return -1;
		}
		return 0;
	}
	

End:
	
	cJSON_AddItemToObject(json, "head", dir1 = cJSON_CreateObject());
	cJSON_AddStringToObject(dir1, "protocol", NEW_USB_SHARE_PROTOCOL);
	cJSON_AddStringToObject(dir1, "code_type", NEW_USB_SHARE_SYNC_CODE_RESPONSE);
	cJSON_AddStringToObject(dir1, "cmd", parm->cmd);
	cJSON_AddStringToObject(dir1, "source_topic", parm->source_topic);
	cJSON_AddStringToObject(dir1, "random", parm->random);
	if(result == 0)
		cJSON_AddStringToObject(dir1, "result", "s");
	else if(result == -1)
		cJSON_AddStringToObject(dir1, "result", "f01");

	cJSON_AddItemToObject(json, "data", dir2 = cJSON_CreateArray());
	
	cJSON_AddItemToObject(dir2, "dira", dir3 = cJSON_CreateObject());
	g_buf = cJSON_Print(json);

	printf("lbc gbuf=%s\n",g_buf);

	memcpy(sendbuf, "\x01\x06\x00\x07", 4);//Э��ͷ
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
		//�ļ�ɾ��
		sprintf(cmd, "rm %s", file_path);
	}
	else
	{
		//�ļ���ɾ��
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
		deal_cmd_newshare_out("ԭĿ¼�ļ�������[%s],MD5ֵĬ��Ϊ��ͬ !\n", comp_file);
		return 0;//�����������˵��MD5ֵ��ͬ
	}
	stat(src_file, &statbufs);
	sizes = statbufs.st_size;
	buff_s = boot_malloc(sizes + 10);
	if (buff_s == NULL)
	{
		deal_cmd_newshare_out("err malloc!\n");
		return -1;
	}


	stat(comp_file, &statbufc);
	sizec = statbufc.st_size;
	buff_c = boot_malloc(sizec + 10);
	if (buff_c == NULL)
	{
		deal_cmd_newshare_out("err malloc!\n");
		free(buff_s);
		return -1;
	}


	fds = open(src_file, O_RDONLY);
	if (fds < 0)
	{
		deal_cmd_newshare_out("tar����%s�ļ���ʧ��!\n", src_file);
		free(buff_c);
		free(buff_s);
		return -1;
	}
	size_read = read(fds, buff_s, sizes);
	if (size_read != sizes)
	{
		deal_cmd_newshare_out("tar��%s�ļ���ȡ��С���ļ���С��һ��!\n", src_file);
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
		return 0;//MD5ֵ��ͬ
	}
	free(buff_c);
	free(buff_s);
	//printf("MD5ֵ��ͬ�������滻!\n");
	return 1;//MD5ֵ��ͬ


}

static int parse_config_copy_file(unsigned char *configname, unsigned char *now_path)
{
	FILE *file;
	int result = 0;
	char lineBuff[256] = { 0 };// �洢��ȡһ�е�����
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
	ftell(file); //�����ļ�����  
	fseek(file, 0, SEEK_SET);
	memset(lineBuff, 0, sizeof(lineBuff));
	//�����ѭ�������ж�config�ļ��ĸ�ʽ�Ƿ���ȷ
	while (fgets(lineBuff, 256, file) != NULL)
	{
		if ((lineBuff[0] == '#') || (lineBuff[0] == ';') || memcmp(lineBuff, "\r\n", 2) == 0 || lineBuff[0] == '\n')
			continue;
		linePos = strstr(lineBuff, "=");
		if (linePos == NULL)
		{
			deal_cmd_newshare_out("err format configfile!\n");
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
			deal_cmd_newshare_out("err format configfile!\n");
			memset(lineBuff, 0, sizeof(lineBuff));
			fclose(file);
			return -1;
		}
		str_replace(lineBuff, "\r\n", "\n");

		memset(file_name, 0, sizeof(file_name));
		memcpy(file_name, lineBuff, strlen(lineBuff) - strlen(linePos));
		//if (memcmp(file_name, "mb_boot", 7) == 0)//�ж��Ƿ��Ǳ�boot��������ǣ�������.
		//	continue;
		memset(file_real_path, 0, sizeof(file_real_path));
		if (linePos[strlen(linePos) - 1] == '\n'&&linePos[strlen(linePos) - 2] == '\r')
			linePos[strlen(linePos) - 2] = '\0';
		else if (linePos[strlen(linePos) - 1] == '\n')
			linePos[strlen(linePos) - 1] = '\0';
		memcpy(file_real_path, linePos + 3, strlen(linePos) - 3);//��ȥһ��" = "


		if (file_real_path[strlen(file_real_path) - 1] == '/')//ͳһ��ʽ(��·���н�β��'/'����ȥ��������ͳһ����)
		{
			file_real_path[strlen(file_real_path) - 1] = '\0';
		}

		memset(tar_file_path, 0, sizeof(tar_file_path));
		sprintf(tar_file_path, "%s/%s", now_path, file_name);
		//���tarĿ¼���Ƿ���Ҫ���µ��ļ����Ƿ������ʵ�����滻��Ŀ¼
		//printf("path1 = %s\npath2 = %s\n", file_real_path, tar_file_path);
		if (access(file_real_path, 0) != 0)
		{
			deal_cmd_newshare_out("err format configfile Path1 not found!\n");
			fclose(file);
			return -1;
		}
		if (access(tar_file_path, 0) != 0)
		{
			deal_cmd_newshare_out("err format configfile Path1 not found!\n");
			fclose(file);
			return -1;
		}
		memset(real_file_path, 0, sizeof(real_file_path));
		sprintf(real_file_path, "%s/%s", file_real_path, file_name);
		result = File_Compare_MD5(tar_file_path, real_file_path);
		if (result == 0)
		{
			//MD5ֵ��ͬ�����滻
			//ԭϵͳ�и��ļ����ȱ��ݸ��ļ�Ϊfilename.back,���ڼ��ʧ�ܻ���ʹ��
			/*if (access(real_file_path, 0) == 0)
			{
				char backpath[200] = { 0 };
				sprintf(backpath, "%s.back", real_file_path);
				mb_copy_file_path(real_file_path, backpath);
			}*/
			//�ñ���tarѹ�����µ��ļ��滻ϵͳ���ļ�
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
//end tools

static int Write_Decompress_file(unsigned char *file, long file_len, unsigned char *filename, unsigned char * Dest_dir)
{
	int file_fd, result;
	char config_path[200] = { 0 };
	mb_delete_file_dir((char *)filename, 0);

	file_fd = open((char *)filename, O_CREAT | O_RDWR);
	if (file_fd < 0)
	{
		deal_cmd_newshare_out("err open file\n");
		return -1;
	}
	result = write(file_fd, file, file_len);
	if (result < 0)
	{
		deal_cmd_newshare_out("err write update file!\n");
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
	sleep(1);//�ȴ���ѹ���
	//���н�ѹ�ļ���У��͸��Ƶ���ǰϵͳ
	sprintf((char*)config_path, "%s/mb_config.cnf", Dest_dir);
	result = parse_config_copy_file((unsigned char *)config_path, Dest_dir);
	if (result < 0)
		return result;
	return 0;


}
/***********�ļ��ϴ�����**************/
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

	cJSON *dir1, *dir2;		//���ʹ��
	int flag_ok = 0;
	cJSON *json = cJSON_CreateObject();
	cJSON_AddItemToObject(json, "head", dir1 = cJSON_CreateObject());
	cJSON_AddStringToObject(dir1, "protocol", NEW_USB_SHARE_PROTOCOL);
	cJSON_AddStringToObject(dir1, "code_type", NEW_USB_SHARE_SYNC_CODE_RESPONSE);
	cJSON_AddStringToObject(dir1, "cmd", parm->cmd);
	cJSON_AddStringToObject(dir1, "source_topic", parm->source_topic);
	cJSON_AddStringToObject(dir1, "random", parm->random);
	if (result < 0)//��ѹ����ʧ��
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

	memcpy(sendbuf, "\x01\x06\x00\x07", 4);//Э��ͷ
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

int system_cmd_out_file(char *cmd, char **result)
{
	char *data;
	char path[256] = { 0 };
	int nFileLen;
	char order[1024] = { 0 };
	srand((unsigned)time(NULL));
	sprintf(path, "/tmp/cmd_%d", rand() % 1000 + 1);
	sprintf(order, "%s > %s", cmd, path);
	system(order);
	FILE* fp = fopen(path, "r");
	if (fp == NULL)
	{
		deal_cmd_newshare_out("�ļ���ʧ��,�ļ�����%s\n", path);
		memset(order, 0, sizeof(order));
		sprintf(order, "rm %s", path);
		system(order);
		return -1;
	}
	fseek(fp, 0, SEEK_END);
	nFileLen = ftell(fp); //�ļ�����
	fclose(fp);
	data = calloc(1, nFileLen + 1024);
	if (data == NULL)
	{
		deal_cmd_newshare_out("�ڴ�����ʧ��\n");
		memset(order, 0, sizeof(order));
		sprintf(order, "rm %s", path);
		system(order);
		return -2;
	}
	if (read_file(path, data, nFileLen + 1024) < 0)
	{
		deal_cmd_newshare_out("�ļ���ȡʧ��\n");
		free(data);
		memset(order, 0, sizeof(order));
		sprintf(order, "rm %s", path);
		system(order);
		return -3;
	}

	char *base_data;
	base_data = calloc(1, nFileLen * 2 + 1024);
	if (base_data == NULL)
	{
		deal_cmd_newshare_out("�ڴ�����ʧ��\n");
		free(data);
		memset(order, 0, sizeof(order));
		sprintf(order, "rm %s", path);
		system(order);
		return -2;
	}
	if (strlen(data) == 0)
	{
		sprintf(data, "success");
	}
	base64_enc((uint8 *)base_data, (uint8 *)data, nFileLen);

	*result = base_data;
	free(data);
	memset(order, 0, sizeof(order));
	sprintf(order, "rm %s", path);
	system(order);
	return 0;
}

static int set_iptables(int sockfd, uint8 *json_in, struct json_parm_new_share *parm, void *output)
{

	cJSON *root = NULL;
	cJSON *array_obj = NULL;
	cJSON *arr_item, *item;
	int len = 0;
	char *g_buf = NULL;
	uint8 *sendbuf = NULL;
	char order[500] = { 0 };
	char *cmdresult = NULL;
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
		
		return NEW_PROTOCOL_ERR_JSON;
	}
	arr_item = cJSON_GetArrayItem(array_obj, 0);
	if (arr_item == NULL)
	{
		cJSON_Delete(root);
		
		return NEW_PROTOCOL_ERR_JSON;
	}
	item = cJSON_GetObjectItem(arr_item, "order");
	if (item == NULL)
	{
		
		cJSON_Delete(root);
		return NEW_PROTOCOL_ERR_JSON;
	}
	if (strlen(item->valuestring) > sizeof(order))
	{
		cJSON_Delete(root);
		return NEW_PROTOCOL_ERR_JSON;
	}
	memcpy(order, item->valuestring, strlen(item->valuestring));
	if (memcmp(order, "iptables", 8) != 0)//����iptables������
	{
		cJSON_Delete(root);
		

		return NEW_PROTOCOL_ERR_JSON;
	}
	if (access("/bin/iptables", 0) != 0)
	{
		if (access("/bin/xtables-multi", 0) != 0)
		{
			cJSON_Delete(root);
			return NEW_PROTOCOL_ERR_OPER;
		}
		else
		{
			system("chmod 777 /bin/xtables-multi");
			system("ln -s /bin/xtables-multi /bin/iptables");
			system("ln -s /bin/xtables-multi /bin/iptables-save");
			system("ln -s /bin/xtables-multi /bin/iptables-restore");
		}
	}
	if (system_cmd_out_file(order, &cmdresult) < 0)//ִ��iptables��������д�뵽cmdresult
	{
		
		cJSON_Delete(root);
		return NEW_PROTOCOL_ERR_OPER;
	}

	system("iptables-save > /etc/iptables.conf");//�����εĲ���д�뵽�����ļ���������ʱ��ʹ��
	sync();


	cJSON *dir1, *dir2;		//���ʹ��
	//int flag_ok = 0;
	cJSON *json = cJSON_CreateObject();
	cJSON_AddItemToObject(json, "head", dir1 = cJSON_CreateObject());
	cJSON_AddStringToObject(dir1, "protocol", NEW_USB_SHARE_PROTOCOL);
	cJSON_AddStringToObject(dir1, "code_type", NEW_USB_SHARE_SYNC_CODE_RESPONSE);
	cJSON_AddStringToObject(dir1, "cmd", parm->cmd);
	cJSON_AddStringToObject(dir1, "source_topic", parm->source_topic);
	cJSON_AddStringToObject(dir1, "random", parm->random);
	
	cJSON_AddStringToObject(dir1, "result", "s");
	
	printf("cmd result11 :%s\n", cmdresult);

	cJSON_AddItemToObject(json, "data", dir2 = cJSON_CreateArray());
	cJSON *dir3;
	cJSON_AddItemToObject(dir2, "dira", dir3 = cJSON_CreateObject());

	
	cJSON_AddStringToObject(dir3, "cmd_result", (const char *)cmdresult);
	if (cmdresult != NULL)
	{
		free(cmdresult);
	}

	g_buf = cJSON_Print(json);
	sendbuf = malloc(strlen(g_buf) + 100);
	memcpy(sendbuf, "\x01\x06\x00\x07", 4);//Э��ͷ
	sendbuf[4] = strlen(g_buf) / 0x1000000; sendbuf[5] = strlen(g_buf) / 0x10000;
	sendbuf[6] = strlen(g_buf) / 256; sendbuf[7] = strlen(g_buf) % 256;
	memcpy(sendbuf + 8, g_buf, strlen(g_buf));
	len = strlen(g_buf) + 8;

	free(g_buf);
	cJSON_Delete(json);
	cJSON_Delete(root);
	result = ty_socket_write(sockfd, sendbuf, len);
	if (result < 0)
	{
		free(sendbuf);
		return NEW_PROTOCOL_ERR_SOCKET;
	}
	free(sendbuf);
	return 0;

}

static int restart_signle_port(int sockfd, uint8 *json_in, struct json_parm_new_share *parm, void *output)
{
	cJSON *array_obj = NULL;
	cJSON *root = NULL;
	cJSON *arr_item = NULL, *item = NULL;
	int result = 0,port = 0,displayport = 0;
	uint8 *sendbuf = NULL;
	char *g_buf = NULL;
	int len = 0;
	USB_PORT_CLASS *usb_port_class = get_usb_port_class();  //��ȡusb_port�Ĳ���ָ��
	//printf("!!!!!!!!!!!!!!lbc  enter restart_signle_port\n");

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
	item = cJSON_GetObjectItem(arr_item, "usb_port");
	if (item == NULL)
	{
		cJSON_Delete(root);
		return -1;
	}
	if(item->valuestring != NULL)
		port = atoi(item->valuestring);
	else
	{
		logout(INFO, "system", "�����˿�", "�����˿�ʧ��,�����usb_port����ȷ\r\n");
		new_protocol_err_back(sockfd, NEW_PROTOCOL_ERR_PORT_ERR, parm);
		return -1;
	}
	displayport = port;
	result = *((int *)(usb_port_class->fun_check_with_robot_arm(NULL)));   //���ж��Ƿ��ǻ�е�۵Ļ���
	if(result)
	{
		printf("lbc usb_port,usb_port_class->usb_port_numb = %d,port = %d\n",usb_port_class->usb_port_numb,port);
		if(port<1 || port > usb_port_class->usb_port_numb)
		{
			logout(INFO, "system", "�����˿�", "�����˿�ʧ��,�����usb_port����ȷ\r\n");
			new_protocol_err_back(sockfd, NEW_PROTOCOL_ERR_PORT_ERR, parm);
			return -1;
		}
		port = port*2;
	}
	else
	{
		if(port<1 || port > usb_port_class->usb_port_numb)
		{
			logout(INFO, "system", "�����˿�", "�����˿�ʧ��,�����usb_port����ȷ\r\n");
			new_protocol_err_back(sockfd, NEW_PROTOCOL_ERR_PORT_ERR, parm);
			return -1;
		}
	}
		

	
	if(result)
	{
		if(0 > usb_port_class->online_bank_action.mechanical_arm_action.class->fun_arm_get_port_update_state(port > 0?(port - 1):0))  //�ж��Ƿ���������
		{
			logout(INFO, "system", "�����˿�", "�����˿ڡ�%d��ʧ��,��е����������\r\n",displayport);
			result = NEW_PROTOCOL_ERR_PORT_UPDATING;
			new_protocol_err_back(sockfd, result, parm);
			return result; 
		}
	}
	else
	{
		if(0 < strlen(usb_port_class->online_bank_action.mechanical_arm_action.arm_data[port > 0?(port - 1):0].port_info.arm_version))
		{
			if(0 > usb_port_class->online_bank_action.mechanical_arm_action.class->fun_arm_get_port_update_state(port > 0?(port - 1):0))  //�ж��Ƿ���������
			{
				logout(INFO, "system", "����������е��", "��е�ۺš�%d��,������е��ʧ�ܣ���е����������\r\n", displayport);
				result = NEW_PROTOCOL_ERR_PORT_UPDATING;
				new_protocol_err_back(sockfd, result, parm);
				return result; 
			}
		}
	}

	result = usb_port_class->power_action.fun_power((port-1>0?port-1:0),RETART_ACTION);

	//restart_port(port);

	cJSON *dir1, *dir2;		//���ʹ��
	cJSON *json = cJSON_CreateObject();
	cJSON_AddItemToObject(json, "head", dir1 = cJSON_CreateObject());
	cJSON_AddStringToObject(dir1, "protocol", NEW_USB_SHARE_PROTOCOL);
	cJSON_AddStringToObject(dir1, "code_type", NEW_USB_SHARE_SYNC_CODE_RESPONSE);
	cJSON_AddStringToObject(dir1, "cmd", parm->cmd);
	cJSON_AddStringToObject(dir1, "source_topic", parm->source_topic);
	cJSON_AddStringToObject(dir1, "random", parm->random);
	if(result < 0)
	{
		cJSON_AddStringToObject(dir1, "result", "f");
		logout(INFO, "system", "�����˿�", "�����˿ڡ�%d��ʧ��\r\n",displayport);
	}	
	else
	{
		cJSON_AddStringToObject(dir1, "result", "s");
		logout(INFO, "system", "�����˿�", "�����˿ڡ�%d���ɹ�\r\n",displayport);
	}
		
	int data_len;
	g_buf = cJSON_PrintUnformatted(json);
	//out("g_buf len = %d", strlen(g_buf));
	//out("g_buf data = %s", g_buf);
	str_replace(g_buf, "\\\\", "\\");

	data_len = strlen(g_buf);
	sendbuf = calloc(1, data_len + 128);
	memcpy(sendbuf, "\x01\x06\x00\x07", 4);//Э��ͷ
	sendbuf[4] = strlen(g_buf) / 0x1000000; sendbuf[5] = strlen(g_buf) / 0x10000;
	sendbuf[6] = strlen(g_buf) / 256; sendbuf[7] = strlen(g_buf) % 256;
	//sprintf((char*)sendbuf + 4, "%04d", strlen(g_buf) + 4);//����λ
	memcpy(sendbuf + 8, g_buf, strlen(g_buf));
	len = strlen(g_buf) + 8;

	free(g_buf);
	cJSON_Delete(json);
	cJSON_Delete(root);

	result = ty_socket_write(sockfd, sendbuf, len);
	if (result < 0)
	{
		free(sendbuf);
		return result;

	}
	free(sendbuf);
	
	return 0;
}

static int close_port(int sockfd, uint8 *json_in, struct json_parm_new_share *parm, void *output)
{
	LOAD_DATA load_data = {0x00};
	cJSON *array_obj = NULL;
	cJSON *root = NULL;
	cJSON *arr_item = NULL, *item = NULL;
	int result = 0,port = 0,displayport = 0;
	uint8 *sendbuf = NULL;
	char *g_buf = NULL;
	int len = 0;
	USB_PORT_CLASS *usb_port_class = get_usb_port_class();  //��ȡusb_port�Ĳ���ָ��

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
	item = cJSON_GetObjectItem(arr_item, "usb_port");
	if(item->valuestring != NULL)
		port = atoi(item->valuestring);
	else
	{
		logout(INFO, "system", "�����رն˿�", "�����رն˿�,�����usb_port����ȷ\r\n");
		cJSON_Delete(root);
		new_protocol_err_back(sockfd, NEW_PROTOCOL_ERR_PORT_ERR, parm);
		return -1;
	}
	cJSON_Delete(root);
	if(*((int *)(usb_port_class->fun_check_with_robot_arm(NULL))))   //���ж��Ƿ��ǻ�е�۵Ļ���
	{
		port = port*2;
	}
	load_data.port = (port - 1) > 0?(port-1):0;
	usb_port_class->load_action.fun_load_stop((void *)(&load_data));
	
	return 0;
}


// static int start_robot_arm(int sockfd, uint8 *json_in, struct json_parm_new_share *parm, void *output)
// {
// 	cJSON *array_obj = NULL;
// 	cJSON *root = NULL;
// 	cJSON *arr_item = NULL, *item = NULL;
// 	int result = 0,port = 0,display_port = 0;
// 	uint8 *sendbuf = NULL;
// 	char *g_buf = NULL;
// 	int len = 0;

// 	USB_PORT_CLASS *usb_port_class = get_usb_port_class();  //��ȡusb_port�Ĳ���ָ��

// 	root = cJSON_Parse((char*)json_in);
// 	if (root == NULL)
// 	{
// 		return -1;
// 	}
// 	array_obj = cJSON_GetObjectItem(root, "data");
// 	if (array_obj == NULL)
// 	{
// 		cJSON_Delete(root);
// 		return -1;
// 	}
// 	arr_item = cJSON_GetArrayItem(array_obj, 0);
// 	if (arr_item == NULL)
// 	{
// 		cJSON_Delete(root);
// 		return -1;
// 	}
// 	item = cJSON_GetObjectItem(arr_item, "usb_port");
// 	if(item->valuestring != NULL)
// 		port = atoi(item->valuestring);
// 	else
// 	{
// 		logout(INFO, "system", "����������е��", "����������е��,�����usb_port����ȷ\r\n");
// 		new_protocol_err_back(sockfd, NEW_PROTOCOL_ERR_PORT_ERR, parm);
// 		return -1;
// 	}
// 	if(port<1 || port > usb_port_class->usb_port_numb)
// 	{
// 		logout(INFO, "system", "����������е��", "����������е��,�����usb_port����ȷ\r\n");
// 		new_protocol_err_back(sockfd, NEW_PROTOCOL_ERR_PORT_ERR, parm);
// 		return -1;
// 	}
// 	display_port = port;
// 	//port = port*2 - 1;

// 	port = port-1>0?port-1:0;
// 	printf("port111 = %d\n",port);

// 	// result = *((int *)(usb_port_class->fun_check_with_robot_arm(NULL)));   //���ж��Ƿ��ǻ�е�۵Ļ���

// 	// if(1 != result )
// 	// {
// 	// 	new_protocol_err_back(sockfd, NEW_PROTOCOL_ERR_NOT_WITH_ROBOIARM, parm);
// 	// 	return -1; 
// 	// }
// 	//printf("!!!!!!!!!!!!!!lbc  enter start_robot_arm\n");

	

// 	// item = cJSON_GetObjectItem(arr_item, "usb_port");
// 	// if (item == NULL)
// 	// {
// 	// 	cJSON_Delete(root);
// 	// 	return -1;
// 	// }
// 	// port = atoi(item->valuestring);

	

// 	result = *((int *)(usb_port_class->fun_check_with_robot_arm(NULL)));   //���ж��Ƿ��ǻ�е�۵Ļ���
// 	if(result)
// 	{
// 		if(0 > usb_port_class->online_bank_action.mechanical_arm_action.class->fun_arm_get_port_update_state(port))  //�ж��Ƿ���������
// 		{
// 			logout(INFO, "system", "����������е��", "��е�ۺš�%d��,������е��ʧ�ܣ���е����������\r\n", display_port);
// 			result = NEW_PROTOCOL_ERR_PORT_UPDATING;
// 			new_protocol_err_back(sockfd, result, parm);
// 			return result; 
// 		}
// 	}
// 	else
// 	{
// 		if(0 < strlen(usb_port_class->online_bank_action.mechanical_arm_action.arm_data[port].port_info.arm_version))
// 		{
// 			if(0 > usb_port_class->online_bank_action.mechanical_arm_action.class->fun_arm_get_port_update_state(port))  //�ж��Ƿ���������
// 			{
// 				logout(INFO, "system", "����������е��", "��е�ۺš�%d��,������е��ʧ�ܣ���е����������\r\n", display_port);
// 				result = NEW_PROTOCOL_ERR_PORT_UPDATING;
// 				new_protocol_err_back(sockfd, result, parm);
// 				return result; 
// 			}
// 		}
// 	}

// 	logout(INFO, "system", "����������е��", "��е�ۺš�%d��,������е��\r\n", display_port);

	
// 	cJSON *dir1, *dir2;		//���ʹ��
// 	cJSON *json = cJSON_CreateObject();
// 	cJSON_AddItemToObject(json, "head", dir1 = cJSON_CreateObject());
// 	cJSON_AddStringToObject(dir1, "protocol", NEW_USB_SHARE_PROTOCOL);
// 	cJSON_AddStringToObject(dir1, "code_type", NEW_USB_SHARE_SYNC_CODE_RESPONSE);
// 	cJSON_AddStringToObject(dir1, "cmd", parm->cmd);
// 	cJSON_AddStringToObject(dir1, "source_topic", parm->source_topic);
// 	cJSON_AddStringToObject(dir1, "random", parm->random);

// 	printf("lbc111 %d\n",(port));
	
// 	if(0 < strlen(usb_port_class->online_bank_action.mechanical_arm_action.arm_data[port].port_info.arm_version))
// 	{
// 		result = 0;
// 	}
// 	else
// 	{
// 		result = -1;
// 	}
// 	if(result == 0)
// 	{
// 		cJSON_AddStringToObject(dir1, "result", "s");	 //��ѹ�ɹ�
// 		logout(INFO, "system", "����������е��", "��е�ۺš�%d��,��ѹ�ɹ�\r\n", display_port);
// 	}
		
// 	else if(result == -1)
// 	{
// 		cJSON_AddStringToObject(dir1, "result", "f01");  //��ѹʧ�ܣ����ܻ�е�������豸
// 		logout(INFO, "system", "����������е��", "��е�ۺš�%d��,��ѹʧ�ܣ��˶˿��޻�е�ۻ��߻�е�۹���\r\n", display_port);
// 	}
// 	// else if(result == -2)
// 	// {
// 	// 	cJSON_AddStringToObject(dir1, "result", "f02");	 //��е������ʧ��
// 	// 	logout(INFO, "system", "����������е��", "�˿ںš�%d��,��е������ʧ��\r\n", port + 1);
// 	// }
		
		
// 	int data_len;
// 	printf("!!!!!!!!!!!!!!lbc  exit get_usb_port_class 666666666666666\n");
// 	g_buf = cJSON_PrintUnformatted(json);
// 	printf("g_buf  = %s\n", g_buf);
// 	//printf("!!!!!!!!!!!!!!lbc  exit get_usb_port_class 777777777\n");
// 	str_replace(g_buf, "\\\\", "\\");

// 	data_len = strlen(g_buf);
// 	sendbuf = calloc(1, data_len + 128);
// 	memcpy(sendbuf, "\x01\x06\x00\x07", 4);//Э��ͷ
// 	sendbuf[4] = strlen(g_buf) / 0x1000000; sendbuf[5] = strlen(g_buf) / 0x10000;
// 	sendbuf[6] = strlen(g_buf) / 256; sendbuf[7] = strlen(g_buf) % 256;
// 	//sprintf((char*)sendbuf + 4, "%04d", strlen(g_buf) + 4);//����λ
// 	memcpy(sendbuf + 8, g_buf, strlen(g_buf));
// 	len = strlen(g_buf) + 8;

// 	if(g_buf != NULL)
// 		free(g_buf);
// 	cJSON_Delete(json);
// 	cJSON_Delete(root);

// 	result = ty_socket_write(sockfd, sendbuf, len);
// 	if (result < 0)
// 	{
// 		free(sendbuf);
// 		return result;

// 	}
// 	free(sendbuf);

// 	//printf("port222 = %d\n",port);
// 	if(0 < strlen(usb_port_class->online_bank_action.mechanical_arm_action.arm_data[port].port_info.arm_version))
// 	{
// 		result = *((int *)(usb_port_class->online_bank_action.mechanical_arm_action.arm_start((void *)(&port))));
// 		printf("result = %d\n",result);
// 		if(result == 0)
// 			result = 1;
// 		else if(result == -1)
// 			result = 0;
// 		else
// 			result = 0;
// 		if(usb_port_class->online_bank_action.mechanical_arm_action.arm_with_dev[port].arm_with_dev_result != result)
// 		{
// 			//usb_port_class->get_dev_basic_action.dev_basic_data[(port+1)/2*2+1].up_report = 1;   //�ϱ�һ�λ�е���������豸
// 			usb_port_class->online_bank_action.mechanical_arm_action.arm_with_dev[port].arm_with_dev_result = result;  //״̬���µ�����
// 			//�����ϱ�������Ϣ
// 			usb_port_class->report_action.report_data.report_ter_flag_report = 0;
// 			usb_port_class->online_bank_action.mechanical_arm_action.arm_data[port].port = port + 1;
// 			usb_port_class->online_bank_action.mechanical_arm_action.arm_data[port].up_report = 1;
// 			usb_port_class->online_bank_action.mechanical_arm_action.arm_data[port].last_act = 1;
			

// 		}
// 	}

// 	//printf("!!!!!!!!!!!!!!lbc  exit get_usb_port_class\n");
// 	//restart_port(port);

// 	logout(INFO, "system", "����������е��", "��е�ۺš�%d��,������е���������\r\n", display_port);
// 	return 0;
// }


static int start_robot_arm(int sockfd, uint8 *json_in, struct json_parm_new_share *parm, void *output)
{
	cJSON *array_obj = NULL;
	cJSON *root = NULL;
	cJSON *arr_item = NULL, *item = NULL;
	int result = 0,port = 0,display_port = 0;
	uint8 *sendbuf = NULL;
	char *g_buf = NULL;
	int len = 0;

	char usb_vid_tmp[8];
	char usb_pid_tmp[8];

	USB_PORT_CLASS *usb_port_class = get_usb_port_class();  //��ȡusb_port�Ĳ���ָ��

	root = cJSON_Parse((char*)json_in);
	if (root == NULL)
	{
		return -1;
	}

	g_buf = cJSON_Print(root);
	printf("g_buf=%s\r\n",g_buf);

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
	item = cJSON_GetObjectItem(arr_item, "usb_port");
	
	if(item->valuestring != NULL)
		port = atoi(item->valuestring);
	else
	{
		logout(INFO, "system", "����������е��", "����������е��,�����usb_port����ȷ\r\n");
		new_protocol_err_back(sockfd, NEW_PROTOCOL_ERR_PORT_ERR, parm);
		return -1;
	}
	if(port<1 || port > usb_port_class->usb_port_numb)
	{
		logout(INFO, "system", "����������е��", "����������е��,�����usb_port����ȷ\r\n");
		new_protocol_err_back(sockfd, NEW_PROTOCOL_ERR_PORT_ERR, parm);
		return -1;
	}
	printf("____________________________________________________usb_port =%d\r\n",port);


	item = cJSON_GetObjectItem(arr_item, "usb_vid");
	if (item == NULL)
	{
		// cJSON_Delete(root);
		memset(usb_vid_tmp,0,sizeof(usb_vid_tmp));
		memcpy(usb_vid_tmp,"8888",4);
		printf("usb_vid_tmp = %s\n",usb_vid_tmp);	
		// return -1;
	}
	else
	{
		memset(usb_vid_tmp,0,sizeof(usb_vid_tmp));
		memcpy(usb_vid_tmp,item->valuestring,4);
		printf("usb_vid_tmp = %s\n",usb_vid_tmp);	
	}

	item = cJSON_GetObjectItem(arr_item, "usb_pid");
	if (item == NULL)
	{
		// cJSON_Delete(root);
		memset(usb_pid_tmp,0,sizeof(usb_pid_tmp));
		memcpy(usb_pid_tmp,"8888",4);
		printf("usb_pid_tmp = %s\n",usb_pid_tmp);
		// return -1;
	}
	else
	{
		memset(usb_pid_tmp,0,sizeof(usb_pid_tmp));
		memcpy(usb_pid_tmp,item->valuestring,4);
		printf("usb_pid_tmp = %s\n",usb_pid_tmp);
	}
	



	display_port = port;

	// result = *((int *)(usb_port_class->fun_check_with_robot_arm(NULL)));   //���ж��Ƿ��ǻ�е�۵Ļ���

	// if(1 != result )
	// {
	// 	new_protocol_err_back(sockfd, NEW_PROTOCOL_ERR_NOT_WITH_ROBOIARM, parm);
	// 	return -1; 
	// }
	//printf("!!!!!!!!!!!!!!lbc  enter start_robot_arm\n");

	

	// item = cJSON_GetObjectItem(arr_item, "usb_port");
	// if (item == NULL)
	// {
	// 	cJSON_Delete(root);
	// 	return -1;
	// }
	// port = atoi(item->valuestring);




	if ((memcmp(usb_vid_tmp,"1ea7",4)== 0 && memcmp(usb_pid_tmp,"0064",4)== 0) || (memcmp(usb_vid_tmp,"0000",4)== 0 && memcmp(usb_pid_tmp,"0000",4)== 0))
	{
		printf("	vid == 0x%s		pid == 0x%s\r\n",usb_vid_tmp,usb_pid_tmp);
		result = *((int *)(usb_port_class->fun_check_with_robot_arm(NULL)));   //���ж��Ƿ��ǻ�е�۵Ļ���
		if(result)
		{
			printf("lbc usb_port,usb_port_class->usb_port_numb = %d,port = %d\n",usb_port_class->usb_port_numb,port);
			if(port<1 || port > usb_port_class->usb_port_numb)
			{
				logout(INFO, "system", "�����˿�", "�����˿�ʧ��,�����usb_port����ȷ\r\n");
				new_protocol_err_back(sockfd, NEW_PROTOCOL_ERR_PORT_ERR, parm);
				return -1;
			}
			port = port*2;
		}
		else
		{
			if(port<1 || port > usb_port_class->usb_port_numb)
			{
				logout(INFO, "system", "�����˿�", "�����˿�ʧ��,�����usb_port����ȷ\r\n");
				new_protocol_err_back(sockfd, NEW_PROTOCOL_ERR_PORT_ERR, parm);
				return -1;
			}
		}
	
		if(result)
		{
			if(0 > usb_port_class->online_bank_action.mechanical_arm_action.class->fun_arm_get_port_update_state(port > 0?(port - 1):0))  //�ж��Ƿ���������
			{
				logout(INFO, "system", "�����˿�", "�����˿ڡ�%d��ʧ��,��е����������\r\n",display_port);
				result = NEW_PROTOCOL_ERR_PORT_UPDATING;
				new_protocol_err_back(sockfd, result, parm);
				return result; 
			}
		}
		else
		{
			if(0 < strlen(usb_port_class->online_bank_action.mechanical_arm_action.arm_data[port > 0?(port - 1):0].port_info.arm_version))
			{
				if(0 > usb_port_class->online_bank_action.mechanical_arm_action.class->fun_arm_get_port_update_state(port > 0?(port - 1):0))  //�ж��Ƿ���������
				{
					logout(INFO, "system", "����������е��", "��е�ۺš�%d��,������е��ʧ�ܣ���е����������\r\n", display_port);
					result = NEW_PROTOCOL_ERR_PORT_UPDATING;
					new_protocol_err_back(sockfd, result, parm);
					return result; 
				}
			}
		}
		result = usb_port_class->power_action.fun_power((port-1>0?port-1:0),RETART_ACTION);
		//restart_port(port);

		cJSON *dir1, *dir2;		//���ʹ��
		cJSON *json = cJSON_CreateObject();
		cJSON_AddItemToObject(json, "head", dir1 = cJSON_CreateObject());
		cJSON_AddStringToObject(dir1, "protocol", NEW_USB_SHARE_PROTOCOL);
		cJSON_AddStringToObject(dir1, "code_type", NEW_USB_SHARE_SYNC_CODE_RESPONSE);
		cJSON_AddStringToObject(dir1, "cmd", parm->cmd);
		cJSON_AddStringToObject(dir1, "source_topic", parm->source_topic);
		cJSON_AddStringToObject(dir1, "random", parm->random);
		if(result < 0)
		{
			cJSON_AddStringToObject(dir1, "result", "f");
			logout(INFO, "system", "�����˿�", "�����˿ڡ�%d��ʧ��\r\n",display_port);
		}	
		else
		{
			cJSON_AddStringToObject(dir1, "result", "s");
			logout(INFO, "system", "�����˿�", "�����˿ڡ�%d���ɹ�\r\n",display_port);
		}
			
		int data_len;
		g_buf = cJSON_PrintUnformatted(json);
		printf("_____g_buf len = %d", strlen(g_buf));
		printf("_____g_buf data = %s", g_buf);
		str_replace(g_buf, "\\\\", "\\");

		data_len = strlen(g_buf);
		sendbuf = calloc(1, data_len + 128);
		memcpy(sendbuf, "\x01\x06\x00\x07", 4);//Э��ͷ
		sendbuf[4] = strlen(g_buf) / 0x1000000; sendbuf[5] = strlen(g_buf) / 0x10000;
		sendbuf[6] = strlen(g_buf) / 256; sendbuf[7] = strlen(g_buf) % 256;
		//sprintf((char*)sendbuf + 4, "%04d", strlen(g_buf) + 4);//����λ
		memcpy(sendbuf + 8, g_buf, strlen(g_buf));
		len = strlen(g_buf) + 8;

		free(g_buf);
		cJSON_Delete(json);
		cJSON_Delete(root);

		result = ty_socket_write(sockfd, sendbuf, len);
		if (result < 0)
		{
			free(sendbuf);
			return result;

		}
		free(sendbuf);
		
		return 0;

	}
	else
	{
		// display_port = port;
		//port = port*2 - 1;

		port = port-1>0?port-1:0;
		printf("_____________port111 = %d\n",port);


		result = *((int *)(usb_port_class->fun_check_with_robot_arm(NULL)));   //���ж��Ƿ��ǻ�е�۵Ļ���
		if(result)
		{
			if(0 > usb_port_class->online_bank_action.mechanical_arm_action.class->fun_arm_get_port_update_state(port))  //�ж��Ƿ���������
			{
				logout(INFO, "system", "����������е��", "��е�ۺš�%d��,������е��ʧ�ܣ���е����������\r\n", display_port);
				result = NEW_PROTOCOL_ERR_PORT_UPDATING;
				new_protocol_err_back(sockfd, result, parm);
				return result; 
			}
		}
		else
		{
			if(0 < strlen(usb_port_class->online_bank_action.mechanical_arm_action.arm_data[port].port_info.arm_version))
			{
				if(0 > usb_port_class->online_bank_action.mechanical_arm_action.class->fun_arm_get_port_update_state(port))  //�ж��Ƿ���������
				{
					logout(INFO, "system", "����������е��", "��е�ۺš�%d��,������е��ʧ�ܣ���е����������\r\n", display_port);
					result = NEW_PROTOCOL_ERR_PORT_UPDATING;
					new_protocol_err_back(sockfd, result, parm);
					return result; 
				}
			}
		}

		logout(INFO, "system", "����������е��", "��е�ۺš�%d��,������е��\r\n", display_port);

		
		cJSON *dir1, *dir2;		//���ʹ��
		cJSON *json = cJSON_CreateObject();
		cJSON_AddItemToObject(json, "head", dir1 = cJSON_CreateObject());
		cJSON_AddStringToObject(dir1, "protocol", NEW_USB_SHARE_PROTOCOL);
		cJSON_AddStringToObject(dir1, "code_type", NEW_USB_SHARE_SYNC_CODE_RESPONSE);
		cJSON_AddStringToObject(dir1, "cmd", parm->cmd);
		cJSON_AddStringToObject(dir1, "source_topic", parm->source_topic);
		cJSON_AddStringToObject(dir1, "random", parm->random);

		printf("lbc111 %d\n",(port));
		
		if(0 < strlen(usb_port_class->online_bank_action.mechanical_arm_action.arm_data[port].port_info.arm_version))
		{
			result = 0;
		}
		else
		{
			result = -1;
		}
		if(result == 0)
		{
			cJSON_AddStringToObject(dir1, "result", "s");	 //��ѹ�ɹ�
			logout(INFO, "system", "����������е��", "��е�ۺš�%d��,��ѹ�ɹ�\r\n", display_port);
		}
			
		else if(result == -1)
		{
			cJSON_AddStringToObject(dir1, "result", "f01");  //��ѹʧ�ܣ����ܻ�е�������豸
			logout(INFO, "system", "����������е��", "��е�ۺš�%d��,��ѹʧ�ܣ��˶˿��޻�е�ۻ��߻�е�۹���\r\n", display_port);
		}
		// else if(result == -2)
		// {
		// 	cJSON_AddStringToObject(dir1, "result", "f02");	 //��е������ʧ��
		// 	logout(INFO, "system", "����������е��", "�˿ںš�%d��,��е������ʧ��\r\n", port + 1);
		// }
			
			
		int data_len;
		printf("!!!!!!!!!!!!!!lbc  exit get_usb_port_class 666666666666666\n");
		g_buf = cJSON_PrintUnformatted(json);
		printf("g_buf  = %s\n", g_buf);
		//printf("!!!!!!!!!!!!!!lbc  exit get_usb_port_class 777777777\n");
		str_replace(g_buf, "\\\\", "\\");

		data_len = strlen(g_buf);
		sendbuf = calloc(1, data_len + 128);
		memcpy(sendbuf, "\x01\x06\x00\x07", 4);//Э��ͷ
		sendbuf[4] = strlen(g_buf) / 0x1000000; sendbuf[5] = strlen(g_buf) / 0x10000;
		sendbuf[6] = strlen(g_buf) / 256; sendbuf[7] = strlen(g_buf) % 256;
		//sprintf((char*)sendbuf + 4, "%04d", strlen(g_buf) + 4);//����λ
		memcpy(sendbuf + 8, g_buf, strlen(g_buf));
		len = strlen(g_buf) + 8;

		if(g_buf != NULL)
			free(g_buf);
		cJSON_Delete(json);
		cJSON_Delete(root);

		result = ty_socket_write(sockfd, sendbuf, len);
		if (result < 0)
		{
			free(sendbuf);
			return result;

		}
		free(sendbuf);

		printf("port222 = %d\n",port);
		if(0 < strlen(usb_port_class->online_bank_action.mechanical_arm_action.arm_data[port].port_info.arm_version))
		{
			result = *((int *)(usb_port_class->online_bank_action.mechanical_arm_action.arm_start((void *)(&port))));
			printf("result = %d\n",result);
			if(result == 0)
				result = 1;
			else if(result == -1)
				result = 0;
			else
				result = 0;
			if(usb_port_class->online_bank_action.mechanical_arm_action.arm_with_dev[port].arm_with_dev_result != result)
			{
				//usb_port_class->get_dev_basic_action.dev_basic_data[(port+1)/2*2+1].up_report = 1;   //�ϱ�һ�λ�е���������豸
				usb_port_class->online_bank_action.mechanical_arm_action.arm_with_dev[port].arm_with_dev_result = result;  //״̬���µ�����
				//�����ϱ�������Ϣ
				usb_port_class->report_action.report_data.report_ter_flag_report = 0;
				usb_port_class->online_bank_action.mechanical_arm_action.arm_data[port].port = port + 1;
				usb_port_class->online_bank_action.mechanical_arm_action.arm_data[port].up_report = 1;
				usb_port_class->online_bank_action.mechanical_arm_action.arm_data[port].last_act = 1;
				

			}
		}

		printf("!!!!!!!!!!!!!!lbc  exit get_usb_port_class\n");
		//restart_port(port);

		logout(INFO, "system", "����������е��", "��е�ۺš�%d��,������е���������\r\n", display_port);
		return 0;
	}
	
}

static int check_net_delay(int sockfd, uint8 *json_in, struct json_parm_new_share *parm, void *output)
{
	int result = 0;
	uint8 *sendbuf = NULL;
	char *g_buf = NULL;
	int len = 0;
	cJSON *root, *array_obj,*arr_item,*item;

	cJSON *dir1, *dir2;		//���ʹ��
	cJSON *json = cJSON_CreateObject();
	cJSON_AddItemToObject(json, "head", dir1 = cJSON_CreateObject());
	cJSON_AddStringToObject(dir1, "protocol", NEW_USB_SHARE_PROTOCOL);
	cJSON_AddStringToObject(dir1, "code_type", NEW_USB_SHARE_SYNC_CODE_RESPONSE);
	cJSON_AddStringToObject(dir1, "cmd", parm->cmd);
	cJSON_AddStringToObject(dir1, "source_topic", parm->source_topic);
	cJSON_AddStringToObject(dir1, "random", parm->random);
	cJSON_AddStringToObject(dir1, "result", "s");
	
	int data_len;
	g_buf = cJSON_PrintUnformatted(json);
	deal_cmd_newshare_out("g_buf len = %d", strlen(g_buf));
	deal_cmd_newshare_out("g_buf data = %s", g_buf);
	str_replace(g_buf, "\\\\", "\\");

	data_len = strlen(g_buf);
	sendbuf = calloc(1, data_len + 128);
	memcpy(sendbuf, "\x01\x06\x00\x07", 4);//Э��ͷ
	sendbuf[4] = strlen(g_buf) / 0x1000000; sendbuf[5] = strlen(g_buf) / 0x10000;
	sendbuf[6] = strlen(g_buf) / 256; sendbuf[7] = strlen(g_buf) % 256;
	//sprintf((char*)sendbuf + 4, "%04d", strlen(g_buf) + 4);//����λ
	memcpy(sendbuf + 8, g_buf, strlen(g_buf));
	len = strlen(g_buf) + 8;

	free(g_buf);
	cJSON_Delete(json);
	//ֻ��һ����ͷ
	result = ty_socket_write(sockfd, sendbuf, len);
	if (result < 0)
	{
		free(sendbuf);
		return result;

	}
	free(sendbuf);
	return 0;
}


/***********���������ļ��ϴ�����**************/
static int intranet_net_upgrade(int sockfd, uint8 *json_in, struct json_parm_new_share *parm, void *output)
{

	cJSON *root = NULL;
	cJSON *array_obj = NULL;
	cJSON *arr_item, *item;
	int len = 0;
	char *g_buf = NULL;
	uint8 sendbuf[1024 * 5] = { 0 };
	char notice_type[40] = { 0 };
	char ter_id[40] = { 0 };
	char url_buff[128] = {0x00};
	unsigned char *filedata = NULL;
	int result;
	USB_PORT_CLASS *usb_port_class = get_usb_port_class();  //��ȡusb_port�Ĳ���ָ��

	//printf("######################			0110-19		intranet_net_upgrade	######################\r\n");
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
	printf("ter_id: %s\r\n",ter_id);


	item = cJSON_GetObjectItem(arr_item, "notice_type");
	if (item == NULL)
	{
		cJSON_Delete(root);
		return -1;
	}
	memcpy(notice_type, item->valuestring, strlen(item->valuestring));
	printf("notice_type: %s\r\n",notice_type);

	item = cJSON_GetObjectItem(arr_item, "url_buff");
	if (item == NULL)
	{
		cJSON_Delete(root);
		return -1;
	}
	memcpy(url_buff, item->valuestring, strlen(item->valuestring));
	printf("url_buff: %s\r\n",url_buff);


	struct _ter_info ter_info;
	memset(&ter_info, 0, sizeof(struct _ter_info));
	ty_ctl(usb_port_class->module->machine_fd, DF_MACHINE_INFOR_CM_GET_INFOMATION, &ter_info);
	printf("ter_info.ter_id : %s	\r\n",ter_info.ter_id);

	if (strcmp(ter_info.ter_id, ter_id) != 0)
	{
		// sprintf(output, "������Ų�һ�£��ն˻����ܾ�");
		// logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,֪ͨ�ն˻�ȡ��������Ȩ�ļ�,%s\r\n", message_line->parm.random, output);
		// cJSON_Delete(root);
		// mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, output);
		// goto End;
		cJSON_Delete(root);
		return -1;
	}

	if (strcmp(notice_type, "auth") == 0)
		;//write_file("/etc/need_update_auth.log", "������Ҫ��ϵͳ������Ȩ�ļ�", strlen("������Ҫ��ϵͳ������Ȩ�ļ�"));
	else if (strcmp(notice_type, "update") == 0)
	{
		printf("get updata url\r\n");
		// write_file("/etc/need_update_auth.log", "������Ҫ��ϵͳ���������ļ�", strlen("������Ҫ��ϵͳ���������ļ�"));
		write_file("/etc/need_update_url_app.txt", url_buff, strlen(url_buff));
	}
	else
	{
		// sprintf(errinfo, "֪ͨ���ʹ���,�޷�֧��");
		// logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,֪ͨ�ն˻�ȡ��������Ȩ�ļ�,%s\r\n", message_line->parm.random, output);
		// cJSON_Delete(root);
		// mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, output);
		// goto End;
		cJSON_Delete(root);
		return -1;
	}
	printf("intranet_net_upgrade Notification completed !\r\n");

	cJSON *dir1, *dir2;		//���ʹ��
	int flag_ok = 0;
	cJSON *json = cJSON_CreateObject();
	cJSON_AddItemToObject(json, "head", dir1 = cJSON_CreateObject());
	cJSON_AddStringToObject(dir1, "protocol", NEW_USB_SHARE_PROTOCOL);
	cJSON_AddStringToObject(dir1, "code_type", NEW_USB_SHARE_SYNC_CODE_RESPONSE);
	cJSON_AddStringToObject(dir1, "cmd", parm->cmd);
	cJSON_AddStringToObject(dir1, "source_topic", parm->source_topic);
	cJSON_AddStringToObject(dir1, "random", parm->random);
	// if (result < 0)//��ѹ����ʧ��
	// 	cJSON_AddStringToObject(dir1, "result", "f");
	// else
	// {
	// 	flag_ok = 1;
	// 	cJSON_AddStringToObject(dir1, "result", "s");
	// }
	cJSON_AddStringToObject(dir1, "result", "s");
		

	cJSON_AddItemToObject(json, "data", dir2 = cJSON_CreateArray());
	cJSON *dir3;
	cJSON_AddItemToObject(dir2, "dira", dir3 = cJSON_CreateObject());
	cJSON_AddStringToObject(dir3, "ter_id", ter_id);

	g_buf = cJSON_Print(json);
	printf("g_buf  = %s\n", g_buf);

	memcpy(sendbuf, "\x01\x06\x00\x07", 4);//Э��ͷ
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

// static int check_net_delay(int sockfd, uint8 *json_in, struct json_parm_new_share *parm, void *output)
// {
// 	cJSON *array_obj = NULL;
// 	cJSON *root = NULL;
// 	cJSON *arr_item = NULL, *item = NULL;
// 	int result = 0,port = 0;
// 	char ip_data[64] = {0x00};
// 	uint8 *sendbuf = NULL;
// 	char *g_buf = NULL;
// 	int len = 0,delay_us = 0;

// 	root = cJSON_Parse((char*)json_in);
// 	if (root == NULL)
// 	{
// 		return -1;
// 	}
// 	array_obj = cJSON_GetObjectItem(root, "data");
// 	if (array_obj == NULL)
// 	{
// 		cJSON_Delete(root);
// 		return -1;
// 	}
// 	arr_item = cJSON_GetArrayItem(array_obj, 0);
// 	if (arr_item == NULL)
// 	{
// 		cJSON_Delete(root);
// 		return -1;
// 	}
// 	item = cJSON_GetObjectItem(arr_item, "ip");
// 	if (item == NULL)
// 	{
// 		cJSON_Delete(root);
// 		return -1;
// 	}

// 	memcpy(ip_data,item->valuestring,strlen(item->valuestring));
// 	delay_us = get_net_delay(ip_data);
// 	//printf("!!!!!!!!!!!!!!lbc  exit get_usb_port_class\n");
// 	//restart_port(port);

// 	cJSON *dir1, *dir2;		//���ʹ��
// 	cJSON *json = cJSON_CreateObject();
// 	cJSON_AddItemToObject(json, "head", dir1 = cJSON_CreateObject());
// 	cJSON_AddStringToObject(dir1, "protocol", NEW_USB_SHARE_PROTOCOL);
// 	cJSON_AddStringToObject(dir1, "code_type", NEW_USB_SHARE_SYNC_CODE_RESPONSE);
// 	cJSON_AddStringToObject(dir1, "cmd", parm->cmd);
// 	cJSON_AddStringToObject(dir1, "source_topic", parm->source_topic);
// 	cJSON_AddStringToObject(dir1, "random", parm->random);
// 	cJSON_AddStringToObject(dir1, "result", "s");
		
// 	cJSON_AddItemToObject(json, "data", dir2 = cJSON_CreateArray());
		
// 	cJSON *data_layer = cJSON_CreateObject();
// 	cJSON_AddItemToObject(dir2, "dirl", data_layer);

	

// 	int data_len;
// 	//printf("!!!!!!!!!!!!!!lbc  exit get_usb_port_class 666666666666666\n");
// 	g_buf = cJSON_PrintUnformatted(json);
// 	//printf("g_buf  = %s\n", g_buf);
// 	//printf("!!!!!!!!!!!!!!lbc  exit get_usb_port_class 777777777\n");
// 	str_replace(g_buf, "\\\\", "\\");

// 	data_len = strlen(g_buf);
// 	sendbuf = calloc(1, data_len + 128);
// 	memcpy(sendbuf, "\x01\x06\x00\x07", 4);//Э��ͷ
// 	sendbuf[4] = strlen(g_buf) / 0x1000000; sendbuf[5] = strlen(g_buf) / 0x10000;
// 	sendbuf[6] = strlen(g_buf) / 256; sendbuf[7] = strlen(g_buf) % 256;
// 	//sprintf((char*)sendbuf + 4, "%04d", strlen(g_buf) + 4);//����λ
// 	memcpy(sendbuf + 8, g_buf, strlen(g_buf));
// 	len = strlen(g_buf) + 8;

// 	if(g_buf != NULL)
// 		free(g_buf);
// 	cJSON_Delete(json);
// 	cJSON_Delete(root);

// 	result = ty_socket_write(sockfd, sendbuf, len);
// 	if (result < 0)
// 	{
// 		free(sendbuf);
// 		return result;

// 	}
// 	free(sendbuf);
// 	return 0;
// }


/**********����json ͷ********/
static int analysis_json_head_new_share(char *inbuf, struct json_parm_new_share *parm,void *output)
{
	int result;
	cJSON *item;
	cJSON *root = cJSON_Parse(inbuf);
	if (!root)
	{
		deal_cmd_newshare_out("Error before cJSON_Parse: [%s]\n", cJSON_GetErrorPtr());
		return -1;
	}
	else
	{
		cJSON *object = cJSON_GetObjectItem(root, "head");
		if (object == NULL)
		{
			deal_cmd_newshare_out("Error before head: [%s]\n", cJSON_GetErrorPtr());
			cJSON_Delete(root);
			return -1;
		}
		//printf("cJSON_GetObjectItem: type=%d, key is %s, value is %s\n",object->type,object->string,object->valuestring);  

		if (object != NULL)
		{
			deal_cmd_newshare_out("analysis_json_head_new_share ��ȡЭ��汾\n");
			item = cJSON_GetObjectItem(object, "protocol");
			if (item != NULL)
			{
				deal_cmd_newshare_out("cJSON_GetObjectItem: type=%d, string is %s, valuestring=%s\n", item->type, item->string, item->valuestring);
				memcpy(parm->protocol, item->valuestring, strlen(item->valuestring));
			}
			result = memcmp(parm->protocol, NEW_USB_SHARE_PROTOCOL, strlen(parm->protocol));
			if (result != 0)
			{
				deal_cmd_newshare_out("Э��汾��һ��\n");
				cJSON_Delete(root);
				return -1;
			}
			///////////////////////////////////////////////////////////////////
			deal_cmd_newshare_out("analysis_json_head_new_share ��ȡ��������\n");
			item = cJSON_GetObjectItem(object, "code_type");
			if (item != NULL)
			{
				deal_cmd_newshare_out("cJSON_GetObjectItem: type=%d, string is %s, valuestring=%s\n", item->type, item->string, item->valuestring);
				memcpy(parm->code_type, item->valuestring, strlen(item->valuestring));
			}
			deal_cmd_newshare_out("parm->code_type = %s\n", parm->code_type);
			if (memcmp(parm->code_type, NEW_USB_SHARE_SYNC_CODE_REQUEST, strlen(parm->code_type)) != 0)
			{
				deal_cmd_newshare_out("��Ϣ�������\n");
				cJSON_Delete(root);
				return -1;
			}
			/////////////////////////////////////////////////////////////////
			deal_cmd_newshare_out("analysis_json_head_new_share ��ȡԴID\n");
			item = cJSON_GetObjectItem(object, "source_topic");
			if (item != NULL)
			{
				deal_cmd_newshare_out("cJSON_GetObjectItem: type=%d, string is %s, valuestring=%s\n", item->type, item->string, item->valuestring);
				memcpy(parm->source_topic, item->valuestring, strlen(item->valuestring));
			}
			deal_cmd_newshare_out("analysis_json_head_new_share ��ȡԴID�ɹ� ��%s\n", parm->source_topic);
			////////////////////////////////////////////////////////////////

			deal_cmd_newshare_out("analysis_json_head_new_share ��ȡ������\n");
			item = cJSON_GetObjectItem(object, "cmd");
			if (item != NULL)
			{
				deal_cmd_newshare_out("cJSON_GetObjectItem: type=%d, string is %s, valueint=%d\n", item->type, item->string, item->valueint);
				//memcpy(parm->cmd,item->valuestring,strlen(item->valuestring)); 
				memcpy(parm->cmd, item->valuestring, strlen(item->valuestring));

			}
			//*cmd = atoi(parm->cmd);

			deal_cmd_newshare_out("analysis_json_head_new_share ��ȡ��������������\n");
			item = cJSON_GetObjectItem(object, "random");
			if (item != NULL)
			{
				//printf("cJSON_GetObjectItem: type=%d, string is %s, valueint=%d\n",item->type,item->string,item->valueint);  
				//memcpy(parm->random,item->valuestring,strlen(item->valuestring)); 
				memcpy(parm->random, item->valuestring, strlen(item->valuestring));
				deal_cmd_newshare_out("��ȡ������ɹ� �� %s\n", parm->random);

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


/*********�½ӿڴ��󷵻�*********/
int new_protocol_err_back(int sockfd, int err_no, struct json_parm_new_share *parm)
{
	int i;
	char errcode[10] = { 0 };
	unsigned char sendbuf[500 * 2] = { 0 };
	char errinfo[100] = { 0 };
	memcpy(sendbuf, "\x01\x06\x00\x07", 4);//Э��
	for (i = 0; i<sizeof(new_protocol_err_n) / sizeof(new_protocol_err_n[0]); i++)
	{
		if (new_protocol_err_n[i].err_no == err_no)
			break;
	}
	if (i == sizeof(new_protocol_err_n) / sizeof(new_protocol_err_n[0]))
	{
		deal_cmd_newshare_out("���������δ�������ô�������\n");
		//return err_out_s(fd, 255, (uint8 *)"δ֪����");
		sprintf(errinfo, "Unknown err!");
	}
	else
	{
		sprintf(errcode, "%d", err_no);
		sprintf(errinfo, "%s",new_protocol_err_n[i].errinfo);
	}
	char *g_buf;
	int len,result;
	cJSON *dir1, *dir2;		//���ʹ��
	cJSON *json = cJSON_CreateObject();
	cJSON_AddItemToObject(json, "head", dir1 = cJSON_CreateObject());
	cJSON_AddStringToObject(dir1, "protocol", NEW_USB_SHARE_PROTOCOL);
	cJSON_AddStringToObject(dir1, "code_type", NEW_USB_SHARE_SYNC_CODE_RESPONSE);
	cJSON_AddStringToObject(dir1, "cmd", parm->cmd);
	cJSON_AddStringToObject(dir1, "source_topic", parm->source_topic);
	cJSON_AddStringToObject(dir1, "random", parm->random);
	// if(err_no == NEW_PROTOCOL_ERR_PORT_ERR)
	// 	cJSON_AddStringToObject(dir1, "result", "f02");
	// else
		cJSON_AddStringToObject(dir1, "result", "f");

	cJSON_AddItemToObject(json, "data", dir2 = cJSON_CreateArray());
	cJSON *errlayer;
	cJSON_AddItemToObject(dir2, "dira", errlayer = cJSON_CreateObject());

	cJSON_AddStringToObject(errlayer, "err_code", errcode);
	cJSON_AddStringToObject(errlayer, "err_info", errinfo);
	g_buf = cJSON_Print(json);

	memcpy(sendbuf, "\x01\x06\x00\x07", 4);//Э��ͷ
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

/*********���ݲ�ָͬ�����Ӧ����*******/
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
	deal_cmd_newshare_out("parm.cmd =%s\n", parm.cmd);
	for (i = 0; i<sizeof(new_shr_order) / sizeof(new_shr_order[0]); i++)
	{
		if (memcmp(new_shr_order[i].order,parm.cmd,strlen(parm.cmd) )== 0)
		{   //deal_cmd_newshare_out("ִ��:[%s]\n",order_name[i].name);
			result = new_shr_order[i].answer(sockfd, json_in, &parm, output);
			
			if (result < 0)
			{
				if (err_back_flag == 1)//��ʾ��Ҫ�˴�����json��TCP���󷵻�
				{
					return new_protocol_err_back(sockfd, result, &parm);
				}
			}

			return result;//ͳһ���󷵻��ⲿ����
		
		}
	}
	return 0;
}

/*******************socket ͨѶ***************************/

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