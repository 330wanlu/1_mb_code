#ifndef _SERIA_H
#define _SERIA_H

#include "opt.h"

#include "delay_task.h"
#include "mem_oper.h"

#define UART_BOUDS 9600

#define DF_SERIA_CM_HOOK		100						//设置回调参数
#define DF_SERIA_CM_BPS			(DF_SERIA_CM_HOOK+1)	//设置波特率
#define DF_SERIA_CM_CLS			(DF_SERIA_CM_BPS+1)		//清除串口数据
#define DF_SERIA_CM_GET			(DF_SERIA_CM_CLS+1)		//获取串口数据	

#define serial_out(s,arg...)		//log_out("_lcd_state.c",s,##arg)			
	
struct _seria_fd
{
	int state;
	int queue;
	int head;
	void *arg;
	struct ty_file_mem *mem;
	void *mem_dev;
	void (*hook)(void *arg);
	struct _seria *seria;
	struct ty_file_fd *mem_fd;
};
	
struct _seria_bsp_fuc
{
	int (*open)(struct _seria *data,void *o_arg);
	void (*close)(void *bsp_data);
	int (*write)(void *bsp_data,uint8 *buf,int buf_len);
	int (*bps)(void *bsp_data,int bps);
	int (*del)(void *o_arg);
};

struct _seria 
{
	int bps;
	int tail;
	struct _seria_bsp_fuc *fuc;
	int state;
	void (*write)(struct _seria	*data,uint8 *buf,int buf_len);
	uint8 *a_buf;
	int a_buf_len;
	struct _seria_fd fd[DF_FILE_OPEN_NUMBER];
	void *o_arg;
	void *bsp_data;
};

int seria_add(char *name,const struct _seria_bsp_fuc *fuc,void *o_arg);

#endif
		