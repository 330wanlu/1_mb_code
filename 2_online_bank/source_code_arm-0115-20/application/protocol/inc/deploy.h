#ifndef _DEPLOY_H
#define _DEPLOY_H

#include "opt.h"

#include "switch_dev.h"
#include "ty_usb.h"
#include "linux_ip.h"
#include "get_net_time.h"

#define DF_DEPLOY_CM_SET_BUSID          100                             //����busid
#define DF_DEPLOY_CM_SET_PORT_N         (DF_DEPLOY_CM_SET_BUSID+1)      //����PORT_N
#define DF_DEPLOY_CM_SET_ENUM_TIMER     (DF_DEPLOY_CM_SET_PORT_N+1)     //����ö�����ʱ��
#define DF_DEPLOY_CM_SET_IP             (DF_DEPLOY_CM_SET_ENUM_TIMER+1) //����IP
#define DF_DEPLOY_CM_SET_START_PORT     (DF_DEPLOY_CM_SET_IP+1)         //������ʼ�˿ں�
#define DF_DEPLOY_CM_SET_NET_SOFT       (DF_DEPLOY_CM_SET_START_PORT+1) //������������
#define DF_DEPLOY_CM_SET_VPN_PAR		(DF_DEPLOY_CM_SET_NET_SOFT+1)	//����VPN����
#define DF_DEPLOY_CM_SET_CLOSE_UPDATE	(DF_DEPLOY_CM_SET_VPN_PAR+1)	//�����Ƿ�ر��Զ�����
#define DF_DEPLOY_CM_GET_BUSID          (DF_DEPLOY_CM_SET_CLOSE_UPDATE+1)    //��ȡbusid
#define DF_DEPLOY_CM_GET_PORT_N         (DF_DEPLOY_CM_GET_BUSID+1)      //��ȡport_n
#define DF_DEPLOY_CM_GET_ENUM_TIMER     (DF_DEPLOY_CM_GET_PORT_N+1)     //��ȡö�����ʱ��
#define DF_DEPLOY_CM_GET_IP             (DF_DEPLOY_CM_GET_ENUM_TIMER+1) //��ȡIP
#define DF_DEPLOY_CM_GET_START_PORT     (DF_DEPLOY_CM_GET_IP+1)         //��ȡ��ʼ�˿�
#define DF_DEPLOY_CM_GET_NET_SOFT       (DF_DEPLOY_CM_GET_START_PORT+1) //��ȡ�������� 
#define DF_DEPLOY_CM_GET_REDIT          (DF_DEPLOY_CM_GET_NET_SOFT+1)   //��ȡע����Ϣ  
#define DF_DEPLOY_CM_SET_REDIT          (DF_DEPLOY_CM_GET_REDIT+1)      //ע��    
#define DF_DEPLOY_CM_GET_NET_PAR        (DF_DEPLOY_CM_SET_REDIT+1)      //��ȡ�������
#define	DF_DEPLOY_CM_GET_WIFI_PAR		(DF_DEPLOY_CM_GET_NET_PAR+1)	//��ȡWIFI�������
#define DF_DEPLOY_CM_GET_FTP_DATA		(DF_DEPLOY_CM_GET_WIFI_PAR+1)	//��ȡFTP��������
#define DF_DEPLOY_CM_GET_NET_ADDR		(DF_DEPLOY_CM_GET_FTP_DATA+1)	//��ȡ������������Ϣ
#define DF_DEPLOY_CM_SET_IP_UPDATE		(DF_DEPLOY_CM_GET_NET_ADDR+1)	//ˢ��IP����
#define DF_DEPLOY_CM_GET_VPN_PAR		(DF_DEPLOY_CM_SET_IP_UPDATE+1)	//��ȡVPN����
#define DF_DEPLOY_CM_GET_CLOSE_UPDATE	(DF_DEPLOY_CM_GET_VPN_PAR+1)	//��ȡ�Ƿ�ر��Զ�����
#define DF_DEPLOY_CM_SET_AUTH			(DF_DEPLOY_CM_GET_CLOSE_UPDATE+1)
#define DF_DEPLOY_CM_SET_TER_POW_MODE	(DF_DEPLOY_CM_SET_AUTH + 1)
#define DF_DEPLOY_CM_GET_TER_POW_MODE	(DF_DEPLOY_CM_SET_TER_POW_MODE + 1)
#define DF_DEPLOY_CM_SET_IP_W			(DF_DEPLOY_CM_GET_TER_POW_MODE+1)			//����wifi

#define DF_DEPLOY_PAR_EDIT_ODE              0X00000001
#define DF_DEPLOY_PAR_EDIT                  0X00000002        
#define DF_DEPLOY_PAR_EDIT_2                0X00000003

#define DF_NET_DEV                          "eth0"
#define DF_DEPLOY_TASK_TIMER                1000    

#define deploy_out(s,arg...)       		log_out("all.c",s,##arg) 

struct _vpn_deploy
{
	int enable;
	int mode;
	char addr[50];
	int port;
	char user[20];
	char passwd[20];
	int ftp_port;
	char ftp_user[20];
	char ftp_passwd[20];
};

struct _deploy_file
{
	int edit;                   //�汾��
	int port_n;                 //�˿�����
	int enum_timer;             //ö�����ʱ��
	char busid[30];             //U�������˿�
	int start_port;             //��ʼ�˿ں� 
	uint32 ip;                     //maid               
	uint32 mask;                //��������
	uint32 getway;              //����
	uint32 DNS;
	int ip_type;				//IP����DHCP��̶�
	char net_soft[200];         //����������ַ          
	int vpn_enable;
	int vpn_mode;
	char vpn_addr[50];
	int vpn_port;
	char vpn_user[20];
	char vpn_passwd[20];
	int vpn_ftp_port;
	char vpn_ftp_user[20];
	char vpn_ftp_passwd[20];
	int close_auto_update;		//�ر��Զ����� 0���ر� 1�ر�
	#ifdef MTK_OPENWRT
	int lan_in;//�Ƿ������߲���
	int flag_dhcp;//dhcp�Ƿ�ִ����� 1 ��� 0δ���
	#endif
	
};

struct _deploy_fd
{
	int state;
	struct ty_file_mem	*mem;
	void *mem_dev;
	struct ty_file_fd	*mem_fd;
	struct _deploy   *dev;
};

struct _deploy
{
	int switch_fd;                                  //����ģ��
	int ftp_fd;                                     //FTPģ��
	int sd_fd;
	int ty_usb_fd;                                  //
	int net_time_fd;
	int ty_usb_m_fd;
	int ftp_task;                                   //ftp����
	int lock;
	int hardtype;
	char switch_name[DF_FILE_NAME_MAX_LEN];         //����ģ��
	//char ftp_name[DF_FILE_NAME_MAX_LEN];            //FTP����
	char ty_usb_name[DF_FILE_NAME_MAX_LEN];         //TY_USB����
	//char net_time_name[DF_FILE_NAME_MAX_LEN];		//�����ʱ
	struct _deploy_fd    fd[DF_FILE_OPEN_NUMBER];
	int state;
	struct _deploy_file     file;
	_so_note    *note;
};

struct _deloy_ctl_fu
{
	int cm;
	int (*ctl)(struct _deploy_fd   *id,va_list args);
};

int deploy_add(_so_note    *note,const char *switch_name,const char *usb_name,int hard_type);

#endif
