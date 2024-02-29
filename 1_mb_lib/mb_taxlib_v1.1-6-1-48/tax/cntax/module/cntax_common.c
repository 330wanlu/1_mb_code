/*****************************************************************************
File name:   cntax_common.c
Description: 该文件主要存放自编写构造的函数代码和结构
             统一存放也便于其他模块调用，该文件中不得涉及具体业务逻辑函数
Author:      Zako
Version:     1.0
Date:        2021.02
History:
20210205     最初代码实现
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

//需要先开卡再切换，切换后等同于HDEV仅在当前发票类型下有效
//该函数将继续填充hdev，确保和发票类型相关的信息得到填充。注意！！！每次仅获取需要的新数据，未被填充的数据将保持之前读取的数据
//用于解决原函数获取过多无用数据，导致和蒙柏公司代码分歧碎片化，作出变更 20210930
// Cntax Ukey的selecttype和queryInvTemplate两个API已合并
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
        if (nChildRet > sizeof(hDev->abInvTemplate))  //解决机柜：068180027761 税盘：237008991941
            nChildRet = sizeof(hDev->abInvTemplate) - 1;
        memcpy((LPVOID)hDev->abInvTemplate, cb, nChildRet);
    }
    // 3--pubserv,部分监控数据填充到hdev
    if (option & SELECT_OPTION_MONITOR) {
        struct StMonitorInfo stMonitorInfo;
        if (CntaxGetMonitorInfo(hUSB, &stMonitorInfo) < 0)
            return ERR_LOGIC_MONITOR_FAIL;
        strcpy(hDev->szPubServer, stMonitorInfo.ggfwpt);
    }
    // 4--获取更新设备时间
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

// pDeviceOpenInfo可为空，缓冲大小至少16字节,最大64
int CntaxEntryRetInfo(HUSB hUSB, uint8 bOPType, uint8 *pDeviceOpenInfo)
{
#ifdef _NODEVICE_TEST
    return 0;
#endif
    if (NULL == hUSB)
        return -1;
    // 20210206版本之前默认两个密码，设备默认口里88888888，证书密码12345678。版本之后统一为一个密码12345678
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
        case NOP_OPEN_FULL: {  //发送\xfe\x01亦可，相比3位返回结果少三位，但是不能作为开卡调用
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

//获取盘三个基本要素 SN taxid name
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
//返回设备时间，标准格式20190101151515,szOut缓冲区至少16字节
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

//读取设备的 分散因子=TC_GetRTCRand（RTC时钟随机数）
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
    char szReserve[128] = "";  //保留字段
    int nIndex = 0;
    uint32 nNum = 0;
    Byte2Str(cbHex, cb, ncbLen);
    // SN
    memset(hDev->szDeviceID, 0, sizeof hDev->szDeviceID);
    strncpy(hDev->szDeviceID, (char *)cbHex + 6, 12);
    //登记时间 （qysj 启用时间) 14位，取8位
    memset(hDev->szDeviceEffectDate, 0, sizeof hDev->szDeviceEffectDate);
    strncpy(hDev->szDeviceEffectDate, cbHex + 18, 14);
    //企业类型
    memset(szReserve, 0, sizeof szReserve);
    strncpy(szReserve, cbHex + 32, 2);
    if (!strcmp(szReserve, "08"))
        hDev->bNatureOfTaxpayer = 1;  //小规模纳税人
    else if (!strcmp(szReserve, "05"))
        hDev->bNatureOfTaxpayer = 3;  //转登记纳税人
    else if (!strcmp(szReserve, "01"))
        hDev->bNatureOfTaxpayer = 2;  //增值税一般纳税人
    else if (!strcmp(szReserve, "02"))
        hDev->bNatureOfTaxpayer = 9;  //辅导期一般纳税人
    else
        hDev->bNatureOfTaxpayer = 0;  //"04"=特定征收部门/代开税务机关
    // cpyzlkz 成品油种类XX??
    memset(szReserve, 0, sizeof szReserve);
    strncpy(szReserve, cbHex + 34, 2);
    // kpjh size+num 2字节组成，有可能为空0000，0100=0，0102=2
    memset(szReserve, 0, sizeof szReserve);
    strncpy(szReserve, cbHex + 36, 2);
    sscanf(szReserve, "%x", (uint32 *)&nNum);
    hDev->uICCardNo = (uint16)nNum;
    // djxh 登记序号
    memset(szReserve, 0, sizeof szReserve);
    strncpy(szReserve, cbHex + 38, 20);
    // kpbz 开票标志
    hDev->stDevExtend.kpbz = GetUint8FromString(cbHex + 58);
    // swjgdmex 12位
    memset(hDev->szTaxAuthorityCodeEx, 0, sizeof hDev->szTaxAuthorityCodeEx);
    strncpy(hDev->szTaxAuthorityCodeEx, (char *)cb + 30, 12);
    // swjgdm 税务机关代码
    memset(hDev->szTaxAuthorityCode, 0, sizeof hDev->szTaxAuthorityCode);
    strcpy(hDev->szTaxAuthorityCode, hDev->szTaxAuthorityCodeEx);
    MemoryFormart(MF_TRANSLATE_SPACE2ZERO, hDev->szTaxAuthorityCode,
                  strlen(hDev->szTaxAuthorityCode));
    // regcode 区域代码
    memset(hDev->szRegCode, 0, sizeof hDev->szRegCode);
    strncpy(hDev->szRegCode, hDev->szTaxAuthorityCodeEx + 1, 6);
    // TaxID
    memset(hDev->szCommonTaxID, 0, sizeof hDev->szCommonTaxID);
    strncpy(hDev->szCommonTaxID, (char *)cb + 42, 20);
    MemoryFormart(MF_TRANSLATE_SPACE2ZERO, hDev->szCommonTaxID, strlen(hDev->szCommonTaxID));
    // swjgmc 税务机关名称
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
    // 特定企业
    extInfo->tdqy = GetUint8FromString(cbHex + 0);
    // 农产品企业
    extInfo->ncpqy = GetUint8FromString(cbHex + 2);
    // 电信企业
    extInfo->dxqy = GetUint8FromString(cbHex + 4);
    // 小规模开具专票标识
    extInfo->xgmkjzpbs = GetUint8FromString(cbHex + 6);
    // 二手机动车标识
    extInfo->esjdcbs = GetUint8FromString(cbHex + 8);
    // 成品油标识
    extInfo->cpybs = GetUint8FromString(cbHex + 10);
    // 成品油有效期
    strncpy(extInfo->cpybsyxq, (char *)cbHex + 12, 16);
    // 成品油白名单标识
    extInfo->cpybmdbs = GetUint8FromString(cbHex + 28);
    // 成品油白名单标识有效期
    strncpy(extInfo->cpybmdbsyxq, (char *)cbHex + 30, 16);
    // 稀土企业
    extInfo->xtqy = GetUint8FromString(cbHex + 46);
    // NULL 1
    GetUint8FromString(cbHex + 48);
    // 卷烟企业
    extInfo->jyqy = GetUint8FromString(cbHex + 50);
    // 卷烟企业有效期
    strncpy(extInfo->jyqyyxq, (char *)cbHex + 52, 16);
    // 机动车企业
    extInfo->jdcqy = GetUint8FromString(cbHex + 68);
    // 机动车企业有效期
    strncpy(extInfo->jdcqyyxq, (char *)cbHex + 70, 16);
    // 机动车白名单标识
    extInfo->jdcbmdbs = GetUint8FromString(cbHex + 86);
    // 机动车白名单有效期
    strncpy(extInfo->jdcbmdyxq, (char *)cbHex + 88, 16);
    // 委托代开企业
    extInfo->wtdkqy = GetUint8FromString(cbHex + 104);
    // 委托代开企业有效期
    strncpy(extInfo->wtdkqyyxq, (char *)cbHex + 106, 16);
    // 登记注册类型 index:122,4字节，只拷贝3字节(因为nisec和税局都是3字节)
    strncpy(extInfo->djzclx, (char *)cbHex + 123, 3);  //后3字节
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
        if (nChildRet < 38) {  //原来是64字节、偶遇一盘是56字节
            _WriteLog(LL_INFO, "2CntaxLogicIO nChildRet = %d last errinfo= %s", nChildRet,
                      hUSB->errinfo);
            break;  // 0200433f00000000......0020100
        }
        CntaxGetManageData2(hDev, cb, nChildRet);
        // version
        Byte2Str(hDev->szDriverVersion, abDeviceOpenInfo + 6, 7);
        // fplx support
        // UK_QueryInvoType接口 发送\xfe\x13 返回02 04 1C，可以获取支持的发票类型
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
    // 3张证书 一般可以取到3张
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
                    continue;  //没有证书标识返回
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


//返回值<=0为剩余错误次数，<-100逻辑错误，>0执行成功
int Cntax_Crypt_Password(HUSB hUSB, char *szDeviceID, char *szCertPassword, char *szNewCertPassword)
{
    //如果此前loaddev的时候crypt_deviceopen尚未执行，这里可能还需要重新执行一遍
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
                         "证书口令有误，可能原因为未修改默认证书口令12345678,剩余尝试次数%d", nRet);
            logout(INFO, "TAXLIB", "口令验证", "盘号：%s,错误信息：%s\r\n", szDeviceID,
                   hUSB->errinfo);
            // report_event(szDeviceID, "证书口令错误", hUSB->errinfo, nRet);
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

//需要确保已调用EncryptOpenDevice，打开加密区
//返回值<=0为剩余错误次数，<-100逻辑错误，>0执行成功
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

//改密码时，原始密码错误不减尝试次数
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

//返回值<=0为剩余错误次数，<-100逻辑错误，>0执行成功
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

//尚未使用测试，待修改
void TransferFPDMAndFPHMTo10Byte(char *szInFPDM, char *szInFPHM, uint8 *pOutBuf)
{
    // uint64 u64FPDMSwap = bswap_64(atoll(szInFPDM));
    // uint64 u64FPHMSwap = bswap_64(atoll(szInFPHM));
    // memcpy(pOutBuf, (uint8 *)&u64FPDMSwap + 2, 6);
    // memcpy(pOutBuf + 6, (uint8 *)&u64FPHMSwap + 4, 4);
}

// mxml生成的xml去掉头部
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

//为保持兼容性，只要任何涉及向税局通信的XML，所有的cntax mxml节点转string时都应该使用该方法进行转换
//返回值为空则错误，不为空时返回的指针需要手动free
char *CntaxGetUtf8YwxxStringFromXmlRoot(mxml_node_t *root)
{
    char *pRet = NULL;
    char *szXmlRootString = NULL;
    bool childRet = false;
    do {
        szXmlRootString = mxmlSaveAllocString(root, MXML_NO_CALLBACK);
        //需要前置截断，去掉头部
        if (RemoveFullXmlHead(szXmlRootString) < 0)
            break;
        int szoutlen = strlen(szXmlRootString) * 2 + 1024;
        pRet = calloc(1, szoutlen);
        //蒙柏的gbk模拟需要转换utf8兼容,linux下编译无需转换
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