#ifndef _GET_NET_TIME_DEFINE
    #define _GET_NET_TIME_DEFINE
	#include "../linux_sys/_http.h"
	#include "../file/_file.h"
	#include "_port_core.h"
	#include "../linux_s/_task.h"
	//#include "_port_core_note.h"
	//#include "../linux_s/_timer.h"
/*=========================================立即数定义===============================================*/
		#define DF_TY_GET_NET_TIME                   	1               			//网络对时
   
    #ifdef _get_net_time_c
       #ifdef DEBUG
			#define out(s,arg...)       				log_out("all.c",s,##arg) 
		#else
			#define out(s,arg...)
		#endif	
        #define e_get_net_time
		#define DF_GET_NET_TIME_TASK_TIMER			1000                  //获取IP线程时间
    #else
        #define e_get_net_time                                   extern    
    #endif
    
    
    #ifdef _get_net_time_c
		struct _get_time
		{
			uint8 year[4];
			uint8 month[2];
			uint8 date[2];
			uint8 wday[1];
			uint8 hour[2];
			uint8 minute[2];
			uint8 second[2];
		};
    
    #endif
    
    /*=========================================结构体定义===============================================*/
    struct _get_net_time_fd
    {	
		int state;
        struct ty_file_mem	*mem;
        void *mem_dev;
        struct ty_file_fd	*mem_fd;
        struct _register  *dev;
    };
        

    struct _get_net_time
    {   int state;                                                      //本模块状态
		int lock;                                                       //数据锁
		int task;
		struct _get_net_time_fd fd[DF_FILE_OPEN_NUMBER];
		uint8 timebuf[19];
    };
    
    /*========================================函数定义=================================================*/
    e_get_net_time    int _get_net_time_add(void);
    #ifdef _get_net_time_c
	static int _get_net_time_open(struct ty_file	*file);
	static int _get_net_time_ctl(void *data,int fd,int cm,va_list args);
	static int _get_net_time_close(void *data,int fd);
	//static int _get_http_buf_time(char *str,char *end,uint8 *t_time);
	//static int _get_time_set_sys_time(uint8 *timebuf);
	//static int get_file_head_end(const char *buf,const char *eth,char **p_head,char **p_end);
	static void _get_net_time_task(void *arg,int o_timer);
    #endif
    /*======================================常数定义===================================================*/
    #ifdef _get_net_time_c
        static const struct _file_fuc	get_net_time_fuc=
		{	.open=_get_net_time_open,
			.read=NULL,
			.write=NULL,
			.ctl=_get_net_time_ctl,
			.close=_get_net_time_close,
			.del=NULL
		};
    #endif
#endif
