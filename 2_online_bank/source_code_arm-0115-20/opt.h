#ifndef OPT_H
#define OPT_H

#include "ol_bank_include.h"

#define RELEASE_SO

#define DF_TY_PATH_MAX                  256                //路径最大值
#define DF_TY_USB_BUSID                 32                 //BUSID的值

#define DF_TRAN_OPEN

#define DF_MQ_QOS0		0
#define DF_MQ_QOS1		1
#define DF_MQ_QOS2		2
#define DF_TY_TRAN_PORT						3240
#define DF_TY_TRAN_OPEN						1

#define DF_TY_SEV_ORDER_PORT				10001
#define DF_TY_UDP_SER_PORT					10002	
/*===============================程序使用环境========================================*/
#define NEW_USB_SHARE
#define TAX_SPECIAL				//增值税专用

#define MONITOR_CLOSE

/*===============================系统最小时间定义=====================================*/
#define SYSTEM_LEAST_TIME	"2021-05-25 00:00:00"

#define RELEASE					//release版本开启 RELEASE

//====发布版本类型====//单数版本用VER_ALPHA 双数版本用VER_BETA
#define VER_ALPHA

typedef unsigned char  										uint8;      
typedef signed   char  										int8;        
typedef unsigned short 										uint16;      
typedef signed   short 										int16;        	
typedef unsigned int   										uint32; 
typedef unsigned long long   								uint64;
typedef signed   int   										int32;       	
typedef float          										fp32;        
typedef double         										fp64; 
typedef unsigned int 										uint;

typedef signed char sint8;
typedef unsigned char uint8;
typedef short int16;
typedef signed short sint16;
typedef unsigned short uint16;
typedef unsigned int uint32;
typedef signed int sint32;

typedef void* (*Funaction)(void* args);

typedef int (*Fun_get_ter_all_data_json)(cJSON *data_layer);

#define PORT_START_NUM 	1	 //起始端口号
#define MAX_PORT_NUM 	DF_TY_USB_PORT_N   //最大端口数

#ifdef MCHTYPE_2303
#define MAX_ONLINE_NUM 60  //最大在线端口数
#else
#define MAX_ONLINE_NUM 20  //最大在线端口数
#endif

/*---------------------------------原因------------------------------------------*/
#define DF_EVENT_REASON_NO_HEART			1                               //单位时间内没有心跳
#define DF_EVENT_REASON_NET					100                             //网络启动
#define DF_EVENT_REASON_MAIN_HUB			(DF_EVENT_REASON_NET+1)         //主HUB没有发现 
#define DF_EVENT_REASON_SET_MA				(DF_EVENT_REASON_MAIN_HUB+1)    //设置机器编号
#define DF_EVENT_REASON_SOFT				(DF_EVENT_REASON_SET_MA+1)      //软件升级   
#define DF_EVENT_REASON_SET_BUSID			(DF_EVENT_REASON_SOFT+1)        //设置busid
#define DF_EVENT_REASON_FILE_USB			(DF_EVENT_REASON_SET_BUSID+1)   //设备已经关闭,但文件系统还存在   
#define	DF_EVENT_REASON_CLOSE_SOCKET_FAILED	(DF_EVENT_REASON_FILE_USB+1)
#define DF_EVENT_REASON_CHECK_SYS_ERR		(DF_EVENT_REASON_CLOSE_SOCKET_FAILED+1)
#define DF_EVENT_REASON_GET_M3_FLASH_ERROR  (DF_EVENT_REASON_CLOSE_SOCKET_FAILED+1)


#define MAX_MESSAGE_QUEUE_NUM 400	//最大消息处理队列数量
#define MAX_DEAL_MESSAGE_TASK 80	//最处理消息线程数量

#define MAX_HISTROY_DEAL_RESULT 100	//消息历史处理结果
#define DEAFULT_CONNECT_TIME 60		//MQTT默认连间隔时间

/*--------------------------------事件名称----------------------------------------*/
#define DF_EVENT_NAME_RESET             1                               //重新启动
#define DF_EVENT_NAME_POWER_OFF         (DF_EVENT_NAME_RESET+1)         //关闭电源   
#define DF_EVENT_NAME_IMPORT_RECORD		(DF_EVENT_NAME_POWER_OFF+1)		//重要记录

enum RESPONSE_CODE
{
	NO_ERROR = 0,
	PARAMETER_ERROR,
	POWER_LOWER_PERMISSION_ERROR
};

enum MACHINE_TYPE
{
	RESVER = 0,
	TYPE_2303 = 1,
	TYPE_2110,
	TYPE_2108,
	TYPE_2202,
	TYPE_2306,
	TYPE_2212
};

typedef struct
{
	char *name;						//模块名字
	char *r_date;					//发布日期
	char *ver;						//发布版本
	char *v_num;					//顺序迭代版本号
	char *abdr;						//当前版本类型：alpha beta debug release
	char *cpy;						//公司
	char *writer;					//编写人
	char *note;						//备注
	char *type;						//型号
	char *code;						//码制类型
	int machine_type;				//机器类型
}_so_note;

#define CA_READ_ERR_NOT_FOUND_SO		-200

#define MQTT_GW_USBSHARE_OPEN_PORT_ERR_USED		801
#define MQTT_GW_USBSHARE_OPEN_PORT_ERR_NOT_FOUND		802
#define MQTT_GW_USBSHARE_OPEN_PORT_ERR_SPACE_FULL		803
#define MQTT_GW_USBSHARE_CLOSE_PORT_ERR_GET_START				804
#define MQTT_GW_USBSHARE_CLOSE_PORT_ERR_CLOSE				805
#define MQTT_GW_USBSHARE_OPERA_ERR			806
#define MQTT_GW_USBSHARE_CONNECT_TRANS_SERVER_ERR			807
#define MQTT_GW_USBSHARE_ROBOT_ARM_FAILED_WITHOUT_DEV		808
#define MQTT_GW_USBSHARE_ROBOT_ARM_FAILED_NO_SATRT			809
#define MQTT_GW_USBSHARE_PORT_WITHOUT_AOLLOWED			810
#define MQTT_GW_USBSHARE_PORT_ROBOT_ARM_UODATING			811
#define MQTT_GW_USBSHARE_ISNOT_ROBOTARM_TYPE			812
#define MQTT_GW_USBSHARE_ROBOTARM_NOT_WORK			813

	//数据头
#define DF_MQTT_SYNC_PROTOCOL_VER				"mb_fpkjxt_20190520"
#define DF_MQTT_SYNC_CODE_REPORT				"report"
#define DF_MQTT_SYNC_CODE_REQUEST				"request"
#define DF_MQTT_SYNC_CODE_RESPONSE				"response"

	//主动上报命令字
#define DF_MQTT_SYNC_REPORT_TER_STATUS			"sbzdztxx"			//上报终端状态信息

	//主动查询命令字
#define DF_MQTT_SYNC_REQUEST_KPRJ_ADDR			"cxkprjfwdz"		//查询在开票软件服务器地址

	//被动查询命令字
	/*通用接口*/
#define DF_MQTT_SYNC_REQUEST_CXZDQBXX			"cxzdqbxx"			//查询终端全部信息
//#define DF_MQTT_SYNC_REQUEST_QQKQZFLL			"qqkqzfll"			//请求开启转发链路
//#define DF_MQTT_SYNC_REQUEST_QQQXZFLL			"qqqxzfll"			//请求取消转发链路
#define DF_MQTT_SYNC_REQUEST_AYCXYKFPZS			"aycxykfpzs"		//按月查询已开发票总数
#define DF_MQTT_SYNC_REQUEST_AYCXYKFPXX			"aycxykfpxx"		//按月查询已开发票信息
#define DF_MQTT_SYNC_REQUEST_AFPDMHMCXFP		"afpdmhmcxfp"		//按发票代码号码查询发票
#define DF_MQTT_SYNC_REQUEST_AFPDMHMCXFPBS		"afpdmhmcxfpbs"		//按发票代码号码查询发票二进制及开票字符串
#define DF_MQTT_SYNC_REQUEST_TZYKFPCX			"tzykfpcx"			//停止已开发票查询
#define DF_MQTT_SYNC_REQUEST_GET_DQFPDMHM		"hqdqfpdmhm"		//通过发票种类获取当前发票代码号码	
#define DF_MQTT_SYNC_REQUEST_CXZDJCXX			"cxzdjcxx"			//查询终端基础信息
#define DF_MQTT_SYNC_REQUEST_OPEN_PORT			"openport"			//
#define DF_MQTT_SYNC_REQUEST_CLOSE_PORT			"closeport"			//
#define DF_MQTT_SYNC_REQUEST_START_ROBOT_ARM	"start_robot_arm"	//启动一次机械臂
#define DF_MQTT_SYNC_REQUEST_UPDATE_ROBOT_ARM   "update_robot_arm"  //升级机械臂

#define DF_MQTT_SYNC_REQUEST_TZZDSJSQ			"tzhqbbfbsqxx"		//通知终端获取升级或授权文件
#define	DF_MQTT_SYNC_REQUEST_SQZDDK				"sqzddk"			//授权终端端口
#define DF_MQTT_SYNC_REQUEST_CXDLXX				"cxdlxx"			//查询队列中信息
#define DF_MQTT_SYNC_REQUEST_CXLSCLJG			"cxlscljg"			//查询历史处理结果

	/*开票专用接口*/
#define DF_MQTT_SYNC_REQUEST_PLATE_TIME			"cxspdqsj"			//查询税盘当前时间
#define DF_MQTT_SYNC_REQUEST_PLATE_TAX_INFO		"swxxcx"			//税盘税务信息查询
#define DF_MQTT_SYNC_REQUEST_PLATE_INVOICE		"ykfpcx"			//已开发票查询
#define DF_MQTT_SYNC_REQUEST_PLATE_FPKJ			"fpkj"				//发票开具
#define DF_MQTT_SYNC_REQUEST_PLATE_FPZF			"fpzf"				//发票作废
#define DF_MQTT_SYNC_REQUEST_PLATE_CBKQ			"cbjkxxhc"			//抄报监控信息回传
#define DF_MQTT_SYNC_REQUEST_SET_REPORT_TIME	"szzdsbpl"			//设置终端上报频率
#define DF_MQTT_SYNC_REQUEST_CHECK_CERT_PASSWD	"mrzsmmyz"			//税盘默认证书密码验证
#define DF_MQTT_SYNC_REQUEST_GET_LSH_INV_DATA	"cxfpqqlsh"			//通过发票流水号查询已开发票信息
#define DF_MQTT_SYNC_REQUEST_AFPDMHMSCFP		"afpdmhmscfp"		//按发票代码号码上传发票
#define DF_MQTT_SYNC_REQUEST_ZYFP_TZDBH			"hqhzxxbbh"			//获取专票红字发票通知单编号
#define DF_MQTT_SYNC_REQUEST_CHECK_LZFPDMHM		"jylzfpdmhm"		//校验蓝字发票代码号码
#define DF_MQTT_SYNC_REQUEST_GET_INV_REDNUM		"hzxxbhqfp"			//通过红字信息表获取发票
//#define DF_MQTT_SYNC_REQUEST_GET_NET_INV_COIL	"asjdlyfp"			//按时间段领用发票//停用
#define DF_MQTT_SYNC_REQUEST_UPDATE_INV_MSERVER	"tzzdscmyfp"		//通知终端查询并上传某月发票至M服务
#define DF_MQTT_SYNC_REQUEST_CHANGE_CERT_PASSWD	"xgmrzsmmyz"		//修改默认证书口令
#define DF_MQTT_SYNC_REQUEST_CHANCEL_TZDBH		"hzxxbhcx"			//红字信息表撤销
#define DF_MQTT_SYNC_REQUEST_QUERY_TZDBH_DATE	"asjdcxhzxxb"		//按时间段查询红字信息表
#define DF_MQTT_SYNC_REQUEST_QUREY_TER_LOG		"cxzddqrzxx"		//查询终端当前日志信息
#define DF_MQTT_SYNC_REQUEST_OPEN_SSH_LINE		"kqzdywtd"			//开启终端运维通道
#define DF_MQTT_SYNC_REQUEST_CLOSE_SSH_LINE		"jszdywtd"			//结束终端运维通道
#define DF_MQTT_SYNC_REQUEST_GET_AUTH_FILE		"cxzddqsqxx"		//获取中断授权文件
#define DF_MQTT_SYNC_REQUEST_GET_QUERY_INV_COIL	"asjdlyfpcx"		//按时间段查询发票领用
#define DF_MQTT_SYNC_REQUEST_GET_DOWN_INV_COIL	"asjdlyfpxz"		//按时间段下载发票领用
#define DF_MQTT_SYNC_REQUEST_CHECK_SERVER		"jcfwqlj"		    //检测税盘与服务器连接
#define DF_MQTT_SYNC_REQUEST_MANAGE_PORT_POWER	"adkhgldkdy"		//管理端口电源
#define DF_MQTT_SYNC_REQUEST_MANAGE_TER_POWER	"glzddy"			//管理终端电源
#define DF_MQTT_SYNC_REQUEST_CLEAR_CERT_ERR_ROM	"asphqczshc"		//按税盘号清空证书口令错缓存
#define DF_MQTT_SYNC_REQUEST_ZYFP_TZDBH_ALL		"afpdmhmhqhzxxb"	//按发票代码号码申请整票红字信息表
#define DF_MQTT_SYNC_REQUEST_GET_AUTH_KEY		"hqkeysqxx"			//获取服务商开票点授权key密钥信息
#define DF_MQTT_SYNC_REQUEST_CONECT_PUBSERVER	"sqjrggfwpt"		//接入公共服务平台
#define DF_MQTT_SYNC_REQUEST_DISCONECT_PUBSERVER "qxjrggfwpt"		//取消接入公共服务平台
#define DF_MQTT_SYNC_REQUEST_GET_STATISTICS		"hqhzxxsj"			//获取税盘汇总信息数据
#define DF_MQTT_SYNC_REQUEST_GET_HISTORY_COILS	"hqlsgpxx"			//获取历史购票信息
#define DF_MQTT_SYNC_REQUEST_CHANGE_PORT_SAVE	"qhdksdms"			//切换端口省电模式
#define DF_MQTT_SYNC_REQUEST_TER_POWER_MODE_CHANGE	"qhzjsdms"		//切换整机省电模式
#define DF_MQTT_SYNC_REQUEST_TCGXRZFWSJ			"tcgxrzfwsj"		//透传勾选认证服务数据
#define DF_MQTT_SYNC_REQUSET_KZLXFPSC			"kzlxfpsc"			//控制离线发票上传
#define DF_MQTT_SYNC_REQUSET_TZZDDKMQTT			"tzzddkmqtt"		//通知终端断开mqtt

#define DF_MQTT_DEVOPS_REQUEST_SSH_CMD			"sshcmd"			//ssh指令
#define DF_MQTT_DEVOPS_REQUEST_FILE_UPLOAD		"uploadfile"		//上传文件
#define DF_MQTT_DEVOPS_REQUEST_FILE_DOWNLOAD	"downloadfile"		//下载文件


#define ISspace(x) isspace((int)(x))  
#define DF_ORDER_GET_VERSION							"/api/getVersion"
#define DF_ORDER_READ_CERTINFO							"/api/readCertInfo"
#define DF_ORDER_VERIFRY_PIN							"/api/verifyPin"
#define DF_ORDER_CLIENT_HELLO							"/api/clientHello"
#define DF_ORDER_CLIENT_AUTH							"/api/clientAuth"
#define DF_ORDER_CHECK_KEY								"/api/checkKey"
#define DF_ORDER_GET_DEVICE_NUM							"/api/getDeviceNum"
#define DF_ORDER_READ_CERT								"/api/readCert"

#define DF_LINUX_NET_DIR		"/proc/net"
#define DF_LINUX_DNS			"/etc/resolv.conf"
#define DF_LINUX_DNS_TMP		"/tmp/resolv.conf"
#define DF_TER_INFO_FILE		"/etc/ter.info"
#define DF_TER_ID_INI			"/etc/terid.ini"
#define DF_TER_MAC_ADDR_FILE	"/etc/init.d/mac_address"

struct _new_usbshare_parm
{
	char protocol[51];
	char code_type[11];
	char cmd[51];
	char result[2];
	char source_topic[51];
	char random[129];
	char trans_server[100];//转发链路IP
	int trans_port;//转发链路端口号
	int postion;//标记当前内存下标(用于关闭链路使用)
	int state;//本片内存使用标识  0 未使用
	int data_lock;//本结构体数据锁
	int port;		//用于请求转发链路加载端口记录端口号
	uint8 mechine_id[13];
	char client_id[50];//用户唯一标识符 ，用于转发服务器标识匹配唯一链路
	int handle;//记录转发链路句柄
	int type;//转发链路所用用途 1:加载 2:其他
};

struct mqtt_parm
{
	char protocol[51];
	char code_type[11];
	char cmd[51];
	char result[2];
	char source_topic[51];
	char random[512];

};

struct _upload_inv_num
{
	int state;
	int in_used;			//0未使用  1在使用
	char invoice_month[10];	//月份
	char summary_month[10];	//汇总数据上传月份
	char *dmhmzfsbstr;//当月已有发票数据
	int just_single_month;		//仅同步一个月发票
};

typedef struct MODULE_T
{
	int switch_fd;
	int ty_usb_fd;
	int ty_usb_m_fd;
	int ty_pd_fd;
	int deploy_fd;
	int net_time_fd;
	int event_file_fd;
	int machine_fd;
	int serial_fd;
	int get_net_fd;
	int lcd_fd;
	int mq_sync_fd;
	int m_task_fd;
	int vpnfd;
	int opsfd;
	int ble_fd;
	int device_manage_fd;//add by whl
	int task;
	int mqtt_lock;
	int sql_lock;
	int inv_read_lock;
	int data_lock;
	int uart_ok;
	int sock_fd;
	int udp_fd;
	int tran_fd;
	int update_file;
	char mqtt_server[200];
	int mqtt_port;
	char user_name[20];
	char user_passwd[50];
	char topic[200];
	int mqtt_reg_flag;
	int connect_time;
	int mqtt_state;		//0：未登录					1：已经登录
	int boot_update;
	char address[200];
	int port;
	char topic_gw[100];
	char user_name_gw[20];
	char user_passwd_gw[20];
	char user_id_sub_gw[100];
	char user_id_pub_gw[100];
	int log_enable;
	char log_path[200];
	struct mqtt_parm parm[100];
	char ter_id[13];
}MODULE;

#define DF_EVENT_FILE_NAME                  "event"
#define DF_TY_PD_NAME                       "ty_pd"

#define DF_TY_PD_CM_RESET                   100
#define DF_TY_PC_CM_POWER_OFF               (DF_TY_PD_CM_RESET+1)
#define DF_TY_PD_CM_ADD_NAME                (DF_TY_PC_CM_POWER_OFF+1)
#define DF_TY_PD_CM_DEL_NAME                (DF_TY_PD_CM_ADD_NAME+1)

#define DF_TY_M_CM_PORT				100
#define DF_TY_M_CM_ORDER			(DF_TY_M_CM_PORT+1)
#define DF_TY_M_CM_HOOK				(DF_TY_M_CM_ORDER+1)
#define DF_TY_M_CM_READ				(DF_TY_M_CM_HOOK+1)
#define DF_TY_M_CM_WRITE			(DF_TY_M_CM_READ+1)
/*------------------------控制命令参数最大值---------------------------*/
#define DF_TY_MAX_PAR				8

#define DF_TER_VERSION_NUM				"20"				//程序迭代
#define DF_TER_VERSION_TIME				"2024-02-04"	//程序发布时间
#define DF_MB_HARDWARE_A20_A33			1				//A20定义为0，A33定义为1  MTK定义为2
#define DF_TY_1502_HARD_EDIT			"Y3.0.4"

#define DF_TY_USB_PORT_N_MAX 					120+32   //最大端口数
#define DF_TCP_SER_CONNECT_LINE_NUM_MAX         120+32   //最大连接数


//2303机柜
#define DF_TER_VERSION_NAME_MB2303				"4.0.0.3"		//程序版本
#define DF_TY_MACHINE_TYPE_MB2303				11				//mb1601和mb1603多一级HUB定义为1，mb1602少一级HUB定义为0,mb1702为8口设备3,mb1603C端口排列方式不同为4,mb1805为5两口设备,mb1806为6 mb1806a为6 mb_1905=7
#define DF_TY_USB_PORT_N_MB2303               	60				//最大支持60个端口  
#define DF_TCP_SER_CONNECT_LINE_NUM_MB2303		60				//TCP连接链路上限
#define DF_TER_TYPE_NAME_MB2303					"MB_2303"		//型号名称
#define DF_TER_TYPE_EXE_MB2303					"MB2402303CX"	//程序名称

//2110机柜
#define DF_TER_VERSION_NAME_MB2110				"4.0.0.3"		//程序版本
#define DF_TY_MACHINE_TYPE_MB2110				4				//mb1601和mb1603多一级HUB定义为1，mb1602少一级HUB定义为0,mb1702为8口设备3,mb1603C端口排列方式不同为4,mb1805为5两口设备,mb1806为6 mb1806a为6 mb_1905=7
#define DF_TY_USB_PORT_N_MB2110                 100				//最大支持60个端口  
#define DF_TCP_SER_CONNECT_LINE_NUM_MB2110		100				//TCP连接链路上限
#define DF_TER_TYPE_NAME_MB2110					"MB_2110"		//型号名称
#define DF_TER_TYPE_EXE_MB2110					"MB2402110CX"	//程序名称

//2108机柜
#define DF_TER_VERSION_NAME_MB2108				"4.0.0.3"		//程序版本
#define DF_TY_MACHINE_TYPE_MB2108				6				//mb1601和mb1603多一级HUB定义为1，mb1602少一级HUB定义为0,mb1702为8口设备3,mb1603C端口排列方式不同为4,mb1805为5两口设备,mb1806为6 mb1806a为6 mb_1905=7
#define DF_TY_USB_PORT_N_MB2108                 68				//最大支持60个端口  
#define DF_TCP_SER_CONNECT_LINE_NUM_MB2108		68				//TCP连接链路上限
#define DF_TER_TYPE_NAME_MB2108					"MB_2108"		//型号名称
#define DF_TER_TYPE_EXE_MB2108					"MB2402108CX"	//程序名称

//2202机柜
#define DF_TER_VERSION_NAME_MB2202				"4.0.0.3"		//程序版本
#define DF_TY_MACHINE_TYPE_MB2202				8				//mb1601和mb1603多一级HUB定义为1，mb1602少一级HUB定义为0,mb1702为8口设备3,mb1603C端口排列方式不同为4,mb1805为5两口设备,mb1806为6 mb1806a为6 mb_1905=7
#define DF_TY_USB_PORT_N_MB2202                 20				//最大支持60个端口  
#define DF_TCP_SER_CONNECT_LINE_NUM_MB2202		20				//TCP连接链路上限
#define DF_TER_TYPE_NAME_MB2202					"MB_2202"		//型号名称
#define DF_TER_TYPE_EXE_MB2202					"MB2402202CX"	//程序名称

//2306机柜
#define DF_TER_VERSION_NAME_MB2306				"4.0.0.3"		//程序版本
#define DF_TY_MACHINE_TYPE_MB2306				12				//mb1601和mb1603多一级HUB定义为1，mb1602少一级HUB定义为0,mb1702为8口设备3,mb1603C端口排列方式不同为4,mb1805为5两口设备,mb1806为6 mb1806a为6 mb_1905=7
#define DF_TY_USB_PORT_N_MB2306                 60				//最大支持60个端口  
#define DF_TCP_SER_CONNECT_LINE_NUM_MB2306		60				//TCP连接链路上限
#define DF_TER_TYPE_NAME_MB2306					"MB_2306"		//型号名称
#define DF_TER_TYPE_EXE_MB2306					"MB2402306CX"	//程序名称

//2212机柜
#define DF_TER_VERSION_NAME_MB2212				"4.0.0.3"		//程序版本
#define DF_TY_MACHINE_TYPE_MB2212				9				//mb1601和mb1603多一级HUB定义为1，mb1602少一级HUB定义为0,mb1702为8口设备3,mb1603C端口排列方式不同为4,mb1805为5两口设备,mb1806为6 mb1806a为6 mb_1905=7
#define DF_TY_USB_PORT_N_MB2212                 60				//最大支持60个端口  
#define DF_TCP_SER_CONNECT_LINE_NUM_MB2212		60				//TCP连接链路上限
#define DF_TER_TYPE_NAME_MB2212					"MB_2212"		//型号名称
#define DF_TER_TYPE_EXE_MB2212					"MB2402212CX"	//程序名称

#define MAX_MESSAGE_QUEUE_NUM 400	//最大消息处理队列数量
#define MAX_DEAL_MESSAGE_TASK 80	//最处理消息线程数量
	
#define MAX_HISTROY_DEAL_RESULT 100	//消息历史处理结果
#define DEAFULT_CONNECT_TIME 60		//MQTT默认连间隔时间

#define DF_CODE_GB18030						0
#define DF_CODE_UTF8						1

#define DF_CODE_SELF						DF_CODE_GB18030

#endif	
