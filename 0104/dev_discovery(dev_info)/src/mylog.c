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
	//strDLLPath1 = UnicodeToAnsi(file_dir);// ������ַ�������ƺ�unicode��Ҫ����ת��
	strcat(file_dir, "log\\");

	//���log�ļ����Ƿ����
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
	//fp = fopen(PATH_LOG_WRITE, "at+");      // ���ļ�, ÿ��д���ʱ���ں���׷��
	//if (fp == NULL)
	//{
	//	return;
	//}
	//// д����־ʱ��
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
		fflush(fp);     // ˢ���ļ�
		fclose(fp);     // �ر��ļ�
		fp = NULL;      // ���ļ�ָ����Ϊ��
	}
	LeaveCriticalSection(&cs);

	//// д����־����
	//// ��ԭ�����������־�ȼ���ʶ
	//_snprintf(szLogContent, sizeof(szLogContent)-1, "[WriteLog.c]%s\n", pszContent);
	////fputs(szLogContent, fp);

	//fflush(fp);     // ˢ���ļ�
	//fclose(fp);     // �ر��ļ�
	//fp = NULL;      // ���ļ�ָ����Ϊ��

	return;
}

