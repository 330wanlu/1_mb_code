#ifndef _SERIA_DEFINE
	#define _SERIA_DEFINE
	#include "_port_core.h"	
	#include "../linux_s/_queue.h"
	#include "../file/_mem_oper.h"
	/*=============================立即数定义======================================*/
	#define DF_SERIA_CM_HOOK		100						//设置回调参数
	#define DF_SERIA_CM_BPS			(DF_SERIA_CM_HOOK+1)	//设置波特率
	#define DF_SERIA_CM_CLS			(DF_SERIA_CM_BPS+1)		//清除串口数据
	#define DF_SERIA_CM_GET			(DF_SERIA_CM_CLS+1)		//获取串口数据	
	#ifdef _seria_c
		#define e_seria
		#define out(s,arg...)					
	#else
		#define e_seria					extern
	#endif
	/*=============================结构体定义======================================*/
	struct _seria_fd
	{	int state;
		int queue;
		int head;
		void *arg;
		struct ty_file_mem	*mem;
		void *mem_dev;
		void (*hook)(void *arg);
		struct _seria	*seria;
		struct ty_file_fd	*mem_fd;
	};
	
	
	struct _seria_bsp_fuc
	{	int (*open)(struct _seria	*data,void *o_arg);
		void (*close)(void *bsp_data);
		int (*write)(void *bsp_data,uint8 *buf,int buf_len);
		int (*bps)(void *bsp_data,int bps);
		int (*del)(void *o_arg);
	};
	struct _seria 
	{	int bps;
		int tail;
		struct _seria_bsp_fuc	*fuc;
		int state;
		void (*write)(struct _seria	*data,uint8 *buf,int buf_len);
		uint8 *a_buf;
		int a_buf_len;
		struct _seria_fd	fd[DF_FILE_OPEN_NUMBER];
		void *o_arg;
		void *bsp_data;
	};
	/*================================函数定义=====================================*/
	e_seria	int seria_add(char *name,const struct _seria_bsp_fuc	*fuc,void *o_arg);
	#ifdef _seria_c
		static int seria_open(struct ty_file	*file);
		static int seria_read(void *data,int fd,uint8 *buf,int buf_len);
		static int seria_write(void *data,int fd,uint8 *buf,int buf_len);
		static int seria_ctl(void *data,int fd,int cm,va_list args);
		static int seria_close(void *data,int fd);
		static int seria_del(void *data,int c);
		static void write_hook(struct _seria	*data,uint8 *buf,int buf_len);
		//static void oper_last_hook(void *data);
		//static void oper_task(void *arg);
	#endif
	/*================================全局变量定义================================*/
	#ifdef _seria_c
		static const struct _file_fuc	seria_fuc=
		{	.open=seria_open,
			.read=seria_read,
			.write=seria_write,
			.ctl=seria_ctl,
			.close=seria_close,
			.del=seria_del
		};
	
	#endif
#endif
		