/*****************************************************************************
File name:   aisino_report_clear.c
Description: 用于爱信诺盘的抄报税和清卡
Author:      Zako
Version:     1.0
Date:        2020.02
History:
20200201     最初代码实现
*****************************************************************************/
#include "aisino_report_clear.h"

//返回值》0 锁死期；-1--10，错误；-100 需要抄税
int JudgeIsTimeForReport(HDEV hDev, uint8 bFplx_Aisino, int *pnLastRepIntervalDay)
{
    //这里本来应该还要检测是否报税期的，放宽检测条件，交由上层调用去处理
    int nChildRet = 0;
    int nRet = -3;
    uint8 cb[256] = "";
    uint8 abTransferData[24] = {0};
    char szTimeNow[24] = "";
    int nLockDay = 0;
    if (AisinoGetDeivceTime(hDev->hUSB, szTimeNow))
        return -2;
    Str2Byte(abTransferData, szTimeNow, strlen(szTimeNow));
    bswap_32(*(int *)abTransferData);
    int32 dwTime = 0;
    while (1) {
        //--0 fpxl, 1=vcode
        abTransferData[0] = abTransferData[1] = bFplx_Aisino;
        //上次抄税时间
        memcpy(cb, "\xfe\xff\xcc\xbc\x53\xe1\xe3\x00\x12\x00", 10);
        if ((nChildRet =
                 AisinoLogicIO(hDev->hUSB, (uint8 *)cb, 10, sizeof cb, abTransferData, 2)) != 7)
            break;
        dwTime = bswap_32(*(int *)(cb + 2));
        memset(cb, 0, sizeof(cb));
        Byte2Str((char *)cb, (uint8 *)&dwTime, 4);
        strcat((char *)cb, "000000");
        *pnLastRepIntervalDay = MinusTime(szTimeNow, (char *)cb) / 3600 / 24;
        if (*pnLastRepIntervalDay > 90) {  //僵尸盘
            _WriteLog(LL_FATAL, "Out of date, device has been locked");
            break;
        }
        //本月锁死时间
        memcpy(cb, "\xfe\xff\xcc\xbc\x53\xe1\xe3\x00\x0d\x00", 10);
        if ((nChildRet =
                 AisinoLogicIO(hDev->hUSB, (uint8 *)cb, 10, sizeof cb, abTransferData, 2)) != 5)
            break;
        dwTime = *(int *)cb;
        //--get lock day
        nLockDay = (uint8)dwTime;
        memset(cb, 0, 16);
        Byte2Str((char *)cb, (uint8 *)&nLockDay, 1);
        nLockDay = atoi((char *)cb);
        //下个报税日期
        memcpy(cb, "\xfe\xff\xcc\xbc\x53\xe1\xe3\x00\x13\x00", 10);
        if ((nChildRet =
                 AisinoLogicIO(hDev->hUSB, (uint8 *)cb, 10, sizeof cb, abTransferData, 2)) != 5)
            break;
        // finish
        nRet = 0;
        break;
    }
    if (!nRet)
        nRet = nLockDay;
    return nRet;
}

// byteOrigin buff's len must > 320
int GetFPHZB(uint8 *byteOrigin, uint8 bFplx_Aisino)
{
    int i = 0;
    int nRet = 0;
    uint8 array[320];
    for (i = 0; i < 320; i++) {
        if (i < 312) {
            array[i] = byteOrigin[i];
        } else {
            array[i] = 255;
        }
    }
    i = 0;
    if (FPLX_AISINO_DZFP == bFplx_Aisino) {
        array[312 + i] = FPLX_AISINO_DZFP;
        memcpy(byteOrigin, array, sizeof(array));
        nRet = 320;
    } else if (FPLX_AISINO_PTFP == bFplx_Aisino || FPLX_AISINO_ZYFP == bFplx_Aisino) {
        memcpy(byteOrigin, array, 312);
        nRet = 312;
    }
    return nRet;
}

int AES128CBC_ENCRYPT(unsigned char *sz_in_buff, int sz_in_len, unsigned char *sz_out_buff)
{
#define AES_INBUF_BLOCK_SIZE 128
    char szKey[] = "lxdyccb123456wjh";
    char szIV[] = "wjhyccb123456lxd";
    EVP_CIPHER_CTX *pctx = NULL;
    pctx = EVP_CIPHER_CTX_new();
    int isSuccess = 0;
    unsigned char in[AES_INBUF_BLOCK_SIZE];
    int outl = 0;
    int outl_total = 0;

    EVP_CIPHER_CTX_init(pctx);
    EVP_EncryptInit_ex(pctx, EVP_aes_128_cbc(), NULL, (uint8 *)szKey, (uint8 *)szIV);
    while (sz_in_len >= AES_INBUF_BLOCK_SIZE) {
        memcpy(in, sz_in_buff, AES_INBUF_BLOCK_SIZE);
        sz_in_len -= AES_INBUF_BLOCK_SIZE;
        sz_in_buff += AES_INBUF_BLOCK_SIZE;
        isSuccess =
            EVP_EncryptUpdate(pctx, sz_out_buff + outl_total, &outl, in, AES_INBUF_BLOCK_SIZE);
        if (!isSuccess) {
            EVP_CIPHER_CTX_cleanup(pctx);
            return 0;
        }
        outl_total += outl;
    }
    if (sz_in_len > 0) {
        memcpy(in, sz_in_buff, sz_in_len);
        isSuccess = EVP_EncryptUpdate(pctx, sz_out_buff + outl_total, &outl, in, sz_in_len);
        outl_total += outl;

        isSuccess = EVP_EncryptFinal_ex(pctx, sz_out_buff + outl_total, &outl);
        if (!isSuccess) {
            EVP_CIPHER_CTX_cleanup(pctx);
            return 0;
        }
        outl_total += outl;
    }
    EVP_CIPHER_CTX_cleanup(pctx);
    EVP_CIPHER_CTX_free(pctx);
    return outl_total;
}

// szHZSJHexStr_OutPutBuffer's size must be enough
int DZXML_AES_B64(HDEV hDev, uint8 bFplx_Report_Common, int nLockDay,
                  char *szHZSJHexStr_OutPutBuffer)
{
    // clang-format off
    // Key=lxdyccb123456wjh 6C786479636362313233343536776A68
    // IV=wjhyccb123456lxd 776A68796363623132333435366C7864
    // openssl enc -aes-128-cbc -in plain.txt -out encrypt.txt -K 6C786479636362313233343536776A68 -iv 776A68796363623132333435366C7864 -p -nosalt
    // clang-format on

    // pNewFormat example
    //<?xml version="1.0"
    // encoding="GBK"?><hzData><nsrsbh>92320105MA1WMUAP8T</nsrsbh><kpjh>0</kpjh><bssj>2020-02-01
    // 00:00:00</bssj><sbbh>661814461322</sbbh><hxsh>3201003440893140</hxsh><fplxdm>005</fplxdm><ssq>17</ssq><hzsj>1653...FF</hzsj></hzData>
    int nRet = -1;
    char szBuf[4096] = "";
    mxml_node_t *xml_Node = NULL;
    mxml_node_t *root = mxmlNewXML("1.0");
    if (!root)
        goto Finish;
    xml_Node = mxmlNewElement(root, "hzData");
    mxmlNewText(mxmlNewElement(xml_Node, "nsrsbh"), 0, hDev->szCommonTaxID);
    sprintf(szBuf, "%d", hDev->uICCardNo);
    mxmlNewText(mxmlNewElement(xml_Node, "kpjh"), 0, szBuf);
    GetBJSJ(hDev->szDeviceTime, szBuf, 1);
    mxmlNewText(mxmlNewElement(xml_Node, "bssj"), 0, szBuf);
    mxmlNewText(mxmlNewElement(xml_Node, "sbbh"), 0, hDev->szDeviceID);

    GetHashTaxCode(szBuf, hDev->sz9ByteHashTaxID);
    mxmlNewText(mxmlNewElement(xml_Node, "hxsh"), 0, szBuf);
    sprintf(szBuf, "%03d", bFplx_Report_Common);
    mxmlNewText(mxmlNewElement(xml_Node, "fplxdm"), 0, szBuf);
    sprintf(szBuf, "%d", nLockDay);
    mxmlNewText(mxmlNewElement(xml_Node, "ssq"), 0, szBuf);
    mxmlNewText(mxmlNewElement(xml_Node, "hzsj"), 0, szHZSJHexStr_OutPutBuffer);

    int nChildRet = TAXML_Formater(root, szHZSJHexStr_OutPutBuffer);
    nChildRet = AES128CBC_ENCRYPT((uint8 *)szHZSJHexStr_OutPutBuffer, nChildRet, (uint8 *)szBuf);
    if (!nChildRet)
        goto Finish;
    nRet = Base64_Encode(szBuf, nChildRet, szHZSJHexStr_OutPutBuffer);

Finish:
    mxmlDelete(root);
    return nRet;
}

//注意，这里是通用发票类型，而不是aisino的FPLX
// pReport_EncryptB64 == NULL 时为构造反写xml；否则构造远程抄报xml
int DZXML_ReportClear(HDEV hDev, uint8 bFplx_Report_Common, char *pReport_EncryptB64, char *szOut)
{
    // example szOut
    //<?xml version="1.0" encoding="utf-8"?><business id="HX_KPXXSC" comment="开票信息上传"><body
    // count="1" skph="661814461322" nsrsbh="92320105MA1WMUAP8T" kpjh="0" bssj="2020-02-01 00:00">
    //<group xh="1"> <data name="fplx_dm" value="005"/> <data name="fpmx"
    // value="mjv..uA=="/></group></body></business>

    //<?xml version="1.0" encoding="GBK"?><business id="HX_JKHCQQ" comment="监控回传请求"><body
    // count="1" skph="661814461322" nsrsbh="92320105MA1WMUAP8T" kpjh="0" bssj="2020-02-01
    // 00:00"><group xh="1"><data name="fplx_dm" value="005" /></group></body></business>
    int nRet = -1;
    char szBuf[256] = "";
    mxml_node_t *xml_Node = NULL, *xml_Node2 = NULL;
    mxml_node_t *root = mxmlNewXML("1.0");
    if (!root)
        goto Finish;
    xml_Node = mxmlNewElement(root, "business");
    if (pReport_EncryptB64) {
        mxmlElementSetAttr(xml_Node, "id", "HX_KPXXSC");
        mxmlElementSetAttr(xml_Node, "comment", "开票信息上传");
    } else {
        mxmlElementSetAttr(xml_Node, "id", "HX_JKHCQQ");
        mxmlElementSetAttr(xml_Node, "comment", "监控回传请求");
    }
    xml_Node = mxmlNewElement(xml_Node, "body");
    if (!xml_Node)
        goto Finish;
    mxmlElementSetAttr(xml_Node, "count", "1");
    mxmlElementSetAttr(xml_Node, "skph", hDev->szDeviceID);
    mxmlElementSetAttr(xml_Node, "nsrsbh", hDev->szCommonTaxID);
    sprintf(szBuf, "%d", hDev->uICCardNo);
    mxmlElementSetAttr(xml_Node, "kpjh", szBuf);
    // set fist day, taxreport time
    GetBJSJ(hDev->szDeviceTime, szBuf, 0);
    mxmlElementSetAttr(xml_Node, "bssj", szBuf);
    xml_Node = mxmlNewElement(xml_Node, "group");
    if (!xml_Node)
        goto Finish;
    mxmlElementSetAttr(xml_Node, "xh", "1");
    xml_Node2 = mxmlNewElement(xml_Node, "data");
    mxmlElementSetAttr(xml_Node2, "name", "fplx_dm");
    sprintf(szBuf, "%03d", bFplx_Report_Common);
    mxmlElementSetAttr(xml_Node2, "value", szBuf);
    if (pReport_EncryptB64) {
        xml_Node2 = mxmlNewElement(xml_Node, "data");
        mxmlElementSetAttr(xml_Node2, "name", "fpmx");
        mxmlElementSetAttr(xml_Node2, "value", pReport_EncryptB64);
    }
    nRet = TAXML_Formater(root, szOut);
    //_WriteHexToDebugFile("kpxxsc.xml", (uint8 *)szOut, strlen(szOut));

Finish:
    mxmlDelete(root);
    return nRet;
}

int ClearCardIO(HDEV hDev, char *szFpjkmw)
{
    int nChildRet = -1;
    uint8 cb[256] = "";
    if (strlen(szFpjkmw) < 64)
        return -1;
    uint8 abFpjkmw[512] = {0};
    if ((nChildRet = Base64_Decode(szFpjkmw, strlen(szFpjkmw), (char *)abFpjkmw)) < 0)
        return -2;
    abFpjkmw[nChildRet] = MakePackVerityFlag(abFpjkmw, nChildRet);
    nChildRet += 1;

    memcpy(cb, "\xfe\xff\xcc\xbc\x53\xe1\x42\x00\x00\x00", 10);
    if (AisinoLogicIO(hDev->hUSB, (uint8 *)cb, 10, sizeof cb, abFpjkmw, nChildRet) < 0)
        return -3;
    return 0;
}

// aisino转税局端上报汇总和清卡专用发票类型
void AisinoFPLX2PrivateFPLX(uint8 bFplx_Aisino, uint8 *bFplx_Report_Common,
                            uint8 *bFplx_Report_Aisino)
{
    uint8 bFplx_Report_AisinoTmp;
    switch (bFplx_Aisino) {
        case FPLX_AISINO_ZYFP:
        case FPLX_AISINO_PTFP:
            bFplx_Report_AisinoTmp = FPLX_AISINO_ZYFP;
            break;
        case FPLX_AISINO_DZFP:
        case FPLX_AISINO_ESC:
        case FPLX_AISINO_JDCFP:
            bFplx_Report_AisinoTmp = FPLX_AISINO_JDCFP;
            break;
        default:  // HY JSP
            bFplx_Report_AisinoTmp = bFplx_Aisino;
    }
    if (bFplx_Report_Aisino)
        *bFplx_Report_Aisino = bFplx_Report_AisinoTmp;

    AisinoFPLX2CommonFPLX(bFplx_Report_AisinoTmp, bFplx_Report_Common);
}

//征期抄税接口，注意区别的还有个非征期抄税接口
int PeriodTaxReport(HUSB hUSB, uint8 bFplx_Report_Aisino)
{
    int nChildRet = -1;
    uint8 cb[256] = "";
    uint8 abTransferData[16] = {0};
    if (FPLX_AISINO_ZYFP == bFplx_Report_Aisino)  // PTFP ZYFP
        memcpy(abTransferData, "\x00\x00\x00", 3);
    else if (FPLX_AISINO_JDCFP == bFplx_Report_Aisino)
        memcpy(abTransferData, "\x0c\x02\x0e", 3);
    else
        return -1;
    memcpy(cb, "\xfe\xff\xcc\xbc\x53\xe1\xe2\x00\x00\x00", 10);
    nChildRet = AisinoLogicIO(hUSB, (uint8 *)cb, 10, sizeof cb, abTransferData, 3);
    if (nChildRet < 0) {
        int nRepCode = GetDeviceRepCode(cb);
        if (202 == nRepCode) {
            SetLastError(hUSB, ERR_LOGIC_CHECK_REPORT_TIME, "抄税失败,可能存在多月未正常抄报情况");
            _WriteLog(LL_INFO, "It has period tax-report data exist!");
            return 202;
        } else {
            _WriteLog(LL_FATAL, "Some fatal error happened in PeriodTaxReport, errorcode:%d!",
                      nRepCode);
            return -2;
        }
    } else
        _WriteLog(LL_INFO, "Build new period tax-report data succeeeful!");
    return 0;
}

//远程抄报=上报监控
int ReporSummarytMain(HDEV hDev, uint8 bFplx_Aisino)
{
    HUSB hUsb = hDev->hUSB;
    int nChildRet = 0;
    uint8 cb[4096] = "";
    char szBuf[4096] = "";
    uint8 bFplx_Report_Common = 0;
    uint8 bFplx_Report_Aisino = 0;
    AisinoFPLX2PrivateFPLX(bFplx_Aisino, &bFplx_Report_Common, &bFplx_Report_Aisino);
    uint8 abTransferData[256] = {0};
    int nLastRepIntervalDay = 0;
    int nLockDay = JudgeIsTimeForReport(hDev, bFplx_Aisino, &nLastRepIntervalDay);
    if (nLockDay < 0) {
        SetLastError(hDev->hUSB, ERR_LOGIC_CHECK_REPORT_TIME,
                     "判断是否到抄报汇总时间失败,可能存在多月未正常抄报情况");
        _WriteLog(LL_INFO, "%s", hDev->hUSB->errinfo);
        return -1;
    }
    if (PeriodTaxReport(hUsb, bFplx_Report_Aisino) < 0)
        return -2;
    // get report raw data
    abTransferData[1] = 0x00;
    abTransferData[0] = abTransferData[2] = bFplx_Report_Aisino;  // 0 fpxl, 2=vcode
    memcpy(cb, "\xfe\xff\xcc\xbc\x53\xe1\xe3\x00\x34\x00", 10);
    nChildRet = AisinoLogicIO(hUsb, (uint8 *)cb, 10, sizeof cb, abTransferData, 3);
    if (nChildRet < 0) {
        _WriteLog(LL_FATAL, "[+] 抄税结果执行失败,可能已经完成抄税动作");
    }
    int nRepCode = GetDeviceRepCode(cb);
    if (nChildRet < 0 && 195 == nRepCode) {  //无报税资料
        if (nLastRepIntervalDay > 60)        // 0301-0201==29
        {
            _WriteLog(LL_FATAL, "Zombie device has no tax report data, unknow error!");
            return -3;
        } else {  //当月已汇总请卡，所以无报税资料，无需报税
            _WriteLog(LL_WARN, "No tax summary in the device, may be error operation or date!");
            return -4;
        }
    }
    if (nChildRet < 256) {  // normal > 312 bytes
        _WriteLog(LL_FATAL, "Some error return errorcode:%d!", nRepCode);
        return -5;
    }
    nChildRet = GetFPHZB(cb, bFplx_Aisino);
    //_WriteHexToDebugFile("fphzb-bin.raw", cb, nChildRet);

    Byte2Str(szBuf, cb, nChildRet);
    mystrupr(szBuf);
    uint8 bOPType = 0;
    StateInfo stateinfo;
    if (GetStateInfo(hDev->hUSB, &stateinfo) < 0)
        return -6;
    if (FPLX_COMMON_ZYFP == bFplx_Report_Common) {
        if (ZZSXML_ReportClear(hDev, true, stateinfo.IsLockReached != 0,
                               stateinfo.IsRepReached != 0, stateinfo.LockedDays, szBuf) < 0)
            return -6;
        strcpy((char *)cb, szBuf);
        bOPType = TACMD_AISINO_REPORT_ZZS;
    } else if (FPLX_COMMON_JDCFP == bFplx_Report_Common) {
        if (DZXML_AES_B64(hDev, bFplx_Report_Common, nLockDay, szBuf) < 0)
            return -7;
        if (DZXML_ReportClear(hDev, bFplx_Report_Common, szBuf, (char *)cb) < 0)
            return -8;
        bOPType = TACMD_AISINO_REPORT_DZ;
    } else
        return -9;
    if (TaNetIoOnce(hDev, bOPType, (char *)cb, sizeof(cb)) < 0)
        return -10;
    //_WriteLog(LL_INFO,
    //          "Remote report of the device successful! AisinoFPLX:%d bFplx_Report_Common:%d",
    //          bFplx_Aisino, bFplx_Report_Common);
    _WriteLog(LL_INFO, "上报汇总成功 AisinoFPLX:%d bFplx_Report_Common:%d\n", bFplx_Aisino,
              bFplx_Report_Common);
    return 0;
}

//清卡
int ClearCardMain(HDEV hDev, uint8 bFplx_Aisino)
{
    uint8 cb[40960] = "";
    char szBuf[40960] = "";
    uint8 bFplx_Report_Common = 0;
    AisinoFPLX2PrivateFPLX(bFplx_Aisino, &bFplx_Report_Common, NULL);
    // begin clear
    uint8 bOPType = 0;
    if (FPLX_COMMON_ZYFP == bFplx_Report_Common) {
        if (ZZSXML_ReportClear(hDev, false, false, false, 0, szBuf) < 0)
            return -4;
        bOPType = TACMD_AISINO_CLEAR_ZZS;
        strcpy((char *)cb, szBuf);
    } else {
        if (DZXML_ReportClear(hDev, bFplx_Report_Common, NULL, (char *)cb) < 0)
            return -4;
        bOPType = TACMD_AISINO_CLEAR_DZ;
    }
    if (TaNetIoOnce(hDev, bOPType, (char *)cb, sizeof(cb)) < 0)
        return -5;
    if (ClearCardIO(hDev, (char *)cb))
        return -6;
    _WriteLog(LL_INFO, "Remote clearcard of the device successful!");
    return 0;
}