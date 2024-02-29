/*****************************************************************************
File name:    aisino_getmonitor.c
Description:  用于获取aisino的监控信息和税盘信息获取
Author:       Zako
Version:      1.0
Date:         2020.09
History:
20200920      初始化
20210623      文件标准化更名，加入月度统计信息
*****************************************************************************/
#include "aisino_getmonitor.h"

///////////////////////////////////////////////////////////////////////////////////////////////
//这里暂且不管返回缓冲区大小，暂定缓冲区必须给的足够大；因此返回缓冲区需要给大点
int16 GetInvStockIO(HUSB hUSB, char *szRetBuff)
{
    int16 nRet = 0;
    char cb[512] = "";
    int nRetBuffIndex = 0;
    while (1) {
        memcpy(cb, "\xfe\xff\xcc\xbc\x53\xe1\xe3\x00\x1a\x00", 10);
        int result = AisinoLogicIO(hUSB, (uint8 *)cb, 10, sizeof cb, NULL, 0);
        if (result < 0) {
            nRet = -1;
            break;
        }
        // 0 == result 子项目出错,需要根据 bcontinue来判断是否继续
        if (result > 0) {
            nRet++;
            //最后一个校验位不拷贝
            memcpy(szRetBuff + nRetBuffIndex, cb, result - 1);
            nRetBuffIndex += result - 1;
        }
        uint8 bContinue = cb[result + 1];
        if (!bContinue)
            break;
    }
    return nRet;
}

// oldname:GetInvoiceNum
int GetInvStockBuf(HUSB hUSB, unsigned char *szRetBuff, unsigned char *szRetBuff2)
{
    int ncbLen = 5120;
    uint8 *cb = (uint8 *)calloc(1, ncbLen);
    int nRet = 0;
    while (1) {
        int16 nRetTypeNum = GetInvStockIO(hUSB, (char *)cb);
        if (nRetTypeNum <= 0) {
            nRet = nRetTypeNum;
            *(uint16 *)szRetBuff = 0;
            break;
        }
        memcpy(szRetBuff, &nRetTypeNum, 2);
        if (szRetBuff2 != NULL) {
            memcpy(szRetBuff2, &nRetTypeNum, 2);
            memcpy(szRetBuff2 + 2, cb, nRetTypeNum * 25);
        }
        nRet = GetInvNumLoop(cb, nRetTypeNum, (uint8 *)szRetBuff + 2);
        nRet += 2;
        break;
    }
    free(cb);
    return nRet;
}

//尚未测试完全
//将税盘中从税局获取的但是尚未读入的发票，读取至设备库存,bCheckOrRead = true只检测,否则真实读入
bool ReadUnlockInvoiceFromDevice(HDEV hDev, bool bCheckOrRead)
{
    HUSB hUSB = hDev->hUSB;
    uint8 cb[256] = "", cb2[512] = "", abTransfer[2];
    int result = 0, v8 = 0;
    uint8 v10 = 0;
    bool bHasReadInv = false;  //是否无发票
    bool bCheckParam = memcmp(hDev->szDriverVersion, "SKN6K08L0", 9) >= 0;
    memcpy(cb, "\xfe\xff\xcc\xbc\x53\xe1\xe3\x00\x95\x00", 10);
    if ((result = AisinoLogicIO(hUSB, (uint8 *)cb, 10, sizeof cb, NULL, 0) < 0))
        return -1;
    do {
        v10 = *(cb + v8);
        if (v10 != 0xff && (v10 & 0xF0)) {
            bHasReadInv = true;
            if (bCheckOrRead) {
                ++v8;  //无可读发票时不加会卡死
                continue;
            }

            abTransfer[0] = v8 + 1;
            abTransfer[1] = v8 + 1;
            if (bCheckParam) {
                memcpy(cb2, "\xfe\xff\xcc\xbc\x53\xe1\xe1\x03\x09\x00", 10);
                if ((result = AisinoLogicIO(hUSB, (uint8 *)cb2, 10, sizeof cb2, abTransfer, 2) < 0))
                    break;
            } else {
                memcpy(cb2, "\xfe\xff\xcc\xbc\x53\xe1\xe1\x03\x02\x00", 10);
                if ((result = AisinoLogicIO(hUSB, (uint8 *)cb2, 10, sizeof cb2, abTransfer, 2) < 0))
                    break;
            }
        }
        ++v8;
    } while (v8 < 12);
    return bHasReadInv;
}
///////////////////////////////////////////////////////////////////////////////////////////////

int BuildPreBlock1(HUSB hUSB, uint8 *cb, int ncbLen, char *abPreBlock, uint8 *abDeviceTime)
{
    //这里只是初始化。。。。后面还要再赋值
    abPreBlock[0] = 0x00;
    abPreBlock[1] = 0x00;
    abPreBlock[2] = 0x00;

    memcpy(cb, "\xfe\xff\xcc\xbc\x53\xe1\x1a\x00\x00\x00", 10);
    int result = AisinoLogicIO(hUSB, cb, 10, ncbLen, NULL, 0);
    if (7 != result)
        return -1;
    memcpy(abPreBlock + 6, cb + 4, 2);
    abPreBlock[24] = 0x00;
    memcpy(cb, "\xfe\xff\xcc\xbc\x53\xe1\xe3\x00\x36\x00", 10);
    result = AisinoLogicIO(hUSB, cb, 10, ncbLen, NULL, 0);
    if (result < 0)
        return -2;
    //这是初始化需求字符串最后一个字节，根据某缓冲区第一个字节是否是2,可以是0xff和0xfe两个值，一般是0xff
    if (cb[0] == 0x01 && cb[1] == 0x01)
        abPreBlock[25] = 0xFF;
    else
        abPreBlock[25] = 0xFE;

    memcpy(cb, "\xfe\xff\xcc\xbc\x53\xe1\xe3\x00\x14\x00", 10);
    result = AisinoLogicIO(hUSB, cb, 10, ncbLen, NULL, 0);
    if (result < 128)
        return -3;
    uint8 uByte = cb[1 + 0x7b];
    abPreBlock[8] = (uByte & 0xF) + 10 * (uByte >> 4) - 1;

    char tLockDeadline[5];
    int v48 = 0;
    char *v47 = tLockDeadline + 4;
    do {
        *(_BYTE *)(--v47) = *(_BYTE *)(cb + ++v48 + 123);
    } while (v48 < 4);
    abPreBlock[2] = memcmp(abDeviceTime, tLockDeadline, 4) >= 0;

    char tRepDeadline[5];
    v48 = 0;
    v47 = tRepDeadline + 4;
    do {
        *(_BYTE *)(--v47) = *(_BYTE *)(cb + ++v48 + 136);
    } while (v48 < 4);
    abPreBlock[1] = memcmp(abDeviceTime, tRepDeadline, 4) >= 0;
    return 0;
}

int BuildPreBlock2(uint8 *pOpenCardBin, char *abPreBlock)
{
    uint16 uHeadLen = *(uint16 *)(pOpenCardBin + 3);
    uint8 *pBegin = pOpenCardBin + 5 + uHeadLen;

    signed __int64 v3 = 0;
    WORD2(v3) = *(_WORD *)(pBegin + 3);
    LOBYTE(v3) = ~(unsigned __int8)(HIDWORD(v3) >> 15) & 1;
    *((_BYTE *)abPreBlock + 3) = v3;
    *((_BYTE *)abPreBlock + 4) = BYTE4(v3);
    *((_BYTE *)abPreBlock + 9) = BYTE5(v3) & 0x7F;
    if ((_BYTE)v3 && BYTE4(v3) + ((BYTE5(v3) & 0x7F) << 8) > 0) {
        LODWORD(v3) = 1;
        *((_BYTE *)abPreBlock + 5) = 1;
    } else {
        LODWORD(v3) = 0;
        *((_BYTE *)abPreBlock + 5) = 0;
    }
    return 0;
}

int BuildPreBlock3(HUSB hUSB, uint8 *cb, int ncbLen, char *abPreBlock)
{
    memcpy(cb, "\xfe\xff\xcc\xbc\x53\xe1\x25\x00\x00\x00", 10);
    int result = AisinoLogicIO(hUSB, cb, 10, ncbLen, NULL, 0);
    if (result < 5)
        return -1;
    uint16 v4 = *(uint16 *)cb;
    uint16 v8 = 0;

    memcpy(cb, "\xfe\xff\xcc\xbc\x53\xe1\xe3\x00\x16\x00", 10);
    result = AisinoLogicIO(hUSB, cb, 10, ncbLen, NULL, 0);
    if (result < 5)
        return -2;

    if (HIBYTE(v4) & 0x80)
        v8 = v4 & 0x7FFF;
    else
        v8 = 0;
    *((_BYTE *)abPreBlock + 10) = v8;
    char v9 = HIBYTE(v8);
    char *v10 = (char *)cb;
    *((_BYTE *)abPreBlock + 17) = v9;
    *((_BYTE *)abPreBlock + 11) = *(_BYTE *)(v10 + 1) != 0xff;
    *((_BYTE *)abPreBlock + 12) = *(_BYTE *)(v10 + 2) != 0xff;
    *((_BYTE *)abPreBlock + 13) = *(_BYTE *)(v10 + 3) != 0xff;
    *((_BYTE *)abPreBlock + 14) = *(_BYTE *)(v10 + 4) != 0xff;
    *((_BYTE *)abPreBlock + 15) = *(_BYTE *)(v10 + 5) == 0xff;
    *((_BYTE *)abPreBlock + 16) = 0;
    return 0;
}

int InitDeadLineBuf(HUSB hUSB, uint8 *cb, int ncbLen, char *abPreBlock, char *szRetBuff)
{
    memcpy(szRetBuff, abPreBlock, 9);
    memcpy(szRetBuff + 9, abPreBlock + 10, 8);
    memcpy(szRetBuff + 17, abPreBlock, 9);
    memcpy(szRetBuff + 26, abPreBlock + 18, 8);

    memcpy(cb, "\xfe\xff\xcc\xbc\x53\xe1\xe3\x00\x43\x00", 10);
    int result = AisinoLogicIO(hUSB, cb, 10, ncbLen, NULL, 0);
    if (result < 5)
        return -1;
    memcpy(szRetBuff + 34, cb, 5);
    uint8 *pExtra = (uint8 *)szRetBuff + 39;
    int i = 0;
    for (i = 0; i < 0x34 * 4; i++) {
        pExtra[i] = 0x00;
    }
    for (i = 0; i < 8; i++) {
        pExtra[i * 26] = 0xFF;
    }
    return 0;
}

int GetLockDeadline(HUSB hUSB, uint8 *cb, int ncbLen, char *pExtra)
{
    memcpy(cb, "\xfe\xff\xcc\xbc\x53\xe1\xe3\x00\x0d\x00", 10);
    uint8 bFlag = *pExtra;
    char cbTransfer[2];
    memcpy(cbTransfer, &bFlag, 1);
    memcpy(cbTransfer + 1, &bFlag, 1);
    int result = AisinoLogicIO(hUSB, cb, 10, ncbLen, (uint8 *)cbTransfer, sizeof cbTransfer);
    if (result != 5)
        return -1;

    int v53 = 0;
    BYTE *v49 = (BYTE *)cb;
    int v50 = (*(_BYTE *)v49 & 0xF) + 10 * ((unsigned int)*(_BYTE *)v49 >> 4);
    int v51 = (*(_BYTE *)(v49 + 1) & 0xF) + 10 * ((unsigned int)*(_BYTE *)(v49 + 1) >> 4);
    int v52 =
        (*(_BYTE *)(v49 + 2) & 0xF) +
        10 * (((unsigned int)*(_BYTE *)(v49 + 2) >> 4) +
              10 * ((*(_BYTE *)(v49 + 3) & 0xF) + 10 * ((unsigned int)*(_BYTE *)(v49 + 3) >> 4)));
    if (v50 > sub_41D9EE0(v52, v51)) {
        if (v51 == 12) {
            ++v52;
            v53 = 1;
        } else {
            v53 = v51 + 1;
        }
        char szLockDeadLine[4];
        szLockDeadLine[0] = sub_41D5430(1);
        szLockDeadLine[1] = sub_41D5430(v53);
        szLockDeadLine[2] = sub_41D5430(v52 % 100);
        szLockDeadLine[3] = sub_41D5430(v52 / 100);
        int i = 0;
        for (i = 0; i < 4; i++) {
            pExtra[0x15 - i] = szLockDeadLine[i];
        }
    } else {
        int i = 0;
        for (i = 0; i < 4; i++) {
            pExtra[0x15 - i] = cb[i];
        }
    }
    return 0;
}

int FillLockDeadlineBuf_Raw(HUSB hUSB, uint8 *cb, int ncbLen, char *pExtra, uint8 *abDeviceTime)
{
    if (GetLockDeadline(hUSB, cb, ncbLen, pExtra) < 0)
        return -1;
    uint8 bFlag = *pExtra;
    char cbTransfer[2];
    memcpy(cbTransfer, &bFlag, 1);
    memcpy(cbTransfer + 1, &bFlag, 1);

    memcpy(cb, "\xfe\xff\xcc\xbc\x53\xe1\xe3\x00\x13\x00", 10);
    int result = AisinoLogicIO(hUSB, cb, 10, ncbLen, (uint8 *)cbTransfer, sizeof cbTransfer);
    if (result != 5)
        return -1;
    int i = 0;
    for (i = 0; i < 4; i++) {
        pExtra[0x11 - i] = cb[i];
    }
    memcpy(cb, "\xfe\xff\xcc\xbc\x53\xe1\xe3\x00\x12\x00", 10);
    result = AisinoLogicIO(hUSB, cb, 10, ncbLen, (uint8 *)cbTransfer, sizeof cbTransfer);
    if (result != 7)
        return -1;
    for (i = 0; i < 6; i++) {
        pExtra[0x0D - i] = cb[i];
    }
    pExtra[2] = memcmp(abDeviceTime, pExtra + 18, 4) >= 0;
    pExtra[1] = memcmp(abDeviceTime, pExtra + 14, 4) >= 0;
    return 0;
}

void FillLockDeadLineBuf(HUSB hUSB, uint8 *cb, int ncbLen, char *szRetBuff, uint8 *abDeviceTime)
{
    uint8 *pExtra = (uint8 *)szRetBuff + 39;
    int i = 0;
    for (i = 0; i < 8; i++) {
        uint8 *pExtraPerBlock = pExtra + i * 26;
        uint8 bFlag = *pExtraPerBlock;
        if (0xff == bFlag)
            continue;
        FillLockDeadlineBuf_Raw(hUSB, cb, ncbLen, (char *)pExtraPerBlock, abDeviceTime);
    }
}

// from szRetBuff 500
int PatchInvLimit(uint8 bInvType, uint8 bInvIndex, char *szChildRetBuff)
{
    uint8 dicData[56] = {0xFF, 0xE7, 0x76, 0x48, 0x17, 0x00, 0x00, 0x00, 0x9F, 0x86, 0x01, 0x00,
                         0x00, 0x00, 0x00, 0x00, 0x3F, 0x42, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x00,
                         0x7F, 0x96, 0x98, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xE0, 0xF5, 0x05,
                         0x00, 0x00, 0x00, 0x00, 0xFF, 0xC9, 0x9A, 0x3B, 0x00, 0x00, 0x00, 0x00,
                         0xFF, 0xE3, 0x0B, 0x54, 0x02, 0x00, 0x00, 0x00};
    int nIndex = bInvIndex % 7;
    uint8 *pIndexBuff = nIndex * 8 + dicData;
    szChildRetBuff[0] = bInvType;
    memcpy(szChildRetBuff + 1, pIndexBuff, 6);
    return 0;
}

void PriceInt2DoubleStr(uint64 u64Price, char *szOut)
{
    char szBuf[256];
    uint64 nMod = u64Price % (uint64)0x1000000000000;
    if (!nMod)
        return;
    sprintf(szBuf, "%03llu", nMod);
    int nLen = strlen(szBuf);
    memcpy(szOut, szBuf, nLen - 2);
    szOut[nLen - 2] = '\0';
    strcat(szOut, ".");
    strcat(szOut, szBuf + nLen - 2);
}

// szOutStrBuf > 768
void GetNewInvoiceLimit(uint8 *byte_3, char *szOutStrBuf)
{
    uint16 num8 = *(uint16 *)byte_3;
    byte_3 += 2;
    int k = 0;
    char szBuf[128] = "";
    char text4[768] = "";
    if (num8 > 0) {
        uint8 b2 = 0;
        for (k = 0; k < (int)(num8 - 1); k++) {
            b2 += *(uint8 *)(byte_3 + k);
        }
        if (b2 == *(uint8 *)(byte_3 + (int)num8 - 1)) {
            int num9 = 0;
            do {
                sprintf(szBuf, "%d-", *(byte_3 + num9));
                strcat(text4, szBuf);
                uint64 num10 = *(uint64 *)(byte_3 + num9 + 1);
                PriceInt2DoubleStr(num10, szBuf);
                strcat(text4, szBuf);
                strcat(text4, "-");

                num10 = *(uint64 *)(byte_3 + num9 + 7);
                PriceInt2DoubleStr(num10, szBuf);
                strcat(text4, szBuf);
                strcat(text4, "-");

                num10 = *(uint64 *)(byte_3 + num9 + 13);
                PriceInt2DoubleStr(num10, szBuf);
                strcat(text4, szBuf);
                strcat(text4, "-");

                uint8 b3 = *(uint8 *)(byte_3 + num9 + 19);
                sprintf(szBuf, "%d-", b3);
                strcat(text4, szBuf);
                int l = 0;
                for (l = 0; l < (int)b3; l++) {
                    uint32 num11 = *(uint32 *)(byte_3 + num9 + 20 + l * 6);
                    uint32 num11B = num11 % 16777216;
                    sprintf(szBuf, "%06d", num11B);
                    strcat(text4, szBuf);
                    strcat(text4, "-");

                    num11 = *(uint32 *)(byte_3 + num9 + 20 + l * 6 + 3);
                    double num11Bf = (double)(num11 % 16777216) / 1000;
                    sprintf(szBuf, "%0.2lf", num11Bf);
                    strcat(text4, szBuf);
                    strcat(text4, "-");
                }
                strcat(text4, ",");
                num9 = num9 + 20 + (int)(6 * b3);
            } while (num9 < (int)(num8 - 1));
        }
    }
    strcpy(szOutStrBuf, text4);
}

int FillFPLimitInfoBuf(uint8 *pOpenCardBin, char *szRetBuff, char *szOutNewInvoiceLimitBuf)
{
    uint8 *pIndex = pOpenCardBin + 2;
    uint8 *pExtra = (uint8 *)szRetBuff + 39;
    int nExtraIndex = 0;
    while (*pIndex != 0xff) {
        uint16 uHeadLen = *(uint16 *)(pIndex + 1);
        uint8 bFlag = *pIndex;
        if (uHeadLen) {
            if (0x0b == bFlag) {
                uint8 *pChildIndex = pIndex;
                while (1) {
                    uint8 bInvType = *(pChildIndex + 3);
                    pExtra[nExtraIndex * 26] = bInvType;
                    uint8 bInvLimitIndex = *(pChildIndex + 4);
                    memcpy(szRetBuff + 256 + nExtraIndex * 20, pChildIndex + 3, 20);
                    if (sub_476AE90(bInvType) == 1)
                        PatchInvLimit(bInvType, bInvLimitIndex, szRetBuff + 256 + nExtraIndex * 20);

                    pChildIndex += 0x14;
                    if (pChildIndex - pIndex >= uHeadLen)
                        break;
                    nExtraIndex++;
                }
            } else if (0x6 == bFlag) {
                GetNewInvoiceLimit(pIndex + 1, szOutNewInvoiceLimitBuf);
            }
        }
        pIndex = pIndex + 3 + uHeadLen;
    }
    return 0;
}

// szRetBuff's length > 1024
int GetStateInfoBuf(HUSB hUSB, char *szRetBuff)
{
    uint8 abPreBlock[64] = "", cb[2048] = {0};
    int nRet = -1, ncbLen = sizeof(cb), result;
    uint8 *pOpenCardBin = calloc(1, 10240);
    uint8 abDeviceTimeNow[16] = "";
    while (1) {
        // devicetime
        if (AisinoGetDeivceTime(hUSB, (char *)cb) < 0) {
            nRet = -1;
            break;
        }
        Str2Byte(abDeviceTimeNow, (char *)cb, strlen((char *)cb));
        // opencardstr
        memcpy(pOpenCardBin, "\xfe\xff\xcc\xbc\x53\xe1\x40\x00\x00\x00", 10);
        uint8 abTransferData[] = {0x00, 0x10, 0x10};
        result =
            AisinoLogicIO(hUSB, pOpenCardBin, 10, 10240, abTransferData, sizeof abTransferData);
        if (result < 256) {
            nRet = -1;
            break;
        }
        if (BuildPreBlock1(hUSB, cb, ncbLen, (char *)abPreBlock, abDeviceTimeNow) < 0) {
            nRet = -2;
            break;
        }
        if (BuildPreBlock2(pOpenCardBin, (char *)abPreBlock) < 0) {
            nRet = -3;
            break;
        }
        if (BuildPreBlock3(hUSB, cb, ncbLen, (char *)abPreBlock) < 0) {
            nRet = -4;
            break;
        }
        if (InitDeadLineBuf(hUSB, cb, ncbLen, (char *)abPreBlock, szRetBuff) < 0) {
            nRet = -5;
            break;
        }
        if (FillFPLimitInfoBuf(pOpenCardBin, szRetBuff, (char *)cb) < 0) {
            nRet = -6;
            break;
        }
        // 500偏移为自定义数据,非官方数据,目的是把数据带出去让调用者处理
        strcpy(szRetBuff + 500, (char *)cb);
        // 39 + 208 + space(9) + 160 + space(4)
        FillLockDeadLineBuf(hUSB, cb, ncbLen, szRetBuff, abDeviceTimeNow);

        nRet = 0;
        break;
    }
    free(pOpenCardBin);
    return nRet;
}

struct stFPXE *FindStateFPXEByType(uint8 bInvType, uint8 bStaticMaxTypeCount, StateInfo *pSi)
{
    int i = 0;
    struct stFPXE *pRet = NULL;
    for (i = 0; i < bStaticMaxTypeCount; i++) {
        if (bInvType != pSi->fpType[i].bFPLX)
            continue;
        pRet = &pSi->fpType[i];
    }
    return pRet;
}

int NewDataModify(StateInfo *pRetObj, char *pNewCfgStr, int bStaticMaxTypeCount)
{
    SplitInfo spOut;
    memset(&spOut, 0, sizeof(SplitInfo));

    GetSplitString(pNewCfgStr, ",", &spOut);
    SplitInfo *pOut = &spOut;
    int i, k;
    for (i = 0; i < spOut.num; i++) {
        pOut = pOut->next;
        char *pChild = pOut->info;

        SplitInfo spOutChild;
        memset(&spOutChild, 0, sizeof(spOutChild));
        GetSplitString(pChild, "-", &spOutChild);
        SplitInfo *pOutchild = &spOutChild;
        char szFPLX[16], szLimit1[64], szLimit2[64], szLimit3[64];
        for (k = 0; k < spOutChild.num; k++) {
            pOutchild = pOutchild->next;
            char *pChildChild = pOutchild->info;
            switch (k) {
                case 0: {  // fplx
                    strcpy(szFPLX, pChildChild);
                    break;
                }
                case 1: {
                    strcpy(szLimit1, pChildChild);
                    break;
                }
                case 2: {
                    strcpy(szLimit2, pChildChild);
                    break;
                }
                case 3: {
                    strcpy(szLimit3, pChildChild);
                    break;
                }
            }
        }
        SplitFree(&spOutChild);
        //
        uint8 bInvType = atoi(szFPLX);
        struct stFPXE *pxe = FindStateFPXEByType(bInvType, bStaticMaxTypeCount, pRetObj);
        if (!pxe)
            continue;
        strcpy(pxe->szInvAmountLimit, szLimit1);     // szInvAmountLimit
        strcpy(pxe->szMonthAmountLimit, szLimit2);   // MonthAmountLimit
        strcpy(pxe->szReturnAmountLimit, szLimit3);  // ReturnAmountLimit
    }
    SplitFree(&spOut);
    return 0;
}

int AisinoGetOfflineAmount(HUSB hUSB, uint8 bInvType, char *szOutInvTotalAmount,
                           char *szOutInvAmount)
{
    uint8 cb[64], abTransferData[16] = {0};
    int ncbLen = sizeof cb;
    memcpy(cb, "\xfe\xff\xcc\xbc\x53\xe1\xe3\x00\x47\x00", 10);
    abTransferData[0] = bInvType;
    abTransferData[1] = bInvType;  // auth code
    int nRet = AisinoLogicIO(hUSB, cb, 10, ncbLen, abTransferData, 2);
    if (nRet < 16)
        return -1;
    //月离线金额
    uint64 u64Amount = *(uint64 *)cb;
    if (u64Amount == 0)  //为0时处理有问题，此处补偿
        strcpy(szOutInvTotalAmount, "0.00");
    else
        PriceInt2DoubleStr(u64Amount, szOutInvTotalAmount);
    //月已开金额
    u64Amount = *(uint64 *)(cb + 6);
    if (u64Amount == 0)  //为0时处理有问题，此处补偿
        strcpy(szOutInvAmount, "0.00");
    else
        PriceInt2DoubleStr(u64Amount, szOutInvAmount);
    //已开税额
    // u64Amount = *(uint64 *)(cb + 12);
    return 0;
}

int GetStateInfo(HUSB hUSB, StateInfo *pRetObj)
{
    memset(pRetObj, 0, sizeof(StateInfo));
    uint8 bTypeCount = 0, bStaticMaxTypeCount = STATEINFO_FPXE_MAXCOUNT, bInvType;
    uint8 abInvBuf[256], abRawData[1024] = "";
    int i, nIndex;
    char szInvTotalAmount[64], szInvAmount[64];

    if (GetStateInfoBuf(hUSB, (char *)abRawData) < 0) {
        _WriteLog(LL_FATAL, "Get stateinfo buf failed");
        return -1;
    }
    //_WriteHexToDebugFile("stateinfo.bin", abRawData, 1024);
    pRetObj->IsLockReached = abRawData[1];
    pRetObj->IsRepReached = abRawData[2];
    pRetObj->LockedDays = abRawData[8];
    pRetObj->ICCardNo = abRawData[9];

    //wang 05 04
    uint8 szRegCodepin[2];

	//logout(INFO, "TAXLIB", "地区编号", "读出来地区编号为 %02x%02x%02x\r\n", abRawData[36], abRawData[37], abRawData[38]);
    
	//printf("yuan abRawData[36] = %x\r\n",abRawData[36]);
    //printf("pRetObj->szRegCode = %02x%02x%02x", abRawData[36], abRawData[37], abRawData[38]);

    if((abRawData[36] == 0x91) || (abRawData[36] == 0x99)){          //重庆，单独处理
        abRawData[36] = 0x50;
    }
    else{
        if((abRawData[36]&0x0f) > 0x08){
            szRegCodepin[0] = (abRawData[36]&0x0f)%8;
        }else{
            szRegCodepin[0] = (abRawData[36]&0x0f);
        }
        if((abRawData[36]&0xf0) > 0x80){
            szRegCodepin[1] = (abRawData[36]&0xf0)%0x80;
        }else{
            szRegCodepin[1] = abRawData[36]&0xf0;
        }
        if((szRegCodepin[0]!=0)||(szRegCodepin[1]!=0)){
            abRawData[36] = szRegCodepin[0] + szRegCodepin[1];
            printf("abRawData[36] = %x\r\n",abRawData[36]);
        }
    }

	sprintf(pRetObj->szRegCode, "%02x%02x%02x", abRawData[36], abRawData[37], abRawData[38]);
	//logout(INFO, "TAXLIB", "地区编号", "解密后地区编号为 %02x%02x%02x\r\n", abRawData[36], abRawData[37], abRawData[38]);

    for (i = 0; i < bStaticMaxTypeCount; i++) {
        uint8 *pType = abRawData + 39 + i * 26;
        struct stFPXE *pxe = &(pRetObj->fpType[i]);
        pxe->bFPLX = 0xff;
        if (*pType == 0xff)
            continue;
        pRetObj->abInvTypeList[1 + bTypeCount] = *pType;
        pxe->bFPLX = *pType;
        bTypeCount++;
        memcpy(abInvBuf, abRawData + 39 + i * 26, 26);
        //逐个发票解析
        sprintf(pxe->szBsDeadline, "%02x%02x%02x%02x", abInvBuf[14], abInvBuf[15], abInvBuf[16],
                abInvBuf[17]);
        sprintf(pxe->szKpDeadline, "%02x%02x%02x%02x", abInvBuf[18], abInvBuf[19], abInvBuf[20],
                abInvBuf[21]);
        sprintf(pxe->szBsLastestDate, "%02x%02x%02x%02x", abInvBuf[8], abInvBuf[9], abInvBuf[10],
                abInvBuf[11]);
        sprintf(pxe->szBsBeginDate, "%02x%02x%02x%02x", abInvBuf[8], abInvBuf[9], abInvBuf[10],
                abInvBuf[11]);
    }
    pRetObj->abInvTypeList[0] = bTypeCount;

    uint64 u64Num = 0;
    for (i = 0; i < bStaticMaxTypeCount; i++) {
        nIndex = i * 20;
        bInvType = abRawData[256 + nIndex];
        //单张开票限额
        u64Num = *(uint64 *)(abRawData + 256 + nIndex + 1);
        if (u64Num == 0)
            continue;  //空白，未设置，跳入下一个

        struct stFPXE *pxe = FindStateFPXEByType(bInvType, bStaticMaxTypeCount, pRetObj);
        if (!pxe)
            continue;
        PriceInt2DoubleStr(u64Num, pxe->szInvAmountLimit);
        // 离线剩余金额
        u64Num = *(uint64 *)(abRawData + 256 + nIndex + 7);
        PriceInt2DoubleStr(u64Num, pxe->szMonthAmountLimit);
        // 离线金额
        u64Num = *(uint64 *)(abRawData + 256 + nIndex + 13);
        PriceInt2DoubleStr(u64Num, pxe->szOffLineAmoutLimit);
    }
    //票种数据限额校正
    char *pNewConfigStr = (char *)abRawData + 500;
    NewDataModify(pRetObj, pNewConfigStr, bStaticMaxTypeCount);
    //税盘API数据限额校正
    for (i = 0; i < bStaticMaxTypeCount; i++) {
        uint8 bInvType = *(uint8 *)(abRawData + 39 + i * 26);
        if (bInvType == 0xff)
            continue;
        struct stFPXE *pxe = FindStateFPXEByType(bInvType, bStaticMaxTypeCount, pRetObj);
        if (!pxe)
            continue;
        AisinoGetOfflineAmount(hUSB, bInvType, szInvTotalAmount, szInvAmount);
        strcpy(pxe->szOffLineAmoutLimit, szInvTotalAmount);
        double dSurplus = atof(szInvTotalAmount) - atof(szInvAmount);
        sprintf(pxe->szOffLineAmoutLimit_Surplus, "%0.2lf", dSurplus);
    }
    return 0;
}

//------------------------------------获取月度统计开始----------------------------------------
void MonthStatisticRaw2Struct(uint8 *pBuf, struct StMonthStatistics *pstStatisticsRet,
                              char *szFormatYearMonth)
{
    struct StMonthStatistics *node = pstStatisticsRet;
    uint32 nTmpNum = 0;
    char szReserve[256] = "";
    int i = 0;
    for (i = 0; i < 8; i++) {
        uint8 *pBlockAddr = pBuf + i * 121;
        if (*pBlockAddr == 0xff)
            continue;
        uint8 fplx = *(uint8 *)pBlockAddr;
        if (fplx > FPLX_AISINO_DZZP)
            continue;
        pBlockAddr += 1;
        // new node
        if (i) {
            node->next = calloc(1, sizeof(struct StMonthStatistics));
            node = node->next;
        }
        node->fplx = fplx;
        sprintf(node->qssj, "%s00", szFormatYearMonth);
        sprintf(node->jzsj, "%s00", szFormatYearMonth);
        //
        //份数统计
        //
        //期初库存
        nTmpNum = *(uint32 *)(pBlockAddr);
        sprintf(node->qckcfs, "%d", nTmpNum);
        //购进发票份数
        nTmpNum = *(uint32 *)(pBlockAddr + 4);
        sprintf(node->lgfpfs, "%d", nTmpNum);
        //期末库存份数
        nTmpNum = *(uint32 *)(pBlockAddr + 8);
        sprintf(node->qmkcfs, "%d", nTmpNum);
        //退回发票份数
        nTmpNum = *(uint32 *)(pBlockAddr + 12);
        sprintf(node->thfpfs, "%d", nTmpNum);
        //分配发票份数 reserve
        nTmpNum = *(uint32 *)(pBlockAddr + 16);
        sprintf(szReserve, "%d", nTmpNum);
        //收回发票份数 reserve
        nTmpNum = *(uint32 *)(pBlockAddr + 20);
        sprintf(szReserve, "%d", nTmpNum);
        //正数发票份数
        nTmpNum = *(uint32 *)(pBlockAddr + 24);
        sprintf(node->zsfpfs, "%d", nTmpNum);
        //正废发票份数
        nTmpNum = *(uint32 *)(pBlockAddr + 28);
        sprintf(node->zffpfs, "%d", nTmpNum);
        //负数发票份数
        nTmpNum = *(uint32 *)(pBlockAddr + 32);
        sprintf(node->fsfpfs, "%d", nTmpNum);
        //负废发票份数
        nTmpNum = *(uint32 *)(pBlockAddr + 36);
        sprintf(node->fffpfs, "%d", nTmpNum);
        //
        //金额统计
        //
        //正数发票金额
        strncpy(node->zsfpljje, (char *)pBlockAddr + 40, 20);
        //正数发票税额
        strncpy(node->zsfpljse, (char *)pBlockAddr + 60, 20);
        //负数发票金额
        strncpy(node->fsfpljje, (char *)pBlockAddr + 80, 20);
        //负数发票税额
        strncpy(node->fsfpljse, (char *)pBlockAddr + 100, 20);

        //_WriteLog(LL_DEBUG,
        //          "fplx:%d qssj:%s jzsj:%s qckcfs:%s lgfpfs:%s thfpfs:%s zsfpfs:%s zffpfs:%s "
        //          "fsfpfs:%s fffpfs:%s "
        //          "kffpfs:%s qmkcfs:%s zsfpljje:%s zsfpljse:%s zffpljje:%s zffpljse:%s "
        //          "fsfpljse:%s fffpljje:%s fffpljse:%s",
        //          node->fplx, node->qssj, node->jzsj, node->qckcfs, node->lgfpfs, node->thfpfs,
        //          node->zsfpfs, node->zffpfs, node->fsfpfs, node->fffpfs, node->kffpfs,
        //          node->qmkcfs, node->zsfpljje, node->zsfpljse, node->zffpljje, node->zffpljse,
        //          node->fsfpljse, node->fffpljje, node->fffpljse);
    }
}

int AisinoGetMonthStatistic(HUSB hUSB, char *szFormatYearMonth,
                            struct StMonthStatistics *pstStatisticsRet)
{
    uint8 cb[1280] = {0}, abApiBuf[1280] = {0}, abTransferData[16] = {0};
    if (strlen(szFormatYearMonth) != 6)
        return -1;
    uint32 nCentury, nYear, nMonth;
    int nChildRet = 0;
    if ((nChildRet = sscanf(szFormatYearMonth, "%2x%2x%2x", (unsigned int *)&nCentury,
                            (unsigned int *)&nYear, (unsigned int *)&nMonth)) != 3)
        return -2;
    GetQueryMonthFormat(nCentury, nYear, nMonth, 2, (char *)abTransferData);
    memcpy(cb, "\xfe\xff\xcc\xbc\x53\xe1\xe3\x01\x00\x00", 10);
    if ((nChildRet = AisinoLogicIO(hUSB, cb, 10, sizeof(cb), abTransferData, 4)) < 64)
        return -3;
    // _WriteHexToDebugFile("asisino_statistic.bin", cb, nRet);
    if ((nChildRet = RestoreAisinoMonthStatisticApiBuf(cb, abApiBuf, sizeof(abApiBuf))) < 0)
        return -4;
    if (nChildRet > 1009)
        return -5;
    struct StMonthStatistics *node = pstStatisticsRet;
    memset(node, 0, sizeof(struct StMonthStatistics));
    MonthStatisticRaw2Struct(abApiBuf, node, szFormatYearMonth);
    return 0;
}