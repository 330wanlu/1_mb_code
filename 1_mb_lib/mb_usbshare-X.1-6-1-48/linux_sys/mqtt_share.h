/*
Copyright (c) 2014-2019 Roger Light <roger@atchoo.org>

All rights reserved. This program and the accompanying materials
are made available under the terms of the Eclipse Public License v1.0
and Eclipse Distribution License v1.0 which accompany this distribution.

The Eclipse Public License is available at
http://www.eclipse.org/legal/epl-v10.html
and the Eclipse Distribution License is available at
http://www.eclipse.org/org/documents/edl-v10.php.

Contributors:
Roger Light - initial implementation and documentation.
*/

#ifndef CLIENT_CONFIG_H
#define CLIENT_CONFIG_H


#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef WIN32
#include <unistd.h>
#else
#include <process.h>
#include <winsock2.h>
#define snprintf sprintf_s
#endif

#include "../include/mosquitto_new.h"
#include "../include/mqtt_protocol.h"
#include "../include/mosquitto_internal.h"
#include "../linux_s/_lock.h"

#ifdef WIN32
#  include <winsock2.h>
#else
#  include <sys/time.h>
#endif

/* pub_client.c modes */
#define MSGMODE_NONE 0
#define MSGMODE_CMD 1
#define MSGMODE_STDIN_LINE 2
#define MSGMODE_STDIN_FILE 3
#define MSGMODE_FILE 4
#define MSGMODE_NULL 5

#define STATUS_CONNECTING 0
#define STATUS_CONNACK_RECVD 1
#define STATUS_WAITING 2
#define STATUS_DISCONNECTING 3
#define STATUS_DISCONNECTED 4


#define CLIENT_PUB 1
#define CLIENT_SUB 2
#define CLIENT_RR 3
#define CLIENT_RESPONSE_TOPIC 4


#ifdef _mqtt_share_c
#ifdef DEBUG
#define out(s,arg...)       				log_out("all.c",s,##arg) 
#else
#define out(s,arg...)						log_out("all.c",s,##arg) 
#endif	
#endif 




//#define WITH_TLS
struct mosq_config {
	int need_tls;
	char *id;
	char *id_prefix;
	int protocol_version;
	int keepalive;
	char *host;
	int port;
	int qos;
	bool retain;
	int pub_mode; /* pub, rr */
	char *file_input; /* pub, rr */
	char *message; /* pub, rr */
	long msglen; /* pub, rr */
	char *topic; /* pub, rr */
	char *bind_address;
	int repeat_count; /* pub */
	struct timeval repeat_delay; /* pub */
#ifdef WITH_SRV
	bool use_srv;
#endif
	bool debug;
	bool quiet;
	unsigned int max_inflight;
	char *username;
	char *password;
	char *will_topic;
	char *will_payload;
	long will_payloadlen;
	int will_qos;
	bool will_retain;
#ifdef WITH_TLS
	char *cafile;
	char *capath;
	char *certfile;
	char *keyfile;
	char *ciphers;
	bool insecure;
	char *tls_alpn;
	char *tls_version;
	char *tls_engine;
	char *tls_engine_kpass_sha1;
	char *keyform;
#  ifdef FINAL_WITH_TLS_PSK
	char *psk;
	char *psk_identity;
#  endif
#endif
	bool clean_session;
	char **topics; /* sub */
	int topic_count; /* sub */
	bool exit_after_sub; /* sub */
	bool no_retain; /* sub */
	bool retained_only; /* sub */
	bool remove_retained; /* sub */
	char **filter_outs; /* sub */
	int filter_out_count; /* sub */
	char **unsub_topics; /* sub */
	int unsub_topic_count; /* sub */
	bool verbose; /* sub */
	bool eol; /* sub */
	int msg_count; /* sub */
	char *format; /* sub */
	int timeout; /* sub */
	int sub_opts; /* sub */
#ifdef WITH_SOCKS
	char *socks5_host;
	int socks5_port;
	char *socks5_username;
	char *socks5_password;
#endif
	mosquitto_property *connect_props;
	mosquitto_property *publish_props;
	mosquitto_property *subscribe_props;
	mosquitto_property *unsubscribe_props;
	mosquitto_property *disconnect_props;
	mosquitto_property *will_props;
	bool have_topic_alias; /* pub */
	char *response_topic; /* rr */
	int sub_connect_ok;
	int pub_connect_ok;
	char name[50];
};

struct pub_private_data
{
	bool first_publish;
	int last_mid_sent;
	bool disconnect_sent;
	int publish_count;
	bool ready_for_repeat;
	struct mosq_config cfgp;
	int status;
	int mid_sent;
};

struct sub_private_data
{
	struct mosq_config cfgp;
};

struct _mqtt_parms
{
	
	int task_sub;
	int task_pub;
	int machine_fd;
	int get_net_fd;
	int lock;
	int init_ok;
	char server_address[100];
	int server_port;
	char topic[100];
	void(*hook)(struct mosquitto *mosq, void *obj, const struct mosquitto_message *message);       //应用回调函数
	void(*err_hook)(struct _mqtt_parms  *stream, int errnum);       //应用回调函数
	char user_name[20];
	char user_passwd[50];
	char user_id_sub[100];
	char user_id_pub[100];
	char ca_cert[200];
	char client_cert[200];
	char client_key[200];
	
	struct mosquitto *sub_mosq;
	struct mosquitto *mosq;
	struct pub_private_data pub_data;
	struct sub_private_data sub_data;
	char name[50];
	int mqtt_type;
};

#ifdef _mqtt_share_c
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
#endif


int mosquitto_share_init(struct _mqtt_parms  *mqtt_client);
int mqtt_sub_message(struct _mqtt_parms *mqtt_client);
int mqtt_pub_message(struct _mqtt_parms  *mqtt_client);
int mosquitt_pub_message(struct _mqtt_parms  *mqtt_client, char *topic, char *message, int qos);
#endif
