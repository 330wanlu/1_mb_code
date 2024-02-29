/*****************************************************************************
File name:   cntax_getmonitor.c
Description: cntax的监控信息
Author:      Zako
Version:     1.0
Date:        2021.02
History:
20210205     最初代码实现
*****************************************************************************/
#include "cntax_getmonitor.h"

//未知命令
//\xfe\x11\x11 //读取数据报送时间的另一个接口？
//\xfe\x11\x12

// cmd  : FE111100000000000000000000000000
// recv : 1100170420210401120000302210251200002022090120220930
// cmd  : FE111200000000000000000000000000
// recv :
// 120034040000000098967F0000000000000000000000000000000000000000000000000001000000000000000002FAF080000000000000
//-----------------------------------------监控信息代码开始-----------------------------------------------
int CnTaxReadMonitorFromManageData(HUSB hUSB, struct StMonitorInfo *pSt)
{
    char szStrBuff[256];
    char szReserve[64];
    uint8 cb[256];
    int64 nNum = 0;
    int nChildRet = 0;

    memcpy(cb, "\xfe\x11\x11", 3);
    nChildRet = CntaxLogicIO(hUSB, cb, 3, sizeof(cb), NULL, 0);
    if (nChildRet < 26)
        return -1;
    memset(szStrBuff, 0, sizeof(szStrBuff));
    Byte2Str(szStrBuff, cb, nChildRet);
    //这里的时间都是14字节，兼容nisec，都只拷贝8字节
    // kpqysj 开票启用时间
    memset(szReserve, 0, sizeof(szReserve));
    strncpy(szReserve, szStrBuff + 8, 8);
    // kpjzsj
    strncpy(pSt->kpjzsj, szStrBuff + 22, 8);
    // bsqsrq
    strncpy(pSt->bsqsrq, szStrBuff + 36, 8);
    // bszzrq
    strncpy(pSt->bszzrq, szStrBuff + 44, 8);

    memcpy(cb, "\xfe\x11\x12", 3);
    nChildRet = CntaxLogicIO(hUSB, cb, 3, sizeof(cb), NULL, 0);
    if (nChildRet < 55)
        return -2;
    memset(szStrBuff, 0, sizeof(szStrBuff));
    Byte2Str(szStrBuff, cb, nChildRet);
    // dzkpxe
    nNum = NisecDecodeNum(szStrBuff + 10, 0xc);
    Num2FloatS(nNum, pSt->dzkpxe);
    // zsljxe
    nNum = NisecDecodeNum(szStrBuff + 22, 0xc);
    Num2FloatS(nNum, pSt->zsljxe);
    // fsljxe
    nNum = NisecDecodeNum(szStrBuff + 34, 0xc);
    Num2FloatS(nNum, pSt->fsljxe);
    // fpljzsxz 发票累计张数限制？
    nNum = NisecDecodeNum(szStrBuff + 46, 8);
    sprintf(szReserve, "%lld", nNum);
    // zsfpljzsxz 正数发票累计张数限制？
    nNum = NisecDecodeNum(szStrBuff + 54, 8);
    sprintf(szReserve, "%lld", nNum);
    // fsfpljzsxz 负数发票累计张数限制？
    nNum = NisecDecodeNum(szStrBuff + 62, 8);
    sprintf(szReserve, "%lld", nNum);
    // blank 2byte
    // scjzrq
    nNum = NisecDecodeNum(szStrBuff + 72, 2);
    sprintf(pSt->scjzrq, "%lld", nNum);
    // lxkpsc
    nNum = NisecDecodeNum(szStrBuff + 74, 4);
    sprintf(pSt->lxkpsc, "%lld", nNum);
    // lxkpzs
    nNum = NisecDecodeNum(szStrBuff + 78, 8);
    sprintf(pSt->lxkpzs, "%lld", nNum);
    // lxzsljje 10000.00
    nNum = NisecDecodeNum(szStrBuff + 86, 0xc);
    Num2FloatS(nNum, pSt->lxzsljje);
    // lxfsljje 0.00
    nNum = NisecDecodeNum(szStrBuff + 98, 0xc);
    Num2FloatS(nNum, pSt->lxfsljje);
    return 0;
}

int CntaxFillMIStruct2(HUSB hUSB, struct StMonitorInfo *pSt)
{
    char szStrBuf[256];
    char szReserve[64];
    char *p = NULL;
    uint8 cb[256];
    int nChildRet = 0;
    //-- API ReadLastDeclareDate
    // zxbsrq
    memcpy(cb, "\xfe\x18\x00", 3);
    if ((nChildRet = CntaxLogicIO(hUSB, cb, 3, sizeof(cb), NULL, 0)) < 4)
        return -1;
    Byte2Str(pSt->zxbsrq, cb, 4);
    // zxbssj 最新报税时间
    memcpy(cb, "\xfe\x18\x01", 3);
    if ((nChildRet = CntaxLogicIO(hUSB, cb, 3, sizeof(cb), NULL, 0)) < 7)
        return -2;
    Byte2Str(szReserve, cb, 7);
    // zxhssj 最新核算时间？
    memcpy(cb, "\xfe\x18\x02", 3);
    if ((nChildRet = CntaxLogicIO(hUSB, cb, 3, sizeof(cb), NULL, 0)) < 7)
        return -3;
    Byte2Str(szReserve, cb, 7);
    //获取监控平台信息，无论票种都读取,空==0x14 00 02 00 00
    // ueky电票默认都启用公共平台
    memcpy(cb, "\xfe\x11\x14", 3);
    if ((CntaxLogicIO(hUSB, cb, 3, sizeof(cb), NULL, 0) > 5) && *(uint16 *)(cb + 3)) {
        p = (char *)(cb + 3);
        uint8 bSize = *(uint8 *)p;
        memset(szStrBuf, 0, sizeof(szStrBuf));
        strncpy(szStrBuf, (char *)p + 1, bSize);
        p += 1 + bSize;
        bSize = *(uint8 *)p;
        memset(szReserve, 0, sizeof(szReserve));
        strncpy(szReserve, (char *)p + 1, bSize);
        sprintf(pSt->ggfwpt, "%s%s", szStrBuf, szReserve);  // 01100000000000
    }
    return RET_SUCCESS;
}

int CntaxGetMonitorInfo(HUSB hUSB, struct StMonitorInfo *stMIInfo)
{
    memset(stMIInfo, 0, sizeof(struct StMonitorInfo));
    int nRet = -2;
    while (1) {
        if (CnTaxReadMonitorFromManageData(hUSB, stMIInfo) < 0) {
            break;
        }
        if (CntaxFillMIStruct2(hUSB, stMIInfo) < 0) {
            break;
        }
        nRet = 0;
        break;
    }
    return nRet;
}
//-----------------------------------------监控信息代码结束-----------------------------------------------
//-----------------------------------------发票库存代码开始-----------------------------------------------
int CntaxQueryInvoSection(HDEV hDev, char *szOutputHexStr)
{
    int nChildRet = 0, nRet = -1, nCbLen = 8192;
    uint8 *cb = NULL;
    if (!(cb = calloc(1, nCbLen)))
        return -1;
    memcpy(cb, "\xfe\x1c\x02", 3);
    if ((nChildRet = CntaxLogicIO(hDev->hUSB, cb, 3, nCbLen, NULL, 0)) < 32)
        goto Finish;
    Byte2Str(szOutputHexStr, cb, nChildRet);
    nRet = strlen(szOutputHexStr);
Finish:
    if (cb)
        free(cb);
    return nRet;
}

int CntaxDecodeObject(char *pObjBuff, uint8 *abInvTemplate, struct StStockInfoHead *pSIH,
                      struct StStockInfoGroup *pSI)
{
    // totalsize = (fpdm-szie+fpgm-size)*2+48+56
    uint8 bFPDMLen = 0, bFPHMLen = 0;
    int nIndex = 0;
    char szDqfpdm[64] = "", szDqfphm[64] = "", szBuf[64] = "";
    long lQshm = 0, lZzhm = 0, lDqhm = 0;

    bFPDMLen = CntaxGetInvTemplateValue(abInvTemplate, 0, 1) * 2;
    bFPHMLen = CntaxGetInvTemplateValue(abInvTemplate, 1, 1) * 2;
    // dqfpdm
    strncpy(szDqfpdm, pObjBuff + nIndex, bFPDMLen);
    nIndex += bFPDMLen;
    // dqfphm
    strncpy(szDqfphm, pObjBuff + nIndex, bFPHMLen);
    nIndex += bFPHMLen;
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
    // fpfs
    sscanf(pObjBuff + nIndex, "%08x", &pSI->fpfs);
    nIndex += 8;
    // syfs
    sscanf(pObjBuff + nIndex, "%08x", &pSI->syfs);
    nIndex += 8;
    // zzhm
    lZzhm = lQshm + pSI->fpfs - 1;
    sprintf(pSI->zzhm, "%0*ld", bFPHMLen, lZzhm);
    // dqhm
    lDqhm = lQshm + pSI->fpfs - pSI->syfs;
    sprintf(pSI->dqhm, "%0*ld", bFPHMLen, lDqhm);
    //填入头部当前发票终止号码
    if (!strcmp(pSIH->dqfpdm, pSI->fpdm) && !strcmp(pSIH->dqfphm, pSI->dqhm))
        strcpy(pSIH->dqfpzzhm, pSI->zzhm);
    nIndex += 40;  // padding
    return nIndex;
}

int CntaxFillStockInfo(uint8 *pBuff, uint8 *abInvTemplate, struct StStockInfoHead *pSIH)
{
    int nIndex = 0, i = 0;
    uint8 bLen;
    uint64 uTotal, uCount;

    bLen = CntaxGetInvTemplateValue(abInvTemplate, 0, 1) * 2;
    if (bLen > 12)  //解决特殊税盘库存异常导致崩溃问题
        return ERR_BUF_CHECK;
    strncpy((char *)pSIH->dqfpdm, (char *)pBuff + nIndex, bLen);
    nIndex += bLen;
    bLen = CntaxGetInvTemplateValue(abInvTemplate, 1, 1) * 2;
    if (bLen > 8)  //解决特殊税盘库存异常导致崩溃问题
        return ERR_BUF_CHECK;
    strncpy((char *)pSIH->dqfphm, (char *)pBuff + nIndex, bLen);
    nIndex += bLen;
    uTotal = NisecDecodeNum((char *)pBuff + nIndex, 8);
    uCount = NisecDecodeNum((char *)pBuff + nIndex + 8, 8);
    nIndex += 16;
    if (uTotal == 0 || uCount == 0 || uTotal > 5000000 || uCount > 65536) {
        _WriteLog(LL_WARN,
                  "Cntax GetStockInfo failed, may has no more invoice at this type,uCount = "
                  "%ld,uTotal = %ld",
                  uCount, uTotal);
        return ERR_BUF_CHECK;
    }
    pSIH->zsyfs = uTotal;
    pSIH->nGroupCount = uCount;
    uint8 *pArrayCount = (uint8 *)calloc(pSIH->nGroupCount, sizeof(struct StStockInfoGroup));
    if (pArrayCount == NULL)
        return ERR_BUF_ALLOC;

    for (i = 0; i < pSIH->nGroupCount; i++) {
        uint8 *pObj = pBuff + nIndex;
        struct StStockInfoGroup *pSI =
            (struct StStockInfoGroup *)(pArrayCount + i * sizeof(struct StStockInfoGroup));
        nIndex += CntaxDecodeObject((char *)pObj, abInvTemplate, pSIH, pSI);
        //处理子8位问题
        RemovePrefixTwoZero(pSI->fpdm);
    }
    pSIH->pArray_StStockInfoGroup = pArrayCount;
    //处理8位问题
    RemovePrefixTwoZero(pSIH->dqfpdm);
    return nIndex;
}

int CntaxGetStockInfo(HDEV hDev, struct StStockInfoHead *stSKInfo)
{
    memset(stSKInfo, 0, sizeof(struct StStockInfoHead));
    int nBlockLen = 204800, nRet = -1;
    uint8 *cb = (uint8 *)calloc(1, nBlockLen);
    if (!cb)
        return -1;
    if (CntaxQueryInvoSection(hDev, (char *)cb) < 0) {
        nRet = -2;
        goto Finish;
    }
    if (CntaxFillStockInfo(cb, hDev->abInvTemplate, stSKInfo) < 0) {
        nRet = -3;
        goto Finish;
    }
    nRet = RET_SUCCESS;
Finish:
    free(cb);
    return nRet;
}
//-----------------------------------------发票库存代码结束-----------------------------------------------
//-----------------------------------------月度统计代码开始-----------------------------------------------
// pstStatistics next有指针则为链表形式，需要循环检测free
int CntaxGetMonthStatistics(HUSB hUSB, bool bPeriodAll, struct StMonthStatistics *pstStatistics)
{
    uint8 cb[4096] = {0};
    int nRet = 0, nIndex = 0;

    memcpy(cb, "\xfe\x17\x40", 3);
    if (bPeriodAll)
        cb[2] = '\x41';
    nRet = CntaxLogicIO(hUSB, cb, 3, sizeof cb, NULL, 0);
    if (nRet < 64)
        return -1;
    uint16 count = ntohs(*(uint16 *)cb);
    if (count > 16)
        return -2;
    int i = 0;
    struct StMonthStatistics *node = pstStatistics;
    char *szBuf = (char *)calloc(1, count * 200);
    if (!szBuf)
        return -3;
    Byte2Str(szBuf, cb + 2, nRet);
    for (i = 0; i < count; i++) {
        strncpy(node->qssj, szBuf + nIndex, 8);
        nIndex += 8;
        strncpy(node->jzsj, szBuf + nIndex, 8);
        nIndex += 8;
        // qckcfs
        NisecFormatStr2Num(0, szBuf + nIndex, 8, node->qckcfs);
        nIndex += 8;
        // lgfpfs
        NisecFormatStr2Num(0, szBuf + nIndex, 8, node->lgfpfs);
        nIndex += 8;
        // thfpfs
        NisecFormatStr2Num(0, szBuf + nIndex, 8, node->thfpfs);
        nIndex += 8;
        // zsfpfs
        NisecFormatStr2Num(0, szBuf + nIndex, 8, node->zsfpfs);
        nIndex += 8;
        // zffpfs
        NisecFormatStr2Num(0, szBuf + nIndex, 8, node->zffpfs);
        nIndex += 8;
        // fsfpfs
        NisecFormatStr2Num(0, szBuf + nIndex, 8, node->fsfpfs);
        nIndex += 8;
        // fffpfs
        NisecFormatStr2Num(0, szBuf + nIndex, 8, node->fffpfs);
        nIndex += 8;
        // kffpfs
        NisecFormatStr2Num(0, szBuf + nIndex, 8, node->kffpfs);
        nIndex += 8;
        // qmkcfs
        NisecFormatStr2Num(0, szBuf + nIndex, 8, node->qmkcfs);
        nIndex += 8;

        // zsfpljje
        NisecFormatStr2Num(1, szBuf + nIndex, 0xc, node->zsfpljje);
        nIndex += 12;
        // zsfpljse
        NisecFormatStr2Num(1, szBuf + nIndex, 0xc, node->zsfpljse);
        nIndex += 12;
        // zffpljje
        NisecFormatStr2Num(1, szBuf + nIndex, 0xc, node->zffpljje);
        nIndex += 12;
        // zffpljse
        NisecFormatStr2Num(1, szBuf + nIndex, 0xc, node->zffpljse);
        nIndex += 12;
        // fsfpljje
        NisecFormatStr2Num(1, szBuf + nIndex, 0xc, node->fsfpljje);
        nIndex += 12;
        // fsfpljse
        NisecFormatStr2Num(1, szBuf + nIndex, 0xc, node->fsfpljse);
        nIndex += 12;
        // fffpljje
        NisecFormatStr2Num(1, szBuf + nIndex, 0xc, node->fffpljje);
        nIndex += 12;
        // fffpljse
        NisecFormatStr2Num(1, szBuf + nIndex, 0xc, node->fffpljse);
        nIndex += 12;
        //以下打印方便查看数据用请勿删除
        /* _WriteLog(
             LL_DEBUG,
             "kssj:%s jzsj:%s qckcfs:%s lgfpfs:%s thfpfs:%s zsfpfs:%s zffpfs:%s fsfpfs:%s fffpfs:%s
           " "kffpfs:%s qmkcfs:%s zsfpljje:%s zsfpljse:%s zffpljje:%s zffpljse:%s " "fsfpljse:%s
           fffpljje:%s fffpljse:%s", node->qssj, node->jzsj, node->qckcfs, node->lgfpfs,
           node->thfpfs, node->zsfpfs, node->zffpfs, node->fsfpfs, node->fffpfs, node->kffpfs,
           node->qmkcfs, node->zsfpljje, node->zsfpljse, node->zffpljje, node->zffpljse,
           node->fsfpljse, node->fffpljje, node->fffpljse);*/
        // check add next node
        if (i < count - 1) {
            node->next = calloc(1, sizeof(struct StMonthStatistics));
            node = node->next;
        }
    }
    free(szBuf);
    return 0;
}
//-----------------------------------------获取授权税率代码开始-----------------------------------------------
//返回格式为 hssl|bhssl 0.01，0.03|0.00，0.01
int CntaxGetTaxrateStrings(HUSB hUsb, char *szOut, int nOutBufSize)
{
    int nRet = ERR_LOGIC_MONITOR_TAXRATE, index = 0;
    uint8 cb[2048] = "";
    char szBuf[16], szBuf2[16], szReserve[256], szHs[256] = "", szBhs[256] = "";
    bool bHs = false;
    do {
        memcpy(cb, "\xfe\x11\x13", 3);
        int nChildRet = CntaxLogicIO(hUsb, cb, 3, sizeof(cb), NULL, 0);
        if (nChildRet < 50)  // 2022-05-16修改遇一税盘此处只有51字节情况，667208857789
        {
            _WriteLog(LL_INFO, "nChildRet = %d", nChildRet);
            return 0;  // 2022-06-10修改 高灯一ukey专票此处报错 跳过后其他正常
            break;
        }
        index = 3;
        while (index < nChildRet) {
            // slv
            memset(szBuf, 0, sizeof(szBuf));
            memset(szBuf2, 0, sizeof(szBuf2));
            Byte2Str(szBuf2, cb + index, 3);
            NisecFormatStr2Num(2, szBuf2, 6, szBuf);
            if (!strcmp(szBuf, "0"))
                strcpy(szBuf, "0.00000");
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
            // slmc 增值税
            G2U_GBKIgnore((char *)cb + index, 20, szReserve, sizeof(szReserve));
            MemoryFormart(MF_TRANSLATE_SPACE2ZERO, szReserve, 20);
            index += 20;
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
int CntaxUpdateCompanyInfo(HUSB hUSB, char *szNewCompanyInfoBinStr)
{
    uint8 cb[512] = {0}, abCompanyInfoBuf[4096] = {0};
    if ((strlen(szNewCompanyInfoBinStr) / 2 + 1) > sizeof(abCompanyInfoBuf))
        return ERR_BUF_CHECK;
    int nCIBufLength =
        Str2Byte(abCompanyInfoBuf, szNewCompanyInfoBinStr, strlen(szNewCompanyInfoBinStr));
    memcpy(cb, "\xfe\x10", 2);
    if (CntaxLogicIO(hUSB, cb, 2, sizeof cb, abCompanyInfoBuf, nCIBufLength) < 0)
        return ERR_IO_FAIL;
    return RET_SUCCESS;
}

// UK_WriteManageData
int CntaxUpdateMonitorData(HUSB hUSB, uint8 bFplx, char *szNewCompanyInfoBinStr)
{
    int nRet = ERR_GENERIC;
    if (CntaxEntry(hUSB, NOP_OPEN_FULL))
        return ERR_DEVICE_OPEN;
    do {
        if (CntaxSelectInvoiceTypeMini(hUSB, bFplx))
            break;
        if (CntaxUpdateCompanyInfo(hUSB, szNewCompanyInfoBinStr))
            break;
        //_WriteLog(LL_DEBUG, "Write new manage fplx:%d string:%s", bFplx, szNewCompanyInfoBinStr);
        nRet = RET_SUCCESS;
    } while (false);
    CntaxEntry(hUSB, NOP_CLOSE);
    return nRet;
}

int CntaxUpdateMonitorFromTaSync(HUSB hUSB, uint8 bDefaultFplx, mxml_node_t *pXMLRoot)
{
    int nRet = ERR_GENERIC;
    mxml_node_t *node = NULL;
    char szValue[2048] = "", *pPayload = NULL;
    //--yhxx永远有。yhxx肯定有，如果没有，nRet不改动则出错
    if ((node = mxmlFindElement(pXMLRoot, pXMLRoot, "yhxx", NULL, NULL, MXML_DESCEND)) != NULL) {
        nRet = ERR_GENERIC;
        do {
            if (!(pPayload = (char *)mxmlGetOpaque(node)))
                break;
            //设备的公共管理信息，任何一种发票类型都可以写入配置
            if (CntaxUpdateMonitorData(hUSB, bDefaultFplx, pPayload))
                break;
            nRet = RET_SUCCESS;
        } while (false);
        if (nRet) {
            _WriteLog(LL_WARN, "Update cntax yhxx failed");
            return nRet;
        }
    }
    //--fplx只有在监控数据改动时才存在
    //<fplx><group xh="1"><fplxdm>004</fplxdm><lxkzxx>...</lxkzxx></group>
    //<group xh="2"><fplxdm>026</fplxdm><lxkzxx>...</lxkzxx></group></fplx>
    if ((node = mxmlFindElement(pXMLRoot, pXMLRoot, "fplx", NULL, NULL, MXML_DESCEND)) != NULL &&
        (node = mxmlGetFirstChild(node)) != NULL) {
        do {
            nRet = ERR_GENERIC;
            if (GetChildNodeValue(node, "fplxdm", szValue) < 0)
                break;
            int nFplx = atoi(szValue);
            if (GetChildNodeValue(node, "lxkzxx", szValue) < 0)
                break;
            if (CntaxUpdateMonitorData(hUSB, nFplx, szValue))
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