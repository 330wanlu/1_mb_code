#ifndef _TASK_DEFINE
	#define _TASK_DEFINE
	#include "_linux_s.h"
	#include "../_opt.h"
	/*==============================立即数定义=====================================*/
	#define DF_TASK_STATE_OPER							10
	#define DF_TASK_STATE_SLEEP							(DF_TASK_STATE_OPER)
	
	
	
	
	
	#ifdef _TASK_C
		#define e_task
	#else
		#define e_task					extern
	#endif
	#ifdef _TASK_C
		#define out(s,arg...)					//log_out("all.c",s,##arg) 							
		#define m_sleep(s)							\
			{	int a;								\
				for(a=0;a<s;a++)					\
				{	usleep(1000);					\
				}									\
			}
	#endif

	/*===============================函数定义======================================*/
	e_task	void _task_load(struct _task_t		*task);
	e_task	int _task_open(char *s_name,void (*hook)(void *arg,int o_timer),
			void *arg,uint32 timer,void *par);
	e_task	void _task_week_up(int fd);
	e_task	void _task_close(int fd);
	e_task  void _task_close_self(int fd);
	#ifdef _TASK_C
		static void *task_hook(void *arg);
		static void *task_main(void *arg);
		static void timer_add(struct   timespec	*spec,uint32 t);
		static int tim_subtract1( struct timeval *x, struct timeval *y);		
	#endif	
			
	/*=============================全局变量定义===================================*/
	#ifdef _TASK_C
		static struct _task_t		*stream=NULL;
	#endif			
#endif


