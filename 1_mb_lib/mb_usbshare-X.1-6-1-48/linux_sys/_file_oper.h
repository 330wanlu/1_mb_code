#ifndef _FILE_OPER_DEFINE
    #define _FILE_OPER_DEFINE
    /*======================================包含文件=============================================*/
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
    /*=====================================立即数定义===========================================*/
    #ifdef _file_oper_c
        #define out(s,arg...)                   
        #define e_file_oper
    #else
        #define e_file_oper                     extern
    #endif
    /*=====================================函数定义=============================================*/
    e_file_oper int file_mod(char *file);
    e_file_oper int file_oper_copy(char *scr,char *dest);
    e_file_oper int file_seek_creat_dir(char *dir,char *name);



#endif
