#ifndef _FILE_OPER_DEFINE
    #define _FILE_OPER_DEFINE
    /*======================================�����ļ�=============================================*/
    #include "../_opt.h"
    #include <dirent.h>
    #include <sys/types.h>
    #include <sys/stat.h>
    #include <regex.h>
    #include <errno.h>
    #include <string.h>
    #include <stdio.h>
    #include <fcntl.h>
	//#include "../software/_code_s.h"
    /*=====================================����������===========================================*/
    #ifdef _file_oper_c
        #define out(s,arg...)                   
        #define e_file_oper
    #else
        #define e_file_oper                     extern
    #endif
    /*=====================================��������=============================================*/
    e_file_oper int file_mod(char *file);
    e_file_oper int file_oper_copy(char *scr,char *dest);
    e_file_oper int file_seek_creat_dir(char *dir,char *name);



#endif
