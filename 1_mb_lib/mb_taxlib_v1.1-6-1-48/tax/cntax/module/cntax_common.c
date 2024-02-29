/*****************************************************************************
File name:   cntax_common.c
Description: ���ļ���Ҫ����Ա�д����ĺ�������ͽṹ
             ͳһ���Ҳ��������ģ����ã����ļ��в����漰����ҵ���߼�����
Author:      Zako
Version:     1.0
Date:        2021.02
History:
20210205     �������ʵ��
*****************************************************************************/
#include "cntax_common.h"

int CntaxSelectInvoiceTypeMini(HUSB hUSB, uint8 bInvoiceType)
{
#ifdef _NODEVICE_TEST
    return 0;
#endif
    uint8 cb[32];
    memcpy(cb, "\xfe\x14\x00", 3);
    cb[2] = bInvoiceType;
    if (CntaxLogicIO(hUSB, cb, 3, sizeof(cb), NULL, 0) < 0)
        return -1;
    return 0;
}

//��Ҫ�ȿ������л����л����ͬ��HDEV���ڵ�ǰ��Ʊ��������Ч
//�ú������������hdev��ȷ���ͷ�Ʊ������ص���Ϣ�õ���䡣ע�⣡����ÿ�ν���ȡ��Ҫ�������ݣ�δ���������ݽ�����֮ǰ��ȡ������
//���ڽ��ԭ������ȡ�����������ݣ����º��ɰع�˾���������Ƭ����������� 20210930
// Cntax Ukey��selecttype��queryInvTemplate����API�Ѻϲ�
int CntaxSelectInvoiceType(HDEV hDev, uint8 bInvoiceType, enum SELECT_INVOICE_OPTION option)
{
#ifdef _NODEVICE_TEST
    return 0;
#endif
    HUSB hUSB = hDev->hUSB;
    int nChildRet = 0;
    // 1--bOperateInvType,select inv type
    hDev->bOperateInvType = bInvoiceType;
    uint8 cb[256];
    memcpy(cb, "\xfe\x14\x00", 3);
    cb[2] = bInvoiceType;
    if (CntaxLogicIO(hUSB, cb, 3, sizeof(cb), NULL, 0) < 0)
        return ERR_IO_FAIL;
    // 2--QueryCurInvoTemplate
    if (option & SELECT_OPTION_TEMPLATE) {
        memcpy(cb, "\xfe\x11\xc0", 3);
        nChildRet = CntaxLogicIO(hUSB, cb, 3, sizeof cb, NULL, 0);
        if (nChildRet < 16)
            return ERR_IO_FAIL;
        if (0xc0 != cb[0])
            return ERR_IO_REPCHECK;
        //++todo
        if (nChildRet > sizeof(hDev->abInvTemplate))  //�������068180027761 ˰�̣�237008991941
            nChildRet = sizeof(hDev->abInvTemplate) - 1;
        memcpy((LPVOID)hDev->abInvTemplate, cb, nChildRet);
    }
    // 3--pubserv,���ּ��������䵽hdev
    if (option & SELECT_OPTION_MONITOR) {
        struct StMonitorInfo stMonitorInfo;
        if (CntaxGetMonitorInfo(hUSB, &stMonitorInfo) < 0)
            return ERR_LOGIC_MONITOR_FAIL;
        strcpy(hDev->szPubServer, stMonitorInfo.ggfwpt);
    }
    // 4--��ȡ�����豸ʱ��
    if (option & SELECT_OPTION_TIME) {
        if (CntaxGetDeviceTime(hUSB, hDev->szDeviceTime) < 0)
            return ERR_DEVICE_GET_TIME;
    }
    return RET_SUCCESS;
}

int CntaxEntry(HUSB hUSB, uint8 bOPType)
{
    return CntaxEntryRetInfo(hUSB, bOPType, NULL);
}

// pDeviceOpenInfo��Ϊ�գ������С����16�ֽ�,���64
int CntaxEntryRetInfo(HUSB hUSB, uint8 bOPType, uint8 *pDeviceOpenInfo)
{
#ifdef _NODEVICE_TEST
    return 0;
#endif
    if (NULL == hUSB)
        return -1;
    // 20210206�汾֮ǰĬ���������룬�豸Ĭ�Ͽ���88888888��֤������12345678���汾֮��ͳһΪһ������12345678
    char szPwd[] = DEF_CERT_PASSWORD_DEFAULT;
    uint8 cb[512];
    int nRet = -2;
    switch (bOPType) {
        case NOP_CLOSE:
            memcpy(cb, "\xfe\x02", 2);  // close
            if (CntaxLogicIO(hUSB, cb, 2, sizeof(cb), NULL, 0) < 0)
                break;
            nRet = 0;
            break;
        case NOP_OPEN:
        case NOP_OPEN_FULL: {  //����\xfe\x01��ɣ����3λ���ؽ������λ�����ǲ�����Ϊ��������
            memcpy(cb, "\xfe\x01\x01", 3);  // open
            int nChildRet = CntaxLogicIO(hUSB, cb, 3, sizeof(cb), NULL, 0);
            if (nChildRet < 16 || nChildRet > 64)
                break;
            if (pDeviceOpenInfo)
                memcpy(pDeviceOpenInfo, cb, nChildRet);
            if (NOP_OPEN == bOPType) {
                nRet = 0;
                break;
            }
            // check pin
            memcpy(cb, "\xfe\x03", 2);
            ClearLastError(hUSB);
            nChildRet = CntaxLogicIO(hUSB, cb, 2, sizeof(cb), (uint8 *)szPwd, strlen(szPwd));
            if (nChildRet != 0) {
                PasswordNotice(true, hUSB->szLastErrorDescription);
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

//��ȡ����������Ҫ�� SN taxid name
int CntaxGetBasicInfo(HDEV hDev, char *szOutSN, char *szOutTaxID, char *szOutCusName)
{
#ifdef _NODEVICE_TEST
    PTData pTD = (PTData)hDev->hUSB->pTestData;
    strcpy(szOutSN, pTD->hDev->szDeviceID);
    strcpy(szOutTaxID, pTD->hDev->szCommonTaxID);
    strcpy(szOutCusName, pTD->hDev->szCompanyName);
    if (hDev) {
    }
#else
    strcpy(szOutSN, hDev->szDeviceID);
    strcpy(szOutTaxID, hDev->szCommonTaxID);
    strcpy(szOutCusName, hDev->szCompanyName);
#endif
    return 0;
}

// OK
//�����豸ʱ�䣬��׼��ʽ20190101151515,szOut����������16�ֽ�
int CntaxGetDeviceTime(HUSB hUSB, char *szOutTime)
{
#ifdef _NODEVICE_TEST
    PTData pTD = (PTData)hUSB->pTestData;
    strcpy(szOutTime, pTD->hDev->szDeviceTime);
    return 0;
#endif
    uint8 cb[24] = {0};
    int nRet = -2;
    while (1) {
        memcpy(cb, "\xfe\x07", 2);
        int nChildRet = CntaxLogicIO(hUSB, cb, 3, sizeof cb, NULL, 0);
        if (nChildRet != 7)
            break;
        sprintf((char *)szOutTime, "%02x%02x%02x%02x%02x%02x%02x", cb[0], cb[1], cb[2], cb[3],
                cb[4], cb[5], cb[6]);
        if (14 != strlen(szOutTime))
            break;
        nRet = 0;
        break;
    }
    return nRet;
}

//��ȡ�豸�� ��ɢ����=TC_GetRTCRand��RTCʱ���������
int CntaxGetDeviceFsyz(HUSB hUSB, char *szOutFsyz)
{
    uint8 cb[256] = {0};
    int nRet = ERR_GENERIC, nChildRet = 0;
    while (1) {
        memcpy(cb, "\xfe\x06\x10", 3);
        if ((nChildRet = CntaxLogicIO(hUSB, cb, 3, sizeof cb, NULL, 0)) != 16)
            break;
        Byte2Str(szOutFsyz, cb, 16);
        if (strlen(szOutFsyz) != 32)
            break;
        nRet = RET_SUCCESS;
        break;
    }
    return nRet;
}

void CntaxGetManageData1(HDEV hDev, uint8 *cb, int ncbLen)
{
    char cbHex[768] = {0};
    char szReserve[128] = "";  //�����ֶ�
    int nIndex = 0;
    uint32 nNum = 0;
    Byte2Str(cbHex, cb, ncbLen);
    // SN
    memset(hDev->szDeviceID, 0, sizeof hDev->szDeviceID);
    strncpy(hDev->szDeviceID, (char *)cbHex + 6, 12);
    //�Ǽ�ʱ�� ��qysj ����ʱ��) 14λ��ȡ8λ
    memset(hDev->szDeviceEffectDate, 0, sizeof hDev->szDeviceEffectDate);
    strncpy(hDev->szDeviceEffectDate, cbHex + 18, 14);
    //��ҵ����
    memset(szReserve, 0, sizeof szReserve);
    strncpy(szReserve, cbHex + 32, 2);
    if (!strcmp(szReserve, "08"))
        hDev->bNatureOfTaxpayer = 1;  //С��ģ��˰��
    else if (!strcmp(szReserve, "05"))
        hDev->bNatureOfTaxpayer = 3;  //ת�Ǽ���˰��
    else if (!strcmp(szReserve, "01"))
        hDev->bNatureOfTaxpayer = 2;  //��ֵ˰һ����˰��
    else if (!strcmp(szReserve, "02"))
        hDev->bNatureOfTaxpayer = 9;  //������һ����˰��
    else
        hDev->bNatureOfTaxpayer = 0;  //"04"=�ض����ղ���/����˰�����
    // cpyzlkz ��Ʒ������XX??
    memset(szReserve, 0, sizeof szReserve);
    strncpy(szReserve, cbHex + 34, 2);
    // kpjh size+num 2�ֽ���ɣ��п���Ϊ��0000��0100=0��0102=2
    memset(szReserve, 0, sizeof szReserve);
    strncpy(szReserve, cbHex + 36, 2);
    sscanf(szReserve, "%x", (uint32 *)&nNum);
    hDev->uICCardNo = (uint16)nNum;
    // djxh �Ǽ����
    memset(szReserve, 0, sizeof szReserve);
    strncpy(szReserve, cbHex + 38, 20);
    // kpbz ��Ʊ��־
    hDev->stDevExtend.kpbz = GetUint8FromString(cbHex + 58);
    // swjgdmex 12λ
    memset(hDev->szTaxAuthorityCodeEx, 0, sizeof hDev->szTaxAuthorityCodeEx);
    strncpy(hDev->szTaxAuthorityCodeEx, (char *)cb + 30, 12);
    // swjgdm ˰����ش���
    memset(hDev->szTaxAuthorityCode, 0, sizeof hDev->szTaxAuthorityCode);
    strcpy(hDev->szTaxAuthorityCode, hDev->szTaxAuthorityCodeEx);
    MemoryFormart(MF_TRANSLATE_SPACE2ZERO, hDev->szTaxAuthorityCode,
                  strlen(hDev->szTaxAuthorityCode));
    // regcode �������
    memset(hDev->szRegCode, 0, sizeof hDev->szRegCode);
    strncpy(hDev->szRegCode, hDev->szTaxAuthorityCodeEx + 1, 6);
    // TaxID
    memset(hDev->szCommonTaxID, 0, sizeof hDev->szCommonTaxID);
    strncpy(hDev->szCommonTaxID, (char *)cb + 42, 20);
    MemoryFormart(MF_TRANSLATE_SPACE2ZERO, hDev->szCommonTaxID, strlen(hDev->szCommonTaxID));
    // swjgmc ˰���������
    uint8 bNum = *(cb + 62);
    memset(hDev->szTaxAuthorityName, 0, sizeof hDev->szTaxAuthorityName);
    AutoAsciiToChinese((char *)cb + 63, bNum, hDev->szTaxAuthorityName,
                       sizeof(hDev->szTaxAuthorityName));
    nIndex += 63 + bNum;
    // szCompanyName
    bNum = *(cb + nIndex);
    memset(hDev->szCompanyName, 0, sizeof hDev->szCompanyName);
    AutoAsciiToChinese((char *)cb + nIndex + 1, bNum, hDev->szCompanyName,
                       sizeof(hDev->szCompanyName));
}

void CntaxGetManageData2(HDEV hDev, uint8 *cb, int ncbLen)
{
    char cbHex[768] = {0};
    Byte2Str(cbHex, cb + 4, ncbLen - 4);
    struct DeviceExtendInfo *extInfo = &hDev->stDevExtend;
    // �ض���ҵ
    extInfo->tdqy = GetUint8FromString(cbHex + 0);
    // ũ��Ʒ��ҵ
    extInfo->ncpqy = GetUint8FromString(cbHex + 2);
    // ������ҵ
    extInfo->dxqy = GetUint8FromString(cbHex + 4);
    // С��ģ����רƱ��ʶ
    extInfo->xgmkjzpbs = GetUint8FromString(cbHex + 6);
    // ���ֻ�������ʶ
    extInfo->esjdcbs = GetUint8FromString(cbHex + 8);
    // ��Ʒ�ͱ�ʶ
    extInfo->cpybs = GetUint8FromString(cbHex + 10);
    // ��Ʒ����Ч��
    strncpy(extInfo->cpybsyxq, (char *)cbHex + 12, 16);
    // ��Ʒ�Ͱ�������ʶ
    extInfo->cpybmdbs = GetUint8FromString(cbHex + 28);
    // ��Ʒ�Ͱ�������ʶ��Ч��
    strncpy(extInfo->cpybmdbsyxq, (char *)cbHex + 30, 16);
    // ϡ����ҵ
    extInfo->xtqy = GetUint8FromString(cbHex + 46);
    // NULL 1
    GetUint8FromString(cbHex + 48);
    // ������ҵ
    extInfo->jyqy = GetUint8FromString(cbHex + 50);
    // ������ҵ��Ч��
    strncpy(extInfo->jyqyyxq, (char *)cbHex + 52, 16);
    // ��������ҵ
    extInfo->jdcqy = GetUint8FromString(cbHex + 68);
    // ��������ҵ��Ч��
    strncpy(extInfo->jdcqyyxq, (char *)cbHex + 70, 16);
    // ��������������ʶ
    extInfo->jdcbmdbs = GetUint8FromString(cbHex + 86);
    // ��������������Ч��
    strncpy(extInfo->jdcbmdyxq, (char *)cbHex + 88, 16);
    // ί�д�����ҵ
    extInfo->wtdkqy = GetUint8FromString(cbHex + 104);
    // ί�д�����ҵ��Ч��
    strncpy(extInfo->wtdkqyyxq, (char *)cbHex + 106, 16);
    // �Ǽ�ע������ index:122,4�ֽڣ�ֻ����3�ֽ�(��Ϊnisec��˰�ֶ���3�ֽ�)
    strncpy(extInfo->djzclx, (char *)cbHex + 123, 3);  //��3�ֽ�
    // xgmjdczyms
    extInfo->xgmjdczyms = GetUint8FromString(cbHex + 126);
    // xgmjdczymsyxqzz
    strncpy(extInfo->xgmjdczymsyxqzz, (char *)cbHex + 128, 6);
}

int CnTaxGetDeviceEnv(HDEV hDev, HUSB hUSB, uint8 *abDeviceOpenInfo)
{
#ifdef _NODEVICE_TEST
    return 0;
#endif
    uint8 cb[512] = {0};
    int nRet = -3;
    int nChildRet = -1;
    while (1) {
        // manage1
        memcpy(cb, "\xfe\x11\x01", 3);
        nChildRet = CntaxLogicIO(hUSB, cb, 3, sizeof cb, NULL, 0);
        if (nChildRet < 64) {
            _WriteLog(LL_INFO, "1CntaxLogicIO nChildRet = %d last errinfo= %s", nChildRet,
                      hUSB->errinfo);
            break;  // 010089537100494968202001100934350800001...
        }
        CntaxGetManageData1(hDev, cb, nChildRet);
        // manage2
        memcpy(cb, "\xfe\x11\x02", 3);
        nChildRet = CntaxLogicIO(hUSB, cb, 3, sizeof cb, NULL, 0);
        if (nChildRet < 38) {  //ԭ����64�ֽڡ�ż��һ����56�ֽ�
            _WriteLog(LL_INFO, "2CntaxLogicIO nChildRet = %d last errinfo= %s", nChildRet,
                      hUSB->errinfo);
            break;  // 0200433f00000000......0020100
        }
        CntaxGetManageData2(hDev, cb, nChildRet);
        // version
        Byte2Str(hDev->szDriverVersion, abDeviceOpenInfo + 6, 7);
        // fplx support
        // UK_QueryInvoType�ӿ� ����\xfe\x13 ����02 04 1C�����Ի�ȡ֧�ֵķ�Ʊ����
        FillAllInvType(hDev, abDeviceOpenInfo + 13);
        nRet = 0;
        break;
    }
    return nRet;
}

int LoadCntaxDevInfo(HDEV hDev, HUSB hUSB, bool bAuthDev)
{
    int nRet = ERR_GENERIC;
    hDev->hUSB = hUSB;
    hDev->bDeviceType = DEVICE_CNTAX;
    bool bNeedCloseDevice = true;
    uint8 abDeviceOpenInfo[64] = {0};
#ifdef _NODEVICE_TEST
    if (LoadNoDeviceJson(hDev) < 0)
        return ERR_GENERIC;
    return RET_SUCCESS;
#endif
    do {
        if (CntaxEntryRetInfo(hUSB, NOP_OPEN_FULL, abDeviceOpenInfo) < 0) {
            bNeedCloseDevice = false;
            _WriteLog(LL_INFO, "CntaxEntryRetInfo failed");
            nRet = ERR_DEVICE_OPEN;
            break;
        }
        if (CntaxGetDeviceTime(hUSB, hDev->szDeviceTime) < 0) {
            _WriteLog(LL_INFO, "CntaxGetDeviceTime failed");
            nRet = ERR_DEVICE_GET_TIME;
            break;
        }
        if (CntaxGetDeviceFsyz(hDev->hUSB, hDev->szFsyz))
        {
             _WriteLog(LL_INFO, "CntaxGetDeviceTime szFsyz failed");
            nRet = ERR_DEVICE_GET_TIME;
            break;
        }
        if (CnTaxGetDeviceEnv(hDev, hUSB, abDeviceOpenInfo) < 0) {
            _WriteLog(LL_INFO, "CnTaxGetDeviceEnv failed");
            nRet = ERR_DEVICE_GET_ENV;
            break;
        }
        if (bAuthDev && GetDeviceCert(hDev) < 0) {
            _WriteLog(LL_INFO, "GetDeviceCert failed");
            nRet = ERR_DEVICE_GET_CERT;
            break;
        }
        if ((nRet = LoadAllInvoiceTypeProperty(hDev)) < 0) {
            _WriteLog(LL_INFO, "LoadAllInvoiceTypeProperty failed nRet = %d", nRet);
            nRet = ERR_DEVICE_GET_ENV;
            break;
        }
        if ((CheckDeviceSupportInvType(hDev->abSupportInvtype, FPLX_COMMON_DZFP)) ||
            (CheckDeviceSupportInvType(hDev->abSupportInvtype, FPLX_COMMON_DZZP))) {
            strcpy(hDev->szPubServer, "01100000000000");
        }
        nRet = RET_SUCCESS;
    } while (false);
    if (bNeedCloseDevice)
        CntaxEntry(hUSB, NOP_CLOSE);

    if (nRet)
        _WriteLog(LL_FATAL, "LoadCntaxDevInfo failed,nRet = %d\n", nRet);
#ifdef _CONSOLE
    else
        _WriteLog(LL_DEBUG,
                  "DevInfo, region:%s devid:%s taxidstr:%s szCompressTaxID:%s taxid9b:%s "
                  "cusname:%s taxauth:%s",
                  hDev->szRegCode, hDev->szDeviceID, hDev->szCommonTaxID, hDev->szCompressTaxID,
                  hDev->sz9ByteHashTaxID, hDev->szCompanyName, hDev->szTaxAuthorityName);
#endif
    return nRet;
}

int CntaxEnumCertsMem(HUSB hUSB, void *pRetBuff)
{
    uint8 cb[2560];
    int nChildRet = -1, i = 0, nRetBuffIndex = 2;
    uint16 nCertNum = 0;
    // 3��֤�� һ�����ȡ��3��
    for (i = 3; i >= 0; i--) {
        // szAppName : SM2
        // SendSCSI CMD : FE6F0101000000000000000000000000
        // SendSCSI RESP DATA : 3636323337303130313838353332
        // SendSCSI RV : 00000000
        // SendSCSI CMD : FE6F0410000000000000000000000000
        // SendSCSI RESP DATA : D6FA04F91AB42A3581BF3FB48F99168A
        // SendSCSI RV : 00000000
        // SendSCSI CMD : FE6F0500000000000000000000000000
        // SendSCSI CMD DATA : 0E8F9E974155E2CDF6D153B5F0333F3C
        // SendSCSI RV : 00000000
        // SendSCSI CMD : FE6F0B002E002C000000000000000000
        // SendSCSI RESP DATA : 534D320000000000000000...0000FFFFFFFFFFFFFFFF
        // SendSCSI RV : 00000000
        memcpy(cb, "\xfe\x6f\x01\x01", 4);
        if ((nChildRet = CntaxLogicIO(hUSB, cb, 4, sizeof(cb), NULL, 0)) < 0)
            break;
        memcpy(cb, "\xfe\x6f\x04\x10", 4);
        if ((nChildRet = CntaxLogicIO(hUSB, cb, 4, sizeof(cb), NULL, 0)) < 0)
            break;
        // szContainerName : SM2CONTAINER0002
        // SendSCSI CMD : FE6F0D01000000000000000000000000
        // SendSCSI RESP DATA :
        // SendSCSI RV : 00000000
        // SendSCSI CMD : FE6F0F00000109000000000000000000
        // SendSCSI RESP DATA : 534D3...
        // SendSCSI RV : 00000000
        // SKF_OpenContainer Leave : RV = 00000000
        // SKF_ExportPublicKey : Enter
        // SendSCSI CMD : FE6F1D00000000000000000000000000
        // SendSCSI RESP DATA : 5820E0D5CEA01AD7B87165...
        // SendSCSI RV : 00000000
        // SKF_ExportPublicKey Leave : RV = 00000000
        // SKF_VerifyPIN Enter : hApplication = 01536BE0
        // SendSCSI CMD : FE6F0B00000024000000000000000000
        // SendSCSI RESP DATA : FF...F88FF
        // SendSCSI RV : 00000000
        // SendSCSI CMD : FE6F0902000000000000000000000000
        // SendSCSI RESP DATA : 08
        // SendSCSI RV : 00000000
        // SendSCSI CMD : FE6F0901000000000000000000000000
        // SendSCSI RESP DATA : 08
        // SendSCSI RV : 00000000
        // SendSCSI CMD : FE6F0410000000000000000000000000
        // SendSCSI RESP DATA : BF51C4BFCC35F1C5068D71A0CDB90B4D
        // SendSCSI RV : 00000000
        // SendSCSI CMD : FE6F0701000000000000008828D70B00
        // SendSCSI CMD DATA : EF6D38082AA3E7E4D4A30EBDA9984FF7
        // SendSCSI RV : 00000000
        // SendSCSI CMD : FE6F0B00000024000000000000000000
        // SendSCSI RESP DATA  :FF...FFFFFFFFFFFFFFFFFFFF88FF
        // SendSCSI RV :00000000
        // SendSCSI CMD : FE6F0902000000000000000000000000
        // SendSCSI RESP DATA : 08
        // SendSCSI RV : 00000000
        // SendSCSI CMD : FE6F0901000000000000000000000000
        // SendSCSI RESP DATA : 08
        // SendSCSI RV : 00000000 SKF_VerifyPIN Leave : RV = 00000000
        // SKF_ExportCertificate : Enter
        // SendSCSI CMD : FE6F1A00000000000000000000000000
        // SendSCSI RESP DATA : 308205...
        // SendSCSI RV : 00000000
        // pbCert =  : 3082...
        memcpy(cb, "\xfe\x6f\x0d\x01", 4);
        if ((nChildRet = CntaxLogicIO(hUSB, cb, 4, sizeof(cb), NULL, 0)) < 0)
            break;
        memcpy(cb, "\xfe\x6f\x0f\x00\x00\x01\x09", 7);
        if ((nChildRet = CntaxLogicIO(hUSB, cb, 7, sizeof(cb), NULL, 0)) < 0)
            break;
        if (i <= 2) {
            memset(cb, 0, sizeof(cb));
            memcpy(cb, "\xfe\x6f\x1a\x00", 4);
            cb[3] = i;
            if ((nChildRet = CntaxLogicIO(hUSB, cb, 7, sizeof(cb), NULL, 0)) < 0) {
                break;
            } else {
                if (NULL == MyMemSearch(cb, cb + nChildRet, (uint8 *)"\x30\x82", 2))
                    continue;  //û��֤���ʶ����
                uint16 nCertLen = bswap_16(*(uint16 *)(cb + 2)) + 4;
                if (nChildRet != nCertLen)
                    continue;
                if ((nChildRet = CheckAndAppendCertBuf(pRetBuff, nRetBuffIndex, cb, nCertLen)) == 0)
                    continue;
                nCertNum++;
                nRetBuffIndex = nChildRet;
            }
        }
    }
    memcpy(pRetBuff, &nCertNum, 2);
    return nCertNum;
}


//����ֵ<=0Ϊʣ����������<-100�߼�����>0ִ�гɹ�
int Cntax_Crypt_Password(HUSB hUSB, char *szDeviceID, char *szCertPassword, char *szNewCertPassword)
{
    //�����ǰloaddev��ʱ��crypt_deviceopen��δִ�У�������ܻ���Ҫ����ִ��һ��
    uint8 cb[512] = {0}, abPreHashBegin16[24] = {0}, abStep2_16BytesCMD[24] = {0},
          abStep2_16bytesParament[24] = {0}, abCryptKey[48] = {0}, abUSBCmdVCode[32] = {0};
    char szBuf[64] = "";
    int nChildRet = -1;
    int nRet = -100;
    bool bLoginOrSetPassword = true;
    if (NULL != szNewCertPassword)
        bLoginOrSetPassword = false;
    while (1) {
        memcpy(cb, "\xfe\x6f\x04\x10", 4);  // 1
        if ((nChildRet = NisecLogicIORaw(hUSB, cb, 4, sizeof(cb), NULL, 0)) != 16)
            break;
        memcpy(abPreHashBegin16, cb, 16);
        if (bLoginOrSetPassword) {  // login
            memcpy(abStep2_16BytesCMD, "\xfe\x6f\x07\x01", 4);
            memcpy(abStep2_16bytesParament, "\x00\x00\x80", 3);
        } else {  // set password
                  // 00 09 len-1byte (password)len 0x80
            memcpy(abStep2_16BytesCMD, "\xfe\x6f\x08\x01", 4);
            memcpy(abStep2_16bytesParament, "\x00\x09", 2);
            int nNewPwdLen = strlen(szNewCertPassword);
            abStep2_16bytesParament[2] = (uint8)nNewPwdLen;
            strcpy((char *)abStep2_16bytesParament + 3, szNewCertPassword);
            abStep2_16bytesParament[3 + nNewPwdLen] = (uint8)0x80;
        }
        sprintf(szBuf, "66%s", szDeviceID);
        if (CntaxBuild16ByteCryptKeyAnd4ByteVCode(szBuf, szCertPassword, abPreHashBegin16,
                                                  abStep2_16BytesCMD, abStep2_16bytesParament,
                                                  abCryptKey, abUSBCmdVCode) < 0)
            break;

        memset(cb, 0, sizeof(cb));
        memcpy(cb, abStep2_16BytesCMD, 16);
        memcpy(cb + 11, abUSBCmdVCode, 4);
        ClearLastError(hUSB);
        if ((nChildRet = NisecLogicIORaw(hUSB, cb, 16, sizeof(cb), abCryptKey, 0x10)) < 0) {
            nRet = 0 - PasswordNotice(false, hUSB->szLastErrorDescription);
            SetLastError(hUSB, ERR_CERT_PASSWORD_ERR,
                         "֤��������󣬿���ԭ��Ϊδ�޸�Ĭ��֤�����12345678,ʣ�ೢ�Դ���%d", nRet);
            logout(INFO, "TAXLIB", "������֤", "�̺ţ�%s,������Ϣ��%s\r\n", szDeviceID,
                   hUSB->errinfo);
            // report_event(szDeviceID, "֤��������", hUSB->errinfo, nRet);
            nRet = ERR_CERT_PASSWORD_ERR;
            break;
        }
        nRet = 1;
        break;
    }
    if (nRet < 0)
        _WriteLog(LL_FATAL, "CNTAX_Crypt_Password failed");
    return nRet;
}

//��Ҫȷ���ѵ���EncryptOpenDevice���򿪼�����
//����ֵ<=0Ϊʣ����������<-100�߼�����>0ִ�гɹ�
int CntaxChangeCertPassword(HUSB hUSB, char *szDeviceID, char *szOldPassWord, char *szNewPassWord)
{
    int nRet = -100;
    // login with oldpwd
    if ((nRet = Cntax_Crypt_Password(hUSB, szDeviceID, szOldPassWord, NULL)) <= 0) {
        _WriteLog(LL_FATAL, "CntaxChangeCertPassword failed 1");
        return nRet;
    }
    if ((nRet = Cntax_Crypt_Password(hUSB, szDeviceID, szOldPassWord, szNewPassWord)) <= 0) {
        _WriteLog(LL_FATAL, "CntaxChangeCertPassword failed 2");
        return -2;
    }
    return 0;
}

//������ʱ��ԭʼ������󲻼����Դ���
int CntaxChangeDevicePassword(HUSB hUSB, char *szOldPassWord, char *szNewPassWord)
{
    uint8 cb[128], buf[32];
    int nChildRet = 0;
    if (strlen(szOldPassWord) > 8 || strlen(szNewPassWord) > 8)
        return -1;
    memset(buf, 0, sizeof(buf));
    strncpy((char *)buf, szOldPassWord, strlen(szOldPassWord));
    strncpy((char *)buf + 8, szNewPassWord, strlen(szNewPassWord));
    memcpy(cb, "\xfe\x04", 2);
    if ((nChildRet = CntaxLogicIO(hUSB, cb, 2, sizeof(cb), buf, 16)) < 0)
        return -2;
    return 0;
}

//����ֵ<=0Ϊʣ����������<-100�߼�����>0ִ�гɹ�
int CntaxCryptLogin(HUSB hUSB, char *szDeviceID)
{
    char szCertDefaultPassword[] = DEF_CERT_PASSWORD_DEFAULT;
    return Cntax_Crypt_Password(hUSB, szDeviceID, szCertDefaultPassword, NULL);
}

void Transfer10ByteToFPDMAndFPHM(uint8 *pInBuf, char *szInOutFPDM, char *szInOutFPHM)
{
    Byte2Str(szInOutFPDM, pInBuf, 6);
    Byte2Str(szInOutFPHM, pInBuf + 6, 4);
}

//��δʹ�ò��ԣ����޸�
void TransferFPDMAndFPHMTo10Byte(char *szInFPDM, char *szInFPHM, uint8 *pOutBuf)
{
    // uint64 u64FPDMSwap = bswap_64(atoll(szInFPDM));
    // uint64 u64FPHMSwap = bswap_64(atoll(szInFPHM));
    // memcpy(pOutBuf, (uint8 *)&u64FPDMSwap + 2, 6);
    // memcpy(pOutBuf + 6, (uint8 *)&u64FPHMSwap + 4, 4);
}

// mxml���ɵ�xmlȥ��ͷ��
int RemoveFullXmlHead(char *szXml)
{
    // remove
    //<?xml version="1.0" encoding="GBK"?>
    //<?xml version="1.0" encoding="UTF8"?>
    char *pStart = strchr(szXml, '<');
    char *pEnd = strchr(szXml, '>');
    if (strlen(szXml) < 64)
        return -1;
    if (!pStart || !pEnd)
        return -2;
    int nSize = pEnd - pStart;
    int nOrigSize = strlen(szXml);
    if (nSize > 40 || nSize < 0)
        return -3;
    nSize += 1;
    memmove(szXml, szXml + nSize, nOrigSize - nSize);
    szXml[nOrigSize - nSize] = '\0';
    return 0;
}

//Ϊ���ּ����ԣ�ֻҪ�κ��漰��˰��ͨ�ŵ�XML�����е�cntax mxml�ڵ�תstringʱ��Ӧ��ʹ�ø÷�������ת��
//����ֵΪ������󣬲�Ϊ��ʱ���ص�ָ����Ҫ�ֶ�free
char *CntaxGetUtf8YwxxStringFromXmlRoot(mxml_node_t *root)
{
    char *pRet = NULL;
    char *szXmlRootString = NULL;
    bool childRet = false;
    do {
        szXmlRootString = mxmlSaveAllocString(root, MXML_NO_CALLBACK);
        //��Ҫǰ�ýضϣ�ȥ��ͷ��
        if (RemoveFullXmlHead(szXmlRootString) < 0)
            break;
        int szoutlen = strlen(szXmlRootString) * 2 + 1024;
        pRet = calloc(1, szoutlen);
        //�ɰص�gbkģ����Ҫת��utf8����,linux�±�������ת��
        G2U_UTF8Ignore(szXmlRootString, strlen(szXmlRootString), pRet, szoutlen);
        childRet = true;
    } while (false);
    if (szXmlRootString)
        free(szXmlRootString);
    if (!childRet && pRet != NULL)
        free(pRet);
    return pRet;
}

int GetCntaxInvoiceTypeProperty(uint8 bInvType, HUSB hUSB, HDITI hDiti)
{
    int nRet = ERR_GENERIC;
    hDiti->bInvType = bInvType;
    do {
        if (CntaxSelectInvoiceTypeMini(hUSB, bInvType) < 0) {
            nRet = ERR_DEVICE_SELECTINV;
            break;
        }
        if (CntaxGetMonitorInfo(hUSB, &hDiti->monitor) < 0) {
            nRet = ERR_LOGIC_MONITOR_FAIL;
            break;
        }
        if ((nRet = CntaxGetTaxrateStrings(hUSB, hDiti->szTaxrate, sizeof(hDiti->szTaxrate))) < 0) {
            _WriteLog(LL_INFO, "CntaxGetTaxrateStrings failed,nRet = %d bInvType= %d", nRet,
                      bInvType);
            nRet = ERR_LOGIC_MONITOR_TAXRATE;
            break;
        }
        nRet = RET_SUCCESS;
    } while (false);
    return nRet;
}

uint8 CntaxGetInvTemplateValue(uint8 *abInvTemplate, uint8 bType, uint8 bIndex)
{
    // bType bIndex 0,1==fpdm'len 1,1=fphm'len
    return *(abInvTemplate + 6 + 4 * bType + bIndex);
}