#ifndef _MODULE_H
#define _MODULE_H

#include "opt.h"

#include "bluetooth.h"
#include "lcd_state.h"
#include "seria.h"
#include "switch_dev.h"
#include "usart.h"
#include "tran_data.h"
#include "ty_usb.h"
#include "machine_infor.h"
#include "deploy.h"
#include "get_net_state.h"
#include "register.h"
#include "get_net_time.h"
#include "m_server.h"
#include "linux_s.h"
#include "usb_port.h"
#include "usb_file.h"
#include "http_reg.h"
#include "linux_ip.h"
#include "udp_sev.h"
#include "tax_interface.h"
#include "update.h"
#include "https_client.h"
#include "ip_check.h"
#include "test.h"

#define module_out(s,arg...)       		//log_out("all.c",s,##arg) 

#define DF_UARTSET_MAIN_TASK_TIMER        800            	//主线程轮询时间
// #define DF_UDP_SEV_MAIN_TASK_TIMER        300            	//主线程轮询时间
#define DF_PC_SERVER_ORDER_HEAD_LEN		  7              	//STM32命令头长读
#define DF_TY_PC_SERVER_OVER_TIMER        1000           	//STM32通讯超时时间


#define DF_TY_HARD_EDIT                                         "YM1.00"

#define DF_TY_1502_COM_EDIT                                     0X0110
/*-----------------------------上行命令--------------------------------------*/
#define DF_TY_1502_SEV_ORDER_SET_USB_START                      0X0101
#define DF_TY_1502_SEV_ORDER_SET_IP                             0X0102
#define DF_TY_1502_SEV_ORDER_SET_ID                             0X0103
#define DF_TY_1502_SEV_ORDER_SET_TIMER                          0X0104
#define DF_TY_1502_SEV_ORDER_SET_PAR                            0X0105
#define DF_TY_1502_SEV_ORDER_SET_TRYOUT_TIME					0X0106
#define	DF_TY_1502_SEV_ORDER_SET_UPDATE_ADDR					0X0107 

#define DF_TY_1502_SEV_ORDER_SET_VPN_PAR						0X0109
#define DF_TY_1502_SEV_ORDER_SET_CLOSE_AUTO_UPDATE				0X0110
#define DF_TY_1502_SEV_ORDER_SEE_SEV_PRO                        0X0201
#define DF_TY_1502_SEV_ORDER_SEE_SEV_STATE                      0X0202
#define DF_TY_1502_SEV_ORDER_SEE_USB_PORT_PRO                   0X0203
#define DF_TY_1502_SEV_ORDER_SEE_PAR                            0X0204
#define DF_TY_1502_SEV_ORDER_SEE_USB_PORT_INFOR                 0X0205
#define DF_TY_1502_SEV_ORDER_SEE_LOG                            0X0206
#define DF_TY_1502_SEV_ORDER_SEE_NET_PAR                        0X0207
#define DF_TY_1502_SEV_ORDER_SEE_CHECK							0X0208
#define	DF_TY_1502_SEV_ORDER_SEE_UPDATE_ADDR					0X0209
#define DF_TY_1502_SEV_ORDER_SEE_ALL_PORT_STATE					0X0210

#define	DF_TY_1502_SEV_ORDER_SEE_TER_SOFT_VERSION				0X0212
#define DF_TY_1502_SEV_ORDER_SEE_ALL_PORT_CA_NAME				0X0213
#define	DF_TY_1502_SEV_ORDER_SEE_ALL_PORT_TYPE					0x0214
#define	DF_TY_1502_SEV_ORDER_SEE_VPN_PAR						0x0215
#define DF_TY_1502_SEV_ORDER_SEE_CLOSE_AUTO_UPDATE				0X0216
#define DF_TY_1502_SEV_ORDER_SEE_MONTH_FP_NUM					0X0217
#define DF_TY_1502_SEV_ORDER_SEE_FP_DETAILS						0X0218	
#define	DF_TY_1502_SEV_ORDER_SEE_FP_TYPES						0X0219
#define	DF_TY_1502_SEV_ORDER_SEE_MONITOR_DATA					0X0220
#define	DF_TY_1502_SEV_ORDER_SEE_INVOICE_NUM					0X0221
#define	DF_TY_1502_SEV_ORDER_SEE_FP_DETAILS_T					0X0222
#define	DF_TY_1502_SEV_ORDER_SEE_FP_DETAILS_S					0X0223
#define DF_TY_1502_SEV_ORDER_SEE_FP_AREA_CODE					0X0224
#define DF_TY_1502_SEV_ORDER_SEE_TAX_INFOS						0X0225
#define DF_TY_1502_SEV_ORDER_SEE_VERSION_DESCRIBE				0X0226
#define DF_TY_1502_SEV_ORDER_OPER_OPEN_USB                      0X0301
#define DF_TY_1502_SEV_ORDER_OPER_CLOSE_USB                     0X0302
#define DF_TY_1502_SEV_ORDER_OPER_ID_OPEN_USB                   0X0303
#define DF_TY_1502_SEV_ORDER_OPER_ID_CLOSE_USB                  0X0304
#define DF_TY_1502_SEV_ORDER_OPER_CLOSE_NOW                     0X0305
#define DF_TY_1502_SEV_ORDER_OPER_POWER_OFF                     0X0306
#define DF_TY_1502_SEV_ORDER_OPER_HEART                         0X0307
#define DF_TY_1502_SEV_ORDER_OPER_RESET                         0X0308
#define DF_TY_1502_SEV_ORDER_OPER_REDIT                         0X0309
#define DF_TY_1502_SEV_ORDER_OPER_OPEN_NOACTIVE                 0X0310         
#define DF_TY_1502_SEV_ORDER_SEE_OPEN_PORT                      0X0311  
#define DF_TY_1502_SEV_ORDER_OPER_UPDATA						0X0312
#define DF_TY_1502_SEV_ORDER_OPER_DETTACH_PORT					0X0313
#define	DF_MB_ORDER_SEE_USB_SHARE_DEV							0X0401
#define	DF_MB_ORDER_OPER_RESTART								0x0402

/*-----------------------------下行命令--------------------------------------*/
#define DF_TY_1502_ANSWER_ORDER_SET_USB_START                   0X1101
#define DF_TY_1502_ANSWER_ORDER_SET_IP                          0X1102
#define DF_TY_1502_ANSWER_ORDER_SET_ID                          0X1103
#define DF_TY_1502_ANSWER_ORDER_SET_TIMER                       0X1104
#define DF_TY_1502_ANSWER_ORDER_SET_PAR                         0X1105
#define DF_TY_1502_ANSWER_ORDER_SET_TRYOUT_TIME					0X1106
#define	DF_TY_1502_ANSWER_ORDER_SET_UPDATE_ADDR					0X1107	

#define DF_TY_1502_ANSWER_ORDER_SET_VPN_PAR						0X1109
#define DF_TY_1502_ANSWER_ORDER_SET_CLOSE_AUTO_UPDATE			0X1110
#define DF_TY_1502_ANSWER_ORDER_SEE_SEV_PRO                     0X1201
#define DF_TY_1502_ANSWER_ORDER_SEE_SEV_STATE                   0X1202
#define DF_TY_1502_ANSWER_ORDER_SEE_USB_PORT_PRO                0X1203
#define DF_TY_1502_ANSWER_ORDER_SEE_PAR                         0X1204
#define DF_TY_1502_ANSWER_ORDER_SEE_USB_PORT_INFOR              0X1205
#define DF_TY_1502_ANSWER_ORDER_SEE_LOG                         0X1206
#define DF_TY_1502_ANSWER_ORDER_SEE_NET_PAR                     0X1207
#define DF_TY_1502_ANSWER_ORDER_SEE_CHECK						0X1208
#define	DF_TY_1502_ANSWER_ORDER_SEE_UPDATE_ADDR					0X1209
#define DF_TY_1502_ANSWER_ORDER_SEE_ALL_PORT_STATE				0X1210

#define	DF_TY_1502_ANSWER_ORDER_SEE_TER_SOFT_VERSION			0X1212
#define DF_TY_1502_ANSWER_ORDER_SEE_ALL_PORT_CA_NAME			0X1213
#define	DF_TY_1502_ANSWER_ORDER_SEE_ALL_PORT_TYPE				0X1214
#define DF_TY_1502_ANSWER_ORDER_SEE_VPN_PAR						0X1215
#define DF_TY_1502_ANSWER_ORDER_SEE_CLOSE_AUTO_UPDATE			0X1216
#define DF_TY_1502_ANSWER_ORDER_SEE_MONTH_FP_NUM				0X1217
#define DF_TY_1502_ANSWER_ORDER_SEE_FP_DETAILS					0X1218	
#define	DF_TY_1502_ANSWER_ORDER_SEE_FP_TYPES					0X1219
#define	DF_TY_1502_ANSWER_ORDER_SEE_MONITOR_DATA				0X1220
#define	DF_TY_1502_ANSWER_ORDER_SEE_INVOICE_NUM					0X1221
#define DF_TY_1502_ANSWER_ORDER_SEE_FP_DETAILS_T				0X1222
#define DF_TY_1502_ANSWER_ORDER_SEE_FP_DETAILS_S				0X1223
#define DF_TY_1502_ANSWER_ORDER_SEE_FP_AREA_CODE				0X1224
#define DF_TY_1502_ANSWER_ORDER_SEE_TAX_INFOS					0X1225
#define DF_TY_1502_ANSWER_ORDER_SEE_VERSION_DESCRIBE			0X1226
#define DF_TY_1502_ANSWER_ORDER_OPER_OPEN_USB                   0X1301
#define DF_TY_1502_ANSWER_ORDER_OPER_CLOSE_USB                  0X1302
#define DF_TY_1502_ANSWER_ORDER_OPER_ID_OPEN_USB                0X1303
#define DF_TY_1502_ANSWER_ORDER_OPER_ID_CLOSE_USB               0X1304
#define DF_TY_1502_ANSWER_ORDER_OPER_CLOSE_NOW                  0X1305
#define DF_TY_1502_ANSWER_ORDER_OPER_POWER_OFF                  0X1306 
#define DF_TY_1502_ANSWER_ORDER_OPER_HEART                      0X1307
#define DF_TY_1502_ANSWER_ORDER_OPER_RESET                      0X1308
#define DF_TY_1502_ANSWER_ORDER_OPER_REDIT                      0X1309
#define DF_TY_1502_ANSWER_ORDER_OPER_OPEN_NOACTIVE              0X1310
#define DF_TY_1502_ANSWER_ORDER_SEE_OPEN_PORT                   0X1311 
#define DF_TY_1502_ANSWER_ORDER_OPER_UPDATA						0X1312
#define DF_TY_1502_ANSWER_ORDER_OPER_DETTACH_PORT				0X1313
#define DF_MB_ANSWER_SEE_USB_SHARE_DEV							0X1401
#define	DF_MB_ANSWER_OPER_RESTART								0X1402
/*------------------------------应答错误命令---------------------------------*/
#define DF_TY_1502_ANSWER_ORDER_ERR                             0XFFFF




// typedef struct
// {
// 	char *name;						//模块名字
// 	char *r_date;					//发布日期
// 	char *ver;						//发布版本
// 	char *v_num;					//顺序迭代版本号
// 	char *abdr;						//当前版本类型：alpha beta debug release
// 	char *cpy;						//公司
// 	char *writer;					//编写人
// 	char *note;						//备注
// 	char *type;						//型号
// 	char *code;						//码制类型
	
// }_so_note;


struct _message_line
{
	unsigned short inuse;
	unsigned short state;
	struct mqtt_parm parm;
	unsigned char fpqqlsh[60];
	unsigned int message_time;
	char *message;
};

struct _history_line
{
	unsigned short state;
	unsigned short result;
	char random[129];
	char data[4096];
};

struct _message_callback
{
	struct _message_line message_line[MAX_MESSAGE_QUEUE_NUM];
	unsigned int insert_num;
	unsigned int deal_num;
	unsigned int task_num;
	unsigned int err_count;
};

struct _message_history
{
	struct _history_line history_line[MAX_HISTROY_DEAL_RESULT];
	unsigned int deal_num;
};

struct _app_order
{   
	int order;
	char *name;  
	int (*answer)(int fd,uint32 ip,MODULE  *par,uint8 *buf,int buf_len);
};

struct _ty_err_table
{
	uint8 err;
	int err_ty;
};

struct err_no_infos
{
	int err;
	int errnum;
	char errinfo[200];
};

struct err_info
{
	int err_code;
	char *err_info;
};

MODULE *get_module(void);

#endif
