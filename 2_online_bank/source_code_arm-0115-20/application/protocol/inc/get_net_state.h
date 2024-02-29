#ifndef _GET_NET_STATE_H
#define _GET_NET_STATE_H

#include "opt.h"

#include "switch_dev.h"
#include "seria.h"
#include "linux_ip.h"
#include "deploy.h"
#include "lcd_state.h"
#include "bluetooth.h"

#include "usb_port.h"


#define DF_GET_NET_STATE_CM_CLEAN_STATUS	100
#define DF_GET_NET_STATE_CM_GET_NET_STATUS  (DF_GET_NET_STATE_CM_CLEAN_STATUS+1)
#define DF_SET_NET_WIFI_USED				(DF_GET_NET_STATE_CM_GET_NET_STATUS+1)
#define DF_GET_IP_STATE_TASK_TIMER		 	100                  //获取IP线程时间

#define get_net_state_out(s,arg...)			//log_out("_get_net_state.c",s,##arg) 

struct _net_status
{
	char first_net[10];
	char internet[10];
	char ip_type[10];
	char ip_address[20];
	char netmask[20];
	char gateway[20];
	char dns[20];
	char mac[50];
	char eth_wlan[20];
	int nettime_ok;			//外网是否准备就绪
	char internet_ip[20];
	char longitude[20];
	char latitude[20];
	char area_info[200];
	char wifi_ssid[50];
};

struct _get_net_state_fd
{
	int state;
	struct ty_file_mem	*mem;
	void *mem_dev;
	struct ty_file_fd	*mem_fd;
	struct _get_net_state  *dev;
};
    
struct _get_net_state
{
	struct _get_net_state_fd   fd[DF_FILE_OPEN_NUMBER];  
	int state;
	int lock;
	int task;
	int switch_dev;
	int eth_off_count;
	int wifi_off_count;
	int net_state; //10没网 20有网 30有外网
	int wifi_state;//wifi灯状态
	int sdcard_state;
	int deploy;
	int machine_fd;
	// struct _mode_support mode_support;
	int lcd_fd;
	int restart_mac_count;
	int net_mode;  //0有线网络 1无线网络
	uint32 ip;
	uint32 mask;
	uint32 getway;
	uint32 dns;
	char eth_mac[50];
	char wlan_mac[50];
	int type;
	int internet_info_flag;  //获取公网信息成功与否标志,0不成功  1成功
	int internet_info_err_num;  
	// int wifi_status;
	char now_ssid[50];
	char connect_ssid[50];
	struct _net_status net_status;

};

struct _get_net_ctl_fu
{
	int cm;
	int (*ctl)(struct _get_net_state_fd   *id,va_list args);
};

int get_net_state_add(void);
     
#endif
