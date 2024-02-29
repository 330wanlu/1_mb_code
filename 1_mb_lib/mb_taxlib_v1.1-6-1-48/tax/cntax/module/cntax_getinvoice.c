/*****************************************************************************
File name:   cntax_getinvoice.c
Description: cntax的发票获取相关代码
Author:      Zako
Version:     1.0
Date:        2021.02
History:
20210205     最初代码实现
*****************************************************************************/
#include "cntax_getinvoice.h"

uint16 CopySignDataBuf(char *pOrigBuffCopy, int nOrigBufIndex, char *pDetailBuff,
                       int nDetailBufIndex)
{
    //官方的做法是开完票后立即利用数据库数据进行签名，刷新签名，写入签名后应该不存在签名字段乱码情况
    //我方无法实现开票软件效果，只能采用固定签名长度匹配方式进行
    //签名长度获取,部分盘存在尚未签名时签名字段数据尚未初始化，读取到乱码随机数情况，目前妥协的办法只能通过强制刷新静态值的方法
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
    //如果有其他签名长度，再添加else if

    //更新新的签名长度
    sprintf(szBuf, "%04x", usSignSize);
    strncpy(pDetailBuff + 0x11a, szBuf, 4);
    //拷贝或初始化Sign签名区
    if (bHasInitSignData) {
        Str2Byte((uint8 *)pDetailBuff + nDetailBufIndex, pOrigBuffCopy + nOrigBufIndex + 4,
                 usSignSize * 2);
    } else
        memset((uint8 *)pDetailBuff + nDetailBufIndex, 0, usSignSize * 2);
    return usSignSize;
}

//会改变pOrigBuffCopy缓冲区内容
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
        //这里要对缓冲区进行简单变换，还要取出基本要素来计算密文和校验码,这些要素的偏移确认为固定偏移
        //!!!注意，这里的几个要素结尾都是以空格0x20填充并且带入计算（和nisec略不一样）
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
        // mw'size,pDetailBuff的新签名长度需要修正，在最后修改
        memset(szBuf, 0, sizeof(szBuf));
        strncpy(szBuf, pOrigBuffCopy + 0x11a, 4);  // 0x116?^
        int nSignStrLen = strtol(szBuf, NULL, 16);
        if (nSignStrLen <= 0 || nSignStrLen > nBlockBinLen)
            break;
        // mw's offset
        nMWIndex = 2 * (nBlockBinLen + 0 - (usMWBytesSize + nSignStrLen)) + 0xE;
        if (nMWIndex < 0)
            break;
        //真实内存拷贝开始，将USB数据变换后拷贝入新内存,到此处为止正式开始拷贝变换内存，所有内存偏移对齐一致
        memcpy(pDetailBuff, pOrigBuffCopy, nMWIndex);
        nDetailBufIndex = nMWIndex;
        nOrigBufIndex = nMWIndex;
        if (FPSTATUS_BLANK_WASTE != bInvoiceStatusFlag) {
            //密文
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
        mystrupr(pDetailBuff);  //如上全转大写
        //拷贝并修复签名字段数据
        usSignSize = CopySignDataBuf(pOrigBuffCopy, nOrigBufIndex, pDetailBuff, nDetailBufIndex);
        nOrigBufIndex += 4 + usSignSize * 2;  // 多增加4字节为'0164'字符串长度
        nDetailBufIndex += usSignSize;
        //头部长度修正，更新Block+3处长度信息,理论上不会出现奇数
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
        return -1;  //分组算法，总长度需要被8整除
    int nBufSize = nEncryptStringSize + 64;
    do {
        pDecBuf = (uint8 *)calloc(1, nBufSize);
        pDecBuf2 = (uint8 *)calloc(1, nBufSize);
        if (!pDecBuf || !pDecBuf2)
            break;
        // 10=8+2，固定长度
        // 8字节+2字节(string:0c,不知道干嘛的)
        uint8 *pNewBufAddr = pDetailBuff + nIndex + 10;
        memcpy(pDecBuf2, pNewBufAddr, nEncryptStringSize);
        nEncryptBytesSize = Str2Byte(pDecBuf, (char *)pDecBuf2, nEncryptStringSize);
        if (EncryptDecryptV2String(0, pDecBuf, nEncryptBytesSize, pDecBuf2) < 0)
            break;
        // nEncryptStringSize与解密后长度等同，直接拷贝回去,Byte2Str不会将分组最后的0x00转换成00，先手动置0
        memset(pNewBufAddr, 0, nEncryptStringSize);
        _WriteHexToDebugFile("CntaxV2-plaintext.my.bin", pDecBuf2, nEncryptBytesSize);
        if (IsUTF8WithBom((char *)pDecBuf2, nEncryptBytesSize)) {
            *pIsUtf8Buf = true;
            // jump UTF8bom 'efbbbf'
            nContentOffset = 3;
        }
        Byte2Str((char *)pNewBufAddr, pDecBuf2 + nContentOffset,
                 nEncryptBytesSize - nContentOffset);
        //这里只能原封不动的返回原有的加密缓冲大小，因为外部调用需要加上对应偏移
        nRet = nEncryptStringSize;
    } while (false);

    if (pDecBuf)
        free(pDecBuf);
    if (pDecBuf2)
        free(pDecBuf2);
    return nRet;
}

// nToRead不大于255
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
    // fpzt 包含upload、waste、redinv等
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
    //这里往后和nisec的不一样，nisec采用明文，cntax采用byte-string格式，长度变两倍
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
        strcpy(fpxx->gfsh, "000000000000000");  //无购方税号、空税号
    nIndex += 40;
    // xfsh2
    nIndex += 40;
    // 190字节处,是否红票
    if (!strncmp(pDetailBuff + nIndex, "01", 2)) {
        _WriteLog(LL_DEBUG, "CntaxDecodeBuf2Fpxx:red-invoice");
        fpxx->isRed = true;
    }
    nIndex += 2;
    // kpsj-官方代码固定在偏移192字节处读取，实际xfsh2累加完成时190个字节
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
    //#static index,此处自己累加也到索引258字节处，固定值也为258
    nIndex = 258;
    // devid
    nIndex += 12;
    // ykfsje 已开负数金额, ignore
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
    //已确认，官方代码中发票代码号码直接以发票模板确认，似乎没有18年之前长度遗留问题
    nTmpNum = CntaxGetInvTemplateValue(fpxx->hDev->abInvTemplate, 0, 1) * 2;
    strncpy(fpxx->fpdm, pDetailBuff + nIndex, nTmpNum);
    RemovePrefixTwoZero(fpxx->fpdm);  //专票10位
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
        RemovePrefixTwoZero(fpxx->blueFpdm);  //专票10位
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
    //公共平台配置，税局版式提取 ptlx(平台类型)ptxh（平台序号） tqm（提取码）
    nIndex = Decode2020YearBuf(fpxx, bUtf8Buf, pDetailBuff, nIndex);
    // fyxm
    nIndex += GetFYXMorQDXM(fpxx, bUtf8Buf, true, pDetailBuff + nIndex);
    // qdxm
    nIndex += GetFYXMorQDXM(fpxx, bUtf8Buf, false, pDetailBuff + nIndex);
    // qtxm
    nIndex += GetQTXM(bUtf8Buf, pDetailBuff + nIndex);
    // fix v2 mw's index;调整分组解密后的空格，
    //因为是复用的解密前的buf(解密后的比解密前的小)，所以得跳过junkBuf
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
    //兼容代码
    strcpy(fpxx->jqbh, fpxx->hDev->szDeviceID);
    return nIndex;
}

//返回长度为该FP单张占用长度
int CntaxInvRaw2Fpxx(HDEV hDev, uint8 bBlockBufType, uint8 *pBlockBin, HFPXX pOutFpxx,
                     int *pnBlockBinLen)
{
    int nRet = 0xff;
    char *pInvoiceStr = NULL;
    char *pInvoiceDecodeBuf = NULL;
    //长度的转换方式任然不确定，需要进一步测试,++待办
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
    //原始发票，用于发票上传
    if (pOutFpxx->pRawFPBin)
        free(pOutFpxx->pRawFPBin);
    //内存分布: RawFPBin-size(4bytes)+pRawFPBin(xxBytes)+fpUploadMW1(64bytes(实际0x38=56字节))
    pOutFpxx->pRawFPBin = calloc(1, 4 + nChildRet + 64);
    if (pOutFpxx->pRawFPBin) {
        // RawFPBin-size
        memcpy(pOutFpxx->pRawFPBin, &nChildRet, 4);
        // RawFPBin
        memcpy(pOutFpxx->pRawFPBin + 4, pInvoiceDecodeBuf, nChildRet);
        // fpUploadMW1，不一定有，只有读取未上传发票时才含有数据，否则为0,pBlockBin结尾时附带数据
        if (bBlockBufType == FPCX_COMMON_OFFLINE) {
            // abUploadSureMW10x1c 实际数据类型如下格式
            // 00000000: 0000 15e2 0320 0210 0204 0380 0124 0000  ..... .......$..
            // 00000010: 15e2 0320 0210 0204 0380 0124 0000 0000  ... .......$....
            int nFpUploadMw1Index = *pnBlockBinLen;
            memcpy(pOutFpxx->pRawFPBin + 4 + nChildRet, pBlockBin + nFpUploadMw1Index, 0x38 / 2);
        } else {
            // abUploadSureMW10x1c,example，因此不能使用strlen(abUploadSureMW10x1c)来判断是否存在值
            // 只能通过写入自定义数值来判断 "NoData!"，该字符串为自定义数据，无实际意义
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

// hDev->bOperateInvType必须被设置正确值
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
        if (pCallBack)  //单张发票一张即满足nBlockIndex>= nBufSize
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

//后续其他接口(例如:发票开具后响应)需要该读取方式，因此分离出该接口
int CntaxChunkRead(HUSB hUSB, uint8 **pRetBuff, int *nRetBuffLen)
{
    bool bReadFinish = false;
    int nRet = ERR_GENERIC;
    const int nBlockSize = 65536;  //必须时65536不能修改，否则IO出错
    int nInvBufLen = nBlockSize, nIndex = 0, nChildRet = 0, i = 0;
    uint8 cbCmd[16] = "";
    *pRetBuff = NULL;
    *nRetBuffLen = 0;
    uint8 *pBuf = calloc(1, nInvBufLen);
    if (!pBuf) {
        nRet = ERR_BUF_ALLOC;
        return nRet;
    }
    //从第16位0x01开始 fe 1f --- 02
    memcpy(cbCmd, "\xfe\x0e\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 16);
    for (i = 1; i < 256; i++) {  //官方代码中，数值为al，8位最多255。 1-255 16*65535=16M 最大
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
            //读取到最后有个09d200的错误，意为无更多缓存可读取
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

// Ukey不支持指定大小读取，只能chunk读取.官方每次IO65k，最大发票读取容量16M，返回值发票内容IO数值
// FPCX_COMMON_OFFLINE_NUM时需要szDriverVersion，其他方式调用填NULL即可
int CntaxGetInvBuff(HUSB hUSB, uint8 bOPType, uint8 **pRetBuff, int *nRetBuffLen,
                    uint8 *abQueryInput, int nQueryInputSize, char *szDriverVersion)
{
    int nRet = ERR_GENERIC;
    bool bSimpleIO = false;  //只发送一个command即可读取到数据，无需后续chunk读取
    uint8 cb[512];
    uint8 cbCmd[16] = "";
    int nChildRet = 0;
    // 00 01 0x10 0x21 四个参数为一个接口下子接口，02为另一个子接口
    switch (bOPType) {
        case FPCX_COMMON_MONTH:  // 月度查询发票恢复
            memcpy(cbCmd, "\xfe\x17\x01", 3);
            //<business comment="发票查询" id="FPCX">
            //<fplxdm>007</fplxdm><cxfs>1</cxfs>
            //<cxtj>2022070120220722</cxtj><cxlx>0</cxlx>
            break;
        case FPCX_COMMON_NUMBER:  // 指定发票代码号码查询
            memcpy(cbCmd, "\xfe\x17\x00", 3);
            break;
        case FPCX_COMMON_OFFLINE:  // 查询未上传发票(官方空闲时循环检测)
            memcpy(cbCmd, "\xfe\x17\x21", 3);
            //\xfe\x17\x21由如下xml触发
            //<business comment="发票上传" id="FPSC">
            //<fplxdm>026</fplxdm><fpzs>20</fpzs><czlx>0</czlx>
            //<fplxdm>007</fplxdm><fpzs>20</fpzs><czlx>0</czlx>
            break;
        case FPCX_COMMON_MODIFY:  //已上传列表查询(官方空闲时循环检测)
            bSimpleIO = true;
            memcpy(cbCmd, "\xfe\x17\x30", 3);  //\x30|\x31|\x32
            // \xfe\x17\x30由如下xml触发，czlx为1时命令为\xfe\x17\x31,czlx为2时命令为\xfe\x17\x32,
            //<business comment="已上传列表查询" id="YSCLBCX">
            //<fplxdm>007</fplxdm><czlx>0</czlx></input>
            break;
        case FPCX_COMMON_OFFLINE_NUM:
            // 未上传发票张数查询（包含尚未上传的离线票张数（modify发票不包含其中））(官方空闲时循环检测)
            bSimpleIO = true;
            if (!szDriverVersion)
                return ERR_PARM_NULLPTR;
            if (!strcmp(szDriverVersion, "00020001220225"))
                memcpy(cbCmd, "\xfe\x17\x20", 3);  //返回疑为10字节?
            else
                memcpy(cbCmd, "\xfe\x17\x33", 3);
            //无输入参数，返回内容为4字节,0x00 0x00 0x00 0x01
            // \xfe\x17\x33由如下xml触发
            //<business comment="发票查询" id="FPCX">
            //<fplxdm>026</fplxdm><cxfs>2</cxfs><cxtj>0</cxtj><cxlx>0</cxlx>
            //<business comment="发票上传" id="FPSC">
            //<fplxdm>007</fplxdm><fpzs/><czlx>1</czlx>
            break;
        case FPCX_COMMON_LASTINV:
            memcpy(cbCmd, "\xfe\x17\x02", 3);  //获取已开具的最后一张发票内容
            break;
        default:
            // unknow
            // memcpy(cbCmd, "\xfe\x17\x10", 3);
            // memcpy(cbCmd, "\xfe\x17\x40", 3);
            // memcpy(cbCmd, "\xfe\x17\x41", 3);
            // memcpy(cbCmd, "\xfe\x17\x34", 3); 需要输入参数
            _WriteLog(LL_DEBUG, "GetInvBuff, unsupport optype");
            return -1;
            break;
    }
    //先获取后续的参数信息,分配对应内存
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

// QueryCount为begin之后查询的张数，最少为1
int TransferFPDMAndFPHMToBytes(char *szInFPDM, char *szInFPHMBegin, int nQueryCount, uint8 *pOutBuf)
{
    if (!nQueryCount)
        nQueryCount = 1;
    char szBuf[36] = "";
    snprintf(szBuf, 36, "%12s%8s%08d", szInFPDM, szInFPHMBegin, nQueryCount);
    MemoryFormart(MF_TRANSLATE_SPACE2STRZERO, szBuf, strlen(szBuf));
    return Str2Byte(pOutBuf, szBuf, strlen(szBuf));
}

//根据FPDM FPHM及类型查询FPXX
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
    if (!strlen(fpxx->je)) {  //需要测试200.34等特殊金额情况
        _WriteLog(LL_FATAL, "ZFFP must has jshj");
        return -1;
    }
    if (!strlen(fpxx->kpr)) {  // kpr当作zfr,免得再加入一个参数
        _WriteLog(LL_WARN, "kpr incorrect, 5word");
        return -2;
    }
    strncpy(szHexBuf, hDev->szDeviceTime, 8);
    GetFotmatJEStr(fpxx->je, szHexBuf + 8);
    FillTwocharRepeat(szHexBuf + 20, "20", 20);
    nIndex = 20;
    //!!!注意，20220429版本后，开票统一改为UTF8格式，但是202206月份测试时这里的作废依然是gbk模式，
    //如果后期调整为utf8模式时，后面strncpy需要需要再多偏移2个字节，由2个字节改为4字节
    memset(szBuf, 0, sizeof szBuf);
    StrToNisecTemplate(false, szBuf, fpxx->kpr, strlen(fpxx->kpr));
    strncpy(szHexBuf + nIndex, szBuf + 2, strlen(szBuf + 2));
    nIndex += 40;

    sprintf(szBuf, "%012llu", atoll(fpxx->fpdm));
    strcat(szHexBuf, szBuf);
    strcat(szHexBuf, fpxx->fphm);
    strcat(szHexBuf, "0200");  // unknow(疑为密文长度)
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
        //这里得需要实时时间，不能用静态时间
        if (CntaxGetDeviceTime(hDev->hUSB, hDev->szDeviceTime) < 0)
            break;
        //没有销方税号的当作空白作废(底层设备自动填写销方税号和名称)
        if (strlen(fpxx->xfsh)) {  // stand waste
            if (TimeOutMonthDeadline(fpxx->kpsj_standard, hDev->szDeviceTime)) {
                _WriteLog(
                    LL_FATAL,
                    "Current device time is next month of the waste-invoice's month,DevTime:%s "
                    "InvTime:%s",
                    hDev->szDeviceTime, fpxx->kpsj_standard);
                SetLastError(fpxx->hDev->hUSB, ERR_LOGIC_WASTETIME_OVERFLOW,
                             "超过作废截止日期不允许作废");
                break;
            }
        } else {
            if (CommonGetCurrentInvCode(hDev, fpxx->fpdm, fpxx->fphm, fpxx->fpendhm)) {
                nRet = SetLastError(hDev->hUSB, ERR_PARM_CHECK,
                                    "发票代码号码指定错误，必须是发票卷当前头部代码号码");
                break;
            }
            if (!CheckFphmRangeIncorrect(fpxx->fphm, fpxx->fpendhm, nCount)) {
                nRet = SetLastError(fpxx->hDev->hUSB, ERR_PARM_CHECK,
                                    "发票代码号码范围错误，超过发票卷范围");
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

//作废发票-指定发票号码代码/不指定发票号码代码,只能作废顶部发票，顺序进行
int CntaxWasteInvoice(HDEV hDev, uint8 bInvType, bool bBlankWaste, char *szFPDM, char *szFPHM,
                      char *zfr, int nCount,int *need_restart)
{
    if ((FPLX_COMMON_DZFP == bInvType) || (FPLX_COMMON_DZZP == bInvType)) {
        return SetLastError(hDev->hUSB, ERR_LOGIC_DZFP_CANNOT_WASTE,
                            "发票类型代码错误，作废发票只能为纸专、纸普");
    }
    HFPXX fpxx = MallocFpxxLinkDev(hDev);
    int nRet = ERR_GENERIC;

    int nChildRet = -1;
    while (1) {
        fpxx->fplx = bInvType;
        if (!bBlankWaste) {  //非空白作废，需要查询到原发票
            if (nCount > 1) {
                nRet = SetLastError(hDev->hUSB, ERR_PARM_CHECK, "常规作废一次只允许作废一张");
                break;
            }
            _WriteLog(LL_INFO, "作废发票-指定发票号码代码查询发票\n");
            if ((nChildRet = CntaxQueryInvInfo(hDev, bInvType, szFPDM, szFPHM, fpxx)) < 0) {
                nRet =
                    SetLastError(hDev->hUSB, ERR_PARM_CHECK, "发票代码号码查询发票失败,无法作废");
                break;
            }
            _WriteLog(LL_INFO, "作废发票-指定发票号码代码查询发票成功\n");
            if (fpxx->zfbz) {
                nRet = SetLastError(hDev->hUSB, ERR_PARM_CHECK,
                                    "该发票代码号码已经作废过,无法重复作废");
                break;
            }
            if (fpxx->fplx != bInvType) {
                nRet = SetLastError(hDev->hUSB, ERR_PARM_CHECK,
                                    "传入的发票类型与实际开具的发票类型不符");
                break;
            }

			if (strcmp(fpxx->zyspsmmc, "*机动车*") == 0)
			{
				nRet = SetLastError(hDev->hUSB, ERR_PARM_CHECK,
					"机动车发票不允许作废");
				_WriteLog(LL_DEBUG, "%s", (char *)hDev->hUSB->errinfo);
				nRet = -1;
				break;
			}
			struct Spxx *stp_Spxx = NULL;
			int i;
			stp_Spxx = fpxx->stp_MxxxHead->stp_next;
			//_WriteLog(LL_FATAL, "商品数量=%d\n", Spxx_num);
			for (i = 0; i < fpxx->spsl; i++)
			{
				//_WriteLog(LL_FATAL,"组包第%d行商品\n",i+1);
				if (stp_Spxx == NULL)
				{
					break;
				}
				if (memcmp(stp_Spxx->spbh, "10701010", strlen("10701010")) == 0) {
					nRet = SetLastError(hDev->hUSB, ERR_PARM_CHECK,
						"开具成品油发票暂不支持作废");
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
						"机动车发票不允许作废");
					_WriteLog(LL_DEBUG, "%s", (char *)hDev->hUSB->errinfo);
					FreeFpxx(fpxx);
					return -2;
				}
                if ((memcmp(stp_Spxx->spbh, "103040501", strlen("103040501")) == 0) ||
                    (memcmp(stp_Spxx->spbh, "103040502", strlen("103040502")) == 0) ||
                    (memcmp(stp_Spxx->spbh, "103040503", strlen("103040503")) == 0) ||
                    (memcmp(stp_Spxx->spbh, "103040504", strlen("103040504")) == 0)) {
					nRet = SetLastError(hDev->hUSB, ERR_PARM_CHECK,
						"电子烟发票不允许作废");
					_WriteLog(LL_DEBUG, "%s", (char *)hDev->hUSB->errinfo);
					FreeFpxx(fpxx);
					return -3;
				}
                //wang 02 01 稀土编码并入矿产品
                if ((memcmp(stp_Spxx->spbh, "102", strlen("102")) == 0)) {
                    nRet = SetLastError(hDev->hUSB, ERR_PARM_CHECK,
						"矿产品发票不允许作废");
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
            //wang 发票卷最后一张
            if (strcmp(fpxx->fphm, fpxx->fpendhm) == 0) {
                _WriteLog(LL_WARN, "已使用到发票卷最后一张发票,开具完成后需要进行断电操作");
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
