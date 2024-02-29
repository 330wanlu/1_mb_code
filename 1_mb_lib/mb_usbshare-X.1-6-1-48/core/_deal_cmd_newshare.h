#ifndef _DEAL_CMD_NEWSHARE_H
#define _DEAL_CMD_NEWSHARE_H
#include <string.h>
#include "_deploy.h"
#include "_ty_usb_m.h"
#include "../_opt.h"
#include "_machine_infor.h"
#include "_port_core.h"	
#include <openssl/md5.h>

#ifdef _deal_cmd_newshare_c
	#ifdef DEBUG
	#define out(s,arg...)       				//log_out("all.c",s,##arg) 
	#else
	#define out(s,arg...)						//log_out("all.c",s,##arg) 
	#endif
#define _deal_cmd_newshare
#else
#define _deal_cmd_newshare		extern
#endif


	#define DF_MB_HARDWARE_A20_A33_THIS	1//A20定义为0，A33定义为1
	#define DF_DEV_TYPE_THIS		2//1:MB1601 MB1603A MB1603B	2:MB1602	3:MB1603C	4:MB1602A	5:MB1805	6:MB1806

#define NEW_USB_SHARE_PROTOCOL				"mb_usbshare_20191024"
#define NEW_USB_SHARE_SYNC_CODE_REQUEST		"request"
#define NEW_USB_SHARE_SYNC_CODE_REPORT		"report"
#define NEW_USB_SHARE_SYNC_CODE_RESPONSE	"response"


#define NEW_PROTOCOL_ERR_SYSTEM		-100
#define NEW_PROTOCOL_ERR_NOT_TAX	-101
#define NEW_PROTOCOL_ERR_JSON		-102
#define NEW_PROTOCOL_ERR_PORT_ERR		-105
#define NEW_PROTOCOL_ERR_NOT_AUTH		-106
#define NEW_PROTOCOL_ERR_PORT_BUSY		-107
//#define NEW_PROTOCOL_ERR_



#define RESULT_OK_FOR_APPLY_PORT_USE_GW		2
#define RESULT_OK_FOR_APPLY_PORT_USE_LOCAL 3


/******************************************************结构体*********************************************************/
#ifdef NEW_USB_SHARE

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

#ifdef _deal_cmd_newshare_c

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
#endif
#endif


/*****************************函数声明**************************************/
		

	_deal_cmd_newshare int new_protocol_err_back(int sockfd, int err_no, struct json_parm_new_share *parm);
	_deal_cmd_newshare	int init_parm(const _so_note    *note);
	_deal_cmd_newshare int deal_cmd_newshare(int sockfd,unsigned char *json_in,void *output,int err_back_flag);
#ifdef _deal_cmd_newshare_c
	static int ty_socket_write(int sock, uint8 *buf, int buf_len);
	static int set_ip_new_share(int sockfd, uint8 *json_in, struct json_parm_new_share *parm, void *output);
	static int set_id_date_new_share(int sockfd, uint8 *json_in, struct json_parm_new_share *parm, void *output);
	static int apply_port_use(int sockfd, uint8 *json_in, struct json_parm_new_share *parm, void *output);
	static int restart_ter(int sockfd, uint8 *json_in, struct json_parm_new_share *parm, void *output);
	static int set_ter_func(int sockfd, uint8 *json_in, struct json_parm_new_share *parm, void *output);
	static int get_ter_all_infos_new_share(int sockfd, uint8 *json_in, struct json_parm_new_share *parm, void *output);
	static int ter_update_new_share(int sockfd, uint8 *json_in, struct json_parm_new_share *parm, void *output);



	//file opera
	static int Write_Decompress_file(unsigned char *file, long file_len, unsigned char *filename, unsigned char * Dest_dir);
	static int parse_config_copy_file(unsigned char *configname, unsigned char *now_path);
#ifndef MTK_OPENWRT
	static int mb_delete_file_dir(char *file_path, int file_type);
#endif
	static int mb_tar_decompress(char *tar, char *path);
	static int mb_chmod_file_path(char *power, char *path);
	static int mb_create_file_dir(char *file_path);
	static int mb_copy_file_path(char *from, char *to);
	static int File_Compare_MD5(char *src_file, char *comp_file);

	//end file opera
#endif
#ifdef _deal_cmd_newshare_c
	struct json_parm_new_share ccomm_stream;
	static const struct _new_share_errinfo new_protocol_err_n[] =
	{
		{ NEW_PROTOCOL_ERR_SYSTEM, "System err!" },
		{ NEW_PROTOCOL_ERR_NOT_TAX, "This device is not Tax disc!" },
		{ NEW_PROTOCOL_ERR_JSON, "Json parm err!" },
		{ NEW_PROTOCOL_ERR_PORT_ERR, "Port err(device not exist)!" },
		{ NEW_PROTOCOL_ERR_NOT_AUTH, "Port unauthorized!" },
		{ NEW_PROTOCOL_ERR_PORT_BUSY, "Port in used!" },

	};
	
	static const struct _new_share_order new_shr_order[] =
	{

			{ "set_ip",						"[设置类]:设置IP地址",						set_ip_new_share					},
			{ "set_id_date",				"[设置类]:设置机器编号和生产日期",			set_id_date_new_share				},
			{ "ter_status",					"[查询类]:查询终端全部信息",				get_ter_all_infos_new_share			},
			{ "apply_use",					"[动作类]:申请端口使用权",					apply_port_use						},
			{"ter_restart",					"[动作类]:重新启动",						restart_ter							},
			{ "set_ter_func",				"[动作类]:设置终端功能停启用",				set_ter_func						},
			{ "ter_update",					"[动作类]:文件上传及升级",					ter_update_new_share				},




		

	};
#endif


#endif