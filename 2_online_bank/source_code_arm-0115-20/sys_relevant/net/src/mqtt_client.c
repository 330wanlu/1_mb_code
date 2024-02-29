#include "../inc/mqtt_client.h"

struct _mqtt_parms  mqtt_client;
struct _mqtt_parms  mqtt_devops;

static void task_pub(void *arg, int timer);
static void task_sub(void *arg, int timer);

int _mqtt_client_init(void *function,void *err_deal_function ,char *server, int port, char *topic, char *user_name, char *user_passwd, char *sub_id, char *pub_id)
{   
#ifdef PRODUCE_PROGRAM
	memset(&mqtt_client, 0, sizeof(struct _mqtt_parms));
	return 0;
#endif
	memset(&mqtt_client, 0, sizeof(struct _mqtt_parms));
	mosquitto_share_init(&mqtt_client);
	sprintf(mqtt_client.name,"mqtt_client");
	mqtt_client.hook = function;
	mqtt_client.err_hook = err_deal_function;
	memcpy(mqtt_client.server_address, server, strlen(server));
	mqtt_client.server_port = port;
	memcpy(mqtt_client.topic, topic, strlen(topic));
	memcpy(mqtt_client.user_name, user_name, strlen(user_name));
	memcpy(mqtt_client.user_passwd, user_passwd, strlen(user_passwd));
	memcpy(mqtt_client.user_id_sub, sub_id, strlen(sub_id));
	memcpy(mqtt_client.user_id_pub, pub_id, strlen(pub_id));
	mqtt_client.pub_data.cfgp.need_tls = 1;
	mqtt_client.sub_data.cfgp.need_tls = 1;
	sprintf(mqtt_client.ca_cert, "/etc/ca.crt");
	sprintf(mqtt_client.client_cert, "/etc/client.crt");
	sprintf(mqtt_client.client_key, "/etc/client.key");
	mqtt_client.init_ok = 1;
	mqtt_client.machine_fd = ty_open("/dev/machine", 0, 0, NULL);
	mqtt_client.get_net_fd = ty_open("/dev/get_net_state", 0, 0, NULL);
	mqtt_client.task_sub = _scheduled_task_open("_mqtt_client.c", task_sub, &mqtt_client, 200, NULL);
	mqtt_client.task_pub = _scheduled_task_open("_mqtt_client.c", task_pub, &mqtt_client, 200, NULL);
    mqtt_client_out("MQTT消息处理客户端初始化成功\n");
    return 0;
}

struct _mqtt_parms *my_get_mqtt_parm(void)
{
	return &mqtt_client;
}

int _mqtt_devops_init(void *function, void *err_deal_function, char *server, int port, char *topic, char *user_name, char *user_passwd, char *sub_id, char *pub_id)
{
#ifdef PRODUCE_PROGRAM
	memset(&mqtt_devops, 0, sizeof(struct _mqtt_parms));
	return 0;
#endif
	memset(&mqtt_devops, 0, sizeof(struct _mqtt_parms));
	mosquitto_share_init(&mqtt_devops);
	sprintf(mqtt_devops.name, "mqtt_devops");
	mqtt_devops.hook = function;
	mqtt_devops.err_hook = err_deal_function;
	memcpy(mqtt_devops.server_address, server, strlen(server));
	mqtt_devops.server_port = port;
	memcpy(mqtt_devops.topic, topic, strlen(topic));
	memcpy(mqtt_devops.user_name, user_name, strlen(user_name));
	memcpy(mqtt_devops.user_passwd, user_passwd, strlen(user_passwd));
	memcpy(mqtt_devops.user_id_sub, sub_id, strlen(sub_id));
	memcpy(mqtt_devops.user_id_pub, pub_id, strlen(pub_id));
	mqtt_client.pub_data.cfgp.need_tls = 0;
	mqtt_client.sub_data.cfgp.need_tls = 0;
	//sprintf(mqtt_devops.ca_cert, "/etc/devops_ca.crt");
	//sprintf(mqtt_devops.client_cert, "/etc/devops_client.crt");
	//sprintf(mqtt_devops.client_key, "/etc/devops_client.key");
	mqtt_devops.init_ok = 1;
	mqtt_client.machine_fd = ty_open("/dev/machine", 0, 0, NULL);
	mqtt_devops.get_net_fd = ty_open("/dev/get_net_state", 0, 0, NULL);
	mqtt_devops.task_sub = _scheduled_task_open("_mqtt_client.c", task_sub, &mqtt_devops, 200, NULL);
	mqtt_devops.task_pub = _scheduled_task_open("_mqtt_client.c", task_pub, &mqtt_devops, 200, NULL);
	//mqtt_client_out("MQTT运维通道客户端初始化成功\n");
	return 0;
}

static int analysis_mqtt_connect_data_md5_data(char *data, char *server_addr, int *port, char *mqtt_uid, char *mqtt_pwd, char *crt_md5)
{
	char tmp[50] = { 0 };
	cJSON *root = NULL, *item;
	root = cJSON_Parse((const char *)data);
	if (root == NULL)
	{
		return -1;
	}
	item = cJSON_GetObjectItem(root, "server_addr");
	if (get_json_value_can_not_null(item, server_addr, 1, 100) < 0)
	{
		cJSON_Delete(root);
		return -2;
	}

	item = cJSON_GetObjectItem(root, "server_port");
	if (get_json_value_can_not_null(item, tmp, 1, 6) < 0)
	{
		cJSON_Delete(root);
		return -3;
	}
	*port = atoi(tmp);

	item = cJSON_GetObjectItem(root, "uid");
	if (get_json_value_can_not_null(item, mqtt_uid, 1, 20) < 0)
	{
		cJSON_Delete(root);
		return -4;
	}

	item = cJSON_GetObjectItem(root, "pwd");
	if (get_json_value_can_not_null(item, mqtt_pwd, 1, 50) < 0)
	{
		cJSON_Delete(root);
		return -5;
	}

	item = cJSON_GetObjectItem(root, "crt_md5");
	if (get_json_value_can_not_null(item, crt_md5, 1, 50) < 0)
	{
		cJSON_Delete(root);
		return -6;
	}

	mqtt_client_out("服务器反馈的当前mqtt证书文件md5值为：%s\n", crt_md5);
	cJSON_Delete(root);
	return 0;
}

int function_get_mqtt_connect_data_md5_jude_update(char *server_addr, int *server_port, char *mqtt_uid, char *mqtt_pwd)
{
	int ret;
	char *r_buf = NULL;
	char *outdata;
	int outdatalen;
	int r_len;
	char *json_data = NULL;
	USB_PORT_CLASS *stream = get_usb_port_class();
	struct _ter_info ter_info;

	mqtt_client_out("enter function_get_mqtt_connect_data_md5_jude_update111\n");

	memset(&ter_info, 0, sizeof(struct _ter_info));
	mqtt_client_out("enter function_get_mqtt_connect_data_md5_jude_update 222222222222222222222222.stream->module->machine_fd = %d\n",stream->module->machine_fd);
	ty_ctl(stream->module->machine_fd, DF_MACHINE_INFOR_CM_GET_INFOMATION, &ter_info);
    printf("-------------------stream->key_auth_state.key_s = %s\n",stream->key_auth_state.key_s);
	printf("-------------------ter_info.ter_id = %s\n",ter_info.ter_id);
	mb_enc_get_mqtt_connect_info_json(stream->key_auth_state.key_s, ter_info.ter_id, &json_data);

	if (json_data == NULL)
	{
		mqtt_client_out("组包数据有误\n");
		return -1;
	}

	struct _http_comon parm;
	memset(&parm, 0, sizeof(struct _http_comon));

#ifndef LBC_TEST
	sprintf(parm.s_ip, "%s", "middle.mbizj.com");			//新中台地址
	parm.s_port = 20020;	
#else
	sprintf(parm.s_ip, "%s", "192.168.0.139");
	parm.s_port = 8000;
#endif

	sprintf(parm.appid, "%s", DF_ERR_SERVER_APPID);
	sprintf(parm.appsecret, "%s", DF_ERR_SERVER_APPSECERT);
	sprintf((char*)parm.timestamp, "%ld", get_time_sec());

	//g_buf = cJSON_Print(json);

	// mqtt_client_out("enter function_get_mqtt_connect_data_md5_jude_update,parm->s_ip=%s,parm->s_port=%d\n",parm->s_ip,parm->s_port);
	ret = http_token_get_mqtt_connect_md5(&parm, json_data, strlen(json_data), &r_buf, &r_len);
	if (ret < 0)
	{
		mqtt_client_out("function_get_mqtt_connect_data_md5_jude_update 与服务器交互失败,ret = %d\n",ret);
		free(json_data);
		return ret;
	}
	if(NULL != json_data)
		free(json_data);
	mb_dec_file_function(stream->key_auth_state.key_s, r_buf, strlen(r_buf), &outdata, &outdatalen);
	if(NULL != r_buf)
		free(r_buf);


	char crt_md5[200] = { 0 };
	ret = analysis_mqtt_connect_data_md5_data(outdata, server_addr, server_port, mqtt_uid, mqtt_pwd, crt_md5);
	if (ret < 0)
	{
		mqtt_client_out("function_get_mqtt_connect_data_md5_jude_update 返回json数据解析失败,ret = %d,data %s\n", ret, outdata);
		free(outdata);
		return -1;
	}
	free(outdata);
	char local_file[81920] = { 0 };
	read_file("/etc/ca.crt", local_file + strlen(local_file), sizeof(local_file)-strlen(local_file));
	read_file("/etc/client.crt", local_file + strlen(local_file), sizeof(local_file)-strlen(local_file));
	read_file("/etc/client.key", local_file + strlen(local_file), sizeof(local_file)-strlen(local_file));

	//mqtt_client_out("local_file %s", local_file);

	char md5_data[200] = { 0 };
	md5_value(local_file, strlen(local_file), md5_data);


	mqtt_client_out("开票服务器本地mqtt证书文件md5值为：%s\n", md5_data);

	if (strcmp(md5_data, crt_md5) == 0)
	{
		mqtt_client_out("本地证书文件与服务器证书文件md5值一致,不需要更新\n");
		return 0;
	}

	mqtt_client_out("本地证书文件与服务器证书文件md5值不一致,需要更新\n");
	return 1;
}

static int save_cert_file(char *base_file, char *path)
{
	char cmd[1024] = {0};
	sprintf(cmd, "rm %s", path);
	system(cmd);
	write_file(path, base_file, strlen(base_file));
	sync();
	return 0;
}

static int analysis_mqtt_info_data(char *data, char *server_addr, int *port, char *uid, char *pwd)
{

	char protocol[50];
	char code_type[10];
	char cmd[50];
	char result_s[2];
	char mqtt_server[200];
	char mqtt_uid[20];
	char mqtt_pwd[50];
	char mqtt_port[6];
	char ca_cert_base[4000];
	char client_cert_base[4000];
	char client_key_base[4000];

	memset(protocol, 0, sizeof(protocol));
	memset(code_type, 0, sizeof(code_type));
	memset(cmd, 0, sizeof(cmd));
	memset(result_s, 0, sizeof(result_s));
	memset(mqtt_server, 0, sizeof(mqtt_server));
	memset(mqtt_port, 0, sizeof(mqtt_port));
	cJSON *item;
	cJSON *root = cJSON_Parse(data);
	if (!root)
	{
		mqtt_client_out("Error before: [%s]\n", cJSON_GetErrorPtr());
		return -1;
	}

	item = cJSON_GetObjectItem(root, "server_addr");
	if (item == NULL)
	{
		cJSON_Delete(root);
		return -1;
	}
	if (item->valuestring != NULL)
	{
		memset(mqtt_server, 0, sizeof(mqtt_server));
		memcpy(mqtt_server, item->valuestring, strlen(item->valuestring));
		memcpy(server_addr, mqtt_server, strlen(item->valuestring));
	}
	mqtt_client_out("获取到的MQTT服务器地址为%s\n", mqtt_server);
	item = cJSON_GetObjectItem(root, "server_port");
	if (item == NULL)
	{
		cJSON_Delete(root);
		return -1;
	}
	if (item->valuestring != NULL)
	{
		memset(mqtt_port, 0, sizeof(mqtt_port));
		memcpy(mqtt_port, item->valuestring, strlen(item->valuestring));
		*port = atoi(mqtt_port);
	}
	mqtt_client_out("获取到的MQTT服务器端口号为%s\n", mqtt_port);



	item = cJSON_GetObjectItem(root, "uid");
	if (item == NULL)
	{
		cJSON_Delete(root);
		return -1;
	}
	if (item->valuestring != NULL)
	{
		memset(mqtt_uid, 0, sizeof(mqtt_pwd));
		memcpy(mqtt_uid, item->valuestring, strlen(item->valuestring));
		memcpy(uid, mqtt_uid, strlen(item->valuestring));
	}
	mqtt_client_out("获取到的MQTT uid%s\n", uid);
	item = cJSON_GetObjectItem(root, "pwd");
	if (item == NULL)
	{
		cJSON_Delete(root);
		return -1;
	}
	if (item->valuestring != NULL)
	{
		memset(mqtt_pwd, 0, sizeof(mqtt_pwd));
		memcpy(mqtt_pwd, item->valuestring, strlen(item->valuestring));
		memcpy(pwd, mqtt_pwd, strlen(item->valuestring));
	}
	mqtt_client_out("获取到的MQTT pwd%s\n", pwd);



	item = cJSON_GetObjectItem(root, "ca_crt");
	if (item == NULL)
	{
		cJSON_Delete(root);
		return -1;
	}
	if (item->valuestring != NULL)
	{
		memset(ca_cert_base, 0, sizeof(ca_cert_base));
		memcpy(ca_cert_base, item->valuestring, strlen(item->valuestring));
	}
	//mqtt_client_out("获取到的ca.crt证书文件为%s\n", ca_cert_base);

	item = cJSON_GetObjectItem(root, "client_crt");
	if (item == NULL)
	{
		cJSON_Delete(root);
		return -1;
	}
	if (item->valuestring != NULL)
	{
		memset(client_cert_base, 0, sizeof(client_cert_base));
		memcpy(client_cert_base, item->valuestring, strlen(item->valuestring));
	}
	//mqtt_client_out("获取到的client.crt证书文件为%s\n", client_cert_base);

	item = cJSON_GetObjectItem(root, "client_key");
	if (item == NULL)
	{
		cJSON_Delete(root);
		return -1;
	}
	if (item->valuestring != NULL)
	{
		memset(client_key_base, 0, sizeof(client_key_base));
		memcpy(client_key_base, item->valuestring, strlen(item->valuestring));
	}
	//mqtt_client_out("获取到的client.key证书文件为%s\n", client_key_base);

	char tmp[4000];
	memset(tmp, 0, sizeof(tmp));
	if (read_file("/etc/ca.crt", tmp, sizeof(tmp))<0)
	{
		mqtt_client_out("获取文件失败:[%s]\n", "/etc/ca.crt");
		save_cert_file(ca_cert_base, "/etc/ca.crt");
	}
	else
	{
		if (memcmp(ca_cert_base, tmp, sizeof(tmp)) != 0)
		{
			mqtt_client_out("ca.cert文件不一致需要更新\n");
			save_cert_file(ca_cert_base, "/etc/ca.crt");
		}
	}
	memset(tmp, 0, sizeof(tmp));
	if (read_file("/etc/client.crt", tmp, sizeof(tmp))<0)
	{
		mqtt_client_out("获取文件失败:[%s]\n", "/etc/client.crt");
		save_cert_file(client_cert_base, "/etc/client.crt");
	}
	else
	{
		if (memcmp(client_cert_base, tmp, sizeof(tmp)) != 0)
		{
			mqtt_client_out("client.cert文件不一致需要更新\n");
			save_cert_file(client_cert_base, "/etc/client.crt");
		}
	}


	memset(tmp, 0, sizeof(tmp));
	if (read_file("/etc/client.key", tmp, sizeof(tmp))<0)
	{
		mqtt_client_out("获取文件失败:[%s]\n", "/etc/client.key");
		save_cert_file(client_key_base, "/etc/client.key");
	}
	else
	{
		if (memcmp(client_key_base, tmp, sizeof(tmp)) != 0)
		{
			mqtt_client_out("client.key文件不一致需要更新\n");
			save_cert_file(client_key_base, "/etc/client.key");
		}
	}
	cJSON_Delete(root);
	return 0;
}



int function_get_mqtt_connect_data_info_update(char *server_addr, int *server_port, char *mqtt_uid, char *mqtt_pwd)
{

	//timer_read_asc(ter_time);
	int ret;
	char *r_buf = NULL;
	char *outdata;
	int outdatalen;
	int r_len;
	char *json_data = NULL;
	USB_PORT_CLASS *stream = get_usb_port_class();
	struct _ter_info ter_info;

	ty_ctl(stream->module->machine_fd, DF_MACHINE_INFOR_CM_GET_INFOMATION, &ter_info);

	mb_enc_get_mqtt_connect_info_json(stream->key_auth_state.key_s, ter_info.ter_id, &json_data);
	struct _http_comon parm;
	memset(&parm, 0, sizeof(struct _http_comon));
	
	
	/*sprintf(parm.s_ip, "%s", DF_ERR_SERVER_ADDRESS);
	parm.s_port = DF_ERR_SERVER_PORT;*/

// #ifndef DF_OLD_MSERVER										//wang 添加对新老中台宏定义  ifndef未定义老中台宏定义
// 	sprintf(parm.s_ip, "%s", "middle.mbizj.com");			//新中台地址
// 	parm.s_port = 20020;	

// 	// sprintf(parm.s_ip, "%s", "192.168.0.139");
// 	// parm.s_port = 8000;
// #else
// 	sprintf(parm.s_ip, "%s", DF_ERR_SERVER_ADDRESS);
// 	parm.s_port = DF_ERR_SERVER_PORT;
// #endif
#ifndef LBC_TEST
	sprintf(parm.s_ip, "%s", "middle.mbizj.com");			//新中台地址
	parm.s_port = 20020;	
#else
	sprintf(parm.s_ip, "%s", "192.168.0.139");
	parm.s_port = 8000;
#endif

	sprintf(parm.appid, "%s", DF_ERR_SERVER_APPID);
	sprintf(parm.appsecret, "%s", DF_ERR_SERVER_APPSECERT);
	sprintf((char*)parm.timestamp, "%ld", get_time_sec());

	//g_buf = cJSON_Print(json);

	ret = http_token_get_mqtt_connect_info(&parm, json_data, strlen(json_data), &r_buf, &r_len);
	if (ret < 0)
	{
		mqtt_client_out("function_get_mqtt_connect_data_info_update 与服务器交互失败,ret = %d\n", ret);
		free(json_data);
		return ret;
	}
	if(NULL != json_data)
		free(json_data);
	mb_dec_file_function(stream->key_auth_state.key_s, r_buf, strlen(r_buf), &outdata, &outdatalen);
	if(NULL != r_buf)
		free(r_buf);


	analysis_mqtt_info_data(outdata, server_addr, server_port, mqtt_uid, mqtt_pwd);
	free(outdata);
	if (strcmp(mqtt_uid, ter_info.ter_id) != 0)
	{
		logout(INFO, "system", "mqtt信息获取", "服务器返回的终端ID与本机不符\r\n");
		return -1;
	}

	
	//=======================================================================================//
	char tmp[1024] = { 0 };
	char cmd[1024] = { 0 };
	sprintf(tmp, "#=====MQTT SERVER SET=====#\r\n  #MQTT SERVER ADDRESS\r\nserver_address = %s\r\n  #MQTT SERVER PORT\r\nserver_port = %d\r\n", server_addr, *server_port);
	sprintf(cmd, "rm %s", DF_CONFIG_FILE_PATH);
	system(cmd);
	write_file(DF_CONFIG_FILE_PATH, tmp, strlen(tmp));
	sync();

	return 0;
}


static void task_sub(void *arg, int timer)
{
	struct _mqtt_parms  *mqtt_parms;
	int result;
	mqtt_parms = arg;
	//char sub_id[100] = {0};
	//char time_s[20];

	struct _net_status net_status;
	memset(&net_status, 0, sizeof(struct _net_status));
	ty_ctl(mqtt_parms->get_net_fd, DF_GET_NET_STATE_CM_GET_NET_STATUS, &net_status);
	if (net_status.nettime_ok != 1)
	{
		return;
	}




//#ifdef MQTT_CTL
	memset(mqtt_parms->server_address, 0, sizeof(mqtt_parms->server_address));
	memset(mqtt_parms->topic, 0, sizeof(mqtt_parms->topic));
	memset(mqtt_parms->user_name, 0, sizeof(mqtt_parms->user_name));
	memset(mqtt_parms->user_passwd, 0, sizeof(mqtt_parms->user_passwd));
	memset(mqtt_parms->user_id_sub, 0, sizeof(mqtt_parms->user_id_sub));
	memset(mqtt_parms->user_id_pub, 0, sizeof(mqtt_parms->user_id_pub));
	mqtt_client.server_port = 0;

#ifndef LBC_TEST
	result = function_get_mqtt_connect_data_md5_jude_update(mqtt_parms->server_address, &mqtt_client.server_port, mqtt_parms->user_name, mqtt_parms->user_passwd);
	if (result < 0)
	{
		mqtt_client_out("与服务器交互失败,直接使用本地配置连接服务器\n");
		result = ty_ctl(mqtt_parms->machine_fd, DF_MACHINE_INFOR_CM_GET_MQTT_USER, mqtt_parms->server_address, &mqtt_parms->server_port, mqtt_parms->user_name, mqtt_parms->user_passwd);
		if (result < 0)
		{
			mqtt_client_out("获取本地配置失败\n");
			return;
		}
		sprintf(mqtt_parms->topic, "mb_fpkjxt_%s", mqtt_parms->user_name);
		sprintf(mqtt_parms->user_id_sub, "sub%s", mqtt_parms->user_name);
		sprintf(mqtt_parms->user_id_pub, "pub%s", mqtt_parms->user_name);
	}
	else if (result == 0)
	{
		mqtt_client_out("与服务器成功,服务器与当前配置一致无需更新\n");
		sprintf(mqtt_parms->topic, "mb_fpkjxt_%s", mqtt_parms->user_name);
		sprintf(mqtt_parms->user_id_sub, "sub%s", mqtt_parms->user_name);
		sprintf(mqtt_parms->user_id_pub, "pub%s", mqtt_parms->user_name);
	}
	else
	{
		mqtt_client_out("与服务器成功,服务器与当前配置不一致需更新后再连接\n");
		result = function_get_mqtt_connect_data_info_update(mqtt_parms->server_address, &mqtt_client.server_port, mqtt_parms->user_name, mqtt_parms->user_passwd);
		if (result < 0)
		{
			sleep(10);
			return;
		}
		sprintf(mqtt_parms->topic, "mb_fpkjxt_%s", mqtt_parms->user_name);
		sprintf(mqtt_parms->user_id_sub, "sub%s", mqtt_parms->user_name);
		sprintf(mqtt_parms->user_id_pub, "pub%s", mqtt_parms->user_name);
	}
#else
//#endif
	
	//mqtt_client_out("测试订阅任务是否会退出 task_sub\n");	
	//mqtt_client_out("task_sub server address %s\n", mqtt_parms->server_address);
	//mqtt_client_out("task_sub server port %d\n", mqtt_parms->server_port);
	//mqtt_client_out("task_sub topics %s\n", mqtt_parms->topic);
	//mqtt_client_out("task_sub user %s\n", mqtt_parms->user_name);
	//mqtt_client_out("task_sub passwd %s\n", mqtt_parms->user_passwd);
	//mqtt_client_out("task_sub usrid = %s\n", mqtt_parms->user_id_sub);
	


	result = ty_ctl(mqtt_parms->machine_fd, DF_MACHINE_INFOR_CM_GET_MQTT_USER, mqtt_parms->server_address, &mqtt_parms->server_port, mqtt_parms->user_name, mqtt_parms->user_passwd);
	if (result < 0)
	{
		mqtt_client_out("获取本地配置失败\n");
		return;
	}
	sprintf(mqtt_parms->topic, "mb_fpkjxt_%s", mqtt_parms->user_name);
	sprintf(mqtt_parms->user_id_sub, "sub%s", mqtt_parms->user_name);
	sprintf(mqtt_parms->user_id_pub, "pub%s", mqtt_parms->user_name);
#endif


	result = mqtt_sub_message(mqtt_parms);
	mqtt_client_out("退出 task_sub 消息订阅循环\n");
	if (result != 0)
	{
		//mqtt_client_out("task_sub 订阅消息失败,result = %d\n\n\n\n\n",result);
		sleep(10);
		mqtt_parms->err_hook(mqtt_parms, result);
		return;

	}
	sleep(10);
	mqtt_parms->err_hook(mqtt_parms, result);
	return;
}


static void task_pub(void *arg, int timer)
{
	struct _mqtt_parms  *mqtt_parms;
	mqtt_parms = arg;
	//mqtt_client_out("task_pub初始化pub，name = %s\n", mqtt_parms->name);
	mqtt_pub_message(mqtt_parms);
	return;
}



int _mqtt_client_pub(char *topic, char *message)
{
	int result;
	if (mqtt_client.init_ok != 1)
	{
		return -200;
	}
	result = mosquitt_pub_message(&mqtt_client, topic, message);
	return result;
}


int _mqtt_devops_pub(char *topic, char *message)
{
	int result;
	if (mqtt_devops.init_ok != 1)
	{
		return -200;
	}
	result = mosquitt_pub_message(&mqtt_devops, topic, message);
	return result;
}




int _mqtt_client_disconnect()
{
	mqtt_client_out("断开sub链路连接\n");
	mosquitto_disconnect(mqtt_client.sub_mosq);
	mqtt_client.sub_data.cfgp.sub_connect_ok = 0;
	//mosquitto_destroy(mqtt_client.sub_mosq);


	mqtt_client_out("断开pub链路连接\n");
	mosquitto_disconnect(mqtt_client.mosq);
	mqtt_client.pub_data.cfgp.pub_connect_ok = 0;
	//mosquitto_destroy(mqtt_client.mosq);
	return 0;
}

