#ifndef _TY_1502_DEFINE
    #define _TY_1502_DEFINE
    /*===================================包含文件==============================================*/
    #include "../core/_switch_dev.h"
    #include "../core/_ty_usb.h"
    #include "../core/_ty_usb_m.h"
    #include "../core/_ty_pd.h"
    #include "../core/_deploy.h"
    #include "../core/_event_file.h"
    #include "../core/_machine_infor.h"
    #include "../core/_socket_sev.h"
	#include "../core/_get_net_state.h"
	#include "../core/_get_net_time.h"
	#include "../core/_udp_sev.h"	
	#include "../core/_tran_data.h"
	#include "../core/_vpn_set.h"
	#include "../manage/_process_manage.h"
	#include "../linux_sys/mqtt_client.h"	
#ifndef MTK_OPENWRT
	#include "../linux_sys/_mb_sql.h"
	#include "sqlite3.h"
	#include "../core/_bluetooth.h"
#endif
	#include "../core/_lcd_state.h"
	#include "../linux_sys/_http_reg.h"
	#include "../core/_m_server.h"
	#include "../core/_invoice_transmit.h"
#ifndef RELEASE_SO
	#include "../software/_base64.h"
#endif
#ifdef _app_c
		static const _so_note	app_note =
		{
			.r_date = DF_TER_VERSION_TIME,//版本发布日期
			.ver = DF_TER_VERSION_NAME,//版本号
			.v_num = DF_TER_VERSION_NUM,//顺序迭代版本号
			.abdr = "Beta",
			.cpy = "mengbaiinfo",
			.writer = "mengbai",
			.note = "usb_share",		
			.code = "gbk",
			.type = DF_TER_TYPE_NAME,
			.name = DF_TER_TYPE_EXE
		};
#endif
    /*==================================立即数定义=============================================*/ 


    #ifdef _app_c
		#ifdef DEBUG
			#define out(s,arg...)       				log_out("all.c",s,##arg) 
		#else
			#define out(s,arg...)						log_out("all.c",s,##arg)
		#endif	
        #define e_app

    #else
        #define e_app                       	extern
    #endif
    /*++++++++++++++++++++++++++++++++++++命令定义+++++++++++++++++++++++++++++++++++++++*/
    #ifdef _app_c
	

		#define DF_UARTSET_MAIN_TASK_TIMER        800            	//主线程轮询时间
		#define DF_UDP_SEV_MAIN_TASK_TIMER        300            	//主线程轮询时间
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
    #endif
    /*==================================结构体定义=============================================*/
    #ifdef _app_c
#ifdef NEW_USB_SHARE
		struct err_info{
			int err_code;
			char *err_info;
		};
#endif

        struct _app
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
            //uint16 order_port;
            //uint16 usbip_port;
			//uint16 udpser_port;
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
			//char local_ip[50];
			
			//MYSQL	mysql_fd;
			//int mysql_ok;
			int boot_update;
#ifdef NEW_USB_SHARE
			//config info by mqtt
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
#endif
        };


		void *fd_stream;

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
#ifndef DF_OLD_MSERVER
#else
			char inv_data[4096];
#endif
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

		struct _message_callback message_callback;
		struct _message_history	message_history;
		struct _err_no_infos
		{
			int err;
			int errnum;
			char errinfo[200];
		};


		static const struct _err_no_infos err_no_infos[] =
		{
			//协议数据或终端错误
			{ DF_TAX_ERR_CODE_TER_SYSTEM_ERROR, 301, "Terminal system error." },									//终端系统错误
			{ DF_TAX_ERR_CODE_COMMAND_UNSUPPORTED, 302, "This command word is not supported." },					//命令字不支持
			{ DF_TAX_ERR_CODE_SYSTEM_MEM_INSUFFICIENT, 303, "The terminal memory is insufficient." },				//系统内存不足
			{ DF_TAX_ERR_CODE_DATA_PARSING_FAILED, 304, "The JSON data parsing failure." },							//json解析失败
			{ DF_TAX_ERR_CODE_SYSTEM_TIME_ERROR,312,"The terminal system time error."},								//当前终端系统时间错误
			{ DF_TAX_ERR_CODE_CONNECT_MYSQL_ERROR,325,"Mysql database connection error."},							//数据库连接错误
			//不允许开票错误
			{ DF_TAX_ERR_CODE_EXCEED_KP_END_DATE, 305, "Exceeding the billing deadline." },							//超过开票截止日期
			{ DF_TAX_ERR_CODE_EXCEED_SINGLE_LIMIT, 306, "Exceeding a single invoice billing limit." },				//超过单张发票开票限额
			{ DF_TAX_ERR_CODE_EXCEED_OFFLINE_LIMIT, 307, "Exceeding the cumulative billing limit." },				//超过离线限额
			{ DF_TAX_ERR_CODE_UNSUPPORT_INV_TYPE, 308, "This invoice type invoice is not supported." },				//不支持该发票类型
			{ DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR, 309, "Invoice data parsing error." },							//发票数据解析失败
			{ DF_TAX_ERR_CODE_TAX_RATE_UNSUPPORTED, 310, "Authorized tax rate does not exist." },					//授权税率不支持
			{ DF_TAX_ERR_CODE_HAVE_NO_INVOICE, 311, "Have no invoice available." },									//没有可用发票			
			{ DF_TAX_ERR_CODE_INV_DATA_TOO_LARGE, 313, "Invoice data too large." },									//发票数量过大			
			{ DF_TAX_ERR_CODE_INV_DATA_DISACCORD,315,"Invoice data is inconsistent with plate infomation."},		//开票数据与税盘信息不一致
			{ DF_TAX_ERR_CODE_EXCEED_OFFLINE_LIMIT_TIME,318,"Exceeding offline invoice time."},						//超过离线时长
			{ DF_TAX_ERR_CODE_FPQQLSH_IS_INUSING,326,"The invoice serial number is inusing."},						//当前流水号正在开票中
			//税盘操作通用错误
			{ DF_TAX_ERR_CODE_PLATE_IN_USED,314,"The plate is in used."},											//税盘正在被使用
			{ DF_TAX_ERR_CODE_PLATE_OPER_FAILED,316,"The plate operation failed."},									//税盘操作失败
			{ DF_TAX_ERR_CODE_PLATE_CERT_PASSWD_ERR,317,"The plate cert passwd failed."},							//证书口令错误			
			{ DF_TAX_ERR_CODE_PLATE_NON_EXISTENT,319,"The plate is not on this terminal."},							//税盘不在该终端上
			{ DF_TAX_ERR_CODE_MONTH_HAVE_NO_INVPICE,320,"No invoice for this month's inquiry."},					//当前要查询无发票
			{ DF_TAX_ERR_CODE_USE_ILL_CHAR_OR_UTF8,321,"Illegal character or UTF8 encoding is used."},				//使用了非法字符或utf8编码
			{ DF_TAX_ERR_CODE_CAN_NOT_USE_IOZSV,322,"Illegal characters cannot be used in tax numbers.(Example : I O Z S V)"},//税号中不能使用IOZSV非法字符
			{ DF_TAX_ERR_CODE_INV_SERIAL_NUMBER_UNEXIST, 323, "Invoice serial number does not exist." },				//发票流水号不存在
			{ DF_TAX_ERR_CODE_INV_SERIAL_NUMBER_BEEN_USED, 324, "Invoice serial number has already been issued. " },//该发票流水号已经开具过了
			{ DF_TAX_ERR_CODE_USB_PORT_NOT_AUTH, 327, "This USB port is not authorized." }//该USB端口未授权
		};
//#endif

        struct _ty_err_table
        {   
			uint8 err;
            int err_ty;
        };
        
        struct _app_order
        {   
			int order;
            char *name;  
            int (*answer)(int fd,uint32 ip,struct _app    *par,uint8 *buf,int buf_len);
        };
        
    #endif
    /*==================================函数定义==============================================*/
	e_app   int app_main(void);
	e_app   _so_note    *get_application_version(void);
    #ifdef _app_c

		static int _app_order(int fd,uint32 ip,uint16 port,void *arg);
		void recvSignal(int sig)  ;
		static int set_start_port(int fd,uint32 ip,struct _app    *par,uint8 *buf,int buf_len); 
		static int set_ip_ip(int fd,uint32 ip,struct _app    *par,uint8 *buf,int buf_len);
		static int set_id(int fd,uint32 ip,struct _app    *par,uint8 *buf,int buf_len);
		static int set_timer(int fd,uint32 ip,struct _app    *par,uint8 *buf,int buf_len);
		static int set_par(int fd,uint32 ip,struct _app    *par,uint8 *buf,int buf_len);
		static int set_update_address(int fd,uint32 ip,struct _app    *par,uint8 *buf,int buf_len);
		static int set_tryout_date(int fd,uint32 ip,struct _app    *par,uint8 *buf,int buf_len);
		static int set_vpn_server_par(int fd,uint32 ip,struct _app    *par,uint8 *buf,int buf_len);
		static int set_close_update(int fd,uint32 ip,struct _app    *par,uint8 *buf,int buf_len);
		static int get_sev_pro(int fd,uint32 ip,struct _app  *par,uint8 *buf,int buf_len);
		static int get_sev_status(int fd,uint32 ip,struct _app  *par,uint8 *buf,int buf_len);
		static int get_port_pro(int fd,uint32 ip,struct _app  *par,uint8 *buf,int buf_len);
		static int get_sev_par(int fd,uint32 ip,struct _app  *par,uint8 *buf,int buf_len);
		static int get_check_all_port(int fd,uint32 ip,struct _app  *par,uint8 *buf,int buf_len);
		static int get_all_port_status(int fd,uint32 ip,struct _app  *par,uint8 *buf,int buf_len);
		static int get_all_port_ca_name(int fd,uint32 ip,struct _app  *par,uint8 *buf,int buf_len);
		static int get_all_port_type(int fd,uint32 ip,struct _app  *par,uint8 *buf,int buf_len);
		static int get_ter_soft_version(int fd,uint32 ip,struct _app  *par,uint8 *buf,int buf_len);
		static int get_vpn_server_par(int fd,uint32 ip,struct _app    *par,uint8 *buf,int buf_len);		
		static int get_close_update(int fd,uint32 ip,struct _app    *par,uint8 *buf,int buf_len);
		static int get_month_fp_num(int fd,uint32 ip,struct _app    *par,uint8 *buf,int buf_len);
		static int get_fp_details(int fd,uint32 ip,struct _app    *par,uint8 *buf,int buf_len);
		static int get_fp_details_t(int fd, uint32 ip, struct _app    *par, uint8 *buf, int buf_len);
		static int get_fp_details_s(int fd, uint32 ip, struct _app    *par, uint8 *buf, int buf_len);
		static int get_fp_types(int fd,uint32 ip,struct _app    *par,uint8 *buf,int buf_len);
		static int get_monitor_data(int fd,uint32 ip,struct _app    *par,uint8 *buf,int buf_len);
		static int get_fp_counts_info(int fd,uint32 ip,struct _app    *par,uint8 *buf,int buf_len);
		static int get_area_code(int fd,uint32 ip,struct _app    *par,uint8 *buf,int buf_len);
		static int get_tax_infos(int fd,uint32 ip,struct _app	*par,uint8 *buf,int buf_len);
		static int get_version_describe(int fd,uint32 ip,struct _app	*par,uint8 *buf,int buf_len);
		static int open_port(int fd,uint32 ip,struct _app    *par,uint8 *buf,int buf_len);
		static int close_port(int fd,uint32 ip,struct _app    *par,uint8 *buf,int buf_len);
		static int open_port_id(int fd,uint32 ip,struct _app    *par,uint8 *buf,int buf_len);
		static int close_port_id(int fd,uint32 ip,struct _app    *par,uint8 *buf,int buf_len);
		static int close_port_now(int fd,uint32 ip,struct _app    *par,uint8 *buf,int buf_len);
		static int power_off(int fd,uint32 ip,struct _app  *par,uint8 *buf,int buf_len);
		static int heart(int fd,uint32 ip,struct _app    *par,uint8 *buf,int buf_len);
		static int reset(int fd,uint32 ip,struct _app    *par,uint8 *buf,int buf_len);
		static int regdit(int fd,uint32 ip,struct _app  *par,uint8 *buf,int buf_len);
		static int open_port_no_active(int fd,uint32 ip,struct _app    *par,uint8 *buf,int buf_len);
		static int get_log(int fd,uint32 ip,struct _app *par,uint8 *buf,int buf_len);
		static int get_net_par(int fd,uint32 ip,struct _app *par,uint8 *buf,int buf_len);
		static int keep_open_port(int fd,uint32 ip,struct _app *par,uint8 *buf,int buf_len);
		static int check_version_updata(int fd,uint32 ip,struct _app *par,uint8 *buf,int buf_len);
		static int close_port_by_dettach(int fd,uint32 ip,struct _app *par,uint8 *buf,int buf_len);
		static int get_usb_dev_info(int fd,uint32 ip,struct _app *par,uint8 *buf,int buf_len);
		static int get_update_address(int fd,uint32 ip,struct _app *par,uint8 *buf,int buf_len);
        
		static int answer_no_order(int fd,struct _app    *par,uint8 *buf,int buf_len);
		static int err_out_tcp(int fd,int err);
		static int err_out_s(int fd,uint8 err,uint8 *name);
		static int write_ty1502_order(int fd,int order,uint8 *buf,int buf_len);
		static int unpack(int fd,uint8 *out_buf,int *out_len);
		static int ty_socket_write(int sock,uint8 *buf,int buf_len);
		static int ty_socket_read(int sock,uint8 *buf,int buf_len);

		void update_task_hook(void *arg, int timer);
		//////////////////////UDP////////////////////////////
		static void udp_ser_main_task(void *arg,int o_timer);
		static int udp_unpack(uint8 *in_buf,int in_len,uint8 *out_buf,int *out_len);
		static int udp_get_sev_pro(int fd,struct _app  *par,uint8 *buf,int buf_len);
		static int udp_write_order(int fd,int order,uint8 *buf,int buf_len);
		static int udp_order_sev_restart(int fd,struct _app  *par,uint8 *buf,int buf_len);
		static int Key_Check_Http(struct http_parm * parm);
		static int check_port_status(struct _app  *par);
		static int udp_unpack_new_json(uint8 *in_buf, int in_len);
		static int udp_get_sev_pro_new(int fd, struct _app  *par);
		//////////////////////MQTT//////////////////////////
#ifdef TEST_KP_MEM
		void task_kp(void *arg, int o_timer);
#endif
		void task_deal_message_queue(void *arg, int o_timer);
		static int _mqtt_client_message_pub(char *topic, char *sbuf, int try_num,int qos);
		static int find_available_queue_mem(struct _app     *stream);
		static int find_deal_queue_mem(struct _app     *stream);
		
		static void sub_message_callback_function_sync(struct mosquitto *mosq, void *obj, const struct mosquitto_message *message);
		static void sub_err_callback_deal_function(struct _mqtt_parms  *mqtt_client, int errnum);
		static int analysis_json_head(char *inbuf, struct mqtt_parm *parm);
		static int mqtt_response_errno(int errnum, struct mqtt_parm *parm, char *errinfo,int qos);
		static void deal_with_cmd(struct _app *stream, int mem_i, void *fucntion);

		//解析并处理查询队列信息
		static void analysis_request_cxdlxx(void *arg);
		//解析并处理查询历史处理结果
		static void analysis_request_cxlscljg(void *arg);
		//解析并处理命令字异常请求
		static void analysis_request_cmd_not_support(void *arg);
		//解析并处理查询终端全部信息
		static void analysis_request_cxzdqbxx(void *arg);
		//解析并处理按月查询已开发票总数
		static void analysis_request_aycxykfpzs(void *arg);
		//解析并处理按月查询已开发票信息
		static void analysis_request_aycxykfpxx(void *arg);
		//解析并处理按发票代码号码查询发票
		static void analysis_request_afpdmhmcxfp(void *arg);
		//解析并处理按发票代码号码查询发票
		static void analysis_request_afpdmhmcxfpbs(void *arg);
		//解析并处理停止已开发票查询
		static void analysis_request_tzykfpcx(void *arg);
		//解析并处理按发票种类查询当前发票代码号码
		static void analysis_request_fpzl_fpdm_fphm(void *arg);
		//解析并处理查询终端基础信息
		static void analysis_request_cxzdjcxx(void *arg);
		//通知终端获取升级或授权文件
		static void analysis_request_tzzdsjsq(void *arg);
		//授权终端的端口
		static void analysis_request_sqzddk(void *arg);
		//通知终端查询并上传某月发票至M服务
		static void analysis_request_tzzdscmyfp(void *arg);
		//发票查询回调函数
		static int mqtt_pub_hook(unsigned char *s_data, int total_num, int now_num, void *arg, char *plate_num);
		//解析并处理查询税盘时间
		static void analysis_request_plate_time(void *arg);
		//解析并处理查询税务信息
		static void analysis_request_plate_tax_info(void *arg);
		//解析并处理查询发票信息
		static void analysis_request_plate_invoice_details(void *arg);
		//解析并处理发票开具
		static void analysis_request_plate_fpkj(void *arg);
		//解析并处理全电登陆
		static void analysis_request_qddengl(void *arg);
		//解析并处理全电用户信息获取
		static void analysis_request_qdyongyxxhq(void *arg);
		//解析并处理
		static void analysis_request_qdqiyeqiehuanlbcx(void *arg);
		//
		static void analysis_request_qdqiyeqiehuan(void *arg);
		//解析并处理全电发票查询
		static void analysis_request_qdfpcx(void *arg);
		
		static void analysis_request_qd_get_cookie(void *arg);
		//全电可开红票的蓝票查询
		static void analysis_request_qdhzqrxxszzhquery(void *arg);
		//全电红票确认单开具
		static void analysis_request_hzqrxxsave(void *arg);
		//全电红票确认单列表查询
		static void analysis_request_hzqrxxquery(void *arg);
		//全电红票确认单详情查询
		static void analysis_request_hzqrxxquerydetail(void *arg);
		//解析并处理全电发票查询
		static void analysis_request_qdfpcxxq(void *arg);
		//解析并处理全电发票查询
		static void analysis_request_qdfpcxxz(void *arg);	
		//
		static void analysis_request_qdsxedcx(void *arg);
		//
		static void analysis_request_qdfpkjrpasl(void *arg);
		//
		static void analysis_request_qdlzfpkj(void *arg);
		//
		static void analysis_request_qdhcstaquery(void *arg);
		static void analysis_request_qdfjxxpeizhi(void *arg);
		static void analysis_request_qdcjmbpeizhi(void *arg);
		//解析并处理发票作废
		static void analysis_request_plate_fpzf(void *arg);
		//解析并处理抄报清卡
		static void analysis_request_plate_cbqk(void *arg);
		//解析并处理设置上报时间
		static void analysis_request_set_report_time(void *arg);
		//解析并处理验证证书密码
		static void analysis_request_check_cert_passwd(void *arg);
		//解析并处理发票请求流水号查询发票结果
		static void analysis_request_fpqqlsh_inv_data(void *arg);
		//解析并处理按发票代码号码上传发票
		static void analysis_request_afpdmhmscfp(void *arg);
		//解析并处理校验蓝字发票代码号码信息
		static void analysis_request_jylzfpdmhm(void *arg);
		//解析并处理专用发票红字信息表编号获取
		static void analysis_request_hqhzxxbbh(void *arg);
		//解析并处理红字信息表获取发票
		static void analysis_request_hzxxbhqfp(void *arg);
		////解析并处理发票领用
		//static void analysis_request_zxfply(void *arg); //接口停用
		//解析并处理修改税盘证书口令
		static void analysis_request_xgmrzsmmyz(void *arg);
		//解析并处理红字信息表撤销
		static void analysis_request_hzxxbhcx(void *arg);
		//解析并处理红字信息表按时间段查找
		static void analysis_request_asjdcxhzxxb(void *arg);
		//解析并处理查询终端日志文件
		static void analysis_request_cxzddqrzxx(void *arg);
		//解析并处理开启运维通道
		static void analysis_request_kqzdywtd(void *arg);
		//解析并处理结束维护通道
		static void analysis_request_jszdywtd(void *arg);
		//解析并处理查询终端授权文件
		static void analysis_request_cxzddqsqxx(void *arg);
		//解析并处理发票领用查询
		static void analysis_request_asjdlyfpcx(void *arg);
		//解析并处理发票领用下载
		static void analysis_request_asjdlyfpxz(void *arg);
		//解析并处理服务器测试连接
		static void analysis_request_jcfwqlj(void *arg);
		//解析并处理管理端口电源
		static void analysis_request_adkhgldkdy(void *arg);
		//解析并处理重启终端
		static void analysis_request_glzddy(void *arg);
		//解析并处理清除证书错误信息缓存
		static void analysis_request_asphqczshc(void *arg);
		//按发票代码号码申请整票红字信息表
		static void analysis_request_afpdmhmhqhzxxb(void *arg);
		//获取服务商开票点授权key密钥信息
		static void analysis_request_hqkeysqxx(void *arg);
		//申请接入公共服务平台
		static void analysis_request_sqjrggfwpt(void *arg);
		//取消接入公共服务平台
		static void analysis_request_qxjrggfwpt(void *arg);
		//获取汇总信息数据
		static void analysis_request_hqhzxxsj(void *arg);
		//获取历史购票信息
		static void analysis_request_hqlsgpxx(void *arg);
		//切换端口省电模式
		static void analysis_request_qhdksdms(void *arg);
		//切换整机省电模式
		static void analysis_request_qhzjsdms(void *arg);
		//解析并处理透传勾选认证服务数据
		static void analysis_request_tcgxrzfwsj(void *arg);
		//解析并处理控制税盘离线发票上传
		static void analysis_request_kzlxfpsc(void *arg);
		//解析并处理通知终端断开mqtt连接
		static void analysis_request_tzzddkmqtt(void *arg);
#ifndef MTK_OPENWRT
		static int invoice_fpqqlsh_get_data(struct _app *stream, char *kp_topic, char *plate_num, struct _make_invoice_result *invoice);
		static int invoice_fpqqlsh_get_exsit(struct _app *stream, char *kp_topic, char *plate_num, struct _make_invoice_result *invoice);
	    //static int update_invoice_to_m_server(struct _app     *stream, char *plate_num, struct _make_invoice_result *make_invoice_result);
		//static int update_cancel_invoice_to_m_server(struct _app     *stream, char *plate_num, struct _cancel_invoice_result *cancel_invoice_result);
		static int invoice_data_to_mysql(struct _app *stream, struct mqtt_parm *parm, char *plate_num, struct _make_invoice_result *invoice);
		//static int mysql_re_connect_e(struct _app *stream, MYSQL	*mysql_fd);
#endif
		static int analysis_invoice_sfsylsh(char *inbuf, struct _make_invoice_result *invoice);



#ifdef NEW_USB_SHARE
	
		//static void mqtt_respoonse_port(char *sendbuf, struct _new_usbshare_parm *parm);
		
		//static int get_vid_pid(struct _usb_pro *usb, char *outpid, char *outvid);
		static int mqtt_response_errno_gw(struct _new_usbshare_parm *parm, int err_code);

		static int prepare_mqtt_gw_close_packet(char *sendbuf, struct _new_usbshare_parm *parm);
		static int mqtt_open_share_response(struct _new_usbshare_parm *parm, int fd);
		static void analysis_request_openport(void *arg);
		static void analysis_request_closeport(void *arg);
#endif
#ifdef MTK_OPENWRT
//for mb_boot
static int mb_boot_update(uint8 *name);
int File_Compare_MD5(char *src_file, char *comp_file);
void update_boot_task_hook(void *arg, int timer);
#endif
#endif        
/*===================================常数定义=============================================*/
#ifdef _app_c
#ifdef NEW_USB_SHARE
static const struct err_info       mqttgw_share_err_n[] =
{
	{ MQTT_GW_USBSHARE_OPEN_PORT_ERR_USED, "The port is already used!" },
	{ MQTT_GW_USBSHARE_OPEN_PORT_ERR_NOT_FOUND, "The port no device!" },
	{ MQTT_GW_USBSHARE_OPEN_PORT_ERR_SPACE_FULL, "The space is full!" },
	{ MQTT_GW_USBSHARE_CLOSE_PORT_ERR_GET_START, "Get start port err!" },
	{ MQTT_GW_USBSHARE_CLOSE_PORT_ERR_CLOSE, "Close Port err!" },
	{ MQTT_GW_USBSHARE_OPERA_ERR, "The operation failure!" },
	{ MQTT_GW_USBSHARE_CONNECT_TRANS_SERVER_ERR, "Connect trans server error!" }

};
#endif
        static const struct _ty_err_table       err_n[]=
        {   {   1,	DF_ERR_PORT_CORE_TY_USB_INIT            },//stm32初始化中
            {   2,	DF_ERR_PORT_CORE_TY_USB_PORT_OVER       },//超出USB物理端口数量
            {   3,	DF_ERR_PORT_CORE_TY_USB_PCB_NO          },//该块PCB初始化时,出问题
            {   4,	DF_ERR_PORT_CORE_TY_USB_STM32_OPEN      },//stm32打开端口失败
            {   5,	DF_ERR_PORT_CORE_TY_USB_BUS_NUMBER      },//usb bus上设备超限
            {   6,	DF_ERR_PORT_CORE_TY_USB_NO_FIND_DEV     },//usb端口已经打开,没有发现该设备
            {	7,	DF_ERR_PORT_CORE_TY_USB_NO_PERMISSION	},//无操作权限
            {   8,	DF_ERR_PORT_CORE_TY_USB_STM32_ANSWER    },//stm32在规定时间内没有应答
            {   9,	DF_ERR_PORT_CORE_TY_USB_STM32_CLOSE     },//stm32应答关闭端口失败
            {   10,	DF_ERR_PORT_CORE_TY_USB_STM32_PCB       },//stm32未找到PCB
            {   11,	DF_ERR_PORT_CORE_TY_USB_STM32_DEV       },//stm32未找到DEV
            {   12,	DF_ERR_PORT_CORE_TY_USB_TREE_READ       },//读usb树失败
            {   13,	DF_ERR_PORT_CORE_TY_USB_TREE_PCB        },//没有找到该板子
            {	14,	DF_ERR_PORT_CORE_TY_USB_SET_START_ZERO	},//起始号不能为0
            {   15,	DF_ERR_PORT_CORE_TY_USB_PORT_MANAGE_PORT_USEED  },//该端口已经给使用了  
            {   16,	DF_ERR_PORT_CORE_TY_USB_PORT_MANAGE_NO_CLOSE    },//无权关闭该端口
            {   17,	DF_ERR_PORT_CORE_USB_IP_ROOT            },//服务器非root用户登录
            {   18,	DF_ERR_PORT_CORE_USB_IP_KO              },//usbip没有找到驱动
            {   19,	DF_ERR_PORT_CORE_REGIT_ERR              },//注册协议栈失败
            {   20,	DF_ERR_PORT_CORE_TY_USB_PORT_MANAGE_CHECK_USB_BUSY},//port_manage检测端口中
            {   21,	DF_ERR_PORT_CORE_TY_USB_PORT_MANAGE_CHECK_PAR},//参数检测中
            {   22,	DF_ERR_PORT_CORE_TY_USB_OPER_ERR            },//线路在运行过程中发生故障,USB HUB打开失败,系统要重启
            {   23,	DF_ERR_PORT_CORE_TY_USB_PORT_MANAGE_LINUX_FILE  },//操作系统USB文件系统发生致命错误,服务器要求重新启动
            {   24,	DF_ERR_PORT_CORE_SOCKET_SEV_BIND            },//socket绑定端口失败
            {   25,	DF_ERR_PORT_CORE_SOCKET_LISTER              },//socket监听端口失败
            {   26,	DF_ERR_PORT_CORE_SWITCH_SERIA               },//seria操作错误
            {   27,	DF_ERR_PORT_CORE_TY_USB_MAIN_BUSID          },//没有可用的主busid
            {   28,	DF_ERR_PORT_CORE_TY_USB_MAIN_HUB            },//主hub丢失,要求重新启动
            {   29,	DF_ERR_PORT_CORE_TY_USB_PCB_HUB             },//单板hub丢失
            {   30,	DF_ERR_PORT_CORE_TY_USB_PORT_NO_ENUM        },//没有找到对应的设备枚举
            {   31,	DF_ERR_PORT_CORE_TY_USB_CLOSE               },//设备已经关闭,但文件系统上有设备信息,系统重新启动
            {   32,	DF_ERR_PORT_CORE_USB_IP_BUSID_RE            },//该busid已经存在
            {   33,	DF_ERR_PORT_CORE_USB_IP_WRITE				},//写usbip失败
            {	34,	DF_ERR_PORT_CORE_FTP_DOWNLOAD_ERR			},//下载更新失败
            {	35,	DF_ERR_PORT_CORE_FTP_DOWNLOAD_PROGRAM_IS_NEW},//软件已经是最新版本
			{	36,	DF_ERR_PORT_CORE_PORT_ISNOT_SHUIPAN			},//该端口非支持的税盘
			{	37, DF_ERR_PORT_CORE_GET_INVOICE_ERR            },//获取税盘发票信息错误
			{	38, DF_ERR_PORT_CORE_GET_FP_MONITOR_DATA		},//不支持此类发票查询
			{	39, DF_ERR_PORT_CORE_GET_FP_PARAMETERS			},//获取税盘发票参数错误
			{	40, DF_ERR_PORT_CORE_GET_FP_OPEER_USB			},//获取税盘信息操作USB失败
			{	41, DF_ERR_PORT_CORE_GET_FP_INVIOCE_NUM			},//获取税盘发票数量错误
			{	42, DF_ERR_PORT_CORE_GET_FP_INVOICE_LEN			}//获取发票数据大小错误
            
        };
        #define DF_ERR_PC_START_ZEO                             14
        static const struct _app_order      order_name[]=
        {   
            {   DF_TY_1502_SEV_ORDER_SET_USB_START,			"[设置类]:设置服务器的USB起始号",    		set_start_port		},
            {   DF_TY_1502_SEV_ORDER_SET_IP,				"[设置类]:设置服务器IP",             		set_ip_ip			},
            {   DF_TY_1502_SEV_ORDER_SET_ID,				"[设置类]:设置设备机器编号",           		set_id        		},
            {   DF_TY_1502_SEV_ORDER_SET_TIMER,				"[设置类]:设置服务器时间",             		set_timer       	},
            {   DF_TY_1502_SEV_ORDER_SET_PAR,				"[设置类]:设置服务器参数",             		set_par           	},
            {	DF_TY_1502_SEV_ORDER_SET_TRYOUT_TIME,		"[设置类]:设置试用期限",					set_tryout_date		},
            {	DF_TY_1502_SEV_ORDER_SET_UPDATE_ADDR,		"[设置类]:设置升级服务器地址",				set_update_address	},
            {	DF_TY_1502_SEV_ORDER_SET_VPN_PAR,			"[设置类]:设置VPN服务器参数",				set_vpn_server_par	},
            {	DF_TY_1502_SEV_ORDER_SET_CLOSE_AUTO_UPDATE,	"[设置类]:设置是否关闭自动升级",			set_close_update	},
            {   DF_TY_1502_SEV_ORDER_SEE_SEV_PRO,			"[查询类]:服务器属性",             			get_sev_pro       	},
            {	DF_TY_1502_SEV_ORDER_SEE_USB_PORT_INFOR,	"[查询类]:获取USB设备详细信息",				get_usb_dev_info	},
            {   DF_TY_1502_SEV_ORDER_SEE_SEV_STATE,			"[查询类]:服务器状态",             			get_sev_status      },
            {   DF_TY_1502_SEV_ORDER_SEE_USB_PORT_PRO,		"[查询类]:按USB端口查询端口属性",   		get_port_pro    	},
            {   DF_TY_1502_SEV_ORDER_SEE_PAR,				"[查询类]:服务器参数",             			get_sev_par       	},
            {   DF_TY_1502_SEV_ORDER_SEE_LOG,				"[查询类]:服务器异常日志",          		get_log           	},
            {   DF_TY_1502_SEV_ORDER_SEE_NET_PAR,			"[查询类]:网络设置参数",					get_net_par    		},
            {	DF_TY_1502_SEV_ORDER_SEE_CHECK,				"[查询类]:检测终端外设是否正常",			get_check_all_port	},
            {	DF_TY_1502_SEV_ORDER_SEE_UPDATE_ADDR,		"[查询类]:查询升级服务器地址",				get_update_address	},
            {	DF_TY_1502_SEV_ORDER_SEE_ALL_PORT_STATE,	"[查询类]:查询终端所有USB端口状态",			get_all_port_status	},
            {	DF_TY_1502_SEV_ORDER_SEE_TER_SOFT_VERSION,	"[查询类]:查询终端软件版本信息",			get_ter_soft_version},
            {	DF_TY_1502_SEV_ORDER_SEE_ALL_PORT_CA_NAME,	"[查询类]:查询终端所有USB端口上CA名",		get_all_port_ca_name},
            {	DF_TY_1502_SEV_ORDER_SEE_ALL_PORT_TYPE,		"[查询类]:查询终端所有端口设备类型和证书",	get_all_port_type	},
            {	DF_TY_1502_SEV_ORDER_SEE_VPN_PAR,			"[查询类]:查询VPN服务器参数",				get_vpn_server_par	},
            {	DF_TY_1502_SEV_ORDER_SEE_CLOSE_AUTO_UPDATE,	"[查询类]:查询是否关闭自动升级",			get_close_update	},
			{	DF_TY_1502_SEV_ORDER_SEE_MONTH_FP_NUM,		"[查询类]:查询某月发票数量",				get_month_fp_num	},
			{	DF_TY_1502_SEV_ORDER_SEE_FP_DETAILS,		"[查询类]:查询某月第几张发票详细信息",		get_fp_details		},
			{   DF_TY_1502_SEV_ORDER_SEE_FP_DETAILS_T,      "[查询类]:查询金税盘发票明细数据续传",		get_fp_details_t	},
			{   DF_TY_1502_SEV_ORDER_SEE_FP_DETAILS_S,      "[查询类]:查询金税盘发票明细明文续传",		get_fp_details_s	},
			{	DF_TY_1502_SEV_ORDER_SEE_FP_TYPES,			"[查询类]:查询金税盘支持发票种类",			get_fp_types		},
			{	DF_TY_1502_SEV_ORDER_SEE_MONITOR_DATA,		"[查询类]:查询金税盘监控信息数据",			get_monitor_data	},
			{	DF_TY_1502_SEV_ORDER_SEE_INVOICE_NUM,		"[查询类]:查询金税盘发票段信息",			get_fp_counts_info	},
			{	DF_TY_1502_SEV_ORDER_SEE_FP_AREA_CODE,		"[查询类]:查询金税盘区域代码",				get_area_code		},
			{	DF_TY_1502_SEV_ORDER_SEE_TAX_INFOS,			"[查询类]:查询税务信息数据",				get_tax_infos		},
			{	DF_TY_1502_SEV_ORDER_SEE_VERSION_DESCRIBE,	"[查询类]:查询版本控制描述",				get_version_describe},
            {   DF_TY_1502_SEV_ORDER_OPER_OPEN_USB,			"[动作类]:按USB端口号申请使用权",			open_port    		},
            {   DF_TY_1502_SEV_ORDER_OPER_CLOSE_USB,		"[动作类]:按USB端口号释放使用权",			close_port    		},
            {   DF_TY_1502_SEV_ORDER_OPER_ID_OPEN_USB,		"[动作类]:按机柜编号申请使用权",        	open_port_id     	},
            {   DF_TY_1502_SEV_ORDER_OPER_ID_CLOSE_USB,		"[动作类]:按机柜编号释放控制权",        	close_port_id     	},
            {   DF_TY_1502_SEV_ORDER_OPER_CLOSE_NOW,		"[动作类]:强制关闭端口",             		close_port_now     	},
            {   DF_TY_1502_SEV_ORDER_OPER_POWER_OFF,		"[动作类]:关机",            	 			power_off          	},
            {   DF_TY_1502_SEV_ORDER_OPER_HEART,			"[动作类]:客户端已打开端口上报",			heart     			},
            {   DF_TY_1502_SEV_ORDER_OPER_RESET,			"[动作类]:重新启动",             			reset     			},
            {   DF_TY_1502_SEV_ORDER_OPER_REDIT,			"[动作类]:注册主板",             			regdit              },
            {   DF_TY_1502_SEV_ORDER_OPER_OPEN_NOACTIVE,	"[动作类]:打开USB端口但不添加USBIP",		open_port_no_active	},
            {   DF_TY_1502_SEV_ORDER_SEE_OPEN_PORT,			"[动作类]:维持查看打开的设备并持续打开",	keep_open_port		},
            {	DF_TY_1502_SEV_ORDER_OPER_UPDATA,			"[动作类]:检查版本更新并升级",				check_version_updata},
            {	DF_TY_1502_SEV_ORDER_OPER_DETTACH_PORT,		"[动作类]:使用Dettach关闭USB端口",			close_port_by_dettach}
        };
        



    #endif        
        




#endif
