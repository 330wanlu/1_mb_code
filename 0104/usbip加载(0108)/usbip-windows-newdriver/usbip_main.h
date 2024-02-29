#ifndef _USBIP_MAIN_H_
#define  _USBIP_MAIN_H_
#define WINVER 0x0501
#include <Winsock2.h>
#include <Windows.h>
#include "Cjson\cJSON.h"
#include <PROCESS.H>
#include "usbip.h"
#include "mylog.h"
#include <winsock2.h>
#include <iphlpapi.h>

#include "manage_set_file.h"

#include <setupapi.h>
#define EXPORT_DLL _declspec(dllexport) 

//extern  EXPORT_DLL int add(int a, int b); // 即 int add(int a,int b)

#pragma comment(lib, "setupapi.lib")




#ifdef __cplusplus
extern "C" {
#endif
	/*******************************************宏定义************************************************/
	#define DLL_ERROR_PARAM			201
	#define DLL_ERROR_MEM			DLL_ERROR_PARAM+1
	#define DLL_ERROR_SYSTEM		DLL_ERROR_MEM+1
	#define DLL_ERROR_CMD			DLL_ERROR_SYSTEM+1
	#define DLL_ERROR_NOT_FOUND		DLL_ERROR_CMD+1
	#define DLL_ERROR_NO_AUTHORITY  DLL_ERROR_NOT_FOUND+1
	#define DLL_ERROR_DEVICE_USED   DLL_ERROR_NO_AUTHORITY+1


	#define DLL_ERROR_DRIVER		220

	
	/*******************************************函数申明**********************************************/
	typedef  void(__cdecl *result_CallBack)(char * result_str);
	int get_online_dev(char *output);
	EXPORT_DLL int init_dll_interface();
	EXPORT_DLL int send_cmd_deal(char * Json_In, result_CallBack callback,char *output);
	//EXPORT_DLL int send_cmd_deal(char * Json_In);

	EXPORT_DLL int clean_dll_interface();

	/**********************************************结构体**********************************************/

	struct transmit_line_info{

		int server_port;//链接服务端口号
		char server_ip[16];//链接服务IP
		int dev_port;//申请的设备端口号
		char ter_id[13];//申请的设备机器编号
		char user[64];//用户名
		char bus_id[32];//申请的设备busid
		//char client_id[50];//标识转发链路唯一标识符
		int trans_type;//是否公网转发：0:本地  1:转发
		int use_flag;//当前是否被使用：0：未 1：已
		int thread_flag;//当前是否线程使用：0：未 1：已
		int point;//标识当前位置索引
		int sockfd;//链接TCP句柄
		char oper_id[64];
		result_CallBack *r_Call;//回调函数指针（未用到)
	};

	struct _usbip_errinfo{
		int ErrCode;
		char *ErrInfo;
	};
	/********************************************全局变量定义******************************************/
	int api_nThreadFlag = 1;//主线程运行标志
	int dll_inint_flag;
	result_CallBack r_Call[20];//回调函数数据
	struct transmit_line_info line_info[20];//链路信息数据
	HANDLE main_Handle;//主线程句柄
	HANDLE    Mutex;//全局互斥锁
	UINT8   g_szLogFile[100] = {0};  // 带路径的日志文件名


	static const struct _usbip_errinfo usbip_err_n[] =
	{
		{ USBIP_ERR_CONNECT_SERVER, "服务器连接失败，请重试" },
		{ USBIP_ERR_PORT_ERR, "端口设备有误，未检测到设备!" },
		{ USBIP_ERR_NOT_AUTH, "端口未授权" },
		{ USBIP_ERR_PORT_BUSY, "端口正在被使用" },
		{USBIP_ERROR_GET_DEVICE,"端口设备有误，设备异常"},
		{ USBIP_ERR_IMPORT_PORT,"导入虚拟usb设备失败,驱动未安装"},
		//内网报错
		{ USBIP_ERR_INNER_PORT_AUTH,"端口未授权!"},
		{USBIP_ERR_INNER_PORT_ERR,"端口有误未检测到设备"},
		{USBIP_ERR_INNER_PORT_ERR1,"此端口是机械臂，不支持加载"},

		//链路关闭
		{USBIP_ERR_PORT_DETTACH,"加载端口已释放"},
		


	};

#ifdef __cplusplus
}
#endif



#endif