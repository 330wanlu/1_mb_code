/*****************************************************************************
File name:   aisino_common.c
Description: ���ļ���Ҫ����Ա�д����ĺ�������ͽṹ
             ͳһ���Ҳ��������ģ����ã����ļ��в����漰����ҵ���߼�����
Author:      Zako
Version:     1.0
Date:        2019.05
History:
20190518     �������ʵ��
20220717     ���aisino�豸������Ϣδʵ�ֵĲ��ֽ��н�����ʵ�ֲ��ִ��룬��ʣ��һЩ�����
*****************************************************************************/
#include "aisino_common.h"

//�����豸ʱ�䣬��׼��ʽ20190101151515,szOut����������16�ֽ�
int AisinoGetDeivceTime(HUSB hUSB, char *szOutTime)
{
    memset(szOutTime, 0, 16);
#ifdef _NODEVICE_TEST
    PTData pTD = (PTData)hUSB->pTestData;
    strcpy(szOutTime, pTD->hDev->szDeviceTime);
    return 0;
#endif
    uint8 cb[64];
    int ncbLen = sizeof cb;
    memcpy(cb, "\xfe\xff\xcc\xbc\x53\xe1\xe3\x00\x07\x00", 10);
    int nRet = AisinoLogicIO(hUSB, cb, 10, ncbLen, NULL, 0);
    if (8 != nRet)
        return -1;
    sprintf((char *)szOutTime, "%02x%02x%02x%02x%02x%02x%02x", cb[0], cb[1], cb[2], cb[3], cb[4],
            cb[5], cb[6]);
    return 0;
}

//��ȡ����������֤�룬һ��ƴ�ڷ��Ͱ����һλ
uint8 MakePackVerityFlag(uint8 *pData, int nDataLen)
{
    int i = 0;
    uint8 bFlag = 0;
    for (i = 0; i < nDataLen; i++) {
        bFlag += pData[i];
    }
    return bFlag;
}

int TaxRateAuthorizeBin2Str(uint8 *byteIn, char *szOut)
{
    int num = *(uint16 *)byteIn;
    *(uint8 *)(byteIn + 1) = (uint8)num;
    uint8 *byte_3 = byteIn + 1;
    uint8 array[30];
    BYTE b = 0;
    int i, l;
    for (i = 1; i < num; i++) {
        array[i - 1] = byte_3[i];
        b += byte_3[i];
    }
    if (b != byte_3[num]) {
        num = 0;
    } else {
        num--;
    }
    //�����и�ð�������㷨����������ľ�������õģ���ʱ������������
    for (l = 0; l < num; l++) {
        char szTmp[8] = "";
        if (l != num - 1)
            sprintf(szTmp, "%d,", array[l] / 10);
        else
            sprintf(szTmp, "%d", array[l] / 10);
        strcat(szOut, szTmp);
    }
    return 0;
}

int AisinoStateInfoToHDEV(HDEV hDev)
{
    StateInfo info;
    if (GetStateInfo(hDev->hUSB, &info))
        return -1;
    hDev->uICCardNo = info.ICCardNo;
    strcpy(hDev->szRegCode, info.szRegCode);
    memcpy(hDev->abSupportInvtype, info.abInvTypeList, sizeof(hDev->abSupportInvtype));
    return 0;
}

void AisinoSetDeviceInvoiceTypeInfo(char *lxkzxx, struct DeviceExtendInfo *extInfo)
{
    // [0]�ض���ҵ(��������)
    extInfo->tdqy = GetOneByteFromString(lxkzxx + 0);
    // [1]ũ��Ʒ�չ���ҵ��ʶ 2���չ���ҵ 1��3��
    extInfo->ncpqy = GetOneByteFromString(lxkzxx + 1);
    // [2]DZFPQY
    //
    // [3]������ҵ
    extInfo->dxqy = GetOneByteFromString(lxkzxx + 3);
    // [4]С��ģ�Կ�רƱ��־��2020��1��31��ȫ���ſ����ñ�־����Ч
    //
    // [5] ���ֳ� 1=��Ӫ��λ 2=������λ
    extInfo->esjdcbs = GetOneByteFromString(lxkzxx + 5);
    // [6] ��Ʒ�� 1���� 2����
    extInfo->cpybs = GetOneByteFromString(lxkzxx + 6);
    // [7] ��Ʒ�Ͱ�����
    extInfo->cpybmdbs = GetOneByteFromString(lxkzxx + 7);
    // [8] ������ҵ 1������ҵ 2������ҵ
    extInfo->jyqy = GetOneByteFromString(lxkzxx + 8);
    // [9] ��������ҵ 1���������� 2���ڻ�����פ����ҵ
    extInfo->jdcqy = GetOneByteFromString(lxkzxx + 9);
    // [10] ��·��ҵ
    extInfo->tlqy = GetOneByteFromString(lxkzxx + 10);
}

//��������Ŀ����flag+size(2)+content
int AisinoExtraDeviceInfoSwitch(HDEV hDev, uint8 *pIndex)
{
    uint8 bFlag = *pIndex;
    uint16 uHeadLen = *(uint16 *)(pIndex + 1);
    char *pContent = (char *)pIndex + 3;
    char szBuf[512];
    switch (bFlag) {
        case 0x00:  //��ǰ�豸��ҵ����
        case 0x1:   // unknow
        case 0x2:   // unknow
            break;
        case 0x3: {
            //ϡ����ҵ
            // 02C2511011 ʯ������ҵ
            // 01B0932011 CORP_RATE_XTKCPSCQY
            // 01C3332011 CORP_RATE_XTYLFLQY
            // 01Y0001011 CORP_RATE_XTSMQY
            // 01B0932010 CORP_RATE_XTKCPSCQYYCX
            // 01C3332010 CORP_RATE_XTYLFLQYYCX
            // 01Y0001010 CORP_RATE_XTSMQYYCX
            memset(szBuf, 0, sizeof(szBuf));
            strncpy(szBuf, pContent, uHeadLen);
            if (!strcmp(szBuf, "02C2511011") || !strcmp(szBuf, "01B0932011") ||
                !strcmp(szBuf, "01C3332011") || !strcmp(szBuf, "01Y0001011") ||
                !strcmp(szBuf, "01B0932010") || !strcmp(szBuf, "01C3332010") ||
                !strcmp(szBuf, "01Y0001010"))
                hDev->stDevExtend.xtqy = 1;  //Ŀǰ���ܱ������
            break;
        }
        case 0x04: {  //����˰��Ȩ˰��
            memset(szBuf, 0, sizeof(szBuf));
            TaxRateAuthorizeBin2Str(pIndex + 1, szBuf);
            if (strlen(szBuf) == 0)
                break;
            break;
        }
        case 0x05: {  //��˰��Ȩ˰��
            memset(szBuf, 0, sizeof(szBuf));
            TaxRateAuthorizeBin2Str(pIndex + 1, szBuf);
            if (strlen(szBuf) == 0)
                break;
            break;
        }
        case 0x6:  //Ʊ����Ȩ
        case 0x7:  //��ҵ��Ȩ,С��ģʹ��
        case 0x8:  //����ʱ�� "20190806"
            break;
        case 0x09: {  //��˰������
            //�˴�ȡ��ʱ���߲���������ͨ��˰�ַ���0001��Ϣ����ȡ���߲���
            if (0x8 == *pContent)  // 8==С��ģ��˰��
                hDev->bNatureOfTaxpayer = 1;
            else if (0x5 == *pContent)  // 5==ת�Ǽ���˰��
                hDev->bNatureOfTaxpayer = 3;
            else  //һ��NSR
                hDev->bNatureOfTaxpayer = 2;
            //����
            if (15 == strlen(hDev->szCommonTaxID) && !strncmp(hDev->szCommonTaxID + 8, "DK", 2))
                hDev->bNatureOfTaxpayer = 0;
            // lxkzxx
            memset(szBuf, 0, sizeof(szBuf));
            strncpy(szBuf, pContent + 1, 11);
            AisinoSetDeviceInvoiceTypeInfo(szBuf, &hDev->stDevExtend);
            // bUploadDeadline = *p;
            uint8 *p = (uint8 *)pContent + uHeadLen - 5;
            // nMaxOfflineHour
            hDev->nMaxOfflineHour = *(uint32 *)(p + 1);
            break;
        }
        case 0xa:   // ��Ϊ���ˡ�������Ʊ�����Ϣ?
        case 0xb:   // ��Ϊ���ˡ�������Ʊ�����Ϣ?
        case 0x1b:  // ��ǰ�豸��ҵͨ��˰��
            break;
        case 0x1c: {  //˰��������ƴ���
            uint64 lNum = *(uint64 *)pContent % 0x10000000000;
            snprintf(hDev->szTaxAuthorityCodeEx, sizeof(hDev->szTaxAuthorityCodeEx), "%-12llu",
                     lNum);
            snprintf(hDev->szTaxAuthorityCode, sizeof(hDev->szTaxAuthorityCode), "%llu", lNum);
            int nNameLen = uHeadLen - 5;
            if (nNameLen > sizeof(hDev->szTaxAuthorityName))
                break;
            G2U_GBKIgnore(pContent + 5, nNameLen, hDev->szTaxAuthorityName,
                          sizeof(hDev->szTaxAuthorityName));
            break;
        }
        case 0x1d:  //���̵ǼǺš��������С������ʺš�ע���ַ
        case 0x1e:  //Ʊ����ҵ��Ȩ,С��ģʹ��
        case 0x1f:  //��ֵ˰ר��Ʊ��Ȩ
        case 0x20:  // unknow
        case 0x21:  //������������Ʊ�����ӷ�Ʊ����ֵ˰��ͨ��Ʊ����ʽ������ʱ�䣨�����գ�"320190806"
        case 0x22:  //��˰�ű�־
            break;
        case 0x24: {  //"@@@"
            int nCount = 0;
            char dst[4][768];
            memset(dst, 0, sizeof(dst));
            memset(szBuf, 0, sizeof(szBuf));
            strncpy(szBuf, pContent, uHeadLen);
            if ((nCount = GetSplitStringSimple(szBuf, "@", dst, 4)) < 2)
                break;
            // value1@value2@@,�г�2��,value1��value2
            if (strlen(dst[0]) && strlen(dst[1])) {
                sprintf(hDev->stDevExtend.cpybsyxq, "%s%s", dst[0], dst[1]);
            }
            if (nCount == 4 && strlen(dst[2]) && strlen(dst[3]))
                sprintf(hDev->stDevExtend.cpybmdbsyxq, "%s%s", dst[2], dst[3]);
            break;
        }
        case 0x25:  // ����ʱ�䣿 0x00"20200101"
            break;
        case 0x26: {
            // ��Ʒ�������Ϣ
            // 0x26	0x08 0x00 0x1f 0x29	0x47 0x48 0x19 0x9c 0xb1 0xa1
            //   this.list_1 = new List<string>();
            // 		num9 = BitConverter.ToUInt16(byte_3, 238 + num8);
            // 			for (int num24 = 0; num24 < (int)(num9 - 4); num24++)
            // 			{
            // 				int key = (int)byte_3[240 + num8 + num24];
            // 				string text14 = OilTools.F2CList[key];
            // 				if (!string.IsNullOrEmpty(text14))
            // 					this.list_1.Add(text14);
            // 			}
            break;
        }
        case 0x27:  //��Ʊ����˰�ʿ�Ʊ��ֹʱ��
            break;
        case 0x2d: {  //���ӷ�Ʊ���Ƿ����ù�������ƽ̨,���߰�ʽԤ��
            bool bEnable = *pContent == 0x1;
            if (!bEnable)
                break;
            uint8 bSize = *(pContent + 1);
            strncpy(hDev->szPubServer, pContent + 2, bSize);
            break;
        }
        case 0x2e:  //����רƱ��������ƽ̨
            break;
        case 0x2f:  // unknow
            break;
        case 0x30: {  //������ҵʱ��
            //@@@@0@@@0@@@4,��@�ָ�
            //	JYQYQYSJ = array3[0]; JYQYJZSJ = array3[1];
            break;
        }
        default:
            _WriteLog(LL_WARN, "New Aisino Extend-info found,flag:0x%x", bFlag);
            break;
    }
    return uHeadLen;
}

int AisinoFillExtraDeviceInfo(HDEV hDev)
{
    int ncbLen = 7680, nChildRet = ERR_GENERIC;
    uint8 *cb = (uint8 *)calloc(1, ncbLen);
    if (NULL == cb)
        return ERR_BUF_ALLOC;
    //�豸����ʱ��
    AisinoDeviceEffectDate(hDev->hUSB, hDev->szDeviceEffectDate);
    //��ȡ��չ��Ϣ
    memcpy(cb, "\xfe\xff\xcc\xbc\x53\xe1\x40\x00\x00\x00", 10);
    uint8 abTransferData[] = {0x00, 0x10, 0x10};
    nChildRet = AisinoLogicIO(hDev->hUSB, cb, 10, ncbLen, abTransferData, sizeof abTransferData);
    if (nChildRet < 256) {
        free(cb);
        return ERR_IO_FAIL;
    }
    uint8 *pIndex = cb + 2;
    while (*pIndex != 0xff) {
        nChildRet = AisinoExtraDeviceInfoSwitch(hDev, pIndex);
        pIndex = pIndex + 3 + nChildRet;
    }
    free(cb);
    return RET_SUCCESS;
}

int AisinoClose(HUSB hUSB)
{
    if (!hUSB)
        return 0;
    uint8 cb[64];
    int ncbLen = sizeof cb;
    memcpy(cb, "\xfe\xff\xcc\xbc\x53\xe1\xe8\x01\x01\x00", 10);
    int nRet = AisinoLogicIO(hUSB, cb, 10, ncbLen, NULL, 0);
    if (nRet < 0)
        return -1;
    memcpy(cb, "\xfe\xff\xcc\xbc\x53\xe1\xe6\x00\x00\x00", 10);
    nRet = AisinoLogicIO(hUSB, cb, 10, ncbLen, NULL, 0);
    if (nRet < 0)
        return -2;
    return 0;
}

int AisinoOpen(HUSB hUSB)
{
    uint8 cb[64];
    int ncbLen = sizeof cb;
    uint8 extraIn_retdataOut[16];
    char szDefaultPassword[] = "23456789";
    extraIn_retdataOut[0] = '\x08';
    memcpy(extraIn_retdataOut + 1, szDefaultPassword, strlen(szDefaultPassword));
    int nPacketLen = 1 + strlen(szDefaultPassword);
    extraIn_retdataOut[nPacketLen] = MakePackVerityFlag(extraIn_retdataOut, nPacketLen);
    nPacketLen += 1;
    memcpy(cb, "\xfe\xff\xcc\xbc\x53\xe1\xe8\x01\x00\x00", 10);
    if (AisinoLogicIO(hUSB, cb, 10, ncbLen, extraIn_retdataOut, nPacketLen) != 2)
        return -1;
    return 0;
}

//<=0 ʣ�������<-100�쳣����>0��ȷ
int SignAPI_OpenDevice(HUSB hUSB, bool bBigCryptIO, char *szPassWord)
{
    if (strlen(szPassWord) < 8)
        return -100;
    uint8 cb[256];
    uint8 pRecvBuff[256] = "";
    int nSendLen = 0;
    int nRecvBuffLen = sizeof(pRecvBuff);
    if (AisinoDevSelectFileCMD(hUSB, (uint8 *)"\x01\x4f", 2, bBigCryptIO) < 0)
        return -101;
    nSendLen = AisinoCryptSimpleTransferPacket((uint8 *)"\x00\x20\x00", (uint8 *)szPassWord,
                                               strlen(szPassWord), cb);
    AisinoCryptIO(hUSB, bBigCryptIO, cb, nSendLen, pRecvBuff, nRecvBuffLen);
    if (!memcmp(pRecvBuff, "\x00\x02\x00\x63", 4)) {
        //ʣ�༸�γ��Ի���
        uint8 bRetryCount = *(pRecvBuff + 4) & 0x0f;
        _WriteLog(LL_FATAL,
                  "Cert's password incorrect, please check. Remaining [\033[1;31m--%d--\033[0m] "
                  "times to retry",
                  bRetryCount);
        return -bRetryCount;
    } else if (!memcmp(pRecvBuff, "\x00\x02\x00\x90\x00", 5)) {  //
        //_WriteLog(LL_INFO, "Crypt login successful");
        return 1;  //��return1��˰����ukey����һ�£�����0����֤���������
    } else {
        _WriteLog(LL_FATAL, "Crypt login failed, unknow error");
        return -102;
    }
    return -103;
}

//��Ҫȷ���ѵ���SignAPI_OpenDevice���򿪼�����
int SignAPI_ChangePassword(HUSB hUSB, bool bBigCryptIO, char *szOldPassWord, char *szNewPassWord)
{
    uint8 cb[256];
    uint8 abPwdBuf[256];
    uint8 pRecvBuff[256] = "";
    int nSendLen = 0;
    int nRecvBuffLen = sizeof(pRecvBuff);
    if (AisinoDevSelectFileCMD(hUSB, (uint8 *)"\x01\x4f", 2, bBigCryptIO) < 0)
        return -1;
    memset(abPwdBuf, 0, sizeof(abPwdBuf));
    strcpy((char *)abPwdBuf, szOldPassWord);
    abPwdBuf[strlen((char *)abPwdBuf)] = 0xff;
    strcat((char *)abPwdBuf, szNewPassWord);
    nSendLen = AisinoCryptSimpleTransferPacket((uint8 *)"\x80\x5e\x01", (uint8 *)abPwdBuf,
                                               strlen((char *)abPwdBuf), cb);
    AisinoCryptIO(hUSB, bBigCryptIO, cb, nSendLen, pRecvBuff, nRecvBuffLen);
    if (!memcmp(pRecvBuff, "\x00\x02\x00\x90\x00", 5)) {  //
        _WriteLog(LL_INFO, "Cert's password change successful");
        return 0;
    } else {
        _WriteLog(LL_FATAL, "Cert's password change failed, unknow error");
        return -2;
    }
    return 0;
}

int AisinoReadTaxID(HUSB hUSB, char *szTaxID)
{
    char szComName[256] = "";
    return AisinoReadTaxIDandCustomName(hUSB, szTaxID, szComName, sizeof(szComName));
}

int AisinoReadTaxIDandCustomName(
    HUSB hUSB, char *szTaxID, char *szComName,
    size_t nComNameBufLen)  //��ʹ��size_t���ͼģ�unsigned int������mips�ܹ������Ī���δ���
{
#ifdef _NODEVICE_TEST
    PTData pTD = (PTData)hUSB->pTestData;
    strcpy(szComName, pTD->hDev->szCompanyName);
    strcpy(szTaxID, pTD->hDev->szCommonTaxID);
    return 0;
#endif
    char szComNameChar[512] = "";
    int ncbLen = 5120;
    uint8 *cb = (uint8 *)calloc(1, ncbLen);
    if (NULL == cb)
        return -1;
    memcpy(cb, "\xfe\xff\xcc\xbc\x53\xe1\x40\x00\x00\x00", 10);
    int nChild = AisinoLogicIO(hUSB, cb, 10, ncbLen, (uint8 *)"\x00\x10\x10", 3);
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
    if (G2U_GBKIgnore(szComNameChar, strlen(szComNameChar), szComName, nComNameBufLen) < 0)
        return -4;
    return 0;
}

//<0 error; == 0 small; > 0 bigio
int IsCryptBigIO(uint8 *abBanner)
{

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

//����ֵ == 0,СIO;==1 ��IO
int ReadTaxDeviceID(HUSB hUSB, char *szDevID)
{
#ifdef _NODEVICE_TEST
    PTData pTD = (PTData)hUSB->pTestData;
    strcpy(szDevID, pTD->hDev->szDeviceID);
    return 0;
#endif
    uint8 szSeed[8];
    char cb[512] = "";
    buildOriginSeed8B((uint8 *)szSeed);
    AisinoDesEcbEncrypt((uint8 *)szSeed);
    memcpy(cb, "\xf1\xf1\x00\x00\x00\x00\x02\x08\x00\xff", 10);
    int nRetChild = mass_storage_io(hUSB, (uint8 *)cb, 10, 0, MSC_DIR_H2D, (uint8 *)szSeed,
                                    sizeof(szSeed), (uint8 *)cb, sizeof(cb));
    if (nRetChild != 0)
        return -1;
    memcpy(cb, "\xf1\xf1\x00\x00\x00\x00\x01\x04\x01\xff", 10);
    nRetChild =
        mass_storage_io(hUSB, (uint8 *)cb, 10, 0, MSC_DIR_D2H, NULL, 0, (uint8 *)cb, sizeof(cb));
    if (nRetChild < 24)
        return -2;
    char *pRawID = cb + 0x12;
    pRawID[12] = '\0';
    strcpy(szDevID, pRawID);
    // check is bigio
    nRetChild = IsCryptBigIO((uint8 *)cb);
    return nRetChild;
}

//���������hDev��taxid��customname֮���ٵ��øú������ܷ��ؾ�ȷ�Ĳ�������Ϣ���ͣ�������ܷ����Ƿ������
// 0 = not train device; > 0 train-device's type
uint8 GetTrainDeviceType(HUSB hUSB, HDEV hDev)
{
#ifdef _NODEVICE_TEST
    uint8 bTrainT = 0;
    hDev->bTrainFPDiskType = bTrainT;
    return bTrainT;
#endif
    char szComNameChar[256] = "";
    char szTaxID[32] = "";
    uint8 bTrain = 0;
    while (1) {
        strcpy(szTaxID, hDev->szCommonTaxID);
        strcpy(szComNameChar, hDev->szCompanyName);
        // if (AisinoReadTaxIDandCustomName(device, szTaxID, szComNameChar, sizeof(szComNameChar)) <
        // 0)//�˴���ʡ�Ե�IO������֮ǰ�Ѿ���ȡ���洢��DEV�ṹ��
        //    break;
        if (strlen(szTaxID) < 12)
            break;
        if (strncmp(szTaxID + 6, "999999", 6))
            break;
        char szComNameB64[256] = "";
        if (Base64_Encode(szComNameChar, strlen(szComNameChar), szComNameB64) < 0)
            break;
#ifdef PROTOCOL_GBK
        if (strcmp(szComNameB64, "ur3QxcXg0bXG89K1"))  //������ѵ��ҵ GBK
            break;
#else
        if (strcmp(szComNameB64, "6Iiq5L+h5Z+56K6t5LyB5Lia"))  //������ѵ��ҵ UTF8
            break;
#endif
        bTrain = 1;
        break;
    }
    uint8 bTrainRet = 0;
    //_WriteLog(LL_DEBUG, "Is a train device:[%s]\r\n", bTrain == 1 ? "True" : "False");
    if (!bTrain && !strcmp(hDev->szCommonTaxID, hDev->szCompanyName)) {
        bTrainRet = 2;
    } else if (bTrain)
        bTrainRet = 1;
    else
        bTrainRet = 0;

    bTrain =
        StaticCheckTrainDeviceByDeviceID(hDev->szDeviceID);  //�жϲ����̸����̺ţ�˰���жϲ�׼ȷ
    if (bTrain > 0)
        bTrainRet = bTrain;
    return bTrainRet;
}

// abOutBuf must be enough
int AisinoCryptSimpleTransferPacket(uint8 *ab3BytesCmd, uint8 *pData, int nDataLen, uint8 *abOutBuf)
{
    int nRet = 0;
    memset(abOutBuf, 0, 8);  // pre 8bytes must init
    uint8 *abCmdBuf = abOutBuf + 3;

    memcpy(abCmdBuf, ab3BytesCmd, 3);
    abCmdBuf[3] = 0x01;
    abCmdBuf[4] = (uint8)nDataLen;
    memcpy(abCmdBuf + 5, pData, nDataLen);
    nRet = 5 + nDataLen;

    memcpy(abOutBuf + 1, (uint8 *)&nRet, 2);  // copy 2bytes;
    nRet += 3;
    return nRet;
}

//����ֵΪ����ȡ�ļ�������,��������readfile��ȡ
int AisinoDevSelectFileCMD(HUSB hUSB, uint8 *abCmd, int nCmdLen, bool bBigCryptIO)
{
    uint8 abRecvBuff[1280];
    int i = 0, nRet = 0, nChildRet = 0;
    int nLoopCount = nCmdLen / 2;
    uint8 abBuf[64] = {0};
    uint8 *abCmdBuf = abBuf + 3;
    uint8 *v11 = abCmdBuf + 4;
    abCmdBuf[0] = 0;
    abCmdBuf[1] = 0xA4u;
    if (nLoopCount == 1)
        abCmdBuf[2] = 0;
    else
        abCmdBuf[2] = 8;
    abCmdBuf[3] = 0;
    *v11 = (uint8)(2 * nLoopCount);
    nRet = 5;
    for (i = 0; i < nLoopCount; ++i) {
        *((_BYTE *)v11 + 2 * i + 1) = *(_WORD *)(abCmd + 2 * i) >> 8;
        *((_BYTE *)v11 + 2 * i + 2) = *(_WORD *)(abCmd + 2 * i);
        nRet += 2;
    }
    memcpy(abBuf + 1, (uint8 *)&nRet, 2);  // copy 2bytes;
    nRet += 3;
    nChildRet = AisinoCryptIO(hUSB, bBigCryptIO, abBuf, nRet, abRecvBuff, sizeof abRecvBuff);
    if (nChildRet < 0)  //ѡ��application
        return -1;
    nRet = 0;
    memcpy(&nRet, abRecvBuff + nChildRet - 4, 2);
    nRet = bswap_16(nRet);
    return nRet;  // to read's data len
}

int AisinoDevReadFileCMD(HUSB hUSB, int nNeedReadLen, uint8 *pOutBuf, bool bBigCryptIO)
{
    __int16 a4 = 0;
    int nChildRet = 0, v9, nOpSize, nOutBufIndex = 0, nSingleBufLen;
    unsigned int v8, nCmdLen;
    uint8 abCmdBuf[64], abRecvBuff[2200] = {0};
    uint8 *pCmdBuf = abCmdBuf + 3;

    if (bBigCryptIO)
        v8 = 2044;
    else
        v8 = 240;
    v9 = 0;
    while (nNeedReadLen) {  // a5
        memset(abCmdBuf, 0, sizeof abCmdBuf);
        if (nNeedReadLen <= v8)
            nOpSize = nNeedReadLen;
        else
            nOpSize = v8;
        pCmdBuf[0] = 0x00;
        pCmdBuf[1] = 0xb0;
        pCmdBuf[2] = (v9 + a4) >> 8;
        pCmdBuf[3] = v9 + a4;
        if (bBigCryptIO) {
            pCmdBuf[4] = BYTE1(nOpSize);
            pCmdBuf[5] = nOpSize;
            nCmdLen = 6;
        } else {
            pCmdBuf[4] = nOpSize;
            nCmdLen = 5;
        }
        memcpy(abCmdBuf + 1, (uint8 *)&nCmdLen, 2);  // copy 2bytes;
        nCmdLen += 3;
        nChildRet =
            AisinoCryptIO(hUSB, bBigCryptIO, abCmdBuf, nCmdLen, abRecvBuff, sizeof abRecvBuff);
        if (nChildRet < 0)  //ѡ��application
            return -1;
        nSingleBufLen = *(uint16 *)(abRecvBuff + 1) - 2;  // remove 0x90 00
        memcpy(pOutBuf + nOutBufIndex, abRecvBuff + 3, nSingleBufLen);
        nOutBufIndex += nSingleBufLen;

        v9 += nOpSize;
        nNeedReadLen -= nOpSize;
    }
    return nOutBufIndex;
}

//��������ָ����������������������
int AisinoReadCryptZone(HUSB hUsb, uint8 *pWordCmd, uint8 *pRecvBuf, int nRecvBufSize,
                        bool bBigCryptIO)
{
    int nChildRet = 0;
    // pre query
    AisinoDevSelectFileCMD(hUsb, (uint8 *)"\x01\x4f", 2, bBigCryptIO);
    // query table
    nChildRet = AisinoDevSelectFileCMD(hUsb, pWordCmd, 2, bBigCryptIO);
    if (nChildRet < 0 || nChildRet > nRecvBufSize)
        return -1;
    if ((nChildRet = AisinoDevReadFileCMD(hUsb, nChildRet, pRecvBuf, bBigCryptIO)) < 0)
        return -2;
    return nChildRet;
}

int AisinoReadCerts(HUSB hUsb, uint8 *pWordCmd, uint8 *pRetBuff, int *pnRetBuffIndex,
                    uint16 *pnCertNum, bool bBigCryptIO)
{
    uint16 wCmdFlag = *(uint16 *)pWordCmd;
    if (0xffff == wCmdFlag || 0 == wCmdFlag)
        return -1;
    unsigned char ca100002[16] = {0x63, 0x00, 0x61, 0x00, 0x31, 0x00, 0x30, 0x00,
                                  0x30, 0x00, 0x30, 0x00, 0x30, 0x00, 0x32, 0x30};
    int nChildRet = 0;
    uint8 *pRawinvWith2BytesSize = NULL;
    uint8 abSingleCertBuf[4096] = {0};
    nChildRet =
        AisinoReadCryptZone(hUsb, pWordCmd, abSingleCertBuf, sizeof(abSingleCertBuf), bBigCryptIO);
    if (nChildRet <= 0)
        return -2;
    if (NULL == MyMemSearch(abSingleCertBuf, abSingleCertBuf + nChildRet, (uint8 *)ca100002,
                            sizeof(ca100002)))
        return -3;  //û��֤���ʶca100002����
    //�Ƿ��cotainer��ʶ������ukey��,�°汾aisino�����豸ʹ��
    char szContainerFlag[] = "My Subject";
    uint8 *pFlag = NULL;
    if (!(pFlag = MyMemSearch(abSingleCertBuf, abSingleCertBuf + 256, (uint8 *)szContainerFlag,
                              strlen(szContainerFlag))))
        pRawinvWith2BytesSize = abSingleCertBuf + 24;  //������
    else
        pRawinvWith2BytesSize = pFlag + strlen(szContainerFlag) + 17;
    //���뵥��֤��
    uint16 nCertLen = *(uint16 *)pRawinvWith2BytesSize;
    if (nCertLen > nChildRet)
        return -4;  // cert's length can not large with buf's length
    if ((nChildRet = CheckAndAppendCertBuf(pRetBuff, *pnRetBuffIndex, pRawinvWith2BytesSize + 2,
                                           nCertLen)) == 0)
        return -5;
    // appaend
    *pnRetBuffIndex = nChildRet;
    *pnCertNum += 1;
    return nChildRet;
}

int AisinoEnumCertsMem(HUSB hUsb, void *pRetBuff, bool bBigCryptIO)
{
    int nIndexTableLen = 0, i = 0, nRetBuffIndex = 2;
    uint16 nCertNum = 0;
    uint8 abIndexTable[2048] = {0};
    //����024e����,��������ٳ���014e
    if ((nIndexTableLen = AisinoReadCryptZone(hUsb, (uint8 *)"\x02\x4e", abIndexTable,
                                              sizeof(abIndexTable), bBigCryptIO)) < 0)
        return -1;
    //���ص�������,���ּ������򲻴�������,��Ҫͨ��һ�����㷨�����ж�
    //�㷨�Ƚϸ���,����ֱ�ӳ��Զ�ȡ���м�������,�������ݽ���֤���ж�
    while (1) {
        uint16 *pCmd = (uint16 *)(abIndexTable + i + 8);
        AisinoReadCerts(hUsb, (uint8 *)pCmd, pRetBuff, &nRetBuffIndex, &nCertNum, bBigCryptIO);
        pCmd = (uint16 *)(abIndexTable + i + 6);
        AisinoReadCerts(hUsb, (uint8 *)pCmd, pRetBuff, &nRetBuffIndex, &nCertNum, bBigCryptIO);

        i += 0x12;
        if (i >= nIndexTableLen)
            break;
    }
    memcpy(pRetBuff, &nCertNum, 2);
    return nCertNum;
}

//����Ʊǩ���ַ������͵�˰�̲���ȡ��Ӧֵ
int InvSignStrToDev128ByteRep(HUSB hUSB, bool bBigCryptIO, const char *sSignStrGBKBuff,
                              int nGBKBuffLen, char *pRecvBuff, int nRecvBuffLen)
{
    //�滻�����䷢�ߺ�֤�����л���ǩ����Ϣ
    uint8 abDevSignIOModel[136] = {
        0x00, 0x85, 0x00, 0x80, 0xB8, 0x50, 0x03, 0x80, 0x00, 0x01, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0x00, 0x30, 0x21, 0x30, 0x09, 0x06, 0x05, 0x2B, 0x0E, 0x03, 0x02, 0x1A,
        0x05, 0x00, 0x04, 0x14, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11,
        0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11};  // 0x11,Ϊ����ַ�,���ᱻ�滻
    // https://tools.ietf.org/rfc/rfc3110.txt
    //  "prefix" is the ASN.1 BER SHA1 algorithm designator prefix required in PKCS1 [RFC2437],
    //  that is hex: 30 21 30 09 06 05 2B 0E 03 02 1A 05 00 04 14
    uint8 digest[SHA_DIGEST_LENGTH];
    if (CalcSHA1((void *)sSignStrGBKBuff, nGBKBuffLen, digest))
        return -1;
    //ѡ��application�������е��̼����Բ��ã�û��δ����ǩ������
    if (AisinoCryptIO(hUSB, bBigCryptIO, (uint8 *)"\x00\x07\x00\x00\xa4\x00\x00\x02\x4f\x01", 10,
                      (uint8 *)pRecvBuff, nRecvBuffLen) < 0)
        return -4;
    memcpy(abDevSignIOModel + sizeof(abDevSignIOModel) - 20, digest, 20);
    int nChildRet = AisinoCryptIO(hUSB, bBigCryptIO, abDevSignIOModel, sizeof(abDevSignIOModel),
                                  (uint8 *)pRecvBuff, nRecvBuffLen);
    if (nChildRet != 133) {
        if (nChildRet == 5) {
            _WriteLog(LL_WARN, "Open cert failed, maybe cert password incorrect. We can not signed "
                               "hash\r\n");
        }
        return -4;
    }
    return 0;
}

//��ȡ�ײ�Ӳ�������汾��
int GetDriverVersion(HUSB hUSB, char *szOutVersion, uint32 *nNumVerion)
{
#ifdef _NODEVICE_TEST
    PTData pTD = (PTData)hUSB->pTestData;
    strcpy(szOutVersion, pTD->hDev->szDriverVersion);
    return 0;
#endif
    char szBuf[64] = "";
    char szBuf2[64] = "";
    uint8 cb[128];
    int ncbLen = sizeof cb;
    memcpy(cb, "\xfe\xff\xcc\xbc\x53\xe1\xe3\x00\x30\x00", 10);
    int nRet = AisinoLogicIO(hUSB, cb, 10, ncbLen, NULL, 0);
    if (nRet < 8 || nRet > 32)
        return -1;
    strncpy(szBuf, (char *)cb, nRet - 1);

    memcpy(cb, "\xfe\xff\xcc\xbc\x53\xe1\x1a\x00\x00\x00", 10);
    nRet = AisinoLogicIO(hUSB, cb, 10, ncbLen, NULL, 0);
    if (nRet != 7)
        return -2;
    Byte2Str(szBuf2, cb, 6);
    memset(cb, 0, sizeof(cb));
    if (strlen(szBuf2) > 8)
        strncpy((char *)cb, szBuf2 + 2, 6);
    else
        strcpy((char *)cb, szBuf2);
    sprintf(szOutVersion, "%s-%s", szBuf, (char *)cb);
    *nNumVerion = atoi((char *)cb);
    return 0;
}

//��ȡ�ײ���ܰ汾��
int GetJMBBH(HUSB hUSB, char *szJMBBH)
{
    char szDefaultVer[] = "11";
    strcpy(szJMBBH, szDefaultVer);
#ifdef _NODEVICE_TEST
    strcpy(szJMBBH, "17");
    return RET_SUCCESS;
#endif
    uint8 cb[512];
    int ncbLen = sizeof cb;
    memcpy(cb, "\xfe\xff\xcc\xbc\x53\xe1\xe3\x00\x14\x00", 10);
    int nRet = AisinoLogicIO(hUSB, cb, 10, ncbLen, NULL, 0);
    if (nRet < 8 || nRet > 500)
        return ERR_BUF_CHECK;
    sprintf(szJMBBH, "%d", (uint8)cb[0]);
    return RET_SUCCESS;
}

//��ȡ�豸����ʱ��
int AisinoDeviceEffectDate(HUSB hUSB, char *szOutTime)
{
#ifdef _NODEVICE_TEST
    strcpy(szOutTime, "202001010000");
    return RET_SUCCESS;
#endif
    uint8 cb[64];
    int ncbLen = sizeof cb;
    memcpy(cb, "\xfe\xff\xcc\xbc\x53\xe1\xe3\x00\x1e\x00", 10);
    if (AisinoLogicIO(hUSB, cb, 10, ncbLen, NULL, 0) != 4)
        return ERR_BUF_CHECK;
    sprintf(szOutTime, "%d%02d010000", *(uint16 *)cb, (uint8)cb[2]);
    return RET_SUCCESS;
}

int CommonFPLX2AisinoFPLX(uint8 fplxdm, uint8 *aisino_fplxdm)
{
    switch (fplxdm) {
        case FPLX_COMMON_ZYFP:
            *aisino_fplxdm = FPLX_AISINO_ZYFP;
            break;
        case FPLX_COMMON_JDCFP:
            *aisino_fplxdm = FPLX_AISINO_JDCFP;
            break;
        case FPLX_COMMON_ESC:
            *aisino_fplxdm = FPLX_AISINO_ESC;
            break;
        case FPLX_COMMON_PTFP:
            *aisino_fplxdm = FPLX_AISINO_PTFP;
            break;
        case FPLX_COMMON_JSFP:
            *aisino_fplxdm = FPLX_AISINO_JSFP;
            break;
        case FPLX_COMMON_DZFP:
            *aisino_fplxdm = FPLX_AISINO_DZFP;
            break;
        case FPLX_COMMON_HYFP:
            *aisino_fplxdm = FPLX_AISINO_HYFP;
            break;
        case FPLX_COMMON_DZZP:
            *aisino_fplxdm = FPLX_AISINO_DZZP;
            break;
        default:
            *aisino_fplxdm = 0xff;
            break;
    }
    return 0;
}

int AisinoFPLX2CommonFPLX(uint8 aisino_fplxdm, uint8 *fplxdm)
{
    switch (aisino_fplxdm) {
        case FPLX_AISINO_ZYFP:
            *fplxdm = FPLX_COMMON_ZYFP;
            break;
        case FPLX_AISINO_JDCFP:
            *fplxdm = FPLX_COMMON_JDCFP;
            break;
        case FPLX_AISINO_ESC:
            *fplxdm = FPLX_COMMON_ESC;
            break;
        case FPLX_AISINO_PTFP:
            *fplxdm = FPLX_COMMON_PTFP;
            break;
        case FPLX_AISINO_JSFP:
            *fplxdm = FPLX_COMMON_JSFP;
            break;
        case FPLX_AISINO_DZFP:
            *fplxdm = FPLX_COMMON_DZFP;
            break;
        case FPLX_AISINO_HYFP:
            *fplxdm = FPLX_COMMON_HYFP;
            break;
        case FPLX_AISINO_DZZP:
            *fplxdm = FPLX_COMMON_DZZP;
            break;
        default:
            *fplxdm = 0xff;
            break;
    }
    return 0;
}

// Taxcard API GetHashTaxCode(); sz9BitTaxCodeΪhDev��9bit˰��
void GetHashTaxCode(char *szHashTaxCodeOut, char *sz9BitTaxCodeIn)
{
    strcpy(szHashTaxCodeOut, sz9BitTaxCodeIn);
    if (strlen(szHashTaxCodeOut) > 8)
        szHashTaxCodeOut[16] = '\0';
}

// Taxcard API Get9BitHashTaxCode()
int Get9BitHashTaxCode(HUSB hUSB, char *sz9BitTaxCode)
{
#ifdef _NODEVICE_TEST
    PTData pTD = (PTData)hUSB->pTestData;
    strcpy(sz9BitTaxCode, pTD->hDev->sz9ByteHashTaxID);
    return 0;
#endif
    uint8 ab9BitTaxCode[24];
    memset(ab9BitTaxCode, 0, 24);
    uint8 cb[520];
    int ncbLen = sizeof cb;
    memcpy(cb, "\xfe\xff\xcc\xbc\x53\xe1\xe3\x00\x08\x00", 10);
    int nRet = AisinoLogicIO(hUSB, cb, 10, ncbLen, NULL, 0);
    if (nRet != 9)
        return -1;
    memcpy(ab9BitTaxCode, cb, nRet - 1);
    memcpy(cb, "\xfe\xff\xcc\xbc\x53\xe1\xe3\x00\x11\x00", 10);
    nRet = AisinoLogicIO(hUSB, cb, 10, ncbLen, NULL, 0);
    if (nRet != 2)
        return -2;
    memcpy(ab9BitTaxCode + 8, cb, 1);
    Byte2Str(sz9BitTaxCode, ab9BitTaxCode, 9);
    return 0;
}

int GetCompressTaxCode(HUSB hUSB, char *szCompressTaxCode)
{
#ifdef _NODEVICE_TEST
    PTData pTD = (PTData)hUSB->pTestData;
    strcpy(szCompressTaxCode, pTD->hDev->szCompressTaxID);
    return 0;
#endif
    memset(szCompressTaxCode, 0, 10);
    uint8 cb[520] = {0};
    int ncbLen = sizeof cb;
    memcpy(cb, "\xfe\xff\xcc\xbc\x53\xe1\xe3\x00\x20\x00", 10);
    int nRet = AisinoLogicIO(hUSB, cb, 10, ncbLen, NULL, 0);
    if (nRet < 15)
        return -1;
    memcpy(szCompressTaxCode, cb, nRet - 1);
    return 0;
}

bool IsAisino3Device(int nVersionNum)
{
    return nVersionNum >= 200812;
}

void GetAisinoKPRJVersion(int nVersionNum, char *szOutStr)
{
    //��Ʊ����̶��ַ���,Aisino.Framework.Plugin.Core.Util, ���ߴӿ�Ʊ�����about������Կ���
    //Ŀǰ��ȫ��ת��3.0����Ҫ���ݲ���2.0��
    if (IsAisino3Device(nVersionNum))
        strcpy(szOutStr, defAisinoInnerVersion3);
    else
        strcpy(szOutStr, defAisinoInnerVersion2);
}

bool GetAisino3NodeValue(char *szOldTypeStringLine, char *szOut, bool bRemoveVauleFromLine)
{
    char *pAisino3SplitFlag = strstr(szOldTypeStringLine, defAisino3SplitFlag);
    if (!pAisino3SplitFlag)
        return false;
    strcpy(szOut, pAisino3SplitFlag + strlen(defAisino3SplitFlag));
    if (bRemoveVauleFromLine)
        memset(pAisino3SplitFlag, 0, sizeof(defAisino3SplitFlag));
    return true;
}