#ifndef _DEAL_CMD_NEWSHARE_H
#define _DEAL_CMD_NEWSHARE_H

#include "opt.h"

#include "deploy.h"
#include "opt.h"
#include "machine_infor.h"
#include "ip_check.h"
#include "usb_port.h"	

#define deal_cmd_newshare_out(s,arg...)       				//log_out("all.c",s,##arg)
	
#define DF_MB_HARDWARE_A20_A33_THIS		1//A20定义为0，A33定义为1
#define DF_DEV_TYPE_THIS				2//1:MB1601 MB1603A MB1603B	2:MB1602	3:MB1603C	4:MB1602A	5:MB1805	6:MB1806

#define NEW_USB_SHARE_PROTOCOL				"mb_usbshare_20191024"
#define NEW_USB_SHARE_SYNC_CODE_REQUEST		"request"
#define NEW_USB_SHARE_SYNC_CODE_REPORT		"report"
#define NEW_USB_SHARE_SYNC_CODE_RESPONSE	"response"

#define NEW_PROTOCOL_ERR_SYSTEM		-100
#define NEW_PROTOCOL_ERR_NOT_TAX	-101
#define NEW_PROTOCOL_ERR_JSON		-102

#define NEW_PROTOCOL_ERR_OPER		-103	
#define NEW_PROTOCOL_ERR_SOCKET		-104

#define NEW_PROTOCOL_ERR_PORT_ERR		-105
#define NEW_PROTOCOL_ERR_NOT_AUTH		-106
#define NEW_PROTOCOL_ERR_PORT_BUSY		-107
#define NEW_PROTOCOL_ERR_PORT_UPDATING		-108
#define NEW_PROTOCOL_ERR_PORT_IS_ROBOIARM		-109
#define NEW_PROTOCOL_ERR_NOT_WITH_ROBOIARM -110
//#define NEW_PROTOCOL_ERR_

#define RESULT_OK_FOR_APPLY_PORT_USE_GW		2
#define RESULT_OK_FOR_APPLY_PORT_USE_LOCAL 3

struct _deal_cmd_new_fd
{
	int deploy_fd;
	int machine_fd;
	int ty_usb_fd;
	int ty_usb_m_fd;
	int event_file_fd;
	int get_net_fd;
	int inv_read_lock;
	int sockfd;
	int state;
	_so_note    *note;
};

struct json_parm_new_share
{
	char protocol[15];
	char code_type[10];
	char cmd[50];
	char result[2];
	char source_topic[100];
	char random[128];
	int port;
	struct _deal_cmd_new_fd deal_cmd_fd;
	//char mechine_id[13];
};

struct _new_share_order
{
	char *order;
	char *name;
	int(*answer)(int sockfd, uint8 *json_in, struct json_parm_new_share *parm, void *output);
};

struct _new_share_errinfo
{
	int err_no;
	char *errinfo;
};

int new_protocol_err_back(int sockfd, int err_no, struct json_parm_new_share *parm);
int init_parm(const _so_note    *note);
int deal_cmd_newshare(int sockfd,unsigned char *json_in,void *output,int err_back_flag);

#endif
