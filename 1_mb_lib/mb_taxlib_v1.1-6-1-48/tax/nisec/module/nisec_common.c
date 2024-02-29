/*****************************************************************************
File name:   nisec_common.c
Description: ���ļ���Ҫ����Ա�д����ĺ�������ͽṹ
             ͳһ���Ҳ��������ģ����ã����ļ��в����漰����ҵ���߼�����
Author:      Zako
Version:     1.0
Date:        2019.03
History:
20190319     �������ʵ��
*****************************************************************************/
#include "nisec_common.h"

//�򻯰棬��IO
int NisecSelectInvoiceTypeMini(HUSB hUSB, uint8 bInvoiceType)
{
#ifdef _NODEVICE_TEST
    return RET_SUCCESS;
#endif
    uint8 cb[32];
    memcpy(cb, "\xfe\x21\x00", 3);
    cb[2] = bInvoiceType;
    if (NisecLogicIO(hUSB, cb, 3, sizeof(cb), NULL, 0) < 0)
        return -1;
    return RET_SUCCESS;
}

//��Ҫ�ȿ������л����л����ͬ��HDEV���ڵ�ǰ��Ʊ��������Ч
//�ú������������hdev��ȷ���ͷ�Ʊ������ص���Ϣ�õ���䡣ע�⣡����ÿ�ν���ȡ��Ҫ�������ݣ�δ���������ݽ�����֮ǰ��ȡ������
//���ڽ��ԭ������ȡ�����������ݣ����º��ɰع�˾���������Ƭ����������� 20210930
int NisecSelectInvoiceType(HDEV hDev, uint8 bInvoiceType, enum SELECT_INVOICE_OPTION option)
{
#ifdef _NODEVICE_TEST
    return RET_SUCCESS;
#endif
    HUSB hUSB = hDev->hUSB;
    int nChildRet = 0;
    // 1--bOperateInvType,select inv type
    hDev->bOperateInvType = bInvoiceType;
    uint8 cb[256];
    memcpy(cb, "\xfe\x21\x00", 3);
    cb[2] = bInvoiceType;
    if (NisecLogicIO(hUSB, cb, 3, sizeof(cb), NULL, 0) < 0)
        return ERR_IO_FAIL;
    // 2--QueryCurInvoTemplate
    if (option & SELECT_OPTION_TEMPLATE) {
        memcpy(cb, "\xfe\x22", 2);
        nChildRet = NisecLogicIO(hUSB, cb, 2, sizeof cb, NULL, 0);
        if (nChildRet < 16)
            return ERR_IO_FAIL;
        if (0xc0 != cb[0])
            return ERR_IO_REPCHECK;
        nChildRet =
            nChildRet < sizeof(hDev->abInvTemplate) ? nChildRet : sizeof(hDev->abInvTemplate);
        memcpy((LPVOID)hDev->abInvTemplate, cb, nChildRet);
        // TC_QueryCurInvoTemplate �����и�������������������������⣬�ٵ�����
        if (7 == hDev->abInvTemplate[3]) {  //��Ʊǿ�Ʒ�Ʊ���볤��12λ
            hDev->abInvTemplate[7] = 6;
            hDev->abInvTemplate[15] = 6;
        }
    }
    // 3--pubserv,���ּ��������䵽hdev
    if (option & SELECT_OPTION_MONITOR) {
        struct StMonitorInfo stMonitorInfo;
        if (GetMonitorInfo(hUSB, &stMonitorInfo) < 0)
            return ERR_LOGIC_MONITOR_FAIL;
        strcpy(hDev->szPubServer, stMonitorInfo.ggfwpt);
    }
    // 4--��ȡ�����豸ʱ��
    if (option & SELECT_OPTION_TIME) {
        if (NisecGetDeviceTime(hUSB, hDev->szDeviceTime) < 0)
            return ERR_DEVICE_GET_TIME;
    }
    return RET_SUCCESS;
}

//[�����ؿ����Ϻ���]�����������ϸ�����Ŀ����ؿ���һ��APIһ�ο����ؿ������еĹ��ܵ���ǰ����Ҫ���ø����
//\xfe\x16\x00|01ΪTC_GetAuthKeyVersion���������ֵΪ0����"����汾����˰�����豸��ȫ�汾�����������"
int NisecEntry(HUSB hUSB, uint8 bOPType)
{
#ifdef _NODEVICE_TEST
    return 0;
#endif
    if (NULL == hUSB)
        return -1;
    char szPwd[] = DEF_DEVICE_PASSWORD_DEFAULT;
    uint8 cb[512];
    int nRet = -2;
    switch (bOPType) {
        case NOP_CLOSE:
            memcpy(cb, "\xfe\x02", 2);  // close
            if (NisecLogicIO(hUSB, cb, 2, sizeof(cb), NULL, 0) < 0)
                break;
            nRet = 0;
            break;
        case NOP_OPEN:
            memcpy(cb, "\xfe\x01", 2);  // open
            int nChildRet = NisecLogicIO(hUSB, cb, 2, sizeof(cb), NULL, 0);
            if (nChildRet != 10)
                break;
            nRet = 0;
            break;
        case NOP_OPEN_FULL: {
            memcpy(cb, "\xfe\x01", 2);  // open
            int nChildRet = NisecLogicIO(hUSB, cb, 2, sizeof(cb), NULL, 0);
            if (nChildRet != 10)
                break;
            memcpy(cb, "\xfe\x23\x01", 3);  // select tax app
            nChildRet = NisecLogicIO(hUSB, cb, 3, sizeof(cb), NULL, 0);
            if (nChildRet != 0)
                break;
            // check pin
            memcpy(cb, "\xfe\x12", 2);
            nChildRet = NisecLogicIO(hUSB, cb, 2, sizeof(cb), (uint8 *)szPwd, strlen(szPwd));
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

//��ȡ����������Ҫ�� SN taxid name
int NisecGetBasicInfo(HDEV hDev, char *szOutSN, char *szOutTaxID, char *szOutCusName)
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

//�����豸ʱ�䣬��׼��ʽ20190101151515,szOut����������16�ֽ�
int NisecGetDeviceTime(HUSB hUSB, char *szOutTime)
{
#ifdef _NODEVICE_TEST
    PTData pTD = (PTData)hUSB->pTestData;
    strcpy(szOutTime, pTD->hDev->szDeviceTime);
    return 0;
#endif
    uint8 cb[24] = {0};
    int nRet = ERR_GENERIC;
    do {
        memcpy(cb, "\xfe\x04", 2);
        int nChildRet = NisecLogicIO(hUSB, cb, 3, sizeof cb, NULL, 0);
        if (nChildRet != 7)
            break;
        sprintf((char *)szOutTime, "%02x%02x%02x%02x%02x%02x%02x", cb[0], cb[1], cb[2], cb[3],
                cb[4], cb[5], cb[6]);
        if (14 != strlen(szOutTime))
            break;
        nRet = RET_SUCCESS;
    } while (false);
    return nRet;
}

//��ȡ�豸�� ��ɢ����=TC_GetRTCRand��RTCʱ���������
int NisecGetDeviceFsyz(HUSB hUSB, char *szOutFsyz)
{
    uint8 cb[256] = {0};
    char szCommand[] = "GETRND\n";
    int nRet = ERR_GENERIC, nChildRet = 0;
    while (1) {
        memcpy(cb, "\xfe\x3b\x21", 3);
        if (0 != NisecLogicIO(hUSB, cb, 3, sizeof cb, (uint8 *)szCommand, strlen(szCommand)))
            break;
        memcpy(cb, "\xfe\x08", 2);
        if ((nChildRet = NisecLogicIO(hUSB, cb, 2, sizeof cb, NULL, 0)) < 16)
            break;
        Byte2Str(szOutFsyz, cb, 16);
        if (strlen(szOutFsyz) != 32)
            break;
        nRet = RET_SUCCESS;
        break;
    }
    return nRet;
}

void FillDInfoFromManageData2(HDEV hDev, uint8 *cb, int ncbLen)
{
    char cbHex[768] = {0};
    char szReserve[128] = "";  //�����ֶ�
    Byte2Str(cbHex, cb, ncbLen);
    // SN
    memset(hDev->szDeviceID, 0, sizeof hDev->szDeviceID);
    strncpy(hDev->szDeviceID, (char *)cbHex + 6, 12);
    // TaxID
    memset(hDev->szCommonTaxID, 0, sizeof hDev->szCommonTaxID);
    NisecBaseConv((char *)cbHex + 18, 26, 0x10, hDev->szCommonTaxID, 0x24,
                  sizeof hDev->szCommonTaxID);
    MemoryFormart(MF_REMOVE_HEADZERO, hDev->szCommonTaxID, strlen(hDev->szCommonTaxID));
    // qysj
    strncpy(hDev->szDeviceEffectDate, cbHex + 44, 14);
    // qylx
    memset(szReserve, 0, sizeof szReserve);
    strncpy(szReserve, cbHex + 58, 2);
    // kpjh
    memset(szReserve, 0, sizeof szReserve);
    strncpy(szReserve, cbHex + 60 + 2, 2);
    uint32 uICCardNo;
    sscanf(szReserve, "%x", (uint32 *)&uICCardNo);
    hDev->uICCardNo = (uint16)uICCardNo;
    //_WriteLog(LL_INFO, "�ֻ��ţ�%d", hDev->uICCardNo);

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
        hDev->bNatureOfTaxpayer = 2;  //��ֵ˰һ����˰��
    else if (!strcmp(szReserve, "02"))
        hDev->bNatureOfTaxpayer = 9;  //������һ����˰��
    else
        hDev->bNatureOfTaxpayer = 0;  //"04"=�ض����ղ���/����˰�����
}

void FillDInfoFromManageData3(HDEV hDev, uint8 *cb, int ncbLen)
{
    char cbHex[768] = {0};
    char abBuf[256] = {0};
    char szReserve[128] = "";  //�����ֶ�
    Byte2Str(cbHex, cb, ncbLen);
    // swjgdmex
    NisecBaseConv(cbHex + 6, 10, 0x10, (char *)hDev->szTaxAuthorityCodeEx, 0xa, 0xc);
    // szRegCode--�ϱ���ַѡ����ݴ���,�ǹٷ�����ʵ��,��������
    if (!strncmp(hDev->szTaxAuthorityCodeEx, "20", 2)) {  //����4λ,nisecΪ�����ϻ�������
        strncpy(hDev->szRegCode, hDev->szTaxAuthorityCodeEx + 4, 6);
        strcpy(hDev->szTaxAuthorityCode, hDev->szTaxAuthorityCodeEx + 3);
    } else {  //����2λ
        strncpy(hDev->szRegCode, hDev->szTaxAuthorityCodeEx + 2, 6);
        strcpy(hDev->szTaxAuthorityCode, hDev->szTaxAuthorityCodeEx + 1);
    }
    // swjgmc
    memset(abBuf, 0, sizeof abBuf);
    memcpy(abBuf, cb + 8, 80);  // 16��160�̶��ֽڳ���
    int nChildRet = MemoryFormart(MF_TRANSLATE_SPACE2ZERO, abBuf, 80);
    G2U_GBKIgnore(abBuf, nChildRet, hDev->szTaxAuthorityName, sizeof hDev->szTaxAuthorityName);
    // device customer name
    memset(abBuf, 0, sizeof abBuf);
    memcpy(abBuf, cb + 88, 80);  // 16��160�̶��ֽڳ���
    nChildRet = MemoryFormart(MF_TRANSLATE_SPACE2ZERO, abBuf, 80);
    G2U_GBKIgnore(abBuf, nChildRet, hDev->szCompanyName, sizeof hDev->szCompanyName);
    // unknow 168 8byte
    // hydm 4710סլ���ݽ���
    memset(abBuf, 0, sizeof abBuf);
    memcpy(abBuf, cb + 176, 80);  // 16��160�̶��ֽڳ���
    nChildRet = MemoryFormart(MF_TRANSLATE_SPACE2ZERO, abBuf, 80);
    G2U_GBKIgnore(abBuf, nChildRet, szReserve, sizeof szReserve);
}

inline void FillAllInvType(HDEV hDev, uint8 *cb)
{
    uint8 bInvTypeCount = *(cb + 1);
    if (!bInvTypeCount || bInvTypeCount > 8)
        return;
    int ture_cout = 0;
    int type_cout = 0;
    for (type_cout = 0; type_cout < bInvTypeCount; type_cout++) {
        uint8 bInvType = *(cb + 2 + type_cout);
        if ((bInvType == FPLX_COMMON_ZYFP) || (bInvType == FPLX_COMMON_JDCFP) ||
            (bInvType == FPLX_COMMON_ESC) || (bInvType == FPLX_COMMON_PTFP) ||
            (bInvType == FPLX_COMMON_HYFP) || (bInvType == FPLX_COMMON_JSFP) ||
            (bInvType == FPLX_COMMON_DZFP) || (bInvType == FPLX_COMMON_DZZP)) {
            ture_cout += 1;
            hDev->abSupportInvtype[ture_cout] = bInvType;
        }
    }
    hDev->abSupportInvtype[0] = ture_cout;
    // memcpy(hDev->abSupportInvtype, cb + 1, bInvTypeCount + 1);  //�������1+7
}

uint8 CheckDeviceSupportInvType(uint8 *abSupportInvtypeBuf, uint8 bInvType)
{
    uint8 bRet = 0;
    uint8 bInvTypeCount = *(abSupportInvtypeBuf);
    int i = 0;
    for (i = 0; i < bInvTypeCount; i++) {
        if (bInvType == abSupportInvtypeBuf[1 + i]) {
            bRet = 1;
            break;
        }
    }
    return bRet;
}

int GetNisecDeviceVersion(HUSB hUSB, char *szOut)
{
    // 1007180713
    uint8 cb[64];
    int nRet = 0;
    memcpy(cb, "\xfe\x2b", 2);
    nRet = NisecLogicIO(hUSB, cb, 2, sizeof cb, NULL, 0);
    if (nRet < 5 || nRet > 8)
        return -1;
    Byte2Str(szOut, cb, nRet);
    return 0;
}

int GetDiskEnvFromManageData(HDEV hDev)
{
#ifdef _NODEVICE_TEST
    return 0;
#endif
    HUSB hUSB = hDev->hUSB;
    uint8 cb[512] = {0};
    int nRet = -3;
    int nChildRet = -1;
    while (1) {
        // ###manage data
        //\xfe\x06\x01 \xfe\x06\x06 ��Ҫָ��invtype
        // ###manage data2
        memcpy(cb, "\xfe\x06\x02", 3);
        nChildRet = NisecLogicIO(hUSB, cb, 3, sizeof cb, NULL, 0);
        if (nChildRet < 32)
            break;  // 0200255399002104910000000bbe2476a3eaad611f85201903110000000000000000010100000000
        FillDInfoFromManageData2(hDev, cb, nChildRet);
        // ###manage data3
        memcpy(cb, "\xfe\x06\x03", 3);
        nChildRet = NisecLogicIO(hUSB, cb, 3, sizeof cb, NULL, 0);
        if (nChildRet < 250)
            break;  // 0300fd2e99dea181d6d8c7eccad0b9fabcd2cbb0cef1bed62020202020202020...
        FillDInfoFromManageData3(hDev, cb, nChildRet);
        // ##QueryAllInvoType
        memcpy(cb, "\xfe\x1f", 2);
        nChildRet = NisecLogicIO(hUSB, cb, 2, sizeof cb, NULL, 0);
        if (nChildRet < 4)  //����һ����Ʊ����
            break;          // (01 01 04) (02)
        FillAllInvType(hDev, cb);
        //����һЩ����Ҫ���¿��ؿ��Ϳ��Ի�ȡ����Ϣ
        if (GetNisecDeviceVersion(hUSB, hDev->szDriverVersion) < 0)
            break;

        nRet = 0;
        break;
    }
    return nRet;
}

int EncryptOpenDevice(HUSB hUSB, uint8 bDeviceType, char *szDeviceID)
{
    uint8 cb[512];
    uint8 abCryptKey[48] = {0};
    char szPrefixDeviceID[4] = {0};
    int nChildRet = -1, nRet = -1, nIOBytes = 0;

    while (1) {
        if (DEVICE_NISEC == bDeviceType) {
            strcpy(szPrefixDeviceID, "33");
            nIOBytes = 3;
            memcpy(cb, "\xfe\x6f\x01", nIOBytes);
        } else if (DEVICE_CNTAX == bDeviceType) {
            strcpy(szPrefixDeviceID, "66");
            nIOBytes = 4;
            memcpy(cb, "\xfe\x6f\x01\x01", nIOBytes);
        } else
            break;
        if ((nChildRet = NisecLogicIO(hUSB, cb, nIOBytes, sizeof(cb), NULL, 0)) < 12)
            break;

        memcpy(cb, "\xfe\x6f\x04\x10", 4);
        if ((nChildRet = NisecLogicIO(hUSB, cb, 4, sizeof(cb), NULL, 0)) < 12)
            break;
        if (Build16ByteCryptKey_DeviceOpen(szPrefixDeviceID, szDeviceID, cb, abCryptKey) < 0)
            break;
        memcpy(cb, "\xfe\x6f\x05", 3);
        if ((nChildRet = NisecLogicIO(hUSB, cb, 3, sizeof(cb), abCryptKey, 0x10)) < 0)
            break;
        nRet = 0;
        break;
    }
    if (nRet < 0)
        _WriteLog(LL_FATAL, "EncryptOpenDevice failed");
    return nRet;
}

int NisecEnumCertsMem(HUSB hUSB, void *pRetBuff)
{
    uint8 cb[2560];
    int nChildRet = -1, i = 0, nRetBuffIndex = 2;
    uint16 nCertNum = 0;
    for (i = 2; i >= 0; i--) {  // 3��֤��
        memcpy(cb, "\xfe\x6f\x0d\x01", 4);
        if ((nChildRet = NisecLogicIO(hUSB, cb, 4, sizeof(cb), NULL, 0)) < 0)
            break;
        memcpy(cb, "\xfe\x6f\x0f\x00\x00\x01\x09", 7);
        if ((nChildRet = NisecLogicIO(hUSB, cb, 7, sizeof(cb), NULL, 0)) < 0)
            break;
        memcpy(cb, "\xfe\x6f\x1a\x00", 4);
        cb[3] = i;
        if ((nChildRet = NisecLogicIO(hUSB, cb, 4, sizeof(cb), NULL, 0)) < 0)
            break;
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
    memcpy(pRetBuff, &nCertNum, 2);
    return nCertNum;
}

int GetDeviceCert(HDEV hDev)
{
    if (hDev->pX509Cert == NULL) {
        if (EncryptOpenDevice(hDev->hUSB, hDev->bDeviceType, hDev->szDeviceID) < 0)
            return -1;
        X509 *pCert = GetX509Cert(hDev->hUSB, hDev->bDeviceType, false);
        if (!pCert) {
            _WriteLog(LL_FATAL, "Get cert failed!,last errinfo =��%s \r\n", hDev->hUSB->errinfo);
            return -2;
        }
        hDev->pX509Cert = pCert;
        // check is test Device
        X509_NAME *pCommonName = X509_get_issuer_name(pCert);
        if (!pCommonName)
            return -3;
        int nNameLen = 64;
        char csCommonName[64] = {0};
        nNameLen = X509_NAME_get_text_by_NID(pCommonName, NID_commonName, csCommonName, nNameLen);
        if (-1 == nNameLen)
            return -4;
        // ukey��ͳһΪ
        // ˰�����֤���������(SM2)����������ʽ�̶�Ϊһ��֤�飬���ukey�޷�����֤�����ж��Ƿ������
        if (!memcmp(csCommonName,
                    "\x4e\x2d\x56\xfd\x7a\x0e\x52\xa1\x8b\xa4\x8b\xc1\x4e\x2d\x5f\xc3", 16))
            hDev->bTrainFPDiskType = 1;  //�й�˰����֤���� ������
        else {
            //����˰���ܾ�Ϊ��ʽ��
            // pass
        }
        if (StaticCheckTrainDeviceByDeviceID(hDev->szDeviceID))
            hDev->bTrainFPDiskType = 1;
    }
    //_WriteLog(LL_DEBUG, "Is a train device:[%s]\r\n",
    //          hDev->bTrainFPDiskType == 1 ? "True" : "False");
    return 0;
}

int LoadNisecDevInfo(HDEV hDev, HUSB hUSB, uint8 bAuthDev)
{
    int nRet = ERR_GENERIC;
    hDev->hUSB = hUSB;
    hDev->bDeviceType = DEVICE_NISEC;
    bool bNeedCloseDevice = true;
#ifdef _NODEVICE_TEST
    if (LoadNoDeviceJson(hDev) < 0)
        return ERR_GENERIC;
    return RET_SUCCESS;
#endif
    do {
        if (NisecEntry(hUSB, NOP_OPEN_FULL) < 0) {
            bNeedCloseDevice = false;
            nRet = ERR_DEVICE_OPEN;
            break;
        }
        if (NisecGetDeviceTime(hUSB, hDev->szDeviceTime) < 0) {
            nRet = ERR_DEVICE_GET_TIME;
            break;
        }
        if (NisecGetDeviceFsyz(hDev->hUSB, hDev->szFsyz))
        {
             _WriteLog(LL_INFO, "CntaxGetDeviceTime szFsyz failed");
            nRet = ERR_DEVICE_GET_TIME;
            break;
        }
        if (GetDiskEnvFromManageData(hDev) < 0) {
            nRet = ERR_DEVICE_GET_ENV;
            break;
        }
        if (bAuthDev && GetDeviceCert(hDev) < 0) {
            nRet = ERR_DEVICE_GET_CERT;
            break;
        }
        if ((nRet = LoadAllInvoiceTypeProperty(hDev)) < 0) {
            nRet = ERR_DEVICE_GET_ENV;
            break;
        }
        if (CheckDeviceSupportInvType(hDev->abSupportInvtype, FPLX_COMMON_DZFP)) {
            HDITI invEnv = NULL;
            invEnv = GetInvoiceTypeEnvirnment(hDev, FPLX_COMMON_DZFP);
            if (invEnv == NULL) {
                _WriteLog(LL_INFO, "��ȡ������Ϣʧ��,��Ʊ���ͣ�%03d", FPLX_COMMON_DZFP);
                nRet = ERR_DEVICE_GET_ENV;
                break;
            }
            strcpy(hDev->szPubServer, invEnv->monitor.ggfwpt);
        }
        if (bAuthDev && !GetDeviceQmcs(hDev, hDev->szSignParameter)) {
            nRet = ERR_DEVICE_GET_SIGNPARAMETER;
            break;
        }
        nRet = RET_SUCCESS;
    } while (false);
    if (bNeedCloseDevice)
        NisecEntry(hUSB, NOP_CLOSE);

    if (nRet)
        _WriteLog(LL_FATAL, "LoadNisecDevInfo failed,nRet = %d", nRet);
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

// nisec��cntax���ýӿڡ�szInOutFPDM szInOutFPHM
// ���Բ�ָ��ֵ,ֱ���Զ�ѡ��һ�ŷ�Ʊ;���ָ��ֵ,��ô��������ָ����Ʊ��������Ƿ�Ϊ��һ��
int CommonGetCurrentInvCode(HDEV hDev, char *szInOutFPDM, char *szInOutFPHM,
                            char *szOptOutInvEndNum)
{
#ifdef _NODEVICE_TEST
    PTData pTD = (PTData)hDev->hUSB->pTestData;
    strcpy(szInOutFPDM, pTD->szFPDM);
    strcpy(szInOutFPHM, pTD->szFPHM);
    return 0;
#endif
    bool bGiveFPDMHM = false;
    if (strlen(szInOutFPHM) || strlen(szInOutFPDM)) {
        bGiveFPDMHM = true;
        _WriteLog(LL_INFO,
                  "Check input fpdm and fphm is the top of invoice volume, fpdm:%s fphm:%s",
                  szInOutFPDM, szInOutFPHM);
    }

    struct StStockInfoHead *stMIInf =
        (struct StStockInfoHead *)malloc(sizeof(struct StStockInfoHead));
    int nRet = -2;
    while (1) {
        memset(stMIInf, 0, sizeof(struct StStockInfoHead));
        if (DEVICE_NISEC == hDev->bDeviceType) {
            if (GetStockInfo(hDev, stMIInf) < 0)
                break;
        } else if (DEVICE_CNTAX == hDev->bDeviceType) {
            if (CntaxGetStockInfo(hDev, stMIInf) < 0)
                break;
        } else {
            break;
        }

        if (!bGiveFPDMHM) {
            strcpy(szInOutFPDM, stMIInf->dqfpdm);
            strcpy(szInOutFPHM, stMIInf->dqfphm);
            strcpy(szOptOutInvEndNum, stMIInf->dqfpzzhm);
            if (NULL != stMIInf->pArray_StStockInfoGroup)
                free(stMIInf->pArray_StStockInfoGroup);
            nRet = 0;
            break;
        } else {  //ָ��dm hm��Ʊ����Ҫ���
            int i = 0;
            for (i = 0; i < stMIInf->nGroupCount; i++) {
                struct StStockInfoGroup *pSI =
                    (struct StStockInfoGroup *)(stMIInf->pArray_StStockInfoGroup +
                                                i * sizeof(struct StStockInfoGroup));
                if (!pSI->syfs)
                    continue;
                if (strcmp(szInOutFPDM, pSI->fpdm))
                    continue;
                if (strcmp(szInOutFPHM, pSI->dqhm))
                    continue;
                // has found, FPDM FPHM OK
                strcpy(szOptOutInvEndNum, pSI->zzhm);
                nRet = 0;
                break;
            }
            if (NULL != stMIInf->pArray_StStockInfoGroup)
                free(stMIInf->pArray_StStockInfoGroup);
            break;
        }
    }
    free(stMIInf);
    if (!nRet)
        _WriteLog(LL_DEBUG, "GetCurrentInvCode bNeedType:%d fpdm:%s fphm:%s", hDev->bOperateInvType,
                  szInOutFPDM, szInOutFPHM);
    else
        _WriteLog(LL_WARN, "GetCurrentInvCode failed");
    return nRet;
}

void Transfer9ByteToFPDMAndFPHM(uint8 *pInBuf, char *szInOutFPDM, char *szInOutFPHM)
{
    uint64 u64FPDMSwap = 0;
    uint32 nFPHMSwap = 0;
    memcpy((uint8 *)&u64FPDMSwap, pInBuf, 5);
    u64FPDMSwap = bswap_64(u64FPDMSwap) >> 24;
    memcpy(&nFPHMSwap, pInBuf + 5, 4);
    nFPHMSwap = bswap_32(nFPHMSwap);
    sprintf(szInOutFPDM, "%llu", u64FPDMSwap);
    sprintf(szInOutFPHM, "%u", nFPHMSwap);
}

void TransferFPDMAndFPHMTo9Byte(char *szInFPDM, char *szInFPHM, uint8 *pOutBuf)
{  // pOutBuf 11bytes
    uint64 u64FPDMSwap = bswap_64(atoll(szInFPDM));
    uint64 u64FPHMSwap = bswap_64(atoll(szInFPHM));
    memcpy(pOutBuf, (uint8 *)&u64FPDMSwap + 3, 5);
    memcpy(pOutBuf + 5, (uint8 *)&u64FPHMSwap + 4, 4);
}

//����ֵ<=0Ϊʣ����������<-100�߼�����>0ִ�гɹ�
int Nisec_Crypt_Password(HUSB hUSB, char *szDeviceID, char *szCertPassword, char *szNewCertPassword)
{
    //�����ǰloaddev��ʱ��crypt_deviceopen��δִ�У�������ܻ���Ҫ����ִ��һ��
    uint8 cb[512] = {0}, abPreHashBegin8[24] = {0}, abStep2_16BytesCMD[24] = {0},
          abStep2_16bytesParament[24] = {0}, abCryptKey[48] = {0}, abUSBCmdVCode[32] = {0};
    char szBuf[64] = "";
    int nChildRet = -1;
    int nRet = -100;
    bool bLoginOrSetPassword = true;
    if (NULL != szNewCertPassword)
        bLoginOrSetPassword = false;
    while (1) {
        memcpy(cb, "\xfe\x6f\x04\x08", 4);
        if ((nChildRet = NisecLogicIORaw(hUSB, cb, 4, sizeof(cb), NULL, 0)) != 8) {
            nRet = DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
            break;
        }
        memcpy(abPreHashBegin8, cb, 8);
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
        sprintf(szBuf, "33%s", szDeviceID);
        if (Build16ByteCryptKeyAnd4ByteVCode(szBuf, szCertPassword, abPreHashBegin8,
                                             abStep2_16BytesCMD, abStep2_16bytesParament,
                                             abCryptKey, abUSBCmdVCode) < 0) {
            nRet = DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
            break;
        }

        memset(cb, 0, sizeof(cb));
        memcpy(cb, abStep2_16BytesCMD, 16);
        memcpy(cb + 11, abUSBCmdVCode, 4);
        ClearLastError(hUSB);
        if ((nChildRet = NisecLogicIORaw(hUSB, cb, 16, sizeof(cb), abCryptKey, 0x10)) < 0) {
            nRet = 0 - PasswordNotice(false, hUSB->szLastErrorDescription);
            break;
        }
        nRet = 1;
        break;
    }
    if (nRet < 0)
        _WriteLog(LL_FATAL, "Nisec_Crypt_Password failed");
    return nRet;
}

//����ֵ<=0Ϊʣ����������<-100�߼�����>0ִ�гɹ�
int Nisec_Crypt_Login(HUSB hUSB, char *szDeviceID)
{
    char szCertDefaultPassword[] = DEF_CERT_PASSWORD_DEFAULT;
    return Nisec_Crypt_Password(hUSB, szDeviceID, szCertDefaultPassword, NULL);
}

//��Ҫȷ���ѵ���EncryptOpenDevice���򿪼�����, old:Nisec_SignAPI_ChangePassword
//����ֵ<=0Ϊʣ����������<-100�߼�����>0ִ�гɹ�
int NisecChangeCertPassword(HUSB hUSB, char *szDeviceID, char *szOldPassWord, char *szNewPassWord)
{
    // login with oldpwd
    int nRet = 0;
    if ((nRet = Nisec_Crypt_Password(hUSB, szDeviceID, szOldPassWord, NULL)) <= 0) {
        _WriteLog(LL_FATAL, "NisecChangeCertPassword failed 1");
        return nRet;
    }
    if ((nRet = Nisec_Crypt_Password(hUSB, szDeviceID, szOldPassWord, szNewPassWord)) <= 0) {
        _WriteLog(LL_FATAL, "NisecChangeCertPassword failed 2");
        return nRet;
    }
    return 1;
}

int RemoveTailSpeicalChar(char *pInStr, int nInLen)
{
    //����һ����δȷ������ת����nisec��Ʊ������Ὣ\n�滻Ϊ\r\n
    //Ŀǰ�����������������������ʱ��������ת��
    uint8 *v3 = NULL;
    char result = *pInStr;
    char *v2 = pInStr;
    char *v5 = NULL;
    int nNewLen = nInLen;

    do {
        if (0x20 == result || (result >= 0x9 && result <= 0xd)) {
            if (!v3)
                v3 = (uint8 *)v2;
        } else {
            v3 = 0;
        }
        v5 = v2 + 1;
        v2 = v5;
        result = *v5;
    } while (result);
    if (v3) {
        *v3 = 0;
        nNewLen = (char *)v3 - pInStr;
    }
    return nNewLen;
}

//����ַ�����ʽ���Ƕ����Ƹ�ʽ����Ʊ���ݵĵ���Ŀ����
int FormatTemplate(bool bUtf8Buf, char *pDetailBuff, int nIndex, char *pOutBuf, int nOutBufSize)
{
    memset(pOutBuf, 0, nOutBufSize);
    uint8 szBuf[768] = "";
    char szBuf2[1024] = "";
    int nHeadNum = bUtf8Buf ? 4 : 2;
    strncpy((char *)szBuf, pDetailBuff + nIndex, nHeadNum);
    long lDataNum = strtol((char *)szBuf, 0, 16) << 1;
    uint8 bOK = 0;
    while (1) {
        if (!lDataNum) {
            bOK = 1;
            break;
        }
        if (lDataNum / 2 > sizeof(szBuf))
            break;
        memset(szBuf, 0, sizeof szBuf);
        int nChildRet = Str2Byte(szBuf, pDetailBuff + nIndex + nHeadNum, lDataNum);
        if (!nChildRet)
            break;
        //�ٷ���Ʊ������Ƴ�ǰ�������ַ����ҷ�ҲҪ����һ�𡣾��������������޸��û����ݣ���������ҷ�������ǩ���Ļ��ᵼ��ǩ����һ��
        // 20200104ע��
        // RemoveTailSpeicalChar ������������� �������Ǻ�˰���㷨�Բ��ϣ�����������Ƴ�ǰ�������ַ�
        // ˰���ǲ��Ƴ��� �Ͱ���һ�� �ͺ�˰�ֶԲ��� ��ʾǩ������ ��˰��һ�°�������ʾǩ������
        // ���Դ����ĺ����������Ǵ���Ʊ�ˣ�����ֻ���ڵ���ʱ����ǰ�������ַ�
        // �����Ϊ�����ú��������޷��ϱ������Խ��øú�����ʱ�ϱ����ŷ�Ʊ�������ϱ���ɻ��ǵûָ�����������Ӱ��������Ʊ
        nChildRet = RemoveTailSpeicalChar((char *)szBuf, nChildRet);
        //����ֱ�������pOutBuf������nOutBufSize�պúͽ������������һ���������ͻ�����
        if (nChildRet * 2 > sizeof(szBuf2))
            break;
        if (!bUtf8Buf) {  // gbk
            if ((nChildRet = G2U_GBKIgnore((char *)szBuf, nChildRet, szBuf2, sizeof(szBuf2))) < 0 ||
                nChildRet > nOutBufSize) {
                _WriteLog(LL_WARN, "Nisec FormatTemplate f1");
                break;
            }
        } else {  // utf8
            if ((nChildRet = U2G_UTF8Ignore((char *)szBuf, nChildRet, szBuf2, sizeof(szBuf2))) <
                    0 ||
                nChildRet > nOutBufSize) {
                _WriteLog(LL_WARN, "Nisec FormatTemplate f2");
                break;
            }
        }
        strcpy(pOutBuf, szBuf2);
        bOK = 1;
        break;
    }
    if (!bOK)
        _WriteLog(LL_WARN, "Nisec FormatTemplate failed");
    // else
    //      _WriteLog(LL_DEBUG, "Nisec FormatTemplate:%s", strlen(pOutBuf) ? pOutBuf : "[NULL]");
    nIndex += nHeadNum + lDataNum;
    return nIndex;
}

uint8 GetDeviceBBH(char *szDeviceVersion)
{
    uint8 bVersion = 0;
    int v89 = 0;
    char szHead[4] = "";
    strncpy(szHead, szDeviceVersion, 2);
    v89 = atoi(szHead);
    if (v89 > 10 && v89 < 20)
        bVersion = 1;
    if (v89 > 20) {
        if (v89 >= 30)
            goto LABEL_55;
        bVersion = 1;
    }
LABEL_55:
    //... ��������˵
    return bVersion;
}

// old:GetInvTemplateValue
uint8 NisecGetInvTemplateValue(uint8 *abInvTemplate, uint8 bType, uint8 bIndex)
{
    // bType bIndex 0,1==fpdm'len 1,1=fphm'len
    uint8 *pBuf = NULL;
    int i = 0;
    uint8 bRet = 0xff;
    while (true) {
        pBuf = abInvTemplate + 6 + 4 * i;
        if (pBuf[0] == bType) {
            bRet = pBuf[bIndex];
        }
        // end
        if (0xff == pBuf[0])
            break;
        i++;
    }
    return bRet;
}

//����ֵΪ���ַ�������
int NisecBaseStrExpand(uint8 *abInvTemplate, char **pszStr, int *pnStrBufLen, int nIndex)
{
    char *szStr = *pszStr;
    int nStrLen = strlen(szStr);
    int bExpandLen = 0, bOrigLen = 0, nSubValue = 0;
    if (nStrLen * 2 + 512 > *pnStrBufLen) {
        char *p = realloc(*pszStr, nStrLen * 2 + 512);
        if (p)
            *pszStr = p;
        else
            return 0;
        *pnStrBufLen = nStrLen * 2 + 512;
        szStr = *pszStr;
    }
    bOrigLen = abInvTemplate[8] * 2;
    bExpandLen = abInvTemplate[7] * 2;
    nSubValue = bExpandLen - bOrigLen;
    if (nSubValue) {
        memmove(szStr + nIndex + bExpandLen, szStr + nIndex + bOrigLen,
                nStrLen - nIndex - bOrigLen + 1);  //+1�и�ĩβ�ַ���
        nStrLen += nSubValue;
    }
    NisecBaseConv(szStr + nIndex, bOrigLen, 16, szStr + nIndex, 10, bExpandLen);
    nIndex += bExpandLen;

    bOrigLen = abInvTemplate[12] * 2;
    bExpandLen = abInvTemplate[11] * 2;
    nSubValue = bExpandLen - bOrigLen;
    if (nSubValue) {
        memmove(szStr + nIndex + bExpandLen, szStr + nIndex + bOrigLen,
                nStrLen - nIndex - bOrigLen + 1);  //+1�и�ĩβ�ַ���
        nStrLen += nSubValue;
    }
    NisecBaseConv(szStr + nIndex, bOrigLen, 16, szStr + nIndex, 10, bExpandLen);
    return nIndex += bExpandLen;
}

//�ٷ�������������Ҫ����ԭFPDM 10λ�ϸ�ʽ
void RemovePrefixTwoZero(char *szInput)
{
    if (strncmp(szInput, "00", 2))
        return;
    int nNewLen = strlen(szInput) - 2;
    memmove(szInput, szInput + 2, nNewLen);
    szInput[nNewLen] = '\0';
}

void NisecFPZTAnalyze(int nFPZT, HFPXX fpxx)
{
    uint8 bInvoiceStatus = nFPZT & 0x7f;
    switch (bInvoiceStatus) {
        case FPSTATUS_BLUE:
            break;
        case FPSTATUS_RED:
            fpxx->isRed = 1;
            break;
        case FPSTATUS_BLANK_WASTE:
            fpxx->zfbz = 1;
            break;
        case FPSTATUS_BLUE_WASTE:
            fpxx->zfbz = 1;
            break;
        case FPSTATUS_RED_WASTE:
            fpxx->isRed = 1;
            fpxx->zfbz = 1;
            break;
    }
    fpxx->bIsUpload = nFPZT & 0x80 ? 1 : 0;  // scbz
    fpxx->fpzt = bInvoiceStatus;
}

//����Ʊ�����޺ð취����Ŀǰ��ͨ����ע���ж�
void DetectSpecialFplxFromBZ(HFPXX fpxx)
{
    char *szBZ = fpxx->bz;
    char *p = strstr(szBZ, "�����˰��");
    if (p && strstr(p, "��") && (strstr(p, "��") - p < 64))
        fpxx->zyfpLx = ZYFP_CEZS;
}

// windowsƽ̨���0x7FFF��linuxƽ̨���0x7FFFFFFF������޷��͹ٷ�һ�£�ֻҪ����һ��8�ֽ����������
//�㷨������ͬ�����ɸ�ʽһ�¼��ɣ�����޷��͹ٷ����һ��
void Build8BytesRandomTQM(char *szOutBuf)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    int nUS = tv.tv_sec * 1000000 + tv.tv_usec;  //΢��
    int a2 = 8;                                  //����2
    int i;
    int v4;
    // nisec�ı���������time(0)�������һ��������Ʊ��ô����һ��������ȡ��Ҳһ������������޸�
    srand(nUS);
    for (i = 0; i < a2; ++i) {
        v4 = rand() % 9;
        if (i % 2)
            sprintf(szOutBuf + i, "%d", v4);
        else
            sprintf(szOutBuf + i, "%c", v4 + 65);
        srand(rand() % 100);
    }
}

//<0 IO error; ==0 not found; > 0 found
int CheckInvoiceExistAtDevice(HDEV hDev, char (*dst)[768])
{
    int ret = 0;
    struct StStockInfoHead stMIInf;
    memset(&stMIInf, 0, sizeof(struct StStockInfoHead));
    int nRet = 0;
    if (hDev->bDeviceType == DEVICE_NISEC) {
        nRet = GetStockInfo(hDev, &stMIInf);
    } else if (hDev->bDeviceType == DEVICE_CNTAX) {
        nRet = CntaxGetStockInfo(hDev, &stMIInf);
    } else {
        return ERR_DEVICE_UNSUPPORT;
    }
    if (nRet < 0)
        return -1;

    uint8 *pArrayCount = stMIInf.pArray_StStockInfoGroup;
    int i = 0;
    for (i = 0; i < stMIInf.nGroupCount; i++) {
        struct StStockInfoGroup *pSI =
            (struct StStockInfoGroup *)(pArrayCount + i * sizeof(struct StStockInfoGroup));
        if (strcmp(pSI->fpdm, dst[1]))
            continue;
        if (strcmp(pSI->qshm, dst[2]))
            continue;
        if (pSI->fpfs != atoi(dst[3]))
            continue;
        ret = 1;
        break;
    }
    if (NULL != stMIInf.pArray_StStockInfoGroup)
        free(stMIInf.pArray_StStockInfoGroup);
    return ret;
}

void GetNatureOfTaxpayerStr(uint8 bNatureOfTaxpayer, char *szOutString)
{
    switch (bNatureOfTaxpayer) {
        case 1:  //С��ģ��˰��
            strcpy(szOutString, "08");
            break;
        case 2:  //��ֵ˰һ����˰��
            strcpy(szOutString, "01");
            break;
        case 3:  //ת�Ǽ���˰��
            strcpy(szOutString, "05");
            break;
        case 9:  //������һ����˰��
            strcpy(szOutString, "02");
            break;
        default:  // 0 "04"=�ض����ղ���/����˰�����
            strcpy(szOutString, "04");
            break;
    }
}

//�����ѷ��������ǽ���aisinoʹ��
// hsOrbhs=��˰/����˰
int GetTaxrateStatic(HDEV hDev, uint8 bInvType, bool hsOrbhs, char *szOut)
{
    // hs
    if (hsOrbhs) {
        if (bInvType == FPLX_COMMON_ZYFP || bInvType == FPLX_COMMON_DZZP)
            strcpy(szOut, "0.05000");
        else
            strcpy(szOut, "");
        return RET_SUCCESS;
    }
    // bhs
    strcpy(szOut, "");
    if (bInvType != FPLX_COMMON_ZYFP && bInvType != FPLX_COMMON_DZZP) {
        if (hDev->bDeviceType == DEVICE_CNTAX)
            strcat(szOut, "0.00000,");
        else if (hDev->bDeviceType == DEVICE_NISEC)
            strcat(szOut, "0,");
    }
    switch (hDev->bNatureOfTaxpayer) {
        case 1:  //С��ģ
            strcat(szOut, "0.01000,0.01500,0.03000,0.05000");
            break;
        case 2:  //һ��
        case 9:  //������һ��
            strcat(szOut, "0.01500,0.03000,0.04000,0.05000,0.06000,0.09000,0.13000");
            break;
        default:  //����˰����� ת�Ǽ�
            strcat(szOut, "0.01000,0.01500,0.03000,0.04000,0.05000,0.06000,0.09000,0.10000,0."
                          "11000,0.13000,0.16000,0.17000");
            break;
    }
    return RET_SUCCESS;
}

// szOut�������������128�ֽ�
int GetTaxrate(char *szInTaxrateConfig, bool hsOrbhs, char *szOut)
{
    char szBuf[512] = "";
    if (strlen(szInTaxrateConfig) > sizeof(szBuf) - 1)
        return ERR_PROGRAME_API;
    strcpy(szBuf, szInTaxrateConfig);
    char *p = strchr(szBuf, '|');
    if (!p)
        return ERR_PROGRAME_API;
    *p = '\0';
    if (hsOrbhs)
        strcpy(szOut, szBuf);
    else
        strcpy(szOut, p + 1);
    return RET_SUCCESS;
}

int SetDeviceInvoiceTypeInfo(char *lxkzxx, struct DeviceExtendInfo *extInfo)
{
    // bmtsqybs
    extInfo->bmtsqybs = GetUint8FromString(lxkzxx + 0);
    // xgmkjzpbs
    extInfo->xgmkjzpbs = GetUint8FromString(lxkzxx + 2);  //�ٷ��ѷϳ�
    // esjdcbs
    extInfo->esjdcbs = GetUint8FromString(lxkzxx + 4);
    // cpybs
    extInfo->cpybs = GetUint8FromString(lxkzxx + 6);
    // cpybsyxq
    strncpy(extInfo->cpybsyxq, lxkzxx + 8, 16);
    // cpybmdbs
    extInfo->cpybmdbs = GetUint8FromString(lxkzxx + 24);
    // cpybmdbsyxq
    strncpy(extInfo->cpybmdbsyxq, lxkzxx + 26, 16);
    // tzqy nisec��Ϊ������ҵ?
    extInfo->tdqy = GetUint8FromString(lxkzxx + 42);
    // jyqy
    extInfo->jyqy = GetUint8FromString(lxkzxx + 44);
    // jyqyyxq
    strncpy(extInfo->jyqyyxq, lxkzxx + 46, 16);
    // jdcqy
    extInfo->jdcqy = GetUint8FromString(lxkzxx + 62);
    // jdcqyyxq
    strncpy(extInfo->jdcqyyxq, lxkzxx + 64, 16);
    // jdcbmdbs
    extInfo->jdcbmdbs = GetUint8FromString(lxkzxx + 80);
    // jdcbmdyxq
    strncpy(extInfo->jdcbmdyxq, lxkzxx + 82, 16);
    // wtdkqy
    extInfo->wtdkqy = GetUint8FromString(lxkzxx + 98);
    // wtdkqyyxq
    strncpy(extInfo->wtdkqyyxq, lxkzxx + 100, 16);
    //���ֽ�0����һλ 116
    //
    // djzclx 3�ֽ�
    strncpy(extInfo->djzclx, lxkzxx + 117, 3);
    return RET_SUCCESS;
}

int GetNisecInvoiceTypeProperty(uint8 bInvType, HDEV hDev, HDITI hDiti)
{
    HUSB hUSB = hDev->hUSB;
    int nRet = ERR_GENERIC;
    hDiti->bInvType = bInvType;
    do {
        if (NisecSelectInvoiceTypeMini(hUSB, bInvType) < 0) {
            nRet = ERR_DEVICE_SELECTINV;
            break;
        }
        if (GetMonitorInfo(hUSB, &hDiti->monitor) < 0) {
            nRet = ERR_LOGIC_MONITOR_FAIL;
            break;
        }
        // nisec��������չ��Ϣ������Ʊ�ֺ���һ��
        //��Ʊ������׸���Ʊ����������չ��ϢΪ׼��������
        SetDeviceInvoiceTypeInfo(hDiti->monitor.lxkzxx, &hDev->stDevExtend);
        if (NisecGetTaxrateStrings(hUSB, hDiti->szTaxrate, sizeof(hDiti->szTaxrate))) {
            nRet = ERR_LOGIC_MONITOR_TAXRATE;
            break;
        }
        nRet = RET_SUCCESS;
    } while (false);
    return nRet;
}

// szOut>16bytes,ֻ��nisec�ŵ��øú���;
bool GetDeviceQmcs(HDEV hDev, char *szOut)
{
    //�ɰط��˴����ô�������,��һ��LoadDev false���ڶ���LoadDev trueʱ�쳣
    //��������nisec�����̣�Ŀǰ����̶�ֵ
    strcpy(szOut, "01b4000000000000");
    return true;
    // char *pBase64 = NULL;
    // bool ret = false;
    // char szDummy[] = "Dummy";  //���κ����壬ֻ�Ǽ���ǩ����С
    // do {
    //     if (!(pBase64 = GetDeviceSignHashBase64(hDev, szDummy, strlen(szDummy))))
    //         break;
    //     sprintf(szOut, "%04x000000000000", (int)strlen(pBase64));
    //     if (16 != strlen(szOut))
    //         break;
    //     ret = true;
    // } while (false);
    // if (pBase64)
    //     free(pBase64);
    // return ret;
}
