#ifndef _DELAY_TASK_H
#define _DELAY_TASK_H

#include "opt.h"

#include "linux_s.h"
#include "lock.h"
#include "scheduled_task.h"

#define delay_task_out(s,arg...)	   //		

#define DF_LINUX_MAX_DELAY_TASK_NUMBER					512		//最大延时任务数
#define DF_MAX_DELAY_TASK_NAME_LEN 						64		//延时任务名称最大长度

struct _delay_task_t
{
	int lock;
	struct _delay_task *delay_task[DF_LINUX_MAX_DELAY_TASK_NUMBER];
};

struct _delay_task
{
	char name[DF_MAX_DELAY_TASK_NAME_LEN];
	void (*hook)(void *arg);
	void *arg;
	int delay;
	pthread_t task;
	int fd;
	struct _delay_task_t *delay_task_t;
};

void _delay_task_load(void);
int _delay_task_add(char *name,void (*hook)(void *arg),void *arg,int timer);

#endif

