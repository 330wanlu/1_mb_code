#ifndef CLIENT_PORTOCOL_H_
#define CLIENT_PORTOCOL_H_


#include <Winsock2.h>
#include "typedef_data.h"
#include "err.h"
#include "Macro.h"
//#ifdef NINGBO_BANK
#define DF_MANAGE_SET_FILE
#include "manage_set_file.h"
//#endif
struct Client_Err{
	int self_err;
	uint8 ret_err;
	char *err_des;
};


struct FPHMDM{
	uint8 fpdm[20];
	uint8 fphm[20];
};

struct ClientInvoiceInfo{
	uint32 invoice_sum;//��Ʊ����

	struct FPHMDM fpdm_hm[10000];

	uint8 dir[260];
};

typedef struct ClientProtocol{
	unsigned char head[4];
	unsigned char cmd;
	unsigned char protocol_data[20000];
}ST_CLientProtocol, *p_ST_ClientProtocol;


typedef struct ClientVPNSetting{
	unsigned char terminal_id[20];
	int open;//�Ƿ���VPN
	int login_type;//VPN��¼��ʽ
	unsigned char VPNServerIP[50];//VPN��������ַ
	unsigned char VPNServerPort[2];//VPN�������˿ں�
	unsigned char account[20];//�û���
	unsigned char passwd[20];//����
	unsigned char FTPPort[2];//FTP���ض˿ں�
	unsigned char FTPUsername[20];//FTP�����û���
	unsigned char FTPPasswd[20];//FTP�����û�����
}ST_ClientVPNSetting;


volatile struct InvoiceSchedulePerCmd
{
	uint8 terminal_id[20];//�ն�id
	uint32 usbport;//�˿ں�

	volatile double invoice_bin_len;//��Ʊ�����������ܳ���
	volatile double invoice_bin_recved;//�ѽ��ܵķ�Ʊ���ݲ峤��

	volatile double invoice_sum;//��Ʊ����
	volatile double invoice_recved;//�ѽ��շ�Ʊ����
	uint32 schedule;//����
};

//��Ʊ��ȡ���ȱ�
struct InvoiceSchedule
{
	struct InvoiceSchedulePerCmd schedule_table[SCHEDULE_TABLE];
};

int ExchangeInvoiceNum(int mode, int type);
int cmd_handle(SOCKET	sockfd, char *ip, int ip_port, void *arg, int* cmd, unsigned char* cmd_buf);
int recv_client_portocol(SOCKET sockfd, unsigned char** out_buf);

#ifdef DF_CLIENT_PROTOCOL_C
static const struct Client_Err client_err[] =
{
	{ DF_ERR_TERMINAL_SYSTEM, 0x01, "�ն�ϵͳ����" },
	{ DF_ERR_USBPORT_NO_DEVICE, 0x06, "�˿����豸" },
	{ DF_ERR_NO_RIGHT_TERMINAL, 0x07, "�޲���Ȩ��" },
	{ DF_ERR_UBSPORT_OPENED, 0x0f, "�豸�ѱ�ʹ��" },
	{ DF_ERR_NO_RIGHT_CLOSE_PORT, 0x10, "��Ȩ�ͷŸö˿�" },
	{ DF_UPGRADE_PROGRAM_ERROR, 0x22, "�����ն˳���ʧ��" },
	{ DF_UPGRADE_PROGRAM_ERROR, 0x23, "�����ն˳���ʧ��" },
	{ DF_ERR_TYPE_NO_SUPPORT, 0x24, "˰�̲�֧��" },
	{ DF_ERR_INVOICE, 0x25, "��ѯ���޷�Ʊ��Ϣ" },
	{ DF_ERR_GUARD, 0x26, "��֧�ִ����ͷ�Ʊ" },
	{ DF_ERR_PORT_CORE_GETFP_PARAMENTS  ,0x27 ,"��ȡ˰�̷�Ʊ��������"},
	{ DF_ERR_PORT_GETFP_OPENR_USB ,0x28 , "��ȡ˰����Ϣ����USBʧ��"},
	{ DF_ERR_PORT_CORE_GET_FP_INVOICE_NUM ,0x29 , "��ȡ˰�̷�Ʊ��������"},
	{ DF_ERR_PORT_CORE_GETFP_INVOICE_LEN ,0x2A , "��ȡ��Ʊ���ݴ�С����"},
	{ DF_ERR_SOCKET_ERROR, 0x81, "�������" },//�������Ӳ���
	{ DF_ERR_PROTOCOL_ERROR, 0x82, "ͨѶЭ�����" },
	{ DF_ERR_EXCUTE_USBSHARE_ERR, 0x85, "USB���߶˿�����ʧ��" },//
	{ DF_ERR_USBPORT_ERROR, 0x86, "�����USB�˿ںŴ���" },
	{ DF_ERR_TERMINAL_ID, 0x8E, "������ն�ID����" },
	{ DF_ERR_TERMINAL_OFFLINE, 0x8F, "�ն˲�����" },
	{ DF_ERR_CLIENT_PROTOCOL, 0x88, "Э�鲻֧��" },
	{ DF_USB_UNLOAD_TIMEOUT, 0x90, "�˿��ͷų�ʱ" },
	{ DF_USB_LOAD_TIMEOUT, 0x91, "�˿ڼ��س�ʱ" },
	{ DF_SEARCH_COMPLETE, 0x92, "˰�̲�ѯ���" },//  
	{ DF_ERR_ON_SEARCHING, 0x93, "���ڲ�ѯ��˰��,����н��Ȳ�ѯ" },//
	{ DF_SYSTEM_ERROR  , 0x94 , "���������"},
	{ DF_ERR_NO_SEARCH_SCHEDULE  , 0x95 , "δ�и��·�Ʊ��ϸ�����ѯ"}
};
#endif


#endif
