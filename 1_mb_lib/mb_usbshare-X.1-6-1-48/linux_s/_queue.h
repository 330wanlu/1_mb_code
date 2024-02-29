#ifndef _QUEUE_DEFINE
	#define _QUEUE_DEFINE
	#include "_linux_s.h"
	#include "_lock.h"
	#include "_task.h"
	#include "../_opt.h"
	/*================================立即数定义=======================================*/
	#ifdef _queue_c
		#define out(s,arg...)				
		#define e_queue
	#else
		#define e_queue						extern
	#endif
	
	/*=================================函数定义========================================*/
	e_queue	void _queue_load(struct _queue_t	*queue_t);
	e_queue	int _queue_add(char *name,void (*hook)(void *arg),void *arg,int timer);
	#ifdef _queue_c
		static void * queue_hook(void *arg);
	#endif
	/*==============================全局变量定义======================================*/
	#ifdef _queue_c
		struct _queue_t		*q_data;
	#endif
#endif

