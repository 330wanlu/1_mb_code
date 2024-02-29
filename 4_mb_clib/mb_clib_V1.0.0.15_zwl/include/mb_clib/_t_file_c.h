#ifndef _T_FILE_C_DEFINE
    #define _T_FILE_C_DEFINE
    #include "mb_typedef.h"
	#include "_file_pro.h"
	#include <sys/types.h>
	#include <unistd.h>
	#include <fcntl.h>
	#include "_code_s.h"
    /*=================================立即数定义========================================*/
    #ifdef _t_file_c_c
        #define out(s,arg...)           //log_out("_t_file_c.c",s,##arg)   
        #define e_t_file_c
    #else
        #define e_t_file_c              extern
    #endif
    /*=================================结构体定义========================================*/
    
    
    /*==================================函数定义=========================================*/
    e_t_file_c  int _t_file_c_read(const char *file_name,char *buf,int buf_size);
    e_t_file_c  int _t_file_c_write(const char *file_name,char *buf,int buf_size);
	e_t_file_c	int _t_file_c_write_no_file(const char *file_name, char *buf, int buf_size);
	e_t_file_c	int _t_file_c_write_b(const char *file_name,char *buf,int buf_size);

	e_t_file_c int read_file(const char *file_name, char *buf, int buf_size);
	e_t_file_c int write_file(const char *file_name, char *buf, int buf_size);
	e_t_file_c void chmod_file(char *file_name, char *level);
	e_t_file_c void delete_file(char *file_name);
	e_t_file_c void delete_dir(char *dir_name);
	e_t_file_c void copy_file(char *src_file_name, char *dest_file_name);
	e_t_file_c void copy_file_cmp(char *src_file_name, char *dest_file_name);
	e_t_file_c void copy_dir(char *src_dir_name, char *dest_dir_name);
	e_t_file_c int file_exists(char *file_name);
#endif

