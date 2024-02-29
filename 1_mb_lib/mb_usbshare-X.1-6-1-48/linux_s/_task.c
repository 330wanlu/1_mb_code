#define _TASK_C
#include "_task.h"
void _task_load(struct _task_t		*task)
{	stream=task;
	memset(stream,0,sizeof(struct _task_t));
	stream->task_len=sizeof(stream->task)/sizeof(stream->task[0]); 
	//clock_get(stream->timer);
	pthread_mutex_init(&stream->lock,NULL);
	sem_init(&stream->sem,0,0);
	pthread_create(&stream->fd,NULL,task_main,stream);
}

int _task_open(char *s_name,void (*hook)(void *arg,int o_timer),void *arg,uint32 timer,void *par)
{	struct _task	*data;
	//struct _task	*task,**ptask;
	int fd;
	pthread_mutex_lock(&stream->lock);
	for(fd=0;fd<stream->task_len;fd++)
	{	if(stream->task[fd].state==0)
			break;
	}
	memset(&stream->task[fd],0,sizeof(struct _task));
	stream->task[fd].state=1;
	data=&stream->task[fd];
	pthread_mutex_unlock(&stream->lock);
	data->hook=hook;
	data->arg=arg;
	data->s_timer=timer;
	#if 0
    len=strlen(s_name);
	if(len>sizeof(data->name))
		len=sizeof(data->name);
	memcpy(data->name,s_name,len);
    #endif
	pthread_mutex_init(&data->lock,NULL);
	pthread_cond_init(&data->con,NULL);
	if(pthread_create(&data->fd,NULL,task_hook,data)<0)
	{	pthread_mutex_lock(&stream->lock);
		data->state=0;
		pthread_mutex_unlock(&stream->lock);
		return DF_ERR_LINUX_HARD;
	}
	out("task suc\n");
	for(;data->start==0;usleep(1000));
	return fd+1;	
}

void _task_close(int fd)
{	if((fd==0)||(fd>stream->task_len))
		return ;
	fd--;
	if(stream->task[fd].state==0)
		return ;
	stream->task[fd].over=1;
	_task_week_up(fd+1);
	pthread_join(stream->task[fd].fd,NULL);
	for(;stream->task[fd].over==1;)
	{	m_sleep(1);
		
	}
	stream->task[fd].state=0;
	return ;
}

void _task_week_up(int fd)
{	if((fd==0)||(fd>stream->task_len))
		return ;
	fd--;
	if(stream->task[fd].state==0)
		return ;
	pthread_mutex_lock(&stream->task[fd].lock);
	pthread_cond_signal(&stream->task[fd].con);	
	pthread_mutex_unlock(&stream->task[fd].lock);
}

void _task_close_self(int fd)
{	if((fd==0)||(fd>stream->task_len))
		return ;
	fd--;
	if(stream->task[fd].state==0)
		return ;
	stream->task[fd].over=1; 
	stream->task[fd].over_ing=1;
	sem_post(&stream->sem);
}

static void *task_main(void *arg)					//根据信号量来运行
{	struct _task_t		*data;
	int i;
	data=arg;
	if(data==NULL)
		return NULL;
	for(;;)
	{	if(sem_wait(&data->sem)!=0)
		{	m_sleep(10);
			continue;
		}
		for(i=0;i<data->task_len;i++)
		{	if(data->task[i].over_ing!=0)
			{	_task_close(i+1);
				break;
			}
		}
	}	
}

static void *task_hook(void *arg)
{	struct _task	*data;
	struct   timeval	stop;
	struct   timespec   spec;
	uint32 timer=0;
	uint32 tmp;
	
	pthread_detach(pthread_self());
	data=arg;
	data->start=1;
	
	gettimeofday(&data->start_timer,0);
	gettimeofday(&data->near_timer,0);
	for(;;)
	{	if(data!=NULL)
		{	data->state=DF_TASK_STATE_OPER;
			if(data->over!=0)			//退出线程
			{	data->over=0;
				pthread_exit(0);
				break;
			}
			pthread_mutex_lock(&data->lock);
			gettimeofday(&stop,0);
			timer=tim_subtract1(&data->start_timer,&stop);
			memcpy(&data->near_timer,&stop,sizeof(struct timeval));
			if(timer<0)
				timer=0;
			if(data->s_timer!=0)
			{	tmp = data->s_timer;
				out("线程休眠时间 %d\n",tmp);
				for(;tmp>=30000;)
				{	out("剩余线程休眠时间 %d\n",tmp);
					out("timer add1 \n");
					timer_add(&spec,30000);
					out("cond_timedwait1 \n");
					data->state=DF_TASK_STATE_SLEEP;
					pthread_cond_timedwait(&data->con,&data->lock,&spec);
					tmp = tmp-30000;
				}
				if(tmp>0)
				{	out("最后线程休眠时间 %d\n",tmp);
					out("timer add2 \n");
					timer_add(&spec,tmp);
					out("cond_timedwait2 \n");
					data->state=DF_TASK_STATE_SLEEP;
					pthread_cond_timedwait(&data->con,&data->lock,&spec);
				}
			}
			else
			{	pthread_cond_wait(&data->con,&data->lock);

			}
			out("unlock \n");
			pthread_mutex_unlock(&data->lock);
		}
		if((data!=NULL)&&(data->hook!=NULL))
			data->hook(data->arg,timer);
	}
}


//static void task_hook(void *arg)
//{	struct _task	*data;
//	struct   timeval	stop;
//	struct   timespec   spec;
//	uint32 timer=0;
//	pthread_detach(pthread_self());
//	data=arg;
//	data->start=1;
//	gettimeofday(&data->start_timer,0);
//	gettimeofday(&data->near_timer,0);
//	for(;;)
//	{	if(data!=NULL)
//		{	data->state=DF_TASK_STATE_OPER;
//			if(data->over!=0)			//退出线程
//			{	data->over=0;
//				pthread_exit(0);
//				break;
//			}
//			pthread_mutex_lock(&data->lock);
//			gettimeofday(&stop,0);
//			timer=tim_subtract1(&data->start_timer,&stop);
//			memcpy(&data->near_timer,&stop,sizeof(struct timeval));
//			if(timer<0)
//				timer=0;
//			if(data->s_timer!=0)
//			{	out("timer add \n");
//				timer_add(&spec,data->s_timer);
//				out("cond_timedwait \n");
//				data->state=DF_TASK_STATE_SLEEP;
//				pthread_cond_timedwait(&data->con,&data->lock,&spec);	
//			}
//			else
//			{	pthread_cond_wait(&data->con,&data->lock);
//
//			}
//			out("unlock \n");
//			pthread_mutex_unlock(&data->lock);
//		}
//		if((data!=NULL)&&(data->hook!=NULL))
//			data->hook(data->arg,timer);
//	}
//}

static void timer_add(struct   timespec	*spec,uint32 t)
{	struct timeb tp;			
	int a,b;
	ftime(&tp);
	tp.millitm+=t;
	a=tp.millitm/1000;
	b=tp.millitm%1000;
	memset(spec,0,sizeof(struct timespec));
	spec->tv_sec+=(tp.time+a);
	spec->tv_nsec+=(b*1000000);	
}

static int tim_subtract1( struct timeval *x, struct timeval *y)
{	struct timeval result;
	int val;
	//int nsec;
	if ( x->tv_sec > y->tv_sec )
		return   -1;
	if ((x->tv_sec==y->tv_sec) && (x->tv_usec>y->tv_usec))
		return   -1;
	result.tv_sec = ( y->tv_sec-x->tv_sec );
	result.tv_usec = ( y->tv_usec-x->tv_usec );
	if (result.tv_usec<0)
	{	result.tv_sec--;
		result.tv_usec+=1000000;
	}
	val=result.tv_sec*1000;
	val=result.tv_usec/1000+val;
	return val;	
}

