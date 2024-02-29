#ifndef _FILE_PRO_DEFINE
    #define _FILE_PRO_DEFINE
    //#include "_linux_s.h"
    #include <dirent.h>
    #include <sys/types.h>
    #include <sys/stat.h> 
	#include "mb_typedef.h"
	#include "_code_s.h"
    /*===================================立即数定义================================================*/
        /*---------------------------------文件类型---------------------------------------*/
        #define DF_FILE_PRO_DIR                 0X10000000              //该文件类型为文件夹
        #define DF_FILE_PRO_FILE                0X01000000              //普通文件
        #define DF_FILE_PRO_FILE_CHAR           0X01100000              //字符类文件
        #define DF_FILE_PRO_FILE_BLK            0X01200000              //块设备文件
        #define DF_FILE_PRO_FILE_FIFO           0X01300000              //管道文件
        #define DF_FILE_PRO_FILE_LNK            0X01400000              //软链接(符号链接)文件
        #define DF_FILE_PRO_FILE_SOCK           0X01500000              //套接字文件
        
        /*--------------------------------程序初始定义------------------------------------*/
        #ifdef _file_pro_c
            #define e_file_pro
            #define out(s,arg...)               //log_out("all.c",s,##arg)
        #else
            #define e_file_pro
        #endif
        /*--------------------------------本程序内部定义----------------------------------*/
        
    /*==================================结构体定义=================================================*/
    

    /*===================================函数定义==================================================*/
    e_file_pro  int _file_pro_ls(const char *file_name);        //判断该文件是否存在
    e_file_pro  int _file_pro_size(const char *file_name);      //获取文件的大小
    e_file_pro  int _file_pro_type(const char *file_name);      //获取文件的类型    
	e_file_pro	int _file_exsit_size(char *file_name);			//获取文件是否存在并得知文件大小



#endif

