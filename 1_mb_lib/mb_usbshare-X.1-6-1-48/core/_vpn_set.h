#ifndef _VPN_SET_DEFINE
    #define _VPN_SET_DEFINE
    #include "_deploy.h"
	#include "../linux_sys/_ftp_n.h"
    #include "_event_file.h"
#ifdef MTK_OPENWRT
	#include "../linux_sys/_linux_ip_mtk.h"
#else
    #include "../linux_sys/_linux_ip.h"
#endif
	#include "_get_net_time.h"
	#include "signal.h"
	#include "../manage/_process_manage.h"
    /*==================================立即数定义=================================================*/
    #define DF_VPN_SET_CM_SET_VPN			100                             //设置VPN参数
	#define DF_VPN_GET_CM_GET_VPN			DF_VPN_SET_CM_SET_VPN+1			//获取VPN参数			

    
    #ifdef _vpn_set_c
		 #ifdef DEBUG
			#define out(s,arg...)       				log_out("all.c",s,##arg) 
		#else
			#define out(s,arg...)
		#endif		
		#define DF_VPN_CONFIG_FILE						"/home/share/openvpn/openvpn-client.conf"
        #define DF_VPN_FTP_DOWNLOAD_NAME				"vpnuser"
		#define DF_VPN_FTP_DOWNLOAD_PASS				"vpnuser123"
		#define DF_VPN_FTP_DOWNLOAD_PORT				21
        #define e_vpn_set
		struct _vpn_set  *fd_stream;
    #else
        #define e_vpn_set                    extern
    #endif
    /*====================================结构体===================================================*/
    #ifdef _vpn_set_c

        struct _vpn_set_fd
        {	int state;
            struct ty_file_mem	*mem;
            void *mem_dev;
            struct ty_file_fd	*mem_fd;
            struct _vpn_set   *dev;
        };
        
        struct _vpn_set
        {   int deploy_fd;                                  //开关模块
            int lock;
			struct _vpn_deploy	vpn_deploy;
            int state;
			struct _vpn_set_fd    fd[DF_FILE_OPEN_NUMBER];
        };
        struct vpn_set_fuc
        {   int cm;
            int (*ctl)(struct _vpn_set_fd   *id,va_list args);
        };
        
    #endif
    /*=======================================函数定义============================================*/   
    e_vpn_set    int vpn_set_add(void);
    #ifdef _vpn_set_c
        static int vpn_set_open(struct ty_file	*file);  
        static int vpn_set_ctl(void *data,int fd,int cm,va_list args); 
        static int vpn_set_close(void *data,int fd);    
		
		static void check_vpn_config_file(void *arg);
		static void check_vpn_process_status(void *arg,int timer);
		static int jude_process_exsit_execute(struct _vpn_set  *stream);
		static int set_user_passwd_file(struct _vpn_set *stream);
		static int check_file_and_ca(struct _vpn_set  *stream);
		static int check_lib_so_update(struct _vpn_set  *stream,uint8 *path,uint8 *so_name);



        /*====================================CTL函数=====================================*/
		static int set_vpn_par_file(struct _vpn_set_fd  *id,va_list args);
		static int get_vpn_par_file(struct _vpn_set_fd  *id,va_list args);
        
    #endif    
    /*=======================================常数定义============================================*/
    #ifdef _vpn_set_c

        static const struct vpn_set_fuc ctl_fun[]=
        {   {   DF_VPN_SET_CM_SET_VPN,    set_vpn_par_file			},
            {   DF_VPN_GET_CM_GET_VPN,    get_vpn_par_file			}

        };
        
        static const struct _file_fuc	vpn_set_fuc=
		{	.open=vpn_set_open,
			.read=NULL,
			.write=NULL,
			.ctl=vpn_set_ctl,
			.close=vpn_set_close,
			.del=NULL
		};
        
        

    #endif        
        

#endif
