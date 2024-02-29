#ifndef _LCD_STATE_DEFINE
	#define _LCD_STATE_DEFINE
    /*======================================�����ļ�================================================*/
	#include "_switch_dev.h"
	
    /*=====================================����������===============================================*/
        /*++++++++++++++++++++++++++++++++++�������+++++++++++++++++++++++++++++++++++++*/
		#define DF_LCD_STATE_GET_STATE	100
		#define DF_LCD_STATE_UPDATE_STATE	(DF_LCD_STATE_GET_STATE+1)
        
        /*---------------------------------STM32-----------------------------------------*/
    #ifdef _lcd_state_c    
        /*+++++++++++++++++++++++++++++++����ͷ����++++++++++++++++++++++++++++++++++++++*/

    #endif
    /*--------------------------------------�ڲ����ݶ���------------------------------------------*/
    #ifdef _lcd_state_c
        #define out(s,arg...)						//log_out("_lcd_state.c",s,##arg) 
        #define e_lcd_state
    #else
        #define e_lcd_state                    extern
    #endif
    #ifdef _lcd_state_c


    #endif    

    /*======================================�ṹ�嶨��============================================*/
		struct _lcd_print_info
		{
			char version[20];
			char local_ip[20];
			int usbshare_en;	//0����usb-share����		1����usb - share����
			int usbshare_state;	//0����ֹ״̬				1������ʹ��
			int tax_en;			//0������ֵ˰��Ʊ����		1������ֵ˰��Ʊ����
			int tax_state;		//0����ֹ״̬				1������ʹ��
			int eth_en;			//0������������				1������������
			int eth_state;		//0��û�в��ҵ�����			1�����������ѻ�ȡ��ip��ַ					2������������������				8�������������ڻ�ȡip��ַ
			int wifi_en;		//0�����ȵ㹦��				1�����ȵ㹦��
			int wifi_state;		//0���ȵ����������			1���ȵ��Ѿ�����								2���ȵ��Ѿ�������
			int four_g_en;		//0:��4G����				1����4G����
			int four_g_state;	//0:���ڲ�������			1:���Ŵ���									2�����������ѻ�ȡ��ip��ַ		3��������������������δ����		8�������������ڻ�ȡip��ַ
			int sd_en;			//0����SD������				1����SD������
			int sd_state;		//0����SD������				1����SD�����룬��SD��״̬������				2����SD�����룬����
			int mqtt_en;		//0����						1����
			int mqtt_state;		//0��δ��¼					1���Ѿ���¼
			int udp_en;			//0����						1����
			int udp_state;		//0��δ��¼					1���Ѿ���¼
			#ifdef MTK_OPENWRT
			int now_net_mode;//��ǰ����ģʽ 0 LAN 1 WIFI 2 4G
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
			.version = { '0', '.', '0', '.', '0', '0'},         //�汾��
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
    /*========================================��������=================================================*/    
		e_lcd_state    int lcd_state_add(void);
    #ifdef _lcd_state_c
			static int lcd_state_open(struct ty_file	*file);
			static int lcd_state_ctl(void *data, int fd, int cm, va_list args);
			static int lcd_state_close(void *data, int fd);

			static int set_lcd_update_state(struct _lcd_state_fd *id, va_list args);
			static int get_lcd_update_state(struct _lcd_state_fd *id, va_list args);
    #endif    
    /*=======================================��������==================================================*/
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
