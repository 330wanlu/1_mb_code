#ifndef _LINUX_S_DEFINE
	#define _LINUX_S_DEFINE
	/*===========================include file=======================================*/
	#include "../_opt.h"
	#include <signal.h> 
	#include <fcntl.h>
	#include <sys/time.h>
	#include <sys/timeb.h>
	#include <time.h>
	#include <pthread.h>
	#include <semaphore.h>
	#include <dlfcn.h>
#ifndef RELEASE_SO
	#include "_timer.h"
#endif
	/*============================结构体定义========================================*/
/*===============================linux 参数设置====================================*/
#define DF_LINUX_MAX_TASK_NUMBER								600
#define DF_LINUX_MAX_LOCK_NUMBER								2000
#define DF_LINUX_MAX_QUEUE_NUMBER								1000	
/*==================================文件相关=======================================*/
#define DF_FILE_NAME_MAX_LEN									100	//文件名最大数量
/*====================================串口类=======================================*/
#define DF_SERIA_A_BUF_MAX_LEN								4096 //串口接收缓存数据		

/*=============================命令操作==================================*/
#define DF_COM_MAX_LEN					1460
/*----------------------------动作类-----------------------------*/
#define DF_ORDER_ACTION_SOFT_INFOR		100
#define DF_ORDER_ACTION_SOFT_ERR		(DF_ORDER_ACTION_SOFT_INFOR+1)
#define DF_ORDER_ACTION_SOFT_LOG		(DF_ORDER_ACTION_SOFT_ERR+1)
#define DF_ORDER_ACTION_READ_BUF		(DF_ORDER_ACTION_SOFT_LOG+1)
#define DF_ORDER_ACTION_WRITE_BUF		(DF_ORDER_ACTION_READ_BUF+1)
#define DF_ORDER_ACTION_HOOK			(DF_ORDER_ACTION_WRITE_BUF+1)
#define DF_ORDER_ACTION_STATE			(DF_ORDER_ACTION_HOOK+1)

/*----------------------------全局内核错误---------------------------------*/
#define DF_ERR_LINUX_HARD				-100000000				//内核错误
#define DF_ERR_MEM_ERR					(DF_ERR_LINUX_HARD-1)	//内存分配错误
#define DF_ERR_LINUX_FD					(DF_ERR_MEM_ERR-1)		//FD 不合法
#define DF_ERR_NO_INIT					(DF_ERR_LINUX_FD-1)		//没有初始化
#define DF_ERR_NO_SO					(DF_ERR_NO_INIT-1)		//该文件不存在

/*------------------------------错误代码-----------------------------------*/
#define DF_ERR_PORT_CORE_START	-200000000						//错误代码开始
#define DF_ERR_FILE				(DF_ERR_PORT_CORE_START-1000)	//文件的错误代码开始
#define DF_ERR_BSP_START		(DF_ERR_FILE-1000)				//bsp错误代码开始
#define DF_ERR_CLASS_START		(DF_ERR_BSP_START-1000)			//class错误代码开始	
#define DF_ERR_USB_IP			(DF_ERR_CLASS_START-1000)		//USB IP错误代码

	struct _queue_t
	{	
		int lock;
		struct _queue	*queue[DF_LINUX_MAX_QUEUE_NUMBER];
	};
	
	struct _queue
	{	
		void (*hook)(void *arg);
		void *arg;
		int delay;
		pthread_t task;
		int fd;
		struct _queue_t		*queue_t;
	};	
	
	struct _task
	{	
		void (*hook)(void *arg,int o_timer);		//线程回调
		void *arg;									//应用程序所带的参数
		uint32 s_timer;
		pthread_t		fd;							//线程的id
		pthread_cond_t	con;						//使用触发线程
		pthread_mutex_t lock;						//线程锁
		struct   timeval	start_timer;			//开始时间
		struct 	 timeval	near_timer;				//最近运行时间
		int timer_fd;								//时间FD
		int over;									//任务结束
		int start;									//任务开始
		int state;									//运行状态
		int number;									//运行次数
		int over_ing;								//正在关闭
	};
	
	struct _task_t
	{	
		pthread_mutex_t	lock;						//锁
		pthread_t		fd;							//本任务运行
		sem_t			sem;						//信号量
		struct _task	task[DF_LINUX_MAX_TASK_NUMBER];						//任务
		int task_len;
	};
	
	
	
	struct _lock
	{	
		pthread_mutex_t		data;					//锁
		int state;									//状态
	};
	
	struct _lock_t
	{	
		pthread_mutex_t	lock;	
		struct _lock	data[DF_LINUX_MAX_LOCK_NUMBER];
		int data_len;
	};	
	
	struct _linux_s
	{	struct _task_t	task;
		struct _lock_t	lock;
		struct _queue_t	queue;
	};


#endif

