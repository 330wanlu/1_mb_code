#ifndef _FILE_H
#define _FILE_H

#include "opt.h"

#include "file_ac.h"

#define DF_FILE_GET_CM_NAME				100000

#define file_out(s,arg...)		//log_out("all.c",s,##arg)

/*===============================º¯Êý¶¨Òå========================================*/
int ty_open(char *name,int ty_fd,int c,...);
int ty_read(int fd,void *buf,int buf_len);
int ty_write(int fd,void *buf,int buf_len);
int ty_close(int fd);
int ty_ctl(int fd,int cm,...);

#endif
