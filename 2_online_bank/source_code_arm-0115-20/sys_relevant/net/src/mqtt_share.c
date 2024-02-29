#include "../inc/mqtt_share.h"

static void client_config_cleanup(struct mosq_config *cfg, int pub_or_sub);
static int client_opts_set(struct mosquitto *mosq, struct mosq_config *cfg);
static int client_id_generate(struct mosq_config *cfg);
static int client_connect(struct mosquitto *mosq, struct mosq_config *cfg);
static int my_publish(struct mosquitto *mosq, int *mid, const char *topic, int payloadlen, void *payload, int qos, bool retain, struct mosq_config *cfg, bool ready_for_repeat, bool first_publish);
static int client_config_init_pub(struct mosq_config *cfg, char *serveraddr, int port, char *topic_s, char *s_buf, char *user, char *pwd, char *userid);
static void init_config(struct mosq_config *cfg);
static void my_subscribe_callback(struct mosquitto *mosq, void *obj, int mid, int qos_count, const int *granted_qos);
static void sub_connect_callback(struct mosquitto *mosq, void *obj, int result, int flags, const mosquitto_property *properties);
static void my_connect_callback(struct mosquitto *mosq, void *obj, int result, int flags, const mosquitto_property *properties);
static int client_config_init_sub(struct mosq_config *cfg, char *serveraddr, int port, char *topic_s, char *user, char *pwd, char *userid);

static struct timeval next_publish_tv;

static void set_repeat_time(struct mosq_config *cfg)
{
	gettimeofday(&next_publish_tv, NULL);
	next_publish_tv.tv_sec += cfg->repeat_delay.tv_sec;
	next_publish_tv.tv_usec += cfg->repeat_delay.tv_usec;
	next_publish_tv.tv_sec += next_publish_tv.tv_usec / 1e6;
	next_publish_tv.tv_usec = next_publish_tv.tv_usec % 1000000;
}

static void init_config(struct mosq_config *cfg)
{
	cfg->qos = 0;
	cfg->port = 1883;
	cfg->max_inflight = 100;
	cfg->keepalive = 10;//5.02版本之后由60秒改为10秒
	cfg->clean_session = true;
	cfg->debug = false;
	cfg->pub_mode = MSGMODE_CMD;
	cfg->eol = true;
	cfg->repeat_count = 1;
	cfg->repeat_delay.tv_sec = 0;
	cfg->repeat_delay.tv_usec = 0;
	//cfg->topic_count = 1;
	cfg->protocol_version = MQTT_PROTOCOL_V311;
#ifdef WITH_TLS
	if(cfg->need_tls == 1)
		cfg->tls_version = strdup("tlsv1.2");
#endif
	cfg->format = strdup("payload");
	//cfg->cafile = strdup("/etc/ca.crt");
	//cfg->certfile = strdup("/etc/client.crt");
	//cfg->keyfile = strdup("/etc/client.key");
}

static void client_config_cleanup(struct mosq_config *cfg, int pub_or_sub)
{
	int i;
	if (pub_or_sub == 0)//pub
	{
		if (cfg->host != NULL)
		{
			free(cfg->host);
			cfg->host = NULL;
		}		
		//if (cfg->topic != NULL)
		//{
		//	free(cfg->topic);
		//	cfg->topic = NULL;
		//}
		//if (cfg->message != NULL)
		//{
		//	free(cfg->message);
		//	cfg->message = NULL;
		//}
		if (cfg->username != NULL)
		{
			free(cfg->username);
			cfg->username = NULL;
		}
		if (cfg->password != NULL)
		{
			free(cfg->password);
			cfg->password = NULL;
		}
#ifdef WITH_TLS
		if(cfg->need_tls == 1)
		{
			if (cfg->tls_version != NULL)
			{
				free(cfg->tls_version);
				cfg->tls_version = NULL;
			}
			if (cfg->format != NULL)
			{
				free(cfg->format);
				cfg->format = NULL;
			}
			if (cfg->cafile != NULL)
			{
				free(cfg->cafile);
				cfg->cafile = NULL;
			}
			if (cfg->certfile != NULL)
			{
				free(cfg->certfile);
				cfg->certfile = NULL;
			}
			if (cfg->keyfile != NULL)
			{
				free(cfg->keyfile);
				cfg->keyfile = NULL;
			}
		}
#endif
		mosquitto_property_free_all(&cfg->connect_props);
		mosquitto_property_free_all(&cfg->publish_props);
		mosquitto_property_free_all(&cfg->subscribe_props);
		mosquitto_property_free_all(&cfg->unsubscribe_props);
		mosquitto_property_free_all(&cfg->disconnect_props);
		mosquitto_property_free_all(&cfg->will_props);
	}
	else if (pub_or_sub == 1)//sub
	{
		if (cfg->host!=NULL)
		{
			free(cfg->host);
			cfg->host = NULL;
		}
		if (cfg->username != NULL)
		{
			free(cfg->username);
			cfg->username = NULL;
		}
		if (cfg->password != NULL)
		{
			free(cfg->password);
			cfg->password = NULL;
		}
#ifdef WITH_TLS
		if(cfg->need_tls == 1)
		{
			if (cfg->tls_version != NULL)
			{
				free(cfg->tls_version);
				cfg->tls_version = NULL;
			}
			if (cfg->format != NULL)
			{
				free(cfg->format);
				cfg->format = NULL;
			}
			if (cfg->cafile != NULL)
			{
				free(cfg->cafile);
				cfg->cafile = NULL;
			}
			if (cfg->certfile != NULL)
			{
				free(cfg->certfile);
				cfg->certfile = NULL;
			}
			if (cfg->keyfile != NULL)
			{
				free(cfg->keyfile);
				cfg->keyfile = NULL;
			}
		}
#endif
		if (cfg->topics){
			for (i = 0; i<cfg->topic_count; i++){
				if (cfg->topics[i])
				{
					free(cfg->topics[i]);
					cfg->topics[i] = NULL;
				}
			}
			free(cfg->topics);
			cfg->topics = NULL;
		}
		cfg->topic_count = 0;

	}
}

static int client_config_init_pub(struct mosq_config *cfg, char *serveraddr, int port, char *topic_s, char *s_buf, char *user, char *pwd, char *userid)
{
	int rc;
	init_config(cfg);
	cfg->host = strdup(serveraddr);
	cfg->port = port;
	cfg->topic = strdup(topic_s);
	cfg->message = strdup(s_buf);
	cfg->msglen = strlen(s_buf);
	cfg->username = strdup(user);
	cfg->password = strdup(pwd);
	cfg->id = userid;
	cfg->retain = false;
	cfg->repeat_count = 0;
	rc = mosquitto_property_check_all(CMD_CONNECT, cfg->connect_props);
	if (rc){
		printf("Error in CONNECT properties: %s\n", mosquitto_strerror(rc));
		return 1;
	}
	rc = mosquitto_property_check_all(CMD_PUBLISH, cfg->publish_props);
	if (rc){
		printf("Error in PUBLISH properties: %s\n", mosquitto_strerror(rc));
		return 1;
	}
	rc = mosquitto_property_check_all(CMD_SUBSCRIBE, cfg->subscribe_props);
	if (rc){
		printf("Error in SUBSCRIBE properties: %s\n", mosquitto_strerror(rc));
		return 1;
	}
	rc = mosquitto_property_check_all(CMD_UNSUBSCRIBE, cfg->unsubscribe_props);
	if (rc){
		printf("Error in UNSUBSCRIBE properties: %s\n", mosquitto_strerror(rc));
		return 1;
	}
	rc = mosquitto_property_check_all(CMD_DISCONNECT, cfg->disconnect_props);
	if (rc){
		printf("Error in DISCONNECT properties: %s\n", mosquitto_strerror(rc));
		return 1;
	}
	rc = mosquitto_property_check_all(CMD_WILL, cfg->will_props);
	if (rc){
		printf("Error in Will properties: %s\n", mosquitto_strerror(rc));
		return 1;
	}

	return 0;
}

static int client_opts_set(struct mosquitto *mosq, struct mosq_config *cfg)
{
	mosquitto_int_option(mosq, MOSQ_OPT_PROTOCOL_VERSION, cfg->protocol_version);

	if (cfg->will_topic && mosquitto_will_set(mosq, cfg->will_topic,
		cfg->will_payloadlen, cfg->will_payload, cfg->will_qos,
		cfg->will_retain)){

		if (!cfg->quiet) fprintf(stderr, "Error: Problem setting will.\n");
		//mosquitto_lib_cleanup();
		return 1;
	}
	if (cfg->username && mosquitto_username_pw_set(mosq, cfg->username, cfg->password)){
		if (!cfg->quiet) fprintf(stderr, "Error: Problem setting username and password.\n");
		//mosquitto_lib_cleanup();
		return 1;
	}
#ifdef WITH_TLS
	if(cfg->need_tls == 1)
	{
		/*printf("cafile = %s\n", cfg->cafile);
		printf("capath = %s\n", cfg->capath);
		printf("certfile = %s\n", cfg->certfile);
		printf("keyfile = %s\n", cfg->keyfile);*/
		if ((cfg->cafile || cfg->capath)
			&& mosquitto_tls_set(mosq, cfg->cafile, cfg->capath, cfg->certfile, cfg->keyfile, NULL)){

			if (!cfg->quiet) fprintf(stderr, "Error: Problem setting TLS options.\n");
			//mosquitto_lib_cleanup();
			return 1;
		}
		if (cfg->insecure && mosquitto_tls_insecure_set(mosq, true)){
			if (!cfg->quiet) fprintf(stderr, "Error: Problem setting TLS insecure option.\n");
			//mosquitto_lib_cleanup();
			return 1;
		}
#  ifdef WITH_TLS_PSK
		if (cfg->psk && mosquitto_tls_psk_set(mosq, cfg->psk, cfg->psk_identity, NULL)){
			if (!cfg->quiet) fprintf(stderr, "Error: Problem setting TLS-PSK options.\n");
			//mosquitto_lib_cleanup();
			return 1;
		}
#  endif
		if ((cfg->tls_version || cfg->ciphers) && mosquitto_tls_opts_set(mosq, 1, cfg->tls_version, cfg->ciphers)){
			if (!cfg->quiet) fprintf(stderr, "Error: Problem setting TLS11 options.\n");
			//mosquitto_lib_cleanup();
			return 1;
		}
	}
#endif
	mosquitto_max_inflight_messages_set(mosq, cfg->max_inflight);
#ifdef WITH_SOCKS
	int rc;
	if (cfg->socks5_host){
		rc = mosquitto_socks5_set(mosq, cfg->socks5_host, cfg->socks5_port, cfg->socks5_username, cfg->socks5_password);
		if (rc){
			//mosquitto_lib_cleanup();
			return rc;
		}
	}
#endif
	mosquitto_opts_set(mosq, MOSQ_OPT_PROTOCOL_VERSION, &(cfg->protocol_version));
	return MOSQ_ERR_SUCCESS;
}

static int client_id_generate(struct mosq_config *cfg)
{
	if (cfg->id_prefix){
		cfg->id = malloc(strlen(cfg->id_prefix) + 10);
		if (!cfg->id){
			//err_printf(cfg, "Error: Out of memory.\n");
			//mosquitto_lib_cleanup();
			return 1;
		}
		snprintf(cfg->id, strlen(cfg->id_prefix) + 10, "%s%d", cfg->id_prefix, getpid());
	}
	return MOSQ_ERR_SUCCESS;
}

static int client_connect(struct mosquitto *mosq, struct mosq_config *cfg)
{
	char err[1024];
	int rc;

#ifdef WITH_SRV
	if (cfg->use_srv){
		rc = mosquitto_connect_srv(mosq, cfg->host, cfg->keepalive, cfg->bind_address);
	}
	else{
		rc = mosquitto_connect_bind(mosq, cfg->host, cfg->port, cfg->keepalive, cfg->bind_address);
	}
#else
	//printf("host is:%s ---port is: %d -- bind_address:%s\n", cfg->host, cfg->port, cfg->bind_address);
	rc = mosquitto_connect_bind(mosq, cfg->host, cfg->port, cfg->keepalive, cfg->bind_address);
#endif
	if (rc>0){
		//printf("connect err!!!!!!!!!!!\n");
		if (!cfg->quiet){
			if (rc == MOSQ_ERR_ERRNO){
#ifndef WIN32
				strerror_r(errno, err, 1024);
#else
				FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, errno, 0, (LPTSTR)&err, 1024, NULL);
#endif
				fprintf(stderr, "Error: %s\n", err);
			}
			else{
				fprintf(stderr, "Unable to connect (%s).\n", mosquitto_strerror(rc));
				//printf("Unable to connect (%s).!!!!!!!!!!!\n", mosquitto_strerror(rc));
			}
		}
		//printf("connect err2!!!!!!!!!!!\n");
		//mosquitto_lib_cleanup();
		return rc;
	}
	return MOSQ_ERR_SUCCESS;
}



static void my_connect_callback(struct mosquitto *mosq, void *obj, int result, int flags, const mosquitto_property *properties)
{
	int rc = MOSQ_ERR_SUCCESS;
	struct mosq_config *cfg;
	//UNUSED(obj);
	//UNUSED(flags);
	//UNUSED(properties);
	struct pub_private_data *pub_data;
	pub_data = (struct pub_private_data *)obj;
	cfg = (struct mosq_config *)&(pub_data->cfgp);
	printf("---------------result:%d !!!!!!!!!\n", result);
	if (!result)
	{
		switch (cfg->pub_mode){
		case MSGMODE_CMD:
		case MSGMODE_FILE:
		case MSGMODE_STDIN_FILE:
			mqtt_share_out("Pub connect 成功\n");
			pub_data->cfgp.pub_connect_ok = 1;
			pub_data->status = STATUS_CONNECTING;
			rc = my_publish(mosq, &(pub_data->mid_sent), cfg->topic, cfg->msglen, cfg->message, cfg->qos, cfg->retain, cfg, pub_data->ready_for_repeat, pub_data->first_publish);
			break;
		case MSGMODE_NULL:
			rc = my_publish(mosq, &(pub_data->mid_sent), cfg->topic, 0, NULL, cfg->qos, cfg->retain, cfg, pub_data->ready_for_repeat, pub_data->first_publish);
			break;
		case MSGMODE_STDIN_LINE:
			pub_data->status = STATUS_CONNACK_RECVD;
			break;
		}
		if (rc){
			switch (rc){
			case MOSQ_ERR_INVAL:
				printf("Error: Invalid input. Does your topic contain '+' or '#'?\n");
				break;
			case MOSQ_ERR_NOMEM:
				printf("Error: Out of memory when trying to publish message.\n");
				break;
			case MOSQ_ERR_NO_CONN:
				printf( "Error: Client not connected when trying to publish.\n");
				break;
			case MOSQ_ERR_PROTOCOL:
				printf("Error: Protocol error when communicating with broker.\n");
				break;
			case MOSQ_ERR_PAYLOAD_SIZE:
				printf( "Error: Message payload is too large.\n");
				break;
			case MOSQ_ERR_QOS_NOT_SUPPORTED:
				printf("Error: Message QoS not supported on broker, try a lower QoS.\n");
				break;
			}
			mosquitto_disconnect_v5(mosq, 0, cfg->disconnect_props);
		}
		//mosquitto_disconnect_v5(mosq, 0, cfg->disconnect_props);
	}
	else
	{
		if (result)
		{
			if (cfg->protocol_version == MQTT_PROTOCOL_V5)
			{
				if (result == MQTT_RC_UNSUPPORTED_PROTOCOL_VERSION)
				{
				//	err_printf(&cfg, "Connection error: %s. Try connecting to an MQTT v5 broker, or use MQTT v3.x mode.\n", mosquitto_reason_string(result));
				}
				else
				{
				//	err_printf(&cfg, "Connection error: %s\n", mosquitto_reason_string(result));
				}
			}
			else
			{
			//	err_printf(&cfg, "Connection error: %s\n", mosquitto_connack_string(result));
			}
			mqtt_share_out("链路断开，关闭链路\n");
			pub_data->cfgp.pub_connect_ok = 0;
			mosquitto_disconnect_v5(mosq, 0, cfg->disconnect_props);
		}
	}
}

static void pub_disconnect_callback(struct mosquitto *mosq, void *obj, int rc, const mosquitto_property *properties)
{
	//UNUSED(mosq);
	//UNUSED(obj);
	//UNUSED(rc);
	//	UNUSED(properties);
	struct pub_private_data *pub_data;
	pub_data = (struct pub_private_data *)obj;
	printf("enter pub_disconnect_callback,name %s,rc = %d\n", pub_data->cfgp.name,rc);
	if (rc == 0 || rc == 7){
		printf("in pub_disconnect_callback!!!\n\n\n");
		pub_data->status = STATUS_DISCONNECTED;
	}
	// pub_data->status = STATUS_DISCONNECTED;
	
}

static void pub_publish_callback(struct mosquitto *mosq, void *obj, int mid, int reason_code, const mosquitto_property *properties)
{
//	UNUSED(obj);
//	UNUSED(properties);
	struct mosq_config *cfg;

	struct pub_private_data *pub_data;
	pub_data = (struct pub_private_data *)obj;

	cfg = (struct mosq_config *)&(pub_data->cfgp);
	pub_data->last_mid_sent = mid;
	if (reason_code > 127){
		//err_printf(&cfg, "Warning: Publish %d failed: %s.\n", mid, mosquitto_reason_string(reason_code));
		printf("Warning: Publish %d failed: %s.\n", mid, mosquitto_reason_string(reason_code));
	}
	pub_data->publish_count++;

	if (pub_data->publish_count < cfg->repeat_count){
		pub_data->ready_for_repeat = true;
		set_repeat_time(cfg);
	}
	else if (pub_data->disconnect_sent == false){
		//mosquitto_disconnect_v5(mosq, 0, cfg->disconnect_props);
		//pub_data->disconnect_sent = true;
	}
}
static int my_publish(struct mosquitto *mosq, int *mid, const char *topic, int payloadlen, void *payload, int qos, bool retain, struct mosq_config *cfg, bool ready_for_repeat, bool first_publish)
{
	ready_for_repeat = false;
	if (cfg->protocol_version == MQTT_PROTOCOL_V5 && cfg->have_topic_alias && first_publish == false){
		return mosquitto_publish_v5(mosq, mid, NULL, payloadlen, payload, qos, retain, cfg->publish_props);
	}
	else{
		first_publish = false;
		return mosquitto_publish_v5(mosq, mid, topic, payloadlen, payload, qos, retain, cfg->publish_props);
	}
}



int mqtt_pub_message(struct _mqtt_parms  *mqtt_client)
{
	
	int rc;
	if (mqtt_client->sub_data.cfgp.sub_connect_ok != 1)
	{
		return -100;
	}
	struct mosq_config *cfg;
	//mqtt_share_out("初始化pub，name = %s\n", mqtt_client->name);
	int need_tls = mqtt_client->pub_data.cfgp.need_tls;
	memset(&mqtt_client->pub_data, 0, sizeof(struct pub_private_data));
	strcpy(mqtt_client->pub_data.cfgp.name ,mqtt_client->name);
	mqtt_client->pub_data.cfgp.need_tls = need_tls;
	mqtt_client->pub_data.first_publish = true;
	mqtt_client->pub_data.last_mid_sent = -1;
	mqtt_client->pub_data.disconnect_sent = false;
	mqtt_client->pub_data.publish_count = 0;
	mqtt_client->pub_data.ready_for_repeat = false;
	//client_config_init_pub(&(mqtt_client->pub_data.cfgp), mqtt_client->server_address, mqtt_client->server_port, "mb_fpkjxt_kpfw123456789012345678901234567890123456", "123456", mqtt_client->user_name, mqtt_client->user_passwd, mqtt_client->user_id_pub);
	client_config_init_pub(&(mqtt_client->pub_data.cfgp), mqtt_client->server_address, mqtt_client->server_port, "mb_fpkjxt_kpfw123456789012345678901234567890123456", "123456", "liuzhide", "123456", mqtt_client->user_id_pub);
#ifdef WITH_TLS
	if(mqtt_client->pub_data.cfgp.need_tls ==1)
	{
		mqtt_client->pub_data.cfgp.cafile = strdup(mqtt_client->ca_cert);
		mqtt_client->pub_data.cfgp.certfile = strdup(mqtt_client->client_cert);
		mqtt_client->pub_data.cfgp.keyfile = strdup(mqtt_client->client_key);
	}
#endif
	mqtt_client->pub_data.status = STATUS_CONNECTING;
	mqtt_client->mosq = mosquitto_new(mqtt_client->pub_data.cfgp.id, true, &mqtt_client->pub_data);
	if (!mqtt_client->mosq){
		switch (errno){
		case ENOMEM:
			printf("Error: Out of memory.\n");
			break;
		case EINVAL:
			printf("Error: Invalid id.\n");
			break;
		}
		client_config_cleanup(&(mqtt_client->pub_data.cfgp), 0);
		return -1;
	}
	cfg = (struct mosq_config *)&(mqtt_client->pub_data.cfgp);
	mosquitto_connect_v5_callback_set(mqtt_client->mosq, my_connect_callback);
	mosquitto_disconnect_v5_callback_set(mqtt_client->mosq, pub_disconnect_callback);
	mosquitto_publish_v5_callback_set(mqtt_client->mosq, pub_publish_callback);
	if (client_opts_set(mqtt_client->mosq, &(mqtt_client->pub_data.cfgp)))
	{
		printf("pub client_opts_set err!\n");
		client_config_cleanup(&(mqtt_client->pub_data.cfgp), 0);
		return -2;
	}

	rc = client_connect(mqtt_client->mosq, &(mqtt_client->pub_data.cfgp));
	if (rc){
		printf("pub connetc err!\n");
		client_config_cleanup(&(mqtt_client->pub_data.cfgp), 0);
		return -3;
	}
	sleep(1);
	mqtt_client->pub_data.cfgp.pub_connect_ok = 1;
	mqtt_share_out("链路开启成功,即将开启loop\n");
	rc = mosquitto_loop_forever(mqtt_client->mosq, -1, 1);
	mqtt_client->pub_data.cfgp.pub_connect_ok = 0;
	mqtt_share_out("断开链路\n");
	mosquitto_disconnect_v5(mqtt_client->mosq, 0, cfg->disconnect_props);

	mqtt_share_out("清除mosq\n");
	mosquitto_destroy(mqtt_client->mosq);

	mqtt_share_out("清空cfgp\n");
	client_config_cleanup(&(mqtt_client->pub_data.cfgp), 0);

	mqtt_share_out("退出loop\n");
	return -10;
}

//long long mqtt_w_len = 0;

int mosquitt_pub_message(struct _mqtt_parms  *mqtt_client, char *topic, char *message)
{
	int time=0;
	for (time = 0;time<60*1000;time++)
	{
		if (mqtt_client->pub_data.cfgp.pub_connect_ok != 1)
		{
			usleep(1000);
			continue;
		}
		break;
	}
	if (mqtt_client->pub_data.cfgp.pub_connect_ok != 1)
		return -1;
	_lock_set(mqtt_client->lock);
	mqtt_client->pub_data.cfgp.topic = topic;
	mqtt_client->pub_data.cfgp.message = message;
	mqtt_client->pub_data.cfgp.msglen = strlen(message);
	//mqtt_share_out("MQTT PUB [%s:%s]\n", topic, message);
	mosquitto_publish(mqtt_client->mosq, NULL, topic, strlen(message), message, 0, 0);
	//mqtt_w_len += (strlen(message) + strlen(topic));
	//mqtt_share_out("MQTT发送字节数:%lld\n", mqtt_w_len);
	_lock_un(mqtt_client->lock);
	return 0;

}


/******************************************sub***************************************/









static void sub_connect_callback(struct mosquitto *mosq, void *obj, int result, int flags, const mosquitto_property *properties)
{
	int i;

	//UNUSED(obj);
//	UNUSED(flags);
	//UNUSED(properties);
	struct mosq_config *cfg;

	//assert(obj);
	cfg = (struct mosq_config *)obj;
	if (!result){
		mosquitto_subscribe_multiple(mosq, NULL, cfg->topic_count, cfg->topics, cfg->qos, cfg->sub_opts, cfg->subscribe_props);

		for (i = 0; i<cfg->unsub_topic_count; i++){
			mosquitto_unsubscribe_v5(mosq, NULL, cfg->unsub_topics[i], cfg->unsubscribe_props);
		}
	}
	else{
		if (result){
			if (cfg->protocol_version == MQTT_PROTOCOL_V5){
				if (result == MQTT_RC_UNSUPPORTED_PROTOCOL_VERSION){
					//err_printf(&cfg, "Connection error: %s. Try connecting to an MQTT v5 broker, or use MQTT v3.x mode.\n", mosquitto_reason_string(result));
				}
				else{
					//err_printf(&cfg, "Connection error: %s\n", mosquitto_reason_string(result));
				}
			}
			else{
			//	err_printf(&cfg, "Connection error: %s\n", mosquitto_connack_string(result));
			}
		}
		mosquitto_disconnect_v5(mosq, 0, cfg->disconnect_props);
	}
}
static void my_subscribe_callback(struct mosquitto *mosq, void *obj, int mid, int qos_count, const int *granted_qos)
{
	int i;

	struct mosq_config *cfg;

	//assert(obj);
	cfg = (struct mosq_config *)obj;

	
	if (!(cfg->quiet))
	{
		printf("Subscribed (mid: %d): %d\n", mid, granted_qos[0]);
		printf("MQTT subscribe succcess!\n");
		cfg->sub_connect_ok = 1;
	}
	for (i = 1; i<qos_count; i++){
		if (!(cfg->quiet))
			//printf(", %d", granted_qos[i]);
			;
	}
	if (!(cfg->quiet))
		;//printf("\n");

	if (cfg->exit_after_sub)
	{
		mosquitto_disconnect_v5(mosq, 0, cfg->disconnect_props);
	}

	
}





static int client_config_init_sub(struct mosq_config *cfg, char *serveraddr, int port, char *topic_s, char *user, char *pwd, char *userid)
{
	init_config(cfg);
	cfg->host = strdup(serveraddr);
	cfg->port = port;
	cfg->username = strdup(user);
	cfg->password = strdup(pwd);
	cfg->id = userid;
	//cfg->retain = true;
	cfg->pub_mode = 0;

	cfg->topic_count++;
	cfg->topics = realloc(cfg->topics, cfg->topic_count*sizeof(char *));
	cfg->topics[cfg->topic_count - 1] = strdup(topic_s);
	return 0;
}


int mqtt_sub_message(struct _mqtt_parms *mqtt_client)
{

	int rc;
	//static struct mosq_config cfgs;
	
	char sub_id_random[200] = {0};
	//mosquitto_lib_init();
	printf("mqtt_sub_message \n");
	if ((strlen(mqtt_client->server_address) == 0) ||
		(strlen(mqtt_client->topic) == 0) ||
		(strlen(mqtt_client->user_name) == 0) ||
		(strlen(mqtt_client->user_passwd) == 0)
		)
	{
		printf("exit mqtt_sub_message \n");
		return -100;
	}

	srand((unsigned)time(NULL));
	sprintf(sub_id_random, "%s_%d", mqtt_client->user_id_sub, rand() % 1000 + 1);


	//client_config_init_sub(&mqtt_client->sub_data.cfgp, mqtt_client->server_address, mqtt_client->server_port, mqtt_client->topic, mqtt_client->user_name, mqtt_client->user_passwd, sub_id_random);
	printf("~~~~~~~~~~~~~~~mqtt_client->server_address=%s,mqtt_client->server_port=%d\n",mqtt_client->server_address,mqtt_client->server_port);
	client_config_init_sub(&mqtt_client->sub_data.cfgp, mqtt_client->server_address, mqtt_client->server_port, mqtt_client->topic, "liuzhide", "123456", sub_id_random);
#ifdef WITH_TLS
	if(mqtt_client->sub_data.cfgp.need_tls ==1)
	{
		mqtt_client->sub_data.cfgp.cafile = strdup(mqtt_client->ca_cert);
		mqtt_client->sub_data.cfgp.certfile = strdup(mqtt_client->client_cert);
		mqtt_client->sub_data.cfgp.keyfile = strdup(mqtt_client->client_key);
	}
#endif
	if (client_id_generate(&mqtt_client->sub_data.cfgp))
	{
		return 1;
	}

	mqtt_client->sub_mosq = mosquitto_new(mqtt_client->sub_data.cfgp.id, mqtt_client->sub_data.cfgp.clean_session, &mqtt_client->sub_data.cfgp);
	if (!mqtt_client->sub_mosq){
		switch (errno){
		case ENOMEM:
			if (!mqtt_client->sub_data.cfgp.quiet) fprintf(stderr, "Error: Out of memory.\n");
			break;
		case EINVAL:
			if (!mqtt_client->sub_data.cfgp.quiet) fprintf(stderr, "Error: Invalid id and/or clean_session.\n");
			break;
		}
		//mosquitto_lib_cleanup();
		{
			return 1;
		}
	}
	//if (1) //测试遗嘱消息
	//{
	//	mqtt_client->sub_data.cfgp.will_topic = malloc(100);
	//	mqtt_client->sub_data.cfgp.will_payload = malloc(100);
	//	memset(mqtt_client->sub_data.cfgp.will_topic, 0, 100);
	//	memset(mqtt_client->sub_data.cfgp.will_payload, 0, 100);
	//	strcpy(mqtt_client->sub_data.cfgp.will_topic, "mb_equ_ter_will");
	//	mqtt_client->sub_data.cfgp.will_payloadlen = 12;
	//	strcpy(mqtt_client->sub_data.cfgp.will_payload, mqtt_client->sub_data.cfgp.username);
	//	mqtt_client->sub_data.cfgp.will_qos = 1;
	//	mqtt_client->sub_data.cfgp.will_retain = 0;
	//}


	if (client_opts_set(mqtt_client->sub_mosq, &mqtt_client->sub_data.cfgp))
	{
		return 1;
	}
	mosquitto_subscribe_callback_set(mqtt_client->sub_mosq, my_subscribe_callback);
	mosquitto_connect_v5_callback_set(mqtt_client->sub_mosq, sub_connect_callback);
	mosquitto_message_v5_callback_set(mqtt_client->sub_mosq, (void *)mqtt_client->hook);

	rc = client_connect(mqtt_client->sub_mosq, &mqtt_client->sub_data.cfgp);
	if (rc){
		printf("goto clean!!!!\n");
		goto cleanup;
	}

	rc = mosquitto_loop_forever(mqtt_client->sub_mosq, -1, 1);
	printf("exit mosquitto_loop_forever \n\n\n\n\n\n");
	mqtt_client->sub_data.cfgp.sub_connect_ok = 0;
	mosquitto_destroy(mqtt_client->sub_mosq);
	//mosquitto_lib_cleanup();

	if (mqtt_client->sub_data.cfgp.msg_count>0 && rc == MOSQ_ERR_NO_CONN){
		rc = 0;
	}
	client_config_cleanup(&mqtt_client->sub_data.cfgp, 1);
	if (rc){
		//err_printf(&cfg, "Error: %s\n", mosquitto_strerror(rc));
	}
	return -1;
cleanup:
	//mosquitto_lib_cleanup();
	client_config_cleanup(&mqtt_client->sub_data.cfgp, 1);
	return 1;
}

int mosquitto_share_init(struct _mqtt_parms  *mqtt_client)
{
	//int result;
	//printf("mosquitto_share_init\n\n\n\n\n\n\n\n\n\n\n\n\n\n");
	//pthread_mutex_init(&mutex, NULL); //初始化互斥锁 
	mqtt_client->lock = _lock_open("mqtt_share.c", NULL);

	
	return 0;
}

