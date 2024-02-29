#ifndef _TY_S_A_ORDER_DEFINE
	#define _TY_S_A_ORDER_DEFINE
	#include "_ty_core.h"
	#include "../core/_ty_file_core.h"
	/*==================================立即数定义==================================*/
	#ifdef _ty_s_a_order_c
		#define out(s,arg...)
		#define out1(s,arg...)					
		#define e_ty_s_a_order
	#else
		#define e_ty_s_a_order							extern
	#endif
	/*===============================结构体定义=====================================*/
	#ifdef _ty_s_a_order_c
		typedef struct
		{	char *name;
			int order;
			int (*oper)(struct _ty_core	*core,uint8 *buf,int buf_len);
		}no_buf;
	#endif

	/*================================函数定义======================================*/
	e_ty_s_a_order int ty_s_a_order(struct _ty_core	*core,int order,uint8 *in_buf,
		uint8 **out_buf,int in_len,int *out_len);
	#ifdef _ty_s_a_order_c
		static int soft_state(struct _ty_core *core,uint8 *buf,int len);
		static int soft_read_buf(struct _ty_core	*core,uint8 *buf,uint8 **o_buf,
								int in_buf_len,int *o_buf_len);
		static int soft_infor(struct _ty_core	*core,uint8 *buf,int len);
		static int soft_err(struct _ty_core	*core,uint8 *buf,int len);
		static int soft_log(struct _ty_core	*core,uint8 *buf,int len);
		static int soft_write_buf(struct _ty_core	*core,uint8 *buf,int len);
		static int soft_hook_ctl(struct _ty_core	*core,uint8 *buf,int len);
		static int new_dev(struct _ty_core	*core,uint8 *buf,int len);
		static int del_dev(struct _ty_core	*core,uint8 *buf,int len);
		static int open_dev(struct _ty_core	*core,uint8 *buf,int len);
		static int close_dev(struct _ty_core	*core,uint8 *buf,int buf_len);
		static int read_dev(struct _ty_core	*core,uint8 *buf,int buf_len);
		static int write_dev(struct _ty_core	*core,uint8 *buf,int buf_len);
		static int ctl_dev(struct _ty_core	*core,uint8 *buf,int buf_len);
	#endif
	/*=============================全局变量定义===================================*/
	#ifdef _ty_s_a_order_c
		static const no_buf		s_a_order_buf[]=
		{	{	.name="版本信息",
				.order=DF_ORDER_ACTION_SOFT_INFOR,
				.oper=soft_infor
			},
			{	.name="错误信息",
				.order=DF_ORDER_ACTION_SOFT_ERR,
				.oper=soft_err
			},
			{	.name="调试信息",
				.order=DF_ORDER_ACTION_SOFT_LOG,
				.oper=soft_log
			},
			{	.name="写BUF",
				.order=DF_ORDER_ACTION_WRITE_BUF,
				.oper=soft_write_buf
			},
			{	.name="执行回调",
				.order=DF_ORDER_ACTION_HOOK,
				.oper=soft_hook_ctl
			},
			{	.name="系统信息",
				.order=DF_ORDER_ACTION_STATE,
				.oper=soft_state	
			},
			{	.name="创建文件",
				.order=DF_ORDER_FILE_NEW,
				.oper=new_dev
			},
			{	.name="删除文件",
				.order=DF_ORDER_FILE_DEL,
				.oper=del_dev
			},
			{	.name="打开文件",
				.order=DF_ORDER_FILE_OPEN,
				.oper=open_dev
			},
			{	.name="读文件",
				.order=DF_ORDER_FILE_READ,
				.oper=read_dev
			},
			{	.name="写文件",
				.order=DF_ORDER_FILE_WRITE,
				.oper=write_dev
			},
			{	.name="控制文件",
				.order=DF_ORDER_FILE_CTL,
				.oper=ctl_dev
			},
			{	.name="关闭文件",
				.order=DF_ORDER_FILE_CLOSE,
				.oper=close_dev
			}
		};
	#endif
#endif
