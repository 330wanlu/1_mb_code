#define _tax_interface_c
#include "../inc/tax_interface.h"
int EncordingConvert(char *from_charset, char *to_charset, char *inbuf, size_t inlen, char *outbuf,
	size_t outlen);
#define LIBUSB_CHECK(action)                                                                   \
do {                                                                                           \
    int ret = (action);                                                                        \
    if (ret != LIBUSB_SUCCESS) {                                                               \
        return -1;                                                                             \
    }                                                                                          \
    break;                                                                                     \
} while (0)
uint8 g_CmdE1AA[7] = "\xfe\xff\xcc\xbc\x53\xe1\xaa";
uint8 g_CmdE1AB[7] = "\xfe\xff\xcc\xbc\x53\xe1\xab";
uint16 GetDeviceRepCode(uint8 *pAARep)
{
	uint8 *v13 = pAARep;
	int v17;
	int v18;
	LOWORD(v17) = *(_BYTE *)v13;
	LOWORD(v18) = *(_BYTE *)(v13 + 1);
	int v19 = v18 + (v17 << 8);
	return (uint16)v19;
}


int usb_bulk_read_local(HUSB_LOCAL husb, void *data, int length)
{
	int transferred;
	if (husb == NULL)
		return -1;
	if (husb->handle == NULL)
		return -2;
	LIBUSB_CHECK(libusb_bulk_transfer(husb->handle, husb->endpoint_d2h, (uint8 *)data, length,
		&transferred, 5000));//0�ĳ�5S
	return transferred;
}

int usb_bulk_write_local(HUSB_LOCAL husb, const void *data, int length)
{
	int transferred;
	if (husb == NULL)
		return -1;
	if (husb->handle == NULL)
		return -2;
	LIBUSB_CHECK(libusb_bulk_transfer(husb->handle, husb->endpoint_h2d, (uint8 *)data, length,
		&transferred, 5000));//0�ĳ�5000
	return transferred;
}

int mass_storage_send_command_local(HUSB_LOCAL husb, const void *command_block, int command_block_length,
	uint8 direction, int transfer_length)
{
	uint8 cbw[31];
	memset(cbw, 0, 31);
	memcpy(cbw + 15, command_block, command_block_length);
	memcpy(cbw, "\x55\x53\x42\x43", 4);
	memcpy(cbw + 4, "\x98\xf8\xd6\x81", 4);
	memcpy(cbw + 8, &transfer_length, 4);
	cbw[12] = direction;
	cbw[13] = 0;
	cbw[14] = command_block_length;
	return usb_bulk_write_local(husb, cbw, 31);
}

//����ֵ<0���豸IO����,-100ΪUSB SCSIЭ�鷵�ص�ִ��״̬���벻Ϊ0������
//����ֵ=0���޶������ݿɶ�
//����ֵ>0���п������ݳ���;pRecvDataBuff��ſ������ݻ���
int mass_storage_io_local(HUSB_LOCAL husb, uint8 *pSendCmd, int nCmdLen, int UMSHeader_nDataTransferLen,
	uint8 UMSHeader_direction, uint8 *pTransferData, int nTransferLen,
	uint8 *pRecvDataBuff, int nRecvDataBuffLen)
{
	int nReallyTransferLen = UMSHeader_nDataTransferLen;
	if (0 != pTransferData)
		nReallyTransferLen = nTransferLen;
	int result =
		mass_storage_send_command_local(husb, pSendCmd, nCmdLen, UMSHeader_direction, nReallyTransferLen);
	if (result < 0)
		return -1;
	if (0 != pTransferData) {
		result = usb_bulk_write_local(husb, pTransferData, nTransferLen);
		if (result < 0)
			return -2;
	}
	memset(pRecvDataBuff, 0, nRecvDataBuffLen);
	result = usb_bulk_read_local(husb, pRecvDataBuff, nRecvDataBuffLen);
	if (result < 0)
		return -3;
	if (0x55 == pRecvDataBuff[0] && 0x53 == pRecvDataBuff[1] && 0x42 == pRecvDataBuff[2]) {
		///��USBflag��������;����USBFlag��˵��ִ���Ѿ���ɣ��������Ѿ�����ֱ��ȡִ�н��
		if (13 != result)
			return -3;
		uint8 bOK = pRecvDataBuff[12];
		if (0 != bOK)
			return -100;  //�������ֵ���ܸĶ�
		uint32 uFollowDataLen = ntohl(*(uint32 *)(pRecvDataBuff + 8));
		if (uFollowDataLen > nRecvDataBuffLen)
			return -5;
		if (0 == uFollowDataLen)
			return 0;
		memset(pRecvDataBuff, 0, nRecvDataBuffLen);
		result = usb_bulk_read_local(husb, pRecvDataBuff, nRecvDataBuffLen);
		if (result < 0)
			return -4;
		return result;
	}
	else {  //�������Ѿ��������read�����ˣ�ʣ��ֻʣ�º�USBflag(����ŵ)�����������read����Ϊ0��ִ�н��û��������Ҫ�Ȼ��ٶ�һ��
		uint8 abNew[512];
		int i = 0;
		for (i = 0; i < 30; i++) {  //���ؽ����ʱ3���ȡ
			memset(abNew, 0, 512);
			int nNew = usb_bulk_read_local(husb, abNew, 512);
			if (nNew < 0)
				break;
			if (0 == nNew) {
				usleep(100000);  // 100ms
				continue;
			}
			if (13 != nNew)
				break;
			uint8 bOK = abNew[12];
			if (0 != bOK)
				return -100;  //�������ֵ���ܸĶ�
			return result;
		}
		return -6;
	}
	return 0;
}
// GetNisecDeviceErrorCode
int GetNisecDeviceLastErrorCode(HUSB_LOCAL hUSB, char *szOutErrorCode)
{
	int nRet = 0;
	uint v2 = 0, dwDeviceLastErrorCode = 0;  // dword_3BAF30
	uint8 cb[256] = { 0 };
	memcpy(cb, "\x03\x00\x00\x00\x60", 5);
	if ((nRet = NisecLogicIORawLocal(hUSB, cb, 5, sizeof(cb), NULL, 0)) < 16)
		return -1;
	if (0xa != cb[7] && 0xc != cb[7])  // nisec after len == 10, cntax == 12
		return -2;
	dwDeviceLastErrorCode = bswap_32(*(uint32 *)(cb + 10));
	if (dwDeviceLastErrorCode) {
		v2 = (unsigned int)dwDeviceLastErrorCode >> 16;
		if (BYTE1(v2))
			sprintf(szOutErrorCode, "[DEV] %.8x", dwDeviceLastErrorCode);
		else
			sprintf(szOutErrorCode, "[DEV] 09%.4x", dwDeviceLastErrorCode % 0xFFFFu);
	}
	else
		sprintf(szOutErrorCode, "[DEV] %.2x", dwDeviceLastErrorCode);
	//�������ЩԤ���ô�����������nisec�ٷ����ģ�cntax�ľ���������������ο�
	char *pErr = szOutErrorCode + 6;
	if (!strncmp(pErr, "04", 2))
		tax_interface_out("[NisecLogicIO_local] Hardware of device error\r\n");
	else if (!strncmp(pErr, "05", 2))
		tax_interface_out("[NisecLogicIO_local] Paraments of command error\r\n");
	else if (!strcmp(pErr, "092000"))
		tax_interface_out("[NisecLogicIO_local] Illegal command\r\n");
	else if (!strcmp(pErr, "092100"))
		tax_interface_out("[NisecLogicIO_local] Overflow read/write\r\n");
	else if (!strcmp(pErr, "092400"))
		tax_interface_out("[NisecLogicIO_local] Paraments of command error\r\n");
	else if (!strcmp(pErr, "092700"))
		tax_interface_out("[NisecLogicIO_local] Device write protection\r\n");
	else if (!strcmp(pErr, "09d110"))
		tax_interface_out("[NisecLogicIO_local] Invoice type unsupported\r\n");
	else if (!strcmp(pErr, "09c000"))
		tax_interface_out("[NisecLogicIO_local] Device didnot open normal\r\n");
	else {
		tax_interface_out("[NisecLogicIO_local] Command at application layer excute error, error-value:%s\r\n",
			pErr);
	}
	return 0;
}
//ԭ�����ͣ����Ķ�USBCmd
int NisecLogicIORawLocal(HUSB_LOCAL device, uint8 *pInOutCmd, int nInCmdLen, int nInCmdBufLen,uint8 *pTransferData, int nTransferDataLen)
{
	// pInOutCmd 0xfe0x6f �̶�
	int nRet = 0;
	uint8 cmd[24] = { 0 };
	if (nInCmdLen > 16)
		return -1;
	memcpy(cmd, pInOutCmd, nInCmdLen);
	uint8 bTransferArrow = MSC_DIR_D2H;
	if (pTransferData)
		bTransferArrow = MSC_DIR_H2D;
	nRet = mass_storage_io_local(device, cmd, nInCmdLen, nInCmdBufLen, bTransferArrow, pTransferData,
		nTransferDataLen, (uint8 *)pInOutCmd, nInCmdBufLen);
	//tax_interface_out("NisecLogicIORawLocal nRet = %d\n",nRet);
	if (-100 == nRet)  // SCSCI ����
		GetNisecDeviceLastErrorCode(device, device->szLastDevError);
	return nRet;
}

// Transfer mode, 0xeλ�����и�У��λ
// a1 src;a2 src-len;ret=vcode
uint16 NisecBuildVCode(uint8 *a1, int a2)
{
    //���ֽ����FF��2=1fe����һ���ֽ�Ԥ�������1ff
    uint8 word_38E680[512] = {
        0x00, 0x00, 0x89, 0x11, 0x12, 0x23, 0x9B, 0x32, 0x24, 0x46, 0xAD, 0x57, 0x36, 0x65, 0xBF,
        0x74, 0x48, 0x8C, 0xC1, 0x9D, 0x5A, 0xAF, 0xD3, 0xBE, 0x6C, 0xCA, 0xE5, 0xDB, 0x7E, 0xE9,
        0xF7, 0xF8, 0x81, 0x10, 0x08, 0x01, 0x93, 0x33, 0x1A, 0x22, 0xA5, 0x56, 0x2C, 0x47, 0xB7,
        0x75, 0x3E, 0x64, 0xC9, 0x9C, 0x40, 0x8D, 0xDB, 0xBF, 0x52, 0xAE, 0xED, 0xDA, 0x64, 0xCB,
        0xFF, 0xF9, 0x76, 0xE8, 0x02, 0x21, 0x8B, 0x30, 0x10, 0x02, 0x99, 0x13, 0x26, 0x67, 0xAF,
        0x76, 0x34, 0x44, 0xBD, 0x55, 0x4A, 0xAD, 0xC3, 0xBC, 0x58, 0x8E, 0xD1, 0x9F, 0x6E, 0xEB,
        0xE7, 0xFA, 0x7C, 0xC8, 0xF5, 0xD9, 0x83, 0x31, 0x0A, 0x20, 0x91, 0x12, 0x18, 0x03, 0xA7,
        0x77, 0x2E, 0x66, 0xB5, 0x54, 0x3C, 0x45, 0xCB, 0xBD, 0x42, 0xAC, 0xD9, 0x9E, 0x50, 0x8F,
        0xEF, 0xFB, 0x66, 0xEA, 0xFD, 0xD8, 0x74, 0xC9, 0x04, 0x42, 0x8D, 0x53, 0x16, 0x61, 0x9F,
        0x70, 0x20, 0x04, 0xA9, 0x15, 0x32, 0x27, 0xBB, 0x36, 0x4C, 0xCE, 0xC5, 0xDF, 0x5E, 0xED,
        0xD7, 0xFC, 0x68, 0x88, 0xE1, 0x99, 0x7A, 0xAB, 0xF3, 0xBA, 0x85, 0x52, 0x0C, 0x43, 0x97,
        0x71, 0x1E, 0x60, 0xA1, 0x14, 0x28, 0x05, 0xB3, 0x37, 0x3A, 0x26, 0xCD, 0xDE, 0x44, 0xCF,
        0xDF, 0xFD, 0x56, 0xEC, 0xE9, 0x98, 0x60, 0x89, 0xFB, 0xBB, 0x72, 0xAA, 0x06, 0x63, 0x8F,
        0x72, 0x14, 0x40, 0x9D, 0x51, 0x22, 0x25, 0xAB, 0x34, 0x30, 0x06, 0xB9, 0x17, 0x4E, 0xEF,
        0xC7, 0xFE, 0x5C, 0xCC, 0xD5, 0xDD, 0x6A, 0xA9, 0xE3, 0xB8, 0x78, 0x8A, 0xF1, 0x9B, 0x87,
        0x73, 0x0E, 0x62, 0x95, 0x50, 0x1C, 0x41, 0xA3, 0x35, 0x2A, 0x24, 0xB1, 0x16, 0x38, 0x07,
        0xCF, 0xFF, 0x46, 0xEE, 0xDD, 0xDC, 0x54, 0xCD, 0xEB, 0xB9, 0x62, 0xA8, 0xF9, 0x9A, 0x70,
        0x8B, 0x08, 0x84, 0x81, 0x95, 0x1A, 0xA7, 0x93, 0xB6, 0x2C, 0xC2, 0xA5, 0xD3, 0x3E, 0xE1,
        0xB7, 0xF0, 0x40, 0x08, 0xC9, 0x19, 0x52, 0x2B, 0xDB, 0x3A, 0x64, 0x4E, 0xED, 0x5F, 0x76,
        0x6D, 0xFF, 0x7C, 0x89, 0x94, 0x00, 0x85, 0x9B, 0xB7, 0x12, 0xA6, 0xAD, 0xD2, 0x24, 0xC3,
        0xBF, 0xF1, 0x36, 0xE0, 0xC1, 0x18, 0x48, 0x09, 0xD3, 0x3B, 0x5A, 0x2A, 0xE5, 0x5E, 0x6C,
        0x4F, 0xF7, 0x7D, 0x7E, 0x6C, 0x0A, 0xA5, 0x83, 0xB4, 0x18, 0x86, 0x91, 0x97, 0x2E, 0xE3,
        0xA7, 0xF2, 0x3C, 0xC0, 0xB5, 0xD1, 0x42, 0x29, 0xCB, 0x38, 0x50, 0x0A, 0xD9, 0x1B, 0x66,
        0x6F, 0xEF, 0x7E, 0x74, 0x4C, 0xFD, 0x5D, 0x8B, 0xB5, 0x02, 0xA4, 0x99, 0x96, 0x10, 0x87,
        0xAF, 0xF3, 0x26, 0xE2, 0xBD, 0xD0, 0x34, 0xC1, 0xC3, 0x39, 0x4A, 0x28, 0xD1, 0x1A, 0x58,
        0x0B, 0xE7, 0x7F, 0x6E, 0x6E, 0xF5, 0x5C, 0x7C, 0x4D, 0x0C, 0xC6, 0x85, 0xD7, 0x1E, 0xE5,
        0x97, 0xF4, 0x28, 0x80, 0xA1, 0x91, 0x3A, 0xA3, 0xB3, 0xB2, 0x44, 0x4A, 0xCD, 0x5B, 0x56,
        0x69, 0xDF, 0x78, 0x60, 0x0C, 0xE9, 0x1D, 0x72, 0x2F, 0xFB, 0x3E, 0x8D, 0xD6, 0x04, 0xC7,
        0x9F, 0xF5, 0x16, 0xE4, 0xA9, 0x90, 0x20, 0x81, 0xBB, 0xB3, 0x32, 0xA2, 0xC5, 0x5A, 0x4C,
        0x4B, 0xD7, 0x79, 0x5E, 0x68, 0xE1, 0x1C, 0x68, 0x0D, 0xF3, 0x3F, 0x7A, 0x2E, 0x0E, 0xE7,
        0x87, 0xF6, 0x1C, 0xC4, 0x95, 0xD5, 0x2A, 0xA1, 0xA3, 0xB0, 0x38, 0x82, 0xB1, 0x93, 0x46,
        0x6B, 0xCF, 0x7A, 0x54, 0x48, 0xDD, 0x59, 0x62, 0x2D, 0xEB, 0x3C, 0x70, 0x0E, 0xF9, 0x1F,
        0x8F, 0xF7, 0x06, 0xE6, 0x9D, 0xD4, 0x14, 0xC5, 0xAB, 0xB1, 0x22, 0xA0, 0xB9, 0x92, 0x30,
        0x83, 0xC7, 0x7B, 0x4E, 0x6A, 0xD5, 0x58, 0x5C, 0x49, 0xE3, 0x3D, 0x6A, 0x2C, 0xF1, 0x1E,
        0x78, 0x0F};
    uint16 v3 = 0xFFFF;
    if (a2) {
        do {
            LOWORD(v3) = BYTE1(v3) ^ *(uint16 *)&word_38E680[(*a1++ ^ (unsigned __int8)v3) * 2];
            --a2;
        } while (a2);
    }
    v3 = ~v3;
    return v3;
}

uint8 BuildTransferVCode(uint8 *a1, int a2)
{
	uint16 v3 = NisecBuildVCode(a1, a2);
	uint8 v7 = HIBYTE(v3) ^ LOBYTE(v3);
	return v7;
}

// pInOutCmd�������������ݻᱻ�޸ģ����ǲ���Խ���޸�
int NisecLogicIO_local(HUSB_LOCAL device, uint8 *pInOutCmd, int nInCmdLen, int nInCmdBufLen,
	uint8 *pTransferData, int nTransferDataLen)
{
	memset(pInOutCmd + nInCmdLen, 0, 16 - nInCmdLen);
	if (pTransferData)
		pInOutCmd[0xe] = BuildTransferVCode(pTransferData, nTransferDataLen);
	return NisecLogicIORawLocal(device, pInOutCmd, 16, nInCmdBufLen, pTransferData, nTransferDataLen);
}

//����ֵΪʣ�ೢ�Դ˴���<0����������ִ�У����Ժ��Է���ֵ
int PasswordNotice(bool bDeviceOrCert, char *szLastDevError)
{
	char szTmpBuf[16] = "";
	if (!strlen(szLastDevError))
		return -1;
	char *pErr = szLastDevError + 6;
	// ukey��û��ʵ�̲����Ƿ�Ϊ��ֵ������������������̼ǵò�����
	if (!strcmp(pErr, "096983"))
		strcpy(szTmpBuf, "0");
	else {
		char t = pErr[strlen(pErr) - 1];
		sprintf(szTmpBuf, "%c", t);
	}
	tax_interface_out("%s password incorrect, please check. Remaining [\033[1;31m--%s--\033[0m] times to retry",	bDeviceOrCert ? "Device" : "Certificate", szTmpBuf);
	return atoi(szTmpBuf);
}

// pDeviceOpenInfo��Ϊ�գ������С����16�ֽ�,���64
int CntaxEntryRetInfo_local(HUSB_LOCAL device, uint8 bOPType, uint8 *pDeviceOpenInfo)
{
#ifdef _NODEVICE_TEST
	return 0;
#endif
	if (NULL == device)
		return -1;
	// 20210206�汾֮ǰĬ���������룬�豸Ĭ�Ͽ���88888888��֤������12345678���汾֮��ͳһΪһ������12345678
	char szPwd[] = "12345678";
	// char szPwd[] = "88888888";
	uint8 cb[512];
	int nRet = -2;
	switch (bOPType) {
	case NOP_CLOSE_LOCAL:
		memcpy(cb, "\xfe\x02", 2);  // close
		if (NisecLogicIO_local(device, cb, 2, sizeof(cb), NULL, 0) < 0)
			break;
		nRet = 0;
		break;
	case NOP_OPEN_LOCAL:
	case NOP_OPEN_FULL_LOCAL: 
	{  //����\xfe\x01��ɣ����3λ���ؽ������λ����Ӱ��
		memcpy(cb, "\xfe\x01\x01", 3);  // open
		int nChildRet = NisecLogicIO_local(device, cb, 3, sizeof(cb), NULL, 0);
		if (nChildRet < 16 || nChildRet > 64)
			break;
		if (pDeviceOpenInfo)
			memcpy(pDeviceOpenInfo, cb, nChildRet);
		if (NOP_OPEN_LOCAL == bOPType) {
			nRet = 0;
			break;
		}
		// check pin
		memcpy(cb, "\xfe\x03", 2);
		memset(device->szLastDevError, 0, sizeof(device->szLastDevError));
		nChildRet = NisecLogicIO_local(device, cb, 2, sizeof(cb), (uint8 *)szPwd, strlen(szPwd));
		if (nChildRet != 0) {
			tax_interface_out("�ײ������룺%s\n", device->szLastDevError);
			PasswordNotice(true, device->szLastDevError);
			break;
		}
		nRet = 0;
		break;
	}
	default:
		break;
	}
	return nRet;
}


int CntaxEntry_local(HUSB_LOCAL device, uint8 bOPType)
{
	return CntaxEntryRetInfo_local(device, bOPType, NULL);
}



int NisecEntry_local(HUSB_LOCAL device, uint8 bOPType)
{
#ifdef _NODEVICE_TEST
	return 0;
#endif
	if (NULL == device)
		return -1;
	char szPwd[] = "88888888";
	uint8 cb[512];
	int nRet = -2;
	switch (bOPType) {
	case NOP_CLOSE_LOCAL:
		memcpy(cb, "\xfe\x02", 2);  // close
		if (NisecLogicIO_local(device, cb, 2, sizeof(cb), NULL, 0) < 0)
			break;
		nRet = 0;
		break;
	case NOP_OPEN_LOCAL:
		memcpy(cb, "\xfe\x01", 2);  // open
		int nChildRet = NisecLogicIO_local(device, cb, 2, sizeof(cb), NULL, 0);
		if (nChildRet != 10)
			break;
		nRet = 0;
		break;
	case NOP_OPEN_FULL_LOCAL: 
	{
		memcpy(cb, "\xfe\x01", 2);  // open
		int nChildRet = NisecLogicIO_local(device, cb, 2, sizeof(cb), NULL, 0);
		if (nChildRet != 10)
			break;
		memcpy(cb, "\xfe\x23\x01", 3);  // select tax app
		nChildRet = NisecLogicIO_local(device, cb, 3, sizeof(cb), NULL, 0);
		if (nChildRet != 0)
			break;
		// check pin
		memcpy(cb, "\xfe\x12", 2);
		nChildRet = NisecLogicIO_local(device, cb, 2, sizeof(cb), (uint8 *)szPwd, strlen(szPwd));
		if (nChildRet != 0)
			break;
		nRet = 0;
		break;
	}
	default:
		break;
	}
	return nRet;
}

//����ֵ<0 ����;0 δ֪�豸��1 aisino; 2 nisec
int usb_device_open_local(HUSB_LOCAL hUSB, int nBusNum, int nDevNum)
{
	//if ((nBusNum<1) || (nBusNum>127))
	//{
	//	printf("nBusNum range error\n");
	//	
	//	return -1;
	//}
	//if ((nDevNum<1) || (nDevNum>127))
	//{
	//	printf("nDevNum range error\n");
	//	libusb_exit(hUSB->context);
	//	return -1;
	//}


	//uint16 payload_d2h = 0;
	//uint16 payload_h2d = 0;
	libusb_device **device_list = NULL;
	int device_num = libusb_get_device_list(hUSB->context, &device_list);
	if (device_num <= 0) {
		libusb_free_device_list(device_list, 1);
		fprintf(stderr, "[Device] No device found on the bus.\n");
		libusb_exit(hUSB->context);
		return -1;
	}


	int i = 0;
	for (i = 0; i < device_num; i++)
	{
		struct libusb_device_descriptor desc;
		LIBUSB_CHECK(libusb_get_device_descriptor(device_list[i], &desc));
		//printf("desc.idVendor=%x,desc.idProduct=%x\n",desc.idVendor,desc.idProduct);
		int nNowBus = libusb_get_bus_number(device_list[i]);
		int nNowDev = libusb_get_device_address(device_list[i]);
		if (nBusNum != 0 || nDevNum != 0)
		{
			if (nNowBus != nBusNum || nNowDev != nDevNum)
			{
				continue;
			}
		}
		struct libusb_config_descriptor *conf_desc;
		LIBUSB_CHECK(libusb_get_active_config_descriptor(device_list[i], &conf_desc));
		unsigned int j = 0;
		for (j = 0; j < conf_desc->bNumInterfaces; j++)
		{
			const struct libusb_interface *interface = &conf_desc->interface[j];
			if (interface->num_altsetting >= 1)
			{
				const struct libusb_interface_descriptor *if_desc = &interface->altsetting[0];
				if (if_desc->bInterfaceClass == LIBUSB_CLASS_MASS_STORAGE && if_desc->bInterfaceSubClass == 0x06 && if_desc->bInterfaceProtocol == 0x50)
				{
					unsigned int k = 0;
					for (k = 0; k < if_desc->bNumEndpoints; k++)
					{
						const struct libusb_endpoint_descriptor *ep_desc = &if_desc->endpoint[k];

						if ((ep_desc->bmAttributes & 0x03) == LIBUSB_TRANSFER_TYPE_BULK)
						{
							if (ep_desc->bEndpointAddress & 0x80)
							{
								hUSB->endpoint_d2h = ep_desc->bEndpointAddress;
								//payload_d2h = ep_desc->wMaxPacketSize;
							}
							else
							{
								hUSB->endpoint_h2d = ep_desc->bEndpointAddress;
								//payload_h2d = ep_desc->wMaxPacketSize;
							}
						}
					}
					if (hUSB->endpoint_d2h == 0 || hUSB->endpoint_h2d == 0)
					{
						fprintf(stderr, "[Device] Interface is not complete.\n");
						libusb_free_config_descriptor(conf_desc);
						libusb_free_device_list(device_list, 1);
						libusb_exit(hUSB->context);
						return -2;
					}

					LIBUSB_CHECK(libusb_open(device_list[i], &(hUSB->handle)));

					libusb_set_auto_detach_kernel_driver(hUSB->handle, 1);
					libusb_reset_device(hUSB->handle);//���ܼ�LIBUSB_CHECK��mips���ݴ�������
					libusb_claim_interface(hUSB->handle, 0);


					char szManufacturer[256] = "";
					char szProduct[256] = "";
					if (libusb_get_string_descriptor_ascii(hUSB->handle, desc.iManufacturer, (uint8 *)szManufacturer, 256) < 0 || libusb_get_string_descriptor_ascii(hUSB->handle, desc.iProduct, (uint8 *)szProduct, 256) < 0){
						fprintf(stderr, "[Device] libusb_get_string_descriptor_ascii is not complete.\n");
						if ((desc.idVendor == 0x0680) && (desc.idProduct == 0x2006))
						{
							libusb_free_config_descriptor(conf_desc);
							libusb_free_device_list(device_list, 1);
							return DEVICE_MENGBAI2_LOCAL;
						}
						if ((desc.idVendor == 0x2ec9) && (desc.idProduct == 0x1001))
						{
							libusb_free_config_descriptor(conf_desc);
							libusb_free_device_list(device_list, 1);
							return DEVICE_CNTAX_LOCAL;
						}
						if ((desc.idVendor == 0x3a59) && (desc.idProduct == 0x4458))
						{
							libusb_free_config_descriptor(conf_desc);
							libusb_free_device_list(device_list, 1);
							return DEVICE_CNTAX_LOCAL;
						}
						libusb_free_config_descriptor(conf_desc);
						libusb_free_device_list(device_list, 1);
						libusb_close(hUSB->handle);
						libusb_exit(hUSB->context);
						return -3;
					}
					//printf("[Device] BusID:%d DevID:%d Manufacturer:%s Product:%s\r\n", nNowBus,   nNowDev, szManufacturer, szProduct);
					//printf("Wang Printf [Device] BusID:%d DevID:%d Manufacturer:%s Product:%s\r\n", nNowBus,   nNowDev, szManufacturer, szProduct);
					//printf("Wang Printf desc.idVendor ==%x , desc.idProduct ==%x\r\n",desc.idVendor,desc.idProduct);
					int nDeviceType = -1;
					if (strstr((const char *)szManufacturer, "NISEC"))
					{
						if ((desc.idVendor == 0x1432) && (desc.idProduct == 0x07dc))
						{
							nDeviceType = DEVICE_NISEC_LOCAL;
						}
						else if ((desc.idVendor == 0x1432) && (desc.idProduct == 0x07e2))
						{
							nDeviceType = DEVICE_SKP_KEY_LOCAL;
						}
					}
					else if (strstr((const char *)szManufacturer, "Aisino"))
					{
						if (!strcmp(szProduct, "USB Flash Disk"))
							nDeviceType = DEVICE_AISINO_LOCAL;
						else if (!strcmp(szProduct, "ChinaTax SWUKey"))
							nDeviceType = DEVICE_CNTAX_LOCAL;
						else
							nDeviceType = DEVICE_AISINO_LOCAL;
					}
					else if (strstr((const char *)szManufacturer, "Aisno"))
					{
						nDeviceType = DEVICE_AISINO_LOCAL;
					}
					else if (strstr((const char *)szManufacturer, "ChinaTax SWUKey"))
					{
						if (strcmp(szProduct, "ChinaTax SWUKey") == 0)
						{
							if ((desc.idVendor == 0x14d6) && (desc.idProduct == 0xa002))
							{
								nDeviceType = DEVICE_CNTAX_LOCAL;
							}
							else if ((desc.idVendor == 0x3254) && (desc.idProduct == 0xff10))
							{
								nDeviceType = DEVICE_CNTAX_LOCAL;
							}
							else if ((desc.idVendor == 0x1677) && (desc.idProduct == 0x6101))
							{
								nDeviceType = DEVICE_CNTAX_LOCAL;
							}
							else if ((desc.idVendor == 0x2012) && (desc.idProduct == 0x2010))
							{
								nDeviceType = DEVICE_CNTAX_LOCAL;
							}
							else if ((desc.idVendor == 0x1432) && (desc.idProduct == 0x07d0))
							{
								nDeviceType = DEVICE_CNTAX_LOCAL;
							}
							else
							{
								nDeviceType = DEVICE_UNKNOW_LOCAL;
							}
						}
						else
						{
							nDeviceType = DEVICE_UNKNOW_LOCAL;
						}

					}
					else if (strstr((const char *)szManufacturer, "USB_MSC"))
					{
						if (strcmp(szProduct, "ChinaTax SWUKey") == 0)
						{
							if ((desc.idVendor == 0x287f) && (desc.idProduct == 0x00d4))
							{
								nDeviceType = DEVICE_CNTAX_LOCAL;
							}
						}
						else
						{
							nDeviceType = DEVICE_UNKNOW_LOCAL;
						}
					}
					else if (strstr((const char *)szManufacturer, "ExcelSecu"))
					{
						if ((strcmp(szProduct, "ChinaTax SWUKey") == 0) || (strcmp(szProduct, "USB Key") == 0))
						{
							if ((desc.idVendor == 0x1ea8) && (desc.idProduct == 0xb00b))
							{
								nDeviceType = DEVICE_CNTAX_LOCAL;
							}
						}
						else
						{
							nDeviceType = DEVICE_UNKNOW_LOCAL;
						}
					}
					else if (strstr((const char *)szManufacturer, "ShuiWu"))
					{
						if (strcmp(szProduct, "UKEY") == 0)
						{
							if ((desc.idVendor == 0x1ea8) && (desc.idProduct == 0xb00b))
							{
								nDeviceType = DEVICE_CNTAX_LOCAL;
							}
						}
						else
						{
							nDeviceType = DEVICE_UNKNOW_LOCAL;
						}
					}
					else if (strstr((const char *)szManufacturer, "SWUKEY"))
					{
						if (strcmp(szProduct + 1, "ChinaTax SWUKey") == 0)
						{
							if ((desc.idVendor == 0xd449) && (desc.idProduct == 0x0001))
							{
								nDeviceType = DEVICE_CNTAX_LOCAL;
							}

						}
						else
						{
							nDeviceType = DEVICE_UNKNOW_LOCAL;
						}
					}
					else if (strstr((const char *)szManufacturer, "Watchdata"))
					{
						if (strcmp(szProduct + 1, "ChinaTax SWUKey") == 0)
						{
							if ((desc.idVendor == 0x163c) && (desc.idProduct == 0x5480))
							{
								nDeviceType = DEVICE_CNTAX_LOCAL;
							}

						}
						else
						{
							nDeviceType = DEVICE_UNKNOW_LOCAL;
						}
					}
					else if (strstr((const char *)szManufacturer, "RONGANCHINA"))
					{
						if (strcmp(szProduct, "SWUKey") == 0)
						{
							if ((desc.idVendor == 0x2ec9) && (desc.idProduct == 0x1001))
							{
								nDeviceType = DEVICE_CNTAX_LOCAL;
							}
						}
						
						else if (strcmp(szProduct, "USBKEY 200") == 0)
						{
							if ((desc.idVendor == 0x2ec9) && (desc.idProduct == 0x1001))
							{
								nDeviceType = DEVICE_NONGYE_LOCAL;
							}
						}
						else
						{
							nDeviceType = DEVICE_UNKNOW_LOCAL;
						}
					}
					else
					{
						nDeviceType = DEVICE_UNKNOW_LOCAL;
					}
					if (nDeviceType == DEVICE_NISEC_LOCAL)
					{  //����������ȡ����������
						unsigned char usb_data[128];
						memset(usb_data, 0, sizeof usb_data);
						// USB�������洢��Ҫ��δ��룿����ǵ�һ�β��ϵ��豸ִ�лᱨ������(��Դռ�õĴ��󣬲��ù�)���ڶ�����ִ�о��޴���
						libusb_control_transfer(hUSB->handle, 0xa1, 0xfe, 0x0000, 0, usb_data, 1, 3000);
					}

					libusb_free_config_descriptor(conf_desc);
					libusb_free_device_list(device_list, 1);
					return nDeviceType;
				}
			}
		}
		libusb_free_config_descriptor(conf_desc);
	}
	libusb_free_device_list(device_list, 1);
	libusb_exit(hUSB->context);
	fprintf(stderr, "[Device] No target device found.\n");
	return -4;
}

int usb_device_open_hid_local(HUSB_LOCAL hUSB, int nBusNum, int nDevNum)
{
	if ((nBusNum<1) || (nBusNum>127))
	{
		printf("nBusNum range error\n");
		return -1;
	}
	if ((nDevNum<1) || (nDevNum>127))
	{
		printf("nDevNum range error\n");
		return -1;
	}

	//if (nBusNum) {
	//    fprintf(stdout, "[Device] Assign busid devid, %d:%d\n", nBusNum, nDevNum);
	//} else {
	//    fprintf(stdout, "[Device] Auto detect mass storage device\n");
	//}
	uint16 payload_d2h = 0;
	uint16 payload_h2d = 0;
	libusb_device **device_list = NULL;
	int device_num = libusb_get_device_list(hUSB->context, &device_list);
	if (device_num <= 0) {
		libusb_free_device_list(device_list, 1);
		fprintf(stderr, "[Device] No device found on the bus.\n");
		return -1;
	}
	int i = 0;
	for (i = 0; i < device_num; i++) {
		struct libusb_device_descriptor desc;
		LIBUSB_CHECK(libusb_get_device_descriptor(device_list[i], &desc));
		int nNowBus = libusb_get_bus_number(device_list[i]);
		int nNowDev = libusb_get_device_address(device_list[i]);
		if (nBusNum != 0 || nDevNum != 0) {
			if (nNowBus != nBusNum || nNowDev != nDevNum) {
				continue;
			}
		}
		struct libusb_config_descriptor *conf_desc;
		LIBUSB_CHECK(libusb_get_active_config_descriptor(device_list[i], &conf_desc));
		unsigned int j = 0;
		for (j = 0; j < conf_desc->bNumInterfaces; j++) {
			const struct libusb_interface *interface = &conf_desc->interface[j];
			if (interface->num_altsetting >= 1) {
				const struct libusb_interface_descriptor *if_desc = &interface->altsetting[0];
				/*if (if_desc->bInterfaceClass == LIBUSB_CLASS_MASS_STORAGE &&
				if_desc->bInterfaceSubClass == 0x06 && if_desc->bInterfaceProtocol == 0x50) {*/
				if (1){
					unsigned int k = 0;
					for (k = 0; k < if_desc->bNumEndpoints; k++) {
						const struct libusb_endpoint_descriptor *ep_desc = &if_desc->endpoint[k];

						if ((ep_desc->bmAttributes & 0x03) == LIBUSB_TRANSFER_TYPE_BULK) {
							if (ep_desc->bEndpointAddress & 0x80) {
								hUSB->endpoint_d2h = ep_desc->bEndpointAddress;
								payload_d2h = ep_desc->wMaxPacketSize;
							}
							else {
								hUSB->endpoint_h2d = ep_desc->bEndpointAddress;
								payload_h2d = ep_desc->wMaxPacketSize;
							}
						}
					}
					//printf("payload_d2h = %d,payload_h2d = %d\n", payload_d2h, payload_h2d);
					/*if (hUSB->endpoint_d2h == 0 || hUSB->endpoint_h2d == 0) {
					fprintf(stderr, "[Device] Interface is not complete.\n");
					libusb_free_config_descriptor(conf_desc);
					libusb_free_device_list(device_list, 1);
					return -2;
					}*/

					LIBUSB_CHECK(libusb_open(device_list[i], &(hUSB->handle)));
					LIBUSB_CHECK(libusb_reset_device(hUSB->handle));
					libusb_set_auto_detach_kernel_driver(hUSB->handle, 1);
					LIBUSB_CHECK(libusb_claim_interface(hUSB->handle, 0));

					//printf("usb_device_open_hid_local hUSB->handle = %x\n",hUSB->handle);

					libusb_set_auto_detach_kernel_driver(hUSB->handle, 1);
					if (libusb_reset_device(hUSB->handle) < 0) {
						fprintf(stderr, "[Device] libusb_reset_device reset failed\n");
						libusb_free_config_descriptor(conf_desc);
						libusb_free_device_list(device_list, 1);
						libusb_close(hUSB->handle);
						return -3;
					}
					uint8 szManufacturer[256] = "";
					if (libusb_get_string_descriptor_ascii(hUSB->handle, desc.iManufacturer,szManufacturer, 256) < 0)
					{
						fprintf(stderr, "[Device] libusb_get_string_descriptor_ascii failed\n");
						libusb_free_config_descriptor(conf_desc);
						libusb_free_device_list(device_list, 1);
						libusb_close(hUSB->handle);
						return -1;
					}

					int nDeviceType = -1;
					if ((desc.idVendor == 0x0680) && (desc.idProduct == 0x1901))
					{
						nDeviceType = DEVICE_MENGBAI_LOCAL;
					}
					else if ((desc.idVendor == 0x0680) && (desc.idProduct == 0x2008))
					{
						nDeviceType = DEVICE_MENGBAI2_LOCAL;
					}
					else if ((desc.idVendor == ROBOT_ARM_PID) && (desc.idProduct == ROBOT_ARM_VID))
					{
						nDeviceType = DEVICE_ROBOT_ARM_LOCAL;
					}
					else
						nDeviceType = DEVICE_UNKNOW_LOCAL;

					libusb_free_config_descriptor(conf_desc);
					libusb_free_device_list(device_list, 1);
					//("exit usb_device_open_hid_local\n");
					return nDeviceType;
				}
			}
		}
		libusb_free_config_descriptor(conf_desc);
	}
	libusb_free_device_list(device_list, 1);
	fprintf(stderr, "[Device] No target device found.\n");
	return -4;
}

void usb_device_close_local(HUSB_LOCAL husb)
{
	if (husb != NULL)
		libusb_close(husb->handle);
}

void usb_hid_close_local(HUSB_LOCAL husb)
{
	if (husb != NULL)
	{
		if(husb->handle != NULL)
		{
			//printf("husb->handle = %x\n",husb->handle);
			libusb_close(husb->handle);
			husb->handle = NULL;
		}
	}
		
}

int IsCryptBigIO(uint8 *abBanner)
{
	// ������СIO��ʽ banner example
	// 0000   00 1f 00 3b fc 20 00 00 10 80 57 45 53 54 4f 4e   ...;�� ....WESTON
	// 0010   10 61 36 36 31 35 32 31 30 35 39 32 35 30 00 ff   .a661521059250.
	// 0020   00 00                                             ..
	// ��������IO��ʽ banner example
	// 0000   00 1f 00 3b fc 20 00 00 10 80 57 45 53 54 4f 4e   ...;�� ....WESTON
	// 0010   20 60 36 36 31 39 31 37 38 34 35 30 38 39 00 ff    `661917845089.
	// 0020   00 00                                             ..
	uint8 *pFlag = abBanner + 3;
	bool bFlag1 = !memcmp(pFlag, "\x3b\xfc", 2) && !memcmp(pFlag + 5, "\x10\x80\x57\x45", 4);
	bool bFlag2 = !memcmp(pFlag, "\x3b\xff", 2) && !memcmp(pFlag + 5, "\x30\x80\x57\x45", 4);
	if (!(bFlag1 || bFlag2))
		return -1;
	uint8 bIOFlag = *(abBanner + 16);
	if (0x20 == bIOFlag)
		return 1;
	else if (0x10 == bIOFlag)
		return 0;
	else
		return -1;
}

void buildOriginSeed8B(uint8 *szSeed)
{
	szSeed[0] = 0x73;
	szSeed[1] = 0x75;
	szSeed[2] = 0x62;
	int nTimeNow = time(0);
	srand(nTimeNow);
	int i = 3;
	for (i = 3; i < 8; i++) {
		int nRand = rand();
		szSeed[i] = (uint8)nRand;
	}
}

// pInputBuf's len must >=8;������ֱ���޸�ԭbuf
int AisinoDesEcbEncrypt(uint8 *pInputBuf)
{
	// https://www.ssleye.com/des_cipher.html
	// aes_64_ecb zeropadding key=password
	int nInputBufLen = 8;  //�̶�8�ֽ�
	EVP_CIPHER_CTX *ctx = NULL;
	char szKey[8] = "password";
	int len, nCopyLen = 8, nRet = -1;
	uint8 abInputBuf[100] = { 0 };//ԭ��ֻ��8�ֽڣ�̫С�ᵼ�¶δ���

	// ZeroPadding
	memset(abInputBuf, 0, sizeof abInputBuf);
	if (nInputBufLen < nCopyLen)
		nCopyLen = nInputBufLen;
	memcpy(abInputBuf, pInputBuf, nCopyLen);

	if (!(ctx = EVP_CIPHER_CTX_new()))
		return -1;
	while (1) {
		if (!EVP_EncryptInit_ex(ctx, EVP_des_ecb(), NULL, (const unsigned char *)szKey, NULL))
			break;
		if (!EVP_EncryptUpdate(ctx, abInputBuf, &len, abInputBuf, 8))
			break;
		if (!EVP_EncryptFinal_ex(ctx, abInputBuf + len, &len))
			break;
		nRet = 0;
		break;
	}
	memcpy(pInputBuf, abInputBuf, 8);
	EVP_CIPHER_CTX_free(ctx);
	return nRet;
}

//����ֵ == 0,СIO;==1 ��IO
int ReadTaxDeviceID_local(HUSB_LOCAL device, char *szDevID)
{

	uint8 szSeed[8];
	char cb[512] = "";
	buildOriginSeed8B((uint8 *)szSeed);
	AisinoDesEcbEncrypt((uint8 *)szSeed);
	memcpy(cb, "\xf1\xf1\x00\x00\x00\x00\x02\x08\x00\xff", 10);
	int nRetChild = mass_storage_io_local(device, (uint8 *)cb, 10, 0, MSC_DIR_H2D, (uint8 *)szSeed,
		sizeof(szSeed), (uint8 *)cb, sizeof(cb));
	if (nRetChild != 0)
		return -1;
	memcpy(cb, "\xf1\xf1\x00\x00\x00\x00\x01\x04\x01\xff", 10);
	nRetChild =
		mass_storage_io_local(device, (uint8 *)cb, 10, 0, MSC_DIR_D2H, NULL, 0, (uint8 *)cb, sizeof(cb));
	if (nRetChild < 24)
		return -2;
	char *pRawID = cb + 0x12;
	pRawID[12] = '\0';
	strcpy(szDevID, pRawID);
	// check is bigio
	nRetChild = IsCryptBigIO((uint8 *)cb);
	return nRetChild;
}

//�������pInOutCmd��nInCmdLen��nInCmdBufLen�����һ����IO������,pTransferData��nTransferDataLen����ڶ�����IO����
//�����������������pInOutCmd��nRet��һ��Ϊ��������һ��Ϊ������ֵ����
int AisinoLogicIO_local(HUSB_LOCAL device, uint8 *pInOutCmd, int nInCmdLen, int nInCmdBufLen,
	uint8 *pTransferData, int nTransferDataLen)
{
	int nRetChild = 0;
	//--send command
	if ((nRetChild =
		mass_storage_io_local(device, pInOutCmd, nInCmdLen, 0, MSC_DIR_H2D, (uint8 *)pTransferData,
		nTransferDataLen, pInOutCmd, nInCmdBufLen)) < 0) {
		tax_interface_out( "[AisinoLogicIO_local] Command send failed\r\n");
		return -1;
	}
	//--recv header
	if ((nRetChild = mass_storage_io_local(device, g_CmdE1AA, sizeof g_CmdE1AA, 11, MSC_DIR_D2H, 0, 0,
		pInOutCmd, nInCmdBufLen)) < 0) {
		tax_interface_out( "[AisinoLogicIO_local] Logic header recv failed\r\n");
		return -2;
	}
	int wRepCode = GetDeviceRepCode(pInOutCmd);  //����ִ�гɹ��Ƿ�
	if (wRepCode != 0) {
		sprintf(device->szLastDevError, "[DEV] %d", wRepCode);
		tax_interface_out(
			"[AisinoLogicIO_local] Command at application layer excute error, error-value:%d\r\n",
			wRepCode);
		return -3;
	}
	wRepCode = GetDeviceRepCode(pInOutCmd + 4);  //����content����,ʵ�ʷ��س���Ϊlen+1λ��У����
	if (!wRepCode)
		return 0;
	//���ص�bufflen���ڻ�����,Ԥ��һ�������ֽڸ������Լ����ض������
	uint8 abExtraMyBuf[12] = { 0 };
	//��������ڴ����� 00[1] signed myret flag[1] CmdE1AARet[10]
	int nMyExtraLen = sizeof(abExtraMyBuf);
	if (wRepCode + nMyExtraLen > nInCmdBufLen) {
		tax_interface_out( "[AisinoLogicIO_local] Out buf's size not enough, need:%d\r\n",
			wRepCode + nMyExtraLen);
		return -4;
	}
	//����CmdE1AARet���ں��ڱ���
	memcpy(abExtraMyBuf + 2, pInOutCmd, 10);  //�豸����У��λ1�ֽڣ��Լ���1�ֽ�;
	if (pInOutCmd[3] != 0xe3)                 //==\xe3 ��ֹ��ȡ���������
		*(sint8 *)(abExtraMyBuf + 1) = 1;     //������ȡ
	//--recv data
	nRetChild = mass_storage_io_local(device, g_CmdE1AB, sizeof g_CmdE1AB, nInCmdBufLen, MSC_DIR_D2H, 0,
		0, pInOutCmd, nInCmdBufLen);
	if (nRetChild < 0 || nRetChild != wRepCode + 1) {
		tax_interface_out( "[AisinoLogicIO_local] Output buff's size not match header's num\r\n");
		return -5;
	}
	//������Զ��������������ݺ��汸��
	uint8 *pExtraMyRetBuf = pInOutCmd + nRetChild;
	memcpy(pExtraMyRetBuf, abExtraMyBuf, nMyExtraLen);
	//ֻ����ʵ�ʳ��ȣ��Զ��峤�Ȳ������ڷ��س�����
	return nRetChild;
}

int AisinoReadTaxIDandCustomName_local(HUSB_LOCAL device, char *szTaxID, char *szComName, size_t nComNameBufLen)
{

	char szComNameChar[512] = "";
	int ncbLen = 5120;
	uint8 *cb = (uint8 *)calloc(1, ncbLen);
	if (NULL == cb)
		return -1;
	memcpy(cb, "\xfe\xff\xcc\xbc\x53\xe1\x40\x00\x00\x00", 10);
	int nChild = AisinoLogicIO_local(device, cb, 10, ncbLen, (uint8 *)"\x00\x10\x10", 3);
	if (nChild < 1024) {
		free(cb);
		return -2;
	}
	int nIndex = 2;
	int i = 0;
	for (i = 0;; i++) {
		uint8 bFlag = 0;
		memcpy(&bFlag, cb + nIndex, 1);
		if (0xff == bFlag)
			break;
		uint16 uHXLen = 0;
		memcpy(&uHXLen, cb + nIndex + 1, 2);
		if (uHXLen > 512)
			break;
		if (bFlag == 0x00) {
			char *szComNameCharIndex = (char *)(cb + nIndex + 3);
			memcpy(szComNameChar, szComNameCharIndex, uHXLen);
		}
		if (bFlag == 0x1b) {
			char *pTaxID = (char *)(cb + nIndex + 3);
			strncpy(szTaxID, pTaxID, uHXLen);
		}
		// to next
		nIndex += 3 + uHXLen;
	}
	free(cb);
	if (szComNameChar[0] == 0x0 || strlen(szTaxID) == 0)
		return -3;

	memcpy(szComName, szComNameChar, strlen(szComNameChar));
	return 0;
}

static unsigned char *SearchBytes(unsigned char *pBegin, unsigned char *pEnd, unsigned char *s, unsigned int nsize)
{
	unsigned int count;
	unsigned char *pDst;

	while ((unsigned long)pBegin + (unsigned long)nsize <= (unsigned long)pEnd)
	{
		pDst = s;
		count = 0;
		while (count < nsize && *pBegin == *pDst)
		{
			pBegin++;
			pDst++;
			count++;
		}
		if (count == nsize)
			break;
		pBegin = pBegin - count + 1;
	}
	//printf("count = %d \n",count);
	if (count == nsize)
	{
		return (unsigned char *)((unsigned long)pBegin - (unsigned long)count);
	}
	return NULL;

}

//------------------------------Str Byteת�� ������ begin------------
uint8 sub_100061B0(uint8 a1)
{
	uint8 result;
	if (a1 <= 9u)
		result = a1 + 48;
	else
		result = a1 + 87;
	return result;
}

// 0x120x340x50->"123450" len=6
// 0x1a0x05->"1a05"  //ת������Сд�ַ���
int Byte2Str(char *szOut, uint8 *szIn, int nInputLen)
{
	uint8 *v5;
	int result = nInputLen;
	int v4 = 0;
	if (nInputLen > 0) {
		v5 = (uint8 *)szOut;
		do {
			*(_BYTE *)v5 = sub_100061B0(*(_BYTE *)(v4 + szIn) >> 4);
			*(_BYTE *)(v5 + 1) = sub_100061B0(*(_BYTE *)(v4 + szIn) & 0xF);
			result = nInputLen;
			++v4;
			v5 += 2;
		} while (v4 < nInputLen);
	}
	return result * 2;  // 0x010x02 -> 0102
}

//�˺������Խ��ַ�������
void mystrrev(char *str)
{
	if (NULL == str)
		return;
	char *pBegin = str;
	char *pEnd = str + strlen(str) - 1;
	uint8 pTemp;
	while (pBegin < pEnd) {
		pTemp = *pBegin;
		*pBegin = *pEnd;
		*pEnd = pTemp;
		++pBegin, --pEnd;
	}
}
void mytrim(char *s, char c)
{
	char *t = s;
	while (*s == c) {
		s++;
	};
	if (*s) {
		char *t1 = s;
		while (*s) {
			s++;
		};
		s--;
		while (*s == c) {
			s--;
		};
		while (t1 <= s) {
			*(t++) = *(t1++);
		}
	}
	*t = 0;
}

//�����ַ���ǰ���пո�û����
//ע��:�ú��������memcpy��dst���壬�����󲻻��\0����Ҫ����Ԥ�л������Ƿ���Ҫ��0
int NisecBaseConv(const char *szInputStr, int nInputLen, uint8 bDstBase, char *szRetBuff,
	uint8 bOrigBase, uint8 bRetFormatLen)
{
	// pFunc(bT, 0x24, szDst, 0x10, 0x1a); //913201234567891 -> 0000000BBE2476A3EAAD611F85
	// pFunc(bT, 0x10, szDst, 0x24, 15);//0000000BBE2476A3EAAD611F85 -> 913201234567891
	// pFunc(bT, 0xa, szDst, 0x10, 0); //200150000001 -> 2E99DEA181
	// pFunc(bT, 0x16, szDst, 0xa, 0xa); //ffffffffff -> 0995116277
	_BYTE *lpMem;
	unsigned int v21;
	uint8 *v22;
	unsigned int v23;
	unsigned int v24;
	uint8 *v25;
	unsigned int v26;
	unsigned int v60;
	unsigned int v27;
	sint8 v28;
	uint8 *v29;
	sint8 v30;
	uint8 v31;
	unsigned int v32;
	void *v33 = NULL;
	unsigned int v34;
	uint8 *v35;
	uint8 *v36;
	int v37;
	int v38;
	unsigned int *v39;
	int v40;
	unsigned int v63;
	unsigned int *v64;
	uint8 *v70;
	uint8 v71;
	unsigned int v41;
	unsigned int v42;
	_DWORD *v43;
	bool v45;
	void *v59;
	unsigned int v61;
	uint8 vMemBlock[768] = "";
	char szDstTmpBuf[768] = "";
	if (nInputLen > (sizeof(vMemBlock)-64))
		return -1;
	memcpy(vMemBlock + 64, szInputStr, nInputLen);  //�ճ�ǰ��Ļ�������򣬷�ֹԽ���ڴ����
	uint8 *v66 = vMemBlock + 64;
	unsigned int v9 = strlen((char *)v66);
	signed int v13;
	int nRetIndex = 0;
	uint8 v68[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	unsigned int v67 = v9;
	uint8 unk_3AE3B8 = 0;
	sint8 v14;
	_BYTE *v18;
	_BYTE *v19;
	uint8 v20;
	unsigned int v10;

LABEL_13:
	v10 = v67;
	uint8 *v11 = (uint8 *)v66;
	_BYTE *v12;
	while (v67 > 0) {
		if (v11) {
			v11 = (uint8 *)v66;
			v10 = v67;
			v12 = v66;
		}
		else {
			v12 = &unk_3AE3B8;
		}
		if (*v12 != 48)
			break;
		v13 = 1;
		if (v11) {
			v14 = *(v11 - 1);
			if (v14) {
				if (v14 != -1) {
					// v15 = v11;
					v10 = v67;
					v11 = (uint8 *)v66;
					//����ȱ������;�ڴ�Խ�紦�����������뻺����֮ǰ��һ�㲻���õ���ʵ��̫���ӣ���ʱ������
				}
			}
		}
		if (v10 < 1) {
			v13 = v10;
			if (!v10)
				continue;
		}
		// memcpy(v11, &v11[v13], v10 - v13);
		memmove(v11, v11 + v13, v10 - v13);
		//����ȱ������;
		// v16 = v67 - v13;
		v67--;
		goto LABEL_13;
	}
	if (!v10) {  //ԭ�ⲻ������
		memcpy(szRetBuff, szInputStr, nInputLen);
		return 0;
	}
	v18 = malloc(v10 + 1);
	v19 = v18;
	v20 = v67 + 1;
	lpMem = v18;
	v21 = (v67 + 1) >> 2;
	memset(v18, 0, 4 * v21);
	v22 = &v18[4 * v21];
	LOBYTE(v21) = v20;
	v23 = 0;
	v60 = 0;
	memset(v22, 0, v21 & 3);
	v24 = v67;
	v25 = (uint8 *)v66;
	v26 = 0;
	do {
		v27 = v24 - v23 - 1;
		if (v67 >= v27 && v25) {
			v28 = *(v25 - 1);
			if (v28 && v28 != -1) {
				//����ȱ������;
				v25 = (uint8 *)v66;
				v23 = v60;
			}
			if (v25) {
				*(v25 - 1) = -1;
				v25 = (uint8 *)v66;
			}
			v29 = &v25[v27];
		}
		else {
			v29 = &unk_3AE3B8;
		}
		v30 = *v29;
		if (v30 < 48 || v30 > 57) {
			if (v30 < 97 || v30 > 122) {
				if (v30 < 65 || v30 > 90)
					goto LABEL_53;
				v31 = v30 - 55;
			}
			else {
				v31 = v30 - 87;
			}
		}
		else {
			v31 = v30 - 48;
		}
		v19[v26] = v31;
		if (v31 < (uint8)bDstBase) {
			++v26;
		}
		v25 = (uint8 *)v66;
	LABEL_53:
		v24 = v67;
		v60 = ++v23;
	} while (v23 < v67);
	v63 = v26;
	if (v26) {
		v32 = v26 / 5 + 1;
		v33 = malloc(4 * v32 + 4);
		v59 = v33;
		memset(v33, 0, 4 * v32 + 4);
		v34 = 0;
		if (v63) {
			do {
				switch (v34 % 5) {
				case 1u:
					v35 = (uint8 *)v33 + 4 * (v32 - v34 / 5) - 4;
					*(_DWORD *)v35 += (uint8)bDstBase * lpMem[v34];
					break;
				case 2u:
					v36 = (uint8 *)v33 + 4 * (v32 - v34 / 5) - 4;
					*(_DWORD *)v36 += (uint8)bDstBase * (uint8)bDstBase * lpMem[v34];
					break;
				case 3u:
					v70 = (uint8 *)v33 + 4 * (v32 - v34 / 5) - 4;
					v37 = (uint8)bDstBase;
					v38 = lpMem[v34];
					goto LABEL_62;
				case 4u:
					v70 = (uint8 *)v33 + 4 * (v32 - v34 / 5) - 4;
					v37 = (uint8)bDstBase;
					v38 = (uint8)bDstBase * lpMem[v34];
				LABEL_62:
					*(_DWORD *)v70 += v37 * v37 * v37 * v38;
					break;
				default:
					v33 = v59;
					*((_DWORD *)v59 + v32 - v34 / 5 - 1) = lpMem[v34];
					break;
				}
				++v34;
			} while (v34 < v63);
		}
		v61 = 0;
		if (v32) {
			v64 = (unsigned int *)v33;
			do {
				if (*v64) {
					do {
						if (v61 >= v32 - 1) {
							v43 = v59;
						}
						else {
							v39 = v64;
							v40 = v32 - 1 - v61;
							do {
								v41 = *v39 % bOrigBase;
								v42 = v39[1];
								++v39;
								*v39 = (uint8)bDstBase * (uint8)bDstBase * (uint8)bDstBase *
									(uint8)bDstBase * (uint8)bDstBase * v41 +
									v42;
								--v40;
								*(v39 - 1) /= bOrigBase;
							} while (v40);
							v43 = v59;
						}
						v71 = v68[v43[v32 - 1] % (unsigned int)bOrigBase];
						szDstTmpBuf[nRetIndex++] = v71;
						v43[v32 - 1] /= (unsigned int)bOrigBase;
					} while (*v64);
					v33 = v59;
				}
				v45 = v61++ + 1 < v32;
				++v64;
			} while (v45);
		}
	}
	//����ֱ�Ӳ���dstbuf���п���dstbufδ���
	if (bRetFormatLen != 0) {
		int nDstTmpLen = strlen(szDstTmpBuf);
		if (nDstTmpLen < bRetFormatLen) {
			int nNum = bRetFormatLen - strlen(szDstTmpBuf);
			int i = 0;
			for (i = 0; i < nNum; i++) {
				strcat(szDstTmpBuf, "0");
			}
		}
		else if (nDstTmpLen == bRetFormatLen) {
		}
		else {  //>;
			// 20201106 �ú���Ϊ����nisecת����ģ��˴��޸���Ҫ��Ϊ����Ϲٷ������޸�
			//�˴����ֵ�����������ʵ����Ӱ�죬����۲�
			szDstTmpBuf[nDstTmpLen - 1] = '\0';
			nDstTmpLen -= 1;
			char *pTmp = (char *)calloc(1, bRetFormatLen + 1);
			strncpy(pTmp, szDstTmpBuf + nDstTmpLen - bRetFormatLen, bRetFormatLen);
			strcpy(szDstTmpBuf, pTmp);
			free(pTmp);
		}
	}
	mystrrev(szDstTmpBuf);
	memcpy(szRetBuff, szDstTmpBuf, strlen(szDstTmpBuf));
	if (NULL != v18)
		free(v18);
	if (NULL != v33)
		free(v33);
	return strlen(szRetBuff);
}

// szInOutBuf�������ᱻ�Ķ�����΢������˴���Ļ�������Ҫ�㹻��С
int MemoryFormart(uint8 bOPType, char *szInOutBuf, int nInBufLen)
{
    int i, nRet = -1;
    switch (bOPType) {
        case MF_REMOVE_HEADZERO_LOCAL: {  // string:remove head 0; 00000123->123
            mystrrev(szInOutBuf);
            int nLen = strlen(szInOutBuf);
            if (!nLen)
                return 0;
            for (i = nLen - 1; i > 0; i--) {
                if ('0' == szInOutBuf[i])
                    szInOutBuf[i] = '\0';
                else
                    break;
            }
            mystrrev((char *)szInOutBuf);
            nRet = nLen;
            break;
        }
        case MF_TRANSLATE_SPACE2ZERO_LOCAL: {  //  0x20 -> 0x00
            for (i = 0; i < nInBufLen; i++) {
                if (0x20 == szInOutBuf[i]) {
                    szInOutBuf[i] = 0x00;
                }
            }
            //ȷʵ����Ҫ�ض�
            nRet = strlen(szInOutBuf);
            break;
        }
        case MF_TRANSLATE_ZERO2SPACE_LOCAL: {  // array:0x00 -> 0x20
            for (i = 0; i < nInBufLen; i++) {
                if (0x00 == szInOutBuf[i]) {
                    szInOutBuf[i] = 0x20;
                }
            }
            nRet = strlen(szInOutBuf);
            break;
        }
        case MF_TRANSLATE_ABSOLUTE_LOCAL: {  //�ַ�����ʽ���ȡ����ֵ
            if ('-' != szInOutBuf[0])
                break;
            memmove(szInOutBuf, szInOutBuf + 1, nInBufLen - 1);
            szInOutBuf[nInBufLen - 1] = '\0';
            break;
        }
        case MF_TRANSLATE_RED_LOCAL: {  //����ֵ�ַ�����ʽ���ȡ��Ʊ���
            if ('-' == szInOutBuf[0])
                break;
            if (0 == atof(szInOutBuf))  // 0.00���߷�������ת���ַ� ��ת��
                break;
            memmove(szInOutBuf + 1, szInOutBuf, nInBufLen);
            szInOutBuf[0] = '-';
            break;
        }
        case MF_TRANSLATE_HEADTAILZERO_LOCAL: {
            if (!strlen(szInOutBuf))
                break;
            mytrim(szInOutBuf, ' ');
            break;
        }
		case MF_TRANSLATE_TAILZERO_LOCAL: {  // string:remove tail 0; 0.123000->0.123           
            int nLen = strlen(szInOutBuf);
            if (!nLen)
                return 0;
			if (!strstr(szInOutBuf, "."))
				return 0;
            for (i = nLen - 1; i > 0; i--) {
                if ('0' == szInOutBuf[i])
                    szInOutBuf[i] = '\0';
                else
                    break;
            }
            nRet = nLen;
            break;
        }
        case MF_TRANSLATE_SPACE2STRZERO_LOCAL: {
            for (i = 0; i < nInBufLen; i++) {
                if (0x20 == szInOutBuf[i]) {
                    szInOutBuf[i] = 0x30;
                }
            }
            break;
        }
        default:
            break;
    }
    return nRet;
}

void FillDInfoFromManageData2(HDEV_LOCAL hDev, uint8 *cb, int ncbLen)
{
	char cbHex[768] = { 0 };
	char szReserve[128] = "";  //�����ֶ�
	Byte2Str(cbHex, cb, ncbLen);
	// SN
	memset(hDev->szDeviceID, 0, sizeof hDev->szDeviceID);
	strncpy(hDev->szDeviceID, (char *)cbHex + 6, 12);
	// TaxID
	memset(hDev->szCommonTaxID, 0, sizeof hDev->szCommonTaxID);
	NisecBaseConv((char *)cbHex + 18, 26, 0x10, hDev->szCommonTaxID, 0x24,
		sizeof hDev->szCommonTaxID);
	MemoryFormart(MF_REMOVE_HEADZERO_LOCAL, hDev->szCommonTaxID, strlen(hDev->szCommonTaxID));
	// qysj
	strncpy(hDev->szDeviceEffectDate, cbHex + 44, 8);
	// qylx
	memset(szReserve, 0, sizeof szReserve);
	strncpy(szReserve, cbHex + 58, 2);
	// kpjh
	memset(szReserve, 0, sizeof szReserve);
	strncpy(szReserve, cbHex + 60, 4);
	hDev->uICCardNo = atoi(szReserve);
	// blxx
	// 1.��һ���ֽڱ�ʶ�Ƿ���ũ��Ʒ�����չ���Ʊ��ҵ��00�������ǣ�01�����������ۣ�02���������չ���ҵ��03�������ǡ�
	// 2.�ڶ����ֽڱ�ʶ��·��ҵ���ͣ�00������·�����˰�ˣ�
	//   01����·�ܹ�˾���н�����˰�ˣ�02����ʡ������·��ҵ��˰�ˡ�
	// 3.�������ֽڱ�ʶ��ֵ˰��˰�����ʣ�01:����һ����˰�ˣ�08:����С��ģ��˰��, 05:ת�Ǽ���˰��
	memset(szReserve, 0, sizeof szReserve);
	strncpy(szReserve, cbHex + 64 + 4, 2);
	if (!strcmp(szReserve, "08"))
		hDev->bNatureOfTaxpayer = 1;  //С��ģ��˰��
	else if (!strcmp(szReserve, "05"))
		hDev->bNatureOfTaxpayer = 3;  //ת�Ǽ���˰��
	else if (!strcmp(szReserve, "01"))
		hDev->bNatureOfTaxpayer = 2;  //һ����˰��
	else
		hDev->bNatureOfTaxpayer = 0;  // unknow/˰�ִ���
}

void FillDInfoFromManageData3(HDEV_LOCAL hDev, uint8 *cb, int ncbLen)
{
	char cbHex[768] = { 0 };
	char abBuf[256] = { 0 };
	char szReserve[128] = "";  //�����ֶ�
	Byte2Str(cbHex, cb, ncbLen);
	// swjgdmex
	NisecBaseConv(cbHex + 6, 10, 0x10, (char *)hDev->szTaxAuthorityCodeEx, 0xa, 0xc);
	// szRegCode--�ϱ���ַѡ����ݴ���,�ǹٷ�����ʵ��,��������
	if (!strncmp(hDev->szTaxAuthorityCodeEx, "20", 2)) {  //����4λ,nisecΪ�����ϻ�������
		strncpy(hDev->szRegCode, hDev->szTaxAuthorityCodeEx + 4, 6);
		strcpy(hDev->szTaxAuthorityCode, hDev->szTaxAuthorityCodeEx + 3);
	}
	else {  //����2λ
		strncpy(hDev->szRegCode, hDev->szTaxAuthorityCodeEx + 2, 6);
		strcpy(hDev->szTaxAuthorityCode, hDev->szTaxAuthorityCodeEx + 1);
	}
	// swjgmc
	memset(abBuf, 0, sizeof abBuf);
	memcpy(abBuf, cb + 8, 80);  // 16��160�̶��ֽڳ���
	int nChildRet = MemoryFormart(MF_TRANSLATE_SPACE2ZERO_LOCAL, abBuf, 80);
	//g2u(abBuf, nChildRet, hDev->szTaxAuthorityName, sizeof hDev->szTaxAuthorityName);
	memcpy(hDev->szTaxAuthorityName, abBuf, nChildRet);
	// device customer name
	memset(abBuf, 0, sizeof abBuf);
	memcpy(abBuf, cb + 88, 80);  // 16��160�̶��ֽڳ���
	nChildRet = MemoryFormart(MF_TRANSLATE_SPACE2ZERO_LOCAL, abBuf, 80);
	//g2u(abBuf, nChildRet, hDev->szCompanyName, sizeof hDev->szCompanyName);
	memcpy(hDev->szCompanyName, abBuf, nChildRet);
	// unknow 168 8byte
	// hydm 4710סլ���ݽ���
	memset(abBuf, 0, sizeof abBuf);
	memcpy(abBuf, cb + 176, 80);  // 16��160�̶��ֽڳ���
	nChildRet = MemoryFormart(MF_TRANSLATE_SPACE2ZERO_LOCAL, abBuf, 80);
	//g2u(abBuf, nChildRet, szReserve, sizeof szReserve);
	memcpy(szReserve, abBuf, nChildRet);
}

inline void FillAllInvType_local(HDEV_LOCAL hDev, uint8 *cb)
{
	uint8 bInvTypeCount = *(cb + 1);
	if (!bInvTypeCount || bInvTypeCount > 8)
		return;
	memcpy(hDev->abSupportInvtype, cb + 1, bInvTypeCount + 1);  //�������1+7
}

int GetNisecDeviceVersion(HUSB_LOCAL hUSB, char *szOut)
{
	// 1007180713
	uint8 cb[64];
	int nRet = 0;
	memcpy(cb, "\xfe\x2b", 2);
	nRet = NisecLogicIO_local(hUSB, cb, 2, sizeof cb, NULL, 0);
	if (nRet < 5 || nRet > 8)
		return -1;
	Byte2Str(szOut, cb, nRet);
	return 0;
}

int GetDiskEnvFromManageData_local(HDEV_LOCAL hDev)
{

	HUSB_LOCAL hUSB = hDev->hUSBDevice;
	uint8 cb[512] = { 0 };
	int nRet = -3;
	int nChildRet = -1;
	while (1) {
		// ###manage data
		//\xfe\x06\x01 \xfe\x06\x06 ��Ҫָ��invtype
		// ###manage data2
		memcpy(cb, "\xfe\x06\x02", 3);
		nChildRet = NisecLogicIO_local(hUSB, cb, 3, sizeof cb, NULL, 0);
		if (nChildRet < 32)
			break;  // 0200255399002104910000000bbe2476a3eaad611f85201903110000000000000000010100000000
		FillDInfoFromManageData2(hDev, cb, nChildRet);
		// ###manage data3
		memcpy(cb, "\xfe\x06\x03", 3);
		nChildRet = NisecLogicIO_local(hUSB, cb, 3, sizeof cb, NULL, 0);
		if (nChildRet < 250)
			break;  // 0300fd2e99dea181d6d8c7eccad0b9fabcd2cbb0cef1bed62020202020202020...
		FillDInfoFromManageData3(hDev, cb, nChildRet);
		// ##QueryAllInvoType
		memcpy(cb, "\xfe\x1f", 2);
		nChildRet = NisecLogicIO_local(hUSB, cb, 2, sizeof cb, NULL, 0);
		if (nChildRet < 4)  //����һ����Ʊ����
			break;          // (01 01 04) (02)
		FillAllInvType_local(hDev, cb);
		//����һЩ����Ҫ���¿��ؿ��Ϳ��Ի�ȡ����Ϣ
		if (GetNisecDeviceVersion(hUSB, hDev->szDriverVersion) < 0)
			break;

		nRet = 0;
		break;
	}
	return nRet;
}

// utf8תgbk�����������UTF8��ת��
inline int U2G_UTF8Ignore(char *inbuf, int inlen, char *outbuf, int outlen)
{
	if (!inlen)
		return 0;
#ifdef PROTOCOL_GBK
	return EncordingConvert("utf-8", "gbk", inbuf, inlen, outbuf, outlen);
#else
	if (outlen < inlen)
		return -1;
	memcpy(outbuf, inbuf, inlen);
	return inlen;
#endif
}

// gbkתutf8�����Ĭ�ϱ�����GBK��ת��
inline int G2U_GBKIgnore(char *inbuf, int inlen, char *outbuf, int outlen)
{
	if (!inlen)
		return 0;
#ifdef PROTOCOL_GBK
	if (outlen < inlen)
		return -1;
	memcpy(outbuf, inbuf, inlen);
	return inlen;
#else
	return EncordingConvert("gbk", "utf-8", inbuf, inlen, outbuf, outlen);
#endif
}

int AutoAsciiToChinese(char *pBytes, uint32 nSize, char *pOutBufBytes, uint32 nOutBufSize)
{
	uint8 abUTF8HeadFlag[] = { 0xef, 0xbb, 0xbf };
	int nUTF8FlagSize = sizeof(abUTF8HeadFlag);
	if (nSize > 3 && !memcmp(pBytes, abUTF8HeadFlag, nUTF8FlagSize))  // UTF8�ֽ���
		return U2G_UTF8Ignore(pBytes + nUTF8FlagSize, nSize - nUTF8FlagSize, pOutBufBytes,
		nOutBufSize);
	else  // GBK�ֽ���
		return G2U_GBKIgnore((char *)pBytes, nSize, pOutBufBytes, nOutBufSize);
}

// CntaxFillDInfoFromManageData2
void CntaxGetManageData(HDEV_LOCAL hDev, uint8 *cb, int ncbLen)
{
	char cbHex[768] = { 0 };
	char szReserve[128] = "";  //�����ֶ�
	int nIndex = 0;
	Byte2Str(cbHex, cb, ncbLen);
	// SN
	memset(hDev->szDeviceID, 0, sizeof hDev->szDeviceID);
	strncpy(hDev->szDeviceID, (char *)cbHex + 6, 12);
	//printf("-------CntaxGetManageData hDev->szDeviceID=%s\n",hDev->szDeviceID);
	//�Ǽ�ʱ�� ��qysj ����ʱ��) 14λ��ȡ8λ
	memset(hDev->szDeviceEffectDate, 0, sizeof hDev->szDeviceEffectDate);
	strncpy(hDev->szDeviceEffectDate, cbHex + 18, 8);
	//��ҵ����
	memset(szReserve, 0, sizeof szReserve);
	strncpy(szReserve, cbHex + 32, 2);
	if (!strcmp(szReserve, "08"))
		hDev->bNatureOfTaxpayer = 1;  //С��ģ��˰��
	else if (!strcmp(szReserve, "05"))
		hDev->bNatureOfTaxpayer = 3;  //ת�Ǽ���˰��
	else if (!strcmp(szReserve, "01"))
		hDev->bNatureOfTaxpayer = 2;  //һ����˰��
	else
		hDev->bNatureOfTaxpayer = 0;  // unknow/˰�ִ���
	// kpjh
	memset(szReserve, 0, sizeof szReserve);
	strncpy(szReserve, cbHex + 34, 4);
	hDev->uICCardNo = (uint16)atoll(szReserve);
	// djxh �Ǽ����
	memset(szReserve, 0, sizeof szReserve);
	strncpy(szReserve, cbHex + 38, 20);
	// unknow str2 cbHex + 58-60
	// swjgdmex 12λ
	memset(hDev->szTaxAuthorityCodeEx, 0, sizeof hDev->szTaxAuthorityCodeEx);
	strncpy(hDev->szTaxAuthorityCodeEx, (char *)cb + 30, 12);
	// swjgdm
	memset(hDev->szTaxAuthorityCode, 0, sizeof hDev->szTaxAuthorityCode);
	strcpy(hDev->szTaxAuthorityCode, hDev->szTaxAuthorityCodeEx);
	MemoryFormart(MF_TRANSLATE_SPACE2ZERO_LOCAL, hDev->szTaxAuthorityCode,
		strlen(hDev->szTaxAuthorityCode));
	// regcode
	memset(hDev->szRegCode, 0, sizeof hDev->szRegCode);
	strncpy(hDev->szRegCode, hDev->szTaxAuthorityCodeEx + 1, 6);
	// TaxID
	memset(hDev->szCommonTaxID, 0, sizeof hDev->szCommonTaxID);
	strncpy(hDev->szCommonTaxID, (char *)cb + 42, 20);
	MemoryFormart(MF_TRANSLATE_SPACE2ZERO_LOCAL, hDev->szCommonTaxID, strlen(hDev->szCommonTaxID));
	// swjgmc
	uint8 bNum = *(cb + 62);
	memset(hDev->szTaxAuthorityName, 0, sizeof hDev->szTaxAuthorityName);
	//g2u((char *)cb + 63, bNum, hDev->szTaxAuthorityName, sizeof(hDev->szTaxAuthorityName));
	//memcpy(hDev->szTaxAuthorityName, (char *)cb + 63, bNum);
	AutoAsciiToChinese((char *)cb + 63, bNum, hDev->szTaxAuthorityName,
		sizeof(hDev->szTaxAuthorityName));
	nIndex += 63 + bNum;
	// szCompanyName
	bNum = *(cb + nIndex);
	memset(hDev->szCompanyName, 0, sizeof hDev->szCompanyName);

	//g2u((char *)cb + nIndex + 1, bNum, hDev->szCompanyName, sizeof(hDev->szCompanyName));
	//memcpy(hDev->szCompanyName, (char *)cb + nIndex + 1, bNum);

	AutoAsciiToChinese((char *)cb + nIndex + 1, bNum, hDev->szCompanyName,
		sizeof(hDev->szCompanyName));
	//Ӧ�û��и�ManageData2 3�����Ǻ���ûʲô��Ҫ����
}
// CnTaxGetDiskEnvFromManageData
int CnTaxGetDeviceEnv_local(HDEV_LOCAL hDev, HUSB_LOCAL hUSB)
{

	uint8 abDeviceOpenInfo[64] = { 0 };
	if (CntaxEntryRetInfo_local(hUSB, NOP_OPEN_FULL_LOCAL, abDeviceOpenInfo) < 0)
		return -1;
	uint8 cb[512] = { 0 };
	int nRet = -3;
	int nChildRet = -1;
	while (1) {
		memcpy(cb, "\xfe\x11\x01", 3);
		nChildRet = NisecLogicIO_local(hUSB, cb, 3, sizeof cb, NULL, 0);
		//printf("-----------------CnTaxGetDeviceEnv_local nChildRet = %d\n",nChildRet);
		if (nChildRet < 64)
			break;  // 010089537100494968202001100934350800001...
		CntaxGetManageData(hDev, cb, nChildRet);
		// version
		Byte2Str(hDev->szDriverVersion, abDeviceOpenInfo + 6, 7);
		// fplx support ��ȷ��
		FillAllInvType_local(hDev, abDeviceOpenInfo + 13);
		nRet = 0;
		break;
	}
	if (CntaxEntry_local(hUSB, NOP_CLOSE_LOCAL) < 0)
		return -3;
	return nRet;
}

int base_err_to_errnum(unsigned char err, char *errinfo)
{
	int errnum;
	switch (err)
	{
	case 0xFF:
		tax_interface_out("δ����\n");
		sprintf(errinfo, "δ����");
		errnum = DF_TAX_ERR_CODE_UNSUPPORT_INV_TYPE;
		break;
	case 0xFE:
		tax_interface_out("ʱ���ʽ����\n");
		sprintf(errinfo, "ʱ���ʽ����");
		errnum = DF_TAX_ERR_CODE_UNSUPPORT_INV_TYPE;
		break;
	case 0xFD:
		tax_interface_out("����Ų����Ϲ淶\n");
		sprintf(errinfo, "����Ų����Ϲ淶");
		errnum = DF_TAX_ERR_CODE_UNSUPPORT_INV_TYPE;
		break;
	case 0xFC:
		tax_interface_out("δע��\n");
		sprintf(errinfo, "δע��");
		errnum = DF_TAX_ERR_CODE_UNSUPPORT_INV_TYPE;
		break;
	case 0xFB:
		tax_interface_out("δ����\n");
		sprintf(errinfo, "δ����");
		errnum = DF_TAX_ERR_CODE_UNSUPPORT_INV_TYPE;
		break;
	case 0xFA:
		tax_interface_out("�˿��Ѹ�ʽ�����������ϵ�\n");
		sprintf(errinfo, "�˿��Ѹ�ʽ�����������ϵ�");
		errnum = DF_TAX_ERR_CODE_UNSUPPORT_INV_TYPE;
		break;
	case 0xF9:
		tax_interface_out("��������,�����Ѿ�����\n");
		sprintf(errinfo, "��������,�����Ѿ�����");
		errnum = DF_TAX_ERR_CODE_UNSUPPORT_INV_TYPE;
		break;
	case 0xF8:
		tax_interface_out("�޴˷�Ʊ����\n");
		sprintf(errinfo, "�޴˷�Ʊ����");
		errnum = DF_TAX_ERR_CODE_UNSUPPORT_INV_TYPE;
		break;
	case 0xF7:
		tax_interface_out("�޿��÷�Ʊ\n");
		sprintf(errinfo, "�޿��÷�Ʊ");
		errnum = DF_TAX_ERR_CODE_HAVE_NO_INVOICE;
		break;
	case 0xF6:
		tax_interface_out("�������\n");
		sprintf(errinfo, "�������");
		errnum = DF_TAX_ERR_CODE_PLATE_CERT_PASSWD_ERR;
		break;
	case 0xF5:
		tax_interface_out("����֤�����Ϊ8λ\n");
		sprintf(errinfo, "����֤�����Ϊ8λ");
		errnum = DF_TAX_ERR_CODE_PLATE_CERT_PASSWD_ERR;
		break;
	case 0xF4:
		tax_interface_out("�÷�Ʊ���ʹ����Ѵ���\n");
		sprintf(errinfo, "�÷�Ʊ���ʹ����Ѵ���");
		errnum = DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
		break;
	case 0xF3:
		tax_interface_out("����Ȩ˰���Ѵ���\n");
		sprintf(errinfo, "����Ȩ˰���Ѵ���");
		errnum = DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
		break;
	case 0xF2:
		tax_interface_out("�÷�Ʊ�δ����ظ�\n");
		sprintf(errinfo, "�÷�Ʊ�δ����ظ�");
		errnum = DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
		break;
	case 0xF1:
		tax_interface_out("��Ʊ�������\n");
		sprintf(errinfo, "��Ʊ�������");
		errnum = DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
		break;
	case 0xF0:
		tax_interface_out("�޴˷�Ʊ��\n");
		sprintf(errinfo, "�޴˷�Ʊ��");
		errnum = DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
		break;
	case 0xEF:
		tax_interface_out("������λ�Ǳ�����\n");
		sprintf(errinfo, "������λ�Ǳ�����");
		errnum = DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
		break;
	case 0xEE:
		tax_interface_out("������Ʊ��������\n");
		sprintf(errinfo, "������Ʊ��������");
		errnum = DF_TAX_ERR_CODE_EXCEED_KP_END_DATE;
		break;
	case 0xED:
		tax_interface_out("�������ŷ�Ʊ�޶�\n");
		sprintf(errinfo, "�������ŷ�Ʊ�޶�");
		errnum = DF_TAX_ERR_CODE_EXCEED_SINGLE_LIMIT;
		break;
	case 0xEC:
		tax_interface_out("��������ʱ��\n");
		sprintf(errinfo, "��������ʱ��");
		errnum = DF_TAX_ERR_CODE_EXCEED_OFFLINE_LIMIT_TIME;
		break;
	case 0xEB:
		tax_interface_out("���������ۼƽ��\n");
		sprintf(errinfo, "���������ۼƽ��");
		errnum = DF_TAX_ERR_CODE_EXCEED_OFFLINE_LIMIT;
		break;
	case 0xEA:
		tax_interface_out("�޴���Ȩ˰��\n");
		sprintf(errinfo, "�޴���Ȩ˰��");
		errnum = DF_TAX_ERR_CODE_TAX_RATE_UNSUPPORTED;
		break;
	case 0xE9:
		tax_interface_out("���ݿ��޴˷�Ʊ\n");
		sprintf(errinfo, "���ݿ��޴˷�Ʊ");
		errnum = DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
		break;
	case 0xE8:
		tax_interface_out("δ������ʱ��\n");
		sprintf(errinfo, "δ������ʱ��");
		errnum = DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
		break;
	case 0xE7:
		tax_interface_out("��������Ϣ�ش�\n");
		sprintf(errinfo, "��������Ϣ�ش�");
		errnum = DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
		break;
	case 0xE6:
		tax_interface_out("��¼��������Ʊ��Χ\n");
		sprintf(errinfo, "��¼��������Ʊ��Χ");
		errnum = DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
		break;
	case 0xE5:
		tax_interface_out("��Ʊ������\n");
		sprintf(errinfo, "��Ʊ������");
		errnum = DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
		break;
	case 0xE4:
		tax_interface_out("�������ϸ���Ʊ\n");
		sprintf(errinfo, "�������ϸ���Ʊ");
		errnum = DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
		break;
	case 0xE3:
		tax_interface_out("ϵͳʱ�Ӵ���\n");
		sprintf(errinfo, "ϵͳʱ�Ӵ���");
		errnum = DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
		break;
	case 0xE2:
		tax_interface_out("δ���������ܽ��м����Ϣ�ش�\n");
		sprintf(errinfo, "δ���������ܽ��м����Ϣ�ش�");
		errnum = DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
		break;
	case 0xE1:
		tax_interface_out("�����Ϣ�ش�����Ʊ��������Ӧ����˳��\n");
		sprintf(errinfo, "�����Ϣ�ش�����Ʊ��������Ӧ����˳��");
		errnum = DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
		break;
	case 0xE0:
		tax_interface_out("�����Ϸ�Ʊδ�ϴ������ϴ����Ϸ�Ʊ\n");
		sprintf(errinfo, "�����Ϸ�Ʊδ�ϴ������ϴ����Ϸ�Ʊ");
		errnum = DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
		break;
	case 0xDF:
		tax_interface_out("������Ʊδ�ϴ��������ϴ�����Ʊ\n");
		sprintf(errinfo, "������Ʊδ�ϴ��������ϴ�����Ʊ");
		errnum = DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
		break;
	case 0xDE:
		tax_interface_out("ռδ֧�ָ�����\n");
		sprintf(errinfo, "ռδ֧�ָ�����");
		errnum = DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
		break;
	case 0xDD:
		tax_interface_out("�к�Ʊδ�ϴ�\n");
		sprintf(errinfo, "�к�Ʊδ�ϴ�");
		errnum = DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
		break;
	case 0xDC:
		tax_interface_out("�հ׷ϲ�֧������\n");
		sprintf(errinfo, "�հ׷ϲ�֧������");
		errnum = DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
		break;
	default:
		tax_interface_out("�������δ����\n");
		sprintf(errinfo, "ģ���̴������δ����");
		errnum = DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
		break;
	}
	return errnum;
}

int usb_data_write_read_local(HUSB_LOCAL husb, unsigned char *indata, int in_len, unsigned char *outdata, int *out_len, char *errinfo)
{
	int err_num;
	int result;
	int i;
	int num;
	int size;
	int total_len;
	int left_len;
	int index = 0;
	unsigned char s_data[50];
	unsigned char *r_data;
	uint16 crc_v;
	uint16 crc;

	r_data = (uint8 *)malloc(1000000);
	memset(r_data, 0, (1000000));
	num = in_len / 64 + 1;

	//tax_interface_out("��������,num= %d,in_len=%d\n",num,in_len);
	//tax_interface_out("��������:");
	//printf_array((char *)indata, in_len);
	//char s_time[100] = { 0 };
	//char e_time[100] = { 0 };
	//long start_time;
	//long end_time;
	//start_time = get_time_sec();
	//timer_read_asc(s_time);
	for (i = 0; i < num; i++)
	{
		result = libusb_interrupt_transfer(husb->handle, 0x01, indata + i * 64, 64, &size, 4000);
		if (result < 0)
		{
			tax_interface_out("���ݷ���ʧ��,result = %d\n", result);
			sprintf(errinfo, "ģ���̵ײ�����д�볬ʱʧ��");
			free(r_data);
			return DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
		}

	}
	//tax_interface_out("׼���������ݽ���\n");	
	result = libusb_interrupt_transfer(husb->handle, 0x82, r_data, 64, &size, 2000);
	if (result < 0)
	{
		tax_interface_out("���ݽ���ʧ��,result = %d\n", result);
		sprintf(errinfo, "ģ���̵ײ�����Ӧ��ʱʧ��");
		free(r_data);
		return DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
	}
	if ((r_data[0] != 0x4d) || (r_data[1] != 0x42))
	{
		tax_interface_out("�汾�Ŵ���\n");
		sprintf(errinfo, "ģ���̵ײ�����Ӧ��汾�Ŵ���");
		free(r_data);
		return DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
	}
	total_len = r_data[2] * 0x100 + r_data[3];
	left_len = total_len - 22;
	//tax_interface_out("total_len = %d\n", total_len);
	//tax_interface_out("left_len = %d\n", left_len);
	if (total_len > *out_len)
	{
		tax_interface_out("���ջ���ȥ�ڴ�̫С\n");
		sprintf(errinfo, "ģ���̵ײ����ݽ��ջ�����̫С");
		free(r_data);
		return DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
	}
	if (left_len > 0)
	{
		num = (left_len / 22) + 1;
		//tax_interface_out("num = %d\n", num);
		for (i = 0; i < num; i++)
		{
			memset(s_data, 0, sizeof(s_data));
			libusb_interrupt_transfer(husb->handle, 0x82, s_data, 22, &size, 1000);
			memcpy(r_data + (22 * (i + 1)), s_data, size);
			index += size;
		}
	}
	else
	{
		num = 0;
	}
	crc = crc_8005(r_data, total_len - 2, 0);
	crc_v = r_data[total_len - 2] * 256 + r_data[total_len - 1];
	if (crc != crc_v)
	{
		tax_interface_out("crcУ�����\n");
		sprintf(errinfo, "ģ���̵ײ�Ӧ��CRC����У�����");
		free(r_data);
		return DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
	}
	//tax_interface_out("Ӧ�����ݣ�");
	//printf_array((char *)r_data, total_len);

	if (r_data[4] != 0)
	{
		tax_interface_out("USB����Ӧ��Ϊ�������,%02x\n", r_data[4]);

		err_num = base_err_to_errnum(r_data[4], errinfo);
		free(r_data);
		r_data = NULL;
		return err_num;
	}

	*out_len = total_len;
	memcpy(outdata, r_data + 6, total_len - 8);
	free(r_data);
	r_data = NULL;
	return *out_len;
}

static int pack_s_buf(uint8 order, int s_num, uint8 *in_buf, uint8 *out_buf, int in_len)
{
	uint16 len;
	uint16 crc;
	len = in_len + 8;
	out_buf[0] = 'M'; out_buf[1] = 'B';
	//tax_interface_out("len = %d\n",len);
	out_buf[2] = ((len >> 8) & 0xff); out_buf[3] = (len & 0xff);
	out_buf[4] = order;
	out_buf[5] = s_num;
	memcpy(out_buf + 6, in_buf, in_len);
	crc = crc_8005(out_buf, len - 2, 0);
	out_buf[len - 2] = ((crc >> 8) & 0xff);
	out_buf[len - 1] = (crc & 0xff);
	return (int)len;
}



int EncordingConvert(char *from_charset, char *to_charset, char *inbuf, size_t inlen, char *outbuf,
	size_t outlen)
{
	int nOutBuffOrigLen = outlen;
	iconv_t cd;
	char **pin = &inbuf;
	char **pout = &outbuf;
	cd = iconv_open(to_charset, from_charset);
	if (cd == 0)
		return -1;
	memset(outbuf, 0, outlen);
	int nRet = iconv(cd, pin, (size_t *)&inlen, pout, (size_t *)&outlen);
	iconv_close(cd);
	if (nRet == -1)
		return -1;
	nRet = nOutBuffOrigLen - outlen;
	return nRet;
}

int LoadAisinoDevInfo_local(HDEV_LOCAL hDev, HUSB_LOCAL device, uint8 bAuthDev)
{
    int nRet = -1, nChildRet = 0;
    hDev->hUSBDevice = device;
    hDev->bDeviceType = DEVICE_AISINO_LOCAL;

    //�豸���SN��������IO�ж�
    nChildRet = ReadTaxDeviceID_local(device, hDev->szDeviceID);
    if (nChildRet > 0)
        hDev->bCryptBigIO = 1;
    else if (0 == nChildRet)
        hDev->bCryptBigIO = 0;
    else {  //< 0
        nRet = -6;
        goto LoadAisinoDevInfo_Finish;
    }
    //ͨ����ʾ˰�� ˰�̹�˾����
    if ((nRet = AisinoReadTaxIDandCustomName_local(device, hDev->szCommonTaxID, hDev->szCompanyName,
                                     sizeof(hDev->szCompanyName))) < 0) {
        nRet = -7;
        goto LoadAisinoDevInfo_Finish;
    }
LoadAisinoDevInfo_Finish:
    if (nRet) {
        tax_interface_out("LoadDevInfo failed,nRet = %d\n", nRet);
    } else {
        //_WriteLog(
        //    LL_DEBUG,
        //    "DevInfo, region:%s devid:%s taxidstr:%s szCompressTaxID:%s taxid9b:%s cusname:%s",
        //    hDev->szRegCode, hDev->szDeviceID, hDev->szCommonTaxID, hDev->szCompressTaxID,
        //    hDev->sz9ByteHashTaxID, hDev->szCompanyName);
    }
    return nRet;
}

//���Ź�ѡ˰��֧��

int LoadAisinoSelectDevInfo_local(HDEV_LOCAL hDev, HUSB_LOCAL device, uint8 bAuthDev)
{
	int nRet = -1;
	hDev->hUSBDevice = device;
	hDev->bDeviceType = DEVICE_AISINO_LOCAL;

	

	//tax_interface_out("ѹ��˰�ţ�%s��hash˰�ţ�%s\n", hDev->szCompressTaxID, hDev->sz9ByteHashTaxID);
	//�ֻ�����
	//hDev->uICCardNo = GetICCardNo(device);
	////�������
	//if (GetRegionCode(device, hDev->szRegCode) < 0) {
	//	nRet = -4;
	//	goto LoadAisinoDevInfo_Finish;
	//}
	//if (AisinoGetDeivceTime(device, hDev->szDeviceTime) < 0) {
	//	nRet = -5;
	//	goto LoadAisinoDevInfo_Finish;
	//}
	//�豸���SN
	if (ReadTaxDeviceID_local(device, hDev->szDeviceID) < 0) {
		nRet = -6;
		goto LoadAisinoDevInfo_Finish;
	}
	tax_interface_out("��һ�λ�ȡ���Ļ�����ţ�%s\n", hDev->szDeviceID);




	uint8 cb[51200] = "";
	char data[512] = "";
	int nRetChild;
	////tax_interface_out("��1�ν���\n");
	//memset(cb, 0, sizeof(cb));
	//memset(data, 0, sizeof(data));
	//memcpy(cb, "\xf1\xf1\x00\x00\x00\x00\x02\x08\x00\xff", 10);
	//memcpy(data, "\xb5\xf0\xaa\x1d\x69\x10\x51\x61", 8);
	//int nRetChild = mass_storage_io_local(device, (uint8 *)cb, 10, 0, MSC_DIR_H2D, (uint8 *)data,	8, (uint8 *)cb, sizeof(cb));
	//if (nRetChild != 0)
	//	return -1;
	//memcpy(cb, "\xf1\xf1\x00\x00\x00\x00\x01\x04\x01\xff", 10);
	//nRetChild =	mass_storage_io_local(device, (uint8 *)cb, 10, 0, MSC_DIR_D2H, NULL, 0, (uint8 *)cb, sizeof(cb));
	//if (nRetChild < 0)
	//	return -2;
	//char *pRawID = cb + 0x12;
	//pRawID[12] = '\0';
	//printf_array(cb, 200);
	//tax_interface_out("�ڶ��λ�ȡ���Ļ�����ţ�%s\n", pRawID);

	//tax_interface_out("��2�ν���\n");
	memset(cb, 0, sizeof(cb));
	memset(data, 0, sizeof(data));
	memcpy(cb, "\xf1\x00\x00\x00\x00\x00\x02\x08\x00\xff", 10);
	memcpy(data, "\x00\x05\x00\x00\xac\x06\x00\x04", 8);
	nRetChild = mass_storage_io_local(device, (uint8 *)cb, 10, 0, MSC_DIR_H2D, (uint8 *)data, 8, (uint8 *)cb, sizeof(cb));
	if (nRetChild != 0)
		return -3;
	memset(cb, 0, sizeof(cb));
	memcpy(cb, "\xf1\x00\x00\x00\x00\x00\x01\xfc\x08\xff", 10);
	nRetChild = mass_storage_io_local(device, (uint8 *)cb, 10, 0, MSC_DIR_D2H, NULL, 0, (uint8 *)cb, sizeof(cb));
	if (nRetChild < 0)
		return -4;
	//printf_array(cb,200);

	//tax_interface_out("��3�ν���\n");
	memset(cb, 0, sizeof(cb));
	memset(data, 0, sizeof(data));
	memcpy(cb, "\xf1\x00\x00\x00\x00\x00\x02\x0a\x00\xff", 10);
	memcpy(data, "\x00\x07\x00\x00\xa4\x00\x00\x02\x4f\x01", 10);
	nRetChild = mass_storage_io_local(device, (uint8 *)cb, 10, 0, MSC_DIR_H2D, (uint8 *)data, 10, (uint8 *)cb, sizeof(cb));
	if (nRetChild != 0)
		return -5;
	memset(cb, 0, sizeof(cb));
	memcpy(cb, "\xf1\x00\x00\x00\x00\x00\x01\xfc\x08\xff", 10);
	nRetChild = mass_storage_io_local(device, (uint8 *)cb, 10, 0, MSC_DIR_D2H, NULL, 0, (uint8 *)cb, sizeof(cb));
	if (nRetChild < 0)
		return -6;
	//printf_array(cb, 200);


	//tax_interface_out("��4�ν���\n");
	memset(cb, 0, sizeof(cb));
	memset(data, 0, sizeof(data));
	memcpy(cb, "\xf1\x00\x00\x00\x00\x00\x02\x08\x00\xff", 10);
	memcpy(data, "\x00\x05\x00\x00\xac\x02\x00\x04", 8);
	nRetChild = mass_storage_io_local(device, (uint8 *)cb, 10, 0, MSC_DIR_H2D, (uint8 *)data, 8, (uint8 *)cb, sizeof(cb));
	if (nRetChild != 0)
		return -5;
	memset(cb, 0, sizeof(cb));
	memcpy(cb, "\xf1\x00\x00\x00\x00\x00\x01\xfc\x08\xff", 10);
	nRetChild = mass_storage_io_local(device, (uint8 *)cb, 10, 0, MSC_DIR_D2H, NULL, 0, (uint8 *)cb, sizeof(cb));
	if (nRetChild < 0)
		return -6;
	//printf_array(cb, 200);


	//tax_interface_out("��5�ν���\n");
	memset(cb, 0, sizeof(cb));
	memset(data, 0, sizeof(data));
	memcpy(cb, "\xf1\x00\x00\x00\x00\x00\x02\x08\x00\xff", 10);
	memcpy(data, "\x00\x05\x00\x00\xac\x00\x01\x01", 8);
	nRetChild = mass_storage_io_local(device, (uint8 *)cb, 10, 0, MSC_DIR_H2D, (uint8 *)data, 8, (uint8 *)cb, sizeof(cb));
	if (nRetChild != 0)
		return -5;
	memset(cb, 0, sizeof(cb));
	memcpy(cb, "\xf1\x00\x00\x00\x00\x00\x01\xfc\x08\xff", 10);
	nRetChild = mass_storage_io_local(device, (uint8 *)cb, 10, 0, MSC_DIR_D2H, NULL, 0, (uint8 *)cb, sizeof(cb));
	if (nRetChild < 0)
		return -6;
	//printf_array(cb, 200);



	//tax_interface_out("��6�ν���\n");
	memset(cb, 0, sizeof(cb));
	memset(data, 0, sizeof(data));
	memcpy(cb, "\xf1\x00\x00\x00\x00\x00\x02\x08\x00\xff", 10);
	memcpy(data, "\x00\x05\x00\x00\x84\x00\x00\x08", 8);
	nRetChild = mass_storage_io_local(device, (uint8 *)cb, 10, 0, MSC_DIR_H2D, (uint8 *)data, 8, (uint8 *)cb, sizeof(cb));
	if (nRetChild != 0)
		return -5;
	memset(cb, 0, sizeof(cb));
	memcpy(cb, "\xf1\x00\x00\x00\x00\x00\x01\xfc\x08\xff", 10);
	nRetChild = mass_storage_io_local(device, (uint8 *)cb, 10, 0, MSC_DIR_D2H, NULL, 0, (uint8 *)cb, sizeof(cb));
	if (nRetChild < 0)
		return -6;
	//printf_array(cb, 200);

	//tax_interface_out("��7�ν���\n");
	memset(cb, 0, sizeof(cb));
	memset(data, 0, sizeof(data));
	memcpy(cb, "\xf1\x00\x00\x00\x00\x00\x02\x0a\x00\xff", 10);
	memcpy(data, "\x00\x07\x00\x00\xa4\x00\x00\x02\x3f\x00", 10);
	nRetChild = mass_storage_io_local(device, (uint8 *)cb, 10, 0, MSC_DIR_H2D, (uint8 *)data, 10, (uint8 *)cb, sizeof(cb));
	if (nRetChild != 0)
		return -5;
	memset(cb, 0, sizeof(cb));
	memcpy(cb, "\xf1\x00\x00\x00\x00\x00\x01\xfc\x08\xff", 10);
	nRetChild = mass_storage_io_local(device, (uint8 *)cb, 10, 0, MSC_DIR_D2H, NULL, 0, (uint8 *)cb, sizeof(cb));
	if (nRetChild < 0)
		return -6;
	//printf_array(cb, 200);


	//tax_interface_out("��8�ν���\n");
	memset(cb, 0, sizeof(cb));
	memset(data, 0, sizeof(data));
	memcpy(cb, "\xf1\x00\x00\x00\x00\x00\x02\x0a\x00\xff", 10);
	memcpy(data, "\x00\x07\x00\x00\xa4\x00\x00\x02\x4d\x01", 10);
	nRetChild = mass_storage_io_local(device, (uint8 *)cb, 10, 0, MSC_DIR_H2D, (uint8 *)data, 10, (uint8 *)cb, sizeof(cb));
	if (nRetChild != 0)
		return -5;
	memset(cb, 0, sizeof(cb));
	memcpy(cb, "\xf1\x00\x00\x00\x00\x00\x01\xfc\x08\xff", 10);
	nRetChild = mass_storage_io_local(device, (uint8 *)cb, 10, 0, MSC_DIR_D2H, NULL, 0, (uint8 *)cb, sizeof(cb));
	if (nRetChild < 0)
		return -6;
	//printf_array(cb, 200);


	//tax_interface_out("��9�ν���\n");
	memset(cb, 0, sizeof(cb));
	memset(data, 0, sizeof(data));
	memcpy(cb, "\xf1\x00\x00\x00\x00\x00\x02\x09\x00\xff", 10);
	memcpy(data, "\x00\x06\x00\x00\xb0\x00\x00\x00\x20", 9);
	nRetChild = mass_storage_io_local(device, (uint8 *)cb, 10, 0, MSC_DIR_H2D, (uint8 *)data, 9, (uint8 *)cb, sizeof(cb));
	if (nRetChild != 0)
		return -5;
	memset(cb, 0, sizeof(cb));
	memcpy(cb, "\xf1\x00\x00\x00\x00\x00\x01\xfc\x08\xff", 10);
	nRetChild = mass_storage_io_local(device, (uint8 *)cb, 10, 0, MSC_DIR_D2H, NULL, 0, (uint8 *)cb, sizeof(cb));
	if (nRetChild < 0)
		return -6;
	//printf_array(cb, 200);





	//tax_interface_out("��10�ν���\n");
	memset(cb, 0, sizeof(cb));
	memset(data, 0, sizeof(data));
	memcpy(cb, "\xf1\x00\x00\x00\x00\x00\x02\x0a\x00\xff", 10);
	memcpy(data, "\x00\x07\x00\x00\xa4\x00\x00\x02\x4f\x01", 10);
	nRetChild = mass_storage_io_local(device, (uint8 *)cb, 10, 0, MSC_DIR_H2D, (uint8 *)data, 10, (uint8 *)cb, sizeof(cb));
	if (nRetChild != 0)
		return -5;
	memset(cb, 0, sizeof(cb));
	memcpy(cb, "\xf1\x00\x00\x00\x00\x00\x01\xfc\x08\xff", 10);
	nRetChild = mass_storage_io_local(device, (uint8 *)cb, 10, 0, MSC_DIR_D2H, NULL, 0, (uint8 *)cb, sizeof(cb));
	if (nRetChild < 0)
		return -6;
	//tax_interface_out("�������ݳ���Ϊ%d,����Ϊ��", nRetChild);
	//printf_array(cb, nRetChild);


	//tax_interface_out("��11�ν���\n");
	memset(cb, 0, sizeof(cb));
	memset(data, 0, sizeof(data));
	memcpy(cb, "\xf1\x00\x00\x00\x00\x00\x02\x0a\x00\xff", 10);
	memcpy(data, "\x00\x07\x00\x00\xa4\x00\x00\x02\x4e\x01", 10);
	nRetChild = mass_storage_io_local(device, (uint8 *)cb, 10, 0, MSC_DIR_H2D, (uint8 *)data, 10, (uint8 *)cb, sizeof(cb));
	if (nRetChild != 0)
		return -5;
	memset(cb, 0, sizeof(cb));
	memcpy(cb, "\xf1\x00\x00\x00\x00\x00\x01\xfc\x08\xff", 10);
	nRetChild = mass_storage_io_local(device, (uint8 *)cb, 10, 0, MSC_DIR_D2H, NULL, 0, (uint8 *)cb, sizeof(cb));
	if (nRetChild < 0)
		return -6;
	//tax_interface_out("�������ݳ���Ϊ%d,����Ϊ��", nRetChild);
	//printf_array(cb, nRetChild);

	//tax_interface_out("��12�ν���\n");
	memset(cb, 0, sizeof(cb));
	memset(data, 0, sizeof(data));
	memcpy(cb, "\xf1\x00\x00\x00\x00\x00\x02\x09\x00\xff", 10);
	memcpy(data, "\x00\x06\x00\x00\xb0\x00\x00\x00\x01", 9);
	nRetChild = mass_storage_io_local(device, (uint8 *)cb, 10, 0, MSC_DIR_H2D, (uint8 *)data, 9, (uint8 *)cb, sizeof(cb));
	if (nRetChild != 0)
		return -5;
	memset(cb, 0, sizeof(cb));
	memcpy(cb, "\xf1\x00\x00\x00\x00\x00\x01\xfc\x08\xff", 10);
	nRetChild = mass_storage_io_local(device, (uint8 *)cb, 10, 0, MSC_DIR_D2H, NULL, 0, (uint8 *)cb, sizeof(cb));
	if (nRetChild < 0)
		return -6;
	//tax_interface_out("�������ݳ���Ϊ%d,����Ϊ��", nRetChild);
	//printf_array(cb, nRetChild);

	//tax_interface_out("��13�ν���\n");
	memset(cb, 0, sizeof(cb));
	memset(data, 0, sizeof(data));
	memcpy(cb, "\xf1\x00\x00\x00\x00\x00\x02\x09\x00\xff", 10);
	memcpy(data, "\x00\x06\x00\x00\xb0\x00\x00\x00\x40", 9);
	nRetChild = mass_storage_io_local(device, (uint8 *)cb, 10, 0, MSC_DIR_H2D, (uint8 *)data, 9, (uint8 *)cb, sizeof(cb));
	if (nRetChild != 0)
		return -5;
	memset(cb, 0, sizeof(cb));
	memcpy(cb, "\xf1\x00\x00\x00\x00\x00\x01\xfc\x08\xff", 10);
	nRetChild = mass_storage_io_local(device, (uint8 *)cb, 10, 0, MSC_DIR_D2H, NULL, 0, (uint8 *)cb, sizeof(cb));
	if (nRetChild < 0)
		return -6;
	//tax_interface_out("�������ݳ���Ϊ%d,����Ϊ��", nRetChild);
	//printf_array(cb, nRetChild);


	//tax_interface_out("��14�ν���\n");
	memset(cb, 0, sizeof(cb));
	memset(data, 0, sizeof(data));
	memcpy(cb, "\xf1\x00\x00\x00\x00\x00\x02\x0a\x00\xff", 10);
	memcpy(data, "\x00\x07\x00\x00\xa4\x00\x00\x02\x4e\x02", 10);
	nRetChild = mass_storage_io_local(device, (uint8 *)cb, 10, 0, MSC_DIR_H2D, (uint8 *)data, 10, (uint8 *)cb, sizeof(cb));
	if (nRetChild != 0)
		return -5;
	memset(cb, 0, sizeof(cb));
	memcpy(cb, "\xf1\x00\x00\x00\x00\x00\x01\xfc\x08\xff", 10);
	nRetChild = mass_storage_io_local(device, (uint8 *)cb, 10, 0, MSC_DIR_D2H, NULL, 0, (uint8 *)cb, sizeof(cb));
	if (nRetChild < 0)
		return -6;
	//tax_interface_out("�������ݳ���Ϊ%d,����Ϊ��", nRetChild);
	//printf_array(cb, nRetChild);


	//tax_interface_out("��15�ν���\n");
	memset(cb, 0, sizeof(cb));
	memset(data, 0, sizeof(data));
	memcpy(cb, "\xf1\x00\x00\x00\x00\x00\x02\x09\x00\xff", 10);
	memcpy(data, "\x00\x06\x00\x00\xb0\x00\x00\x04\x80", 9);
	nRetChild = mass_storage_io_local(device, (uint8 *)cb, 10, 0, MSC_DIR_H2D, (uint8 *)data, 9, (uint8 *)cb, sizeof(cb));
	if (nRetChild != 0)
		return -5;
	memset(cb, 0, sizeof(cb));
	memcpy(cb, "\xf1\x00\x00\x00\x00\x00\x01\xfc\x08\xff", 10);
	nRetChild = mass_storage_io_local(device, (uint8 *)cb, 10, 0, MSC_DIR_D2H, NULL, 0, (uint8 *)cb, sizeof(cb));
	if (nRetChild < 0)
		return -6;
	//tax_interface_out("�������ݳ���Ϊ%d,����Ϊ��", nRetChild);
	//printf_array(cb, nRetChild);

	//tax_interface_out("��16�ν���\n");
	memset(cb, 0, sizeof(cb));
	memset(data, 0, sizeof(data));
	memcpy(cb, "\xf1\x00\x00\x00\x00\x00\x02\x0a\x00\xff", 10);
	memcpy(data, "\x00\x07\x00\x00\xa4\x00\x00\x02\x4f\x01", 10);
	nRetChild = mass_storage_io_local(device, (uint8 *)cb, 10, 0, MSC_DIR_H2D, (uint8 *)data, 10, (uint8 *)cb, sizeof(cb));
	if (nRetChild != 0)
		return -5;
	memset(cb, 0, sizeof(cb));
	memcpy(cb, "\xf1\x00\x00\x00\x00\x00\x01\xfc\x08\xff", 10);
	nRetChild = mass_storage_io_local(device, (uint8 *)cb, 10, 0, MSC_DIR_D2H, NULL, 0, (uint8 *)cb, sizeof(cb));
	if (nRetChild < 0)
		return -6;
	//tax_interface_out("�������ݳ���Ϊ%d,����Ϊ��", nRetChild);
	//printf_array(cb, nRetChild);

	//tax_interface_out("��17�ν���\n");
	memset(cb, 0, sizeof(cb));
	memset(data, 0, sizeof(data));
	memcpy(cb, "\xf1\x00\x00\x00\x00\x00\x02\x0a\x00\xff", 10);
	memcpy(data, "\x00\x07\x00\x00\xa4\x00\x00\x02\x50\x01", 10);
	nRetChild = mass_storage_io_local(device, (uint8 *)cb, 10, 0, MSC_DIR_H2D, (uint8 *)data, 10, (uint8 *)cb, sizeof(cb));
	if (nRetChild != 0)
		return -5;
	memset(cb, 0, sizeof(cb));
	memcpy(cb, "\xf1\x00\x00\x00\x00\x00\x01\xfc\x08\xff", 10);
	nRetChild = mass_storage_io_local(device, (uint8 *)cb, 10, 0, MSC_DIR_D2H, NULL, 0, (uint8 *)cb, sizeof(cb));
	if (nRetChild < 0)
		return -6;
	//tax_interface_out("�������ݳ���Ϊ%d,����Ϊ��", nRetChild);
	//printf_array(cb, nRetChild);

	//tax_interface_out("��18�ν���\n");
	memset(cb, 0, sizeof(cb));
	memset(data, 0, sizeof(data));
	memcpy(cb, "\xf1\x00\x00\x00\x00\x00\x02\x09\x00\xff", 10);
	memcpy(data, "\x00\x06\x00\x00\xb0\x00\x00\x00\x92", 9);
	nRetChild = mass_storage_io_local(device, (uint8 *)cb, 10, 0, MSC_DIR_H2D, (uint8 *)data, 9, (uint8 *)cb, sizeof(cb));
	if (nRetChild != 0)
		return -5;
	memset(cb, 0, sizeof(cb));
	memcpy(cb, "\xf1\x00\x00\x00\x00\x00\x01\xfc\x08\xff", 10);
	nRetChild = mass_storage_io_local(device, (uint8 *)cb, 10, 0, MSC_DIR_D2H, NULL, 0, (uint8 *)cb, sizeof(cb));
	if (nRetChild < 0)
		return -6;
	//tax_interface_out("�������ݳ���Ϊ%d,����Ϊ��", nRetChild);
	//printf_array(cb, nRetChild);

	//tax_interface_out("��19�ν���\n");
	memset(cb, 0, sizeof(cb));
	memset(data, 0, sizeof(data));
	memcpy(cb, "\xf1\x00\x00\x00\x00\x00\x02\x0a\x00\xff", 10);
	memcpy(data, "\x00\x07\x00\x00\xa4\x00\x00\x02\x4f\x01", 10);
	nRetChild = mass_storage_io_local(device, (uint8 *)cb, 10, 0, MSC_DIR_H2D, (uint8 *)data, 10, (uint8 *)cb, sizeof(cb));
	if (nRetChild != 0)
		return -5;
	memset(cb, 0, sizeof(cb));
	memcpy(cb, "\xf1\x00\x00\x00\x00\x00\x01\xfc\x08\xff", 10);
	nRetChild = mass_storage_io_local(device, (uint8 *)cb, 10, 0, MSC_DIR_D2H, NULL, 0, (uint8 *)cb, sizeof(cb));
	if (nRetChild < 0)
		return -6;
	//tax_interface_out("�������ݳ���Ϊ%d,����Ϊ��", nRetChild);
	//printf_array(cb, nRetChild);

	//tax_interface_out("��20�ν���\n");
	memset(cb, 0, sizeof(cb));
	memset(data, 0, sizeof(data));
	memcpy(cb, "\xf1\x00\x00\x00\x00\x00\x02\x0a\x00\xff", 10);
	memcpy(data, "\x00\x07\x00\x00\xa4\x00\x00\x02\x50\x00", 10);
	nRetChild = mass_storage_io_local(device, (uint8 *)cb, 10, 0, MSC_DIR_H2D, (uint8 *)data, 10, (uint8 *)cb, sizeof(cb));
	if (nRetChild != 0)
		return -5;
	memset(cb, 0, sizeof(cb));
	memcpy(cb, "\xf1\x00\x00\x00\x00\x00\x01\xfc\x08\xff", 10);
	nRetChild = mass_storage_io_local(device, (uint8 *)cb, 10, 0, MSC_DIR_D2H, NULL, 0, (uint8 *)cb, sizeof(cb));
	if (nRetChild < 0)
		return -6;
	//tax_interface_out("�������ݳ���Ϊ%d,����Ϊ��", nRetChild);
	//printf_array(cb, nRetChild);

	//tax_interface_out("��21�ν���\n");
	memset(cb, 0, sizeof(cb));
	memset(data, 0, sizeof(data));
	memcpy(cb, "\xf1\x00\x00\x00\x00\x00\x02\x09\x00\xff", 10);
	memcpy(data, "\x00\x06\x00\x00\xb0\x00\x00\x00\xb4", 9);
	nRetChild = mass_storage_io_local(device, (uint8 *)cb, 10, 0, MSC_DIR_H2D, (uint8 *)data, 9, (uint8 *)cb, sizeof(cb));
	if (nRetChild != 0)
		return -5;
	memset(cb, 0, sizeof(cb));
	memcpy(cb, "\xf1\x00\x00\x00\x00\x00\x01\xfc\x08\xff", 10);
	nRetChild = mass_storage_io_local(device, (uint8 *)cb, 10, 0, MSC_DIR_D2H, NULL, 0, (uint8 *)cb, sizeof(cb));
	if (nRetChild < 0)
		return -6;
	//tax_interface_out("�������ݳ���Ϊ%d,����Ϊ��", nRetChild);
	//printf_array(cb, nRetChild);


	//tax_interface_out("��22�ν���\n");
	memset(cb, 0, sizeof(cb));
	memset(data, 0, sizeof(data));
	memcpy(cb, "\xf1\x00\x00\x00\x00\x00\x02\x0a\x00\xff", 10);
	memcpy(data, "\x00\x07\x00\x00\xa4\x00\x00\x02\x4f\x01", 10);
	nRetChild = mass_storage_io_local(device, (uint8 *)cb, 10, 0, MSC_DIR_H2D, (uint8 *)data, 10, (uint8 *)cb, sizeof(cb));
	if (nRetChild != 0)
		return -5;
	memset(cb, 0, sizeof(cb));
	memcpy(cb, "\xf1\x00\x00\x00\x00\x00\x01\xfc\x08\xff", 10);
	nRetChild = mass_storage_io_local(device, (uint8 *)cb, 10, 0, MSC_DIR_D2H, NULL, 0, (uint8 *)cb, sizeof(cb));
	if (nRetChild < 0)
		return -6;
	//tax_interface_out("�������ݳ���Ϊ%d,����Ϊ��", nRetChild);
	//printf_array(cb, nRetChild);


	//tax_interface_out("��23�ν���\n");
	memset(cb, 0, sizeof(cb));
	memset(data, 0, sizeof(data));
	memcpy(cb, "\xf1\x00\x00\x00\x00\x00\x02\x0a\x00\xff", 10);
	memcpy(data, "\x00\x07\x00\x00\xa4\x00\x00\x02\x50\x04", 10);
	nRetChild = mass_storage_io_local(device, (uint8 *)cb, 10, 0, MSC_DIR_H2D, (uint8 *)data, 10, (uint8 *)cb, sizeof(cb));
	if (nRetChild != 0)
		return -5;
	memset(cb, 0, sizeof(cb));
	memcpy(cb, "\xf1\x00\x00\x00\x00\x00\x01\xfc\x08\xff", 10);
	nRetChild = mass_storage_io_local(device, (uint8 *)cb, 10, 0, MSC_DIR_D2H, NULL, 0, (uint8 *)cb, sizeof(cb));
	if (nRetChild < 0)
		return -6;
	//tax_interface_out("�������ݳ���Ϊ%d,����Ϊ��", nRetChild);
	//printf_array(cb, nRetChild);


	//tax_interface_out("��24�ν���\n");
	memset(cb, 0, sizeof(cb));
	memset(data, 0, sizeof(data));
	memcpy(cb, "\xf1\x00\x00\x00\x00\x00\x02\x09\x00\xff", 10);
	memcpy(data, "\x00\x06\x00\x00\xb0\x00\x00\x04\xef", 9);
	nRetChild = mass_storage_io_local(device, (uint8 *)cb, 10, 0, MSC_DIR_H2D, (uint8 *)data, 9, (uint8 *)cb, sizeof(cb));
	if (nRetChild != 0)
		return -5;
	memset(cb, 0, sizeof(cb));
	memcpy(cb, "\xf1\x00\x00\x00\x00\x00\x01\xfc\x08\xff", 10);
	nRetChild = mass_storage_io_local(device, (uint8 *)cb, 10, 0, MSC_DIR_D2H, NULL, 0, (uint8 *)cb, sizeof(cb));
	if (nRetChild < 0)
		return -6;
	/*tax_interface_out("�������ݳ���Ϊ%d,����Ϊ��", nRetChild);
	printf_array(cb, nRetChild);*/

	//tax_interface_out("��25�ν���\n");
	memset(cb, 0, sizeof(cb));
	memset(data, 0, sizeof(data));
	memcpy(cb, "\xf1\x00\x00\x00\x00\x00\x02\x0a\x00\xff", 10);
	memcpy(data, "\x00\x07\x00\x00\xa4\x00\x00\x02\x4f\x01", 10);
	nRetChild = mass_storage_io_local(device, (uint8 *)cb, 10, 0, MSC_DIR_H2D, (uint8 *)data, 10, (uint8 *)cb, sizeof(cb));
	if (nRetChild != 0)
		return -5;
	memset(cb, 0, sizeof(cb));
	memcpy(cb, "\xf1\x00\x00\x00\x00\x00\x01\xfc\x08\xff", 10);
	nRetChild = mass_storage_io_local(device, (uint8 *)cb, 10, 0, MSC_DIR_D2H, NULL, 0, (uint8 *)cb, sizeof(cb));
	if (nRetChild < 0)
		return -6;
	/*tax_interface_out("�������ݳ���Ϊ%d,����Ϊ��", nRetChild);
	printf_array(cb, nRetChild);*/



	//tax_interface_out("��26�ν���\n");
	memset(cb, 0, sizeof(cb));
	memset(data, 0, sizeof(data));
	memcpy(cb, "\xf1\x00\x00\x00\x00\x00\x02\x0a\x00\xff", 10);
	memcpy(data, "\x00\x07\x00\x00\xa4\x00\x00\x02\x50\x08", 10);
	nRetChild = mass_storage_io_local(device, (uint8 *)cb, 10, 0, MSC_DIR_H2D, (uint8 *)data, 10, (uint8 *)cb, sizeof(cb));
	if (nRetChild != 0)
		return -5;
	memset(cb, 0, sizeof(cb));
	memcpy(cb, "\xf1\x00\x00\x00\x00\x00\x01\xfc\x08\xff", 10);
	nRetChild = mass_storage_io_local(device, (uint8 *)cb, 10, 0, MSC_DIR_D2H, NULL, 0, (uint8 *)cb, sizeof(cb));
	if (nRetChild < 0)
		return -6;
	//tax_interface_out("�������ݳ���Ϊ%d,����Ϊ��", nRetChild);
	//printf_array(cb, nRetChild);


	//tax_interface_out("��27�ν���\n");
	memset(cb, 0, sizeof(cb));
	memset(data, 0, sizeof(data));
	memcpy(cb, "\xf1\x00\x00\x00\x00\x00\x02\x0a\x00\xff", 10);
	memcpy(data, "\x00\x07\x00\x00\xa4\x00\x00\x02\x50\x08", 10);
	nRetChild = mass_storage_io_local(device, (uint8 *)cb, 10, 0, MSC_DIR_H2D, (uint8 *)data, 10, (uint8 *)cb, sizeof(cb));
	if (nRetChild != 0)
		return -5;
	memset(cb, 0, sizeof(cb));
	memcpy(cb, "\xf1\x00\x00\x00\x00\x00\x01\xfc\x08\xff", 10);
	nRetChild = mass_storage_io_local(device, (uint8 *)cb, 10, 0, MSC_DIR_D2H, NULL, 0, (uint8 *)cb, sizeof(cb));
	if (nRetChild < 0)
		return -6;
	//tax_interface_out("�������ݳ���Ϊ%d,����Ϊ��", nRetChild);
	//printf_array(cb, nRetChild);


	//tax_interface_out("��28�ν���\n");
	memset(cb, 0, sizeof(cb));
	memset(data, 0, sizeof(data));
	memcpy(cb, "\xf1\x00\x00\x00\x00\x00\x02\x09\x00\xff", 10);
	memcpy(data, "\x00\x06\x00\x00\xb0\x00\x00\x00\x92", 9);
	nRetChild = mass_storage_io_local(device, (uint8 *)cb, 10, 0, MSC_DIR_H2D, (uint8 *)data, 9, (uint8 *)cb, sizeof(cb));
	if (nRetChild != 0)
		return -5;
	memset(cb, 0, sizeof(cb));
	memcpy(cb, "\xf1\x00\x00\x00\x00\x00\x01\xfc\x08\xff", 10);
	nRetChild = mass_storage_io_local(device, (uint8 *)cb, 10, 0, MSC_DIR_D2H, NULL, 0, (uint8 *)cb, sizeof(cb));
	if (nRetChild < 0)
		return -6;
	/*tax_interface_out("�������ݳ���Ϊ%d,����Ϊ��", nRetChild);
	printf_array(cb, nRetChild);*/

	//tax_interface_out("��29�ν���\n");
	memset(cb, 0, sizeof(cb));
	memset(data, 0, sizeof(data));
	memcpy(cb, "\xf1\x00\x00\x00\x00\x00\x02\x0a\x00\xff", 10);
	memcpy(data, "\x00\x07\x00\x00\xa4\x00\x00\x02\x4f\x01", 10);
	nRetChild = mass_storage_io_local(device, (uint8 *)cb, 10, 0, MSC_DIR_H2D, (uint8 *)data, 10, (uint8 *)cb, sizeof(cb));
	if (nRetChild != 0)
		return -5;
	memset(cb, 0, sizeof(cb));
	memcpy(cb, "\xf1\x00\x00\x00\x00\x00\x01\xfc\x08\xff", 10);
	nRetChild = mass_storage_io_local(device, (uint8 *)cb, 10, 0, MSC_DIR_D2H, NULL, 0, (uint8 *)cb, sizeof(cb));
	if (nRetChild < 0)
		return -6;
	/*tax_interface_out("�������ݳ���Ϊ%d,����Ϊ��", nRetChild);
	printf_array(cb, nRetChild);*/

	//tax_interface_out("��30�ν���\n");
	memset(cb, 0, sizeof(cb));
	memset(data, 0, sizeof(data));
	memcpy(cb, "\xf1\x00\x00\x00\x00\x00\x02\x0a\x00\xff", 10);
	memcpy(data, "\x00\x07\x00\x00\xa4\x00\x00\x02\x50\x07", 10);
	nRetChild = mass_storage_io_local(device, (uint8 *)cb, 10, 0, MSC_DIR_H2D, (uint8 *)data, 10, (uint8 *)cb, sizeof(cb));
	if (nRetChild != 0)
		return -5;
	memset(cb, 0, sizeof(cb));
	memcpy(cb, "\xf1\x00\x00\x00\x00\x00\x01\xfc\x08\xff", 10);
	nRetChild = mass_storage_io_local(device, (uint8 *)cb, 10, 0, MSC_DIR_D2H, NULL, 0, (uint8 *)cb, sizeof(cb));
	if (nRetChild < 0)
		return -6;
	//tax_interface_out("�������ݳ���Ϊ%d,����Ϊ��", nRetChild);
	//printf_array(cb, nRetChild);

	//tax_interface_out("��31�ν���\n");
	memset(cb, 0, sizeof(cb));
	memset(data, 0, sizeof(data));
	memcpy(cb, "\xf1\x00\x00\x00\x00\x00\x02\x09\x00\xff", 10);
	memcpy(data, "\x00\x06\x00\x00\xb0\x00\x00\x00\x9c", 9);
	nRetChild = mass_storage_io_local(device, (uint8 *)cb, 10, 0, MSC_DIR_H2D, (uint8 *)data, 9, (uint8 *)cb, sizeof(cb));
	if (nRetChild != 0)
		return -5;
	memset(cb, 0, sizeof(cb));
	memcpy(cb, "\xf1\x00\x00\x00\x00\x00\x01\xfc\x08\xff", 10);
	nRetChild = mass_storage_io_local(device, (uint8 *)cb, 10, 0, MSC_DIR_D2H, NULL, 0, (uint8 *)cb, sizeof(cb));
	if (nRetChild < 0)
		return -6;
	//tax_interface_out("�������ݳ���Ϊ%d,����Ϊ��", nRetChild);
	//printf_array(cb, nRetChild);


	//tax_interface_out("��32�ν���\n");
	memset(cb, 0, sizeof(cb));
	memset(data, 0, sizeof(data));
	memcpy(cb, "\xf1\x00\x00\x00\x00\x00\x02\x0a\x00\xff", 10);
	memcpy(data, "\x00\x07\x00\x00\xa4\x00\x00\x02\x4f\x01", 10);
	nRetChild = mass_storage_io_local(device, (uint8 *)cb, 10, 0, MSC_DIR_H2D, (uint8 *)data, 10, (uint8 *)cb, sizeof(cb));
	if (nRetChild != 0)
		return -5;
	memset(cb, 0, sizeof(cb));
	memcpy(cb, "\xf1\x00\x00\x00\x00\x00\x01\xfc\x08\xff", 10);
	nRetChild = mass_storage_io_local(device, (uint8 *)cb, 10, 0, MSC_DIR_D2H, NULL, 0, (uint8 *)cb, sizeof(cb));
	if (nRetChild < 0)
		return -6;
	//tax_interface_out("�������ݳ���Ϊ%d,����Ϊ��", nRetChild);
	//printf_array(cb, nRetChild);


	//tax_interface_out("��33�ν���\n");
	memset(cb, 0, sizeof(cb));
	memset(data, 0, sizeof(data));
	memcpy(cb, "\xf1\x00\x00\x00\x00\x00\x02\x0a\x00\xff", 10);
	memcpy(data, "\x00\x07\x00\x00\xa4\x00\x00\x02\x50\x09", 10);
	nRetChild = mass_storage_io_local(device, (uint8 *)cb, 10, 0, MSC_DIR_H2D, (uint8 *)data, 10, (uint8 *)cb, sizeof(cb));
	if (nRetChild != 0)
		return -5;
	memset(cb, 0, sizeof(cb));
	memcpy(cb, "\xf1\x00\x00\x00\x00\x00\x01\xfc\x08\xff", 10);
	nRetChild = mass_storage_io_local(device, (uint8 *)cb, 10, 0, MSC_DIR_D2H, NULL, 0, (uint8 *)cb, sizeof(cb));
	if (nRetChild < 0)
		return -6;
	//tax_interface_out("�������ݳ���Ϊ%d,����Ϊ��", nRetChild);
	//printf_array(cb, nRetChild);


	//tax_interface_out("��34�ν���\n");
	memset(cb, 0, sizeof(cb));
	memset(data, 0, sizeof(data));
	memcpy(cb, "\xf1\x00\x00\x00\x00\x00\x02\x09\x00\xff", 10);
	memcpy(data, "\x00\x06\x00\x00\xb0\x00\x00\x06\x20", 9);
	nRetChild = mass_storage_io_local(device, (uint8 *)cb, 10, 0, MSC_DIR_H2D, (uint8 *)data, 9, (uint8 *)cb, sizeof(cb));
	if (nRetChild != 0)
		return -5;
	memset(cb, 0, sizeof(cb));
	memcpy(cb, "\xf1\x00\x00\x00\x00\x00\x01\xfc\x08\xff", 10);
	nRetChild = mass_storage_io_local(device, (uint8 *)cb, 10, 0, MSC_DIR_D2H, NULL, 0, (uint8 *)cb, sizeof(cb));
	if (nRetChild < 0)
		return -6;
	//tax_interface_out("�������ݳ���Ϊ%d,����Ϊ��", nRetChild);
	//printf_array(cb, nRetChild);




	unsigned char Flag1[] = { 0x06, 0x03, 0x55, 0x04, 0x03 };
	unsigned char *p = NULL;

	p = SearchBytes(cb, cb + nRetChild, Flag1, sizeof Flag1);
	if (p == NULL)
	{
		tax_interface_out("δ�ҵ���һ����־λ��0x06, 0x03, 0x55, 0x04, 0x03\n");
		return -7;
	}

	p = SearchBytes(p + 1, cb + nRetChild, Flag1, sizeof Flag1);
	if (p == NULL)
	{
		tax_interface_out("δ�ҵ��ڶ�����־λ��0x06, 0x03, 0x55, 0x04, 0x03\n");
		return -8;
	}

	if (*(p + 5) == 0x1e)
	{//unicode

	}
	else if ((*(p + 5) == 0x0c))
	{//utf8
		char name[500] = { 0 };
		unsigned char bLen = (unsigned char)*(p - 1);
		int nExtraLen = sizeof Flag1 + 2;
		unsigned char *pStr = p + nExtraLen;
		memcpy(name, pStr, bLen - nExtraLen);


		EncordingConvert("utf-8", "gbk", name, strlen(name), hDev->szCompanyName, sizeof(hDev->szCompanyName));
		tax_interface_out("szCompanyName��%s\n", hDev->szCompanyName);
		return 0;
	}

	nRet = 0;
LoadAisinoDevInfo_Finish:
	if (nRet) {
		tax_interface_out("[-] LoadAisinoSelectDevInfo_local failed,nRet = %d\n", nRet);
	}
	else {
		;
	}
	return nRet;
}

int LoadNisecDevInfo_local(HDEV_LOCAL hDev, HUSB_LOCAL hUSB, uint8 bAuthDev)
{
	int nRet = -1;
	hDev->hUSBDevice = hUSB;
	hDev->bDeviceType = DEVICE_NISEC_LOCAL;
#ifdef _NODEVICE_TEST
	if (LoadNoDeviceJson(hDev) < 0)
		goto LoadNisecDevInfo_Finish;
	nRet = 0;
	goto LoadNisecDevInfo_Finish;
#endif
	if (NisecEntry_local(hUSB, NOP_OPEN_FULL_LOCAL) < 0)
		return -1;

	if (GetDiskEnvFromManageData_local(hDev) < 0) {
		nRet = -4;
		goto LoadNisecDevInfo_Finish;
	}

	nRet = 0;


	//_WriteLog(LL_INFO, "��������ƽ̨��Ϣ��%s", hDev->szPubServer + 2);
	//printf_array(hDev->szPubServer, sizeof(hDev->szPubServer));
LoadNisecDevInfo_Finish:
	if (nRet)
		tax_interface_out( "LoadDevInfo failed,nRet = %d\n", nRet);
	else
		;/*_WriteLog(LL_DEBUG,
		 "DevInfo, region:%s devid:%s taxidstr:%s szCompressTaxID:%s taxid9b:%s "
		 "cusname:%s taxauth:%s",
		 hDev->szRegCode, hDev->szDeviceID, hDev->szCommonTaxID, hDev->szCompressTaxID,
		 hDev->sz9ByteHashTaxID, hDev->szCompanyName, hDev->szTaxAuthorityName);*/
	NisecEntry_local(hUSB, NOP_CLOSE_LOCAL);
	return nRet;
}

int LoadCntaxDevInfo_local(HDEV_LOCAL hDev, HUSB_LOCAL hUSB, bool bAuthDev)
{
	int nRet = -1;
	hDev->hUSBDevice = hUSB;
	hDev->bDeviceType = DEVICE_CNTAX_LOCAL;


	if ((nRet = CnTaxGetDeviceEnv_local(hDev, hUSB)) < 0) {
		nRet = -4;
		tax_interface_out("CnTaxGetDeviceEnv_local failed,nRet = %d\n", nRet);
		goto LoadCntaxDevInfo_Finish;
	}

	//tax_interface_out("LoadCntaxDevInfo_local,nRet = %d\n", nRet);

	nRet = 0;

LoadCntaxDevInfo_Finish:
	if (nRet)
		tax_interface_out("LoadCntaxDevInfo_local failed,nRet = %d\n", nRet);
	//else
	//    _WriteLog(LL_DEBUG,
	//              "DevInfo, region:%s devid:%s taxidstr:%s szCompressTaxID:%s taxid9b:%s "
	//              "cusname:%s taxauth:%s",
	//              hDev->szRegCode, hDev->szDeviceID, hDev->szCommonTaxID, hDev->szCompressTaxID,
	//              hDev->sz9ByteHashTaxID, hDev->szCompanyName, hDev->szTaxAuthorityName);
	return nRet;
}

int LoadSkpKeyDevInfo_local(HDEV_LOCAL hDev, HUSB_LOCAL hUSB)
{
	unsigned char cb[1024 * 10] = { 0 };
	int nChildRet;

	memset(cb, 0, sizeof(cb));
	memcpy(cb, "\xfe\x6f\x01", 3);  // 
	nChildRet = NisecLogicIO_local(hUSB, cb, 3, sizeof(cb), NULL, 0);
	//printf("nChildRet = %d\n", nChildRet);
	//printf_array((char *)cb, nChildRet);

	sprintf(hDev->szCompanyName, "TCG-O1K˰��Կ��");
	memcpy(hDev->szDeviceID, cb + 2, 12);
	sprintf(hDev->szCommonTaxID, "33-%s", hDev->szDeviceID);

	return 0;
}

int LoadMengBaiInfo_local(HUSB_LOCAL device, HDEV_LOCAL HDev)
{
	int in_len, out_len;
	unsigned char s_data[4096];
	unsigned char r_data[4096];
	int result;
	char errinfo[2048] = { 0 };
	//��������
	//////////////////////////////////////////////////////////////////////////
	//tax_interface_out("��������\n");
	char time_now[20];
	memset(time_now, 0, sizeof(time_now));
	//tax_interface_out("LoadMengBaiInfo_local 11111111111111\n");
	timer_read_y_m_d_h_m_s(time_now);
	//tax_interface_out("���õ�ǰʱ��%s\n", time_now);

	strcpy(HDev->szDeviceTime, time_now);

	//////////print_array((char *)usb_data, size);
	//////////////////////////////////////////////////////////////////////
	memset(s_data, 0, sizeof s_data);
	in_len = pack_s_buf(DF_PC_USB_ORDER_CONNECT, 0, (unsigned char *)time_now, s_data, strlen(time_now));
	out_len = sizeof(r_data);
	//tax_interface_out("LoadMengBaiInfo_local 22222222222222222\n");
	result = usb_data_write_read_local(device, s_data, in_len, r_data, &out_len, errinfo);
	if (result < 0)
	{
		tax_interface_out("����ʧ��\n");
		return result;
	}
	//tax_interface_out("LoadMengBaiInfo_local 33333333333333333\n");
	//��ȡ����Ϣ
	////////////////////////////////////////////////////////////////////////
	//tax_interface_out("��ȡ����Ϣ\n");
	memset(s_data, 0, sizeof(s_data));
	memset(r_data, 0, sizeof(r_data));
	in_len = pack_s_buf(DF_PC_USB_ORDER_GET_CARD_IFNFO, 0, NULL, s_data, 0);
	out_len = sizeof(r_data);
	//tax_interface_out("LoadMengBaiInfo_local 4444444444444444\n");
	result = usb_data_write_read_local(device, s_data, in_len, r_data, &out_len, errinfo);
	if (result < 0)
	{
		tax_interface_out("��ȡ����Ϣʧ��\n");
		return -1;
	}
	//tax_interface_out("��ȡ����Ϣ�ɹ�\n");

	HDev->uICCardNo = r_data[122] * 0x1000000 + r_data[123] * 0x10000 + r_data[124] * 0x100 + r_data[125] * 0x1;
	//tax_interface_out("LoadMengBaiInfo_local 55555555555555\n");
	memcpy(HDev->szDeviceID, r_data, 12);
	memcpy(HDev->szCommonTaxID, r_data + 12, 20);
	memcpy(HDev->szCompanyName, r_data + 32, 80);

	tax_interface_out("��ȡ������˰�����ƣ�%s\n", HDev->szCompanyName);
	return 0;
}

int LoadMengBaiSeverKey_local(HUSB_LOCAL device, HDEV_LOCAL HDev)
{
	int in_len, out_len;
	unsigned char s_data[4096];
	unsigned char r_data[4096];
	int result;
	char errinfo[2048] = { 0 };
	//��������
	//////////////////////////////////////////////////////////////////////////
	//tax_interface_out("��������\n");
	char time_now[20];
	memset(time_now, 0, sizeof(time_now));

	timer_read_y_m_d_h_m_s(time_now);
	//tax_interface_out("���õ�ǰʱ��%s\n", time_now);

	strcpy(HDev->szDeviceTime, time_now);

	//////////print_array((char *)usb_data, size);
	//////////////////////////////////////////////////////////////////////
	memset(s_data, 0, sizeof s_data);
	in_len = pack_s_buf(DF_PC_USB_ORDER_CONNECT, 0, (unsigned char *)time_now, s_data, strlen(time_now));
	out_len = sizeof(r_data);

	result = usb_data_write_read_local(device, s_data, in_len, r_data, &out_len, errinfo);
	if (result < 0)
	{
		tax_interface_out("����ʧ��\n");
		return result;
	}

	//��ȡ����Ϣ
	////////////////////////////////////////////////////////////////////////
	//tax_interface_out("��ȡ����Ϣ\n");
	memset(s_data, 0, sizeof(s_data));
	memset(r_data, 0, sizeof(r_data));
	in_len = pack_s_buf(DF_PC_USB_ORDER_GET_CARD_IFNFO, 0, NULL, s_data, 0);
	out_len = sizeof(r_data);
	result = usb_data_write_read_local(device, s_data, in_len, r_data, &out_len, errinfo);
	if (result < 0)
	{
		tax_interface_out("��ȡ����Ϣʧ��\n");
		return -1;
	}
	//tax_interface_out("��ȡ����Ϣ�ɹ�\n");

	HDev->uICCardNo = 0;
	memcpy(HDev->szDeviceID, r_data, 12);
	memcpy(HDev->szCommonTaxID, r_data + 12 + 1024 + 1024, 12);
	memcpy(HDev->szCompanyName, r_data + 12 + 1024 + 1024 + 12, 80);
	tax_interface_out("��ȡ���Ŀ���ţ�%s,ʶ��ţ�%s,���ƣ�%s\n", HDev->szDeviceID, HDev->szCommonTaxID, HDev->szCompanyName);
	return 0;
}

int DetectDeviceInit_local(HUSB_LOCAL hUSB,HDEV_LOCAL hDev, uint8 bAuthDev, int nBusID, int nDevID)
{
	int nRet = -1;
	if (!hDev || !hUSB)
	{
		libusb_exit(hUSB->context);
		tax_interface_out("[-] DetectDeviceInit_local !hDev || !hUSB\n");
		return -1;
	}
	tax_interface_out("[+] Open usb device\n");

	int nhUsbType = usb_device_open_local(hUSB, nBusID, nDevID);
	if (nhUsbType <= 0) {
		//tax_interface_out("[-] Usb device open failed,nhUsbType = %d\n", nhUsbType);
		if (hUSB->handle &&	nhUsbType == DEVICE_UNKNOW_LOCAL && hUSB->context)
		{
			usb_device_close_local(hUSB);
			libusb_exit(hUSB->context);
		}
		//report_event(NULL, "USB��ʧ��", "libusbͨ��busid��devid���豸ʧ��", nhUsbType);
		return -2;
	}
	//report_event(NULL, "USB��ʧ��", "libusbͨ��busid��devid���豸ʧ��", nhUsbType);
	tax_interface_out("[+] Open usb device success nhUsbType = %d\n", nhUsbType);
	if (DEVICE_NONGYE_LOCAL == nhUsbType) {
		nhUsbType = DEVICE_NONGYE_LOCAL;
	}
	if (DEVICE_AISINO_LOCAL == nhUsbType) {  // aisino init

		nRet = LoadAisinoDevInfo_local(hDev, hUSB, bAuthDev);
		if (nRet < 0) {

			if (nRet == -4)//����Ϊ��ѡ˰��
			{

				nRet = LoadAisinoSelectDevInfo_local(hDev, hUSB, bAuthDev);
				if (nRet < 0)
				{
					//report_event(NULL, "��˰�̼��ػ�����Ϣʧ��", "1��˰�̼��ػ�����Ϣʧ��,���ܴ��ڼ�������", nRet);
					tax_interface_out("[+] Init device to open failed, nRet:%d\n", nRet);
					tax_interface_out("[+] Init device to open failed, nRet:%d\n", nRet);
					if (hUSB->handle)
						usb_device_close_local(hUSB);
					libusb_exit(hUSB->context);
					return -4;
				}
				nhUsbType = DEVICE_JSP_KEY_LOCAL;

			}
			else
			{
				//report_event(NULL, "��˰�̼��ػ�����Ϣʧ��", "2��˰�̼��ػ�����Ϣʧ��,���ܴ��ڼ�������", nRet);
				tax_interface_out("[+] Init device to open failed, nRet:%d\n", nRet);
				tax_interface_out("[+] Init device to open failed, nRet:%d\n", nRet);
				if (hUSB->handle)
					usb_device_close_local(hUSB);
				libusb_exit(hUSB->context);
				return -4;
			}


		}
	}
	else if (DEVICE_NISEC_LOCAL == nhUsbType)  {  // nisec init
		//tax_interface_out("[+] Nisec device found\n");
		//tax_interface_out("Nisec device found\n");
		//������ʵ���ùرյģ�����Ϊ�˱�֤ÿ�ζ����µ�״̬�����ǹر������´򿪱ȽϺ�,�п���֮ǰû�����ٹؾͲ��ɹ�����˲��������ж�
		//tax_interface_out("���豸ǰ�ȹؿ�\n");
		//NisecEntry_local(hUSB, NOP_CLOSE_LOCAL);
		//tax_interface_out("���豸ǰ�ȹؿ�,�ؿ����\n");
		nRet = LoadNisecDevInfo_local(hDev, hUSB, bAuthDev);
		if (nRet < 0) {
			//report_event(NULL, "˰���̼��ػ�����Ϣʧ��", "˰���̼��ػ�����Ϣʧ��,���ܴ��ڼ�������", nRet);
			tax_interface_out("[+] Init device to open failed, nRet:%d\n", nRet);
			if (hUSB->handle)
				usb_device_close_local(hUSB);
			libusb_exit(hUSB->context);
			return -5;
		}
		//tax_interface_out( "[+] LoadNisecDevInfo_local hDev %s\n", hDev->szTaxAuthorityCodeEx);
	}
	else if (nhUsbType == DEVICE_CNTAX_LOCAL)
	{
		tax_interface_out("[+] Cntax-UKey device found");
		hDev->bDeviceType = nhUsbType;
		// nRet = CntaxEntry_local(hUSB, NOP_CLOSE_LOCAL);  //�ر������´�
		// printf("CntaxEntry_local nRet = %d\n",nRet);
		nRet = LoadCntaxDevInfo_local(hDev, hUSB, bAuthDev);
		if (nRet < 0) {
			//report_event(NULL, "˰��UKEY���ػ�����Ϣʧ��", "˰��UKEY���ػ�����Ϣʧ��,���ܴ��ڼ�������", nRet);
			tax_interface_out("[+] Init device to open failed, nRet:%d\n", nRet);
			if (hUSB->handle)
				usb_device_close_local(hUSB);
			libusb_exit(hUSB->context);
			return ERR_DEV_PASSWD_ERR;
		}
		//printf("1-------CntaxGetManageData hDev->szDeviceID=%s\n",hDev->szDeviceID);
		tax_interface_out( "[+] Load cntax device successful!\n");
	}
	else if (DEVICE_SKP_KEY_LOCAL == nhUsbType)  {  // nisec init
		tax_interface_out("[+] �����豸 device found\n");
		//tax_interface_out("Nisec device found\n");
		//������ʵ���ùرյģ�����Ϊ�˱�֤ÿ�ζ����µ�״̬�����ǹر������´򿪱ȽϺ�,�п���֮ǰû�����ٹؾͲ��ɹ�����˲��������ж�
		LoadSkpKeyDevInfo_local(hDev, hUSB);
	}
	//printf("1-------CntaxGetManageData hDev->szDeviceID=%s\n",hDev->szDeviceID);
	return nhUsbType;
}



HDEV_LOCAL MallocDevInfo_local()
{
	HDEV_LOCAL hDev = calloc(1, sizeof(struct StaticDeviceInfo));
	if (!hDev)
		return NULL;
	return hDev;
}

HUSB_LOCAL MallocUSBSession_local()
{
	HUSB_LOCAL husb = calloc(1, sizeof(struct USBSession));
	if (!husb)
		return NULL;
	husb->handle = NULL;
	husb->context = NULL;
	libusb_init(&husb->context);
	// libusb_set_debug(husb->context, LIBUSB_LOG_LEVEL_INFO);  // LIBUSB_LOG_LEVEL_DEBUG
// #ifdef _NODEVICE_TEST
// 	husb->pTestData = calloc(1, sizeof(struct stTestData));
// #endif
	return husb;
}


void FreeDevInfo_local(HDEV_LOCAL hDev)
{
	if (!hDev)
		return;
	//printf("FreeDevInfo_local 111111\n");
	// if (hDev->pX509Cert != NULL) {
	// 	printf("FreeDevInfo_local 222222\n");
	// 	X509_free(hDev->pX509Cert);
	// 	printf("FreeDevInfo_local 3333333\n");
	// 	hDev->pX509Cert = NULL;
	// }
	//printf("FreeDevInfo_local 4444444\n");
	//free(hDev);
	//printf("FreeDevInfo_local 5555555\n");
}
void FreeUSBSession_local(HUSB_LOCAL husb)
{
	// udev's cache 8192bytes ȫ��leak���������ѭ��leak
	if (!husb)
		return;
	if (husb->handle)
		usb_device_close_local(husb);
	libusb_exit(husb->context);
#ifdef _NODEVICE_TEST
	free(husb->pTestData);
#endif
	//free(husb);
}


static int usb_device_close_serssion(HUSB_LOCAL h_USB, HDEV_LOCAL h_Dev, int nDeviceType)
{
	FreeUSBSession_local(h_USB);
	//FreeDevInfo_local(h_Dev);
	return 0;
}
static int usb_hid_close_serssion(HUSB_LOCAL h_USB, HDEV_LOCAL h_Dev)
{
	if (!h_USB)
		return 0;
	if (h_USB->handle)
	{
		//printf("usb_hid_close_serssion 0\n");
		libusb_close(h_USB->handle);
		h_USB->handle = NULL;
		//printf("usb_hid_close_serssion 1\n");
	}
	//printf("usb_hid_close_serssion 11111\n");
	//printf("h_USB->context = %x\n",h_USB->context);
	if(NULL != h_USB->context)
	{
		libusb_exit(h_USB->context);
		h_USB->context = NULL;
	}
	//printf("usb_hid_close_serssion 222222\n");
	//free(h_USB);
	//printf("usb_hid_close_serssion 3333333\n");
	//FreeDevInfo_local(h_Dev);
	//printf("usb_hid_close_serssion 444444\n");
	return 0;
}

static int usb_hid_open_serssion(HUSB_LOCAL h_USB, HDEV_LOCAL h_Dev, int nBusID, int nDevID)
{
	//tax_interface_out("enter usb_hid_open_serssion nBusID = %d,nDevID=%d\n",nBusID,nDevID);
	if ((nBusID<1) || (nDevID>127))
	{
		//printf("nBusNum range error\n");
		return -1;
	}
	if ((nBusID<1) || (nDevID>127))
	{
		//printf("nDevNum range error\n");
		return -2;
	}

	

	// HUSB_LOCAL hUSB = MallocUSBSession_local();
	// HDEV_LOCAL hDev = MallocDevInfo_local();
	//tax_interface_out("usb_hid_open_serssion 11111111111111\n");
	libusb_init(&(h_USB->context));
	int nhUsbType = usb_device_open_hid_local(h_USB, nBusID, nDevID);
	//tax_interface_out("usb_hid_open_serssion 22222222222222\n");
	if (nhUsbType == DEVICE_MENGBAI_LOCAL)
	{
		if (LoadMengBaiInfo_local(h_USB, h_Dev) < 0)
		{
			tax_interface_out("�豸�򿪳ɹ�,����ʧ��\n");
			usb_hid_close_serssion(h_USB, h_Dev);
			return -3;
		}
	}
	else if (nhUsbType == DEVICE_MENGBAI2_LOCAL)
	{
		if (LoadMengBaiSeverKey_local(h_USB, h_Dev) < 0)
		{
			tax_interface_out("�豸�򿪳ɹ�,����ʧ��\n");
			usb_hid_close_serssion(h_USB, h_Dev);
			return -4;
		}
	}
	else
	{
		tax_interface_out("�豸��ʧ��,result = %d\n", nhUsbType);
		usb_hid_close_serssion(h_USB, h_Dev);
		 //libusb_exit(hUSB->context);
		// free(hUSB);
		// FreeDevInfo_local(hDev);
		return -5;
	}

	// *h_USB = &test_USB;//hUSB;
	// *h_Dev = &test_HDEV;//hDev;
	//h_Dev->hUSBDevice = h_USB;
	return 0;
}

static int usb_device_open_serssion_lasterr(HUSB_LOCAL h_USB, HDEV_LOCAL h_Dev, uint8 bAuthDev, int nBusID, int nDevID, char *lasterror)
{
	if ((nBusID<1) || (nBusID>127))
	{
		printf("nBusNum range error,nBusID = %d\n", nBusID);
		return -1;
	}
	if ((nDevID<1) || (nDevID>127))
	{
		printf("nDevNum range error,nDevID = %d\n", nDevID);
		return -2;
	}
	int nDeviceType = 0;
	// HUSB_LOCAL hUSB = MallocUSBSession_local();
	// HDEV_LOCAL hDev = MallocDevInfo_local();
	//tax_interface_out("usb_device_open_serssion enter\n");
	libusb_init(&(h_USB->context));
	nDeviceType = DetectDeviceInit_local(h_USB, h_Dev, bAuthDev, nBusID, nDevID);
	//tax_interface_out("DetectDeviceInit_local success\n");
	if (nDeviceType < 0) {
		tax_interface_out("[-] No support device found\n");
		strcpy(lasterror, h_USB->szLastDevError);
		tax_interface_out("usb_device_open_serssion_lasterr DetectDeviceInit_local nDeviceType = %d\n", nDeviceType);
		// if (hUSB != NULL)
		// 	free(hUSB);
		// if (hDev != NULL)
		// 	free(hDev);
		return nDeviceType;
	}
	// *h_USB = hUSB;
	// *h_Dev = hDev;
	// hDev->hUSBDevice = hUSB;
	//printf("22-------CntaxGetManageData hDev->szDeviceID=%s\n",h_Dev->szDeviceID);
	return nDeviceType;
}

static int usb_device_open_serssion(HUSB_LOCAL *h_USB, HDEV_LOCAL *h_Dev, uint8 bAuthDev, int nBusID, int nDevID)
{
	if ((nBusID<1) || (nBusID>127))
	{
		printf("nBusNum range error,nBusID = %d\n", nBusID);
		return -1;
	}
	if ((nDevID<1) || (nDevID>127))
	{
		printf("nDevNum range error,nDevID = %d\n", nDevID);
		return -2;
	}

	int nDeviceType = 0;
	HUSB_LOCAL hUSB = MallocUSBSession_local();
	HDEV_LOCAL hDev = MallocDevInfo_local();
	//tax_interface_out("usb_device_open_serssion enter\n");
	nDeviceType = DetectDeviceInit_local(hUSB,hDev, bAuthDev, nBusID, nDevID);
	//tax_interface_out("DetectDeviceInit_local success\n");
	if (nDeviceType < 0) {
		tax_interface_out("[-] No support device found\n");
		tax_interface_out("usb_device_open_serssion DetectDeviceInit_local nDeviceType = %d\n", nDeviceType);
		if (hUSB != NULL)
			free(hUSB);
		if (hDev != NULL)
			free(hDev);
		return nDeviceType;
	}
	*h_USB = hUSB;
	*h_Dev = hDev;
	hDev->hUSBDevice = hUSB;
	return nDeviceType;
}



int function_common_get_basic_tax_info_local(char *busid,  int usb_type, char *ca_name, char *ca_serial, char *sn,int *passwd_right ,int *left_num)
{
	int result;
	int nDeviceType;

	//printf("lbc1111----------------sn = %x\n",sn);

	struct USBSession_Local tmp_USB;
	struct StaticDeviceInfoLocal tmp_HDEV;

	memset(&tmp_USB,0x00,sizeof(struct USBSession_Local));
	memset(&tmp_HDEV,0x00,sizeof(struct StaticDeviceInfoLocal));

	//printf("lbc2222----------------sn = %x\n",sn);

	HUSB_LOCAL h_usb = &tmp_USB;
	HDEV_LOCAL h_dev = &tmp_HDEV;
	int nBusID;
	int nDevID;

	//tax_interface_out("ebetr function_common_get_basic_tax_info_local\n");

	//printf("lbc3333----------------sn = %x\n",sn);

	nBusID = get_busid_busnum(busid);
	nDevID = get_busid_devnum(busid);

	//printf("lbc4444----------------sn = %x\n",sn);

	if (usb_type == DEVICE_MENGBAI_LOCAL)
	{
		//tax_interface_out("usb_type == DEVICE_MENGBAI_LOCAL\n");
		//*passwd_right= 1;
		//tax_interface_out("usb_type == DEVICE_MENGBAI_LOCAL 1111111\n");
		//printf("lbc5555----------------sn = %x\n",sn);
		result = usb_hid_open_serssion(h_usb, h_dev, nBusID, nDevID);
		//printf("lbc6666----------------sn = %x\n",sn);
		if (result < 0)
		{
			tax_interface_out("�豸��ʧ��,result = %d\n", result);
			return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
		}
		//tax_interface_out("function_common_get_basic_tax_info_local 11111111111111\n");
		strcpy(ca_name,h_dev->szCompanyName);
		//tax_interface_out("function_common_get_basic_tax_info_local 2\n");
		strcpy(ca_serial, h_dev->szCommonTaxID);
		//tax_interface_out("function_common_get_basic_tax_info_local 3,h_dev->szDeviceID=%s,strlen(h_dev->szDeviceID) = %d\n",h_dev->szDeviceID,strlen(h_dev->szDeviceID));
		//printf("lbc----------------sn = %x\n",sn);
		memcpy(sn, h_dev->szDeviceID,strlen(h_dev->szDeviceID));
		//tax_interface_out("function_common_get_basic_tax_info_local 22222222222\n");
		usb_hid_close_serssion(h_usb, h_dev);
		// h_usb = NULL;
		// h_dev = NULL;
		//printf("ca_name=%s,ca_serial=%s,sn=%s\n",ca_name,ca_serial,sn);
		return 0;
	}
	else if (usb_type == DEVICE_MENGBAI2_LOCAL)
	{
		*passwd_right= 1;
		result = usb_hid_open_serssion(h_usb, h_dev, nBusID, nDevID);
		if (result < 0)
		{
			tax_interface_out("�豸��ʧ��,result = %d\n", result);
			return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
		}
		strcpy(ca_name, h_dev->szCompanyName);
		strcpy(ca_serial, h_dev->szCommonTaxID);
		strcpy(sn, h_dev->szDeviceID);
		usb_hid_close_serssion(h_usb, h_dev);
		return 0;
	}
	else if (usb_type == DEVICE_NISEC_LOCAL)
	{
		char last_error[1024] = { 0 };
		*passwd_right= 1;
		nDeviceType = usb_device_open_serssion_lasterr(h_usb, h_dev, 1, nBusID, nDevID, last_error);
		//tax_interface_out("USB�豸�򿪳ɹ�\n");
		//tax_interface_out("usb_device_open_serssion success ���ߺţ�%d���豸�ţ�%d\n", nBusID, nDevID);
		if (nDeviceType < 0)
		{
			tax_interface_out("˰�����豸��ʼ����ʧ��,���������Ϊ%s\n", last_error);
			if (memcmp(last_error, "[DEV] 09d1c", strlen("[DEV] 09d1c")) == 0)
			{
				char devpass_leftcount[10] = { 0 };
				char errinfo[1024] = { 0 };
				memcpy(devpass_leftcount, last_error + strlen("[DEV] 09d1c"), 1);
				sprintf(errinfo, "��Ĭ���豸����88888888,ʣ�ೢ�Դ���%s\n", devpass_leftcount);
				tax_interface_out("errinfo: %s\n", errinfo);
				*passwd_right= 0;
				*left_num = atoi(devpass_leftcount);
			}
			if (memcmp(last_error, "[DEV] 09d10b", strlen("[DEV] 09d10b")) == 0)
			{
				char devpass_leftcount[10] = { 0 };
				char errinfo[1024] = { 0 };
				memcpy(devpass_leftcount, last_error + strlen("[DEV] 09d10b"), 1);
				sprintf(errinfo, "˰��������\n");
				tax_interface_out("errinfo: %s\n", errinfo);
				*passwd_right = 0;
				*left_num = atoi(devpass_leftcount);
			}
			return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
		}
		strcpy(ca_name, h_dev->szCompanyName);
		strcpy(ca_serial, h_dev->szCommonTaxID);
		strcpy(sn, h_dev->szDeviceID);
		usb_device_close_serssion(h_usb, h_dev, nDeviceType);
		return 0;
	}
	else if (usb_type == DEVICE_AISINO_LOCAL)
	{
		char last_error[1024] = { 0 };
		*passwd_right= 1;
		nDeviceType = usb_device_open_serssion_lasterr(h_usb, h_dev, 1, nBusID, nDevID, last_error);
		if (nDeviceType < 0)
		{
			tax_interface_out("��˰���豸��ʼ����ʧ��,���������Ϊ%s\n", last_error);
			if (memcmp(last_error, "[DEV] 09d1c", strlen("[DEV] 09d1c")) == 0)
			{
				char devpass_leftcount[10] = { 0 };
				char errinfo[1024] = { 0 };
				memcpy(devpass_leftcount, last_error + strlen("[DEV] 09d1c"), 1);
				sprintf(errinfo, "��Ĭ���豸����88888888,ʣ�ೢ�Դ���%s\n", devpass_leftcount);
				tax_interface_out("errinfo: %s\n", errinfo);
				*passwd_right = 0;
				*left_num = atoi(devpass_leftcount);
			}
			if (memcmp(last_error, "[DEV] 09d10b", strlen("[DEV] 09d10b")) == 0)
			{
				char devpass_leftcount[10] = { 0 };
				char errinfo[1024] = { 0 };
				memcpy(devpass_leftcount, last_error + strlen("[DEV] 09d10b"), 1);
				sprintf(errinfo, "˰��������\n");
				tax_interface_out("errinfo: %s\n", errinfo);
				*passwd_right = 0;
				*left_num = atoi(devpass_leftcount);
			}
			return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
		}
		strcpy(ca_name, h_dev->szCompanyName);
		strcpy(ca_serial, h_dev->szCommonTaxID);
		strcpy(sn, h_dev->szDeviceID);
		usb_device_close_serssion(h_usb, h_dev, nDeviceType);
		return 0;

	}
	else if (usb_type == DEVICE_CNTAX_LOCAL)
	{
		char last_error[1024] = { 0 };
		*passwd_right = 1;
		//printf("############CNTAX KEY !##############\n");
		nDeviceType = usb_device_open_serssion_lasterr(h_usb, h_dev, 1, nBusID, nDevID, last_error);
		//printf("2-------CntaxGetManageData hDev->szDeviceID=%s\n",h_dev->szDeviceID);
		if(nDeviceType == DEVICE_NONGYE_LOCAL)		//wang 11/18
		{
			return 1;
		}
		else if (nDeviceType < 0)
		{
			tax_interface_out("˰��UKey�豸��ʼ����ʧ��,���������Ϊ%s\n", last_error);
			if (memcmp(last_error, "[DEV] 09d1c", strlen("[DEV] 09d1c")) == 0)
			{
				char devpass_leftcount[10] = { 0 };
				char errinfo[1024] = { 0 };
				memcpy(devpass_leftcount, last_error + strlen("[DEV] 09d1c"), 1);
				sprintf(errinfo, "��Ĭ���豸����88888888,ʣ�ೢ�Դ���%s\n", devpass_leftcount);
				tax_interface_out("errinfo: %s\n", errinfo);
				*passwd_right = 0;
				*left_num = atoi(devpass_leftcount);
			}
			if (memcmp(last_error, "[DEV] 09d10b", strlen("[DEV] 09d10b")) == 0)
			{
				char devpass_leftcount[10] = { 0 };
				char errinfo[1024] = { 0 };
				memcpy(devpass_leftcount, last_error + strlen("[DEV] 09d10b"), 1);
				sprintf(errinfo, "˰��������\n");
				tax_interface_out("errinfo: %s\n", errinfo);
				*passwd_right = 0;
				*left_num = atoi(devpass_leftcount);
			}
			return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
		}
		//printf("usb_type == DEVICE_CNTAX_LOCAL h_dev->szCompanyName=%s,h_dev->szCommonTaxID=%s,h_dev->szDeviceID=%s\n",h_dev->szCompanyName,h_dev->szCommonTaxID,h_dev->szDeviceID);
		strcpy(ca_name, h_dev->szCompanyName);
		strcpy(ca_serial, h_dev->szCommonTaxID);
		strcpy(sn, h_dev->szDeviceID);
		//printf("usb_type == DEVICE_CNTAX_LOCAL 222222\n");
		usb_device_close_serssion(h_usb, h_dev, nDeviceType);
		//printf("usb_type == DEVICE_CNTAX_LOCAL 333333\n");
		return 0;
	}
	else if (usb_type == DEVICE_SKP_KEY_LOCAL)
	{

		*passwd_right= 1;
		nDeviceType = usb_device_open_serssion( h_usb,  h_dev, 0, nBusID, nDevID);
		tax_interface_out("usb_device_open_serssion success ���ߺţ�%d���豸�ţ�%d\n", nBusID, nDevID);
		if (nDeviceType < 0)
		{
			return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
		}
		strcpy(ca_name, h_dev->szCompanyName);
		strcpy(ca_serial, h_dev->szCommonTaxID);
		strcpy(sn, h_dev->szDeviceID);
		usb_device_close_serssion(h_usb, h_dev, nDeviceType);
		return 0;
	}
	return -1;
}