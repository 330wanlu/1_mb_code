#include "../inc/scheduled_task.h"

static struct _scheduled_task_t *stream = NULL;

static void *scheduled_task_hook(void *arg);
static void *scheduled_task_main(void *arg);
static void timer_add(struct timespec *spec,uint32 t);
static int tim_subtract(struct timeval *x,struct timeval *y);

void _scheduled_task_load(void)
{
	stream = (struct _scheduled_task_t *)malloc(sizeof(struct _scheduled_task_t));
	memset(stream,0,sizeof(struct _scheduled_task_t));
	stream->scheduled_task_len = sizeof(stream->scheduled_task) / sizeof(stream->scheduled_task[0]);
	//clock_get(stream->timer);
	pthread_mutex_init(&stream->lock,NULL);
	sem_init(&stream->sem,0,0);
	pthread_create(&stream->fd,NULL,scheduled_task_main,stream);
}

int _scheduled_task_open(char *s_name,void (*hook)(void *arg,int o_timer),void *arg,uint32 timer,void *par)
{
	struct _scheduled_task *data = NULL;
	int fd = 0;
	pthread_mutex_lock(&stream->lock);
	for(fd=0;fd < stream->scheduled_task_len;fd++)
	{
		if(stream->scheduled_task[fd].state == TASK_CLOSE)
			break;
	}
	memset(&stream->scheduled_task[fd],0,sizeof(struct _scheduled_task));
	stream->scheduled_task[fd].state = TASK_OPEN;
	data = &stream->scheduled_task[fd];
	pthread_mutex_unlock(&stream->lock);
	data->hook = hook;
	data->arg = arg;
	data->s_timer = timer;
	memcpy(data->name,s_name,strlen(s_name) <= sizeof(data->name)?strlen(s_name):sizeof(data->name));
	pthread_mutex_init(&data->lock,NULL);
	pthread_cond_init(&data->con,NULL);
	if(pthread_create(&data->fd,NULL,scheduled_task_hook,data) < 0)
	{	
		pthread_mutex_lock(&stream->lock);
		data->state = TASK_CLOSE;
		pthread_mutex_unlock(&stream->lock);
		return DF_ERR_LINUX_HARD;
	}
	scheduled_task_out("scheduled_task suc\n");
	for(;data->start == 0;usleep(1000));
	return fd+1;
}

//开启一个任务堆栈大小为size的任务
int _scheduled_task_open_set_size(char *s_name,void (*hook)(void *arg,int o_timer),void *arg,uint32 timer,void *par,size_t size)
{
	struct _scheduled_task *data = NULL;
	int fd = 0;
	size_t stack_size = 0; //堆栈大小变量
    pthread_attr_t attr; //线程属性结构体变量
	 //初始化线程属性
    int ret = pthread_attr_init(&attr);
    if(ret != 0)
    {
        perror("pthread_attr_init");
        return -1;
    }
    stack_size = size; //堆栈大小设置
    ret = pthread_attr_setstacksize(&attr, stack_size);//设置线程堆栈大小
    if(ret != 0)
    {
        perror("pthread_attr_getstacksize");
        return -1;
    }
	pthread_mutex_lock(&stream->lock);
	for(fd=0;fd < stream->scheduled_task_len;fd++)
	{
		if(stream->scheduled_task[fd].state == TASK_CLOSE)
			break;
	}
	memset(&stream->scheduled_task[fd],0,sizeof(struct _scheduled_task));
	stream->scheduled_task[fd].state = TASK_OPEN;
	data = &stream->scheduled_task[fd];
	pthread_mutex_unlock(&stream->lock);
	data->hook = hook;
	data->arg = arg;
	data->s_timer = timer;
	memcpy(data->name,s_name,strlen(s_name) <= sizeof(data->name)?strlen(s_name):sizeof(data->name));
	pthread_mutex_init(&data->lock,NULL);
	pthread_cond_init(&data->con,NULL);
	if(pthread_create(&data->fd,&attr,scheduled_task_hook,data) < 0)
	{	
		pthread_mutex_lock(&stream->lock);
		data->state = TASK_CLOSE;
		pthread_mutex_unlock(&stream->lock);
		return DF_ERR_LINUX_HARD;
	}
	scheduled_task_out("scheduled_task suc\n");
	for(;data->start == 0;usleep(1000));
	return fd+1;
}

void _scheduled_task_close(int fd)
{
	if((fd <= 0)||(fd > stream->scheduled_task_len))
		return;
	fd--;
	if(stream->scheduled_task[fd].state == TASK_CLOSE)
		return;
	stream->scheduled_task[fd].over = TASK_NEED_OVER;
	_scheduled_task_week_up(fd+1);
	pthread_join(stream->scheduled_task[fd].fd,NULL);
	for(;stream->scheduled_task[fd].over == TASK_NEED_OVER;)
	{
		m_sleep(1);
	}
	stream->scheduled_task[fd].state = TASK_CLOSE;
	return ;
}

void _scheduled_task_week_up(int fd)
{
	if((fd == 0)||(fd > stream->scheduled_task_len))
		return;
	fd--;
	if(stream->scheduled_task[fd].state == TASK_CLOSE)
		return;
	pthread_mutex_lock(&stream->scheduled_task[fd].lock);
	pthread_cond_signal(&stream->scheduled_task[fd].con);	
	pthread_mutex_unlock(&stream->scheduled_task[fd].lock);
}

void _scheduled_task_close_self(int fd)
{
	if((fd == 0)||(fd > stream->scheduled_task_len))
		return;
	fd--;
	if(stream->scheduled_task[fd].state == TASK_CLOSE)
		return;
	stream->scheduled_task[fd].over = TASK_NEED_OVER;
	stream->scheduled_task[fd].over_ing = TASK_OVERING;
	sem_post(&stream->sem);
}

//根据信号量来运行
static void *scheduled_task_main(void *arg)
{
	struct _scheduled_task_t *data;
	int i;
	data = (struct _scheduled_task_t *)arg;
	if(data == NULL)
		return NULL;
	for(;;)
	{
		if(sem_wait(&data->sem) != 0)
		{
			m_sleep(10);
			continue;
		}
		for(i=0;i < data->scheduled_task_len;++i)
		{
			if(data->scheduled_task[i].over_ing == TASK_OVERING)
			{
				_scheduled_task_close(i+1);
				break;
			}
		}
	}
}

static void *scheduled_task_hook(void *arg)
{
	struct _scheduled_task *data = NULL;
	struct timeval stop;
	struct timespec spec;
	uint32 timer = 0,tmp = 0;

	pthread_detach(pthread_self());
	data = (struct _scheduled_task *)arg;
	data->start = 1;

	gettimeofday(&data->start_timer,0);
	gettimeofday(&data->near_timer,0);
	for(;;)
	{
		if(data != NULL)
		{
			data->state = TASK_RUNING;
			if(data->over != TASK_NOT_NEED_OVER)			//退出线程
			{
				data->over = TASK_NOT_NEED_OVER;
				pthread_exit(0);
				break;
			}
			pthread_mutex_lock(&data->lock);
			gettimeofday(&stop,0);
			timer=tim_subtract(&data->start_timer,&stop);
			memcpy(&data->near_timer,&stop,sizeof(struct timeval));
			if(data->s_timer)
			{
				tmp = data->s_timer;
				scheduled_task_out("线程休眠时间 %d\n",tmp);
				for(;tmp >= SLEEP_PERIOD;)
				{
					scheduled_task_out("剩余线程休眠时间 %d\n",tmp);
					scheduled_task_out("timer add1 \n");
					timer_add(&spec,SLEEP_PERIOD);
					scheduled_task_out("cond_timedwait1 \n");
					data->state = TASK_SLEEPING;
					pthread_cond_timedwait(&data->con,&data->lock,&spec);
					tmp = tmp - SLEEP_PERIOD;
				}
				if(tmp > 0)
				{
					scheduled_task_out("最后线程休眠时间 %d\n",tmp);
					scheduled_task_out("timer add2 \n");
					timer_add(&spec,tmp);
					scheduled_task_out("cond_timedwait2 \n");
					data->state=TASK_SLEEPING;
					pthread_cond_timedwait(&data->con,&data->lock,&spec);
				}
			}
			else
			{
				pthread_cond_wait(&data->con,&data->lock);
			}
			scheduled_task_out("unlock \n");
			pthread_mutex_unlock(&data->lock);
		}
		if((data != NULL)&&(data->hook != NULL))
		{
			data->hook(data->arg,timer);
		}
	}
}

static void timer_add(struct timespec *spec,uint32 t)
{
	struct timeb tp;		
	int a = 0,b = 0;
	ftime(&tp);
	tp.millitm += t;
	a = tp.millitm/1000;
	b = tp.millitm%1000;
	memset(spec,0,sizeof(struct timespec));
	spec->tv_sec += (tp.time+a);
	spec->tv_nsec += (b*1000000);
}

static int tim_subtract(struct timeval *x,struct timeval *y)
{
	struct timeval result;
	int val = 0;
	//int nsec;
	if(x->tv_sec > y->tv_sec)
		return -1;
	if((x->tv_sec==y->tv_sec)&&(x->tv_usec>y->tv_usec))
		return -1;
	result.tv_sec = (y->tv_sec-x->tv_sec);
	result.tv_usec = (y->tv_usec-x->tv_usec);
	if(result.tv_usec<0)
	{
		result.tv_sec--;
		result.tv_usec+=1000000;
	}
	val=result.tv_sec*1000;
	val=result.tv_usec/1000+val;
	return (val >= 0 ? val : 0);
}

