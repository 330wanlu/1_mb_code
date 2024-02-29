/*****************************************************************************
File name:   nisec_getinvoice.c
Description: 发票读取恢复函数
Author:      Zako
Version:     1.0
Date:        2019.02
History:
20190220     代码初始化
*****************************************************************************/
#include "nisec_getinvoice.h"

// input example: "20180501-20190220"
inline int GetInvMonthFormat(char *szFormatYearMonth, uint8 *szRetBuff)
{
    int nRet = sscanf(szFormatYearMonth, "%2x%2x%2x%2x-%2x%2x%2x%2x", (unsigned int *)&szRetBuff[0],
                      (unsigned int *)&szRetBuff[1], (unsigned int *)&szRetBuff[2],
                      (unsigned int *)&szRetBuff[3], (unsigned int *)&szRetBuff[4],
                      (unsigned int *)&szRetBuff[5], (unsigned int *)&szRetBuff[6],
                      (unsigned int *)&szRetBuff[7]);

    if (nRet != 8)
        return -1;
    return 0;
}

//用于单张查询、月恢复、未上传查询,通用接口
// bOPType=0 月查询, = 1单张查询,=2 未上传查询
// pRetBuff返回缓冲区，nRetBuffLen返回缓冲区长度 return 返回发票张数
// abQueryInput input‘s buf，nQueryInputSize input's bufsize
// bReadOrPeek == false 只检测并获取可读取发票张数，不进行chunk IO;
//             == true  读取IO。Peek时只有返回值和nRetBuffLen被设置
int GetInvBuff(HUSB hUSB, uint8 bOPType, uint8 **pRetBuff, int *nRetBuffLen, uint8 *abQueryInput,
               int nQueryInputSize, bool bReadOrPeek)
{
    uint8 cb[512];
    int nRet = -2;
    uint8 cbCmd[16] = "";
    uint8 cbCmd2[16] = "";
    switch (bOPType) {
        case FPCX_COMMON_MONTH:  // montho query
            memcpy(cbCmd, "\xfe\x0e\x01\x10", 4);
            memcpy(cbCmd2, "\xfe\x0e\x01\x00", 4);
            break;
        case FPCX_COMMON_NUMBER:  // single query
            memcpy(cbCmd, "\xfe\x0e\x00\x10", 4);
            memcpy(cbCmd2, "\xfe\x0e\x00\x00", 4);
            break;
        case FPCX_COMMON_OFFLINE:  // to upload,官方FPSC接口
            memcpy(cbCmd, "\xfe\x0e\x01\x11", 4);
            memcpy(cbCmd2, "\xfe\x0e\x01\x01", 4);
            break;
        default:
            _WriteLog(LL_DEBUG, "GetInvBuff, unsupport optype");
            return -1;
            break;
    }
    while (1) {  // TC_ReadInvoDetail
        int nChildRet = 0;
        //先获取后续的参数信息,分配对应内存
        memcpy(cb, cbCmd, 4);
        nChildRet = NisecLogicIO(hUSB, cb, 4, sizeof(cb), abQueryInput, nQueryInputSize);
        if (nChildRet != 0) {
            nRet = -1;
            break;
        }
        //获取后续buf数量
        memcpy(cb, "\xfe\x08\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01", 16);
        nChildRet = NisecLogicIO(hUSB, cb, 16, sizeof(cb), NULL, 0);
        if (nChildRet != 8) {
            nRet = -2;
            break;
        }
        int nInvNum = bswap_32(*(uint32 *)cb);
        int nInvBufLen = bswap_32(*(uint32 *)(cb + 4));
        if (nInvNum == 0) {
            nRet = 0;  // no voice
            break;
        }
        if (nInvNum < 0) {
            _WriteLog(LL_DEBUG, "GetInvBuff, unknow retbuf");
            nRet = -3;
            break;
        }
#ifdef MIPS_MTK
        if (nInvBufLen > 16 * DEF_MAX_FPBUF_LEN) {  // MAX 16M
#else
        if (nInvBufLen > 40 * DEF_MAX_FPBUF_LEN) {  // MAX 40M
#endif

            _WriteLog(LL_DEBUG, "GetInvBuff, too big IO, please narrow the query's range");
            nRet = -4;
            break;
        }
        if (!bReadOrPeek) {  // just peek
            nRet = nInvNum;
            *nRetBuffLen = nInvBufLen;
            break;
        }
        // chunk IO
        memcpy(cb, cbCmd2, 4);
        nChildRet = NisecLogicIO(hUSB, cb, 4, sizeof(cb), abQueryInput, nQueryInputSize);
        if (nChildRet != 0) {
            nRet = -5;
            break;
        }
        uint8 *pBuf = calloc(1, nInvBufLen);
        if (!pBuf) {
            _WriteLog(LL_INFO, "读取发票时申请内存失败,申请内存大小为%d字节", nInvBufLen);
            nRet = -11;
            break;
        }
        memcpy(pBuf, "\xfe\x08\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01", 16);
        nChildRet = NisecLogicIO(hUSB, pBuf, 16, nInvBufLen, 0, 0);
        if (nChildRet < 0 || nChildRet != nInvBufLen) {
            free(pBuf);
            _WriteLog(LL_INFO, "GetInvBuff error nChildRet = %d", nChildRet);

            nRet = -6;
            break;
        }
        *pRetBuff = pBuf;
        *nRetBuffLen = nInvBufLen;
        nRet = nInvNum;
        break;
    }
    return nRet;
}

//会改变pOrigBuffCopy缓冲区内容
int DecodeSingleInvoice(char *szDeviceID, uint8 *abCurInvoTemp, int nBlockBinLen,
                        char *pOrigBuffCopy, char *pDetailBuff, char *szOutJYM)
{
    int nOrigBuffCopyIndex = 0, npDetailBuffIndex = 0, nRet = 0, nChildRet = 0;
    char szXfsh[24] = "", szKpsj_standard[32] = "", szTaxAuthorityCodeEx[32] = "", szBuf[256] = "",
         szMWBuf[256] = "";
    uint8 bInvType = abCurInvoTemp[3];
    bool bIsRed = false;
    while (1) {  //这里要对缓冲区进行简单变换，还要取出基本要素来计算密文和校验码
        uint8 bInvoiceStatusFlag =
            NisecDecodeNum(pOrigBuffCopy + 4, 0x2) & 0x7f;  // enCOMMON_FP_STATUS
        strncpy(szKpsj_standard, pOrigBuffCopy + 0xe, 8);
        if (FPSTATUS_RED == bInvoiceStatusFlag || FPSTATUS_RED_WASTE == bInvoiceStatusFlag)
            bIsRed = true;
        // 解密后比之前小
        nOrigBuffCopyIndex = 118;
        npDetailBuffIndex = 118;
        memcpy(pDetailBuff, pOrigBuffCopy, nOrigBuffCopyIndex);
        if (NisecBaseConv(pOrigBuffCopy + nOrigBuffCopyIndex, 10, 0x10,
                          pDetailBuff + npDetailBuffIndex, 0xa, 0xc) < 0)
            break;
        strncpy(szTaxAuthorityCodeEx, pDetailBuff + npDetailBuffIndex, 0xc);
        nOrigBuffCopyIndex += 10;
        npDetailBuffIndex += 12;
        // xhdwsbh
        if (NisecBaseConv(pOrigBuffCopy + nOrigBuffCopyIndex, 26, 0x10,
                          pDetailBuff + npDetailBuffIndex, 0x24, 0) < 0)
            break;
        strncpy(szXfsh, pDetailBuff + npDetailBuffIndex, 20);
        nOrigBuffCopyIndex += 26;
        npDetailBuffIndex += 20;  //实际18位，预留20位
        // ghdwsbh
        if (NisecBaseConv(pOrigBuffCopy + nOrigBuffCopyIndex, 26, 0x10,
                          pDetailBuff + npDetailBuffIndex, 0x24, 0) < 0)
            break;
        SpecialGfshRestore(pDetailBuff + npDetailBuffIndex,
                           strlen(pDetailBuff + npDetailBuffIndex));
        nOrigBuffCopyIndex += 26;
        npDetailBuffIndex += 20;
        // xhdwsbh2
        if (NisecBaseConv(pOrigBuffCopy + nOrigBuffCopyIndex, 26, 0x10,
                          pDetailBuff + npDetailBuffIndex, 0x24, 0) < 0)
            break;
        nOrigBuffCopyIndex += 26;
        npDetailBuffIndex += 20;
        // append kpsj
        strncpy(szKpsj_standard + 8, pOrigBuffCopy + nOrigBuffCopyIndex, 6);
        if (TranslateStandTime(0, szKpsj_standard, szBuf) < 0)  // check is ok
            break;
        //#bin copy
        memcpy(pDetailBuff + npDetailBuffIndex, pOrigBuffCopy + nOrigBuffCopyIndex, 88);
        nOrigBuffCopyIndex += 88;
        npDetailBuffIndex += 88;
        // get header
        memset(szBuf, 0, sizeof(szBuf));
        strncpy(szBuf, pOrigBuffCopy + nOrigBuffCopyIndex - 16, 4);
        int nSignStrLen = strtol(szBuf, NULL, 16);
        if (!nSignStrLen || nSignStrLen > DEF_MAX_FPBUF_LEN)
            break;
        int nDetailMovLen = 0;
        int nRemainLen = strlen(pOrigBuffCopy) - nOrigBuffCopyIndex;
        memcpy(pDetailBuff + npDetailBuffIndex, pOrigBuffCopy + nOrigBuffCopyIndex, nRemainLen);
        //以上发票头部部分转换完成
        int nOrigMovLen =
            DecodeTail(bInvoiceStatusFlag, abCurInvoTemp, pDetailBuff + npDetailBuffIndex,
                       nRemainLen, nBlockBinLen, nSignStrLen, &nDetailMovLen);
        if (nOrigMovLen < 0)
            break;
        npDetailBuffIndex += nDetailMovLen;
        nOrigBuffCopyIndex += nOrigMovLen;
        //之前多拷贝的清0,为了方便比对;前面的需要和官方一样变成大写，后面的是校验码和签名，有base64，因此不能全部无脑转换，保留原格式
        memset(pDetailBuff + npDetailBuffIndex, 0, nRemainLen);
        int i = 0;
        for (i = 0; i < npDetailBuffIndex; i++) {
            if (pDetailBuff[i] == 0x00)
                pDetailBuff[i] = 0x20;
        }
        mystrupr(pDetailBuff);
        if (FPSTATUS_BLANK_WASTE != bInvoiceStatusFlag) {
            //如果不是空白作废，才解析密文
            // jym
            memset(szBuf, 0, sizeof szBuf);
            Str2Byte((uint8 *)szBuf, pOrigBuffCopy + nOrigBuffCopyIndex, 0x50);
            if (NisecJYMDecAPI((uint8 *)szBuf, 40, szOutJYM) < 0)
                break;
            // mw
            if ((nChildRet = NisecMWDecAPI(bInvType, bIsRed, szDeviceID, szTaxAuthorityCodeEx,
                                           szKpsj_standard, szXfsh, (uint8 *)szBuf, szMWBuf)) < 0)
                break;
            strcpy(pDetailBuff + npDetailBuffIndex, szMWBuf);
            nOrigBuffCopyIndex += 0x50;
            npDetailBuffIndex += nChildRet;
        }  // cntax如果是空白作废，会将原usb数据中无价值数据拷贝到dstbuf，而nisec不拷贝
        // sign,原封不动的copy,得转换成bin copy
        nChildRet = Str2Byte((uint8 *)pDetailBuff + npDetailBuffIndex,
                             pOrigBuffCopy + nOrigBuffCopyIndex, nSignStrLen * 2);
        nOrigBuffCopyIndex += nSignStrLen;
        npDetailBuffIndex += nSignStrLen;
        // 更新Block+3处长度信息,理论上不会出现奇数
        int nNewBlockLen = (npDetailBuffIndex - 14) / 2;
        sprintf(szBuf, "%08x", nNewBlockLen);
        memcpy(pDetailBuff + 6, szBuf, 8);

        nRet = npDetailBuffIndex;
        break;
    }
    return nRet;
}

//明细和清单解析,nisec/cntax目前暂时共用一个解析函数
int GetFYXMorQDXM(HFPXX fpxx, bool bUtf8Buf, bool bFYorQD, char *pDetailBuff)
{
    // init and get config
    int hsjbz = -1;
    uint8 same_hsjbz = 1;
    char szBuf[256] = "";
    int i = 0, nIndex = 0, nCount = 0;
    memset(szBuf, 0, sizeof szBuf);
    strncpy(szBuf, pDetailBuff + nIndex, 4);
    nIndex += 4;
    nCount = strtol(szBuf, 0, 16);  //无清单时,0 == count,qdxm不处理
    bool bFirstNode = false;
    if (!strcasecmp(fpxx->qdbj, "N")) {  // no qd
        if (bFYorQD)
            fpxx->spsl = nCount;  // step1
    } else {                      // has qd
        if (bFYorQD)
            bFirstNode = true;
        else
            fpxx->spsl = nCount;  // step2
    }
    // load detail
    for (i = 0; i < nCount; i++) {
        struct Spxx *pItemSpxx = NULL;
        if (bFirstNode)
            pItemSpxx = fpxx->stp_MxxxHead;
        else
            pItemSpxx = (struct Spxx *)calloc(1, sizeof(struct Spxx));
        // xh nisec永远是4位长度01xx，因此就有超过255个的bug；cntax为标准FormatTemplate
        sprintf(pItemSpxx->xh, "%d", i + 1);
        char spxh[256] = {0};
        nIndex = FormatTemplate(bUtf8Buf, pDetailBuff, nIndex, spxh, sizeof spxh);
        // fphxz
        nIndex = FormatTemplate(bUtf8Buf, pDetailBuff, nIndex, pItemSpxx->fphxz,
                                sizeof pItemSpxx->fphxz);
        // je(no tax)
        nIndex = FormatTemplate(bUtf8Buf, pDetailBuff, nIndex, pItemSpxx->je, sizeof pItemSpxx->je);
        // slv
        nIndex =
            FormatTemplate(bUtf8Buf, pDetailBuff, nIndex, pItemSpxx->slv, sizeof pItemSpxx->slv);
        // se
        nIndex = FormatTemplate(bUtf8Buf, pDetailBuff, nIndex, pItemSpxx->se, sizeof pItemSpxx->se);
        // SPSMMC+SPMC
        memset(szBuf, 0, sizeof szBuf);
        nIndex = FormatTemplate(bUtf8Buf, pDetailBuff, nIndex, szBuf, sizeof szBuf);
        GetSpsmmcSpmc(szBuf, pItemSpxx->spsmmc, pItemSpxx->spmc);
        // spsm,该参数已在我方代码中暂不支持
        nIndex =
            FormatTemplate(bUtf8Buf, pDetailBuff, nIndex, pItemSpxx->spsm, sizeof pItemSpxx->spsm);
        // ggxh
        nIndex =
            FormatTemplate(bUtf8Buf, pDetailBuff, nIndex, pItemSpxx->ggxh, sizeof pItemSpxx->ggxh);
        // jldw
        nIndex =
            FormatTemplate(bUtf8Buf, pDetailBuff, nIndex, pItemSpxx->jldw, sizeof pItemSpxx->jldw);
        // spsl
        nIndex = FormatTemplate(bUtf8Buf, pDetailBuff, nIndex, pItemSpxx->sl, sizeof pItemSpxx->sl);
        // dj
        nIndex = FormatTemplate(bUtf8Buf, pDetailBuff, nIndex, pItemSpxx->dj, sizeof pItemSpxx->dj);
        // hsjbz
        nIndex = FormatTemplate(bUtf8Buf, pDetailBuff, nIndex, pItemSpxx->hsjbz,
                                sizeof pItemSpxx->hsjbz);
        if (atoi(pItemSpxx->hsjbz) != 1) {
            memset(pItemSpxx->hsjbz, 0, sizeof(pItemSpxx->hsjbz));
            sprintf(pItemSpxx->hsjbz, "0");
        }
        if (fpxx->hDev->bDeviceType == DEVICE_NISEC && fpxx->nVerInvoice <= 0x9912) {
            // nisec老票种兼容
            sprintf(pItemSpxx->spbh, "0000000000000000000");
            sprintf(pItemSpxx->zxbh, "01");
        } else {
            // spbh
            nIndex = FormatTemplate(bUtf8Buf, pDetailBuff, nIndex, pItemSpxx->spbh,
                                    sizeof pItemSpxx->spbh);
            // zxbm
            nIndex = FormatTemplate(bUtf8Buf, pDetailBuff, nIndex, pItemSpxx->zxbh,
                                    sizeof pItemSpxx->zxbh);
            // yhzcbs
            nIndex = FormatTemplate(bUtf8Buf, pDetailBuff, nIndex, pItemSpxx->xsyh,
                                    sizeof pItemSpxx->xsyh);
            // lslvbs
            nIndex = FormatTemplate(bUtf8Buf, pDetailBuff, nIndex, pItemSpxx->lslvbs,
                                    sizeof pItemSpxx->lslvbs);
            // ZZSTSGL
            nIndex = FormatTemplate(bUtf8Buf, pDetailBuff, nIndex, pItemSpxx->yhsm,
                                    sizeof pItemSpxx->yhsm);
        }
        //判断是否同一税率
        if (bFYorQD) {
            int hsj_tmp = atoi(pItemSpxx->hsjbz);
            if (hsjbz == -1)
                hsjbz = hsj_tmp;
            else {
                if (hsjbz != hsj_tmp)
                    same_hsjbz = 0;
            }
        }
        if (!bFirstNode)
            InsertMxxx(fpxx, pItemSpxx);
    }
    if (bFYorQD) {
        if (same_hsjbz)
            sprintf(fpxx->hsjbz, "%d", hsjbz);
        else
            sprintf(fpxx->hsjbz, "%d", 2);
    }
    return nIndex;
}

//只是累加偏移,三个要素在主干代码已获取到,在此不重复获取
int GetQTXM(bool bUtf8Buf, char *pDetailBuff)
{
    char szBuf[256] = "";
    int i = 0, nIndex = 0, nCount = 0;
    strncpy(szBuf, pDetailBuff + nIndex, 4);
    nIndex += 4;
    nCount = strtol(szBuf, 0, 16);
    // load detail
    for (i = 0; i < nCount; i++) {
        // sl
        nIndex = FormatTemplate(bUtf8Buf, pDetailBuff, nIndex, szBuf, sizeof szBuf);
        // je
        nIndex = FormatTemplate(bUtf8Buf, pDetailBuff, nIndex, szBuf, sizeof szBuf);
        // se
        nIndex = FormatTemplate(bUtf8Buf, pDetailBuff, nIndex, szBuf, sizeof szBuf);
    }
    return nIndex;
}

// 原函数名NisecDecode9915Buf,为2020年nisec和cntax新加入的buf
int Decode2020YearBuf(HFPXX fpxx, bool bUtf8Buf, char *pDetailBuff, int nIndex)
{
    char szBuf[512] = "";
    char saSplitBuf[5][768];
    memset(saSplitBuf, 0, sizeof(saSplitBuf));
    // ptlx(平台类型)ptxh（平台序号） tqm（提取码）
    nIndex = FormatTemplate(bUtf8Buf, pDetailBuff, nIndex, szBuf, sizeof szBuf);  // ptlx预读取
    if (!strlen(szBuf)) {  //未启用或者启用无配置信息
        nIndex = FormatTemplate(bUtf8Buf, pDetailBuff, nIndex, szBuf, sizeof szBuf);  // ptxh空白
        nIndex = FormatTemplate(bUtf8Buf, pDetailBuff, nIndex, szBuf, sizeof szBuf);  // tqm空白
    } else {  //启用且有配置信息
        // ptlx
        strcat(fpxx->pubcode, szBuf);
        strcat(fpxx->pubcode, ";");
        // ptxh
        nIndex = FormatTemplate(bUtf8Buf, pDetailBuff, nIndex, szBuf, sizeof szBuf);
        strcat(fpxx->pubcode, szBuf);
        strcat(fpxx->pubcode, ";");
        // tqm
        nIndex = FormatTemplate(bUtf8Buf, pDetailBuff, nIndex, szBuf, sizeof szBuf);
        strcat(fpxx->pubcode, szBuf);
    }
    // unknow
    nIndex = FormatTemplate(bUtf8Buf, pDetailBuff, nIndex, szBuf, sizeof szBuf);
    // unknow
    nIndex = FormatTemplate(bUtf8Buf, pDetailBuff, nIndex, szBuf, sizeof szBuf);
    // 1%3% 优惠税率开具原因
    nIndex = FormatTemplate(bUtf8Buf, pDetailBuff, nIndex, szBuf, sizeof szBuf);
    fpxx->sslkjly = atoi(szBuf);
    // unknow
    nIndex = FormatTemplate(bUtf8Buf, pDetailBuff, nIndex, szBuf, sizeof szBuf);
    // 红冲信息,无设置则为""或者,,,,
    nIndex = FormatTemplate(bUtf8Buf, pDetailBuff, nIndex, szBuf, sizeof szBuf);
    if ((strlen(szBuf) > 4) && (GetSplitStringSimple(szBuf, ",", saSplitBuf, 5) == 5)) {
        fpxx->hcyy = atoi(saSplitBuf[0]);
        fpxx->blueFplx = atoi(saSplitBuf[1]);
        strcpy(fpxx->blueKpsj, saSplitBuf[2]);
        strcat(fpxx->blueKpsj, "000000");
        strcpy(fpxx->blueFpdm, saSplitBuf[3]);
        strcpy(fpxx->blueFphm, saSplitBuf[4]);
    }
    // unknow
    nIndex = FormatTemplate(bUtf8Buf, pDetailBuff, nIndex, szBuf, sizeof szBuf);
    // unknow
    nIndex = FormatTemplate(bUtf8Buf, pDetailBuff, nIndex, szBuf, sizeof szBuf);
    return nIndex;
}

int NisecDecodeBuf2Fpxx(char *pDetailBuff, HFPXX fpxx)
{
    bool bUtf8Buf = false;  //预留
    CommonFPLX2AisinoFPLX(fpxx->fplx, &fpxx->fplx_aisino);
    char szBuf[512];
    uint16 nZyfplxEnable = 0;
    int nIndex = 4;

    //printf_array(pDetailBuff, strlen(pDetailBuff));

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
    nIndex += 72;
    // tspz标志?("ABCD"->abcd)
    nZyfplxEnable = 0;
    Str2Byte((uint8 *)&nZyfplxEnable, pDetailBuff + nIndex, 4);
    nZyfplxEnable = bswap_16(nZyfplxEnable);
    nIndex += 4;
    //新版本标识?
    if (strncmp(pDetailBuff + nIndex, "FFFEFDFC", 8))
        nZyfplxEnable = 0;  //不等
    nIndex += 8;
    // hjse
    nTmpNum = NisecDecodeNum(pDetailBuff + nIndex, 0xc);
    sprintf(fpxx->se, "%0.2f", (double)nTmpNum / 100);
    if (fpxx->isRed)
        MemoryFormart(MF_TRANSLATE_RED, fpxx->se, strlen(fpxx->se));
    nIndex += 12;
    // szTaxAuthorityCodeEx
    nIndex += 12;
    // xfsh
    strncpy(fpxx->xfsh, pDetailBuff + nIndex, 20);
    MemoryFormart(MF_TRANSLATE_SPACE2ZERO, fpxx->xfsh, strlen(fpxx->xfsh));
    nIndex += 20;
    // gfsh
    strncpy(fpxx->gfsh, pDetailBuff + nIndex, 20);
    MemoryFormart(MF_TRANSLATE_SPACE2ZERO, fpxx->gfsh, strlen(fpxx->gfsh));
    if (!strcmp(fpxx->gfsh, "00000000000000000000"))
        strcpy(fpxx->gfsh, "000000000000000");  //无购方税号、空税号
    nIndex += 20;
    // xfsh2
    nIndex += 20;
    // kpsj
    strncpy(fpxx->kpsj_standard + 8, pDetailBuff + nIndex, 6);
    UpdateFpxxAllTime(fpxx->kpsj_standard, fpxx);
    nIndex += 6;
    // zfsj
    memset(fpxx->zfsj, 0, sizeof(fpxx->zfsj));
    strncpy(fpxx->zfsj, pDetailBuff + nIndex, 14);
    TranslateStandTime(2, fpxx->zfsj, fpxx->zfsj_F2);
    nIndex += 14;
    // zfr
    strncpy(szBuf, pDetailBuff + nIndex, 40);
    nIndex += 40;
    // unknow 12
    nIndex += 12;
    // qmcs
    memset(szBuf, 0, sizeof szBuf);
    strncpy(szBuf, pDetailBuff + nIndex, 16);
    nIndex += 16;
    strcpy(fpxx->qmcs, szBuf);
    szBuf[4] = '\0';
    int nSignStrLen = strtol(szBuf, NULL, 16);
    // fpdm 官方代码中长度为固定值，ZYFP固定10和8字节,PTFP/DZFP固定12和8字节
    // 18年以前是否存在10位PT发票代码，根据官方购票信息做法，移除前面2个0（尚未实盘确认）
    nTmpNum = 12;
    if (FPLX_COMMON_ZYFP == fpxx->fplx)
        nTmpNum = 10;
    strncpy(fpxx->fpdm, pDetailBuff + nIndex, nTmpNum);
    RemovePrefixTwoZero(fpxx->fpdm);  //兼容18年前老格式fpdm 10位
    nIndex += nTmpNum;
    // fphm
    strncpy(fpxx->fphm, pDetailBuff + nIndex, 8);
    nIndex += 8;
    if (FPSTATUS_BLANK_WASTE == fpxx->fpzt)
        goto DecodeSign;
    // yfpdm
    nTmpNum = 12;
    if (FPLX_COMMON_ZYFP == fpxx->fplx)
        nTmpNum = 10;
    strncpy(fpxx->blueFpdm, pDetailBuff + nIndex, nTmpNum);
    if (atoll(fpxx->blueFpdm))
        RemovePrefixTwoZero(fpxx->blueFpdm);  //兼容18年前老格式fpdm 10位
    nIndex += nTmpNum;
    // yfphm
    strncpy(fpxx->blueFphm, pDetailBuff + nIndex, 8);
    nIndex += 8;
    // unknow
    nIndex = FormatTemplate(bUtf8Buf, pDetailBuff, nIndex, szBuf, sizeof szBuf);
    // version--我们也即将逐渐淘汰老版本的支持
    // 20200228之前版本9915 0199150399=10bytes
    // 20200228更新9916 0199160399
    // 20211231更新9917
    memset(szBuf, 0, sizeof szBuf);
    strncpy(szBuf, pDetailBuff + nIndex + 2, 4);
    fpxx->nVerInvoice = strtol(szBuf, 0, 16);  //无清单时,0 == count,qdxm不处理
    if (fpxx->nVerInvoice > 0x9917 || fpxx->nVerInvoice < 0x9912) {
        //格式太老,甩掉包袱,不支持2016年之前的读取（老版本无编码表版本号、商品编码等）
        _WriteLog(LL_FATAL,
                  "New nisec's invoice model has found or incorrect invoice buf, contact "
                  "developer please...,nVer:%d FPDM:%s FPHM:%s",
                  fpxx->nVerInvoice, fpxx->fpdm, fpxx->fphm);
        return -4;
    }
    nIndex += 10;
    if (fpxx->nVerInvoice > 0x9916 && nZyfplxEnable) {
        // example1-ofset68 :0101313037303130313037303130303030303030302020202020000000000195F1E2
        // example2-ofset132:02013130373031303130373032303030303030303020202020200000000000BF96E03130373031303130373031303030303030303020202020200000000005B46B30
        //柴油:10701010301；润滑油编码:10701010701;此处跳过 成品油授权编码处理过程
        memset(szBuf, 0, sizeof(szBuf));
        strncpy(szBuf, pDetailBuff + nIndex, 2);
        nTmpNum = (atoi(szBuf) << 6) + 4;
        nIndex += nTmpNum;
    }
    //----动态不定长数据
    // rednum-tzdbh
    nIndex = FormatTemplate(bUtf8Buf, pDetailBuff, nIndex, fpxx->redNum, sizeof fpxx->redNum);
    // kpjh?
    nIndex = FormatTemplate(bUtf8Buf, pDetailBuff, nIndex, szBuf, sizeof szBuf);
    fpxx->kpjh = atoi(szBuf);
    // tspz
    nIndex = FormatTemplate(bUtf8Buf, pDetailBuff, nIndex, szBuf, sizeof szBuf);
    Zyfplx2String(true, szBuf, &fpxx->zyfpLx);
    // deviceid
    nIndex = FormatTemplate(bUtf8Buf, pDetailBuff, nIndex, szBuf, sizeof szBuf);
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
    if (!strcmp(fpxx->slv, "99.01")) {
        fpxx->isMultiTax = true;
        memset(fpxx->slv, 0, sizeof(fpxx->slv));
    }
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
    // bmbbh,hsjbz
    if (fpxx->nVerInvoice > 0x9912) {  // 已确认9913加入编码表版本号
        nIndex = FormatTemplate(bUtf8Buf, pDetailBuff, nIndex, fpxx->bmbbbh, sizeof fpxx->bmbbbh);
        if (fpxx->nVerInvoice > 0x9914)  // 已确认9915加入含税价标志
            nIndex = FormatTemplate(bUtf8Buf, pDetailBuff, nIndex, fpxx->hsjbz, sizeof fpxx->hsjbz);
    }
    //公共平台配置，税局版式提取等新版本加入
    if (fpxx->nVerInvoice > 0x9915)
        nIndex = Decode2020YearBuf(fpxx, bUtf8Buf, pDetailBuff, nIndex);
    // fyxm
    nIndex += GetFYXMorQDXM(fpxx, bUtf8Buf, true, pDetailBuff + nIndex);
    // qdxm
    nIndex += GetFYXMorQDXM(fpxx, bUtf8Buf, false, pDetailBuff + nIndex);
    // qtxm
    nIndex += GetQTXM(bUtf8Buf, pDetailBuff + nIndex);
    // mw
    strncpy(fpxx->mw, pDetailBuff + nIndex, 0x70);
    nIndex += 0x70;
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
int NisecInvRaw2Fpxx(HDEV hDev, uint8 bInvType, uint8 *pBlockBin, HFPXX fpxx, int *pnBlockBinLen)
{
    int nRet = -1;
    int result;
    char *pInvoiceStr = NULL;
    char *pInvoiceDecodeBuf = NULL;
    //长度的转换方式任然不确定，需要进一步测试,++待办
    uint32 nBlockBinLen = bswap_32(*(uint32 *)(pBlockBin + 3));
    if (nBlockBinLen > DEF_MAX_FPBUF_LEN)
        return -100;
    *pnBlockBinLen = nBlockBinLen + 7;
    int nMaxInvoiceLen = DEF_MAX_FPBUF_LEN * 2 + 2048;
    pInvoiceStr = (char *)malloc(nMaxInvoiceLen);
    if (!pInvoiceStr) {
        nRet = -2;
        goto Finish;
    }
    pInvoiceDecodeBuf = calloc(1, nBlockBinLen * 4 + DEF_MAX_FPBUF_LEN);
    if (!pInvoiceDecodeBuf) {
        nRet = -3;
        goto Finish;
    }
    memset(pInvoiceStr, 0, nMaxInvoiceLen);
    Byte2Str(pInvoiceStr, pBlockBin, nBlockBinLen + 7);  // 7byte(3+4) +nBlockBinLen
    char szJYM[32] = "";
    int nChildRet = DecodeSingleInvoice(hDev->szDeviceID, hDev->abInvTemplate, nBlockBinLen,
                                        pInvoiceStr, (char *)pInvoiceDecodeBuf, szJYM);
    if (!nChildRet) {
        nRet = -4;
        goto Finish;
    }
    fpxx->fplx = bInvType;
    strcpy(fpxx->jym, szJYM);
    strcpy(fpxx->xfmc, hDev->szCompanyName);
    //原始发票，用于发票上传
    if (fpxx->pRawFPBin)
        free(fpxx->pRawFPBin);
    fpxx->pRawFPBin = calloc(1, nChildRet + 4);
    if (fpxx->pRawFPBin) {
        memcpy(fpxx->pRawFPBin, &nChildRet, 4);
        memcpy(fpxx->pRawFPBin + 4, pInvoiceDecodeBuf, nChildRet);
    }
    //_WriteHexToDebugFile("nisec-decode-hex.txt", (uint8 *)pInvoiceDecodeBuf, nChildRet);
    if ((result = NisecDecodeBuf2Fpxx((char *)pInvoiceDecodeBuf, fpxx)) < 0) {
        _WriteLog(LL_INFO, "NisecDecodeBuf2Fpxx failed: result:%d", result);
        nRet = -5;
        goto Finish;
    }
    nRet = nBlockBinLen + 7;
#ifdef _CONSOLE
    _WriteLog(LL_DEBUG,
              "QD:%s TSPZ:%d IsUpload:%d FPLX:%d KPSJ:%s FPDM:%s FPHM:%s BZ:%s JYM:%s MW:%s",
              fpxx->qdbj, fpxx->zyfpLx, fpxx->bIsUpload, fpxx->fplx, fpxx->kpsj_standard,
              fpxx->fpdm, fpxx->fphm, fpxx->bz, fpxx->jym, fpxx->mw);
#endif
Finish:
    if (pInvoiceStr)
        free(pInvoiceStr);
    if (pInvoiceDecodeBuf)
        free(pInvoiceDecodeBuf);
    return nRet;
}

//只能按照FPLX进行读取和解析
//返回值无回调，返回发票张数；有回调返回单张发票上传成功与否
int NisecGetInvDetail(HDEV hDev, uint8 bInvType, uint8 *pBuf, int nInvNum, int (*pCallBack)(HFPXX))
{
    int i = 0, nBlockIndex = 0, nRet = -1, nChildRet = -1;
    for (i = 0; i < nInvNum; i++) {
        HFPXX fpxx = MallocFpxxLinkDev(hDev);
        if (!fpxx)
            break;
        uint8 *pBlockBin = pBuf + nBlockIndex;
        int nBlockBinLen = 0;
        nChildRet = NisecInvRaw2Fpxx(hDev, bInvType, pBlockBin, fpxx, &nBlockBinLen);
        if (nChildRet < 0) {
            FreeFpxx(fpxx);
            break;
        }
        nBlockIndex += nChildRet;
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
#endif
        if (pCallBack)
            nChildRet = pCallBack(fpxx);
        FreeFpxx(fpxx);
    }
    if (pCallBack)
        nRet = nChildRet;
    else {
        if (i == nInvNum)
            nRet = nInvNum;
        else
            _WriteLog(LL_FATAL, "NisecGetInvDetail failed, %d!=%d", nInvNum, i);
    }
    return nRet;
}

//根据FPDM FPHM及类型查询FPXX
int NisecQueryInvInfo(HDEV hDev, uint8 bInvType, char *szFPDM, char *szFPHM, HFPXX fpxx)
{
    uint8 *pRetBuff = NULL;
    int nRetBuffLen = 0, nRet = -1, nChildRet = 0;
    int nInvNum = -1;
    int result;
    if (strlen(szFPDM) < 8 || strlen(szFPDM) < 8)
        return -2;
    uint8 abParamBuf[11] = {0};
    TransferFPDMAndFPHMTo9Byte(szFPDM, szFPHM, abParamBuf);
    memcpy(abParamBuf + 9, "\x00\x01", 2);

    if (NisecEntry(hDev->hUSB, NOP_OPEN_FULL) < 0)
        return -3;
    if (NisecSelectInvoiceType(hDev, bInvType, SELECT_OPTION_TEMPLATE) < 0) {
        if (NisecEntry(hDev->hUSB, NOP_CLOSE) < 0)
            return -4;
        return -5;
    }
    nInvNum = GetInvBuff(hDev->hUSB, FPCX_COMMON_NUMBER, &pRetBuff, &nRetBuffLen, abParamBuf,
                         sizeof abParamBuf, true);
    if (nInvNum <= 0) {
        _WriteLog(LL_WARN, "No invoice found, fpdm:%s fphm:%s", szFPDM, szFPHM);
        return -4;
    }
    _WriteHexToDebugFile("nisec_raw.bin", (uint8 *)pRetBuff, nRetBuffLen);

    while (1) {
        if ((result = NisecInvRaw2Fpxx(hDev, bInvType, pRetBuff, fpxx, &nChildRet)) < 0) {
            _WriteLog(LL_FATAL, "NisecInvRaw2Fpxx failed, result =%d", result);
            break;
        }
        //   if (NisecGetFPHumanceBuf(hDev, bInvType, pRetBuff) < 0)
        //       break;
        nRet = 0;
        break;
    }
    if (pRetBuff)
        free(pRetBuff);
    if (NisecEntry(hDev->hUSB, NOP_CLOSE) < 0)
        return -6;
    return nRet;
}

int NisecBuildWasteInvBuf(HFPXX fpxx, uint8 *pBuff)
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
    GetFotmatJEStr(fpxx->je, szHexBuf + 20);
    FillTwocharRepeat(szHexBuf + 32, "00", 30);
    nIndex = 92;

    FillTwocharRepeat(szHexBuf + nIndex, "20", 20);
    memset(szBuf, 0, sizeof szBuf);
    StrToNisecTemplate(false, szBuf, fpxx->kpr, strlen(fpxx->kpr));
    strncpy(szHexBuf + nIndex, szBuf + 2, strlen(szBuf + 2));
    nIndex += 40;

#ifdef _NODEVICE_TEST
    int nFpdmhmIndex = nIndex;
    strcat(szHexBuf, fpxx->fpdm);
    strcat(szHexBuf, fpxx->fphm);
    strcat(szHexBuf, fpxx->qmcs);
    _WriteHexToDebugFile("kpstring_nisec_wasteinvoice_my.txt", (uint8 *)szHexBuf, strlen(szHexBuf));
    memset(szHexBuf + nFpdmhmIndex, 0, sizeof(szHexBuf) - nFpdmhmIndex);
#endif
    memset(szBuf, 0, sizeof szBuf);
    NisecBaseConv(fpxx->fpdm, strlen(fpxx->fpdm), 10, szBuf, 16, 10);
    strcat(szHexBuf, szBuf);
    memset(szBuf, 0, sizeof szBuf);
    NisecBaseConv(fpxx->fphm, strlen(fpxx->fphm), 10, szBuf, 16, 8);
    strcat(szHexBuf, szBuf);
    strcat(szHexBuf, fpxx->qmcs);
    nIndex = Str2Byte(pBuff, szHexBuf, strlen(szHexBuf));
    _WriteHexToDebugFile("kpstring_nisec_wasteinvoice_my.bin", (uint8 *)pBuff, nIndex);
    return nIndex;
}

int NisecWasteInvoiceLoopIO(HFPXX fpxx, int nCount)
{
    int i = 0, nChildRet = 0, nRet = ERR_GENERIC;
    uint8 szOut[512] = "", cb[256];
    for (i = 0; i < nCount; i++) {
        if ((nChildRet = NisecBuildWasteInvBuf(fpxx, szOut)) < 0) {
            nRet = ERR_LOGIC_BUILD_FORMATBUF;
            break;
        }
        memcpy(cb, "\xfe\x0a", 2);
#ifndef _NODEVICE_TEST
        if (NisecLogicIO(fpxx->hDev->hUSB, cb, 2, sizeof(cb), szOut, nChildRet) != 0) {
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

int NisecWasteInvoiceInner(HFPXX fpxx, int nCount)
{
    HDEV hDev = fpxx->hDev;
    int nRet = ERR_GENERIC;
    if (NisecEntry(hDev->hUSB, NOP_OPEN_FULL) < 0)
        return ERR_DEVICE_OPEN;
    while (1) {
        if (NisecSelectInvoiceType(hDev, fpxx->fplx, SELECT_OPTION_TEMPLATE) < 0)
            break;
        //这里得需要实时时间，不能用静态时间
        if (NisecGetDeviceTime(hDev->hUSB, hDev->szDeviceTime) < 0)
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
            strcpy(fpxx->qmcs, hDev->szSignParameter);
            strcpy(fpxx->je, "0");
        }
        if ((nRet = NisecWasteInvoiceLoopIO(fpxx, nCount)) < 0)
            break;
        nRet = RET_SUCCESS;
        break;
    }
    NisecEntry(hDev->hUSB, NOP_CLOSE);
    return nRet;
}

//作废发票-指定发票号码代码/不指定发票号码代码,只能作废顶部发票，顺序进行
int NisecWasteInvoice(HDEV hDev, uint8 bInvType, bool bBlankWaste, char *szFPDM, char *szFPHM,
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
            if ((nChildRet = NisecQueryInvInfo(hDev, bInvType, szFPDM, szFPHM, fpxx)) < 0) {
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

        if ((nRet = NisecWasteInvoiceInner(fpxx, nCount)) < 0)
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

//----------------------------------对比可视化代码 begin---------------------------------
// src mustbe string
int AppendHumanceString(char *szSrc, int nSrcLen, char *pDstBufBegin, int nDstIndex, uint8 bSepType)
{
    strncpy(pDstBufBegin + nDstIndex, szSrc, nSrcLen);
    nDstIndex += nSrcLen;
    switch (bSepType) {
        case 0: {
            strcat(pDstBufBegin + nDstIndex, "\r\n");
            nDstIndex += 2;
            break;
        }
        case 1:
            strcat(pDstBufBegin + nDstIndex, "|");
            nDstIndex += 1;
            break;
        default:
            break;
    }
    return nDstIndex;
}

//int GetExtraHumanceString(bool bQTXM, char *pInvoiceDecodeBuf, char *pHumanceStr,
//                          int *nHumanceStrIndex)
//{
//    char szBuf[512] = "";
//    int i = 0, j = 0, nIndex = 0, nICount = 0, nJCount = 0;
//    strncpy(szBuf, pInvoiceDecodeBuf + nIndex, 4);
//    nICount = strtol(szBuf, 0, 16);
//    nIndex += 4;
//    for (i = 0; i < nICount; i++) {
//        if (bQTXM) {
//            nJCount = 3;
//        } else {
//            nIndex += 4;
//            nJCount = 16;
//        }
//        for (j = 0; j < nJCount; j++) {
//			nIndex = FormatTemplate(pInvoiceDecodeBuf, nIndex, szBuf, sizeof szBuf, fpxx->encoding);
//            *nHumanceStrIndex =
//                AppendHumanceString(szBuf, strlen(szBuf), pHumanceStr, *nHumanceStrIndex, 1);
//        }
//        *nHumanceStrIndex = AppendHumanceString("\r\n", 2, pHumanceStr, *nHumanceStrIndex, -1);
//    }
//    if (!nICount)
//        *nHumanceStrIndex = AppendHumanceString("\r\n", 2, pHumanceStr, *nHumanceStrIndex, -1);
//    return nIndex;
//}

//获取可视化FP数据，用于对比
//int NisecGetFPHumanceBuf(HDEV hDev, uint8 bInvType, uint8 *pBlockBin)
//{
//    char *pHumanceStr = NULL, *pInvoiceDecodeBuf = NULL;
//    int nRet = -3, nVer = 0, nHumanceStrIndex = 0, nInvoiceDecodeBufIndex = 4, i = 0;
//    char szBuf[1024], szFPHM[32] = "", szFPDM[32] = "";
//    //长度的转换方式任然不确定，需要进一步测试,++待办
//    uint32 nBlockBinLen = bswap_32(*(uint32 *)(pBlockBin + 3));
//    if (nBlockBinLen > DEF_MAX_FPBUF_LEN)
//        return -1;
//    int nMaxInvoiceLen = DEF_MAX_FPBUF_LEN * 2 + 2048;
//    pHumanceStr = (char *)malloc(nMaxInvoiceLen);
//    if (!pHumanceStr)
//        goto Finish;
//    pInvoiceDecodeBuf = calloc(1, nBlockBinLen * 2 + 4096);
//    if (!pInvoiceDecodeBuf)
//        goto Finish;
//    memset(pHumanceStr, 0, nMaxInvoiceLen);
//    Byte2Str(pHumanceStr, pBlockBin, nBlockBinLen + 7);  // 7byte(3+4) +nBlockBinLen
//    // szBuf == JYM
//    int nChildRet = DecodeSingleInvoice(hDev->szDeviceID, hDev->abInvTemplate, nBlockBinLen,
//                                        pHumanceStr, (char *)pInvoiceDecodeBuf, szBuf);
//    if (!nChildRet)
//        goto Finish;
//    //----------------------------------------------------------------------
//    memset(pHumanceStr, 0, nMaxInvoiceLen);
//    // fpzt num
//    int nTmpNum = NisecDecodeNum(pInvoiceDecodeBuf + nInvoiceDecodeBufIndex, 0x2);
//    //第一行全部放计算的数据,原发票并不存在
//    sprintf(pHumanceStr, "%d|%s|%d\r\n", bInvType, szBuf, nTmpNum);
//    nHumanceStrIndex = strlen(pHumanceStr);
//    nInvoiceDecodeBufIndex += 10;
//    // kprq
//    nHumanceStrIndex = AppendHumanceString(pInvoiceDecodeBuf + nInvoiceDecodeBufIndex, 8,
//                                           pHumanceStr, nHumanceStrIndex, 0);
//    nInvoiceDecodeBufIndex += 8;
//    // hjje
//    nTmpNum = NisecDecodeNum(pInvoiceDecodeBuf + nInvoiceDecodeBufIndex, 0xc);
//    sprintf(szBuf, "%0.2f", (double)nTmpNum / 100);
//    nHumanceStrIndex = AppendHumanceString(szBuf, strlen(szBuf), pHumanceStr, nHumanceStrIndex, 0);
//    nInvoiceDecodeBufIndex += 84;
//    // hjse
//    nTmpNum = NisecDecodeNum(pInvoiceDecodeBuf + nInvoiceDecodeBufIndex, 0xc);
//    sprintf(szBuf, "%0.2f", (double)nTmpNum / 100);
//    nHumanceStrIndex = AppendHumanceString(szBuf, strlen(szBuf), pHumanceStr, nHumanceStrIndex, 0);
//    nInvoiceDecodeBufIndex += 12;
//    // szTaxAuthorityCodeEx
//    nHumanceStrIndex = AppendHumanceString(pInvoiceDecodeBuf + nInvoiceDecodeBufIndex, 12,
//                                           pHumanceStr, nHumanceStrIndex, 0);
//    nInvoiceDecodeBufIndex += 12;
//    // xfsh
//    nHumanceStrIndex = AppendHumanceString(pInvoiceDecodeBuf + nInvoiceDecodeBufIndex, 20,
//                                           pHumanceStr, nHumanceStrIndex, 0);
//    nInvoiceDecodeBufIndex += 20;
//    // gfsh
//    nHumanceStrIndex = AppendHumanceString(pInvoiceDecodeBuf + nInvoiceDecodeBufIndex, 20,
//                                           pHumanceStr, nHumanceStrIndex, 0);
//    nInvoiceDecodeBufIndex += 20;
//    // xfsh2
//    nHumanceStrIndex = AppendHumanceString(pInvoiceDecodeBuf + nInvoiceDecodeBufIndex, 20,
//                                           pHumanceStr, nHumanceStrIndex, 0);
//    nInvoiceDecodeBufIndex += 20;
//    // kpsj
//    nHumanceStrIndex = AppendHumanceString(pInvoiceDecodeBuf + nInvoiceDecodeBufIndex, 6,
//                                           pHumanceStr, nHumanceStrIndex, 0);
//    nInvoiceDecodeBufIndex += 6;
//    // zfsj
//    nHumanceStrIndex = AppendHumanceString(pInvoiceDecodeBuf + nInvoiceDecodeBufIndex, 14,
//                                           pHumanceStr, nHumanceStrIndex, 0);
//    nInvoiceDecodeBufIndex += 14;
//    // zfr ignore
//    nInvoiceDecodeBufIndex += 40;
//    // 12
//    nInvoiceDecodeBufIndex += 12;
//    // qmcs
//    strncpy(szBuf, pInvoiceDecodeBuf + nInvoiceDecodeBufIndex, 16);
//    nHumanceStrIndex = AppendHumanceString(szBuf, 16, pHumanceStr, nHumanceStrIndex, 0);
//    nInvoiceDecodeBufIndex += 16;
//    szBuf[4] = '\0';
//    int nSignStrLen = strtol(szBuf, NULL, 16);
//    // fpdm 官方代码中长度为固定值，ZYFP固定10和8字节,PTFP固定12和8字节
//    nTmpNum = 12;
//    if (FPLX_COMMON_ZYFP == bInvType)
//        nTmpNum = 10;
//    strncpy(szFPDM, pInvoiceDecodeBuf + nInvoiceDecodeBufIndex, nTmpNum);
//    nHumanceStrIndex = AppendHumanceString(pInvoiceDecodeBuf + nInvoiceDecodeBufIndex, nTmpNum,
//                                           pHumanceStr, nHumanceStrIndex, 0);
//    nInvoiceDecodeBufIndex += nTmpNum;
//    // fphm
//    strncpy(szFPHM, pInvoiceDecodeBuf + nInvoiceDecodeBufIndex, 8);
//    nHumanceStrIndex = AppendHumanceString(pInvoiceDecodeBuf + nInvoiceDecodeBufIndex, 8,
//                                           pHumanceStr, nHumanceStrIndex, 0);
//    nInvoiceDecodeBufIndex += 8;
//    // yfpdm
//    nTmpNum = 12;
//    if (FPLX_COMMON_ZYFP == bInvType)
//        nTmpNum = 10;
//    nHumanceStrIndex = AppendHumanceString(pInvoiceDecodeBuf + nInvoiceDecodeBufIndex, nTmpNum,
//                                           pHumanceStr, nHumanceStrIndex, 0);
//    nInvoiceDecodeBufIndex += nTmpNum;
//    // yfphm
//    nHumanceStrIndex = AppendHumanceString(pInvoiceDecodeBuf + nInvoiceDecodeBufIndex, 8,
//                                           pHumanceStr, nHumanceStrIndex, 0);
//    nInvoiceDecodeBufIndex += 8;
//    // unknow
//    nInvoiceDecodeBufIndex =
//        FormatTemplate(pInvoiceDecodeBuf, nInvoiceDecodeBufIndex, szBuf, sizeof szBuf);
//    // version
//    memset(szBuf, 0, sizeof szBuf);
//    strncpy(szBuf, pInvoiceDecodeBuf + nInvoiceDecodeBufIndex + 2, 4);
//    nVer = strtol(szBuf, 0, 16);  //无清单时,0 == count,qdxm不处理
//    if (nVer < 0x9915)
//        goto Finish;
//    nInvoiceDecodeBufIndex += 10;
//    for (i = 0; i < 25; i++) {
//        nInvoiceDecodeBufIndex =
//            FormatTemplate(pInvoiceDecodeBuf, nInvoiceDecodeBufIndex, szBuf, sizeof szBuf);
//        nHumanceStrIndex =
//            AppendHumanceString(szBuf, strlen(szBuf), pHumanceStr, nHumanceStrIndex, 0);
//    }
//    if (nVer > 0x9915)
//        nInvoiceDecodeBufIndex += 20;
//    nInvoiceDecodeBufIndex += GetExtraHumanceString(
//        false, pInvoiceDecodeBuf + nInvoiceDecodeBufIndex, pHumanceStr, &nHumanceStrIndex);
//    nInvoiceDecodeBufIndex += GetExtraHumanceString(
//        false, pInvoiceDecodeBuf + nInvoiceDecodeBufIndex, pHumanceStr, &nHumanceStrIndex);
//    nInvoiceDecodeBufIndex += GetExtraHumanceString(
//        true, pInvoiceDecodeBuf + nInvoiceDecodeBufIndex, pHumanceStr, &nHumanceStrIndex);
//    // mw
//    nHumanceStrIndex = AppendHumanceString(pInvoiceDecodeBuf + nInvoiceDecodeBufIndex, 0x70,
//                                           pHumanceStr, nHumanceStrIndex, 0);
//    nInvoiceDecodeBufIndex += 0x70;
//    // sign
//    strncpy(szBuf, pInvoiceDecodeBuf + nInvoiceDecodeBufIndex, nSignStrLen);
//    MemoryFormart(MF_TRANSLATE_SPACE2ZERO, szBuf, nSignStrLen);
//    nHumanceStrIndex = AppendHumanceString(szBuf, strlen(szBuf), pHumanceStr, nHumanceStrIndex, 0);
//    nInvoiceDecodeBufIndex += nSignStrLen;
//    // _WriteHexToDebugFile("humanceFP.txt", (uint8 *)pHumanceStr, nHumanceStrIndex);
//#ifdef _CONSOLE
//    char szFile[256] = "";
//    sprintf(szFile, "humanceFP-%s-%s.txt", szFPDM, szFPHM);
//    //_WriteHexToDebugFile(szFile, (uint8 *)pHumanceStr, nHumanceStrIndex);
//#endif
//    nRet = nBlockBinLen + 7;
//Finish:
//    if (pHumanceStr)
//        free(pHumanceStr);
//    if (pInvoiceDecodeBuf)
//        free(pInvoiceDecodeBuf);
//    if (nRet < 0)
//        _WriteLog(LL_FATAL, "NisecGetFPHumanceBuf Error, FPDM:%s FPHM:%s\r\n", szFPDM, szFPHM);
//    return nRet;
//}
//----------------------------------对比可视化代码 finish---------------------------------

int NisecGetTailInvVersion(HDEV hDev, char *szOutVersion)
{
    HUSB hUSB = hDev->hUSB;
    int nRet = ERR_GENERIC, nChildRet = 0;
    uint8 cb[256];
    char szBuf[64] = "";
    char abVersion[3][768];
    if (NisecEntry(hUSB, NOP_OPEN_FULL) < 0)
        return SetLastError(hUSB, ERR_DEVICE_OPEN, "OpenDevice failed");
    do {
        memcpy(cb, "\xfe\x3f", 2);
        // 0500000000 0600000000 0700000000
        sprintf(szBuf, "%sff%s", hDev->szDeviceID, "0500000000");
        Str2Byte(cb + 2, szBuf, strlen(szBuf));
        nChildRet = NisecLogicIO(hUSB, cb, 15, sizeof(cb), NULL, 0);
        if (nChildRet < 20 || nChildRet > 64)
            break;
        cb[nChildRet] = '\0';
        // V2.0.49_ZS_20220630,40.0,1
        if (strlen((char *)cb) < 20 || !strchr((char *)cb, '.') || !strchr((char *)cb, ','))
            break;
        if (GetSplitStringSimple((char *)cb, ",", abVersion, 3) != 3)
            break;
        strcpy(szOutVersion, abVersion[0]);
        nRet = RET_SUCCESS;
    } while (false);
    if (NisecEntry(hUSB, NOP_CLOSE) < 0)
        return ERR_DEVICE_CLOSE;
    return nRet;
}