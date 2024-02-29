#ifndef _M_SERVER_H
#define _M_SERVER_H

#include "opt.h"
//#include "ol_bank_include.h"
//#include "port_core.h"	
#include "mqtt_client.h"
#include "machine_infor.h"
#include "lock.h"

#define DF_MB_MQTT_PUB_NO_ANSWER                            100                         //MQTT发布无需应答
#define DF_MB_MQTT_PUB_NEED_ANSWER                          101                         //MQTT发布需要应答

#define m_server_out(s,arg...)       				//log_out("all.c",s,##arg) 

#define DEAFULT_CONNECT_TIME 60		//MQTT默认连间隔时间
#define DF_MESSAGE_LINE_NUM 1000

struct _message_queue_line
{
	int state;//当前内存状态
	char qqlsh[50];//请求流水号
	int answer_flag;//应答标志
	int answer_result;//0失败 1成功
	char *answer_data;
};

struct _m_server_fd
{
	int state;
	struct ty_file_mem	*mem;
	void *mem_dev;
	struct ty_file_fd	*mem_fd;
	struct _m_server  *dev;
};

struct _m_server
{
	int state;                                      //状态
	int lock;
	int machine_fd;
	char mqtt_server[200];
	int mqtt_port;
	char user_name[20];
	char user_passwd[50];
	char topic[200];
	int mqtt_reg_flag;
	int connect_time;
	int mqtt_state;		//0：未登录					1：已经登录
	struct _message_queue_line m_line[DF_MESSAGE_LINE_NUM];
	struct _m_server_fd fd[10];
};

struct _m_server_ctl_fu
{
	int cm;
	int(*ctl)(struct _m_server_fd   *id, va_list args);
};

int _m_server_add();
int message_callback_deal_answer(struct mqtt_parm	*parm, char *message);
int _m_pub_no_answer(char *topic,char *message);
int _m_pub_need_answer(char *qqlsh, char *topic, char *message, char *g_data);

#endif
