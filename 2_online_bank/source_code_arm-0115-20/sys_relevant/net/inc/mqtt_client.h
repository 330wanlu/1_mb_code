#ifndef _MQTT_CLIENT_H
#define _MQTT_CLIENT_H

#include "opt.h"

#include "scheduled_task.h"
#include "lock.h"
#include "mqtt_share.h"
#include "get_net_state.h"
#include "opt.h"
#include "usb_port.h"

#define mqtt_client_out(s,arg...)       				//log_out("all.c",s,##arg) 

#define DF_ERR_SERVER_ADDRESS	"www.njmbxx.com"
//#define DF_ERR_SERVER_ADDRESS	"103.45.250.88"

#define DF_ERR_SERVER_PORT 20020
#define DF_ERR_SERVER_APPID "EE8B56879B7D5449933C1F8918C33FB6"
#define DF_ERR_SERVER_APPSECERT "74993F73731FAE499F0F2A98CC4A9E90"

int _mqtt_client_init(void *function, void *err_deal_function, char *server, int port, char *topic, char *user_name, char *user_passwd, char *sub_id, char *pub_id);
int _mqtt_devops_init(void *function, void *err_deal_function, char *server, int port, char *topic, char *user_name, char *user_passwd, char *sub_id, char *pub_id);
int _mqtt_client_pub(char *topic, char *message);
int _mqtt_devops_pub(char *topic, char *message);
int _mqtt_client_disconnect();

struct _mqtt_parms *my_get_mqtt_parm(void);
	
#endif
