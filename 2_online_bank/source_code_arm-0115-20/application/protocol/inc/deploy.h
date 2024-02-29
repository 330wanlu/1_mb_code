#ifndef _DEPLOY_H
#define _DEPLOY_H

#include "opt.h"

#include "switch_dev.h"
#include "ty_usb.h"
#include "linux_ip.h"
#include "get_net_time.h"

#define DF_DEPLOY_CM_SET_BUSID          100                             //设置busid
#define DF_DEPLOY_CM_SET_PORT_N         (DF_DEPLOY_CM_SET_BUSID+1)      //设置PORT_N
#define DF_DEPLOY_CM_SET_ENUM_TIMER     (DF_DEPLOY_CM_SET_PORT_N+1)     //设置枚举最大时间
#define DF_DEPLOY_CM_SET_IP             (DF_DEPLOY_CM_SET_ENUM_TIMER+1) //设置IP
#define DF_DEPLOY_CM_SET_START_PORT     (DF_DEPLOY_CM_SET_IP+1)         //设置起始端口号
#define DF_DEPLOY_CM_SET_NET_SOFT       (DF_DEPLOY_CM_SET_START_PORT+1) //设置网络升级
#define DF_DEPLOY_CM_SET_VPN_PAR		(DF_DEPLOY_CM_SET_NET_SOFT+1)	//设置VPN配置
#define DF_DEPLOY_CM_SET_CLOSE_UPDATE	(DF_DEPLOY_CM_SET_VPN_PAR+1)	//设置是否关闭自动升级
#define DF_DEPLOY_CM_GET_BUSID          (DF_DEPLOY_CM_SET_CLOSE_UPDATE+1)    //获取busid
#define DF_DEPLOY_CM_GET_PORT_N         (DF_DEPLOY_CM_GET_BUSID+1)      //获取port_n
#define DF_DEPLOY_CM_GET_ENUM_TIMER     (DF_DEPLOY_CM_GET_PORT_N+1)     //获取枚举最大时间
#define DF_DEPLOY_CM_GET_IP             (DF_DEPLOY_CM_GET_ENUM_TIMER+1) //获取IP
#define DF_DEPLOY_CM_GET_START_PORT     (DF_DEPLOY_CM_GET_IP+1)         //获取起始端口
#define DF_DEPLOY_CM_GET_NET_SOFT       (DF_DEPLOY_CM_GET_START_PORT+1) //获取网络升级 
#define DF_DEPLOY_CM_GET_REDIT          (DF_DEPLOY_CM_GET_NET_SOFT+1)   //获取注册信息  
#define DF_DEPLOY_CM_SET_REDIT          (DF_DEPLOY_CM_GET_REDIT+1)      //注册    
#define DF_DEPLOY_CM_GET_NET_PAR        (DF_DEPLOY_CM_SET_REDIT+1)      //获取网络参数
#define	DF_DEPLOY_CM_GET_WIFI_PAR		(DF_DEPLOY_CM_GET_NET_PAR+1)	//获取WIFI网络参数
#define DF_DEPLOY_CM_GET_FTP_DATA		(DF_DEPLOY_CM_GET_WIFI_PAR+1)	//获取FTP下载任务
#define DF_DEPLOY_CM_GET_NET_ADDR		(DF_DEPLOY_CM_GET_FTP_DATA+1)	//获取升级服务器信息
#define DF_DEPLOY_CM_SET_IP_UPDATE		(DF_DEPLOY_CM_GET_NET_ADDR+1)	//刷新IP设置
#define DF_DEPLOY_CM_GET_VPN_PAR		(DF_DEPLOY_CM_SET_IP_UPDATE+1)	//获取VPN配置
#define DF_DEPLOY_CM_GET_CLOSE_UPDATE	(DF_DEPLOY_CM_GET_VPN_PAR+1)	//获取是否关闭自动升级
#define DF_DEPLOY_CM_SET_AUTH			(DF_DEPLOY_CM_GET_CLOSE_UPDATE+1)
#define DF_DEPLOY_CM_SET_TER_POW_MODE	(DF_DEPLOY_CM_SET_AUTH + 1)
#define DF_DEPLOY_CM_GET_TER_POW_MODE	(DF_DEPLOY_CM_SET_TER_POW_MODE + 1)
#define DF_DEPLOY_CM_SET_IP_W			(DF_DEPLOY_CM_GET_TER_POW_MODE+1)			//设置wifi

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
	int edit;                   //版本号
	int port_n;                 //端口排列
	int enum_timer;             //枚举最大时间
	char busid[30];             //U盘升级端口
	int start_port;             //开始端口号 
	uint32 ip;                     //maid               
	uint32 mask;                //子网掩码
	uint32 getway;              //网关
	uint32 DNS;
	int ip_type;				//IP类型DHCP或固定
	char net_soft[200];         //网络升级地址          
	int vpn_enable;
	int vpn_mode;
	char vpn_addr[50];
	int vpn_port;
	char vpn_user[20];
	char vpn_passwd[20];
	int vpn_ftp_port;
	char vpn_ftp_user[20];
	char vpn_ftp_passwd[20];
	int close_auto_update;		//关闭自动升级 0不关闭 1关闭
	#ifdef MTK_OPENWRT
	int lan_in;//是否有网线插入
	int flag_dhcp;//dhcp是否执行完毕 1 完毕 0未完毕
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
	int switch_fd;                                  //开关模块
	int ftp_fd;                                     //FTP模块
	int sd_fd;
	int ty_usb_fd;                                  //
	int net_time_fd;
	int ty_usb_m_fd;
	int ftp_task;                                   //ftp任务
	int lock;
	int hardtype;
	char switch_name[DF_FILE_NAME_MAX_LEN];         //开关模块
	//char ftp_name[DF_FILE_NAME_MAX_LEN];            //FTP名字
	char ty_usb_name[DF_FILE_NAME_MAX_LEN];         //TY_USB名字
	//char net_time_name[DF_FILE_NAME_MAX_LEN];		//网络对时
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
