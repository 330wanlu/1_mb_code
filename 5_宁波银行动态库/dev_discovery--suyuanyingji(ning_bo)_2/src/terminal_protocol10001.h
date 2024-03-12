#ifndef TERMINAL_PROTOCOL_10001_H_
#define TERMINAL_PROTOCOL_10001_H_

#include "socket_client.h"
#include "client_portocol.h"
//#include "terminal_manage.h"

#define PER_TIMES_NUMBER  100   //ÿ�β�ѯ�ķ�Ʊ����

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

/**********************************************************���õĽӿ�***************************************************************/
/*��ȡ���е�ca��˰��˰�ź�����*/
int ProtocolGetTerminalAllCATaxpayerName(char* ip, int port, struct CabinetInfo *cabinet);


/**********************************************************�����õĽӿ�**************************************************************/
/*�����ն�VPN����*/
int ProtocolSetVPN(char * ip, int port, char *in_buf, int in_size);
/*��ȡ�ն�VPN����*/
int ProtocolGetVPN(char * ip, int port, ST_ClientVPNSetting *vpn);
/*�����ն���������*/
int ProtocolSetAutoUpgade(char * ip, int port, char * in_buf, int in_size);
/*��ȡ�ն���������*/
int ProtocolGetAutoUpgrade(char * ip, int port, unsigned char* out,int out_size);

/*��ȡ��Ʊ����Ϣ*/
int ProtocolGetInvoiceSectionInfo(char* ip, int port, int usbport, char tax_type, struct AllInvoiceSectionInfo * invoices);
/*��ȡ��Ʊ����*/
int ProtocolGetInvoiceNumByMouth(char* ip, int port, int usbport, char *date, int *p_InvocieNum);
/*��ȡ�������*/
int ProtocolGetAreacode(char* ip, int port, int usbport, char* areacode);
/*��ȡ��Ʊ����*/
int ProtocolGetInvoiceSum(char* ip, int port, int usbport, char *date, int* invoice_num);


/********************************************************���õĽӿڹ���*******************************************************************/
/*�򿪶˿�*/
int ProtocolOpenUsbport(char *client_id, char *ip, int port, int usbport, int *p_usb_handle, char* busid);
/*�رն˿�*/
int ProtocolCloseUsbport(char *client_id, char *ip, int port, int usbport, int usb_hanlde);
/*ǿ�ƹرն˿�*/
int ProtocolCloseUsbportForce(char *client_id, char *ip, int port, int usbport);
/*�����ն�*/
int ProtocolRestartCabinet(char *client_id, char* ip, int port);
/*�����������*/
int ProtocolSetNetwork(struct Cabinet_Network_Param * network_param, char *ip, int port);
/*�����������*/
int ProtocolUpgradeProgram(char *ip, int port);
/*���²�ѯ��Ʊ��ϸ*/
int ProtocolGetTaxInfoByMouth(char* ip, int port, int usbport, char *date, char* dir, struct InvoiceDataHead* out_data);
/*��ѯ˰����Ϣͳһ�ӿ�*/
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