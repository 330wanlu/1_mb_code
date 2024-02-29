#ifndef _TY_CORE_DEFINE
	#define _TY_CORE_DEFINE
	#include "../file/_file.h"
	#include "../core/_ty_class_data.h"
	#include "_ty_file.h"
	#include "_ty_m_s_order.h"
	#include "_ty_m_a_order.h"
	#include "_ty_s_a_order.h"
	#include "_ty_class_err.h"
	//#include "_ty_explain.h"
	
	#include "_ty_s.h"
	#include "_ty_m.h"
	
	/*===============================立即数定义=========================================*/
	#ifdef _ty_core_c
		#define out(s,arg...)									
		#define e_ty_core
		#define DF_TY_CORE_S_NAME					"ty_s"
		#define DF_TY_CORE_M_NAME					"ty_m"
		#define DF_TY_CORE_NAME						"ty"		
	#else
		#define e_ty_core								extern
	#endif
	/*===============================结构体定义=========================================*/
	struct _ty_core_out
	{	struct _ty_file_add_in	*file_add;			//文件注册所有的库
		struct _ty_file_s_add	*file;				//加载给对方的文件
		_so_note	*m_note;						//本机的描述文件
		int i_o;									//是否发送注册文件给对方
		int f_s;									//打开以后是否先发
		int heart_timer;							//心跳时间
		void (*err)(void *arg);						//发生错误后的回调
		void *arg;									//回调参数
		
	};
	
	
	//struct _ty_file_add									//文件增加
	//{	int pro;
	//	int (*add)(char *name,struct _ty_core_add_file	*file);
	//};
	
	struct _ty_file_add_in								
	{	struct _ty_file_add		*file_add;	
		int n;
	};
	
	
	
	
	//struct	_ty_agree 									//接受数据
	//{	int order;
	//	int ls;
	//	int len;
	//	int m_s;
	//	uint8 buf[DF_COM_MAX_LEN];
	//};
	
	//struct _ty_action
	//{	int (*state)(struct _ty_core *core,int state);
	//	int (*note)(struct _ty_core	*core,_so_note	*note);
	//	int (*err)(struct _ty_core	*core,int err,char *err_c);
	//	int (*new_file)(struct _ty_core	*core,int pro,char *name);
	//};
	
	
	
	
	
	struct _ty_core_add_file							//增加文件
	{	char name[DF_FILE_NAME_MAX_LEN];
		char self_name[DF_FILE_NAME_MAX_LEN];
		int pro;
		struct _ty_core	*core;
		struct _ty_core_add_file *next;
	};

	struct _ty_file_add									//文件增加
	{
		int pro;
		int(*add)(char *name, struct _ty_core_add_file	*file);
	};
	
	struct _ty_core_open_file							//打开文件处理
	{	char name[DF_FILE_NAME_MAX_LEN];
		int fd;
		struct _ty_core	*core;
		struct _ty_core_open_file	*next;
	};
	
	struct _ty_core_file
	{	int (*open)(struct _ty_core		*core,char *name);
		int (*read)(struct _ty_core		*core,int fd,uint8 *buf,int buf_len);
		int (*write)(struct _ty_core	*core,int fd,uint8 *buf,int buf_len);
		int (*close)(struct _ty_core	*core,int fd);
		int (*ctl)(struct _ty_core		*core,int fd,int cm,...);
	};
	
	struct _ty_heart
	{	int task;
		int o_timer;
		void (*err)(void *arg);
		void *arg;
		int err_n;
		struct _ty_core	*core;
		int state;
	};
	
	
	
	struct _ty_core										//主结构
	{	int state;
		char use_port[DF_FILE_NAME_MAX_LEN];
		struct _ty_file_add_in		*file_add;
		struct _ty_file_s_add		*file;
		_so_note					*m_note;			//本设备
		int i_o;										//上电是否发送信息
		int f_s;										//先运行，等待连接
		int queue;
		
		int (*s_a_order)(struct _ty_core	*core,int order,uint8 *in_buf,
						uint8 **out_buf,int in_len,int *out_len);		//接受到命令后的处理	
		int (*s_pack)(int order,uint8 *buf,va_list *args);		//打包发送
		int (*m_a_order)(int order,uint8 *buf,int buf_len,va_list args);//接受尾巴处理
		int (*explain)(struct	_ty_agree	*data,uint8 *buf,int *tail);	//解析	
		void (*self_m)(void *arg);
		struct _ty_core_file	*core_file;
		struct ty_file_mem		*mem;
		struct _ty_action		*action;
		void *mem_dev;
		
		
		char dev_name[DF_FILE_NAME_MAX_LEN];
		char ty_m_name[DF_FILE_NAME_MAX_LEN];
		char ty_s_name[DF_FILE_NAME_MAX_LEN];
		char ty_name[DF_FILE_NAME_MAX_LEN];
		char ty_class_name[DF_FILE_NAME_MAX_LEN];
		char ty_class_m_name[DF_FILE_NAME_MAX_LEN];
		
		void *ty_m;
		void *ty_s;
		int m_state;
		int s_state;
		
		int ty_m_fd;
		int ty_s_fd;
		
		
		struct _ty_core_add_file	*add_file;
		struct _ty_core_open_file	*open_file;
		_so_note	note;
		int close;
		struct _ty_heart	heart;
	};

	struct _ty_action
	{	
	int (*state)(struct _ty_core *core,int state);
	int (*note)(struct _ty_core	*core,_so_note	*note);
	int (*err)(struct _ty_core	*core,int err,char *err_c);
	int (*new_file)(struct _ty_core	*core,int pro,char *name);
	};
	/*==================================函数定义==========================================*/
	e_ty_core	int ty_core_add(char *use_port,struct _ty_core_out	*app_in);
	#ifdef _ty_core_c
		static int ty_core_open(struct ty_file	*file);
		static int ty_core_close(void *data,int fd);
		static int ty_core_init(struct _ty_core	*core);
		static void ty_core_hook(void *arg);
		static int ty_core_del(void *data,int c);
		static void ty_core_heart(void *arg ,int o_timer);
	#endif
	/*===================================全局变量==========================================*/
	#ifdef _ty_core_c
		static const struct _file_fuc	ty_core_fuc=
		{	.open=ty_core_open,
			.close=ty_core_close,
			.del=ty_core_del
		};
	#endif
#endif
