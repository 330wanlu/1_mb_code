#ifndef _SOCKER_SEV_DEFINE
    #define _SOCKER_SEV_DEFINE
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
    #define DF_SOCKET_SEV_MAX_LINE                              245             //���������245
    
    #define DF_SOCKET_SEV_CM_HOOK                   100                             //���ò����ص�
    #define DF_SOCKET_SEV_CM_CLOSE_TYPE             (DF_SOCKET_SEV_CM_HOOK+1)       //���ùر�����
    #define DF_SOCKET_SEV_CM_OPEN_KEEP              (DF_SOCKET_SEV_CM_CLOSE_TYPE+1) //����KEEPLIVER    
    #define DF_SOCKET_SEV_CM_NODELAY                (DF_SOCKET_SEV_CM_OPEN_KEEP+1)  //СƬ�ȴ�
    #define DF_SOCKET_SEV_CM_REV_TIMER              (DF_SOCKET_SEV_CM_NODELAY+1)    //���ܳ�ʱʱ��   
    #define DF_SOCKET_SEV_CM_SEND_TIMER             (DF_SOCKET_SEV_CM_REV_TIMER+1)  //���ͳ�ʱʱ�� 
    #define DF_SOCKET_SEV_CM_LINE_NODELAY           (DF_SOCKET_SEV_CM_SEND_TIMER+1) //���õ�����·СƬ�ȴ�    
    #define DF_SOCKET_SEV_CM_LINE_REC_O             (DF_SOCKET_SEV_CM_LINE_NODELAY+1)   //���ܳ�ʱʱ��     
	#define	DF_SOCKET_SEV_CM_LINE_CLOSE				(DF_SOCKET_SEV_CM_LINE_REC_O+1)//�ر���·
	#define DF_SOCKET_SEV_CM_LINE_SEND_O			(DF_SOCKET_SEV_CM_LINE_CLOSE+1)//���ͳ�ʱʱ��
    #ifdef _socket_sev_c
         #ifdef DEBUG
			#define out(s,arg...)       				//log_out("all.c",s,##arg) 
		#else
			#define out(s,arg...)
		#endif	
        #define e_socket_sev
    #else
        #define e_socket_sev                                    extern
    
    #endif
    
    
    #ifdef _socket_sev_c
        #define DF_SOCKET_SEV_MAIN_TASK_TIMER                   1               //���߳���ѯʱ��
        
    #else
    
    
    #endif
    
    /*=========================================�ṹ�嶨��===============================================*/
    struct _socket_sev_fd
    {	int state;
        struct ty_file_mem	*mem;
        void *mem_dev;
        struct ty_file_fd	*mem_fd;
        struct _socket_sev  *dev;
    };
        
    struct  _socket_sev_line 
    {   int socketfd;                               //����·�ļ���                             
        uint32 ip;                                  //�ͻ��˵�IP��ַ
        uint16 port;                                //�ͻ��˵Ķ˿ں�
        struct _socket_sev  *stream;                
    };
    struct _socket_sev
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
        struct  _socket_sev_line    line[DF_SOCKET_SEV_MAX_LINE];       //���ӵ���·
        struct _socket_sev_fd   fd[DF_FILE_OPEN_NUMBER];
    };
    
    /*========================================��������=================================================*/
    e_socket_sev    int _socket_sev_add(uint16 port);
    #ifdef _socket_sev_c
        static int _socket_sev_open(struct ty_file	*file);
        static int _socket_sev_ctl(void *data,int fd,int cm,va_list args);
        static int _socket_sev_read(void *data,int fd,uint8 *buf,int buf_len);
        static int _sock_sev_close(void *data,int fd);
        static void _socket_sev_main_task(void *arg,int o_timer);
        static int open_tcp_nodelay(int socketfd);
        static int open_tcp_keepalive(int socketfd);
        static int open_rec_over_timer(int socketfd,int over_timer);
        static int open_send_over_timer(int socketfd,int over_timer);
        static int close_line(struct _socket_sev_line    *line);
        static void _task_line(void *arg);
    #endif
    /*======================================��������===================================================*/
    #ifdef _socket_sev_c
        static const struct _file_fuc	socket_sev_fuc=
		{	.open=_socket_sev_open,
			.read=_socket_sev_read,
			.write=NULL,
			.ctl=_socket_sev_ctl,
			.close=_sock_sev_close,
			.del=NULL
		};
    #endif
#endif
