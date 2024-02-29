#ifndef _APP_FILE_DEFINE
    #define _APP_FILE_DEFINE
    #include "../_opt.h"
    #include <dirent.h>
    #include <sys/types.h>
    #include <sys/stat.h>
    #include <regex.h>
    #include <errno.h>
    #include <string.h>
    #include <stdio.h>
    #include <fcntl.h>
	#include <mb_clib/_algorithm.h>
	#include <mb_clib/_code_s.h>
	#include "../linux_sys/_file_oper.h"
#include <openssl/md5.h>
    /*==========================================����������===============================================*/
    #ifdef _app_file_c
        #define out(s,arg...)                       //log_out("_app_file.c",s,##arg) 
        #define e_app_file
        #define DF_FORMATER_TAB					    "	"
        #define DF_FORMATER_ERR					    "������"
        #define DF_FORMATER_SUC					    "��v��"	
        #define _INFOR_ERR_CUT				        -1
		#define _INFOR_ERR_SOFTWARE			        _INFOR_ERR_CUT-1
        
    #else
        #define e_app_file                          extern
    #endif
    /*========================================�ṹ�嶨��==============================================*/
    struct _app_file_note
    {   char name[100];             //��������
        int len;                    //���򳤶�
        char r_date[20];            //��������
        char ver[20];               //�汾��
        char writer[20];            //����
    };
    
    
    struct	_s_info
    {	char name[100];
        int s_addree;			//Ӧ�ó����ַ
        int s_len;				//Ӧ�ó��򳤶�
        int offset;				//Ӧ�ó����ַƫ����
        uint16 crc_val;			//У��
    };

    /*=======================================��������===============================================*/
    e_app_file  int app_file_juge_new(const char *file,char *ver,char *file_ver);
    e_app_file  int app_file_get(const char *file,struct _app_file_note *file_note);
    e_app_file  int app_file_get_infor(const char *file,struct	_s_info *infor);  
    e_app_file  int app_file_oper(const char *file);
	e_app_file  int app_file_juge_down_now_ser(const char *file,char *ver,char *ser_ver);
    #ifdef _app_file_c
        static int cmp_soft_new(const char *soft_file,uint8 *soft_buf,int size_buf);
        static int get_s_infor(struct	_s_info		*infor,uint8 *buf);
        static uint8 *get_soft_ware(char *file,struct _s_info     *infor);
        static int get_soft_file(const char *file,char *r_file);
        static int cmp_soft_new(const char *soft_file,uint8 *soft_buf,int size_buf);
        static int get_soft_infor(const char *soft_file,struct _app_file_note *file_note);
    
    #endif
    
    
    

#endif
