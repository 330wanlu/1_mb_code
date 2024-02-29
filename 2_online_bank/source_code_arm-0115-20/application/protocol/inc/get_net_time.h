#ifndef _GET_NET_TIME_H
#define _GET_NET_TIME_H

#include "opt.h"

#include "http.h"
#include "file.h"
//#include "port_core.h"
#include "scheduled_task.h"

#define DF_TY_GET_NET_TIME                1            //网络对时

#define get_net_time_out(s,arg...)       				//log_out("all.c",s,##arg) 
		
#define DF_GET_NET_TIME_TASK_TIMER			1000                  //获取IP线程时间

struct _get_time
{
	uint8 year[4];
	uint8 month[2];
	uint8 date[2];
	uint8 wday[1];
	uint8 hour[2];
	uint8 minute[2];
	uint8 second[2];
};

struct _get_net_time_fd
{	
	int state;
	struct ty_file_mem	*mem;
	void *mem_dev;
	struct ty_file_fd	*mem_fd;
	struct _register  *dev;
};

struct _get_net_time
{   int state;                                                      //本模块状态
	int lock;                                                       //数据锁
	int task;
	struct _get_net_time_fd fd[DF_FILE_OPEN_NUMBER];
	uint8 timebuf[19];
};

int _get_net_time_add(void);
   
        
#endif
