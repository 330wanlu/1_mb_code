/*****************************************************************************
File name:   nisec_usbio.c
Description: ���ڰ����̵��豸USBЭ�����
Author:      Zako
Version:     1.0
Date:        2019.09
History:
20190902     �������ʵ��
20220630     ��ʽ��Э�����
*****************************************************************************/
#include "nisec_usbio.h"

int NisecVendorErrorCodeToDescription(int wRepCode, char *describe)
{
    int nFormatErrorCode = ERR_NISEC_COMMON;
    switch (wRepCode) {
        case 0x4:
            nFormatErrorCode = ERR_NISEC_DEVICE;
            strcpy(describe, "Ӳ�����豸����");
            break;
        case 0x5:
        case 0x092400:
            nFormatErrorCode = ERR_NISEC_PARAMENTER;
            strcpy(describe, "�������������");
            break;
        case 0x092403:
            nFormatErrorCode = ERR_NISEC_INCORRECT_INPUT_DATE;
            strcpy(describe, "�������ڲ��Ϸ�");
            break;
        case 0x09240d:
            // ������20221128ʵ�ʲ��Եõ�����(�ײ������汾<1106xxxxxx��TC_GetAuthKeyVersion����0������δTC_UpdateAuthKey����Ʊ�ᱨ�˴���)
            nFormatErrorCode = ERR_DEVICE_NEEDUPDATE_CONFIG;
            strcpy(describe, "��Ҫ���°�ȫ��֤��Կ");
            break;
        case 0x092000:
            nFormatErrorCode = ERR_NISEC_ILLEGAL_COMMAND;
            strcpy(describe, "�Ƿ�����");
            break;
        case 0x092100:
            nFormatErrorCode = ERR_NISEC_OVERFLOW;
            strcpy(describe, "���ݶ�д���");
            break;
        case 0x092700:
            nFormatErrorCode = ERR_NISEC_WRITE_PROTECT;
            strcpy(describe, "�豸д����");
            break;
        case 0x09d110:
            nFormatErrorCode = ERR_NISEC_UNSUPPORT_INVOICETYPE;
            strcpy(describe, "��Ʊ���Ͳ�֧��");
            break;
        case 0x09d122:
            nFormatErrorCode = ERR_NISEC_DEADLINE_DATE;
            strcpy(describe, "�������߿�Ʊ�ϴ��޶���ֹ����");
            break;
        case 0x09d121:
            nFormatErrorCode = ERR_NISEC_DEADLINE_PRICE;
            strcpy(describe, "�������߿�Ʊ�޶����");
            break;
        case 0x09d120:
            nFormatErrorCode = ERR_NISEC_DEADLINE_COUNT;
            strcpy(describe, "�������߿�Ʊ�޶�����");
            break;
        case 0x09d11f:
            nFormatErrorCode = ERR_NISEC_DEADLINE_TIME;
            strcpy(describe, "�������߿�Ʊ�޶�ʱ��");
            break;
        case 0x09c000:
            nFormatErrorCode = ERR_NISEC_DEVICE_NOTOPEN;
            strcpy(describe, "�豸δ����");
            break;
        case 0x0963c7:
        case 0x0963c6:
        case 0x0963c5:
        case 0x0963c4:
        case 0x0963c3:
        case 0x0963c2:
        case 0x0963c1:
        case 0x0963c0:
            //���һ���ֽڵ�8λ��ʣ�ೢ�Դ�������Ҫ����ȥ 0x0963cX
            //'ECode:0x'Ϊ�ؼ���,��ȡ����ʣ�����ʱ�������ж�,���������޸�
            nFormatErrorCode = ERR_NISEC_CERTPASSWORD_INCORRECT;
            sprintf(describe, "֤��������ECode:0x%x", wRepCode);
            break;
        case 0x09d106:
        case 0x09d200:  //��Ʊ�ָ����Ժ󣬵õ��ô���ֵδ�޸�������
            nFormatErrorCode = ERR_NISEC_NO_DATA;
            strcpy(describe, "û���ҵ�ָ��������");
            break;
        case 0x096a80:
            nFormatErrorCode = ERR_NISEC_REPORT;
            strcpy(describe, "˰���̶�ʱ�ӻ�������ʧ�ܣ�ʱ��оƬ��������");
            break;
        case 0x09d109:
            nFormatErrorCode = ERR_NISEC_DEADLINE;
            strcpy(describe, "�ѵ���Ʊ��ֹ���ڣ���ֹ��Ʊ");
            break;
        case 0x09d11c:
            nFormatErrorCode = ERR_NISEC_CANNOT_WASTE;
            strcpy(describe, "�˷�Ʊ�ѱ�˰��������");
            break;
        case 0x09d108:
            nFormatErrorCode = ERR_NISEC_INCORRECT_FPDMHM;
            strcpy(describe, "��Ʊ��Ϣ�еķ�Ʊ���롢���벻�Ϸ�");
            break;
        case 0x09e105:
            nFormatErrorCode = ERR_NISEC_CLOCK_DAMAGE;
            strcpy(describe, "ʱ�Ӵ���");
            break;
        default:  //�и��������Ķ�̬�������������,'ECode:0x'Ϊ�ؼ��֣����������޸�
            sprintf(describe, "�豸�ײ����ECode:0x%x", wRepCode);
            break;
    }
    //  _WriteLog(LL_WARN, "Nisec/Cntax LogicIO error, code:0x%x desc:%s", wRepCode, describe);
    return nFormatErrorCode;
}

int GetNisecDeviceLastErrorCode(HUSB hUSB, char *szOutErrorCode)
{
    int nRet = 0;
    char szErrorCode[512] = {0};
    char describe[512] = {0};
    int nErrorCode = 0;
    uint v2 = 0, dwDeviceLastErrorCode = 0;  // dword_3BAF30
    uint8 cb[256] = {0};
    memcpy(cb, "\x03\x00\x00\x00\x60", 5);
    if ((nRet = NisecLogicIORaw(hUSB, cb, 5, sizeof(cb), NULL, 0)) < 16)
        return -1;
    if (0xa != cb[7] && 0xc != cb[7])  // nisec after len == 10, cntax == 12
        return -2;
    //�ٷ��Ĵ�������ʽ�������ܶ�
    dwDeviceLastErrorCode = bswap_32(*(uint32 *)(cb + 10));
    if (dwDeviceLastErrorCode) {
        v2 = (unsigned int)dwDeviceLastErrorCode >> 16;
        if (BYTE1(v2))
            sprintf(szErrorCode, "%.8x", dwDeviceLastErrorCode);
        else
            sprintf(szErrorCode, "09%.4x", dwDeviceLastErrorCode % 0xFFFFu);
    } else
        sprintf(szErrorCode, "%.2x", dwDeviceLastErrorCode);
    sscanf(szErrorCode, "%x", &nErrorCode);
    int nFormatErrorCode = NisecVendorErrorCodeToDescription(nErrorCode, describe);
    SetLastError(hUSB, nFormatErrorCode, "%s,%s", szErrorCode, describe);
    return 0;
}

//ԭ�����ͣ����Ķ�USBCmd
int NisecLogicIORaw(HUSB hUSB, uint8 *pInOutCmd, int nInCmdLen, int nInCmdBufLen,
                    uint8 *pTransferData, int nTransferDataLen)
{
    // pInOutCmd 0xfe0x6f �̶�
    int nRet = 0;
    uint8 cmd[24] = {0};
    if (nInCmdLen > 16)
        return -1;
    memcpy(cmd, pInOutCmd, nInCmdLen);
    uint8 bTransferArrow = MSC_DIR_D2H;
    if (pTransferData)
        bTransferArrow = MSC_DIR_H2D;
    nRet = mass_storage_io(hUSB, cmd, nInCmdLen, nInCmdBufLen, bTransferArrow, pTransferData,
                           nTransferDataLen, (uint8 *)pInOutCmd, nInCmdBufLen);
    if (-100 == nRet)  // SCSCI ����
        GetNisecDeviceLastErrorCode(hUSB, hUSB->szLastErrorDescription);
    return nRet;
}

// pInOutCmd�������������ݻᱻ�޸ģ����ǲ���Խ���޸�
int NisecLogicIO(HUSB hUSB, uint8 *pInOutCmd, int nInCmdLen, int nInCmdBufLen, uint8 *pTransferData,
                 int nTransferDataLen)
{
    memset(pInOutCmd + nInCmdLen, 0, 16 - nInCmdLen);
    if (pTransferData)
        pInOutCmd[0xe] = BuildTransferVCode(pTransferData, nTransferDataLen);
    return NisecLogicIORaw(hUSB, pInOutCmd, 16, nInCmdBufLen, pTransferData, nTransferDataLen);
}


int CntaxLogicIO(HUSB hUSB, uint8 *pInOutCmd, int nInCmdLen, int nInCmdBufLen, uint8 *pTransferData,
                 int nTransferDataLen)
{
    memset(pInOutCmd + nInCmdLen, 0, 16 - nInCmdLen);
    if (pTransferData)
        pInOutCmd[0xe] = BuildTransferVCode(pTransferData, nTransferDataLen);
    return NisecLogicIORaw(hUSB, pInOutCmd, 16, nInCmdBufLen, pTransferData, nTransferDataLen);
}