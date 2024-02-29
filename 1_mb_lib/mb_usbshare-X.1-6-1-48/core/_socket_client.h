#ifndef _SOCKET_CLIENT_DEFINE
	#define _SOCKET_CLIENT_DEFINE
	/*============================包含文件====================================*/
	#include "../file/_file.h"
	#include "_port_core.h"
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
    #include <netinet/tcp.h>
    #include <unistd.h>
    #include <netdb.h>
    #include <strings.h>
    #include <sys/stat.h>
    #include <arpa/inet.h>
    #include <netinet/in.h>
    #include <stdlib.h>
	#include "_ty_pd.h"
	#include <stdio.h>
	#include <string.h>
#ifdef MTK_OPENWRT
	#include "../linux_sys/_linux_ip_mtk.h"
#else
	#include "../linux_sys/_linux_ip.h"
#endif
	#include "../file/_mem_oper.h"
	/*===========================立即数======================================*/
	#define DF_SOCKET_CLIENT_CM_HOOK					100
	#define DF_SOCKET_CLIENT_CM_HOST					(DF_SOCKET_CLIENT_CM_HOOK+1)
	#define DF_SOCKET_CLIENT_CM_CLOSE_TASK				(DF_SOCKET_CLIENT_CM_HOST+1)
	
	#define DF_SOCKET_TCP								1000
	#define DF_SOCKET_UDP								(DF_SOCKET_TCP+1)
	
	
	#ifdef _socket_client_c
		#ifdef DEBUG
			#define out(s,arg...)       				log_out("all.c",s,##arg) 
		#else
			#define out(s,arg...)   					log_out("all.c",s,##arg) 
		#endif	
		#define e_socket_client
	#else
		#define e_socket_client						extern	
	#endif
	/*===========================结构体定义=================================*/
	#ifdef _socket_client_c
		struct _socket_client_fd
		{	int task;
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
		{	int state;			
			struct _socket_client_fd	fd[DF_FILE_OPEN_NUMBER];
		};

	#endif	
	/*=============================函数定义==========================================*/	
	e_socket_client int _socket_client_add(char *name);
	e_socket_client int socket_http_netPost(char *hostname,int port,char *request,char *content_type,char *parmdata,int time_out,char **returndata,int *returndata_len);
	e_socket_client int socket_tcp_open_send(char *hostname,int port,uint8 *inbuf,int inlen);
	e_socket_client	int socket_tcp_open_send_recv_timeout(char *hostname,int port,uint8 *inbuf,int inlen);
	#ifdef 	_socket_client_c
		static int _socket_client_del(void *data,int c);
		static int _socket_client_open(struct ty_file	*file);
		static int _socket_client_ctl(void *data,int fd,int cm,va_list args);
		static int _socket_client_close(void *data,int fd);
		static int _socket_client_del(void *data,int c);	
		static int _socket_client_write(void *data,int fd,uint8 *buf,int buf_len);
		static int _socket_client_read(void *pro_data,int fd,uint8 *buf,int buf_len);
		static void _socket_client_task(void *arg,int o_timer);
		//static int accept_respone(int sockfd,char *outbuf);
		//static int deal_response(int client,char *outbuf);
		static int get_line(int sock, char *buf, int size);
		static int deal_http_data(int sockfd,char **outbuf);
	#endif	
		
	/*=============================常数定义=======================================*/
	#ifdef _socket_client_c
		static const struct _file_fuc	socket_client_fuc=
		{	.open=_socket_client_open,
			.write=_socket_client_write,
			.read=_socket_client_read,
			.ctl=_socket_client_ctl,
			.close=_socket_client_close,
			.del=_socket_client_del
		};
	
	#endif	
#endif

