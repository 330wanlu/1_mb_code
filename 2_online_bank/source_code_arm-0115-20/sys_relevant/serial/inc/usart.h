#ifndef _USART_H
#define _USART_H

#include "opt.h"

#include "scheduled_task.h"
#include "seria.h"
// #include "usart_linux.c"

#define usart_out(s,arg...)        //log_out("all.c",s,##arg)

struct _usart
{
	int fd;
	int task;
	void (*write_data)(struct _seria *seria,uint8 *buf,int buf_len);
	struct _seria *seria;
	int aa;
};

void _linux_usart_load(int hard_type);

#endif
