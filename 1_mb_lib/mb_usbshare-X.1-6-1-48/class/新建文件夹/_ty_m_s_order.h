#ifndef _TY_M_S_ORDER_DEFINE
	#define _TY_M_S_ORDER_DEFINE
	#include "_ty_core.h"
	//#include "../file/_file.h"
	#include <string.h>
	/*===========================����������================================*/
	#ifdef _ty_m_s_order_c
		#define out(s,arg...)						
		#define e_ty_m_s_order
	#else
		#define e_ty_m_s_order				extern	
	#endif

	/*=============================�ṹ����================================*/
	#ifdef _ty_m_s_order_c
		struct s_order
		{	char *name;
			int order;
			uint8 *(*pack)(va_list	*args,int *len);
		};
	#endif
	/*=============================��������================================*/
	e_ty_m_s_order	int ty_m_s_pack(int order,uint8 *buf,va_list *args);
		int ty_m_s_test(int order,...);
	#ifdef _ty_m_s_order_c
		/*------------------------���������-------------------------*/
		static uint8 *s_order_soft_infor(va_list *args,int *len);
		static uint8 *s_order_soft_err(va_list *args,int *len);
		static uint8 *s_order_soft_log(va_list *args,int *len);
		static uint8 *s_order_read_buf(va_list *args,int *len);
		static uint8 *s_order_write_buf(va_list *args,int *len);
		static uint8 *s_order_hook_ctl(va_list *args,int *len);
		static uint8 *s_order_state(va_list *args,int *len);
		/*------------------------�ļ������-------------------------*/
		static uint8 *s_order_new_dev(va_list *args,int *len);
		static uint8 *s_order_del_dev(va_list *args,int *len);
		static uint8 *s_order_open(va_list *args,int *len);
		static uint8 *s_order_read(va_list *args,int *len);
		static uint8 *s_order_write(va_list *args,int *len);
		static uint8 *s_order_ctl(va_list *args,int *len);
		static uint8 *s_order_close(va_list *args,int *len);
	#endif		

	/*===========================ȫ�ֱ���=====================================*/
	#ifdef _ty_m_s_order_c
		static const struct s_order	s_order_pack[]=
		{	
			/*-----------------��ض�����--------------------*/
			{	.name="�汾��Ϣ",
				.order=DF_ORDER_ACTION_SOFT_INFOR,
				.pack=s_order_soft_infor
			},
			{	.name="������Ϣ",
				.order=DF_ORDER_ACTION_SOFT_ERR,
				.pack=s_order_soft_err
			},
			{	.name="LOG��Ϣ",
				.order=DF_ORDER_ACTION_SOFT_LOG,
				.pack=s_order_soft_log
			},
			{	.name="��BUF",
				.order=DF_ORDER_ACTION_READ_BUF,
				.pack=s_order_read_buf
			},
			{	.name="дBUF",
				.order=DF_ORDER_ACTION_WRITE_BUF,
				.pack=s_order_write_buf
			},
			{	.name="�ص�HOOK",
				.order=DF_ORDER_ACTION_HOOK,
				.pack=s_order_hook_ctl
			},
			{	.name="ϵͳ״̬",
				.order=DF_ORDER_ACTION_STATE,
				.pack=s_order_state
			},
			
			/*-----------------------�ļ������----------------*/
			{	.name="�����ļ�",
				.order=DF_ORDER_FILE_NEW,
				.pack=s_order_new_dev
			},
			{	.name="ɾ���ļ�",
				.order=DF_ORDER_FILE_DEL,
				.pack=s_order_del_dev
			},
			{	.name="���ļ�",
				.order=DF_ORDER_FILE_OPEN,
				.pack=s_order_open
			},
			{	.name="���ļ�",
				.order=DF_ORDER_FILE_READ,
				.pack=s_order_read
			},
			{	.name="д�ļ�",
				.order=DF_ORDER_FILE_WRITE,
				.pack=s_order_write
			},
			{	.name="�����ļ�",
				.order=DF_ORDER_FILE_CTL,
				.pack=s_order_ctl
			},
			{	.name="�ر��ļ�",
				.order=DF_ORDER_FILE_CLOSE,
				.pack=s_order_close
			}
		};



	#endif

#endif


