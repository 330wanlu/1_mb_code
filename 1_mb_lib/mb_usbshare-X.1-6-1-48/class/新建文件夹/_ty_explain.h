#ifndef _TY_EXPLAIN_DEFINE
	#define _TY_EXPLAIN_DEFINE
	#include "_ty_core.h"
	#include "_ty_m.h"
	/*==========================����������=================================*/
	#ifdef _ty_explain_c
		#define out(s,arg...)					
		#define e_ty_explain
	#else
		#define e_ty_explain				extern
	#endif
	/*==========================�ṹ�嶨��=================================*/
	
	
	/*===========================��������=================================*/
	e_ty_explain	int ty_explain(struct	_ty_agree	*data,uint8 *buf,int *tail); 




#endif
