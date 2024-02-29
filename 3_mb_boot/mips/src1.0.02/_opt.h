#ifndef _TYPE_DEFINE_DEFINE
	#define _TYPE_DEFINE_DEFINE
	/*==============================�����Ĺ����ļ�========================================*/
	#include <string.h>
	#include <stdio.h>
	#include <stdlib.h>	
	#include <unistd.h>
    #include <limits.h>
	
	//#define MIPS_MTK
	#define ARM_A33
	/*================================����������==========================================*/
    #define DF_TY_PATH_MAX                  256                //·�����ֵ
    #define DF_TY_USB_BUSID                 32                 //BUSID��ֵ
    #define DF_TY_USB_PORT_N                100                //���֧��100���˿�    
	#define DF_TRAN_OPEN

	/*===============================����ʹ�û���========================================*/
	//#define TAX_SPECIAL				//��ֵ˰ר��
	#define GENERAL_SERIES			//ͨ��ϵ��

	/*===============================����ģʽ============================================*/
	#define OUT_KERNEL
	//#define TRAN

	/*==========================�Ƿ���CPU�ڴ���=======================================*/
	//#define MONITOR_OEPN
	#define MONITOR_CLOSE
    
	/*===============================ϵͳ��Сʱ�䶨��=====================================*/
	#define SYSTEM_LEAST_TIME	"2019-06-18 15:00:00"
    
	/*===============================����ѡ��============================================*/
	//====DEBUG����====//
	//#define DEBUG					//alpha beta�汾���� DEBUG
	
	//====������������====//
	//#define RELEASE_TEST			//ʹ��������������������

	//====��ʽ�汾���������԰汾����====//
	#define RELEASE					//release�汾���� RELEASE
	
	

	/*==============================���������Ͳ������===================================*/
	//#define CONTROL_OUT
	//#define INTERRUPT_OUT
	//#define BULK_OUT
	

    /*=================================���Ͷ���============================================*/	
	typedef unsigned char  										uint8;      
	typedef signed   char  										int8;        
	typedef unsigned short 										uint16;      
	typedef signed   short 										int16;        	
	typedef unsigned long   									uint32; 
	typedef unsigned long long   								uint64;
	typedef signed   long   									int32;       	
	typedef float          										fp32;        
	typedef double         										fp64; 
	typedef unsigned int 										uint;	
	
	typedef struct
	{	char *name;						//ģ������
		char *r_date;					//��������
		char *ver;						//�����汾
		char *abdr;						//��ǰ�汾���ͣ�alpha beta debug release
		char *cpy;						//��˾
		char *writer;					//��д��
		char *note;						//��ע
		char *type;						//�ͺ�
		char *code;						//��������
	}_so_note;
	

	//typedef enum {

	//	false_mb, true_mb
	//} bool;

	//typedef enum {

	//	FALSE_MB, TRUE_MB
	//} BOOL;

	/*=================================����������=========================================*/
	
	
	#define mem_free(buf)						free(buf)
	#define mem_realloc(buf,len)				realloc(buf,len)
	
	#define DF_CODE_GB18030						0
	#define DF_CODE_UTF8						1
	
	#define DF_CODE_SELF						DF_CODE_GB18030


#endif	



























