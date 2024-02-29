#ifndef _MY_LOG_H_
#define _MY_LOG_H_
#include <stdio.h>
#include <io.h>
#include <Windows.h>
#include <direct.h>

#define PATH_LOG_WRITE  "C:\\WriteLog.log"
//#define ONLY_DEV_OPER
typedef signed char         INT8, *PINT8;
typedef signed short        INT16, *PINT16;
typedef signed int          INT32, *PINT32;
typedef signed __int64      INT64, *PINT64;
typedef unsigned char       UINT8, *PUINT8;
typedef unsigned short      UINT16, *PUINT16;
typedef unsigned int        UINT32, *PUINT32;
typedef unsigned __int64    UINT64, *PUINT64;

void WriteLogFile(const char* format, ...);
static void GetTime(UINT8 *pszTimeStr);

#endif