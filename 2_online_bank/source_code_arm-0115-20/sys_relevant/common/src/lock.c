#include "../inc/lock.h"

static struct _lock_t *stream = NULL;

void _lock_load(void)
{
	stream = (struct _lock_t *)malloc(sizeof(struct _lock_t));;
	memset(stream,0,sizeof(struct _lock_t));
	if(stream->data == NULL)
	{
		lock_out("_lock_load err\n");
		return;
	}
	stream->data_len = sizeof(stream->data) / sizeof(stream->data[0]);
	pthread_mutex_init(&stream->lock,NULL);
}

int _lock_open(char *s_name,void *par)
{
	int fd = 0;
	lock_out("_lock_open\n");
	pthread_mutex_lock(&stream->lock);
	for(fd = 0;fd < stream->data_len;++fd)
	{
		if(stream->data[fd].state == 0)
			break;
	}
	if(fd == stream->data_len)
	{
		lock_out(INFO, "system", "互斥锁", "_lock_open  fd==stream->data_len失败,退出程序\r\n");
		exit(0);
		return -1;
	}
	lock_out("_lock_open:fd=%d\n",fd);
	memset(&stream->data[fd],0,sizeof(struct _lock));
	stream->data[fd].state=DF_LOCK_UN;
	pthread_mutex_unlock(&stream->lock);
	lock_out("_lock_open:ok\n");
	if(pthread_mutex_init(&stream->data[fd].data,NULL) < 0)
    {
		lock_out(INFO, "system", "互斥锁", "_lock_open  pthread_mutex_init失败,退出程序\r\n");
		exit(0);
		return -1;
    }
	return fd + 1;
}

void _lock_close(int fd)
{
	if((fd == 0)||(fd > stream->data_len))
		return ;
	fd--;
	if(stream->data[fd].state == 0)
		return;
	stream->data[fd].state = 0;
}

int _lock_set_no_dely(int fd)
{
	if((fd == 0)||(fd > stream->data_len))
	{
		lock_out("[应用程序发生致命错误]\n");
        for(;;);
        return DF_ERR_LINUX_FD;
    }
	fd--;
    if(stream->data[fd].state == 0)
	{
		lock_out("[应用程序内存发生问题]\n");
        for(;;);
        return -1;
    }
    if(stream->data[fd].state == DF_LOCK_SET)
        return -1;
    pthread_mutex_lock(&stream->data[fd].data);
	stream->data[fd].state = DF_LOCK_SET;
	return 0;
}

int _lock_set(int fd)
{
	if((fd == 0)||(fd > stream->data_len))
	{
		lock_out("[应用程序发生致命错误]\n");
        for(;;);
        return DF_ERR_LINUX_FD;
    }
	fd--;
	if(stream->data[fd].state==0)
	{
		lock_out("[应用程序内存发生问题]\n");
        for(;;);
        return -1;
    }
	//for(;stream->data[fd].state==DF_LOCK_SET;usleep(100));
	pthread_mutex_lock(&stream->data[fd].data);
	stream->data[fd].state = DF_LOCK_SET;
	return 0;
}

int _lock_un(int fd)
{
	if((fd == 0)||(fd > stream->data_len))
	{
		lock_out("[应用程序内存发生问题]1\n");
        return DF_ERR_LINUX_FD;
    }
	fd--;
	if(stream->data[fd].state == 0)
	{
		lock_out("[应用程序内存发生问题]\n");
        for(;;);
        return -2;
    }
	pthread_mutex_unlock(&stream->data[fd].data);
	stream->data[fd].state = DF_LOCK_UN;
	return 0;
}
