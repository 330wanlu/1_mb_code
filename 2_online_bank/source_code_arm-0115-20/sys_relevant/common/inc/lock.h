#ifndef _LOCK_H
#define _LOCK_H

#include "opt.h"

#include "linux_s.h"

#define DF_LOCK_UN							100
#define DF_LOCK_SET							(DF_LOCK_UN + 1)

#define lock_out(s,arg...)					//log_out("all.c",s,##arg)

struct _lock
{
	pthread_mutex_t data;					//Ëø
	int state;								//×´Ì¬
};

struct _lock_t
{
	pthread_mutex_t	lock;
	struct _lock data[DF_LINUX_MAX_LOCK_NUMBER];
	int data_len;
};

void _lock_load(void);
int _lock_open(char *s_name,void *par);
void _lock_close(int fd);
int _lock_set(int fd);
int _lock_set_no_dely(int fd);
int	_lock_un(int fd);

#endif

