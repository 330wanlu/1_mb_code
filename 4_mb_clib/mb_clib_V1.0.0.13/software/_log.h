#ifndef _LOG_DEFINE
	#define _LOG_DEFINE
	/*=============================包含文件=====================================*/
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
	/*=============================常数定义====================================*/
	#define DF_LOG_DIR_STDOUT								0		//输出至控制台
	#define DF_LOG_DIR_ANDROID								1		//输出至安卓日志
	#define DF_LOG_DIR_D									0XFF	//禁止输出
	#define DF_LOG_NAME_E									0		//文件输出允许
	#define DF_LOG_NMAE_D									1		//文件输出不允许
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
	/*==============================结构体定义========================================*/
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
	/*===============================函数定义========================================*/


	e_log	void log_out(char *name,const char* format, ...);	
	e_log	int write_log(const char *format, ...);
	e_log   int logout(unsigned char bLoglevl, char *appcode, char *event, const char *format, ...);
	//e_log   int _WriteLog(const char *format, ...);
	e_log	void printf_array(char *data, int len);
	/*===============================全局变量定义====================================*/
	#ifdef _log_c 
		static _log			log;
	#endif
#endif	