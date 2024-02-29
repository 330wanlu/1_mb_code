#include "log.h"

#include <time.h>
#include <stdarg.h>
#include <Windows.h>

#include <stdlib.h>
#include <stdio.h>
#include <io.h>

int log_type = 0;
FILE  *log_fp = NULL;
CRITICAL_SECTION cs;

int log_file_open1();
/*********************************************************************
-   Function : cmp_log_date
-   Description：保证日志文件只有两天
-   Calls：
-   Calles By:
-   Table Accessed :
-   Table Update :
-   Input  :
-   Output :
-   Return :
-   Other :
***********************************************************************/
int cmp_log_date(char* file_name)
{
	char *postion = file_name;
	char year[10] = { 0 }, mouth[10] = { 0 }, day[10] = {0};
	int i_year = 0 , i_mouth = 0 , i_day = 0,times = 0;
	SYSTEMTIME sys = {0};

	GetLocalTime(&sys);

	while ((postion = strchr(postion, '.'))){
		if (times == 0){
			memcpy(mouth, postion + 1, 2);
			i_mouth = atoi(mouth);
			postion++;
			times++;
		}else if (times == 1){
			memcpy(day, postion + 1, 2);
			i_day = atoi(day);
			postion++;
			times++;
		}else{
			postion++;
			times++;
		}
	}

	if (i_mouth == sys.wMonth){//同一个月份;
		if (sys.wDay - i_day > 1){
			return 1;
		}
		else{
			return 0;
		}
	}else if (i_mouth != sys.wMonth){
		if (abs(i_day - sys.wDay) > 2){
			return 1;
		}
		else{
			return 0;
		}
	}

	return 0;
}

//1.获取日志目录下的所有文件;
//2.获取当前时间;
//3.比较时间 超过两天的删除;
//4.
int log_init()
{
	char module_address[MAX_PATH] = { 0 }, log_dir[MAX_PATH] = {0};
	char file_dir[MAX_PATH] = {0};
	long  handle;//用于查找的句柄;
	struct _finddata_t fileinfo;//文件信息的结构体;

	InitializeCriticalSection(&cs);

	if (!GetModuleFileName(NULL, module_address, MAX_PATH)){
		return -1;
	}
	*(strrchr(module_address, '\\') + 1) = '\0';
	
	memcpy(log_dir , module_address , strlen(module_address));
	strcat(log_dir, "log\\");
	if (_access(log_dir, 0)){
		if (mkdir(log_dir)){
			return FALSE;
		}
	}
	strcat(log_dir, "*.txt");//*.txt

	memcpy(file_dir, log_dir , strlen(log_dir));
	*(strrchr(file_dir, '\\') + 1) = '\0';

	handle = _findfirst(log_dir , &fileinfo);
	if (handle == -1) 
		return 0;

	do{
		if (cmp_log_date(fileinfo.name)){
			strcat(file_dir , fileinfo.name);
			remove(file_dir);
			*(strrchr(file_dir, '\\') + 1) = '\0';
		}
	} while (_findnext(handle, &fileinfo) != -1);

	_findclose(handle);
	return 0;
}

BOOL log_file_open1()
{
	char file_dir[MAX_PATH] = { 0 }, time[MAX_PATH] = {0};
	SYSTEMTIME sys;

	GetLocalTime(&sys);

	if (GetModuleFileName(NULL, file_dir, MAX_PATH) == 0){
		return FALSE;
	}
	*(strrchr(file_dir, '\\') + 1) = '\0';
	strcat(file_dir, "log\\");

	//检查log文件夹是否存在
	if (_access(file_dir, 0) != 0){
		mkdir(file_dir);
	}
	sprintf(time, "%4d.%2d.%2d.txt", sys.wYear, sys.wMonth, sys.wDay);
	strcat(file_dir, time);
	log_fp = fopen(file_dir, "at+");
	if (log_fp != NULL)
		return TRUE;
	return FALSE;
}
/*********************************************************************
-   Function : log_out
-   Description：日志输出 
-   Calls：
-   Calles By:
-   Table Accessed :
-   Table Update :
-   Input  :
-   Output :
-   Return :
-   Other :
***********************************************************************/
void log_out(const char* format, ...)
{
	va_list args;
	SYSTEMTIME sys = {0};
	
	EnterCriticalSection(&cs);
	GetLocalTime(&sys);
	if (log_file_open1() == FALSE){
		LeaveCriticalSection(&cs);
		return;
	}
	fprintf(log_fp, "%4d-%2d-%2d %d:%d:%d:%d ", sys.wYear, sys.wMonth, sys.wDay, sys.wHour, sys.wMinute, sys.wSecond, sys.wMilliseconds);
	va_start(args, format);
	if (log_fp != NULL){
		vfprintf(log_fp, format, args);
		fflush(log_fp);
	}
	va_end(args);
	fwrite("\r\n", 1, 2, log_fp);
	if (log_fp)
		fclose(log_fp);
	LeaveCriticalSection(&cs);	
}