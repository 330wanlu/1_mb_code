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
    /*=========================================����������===============================================*/
    #define DF_UDP_SEV_MAX_LINE                              128             //���������245
    
    #define DF_UDP_SEV_CM_HOOK                   100                             //���ò����ص�
    #define DF_UDP_SEV_CM_CLOSE_TYPE             (DF_UDP_SEV_CM_HOOK+1)       //���ùر�����
    #define DF_UDP_SEV_CM_OPEN_KEEP              (DF_UDP_SEV_CM_CLOSE_TYPE+1) //����KEEPLIVER    
    #define DF_UDP_SEV_CM_NODELAY                (DF_UDP_SEV_CM_OPEN_KEEP+1)  //СƬ�ȴ�
    #define DF_UDP_SEV_CM_REV_TIMER              (DF_UDP_SEV_CM_NODELAY+1)    //���ܳ�ʱʱ��   
    #define DF_UDP_SEV_CM_SEND_TIMER             (DF_UDP_SEV_CM_REV_TIMER+1)  //���ͳ�ʱʱ�� 
    #define DF_UDP_SEV_CM_LINE_NODELAY           (DF_UDP_SEV_CM_SEND_TIMER+1) //���õ�����·СƬ�ȴ�    
    #define DF_UDP_SEV_CM_LINE_REC_O             (DF_UDP_SEV_CM_LINE_NODELAY+1)   //���ܳ�ʱʱ��     
	#define	DF_UDP_SEV_CM_LINE_CLOSE				(DF_UDP_SEV_CM_LINE_REC_O+1)
    #ifdef _udp_sev_c
        #define out(s,arg...)						//log_out("_udp_sev.c",s,##arg)                                    
        #define e_udp_sev
    #else
        #define e_udp_sev                                    extern
    
    #endif
    
    
    #ifdef _udp_sev_c
        #define DF_UDP_SEV_MAIN_TASK_TIMER                   10               //���߳���ѯʱ��
        
    #else
    
    
    #endif
    
    /*=========================================�ṹ�嶨��===============================================*/
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
    {   int socketfd;                               //����·�ļ���                             
        uint32 ip;                                  //�ͻ��˵�IP��ַ
        uint16 port;                                //�ͻ��˵Ķ˿ں�
        struct _udp_sev  *stream;                
    };
    struct _udp_sev
    {   int state;                                                      //��ģ��״̬
        int socketfd;                                                   //������ļ���
        int lock;                                                       //������
        int task_fd;                                                    //�������
        int close_type;                                                 //�رշ�ʽ
        int open_keepaliver;                                            //�Ƿ����������
        int nodelay;                                                    //�Ƿ����С����delay
        int rec_timer_out;                                              //���ܳ�ʱʱ��
        int send_timer_out;                                             //���ͳ�ʱʱ��
        uint16 port;                                                    //����Ķ˿ں�
        void *arg;                                                      //Ӧ�ò���
        int (*hook)(int sockted,uint32 ip,uint16 port,void *arg);       //Ӧ�ûص�����
        struct  _udp_sev_line    line[DF_UDP_SEV_MAX_LINE];       //���ӵ���·
        struct _udp_sev_fd   fd[DF_FILE_OPEN_NUMBER];
    };
    
    /*========================================��������=================================================*/
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
    /*======================================��������===================================================*/
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
