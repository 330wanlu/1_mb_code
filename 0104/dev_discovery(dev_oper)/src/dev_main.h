#ifndef _USBIP_MAIN_H_
#define  _USBIP_MAIN_H_
#define WINVER 0x0501
#include <Winsock2.h>
#include <Windows.h>
#include "Cjson\cJSON.h"
#include <PROCESS.H>
#include "mylog.h"
#include<time.h>
#include <iphlpapi.h>
#include <stdlib.h>
#include "../dev_discovery/new_protocol_share.h"
#include <setupapi.h>
#include <Ws2tcpip.h>
#pragma comment(lib,"Ws2_32.lib")


#define EXPORT_DLL _declspec(dllexport) 

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "IPHLPAPI.lib")

#pragma comment(lib, "setupapi.lib")


#ifdef __cplusplus
extern "C" {
#endif
	/*******************************************宏定义************************************************/
//#define ONLY_DEV_OPER

#define MALLOC(x) HeapAlloc(GetProcessHeap(), 0, (x))
#define FREE(x) HeapFree(GetProcessHeap(), 0, (x))
#define WORKING 1
#define NOT_WORKING	0
#define MAX_NUM 100
#define bool char

#define DLL_ERROR_PARAM			201
#define DLL_ERROR_MEM			DLL_ERROR_PARAM+1
#define DLL_ERROR_SYSTEM		DLL_ERROR_MEM+1
#define DLL_ERROR_CMD			DLL_ERROR_SYSTEM+1
#define DLL_ERROR_NOT_FOUND		DLL_ERROR_CMD+1
#define DLL_ERROR_GET_ADAPTER	DLL_ERROR_NOT_FOUND+1
#define DLL_ERROR_TIME_OUT	DLL_ERROR_GET_ADAPTER+1
	/*******************************************函数申明**********************************************/
	typedef  void(*result_CallBack)(char * result_str);
	int get_online_dev(char *output);
	int dev_discovery(char *output);


	int set_ip_ip(char *json_in, void *hook, char *output);
	int get_ter_all_info(char *json_in, void *hook, char *output);
	int get_ters_all_info(char *json_in, void *hook, char *output);
	int reboot_terminal(char *json_in, void *hook, char *output);
	int set_id_date(char *json_in, void *hook, char *output);
	int set_iptables(char *json_in, void *hook, char *output);
	int start_robot_arm(char *json_in, void *hook, char *output);
	int restart_single_port(char *json_in, void *hook, char *output);
	int get_mac(char *json_in, void *hook,char *output);

	//EXPORT_DLL int init_dll_interface(int time);
	EXPORT_DLL int send_cmd_deal(char * json_in, result_CallBack callback, char *output);
	//EXPORT_DLL int  send_cmd_deal(char * json_in);
	//EXPORT_DLL int clean_dll_interface();
	int GetAdapterState(DWORD index);
	/**********************************************结构体**********************************************/


	struct dev_info{
		char ip[16];//
		char ter_id[50];//申请的设备机器编号
		int usb_port_num;
		char ter_version[50];//
		char ter_type[30];//
		char protocol_ver[30];
		int cmd_port;
		int data_port;
		int state;//是否被使用标识
		char tmpinfo[800];
	};
	struct ip_info{
		char ip[16];//
		char mask[20];
		int port_old;
		int port_new;
		int sockfd_old;
		int sockfd_new;
	};
	
	struct server_ip_struct{
		char ip[64];
		int port;
	};

	struct trans_tcp_order{
		char *order;
		char *name;
		int(*answer)(char *json_in, void *hook, char *output);
	};

	/********************************************全局变量定义******************************************/
	struct dev_info dev_info_old[MAX_NUM];//旧协议缓冲
	struct dev_info dev_info_new[MAX_NUM];//新协议缓冲
	char ip_group[10][20];//保存不同网卡获取到的IP信息
	char mask_group[10][20];
	int api_nThreadFlag;
	char dev_info_back[1024 * 10];
	HANDLE main_Handle;//主线程句柄
	HANDLE main_mutex;
	HANDLE    Mutex_old;//全局互斥锁旧协议
	HANDLE    Mutex_new;//全局互斥锁新协议
	UINT8   g_szLogFile[100] = { 0 };  // 带路径的日志文件名

	HANDLE deal_thread_new[100];
	//HANDLE deal_thread_old[100];
	int init_flag;
	//int last_find_over_flag;//上次是否查询完毕标识位:0为完毕
	int already_found;//是否已经已经有查找的记录缓存
	int time_loop;
	static const  int ADAPTERNUM = 10;
	HINSTANCE hinstDLLs;
	static const struct trans_tcp_order tcp_order[] =
	{
		{ "set_ip",				"设置机柜IP",				   set_ip_ip		},
		{ "set_id_date",		"设置机器编号和生产日期",		set_id_date		},
		{ "ter_restart",		"重启机柜",						reboot_terminal },
		{ "get_ter_status",		"获取机柜全部信息",				get_ter_all_info},
		{ "setup_iptables",		"设置iptables防火墙",			set_iptables	},
		{ "start_robot_arm",	"启动机械臂",					start_robot_arm	},
		{ "restart_single_port",	"重启某个端口",				restart_single_port	},
		{ "get_ters_status",		"获取列表机柜全部信息",		get_ters_all_info},
		{ "get_mac",		"获取mac地址",		get_mac}
	};

#ifdef __cplusplus
}
#endif



#endif