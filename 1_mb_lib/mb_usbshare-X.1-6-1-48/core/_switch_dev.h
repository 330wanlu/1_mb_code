#ifndef _SWITCH_DEV_DEFINE
    #define _SWITCH_DEV_DEFINE
    /*======================================�����ļ�================================================*/
    #include "../core/_seria.h"
	#include "../linux_sys/mqtt_client.h"
    //#include "../software/_algorithm.h"
    //#include "../software/_code_s.h"
	//#include "../manage/_log_file.h"
	#ifdef MTK_OPENWRT
	#include "../linux_sys/_linux_ip_mtk.h"
	#else
	#include "../linux_sys/_linux_ip.h"
	#endif
    /*=====================================����������===============================================*/
        /*++++++++++++++++++++++++++++++++++�������+++++++++++++++++++++++++++++++++++++*/
        #define DF_SWITCH_CM_OPEN               100                         //�򿪶˿�
        #define DF_SWITCH_CM_CLOSE              (DF_SWITCH_CM_OPEN+1)       //�رն˿�
        #define DF_SWITCH_CM_SET_ID             (DF_SWITCH_CM_CLOSE+1)      //����ID
        #define DF_SWITCH_CM_GET_ID             (DF_SWITCH_CM_SET_ID+1)     //��ȡID
		#define DF_SWITCH_CM_GET_CPUID          (DF_SWITCH_CM_GET_ID+1)     //��ȡCPUID
        #define DF_SWITCH_CM_SET_PCB_ID         (DF_SWITCH_CM_GET_CPUID+1)     //���õ���ID
        #define DF_SWITCH_CM_GET_PCB_ID         (DF_SWITCH_CM_SET_PCB_ID+1) //��ȡ����ID
        #define DF_SWITCH_CM_GET_CLASS          (DF_SWITCH_CM_GET_PCB_ID+1) //��ȡ����
        #define DF_SWITCH_CM_GET_PCB            (DF_SWITCH_CM_GET_CLASS+1)  //��ȡ����
        #define DF_SWITCH_CM_PORT_STATE         (DF_SWITCH_CM_GET_PCB+1)    //�˿�״̬      
        #define DF_SWITCH_CM_RESET_MA           (DF_SWITCH_CM_PORT_STATE+1) //��������
        #define DF_SWITCH_CM_POWER_CLOSE        (DF_SWITCH_CM_RESET_MA+1)   //�رյ�Դ    
        #define DF_SWITCH_CM_WRITE_FLASH        (DF_SWITCH_CM_POWER_CLOSE+1) //дflash
        #define DF_SWITCH_CM_READ_FLASH         (DF_SWITCH_CM_WRITE_FLASH+1) //��flash  
        #define DF_SWITCH_CM_GET_INFOR          (DF_SWITCH_CM_READ_FLASH+1)  //��ȡ�豸��Ϣ  
        #define DF_SWITCH_CM_GET_DEV_SOFT_EDIT  (DF_SWITCH_CM_GET_INFOR+1)      //��ȡ����汾   
        #define DF_SWITCH_CM_GET_HEART_TIMER    (DF_SWITCH_CM_GET_DEV_SOFT_EDIT+1)  //��ѯ����ʱ��  
        #define DF_SWITCH_CM_HEART              (DF_SWITCH_CM_GET_HEART_TIMER+1)    //����   
        #define DF_SWITCH_CM_ADD_RECODER        (DF_SWITCH_CM_HEART+1)          //���Ӽ�¼
        #define DF_SWITCH_CM_READ_RECODER       (DF_SWITCH_CM_ADD_RECODER+1)    //����¼
		#define DF_SWITCH_CM_OPEN_HUB			(DF_SWITCH_CM_READ_RECODER+1)	//��HUB����Ƭ��һֱ��HUB����
		#define DF_SWITCH_CM_GET_NET_OK			(DF_SWITCH_CM_OPEN_HUB+1)		//��ȡ������,ָ֪ͨʾ
		#define DF_SWITCH_CM_NO_NET				(DF_SWITCH_CM_GET_NET_OK+1)		//���綪ʧ,֪ͨ
		#define DF_SWITCH_CM_GET_SERIAL_NUM     (DF_SWITCH_CM_NO_NET+1)			//��ȡ��Ƭ��Ψһ��ʶ�� 
		#define DF_SWITCH_CM_PORT_NO_DEV		(DF_SWITCH_CM_GET_SERIAL_NUM+1)			//ָʾ�˿�û���豸
		#define DF_SWITCH_CM_PORT_DEV_LOCAL     (DF_SWITCH_CM_PORT_NO_DEV+1)			//ָʾ�˿��豸�ڱ���
		#define DF_SWITCH_CM_PORT_DEV_SHARE     (DF_SWITCH_CM_PORT_DEV_LOCAL+1)			//ָʾ�˿��豸��Զ�̹���
		#define DF_SWITCH_CM_CONN				DF_SWITCH_CM_PORT_DEV_SHARE+1
#ifdef MTK_OPENWRT
		#define	DF_SWITCH_CM_STATUS_SET			(DF_SWITCH_CM_CONN+1)		//״ָ̬ʾ����ʾ
		#define DF_SWITCH_CM_PORT_RED_OFF_GREN_OFF	(DF_SWITCH_CM_STATUS_SET+1)
		#define DF_SWITCH_CM_PORT_RED_ON_GREN_OFF	(DF_SWITCH_CM_PORT_RED_OFF_GREN_OFF+1)
		#define DF_SWITCH_CM_PORT_RED_OFF_GREN_ON	(DF_SWITCH_CM_PORT_RED_ON_GREN_OFF+1)
		#define DF_SWITCH_CM_PORT_RED_ON_GREN_TWI	(DF_SWITCH_CM_PORT_RED_OFF_GREN_ON+1)
		#define DF_SWITCH_CM_PORT_RED_ON_GREN_ON	(DF_SWITCH_CM_PORT_RED_ON_GREN_TWI+1)
		#define DF_SWITCH_CM_PORT_RED_TWI_GREN_ON	(DF_SWITCH_CM_PORT_RED_ON_GREN_ON+1)
		#define DF_SWITCH_CM_PORT_RED_TWI_GREN_TWI_1S		(DF_SWITCH_CM_PORT_RED_TWI_GREN_ON+1)
		#define DF_SWITCH_CM_LCD_SCREEN_PRINT		(DF_SWITCH_CM_PORT_RED_TWI_GREN_TWI_1S+1)
		#define DF_SWITCH_CM_GET_KEY_VALUE			(DF_SWITCH_CM_LCD_SCREEN_PRINT+1)
		#define DF_SWITCH_CM_PORT_RED_TWI_GREN_TWI	(DF_SWITCH_CM_GET_KEY_VALUE+1)
		#define DF_SWITCH_CM_PORT_RED_OFF_GREN_TWI		(DF_SWITCH_CM_PORT_RED_TWI_GREN_TWI+1)
		#define DF_SWITCH_CM_PORT_RED_TWI_GREN_OFF	(DF_SWITCH_CM_PORT_RED_OFF_GREN_TWI+1)
		#define DF_SWITCH_CM_MAC_RESTART_POWER		(DF_SWITCH_CM_PORT_RED_TWI_GREN_OFF+1)

		#define DF_SWITCH_CM_GET_ALL_PORT_STATUS	(DF_SWITCH_CM_MAC_RESTART_POWER+1)
		#define DF_SWITCH_CM_CONTRL_FAN			(DF_SWITCH_CM_GET_ALL_PORT_STATUS+1)
		#define DF_SWITCH_CM_PORT_DOUBLE_LIGHT_YELLOW			(DF_SWITCH_CM_CONTRL_FAN+1)
		#define DF_SWITCH_CM_GET_AD_STATUS			(DF_SWITCH_CM_PORT_DOUBLE_LIGHT_YELLOW+1)
		#define DF_SWITCH_CM_RESTORE				(DF_SWITCH_CM_GET_AD_STATUS+1)
		#define DF_SWITCH_CM_BLUETOOTH_MODE			(DF_SWITCH_CM_RESTORE+1)
		#define DF_SWITCH_CM_BLUETOOTH_POWER				(DF_SWITCH_CM_BLUETOOTH_MODE+1)
		#define DF_SWITCH_CM_GET_SUPPORT			(DF_SWITCH_CM_BLUETOOTH_POWER+1)
#else
		#define DF_SWITCH_CM_PORT_RED_OFF_GREN_OFF	(DF_SWITCH_CM_CONN+1)
		#define DF_SWITCH_CM_PORT_RED_ON_GREN_OFF	(DF_SWITCH_CM_PORT_RED_OFF_GREN_OFF+1)
		#define DF_SWITCH_CM_PORT_RED_OFF_GREN_ON	(DF_SWITCH_CM_PORT_RED_ON_GREN_OFF+1)
		#define DF_SWITCH_CM_PORT_RED_ON_GREN_TWI	(DF_SWITCH_CM_PORT_RED_OFF_GREN_ON+1)
		#define DF_SWITCH_CM_PORT_RED_ON_GREN_ON	(DF_SWITCH_CM_PORT_RED_ON_GREN_TWI+1)
		#define DF_SWITCH_CM_PORT_RED_TWI_GREN_ON	(DF_SWITCH_CM_PORT_RED_ON_GREN_ON+1)
		#define DF_SWITCH_CM_PORT_RED_TWI_GREN_TWI_1S		(DF_SWITCH_CM_PORT_RED_TWI_GREN_ON+1)
		#define DF_SWITCH_CM_LCD_SCREEN_PRINT		(DF_SWITCH_CM_PORT_RED_TWI_GREN_TWI_1S+1)
		#define DF_SWITCH_CM_GET_KEY_VALUE			(DF_SWITCH_CM_LCD_SCREEN_PRINT+1)
		#define DF_SWITCH_CM_PORT_RED_TWI_GREN_OFF	(DF_SWITCH_CM_GET_KEY_VALUE+1)
		#define DF_SWITCH_CM_PORT_RED_OFF_GREN_TWI	(DF_SWITCH_CM_PORT_RED_TWI_GREN_OFF+1)
		#define DF_SWITCH_CM_MAC_RESTART_POWER		(DF_SWITCH_CM_PORT_RED_OFF_GREN_TWI+1)

		#define DF_SWITCH_CM_GET_ALL_PORT_STATUS	(DF_SWITCH_CM_MAC_RESTART_POWER+1)
		#define DF_SWITCH_CM_CONTRL_FAN			(DF_SWITCH_CM_GET_ALL_PORT_STATUS+1)

		#define DF_SWITCH_CM_PORT_DOUBLE_LIGHT_YELLOW			(DF_SWITCH_CM_CONTRL_FAN+1)
		#define DF_SWITCH_CM_GET_AD_STATUS			(DF_SWITCH_CM_PORT_DOUBLE_LIGHT_YELLOW+1)
		#define DF_SWITCH_CM_RESTORE				(DF_SWITCH_CM_GET_AD_STATUS+1)
		#define DF_SWITCH_CM_BLUETOOTH_MODE			(DF_SWITCH_CM_RESTORE+1)
		#define DF_SWITCH_CM_BLUETOOTH_POWER				(DF_SWITCH_CM_BLUETOOTH_MODE+1)
		#define DF_SWITCH_CM_GET_SUPPORT			DF_SWITCH_CM_BLUETOOTH_POWER+1



#endif
	



        #define DF_SWITCH_CM_CLASS_ID           100                         //ֻ��ȡID
        #define DF_SWITCH_CM_CLASS_ALL          (DF_SWITCH_CM_CLASS_ID+1)   //��ȡ���е�class
        
        
        /*---------------------------------STM32-----------------------------------------*/
    #ifdef _switch_dev_c    
        /*+++++++++++++++++++++++++++++++����ͷ����++++++++++++++++++++++++++++++++++++++*/
        #define DF_PC_ORDER_HEAD_LEN		 7                  //STM32����ͷ����
        /*+++++++++++++++++++++++++++++++�����ֶ���++++++++++++++++++++++++++++++++++++++*/
        #define DF_PC_ORDER_CONNECT			0X01				//��������
		#define DF_PC_ORDER_DISCONNECT		0X02				//�ͷſ���Ȩ����
		#define DF_PC_ORDER_SET_DEV_ID		0X07				//�����豸ID
		#define DF_PC_ORDER_READ_DEV_ID		0X08				//��ȡ�豸ID����������
		#define DF_PC_ORDER_SET_SUB_ID		0X11				//���õ���ID
		#define DF_PC_ORDER_READ_SUB_ID		0X12				//��ȡ����ID
		#define DF_PC_ORDER_GROUNP_PRO		0X13				//��ȡ����Ⱥ����
		#define DF_PC_ORDER_SUB_PRO			0X14				//��ȡ��������
		#define DF_PC_ORDER_USB_OPEN		0X15				//��ָ���˿�
		#define DF_PC_ORDER_USB_CLOSE		0X16				//�ر�ָ���˿�
		#define DF_PC_ORDER_USB_OPEN_LIST	0X17				//��ȡ�Ѵ�USB�б�	
		#define DF_PC_ORDER_USB_GET_SOFT	0X18				//��ȡ����汾
		#define DF_PC_ORDER_USB_WRITE_SOFT	0X19				//д���
		#define DF_PC_ORDER_USB_GET_OPEN_MA	0X20				//���������������ʾ�Ѿ��򿪵�USB�˿�
		#define DF_PC_ORDER_USB_GET_MA_OPEN	0X21				//ָ��������Ż�ȡ�Ѿ��򿪵�USB�˿�
		#define DF_PC_ORDER_USB_MA_OPEN		0X22				//ָ��������Ŵ�USB�˿�
		#define DF_PC_ORDER_USB_MA_CLOSE	0X23				//ָ��������Źر�USB�˿�
		#define DF_PC_ORDER_POWER_OFF		0X24				//�رջ���
		#define DF_PC_ORDER_USB_HUB_N		0X25				//USB HUB����
        #define DF_PC_ORDER_OPEN_HUB        0X26                //������HUB
        #define DF_PC_ORDER_RESET           0X27                //��������
        #define DF_PC_ORDER_HREAT           0X28                //��������
        #define DF_PC_ORDER_HREAT_TIMER     0X29                //��ѯ����ʱ��
        #define DF_PC_ORDER_WRITE_FLASH     0X2a                //дflash
        #define DF_PC_ORDER_READ_FLASH      0X2B                //��flash
        #define DF_PC_ORDER_GET_EDIT_INFOR  0X2c                //��ȡ�汾��Ϣ
        #define DF_PC_ORDER_WRITE_RECODER   0X2D                //���Ӽ�¼�ļ�
        #define DF_PC_ORDER_READ_RECODER    0X2E                //����¼�ļ�  
		#define DF_PC_ORDER_GET_NET_OK		0X2F				//��ȡ������֪ͨ��λ��
		#define DF_PC_ORDER_NO_NET			0X30				//���綪ʧ֪ͨ��λ��
		#define DF_PC_ORDER_GET_SERIAL_NUM	0X31				//��ȡ��Ƭ��Ψһ���к�
		#define DF_PC_ORDER_CTL_PORT_LED	0X32				//���ƶ˿ڵ�״ָ̬ʾ
		#define DF_PC_ORDER_CTL_MAC_POWER	0X33				//���ƿ���MAC��Դ

		#define DF_PC_ORDER_CTL_PORT_LED_2	0X40				//���ƶ˿ڵ�״ָ̬ʾ(��������)
#ifdef MTK_OPENWRT
		#define	DF_PC_ORDER_CTL_NEW_STATUS	0X41			//	����״̬
		
#else
		#define DF_PC_ORDER_CTL_LCD_SCREEN	0X41				//����LCD��ʾ����ʾ
#endif

		#define DF_PC_ORDER_CTL_WRITE_KEY	0X45				//д����Կ�ļ�
		#define DF_PC_ORDER_CTL_READ_KEY	0X46				//��ȡ��Կ�ļ�
		#define DF_PC_ORDER_CTL_GET_ALL_PORT_STATUS	0X47		//��ȡ���µ�˿��豸״̬
		#define DF_PC_ORDER_CTL_FAN					0X49		//���Ʒ��ȴ򿪹ر�
		#define DF_PC_ORDER_CTL_BLUETOOTH	0x51				//��������ģʽ AT  ͸��
		#define	DF_PC_ORDER_BLUETOOTH_POWER	0X52				//����������Դ
		#define DF_PC_ORDER_RESTORE						0X53	//�س���
		#define DF_PC_ORDER_SET_PORT_DOUBLE_LIGHT			0X54
		#define DF_PC_ORDER_GET_AD_STATUS				0x55

		
    #endif
#define	DF_WIFI_LED_SAME_USBPORT	10001
    /*--------------------------------------�ڲ����ݶ���------------------------------------------*/
    #ifdef _switch_dev_c
         #ifdef DEBUG
			#define out(s,arg...)       				//log_out("all.c",s,##arg) 
		#else
			#define out(s,arg...)						//log_out("all.c",s,##arg) 
		#endif	
        #define e_switch_dev
    #else
        #define e_switch_dev                    extern
    #endif
    #ifdef _switch_dev_c
        #define DF_SERIA_DIR                    "/dev/seria0"   
        #define DF_SERIA_BPS                    115200    
        #define DF_TY_STM32_OVER_TIMER          2000        //STM32ͨѶ��ʱʱ��
    #endif    

    /*======================================�ṹ�嶨��============================================*/
    struct _switch_event
    {   int event;
        int reason;
        uint8 clock[8];
    };
    
    struct _switch_event_all
    {   int n;
        struct _switch_event    event[30];
    };
    
    struct _switch_dev_pcb_id
    {   uint8 id[6];
        uint8 date[4];
        int n;          //���忪�ؿ�����
    };
    struct _switch_dev_id
    {   uint8 id[6];
        uint8 date[4];
    };
    struct _switch_dev_only
	{
		uint8 serial_num[12];
	};
    struct _switch_dev_class        //����Ⱥ����
    {   uint8 id[6];
        int n;
    };
    
    struct _switch_dev_pcb      //��������
    {   uint8 id[6];
        int n;
    };
    struct _stm32_infor
    {   char r_date[11];        //��������
        char soft[9];           //����汾
        char hard[9];           //Ӳ���汾
    };
    
	struct _dev_support
	{
		int support_fan;//����
		int support_ble;//�����ͻƵ�
		int support_power;//�ڵ�
		int support_lcd;//��ʾ����LED�°�֧��
	};
    #ifdef _switch_dev_c
        struct _switch_dev_fd
        {	int state;
            struct ty_file_mem	*mem;
            void *mem_dev;
            struct ty_file_fd	*mem_fd;
            struct _switch_dev  *dev;
        };
        
        struct _switch_dev
        {   struct _switch_dev_fd   fd[DF_FILE_OPEN_NUMBER];  
            int state;
            int lock;
            int seria;
			int hardtype;
			struct _dev_support dev_support;

			char ter_type[50];
			char ter_id[20];
			char cpu_id[50];
        };
        struct _switch_ctl_fu
        {   int cm;
            int (*ctl)(struct _switch_dev_fd   *id,va_list args);
        };
		
        
        
    #endif
    /*========================================��������=================================================*/    
		e_switch_dev    int switch_dev_add(int hard_type, char *ter_type);
    #ifdef _switch_dev_c
        /*------------------------------------�ļ�ϵͳ-------------------------------------------------*/
        static int switch_dev_open(struct ty_file	*file);
        static int switch_dev_ctl(void *data,int fd,int cm,va_list args);
        static int switch_dev_close(void *data,int fd);
        /*-----------------------------------ctlִ������-----------------------------------------------*/
        static int dev_open_port(struct _switch_dev_fd   *id,va_list args);
        static int dev_close_port(struct _switch_dev_fd   *id,va_list args);
        static int dev_set_id(struct _switch_dev_fd   *id,va_list args);
        static int dev_get_id(struct _switch_dev_fd   *id,va_list args);
		static int dev_get_cpuid(struct _switch_dev_fd   *id, va_list args);
        static int dev_set_pcb_id(struct _switch_dev_fd   *id,va_list args);
        static int dev_get_pcb_id(struct _switch_dev_fd   *id,va_list args);
        static int dev_get_class(struct _switch_dev_fd   *id,va_list args);
        static int dev_get_pcb(struct _switch_dev_fd   *id,va_list args);
        static int dev_port_state(struct _switch_dev_fd   *id,va_list args);
        static int dev_power_close(struct _switch_dev_fd   *id,va_list args);
        static int dev_read_flash(struct _switch_dev_fd   *id,va_list args);
        static int dev_reset_ma(struct _switch_dev_fd   *id,va_list args);
        static int dev_power_up(struct _switch_dev	*stream);
        static int dev_power_down(struct _switch_dev	*stream);
        static int dev_write_flash(struct _switch_dev_fd   *id,va_list args);
        static int dev_get_infor(struct _switch_dev_fd   *id,va_list args);
        static int dev_get_sev_soft_edit(struct _switch_dev_fd   *id,va_list args);
        static int dev_get_heart_timer(struct _switch_dev_fd   *id,va_list args);
        static int dev_oper_heart(struct _switch_dev_fd   *id,va_list args);
        static int dev_oper_add_recoder(struct _switch_dev_fd   *id,va_list args);
        static int dev_open_read_recoder(struct _switch_dev_fd   *id,va_list args);
		static int dev_oper_get_net_ok(struct _switch_dev_fd   *id,va_list args);
		static int dev_oper_no_net(struct _switch_dev_fd   *id,va_list args);
		static int dev_open_hub(struct _switch_dev_fd   *id,va_list args);
		static int dev_get_only_serial_number(struct _switch_dev_fd   *id,va_list args);
		static int dev_oper_port_dev_local(struct _switch_dev_fd *id,va_list args);
		static int dev_oper_port_dev_share(struct _switch_dev_fd *id,va_list args);
		static int dev_oper_port_no_dev(struct _switch_dev_fd *id,va_list args);
		static int dev_oper_red_off_gren_off(struct _switch_dev_fd *id,va_list args);
		static int dev_oper_red_on_gren_off(struct _switch_dev_fd *id,va_list args);
		static int dev_oper_red_off_gren_on(struct _switch_dev_fd *id,va_list args);
		static int dev_oper_red_on_gren_twi(struct _switch_dev_fd *id,va_list args);
		static int dev_oper_red_on_gren_on(struct _switch_dev_fd *id,va_list args);
		static int dev_oper_red_twi_gren_on(struct _switch_dev_fd *id,va_list args);
		static int dev_oper_red_twi_gren_twi_1s(struct _switch_dev_fd *id, va_list args);

		static int dev_oper_get_key_value(struct _switch_dev_fd *id, va_list args);
		//static int send_oper_stm32_dev(struct _switch_dev	*stream,uint8 order,uint8 *in_buf,uint8 *out_buf,int in_len,int8 *r_order);
		static int dev_oper_red_off_gren_twi(struct _switch_dev_fd *id, va_list args);
		static int dev_oper_red_twi_gren_off(struct _switch_dev_fd *id, va_list args);
		//static void restart_mac(void *arg);
		
		static int dev_get_hard_version(struct _switch_dev	*stream);
        /*------------------------------------stm32ִ��------------------------------------------------*/
        static int oper_stm32_dev(struct _switch_dev	*stream,uint8 order,uint8 *in_buf,uint8 *out_buf,int in_len,int8 *r_order);
        static int pack_s_buf(uint8 order,uint8 *in_buf,uint8 *out_buf,int in_len);    
		//static int _mac_restart(struct _switch_dev *stream);
#ifdef MTK_OPENWRT
		static int dev_oper_red_twi_gren_off(struct _switch_dev_fd *id, va_list args);
		static int dev_oper_red_twi_gren_twi(struct _switch_dev_fd *id, va_list args);
		//static int dev_get_hard_version(struct _switch_dev	*stream);
		static int dev_status_set(struct _switch_dev_fd *id, va_list args);
#else
		static int dev_oper_lcd_screen_print(struct _switch_dev_fd *id, va_list args);
		static int dev_restart_mac_ic(struct _switch_dev_fd   *id, va_list args);
		static int mac_ic_restart(struct _switch_dev	*stream);
		static int dev_port_get_all_port_status(struct _switch_dev_fd   *id, va_list args);
		static int dev_control_fan(struct _switch_dev_fd   *id, va_list args);
		static int dev_bluetooth_mode_change(struct _switch_dev_fd *id, va_list args);
		static int dev_restore(struct _switch_dev_fd   *id, va_list args);
		static int dev_bluetooth_power(struct _switch_dev_fd   *id, va_list args);
		static int dev_oper_double_light_yellow(struct _switch_dev_fd   *id, va_list args);
		static int dev_get_ad_status(struct _switch_dev_fd   *id, va_list args);
#endif
		
		

		
		
		
		
		static int dev_get_support(struct _switch_dev_fd   *id, va_list args);
		static int dev_conn(struct _switch_dev_fd   *id, va_list args);
    #endif    
    /*=======================================��������==================================================*/
    #ifdef _switch_dev_c
		static const struct _switch_ctl_fu   ctl_fun[]=
		{   {   DF_SWITCH_CM_OPEN,                  dev_open_port   },                        //�򿪶˿�
		{	DF_SWITCH_CM_CONN,						dev_conn},
		{   DF_SWITCH_CM_CLOSE,                 dev_close_port  },      
		{   DF_SWITCH_CM_SET_ID,                dev_set_id      },
		{   DF_SWITCH_CM_GET_ID,                dev_get_id      },
		{	DF_SWITCH_CM_GET_CPUID,				dev_get_cpuid },
		{   DF_SWITCH_CM_SET_PCB_ID,            dev_set_pcb_id  },
		{   DF_SWITCH_CM_GET_PCB_ID,            dev_get_pcb_id  },
		{   DF_SWITCH_CM_GET_CLASS,             dev_get_class   },
		{   DF_SWITCH_CM_GET_PCB,               dev_get_pcb     },
		{   DF_SWITCH_CM_PORT_STATE,            dev_port_state  },
		{   DF_SWITCH_CM_RESET_MA,              dev_reset_ma    },
		{   DF_SWITCH_CM_POWER_CLOSE,           dev_power_close },
		{   DF_SWITCH_CM_WRITE_FLASH,           dev_write_flash },
		{   DF_SWITCH_CM_READ_FLASH,            dev_read_flash  },
		{   DF_SWITCH_CM_GET_INFOR,             dev_get_infor   },
		{   DF_SWITCH_CM_GET_DEV_SOFT_EDIT,     dev_get_sev_soft_edit},
		{   DF_SWITCH_CM_GET_HEART_TIMER,       dev_get_heart_timer},
		{   DF_SWITCH_CM_HEART,                 dev_oper_heart  },
		{   DF_SWITCH_CM_ADD_RECODER,           dev_oper_add_recoder},
		{   DF_SWITCH_CM_READ_RECODER,          dev_open_read_recoder   },
		{	DF_SWITCH_CM_OPEN_HUB,				dev_open_hub	},
		{	DF_SWITCH_CM_GET_NET_OK,			dev_oper_get_net_ok	},
		{	DF_SWITCH_CM_NO_NET,				dev_oper_no_net	},
		{	DF_SWITCH_CM_GET_SERIAL_NUM,		dev_get_only_serial_number},
		{	DF_SWITCH_CM_PORT_NO_DEV,			dev_oper_port_no_dev},
		{	DF_SWITCH_CM_PORT_DEV_LOCAL,		dev_oper_port_dev_local},
		{	DF_SWITCH_CM_PORT_DEV_SHARE,		dev_oper_port_dev_share},
		{	DF_SWITCH_CM_PORT_RED_OFF_GREN_OFF,	dev_oper_red_off_gren_off},
		{	DF_SWITCH_CM_PORT_RED_ON_GREN_OFF,	dev_oper_red_on_gren_off},
		{	DF_SWITCH_CM_PORT_RED_OFF_GREN_ON,	dev_oper_red_off_gren_on},
		{	DF_SWITCH_CM_PORT_RED_ON_GREN_TWI,	dev_oper_red_on_gren_twi},
		{	DF_SWITCH_CM_PORT_RED_ON_GREN_ON,	dev_oper_red_on_gren_on},
		{	DF_SWITCH_CM_PORT_RED_TWI_GREN_ON,	dev_oper_red_twi_gren_on},
		{	DF_SWITCH_CM_PORT_RED_TWI_GREN_TWI_1S,	dev_oper_red_twi_gren_twi_1s},
	
		{	DF_SWITCH_CM_GET_KEY_VALUE,			dev_oper_get_key_value},
		{DF_SWITCH_CM_PORT_RED_OFF_GREN_TWI,	dev_oper_red_off_gren_twi},
		{DF_SWITCH_CM_PORT_RED_TWI_GREN_OFF,dev_oper_red_twi_gren_off},
		#ifdef MTK_OPENWRT
			{ DF_SWITCH_CM_PORT_RED_TWI_GREN_TWI,  dev_oper_red_twi_gren_twi },
			{	DF_SWITCH_CM_STATUS_SET,			dev_status_set },
		#else
			{	DF_SWITCH_CM_LCD_SCREEN_PRINT,		dev_oper_lcd_screen_print },
			{ DF_SWITCH_CM_MAC_RESTART_POWER, dev_restart_mac_ic },
			{ DF_SWITCH_CM_GET_ALL_PORT_STATUS, dev_port_get_all_port_status },
			{ DF_SWITCH_CM_CONTRL_FAN,		dev_control_fan },
			{DF_SWITCH_CM_PORT_DOUBLE_LIGHT_YELLOW,dev_oper_double_light_yellow},
			{DF_SWITCH_CM_GET_AD_STATUS,dev_get_ad_status},
			{DF_SWITCH_CM_BLUETOOTH_MODE,dev_bluetooth_mode_change},
			{ DF_SWITCH_CM_RESTORE, dev_restore },
			{DF_SWITCH_CM_BLUETOOTH_POWER,dev_bluetooth_power},
			
		#endif
			{ DF_SWITCH_CM_GET_SUPPORT,dev_get_support },
        };

        static const struct _file_fuc	switch_fuc=
		{	.open=switch_dev_open,
			.read=NULL,
			.write=NULL,
			.ctl=switch_dev_ctl,
			.close=switch_dev_close,
			.del=NULL
		};
    
    #endif

        
#endif
