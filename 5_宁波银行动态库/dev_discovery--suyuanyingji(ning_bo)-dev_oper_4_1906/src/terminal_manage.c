#define DF_TERMINAL_MANAGE_C
#include <io.h>
#include "terminal_manage.h"
//#include "usbshare_handle.h"
//#include "log.h"
#include "err.h"
//#include "system.h"
//#include "guid.h"
#include "manage_set_file.h"
//#include "InvoiceDetail.h"
//#include "release_mem.h"

DWORD WINAPI cabinet_thread1(LPARAM lparam);

enum CheckState{ UNCHECK = 0, CHECKING, CHECKED };

//#define WriteLogFile(format,...)		log_WriteLogFile(format , ##__VA_ARGS__)

struct CabinetInfo* SearchFreeCabinet();


/*静态函数*/
static int find_cabinet_by_usbport(struct Cabinets * cabinet_data, int usbport);
static int old_find_cabinet_by_usbport(struct Cabinets * cabinet_data, int usbport);
static BOOL InsertToUbBlockHeadTable(struct UnBlockRetrieveGetInvoiceNode *node);

/**********************老版本协议***********************************/
int old_updata_terminal_setting(struct file_setting file_setting[], int size_file)
{
	int count = size_file, i = 0, j = 0;
	int ret = 0;

	WaitForSingleObject(stream.hmutex, INFINITE);

	//判断是否已经存在
	int num = sizeof(stream.attribute) / sizeof(stream.attribute[0]);
	for (i = 0; i < size_file; i++){//原本内存中是否有相同的ip配置 （!= 0） (== 1)
		for (j = 0; j < num; j++){
			if ((strcmp(file_setting[i].terminal_ip, stream.attribute[j].ca_SettingIP) == 0)){
				stream.attribute[j].used = 1;
			}
		}
	}

	//新输入的数据中相同的ip配置
	for (i = 0; i < sizeof(stream.attribute) / sizeof(stream.attribute[0]); i++){
		for (j = 0; j < size_file; j++){

			if ((strcmp(file_setting[j].terminal_ip, stream.attribute[i].ca_SettingIP) == 0)){
				memset(&file_setting[j], 0, sizeof(struct file_setting));
			}
		}
	}

	for (j = 0; j < sizeof(stream.attribute) / sizeof(stream.attribute[0]); j++){
		if (stream.attribute[j].used == 0){
			memset(&stream.attribute[j], 0, sizeof(struct CabinetInfo));
		}
	}

	stream.count = size_file;
	//增加的配置写进内存表中
	for (j = 0; j < size_file; j++){
		if (*file_setting[j].terminal_ip != 0){
			for (i = 0; i < 128; i++){
				if (*stream.attribute[i].ca_SettingIP == 0){
					stream.attribute[i].dataport = file_setting[j].data_port;
					stream.attribute[i].port = file_setting[j].terminal_port;
					memcpy(stream.attribute[i].ca_SettingIP, file_setting[j].terminal_ip, strlen(file_setting[j].terminal_ip));
					break;
				}
			}
		}
	}

	//清除标记位
	for (i = 0; i < 128; i++){
		if (stream.attribute[i].used == 1)
			stream.attribute[i].used = 0;
	}

	ReleaseMutex(stream.hmutex);
	return i;
}

int old_query_cabinet_attribute(struct CabinetInfo cabinet[], int num_cabinets)
{
	int i = 0;
	for (i = 0; i < 128; i++){
		if (*stream.attribute[i].ca_SettingIP != 0)
			memcpy(&cabinet[i], &stream.attribute[i], sizeof(stream.attribute[0]));
	}

	return stream.count;
}


int old_query_cabinet_num_by_usbport(int usbport, struct CabinetInfo *cabinet)
{
	int ret = 0;
	ret = find_cabinet_by_usbport(&stream, usbport);
	if (ret < 0){
		WriteLogFile("[find_cabinet_by_usbport] ret = %d\n",ret);
		return ret;
	}
	if (cabinet != NULL)
		memcpy(cabinet, &stream.attribute[ret], sizeof(stream.attribute[0]));
	return 0;
}

int old_terminal_open_port(int usbport, char *busid, int *p_usb_handle)
{
	int ret = 0;
	char guid[64];

	memset(guid , 0, sizeof(guid));
	ret = old_find_cabinet_by_usbport(&stream, usbport);
	if (ret < 0){
		return ret;
	}
	//guid_read(guid);
	return ProtocolOpenUsbport(guid, stream.attribute[ret].ca_SettingIP, stream.attribute[ret].port, usbport, p_usb_handle, busid);
}


int old_terminal_close_port(int usbport)
{
	//int ret = 0;
	//struct Manage_usbshare_port usbip = {0};

	//ret = old_find_cabinet_by_usbport(&stream, usbport);
	//if (ret < 0){
	//	return ret;
	//}

	//if (GetUSBIPInfo_LastVer(usbport, &usbip) != TRUE){
	//	return DF_ERR_NO_RIGHT_CLOSE_PORT;
	//}

	//return ProtocolCloseUsbport(stream.client_id, stream.attribute[ret].ca_SettingIP, stream.attribute[ret].port, usbport, usbip.usb_handle);
	return 0;
}



int old_terminal_close_port_force(int usbport)
{
	int ret = 0;
	ret = find_cabinet_by_usbport(&stream, usbport);
	if (ret < 0){
		return ret;
	}
	return ProtocolCloseUsbportForce(stream.client_id, stream.attribute[ret].ca_SettingIP, stream.attribute[ret].port, usbport);
}



int old_terminal_query_usbport_state(int usbport, struct Port_State *port_state)
{
	int ret = 0;
	ret = find_cabinet_by_usbport(&stream, usbport);
	if (ret < 0){
		return ret;
	}
	return  ProtocolGetUsbportState(stream.client_id, stream.attribute[ret].ca_SettingIP, stream.attribute[ret].port, usbport, port_state);
}




static int old_find_cabinet_by_usbport(struct Cabinets * cabinet_data, int usbport)
{
	int i = 0;
	for (i = 0; i < 128; i++){
		if (cabinet_data->attribute[i].terminal_system_state == 0){
			WriteLogFile("cabinet_data->attribute[%d].state= %d", i, cabinet_data->attribute[i].terminal_system_state);
			continue;
		}
		if (usbport >= cabinet_data->attribute[i].i_USBOriginNum && (usbport < cabinet_data->attribute[i].i_USBOriginNum + cabinet_data->attribute[i].i_USBSumNumber)){
			return i;
		}
	}

	return -1;
}


/*************************************************新版本协议**********************************************************/
static int i_CheckingNum;
/*********************************************************************
-   Function : terminal_init
-   Description：终端管理初始化
-   Calls：
-   Calles By:
-   Table Accessed :
-   Table Update :
-   Input  :
-   WriteLogFileput :
-   Return :
-   Other :
***********************************************************************/
BOOL terminal_init(void)
{
	struct file_setting file[128] = {0};
	int i;
	i_CheckingNum = 0;
	memset(&stream, 0, sizeof(struct Cabinets));
	

	//guid_read(stream.client_id);
	stream.hmutex = CreateMutex(NULL, FALSE, NULL);
	if (stream.hmutex == NULL){
		WriteLogFile("终端表互斥量初始化失败");
		return FALSE;
	}
	stream.singal = CreateSemaphore(NULL , 1 , 1,  NULL);
	if (stream.singal == NULL){
		WriteLogFile("终端表信号量初始化失败");
		return FALSE;
	}
	stream.count = read_setting(file, sizeof(file) / sizeof(file[0]));
	//WriteLogFile("lbc stream.count = %d",stream.count);
	for (i = 0; i < stream.count; i++){
		memcpy(stream.attribute[i].ca_SettingIP, file[i].terminal_ip, sizeof(stream.attribute[0].ca_SettingIP));
		stream.attribute[i].port = file[i].terminal_port;
		stream.attribute[i].dataport = file[i].data_port;
	}
	//lbc test
	for (i = 0; i < 10; i++){
		stream.process_id[i] = CreateThread(0, 0, cabinet_thread1, &stream, 0, 0);
		if (stream.process_id[i] == NULL){
			WriteLogFile("管理机柜线程创建失败 ");
			return FALSE;
		}
		//WriteLogFile("管理机柜线程创建成功 %d",i);
	}
	
	return TRUE;
}


int TerminalGetTerminalCAInfo(char* terminal_id, struct CabinetInfo *cabinet)
{
	int  ret = 0;
	ret = GetDeivceInfoByID(cabinet, terminal_id);
	if (ret < 0){
		WriteLogFile("没有该设备");
		return ret;
	}

	return ProtocolGetTerminalAllCATaxpayerName(cabinet->ca_SettingIP, cabinet->port, cabinet);
}


int TerminalGetTerminalUSBListInfo(char* terminal_id, struct CabinetInfo *cabinet)
{
	int  ret = 0;
	ret = GetDeivceInfoByID(cabinet, terminal_id);
	if (ret < 0){
		WriteLogFile("没有该设备");
		return ret;
	}

	return ProtocolGetTerminalAllUSBInfoList(cabinet->ca_SettingIP, cabinet->port, cabinet);
}


int TerminalGetHeartBeatVersion(char* terminal_id, struct TerminalProtocol0226_Result * stp_HeartBeat)
{
	int ret = 0;
	struct CabinetInfo cabinet = { 0 };
	ret = GetDeivceInfoByID(&cabinet, terminal_id);
	if (ret < 0){
		WriteLogFile("终端ID %s该设备没有配置进中间件", terminal_id);
		return ret;
	}

	return  ProtocolHeartBeat(cabinet.ca_SettingIP, cabinet.port, stp_HeartBeat);
}


/*********************************************************************
-   Function : terminal_open_port
-   Description：打开机柜端口
-   Calls：
-   Calles By:
-   Table Accessed :
-   Table Update :
-   Input  :
-   WriteLogFileput :
-   Return :
-   Other :
***********************************************************************/
int terminal_open_port(char* terminal_id, int usbport, char *busid, int *usb_handle)
{
	int ret = 0;
	char client_id[40] = {0};
	struct CabinetInfo cabinet = { 0 };

	ret = GetDeivceInfoByID(&cabinet, terminal_id);
	if (ret < 0){
		WriteLogFile("没有该设备");
		return ret;
	}
	get_client_id(client_id);
	return ProtocolOpenUsbport(client_id, cabinet.ca_SettingIP, cabinet.port, usbport, usb_handle, busid);
}


/*********************************************************************
-   Function : terminal_close_port
-   Description：关闭终端端口
-   Calls：
-   Calles By:
-   Table Accessed :
-   Table Update :
-   Input  :
-   WriteLogFileput :
-   Return :
-   Other :
***********************************************************************/
int terminal_close_port(char* terminal_id,int usbport)
{
	/*int ret = 0;
	struct cabinetinfo cabinet = {0};
	struct manage_usbshare_port usbip = {0};

	if (getusbipinfo(terminal_id, usbport, &usbip) != true){
		return df_err_no_right_close_port;
	}
	ret = getdeivceinfobyid(&cabinet, terminal_id);
	if (ret < 0){
		WriteLogFile("没有该设备");
		return ret;
	}
	WriteLogFile("关闭终端id=%s ip = %s 端口号= %d bus_handle = %d", terminal_id, cabinet.ca_settingip, usbport, usbip.usb_handle);

	return protocolCloseUsbport(stream.client_id, cabinet.ca_SettingIP, cabinet.port, usbport, usbip.usb_handle);*/

	return 0;
}


/*********************************************************************
-   Function : terminal_close_port_force
-   Description：强制关闭终端端口
-   Calls：
-   Calles By:
-   Table Accessed :
-   Table Update :
-   Input  :
-   WriteLogFileput :
-   Return :
-   Other :
***********************************************************************/
int terminal_close_port_force(char* terminal_id,int usbport)
{
	int ret = 0;
	struct CabinetInfo cabinet = {0};
	ret = GetDeivceInfoByID(&cabinet, terminal_id);
	if (ret < 0){
		WriteLogFile("没有该设备");
		return ret;
	}
	return ProtocolCloseUsbportForce(stream.client_id, cabinet.ca_SettingIP, cabinet.port, usbport);
}


/*********************************************************************
-   Function : terminal_query_cabinet_num_by_usbport
-   Description：根据usb端口号查找机柜
-   Calls：
-   Calles By:
-   Table Accessed :
-   Table Update :
-   Input  :
-   WriteLogFileput :
-   Return :
-   Other :
***********************************************************************/
int terminal_query_cabinet_attribute(struct CabinetInfo* cabinet, int num_cabinets)
{
	int i = 0;
	//WriteLogFile("terminal_query_cabinet_attribute ca_TerminalID=%s",stream.attribute[0].ca_TerminalID);
	for (i = 0; i < 128; i++){
		memcpy(&cabinet[i], &stream.attribute[i], sizeof(struct CabinetInfo));
	}
	return stream.count;
}

/*********************************************************************
-   Function : terminal_query_cabinet_network_param
-   Description：查询终端网络参数
-   Calls：
-   Calles By:
-   Table Accessed :
-   Table Update :
-   Input  :
-   WriteLogFileput :
-   Return :
-   Other :
***********************************************************************/
int terminal_query_cabinet_network_param(struct Cabinet_Network_Param *network_param, char * terminal_id)
{
	int ret = 0;
	struct CabinetInfo cabinet = {0};
	ret = GetDeivceInfoByID(&cabinet, terminal_id);
	if (ret < 0){
		WriteLogFile("没有该设备");
		return ret;
	}
	return protocol_query_IP_param(network_param, cabinet.ca_SettingIP, cabinet.port);
}


/*********************************************************************
-   Function : terminal_query_cabinet_network_param
-   Description：查询终端FTP
-   Calls：
-   Calles By:
-   Table Accessed :
-   Table Update :
-   Input  :
-   WriteLogFileput :
-   Return :
-   Other :
***********************************************************************/
int terminal_query_cabinet_FTP_param(struct Cabinet_Network_Param *network_param, char * terminal_id)
{
	int ret = 0;
	struct CabinetInfo cabinet = {0};
	ret = GetDeivceInfoByID(&cabinet, terminal_id);
	if (ret < 0){
		WriteLogFile("没有该设备");
		return ret;
	}
	return protocol_query_FTP_param(network_param, cabinet.ca_SettingIP, cabinet.port);
}



/*********************************************************************
-   Function : terminal_upgrade_program
-   Description：更新终端程序
-   Calls：
-   Calles By:
-   Table Accessed :
-   Table Update :
-   Input  :
-   WriteLogFileput :
-   Return :
-   Other :
***********************************************************************/
int terminal_upgrade_program(char * terminal_id)
{
	int ret = 0;
	struct CabinetInfo cabinet = {0};
	ret = GetDeivceInfoByID(&cabinet, terminal_id);
	if (ret < 0){
		WriteLogFile("没有该设备");
		return ret;
	}
	return ProtocolUpgradeProgram(cabinet.ca_SettingIP, cabinet.port);
}

/*********************************************************************
-   Function : terminal_updata_terminal_setting
-   Description：更新终端配置
-   Calls：
-   Calles By:
-   Table Accessed :
-   Table Update :
-   Input  :
-   WriteLogFileput :
-   Return :
-   Other :
***********************************************************************/
int terminal_updata_terminal_setting(struct file_setting* file_setting, int size_file)
{
	int count = size_file , i = 0 ,j = 0,num;
	WaitForSingleObject(stream.hmutex , INFINITE);
	
#if 0
	for (i = 0; i < 128; i++){
		memset(&stream.attribute[i], 0, sizeof(stream.attribute[i]));

	}
#endif
	stream.count = size_file;
	num = sizeof(stream.attribute) / sizeof(stream.attribute[0]);
	for (i = 0; i < size_file; i++){//原本内存中是否有相同的ip配置 （!= 0） (== 1)
		for (j = 0; j < num; j++){
			if ((strcmp(file_setting[i].terminal_ip, stream.attribute[j].ca_SettingIP) == 0) && (file_setting[i].data_port == stream.attribute[j].dataport) && (file_setting[i].terminal_port == stream.attribute[j].port)){
				stream.attribute[j].used = 1;
			}
		}
	}
	
	for (i = 0; i < sizeof(stream.attribute) / sizeof(stream.attribute[0]); i++){//新输入的数据中相同的ip配置
		for (j = 0; j < size_file; j++){

			if ((strcmp(file_setting[j].terminal_ip, stream.attribute[i].ca_SettingIP) == 0) && (file_setting[j].data_port == stream.attribute[i].dataport) && (file_setting[j].terminal_port == stream.attribute[i].port)){
				memset(&file_setting[j], 0, sizeof(struct file_setting));
			}
		}
	}


	for (j = 0; j < sizeof(stream.attribute) / sizeof(stream.attribute[0]); j++){
		if ( stream.attribute[j].used == 0){
			memset(&stream.attribute[j], 0, sizeof(struct CabinetInfo));
		}
	}

	for (j = 0; j < size_file; j++){
		if (*file_setting[j].terminal_ip != 0){
			for (i = 0; i < 128; i++){
				if (*stream.attribute[i].ca_SettingIP == 0){
					stream.attribute[i].dataport = file_setting[j].data_port;
					stream.attribute[i].port = file_setting[j].terminal_port;
					memcpy(stream.attribute[i].ca_SettingIP, file_setting[j].terminal_ip, strlen(file_setting[j].terminal_ip));
					break;
				}
			}
		}
	}


	for (i = 0; i < 128; i++){
		if (stream.attribute[i].used == 1)
			stream.attribute[i].used = 0;
	}
#if 0
	for (i = 0; i < count; i++){
		memcpy(stream.attribute[i].ip, file_setting[i].terminal_ip, strlen(file_setting[i].terminal_ip));
		stream.attribute[i].port = file_setting[i].terminal_port;
	}
#endif
	ReleaseMutex(stream.hmutex);
	return i;
}

/*********************************************************************
-   Function : terminal_restart
-   Description：更新终端配置
-   Calls：
-   Calles By:
-   Table Accessed :
-   Table Update :
-   Input  :
-   WriteLogFileput :
-   Return :
-   Other :
***********************************************************************/
int terminal_restart_terminal(char* terminal_id)
{
	int ret = 0;

	struct CabinetInfo cabinet = { 0 };
	ret = GetDeivceInfoByID(&cabinet, terminal_id);
	if (ret < 0){
		WriteLogFile("没有该设备");
		return ret;
	}
	return ProtocolRestartCabinet(terminal_id, cabinet.ca_SettingIP, cabinet.port);
}



/*********************************************************************
-   Function : terminal_query_cabinet_num_by_usbport
-   Description：根据usb端口号查找机柜
-   Calls：
-   Calles By:
-   Table Accessed :
-   Table Update :
-   Input  :
-   WriteLogFileput :
-   Return :
-   Other :
***********************************************************************/
int terminal_query_cabinet_num_by_usbport(int usbport, struct CabinetInfo *cabinet)
{
	int ret = 0;
	ret = find_cabinet_by_usbport(&stream, usbport);
	if (ret < 0){
		WriteLogFile("不存在该设备");
		return ret;
	}
	if (cabinet!=NULL)
		memcpy(cabinet, &stream.attribute[ret], sizeof(stream.attribute[0]));
	return ret;
}

/*********************************************************************
-   Function : GetDeivceInfoByID
-   Description：根据终端ID获取设备所有的信息
-   Calls：
-   Calles By:
-   Table Accessed :
-   Table Update :
-   Input  :
-   WriteLogFileput :
-   Return :如果错误返回错误码 如果正确返回0
-   Other :
***********************************************************************/
int GetDeivceInfoByID(struct CabinetInfo *stp_cabinet, char *terminal_id)
{
	int i = 0;

	if (stp_cabinet == NULL || terminal_id == NULL)
		return DF_SYSTEM_ERROR;

	for (i = 0; i <128; i++){
		if (strcmp(stream.attribute[i].ca_TerminalID, terminal_id) == 0){
			if (stream.attribute[i].terminal_system_state == 0){
				WriteLogFile("函数：【GetDeivceInfoByID】终端ID:%s 不在线", terminal_id);
				return DF_ERR_TERMINAL_OFFLINE;
			}
			else{
				if (stp_cabinet != NULL){
					memcpy(stp_cabinet, &stream.attribute[i], sizeof(struct CabinetInfo));
					break;
				}
				else{
					WriteLogFile("函数：【GetDeivceInfoByID】参数：【stp_cabinet】 为空");
					return DF_SYSTEM_ERROR;
				}
			}
		}
	}

	return 0;
}


int TerminalSetCabinetNetwork(char* terminal_id, struct Cabinet_Network_Param *network_param)
{
	struct CabinetInfo st_cabinetinfo = { 0 };
	int ret = 0;

	if ((ret = GetDeivceInfoByID(&st_cabinetinfo, terminal_id)) != 0){
		return ret;
	}

	ret = ProtocolSetNetwork(network_param, st_cabinetinfo.ca_SettingIP, st_cabinetinfo.port);
	if (ret != 0){
		WriteLogFile("IP参数设置失败");
		return ret;
	}

	ret = ProtocolSetFTP(network_param, st_cabinetinfo.ca_SettingIP, st_cabinetinfo.port);
	if (ret != 0){
		WriteLogFile("FTP参数设置失败");
		return ret;
	}

	return 0;
}

/*********************************************************************
-   Function : query_usbport_state
-   Description：查询端口号属性
-   Calls：
-   Calles By:
-   Table Accessed :
-   Table Update :
-   Input  :
-   WriteLogFileput :
-   Return :
-   Other :
***********************************************************************/
int terminal_query_terminal_soft_version(char* terminal_id, struct CabinetInfo *pst_cabinetinfo)
{
	int ret = 0;

	ret = GetDeivceInfoByID(pst_cabinetinfo , terminal_id);
	if (ret < 0){
		WriteLogFile("没有该设备");
		return ret;
	}
	return  protocol_query_terminal_soft_version(pst_cabinetinfo->ca_SettingIP, pst_cabinetinfo->port, pst_cabinetinfo);
}



/*********************************************************************
-   Function : query_usbport_state
-   Description：查询端口号属性
-   Calls：
-   Calles By:
-   Table Accessed :
-   Table Update :
-   Input  :
-   WriteLogFileput :
-   Return :
-   Other :
***********************************************************************/
int terminal_query_usbport_state(char* terminal_id, int usbport, struct Port_State *port_state)
{
	int ret = 0;
	struct CabinetInfo cabinet = {0};
	ret = GetDeivceInfoByID(&cabinet, terminal_id);
	if (ret < 0){
		WriteLogFile("没有该设备");
		return ret;
	}
	return  ProtocolGetCabinetUsbportDetail(cabinet.ca_SettingIP, cabinet.port, usbport, port_state);
}
/*********************************************************************
-   Function : terminal_udp_search_terminal
-   Description：udp广播获取终端信息
-   Calls：
-   Calles By:
-   Table Accessed :
-   Table Update :
-   Input  :
-   WriteLogFileput :
-   Return :
-   Other :
***********************************************************************/
int terminal_udp_search_terminal(struct Cabinets *data)
{
	return  udp_send(data);
}


/*********************************************************************
-   Function : terminal_get_usbport_state_by_id
-   Description：根据id查询端口号信息
-   Calls：
-   Calles By:
-   Table Accessed :
-   Table Update :
-   Input  :
-   WriteLogFileput :
-   Return :
-   Other :
***********************************************************************/
int terminal_get_usbport_state_by_id(char* terminal_id)
{
	int ret = 0;
	struct CabinetInfo st_cabinet = { 0 };
	ret = GetDeivceInfoByID(&st_cabinet, terminal_id);
	if (ret != 0){
		WriteLogFile("没有该设备");
		return ret;
	}
	WriteLogFile("protocol_get_cabinet_all_usbport_state 44444");
	return protocol_get_cabinet_all_usbport_state(st_cabinet.ca_SettingIP, st_cabinet.port, st_cabinet.port_state);
}


/*********************************************************************
-   Function : query_usbport_state
-   Description：配置设备vpn
-   Calls：
-   Calles By:
-   Table Accessed :
-   Table Update :
-   Input  :
-   WriteLogFileput :
-   Return :
-   Other :
***********************************************************************/
int terminal_set_vpn(char* terminal_id, ST_ClientVPNSetting *vpn)
{
	int ret = 0,postion = 0;
	unsigned char in_buf[1000] = {0};
	struct CabinetInfo cabinet = {0};


	in_buf[postion++] = (char)vpn->open;
	in_buf[postion++] = (char)vpn->login_type;
	memcpy(in_buf + postion, vpn->VPNServerIP, 50); postion += 50;
	memcpy(in_buf + postion, vpn->VPNServerPort, 2); postion += 2;
	memcpy(in_buf + postion, vpn->account, 20); postion += 20;
	memcpy(in_buf + postion, vpn->passwd, 20); postion += 20;
	memcpy(in_buf + postion, vpn->FTPPort, 2); postion += 2;
	memcpy(in_buf + postion, vpn->FTPUsername, 20); postion += 20;
	memcpy(in_buf + postion, vpn->FTPPasswd, 20); postion += 20;

	ret = GetDeivceInfoByID(&cabinet, terminal_id);
	if (ret < 0){
		WriteLogFile("没有该设备");
		return ret;
	}
	return  ProtocolSetVPN(cabinet.ca_SettingIP, cabinet.port, in_buf, postion);
}

/*********************************************************************
-   Function : query_usbport_state
-   Description：设置终端设备自动升级
-   Calls：
-   Calles By:
-   Table Accessed :
-   Table Update :
-   Input  :
-   WriteLogFileput :
-   Return :
-   Other :
***********************************************************************/
int terminal_set_autoupgrade(char* terminal_id, char auto_upgrade)
{
	int ret = 0;
	unsigned char in_buf[100];
	struct CabinetInfo cabinet = {0};

	memset(in_buf, 0, sizeof(in_buf));
	in_buf[0] = auto_upgrade;
	ret = GetDeivceInfoByID(&cabinet, terminal_id);
	if (ret < 0){
		WriteLogFile("没有该设备");
		return ret;
	}
	return  ProtocolSetAutoUpgade(cabinet.ca_SettingIP, cabinet.port, in_buf, 1);
}


/*********************************************************************
-   Function : query_usbport_state
-   Description：获取vpn设置
-   Calls：
-   Calles By:
-   Table Accessed :
-   Table Update :
-   Input  :
-   WriteLogFileput :
-   Return :
-   Other :
***********************************************************************/
int terminal_get_vpn(char* terminal_id, ST_ClientVPNSetting *vpn)
{
	int ret = 0;
	unsigned char in_buf[100];
	struct CabinetInfo cabinet = {0};

	memset(in_buf, 0, sizeof(in_buf));
	ret = GetDeivceInfoByID(&cabinet, terminal_id);
	if (ret < 0){
		WriteLogFile("没有该设备");
		return ret;
	}
	return  ProtocolGetVPN(cabinet.ca_SettingIP, cabinet.port, vpn);
}

/*********************************************************************
-   Function : query_usbport_state
-   Description：设置终端设备自动升级
-   Calls：
-   Calles By:
-   Table Accessed :
-   Table Update :
-   Input  :
-   WriteLogFileput :
-   Return :
-   Other :
***********************************************************************/
int terminal_get_autoupgrade(char* terminal_id, char* WriteLogFile_buf, int WriteLogFile_size)
{
	int ret = 0;
	unsigned char in_buf[100];
	struct CabinetInfo cabinet = {0};

	memset(in_buf, 0, sizeof(in_buf));
	ret = GetDeivceInfoByID(&cabinet, terminal_id);
	if (ret < 0){
		WriteLogFile("没有该设备");
		return ret;
	}
	return  ProtocolGetAutoUpgrade(cabinet.ca_SettingIP, cabinet.port, WriteLogFile_buf, WriteLogFile_size);
}


/*根据端口号查询设备表位置(老版本命令)*/
static int find_cabinet_by_usbport(struct Cabinets * cabinet_data, int usbport)
{
	int i = 0;
	for (i = 0; i < 128; i++){
		if ((usbport >= cabinet_data->attribute[i].i_USBOriginNum) && (usbport <= cabinet_data->attribute[i].i_USBSumNumber)){
			WriteLogFile("cabinet_data->attribute[%d].origin_num = %d , cabinet_data->attribute[%d].usb_port_sum = %d", i, cabinet_data->attribute[i].i_USBOriginNum, i, cabinet_data->attribute[i].i_USBSumNumber);

			if (cabinet_data->attribute[i].terminal_system_state == 0){
				return DF_ERR_TERMINAL_OFFLINE;//不在线
			}
			else{
				
				return i;
			}
		}
		else{
			WriteLogFile("cabinet_data->attribute[%d].origin_num = %d , cabinet_data->attribute[%d].usb_port_sum = %d", i, cabinet_data->attribute[i].i_USBOriginNum, i, cabinet_data->attribute[i].i_USBSumNumber);

			continue;
		}
	}
	return DF_ERR_USBPORT_ERROR;
}

int GetIndexById(uint8 * terminal_id)
{
	int i = 0;
	for (i = 0; i <128 ; i++){
		if (strcmp(stream.attribute[i].ca_TerminalID, terminal_id) == 0){
			if (stream.attribute[i].terminal_system_state == 0)
				return DF_ERR_TERMINAL_OFFLINE;
			else{
				return i;
			}
		}
	}
	return DF_ERR_TERMINAL_ID;
}

/*多线程方式循环查询配置表中的设备信息
(未实现:任务队列链表->工作线程执行任务(优先级顺序:新配置到中间件的终端查询、查询不在线的设备、已经查询过的终端设备)(取完任务就删除掉任务队列任务)->添加新的任务)*/
DWORD WINAPI cabinet_thread1(LPARAM lparam)
{
	struct Cabinets * cabinet_data = (struct Cabinets *)lparam;
	struct CabinetInfo *stp_cabinetinfo = NULL;
	SOCKET clifd;
	for (;;Sleep(5000)){
		WaitForSingleObject(cabinet_data->hmutex, INFINITE);
		stp_cabinetinfo = SearchFreeCabinet();
		ReleaseMutex(cabinet_data->hmutex);
		
		if (stp_cabinetinfo == NULL) 
		{
			//WriteLogFile("stp_cabinetinfo == NULL");
			continue;
		}
		if (*(stp_cabinetinfo->ca_SettingIP) == 0){//如果这个ip为空
			//WriteLogFile("stp_cabinetinfo->ca_SettingIP:%s", stp_cabinetinfo->ca_SettingIP);
			continue;
		}
#ifdef NINGBO_BANK
		char remote_ip[100] = { 0 };
		int remote_port;
		for (int i = 0; i < all_terminal_setting_table.count; i++){
			//WriteLogFile("ca_SettingIP :%s\n", all_terminal_setting_table.terminal_setting[i].terminal_ip);
			if ((memcmp(all_terminal_setting_table.terminal_setting[i].terminal_ip, stp_cabinetinfo->ca_SettingIP, strlen(stp_cabinetinfo->ca_SettingIP)) == 0) \
				&& (stp_cabinetinfo->port == all_terminal_setting_table.terminal_setting[i].terminal_port))
			{
				//WriteLogFile("cabinet_thread1 配置中找到对应ip:%s\n", stp_cabinetinfo->ca_SettingIP);
				memcpy(remote_ip, all_terminal_setting_table.terminal_setting[i].remote_ip, strlen(all_terminal_setting_table.terminal_setting[i].remote_ip));
				remote_port = all_terminal_setting_table.terminal_setting[i].remote_port;
				break;
			}
		}
		if (strlen(remote_ip) == 0)
		{
			//WriteLogFile("未在配置中找到对应ip:%s", stp_cabinetinfo->ca_SettingIP);
			return DF_ERR_SOCKET_ERROR;
		}
#endif
		//WriteLogFile("stp_cabinetinfo->check:%d", stp_cabinetinfo->check);
		if (stp_cabinetinfo->check == CHECKED){//已获取了该终端信息 进行连接测试 
			//WriteLogFile("stp_cabinetinfo->check == CHECKED");
			if ((clifd = socket_connect(stp_cabinetinfo->ca_SettingIP, stp_cabinetinfo->port, remote_ip, remote_port,3000)) == SOCKET_ERROR){
				WriteLogFile("终端IP=%s 机柜已离线", stp_cabinetinfo->ca_SettingIP);
				WaitForSingleObject(cabinet_data->hmutex , INFINITE);
				stp_cabinetinfo->check = UNCHECK;//该终端已不在线
				stp_cabinetinfo->terminal_system_state = 0;
				ReleaseMutex(cabinet_data->hmutex);
			}
			else{
				//WriteLogFile("protocol_get_cabinet_all_usbport_state");
				// WriteLogFile("protocol_get_cabinet_all_usbport_state 66666");
				protocol_get_cabinet_all_usbport_state(stp_cabinetinfo->ca_SettingIP, stp_cabinetinfo->port, stp_cabinetinfo->port_state);
				socket_close(clifd);
				stp_cabinetinfo->check = CHECKED;
				continue;
			}
		}
		else{
			//WriteLogFile("########lbc enetr socket_connect,ip=%s,port=%d",stp_cabinetinfo->ca_SettingIP,stp_cabinetinfo->port);
			if ((clifd = socket_connect(stp_cabinetinfo->ca_SettingIP, stp_cabinetinfo->port, remote_ip, remote_port, 3000)) == SOCKET_ERROR){
				//WriteLogFile("1234");
				WaitForSingleObject(cabinet_data->hmutex, INFINITE);
				//WriteLogFile("1111");
				stp_cabinetinfo->check = UNCHECK;//该终端已不在线
				stp_cabinetinfo->terminal_system_state = 0;
				ReleaseMutex(cabinet_data->hmutex);
			}
			else{
				//WriteLogFile("22222");
				socket_close(clifd);
				if (protocol_get_cabinet_attribute(stp_cabinetinfo->ca_SettingIP, stp_cabinetinfo->port, stp_cabinetinfo) < 0){
					//WriteLogFile("ip = %s 获取机柜信息失败", stp_cabinetinfo->ca_SettingIP);
					WaitForSingleObject(cabinet_data->hmutex, INFINITE);
					stp_cabinetinfo->check = UNCHECK;//该终端已不在线
					stp_cabinetinfo->terminal_system_state = 0;
					ReleaseMutex(cabinet_data->hmutex);
				}
				else{
					//WriteLogFile("33333");
					// WriteLogFile("protocol_get_cabinet_all_usbport_state 77777");
					protocol_get_cabinet_all_usbport_state(stp_cabinetinfo->ca_SettingIP, stp_cabinetinfo->port, stp_cabinetinfo->port_state);
					WaitForSingleObject(cabinet_data->hmutex, INFINITE);
					stp_cabinetinfo->terminal_system_state = 1;
					stp_cabinetinfo->check = CHECKED;
					ReleaseMutex(cabinet_data->hmutex);
				}
				//WriteLogFile("lbc  stp_cabinetinfo->ca_TerminalID=%s",stp_cabinetinfo->ca_TerminalID);
				
				//WriteLogFile("lbc  stream.attribute[0].ca_TerminalID=%s",stream.attribute[0].ca_TerminalID);
			}
		}
	}
}

//根据设备id计算设备型号
int cal_device_model(char *machine_id, char*s_buf)
{
	uint8 mach_type[60] = {0};

	if (machine_id[4] == 0x31 || machine_id[4] == 0x32 || machine_id[4] == 0x33)
	{
		sprintf(mach_type, "MB_16");
		memcpy(mach_type + 5, machine_id + 3, 2);
	}
	if (machine_id[4] == 0x34)
	{
		sprintf(mach_type, "MB_1702");
	}
	if (machine_id[5] != 0x30)
	{
		memset(mach_type + 7, machine_id[5] + 0x10, 1);
	}
	memcpy(s_buf, mach_type, 10);
	return 0;
}



struct CabinetInfo* SearchFreeCabinet()
{
	int i = 0;
	int i_RetNum = 0;

	for (i = 0; i < 128; i++){
		//WriteLogFile("stream.attribute[%d].check = %d,stream.attribute[i_CheckingNum].ca_SettingIP[0] = %d", i_CheckingNum,stream.attribute[i_CheckingNum].check, stream.attribute[i_CheckingNum].ca_SettingIP[0]);
		if (stream.attribute[i_CheckingNum].check == CHECKING || stream.attribute[i_CheckingNum].ca_SettingIP[0] == 0){
			i_CheckingNum++;
			if (i_CheckingNum > 127)
				i_CheckingNum = 0;
			continue;
		}
		else{
			stream.attribute[i_CheckingNum].check = CHECKING;
			i_RetNum = i_CheckingNum;
			i_CheckingNum++;
			if (i_CheckingNum > 127)
				i_CheckingNum = 0;
			//WriteLogFile("SearchFreeCabinet i_RetNum = %d",i_RetNum);
			return stream.attribute + i_RetNum;
		}
	}
	return NULL;
}

/*********************************************************************
-   Function : DeleteDir
-   Description：删除指定目录下所有文件(不包括目录)
-   Calls：
-   Calles By:
-   Table Accessed :
-   Table Update :
-   Input  :
-   WriteLogFileput :
-   Return :
-   Other :
***********************************************************************/
BOOL DeleteDir(uint8* dir_path)
{
	struct _finddata_t fileDir;
	uint8 file_path[MAX_PATH] = { 0 }, base_path[MAX_PATH] = {0};
	long lfDir;

	memcpy(base_path, dir_path, strlen(dir_path));

	*(strrchr(base_path, '\\') + 1) = 0;

	if ((lfDir = _findfirst(dir_path, &fileDir)) == -1l)
		;
	else{
		do{
			sprintf(file_path, "%s%s", base_path, fileDir.name);
			if (fileDir.attrib == _A_ARCH)
				remove(file_path);
		} while (_findnext(lfDir, &fileDir) == 0);
	}
	_findclose(lfDir);
	return 0;
}

static void ThreadGetInvoiceDataByMWriteLogFileh(PVOID lpParam);



/*********************************************************************
-   Function : TerminalGetInvoiceNum
-   Description：非阻塞方式获取发票信息
-   Calls：
-   Calles By:
-   Table Accessed :
-   Table Update :
-   Input  :
-   WriteLogFileput :
-   Return :
-   Other :
***********************************************************************/
int TerminalGetInvoiceInfo(char* terminal_id, int usbport, char* date, char* dir, int *p_InvocieNum)
{
	//struct CabinetInfo cabinet = { 0 };
	//int ret = 0;

	////根据终端ID查找终端信息
	//ret = GetDeivceInfoByID(&cabinet , terminal_id);
	//if (ret < 0){
	//	WriteLogFile("没有该设备");
	//	return ret;
	//}
	///*查询发票总数*/
	//ret = ProtocolGetInvoiceNumByMWriteLogFileh(cabinet.ca_SettingIP, cabinet.port, usbport, date, p_InvocieNum);
	//if (ret != 0){
	//	return ret;
	//}
	//else{
	//	/*如果为0 直接返回无发票*/
	//	if (*p_InvocieNum == 0){
	//		return DF_ERR_INVOICE;
	//	}

	//	//创建线程获取发票信息

	//	/*查询搜索的税盘(终端id、端口号、日期)是否已经查询过,如果查询过重新初始化结点字段，否则动态分配结点插入队列中*/ 
	//	struct UnBlockRetrieveGetInvoiceNode *pst_Tmp = NULL, *UnBlockNode = NULL;
	//	if ((pst_Tmp = SearchNodeFromHeadTable(terminal_id, usbport, date))){
	//		pst_Tmp->p_InvoiceHead->InvoiceSumNum = 0;
	//		pst_Tmp->p_InvoiceHead->RecvedInvoiceNum = 0;
	//		memset(pst_Tmp->dir, 0, sizeof(pst_Tmp->dir));
	//		memcpy(pst_Tmp->dir, dir, strlen(dir));
	//		UnBlockNode = pst_Tmp;
	//	}
	//	else{
	//		UnBlockNode = malloc(sizeof(struct UnBlockRetrieveGetInvoiceNode));
	//		if (!UnBlockNode) {
	//			return DF_SYSTEM_ERROR;
	//		}
	//		memset(UnBlockNode, 0, sizeof(struct UnBlockRetrieveGetInvoiceNode));
	//		memcpy(UnBlockNode->terminal_id, terminal_id, strlen(terminal_id));
	//		memcpy(UnBlockNode->date, date, strlen(date));
	//		memcpy(UnBlockNode->dir, dir, strlen(dir));
	//		UnBlockNode->p_InvoiceHead = malloc(sizeof(struct InvoiceDataHead));
	//		if (!UnBlockNode->p_InvoiceHead){
	//			if (UnBlockNode) free(UnBlockNode);
	//			return DF_SYSTEM_ERROR;
	//		}
	//		memset(UnBlockNode->p_InvoiceHead, 0, sizeof(struct InvoiceDataHead));
	//		UnBlockNode->usbport = usbport;

	//		if (InsertToUbBlockHeadTable(UnBlockNode) != TRUE){//添加到链表中（链表中已存在）
	//			return DF_SYSTEM_ERROR;
	//		}
	//	}
	//	UnBlockNode->p_InvoiceHead->InvoiceSumNum = *p_InvocieNum;
	//	HANDLE thread_id = NULL;
	//	thread_id = CreateThread(0 , 0 , (LPTHREAD_START_RWriteLogFileINE)ThreadGetInvoiceDataByMWriteLogFileh, UnBlockNode, 0, 0);
	//	if (thread_id == NULL){
	//		WriteLogFile("创建查询发票信息线程失败");
	//		return DF_SYSTEM_ERROR;
	//	}
	//	else{
	//		CloseHandle(thread_id);
	//	}
	//	
	//	return 0;
	//}

	return 0;

}
/*非阻塞方式获取的发票信息结点插入到链表中*/
static BOOL InsertToUbBlockHeadTable(struct UnBlockRetrieveGetInvoiceNode *node)
{
	/*struct UnBlockRetrieveGetInvoiceNode *tmp = unblock_invocie_head;

	while (tmp != NULL){
		if (tmp->next == NULL){
			tmp->next = node;
			return TRUE;
		}
		tmp = tmp->next;
	}*/
	return FALSE;
}
/*********************************************************************
-   Function : DeleteFromUnblockHeadTable
-   Description：从非阻塞链表中删除节点
-   Calls：
-   Calles By:
-   Table Accessed :
-   Table Update :
-   Input  :
-   WriteLogFileput :
-   Return :
-   Other :
***********************************************************************/
BOOL DeleteFromUnblockHeadTable(uint8* terminal_id, int32 usbport, uint8* date)
{
	/*struct UnBlockRetrieveGetInvoiceNode *tmp = unblock_invocie_head, *free_invoice_node = NULL;

	while (tmp->next != NULL){
		if ((tmp->next->terminal_id != 0) && (tmp->next->date != 0) && (tmp->next->usbport != 0) && (strcmp(tmp->next->terminal_id, terminal_id) == 0) && (tmp->next->usbport == usbport) && (strcmp(tmp->next->date, date) == 0)){
			free_invoice_node = tmp->next;
			tmp->next = free_invoice_node->next;

			if (free_invoice_node->p_InvoiceHead != NULL){
				free(free_invoice_node->p_InvoiceHead);
				free_invoice_node->p_InvoiceHead = NULL;
			}
			if (free_invoice_node != NULL){
				free(free_invoice_node);
				free_invoice_node = NULL;
			}
		}
		tmp = tmp->next;
		if (tmp == NULL) return TRUE;
	}*/
	return TRUE;
}

/*********************************************************************
-   Function : DeleteFromUnblockHeadTable
-   Description：根据终端id、端口号、日期 从非阻塞链表中查找节点
-   Calls：
-   Calles By:
-   Table Accessed :
-   Table Update :
-   Input  :
-   WriteLogFileput :
-   Return :
-   Other :
***********************************************************************/

struct UnBlockRetrieveGetInvoiceNode * SearchNodeFromHeadTable(uint8* terminal_id, uint32 usbport, uint8* date)
{
	/*struct UnBlockRetrieveGetInvoiceNode *tmp_node = unblock_invocie_head;

	while (tmp_node->next != NULL){
		if ((strcmp(tmp_node->next->terminal_id, terminal_id) == 0) && (tmp_node->next->usbport == usbport) && (strcmp(tmp_node->next->date, date) == 0)){
			return tmp_node->next;
		}
		tmp_node = tmp_node->next;
	}*/
	return NULL;
}

//线程方式获取发票信息
static void ThreadGetInvoiceDataByMWriteLogFileh(PVOID lpParam)
{
	//struct UnBlockRetrieveGetInvoiceNode *UnBlockNode = (struct UnBlockRetrieveGetInvoiceNode *)lpParam;
	//struct CabinetInfo cabinet = { 0 };
	//int  ret = 0;
	////根据终端ID查找终端信息
	//ret = GetDeivceInfoByID(&cabinet, UnBlockNode->terminal_id);
	//if (ret < 0){
	//	WriteLogFile("没有该设备");
	//	FreeJsonInvoice(UnBlockNode->p_InvoiceHead->next);
	//	UnBlockNode->i_ErrNum = ret;
	//	return;
	//}

	//if ((ret = ProtocolGetTaxInfoByMWriteLogFileh(cabinet.ca_SettingIP, cabinet.port, UnBlockNode->usbport, UnBlockNode->date, UnBlockNode->dir, UnBlockNode->p_InvoiceHead)) < 0){
	//	UnBlockNode->i_ErrNum = ret;
	//}
	//FreeJsonInvoice(UnBlockNode->p_InvoiceHead->next);
	//UnBlockNode->p_InvoiceHead->next = NULL;
}

int TerminalGetInvocieByMWriteLogFileh(char* terminal_id, int usbport, char* date, char* dir, struct InvoiceDataHead *p_InvoiceDataHead)
{
	/*int ret = 0, details_num = 0, first_flag = 0, postion = 0, i_InvoiceSum = 0;
	struct CabinetInfo cabinet = { 0 };

	ret = GetDeivceInfoByID(&cabinet, terminal_id);
	if (ret < 0){
		WriteLogFile("没有该设备");
		return ret;
	}

	ret = ProtocolGetInvoiceSum(cabinet.ca_SettingIP, cabinet.port, usbport, date, &i_InvoiceSum);
	if (ret != 0){
		WriteLogFile("获取发票总数失败");
		return ret;
	}
	p_InvoiceDataHead->InvoiceSumNum = i_InvoiceSum;


	ret = ProtocolGetTaxInfoByMWriteLogFilehStatic(cabinet.ca_SettingIP, cabinet.port, usbport, date, dir, p_InvoiceDataHead);
	if (ret != 0){
		return ret;
	}*/

	return 0;
}


//获取金税盘发票种类
int TerminalGetInvoiceTypeNum(char* terminal_id, int usbport, unsigned char* result_buf)
{
	struct CabinetInfo cabinet = { 0 };
	int ret = 0;

	//根据终端ID查找终端信息
	ret = GetDeivceInfoByID(&cabinet, terminal_id);
	if (ret < 0){
		WriteLogFile("没有该设备");
		return ret;
	}
	return  ProtocolGetTaxType(cabinet.ca_SettingIP, cabinet.port, usbport, result_buf);

}

//获取金税盘发票监控信息
int TerminalGetTaxGuardInfo(char* terminal_id, int usbport, int tax_type, struct InvoiceGuardInfo* guard_info)
{
	struct CabinetInfo cabinet = {0};
	int ret = 0;

	//根据终端ID查找终端信息
	ret = GetDeivceInfoByID(&cabinet, terminal_id);
	if (ret < 0){
		WriteLogFile("没有该设备");
		return ret;
	}
	return  ProtocolGetTaxGuardInfo(cabinet.ca_SettingIP, cabinet.port, usbport, tax_type, guard_info);
}


/*********************************************************************
-   Function : GetAllInvoiceSectionInfo
-   Description：查询金税盘发票卷信息
-   Calls：
-   Calles By:
-   Table Accessed :
-   Table Update :
-   Input  :
-   WriteLogFileput :
-   Return :
-   Other :
***********************************************************************/
int GetAllInvoiceSectionInfo(char* terminal_id, int usbport, int tax_type, struct AllInvoiceSectionInfo * invoices)
{
	struct CabinetInfo cabinet = {0};
	int ret = 0;

	//根据终端ID查找终端信息
	ret = GetDeivceInfoByID(&cabinet, terminal_id);
	if (ret < 0){
		WriteLogFile("没有该设备");
		return ret;
	}

	return ProtocolGetInvoiceSectionInfo(cabinet.ca_SettingIP, cabinet.port, usbport, tax_type, invoices);
}
/*********************************************************************
-   Function : TerminalGetAreacode
-   Description：获取(金税盘)区域代码
-   Calls：
-   Calles By:
-   Table Accessed :
-   Table Update :
-   Input  :
-   WriteLogFileput :
-   Return :
-   Other :
***********************************************************************/
int TerminalGetAreacode(char* terminal_id, int usbport, char* areacode)
{
	struct CabinetInfo cabinet = { 0 };
	int ret = 0;


	ret = GetDeivceInfoByID(&cabinet, terminal_id);
	if (ret < 0){
		WriteLogFile("没有该设备");
		return ret;
	}
	return ProtocolGetAreacode(cabinet.ca_SettingIP, cabinet.port, usbport, areacode);
}

/*********************************************************************
-   Function : TerminalGetInvoiceSum
-   Description：获取(金税盘)发票总数
-   Calls：
-   Calles By:
-   Table Accessed :
-   Table Update :
-   Input  :
-   WriteLogFileput :
-   Return :
-   Other :
***********************************************************************/
int TerminalGetInvoiceSum(char* terminal_id, int usbport, char* date, int* invoice_num)
{

	struct CabinetInfo cabinet = {0};
	int ret = 0;

	//根据终端ID查找终端信息
	ret = GetDeivceInfoByID(&cabinet, terminal_id);
	if (ret < 0){
		WriteLogFile("没有该设备");
		return ret;
	}

	return ProtocolGetInvoiceSum(cabinet.ca_SettingIP, cabinet.port, usbport, date, invoice_num);
}



/*********************************************************************
-   Function : TerminalGetTaxPlateInfoUnify
-   Description：获取税盘信息统一接口
-   Calls：
-   Calles By:
-   Table Accessed :
-   Table Update :
-   Input  :
-   WriteLogFileput :
-   Return :
-   Other :
***********************************************************************/
int TerminalGetTaxPlateInfoUnify(uint8* terminal_id, struct st_TaxPlateInfoUnify* pst_UnifyInfo)
{
	struct CabinetInfo cabinet = { 0 };
	int ret = 0;

	//根据终端ID查找终端信息
	ret = GetDeivceInfoByID(&cabinet, terminal_id);
	if (ret < 0){
		WriteLogFile("没有该设备");
		return ret;
	}
	return ProtocolGetAllTaxPlateInfoUnify(cabinet.ca_SettingIP, cabinet.port, pst_UnifyInfo);
}
