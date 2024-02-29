#ifndef _FILE_H
	#define _FILE_H
	#include "_file_ac.h"
	/*==============================立即数定义=====================================*/
	#define DF_FILE_GET_CM_NAME				100000
	
	#ifdef _file_c
		#define out(s,arg...)				
		#define e_file						
	#else
		#define e_file						extern
	#endif
	
	/*===============================函数定义========================================*/
	e_file int ty_open(char *name,int ty_fd,int c,...);
	e_file int ty_read(int fd,void *buf,int buf_len);
	e_file int ty_write(int fd,void *buf,int buf_len);
	e_file int ty_close(int fd);
	e_file int ty_ctl(int fd,int cm,...);
#endif