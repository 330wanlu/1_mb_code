#ifndef _MEM_OPER_H 
#define _MEM_OPER_H

#include "opt.h"

#include "file.h"

#define mem_oper_out(s,arg...)			//log_out("mem_oper.c",s,##arg)

#define DF_MEM_BUF_MAX_NUMBER		512

/*==============================º¯Êý¶¨Òå====================================*/
int mem_read(void *mem_dev,void *ty,void *buf,int len);
int mem_write(void *mem_dev,void *ty,void *buf,int len);
int mem_hook(void *mem_dev,void *hook,void *arg);

#endif
