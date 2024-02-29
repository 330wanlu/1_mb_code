#ifndef TERMINAL_PROTOCOL_10001_H_
#define TERMINAL_PROTOCOL_10001_H_

#include "socket_client.h"
#include "client_portocol.h"
//#include "terminal_manage.h"

#define PER_TIMES_NUMBER  100   //每次查询的发票数量

struct Cabinet_Network_Param{
	int network_type;
	char ip[24];
	char mask[24];
	char gate[24];
	char DNS[24];
	char FTP_upgrade_address[60];
	char FTP_user[20];
	char FTP_secret[20];
};

struct TerminalProtocolParam{
	char c_ip[100];
	int i_port;
	int i_cmd;
	char *p_in_buf;
	int i_insize;

	char* p_out_buf;
	int* p_outsize;

};

struct TerminalProtocol0226_Result
{
	char ca_Version[20];
	char ca_HeartBeat[10];
};


int protocol_query_terminal_soft_version(char *ip, int port, struct CabinetInfo *pst_cabinetinfo);

int ProtocolGetUsbportState(char *client_id, char *ip, int port, int usbport, struct Port_State *port_state);

int protocol_get_cabinet_attribute(char *ip, int port, struct CabinetInfo *pst_cabinetinfo);

int ProtocolSetFTP(struct Cabinet_Network_Param * network_param, char *ip, int port);

int ProtocolGetCabinetUsbportDetail(char *ip, int port, int usbport, struct Port_State *port_state);

/**********************************************************不用的接口***************************************************************/
/*获取所有的ca纳税人税号和名称*/
int ProtocolGetTerminalAllCATaxpayerName(char* ip, int port, struct CabinetInfo *cabinet);


/**********************************************************不常用的接口**************************************************************/
/*配置终端VPN设置*/
int ProtocolSetVPN(char * ip, int port, char *in_buf, int in_size);
/*获取终端VPN设置*/
int ProtocolGetVPN(char * ip, int port, ST_ClientVPNSetting *vpn);
/*设置终端升级设置*/
int ProtocolSetAutoUpgade(char * ip, int port, char * in_buf, int in_size);
/*获取终端升级设置*/
int ProtocolGetAutoUpgrade(char * ip, int port, unsigned char* out,int out_size);

/*获取发票段信息*/
int ProtocolGetInvoiceSectionInfo(char* ip, int port, int usbport, char tax_type, struct AllInvoiceSectionInfo * invoices);
/*获取发票总数*/
int ProtocolGetInvoiceNumByMouth(char* ip, int port, int usbport, char *date, int *p_InvocieNum);
/*获取区域代码*/
int ProtocolGetAreacode(char* ip, int port, int usbport, char* areacode);
/*获取发票总数*/
int ProtocolGetInvoiceSum(char* ip, int port, int usbport, char *date, int* invoice_num);


/********************************************************常用的接口功能*******************************************************************/
/*打开端口*/
int ProtocolOpenUsbport(char *client_id, char *ip, int port, int usbport, int *p_usb_handle, char* busid);
/*关闭端口*/
int ProtocolCloseUsbport(char *client_id, char *ip, int port, int usbport, int usb_hanlde);
/*强制关闭端口*/
int ProtocolCloseUsbportForce(char *client_id, char *ip, int port, int usbport);
/*重启终端*/
int ProtocolRestartCabinet(char *client_id, char* ip, int port);
/*设置网络参数*/
int ProtocolSetNetwork(struct Cabinet_Network_Param * network_param, char *ip, int port);
/*升级机柜程序*/
int ProtocolUpgradeProgram(char *ip, int port);
/*按月查询发票明细*/
int ProtocolGetTaxInfoByMouth(char* ip, int port, int usbport, char *date, char* dir, struct InvoiceDataHead* out_data);
/*查询税盘信息统一接口*/
int ProtocolGetAllTaxPlateInfoUnify(char* ip, int port, struct st_TaxPlateInfoUnify * pst_TaxPlateInfo);

int ProtocolGetTaxInfoByMouthStatic(char* ip, int port, int usbport, char *date, char* dir, struct InvoiceDataHead* p_InvoiceHead);

int ProtocolHeartBeat(char* ip, int port, struct TerminalProtocol0226_Result * stp_HeartBeat);

int ProtocolGetTerminalAllUSBInfoList(char* ip, int port, struct CabinetInfo *cabinet);
int protocol_get_cabinet_all_usbport_state(char *ip, int port, struct Port_State *port_state);
int protocol_query_IP_param(struct Cabinet_Network_Param * network_param, char *ip, int port);
int protocol_query_FTP_param(struct Cabinet_Network_Param * network_param, char *ip, int port);
int ProtocolGetTaxType(char* ip, int port, int usbport, char* result_buf);
int ProtocolGetTaxGuardInfo(char* ip, int port, int usbport, char tax_type, struct InvoiceGuardInfo* guard_info);
#endif