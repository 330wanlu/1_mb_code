#ifndef _TY_USB_M_DEFINE 
#define _TY_USB_M_DEFINE
#include "_ty_usb.h"
#include "_ty_pd.h"
#include "_event_file.h"
#include <dlfcn.h>
#include <signal.h>
#include <errno.h>
#include <stdlib.h>
#include "_tran_data.h"
#include "../manage/_process_manage.h"
#include "../linux_sys/mqtt_client.h"
#include "../linux_sys/_http.h"
#include "../manage/_process_manage.h"
#include "_lcd_state.h"
#ifndef MTK_OPENWRT
#include "../linux_sys/_mb_sql.h"
#endif
#include "_deploy.h"
#include "_get_net_state.h"
#include "_machine_infor.h"
#include "_m_server.h"
#include "_invoice_transmit.h"

/*====================================立即数定义==============================================*/

#define DF_CLIENT_ASC_NUMBER            64      //客户端标识符
#ifdef _ty_usb_m_c
#ifdef DEBUG
#define out(s,arg...)       				log_out("all.c",s,##arg) 
#else
#define out(s,arg...)						log_out("all.c",s,##arg)
#endif	

#define e_ty_usb_m
#define DF_TY_USB_DATA_MAX_TIMER_S      5				//链路连接超时时间
#define DF_TY_USB_MQTT_REPORT_STATUS	15				//MQTT定时上报间隔时间老接口
#define DF_TY_USB_MQTT_M_SERVER_HEART_REPORT	60			//MQTT定时上报间隔时间新接口
#define DF_TY_USB_MQTT_M_SERVER_STATUS_REPORT	3			//MQTT定时第一次状态上报间隔时间新接口
#define DF_TY_USB_MQTT_OFFLINE_UP_TIME   2000				//上传离线票与最后一次开票间隔时间

#define POWER_ERR_FULL_LOAD	-1008
#define POWER_ERR_NO_DEV 	-1009

#define DF_PLATE_USB_PORT_NOT_AUTH -106

#else
#define e_ty_usb_m                      extern 
#endif
/*====================================结构体定义==============================================*/


struct	_queue_state
{
	uint8 use_flag;         //是否在使用
	char summary_month[64]; //需要上传月份
	char now_month[64]; 	//目前上传月份
	char start_time[64];    //开始时间
	char end_time[64];    //开始时间
};


struct _usb_port_infor
{
	/*USB属性*/
	char port_str[200];
	unsigned short port;
	unsigned short port_power;					//端口电源 0关电状态	1开电状态
	unsigned short with_dev;
	char busid[DF_TY_USB_BUSID];
	uint8 subclass;
	uint8 dev_type;
	uint8 protocol_ver;
	uint16 vid;
	uint16 pid;
	uint8 bnuminterface;
	int port_used;		//端口是否使用 1使用中  0未使用
	unsigned short system_used;		//系统使用 1系统使用
	long used_time;					//端口使用时间

	int used_level;					//1usb_share使用  2证书及基本信息读取  3发票开具、作废、汇总反写及上传  4税盘数据查询  5发票数据查询  6发票数据查询并上传
	char used_info[1024];

	unsigned short usb_app; // 0未知	1金税盘	2税控盘	3模拟盘	4 ca / ukey	5 其他
	unsigned short usb_share;//是否被usb-share加载使用0无		1有
	unsigned short usb_share_mode;//usb-share加载使用模式 0内网使用		1公网转发使用			2其他方式


	int check_num;				//检测次数，关闭端口后设备在文件系统中消失，检测次数
	//long port_insert_time;			//端口USB设备插入时间戳
	int tran_data;					//USB有数据读写



	/*USBIP属性*/
	unsigned short port_status;            //状态,usbip中状态  0未使用  2共享中
	uint32 ip;
	long last_open_time;		//最后一次开启使用时间
	int usbip_state;			//端口是否使用  1usbip打开状态   0usbip关闭状态
	char client[DF_CLIENT_ASC_NUMBER];      //客户端标识
	//int connect_timer;          //连接时间（命令层打开端口后登录数据链路连接时长）
	int app_fd;                 //应用使用句柄


	/*证书属性*/
	//unsigned short ca_read;	//标记证书名称和税号读取--标志位
	uint8 ca_name[110];
	uint8 ca_serial[20];
	unsigned short encording;
	unsigned short ca_ok;
	unsigned short in_read_base_tax;


	/*税盘信息属性*/
	unsigned short tax_read;//标记税务信息读取--标志位
	unsigned short need_chaoshui;//需要抄税
	unsigned short need_huizong;//需要上报汇总
	unsigned short need_fanxiejiankong;//需要反写监控


	unsigned short inv_query;				//发票查询动作，0关闭 1开启（作为停止发票查询使用）
	uint8 sn[16];//税盘编号
	unsigned char plate_time[20]; //读取税盘基本信息时的税盘时间
	unsigned short extension;//分机号 主盘为0
	int plate_sys_time;			//税盘与当前系统时间差
	uint8 month_fp_num[6]; //记录上次查询月份发票总数
	int fp_num;				//记录上次查询到的发票总数
	unsigned long long last_kp_time;		//最后开票时间,超过2秒方允许上传离线票
	int offinv_num_exit;	//离线票张数或是否存在离线票
	int offinv_stop_upload;	//停止离线票上传功能， 0不停止 1停止
	int off_up_state;		//离线上传标志 0未在上传  1上传中
	int upfailed_count;		//上传失败次数
	struct _upload_inv_num upload_inv_num;
	int cb_state;		//抄报状态  0 未知  1 抄报成功  -1失败
	int hz_state;		//汇总状态  0 未知  1 汇总成功  -1失败
	int qk_state;		//清卡状态  0 未知  1 清卡成功  -1失败


	int m_inv_state;	//发票数据与M服务同步状态	0未同步  1正在同步  2同步完成	-1出错
	int m_tzdbh_state;	//红字信息表与M服务同步状态 0未同步  1正在同步  2同步完成	-1出错
	//int m_netinv_state; //待下载票源与M服务同步状态 0未同步  1正在同步  2同步完成	-1出错

	int e_tzdbh_c;	//红字信息表获取失败次数
	//int e_netinv_c; //待下载票源获取失败次数

	struct _plate_infos plate_infos;

	struct _queue_state queue_state; //发票同步队列使用情况


	int connect_success_count;
	int connect_failed_count;
	int connect_success_time;
	int connect_failed_time;

	char start_time_s[30];
	char end_time_s[30];
	//char connect_errinfo[1024];

	char tax_business[3000];

	int ad_status;//ad状态
#ifdef ALL_SHARE
	unsigned short ca_read;
	int fail_num;
#endif
	
};
#define REFER_TO_TAX_ADDRESS 1
#define NOT_REFER_TO_TAX_ADDRESS 0



#ifdef _ty_usb_m_c   
enum POWER_MODE
{
	Normal_Mode = 0,
	Saving_Mode,	
};


struct _ty_usb_m_fd
{
	int state;
	struct ty_file_mem	*mem;
	void *mem_dev;
	struct ty_file_fd	*mem_fd;
	struct _ty_usb_m    *dev;
};

struct _port_last_info
{
	unsigned short usb_app;// 0未知	1金税盘	2税控盘	3模拟盘	4 ca / ukey	5 其他
	int cert_err;				//默认初始为0  如第一次验证错误该至为-1  验证成功该值为1
	uint8 ca_name[110];
	uint8 ca_serial[20];
	uint8 sn[20]; //读取税盘基本信息时的税盘时间
	unsigned short extension;//分机号 主盘为0
	uint16 vid;
	uint16 pid;
};

//端口属性结构体
struct _port_state
{
	int port;                   //端口号
	int lock;                   //端口锁
	unsigned short power;					//端口电源 0关电状态	1开电状态
	int port_led_status;		//1没有设备且未授权   2设备在本地    3设备被远程使用 4在本地但证书异常 5系统使用中 6远程加载后有数据读写 7税盘默认口令异常 8税盘设备密码错误 9端口没有设备且授权过
	long insert_time;			//USB插入时间
	int usb_err;				//0未出现过错误 1出现过错误
	int usb_err_cs;				//连续出现过错误的次数
	//int cert_open;				//0未授权  1授权
	int aisino_sup;
	int nisec_sup;
	int cntax_sup;
	int mengbai_sup;
	char aisino_end_time[20];
	char nisec_end_time[20];
	char cntax_end_time[20];
	char mengbai_end_time[20];

	char start_time[20];
	char end_time[20];
	char frozen_ago_time[20];

	int retry_forbidden;		//禁止重试端口信息读取；  0不禁止  1禁止

	int off_report;//离线上报 0不需要  1需要
	int up_report;//更新上报 0不需要  1需要
	int last_act;//最后一次动作为 1在线  2离线

	int need_getinv;			//0不需要获取需上传的发票月份信息 1需要获取需上传的发票月份信息
	//int need_sync_tzdbh;		//0不需要获取并同步红字信息表 1需要获取并同步红字信息表
	//int need_sync_netinv;		//0不需要获取并同步待下载票源 1需要获取并同步待下载票源
	unsigned short in_read_base_tax;

	int inuse_sync;				//线程正在同步数据
	long last_without_time;
	struct _port_last_info port_last_info;//上一个USB设备的基本信息
	struct _usb_port_infor port_info;//端口信息
	//struct _ty_usb_m		*dev_stream;
	struct HandlePlate h_plate;
	int sys_reload; //处理某税务ukey加载前需断电重启端口但又不能重新读取名称税号的情况
	int power_mode;// 0 正常模式端口  1可省电模式端口
#ifndef DF_OLD_MSERVER
	unsigned long last_report_time_t;//发票同步信息查询时间
	uint16 now_report_counts;
	int in_report;
	int off_up_flag; //1离线上报 2在线上报
#endif
	//节电重新定义
	int port_init;	//端口一次使用或者出错后重新开始使用（重新读取基础信息）  0初始使用  1准备读取名称税号  2读取名称税号完成完成初始
	int need_up_power; //0不需要上电   1需要上电
	int need_down_power; //0不需要下电   1需要下电
	int ad_repair_check;
	long ad_repair_up_time;//记录最后一次操作上电的时间
	long ad_repair_down_time;//记录最后一次操作下电的时间
	int status;// 0无设备 1有设备  2短路
	int status_counts;
	
#ifdef ALL_SHARE
	int fail_num;
#endif
};

//全局结构体
struct _ty_usb_m
{
	int state;                                  //
	int lock;                                   //
	unsigned char key_s[4096];
	struct _port_state  port[DF_TY_USB_PORT_N];
	struct _ty_usb_m_fd fd[DF_FILE_OPEN_NUMBER];
	char ty_usb_name[DF_FILE_NAME_MAX_LEN];
	int ty_usb_fd;
	int lcd_fd;
	int timer_fd;
	int tran_fd;
	int deploy_fd;
	int event_file_fd;
	int switch_fd;
	int get_net_fd;
	int usb_port_numb;
	int frist_start;
	int frist_up_inv;
	int machine_fd;
	char ter_id[20];
	char *cert_json;
	char tax_so_ver[50];
	char mbc_so_ver[50];
	int up_queue_num;	//上传发票队列数
	int sync_queue_num;	//发票同步查询队列
	int base_read_queue_num;	//基础信息读取队列数
	int onlineport;//在线已经开电设备数量
	int poweredport;//已开电端口数量
	sem_t	cb_qk_sem;//离线票上传信号量

	//状态字
	int stop_aisino_up; //停止发票上传
	int stop_nisec_up;//停止发票上传
	int stop_cntax_up;//停止发票上传
	int stop_mengbai_up;//停止发票上传


	unsigned long last_report_time_o;//老上报接口最后上报时间
	unsigned long last_report_time_h;//新上报心跳最后上报时间
	unsigned long last_report_time_s;//新上报状态最后上报时间
	uint8 ad_status[4098];
	uint8 all_port_status[4096];
	int all_port_status_len;
	int all_port_updata;	//02 24 添加字段

	int report_s_used;//上报终端状态队列是否结束
	int report_h_used;//上报终端心跳队列是否结束
	int report_o_used;//上报终端老接口队列是否结束
#ifndef DF_OLD_MSERVER
	int report_t_used;//发票同步查询队列是否结束
#else
	int report_p_used;//上报终端端口信息队列是否结束
	int report_i_used;//上报终端口信息队列是否结束
#endif

	int report_ter_flag;//终端信息仅上传一次 0未成功上传需再次上传 1已成功上传无需再次上传	

	int lcd_share_state;//10usbshare不使用  20usbshare使用
	int lcd_tax_state;//10usbshare不使用  20usbshare使用
	int keyfile_load_err;//密钥数据读取失败
	int lib_load_err;//加密库加载失败
	int lib_dec_err;//加密库解密失败
	int auth_dec_err;//授权文件解密失败

#ifndef DF_OLD_MSERVER
	uint16 report_counts;
#endif
	struct _dev_support dev_support;

};

struct _ty_usb_m_ctl_fu
{
	int cm;
	int(*ctl)(struct _ty_usb_m_fd   *id, va_list args);
};
void *ty_usb_m_id;








#endif        

#ifdef RELEASE_SO

//==通用接口==//
//初始化动态库
typedef int(*_so_common_init_tax_lib)(uint8 *, char *);
_so_common_init_tax_lib							so_common_init_tax_lib;

//重载授权文件
typedef int(*_so_common_reload_tax_auth)(uint8 *);
_so_common_reload_tax_auth						so_common_reload_tax_auth;

//判断已识别税盘的授权信息
typedef int(*_so_common_jude_plate_auth)(uint8, char *,char *);
_so_common_jude_plate_auth						so_common_jude_plate_auth;

//初始化动态库内部日志
typedef int(*_so_common_init_tax_lib_log_hook)(void *, void *);
_so_common_init_tax_lib_log_hook				so_common_init_tax_lib_log_hook;

//获取票种信息对应结构体内存号
typedef int(*_so_common_find_inv_type_mem)(uint8, int *);
_so_common_find_inv_type_mem					so_common_find_inv_type_mem;

//获取授权文件信息
typedef int(*_so_common_get_cert_json_file)(uint8 *, char **);
_so_common_get_cert_json_file					so_common_get_cert_json_file;

//严重错误信息上报运维后台
typedef int(*_so_common_report_event)(char *, char *, char *, int);
_so_common_report_event							so_common_report_event;

//发票数据预检接口
typedef int(*_so_common_analyze_json_buff)(uint8 , char *, struct Fpxx *, char *);
_so_common_analyze_json_buff					so_common_analyze_json_buff;

//读取基础信息、税务信息及发票卷信息，支持金税盘、税控盘、模拟盘、税务ukey、航信勾选key、百旺勾选key
typedef int(*_so_common_get_basic_tax_info)(struct HandlePlate *, struct _plate_infos *);
_so_common_get_basic_tax_info					so_common_get_basic_tax_info;

//关闭USB设备句柄
typedef int(*_so_common_close_usb_device)(struct HandlePlate *);
_so_common_close_usb_device						so_common_close_usb_device;

typedef int(*_so_common_get_basic_simple)(struct HandlePlate *, struct _plate_infos *);
_so_common_get_basic_simple					so_common_get_basic_simple;



//==不需要授权且不需要口令验证通过亦可调用的接口==//
//获取税盘当前时间
typedef int(*_so_aisino_get_tax_time)(struct HandlePlate *, char *);
_so_aisino_get_tax_time							so_aisino_get_tax_time;
typedef int(*_so_nisec_get_tax_time)(struct HandlePlate *, char *);
_so_nisec_get_tax_time							so_nisec_get_tax_time;
typedef int(*_so_cntax_get_tax_time)(struct HandlePlate *, char *);
_so_cntax_get_tax_time							so_cntax_get_tax_time;
typedef int(*_so_mb_get_plate_time)(struct HandlePlate *, char *);
_so_mb_get_plate_time							so_mb_get_plate_time;

//mqtt接口获取发票明细
typedef int(*_so_aisino_get_inv_details_mqtt)(struct HandlePlate *, unsigned char *, void *, void *, char *, int, unsigned short *, int);
_so_aisino_get_inv_details_mqtt					so_aisino_get_inv_details_mqtt;
typedef int(*_so_nisec_get_inv_details_mqtt)(struct HandlePlate *, unsigned char *, void *, void *, char *, int, unsigned short *, int);
_so_nisec_get_inv_details_mqtt					so_nisec_get_inv_details_mqtt;
typedef int(*_so_cntax_get_inv_details_mqtt)(struct HandlePlate *, unsigned char *, void *, void *, char *, int, unsigned short *, int);
_so_cntax_get_inv_details_mqtt					so_cntax_get_inv_details_mqtt;
typedef int(*_so_mb_get_inv_details_mqtt)(struct HandlePlate *, unsigned char *, void *, void *, char *, int, unsigned short *, int);
_so_mb_get_inv_details_mqtt						so_mb_get_inv_details_mqtt;

//发票上传M服务
typedef int(*_so_aisino_upload_m_server)(struct HandlePlate *, unsigned short *, struct _upload_inv_num *, void *, void *,struct _inv_sum_data *);
_so_aisino_upload_m_server						so_aisino_upload_m_server;
typedef int(*_so_nisec_upload_m_server)(struct HandlePlate *, unsigned short *, struct _upload_inv_num *, void *, void *,struct _inv_sum_data *);
_so_nisec_upload_m_server						so_nisec_upload_m_server;
typedef int(*_so_cntax_upload_m_server)(struct HandlePlate *, unsigned short *, struct _upload_inv_num *, void *, void *,struct _inv_sum_data *);
_so_cntax_upload_m_server						so_cntax_upload_m_server;
typedef int(*_so_mb_upload_m_server)(struct HandlePlate *, unsigned short *, struct _upload_inv_num *, void *, void *,struct _inv_sum_data *);
_so_mb_upload_m_server							so_mb_upload_m_server;

//获取月发票总数
typedef int(*_so_aisino_get_month_invoice_num)(struct HandlePlate *, unsigned char *, unsigned int  *, unsigned long *);
_so_aisino_get_month_invoice_num				so_aisino_get_month_invoice_num;
typedef int(*_so_nisec_get_month_invoice_num)(struct HandlePlate *, unsigned char *, unsigned int  *, unsigned long *);
_so_nisec_get_month_invoice_num					so_nisec_get_month_invoice_num;
typedef int(*_so_cntax_get_month_invoice_num)(struct HandlePlate *, unsigned char *, unsigned int  *, unsigned long *);
_so_cntax_get_month_invoice_num					so_cntax_get_month_invoice_num;
typedef int(*_so_mb_get_month_invoice_num)(struct HandlePlate *, unsigned char *, unsigned int  *, unsigned long *);
_so_mb_get_month_invoice_num					so_mb_get_month_invoice_num;

//按发票代码号码获取发票信息
typedef int(*_so_aisino_fpdm_fphm_get_invs)(struct HandlePlate *, char  *, char *, int, char **);
_so_aisino_fpdm_fphm_get_invs					so_aisino_fpdm_fphm_get_invs;
typedef int(*_so_nisec_fpdm_fphm_get_invs)(struct HandlePlate *, char  *, char *, int, char **);
_so_nisec_fpdm_fphm_get_invs					so_nisec_fpdm_fphm_get_invs;
typedef int(*_so_cntax_fpdm_fphm_get_invs)(struct HandlePlate *, char  *, char *, int , char **);
_so_cntax_fpdm_fphm_get_invs					so_cntax_fpdm_fphm_get_invs;

//按发票代码号码获取发票二进制文件信息
typedef int(*_so_aisino_fpdm_fphm_get_invs_kpstr)(struct HandlePlate *, char  *, char *, int, char **, char **, char **);
_so_aisino_fpdm_fphm_get_invs_kpstr				so_aisino_fpdm_fphm_get_invs_kpstr;
typedef int(*_so_nisec_fpdm_fphm_get_invs_kpstr)(struct HandlePlate *, char  *, char *, int, char **, char **, char **);
_so_nisec_fpdm_fphm_get_invs_kpstr				so_nisec_fpdm_fphm_get_invs_kpstr;
typedef int(*_so_cntax_fpdm_fphm_get_invs_kpstr)(struct HandlePlate *, char  *, char *, int, char **, char **, char **);
_so_cntax_fpdm_fphm_get_invs_kpstr				so_cntax_fpdm_fphm_get_invs_kpstr;

//==需要授权未验证口令或口令错误可调用的接口==//
//程序主动验证证书口令
typedef int(*_so_aisino_get_auth_passwd)(struct HandlePlate *, struct _plate_infos *);
_so_aisino_get_auth_passwd						so_aisino_get_auth_passwd;
typedef int(*_so_nisec_get_auth_passwd)(struct HandlePlate *, struct _plate_infos *);
_so_nisec_get_auth_passwd						so_nisec_get_auth_passwd;
typedef int(*_so_cntax_get_auth_passwd)(struct HandlePlate *, struct _plate_infos *);
_so_cntax_get_auth_passwd						so_cntax_get_auth_passwd;
typedef int(*_so_mb_get_auth_passwd)(struct HandlePlate *, struct _plate_infos *);
_so_mb_get_auth_passwd						so_mb_get_auth_passwd;

//接口调用验证默认证书口令是否正确
typedef int(*_so_aisino_verify_cert_passwd)(struct HandlePlate *, char *);
_so_aisino_verify_cert_passwd					so_aisino_verify_cert_passwd;
typedef int(*_so_nisec_verify_cert_passwd)(struct HandlePlate *, char *);
_so_nisec_verify_cert_passwd					so_nisec_verify_cert_passwd;
typedef int(*_so_cntax_verify_cert_passwd)(struct HandlePlate *, char *);
_so_cntax_verify_cert_passwd					so_cntax_verify_cert_passwd;
typedef int(*_so_mb_verify_cert_passwd)(struct HandlePlate *, char *);
_so_mb_verify_cert_passwd						so_mb_verify_cert_passwd;

//修改证书口令
typedef int(*_so_aisino_change_certpassword)(struct HandlePlate *, char *, char *, char *);
_so_aisino_change_certpassword					so_aisino_change_certpassword;
typedef int(*_so_nisec_change_certpassword)(struct HandlePlate *, char *, char *, char *);
_so_nisec_change_certpassword					so_nisec_change_certpassword;
typedef int(*_so_cntax_change_certpassword)(struct HandlePlate *, char *, char *, char *);
_so_cntax_change_certpassword					so_cntax_change_certpassword;
typedef int(*_so_mb_change_certpassword)(struct HandlePlate *, char *, char *, char *);
_so_mb_change_certpassword						so_mb_change_certpassword;

//获取当前发票代码号码
typedef int(*_so_aisino_get_current_invcode)(struct HandlePlate *, unsigned char, char*, char*);
_so_aisino_get_current_invcode					so_aisino_get_current_invcode;
typedef int(*_so_nisec_get_current_invcode)(struct HandlePlate *, unsigned char, char*, char*);
_so_nisec_get_current_invcode					so_nisec_get_current_invcode;
typedef int(*_so_cntax_get_current_invcode)(struct HandlePlate *, unsigned char, char*, char*);
_so_cntax_get_current_invcode					so_cntax_get_current_invcode;
typedef int(*_so_mb_get_current_invcode)(struct HandlePlate *, unsigned char, char*, char*);
_so_mb_get_current_invcode						so_mb_get_current_invcode;


//======需要授权且验证口令正确后方可操作的接口======//
//抄税汇总
typedef int(*_so_aisino_copy_report_data)(struct HandlePlate *, uint8, char *);
_so_aisino_copy_report_data						so_aisino_copy_report_data;
typedef int(*_so_nisec_copy_report_data)(struct HandlePlate *, uint8, char *);
_so_nisec_copy_report_data						so_nisec_copy_report_data;
typedef int(*_so_cntax_copy_report_data)(struct HandlePlate *, uint8, char *);
_so_cntax_copy_report_data						so_cntax_copy_report_data;
typedef int(*_so_mb_copy_report_data)(struct HandlePlate *, uint8, char *);
_so_mb_copy_report_data							so_mb_copy_report_data;

//汇总清卡
typedef int(*_so_aisino_report_summary_write_back)(struct HandlePlate *, uint8, char *);
_so_aisino_report_summary_write_back			so_aisino_report_summary_write_back;
typedef int(*_so_nisec_report_summary_write_back)(struct HandlePlate *, uint8, char *);
_so_nisec_report_summary_write_back				so_nisec_report_summary_write_back;
typedef int(*_so_cntax_report_summary_write_back)(struct HandlePlate *, uint8, char *);
_so_cntax_report_summary_write_back				so_cntax_report_summary_write_back;
typedef int(*_so_mb_report_summary_write_back)(struct HandlePlate *, uint8, char *);
_so_mb_report_summary_write_back				so_mb_report_summary_write_back;

//离线发票上传
typedef int(*_so_aisino_inv_upload_server)(struct HandlePlate *, struct _offline_upload_result *);
_so_aisino_inv_upload_server					so_aisino_inv_upload_server;
typedef int(*_so_nisec_inv_upload_server)(struct HandlePlate *, struct _offline_upload_result *);
_so_nisec_inv_upload_server						so_nisec_inv_upload_server;
typedef int(*_so_cntax_inv_upload_server)(struct HandlePlate *, struct _offline_upload_result *);
_so_cntax_inv_upload_server						so_cntax_inv_upload_server;
typedef int(*_so_mb_inv_upload_server)(struct HandlePlate *, struct _offline_upload_result *);
_so_mb_inv_upload_server						so_mb_inv_upload_server;

//按发票代码号码上传离线发票
typedef int(*_so_aisino_fpdm_fphm_update_invs)(struct HandlePlate *, char  *, char *, int, char **);
_so_aisino_fpdm_fphm_update_invs				so_aisino_fpdm_fphm_update_invs;
typedef int(*_so_nisec_fpdm_fphm_update_invs)(struct HandlePlate *, char  *, char *, int, char **);
_so_nisec_fpdm_fphm_update_invs					so_nisec_fpdm_fphm_update_invs;

//测试税盘与服务器连接
typedef int(*_so_aisino_test_server_connect)(struct HandlePlate *, char **,char *);
_so_aisino_test_server_connect					so_aisino_test_server_connect;
typedef int(*_so_nisec_test_server_connect)(struct HandlePlate *, char **,char *);
_so_nisec_test_server_connect					so_nisec_test_server_connect;
typedef int(*_so_cntax_test_server_connect)(struct HandlePlate *,char **, char *);
_so_cntax_test_server_connect					so_cntax_test_server_connect;
typedef int(*_so_mb_test_server_connect)(struct HandlePlate *, char **,char *);
_so_mb_test_server_connect						so_mb_test_server_connect;

//正数、负数发票开具
typedef int(*_so_aisino_make_invoice)(struct HandlePlate *, unsigned char, char*, struct _plate_infos *, struct _make_invoice_result *);
_so_aisino_make_invoice							so_aisino_make_invoice;
typedef int(*_so_nisec_make_invoice)(struct HandlePlate *, unsigned char, char*, struct _plate_infos *, struct _make_invoice_result *);
_so_nisec_make_invoice							so_nisec_make_invoice;
typedef int(*_so_cntax_make_invoice)(struct HandlePlate *, unsigned char , char* , struct _plate_infos *, struct _make_invoice_result *);
_so_cntax_make_invoice							so_cntax_make_invoice;
typedef int(*_so_mb_make_invoice)(struct HandlePlate *, unsigned char, char*, struct _plate_infos *, struct _make_invoice_result *);
_so_mb_make_invoice								so_mb_make_invoice;

//已开发票作废
typedef int(*_so_aisino_waste_invoice)(struct HandlePlate *, unsigned char, char  *, char *, char *, struct _cancel_invoice_result *);
_so_aisino_waste_invoice						so_aisino_waste_invoice;
typedef int(*_so_nisec_waste_invoice)(struct HandlePlate *, unsigned char, char  *, char *, char *, struct _cancel_invoice_result *);
_so_nisec_waste_invoice							so_nisec_waste_invoice;
typedef int(*_so_cntax_waste_invoice)(struct HandlePlate *, unsigned char, char  *, char *, char *, struct _cancel_invoice_result *);
_so_cntax_waste_invoice							so_cntax_waste_invoice;
typedef int(*_so_mb_waste_invoice)(struct HandlePlate *, unsigned char, char  *, char *, char *, struct _cancel_invoice_result *);
_so_mb_waste_invoice							so_mb_waste_invoice;

//未开发票作废
typedef int(*_so_aisino_waste_null_invoice)(struct HandlePlate *,struct _plate_infos *, struct _cancel_invoice_result *);
_so_aisino_waste_null_invoice						so_aisino_waste_null_invoice;
typedef int(*_so_nisec_waste_null_invoice)(struct HandlePlate *,struct _plate_infos *, struct _cancel_invoice_result *);
_so_nisec_waste_null_invoice						so_nisec_waste_null_invoice;
typedef int(*_so_cntax_waste_null_invoice)(struct HandlePlate *,struct _plate_infos *, struct _cancel_invoice_result *);
_so_cntax_waste_null_invoice						so_cntax_waste_null_invoice;
typedef int(*_so_mb_waste_null_invoice)(struct HandlePlate *,struct _plate_infos *, struct _cancel_invoice_result *);
_so_mb_waste_null_invoice						so_mb_waste_null_invoice;

//检查蓝字发票代码号码是否可以开具红票
typedef int(*_so_aisino_check_lzfpdm_lzfphm_allow)(struct HandlePlate *, unsigned char, char *, char *, uint32, char *);
_so_aisino_check_lzfpdm_lzfphm_allow			so_aisino_check_lzfpdm_lzfphm_allow;

//红字信息表申请
typedef int(*_so_aisino_upload_hzxxb)(struct HandlePlate *, uint8,char*, struct _plate_infos *, struct _askfor_tzdbh_result *);
_so_aisino_upload_hzxxb						so_aisino_upload_hzxxb;
typedef int(*_so_nisec_upload_hzxxb)(struct HandlePlate *, uint8, char*, struct _plate_infos *, struct _askfor_tzdbh_result *);
_so_nisec_upload_hzxxb						so_nisec_upload_hzxxb;
typedef int(*_so_cntax_upload_hzxxb)(struct HandlePlate *, uint8,char*, struct _plate_infos *, struct _askfor_tzdbh_result *);
_so_cntax_upload_hzxxb						so_cntax_upload_hzxxb;

//通过红字信息表编号查询申请信息
typedef int(*_so_aisino_download_hzxxb_from_rednum)(struct HandlePlate *, char *, uint8 **, char *);
_so_aisino_download_hzxxb_from_rednum				so_aisino_download_hzxxb_from_rednum;
typedef int(*_so_nisec_download_hzxxb_from_rednum)(struct HandlePlate *, char *, uint8 **, char *);
_so_nisec_download_hzxxb_from_rednum				so_nisec_download_hzxxb_from_rednum;
typedef int(*_so_cntax_download_hzxxb_from_rednum)(struct HandlePlate *, char *, uint8 **, char *);
_so_cntax_download_hzxxb_from_rednum				so_cntax_download_hzxxb_from_rednum;

//通过时间段查询红字信息表申请信息
typedef int(*_so_aisino_download_hzxxb_from_date_range)(struct HandlePlate *, char *, uint8 **, char *);
_so_aisino_download_hzxxb_from_date_range				so_aisino_download_hzxxb_from_date_range;
typedef int(*_so_aisino_download_hzxxb_from_date_range_new)(struct HandlePlate *, char *, char *,uint8 **, char *);
_so_aisino_download_hzxxb_from_date_range_new				so_aisino_download_hzxxb_from_date_range_new;
typedef int(*_so_nisec_download_hzxxb_from_date_range)(struct HandlePlate *, char *, uint8 **, char *);
_so_nisec_download_hzxxb_from_date_range				so_nisec_download_hzxxb_from_date_range;
typedef int(*_so_cntax_download_hzxxb_from_date_range)(struct HandlePlate *, char *, uint8 **, char *);
_so_cntax_download_hzxxb_from_date_range				so_cntax_download_hzxxb_from_date_range;

//撤销红字信息表申请
typedef int(*_so_aisino_cancel_hzxxb)(struct HandlePlate *, char *, char *);
_so_aisino_cancel_hzxxb							so_aisino_cancel_hzxxb;
typedef int(*_so_nisec_cancel_hzxxb)(struct HandlePlate *, char *, char *);
_so_nisec_cancel_hzxxb							so_nisec_cancel_hzxxb;
typedef int(*_so_cntax_cancel_hzxxb)(struct HandlePlate *, char *, char *);
_so_cntax_cancel_hzxxb							so_cntax_cancel_hzxxb;

////在线发票一键领用//接口停用
//typedef int(*_so_aisino_get_download_net_invoice_coil)(struct HandlePlate *, char *, char *);
//_so_aisino_get_download_net_invoice_coil		so_aisino_get_download_net_invoice_coil;

//在线发票领用查询
typedef int(*_so_aisino_query_net_invoice_coil)(struct HandlePlate *, char *, char *, char *);
_so_aisino_query_net_invoice_coil				so_aisino_query_net_invoice_coil;
typedef int(*_so_nisec_query_net_invoice_coil)(struct HandlePlate *, char *, char *, char *);
_so_nisec_query_net_invoice_coil				so_nisec_query_net_invoice_coil;
typedef int(*_so_cntax_query_net_invoice_coil)(struct HandlePlate *, char *, char *, char *);
_so_cntax_query_net_invoice_coil				so_cntax_query_net_invoice_coil;
//在线发票领用
typedef int(*_so_aisino_net_invoice_coil_download_unlock)(struct HandlePlate *, char *, uint8, char *, char *, int, char *);
_so_aisino_net_invoice_coil_download_unlock		so_aisino_net_invoice_coil_download_unlock;
typedef int(*_so_nisec_net_invoice_coil_download_unlock)(struct HandlePlate *, char *, uint8, char *, char *, int, char *);
_so_nisec_net_invoice_coil_download_unlock		so_nisec_net_invoice_coil_download_unlock;
typedef int(*_so_cntax_net_invoice_coil_download_unlock)(struct HandlePlate *, char *, uint8, char *, char *, int, char *);
_so_cntax_net_invoice_coil_download_unlock		so_cntax_net_invoice_coil_download_unlock;

//申请接入公共服务平台
typedef int(*_so_aisino_connect_pubservice)(struct HandlePlate *, uint8, char *);
_so_aisino_connect_pubservice					so_aisino_connect_pubservice;
typedef int(*_so_nisec_connect_pubservice)(struct HandlePlate *, uint8, char *);
_so_nisec_connect_pubservice					so_nisec_connect_pubservice;

//撤销接入公共服务平台
typedef int(*_so_aisino_disconnect_pubservice)(struct HandlePlate *, uint8, char *);
_so_aisino_disconnect_pubservice				so_aisino_disconnect_pubservice;
typedef int(*_so_nisec_disconnect_pubservice)(struct HandlePlate *, uint8, char *);
_so_nisec_disconnect_pubservice					so_nisec_disconnect_pubservice;

//服务器连接握手--hello
typedef int(*_so_aisino_client_hello)(struct HandlePlate *, char *, char *);
_so_aisino_client_hello				so_aisino_client_hello;
typedef int(*_so_nisec_client_hello)(struct HandlePlate *, char *, char *);
_so_nisec_client_hello				so_nisec_client_hello;
typedef int(*_so_cntax_client_hello)(struct HandlePlate *, char *, char *);
_so_cntax_client_hello				so_cntax_client_hello;

//服务器连接认证--auth
typedef int(*_so_aisino_client_auth)(struct HandlePlate *, char *, char *,char *);
_so_aisino_client_auth				so_aisino_client_auth;
typedef int(*_so_nisec_client_auth)(struct HandlePlate *, char *, char *,char *);
_so_nisec_client_auth				so_nisec_client_auth;
typedef int(*_so_cntax_client_auth)(struct HandlePlate *, char *, char *,char *);
_so_cntax_client_auth				so_cntax_client_auth;

//获取税盘证书
typedef int(*_so_aisino_get_cert)(struct HandlePlate *, char *, char *);
_so_aisino_get_cert				so_aisino_get_cert;
typedef int(*_so_nisec_get_cert)(struct HandlePlate *, char *, char *);
_so_nisec_get_cert				so_nisec_get_cert;
typedef int(*_so_cntax_get_cert)(struct HandlePlate *, char *, char *);
_so_cntax_get_cert				so_cntax_get_cert;

//按月获取税盘汇总信息接口
typedef int(*_so_aisino_query_invoice_month_all_data)(struct HandlePlate *, char *, char **, char *);
_so_aisino_query_invoice_month_all_data				so_aisino_query_invoice_month_all_data;
typedef int(*_so_nisec_query_invoice_month_all_data)(struct HandlePlate *, char *, char **, char *);
_so_nisec_query_invoice_month_all_data				so_nisec_query_invoice_month_all_data;
typedef int(*_so_cntax_query_invoice_month_all_data)(struct HandlePlate *, char *, char **, char *);
_so_cntax_query_invoice_month_all_data				so_cntax_query_invoice_month_all_data;

//读历史票卷信息

typedef int(*_so_nisec_read_inv_coil_number_history)(struct HandlePlate *, char **, char *);
_so_nisec_read_inv_coil_number_history				so_nisec_read_inv_coil_number_history;
typedef int(*_so_cntax_read_inv_coil_number_history)(struct HandlePlate *, char **, char *);
_so_cntax_read_inv_coil_number_history				so_cntax_read_inv_coil_number_history;

//获取蒙柏服务商key加密信息
typedef int(*_so_mb_server_key_info)(struct HandlePlate *, char *);
_so_mb_server_key_info							so_mb_server_key_info;


//全电登陆
typedef int(*_so_etax_login_info)(char *, char *, char *, char *, char *, char *, char *,char *,char *,char *,char *,char *,char *,int,char *,char *);
_so_etax_login_info						so_etax_login_info;


//全电用户信息获取
typedef int(*_so_etax_user_query_info)(char *, char *,char **, char *, char *,int, char *, char *);
_so_etax_user_query_info				so_etax_user_query_info;

//企业列表
typedef int(*_so_etax_relationlist_info)(char *, char *, char *, char *, char *, char *, char *,char **, char *, char *,int, char *, char *);
_so_etax_relationlist_info						so_etax_relationlist_info;

//企业切换
typedef int(*_so_etax_relationchange_info)(char *, char *, char *, char *, char *, char *, char *,char *,char *,char *,char *,char *,char *, char *, char *,int, char *, char *);
_so_etax_relationchange_info						so_etax_relationchange_info;

//全电发票查询
typedef int(*_so_etax_invoice_query_info)(char *, char *, char *, char *, char *, char *, char *,int, char *, char *, char *,char **,char *, char *, char *, char *, char *, char *, char *,int);
_so_etax_invoice_query_info				so_etax_invoice_query_info;

//全电获取cookie
typedef int(*_so_etax_get_cookie_info)(char *,  char *, char *, char *, char *,int, char *, char *, char *,char **,char **,char **,char *,char *,int,char *,char *,char *);
_so_etax_get_cookie_info				so_etax_get_cookie_info;

// typedef int(*_so_etax_get_cookie_info_login)(char *,  char *, char *, char *, char *,int, char *, char *, char *,char **,char **,char **,char *,char *,int,char *,char *,char *);
// _so_etax_get_cookie_info_login				so_etax_get_cookie_info_login;

//全电发票详情
typedef int(*_so_etax_invoice_query_detail_info)(char *, char *, char *, char *, char *, char *, char *,int, char *, char *, char *,char **,char *, char *, char *, char *, char *, char *, char *,int);
_so_etax_invoice_query_detail_info				so_etax_invoice_query_detail_info;

//全电发票下载
typedef int(*_so_etax_invoice_query_down_info)(char *, char *, char *, char *, char *, char *, char *,int, char *, char *, char *,char **,char **,char *, char *, char *, char *, char *, char *, char *,int);
_so_etax_invoice_query_down_info				so_etax_invoice_query_down_info;

//全电rpa扫码接口
typedef int(*_so_etax_rpa_invoice_issuance_info)(char *, char *, char *, char *, char *, char *,char *,char *,int, char *, char *,char **,char *, char *, char *, char *, char *, char *, char *,int);
_so_etax_rpa_invoice_issuance_info				so_etax_rpa_invoice_issuance_info;

//全电身份认证状态接口
typedef int(*_so_etax_rpa_invoice_issuance_status_info)(char *,char *, int ,char *, char *, char *, char *, char *,char *,char *,int, char *, char *,char **,char *, char *, char *, char *, char *, char *, char *,int);
_so_etax_rpa_invoice_issuance_status_info				so_etax_rpa_invoice_issuance_status_info;

//全电授信额度接口
typedef int(*_so_etax_credit_limit_info)(char *, char *, char *, char *, char *, char *,char *,char *, char *, char *,char **,char *, char *, char *, char *, char *, char *, char *,int);
_so_etax_credit_limit_info			so_etax_credit_limit_info;

//全电蓝字发票开具
typedef int(*_so_etax_blue_ink_invoice_issuance_info)(char *, char *, char *, char *, char *, char *,int ,char *, char *, char *,char **,char **,char *, char *, char *, char *, char *, char *, char *,int);
_so_etax_blue_ink_invoice_issuance_info			so_etax_blue_ink_invoice_issuance_info;

//全电可开红票的蓝票查询
typedef int(*_so_etax_hzqrxxSzzhQuery)(char *, char *, char *, char *, char *, char *, char *,int, char *, char *, char *,char **,char *, char *, char *, char *, char *, char *, char *,int);
_so_etax_hzqrxxSzzhQuery				so_etax_hzqrxxSzzhQuery_info;

//全电红票确认单开具
typedef int(*_so_etax_hzqrxxSave)(char *, char *, char *, char *, char *, char *, char *,int, char *, char *, char *,char **,char *, char *, char *, char *, char *, char *, char *,int);
_so_etax_hzqrxxSave				so_etax_hzqrxxSave_info;

//全电红票确认单列表查询
typedef int(*_so_etax_hzqrxxQuery)(char *, char *, char *, char *, char *, char *, char *,int, char *, char *, char *,char **,char *, char *, char *, char *, char *, char *, char *,int);
_so_etax_hzqrxxQuery				so_etax_hzqrxxQuery_info;

//全电红票确认单详情查询
typedef int(*_so_etax_hzqrxxQueryDetail)(char *, char *, char *, char *, char *, char *, char *,int, char *, char *, char *,char **,char *, char *, char *, char *, char *, char *, char *,int);
_so_etax_hzqrxxQueryDetail				so_etax_hzqrxxQueryDetail_info;


typedef int(*_so_etax_hcstaquery_info)(char *, char *, char *, char *, char *, char *,int, char *, char *, char *,char **,char *, char *, char *, char *, char *, char *, char *,int);
_so_etax_hcstaquery_info				so_etax_hcstaquery_info;


typedef int(*_so_etax_fjxxpeizhi_info)(char *, char *, char *, char *,char *, char *, char *,int, char *, char *, char *,char **,char *, char *, char *, char *, char *, char *, char *,int);
_so_etax_fjxxpeizhi_info				so_etax_fjxxpeizhi_info;


typedef int(*_so_etax_cjmbpeizhi_info)(char *, char *, char *, char *,char *, char *, char *,int, char *, char *, char *,char **,char *, char *, char *, char *, char *, char *, char *,int);
_so_etax_cjmbpeizhi_info				so_etax_cjmbpeizhi_info;

typedef int(*_so_etax_get_nsrjcxx_info)(char *, char *, char *, char *,char *,int, char *, char *, char *,char **,char *, char *, char *, char *, int);
_so_etax_get_nsrjcxx_info				so_etax_get_nsrjcxx_info;

typedef int(*_so_etax_get_nsrfxxx_info)(char *, char *, char *, char *,char *,int, char *, char *, char *,char **,char *, char *, char *, char *, int);
_so_etax_get_nsrfxxx_info				so_etax_get_nsrfxxx_info;

#endif 

/*======================================全电函数接口================================================*/

//全电登陆
int mbi_etax_login_Interface(int port,char *busid,char *related_type,char *full_name,char *mobile,char *idcard,char *sz_password,char *errinfo,char *lpgoto,char *lpcode,char *lptoken,char *sys_type,char *svr_container,char *soft_cert_ip,int soft_cert_port,char *login_method,char *area_code);

//全电用户信息获取
int mbi_etax_user_query_Interface(int port,char *busid,char *errinfo,char **outinfo,char *svr_container,char *soft_cert_ip,int soft_cert_port,char *login_method,char *area_code);

//企业列表查询
int mbi_etax_relationlist_Interface(int port,char *busid,char *related_type,char *full_name,char *mobile,char *idcard,char *sz_password,char *errinfo,char **outenterpriselist,char *svr_container,char *soft_cert_ip,int soft_cert_port,char *login_method,char *area_code);

//企业切换
int mbi_etax_relationchange_Interface(int port,char *busid,char *related_type,char *full_name,char *mobile,char *idcard,char *sz_password,char *errinfo,char *uniqueIdentity,char *realationStatus,char *lpgoto,char *lpcode,char *lptoken,char *sys_type,char *svr_container,char *soft_cert_ip,int soft_cert_port,char *login_method,char *area_code);

//全电发票查询
//int mbi_etax_invoice_query_Interface(int port,char *busid,char *fpcxsj,char *related_type,char *full_name,char *mobile,char *idcard,char *sz_password,int realationtype,char *uniqueIdentity,char *realationStatus,char *errinfo,char **fpcxout);
int mbi_etax_invoice_query_Interface(int port,char *busid,char *fpcxsj,char *related_type,char *full_name,char *mobile,char *idcard,char *sz_password,int realationtype,char *uniqueIdentity,char *realationStatus,char *errinfo,char **fpcxout,char *area_code,char *etax_cookie,char *tpass_cookie,char *dppt_cookie,char *new_etax_cookie,char *new_tpass_cookie,char *new_dppt_cookie,int use_cookie);

int mbi_etax_get_cookie_Interface(int port,char *busid,char *related_type,char *mobile,char *idcard,char *sz_password,int realationtype,char *uniqueIdentity,char *realationStatus,char *errinfo,char **etax_cookie,char **tpass_cookie,char **dppt_cookie,char *svr_container,char *soft_cert_ip,int soft_cert_port,char *login_method,char *area_code,char *temp_data);

// int mbi_etax_get_cookie_Interface_login(int port,char *busid,char *related_type,char *mobile,char *idcard,char *sz_password,int realationtype,char *uniqueIdentity,char *realationStatus,char *errinfo,char **etax_cookie,char **tpass_cookie,char **dppt_cookie,char *svr_container,char *soft_cert_ip,int soft_cert_port,char *login_method,char *area_code,char *temp_data);


// int mbi_etax_invoice_query_detail_Interface(int port,char *busid,char *fpcxsj,char *related_type,char *full_name,char *mobile,char *idcard,char *sz_password,int realationtype,char *uniqueIdentity,char *realationStatus,char *errinfo,char **fpcxout);
int mbi_etax_invoice_query_detail_Interface(int port,char *busid,char *fpcxsj,char *related_type,char *full_name,char *mobile,char *idcard,char *sz_password,int realationtype,char *uniqueIdentity,char *realationStatus,char *errinfo,char **fpcxout,char *area_code,char *etax_cookie,char *tpass_cookie,char *dppt_cookie,char *new_etax_cookie,char *new_tpass_cookie,char *new_dppt_cookie,int use_cookie);

// int mbi_etax_invoice_query_down_Interface(int port,char *busid,char *fpcxsj,char *related_type,char *full_name,char *mobile,char *idcard,char *sz_password,int realationtype,char *uniqueIdentity,char *realationStatus,char *errinfo,char **fpcxdata,char **fpcxname);
int mbi_etax_invoice_query_down_Interface(int port,char *busid,char *fpcxsj,char *related_type,char *full_name,char *mobile,char *idcard,char *sz_password,int realationtype,char *uniqueIdentity,char *realationStatus,char *errinfo,char **fpcxdata,char **fpcxname,char *area_code,char *etax_cookie,char *tpass_cookie,char *dppt_cookie,char *new_etax_cookie,char *new_tpass_cookie,char *new_dppt_cookie,int use_cookie);

// int mbi_etax_rpa_invoice_issuance_Interface(int port,char *busid,char *fpkjurl,char *nsrsbh,char *related_type,char *mobile,char *idcard,char *sz_password,int realationtype,char *uniqueIdentity,char *realationStatus,char *errinfo,char **fpkjdata);
int mbi_etax_rpa_invoice_issuance_Interface(int port,char *busid,char *fpkjurl,char *nsrsbh,char *related_type,char *mobile,char *idcard,char *sz_password,int realationtype,char *uniqueIdentity,char *realationStatus,char *errinfo,char **fpkjdata,char *area_code,char *etax_cookie,char *tpass_cookie,char *dppt_cookie,char *new_etax_cookie,char *new_tpass_cookie,char *new_dppt_cookie,int use_cookie);

int mbi_etax_rpa_invoice_issuance_status_Interface(int port,char *busid,char *rzid,char *app_code,char *fpkjurl,char *nsrsbh,char *related_type,char *mobile,char *idcard,char *sz_password,int realationtype,char *uniqueIdentity,char *realationStatus,char *errinfo,char **fpkjdata,char *area_code,char *etax_cookie,char *tpass_cookie,char *dppt_cookie,char *new_etax_cookie,char *new_tpass_cookie,char *new_dppt_cookie,int use_cookie);


// int mbi_etax_credit_limit_Interface(int port,char *busid,char *nsrsbh,char *related_type,char *mobile,char *idcard,char *sz_password,int realationtype,char *uniqueIdentity,char *realationStatus,char *errinfo,char **sxeddata);
int mbi_etax_credit_limit_Interface(int port,char *busid,char *nsrsbh,char *related_type,char *mobile,char *idcard,char *sz_password,int realationtype,char *uniqueIdentity,char *realationStatus,char *errinfo,char **sxeddata,char *area_code,char *etax_cookie,char *tpass_cookie,char *dppt_cookie,char *new_etax_cookie,char *new_tpass_cookie,char *new_dppt_cookie,int use_cookie);

int mbi_etax_blue_ink_invoice_issuance_Interface(int port,char *busid,char *fpkjjson,char *related_type,char *mobile,char *idcard,char *sz_password,int realationtype,char *uniqueIdentity,char *realationStatus,char *errinfo,char **fpkjout,char **hqnsrjcxx,char *area_code,char *etax_cookie,char *tpass_cookie,char *dppt_cookie,char *new_etax_cookie,char *new_tpass_cookie,char *new_dppt_cookie,int use_cookie);

int mbi_etax_hzqrxxSzzhQuery_Interface(int port,char *busid,char *fpcxsj,char *related_type,char *full_name,char *mobile,char *idcard,char *sz_password,int realationtype,char *uniqueIdentity,char *realationStatus,char *errinfo,char **fpcxout,char *area_code,char *etax_cookie,char *tpass_cookie,char *dppt_cookie,char *new_etax_cookie,char *new_tpass_cookie,char *new_dppt_cookie,int use_cookie);

int mbi_etax_hzqrxxSave_Interface(int port,char *busid,char *fpcxsj,char *related_type,char *full_name,char *mobile,char *idcard,char *sz_password,int realationtype,char *uniqueIdentity,char *realationStatus,char *errinfo,char **fpcxout,char *area_code,char *etax_cookie,char *tpass_cookie,char *dppt_cookie,char *new_etax_cookie,char *new_tpass_cookie,char *new_dppt_cookie,int use_cookie);

int mbi_etax_hzqrxxQuery_Interface(int port,char *busid,char *fpcxsj,char *related_type,char *full_name,char *mobile,char *idcard,char *sz_password,int realationtype,char *uniqueIdentity,char *realationStatus,char *errinfo,char **fpcxout,char *area_code,char *etax_cookie,char *tpass_cookie,char *dppt_cookie,char *new_etax_cookie,char *new_tpass_cookie,char *new_dppt_cookie,int use_cookie);

int mbi_etax_hzqrxxQueryDetail_Interface(int port,char *busid,char *fpcxsj,char *related_type,char *full_name,char *mobile,char *idcard,char *sz_password,int realationtype,char *uniqueIdentity,char *realationStatus,char *errinfo,char **fpcxout,char *area_code,char *etax_cookie,char *tpass_cookie,char *dppt_cookie,char *new_etax_cookie,char *new_tpass_cookie,char *new_dppt_cookie,int use_cookie);

int mbi_etax_hcstaquery_Interface(int port,char *busid,char *fpcxsj,char *related_type,char *mobile,char *idcard,char *sz_password,int realationtype,char *uniqueIdentity,char *realationStatus,char *errinfo,char **fpcxout,char *area_code,char *etax_cookie,char *tpass_cookie,char *dppt_cookie,char *new_etax_cookie,char *new_tpass_cookie,char *new_dppt_cookie,int use_cookie);

int mbi_etax_fjxxpeizhi_Interface(int port,char *busid,char *fpcxsj,char *mode,char *related_type,char *mobile,char *idcard,char *sz_password,int realationtype,char *uniqueIdentity,char *realationStatus,char *errinfo,char **fpcxout,char *area_code,char *etax_cookie,char *tpass_cookie,char *dppt_cookie,char *new_etax_cookie,char *new_tpass_cookie,char *new_dppt_cookie,int use_cookie);

int mbi_etax_cjmbpeizhi_Interface(int port,char *busid,char *fpcxsj,char *mode,char *related_type,char *mobile,char *idcard,char *sz_password,int realationtype,char *uniqueIdentity,char *realationStatus,char *errinfo,char **fpcxout,char *area_code,char *etax_cookie,char *tpass_cookie,char *dppt_cookie,char *new_etax_cookie,char *new_tpass_cookie,char *new_dppt_cookie,int use_cookie);
int mbi_etax_get_nsrjcxx_Interface(int port,char *busid,char *related_type,char *mobile,char *idcard,char *sz_password,int realationtype,char *uniqueIdentity,char *realationStatus,char *errinfo,char **fpcxout,char *area_code,char *etax_cookie,char *tpass_cookie,char *dppt_cookie,int use_cookie);
int mbi_etax_get_nsrfxxx_Interface(int port,char *busid,char *related_type,char *mobile,char *idcard,char *sz_password,int realationtype,char *uniqueIdentity,char *realationStatus,char *errinfo,char **fpcxout,char *area_code,char *etax_cookie,char *tpass_cookie,char *dppt_cookie,int use_cookie);

/*======================================函数定义================================================*/
e_ty_usb_m  int ty_usb_m_add(const char *usb_name, int dev_type);
e_ty_usb_m int open_port_attach(int port, uint32 ip, char *client, int *port_fd);
e_ty_usb_m int close_port_dettach(int port);
e_ty_usb_m int get_usb_busid(int port, char*busid);
e_ty_usb_m int change_port_gw_state(struct _new_usbshare_parm* parm, int port);
e_ty_usb_m int change_port_power_quandian(int port,char *inbusid);
e_ty_usb_m int change_all_port_power_mode(int mode);
e_ty_usb_m int get_reg_ble_name(char *ble_name);
e_ty_usb_m int get_module_state(void);
e_ty_usb_m int get_transport_line(struct _new_usbshare_parm *parm);
//***TCP接口****//
//按月获取发票数据JSON传出
e_ty_usb_m int fun_get_plate_usb_info(int port_num, struct _usb_port_infor *port_info);
e_ty_usb_m int set_port_usb_data_tran(int port);
//根据发票代码号码查询发票信息
e_ty_usb_m int fun_get_inv_from_fpdm_fphm(char *plate_num, char *fpdm, char *fphm, char **inv_data, int dzsyh, char *errinfo);
e_ty_usb_m int fun_get_inv_from_fpdm_fphm_get_bin_str(char *plate_num, char *fpdm, char *fphm, int dzsyh, char **inv_data, char **kp_bin, char **kp_str, char *errinfo);
//按月查询查询发票总数
e_ty_usb_m int fun_get_palte_month_inv_num(char *plate_num, char *month, unsigned int *fp_num, unsigned long *fp_len, char *errinfo);
//按月查询发票详细信息
e_ty_usb_m int fun_get_invoice_detail_mqtt(char *plate_num, unsigned char *month, void * function, void * arg, char *errinfo, int old_new, int sfxqqd);
//停止正在查询的发票
e_ty_usb_m int fun_stop_invoice_query(char *plate_num, char *errinfo);
e_ty_usb_m int get_usb_vid_pid(int port, uint16 *vid, uint16 *pid);

e_ty_usb_m int fun_deal_all_port_report(void);

//***MQTT接口****//
//盘号获取USB相关信息

//盘号获取税盘位置
e_ty_usb_m int fun_get_plate_usb_port(unsigned char *plate_num);
//获取税盘当前时间
e_ty_usb_m int fun_get_plate_now_time(unsigned char *plate_num, char *time);
//发票开具
e_ty_usb_m int fun_make_invoice_to_plate(unsigned char *plate_num, int inv_type, unsigned char *inv_data, struct _make_invoice_result *make_invoice_result);
//发票作废
e_ty_usb_m int fun_cancel_invoice_to_plate(unsigned char *plate_num, int inv_type, int zflx, unsigned char *fpdm, unsigned char *fphm, unsigned char *zfr, struct _cancel_invoice_result *cancel_invoice_result);
//设置上报频率
e_ty_usb_m int function_set_report_time(int report_time);
//验证证书密钥是否正确
e_ty_usb_m int fun_check_cert_passwd(unsigned char *plate_num, unsigned char *cert_passwd);
//获取当前发票代码号码
e_ty_usb_m int fun_check_now_fpdm_fphm(unsigned char *plate_num, int inv_type, char *fpdm, char *fphm, char *errinfo);
//执行抄报清卡并返回监控信息数据
e_ty_usb_m int fun_cb_qingka_get_usb_info(unsigned char *plate_num, struct _usb_port_infor *port_info);
//按发票代码号码查询并上传发票
e_ty_usb_m int fun_update_inv_from_fpdm_fphm(char *plate_num, char *fpdm, char *fphm, int dzsyh, char **inv_data, char *errinfo);
//申请红字信息表
e_ty_usb_m int fun_upload_hzxxb(char *plate_num,uint8 int_type, char* inv_data, struct _askfor_tzdbh_result *askfor_tzdbh_result);
//红票开具前对蓝票校验
e_ty_usb_m int fun_check_lzfpdm_lzfphm_allow(char *plate_num, unsigned char inv_type, char *lzfpdm, char *lzfphm, uint32 dzsyh, char *errinfo);
//通过红字信息表查询申请的红票信息
e_ty_usb_m int fun_get_redinv_online_from_rednum(char *plate_num, char *rednum, uint8 **inv_json, char *errinfo);
////发票领用//停用
//e_ty_usb_m int fun_get_download_inv_coils(char *plate_num, char *date_range, char *errinfo);
//通知税盘上传月发票
e_ty_usb_m int fun_notice_plate_update_inv_to_mserver(char *plate_num, char *month, char * summary_month,char *errinfo);
//修改证书口令
e_ty_usb_m int fun_change_cert_passwd(char *plate_num, char *old_passwd, char *new_passwd, char *errinfo);
//撤销红字信息表申请
e_ty_usb_m int fun_cancel_hzxxb_online(char *plate_num, char *rednum, char *errinfo);
//按时间段查询红字信息表
e_ty_usb_m int fun_query_rednum_from_date_range(char *plate_num, char *date_range, uint8 **rednum_data, int *rednum_count, char *errinfo);
e_ty_usb_m int fun_query_rednum_from_date_range_new(char *plate_num, char *date_range,char *gfsh,uint8 **rednum_data, int *rednum_count,char *errinfo);
//查询授权信息
e_ty_usb_m int fun_query_auth_file(char **auth_file);
//发票领用查询
e_ty_usb_m int fun_query_net_inv_coils(char *plate_num, char *date_range, char *inv_data, char *errinfo);
//发票领用下载
e_ty_usb_m int fun_net_inv_coils_download(char *plate_num, char *date_range, uint8 fplxdm, char *fpdm, char *fpqshm, int fpzs, char *errinfo);
//检测服务器连接
e_ty_usb_m int fun_test_plate_server_connect(char *plate_num,char **splxxx, char *errinfo);
//操作USB端口电源
e_ty_usb_m int fun_deal_usb_port_power(int usb_port, int power_state, char *errinfo);
//获取库版本号
e_ty_usb_m int fun_deal_get_so_version(char *tax_ver, char *mbc_ver,char *tax_load, char *auth_load);
//清除证书口令错误缓存
e_ty_usb_m int fun_deal_clear_cert_err(char *plate_num, int mode);
//获取授权密钥
e_ty_usb_m int fun_auth_key_info(char *plate_num, char *key_info, char *errinfo);
//接入公共服务平台
e_ty_usb_m int fun_connect_pubservice(char *plate_num, uint8 inv_type, char *errinfo);
//取消接入公共服务平台
e_ty_usb_m int fun_disconnect_pubservice(char *plate_num, uint8 inv_type, char *errinfo);
//服务器连接握手--hello
e_ty_usb_m int fun_client_hello(char *plate_num, char *client_hello, char *errinfo,int postMessage_flag);
//服务器连接认证--auth
e_ty_usb_m int fun_client_auth(char *plate_num, char* server_hello, char *client_auth, char *errinfo);

//服务器连接握手--hello
e_ty_usb_m int fun_client_hello_d(char *plate_num, char *client_hello, char *errinfo,int postMessage_flag);
//服务器连接认证--auth
e_ty_usb_m int fun_client_auth_d(char *plate_num, char* server_hello, char *client_auth, char *errinfo);

//增值税发票综合服务平台数据透传
e_ty_usb_m int fun_zzsfpzhfwpt_auth_hello_data_tran(char *plate_num, char *https_data, int https_size);
//按月获取税盘汇总信息接口
e_ty_usb_m int fun_query_invoice_month_all_data(char *plate_num, char *month, char **data_json, char *errinfo);
//历史购票信息查询
e_ty_usb_m int fun_query_history_invoice_coils(char *plate_num, char **data_json, char *errinfo);
//获取机柜全部信息json传出
e_ty_usb_m int fun_get_ter_all_data_json(cJSON *data_layer);
//获取机柜除端口外全部信息json传出
e_ty_usb_m int fun_get_ter_all_data_json_without_port(cJSON *data_layer);
//获取机柜除税务信息外全部信息json传出
e_ty_usb_m int fun_get_ter_all_data_json_without_tax(cJSON *data_layer);
//获取机柜基础信息json传出
e_ty_usb_m int fun_get_ter_base_data_json(cJSON *data_layer);
//获取税盘信息json传出
e_ty_usb_m int fun_get_port_data_json(cJSON *data_layer, int port,int is_common);
//切换端口节电模式
e_ty_usb_m int change_port_power_mode(int port, int mode);
//获取mqtt连接MD5信息
e_ty_usb_m int function_get_mqtt_connect_data_md5_jude_update(char *server_addr, int *server_port, char *mqtt_uid, char *mqtt_pwd,int mqtt_type);
e_ty_usb_m int function_get_mqtt_connect_data_md5_jude_update_1201(char *server_addr, int *server_port, char *mqtt_uid, char *mqtt_pwd,int mqtt_type);
//获取mqtt连接证书信息
e_ty_usb_m int function_get_mqtt_connect_data_info_update(char *server_addr, int *server_port, char *mqtt_uid, char *mqtt_pwd, int mqtt_type);
//更新授权文件
e_ty_usb_m int fun_upload_auth_cert_file(void);
//停止或开启发票上传功能
e_ty_usb_m int fun_stop_plate_upload_offline_invoice(char *plate_num,int mode, char *errinfo);

#ifdef _ty_usb_m_c
static int ty_usb_m_open(struct ty_file	*file);
static int ty_usb_m_ctl(void *data, int fd, int cm, va_list args);
static int ty_usb_m_close(void *data, int fd);

static void pack_report_m_server_status_data_linux64(void *arg);

static int jude_port_auth_passwd(struct _port_state   *state,char *errinfo);
static int update_tax_business_info(struct _port_state   *state,char *use_info ,int if_defer_tax,int result ,char *errinfo);
static void task_port_led_manage(void *arg, int timer);
static int update_plate_info_cbqkzt(struct _port_state   *state);
static int close_port_socket(struct _ty_usb_m *stream, int port);
static int open_port(struct _ty_usb_m  *stream, int port, uint32 ip, uint8 *client_id);
static int open_port_ty_usb(struct _ty_usb_m  *stream, int port);
static int close_port_ty_usb(struct _ty_usb_m  *stream, int port);
static int check_stop_upload(int *stop_aisino_up, int *stop_nisec_up, int *stop_cntax_up, int *stop_mengbai_up);
static int timer_read_now_time_add_hour_asc(int hour, uint8 *timer_asc);
static void task_m(void *data, int timer);
static void task_check_port_repair_51err(void *arg, int timer);
static void task_check_port(void *arg, int timer);
static void reset_systerm(void *arg);

static int all_port_power_open(struct _ty_usb_m  *stream);
static int check_port(struct _port_state   *state, struct _ty_usb_m  *stream);
static int check_port_used_time(struct _port_state   *state);
static void deal_plate_upload_m_server_queue(void *arg);


static int check_port_auth_info(struct _ty_usb_m  *stream);

static int common_close_usb_device(struct _port_state   *state,struct HandlePlate *h_plate);
static int find_usb_port_by_plate_num(struct _ty_usb_m  *stream, unsigned char *palte_num);
#ifdef RELEASE_SO
static int load_tax_lib_so(unsigned char *key_file);
static int load_tax_lib_so_fun(void);
static int load_tax_lib_so_fun_wang(void);
#endif
static uint16 crc16_MAXIM(uint8 *addr, int num);

#ifdef PRODUCE_PROGRAM
static void task_read_port_base_tax_info_simple(void *arg, int timer);
#endif

static int get_dev_passwd_err_port(int port, int *left_num);
static int get_cert_passwd_err_port(char *plate_num, int *left_num);
static void update_dev_passwd_err_port(int port, int left_num);
static void update_cert_passwd_err_port(char *plate_num,int left_num);

static int vid_pid_jude_plate(uint16 vid, uint16 pid);

static void timely_tigger_report_status(void *arg, int timer);
static void pack_report_json_data(void *arg);
static void pack_report_m_server_heart_data(void *arg);
static void pack_report_m_server_status_data(void *arg);

static void timely_tigger_report_port_status(void *arg, int timer);


static void pack_request_upload_inv_num(void *arg);
static int analysis_get_inv_num_response(struct _port_state   *state, char *json_data);

static void timely_tigger_deal_plate(void *arg, int timer);
#ifndef DF_OLD_MSERVER
static int pack_report_plate_info_json(struct _port_state   *state, int off_up_flag,char **g_buf);
static int pack_report_other_info_json(struct _port_state   *state, int off_up_flag,char **g_buf);
#ifdef ALL_SHARE
static int pack_report_ca_info_json(struct _port_state   *state, int off_up_flag,char **g_buf);
#endif

static int single_inv_upload(struct _port_state   *state, char *scfpsj, char *kpsj);

static int analysis_ports_report_response(struct _port_state   *state, char *json_data);
static void timely_tigger_get_plate_invoice_db_info(void *arg, int timer);
static int mqtt_pub_hook(void *arg, char *month, char *s_data,int inv_sum);
#else
static int mengbai_plate_inv_upload(struct _port_state   *state, char *scfpsj, char *kpsj);
static int pack_report_plate_info_json(char **g_buf, int off_up_flag, char *random, struct _port_state   *state);
static void pack_report_plate_info(void *arg);
static void pack_report_other_info(void *arg);
static int mqtt_pub_hook(unsigned char *s_data, int total_num, int now_count, void *arg, char *plate_num, unsigned char *month,char *statistics,int complete_err);
#endif

static int judge_plate_allow_kp(struct _port_state   *state, int inv_type, char *inv_data, char *errinfo);

static int inv_change_notice_upload(struct _port_state   *state, int act_type, char *fpdm, char *fphm, char *scfpsj, char *kpsj);

static int make_invoice_ok_update_monitor_status(struct _port_state   *state, int read_flag);
static int make_invoice_ok_update_monitor_status_and_notice_server(struct _port_state   *state, struct _plate_infos *plate_infos);
static void report_offline_inv_to_server(void *arg, int timer);
static void auto_cb_jkxxhc_thread(void *arg, int timer);
static void timely_tigger_auto_cb_jkxxhc(void *arg, int timer);
//static void timely_tigger_sync_plate_cpy_info(void *arg, int timer);
static void get_offline_inv_and_upload_aisino(void *arg);
static void get_offline_inv_and_upload_nisec(void *arg);
static void get_offline_inv_and_upload_cntax(void *arg);
static void get_offline_inv_and_upload_mengbai(void *arg);
static int mengbai_get_cb_data_and_upload(struct _port_state   *state, char *plate_time, long plate_time_l);
static int nisec_get_cb_data_and_upload(struct _port_state   *state, char *plate_time, long plate_time_l);
static int cntax_get_cb_data_and_upload(struct _port_state   *state, char *plate_time, long plate_time_l);
static int aisino_get_cb_data_and_upload(struct _port_state   *state, char *plate_time, long plate_time_l);

static int waiting_for_plate_install(uint8 *plate_num, struct _port_state   *state, int outtimes);
static int waiting_for_plate_uninstall(uint8 *plate_num, struct _port_state   *state, int outtimes);
static int restart_plate_power(struct _port_state   *state, uint8 *plate_num, int port, int wait_install);
static int plate_useinfo_errinfo(struct _port_state   *state, char *errinfo);
#ifndef ALL_SHARE
static int jude_port_allow_attach(struct _port_state  *state);
#endif
static int jude_port_led_auth(struct _port_state  *state);
static int port_lock(struct _port_state   *state, int second, int used_level, char *used_info);
static int port_unlock(struct _port_state   *state);

static void task_read_port_base_tax_info(void *arg, int timer);
static void deal_read_plate_base_tax_info(void *arg);




static void task_check_port_ad_status(void *arg, int timer);
static void task_port_power_up_down(void *arg,int timer);
static void task_port_repair_ad_error(void *arg, int timer);
static void task_check_power_down_dev_remove(void *arg, int timer);
static int free_h_plate(struct _port_state   *state);
static int port_power_up(struct _port_state   *state);
static void port_power_down(struct _port_state   *state);
#endif   


struct _port_used_level_info
{
	int level;
	char info[200];
};

/*usb-share功能类*///一类
#define DF_PORT_USED_USB_SHARE		"usb-share功能"
/*税盘、ca基础信息读取类*///二类
#define DF_PORT_USED_DQMCSHXX		"读取名称税号等基础信息"
/*税盘读写操作类*///三类
#define DF_PORT_USED_FPKJ			"税盘发票开具"
#define DF_PORT_USED_FPZF			"税盘发票作废"
#define DF_PORT_USED_CXDQFPHM		"按发票类型查询当前发票代码号码"
#define DF_PORT_USED_MRKLRZ			"默认口令认证与修改"
#define DF_PORT_USED_HQSPDQSJ		"获取税盘当前时间"
#define DF_PORT_USED_HQFWSJMXX		"获取蒙柏服务商key加密信息"
#define DF_PORT_USED_FWQLJWS		"服务器连接握手"
#define DF_PORT_USED_FWQLJRZ		"服务器连接认证"
#define DF_PORT_USBD_QD_LOCK         "全电接口调用"
/*涉及局端交互接口--耗时*///四类
#define DF_PORT_USED_LXSC			"获取离线发票并上传局端"
#define DF_PORT_USED_HZSC			"抄税并汇总上传"
#define DF_PORT_USED_FXJK			"反写监控信息"
#define DF_PORT_USED_FPHM_SCFP		"按发票代码号码上传发票"
#define DF_PORT_USED_FPYJCXBLJ		"发票一键查询并领用"
#define DF_PORT_USED_DLYFPCX		"待领用发票查询"
#define DF_PORT_USED_DLYFPXZAZ		"待领用发票下载安装"
#define DF_PORT_USED_HPKJLZHMJY		"红票开具前对蓝票校验"
#define DF_PORT_USED_SQHZXXB		"申请红字信息表"
#define DF_PORT_USED_CXHZXXB		"撤销红字信息表"
#define DF_PORT_USED_ASJDCXHZXXB	"按时间段查询红字信息数据"
#define DF_PORT_USED_AXXBBHCXHZXXB	"按信息表编号查询红字信息数据"
#define DF_PORT_USED_CSLJ			"测试与服务器连接"
#define DF_PORT_USED_SQJRGGFWPT		"申请接入公共服务平台"
#define DF_PORT_USED_CXJRGGFWPT		"撤销接入公共服务平台"
/*税盘只读操作类--耗时*///五类
#define DF_PORT_USED_AYCXFPSLMQTT	"按月查询发票数量MQTT接口"
#define DF_PORT_USED_AYCXFPSJMQTT	"按月查询发票数据MQTT传出"
#define DF_PORT_USED_AFPHMCXFP		"按发票代码号码查询发票数据"
#define DF_PORT_USED_CXHZSJ			"查询汇总数据"
/*税盘只读并上传蒙柏类--耗时*///六类
#define DF_PORT_USED_FPCX_SCMB		"税盘发票查询并上传M_SERVER"

static const struct _port_used_level_info used_info[] =
{
	/*usb-share功能类*/
	{ 1, DF_PORT_USED_USB_SHARE },
	/*税盘、ca基础信息读取类*/
	{ 2, DF_PORT_USED_DQMCSHXX },
	/*税盘读写操作类*/
	{ 3, DF_PORT_USED_FPKJ },
	{ 3, DF_PORT_USED_FPZF },
	{ 3, DF_PORT_USED_CXDQFPHM },
	{ 3, DF_PORT_USED_MRKLRZ },
	{ 3, DF_PORT_USED_HQSPDQSJ },
	{ 3, DF_PORT_USED_HQFWSJMXX },
	{ 3, DF_PORT_USED_FWQLJWS },
	{ 3, DF_PORT_USED_FWQLJRZ },
	{ 3, DF_PORT_USBD_QD_LOCK },
	/*涉及局端交互接口*/
	{ 4, DF_PORT_USED_LXSC },
	{ 4, DF_PORT_USED_HZSC },
	{ 4, DF_PORT_USED_FXJK },
	{ 4, DF_PORT_USED_FPHM_SCFP },
	{ 4, DF_PORT_USED_FPYJCXBLJ },
	{ 4, DF_PORT_USED_DLYFPCX },
	{ 4, DF_PORT_USED_DLYFPXZAZ },
	{ 4, DF_PORT_USED_HPKJLZHMJY },
	{ 4, DF_PORT_USED_SQHZXXB },
	{ 4, DF_PORT_USED_CXHZXXB },
	{ 4, DF_PORT_USED_ASJDCXHZXXB },
	{ 4, DF_PORT_USED_AXXBBHCXHZXXB },
	{ 4, DF_PORT_USED_CSLJ },
	{ 4, DF_PORT_USED_SQJRGGFWPT },
	{ 4, DF_PORT_USED_CXJRGGFWPT },
	/*税盘只读操作类*/
	{ 5, DF_PORT_USED_AYCXFPSLMQTT },
	{ 5, DF_PORT_USED_AYCXFPSJMQTT },
	{ 5, DF_PORT_USED_AFPHMCXFP },
	{ 5, DF_PORT_USED_CXHZSJ },
	/*税盘只读并上传蒙柏类*/
	{ 6, DF_PORT_USED_FPCX_SCMB }
};


	struct _plate_function
	{
		char *usb_app;		//税盘类型，1金税盘 2税控盘 3模拟盘  101税务ukey
		char *plate_type;	//税盘类型名称
		char *jcxxdq;		//基础信息读取
		char *jkxxdq;		//监控信息读取
		char *kcxxdq;		//库存信息读取
		char *cxspdqsj;		//查询税盘当前时间
		char *cxdqfpdmhm;	//查询当前发票代码号码
		char *aycxykfpsj;	//按月查询已开发票数据
		char *aycxykfpzs;	//按月查询已开发票总数
		char *afpdmhmcxykfp;//按发票代码号码查询已开发票
		char *yzzskl;		//验证证书口令
		char *xgzskl;		//修改证书口令
		char *zsfpkj;		//正数发票开具
		char *fsfpkj;		//负数发票开具
		char *ykzsfpzf;		//已开正数发票作废
		char *ykfsfpzf;		//已开负数发票作废
		char *wkfpzf;		//未开发票作废
		char *yqsbfpzf;		//验签失败发票作废
		char *asjdcxhzxxb;	//按时间段查询红字信息表
		char *axxbbhcxhzxxb;//按信息表编号查询红字信息表
		char *xfsqhzxxb;	//销方申请红字信息表
		char *gfsqhzxxb;	//购方申请红字信息表
		char *cxhzxxb;		//撤销红字信息表
		char *zdcshz;		//自动抄税汇总
		char *zdqk;			//自动清卡
		char *lxfpzdsc;		//离线发票自动上传
		char *afpdmhmscfp;	//按发票代码号码上传发票
		char *csspyfwqlj;	//测试税盘与服务器连接
		char *dlyfpcx;		//待领用发票查询
		char *dlyfpxzaz;	//待领用发票下载安装
		char *jrggfwpt;		//接入公共服务平台
		char *qxggfwpt;		//取消公共服务平台
	};

	static const struct _plate_function plate_function[] =
	{
		{
			.usb_app = "1",//税盘类型，1金税盘 2税控盘 3模拟盘  101税务ukey
			.plate_type = "金税盘",//税盘类型名称
			.jcxxdq = "1",//基础信息读取
			.jkxxdq = "1",//监控信息读取
			.kcxxdq = "1",//库存信息读取
			.cxspdqsj = "1",//查询税盘当前时间
			.cxdqfpdmhm = "1",//查询当前发票代码号码
			.aycxykfpsj = "1",//按月查询已开发票数据
			.aycxykfpzs = "1",//按月查询已开发票数量
			.afpdmhmcxykfp = "1",//按发票代码号码查询已开发票
			.yzzskl = "1",//验证证书口令
			.xgzskl = "1",//修改证书口令
			.zsfpkj = "1",//正数发票开具
			.fsfpkj = "1",//负数发票开具
			.ykzsfpzf = "1",//已开正数发票作废
			.ykfsfpzf = "1",//已开负数发票作废
			.wkfpzf = "1",//未开发票作废
			.yqsbfpzf = "0",//验签失败发票作废
			.asjdcxhzxxb = "1",//按时间段查询红字信息表
			.axxbbhcxhzxxb = "1",//按信息表编号查询红字信息表
			.xfsqhzxxb = "1",//销方申请红字信息表
			.gfsqhzxxb = "1",//购方申请红字信息表
			.cxhzxxb = "1",//撤销红字信息表
			.zdcshz = "1",//自动抄税汇总
			.zdqk = "1",//自动清卡
			.lxfpzdsc = "1",//离线发票自动上传
			.afpdmhmscfp = "1",//按发票代码号码上传发票
			.csspyfwqlj = "1",//测试税盘与服务器连接
			.dlyfpcx = "1",//待领用发票查询
			.dlyfpxzaz = "1",//待领用发票下载安装
			.jrggfwpt = "1",//接入公共服务平台
			.qxggfwpt = "1",//取消公共服务平台

		},
		{
			.usb_app = "2",//税盘类型，1金税盘 2税控盘 3模拟盘  101税务ukey
			.plate_type = "税控盘",//税盘类型名称
			.jcxxdq = "1",//基础信息读取
			.jkxxdq = "1",//监控信息读取
			.kcxxdq = "1",//库存信息读取
			.cxspdqsj = "1",//查询税盘当前时间
			.cxdqfpdmhm = "1",//查询当前发票代码号码
			.aycxykfpsj = "1",//按月查询已开发票数据
			.aycxykfpzs = "1",//按月查询已开发票数量
			.afpdmhmcxykfp = "1",//按发票代码号码查询已开发票
			.yzzskl = "1",//验证证书口令
			.xgzskl = "1",//修改证书口令
			.zsfpkj = "1",//正数发票开具
			.fsfpkj = "1",//负数发票开具
			.ykzsfpzf = "1",//已开正数发票作废
			.ykfsfpzf = "1",//已开负数发票作废
			.wkfpzf = "1",//未开发票作废
			.yqsbfpzf = "0",//验签失败发票作废
			.asjdcxhzxxb = "1",//按时间段查询红字信息表
			.axxbbhcxhzxxb = "1",//按信息表编号查询红字信息表
			.xfsqhzxxb = "1",//销方申请红字信息表
			.gfsqhzxxb = "1",//购方申请红字信息表
			.cxhzxxb = "1",//撤销红字信息表
			.zdcshz = "1",//自动抄税汇总
			.zdqk = "1",//自动清卡
			.lxfpzdsc = "1",//离线发票自动上传
			.afpdmhmscfp = "1",//按发票代码号码上传发票
			.csspyfwqlj = "1",//测试税盘与服务器连接
			.dlyfpcx = "1",//待领用发票查询
			.dlyfpxzaz = "1",//待领用发票下载安装
			.jrggfwpt = "1",//接入公共服务平台
			.qxggfwpt = "1",//取消公共服务平台
		},
		{
			.usb_app = "3",//税盘类型，1金税盘 2税控盘 3模拟盘  101税务ukey
			.plate_type = "模拟盘",//税盘类型名称
			.jcxxdq = "1",//基础信息读取
			.jkxxdq = "1",//监控信息读取
			.kcxxdq = "1",//库存信息读取
			.cxspdqsj = "1",//查询税盘当前时间
			.cxdqfpdmhm = "1",//查询当前发票代码号码
			.aycxykfpsj = "1",//按月查询已开发票数据
			.aycxykfpzs = "1",//按月查询已开发票数量
			.afpdmhmcxykfp = "0",//按发票代码号码查询已开发票
			.yzzskl = "1",//验证证书口令
			.xgzskl = "1",//修改证书口令
			.zsfpkj = "1",//正数发票开具
			.fsfpkj = "1",//负数发票开具
			.ykzsfpzf = "1",//已开正数发票作废
			.ykfsfpzf = "1",//已开负数发票作废
			.wkfpzf = "1",//未开发票作废
			.yqsbfpzf = "0",//验签失败发票作废
			.asjdcxhzxxb = "0",//按时间段查询红字信息表
			.axxbbhcxhzxxb = "0",//按信息表编号查询红字信息表
			.xfsqhzxxb = "0",//销方申请红字信息表
			.gfsqhzxxb = "0",//购方申请红字信息表
			.cxhzxxb = "0",//撤销红字信息表
			.zdcshz = "1",//自动抄税汇总
			.zdqk = "1",//自动清卡
			.lxfpzdsc = "1",//离线发票自动上传
			.afpdmhmscfp = "0",//按发票代码号码上传发票
			.csspyfwqlj = "1",//测试税盘与服务器连接
			.dlyfpcx = "0",//待领用发票查询
			.dlyfpxzaz = "0",//待领用发票下载安装
			.jrggfwpt = "0",//接入公共服务平台
			.qxggfwpt = "0",//取消公共服务平台
		},
		{
			.usb_app = "101",//税盘类型，1金税盘 2税控盘 3模拟盘  101税务ukey
			.plate_type = "税务UKEY",//税盘类型名称
			.jcxxdq = "1",//基础信息读取
			.jkxxdq = "1",//监控信息读取
			.kcxxdq = "1",//库存信息读取
			.cxspdqsj = "1",//查询税盘当前时间
			.cxdqfpdmhm = "1",//查询当前发票代码号码
			.aycxykfpsj = "1",//按月查询已开发票数据
			.aycxykfpzs = "1",//按月查询已开发票数量
			.afpdmhmcxykfp = "1",//按发票代码号码查询已开发票
			.yzzskl = "1",//验证证书口令
			.xgzskl = "1",//修改证书口令
			.zsfpkj = "1",//正数发票开具
			.fsfpkj = "1",//负数发票开具
			.ykzsfpzf = "1",//已开正数发票作废
			.ykfsfpzf = "1",//已开负数发票作废
			.wkfpzf = "1",//未开发票作废
			.yqsbfpzf = "0",//验签失败发票作废
			.asjdcxhzxxb = "1",//按时间段查询红字信息表
			.axxbbhcxhzxxb = "1",//按信息表编号查询红字信息表
			.xfsqhzxxb = "1",//销方申请红字信息表
			.gfsqhzxxb = "1",//购方申请红字信息表
			.cxhzxxb = "1",//撤销红字信息表
			.zdcshz = "1",//自动抄税汇总
			.zdqk = "1",//自动清卡
			.lxfpzdsc = "1",//离线发票自动上传
			.afpdmhmscfp = "1",//按发票代码号码上传发票
			.csspyfwqlj = "1",//测试税盘与服务器连接
			.dlyfpcx = "1",//待领用发票查询
			.dlyfpxzaz = "1",//待领用发票下载安装
			.jrggfwpt = "1",//接入公共服务平台
			.qxggfwpt = "1",//取消公共服务平台
		}
	};


/*======================================常数定义===============================================*/
#ifdef _ty_usb_m_c
static const struct _ty_usb_m_ctl_fu ctl_fun[] = {
};
static const struct _file_fuc	ty_usb_m_fuc =
{ .open = ty_usb_m_open,
.read = NULL,
.write = NULL,
.ctl = ty_usb_m_ctl,
.close = ty_usb_m_close,
.del = NULL
};


#endif



#endif
