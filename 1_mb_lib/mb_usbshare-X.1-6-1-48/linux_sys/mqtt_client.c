#define _mqtt_client_c
#include "mqtt_client.h"
//===================================//
//==========全局结构体===============//
struct _mqtt_parms  mqtt_client;

struct _mqtt_parms  mqtt_invoice;

int _mqtt_client_init(void *function,void *err_deal_function ,char *server, int port, char *topic, char *user_name, char *user_passwd, char *sub_id, char *pub_id)
{   
#ifdef PRODUCE_PROGRAM
	memset(&mqtt_client, 0, sizeof(struct _mqtt_parms));
	return 0;
#endif
	memset(&mqtt_client, 0, sizeof(struct _mqtt_parms));
	mosquitto_share_init(&mqtt_client);
	sprintf(mqtt_client.name,"mqtt_fpkjxt");
	mqtt_client.mqtt_type = DF_MQTT_FPKJXT;
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
#ifndef DF_1201_1
	sprintf(mqtt_client.ca_cert, "/etc/ca.crt");
	sprintf(mqtt_client.client_cert, "/etc/client.crt");
	sprintf(mqtt_client.client_key, "/etc/client.key");
#else
	#ifndef CA_1201
		sprintf(mqtt_client.ca_cert, "/etc/ca.crt");// 云端证书
		sprintf(mqtt_client.client_cert, "/etc/server.crt");
		sprintf(mqtt_client.client_key, "/etc/server.key");
	#else

		#ifndef CA_1201_37
			sprintf(mqtt_client.ca_cert, "/etc/ca.crt");
			sprintf(mqtt_client.client_cert, "/etc/client.crt");
			sprintf(mqtt_client.client_key, "/etc/client.key");
		#else
			sprintf(mqtt_client.ca_cert, "/home/zwl/work/zshu_37/ca.crt");   //阿里云测试证书使用
			sprintf(mqtt_client.client_cert, "/home/zwl/work/zshu_37/client.crt");
			sprintf(mqtt_client.client_key, "/home/zwl/work/zshu_37/client.key");
		#endif

	#endif
#endif
	mqtt_client.init_ok = 1;
#ifndef DF_1201_1
	mqtt_client.machine_fd = ty_open("/dev/machine", 0, 0, NULL);
	mqtt_client.get_net_fd = ty_open("/dev/get_net_state", 0, 0, NULL);
	mqtt_client.task_sub = _task_open("_mqtt_client.c", task_sub, &mqtt_client, 200, NULL);
#else
	mqtt_client.task_sub = _task_open("_mqtt_client.c", task_sub_1201, &mqtt_client, 200, NULL);
#endif
	mqtt_client.task_pub = _task_open("_mqtt_client.c", task_pub, &mqtt_client, 200, NULL);
    out("MQTT消息处理客户端初始化成功\n");
    return 0;
}

int _mqtt_invoice_init(void *function, void *err_deal_function, char *server, int port, char *topic, char *user_name, char *user_passwd, char *sub_id, char *pub_id)
{
#ifdef PRODUCE_PROGRAM
	memset(&mqtt_invoice, 0, sizeof(struct _mqtt_parms));
	return 0;
#endif
	memset(&mqtt_invoice, 0, sizeof(struct _mqtt_parms));
	mosquitto_share_init(&mqtt_invoice);
	sprintf(mqtt_invoice.name, "mqtt_fptbxt");
	mqtt_invoice.mqtt_type = DF_MQTT_FPTBXT;
	mqtt_invoice.hook = function;
	mqtt_invoice.err_hook = err_deal_function;
	memcpy(mqtt_invoice.server_address, server, strlen(server));
	mqtt_invoice.server_port = port;
	memcpy(mqtt_invoice.topic, topic, strlen(topic));
	memcpy(mqtt_invoice.user_name, user_name, strlen(user_name));
	memcpy(mqtt_invoice.user_passwd, user_passwd, strlen(user_passwd));
	memcpy(mqtt_invoice.user_id_sub, sub_id, strlen(sub_id));
	memcpy(mqtt_invoice.user_id_pub, pub_id, strlen(pub_id));
	mqtt_invoice.pub_data.cfgp.need_tls = 1;
	mqtt_invoice.sub_data.cfgp.need_tls = 1;
	sprintf(mqtt_invoice.ca_cert, "/etc/fptbxt_ca.crt");
	sprintf(mqtt_invoice.client_cert, "/etc/fptbxt_client.crt");
	sprintf(mqtt_invoice.client_key, "/etc/fptbxt_client.key");
	mqtt_invoice.init_ok = 1;
	mqtt_invoice.machine_fd = ty_open("/dev/machine", 0, 0, NULL);
	mqtt_invoice.get_net_fd = ty_open("/dev/get_net_state", 0, 0, NULL);
	mqtt_invoice.task_sub = _task_open("_mqtt_client.c", task_sub, &mqtt_invoice, 200, NULL);
	mqtt_invoice.task_pub = _task_open("_mqtt_client.c", task_pub, &mqtt_invoice, 200, NULL);
	//out("MQTT运维通道客户端初始化成功\n");
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
		//out("外网功能未就绪,暂不开启mqtt\n");
#ifdef MTK_OPENWRT
		mqtt_parms->err_hook(mqtt_parms, DF_MQTT_ERR_RED_OFF_GREEN_TWI);//用作灯的状态修改 999
#endif
		return;
	}
	out("初始化%s,开启sub链路\n", mqtt_parms->name);


	memset(mqtt_parms->server_address, 0, sizeof(mqtt_parms->server_address));
	memset(mqtt_parms->topic, 0, sizeof(mqtt_parms->topic));
	memset(mqtt_parms->user_name, 0, sizeof(mqtt_parms->user_name));
	memset(mqtt_parms->user_passwd, 0, sizeof(mqtt_parms->user_passwd));
	memset(mqtt_parms->user_id_sub, 0, sizeof(mqtt_parms->user_id_sub));
	memset(mqtt_parms->user_id_pub, 0, sizeof(mqtt_parms->user_id_pub));
	mqtt_parms->server_port = 0;

	result = function_get_mqtt_connect_data_md5_jude_update(mqtt_parms->server_address, &mqtt_parms->server_port, mqtt_parms->user_name, mqtt_parms->user_passwd, mqtt_parms->mqtt_type);
	if (result < 0)
	{
		out("与服务器交互失败,直接使用本地配置连接服务器,%s\n", mqtt_parms->name);
		result = ty_ctl(mqtt_parms->machine_fd, DF_MACHINE_INFOR_CM_GET_MQTT_USER, mqtt_parms->server_address, &mqtt_parms->server_port, mqtt_parms->user_name, mqtt_parms->user_passwd);
		if (result < 0)
		{
			out("获取本地配置失败\n");
			return;
		}
		if (mqtt_parms->mqtt_type==1)
		{
			sprintf(mqtt_parms->topic, "mb_fpkjxt_%s", mqtt_parms->user_name);
			sprintf(mqtt_parms->user_id_sub, "mb_fpkjxt_sub%s", mqtt_parms->user_name);
			sprintf(mqtt_parms->user_id_pub, "mb_fpkjxt_pub%s", mqtt_parms->user_name);
		}
		else
		{
			sprintf(mqtt_parms->topic, "mb_fptbxt_%s", mqtt_parms->user_name);
			sprintf(mqtt_parms->user_id_sub, "mb_fptbxt_sub%s", mqtt_parms->user_name);
			sprintf(mqtt_parms->user_id_pub, "mb_fptbxt_pub%s", mqtt_parms->user_name);
		}
		
	}
	else if (result == 0)
	{
		out("与服务器成功,服务器与当前配置一致无需更新\n");
		if (mqtt_parms->mqtt_type == 1)
		{
			sprintf(mqtt_parms->topic, "mb_fpkjxt_%s", mqtt_parms->user_name);
			sprintf(mqtt_parms->user_id_sub, "mb_fpkjxt_sub%s", mqtt_parms->user_name);
			sprintf(mqtt_parms->user_id_pub, "mb_fpkjxt_pub%s", mqtt_parms->user_name);
		}
		else
		{
			sprintf(mqtt_parms->topic, "mb_fptbxt_%s", mqtt_parms->user_name);
			sprintf(mqtt_parms->user_id_sub, "mb_fptbxt_sub%s", mqtt_parms->user_name);
			sprintf(mqtt_parms->user_id_pub, "mb_fptbxt_pub%s", mqtt_parms->user_name);
		}
	}
	else
	{
		out("与服务器成功,服务器与当前配置不一致需更新后再连接\n");
		result = function_get_mqtt_connect_data_info_update(mqtt_parms->server_address, &mqtt_parms->server_port, mqtt_parms->user_name, mqtt_parms->user_passwd, mqtt_parms->mqtt_type);
		if (result < 0)
		{
			sleep(10);
			return;
		}
		if (mqtt_parms->mqtt_type == 1)
		{
			sprintf(mqtt_parms->topic, "mb_fpkjxt_%s", mqtt_parms->user_name);
			sprintf(mqtt_parms->user_id_sub, "mb_fpkjxt_sub%s", mqtt_parms->user_name);
			sprintf(mqtt_parms->user_id_pub, "mb_fpkjxt_pub%s", mqtt_parms->user_name);
		}
		else
		{
			sprintf(mqtt_parms->topic, "mb_fptbxt_%s", mqtt_parms->user_name);
			sprintf(mqtt_parms->user_id_sub, "mb_fptbxt_sub%s", mqtt_parms->user_name);
			sprintf(mqtt_parms->user_id_pub, "mb_fptbxt_pub%s", mqtt_parms->user_name);
		}
	}

	
	//out("测试订阅任务是否会退出 task_sub\n");	
	//out("task_sub server address %s\n", mqtt_parms->server_address);
	//out("task_sub server port %d\n", mqtt_parms->server_port);
	//out("task_sub topics %s\n", mqtt_parms->topic);
	//out("task_sub user %s\n", mqtt_parms->user_name);
	//out("task_sub passwd %s\n", mqtt_parms->user_passwd);
	//out("task_sub usrid = %s\n", mqtt_parms->user_id_sub);
	

#ifdef MTK_OPENWRT
	mqtt_parms->err_hook(mqtt_parms, DF_MQTT_ERR_RED_OFF_GREEN_ON);//用作灯的状态修改 888
#endif
	result = mqtt_sub_message(mqtt_parms);
	out("退出 task_sub 消息订阅循环\n");

	sleep(10);
	mqtt_parms->err_hook(mqtt_parms, result);
	return;
}



static void task_sub_1201(void *arg, int timer)
{
	struct _mqtt_parms  *mqtt_parms;
	int result;
	mqtt_parms = arg;
	//char sub_id[100] = {0};
	//char time_s[20];


/*	struct _net_status net_status;
	memset(&net_status, 0, sizeof(struct _net_status));
	ty_ctl(mqtt_parms->get_net_fd, DF_GET_NET_STATE_CM_GET_NET_STATUS, &net_status);
	if (net_status.nettime_ok != 1)
	{
		//out("外网功能未就绪,暂不开启mqtt\n");
#ifdef MTK_OPENWRT
		mqtt_parms->err_hook(mqtt_parms, DF_MQTT_ERR_RED_OFF_GREEN_TWI);//用作灯的状态修改 999
#endif
		return;
	}
*/
	out("初始化%s,开启sub链路\n", mqtt_parms->name);


	// memset(mqtt_parms->server_address, 0, sizeof(mqtt_parms->server_address));
	// memset(mqtt_parms->topic, 0, sizeof(mqtt_parms->topic));
	// memset(mqtt_parms->user_name, 0, sizeof(mqtt_parms->user_name));
	// memset(mqtt_parms->user_passwd, 0, sizeof(mqtt_parms->user_passwd));
	// memset(mqtt_parms->user_id_sub, 0, sizeof(mqtt_parms->user_id_sub));
	// memset(mqtt_parms->user_id_pub, 0, sizeof(mqtt_parms->user_id_pub));
	// mqtt_parms->server_port = 0;

	result = function_get_mqtt_connect_data_md5_jude_update_1201(mqtt_parms->server_address, &mqtt_parms->server_port, mqtt_parms->user_name, mqtt_parms->user_passwd, mqtt_parms->mqtt_type);
	printf("			 function_get_mqtt_connect_data_md5_jude_update = [%d]			\r\n",result);
	if (result < 0)
	{	
		printf("			1qqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqq		\r\n");

		out("与服务器交互失败,直接使用本地配置连接服务器,%s\n", mqtt_parms->name);
		result = ty_ctl(mqtt_parms->machine_fd, DF_MACHINE_INFOR_CM_GET_MQTT_USER, mqtt_parms->server_address, &mqtt_parms->server_port, mqtt_parms->user_name, mqtt_parms->user_passwd);
		if (result < 0)
		{
			out("获取本地配置失败\n");
			return;
		}
		if (mqtt_parms->mqtt_type==1)
		{
			sprintf(mqtt_parms->topic, "mb_fpkjxt_%s", mqtt_parms->topic);
			sprintf(mqtt_parms->user_id_sub, "mb_fpkjxt_sub%s", mqtt_parms->user_name);
			sprintf(mqtt_parms->user_id_pub, "mb_fpkjxt_pub%s", mqtt_parms->user_name);
		}
		else
		{
			sprintf(mqtt_parms->topic, "mb_fptbxt_%s", mqtt_parms->topic);
			sprintf(mqtt_parms->user_id_sub, "mb_fptbxt_sub%s", mqtt_parms->user_name);
			sprintf(mqtt_parms->user_id_pub, "mb_fptbxt_pub%s", mqtt_parms->user_name);
		}
		
	}
	else if (result == 0)
	{
		printf("			1qqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqq222222		\r\n");
		out("与服务器成功,服务器与当前配置一致无需更新\n");
		if (mqtt_parms->mqtt_type == 1)
		{
			// sprintf(mqtt_parms->topic, "mb_fpkjxt_%s", mqtt_parms->topic);
			// sprintf(mqtt_parms->user_id_sub, "mb_fpkjxt_sub%s", mqtt_parms->user_name);
			// sprintf(mqtt_parms->user_id_pub, "mb_fpkjxt_pub%s", mqtt_parms->user_name);
			sprintf(mqtt_parms->topic, "mb_fptbxt_%s","000000001234");
			sprintf(mqtt_parms->user_id_sub, "mb_fptbxt_sub%s", "000000001234");
			sprintf(mqtt_parms->user_id_pub, "mb_fptbxt_pub%s", "000000001234");
		}
		else
		{
			// sprintf(mqtt_parms->topic, "mb_fptbxt_%s", mqtt_parms->topic);
			// sprintf(mqtt_parms->user_id_sub, "mb_fptbxt_sub%s", mqtt_parms->user_name);
			// sprintf(mqtt_parms->user_id_pub, "mb_fptbxt_pub%s", mqtt_parms->user_name);
			sprintf(mqtt_parms->topic, "mb_fptbxt_%s", "000000001234");
			sprintf(mqtt_parms->user_id_sub, "mb_fptbxt_sub%s", "000000001234");
			sprintf(mqtt_parms->user_id_pub, "mb_fptbxt_pub%s", "000000001234");
		}
	}
	else
	{
		printf("			1qqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqq33333333333		\r\n");
		// out("与服务器成功,服务器与当前配置不一致需更新后再连接\n");
		// result = function_get_mqtt_connect_data_info_update(mqtt_parms->server_address, &mqtt_parms->server_port, mqtt_parms->user_name, mqtt_parms->user_passwd, mqtt_parms->mqtt_type);
		// if (result < 0)
		// {
		// 	sleep(10);
		// 	return;
		// }
		if (mqtt_parms->mqtt_type == 1)
		{
			printf("			1qqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqq	444444444444	\r\n");
			sprintf(mqtt_parms->topic, "mb_fpkjxt_%s", "000000001234");
			sprintf(mqtt_parms->user_id_sub, "mb_fpkjxt_sub%s", "000000001234");
			sprintf(mqtt_parms->user_id_pub, "mb_fpkjxt_pub%s", "000000001234");

		}
		else
		{
			printf("			1qqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqq	555555555555555	\r\n");
			sprintf(mqtt_parms->topic, "mb_fptbxt_%s", "000000001234");
			sprintf(mqtt_parms->user_id_sub, "mb_fptbxt_sub%s", "000000001234");
			sprintf(mqtt_parms->user_id_pub, "mb_fptbxt_pub%s", "000000001234");

		}
	}

	
	out("测试订阅任务是否会退出 task_sub\n");	
	out("task_sub server address %s\n", mqtt_parms->server_address);
	out("task_sub server port %d\n", mqtt_parms->server_port);
	out("task_sub topics %s\n", mqtt_parms->topic);
	out("task_sub user %s\n", mqtt_parms->user_name);
	out("task_sub passwd %s\n", mqtt_parms->user_passwd);
	out("task_sub usrid = %s\n", mqtt_parms->user_id_sub);
	

#ifdef MTK_OPENWRT
	mqtt_parms->err_hook(mqtt_parms, DF_MQTT_ERR_RED_OFF_GREEN_ON);//用作灯的状态修改 888
#endif
	result = mqtt_sub_message(mqtt_parms);
	out("退出 task_sub 消息订阅循环\n");

	sleep(10);
	mqtt_parms->err_hook(mqtt_parms, result);
	return;
}

static void task_pub(void *arg, int timer)
{
	struct _mqtt_parms  *mqtt_parms;
	mqtt_parms = arg;
	//out("task_pub初始化pub，name = %s\n", mqtt_parms->name);
	mqtt_pub_message(mqtt_parms);
	return;
}



int _mqtt_client_pub(char *topic, char *message, int qos)
{
	int result;
	if (mqtt_client.init_ok != 1)
	{
		return -200;
	}
	result = mosquitt_pub_message(&mqtt_client, topic, message,qos);
	return result;
}


int _mqtt_invoice_pub(char *topic, char *message, int qos)
{
	int result;
	if (mqtt_invoice.init_ok != 1)
	{
		return -200;
	}
	result = mosquitt_pub_message(&mqtt_invoice, topic, message,qos);
	return result;
}




int _mqtt_client_disconnect()
{
	out("断开sub链路连接\n");
	mosquitto_disconnect(mqtt_client.sub_mosq);
	mqtt_client.sub_data.cfgp.sub_connect_ok = 0;
	//mosquitto_destroy(mqtt_client.sub_mosq);


	out("断开pub链路连接\n");
	mosquitto_disconnect(mqtt_client.mosq);
	mqtt_client.pub_data.cfgp.pub_connect_ok = 0;
	//mosquitto_destroy(mqtt_client.mosq);
	return 0;
}


int _mqtt_invoice_disconnect()
{
	out("断开sub链路连接\n");
	mosquitto_disconnect(mqtt_invoice.sub_mosq);
	mqtt_invoice.sub_data.cfgp.sub_connect_ok = 0;
	//mosquitto_destroy(mqtt_client.sub_mosq);


	out("断开pub链路连接\n");
	mosquitto_disconnect(mqtt_invoice.mosq);
	mqtt_invoice.pub_data.cfgp.pub_connect_ok = 0;
	//mosquitto_destroy(mqtt_client.mosq);
	return 0;
}
