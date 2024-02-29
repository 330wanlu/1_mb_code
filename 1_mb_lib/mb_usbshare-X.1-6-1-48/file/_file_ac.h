#ifndef _FILE_AC_DEFINE
	#define _FILE_AC_DEFINE
	#include "../linux_s/_lock.h"
	#include "../manage/_err.h"	
	#include <stdarg.h>
	
	/*============================立即数定义=========================================*/
		/*-------------------------形参获取---------------------------------*/
		/*
		typedef char * va_list;
		#define _INTSIZEOF(n) ((sizeof(n)+sizeof(int)-1)&~(sizeof(int) - 1) )
		#define va_start(ap,v) ( ap = (va_list)&v + _INTSIZEOF(v) ) 
		#define va_arg(ap,t) ( *(t *)((ap += _INTSIZEOF(t)) - _INTSIZEOF(t)) ) 
		#define va_end(ap) ( ap = (va_list)0 )*/
		
		
		/*------------------------文件属性参数------------------------------*/
		#define DF_DEV_TY_SPI				100
		#define DF_DEV_TY_USART				DF_DEV_TY_SPI+1
		#define DF_DEV_TY_I2C				DF_DEV_TY_USART+1
		#define DF_DEV_TY_IO				DF_DEV_TY_I2C+1
		#define DF_DEV_TY_M					DF_DEV_TY_IO+1
		#define DF_DEV_TY_S					DF_DEV_TY_M+1
		#define DF_DEV_TY					DF_DEV_TY_S+1
		#define DF_DEV_TY_STM32_FILE		DF_DEV_TY+1	
		#define DF_DEV_TY_PRINTER			DF_DEV_TY_STM32_FILE+1
		#define DF_DEV_TY_FILE_EVENT		DF_DEV_TY_PRINTER+1
		#define DF_DEV_TY_PSAM				DF_DEV_TY_FILE_EVENT+1
		#define DF_DEV_TY_SOCKET_S			DF_DEV_TY_PSAM+1
		#define DF_DEV_TY_SOCKET_M			DF_DEV_TY_SOCKET_S+1
		#define DF_DEV_TY_CLASS_DATA		DF_DEV_TY_SOCKET_M+1
		#define DF_DEV_TY_SEV_TCP_DATA		DF_DEV_TY_CLASS_DATA+1	
		#define DF_DEV_TY_CLIENT_TCP_DATA	DF_DEV_TY_SEV_TCP_DATA+1
		#define DF_DEV_TY_BUZZER			DF_DEV_TY_CLIENT_TCP_DATA+1
		#define DF_DEV_TY_CASH_BOX			DF_DEV_TY_BUZZER+1
		#define DF_DEV_TY_ISO14443A			DF_DEV_TY_CASH_BOX+1
		#define DF_DEV_TY_SCAN				DF_DEV_TY_ISO14443A+1
        #define DF_DEV_TY_USB               DF_DEV_TY_SCAN+1
        #define DF_DEV_TY_USB_MANAGE        DF_DEV_TY_USB+1
        #define DF_DEV_TY_USB_DOWNLOAD      DF_DEV_TY_USB_MANAGE+1
        #define DF_DEV_TY_NET_DOWNLOAD      DF_DEV_TY_USB_DOWNLOAD+1
		#define DF_DEV_TY_SD_DOWNLOAD		DF_DEV_TY_NET_DOWNLOAD+1
        #define DF_DEV_TY_USBIP             DF_DEV_TY_SD_DOWNLOAD+1
        #define DF_DEV_TY_SWITCH_DEV        DF_DEV_TY_USBIP+1
        #define DF_DEV_TY_PD_DEV            DF_DEV_TY_SWITCH_DEV+1
        #define DF_DEV_TY_EVENT             DF_DEV_TY_PD_DEV+1
        #define DF_DEV_DEPLOY               DF_DEV_TY_EVENT+1
        #define DF_DEV_USB_M                DF_DEV_DEPLOY+1
        #define DF_MACHINE_INFOR            DF_DEV_USB_M+1
		#define DF_DEV_GET_NET_STATE		DF_MACHINE_INFOR+1
		#define DF_DEV_TY_SOCKET_CLIENT		DF_DEV_GET_NET_STATE+1
		#define	DF_DEV_REGISTER				DF_DEV_TY_SOCKET_CLIENT+1	
		#define DF_DEV_GET_NET_TIME			DF_DEV_REGISTER+1
		#define	DF_DEV_MB_UDP_S				DF_DEV_GET_NET_TIME+1
		#define DF_DEV_MB_HTTP_UPDATE		DF_DEV_MB_UDP_S+1
		#define DF_DEV_MB_UPDATE_FILE		DF_DEV_MB_HTTP_UPDATE+1
		#define DF_DEV_TRAN_DATA			DF_DEV_MB_UPDATE_FILE+1
		#define DF_DEV_VPN_SET				DF_DEV_TRAN_DATA+1
		#define DF_DEV_DEVICE_MANAGE		DF_DEV_VPN_SET+1		//add by whl
		#define DF_DEV_LCD_STATE			DF_DEV_DEVICE_MANAGE+1
		#define DF_DEV_M_SERVER				DF_DEV_LCD_STATE+1
		#define DF_DEV_INVOICE_TRANSMIT		DF_DEV_M_SERVER+1
		#define DF_DEV_BLUETOOTH			DF_DEV_INVOICE_TRANSMIT+1
		/*-------------------------文件相关数量-----------------------------*/
		#define DF_FILE_OPEN_NUMBER			10	//一个文件重复打开次数
		
		/*-------------------------错误代码定义-----------------------------*/
		#define DF_ERR_FILE_LOADED			DF_ERR_FILE		//文件已经加载
		#define DF_ERR_FILE_NO_SPACE		(DF_ERR_FILE_LOADED-1)	//文件没有空间加载
		#define DF_ERR_FILE_NO_DEV			(DF_ERR_FILE_NO_SPACE-1) //非DEV文件	
		#define DF_ERR_FILE_NO_FILE			(DF_ERR_FILE_NO_DEV-1)	//没有该文件
		#define DF_ERR_FILE_FD_ERR			(DF_ERR_FILE_NO_FILE-1)	//文件FD错误
		#define DF_ERR_FILE_NO_OPEN			(DF_ERR_FILE_FD_ERR-1)	//文件没有打开
		
		
		
		
		#ifdef file_ac_c
			#define out(s,arg...)						
			#define e_file_as
		#else	
			#define e_file_as				extern
		#endif
	/*============================结构体定义=========================================*/
	struct _ty_file_s_add								//多少文件对外
	{	char name[100];
		int pro;
		struct _ty_file_s_add		*next;
	};
	

	struct ty_file_fd
	{	int ty_fd;					//协议栈使用的fd
		int m_fd;					//本设备的FD
		int s_fd;					//下面设备的FD
	};
	
	
	struct _file_dir
	{
		char name[100];			//目录名称
		struct ty_file		*n_file;				//目录下文件,DEV设备
		struct _file_dir 	*back_dir;				//上一级目录
		int lock;									//竞争锁
		struct _file_dir	*n_file_dir;			//该目录下所有的目录
		int f_number;								//该目录下文件总数量
		int n_number;								//该目录下目录总数量
		struct ty_file		*o_file;				//与该目录相关的文件
		void *pro;									//私有数据
	};
	
	struct ty_file
	{	uint8 state;
		struct _file_fuc	*fuc;						//core函数
		char name[100];				//文件名称
		int pro;										//文件属性
		int lock;										//该文件锁
		void *o_arg;									
		void *pro_data;									//core相关私有数据
		struct ty_file_fd	fd[100];	//文件共享操作的数据
		struct _file_dir	*back_dir;					//上一级目录
		struct _file_dir	*o_dir;						//相关的目录
		int file_mod;									//文件打开的方式
		struct ty_file	*b_next;						//平级文件的下一个文件
		struct ty_file	*b_back;						//平级文件的上一个文件
	};
	
	struct _file_fuc
	{
		int(*open)(struct ty_file	*file);
		int(*read)(void *pro_data, int fd, uint8 *buf, int buf_len);
		int(*write)(void *pro_data, int fd, uint8 *buf, int buf_len);
		int(*ctl)(void *pro_data, int fd, int cm, va_list args);
		int(*close)(void *pro_data, int fd);
		int(*del)(void *pro_data, int c);
	};

	struct ty_file_mem
	{	int (*read)(void *mem_dev,void *ty,void *buf,int len);
		int (*write)(void *mem_dev,void *ty,void *buf,int len);
		int (*hook)(void *mem_dev,void *buf,void *arg);
	};
	
	struct ty_file_state
	{	uint8 state;
		struct ty_file	*file;	
	};
	
	
	struct ty_file_n
	{	struct ty_file_state	file_state[150];
		int lock;
	};
	
	#ifdef file_ac_c
		struct pro_ex
		{	int pro;
			char *ex;
		};
	
	#endif
	/*===============================函数定义===========================================*/
	e_file_as	int ty_file_load();
	e_file_as	int ty_file_add(int pro,void *pro_data,char *name,struct _file_fuc	*fuc);
	e_file_as	int ty_file_del(char *name);	
	e_file_as	void _ty_file_get_file(struct ty_file_n		*file_n);
	e_file_as	struct _ty_file_s_add 	*ty_get_file(void);	
	e_file_as	void ty_file_free(struct _ty_file_s_add 	*file);	
	/*================================属性中文定义======================================*/
	
	#ifdef file_ac_c
		static _err	file_err[]=
		{	{	.err=DF_ERR_FILE_LOADED,
				.name="文件已经加载"
			},
			{	.err=DF_ERR_FILE_NO_SPACE,
				.name="文件已经没有可用的空间"
			},
			{	.err=DF_ERR_FILE_NO_DEV,
				.name="不是DEV文件"
			},
			{	.err=DF_ERR_FILE_NO_FILE,
				.name="没有该文件"
			},
			{	.err=DF_ERR_FILE_FD_ERR,
				.name="文件fd错误"
			},
			{	.err=DF_ERR_FILE_NO_OPEN,
				.name="文件未打开"
			},
            {   .err=DF_DEV_USB_M,
                .name="ty_usb_m"
            },
		};
		static const _err_buf	file_err_buf=
		{	file_err,
			sizeof(file_err)/sizeof(_err),
			0
		};
		
		
		struct ty_file_n		file_stream;
	#else
		e_file_as	struct ty_file_n		file_stream;
	#endif	

#endif

