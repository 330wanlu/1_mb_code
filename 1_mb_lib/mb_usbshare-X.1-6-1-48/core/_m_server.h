#ifndef _M_SERVER_DEFINE
	#define _M_SERVER_DEFINE
    /*======================================�����ļ�=============================================*/
    #include "_port_core.h"	
	#include "../linux_sys/mqtt_client.h"
	#include "../core/_machine_infor.h"
	#include "../linux_s/_lock.h"
    /*=====================================����������============================================*/
    #define DF_MB_MQTT_PUB_NO_ANSWER                            100                         //MQTT��������Ӧ��
	#define DF_MB_MQTT_PUB_NEED_ANSWER                          101                         //MQTT������ҪӦ��

    #ifdef _m_server_c
		#ifdef DEBUG
			#define out(s,arg...)       				log_out("all.c",s,##arg) 
		#else
			#define out(s,arg...)						log_out("all.c",s,##arg) 
		#endif	
        #define e_m_server
    #else
        #define e_m_server                      extern
    #endif
    

    /*======================================�ṹ��===============================================*/
    #ifdef _m_server_c
		#define DEAFULT_CONNECT_TIME 60		//MQTTĬ�������ʱ��
		#define DF_MESSAGE_LINE_NUM 1000


		struct _message_queue_line
		{
			int state;//��ǰ�ڴ�״̬
			char qqlsh[50];//������ˮ��
			int answer_flag;//Ӧ���־
			int answer_result;//0ʧ�� 1�ɹ�
			char *answer_data;
		};

		void *_mfd_stream;
        struct _m_server_fd
        {	int state;
            struct ty_file_mem	*mem;
            void *mem_dev;
            struct ty_file_fd	*mem_fd;
            struct _m_server  *dev;
        };
        
        struct _m_server
        {   int state;                                      //״̬
            int lock;
			int machine_fd;

			char mqtt_server[200];
			int mqtt_port;
			char user_name[20];
			char user_passwd[50];
			char topic[200];
			int mqtt_reg_flag;
			int connect_time;
			int mqtt_state;		//0��δ��¼					1���Ѿ���¼
			struct _message_queue_line m_line[DF_MESSAGE_LINE_NUM];
            struct _m_server_fd fd[DF_FILE_OPEN_NUMBER];
        };
		struct _m_server_ctl_fu
		{
			int cm;
			int(*ctl)(struct _m_server_fd   *id, va_list args);
		};




    #endif
    /*======================================��������==================================================*/
    e_m_server  int _m_server_add();
	e_m_server  int message_callback_deal_answer(struct mqtt_parm	*parm, char *message);
	e_m_server	int _m_pub_no_answer(char *topic,char *message);
	e_m_server	int _m_pub_need_answer(char *qqlsh, char *topic, char *message, char *g_data, int mqtt_type);
    #ifdef _m_server_c
        static int _m_server_open(struct ty_file	*file);
        static int _m_server_ctl(void *data,int fd,int cm,va_list args);
        static int _m_server_close(void *data,int fd);
        
		//static void sub_message_callback_function_sync1(struct mosquitto *mosq, void *obj, const struct mosquitto_message *message);
		//static void sub_err_callback_deal_function1(struct _mqtt_client  *mqtt_client, int errnum);
		static int find_available_queue_mem(struct _m_server     *stream);
    #endif 

    /*=====================================��������===================================================*/
    #ifdef _m_server_c
              
		static const struct _m_server_ctl_fu ctl_fun[] = {
		
		};

        static const struct _file_fuc	m_server_fuc=
		{	.open=_m_server_open,
			.read=NULL,
			.write=NULL,
			.ctl=_m_server_ctl,
			.close=_m_server_close,
			.del=NULL
		};      

    #endif        
        
    




#endif
