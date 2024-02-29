#include "mylog.h"


EXTERN_C IMAGE_DOS_HEADER __ImageBase;


char* UnicodeToAnsi(const wchar_t* szStr)
{
	int nLen = WideCharToMultiByte(CP_ACP, 0, szStr, -1, NULL, 0, NULL, NULL);
	if (nLen == 0)
	{
		return NULL;
	}
	char* pResult = malloc(nLen);
	WideCharToMultiByte(CP_ACP, 0, szStr, -1, pResult, nLen, NULL, NULL);
	return pResult;
}


BOOL log_file_open(FILE  **log_fp)
{
	DWORD dwHigh = 0;
	DWORD dwSize;
	__int64 nSize2;

	char file_dir[MAX_PATH] = { 0 }, time[MAX_PATH] = { 0 };
	SYSTEMTIME sys;
	//char *strDLLPath1;
	GetLocalTime(&sys);
	/*GetModuleFileName((HINSTANCE)&__ImageBase, strDLLPath1, _MAX_PATH);
	printf("file_dir1:%s\n", strDLLPath1);*/
	if (GetModuleFileName(NULL,file_dir, MAX_PATH) == 0){
		return FALSE;
	}
	//strDLLPath1 = UnicodeToAnsi(file_dir);// 程序的字符集如果似乎unicode则要进行转换
	strcat(file_dir, "log\\");

	//检查log文件夹是否存在
	if (_access(file_dir, 0) != 0){
		_mkdir(file_dir);
	}
	sprintf(time, "%4d.%02d.%02d.log", sys.wYear, sys.wMonth, sys.wDay);
	strcat(file_dir, time);
RE_OPEN:
	*log_fp = fopen(file_dir, "at+");
	if (*log_fp != NULL)
	{
		fseek(*log_fp, 0, SEEK_END);
		long int size = ftell(*log_fp);
		if (size > (1024 * 1024 * 10))
		{
			fclose(*log_fp);
			DeleteFile(file_dir);
			goto RE_OPEN;
		}
		
		return TRUE;
	}
		
	return FALSE;
}


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
void WriteLogFile(const char* format, ...)
{
#ifdef ONLY_DEV_OPER
	return 0;
#endif
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
	//fp = fopen(PATH_LOG_WRITE, "at+");      // 打开文件, 每次写入的时候在后面追加
	//if (fp == NULL)
	//{
	//	return;
	//}
	//// 写入日志时间
	//GetTime(szTimeStr);
	//fputs(szTimeStr, fp);
	//va_start(args, format);
	//if (fp != NULL){
	//	vfprintf(fp, format, args);
	//	fwrite("\r\n", 1, 2, fp);
	//	fflush(fp);
	//}
	//va_end(args);
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

	//// 写入日志内容
	//// 在原内容中添加日志等级标识
	//_snprintf(szLogContent, sizeof(szLogContent)-1, "[WriteLog.c]%s\n", pszContent);
	////fputs(szLogContent, fp);

	//fflush(fp);     // 刷新文件
	//fclose(fp);     // 关闭文件
	//fp = NULL;      // 将文件指针置为空

	return;
}

