#define _queue_c
#include "_queue.h"
void _queue_load(struct _queue_t	*queue_t)
{	q_data=queue_t;
	memset(q_data,0,sizeof(struct _queue_t));
	q_data->lock=_lock_open("queue",NULL);
}

int _queue_add(char *name,void (*hook)(void *arg),void *arg,int timer)
{	struct _queue	*queue;
	int i;
	queue=malloc(sizeof(struct _queue));	
	if(queue==NULL)
		return DF_ERR_MEM_ERR;
	memset(queue,0,sizeof(struct _queue));
	queue->arg=arg;
	queue->delay=timer;
	queue->hook=hook;
	queue->queue_t=q_data;
    //_lock_set(q_data->lock);
	for(i=0;i<sizeof(q_data->queue)/sizeof(struct _queue	*);i++)
	{	if(q_data->queue[i]==NULL)
			break;
	}
	if(i==sizeof(q_data->queue)/sizeof(struct _queue	*))
	{
		printf("_queue_add have no space error\n");
		free(queue);
		//_lock_un(q_data->lock);
		return DF_ERR_LINUX_FD;
	}
	_lock_set(q_data->lock);
	q_data->queue[i]=queue;
	queue->fd=i;
	_lock_un(q_data->lock);
	//log_out("","创建队列:%d\n",i);
	if(pthread_create(&queue->task,NULL,queue_hook,queue)!=0)
	{	
		printf("_queue_add pthread_create error\n");
		free(queue);
		q_data->queue[i]=NULL;
		_lock_un(q_data->lock);
		return DF_ERR_LINUX_HARD;
	}
	
	return i+1;
}

static void * queue_hook(void *arg)
{	struct _queue	*queue;
	struct _queue_t	*data;
	int i;
	queue=arg;
	pthread_detach(pthread_self());
	if(queue==NULL)
	{	pthread_exit(0);
		return NULL;
	}
	for(i=0;i<queue->delay;i++,usleep(1000))
		;
	if(queue->hook!=NULL)
		queue->hook(queue->arg);
	data=queue->queue_t;
	_lock_set(data->lock);	
	i=queue->fd;
	data->queue[queue->fd]=NULL; 
	free(queue);
	_lock_un(data->lock);
	pthread_exit(0);
}

