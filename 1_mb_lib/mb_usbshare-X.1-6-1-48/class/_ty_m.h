#ifndef _TY_M_DEFINE
	#define _TY_M_DEFINE
	#include "../file/_file.h"
	#include "../core/_seria.h"
	//#include "_ty_core.h"
	//#include "_ty_m_s_order.h"
	//#include "_ty_m_a_order.h"
	/*===============================����������=======================================*/
	#ifdef _ty_m_c
		#define out(s,arg...)															
		#define e_ty_m
	#else
		#define e_ty_m							extern 
	#endif
		
		
		/*----------------------------��������----------------------------------*/
		#define DF_TY_M_CM_PORT				100
		#define DF_TY_M_CM_ORDER			(DF_TY_M_CM_PORT+1)
		#define DF_TY_M_CM_HOOK				(DF_TY_M_CM_ORDER+1)
		#define DF_TY_M_CM_READ				(DF_TY_M_CM_HOOK+1)
		#define DF_TY_M_CM_WRITE			(DF_TY_M_CM_READ+1)
		/*------------------------��������������ֵ---------------------------*/
		#define DF_TY_MAX_PAR				8
	/*================================�ṹ�嶨��=======================================*/
struct	_ty_agree 									//��������
{
	int order;
	int ls;
	int len;
	int m_s;
	uint8 buf[DF_COM_MAX_LEN];
};

struct _ty_heart
{
	int task;
	int o_timer;
	void(*err)(void *arg);
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
		uint8 **out_buf, int in_len, int *out_len);		//���ܵ������Ĵ���	
	int(*s_pack)(int order, uint8 *buf, va_list *args);		//�������
	int(*m_a_order)(int order, uint8 *buf, int buf_len, va_list args);//����β�ʹ���
	int(*explain)(struct	_ty_agree	*data, uint8 *buf, int *tail);	//����	
	void(*self_m)(void *arg);
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
	
	#ifdef _ty_m_c
		
		
		struct _ty_m
		{	int state;	
			struct _ty_core	*core;
			int ls;
			int class_fd;
		};
	#endif
		
	/*===============================��������=========================================*/
	e_ty_m	int ty_m_add(struct _ty_core	*core);	
	#ifdef _ty_m_c
		static int ty_m_open(struct ty_file	*file);
		static int ty_m_ctl(void *data,int fd,int cm,va_list args);
		static int ty_m_close(void *data,int fd);
		static int oper_order(struct _ty_m	*ty_m,va_list *args);
		static void ty_m_hook(void *arg);
		static int open_port(struct _ty_m_re	*m_re);
		static int ty_m_del(void *data,int c);
	#endif		
	/*===============================ȫ�ֱ���==========================================*/	
	#ifdef _ty_m_c
		static const struct _file_fuc	ty_m_fuc=
		{	.open=ty_m_open,
			.ctl=ty_m_ctl,
			.close=ty_m_close,
			.del=ty_m_del
		};
	#endif		
		
#endif
