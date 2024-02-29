#ifndef _UDP_SEV_DEFINE
    #define _UDP_SEV_DEFINE
    #include "../file/_file.h"
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
    /*=========================================立即数定义===============================================*/
    #define DF_UDP_SEV_MAX_LINE                              128             //最大连接数245
    
    #define DF_UDP_SEV_CM_HOOK                   100                             //设置参数回调
    #define DF_UDP_SEV_CM_CLOSE_TYPE             (DF_UDP_SEV_CM_HOOK+1)       //设置关闭类型
    #define DF_UDP_SEV_CM_OPEN_KEEP              (DF_UDP_SEV_CM_CLOSE_TYPE+1) //开启KEEPLIVER    
    #define DF_UDP_SEV_CM_NODELAY                (DF_UDP_SEV_CM_OPEN_KEEP+1)  //小片等待
    #define DF_UDP_SEV_CM_REV_TIMER              (DF_UDP_SEV_CM_NODELAY+1)    //接受超时时间   
    #define DF_UDP_SEV_CM_SEND_TIMER             (DF_UDP_SEV_CM_REV_TIMER+1)  //发送超时时间 
    #define DF_UDP_SEV_CM_LINE_NODELAY           (DF_UDP_SEV_CM_SEND_TIMER+1) //设置单独链路小片等待    
    #define DF_UDP_SEV_CM_LINE_REC_O             (DF_UDP_SEV_CM_LINE_NODELAY+1)   //接受超时时间     
	#define	DF_UDP_SEV_CM_LINE_CLOSE				(DF_UDP_SEV_CM_LINE_REC_O+1)
    #ifdef _udp_sev_c
        #define out(s,arg...)						//log_out("_udp_sev.c",s,##arg)                                    
        #define e_udp_sev
    #else
        #define e_udp_sev                                    extern
    
    #endif
    
    
    #ifdef _udp_sev_c
        #define DF_UDP_SEV_MAIN_TASK_TIMER                   10               //主线程轮询时间
        
    #else
    
    
    #endif
    
    /*=========================================结构体定义===============================================*/
    struct _udp_sev_fd
    {	int state;
        struct ty_file_mem	*mem;
        void *mem_dev;
        struct ty_file_fd	*mem_fd;
        struct _udp_sev  *dev;	
		struct sockaddr_in	saClient;
		uint8 client_ip[50];
		int client_port;
    };
        
    struct  _udp_sev_line 
    {   int socketfd;                               //本线路文件号                             
        uint32 ip;                                  //客户端的IP地址
        uint16 port;                                //客户端的端口号
        struct _udp_sev  *stream;                
    };
    struct _udp_sev
    {   int state;                                                      //本模块状态
        int socketfd;                                                   //服务的文件号
        int lock;                                                       //数据锁
        int task_fd;                                                    //主任务号
        int close_type;                                                 //关闭方式
        int open_keepaliver;                                            //是否开启心跳检测
        int nodelay;                                                    //是否采用小数据delay
        int rec_timer_out;                                              //接受超时时间
        int send_timer_out;                                             //发送超时时间
        uint16 port;                                                    //服务的端口号
        void *arg;                                                      //应用参数
        int (*hook)(int sockted,uint32 ip,uint16 port,void *arg);       //应用回调函数
        struct  _udp_sev_line    line[DF_UDP_SEV_MAX_LINE];       //连接的线路
        struct _udp_sev_fd   fd[DF_FILE_OPEN_NUMBER];
    };
    
    /*========================================函数定义=================================================*/
    e_udp_sev    int _udp_sev_add(uint16 port);
    #ifdef _udp_sev_c
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
    #endif
    /*======================================常数定义===================================================*/
    #ifdef _udp_sev_c
        static const struct _file_fuc	udp_sev_fuc=
		{	.open=_udp_sev_open,
			.read=_udp_sev_read,
			.write=_udp_sev_write,
			.ctl=_udp_sev_ctl,
			.close=_udp_sev_close,
			.del=NULL
		};
    #endif
#endif
