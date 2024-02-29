#ifndef _USB_PORT_H
#define _USB_PORT_H

#include "opt.h"

#include "power_saving.h"
//#include "module.h"
#include "../../app/inc/module.h"
// #include "define.h"
#include "usb_file.h"
#include "power_saving.h"
#include "get_net_state.h"
#include "ty_usb.h"
#include "robot_arm.h"

#ifdef TEST_MODEL
#include "test.h"
#endif

#define usb_port_out(s,arg...)       				//log_out("all.c",s,##arg)  

#define READ_QUENE_MAX_NUM  50

#define DF_CLIENT_ASC_NUMBER            64      //客户端标识符
#define DF_TY_USB_DATA_MAX_TIMER_S      11/2				//链路连接超时时间
#define DF_TY_USB_MQTT_REPORT_STATUS	15				//MQTT定时上报间隔时间老接口
#define DF_TY_USB_MQTT_M_SERVER_HEART_REPORT	60			//MQTT定时上报间隔时间新接口
#define DF_TY_USB_MQTT_M_SERVER_STATUS_REPORT	3			//MQTT定时第一次状态上报间隔时间新接口
#define DF_TY_USB_MQTT_OFFLINE_UP_TIME   2000				//上传离线票与最后一次开票间隔时间

#define DF_TY_USB_POWER_OFF_TIME 	1
#define DF_TY_USB_CHECK_WITH_DEV 	10
#define DF_TY_USB_CHECK_WITHOUT_DEV 2


#define AUTH_CERT_FILE_PATH		"/etc/auth_cert.json"

typedef int (*Funaction_power)(int port_num,PORT_STATE_ENUM port_state);

//端口电源操作相关参数数据
#pragma pack(4)
typedef struct ACTION_POWER_DATA_T
{
	uint8 power_status;   //电源状态
	int power_off_time;   //下电时间
	int power_control_type;  //1:机械臂方式    0:传统方式
}ACTION_POWER_DATA;
//端口电源结构体
#pragma pack(4)
typedef struct ACTION_POWER_T
{
	Funaction power_on_off;  //电源开关
	Funaction get_power_status;  //获取电源相关参数
	ACTION_POWER_DATA power_data[DF_TY_USB_PORT_N_MAX];  //电源参数数据
	Funaction_power fun_power;

}ACTION_POWER;

enum LOAD_STATE
{
	UNATTACHED = 0,//未加载
	ATTACHED //已加载
		  
};

enum LOAD_NET_TYPE
{
	INTRANET = 0,   //内网
	EXTERNAL_NETWORK,//外网
	OTHER_NETWORK	 //其他
};

//加载操作相关参数数据
#pragma pack(4)
typedef struct LOAD_DATA_T
{
	enum LOAD_STATE usbip_state;    //加载状态
	enum LOAD_NET_TYPE net_type;
	int port; 
	unsigned int ip;
	char oper_id[128];
	char user[128];
	int *port_fd;
}LOAD_DATA;
//加载结构体

#pragma pack(4)
typedef struct ACTION_LOAD_T
{
	Funaction fun_load_start;  //启动一次加载
	Funaction fun_load_stop;  //停止一次加载
	LOAD_DATA load_data[DF_TY_USB_PORT_N_MAX]; //加载参数数据

}ACTION_LOAD;


//信息上报相关参数数据
#pragma pack(4)
typedef struct REPORT_DATA_T
{
	uint8 report_status;    //上报状态
	int report_s_used;//上报终端状态队列是否结束
	int report_h_used;//上报终端心跳队列是否结束
	int report_o_used;//上报终端老接口队列是否结束
	int report_p_used;//上报终端端口信息队列是否结束
	int report_i_used;//上报终端口信息队列是否结束
	int report_ter_flag;//终端信息仅上传一次 0未成功上传需再次上传 1已成功上传无需再次上传
	unsigned long last_report_time_o;//老上报接口最后上报时间
	unsigned long last_report_time_h;//新上报心跳最后上报时间
	unsigned long last_report_time_s;//新上报状态最后上报时间
	unsigned long last_report_time_s_report;//新上报状态最后上报时间
	int report_ter_flag_report;
	uint16 report_counts;
}REPORT_DATA;
//信息上报结构体
#pragma pack(4)
typedef struct ACTION_REPORT_T
{
	Funaction report;  //上报信息
	REPORT_DATA report_data; //上报信息参数数据

}ACTION_REPORT;


//获取设备信息相关参数数据
#pragma pack(4)
typedef struct GET_DEV_INFO_DATA_T
{
	uint8 get_dev_info_status;    //状态

}GET_DEV_INFO_DATA;
//获取设备信息结构体
#pragma pack(4)
typedef struct ACTION_GET_DEV_INFO_T
{
	Funaction get_dev_info;  //获取设备信息
	GET_DEV_INFO_DATA get_dev_info_data; //获取设备信息参数数据

}ACTION_GET_DEV_INFO;

#pragma pack(4)
typedef struct KEY_AUTH_STATE_T
{
	unsigned char key_s[4096];
	char *cert_json;
	char start_time[20];
	char end_time[20];
	int keyfile_load_err;//密钥数据读取失败
	int lib_load_err;//加密库加载失败
	int lib_dec_err;//加密库解密失败
	int auth_dec_err;//授权文件解密失败
	Funaction upload_auth_cert_file;  //更新授权信息
	Funaction get_auth_cert_file;  //获取授权信息
}KEY_AUTH_STATE;

#pragma pack(4)
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

#pragma pack(4)
struct _usb_port_infor
{
	/*USB属性*/
	char arm_version[32];  //机械臂专用，版本号
	int arm_status;
	int arm_with_dev;
	char port_str[200];
	unsigned short port;
	unsigned short port_power;					//端口电源 0关电状态	1开电状态
	unsigned short with_dev;					///有无设备
	char busid[DF_TY_USB_BUSID];
	uint8 subclass;
	uint8 dev_type;
	uint8 protocol_ver;
	uint16 vid;
	uint16 pid;
	uint8 bnuminterface;
	int port_used;					//端口是否使用 1使用中  0未使用
	unsigned short system_used;		//系统使用 1系统使用
	long used_time;					//端口使用时间
	int used_level;					//1usb_share使用  2证书及基本信息读取  3发票开具、作废、汇总反写及上传  4税盘数据查询  5发票数据查询  6发票数据查询并上传
	char used_info[1024];
	unsigned short usb_app; // 0未知	1金税盘	2税控盘	3模拟盘	4 ca / ukey	5 其他
	unsigned short usb_share;//是否被usb-share加载使用0无		1有
	unsigned short usb_share_mode;//usb-share加载使用模式 0内网使用		1公网转发使用			2其他方式
	int check_num;				//检测次数，关闭端口后设备在文件系统中消失，检测次数
	//long port_insert_time;			//端口USB设备插入时间戳
	unsigned short plate_type;
	int tran_data;					//USB有数据读写
	/*USBIP属性*/
	unsigned short port_status;            //状态,usbip中状态  0未使用  2共享中
	uint32 ip;
	long last_open_time;		//最后一次开启使用时间
	int usbip_state;			//端口是否使用  1usbip打开状态   0usbip关闭状态
	int tran_line_closed_flag;
	char client[DF_CLIENT_ASC_NUMBER];      //客户端标识
	char oper_id[128];
	char user[128];
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
	int connect_success_count;
	int connect_failed_count;
	int connect_success_time;
	int connect_failed_time;
	char start_time_s[30];
	char end_time_s[30];
	//char connect_errinfo[1024];
	char tax_business[3000];
	int ad_status;//ad状态
	unsigned short ca_read;		//置1后进入一次CA证书税号读取
	int fail_num;
	unsigned int close_port_flag;
};

#pragma pack(4)
//获取设备基础信息数据（名称，税号）
typedef struct DEV_BASIC_DATA_T
{
	int port;                   //端口号
	int lock;                   //端口锁
	unsigned short power;		//端口电源 0关电状态	1开电状态
	int port_led_status;		//1没有设备且未授权   2设备在本地    3设备被远程使用 4在本地但证书异常 5系统使用中 6远程加载后有数据读写 7税盘默认口令异常 8税盘设备密码错误 9端口没有设备且授权过
	long insert_time;			//USB插入时间
	int usb_err;				//0未出现过错误 1出现过错误
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
	int read_success_flag;  //读取标志
	unsigned short in_read_base_tax;
	long last_without_time;
	struct _port_last_info port_last_info;//上一个USB设备的基本信息
	struct _usb_port_infor port_info;//端口信息
	struct HandlePlate h_plate;
	int sys_reload; //处理某税务ukey加载前需断电重启端口但又不能重新读取名称税号的情况
	int power_mode;// 0 正常模式端口  1可省电模式端口
	int status_counts;
	int fail_num;
	unsigned int now_report_counts; 		//wang 11/10
	unsigned int nongye_report;				//wang 11/18

}DEV_BASIC_DATA;
//获取设备基础信息结构体
#pragma pack(4)
typedef struct ACTION_GET_DEV_BASIC_T
{
	Funaction get_dev_basic;  //获取设备基础信息
	DEV_BASIC_DATA dev_basic_data[DF_TY_USB_PORT_N_MAX]; //设备基础信息
	DEV_BASIC_DATA usb_dev_basic_data[DF_TY_USB_PORT_N_MAX]; //usb设备基础信息,真正的数据存在地方

}ACTION_GET_DEV_BASIC;

//机械臂操作相关参数数据
#pragma pack(4)
typedef struct MECHANICAL_ARM_DATA_T
{
	uint8 arm_status;    //机械臂状态

}MECHANICAL_ARM_DATA;
//机械臂结构体

typedef struct
{
	int arm_with_dev_result;
	int arm_with_dev_fd;
	int port;

}ARM_WITH_DEV;

#pragma pack(4)
typedef struct ACTION_MECHANICAL_ARM_T
{
	Funaction arm_start;  //启动一次机械臂
	Funaction arm_update;  //启动一次机械臂升级
	Funaction get_arm_update_state;  //获取机械臂升级状态
	// Funaction get_arm_status; //获取机械臂状态
	// MECHANICAL_ARM_DATA arm_data; //机械臂参数数据
	ROBOT_ARM_CALSS *class;
	ARM_WITH_DEV arm_with_dev[DF_TY_USB_PORT_N_MAX];
	DEV_BASIC_DATA arm_data[DF_TY_USB_PORT_N_MAX];
	// char arm_with_dev_result[DF_TY_USB_PORT_N_MAX/2];   //机械臂下是否存在设备
	// int arm_with_dev_fd[DF_TY_USB_PORT_N_MAX/2];

}ACTION_MECHANICAL_ARM;

typedef struct
{
	char arm_version[32];  //机械臂版本号
	int arm_status;
	int arm_with_dev;
}REBOT_ARM_DATA_STRUCT;

//U盾操作相关参数数据
#pragma pack(4)
typedef struct U_SHIELD_DATA_T
{
	uint8 u_shield_status;    //U盾状态

}U_SHIELD_DATA;
//U盾结构体
#pragma pack(4)
typedef struct ACTION_U_SHIELD_T
{
	Funaction u_shield_get_data;  //U盾获取数据
	U_SHIELD_DATA u_shield_data; //U盾参数数据

}ACTION_U_SHIELD;
//网银相关结构体
#pragma pack(4)
typedef struct ACTION_ONLINE_BANK_T
{
	ACTION_MECHANICAL_ARM mechanical_arm_action;  //机械臂相关
	ACTION_U_SHIELD u_shield_action;  //U盾相关

}ACTION_ONLINE_BANK;

//端口检查用到的数据参数及动作
#pragma pack(4)
typedef struct PROCESS_JUDGE_USBPORT_EXIST_FUN_T
{
	Funaction fun_start_step;  	 //开始的执行动作
	Funaction fun_recheck_step;  //重新检测的执行动作
	Funaction fun_conform_step;  //确认的执行动作
	Funaction fun_end_step;  	 //结束的执行动作
	Funaction fun_init;			 //设备拔出后操作

}PROCESS_JUDGE_USBPORT_EXIST_FUN;

enum IGNORE_FLAG  //略过检测标志
{
	NO_IGNORE = 0, //不略过
	IGNORE	//略过
};

//确认有无设备流程步骤枚举
enum PROCESS_JUDGE_USBPORT_EXIST_STEPS
{
	WAIT_STEP = 0,
	START_STEP,		//开始步骤，获取端口有无设备，返回数据仅供参考，需要recheck
	RECHECK_STEP,	//重新检测
	CONFROM_STEP, 	//重新确认完成

};

enum USBPORT_EXIST_STATE
{
	WITHOUT_DEV = 0, //无设备
	WITH_DEV,		 //有设备
};

enum USBPORT_ACTION
{
	NO_ACTON = 0, //无动作
	DEV_INSERT,	  //设备插入
	DEV_EXTRACT	  //设备拔出
};

enum USBPORT_RECHECK_RESULT
{
	RECHECK_RESULT_NOT_EXIST = 0, //重新检测不存在
	RECHECK_RESULT_EXIST	  //重新检测存在
};
#pragma pack(4)
typedef struct PROCESS_JUDGE_USBPORT_EXIST_DATA_T
{
	char busid[32];		//用于检测设备存在与否的busid
	enum PROCESS_JUDGE_USBPORT_EXIST_STEPS process_step;   //重新确认步骤
	enum USBPORT_EXIST_STATE now_port_state;			   //当前记录的端口设备存在情况
	enum USBPORT_EXIST_STATE target_port_state;			   //此次检测到的状态，需要去重新确认的状态
	enum USBPORT_ACTION usbport_action;					   //端口的动作
	enum USBPORT_EXIST_STATE recheck_result;			   //重新检测的结果
	int with_dev_recheck_failed_count;					 //相当于检测时的软件滤波
	int without_dev_recheck_failed_count;				 //相当于检测时的软件滤波
	int without_dev_count;
	int without_robot_arm_count;
	enum IGNORE_FLAG ignore_judge_flag;								//略过检测标志
	int start_step_time;

}PROCESS_JUDGE_USBPORT_EXIST_DATA;
#pragma pack(4)
typedef struct PROCESS_JUDGE_USBPORT_EXIST_T
{
	PROCESS_JUDGE_USBPORT_EXIST_FUN judge_exist_fun;
	PROCESS_JUDGE_USBPORT_EXIST_DATA port_exist_state[DF_TY_USB_PORT_N_MAX];

}PROCESS_JUDGE_USBPORT_EXIST;

//usb端口结构体，包含所有操作
#pragma pack(4)
typedef struct GET_PORT_STATUS_T
{
	Funaction fun_get_port_ad_status; //获取端口有无设备（仅做参考值）
	char port_status_buff[DF_TY_USB_PORT_N_MAX];
	char last_port_status_buff[DF_TY_USB_PORT_N_MAX]; //上一次的端口状态
}GET_PORT_STATUS;

//读取税号名称数据参数
enum READ_TAX_NAME_STATE
{
	WAIT_READ = 0,
	NEED_READ,
	READING,
	READ_COMPLATED,
	READ_FAILED
};

#pragma pack(4)
typedef struct READ_TAX_NAME_LIST_T
{
	int port_num;
	enum READ_TAX_NAME_STATE read_state;
	struct READ_TAX_NAME_LIST_T *next;

}READ_TAX_NAME_LIST;

#pragma pack(4)
typedef struct READ_TAX_NAME_T
{
	enum READ_TAX_NAME_STATE read_state[DF_TY_USB_PORT_N_MAX];
	Funaction fun_read_tax_name;
	int need_read_list_lock;	//待读取列表锁
	READ_TAX_NAME_LIST *need_read_tax_name_list_head; //待读取列表
	int reading_list_lock;		//正在读取列表锁
	READ_TAX_NAME_LIST *reading_tax_name_list_head;	  //正在读取列表
	int read_queue_num;
	int ca_read_lock;
	int first_start;
}READ_TAX_NAME;

#pragma pack(4)
typedef struct TASK_FD_T
{
	int get_port_status_fd;
	int usbport_exist_recheck_fd;
	int port_check_fd;
	int read_tax_name_fd;
	int led_manage_fd;
	int report_fd;
	int upgrade_strategy_fd;
	int attach_fd;
	
}TASK_FD;

//升级状态  0：未升级   1：升级中    2：升级成功
typedef enum
{
	UPGRADE_STRATEGY_NO_UPDATE = 0,
	UPGRADE_STRATEGY_UPDATING,
	UPGRADE_STRATEGY_UPDATE_SUCCESS,
	UPGRADE_STRATEGY_UPDATE_FAILED
}UPGRADE_STRATEGY_STATE;

//升级策略结构体
typedef struct 
{
	char old_version[32];	 //现有的机械臂版本号
	UPGRADE_STRATEGY_STATE  update_state;		 

}UPGRADE_STRATEGY_T;

typedef struct
{
	char file_version[32];	 //升级文件版本号
	int  need_update_port_num[DF_TY_USB_PORT_N_MAX]; //需要升级的机械臂号
	UPGRADE_STRATEGY_T upgrade_strategy_struct[DF_TY_USB_PORT_N_MAX];   //升级策略结构
}UPGRADE_STRATEGY;

//usb端口结构体，包含所有操作
#pragma pack(4)
typedef struct USB_PORT_CLASS_T
{
	int data_lock; 			//数据锁，防止多线程的意外操作
	GET_PORT_STATUS get_port_status;
	ACTION_POWER power_action; //端口电源相关
	ACTION_ONLINE_BANK online_bank_action; //机械臂相关
	ACTION_LOAD load_action; //加载相关
	PROCESS_JUDGE_USBPORT_EXIST judge_usbport_exist; //检测端口存在与否
	ACTION_REPORT report_action; //上报相关
	ACTION_GET_DEV_INFO get_dev_info_action; //获取设备信息相关
	ACTION_GET_DEV_BASIC get_dev_basic_action; //获取设备基础信息
	READ_TAX_NAME read_tax_name;
	Fun_get_ter_all_data_json fun_get_ter_all_data_json;              //三个数据打包函数，一 （为什么不直接搞一个，真tm。。）
	Fun_get_ter_all_data_json fun_get_ter_all_data_json_without_port; //三个数据打包函数，二
	Fun_get_ter_all_data_json fun_get_ter_all_data_json_without_tax;  //三个数据打包函数，三
	Funaction fun_check_port_status;  //检查有无端口在使用
	Funaction fun_check_port_with_num;  //检查端口是否正常
	Funaction fun_get_machine_type;
	Funaction fun_check_with_robot_arm;
	MODULE *module; //模块管理指针
	KEY_AUTH_STATE key_auth_state;
	TASK_FD task_fd;
	int timer_fd;
	int usb_port_numb;
	int machine_type;  //机柜型号,1为内置机械臂的
	float intranet_delay_ms;
	float external_delay_ms;
	_so_note *app_note;
	UPGRADE_STRATEGY upgrade_strategy;   //升级策略结构
}USB_PORT_CLASS;

//USB_PORT返回值
enum USB_PORT_RESPONSE_CODE
{
	USB_PORT_PARAMETER_ERROR = -5, //传参错误
	USB_PORT_NULL_FUN,  	  //函数指针为空
	USB_SYS_ERROR,		      //系统层面错误
	USB_PARAMETER_NO_MATCH,	  //找不到匹配的参数
	USB_COMMON_ERROR,		  //一般错误
	USB_PORT_NO_ERROR = 0,
};

int usb_port_init(MODULE *module,_so_note *app_note);
USB_PORT_CLASS *get_usb_port_class(void);

#endif
