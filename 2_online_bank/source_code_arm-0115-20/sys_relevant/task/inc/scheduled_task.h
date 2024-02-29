#ifndef _SCHEDULED_TASK_H
#define _SCHEDULED_TASK_H

#include "opt.h"

#include "linux_s.h"

#define DF_LINUX_MAX_TASK_NUMBER	600			//任务最大数量
#define SLEEP_PERIOD 				30000		//休眠周期
#define DF_MAX_SCHEDULED_TASK_NAME_LEN 			32//循环任务名称最大长度

#define TASK_SIZE_500K				500*1024

enum scheduled_task_run_state
{
	TASK_CLOSE = 0,  		//关闭状态
	TASK_OPEN,	   		//开启状态
	TASK_RUNING,				//正在运行
	TASK_SLEEPING				//休眠
};

enum scheduled_task_over_state
{
	TASK_NOT_NEED_OVER = 0,	//任务不需要关闭
	TASK_OVERING,			//任务正在关闭
	TASK_NEED_OVER			//任务需要关闭
};

#define scheduled_task_out(s,arg...)					//log_out("all.c",s,##arg)
#define m_sleep(s)							\
			{	int a;								\
				for(a=0;a<s;a++)					\
				{									\
					usleep(1000);					\
				}									\
			}


struct _scheduled_task
{
	char name[DF_MAX_SCHEDULED_TASK_NAME_LEN];
	void (*hook)(void *arg,int o_timer);		//线程回调
	void *arg;									//应用程序所带的参数
	uint32 s_timer;
	pthread_t fd;								//线程的id
	pthread_cond_t con;							//使用触发线程
	pthread_mutex_t lock;						//线程锁
	struct timeval start_timer;					//开始时间
	struct timeval near_timer;					//最近运行时间
	int timer_fd;								//时间FD
	int over;									//任务结束
	int start;									//任务开始
	int state;									//运行状态
	int number;									//运行次数
	int over_ing;								//正在关闭
};

struct _scheduled_task_t
{
	pthread_mutex_t	lock;												//锁
	pthread_t fd;														//本任务运行
	sem_t sem;															//信号量
	struct _scheduled_task	scheduled_task[DF_LINUX_MAX_TASK_NUMBER];	//任务
	int scheduled_task_len;
};

void _scheduled_task_load(void);
int _scheduled_task_open(char *s_name,void (*hook)(void *arg,int o_timer),void *arg,uint32 timer,void *par);
int _scheduled_task_open_set_size(char *s_name,void (*hook)(void *arg,int o_timer),void *arg,uint32 timer,void *par,size_t size);
void _scheduled_task_week_up(int fd);
void _scheduled_task_close(int fd);
void _scheduled_task_close_self(int fd);

#endif


