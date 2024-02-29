/*****************************************************************************
File name:   aisino_getinvoice.c
Description: ��Ʊ��ȡ�ָ�����
Author:      Zako
Version:     1.0
Date:        2019.10
History:
20200901     ��rawio2fpxx.c�ļ�����������ϲ�
20200925     �޸�רƱ��Ʊ�ϰ汾��Ʊ��ϸ��������
20220222     ��Ʊ���220107�汾����,�ṹ�Ż�����
20220908     ũ��Ʒ�չ���Ʊ���Ľ����޸�
*****************************************************************************/
#include "aisino_getinvoice.h"

// szRet һ����6-8k�����黺��������20k����
int DecodeInvoiceBlock2(uint8 *pInv, int nInvBuffLen, char *szRet)
{
    uint8 *pSysMemBlock = NULL;
    uint8 *pIndexHaad = (uint8 *)pInv + 0x70;
    if (pIndexHaad[0] != 0x48 || pIndexHaad[1] != 0x65 || pIndexHaad[2] != 0x61 ||
        pIndexHaad[3] != 0x64)
        return -1;

    int nEnData1Len = *(uint16 *)(pIndexHaad + 0x26) + (*(uint8 *)(pIndexHaad + 0x25) << 16);
    uint8 *pEnData1 = pIndexHaad + 0x28;
    //Ԥ���ռ�30k,ʵ����ԶԶ�ò�����ô���ڴ棬ʵ�����ڴ��ѹ����szRet���棬Ϊ�˺�Test����ͬ�����Ͷ������
    pSysMemBlock = calloc(1, nEnData1Len * 30 + 30000);
    if (!pSysMemBlock)
        return -2;
    int nLen =
        InvDecryptBlock2((uint8 *)pSysMemBlock, (long *)pEnData1, nEnData1Len, (long *)szRet);
    free(pSysMemBlock);
    nInvBuffLen = 0;  // no use
    return nLen;
}

//ÿһ��block����Ҫ���������м���
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
    //��������У����
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
    // raw 0x31 �����и�����Ҫ���ȥ
    uint16 nOldTypeBlockBufLen = (uint16)(pBlock3 + 128 - pBlock1 - 0x41);
    memcpy(pBlock1 + 0x31, &nOldTypeBlockBufLen, 2);
    //ǩ������
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
    //�˴���pInvCopy���ȳ䵱���룬�ֳ䵱������壬�����Ҫ�Դ��
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
//�����ò�ѯ�·ݷ�Ʊ����ʱ����һ���� bChildQuery = 0�������İ�bChildQuery =1��
// ����5λ�������ò�ѯ�·ݷ�Ʊ��ϸʱ bChildQuery = 2������4λ
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
    int v20 = 8;  // 8����Ʊ����
    const void *v10 = (const void *)(cb + 8);
    do {
        --v20;
        memcpy(v18, v10, 0x37);
        if (v18[0] != 0xff) {  //��Ʊ�����жϲ��ǺϷ�ֵ���ۼ�
            long v11 = (long)v10 + 0x13;
            //��ǰ��Ʊ���������з�Ʊ�����ѿ��߷�Ʊ�ۼ�
            int v12 = 4;  //�����4����Ʊ��
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

//ͨ����˵�����������ݰ������������ݰ����ȶ���������һ�����ݰ�����block1��block2,�������ǵڶ������ݰ�������Ʊ������������ǩ��������block��У��
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

// ���ܺ�:4 input example: "201805","201611"
int GetInvDetail(HDEV hDev, int nInvoiceNb, char *szFormatYearMonth, uint8 **ppInBuff,
                 int *pnBuffLen,
                 uint8 *aisino_fplx)  //���ӷ�Ʊ���ʹ���������Ӧ�ÿ����жϲ�ʶ��ķ�Ʊ����
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
            _WriteLog(LL_DEBUG, "�̺ţ�%s,GetInvDetail AisinoLogicIO nRet = %d��lasterr = %s",
                      hDev->szDeviceID, nRet, hDev->hUSB->errinfo);
            return -2;
        }
        int nDataLen = nRet - 1;  //���һ����У���ַ�������Ҫȥ��
        memcpy(pEncodeInvoiceBuff + nInvoiceBuffIndex, cb, nDataLen);
        nInvoiceBuffIndex += nDataLen;
        if ((cb[nDataLen - 4] == 0x54 && cb[nDataLen - 3] == 0x61 && cb[nDataLen - 2] == 0x69 &&
             cb[nDataLen - 1] == 0x6c) ||
            nInvoiceBuffIndex > 10240000)  //����ȡ����Ʊ��β��־�����趨��Ʊ���ֵ 10M
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
        int nNewBuffLen = nInvoiceBuffIndex * 20 + 30000;  //��������£���չ10+��
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
    //���ݿ�Ʊ����߼����˴�Ӧ���Ƿ�Ʊ����2(��Ʊ)��41(��Ʊ)��51(��Ʊ)���ַ�Ʊ
    //���ж��Ƿ�ũ��Ʒ�չ���Ʊ��Ŀǰ��ʱ����ȷ�����߼����ݲ����Ϸ�Ʊ���͵��ж�
    // a3Ϊrawinv
    // if ( *(_BYTE *)(a3 + 1) == 2 || *(_BYTE *)(a3 + 1) == 51 || *(_BYTE *)(a3 + 1) == 41 ){}
    //
    //��Ʊ���ũ��Ʒ��Ʊ��־�Ǵ�block2��Ʊ�������ݴӻ�ȡ
    GetFpdmAndSpecialFlagFromBlock2(pBlock2, szBuf);
    if (strstr(szBuf, "V6"))
        bIsNcpSG = true;
    nRet =
        DecodeMWandJYM(bCont, bIsMulitTaxRate, bIsNcpSG, (uint8 *)abRawInvoice + 0x40,
                       hDev->szCompressTaxID, szBuyTaxcode, hDev->szCommonTaxID, szOutMW, szOutJYM);
    return nRet;
}

//��Ʊ�޸������в��ܵ�������API���������ݴ��ң�����޸�����,���Ϊ�˼�����̬������ֻ�ܴ�Devinfo������
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
        //����continue
        bContinue = cb[nChildRet + 1];
        int nDataLen = nChildRet - 1;  //���һ����У���ַ���Ҫȥ��
        memcpy(pInvIOBuf + nIndex, cb, nDataLen);
        nIndex += nDataLen;
        if ((cb[nDataLen - 4] == 0x54 && cb[nDataLen - 3] == 0x61 && cb[nDataLen - 2] == 0x69 &&
             cb[nDataLen - 1] == 0x6c) ||
            nIndex > 10240000)  //����ȡ����Ʊ��β��־�����趨��Ʊ���ֵ 10M
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
    int nOutBuffLen = nIndex * 30 + 30000;  //��������£���չ10+��
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

//�ú�������������ַ���
// 1�����뷢Ʊ���롢���� ���Բ��ҵ�ǰ�·ݻ���������δ�����ķ�Ʊ��Ϣ�����ǳ����˾Ͳ���
// 2����ȫ3�����������Բ�ѯ������ڷ�Ʊ,����ѯʵ�ʿ����ǰ��ꣿ����
// 3) ע�������012345678901 ��Ʊ���룬�����ǰ��0���в�ѯ��12345678901�����
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

// szOutVersion>32�ֽ�,��ȡ���Ƿ�Ʊ��aisino�ڲ��汾�ţ�V3.2.00.220507
//!!!ע�⣬���ڶ����δ����Ʊ���̣�����������Ʊ�����ǿ�Ʊ����汾�޷����
int AisinoGetTailInvVersion(HDEV hDev, char *szOutVersion)
{
    uint8 cb[768] = {0};
    int nChildRet = 0, ncbLen = 0;
    HFPXX fpxx = MallocFpxxLinkDev(hDev);
    int nRet = ERR_GENERIC;
    while (1) {
        ncbLen = sizeof(cb);
        //��IOָ��һ��aa�����������壬������
        // memcpy(cb, "\xfe\xff\xcc\xbc\x53\xe1\xe3\x00\x1f\x00", 10);
        memcpy(cb, "\xfe\xff\xcc\xbc\x53\xe1\xe3\x01\x20\x00", 10);
        if ((nChildRet = AisinoLogicIO(hDev->hUSB, cb, 10, ncbLen, NULL, 0)) < 0) {
            nRet = nChildRet;
            break;
        }
        //������ش���ֵΪff ff ff ff ff ff ff ff ff 00 00 00 00 f7
        if (cb[0] == 0xff && cb[1] == 0xff && cb[2] == 0xff && cb[3] == 0xff && cb[4] == 0xff &&
            cb[5] == 0xff) {
            // 3-4����δ����Ʊ���޷�ͨ��API��ȡ����Ʊ��Ϣ��ֱ������Ʊ�������޿�Ʊ����汾���
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
    //_WriteLog(LL_INFO, "��Ʊ���룺%s,��Ʊ���룺%s,��Ʊʱ�䣺%s,��ַ������ %s", szOutFPDM,
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
    // 0x1b;0x1b ���ȸ�λ��һ�㲻��,0x1c==len
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
    //�������Ҫʵʱʱ�䣬�����þ�̬ʱ��
    AisinoGetDeivceTime(fpxx->hDev->hUSB, fpxx->hDev->szDeviceTime);
    if (TimeOutMonthDeadline(fpxx->kpsj_standard, fpxx->hDev->szDeviceTime)) {
        _WriteLog(LL_FATAL,
                  "Current device time is next month of the waste-invoice's month,DevTime:%s "
                  "InvTime:%s",
                  fpxx->hDev->szDeviceTime, fpxx->kpsj_standard);
        SetLastError(fpxx->hDev->hUSB, ERR_LOGIC_WASTETIME_OVERFLOW, "�������Ͻ�ֹ���ڲ���������");
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
            break;  //ֻ�Ƴ�ĩβ�Ŀո��м�Ŀո񲻹�
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
    //�˴�G2U_GBKIgnoreʹ��sizeof��bufת������ʹ��strlen��ֹ��������ڴ�ճ��
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
            // SPBH blank, ��Ʒ�������ȥ��������
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
//������ÿһ������
// Aisino.Fwkp.BusinessObject.Fpxx.method_0(InvDetail, int) : string+600�����ң����в�����method_3
int AnalyzeDetail(HFPXX stp_fpxx, char *data, uint8 bHeadNode)
{
    int hsjbz = -1;  // init -1
    char szSlvSumary[64] = "";
    bool same_hsjbz = true, same_slv = true;
    size_t nUTF8Len =
        strlen((char *)data) + 1024;  //����1024�ֽڷ�ֹV2��ʽ����ר�÷�Ʊ��ȡʱ��Ʒ��Ϣȱʧ

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
        bAisinoSpmxVersion = 3;  //���а汾
    else if (0 == length % 252)
        bAisinoSpmxVersion = 2;  //���ֵ���רƱ���ַ�Ʊ����ʹ��
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
    //ֻ����v2��v3
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
        //���ֽڵ�����
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
        //�׽ڵ��ж�
        if (bHeadNode) {
            memcpy(stp_fpxx->stp_MxxxHead, Head, sizeof(struct Spxx));
            free(Head);
            stp_fpxx->stp_MxxxHead->stp_next = NULL;
        } else {
            //�Ƿ�˰�۸�
            int hsj_tmp = atoi(Head->hsjbz);
            if (hsjbz == -1) {
                hsjbz = hsj_tmp;  //��һ�γ�ʼ����ֵ
            } else {
                if (hsjbz != hsj_tmp)
                    same_hsjbz = false;
            }
            //�Ƿ��˰��
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
        //�Ƿ�˰�۸�
        if (same_hsjbz)  //��˰�Ͳ���˰ȫһ��
            sprintf(stp_fpxx->hsjbz, "%d", hsjbz);
        else  //��һ��,����
            sprintf(stp_fpxx->hsjbz, "%d", 2);
        //�Ƿ��˰��
        stp_fpxx->isMultiTax = (uint8)!same_slv;
    }

    free(pGBKBuf);
    return 0;
}

int GetArrayLine1(HFPXX stp_fpxx, char *arrays)
{
    int index = 0;
    //��Ʊ����
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
        stp_fpxx->zyfpLx = ZYFP_SNY;  //����ΪZYFP_SNY��ZYFP_SNY_DDZG��ZYFP_RLY��ZYFP_RLY_DDZG
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
    //רƱ��Ʊ�ޱ����汾���������������
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
//������Ʊ�����˰��
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
    if (slv_s[slv_len - 1] == 0x30)  //���һλ�����0�Ļ���ɾ�����2λ
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
        strcpy(stp_fpxx->zyspmc, arrays);  //�����������
        return 0;
    }
    int nIndex = p - szBuf;
    if (nIndex > 768 || nIndex < 0)
        return -3;
    strncpy(stp_fpxx->zyspsmmc, arrays, nIndex + 1);  //Ĭ���ַ������������
    strcpy(stp_fpxx->zyspmc, arrays + nIndex + 1);
    return 0;
}

//���ַ�Ʊ��ϸ
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
                _WriteLog(LL_WARN, "��Ʊ�ַ�����û�����ַ�Ʊ���������Ϣ");  //���ڴ������
            } else {
                if (strstr(start, "51YKP") != NULL) {
                    _WriteLog(LL_WARN, "����51�ƿ�Ʊ�ӿڿ����ĺ���רƱ��Ʊ�������ȱʧ");
                } else {
                    memcpy(fpxx->blueFpdm, yfpdmhm, start - yfpdmhm);
                    //���޸ķ�ֹ���ַ�Ʊ����ԭ��Ʊ�������������
                    memcpy(fpxx->blueFphm, start + 1, 8);
                }
            }
        }
        if (strcmp(fpxx->bz, "������ѵʹ��")) {
            char *start;
            start = strstr(fpxx->bz, "���ߺ�����ֵ˰ר�÷�Ʊ��Ϣ����");
            if (start == NULL) {
                _WriteLog(LL_WARN, "RefOfflineinv decode failed, BZ=%s", fpxx->bz);
                return ERR_BUF_CHECK;
            }
            start += strlen("���ߺ�����ֵ˰ר�÷�Ʊ��Ϣ����");
            memcpy(fpxx->redNum, start, 16);
            if (!CheckTzdh(fpxx->redNum, "s")) {
                //�������������Ҫ����
                _WriteLog(LL_WARN, "HZTZD failed : %s", fpxx->redNum);
            }
        }
    } else {  // red PT DZ
        if (strcmp(fpxx->bz, "������ѵʹ��")) {
            char *start;
            char *end;
            start = strstr(fpxx->bz, "��Ӧ������Ʊ����:");
            if (start == NULL) {
                _WriteLog(LL_WARN, "RefOfflineinv decode failed, BZ=%s", fpxx->bz);
                return ERR_BUF_CHECK;
            }
            start += strlen("��Ӧ������Ʊ����:");
            end = strstr(start, "����:");
            memcpy(fpxx->blueFpdm, start, end - start);
            start += strlen(fpxx->blueFpdm) + strlen("����:");
            end = start + 8;  //������ʾ��Ʊ����һ��Ϊ8λ����֪�������������
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
                  (char *)pOldTypeCode,  //���ܴ���0x00�ַ���ԭ��strlen��ʽ�ͻ�������
                  nOldTypeCodeLen);
    StringRemoveChar((char *)pOldTypeCode, nOldTypeCodeGBK,
                     0x00);  //������ܴ��ڵ�0x00�ַ�����֪�Ƿ�������Ӱ��
    i = SplitInvoiceDetail((uint8 *)pOldTypeCode, (uint8 **)arrays);
    if (i < 23) {
        _WriteLog(LL_FATAL, "strlen(pOldTypeCodeGBK) = %d  sizeof(pOldTypeCodeGBK) = %d",
                  strlen((char *)pOldTypeCodeGBK), nOldTypeCodeGBK);
        _WriteLog(LL_FATAL, "SplitInvoiceDetail failed,line_num = %d", i);
        goto DecodeOldTypeCode_Finish;
    }
    //--line 3-7
    GetArrayLine3(fpxx, arrays[3]);  //�Ƚ����Ʊ����汾�ţ������������жϰ汾�ŵ�
    memcpy(fpxx->gfmc, arrays[4], strlen(arrays[4]));
    memcpy(fpxx->gfsh, arrays[5], strlen(arrays[5]));
    memcpy(fpxx->gfdzdh, arrays[6], strlen(arrays[6]));
    memcpy(fpxx->gfyhzh, arrays[7], strlen(arrays[7]));
    //--line 8 С��ģ��˰��1% 3%����˰�ʿ���ԭ��
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
        }  // else == 1 �հ����Ϸ�Ʊ����ר����Ʒ��ϸ
    }
    //--line 10
    // pAisino3SepString = strstr(arrays[10],
    // defAisino3SepString);//ע��ԭ���򲿷ַ�Ʊ�����˴��쳣���±������˴��������岻�� if
    // (pAisino3SepString) {
    //    //++todo
    //    // XSDJBH(���۵��ݱ��)��δ֪��������
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
    //--line 16 ������
    if (GetAisino3NodeValue(arrays[16], szBuf, true) && strcmp(szBuf, ";;")) {
        // example������������[defAisino3SplitFlag]026;20220121;2
        char szDst[3][768];
        memset(szDst, 0, sizeof(szDst));
        if (GetSplitStringSimple(szBuf, ";", szDst, 3) < 0)
            goto DecodeOldTypeCode_Finish;
        fpxx->blueFplx = atoi(szDst[0]);
        sprintf(fpxx->blueKpsj, "%s000000", szDst[1]);
        fpxx->hcyy = atoi(szDst[2]);
    }
    strcpy(fpxx->fhr, arrays[16]);
    // line17 �տ���
    GetAisino3NodeValue(arrays[17], fpxx->kjbbh, true);
    strcpy(fpxx->skr, arrays[17]);
    // line 18
    if (strlen(arrays[18]))
        GBKBase64ToUTF8(arrays[18], fpxx->bz, sizeof(fpxx->bz));
    memcpy(fpxx->je, arrays[19], strlen(arrays[19]));
    memcpy(fpxx->se, arrays[20], strlen(arrays[20]));
    double jshj = atof(fpxx->je) + atof(fpxx->se);
    sprintf(fpxx->jshj, "%4.2f", jshj);
    //�����ż�������Ϣ
    if (atof(fpxx->je) < 0 && GetRedInformation(fpxx, arrays[10]) < 0) {
        nRet = ERR_BUF_CHECK;
        goto DecodeOldTypeCode_Finish;
    }
    //�Ƿ����嵥
    if (strcmp((const char *)arrays[21], "Y") == 0) {
        sprintf(fpxx->qdbj, "Y");
        AnalyzeDetail(fpxx, arrays[22], 1);
        AnalyzeDetail(fpxx, arrays[23], 0);
    } else if (strcmp((const char *)arrays[21], "N") == 0) {
        sprintf(fpxx->qdbj, "N");
        AnalyzeDetail(fpxx, arrays[22], 0);
    }
    GetArrayLine2(fpxx, arrays[2]);
    GetArrayLine1(fpxx, arrays[1]);  // 1��������2����������

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
    //������ʲô���ݳ���
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

//��ǰ���ֶ������Ѿ��㹻�ˣ�����ͨ���ٷ��������Ǽ�����Ҫ�Ĳ���ֵ.��������⣬����ʾ������Ϣ��ƥ�䣬��Ҫ�Թٷ�Ϊ׼
//��������Ķ�����ȷ�ģ����ǹٷ���Ҳ������Ӱ��
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
    // slv �����и�Ĭ��˰�ʵĶ�ȡ������������ڿհ�����ʱ���ۺ�˰��ȡ���������
    GetAisinoFpzt(fpxx);
    if (fpxx->fpzt == FPSTATUS_BLANK_WASTE)
        strcpy(fpxx->slv, "0.13");  //�հ�����0.13˰��
    // gfsh
    strncpy(fpxx->gfsh, (char *)byte_2 + 18, 20);
    if (!strcmp(fpxx->gfsh, "000000123456789"))
        strcpy(fpxx->gfsh, "000000000000000");
    //���м���Ӳ���������п������߷�Ʊ��������Ϣ����û�У�����Ҫ��ȫ
    GetAisinoKPRJVersion(fpxx->hDev->nDriverVersionNum, szKPRJVersion);
    //��Ʊ������룬Ĭ�ϱ���汾�Ų�ǿ�и��ǣ����û���򲻸��ǣ��հ����ϣ�
    if (!strlen(fpxx->driverVersion))
        strcpy(fpxx->driverVersion, fpxx->hDev->szDriverVersion);
    if (!strlen(fpxx->kprjbbh))
        strcpy(fpxx->kprjbbh, szKPRJVersion);
    if (!strlen(fpxx->jmbbh))
        strcpy(fpxx->jmbbh, fpxx->hDev->szJMBBH);
    if (!strlen(fpxx->taVersion))
        strcpy(fpxx->taVersion, defTaxAuthorityVersionAisino);
    //--ǿ�Ƹ��¶�ȡ��Ʊ����û����Ϣ
    //��Ʊ����
    fpxx->kpjh = fpxx->hDev->uICCardNo;
    //�����·�
    memset(fpxx->ssyf, 0, sizeof(fpxx->ssyf));
    strncpy(fpxx->ssyf, fpxx->kpsj_standard, 6);
    return 0;
}

//������Ʊ���ݵ��ṹ��
//�ο� Aisino.FTaxBase.TaxCard.method_24(bool) : InvDetail @060001B9
int AisinoInvRaw2Fpxx(uint8 *byte_2, HFPXX fpxx)
{
    uint8 *pOldTypeCodeIndex;  //���ݷ�Ʊǰ���ֻ���BarcodeKey�ַ������ⷢƱ
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
                  byte_2[65]);  //�������жϵ�Ŀ��Ϊʶ���Ƿ�Ϊ���෢Ʊ����ʵ������
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
    int num5 = 108;  //רƱ��Ʊ��Ʊ��Ʊ������������Ϊ108,������190������144
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
    //��ֹ�������֤������ĸ��ţ�utf8ת��
    uint8 *pOldTypeCode = calloc(1, nOldTypeCodeLen + 1);
    if (!pOldTypeCode)
        return -3;
    if (strstr((const char *)byte_2 + 65, "BarcodeKey") != NULL) {
        memcpy(pOldTypeCode, pOldTypeCodeIndex, nOldTypeCodeLen - 32);
    } else {
        memcpy(pOldTypeCode, pOldTypeCodeIndex, nOldTypeCodeLen);
    }
    // 20220314��Ϊ�˼������̵�������Ʊ������⣬ǿ�ƽ�oldtypdecode�ڴ�������\0ǿ��ת���ɿո�
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
