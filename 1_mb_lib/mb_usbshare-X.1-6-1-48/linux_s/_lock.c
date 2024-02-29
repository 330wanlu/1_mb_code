#define _LOCK_C
#include "_lock.h"
void _lock_load(struct _lock_t	*lock)
{	stream=lock;
	memset(stream,0,sizeof(struct _lock_t));
	if(stream->data==NULL)
	{	out("_lock_load err\n");
		return ;
	}
	stream->data_len=sizeof(stream->data)/sizeof(stream->data[0]); 
	pthread_mutex_init(&stream->lock,NULL);	
}

int _lock_open(char *s_name,void *par)
{	int fd;
	out("_lock_open\n");
	pthread_mutex_lock(&stream->lock);
	for(fd=0;fd<stream->data_len;fd++)
	{	if(stream->data[fd].state==0)
			break;
	}
	if(fd==stream->data_len)
	{	
		logout(INFO, "system", "������", "_lock_open  fd==stream->data_lenʧ��,�˳�����\r\n");
		exit(0);
		return -1;
	}	
	out("_lock_open:fd=%d\n",fd);
	memset(&stream->data[fd],0,sizeof(struct _lock));
	stream->data[fd].state=DF_LOCK_UN;
	pthread_mutex_unlock(&stream->lock);
	out("_lock_open:ok\n");
	if(pthread_mutex_init(&stream->data[fd].data,NULL)<0)
    {  
		logout(INFO, "system", "������", "_lock_open  pthread_mutex_initʧ��,�˳�����\r\n");
		exit(0);
		return -1;

    }
	return fd+1;
}

void _lock_close(int fd)
{	if((fd==0)||(fd>stream->data_len))
		return ;
	fd--;
	if(stream->data[fd].state==0)
		return ;	
	stream->data[fd].state=0; 
}

int _lock_set_no_dely(int fd)
{   if((fd==0)||(fd>stream->data_len))
	{	printf("[Ӧ�ó�������������]\n");
        for(;;);
        return DF_ERR_LINUX_FD;
    }    
	fd--;
    if(stream->data[fd].state==0)
	{	printf("[Ӧ�ó����ڴ淢������]\n");
        for(;;);
        return -1;   
    } 
    if(stream->data[fd].state==DF_LOCK_SET)
        return -1;
    pthread_mutex_lock(&stream->data[fd].data);
	stream->data[fd].state=DF_LOCK_SET; 
	return 0;   
}



int _lock_set(int fd)
{	if((fd==0)||(fd>stream->data_len))
	{	printf("[Ӧ�ó�������������]\n");
        for(;;);
        return DF_ERR_LINUX_FD;
    }    
	fd--;
	if(stream->data[fd].state==0)
	{	printf("[Ӧ�ó����ڴ淢������]\n");
        for(;;);
        return -1;   
    }    
	//for(;stream->data[fd].state==DF_LOCK_SET;usleep(100));
	pthread_mutex_lock(&stream->data[fd].data);
	stream->data[fd].state=DF_LOCK_SET; 
	return 0;
}

int	_lock_un(int fd)
{	if((fd==0)||(fd>stream->data_len))
	{	printf("[Ӧ�ó����ڴ淢������]1\n");
        return DF_ERR_LINUX_FD;
    }    
	fd--;
	if(stream->data[fd].state==0)
	{	printf("[Ӧ�ó����ڴ淢������]\n");
        for(;;);
        return -2;
    }    
	pthread_mutex_unlock(&stream->data[fd].data);
	stream->data[fd].state=DF_LOCK_UN;
	return 0;
}










