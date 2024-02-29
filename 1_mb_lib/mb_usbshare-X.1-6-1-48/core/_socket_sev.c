#define _socket_sev_c
#include "_socket_sev.h"
int _socket_sev_add(uint16 port)
{	struct _socket_sev  *stream;
	char name[DF_FILE_NAME_MAX_LEN];
	int result;
	//out("���socket sev�ļ�\n");
    memset(name,0,sizeof(name));
	stream=malloc(sizeof(struct _socket_sev));
	if(stream==NULL)
		return DF_ERR_MEM_ERR;
	memset(stream,0,sizeof(struct _socket_sev));
	sprintf(name,"socket_sev/sever/%d",port);
	stream->port=port;
	result = ty_file_add(DF_DEV_TY_SOCKET_S, stream, name, (struct _file_fuc *)&socket_sev_fuc);
	if(result<0)
		free(stream);
	return result;
}

static int _socket_sev_open(struct ty_file	*file)
{   struct _socket_sev  *stream;
    struct sockaddr_in	add;
    const int val = 1;
    int j,ty_pd=1;
    //	out("���ļ�:socket_sev\n");
    stream=file->pro_data;
	if(stream==NULL)
		return DF_ERR_PORT_CORE_FD_NULL;
        
	if(stream->state==0)
    {   //out("����һ��������\n");
        stream->lock=_lock_open("_socket_sev.c",NULL);
        if(stream->lock<0)
        {   out("������ʧ��\n");
            return stream->lock;
        }
        //out("����һ���µ�socket�ķ���\n");
        stream->socketfd=socket(AF_INET,SOCK_STREAM,0);
        if(stream->socketfd<0)
        {   out("�����µ�socket����ʧ��\n");
            _lock_close(stream->lock);
            return stream->socketfd;
        }
        //out("�趨������,�����ܹ���\n");
        setsockopt(stream->socketfd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));
       // out("����tcpû����ʱ\n");
        setsockopt(stream->socketfd, IPPROTO_TCP, TCP_NODELAY, &val, sizeof(val));    
        memset(&add,0,sizeof(add));
        add.sin_family=AF_INET;
        add.sin_addr.s_addr=INADDR_ANY;
        add.sin_port=htons(stream->port);
        //out("�󶨶˿�=%d\n",stream->port);
        if(bind(stream->socketfd,(struct sockaddr	*)&add,sizeof(add))==-1)
        {	out("��ʧ��\n");
            close(stream->socketfd);
            _lock_close(stream->lock);
            return DF_ERR_PORT_CORE_SOCKET_SEV_BIND;
        }
        //out("�򿪼���\n");
        if(listen(stream->socketfd,DF_SOCKET_SEV_MAX_LINE)==-1)
        {	out("������ʧ��\n");
            close(stream->socketfd);
            _lock_close(stream->lock);
            return DF_ERR_PORT_CORE_SOCKET_LISTER;
        }
        stream->task_fd=_task_open("_socket_sev",_socket_sev_main_task,stream,DF_SOCKET_SEV_MAIN_TASK_TIMER,NULL);
        if(stream->task_fd<0)
        {   out("�����߳�ʧ��\n");
            close(stream->socketfd);
            _lock_close(stream->lock);
            return stream->task_fd;
        }
        ty_pd=0;
    }  
    for(j=0;j<sizeof(stream->fd)/sizeof(stream->fd[0]);j++)
	{	if(stream->fd[j].state==0)
			break;
	}
	if(j==sizeof(stream->fd)/sizeof(stream->fd[0]))
	{	return DF_ERR_PORT_CORE_SLAVE_SPACE;
	}
	memset(&stream->fd[j],0,sizeof(stream->fd[0]));
	stream->fd[j].dev=stream; 
	stream->fd[j].state=1; 
	stream->fd[j].mem_fd=&file->fd[j];
	stream->state++;	
    if(ty_pd==0)
    {  
		//out("���ļ���Ҫ����ػ��߳�\n");
        //char name[DF_FILE_NAME_MAX_LEN];
        //memset(name,0,sizeof(name));
        //sprintf(name,"socket_sev/sever/%d",stream->port);
        //ty_pd_add_file(name);
    }
    out("��socket����ɹ�\n");
    return 1;
}

static int _socket_sev_ctl(void *data,int fd,int cm,va_list args)
{   struct _socket_sev 	*stream;
    struct _socket_sev_fd   *id;
	stream=data;
	if((stream==NULL))	
		return DF_ERR_PORT_CORE_FD_NULL;
	if((fd==0)||(fd>sizeof(stream->fd)/sizeof(stream->fd[0])))
		return DF_ERR_PORT_CORE_FD_Z;
	fd--;
	id=&stream->fd[fd];
	if(id->state==0)
		return DF_ERR_PORT_CORE_SLAVE_DEV;
    if(cm==DF_SOCKET_SEV_CM_HOOK)
    {   //out("���ûص������ͻص�����\n");
        _lock_set(stream->lock);
        stream->arg=va_arg(args,void *);
		stream->hook=va_arg(args,void *);
        _lock_un(stream->lock);
        return 0;
    }
    else if(cm==DF_SOCKET_SEV_CM_CLOSE_TYPE)
    {   //out("���ùر�����\n");
        _lock_set(stream->lock);
        stream->close_type=va_arg(args,int);
        _lock_un(stream->lock);
        return stream->close_type;
    }
    else if(cm==DF_SOCKET_SEV_CM_OPEN_KEEP)
    {   //out("����TCP����\n");
        _lock_set(stream->lock);
        stream->open_keepaliver=va_arg(args,int);
        _lock_un(stream->lock);
        return stream->open_keepaliver;
    }
    else if(cm==DF_SOCKET_SEV_CM_NODELAY)
    {  // out("����С���ݲ��ȴ�\n");
        _lock_set(stream->lock);
        stream->nodelay=va_arg(args,int);
        _lock_un(stream->lock);
        return stream->nodelay;
    }
    else if(cm==DF_SOCKET_SEV_CM_REV_TIMER)
    {   //out("����ȫ�ֽ��ճ�ʱʱ��\n");
        _lock_set(stream->lock);
        stream->rec_timer_out=va_arg(args,int);
        _lock_un(stream->lock);
        return stream->rec_timer_out;
    }
    else if(cm==DF_SOCKET_SEV_CM_SEND_TIMER)
    {   //out("����ȫ�ַ��ͳ�ʱʱ��\n");
        _lock_set(stream->lock);
        stream->send_timer_out=va_arg(args,int);
        _lock_un(stream->lock);
        return stream->rec_timer_out;
    }
    else if(cm==DF_SOCKET_SEV_CM_LINE_NODELAY)
    {   int socketfd;
        //out("���õ���·СƬ�ȴ�\n");
        socketfd=va_arg(args,int);
        return open_tcp_nodelay(socketfd);
    }
    else if(cm==DF_SOCKET_SEV_CM_LINE_REC_O)
    {   int socketfd;
        int o_timer;
        socketfd=va_arg(args,int);
        o_timer=va_arg(args,int);
        return open_rec_over_timer(socketfd,o_timer);
    }
	else if(cm==DF_SOCKET_SEV_CM_LINE_SEND_O)
	{
		int socketfd;
		int o_timer;
		socketfd = va_arg(args, int);
		o_timer = va_arg(args, int);
		return open_send_over_timer(socketfd, o_timer);
	}
    //else if(cm==DF_TY_CORE_CM_PD)
    //{   //out("�ػ��̵߳���\n");
    //    _lock_set(stream->lock);
    //    _lock_un(stream->lock);
    //    return 0;
    //}
	else if(cm==DF_SOCKET_SEV_CM_LINE_CLOSE)
	{
		int sockfd;
		int i;
		//out("Ӧ�ò�����ǿ�ƹر�socket��·\n");
		sockfd=va_arg(args,int);
		//out("�ر�һ��socket��·\n");
		_lock_set(stream->lock);

		for(i=0;i<sizeof(stream->line)/sizeof(stream->line[0]);i++)
		{   if(stream->line[i].socketfd!=sockfd)
				continue;
			break;
		}
		if(i==sizeof(stream->line)/sizeof(stream->line[0]))
		{   _lock_un(stream->lock);  
			//out("û��������·\n");
			return -1;
		}
		shutdown(sockfd,SHUT_RDWR);
        close_line(&stream->line[i]);
        _lock_un(stream->lock);
		return 0;
	}
    
    return DF_ERR_PORT_CORE_CM;
}

static int _socket_sev_read(void *data,int fd,uint8 *buf,int buf_len)
{   struct _socket_sev 	*stream;
    struct _socket_sev_fd   *id;
	int i,k;
	stream=data;
	if((stream==NULL))	
		return DF_ERR_PORT_CORE_FD_NULL;
	if((fd==0)||(fd>sizeof(stream->fd)/sizeof(stream->fd[0])))
		return DF_ERR_PORT_CORE_FD_Z;
	fd--;
	id=&stream->fd[fd];
	if(id->state==0)
		return DF_ERR_PORT_CORE_SLAVE_DEV;
    out("��ȡ�Ѿ����ӵ�TCP��������\n");
    _lock_set(stream->lock);
    for(i=0,k=0;i<sizeof(stream->line)/sizeof(stream->line[0]);i++)
    {   if(stream->line[i].socketfd>0)
            k++;
    }
    _lock_un(stream->lock);
    out("��������=%d\n",k);
    return k;   
}

static int _sock_sev_close(void *data,int fd)
{   struct _socket_sev 	*stream;
    struct _socket_sev_fd 	*id;
	//int i,k;
	stream=data;
	if((stream==NULL))	
		return DF_ERR_PORT_CORE_FD_NULL;
	if((fd==0)||(fd>sizeof(stream->fd)/sizeof(stream->fd[0])))
		return DF_ERR_PORT_CORE_FD_Z;
	fd--;
	id=&stream->fd[fd];
	if(id->state==0)
		return DF_ERR_PORT_CORE_SLAVE_DEV;
	id->state=0;
	stream->state--;
    memset(id,0,sizeof(stream->fd[0]));
    if(stream->state==0)
    {   out("�رձ�ģ��\n");
        close(stream->socketfd);
        _task_close(stream->task_fd);
        for(;_socket_sev_read(data,fd,NULL,0)!=0;usleep(100000));
        _lock_close(stream->lock);
        stream->state=0;
        out("�رճɹ�\n");
    }    
    return 0;
}


static void _socket_sev_main_task(void *arg,int o_timer)
{   struct _socket_sev  *stream;
	struct sockaddr_in	new_s;
	int sock;
	int i;
	int len;
	stream=arg;
    for(;;)
    {   memset(&new_s,0,sizeof(new_s));
        len=sizeof(struct sockaddr_in);
		sock = accept(stream->socketfd, (struct sockaddr *)&new_s, (socklen_t *)&len);
        if(sock<0)
            return ;
        //out("��ȡ���µ�����\n");
        _lock_set(stream->lock);
        for(i=0;i<sizeof(stream->line)/sizeof(stream->line[0]);i++)
        {   if(stream->line[i].socketfd<=0)
                break;
        }
        if(i==sizeof(stream->line)/sizeof(stream->line[0]))
		{   out("�Ѿ�������Ŀǰ��������:[%d],�˿ں�Ϊ��%d��\n",sizeof(stream->line)/sizeof(stream->line[0]),stream->port);
            close(sock);
            _lock_un(stream->lock);
            continue;
        }
        stream->line[i].socketfd=sock;
        _lock_un(stream->lock);
        //out("��ȡ�ͻ��˵�IP��ַ�Լ��˿ں�\n");
        char ip_asc[100];
        memset(ip_asc,0,sizeof(ip_asc));
        inet_ntop(AF_INET,&new_s.sin_addr,ip_asc,sizeof(ip_asc));
        //out("ip��ַΪ:%s\n",ip_asc);
        stream->line[i].ip=ip_asc2ip(ip_asc,strlen(ip_asc));
        stream->line[i].port=htons(new_s.sin_port);
        //out("�˿ں�Ϊ:%d\n",stream->line[i].port);
        stream->line[i].stream=stream;
        if(stream->open_keepaliver)
            open_tcp_keepalive(sock);
        if(stream->nodelay)
            open_tcp_nodelay(sock);
        if(stream->rec_timer_out!=0)
        {   open_rec_over_timer(sock,stream->rec_timer_out);
        }
        if(stream->send_timer_out!=0)
        {   open_send_over_timer(sock,stream->send_timer_out);
        }
		if (stream->port == 10001)//���FIN_WAIT ���� �������޸� 20220519ͬ����Ʊ������
		{
		
			struct linger so_linger;
			so_linger.l_onoff = 1;
			so_linger.l_linger = 0;
			setsockopt(sock, SOL_SOCKET, SO_LINGER, &so_linger, sizeof so_linger);
		}

        if(_queue_add("0",_task_line,&stream->line[i],0)<0)
        {   out("��������ʧ��\n");
            close_line(&stream->line[i]);
        }
        continue;
    }
}



//static void _task_line(void *arg)
//{   struct  _socket_sev_line    *line;
//    struct _socket_sev          *stream;
//    uint8 byte[2];
//    int result;
//
//    line=arg;
//    stream=line->stream;
//    //out("ά����·����������\n");
//    if(stream->hook==NULL)
//    {   //out("��Ӧ�ó���,Ӧ�ر���·\n");
//        shutdown(line->socketfd,SHUT_RDWR);
//        close_line(line);
//        return ;    
//    }
//    if(stream->hook(line->socketfd,line->ip,line->port,stream->arg)<0)
//    {   //out("Ӧ�÷�������,�رո���·\n");
//        shutdown(line->socketfd,SHUT_RDWR);
//        close_line(line);
//        return ;
//    }
//    //if(stream->close_type==1)
//    //{   out("Ҫ��shutdown�رո���·\n");
//    //    shutdown(line->socketfd,SHUT_RDWR);
//    //}
//	result=read(line->socketfd,byte,1);
//	if(result == 0)
//	{
//		//out("�ͻ����ѹر���·��Ҫ��ɾ������·\n");
//		close_line(line); 
//		return ;
//	}
//	//out("�ͻ���δ�������ر�socket��Ҫ��close����·\n");
//	shutdown(line->socketfd,SHUT_RDWR);
//	close_line(line);  
//	return ;
//}








static void _task_line(void *arg)
{   struct  _socket_sev_line    *line;
    struct _socket_sev          *stream;
    uint8 byte=0;
    int result;
    line=arg;
    stream=line->stream;
    //out("ά����·����������\n");
    if(stream->hook==NULL)
    {   //out("��Ӧ�ó���,Ӧ�ر���·\n");
        shutdown(line->socketfd,SHUT_RDWR);
        close_line(line);
        return ;    
    }
    if(stream->hook(line->socketfd,line->ip,line->port,stream->arg)<0)
    {   //out("Ӧ�÷�������,ȫ���ر���·\n");
        shutdown(line->socketfd,SHUT_RDWR);
        close_line(line);
        return ;
    }
    if(stream->close_type==1)
    {   //out("Ҫ��ȫ���رո���·\n");
		result=read(line->socketfd,&byte,1);
		if(result == 0)
		{	//out("�ͻ����ѹر���·��Ҫ��ɾ������·\n");
			shutdown(line->socketfd,SHUT_RDWR);
			close_line(line); 
			return ;
		}  
		//out("��ѯ���ͻ��˳�ʱû�йر�����\n");
    }
	else if(stream->close_type==0)
	{	//out("3240ģʽ�¹ر�\n");
		//shutdown(line->socketfd,SHUT_RDWR);
		close_line(line);   
		return ;
	}
	shutdown(line->socketfd,SHUT_RDWR);
    close_line(line);
	return ;
}








static int open_rec_over_timer(int socketfd,int over_timer)
{   struct timeval timeout;
    //out("���ý��ܳ�ʱʱ��Ϊ:%d\n",over_timer);
    memset(&timeout,0,sizeof(timeout));
    timeout.tv_sec=over_timer;
    return setsockopt(socketfd,SOL_SOCKET,SO_RCVTIMEO,&timeout,sizeof(timeout));
}

static int open_send_over_timer(int socketfd,int over_timer)
{   struct timeval timeout;
    //out("���÷��ͳ�ʱʱ��Ϊ:%d\n",over_timer);
    memset(&timeout,0,sizeof(timeout));
    timeout.tv_sec=over_timer;
    return setsockopt(socketfd,SOL_SOCKET,SO_SNDTIMEO,&timeout,sizeof(timeout));
    
}



static int open_tcp_nodelay(int socketfd)
{   const int val = 1;
    return setsockopt(socketfd, IPPROTO_TCP, TCP_NODELAY, &val, sizeof(val));
    
}
static int open_tcp_keepalive(int socketfd)
{   const int keepAlive = 1;   
    const int keepIdle = 60;     
    const int keepInterval = 5;     
    const int keepCount = 2;     
    setsockopt(socketfd, SOL_SOCKET, SO_KEEPALIVE, (void*)&keepAlive, sizeof(keepAlive));  
    setsockopt(socketfd, SOL_TCP, TCP_KEEPIDLE, (void*)&keepIdle, sizeof(keepIdle));  
    setsockopt(socketfd, SOL_TCP, TCP_KEEPINTVL, (void*)&keepInterval, sizeof(keepInterval));  
    setsockopt(socketfd, SOL_TCP, TCP_KEEPCNT, (void*)&keepCount, sizeof(keepCount));   
    return 0;    
}

static int close_line(struct _socket_sev_line    *line)
{   struct _socket_sev  *stream;
    stream=line->stream;
    close(line->socketfd);
    _lock_set(stream->lock);  
    memset(line,0,sizeof(struct _socket_sev_line));   
    _lock_un(stream->lock);    
    return 0;
}













