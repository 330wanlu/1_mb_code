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
	/*============================�ṹ�嶨��========================================*/
/*===============================linux ��������====================================*/
#define DF_LINUX_MAX_TASK_NUMBER								600
#define DF_LINUX_MAX_LOCK_NUMBER								2000
#define DF_LINUX_MAX_QUEUE_NUMBER								1000	
/*==================================�ļ����=======================================*/
#define DF_FILE_NAME_MAX_LEN									100	//�ļ����������
/*====================================������=======================================*/
#define DF_SERIA_A_BUF_MAX_LEN								4096 //���ڽ��ջ�������		

/*=============================�������==================================*/
#define DF_COM_MAX_LEN					1460
/*----------------------------������-----------------------------*/
#define DF_ORDER_ACTION_SOFT_INFOR		100
#define DF_ORDER_ACTION_SOFT_ERR		(DF_ORDER_ACTION_SOFT_INFOR+1)
#define DF_ORDER_ACTION_SOFT_LOG		(DF_ORDER_ACTION_SOFT_ERR+1)
#define DF_ORDER_ACTION_READ_BUF		(DF_ORDER_ACTION_SOFT_LOG+1)
#define DF_ORDER_ACTION_WRITE_BUF		(DF_ORDER_ACTION_READ_BUF+1)
#define DF_ORDER_ACTION_HOOK			(DF_ORDER_ACTION_WRITE_BUF+1)
#define DF_ORDER_ACTION_STATE			(DF_ORDER_ACTION_HOOK+1)

/*----------------------------ȫ���ں˴���---------------------------------*/
#define DF_ERR_LINUX_HARD				-100000000				//�ں˴���
#define DF_ERR_MEM_ERR					(DF_ERR_LINUX_HARD-1)	//�ڴ�������
#define DF_ERR_LINUX_FD					(DF_ERR_MEM_ERR-1)		//FD ���Ϸ�
#define DF_ERR_NO_INIT					(DF_ERR_LINUX_FD-1)		//û�г�ʼ��
#define DF_ERR_NO_SO					(DF_ERR_NO_INIT-1)		//���ļ�������

/*------------------------------�������-----------------------------------*/
#define DF_ERR_PORT_CORE_START	-200000000						//������뿪ʼ
#define DF_ERR_FILE				(DF_ERR_PORT_CORE_START-1000)	//�ļ��Ĵ�����뿪ʼ
#define DF_ERR_BSP_START		(DF_ERR_FILE-1000)				//bsp������뿪ʼ
#define DF_ERR_CLASS_START		(DF_ERR_BSP_START-1000)			//class������뿪ʼ	
#define DF_ERR_USB_IP			(DF_ERR_CLASS_START-1000)		//USB IP�������

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
		void (*hook)(void *arg,int o_timer);		//�̻߳ص�
		void *arg;									//Ӧ�ó��������Ĳ���
		uint32 s_timer;
		pthread_t		fd;							//�̵߳�id
		pthread_cond_t	con;						//ʹ�ô����߳�
		pthread_mutex_t lock;						//�߳���
		struct   timeval	start_timer;			//��ʼʱ��
		struct 	 timeval	near_timer;				//�������ʱ��
		int timer_fd;								//ʱ��FD
		int over;									//�������
		int start;									//����ʼ
		int state;									//����״̬
		int number;									//���д���
		int over_ing;								//���ڹر�
	};
	
	struct _task_t
	{	
		pthread_mutex_t	lock;						//��
		pthread_t		fd;							//����������
		sem_t			sem;						//�ź���
		struct _task	task[DF_LINUX_MAX_TASK_NUMBER];						//����
		int task_len;
	};
	
	
	
	struct _lock
	{	
		pthread_mutex_t		data;					//��
		int state;									//״̬
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

