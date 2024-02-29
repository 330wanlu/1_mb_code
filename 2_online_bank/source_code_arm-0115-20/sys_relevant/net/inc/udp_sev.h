#ifndef _UDP_SEV_H
#define _UDP_SEV_H

#include "opt.h"

#include "file.h"

#define DF_UDP_SEV_MAX_LINE                  128             //最大连接数245
#define DF_UDP_SEV_CM_HOOK                   100                             //设置参数回调
#define DF_UDP_SEV_CM_CLOSE_TYPE             (DF_UDP_SEV_CM_HOOK+1)       //设置关闭类型
#define DF_UDP_SEV_CM_OPEN_KEEP              (DF_UDP_SEV_CM_CLOSE_TYPE+1) //开启KEEPLIVER    
#define DF_UDP_SEV_CM_NODELAY                (DF_UDP_SEV_CM_OPEN_KEEP+1)  //小片等待
#define DF_UDP_SEV_CM_REV_TIMER              (DF_UDP_SEV_CM_NODELAY+1)    //接受超时时间   
#define DF_UDP_SEV_CM_SEND_TIMER             (DF_UDP_SEV_CM_REV_TIMER+1)  //发送超时时间 
#define DF_UDP_SEV_CM_LINE_NODELAY           (DF_UDP_SEV_CM_SEND_TIMER+1) //设置单独链路小片等待    
#define DF_UDP_SEV_CM_LINE_REC_O             (DF_UDP_SEV_CM_LINE_NODELAY+1)   //接受超时时间     
#define	DF_UDP_SEV_CM_LINE_CLOSE			 (DF_UDP_SEV_CM_LINE_REC_O+1)
#define DF_UDP_SEV_MAIN_TASK_TIMER           10               //主线程轮询时间

#define udp_sev_out(s,arg...)				 //log_out("_udp_sev.c",s,##arg)                                    

struct _udp_sev_fd
{
    int state;
    struct ty_file_mem	*mem;
    void *mem_dev;
    struct ty_file_fd	*mem_fd;
    struct _udp_sev  *dev;
    struct sockaddr_in	saClient;
    uint8 client_ip[50];
    int client_port;
};

struct  _udp_sev_line
{
    int socketfd;                               //本线路文件号
    uint32 ip;                                  //客户端的IP地址
    uint16 port;                                //客户端的端口号
    struct _udp_sev  *stream;
};

struct _udp_sev
{
    int state;                                                      //本模块状态
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
    struct  _udp_sev_line    line[DF_UDP_SEV_MAX_LINE];             //连接的线路
    struct _udp_sev_fd   fd[DF_FILE_OPEN_NUMBER];
};

int _udp_sev_add(uint16 port);

#endif
