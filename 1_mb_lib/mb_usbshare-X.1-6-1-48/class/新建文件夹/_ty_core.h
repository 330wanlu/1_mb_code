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
	
	/*===============================����������=========================================*/
	#ifdef _ty_core_c
		#define out(s,arg...)									
		#define e_ty_core
		#define DF_TY_CORE_S_NAME					"ty_s"
		#define DF_TY_CORE_M_NAME					"ty_m"
		#define DF_TY_CORE_NAME						"ty"		
	#else
		#define e_ty_core								extern
	#endif
	/*===============================�ṹ�嶨��=========================================*/
	struct _ty_core_out
	{	struct _ty_file_add_in	*file_add;			//�ļ�ע�����еĿ�
		struct _ty_file_s_add	*file;				//���ظ��Է����ļ�
		_so_note	*m_note;						//�����������ļ�
		int i_o;									//�Ƿ���ע���ļ����Է�
		int f_s;									//���Ժ��Ƿ��ȷ�
		int heart_timer;							//����ʱ��
		void (*err)(void *arg);						//���������Ļص�
		void *arg;									//�ص�����
		
	};
	
	
	//struct _ty_file_add									//�ļ�����
	//{	int pro;
	//	int (*add)(char *name,struct _ty_core_add_file	*file);
	//};
	
	struct _ty_file_add_in								
	{	struct _ty_file_add		*file_add;	
		int n;
	};
	
	
	
	
	//struct	_ty_agree 									//��������
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
	
	
	
	
	
	struct _ty_core_add_file							//�����ļ�
	{	char name[DF_FILE_NAME_MAX_LEN];
		char self_name[DF_FILE_NAME_MAX_LEN];
		int pro;
		struct _ty_core	*core;
		struct _ty_core_add_file *next;
	};

	struct _ty_file_add									//�ļ�����
	{
		int pro;
		int(*add)(char *name, struct _ty_core_add_file	*file);
	};
	
	struct _ty_core_open_file							//���ļ�����
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
	
	
	
	struct _ty_core										//���ṹ
	{	int state;
		char use_port[DF_FILE_NAME_MAX_LEN];
		struct _ty_file_add_in		*file_add;
		struct _ty_file_s_add		*file;
		_so_note					*m_note;			//���豸
		int i_o;										//�ϵ��Ƿ�����Ϣ
		int f_s;										//�����У��ȴ�����
		int queue;
		
		int (*s_a_order)(struct _ty_core	*core,int order,uint8 *in_buf,
						uint8 **out_buf,int in_len,int *out_len);		//���ܵ������Ĵ���	
		int (*s_pack)(int order,uint8 *buf,va_list *args);		//�������
		int (*m_a_order)(int order,uint8 *buf,int buf_len,va_list args);//����β�ʹ���
		int (*explain)(struct	_ty_agree	*data,uint8 *buf,int *tail);	//����	
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
	/*==================================��������==========================================*/
	e_ty_core	int ty_core_add(char *use_port,struct _ty_core_out	*app_in);
	#ifdef _ty_core_c
		static int ty_core_open(struct ty_file	*file);
		static int ty_core_close(void *data,int fd);
		static int ty_core_init(struct _ty_core	*core);
		static void ty_core_hook(void *arg);
		static int ty_core_del(void *data,int c);
		static void ty_core_heart(void *arg ,int o_timer);
	#endif
	/*===================================ȫ�ֱ���==========================================*/
	#ifdef _ty_core_c
		static const struct _file_fuc	ty_core_fuc=
		{	.open=ty_core_open,
			.close=ty_core_close,
			.del=ty_core_del
		};
	#endif
#endif
