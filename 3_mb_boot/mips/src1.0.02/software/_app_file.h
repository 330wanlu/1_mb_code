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
	#include "_code_s.h"
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
	struct _s_info_new
	{
		char name[20];//����������
		char type[10];//���������û���
		char app_ver[20];//������汾��
		unsigned char app_md5[20];//������MD5 ֵ  ��Ч16λ
		int app_len;//�����򳤶�
		unsigned char *app_content;//����������
	};
    /*=======================================��������===============================================*/
    e_app_file  int app_file_juge_new(const char *file,char *ver,char *file_ver);
				
	e_app_file  int app_file_get_new(const char *file, struct _app_file_note *file_note);
	e_app_file  int app_file_get_new_detect(const char *file, struct _app_file_note *file_note);
	e_app_file	int app_file_get_infor_new(const char *file, struct _s_info_new *infor);
    e_app_file  int app_file_oper(const char *file);
	e_app_file	int app_file_detect(const char *file, char *outerr);
    #ifdef _app_file_c
	static int get_s_info_new(struct	_s_info_new		*infor, unsigned char *buf);
		static int app_file_write_real_app(char *file,unsigned char *writebuf, int len);
        static int get_soft_file(const char *file,char *r_file);
    #endif
    
    
    

#endif
