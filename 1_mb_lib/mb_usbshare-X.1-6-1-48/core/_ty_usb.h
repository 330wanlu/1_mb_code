#ifndef _TY_USB_DEFINE
    #define _TY_USB_DEFINE
    /*======================================包含文件================================================*/
    #include "_switch_dev.h"
    #include "../linux_s/_usb_file.h"
    #include "_ty_pd.h"
	//#include "../linux_s/_t_file_c.h"
    /*=====================================立即数定义===============================================*/
        /*----------------------------------参数定义--------------------------------------------*/
		//#define MB1601_OR_MB1602					0 //MB1601为1，MB1602为0
        #define DF_TY_USB_CM_GET_BUSID              100                         //获取主busid
        #define DF_TY_USB_CM_SET_BUSID              (DF_TY_USB_CM_GET_BUSID+1)  //设置主busid
        #define DF_TY_USB_CM_SET_PORT_N             (DF_TY_USB_CM_SET_BUSID+1)  //设置排列顺序
        #define DF_TY_USB_CM_GET_PORT_N             (DF_TY_USB_CM_SET_PORT_N+1) //获取排列顺序
        #define DF_TY_USB_CM_SET_ENUM_TIMER         (DF_TY_USB_CM_GET_PORT_N+1) //最大枚举时间
        #define DF_TY_USB_CM_GET_ENUM_TIMER         (DF_TY_USB_CM_SET_ENUM_TIMER+1) //获取最大枚举时间
        #define DF_TY_USB_CM_OPEN_PORT              (DF_TY_USB_CM_GET_ENUM_TIMER+1) //打开端口
        #define DF_TY_USB_CM_CLOSE_PORT             (DF_TY_USB_CM_OPEN_PORT+1)      //关闭端口
        #define DF_TY_USB_CM_PORT_BUSID             (DF_TY_USB_CM_CLOSE_PORT+1)      //获取端口的busid
        #define DF_TY_USB_CM_PORT_NUMBER            (DF_TY_USB_CM_PORT_BUSID+1)     //获取端口总数量
        #define DF_TY_USB_CM_JUGE_PORT              (DF_TY_USB_CM_PORT_NUMBER+1)    //判断端口的合法性
        #define DF_TY_USB_CM_GET_PORT_MA_ID         (DF_TY_USB_CM_JUGE_PORT+1)      //获取端口所属机柜编号
        #define DF_TY_USB_CM_GET_PORT_PCB_ID        (DF_TY_USB_CM_GET_PORT_MA_ID+1) //获取端口所在的PCB
        #define DF_TY_USB_CM_GET_BUSID_PORT         (DF_TY_USB_CM_GET_PORT_PCB_ID+1)    //通过busid获取port
		#define DF_TY_USB_CM_GET_REGISTER_WHETHER	(DF_TY_USB_CM_GET_BUSID_PORT+1)
		#define DF_TY_USB_CM_OPEN_PORT_POWER		(DF_TY_USB_CM_GET_REGISTER_WHETHER+1)//仅仅打开端口电源
		#define DF_TY_USB_CM_GET_PORT_FILE_EXIST	(DF_TY_USB_CM_OPEN_PORT_POWER+1)//检查usb文件是否存在
		#define DF_TY_USB_CM_GET_TRYOUT_WHETHER		(DF_TY_USB_CM_GET_PORT_FILE_EXIST+1)//获取试用期限
		#define DF_TY_USB_CM_PORT_SEV_LED_NO		(DF_TY_USB_CM_GET_TRYOUT_WHETHER+1)
		#define DF_TY_USB_CM_PORT_SEV_LED_LOCAL		(DF_TY_USB_CM_PORT_SEV_LED_NO+1)
		#define DF_TY_USB_CM_PORT_SEV_LED_SHARE		(DF_TY_USB_CM_PORT_SEV_LED_LOCAL+1)
		#define DF_TY_USB_CM_PORT_CLOSE_PORT_POWER	(DF_TY_USB_CM_PORT_SEV_LED_SHARE+1)
		#define DF_TY_USB_CM_GET_MACHINE_ID			(DF_TY_USB_CM_PORT_CLOSE_PORT_POWER+1)
		#define DF_TY_USB_CM_CLOSE_PORT_POWER		(DF_TY_USB_CM_GET_MACHINE_ID+1)
		#define DF_TY_USB_CM_PORT_RED_OFF_GREN_OFF	(DF_TY_USB_CM_CLOSE_PORT_POWER+1)
		#define DF_TY_USB_CM_PORT_RED_ON_GREN_OFF	(DF_TY_USB_CM_PORT_RED_OFF_GREN_OFF+1)
		#define DF_TY_USB_CM_PORT_RED_OFF_GREN_ON	(DF_TY_USB_CM_PORT_RED_ON_GREN_OFF+1)
		#define DF_TY_USB_CM_PORT_RED_ON_GREN_TWI	(DF_TY_USB_CM_PORT_RED_OFF_GREN_ON+1)
		#define DF_TY_USB_CM_PORT_RED_ON_GREN_ON	(DF_TY_USB_CM_PORT_RED_ON_GREN_TWI+1)
		#define DF_TY_USB_CM_PORT_RED_TWI_GREN_ON	(DF_TY_USB_CM_PORT_RED_ON_GREN_ON+1)
		#define DF_TY_USB_CM_PORT_RED_TWI_GREN_TWI_1S	(DF_TY_USB_CM_PORT_RED_TWI_GREN_ON+1)
		#define DF_TU_USB_CM_PORT_GET_KEY_VALUE		(DF_TY_USB_CM_PORT_RED_TWI_GREN_TWI_1S+1)
		#define DF_TY_USB_CM_PORT_RED_TWI_GREN_TWI	(DF_TU_USB_CM_PORT_GET_KEY_VALUE+1)
		#define DF_TY_USB_CM_PORT_RED_OFF_GREN_TWI	(DF_TY_USB_CM_PORT_RED_TWI_GREN_TWI+1)
		#define DF_TY_USB_CM_PORT_RED_TWI_GREN_OFF	(DF_TY_USB_CM_PORT_RED_OFF_GREN_TWI+1)

		#define DF_TY_USB_CM_PORT_GET_ALL_PORT_STATUS	(DF_TY_USB_CM_PORT_RED_TWI_GREN_OFF+1)
		#define DF_TY_USB_CM_PORT_SET_FAN	(DF_TY_USB_CM_PORT_GET_ALL_PORT_STATUS+1)

		#define DF_TY_USB_CM_GET_AD_STATUS	(DF_TY_USB_CM_PORT_SET_FAN+1)
		#define DF_TY_USB_CM_RESTORE	(DF_TY_USB_CM_GET_AD_STATUS+1)
		#define DF_TY_USB_CM_PORT_DOUBLE_LIGHT_YELLOW	(DF_TY_USB_CM_RESTORE+1)
		#define DF_TY_USB_CM_CTL_BLUETOOTH_MODE			(DF_TY_USB_CM_PORT_DOUBLE_LIGHT_YELLOW+1)
		#define DF_TY_USB_CM_BLUETOOTH_POWER			(DF_TY_USB_CM_CTL_BLUETOOTH_MODE+1)
		#define DF_TY_USB_CM_CONN		DF_TY_USB_CM_BLUETOOTH_POWER+1
    #ifdef _ty_usb_c
        #ifdef DEBUG
			#define out(s,arg...)       				//log_out("all.c",s,##arg) 
		#else
#define out(s,arg...)	
		#endif	
		#define e_ty_usb
    #else    
        #define e_ty_usb                    extern    
    #endif
    
    #ifdef _ty_usb_c
        #define DF_TY_BUS_DEV_NUMBER        127                 //BUS承载的总数量
        #define DF_TY_USB_HUB_OPEN			1					//上电后,HUB一直开着
		#define DF_TY_USB_HUB_CLOSE			0					//上电后,只有打开端口HUB才开
    #endif
    
    
    
    
    /*=====================================结构体定义=================================================*/
    #ifdef _ty_usb_c    
        struct _ty_usb_fd
        {	int state;
            struct ty_file_mem	*mem;
            void *mem_dev;
            struct ty_file_fd	*mem_fd;
            struct _ty_usb  *dev;
        };
        
        struct _ty_usb_pcb                                  //单板PCB
        {   uint8 id[6];                                    //单板id
            uint16 n;                                       //USB数量
            int state;                                      //该PCB状态
            uint8 hub_state[8];                             //HUB的状态
        };
        
        struct _ty_usb_class
        {   uint8 id[6];                                    //机柜的id
            struct _ty_usb_pcb      pcb[8];                 //单板属性描述
            uint16 n;                                       //单板数量
            int usb_port_n;                                 //USB端口数量
            uint8 soft[8];                                  //软件版本
            uint8 hard[8];                                  //硬件版本
            int port_state[DF_TY_USB_PORT_N];               //端口状态
            int port_lock[DF_TY_USB_PORT_N];                //端口锁
            char busid[DF_TY_USB_BUSID];                    //主芯片busid
        };
        
        struct _ty_usb_dev
        {   struct _ty_usb_class    dev;                    //设备 
            struct _switch_dev_id   self_id;                //自己本身的ID
            struct _stm32_infor     stm32_infor;            //STM32 INFOR
        };
        
        struct _ty_usb
        {   struct _ty_usb_dev     data;                    //USB数据
            int lock;                                       //数据锁
			int type;										//机器类型
            int state;                                      //状态
            struct _ty_usb_fd   fd[DF_FILE_OPEN_NUMBER];    //各个句柄状态                          
            int enum_timer;                                 //枚举最大时间
            int port_n;                                     //端口排列顺序
            char switch_name[DF_FILE_NAME_MAX_LEN];         //开关名
            int switch_fd;                                  //开关fd
			int hub;										//HUB是否一直打开着
        };
        
        struct _ty_usb_ctl_fu
        {   int cm;
            int (*ctl)(struct _ty_usb_fd   *id,va_list args);
        };

		void *ty_usb_id;
    #endif
    /*===================================函数定义================================================*/
    e_ty_usb    int ty_usb_add(char *switch_name,int type);
	e_ty_usb 	int get_busid_info(int port,char *busid);
    #ifdef _ty_usb_c
        static int ty_usb_open(struct ty_file	*file);
        static int ty_usb_ctl(void *data,int fd,int cm,va_list args);
        static int ty_usb_close(void *data,int fd);
        static int get_busid_exist(char *busid);
        static int _set_busid(struct _ty_usb_fd *id,va_list args);
        static int _get_busid(struct _ty_usb_fd *id,va_list args);
        static int _set_port_n(struct _ty_usb_fd *id,va_list args);
        static int _get_port_n(struct _ty_usb_fd *id,va_list args);
        static int _set_enum_timer(struct _ty_usb_fd *id,va_list args);
        static int _get_enum_timer(struct _ty_usb_fd *id,va_list args);
        static int _open_port(struct _ty_usb_fd *id,va_list args);
		static int _open_port_power(struct _ty_usb_fd *id,va_list args);
		static int _close_port_power(struct _ty_usb_fd *id,va_list args);
		static int _get_port_exist(struct _ty_usb_fd *id,va_list args);
        static int _close_port(struct _ty_usb_fd *id,va_list args);
		static int _close_port_power_just(struct _ty_usb_fd *id,va_list args);
        static int get_busid(struct _ty_usb_fd *id,va_list args);
        static int get_port_number(struct _ty_usb_fd *id,va_list args);
        static int juge_port(struct _ty_usb_fd *id,va_list args);
        static int get_port_maid(struct _ty_usb_fd *id,va_list args);
        static int get_port_pcb(struct _ty_usb_fd *id,va_list args);
        //static int pd_oper(struct _ty_usb_fd *id,va_list args);
        static int _get_busid2port(struct _ty_usb_fd *id,va_list args);
		static int _get_register_whether(struct _ty_usb_fd *id,va_list args);
		static int _get_tryout_whether(struct _ty_usb_fd *id,va_list args);
		static int _get_machine_id(struct _ty_usb_fd *id, va_list args);
  		//static int juge_sub_hub(struct _ty_usb  *stream,int port);
		//static int juge_pcb_hub(struct _ty_usb  *stream,int port);
		static int _set_led_red_off_gren_off(struct _ty_usb_fd *id, va_list args);
		static int _set_led_red_on_gren_off(struct _ty_usb_fd *id, va_list args);
		static int _set_led_red_off_gren_on(struct _ty_usb_fd *id, va_list args);
		static int _set_led_red_on_gren_twi(struct _ty_usb_fd *id, va_list args);
		static int _set_led_red_on_gren_on(struct _ty_usb_fd *id, va_list args);
		static int _set_led_red_twi_gren_on(struct _ty_usb_fd *id, va_list args);
		static int _set_led_read_off_green_twi(struct _ty_usb_fd *id, va_list args);
		static int _set_led_read_twi_green_off(struct _ty_usb_fd *id, va_list args);
      #ifdef MTK_OPENWRT
	  	static int _set_led_red_twi_gren_twi(struct _ty_usb_fd *id, va_list args);
	  #endif
	    static int _set_led_red_twi_gren_twi_1s(struct _ty_usb_fd *id, va_list args);

        static int get_port(struct _ty_usb  *stream,int port);
        static int get_port_busid(struct _ty_usb  *stream,int port,char *busid);
        static int get_usb_file(char *busid,int enum_timer,int mach_type);
        static int get_busid_over_timer(const char *busid,int enum_timer);
        static int get_port(struct _ty_usb  *stream,int port);
        static int wait_usb_file_no(char *busid,int enum_timer);

		static int _ctl_port_no_dev_led(struct _ty_usb_fd *id,va_list args);
		static int _ctl_port_dev_local_led(struct _ty_usb_fd *id,va_list args);
		static int _ctl_port_dev_share_led(struct _ty_usb_fd *id,va_list args);
		static int _get_public_praviate_key(struct _ty_usb_fd *id, va_list args);

		static int _get_all_port_status(struct _ty_usb_fd *id, va_list args);
		static int _set_fan(struct _ty_usb_fd *id, va_list args);

		static int _get_all_ad_status(struct _ty_usb_fd *id, va_list args);
		
		static int _set_led_double_light_yellow(struct _ty_usb_fd *id, va_list args);

		static int _ctl_dev_restore(struct _ty_usb_fd *id, va_list args);

		static int _ctl_bluetooth_mode(struct _ty_usb_fd *id, va_list args);
		static int _ctl_bluetooth_power(struct _ty_usb_fd *id, va_list args);
		static int _ctl_dev_conn(struct _ty_usb_fd *id, va_list args);

    #endif  
    /*==================================常数定义=================================================*/
    #ifdef _ty_usb_c
        static const struct _file_fuc	ty_usb_fuc=
		{	.open=ty_usb_open,
			.read=NULL,
			.write=NULL,
			.ctl=ty_usb_ctl,
			.close=ty_usb_close,
			.del=NULL
		};
        static const struct _ty_usb_ctl_fu ctl_fun[]=
        {  
			//{   DF_TY_CORE_CM_PD,                   pd_oper             },
            {   DF_TY_USB_CM_GET_BUSID,             _get_busid          },
            {   DF_TY_USB_CM_SET_BUSID,             _set_busid          },
            {   DF_TY_USB_CM_SET_PORT_N,            _set_port_n         },
            {   DF_TY_USB_CM_GET_PORT_N,            _get_port_n         },
            {   DF_TY_USB_CM_SET_ENUM_TIMER,        _set_enum_timer     },
            {   DF_TY_USB_CM_GET_ENUM_TIMER,        _get_enum_timer     },
            {   DF_TY_USB_CM_OPEN_PORT,             _open_port          },
            {   DF_TY_USB_CM_PORT_BUSID,            get_busid           },
            {   DF_TY_USB_CM_PORT_NUMBER,           get_port_number     },
            {   DF_TY_USB_CM_CLOSE_PORT,            _close_port         },
            {   DF_TY_USB_CM_JUGE_PORT,             juge_port           },
            {   DF_TY_USB_CM_GET_PORT_MA_ID,        get_port_maid       },
            {   DF_TY_USB_CM_GET_PORT_PCB_ID,       get_port_pcb        },
            {   DF_TY_USB_CM_GET_BUSID_PORT,        _get_busid2port     },
			{	DF_TY_USB_CM_GET_REGISTER_WHETHER,	_get_register_whether},
			{	DF_TY_USB_CM_OPEN_PORT_POWER,		_open_port_power	},
			{	DF_TY_USB_CM_CLOSE_PORT_POWER,		_close_port_power	},
			{	DF_TY_USB_CM_GET_PORT_FILE_EXIST,	_get_port_exist		},
			{	DF_TY_USB_CM_GET_TRYOUT_WHETHER,	_get_tryout_whether	},
			{	DF_TY_USB_CM_PORT_SEV_LED_NO,		_ctl_port_no_dev_led},
			{	DF_TY_USB_CM_PORT_SEV_LED_LOCAL,	_ctl_port_dev_local_led},
			{	DF_TY_USB_CM_PORT_SEV_LED_SHARE,	_ctl_port_dev_share_led},
			{	DF_TY_USB_CM_PORT_CLOSE_PORT_POWER,	_close_port_power_just},
			{	DF_TY_USB_CM_GET_MACHINE_ID,		_get_machine_id		},
			{	DF_TY_USB_CM_PORT_RED_OFF_GREN_OFF,	_set_led_red_off_gren_off},
			{	DF_TY_USB_CM_PORT_RED_ON_GREN_OFF,	_set_led_red_on_gren_off},
			{	DF_TY_USB_CM_PORT_RED_OFF_GREN_ON,	_set_led_red_off_gren_on},
			{	DF_TY_USB_CM_PORT_RED_ON_GREN_TWI,	_set_led_red_on_gren_twi},
			{	DF_TY_USB_CM_PORT_RED_ON_GREN_ON,	_set_led_red_on_gren_on},
			{	DF_TY_USB_CM_PORT_RED_TWI_GREN_ON,	_set_led_red_twi_gren_on},
			{	DF_TU_USB_CM_PORT_GET_KEY_VALUE,	_get_public_praviate_key },
			{	DF_TY_USB_CM_PORT_RED_TWI_GREN_TWI_1S,_set_led_red_twi_gren_twi_1s},
			{ DF_TY_USB_CM_PORT_RED_OFF_GREN_TWI, _set_led_read_off_green_twi },
			{DF_TY_USB_CM_PORT_RED_TWI_GREN_OFF,_set_led_read_twi_green_off},

			#ifdef MTK_OPENWRT
			{	DF_TY_USB_CM_PORT_RED_TWI_GREN_TWI, _set_led_red_twi_gren_twi },
			#endif

			{ DF_TY_USB_CM_PORT_GET_ALL_PORT_STATUS, _get_all_port_status },
			{ DF_TY_USB_CM_PORT_SET_FAN, _set_fan },

			{DF_TY_USB_CM_GET_AD_STATUS	,_get_all_ad_status	},
			
			{DF_TY_USB_CM_PORT_DOUBLE_LIGHT_YELLOW,	_set_led_double_light_yellow},

			{DF_TY_USB_CM_RESTORE,	_ctl_dev_restore		},


			{DF_TY_USB_CM_CTL_BLUETOOTH_MODE,	_ctl_bluetooth_mode},
			{DF_TY_USB_CM_BLUETOOTH_POWER,		_ctl_bluetooth_power},

			{DF_TY_USB_CM_CONN		,			_ctl_dev_conn},
        };
        const char *usb_port_busid[]=
        {   "1.4",
            "1.3",
            "1.2",
            "1.1",
            "1.5",
            "1.6",
            "1.7",
            "2.4",
            "2.3",
            "2.2",
            "2.1",
            "2.5",
            "2.6",
            "2.7",
            "3.4",
            "3.3",
            "3.2",
            "3.1",
            "3.5",
            "3.6",
            "3.7",
            "4.4",
            "4.3",
            "4.2",
            "4.1",
            "4.5",
            "4.6",
            "4.7",
            "5.4",
            "5.3",
            "5.2",
            "5.1",
            "5.5",
            "5.6",
            "5.7",
            "6.4",
            "6.3",
            "6.2",
            "6.1",
            "6.5",
            "6.6",
            "6.7",
            "7.4",
            "7.3",
            "7.2",
            "7.1",
            "7.5",
            "7.6",
            "7.7"
        };


		const char *usb_port_busid_a33[]=
        {   "1.7",
            "1.6",
            "1.5",
            "1.1",
            "1.2",
            "1.3",
            "1.4",
            "2.7",
            "2.6",
            "2.5",
            "2.1",
            "2.2",
            "2.3",
            "2.4",
            "3.7",
            "3.6",
            "3.5",
            "3.1",
            "3.2",
            "3.3",
            "3.4",
            "4.7",
            "4.6",
            "4.5",
            "4.1",
            "4.2",
            "4.3",
            "4.4",
            "5.7",
            "5.6",
            "5.5",
            "5.1",
            "5.2",
            "5.3",
            "5.4",
            "6.7",
            "6.6",
            "6.5",
            "6.1",
            "6.2",
            "6.3",
            "6.4",
            "7.7",
            "7.6",
            "7.5",
            "7.1",
            "7.2",
            "7.3",
            "7.4"
        };
		static const char* busid_100[] =
		{
			"1-1.1.1.7",//1
			"1-1.1.1.6",//2
			"1-1.1.1.5",//3
			"1-1.1.1.1",//4
			"1-1.1.1.2",//5
			"1-1.1.1.3",//6
			"1-1.1.1.4",//7
			"1-1.1.2.7",//8
			"1-1.1.2.6",//9
			"1-1.1.2.5",//10
			"1-1.1.2.1",//11
			"1-1.1.2.2",//12
			"1-1.1.2.3",//13
			"1-1.1.2.4",//14
			"1-1.1.3.7",//15
			"1-1.1.3.6",//16
			"1-1.1.3.5",//17
			"1-1.1.3.1",//18
			"1-1.1.3.2",//19
			"1-1.1.3.3",//20
			"1-1.2.1.7",//1
			"1-1.2.1.6",//2
			"1-1.2.1.5",//3
			"1-1.2.1.1",//4
			"1-1.2.1.2",//5
			"1-1.2.1.3",//6
			"1-1.2.1.4",//7
			"1-1.2.2.7",//8
			"1-1.2.2.6",//9
			"1-1.2.2.5",//10
			"1-1.2.2.1",//11
			"1-1.2.2.2",//12
			"1-1.2.2.3",//13
			"1-1.2.2.4",//14
			"1-1.2.3.7",//15
			"1-1.2.3.6",//16
			"1-1.2.3.5",//17
			"1-1.2.3.1",//18
			"1-1.2.3.2",//19
			"1-1.2.3.3",//20
			"1-1.3.1.7",//1
			"1-1.3.1.6",//2
			"1-1.3.1.5",//3
			"1-1.3.1.1",//4
			"1-1.3.1.2",//5
			"1-1.3.1.3",//6
			"1-1.3.1.4",//7
			"1-1.3.2.7",//8
			"1-1.3.2.6",//9
			"1-1.3.2.5",//10
			"1-1.3.2.1",//11
			"1-1.3.2.2",//12
			"1-1.3.2.3",//13
			"1-1.3.2.4",//14
			"1-1.3.3.7",//15
			"1-1.3.3.6",//16
			"1-1.3.3.5",//17
			"1-1.3.3.1",//18
			"1-1.3.3.2",//19
			"1-1.3.3.3",//20
			"1-1.4.1.7",//1
			"1-1.4.1.6",//2
			"1-1.4.1.5",//3
			"1-1.4.1.1",//4
			"1-1.4.1.2",//5
			"1-1.4.1.3",//6
			"1-1.4.1.4",//7
			"1-1.4.2.7",//8
			"1-1.4.2.6",//9
			"1-1.4.2.5",//10
			"1-1.4.2.1",//11
			"1-1.4.2.2",//12
			"1-1.4.2.3",//13
			"1-1.4.2.4",//14
			"1-1.4.3.7",//15
			"1-1.4.3.6",//16
			"1-1.4.3.5",//17
			"1-1.4.3.1",//18
			"1-1.4.3.2",//19
			"1-1.4.3.3",//20
			"1-1.5.1.7",//1
			"1-1.5.1.6",//2
			"1-1.5.1.5",//3
			"1-1.5.1.1",//4
			"1-1.5.1.2",//5
			"1-1.5.1.3",//6
			"1-1.5.1.4",//7
			"1-1.5.2.7",//8
			"1-1.5.2.6",//9
			"1-1.5.2.5",//10
			"1-1.5.2.1",//11
			"1-1.5.2.2",//12
			"1-1.5.2.3",//13
			"1-1.5.2.4",//14
			"1-1.5.3.7",//15
			"1-1.5.3.6",//16
			"1-1.5.3.5",//17
			"1-1.5.3.1",//18
			"1-1.5.3.2",//19
			"1-1.5.3.3" //20
		};
		static const char* busid_20[] =
		{
			"1-1.1.7",
			"1-1.1.6",
			"1-1.1.5",
			"1-1.2.4",
			"1-1.2.7",
			"1-1.2.6",
			"1-1.2.5",
			"1-1.3.4",
			"1-1.3.7",
			"1-1.3.6",
			"1-1.1.4",
			"1-1.1.2",
			"1-1.1.1",
			"1-1.1.3",
			"1-1.2.3",
			"1-1.2.2",
			"1-1.2.1",
			"1-1.3.3",
			"1-1.3.2",
			"1-1.3.1"
		};


		static const char* busid_120[] =
		{
			"1-1.1.1.1.7",			"1-1.1.1.1.6",			"1-1.1.1.1.5",			"1-1.1.1.1.1",			"1-1.1.1.1.2",
			"1-1.1.1.1.3",			"1-1.1.1.1.4",			"1-1.1.1.2.7",			"1-1.1.1.2.6",			"1-1.1.1.2.5",
			"1-1.1.1.2.1",			"1-1.1.1.2.2",			"1-1.1.1.2.3",			"1-1.1.1.2.4",			"1-1.1.1.3.7",
			"1-1.1.1.3.6",			"1-1.1.1.3.5",			"1-1.1.1.3.1",			"1-1.1.1.3.2",			"1-1.1.1.3.3",
			"1-1.1.1.4.7",			"1-1.1.1.4.6",			"1-1.1.1.4.5",			"1-1.1.1.4.1",			"1-1.1.2.1.7",
			"1-1.1.2.1.6",			"1-1.1.2.1.5",			"1-1.1.2.1.1",			"1-1.1.2.1.2",			"1-1.1.2.1.3",
			"1-1.1.2.1.4",			"1-1.1.2.2.7",			"1-1.1.2.2.6",			"1-1.1.2.2.5",			"1-1.1.2.2.1",
			"1-1.1.2.2.2",			"1-1.1.2.2.3",			"1-1.1.2.2.4",			"1-1.1.2.3.7",			"1-1.1.2.3.6",
			"1-1.1.2.3.5",			"1-1.1.2.3.1",			"1-1.1.2.3.2",			"1-1.1.2.3.3",			"1-1.1.2.4.7",
			"1-1.1.2.4.6",			"1-1.1.2.4.5",			"1-1.1.2.4.1",			"1-1.1.3.1.7",			"1-1.1.3.1.6",
			"1-1.1.3.1.5",			"1-1.1.3.1.1",			"1-1.1.3.1.2",			"1-1.1.3.1.3",			"1-1.1.3.1.4",
			"1-1.1.3.2.7",			"1-1.1.3.2.6",			"1-1.1.3.2.5",			"1-1.1.3.2.1",			"1-1.1.3.2.2",
			"1-1.1.3.2.3",			"1-1.1.3.2.4",			"1-1.1.3.3.7",			"1-1.1.3.3.6",			"1-1.1.3.3.5",
			"1-1.1.3.3.1",			"1-1.1.3.3.2",			"1-1.1.3.3.3",			"1-1.1.3.4.7",			"1-1.1.3.4.6",
			"1-1.1.3.4.5",			"1-1.1.3.4.1",			"1-1.1.4.1.7",			"1-1.1.4.1.6",			"1-1.1.4.1.5",
			"1-1.1.4.1.1",			"1-1.1.4.1.2",			"1-1.1.4.1.3",			"1-1.1.4.1.4",			"1-1.1.4.2.7",
			"1-1.1.4.2.6",			"1-1.1.4.2.5",			"1-1.1.4.2.1",			"1-1.1.4.2.2",			"1-1.1.4.2.3",
			"1-1.1.4.2.4",			"1-1.1.4.3.7",			"1-1.1.4.3.6",			"1-1.1.4.3.5",			"1-1.1.4.3.1",
			"1-1.1.4.3.2",			"1-1.1.4.3.3",			"1-1.1.4.4.7",			"1-1.1.4.4.6",			"1-1.1.4.4.5",
			"1-1.1.4.4.1",			"1-1.1.5.1.7",			"1-1.1.5.1.6",			"1-1.1.5.1.5",			"1-1.1.5.1.1",
			"1-1.1.5.1.2",			"1-1.1.5.1.3",			"1-1.1.5.1.4",			"1-1.1.5.2.7",			"1-1.1.5.2.6",
			"1-1.1.5.2.5",			"1-1.1.5.2.1",			"1-1.1.5.2.2",			"1-1.1.5.2.3",			"1-1.1.5.2.4",
			"1-1.1.5.3.7",			"1-1.1.5.3.6",			"1-1.1.5.3.5",			"1-1.1.5.3.1",			"1-1.1.5.3.2",
			"1-1.1.5.3.3",			"1-1.1.5.4.7",			"1-1.1.5.4.6",			"1-1.1.5.4.5",			"1-1.1.5.4.1"
		};

		//wang
		static const char* busid_60[] =
		{
			"1-1.1.1.4",
			"1-1.1.1.7",
			"1-1.1.1.6",
			"1-1.1.1.5",
			"1-1.1.2.7",
			"1-1.1.2.6",
			"1-1.1.2.5",
			"1-1.1.3.4",
			"1-1.1.3.7",
			"1-1.1.3.6",
			"1-1.1.3.5",
			"1-1.1.4.7",
			"1-1.1.4.6",
			"1-1.1.4.5",
			"1-1.2.5.7",
			"1-1.1.1.3",
			"1-1.1.1.2",
			"1-1.1.1.1",
			"1-1.1.2.4",
			"1-1.1.2.3",
			"1-1.1.2.2",
			"1-1.1.2.1",
			"1-1.1.3.3",
			"1-1.1.3.2",
			"1-1.1.3.1",
			"1-1.1.4.4",
			"1-1.1.4.3",
			"1-1.1.4.2",
			"1-1.1.4.1",
			"1-1.2.5.6",
			"1-1.2.1.4",
			"1-1.2.1.7",
			"1-1.2.1.6",
			"1-1.2.1.5",
			"1-1.2.2.7",
			"1-1.2.2.6",
			"1-1.2.2.5",
			"1-1.2.3.4",
			"1-1.2.3.7",
			"1-1.2.3.6",
			"1-1.2.3.5",
			"1-1.2.4.7",
			"1-1.2.4.6",
			"1-1.2.4.5",
			"1-1.2.5.5",
			"1-1.2.1.3",
			"1-1.2.1.2",
			"1-1.2.1.1",
			"1-1.2.2.4",
			"1-1.2.2.3",
			"1-1.2.2.2",
			"1-1.2.2.1",
			"1-1.2.3.3",
			"1-1.2.3.2",
			"1-1.2.3.1",
			"1-1.2.4.4",
			"1-1.2.4.3",
			"1-1.2.4.2",
			"1-1.2.4.1",
			"1-1.2.5.1"
		};

		static const char* busid_240[] =
		{
			"1-1.1.1.1.7",			"1-1.1.1.1.6",			"1-1.1.1.1.5",			"1-1.1.1.1.1",			"1-1.1.1.1.2",
			"1-1.1.1.1.3",			"1-1.1.1.1.4",			"1-1.1.1.2.7",			"1-1.1.1.2.6",			"1-1.1.1.2.5",
			"1-1.1.1.2.1",			"1-1.1.1.2.2",			"1-1.1.1.2.3",			"1-1.1.1.2.4",			"1-1.1.1.3.7",
			"1-1.1.1.3.6",			"1-1.1.1.3.5",			"1-1.1.1.3.1",			"1-1.1.1.3.2",			"1-1.1.1.3.3",
			"1-1.1.1.4.7",			"1-1.1.1.4.6",			"1-1.1.1.4.5",			"1-1.1.1.4.1",			"1-1.1.2.1.7",
			"1-1.1.2.1.6",			"1-1.1.2.1.5",			"1-1.1.2.1.1",			"1-1.1.2.1.2",			"1-1.1.2.1.3",
			"1-1.1.2.1.4",			"1-1.1.2.2.7",			"1-1.1.2.2.6",			"1-1.1.2.2.5",			"1-1.1.2.2.1",
			"1-1.1.2.2.2",			"1-1.1.2.2.3",			"1-1.1.2.2.4",			"1-1.1.2.3.7",			"1-1.1.2.3.6",
			"1-1.1.2.3.5",			"1-1.1.2.3.1",			"1-1.1.2.3.2",			"1-1.1.2.3.3",			"1-1.1.2.4.7",
			"1-1.1.2.4.6",			"1-1.1.2.4.5",			"1-1.1.2.4.1",			"1-1.1.3.1.7",			"1-1.1.3.1.6",
			"1-1.1.3.1.5",			"1-1.1.3.1.1",			"1-1.1.3.1.2",			"1-1.1.3.1.3",			"1-1.1.3.1.4",
			"1-1.1.3.2.7",			"1-1.1.3.2.6",			"1-1.1.3.2.5",			"1-1.1.3.2.1",			"1-1.1.3.2.2",
			"1-1.1.3.2.3",			"1-1.1.3.2.4",			"1-1.1.3.3.7",			"1-1.1.3.3.6",			"1-1.1.3.3.5",
			"1-1.1.3.3.1",			"1-1.1.3.3.2",			"1-1.1.3.3.3",			"1-1.1.3.4.7",			"1-1.1.3.4.6",
			"1-1.1.3.4.5",			"1-1.1.3.4.1",			"1-1.1.4.1.7",			"1-1.1.4.1.6",			"1-1.1.4.1.5",
			"1-1.1.4.1.1",			"1-1.1.4.1.2",			"1-1.1.4.1.3",			"1-1.1.4.1.4",			"1-1.1.4.2.7",
			"1-1.1.4.2.6",			"1-1.1.4.2.5",			"1-1.1.4.2.1",			"1-1.1.4.2.2",			"1-1.1.4.2.3",
			"1-1.1.4.2.4",			"1-1.1.4.3.7",			"1-1.1.4.3.6",			"1-1.1.4.3.5",			"1-1.1.4.3.1",
			"1-1.1.4.3.2",			"1-1.1.4.3.3",			"1-1.1.4.4.7",			"1-1.1.4.4.6",			"1-1.1.4.4.5",
			"1-1.1.4.4.1",			"1-1.1.5.1.7",			"1-1.1.5.1.6",			"1-1.1.5.1.5",			"1-1.1.5.1.1",
			"1-1.1.5.1.2",			"1-1.1.5.1.3",			"1-1.1.5.1.4",			"1-1.1.5.2.7",			"1-1.1.5.2.6",
			"1-1.1.5.2.5",			"1-1.1.5.2.1",			"1-1.1.5.2.2",			"1-1.1.5.2.3",			"1-1.1.5.2.4",
			"1-1.1.5.3.7",			"1-1.1.5.3.6",			"1-1.1.5.3.5",			"1-1.1.5.3.1",			"1-1.1.5.3.2",
			"1-1.1.5.3.3",			"1-1.1.5.4.7",			"1-1.1.5.4.6",			"1-1.1.5.4.5",			"1-1.1.5.4.1",
			"1-1.2.1.1.7",			"1-1.2.1.1.6",			"1-1.2.1.1.5",			"1-1.2.1.1.1",			"1-1.2.1.1.2",
			"1-1.2.1.1.3",			"1-1.2.1.1.4",			"1-1.2.1.2.7",			"1-1.2.1.2.6",			"1-1.2.1.2.5",
			"1-1.2.1.2.1",			"1-1.2.1.2.2",			"1-1.2.1.2.3",			"1-1.2.1.2.4",			"1-1.2.1.3.7",
			"1-1.2.1.3.6",			"1-1.2.1.3.5",			"1-1.2.1.3.1",			"1-1.2.1.3.2",			"1-1.2.1.3.3",
			"1-1.2.1.4.7",			"1-1.2.1.4.6",			"1-1.2.1.4.5",			"1-1.2.1.4.1",			"1-1.2.2.1.7",
			"1-1.2.2.1.6",			"1-1.2.2.1.5",			"1-1.2.2.1.1",			"1-1.2.2.1.2",			"1-1.2.2.1.3",
			"1-1.2.2.1.4",			"1-1.2.2.2.7",			"1-1.2.2.2.6",			"1-1.2.2.2.5",			"1-1.2.2.2.1",
			"1-1.2.2.2.2",			"1-1.2.2.2.3",			"1-1.2.2.2.4",			"1-1.2.2.3.7",			"1-1.2.2.3.6",
			"1-1.2.2.3.5",			"1-1.2.2.3.1",			"1-1.2.2.3.2",			"1-1.2.2.3.3",			"1-1.2.2.4.7",			
			"1-1.2.2.4.6",			"1-1.2.2.4.5",			"1-1.2.2.4.1",			"1-1.2.3.1.7",			"1-1.2.3.1.6",
			"1-1.2.3.1.5",			"1-1.2.3.1.1",			"1-1.2.3.1.2",			"1-1.2.3.1.3",			"1-1.2.3.1.4",
			"1-1.2.3.2.7",			"1-1.2.3.2.6",			"1-1.2.3.2.5",			"1-1.2.3.2.1",			"1-1.2.3.2.2",
			"1-1.2.3.2.3",			"1-1.2.3.2.4",			"1-1.2.3.3.7",			"1-1.2.3.3.6",			"1-1.2.3.3.5",
			"1-1.2.3.3.1",			"1-1.2.3.3.2",			"1-1.2.3.3.3",			"1-1.2.3.4.7",			"1-1.2.3.4.6",
			"1-1.2.3.4.5",			"1-1.2.3.4.1",			"1-1.2.4.1.7",			"1-1.2.4.1.6",			"1-1.2.4.1.5",
			"1-1.2.4.1.1",			"1-1.2.4.1.2",			"1-1.2.4.1.3",			"1-1.2.4.1.4",			"1-1.2.4.2.7",
			"1-1.2.4.2.6",			"1-1.2.4.2.5",			"1-1.2.4.2.1",			"1-1.2.4.2.2",			"1-1.2.4.2.3",
			"1-1.2.4.2.4",			"1-1.2.4.3.7",			"1-1.2.4.3.6",			"1-1.2.4.3.5",			"1-1.2.4.3.1",
			"1-1.2.4.3.2",			"1-1.2.4.3.3",			"1-1.2.4.4.7",			"1-1.2.4.4.6",			"1-1.2.4.4.5",
			"1-1.2.4.4.1",			"1-1.2.5.1.7",			"1-1.2.5.1.6",			"1-1.2.5.1.5",			"1-1.2.5.1.1",
			"1-1.2.5.1.2",			"1-1.2.5.1.3",			"1-1.2.5.1.4",			"1-1.2.5.2.7",			"1-1.2.5.2.6",
			"1-1.2.5.2.5",			"1-1.2.5.2.1",			"1-1.2.5.2.2",			"1-1.2.5.2.3",			"1-1.2.5.2.4",
			"1-1.2.5.3.7",			"1-1.2.5.3.6",			"1-1.2.5.3.5",			"1-1.2.5.3.1",			"1-1.2.5.3.2",
			"1-1.2.5.3.3",			"1-1.2.5.4.7",			"1-1.2.5.4.6",			"1-1.2.5.4.5",			"1-1.2.5.4.1"
		};
    #endif        
            
        


#endif