#ifndef CLIENT_PORTOCOL_H_
#define CLIENT_PORTOCOL_H_


#include <Winsock2.h>
#include "typedef_data.h"
#include "err.h"
#include "Macro.h"
//#ifdef NINGBO_BANK
#define DF_MANAGE_SET_FILE
#include "manage_set_file.h"
//#endif
struct Client_Err{
	int self_err;
	uint8 ret_err;
	char *err_des;
};


struct FPHMDM{
	uint8 fpdm[20];
	uint8 fphm[20];
};

struct ClientInvoiceInfo{
	uint32 invoice_sum;//发票数量

	struct FPHMDM fpdm_hm[10000];

	uint8 dir[260];
};

typedef struct ClientProtocol{
	unsigned char head[4];
	unsigned char cmd;
	unsigned char protocol_data[20000];
}ST_CLientProtocol, *p_ST_ClientProtocol;


typedef struct ClientVPNSetting{
	unsigned char terminal_id[20];
	int open;//是否开启VPN
	int login_type;//VPN登录方式
	unsigned char VPNServerIP[50];//VPN服务器地址
	unsigned char VPNServerPort[2];//VPN服务器端口号
	unsigned char account[20];//用户名
	unsigned char passwd[20];//密码
	unsigned char FTPPort[2];//FTP下载端口号
	unsigned char FTPUsername[20];//FTP下载用户名
	unsigned char FTPPasswd[20];//FTP下载用户密码
}ST_ClientVPNSetting;


volatile struct InvoiceSchedulePerCmd
{
	uint8 terminal_id[20];//终端id
	uint32 usbport;//端口号

	volatile double invoice_bin_len;//发票二进制数据总长度
	volatile double invoice_bin_recved;//已接受的发票数据插长度

	volatile double invoice_sum;//发票总数
	volatile double invoice_recved;//已接收发票数量
	uint32 schedule;//进度
};

//发票获取进度表
struct InvoiceSchedule
{
	struct InvoiceSchedulePerCmd schedule_table[SCHEDULE_TABLE];
};

int ExchangeInvoiceNum(int mode, int type);
int cmd_handle(SOCKET	sockfd, char *ip, int ip_port, void *arg, int* cmd, unsigned char* cmd_buf);
int recv_client_portocol(SOCKET sockfd, unsigned char** out_buf);

#ifdef DF_CLIENT_PROTOCOL_C
static const struct Client_Err client_err[] =
{
	{ DF_ERR_TERMINAL_SYSTEM, 0x01, "终端系统错误" },
	{ DF_ERR_USBPORT_NO_DEVICE, 0x06, "端口无设备" },
	{ DF_ERR_NO_RIGHT_TERMINAL, 0x07, "无操作权限" },
	{ DF_ERR_UBSPORT_OPENED, 0x0f, "设备已被使用" },
	{ DF_ERR_NO_RIGHT_CLOSE_PORT, 0x10, "无权释放该端口" },
	{ DF_UPGRADE_PROGRAM_ERROR, 0x22, "升级终端程序失败" },
	{ DF_UPGRADE_PROGRAM_ERROR, 0x23, "升级终端程序失败" },
	{ DF_ERR_TYPE_NO_SUPPORT, 0x24, "税盘不支持" },
	{ DF_ERR_INVOICE, 0x25, "查询月无发票信息" },
	{ DF_ERR_GUARD, 0x26, "不支持此类型发票" },
	{ DF_ERR_PORT_CORE_GETFP_PARAMENTS  ,0x27 ,"获取税盘发票参数错误"},
	{ DF_ERR_PORT_GETFP_OPENR_USB ,0x28 , "获取税盘信息操作USB失败"},
	{ DF_ERR_PORT_CORE_GET_FP_INVOICE_NUM ,0x29 , "获取税盘发票数量错误"},
	{ DF_ERR_PORT_CORE_GETFP_INVOICE_LEN ,0x2A , "获取发票数据大小错误"},
	{ DF_ERR_SOCKET_ERROR, 0x81, "网络错误" },//网络连接不上
	{ DF_ERR_PROTOCOL_ERROR, 0x82, "通讯协议错误" },
	{ DF_ERR_EXCUTE_USBSHARE_ERR, 0x85, "USB总线端口申请失败" },//
	{ DF_ERR_USBPORT_ERROR, 0x86, "输入的USB端口号错误" },
	{ DF_ERR_TERMINAL_ID, 0x8E, "输入的终端ID错误" },
	{ DF_ERR_TERMINAL_OFFLINE, 0x8F, "终端不在线" },
	{ DF_ERR_CLIENT_PROTOCOL, 0x88, "协议不支持" },
	{ DF_USB_UNLOAD_TIMEOUT, 0x90, "端口释放超时" },
	{ DF_USB_LOAD_TIMEOUT, 0x91, "端口加载超时" },
	{ DF_SEARCH_COMPLETE, 0x92, "税盘查询完成" },//  
	{ DF_ERR_ON_SEARCHING, 0x93, "正在查询该税盘,请进行进度查询" },//
	{ DF_SYSTEM_ERROR  , 0x94 , "计算机错误"},
	{ DF_ERR_NO_SEARCH_SCHEDULE  , 0x95 , "未有该月发票明细任务查询"}
};
#endif


#endif
