#include "mylog.h"



/**********************************************************************
* 功能描述： 获取时间串
* 输入参数： 无
* 输出参数： pszTimeStr-时间串
* 返 回 值： 无
* 其它说明： 无
* 修改日期        版本号        修改人        修改内容
* -------------------------------------------------------------------
*
********************************************************************/
static void GetTime(UINT8 *pszTimeStr)
{
	SYSTEMTIME  tSysTime = { 0 };

	GetLocalTime(&tSysTime);
	sprintf(pszTimeStr, "[%04d.%02d.%02d %02d:%02d:%02d.%03d]",
		tSysTime.wYear, tSysTime.wMonth, tSysTime.wDay,
		tSysTime.wHour, tSysTime.wMinute, tSysTime.wSecond,
		tSysTime.wMilliseconds);

	return;
}


/**********************************************************************
* 功能描述： 将内容写到日志文件中
* 输入参数： iLogLevel-日志等级
*pszContent-每条日志的具体内容
* 输出参数： 无
* 返 回 值： 无
* 其它说明： 无
* 修改日期        版本号        修改人        修改内容
* -------------------------------------------------------------------
* 
********************************************************************/
//void WriteLogFile(const char* format, ...)
//{
//	FILE  *fp = NULL;
//	va_list args;
//	UINT8  szLogContent[2048] = { 0 };
//	UINT8  szTimeStr[128] = { 0 };
//
//	/*if (pszContent == NULL)
//	{
//	return;
//	}
//	*/
//
//
//	fp = fopen(PATH_LOG_WRITE, "at+");      // 打开文件, 每次写入的时候在后面追加
//	if (fp == NULL)
//	{
//		return;
//	}
//	//// 写入日志时间
//	GetTime(szTimeStr);
//	fputs(szTimeStr, fp);
//	va_start(args, format);
//	if (fp != NULL){
//		vfprintf(fp, format, args);
//		fwrite("\r\n", 1, 2, fp);
//		fflush(fp);
//	}
//	va_end(args);
//
//
//	//// 写入日志内容
//	//// 在原内容中添加日志等级标识
//	//_snprintf(szLogContent, sizeof(szLogContent)-1, "[WriteLog.c]%s\n", pszContent);
//	//fputs(szLogContent, fp);
//
//	fflush(fp);     // 刷新文件
//	fclose(fp);     // 关闭文件
//	fp = NULL;      // 将文件指针置为空
//
//	return;
//}


BOOL log_file_open(FILE  **log_fp)
{
	//DWORD dwHigh = 0;
	//DWORD dwSize;
	//__int64 nSize2;
	char file_dir[MAX_PATH] = { 0 }, time[MAX_PATH] = { 0 };
	SYSTEMTIME sys;

	GetLocalTime(&sys);

	if (GetModuleFileName(NULL,file_dir, MAX_PATH) == 0){
		return FALSE;
	}
	strcat(file_dir, "log\\");

	//检查log文件夹是否存在
	if (_access(file_dir, 0) != 0){
		_mkdir(file_dir);
	}
	sprintf(time, "usb-%4d.%02d.%02d.log", sys.wYear, sys.wMonth, sys.wDay);
	strcat(file_dir, time);
	*log_fp = fopen(file_dir, "at+");

	
	
	if(*log_fp != NULL)
	{
		/*dwSize = GetFileSize(log_fp, &dwHigh);
		nSize2 = ((__int64)dwHigh << 32) + dwSize;*/
		return TRUE;
	}
	return FALSE;
}

void WriteLogFile(const char* format, ...)
{
	FILE  *fp = NULL;
	va_list args;
	UINT8  szLogContent[2048] = { 0 };
	UINT8  szTimeStr[128] = { 0 };

	SYSTEMTIME sys = { 0 };
	CRITICAL_SECTION cs;
	InitializeCriticalSection(&cs);
	EnterCriticalSection(&cs);
	GetLocalTime(&sys);
	if (log_file_open(&fp) == FALSE)
	{
		LeaveCriticalSection(&cs);
		return;
	}

	fprintf(fp, "[%4d-%02d-%02d %d:%d:%d:%d] :", sys.wYear, sys.wMonth, sys.wDay, sys.wHour, sys.wMinute, sys.wSecond, sys.wMilliseconds);
	va_start(args, format);
	if (fp != NULL){
		vfprintf(fp, format, args);
		fflush(fp);
	}
	va_end(args);
	fwrite("\r\n", 1, 2, fp);
	if (fp)
	{
		fflush(fp);     // 刷新文件
		fclose(fp);     // 关闭文件
		fp = NULL;      // 将文件指针置为空
	}
	LeaveCriticalSection(&cs);

	return;
}

DWORD WriteDetachFile(char *name,char *msg)
{

	char file_dir[MAX_PATH] = { 0 }, filename[MAX_PATH] = { 0 };
	char file_path[MAX_PATH] = { 0 };
	FILE *log_fp =NULL;
	CRITICAL_SECTION cs;
	if(name ==NULL )
		return -1;
	InitializeCriticalSection(&cs);
	EnterCriticalSection(&cs);
	if (GetModuleFileName(NULL,file_dir, MAX_PATH) == 0){
		return FALSE;
	}
	memcpy(file_path,file_dir,strlen(file_dir)-13);
	sprintf(filename, "%s%s", file_path,name);
	WriteLogFile("write detach usb file::%s\n%s\n",filename,msg);
	log_fp = fopen(filename, "at+");
	if(log_fp == NULL)
	{
		LeaveCriticalSection(&cs);
		return -1;
	}
	if(msg != NULL)
	{
		fwrite(msg, 1, strlen(msg), log_fp);
		fwrite("\r\n", 1, 2, log_fp);
		fflush(log_fp);
	}
	fclose(log_fp);     // 关闭文件
	log_fp = NULL;

	LeaveCriticalSection(&cs);

	return 0;
}