#define SYSTEM_C
#include "terminal_manage.h"
#include "manage_usbip_vbus_ui.h"
#include "system.h"
#include "regedit_operate.h"
#include <Windows.h>
#include "tlhelp32.h"
#include "log.h"
#include <crtdbg.h> 
#include <Windows.h>
#include <setupapi.h>



#pragma comment(lib, "setupapi.lib")
#define out(format,...)		log_out(format , ##__VA_ARGS__ )
typedef BOOL(__stdcall *pfunc)(DWORD, char*, char*, BOOL);


extern HFILE *fp;

void __stdcall log_out(const char* format, ...);
int __stdcall sprintf(char *buffer, const char *format , ...);


void SaveBinFile(char* file_name , unsigned char* pc_bindata,int i_binlen)
{
	FILE * fp = fopen(file_name , "wb+");
	if (fp == NULL) return;

	fwrite(pc_bindata , sizeof(uint8) , i_binlen , fp);
	fflush(fp);
	fclose(fp);
}

BOOL USBLoadedByPidVid(struct DriverInfo* info)
{
	int i = 0,count = 0;
	struct DriverInfo newinfo;

	newinfo.pid = info->pid;
	newinfo.vid = info->vid;
	newinfo.rhport = info->rhport;

	for (i = 0; i < 3; i++,Sleep(4000)){
		if (JudgeUSBInserted(&newinfo) == TRUE){
			return TRUE;
		}
	}
	return FALSE;
}
/*********************************************************************
-   Function : USBUnloadedByPidVid
-   Description：判断虚拟端口的设备是否拔出
-   Calls：
-   Calles By:
-   Table Accessed :
-   Table Update :
-   Input  :
-   Output :
-   Return :
-   Other :超时12s
***********************************************************************/
BOOL USBUnloadedByPidVid(struct DriverInfo* info)
{
	int i = 0, count = 0;
	struct DriverInfo newinfo;

	memset(&newinfo, 0, sizeof(struct DriverInfo));
	newinfo.pid = info->pid;
	newinfo.vid = info->vid;
	newinfo.rhport = info->rhport;
	
	for (i = 0; i < 3; i++, Sleep(4000)){
		if (JudgeUSBRemoved(&newinfo) == TRUE)
			return TRUE;
	}
	return FALSE;
}

extern zlib_func func_zlib;

int LoadZlibDll(zlib_func* p_zlib_func)
{
	HINSTANCE his = LoadLibrary("zlib.dll");
	p_zlib_func->compress = (pCompress)GetProcAddress(his, "compress");
	p_zlib_func->compressBound = (pCompressBound)GetProcAddress(his, "compressBound");
	p_zlib_func->uncompress = (pUncompress)GetProcAddress(his, "uncompress");
	return 0;
}

void FreeZlibLibrary()
{
	if (his != NULL){
		FreeLibrary(his);
		his = NULL;
	}
}


BOOL JudgeUSBInserted(struct DriverInfo* info)
{
	HDEVINFO hDevInfo;
	SP_DEVINFO_DATA DeviceInfoData;
	int i = 0, i_count = 0;
	struct DriverInfo myinfo;
	BOOL state = FALSE;

	hDevInfo = SetupDiGetClassDevs(NULL, 0, 0, DIGCF_PRESENT | DIGCF_ALLCLASSES);
	if (hDevInfo == INVALID_HANDLE_VALUE)
		return 0;

	DeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
	for (i = 0; SetupDiEnumDeviceInfo(hDevInfo, i, &DeviceInfoData); i++)
	{
		char szDescBuf[MAX_PATH] = { 0 };
		char szHardwareBuf[MAX_PATH] = { 0 };
		char szRhportBuf[MAX_PATH] = { 0 };
		memset(&myinfo, 0, sizeof(myinfo));

		if (!SetupDiGetDeviceRegistryProperty(hDevInfo, &DeviceInfoData, SPDRP_HARDWAREID, NULL, (PBYTE)szHardwareBuf, MAX_PATH - 1, NULL))
			continue;
		memset(szRhportBuf, 0, sizeof(szRhportBuf));
		if (!SetupDiGetDeviceRegistryProperty(hDevInfo, &DeviceInfoData, SPDRP_UI_NUMBER, NULL, (PBYTE)szRhportBuf, MAX_PATH - 1, NULL))
			continue;
		myinfo.rhport = szRhportBuf[0];

		if (GetPidVid(szHardwareBuf, &myinfo)){
			if ((myinfo.pid == info->pid) && (myinfo.vid == info->vid) && (info->rhport == myinfo.rhport)){
			//if ((myinfo.pid == info->pid) && (myinfo.vid == info->vid)){
				state = TRUE;
				break;
			}
		}
	}
	SetupDiDestroyDeviceInfoList(hDevInfo);
	return state;
}

/*********************************************************************
-   Function : JudgeUSBRemoved
-   Description：从设备列表中判断PID 、VID 、虚拟端口对应的设备是否拔出
-   Calls：
-   Calles By:
-   Table Accessed :
-   Table Update :
-   Input  :
-   Output :
-   Return :
-   Other :超时12s
***********************************************************************/
BOOL JudgeUSBRemoved(struct DriverInfo* info)
{
	HDEVINFO hDevInfo;
	SP_DEVINFO_DATA DeviceInfoData;
	int i = 0, i_count = 0;;
	struct DriverInfo myinfo;
	BOOL state = TRUE;

	hDevInfo = SetupDiGetClassDevs(NULL, 0, 0, DIGCF_PRESENT | DIGCF_ALLCLASSES);
	if (hDevInfo == INVALID_HANDLE_VALUE)
		return 0;
	DeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
	for (i = 0; SetupDiEnumDeviceInfo(hDevInfo, i, &DeviceInfoData); i++)
	{
		char szDescBuf[MAX_PATH] = { 0 };
		char szHardwareBuf[MAX_PATH] = { 0 };
		char szRhportBuf[MAX_PATH] = { 0 };
		memset(&myinfo, 0, sizeof(myinfo));

		if (!SetupDiGetDeviceRegistryProperty(hDevInfo, &DeviceInfoData, SPDRP_DEVICEDESC, NULL, (PBYTE)szDescBuf, MAX_PATH - 1, NULL))
			continue;
		if (!SetupDiGetDeviceRegistryProperty(hDevInfo, &DeviceInfoData, SPDRP_HARDWAREID, NULL, (PBYTE)szHardwareBuf, MAX_PATH - 1, NULL))
			continue;
		memset(szRhportBuf, 0, sizeof(szRhportBuf));
		if (!SetupDiGetDeviceRegistryProperty(hDevInfo, &DeviceInfoData, SPDRP_UI_NUMBER, NULL, (PBYTE)szRhportBuf, MAX_PATH - 1, NULL))
			continue;
		myinfo.rhport = szRhportBuf[0];

		if (GetPidVid(szHardwareBuf, &myinfo)){
			if ((myinfo.pid == info->pid) && (myinfo.vid == info->vid) && (info->rhport == myinfo.rhport)){
				state = TRUE;
				break;
			}
				
		}
	}
	SetupDiDestroyDeviceInfoList(hDevInfo);
	return state;
}


//获取Pid Vid在设备列表中有几个
int GetPidVidCount(struct DriverInfo* info)
{
	HDEVINFO hDevInfo;
	SP_DEVINFO_DATA DeviceInfoData;
	int i=0  , count = 0;
	struct DriverInfo myinfo;

	hDevInfo = SetupDiGetClassDevs(NULL, 0, 0, DIGCF_PRESENT | DIGCF_ALLCLASSES);
	if (hDevInfo == INVALID_HANDLE_VALUE)
		return 0;
	DeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
	for (i = 0; SetupDiEnumDeviceInfo(hDevInfo, i, &DeviceInfoData); i++)
	{
		char szClassBuf[MAX_PATH] = { 0 };
		char szDescBuf[MAX_PATH] = { 0 };
		char szHardwareBuf[MAX_PATH] = { 0 };
		char szRhportBuf[MAX_PATH] = { 0 };
		memset(&myinfo, 0, sizeof(myinfo));

		if (!SetupDiGetDeviceRegistryProperty(hDevInfo, &DeviceInfoData, SPDRP_DEVICEDESC, NULL, (PBYTE)szDescBuf, MAX_PATH - 1, NULL))
			continue;
		if ((strcmp(szDescBuf, "USB 输入设备") != 0) && (strcmp(szDescBuf, "USB 大容量存储设备") != 0))
			continue;
		if (!SetupDiGetDeviceRegistryProperty(hDevInfo, &DeviceInfoData, SPDRP_HARDWAREID, NULL, (PBYTE)szHardwareBuf, MAX_PATH - 1, NULL))
			continue;
		memset(szRhportBuf, 0, sizeof(szRhportBuf));
		if (!SetupDiGetDeviceRegistryProperty(hDevInfo, &DeviceInfoData, SPDRP_UI_NUMBER, NULL, (PBYTE)szRhportBuf, MAX_PATH - 1, NULL))
			continue;
		if (GetPidVid(szHardwareBuf, &myinfo)){
			if ((myinfo.pid == info->pid) && (myinfo.vid == info->vid))
				count++;
		}
	} 
	info->count = count;
	SetupDiDestroyDeviceInfoList(hDevInfo);
	return count;
}


/*从设备管理获取PID VID*/
BOOL GetPidVid(char* input, struct DriverInfo * info)
{
	char tmp[10];
	memset(tmp, 0, sizeof(tmp));

	char* des = strstr(input, "Vid_");
	if (des == NULL){
		des = strstr(input, "VID_");
		if (des == NULL){
			return FALSE;
		}
	}
	memcpy(tmp, des + 4, 4);
	info->vid = strtol(tmp, NULL, 16);

	des = strstr(input, "Pid_");
	if (des == NULL){
		des = strstr(input, "PID_");
		if (des == NULL){
			return FALSE;
		}
	}
	memset(tmp, 0, sizeof(tmp));
	memcpy(tmp, des + 4, 4);
	info->pid = strtol(tmp, NULL, 16);
	return TRUE;
}



void hex2str(unsigned char* in_buf , int in_size , char* out_buf)
{
	int i = 0;

	for (i = 0; i < in_size; i++){
		sprintf(out_buf + (i*5) , "0x%02x " , *(in_buf+i));
	}
}


DWORD GetProcessidFromName(LPCTSTR name)
{
	PROCESSENTRY32 pe;
	DWORD id = 0;
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	pe.dwSize = sizeof(PROCESSENTRY32);
	if (!Process32First(hSnapshot, &pe))
		return 1;
	while (1)
	{
		pe.dwSize = sizeof(PROCESSENTRY32);
		if (Process32Next(hSnapshot, &pe) == FALSE)
			break;
		if (strcmp(pe.szExeFile, name) == 0)
		{
			id = pe.th32ProcessID;
			break;
		}
	}
	CloseHandle(hSnapshot);
	if (id == 0){
		return 1;
	}
	else{
		return 0;
	}
	
}


void execute_guard_process(char* guard)
{
	char path[260];
	STARTUPINFO si;

	PROCESS_INFORMATION pi;

	ZeroMemory(&pi, sizeof(pi));

	ZeroMemory(&si, sizeof(si));
	memset(path , 0, sizeof(path));

	if (!GetModuleFileName(NULL, path, sizeof(path))){
		out("获取当前路径错误");
		return;
	}
	*(strrchr(path, '\\') + 1) = 0;
	strcat(path , guard );

	if (CreateProcess(path,//不在此指定可执行文件的文件名
		NULL,//命令行参数
		NULL,//默认进程安全性
		NULL,//默认进程安全性
		FALSE,//指定当前进程内句柄不可以被子进程继承
		CREATE_NEW_CONSOLE,//为新进程创建一个新的控制台窗口
		NULL,//使用本进程的环境变量
		NULL,//使用本进程的驱动器和目录
		&si,
		&pi)){
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
	}

	return;
}
//判断本程序只运行了一次
BOOL RunOneTime()
{
	uint8 WorkPath[MAX_PATH] = { 0 };
	int RunCount = 0;
	GetModuleFileName(NULL , WorkPath , MAX_PATH);
	uint8* exe_name = strrchr(WorkPath , '\\')+1;
	PROCESSENTRY32 pe;
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	pe.dwSize = sizeof(PROCESSENTRY32);
	if (!Process32First(hSnapshot, &pe))
		return FALSE;
	while (Process32Next(hSnapshot, &pe))
	{
		if (strcmp(pe.szExeFile, exe_name) == 0)
		{
			RunCount++;
		}
	}
	CloseHandle(hSnapshot);
	if (RunCount != 1)
		return FALSE;
	return TRUE;
}



int check_driver_install()
{
	HANDLE fd;
	if ((fd = vbus_open()) == INVALID_HANDLE_VALUE){
		return 1;
	}
	CloseHandle(fd);
	return 0;
}

HANDLE excute_exe(char *filename, char* param)
{
	SHELLEXECUTEINFO ShellInfo;
	int ret = 0;

	memset(&ShellInfo, 0, sizeof(ShellInfo));

	ShellInfo.cbSize = sizeof(ShellInfo);
	ShellInfo.hwnd = NULL;
	ShellInfo.lpVerb = _T("open");
	ShellInfo.lpFile = _T(filename);
	ShellInfo.lpParameters = _T(param);
	ShellInfo.nShow = SW_HIDE;
	ShellInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
	ret = ShellExecuteEx(&ShellInfo);
	out(filename);
	out(param);
	if (ret == FALSE){
		ret = GetLastError();
		out("usbshare.exe执行失败Windows系统错误代码=%d",ret);
		return NULL;
	}
	
	return ShellInfo.hProcess;
}



int GuidToString(const GUID *guid, unsigned char* buf)
{
	_snprintf(buf, 60, "{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}", guid->Data1, guid->Data2, guid->Data3, guid->Data4[0], guid->Data4[1],

		guid->Data4[2], guid->Data4[3],

		guid->Data4[4], guid->Data4[5],

		guid->Data4[6], guid->Data4[7]);
	return 0;
}


int produce_guid(char *buf)
{
	GUID guid;
	if (CoCreateGuid(&guid) != S_OK){
		out("guid生成失败");
		return -1;
	}

	if (GuidToString(&guid, buf) != 0){
		out("GUIDToString失败");
		return-1;
	}
	return 0;
}


int init_regedit()
{
	int result = 0, dw = 0;
	HKEY hkey;
	char path[200] = "HKEY_LOCAL_MACHINE\\SOFTWARE\\Wow6432Node\\MBInfoSofts", nape[10] = "usbshare";
	DWORD cbData = 64;
	LPBYTE guid_asc[64], set_guid_asc[64];
	char set_path[120];

	memset(set_path , 0, sizeof(set_path));
	memset(guid_asc, 0, sizeof(guid_asc));
	memset(set_guid_asc, 0, sizeof(set_guid_asc));

	if (!GetModuleFileName(NULL, set_path, sizeof(set_path))){
		out("获取当前路径错误");
		return FALSE;
	}
	*(strrchr(set_path, '\\') + 1) = 0;

	if (open_regedit(path, &hkey) == 0){//此处注册表标识符需要修改
		if (result = query_regedit(&hkey, nape, &cbData, guid_asc) != 0){
			produce_guid(guid_asc);
			if (result = write_regedit(&hkey, nape, guid_asc) != 0){
				return -1;
			}
			else{
				WritePrivateProfileString(TEXT("Set"), TEXT("GUID"), TEXT(guid_asc), TEXT(set_path));
				return 0;
			}
		}
		else{
			GetPrivateProfileString(TEXT("Set"), TEXT("GUID"), TEXT(""), set_guid_asc, 60, TEXT(set_path));
			if (guid_asc == NULL){
				return -1;
			}
			else{
				if (strcmp((const char*)guid_asc, (const char*)set_guid_asc) != 0){
					WritePrivateProfileString(TEXT("Set"), TEXT("GUID"), TEXT(guid_asc), TEXT(set_path));
					return 0;
				}
				else{
					return 0;
				}
			}
		}
	}
	else{//打开失败 说明未创建该注册表项
		if (result = create_regedit(path, &hkey, &dw) != 0){
			return result;
		}
		else{
			produce_guid(guid_asc);
			if (result = write_regedit(&hkey, nape, guid_asc) != 0){
				return result;
			}
			else{
				WritePrivateProfileString(TEXT("Set"), TEXT("GUID"), TEXT(guid_asc), TEXT(set_path));
				return 0;
			}
		}
	}
}


BOOL initSocket()
{
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0){
		return FALSE;
	}
	return TRUE;
}

uint8 algorism2asc(uint32 algorism, uint8 *asc)
{
	uint8 i, j, flag, h;
	uint32 hex = 1000000000;
	if (algorism == 0)
	{
		asc[0] = '0';
		return 1;
	}
	for (i = 0, flag = 0, h = 0; i<10; i++)
	{
		j = (unsigned char)(algorism / hex);
		if (!((j == 0) && (flag == 0)))
		{
			asc[h++] = j + '0';
			flag = 1;
			if (algorism >= hex)
				algorism = algorism - hex*j;
		}
		hex = hex / 10;
	}
	return h;
}

int ip2ip_asc(int ip, char *ip_asc)
{
	unsigned int hex;
	int i, j;
	for (i = 0, j = 0; i<sizeof(ip); i++)
	{
		hex = ((ip >> ((3 - i) * 8)) & 0xff);
		j += algorism2asc(hex, ip_asc + j);
		if (i != (sizeof(ip)-1))
			ip_asc[j++] = '.';
	}
	return j;
}


/*********************************************************************
-   Function : inet_ntop
-   Description：ip address
-   Calls：
-   Calles By:
-   Table Accessed :
-   Table Update :
-   Input  :
-   Output :
-   Return :
-   Other :
***********************************************************************/
char * inet_ntop2(int family, const void *addrptr, char *strptr, size_t len)
{
	const u_char *p = (const u_char *)addrptr;
	char temp[16];
	if (family == AF_INET) {
		
		sprintf(temp, "%d.%d.%d.%d", p[0], p[1], p[2], p[3]);
		if (strlen(temp) >= len){

			return (NULL);
		}
		strcpy(strptr, temp);
		return (strptr);
	}

	return (NULL);
}


/*********************************************************************
-   Function : domain_resolve
-   Description：域名解析
-   Calls：
-   Calles By:
-   Table Accessed :
-   Table Update :
-   Input  :
-   Output :
-   Return :
-   Other :
***********************************************************************/


int domain_resolve(unsigned char *domain, unsigned char *out_buf, unsigned int out_size)
{
	struct hostent *h;
	unsigned char ip_asc[100];

	memset(ip_asc, 0, sizeof(ip_asc));
	h = gethostbyname(domain);
	if (h == NULL){
		out("域名解析错误\n");
		return -1;
	}
	else{
		inet_ntop2(h->h_addrtype, *(h->h_addr_list), ip_asc, sizeof(ip_asc));
		if (strlen(ip_asc) > out_size){
			return -1;
		}
		else{
			sprintf(out_buf, "%s", ip_asc) ;
			return 0;
		}
	}
}






BOOL MkDir(uint8* path)
{
	uint8 d_path[260][100] = { 0 }, *tmp = NULL, dir_path[260] = {0};
	int i = 0;

	tmp = strtok(path, "\\");

	while (tmp != NULL){
		memcpy((char*)path[i++], tmp, strlen(tmp));
		tmp = strtok(NULL, "\\");
	}

	for (int j = 0; j < i; j++){
		sprintf(dir_path, "%s\\", dir_path, path[j]);
		if (access(dir_path , 0)){
			if (mkdir(dir_path)){
				return FALSE;
			}
		}
	}
	return TRUE;
}



BOOL CheckDirUsed(char* dir)
{
	DWORD dwAttr = GetFileAttributes(dir);
	if (INVALID_FILE_ATTRIBUTES == dwAttr)
	{
	return FALSE;
	}
	if (dwAttr & FILE_ATTRIBUTE_DIRECTORY)
	{
	return TRUE;
	}
	else{
	return FALSE;
	}
}



void load_library(struct write_reg_info *reg_info)
{
	DWORD processId = GetCurrentProcessId();
	int result = 0;
	HINSTANCE his = LoadLibrary("MBVerLib.dll");

	typedef BOOL(__stdcall *pfunc)(HANDLE, struct write_reg_info *reg_info);

	pfunc func = GetProcAddress(his, "WriteDevSoftDataToRegisty");
	result = func(processId, reg_info);
	if (result == 0){
		printf("失败");
	}
	else{
		printf("成功");
	}
	FreeLibrary(his);
}

//释放税盘信息统一接口的空间
void FreeTaxPlateInfoUnifySpace(struct st_TaxPlateInfoUnify* pst_UnifyInfo)
{
	if (pst_UnifyInfo->cp_TaxPlateInfoAfterUnzlib != NULL){
		free(pst_UnifyInfo->cp_TaxPlateInfoAfterUnzlib);
	}
	if (pst_UnifyInfo->cp_TaxPlateInfoBeforeUnZlib != NULL){
		free(pst_UnifyInfo->cp_TaxPlateInfoBeforeUnZlib);
	}
}

/*校验终端id*/
int check_terminal_id(char* terminal_id)
{
	if (strlen(terminal_id) != 12)
		return DF_ERR_TERMINAL_ID;
	for (int i = 0; i < 12; i++){
		if ((*(terminal_id + i) < '0') || (*(terminal_id + i) > '9'))
			return DF_ERR_TERMINAL_ID;
	}
	return 0;
}

BOOL SetPrivilege(
	HANDLE hToken,          // access token handle
	LPCTSTR lpszPrivilege,  // name of privilege to enable/disable
	BOOL bEnablePrivilege   // to enable or disable privilege
	)
{
	TOKEN_PRIVILEGES tp;
	LUID luid;

	if (!LookupPrivilegeValue(
		NULL,            // lookup privilege on local system
		lpszPrivilege,   // privilege to lookup 
		&luid))        // receives LUID of privilege
	{
		out("LookupPrivilegeValue error: %u\n", GetLastError());
		return FALSE;
	}

	tp.PrivilegeCount = 1;
	tp.Privileges[0].Luid = luid;
	if (bEnablePrivilege)
		tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	else
		tp.Privileges[0].Attributes = 0;

	// Enable the privilege or disable all privileges.

	if (!AdjustTokenPrivileges(
		hToken,
		FALSE,
		&tp,
		sizeof(TOKEN_PRIVILEGES),
		(PTOKEN_PRIVILEGES)NULL,
		(PDWORD)NULL))
	{
		out("AdjustTokenPrivileges error: %u\n", GetLastError());
		return FALSE;
	}

	if (GetLastError() == ERROR_NOT_ALL_ASSIGNED)
	{
		out("The token does not have the specified privilege. \n");
		return FALSE;
	}

	return TRUE;
}



BOOL RunAsAdministrator()
{
	HANDLE hToken;
	BOOL bRet = OpenProcessToken(
		GetCurrentProcess(),    // 进程句柄(当前进程)
		TOKEN_ALL_ACCESS,    // 全权访问令牌
		&hToken    // 返回的参数 进程令牌句柄 （就是AdjustTokenPrivileges的第一个参数）
		); // 获取进程的令牌句柄
	if (bRet != TRUE){
		out("获取令牌句柄失败!");
		return FALSE;
	}

	BOOL set = SetPrivilege(hToken, SE_DEBUG_NAME, TRUE);
	if (!set || GetLastError() != ERROR_SUCCESS){
		out("提升权限失败 error：");
		return FALSE;
	}

	return TRUE;
}



int InitRegeditSoftVer()
{
	int  result = 0;
	char set_path[120] = { 0 }, exe_name[20] = { 0 };
	DWORD processId = 0;
	pfunc func;

	HINSTANCE his = LoadLibrary("MBVerLib.dll");
	if (his == NULL)
		return 0;

	processId = GetCurrentProcessId();
	func = GetProcAddress(his, "WriteSoftVerToRegisty");

	if (!GetModuleFileName(NULL, set_path, sizeof(set_path))){
		out("获取当前路径错误");
		return FALSE;
	}
	memcpy(exe_name, strrchr(set_path, '\\') + 1, strlen(strrchr(set_path, '\\')) - 1);

	*(strrchr(set_path, '\\') + 1) = 0;
	strcat(set_path, exe_name);

	result = func(processId, exe_name, set_path, 0);
	if (result == 0){
		out("写注册表失败");
	}
	else{
		out("写注册表成功");
	}

	//写新版控制中心升级注册表信息
	*(strrchr(set_path, '\\') + 1) = 0;
	*(strrchr(set_path, '\\') + 1) = 0;
	strcat(set_path, "usbManager\\usb-share.exe");

	result = func(processId, "usb-share.exe", set_path, 0);
	if (result == 0){
		out("新版控制中心写升级注册表失败");
	}
	else{
		out("新版控制中心写升级注册表成功");
	}
	//守护进程注册表信息
	*(strrchr(set_path, '\\') + 1) = 0;
	strcat(set_path, "guard.exe");

	result = func(processId, "guard.exe", set_path, 0);
	if (result == 0){
		out("守护进程写升级注册表失败");
	}
	else{
		out("守护进程写升级注册表成功");
	}
	FreeLibrary(his);
	return 0;
}

struct Time{
	int year;
	int mouth;
	int day;
	int hour;
	int minute;
	int second;
	int million;
};



int Exchange(char* str ,int str_len , int * dest)
{
	char ca_tmp[100] = {0};
	if (!str || !dest)
		return -1;
	for (int i = 0; i < str_len; i++){
		if (str[i] < '0' || str[i] > '9')
			return -1;
	}

	memcpy(ca_tmp, str, str_len);
	int val = atoi(ca_tmp);
	if (val < 0)return -1;
	*dest = val;
	return 0;

}


/*日期格式转换
目前支持格式:
1.YYYYMMDD
2.YYYY-MM-DD
3.YYYYMMDD hh:mm:ss
4.YYYY-MM-DD hh:mm:ss
5.YYYY/MM/DD
6.YYYY/MM/DD hh:mm:ss
7.YYYY年MM月DD日
8.YYYY年MM月DD日 hh:mm:ss
*/
int DateFormat(char* pc_InDate ,char* pc_OutType , char* pc_OutData)
{
	int ia_date[7] = { 0 }, i = 0;
	char ca_tmp[20] = { 0 }, ca_CopyDate[40] = {0};
	int i_InSelect = -1, i_OutSelect = -1;;


	if (!pc_InDate || !pc_OutType || !pc_OutData || strlen(pc_InDate) >= 40)
		return -1;

	memcpy(ca_CopyDate , pc_InDate , strlen(pc_InDate));

	for (i = 0; i < strlen(ca_CopyDate); i++){
		if (ca_CopyDate[i] >= '0' &&  ca_CopyDate[i] <= '9')
			ca_CopyDate[i] = '*';
	}

	/*输入模式*/
	if (!strcasecmp(ca_CopyDate, "********"))i_InSelect = 0;
	if (!strcasecmp(ca_CopyDate, "****-**-**"))i_InSelect = 1;
	if (!strcasecmp(ca_CopyDate, "******** **:**:**"))i_InSelect = 2;
	if (!strcasecmp(ca_CopyDate, "****-**-** **:**:**"))i_InSelect = 3;
	if (!strcasecmp(ca_CopyDate, "****/**/**"))	i_InSelect = 4;
	if (!strcasecmp(ca_CopyDate, "****/**/** **:**:**"))i_InSelect = 5;
	if (!strcasecmp(ca_CopyDate, "****年**月**日"))	i_InSelect = 6;
	if (!strcasecmp(ca_CopyDate, "****年**月**日 **:**:**"))i_InSelect = 7;

	/*输出模式*/
	if (!strcasecmp(pc_OutType, "YYYYMMDD"))i_OutSelect = 0;
	if (!strcasecmp(pc_OutType, "YYYY-MM-DD"))i_OutSelect = 1;
	if (!strcasecmp(pc_OutType, "YYYYMMDD hh:mm:ss"))i_OutSelect = 2;
	if (!strcasecmp(pc_OutType, "YYYY-MM-DD hh:mm:ss"))i_OutSelect = 3;
	if (!strcasecmp(pc_OutType, "YYYY/MM/DD"))i_OutSelect = 4;
	if (!strcasecmp(pc_OutType, "YYYY/MM/DD hh:mm:ss"))i_OutSelect = 5;
	if (!strcasecmp(pc_OutType, "YYYY年MM月DD日"))i_OutSelect = 6;
	if (!strcasecmp(pc_OutType, "YYYY年MM月DD日 hh:mm:ss"))i_OutSelect = 7;


	if (i_OutSelect == -1 || i_InSelect == -1)
		return -1;

	/*日 不做精准校验*/
	switch (i_InSelect){
	case 0://yyyymmdd
		/*年*/
		/*if (Exchange(pc_InDate, 4, ia_date) < 0)
			return -1;
		if (Exchange(pc_InDate + 4, 2, ia_date + 1) < 0)
			return -1;
		if (Exchange(pc_InDate + 6, 2, ia_date + 2) < 0)
			return -1;*/



		memset(ca_tmp , 0, sizeof(ca_tmp)); memcpy(ca_tmp, pc_InDate, 4);
		ia_date[0] = atoi(ca_tmp);
		if (ia_date[0] <= 0)return -1;
		/*月*/
		memset(ca_tmp, 0, sizeof(ca_tmp)); memcpy(ca_tmp, pc_InDate + 4, 2);
		ia_date[1] = atoi(ca_tmp);
		if (ia_date[1] <= 0 || ia_date[1] >12)return -1;
		/*日*/
		memset(ca_tmp, 0, sizeof(ca_tmp)); memcpy(ca_tmp, pc_InDate + 6, 2);
		ia_date[2] = atoi(ca_tmp);
		if ( (ia_date[2] <= 0) || (ia_date[2] > 31))return -1;
		break;
	case 1://YYYY-MM-DD
		/*年*/
		memset(ca_tmp, 0, sizeof(ca_tmp)); memcpy(ca_tmp, pc_InDate, 4);
		ia_date[0] = atoi(ca_tmp);
		if (ia_date[0] <= 0)return -1;
		
		/*月*/
		memset(ca_tmp, 0, sizeof(ca_tmp)); memcpy(ca_tmp, pc_InDate + 5, 2);
		ia_date[1] = atoi(ca_tmp);
		if ( ia_date[1] <= 0 || ia_date[1] >12)return -1;
		
		/*日*/
		memset(ca_tmp, 0, sizeof(ca_tmp)); memcpy(ca_tmp, pc_InDate + 8, 2);
		ia_date[2] = atoi(ca_tmp);
		if ((ia_date[2] <= 0) || (ia_date[2] > 31))return -1;
		
		break;
	case 2://YYYYMMDD hh:mm:ss
		/*年*/
		memset(ca_tmp, 0, sizeof(ca_tmp)); memcpy(ca_tmp, pc_InDate, 4);
		ia_date[0] = atoi(ca_tmp);
		if (ia_date[0] < 0)return -1;
		/*月*/
		memset(ca_tmp, 0, sizeof(ca_tmp)); memcpy(ca_tmp, pc_InDate + 4, 2);
		ia_date[1] = atoi(ca_tmp);
		if (ia_date[1] < 0 || ia_date[1] >12)return -1;
		/*日*/
		memset(ca_tmp, 0, sizeof(ca_tmp)); memcpy(ca_tmp, pc_InDate + 6, 2);
		ia_date[2] = atoi(ca_tmp);
		if ((ia_date[2] < 0) || (ia_date[2] > 31))return -1;
		/*时*/
		memset(ca_tmp, 0, sizeof(ca_tmp)); memcpy(ca_tmp, pc_InDate + 9, 2);
		ia_date[3] = atoi(ca_tmp);
		if (ia_date[3] < 0 || ia_date[3] >24)return -1;
		/*分*/
		memset(ca_tmp, 0, sizeof(ca_tmp)); memcpy(ca_tmp, pc_InDate + 12, 2);
		ia_date[4] = atoi(ca_tmp);
		if ( ia_date[4] < 0 || ia_date[4] >60)return -1;
		/*秒*/
		memset(ca_tmp, 0, sizeof(ca_tmp)); memcpy(ca_tmp, pc_InDate + 15, 2);
		ia_date[5] = atoi(ca_tmp);
		if ((ia_date[5] < 0) || (ia_date[5] > 60))return -1;
		break;
	case 3://YYYY-MM-DD hh:mm:ss
		/*年*/
		memset(ca_tmp, 0, sizeof(ca_tmp)); memcpy(ca_tmp, pc_InDate, 4);
		ia_date[0] = atoi(ca_tmp);
		if (ia_date[0] <= 0)return -1;
		/*月*/
		memset(ca_tmp, 0, sizeof(ca_tmp)); memcpy(ca_tmp, pc_InDate + 5, 2);
		ia_date[1] = atoi(ca_tmp);
		if ( ia_date[1] <= 0 || ia_date[1] >12)return -1;
		/*日*/
		memset(ca_tmp, 0, sizeof(ca_tmp)); memcpy(ca_tmp, pc_InDate + 8, 2);
		ia_date[2] = atoi(ca_tmp);
		if ((ia_date[2] <= 0) || (ia_date[2] > 31))return -1;
		/*时*/
		memset(ca_tmp, 0, sizeof(ca_tmp)); memcpy(ca_tmp, pc_InDate + 11, 2);
		ia_date[3] = atoi(ca_tmp);
		if (ia_date[3] < 0 || ia_date[3] > 24)return -1;
		/*分*/
		memset(ca_tmp, 0, sizeof(ca_tmp)); memcpy(ca_tmp, pc_InDate + 14, 2);
		ia_date[4] = atoi(ca_tmp);
		if (ia_date[4] < 0 || ia_date[4] >60)return -1;
		/*秒*/
		memset(ca_tmp, 0, sizeof(ca_tmp)); memcpy(ca_tmp, pc_InDate + 17, 2);
		ia_date[5] = atoi(ca_tmp);
		if ((ia_date[5] < 0) || (ia_date[5] > 60))return -1;
		break;
	case 4://yyyy/mm/dd
		/*年*/
		memset(ca_tmp, 0, sizeof(ca_tmp)); memcpy(ca_tmp, pc_InDate, 4);
		ia_date[0] = atoi(ca_tmp);
		if (ia_date[0] <= 0)return -1;
		/*月*/
		memset(ca_tmp, 0, sizeof(ca_tmp)); memcpy(ca_tmp, pc_InDate + 5, 2);
		ia_date[1] = atoi(ca_tmp);
		if (ia_date[1] <= 0 || ia_date[1] >12)return -1;
		/*日*/
		memset(ca_tmp, 0, sizeof(ca_tmp)); memcpy(ca_tmp, pc_InDate + 8, 2);
		ia_date[2] = atoi(ca_tmp);
		if ((ia_date[2] <= 0) || (ia_date[2] > 31))return -1;
		break;
	case 5://yyyy/mm/dd hh:mm:ss
		/*年*/
		memset(ca_tmp, 0, sizeof(ca_tmp)); memcpy(ca_tmp, pc_InDate, 4);
		ia_date[0] = atoi(ca_tmp);
		if ( ia_date[0] <= 0)return -1;
		/*月*/
		memset(ca_tmp, 0, sizeof(ca_tmp)); memcpy(ca_tmp, pc_InDate + 5, 2);
		ia_date[1] = atoi(ca_tmp);
		if (ia_date[1] <= 0 || ia_date[1] >12)return -1;
		/*日*/
		memset(ca_tmp, 0, sizeof(ca_tmp)); memcpy(ca_tmp, pc_InDate + 8, 2);
		ia_date[2] = atoi(ca_tmp);
		if ((ia_date[2] <= 0) || (ia_date[2] > 31))return -1;
		/*时*/
		memset(ca_tmp, 0, sizeof(ca_tmp)); memcpy(ca_tmp, pc_InDate + 11, 2);
		ia_date[3] = atoi(ca_tmp);
		if (ia_date[3] < 0 || ia_date[3] > 24)return -1;
		/*分*/
		memset(ca_tmp, 0, sizeof(ca_tmp)); memcpy(ca_tmp, pc_InDate + 14, 2);
		ia_date[4] = atoi(ca_tmp);
		if (ia_date[4] < 0 || ia_date[4] >60)return -1;
		/*秒*/
		memset(ca_tmp, 0, sizeof(ca_tmp)); memcpy(ca_tmp, pc_InDate + 17, 2);
		ia_date[5] = atoi(ca_tmp);
		if ((ia_date[5] < 0) || (ia_date[5] > 60))return -1;
		break;
	case 6://yyyy年mm月dd日 
		/*年*/
		memset(ca_tmp, 0, sizeof(ca_tmp)); memcpy(ca_tmp, pc_InDate, 4);
		ia_date[0] = atoi(ca_tmp);
		if (ia_date[0] <= 0)return -1;
		/*月*/
		memset(ca_tmp, 0, sizeof(ca_tmp)); memcpy(ca_tmp, pc_InDate + 6, 2);
		ia_date[1] = atoi(ca_tmp);
		if (ia_date[1] <= 0 || ia_date[1] >12)return -1;

		/*日*/
		memset(ca_tmp, 0, sizeof(ca_tmp)); memcpy(ca_tmp, pc_InDate + 10, 2);
		ia_date[2] = atoi(ca_tmp);
		if ((ia_date[2] <= 0) || (ia_date[2] > 31))return -1;

		break;
	case 7://yyyy年mm月dd日 hh:mm:ss
		/*年*/
		memset(ca_tmp, 0, sizeof(ca_tmp)); memcpy(ca_tmp, pc_InDate, 4);
		ia_date[0] = atoi(ca_tmp);
		if (ia_date[0] <= 0)return -1;
		/*月*/
		memset(ca_tmp, 0, sizeof(ca_tmp)); memcpy(ca_tmp, pc_InDate + 6, 2);
		ia_date[1] = atoi(ca_tmp);
		if (ia_date[1] <= 0 || ia_date[1] >12)return -1;
		/*日*/
		memset(ca_tmp, 0, sizeof(ca_tmp)); memcpy(ca_tmp, pc_InDate + 10, 2);
		ia_date[2] = atoi(ca_tmp);
		if ((ia_date[2] <= 0) || (ia_date[2] > 31))return -1;
		/*时*/
		memset(ca_tmp, 0, sizeof(ca_tmp)); memcpy(ca_tmp, pc_InDate + 15, 2);
		ia_date[3] = atoi(ca_tmp);
		if (ia_date[3] < 0 || ia_date[3] > 24)return -1;
		/*分*/
		memset(ca_tmp, 0, sizeof(ca_tmp)); memcpy(ca_tmp, pc_InDate + 18, 2);
		ia_date[4] = atoi(ca_tmp);
		if (ia_date[4] < 0 || ia_date[4] > 60)return -1;
		/*秒*/
		memset(ca_tmp, 0, sizeof(ca_tmp)); memcpy(ca_tmp, pc_InDate + 21, 2);
		ia_date[5] = atoi(ca_tmp);
		if ((ia_date[5] < 0) || (ia_date[5] > 60))return -1;
		break;
	default:
		return -1;
	}


	switch (i_OutSelect){
	case 0:
		sprintf(pc_OutData, "%04d%02d%02d", ia_date[0], ia_date[1], ia_date[2]);
		break;
	case 1:
		sprintf(pc_OutData, "%04d-%02d-%02d", ia_date[0], ia_date[1], ia_date[2]);
		break;
	case 2:
		sprintf(pc_OutData, "%04d%02d%02d %02d:%02d:%02d", ia_date[0], ia_date[1], ia_date[2], ia_date[3], ia_date[4], ia_date[5]);
		break;
	case 3:
		sprintf(pc_OutData, "%04d-%02d-%02d %02d:%02d:%02d", ia_date[0], ia_date[1], ia_date[2], ia_date[3], ia_date[4], ia_date[5]);
		break;
	case 4:
		sprintf(pc_OutData, "%04d/%02d/%02d", ia_date[0], ia_date[1], ia_date[2]);
		break;
	case 5:
		sprintf(pc_OutData, "%04d/%02d/%02d %02d:%02d:%02d", ia_date[0], ia_date[1], ia_date[2], ia_date[3], ia_date[4], ia_date[5]);
		break;
	case 6:
		sprintf(pc_OutData, "%04d年%02d月%02d日", ia_date[0], ia_date[1], ia_date[2]);
		break;
	case 7:
		sprintf(pc_OutData, "%04d年%02d月%02d日 %02d:%02d:%02d", ia_date[0], ia_date[1], ia_date[2], ia_date[3], ia_date[4], ia_date[5]);
		break;
	default:
		return -1;
	}
	return 0;
}