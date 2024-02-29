#ifndef _TY_FILE_DEFINE
	#define _TY_FILE_DEFINE
	#include "_ty_core.h"
	/*=========================立即数定义======================================*/
	#ifdef _ty_file_c
		#define e_ty_file
		#define out(s,arg...)					
	#else
		#define e_ty_file						extern
	#endif
	/*==========================函数定义======================================*/
	#ifdef _ty_file_c
		static int ty_file_open(struct _ty_core *core,char *name);
		static int ty_file_close(struct _ty_core *core,int fd);
		static int ty_file_read(struct _ty_core *core,int fd,uint8 *buf,int buf_len);
		static int ty_file_write(struct _ty_core *core,int fd,uint8 *buf,int buf_len);
		static int ty_file_ctl(struct _ty_core *core,int fd,int cm,...);
		static int ty_core_read_buf(void *mem_dev,void *ty,void *buf,int len);
		static int ty_core_write_buf(void *mem_dev,void *ty,void *buf,int len);
		static int ty_core_hook(void *mem_dev,void *hook,void *arg);
		static int ty_core_state(struct _ty_core *core,int state);
		static int ty_core_note(struct _ty_core	*core,_so_note	*note);
		static int ty_core_err(struct _ty_core *core,int err,char *err_c);
		static int ty_core_new_file(struct _ty_core	*core,int pro,char *name);
	#endif
	/*============================全局变量定义================================*/
	#ifdef _ty_file_c
		const struct _ty_core_file	core_file=
		{	.open=ty_file_open,
			.close=ty_file_close,
			.read=ty_file_read,
			.write=ty_file_write,
			.ctl=ty_file_ctl
		};
		const struct ty_file_mem	ty_mem=
		{	.read=ty_core_read_buf,
			.write=ty_core_write_buf,
			.hook=ty_core_hook
		};
		const struct _ty_action		ty_action=
		{	.state=ty_core_state,
			.note=ty_core_note,
			.err=ty_core_err,
			.new_file=ty_core_new_file
		};
	#else
		extern const struct _ty_core_file	core_file;
		extern const struct ty_file_mem	ty_mem; 
		extern const struct _ty_action		ty_action;
	#endif
	


#endif

