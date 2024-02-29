#ifndef _TY_M_A_ORDER_DEFINE
	#define _TY_M_A_ORDER_DEFINE
	#include "_ty_m.h"
	/*===========================立即数定义===================================*/
	#ifdef _ty_m_a_order_c
		#define out(s,arg...)			
		#define e_ty_m_a_order
	#else
		#define e_ty_m_a_order			extern
	#endif
	/*=============================结构体=====================================*/
	#ifdef _ty_m_a_order_c
		struct ty_m_a_buf
		{	char *name;
			int order;
			int (*oper_buf)(uint8 *buf,int buf_len,va_list args);
		};
	#endif
	/*=============================函数定义==================================*/
	e_ty_m_a_order int ty_m_a_order(int order,uint8 *buf,int buf_len,
									va_list args);
	#ifdef _ty_m_a_order_c
		static int ty_m_a_order_read_buf(uint8 *buf,int buf_len,va_list args);

	#endif		
	/*=============================全局变量==================================*/
	#ifdef _ty_m_a_order_c
		static const struct ty_m_a_buf	order_buf[]=
		{	{	.name="读BUF处理",
				.order=DF_ORDER_ACTION_READ_BUF,
				.oper_buf=ty_m_a_order_read_buf
			}	
		};
		
	#endif	
#endif

