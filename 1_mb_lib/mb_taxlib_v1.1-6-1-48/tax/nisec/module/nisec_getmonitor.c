/*****************************************************************************
File name:    nisec_getmonitor.c
Description:  用于获取nisec的监控信息和税盘信息获取
Author:       Zako
Version:      1.0
Date:         2019.05
History:
201905        代码初始化
*****************************************************************************/
#include "nisec_getmonitor.h"

//-----------------------------------------发票库存代码开始-----------------------------------------------
//获取invoice领购报文,szOutputHexStr返回长度可能有4k+，szOutputHexStr建议大小20K+
// 10卷发票长度1.5k,一般不会有问题除非150卷+发票
int QueryInvoSection(HDEV hDev, char *szOutputHexStr)
{
    bool bVersion = 0;
    int nChildRet = 0, nIndex = 0, nRet = -1, nCbLen = 64000, nUsbHexBufLen = 2 * nCbLen + 1024;
    uint8 *cb = calloc(1, nCbLen);
    char *szUsbHexStr = calloc(1, nUsbHexBufLen);
    if (!szUsbHexStr || !cb) {
        nRet = -2;
        goto Finish;
    }
    bVersion = GetDeviceBBH(hDev->szDriverVersion);
    //新版本盘，购票信息也的重新测试核对
    if (0 == bVersion) {
        memcpy(cb, "\xfe\x0c\x01\x00", 4);  //统一为4字节，实际为3
        if ((nChildRet = NisecLogicIO(hDev->hUSB, cb, 4, nCbLen, NULL, 0)) < 32) {
            nRet = -3;
            goto Finish;
        }

        Byte2Str(szUsbHexStr, cb, nChildRet);
        nIndex = 0;
        nIndex = NisecBaseStrExpand(hDev->abInvTemplate, &szUsbHexStr, &nUsbHexBufLen, nIndex);
        if (!nIndex) {
            nRet = -4;
            goto Finish;
        }
        nIndex += 0x10;
        while (nIndex < strlen(szUsbHexStr)) {
            nIndex += 0x30;
            nIndex = NisecBaseStrExpand(hDev->abInvTemplate, &szUsbHexStr, &nUsbHexBufLen, nIndex);
            nIndex += 8;
        }

    } else if (1 == bVersion) {
        memcpy(cb, "\xfe\x0c\x11\x01", 4);
        if ((nChildRet = NisecLogicIO(hDev->hUSB, cb, 4, nCbLen, NULL, 0)) < 32) {
            nRet = -5;
            goto Finish;
        }
        Byte2Str(szUsbHexStr, cb, nChildRet);
        nIndex = 0x10;
        while (nIndex < strlen(szUsbHexStr)) {
            nIndex = NisecBaseStrExpand(hDev->abInvTemplate, &szUsbHexStr, &nUsbHexBufLen, nIndex);
            nIndex += 0x30;
            nIndex = NisecBaseStrExpand(hDev->abInvTemplate, &szUsbHexStr, &nUsbHexBufLen, nIndex);
            nIndex += 0xb0;
        }
    } else {
        nRet = -6;
        goto Finish;
    }
    mystrupr(szUsbHexStr);
    strcpy(szOutputHexStr, szUsbHexStr);
    //_WriteHexToDebugFile("mycode-new.txt", szOutputHexStr, strlen(szOutputHexStr));
    nRet = strlen(szOutputHexStr);
Finish:
    if (szUsbHexStr)
        free(szUsbHexStr);
    if (cb)
        free(cb);
    return nRet;
}

int FillSIStructHead(char *pBuff, uint8 *abInvTemplate, struct StStockInfoHead *pSI)
{
    int nIndex = 0;
    uint8 bLen = NisecGetInvTemplateValue(abInvTemplate, 0, 1) * 2;
    strncpy(pSI->dqfpdm, pBuff + nIndex, bLen);
    nIndex += bLen;
    bLen = NisecGetInvTemplateValue(abInvTemplate, 1, 1) * 2;
    strncpy(pSI->dqfphm, pBuff + nIndex, bLen);
    nIndex += bLen;
    uint64 uTotal = NisecDecodeNum(pBuff + nIndex, 8);
    uint64 uCount = NisecDecodeNum(pBuff + nIndex + 8, 8);
    nIndex += 16;
    if (uTotal > 65536 || uCount > 65536)
        return -1;
    pSI->zsyfs = uTotal;
    pSI->nGroupCount = uCount;
    return nIndex;
}

int DecodeObject(char *pObjBuff, uint8 bVersion, uint8 *abInvTemplate, struct StStockInfoHead *pSIH,
                 struct StStockInfoGroup *pSI)
{
    uint8 bFPDMLen = 0, bFPHMLen = 0;
    int nIndex = 0;
    char szDqfpdm[64] = "", szDqfphm[64] = "", szBuf[64] = "";
    long lQshm = 0, lZzhm = 0, lDqhm = 0;

    bFPDMLen = NisecGetInvTemplateValue(abInvTemplate, 0, 1) * 2;
    bFPHMLen = NisecGetInvTemplateValue(abInvTemplate, 1, 1) * 2;
    if (1 == bVersion) {
        // dqfpdm
        strncpy(szDqfpdm, pObjBuff + nIndex, bFPDMLen);
        nIndex += bFPDMLen;
        // dqfphm
        strncpy(szDqfphm, pObjBuff + nIndex, bFPHMLen);
        nIndex += bFPHMLen;
    }
    // lgrq
    strncpy(pSI->lgrq, pObjBuff + nIndex, 8);
    nIndex += 8;
    // lgry
    Str2Byte((uint8 *)szBuf, pObjBuff + nIndex, 40);
    MemoryFormart(MF_TRANSLATE_HEADTAILZERO, szBuf, 20);
    G2U_GBKIgnore(szBuf, strlen(szBuf), pSI->lgry, sizeof(pSI->lgrq));
    nIndex += 40;
    // fqdm
    strncpy(pSI->fpdm, pObjBuff + nIndex, bFPDMLen);
    nIndex += bFPDMLen;
    // qshm
    strncpy(pSI->qshm, pObjBuff + nIndex, bFPHMLen);
    sscanf(pSI->qshm, "%010ld", &lQshm);
    nIndex += bFPHMLen;
    if (1 == bVersion) {
        // fpfs
        sscanf(pObjBuff + nIndex, "%08x", &pSI->fpfs);
        nIndex += 168;
        // syfs
        sscanf(pObjBuff + nIndex, "%08x", &pSI->syfs);
        nIndex += 8;
        //判断如果Head中当前发票代码号码为空，且当次解析剩余份数不为0，将当前填入总头
        if (!strlen(pSIH->dqfpdm) && pSI->syfs > 0) {
            strcpy(pSIH->dqfpdm, szDqfpdm);
            strcpy(pSIH->dqfphm, szDqfphm);
        }
    } else {
        // fpfs
        sscanf(pObjBuff + nIndex, "%04x", &pSI->fpfs);
        nIndex += 4;
        // syfs
        sscanf(pObjBuff + nIndex, "%04x", &pSI->syfs);
        nIndex += 4;
    }
    // zzhm
    lZzhm = lQshm + pSI->fpfs - 1;
    sprintf(pSI->zzhm, "%0*ld", bFPHMLen, lZzhm);
    // dqhm
    lDqhm = lQshm + pSI->fpfs - pSI->syfs;
    sprintf(pSI->dqhm, "%0*ld", bFPHMLen, lDqhm);
    //填入头部当前发票终止号码
    if (!strcmp(pSIH->dqfpdm, pSI->fpdm) && !strcmp(pSIH->dqfphm, pSI->dqhm))
        strcpy(pSIH->dqfpzzhm, pSI->zzhm);
    return nIndex;
}

int FillStockInfo(uint8 *pBuff, uint8 bVersion, uint8 *abInvTemplate, struct StStockInfoHead *pSIH)
{
    int nIndex = 0, i = 0;
    uint8 bLen;
    uint64 uTotal, uCount;
    if (bVersion) {
        uTotal = NisecDecodeNum((char *)pBuff + nIndex, 8);
        uCount = NisecDecodeNum((char *)pBuff + nIndex + 8, 8);
        nIndex += 16;
    } else {
        bLen = NisecGetInvTemplateValue(abInvTemplate, 0, 1) * 2;
        strncpy(pSIH->dqfpdm, (char *)pBuff + nIndex, bLen);
        nIndex += bLen;
        bLen = NisecGetInvTemplateValue(abInvTemplate, 1, 1) * 2;
        strncpy(pSIH->dqfphm, (char *)pBuff + nIndex, bLen);
        nIndex += bLen;
        uTotal = NisecDecodeNum((char *)pBuff + nIndex, 8);
        uCount = NisecDecodeNum((char *)pBuff + nIndex + 8, 8);
        nIndex += 16;
    }
    if (uTotal == 0 || uCount == 0 || uTotal > 5000000 || uCount > 65536) {
        _WriteLog(LL_WARN,
                  "Nisec GetStockInfo failed, may has no more invoice at this type,uCount = "
                  "%lld,uTotal = %lld",
                  uCount, uTotal);
        return -2;
    }
    pSIH->zsyfs = uTotal;
    pSIH->nGroupCount = uCount;
    uint8 *pArrayCount = (uint8 *)calloc(pSIH->nGroupCount, sizeof(struct StStockInfoGroup));
    if (pArrayCount == NULL)
        return -3;

    for (i = 0; i < pSIH->nGroupCount; i++) {
        uint8 *pObj = pBuff + nIndex;
        struct StStockInfoGroup *pSI =
            (struct StStockInfoGroup *)(pArrayCount + i * sizeof(struct StStockInfoGroup));
        nIndex += DecodeObject((char *)pObj, bVersion, abInvTemplate, pSIH, pSI);
        //处理子8位问题
        RemovePrefixTwoZero(pSI->fpdm);
    }
    pSIH->pArray_StStockInfoGroup = pArrayCount;
    //处理8位问题
    RemovePrefixTwoZero(pSIH->dqfpdm);
    return nIndex;
}

// GPXXCX
int GetStockInfo(HDEV hDev, struct StStockInfoHead *stSKInfo)
{
    int ret;
    memset(stSKInfo, 0, sizeof(struct StStockInfoHead));
    int nBlockLen = 65000, nRet = -1;
    uint8 *cb = (uint8 *)calloc(1, nBlockLen);
    if (!cb)
        return -1;
    while (1) {
        //新版本用旧版本接口也是可以的
        if ((ret = QueryInvoSection(hDev, (char *)cb)) < 0) {
            _WriteLog(LL_WARN, "QueryInvoSection FAILED ret =%d", ret);
            nRet = -2;
            break;
        }
        if (FillStockInfo(cb, GetDeviceBBH(hDev->szDriverVersion), hDev->abInvTemplate, stSKInfo) <
            0) {
            nRet = -3;
            break;
        }
        nRet = 0;
        break;
    }
    free(cb);
    return nRet;
}
//-----------------------------------------发票库存代码结束-----------------------------------------------
//-----------------------------------------监控信息代码开始-----------------------------------------------
int FillMIStruct1(uint8 *pBuff, int nBuffLen, struct StMonitorInfo *pSt)
{
    char szStrBuff[512] = "";
    Byte2Str(szStrBuff, pBuff, nBuffLen);
    if (strlen(szStrBuff) < 0x50)
        return -1;
    strncpy(pSt->kpjzsj, szStrBuff + 0x6, 8);
    strncpy(pSt->bsqsrq, szStrBuff + 0x34, 8);
    strncpy(pSt->bszzrq, szStrBuff + 0x3c, 8);

    int64 nNum = NisecDecodeNum(szStrBuff + 0xe, 0xc);
    Num2FloatS(nNum, pSt->dzkpxe);
    nNum = NisecDecodeNum(szStrBuff + 0x1a, 0xc);
    Num2FloatS(nNum, pSt->zsljxe);
    nNum = NisecDecodeNum(szStrBuff + 0x26, 0xc);
    Num2FloatS(nNum, pSt->fsljxe);
    return 0;
}

int FillMIStruct2(uint8 *pBuff, int nBuffLen, struct StMonitorInfo *pSt)
{
    char szStrBuff[512] = "";
    Byte2Str(szStrBuff, pBuff, nBuffLen);
    if (strlen(szStrBuff) < 0x50)
        return -1;
    //
    uint64 nNum = NisecDecodeNum(szStrBuff + 0x6, 2);
    sprintf(pSt->scjzrq, "%ld", (long)nNum);
    //
    strncpy(pSt->xdgnbs, szStrBuff + 0x8, 2);
    //
    nNum = NisecDecodeNum(szStrBuff + 0xa, 4);
    sprintf(pSt->lxkpsc, "%ld", (long)nNum);
    //
    nNum = NisecDecodeNum(szStrBuff + 0xe, 8);
    sprintf(pSt->lxkpzs, "%ld", (long)nNum);
    //
    nNum = NisecDecodeNum(szStrBuff + 0x16, 0xc);
    Num2FloatS(nNum, pSt->lxzsljje);
    //
    nNum = NisecDecodeNum(szStrBuff + 0x22, 0xc);
    Num2FloatS(nNum, pSt->lxfsljje);
    //
    strncpy(pSt->lxkzxx, szStrBuff + 0x2e, 0xa0);
    if (nBuffLen * 2 - 6 > 0xce) {
        // xxx
        if (nBuffLen * 2 - 6 > 0xd6) {
            //如果开票软件未开启，则数据长度不够，无该数据
            char szBuf1[64], szBuf2[64];
            //有可能专普票没有该数据，而电票有
            char *pPub = szStrBuff + 0xd6;
            while (true) {
                int nIndex = FormatTemplate(false, pPub, 0, szBuf1, 64);
                if (2 != strlen(szBuf1)) {
                    _WriteLog(LL_WARN, "Unknow pub ptlx");
                    break;
                }
                FormatTemplate(false, pPub, nIndex, szBuf2, 64);
                sprintf(pSt->ggfwpt, "%s%s", szBuf1, szBuf2);
                break;
            }
        }
    }
    return 0;
}

int GetMonitorInfo(HUSB hUSB, struct StMonitorInfo *stMIInfo)
{
    memset(stMIInfo, 0, sizeof(struct StMonitorInfo));
    uint8 cb[256];
    int nRet = -2;
    while (1) {
        memcpy(cb, "\xfe\x06\x01", 3);
        int nChildRet = NisecLogicIO(hUSB, cb, 3, sizeof(cb), NULL, 0);
        if (nChildRet < 24)
            break;
        if (FillMIStruct1(cb, nChildRet, stMIInfo) < 0) {
            break;
        }
        // ReadLastDeclareDate
        memcpy(cb, "\xfe\x2a", 2);
        nChildRet = NisecLogicIO(hUSB, cb, 2, sizeof(cb), NULL, 0);
        if (nChildRet != 4)
            break;
        Byte2Str(stMIInfo->zxbsrq, cb, nChildRet);
        // QuerySpace
        memcpy(cb, "\xfe\x1d", 2);
        nChildRet = NisecLogicIO(hUSB, cb, 2, sizeof(cb), NULL, 0);
        if (nChildRet != 4)
            break;
        char szTmpStr[16] = "";
        Byte2Str(szTmpStr, cb, nChildRet);
        uint64 nSyrl = NisecDecodeNum(szTmpStr, 0x8);
        sprintf(stMIInfo->syrl, "%llu", nSyrl);
        // read managedata --
        memcpy(cb, "\xfe\x06\x06", 3);
        nChildRet = NisecLogicIO(hUSB, cb, 3, sizeof(cb), NULL, 0);
        if (nChildRet < 64)
            break;
        if (FillMIStruct2(cb, nChildRet, stMIInfo) < 0) {
            break;
        }
        nRet = 0;
        break;
    }
    return nRet;
}
//-----------------------------------------监控信息代码结束-----------------------------------------------
//-----------------------------------------月度统计代码开始-----------------------------------------------
// nisec对StMonthStatistics的使用是无链表模式
int GetMonthStatistics(HUSB hUSB, char *szDateBegin, char *szDateEnd,
                       struct StMonthStatistics *pstStatistics)
{
    // API-TC_QueryCurSum
    uint8 cb[512];
    char szBuf[1024] = "";
    int nRet = 0, nIndex = 0;
    memcpy(cb, "\xfe\x18\x02", 3);
    Str2Byte(cb + 3, szDateBegin, 8);
    Str2Byte(cb + 7, szDateEnd, 8);
    nRet = NisecLogicIO(hUSB, cb, 11, sizeof cb, NULL, 0);
    if (nRet < 160 || nRet > 512)
        return -1;
    Byte2Str(szBuf, cb, nRet);
    nIndex += 4;
    // qssj
    strncpy(pstStatistics->qssj, szBuf + nIndex, 8);
    nIndex += 8;
    // jzsj
    strncpy(pstStatistics->jzsj, szBuf + nIndex, 8);
    nIndex += 8;
    // qckcfs
    NisecFormatStr2Num(0, szBuf + nIndex, 8, pstStatistics->qckcfs);
    // lgfpfs
    nIndex += 8;
    NisecFormatStr2Num(0, szBuf + nIndex, 8, pstStatistics->lgfpfs);
    // thfpfs
    nIndex += 8;
    NisecFormatStr2Num(0, szBuf + nIndex, 8, pstStatistics->thfpfs);
    // zsfpfs
    nIndex += 8;
    NisecFormatStr2Num(0, szBuf + nIndex, 8, pstStatistics->zsfpfs);
    // zffpfs
    nIndex += 8;
    NisecFormatStr2Num(0, szBuf + nIndex, 8, pstStatistics->zffpfs);
    // fsfpfs
    nIndex += 8;
    NisecFormatStr2Num(0, szBuf + nIndex, 8, pstStatistics->fsfpfs);
    // fffpfs
    nIndex += 8;
    NisecFormatStr2Num(0, szBuf + nIndex, 8, pstStatistics->fffpfs);
    // kffpfs
    nIndex += 8;
    NisecFormatStr2Num(0, szBuf + nIndex, 8, pstStatistics->kffpfs);
    // qmkcfs
    nIndex += 8;
    NisecFormatStr2Num(0, szBuf + nIndex, 8, pstStatistics->qmkcfs);
    // zsfpljje
    nIndex += 8;
    NisecFormatStr2Num(1, szBuf + nIndex, 0xc, pstStatistics->zsfpljje);
    // zsfpljse
    nIndex += 0x54;
    NisecFormatStr2Num(1, szBuf + nIndex, 0xc, pstStatistics->zsfpljse);
    // zffpljje
    nIndex += 0xc;
    NisecFormatStr2Num(1, szBuf + nIndex, 0xc, pstStatistics->zffpljje);
    // zffpljse
    nIndex += 0x54;
    NisecFormatStr2Num(1, szBuf + nIndex, 0xc, pstStatistics->zffpljse);
    // fsfpljje
    nIndex += 0xc;
    NisecFormatStr2Num(1, szBuf + nIndex, 0xc, pstStatistics->fsfpljje);
    // fsfpljse
    nIndex += 0x54;
    NisecFormatStr2Num(1, szBuf + nIndex, 0xc, pstStatistics->fsfpljse);
    // fffpljje
    nIndex += 0xc;
    NisecFormatStr2Num(1, szBuf + nIndex, 0xc, pstStatistics->fffpljje);
    // fffpljse
    nIndex += 0x54;
    NisecFormatStr2Num(1, szBuf + nIndex, 0xc, pstStatistics->fffpljse);

    // _WriteLog(LL_DEBUG,
    //           "qckcfs:%s lgfpfs:%s thfpfs:%s zsfpfs:%s zffpfs:%s fsfpfs:%s fffpfs:%s "
    //           "kffpfs:%s qmkcfs:%s zsfpljje:%s zsfpljse:%s zffpljje:%s zffpljse:%s "
    //           "fsfpljse:%s fffpljje:%s fffpljse:%s",
    //           pstStatistics->qckcfs, pstStatistics->lgfpfs, pstStatistics->thfpfs,
    //           pstStatistics->zsfpfs, pstStatistics->zffpfs, pstStatistics->fsfpfs,
    //           pstStatistics->fffpfs, pstStatistics->kffpfs, pstStatistics->qmkcfs,
    //           pstStatistics->zsfpljje, pstStatistics->zsfpljse, pstStatistics->zffpljje,
    //           pstStatistics->zffpljse, pstStatistics->fsfpljse, pstStatistics->fffpljje,
    //           pstStatistics->fffpljse);
    return 0;
}
//-----------------------------------------获取授权税率代码开始-----------------------------------------------
//返回格式为 hssl|bhssl 0.01，0.03|0.00，0.01
int NisecGetTaxrateStrings(HUSB hUsb, char *szOut, int nOutBufSize)
{
    int nRet = ERR_LOGIC_MONITOR_TAXRATE, index = 0;
    uint8 cb[2048] = "";
    char szBuf[16], szBuf2[16], szReserve[256], szHs[256] = "", szBhs[256] = "";
    bool bHs = false;
    do {
        memcpy(cb, "\xfe\x06\x04", 3);
        int nChildRet = NisecLogicIO(hUsb, cb, 3, sizeof(cb), NULL, 0);
        if (nChildRet < 128)
            break;
        index = 3;
        while (index < nChildRet) {
            // xh
            index += 1;
            // szsmdm
            index += 3;
            // slv
            memset(szBuf, 0, sizeof(szBuf));
            memset(szBuf2, 0, sizeof(szBuf2));
            Byte2Str(szBuf2, cb + index, 3);
            NisecFormatStr2Num(2, szBuf2, 6, szBuf);
            index += 3;
            // hsbz
            bHs = *(cb + index) != 0x00;
            index += 1;
            if (bHs) {
                strcat(szHs, szBuf);
                strcat(szHs, ",");
            } else {
                strcat(szBhs, szBuf);
                strcat(szBhs, ",");
            }
            // szmc 增值税
            G2U_GBKIgnore((char *)cb + index, 40, szReserve, sizeof(szReserve));
            MemoryFormart(MF_TRANSLATE_SPACE2ZERO, szReserve, 40);
            index += 40;
            // smmc 增值税%3
            G2U_GBKIgnore((char *)cb + index, 40, szReserve, sizeof(szReserve));
            MemoryFormart(MF_TRANSLATE_SPACE2ZERO, szReserve, 40);
            index += 40;
        }
        //拼接输出格式
        MemoryFormart(MF_REMOVE_TAILCOMMMA, szHs, strlen(szHs));
        MemoryFormart(MF_REMOVE_TAILCOMMMA, szBhs, strlen(szBhs));
        snprintf(szOut, nOutBufSize, "%s|%s", szHs, szBhs);
        nRet = RET_SUCCESS;
    } while (false);
    return nRet;
}

//-----------------------------------------更新监控信息代码开始-----------------------------------------------
int NisecUpdateMonitorData(HUSB hUSB, uint8 bFplx, char *szNewCompanyInfoBinStr)
{
    int nRet = ERR_GENERIC;
    if (NisecEntry(hUSB, NOP_OPEN_FULL))
        return ERR_DEVICE_OPEN;
    do {
        if (NisecSelectInvoiceTypeMini(hUSB, bFplx))
            break;
        if (NisecUpdateCompanyInfo(hUSB, szNewCompanyInfoBinStr))
            break;
        nRet = RET_SUCCESS;
    } while (false);
    NisecEntry(hUSB, NOP_CLOSE);
    return nRet;
}

int NisecUpdateMonitorFromTaSync(HUSB hUSB, mxml_node_t *pXMLRoot)
{
    //<fplx><group xh="1"><fplxdm>004</fplxdm><lxkzxx>...</lxkzxx></group>
    //<group xh="2"><fplxdm>026</fplxdm><lxkzxx>...</lxkzxx></group></fplx>
    int nRet = ERR_GENERIC;
    mxml_node_t *node = NULL;
    char szValue[2048] = "";
    // fplx肯定有，如果没有，nRet不改动则出错
    if ((node = mxmlFindElement(pXMLRoot, pXMLRoot, "fplx", NULL, NULL, MXML_DESCEND)) != NULL &&
        (node = mxmlGetFirstChild(node)) != NULL) {
        do {
            nRet = ERR_GENERIC;
            if (GetChildNodeValue(node, "fplxdm", szValue) < 0)
                break;
            int nFplx = atoi(szValue);
            if (GetChildNodeValue(node, "lxkzxx", szValue) < 0)
                break;
            if (NisecUpdateMonitorData(hUSB, nFplx, szValue))
                break;
            // next
            if ((node = mxmlGetNextSibling(node)) == NULL) {
                //最个一个节点
                nRet = RET_SUCCESS;
                break;
            }
            //多发票，继续循环
        } while (true);
    }
    return nRet;
}