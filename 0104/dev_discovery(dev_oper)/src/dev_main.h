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
	/*******************************************�궨��************************************************/
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
	/*******************************************��������**********************************************/
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
	/**********************************************�ṹ��**********************************************/


	struct dev_info{
		char ip[16];//
		char ter_id[50];//������豸�������
		int usb_port_num;
		char ter_version[50];//
		char ter_type[30];//
		char protocol_ver[30];
		int cmd_port;
		int data_port;
		int state;//�Ƿ�ʹ�ñ�ʶ
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

	/********************************************ȫ�ֱ�������******************************************/
	struct dev_info dev_info_old[MAX_NUM];//��Э�黺��
	struct dev_info dev_info_new[MAX_NUM];//��Э�黺��
	char ip_group[10][20];//���治ͬ������ȡ����IP��Ϣ
	char mask_group[10][20];
	int api_nThreadFlag;
	char dev_info_back[1024 * 10];
	HANDLE main_Handle;//���߳̾��
	HANDLE main_mutex;
	HANDLE    Mutex_old;//ȫ�ֻ�������Э��
	HANDLE    Mutex_new;//ȫ�ֻ�������Э��
	UINT8   g_szLogFile[100] = { 0 };  // ��·������־�ļ���

	HANDLE deal_thread_new[100];
	//HANDLE deal_thread_old[100];
	int init_flag;
	//int last_find_over_flag;//�ϴ��Ƿ��ѯ��ϱ�ʶλ:0Ϊ���
	int already_found;//�Ƿ��Ѿ��Ѿ��в��ҵļ�¼����
	int time_loop;
	static const  int ADAPTERNUM = 10;
	HINSTANCE hinstDLLs;
	static const struct trans_tcp_order tcp_order[] =
	{
		{ "set_ip",				"���û���IP",				   set_ip_ip		},
		{ "set_id_date",		"���û�����ź���������",		set_id_date		},
		{ "ter_restart",		"��������",						reboot_terminal },
		{ "get_ter_status",		"��ȡ����ȫ����Ϣ",				get_ter_all_info},
		{ "setup_iptables",		"����iptables����ǽ",			set_iptables	},
		{ "start_robot_arm",	"������е��",					start_robot_arm	},
		{ "restart_single_port",	"����ĳ���˿�",				restart_single_port	},
		{ "get_ters_status",		"��ȡ�б����ȫ����Ϣ",		get_ters_all_info},
		{ "get_mac",		"��ȡmac��ַ",		get_mac}
	};

#ifdef __cplusplus
}
#endif



#endif