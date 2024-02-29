#ifndef _GET_NET_STATE_DEFINE
	#define _GET_NET_STATE_DEFINE
    /*======================================包含文件================================================*/
	#include "_switch_dev.h"
	#include "../core/_seria.h"
    //#include "../software/_algorithm.h"
    //#include "../software/_code_s.h"
#ifdef MTK_OPENWRT
	#include "../linux_sys/_linux_ip_mtk.h"
#else
	#include "../linux_sys/_linux_ip.h"
#endif
	#include "_deploy.h"
	#include "_lcd_state.h"
	#include "_bluetooth.h"
    /*=====================================立即数定义===============================================*/
#ifdef MTK_OPENWRT
#define NET_ERROR_NO_ETHERNET_CABEL		-100		//无网线插入 4g没有模块 附件无wifi
#define NET_ERROR_NOT_FOUND_IP			-101		//未获取到内网IP(4g 为外网IP)
#define NET_ERROR_NOT_LINK_NETWORK		-102		//不能联外网
#endif
        /*++++++++++++++++++++++++++++++++++命令参数+++++++++++++++++++++++++++++++++++++*/
		#define DF_GET_NET_STATE_CM_CLEAN_STATUS	100
		#define DF_GET_NET_STATE_CM_GET_NET_STATUS  (DF_GET_NET_STATE_CM_CLEAN_STATUS+1)
		#define DF_SET_NET_WIFI_USED				(DF_GET_NET_STATE_CM_GET_NET_STATUS+1)
        
        /*---------------------------------STM32-----------------------------------------*/
    #ifdef _get_net_state_c    
        /*+++++++++++++++++++++++++++++++命令头定义++++++++++++++++++++++++++++++++++++++*/
         #define DF_GET_IP_STATE_TASK_TIMER		 100                  //获取IP线程时间

    #endif
    /*--------------------------------------内部数据定义------------------------------------------*/
    #ifdef _get_net_state_c
        #define out(s,arg...)						log_out("_get_net_state.c",s,##arg) 
        #define e_get_net_state
    #else
        #define e_get_net_state                    extern
    #endif
    #ifdef _get_net_state_c


    #endif    

    /*======================================结构体定义============================================*/
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
		//int ip_conflict_last;
		//int ip_conflict;
		//int internet_ok;
		char eth_wlan[20];
		int nettime_ok;
		char internet_ip[20];
		char longitude[20];
		char latitude[20];
		char area_info[200];
		char wifi_ssid[50];
#ifdef MTK_OPENWRT
		int last_status;
#endif
	};
    
    #ifdef _get_net_state_c
        struct _get_net_state_fd
        {	int state;
            struct ty_file_mem	*mem;
            void *mem_dev;
            struct ty_file_fd	*mem_fd;
            struct _get_net_state  *dev;
        };

		
        
        struct _get_net_state
        {   struct _get_net_state_fd   fd[DF_FILE_OPEN_NUMBER];  
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
			struct _mode_support mode_support;
			int lcd_fd;
			int restart_mac_count;
			int net_mode;  //0有线网络 1无线网络
#ifdef MTK_OPENWRT
			int ty_usb_fd;
#endif
			uint32 ip;
			uint32 mask;
			uint32 getway;
			uint32 dns;
			char eth_mac[50];
			char wlan_mac[50];
			int type;
			int internet_info_flag;  //获取公网信息成功与否标志,0不成功  1成功
			int internet_info_err_num;  
			//int wifi_status;
			char now_ssid[50];
			char connect_ssid[50];
			struct _net_status net_status;

        };
        struct _get_net_ctl_fu
        {   int cm;
            int (*ctl)(struct _get_net_state_fd   *id,va_list args);
        };
        
        
        
    #endif
    /*========================================函数定义=================================================*/    
    e_get_net_state    int get_net_state_add(void);
    #ifdef _get_net_state_c
			static void _get_ip_state_task(void *arg,int o_timer);
			static int get_net_state_open(struct ty_file	*file);
			static int get_net_state_ctl(void *data,int fd,int cm,va_list args);
			static int get_net_state_close(void *data,int fd);
			static int get_internet_infomation(struct _get_net_state *stream);
			static int analysis_json_internet_info(struct _get_net_state *stream, char *inbuf);
			

			static int clean_all_status(struct _get_net_state_fd *id, va_list args);
			static int get_net_status(struct _get_net_state_fd *id, va_list args);
			static int set_wifi_used_status(struct _get_net_state_fd *id, va_list args);//设置wifi当前使用哪个ssid
#ifdef MTK_OPENWRT
			static int get_net_state_mtk(struct _get_net_state *stream);
#else
			static int check_net_internet(struct _get_net_state *stream);
#endif
    #endif    
    /*=======================================常数定义==================================================*/
    #ifdef _get_net_state_c

			static const struct _get_net_ctl_fu ctl_fun[]=
			{   
				{   DF_GET_NET_STATE_CM_CLEAN_STATUS,             clean_all_status             },
				{	DF_GET_NET_STATE_CM_GET_NET_STATUS,			  get_net_status               },
				{	DF_SET_NET_WIFI_USED,						  set_wifi_used_status           }
			};
        
        static const struct _file_fuc	get_net_state_fuc=
		{	.open=get_net_state_open,
			.read=NULL,
			.write=NULL,
			.ctl=get_net_state_ctl,
			.close=get_net_state_close,
			.del=NULL
		};
    
    #endif

        
#endif
