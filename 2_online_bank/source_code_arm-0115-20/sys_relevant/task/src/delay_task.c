#include "../inc/delay_task.h"

struct _delay_task_t *q_data = NULL;

static void * delay_task_hook(void *arg);

void _delay_task_load(void)
{
	q_data = (struct _delay_task_t *)malloc(sizeof(struct _delay_task_t));
	memset(q_data,0,sizeof(struct _delay_task_t));
	q_data->lock=_lock_open("delay_task",NULL);
}

int _delay_task_add(char *name,void (*hook)(void *arg),void *arg,int timer)
{	struct _delay_task *delay_task = NULL;
	int i = 0;
	delay_task= (struct _delay_task *)malloc(sizeof(struct _delay_task));	
	if(delay_task == NULL)
		return DF_ERR_MEM_ERR;
	memset(delay_task,0,sizeof(struct _delay_task));
	strcpy(delay_task->name,name);
	delay_task->arg = arg;
	delay_task->delay = timer;
	delay_task->hook = hook;
	delay_task->delay_task_t = q_data;
    //_lock_set(q_data->lock);
	for(i=0;i<sizeof(q_data->delay_task)/sizeof(struct _delay_task*);++i)
	{
		if(q_data->delay_task[i] == NULL)
			break;
	}
	if(i == sizeof(q_data->delay_task)/sizeof(struct _delay_task *))
	{
		printf("_delay_task_add have no space error\n");
		free(delay_task);
		return DF_ERR_LINUX_FD;
	}
	_lock_set(q_data->lock);
	q_data->delay_task[i] = delay_task;
	delay_task->fd = i;
	_lock_un(q_data->lock);
	if(pthread_create(&delay_task->task,NULL,delay_task_hook,delay_task) != 0)
	{
		printf("_delay_task_add pthread_create error\n");
		free(delay_task);
		q_data->delay_task[i] = NULL;
		_lock_un(q_data->lock);
		return DF_ERR_LINUX_HARD;
	}
	return i+1;
}

static void *delay_task_hook(void *arg)
{
	struct _delay_task *delay_task = NULL;
	struct _delay_task_t	*data = NULL;
	int i = 0;
	delay_task = (struct _delay_task *)arg;
	pthread_detach(pthread_self());
	if(delay_task == NULL)
	{
		pthread_exit(0);
		return NULL;
	}
	for(i = 0;i < delay_task->delay;++i,usleep(1000));
	if(delay_task->hook != NULL)
		delay_task->hook(delay_task->arg);
	data = delay_task->delay_task_t;
	_lock_set(data->lock);	
	i = delay_task->fd;
	data->delay_task[delay_task->fd] = NULL; 
	free(delay_task);
	_lock_un(data->lock);
	pthread_exit(0);
}

