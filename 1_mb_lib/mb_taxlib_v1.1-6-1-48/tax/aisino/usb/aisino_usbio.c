/*****************************************************************************
File name:   aisino_usbio.c
Description: ���ڰ���ŵ�̵��豸USBЭ�����
Author:      Zako
Version:     1.0
Date:        2019.09
History:
20190902     �������ʵ��
20220630     ��ʽ��Э�����
*****************************************************************************/
#include "aisino_usbio.h"

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

int AisinoVendorErrorCodeToDescription(int wRepCode, char *describe)
{
    int nFormatErrorCode = ERR_AISINO_COMMON;
    switch (wRepCode) {
        case 83:
            nFormatErrorCode = ERR_AISINO_QUERYTIME_LATER;
            strcpy(describe, "��ѯʱ����ڵ�ǰʱ�䣡");
            break;
        case 84:
            nFormatErrorCode = ERR_AISINO_QUERYTIME_EARLY;
            strcpy(describe, "��ѯʱ�����");
            break;
        case 87:
            nFormatErrorCode = ERR_AISINO_INVOICEVOLUME_TIME;
            strcpy(describe, "��˰�豸�з�Ʊ��ʱ���");
            break;
        case 99:
            nFormatErrorCode = ERR_AISINO_MAKEINVOICE_DATA;
            strcpy(describe, "��Ʊ���������⣡");
        case 161:
            nFormatErrorCode = ERR_AISINO_MAKEINVOICE_MAX;
            strcpy(describe, "�ɿ���Ʊ�������������");
            break;
        case 220:
            nFormatErrorCode = ERR_AISINO_NO_DATA;
            strcpy(describe, "��������������߼����������ݣ�");
            break;
        case 223:
            nFormatErrorCode = ERR_AISINO_NO_STOCK;
            strcpy(describe, "��������ָ���·����ô���Ϣ");
            break;
        case 246:
            nFormatErrorCode = ERR_AISINO_MAKEINVOICE_DEADLINE;
            strcpy(describe, "�ѵ��ϴ���ֹ���ڣ��������ѿ�δ�ϴ���Ʊ�����ܼ�����Ʊ��");
            break;
        case 248:
            nFormatErrorCode = ERR_AISINO_NEED_REPORT;
            strcpy(describe, "��Ʊ��û��������ڳ���˰�����ܼ�����Ʊ");
            break;
        case 1521:
            nFormatErrorCode = ERR_AISINO_UNKNOW;
            strcpy(describe, "�쳣������Ʊ");
            break;
        default:
            sprintf(describe, "�豸�ײ���󣬺���:%d", wRepCode);
            break;
    }
    //_WriteLog(LL_WARN, "AisinoLogicIO error, code:%d desc:%s", wRepCode, describe);
    return nFormatErrorCode;
}

//�������pInOutCmd��nInCmdLen��nInCmdBufLen�����һ����IO������,pTransferData��nTransferDataLen����ڶ�����IO����
//�����������������pInOutCmd��nRet��һ��Ϊ��������һ��Ϊ������ֵ����
int AisinoLogicIO(HUSB hUSB, uint8 *pInOutCmd, int nInCmdLen, int nInCmdBufLen,
                  uint8 *pTransferData, int nTransferDataLen)
{
    int nRetChild = 0;
    //--send command
    if ((nRetChild =
             mass_storage_io(hUSB, pInOutCmd, nInCmdLen, 0, MSC_DIR_H2D, (uint8 *)pTransferData,
                             nTransferDataLen, pInOutCmd, nInCmdBufLen)) < 0) {
        _WriteLog(LL_DEBUG, "[AisinoLogicIO] Command send failed\r\n");
        return -1;
    }
    //--recv header
    if ((nRetChild = mass_storage_io(hUSB, g_CmdE1AA, sizeof g_CmdE1AA, 11, MSC_DIR_D2H, 0, 0,
                                     pInOutCmd, nInCmdBufLen)) < 0) {
        _WriteLog(LL_FATAL, "[AisinoLogicIO] Logic header recv failed\r\n");
        return -2;
    }
    int wRepCode = GetDeviceRepCode(pInOutCmd);  //����ִ�гɹ��Ƿ�
    if (wRepCode != 0) {
        char describe[512] = {0};
        int nFormatErrorCode = AisinoVendorErrorCodeToDescription(wRepCode, describe);
        SetLastError(hUSB, nFormatErrorCode, "%d,%s", wRepCode, describe);
        return -3;
    }
    wRepCode = GetDeviceRepCode(pInOutCmd + 4);  //����content����,ʵ�ʷ��س���Ϊlen+1λ��У����
    if (!wRepCode)
        return 0;
    //���ص�bufflen���ڻ�����,Ԥ��һ�������ֽڸ������Լ����ض������
    uint8 abExtraMyBuf[12] = {0};
    //��������ڴ����� 00[1] signed myret flag[1] CmdE1AARet[10]
    int nMyExtraLen = sizeof(abExtraMyBuf);
    if (wRepCode + nMyExtraLen > nInCmdBufLen) {
        _WriteLog(LL_FATAL, "[AisinoLogicIO] Out buf's size not enough, need:%d\r\n",
                  wRepCode + nMyExtraLen);
        return -4;
    }
    //����CmdE1AARet���ں��ڱ���
    memcpy(abExtraMyBuf + 2, pInOutCmd, 10);  //�豸����У��λ1�ֽڣ��Լ���1�ֽ�;
    if (pInOutCmd[3] != 0xe3)                 //==\xe3 ��ֹ��ȡ���������
        *(sint8 *)(abExtraMyBuf + 1) = 1;     //������ȡ
                                              //--recv data
    nRetChild = mass_storage_io(hUSB, g_CmdE1AB, sizeof g_CmdE1AB, nInCmdBufLen, MSC_DIR_D2H, 0, 0,
                                pInOutCmd, nInCmdBufLen);
    if (nRetChild < 0 || nRetChild != wRepCode + 1) {
        _WriteLog(LL_FATAL,
                  "[AisinoLogicIO] Output buff's size not match header's num,nRetChild = "
                  "%d,wRepCode = %d\r\n",
                  nRetChild, wRepCode);
        return -5;
    }
    //������Զ��������������ݺ��汸��
    uint8 *pExtraMyRetBuf = pInOutCmd + nRetChild;
    memcpy(pExtraMyRetBuf, abExtraMyBuf, nMyExtraLen);
    //ֻ����ʵ�ʳ��ȣ��Զ��峤�Ȳ������ڷ��س�����
    return nRetChild;
}

int AisinoCryptIO(HUSB hUSB, bool bBigCryptIO, uint8 *TransferData, int nTransferLen,
                  uint8 *pRecvDataBuff, int nRecvDataBuffLen)
{
    int nRetChild = 0;
    uint8 cb[64], abTransferData[512], abRep[512];
    memset(cb, 0, sizeof(cb));
    cb[0] = 0xf1;
    // TransferData һ��Ƚ�С,���ĳ���Ӧ����ǩ��ģ��
    //���ù̶���С����,��������д󻺳��ٸ�Ϊ�ѷ���
    memcpy(abTransferData, TransferData, nTransferLen);
    if (!bBigCryptIO) {
        cb[1] = 0xf1;
        //ֻ����ǰ8�ֽ�
        AisinoDesEcbEncrypt(abTransferData);
    }
    cb[6] = 0x2;  // writeflag
    memcpy(cb + 7, &nTransferLen, 2);
    cb[9] = 0xff;
    nRetChild = mass_storage_io(hUSB, cb, 16, 0, MSC_DIR_H2D, (uint8 *)abTransferData, nTransferLen,
                                abRep, sizeof(abRep));
    if (nRetChild < 0)
        return -1;
    memset(pRecvDataBuff, 0, nRecvDataBuffLen);
    cb[6] = 0x1;  // readflag
    memcpy(cb + 7, &nRecvDataBuffLen, 2);
    nRetChild =
        mass_storage_io(hUSB, cb, 16, 260, MSC_DIR_D2H, NULL, 0, pRecvDataBuff, nRecvDataBuffLen);
    if (nRetChild < 0)
        return -2;
    return nRetChild;
}