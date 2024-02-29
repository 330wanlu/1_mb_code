#ifndef _REGISTER_DEFINE
    #define _REGISTER_DEFINE
	#include "_port_core.h"
    #include "../file/_file.h"
	#include "../linux_sys/_http.h"
#ifdef MTK_OPENWRT
	#include "../linux_sys/_linux_ip_mtk.h"
#else
	#include "../linux_sys/_linux_ip.h"
#endif
	#include "../linux_sys/mqtt_client.h"
	#include "../linux_sys/_http_reg.h"
	/*=========================================立即数定义===============================================*/
		#define DF_TY_REGISTER_CM_WRITE                   	1               			//写注册
		#define DF_TY_REGISTER_CM_WHETHER 					DF_TY_REGISTER_CM_WRITE+1	//获取注册与否
		#define DF_TY_MQTT_REGISTER_CM_WHETHER 				DF_TY_REGISTER_CM_WHETHER+1	//获取是否在MQTT系统中注册
		#define DF_TY_MQTT_USRER_NAME_PASSWD 				DF_TY_MQTT_REGISTER_CM_WHETHER+1	//获取是否在MQTT系统中注册

		#define DF_M_SERVER_ADDRESS	"www.njmbxx.com"
		#define DF_M_SERVER_PORT 20020
		#define DF_M_SERVER_APPID "EE8B56879B7D5449933C1F8918C33FB6"
		#define DF_M_SERVER_APPSECERT "74993F73731FAE499F0F2A98CC4A9E90"


       
    #ifdef _register_c
         #ifdef DEBUG
			#define out(s,arg...)       					log_out("all.c",s,##arg) 
		#else
			#define out(s,arg...)							//log_out("all.c",s,##arg)
		#endif	
        #define e_register
    #else
        #define e_register                                    extern
    
    #endif
    
    
 
    
    /*=========================================结构体定义===============================================*/
    struct _register_fd
    {	int state;
        struct ty_file_mem	*mem;
        void *mem_dev;
        struct ty_file_fd	*mem_fd;
        struct _register  *dev;
    };
        

    struct _register
    {   int state;                                                      //本模块状态
		int lock;                                                       //数据锁
        int task_fd;                                                    //主任务号
		struct _register_fd fd[DF_FILE_OPEN_NUMBER];
		//uint8 hard_ware[20];//硬件版本最大8字节
		//uint8 soft_ware[20];//软件版本最大8字节
		//uint8 ser_num[30];//
		//uint8 id[20];//
		//uint8 onlyid[20];//
		//uint8 type[20];//
		char reg_err_info[1024];
		_so_note    *note;
    };
    

#ifdef _register_c

	struct _register_ctl_fu
	{
		int cm;
		int(*ctl)(struct _register_fd   *id, va_list args);
	};

#endif
	void *reg_stream_fd;

    /*========================================函数定义=================================================*/
	e_register    int _register_add(const _so_note    *note);
	e_register    int data_3des_base(uint8 *in_data, uint8 *out_data);
	e_register	  int get_reg_err_info(char *err_info);
    #ifdef _register_c
	static int _register_open(struct ty_file	*file);
	static int _register_ctl(void *data,int fd,int cm,va_list args);
	static int _register_close(void *data,int fd);

	static int _register_terminal_to_server(struct _register_fd *id,va_list args);
	static int _get_mqtt_server_info_from_local(struct _register_fd *id,va_list args);
	static int _get_mqtt_server_info_from_server(struct _register_fd *id, va_list args);

	 
	static int analysis_deal_respone_data(char *data, char *server_addr, int *port);
	static int save_cert_file(char *base_file, char *path);
	static int get_file_base64(char *file, char *buf, int len);


	//static int print_data(uint8 *data,int data_len);
    #endif
    /*======================================常数定义===================================================*/
    #ifdef _register_c
		static const struct _register_ctl_fu ctl_fun[] =
		{
			{ DF_TY_REGISTER_CM_WRITE, _register_terminal_to_server },
			{ DF_TY_MQTT_REGISTER_CM_WHETHER, _get_mqtt_server_info_from_server },
			{ DF_TY_MQTT_USRER_NAME_PASSWD, _get_mqtt_server_info_from_local }
		};

        static const struct _file_fuc	register_fuc=
		{	.open=_register_open,
			.read=NULL,
			.write=NULL,
			.ctl=_register_ctl,
			.close=_register_close,
			.del=NULL
		};
    #endif
#endif
