#ifndef _SOCKER_SEV_H
#define _SOCKER_SEV_H

#include "opt.h"

#include "file.h"

#define DF_SOCKET_SEV_MAX_LINE                   245             //���������245
#define DF_SOCKET_SEV_MAIN_TASK_TIMER            1               //���߳���ѯʱ��

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

#define socket_sev_out(s,arg...)       			//log_out("all.c",s,##arg) 

struct _socket_sev_fd
{
    int state;
    struct ty_file_mem	*mem;
    void *mem_dev;
    struct ty_file_fd	*mem_fd;
    struct _socket_sev  *dev;
};

struct  _socket_sev_line 
{
    int socketfd;                               //����·�ļ���                             
    uint32 ip;                                  //�ͻ��˵�IP��ַ
    uint16 port;                                //�ͻ��˵Ķ˿ں�
    struct _socket_sev  *stream;                
};

struct _socket_sev
{
    int state;                                                      //��ģ��״̬
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
    
int _socket_sev_add(uint16 port);

#endif
