#ifndef _DEVOPS_DEFINE
	#define _DEVOPS_DEFINE
    #include "_switch_dev.h"
    #include "_ty_pd.h"
    #include "_event_file.h"
	#include "_register.h"
	#include <sys/vfs.h>
	#include "../manage/_process_manage.h"


    /*===================================����������=============================================*/
   /* #define DF_MACHINE_INFOR_CM_GET_ID				100
    #define DF_MACHINE_INFOR_CM_SET_ID				(DF_MACHINE_INFOR_CM_GET_ID+1)
    #define DF_MACHINE_INFOR_CM_GET_HARD			(DF_MACHINE_INFOR_CM_SET_ID+1)
    #define DF_MACHINE_INFOR_CM_GET_KERNEL			(DF_MACHINE_INFOR_CM_GET_HARD+1)
    #define DF_MACHINE_INFOR_CM_GET_SOFT			(DF_MACHINE_INFOR_CM_GET_KERNEL+1)
    #define DF_MACHINE_INFOR_CM_GET_MACHINE_S		(DF_MACHINE_INFOR_CM_GET_SOFT+1)
    #define DF_MACHINE_INFOR_CM_POWER_OFF			(DF_MACHINE_INFOR_CM_GET_MACHINE_S+1) 
    #define DF_MACHINE_INFOR_CM_RESET_POWER			(DF_MACHINE_INFOR_CM_POWER_OFF+1)    
	#define DF_MACHINE_INFOR_CM_SET_TRY_TIME		(DF_MACHINE_INFOR_CM_RESET_POWER+1)
	#define DF_MACHINE_INFOR_CM_GET_TRY_TIME		(DF_MACHINE_INFOR_CM_SET_TRY_TIME+1)
	#define DF_MACHINE_INFOR_CM_GET_MQTT_INFO		(DF_MACHINE_INFOR_CM_GET_TRY_TIME+1)
	#define	DF_MACHINE_INFOR_CM_GET_MQTT_USER		(DF_MACHINE_INFOR_CM_GET_MQTT_INFO+1)
	#define DF_MACHINE_INFOR_CM_GET_INFOMATION		(DF_MACHINE_INFOR_CM_GET_MQTT_USER+1)
    #define DF_MACHINE_INFOR_CM_GET_MODE_SUPPORT	(DF_MACHINE_INFOR_CM_GET_INFOMATION+1)
	#define DF_MACHINE_INFOR_CM_GET_MODE_ENABLE		(DF_MACHINE_INFOR_CM_GET_MODE_SUPPORT+1)
	#define DF_MACHINE_INFOR_CM_GET_MYSQL_PROCESS	(DF_MACHINE_INFOR_CM_GET_MODE_ENABLE+1)
	#define DF_MACHINE_INFOR_CM_GET_EXTEND_SPACE_PROCESS	(DF_MACHINE_INFOR_CM_GET_MYSQL_PROCESS+1)
	#define DF_MACHINE_INFOR_CM_GET_FTP_PROCESS		(DF_MACHINE_INFOR_CM_GET_EXTEND_SPACE_PROCESS+1)
	#define DF_MACHINE_INFOR_CM_GET_SSH_PROCESS		(DF_MACHINE_INFOR_CM_GET_FTP_PROCESS+1)
	#define DF_MACHINE_INFOR_CM_GET_NTPDATE_PROCESS		(DF_MACHINE_INFOR_CM_GET_SSH_PROCESS+1)
	#define DF_MACHINE_INFOR_CM_GET_TFCARD_PROCESS		(DF_MACHINE_INFOR_CM_GET_NTPDATE_PROCESS+1)
	#define DF_MACHINE_INFOR_CM_GET_CAREADER_PROCESS		(DF_MACHINE_INFOR_CM_GET_TFCARD_PROCESS+1)
	#define DF_MACHINE_INFOR_CM_GET_USERLOAD_PROCESS		(DF_MACHINE_INFOR_CM_GET_CAREADER_PROCESS+1)
	#define DF_CONFIG_FILE_PATH			"/etc/mqttserver.conf"*/
    #ifdef _invoice_transmit_c
         #ifdef DEBUG
			#define out(s,arg...)       				log_out("all.c",s,##arg) 
		#else
			#define out(s,arg...)						log_out("all.c",s,##arg) 
		#endif	
        #define e_invoice_transmit
    #else
        #define e_invoice_transmit                 extern
    #endif



    /*===================================�ṹ�嶨��============================================*/

    #ifdef _invoice_transmit_c
        struct _invoice_transmit_fd
        {	int state;
            struct ty_file_mem	*mem;
            void *mem_dev;
            struct ty_file_fd	*mem_fd;
            struct _invoice_transmit  *dev;
        };
        
        struct _invoice_transmit
        {   int state;
            int mqtt_lock;;
			int task;
			int get_net_fd;
			int ty_usb_fd;
			int machine_fd;
			char ter_id[20];
            struct _invoice_transmit_fd    fd[DF_FILE_OPEN_NUMBER];


			char mqtt_server[200];
			int mqtt_port;
			char user_name[20];
			char user_passwd[50];
			char topic[200];
            //char *hard;   
			int connect_time;
			
			int is_invoice_transmit; // 0��  1��
			int is_debug;// 0��  1��0���ػ��Լ������ļ��洢    1���ؼ�Զ�����
        };
        
		void *invoice_transmit_fd;

        struct _invoice_transmit_fu
        {   int cm;
			int(*ctl)(struct _invoice_transmit_fd   *id, va_list args);
        };

#ifndef MTK_OPENWRT
	#define INVOICE_TRANSMIT_MAX_MESSAGE_QUEUE_NUM 100	//�����Ϣ�����������
	#define INVOICE_TRANSMIT_MAX_DEAL_MESSAGE_TASK 10	//�����Ϣ�߳�����
#else
	#define INVOICE_TRANSMIT_MAX_MESSAGE_QUEUE_NUM 100	//�����Ϣ�����������
	#define INVOICE_TRANSMIT_MAX_DEAL_MESSAGE_TASK 5	//�����Ϣ�߳�����
#endif	

	#define INVOICE_TRANSMIT_DEAFULT_CONNECT_TIME 60		//MQTTĬ�������ʱ��
		struct _invoice_transmit_message_line
		{
			unsigned short inuse;
			unsigned short state;
			struct mqtt_parm parm;
			unsigned char fpqqlsh[60];
			unsigned int message_time;
			char *message;
		};

		struct _invoice_transmit_message_callback
		{
			struct _invoice_transmit_message_line invoice_transmit_message_line[INVOICE_TRANSMIT_MAX_MESSAGE_QUEUE_NUM];
			unsigned int insert_num;
			unsigned int deal_num;
			unsigned int task_num;
			unsigned int err_count;
		};

		struct _invoice_transmit_message_callback invoice_transmit_message_callback;
			

		//struct _invoice_transmit_err_no_infos
		//{
		//	int err;
		//	int errnum;
		//	char errinfo[200];
		//};
		//static const struct _invoice_transmit_err_no_infos invoice_transmiterr_no_infos[] =
		//{
		//	//Э�����ݻ��ն˴���
		//	{ DF_TAX_ERR_CODE_TER_SYSTEM_ERROR, 301, "Terminal system error." },									//�ն�ϵͳ����
		//	{ DF_TAX_ERR_CODE_COMMAND_UNSUPPORTED, 302, "This command word is not supported." },					//�����ֲ�֧��
		//	{ DF_TAX_ERR_CODE_SYSTEM_MEM_INSUFFICIENT, 303, "The terminal memory is insufficient." },				//ϵͳ�ڴ治��
		//	{ DF_TAX_ERR_CODE_DATA_PARSING_FAILED, 304, "The JSON data parsing failure." },							//json����ʧ��
		//	{ DF_TAX_ERR_CODE_SYSTEM_TIME_ERROR, 312, "The terminal system time error." },								//��ǰ�ն�ϵͳʱ�����
		//	{ DF_TAX_ERR_CODE_CONNECT_MYSQL_ERROR, 325, "Mysql database connection error." },							//���ݿ����Ӵ���
		//	//������Ʊ����
		//	{ DF_TAX_ERR_CODE_EXCEED_KP_END_DATE, 305, "Exceeding the billing deadline." },							//������Ʊ��ֹ����
		//	{ DF_TAX_ERR_CODE_EXCEED_SINGLE_LIMIT, 306, "Exceeding a single invoice billing limit." },				//�������ŷ�Ʊ��Ʊ�޶�
		//	{ DF_TAX_ERR_CODE_EXCEED_OFFLINE_LIMIT, 307, "Exceeding the cumulative billing limit." },				//���������޶�
		//	{ DF_TAX_ERR_CODE_UNSUPPORT_INV_TYPE, 308, "This invoice type invoice is not supported." },				//��֧�ָ÷�Ʊ����
		//	{ DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR, 309, "Invoice data parsing error." },							//��Ʊ���ݽ���ʧ��
		//	{ DF_TAX_ERR_CODE_TAX_RATE_UNSUPPORTED, 310, "Authorized tax rate does not exist." },					//��Ȩ˰�ʲ�֧��
		//	{ DF_TAX_ERR_CODE_HAVE_NO_INVOICE, 311, "Have no invoice available." },									//û�п��÷�Ʊ			
		//	{ DF_TAX_ERR_CODE_INV_DATA_TOO_LARGE, 313, "Invoice data too large." },									//��Ʊ��������			
		//	{ DF_TAX_ERR_CODE_INV_DATA_DISACCORD, 315, "Invoice data is inconsistent with plate infomation." },		//��Ʊ������˰����Ϣ��һ��
		//	{ DF_TAX_ERR_CODE_EXCEED_OFFLINE_LIMIT_TIME, 318, "Exceeding offline invoice time." },						//��������ʱ��
		//	{ DF_TAX_ERR_CODE_FPQQLSH_IS_INUSING, 326, "The invoice serial number is inusing." },						//��ǰ��ˮ�����ڿ�Ʊ��
		//	//˰�̲���ͨ�ô���
		//	{ DF_TAX_ERR_CODE_PLATE_IN_USED, 314, "The plate is in used." },											//˰�����ڱ�ʹ��
		//	{ DF_TAX_ERR_CODE_PLATE_OPER_FAILED, 316, "The plate operation failed." },									//˰�̲���ʧ��
		//	{ DF_TAX_ERR_CODE_PLATE_CERT_PASSWD_ERR, 317, "The plate cert passwd failed." },							//֤��������			
		//	{ DF_TAX_ERR_CODE_PLATE_NON_EXISTENT, 319, "The plate is not on this terminal." },							//˰�̲��ڸ��ն���
		//	{ DF_TAX_ERR_CODE_MONTH_HAVE_NO_INVPICE, 320, "No invoice for this month's inquiry." },					//��ǰҪ��ѯ�޷�Ʊ
		//	{ DF_TAX_ERR_CODE_USE_ILL_CHAR_OR_UTF8, 321, "Illegal character or UTF8 encoding is used." },				//ʹ���˷Ƿ��ַ���utf8����
		//	{ DF_TAX_ERR_CODE_CAN_NOT_USE_IOZSV, 322, "Illegal characters cannot be used in tax numbers.(Example : I O Z S V)" },//˰���в���ʹ��IOZSV�Ƿ��ַ�
		//	{ DF_TAX_ERR_CODE_INV_SERIAL_NUMBER_UNEXIST, 323, "Invoice serial number does not exist." },				//��Ʊ��ˮ�Ų�����
		//	{ DF_TAX_ERR_CODE_INV_SERIAL_NUMBER_BEEN_USED, 324, "Invoice serial number has already been issued. " },//�÷�Ʊ��ˮ���Ѿ����߹���
		//	{ DF_TAX_ERR_CODE_USB_PORT_NOT_AUTH, 327, "This USB port is not authorized." }//��USB�˿�δ��Ȩ
		//};
		
    #endif
    /*====================================��������=============================================*/
	e_invoice_transmit int _invoice_transmit_add(void);	

    #ifdef _invoice_transmit_c
        static int invoice_transmit_open(struct ty_file	*file);
        static int invoice_transmit_ctl(void *data,int fd,int cm,va_list args);
        static int invoice_transmit_close(void *data,int fd);   

		//static int find_deal_queue_mem(struct _invoice_transmit     *stream);
		static void task_deal_message_queue(void *arg, int o_timer);
		static int analysis_json_head(char *inbuf, struct mqtt_parm *parm);
		static int find_available_queue_mem(struct _invoice_transmit     *stream);
		static void sub_message_callback_function_sync(struct mosquitto *mosq, void *obj, const struct mosquitto_message *message);
		static void sub_err_callback_deal_function(struct _mqtt_parms  *mqtt_client, int errnum);
		//static int deal_open_invoice_transmit_function(struct _invoice_transmit      *stream);
		//static void task_http_get_is_open_invoice_transmit(void *arg, int timer);
		//static int mqtt_response_errno(int errnum, struct mqtt_parm *parm, char *errinfo);
		//static int _mqtt_client_message_pub(char *topic, char *sbuf, int try_num);

		//static int analysis_json_is_open_invoice_transmit(struct _invoice_transmit *stream, char *inbuf);
		//static int save_cert_file(char *base_file, char *path);
		//static int get_file_base64(char *file, char *buf, int len);
		static void  deal_with_cmd(struct _invoice_transmit *stream, int mem_i, void *fucntion);

		//�����������������쳣����
		static void analysis_request_cmd_not_support(void *arg);
		//����������sshָ��
		//static void analysis_request_sshcmd(void *arg);
		//�����������ļ��ϴ�
		//static void analysis_request_uploadfile(void *arg);
		//�����������ļ�����
		//static void analysis_request_downloadfile(void *arg);
    #endif
    /*====================================��������============================================*/
    #ifdef _invoice_transmit_c
		static const struct _invoice_transmit_fu ctl_fun[] =
		{ 
		//	{ DF_MACHINE_INFOR_CM_GET_ID, get_id },
		//{ DF_MACHINE_INFOR_CM_SET_ID, set_id },
		////{ DF_MACHINE_INFOR_CM_GET_HARD, get_hard },
		//{ DF_MACHINE_INFOR_CM_GET_KERNEL, get_kernel },
		//{ DF_MACHINE_INFOR_CM_GET_SOFT, get_soft },
		//{ DF_MACHINE_INFOR_CM_GET_MACHINE_S, get_dev_infor },
		//{ DF_MACHINE_INFOR_CM_POWER_OFF, power_off },
		//{ DF_MACHINE_INFOR_CM_RESET_POWER, power_reset },
		//{ DF_MACHINE_INFOR_CM_SET_TRY_TIME, set_try_out_date },
		//{ DF_MACHINE_INFOR_CM_GET_TRY_TIME, get_try_out_date },
		//{ DF_MACHINE_INFOR_CM_GET_MQTT_INFO, get_mqtt_server_info },
		//{ DF_MACHINE_INFOR_CM_GET_MQTT_USER, get_mqtt_user_passwd },
		//{ DF_MACHINE_INFOR_CM_GET_INFOMATION, get_ter_information },
		//{ DF_MACHINE_INFOR_CM_GET_MODE_SUPPORT, get_mode_support },
		//{ DF_MACHINE_INFOR_CM_GET_MODE_ENABLE, get_mode_enable },
		//#ifndef MTK_OPENWRT
		//{ DF_MACHINE_INFOR_CM_GET_MYSQL_PROCESS, get_mysql_process },
		//#endif
		//{ DF_MACHINE_INFOR_CM_GET_EXTEND_SPACE_PROCESS, get_extend_space_process },
		//{ DF_MACHINE_INFOR_CM_GET_FTP_PROCESS, get_ftp_process },
		//{ DF_MACHINE_INFOR_CM_GET_SSH_PROCESS, get_ssh_process },
		//{ DF_MACHINE_INFOR_CM_GET_NTPDATE_PROCESS, get_ntpdate_process },
		//{ DF_MACHINE_INFOR_CM_GET_TFCARD_PROCESS, get_tfcard_process },
		//{DF_MACHINE_INFOR_CM_GET_CAREADER_PROCESS,get_careader_process},
		//{DF_MACHINE_INFOR_CM_GET_USERLOAD_PROCESS,get_userload_process}
        };
        
		static const struct _file_fuc	invoice_transmit_fuc =
		{	.open=invoice_transmit_open,
			.read=NULL,
			.write=NULL,
			.ctl=invoice_transmit_ctl,
			.close=invoice_transmit_close,
			.del=NULL
		};

    #endif        

#endif
