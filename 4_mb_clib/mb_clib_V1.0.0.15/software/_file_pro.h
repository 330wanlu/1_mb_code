#ifndef _FILE_PRO_DEFINE
    #define _FILE_PRO_DEFINE
    //#include "_linux_s.h"
    #include <dirent.h>
    #include <sys/types.h>
    #include <sys/stat.h> 
	#include "mb_typedef.h"
	#include "_code_s.h"
    /*===================================����������================================================*/
        /*---------------------------------�ļ�����---------------------------------------*/
        #define DF_FILE_PRO_DIR                 0X10000000              //���ļ�����Ϊ�ļ���
        #define DF_FILE_PRO_FILE                0X01000000              //��ͨ�ļ�
        #define DF_FILE_PRO_FILE_CHAR           0X01100000              //�ַ����ļ�
        #define DF_FILE_PRO_FILE_BLK            0X01200000              //���豸�ļ�
        #define DF_FILE_PRO_FILE_FIFO           0X01300000              //�ܵ��ļ�
        #define DF_FILE_PRO_FILE_LNK            0X01400000              //������(��������)�ļ�
        #define DF_FILE_PRO_FILE_SOCK           0X01500000              //�׽����ļ�
        
        /*--------------------------------�����ʼ����------------------------------------*/
        #ifdef _file_pro_c
            #define e_file_pro
            #define out(s,arg...)               //log_out("all.c",s,##arg)
        #else
            #define e_file_pro
        #endif
        /*--------------------------------�������ڲ�����----------------------------------*/
        
    /*==================================�ṹ�嶨��=================================================*/
    

    /*===================================��������==================================================*/
    e_file_pro  int _file_pro_ls(const char *file_name);        //�жϸ��ļ��Ƿ����
    e_file_pro  int _file_pro_size(const char *file_name);      //��ȡ�ļ��Ĵ�С
    e_file_pro  int _file_pro_type(const char *file_name);      //��ȡ�ļ�������    
	e_file_pro	int _file_exsit_size(char *file_name);			//��ȡ�ļ��Ƿ���ڲ���֪�ļ���С



#endif

