#ifndef _TYPE_DEFINE_DEFINE
	#define _TYPE_DEFINE_DEFINE
	/*==============================包含的公用文件========================================*/
	#include <string.h>
	#include <stdio.h>
	#include <stdlib.h>	
	#include <unistd.h>
    #include <limits.h>

	#include <mb_clib/mb_typedef.h>
	#include <mb_clib/_algorithm.h>
	#include <mb_clib/_base64.h>
	#include <mb_clib/_code_s.h>
	#include <mb_clib/_confl.h>
	#include <mb_clib/_Date.h>
	#include <mb_clib/_des.h>
	#include <mb_clib/_utf82gbk.h>
	#include <mb_clib/_zlib.h>
	#include <mb_clib/cJSON.h>
	#include <mb_clib/test_malloc.h>
	#include <mb_clib/_folder.h>
	#include <mb_clib/_file_pro.h>
	#include <mb_clib/_t_file_c.h>
	#include <mb_clib/_timer.h>
	#include <mb_clib/_log.h>
	#include <mb_clib/_dec_file.h>
	#include <mb_taxlib/interface_main.h>
	#include <mb_taxlib/common/common.h>
	#include <mb_taxlib/common/define.h>
	#include <mb_taxlib/common/function.h>

	/*================================立即数定义==========================================*/
    #define DF_TY_PATH_MAX                  256                //路径最大值
    #define DF_TY_USB_BUSID                 32                 //BUSID的值
    
	#define DF_TRAN_OPEN

	#define DF_MQ_QOS0		0
	#define DF_MQ_QOS1		1
	#define DF_MQ_QOS2		2
	#define DF_TY_USBIP_PORT					3241
	#define DF_TY_TRAN_PORT						3240
	#define DF_TY_TRAN_OPEN						1

	#define DF_TY_SEV_ORDER_PORT				10001
	#define DF_TY_UDP_SER_PORT					10002	
	/*===============================程序使用环境========================================*/
	#define NEW_USB_SHARE
	#define TAX_SPECIAL				//增值税专用
	//#define GENERAL_SERIES			//通用系列
	
	//#define TEST_KP_MEM				//测试开票内存

	//#define NEED_FP_DATA			//发票数据写文件

	/*===============================程序模式============================================*/
	//#define TRAN

	/*==========================是否开启CPU内存监控=======================================*/
	//#define MONITOR_OEPN
	#define MONITOR_CLOSE
    
	/*===============================系统最小时间定义=====================================*/
	#define SYSTEM_LEAST_TIME	"2021-05-25 00:00:00"
    
	/*===============================编译选项============================================*/
	//====DEBUG开启====//
	//#define DEBUG					//alpha beta版本开启 DEBUG
	
	//====测试升级开启====//
	//#define RELEASE_TEST			//使用内网升级服务器测试

	//====正式版本或升级测试版本开启====//
	#define RELEASE					//release版本开启 RELEASE
	
	//====发布版本类型====//单数版本用VER_ALPHA 双数版本用VER_BETA
	#define VER_ALPHA
	//#define VER_BETA
	//#define VER_GAMMA

	//===特殊机型定义===//

	//#define RELEASE_SO

	/*==============================各传输类型测试输出===================================*/
	//#define CONTROL_OUT
	//#define INTERRUPT_OUT
	//#define BULK_OUT
	
    /*=================================类型定义============================================*/	
#ifndef RELEASE_SO
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
#endif	
	typedef struct
	{	char *name;						//模块名字
		char *r_date;					//发布日期
		char *ver;						//发布版本
		char *v_num;					//顺序迭代版本号
		char *abdr;						//当前版本类型：alpha beta debug release
		char *cpy;						//公司
		char *writer;					//编写人
		char *note;						//备注
		char *type;						//型号
		char *code;						//码制类型
	}_so_note;
	



#define CA_READ_ERR_NOT_FOUND_SO		-200



#ifdef MTK_OPENWRT
#define DF_MQTT_ERR_RED_OFF_GREEN_TWI		999
#define DF_MQTT_ERR_RED_OFF_GREEN_ON		888
#endif

#ifdef  NEW_USB_SHARE
#define MQTT_GW_USBSHARE_OPEN_PORT_ERR_USED		801
#define MQTT_GW_USBSHARE_OPEN_PORT_ERR_NOT_FOUND		802
#define MQTT_GW_USBSHARE_OPEN_PORT_ERR_SPACE_FULL		803
#define MQTT_GW_USBSHARE_CLOSE_PORT_ERR_GET_START				804
#define MQTT_GW_USBSHARE_CLOSE_PORT_ERR_CLOSE				805
#define MQTT_GW_USBSHARE_OPERA_ERR			806
#define MQTT_GW_USBSHARE_CONNECT_TRANS_SERVER_ERR			807
#endif	

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

#define DF_MQTT_SYNC_REQUSET_QDDLXX	             "qddlxx"			//全电登录  	   0304
#define DF_MQTT_SYNC_REQUSET_QDDLLB				 "qyrylb"			//全电用户信息获取	0423
#define DF_MQTT_SYNC_REQUSET_QDFPCX				 "qdfpcx"		    //全电发票查询	    0515

#define DF_MQTT_SYNC_REQUSET_QDFPCXXQ             "qdfpcxxq"		//全电发票详情查询	
#define DF_MQTT_SYNC_REQUSET_QDFPXZ               "qdfpxz"			//全电发票下载	
#define DF_MQTT_SYNC_REQUSET_FPKJRPA              "fpkjrpa"			//解析并处理全电APP扫脸、扫码请求

#define DF_MQTT_SYNC_REQUSET_FPKJRPAZT			  "fpkjrpastate"	//解析并处理全电APP扫脸、扫码请求 【状态查询】

#define DF_MQTT_SYNC_REQUSET_QDQYLB				 "qdqylb"		    //全电企业列表查询	0526
#define DF_MQTT_SYNC_REQUSET_QDQYQH				 "qdqyqh"		    //全电企业切换	    0526

#define DF_MQTT_SYNC_REQUSET_QDSXED              "qdsxed"			//全电授信额度查询

#define DF_MQTT_SYNC_REQUSET_QDLZFPKJ		      "qdfpkj"			//全电蓝字发票开具

//全电红票相关
#define DF_MQTT_SYNC_REQUSET_HZQRXXSZZHQUERY  	 "hzqrxxszzhquery"	 //全电可开红票的蓝票查询
#define DF_MQTT_SYNC_REQUSET_HZQRXXSAVE  	 	 "hzqrxxsave"	 	 //全电红票确认单开具
#define DF_MQTT_SYNC_REQUSET_HZQRXXQUERY  	 	 "hzqrxxquery"	 	 //全电红票确认单列表查询
#define DF_MQTT_SYNC_REQUSET_HZQRXXQUERYDETAIL   "hzqrxxquerydetail" //全电红票确认单详情查询

#define DF_MQTT_SYNC_REQUSET_HCSTAQUERY           "hcstaquery"		//全电发票红字类查询

#define DF_MQTT_SYNC_REQUSET_FJXXPEIZHI			  "fjxxpeizhi"		//附加信息配置(创建、修改、删除)
#define DF_MQTT_SYNC_REQUSET_CJMBPEIZHI			  "cjmbpeizhi"		//场景模版配置(创建、修改、删除)

//剥离后的接口
#define DF_MQTT_SYNC_REQUSET_QDFPCX_BY_COOKIES				 "qdfpcxbycookies"		    //全电发票查询，从cookies
#define DF_MQTT_SYNC_REQUSET_QDFPCXDETAIL_BY_COOKIES		 "qdfpcxdetailbycookies"		    //全电发票详情查询，从cookies

#define DF_MQTT_SYNC_REQUSET_QD_GET_COOKIES		 "qdgetcookies"		    //全电获取cookies

#define DF_MQTT_SYNC_REQUSET_QD_NSRJCXX		 "nsrjcxx"		    //纳税人基础信息
#define DF_MQTT_SYNC_REQUSET_QD_NSRFXXX		 "nsrfxxx"		    //纳税人风险信息


#define DF_MQTT_SYNC_REQUSET_GET_CLIENTHELLO_BY_PLATE	"mqttgetclienthello"	//mqtt获取clienthello
#define DF_MQTT_SYNC_REQUSET_GET_CLIENTAUTH_BY_PLATE	"mqttgetclientauth"		//mqtt获取clientauth
#define DF_MQTT_SYNC_REQUSET_READCERT_BY_PLATE			"mqttReadCert"	//读取证书

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
		char random[129];

	};

//以下需修改
//.0.6.26   //看版本号是否增加
//.0.5.26	//看版本号是否增加
//-6-1-16	//看迭代号是否增加
//-5-26-1 //看版本号与迭代号是否增加
//-6-1-16	//看从哪个版本备份include与lib文件
#define DF_TER_VERSION_NUM				"48"				//程序迭代
#define DF_TER_VERSION_TIME				"2024-01-12"	    //程序发布时间
//以上需修改

#ifndef MTK_OPENWRT
	#define MAX_MESSAGE_QUEUE_NUM 400	//最大消息处理队列数量		
	#define MAX_DEAL_MESSAGE_TASK 80	//最处理消息线程数量		
#else
	#define MAX_MESSAGE_QUEUE_NUM 400	//最大消息处理队列数量		
	#define MAX_DEAL_MESSAGE_TASK 20	//最处理消息线程数量		
#endif		
#define MAX_HISTROY_DEAL_RESULT 100		//消息历史处理结果
#define DEAFULT_CONNECT_TIME 	60		//MQTT默认连间隔时间


//wang 60口定义
#ifdef DF_2306_1
#define DF_TY_USB_PORT_N                60				//最大支持4个端口    
#define DF_TCP_SER_CONNECT_LINE_NUM		60				//TCP连接链路上限
#define DF_TER_TYPE_NAME				"MB_2306"		//型号名称
#define DF_TER_TYPE_EXE					"MB2402306CX"	//程序名称
#define DF_TER_VERSION_NAME				"1.1.6.1"		//程序版本
#define DF_TY_1502_HARD_EDIT			"Y3.0.3"		//mb1602为Y1.0.1 mb1603A和mb1603B位Y1.0.2 mb1603C为3.0.1 mb1602a为3.0.2 mb1805为3.03 mb1806为3.04 mb1905为3.04
#define DF_TY_MACHINE_TYPE				11				//【0】32口机型  【1】60口机型,100口机型(多层板结构) 【3】8口(未量产)  【4】100口机型（一块板结构 【5】2口  【6】68口  【7】4口  【8】20口    【9】120口
#define DF_MB_HARDWARE_A20_A33			3				//A20定义为0，A33定义为1	MTK定义为2  A33带wifi定义为3
#define DF_FULL_LOAD_NUM				20				//20
#define	DF_FULL_POWER_NUM				30				//30
#endif


#ifdef DF_1902_1
#define DF_TY_USB_PORT_N                2				//最大支持4个端口    
#define DF_TCP_SER_CONNECT_LINE_NUM		2				//TCP连接链路上限
#define DF_TER_TYPE_NAME				"MB_1902"		//型号名称
#define DF_TER_TYPE_EXE					"MB2401902CX"	//程序名称
#define DF_TER_VERSION_NAME				"1.1.6.1"		//程序版本
#define DF_TY_1502_HARD_EDIT			"Y3.0.3"		//mb1602为Y1.0.1 mb1603A和mb1603B位Y1.0.2 mb1603C为3.0.1 mb1602a为3.0.2 mb1805为3.03 mb1806 mb1905为3.04
#define DF_TY_MACHINE_TYPE				5				//mb1601和mb1603多一级HUB定义为1，mb1602少一级HUB定义为0,mb1702为8口设备3,mb1603C端口排列方式不同为4,mb1805为5两口设备,mb1806为6 mb1806a为6 mb_1905=7
#define DF_MB_HARDWARE_A20_A33			1				//A20定义为0，A33定义为1  MTK定义为2
#define DF_FULL_LOAD_NUM				2
#define	DF_FULL_POWER_NUM				2
#endif

#ifdef DF_1905_1
#define DF_TY_USB_PORT_N                4				//最大支持4个端口    
#define DF_TCP_SER_CONNECT_LINE_NUM		4				//TCP连接链路上限
#define DF_TER_TYPE_NAME				"MB_1905"		//型号名称
#define DF_TER_TYPE_EXE					"MB2401905CX"	//程序名称
#define DF_TER_VERSION_NAME				"1.1.6.1"		//程序版本
#define DF_TY_1502_HARD_EDIT			"Y3.0.4"		//mb1602为Y1.0.1 mb1603A和mb1603B位Y1.0.2 mb1603C为3.0.1 mb1602a为3.0.2 mb1805为3.03 mb1806 mb1905为3.04
#define DF_TY_MACHINE_TYPE				7				//mb1601和mb1603多一级HUB定义为1，mb1602少一级HUB定义为0,mb1702为8口设备3,mb1603C端口排列方式不同为4,mb1805为5两口设备,mb1806为6 mb1806a为6 mb_1905=7
#define DF_MB_HARDWARE_A20_A33			2				//A20定义为0，A33定义为1  MTK定义为2
#define DF_FULL_LOAD_NUM				4
#define	DF_FULL_POWER_NUM				4
#endif

#ifdef DF_1906_1
#define DF_TY_USB_PORT_N                32				//最大支持32个端口  
#define DF_TCP_SER_CONNECT_LINE_NUM		32				//TCP连接链路上限
#define DF_TER_TYPE_NAME				"MB_1906"		//型号名称
#define DF_TER_TYPE_EXE					"MB2401906CX"	//程序名称
#define DF_TER_VERSION_NAME				"1.1.6.1"		//程序版本
#define DF_TY_1502_HARD_EDIT			"Y3.0.2"		//mb1602为Y1.0.1 mb1603A和mb1603B位Y1.0.2 mb1603C为3.0.1 mb1602a为3.0.2 mb1805为3.03 mb1806为3.04 mb1905为3.04
#define DF_TY_MACHINE_TYPE				0				//【0】32口机型  【1】60口机型,100口机型(多层板结构) 【3】8口(未量产)  【4】100口机型（一块板结构 【5】2口  【6】68口  【7】4口  【8】20口    【9】120口
#define DF_MB_HARDWARE_A20_A33			1				//A20定义为0，A33定义为1	MTK定义为2  A33带wifi定义为3
#define DF_FULL_LOAD_NUM				32
#define	DF_FULL_POWER_NUM				32
#endif

#ifdef DF_1908_1
#define DF_TY_USB_PORT_N                68				//最大支持68个端口  
#define DF_TCP_SER_CONNECT_LINE_NUM		68				//TCP连接链路上限
#define DF_TER_TYPE_NAME				"MB_1908"		//型号名称
#define DF_TER_TYPE_EXE					"MB2401908CX"	//程序名称
#define DF_TER_VERSION_NAME				"1.1.6.1"		//程序版本
#define DF_TY_1502_HARD_EDIT			"Y3.0.4"		//mb1602为Y1.0.1 mb1603A和mb1603B位Y1.0.2 mb1603C为3.0.1 mb1602a为3.0.2 mb1805为3.03 mb1806为3.04 mb1905为3.04
#define DF_TY_MACHINE_TYPE				6				//【0】32口机型  【1】60口机型,100口机型(多层板结构) 【3】8口(未量产)  【4】100口机型（一块板结构 【5】2口  【6】68口  【7】4口  【8】20口    【9】120口
#define DF_MB_HARDWARE_A20_A33			1				//A20定义为0，A33定义为1	MTK定义为2  A33带wifi定义为3
#define DF_FULL_LOAD_NUM				68
#define	DF_FULL_POWER_NUM				68
#endif

#ifdef DF_1910_1
#define DF_TY_USB_PORT_N                100				//最大支持100个端口    
#define DF_TCP_SER_CONNECT_LINE_NUM		100				//TCP连接链路上限
#define DF_TER_TYPE_NAME				"MB_1910"		//型号名称
#define DF_TER_TYPE_EXE					"MB2401910CX"	//程序名称
#define DF_TER_VERSION_NAME				"1.1.6.1"		//程序版本
#define DF_TY_1502_HARD_EDIT			"Y3.0.1"		//mb1602为Y1.0.1 mb1603A和mb1603B位Y1.0.2 mb1603C为3.0.1 mb1602a为3.0.2 mb1805为3.03 mb1806为3.04 mb1905为3.04
#define DF_TY_MACHINE_TYPE				4				//【0】32口机型  【1】60口机型,100口机型(多层板结构) 【3】8口(未量产)  【4】100口机型（一块板结构 【5】2口  【6】68口  【7】4口  【8】20口    【9】120口
#define DF_MB_HARDWARE_A20_A33			1				//A20定义为0，A33定义为1	MTK定义为2  A33带wifi定义为3
#define DF_FULL_LOAD_NUM				100
#define	DF_FULL_POWER_NUM				100
#endif

#ifdef DF_2106_1
#define DF_TY_USB_PORT_N                32				//最大支持32个端口    
#define DF_TCP_SER_CONNECT_LINE_NUM		32				//TCP连接链路上限
#define DF_TER_TYPE_NAME				"MB_2106"		//型号名称
#define DF_TER_TYPE_EXE					"MB2402106CX"	//程序名称
#define DF_TER_VERSION_NAME				"1.1.6.1"		//程序版本
#define DF_TY_1502_HARD_EDIT			"Y3.0.2"		//mb1602为Y1.0.1 mb1603A和mb1603B位Y1.0.2 mb1603C为3.0.1 mb1602a为3.0.2 mb1805为3.03 mb1806为3.04 mb1905为3.04
#define DF_TY_MACHINE_TYPE				0				//【0】32口机型  【1】60口机型,100口机型(多层板结构) 【3】8口(未量产)  【4】100口机型（一块板结构 【5】2口  【6】68口  【7】4口  【8】20口    【9】120口
#define DF_MB_HARDWARE_A20_A33			1				//A20定义为0，A33定义为1	MTK定义为2  A33带wifi定义为3
#define DF_FULL_LOAD_NUM				32
#define	DF_FULL_POWER_NUM				32
#endif

#ifdef DF_2108_1
#define DF_TY_USB_PORT_N                68				//最大支持68个端口  
#define DF_TCP_SER_CONNECT_LINE_NUM		68				//TCP连接链路上限
#define DF_TER_TYPE_NAME				"MB_2108"		//型号名称
#define DF_TER_TYPE_EXE					"MB2402108CX"	//程序名称
#define DF_TER_VERSION_NAME				"1.1.6.1"		//程序版本
#define DF_TY_1502_HARD_EDIT			"Y3.0.4"		//mb1602为Y1.0.1 mb1603A和mb1603B位Y1.0.2 mb1603C为3.0.1 mb1602a为3.0.2 mb1805为3.03 mb1806为3.04 mb1905为3.04
#define DF_TY_MACHINE_TYPE				6				//【0】32口机型  【1】60口机型,100口机型(多层板结构) 【3】8口(未量产)  【4】100口机型（一块板结构 【5】2口  【6】68口  【7】4口  【8】20口    【9】120口
#define DF_MB_HARDWARE_A20_A33			1				//A20定义为0，A33定义为1	MTK定义为2  A33带wifi定义为3
#define DF_FULL_LOAD_NUM				20
#define	DF_FULL_POWER_NUM				30
#endif
	
#ifdef DF_2110_1
#define DF_TY_USB_PORT_N                100				//最大支持100个端口   
#define DF_TCP_SER_CONNECT_LINE_NUM		100				//TCP连接链路上限
#define DF_TER_TYPE_NAME				"MB_2110"		//型号名称
#define DF_TER_TYPE_EXE					"MB2402110CX"	//程序名称
#define DF_TER_VERSION_NAME				"1.1.6.1"		//程序版本
#define DF_TY_1502_HARD_EDIT			"Y3.0.4"		//mb1602为Y1.0.1 mb1603A和mb1603B位Y1.0.2 mb1603C为3.0.1 mb1602a为3.0.2 mb1805为3.03 mb1806为3.04 mb1905为3.04
#define DF_TY_MACHINE_TYPE				4				//【0】32口机型  【1】60口机型,100口机型(多层板结构) 【3】8口(未量产)  【4】100口机型（一块板结构 【5】2口  【6】68口  【7】4口  【8】20口    【9】120口
#define DF_MB_HARDWARE_A20_A33			1				//A20定义为0，A33定义为1	MTK定义为2  A33带wifi定义为3
#define DF_FULL_LOAD_NUM				20
#define	DF_FULL_POWER_NUM				30
#endif

#ifdef DF_2202_1
#define DF_TY_USB_PORT_N                20				//最大支持20个端口    
#define DF_TCP_SER_CONNECT_LINE_NUM		20				//TCP连接链路上限
#define DF_TER_TYPE_NAME				"MB_2202"		//型号名称
#define DF_TER_TYPE_EXE					"MB2402202CX"	//程序名称
#define DF_TER_VERSION_NAME				"1.1.6.1"		//程序版本
#define DF_TY_1502_HARD_EDIT			"Y3.0.4"		//mb1602为Y1.0.1 mb1603A和mb1603B位Y1.0.2 mb1603C为3.0.1 mb1602a为3.0.2 mb1805为3.03 mb1806为3.04 mb1905为3.04
#define DF_TY_MACHINE_TYPE				8				//【0】32口机型  【1】60口机型,100口机型(多层板结构) 【3】8口(未量产)  【4】100口机型（一块板结构 【5】2口  【6】68口  【7】4口  【8】20口    【9】120口
#define DF_MB_HARDWARE_A20_A33			3				//A20定义为0，A33定义为1	MTK定义为2  A33带wifi定义为3
#define DF_FULL_LOAD_NUM				20
#define	DF_FULL_POWER_NUM				20
#endif

#ifdef DF_2204_1
#define DF_TY_USB_PORT_N                4				//最大支持4个端口    
#define DF_TCP_SER_CONNECT_LINE_NUM		4				//TCP连接链路上限
#define DF_TER_TYPE_NAME				"MB_2204"		//型号名称
#define DF_TER_TYPE_EXE					"MB2402204CX"	//程序名称
#define DF_TER_VERSION_NAME				"1.1.6.1"		//程序版本
#define DF_TY_1502_HARD_EDIT			"Y3.0.4"		//mb1602为Y1.0.1 mb1603A和mb1603B位Y1.0.2 mb1603C为3.0.1 mb1602a为3.0.2 mb1805为3.03 mb1806为3.04 mb1905为3.04
#define DF_TY_MACHINE_TYPE				7				//【0】32口机型  【1】60口机型,100口机型(多层板结构) 【3】8口(未量产)  【4】100口机型（一块板结构 【5】2口  【6】68口  【7】4口  【8】20口    【9】120口
#define DF_MB_HARDWARE_A20_A33			3				//A20定义为0，A33定义为1	MTK定义为2  A33带wifi定义为3
#define DF_FULL_LOAD_NUM				4
#define	DF_FULL_POWER_NUM				4
#endif

#ifdef DF_2212_1
#define DF_TY_USB_PORT_N                120				//最大支持4个端口    
#define DF_TCP_SER_CONNECT_LINE_NUM		120				//TCP连接链路上限
#define DF_TER_TYPE_NAME				"MB_2212"		//型号名称
#define DF_TER_TYPE_EXE					"MB2402212CX"	//程序名称
#define DF_TER_VERSION_NAME				"1.1.6.1"		//程序版本
#define DF_TY_1502_HARD_EDIT			"Y3.0.4"		//mb1602为Y1.0.1 mb1603A和mb1603B位Y1.0.2 mb1603C为3.0.1 mb1602a为3.0.2 mb1805为3.03 mb1806为3.04 mb1905为3.04
#define DF_TY_MACHINE_TYPE				9				//【0】32口机型  【1】60口机型,100口机型(多层板结构) 【3】8口(未量产)  【4】100口机型（一块板结构 【5】2口  【6】68口  【7】4口  【8】20口    【9】120口
#define DF_MB_HARDWARE_A20_A33			3				//A20定义为0，A33定义为1	MTK定义为2  A33带wifi定义为3
#define DF_FULL_LOAD_NUM				15
#define	DF_FULL_POWER_NUM				25
#endif

#ifdef DF_2216_1
#define DF_TY_USB_PORT_N                240				//最大支持4个端口    
#define DF_TCP_SER_CONNECT_LINE_NUM		240				//TCP连接链路上限
#define DF_TER_TYPE_NAME				"MB_2216"		//型号名称
#define DF_TER_TYPE_EXE					"MB2402216CX"	//程序名称
#define DF_TER_VERSION_NAME				"1.1.6.1"		//程序版本
#define DF_TY_1502_HARD_EDIT			"Y3.0.4"		//mb1602为Y1.0.1 mb1603A和mb1603B位Y1.0.2 mb1603C为3.0.1 mb1602a为3.0.2 mb1805为3.03 mb1806为3.04 mb1905为3.04
#define DF_TY_MACHINE_TYPE				10				//【0】32口机型  【1】60口机型,100口机型(多层板结构) 【3】8口(未量产)  【4】100口机型（一块板结构 【5】2口  【6】68口  【7】4口  【8】20口    【9】120口
#define DF_MB_HARDWARE_A20_A33			3				//A20定义为0，A33定义为1	MTK定义为2  A33带wifi定义为3
#define DF_FULL_LOAD_NUM				20
#define	DF_FULL_POWER_NUM				30
#endif

#ifdef DF_1201_1
#define DF_TY_USB_PORT_N                4				//最大支持4个端口    
#define DF_TCP_SER_CONNECT_LINE_NUM		4				//TCP连接链路上限
#define DF_TER_TYPE_NAME				"MB_1201"		//型号名称
#define DF_TER_TYPE_EXE					"MB2401201CX"	//程序名称
#define DF_TER_VERSION_NAME				"1.2.0.1"		//程序版本
#define DF_TY_1502_HARD_EDIT			"Y3.0.4"		//mb1602为Y1.0.1 mb1603A和mb1603B位Y1.0.2 mb1603C为3.0.1 mb1602a为3.0.2 mb1805为3.03 mb1806为3.04 mb1905为3.04
#define DF_TY_MACHINE_TYPE				7				//【0】32口机型  【1】60口机型,100口机型(多层板结构) 【3】8口(未量产)  【4】100口机型（一块板结构 【5】2口  【6】68口  【7】4口  【8】20口    【9】120口
#define DF_MB_HARDWARE_A20_A33			3				//A20定义为0，A33定义为1	MTK定义为2  A33带wifi定义为3
#define DF_FULL_LOAD_NUM				4
#define	DF_FULL_POWER_NUM				4
#endif

	//typedef enum {

	//	false_mb, true_mb
	//} bool;

	//typedef enum {

	//	FALSE_MB, TRUE_MB
	//} BOOL;

	/*=================================立即数定义=========================================*/
	
	

	
	#define DF_CODE_GB18030						0
	#define DF_CODE_UTF8						1
	
	#define DF_CODE_SELF						DF_CODE_GB18030


#endif	



























