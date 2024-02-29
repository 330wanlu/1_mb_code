#ifndef _LOG_DEFINE
	#define _LOG_DEFINE
	/*=============================�����ļ�=====================================*/
	#ifdef _log_c
		#include <string.h>
		#include <stdio.h>
		#include <stdlib.h>	
		#include <unistd.h>
		#include <stdarg.h>
		//#include "../linux_s/_lock.h"
		#include "_timer.h"
		//#include "_err.h"
		#include "_code_s.h"
		#include "_utf82gbk.h"
	#endif	
	/*=============================��������====================================*/
	#define DF_LOG_DIR_STDOUT								0		//���������̨
	#define DF_LOG_DIR_ANDROID								1		//�������׿��־
	#define DF_LOG_DIR_D									0XFF	//��ֹ���
	#define DF_LOG_NAME_E									0		//�ļ��������
	#define DF_LOG_NMAE_D									1		//�ļ����������
#ifdef TAX_SPECIAL	
	#define DF_TERMINAL_LOG_FILE			"/etc/terminal_tax.log"
#else
	#define DF_TERMINAL_LOG_FILE			"/etc/terminal.log"
#endif
	#ifdef _log_c
		#define e_log		
			
	#else
		#define e_log						extern
	#endif
	/*==============================�ṹ�嶨��========================================*/
	#ifdef _log_c
		struct name_pro
		{	char name[50];
			int dir;
		};
		typedef struct
		{	int code;
			int lock;
			int dir;
			struct name_pro	file[100];	
		}_log;
	#endif	
	/*===============================��������========================================*/


	e_log	void log_out(char *name,const char* format, ...);	
	e_log	int write_log(const char *format, ...);
	e_log   int logout(unsigned char bLoglevl, char *appcode, char *event, const char *format, ...);
	//e_log   int _WriteLog(const char *format, ...);
	e_log	void printf_array(char *data, int len);
	/*===============================ȫ�ֱ�������====================================*/
	#ifdef _log_c 
		static _log			log;
	#endif
#endif	