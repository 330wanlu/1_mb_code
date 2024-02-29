#ifndef _BLUETOOTH_H
#define _BLUETOOTH_H
#include "_switch_dev.h"
#include "_machine_infor.h"


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
/*--------------------------------------�ڲ����ݶ���------------------------------------------*/
#ifdef _bluetooth_c
	#ifdef DEBUG
	#define out(s,arg...)       				//log_out("all.c",s,##arg) 
	#else
	#define out(s,arg...)						log_out("all.c",s,##arg) 
	#endif
#define e_bluetooth
#else
#define e_bluetooth             extern
#endif




#ifdef _bluetooth_c
struct _bluetooth_fd
{
	int state;
	struct ty_file_mem	*mem;
	void *mem_dev;
	struct ty_file_fd	*mem_fd;
	struct _bluetooth  *dev;
};

enum Blesta
{
	Ble_init = 1,
	Ble_broadcasting,
	Ble_ready,
	Ble_connect_timeout,
	Ble_connect_success,

};
struct _bluetooth
{
	struct _bluetooth_fd   fd[DF_FILE_OPEN_NUMBER];
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
	int ble_sta;//0 init 5 connected 2 disconnected
	int ble_name_init;
	int ble_start_time;
	char switch_name[DF_FILE_NAME_MAX_LEN];
	char machine_id[13];
	struct _dev_support dev_support;
	_so_note    *note;
	//MyAttr ble_attr;
};


struct _bluetooth_ctl_fu
{
	int cm;
	int(*ctl)(struct _bluetooth_fd   *id, va_list args);
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
#endif
struct _bluetooth_head{
	char ver[5];
	char data_len[7];
	char data_format[3];
	char encryption[3];
	char whole_packs_num[5];
	char pack_seria_num[5];
	char reserved_bit[9];
	char stopbit;
};

/*========================================��������=================================================*/
e_bluetooth int bluetooth_add(char *seria_name, const _so_note    *note, const char *switch_name);
#ifdef _bluetooth_c
static int bluetooth_open(struct ty_file	*file);
static int bluetooth_ctl(void *data, int fd, int cm, va_list args);
static int bluetooth_close(void *data, int fd);
//static int bluetooth_init(struct _bluetooth *stream, uint8 *machine_id);
//ctl order
static int set_bluetooth_name(struct _bluetooth_fd   *id, va_list args);

//end ctl order
static int ble_2at_disconnect(struct _bluetooth *stream);
static void bluetooth_order_task(void *arg, int timer);
static void bluetooth_detect_task(void *arg, int timer);
//����ص�����
//������������
static int ble_test_conn(struct _bluetooth *stream, uint8 *json_in, struct json_parm_bluetooth *parm);

//��ȡ���ж˿�״̬--��������
static int get_all_ports_status(struct _bluetooth *stream, uint8 *json_in, struct json_parm_bluetooth *parm);
//��ȡ���ж˿�ADֵ
static int get_all_ports_ad(struct _bluetooth *stream, uint8 *json_in, struct json_parm_bluetooth *parm);
//��ȡ��Կ�������״̬
static int get_key_status(struct _bluetooth *stream, uint8 *json_in, struct json_parm_bluetooth *parm);
static int set_ip_by_bluetooth(struct _bluetooth *stream, uint8 *json_in, struct json_parm_bluetooth *parm);
static int get_terminal_info_by_bluetooth(struct _bluetooth *stream, uint8 *json_in, struct json_parm_bluetooth *parm);


static int scanning_wifi_list(struct _bluetooth *stream, uint8 *json_in, struct json_parm_bluetooth *parm);
static int set_wifi_config(struct _bluetooth *stream, uint8 *json_in, struct json_parm_bluetooth *parm);
static int get_configured_wifi_list(struct _bluetooth *stream, uint8 *json_in, struct json_parm_bluetooth *parm);
static int delete_configured_wifi_list(struct _bluetooth *stream, uint8 *json_in, struct json_parm_bluetooth *parm);
//�ϵ����ж˿�
static int ble_up_all_power(struct _bluetooth *stream, uint8 *json_in, struct json_parm_bluetooth *parm);
//�µ����ж˿�
static int ble_down_all_power(struct _bluetooth *stream, uint8 *json_in, struct json_parm_bluetooth *parm);
//��ȡhub��mcu״̬
static int get_hub_mcu_status(struct _bluetooth *stream, uint8 *json_in, struct json_parm_bluetooth *parm);
static int reg_ter_id(struct _bluetooth *stream, uint8 *json_in, struct json_parm_bluetooth *parm);
static int control_dev_restore(struct _bluetooth *stream, uint8 *json_in, struct json_parm_bluetooth *parm);
static int control_ble_disconn(struct _bluetooth *stream, uint8 *json_in, struct json_parm_bluetooth *parm);
//����RX TX ����
static int bluetooth_send_recv(struct _bluetooth	*stream, char *sbuf, char *rbuf, int rbuf_len);
static int comport_send(struct _bluetooth *stream, char *sbuf, int sbuf_len);
static int bluetooth_send_packets(struct _bluetooth *stream, char *sbuf, int len, char *encryption);
#endif    
/*=======================================��������==================================================*/
#ifdef _bluetooth_c
static const struct _bluetooth_ctl_fu ctl_fun[] = {
	{ DF_CORE_CM_SET_BLE_NAME ,set_bluetooth_name},
};

static const struct _file_fuc	bluetooth_fuc =
{ 
	.open = bluetooth_open,
	.read = NULL,
	.write = NULL,
	.ctl = bluetooth_ctl,
	.close = bluetooth_close,
	.del = NULL
};
static const struct _bluetooth_errinfo ble_err[] = {
	{ BLUETOOTH_ERR_SYSTEM,					"System Err"						},
	{ BLUETOOTH_ERR_SEND,					"Send Err"							},
	{ BLUETOOTH_ERR_RECV,					"Recv Err"							},
	{ BLUETOOTH_ERR_PARM,					"Parm Jason Err"					},
	{ BLUETOOTH_ERR_HEAD ,					"Parm Head Err"						},	
	{ BLUETOOTH_ERR_WIFI_SET_LAN_ERR,		"Wired network is not turned off"	},
	{ BLUETOOTH_ERR_WIFI_SET_PASSWD_ERR,	"Wifi passwd error"					},
	{ BLUETOOTH_ERR_WIFI_SCAN_WIFI_ERR,		"WiFi device list search failed"	},	
	{ BLUETOOTH_ERR_WIFI_NO_CONGIGUED_INFO, "No wifi local configuration"		}
};

static const struct _bluetooth_order bluetooth_orders[] =
{
	{ "test_conn",				"[������]����������",				ble_test_conn					},
	{ "get_terinfo",			"[��ѯ��]:��ѯ����ȫ����Ϣ",		get_terminal_info_by_bluetooth},
	{ "get_all_ports_status",	"[��ѯ��]:��ȡ�������ж˿�״̬",	get_all_ports_status			},
	{ "get_all_ports_ad",		"[��ѯ��]:��ȡ�������ж˿�ADֵ",	get_all_ports_ad				},
	{ "get_key_status",			"[��ѯ��]:��ȡ������Կ״̬",		get_key_status					},
	{ "get_hub_mcu_status",		"[��ѯ��]:��ȡ����hub����Ƭ��״̬", get_hub_mcu_status				},
	{ "set_ip",					"[������]:����IP��ַ",				set_ip_by_bluetooth},
	{ "dev_restore",			"[������]:�س���",					control_dev_restore},
	{ "ble_disconn",			"[������]:�Ͽ�����",				control_ble_disconn},
	{ "reg_terminal",			"[������]:�������ע��",			reg_ter_id						},
	{ "down_all_power",			"[������]:�µ����ж˿�",			ble_down_all_power				},
	{ "up_all_power",			"[������]:�ϵ����ж˿�",			ble_up_all_power				},
	{ "scan_wifi",				"[��ѯ��]:����WLAN����",			scanning_wifi_list},
	{ "get_wifi",				"[��ѯ��]:��ѯ��ǰ������wifi",		get_configured_wifi_list },
	{ "set_wifi",				"[������]:����wifi����",			set_wifi_config },
	{ "del_wifi",				"[������]:ɾ��������wifi��Ϣ",		delete_configured_wifi_list }
};

#endif

#endif