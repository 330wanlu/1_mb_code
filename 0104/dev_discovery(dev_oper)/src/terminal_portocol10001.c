
#include "typedef_data.h"
#include "terminal_protocol10001.h"
#include "terminal_manage.h"
//#include "log.h"
//#include "InvoiceDetail.h"
//#include "system.h"
#include "err.h"
//#include "zlib.h"
#include "./cJSON/cJSON.h"
//#include "Protocol.h"

//#define WriteLogFile(format,...)	log_out(format , ##__VA_ARGS__ )	

static int run_protocol_0223(char *ip, int port, char *in_buf, int in_size, char* dir, struct InvoiceDataHead* out_data, int *err);
static int run_protocol_0223_static(char *ip, int port, char *in_buf, int in_size, char* dir, struct InvoiceDataHead* out_data, int *err);
static int run_protocol_2(char *ip, int port, int cmd, char *in_buf, int in_size, char **out_buf, int *out_size, int *err);

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

//�ն˴������->�м�������������� 
int  ErrNumberChange(int err_2)
{
	switch (err_2){
	case 6://�˿����豸
		return DF_ERR_USBPORT_NO_DEVICE;
	case 7://�޲���Ȩ��
		return DF_ERR_NO_RIGHT_TERMINAL;
	case 15://�豸�ѱ�ʹ��
		return DF_ERR_UBSPORT_OPENED;
	case 16://��Ȩ�ͷŸö˿�
		return DF_ERR_NO_RIGHT_CLOSE_PORT;
	case 0x22://�����ն˳���ʧ��
		return DF_UPGRADE_PROGRAM_ERROR;
	case 0x23://�����ն˳���ʧ��
		return DF_UPGRADE_PROGRAM_ERROR;
	case 0x24://˰�̲�֧��
		return DF_ERR_TYPE_NO_SUPPORT;
	case 0x25://��ѯ���޷�Ʊ
		return DF_ERR_INVOICE;
	case 0x26://��֧�ִ��෢Ʊ���͵ļ����Ϣ
		return DF_ERR_GUARD;
	default:
		return DF_ERR_TERMINAL_SYSTEM;
	}
}


int ProtocolHeartBeat(char* ip, int port, struct TerminalProtocol0226_Result * stp_HeartBeat)
{
	cJSON* root = NULL, *pjsn_Tmp = NULL;
	unsigned char in_buf[1024] = { 0 }, *pc_OutBuf = NULL;
	int i_OutBufLen = 0, i_Ret = 0, postion = 0, terminal_ip = 0;
	int err_2 = 0;

	if ((i_Ret = run_protocol_2(ip, port, 0x0226, in_buf, postion, &pc_OutBuf, &i_OutBufLen, &err_2)) < 0){
		WriteLogFile("��ѯ�ն��Ƿ�֧������ʧ��");
		goto ERR;
	}
	if (err_2 != 0){
		i_Ret = ErrNumberChange(err_2);
		goto ERR;
	}
	/*����json���*/
	
	root = cJSON_Parse(pc_OutBuf);
	if (root == NULL){
		i_Ret = DF_ERR_PROTOCOL_ERROR;//ͨѶЭ�����
		goto ERR;
	}
	pjsn_Tmp = cJSON_GetObjectItem(root , "version");
	if (pjsn_Tmp == NULL){
		i_Ret = DF_ERR_PROTOCOL_ERROR;//ͨѶЭ�����
		goto ERR;
	}
	if (strlen(pjsn_Tmp->valuestring) > 50){
		i_Ret = DF_ERR_PROTOCOL_ERROR;//ͨѶЭ�����
		goto ERR;
	}
	memcpy(stp_HeartBeat->ca_Version, pjsn_Tmp->valuestring, strlen(pjsn_Tmp->valuestring));


	pjsn_Tmp = cJSON_GetObjectItem(root, "heartbeat");
	if (pjsn_Tmp == NULL){
		i_Ret = DF_ERR_PROTOCOL_ERROR;//ͨѶЭ�����
		goto ERR;
	}
	if (strlen(pjsn_Tmp->valuestring) > 1){
		i_Ret = DF_ERR_PROTOCOL_ERROR;//ͨѶЭ�����
		goto ERR;
	}
	memcpy(stp_HeartBeat->ca_HeartBeat, pjsn_Tmp->valuestring, strlen(pjsn_Tmp->valuestring));

ERR:
	if (root != NULL) cJSON_Delete(root);
	if (pc_OutBuf)free(pc_OutBuf);
	return i_Ret;
}




int ProtocolOpenUsbport(char *client_id, char *ip, int port, int usbport, int *p_usb_handle, char* busid)
{
	unsigned char in_buf[1024] = { 0 }, *pc_OutBuf = NULL;
	int i_OutBufLen = 0, i_Ret = 0, postion = 0, terminal_ip = 0;
	int err_2 = 0;

	in_buf[0] = (usbport >> 24)&(0xFF);
	in_buf[1] = (usbport >> 16)&(0xFF);
	in_buf[2] = (usbport >> 8)&(0xFF);
	in_buf[3] = (usbport)&(0xFF);
	memcpy(in_buf + 4, client_id, 38);
	postion = 42 + 26;
	WriteLogFile("�ն�ͨ��Э�� �򿪶˿�");

	if ((i_Ret = run_protocol_2(ip, port, 0x0301, in_buf, postion, &pc_OutBuf, &i_OutBufLen, &err_2)) < 0){
		WriteLogFile("�ն˴򿪶˿�ʧ��");
		goto ERR;
	}
	if (err_2 != 0){
		i_Ret = ErrNumberChange(err_2);
		goto ERR;
	}
	if (i_OutBufLen != 82){
		WriteLogFile("[����ͨѶ]�ն˴򿪶˿�Э�����");
		i_Ret = DF_ERR_PROTOCOL_ERROR;
		goto ERR;
	}
		
	(*p_usb_handle) = ((((int)pc_OutBuf[0]) << 24) & 0xff000000) + ((((int)pc_OutBuf[1]) << 16) & 0x00FF0000) \
		+ ((((int)pc_OutBuf[2]) << 8) & 0x0000FF00) + ((((int)pc_OutBuf[3])) & 0x000000FF);
	memcpy(busid, pc_OutBuf + 8, 20);
ERR:
	if (pc_OutBuf)free(pc_OutBuf);
	return i_Ret;
}


int ProtocolCloseUsbport(char *client_id, char *ip, int port, int usbport, int usb_hanlde)
{
	unsigned char in_buf[1024] = { 0 }, *pc_OutBuf = NULL;
	int i_OutBufLen = 0, i_Ret = 0, postion = 0, terminal_ip = 0;
	int err_2 = 0;

	in_buf[0] = (usb_hanlde >> 24)&(0xFF);
	in_buf[1] = (usb_hanlde >> 16)&(0xFF);
	in_buf[2] = (usb_hanlde >> 8)&(0xFF);
	in_buf[3] = (usb_hanlde)&(0xFF);

	in_buf[4] = (usbport >> 24)&(0xFF);
	in_buf[5] = (usbport >> 16)&(0xFF);
	in_buf[6] = (usbport >> 8)&(0xFF);
	in_buf[7] = (usbport)&(0xFF);

	postion = 8;

	WriteLogFile("�ն�ͨ��Э�� �رն˿�");

	if ((i_Ret = run_protocol_2(ip, port, 0x0302, in_buf, postion, &pc_OutBuf, &i_OutBufLen, &err_2)) < 0){
		WriteLogFile("�ն˹رն˿�ʧ��");
		goto ERR;
	}
	if (err_2 != 0){
		i_Ret = ErrNumberChange(err_2);
		goto ERR;
	}
	if (i_OutBufLen != 0){
		WriteLogFile("[����ͨѶ]�ն˹رն˿�Э�����");
		i_Ret = DF_ERR_PROTOCOL_ERROR;
		goto ERR;
	}
ERR:
	if (pc_OutBuf)free(pc_OutBuf);
	return i_Ret;
}


int ProtocolCloseUsbportForce(char *client_id, char *ip, int port, int usbport)
{
	unsigned char in_buf[1024] = { 0 }, *pc_OutBuf = NULL;
	int i_OutBufLen = 0, i_Ret = 0, postion = 0, terminal_ip = 0;
	int err_2 = 0;

	in_buf[0] = (usbport >> 24)&(0xFF);
	in_buf[1] = (usbport >> 16)&(0xFF);
	in_buf[2] = (usbport >> 8)&(0xFF);
	in_buf[3] = (usbport)&(0xFF);

	postion = 4;
	WriteLogFile("�ն�ͨ��Э�� ǿ�ƹرն˿ں�");

	if ((i_Ret = run_protocol_2(ip, port, 0x0305, in_buf, postion, &pc_OutBuf, &i_OutBufLen, &err_2)) < 0){
		WriteLogFile("�ն˹رն˿�ʧ��");
		goto ERR;
	}
	if (err_2 != 0){
		i_Ret = ErrNumberChange(err_2);
		goto ERR;
	}
	if (i_OutBufLen != 0){
		WriteLogFile("[����ͨѶ]�ն�ǿ�ƹرն˿�Э�����");
		i_Ret = DF_ERR_PROTOCOL_ERROR;
		goto ERR;
	}
ERR:
	if (pc_OutBuf)free(pc_OutBuf);
	return i_Ret;

}

int protocol_query_IP_param(struct Cabinet_Network_Param * network_param, char *ip, int port)
{

	unsigned char in_buf[1024] = { 0 }, *pc_OutBuf = NULL;
	int i_OutBufLen = 0, i_Ret = 0, postion = 0, terminal_ip = 0;
	int err_2 = 0;

	WriteLogFile("�ն�ͨ��Э�� ��ȡ�ն�IP����");
	if ((i_Ret = run_protocol_2(ip, port, 0x0207, in_buf, postion, &pc_OutBuf, &i_OutBufLen, &err_2)) < 0){
		WriteLogFile("��ȡ�ն�IP����ʧ��");
		goto ERR;
	}
	if (err_2 != 0){
		i_Ret = ErrNumberChange(err_2);
		goto ERR;
	}

	if (i_OutBufLen != 17){
		WriteLogFile("[����ͨѶ]��ȡ�ն�IP����Э�����");
		i_Ret = DF_ERR_PROTOCOL_ERROR;
		goto ERR;
	}

	postion = 0;
	network_param->network_type = pc_OutBuf[postion++];
	terminal_ip = ((((int)pc_OutBuf[1]) << 24) & 0xff000000) + ((((int)pc_OutBuf[2]) << 16) & 0x00FF0000) \
		+ ((((int)pc_OutBuf[3]) << 8) & 0x0000FF00) + ((((int)pc_OutBuf[4])) & 0x000000FF);
	ip2ip_asc(terminal_ip, network_param->ip);
	terminal_ip = ((((int)pc_OutBuf[5]) << 24) & 0xff000000) + ((((int)pc_OutBuf[6]) << 16) & 0x00FF0000) \
		+ ((((int)pc_OutBuf[7]) << 8) & 0x0000FF00) + ((((int)pc_OutBuf[8])) & 0x000000FF);
	ip2ip_asc(terminal_ip, network_param->mask);
	terminal_ip = ((((int)pc_OutBuf[9]) << 24) & 0xff000000) + ((((int)pc_OutBuf[10]) << 16) & 0x00FF0000) \
		+ ((((int)pc_OutBuf[11]) << 8) & 0x0000FF00) + ((((int)pc_OutBuf[12])) & 0x000000FF);
	ip2ip_asc(terminal_ip, network_param->gate);
	terminal_ip = ((((int)pc_OutBuf[13]) << 24) & 0xff000000) + ((((int)pc_OutBuf[14]) << 16) & 0x00FF0000) \
		+ ((((int)pc_OutBuf[15]) << 8) & 0x0000FF00) + ((((int)pc_OutBuf[16])) & 0x000000FF);
	ip2ip_asc(terminal_ip, network_param->DNS);
ERR:
	if (pc_OutBuf)free(pc_OutBuf);
	return i_Ret;
}

int ProtocolSetNetwork(struct Cabinet_Network_Param * network_param, char *ip, int port)
{
	unsigned char in_buf[1024] = { 0 }, *pc_OutBuf = NULL;
	int i_OutBufLen = 0, i_Ret = 0, postion = 0, terminal_ip = 0;
	int err_2 = 0;

	int n32_ip, n32_mask, n32_gate, n32_DNS;

	in_buf[postion++] = network_param->network_type;
	n32_ip = inet_addr(network_param->ip);
	if (INADDR_NONE == n32_ip){
		WriteLogFile("��Ч��IP��ַ");
	}
	in_buf[postion++] = ((n32_ip)& 0xFF);
	in_buf[postion++] = ((n32_ip >> 8) & 0xFF);
	in_buf[postion++] = ((n32_ip >> 16) & 0xFF);
	in_buf[postion++] = ((n32_ip >> 24) & 0xFF);

	n32_mask = inet_addr(network_param->mask);
	if (INADDR_NONE == n32_mask){
		WriteLogFile("��Ч��IP��ַ");
	}
	in_buf[postion++] = ((n32_mask)& 0xFF);
	in_buf[postion++] = ((n32_mask >> 8) & 0xFF);
	in_buf[postion++] = ((n32_mask >> 16) & 0xFF);
	in_buf[postion++] = ((n32_mask >> 24) & 0xFF);


	n32_gate = inet_addr(network_param->gate);
	if (INADDR_NONE == n32_gate){
		WriteLogFile("��Ч��IP��ַ");
	}
	in_buf[postion++] = ((n32_gate)& 0xFF);
	in_buf[postion++] = ((n32_gate >> 8) & 0xFF);
	in_buf[postion++] = ((n32_gate >> 16) & 0xFF);
	in_buf[postion++] = ((n32_gate >> 24) & 0xFF);

	n32_DNS = inet_addr(network_param->DNS);
	if (INADDR_NONE == n32_DNS){
		WriteLogFile("��Ч��IP��ַ");
	}
	in_buf[postion++] = ((n32_DNS)& 0xFF);
	in_buf[postion++] = ((n32_DNS >> 8) & 0xFF);
	in_buf[postion++] = ((n32_DNS >> 16) & 0xFF);
	in_buf[postion++] = ((n32_DNS >> 24) & 0xFF);
	WriteLogFile("�ն�ͨ��Э�� �����ն�IP����");

	if ((i_Ret = run_protocol_2(ip, port, 0x0102, in_buf, postion, &pc_OutBuf, &i_OutBufLen, &err_2)) < 0){
		WriteLogFile("�����ն�IP����ʧ��");
		goto ERR;
	}
	if (err_2 != 0){
		i_Ret = ErrNumberChange(err_2);
		goto ERR;
	}
	if (i_OutBufLen != 0){
		WriteLogFile("[����ͨѶ]�����ն�IP����Э�����");
		i_Ret = DF_ERR_PROTOCOL_ERROR;
		goto ERR;
	}
ERR:
	if (pc_OutBuf)free(pc_OutBuf);
	return i_Ret;
}

int ProtocolSetFTP(struct Cabinet_Network_Param * network_param, char *ip, int port)
{

	unsigned char in_buf[1024] = { 0 }, *pc_OutBuf = NULL;
	int i_OutBufLen = 0, i_Ret = 0, postion = 0, terminal_ip = 0;
	int err_2 = 0;

	memcpy(in_buf + postion, network_param->FTP_upgrade_address, strlen(network_param->FTP_upgrade_address));
	postion += 60;
	memcpy(in_buf + postion, network_param->FTP_user, strlen(network_param->FTP_user));
	postion += 20;
	memcpy(in_buf + postion, network_param->FTP_secret, strlen(network_param->FTP_secret));
	postion += 20;
	WriteLogFile("�ն�ͨ��Э�� �����ն�FTP����");

	if ((i_Ret = run_protocol_2(ip, port, 0x0107, in_buf, postion, &pc_OutBuf, &i_OutBufLen, &err_2)) < 0){
		WriteLogFile("�����ն�FTP����ʧ��");
		goto ERR;
	}
	if (err_2 != 0){
		i_Ret = ErrNumberChange(err_2);
		goto ERR;
	}
	if (i_OutBufLen != 0){
		WriteLogFile("[����ͨѶ]�����ն�FTP����Э�����");
		i_Ret = DF_ERR_PROTOCOL_ERROR;
		goto ERR;
	}
ERR:
	if (pc_OutBuf)free(pc_OutBuf);
	return i_Ret;
}


int protocol_query_FTP_param(struct Cabinet_Network_Param * network_param, char *ip, int port)
{
	unsigned char in_buf[1024] = { 0 }, *pc_OutBuf = NULL;
	int i_OutBufLen = 0, i_Ret = 0, postion = 0, terminal_ip = 0;
	int err_2 = 0;

	WriteLogFile("�ն�ͨ��Э�� ��ȡ�ն�FTP����");
	if ((i_Ret = run_protocol_2(ip, port, 0x0209, in_buf, postion, &pc_OutBuf, &i_OutBufLen, &err_2)) < 0){
		WriteLogFile("��ȡ�ն�FTP����ʧ��");
		goto ERR;
	}
	if (err_2 != 0){
		i_Ret = ErrNumberChange(err_2);
		goto ERR;
	}
	if (i_OutBufLen != 100){
		WriteLogFile("[����ͨѶ]��ȡ�ն�FTP����ʧ��Э�����");
		i_Ret = DF_ERR_PROTOCOL_ERROR;
		goto ERR;
	}
	memcpy(network_param->FTP_upgrade_address, pc_OutBuf, 60);
	memcpy(network_param->FTP_user, pc_OutBuf + 60, 20);
	memcpy(network_param->FTP_secret, pc_OutBuf + 80, 20);

ERR:
	if (pc_OutBuf)free(pc_OutBuf);
	return i_Ret;
}

int ProtocolGetUsbportState(char *client_id, char *ip, int port, int usbport, struct Port_State *port_state)
{

	unsigned char in_buf[1024] = { 0 }, *pc_OutBuf = NULL;
	int i_OutBufLen = 0, i_Ret = 0, postion = 0, terminal_ip = 0;
	int err_2 = 0;

	in_buf[0] = (usbport >> 24)&(0xFF);
	in_buf[1] = (usbport >> 16)&(0xFF);
	in_buf[2] = (usbport >> 8)&(0xFF);
	in_buf[3] = (usbport)&(0xFF);

	postion = 4;
	WriteLogFile("�ն�ͨ��Э�� ��ȡ�ն˶˿���Ϣ");

	if ((i_Ret = run_protocol_2(ip, port, 0x0203, in_buf, postion, &pc_OutBuf, &i_OutBufLen, &err_2)) < 0){
		WriteLogFile("��ѯ�˿���Ϣʧ��[0x0203]");
		goto ERR;
	}
	if (err_2 != 0){
		i_Ret = ErrNumberChange(err_2);
		goto ERR;
	}

	if (i_OutBufLen != 167){
		WriteLogFile("[����ͨѶ]��ѯ�˿���Ϣʧ��Э�����");
		i_Ret = DF_ERR_PROTOCOL_ERROR;
		goto ERR;
	}

	port_state->c_port_state = pc_OutBuf[0];
	memcpy(port_state->cabinet_id, pc_OutBuf + 1, 12);
	port_state->usbport = ((((int)pc_OutBuf[13]) << 8)&(0xFF00)) + ((((int)pc_OutBuf[14]))&(0xFF));
	memcpy(port_state->busid, pc_OutBuf + 29, 20);
	memcpy(port_state->driver, pc_OutBuf + 49, 50);

	terminal_ip = ((((int)pc_OutBuf[99]) << 24) & 0xff000000) + ((((int)pc_OutBuf[100]) << 16) & 0x00FF0000) \
		+ ((((int)pc_OutBuf[101]) << 8) & 0x0000FF00) + ((((int)pc_OutBuf[102])) & 0x000000FF);
	ip2ip_asc(terminal_ip, port_state->user_ip);
	memcpy(port_state->client_id, pc_OutBuf + 103, 38);
	memcpy(port_state->device_ip, ip, strlen(ip));

ERR:
	if (pc_OutBuf)free(pc_OutBuf);
	return i_Ret;
}


int ProtocolGetCabinetUsbportDetail(char *ip, int port, int usbport, struct Port_State *port_state)
{
	unsigned char in_buf[1024] = { 0 }, *pc_OutBuf = NULL;
	int i_OutBufLen = 0, i_Ret = 0, postion = 0, terminal_ip = 0;
	int err_2 = 0;

	in_buf[0] = (usbport >> 24)&(0xFF);
	in_buf[1] = (usbport >> 16)&(0xFF);
	in_buf[2] = (usbport >> 8)&(0xFF);
	in_buf[3] = (usbport)&(0xFF);
	postion = 4;

	WriteLogFile("lbc 11111");
	if ((i_Ret = run_protocol_2(ip, port, 0x0205, in_buf , postion , &pc_OutBuf, &i_OutBufLen, &err_2)) < 0){
		WriteLogFile("��ѯ����usb�˿ں���ϸ��Ϣʧ��[0x0205]");
		goto ERR;
	}
	if (err_2 != 0){
		i_Ret = ErrNumberChange(err_2);
		goto ERR;
	}
	WriteLogFile("��ȡ�˿���ϸ��Ϣ�ɹ�");
	if (i_OutBufLen != 148){
		WriteLogFile("[����ͨѶ]��ѯ����usb�˿ں���ϸ��ϢЭ�����");
		i_Ret = DF_ERR_PROTOCOL_ERROR;
		goto ERR;
	}
	port_state->usbport = usbport;
	port_state->port_exist_device = pc_OutBuf[0];

	if (pc_OutBuf[1] == 0 || pc_OutBuf[1] == 3)
		port_state->c_port_state = 0;
	if ((pc_OutBuf[1] == 1) || (pc_OutBuf[1] == 2) || (pc_OutBuf[1] == 4))
		port_state->c_port_state = 2;
	memcpy(port_state->busid, pc_OutBuf + 2, 20);
	port_state->port_device_type = pc_OutBuf[22];
	memcpy(port_state->device_protocol_type, &pc_OutBuf[23], 2);
	memcpy(port_state->PID, &pc_OutBuf[25], 2);
	memcpy(port_state->VID, &pc_OutBuf[27], 2);
	port_state->interface_num = pc_OutBuf[29];
	memcpy(port_state->driver, &pc_OutBuf[30], 50);

	terminal_ip = ((((int)pc_OutBuf[80]) << 24) & 0xff000000) + ((((int)pc_OutBuf[81]) << 16) & 0x00FF0000) \
		+ ((((int)pc_OutBuf[82]) << 8) & 0x0000FF00) + ((((int)pc_OutBuf[83])) & 0x000000FF);

	ip2ip_asc(terminal_ip, port_state->user_ip);

	memcpy(port_state->client_id, pc_OutBuf + 84, 38);
ERR:
	if (pc_OutBuf)free(pc_OutBuf);
	return i_Ret;
}


int ProtocolRestartCabinet(char *client_id, char *ip, int port)
{
	unsigned char in_buf[1024] = { 0 }, *pc_OutBuf = NULL;
	int i_OutBufLen = 0, i_Ret = 0, postion = 0;
	int err_2 = 0;

	WriteLogFile("�ն�ͨ��Э�� �����ն�");
	if ((i_Ret = run_protocol_2(ip, port, 0x0308, in_buf, 0, &pc_OutBuf, &i_OutBufLen, &err_2)) < 0){
		WriteLogFile("�����ն�ʧ��");
		goto ERR;
	}
	if (err_2 != 0){
		i_Ret = ErrNumberChange(err_2);
		goto ERR;
	}
	if (i_OutBufLen != 0){
		WriteLogFile("[����ͨѶ]�����ն�ʧ��Э�����");
		i_Ret = DF_ERR_PROTOCOL_ERROR;
		goto ERR;
	}
ERR:
	if (pc_OutBuf)free(pc_OutBuf);
	return i_Ret;
}






int ProtocolUpgradeProgram(char *ip, int port)
{
	unsigned char in_buf[1024] = { 0 }, *pc_OutBuf = NULL;
	int i_OutBufLen = 0, i_Ret = 0, postion = 0;
	int err_2 = 0;

	WriteLogFile("�ն�ͨ��Э�� �����������");
	if ((i_Ret = run_protocol_2(ip, port, 0x0312, in_buf, 0, &pc_OutBuf, &i_OutBufLen, &err_2)) < 0){
		WriteLogFile("�����������ʧ��");
		goto ERR;
	}
	if (err_2 != 0){
		i_Ret = ErrNumberChange(err_2);
		goto ERR;
	}
	if (i_OutBufLen != 0){
		WriteLogFile("[����ͨѶ]�����������Э�����");
		i_Ret = DF_ERR_PROTOCOL_ERROR;
		goto ERR;
	}
ERR:
	if (pc_OutBuf)free(pc_OutBuf);
	return i_Ret;
}

int protocol_query_terminal_soft_version(char *ip, int port, struct CabinetInfo *pst_cabinetinfo)
{
	unsigned char in_buf[1024] = { 0 }, *pc_OutBuf = NULL;
	int i_OutBufLen = 0, i_Ret = 0, postion = 0;
	int err_2 = 0;

	//WriteLogFile("#####lbc enter protocol_query_terminal_soft_version");
	if ((i_Ret = run_protocol_2(ip, port, 0x0212, in_buf, 0, &pc_OutBuf, &i_OutBufLen, &err_2)) < 0){
		WriteLogFile("��ѯ��������汾ʧ��[0x0212]");
		goto ERR;
	}
	if (err_2 != 0){
		i_Ret = ErrNumberChange(err_2);
		goto ERR;
	}

	if (i_OutBufLen != 280){
		WriteLogFile("[����ͨѶ]��ѯ��������汾Э�����");
		i_Ret = DF_ERR_PROTOCOL_ERROR;
		goto ERR;
	}
	memcpy(pst_cabinetinfo->ca_SoftName, pc_OutBuf + postion, 50);
	postion += 50;
	pst_cabinetinfo->ca_SoftType = pc_OutBuf[postion++];
	memcpy(pst_cabinetinfo->ca_ReleaseData, pc_OutBuf + postion, 10);
	postion += 10;
	memcpy(pst_cabinetinfo->ca_SoftVersion, pc_OutBuf + postion, 100);
	postion += 100;
	memcpy(pst_cabinetinfo->ca_Author, pc_OutBuf + postion, 50);
	postion += 50;
	memcpy(pst_cabinetinfo->ca_SoftDescription, pc_OutBuf + postion, 50);
	postion += 50;
	memcpy(pst_cabinetinfo->ca_CodeType, pc_OutBuf + postion, 20);
ERR:
	if (pc_OutBuf)free(pc_OutBuf);
	return i_Ret;
}


int protocol_get_cabinet_attribute(char *ip, int port, struct CabinetInfo *pst_cabinetinfo)
{

	unsigned char in_buf[1024] = { 0 }, *pc_OutBuf = NULL;
	int i_OutBufLen = 0, i_Ret = 0, postion = 0, termianl_ip;
	int err_2 = 0;
	//WriteLogFile("#####lbc enter protocol_get_cabinet_attribute");
	if ((i_Ret = run_protocol_2(ip, port, 0x0201, in_buf, 0, &pc_OutBuf, &i_OutBufLen, &err_2)) < 0){
		WriteLogFile("��ѯ������Ϣʧ��[0x0201]");
		goto ERR;
	}
	if (err_2 != 0){
		i_Ret = ErrNumberChange(err_2);
		goto ERR;
	}
	if (i_OutBufLen != 471){
		WriteLogFile("[����ͨѶ]��ѯ������ϢЭ�����");
		i_Ret = DF_ERR_PROTOCOL_ERROR;
		goto ERR;
	}

	postion = 0;
	postion++;
    //WriteLogFile("protocol_get_cabinet_attribute pc_OutBuf=%s",pc_OutBuf);
	memcpy(pst_cabinetinfo->ca_Description, pc_OutBuf + postion, 50); postion += 50;//�������� 50���ֽ�
	memcpy(pst_cabinetinfo->ca_HardwareVersion, pc_OutBuf + postion, 8); postion += 8;//Ӳ���汾
	memcpy(pst_cabinetinfo->ca_KernelVersion, pc_OutBuf + postion, 30); postion += 30;//�ں˰汾
	memcpy(pst_cabinetinfo->ca_SoftVesion, pc_OutBuf + postion, 8); postion += 8;//����汾
	memcpy(pst_cabinetinfo->ca_TerminalID, pc_OutBuf + postion, 12); postion += 12;//�������
	//WriteLogFile("pst_cabinetinfo->ca_TerminalID=%s",pst_cabinetinfo->ca_TerminalID);

	termianl_ip = ((((int)pc_OutBuf[postion]) << 24) & 0xff000000) + ((((int)pc_OutBuf[postion + 1]) << 16) & 0x00FF0000) \
		+ ((((int)pc_OutBuf[postion + 2]) << 8) & 0x0000FF00) + ((((int)pc_OutBuf[postion + 3])) & 0x000000FF); postion += 4;
	ip2ip_asc(termianl_ip, pst_cabinetinfo->ca_TerminalIP);
	
	pst_cabinetinfo->i_USBOriginNum = ((((int)pc_OutBuf[postion]) << 24) & 0xff000000) + ((((int)pc_OutBuf[postion + 1]) << 16) & 0x00FF0000) \
		+ ((((int)pc_OutBuf[postion + 2]) << 8) & 0x0000FF00) + ((((int)pc_OutBuf[postion + 3])) & 0x000000FF); postion += 4;

	pst_cabinetinfo->i_USBSumNumber = ((((int)pc_OutBuf[postion]) << 24) & 0xff000000) + ((((int)pc_OutBuf[postion + 1]) << 16) & 0x00FF0000) \
		+ ((((int)pc_OutBuf[postion + 2]) << 8) & 0x0000FF00) + ((((int)pc_OutBuf[postion + 3])) & 0x000000FF); postion += 4;
	
	pst_cabinetinfo->i_CmdPort = ((((int)pc_OutBuf[postion]) << 8) & 0xFF00) + ((((int)pc_OutBuf[postion + 1])) & 0x00FF); postion += 2;
	pst_cabinetinfo->i_DataPort = ((((int)pc_OutBuf[postion]) << 8) & 0xFF00) + ((((int)pc_OutBuf[postion + 1])) & 0x00FF); postion += 2;

	memcpy(pst_cabinetinfo->ca_SoftName, pc_OutBuf + 180, 50); postion += 50;//�������
	pst_cabinetinfo->ca_SoftType = pc_OutBuf[230]; postion++;//�������
	memcpy(pst_cabinetinfo->ca_ReleaseData, pc_OutBuf + 231, 10); postion += 10;
	memcpy(pst_cabinetinfo->ca_SoftVersion, pc_OutBuf + 241, 100); postion += 100;//����汾
	memcpy(pst_cabinetinfo->ca_Author, pc_OutBuf + 341, 50); postion += 50;//����
	memcpy(pst_cabinetinfo->ca_SoftDescription, pc_OutBuf + 391, 50); postion += 50;//�������
	memcpy(pst_cabinetinfo->ca_CodeType, pc_OutBuf + 441, 20); postion += 20;//��������

	if (*(pc_OutBuf + 461) == '\0'){
		cal_device_model(pst_cabinetinfo->ca_TerminalID, pst_cabinetinfo->ca_TerminalModel);
	}
	else{
		memcpy(pst_cabinetinfo->ca_TerminalModel, pc_OutBuf + 461, 10);
	}

ERR:
	if (pc_OutBuf)free(pc_OutBuf);
	return i_Ret;
}


int protocol_get_cabinet_all_usbport_state(char *ip, int port, struct Port_State *port_state)
{
	unsigned char in_buf[1024] = { 0 }, *pc_OutBuf = NULL;
	int i_OutBufLen = 0, i_Ret = 0, postion = 0, i_PortNum = 0;

	int err_2 = 0;

    WriteLogFile("lbc 22222");
	if ((i_Ret = run_protocol_2(ip, port, 0x0210, in_buf, 0, &pc_OutBuf, &i_OutBufLen, &err_2)) < 0){
		WriteLogFile("��ѯ�������е�usb�˿ں�״̬ʧ��[0x0210]");
		goto ERR;
	}
	if (err_2 != 0){
		i_Ret = ErrNumberChange(err_2);
		goto ERR;
	}

	if (postion > i_OutBufLen){
		i_Ret = DF_ERR_PROTOCOL_ERROR;
		goto ERR;
	}
	i_PortNum = pc_OutBuf[postion++];
	for (int i = 0; i < i_PortNum; i++){
		if (postion > i_OutBufLen){
			i_Ret = DF_ERR_PROTOCOL_ERROR;
			goto ERR;
		}
		port_state[i].c_port_state = pc_OutBuf[postion++];
	}
ERR:
	if (pc_OutBuf)free(pc_OutBuf);
	return i_Ret;
}
/*��ȡ�ն�������ca��˰������*/
int ProtocolGetTerminalAllCATaxpayerName(char* ip, int port, struct CabinetInfo *cabinet)
{
	unsigned char in_buf[1024] = { 0 }, *pc_OutBuf = NULL;
	int i_OutBufLen = 0, i_Ret = 0, postion = 0, i_PortNum = 0;
	int err_2;

    WriteLogFile("lbc 33333");
	if ((i_Ret = run_protocol_2(ip, port, 0x0213, in_buf, 0, &pc_OutBuf, &i_OutBufLen, &err_2)) < 0){
		WriteLogFile("��ȡ�ն�������ca��˰������ʧ��[0x0213]");
		goto ERR;
	}
	if (err_2 != 0){
		i_Ret = ErrNumberChange(err_2);
		goto ERR;
	}

	if (postion > i_OutBufLen){
		i_Ret = DF_ERR_PROTOCOL_ERROR;
		goto ERR;
	}

	postion = 0;
	i_PortNum = pc_OutBuf[0];
	cabinet->i_USBSumNumber = i_PortNum;
	postion = 1;
	for (int i = 0; i < i_PortNum; i++){
		postion++;
		if (postion > i_OutBufLen){
			i_Ret = DF_ERR_PROTOCOL_ERROR;
			goto ERR;
		}
		cabinet->port_state[i].c_port_state = pc_OutBuf[postion++];
		if (postion > i_OutBufLen){
			i_Ret = DF_ERR_PROTOCOL_ERROR;
			goto ERR;
		}
		cabinet->port_state[i].ReadCAName = pc_OutBuf[postion++];
		if (postion > i_OutBufLen){
			i_Ret = DF_ERR_PROTOCOL_ERROR;
			goto ERR;
		}
		memcpy(cabinet->port_state[i].CAName, pc_OutBuf + postion, 150);
		postion += 150;
		if (postion > i_OutBufLen){
			i_Ret = DF_ERR_PROTOCOL_ERROR;
			goto ERR;
		}
	}
ERR:
	if (pc_OutBuf)free(pc_OutBuf);
	return i_Ret;
}


/*��ȡ�ն����ж˿���USB�豸�����б�*/
int ProtocolGetTerminalAllUSBInfoList(char* ip, int port, struct CabinetInfo *cabinet)
{
	unsigned char in_buf[1024] = { 0 }, *pc_OutBuf = NULL;
	int i_OutBufLen = 0, i_Ret = 0, postion = 0, i_PortNum = 0;
	int err_2;

    //WriteLogFile("lbc 44444");
	if ((i_Ret = run_protocol_2(ip, port, 0x0214, in_buf, 0, &pc_OutBuf, &i_OutBufLen, &err_2)) < 0){
		WriteLogFile("��ȡ�ն����ж˿���USB�豸�����б�ʧ��[0x0214]");
		goto ERR;
	}
	if (err_2 != 0){
		i_Ret = ErrNumberChange(err_2);
		goto ERR;
	}
	if (postion > i_OutBufLen){
		i_Ret = DF_ERR_PROTOCOL_ERROR;
		goto ERR;
	}

	i_PortNum = pc_OutBuf[postion++];//�˿�����
	cabinet->i_USBSumNumber = i_PortNum;
	memcpy(cabinet->ca_TerminalID, pc_OutBuf, 12);//�������
	postion += 12;
	if (postion > i_OutBufLen){
		i_Ret = DF_ERR_PROTOCOL_ERROR;
		goto ERR;
	}
	for (int i = 0; i < i_PortNum; i++){
		postion++;//�˿ں��Զ�����
		if (postion > i_OutBufLen){
			i_Ret = DF_ERR_PROTOCOL_ERROR;
			goto ERR;
		}
		cabinet->port_state[i].USB_Type = pc_OutBuf[postion++];//USB����
		if (postion > i_OutBufLen){
			i_Ret = DF_ERR_PROTOCOL_ERROR;
			goto ERR;
		}
		memcpy(cabinet->port_state[i].VID, pc_OutBuf + postion, 2);
		postion += 2;
		if (postion > i_OutBufLen){
			i_Ret = DF_ERR_PROTOCOL_ERROR;
			goto ERR;
		}
		memcpy(cabinet->port_state[i].PID, pc_OutBuf + postion, 2);
		postion += 2;
		if (postion > i_OutBufLen){
			i_Ret = DF_ERR_PROTOCOL_ERROR;
			goto ERR;
		}
		cabinet->port_state[i].supportCA = pc_OutBuf[postion++];//�Ƿ��ȡ��˰��ʶ��š���˰��
		if (postion > i_OutBufLen){
			i_Ret = DF_ERR_PROTOCOL_ERROR;
			goto ERR;
		}
		memcpy(cabinet->port_state[i].CAName, pc_OutBuf + postion, 110);
		postion += 110;
		if (postion > i_OutBufLen){
			i_Ret = DF_ERR_PROTOCOL_ERROR;
			goto ERR;
		}
		memcpy(cabinet->port_state[i].tax_type, pc_OutBuf + postion, 2);//������
		postion += 2;
		if (postion > i_OutBufLen){
			i_Ret = DF_ERR_PROTOCOL_ERROR;
			goto ERR;
		}
		memcpy(cabinet->port_state[i].tax_serial, pc_OutBuf + postion, 16);//˰�̱��
		postion += 16;
		if (postion > i_OutBufLen){
			i_Ret = DF_ERR_PROTOCOL_ERROR;
			goto ERR;
		}
		memcpy(cabinet->port_state[i].tax_extension, pc_OutBuf + postion, 2);//���̺�
		postion += 2;
		if (postion > i_OutBufLen){
			i_Ret = DF_ERR_PROTOCOL_ERROR;
			goto ERR;
		}
		memcpy(cabinet->port_state[i].tax_number, pc_OutBuf + postion, 20);//��˰��ʶ���
		postion += 20;
		if (postion > i_OutBufLen){
			i_Ret = DF_ERR_PROTOCOL_ERROR;
			goto ERR;
		}
	}
ERR:
	if (pc_OutBuf)free(pc_OutBuf);
	return i_Ret;
}


/****************************************************/
//����VPN����
int ProtocolSetVPN(char * ip, int port, char *in_buf, int in_size)
{
	unsigned char  *pc_OutBuf = NULL;
	int i_OutBufLen = 0, i_Ret = 0, postion = 0, i_PortNum = 0;
	int err_2;


	if ((i_Ret = run_protocol_2(ip, port, 0x0109, in_buf, in_size, &pc_OutBuf, &i_OutBufLen, &err_2)) < 0){
		goto ERR;
	}
	if (err_2 != 0){
		i_Ret = ErrNumberChange(err_2);
		goto ERR;
	}
ERR:
	if (pc_OutBuf)free(pc_OutBuf);
	return i_Ret;
}


int ProtocolSetAutoUpgade(char * ip, int port, char * in_buf, int in_size)
{
	unsigned char *pc_OutBuf = NULL;
	int i_OutBufLen = 0, i_Ret = 0, postion = 0, i_PortNum = 0;
	int err_2;

	if ((i_Ret = run_protocol_2(ip, port, 0x0110, in_buf, in_size, &pc_OutBuf, &i_OutBufLen, &err_2)) < 0){
		goto ERR;
	}
	if (err_2 != 0){
		i_Ret = ErrNumberChange(err_2);
		goto ERR;
	}
ERR:
	if (pc_OutBuf)free(pc_OutBuf);
	return i_Ret;
}



//��ȡ����VPN����

int ProtocolGetVPN(char * ip, int port, ST_ClientVPNSetting *vpn)
{
	unsigned char in_buf[1024] = { 0 }, *pc_OutBuf = NULL;
	int i_OutBufLen = 0, i_Ret = 0, postion = 0;
	int err_2;


	if ((i_Ret = run_protocol_2(ip, port, 0x0215, in_buf, 0, &pc_OutBuf, &i_OutBufLen, &err_2)) < 0){
		WriteLogFile("��ȡ����VPN����ʧ��[0x0215]");
		goto ERR;
	}
	if (err_2 != 0){
		i_Ret = ErrNumberChange(err_2);
		goto ERR;
	}

	if (i_OutBufLen != 156){
		WriteLogFile("[����ͨѶ]��ȡ����VPN����Э�����");
		i_Ret = DF_ERR_PROTOCOL_ERROR;
		goto ERR;
	}

	vpn->open = pc_OutBuf[postion++];
	vpn->login_type = pc_OutBuf[postion++];
	memcpy(vpn->VPNServerIP, pc_OutBuf + postion, 50); postion += 50;
	memcpy(vpn->VPNServerPort, pc_OutBuf + postion, 2); postion += 2;
	memcpy(vpn->account, pc_OutBuf + postion, 20); postion += 20;
	memcpy(vpn->passwd, pc_OutBuf + postion, 20); postion += 20;
	memcpy(vpn->FTPPort, pc_OutBuf + postion, 2); postion += 2;
	memcpy(vpn->FTPUsername, pc_OutBuf + postion, 20); postion += 20;
	memcpy(vpn->FTPPasswd, pc_OutBuf + postion, 20); postion += 20;

ERR:
	if (pc_OutBuf)free(pc_OutBuf);
	return i_Ret;
}



int ProtocolGetAutoUpgrade(char * ip, int port, unsigned char* in, int in_size)
{
	unsigned char in_buf[1024] = { 0 }, *pc_OutBuf = NULL;
	int i_OutBufLen = 0, i_Ret = 0, postion = 0;
	int err_2;

	if ((i_Ret = run_protocol_2(ip, port, 0x0216, in_buf, 0, &pc_OutBuf, &i_OutBufLen, &err_2)) < 0){
		WriteLogFile("��ѯ�����Զ���������[0x0216]");
		goto ERR;
	}
	if (err_2 != 0){
		i_Ret = ErrNumberChange(err_2);
		goto ERR;
	}
	if (i_OutBufLen != 1){
		WriteLogFile("[����ͨѶ]��ѯ�����Զ���������Э�����");
		i_Ret = DF_ERR_PROTOCOL_ERROR;
		goto ERR;
	}
	*in = pc_OutBuf[0];
ERR:
	if (pc_OutBuf)free(pc_OutBuf);
	return i_Ret;
}

//��ȡ��Ʊ����
int ProtocolGetInvoiceNumByMouth(char* ip, int port, int usbport,  char *date ,int *p_InvocieNum)
{
	unsigned char in_buf[1024] = { 0 }, *pc_OutBuf = NULL;
	int i_OutBufLen = 0, i_Ret = 0, postion = 0;

	int err_2 = 0;

	in_buf[postion++] = (usbport >> 24) & 0xff;
	in_buf[postion++] = (usbport >> 16) & 0xff;
	in_buf[postion++] = (usbport >> 8) & 0xff;
	in_buf[postion++] = (usbport)& 0xff;
	memcpy(in_buf + postion, date, 6);	postion += 6;


	if ((i_Ret = run_protocol_2(ip, port, 0x0217, in_buf, postion, &pc_OutBuf, &i_OutBufLen, &err_2)) < 0){
		WriteLogFile("��ѯ��ȡ��Ʊ����ʧ��[0x0217]");
		goto ERR;
	}
	if (err_2 != 0){
		i_Ret = ErrNumberChange(err_2);
		goto ERR;
	}
	if (i_OutBufLen != 8){
		WriteLogFile("[����ͨѶ]��ѯ��ȡ��Ʊ����Э�����");
		i_Ret = DF_ERR_PROTOCOL_ERROR;
		goto ERR;
	}
	//��Ʊ����
	*p_InvocieNum = (int)(pc_OutBuf[0] << 24) + (int)(pc_OutBuf[1] << 16) + (int)(pc_OutBuf[2] << 8) + (int)pc_OutBuf[3];
ERR:
	if (pc_OutBuf)free(pc_OutBuf);
	return i_Ret;
}

//��ȡ��Ʊ����
int ProtocolGetInvoiceSum(char* ip, int port, int usbport, char *date, int* invoice_num)
{

	unsigned char in_buf[1024] = { 0 }, *pc_OutBuf = NULL;
	int i_OutBufLen = 0, i_Ret = 0, postion = 0;

	int err_2 = 0 ;

	in_buf[postion++] = (usbport >> 24) & 0xff;
	in_buf[postion++] = (usbport >> 16) & 0xff;
	in_buf[postion++] = (usbport >> 8) & 0xff;
	in_buf[postion++] = (usbport)& 0xff;
	memcpy(in_buf + postion, date, 6);	postion += 6;

	if ((i_Ret = run_protocol_2(ip, port, 0x0217, in_buf, postion, &pc_OutBuf, &i_OutBufLen, &err_2)) < 0){
		WriteLogFile("��ѯ��ȡ��Ʊ����ʧ��[0x0217]");
		goto ERR;
	}
	if (err_2 != 0){
		i_Ret = ErrNumberChange(err_2);
		goto ERR;
	}
	if (i_OutBufLen != 8){
		WriteLogFile("[����ͨѶ]��ѯ��ȡ��Ʊ����Э�����");
		i_Ret = DF_ERR_PROTOCOL_ERROR;
		goto ERR;
	}
	*invoice_num = (int)(pc_OutBuf[0] << 24) + (int)(pc_OutBuf[1] << 16) + (int)(pc_OutBuf[2] << 8) + (int)pc_OutBuf[3];
ERR:
	if (pc_OutBuf)free(pc_OutBuf);
	return i_Ret;
}



//�����·ݻ�ȡ��˰�̷�Ʊ��Ϣ  
int ProtocolGetTaxInfoByMouth(char* ip, int port, int usbport, char *date, char* dir, struct InvoiceDataHead* p_InvoiceHead)
{
	/*unsigned char in_buf[1024] = { 0 };
	int i_OutBufLen = 0, i_Ret = 0, postion = 0;
	int  err_2 = 0;

	in_buf[postion++] = (usbport >> 24) & 0xff;
	in_buf[postion++] = (usbport >> 16) & 0xff;
	in_buf[postion++] = (usbport >> 8) & 0xff;
	in_buf[postion++] = (usbport)& 0xff;
	memcpy(in_buf + postion, date, 6);	postion += 6;

	in_buf[13] = 1;
	in_buf[14] = ((p_InvoiceHead->InvoiceSumNum) >> 24) & 0xff;
	in_buf[15] = ((p_InvoiceHead->InvoiceSumNum) >> 16) & 0xff;
	in_buf[16] = ((p_InvoiceHead->InvoiceSumNum) >> 8) & 0xff;
	in_buf[17] = (p_InvoiceHead->InvoiceSumNum) & 0xff;
	postion += 8;
		
	if ((i_Ret = run_protocol_0223(ip, port, in_buf, 18, dir, p_InvoiceHead, &err_2)) < 0){
		p_InvoiceHead->InvoiceSumNum = 0;
		goto ERR;
	}

	if (err_2 != 0){
		i_Ret = ErrNumberChange(err_2);
		p_InvoiceHead->InvoiceSumNum = 0;
		goto ERR;
	}
ERR:
	return i_Ret;*/

	return 0;
}


//������ʽ�����·ݻ�ȡ��˰�̷�Ʊ��Ϣ  
int ProtocolGetTaxInfoByMouthStatic(char* ip, int port, int usbport, char *date, char* dir, struct InvoiceDataHead* p_InvoiceHead)
{
//	unsigned char in_buf[1024] = { 0 }, *pc_OutBuf = NULL;
//	int i_OutBufLen = 0, i_Ret = 0, postion = 0;
//	int  err_2 = 0;
//
//	in_buf[postion++] = (usbport >> 24) & 0xff;
//	in_buf[postion++] = (usbport >> 16) & 0xff;
//	in_buf[postion++] = (usbport >> 8) & 0xff;
//	in_buf[postion++] = (usbport)& 0xff;
//	memcpy(in_buf + postion, date, 6);	postion += 6;
//
//	in_buf[13] = 1;
//	in_buf[14] = ((p_InvoiceHead->InvoiceSumNum) >> 24) & 0xff;
//	in_buf[15] = ((p_InvoiceHead->InvoiceSumNum) >> 16) & 0xff;
//	in_buf[16] = ((p_InvoiceHead->InvoiceSumNum) >> 8) & 0xff;
//	in_buf[17] = (p_InvoiceHead->InvoiceSumNum) & 0xff;
//	postion += 8;
//
//	if ((i_Ret = run_protocol_0223(ip, port, in_buf, 18, dir, p_InvoiceHead, &err_2)) < 0){
//		p_InvoiceHead->InvoiceSumNum = 0;
//		goto ERR;
//	}
//
//	/*if ((i_Ret = run_protocol_0223_static(ip, port, in_buf, 18, dir, p_InvoiceHead, &err_2)) < 0){
//		p_InvoiceHead->InvoiceSumNum = 0;
//		goto ERR;
//	}*/
//	if (err_2 != 0){
//		i_Ret = ErrNumberChange(err_2);
//		p_InvoiceHead->InvoiceSumNum = 0;
//		goto ERR;
//	}
//ERR:
//	if (pc_OutBuf)free(pc_OutBuf);
//	return i_Ret;

	return 0;
}


//��ѯ��˰�̷�Ʊ����
int ProtocolGetTaxType(char* ip, int port, int usbport, char* result_buf)
{
	unsigned char in_buf[1024] = { 0 }, *pc_OutBuf = NULL;
	int i_OutBufLen = 0, i_Ret = 0, postion = 0;
	int err_2 = 0;

	in_buf[postion++] = (usbport >> 24) & 0xff;
	in_buf[postion++] = (usbport >> 16) & 0xff;
	in_buf[postion++] = (usbport >> 8) & 0xff;
	in_buf[postion++] = (usbport)& 0xff;


	if ((i_Ret = run_protocol_2(ip, port, 0x0219, in_buf, postion, &pc_OutBuf, &i_OutBufLen, &err_2)) < 0){
		WriteLogFile("��ѯ��˰�̷�Ʊ����ʧ��[0x0219]");
		goto ERR;
	}
	if (err_2 != 0){
		i_Ret = ErrNumberChange(err_2);
		goto ERR;
	}
	if (i_OutBufLen != 8){
		WriteLogFile("[����ͨѶ]��ѯ��˰�̷�Ʊ����Э�����");
		i_Ret = DF_ERR_PROTOCOL_ERROR;
		goto ERR;
	}

	for (int i = 0; i < 8; i++){
		switch (pc_OutBuf[i])
		{
		case 0:
			pc_OutBuf[i] = 0;
			break;
		case 2:
			pc_OutBuf[i] = 1;
			break;
		case 11:
			pc_OutBuf[i] = 2;
			break;
		case 12:
			pc_OutBuf[i] = 3;
			break;
		case 41:
			pc_OutBuf[i] = 4;
			break;
		case 42:
			pc_OutBuf[i] = 5;
			break;
		case 51:
			pc_OutBuf[i] = 6;
			break;
		default:
			pc_OutBuf[i] = 0xff;
		}
	}

	memcpy(result_buf, pc_OutBuf, 8);

ERR:
	if (pc_OutBuf)free(pc_OutBuf);
	return i_Ret;
}
//��ѯ��˰�̷�Ʊ�����Ϣ
int ProtocolGetTaxGuardInfo(char* ip, int port, int usbport, char tax_type, struct InvoiceGuardInfo* guard_info)
{
//	unsigned char in_buf[1024] = { 0 }, *pc_OutBuf = NULL;
//	int i_OutBufLen = 0, i_Ret = 0, postion = 0;
//	int err_2 = 0;
//
//	in_buf[postion++] = (usbport >> 24) & 0xff;
//	in_buf[postion++] = (usbport >> 16) & 0xff;
//	in_buf[postion++] = (usbport >> 8) & 0xff;
//	in_buf[postion++] = (usbport)& 0xff;
//	in_buf[postion++] = tax_type;
//
//	if ((i_Ret = run_protocol_2(ip, port, 0x0220, in_buf, postion, &pc_OutBuf, &i_OutBufLen, &err_2)) < 0){
//		WriteLogFile("��ѯ��˰�̷�Ʊ�����Ϣʧ��[0x0220]");
//		goto ERR;
//	}
//	if (err_2 != 0){
//		i_Ret = ErrNumberChange(err_2);
//		goto ERR;
//	}
//	if (i_OutBufLen != 107){
//		WriteLogFile("[����ͨѶ]��ѯ��˰�̷�Ʊ�����ϢЭ�����");
//		i_Ret = DF_ERR_PROTOCOL_ERROR;
//		goto ERR;
//	}
//	//��������
//	postion = 0;
//	guard_info->MainTicket = pc_OutBuf[postion++];
//	guard_info->SepartTickets = pc_OutBuf[postion++];
//	guard_info->TicketNum = pc_OutBuf[postion++];
//	guard_info->MainProgramVersion = pc_OutBuf[postion++];
//	guard_info->ChildProgramVersion = pc_OutBuf[postion++];
//	guard_info->BSQ = pc_OutBuf[postion++];
//	guard_info->SSQ = pc_OutBuf[postion++];
//
//	memcpy(guard_info->LastBSQ, pc_OutBuf + postion, 20); postion += 20;
//	memcpy(guard_info->NextBSQ, pc_OutBuf + postion, 20); postion += 20;
//	memcpy(guard_info->LockDate, pc_OutBuf + postion, 20); postion += 20;
//	memcpy(guard_info->Limite, pc_OutBuf + postion, 20); postion += 20;
//	memcpy(guard_info->OffLimite, pc_OutBuf + postion, 20); postion += 20;
//
//ERR:
//	if (pc_OutBuf)free(pc_OutBuf);
//	return i_Ret;

	return 0;
}


int ProtocolGetInvoiceSectionInfo(char* ip, int port, int usbport, char tax_type, struct AllInvoiceSectionInfo * invoices)
{
//	unsigned char in_buf[1024] = { 0 }, *pc_OutBuf = NULL;
//	int i_OutBufLen = 0, i_Ret = 0, postion = 0;
//	int err_2 = 0;
//	char* cp_LogStr = NULL;
//
//	in_buf[postion++] = (usbport >> 24) & 0xff;
//	in_buf[postion++] = (usbport >> 16) & 0xff;
//	in_buf[postion++] = (usbport >> 8) & 0xff;
//	in_buf[postion++] = (usbport)& 0xff;
//	in_buf[postion++] = tax_type;
//
//	if ((i_Ret = run_protocol_2(ip, port, 0x0221, in_buf, postion, &pc_OutBuf, &i_OutBufLen, &err_2)) < 0){
//		WriteLogFile("��ѯ˰�̷�Ʊ����Ϣ[0x0221] ʧ��");
//		goto ERR;
//	}
//	if (err_2 != 0){
//		i_Ret = ErrNumberChange(err_2);
//		goto ERR;
//	}
//
//	if (i_LogLevel){
//		cp_LogStr = (char*)malloc(i_OutBufLen * 10);
//		if (cp_LogStr != NULL){
//			hex2str(pc_OutBuf, i_OutBufLen, cp_LogStr);
//			WriteLogFile("%s", cp_LogStr);
//			free(cp_LogStr);
//		}
//		else{
//			WriteLogFile("malloc����ʧ��");
//		}
//	}
//
//	if (i_OutBufLen < 1){
//		i_Ret = DF_ERR_PROTOCOL_ERROR;
//		goto ERR;
//	}
//
//	//��������
//	postion = 0;
//	invoices->section_all_num = pc_OutBuf[postion++];
//	if (postion > i_OutBufLen){
//		i_Ret = DF_ERR_PROTOCOL_ERROR;
//		goto ERR;
//	}
//	if (i_LogLevel){
//		WriteLogFile("�ն˽��յ���Ʊ������= %d", invoices->section_all_num);
//	}
//	for (int i = 0; i < pc_OutBuf[0]; i++){
//
//		invoices->InvoiceSectionInfo[i].serial_num = pc_OutBuf[postion++];
//		if (postion > i_OutBufLen){
//			i_Ret = DF_ERR_PROTOCOL_ERROR;
//			goto ERR;
//		}
//		memcpy(invoices->InvoiceSectionInfo[i].time, pc_OutBuf + postion, 20); postion += 20;
//		if (postion > i_OutBufLen){
//			i_Ret = DF_ERR_PROTOCOL_ERROR;
//			goto ERR;
//		}
//		memcpy(invoices->InvoiceSectionInfo[i].invoice_num, pc_OutBuf + postion, 4); postion += 4;
//		if (postion > i_OutBufLen){
//			i_Ret = DF_ERR_PROTOCOL_ERROR;
//			goto ERR;
//		}
//		memcpy(invoices->InvoiceSectionInfo[i].origin_num, pc_OutBuf + postion, 4); postion += 4;
//		if (postion > i_OutBufLen){
//			i_Ret = DF_ERR_PROTOCOL_ERROR;
//			goto ERR;
//		}
//		memcpy(invoices->InvoiceSectionInfo[i].remianing_num, pc_OutBuf + postion, 4); postion += 4;
//		if (postion > i_OutBufLen){
//			i_Ret = DF_ERR_PROTOCOL_ERROR;
//			goto ERR;
//		}
//	}
//
//ERR:
//	if (pc_OutBuf)free(pc_OutBuf);
//	return i_Ret;

	return 0;

}


int ProtocolGetAreacode(char* ip, int port, int usbport, char* areacode)
{
	unsigned char in_buf[1024] = { 0 }, *pc_OutBuf = NULL;
	int i_OutBufLen = 0,i_Ret = 0;
	int postion = 0, err_2 = 0;

	in_buf[postion++] = (usbport >> 24) & 0xff;
	in_buf[postion++] = (usbport >> 16) & 0xff;
	in_buf[postion++] = (usbport >> 8) & 0xff;
	in_buf[postion++] = (usbport)& 0xff;

	if ((i_Ret = run_protocol_2(ip, port, 0x0224, in_buf, postion, &pc_OutBuf, &i_OutBufLen, &err_2)) < 0){
		WriteLogFile("��ѯ��˰���������ʧ��");
		goto ERR;
	}
	
	if (err_2 != 0){
		i_Ret = ErrNumberChange(err_2);
		goto ERR;
	}
	if (i_OutBufLen != 20){
		i_Ret = DF_ERR_PROTOCOL_ERROR;
		goto ERR;
	}

	memcpy(areacode, pc_OutBuf, 20);

	
ERR:
	if (pc_OutBuf)free(pc_OutBuf);
	return i_Ret;
}



/*********************************************************************
-   Function : ProtocolGetAllTaxPlateInfoUnify
-   Description����ȡĳ���ն����е�˰����Ϣ(��˰�̡�˰����)
-   Calls��
-   Calles By:
-   Table Accessed :
-   Table Update :
-   Input  :
-   Output :
-   Return :
-   Other :
***********************************************************************/
int ProtocolGetAllTaxPlateInfoUnify(char* ip, int port, struct st_TaxPlateInfoUnify * pst_TaxPlateInfo)
{
	int i = 0, postion = 0, out_size = 0, ret = 0, err_2 = 0;

	if ((ret = run_protocol_2(ip, port, 0x0225, NULL, postion, &(pst_TaxPlateInfo->cp_TaxPlateInfoBeforeUnZlib), &(pst_TaxPlateInfo->i_LenBeforeUnZlib), &err_2)) < 0){
		return ret;
	}

	//���񷵻صĴ������ת��
	if (err_2 != 0)
		return ErrNumberChange(err_2);
	return 0;
}


//��Ʊ���ݲ��뵽����
static BOOL InsertInvoiceToList(struct InvoiceDataHead* p_InvoiceDataHead, struct InvoiceDataNode* p_InvoiceData)
{

	//if (p_InvoiceDataHead == NULL || p_InvoiceData == NULL){
	//	WriteLogFile("p_InvoiceDataHead����p_InvoiceData Ϊ��");
	//	return FALSE;
	//}
	//else{
	//	if (p_InvoiceDataHead->next == NULL)
	//		p_InvoiceDataHead->next = p_InvoiceData;
	//	else{
	//		struct InvoiceDataNode* p_InvoiceDataTmp = p_InvoiceDataHead->next;

	//		while (p_InvoiceDataTmp != NULL){
	//			if (p_InvoiceDataTmp->next == NULL){
	//				p_InvoiceDataTmp->next = p_InvoiceData;
	//				break;
	//			}
	//			p_InvoiceDataTmp = p_InvoiceDataTmp->next;
	//		}

	//		//while (p_InvoiceDataTmp->next != NULL)
	//		//	p_InvoiceDataTmp = p_InvoiceDataTmp->next;
	//		//p_InvoiceDataTmp->next = p_InvoiceData;
	//	}
	//	
	//}
	return TRUE;

}
int i_InvoiceBinIndex = 0;
/********************************************************/
BOOL PackageBinDate(struct InvoiceDataHead* p_InvoiceDataHead, uint8 * before_unzlib_bin_data, int before_unzlib_bin_data_len)
{
	//char ca_FileDir[MAX_PATH] = {0};
	//unsigned char* p_after_zlib = calloc(1 , before_unzlib_bin_data_len* UNZLIB);
	//unsigned int postion = 0, i = 0, invoice_sum = 0, i_after_unzlib_buf_len = before_unzlib_bin_data_len * UNZLIB;
	//if (p_after_zlib == NULL)
	//	return DF_SYSTEM_ERROR;

	//uncompress(p_after_zlib, &i_after_unzlib_buf_len, before_unzlib_bin_data, before_unzlib_bin_data_len);

	//WriteLogFile("��Ʊ����%d", p_after_zlib[3]);
	//invoice_sum = ((((unsigned int)p_after_zlib[postion]) << 24) + (((unsigned int)p_after_zlib[postion + 1]) << 16) + (((unsigned int)p_after_zlib[postion + 2]) << 8) + p_after_zlib[postion + 3]); postion += 4;
	//if (invoice_sum < 0){
	//	WriteLogFile("��Ʊ����=%d" , invoice_sum);
	//	return DF_ERR_PROTOCOL_ERROR;
	//}

	//p_InvoiceDataHead->RecvedInvoiceNum += invoice_sum;
	//for (i = 0; i < invoice_sum; i++){
	//	//��Ʊ��ϸ����
	//	struct InvoiceDataNode* p_invoice = (struct InvoiceDataNode*)malloc(sizeof(struct InvoiceDataNode));
	//	if (p_invoice == NULL)
	//		return DF_SYSTEM_ERROR;
	//	memset(p_invoice, 0, sizeof(struct InvoiceDataNode));


	//	//������Ʊ���
	//	int serial_num = ((((unsigned int)p_after_zlib[postion]) << 24) + (((unsigned int)p_after_zlib[postion + 1]) << 16) + (((unsigned int)p_after_zlib[postion + 2]) << 8) + p_after_zlib[postion + 3]);
	//	postion += 4;
	//	memcpy(p_invoice->ca_FPDM, p_after_zlib + postion, 20); postion += 20;
	//	memcpy(p_invoice->ca_FPHM, p_after_zlib + postion, 20); postion += 20;
	//	//������Ʊ����
	//	int invoice_len = ((((unsigned int)p_after_zlib[postion]) << 24) + (((unsigned int)p_after_zlib[postion + 1]) << 16) + (((unsigned int)p_after_zlib[postion + 2]) << 8) + p_after_zlib[postion + 3]);

	//	if (invoice_len < 0){
	//		WriteLogFile("��Ʊ���ݳ���= %d" , invoice_len);
	//		return DF_ERR_PROTOCOL_ERROR;
	//	}

	//	postion += 4;
	//	

	//	p_invoice->pc_JsonInvoice = malloc(invoice_len + 100);
	//	if (p_invoice->pc_JsonInvoice == NULL){
	//		if (p_after_zlib != NULL){
	//			free(p_after_zlib);
	//			p_after_zlib = NULL;
	//		}
	//		return DF_SYSTEM_ERROR;
	//	}
	//	memset(p_invoice->pc_JsonInvoice, 0, invoice_len + 100);
	//	memcpy(p_invoice->pc_JsonInvoice, p_after_zlib + postion, invoice_len);
	//	p_invoice->i_SerialNum = serial_num;
	//	p_invoice->i_InvoiceLength = invoice_len;

	//	if (InsertInvoiceToList(p_InvoiceDataHead, p_invoice) == FALSE){
	//		if (p_after_zlib != NULL){
	//			free(p_after_zlib);
	//			p_after_zlib = NULL;
	//		}
	//		return DF_SYSTEM_ERROR;
	//	}
	//	postion += invoice_len;
	//}

	//if (p_after_zlib != NULL){
	//	free(p_after_zlib);
	//	p_after_zlib = NULL;
	//}
	return 0;
}

/*���յ��ķ�Ʊ���ݱ��浽�ļ���*/
void SaveInvoiceBinDataToFile(struct InvoiceDataHead* out_data , char* dir)
{
	/*struct InvoiceDataNode* invoice_node = out_data->next;
	
	while (invoice_node != NULL){
		SaveInvoiceToFile_V2(dir, invoice_node->pc_JsonInvoice, invoice_node->ca_FPDM, invoice_node->ca_FPHM);

		invoice_node = invoice_node->next;
	}*/
}


//�̷߳�ʽ��ȡ��Ʊ��ϸ������һ���ӿ�
static int run_protocol_0223(char *ip, int port, char *in_buf, int in_size, char* dir, struct InvoiceDataHead* out_data, int *err)
{
	int len = 0, postion = 0,ret = 0;
	unsigned int cmd = 0;
	SOCKET sockfd = 0;
	BOOL flag = TRUE;//�ϵ���ܷ�Ʊ��Ϣ,���һ�ν��ܱ��λ
	unsigned char buf[200] = { 0 }, *recv_data_buf = NULL;//
#ifdef NINGBO_BANK
	char remote_ip[100] = { 0 };
	int remote_port;
	for (int i = 0; i < all_terminal_setting_table.count; i++){

		//WriteLogFile("�����е�ǰip:%s\n", all_terminal_setting_table.terminal_setting[i].terminal_ip);
		if ((memcmp(all_terminal_setting_table.terminal_setting[i].terminal_ip, ip, strlen(ip)) == 0) && (port == all_terminal_setting_table.terminal_setting[i].terminal_port))
		{
			//WriteLogFile("run_protocol_0223�������ҵ���Ӧip:%s\n", ip);
			memcpy(remote_ip, all_terminal_setting_table.terminal_setting[i].remote_ip, strlen(all_terminal_setting_table.terminal_setting[i].remote_ip));
			remote_port = all_terminal_setting_table.terminal_setting[i].remote_port;
			break;
		}
	}
	if (strlen(remote_ip) == 0)
	{
		WriteLogFile("δ���������ҵ���Ӧip��%s", ip);
		return DF_ERR_SOCKET_ERROR;
	}
#endif
	sockfd = socket_connect(ip, port, ip, port, 10000);
	if (sockfd == SOCKET_ERROR){
		socket_close(sockfd);
		return DF_ERR_SOCKET_ERROR;
	}

	buf[0] = 0x01;
	buf[1] = 0x10;
	len = in_size + 6;
	buf[2] = ((len >> 8) & 0xFF);
	buf[3] = (len & 0xFF);
	buf[4] = 0x02;
	buf[5] = 0x23;
	if (in_size != 0){
		memcpy(buf + 6, in_buf, in_size);
	}

	if ((socket_send(sockfd, buf, len)) != len){
		socket_close(sockfd);
		return DF_ERR_SOCKET_ERROR;
	}

	while (flag){
		memset(buf, 0, sizeof(buf));
	
protocol_handle://��ʱ�ȴ�
		postion = 0;
		if ((socket_recv(sockfd, buf, 6)) != 6){
			socket_close(sockfd);
			return DF_ERR_SOCKET_ERROR;
		}

		if ((buf[0] != 0x01) || (buf[1] != 0x10)){
			socket_close(sockfd);
			return DF_ERR_PROTOCOL_ERROR;
		}

		cmd = (((unsigned int)buf[4] << 8) + buf[5]);
		switch (cmd){
		case 0xFFFE:
			goto protocol_handle;

		case 0xFFFD://�ϵ���շ�Ʊ��ϸ
			len = (((unsigned int)buf[2] << 8) + (unsigned int)buf[3]);
			if (len == 0){
				if ((socket_recv(sockfd, buf + 6, 4)) != 4){
					socket_close(sockfd);
					return DF_ERR_SOCKET_ERROR;
				}
				len = (((unsigned int)buf[6] << 24) + ((unsigned int)buf[7] << 16) + ((unsigned int)buf[8] << 8) + (unsigned int)buf[9]);
				if (len < 0){
					socket_close(sockfd);
					return DF_ERR_PROTOCOL_ERROR;
				}
				postion = 10;
			}
			else{
				postion = 6;
			}
			recv_data_buf = malloc(len + 100);
			if (recv_data_buf == NULL){
				socket_close(sockfd);
				return DF_SYSTEM_ERROR;
			}
			else{
				memset(recv_data_buf, 0, len + 100);
				memcpy(recv_data_buf, buf, postion);
			}
			//��ȡ��Ʊ��������
			if ((socket_recv(sockfd, recv_data_buf + postion, len - postion)) != (len - postion)){
				if (recv_data_buf != NULL){
					free(recv_data_buf);
					recv_data_buf = NULL;
				}
				socket_close(sockfd);
				return DF_ERR_SOCKET_ERROR;
			}
			if ((ret = PackageBinDate(out_data, recv_data_buf + postion, len)) != 0){
				if (recv_data_buf != NULL){
					free(recv_data_buf);
					recv_data_buf = NULL;
				}
				socket_close(sockfd);
				return ret;
			}
			if (dir != NULL){
				SaveInvoiceBinDataToFile(out_data, dir);
			}
			if (recv_data_buf != NULL){
				free(recv_data_buf);
				recv_data_buf = NULL;
			}
			break;
		case 0x1223://
			len = (((unsigned int)buf[2] << 8) + (unsigned int)buf[3]);
			if (len == 0){
				if ((socket_recv(sockfd, buf + 6, 4)) != 4){
					socket_close(sockfd);
					return DF_ERR_SOCKET_ERROR;
				}
				len = (((unsigned int)buf[6] << 24) + ((unsigned int)buf[7] << 16) + ((unsigned int)buf[8] << 8) + (unsigned int)buf[9]);
				postion = 10;
			}
			else{
				postion = 6;
			}
			recv_data_buf = malloc(len + 100);
			if (recv_data_buf == NULL){
				socket_close(sockfd);
				return DF_SYSTEM_ERROR;
			}
			else{
				memset(recv_data_buf, 0, len + 100);
				memcpy(recv_data_buf, buf, postion);
			}
			//��ȡ��Ʊ��������
			if (socket_recv(sockfd, recv_data_buf + postion, len - postion) != (len - postion)){
				if (recv_data_buf != NULL){
					free(recv_data_buf);
					recv_data_buf = NULL;
				}
				socket_close(sockfd);
				return DF_ERR_SOCKET_ERROR;
			}

			//���յ���ÿһ�ŷ�Ʊ�浽������
			if ((ret = PackageBinDate(out_data, recv_data_buf + postion, len)) != 0){
				if (recv_data_buf != NULL){
					free(recv_data_buf);
					recv_data_buf = NULL;
				}
				socket_close(sockfd);
				return ret;
			}

			if (dir != NULL){
				SaveInvoiceBinDataToFile(out_data , dir);
			}

			if (recv_data_buf != NULL){
				free(recv_data_buf);
				recv_data_buf = NULL;
			}
			flag = FALSE;
			socket_close(sockfd);
			break;
		default:
			socket_close(sockfd);
			flag = FALSE;
			break;
		}
	}

	return 0;
}

/*********************************************************************
-   Function : run_portocol_2
-   Description���ͻ���Э��ͨѶ�ӿ�(�ڶ���)
-   Calls��
-   Calles By:
-   Table Accessed :
-   Table Update :
-   Input  :
-   Output :
-   Return :
-   Other :
***********************************************************************/
static int run_protocol_2(char *ip, int port, int cmd, unsigned char *in_buf, int in_size, unsigned char **out_buf, int *out_size, int *err)
{
	int ret = 0, len = 0, recv_len = 0, i_HeadLen = 0;
	SOCKET sockfd = 0;
	unsigned char buf[20000] = { 0 }, ca_RecvHead[100] = { 0 };
	char* cp_LogStr = NULL;

	WriteLogFile("lbc run_protocol_2 cmd:%x\n", cmd);

#ifdef NINGBO_BANK
	char remote_ip[100] = { 0 };
	int remote_port;
	for (int i = 0; i < all_terminal_setting_table.count; i++){

		//WriteLogFile("�����е�ǰip:%s\n", all_terminal_setting_table.terminal_setting[i].terminal_ip);
		if ((memcmp(all_terminal_setting_table.terminal_setting[i].terminal_ip, ip, strlen(ip)) == 0) && (port == all_terminal_setting_table.terminal_setting[i].terminal_port))
		{
			//WriteLogFile("run_protocol_2�������ҵ���Ӧip:%s\n", ip);
			memcpy(remote_ip, all_terminal_setting_table.terminal_setting[i].remote_ip, strlen(all_terminal_setting_table.terminal_setting[i].remote_ip));
			remote_port = all_terminal_setting_table.terminal_setting[i].remote_port;
			break;
		}
	}
	if (strlen(remote_ip) == 0)
	{
		WriteLogFile("δ���������ҵ���Ӧip��%s",ip);
		return DF_ERR_SOCKET_ERROR;
	}
#endif
	sockfd = socket_connect(ip, port, ip, port,10000);
	if (sockfd == SOCKET_ERROR){
		socket_close(sockfd);
		return DF_ERR_SOCKET_ERROR;
	}

	buf[0] = 0x01;
	buf[1] = 0x10;
	len = in_size + 6;
	buf[2] = ((len >> 8) & 0xFF);
	buf[3] = (len & 0xFF);
	buf[4] = ((cmd >> 8) & (0xFF));
	buf[5] = (cmd & 0xFF);
	if (in_size != 0){
		memcpy(buf + 6, in_buf, in_size);
	}

	if (i_LogLevel){
		cp_LogStr = (char*)malloc((in_size + 6) * 10);
		if (cp_LogStr != NULL){
			hex2str(buf, in_size + 6, cp_LogStr);
			WriteLogFile("����[%04x]���͵�ip =[%s] port = [%d] ������%s", cmd, ip, port, cp_LogStr);
			free(cp_LogStr);
		}
		else{
			WriteLogFile("malloc����ʧ��");
		}
	}
	if (socket_send(sockfd, buf, len) != len){
		WriteLogFile("socket_send err1");
		socket_close(sockfd);
		return DF_ERR_SOCKET_ERROR;
	}
protocol_handle:
	//��ȡͷ��6���ֽ�
	memset(ca_RecvHead, 0, sizeof(ca_RecvHead));
	if (socket_recv(sockfd , ca_RecvHead , 6) != 6){
		WriteLogFile("socket_recv err1");
		socket_close(sockfd);
		return DF_ERR_SOCKET_ERROR;
	}
	//�ж�ͷ
	if ((ca_RecvHead[0] != 0x01) || (ca_RecvHead[1] != 0x10)){
		socket_close(sockfd);
		WriteLogFile("ca_RecvHead err1 --%02x %02x %02x %02x %02x %02x", ca_RecvHead[0], ca_RecvHead[1], ca_RecvHead[2], ca_RecvHead[3], ca_RecvHead[4], ca_RecvHead[5]);
		return DF_ERR_PROTOCOL_ERROR;
	}
	//�ж������ֳ�ʱ�ȴ�
	if ((ca_RecvHead[4] == 0xFF) && (ca_RecvHead[5] == 0xFE)){
		goto protocol_handle;
	}
	unsigned int i_Len = (((unsigned int)ca_RecvHead[2] << 8) + (unsigned int)ca_RecvHead[3]);
	//���������ֺ�����
	if ((ca_RecvHead[4] == 0xFF) && (ca_RecvHead[5] == 0xFF)){
		if (i_Len < 6){
			WriteLogFile("��ȡ��������������ȴ���");
			socket_close(sockfd);
			return DF_ERR_PROTOCOL_ERROR;
		}
			
		if (socket_recv(sockfd, ca_RecvHead + 6, i_Len - 6) != (i_Len - 6)){
			socket_close(sockfd);
			WriteLogFile("socket_recv err2");
			return DF_ERR_SOCKET_ERROR;
		}
		if (err != NULL){
			*err = ca_RecvHead[6];
			WriteLogFile("%04x ���񷵻ش������:%d", cmd, *err);
		}

		if ((out_size != NULL) && (out_buf != NULL)){
			*out_buf = (unsigned char*)malloc(i_Len - 7 + 100);
			if (*out_buf == NULL){
				WriteLogFile("��̬����ռ�ϵͳ����");
				socket_close(sockfd);
				return DF_ERR_PROTOCOL_ERROR;
			}
			memset(*out_buf, 0, i_Len - 7 + 100);
			memcpy(*out_buf, ca_RecvHead + 7, i_Len - 7);
			WriteLogFile("�ն˷��صĴ�������:%s", *out_buf);
			*out_size = i_Len - 7;
		}
		socket_close(sockfd);
		return 0;
	}

	//����������
	if (buf[2] == 0x00 && buf[3] == 0x00){
		if (socket_recv(sockfd, ca_RecvHead + 6, 4) != 4){
			socket_close(sockfd);
			return DF_ERR_SOCKET_ERROR;
		}
		i_Len = (((unsigned int)ca_RecvHead[6] << 24) + ((unsigned int)ca_RecvHead[7] << 16) + ((unsigned int)ca_RecvHead[8] << 8) + (unsigned int)ca_RecvHead[9]);
		//WriteLogFile("i_Len = %d",i_Len);
		if (i_Len < 10){
			WriteLogFile("����ͨѶЭ�鳤�ȴ���");
			socket_close(sockfd);
			return DF_ERR_PROTOCOL_ERROR;
		}
		i_HeadLen = 10;
	}
	else{
		if (i_Len == 6){
			socket_close(sockfd);
			return 0;
		}
		else if (i_Len < 6){
			WriteLogFile("����ͨѶЭ�鳤�ȴ���");
			socket_close(sockfd);
			return DF_ERR_PROTOCOL_ERROR;
		}
		i_HeadLen = 6;
	}
	//������������
	*out_buf = (unsigned char*)malloc(i_Len + 100);
	if (*out_buf != NULL){
		memset(*out_buf, 0, i_Len + 100);
	}
	else{
		WriteLogFile("��̬����ռ�ϵͳ����");
		socket_close(sockfd);
		return DF_ERR_PROTOCOL_ERROR;
	}
	WriteLogFile("socket_recv");
	if (socket_recv(sockfd, *out_buf, i_Len - i_HeadLen) != (i_Len - i_HeadLen)){
		WriteLogFile("socket_recv failed");
		if (*out_buf != NULL){
			free(*out_buf);
			*out_buf = NULL;
		}
		socket_close(sockfd);
		return DF_ERR_SOCKET_ERROR;
	}
	if (i_LogLevel){
		cp_LogStr = (char*)malloc((i_Len - i_HeadLen) * 10);
		if (cp_LogStr != NULL){
			hex2str(*out_buf, i_Len - i_HeadLen, cp_LogStr);
			WriteLogFile("����[%04x]����ip =[%s] port = [%d] ������%s", cmd, ip, port, cp_LogStr);
			free(cp_LogStr);
		}
		else{
			WriteLogFile("malloc����ʧ��");
		}
	}

	//WriteLogFile("exit");
	//����Э����ȷ����������
	(*err) = 0;
	if ((out_size != NULL)){
		*out_size = i_Len - i_HeadLen;
	}
	socket_close(sockfd);
	return 0;
}

