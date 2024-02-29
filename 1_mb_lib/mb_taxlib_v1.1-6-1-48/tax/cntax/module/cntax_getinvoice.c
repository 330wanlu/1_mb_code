/*****************************************************************************
File name:   cntax_getinvoice.c
Description: cntax�ķ�Ʊ��ȡ��ش���
Author:      Zako
Version:     1.0
Date:        2021.02
History:
20210205     �������ʵ��
*****************************************************************************/
#include "cntax_getinvoice.h"

uint16 CopySignDataBuf(char *pOrigBuffCopy, int nOrigBufIndex, char *pDetailBuff,
                       int nDetailBufIndex)
{
    //�ٷ��������ǿ���Ʊ�������������ݿ����ݽ���ǩ����ˢ��ǩ����д��ǩ����Ӧ�ò�����ǩ���ֶ��������
    //�ҷ��޷�ʵ�ֿ�Ʊ���Ч����ֻ�ܲ��ù̶�ǩ������ƥ�䷽ʽ����
    //ǩ�����Ȼ�ȡ,�����̴�����δǩ��ʱǩ���ֶ�������δ��ʼ������ȡ����������������Ŀǰ��Э�İ취ֻ��ͨ��ǿ��ˢ�¾�ֵ̬�ķ���
    uint16 usSignSize = 0;
    char szBuf[256] = "";
    bool bHasInitSignData = true;

    memset(szBuf, 0, sizeof(szBuf));
    strncpy(szBuf, pOrigBuffCopy + nOrigBufIndex, 4);
    Str2Byte((uint8 *)&usSignSize, szBuf, 4);
    usSignSize = bswap_16(usSignSize);
    if (usSignSize != 0x0164) {
        usSignSize = 0x0164;
        bHasInitSignData = false;
        _WriteLog(LL_WARN, "usSignSize != 0x0164, force set");
    }
    //���������ǩ�����ȣ������else if

    //�����µ�ǩ������
    sprintf(szBuf, "%04x", usSignSize);
    strncpy(pDetailBuff + 0x11a, szBuf, 4);
    //�������ʼ��Signǩ����
    if (bHasInitSignData) {
        Str2Byte((uint8 *)pDetailBuff + nDetailBufIndex, pOrigBuffCopy + nOrigBufIndex + 4,
                 usSignSize * 2);
    } else
        memset((uint8 *)pDetailBuff + nDetailBufIndex, 0, usSignSize * 2);
    return usSignSize;
}

//��ı�pOrigBuffCopy����������
int CnTaxDecodeSingleInvoice(uint8 *abCurInvoTemp, int nBlockBinLen, char *pOrigBuffCopy,
                             char *pDetailBuff, char *szOutJYM, char *szOutMW)
{
    int nOrigBufIndex = 0, nDetailBufIndex = 0, nRet = 0, nMWIndex = 0, nChildRet = 0;
    char szXfsh[24] = "", szKprq[16] = "", szTaxAuthorityCodeEx[16] = "", szDeviceID[24] = "",
         szBuf[256] = "";
    uint8 bInvType = abCurInvoTemp[3], bMWClearByte0 = 0;
    uint16 usMWBytesSize = 0x30, usSignSize = 0;
    bool bIsRed = false;
    while (1) {
        //����Ҫ�Ի��������м򵥱任����Ҫȡ������Ҫ�����������ĺ�У����,��ЩҪ�ص�ƫ��ȷ��Ϊ�̶�ƫ��
        //!!!ע�⣬����ļ���Ҫ�ؽ�β�����Կո�0x20��䲢�Ҵ�����㣨��nisec�Բ�һ����
        uint8 bInvoiceStatusFlag =
            NisecDecodeNum(pOrigBuffCopy + 4, 0x2) & 0x7f;  // enCOMMON_FP_STATUS
        if (FPSTATUS_RED == bInvoiceStatusFlag || FPSTATUS_RED_WASTE == bInvoiceStatusFlag)
            bIsRed = true;
        // kprq
        strncpy(szKprq, pOrigBuffCopy + 0xe, 8);
        if (strlen(szKprq) != 8)
            break;
        // szTaxAuthorityCodeEx
        memset(szBuf, 0, sizeof(szBuf));
        strncpy(szBuf, pOrigBuffCopy + 0x2e, 0x18);
        Str2Byte((uint8 *)szTaxAuthorityCodeEx, szBuf, strlen(szBuf));
        // taxid
        memset(szBuf, 0, sizeof(szBuf));
        strncpy(szBuf, pOrigBuffCopy + 0x96, 0x28);
        Str2Byte((uint8 *)szXfsh, szBuf, strlen(szBuf));
        // bMWClearByte0,isRed?
        memset(szBuf, 0, sizeof(szBuf));
        strncpy(szBuf, pOrigBuffCopy + 0xfe, 2);
        Str2Byte(&bMWClearByte0, szBuf, 2);
        // devid
        memset(szBuf, 0, sizeof(szBuf));
        strncpy(szDeviceID, pOrigBuffCopy + 0x102, 0xc);
        // mw'size,pDetailBuff����ǩ��������Ҫ������������޸�
        memset(szBuf, 0, sizeof(szBuf));
        strncpy(szBuf, pOrigBuffCopy + 0x11a, 4);  // 0x116?^
        int nSignStrLen = strtol(szBuf, NULL, 16);
        if (nSignStrLen <= 0 || nSignStrLen > nBlockBinLen)
            break;
        // mw's offset
        nMWIndex = 2 * (nBlockBinLen + 0 - (usMWBytesSize + nSignStrLen)) + 0xE;
        if (nMWIndex < 0)
            break;
        //��ʵ�ڴ濽����ʼ����USB���ݱ任�󿽱������ڴ�,���˴�Ϊֹ��ʽ��ʼ�����任�ڴ棬�����ڴ�ƫ�ƶ���һ��
        memcpy(pDetailBuff, pOrigBuffCopy, nMWIndex);
        nDetailBufIndex = nMWIndex;
        nOrigBufIndex = nMWIndex;
        if (FPSTATUS_BLANK_WASTE != bInvoiceStatusFlag) {
            //����
            nChildRet = CntaxMWDecAPI(bInvType, bIsRed, szDeviceID, szTaxAuthorityCodeEx, szKprq,
                                      szXfsh, (uint8 *)pOrigBuffCopy + nMWIndex, szOutMW);
            if (nChildRet < 0)
                break;
            // JYM
            Str2Byte((uint8 *)szBuf, pOrigBuffCopy + nMWIndex, usMWBytesSize * 2);
            NisecJYMDecAPI((uint8 *)szBuf, usMWBytesSize, szOutJYM);
            // copy mw
            memcpy(pDetailBuff + nDetailBufIndex, szOutMW, nChildRet);
            nDetailBufIndex += nChildRet;
        } else {
            memcpy(pDetailBuff + nDetailBufIndex, pOrigBuffCopy + nOrigBufIndex, 0x60);
            nDetailBufIndex += 0x60;
        }
        nOrigBufIndex += 0x60;
        mystrupr(pDetailBuff);  //����ȫת��д
        //�������޸�ǩ���ֶ�����
        usSignSize = CopySignDataBuf(pOrigBuffCopy, nOrigBufIndex, pDetailBuff, nDetailBufIndex);
        nOrigBufIndex += 4 + usSignSize * 2;  // ������4�ֽ�Ϊ'0164'�ַ�������
        nDetailBufIndex += usSignSize;
        //ͷ����������������Block+3��������Ϣ,�����ϲ����������
        int nNewBlockLen = (nDetailBufIndex - 14) / 2;
        sprintf(szBuf, "%08x", nNewBlockLen);
        memcpy(pDetailBuff + 6, szBuf, 8);

        nRet = nDetailBufIndex;
        break;
    }
    if (nRet <= 0)
        _WriteLog(LL_WARN, "CnTaxDecodeSingleInvoice failed, jump out decode");
    return nRet;
}

int RestorClearBuf(uint8 *pDetailBuff, int nIndex, bool *pIsUtf8Buf)
{
    uint8 *pDecBuf = NULL, *pDecBuf2 = NULL;
    int nRet = -1, nEncryptStringSize = 0, nEncryptBytesSize = 0, nContentOffset = 0;
    char szBuf[256];
    memset(szBuf, 0, sizeof szBuf);
    strncpy(szBuf, (char *)pDetailBuff + nIndex, 8);
    nEncryptStringSize = strtol(szBuf, 0, 16);
    if (nEncryptStringSize > DEF_MAX_FPBUF_LEN || nEncryptStringSize % 8)
        return -1;  //�����㷨���ܳ�����Ҫ��8����
    int nBufSize = nEncryptStringSize + 64;
    do {
        pDecBuf = (uint8 *)calloc(1, nBufSize);
        pDecBuf2 = (uint8 *)calloc(1, nBufSize);
        if (!pDecBuf || !pDecBuf2)
            break;
        // 10=8+2���̶�����
        // 8�ֽ�+2�ֽ�(string:0c,��֪�������)
        uint8 *pNewBufAddr = pDetailBuff + nIndex + 10;
        memcpy(pDecBuf2, pNewBufAddr, nEncryptStringSize);
        nEncryptBytesSize = Str2Byte(pDecBuf, (char *)pDecBuf2, nEncryptStringSize);
        if (EncryptDecryptV2String(0, pDecBuf, nEncryptBytesSize, pDecBuf2) < 0)
            break;
        // nEncryptStringSize����ܺ󳤶ȵ�ͬ��ֱ�ӿ�����ȥ,Byte2Str���Ὣ��������0x00ת����00�����ֶ���0
        memset(pNewBufAddr, 0, nEncryptStringSize);
        _WriteHexToDebugFile("CntaxV2-plaintext.my.bin", pDecBuf2, nEncryptBytesSize);
        if (IsUTF8WithBom((char *)pDecBuf2, nEncryptBytesSize)) {
            *pIsUtf8Buf = true;
            // jump UTF8bom 'efbbbf'
            nContentOffset = 3;
        }
        Byte2Str((char *)pNewBufAddr, pDecBuf2 + nContentOffset,
                 nEncryptBytesSize - nContentOffset);
        //����ֻ��ԭ�ⲻ���ķ���ԭ�еļ��ܻ����С����Ϊ�ⲿ������Ҫ���϶�Ӧƫ��
        nRet = nEncryptStringSize;
    } while (false);

    if (pDecBuf)
        free(pDecBuf);
    if (pDecBuf2)
        free(pDecBuf2);
    return nRet;
}

// nToRead������255
int GetNBytesSize(char *buf, uint8 nToRead)
{
    char szTmp[256];
    memset(szTmp, 0, sizeof szTmp);
    if (nToRead > sizeof szTmp)
        return 0;
    strncpy(szTmp, buf, nToRead);
    return strtol(szTmp, 0, 16);
}

int CntaxDecodeBuf2Fpxx(char *pDetailBuff, HFPXX fpxx)
{
    bool bUtf8Buf = false;
    // printf_array(pDetailBuff, strlen(pDetailBuff));
    CommonFPLX2AisinoFPLX(fpxx->fplx, &fpxx->fplx_aisino);
    int nIndex = 4, nMWIndex = 0, nChildRet = 0, nExtraBufSize = 0;
    char szBuf[256];
    // fpzt ����upload��waste��redinv��
    int nTmpNum = NisecDecodeNum(pDetailBuff + nIndex, 0x2);
    NisecFPZTAnalyze(nTmpNum, fpxx);
    nIndex += 10;
    // kprq
    strncpy(fpxx->kpsj_standard, pDetailBuff + nIndex, 8);
    nIndex += 8;
    // hjje
    nTmpNum = NisecDecodeNum(pDetailBuff + nIndex, 0xc);
    sprintf(fpxx->je, "%0.2f", (double)nTmpNum / 100);
    if (fpxx->isRed)
        MemoryFormart(MF_TRANSLATE_RED, fpxx->je, strlen(fpxx->je));
    nIndex += 12;
    // hjse
    nTmpNum = NisecDecodeNum(pDetailBuff + nIndex, 0xc);
    sprintf(fpxx->se, "%0.2f", (double)nTmpNum / 100);
    if (fpxx->isRed)
        MemoryFormart(MF_TRANSLATE_RED, fpxx->se, strlen(fpxx->se));
    nIndex += 12;
    //���������nisec�Ĳ�һ����nisec�������ģ�cntax����byte-string��ʽ�����ȱ�����
    // szTaxAuthorityCodeEx
    nIndex += 24;
    // xfsh
    Str2Byte((uint8 *)fpxx->xfsh, pDetailBuff + nIndex, 40);
    MemoryFormart(MF_TRANSLATE_SPACE2ZERO, fpxx->xfsh, strlen(fpxx->xfsh));
    nIndex += 40;
    // gfsh
    Str2Byte((uint8 *)fpxx->gfsh, pDetailBuff + nIndex, 40);
    MemoryFormart(MF_TRANSLATE_SPACE2ZERO, fpxx->gfsh, strlen(fpxx->gfsh));
    if ((!strcmp(fpxx->gfsh, "00000000000000000000")) || (!strlen(fpxx->gfsh)))
        strcpy(fpxx->gfsh, "000000000000000");  //�޹���˰�š���˰��
    nIndex += 40;
    // xfsh2
    nIndex += 40;
    // 190�ֽڴ�,�Ƿ��Ʊ
    if (!strncmp(pDetailBuff + nIndex, "01", 2)) {
        _WriteLog(LL_DEBUG, "CntaxDecodeBuf2Fpxx:red-invoice");
        fpxx->isRed = true;
    }
    nIndex += 2;
    // kpsj-�ٷ�����̶���ƫ��192�ֽڴ���ȡ��ʵ��xfsh2�ۼ����ʱ190���ֽ�
    strncpy(fpxx->kpsj_standard + 8, pDetailBuff + 192, 6);
    UpdateFpxxAllTime(fpxx->kpsj_standard, fpxx);
    nIndex = 192 + 6;
    // zfrq
    memset(fpxx->zfsj, 0, sizeof(fpxx->zfsj));
    strncpy(fpxx->zfsj, pDetailBuff + nIndex, 14);
    TranslateStandTime(2, fpxx->zfsj, fpxx->zfsj_F2);
    nIndex += 14;
    // zfr, ignore
    memset(szBuf, 0, sizeof(szBuf));
    strncpy(szBuf, pDetailBuff + nIndex, 40);
    nIndex += 40;
    // unknow
    nIndex += 2;
    // ver
    fpxx->nVerInvoice = GetNBytesSize(pDetailBuff + nIndex, 4);
    nIndex += 4;
    //#static index,�˴��Լ��ۼ�Ҳ������258�ֽڴ����̶�ֵҲΪ258
    nIndex = 258;
    // devid
    nIndex += 12;
    // ykfsje �ѿ��������, ignore
    nIndex += 12;
    // qmcs
    memset(szBuf, 0, sizeof szBuf);
    strncpy(fpxx->qmcs, pDetailBuff + nIndex, 4);
    fpxx->qmcs[4] = '\0';
    int nSignStrLen = GetNBytesSize(pDetailBuff + nIndex, 4);
    nIndex += 4;
    // nExtraBufSize 4bytes
    nExtraBufSize = GetNBytesSize(pDetailBuff + nIndex, 4);
    nIndex += 4;
    //#fpdm static index 290
    nIndex = 290;
    //��ȷ�ϣ��ٷ������з�Ʊ�������ֱ���Է�Ʊģ��ȷ�ϣ��ƺ�û��18��֮ǰ������������
    nTmpNum = CntaxGetInvTemplateValue(fpxx->hDev->abInvTemplate, 0, 1) * 2;
    strncpy(fpxx->fpdm, pDetailBuff + nIndex, nTmpNum);
    RemovePrefixTwoZero(fpxx->fpdm);  //רƱ10λ
    nIndex += nTmpNum;
    // fphm
    nTmpNum = CntaxGetInvTemplateValue(fpxx->hDev->abInvTemplate, 1, 1) * 2;
    strncpy(fpxx->fphm, pDetailBuff + nIndex, nTmpNum);
    nIndex += nTmpNum;
    if (FPSTATUS_BLANK_WASTE == fpxx->fpzt)
        goto DecodeSign;
    // yfpdm
    memset(szBuf, 0, sizeof(szBuf));
    nTmpNum = CntaxGetInvTemplateValue(fpxx->hDev->abInvTemplate, 0, 1) * 2;
    strncpy(szBuf, pDetailBuff + nIndex, nTmpNum);
    if (atoll(szBuf)) {
        strcpy(fpxx->blueFpdm, szBuf);
        RemovePrefixTwoZero(fpxx->blueFpdm);  //רƱ10λ
    }
    nIndex += nTmpNum;
    // yfphm
    memset(szBuf, 0, sizeof(szBuf));
    nTmpNum = CntaxGetInvTemplateValue(fpxx->hDev->abInvTemplate, 1, 1) * 2;
    strncpy(szBuf, pDetailBuff + nIndex, nTmpNum);
    if (atoll(szBuf))
        strcpy(fpxx->blueFphm, szBuf);
    nIndex += nTmpNum;
    // nExtraBufSize
    if (nExtraBufSize != 0) {
        int nSizeTmp = GetNBytesSize(pDetailBuff + nIndex, 2);
        nIndex += 64 * nSizeTmp + 4;
    }
    if (fpxx->nVerInvoice == 2) {
        nChildRet = RestorClearBuf((uint8 *)pDetailBuff, nIndex, &bUtf8Buf);
        if (nChildRet < 0)
            return -2;
        fpxx->utf8Invoice = bUtf8Buf;
        nMWIndex = nIndex + 10 + nChildRet;
        nIndex += 10;
    }
    // rednum
    nIndex = FormatTemplate(bUtf8Buf, pDetailBuff, nIndex, fpxx->redNum, sizeof fpxx->redNum);
    // kpjh
    nIndex = FormatTemplate(bUtf8Buf, pDetailBuff, nIndex, szBuf, sizeof szBuf);
    fpxx->kpjh = atoi(szBuf);
    // tspz
    nIndex = FormatTemplate(bUtf8Buf, pDetailBuff, nIndex, szBuf, sizeof szBuf);
    Zyfplx2String(true, szBuf, &fpxx->zyfpLx);
    // gfmc
    nIndex = FormatTemplate(bUtf8Buf, pDetailBuff, nIndex, fpxx->gfmc, sizeof fpxx->gfmc);
    // gfdzdh
    nIndex = FormatTemplate(bUtf8Buf, pDetailBuff, nIndex, fpxx->gfdzdh, sizeof fpxx->gfdzdh);
    // gfyhzh
    nIndex = FormatTemplate(bUtf8Buf, pDetailBuff, nIndex, fpxx->gfyhzh, sizeof fpxx->gfyhzh);
    // xfmc
    nIndex = FormatTemplate(bUtf8Buf, pDetailBuff, nIndex, fpxx->xfmc, sizeof fpxx->xfmc);
    // xfdzdh
    nIndex = FormatTemplate(bUtf8Buf, pDetailBuff, nIndex, fpxx->xfdzdh, sizeof fpxx->xfdzdh);
    // xfyhzh
    nIndex = FormatTemplate(bUtf8Buf, pDetailBuff, nIndex, fpxx->xfyhzh, sizeof fpxx->xfyhzh);
    // unknow
    nIndex = FormatTemplate(bUtf8Buf, pDetailBuff, nIndex, szBuf, sizeof szBuf);
    // slv
    nIndex = FormatTemplate(bUtf8Buf, pDetailBuff, nIndex, fpxx->slv, sizeof fpxx->slv);
    if (!strcmp(fpxx->slv, "99.01"))
        fpxx->isMultiTax = true;
    // jshj
    nIndex = FormatTemplate(bUtf8Buf, pDetailBuff, nIndex, fpxx->jshj, sizeof fpxx->jshj);
    if (fpxx->isRed)
        MemoryFormart(MF_TRANSLATE_RED, fpxx->jshj, strlen(fpxx->jshj));
    // zyspmc
    memset(szBuf, 0, sizeof szBuf);
    nIndex = FormatTemplate(bUtf8Buf, pDetailBuff, nIndex, szBuf, sizeof szBuf);
    GetSpsmmcSpmc(szBuf, fpxx->zyspsmmc, fpxx->zyspmc);
    // spsm?
    nIndex = FormatTemplate(bUtf8Buf, pDetailBuff, nIndex, szBuf, sizeof szBuf);
    // xfmc2?
    nIndex = FormatTemplate(bUtf8Buf, pDetailBuff, nIndex, szBuf, sizeof szBuf);
    // swjgmc
    nIndex = FormatTemplate(bUtf8Buf, pDetailBuff, nIndex, szBuf, sizeof szBuf);
    // bz
    nIndex = FormatTemplate(bUtf8Buf, pDetailBuff, nIndex, fpxx->bz, sizeof fpxx->bz);
    DetectSpecialFplxFromBZ(fpxx);
    // kpr
    nIndex = FormatTemplate(bUtf8Buf, pDetailBuff, nIndex, fpxx->kpr, sizeof fpxx->kpr);
    // fhr
    nIndex = FormatTemplate(bUtf8Buf, pDetailBuff, nIndex, fpxx->fhr, sizeof fpxx->fhr);
    // skr
    nIndex = FormatTemplate(bUtf8Buf, pDetailBuff, nIndex, fpxx->skr, sizeof fpxx->skr);
    // qdbz
    nIndex = FormatTemplate(bUtf8Buf, pDetailBuff, nIndex, fpxx->qdbj, sizeof fpxx->qdbj);
    if (!strcasecmp(fpxx->qdbj, "1"))
        strcpy(fpxx->qdbj, "Y");
    else
        strcpy(fpxx->qdbj, "N");
    // ssyf
    nIndex = FormatTemplate(bUtf8Buf, pDetailBuff, nIndex, fpxx->ssyf, sizeof fpxx->ssyf);
    // bmbbbh
    nIndex = FormatTemplate(bUtf8Buf, pDetailBuff, nIndex, fpxx->bmbbbh, sizeof fpxx->bmbbbh);
    // hsslbs
    nIndex = FormatTemplate(bUtf8Buf, pDetailBuff, nIndex, fpxx->hsjbz, sizeof fpxx->hsjbz);
    //����ƽ̨���ã�˰�ְ�ʽ��ȡ ptlx(ƽ̨����)ptxh��ƽ̨��ţ� tqm����ȡ�룩
    nIndex = Decode2020YearBuf(fpxx, bUtf8Buf, pDetailBuff, nIndex);
    // fyxm
    nIndex += GetFYXMorQDXM(fpxx, bUtf8Buf, true, pDetailBuff + nIndex);
    // qdxm
    nIndex += GetFYXMorQDXM(fpxx, bUtf8Buf, false, pDetailBuff + nIndex);
    // qtxm
    nIndex += GetQTXM(bUtf8Buf, pDetailBuff + nIndex);
    // fix v2 mw's index;����������ܺ�Ŀո�
    //��Ϊ�Ǹ��õĽ���ǰ��buf(���ܺ�ıȽ���ǰ��С)�����Ե�����junkBuf
    if (fpxx->nVerInvoice >= 2)
        nIndex = nMWIndex;
    // mw
    strncpy(fpxx->mw, pDetailBuff + nIndex, 0x70);
    nIndex += 0x70;
    //printf_array(pDetailBuff, strlen(pDetailBuff));
DecodeSign:
    // sign
    strncpy(fpxx->sign, pDetailBuff + nIndex, nSignStrLen);
    MemoryFormart(MF_TRANSLATE_SPACE2ZERO, fpxx->sign, strlen(fpxx->sign));
    nIndex += nSignStrLen;
    //���ݴ���
    strcpy(fpxx->jqbh, fpxx->hDev->szDeviceID);
    return nIndex;
}

//���س���Ϊ��FP����ռ�ó���
int CntaxInvRaw2Fpxx(HDEV hDev, uint8 bBlockBufType, uint8 *pBlockBin, HFPXX pOutFpxx,
                     int *pnBlockBinLen)
{
    int nRet = 0xff;
    char *pInvoiceStr = NULL;
    char *pInvoiceDecodeBuf = NULL;
    //���ȵ�ת����ʽ��Ȼ��ȷ������Ҫ��һ������,++����
    uint32 nBlockBinLen = bswap_32(*(uint32 *)(pBlockBin + 3));
    if (nBlockBinLen > DEF_MAX_FPBUF_LEN)
        return -100;
    *pnBlockBinLen = nBlockBinLen + 7;
    int nMaxInvoiceLen = DEF_MAX_FPBUF_LEN * 2 + 2048;
    pInvoiceStr = (char *)calloc(1, nMaxInvoiceLen);
    if (!pInvoiceStr) {
        nRet = -2;
        goto Finish;
    }
    pInvoiceDecodeBuf = calloc(1, nBlockBinLen * 4 + DEF_MAX_FPBUF_LEN);
    if (!pInvoiceDecodeBuf) {
        nRet = -3;
        goto Finish;
    }
    Byte2Str(pInvoiceStr, pBlockBin, nBlockBinLen + 7);  // 7byte(3+4) +nBlockBinLen
    int nChildRet =
        CnTaxDecodeSingleInvoice(hDev->abInvTemplate, nBlockBinLen, pInvoiceStr,
                                 (char *)pInvoiceDecodeBuf, pOutFpxx->jym, pOutFpxx->mw);
    if (!nChildRet) {
        nRet = -4;
        goto Finish;
    }

    pOutFpxx->fplx = hDev->bOperateInvType;
    strcpy(pOutFpxx->xfmc, hDev->szCompanyName);
    //ԭʼ��Ʊ�����ڷ�Ʊ�ϴ�
    if (pOutFpxx->pRawFPBin)
        free(pOutFpxx->pRawFPBin);
    //�ڴ�ֲ�: RawFPBin-size(4bytes)+pRawFPBin(xxBytes)+fpUploadMW1(64bytes(ʵ��0x38=56�ֽ�))
    pOutFpxx->pRawFPBin = calloc(1, 4 + nChildRet + 64);
    if (pOutFpxx->pRawFPBin) {
        // RawFPBin-size
        memcpy(pOutFpxx->pRawFPBin, &nChildRet, 4);
        // RawFPBin
        memcpy(pOutFpxx->pRawFPBin + 4, pInvoiceDecodeBuf, nChildRet);
        // fpUploadMW1����һ���У�ֻ�ж�ȡδ�ϴ���Ʊʱ�ź������ݣ�����Ϊ0,pBlockBin��βʱ��������
        if (bBlockBufType == FPCX_COMMON_OFFLINE) {
            // abUploadSureMW10x1c ʵ�������������¸�ʽ
            // 00000000: 0000 15e2 0320 0210 0204 0380 0124 0000  ..... .......$..
            // 00000010: 15e2 0320 0210 0204 0380 0124 0000 0000  ... .......$....
            int nFpUploadMw1Index = *pnBlockBinLen;
            memcpy(pOutFpxx->pRawFPBin + 4 + nChildRet, pBlockBin + nFpUploadMw1Index, 0x38 / 2);
        } else {
            // abUploadSureMW10x1c,example����˲���ʹ��strlen(abUploadSureMW10x1c)���ж��Ƿ����ֵ
            // ֻ��ͨ��д���Զ�����ֵ���ж� "NoData!"�����ַ���Ϊ�Զ������ݣ���ʵ������
            strcpy((char *)pOutFpxx->pRawFPBin + 4 + nChildRet, "NoData!");
        }
    }
    _WriteHexToDebugFile("cntax-hex_my.txt", (uint8 *)pInvoiceDecodeBuf, nChildRet);
    if ((nChildRet = CntaxDecodeBuf2Fpxx((char *)pInvoiceDecodeBuf, pOutFpxx)) < 0) {
        _WriteLog(LL_INFO, "CntaxDecodeBuf2Fpxx nChildRet = %d", nChildRet);
        nRet = -5;
        goto Finish;
    }
    nRet = nBlockBinLen + 7;

Finish:
    if (pInvoiceStr)
        free(pInvoiceStr);
    if (pInvoiceDecodeBuf)
        free(pInvoiceDecodeBuf);

    return nRet;
}

// hDev->bOperateInvType���뱻������ȷֵ
int CntaxGetInvDetail(HDEV hDev, uint8 bBlockBufType, uint8 *pBuf, int nBufSize,
                      int (*pCallBack)(HFPXX))
{
    int nBlockIndex = 0, nRet = -1, nChildRet = -1, nInvNum = 0;
    while (nBlockIndex < nBufSize) {
        HFPXX fpxx = MallocFpxxLinkDev(hDev);
        if (!fpxx)
            break;
        uint8 *pBlockBin = pBuf + nBlockIndex;
        int nBlockBinLen = 0;
        nChildRet = CntaxInvRaw2Fpxx(hDev, bBlockBufType, pBlockBin, fpxx, &nBlockBinLen);
        nBlockIndex += nBlockBinLen;
        if (nChildRet < 0) {
            FreeFpxx(fpxx);
            if (nChildRet == -100)
                break;  // fatal
            else
                continue;  // try next
        }
#ifdef _CONSOLE
        bool bSaveAllOrSignError = true;
        if (bSaveAllOrSignError)
            SaveFpxxToDisk(fpxx);
        else {
            char *pSignGBKBuf = GetSignBase64(fpxx);
            if (pSignGBKBuf && strcmp(pSignGBKBuf, fpxx->sign)) {
                _WriteLog(LL_DEBUG,
                          "!!!Our fp's sign not match with orig, fplx:%d kpsj:%s fpdm:%s fphm:%s",
                          fpxx->fplx, fpxx->kpsj_standard, fpxx->fpdm, fpxx->fphm);
                SaveFpxxToDisk(fpxx);
            }
        }
        _WriteLog(LL_DEBUG,
                  "QD:%s TSPZ:%d IsUpload:%d FPLX:%d KPSJ:%s FPDM:%s FPHM:%s BZ:%s JYM:%s MW:%s",
                  fpxx->qdbj, fpxx->zyfpLx, fpxx->bIsUpload, fpxx->fplx, fpxx->kpsj_standard,
                  fpxx->fpdm, fpxx->fphm, fpxx->bz, fpxx->jym, fpxx->mw);
#endif
        if (pCallBack)  //���ŷ�Ʊһ�ż�����nBlockIndex>= nBufSize
            nChildRet = pCallBack(fpxx);
        FreeFpxx(fpxx);
        nInvNum++;
    }
    if (pCallBack)
        nRet = nChildRet;
    else
        nRet = nInvNum;
    return nRet;
}

//���������ӿ�(����:��Ʊ���ߺ���Ӧ)��Ҫ�ö�ȡ��ʽ����˷�����ýӿ�
int CntaxChunkRead(HUSB hUSB, uint8 **pRetBuff, int *nRetBuffLen)
{
    bool bReadFinish = false;
    int nRet = ERR_GENERIC;
    const int nBlockSize = 65536;  //����ʱ65536�����޸ģ�����IO����
    int nInvBufLen = nBlockSize, nIndex = 0, nChildRet = 0, i = 0;
    uint8 cbCmd[16] = "";
    *pRetBuff = NULL;
    *nRetBuffLen = 0;
    uint8 *pBuf = calloc(1, nInvBufLen);
    if (!pBuf) {
        nRet = ERR_BUF_ALLOC;
        return nRet;
    }
    //�ӵ�16λ0x01��ʼ fe 1f --- 02
    memcpy(cbCmd, "\xfe\x0e\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 16);
    for (i = 1; i < 256; i++) {  //�ٷ������У���ֵΪal��8λ���255�� 1-255 16*65535=16M ���
        cbCmd[15] = (uint8)i;
        if (i > 1)
            cbCmd[1] = 0x1f;
        // prepare buf
        if (nInvBufLen - nIndex < nBlockSize) {
            nInvBufLen += nBlockSize;
            char *p = realloc(pBuf, nInvBufLen);
            if (!p)
                break;
            pBuf = (uint8 *)p;
        }
        memcpy(pBuf + nIndex, cbCmd, 16);
        nChildRet = CntaxLogicIO(hUSB, pBuf + nIndex, 16, nBlockSize, 0, 0);
        if (nChildRet < 0) {
            //��ȡ������и�09d200�Ĵ�����Ϊ�޸��໺��ɶ�ȡ
            if (nIndex > 0)
                bReadFinish = true;
            else
                _WriteLog(LL_FATAL, "Fetch fpxx USB-IO failed");
            break;
        }
        nIndex += nChildRet;
    }
    if (bReadFinish) {
        *pRetBuff = pBuf;
        *nRetBuffLen = nInvBufLen;
        nRet = nIndex;
    } else {
        nRet = ERR_IO_FAIL;
        free(pBuf);
    }
    return nRet;
}

// Ukey��֧��ָ����С��ȡ��ֻ��chunk��ȡ.�ٷ�ÿ��IO65k�����Ʊ��ȡ����16M������ֵ��Ʊ����IO��ֵ
// FPCX_COMMON_OFFLINE_NUMʱ��ҪszDriverVersion��������ʽ������NULL����
int CntaxGetInvBuff(HUSB hUSB, uint8 bOPType, uint8 **pRetBuff, int *nRetBuffLen,
                    uint8 *abQueryInput, int nQueryInputSize, char *szDriverVersion)
{
    int nRet = ERR_GENERIC;
    bool bSimpleIO = false;  //ֻ����һ��command���ɶ�ȡ�����ݣ��������chunk��ȡ
    uint8 cb[512];
    uint8 cbCmd[16] = "";
    int nChildRet = 0;
    // 00 01 0x10 0x21 �ĸ�����Ϊһ���ӿ����ӽӿڣ�02Ϊ��һ���ӽӿ�
    switch (bOPType) {
        case FPCX_COMMON_MONTH:  // �¶Ȳ�ѯ��Ʊ�ָ�
            memcpy(cbCmd, "\xfe\x17\x01", 3);
            //<business comment="��Ʊ��ѯ" id="FPCX">
            //<fplxdm>007</fplxdm><cxfs>1</cxfs>
            //<cxtj>2022070120220722</cxtj><cxlx>0</cxlx>
            break;
        case FPCX_COMMON_NUMBER:  // ָ����Ʊ��������ѯ
            memcpy(cbCmd, "\xfe\x17\x00", 3);
            break;
        case FPCX_COMMON_OFFLINE:  // ��ѯδ�ϴ���Ʊ(�ٷ�����ʱѭ�����)
            memcpy(cbCmd, "\xfe\x17\x21", 3);
            //\xfe\x17\x21������xml����
            //<business comment="��Ʊ�ϴ�" id="FPSC">
            //<fplxdm>026</fplxdm><fpzs>20</fpzs><czlx>0</czlx>
            //<fplxdm>007</fplxdm><fpzs>20</fpzs><czlx>0</czlx>
            break;
        case FPCX_COMMON_MODIFY:  //���ϴ��б��ѯ(�ٷ�����ʱѭ�����)
            bSimpleIO = true;
            memcpy(cbCmd, "\xfe\x17\x30", 3);  //\x30|\x31|\x32
            // \xfe\x17\x30������xml������czlxΪ1ʱ����Ϊ\xfe\x17\x31,czlxΪ2ʱ����Ϊ\xfe\x17\x32,
            //<business comment="���ϴ��б��ѯ" id="YSCLBCX">
            //<fplxdm>007</fplxdm><czlx>0</czlx></input>
            break;
        case FPCX_COMMON_OFFLINE_NUM:
            // δ�ϴ���Ʊ������ѯ��������δ�ϴ�������Ʊ������modify��Ʊ���������У���(�ٷ�����ʱѭ�����)
            bSimpleIO = true;
            if (!szDriverVersion)
                return ERR_PARM_NULLPTR;
            if (!strcmp(szDriverVersion, "00020001220225"))
                memcpy(cbCmd, "\xfe\x17\x20", 3);  //������Ϊ10�ֽ�?
            else
                memcpy(cbCmd, "\xfe\x17\x33", 3);
            //�������������������Ϊ4�ֽ�,0x00 0x00 0x00 0x01
            // \xfe\x17\x33������xml����
            //<business comment="��Ʊ��ѯ" id="FPCX">
            //<fplxdm>026</fplxdm><cxfs>2</cxfs><cxtj>0</cxtj><cxlx>0</cxlx>
            //<business comment="��Ʊ�ϴ�" id="FPSC">
            //<fplxdm>007</fplxdm><fpzs/><czlx>1</czlx>
            break;
        case FPCX_COMMON_LASTINV:
            memcpy(cbCmd, "\xfe\x17\x02", 3);  //��ȡ�ѿ��ߵ����һ�ŷ�Ʊ����
            break;
        default:
            // unknow
            // memcpy(cbCmd, "\xfe\x17\x10", 3);
            // memcpy(cbCmd, "\xfe\x17\x40", 3);
            // memcpy(cbCmd, "\xfe\x17\x41", 3);
            // memcpy(cbCmd, "\xfe\x17\x34", 3); ��Ҫ�������
            _WriteLog(LL_DEBUG, "GetInvBuff, unsupport optype");
            return -1;
            break;
    }
    //�Ȼ�ȡ�����Ĳ�����Ϣ,�����Ӧ�ڴ�
    memcpy(cb, cbCmd, 3);
    if ((nChildRet = CntaxLogicIO(hUSB, cb, 3, sizeof(cb), abQueryInput, nQueryInputSize)) < 0)
        return ERR_IO_FAIL;
    if (bSimpleIO) {
        uint8 *pBuf = NULL;
        calloc(1, nChildRet);
        if (!(pBuf = calloc(1, nChildRet)))
            return ERR_BUF_ALLOC;
        memcpy(pBuf, cb, nChildRet);
        *pRetBuff = pBuf;
        *nRetBuffLen = nChildRet;
        return nChildRet;
    }
    nRet = CntaxChunkRead(hUSB, pRetBuff, nRetBuffLen);
    return nRet;
}

// QueryCountΪbegin֮���ѯ������������Ϊ1
int TransferFPDMAndFPHMToBytes(char *szInFPDM, char *szInFPHMBegin, int nQueryCount, uint8 *pOutBuf)
{
    if (!nQueryCount)
        nQueryCount = 1;
    char szBuf[36] = "";
    snprintf(szBuf, 36, "%12s%8s%08d", szInFPDM, szInFPHMBegin, nQueryCount);
    MemoryFormart(MF_TRANSLATE_SPACE2STRZERO, szBuf, strlen(szBuf));
    return Str2Byte(pOutBuf, szBuf, strlen(szBuf));
}

//����FPDM FPHM�����Ͳ�ѯFPXX
int CntaxQueryInvInfo(HDEV hDev, uint8 bInvType, char *szFPDM, char *szFPHM, HFPXX pOutFpxx)
{
    uint8 *pRetBuff = NULL;
    int nRetBuffLen = 0, nRet = -3, nChildRet = 0;
    int nInvNum = -1;
    if (strlen(szFPDM) < 8 || strlen(szFPDM) < 8)
        return -1;
    uint8 abParamBuf[24] = {0};
    if (TransferFPDMAndFPHMToBytes(szFPDM, szFPHM, 1, abParamBuf) != 14)
        return -2;
    if (CntaxEntry(hDev->hUSB, NOP_OPEN_FULL) < 0)
        return -3;
    if (CntaxSelectInvoiceType(hDev, bInvType, SELECT_OPTION_TEMPLATE) < 0) {
        if (CntaxEntry(hDev->hUSB, NOP_CLOSE) < 0)
            return -4;
        return -5;
    }
    nInvNum = CntaxGetInvBuff(hDev->hUSB, FPCX_COMMON_NUMBER, &pRetBuff, &nRetBuffLen, abParamBuf,
                              14, NULL);
    if (nInvNum <= 0) {
        _WriteLog(LL_WARN, "No invoice found, fpdm:%s fphm:%s", szFPDM, szFPHM);
        return -4;
    }
    _WriteHexToDebugFile("cntax_inv_raw.bin", pRetBuff, nRetBuffLen);
    while (1) {
        if ((nRet = CntaxInvRaw2Fpxx(hDev, FPCX_COMMON_NUMBER, pRetBuff, pOutFpxx, &nChildRet)) < 0)
            break;
        nRet = 0;
        break;
    }
    if (pRetBuff)
        free(pRetBuff);
    if (CntaxEntry(hDev->hUSB, NOP_CLOSE) < 0)
        return -6;
    return nRet;
}

int CntaxBuildWasteInvBuf(HFPXX fpxx, uint8 *pBuff)
{
    int nIndex = 0;
    char szHexBuf[512] = "", szBuf[256] = "";
    HDEV hDev = fpxx->hDev;
    if (!strlen(fpxx->je)) {  //��Ҫ����200.34�����������
        _WriteLog(LL_FATAL, "ZFFP must has jshj");
        return -1;
    }
    if (!strlen(fpxx->kpr)) {  // kpr����zfr,����ټ���һ������
        _WriteLog(LL_WARN, "kpr incorrect, 5word");
        return -2;
    }
    strncpy(szHexBuf, hDev->szDeviceTime, 8);
    GetFotmatJEStr(fpxx->je, szHexBuf + 8);
    FillTwocharRepeat(szHexBuf + 20, "20", 20);
    nIndex = 20;
    //!!!ע�⣬20220429�汾�󣬿�Ʊͳһ��ΪUTF8��ʽ������202206�·ݲ���ʱ�����������Ȼ��gbkģʽ��
    //������ڵ���Ϊutf8ģʽʱ������strncpy��Ҫ��Ҫ�ٶ�ƫ��2���ֽڣ���2���ֽڸ�Ϊ4�ֽ�
    memset(szBuf, 0, sizeof szBuf);
    StrToNisecTemplate(false, szBuf, fpxx->kpr, strlen(fpxx->kpr));
    strncpy(szHexBuf + nIndex, szBuf + 2, strlen(szBuf + 2));
    nIndex += 40;

    sprintf(szBuf, "%012llu", atoll(fpxx->fpdm));
    strcat(szHexBuf, szBuf);
    strcat(szHexBuf, fpxx->fphm);
    strcat(szHexBuf, "0200");  // unknow(��Ϊ���ĳ���)
    nIndex = Str2Byte(pBuff, szHexBuf, strlen(szHexBuf));
    _WriteHexToDebugFile("kpstring_cntax_wasteinvoice_my.bin", (uint8 *)pBuff, nIndex);
    return nIndex;
}

int CntaxWasteInvoiceLoopIO(HFPXX fpxx, int nCount)
{
    int i = 0, nChildRet = 0, nRet = ERR_GENERIC;
    uint8 szOut[512] = "", cb[256];
    for (i = 0; i < nCount; i++) {
        if ((nChildRet = CntaxBuildWasteInvBuf(fpxx, szOut)) < 0) {
            nRet = ERR_LOGIC_BUILD_FORMATBUF;
            break;
        }
        memcpy(cb, "\xfe\x16", 2);
#ifndef _NODEVICE_TEST
        if (CntaxLogicIO(fpxx->hDev->hUSB, cb, 2, sizeof(cb), szOut, nChildRet) != 0) {
            nRet = fpxx->hDev->hUSB->nLastErrorCode;
            break;
        }
#endif
        if (i != nCount - 1)
            StringNumIncrement(fpxx->fphm);
        nRet = RET_SUCCESS;
    }
    return nRet;
}

int CntaxWasteInvoiceInner(HFPXX fpxx, int nCount)
{
    HDEV hDev = fpxx->hDev;
    int nRet = ERR_GENERIC;
    if (CntaxEntry(hDev->hUSB, NOP_OPEN_FULL) < 0)
        return ERR_DEVICE_OPEN;
    while (1) {
        if (CntaxSelectInvoiceType(hDev, fpxx->fplx, SELECT_OPTION_TEMPLATE) < 0)
            break;
        //�������Ҫʵʱʱ�䣬�����þ�̬ʱ��
        if (CntaxGetDeviceTime(hDev->hUSB, hDev->szDeviceTime) < 0)
            break;
        //û������˰�ŵĵ����հ�����(�ײ��豸�Զ���д����˰�ź�����)
        if (strlen(fpxx->xfsh)) {  // stand waste
            if (TimeOutMonthDeadline(fpxx->kpsj_standard, hDev->szDeviceTime)) {
                _WriteLog(
                    LL_FATAL,
                    "Current device time is next month of the waste-invoice's month,DevTime:%s "
                    "InvTime:%s",
                    hDev->szDeviceTime, fpxx->kpsj_standard);
                SetLastError(fpxx->hDev->hUSB, ERR_LOGIC_WASTETIME_OVERFLOW,
                             "�������Ͻ�ֹ���ڲ���������");
                break;
            }
        } else {
            if (CommonGetCurrentInvCode(hDev, fpxx->fpdm, fpxx->fphm, fpxx->fpendhm)) {
                nRet = SetLastError(hDev->hUSB, ERR_PARM_CHECK,
                                    "��Ʊ�������ָ�����󣬱����Ƿ�Ʊ��ǰͷ���������");
                break;
            }
            if (!CheckFphmRangeIncorrect(fpxx->fphm, fpxx->fpendhm, nCount)) {
                nRet = SetLastError(fpxx->hDev->hUSB, ERR_PARM_CHECK,
                                    "��Ʊ������뷶Χ���󣬳�����Ʊ��Χ");
                break;
            }
            strcpy(fpxx->je, "0");
        }
        if ((nRet = CntaxWasteInvoiceLoopIO(fpxx, nCount)) < 0)
            break;
        nRet = RET_SUCCESS;
        break;
    }
    CntaxEntry(hDev->hUSB, NOP_CLOSE);
    return nRet;
}

//���Ϸ�Ʊ-ָ����Ʊ�������/��ָ����Ʊ�������,ֻ�����϶�����Ʊ��˳�����
int CntaxWasteInvoice(HDEV hDev, uint8 bInvType, bool bBlankWaste, char *szFPDM, char *szFPHM,
                      char *zfr, int nCount,int *need_restart)
{
    if ((FPLX_COMMON_DZFP == bInvType) || (FPLX_COMMON_DZZP == bInvType)) {
        return SetLastError(hDev->hUSB, ERR_LOGIC_DZFP_CANNOT_WASTE,
                            "��Ʊ���ʹ���������Ϸ�Ʊֻ��Ϊֽר��ֽ��");
    }
    HFPXX fpxx = MallocFpxxLinkDev(hDev);
    int nRet = ERR_GENERIC;

    int nChildRet = -1;
    while (1) {
        fpxx->fplx = bInvType;
        if (!bBlankWaste) {  //�ǿհ����ϣ���Ҫ��ѯ��ԭ��Ʊ
            if (nCount > 1) {
                nRet = SetLastError(hDev->hUSB, ERR_PARM_CHECK, "��������һ��ֻ��������һ��");
                break;
            }
            _WriteLog(LL_INFO, "���Ϸ�Ʊ-ָ����Ʊ��������ѯ��Ʊ\n");
            if ((nChildRet = CntaxQueryInvInfo(hDev, bInvType, szFPDM, szFPHM, fpxx)) < 0) {
                nRet =
                    SetLastError(hDev->hUSB, ERR_PARM_CHECK, "��Ʊ��������ѯ��Ʊʧ��,�޷�����");
                break;
            }
            _WriteLog(LL_INFO, "���Ϸ�Ʊ-ָ����Ʊ��������ѯ��Ʊ�ɹ�\n");
            if (fpxx->zfbz) {
                nRet = SetLastError(hDev->hUSB, ERR_PARM_CHECK,
                                    "�÷�Ʊ��������Ѿ����Ϲ�,�޷��ظ�����");
                break;
            }
            if (fpxx->fplx != bInvType) {
                nRet = SetLastError(hDev->hUSB, ERR_PARM_CHECK,
                                    "����ķ�Ʊ������ʵ�ʿ��ߵķ�Ʊ���Ͳ���");
                break;
            }

			if (strcmp(fpxx->zyspsmmc, "*������*") == 0)
			{
				nRet = SetLastError(hDev->hUSB, ERR_PARM_CHECK,
					"��������Ʊ����������");
				_WriteLog(LL_DEBUG, "%s", (char *)hDev->hUSB->errinfo);
				nRet = -1;
				break;
			}
			struct Spxx *stp_Spxx = NULL;
			int i;
			stp_Spxx = fpxx->stp_MxxxHead->stp_next;
			//_WriteLog(LL_FATAL, "��Ʒ����=%d\n", Spxx_num);
			for (i = 0; i < fpxx->spsl; i++)
			{
				//_WriteLog(LL_FATAL,"�����%d����Ʒ\n",i+1);
				if (stp_Spxx == NULL)
				{
					break;
				}
				if (memcmp(stp_Spxx->spbh, "10701010", strlen("10701010")) == 0) {
					nRet = SetLastError(hDev->hUSB, ERR_PARM_CHECK,
						"���߳�Ʒ�ͷ�Ʊ�ݲ�֧������");
					_WriteLog(LL_DEBUG, "%s", (char *)hDev->hUSB->errinfo);
					FreeFpxx(fpxx);
					return -1;
				}
				if ((memcmp(stp_Spxx->spbh, "1090305", strlen("1090305")) == 0) ||
					(memcmp(stp_Spxx->spbh, "1090306", strlen("1090306")) == 0) ||
					(memcmp(stp_Spxx->spbh, "1090307", strlen("1090307")) == 0) ||
					(memcmp(stp_Spxx->spbh, "1090309", strlen("1090309")) == 0) ||
					(memcmp(stp_Spxx->spbh, "1090312", strlen("1090312")) == 0) ||
					(memcmp(stp_Spxx->spbh, "1090315", strlen("1090315")) == 0)) {
					nRet = SetLastError(hDev->hUSB, ERR_PARM_CHECK,
						"��������Ʊ����������");
					_WriteLog(LL_DEBUG, "%s", (char *)hDev->hUSB->errinfo);
					FreeFpxx(fpxx);
					return -2;
				}
                if ((memcmp(stp_Spxx->spbh, "103040501", strlen("103040501")) == 0) ||
                    (memcmp(stp_Spxx->spbh, "103040502", strlen("103040502")) == 0) ||
                    (memcmp(stp_Spxx->spbh, "103040503", strlen("103040503")) == 0) ||
                    (memcmp(stp_Spxx->spbh, "103040504", strlen("103040504")) == 0)) {
					nRet = SetLastError(hDev->hUSB, ERR_PARM_CHECK,
						"�����̷�Ʊ����������");
					_WriteLog(LL_DEBUG, "%s", (char *)hDev->hUSB->errinfo);
					FreeFpxx(fpxx);
					return -3;
				}
                //wang 02 01 ϡ�����벢����Ʒ
                if ((memcmp(stp_Spxx->spbh, "102", strlen("102")) == 0)) {
                    nRet = SetLastError(hDev->hUSB, ERR_PARM_CHECK,
						"���Ʒ��Ʊ����������");
					_WriteLog(LL_DEBUG, "%s", (char *)hDev->hUSB->errinfo);
					FreeFpxx(fpxx);
					return -4;
                }
				stp_Spxx = stp_Spxx->stp_next;
			}

        } else {
            strcpy(fpxx->fpdm, szFPDM);
            strcpy(fpxx->fphm, szFPHM);
            strcpy(fpxx->kpr, zfr);
            //wang ��Ʊ�����һ��
            if (strcmp(fpxx->fphm, fpxx->fpendhm) == 0) {
                _WriteLog(LL_WARN, "��ʹ�õ���Ʊ�����һ�ŷ�Ʊ,������ɺ���Ҫ���жϵ����");
                fpxx->need_restart = 1;
                *need_restart = 1;
            }
        }
        if ((nRet = CntaxWasteInvoiceInner(fpxx, nCount)) < 0)
            break;
        nRet = RET_SUCCESS;
        break;
    }
    if (!nRet)
        _WriteLog(LL_INFO, "Waste invoice successful: FPDM:%s FPHM:%s", fpxx->fpdm, fpxx->fphm);
    else
        _WriteLog(LL_FATAL, "Waste invoice failed");
    FreeFpxx(fpxx);
    return nRet;
}

int CntaxGetTailInvVersion(HDEV hDev, char *szOutVersion)
{
    HUSB hUSB = hDev->hUSB;
    int nRet = ERR_GENERIC, nChildRet = 0;
    uint8 cb[256];
    char szBuf[64] = "";
    char abVersion[3][768];
    if (CntaxEntry(hUSB, NOP_OPEN_FULL) < 0)
        return SetLastError(hUSB, ERR_DEVICE_OPEN, "OpenDevice failed");
    do {
        memcpy(cb, "\xfe\x26", 2);
        sprintf(szBuf, "%sff%s", hDev->szDeviceID, "0100000000");
        Str2Byte(cb + 2, szBuf, strlen(szBuf));
        nChildRet = CntaxLogicIO(hUSB, cb, 15, sizeof(cb), NULL, 0);
        if (nChildRet < 20 || nChildRet > 64)
            break;
        cb[nChildRet] = '\0';
        memset(szBuf, 0, sizeof(szBuf));
        Base64_Decode((const char *)cb, nChildRet, szBuf);
        // V1.0.17_ZS_20220429,40.0,1
        if (strlen(szBuf) < 20 || !strchr(szBuf, '.') || !strchr(szBuf, ','))
            break;
        if (GetSplitStringSimple(szBuf, ",", abVersion, 3) != 3)
            break;
        strcpy(szOutVersion, abVersion[0]);
        nRet = RET_SUCCESS;
    } while (false);
    if (CntaxEntry(hUSB, NOP_CLOSE) < 0)
        return ERR_DEVICE_CLOSE;
    return nRet;
}
