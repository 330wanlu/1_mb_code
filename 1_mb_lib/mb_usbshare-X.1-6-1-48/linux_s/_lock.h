#ifndef _LOCK_DEFINE
	#define _LOCK_DEFINE
	#include "_linux_s.h"
	#include "../_opt.h"
	/*============================立即数定义=====================================*/
	#define DF_LOCK_UN							100
	#define DF_LOCK_SET							(DF_LOCK_UN+1)
	
	
	
	#ifdef _LOCK_C
		#define out(s,arg...)						
		#define e_lock
	#else
		#define e_lock						extern
	#endif
	/*==============================函数定义=====================================*/
	e_lock	void _lock_load(struct _lock_t	*lock);
	e_lock	int _lock_open(char *s_name,void *par);
	e_lock	void _lock_close(int fd);
	e_lock	int _lock_set(int fd);
    e_lock  int _lock_set_no_dely(int fd);
	e_lock	int	_lock_un(int fd);
	/*=============================全局变量定义==================================*/
	#ifdef _LOCK_C
		struct _lock_t			*stream;
	
	#endif
#endif

