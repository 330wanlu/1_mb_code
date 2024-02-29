#ifndef _SOCKET_CLIENT_H
#define _SOCKET_CLIENT_H

#include "opt.h"

#include "file.h"
#include "linux_ip.h"
#include "mem_oper.h"

#define DF_SOCKET_CLIENT_CM_HOOK					100
#define DF_SOCKET_CLIENT_CM_HOST					(DF_SOCKET_CLIENT_CM_HOOK+1)
#define DF_SOCKET_CLIENT_CM_CLOSE_TASK				(DF_SOCKET_CLIENT_CM_HOST+1)

#define DF_SOCKET_TCP								1000
#define DF_SOCKET_UDP								(DF_SOCKET_TCP+1)
	
#define socket_client_out(s,arg...)       			//log_out("all.c",s,##arg) 
		
struct _socket_client_fd
{
	int task;
	int poll;
	int lock;
	int state;
	char name[50];
	int port;
	void *arg;
	void (*hook)(int sockted,void *arg);
	void (*err)(void *arg);
	int connect;
	int socketfd;
	struct ty_file_fd	*mem_fd;
	struct _socket_client  *dev;
};

struct _socket_client
{
	int state;			
	struct _socket_client_fd	fd[DF_FILE_OPEN_NUMBER];
};

int _socket_client_add(char *name);
int socket_http_netPost(char *hostname,int port,char *request,char *content_type,char *parmdata,int time_out,char **returndata,int *returndata_len);
int socket_tcp_open_send(char *hostname,int port,uint8 *inbuf,int inlen);
int socket_tcp_open_send_recv_timeout(char *hostname,int port,uint8 *inbuf,int inlen);

#endif
