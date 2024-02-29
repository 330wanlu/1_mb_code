#ifndef _EVENT_FILE_DEFINE
    #define _EVENT_FILE_DEFINE
    #include "_switch_dev.h"
    //#include "../linux_s/_timer.h"
    /*===================================立即数定义=============================================*/
    #define DF_EVENT_FILE_NAME                  "event"
    #define DF_EVENT_CM_WRITE                   100                             //写事件
    #define DF_EVENT_CM_READ                    (DF_EVENT_CM_WRITE+1)           //读事件  
    #define DF_EVENT_CM_EVENT_NOTE              (DF_EVENT_CM_READ+1)            //翻译事件
    #define DF_EVENT_CM_REASON_NOTE             (DF_EVENT_CM_EVENT_NOTE+1)      //翻译原因    
        /*--------------------------------事件名称----------------------------------------*/
        #define DF_EVENT_NAME_RESET             1                               //重新启动
        #define DF_EVENT_NAME_POWER_OFF         (DF_EVENT_NAME_RESET+1)         //关闭电源   
		#define DF_EVENT_NAME_IMPORT_RECORD		(DF_EVENT_NAME_POWER_OFF+1)		//重要记录
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
    #ifdef _event_file_c
        #define out(s,arg...)                           //log_out("_event_file.c",s,##arg) 
        #define e_event_file
    #else
        #define e_event_file                            extern
    #endif
    /*===================================结构体定义=============================================*/
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
        
    /*===================================函数定义===============================================*/
    e_event_file    int event_file_add(char *switch_name);
    #ifdef _event_file_c
        static int event_file_open(struct ty_file	*file);
        static int event_file_ctl(void *data,int fd,int cm,va_list args);
        static int _event_file_close(void *data,int fd);
    #endif    
    /*===================================常数定义===============================================*/
    #ifdef _event_file_c
        static const struct _event_file_e   event_name[]=
        {   {   DF_EVENT_NAME_POWER_OFF,		"关闭电源"  },
            {   DF_EVENT_NAME_RESET,			"重新启动"  },
			{	DF_EVENT_NAME_IMPORT_RECORD,	"重要记录"	}
        };
        static const struct _event_file_e   event_reason[]=
        {   {   DF_EVENT_REASON_NO_HEART,				"主机在规定时间内没有心跳"  },
            {   DF_EVENT_REASON_NET,					"客户端发起关机命令"        },
            {   DF_EVENT_REASON_MAIN_HUB,				"运行过程中主HUB丢失"       },
            {   DF_EVENT_REASON_SOFT,					"FTP有升级程序"             },
            {   DF_EVENT_REASON_SET_BUSID,				"设置busid"                 },
            {   DF_EVENT_REASON_FILE_USB,				"设备关闭后,文件系统存在"   },
			{	DF_EVENT_REASON_CLOSE_SOCKET_FAILED,	"关闭端口时，关闭socket失败"},
			{	DF_EVENT_REASON_CHECK_SYS_ERR,			"检测到系统存在异常"		},
			{	DF_EVENT_REASON_GET_M3_FLASH_ERROR,		"获取单片机FLASH数据错误"	}
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
