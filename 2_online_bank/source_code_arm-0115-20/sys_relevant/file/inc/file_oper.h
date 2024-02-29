#ifndef _FILE_OPER_H
#define _FILE_OPER_H

#include "opt.h"
    
#define file_oper_out(s,arg...)        //log_out("all.c",s,##arg)

int file_mod(char *file);
int file_oper_copy(char *scr,char *dest);
int file_seek_creat_dir(char *dir,char *name);

#endif
