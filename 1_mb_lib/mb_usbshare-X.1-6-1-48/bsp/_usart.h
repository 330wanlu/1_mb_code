#ifndef _USART_DEFINE
	#define _USART_DEFINE
	#include "../core/_seria.h"
	#include <termios.h> 
	#include "../linux_s/_task.h"
	#include <fcntl.h>
	#ifdef _usart_c
		#include "_usart_linux.c"
	#endif
	/*=================================立即数定义========================================*/
	#ifdef _usart_c
		#define e_usart
		#define out(s,arg...)							
	#else
		#define e_usart						extern 
	#endif
	/*===================================结构体==========================================*/
	#ifdef _usart_c
        struct _usart
		{	int fd;
			int task;
			void (*write_data)(struct _seria	*seria,uint8 *buf,int buf_len);
			struct _seria	*seria;
			int aa;
		};
        #if 0
        struct _linux_app_usart
        {   char *linux;
            char *app;
        };
        #endif
        
	#endif	
	/*=================================函数定义==========================================*/
	e_usart	void _linux_usart_load(int hard_type);
	#ifdef _usart_c
		static int _usart_open(struct _seria	*data,void *o_arg);
		static void _usart_close(void *bsp_data);
		static int _usart_write(void *bsp_data,uint8 *buf,int buf_len);
		static void _usart_hook(void *arg,int o_timer);
		static  int _usart_set_bps(void *bsp_data,int bps);
	#endif
	/*==================================linux usart======================================*/
	#ifdef _usart_c
		#if 0
        static const struct _linux_app_usart 	linux_usart[]=
		
		{	{	"/dev/s3c2410_serial1","seria0"	},
			{	"/dev/s3c2410_serial2","seria1"	},
			{	"/dev/s3c2410_serial3","print_seria"	}
		};
		#endif
		#if 0
		{	{	"/dev/ttySAC1","seria0"	},
			{	"/dev/ttySAC2","seria1"	},
			{	"/dev/ttySAC3","print_seria"	}
		};
		#endif
		static const struct _seria_bsp_fuc	us_fuc=	
		{	.open=_usart_open,
			.close=_usart_close,
			.write=_usart_write,
			.bps=_usart_set_bps
		};
	#endif
#endif

