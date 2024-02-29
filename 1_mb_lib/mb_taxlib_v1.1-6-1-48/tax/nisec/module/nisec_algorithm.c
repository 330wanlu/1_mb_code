/*****************************************************************************
File name:   nisec_algorithm.c
Description: 用于nisec税盘的所有算法
Author:      Zako
Version:     1.0
Date:        2019.09
History:
201912     代码初始化
*****************************************************************************/
#include "nisec_algorithm.h"

void Num2FloatS(uint64 nNum, char *szRet)
{
    double fNum = (double)nNum / 100;
    sprintf(szRet, "%0.2lf", fNum);
}

//输入字符串前后有空格都没问题
//注意:该函数最后是memcpy到dst缓冲，因此最后不会加\0，需要事先预判缓冲区是否需要清0
int NisecBaseConv(const char *szInputStr, int nInputLen, uint8 bDstBase, char *szRetBuff,
                  uint8 bOrigBase, uint8 bRetFormatLen)
{
    // pFunc(bT, 0x24, szDst, 0x10, 0x1a); //913201234567891 -> 0000000BBE2476A3EAAD611F85
    // pFunc(bT, 0x10, szDst, 0x24, 15);//0000000BBE2476A3EAAD611F85 -> 913201234567891
    // pFunc(bT, 0xa, szDst, 0x10, 0); //200150000001 -> 2E99DEA181
    // pFunc(bT, 0x16, szDst, 0xa, 0xa); //ffffffffff -> 0995116277
    // pFunc(bT, 0x10, szDst, 0x24, 0); //0000002D838A4E8FDB2E340000 -> Z00011000000000
    _BYTE *lpMem;
    unsigned int v21;
    uint8 *v22;
    unsigned int v23;
    unsigned int v24;
    uint8 *v25;
    unsigned int v26;
    unsigned int v60;
    unsigned int v27;
    sint8 v28;
    uint8 *v29;
    sint8 v30;
    uint8 v31;
    unsigned int v32;
    void *v33 = NULL;
    unsigned int v34;
    uint8 *v35;
    uint8 *v36;
    int v37;
    int v38;
    unsigned int *v39;
    int v40;
    unsigned int v63;
    unsigned int *v64;
    uint8 *v70;
    uint8 v71;
    unsigned int v41;
    unsigned int v42;
    _DWORD *v43;
    bool v45;
    void *v59;
    unsigned int v61;
    uint8 vMemBlock[768] = "";
    char szDstTmpBuf[768] = "";
    if (nInputLen > (sizeof(vMemBlock) - 64))
        return -1;
    memcpy(vMemBlock + 64, szInputStr, nInputLen);  //空出前面的缓冲给程序，防止越界内存操作
    uint8 *v66 = vMemBlock + 64;
    unsigned int v9 = strlen((char *)v66);
    signed int v13;
    int nRetIndex = 0;
    uint8 v68[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    unsigned int v67 = v9;
    uint8 unk_3AE3B8 = 0;
    sint8 v14;
    _BYTE *v18;
    _BYTE *v19;
    uint8 v20;
    unsigned int v10;

LABEL_13:
    v10 = v67;
    uint8 *v11 = (uint8 *)v66;
    _BYTE *v12;
    while (v67 > 0) {
        if (v11) {
            v11 = (uint8 *)v66;
            v10 = v67;
            v12 = v66;
        } else {
            v12 = &unk_3AE3B8;
        }
        if (*v12 != 48)
            break;
        v13 = 1;
        if (v11) {
            v14 = *(v11 - 1);
            if (v14) {
                if (v14 != -1) {
                    // v15 = v11;
                    v10 = v67;
                    v11 = (uint8 *)v66;
                    //这里缺个函数;内存越界处理函数，在输入缓冲区之前，一般不会用到，实现太复杂，暂时不考虑
                }
            }
        }
        if (v10 < 1) {
            v13 = v10;
            if (!v10)
                continue;
        }
        // memcpy(v11, &v11[v13], v10 - v13);
        memmove(v11, v11 + v13, v10 - v13);
        //这里缺个函数;
        // v16 = v67 - v13;
        v67--;
        goto LABEL_13;
    }
    if (!v10) {  //原封不动拷贝
        memcpy(szRetBuff, szInputStr, nInputLen);
        return 0;
    }
    v18 = malloc(v10 + 1);
    v19 = v18;
    v20 = v67 + 1;
    lpMem = v18;
    v21 = (v67 + 1) >> 2;
    memset(v18, 0, 4 * v21);
    v22 = &v18[4 * v21];
    LOBYTE(v21) = v20;
    v23 = 0;
    v60 = 0;
    memset(v22, 0, v21 & 3);
    v24 = v67;
    v25 = (uint8 *)v66;
    v26 = 0;
    do {
        v27 = v24 - v23 - 1;
        if (v67 >= v27 && v25) {
            v28 = *(v25 - 1);
            if (v28 && v28 != -1) {
                //这里缺个函数;
                v25 = (uint8 *)v66;
                v23 = v60;
            }
            if (v25) {
                *(v25 - 1) = -1;
                v25 = (uint8 *)v66;
            }
            v29 = &v25[v27];
        } else {
            v29 = &unk_3AE3B8;
        }
        v30 = *v29;
        if (v30 < 48 || v30 > 57) {
            if (v30 < 97 || v30 > 122) {
                if (v30 < 65 || v30 > 90)
                    goto LABEL_53;
                v31 = v30 - 55;
            } else {
                v31 = v30 - 87;
            }
        } else {
            v31 = v30 - 48;
        }
        v19[v26] = v31;
        if (v31 < (uint8)bDstBase) {
            ++v26;
        }
        v25 = (uint8 *)v66;
    LABEL_53:
        v24 = v67;
        v60 = ++v23;
    } while (v23 < v67);
    v63 = v26;
    if (v26) {
        v32 = v26 / 5 + 1;
        v33 = malloc(4 * v32 + 4);
        v59 = v33;
        memset(v33, 0, 4 * v32 + 4);
        v34 = 0;
        if (v63) {
            do {
                switch (v34 % 5) {
                    case 1u:
                        v35 = (uint8 *)v33 + 4 * (v32 - v34 / 5) - 4;
                        *(_DWORD *)v35 += (uint8)bDstBase * lpMem[v34];
                        break;
                    case 2u:
                        v36 = (uint8 *)v33 + 4 * (v32 - v34 / 5) - 4;
                        *(_DWORD *)v36 += (uint8)bDstBase * (uint8)bDstBase * lpMem[v34];
                        break;
                    case 3u:
                        v70 = (uint8 *)v33 + 4 * (v32 - v34 / 5) - 4;
                        v37 = (uint8)bDstBase;
                        v38 = lpMem[v34];
                        goto LABEL_62;
                    case 4u:
                        v70 = (uint8 *)v33 + 4 * (v32 - v34 / 5) - 4;
                        v37 = (uint8)bDstBase;
                        v38 = (uint8)bDstBase * lpMem[v34];
                    LABEL_62:
                        *(_DWORD *)v70 += v37 * v37 * v37 * v38;
                        break;
                    default:
                        v33 = v59;
                        *((_DWORD *)v59 + v32 - v34 / 5 - 1) = lpMem[v34];
                        break;
                }
                ++v34;
            } while (v34 < v63);
        }
        v61 = 0;
        if (v32) {
            v64 = (unsigned int *)v33;
            do {
                if (*v64) {
                    do {
                        if (v61 >= v32 - 1) {
                            v43 = v59;
                        } else {
                            v39 = v64;
                            v40 = v32 - 1 - v61;
                            do {
                                v41 = *v39 % bOrigBase;
                                v42 = v39[1];
                                ++v39;
                                *v39 = (uint8)bDstBase * (uint8)bDstBase * (uint8)bDstBase *
                                           (uint8)bDstBase * (uint8)bDstBase * v41 +
                                       v42;
                                --v40;
                                *(v39 - 1) /= bOrigBase;
                            } while (v40);
                            v43 = v59;
                        }
                        v71 = v68[v43[v32 - 1] % (unsigned int)bOrigBase];
                        szDstTmpBuf[nRetIndex++] = v71;
                        v43[v32 - 1] /= (unsigned int)bOrigBase;
                    } while (*v64);
                    v33 = v59;
                }
                v45 = v61++ + 1 < v32;
                ++v64;
            } while (v45);
        }
    }
    //不能直接操作dstbuf，有可能dstbuf未清空
    if (bRetFormatLen != 0) {
        int nDstTmpLen = strlen(szDstTmpBuf);
        if (nDstTmpLen < bRetFormatLen) {
            int nNum = bRetFormatLen - strlen(szDstTmpBuf);
            int i = 0;
            for (i = 0; i < nNum; i++) {
                strcat(szDstTmpBuf, "0");
            }
        } else if (nDstTmpLen == bRetFormatLen) {
        } else {  //>;
            // 20201106 该函数为整个nisec转码核心，此处修改主要是为了配合官方进行修改
            //此处部分调整，或许会对实际有影响，加入观察
            szDstTmpBuf[nDstTmpLen - 1] = '\0';
            nDstTmpLen -= 1;
            char *pTmp = (char *)calloc(1, bRetFormatLen + 1);
            strncpy(pTmp, szDstTmpBuf + nDstTmpLen - bRetFormatLen, bRetFormatLen);
            strcpy(szDstTmpBuf, pTmp);
            free(pTmp);
        }
    }
    ByteReverse(szDstTmpBuf, strlen(szDstTmpBuf));
    memcpy(szRetBuff, szDstTmpBuf, strlen(szDstTmpBuf));
    if (NULL != v18)
        free(v18);
    if (NULL != v33)
        free(v33);
    return strlen(szRetBuff);
}
//-----
//返回数字为原数字的100倍,需要转浮点数除以100
int64 NisecDecodeNum(const char *pInputBuf, int nInbufLenMax)
{
    //该函数所有的调用将平移至NisecBaseConv函数
    char szBuf[256] = "";
    NisecBaseConv(pInputBuf, nInbufLenMax, 16, szBuf, 10, 0);
    return atoll(szBuf);
}

//返回值为读取的buf长度
// nMethod==0 整数转换；==1 2位小数点转换;==2 5位小数点转换(税率中使用)
int NisecFormatStr2Num(int nMethod, const char *pInputBuf, int nInbufLenMax, char *szOutBuf)
{
    int64 i64Num = NisecDecodeNum(pInputBuf, nInbufLenMax);
    if (0 == nMethod) {  //原型输出
        sprintf(szOutBuf, "%lld", i64Num);
    } else if (1 == nMethod) {  // 2位小数点转换
        double dNum = (double)i64Num / 100;
        if (0 != dNum)
            sprintf(szOutBuf, "%0.2lf", dNum);
        else
            strcpy(szOutBuf, "0");
    } else if (2 == nMethod) {  // 5位小数点转换  000bb8->0.03000
        double dNum = (double)i64Num / 100000;
        if (0 != dNum)
            sprintf(szOutBuf, "%0.5lf", dNum);
        else
            strcpy(szOutBuf, "0");  //这里nisec是0， cntax是0.00000，可以统一为0
    }
    return nInbufLenMax;
}

uint8 word_38E680[512] = {
    0x00, 0x00, 0x89, 0x11, 0x12, 0x23, 0x9B, 0x32, 0x24, 0x46, 0xAD, 0x57, 0x36, 0x65, 0xBF, 0x74,
    0x48, 0x8C, 0xC1, 0x9D, 0x5A, 0xAF, 0xD3, 0xBE, 0x6C, 0xCA, 0xE5, 0xDB, 0x7E, 0xE9, 0xF7, 0xF8,
    0x81, 0x10, 0x08, 0x01, 0x93, 0x33, 0x1A, 0x22, 0xA5, 0x56, 0x2C, 0x47, 0xB7, 0x75, 0x3E, 0x64,
    0xC9, 0x9C, 0x40, 0x8D, 0xDB, 0xBF, 0x52, 0xAE, 0xED, 0xDA, 0x64, 0xCB, 0xFF, 0xF9, 0x76, 0xE8,
    0x02, 0x21, 0x8B, 0x30, 0x10, 0x02, 0x99, 0x13, 0x26, 0x67, 0xAF, 0x76, 0x34, 0x44, 0xBD, 0x55,
    0x4A, 0xAD, 0xC3, 0xBC, 0x58, 0x8E, 0xD1, 0x9F, 0x6E, 0xEB, 0xE7, 0xFA, 0x7C, 0xC8, 0xF5, 0xD9,
    0x83, 0x31, 0x0A, 0x20, 0x91, 0x12, 0x18, 0x03, 0xA7, 0x77, 0x2E, 0x66, 0xB5, 0x54, 0x3C, 0x45,
    0xCB, 0xBD, 0x42, 0xAC, 0xD9, 0x9E, 0x50, 0x8F, 0xEF, 0xFB, 0x66, 0xEA, 0xFD, 0xD8, 0x74, 0xC9,
    0x04, 0x42, 0x8D, 0x53, 0x16, 0x61, 0x9F, 0x70, 0x20, 0x04, 0xA9, 0x15, 0x32, 0x27, 0xBB, 0x36,
    0x4C, 0xCE, 0xC5, 0xDF, 0x5E, 0xED, 0xD7, 0xFC, 0x68, 0x88, 0xE1, 0x99, 0x7A, 0xAB, 0xF3, 0xBA,
    0x85, 0x52, 0x0C, 0x43, 0x97, 0x71, 0x1E, 0x60, 0xA1, 0x14, 0x28, 0x05, 0xB3, 0x37, 0x3A, 0x26,
    0xCD, 0xDE, 0x44, 0xCF, 0xDF, 0xFD, 0x56, 0xEC, 0xE9, 0x98, 0x60, 0x89, 0xFB, 0xBB, 0x72, 0xAA,
    0x06, 0x63, 0x8F, 0x72, 0x14, 0x40, 0x9D, 0x51, 0x22, 0x25, 0xAB, 0x34, 0x30, 0x06, 0xB9, 0x17,
    0x4E, 0xEF, 0xC7, 0xFE, 0x5C, 0xCC, 0xD5, 0xDD, 0x6A, 0xA9, 0xE3, 0xB8, 0x78, 0x8A, 0xF1, 0x9B,
    0x87, 0x73, 0x0E, 0x62, 0x95, 0x50, 0x1C, 0x41, 0xA3, 0x35, 0x2A, 0x24, 0xB1, 0x16, 0x38, 0x07,
    0xCF, 0xFF, 0x46, 0xEE, 0xDD, 0xDC, 0x54, 0xCD, 0xEB, 0xB9, 0x62, 0xA8, 0xF9, 0x9A, 0x70, 0x8B,
    0x08, 0x84, 0x81, 0x95, 0x1A, 0xA7, 0x93, 0xB6, 0x2C, 0xC2, 0xA5, 0xD3, 0x3E, 0xE1, 0xB7, 0xF0,
    0x40, 0x08, 0xC9, 0x19, 0x52, 0x2B, 0xDB, 0x3A, 0x64, 0x4E, 0xED, 0x5F, 0x76, 0x6D, 0xFF, 0x7C,
    0x89, 0x94, 0x00, 0x85, 0x9B, 0xB7, 0x12, 0xA6, 0xAD, 0xD2, 0x24, 0xC3, 0xBF, 0xF1, 0x36, 0xE0,
    0xC1, 0x18, 0x48, 0x09, 0xD3, 0x3B, 0x5A, 0x2A, 0xE5, 0x5E, 0x6C, 0x4F, 0xF7, 0x7D, 0x7E, 0x6C,
    0x0A, 0xA5, 0x83, 0xB4, 0x18, 0x86, 0x91, 0x97, 0x2E, 0xE3, 0xA7, 0xF2, 0x3C, 0xC0, 0xB5, 0xD1,
    0x42, 0x29, 0xCB, 0x38, 0x50, 0x0A, 0xD9, 0x1B, 0x66, 0x6F, 0xEF, 0x7E, 0x74, 0x4C, 0xFD, 0x5D,
    0x8B, 0xB5, 0x02, 0xA4, 0x99, 0x96, 0x10, 0x87, 0xAF, 0xF3, 0x26, 0xE2, 0xBD, 0xD0, 0x34, 0xC1,
    0xC3, 0x39, 0x4A, 0x28, 0xD1, 0x1A, 0x58, 0x0B, 0xE7, 0x7F, 0x6E, 0x6E, 0xF5, 0x5C, 0x7C, 0x4D,
    0x0C, 0xC6, 0x85, 0xD7, 0x1E, 0xE5, 0x97, 0xF4, 0x28, 0x80, 0xA1, 0x91, 0x3A, 0xA3, 0xB3, 0xB2,
    0x44, 0x4A, 0xCD, 0x5B, 0x56, 0x69, 0xDF, 0x78, 0x60, 0x0C, 0xE9, 0x1D, 0x72, 0x2F, 0xFB, 0x3E,
    0x8D, 0xD6, 0x04, 0xC7, 0x9F, 0xF5, 0x16, 0xE4, 0xA9, 0x90, 0x20, 0x81, 0xBB, 0xB3, 0x32, 0xA2,
    0xC5, 0x5A, 0x4C, 0x4B, 0xD7, 0x79, 0x5E, 0x68, 0xE1, 0x1C, 0x68, 0x0D, 0xF3, 0x3F, 0x7A, 0x2E,
    0x0E, 0xE7, 0x87, 0xF6, 0x1C, 0xC4, 0x95, 0xD5, 0x2A, 0xA1, 0xA3, 0xB0, 0x38, 0x82, 0xB1, 0x93,
    0x46, 0x6B, 0xCF, 0x7A, 0x54, 0x48, 0xDD, 0x59, 0x62, 0x2D, 0xEB, 0x3C, 0x70, 0x0E, 0xF9, 0x1F,
    0x8F, 0xF7, 0x06, 0xE6, 0x9D, 0xD4, 0x14, 0xC5, 0xAB, 0xB1, 0x22, 0xA0, 0xB9, 0x92, 0x30, 0x83,
    0xC7, 0x7B, 0x4E, 0x6A, 0xD5, 0x58, 0x5C, 0x49, 0xE3, 0x3D, 0x6A, 0x2C, 0xF1, 0x1E, 0x78, 0x0F};

uint16 NisecBuildVCodeXorNum(uint16 vInputNum, uint8 *a1, int a2)
{
    //单字节最大FF×2=1fe，留一个字节预留，最大1ff
    if (a2) {
        do {
            LOWORD(vInputNum) = BYTE1(vInputNum) ^
                                *(uint16 *)&word_38E680[(*a1++ ^ (unsigned __int8)vInputNum) * 2];
            --a2;
        } while (a2);
    }
    vInputNum = ~vInputNum;
    return vInputNum;
}

// Transfer mode, 0xe位置上有个校验位
// a1 src;a2 src-len;ret=vcode
uint16 NisecBuildVCode(uint8 *a1, int a2)
{
    return NisecBuildVCodeXorNum(0xffff, a1, a2);
}

uint8 BuildTransferVCode(uint8 *a1, int a2)
{
    uint16 v3 = NisecBuildVCode(a1, a2);
    uint8 v7 = HIBYTE(v3) ^ LOBYTE(v3);
    return v7;
}

//专用函数，非通用函数，实际为cntax使用的函数，为了调用word_38E680，放在这了
void Get32bytesOperKeyVersionBuf_InputSeed(uint8 *a1)
{
    uint32 v5 = 0xFFFFFF28;
    uint32 *v4 = (uint32 *)a1;
    uint8 *v18 = a1 + 0xc + 2;
    uint8 num = 0;
    uint16 num2 = 0;
    do {
        v4 = (uint32 *)((char *)v4 + 1);
        num = (uint8)(*((uint8 *)v4 - 1) ^ (uint8)v5);
        num2 = *(uint16 *)(word_38E680 + num * 2);
        v5 = num2 ^ ((uint16)v5 >> 8);
    } while (v4 != (uint32 *)v18);
    uint16 v6 = ~v5;
    *(uint16 *)v18 = bswap_16(v6);
}

// unsigned int sub_465770(char *a1, int a2)
uint16 BuildPreMW_String2Word(char *a1, int a2)
{
    unsigned __int16 v2;
    v2 = NisecBuildVCode((uint8 *)a1, a2);
    return (v2 & 0xF) ^ (((v2 & 0xF0) ^ ((((unsigned int)v2 >> 4) ^ (v2 & 0xF00)) >> 4)) >> 4);
}

//--------------------nisec 密文解密函数
sint16 BuildTwoByteMWVcode(const void *a1, signed int a2)
{
    unsigned char byte_39B4BC[] = {
        //这里将dword缩短为byte
        0xFF, 0x00, 0x01, 0x19, 0x02, 0x32, 0x1A, 0xC6, 0x03, 0xDF, 0x33, 0xEE, 0x1B, 0x68, 0xC7,
        0x4B, 0x04, 0x64, 0xE0, 0x0E, 0x34, 0x8D, 0xEF, 0x81, 0x1C, 0xC1, 0x69, 0xF8, 0xC8, 0x08,
        0x4C, 0x71, 0x05, 0x8A, 0x65, 0x2F, 0xE1, 0x24, 0x0F, 0x21, 0x35, 0x93, 0x8E, 0xDA, 0xF0,
        0x12, 0x82, 0x45, 0x1D, 0xB5, 0xC2, 0x7D, 0x6A, 0x27, 0xF9, 0xB9, 0xC9, 0x9A, 0x09, 0x78,
        0x4D, 0xE4, 0x72, 0xA6, 0x06, 0xBF, 0x8B, 0x62, 0x66, 0xDD, 0x30, 0xFD, 0xE2, 0x98, 0x25,
        0xB3, 0x10, 0x91, 0x22, 0x88, 0x36, 0xD0, 0x94, 0xCE, 0x8F, 0x96, 0xDB, 0xBD, 0xF1, 0xD2,
        0x13, 0x5C, 0x83, 0x38, 0x46, 0x40, 0x1E, 0x42, 0xB6, 0xA3, 0xC3, 0x48, 0x7E, 0x6E, 0x6B,
        0x3A, 0x28, 0x54, 0xFA, 0x85, 0xBA, 0x3D, 0xCA, 0x5E, 0x9B, 0x9F, 0x0A, 0x15, 0x79, 0x2B,
        0x4E, 0xD4, 0xE5, 0xAC, 0x73, 0xF3, 0xA7, 0x57, 0x07, 0x70, 0xC0, 0xF7, 0x8C, 0x80, 0x63,
        0x0D, 0x67, 0x4A, 0xDE, 0xED, 0x31, 0xC5, 0xFE, 0x18, 0xE3, 0xA5, 0x99, 0x77, 0x26, 0xB8,
        0xB4, 0x7C, 0x11, 0x44, 0x92, 0xD9, 0x23, 0x20, 0x89, 0x2E, 0x37, 0x3F, 0xD1, 0x5B, 0x95,
        0xBC, 0xCF, 0xCD, 0x90, 0x87, 0x97, 0xB2, 0xDC, 0xFC, 0xBE, 0x61, 0xF2, 0x56, 0xD3, 0xAB,
        0x14, 0x2A, 0x5D, 0x9E, 0x84, 0x3C, 0x39, 0x53, 0x47, 0x6D, 0x41, 0xA2, 0x1F, 0x2D, 0x43,
        0xD8, 0xB7, 0x7B, 0xA4, 0x76, 0xC4, 0x17, 0x49, 0xEC, 0x7F, 0x0C, 0x6F, 0xF6, 0x6C, 0xA1,
        0x3B, 0x52, 0x29, 0x9D, 0x55, 0xAA, 0xFB, 0x60, 0x86, 0xB1, 0xBB, 0xCC, 0x3E, 0x5A, 0xCB,
        0x59, 0x5F, 0xB0, 0x9C, 0xA9, 0xA0, 0x51, 0x0B, 0xF5, 0x16, 0xEB, 0x7A, 0x75, 0x2C, 0xD7,
        0x4F, 0xAE, 0xD5, 0xE9, 0xE6, 0xE7, 0xAD, 0xE8, 0x74, 0xD6, 0xF4, 0xEA, 0xA8, 0x50, 0x58,
        0xAF};
    unsigned char byte_39B8BC[] = {
        //这里将dword缩短为byte
        0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1D, 0x3A, 0x74, 0xE8, 0xCD, 0x87, 0x13,
        0x26, 0x4C, 0x98, 0x2D, 0x5A, 0xB4, 0x75, 0xEA, 0xC9, 0x8F, 0x03, 0x06, 0x0C, 0x18, 0x30,
        0x60, 0xC0, 0x9D, 0x27, 0x4E, 0x9C, 0x25, 0x4A, 0x94, 0x35, 0x6A, 0xD4, 0xB5, 0x77, 0xEE,
        0xC1, 0x9F, 0x23, 0x46, 0x8C, 0x05, 0x0A, 0x14, 0x28, 0x50, 0xA0, 0x5D, 0xBA, 0x69, 0xD2,
        0xB9, 0x6F, 0xDE, 0xA1, 0x5F, 0xBE, 0x61, 0xC2, 0x99, 0x2F, 0x5E, 0xBC, 0x65, 0xCA, 0x89,
        0x0F, 0x1E, 0x3C, 0x78, 0xF0, 0xFD, 0xE7, 0xD3, 0xBB, 0x6B, 0xD6, 0xB1, 0x7F, 0xFE, 0xE1,
        0xDF, 0xA3, 0x5B, 0xB6, 0x71, 0xE2, 0xD9, 0xAF, 0x43, 0x86, 0x11, 0x22, 0x44, 0x88, 0x0D,
        0x1A, 0x34, 0x68, 0xD0, 0xBD, 0x67, 0xCE, 0x81, 0x1F, 0x3E, 0x7C, 0xF8, 0xED, 0xC7, 0x93,
        0x3B, 0x76, 0xEC, 0xC5, 0x97, 0x33, 0x66, 0xCC, 0x85, 0x17, 0x2E, 0x5C, 0xB8, 0x6D, 0xDA,
        0xA9, 0x4F, 0x9E, 0x21, 0x42, 0x84, 0x15, 0x2A, 0x54, 0xA8, 0x4D, 0x9A, 0x29, 0x52, 0xA4,
        0x55, 0xAA, 0x49, 0x92, 0x39, 0x72, 0xE4, 0xD5, 0xB7, 0x73, 0xE6, 0xD1, 0xBF, 0x63, 0xC6,
        0x91, 0x3F, 0x7E, 0xFC, 0xE5, 0xD7, 0xB3, 0x7B, 0xF6, 0xF1, 0xFF, 0xE3, 0xDB, 0xAB, 0x4B,
        0x96, 0x31, 0x62, 0xC4, 0x95, 0x37, 0x6E, 0xDC, 0xA5, 0x57, 0xAE, 0x41, 0x82, 0x19, 0x32,
        0x64, 0xC8, 0x8D, 0x07, 0x0E, 0x1C, 0x38, 0x70, 0xE0, 0xDD, 0xA7, 0x53, 0xA6, 0x51, 0xA2,
        0x59, 0xB2, 0x79, 0xF2, 0xF9, 0xEF, 0xC3, 0x9B, 0x2B, 0x56, 0xAC, 0x45, 0x8A, 0x09, 0x12,
        0x24, 0x48, 0x90, 0x3D, 0x7A, 0xF4, 0xF5, 0xF7, 0xF3, 0xFB, 0xEB, 0xCB, 0x8B, 0x0B, 0x16,
        0x2C, 0x58, 0xB0, 0x7D, 0xFA, 0xE9, 0xCF, 0x83, 0x1B, 0x36, 0x6C, 0xD8, 0xAD, 0x47, 0x8E,
        0x00};
    uint32 dword_39B4B4 = 0x1e;
    uint32 dword_39B4B0 = 0x0b;
    unsigned __int8 v2;
    signed int v3;
    int v4;
    sint16 result;
    unsigned __int8 v6;
    char v7[256];
    if (a1 && a2 <= 253) {
        //这里有个内存初始化函数sub_362DF0()，没必要了，直接用初始化后的内存进行运算
        if (a2 > 0)
            memcpy(v7, a1, a2);
        if (a2 < 253)
            memset(&v7[a2], 0, 253 - a2);
        v6 = 0;
        v2 = 0;
        v3 = 252;
        do {
            v4 = (uint8)byte_39B4BC[v6 ^ (unsigned __int8)v7[v3]];
            if (v4 == 0xff) {
                v6 = v2;
                v2 = 0;
            } else {
                if (dword_39B4B4 == 0xff)
                    v6 = v2;
                else
                    v6 = v2 ^ byte_39B8BC[(dword_39B4B4 + v4) % 255];
                v2 = byte_39B8BC[(v4 + dword_39B4B0) % 255];
                // printf("v2:%d v4:%d v6:%d\n", v2, v4, v6);
            }
            --v3;
        } while (v3 >= 0);
        result = v6 + (v2 << 8);
    } else {
        result = -1;
    }
    return result;
}

int Build6ByteMWPreTail(uint8 bIsRed, char *kpsj_standard, char *szDeviceID,
                        char *szTaxAuthorityCodeEx, char *xfsh, uint8 *abOutBuf)
{
    if (!strlen(kpsj_standard) || !strlen(szDeviceID) || !strlen(szTaxAuthorityCodeEx) ||
        !strlen(xfsh)) {
        logout(INFO, "TAXLIB", "发票开具",
               "税盘开具结果查询返回信息中开票时间、盘号等信息长度有误\r\n");
        return -1;
    }
    char szTmp[64] = "";
    // kpsj
    strncpy(szTmp, kpsj_standard, 8);
    uint16 v5 = BuildPreMW_String2Word(szTmp, strlen(szTmp)) & 0xF;
    // devid
    uint16 v23 = BuildPreMW_String2Word(szDeviceID, strlen(szDeviceID)) & 0xF;
    // szTaxAuthorityCodeEx
    uint16 v10 = BuildPreMW_String2Word(szTaxAuthorityCodeEx, strlen(szTaxAuthorityCodeEx)) & 0xF;
    // xfsh
    sprintf(szTmp, "%-20s", xfsh);
    uint16 v12 = BuildPreMW_String2Word(szTmp, strlen(szTmp)) & 0xF;
    // flag1,实际运算方式如下，简化为是否红票
    uint16 n1 = bIsRed;
    //"0199150399" 开票软件里面固定值的第一个字节 0x01,确认为静态值1(cntax\nisec皆为固定值无误)
    //疑为加密版本号01，税局正式盘都是01，其他机构为十进制17
    //非税务机关的模拟环境企发卡发行产生的开票金税卡开具出的发票其加密版本号为01
    uint16 n2 = 1;
    // result
    sprintf(szTmp, "%01x%01x%01x%01x%01x%01x", n1, n2, v5, v23, v10, v12);
    Str2Byte(abOutBuf, szTmp, strlen(szTmp));
    return 0;
}

int NisecMWDec(uint8 a1, uint8 a2, uint8 bFlag0, uint8 *a4, uint8 *a5, uint32 a6, char *a7)
{
    int v10;
    char *v12;
    int v14;
    unsigned int v8;
    unsigned int v15;
    int v16;
    char *v17 = NULL;
    char v18 = 0;
    uint8 *v115;
    int nUsbRawLen = 0x50;
    v8 = a6;
    v10 = nUsbRawLen / v8 + 1;
    int nRet = -1;
    uint8 abBuf[256] = {0};
    uint8 abBuf2[256] = {0};
    // abBuf mem
    // bFlag0(1) + UsbRaw(40) + 3byte-tail(3) = 44bytes
    abBuf[0] = bFlag0;
    memcpy(abBuf + 1, (uint8 *)a4, 40);  // 0x70,but use 40bytes
    memcpy(abBuf + 41, (uint8 *)a5, 3);
    v115 = abBuf;
    v12 = (char *)abBuf + 1;
    v14 = 1 + 40 + 3;
    if (*v115 >= 3u) {
        if (a2)
            *(v115 + v14) = a1;
        else
            *(v115 + v14) = a1 - 1;
        ++v14;
    }

    v15 = (v10 * a6 >> 1) - v14;
    v16 = v15 - 2;
    if (v15 < 2)
        v16 = v15;
    if (v16) {
        v17 = v12;
        do {
            v18 = *v17 + v17[1];
            v17 += 2;
            *(v115 + v14++) = v18;
            --v16;
        } while (v16);
    }
    if (v15 >= 2) {
        char szTmp[256] = "";
        Byte2Str(szTmp, v115, v14);
        uint16 vcode = NisecBuildVCode((uint8 *)szTmp, v14 * 2);
        *(uint16 *)(v115 + v14) = bswap_16(vcode);
        v14 += 2;
    }
    int nDoSize = v8 >> 1;
    int nDoCount = v14 / nDoSize;
    if (0 != v14 % nDoSize)
        goto NDEC_LABEL_Finish;
    int i = 0;
    int nDstIndex = 0;
    for (i = 0; i < nDoCount; i++) {
        sint16 nVCode = BuildTwoByteMWVcode(v115 + i * nDoSize, nDoSize);
        memcpy(abBuf2 + nDstIndex, v115 + i * nDoSize, nDoSize);
        nDstIndex += nDoSize;
        *(uint16 *)(abBuf2 + nDstIndex) = bswap_16(nVCode);
        nDstIndex += 2;
    };
    if (nDstIndex * 2 > 120)  //不支持二维码，汉字防伪等超过112字节的密文
        goto NDEC_LABEL_Finish;
    memset(abBuf, 0, sizeof(abBuf));
    Byte2Str((char *)abBuf, abBuf2, nDstIndex);
    char szDic[] = "*+<->/";  // 0123456789*+<->/
    int nStrLen = strlen((char *)abBuf);
    for (i = 0; i < nStrLen; i++) {
        if (!isxdigit(abBuf[i])) {
            goto NDEC_LABEL_Finish;
        }
        if (isdigit(abBuf[i]))
            continue;
        abBuf[i] = toupper(abBuf[i]);
        int nHexIndex = abBuf[i] - 0x41;
        abBuf[i] = szDic[nHexIndex];
    }
    strcpy(a7, (char *)abBuf);
    nRet = strlen((char *)abBuf);
NDEC_LABEL_Finish:
    return nRet;
}

int JYMBinChange(uint8 *a1, int a1Len, uint8 *a3)
{
    uint8 v8;
    unsigned int v9;
    int v10;
    uint8 *v11;
    uint8 v12;
    uint8 vBuf[512] = "";
    v8 = 0;
    if (a1Len >= 512)
        return -1;
    v9 = a1Len >> 3;
    memcpy(vBuf, a1, a1Len);
    uint8 *v33 = vBuf + 8;
    do {
        if (v9 > 1) {
            v10 = v9 - 1;
            v11 = v33 + v8;
            do {
                v12 = *v11;
                v11 += 8;
                v33[v8 - 8] ^= v12;
                --v10;
            } while (v10);
        }
        ++v8;
    } while (v8 < 8);
    memcpy(a3, vBuf, a1Len);
    return a1Len;
}

// NisecMWDec的封装接口
int NisecMWDecAPI(uint8 bFPLX_Common, bool bIsRed, char *szDeviceID, char *szTaxAuthorityCodeEx,
                  char *kpsj_standard, char *xfsh, uint8 *pabUSBMWRawIn_50Bytes, char *szOutMW)
{
    if (!strlen(kpsj_standard) || !strlen(szDeviceID) || !strlen(szTaxAuthorityCodeEx) ||
        !strlen(xfsh))
        return -1;
    //##decode MW
    // bFlag0 =="0199150399" 开票软件里面固定值第4个字节 0x03;目前还没发现有其他值，应该无争议
    uint8 bFlag0 = 0x03;
    uint8 abBufTmp[256] = "";
    if (Build6ByteMWPreTail(bIsRed, kpsj_standard, szDeviceID, szTaxAuthorityCodeEx, xfsh,
                            abBufTmp) < 0)
        return -1;
    uint32 dword_39B334 = 0x00000018;  //固定值,密文分组切割大小
    // pUSBRaw实际只用40bytes
    int nChildRet =
        NisecMWDec(bFPLX_Common, 0, bFlag0, pabUSBMWRawIn_50Bytes, abBufTmp, dword_39B334, szOutMW);
    if (nChildRet < 0) {
        logout(INFO, "TAXLIB", "发票开具",
               "税盘开具结果查询返回信息中密文长度有误, nChildRet = %d\r\n", nChildRet);
        return -2;
    }
    return strlen(szOutMW);
}

// NisecJYMDec的封装接口, szOutMW's buffer > 32
int NisecJYMDecAPI(uint8 *pabUSBMWRawIn, int JymChangeSize, char *szOutJYM)
{
    uint8 abBuf[128] = "";
    if (JYMBinChange(pabUSBMWRawIn, JymChangeSize, abBuf) < 0) {
        logout(INFO, "TAXLIB", "发票开具", "税盘开具结果查询返回信息中校验码有误\r\n");
        return -1;
    }
    uint64 ui64JYM64Num = 0;
    memcpy(&ui64JYM64Num, abBuf, 8);
    ui64JYM64Num = bswap_64(ui64JYM64Num);
    sprintf(szOutJYM, "%020llu", ui64JYM64Num);
    return strlen(szOutJYM);
}
//--------------------nisec 密文解密函数 finish
//和makeinvoice的MoveTail函数功能差不多,搞不清楚的情况下就先分开处理
//返回值为origbuf偏移值，nNewBufOffset为新缓冲的偏移长度
int DecodeTail(uint8 bInvoiceStatusFlag, uint8 *abInvTemplate, char *szDst, int nInputStrLen,
               int nBlockBinLen, int nSignStrLen, int *nNewBufOffset)
{
    int result = -1;
    int v9 = 0;
    v9 = *(_BYTE *)(abInvTemplate + 2) + (*(_BYTE *)(abInvTemplate + 1) << 8);
    uint8 v85 = 0;
    v85 = ((BYTE4(v9) & 3) + (signed int)v9) >> 2;
    uint8 v81 = 0;
    uint32 v66 = 0;
    uint32 nOrigTotalLen = 0;
    uint8 *v8 = NULL;
    int v208 = 0, v218 = 0, v216 = 0, v6 = 0, v7 = 0, nHasDoLen = 0, nNewIndex = 0, nOffset = 0,
        nNewLen = 0, nChildRet = 0, vCount = 0, nChildOK = -1;
    char szTmpVar[128] = {0};
    int8 v67 = 0;
    uint8 v68 = 0;
    uint8 v69 = 0;

    LOBYTE(v6) = *(_BYTE *)(abInvTemplate + 2);
    v7 = *(_BYTE *)(abInvTemplate + 1);
    LOBYTE(v208) = (v6 + (v7 << 8)) / 4;
    if ((_BYTE)v208) {
        v8 = abInvTemplate + 7;
        vCount = (unsigned __int8)v208;
        do {
            if (*(_BYTE *)(v8 - 1) == 254) {
                v218 = *(_BYTE *)(v8 + 1);  // to use
            } else if (*(_BYTE *)(v8 - 1) == 255) {
                v216 = *(_BYTE *)(v8 + 1);  // to use
            }
            v8 += 4;
            --vCount;
        } while (vCount);
    }
    do {
        memset(szTmpVar, 0, sizeof(szTmpVar));
        v67 = (uint8)v81;
        v68 = *(_BYTE *)(abInvTemplate + 4 * (uint8)v81 + 6);
        if (FPSTATUS_BLANK_WASTE == bInvoiceStatusFlag &&
            v68 > 1) {  //这里还有个或条件满足也可以，暂时没遇到这种情况，暂且不考虑
            //空白作废签名
            nChildOK = 0;
            result = nOrigTotalLen;
            *nNewBufOffset += v66;  //相对偏移+正常值
            break;                  //空白作废此处结束函数
        }
        if (253 == v68) {
            nHasDoLen = v66;  //这里和官方的实现略有区别,我们已经处理掉的等会还得加上偏移
            nNewIndex =
                2 * (nBlockBinLen + 0x7FFFFFFF * (nSignStrLen + v218 + v216 + v66 / 2)) - 280;
            if (nNewIndex < 0 || nNewIndex > DEF_MAX_FPBUF_LEN)
                break;                    // finish
            v66 = nNewIndex + nHasDoLen;  //由Newlen覆盖成了origlen了
        } else if (254 == v68) {          //暂时不实现
        } else if (255 == v68) {
            // OK
            if (v81 > 6 && v66 > 256) {
                nChildOK = 0;
                result = v66;  //正常开票，此处赋值后while执行完毕，正常退出循环
                *nNewBufOffset += v66;  //相对偏移+正常值
            }
        } else {
            v69 = *(_BYTE *)(abInvTemplate + 4 * (uint8)v81 + 9);
            if (v69 & 8) {
                nChildRet = -1;
                switch (v69 & 0xF0) {
                    case 0: {
                        int nOrigLen = 0;
                        //这里要和开票那边颠倒下
                        nOffset = v66;
                        nOrigLen = 2 * *(_BYTE *)(abInvTemplate + 4 * (uint8)v81 + 8);
                        strncpy(szTmpVar, szDst + nOffset, nOrigLen);
                        nNewLen = 2 * *(_BYTE *)(abInvTemplate + 4 * (uint8)v81 + 7);
                        if (nNewLen < nOrigLen)
                            break;
                        memmove(szDst + nOffset + nNewLen, szDst + nOffset + nOrigLen,
                                nInputStrLen - nOffset - nOrigLen);
                        nInputStrLen = nInputStrLen - nOrigLen + nNewLen;
                        // 20220629 因空白作废问题和修改
                        nOrigTotalLen += nOrigLen;  // oldLen
                        *nNewBufOffset += nNewLen - nOrigLen;
                        if (NisecBaseConv(szTmpVar, nOrigLen, 0x10, szDst + nOffset, 0xa, nNewLen) <
                            0)
                            break;
                        nChildRet = 0;
                        break;
                    }
                    // case 0x10:  //暂时不实现
                    // case 0x20:
                    // case 0x30:
                    // case 0x40:
                    default: {
                        result = -3;
                        break;
                    }
                }
                if (nChildRet < 0)
                    break;  // jump to finish
            }
            v66 += 2 * *(_BYTE *)(abInvTemplate + 4 * v67 + 7);  // nNewLen
        }
        LOBYTE(v81) = (_BYTE)v81 + 1;
    } while (v81 < v85);
    if (nChildOK < 0)
        return -3;
    return result;
}
//---------------------------cert API---------------------------

int sub_1207150(unsigned int a1)
{
    //国密sm4算法S盒
    unsigned char byte_1215360[256] = {
        0xD6, 0x90, 0xE9, 0xFE, 0xCC, 0xE1, 0x3D, 0xB7, 0x16, 0xB6, 0x14, 0xC2, 0x28, 0xFB, 0x2C,
        0x05, 0x2B, 0x67, 0x9A, 0x76, 0x2A, 0xBE, 0x04, 0xC3, 0xAA, 0x44, 0x13, 0x26, 0x49, 0x86,
        0x06, 0x99, 0x9C, 0x42, 0x50, 0xF4, 0x91, 0xEF, 0x98, 0x7A, 0x33, 0x54, 0x0B, 0x43, 0xED,
        0xCF, 0xAC, 0x62, 0xE4, 0xB3, 0x1C, 0xA9, 0xC9, 0x08, 0xE8, 0x95, 0x80, 0xDF, 0x94, 0xFA,
        0x75, 0x8F, 0x3F, 0xA6, 0x47, 0x07, 0xA7, 0xFC, 0xF3, 0x73, 0x17, 0xBA, 0x83, 0x59, 0x3C,
        0x19, 0xE6, 0x85, 0x4F, 0xA8, 0x68, 0x6B, 0x81, 0xB2, 0x71, 0x64, 0xDA, 0x8B, 0xF8, 0xEB,
        0x0F, 0x4B, 0x70, 0x56, 0x9D, 0x35, 0x1E, 0x24, 0x0E, 0x5E, 0x63, 0x58, 0xD1, 0xA2, 0x25,
        0x22, 0x7C, 0x3B, 0x01, 0x21, 0x78, 0x87, 0xD4, 0x00, 0x46, 0x57, 0x9F, 0xD3, 0x27, 0x52,
        0x4C, 0x36, 0x02, 0xE7, 0xA0, 0xC4, 0xC8, 0x9E, 0xEA, 0xBF, 0x8A, 0xD2, 0x40, 0xC7, 0x38,
        0xB5, 0xA3, 0xF7, 0xF2, 0xCE, 0xF9, 0x61, 0x15, 0xA1, 0xE0, 0xAE, 0x5D, 0xA4, 0x9B, 0x34,
        0x1A, 0x55, 0xAD, 0x93, 0x32, 0x30, 0xF5, 0x8C, 0xB1, 0xE3, 0x1D, 0xF6, 0xE2, 0x2E, 0x82,
        0x66, 0xCA, 0x60, 0xC0, 0x29, 0x23, 0xAB, 0x0D, 0x53, 0x4E, 0x6F, 0xD5, 0xDB, 0x37, 0x45,
        0xDE, 0xFD, 0x8E, 0x2F, 0x03, 0xFF, 0x6A, 0x72, 0x6D, 0x6C, 0x5B, 0x51, 0x8D, 0x1B, 0xAF,
        0x92, 0xBB, 0xDD, 0xBC, 0x7F, 0x11, 0xD9, 0x5C, 0x41, 0x1F, 0x10, 0x5A, 0xD8, 0x0A, 0xC1,
        0x31, 0x88, 0xA5, 0xCD, 0x7B, 0xBD, 0x2D, 0x74, 0xD0, 0x12, 0xB8, 0xE5, 0xB4, 0xB0, 0x89,
        0x69, 0x97, 0x4A, 0x0C, 0x96, 0x77, 0x7E, 0x65, 0xB9, 0xF1, 0x09, 0xC5, 0x6E, 0xC6, 0x84,
        0x18, 0xF0, 0x7D, 0xEC, 0x3A, 0xDC, 0x4D, 0x20, 0x79, 0xEE, 0x5F, 0x3E, 0xD7, 0xCB, 0x39,
        0x48};
    unsigned int v1;
    int result;
    int v3;
    int v4;
    signed int v5;
    char v6;
    int v7;

    v1 = a1;
    result = 0;
    v3 = 0;
    v4 = 0;
    v5 = 4;
    do {
        *((_BYTE *)&a1 + v4) = v1 >> v3;
        v6 = byte_1215360[(v1 >> v3) & 0xFF];
        v7 = (unsigned __int8)v6 << v3;
        v3 += 8;
        result |= v7;
        ++v4;
        --v5;
    } while (v5);
    return result;
}

unsigned int sub_1207120(unsigned int a1)
{
    unsigned int v1;
    v1 = sub_1207150(a1);
    return v1 ^ ((v1 >> 9) | (v1 << 23)) ^ ((v1 << 13) | (v1 >> 19));
}

int sub_1207040(long a1, long a2)
{
    int i = 0;
    int v2;
    signed int v3;
    long v4;
    signed int result;
    int v6;
    int *v7;
    unsigned int v8;
    uint8 v9[64] = {0};
    signed int v13;

    unsigned char dword_12152D0[16] = {0xC6, 0xBA, 0xB1, 0xA3, 0x50, 0x33, 0xAA, 0x56,
                                       0x97, 0x91, 0x7D, 0x67, 0xDC, 0x22, 0x70, 0xB2};
    unsigned char dword_12152E0[128] = {
        0x15, 0x0E, 0x07, 0x00, 0x31, 0x2A, 0x23, 0x1C, 0x4D, 0x46, 0x3F, 0x38, 0x69, 0x62, 0x5B,
        0x54, 0x85, 0x7E, 0x77, 0x70, 0xA1, 0x9A, 0x93, 0x8C, 0xBD, 0xB6, 0xAF, 0xA8, 0xD9, 0xD2,
        0xCB, 0xC4, 0xF5, 0xEE, 0xE7, 0xE0, 0x11, 0x0A, 0x03, 0xFC, 0x2D, 0x26, 0x1F, 0x18, 0x49,
        0x42, 0x3B, 0x34, 0x65, 0x5E, 0x57, 0x50, 0x81, 0x7A, 0x73, 0x6C, 0x9D, 0x96, 0x8F, 0x88,
        0xB9, 0xB2, 0xAB, 0xA4, 0xD5, 0xCE, 0xC7, 0xC0, 0xF1, 0xEA, 0xE3, 0xDC, 0x0D, 0x06, 0xFF,
        0xF8, 0x29, 0x22, 0x1B, 0x14, 0x45, 0x3E, 0x37, 0x30, 0x61, 0x5A, 0x53, 0x4C, 0x7D, 0x76,
        0x6F, 0x68, 0x99, 0x92, 0x8B, 0x84, 0xB5, 0xAE, 0xA7, 0xA0, 0xD1, 0xCA, 0xC3, 0xBC, 0xED,
        0xE6, 0xDF, 0xD8, 0x09, 0x02, 0xFB, 0xF4, 0x25, 0x1E, 0x17, 0x10, 0x41, 0x3A, 0x33, 0x2C,
        0x5D, 0x56, 0x4F, 0x48, 0x79, 0x72, 0x6B, 0x64};

    v2 = 0;
    v3 = 4;
    do {
        uint32 uT1 = *(uint32 *)(dword_12152D0 + v2 * 4);
        uint32 uT2 = *(uint32 *)(a1 + v2 * 4);
        *(uint32 *)(v9 + v2 * 4) = uT1 ^ uT2;
        ++v2;
        --v3;
    } while (v3);
    v4 = a2;
    result = 2;
    v13 = 32;
    i = 0;
    do {
        v6 = result + 1;
        v7 = (int *)(v9 + ((result - 2) % 4) * 4);
        uint32 uT1 = *(uint32 *)(dword_12152E0 + i * 4);
        uint32 uT2 = *(uint32 *)(v9 + (result % 4) * 4);
        uint32 uT3 = *(uint32 *)(v9 + ((result + 1) % 4) * 4);
        uint32 uT4 = *(uint32 *)(v9 + ((result - 1) % 4) * 4);
        uint32 uResult = uT1 ^ uT2 ^ uT3 ^ uT4;
        v8 = sub_1207120(uResult);
        v8 ^= *v7;

        v4 += 4;
        *v7 = v8;
        *(_DWORD *)(v4 - 4) = v8;
        result = v6;
        --v13;
        i++;
    } while (v13);
    return result;
}

int sub_1207570(unsigned int *a1, int a2)
{
    int result;
    unsigned int *v3;
    int v4;
    unsigned int v5;
    unsigned int v6;
    int v7;
    int v8;
    result = a2 - 1;
    if (a2) {
        v3 = a1;
        v4 = a2;
        do {
            v5 = *v3;
            ++v3;
            v6 = v5;
            BYTE1(v6) = 0;
            v8 = (((v5 >> 8) & 0xFF00FF) | (v6 << 8)) << 16;
            v7 = (unsigned __int64)(((v5 >> 8) & 0xFF00FF) | (v6 << 8)) >> 16;
            result = v8;
            --v4;
            *(v3 - 1) = v8 | v7;
        } while (v4);
    }
    return result;
}

int sub_1207280(int a1)
{
    unsigned int v1;
    v1 = sub_1207150(a1);
    return v1 ^ (4 * v1 | (v1 >> 30)) ^ ((v1 << 10) | ((unsigned __int64)v1 >> 22)) ^
           (unsigned __int64)((signed __int64)__PAIR__(v1, v1) >> 8) ^ ((v1 >> 14) | (v1 << 18));
}

int *sub_12071A0(_DWORD *a1, long a2, int *a3)
{
    _DWORD *v4;
    int *v5;
    signed int v6;
    signed int v7;
    int v8;
    int *v9;
    int v10;
    int *v11;
    int *result;
    signed int v13;
    int v14;
    uint8 *v15;

    v15 = 0;
    v15 = (uint8 *)a2;
    v4 = a1;
    v5 = (int *)(a2 + 12);
    v6 = 2;
    v7 = 32;
    do {
        v8 = v6 + 1;
        v9 = (int *)(v15 + (v6 - 2) % 4 * 4);
        v10 =
            sub_1207280(*(uint32 *)v4 ^ *(uint32 *)(v15 + v6 % 4 * 4) ^
                        *(uint32 *)(v15 + (v6 + 1) % 4 * 4) ^ *(uint32 *)(v15 + (v6 - 1) % 4 * 4));
        ++v4;
        --v7;
        *v9 ^= v10;
        v6 = v8;
    } while (v7);
    v11 = a3;
    result = v5;
    v13 = 4;
    do {
        v14 = *result;
        --result;
        *v11 = v14;
        ++v11;
        --v13;
    } while (v13);
    return result;
}

//++todo 早期做nisec时尚未发现，后期发现疑似国密SM4算法，有空测试替换下
int SymEncrypt4(long a2, long a3, int a4, long pDst)
{
    int result;
    int v7;
    uint8 *v8;
    char v13[128] = "";
    char v21[128] = "";
    char szA2Copy[64] = "";
    memcpy(szA2Copy, (char *)a2, 16);
    char szA3Copy[64] = "";
    memcpy(szA3Copy, (char *)a3, 16);

    if (a4 % 16)
        return -1;
    sub_1207570((uint *)szA2Copy, 4);
    sub_1207040((long)szA2Copy, (long)v21);

    v7 = 0;
    if (a4 / 16 > 0) {
        v8 = (uint8 *)pDst;
        memcpy(v13, (char *)szA3Copy, 16);
        do {
            sub_1207570((uint *)v13, 4);
            sub_12071A0((_DWORD *)v21, (long)v13, (int *)v8);
            ++v7;
            v8 += 4;
        } while (v7 < a4 / 16);
    }
    sub_1207570((unsigned int *)pDst, a4 / 4);
    result = 0;
    return result;
}

int GetPreHashBegin16(char *prefixDevID, char *szDevIDInput, uint8 *szOut16)
{
    unsigned char abData_0x1451ef0[256] = {
        0x00, 0x50, 0xD6, 0x77, 0x1C, 0x70, 0xE2, 0x2A, 0x09, 0xA1, 0xC0, 0x55, 0xC4, 0xCC, 0x61,
        0xBD, 0xC4, 0xF0, 0x36, 0x4D, 0x41, 0x17, 0xBE, 0x2D, 0x4A, 0x14, 0xBD, 0x3F, 0xE0, 0xCF,
        0x02, 0x4A, 0x51, 0xBF, 0x3C, 0xAF, 0x19, 0xDE, 0xDA, 0x60, 0xF2, 0x39, 0xA0, 0x88, 0xD6,
        0xA6, 0xE4, 0xD0, 0x23, 0x17, 0x40, 0x2D, 0xFE, 0x4A, 0x28, 0xB1, 0xA8, 0xEC, 0x69, 0x66,
        0xB0, 0x3B, 0xD5, 0x05, 0x9F, 0x82, 0x2E, 0x41, 0x1D, 0xEF, 0x9F, 0x93, 0x1C, 0x44, 0x88,
        0xAA, 0x36, 0x9D, 0x2C, 0x7B, 0x69, 0xA0, 0x4C, 0x55, 0xBA, 0xCB, 0xF8, 0x1A, 0x06, 0x8E,
        0xC9, 0xDD, 0x0E, 0x26, 0x53, 0xD3, 0xDB, 0xAD, 0x4A, 0x26, 0xE2, 0x59, 0x17, 0xCC, 0x8D,
        0x66, 0x70, 0x86, 0x19, 0x8B, 0x72, 0x4A, 0x65, 0x27, 0xF5, 0x4A, 0x4F, 0xBB, 0xD8, 0x11,
        0x22, 0x6D, 0x7D, 0xA7, 0x94, 0x00, 0x21, 0x72, 0x27, 0xBF, 0x88, 0xF4, 0x1A, 0xA9, 0xBE,
        0xDC, 0x50, 0x64, 0x80, 0x33, 0x72, 0x98, 0x0F, 0x62, 0xCF, 0x39, 0x6E, 0x48, 0xBC, 0x35,
        0x87, 0x7E, 0x06, 0x2F, 0xFF, 0xD2, 0xA9, 0x80, 0x6F, 0xEC, 0x0A, 0xFD, 0x39, 0xBB, 0x5A,
        0x99, 0xFA, 0x89, 0xF2, 0x8E, 0xB0, 0x99, 0x8B, 0x16, 0xF8, 0xC8, 0xD9, 0xB8, 0xEF, 0x26,
        0xE6, 0x5C, 0x63, 0x4F, 0x11, 0xAE, 0x95, 0x89, 0x69, 0xE9, 0x3F, 0x61, 0x9C, 0x85, 0x18,
        0x22, 0x63, 0x25, 0xE7, 0x49, 0x9E, 0x99, 0xAE, 0xBD, 0x70, 0xB0, 0x64, 0xB4, 0x69, 0x51,
        0x62, 0x75, 0xA8, 0x86, 0x4E, 0xB3, 0xF0, 0x07, 0xE3, 0x11, 0x98, 0x30, 0xAE, 0x72, 0x58,
        0x98, 0x26, 0x4A, 0xEE, 0x3A, 0xB8, 0x30, 0xCC, 0x1C, 0xEC, 0xE6, 0xA6, 0xE3, 0x75, 0x96,
        0x50, 0x7B, 0x93, 0xC3, 0x6C, 0x69, 0x64, 0xB7, 0x42, 0xEC, 0x3E, 0x3A, 0xF7, 0x87, 0x36,
        0xEB};
    int i = 0, nDevIDRandLen;
    char szDevIDEx[24] = "";
    int nDevIDStrLen = strlen(szDevIDInput);
    uint8 abDevIDRand[48] = {0};  // len = static 28bytes
    if (nDevIDStrLen < 10 || strlen(prefixDevID) != 2)
        return -1;
    // DevID must be expand 2 bytes
    sprintf(szDevIDEx, "%s%s", prefixDevID, szDevIDInput);
    nDevIDStrLen = strlen(szDevIDEx);
    for (i = 0; i < nDevIDStrLen; i++) {
        uint8 bIndex = (uint8)szDevIDEx[i];
        abDevIDRand[i] = abData_0x1451ef0[bIndex];
    }
    // abData_0x14502f0是根据一个静态值(0x3),取索引表得到
    unsigned char abData_0x14502f0[32] = {0x8B, 0x84, 0xEE, 0xF8, 0x68, 0x6C, 0x28, 0xA7,
                                          0x71, 0x11, 0x3F, 0x0A, 0x2B, 0x0D, 0x8F, 0x22,
                                          0xD1, 0xEB, 0x0E, 0x3E, 0x7A, 0x92, 0xFA, 0x8A,
                                          0x20, 0x34, 0x1A, 0x53, 0xE3, 0xD2, 0x7B, 0x42};
    for (i = 0; i < 0x20; i++) {
        uint8 bIndex = (uint8)abData_0x14502f0[i];
        abDevIDRand[14 + i] = abData_0x1451ef0[bIndex];  // 14 == nDevIDStrLen
    }
    nDevIDRandLen = 14 + 0x20;  // 2e
    CalcMD5(abDevIDRand, nDevIDRandLen, (void *)szOut16);
    return 0;
}

// prefixDevID:nise=="33" cntax=="66"
int Build16ByteCryptKey_DeviceOpen(char *prefixDevID, char *szDevIDInput, uint8 *abPreHashEnd16,
                                   uint8 *abCryptKey)
{
    uint8 abPreHash[48] = "";  // 32bytes
    if (GetPreHashBegin16(prefixDevID, szDevIDInput, abPreHash) < 0)
        return -1;
    memcpy(abPreHash + 16, abPreHashEnd16, 16);
    if (SymEncrypt4((long)abPreHash, (long)abPreHash + 16, 0x10, (long)abCryptKey) < 0)
        return -2;
    return 0;
}

// a2输入长度为16字节；a4=0x20时a3输入长度0x20,a4=0x10时a3输入长度0x10
// a5输出缓冲；a6 usb rand
int SymEncrypt5(long a2, long a3, int a4, long pDst, long a6)
{
    long v8, v9;
    int v10 = 0, nDstIndex = 0, v22 = 0;
    char v28[128] = "", v23[128] = "";
    char *v11;
    uint8 v12;
    uint8 abA6Copy[128] = {0}, abA3Copy[128] = {0};
    memcpy(abA3Copy, (void *)a3, sizeof abA3Copy);
    memcpy(abA6Copy, (void *)a6, sizeof abA6Copy);

    if (a4 > 0x20)
        return -1;  // just test 0x10 and 0x20
    if (a4 % 16)
        return -2;
    if (a4 / 16 > 0) {
        v8 = (long)abA6Copy;
        v9 = (long)abA3Copy;
        do {
            v10 = 0;
            //第一次 usbcmd16 + usb datarand-8zero 16字节，进行xor
            //第二次 第一次的最终hash16 + usb datarand-8 calc bytes
            do {
                v11 = &v28[v10];
                uint8 bX1 = *(_BYTE *)(v9 + v10);
                uint8 bX2 = *(_BYTE *)(v8 + v10);
                v12 = bX1 ^ bX2;
                ++v10;
                *v11 = v12;
            } while (v10 < 16);
            memset(v23, 0, sizeof(v23));
            SymEncrypt4(a2, (long)v28, 16, (long)v23);
            memcpy((void *)v8, v23, 16);
            memcpy((void *)(pDst + nDstIndex), v23, 16);
            nDstIndex += 16;
            v9 += 16;
            ++v22;
        } while (v22 < a4 / 16);
    }
    return 0;
}

//输入缓冲区实际大小最好不低于24，无数据置0,szDevIDEx为33devid格式
int Build16ByteCryptKeyAnd4ByteVCode(char *szDevIDEx, char *szCertPwd, uint8 *abPreHashBegin8,
                                     uint8 *ab16BytesCommand, uint8 *ab16BytesParament,
                                     uint8 *abOutCryptKey, uint8 *abOutUSBCmd4ByteVCode)
{
    //所有都长度尽量都不低于64
    uint8 abPreHash[48] = {0}, abBuf[64] = {0}, abBuf2[64] = {0};
    if (strlen(szDevIDEx) != 14)
        return -1;
    if (CalcSHA1(szCertPwd, strlen(szCertPwd), abPreHash))
        return -1;
    // sha1(20位hash-取前16位)16hash+(2+12 税号)+8设备应答=0x26=38bytes 取sha1
    strcpy((char *)abBuf, szDevIDEx);
    memcpy(abPreHash + 16, abBuf, 14);
    memcpy(abPreHash + 30, abPreHashBegin8, 8);
    if (CalcSHA1(abPreHash, 38, abBuf))
        return -2;
    //得到20字节再和开卡算法差不多的变换,即可得到16字节CryptKey 还有个4字节USB VCode
    // abOutCryptKey
    memset(abPreHash, 0, sizeof abPreHash);
    memcpy(abPreHash, abBuf, 16);
    memcpy(abPreHash + 16, ab16BytesParament, 16);
    if (SymEncrypt4((long)abPreHash, (long)abPreHash + 16, 0x10, (long)abOutCryptKey) < 0)
        return -3;
    // abOutUSBCmd4ByteVCode
    memset(abBuf, 0, sizeof abBuf);
    memcpy(abBuf, ab16BytesCommand, 16);
    abBuf[16] = 0x80;  // 0x10时，非必须
    if (SymEncrypt5((long)abPreHash, (long)abBuf, 0x10, (long)abBuf2, (long)abPreHashBegin8))
        return -4;
    memset(abBuf, 0, sizeof abBuf);
    memcpy(abBuf, abOutCryptKey, 16);
    abBuf[16] = 0x80;  // 0x20时，必须
    if (SymEncrypt5((long)abPreHash, (long)abBuf, 0x20, (long)abOutUSBCmd4ByteVCode, (long)abBuf2))
        return -5;
    memmove(abOutUSBCmd4ByteVCode, abOutUSBCmd4ByteVCode + 16, 4);
    return 0;
}

unsigned int sub_365670(long a1, unsigned int a2, _DWORD *a3)
{
    unsigned int result;
    int v4;
    result = 0;
    *a3 = 0;
    if (a2) {
        do {
            v4 = *(_BYTE *)(result++ + a1);
            *a3 += v4;
        } while (result < a2);
    }
    return result;
}

//特殊税号处理，参考define.h gfsh字段说明
void SpecialGfshRestore(char *szInput, int nInputSize)
{
    // Z00011000000000.->000011000000000.
    if (nInputSize != 0xf)
        return;
    if (szInput[0] != 'Z')
        return;
    szInput[0] = '0';
}

void SpecialGfshChange(char *szInput)
{
    // 000011000000000.->Z00011000000000.
    if (strncmp(szInput, "0000", 4))
        return;
    if (szInput[15] != ' ')
        return;
    szInput[0] = 'Z';
}

//--------------------------nisec cntax 加密日志解析 begin----------------------------------
unsigned char byte_A6F940[256] = {
    0x90, 0x81, 0x4F, 0xDC, 0x14, 0x2A, 0x60, 0x88, 0x46, 0xEE, 0xB8, 0x22, 0xDE, 0x5E, 0x0B, 0xDB,
    0x47, 0x82, 0xC9, 0x7D, 0xAF, 0x59, 0xCA, 0xF0, 0xAD, 0xD4, 0xA2, 0xFA, 0x9C, 0xA4, 0x72, 0xC0,
    0xF6, 0x3E, 0xB5, 0x66, 0xB9, 0x03, 0x70, 0x0E, 0x61, 0x35, 0x57, 0x48, 0x86, 0xC1, 0x1D, 0x9E,
    0x05, 0xC7, 0x23, 0xC3, 0xE2, 0x96, 0x04, 0x9A, 0x07, 0x12, 0x80, 0x18, 0xEB, 0x27, 0xB2, 0x75,
    0x5A, 0x83, 0x2C, 0x1A, 0xB3, 0x6E, 0x09, 0xA0, 0x52, 0x3B, 0xD6, 0x1B, 0x29, 0xE3, 0x2F, 0x84,
    0xB1, 0xD1, 0x00, 0xED, 0x39, 0xFC, 0x53, 0x5B, 0x6A, 0xCB, 0xBE, 0x20, 0x4A, 0x4C, 0x58, 0xCF,
    0x33, 0xEF, 0xAA, 0xFB, 0x7F, 0x4D, 0xD0, 0x85, 0x45, 0xF9, 0x02, 0x43, 0x50, 0x3C, 0x9F, 0xA8,
    0x38, 0xA3, 0x40, 0x8F, 0x21, 0x9D, 0x51, 0xF5, 0xBC, 0xB6, 0xDA, 0x92, 0x10, 0xFF, 0xF3, 0xD2,
    0x42, 0xA1, 0x89, 0x0D, 0x0F, 0xE6, 0x8C, 0x68, 0x41, 0x99, 0x2D, 0xBF, 0xB0, 0x54, 0xBB, 0x16,
    0x6F, 0x7C, 0x77, 0x7B, 0x2B, 0x6B, 0x63, 0xC5, 0x30, 0x01, 0x67, 0xF2, 0xFE, 0xD7, 0xAB, 0x76,
    0x24, 0x32, 0x3A, 0x0A, 0x62, 0x06, 0xE0, 0x5C, 0xC2, 0xD3, 0xAC, 0x49, 0x91, 0x95, 0xE4, 0x79,
    0x4E, 0xC8, 0x37, 0x6D, 0xEA, 0xD5, 0xE7, 0xA9, 0x6C, 0x56, 0xF4, 0x8D, 0x65, 0x7A, 0xAE, 0x08,
    0xB4, 0x78, 0x25, 0x2E, 0x1F, 0xA6, 0xBA, 0xC6, 0xE8, 0xDD, 0x74, 0x1C, 0x4B, 0xBD, 0x8B, 0x8A,
    0xF7, 0xFD, 0x93, 0x26, 0xF1, 0x3F, 0xB7, 0xCC, 0x34, 0xA5, 0xE5, 0x36, 0x71, 0xD8, 0x31, 0x15,
    0x8E, 0xF8, 0x98, 0x11, 0xE9, 0xD9, 0xE1, 0x94, 0x9B, 0x1E, 0x87, 0x69, 0xCE, 0x55, 0x28, 0xDF,
    0x44, 0x0C, 0x13, 0xEC, 0x3D, 0x97, 0xCD, 0x17, 0xC4, 0xA7, 0x7E, 0x5F, 0x64, 0x5D, 0x19, 0x73};
unsigned char byte_A6FA40[256] = {
    0x0F, 0x13, 0x3F, 0xCA, 0x02, 0xD0, 0x2C, 0x1E, 0x8F, 0xC1, 0xAF, 0x03, 0xBD, 0x01, 0x6B, 0x8A,
    0x7C, 0xE3, 0x39, 0x82, 0x9B, 0x2F, 0xFF, 0x87, 0x34, 0x8E, 0x43, 0x44, 0xC4, 0xDE, 0xE9, 0xCB,
    0x54, 0x7B, 0x94, 0x32, 0x42, 0xC2, 0x23, 0x3D, 0xEE, 0x4C, 0x95, 0x0B, 0xA6, 0xFA, 0xC3, 0x4E,
    0x08, 0x2E, 0xA1, 0x66, 0x28, 0xD9, 0x24, 0xB2, 0x76, 0x5B, 0xA2, 0x49, 0x6D, 0x8B, 0xD1, 0x25,
    0x72, 0xF8, 0xF6, 0x64, 0x86, 0x68, 0x98, 0x16, 0xD4, 0xA4, 0x5C, 0xCC, 0x5D, 0x65, 0xB6, 0x92,
    0x6C, 0x70, 0x48, 0x50, 0xFD, 0xED, 0xB9, 0xDA, 0x5E, 0x15, 0x46, 0x57, 0xA7, 0x8D, 0x9D, 0x84,
    0x90, 0xD8, 0xAB, 0x00, 0x8C, 0xBC, 0xD3, 0x0A, 0xF7, 0xE4, 0x58, 0x05, 0xB8, 0xB3, 0x45, 0x06,
    0x30, 0x09, 0xA3, 0xD5, 0x52, 0x36, 0xA5, 0x38, 0xBF, 0x40, 0x6A, 0x9E, 0x81, 0xF3, 0xD7, 0xFB,
    0x3A, 0x91, 0x11, 0x41, 0x4F, 0x67, 0xDC, 0xEA, 0x97, 0xF2, 0xCF, 0xCE, 0xF0, 0xB4, 0xE6, 0x73,
    0x96, 0xAC, 0x74, 0x22, 0xE7, 0xAD, 0x35, 0x85, 0xE2, 0xF9, 0x37, 0xE8, 0x1C, 0x75, 0xDF, 0x6E,
    0x47, 0xF1, 0x1A, 0x71, 0x1D, 0x29, 0xC5, 0x89, 0x6F, 0xB7, 0x62, 0x0E, 0xAA, 0x18, 0xBE, 0x1B,
    0xFC, 0x56, 0x3E, 0x4B, 0xC6, 0xD2, 0x79, 0x20, 0x9A, 0xDB, 0xC0, 0xFE, 0x78, 0xCD, 0x5A, 0xF4,
    0x1F, 0xDD, 0xA8, 0x33, 0x88, 0x07, 0xC7, 0x31, 0xB1, 0x12, 0x10, 0x59, 0x27, 0x80, 0xEC, 0x5F,
    0x60, 0x51, 0x7F, 0xA9, 0x19, 0xB5, 0x4A, 0x0D, 0x2D, 0xE5, 0x7A, 0x9F, 0x93, 0xC9, 0x9C, 0xEF,
    0xA0, 0xE0, 0x3B, 0x4D, 0xAE, 0x2A, 0xF5, 0xB0, 0xC8, 0xEB, 0xBB, 0x3C, 0x83, 0x53, 0x99, 0x61,
    0x17, 0x2B, 0x04, 0x7E, 0xBA, 0x77, 0xD6, 0x26, 0xE1, 0x69, 0x14, 0x63, 0x55, 0x21, 0x0C, 0x7D};

int NisecCntaxLogBufXor(uint8 bVendorCode, uint8 bRawBufIndex1, uint8 *pInBuf, int nInBufSize,
                        uint8 *pOutBuf)
{
    // bVendorCode cntax 0x05; nisec 0x03
    int i = 0;
    uint8 nKeyCode = bRawBufIndex1 - bVendorCode * 16;
    for (i = 0; i < nInBufSize; ++i) {
        *(_BYTE *)(pOutBuf + i) = *(&byte_A6F940[16 * nKeyCode] + i % 0xF) ^ *(_BYTE *)(i + pInBuf);
        *(_BYTE *)(pOutBuf + i) ^= *(&byte_A6FA40[16 * (bVendorCode & 0xF)] + i % 0xF);
    }
    return i;
}

int DecryptLogLine(uint8 bVendorCode, char *szEnc, int nEncSize, char *szDecLineBuf)
{
    uint8 *pTmpBuf = NULL;
    int nRet = ERR_GENERIC, nChildRet = 0, nRawDummySize = 12;

    if (strncmp(szEnc, "FF99011099FF", nRawDummySize)) {
        _WriteLog(LL_WARN, "Not encstring");
        return ERR_BUF_CHECK;
    }
    if (strlen(szEnc) < 16 || szEnc[12] != '0' || szEnc[13] != '1') {
        _WriteLog(LL_WARN, "Encrypt version failed");
        return ERR_BUF_CHECK;
    }
    do {
        if (!(pTmpBuf = calloc(1, 2 * nEncSize + 256)))
            break;
        if ((nChildRet = Str2Byte(pTmpBuf, szEnc + nRawDummySize, nEncSize - nRawDummySize)) < 2)
            break;
        nRet = NisecCntaxLogBufXor(bVendorCode, pTmpBuf[1], pTmpBuf + 2, nChildRet - 2,
                                   (uint8 *)szDecLineBuf);
    } while (false);
    if (pTmpBuf)
        free(pTmpBuf);
    return nRet;
}

int PeekEncrypLogToWrite(uint8 bVendorCode, FILE *fp, char *szEncLine, char *szDecLineBuf)
{
    int nChildRet = 0;
    char szFlag[] = "FF99011099FF";
    strcpy(szDecLineBuf, szEncLine);
    char *p = strstr(szEncLine, szFlag);
    do {
        if (!p)
            break;
        char *pDec = strstr(szDecLineBuf, szFlag);
        if ((nChildRet = DecryptLogLine(bVendorCode, p, strlen(p), pDec)) < 0)
            continue;
        pDec[nChildRet] = '\0';
    } while (false);
    strcat(szDecLineBuf, "\r\n");
    if (fwrite(szDecLineBuf, strlen(szDecLineBuf), 1, fp) < 0)
        return ERR_IO_FAIL;
    return RET_SUCCESS;
}

int DecryptLogToFile(uint8 bVendorCode, char *szEnc, int nEncSize, FILE *fp)
{
    int nRet = RET_SUCCESS, nChildRet = 0;
    char *szDecLineBuf = NULL;
    SplitInfo spOut;
    memset(&spOut, 0, sizeof(SplitInfo));
    if (!(szDecLineBuf = calloc(1, nEncSize + 256)))
        return ERR_BUF_ALLOC;
    GetSplitString(szEnc, "\r\n", &spOut);
    SplitInfo *pOut = &spOut;
    int i;
    for (i = 0; i < spOut.num; i++) {
        pOut = pOut->next;
        char *pChild = pOut->info;
        if ((nChildRet = PeekEncrypLogToWrite(bVendorCode, fp, pChild, szDecLineBuf)) < 0) {
            nRet = nChildRet;
            break;
        }
    }
    SplitFree(&spOut);
    if (szDecLineBuf)
        free(szDecLineBuf);
    return nRet;
}

//只能解密日志中每行包含FF99011099FF开头的日志数据，解密输出到当前文件path.txt_dec.txt
// bVendor: nisec = 1, cntax = 2
//可以解密，类似cntax的smssl_2021-11-31.log、nisec的skpc_2022-01-01.log，rsassl_2022-01-01.log
int CallNisecCntaxDecryptLog(uint8 bVendor, char *szEncFilePath)
{
    uint8 bVendorCode = 0;
    char *szEnc = NULL;
    int nEncSize = 0;
    int nRet = ERR_GENERIC;
    char szDecPath[512] = "";
    snprintf(szDecPath, sizeof(szDecPath), "%s_dec.txt", szEncFilePath);
    FILE *fp = NULL;
    do {
        if (bVendor == 1)  // nisec
            bVendorCode = 0x03;
        else if (bVendor == 2)  // cntax
            bVendorCode = 0x05;
        else
            break;
        if ((nEncSize = ReadBinFile(szEncFilePath, (void **)&szEnc, 0)) < 16) {
            _WriteLog(LL_FATAL, "Read srcfile:%s failed", szEncFilePath);
            break;
        }
        if (!(fp = fopen(szDecPath, "w"))) {
            _WriteLog(LL_FATAL, "Open decfile:%s failed", szDecPath);
            break;
        }
        if (DecryptLogToFile(bVendorCode, szEnc, nEncSize, fp) < 0) {
            _WriteLog(LL_FATAL, "ODecryptLogToFile failed");
            break;
        }
        nRet = RET_SUCCESS;
    } while (false);
    if (szEnc)
        free(szEnc);
    if (fp)
        fclose(fp);
    _WriteLog(LL_INFO, "CallNisecCntaxDecryptLog finish, ret:%d", nRet);
    return nRet;
}
//--------------------------nisec 20220401新增加密 begin------------------------------------
int Nisec20220401PreHashXor(uint8 *a1, uint32 a2, uint8 *a3)
{
    // a1首字节必须为1
    unsigned int v4;
    unsigned int v6;
    int v7;
    char v8;
    int a4 = 0;
    v4 = (unsigned int)*(_BYTE *)(a1 + 1) >> 4;
    v6 = 0;
    v7 = 16 * (*(_BYTE *)(a1 + 1) & 0xF);
    do {
        v8 = *(_BYTE *)(v6 + a1 + 2) ^ *(&byte_A6F940[v7] + v6 % 0xF);
        *(_BYTE *)(v6 + a3) = v8;
        *(_BYTE *)(v6 + a3) = v8 ^ *(&byte_A6FA40[16 * (v4 & 0xF)] + v6 % 0xF);
        ++v6;
    } while (v6 < a2);
    a4 = a2 - 2;
    return a4;
}

// abOutKey > 16bytes
void Build16BytesSM4Key(char *szDeviceID, char *szFphm, uint8 *abOutKey)
{
    int v9 = 0;
    // nisec是通过preValue算法计算出来的，cntax直接填的计算出来的静态值
    uint8 abStaticBuf[18] = {0};  //和data18Bytes大小一致
    uint8 abDynamicBuf[16] = {0};
    unsigned char data18Bytes[18] = {0x01, 0x7E, 0xCD, 0x9A, 0x48, 0xA6, 0xD0, 0x9F, 0x36,
                                     0xC6, 0x46, 0x3C, 0x85, 0x9D, 0x22, 0xC5, 0x9C, 0x8F};
    Nisec20220401PreHashXor(data18Bytes, sizeof(data18Bytes), abStaticBuf);
    // abStaticBuf="sksbkprjbbhjmcc1"
    //最大拷贝前8位,多余的不管
    strncpy((char *)abDynamicBuf, szFphm, 8);
    strncpy((char *)abDynamicBuf + 8, szDeviceID + 4, 8);
    do {
        abDynamicBuf[v9] ^= abStaticBuf[v9];
        ++v9;
    } while (v9 < 16);
    memcpy(abOutKey, abDynamicBuf, 16);
}

// nisec官方将总局版本号19位写成静态值;abOutPreBuf >=32bytes
void Build32BytesToEncryptBuf(char *szZjbbh, char *szFpdm, char *szFphm, char *szKprj,
                              uint8 *abOutPreBuf)
{
    int v9 = 0;
    int xorSize = 19;  // 0x13固定值，测试于20220420
    uint8 abStaticBuf[32] = {0};
    uint8 abDynamicBuf[32] = {0};
    strcpy((char *)abStaticBuf, szZjbbh);  //最后都补0x80，实际只用19位，最后保留16位
    abStaticBuf[strlen(szZjbbh)] = 0x80;
    //最大拷贝前8位,多余的不管
    strncpy((char *)abDynamicBuf, szFphm, 8);  //最大拷贝前8位
    strncpy((char *)abDynamicBuf + 8, szKprj, 8);
    strncpy((char *)abDynamicBuf + 16, szFpdm, strlen(szFpdm));
    memset(abDynamicBuf + xorSize, 0, sizeof(abDynamicBuf) - xorSize);
    do {
        abStaticBuf[v9] ^= abDynamicBuf[v9];
        ++v9;
    } while (v9 < xorSize);
    memcpy(abOutPreBuf, abStaticBuf, 0x20);
}

// szKjrq yyyymmdd;
void NisecKjbbhEncrypt(char *szZjbbh, char *szFdm, char *szFphm, char *szDevId, char *szKjrq,
                       uint8 *abOutBuf)
{
    uint8 abSM4Key[16] = {0};
    uint8 abToEncBuf[32] = {0};
    uint8 abOutBufBytes[32] = {0};
    Build16BytesSM4Key(szDevId, szFphm, abSM4Key);
    Build32BytesToEncryptBuf(szZjbbh, szFdm, szFphm, szKjrq, abToEncBuf);
    SM4Algorithm(1, abToEncBuf, sizeof(abToEncBuf), (char *)abSM4Key, abOutBufBytes);
    Byte2Str((char *)abOutBuf, abOutBufBytes, sizeof(abOutBufBytes));
}

//--------------------------底层驱动版本号前四位1106-2000，前0x60字节加密------------------------------
int NisecKpStringHead96bytesSM3Calc(uint8 *abPreIV, uint8 *szOutBuf)
{
    uint8 ab0x80Buf[0x80] = {0}, abBuf[128] = {0}, abBuf2[128] = {0};
    memset(ab0x80Buf, 0x36u, 0x40u);
    memset(ab0x80Buf + 0x40, 0x5Cu, 0x40u);
    //先根据0x80内存排列计算出初始索引表
    uint8 *v5 = (_BYTE *)(ab0x80Buf + 0x40);
    uint8 *v6 = abPreIV;
    int nPreIVHeadSize = 0x10;
    int i = 0;
    do {
        *(v5 - 64) ^= *v6;
        *v5++ ^= *v6++;
    } while (++i < nPreIVHeadSize);  // 16次
    //索引表前0x40和preiv后16，合并计算SM3-1
    memcpy(abBuf, ab0x80Buf, 0x40);
    memcpy(abBuf + 0x40, abPreIV + 0x10, 0x10);
    CalcSM3(abBuf, 0x50, abBuf2 + 0x40);
    //索引表后0x40和SM3-1结果生成SM3-2，作为函数输出
    memcpy(abBuf2, ab0x80Buf + 0x40, 0x40);
    CalcSM3(abBuf2, 0x60, szOutBuf);
    return 0x20;  // sm3 hash-size
}

void NisecKpStringHead96bytesDynamicSeed(char *szDeviceID, uint8 *szOutBuf, uint8 bVersion)
{
    uint8 szBuf[64] = {0}, szBuf2[32] = {0}, abPre32Bytes[32] = {0};
    int a2 = bVersion;
    GetHead0x60DevIdSeed(szDeviceID, szBuf);
    //根据设备号种子生成abPre32Bytes 32字节
    int v4 = 0, v8 = 0;
    int v5 = 16 * (*(szBuf + 5 + 8 * a2 + 2) & 0xF);
    uint8 v6 = 0;
    do {
        v6 = *(szBuf + v4) ^ *(byte_A6F940 + v5 + v4);
        *(szBuf2 + ++v4 + 3) = v6;
    } while (v4 < 16);
    memcpy(abPre32Bytes, byte_A6FA40 + v5, 16);
    memcpy(abPre32Bytes + 16, szBuf2 + 4, 16);
    //由abPre32Bytes生成32字节综合SM3hash数据
    v8 = NisecKpStringHead96bytesSM3Calc(abPre32Bytes, szBuf2);
    //由32字节sm3hash生成最终动态种子
    int result = 0;
    do {
        int v10 = (signed int)result % 16;
        uint8 v11 = *(result + szBuf2);
        result++;
        *(szOutBuf + v10) ^= v11;
    } while (result < v8);
}

bool Nisec3DesCbcEncryptMultiBytes(uint8 *pInputData, int nTotalInputBytes,
                                   uint8 *pabStatic16BytesKey, uint8 *pOutBuf)
{
    int nIndex = 0;
    int nCalcSize = nTotalInputBytes;
    do {
        if (Nisec3DesCbcEncrypt8Bytes(pInputData + nIndex, pabStatic16BytesKey, pOutBuf + nIndex))
            break;
        nIndex += 8;
        nCalcSize -= 8;
    } while (nCalcSize);
    return nIndex == nTotalInputBytes;
}

//返回34字节
int NisecBuildNewAuthKey(uint8 bVersion, char *szDeviceID, uint8 *szOutBuf)
{
    // abBuf1 abBuf2 至少32字节
    uint8 abBuf1[32] = {0}, abBuf2[32] = {0}, abIvKey[16] = {0};
    unsigned char data18BytesAlgNum3[18] = {0x01, 0x7B, 0x99, 0x5C, 0xB8, 0x5C, 0x3B, 0xEA, 0xEB,
                                            0xDF, 0xC8, 0x11, 0x5F, 0xAE, 0xDF, 0xDD, 0x4B, 0x40};
    GetHead0x60DevIdSeed(szDeviceID, abBuf2);
    if (bVersion)  //此处bVersion为静态值0无误
        NisecKpStringHead96bytesDynamicSeed(szDeviceID, abBuf1, 0);
    else  //输出为16字节,算法序号3
        Nisec20220401PreHashXor(data18BytesAlgNum3, sizeof(data18BytesAlgNum3), abBuf1);

    // 3DES加密过程为：C=Ek3(Dk2(Ek1(P)))
    if (!Nisec3DesCbcEncryptMultiBytes(abBuf2, 16, abBuf1, abIvKey))
        return ERR_GENERIC;
    //计算inputdata
    memset(abBuf2, 0, sizeof(abBuf2));
    NisecKpStringHead96bytesDynamicSeed(szDeviceID, abBuf2, bVersion);
    memset(abBuf1, 0, sizeof(abBuf1));  //至少需要0x20 32个字节
    abBuf1[0] = 0x54;
    abBuf1[1] = 0x01;
    abBuf1[8] = 0x10;
    abBuf1[25] = 0x80;
    memcpy(abBuf1 + 9, abBuf2, 16);
    //生成32字节新安全密钥
    memset(abBuf2, 0, sizeof(abBuf2));
    if (!Nisec3DesCbcEncryptMultiBytes(abBuf1, 32, abIvKey, abBuf2))
        return ERR_GENERIC;
    //生成验证码，ff27 static
    uint16 v3 = bswap_16(NisecBuildVCodeXorNum(0xff27, abBuf2, 32));

    memcpy(szOutBuf, abBuf2, 0x20);
    memcpy(szOutBuf + 0x20, &v3, 2);
    return 0x22;
}

//和AisinoDesEcbEncrypt有点类似，非通用长输入buf计算3Des，而是采用只计算8位的方式进行
int Nisec3DesCbcEncrypt8Bytes(uint8 *toEncBuf8Bytes, uint8 *key, uint8 *outBuf8Bytes)
{
    int len = 0, nRet = ERR_GENERIC;
    EVP_CIPHER_CTX *ctx = NULL;
    if (!(ctx = EVP_CIPHER_CTX_new()))
        return ERR_PARM_INIT;
    while (1) {
        if (!EVP_EncryptInit_ex(ctx, EVP_des_ede_cbc(), NULL, key, NULL))
            break;
        if (!EVP_EncryptUpdate(ctx, outBuf8Bytes, &len, toEncBuf8Bytes, 8))
            break;
        //实际只有8位，无尾部填充数据，因此无需 EVP_EncryptFinal_ex
        nRet = RET_SUCCESS;
        break;
    }
    EVP_CIPHER_CTX_free(ctx);
    return nRet;
}

int NisecKpStringHead96bytesSeed32Bytes(char *szDeviceID, uint8 *pBuf)
{
    // abStaticBuf 至少18字节，需要和data18BytesAlgNum5空间大小一致
    uint8 abStaticBuf[18] = {0}, abDynamicBuf[16] = {0};
    // 1)获取staticbuf,序号5的算法种子
    unsigned char data18BytesAlgNum5[18] = {0x01, 0x7B, 0x3B, 0xB0, 0x88, 0x7E, 0xC0, 0xF6, 0x27,
                                            0x40, 0x69, 0x98, 0x7D, 0x2A, 0x63, 0x63, 0xE3, 0x50};
    //输出为16字节
    Nisec20220401PreHashXor(data18BytesAlgNum5, sizeof(data18BytesAlgNum5), abStaticBuf);
    // 2)获取dynamicbuf
    NisecKpStringHead96bytesDynamicSeed(szDeviceID, abDynamicBuf, 1);
    // 3) 3DES加密过程为：C=Ek3(Dk2(Ek1(P)))
    if (!Nisec3DesCbcEncryptMultiBytes(abStaticBuf, 16, abDynamicBuf, pBuf))
        return ERR_GENERIC;
    return RET_SUCCESS;
}

int GetKpHead96BytesXorBin(uint8 *pBuf, uint8 *szOutBuf)
{
    if (!Str2Byte(szOutBuf, (char *)pBuf, 0x60))
        return -1;
    uint8 *v2 = szOutBuf;
    uint8 *v27 = szOutBuf + 40;
    int v3 = 5;
    do {
        int v4 = 0;
        do {
            v2[v4] ^= v27[v4];
            ++v4;
        } while (v4 < 8);
        v2 += 8;
        --v3;
    } while (v3);
    return 0;
}

int EncryptGroupHead96Bytes(uint8 *src, int nSrcSize, uint8 *key, uint8 *outBuf)
{
    uint8 *v6 = src;
    uint8 *v7 = outBuf;
    do {
        if (Nisec3DesCbcEncrypt8Bytes(v6, key, v7))
            return -1;
        v6 += 8;
        v7 += 8;
        nSrcSize -= 8;
    } while (nSrcSize > 0);
    return 0;
}

//==0 小于1106;>0 大于;< 0 2000以上，分析程序处理
int NisecDriverVersionNumAbove1106(char *szDriverVersion)
{
    // 202212月份，官方代码调整为 v492 > 1106 && v492 < 2000
    int nRet = 0;
    char szBuf[64] = {0};
    strncpy((char *)szBuf, szDriverVersion, 4);
    int nDriverVersionNum = atoi((char *)szBuf);
    //截止20220923，官方目前仅有1xxxyyyymmdd版本号
    if (nDriverVersionNum <= 1106)  // 1106及以下版本无需加密
        nRet = 0;
    //不支持新版本，遇到情况待处理,截至2022年9月，最高驱动版本尚无2000以上
    else if (nDriverVersionNum >= 2000)
        nRet = ERR_PROGRAME_UNSUPPORT;
    else
        nRet = 1;
    return nRet;
}

//高版本驱动设备才加密
int NisecEncryptMakeInvoiceHeadBuffer0x60Bytes(HDEV hDev, char *pBuf)
{
    uint8 szBuf[256] = {0}, szBuf2[256] = {0}, sz32BytesSeed[32] = {0};
    int nRet = ERR_GENERIC;
    do {
        if ((nRet = NisecDriverVersionNumAbove1106(hDev->szDriverVersion)) <= 0)
            break;  //过低或者过高都不处理
        if (NisecKpStringHead96bytesSeed32Bytes(hDev->szDeviceID, sz32BytesSeed)) {
            _WriteLog(LL_FATAL, "NisecKpStringHead96bytesSeed32Bytes failed");
            break;
        }
        if (GetKpHead96BytesXorBin((uint8 *)pBuf, szBuf)) {
            break;
        }
        if (EncryptGroupHead96Bytes(szBuf, 0x30, sz32BytesSeed, szBuf2))
            break;
        //二进制转字符串处理，并拷贝到原发票buf进行等长度字符串覆盖
        memset(szBuf, 0, sizeof(szBuf));
        Byte2Str((char *)szBuf, szBuf2, 0x30);
        mystrupr((char *)szBuf);
        if (strlen((char *)szBuf) != 0x30 * 2)
            break;
        strncpy(pBuf, (char *)szBuf, 0x60);
        nRet = RET_SUCCESS;
    } while (false);
    return nRet;
}
