#define _socket_client_c
#include "_socket_client.h"
int _socket_client_add(char *name)
{	struct _socket_client	*socket_client;
	int result;
	socket_client=malloc(sizeof(struct _socket_client));
	if(socket_client==NULL)
		return DF_ERR_MEM_ERR;
	memset(socket_client,0,sizeof(struct _socket_client));	

	result = ty_file_add(DF_DEV_TY_SOCKET_CLIENT, socket_client, name, (struct _file_fuc *)&socket_client_fuc);
	if(result<0)
		free(socket_client);
	return result;
}

static int _socket_client_del(void *data,int c)
{	struct _socket_client	*socket_client;
	int i;
	c=c;
	socket_client=data;
	if(socket_client==NULL)	
		return DF_ERR_PORT_CORE_FD_NULL;
	for(i=0;i<sizeof(socket_client->fd)/sizeof(struct _socket_client_fd);i++)
	{	if(socket_client->fd[i].state==0)
			continue;
		_socket_client_close(data,i+1);
	}
	free(socket_client);
	return 0;
}




static int _socket_client_open(struct ty_file	*file)
{	struct _socket_client	*stream;
	struct _socket_client_fd	*id;
	int j;
	stream=file->pro_data;
	if(stream==NULL)
	{	
		return DF_ERR_PORT_CORE_FD_NULL;
	}
	for(j=0;j<sizeof(stream->fd)/sizeof(struct _socket_client_fd);j++)
	{	if(stream->fd[j].state==0)
			break;
	}
	if(j==sizeof(stream->fd)/sizeof(stream->fd[0]))
	{	out("[_socket_client_open] DF_ERR_PORT_CORE_SLAVE_SPACE\n");
		return DF_ERR_PORT_CORE_SLAVE_SPACE;
	}
	id=&stream->fd[j];
	memset(id,0,sizeof(struct _socket_client_fd));
	//out("[_socket_client_open]申请一个数据锁\n");
    id->lock=_lock_open("_socket_client.c",NULL);
    if(id->lock<0)
    {   out("[_socket_client_open]申请锁失败\n");
        return id->lock;
    }
    //out("[_socket_client_open]申请一个新的socket客户端\n");
	id->socketfd=socket(AF_INET,SOCK_STREAM,0);
	//out("\n\n================================================\n[_socket_client_open]socketfd = %d\n\n\n",id->socketfd);
	if(id->socketfd<0)
		return id->socketfd;
	//out("[_socket_client_open]socket fd = %d\n",id->socketfd);
	//out("[_socket_client_open]客户端:socketfd=%d\n",id->socketfd);
	
	
	//id=&socket_client->fd[j];
	//memset(&stream->fd[j],0,sizeof(stream->fd[0]));
	//stream->fd[j].dev=stream; 
	stream->fd[j].state=1; 
	stream->fd[j].mem_fd=&file->fd[j];
	stream->state++;
	out("[_socket_client_open]打开socket客户端成功 return = %d \n",j+1);
	return j+1;
}

static int _socket_client_ctl(void *pro_data,int fd,int cm,va_list args)
{	struct _socket_client		*stream;
	struct _socket_client_fd	*id;
	//int result;
	stream=pro_data;
	if(stream==NULL)	
		return DF_ERR_PORT_CORE_FD_NULL;
	if((fd==0)||(fd>sizeof(stream->fd)/sizeof(struct _socket_client_fd)))
		return DF_ERR_PORT_CORE_FD_Z;
	fd--;
	id=&stream->fd[fd];
	if(id->state==0)
		return DF_ERR_PORT_CORE_SLAVE_DEV;
	if(cm==DF_SOCKET_CLIENT_CM_HOOK)
	{	_lock_set(id->lock);
		id->arg=va_arg(args,void *);
		id->hook=va_arg(args,void *);
		id->err=va_arg(args,void *);
		_lock_un(id->lock);
		return 1;
	}
	else if(cm==DF_SOCKET_CLIENT_CM_HOST)
	{	uint8 *host;
		int port;
		//int aa;
		int flags;
		int ip_len;
		int result;
		struct sockaddr_in add;
/*		if(id->connect!=0)
		{	out("已经连接了服务器\n");
			return -1;
		}*/	
		host=va_arg(args,uint8 *);
		port=va_arg(args,int);
		memset(id->name,0,sizeof(id->name));
		ip_len = get_DNS_ip((char *)host, (char *)id->name);
		if(ip_len<0)
		{
			out("解析IP失败\n");
			return -1;
		}
		out("解析IP成功 IP为：%s\n",id->name);
		//memcpy(id->name,host,strlen(host));
		id->port=port;
		memset(&add,0,sizeof(add));
		add.sin_family=AF_INET;
		inet_pton(AF_INET,id->name,&add.sin_addr);
		add.sin_port=htons(id->port);
		out("连接服务器:%s:%d\n",id->name,id->port);
		if((result=connect(id->socketfd,(struct sockaddr *)&add,sizeof(add)))==-1)
		{	out("连接服务器失败\n");
			return -1;
		}		
		flags = fcntl(id->socketfd, F_GETFL, 0); //设置成非阻塞模式
		fcntl(id->socketfd, F_SETFL, flags | O_NONBLOCK);
		id->connect=1;
		id->task=_task_open("socket_client",_socket_client_task,id,200,NULL);
		out("_task_open   id->task = %d\n",id->task);
		id->poll = 1;
		out("连接服务器成功\n");
		return 1;	
	}
	else if(cm==DF_SOCKET_CLIENT_CM_CLOSE_TASK)
	{
		out("即将关闭扫描线程  id->task = %d\n",id->task);
		_task_close(id->task);
		out("成功关闭扫描线程\n");
		id->poll=0;
		return 1;
	}
	return DF_ERR_PORT_CORE_CM;
}


static int _socket_client_write(void *pro_data,int fd,uint8 *buf,int buf_len)
{	struct _socket_client		*stream;
	struct _socket_client_fd	*id;
	uint8 *pbuf;
	int result;
	//int i;
	stream=pro_data;
	if(stream==NULL)	
		return DF_ERR_PORT_CORE_FD_NULL;
	if((fd==0)||(fd>sizeof(stream->fd)/sizeof(struct _socket_client_fd)))
		return DF_ERR_PORT_CORE_FD_Z;
	fd--;
	id=&stream->fd[fd];
	if(id->state==0)
		return DF_ERR_PORT_CORE_SLAVE_DEV;
	pbuf=buf;
	if(id->mem_fd->ty_fd!=0)
	{	pbuf=malloc(buf_len);
		if(pbuf==NULL)
			return DF_ERR_MEM_ERR;
		memset(pbuf,0,buf_len);
		mem_read(id->mem_fd,pbuf,buf,buf_len);
	}
	//out("write fd = %d\n",id->socketfd);
	//out("write buf_len = %d\n",buf_len);
	//for(i=0;i<buf_len;i++)
	//{
	//	printf("%02x ",pbuf[i]);
	//}
	//printf("\n");
	result=write(id->socketfd,pbuf,buf_len);
	if(id->mem_fd->ty_fd!=0)
	{	out("释放缓存\n");
		free(pbuf);
	}
	return result;
}

static int _socket_client_read(void *pro_data,int fd,uint8 *buf,int buf_len)
{	struct _socket_client		*stream;
	struct _socket_client_fd	*id;
	int result;
	uint8 *pbuf;
	stream=pro_data;
	if(stream==NULL)	
		return DF_ERR_PORT_CORE_FD_NULL;
	if((fd==0)||(fd>sizeof(stream->fd)/sizeof(struct _socket_client_fd)))
		return DF_ERR_PORT_CORE_FD_Z;
	fd--;
	id=&stream->fd[fd];
	if(id->state==0)
		return DF_ERR_PORT_CORE_SLAVE_DEV;
	pbuf=buf;
	if(id->mem_fd->ty_fd!=0)
	{	pbuf=malloc(buf_len);
		if(pbuf==NULL)
			return DF_ERR_MEM_ERR;
		memset(pbuf,0,buf_len);
	}
	result=read(id->socketfd,pbuf,buf_len);
	if(result>0)
	{	if(id->mem_fd->ty_fd!=0)
		{	mem_write(id->mem_fd,pbuf,buf,result); 
			free(pbuf);
		}
	}	
	return result;
}


static int _socket_client_close(void *pro_data,int fd)
{	struct _socket_client		*stream;
	struct _socket_client_fd	*id;
	//int result;
	//uint8 *pbuf;
	out("关闭客户端\n");
	stream=pro_data;
	
	if(stream==NULL)	
		return DF_ERR_PORT_CORE_FD_NULL;
	if((fd==0)||(fd>sizeof(stream->fd)/sizeof(struct _socket_client_fd)))
		return DF_ERR_PORT_CORE_FD_Z;
	fd--;
	id=&stream->fd[fd];
	if(id->state==0)
		return DF_ERR_PORT_CORE_SLAVE_DEV;
	out("客户端关闭线路:id->socketfd=%d\n",id->socketfd);
	
	//sleep(1);
	if(id->poll!=0)
	{	out("【_socket_client_close】有任务没有关闭，需先关闭任务\n");
		_task_close(id->task);
		id->poll=0;
		id->task=0;
	} 
	shutdown(id->socketfd,SHUT_RDWR);
	close(id->socketfd);
	out("关闭成功\n");
	_lock_close(id->lock);
	memset(id,0,sizeof(struct _socket_client_fd));
	stream->state--;
	id->state--;
	return 0;
}

static void _socket_client_task(void *arg,int o_timer)
{	struct _socket_client_fd		*id;	
	uint8 byte;
	int result;
	id=arg;
	if(id==NULL)
		return ;
	if(id->hook==NULL)
		return ;
	if(id->connect==0)
		return ;
	result=recv(id->socketfd,&byte,sizeof(byte),MSG_PEEK);
	if(result==0)
	{	out("线路出现问题result=%d socketfd=%d\n",result,id->socketfd);
		id->connect=0;
		id->task=0;
		_lock_set(id->lock);
		if(id->err!=NULL)
		{	if(id->mem_fd->ty_fd==0)
			{	if(id->err!=NULL)
					id->err(id->arg);
			}	
			else
				mem_hook(id->mem_fd,id->err,id->arg);
		}
		_lock_un(id->lock);
		return ;	
	}
	else if(result<0)
		return ;
	else
	{	_lock_set(id->lock);
		//out("接受到数据\n");
		if(id->hook!=NULL)
		{	if(id->mem_fd->ty_fd==0)
			{	//out("_socket_client_task test1\n");
				if(id->hook!=NULL)
				{	//out("\n\n================================================\nsocketfd = %d\n\n\n",id->socketfd);
					id->hook(id->socketfd,id->arg);
				}
			}	
			else
			{	//out("_socket_client_task test2\n");
				mem_hook(id->mem_fd,id->hook,id->arg);
			}
		}
		_lock_un(id->lock);
	}
	return ;
}









static int open_rec_over_timer(int socketfd,int over_timer)
{   struct timeval timeout;
    //out("设置接受超时时间为:%d\n",over_timer);
    memset(&timeout,0,sizeof(timeout));
    timeout.tv_sec=over_timer;
    return setsockopt(socketfd,SOL_SOCKET,SO_RCVTIMEO,&timeout,sizeof(timeout));
}

static int open_send_over_timer(int socketfd,int over_timer)
{   struct timeval timeout;
    //out("设置发送超时时间为:%d\n",over_timer);
    memset(&timeout,0,sizeof(timeout));
    timeout.tv_sec=over_timer;
    return setsockopt(socketfd,SOL_SOCKET,SO_SNDTIMEO,&timeout,sizeof(timeout));
    
}

//static int ty_socket_read(int sock,uint8 *buf,int buf_len)
//{   int i,result;
//    for(i=0;i<buf_len;i+=result)
//    {   //out("read data by read i = %d\n",i);
//		result=read(sock,buf+i,buf_len-i);
//        if(result<=0)
//		{	//out("read data time out or err \n\n\n\n");
//            return result;
//		}
//    }
//    return i;
//}
static int ty_socket_write(int sock,uint8 *buf,int buf_len)
{   int i,result;
    for(i=0;i<buf_len;i+=result)
    {   result=write(sock,buf+i,buf_len-i);
        if(result<=0)
            return result;
    }
    return i;   
}


int socket_tcp_open_send_recv_timeout(char *hostname,int port,uint8 *inbuf,int inlen)
{
	char name[300];
	int ip_len; 
	int result;
	int socketfd;
	struct sockaddr_in add;
	memset(name,0,sizeof(name));
	ip_len = get_DNS_ip((char *)hostname, (char *)name);
	if(ip_len<0)
	{	out("解析IP失败\n");
		return -1;
	}
	//out("解析IP成功 IP为：%s\n",name);
	if((socketfd=socket(AF_INET,SOCK_STREAM,0)) == -1)
	{
		out("创建socket失败\n");
		return -2;
	}
	memset(&add,0,sizeof(add));
	add.sin_family=AF_INET;
	add.sin_port=htons(port);
	add.sin_addr.s_addr = htonl(INADDR_ANY);
	inet_pton(AF_INET,name,&add.sin_addr);
	//out("连接服务器:%s:%d\n",name,port);
	if((result=connect(socketfd,(struct sockaddr *)&add,sizeof(add)))==-1)
	{	out("连接服务器%s:%d失败\n",name,port);
		close(socketfd);
		return -3;
	}	
	int set = 1;
	setsockopt(socketfd, SOL_SOCKET, MSG_NOSIGNAL, (void*)&set, sizeof(int));
	open_send_over_timer(socketfd,60);
	open_rec_over_timer(socketfd,60);
	if(ty_socket_write(socketfd,inbuf,inlen)<0)
	{
		out("发送失败\n");
		close(socketfd);
		return -4;
	}
	
	return socketfd;
}

int socket_tcp_open_send(char *hostname,int port,uint8 *inbuf,int inlen)
{	
	char name[300];
	int ip_len; 
	int result;
	int socketfd;
	struct sockaddr_in add;
	memset(name,0,sizeof(name));
	ip_len = get_DNS_ip((char *)hostname, (char *)name);
	if(ip_len<0)
	{	out("解析IP失败\n");
		return -1;
	}
	//out("解析IP成功 IP为：%s\n",name);
	if((socketfd=socket(AF_INET,SOCK_STREAM,0)) == -1)
	{
		out("创建socket失败\n");
		return -1;
	}
	memset(&add,0,sizeof(add));
	add.sin_family=AF_INET;
	add.sin_port=htons(port);
	add.sin_addr.s_addr = htonl(INADDR_ANY);
	inet_pton(AF_INET,name,&add.sin_addr);
	//out("连接服务器:%s:%d\n",name,port);
	
	//new add by whl
	const int val = 1;
	setsockopt(socketfd, IPPROTO_TCP, TCP_NODELAY, &val, sizeof(val));


	const int keepAlive = 1;
	const int keepIdle = 60;
	const int keepInterval = 5;
	const int keepCount = 2;
	setsockopt(socketfd, SOL_SOCKET, SO_KEEPALIVE, (void*)&keepAlive, sizeof(keepAlive));
	setsockopt(socketfd, SOL_TCP, TCP_KEEPIDLE, (void*)&keepIdle, sizeof(keepIdle));
	setsockopt(socketfd, SOL_TCP, TCP_KEEPINTVL, (void*)&keepInterval, sizeof(keepInterval));
	setsockopt(socketfd, SOL_TCP, TCP_KEEPCNT, (void*)&keepCount, sizeof(keepCount));
	//end 
	int nZero = 1*1024*1024;
	setsockopt(socketfd, SOL_TCP, SO_SNDBUF, (char*)&nZero, sizeof(int));
	int nRCV = 1 * 1024 * 1024;
	setsockopt(socketfd, SOL_TCP, SO_RCVBUF, (char*)&nRCV, sizeof(int));
	int set = 1;
	setsockopt(socketfd, SOL_SOCKET, MSG_NOSIGNAL, (void*)&set, sizeof(int));
	
	if((result=connect(socketfd,(struct sockaddr *)&add,sizeof(add)))==-1)
	{	out("连接服务器%s:%d失败\n",name,port);
		close(socketfd);
		return -1;
	}	

	if(ty_socket_write(socketfd,inbuf,inlen)<0)
	{
		out("发送失败\n");
		close(socketfd);
		return -2;
	}

	return socketfd;
}



int socket_http_netPost(char *hostname,int port,char *request,char *content_type,char *parmdata,int time_out,char **returndata,int *returndata_len)
{	char name[300];
	int ip_len; 
	int result;
	int socketfd;
	char s_buf[102400];
	char r_buf[102400];
	struct sockaddr_in add;
	memset(name,0,sizeof(name));
	ip_len = get_DNS_ip((char *)hostname, (char *)name);
	if(ip_len<0)
	{	out("解析IP失败\n");
		return -1;
	}
	//out("解析IP成功 IP为：%s\n",name);
	if((socketfd=socket(AF_INET,SOCK_STREAM,0)) == -1)
	{
		out("创建socket失败\n");
		return -1;
	}
	memset(&add,0,sizeof(add));
	add.sin_family=AF_INET;
	add.sin_port=htons(port);
	add.sin_addr.s_addr = htonl(INADDR_ANY);
	inet_pton(AF_INET,name,&add.sin_addr);
	//out("连接服务器:%s:%d\n",name,port);
	if((result=connect(socketfd,(struct sockaddr *)&add,sizeof(add)))==-1)
	{	out("连接服务器%s:%d失败\n",name,port);
		close(socketfd);
		return -1;
	}	
	open_rec_over_timer(socketfd,time_out);
	open_send_over_timer(socketfd,time_out);
	memset(s_buf,0,sizeof(s_buf));
	memset(r_buf,0,sizeof(r_buf));
	sprintf(s_buf, "POST %s HTTP/1.1\r\n",request);
    sprintf(s_buf+strlen(s_buf),"Content-Type: %s\r\n",content_type);  
	sprintf(s_buf+strlen(s_buf),"Timeout: 5000\r\n");
	sprintf(s_buf+strlen(s_buf),"Host: %s\r\n",name);
	sprintf(s_buf+strlen(s_buf),"content-Length: %d\r\n",strlen(parmdata)+2);
	sprintf(s_buf+strlen(s_buf),"User-Agent: libghttp/1.0\r\n");
    sprintf(s_buf+strlen(s_buf), "\r\n");  
    sprintf(s_buf+strlen(s_buf), "%s\r\n",parmdata);   

	
	if (ty_socket_write(socketfd, (uint8 *)s_buf, strlen(s_buf))<0)
	{
		out("发送失败\n");
		result =  -2;
	}

	//memset(returndata,0,sizeof(returndata));
	//out("进入获取数据函数\n");
	result = deal_http_data(socketfd,returndata);
	//out("退出接收处理函数\n");
	*returndata_len = result;


	//out("接收到数据：%s\n",returndata);
	shutdown(socketfd,SHUT_RDWR);
	close(socketfd); 
	return result;
}








static int deal_http_data(int sockfd,char **outbuf)
{
	char buf[1024];
	int i;
	//int j;
	char c;
	//char num=3;
	char len_buf[50];
	int numchars = 1;  
    int content_length = -1;
	//out("获取第一行数据\n");
	memset(buf,0,sizeof(buf));
	numchars = get_line(sockfd, buf, sizeof(buf));
	//out("获取到的第一行数据为：%s\n",buf);
	memset(buf,0,sizeof(buf));
	numchars = get_line(sockfd, buf, sizeof(buf)); 
	//out("获取到的第二行数据为：%s\n",buf);
    while ((numchars > 0) && strcmp("\n", buf))  
    {  
        if (strncasecmp(buf, "Content-Length:",15) == 0)  
		{	memset(len_buf,0,sizeof(len_buf));
			memcpy(len_buf,&buf[15],20);
			//out("len_buf = %s",len_buf);
			cls_no_asc(len_buf,strlen(len_buf)); 
            content_length = atoi(len_buf);  
			//out("Content-Length = %d\n",content_length);
		}
		memset(buf,0,sizeof(buf));
        numchars = get_line(sockfd, buf, sizeof(buf));  
		//out("获取到的第%d行数据为：%s",num,buf);
		//num +=1;
		//for(j=0;j<numchars;j++)
		//{
		//	printf("%02x ",(unsigned char)buf[j]);
		//}
		//printf("\n");
    }  
    /*没有找到 content_length */  
	//out("退出获取头循环\n");
    if (content_length <=0) {  
        out("错误请求,接收到的长度不对content_length = %d\n",content_length); 
        return -1;  
    }  
	 /*接收 POST 过来的数据*/  
	//if(content_length>102400)
	//{
	//	out("超过缓存大小,无法接收\n");
	//	return -1;
	//}
	int glen=sizeof(char*)*content_length+100;
	
	char *tmp=NULL;
	tmp = malloc(glen);	
	if(tmp ==NULL)
	{	out("分配接收内存失败\n");
		return -1;
	}
	
	memset(tmp,0,glen);
    for (i = 0; i < content_length; i++) 
	{  
        recv(sockfd, &c, 1, 0);
		tmp[i]=c;  
    }  
	//out("HTTP data :%s\n",tmp);
	*outbuf = tmp;
	//memcpy(outbuf,tmp,sizeof(tmp));
	//out("TRAN return data :%s\n",*outbuf);
	return content_length;
}





static int get_line(int sock, char *buf, int size)  
{  
    int i = 0;  
    unsigned char c = '\0';  
    int n;  
	//int j;
  
    /*把终止条件统一为 \n 换行符，标准化 buf 数组*/  
    while ((i < size - 1) && (c != '\n'))  
    {  
        /*一次仅接收一个字节*/  
        n = recv(sock, &c, 1, 0);  
        /* DEBUG*/
		//printf("%02x ", c); 
        if (n > 0)  
        {  
            /*收到 \r 则继续接收下个字节，因为换行符可能是 \r\n */  
            if (c == '\r')  
            {  
                /*使用 MSG_PEEK 标志使下一次读取依然可以得到这次读取的内容，可认为接收窗口不滑动*/  
                n = recv(sock, &c, 1, MSG_PEEK);  
                /* DEBUG printf("%02X\n", c); */  
                //out("\n但如果是换行符则把它吸收掉\n");
                if ((n > 0) && (c == '\n'))  
				{
                    recv(sock, &c, 1, 0);  
					//printf("%02x ", c); 
				}
                else  
				{
                    c = '\n';  
					//printf("%02x ", c); 
				}
            }  
            /*存到缓冲区*/  
            buf[i] = c;  
            i++;  
        }  
        else  
            c = '\n';  
    }  
	//printf("\n")
  
	//out("接收到的一行数据：");
	//for(j=0;j<i;j++)
	//{
	//	printf("%02x ",(unsigned char)buf[j]);
	//}
	//printf("\n");
	//out("获取到的数据：%s\n",buf);
    /*返回 buf 数组大小*/  
    return(i);  
}  