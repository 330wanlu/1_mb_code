#ifndef _FOLDER_DEFINE
    #define _FOLDER_DEFINE
    //#include "linux_s.h"
    #include "_file_pro.h"
    #include <dirent.h>
    #include <sys/types.h>
    #include <sys/stat.h>
    /*===================================����������===============================================*/
        /*--------------------------------�����ʼ����---------------------------------------------*/
        #ifdef _folder_c
            #define e_folder
            #define out(s,arg...)                                
        #else
            #define e_folder
        #endif
        /*--------------------------------�������ڲ�����-------------------------------------------*/
    /*======================================��������==============================================*/
    e_folder int _folder_ls_seek(const char *path,const char *name,char r_file[][256],
        int size_r);
    e_folder    int _folder_new(const char *path,const char *new_folder);


#endif
