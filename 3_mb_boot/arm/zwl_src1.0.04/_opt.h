#ifndef _TYPE_DEFINE_DEFINE
	#define _TYPE_DEFINE_DEFINE
	/*==============================包含的公用文件========================================*/
	#include <string.h>
	#include <stdio.h>
	#include <stdlib.h>	
	#include <unistd.h>
    #include <limits.h>
	
	//#define MIPS_MTK
	#define ARM_A33
	/*================================立即数定义==========================================*/
    #define DF_TY_PATH_MAX                  256                //路径最大值
    #define DF_TY_USB_BUSID                 32                 //BUSID的值
    #define DF_TY_USB_PORT_N                100                //最大支持100个端口    
	#define DF_TRAN_OPEN

	/*===============================程序使用环境========================================*/
	//#define TAX_SPECIAL				//增值税专用
	#define GENERAL_SERIES			//通用系列

	/*===============================程序模式============================================*/
	#define OUT_KERNEL
	//#define TRAN

	/*==========================是否开启CPU内存监控=======================================*/
	//#define MONITOR_OEPN
	#define MONITOR_CLOSE
    
	/*===============================系统最小时间定义=====================================*/
	#define SYSTEM_LEAST_TIME	"2019-06-18 15:00:00"
    
	/*===============================编译选项============================================*/
	//====DEBUG开启====//
	//#define DEBUG					//alpha beta版本开启 DEBUG
	
	//====测试升级开启====//
	//#define RELEASE_TEST			//使用内网升级服务器测试

	//====正式版本或升级测试版本开启====//
	#define RELEASE					//release版本开启 RELEASE
	
	

	/*==============================各传输类型测试输出===================================*/
	//#define CONTROL_OUT
	//#define INTERRUPT_OUT
	//#define BULK_OUT
	

    /*=================================类型定义============================================*/	
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
	{	char *name;						//模块名字
		char *r_date;					//发布日期
		char *ver;						//发布版本
		char *abdr;						//当前版本类型：alpha beta debug release
		char *cpy;						//公司
		char *writer;					//编写人
		char *note;						//备注
		char *type;						//型号
		char *code;						//码制类型
	}_so_note;
	

	//typedef enum {

	//	false_mb, true_mb
	//} bool;

	//typedef enum {

	//	FALSE_MB, TRUE_MB
	//} BOOL;

	/*=================================立即数定义=========================================*/
	
	
	#define mem_free(buf)						free(buf)
	#define mem_realloc(buf,len)				realloc(buf,len)
	
	#define DF_CODE_GB18030						0
	#define DF_CODE_UTF8						1
	
	#define DF_CODE_SELF						DF_CODE_GB18030


#endif	



























