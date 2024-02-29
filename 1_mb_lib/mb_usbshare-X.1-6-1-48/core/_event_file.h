#ifndef _EVENT_FILE_DEFINE
    #define _EVENT_FILE_DEFINE
    #include "_switch_dev.h"
    //#include "../linux_s/_timer.h"
    /*===================================����������=============================================*/
    #define DF_EVENT_FILE_NAME                  "event"
    #define DF_EVENT_CM_WRITE                   100                             //д�¼�
    #define DF_EVENT_CM_READ                    (DF_EVENT_CM_WRITE+1)           //���¼�  
    #define DF_EVENT_CM_EVENT_NOTE              (DF_EVENT_CM_READ+1)            //�����¼�
    #define DF_EVENT_CM_REASON_NOTE             (DF_EVENT_CM_EVENT_NOTE+1)      //����ԭ��    
        /*--------------------------------�¼�����----------------------------------------*/
        #define DF_EVENT_NAME_RESET             1                               //��������
        #define DF_EVENT_NAME_POWER_OFF         (DF_EVENT_NAME_RESET+1)         //�رյ�Դ   
		#define DF_EVENT_NAME_IMPORT_RECORD		(DF_EVENT_NAME_POWER_OFF+1)		//��Ҫ��¼
        /*---------------------------------ԭ��------------------------------------------*/
        #define DF_EVENT_REASON_NO_HEART			1                               //��λʱ����û������
        #define DF_EVENT_REASON_NET					100                             //��������
        #define DF_EVENT_REASON_MAIN_HUB			(DF_EVENT_REASON_NET+1)         //��HUBû�з��� 
        #define DF_EVENT_REASON_SET_MA				(DF_EVENT_REASON_MAIN_HUB+1)    //���û������
        #define DF_EVENT_REASON_SOFT				(DF_EVENT_REASON_SET_MA+1)      //�������   
        #define DF_EVENT_REASON_SET_BUSID			(DF_EVENT_REASON_SOFT+1)        //����busid
        #define DF_EVENT_REASON_FILE_USB			(DF_EVENT_REASON_SET_BUSID+1)   //�豸�Ѿ��ر�,���ļ�ϵͳ������   
		#define	DF_EVENT_REASON_CLOSE_SOCKET_FAILED	(DF_EVENT_REASON_FILE_USB+1)
		#define DF_EVENT_REASON_CHECK_SYS_ERR		(DF_EVENT_REASON_CLOSE_SOCKET_FAILED+1)
		#define DF_EVENT_REASON_GET_M3_FLASH_ERROR  (DF_EVENT_REASON_CLOSE_SOCKET_FAILED+1)
    #ifdef _event_file_c
        #define out(s,arg...)                           //log_out("_event_file.c",s,##arg) 
        #define e_event_file
    #else
        #define e_event_file                            extern
    #endif
    /*===================================�ṹ�嶨��=============================================*/
    #ifdef _event_file_c
        struct _event_file_fd
        {	int state;
            struct ty_file_mem	*mem;
            void *mem_dev;
            struct ty_file_fd	*mem_fd;
			struct _event_file    *dev;
        };
        struct _event_file
        {   int state; 
            struct _event_file_fd    fd[DF_FILE_OPEN_NUMBER]; 
            char switch_name[DF_FILE_NAME_MAX_LEN];
            int switch_fd;
        };
        
        struct _event_file_e
        {   int c;
            char *e;
        };
        
        
    #endif
        
    /*===================================��������===============================================*/
    e_event_file    int event_file_add(char *switch_name);
    #ifdef _event_file_c
        static int event_file_open(struct ty_file	*file);
        static int event_file_ctl(void *data,int fd,int cm,va_list args);
        static int _event_file_close(void *data,int fd);
    #endif    
    /*===================================��������===============================================*/
    #ifdef _event_file_c
        static const struct _event_file_e   event_name[]=
        {   {   DF_EVENT_NAME_POWER_OFF,		"�رյ�Դ"  },
            {   DF_EVENT_NAME_RESET,			"��������"  },
			{	DF_EVENT_NAME_IMPORT_RECORD,	"��Ҫ��¼"	}
        };
        static const struct _event_file_e   event_reason[]=
        {   {   DF_EVENT_REASON_NO_HEART,				"�����ڹ涨ʱ����û������"  },
            {   DF_EVENT_REASON_NET,					"�ͻ��˷���ػ�����"        },
            {   DF_EVENT_REASON_MAIN_HUB,				"���й�������HUB��ʧ"       },
            {   DF_EVENT_REASON_SOFT,					"FTP����������"             },
            {   DF_EVENT_REASON_SET_BUSID,				"����busid"                 },
            {   DF_EVENT_REASON_FILE_USB,				"�豸�رպ�,�ļ�ϵͳ����"   },
			{	DF_EVENT_REASON_CLOSE_SOCKET_FAILED,	"�رն˿�ʱ���ر�socketʧ��"},
			{	DF_EVENT_REASON_CHECK_SYS_ERR,			"��⵽ϵͳ�����쳣"		},
			{	DF_EVENT_REASON_GET_M3_FLASH_ERROR,		"��ȡ��Ƭ��FLASH���ݴ���"	}
        };
        static const struct _file_fuc	event_file_fuc=
		{	.open=event_file_open,
			.read=NULL,
			.write=NULL,
			.ctl=event_file_ctl,
			.close=_event_file_close,
			.del=NULL
		};
        
        
    #endif        
    
    
    
    
    
    
    

#endif
