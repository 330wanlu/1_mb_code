#ifndef _MEM_OPER_DEFINE 
	#define _MEM_OPER_DEFINE
	#include "_file.h"
	#include "../class/_ty_m.h"
	/*=============================立即数定义====================================*/
	#ifdef _mem_oper_c
		#define out(s,arg...)			//log_out("mem_oper.c",s,##arg)
		#define e_mem_oper
		#define DF_MEM_BUF_MAX_NUMBER		512
	#else
		#define e_mem_oper				extern
	#endif
	
	/*==============================函数定义====================================*/
	e_mem_oper	int mem_read(void *mem_dev,void *ty,void *buf,int len);
	e_mem_oper	int mem_write(void *mem_dev,void *ty,void *buf,int len);
	e_mem_oper	int mem_hook(void *mem_dev,void *hook,void *arg);

#endif
