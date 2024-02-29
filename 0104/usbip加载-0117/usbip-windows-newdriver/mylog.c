#include "mylog.h"



/**********************************************************************
* ���������� ��ȡʱ�䴮
* ��������� ��
* ��������� pszTimeStr-ʱ�䴮
* �� �� ֵ�� ��
* ����˵���� ��
* �޸�����        �汾��        �޸���        �޸�����
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
* ���������� ������д����־�ļ���
* ��������� iLogLevel-��־�ȼ�
*pszContent-ÿ����־�ľ�������
* ��������� ��
* �� �� ֵ�� ��
* ����˵���� ��
* �޸�����        �汾��        �޸���        �޸�����
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
//	fp = fopen(PATH_LOG_WRITE, "at+");      // ���ļ�, ÿ��д���ʱ���ں���׷��
//	if (fp == NULL)
//	{
//		return;
//	}
//	//// д����־ʱ��
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
//	//// д����־����
//	//// ��ԭ�����������־�ȼ���ʶ
//	//_snprintf(szLogContent, sizeof(szLogContent)-1, "[WriteLog.c]%s\n", pszContent);
//	//fputs(szLogContent, fp);
//
//	fflush(fp);     // ˢ���ļ�
//	fclose(fp);     // �ر��ļ�
//	fp = NULL;      // ���ļ�ָ����Ϊ��
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

	//���log�ļ����Ƿ����
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
		fflush(fp);     // ˢ���ļ�
		fclose(fp);     // �ر��ļ�
		fp = NULL;      // ���ļ�ָ����Ϊ��
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
	fclose(log_fp);     // �ر��ļ�
	log_fp = NULL;

	LeaveCriticalSection(&cs);

	return 0;
}