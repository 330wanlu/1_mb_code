#ifndef _BLUETOOTH_H
#define _BLUETOOTH_H

#include "opt.h"

#include "switch_dev.h"
#include "machine_infor.h"
#include "usb_port.h"

#define BULETOOTH_HEAD_LEN	30
#define MAX_SINGLE_LEN	1480

#define BLUETOOTH_PROTOCOL				"mb_bluetooth_202106"
#define BLUETOOTH_SYNC_CODE_REQUEST		"request"
#define BLUETOOTH_SYNC_CODE_REPORT		"report"
#define BLUETOOTH_SYNC_CODE_RESPONSE	"response"

#define BLUETOOTH_ERR_SYSTEM					-10001								//-10001
#define BLUETOOTH_ERR_SEND						BLUETOOTH_ERR_SYSTEM-1				//-10002
#define BLUETOOTH_ERR_RECV						BLUETOOTH_ERR_SEND-1				//-10003
#define BLUETOOTH_ERR_PARM						BLUETOOTH_ERR_RECV-1				//-10004
#define BLUETOOTH_ERR_HEAD						BLUETOOTH_ERR_PARM-1				//-10005
#define BLUETOOTH_ERR_WIFI_SET_LAN_ERR			BLUETOOTH_ERR_HEAD-1				//-10006
#define	BLUETOOTH_ERR_WIFI_SET_PASSWD_ERR		BLUETOOTH_ERR_WIFI_SET_LAN_ERR-1	//-10007
#define	BLUETOOTH_ERR_WIFI_SCAN_WIFI_ERR		BLUETOOTH_ERR_WIFI_SET_PASSWD_ERR-1	//-10008
#define BLUETOOTH_ERR_WIFI_NO_CONGIGUED_INFO	BLUETOOTH_ERR_WIFI_SCAN_WIFI_ERR-1	//-10009

#define DF_CORE_CM_SET_BLE_NAME                    300

#define bluetooth_out(s,arg...)       	//log_out("all.c",s,##arg)

struct _bluetooth_fd
{
	int state;
	struct ty_file_mem *mem;
	void *mem_dev;
	struct ty_file_fd *mem_fd;
	struct _bluetooth *dev;
};

enum Blesta
{
	Ble_init = 1,
	Ble_broadcasting,
	Ble_ready,
	Ble_connect_timeout,
	Ble_connect_success
};


///home/share/exe/tar/tmpapp/tmp/app.tar
#define BULETOOTH_UPGRADE_FILE_PATH "/home/share/exe/tar/tmpapp"
#define BULETOOTH_DOWNLOAD_FILENAME	"/tmp/app.tar"

enum upgarde_status_T
{
	NO_UPGRADE = 0,   //未升级
	UPGRADING,		//升级中
	UPGRADED_SUCCESSED,  //升级成功
	UPGRADED_FIALED		//升级失败
};

struct _bluetooth_upgrade
{
	int file_size;
	int pack_size;
	int pack_total_num;   //总包数
	char file_md5[64];
	int cur_pack_num;  //当前包数
	int file_data_size;  //升级文件长度
	enum upgarde_status_T upgarde_status;   //升级状态
	char *file_data;
};

struct _bluetooth
{
	struct _bluetooth_fd fd[DF_FILE_OPEN_NUMBER];
	int state;
	int lock;
	int task;
	int machine_fd;
	int serial_fd;
	int deploy_fd;
	int get_net_fd;
	int ty_usb_fd;
	int ty_usb_m_fd;
	int event_file_fd;
	int switch_fd;
	int ble_sta;	//0 init 5 connected 2 disconnected
	int ble_name_init;
	int ble_start_time;
	char switch_name[DF_FILE_NAME_MAX_LEN];
	char machine_id[13];
	struct _dev_support dev_support;
	_so_note *note;
	struct _bluetooth_upgrade upgrade;
};

struct _bluetooth_ctl_fu
{
	int cm;
	int(*ctl)(struct _bluetooth_fd *id,va_list args);
};

struct json_parm_bluetooth
{
	char protocol[15];
	char code_type[10];
	char cmd[50];
	char result[2];
	char source_topic[100];
	char random[128];
	struct _bluetooth *bluetooth_stream;
	struct _bluetooth_head *head;
};

struct _bluetooth_order
{
	char *order;
	char *name;
	int(*answer)(struct _bluetooth *stream, uint8 *json_in, struct json_parm_bluetooth *parm);
};

struct _bluetooth_errinfo
{
	int err_no;
	char *errinfo;
};

struct _bluetooth_head
{
	char ver[5];
	char data_len[7];
	char data_format[3];
	char encryption[3];
	char whole_packs_num[5];
	char pack_seria_num[5];
	char reserved_bit[9];
	char stopbit;
};

int bluetooth_add(char *seria_name,const _so_note *note,const char *switch_name);

#endif