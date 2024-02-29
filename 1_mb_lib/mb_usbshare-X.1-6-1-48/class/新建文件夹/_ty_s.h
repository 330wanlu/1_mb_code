#ifndef _TY_S_DEFINE
	#define _TY_S_DEFINE
	#include "_ty_core.h"
	#include "../file/_file.h"
	#include "../core/_seria.h"
	/*==============================立即数定义=====================================*/
	#ifdef _ty_s_c
		#define out(s,arg...)					
		#define out1(s,arg...)								
		#define e_ty_s_c
	#else
		#define e_ty_s_c						extern
	#endif
	/*==============================结构体定义====================================*/
	#ifdef _ty_s_c
		struct _ty_s
		{	int state;
			int fd;
			int lock;
			int task;
			int class_fd;
			struct _ty_core	*core;
			struct	_ty_agree_class *buf;
		};
	
	#endif	
	
	/*===============================函数定义====================================*/
	#ifdef _ty_s_c
		static int ty_s_open(struct ty_file	*file);
		static int ty_s_close(void *pro_data,int fd);
		static void seria_hook(void *data,struct	_ty_agree *buf);
		static void oper_task(void *data,int timer);
		static int send_buf(struct _ty_s	*ty_s,int order,int ls,int result,
		uint8 *buf,int buf_len);
		static void ty_s_task(void *arg);
		static int ty_s_del(void *data,int c);
	#endif		
	
	/*==============================全局变量定义================================*/
	#ifdef _ty_s_c
		static const struct _file_fuc		_ty_s_file_fuc=
		{	.open=ty_s_open,
			.close=ty_s_close,
			.del=ty_s_del
		};
	#endif		
#endif

