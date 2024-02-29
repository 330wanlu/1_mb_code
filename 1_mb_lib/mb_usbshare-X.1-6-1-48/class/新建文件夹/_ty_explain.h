#ifndef _TY_EXPLAIN_DEFINE
	#define _TY_EXPLAIN_DEFINE
	#include "_ty_core.h"
	#include "_ty_m.h"
	/*==========================立即数定义=================================*/
	#ifdef _ty_explain_c
		#define out(s,arg...)					
		#define e_ty_explain
	#else
		#define e_ty_explain				extern
	#endif
	/*==========================结构体定义=================================*/
	
	
	/*===========================函数定义=================================*/
	e_ty_explain	int ty_explain(struct	_ty_agree	*data,uint8 *buf,int *tail); 




#endif
