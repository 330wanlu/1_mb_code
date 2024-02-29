/*****************************************************************************
File name:   aisino_getinvoice.c
Description: 发票读取恢复函数
Author:      Zako
Version:     1.0
Date:        2019.10
History:
20200901     与rawio2fpxx.c文件功能相近，合并
20200925     修复专票红票老版本发票明细解析问题
20220222     开票软件220107版本对齐,结构优化调整
20220908     农产品收购发票密文解析修复
*****************************************************************************/
#include "aisino_getinvoice.h"

// szRet 一般在6-8k，建议缓冲区最少20k以上
int DecodeInvoiceBlock2(uint8 *pInv, int nInvBuffLen, char *szRet)
{
    uint8 *pSysMemBlock = NULL;
    uint8 *pIndexHaad = (uint8 *)pInv + 0x70;
    if (pIndexHaad[0] != 0x48 || pIndexHaad[1] != 0x65 || pIndexHaad[2] != 0x61 ||
        pIndexHaad[3] != 0x64)
        return -1;

    int nEnData1Len = *(uint16 *)(pIndexHaad + 0x26) + (*(uint8 *)(pIndexHaad + 0x25) << 16);
    uint8 *pEnData1 = pIndexHaad + 0x28;
    //预留空间30k,实际上远远用不了这么多内存，实际上内存解压是在szRet里面，为了和Test方法同步，就多分配点吧
    pSysMemBlock = calloc(1, nEnData1Len * 30 + 30000);
    if (!pSysMemBlock)
        return -2;
    int nLen =
        InvDecryptBlock2((uint8 *)pSysMemBlock, (long *)pEnData1, nEnData1Len, (long *)szRet);
    free(pSysMemBlock);
    nInvBuffLen = 0;  // no use
    return nLen;
}

//每一个block都需要带进来进行计算
int DecodeInvoiceBlock3(HDEV hDev, uint8 *pRawInv, int nRawInvBuffLen, uint8 *pBlock1,
                        uint8 *pBlock2, uint8 *pBlock3)
{
    pBlock3[129] = 0x01;
    int nBlockEncodeLen = *(uint16 *)(pRawInv + 0x96) + (*(uint8 *)(pRawInv + 0x95) << 16);
    if (nBlockEncodeLen < 0)
        return -1;
    int nBlockEncodeTailIndex = 0x98 + nBlockEncodeLen;
    int i = 0;
    for (i = 0; i < 801; i++) {
        if (0x53 == pRawInv[nBlockEncodeTailIndex + i] &&
            0x69 == pRawInv[nBlockEncodeTailIndex + i + 1] &&
            0x67 == pRawInv[nBlockEncodeTailIndex + i + 2] &&
            0x6e == pRawInv[nBlockEncodeTailIndex + i + 3])
            break;
    }
    if (i == 800)
        return -2;

    int nBlock3StartIndex = nBlockEncodeTailIndex + i - 0x13;
    uint16 wBlock3EncodeLen = *(uint16 *)(pRawInv + nBlock3StartIndex + 0x1c);
    uint16 nTotalSignLen = (wBlock3EncodeLen >> 2) * 4 + (wBlock3EncodeLen & 0x3);
    //计算密文校验码
    char szOutMW[256] = "";
    char szOutJYM[64] = "";
    int ret;
    if ((ret = GetMWJYMFromRawInvoice(hDev, (char *)pRawInv, (char *)pBlock1, (char *)pBlock2,
                                      szOutMW, szOutJYM)) < 0) {
        _WriteLog(LL_DEBUG, "GetMWJYMFromRawInvoice = %d", ret);
        return -3;
    }
    strcpy((char *)pBlock3, szOutMW);
    strcpy((char *)pBlock3 + strlen(szOutMW), szOutJYM);
    memcpy(pBlock3 + 128, &nTotalSignLen, 2);
    // raw 0x31 处还有个长度要填进去
    uint16 nOldTypeBlockBufLen = (uint16)(pBlock3 + 128 - pBlock1 - 0x41);
    memcpy(pBlock1 + 0x31, &nOldTypeBlockBufLen, 2);
    //签名处理
    memcpy(pBlock3 + 130, pRawInv + nBlock3StartIndex + 0x1e, nTotalSignLen);
    int nszRetIndex = 130 + nTotalSignLen;
    memcpy(pBlock3 + nszRetIndex, pRawInv + nBlock3StartIndex + 0x17, 4);
    nszRetIndex += 4;
    uint8 *pBlock3Start = pRawInv + nBlock3StartIndex;
    if (*(_BYTE *)pBlock3Start & 0xF)
        pBlock3[nszRetIndex] = 0x1;
    else
        pBlock3[nszRetIndex] = 0x0;
    nszRetIndex += 1;
    memcpy(pBlock3 + nszRetIndex, pRawInv + nBlock3StartIndex + 0xF, 4);
    nszRetIndex += 4;
    memcpy(pBlock3 + nszRetIndex, pRawInv + 0x78, 4);
    nszRetIndex += 4;
    nRawInvBuffLen = 0;  // no use
    return nszRetIndex;
}

int DecodeInvoiceBlock1(uint8 *pInv, int nInvBuffLen, char *szRet)
{
    //此处的pInvCopy，既充当输入，又充当输出缓冲，因此需要略大点
    int nInvCopyLen = nInvBuffLen * 30 + 30000;
    BYTE *pSysMemBlock = calloc(1, 8 * 1024);
    char *pInvCopy = (char *)calloc(1, nInvCopyLen);
    int nRet = -1;
    while (1) {
        if (!pInvCopy || !pSysMemBlock)
            break;
        memcpy(pInvCopy, pInv, nInvBuffLen);
        if ((nRet = DecodeInvoiceBlock2(pInv, nInvBuffLen, (char *)pInvCopy + 0x41)) < 0)
            break;
        if ((nRet = DecryptBlock1((long)pSysMemBlock, (long)szRet, (long)pInvCopy, nRet)) < 0)
            break;
        nRet = 0x41;
        break;
    }

    free(pSysMemBlock);
    free(pInvCopy);
    return nRet;
}

// in:0x20, 0x18,0x12
// out:
//当采用查询月份发票数量时，第一个包 bChildQuery = 0，后续的包bChildQuery =1，
// 返回5位；当采用查询月份发票详细时 bChildQuery = 2，返回4位
void GetQueryMonthFormat(uint8 nCentury, uint8 nYear, uint8 nMonth, uint8 bChildQuery,
                         char *szOutBuff)
{
    uint8 nCheckNum = 0;
    if (0 == bChildQuery) {
        nCheckNum = nCentury + nYear + nMonth;
        szOutBuff[0] = nMonth;
        szOutBuff[1] = nYear;
        szOutBuff[2] = nCentury;
        szOutBuff[3] = 0x00;
        *(uint8 *)(szOutBuff + 4) = nCheckNum;
    } else if (1 == bChildQuery) {
        nCheckNum = nCentury + nYear;
        szOutBuff[0] = 0x00;
        szOutBuff[1] = nYear;
        szOutBuff[2] = nCentury;
        szOutBuff[3] = 0x00;
        *(uint8 *)(szOutBuff + 4) = nCheckNum;
    } else if (2 == bChildQuery) {
        nCheckNum = nCentury + nYear + nMonth;
        szOutBuff[0] = nMonth;
        szOutBuff[1] = nYear;
        szOutBuff[2] = nCentury;
        *(uint8 *)(szOutBuff + 3) = nCheckNum;
    }
}

// input example: "201805","201611"
int GetInvMonthTotal(HUSB hUSB, char *szFormatYearMonth)
{
    uint32 nCentury, nYear, nMonth;
    int nRet = sscanf(szFormatYearMonth, "%2x%2x%2x", (unsigned int *)&nCentury,
                      (unsigned int *)&nYear, (unsigned int *)&nMonth);
    if (nRet != 3)
        return -1;
    uint8 cb[520];
    int ncbLen = sizeof cb;
    uint8 extraIn_retdataOut[16];
    memcpy(cb, "\xfe\xff\xcc\xbc\x53\xe1\xe3\x00\x1f\x00", 10);
    if (AisinoLogicIO(hUSB, cb, 10, ncbLen, NULL, 0) < 0)
        return -2;
    memcpy(cb, "\xfe\xff\xcc\xbc\x53\xe1\xe3\x01\x00\x00", 10);
    GetQueryMonthFormat(nCentury, nYear, nMonth, 2, (char *)extraIn_retdataOut);
    nRet = AisinoLogicIO(hUSB, cb, 10, ncbLen, extraIn_retdataOut, 4);
    if (nRet < 448)
        return -3;

    uint32 v14 = 0;
    int nCount = 0;
    uint8 v18[0x37];
    char *v13;
    int v20 = 8;  // 8个发票种类
    const void *v10 = (const void *)(cb + 8);
    do {
        --v20;
        memcpy(v18, v10, 0x37);
        if (v18[0] != 0xff) {  //发票种类判断不是合法值不累加
            long v11 = (long)v10 + 0x13;
            //当前发票种类下所有发票卷中已开具发票累加
            int v12 = 4;  //最多库存4个发票卷
            do {
                v13 = (char *)v11;
                v11 += 3;
                v14 = (*(uint32 *)v13) & 0x00ffffff;
                --v12;
                nCount += v14;
            } while (v12);
        }
        v10 = (char *)v10 + 0x37;
    } while (v20);
    return nCount;
}

//通常来说包含两种数据包，这两种数据包长度都不定。第一种数据包包含block1和block2,紧接着是第二种数据包包含发票密码加密区域和签名及两个block的校验
WORD DecodeContentLenFromBlock(uint8 bType, uint8 *pBuff)
{
    if (1 == bType) {
        WORD wBlock2EncodeLen = *(WORD *)(pBuff + 0x96);
        if (wBlock2EncodeLen <= 0 || wBlock2EncodeLen >= 1024)
            return -1;
        return wBlock2EncodeLen;
    } else if (2 == bType) {
        WORD wBlock3EncodeLen = *(WORD *)(pBuff + 0x1c);
        wBlock3EncodeLen = wBlock3EncodeLen >> 2;
        wBlock3EncodeLen = wBlock3EncodeLen * 4 + 2;
        wBlock3EncodeLen += 15;  // extra ???
        if (wBlock3EncodeLen <= 0 || wBlock3EncodeLen >= 1024)
            return -2;
        return wBlock3EncodeLen;
    } else {
        return -3;
    }
}

// 功能号:4 input example: "201805","201611"
int GetInvDetail(HDEV hDev, int nInvoiceNb, char *szFormatYearMonth, uint8 **ppInBuff,
                 int *pnBuffLen,
                 uint8 *aisino_fplx)  //增加发票类型传出，方便应用快速判断不识别的发票类型
{
    uint8 cb[1024] = {0};
    uint8 extraIn_retdataOut[16] = {0};
    int nMaxEncodeInvoiceLen = 25600;
    uint8 *pBuff = *ppInBuff;
    uint32 nCentury, nYear, nMonth;
    int nRet = sscanf(szFormatYearMonth, "%2x%2x%2x", (unsigned int *)&nCentury,
                      (unsigned int *)&nYear, (unsigned int *)&nMonth);
    if (nRet != 3)
        return -1;
    uint8 *pEncodeInvoiceBuff = (uint8 *)calloc(1, nMaxEncodeInvoiceLen);
    if (nInvoiceNb == 0)
        GetQueryMonthFormat(nCentury, nYear, nMonth, 0, (char *)extraIn_retdataOut);
    else
        GetQueryMonthFormat(nCentury, nYear, nMonth, 1, (char *)extraIn_retdataOut);
    int ncbLen = sizeof cb;
    int nInvoiceBuffIndex = 0;
    while (1) {
        if (IsAisino3Device(hDev->nDriverVersionNum))
            memcpy(cb, "\xfe\xff\xcc\xbc\x53\xe1\xe3\x01\x30\x00", 10);
        else
            memcpy(cb, "\xfe\xff\xcc\xbc\x53\xe1\xe3\x01\x1d\x00", 10);
        nRet = AisinoLogicIO(hDev->hUSB, cb, 10, ncbLen, extraIn_retdataOut, 5);
        if (nRet <= 0) {
            free(pEncodeInvoiceBuff);
            _WriteLog(LL_DEBUG, "盘号：%s,GetInvDetail AisinoLogicIO nRet = %d，lasterr = %s",
                      hDev->szDeviceID, nRet, hDev->hUSB->errinfo);
            return -2;
        }
        int nDataLen = nRet - 1;  //最后一个是校验字符？？需要去掉
        memcpy(pEncodeInvoiceBuff + nInvoiceBuffIndex, cb, nDataLen);
        nInvoiceBuffIndex += nDataLen;
        if ((cb[nDataLen - 4] == 0x54 && cb[nDataLen - 3] == 0x61 && cb[nDataLen - 2] == 0x69 &&
             cb[nDataLen - 1] == 0x6c) ||
            nInvoiceBuffIndex > 10240000)  //当读取到发票结尾标志或者设定发票最大值 10M
            break;
        if (nMaxEncodeInvoiceLen - nInvoiceBuffIndex < sizeof(cb)) {  // dynamic memory
            nMaxEncodeInvoiceLen = nMaxEncodeInvoiceLen + 10 * sizeof(cb);
            uint8 *pNewBuff = (uint8 *)realloc(pEncodeInvoiceBuff, nMaxEncodeInvoiceLen);
            if (pNewBuff == NULL) {
                break;
            }
            pEncodeInvoiceBuff = pNewBuff;
        }
    }
    if (nInvoiceBuffIndex * 15 + 30000 > *pnBuffLen) {
        // dynamic memory
        int nNewBuffLen = nInvoiceBuffIndex * 20 + 30000;  //正常情况下，扩展10+倍
        uint8 *pNewBuff = (uint8 *)realloc(*ppInBuff, nNewBuffLen);
        if (pNewBuff == NULL) {
            _WriteLog(LL_DEBUG, "realloc  failed nNewBuffLen = %d", nNewBuffLen);
            return -3;
        }
        *pnBuffLen = nNewBuffLen;
        *ppInBuff = pNewBuff;
        pBuff = pNewBuff;
    }
    *aisino_fplx = *(pEncodeInvoiceBuff + 1);
    if ((*aisino_fplx == FPLX_AISINO_HYFP) || (*aisino_fplx == FPLX_AISINO_JDCFP) ||
        (*aisino_fplx == FPLX_AISINO_JSFP) || (*aisino_fplx == FPLX_AISINO_ESC) ||
        (*aisino_fplx == FPLX_AISINO_DZZP)) {
        _WriteLog(LL_DEBUG, "*aisino_fplx = %d", *aisino_fplx);
        free(pEncodeInvoiceBuff);
        return -4;
    }
    _WriteHexToDebugFile("raw_enc-fp.bin", pEncodeInvoiceBuff, nInvoiceBuffIndex);
    nRet = DecodeInvBuf(hDev, pEncodeInvoiceBuff, nInvoiceBuffIndex, pBuff);
    if (nRet < 0) {
        _WriteLog(LL_DEBUG, "DecodeInvBuf nRet = %d", nRet);
    }
    free(pEncodeInvoiceBuff);
#ifdef _CONSOLE
    HFPXX fp = MallocFpxxLinkDev(hDev);
    if (AisinoInvRaw2Fpxx(pBuff, fp) >= 0) {
        _WriteLog(LL_DEBUG, "Restore invoice fplx:%d fpdm:%s fphm:%s dzsyh:%d", fp->fplx_aisino,
                  fp->fpdm, fp->fphm, fp->dzsyh);
        SaveFpxxToDisk(fp);
    } else {
        _WriteLog(LL_WARN, "||||||||||||||||||||AisinoInvRaw2Fpxx failed||||||||||||||||||||");
    }
    FreeFpxx(fp);
#endif
    return nRet;
}

int GetMWJYMFromRawInvoice(HDEV hDev, char *abRawInvoice, char *abDecInvoice, char *pBlock2,
                           char *szOutMW, char *szOutJYM)
{
    int nRet = -3;
    uint8 v47 = *((uint8 *)abRawInvoice + 0x1b);
    uint8 bCont = (v47 >> 5) & 1;
    char szBuyTaxcode[24] = "", szBuf[128] = "";
    strncpy(szBuyTaxcode, abDecInvoice + 18, 20);
    if (!strcmp(szBuyTaxcode, "000000123456789"))
        strcpy(szBuyTaxcode, "000000000000000");
    uint8 v5 = *(abRawInvoice + 1);
    if (v5 == FPLX_AISINO_HYFP || v5 == FPLX_AISINO_JDCFP)
        return ERR_PROGRAME_UNSUPPORT;
    uint8 bFPRawOffset_1b = abRawInvoice[0x1b];
    bool bIsMulitTaxRate = !((bFPRawOffset_1b & 0x40) == 0);
    bool bIsNcpSG = false;
    //根据开票软件逻辑，此处应该是发票类型2(普票)，41(卷票)，51(电票)三种发票
    //才判断是否农产品收购发票，目前暂时不能确定该逻辑，暂不加上发票类型的判断
    // a3为rawinv
    // if ( *(_BYTE *)(a3 + 1) == 2 || *(_BYTE *)(a3 + 1) == 51 || *(_BYTE *)(a3 + 1) == 41 ){}
    //
    //开票软件农产品发票标志是从block2发票明文数据从获取
    GetFpdmAndSpecialFlagFromBlock2(pBlock2, szBuf);
    if (strstr(szBuf, "V6"))
        bIsNcpSG = true;
    nRet =
        DecodeMWandJYM(bCont, bIsMulitTaxRate, bIsNcpSG, (uint8 *)abRawInvoice + 0x40,
                       hDev->szCompressTaxID, szBuyTaxcode, hDev->szCommonTaxID, szOutMW, szOutJYM);
    return nRet;
}

//发票修复过程中不能调用其他API，否则数据错乱，打断修复过程,因此为了几个静态变量，只能传Devinfo进来了
int DecodeInvBuf(HDEV hDev, uint8 *pInBuf, int nInBufLen, uint8 *pOutBuf)
{
    int nDecodeBlock1 = DecodeInvoiceBlock1(pInBuf, nInBufLen, (char *)pOutBuf);
    if (nDecodeBlock1 < 0)
        return -1;
    int nDecodeBlock2 =
        DecodeInvoiceBlock2(pInBuf, nInBufLen, (char *)((long)pOutBuf + nDecodeBlock1));
    if (nDecodeBlock2 < 0) {
        _WriteLog(LL_INFO, "DecodeInvBuf nDecodeBlock2 = %d", nDecodeBlock2);
        return -2;
    }
    int nDecodeBlock3 =
        DecodeInvoiceBlock3(hDev, pInBuf, nInBufLen, pOutBuf, pOutBuf + nDecodeBlock1,
                            pOutBuf + nDecodeBlock1 + nDecodeBlock2);
    if (nDecodeBlock3 < 0) {
        _WriteLog(LL_INFO, "DecodeInvBuf nDecodeBlock3 = %d", nDecodeBlock3);
        return -3;
    }
    int nTotalDecodeBlockLen = nDecodeBlock1 + nDecodeBlock2 + nDecodeBlock3;
    return nTotalDecodeBlockLen;
}

int QueryInvIo(HDEV hDev, uint8 *abQueryParam, HFPXX pOutFpxx)
{
    int nIndex = 0, nChildRet = 0, nRet = ERR_GENERIC, nMaxEncodeInvoiceLen = DEF_MAX_FPBUF_LEN;
    uint8 *pInvDecBuf = NULL;
    uint8 cb[768];
    int ncbLen = sizeof(cb);
    uint8 *pInvIOBuf = calloc(1, nMaxEncodeInvoiceLen);
    uint8 bContinue = 1;
    while (bContinue) {
        memcpy(cb, "\xfe\xff\xcc\xbc\x53\xe1\xe3\x01\x02\x00", 10);
        nChildRet = AisinoLogicIO(hDev->hUSB, cb, 10, ncbLen, abQueryParam, 14);
        if (nChildRet <= 0)
            break;
        //设置continue
        bContinue = cb[nChildRet + 1];
        int nDataLen = nChildRet - 1;  //最后一个是校验字符需要去掉
        memcpy(pInvIOBuf + nIndex, cb, nDataLen);
        nIndex += nDataLen;
        if ((cb[nDataLen - 4] == 0x54 && cb[nDataLen - 3] == 0x61 && cb[nDataLen - 2] == 0x69 &&
             cb[nDataLen - 1] == 0x6c) ||
            nIndex > 10240000)  //当读取到发票结尾标志或者设定发票最大值 10M
            break;
        if (nMaxEncodeInvoiceLen - nIndex < sizeof(cb)) {  // dynamic memory
            nMaxEncodeInvoiceLen = nMaxEncodeInvoiceLen + 10 * sizeof(cb);
            uint8 *pNewBuff = (uint8 *)realloc(pInvIOBuf, nMaxEncodeInvoiceLen);
            if (pNewBuff == NULL) {
                break;
            }
            pInvIOBuf = pNewBuff;
        }
    }
    if (nIndex <= 512) {
        nRet = -3;
        goto QueryInvInfo_Finish;
    }
    _WriteHexToDebugFile("raw_enc-fp.bin", pInvIOBuf, nIndex);
    int nOutBuffLen = nIndex * 30 + 30000;  //正常情况下，扩展10+倍
    pInvDecBuf = calloc(1, nOutBuffLen);
    if (!pInvDecBuf) {
        nRet = -4;
        goto QueryInvInfo_Finish;
    }
    int nDecLen = DecodeInvBuf(hDev, pInvIOBuf, nIndex, pInvDecBuf);
    if (nDecLen < 0) {
        nRet = -5;
        goto QueryInvInfo_Finish;
    }
    int result;
    //_WriteHexToDebugFile("raw_dec-fp.bin", pInvDecBuf, nDecLen);
    if ((result = AisinoInvRaw2Fpxx(pInvDecBuf, pOutFpxx)) < 0) {
        nRet = -6;
        _WriteLog(LL_DEBUG, "InvRaw2Fpxx,failed result = %d\n", result);
        goto QueryInvInfo_Finish;
    }
#ifdef _CONSOLE
    _WriteLog(LL_DEBUG, "FPQueryResult - fpdm:%s fphm:%s gfmc:%s gfdzdh:%s jshj:%s mw:%s",
              pOutFpxx->fpdm, pOutFpxx->fphm, pOutFpxx->gfmc, pOutFpxx->gfdzdh, pOutFpxx->jshj,
              pOutFpxx->mw);
#endif
    nRet = RET_SUCCESS;
QueryInvInfo_Finish:
    free(pInvIOBuf);
    if (pInvDecBuf)
        free(pInvDecBuf);
    return nRet;
}

//该函数输入参数两种方法
// 1）输入发票号码、代码 可以查找当前月份或者上月尚未抄报的发票信息，但是抄报了就不行
// 2）输全3个参数，可以查询任意存在发票,最大查询实际可能是半年？？？
// 3) 注意如果是012345678901 发票代码，必须带前面0进行查询，12345678901则错误
int QueryInvInfo(HDEV hDev, char *szTypecode, char *szNumer, uint32 nDZSYH, HFPXX pOutFpxx)
{
    uint8 cb[768];
    int nChildRet = 0;
    int ncbLen = sizeof(cb);
    memcpy(cb, "\xfe\xff\xcc\xbc\x53\xe1\xe3\x00\x1f\x00", 10);
    if ((nChildRet = AisinoLogicIO(hDev->hUSB, cb, 10, ncbLen, NULL, 0)) < 0)
        return ERR_IO_FAIL;
    uint8 abQueryParam[14];
    memset(abQueryParam, 0, sizeof(abQueryParam));
    InvTypeCode2Byte5(abQueryParam, szTypecode);
    *(uint32 *)(abQueryParam + 5) = atoi(szNumer);
    if (0 != nDZSYH) {
        int nDZSYH_R = bswap_32(nDZSYH);
        memcpy(abQueryParam + 9, &nDZSYH_R, 4);
    }
    abQueryParam[13] = MakePackVerityFlag(abQueryParam, 13);
    if ((nChildRet = QueryInvIo(hDev, abQueryParam, pOutFpxx)) < 0)
        return nChildRet;
    return RET_SUCCESS;
}

// szOutVersion>32字节,获取的是发票上aisino内部版本号，V3.2.00.220507
//!!!注意，存在多个月未开过票的盘，允许返回允许开票，但是开票软件版本无法输出
int AisinoGetTailInvVersion(HDEV hDev, char *szOutVersion)
{
    uint8 cb[768] = {0};
    int nChildRet = 0, ncbLen = 0;
    HFPXX fpxx = MallocFpxxLinkDev(hDev);
    int nRet = ERR_GENERIC;
    while (1) {
        ncbLen = sizeof(cb);
        //该IO指令一般aa结束，无意义，不发送
        // memcpy(cb, "\xfe\xff\xcc\xbc\x53\xe1\xe3\x00\x1f\x00", 10);
        memcpy(cb, "\xfe\xff\xcc\xbc\x53\xe1\xe3\x01\x20\x00", 10);
        if ((nChildRet = AisinoLogicIO(hDev->hUSB, cb, 10, ncbLen, NULL, 0)) < 0) {
            nRet = nChildRet;
            break;
        }
        //如果返回错误值为ff ff ff ff ff ff ff ff ff 00 00 00 00 f7
        if (cb[0] == 0xff && cb[1] == 0xff && cb[2] == 0xff && cb[3] == 0xff && cb[4] == 0xff &&
            cb[5] == 0xff) {
            // 3-4个月未开过票，无法通过API获取到发票信息，直接允许开票，但是无开票软件版本输出
            strcpy(szOutVersion, "V0.0.00.000000");
            nRet = RET_SUCCESS;
            break;
        }
        if (QueryInvIo(hDev, cb, fpxx) < 0)
            break;
        if (strlen(fpxx->kprjbbh) < 12 || strlen(fpxx->kprjbbh) > 24)
            break;
        strcpy(szOutVersion, fpxx->kprjbbh);
        nRet = RET_SUCCESS;
        break;
    }
    FreeFpxx(fpxx);
    return nRet;
}

// 0B03809B  11 00 00 04 03 19 12 F8  9B 03 D3 68 D0 12 13 7E
// 0B0380AB  3A 7B 00
int GetFirstOffLineInvIOBuf(HUSB husb, uint8 *abIOBuf, int nIOBufLen)
{
    if (nIOBufLen < 24)
        return -1;
    memset(abIOBuf, 0, nIOBufLen);
    uint8 cb[512];
    int ncbLen = sizeof(cb);
    memcpy(cb, "\xfe\xff\xcc\xbc\x53\xe1\xe3\x00\x49\x00", 10);
    int result = AisinoLogicIO(husb, cb, 10, ncbLen, NULL, 0);
    if (result < 5)
        return -2;
    *(uint16 *)abIOBuf = 0x11;
    uint8 *vP = abIOBuf + 2;
    memcpy(vP, cb, 4);
    memcpy(cb, "\xfe\xff\xcc\xbc\x53\xe1\xe3\x00\x4a\x00", 10);
    result = AisinoLogicIO(husb, cb, 10, ncbLen, NULL, 0);
    if (result < 16)
        return -3;
    uint8 *v65 = cb;
    uint32 v121 = *(_DWORD *)(v65 + 9);
    uint32 v122 = *(_DWORD *)(v65 + 13);
    uint8 v123 = *(_BYTE *)(v65 + 17);

    *(uint32 *)((char *)&v123 + 1) = *(uint32 *)v65;
    *(uint32 *)(vP + 4) = v121;
    *(uint32 *)(vP + 8) = v122;
    *(_BYTE *)(vP + 12) = v123;
    *(uint32 *)(vP + 13) = *(uint32 *)v65;
    return 0;
}

void OffLineInv_method_61(uint8 *byte_3, int int_8, int int_9, char *text)
{
    int i = 0;
    for (i = int_8; i < int_9 + int_8; i++) {
        char v[2] = "";
        v[0] = (char)((byte_3[i] >> 4) + 48);
        strcat(text, v);
        v[0] = (char)(byte_3[i] % 16 + 48);
        strcat(text, v);
    }
}

void OffLineInv_method_64(uint8 *byte_3, char *text)
{
    int i, j;
    uint8 array[16];
    memset(array, 0, sizeof(array));
    uint num = *(uint *)(byte_3 + 1);
    char szNum[32] = "";
    sprintf(szNum, "%d", num);
    int num2 = 10 - strlen(szNum);
    for (i = 0; i < 5; i++) {
        array[i] = ((i < num2) ? 48 : szNum[i - num2]);
    }
    num = (uint)(byte_3[0] & 63);
    array[5] = (uint8)(num / 10u + 48u);
    array[6] = (uint8)(num % 10u + 48u);
    for (j = 5; j < 10; j++) {
        array[j + 2] = ((j < num2) ? 48 : szNum[j - num2]);
    }
    strcpy(text, (char *)array);
}

int DecodeOfflineInvBuf(uint8 *abIOBuf, char *InvNo, char *TypeCode, char *OffTime, char *Index)
{
    int i = 0;
    int j = 0;
    int year, month, day, hour, minute, flag = 0;
    uint16 num = 0;
    uint8 array[5];
    uint32 nVar = 0;
    while (i < 4) {
        if (abIOBuf[i + 2] != 255) {
            flag = 1;
        DecodeOfflineInvBuf_1:
            num = *(uint16 *)abIOBuf;
            if (num < 4) {
                flag = 0;
                return flag;
            }
            if (num == 4 && flag) {
                year = (int)(abIOBuf[2] * 16) + (abIOBuf[3] >> 4);
                month = (int)(abIOBuf[3] & 15);
                day = abIOBuf[4] >> 3;
                hour = (int)((abIOBuf[4] & 7) * 4) + (abIOBuf[5] >> 6);
                minute = (int)(abIOBuf[5] & 63);
                sprintf(OffTime, "%02d-%02d-%02d %02d:%02d:%02d", year, month, day, hour, minute,
                        0);
                return flag;
            }
            if (flag) {
                if (abIOBuf[2] == 255 && abIOBuf[3] == 255 && abIOBuf[4] == 255) {
                    if (abIOBuf[5] == 255) {
                        goto DecodeOfflineInvBuf_2;
                    }
                }
                *(uint32 *)(abIOBuf + 2) = bswap_32(*(uint32 *)(abIOBuf + 2));
                nVar = *(uint32 *)(abIOBuf + 2);
                sprintf(Index, "%d", nVar);
            DecodeOfflineInvBuf_2:
                nVar = *(uint32 *)(abIOBuf + 6);
                sprintf(InvNo, "%08d", nVar);
                for (j = 0; j < 5; j++) {
                    array[j] = abIOBuf[j + 10];
                }
                if ((array[0] & 192) != 192) {
                    OffLineInv_method_61(array, 0, 5, TypeCode);
                } else {
                    OffLineInv_method_64(array, TypeCode);
                }
                year = (int)(abIOBuf[15] * 16) + (abIOBuf[16] >> 4);
                month = (int)(abIOBuf[16] & 15);
                day = abIOBuf[17] >> 3;
                hour = (int)((abIOBuf[17] & 7) * 4) + (abIOBuf[18] >> 6);
                minute = (int)(abIOBuf[18] & 63);
                sprintf(OffTime, "%02d-%02d-%02d %02d:%02d:%02d", year, month, day, hour, minute,
                        0);
                return flag;
            }
            return flag;
        } else {
            i++;
        }
    }
    goto DecodeOfflineInvBuf_1;
    return -1;
}

// > 0 has, = 0 no, < 0 failed
int GetFirstOfflineInv(HDEV hDev, char *szOutFPDM, char *szOutFPHM, int *dzsyh, char *kpsj)
{
    uint8 abIOBuf[32];
    memset(abIOBuf, 0, sizeof(abIOBuf));
    if (GetFirstOffLineInvIOBuf(hDev->hUSB, abIOBuf, sizeof(abIOBuf)) < 0)
        return -1;
    char s_kpsj[128] = {0}, s_dzsyh[128] = {0};
    int nRet = DecodeOfflineInvBuf(abIOBuf, szOutFPHM, szOutFPDM, s_kpsj, s_dzsyh);
    //_WriteLog(LL_INFO, "发票代码：%s,发票号码：%s,发票时间：%s,地址索引号 %s", szOutFPDM,
    // szOutFPHM, s_kpsj, s_dzsyh);
    strcpy(kpsj, s_kpsj);
    *dzsyh = atoi(s_dzsyh);
    return nRet;
}

int BuildWasteInvBuf(HFPXX fpxx, uint8 *pBuff)
{
    pBuff[0] = 0x0f;  // or ox0e
    // 1
    char abSignBinBuf[1024] = "";
    int nSignBinLen = Base64_Decode(fpxx->sign, strlen(fpxx->sign), abSignBinBuf);
    if (nSignBinLen < 0)
        return -1;
    int v7 = nSignBinLen + 34;
    int v8 = (v7 >> 9) + (v7 % 512 != 0);
    pBuff[1] = (uint8)v8;
    // 2
    int nFPHM = atoi(fpxx->fphm);
    memcpy(pBuff + 2, &nFPHM, 4);
    // 6
    char szOutTmp[128];
    int nChildRet = InvTypeCode2Byte5((uint8 *)szOutTmp, fpxx->fpdm);
    if (nChildRet != 5)
        return -2;
    memcpy(pBuff + 6, szOutTmp, nChildRet);
    // 0x13
    strcpy((char *)pBuff + 0x13, "Sign");
    // 0x17
    int nTmp = bswap_32(fpxx->dzsyh);
    memcpy(pBuff + 0x17, &nTmp, 4);
    // 0x1b;0x1b 长度高位，一般不用,0x1c==len
    uint16 uLen = nSignBinLen;
    memcpy(pBuff + 0x1c, &uLen, 2);
    // 0x1e
    memcpy(pBuff + 0x1e, abSignBinBuf, nSignBinLen);
    // tail
    int nTailIndex = (v8 & 0xff) << 9;
    strcpy((char *)pBuff + nTailIndex - 4, "Tail");

    uint8 abProcKey[32];
    memset(abProcKey, 0, 32);
    if (GetProcKey(fpxx->hDev->sz9ByteHashTaxID, abProcKey) < 0)
        return -3;
    Packet8ByteVCode(abProcKey, pBuff, pBuff + nTailIndex + 1);
    int nPackStaticEnd = nTailIndex + 1 + 8;
    pBuff[nPackStaticEnd] = MakePackVerityFlag(pBuff, nPackStaticEnd);
    nPackStaticEnd += 1;
    return nPackStaticEnd;
}

int WasteInvoice(HFPXX fpxx)
{
    //这里得需要实时时间，不能用静态时间
    AisinoGetDeivceTime(fpxx->hDev->hUSB, fpxx->hDev->szDeviceTime);
    if (TimeOutMonthDeadline(fpxx->kpsj_standard, fpxx->hDev->szDeviceTime)) {
        _WriteLog(LL_FATAL,
                  "Current device time is next month of the waste-invoice's month,DevTime:%s "
                  "InvTime:%s",
                  fpxx->hDev->szDeviceTime, fpxx->kpsj_standard);
        SetLastError(fpxx->hDev->hUSB, ERR_LOGIC_WASTETIME_OVERFLOW, "超过作废截止日期不允许作废");
        return -1;
    }
    int nRet = -1;
    int nChildRet = -1;
    while (1) {
        uint8 szOut[1024] = "";
        int nOutLen = BuildWasteInvBuf(fpxx, szOut);
        uint8 cb[64];
        int ncbLen = sizeof cb;
        if (IsAisino3Device(fpxx->hDev->nDriverVersionNum))
            memcpy(cb, "\xfe\xff\xcc\xbc\x53\xe1\xd4\x00\x02\x00", 10);
        else
            memcpy(cb, "\xfe\xff\xcc\xbc\x53\xe1\xe0\x00\x02\x00", 10);
        nChildRet = AisinoLogicIO(fpxx->hDev->hUSB, cb, 10, ncbLen, szOut, nOutLen);
        if (9 != nChildRet) {
            _WriteLog(LL_DEBUG, "WasteInvoice response failed, IOnRet:%d", nChildRet);
            nRet = -2;
            break;
        }
        nRet = 0;
        break;
    }
    return nRet;
}

void ClsTailBlank(char *buf, uint32 len)
{
    int i;
    for (i = len; i >= 0; i--) {
        if (buf[i - 1] == ' ')
            buf[i - 1] = '\0';
        else
            break;  //只移除末尾的空格，中间的空格不管
    }
}

void ClsSpace(char *buf, uint32 len)
{
    uint32 i, j;
    for (i = 0, j = 0; i < len; i++) {
        if (buf[i] != ' ') {
            buf[j++] = buf[i];
        }
    }
    memset(buf + j, 0, len - j);
}

void ClsCharacter(char *buf, uint32 len, char del_char)
{
    uint32 i, j;
    for (i = 0, j = 0; i < len; i++) {
        if (buf[i] == del_char)
            continue;
        buf[j++] = buf[i];
    }
    memset(buf + j, 0, len - j);
}

void FillSpxxByVersionStruct(uint8 bAisinoSpmxVersion, char *pGBKBuf, int i, struct Spxx *Head)
{
    //此处G2U_GBKIgnore使用sizeof整buf转换，不使用strlen防止极限情况内存粘连
    switch (bAisinoSpmxVersion) {
        case 2: {
            struct STSPXXv2 *pGBKBlock = (struct STSPXXv2 *)(pGBKBuf + i * sizeof(struct STSPXXv2));
            ClsTailBlank(pGBKBlock->SPMC, sizeof(pGBKBlock->SPMC));
            G2U_GBKIgnore(pGBKBlock->SPMC, sizeof(pGBKBlock->SPMC), Head->spmc, sizeof(Head->spmc));
            ClsTailBlank(pGBKBlock->GGXH, sizeof(pGBKBlock->GGXH));
            G2U_GBKIgnore(pGBKBlock->GGXH, sizeof(pGBKBlock->GGXH), Head->ggxh, sizeof(Head->ggxh));
            ClsTailBlank(pGBKBlock->JLDW, sizeof(pGBKBlock->JLDW));
            G2U_GBKIgnore(pGBKBlock->JLDW, sizeof(pGBKBlock->JLDW), Head->jldw, sizeof(Head->jldw));
            ClsTailBlank(pGBKBlock->SL, sizeof(pGBKBlock->SL));
            G2U_GBKIgnore(pGBKBlock->SL, sizeof(pGBKBlock->SL), Head->sl, sizeof(Head->sl));
            ClsTailBlank(pGBKBlock->DJ, sizeof(pGBKBlock->DJ));
            G2U_GBKIgnore(pGBKBlock->DJ, sizeof(pGBKBlock->DJ), Head->dj, sizeof(Head->dj));
            ClsTailBlank(pGBKBlock->JE, sizeof(pGBKBlock->JE));
            G2U_GBKIgnore(pGBKBlock->JE, sizeof(pGBKBlock->JE), Head->je, sizeof(Head->je));
            ClsTailBlank(pGBKBlock->SLV, sizeof(pGBKBlock->SLV));
            G2U_GBKIgnore(pGBKBlock->SLV, sizeof(pGBKBlock->SLV), Head->slv, sizeof(Head->slv));
            ClsTailBlank(pGBKBlock->SE, sizeof(pGBKBlock->SE));
            G2U_GBKIgnore(pGBKBlock->SE, sizeof(pGBKBlock->SE), Head->se, sizeof(Head->se));
            ClsTailBlank(pGBKBlock->NUM, sizeof(pGBKBlock->NUM));
            G2U_GBKIgnore(pGBKBlock->NUM, sizeof(pGBKBlock->NUM), Head->xh, sizeof(Head->xh));
            ClsTailBlank(pGBKBlock->FPHXZ, sizeof(pGBKBlock->FPHXZ));
            G2U_GBKIgnore(pGBKBlock->FPHXZ, sizeof(pGBKBlock->FPHXZ), Head->fphxz,
                          sizeof(Head->fphxz));
            ClsTailBlank(pGBKBlock->HSJBZ, sizeof(pGBKBlock->HSJBZ));
            G2U_GBKIgnore(pGBKBlock->HSJBZ, sizeof(pGBKBlock->HSJBZ), Head->hsjbz,
                          sizeof(Head->hsjbz));
            strcpy(Head->xsyh, "0");
            break;
        }
        case 3: {
            struct STSPXXv3 *pGBKBlock = (struct STSPXXv3 *)(pGBKBuf + i * sizeof(struct STSPXXv3));
            ClsTailBlank(pGBKBlock->SPMC, sizeof(pGBKBlock->SPMC));
            G2U_GBKIgnore(pGBKBlock->SPMC, sizeof(pGBKBlock->SPMC), Head->spmc, sizeof(Head->spmc));
            ClsTailBlank(pGBKBlock->GGXH, sizeof(pGBKBlock->GGXH));
            G2U_GBKIgnore(pGBKBlock->GGXH, sizeof(pGBKBlock->GGXH), Head->ggxh, sizeof(Head->ggxh));
            ClsTailBlank(pGBKBlock->JLDW, sizeof(pGBKBlock->JLDW));
            G2U_GBKIgnore(pGBKBlock->JLDW, sizeof(pGBKBlock->JLDW), Head->jldw, sizeof(Head->jldw));
            ClsTailBlank(pGBKBlock->SL, sizeof(pGBKBlock->SL));
            G2U_GBKIgnore(pGBKBlock->SL, sizeof(pGBKBlock->SL), Head->sl, sizeof(Head->sl));
            ClsTailBlank(pGBKBlock->DJ, sizeof(pGBKBlock->DJ));
            G2U_GBKIgnore(pGBKBlock->DJ, sizeof(pGBKBlock->DJ), Head->dj, sizeof(Head->dj));
            ClsTailBlank(pGBKBlock->JE, sizeof(pGBKBlock->JE));
            G2U_GBKIgnore(pGBKBlock->JE, sizeof(pGBKBlock->JE), Head->je, sizeof(Head->je));
            ClsTailBlank(pGBKBlock->SLV, sizeof(pGBKBlock->SLV));
            G2U_GBKIgnore(pGBKBlock->SLV, sizeof(pGBKBlock->SLV), Head->slv, sizeof(Head->slv));
            ClsTailBlank(pGBKBlock->SE, sizeof(pGBKBlock->SE));
            G2U_GBKIgnore(pGBKBlock->SE, sizeof(pGBKBlock->SE), Head->se, sizeof(Head->se));
            ClsTailBlank(pGBKBlock->NUM, sizeof(pGBKBlock->NUM));
            G2U_GBKIgnore(pGBKBlock->NUM, sizeof(pGBKBlock->NUM), Head->xh, sizeof(Head->xh));
            ClsTailBlank(pGBKBlock->FPHXZ, sizeof(pGBKBlock->FPHXZ));
            G2U_GBKIgnore(pGBKBlock->FPHXZ, sizeof(pGBKBlock->FPHXZ), Head->fphxz,
                          sizeof(Head->fphxz));
            ClsTailBlank(pGBKBlock->HSJBZ, sizeof(pGBKBlock->HSJBZ));
            G2U_GBKIgnore(pGBKBlock->HSJBZ, sizeof(pGBKBlock->HSJBZ), Head->hsjbz,
                          sizeof(Head->hsjbz));
            ClsTailBlank(pGBKBlock->FLBM, sizeof(pGBKBlock->FLBM));
            G2U_GBKIgnore(pGBKBlock->FLBM, sizeof(pGBKBlock->FLBM), Head->spbh, sizeof(Head->spbh));
            ClsTailBlank(pGBKBlock->SPBH, sizeof(pGBKBlock->SPBH));
            G2U_GBKIgnore(pGBKBlock->SPBH, sizeof(pGBKBlock->SPBH), Head->zxbh, sizeof(Head->zxbh));
            // SPBH blank, 商品编号跳过去，不处理
            ClsTailBlank(pGBKBlock->XSYH, sizeof(pGBKBlock->XSYH));
            G2U_GBKIgnore(pGBKBlock->XSYH, sizeof(pGBKBlock->XSYH), Head->xsyh, sizeof(Head->xsyh));
            ClsTailBlank(pGBKBlock->YHSM, sizeof(pGBKBlock->YHSM));
            G2U_GBKIgnore(pGBKBlock->YHSM, sizeof(pGBKBlock->YHSM), Head->yhsm, sizeof(Head->yhsm));
            ClsTailBlank(pGBKBlock->LSLVBS, sizeof(pGBKBlock->LSLVBS));
            G2U_GBKIgnore(pGBKBlock->LSLVBS, sizeof(pGBKBlock->LSLVBS), Head->lslvbs,
                          sizeof(Head->lslvbs));
            break;
        }
    }
}
//解析出每一条数据
// Aisino.Fwkp.BusinessObject.Fpxx.method_0(InvDetail, int) : string+600行左右；还有部分在method_3
int AnalyzeDetail(HFPXX stp_fpxx, char *data, uint8 bHeadNode)
{
    int hsjbz = -1;  // init -1
    char szSlvSumary[64] = "";
    bool same_hsjbz = true, same_slv = true;
    size_t nUTF8Len =
        strlen((char *)data) + 1024;  //加上1024字节防止V2版式红字专用发票读取时商品信息缺失

    if (!nUTF8Len)
        return -1;
    char *pGBKBuf = (char *)calloc(1, nUTF8Len);
    if (!pGBKBuf)
        return -2;
    if (U2G_GBKIgnore(data, strlen(data), pGBKBuf, nUTF8Len) <= 0) {
        free(pGBKBuf);
        return -3;
    }
    uint32 length = strlen((const char *)pGBKBuf);
    uint8 bAisinoSpmxVersion = 0;
    if (0 == length % 376)
        bAisinoSpmxVersion = 3;  //现行版本
    else if (0 == length % 252)
        bAisinoSpmxVersion = 2;  //部分地区专票红字发票仍在使用
    else if (0 == length % 250) {
        _WriteLog(LL_FATAL, "Aisino spmx find unsupport version, V2");
        bAisinoSpmxVersion = 1;
        free(pGBKBuf);  // unsupport
        return -4;
    } else {
        _WriteLog(LL_FATAL, "Aisino spmx find unsupport version, unknow,length = %d", length);
        free(pGBKBuf);
        return -4;
    }
    //只处理v2和v3
    int nSpxxCount = 0;
    if (3 == bAisinoSpmxVersion)
        nSpxxCount = length / sizeof(struct STSPXXv3);
    else  // v2
        nSpxxCount = length / sizeof(struct STSPXXv2);
    int i = 0;
    stp_fpxx->spsl = nSpxxCount;
    for (i = 0; i < nSpxxCount; i++) {
        struct Spxx *Head = (struct Spxx *)calloc(1, sizeof(struct Spxx));
        FillSpxxByVersionStruct(bAisinoSpmxVersion, pGBKBuf, i, Head);
        //部分节点修正
        //--spmc
        char spsmmc_spmc[200] = "";
        strcpy(spsmmc_spmc, Head->spmc);
        memset(Head->spmc, 0, sizeof(Head->spmc));  // zero buf
        GetSpsmmcSpmc(spsmmc_spmc, Head->spsmmc, Head->spmc);
        //--fphxz
        switch (atoi(Head->fphxz)) {
            case FPHXZ_AISINO_SPXX:
                sprintf(Head->fphxz, "0");
                break;
            case FPHXZ_AISINO_ZKXX:
                sprintf(Head->fphxz, "1");
                break;
            case FPHXZ_AISINO_SPXX_ZK:
                sprintf(Head->fphxz, "2");
                break;
            case FPHXZ_AISINO_XJXHQD:
                sprintf(Head->fphxz, "3");
                break;
            default:
                sprintf(Head->fphxz, "0");
                break;
        }
        //首节点判断
        if (bHeadNode) {
            memcpy(stp_fpxx->stp_MxxxHead, Head, sizeof(struct Spxx));
            free(Head);
            stp_fpxx->stp_MxxxHead->stp_next = NULL;
        } else {
            //是否含税价格
            int hsj_tmp = atoi(Head->hsjbz);
            if (hsjbz == -1) {
                hsjbz = hsj_tmp;  //第一次初始化赋值
            } else {
                if (hsjbz != hsj_tmp)
                    same_hsjbz = false;
            }
            //是否多税率
            if (!strlen(szSlvSumary))
                strcpy(szSlvSumary, Head->slv);
            else {
                if (strcmp(szSlvSumary, Head->slv))
                    same_slv = false;
            }
            InsertMxxx(stp_fpxx, Head);
        }
    }
    if (!bHeadNode) {
        //是否含税价格
        if (same_hsjbz)  //含税和不含税全一致
            sprintf(stp_fpxx->hsjbz, "%d", hsjbz);
        else  //不一致,多混合
            sprintf(stp_fpxx->hsjbz, "%d", 2);
        //是否多税率
        stp_fpxx->isMultiTax = (uint8)!same_slv;
    }

    free(pGBKBuf);
    return 0;
}

int GetArrayLine1(HFPXX stp_fpxx, char *arrays)
{
    int index = 0;
    //发票代码
    int count;
    count = 0;
    while (arrays[index] != 'B' && arrays[index] != 'V' && (index < strlen((const char *)arrays)) &&
           (arrays[index] >= '0') && (arrays[index] <= '9')) {
        stp_fpxx->fpdm[count] = arrays[index];
        count++;
        index++;
    }
    char ca_ZyfpFpdm[10] = {0};
    ca_ZyfpFpdm[0] = arrays[index];
    index++;
    ca_ZyfpFpdm[1] = arrays[index];
    index++;
    if (strcmp(ca_ZyfpFpdm, "V1") == 0) {
        stp_fpxx->zyfpLx = ZYFP_NULL;
    } else if ((strcmp(ca_ZyfpFpdm, "V2") == 0) && (stp_fpxx->fplx_aisino == FPLX_AISINO_ZYFP)) {
        stp_fpxx->zyfpLx = ZYFP_XT_YCL;
    } else if ((strcmp(ca_ZyfpFpdm, "V3") == 0) && (stp_fpxx->fplx_aisino == FPLX_AISINO_ZYFP)) {
        stp_fpxx->zyfpLx = ZYFP_XT_CCP;
    } else if ((strcmp(ca_ZyfpFpdm, "V4") == 0) && (stp_fpxx->fplx_aisino == FPLX_AISINO_ZYFP)) {
        stp_fpxx->zyfpLx = ZYFP_SNY;  //可以为ZYFP_SNY或ZYFP_SNY_DDZG或ZYFP_RLY或ZYFP_RLY_DDZG
    } else if ((strcmp(ca_ZyfpFpdm, "V5") == 0)) {
        stp_fpxx->zyfpLx = ZYFP_NCP_XS;
    } else if ((strcmp(ca_ZyfpFpdm, "V6") == 0)) {
        stp_fpxx->zyfpLx = ZYFP_NCP_SG;
    } else if ((strcmp(ca_ZyfpFpdm, "VA") == 0) && (stp_fpxx->fplx_aisino == FPLX_AISINO_ZYFP) &&
               (stp_fpxx->isRed == 1)) {
        stp_fpxx->zyfpLx = ZYFP_CPY;
    } else if ((strcmp(ca_ZyfpFpdm, "V9") == 0)) {
        stp_fpxx->zyfpLx = ZYFP_CPY;
    } else if ((strcmp(ca_ZyfpFpdm, "VE") == 0) && (stp_fpxx->fplx_aisino == FPLX_AISINO_ZYFP)) {
        stp_fpxx->zyfpLx = ZYFP_XT_JSJHJ;
    } else if ((strcmp(ca_ZyfpFpdm, "VF") == 0) && (stp_fpxx->fplx_aisino == FPLX_AISINO_ZYFP)) {
        stp_fpxx->zyfpLx = ZYFP_XT_CPJGF;
    } else {
        return -1;
    }
    //专票红票无编码表版本，后续检测无意义
    if (arrays[index] != 'B')
        return 0;

    index++;
    count = 0;
    while (arrays[index] != 'H' && arrays[index] != 'J' && arrays[index] != 'C' &&
           arrays[index] != 'Y' && arrays[index] != '\n' &&
           (index < strlen((const char *)arrays))) {
        stp_fpxx->bmbbbh[count] = arrays[index];
        index++;
        count++;
    }
    memset(ca_ZyfpFpdm, 0, sizeof(ca_ZyfpFpdm));
    count = 0;
    while (arrays[index] != '\n' && (index < strlen((const char *)arrays))) {
        ca_ZyfpFpdm[count] = arrays[index];
        count++;
        index++;
    }
    strcpy(stp_fpxx->yysbz, "00000000000000000000");
    if (strchr(ca_ZyfpFpdm, 'J') &&
        (stp_fpxx->fplx_aisino == FPLX_AISINO_ZYFP || stp_fpxx->fplx_aisino == FPLX_AISINO_PTFP)) {
        stp_fpxx->zyfpLx = ZYFP_JYZS;
    } else if (strchr(ca_ZyfpFpdm, 'C') && (stp_fpxx->fplx_aisino == FPLX_AISINO_ZYFP ||
                                            stp_fpxx->fplx_aisino == FPLX_AISINO_PTFP ||
                                            stp_fpxx->fplx_aisino == FPLX_AISINO_DZFP)) {
        stp_fpxx->zyfpLx = ZYFP_CEZS;
        stp_fpxx->yysbz[8] = '2';
    } else if (strchr(ca_ZyfpFpdm, 'Y') && strchr(ca_ZyfpFpdm, 'D')) {
        strcat((char *)stp_fpxx->yddk, "1");
    } else if (strchr(ca_ZyfpFpdm, 'Y') && strchr(ca_ZyfpFpdm, 'S')) {
        strcat((char *)stp_fpxx->dkbdbs, "1");
    } else if (strchr(ca_ZyfpFpdm, 'H') && stp_fpxx->fplx_aisino == FPLX_AISINO_ZYFP &&
               !strcmp(stp_fpxx->slv, "0.05")) {
        stp_fpxx->yysbz[8] = '1';
    }
    return 0;
}
//解析发票代码和税额
int GetArrayLine2(HFPXX stp_fpxx, char *arrays)
{
    int index = 0;
    char slv_s[100] = {0};
    int slv_len;
    int count;
    count = 0;
    while (arrays[index] != 'V' && (index < strlen(arrays))) {
        stp_fpxx->fphm[count] = arrays[index];
        count++;
        index++;
    }
    index++;
    count = 0;
    while (arrays[index] != ',' && (index < strlen(arrays))) {
        slv_s[count] = arrays[index];
        count++;
        index++;
    }
    slv_len = strlen(slv_s);
    if (slv_s[slv_len - 1] == 0x30)  //最后一位如果是0的话就删掉变成2位
    {
        slv_s[slv_len - 1] = 0;
    }
    sprintf(stp_fpxx->slv, "%s", slv_s);
    index++;
    count = 0;
    while (arrays[index] != ',' && (index < strlen(arrays))) {
        stp_fpxx->je[count] = arrays[index];
        count++;
        index++;
    }
    index++;
    count = 0;
    while (arrays[index] != ';' && (index < strlen(arrays))) {
        stp_fpxx->se[count] = arrays[index];
        count++;
        index++;
    }
    return 0;
}

int GetArrayLine3(HFPXX stp_fpxx, char *arrays)
{
    int index = 0;
    int count;
    char kpsj[24] = {0};
    count = 0;
    while (arrays[index] != '@' && (index < strlen(arrays))) {
        kpsj[count] = arrays[index];
        count++;
        index++;
    }
    index++;
    ClsSpace(kpsj, strlen(kpsj));
    ClsCharacter(kpsj, strlen(kpsj), ':');
    UpdateFpxxAllTime(kpsj, stp_fpxx);

    count = 0;
    while (arrays[index] != '@' && (index < strlen(arrays))) {
        stp_fpxx->kprjbbh[count] = arrays[index];
        count++;
        index++;
    }
    index++;

    count = 0;
    while (arrays[index] != '@' && (index < strlen(arrays))) {
        stp_fpxx->driverVersion[count] = arrays[index];
        count++;
        index++;
    }
    index++;
    return 0;
}

int GetArrayLine8(HFPXX stp_fpxx, char *arrays)
{
    if (strlen(arrays) > 768)
        return -1;
    if (!strchr(arrays, '*')) {
        strcpy(stp_fpxx->zyspmc, arrays);
        return 0;
    }
    char szBuf[769] = "";
    strcpy(szBuf, arrays);
    char *p = strchr(szBuf + 1, '*');
    if (!p) {
        strcpy(stp_fpxx->zyspmc, arrays);  //存在特殊情况
        return 0;
    }
    int nIndex = p - szBuf;
    if (nIndex > 768 || nIndex < 0)
        return -3;
    strncpy(stp_fpxx->zyspsmmc, arrays, nIndex + 1);  //默认字符串缓冲已清空
    strcpy(stp_fpxx->zyspmc, arrays + nIndex + 1);
    return 0;
}

//划分发票明细
uint8 SplitInvoiceDetail(uint8 *invoice_data, uint8 *myarray[])
{
    uint32 i = 0, reamining_length = 0;
    uint8 *last = invoice_data, *next = NULL;

    while ((next = (uint8 *)strstr((const char *)last, "\n")) != NULL) {
        myarray[i] = calloc(1, abs(next - last) + 100);
        if (myarray[i] == NULL) {
            return 0;
        }
        memset(myarray[i], 0, abs(next - last) + 100);
        memcpy(myarray[i], last, abs(next - last));
        next += 1;
        last = next;
        next = NULL;
        i++;
    }
    reamining_length = strlen((const char *)last);
    myarray[i] = calloc(1, reamining_length + 100);
    if (myarray[i] == NULL) {
        return 0;
    }
    memset(myarray[i], 0, reamining_length + 100);
    memcpy(myarray[i], last, reamining_length);
    i++;
    return i;
}

int GetRedInformation(HFPXX fpxx, const char *szArrayLine10)
{
    fpxx->isRed = true;
    if ((fpxx->fplx_aisino == FPLX_AISINO_ZYFP) || (fpxx->fplx_aisino == FPLX_AISINO_DZZP)) {
        if (strlen(szArrayLine10) >= 18) {
            char yfpdmhm[64] = {0};
            char *start;
            if (strlen(szArrayLine10) < 18)
                return ERR_BUF_CHECK;
            memcpy(yfpdmhm, szArrayLine10 + 2, strlen(szArrayLine10) - 2);
            start = strstr(yfpdmhm, "@");
            if (start == NULL) {
                _WriteLog(LL_WARN, "发票字符串中没有蓝字发票代码号码信息");  //存在此类情况
            } else {
                if (strstr(start, "51YKP") != NULL) {
                    _WriteLog(LL_WARN, "疑似51云开票接口开出的红字专票蓝票代码号码缺失");
                } else {
                    memcpy(fpxx->blueFpdm, yfpdmhm, start - yfpdmhm);
                    //此修改防止部分发票数据原发票号码跟乱码数据
                    memcpy(fpxx->blueFphm, start + 1, 8);
                }
            }
        }
        if (strcmp(fpxx->bz, "仅供培训使用")) {
            char *start;
            start = strstr(fpxx->bz, "开具红字增值税专用发票信息表编号");
            if (start == NULL) {
                _WriteLog(LL_WARN, "RefOfflineinv decode failed, BZ=%s", fpxx->bz);
                return ERR_BUF_CHECK;
            }
            start += strlen("开具红字增值税专用发票信息表编号");
            memcpy(fpxx->redNum, start, 16);
            if (!CheckTzdh(fpxx->redNum, "s")) {
                //存在特殊情况需要兼容
                _WriteLog(LL_WARN, "HZTZD failed : %s", fpxx->redNum);
            }
        }
    } else {  // red PT DZ
        if (strcmp(fpxx->bz, "仅供培训使用")) {
            char *start;
            char *end;
            start = strstr(fpxx->bz, "对应正数发票代码:");
            if (start == NULL) {
                _WriteLog(LL_WARN, "RefOfflineinv decode failed, BZ=%s", fpxx->bz);
                return ERR_BUF_CHECK;
            }
            start += strlen("对应正数发票代码:");
            end = strstr(start, "号码:");
            memcpy(fpxx->blueFpdm, start, end - start);
            start += strlen(fpxx->blueFpdm) + strlen("号码:");
            end = start + 8;  //资料显示发票号码一般为8位，不知道有无特殊情况
            memcpy(fpxx->blueFphm, start, end - start);
        }
    }
    return RET_SUCCESS;
}

int DecodeOldTypeCode(uint8 *pOldTypeCodeGBK, int nOldTypeCodeGBK, HFPXX fpxx)
{
    char *pOpDataLine = NULL, *pAltSepFlag = NULL;
    size_t nOldTypeCodeLen = nOldTypeCodeGBK * 3 + 1024;
    int nRet = -1, i = 0, nChildRet = 0;
    char *arrays[32] = {NULL};
    char szBuf[256] = "";

    uint8 *pOldTypeCode = calloc(1, nOldTypeCodeLen);
    if (!pOldTypeCode)
        return -1;
    G2U_GBKIgnore((char *)pOldTypeCodeGBK, nOldTypeCodeGBK,
                  (char *)pOldTypeCode,  //可能存在0x00字符，原先strlen方式就会少数据
                  nOldTypeCodeLen);
    StringRemoveChar((char *)pOldTypeCode, nOldTypeCodeGBK,
                     0x00);  //清除可能存在的0x00字符，不知是否有其他影响
    i = SplitInvoiceDetail((uint8 *)pOldTypeCode, (uint8 **)arrays);
    if (i < 23) {
        _WriteLog(LL_FATAL, "strlen(pOldTypeCodeGBK) = %d  sizeof(pOldTypeCodeGBK) = %d",
                  strlen((char *)pOldTypeCodeGBK), nOldTypeCodeGBK);
        _WriteLog(LL_FATAL, "SplitInvoiceDetail failed,line_num = %d", i);
        goto DecodeOldTypeCode_Finish;
    }
    //--line 3-7
    GetArrayLine3(fpxx, arrays[3]);  //先解出开票软件版本号，后续操作有判断版本号的
    memcpy(fpxx->gfmc, arrays[4], strlen(arrays[4]));
    memcpy(fpxx->gfsh, arrays[5], strlen(arrays[5]));
    memcpy(fpxx->gfdzdh, arrays[6], strlen(arrays[6]));
    memcpy(fpxx->gfyhzh, arrays[7], strlen(arrays[7]));
    //--line 8 小规模纳税人1% 3%特殊税率开具原因
    if (GetAisino3NodeValue(arrays[8], szBuf, true))
        fpxx->sslkjly = atoi(szBuf);
    pOpDataLine = arrays[8];
    if (FPLX_AISINO_PTFP == fpxx->fplx_aisino || FPLX_AISINO_ZYFP == fpxx->fplx_aisino) {
        strcpy(fpxx->jmbbh, pOpDataLine);
    } else {  // DZFP DZZP JSFP
        pAltSepFlag = strchr(pOpDataLine, '@');
        if (pAltSepFlag) {
            memset(fpxx->jqbh, 0, sizeof fpxx->jqbh);
            strncpy(fpxx->jqbh, pOpDataLine, pAltSepFlag - pOpDataLine);
            strcpy(fpxx->pubcode, pAltSepFlag + 1);
        } else
            strcpy(fpxx->jqbh, pOpDataLine);
    }
    //--line 9
    nChildRet = GetArrayLine8(fpxx, arrays[9]);
    if (nChildRet < 0) {
        if (fpxx->zfbz != 1) {
            _WriteLog(LL_FATAL, "GetArrayLine8 = %d", nChildRet);
            nRet = -2;
            goto DecodeOldTypeCode_Finish;
        }  // else == 1 空白作废发票，无专用商品明细
    }
    //--line 10
    // pAisino3SepString = strstr(arrays[10],
    // defAisino3SepString);//注释原因因部分发票乱码或此处异常导致崩溃，此处解析意义不大 if
    // (pAisino3SepString) {
    //    //++todo
    //    // XSDJBH(销售单据编号)，未知，待测试
    //    memset(pAisino3SepString, 0, sizeof(defAisino3SepString));
    //}
    // pAltSepFlag = strchr(arrays[10], '@');
    // if (!pAltSepFlag)
    //    strcpy(fpxx->szPeriodCount, arrays[10]);
    // else
    //    strncpy(fpxx->szPeriodCount, arrays[10], pAltSepFlag - arrays[10]);
    //--line 11-15
    memcpy(fpxx->xfmc, arrays[11], strlen(arrays[11]));
    memcpy(fpxx->xfsh, arrays[12], strlen(arrays[12]));
    memcpy(fpxx->xfdzdh, arrays[13], strlen(arrays[13]));
    memcpy(fpxx->xfyhzh, arrays[14], strlen(arrays[14]));
    memcpy(fpxx->kpr, arrays[15], strlen(arrays[15]));
    //wang 06 15 
    char *findstart=NULL;
    char szBuf1[10];
    char kprsave[10];
    char abIn[2];
    int num1,num2;
    findstart = strstr(fpxx->kpr,"?");
    //0x3f,0x39,0x38,0x37,0x65,0xbf,0xa1,0xb1,0xf2,0x0,[2023-06-15 15:25:03.622] 
    if(findstart != NULL){
        for(i=0;i<4;)
        {
            if(findstart[i+1] >= '0' && findstart[i+1] <= '9')
            {
                num1 = findstart[i+1] - '0';
            }
            else if(findstart[i+1] >= 'a' && findstart[i+1] <= 'f')
            {
                num1 = 10 + findstart[i+1] - 'a';
            }
            if(findstart[i+2] >= '0' && findstart[i+2] <= '9')
            {
                num2 = findstart[i+2] - '0';
            }
            else if(findstart[i+2] >= 'a' && findstart[i+2] <= 'f')
            {
                num2 = 10 + findstart[i+2] - 'a';
            }
            if(i == 0)
                abIn[1] = num1*16 + num2;
            else 
                abIn[0] = num1*16 + num2;
            i+=2;
        }
        EncordingConvert("unicodelittle", "gbk", abIn, 2, szBuf1, sizeof(szBuf1));
        //printf("szBuf1=%s,sizeof(szBuf1)=%d\r\n",szBuf1,strlen(szBuf1));
        strcpy(kprsave,findstart+5);
        strcpy(fpxx->kpr,szBuf1);
        strcat(fpxx->kpr,kprsave);
    }
    //--line 16 复核人
    if (GetAisino3NodeValue(arrays[16], szBuf, true) && strcmp(szBuf, ";;")) {
        // example：复核人名称[defAisino3SplitFlag]026;20220121;2
        char szDst[3][768];
        memset(szDst, 0, sizeof(szDst));
        if (GetSplitStringSimple(szBuf, ";", szDst, 3) < 0)
            goto DecodeOldTypeCode_Finish;
        fpxx->blueFplx = atoi(szDst[0]);
        sprintf(fpxx->blueKpsj, "%s000000", szDst[1]);
        fpxx->hcyy = atoi(szDst[2]);
    }
    strcpy(fpxx->fhr, arrays[16]);
    // line17 收款人
    GetAisino3NodeValue(arrays[17], fpxx->kjbbh, true);
    strcpy(fpxx->skr, arrays[17]);
    // line 18
    if (strlen(arrays[18]))
        GBKBase64ToUTF8(arrays[18], fpxx->bz, sizeof(fpxx->bz));
    memcpy(fpxx->je, arrays[19], strlen(arrays[19]));
    memcpy(fpxx->se, arrays[20], strlen(arrays[20]));
    double jshj = atof(fpxx->je) + atof(fpxx->se);
    sprintf(fpxx->jshj, "%4.2f", jshj);
    //负数才检查红字信息
    if (atof(fpxx->je) < 0 && GetRedInformation(fpxx, arrays[10]) < 0) {
        nRet = ERR_BUF_CHECK;
        goto DecodeOldTypeCode_Finish;
    }
    //是否有清单
    if (strcmp((const char *)arrays[21], "Y") == 0) {
        sprintf(fpxx->qdbj, "Y");
        AnalyzeDetail(fpxx, arrays[22], 1);
        AnalyzeDetail(fpxx, arrays[23], 0);
    } else if (strcmp((const char *)arrays[21], "N") == 0) {
        sprintf(fpxx->qdbj, "N");
        AnalyzeDetail(fpxx, arrays[22], 0);
    }
    GetArrayLine2(fpxx, arrays[2]);
    GetArrayLine1(fpxx, arrays[1]);  // 1依赖部分2解析出来的

    nRet = 0;
DecodeOldTypeCode_Finish:
    Free2dArray((uint8 **)arrays, 32);
    free(pOldTypeCode);
    return nRet;
}

void GetAisinoFpzt(HFPXX fpxx)
{
    double num = 0.0;
    num = atof(fpxx->je);
    //金额计算什么数据出来
    if (num > 0.0 && !fpxx->zfbz)
        fpxx->fpzt = FPSTATUS_BLUE;
    if (num < 0.0 && !fpxx->zfbz)
        fpxx->fpzt = FPSTATUS_RED;
    if (num == 0.0 && fpxx->zfbz)
        fpxx->fpzt = FPSTATUS_BLANK_WASTE;
    if (num > 0.0 && fpxx->zfbz)
        fpxx->fpzt = FPSTATUS_BLUE_WASTE;
    if (num < 0.0 && fpxx->zfbz)
        fpxx->fpzt = FPSTATUS_RED_WASTE;
}

//此前的手动解析已经足够了，这里通过官方解析覆盖几个重要的参数值.如果有问题，会提示金额等信息不匹配，需要以官方为准
//如果解析的都是正确的，覆盖官方的也不会有影响
int UpdateOfficiaMethod(uint8 *byte_2, HFPXX fpxx)
{
    char szKPRJVersion[24];
    int num2 = 5;
    int i, j, k;
    // fphm
    double num = (double)(((byte_2[0] & 16) > 0) ? 100 : -100);
    int InvNo = *(uint32 *)(byte_2 + 1) & 0xfffffff;
    sprintf(fpxx->fphm, "%08d", InvNo);
    // fpdm
    if ((byte_2[34 + num2] & 192) != 192) {
        strncpy(fpxx->fpdm, (char *)byte_2 + 33 + num2, 10);
    } else {
        uint8 array[10];
        for (k = 0; k < 10; k++) {
            array[k] = byte_2[34 + num2 + k];
        }
        method_64(array, fpxx->fpdm);
    }
    // je
    double dPrice = 0.0;
    for (i = 0; i < 5; i++) {
        dPrice = dPrice * 256.0 + (double)byte_2[9 - i];
    }
    dPrice /= num;
    sprintf(fpxx->je, "%0.2f", dPrice);
    // se
    double Tax = 0.0;
    Tax = (double)((int)(byte_2[4] & 240) + (byte_2[16] >> 4));
    for (j = 0; j < 4; j++) {
        Tax = Tax * 256.0 + (double)byte_2[13 - j];
    }
    Tax /= num;
    sprintf(fpxx->se, "%0.2f", Tax);
    // slv 这里有个默认税率的读取，本代码可用于空白作废时，综合税率取不到的情况
    GetAisinoFpzt(fpxx);
    if (fpxx->fpzt == FPSTATUS_BLANK_WASTE)
        strcpy(fpxx->slv, "0.13");  //空白作废0.13税率
    // gfsh
    strncpy(fpxx->gfsh, (char *)byte_2 + 18, 20);
    if (!strcmp(fpxx->gfsh, "000000123456789"))
        strcpy(fpxx->gfsh, "000000000000000");
    //还有几个硬件参数，有可能离线发票二进制信息里面没有，这里要补全
    GetAisinoKPRJVersion(fpxx->hDev->nDriverVersionNum, szKPRJVersion);
    //向开票软件看齐，默认编码版本号不强行覆盖，如果没有则不覆盖（空白作废）
    if (!strlen(fpxx->driverVersion))
        strcpy(fpxx->driverVersion, fpxx->hDev->szDriverVersion);
    if (!strlen(fpxx->kprjbbh))
        strcpy(fpxx->kprjbbh, szKPRJVersion);
    if (!strlen(fpxx->jmbbh))
        strcpy(fpxx->jmbbh, fpxx->hDev->szJMBBH);
    if (!strlen(fpxx->taVersion))
        strcpy(fpxx->taVersion, defTaxAuthorityVersionAisino);
    //--强制更新读取发票里面没有信息
    //开票机号
    fpxx->kpjh = fpxx->hDev->uICCardNo;
    //所属月份
    memset(fpxx->ssyf, 0, sizeof(fpxx->ssyf));
    strncpy(fpxx->ssyf, fpxx->kpsj_standard, 6);
    return 0;
}

//解析发票数据到结构体
//参考 Aisino.FTaxBase.TaxCard.method_24(bool) : InvDetail @060001B9
int AisinoInvRaw2Fpxx(uint8 *byte_2, HFPXX fpxx)
{
    uint8 *pOldTypeCodeIndex;  //兼容发票前部分还有BarcodeKey字符的特殊发票
    char fplx_s[5] = {0};
    if (strstr((const char *)byte_2 + 65, "BarcodeKey") != NULL) {
        memcpy(fplx_s, byte_2 + 65 + 32, 1);
        pOldTypeCodeIndex = byte_2 + 65 + 32;
    } else {
        memcpy(fplx_s, byte_2 + 65, 1);
        pOldTypeCodeIndex = byte_2 + 65;
    }
    if (byte_2[65] == 'c' || byte_2[65] == 's' || byte_2[65] == 'q' || byte_2[65] == 'p' ||
        byte_2[65] == 'd') {
    } else {
        _WriteLog(LL_FATAL, "Unkown inv type! byte_2 TYPE  = %c\n",
                  byte_2[65]);  //保留此判断的目的为识别是否为此类发票，无实际意义
    }
    if (fplx_s[0] == 'c' || fplx_s[0] == 's' || fplx_s[0] == 'q' || fplx_s[0] == 'p' ||
        fplx_s[0] == 'd') {
    } else {
        _WriteLog(LL_FATAL, "Unkown inv type! fplx_s TYPE  = %c\n", byte_2[65]);
        return -1;
    }
    int num2 = 5;
    uint8 abOD[12];
    memcpy(abOD, byte_2 + 43 + num2, 12);
    if (abOD[0] != 76)
        return -2;

    int num3 = *(uint16 *)(abOD + 1);
    int num4 = *(uint8 *)(abOD + 9);
    num3 += num4 * 65536;
    int num5 = 108;  //专票普票电票卷票的密文区长度为108,机动车190，货运144
    memcpy(fpxx->mw, byte_2 + 65 + num3 - num5 - 20, num5);
    memcpy(fpxx->jym, byte_2 + 65 + num3 - 20, 20);
    ushort num6 = *(ushort *)(byte_2 + 65 + num3);
    Base64_Encode((char *)(byte_2 + 65 + num3 + 2), num6, fpxx->sign);
    // dzsyh
    if (byte_2[65 + num3 + 2 + (int)num6] != 255 || byte_2[65 + num3 + 2 + (int)num6 + 1] != 255 ||
        byte_2[65 + num3 + 2 + (int)num6 + 2] != 255 ||
        byte_2[65 + num3 + 2 + (int)num6 + 3] != 255) {
        uint32 *pN = (uint32 *)(byte_2 + 65 + num3 + 2 + num6);
        uint32 nIndex = bswap_32(*pN);
        fpxx->dzsyh = nIndex;
    }
    // upload flag
    uint8 bUPload = byte_2[65 + num3 + 2 + (int)num6 + 4];
    fpxx->bIsUpload = bUPload == 0 ? 1 : 0;
    // tax class -- not use
    // uint8 bTaxClass = byte_2[0] & 15;
    // zfbz
    if (byte_2[0] < 80)
        fpxx->zfbz = 1;
    else
        fpxx->zfbz = 0;
    // zfsj
    if (fpxx->zfbz) {
        int num7 = (int)(byte_2[65 + num3 + 2 + (int)num6 + 4 + 1] * 16) +
                   (byte_2[65 + num3 + 2 + (int)num6 + 4 + 2] >> 4);
        int num8 = (int)(byte_2[65 + num3 + 2 + (int)num6 + 4 + 2] & 15);
        int num9 = byte_2[65 + num3 + 2 + (int)num6 + 4 + 3] >> 3;
        int num10 = (int)((byte_2[65 + num3 + 2 + (int)num6 + 4 + 3] & 7) * 4) +
                    (byte_2[65 + num3 + 2 + (int)num6 + 4 + 4] >> 6);
        int num11 = (int)(byte_2[65 + num3 + 2 + (int)num6 + 4 + 4] & 63);
        sprintf(fpxx->zfsj, "%04d%02d%02d%02d%02d00", num7, num8, num9, num10, num11);
        TranslateStandTime(2, fpxx->zfsj, (char *)fpxx->zfsj_F2);
    }
    AisinoFPLXFlag2CommonFPLX((uint8 *)fplx_s, (uint8 *)&fpxx->fplx);
    CommonFPLX2AisinoFPLX(fpxx->fplx, &fpxx->fplx_aisino);
    int nOldTypeCodeLen = num3 - 128;
    //防止后面的验证码和密文干扰，utf8转换
    uint8 *pOldTypeCode = calloc(1, nOldTypeCodeLen + 1);
    if (!pOldTypeCode)
        return -3;
    if (strstr((const char *)byte_2 + 65, "BarcodeKey") != NULL) {
        memcpy(pOldTypeCode, pOldTypeCodeIndex, nOldTypeCodeLen - 32);
    } else {
        memcpy(pOldTypeCode, pOldTypeCodeIndex, nOldTypeCodeLen);
    }
    // 20220314，为了兼容友商第三方开票组件问题，强制将oldtypdecode内存中所有\0强制转换成空格
    MemoryFormart(MF_TRANSLATE_ZERO2SPACE, (char *)pOldTypeCode, nOldTypeCodeLen);
    _WriteHexToDebugFile("oldtypdecode.txt", pOldTypeCode, nOldTypeCodeLen);
    int nChildRet = DecodeOldTypeCode(pOldTypeCode, nOldTypeCodeLen, fpxx);
    free(pOldTypeCode);
    if (nChildRet < 0) {
        printf("DecodeOldTypeCode failed nChildRet = %d\n", nChildRet);
        return -4;
    }
    UpdateOfficiaMethod(byte_2, fpxx);
    return 0;
}
