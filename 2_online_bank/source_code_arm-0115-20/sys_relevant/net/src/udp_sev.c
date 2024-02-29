#include "../inc/udp_sev.h"

static int _udp_sev_open(struct ty_file	*file);
static int _udp_sev_ctl(void *data,int fd,int cm,va_list args);
static int _udp_sev_read(void *data,int fd,uint8 *buf,int buf_len);
static int _udp_sev_close(void *data,int fd);
//static void _udp_sev_main_task(void *arg,int o_timer);
//static int open_tcp_nodelay(int socketfd);
//static int open_tcp_keepalive(int socketfd);
//static int open_rec_over_timer(int socketfd,int over_timer);
// static int open_send_over_timer(int socketfd,int over_timer);
//static int close_line(struct _udp_sev_line    *line);
//static void _task_line(void *arg);
static int _udp_sev_write(void *data,int fd,uint8 *buf,int buf_len);

static const struct _file_fuc	udp_sev_fuc=
{
    .open=_udp_sev_open,
    .read=_udp_sev_read,
    .write=_udp_sev_write,
    .ctl=_udp_sev_ctl,
    .close=_udp_sev_close,
    .del=NULL
};

int _udp_sev_add(uint16 port)
{
    struct _udp_sev  *stream;
	char name[DF_FILE_NAME_MAX_LEN];
	int result;
	//udp_sev_out("添加socket sev文件\n");
    memset(name,0,sizeof(name));
	stream=malloc(sizeof(struct _udp_sev));
	if(stream==NULL)
		return DF_ERR_MEM_ERR;
	memset(stream,0,sizeof(struct _udp_sev));
	sprintf(name,"udp_sev/%d",port);
	stream->port=port;
	result = ty_file_add(DF_DEV_MB_UDP_S, stream, name, (struct _file_fuc *)&udp_sev_fuc);
	if(result<0)
		free(stream);
	return result;
}

static int _udp_sev_open(struct ty_file	*file)
{   struct _udp_sev  *stream;
    struct sockaddr_in	add;
    //const int val = 1;
    int j,ty_pd=1;
    //udp_sev_out("打开文件:socket_sev\n");
    stream=file->pro_data;
	if(stream==NULL)
		return DF_ERR_PORT_CORE_FD_NULL;
        
	if(stream->state==0)
    {   //udp_sev_out("申请一个数据锁\n");
        stream->lock=_lock_open("_udp_sev.c",NULL);
        if(stream->lock<0)
        {   udp_sev_out("申请锁失败\n");
            return stream->lock;
        }
        //udp_sev_out("申请一个新的socket的服务\n");
        stream->socketfd=socket(AF_INET,SOCK_DGRAM,0);
        if(stream->socketfd<0)
        {   udp_sev_out("申请新的socket服务失败\n");
            _lock_close(stream->lock);
            return stream->socketfd;
        }	
        bzero(&add,sizeof(add));  
        add.sin_family=AF_INET;
        add.sin_addr.s_addr=htonl (INADDR_ANY);
        add.sin_port=htons(stream->port);
		//udp_sev_out("udp绑定端口=%d\n",stream->port);
        if(bind(stream->socketfd,(struct sockaddr	*)&add,sizeof(add))==-1)
        {	udp_sev_out("upd绑定失败\n");
            close(stream->socketfd);
            _lock_close(stream->lock);
            return DF_ERR_PORT_CORE_SOCKET_SEV_BIND;
        }
        /*stream->task_fd=_scheduled_task_open("_udp_sev",_udp_sev_main_task,stream,DF_UDP_SEV_MAIN_TASK_TIMER,NULL);
        if(stream->task_fd<0)
        {   udp_sev_out("申请线程失败\n");
            close(stream->socketfd);
            _lock_close(stream->lock);
            return stream->task_fd;
        }*/
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
    {   //udp_sev_out("本文件需要添加守护线程\n");
        //char name[DF_FILE_NAME_MAX_LEN];
        //memset(name,0,sizeof(name));
        //sprintf(name,"socket_sev/sever/%d",stream->port);
        //ty_pd_add_file(name);
    }
    udp_sev_out("打开UDP服务成功\n");
    return 1;
}

static int _udp_sev_ctl(void *data,int fd,int cm,va_list args)
{   struct _udp_sev 	*stream;
    struct _udp_sev_fd   *id;
	stream=data;
	if((stream==NULL))	
		return DF_ERR_PORT_CORE_FD_NULL;
	if((fd==0)||(fd>sizeof(stream->fd)/sizeof(stream->fd[0])))
		return DF_ERR_PORT_CORE_FD_Z;
	fd--;
	id=&stream->fd[fd];
	if(id->state==0)
		return DF_ERR_PORT_CORE_SLAVE_DEV;
    if(cm==DF_UDP_SEV_CM_HOOK)
    {   udp_sev_out("设置回调函数和回调参数\n");
        _lock_set(stream->lock);
        stream->arg=va_arg(args,void *);
		stream->hook=va_arg(args,void *);
        _lock_un(stream->lock);
        return 0;
    }
    else if(cm==DF_UDP_SEV_CM_REV_TIMER)
    {   udp_sev_out("设置接受超时时间\n");
        _lock_set(stream->lock);
        stream->rec_timer_out=va_arg(args,int);
        _lock_un(stream->lock);
        return stream->rec_timer_out;
    }
    else if(cm==DF_UDP_SEV_CM_SEND_TIMER)
    {   udp_sev_out("设置发送超时时间\n");
        _lock_set(stream->lock);
        stream->send_timer_out=va_arg(args,int);
        _lock_un(stream->lock);
        return stream->rec_timer_out;
    }
    //else if(cm==DF_TY_CORE_CM_PD)
    //{   //udp_sev_out("守护线程调用\n");
    //    _lock_set(stream->lock);
    //    _lock_un(stream->lock);
    //    return 0;
    //}
	
    return DF_ERR_PORT_CORE_CM;
}

static int _udp_sev_read(void *data,int fd,uint8 *buf,int buf_len)
{   struct _udp_sev 	*stream;
    struct _udp_sev_fd   *id;
	struct sockaddr_in	saClient;
	int nFromLen;
	int result;
	stream=data;
	if((stream==NULL))	
		return DF_ERR_PORT_CORE_FD_NULL;
	if((fd==0)||(fd>sizeof(stream->fd)/sizeof(stream->fd[0])))
		return DF_ERR_PORT_CORE_FD_Z;
	fd--;
	id=&stream->fd[fd];
	if(id->state==0)
		return DF_ERR_PORT_CORE_SLAVE_DEV;
    udp_sev_out("读取udp数据\n");
	nFromLen =sizeof(saClient);
	result = recvfrom(stream->socketfd, buf, buf_len, 0, (struct sockaddr *)&saClient, (socklen_t * __restrict__)&nFromLen);
	if(result<=0)
	{	udp_sev_out("UDP服务接收出错result%d\n",result);
		return -1;	
	}
    udp_sev_out("读取udp数据完成 result = %d nFromLen =%d  client ip %s,client port = %d\n",result,nFromLen,inet_ntoa(saClient.sin_addr),ntohs(saClient.sin_port));
	memcpy(&id->saClient,&saClient,sizeof(saClient));
	//memcpy(id->client_ip,inet_ntoa(saClient.sin_addr),strlen(inet_ntoa(saClient.sin_addr)));
	//id->client_port = ntohs(saClient.sin_port);
    return result;   
}


static int _udp_sev_write(void *data,int fd,uint8 *buf,int buf_len)
{   struct _udp_sev 	*stream;
    struct _udp_sev_fd   *id;
	struct sockaddr_in	saClient;
	//int nFromLen =0; 
	int result;
	udp_sev_out("UDP发送数据\n");
	stream=data;
	if((stream==NULL))	
		return DF_ERR_PORT_CORE_FD_NULL;
	if((fd==0)||(fd>sizeof(stream->fd)/sizeof(stream->fd[0])))
		return DF_ERR_PORT_CORE_FD_Z;
	fd--;
	id=&stream->fd[fd];
	if(id->state==0)
		return DF_ERR_PORT_CORE_SLAVE_DEV;
	//printf("收到的信息：%s,从%s,%d \n",szBuff,inet_ntoa(saClient.sin_addr),ntohs(saClient.sin_port));  
	//udp_sev_out("发送地址%s端口%d\n",id->client_ip,id->client_port);
	//saClient.sin_addr.s_addr=htonl(id->client_ip);
	//saClient.sin_port=htons(id->client_port);
	memcpy(&saClient,&id->saClient,sizeof(saClient));
	result=sendto(stream->socketfd,buf,buf_len,0,(struct sockaddr *)&saClient,sizeof(saClient));
	if(result<0)
	{	udp_sev_out("UDP服务发送出错result%d\n",result);
		return -1;	
	}
    //udp_sev_out("发送udp数据完成 result = %d nFromLen =%d\n",result,nFromLen);
    return result;   
}

static int _udp_sev_close(void *data,int fd)
{   struct _udp_sev 	*stream;
    struct _udp_sev_fd 	*id;
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
    {   udp_sev_out("关闭本模块\n");
        close(stream->socketfd);
        _scheduled_task_close(stream->task_fd);
        for(;_udp_sev_read(data,fd,NULL,0)!=0;usleep(100000));
        _lock_close(stream->lock);
        stream->state=0;
        udp_sev_out("关闭成功\n");
    }    
    return 0;
}


//static void _udp_sev_main_task(void *arg,int o_timer)
//{   
//	struct _udp_sev 	*stream;
//	struct sockaddr_in	saClient = {0};
//	int nFromLen = sizeof(saClient);  
//	uint8 data[500];
//	int result,i;
//	stream=arg;
//	if(stream==NULL)
//		return ;
//	if(stream->hook==NULL)
//		return ;
//	//result=recvfrom(stream->socketfd,data,sizeof(data),(sockaddr_in *)&saClient,&nFromLen);
//	//if(result<=0)
//	//{	udp_sev_out("UDP服务接收出错result%d\n",result);
//	//	return ;	
//	//}
//	//else
//	//{	_lock_set(stream->lock);
//	//	//udp_sev_out("接受到数据\n");		
//	//	//stream->hook(stream->socketfd,NULL,stream->arg,NULL);		
//	//	_lock_un(stream->lock);
//	//}
//	return ;
//}
















