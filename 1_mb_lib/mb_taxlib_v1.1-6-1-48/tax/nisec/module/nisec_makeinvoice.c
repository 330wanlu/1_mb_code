/*****************************************************************************
File name:   nisec_makeinvoice.c
Description: nisec��Ʊ��ش���
Author:      Zako
Version:     1.0
Date:        2019.10
History:
201910.01    �����ʼ��
*****************************************************************************/
#include "nisec_makeinvoice.h"

int MoveTail(uint8 *abInvTemplate, char *szDst, int nInputStrLen)
{
    int nFormatInputBufLen = nInputStrLen | 0x1f;
    int result = -1;
    int v9 = *(_BYTE *)(abInvTemplate + 2) + (*(_BYTE *)(abInvTemplate + 1) << 8);
    uint8 v85 = ((BYTE4(v9) & 3) + (signed int)v9) >> 2;
    uint8 v81 = 0;
    unsigned int v66 = 280;
    uint8 abEBP[32] = {0};
    memcpy(abEBP + 4, &szDst, 4);
    memcpy(abEBP + 8, &nInputStrLen, 4);
    memcpy(abEBP + 0xc, &nFormatInputBufLen, 4);
    int nChildOK = 1;
    do {
        uint8 v67 = (uint8)v81;
        uint8 v68 = *(_BYTE *)(abInvTemplate + 4 * (uint8)v81 + 6);
        if (v68 == 0xFDu) {  //��ʱ��ʵ��
        } else if (v68 <= 0xFDu || v68 > 0xFFu) {
            uint8 v69 = *(_BYTE *)(abInvTemplate + 4 * (uint8)v81 + 9);
            if (v69 & 8) {
                switch (v69 & 0xF0) {
                    case 0: {
                        int nOffset = v66;
                        int nCtrlNum = 2 * *(_BYTE *)(abInvTemplate + 4 * (uint8)v81 + 7);
                        char szBufVar[128] = {0};
                        strncpy(szBufVar, szDst + nOffset, nCtrlNum);
                        int nCtrlNum2 = 2 * *(_BYTE *)(abInvTemplate + 4 * (uint8)v81 + 8);
                        if (nCtrlNum2 > nCtrlNum) {
                            nChildOK = -1;
                            goto MoveTail_Finish;  // error
                        }
                        memcpy(szDst + nOffset + nCtrlNum2, szDst + nOffset + nCtrlNum,
                               nInputStrLen - nOffset - nCtrlNum);
                        nInputStrLen = nInputStrLen - nCtrlNum + nCtrlNum2;
                        if (0 == atoi(szBufVar))
                            goto MoveTail_LABEL_164;  // all zero/not num
                        if (NisecBaseConv(szBufVar, nCtrlNum, 0xa, szDst + nOffset, 0x10,
                                          nCtrlNum2) < 0)
                            return -2;
                        goto MoveTail_LABEL_164;
                    }
                    case 0x10:
                    case 0x20:
                    case 0x30:
                        //��ʱ��ʵ��
                        result = -2;
                        goto MoveTail_Finish;
                    case 0x40:
                        goto MoveTail_LABEL_164;
                    default:
                        result = -3;
                        goto MoveTail_Finish;
                }
                return result;
            }
        MoveTail_LABEL_164:
            v66 += 2 * *(_BYTE *)(abInvTemplate + 4 * v67 + 8);
        }
        LOBYTE(v81) = (_BYTE)v81 + 1;
    } while ((uint8)v81 < (uint8)v85);

MoveTail_Finish:
    if (nChildOK < 0)
        return -3;
    return nInputStrLen;
}

//����ΪKP��Ϣ���ɵ�hexstr�����Ϊbin
int NisecHexStr2USBRaw(HFPXX hFpxx, char *szInOutBuf)
{
    int nCtrlNum = 0;
    int nInputOrigLen = strlen(szInOutBuf);
    if (nInputOrigLen < 0x68)
        return -1;
    char *szDst = (char *)calloc(1, nInputOrigLen + 1024);  // hexstr -> bin, len/2
    if (!szDst)
        return -2;
    int nChildRet = -1;
    int nDstStrLen = nInputOrigLen;
    while (1) {
        strcpy(szDst, szInOutBuf);
        // copy1 prepare xfshbuf
        nCtrlNum = 0xc;
        //ԭ����10�ֽڸ���ԭ����10�ֽ�
        if (NisecBaseConv(szDst + 104, nCtrlNum, 0xa, szDst + 104, 0x10, 0xa) < 0) {
            nChildRet = -3;
            break;
        }
        memmove(szDst + 114, szDst + nCtrlNum + 104, nDstStrLen - nCtrlNum - 104);
        nDstStrLen = nDstStrLen - nCtrlNum + 10;
        // copy2 prepare gfshbuf
        nCtrlNum = 0x14;
        memmove(szDst + 140, szDst + nCtrlNum + 114, nDstStrLen - nCtrlNum - 114);
        if (NisecBaseConv(szDst + 114, nCtrlNum, 0x24, szDst + 114, 0x10, 0x1a) < 0) {
            nChildRet = -4;
            break;
        }
        nDstStrLen = nDstStrLen - nCtrlNum + 26;
        // copy3 gfsh
        memmove(szDst + 166, szDst + nCtrlNum + 140, nDstStrLen - nCtrlNum - 140);
        SpecialGfshChange((char *)(szDst + 140));
        if (NisecBaseConv((char *)(szDst + 140), nCtrlNum, 0x24, szDst + 140, 0x10, 0x1a) < 0) {
            nChildRet = -5;
            break;
        }
        nDstStrLen = nDstStrLen - nCtrlNum + 26;
        // copy4 xfsh
        memmove(szDst + 192, szDst + nCtrlNum + 166, nDstStrLen - nCtrlNum - 166);
        if (NisecBaseConv((char *)(szDst + 166), nCtrlNum, 0x24, szDst + 166, 0x10, 0x1a) < 0) {
            nChildRet = -6;
            break;
        }
        nDstStrLen = nDstStrLen - nCtrlNum + 26;
        int nNewDstStrLen = MoveTail(hFpxx->hDev->abInvTemplate, szDst, nDstStrLen);
        if (nNewDstStrLen < 0) {
            nChildRet = -7;
            break;
        }
        nDstStrLen = nNewDstStrLen;
        if (nDstStrLen / 2 > nInputOrigLen) {  // check Bufflen, hexstr -> bin, len/2
            nChildRet = -8;
            break;
        }

        nChildRet = 0;
        break;
    }
    if (nChildRet < 0) {
        free(szDst);
        return nChildRet;
    }
    //_WriteHexToDebugFile("nisec-hexstr-modify.bin", (uint8 *)szDst, nDstStrLen);
    memset(szInOutBuf, 0, nInputOrigLen);  // set zero
    int nRetByteLen = Str2Byte((uint8 *)szInOutBuf, szDst, nDstStrLen);
    free(szDst);
    return nRetByteLen;
}

inline int FillTwocharRepeat(char *pBuff, char *szTwocharStr, int nRepeatCount)
{
    int i = 0;
    for (i = 0; i < nRepeatCount; i++) {
        strcat(pBuff + i, szTwocharStr);
    }
    return nRepeatCount * 2;
}

//Ҫ��֤Dstbuff�㹻�� orig:NisecConv_UTF82Str
int StrToNisecTemplate(bool bUtf8Buf, char *pDstBuf, char *abInput, int nInputLen)
{
    char szNumbuf[16] = "", *pEncodingBuf = NULL;
    int nChildRet = 0;  //���ܷ��ظ�������ֹ����д��
    int nGBKBufLen = nInputLen * 3 + 512, nEncodingSize = 0;
    uint8 nHeadSize = 0;
    do {
        nHeadSize = bUtf8Buf ? 4 : 2;
        pEncodingBuf = (char *)calloc(1, nGBKBufLen);
        if (!pEncodingBuf)
            break;
        if (bUtf8Buf) {  // dst want utf8
            if ((nEncodingSize = G2U_UTF8Ignore(abInput, nInputLen, pEncodingBuf, nGBKBufLen)) < 0)
                break;
        } else {  // dst want gbk
            if ((nEncodingSize = U2G_GBKIgnore(abInput, nInputLen, pEncodingBuf, nGBKBufLen)) < 0)
                break;
        }
        nChildRet = Byte2Str(pDstBuf + nHeadSize, (uint8 *)pEncodingBuf, nEncodingSize);
        sprintf(szNumbuf, "%0*x", nHeadSize, nChildRet / 2);
        memcpy(pDstBuf, szNumbuf, strlen(szNumbuf));
        nChildRet += nHeadSize;
    } while (false);

    if (pEncodingBuf)
        free(pEncodingBuf);
    return nChildRet;
}

// ���ɷ�����Ŀ���嵥��Ŀ���ݣ�0�����嵥, 1���嵥
int BuildFYXMAndQDXM(uint8 bPrimaryQDorFY, int nSPXXCount, struct Spxx *headSpxx, char *pDetailBuff)
{
    if (nSPXXCount > 2001) {
        _WriteLog(LL_FATAL, "Spmx count limit 2000");
        return -1;
    }
    char szBuf[512] = "";
    int nIndex = 0;
    struct Spxx *pObjSpxx = NULL;
    if (0 == bPrimaryQDorFY) {
        pObjSpxx = headSpxx->stp_next;  //��ϸ��Ŀ
    } else if (1 == bPrimaryQDorFY)
        pObjSpxx = headSpxx;  //�嵥��Ŀ
    else
        return -2;
    uint8 bDoFYXM = 1;
    int i = 0;
    while (pObjSpxx) {
        if (0 == i) {
            int nToWriteHead = nSPXXCount;
            if (bPrimaryQDorFY && bDoFYXM)
                nToWriteHead = 1;  //�嵥��Ŀfyxmֻ��һ��'����������嵥��'
            // FYXM QDXM --head
            sprintf(pDetailBuff + nIndex, "%04x", nToWriteHead);
            nIndex += 4;
            i++;
            continue;
        }
        // No.
        //�ٷ������и�BUG�����ֻ�洢2λ0000(����1+���1)������256���ֽ�ֻ�ܴ�ͷ����
        uint8 bNumber = (uint8)i;  // 4byte -> 1byte
        sprintf(szBuf, "01%02x", bNumber);
        strcpy(pDetailBuff + nIndex, szBuf);
        nIndex += strlen(szBuf);
        // fphxz
        nIndex += StrToNisecTemplate(false, pDetailBuff + nIndex, pObjSpxx->fphxz,
                                     strlen(pObjSpxx->fphxz));
        // je(no tax),nisec��100.00ת��Ϊ100,�����ݲ�����
        nIndex +=
            StrToNisecTemplate(false, pDetailBuff + nIndex, pObjSpxx->je, strlen(pObjSpxx->je));
        // slv
        nIndex +=
            StrToNisecTemplate(false, pDetailBuff + nIndex, pObjSpxx->slv, strlen(pObjSpxx->slv));
        // se
        nIndex +=
            StrToNisecTemplate(false, pDetailBuff + nIndex, pObjSpxx->se, strlen(pObjSpxx->se));
        // SPSMMC+SPMC
        sprintf(szBuf, "%s%s", pObjSpxx->spsmmc, pObjSpxx->spmc);
        nIndex += StrToNisecTemplate(false, pDetailBuff + nIndex, szBuf, strlen(szBuf));
        // spsm,�ò��������ҷ��������ݲ�֧��
        nIndex +=
            StrToNisecTemplate(false, pDetailBuff + nIndex, pObjSpxx->spsm, strlen(pObjSpxx->spsm));
        // ggxh
        nIndex +=
            StrToNisecTemplate(false, pDetailBuff + nIndex, pObjSpxx->ggxh, strlen(pObjSpxx->ggxh));
        // dw
        nIndex +=
            StrToNisecTemplate(false, pDetailBuff + nIndex, pObjSpxx->jldw, strlen(pObjSpxx->jldw));
        // spsl
        nIndex +=
            StrToNisecTemplate(false, pDetailBuff + nIndex, pObjSpxx->sl, strlen(pObjSpxx->sl));
        // dj
        nIndex +=
            StrToNisecTemplate(false, pDetailBuff + nIndex, pObjSpxx->dj, strlen(pObjSpxx->dj));
        if (!strcmp(pObjSpxx->hsjbz, "1")) {
            _WriteLog(LL_FATAL, "Nisec's rawbin cannot make invoice with hsjbz(spxx)");
            return -3;
        }
        // hsjbz
        nIndex += StrToNisecTemplate(false, pDetailBuff + nIndex, pObjSpxx->hsjbz,
                                     strlen(pObjSpxx->hsjbz));
        // spbh
        nIndex +=
            StrToNisecTemplate(false, pDetailBuff + nIndex, pObjSpxx->spbh, strlen(pObjSpxx->spbh));
        // zxbm��
        nIndex += StrToNisecTemplate(false, pDetailBuff + nIndex, "", 0);
        // yhzcbs
        nIndex +=
            StrToNisecTemplate(false, pDetailBuff + nIndex, pObjSpxx->xsyh, strlen(pObjSpxx->xsyh));
        // lslvbs
        nIndex += StrToNisecTemplate(false, pDetailBuff + nIndex, pObjSpxx->lslvbs,
                                     strlen(pObjSpxx->lslvbs));
        // ZZSTSGL
        nIndex +=
            StrToNisecTemplate(false, pDetailBuff + nIndex, pObjSpxx->yhsm, strlen(pObjSpxx->yhsm));
        // Next
        i++;
        pObjSpxx = pObjSpxx->stp_next;
        // QDXM,fyxm finish
        if (bPrimaryQDorFY && bDoFYXM) {
            i = 0;  //�嵥����FYXMheadֻ��һ�еģ�д����ɣ���ʼд��QDXM����
            bDoFYXM = 0;
        }
    }
    if (!bPrimaryQDorFY) {  //��ϸ д��FYXM��д��QDXM����
        sprintf(pDetailBuff + nIndex, "%04x", 0);
        nIndex += 4;
    }
    return nIndex;
}

//ͬ˰����Ʒ�ϲ�����
int BuildQTXM(bool bUtf8Buf, char *pBuff, HFPXX fpxx)
{
    //ʹ��winhexճ��16�����ַ������ɵõ����ӻ�����
    struct SameTaxRate {
        long double je;
        long double se;
        char slv[16];
        struct SameTaxRate *pNext;
    };
    struct SameTaxRate *linkTaxRate = (struct SameTaxRate *)calloc(1, sizeof(struct SameTaxRate));
    char szBuf[64] = {0};
    int nSlvCount = 0, i = 0, nIndex = 0;
    struct Spxx *spxx = fpxx->stp_MxxxHead;
    struct SameTaxRate *pLink = NULL;
    struct SameTaxRate *pTarget = NULL;
    long double ldPrice = 0;
    //���Ե�spxx = spxx->stp_next;Ϊ��ָ�����
    for (i = 0; i < fpxx->spsl && spxx->stp_next != NULL; i++) {
        // init
        pLink = linkTaxRate;  //�׽ڵ�Ϊ��
        pTarget = NULL;
        ldPrice = 0;
        spxx = spxx->stp_next;
        // find target
        while (NULL != pLink->pNext) {
            pLink = pLink->pNext;
            if (strcmp(pLink->slv, spxx->slv))
                continue;
            pTarget = pLink;
        }
        // pLink����ĩβ�ڵ�
        if (!pTarget) {
            pTarget = (struct SameTaxRate *)calloc(1, sizeof(struct SameTaxRate));
            strcpy(pTarget->slv, spxx->slv);
            pLink->pNext = pTarget;
            nSlvCount++;
        }
        ldPrice = strtold(spxx->je, NULL);
        pTarget->je += ldPrice;
        ldPrice = strtold(spxx->se, NULL);
        pTarget->se += ldPrice;
    }
    sprintf(pBuff + nIndex, "%04x", nSlvCount);
    nIndex += 4;
    pLink = linkTaxRate;
    while (pLink->pNext) {
        pLink = pLink->pNext;
        // slv
        nIndex += StrToNisecTemplate(bUtf8Buf, pBuff + nIndex, pLink->slv, strlen(pLink->slv));
        // ͬ˰����Ʒ�ܽ��
        memset(szBuf, 0, sizeof(szBuf));
        sprintf(szBuf, "%.24Lf", pLink->je);
        PriceRound(szBuf, 2, szBuf);
        nIndex += StrToNisecTemplate(bUtf8Buf, pBuff + nIndex, szBuf, strlen(szBuf));
        //ͬ˰����Ʒ��˰��
        memset(szBuf, 0, sizeof(szBuf));
        sprintf(szBuf, "%.24Lf", pLink->se);
        PriceRound(szBuf, 2, szBuf);
        nIndex += StrToNisecTemplate(bUtf8Buf, pBuff + nIndex, szBuf, strlen(szBuf));
    }
    // free linkTaxRate
    struct SameTaxRate *pNode = NULL;
    pLink = linkTaxRate;
    while (pLink->pNext) {
        pNode = pLink->pNext;
        pLink->pNext = pNode->pNext;
        free(pNode);
    }
    free(linkTaxRate);
    return nIndex;
}

// BuildMakeInvHexStr
int NisecMakeInvHexStr(HFPXX hFpxx, char *pBuff)
{
    int nRet = ERR_LOGIC_BUILD_FORMATBUF;
    int nIndex = 0;
    char szBuf[512] = "", szFPDMHMFormater[24] = "";
    uint8 bFPDMDefaultLen = 0, bFPHMDefaultLen = 0;
    HDEV hDev = hFpxx->hDev;
    int nFpModelVer = 0x9917;  //��Ʊ��δ���ԣ���Ʊ=��Ʊ��ʽ
    while (1) {
        // init
        if (hDev->abInvTemplate[3] != hDev->bOperateInvType ||
            hDev->abInvTemplate[3] != hFpxx->fplx)
            break;  //����Ʊ����һ��
        if ((strlen(hFpxx->fpdm) < 8) || strlen(hFpxx->fphm) < 6)
            break;
        bFPDMDefaultLen = NisecGetInvTemplateValue(hDev->abInvTemplate, 0, 1) * 2;  // FPDM'len
        bFPHMDefaultLen = NisecGetInvTemplateValue(hDev->abInvTemplate, 1, 1) * 2;  // FPHM'len
        if (FPLX_COMMON_HYFP == hFpxx->fplx)
            nFpModelVer = 0x9914;
        // device time-yymmdd
        strncpy(pBuff, hFpxx->kpsj_standard, 8);  //ĩβ����0
        nIndex += 8;
        // hjje
        GetFotmatJEStr(hFpxx->je, szBuf);
        if (12 != strlen(szBuf))
            break;
        sprintf(pBuff + nIndex, "%s%s", szBuf, szBuf);
        nIndex += 24;
        // 52��0
        nIndex += FillTwocharRepeat(pBuff + nIndex, "00", 26);
        // unknow �ٷ�dll�ײ⾲ֵ̬
        if (GetDeviceBBH(hDev->szDriverVersion))  //==1
            strcpy(szBuf, "FFFEFDFC");
        else  //==0
            strcpy(szBuf, "00000000");
        sprintf(pBuff + nIndex, "%s", szBuf);
        nIndex += 8;
        // hjse
        GetFotmatJEStr(hFpxx->se, szBuf);
        if (12 != strlen(szBuf))
            break;
        sprintf(pBuff + nIndex, "%s", szBuf);
        nIndex += 12;
        //˰����ش���-��չ����
        sprintf(pBuff + nIndex, "%s", hFpxx->hDev->szTaxAuthorityCodeEx);
        nIndex += 12;
        // xfsh
        sprintf(pBuff + nIndex, "%-20s", hFpxx->xfsh);
        nIndex += 20;
        // gfsh
        sprintf(pBuff + nIndex, "%-20s", hFpxx->gfsh);
        nIndex += 20;
        // xfsh2
        sprintf(pBuff + nIndex, "%-20s", hFpxx->xfsh);
        nIndex += 20;
        // device time-hhmmss
        strncpy(pBuff + nIndex, hFpxx->kpsj_standard + 8, 6);  //ĩβ����0
        nIndex += 6;
        // zfsj
        nIndex += FillTwocharRepeat(pBuff + nIndex, "00", 7);
        // zfr
        nIndex += FillTwocharRepeat(pBuff + nIndex, "20", 20);
        // unknow
        nIndex += FillTwocharRepeat(pBuff + nIndex, "00", 6);
        // qmcs 16bytes
        strcpy(pBuff + nIndex, hDev->szSignParameter);
        nIndex += 16;
        //�˴�������ȫ���շ�Ʊԭ������������Ϊר/�յ�Ʊ�й̶���ʽ�ͳ���
        //--fpdm
        sprintf(szFPDMHMFormater, "%0*llu", bFPDMDefaultLen, atoll(hFpxx->fpdm));
        strcpy(pBuff + nIndex, szFPDMHMFormater);
        nIndex += strlen(szFPDMHMFormater);
        //--fphm
        sprintf(szFPDMHMFormater, "%0*llu", bFPHMDefaultLen, atoll(hFpxx->fphm));
        strcpy(pBuff + nIndex, szFPDMHMFormater);
        nIndex += strlen(szFPDMHMFormater);
        //--yfpdm
        sprintf(szFPDMHMFormater, "%0*llu", bFPDMDefaultLen, atoll(hFpxx->blueFpdm));
        strcpy(pBuff + nIndex, szFPDMHMFormater);
        nIndex += strlen(szFPDMHMFormater);
        //--yfphm
        sprintf(szFPDMHMFormater, "%0*llu", bFPHMDefaultLen, atoll(hFpxx->blueFphm));
        strcpy(pBuff + nIndex, szFPDMHMFormater);
        nIndex += strlen(szFPDMHMFormater);
        // 0 2 static
        nIndex += StrToNisecTemplate(false, pBuff + nIndex, "", 0);
        // static value
        //"0199150399" ��Ʊ�������̶�ֵ,9915�ǰ汾�ţ�����raw��Ʊ��ʽ���¶��仯
        memset(szBuf, 0, sizeof(szBuf));
        myxtoa(nFpModelVer, szBuf, 16, 0);
        sprintf(pBuff + nIndex, "01%s0399", szBuf);
        nIndex += 10;
        // ZYFP����֪ͨ����һ��16λ
        memset(szBuf, 0, sizeof szBuf);
        if (FPLX_COMMON_ZYFP == hFpxx->fplx && hFpxx->isRed) {
            if (strlen(hFpxx->redNum) < 15)
                break;
            strcpy(szBuf, hFpxx->redNum);
        }
        nIndex += StrToNisecTemplate(false, pBuff + nIndex, szBuf, strlen(szBuf));
        // 0130 kpjh
        sprintf(szBuf, "%d", hFpxx->kpjh);
        nIndex += StrToNisecTemplate(false, pBuff + nIndex, szBuf, strlen(szBuf));
        // tspz ++todo��nisecũ��Ʒ�չ���Ʊ����Ʒ�͵ġ�ϡ��������Ʊ�� ���������
        nIndex += StrToNisecTemplate(false, pBuff + nIndex, "", 0);
        // deviceid
        if (12 != strlen(hFpxx->hDev->szDeviceID))
            break;
        nIndex += StrToNisecTemplate(false, pBuff + nIndex, hFpxx->hDev->szDeviceID,
                                     strlen(hFpxx->hDev->szDeviceID));
        // gfmc
        nIndex += StrToNisecTemplate(false, pBuff + nIndex, hFpxx->gfmc, strlen(hFpxx->gfmc));
        // gfdzdh
        nIndex += StrToNisecTemplate(false, pBuff + nIndex, hFpxx->gfdzdh, strlen(hFpxx->gfdzdh));
        // gfyhzh
        nIndex += StrToNisecTemplate(false, pBuff + nIndex, hFpxx->gfyhzh, strlen(hFpxx->gfyhzh));
        // xfmc
        nIndex += StrToNisecTemplate(false, pBuff + nIndex, hFpxx->xfmc, strlen(hFpxx->xfmc));
        // xfdzdh
        nIndex += StrToNisecTemplate(false, pBuff + nIndex, hFpxx->xfdzdh, strlen(hFpxx->xfdzdh));
        // xfyhzh
        nIndex += StrToNisecTemplate(false, pBuff + nIndex, hFpxx->xfyhzh, strlen(hFpxx->xfyhzh));
        // unknow
        nIndex += StrToNisecTemplate(false, pBuff + nIndex, "", 0);
        // slv  ��˰��ʱǿ��Ϊ99.01
        nIndex += StrToNisecTemplate(false, pBuff + nIndex, hFpxx->slv, strlen(hFpxx->slv));
        // jshj
        memset(szBuf, 0, sizeof(szBuf));
        strcpy(szBuf, hFpxx->jshj);
        MemoryFormart(MF_TRANSLATE_ABSOLUTE, szBuf, strlen(szBuf));
        nIndex += StrToNisecTemplate(false, pBuff + nIndex, szBuf, strlen(szBuf));
        // ZYSPSMMC+ZYSPMC
        sprintf(szBuf, "%s%s", hFpxx->zyspsmmc, hFpxx->zyspmc);
        nIndex += StrToNisecTemplate(false, pBuff + nIndex, szBuf, strlen(szBuf));
        // spsm 0 2
        nIndex += StrToNisecTemplate(false, pBuff + nIndex, "", 0);
        // xfmc2 20210827-�°汾��0
        nIndex += StrToNisecTemplate(false, pBuff + nIndex, "", 0);
        // swjgxx ˰���������
        nIndex += StrToNisecTemplate(false, pBuff + nIndex, hFpxx->hDev->szTaxAuthorityName,
                                     strlen(hFpxx->hDev->szTaxAuthorityName));
        // bz
        nIndex += StrToNisecTemplate(false, pBuff + nIndex, hFpxx->bz, strlen(hFpxx->bz));
        // kpr
        nIndex += StrToNisecTemplate(false, pBuff + nIndex, hFpxx->kpr, strlen(hFpxx->kpr));
        // fhr
        nIndex += StrToNisecTemplate(false, pBuff + nIndex, hFpxx->fhr, strlen(hFpxx->fhr));
        // skr
        nIndex += StrToNisecTemplate(false, pBuff + nIndex, hFpxx->skr, strlen(hFpxx->skr));
        // qdbz
        if (!strcasecmp(hFpxx->qdbj, "N"))
            strcpy(szBuf, "0");  //���嵥
        else if (!strcasecmp(hFpxx->qdbj, "Y"))
            strcpy(szBuf, "1");  //�嵥
        else {
            _WriteLog(LL_FATAL, "Unknow input data...");
            break;
        }
        nIndex += StrToNisecTemplate(false, pBuff + nIndex, szBuf, strlen(szBuf));
        // ssyf
        nIndex += StrToNisecTemplate(false, pBuff + nIndex, hFpxx->ssyf, strlen(hFpxx->ssyf));
        // bmbbbh
        nIndex += StrToNisecTemplate(false, pBuff + nIndex, hFpxx->bmbbbh, strlen(hFpxx->bmbbbh));
        // hsslbs
        nIndex += StrToNisecTemplate(false, pBuff + nIndex, hFpxx->hsjbz, strlen(hFpxx->hsjbz));
        //����ƽ̨��Ϣ
        if (strlen(hFpxx->hDev->szPubServer) && (hFpxx->fplx == FPLX_COMMON_DZFP)) {
            //���ɰز����޸ģ�����Ʊ���ڹ�������ƽ̨��Ϣ��
            char szDst[3][768];
            memset(szDst, 0, sizeof(szDst));
            GetSplitStringSimple(hFpxx->pubcode, ";", szDst, 3);
            nIndex += StrToNisecTemplate(false, pBuff + nIndex, szDst[0], strlen(szDst[0]));
            nIndex += StrToNisecTemplate(false, pBuff + nIndex, szDst[1], strlen(szDst[1]));
            nIndex += StrToNisecTemplate(false, pBuff + nIndex, szDst[2], strlen(szDst[2]));
        } else {  //�޹���ƽ̨3����==6��0
            char szNullPub[] = "000000";
            strcat(pBuff + nIndex, szNullPub);
            nIndex += strlen(szNullPub);
        }
        // unknow
        nIndex += StrToNisecTemplate(false, pBuff + nIndex, "", 0);
        // unknow
        nIndex += StrToNisecTemplate(false, pBuff + nIndex, "", 0);
        // 1%3% �Ż�˰�ʿ���ԭ��,nisec��ѡ3��ֵ0x34 35 36
        // ++issue 20220917
        // ����ʱ���֣�С��ģ��˰��רƱ����1%3%˰��ʱ��Ҫѡ�񿪾�˵�����˴��ݲ��޸ģ������ɰش�������������Ҳ��Ҫһ������
        // if (hFpxx->specialFlag == TSFP_XGM_SPECIAL_TAX && hFpxx->fplx != FPLX_COMMON_ZYFP &&
        //     hFpxx->fplx != FPLX_COMMON_DZZP)
        if (hFpxx->specialFlag == TSFP_XGM_SPECIAL_TAX)
        {
            if ((hFpxx->fplx == 7) || (hFpxx->fplx == 26))
            {
                strcpy(szBuf, "2");
            }
			else 
			{
	            if(hFpxx->sslkjly != 2 && hFpxx->sslkjly != 3)
	                strcpy(szBuf,"2");
	            else
	                sprintf(szBuf, "%d", hFpxx->sslkjly);			
			}           
        }
        else
            strcpy(szBuf, "0");
        nIndex += StrToNisecTemplate(false, pBuff + nIndex, szBuf, strlen(szBuf));
        // unknow
        nIndex += StrToNisecTemplate(false, pBuff + nIndex, "", 0);
        // �����Ϣ
        if (hFpxx->fplx != FPLX_COMMON_DZZP && hFpxx->fplx != FPLX_COMMON_ZYFP) {
            if (hFpxx->isRed) {  //���ԭ��
                sprintf(szBuf, "%d,%03d,%.8s,%s,%s", hFpxx->hcyy, hFpxx->blueFplx, hFpxx->blueKpsj,
                        hFpxx->blueFpdm, hFpxx->blueFphm);
            } else
                strcpy(szBuf, ",,,,");
        } else  //רƱ����
            strcpy(szBuf, "");
        nIndex += StrToNisecTemplate(false, pBuff + nIndex, szBuf, strlen(szBuf));
        // unknow
        nIndex += StrToNisecTemplate(false, pBuff + nIndex, "", 0);
        // ���߰汾�� 20220401����
        memset(szBuf, 0, sizeof(szBuf));
        strncpy(szBuf, hFpxx->kpsj_standard, 8);
        NisecKjbbhEncrypt((char *)defTaxAuthorityVersionNisec, hFpxx->fpdm, hFpxx->fphm,
                          hDev->szDeviceID, szBuf, (uint8 *)szBuf);
        nIndex += StrToNisecTemplate(false, pBuff + nIndex, szBuf, strlen(szBuf));
        // fyxm
        uint8 bQDorFY = 0;
        if (!strcasecmp(hFpxx->qdbj, "N"))
            bQDorFY = 0;  //���嵥
        else if (!strcasecmp(hFpxx->qdbj, "Y"))
            bQDorFY = 1;  //�嵥
        int nChildRet = BuildFYXMAndQDXM(bQDorFY, hFpxx->spsl, hFpxx->stp_MxxxHead, pBuff + nIndex);
        if (nChildRet < 0)
            break;
        nIndex += nChildRet;
        // qtxm
        nIndex += BuildQTXM(false, pBuff + nIndex, hFpxx);
        //��������Ʊ�ֽ�ֹ20220923���ٷ�Ŀǰ����1xxxyyyymmdd�汾�ţ�ǰ4λ����1106�ģ��������ͷ��
        if (NisecEncryptMakeInvoiceHeadBuffer0x60Bytes(hDev, pBuff))
            break;
        // finish
        nRet = nIndex;
        break;
    }
    return nRet;
}

int NisecMakeInvIO(HUSB hUSB, HFPXX hFpxx, uint8 *pBuff, int nBuffLen)
{
    int nRet = -3;
    int nChildRet = -1;
    uint8 cb[512] = {0};
#ifdef _NODEVICE_TEST
    nBuffLen = 0;
    pBuff = NULL;
    hFpxx = NULL;
    hUSB = NULL;
    //�˴���cbӦΪ���Ϳ�Ʊ������ɺ��USB��ӦIO
    unsigned char data[56] = {
        0x5E, 0xA0, 0xFD, 0x38, 0x66, 0xB9, 0xE2, 0xAC, 0x8A, 0x30, 0x6D, 0x15, 0x60, 0xC4,
        0xFF, 0x98, 0x0B, 0x47, 0xD1, 0x32, 0xBA, 0xC3, 0xB1, 0x54, 0xA2, 0x50, 0x5C, 0xB2,
        0x37, 0xD7, 0x22, 0x5A, 0xF8, 0x45, 0xE3, 0xAF, 0x7F, 0xEE, 0x69, 0xC8, 0x0B, 0xA4,
        0x3B, 0x74, 0x0B, 0x00, 0xCF, 0x06, 0x13, 0x20, 0x20, 0x01, 0x27, 0x14, 0x47, 0x41};
    memcpy(cb, data, 56);
    nChildRet = 56;
    if (nChildRet) {
    }
    _WriteLog(LL_FATAL, "_NODEVICE_TEST Mode��Jump really make!");
    nRet = 0;
#else
    //++debug �������Ʊ���Ƚ���
    ////_WriteLog(LL_FATAL, "��Ʊ���̻�������Ϊ��ֹʧ��Ʊ�����ѽ������տ�Ʊ�����ͣ�"
    ////                    "���ֶ��ڴ�����ȡ��ע�Ͳ����Ʊģ��");
    ////SetLastError(hFpxx->hDev->hUSB, ERR_PROGRAME_NEED_UNLOCK,
    ////             "�ѽ��õײ����տ�Ʊ������,����ϵ������Ա");
    ////logout(INFO, "TAXLIB", "��Ʊ����", "�̺�%s,�ײ㿪Ʊʧ�ܣ�%s\r\n", hFpxx->hDev->szDeviceID,
    ////       hFpxx->hDev->hUSB->errinfo);
    ////return -100;

    while (1) {
        int nCmdLen = 2;
        memcpy(cb, "\xfe\x09", nCmdLen);
        if (hFpxx->isRed) {
            nCmdLen = 3;
            memcpy(cb, "\xfe\x09\x01", nCmdLen);
        }
        if ((nChildRet = NisecLogicIO(hUSB, cb, nCmdLen, sizeof cb, pBuff, nBuffLen)) < 0) {
            logout(INFO, "TAXLIB", "��Ʊ����",
                   "���һ����Ʊ�ײ����ʧ��,NisecMakeInvIO failed 1,nChildRet = %d\r\n",
                   nChildRet);
            nRet = -1;
            break;
        }
        memcpy(cb, "\xfe\x08\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01", 16);
        if ((nChildRet = NisecLogicIO(hUSB, cb, 16, sizeof cb, NULL, 0)) < 0) {
            logout(INFO, "TAXLIB", "��Ʊ����",
                   "���һ����Ʊ�ײ����ʧ��,NisecMakeInvIO failed 2,nChildRet = %d\r\n",
                   nChildRet);
            nRet = -2;
            break;
        }
        // get make result
        if ((nChildRet = NisecDecKPRep(hFpxx, cb, nChildRet)) < 0) {
            logout(INFO, "TAXLIB", "��Ʊ����",
                   "���һ����Ʊ�ײ����ʧ��,NisecMakeInvIO failed 3,nChildRet = %d\r\n",
                   nChildRet);
            nRet = -3;
            break;
        }
        nRet = 0;
        break;
    }
#endif
    return nRet;
}

int NisecUpdateAuthKeyDo(uint8 bVersion, HDEV hDev)
{
    int nChildRet = 0, nCmdLen = 3, nRet = RET_SUCCESS;
    uint8 cb[256] = {0}, abBuf[128] = {0};
    do {
        // TC_GetAuthKeyVersion,����ֵΪ0����"����汾����˰�����豸��ȫ�汾�����������"����Ҫ���µײ�-��ȫ��֤��Կ
        memcpy(cb, "\xfe\x16", 2);
        cb[2] = bVersion;
        if ((nChildRet = NisecLogicIO(hDev->hUSB, cb, nCmdLen, sizeof cb, NULL, 0)) < 0) {
            nRet = nChildRet;
            break;
        }
        if (nChildRet != 1 || cb[0] != 0)
            break;
        if ((nChildRet = NisecBuildNewAuthKey(bVersion, hDev->szDeviceID, abBuf)) < 0) {
            nRet = nChildRet;
            break;
        }
        // TC_UpdateAuthKeyVersion
        // FE110000000000000000000000009500
        // 86029CA26EC615FAF244DA47EF453331114222B42B33509EBC77176B67B0CC33A0E0
        // or
        // FE110100000000000000000000009500
        // 33A3508345D80018DDD1D692B7958277493EBF1CF72351AA2329596C3DAE570EC4E8
        memcpy(cb, "\xfe\x11", 2);
        cb[2] = bVersion;
        if ((nChildRet = NisecLogicIO(hDev->hUSB, cb, nCmdLen, sizeof cb, abBuf, nChildRet)) < 0) {
            nRet = nChildRet;
            break;
        }
    } while (false);
    return nRet;
}

int NisecUpdateAuthKey(HDEV hDev)
{
    int nChildRet = 0, nRet = RET_SUCCESS;
    // ��ȫ��֤��Կ��Ϊ�����汾00��01���κ�һ���汾��������0������Ҫ����
    do {
        nChildRet = NisecDriverVersionNumAbove1106(hDev->szDriverVersion);
        if (!nChildRet)
            break;
        else if (nChildRet < 0) {
            nRet = nChildRet;
            break;
        }
        if (NisecUpdateAuthKeyDo(0, hDev) < 0) {
            nRet = ERR_DEVICE_NEEDUPDATE_CONFIG;
            break;
        }
        if (NisecUpdateAuthKeyDo(1, hDev) < 0) {
            nRet = ERR_DEVICE_NEEDUPDATE_CONFIG;
            break;
        }
    } while (false);
    return nRet;
}

//��ͨ��Ʊ�͵��ӷ�Ʊ�İ�ʽһ�£����Ե���һ�ַ�Ʊ����
int NisecMakeInvoiceMain(HFPXX fpxx)
{
    char szBuf[512] = "";
    HDEV hDev = fpxx->hDev;
    HUSB hUSB = fpxx->hDev->hUSB;
    int nRet = -1, nBufLen = 0, nChildRet = -1;
    char *pBuff = NULL;
    // ==������鼰׼��
    if (fpxx->fplx != FPLX_COMMON_DZFP && fpxx->fplx != FPLX_COMMON_PTFP &&
        fpxx->fplx != FPLX_COMMON_ZYFP) {
        return SetLastError(hUSB, ERR_PROGRAME_UNSUPPORT, "Not support invoice type:%d",
                            fpxx->fplx);
    }
    if (!strcmp(fpxx->hsjbz, "1"))
        return SetLastError(hUSB, ERR_LOGIC_UNSUPPORT_METHOD,
                            "Rawbin cannot make invoice with hsjbz(main)");
    if ((nBufLen = CalcAllocBuffLen(fpxx)) < 0)
        return SetLastError(hUSB, ERR_BUF_SIZE, "Alloc buf failed");
    // ���㹫������ƽ̨��ȡ����
    if (strlen(hDev->szPubServer)) {
        char szPubPtlx[16];
#ifdef _NODEVICE_TEST
        strcpy(szBuf, "D4H2G5F4");
#else
        Build8BytesRandomTQM(szBuf);
#endif
        memset(szPubPtlx, 0, sizeof(szPubPtlx));
        strncpy(szPubPtlx, hDev->szPubServer, 2);  // copy type 2bytes
        sprintf(fpxx->pubcode, "%s;%s;%s", szPubPtlx, hDev->szPubServer + 2, szBuf);
    }
    //==��ʽ��Ʊ
    if (NisecEntry(hDev->hUSB, NOP_OPEN_FULL) < 0)
        return SetLastError(hUSB, ERR_DEVICE_OPEN, "OpenDevice failed");
    while (1) {
        if (!CheckDeviceSupportInvType(hDev->abSupportInvtype, fpxx->fplx)) {
            nRet = SetLastError(hDev->hUSB, ERR_LOGIC_NOTFOUND_INVOICETYPE,
                                "Device not has this invoice-type, type:%d", fpxx->fplx);
            break;
        }
        if (!(pBuff = (char *)calloc(1, nBufLen))) {
            nRet = SetLastError(hUSB, ERR_BUF_ALLOC, "Buf alloc");
            break;
        }
        // 1-��ȡ����ģ��,kp��������Ҫʹ��
        // SELECT_OPTION_MONITOR��������ȡ��
        if (NisecSelectInvoiceType(hDev, fpxx->fplx,
                                   SELECT_OPTION_TEMPLATE | SELECT_OPTION_MONITOR) < 0) {
            nRet = SetLastError(hUSB, ERR_DEVICE_SELECTINV, "ERR_DEVICE_SELECTINV");
            break;
        }
        // 2-�����豸��ȫ��Կ
        if (NisecUpdateAuthKey(hDev)) {
            nRet = SetLastError(hUSB, ERR_DEVICE_NEEDUPDATE_CONFIG, "�豸��ȫ��Կ����ʧ��");
            break;
        }
        // 3-��ȡ��ǰ��Ʊ������룬FPDM FPHM,12λ��8λ������ǰ�油0;
        if (CommonGetCurrentInvCode(hDev, fpxx->fpdm, fpxx->fphm, fpxx->fpendhm) < 0) {
            if ((strlen(fpxx->fphm) > 0) || (strlen(fpxx->fpdm) > 0))
                nRet = SetLastError(
                    hUSB, ERR_DEVICE_CURRENT_INVOICE,
                    "��ǰ��Ʊ��������봫��ķ�Ʊ������벻һ��,���ٴ�ȷ�ϱ����ظ�����");
            else
                nRet = SetLastError(hUSB, ERR_DEVICE_CURRENT_INVOICE,
                                    "��ȡ��ǰ��Ʊ�������ʧ��,���ٴ�ȷ�ϱ����ظ�����");
            break;
        }
        // 4-��Ʊ16�����ַ�������
        if ((nChildRet = NisecMakeInvHexStr(fpxx, pBuff)) < 0) {
            nRet =
                SetLastError(hUSB, ERR_LOGIC_BUILD_FORMATBUF, "MakeInvHexStr failed:%d", nChildRet);
            break;
        }
        // 5--�ַ���ת������
        MemoryFormart(MF_TRANSLATE_ZERO2SPACE, pBuff, nChildRet);
        _WriteHexToDebugFile("kpstr_nisec_my.txt", (uint8 *)pBuff, nChildRet);
        if ((nChildRet = NisecHexStr2USBRaw(fpxx, pBuff)) < 0) {
            nRet = SetLastError(hUSB, ERR_BUF_CONVERT, "HexStr2USBRaw failed:%d", nChildRet);
            break;
        }
        // 6-��ƱIO
        // _WriteHexToDebugFile("nisec_usbio.bin", (uint8 *)pBuff, nChildRet);
        if ((nChildRet = NisecMakeInvIO(hDev->hUSB, fpxx, (uint8 *)pBuff, nChildRet)) < 0) {
            logout(INFO, "TAXLIB", "��Ʊ����",
                   "�̺ţ�%s,�ײ㿪Ʊʧ��,result = %d,LastError��%s��\r\n", hDev->szDeviceID,
                   nChildRet, hDev->hUSB->errinfo);
            break;
        }
        nRet = RET_SUCCESS;
        break;
    }
    _WriteLog(!nRet ? LL_INFO : LL_FATAL, "Makeinvoice %s, Type:%d(%d) FPDM:%s FPHM:%s",
              !nRet ? "successful" : "failed", fpxx->fplx, fpxx->fplx_aisino, fpxx->fpdm,
              fpxx->fphm);
    if (strcmp(fpxx->fphm, fpxx->fpendhm) == 0) {
        _WriteLog(LL_FATAL, "��ʹ�õ���Ʊ�����һ�ŷ�Ʊ,������ɺ���Ҫ���жϵ����");
        fpxx->need_restart = 1;
    }
    if (pBuff)
        free(pBuff);
    if (NisecEntry(hDev->hUSB, NOP_CLOSE) < 0)
        return ERR_DEVICE_CLOSE;
    return nRet;
}

int NisecDecKPRep(HFPXX hFpxx, uint8 *pUSBRaw, int nUSBRawLen)
{
    uint8 abBufTmp[256] = "";
    char szOutFPDM[24] = "";
    char szOutFPHM[24] = "";
    char szOutKPSJ[24] = "";
    uint8 bFPDMHMLen = 0;
    // ZY��PT��DZʵ�ʳ��ȷ��س���56�ֽڣ�ת��Ϊ�ַ���Ϊ112���ַ�������
    // 40�ֽ�Ϊ���ģ�5Byte FPDM, 4Byte FPHM, 7Byte KPSJ
    if (nUSBRawLen > 64 || nUSBRawLen < 48) {
        logout(INFO, "TAXLIB", "��Ʊ����", "˰�̿��߽����ѯ���س������� nUSBRawLen = %d\r\n",
               nUSBRawLen);
        return -1;
    }
    if (NisecMWDecAPI(hFpxx->fplx, hFpxx->isRed, hFpxx->hDev->szDeviceID,
                      hFpxx->hDev->szTaxAuthorityCodeEx, hFpxx->kpsj_standard, hFpxx->xfsh, pUSBRaw,
                      hFpxx->mw) < 0)
        return -2;
    Byte2Str((char *)abBufTmp, pUSBRaw + 40, 16);
    bFPDMHMLen = NisecGetInvTemplateValue(hFpxx->hDev->abInvTemplate, 0, 1) * 2;
    NisecBaseConv((char *)abBufTmp, 10, 16, szOutFPDM, 10, bFPDMHMLen);
    bFPDMHMLen = NisecGetInvTemplateValue(hFpxx->hDev->abInvTemplate, 1, 1) * 2;
    NisecBaseConv((char *)abBufTmp + 10, 8, 16, szOutFPHM, 10, bFPDMHMLen);
    strcpy(szOutKPSJ, (char *)abBufTmp + 18);
    if (strcasecmp(szOutFPDM, hFpxx->fpdm) || strcasecmp(szOutFPHM, hFpxx->fphm)) {
        _WriteLog(LL_WARN,
                  "Make invoice maybe some question, orig: FPDM:%s FPHM:%s KPSJ:%s, "
                  "out:FPDM:%s FPHM:%s KPSJ:%s",
                  hFpxx->fpdm, hFpxx->fphm, hFpxx->kpsj_standard, szOutFPDM, szOutFPHM, szOutKPSJ);
    }
    //�Կ��������صĿ�Ʊʱ��Ϊ׼��������д��Ŀ�Ʊʱ��
    UpdateFpxxAllTime(szOutKPSJ, hFpxx);
    if (NisecJYMDecAPI(pUSBRaw, 40, hFpxx->jym) < 0)
        return -3;
    _WriteLog(LL_DEBUG, "MakeInvRet: MW:%s JYM:%s", hFpxx->mw, hFpxx->jym);
    return 0;
}

int NisecUpdateNodeInfo(HFPXX fpxx)
{
    //����Ʊ�ֵĴ���
    if (fpxx->isMultiTax) {
        strcpy(fpxx->slv, "99.01");
    }
    if (ZYFP_CEZS == fpxx->zyfpLx) {
        strcpy(fpxx->hsjbz, "2");
    }
    return 0;
}

// pRecvBuff must > 100bytes
// bHashType==1 nisecǩ��hash(SHA1), ==2˰�ֽ���hash
// bHashType==3 cntaxǩ��hash(SM3)/˰�ֽ���hash
int NisecCryptHash(HUSB hUSB, uint8 bHashType, uint8 *abSha1Hash, int nSignBuflen, char *pRecvBuff,
                   int nRecvBuffLen)
{
    if (bHashType <= 0 || bHashType > 4)
        return -1;
    //Ŀǰ��2������, ǩ����˰��IO,nSignBuflen==21ʱΪǩ��
    uint8 cb[512] = {0};
    int nChildRet = -1;
    int nRet = -1;
    while (1) {  //�����ǰloaddev��ʱ��crypt_deviceopen��δִ�У�������ܻ���Ҫ����ִ��һ��
        //-- Step1
        memcpy(cb, "\xfe\x6f\x0d\x01", 4);
        if ((nChildRet = NisecLogicIO(hUSB, cb, 4, sizeof(cb), NULL, 0)) < 0)
            break;
        //-- Step2
        memset(cb, 0, sizeof cb);
        if (1 == bHashType || 3 == bHashType)  // sign fp
            memcpy(cb, "\xfe\x6f\x12", 3);
        else if (2 == bHashType)  //˰��ͨ��
            memcpy(cb, "\xfe\x6f\x12\x00\x01", 5);
        else
            break;
        // cbʵ�����ݴ���3\5���, ������5��û����
        if ((nChildRet = NisecLogicIO(hUSB, cb, 5, sizeof(cb), abSha1Hash, nSignBuflen)) < 0)
            break;
        //-- Step3
        memcpy(cb, "\xfe\x6f\x03", 3);
        // SHA1=0x80 SM3=0x40
        if ((nChildRet = NisecLogicIO(hUSB, cb, 3, sizeof(cb), NULL, 0)) != 0x80 &&
            nChildRet != 0x40)
            break;
        if (nRecvBuffLen < nChildRet)
            break;
        memcpy(pRecvBuff, cb, nChildRet);
        nRet = 0;
        break;
    }
    if (nRet < 0)
        _WriteLog(LL_FATAL, "Nisec CryptHash failed");
    return nRet;
}

//����Ʊǩ���ַ������͵�˰�̲���ȡ��Ӧֵ
int GetDevice128ByteSignRep(HDEV hDev, const char *sSignStrGBKBuff, int nGBKBuffLen,
                            char *pRecvBuff, int nRecvBuffLen)
{
    uint8 abSha1Hash[SHA_DIGEST_LENGTH + 1] = {
        0x82};  //��ʼ��ʱֻ��ֵ���ֽ�Ϊ0x82,����20�ֽ�Ϊsha1Hash
    if (CalcSHA1((void *)sSignStrGBKBuff, nGBKBuffLen, abSha1Hash + 1))
        return -1;
    if (Nisec_Crypt_Login(hDev->hUSB, hDev->szDeviceID) <= 0)
        return -2;
    // buf �ճ�3���ֽ���Ϊ�˺�֮ǰ��aisino�������
    if (NisecCryptHash(hDev->hUSB, 1, abSha1Hash, SHA_DIGEST_LENGTH + 1, pRecvBuff + 3,
                       nRecvBuffLen - 3))
        return -3;
    return 0;
}

// nisec cntax����
char *GetSignBase64(HFPXX fpxx)
{
    char *pSignGBKBuf = NULL, *pRetSignBase64 = NULL;
    int nRetSignBufLen = 0;
    while (1) {
        if (!(pSignGBKBuf = GetSignString(fpxx, &nRetSignBufLen)))
            break;
        _WriteHexToDebugFile("mysign-str.txt", (uint8 *)pSignGBKBuf, nRetSignBufLen);
        if (!(pRetSignBase64 = GetDeviceSignHashBase64(fpxx->hDev, pSignGBKBuf, nRetSignBufLen)))
            break;

        break;
    }
    if (pSignGBKBuf)
        free(pSignGBKBuf);
    return pRetSignBase64;
}

//����δ�ϴ�ͬһ�ŷ�Ʊ���Զ���ظ�ǩ���������һ��ǩ��IOΪ׼ˢ��ǩ��ֵ
//��bEraseSign=trueʱ,��Ĩ��ǩ��,�����ɿ�Ʊ��������Զ�ǩ��
//ǩ�����㷨��kp���Ŀ¼��signinter.dll signex��������,��ֻ�ڷ�Ʊ�ָ�ʱ����ǩ��
int NisecFpxxAppendSignIO(HFPXX fpxx, bool bEraseSign)
{
    HUSB hUSB = fpxx->hDev->hUSB;
    int nChildRet = -1;
    uint8 cb[128] = {0};
    char *pSignBase64 = NULL;
    char *pIOBuf = NULL;
    char szBuf[128] = "", szBuf2[128] = "";
    if (NULL == hUSB)
        return -1;
    if (strlen(fpxx->fpdm) < 8 || strlen(fpxx->fphm) < 8 || strlen(fpxx->hDev->szDeviceTime) < 8)
        return -2;
    while (1) {
        if (NisecEntry(hUSB, NOP_OPEN_FULL) < 0) {
            nChildRet = -3;
            break;
        }
        if (NisecSelectInvoiceType(fpxx->hDev, fpxx->fplx,
                                   SELECT_OPTION_TEMPLATE | SELECT_OPTION_MONITOR) < 0) {
            nChildRet = -4;
            break;
        }
        if (!(pSignBase64 = GetSignBase64(fpxx))) {
            nChildRet = -5;
            break;
        }
        int nIOBufLen = 13 + strlen(pSignBase64);
        if (!(pIOBuf = calloc(1, nIOBufLen))) {
            nChildRet = -6;
            break;
        }

        TransferFPDMAndFPHMTo9Byte(fpxx->fpdm, fpxx->fphm, (uint8 *)pIOBuf);
        strcpy(szBuf, fpxx->kpsj_standard);
        szBuf[8] = '\0';
        Str2Byte((uint8 *)szBuf2, szBuf, 8);
        memcpy(pIOBuf + 9, szBuf2, 4);
        if (!bEraseSign)
            memcpy(pIOBuf + 13, pSignBase64, strlen(pSignBase64));
        memcpy(cb, "\xfe\x2f", 2);
        // _WriteHexToDebugFile("signIO.txt", pIOBuf, nIOBufLen);
        nChildRet = NisecLogicIO(hUSB, cb, 2, sizeof cb, (uint8 *)pIOBuf, nIOBufLen);
        if (nChildRet < 0) {
            nChildRet = -7;
            break;
        }
        if (bEraseSign) {
            strcpy(fpxx->sign, "");
            _WriteLog(LL_DEBUG, "Invoice's sign has benn erased");
        } else {
            strcpy(fpxx->sign, pSignBase64);
            _WriteLog(LL_DEBUG, "Update invoice's sign, successful");
        }

        nChildRet = 0;
        break;
    }
    NisecEntry(hUSB, NOP_CLOSE);
    if (pSignBase64)
        free(pSignBase64);
    if (pIOBuf)
        free(pIOBuf);
    return nChildRet;
}

//ָ����Ʊ���롢���룬�Է�Ʊǩ����ȷ�Խ��м��
//�����Ʊǩ���ǿջ�����ȷ�򲻸Ķ��������ǩ�������ÿ�
int NisecCheckToReSign(HDEV hDev, uint8 bFplx, char *szFpdm, char *szFphm)
{
    HFPXX fpxx = MallocFpxxLinkDev(hDev);
    char *pSignGBKBuf = NULL;
    int nRet = -1;
    while (1) {
        if (NisecQueryInvInfo(hDev, bFplx, szFpdm, szFphm, fpxx))
            break;
        if (fpxx->bIsUpload) {
            _WriteLog(LL_FATAL, "Invoice has been set upload flag, cannot resign");
            break;
        }
        //----check exist fp's sign
        pSignGBKBuf = GetSignBase64(fpxx);
        if (!pSignGBKBuf)
            break;
        if (!strlen(fpxx->sign)) {
            _WriteLog(LL_INFO, "fp's sign is empty");
            // debug-sign again
            // NisecFpxxAppendSignIO(fpxx, false);
        } else if (strcmp(pSignGBKBuf, fpxx->sign)) {
            //����ȷ��������ǩ������������Ѿ�������ķ�Ʊ���ķ�д�������޷�����ǩ��
            _WriteLog(LL_WARN, "fp's sign incorrect, we try to empty it");
            if (NisecFpxxAppendSignIO(fpxx, true)) {
                _WriteLog(LL_WARN, "fp's set empty failed");
                break;
            }
        } else {
            _WriteLog(LL_INFO, "fp's sign check OK");
            // debug-sign again
            // NisecFpxxAppendSignIO(fpxx, true);
        }
        nRet = 0;
        break;
    }
    if (pSignGBKBuf)
        free(pSignGBKBuf);
    if (!nRet)
        _WriteLog(LL_DEBUG, "Sign check,dm:%s hm:%s successful.", fpxx->fpdm, fpxx->fphm);
    else
        _WriteLog(LL_FATAL, "Sign check,dm:%s hm:%s failed.", fpxx->fpdm, fpxx->fphm);

    FreeFpxx(fpxx);
    return nRet;
}