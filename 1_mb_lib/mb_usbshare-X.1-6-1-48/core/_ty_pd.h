#ifndef _TY_PD_H
    #define _TY_PD_H
    #include "../core/_switch_dev.h"
	#include "../manage/_ma_id.h"
	#include "_event_file.h"
	#include <netdb.h>
	#include "../file/_file_ac.h"
    /*======================================立即数定义=================================================*/
    #define DF_TY_PD_CM_RESET                   100
    #define DF_TY_PC_CM_POWER_OFF               (DF_TY_PD_CM_RESET+1)
    #define DF_TY_PD_CM_ADD_NAME                (DF_TY_PC_CM_POWER_OFF+1)
    #define DF_TY_PD_CM_DEL_NAME                (DF_TY_PD_CM_ADD_NAME+1)
    
    //#define DF_TY_CORE_CM_PD                    20
    #define DF_TY_PD_NAME                       "ty_pd"
    #ifdef _ty_pd_c
        #ifdef DEBUG
			#define out(s,arg...)       				//log_out("all.c",s,##arg) 
		#else
			#define out(s,arg...)
		#endif	
		#define DF_TY_PD_N                      20
		#define	DF_TY_PD_CHEK_NUM				20
		#define	DF_TY_PD_CPU_MAX_USED			80
		#define DF_TY_PD_MEM_MAX_USED			80
        #define e_ty_pd
    #else
        #define e_ty_pd                    extern
    #endif
    /*======================================结构体定义================================================*/
    struct _ty_pd_add
    {   char *name;
    };
    
    
    #ifdef _ty_pd_c



        struct _ty_pd_fd
        {	int state;
            struct ty_file_mem	*mem;
            void *mem_dev;
            struct ty_file_fd	*mem_fd;
            struct _ty_pd    *dev;
        };
        struct _ty_pd_file
        {   int state;
            char name[DF_FILE_NAME_MAX_LEN]; 
        };
        
        
        
        struct _ty_pd
        {   int state;
            int timer;
            int check_timer;  
            int task;    
			int task_pd;
            int lock;
			int sys_state;
			int err_count;
			unsigned long port_check_task_new;
			unsigned long port_check_task_last;
            struct _ty_pd_fd    fd[DF_FILE_OPEN_NUMBER]; 
            char swith_name[DF_FILE_NAME_MAX_LEN];
            int switch_fd;
			int ty_usb_fd;
			int ty_usb_m_fd;
			int mem_err_counts;//内存异常次数
            struct _ty_pd_file  file[DF_TY_PD_N];
        };


    #endif    

#define DF_TASK_PORT_CHECK 100
		void *ty_pd_id;
    /*====================================函数定义=================================================*/
    e_ty_pd int ty_pd_add(char *switch_name);
    //e_ty_pd int ty_pd_add_file(char *name);
    //e_ty_pd int ty_pd_del_file(char *name);
	e_ty_pd int ty_pd_update_task_status(int task_num, unsigned long up_time);
    #ifdef _ty_pd_c
        static int ty_pd_open(struct ty_file	*file);
        static void task_pd(void *arg,int timer);
        static int ty_pd_ctl(void *data,int fd,int cm,va_list args);
        static int ty_pd_close(void *data,int fd);
		static int ty_pd_read(void *data, int fd,uint8 *buf, int buf_len);
		
    #endif 
    /*====================================常数定义=================================================*/
    #ifdef _ty_pd_c
        static const struct _file_fuc	ty_pd_fuc=
		{	.open=ty_pd_open,
			.read=ty_pd_read,
			.write=NULL,
			.ctl=ty_pd_ctl,
			.close=ty_pd_close,
			.del=NULL
		};

    #endif        
#endif
