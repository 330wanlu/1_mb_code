#ifndef _MQTT_CLIENT_DEFINE
    #define _MQTT_CLIENT_DEFINE
	#include <sys/types.h>
    #include <unistd.h>
    #include <strings.h>
    #include <sys/stat.h>
	#include "../linux_s/_task.h"
	#include "../linux_s/_lock.h"
	#include "mqtt_share.h"
	#include "../core/_get_net_state.h"
    #include <stdlib.h>
	#include "../_opt.h"
#ifndef RELEASE_SO
	#include "../manage/_log.h"
#endif
	#ifdef MTK_OPENWRT
	#include "_linux_ip_mtk.h"
	#include "../file/_file.h"
	#include "../core/_ty_usb.h"
	#else
	#include "_linux_ip.h"
	#endif
    /*=========================================立即数定义===============================================*/

#ifdef _mqtt_client_c
	#ifdef DEBUG
	#define out(s,arg...)       				log_out("all.c",s,##arg) 
	#else
	#define out(s,arg...)						log_out("all.c",s,##arg) 
	#endif	
#endif 
    /*=========================================结构体定义===============================================*/

#define DF_MQTT_FPKJXT  1
#define DF_MQTT_FPTBXT	2
	

    /*========================================函数定义=================================================*/
	extern		int _mqtt_client_init(void *function, void *err_deal_function, char *server, int port, char *topic, char *user_name, char *user_passwd, char *sub_id, char *pub_id);
	extern      int _mqtt_invoice_init(void *function, void *err_deal_function, char *server, int port, char *topic, char *user_name, char *user_passwd, char *sub_id, char *pub_id);
	extern		int _mqtt_client_pub(char *topic, char *message, int qos);
	extern      int _mqtt_invoice_pub(char *topic, char *message, int qos);
	extern		int _mqtt_client_disconnect();
	extern		int _mqtt_invoice_disconnect();
#ifdef _mqtt_client_c
	static void task_pub(void *arg, int timer);
	static void task_sub(void *arg, int timer);
	static void task_sub_1201(void *arg, int timer);
#endif
	
#endif