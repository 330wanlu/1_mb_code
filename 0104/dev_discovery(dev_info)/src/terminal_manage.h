#ifndef TERMINAL_MANAGE_H_
#define TERMINAL_MANAGE_H_

#include "client_portocol.h"
#include "terminal_protocol10001.h"
#include "manage_set_file.h"
#include <Windows.h>
#include "typedef_data.h"
#include "mylog.h"


struct Port_State
{
	char cabinet_id[12]; //端口所在机柜编号
	int usbport;//usb port
	char busid[20];//端口树形描述
	char driver[52];//usb设备字符描述
	char user_ip[24];//使用者IP
	char client_id[40];//客户端ID
	char device_ip[24];//设备IP
	char port_exist_device;//端口是否有设备
	char port_device_type;//端口上插得设备类型
	char device_protocol_type[2];

	unsigned char c_port_state; //0x00  0x02 0x03 0x04 端口使用状态
	char interface_num;
	char CAName[150];//纳税人名称
	char ReadCAName;

	//0214命令
	unsigned char USB_Type;//usb类型 
	unsigned char supportCA;//是否支持CA证书 0 不支持  1 纳税人名称 2 纳税人识别号 3 都能读
	unsigned char PID[2];
	unsigned char VID[2];
	unsigned char tax_type[2];//盘类型
	unsigned char tax_serial[16];//序列号
	unsigned char tax_extension[2];//分盘号

	char tax_number[20];//纳税人识别号
};


struct CabinetInfo
{
	char ca_SettingIP[100];//配置表中IP地址
	int used;
	int port;
	int dataport;//数据端口号
	int terminal_system_state;//连接状态已变成系统运行状态

	int i_USBOriginNum;//USB起始号
	int i_USBSumNumber;//终端USB总数
	int i_CmdPort;//命令端口号
	int i_DataPort;//usb ip数据端口号

	char ca_ProduceTime[12];//生产日期
	char ca_TerminalModel[20];//终端型号
	char ca_Reserve[16];//保留
	char ca_Description[52];//描述
	char ca_HardwareVersion[8];//硬件版本
	char ca_KernelVersion[32];//内核版本
	char ca_SoftVesion[10];//软件版本
	char ca_TerminalID[20];//机器编号
	char ca_TerminalIP[24];//机柜ip地址
	char ca_SoftName[52];//软件名称
	char ca_SoftType;//软件类型
	char ca_ReleaseData[12];//发布日期
	char ca_SoftVersion[100];//软件版本
	char ca_Author[52];//作者
	char ca_SoftDescription[52];//软件描述
	char ca_CodeType[20];//码字类型
	struct Port_State           port_state[256];

	int check;//0 未检查 1 正在检查 2
};

struct Cabinets{
	struct CabinetInfo attribute[128];
	char client_id[40];
	int count;
	HANDLE hmutex;
	HANDLE singal;
	HANDLE process_id[128];
};



//非阻塞方式获取发票信息链表结点
struct UnBlockRetrieveGetInvoiceNode
{
	char terminal_id[20];
	int usbport;
	char date[10];
	char dir[260];
	struct InvoiceDataHead* p_InvoiceHead;

	struct UnBlockRetrieveGetInvoiceNode* next;
	int i_ErrNum;
};

/*非阻塞查询发票结点表的头*/
//#ifdef UNBLOCK_INVOICE_TABLE
//struct UnBlockRetrieveGetInvoiceNode * unblock_invocie_head;
//#else
//extern struct UnBlockRetrieveGetInvoiceNode * unblock_invocie_head;
//#endif


//税盘信息统一结构体(金税盘、税控盘) 使用完需要释放
struct st_TaxPlateInfoUnify
{
	char* cp_TaxPlateInfoBeforeUnZlib;//解压之前的信息
	int i_LenBeforeUnZlib;

	char* cp_TaxPlateInfoAfterUnzlib;//zlib解压之后的信息
	int i_LenAfterUnZlib;
};
int old_terminal_close_port_force(int usbport);
int cal_device_model(char *machine_id, char*s_buf);
int old_query_cabinet_num_by_usbport(int usbport, struct CabinetInfo *cabinet);
int old_query_cabinet_attribute(struct CabinetInfo* cabinet, int num_cabinets);
int old_updata_terminal_setting(struct file_setting file_setting[], int size_file);
int old_terminal_open_port(int usbport, char *busid, int *p_usb_handle);

int  terminal_restart_terminal(char* terminal_id);
int  terminal_query_cabinet_num_by_usbport(int usbport, struct CabinetInfo *cabinet);

BOOL terminal_init(void);
int  terminal_open_port(char* terminal_id, int usbport, char *busid, int *p_usb_handle);
int  terminal_close_port(char* terminal_id , int usbport);
int  terminal_close_port_force(char* terminal_id, int usbport);
int  terminal_query_cabinet_attribute(struct CabinetInfo* cabinet, int num_cabinets);
int  terminal_restart_terminal(char* terminal_id);

int  terminal_updata_terminal_setting(struct file_setting file_setting[], int size_file);
int  terminal_query_cabinet_num_by_usbport(int usbport, struct CabinetInfo *cabinet);
int  terminal_query_usbport_state(char* terminal_id, int usbport, struct Port_State *port_state);
int  terminal_query_cabinet_network_param(struct Cabinet_Network_Param *network_param, char * terminal_id);
int  terminal_query_cabinet_FTP_param(struct Cabinet_Network_Param *network_param, char * terminal_id);
int  terminal_upgrade_program(char * terminal_id);
int  terminal_udp_search_terminal(struct Cabinets *data);


int GetDeivceInfoByID(struct CabinetInfo *stp_cabinet, char *terminal_id); //根据终端ID获取所有的设备信息
int TerminalSetCabinetNetwork(char* terminal_id, struct Cabinet_Network_Param *network_param);
int TerminalGetTerminalCAInfo(char* terminal_id, struct CabinetInfo *cabinet);
int old_terminal_query_usbport_state(int usbport, struct Port_State *port_state);
int TerminalGetTaxGuardInfo(char* terminal_id, int usbport, int tax_type, struct InvoiceGuardInfo* guard_info);

int terminal_set_vpn(char* terminal_id, ST_ClientVPNSetting *vpn);
int terminal_set_autoupgrade(char* terminal_id, char auto_upgrade);
int terminal_get_vpn(char* terminal_id, ST_ClientVPNSetting *vpn);
int terminal_get_autoupgrade(char* terminal_id, char* out_buf, int out_size);
int old_terminal_close_port(int usbport);
int TerminalGetTerminalUSBListInfo(char* terminal_id, struct CabinetInfo *cabinet);
int TerminalGetInvoiceTypeNum(char* terminal_id, int usbport, unsigned char *result_buf);
int GetHXTaxInfoSummary(char* terminal_id, int usbport, struct InvoiceSchedulePerCmd* data, struct ClientInvoiceInfo* p_invoiceinfo, char* date);
int GetAllInvoiceSectionInfo(char* terminal_id, int usbport, int tax_type, struct AllInvoiceSectionInfo * invoices);
int TerminalGetInvoiceInfo(char* terminal_id, int usbport, char* date, char* tax_num, int *p_InvocieNum);
int TerminalGetInvocieByMouth(char* terminal_id, int usbport, char* date, char* dir, struct InvoiceDataHead *p_InvoiceDataHead);
void FreeInvoice(struct InvDetail * p_fpxx);
int TerminalGetAreacode(char* terminal_id, int usbport, char* areacode);
int TerminalGetInvoiceSum(char* terminal_id, int usbport, char* date, int* invoice_num);
int TerminalGetTaxPlateInfoUnify(char* terminal_id, struct st_TaxPlateInfoUnify* pst_UnifyInfo);
int TerminalGetHeartBeatVersion(char* terminal_id, struct TerminalProtocol0226_Result * stp_HeartBeat);
int terminal_query_terminal_soft_version(char* terminal_id, struct CabinetInfo *pst_cabinetinfo);


struct UnBlockRetrieveGetInvoiceNode * SearchNodeFromHeadTable(uint8* terminal_id, uint32 usbport, uint8* date);
#ifdef DF_TERMINAL_MANAGE_C
static struct Cabinets stream;
#endif
#endif