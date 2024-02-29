#ifndef _LIB_FILE_DEFINE
    #define _LIB_FILE_DEFINE
    #include "../_opt.h"
    #include <dirent.h>
    #include <sys/types.h>
    #include <sys/stat.h>
    #include <regex.h>
    #include <errno.h>
    #include <string.h>
    #include <stdio.h>
    #include <fcntl.h>
	#include "../linux_sys/_file_oper.h"
    /*==========================================立即数定义===============================================*/
    #ifdef _lib_file_c
        #define out(s,arg...)                       //log_out("_lib_file.c",s,##arg) 
        #define e_lib_file
        #define DF_FORMATER_TAB					    "	"
        #define DF_FORMATER_ERR					    "【×】"
        #define DF_FORMATER_SUC					    "【v】"	
        #define _INFOR_ERR_CUT				        -1
		#define _INFOR_ERR_SOFTWARE			        _INFOR_ERR_CUT-1
        
    #else
        #define e_lib_file                          extern
    #endif
    /*========================================结构体定义==============================================*/
    //struct _lib_file_note
    //{   char name[100];             //程序名称
    //    int len;                    //程序长度
    //    char r_date[20];            //发布日期
    //    char ver[20];               //版本号
    //    char writer[20];            //作者
    //};
    //
    //
    //struct	_s_info
    //{	char name[100];
    //    int s_addree;			//应用程序地址
    //    int s_len;				//应用程序长度
    //    int offset;				//应用程序地址偏移量
    //    uint16 crc_val;			//校验
    //};


	struct _lib_data_file
	{	char name[100];
		char head[20];
		char version[10];
		char time[15];
		char hold[55];
		int lib_num;	
		long size;
	};

    /*=======================================函数定义===============================================*/
	e_lib_file  int lib_file_extract(const char *file);
	e_lib_file  int lib_file_get_infor(const char *file,struct	_lib_data_file *infor);  
    #ifdef _lib_file_c
		 static int get_s_infor(struct	_lib_data_file		*infor,uint8 *buf);
		 static int extract_all_lib_file(const char *file,long size,int lib_num);


    
    #endif
    
    
    

#endif
