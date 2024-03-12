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
	char cabinet_id[12]; //�˿����ڻ�����
	int usbport;//usb port
	char busid[20];//�˿���������
	char driver[52];//usb�豸�ַ�����
	char user_ip[24];//ʹ����IP
	char client_id[40];//�ͻ���ID
	char device_ip[24];//�豸IP
	char port_exist_device;//�˿��Ƿ����豸
	char port_device_type;//�˿��ϲ���豸����
	char device_protocol_type[2];

	unsigned char c_port_state; //0x00  0x02 0x03 0x04 �˿�ʹ��״̬
	char interface_num;
	char CAName[150];//��˰������
	char ReadCAName;

	//0214����
	unsigned char USB_Type;//usb���� 
	unsigned char supportCA;//�Ƿ�֧��CA֤�� 0 ��֧��  1 ��˰������ 2 ��˰��ʶ��� 3 ���ܶ�
	unsigned char PID[2];
	unsigned char VID[2];
	unsigned char tax_type[2];//������
	unsigned char tax_serial[16];//���к�
	unsigned char tax_extension[2];//���̺�

	char tax_number[20];//��˰��ʶ���
};


struct CabinetInfo
{
	char ca_SettingIP[100];//���ñ���IP��ַ
	int used;
	int port;
	int dataport;//���ݶ˿ں�
	int terminal_system_state;//����״̬�ѱ��ϵͳ����״̬

	int i_USBOriginNum;//USB��ʼ��
	int i_USBSumNumber;//�ն�USB����
	int i_CmdPort;//����˿ں�
	int i_DataPort;//usb ip���ݶ˿ں�

	char ca_ProduceTime[12];//��������
	char ca_TerminalModel[20];//�ն��ͺ�
	char ca_Reserve[16];//����
	char ca_Description[52];//����
	char ca_HardwareVersion[8];//Ӳ���汾
	char ca_KernelVersion[32];//�ں˰汾
	char ca_SoftVesion[10];//����汾
	char ca_TerminalID[20];//�������
	char ca_TerminalIP[24];//����ip��ַ
	char ca_SoftName[52];//�������
	char ca_SoftType;//�������
	char ca_ReleaseData[12];//��������
	char ca_SoftVersion[100];//����汾
	char ca_Author[52];//����
	char ca_SoftDescription[52];//�������
	char ca_CodeType[20];//��������
	struct Port_State           port_state[256];

	int check;//0 δ��� 1 ���ڼ�� 2
};

struct Cabinets{
	struct CabinetInfo attribute[128];
	char client_id[40];
	int count;
	HANDLE hmutex;
	HANDLE singal;
	HANDLE process_id[128];
};



//��������ʽ��ȡ��Ʊ��Ϣ������
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

/*��������ѯ��Ʊ�����ͷ*/
//#ifdef UNBLOCK_INVOICE_TABLE
//struct UnBlockRetrieveGetInvoiceNode * unblock_invocie_head;
//#else
//extern struct UnBlockRetrieveGetInvoiceNode * unblock_invocie_head;
//#endif


//˰����Ϣͳһ�ṹ��(��˰�̡�˰����) ʹ������Ҫ�ͷ�
struct st_TaxPlateInfoUnify
{
	char* cp_TaxPlateInfoBeforeUnZlib;//��ѹ֮ǰ����Ϣ
	int i_LenBeforeUnZlib;

	char* cp_TaxPlateInfoAfterUnzlib;//zlib��ѹ֮�����Ϣ
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


int GetDeivceInfoByID(struct CabinetInfo *stp_cabinet, char *terminal_id); //�����ն�ID��ȡ���е��豸��Ϣ
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