#ifndef _ERR_DEFINE
	#define _ERR_DEFINE
	#include "../_opt.h"
	#ifdef _err_c
		#include <sys/time.h>
		#include <time.h>
		#include "../linux_s/_lock.h"
#ifndef RELEASE_SO
		#include "_log.h"
#endif
	#endif
	/*=================================������===============================================*/
	#ifdef _err_c
		#define _err_e	
		#define out(s,arg...)					
	#else
		#define _err_e					extern
	#endif
	/*================================�ṹ�嶨��============================================*/
	typedef struct
	{	int err;
		char *name;	
	}_err;

	typedef struct 
	{	_err	*err;
		int number;	
		int utf8_gbk;
	}_err_buf;
	
	#ifdef _err_c
		struct _err_name 
		{	char name[100];
			_err_buf *buf;
			struct _err_name *next;	
		};
		struct _err_t
		{	time_t ti;
			int err;
			char name[50];
		};
		
		struct _err_l
		{	struct _err_t	buf[100];
			int tail;
		};	
	#endif	
		
	/*=====================================��������=====================================*/
	_err_e	void _err_load(void *par);
	_err_e	void _err_add(char *name,const _err_buf	*buf);
	_err_e	void _err_out(int number,char *o_name,char *note);
		
	/*============================ȫ�ֱ�������========================================*/
	#ifdef _err_c 
		struct _err_name		*err_name=NULL;
		int lock_fd;
		struct _err_l	table_err;
		static _err	linux_err[]=
		{	{	.err=DF_ERR_LINUX_HARD,
				.name="�ں˷��ش���"
			},
			{	.err=DF_ERR_MEM_ERR,
				.name="�����ڴ����"
			},
			{	.err=DF_ERR_LINUX_FD,
				.name="linux fd ���Ϸ�"
			},
			{	.err=DF_ERR_NO_INIT,
				.name="û�г�ʼ��"
			},
			{	.err=DF_ERR_NO_SO,
				.name="��.so������"
			}
		};
		static const _err_buf	linux_err_buf=
		{	linux_err,
			sizeof(linux_err)/sizeof(_err),
			0
		};
	#endif	
#endif

