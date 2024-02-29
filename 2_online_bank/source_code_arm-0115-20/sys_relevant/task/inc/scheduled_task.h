#ifndef _SCHEDULED_TASK_H
#define _SCHEDULED_TASK_H

#include "opt.h"

#include "linux_s.h"

#define DF_LINUX_MAX_TASK_NUMBER	600			//�����������
#define SLEEP_PERIOD 				30000		//��������
#define DF_MAX_SCHEDULED_TASK_NAME_LEN 			32//ѭ������������󳤶�

#define TASK_SIZE_500K				500*1024

enum scheduled_task_run_state
{
	TASK_CLOSE = 0,  		//�ر�״̬
	TASK_OPEN,	   		//����״̬
	TASK_RUNING,				//��������
	TASK_SLEEPING				//����
};

enum scheduled_task_over_state
{
	TASK_NOT_NEED_OVER = 0,	//������Ҫ�ر�
	TASK_OVERING,			//�������ڹر�
	TASK_NEED_OVER			//������Ҫ�ر�
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
	void (*hook)(void *arg,int o_timer);		//�̻߳ص�
	void *arg;									//Ӧ�ó��������Ĳ���
	uint32 s_timer;
	pthread_t fd;								//�̵߳�id
	pthread_cond_t con;							//ʹ�ô����߳�
	pthread_mutex_t lock;						//�߳���
	struct timeval start_timer;					//��ʼʱ��
	struct timeval near_timer;					//�������ʱ��
	int timer_fd;								//ʱ��FD
	int over;									//�������
	int start;									//����ʼ
	int state;									//����״̬
	int number;									//���д���
	int over_ing;								//���ڹر�
};

struct _scheduled_task_t
{
	pthread_mutex_t	lock;												//��
	pthread_t fd;														//����������
	sem_t sem;															//�ź���
	struct _scheduled_task	scheduled_task[DF_LINUX_MAX_TASK_NUMBER];	//����
	int scheduled_task_len;
};

void _scheduled_task_load(void);
int _scheduled_task_open(char *s_name,void (*hook)(void *arg,int o_timer),void *arg,uint32 timer,void *par);
int _scheduled_task_open_set_size(char *s_name,void (*hook)(void *arg,int o_timer),void *arg,uint32 timer,void *par,size_t size);
void _scheduled_task_week_up(int fd);
void _scheduled_task_close(int fd);
void _scheduled_task_close_self(int fd);

#endif


