#ifndef _LCD_STATE_DEFINE
	#define _LCD_STATE_DEFINE
    /*======================================包含文件================================================*/
	#include "_switch_dev.h"
	
    /*=====================================立即数定义===============================================*/
        /*++++++++++++++++++++++++++++++++++命令参数+++++++++++++++++++++++++++++++++++++*/
		#define DF_LCD_STATE_GET_STATE	100
		#define DF_LCD_STATE_UPDATE_STATE	(DF_LCD_STATE_GET_STATE+1)
        
        /*---------------------------------STM32-----------------------------------------*/
    #ifdef _lcd_state_c    
        /*+++++++++++++++++++++++++++++++命令头定义++++++++++++++++++++++++++++++++++++++*/

    #endif
    /*--------------------------------------内部数据定义------------------------------------------*/
    #ifdef _lcd_state_c
        #define out(s,arg...)						//log_out("_lcd_state.c",s,##arg) 
        #define e_lcd_state
    #else
        #define e_lcd_state                    extern
    #endif
    #ifdef _lcd_state_c


    #endif    

    /*======================================结构体定义============================================*/
		struct _lcd_print_info
		{
			char version[20];
			char local_ip[20];
			int usbshare_en;	//0：无usb-share功能		1：有usb - share功能
			int usbshare_state;	//0：静止状态				1：正在使用
			int tax_en;			//0：无增值税开票功能		1：有增值税开票功能
			int tax_state;		//0：静止状态				1：正在使用
			int eth_en;			//0：无有线网络				1：有有线网络
			int eth_state;		//0：没有查找到网线			1：有线网络已获取到ip地址					2：有线网络能上外网				8：有线网络正在获取ip地址
			int wifi_en;		//0：无热点功能				1：有热点功能
			int wifi_state;		//0：热点查找驱动中			1：热点已经开启								2：热点已经有连接
			int four_g_en;		//0:无4G功能				1：有4G功能
			int four_g_state;	//0:正在查找驱动			1:拨号错误									2：无线网络已获取到ip地址		3：无线网络能上外网，未传输		8：无线网络正在获取ip地址
			int sd_en;			//0：无SD卡功能				1：有SD卡功能
			int sd_state;		//0：无SD卡插入				1：有SD卡插入，但SD卡状态有问题				2：有SD卡插入，正常
			int mqtt_en;		//0：无						1：有
			int mqtt_state;		//0：未登录					1：已经登录
			int udp_en;			//0：无						1：有
			int udp_state;		//0：未登录					1：已经登录
			#ifdef MTK_OPENWRT
			int now_net_mode;//当前网络模式 0 LAN 1 WIFI 2 4G
			#endif
		};

		struct _lcd_print_info lcd_print_info;
    
    #ifdef _lcd_state_c
        struct _lcd_state_fd
        {	int state;
            struct ty_file_mem	*mem;
            void *mem_dev;
            struct ty_file_fd	*mem_fd;
            struct _lcd_state  *dev;
        };
        
        struct _lcd_state
        {   struct _lcd_state_fd   fd[DF_FILE_OPEN_NUMBER];  
            int state;
            int lock;
			int task;
			int switch_dev;
			int net_state;
			int deploy;
        };
        struct _lcd_state_ctl_fu
        {   int cm;
            int (*ctl)(struct _lcd_state_fd   *id,va_list args);
        };
        
		static struct _lcd_print_info default_info =
		{ 
			.version = { '0', '.', '0', '.', '0', '0'},         //版本号
			.local_ip = { '1', '2', '7', '.', '0', '.', '0', '.', '1' },                          
			.usbshare_en = 0,                      
			.usbshare_state = 0,       		           
			.tax_en = 1,						
			.tax_state = 0,                            
			.eth_en = 1,                            
			.eth_state = 0,                         
			.wifi_en = 0,								
			.wifi_state = 0,							
			.four_g_en = 0,
			.four_g_state = 0,
			.sd_en = 1,
			.sd_state = 0,
			.mqtt_en = 0,
			.mqtt_state = 0,
			.udp_en = 0,
			.udp_state = 0
		};
        
    #endif
    /*========================================函数定义=================================================*/    
		e_lcd_state    int lcd_state_add(void);
    #ifdef _lcd_state_c
			static int lcd_state_open(struct ty_file	*file);
			static int lcd_state_ctl(void *data, int fd, int cm, va_list args);
			static int lcd_state_close(void *data, int fd);

			static int set_lcd_update_state(struct _lcd_state_fd *id, va_list args);
			static int get_lcd_update_state(struct _lcd_state_fd *id, va_list args);
    #endif    
    /*=======================================常数定义==================================================*/
    #ifdef _lcd_state_c
			static const struct _lcd_state_ctl_fu ctl_fun[] = {
				{ DF_LCD_STATE_GET_STATE, get_lcd_update_state },
				{ DF_LCD_STATE_UPDATE_STATE, set_lcd_update_state }
			};
        
			static const struct _file_fuc	lcd_state_fuc =
			{ .open = lcd_state_open,
			.read=NULL,
			.write=NULL,
			.ctl = lcd_state_ctl,
			.close = lcd_state_close,
			.del=NULL
		};
    
    #endif

        
#endif
