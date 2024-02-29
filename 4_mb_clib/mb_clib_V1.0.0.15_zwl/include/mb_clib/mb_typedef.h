#ifndef _MB_OPT_TYPE_DEFINE
#define _MB_OPT_TYPE_DEFINE
	/*==============================包含的公用文件========================================*/
	#include <string.h>
	#include <stdio.h>
	#include <stdlib.h>	
	#include <unistd.h>
    #include <limits.h>

    /*=================================类型定义============================================*/	
	typedef unsigned char  										uint8;      
	typedef signed   char  										int8;        
	typedef unsigned short 										uint16;      
	typedef signed   short 										int16;        	
	typedef unsigned int   										uint32; 
	typedef unsigned long long   								uint64;
	typedef signed   int   										int32;       	
	typedef float          										fp32;        
	typedef double         										fp64; 
	typedef unsigned int 										uint;	
#define MB_CLIB_VERSION	"1.0.0.15"
		enum eLogLevel {
		OFF,
		DEBUG,
		INFO,
		WARN,
		ERROR,
		FATAL
	};

#endif	



























