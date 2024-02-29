/*****************************************************************************
File name:   aisino_algorithm.c
Description: 该文件主要存放软件中逆向出来的算法结构，统一放在这里就不管他了
             统一存放也便于其他模块调用，该文件中不得存放非算法函数
Author:      Zako
Version:     1.0
Date:        2019.05.18
History:
20190518     最初代码实现
*****************************************************************************/
#include "aisino_algorithm.h"

// Start----------------------------------------发票第一段解密算法----------------------------------------
int sub_39E2630(uint8 a2, uint8 a3, unsigned __int8 a4)
{
    __int64 v4;
    LODWORD(v4) = 80;
    if (a2) {
        v4 = 0x10;  //一般情况下为0x10  //??
    }
    if (a3) {
        LODWORD(v4) = v4 + 16;
    }
    if (a4 != 17) {
        if (a4 == 13) {
            LODWORD(v4) = v4 + 1;
            return v4;
        }
        if (a4 == 6) {
            LODWORD(v4) = v4 + 2;
            return v4;
        }
        if (a4 == 4) {
            LODWORD(v4) = v4 + 3;
            return v4;
        }
        if (a4 == 5) {
            LODWORD(v4) = v4 + 4;
            return v4;
        }
        switch (a4) {
            case 0xBu:
                LODWORD(v4) = v4 + 6;
                break;
            case 3u:
                LODWORD(v4) = v4 + 7;
                break;
            case 0x10u:
                LODWORD(v4) = v4 + 8;
                break;
            case 0xAu:
                LODWORD(v4) = v4 + 9;
                break;
            default:
                LODWORD(v4) = v4 + 5;
                break;
        }
    }
    return v4;
}

int sub_39E2830(long a1, long a2)
{
    long v2;           // ebx@1
    unsigned int v3;   // ecx@1
    _BYTE *v4;         // esi@1
    signed int v5;     // edi@1
    int v6;            // edx@3
    signed int v7;     // ebp@3
    _BYTE *v8;         // edi@3
    signed int v9;     // esi@3
    uint8 v10;         // cl@4
    unsigned int v11;  // eax@4
    signed int v12;    // edx@7
    int result;        // eax@14
    uint8 a0123456789abcd[64] = "0123456789ABCDEFGHJKLMNPQRTUVWXY";

    v2 = a2;
    v3 = *(_BYTE *)a2 + ((*(_BYTE *)(a2 + 1) + ((*(_BYTE *)(a2 + 2) & 0xF) << 8)) << 8);
    v4 = (_BYTE *)(a1 + 5);
    v5 = 6;
    do {
        *v4-- = v3 % 0xA + 48;
        --v5;
        v3 /= 0xAu;
    } while (v5);
    v6 = 0;
    v7 = 0;
    v8 = (_BYTE *)(a2 + 3);
    v9 = 8;
    do {
        v10 = v6;
        v6 += 5;
        v11 = ((unsigned int)*v8 >> v10) & 0x1F;
        v9 -= 5;
        if (v9 < 0) {
            v6 -= 8;
            v9 += 8;
            v11 += (*++v8 << v9 >> 3) & 0x1F;
            v2 = a2;
        }
        *(_BYTE *)(a1 + v7++ + 6) = a0123456789abcd[v11];
    } while (v7 < 14);
    v12 = 15;
    switch ((unsigned int)*(_BYTE *)(v2 + 11) >> 6) {
        case 1u:
            v12 = 18;
            break;
        case 2u:
            v12 = 20;
            break;
        case 3u:
            v12 = 17;
            break;
    }
    if (v12 >= 20) {
        result = a1;
    } else {
        memset((void *)(v12 + a1), 0x20u, 20 - v12);
        result = a1;
    }
    return result;
}

int sub_39D5EE0(unsigned __int8 a1)
{
    int result;  // eax@2

    if (a1 <= 9u)
        result = a1 + 48;
    else
        result = a1 + 87;
    return result;
}
int sub_39D5E90(long a1, long a2, int a3)
{
    int result;  // eax@1
    int v4;      // esi@1
    long v5;     // edi@2

    result = a3;
    v4 = 0;
    if (a3 > 0) {
        v5 = a1;
        do {
            *(_BYTE *)v5 = sub_39D5EE0(*(_BYTE *)(v4 + a2) >> 4);
            *(_BYTE *)(v5 + 1) = sub_39D5EE0(*(_BYTE *)(v4 + a2) & 0xF);
            result = a3;
            ++v4;
            v5 += 2;
        } while (v4 < a3);
    }
    return result;
}

// 暂时不明白与标准xtoa差异，暂不改动
uint8 aisino_private_xtoa(unsigned int a1, _BYTE *a2, unsigned int a3, int a4)
{
    uint8 *v4;
    unsigned int v5;
    uint8 *v6;
    int v7;
    bool v8;
    bool v9;
    uint8 v10;
    uint8 *v11;
    uint8 result;

    v4 = (uint8 *)a2;
    if (a4) {
        *a2 = 45;
        v4 = (uint8 *)a2 + 1;
        v5 = -a1;
    } else {
        v5 = a1;
    }
    v6 = v4;
    do {
        v7 = v5 % a3;
        v8 = v5 % a3 < 9;
        v9 = v5 % a3 == 9;
        v5 /= a3;
        if (v8 || v9)
            v10 = v7 + 48;
        else
            v10 = v7 + 87;
        *v4++ = v10;
    } while (v5);
    *v4 = 0;
    v11 = v4 - 1;
    do {
        result = *v11;
        *v11 = *v6;
        *v6 = result;
        --v11;
        ++v6;
    } while (v6 < v11);
    return result;
}

signed int sub_39DB320(uint8 *pthis, long a2, long a3)
{
    uint8 *v3;
    signed int result = 0;
    signed int v5;
    signed int v6;
    uint8 v7;
    unsigned int v8;
    unsigned int v9;
    signed int v10;
    uint8 v11;
    long v12;
    uint8 v13[12];

    v3 = pthis;
    if ((*(_BYTE *)a3 & 0xC0) == 0xC0) {
        aisino_private_xtoa(*(_DWORD *)(a3 + 1), (uint8 *)v13, 10, 0);
        v5 = 0;
        v6 = 10 - strlen((char *)v13);
        do {
            if (v5 >= v6)
                v7 = v13[v5 - v6];
            else
                v7 = 48;
            *(_BYTE *)(v5++ + a2) = v7;
        } while (v5 < 5);
        v8 = *(_BYTE *)a3 & 0x3F;
        *(_BYTE *)(a2 + 5) = (*(_BYTE *)a3 & 0x3Fu) / 0xA + 48;
        v9 = v8 % 0xA;
        v10 = 5;
        *(_BYTE *)(a2 + 6) = v9 + 48;
        do {
            if (v10 >= v6)
                v11 = v13[v10 - v6];
            else
                v11 = 48;
            *(_BYTE *)(a2 + v10++ + 2) = v11;
        } while (v10 < 10);
        *(_BYTE *)(a2 + 12) = 0;
        v12 = (long)(v3 + 47);
        *(_DWORD *)v12 = *(_DWORD *)a3;
        result = 12;
        *(_BYTE *)(v12 + 4) = *(_BYTE *)(a3 + 4);
    } else {
        sub_39D5E90(a2, a3, 5);
        *(_BYTE *)(a2 + 10) = 0;
        result = 10;
    }
    return result;
}

int Block1PreDecrypt(long a1, long a3, long a4)
{
    long v4;
    long v5;
    long v6;
    signed int v7;
    unsigned int v8;
    _DWORD *v14;
    uint8 v15;
    long v16;
    long v17;
    long v18;
    int result;
    _DWORD *v20;

    v4 = a4;
    v5 = a1;
    *(_BYTE *)a3 =
        sub_39E2630(~*(_BYTE *)a4 & 1, (*(_BYTE *)a4 >> 4) & 1, *(_BYTE *)(a4 + 21) / 10);
    v6 = a3 + 5;
    *(_DWORD *)(a3 + 1) = *(_DWORD *)(v4 + 2) + ((unsigned int)*(_BYTE *)(v4 + 20) >> 4 << 28);
    *(_DWORD *)v6 = *(_DWORD *)(a4 + 11);
    *(_BYTE *)(v6 + 4) = *(_BYTE *)(a4 + 15);
    *(_DWORD *)(a3 + 10) = *(_DWORD *)(a4 + 16);
    LOBYTE(v6) = *(_BYTE *)(a4 + 23) >> 4;
    v7 = (unsigned __int16)((unsigned __int8)v6 + 16 * *(_BYTE *)(a4 + 22));
    v8 = (signed int)(0x51EB851F * (unsigned __int64)(unsigned __int16)((unsigned __int8)v6 +
                                                                        16 * *(_BYTE *)(a4 + 22)) >>
                      32) >>
         5;
    *(_BYTE *)(a3 + 14) = (v8 >> 31) + v8;
    *(_BYTE *)(a3 + 15) = v7 % 100;
    *(_BYTE *)(a3 + 16) = (*(_BYTE *)(v4 + 23) & 0xF) + 16 * (*(_BYTE *)(v4 + 20) & 0xF);
    *(_BYTE *)(a3 + 17) = *(_BYTE *)(a4 + 24) >> 3;
    v20 = (_DWORD *)(a4 + 25);
    if ((BYTE2(*v20) & 0xF0) == 16)
        BYTE2(*v20) &= 0xFu;
    sub_39E2830(a3 + 18, (long)v20);
    v14 = (_DWORD *)(a3 + 38);
    v15 = (char)sub_39DB320((uint8 *)v5, a3 + 38, a4 + 6);
    *(_BYTE *)(v5 + 70) = v15;
    if (v15 == 12) {
        v16 = v5 + 47;
        v17 = a3 + 39;
        *v14 = 0;
        *(_DWORD *)(a3 + 42) = 0;
        *(_WORD *)(a3 + 46) = 0;
        *(_BYTE *)v14 = -1;
        *(_DWORD *)v17 = *(_DWORD *)v16;
        *(_BYTE *)(v17 + 4) = *(_BYTE *)(v16 + 4);
    }
    v18 = a3 + 48;
    *(_DWORD *)v18 = 0x30303030;
    *(_DWORD *)(v18 + 4) = 0x30303030;
    *(_DWORD *)(v18 + 8) = 0x30303030;
    result = *(_BYTE *)(a4 + 37);
    *(_DWORD *)(a3 + 60) = result;
    *(_BYTE *)(a3 + 64) = *(_BYTE *)(a4 + 1);
    return result;
}

long UnMaskBlock1Number(long a1, _BYTE *a2, uint8 a3)
{
    long result;
    uint8 v4;
    uint8 v5;
    uint8 v6;
    _BYTE *v7;
    result = a1;
    v4 = 1;
    while (v4 != a3) {
        if (*(_BYTE *)result != 10) {
            do v5 = *(_BYTE *)(result++ + 1);
            while (v5 != 10);
        }
        ++v4;
        ++result;
    }
    v6 = *(_BYTE *)result;
    int nT = 0;
    if (*(_BYTE *)result != 10) {
        v7 = a2;
        do {
            *v7 = v6;
            v6 = *(_BYTE *)(result + 1);
            ++v7;
            ++result;
            nT++;
        } while (v6 != 10);
    }
    return result;
}

//需要block2数据
int DecryptBlock1(long a1, long szRet, long pInv, int nDecInvoiceBlock2Len)
{
    Block1PreDecrypt(a1, szRet, pInv);
    memset((void *)(szRet + 0x12), 0, 20);
    //兼容特殊发票前缀有乱码可能存在0x0a字符导致后续切割错误
    if (strstr((char *)pInv + 0x41, "BarcodeKey") != NULL) {
        UnMaskBlock1Number(pInv + 0x41 + 32, (_BYTE *)(szRet + 0x12), 6);
    } else {
        UnMaskBlock1Number(pInv + 0x41, (_BYTE *)(szRet + 0x12), 6);
    }

    // 0x00 0x00是发票正文块（详细+密文校验码）长度，在block3结尾处计算并填充，此处先置0
    // DecodeInvoiceBlock3 nOldTypeBlockBufLen
    memcpy((void *)(szRet + 0x30), "\x4C\x00\x00", 3);
    // 0x6c 密文校验码长度??
    int n0x39Flag = (0x6c + nDecInvoiceBlock2Len + 0x14) >> 16;
    ((uint8 *)szRet)[0x39] = (uint8)n0x39Flag;
    return 0;
}
// Finish----------------------------------------发票第一段解密算法----------------------------------------
// Start----------------------------------------发票第二段解密算法----------------------------------------
int My_Func_5B441F0(_WORD *this, int a2, unsigned __int16 a3)
{
    unsigned __int16 v3;
    int v4;
    int v5;
    int result;
    v3 = a3;
    if (a3 < 0x103u) {
        this[(unsigned __int16)a2] = a3;
        result = a2 + 1;
    } else {
        v4 = a2;
        do {
            v5 = (unsigned __int16)v4++;
            this[v5] = *((_BYTE *)this + v3 + 20084);
            v3 = this[v3 + 5021];
        } while (v3 >= 0x103u);
        this[(unsigned __int16)v4] = v3;
        result = v4 + 1;
    }
    return result;
}

int Block2_WriteblockToDstbuff(long this, long a2, int a3)
{
    // this + 25652 为返回buff地址，应为32或者64位
    int i;
    for (i = 0; i < a3; ++i) {
        *(_BYTE *)(*(long *)(this + 25652) + *(DWORD *)(this + 25660) + i) = *(_BYTE *)(i + a2);
        // _WriteLog(LL_DEBUG, "pRet:%p d1:%d d2:%d p2:%p\n", *(long *)(this + 25652),
        //           *(DWORD *)(this + 25660), i,
        //           (long *)(*(long *)(this + 25652) + *(DWORD *)(this + 25660) + i));
    }
    *(_DWORD *)(this + 25660) += a3;
    return a3;
}

__int16 Block2DecodeBuff(long this, uint8 a2)
{
    long v2;
    int v3;
    uint8 *v4;
    v2 = this;
    v3 = *(_WORD *)(this + 25648);
    if ((unsigned __int16)v3 == 256) {  //将解密的小段（256个字节），写到缓冲区
        v4 = (uint8 *)(this + 25388);
        LOWORD(v3) = Block2_WriteblockToDstbuff(this, this + 25388, 256);
        *(_WORD *)(v2 + 25648) = 1;
        *v4 = a2;
    } else {  // uint8 by uint8 逐个解密
        *(_BYTE *)(v3 + this + 25388) = a2;
        ++*(_WORD *)(this + 25648);
    }
    return v3;
}

//将100个字节（或者剩余字节）拷贝到临时缓冲区this+25112，并累加计数
__int16 My_Func_5B44620(long this, long a2, unsigned int a3)
{
    signed int v3;
    signed int v4;
    long v5;
    int result;

    v3 = a3;
    if (*(_DWORD *)(this + 25664) - *(_DWORD *)(this + 25384) < a3)
        v3 = *(_DWORD *)(this + 25664) - *(_DWORD *)(this + 25384);
    v4 = 0;
    if (v3 <= 0) {
        *(_DWORD *)(this + 25384) += v3;
        result = v3;
    } else {
        do {
            v5 = v4++ + *(long *)(this + 25376);
            *(_BYTE *)(v4 + a2 - 1) = *(_BYTE *)(v5 + *(_DWORD *)(this + 25384));
        } while (v4 < v3);
        *(_DWORD *)(this + 25384) += v3;
        result = v3;
    }
    return result;
}

uint8 My_Func_5B44110(long this, _WORD *a2)
{
    long v2;
    __int16 v3;

    v2 = this;
    if (*(_WORD *)(this + 25374)) {
        ++*(_WORD *)(this + 25372);
    } else {
        v3 = My_Func_5B44620(this, this + 25112, 0x100u);
        *(_WORD *)(v2 + 25374) = v3;
        if (!v3) {
            *(_WORD *)(v2 + 25374) = 0;
            *(_WORD *)(v2 + 25372) = 0;
            return 0;
        }
        *(_WORD *)(v2 + 25372) = 1;
    }
    --*(_WORD *)(v2 + 25374);
    *a2 = *(_BYTE *)(*(_WORD *)(v2 + 25372) + v2 + 25111);
    return 1;
}

uint16 My_Func_5B44260(long a1)
{
    long v1 = 0;
    unsigned __int16 *v2 = NULL;
    int v3 = 0;
    int v4 = 0;
    int v5 = 0;
    int v6 = 0;
    int v7 = 0;
    __int16 v8 = 0;
    __int16 result = 0;
    int v10 = 0;
    unsigned __int16 v11 = 0;
    WORD v12 = 0;

    v1 = a1;
    My_Func_5B44110(a1, (uint16 *)&v12);
    v2 = (unsigned __int16 *)(v1 + 25370);
    LOWORD(v3) = *(_WORD *)(v1 + 25368);
    v5 = 0;
    BYTE1(v5) = *(_BYTE *)(v1 + 25370);
    v4 = (unsigned __int16)v5 | v12;
    LOWORD(v5) = *(_WORD *)(v1 + 25108);
    v6 = *(_WORD *)(v1 + 25368);
    v7 = *(_WORD *)(v1 + 25108);
    v12 = v4;
    if (v6 >= v7 - 8) {
        v10 = v3 - v5 + 8;
        *(_WORD *)(v1 + 25368) = v10;
        v11 = (unsigned __int16)(v4 << (16 - v10)) >> (16 - v10);
        result = (unsigned __int16)v12 >> v10;
        *v2 = v11;
    } else {
        *(_WORD *)(v1 + 25368) = v3 - v5 + 16;
        My_Func_5B44110(v1, (_WORD *)(v1 + 25370));
        v8 = *(_WORD *)(v1 + 25368);
        LOWORD(v1) = *v2 >> v8;
        v12 = v1 | (v12 << (8 - v8));
        *v2 = (unsigned __int16)(*v2 << (16 - v8)) >> (16 - v8);
        result = v12;
    }
    return result;
}

signed int Block2_ZeroVar(BYTE *this)
{
    //内存布局
    // 0-25105全部置0作为缓冲区
    // 20106-25664 全部作为函数内局部变量使用
    //次重要地址为
    // 25652
    // 25660
    //其中较为重要的三个地址变量为
    // 25664(4byte)= dwDecodeBuffLen/dwEncodeBuffLen 输入缓冲的长度
    // 25376(8/4byte)= 输入缓冲区地址
    // 25652(8/4byte)= 输出缓冲区地址
    signed int result;
    BYTE *v2;
    result = 0;
    v2 = this;
    do {
        *(_WORD *)v2 = 0;
        *(_WORD *)(v2 + 10042) = 0;
        *(_BYTE *)(this + result++ + 20084) = 0;
        v2 += 2;
    } while (result < 5021);  // stack 0-25105 zero
    *(_WORD *)(this + 25108) = 0;
    *(_WORD *)(this + 25106) = 0;
    *(_DWORD *)(this + 25384) = 0;
    *(_WORD *)(this + 25374) = 0;
    *(_WORD *)(this + 25372) = 0;
    *(_WORD *)(this + 25370) = 0;
    *(_WORD *)(this + 25368) = 0;
    *(_DWORD *)(this + 25660) = 0;
    *(_WORD *)(this + 25650) = 0;
    *(_WORD *)(this + 25648) = 0;
    *(_WORD *)(this + 25646) = 0;
    *(_WORD *)(this + 25644) = 0;
    return result;
}

__int16 My_Func_5B43FD0(long this)
{
    long v1;         // esi@1
    __int16 result;  // ax@1

    v1 = this;
    *(_DWORD *)(this + 25384) = 0;
    *(_WORD *)(this + 25374) = 0;
    *(_WORD *)(this + 25372) = 0;
    *(_WORD *)(this + 25370) = 0;
    *(_WORD *)(this + 25368) = 0;
    result = *(_WORD *)(this + 25648);
    if (result)
        result = Block2_WriteblockToDstbuff(this, this + 25388, *(_WORD *)(this + 25648));
    *(_WORD *)(v1 + 25648) = 0;
    return result;
}

// BYTE *this, 栈地址；long *a2, 加密缓冲区从加密数据开始；_DWORD
// a3加密缓冲区长度, long *a4返回地址
//
//注意--有过6k的解压后60+k的，因此szRet要dwEncodeBuffLen 10倍以上
DWORD InvDecryptBlock2(uint8 *pthis, long *pEncodeBuff, _DWORD dwEncodeBuffLen, long *szRet)
{
    long v4;
    unsigned int v5;
    unsigned __int16 v6;
    unsigned int v7 = 0;
    unsigned int v8;
    unsigned __int16 v9;
    long v10;
    int v11;
    __int16 usBuildOne;
    __int16 v13;
    unsigned int v15;
    unsigned int v16;

    v4 = (long)pthis;
    *(_DWORD *)(pthis + 25664) = dwEncodeBuffLen;
    *(long *)(pthis + 25376) = (long)pEncodeBuff;
    *(long *)(pthis + 25652) = (long)szRet;
    Block2_ZeroVar((BYTE *)pthis);
LABEL_2:
    *(_WORD *)(v4 + 25106) = 259;
    *(_WORD *)(v4 + 25108) = 9;
    LOWORD(v5) = My_Func_5B44260(v4);
    v6 = v5;
    v16 = v5;
    if ((_WORD)v5 != 256) {
        v15 = v5;
        Block2DecodeBuff(v4, v5);
        //目的缓冲区result长度不应该大于最大发票长度
        while (*(_DWORD *)(v4 + 25660) < DEF_MAX_FPBUF_LEN) {
            LOWORD(v7) = My_Func_5B44260(v4);
            v8 = v7;
            // v7==0时，实际是错误发票数据了，但是为了上层兼容性此处我们放行
            if ((_WORD)v7 == 256) {
                break;
            }
            if ((_WORD)v7 == 257) {
                goto LABEL_2;
            }
            if ((_WORD)v7 == 258) {
                ++*(_WORD *)(v4 + 25108);
            } else {
                if ((unsigned __int16)v7 < *(_WORD *)(v4 + 25106)) {
                    v9 = My_Func_5B441F0((_WORD *)v4, 0, v7);
                } else {
                    *(_WORD *)v4 = v15;
                    v9 = My_Func_5B441F0((_WORD *)v4, 1, v6);
                }
                v10 = v4 + 2 * v9;
                LOWORD(v15) = *(_WORD *)(v4 + 2 * v9 - 2);
                if (v9) {
                    v11 = v9;
                    do {
                        usBuildOne = *(_WORD *)(v10 - 2);
                        v10 -= 2;
                        Block2DecodeBuff(v4, usBuildOne);
                        --v11;
                    } while (v11);
                }
                v13 = v16;
                v16 = v8;
                v6 = v8;
                *(_WORD *)(v4 + 2 * *(_WORD *)(v4 + 25106) + 10042) = v13;
                *(_BYTE *)((*(_WORD *)(v4 + 25106))++ + v4 + 20084) = v15;
            }
        }
    }
    My_Func_5B43FD0(v4);
    return *(_DWORD *)(v4 + 25660);
}
// Finish----------------------------------------发票第二段解密算法----------------------------------------

int InvTypeCode2Byte5(uint8 *szOut, char *szIn)
{
    signed int v2;
    int v3 = 0;
    uint8 v5[16];
    if (strlen((const char *)szIn) == 12) {
        *(_DWORD *)v5 = *(_DWORD *)szIn;
        v5[4] = *(_BYTE *)(szIn + 4);
        v2 = 5;
        do {
            v5[v2] = *(_BYTE *)(szIn + v2 + 2);
            ++v2;
        } while (v2 <= 10);
        *(_BYTE *)szOut = (*(_BYTE *)(szIn + 6) + 10 * (*(_BYTE *)(szIn + 5) + 24)) | 0xC0;
        v3 = atol((char *)v5);
        *(_DWORD *)(szOut + 1) = v3;
        return 5;
    } else {
        LOBYTE(v3) = Str2Byte((uint8 *)szOut, szIn, 10);
    }
    return v3;
}

void method_61(uint8 *byte_3, char *szInvTypeCode)
{
    int int_8 = 0;
    int int_9 = 5;
    int i, j = 0;
    for (i = int_8; i < int_9 + int_8; i++) {
        szInvTypeCode[j++] = (char)((byte_3[i] >> 4) + 48);
        szInvTypeCode[j++] = (char)(byte_3[i] % 16 + 48);
    }
}

void method_64(uint8 *pByteIn, char *szInvTypeCode)
{
    uint num = *(uint *)(pByteIn + 1);
    char bytes[24] = "";
    sprintf(bytes, "%d", num);
    int num2 = 10 - strlen(bytes);
    int i, j;
    for (i = 0; i < 5; i++) {
        szInvTypeCode[i] = ((i < num2) ? 48 : bytes[i - num2]);
    }
    num = (uint)(pByteIn[0] & 63);
    szInvTypeCode[5] = (char)(num / 10u + 48u);
    szInvTypeCode[6] = (char)(num % 10u + 48u);
    for (j = 5; j < 10; j++) {
        szInvTypeCode[j + 2] = ((j < num2) ? 48 : bytes[j - num2]);
    }
}

void GetInvTypeCode(int nInvNum, uint8 *pByte5InvTypeInBuff, char *szInvTypeCode)
{
    if (nInvNum == 0) {
        strcpy(szInvTypeCode, "00000000");
        return;
    } else if ((pByte5InvTypeInBuff[0] & 192) != 192) {
        method_61(pByte5InvTypeInBuff, szInvTypeCode);
    } else {
        method_64(pByte5InvTypeInBuff, szInvTypeCode);
    }
}

//-------------------------------------------------------------------------------------------------
int GetInvNumLoop(uint8 *cb, int nTypeCount, uint8 *szRetBuff)
{
    long v9;
    long v10;
    uint8 v14;
    int v45;
    __int16 v17;
    v10 = (long)szRetBuff;
    v45 = nTypeCount;
    v9 = (long)cb;
    do {
        if (*(_BYTE *)(v9 + 12) & 0x80)
            *(_DWORD *)v10 = *(_WORD *)(v9 + 6) + *(_DWORD *)(v9 + 2) +
                             (*(_BYTE *)(v9 + 14) << 16) - (*(_BYTE *)(v9 + 15) << 16) -
                             *(_WORD *)(v9 + 23);
        else
            *(_DWORD *)v10 = *(_WORD *)(v9 + 6) + *(_DWORD *)(v9 + 2) - *(_WORD *)(v9 + 23);
        *(_WORD *)(v10 + 4) = *(_WORD *)(v9 + 23);
        *(_WORD *)(v10 + 6) = *(_BYTE *)(v9 + 9) & 0xF;
        *(_WORD *)(v10 + 8) = (unsigned __int8)(*(_BYTE *)(v9 + 10) >> 3);
        *(_WORD *)(v10 + 10) =
            4 * (*(_BYTE *)(v9 + 10) & 7) + (unsigned __int8)(*(_BYTE *)(v9 + 11) >> 6);
        *(_WORD *)(v10 + 12) = (unsigned __int8)(*(_BYTE *)(v9 + 9) >> 4) + 16 * *(_BYTE *)(v9 + 8);
        v14 = *(_BYTE *)(v9 + 1);
        *(_BYTE *)(v10 + 19) = v14;
        // LABEL_40:
        *(_BYTE *)(v10 + 21) = 0;
        *(_BYTE *)(v10 + 22) = 0;
        if (*(_BYTE *)(v9 + 12) & 0x80) {
            *(_BYTE *)(v10 + 23) = *(_BYTE *)(v9 + 15);
            *(_BYTE *)(v10 + 24) = *(_BYTE *)(v9 + 14);
        } else {
            *(_BYTE *)(v10 + 23) = 0;
            *(_BYTE *)(v10 + 24) = 0;
        }
        v17 = *(_WORD *)(v9 + 6);
        v9 += 25;
        *(_WORD *)(v10 + 25) = v17;
        *(_BYTE *)(v10 + 27) = *(_BYTE *)(v9 - 25);
        v10 += 28;
        --v45;
    } while (v45);
    return v10 + 28 - (long)szRetBuff;
}

//--------------------------------------------------

signed int sub_41D9EE0(signed int a1, int a2)
{
    signed int result;
    if (a2 == 4 || a2 == 6 || a2 == 9 || a2 == 11) {
        result = 30;
    } else if (a2 == 2) {
        if (a1 % 4 || (!(a1 % 100) && a1 % 400))
            result = 28;
        else
            result = 29;
    } else {
        result = 31;
    }
    return result;
}

uint8 sub_41D5430(uint8 a1)
{
    uint8 result;
    if ((unsigned __int8)a1 <= 0x63u)
        result = (unsigned __int16)((unsigned __int8)a1 % 10) + 16 * ((unsigned __int8)a1 / 10);
    else
        result = 0;
    return result;
}

uint8 sub_476AE90(unsigned __int8 a1)
{
    uint8 result;  // al@3
    if (a1 < 0xBu || a1 > 0x13u) {
        if (a1 < 0x29u || (result = 4, a1 > 0x3Cu))
            result = 1;
    } else {
        result = a1 - 8;
    }
    return result;
}

//---------------------------开票加密USBIO第二块 算法开始-----------------------
signed int sub_10037260(long this)
{
    signed int result;  // eax@1
    result = -1;
    memset((void *)this, 0xFFu, 0x2738u);  // 0-10040 FF
    *(_WORD *)(this + 10040) = -1;
    *(_WORD *)(this + 25108) = 259;
    *(_WORD *)(this + 25110) = 9;
    return result;
}

int sub_10037370(_BYTE *this, int a2, int a3)
{
    int result;  // eax@1
    int v4;      // esi@2

    result = a2 ^ 16 * a3;
    if ((_WORD)a2 == 16 * (_WORD)a3)
        v4 = 1;
    else
        v4 = 5021 - result;
    while (*(signed __int16 *)(this + (signed __int16)result * 2) != -1 &&
           (*(signed __int16 *)(this + (signed __int16)result * 2 + 10042) != (_WORD)a2 ||
            *(_BYTE *)(this + (signed __int16)result + 20084) != (_WORD)a3)) {
        result -= v4;
        if ((signed __int16)result < 0)
            result += 5021;
    }
    return result;
}

__int16 sub_100372E0(long this, int a2)
{
    long v2;              // esi@1
    unsigned __int16 v3;  // dx@1
    long v4;              // eax@1

    v2 = this;
    *(_WORD *)(v2 + 25644) += *(_WORD *)(this + 25110) - 8;
    Block2DecodeBuff(this, *(_BYTE *)(this + 25646) | (unsigned __int16)((unsigned __int16)a2 >>
                                                                         *(_WORD *)(this + 25644)));
    v3 = *(_WORD *)(v2 + 25644);
    v4 = a2 << (16 - v3);
    *(_WORD *)(v2 + 25646) = v4;
    if (v3 < 8u) {
        *(_WORD *)(v2 + 25646) = BYTE1(v4);
    } else {
        LOWORD(v4) = Block2DecodeBuff(v2, SBYTE1(v4));
        *(_WORD *)(v2 + 25646) &= 0xFFu;
        *(_WORD *)(v2 + 25644) -= 8;
    }
    return v4;
}

// 0-10040 有填充 20000-30000有内容缓冲，缓冲区因此必须大于30K以上
int InvEncryptBlock2(BYTE *pthis, long *pDecodeBuff, _DWORD dwDecodeBuffLen, long *szRet)
{
    long v6;
    int v7;
    signed int v8;
    unsigned __int16 v9;
    __int16 v10;
    unsigned __int16 v11;

    //因为该解密过程会占用输入缓冲的前几个字节，因此需要保存下前16字节，返回时恢复
    uint8 pDecodeBuffB16[16];
    memcpy(pDecodeBuffB16, pDecodeBuff, 16);

    v6 = (long)pthis;
    *(_DWORD *)(pthis + 25664) = dwDecodeBuffLen;
    *(long *)(pthis + 25376) = (long)pDecodeBuff;
    *(long *)(pthis + 25652) = (long)szRet;
    Block2_ZeroVar(pthis);
    sub_10037260(v6);
    if (!My_Func_5B44110(v6, (_WORD *)pDecodeBuff))
        *pDecodeBuff = 256;
    while (My_Func_5B44110(v6, (_WORD *)&dwDecodeBuffLen)) {
        v7 = (unsigned __int16)sub_10037370((_BYTE *)v6, (int)*pDecodeBuff, dwDecodeBuffLen);
        LOWORD(v8) = *(_WORD *)(v6 + 2 * v7);
        if ((WORD)v8 == -1) {
            v9 = *(_WORD *)(v6 + 25108);
            if (v9 <= 0xFFFu) {
                *(_WORD *)(v6 + 2 * v7) = v9;
                v10 = (__int16)*pDecodeBuff;
                ++*(_WORD *)(v6 + 25108);
                *(_WORD *)(v6 + 2 * v7 + 10042) = v10;
                *(_BYTE *)(v7 + v6 + 20084) = dwDecodeBuffLen;
            }
            sub_100372E0(v6, (int)*pDecodeBuff);
            v11 = *(_WORD *)(v6 + 25108);
            *pDecodeBuff = dwDecodeBuffLen;
            if (v11 <= 0xFFFu) {
                if (v11 >> *(_WORD *)(v6 + 25110)) {
                    sub_100372E0(v6, 258);
                    ++*(_WORD *)(v6 + 25110);
                }
            } else {
                sub_100372E0(v6, 257);
                sub_10037260(v6);
            }
        } else {
            *pDecodeBuff = v8;
        }
    }
    sub_100372E0(v6, (int)*pDecodeBuff);
    sub_100372E0(v6, 256);
    if (*(_WORD *)(v6 + 25644))
        Block2DecodeBuff(v6, *(_WORD *)(v6 + 25646));
    My_Func_5B43FD0(v6);
    //恢复前16字节
    memcpy(pDecodeBuff, pDecodeBuffB16, 16);
    return *(_DWORD *)(v6 + 25660);
}
//---------------------------开票加密USBIO第二块 结束-----------------------
//---------------------------密文、校验码解密函数 开始-----------------------

int sub_10038990(uint8 *a1, int a2)
{
    int result;  // eax@1
    int i;       // ecx@1
    int v4;      // edx@2
    result = 0;
    for (i = 0; i < a2; result = v4 + 10 * result - 48) {
        v4 = *(_BYTE *)(i++ + a1);
    }
    return result;
}

int sub_10038750(unsigned __int8 a1)
{
    uint8 a0123456789ab_0[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxy -*";
    uint8 *v1;   // eax@1
    int result;  // eax@2

    v1 = (uint8 *)strchr((char *)a0123456789ab_0, a1);
    if (v1)
        result = v1 - a0123456789ab_0;
    else
        result = 255;
    return result;
}

int sub_10038B00(uint8 *pOut, uint8 *sTaxID, int nTaxLen)
{
    _BYTE *v3;           // esi@1
    signed int v4;       // ebp@1
    int result;          // eax@1
    int v6;              // edi@1
    signed int v7;       // ebx@2
    unsigned __int8 v8;  // al@3
    uint8 v9;            // dl@3

    v3 = pOut;
    v4 = 0;
    result = 0;
    memset(pOut, 0, (6 * nTaxLen + 7) / 8);
    v6 = 0;
    if (nTaxLen > 0) {
        v7 = 6;
        do {
            v8 = sub_10038750(*(_BYTE *)(v6 + sTaxID));
            v9 = v8 << v4;
            v4 += 6;
            v7 -= 6;
            *v3 += v9;
            if (v4 > 8) {
                v7 += 8;
                v4 -= 8;
                *++v3 += v8 >> v7;
            }
            result = nTaxLen;
            ++v6;
        } while (v6 < nTaxLen);
    }
    return result;
}

uint8 sub_10006230(unsigned __int8 a1)
{
    uint8 result;
    uint8 szStr[] = "0123456789+-*/<>";
    if (a1 >= 0x10u)
        result = 48;
    else
        result = *((_BYTE *)szStr + a1);
    return result;
}

int DecodeMW(uint8 *ab54, uint8 *a4)
{
    long a2 = (long)ab54;
    int k = 0;
    uint8 v4 = 0;
    int v6 = 0x6c;  //该值是个变量，可能需要溯源.密文长度??108个字节,货运144 机动车190
                    //二手车0，其他都为108=0x6c
    long v27 = a2;
    int v30 = (unsigned int)v6 >> 1;
    for (k = 0; k < (signed int)(v30 - 1); ++k) {
        v4 += *(_BYTE *)(k + a2);
    }
    uint8 v32 = *(_BYTE *)(v30 + a2 - 1) & 0xF;
    *(_BYTE *)(v30 + a2 - 1) = v32;
    uint8 bv4 = v4 & 0xF;
    uint8 uA = *(_BYTE *)(v30 + a2 - 1);
    *(_BYTE *)(v30 + a2 - 1) = uA | (16 * (v32 + bv4));
    void *v34 = a4;
    int v33 = 0;
    do {
        *(_BYTE *)v34 = sub_10006230(*(_BYTE *)(v33 + v27) >> 4);
        *((_BYTE *)v34 + 1) = sub_10006230(*(_BYTE *)(v33++ + v27) & 0xF);
        v34 = (uint8 *)v34 + 2;
    } while (v33 < v30);
    return v33 * 2;
}

// uint8 ab[] = {0x8B, 0xF4, 0x29, 0x4C, 0xD7, 0xBB, 0xF4, 0xDE, 0xB0,
//             0x7B, 0x69, 0x12, 0x12, 0x8F, 0xA3, 0x20, 0x70};
//解密后
// 80355481191277817995
signed __int64 DecodeJYM(long src, uint8 *dst)
{
    unsigned int v2;        // esi@1
    unsigned int v3;        // edi@1
    signed int v4;          // ebx@1
    signed __int64 v5;      // kr00_8@2
    signed int v6;          // ebx@3
    unsigned int v7;        // edi@3
    unsigned int v8;        // esi@3
    signed __int64 result;  // rax@4
    signed __int64 v10;     // kr10_8@4

    v2 = *(_DWORD *)src;
    v3 = 0;
    v4 = 19;

    do {
        *(BYTE *)(dst + v4--) = (unsigned __int64)((signed __int64)__PAIR__(v3, v2) % 10) + 48;
        v5 = (signed __int64)__PAIR__(v3, v2) / 10;
        v3 = (unsigned __int64)((signed __int64)__PAIR__(v3, v2) / 10) >> 32;
        v2 = v5;
    } while (v4 >= 10);
    v6 = 9;
    v7 = ((unsigned __int64)*(_DWORD *)(src + 4) + (unsigned __int64)0x100000000) >> 32;
    v8 = *(_DWORD *)(src + 4);
    do {
        *(BYTE *)(dst + v6) = (unsigned __int64)((signed __int64)__PAIR__(v7, v8) % 10) + 48;
        result = (signed __int64)__PAIR__(v7, v8) / 10;
        --v6;
        v10 = (signed __int64)__PAIR__(v7, v8) / 10;
        v7 = (unsigned __int64)((signed __int64)__PAIR__(v7, v8) / 10) >> 32;
        v8 = v10;
    } while (v6 >= 0);
    return result;
}

// szOutMW > 256, outJYM > 64
//开票时，bCondition永远为1, bIsMulitTaxRate 是否多税率发票为false
int DecodeMWandJYM(uint8 bCondition, bool bIsMulitTaxRate, bool bNCP_SG, uint8 *pInDeviceMWJYMBuff,
                   char *szCompressTaxID, char *szGFSH, char *szXFSH, char *szOutMW, char *szOutJYM)
{
    uint8 ab54ByteMWKey[128] = "";
    char szCompressID6[24] = "";
    strncpy(szCompressID6, szCompressTaxID, 6);
    memset(ab54ByteMWKey, 0, sizeof(ab54ByteMWKey));
    memcpy(ab54ByteMWKey, pInDeviceMWJYMBuff, 29);  // ab54ByteMWKey的前29字节
    if (Get25ByteXor(bIsMulitTaxRate, bNCP_SG, (uint8 *)(ab54ByteMWKey + 29), szCompressID6, szGFSH,
                     szXFSH) < 0)
        return -11;
    int v21 = 0;
    BYTE *v22 = 0;
    int nXorCount = 25;
    //通常发票是条件1，暂时未测试到条件是0的发票，无法测试
    if (1 == bCondition)
        nXorCount = 25;
    else {
        //货运发票，机动车发票是24，并且亦或3次左右，需要实盘测试
        //  nXorCount = 24;
        return -12;
    }
    do {
        v22 = v21++ + (BYTE *)ab54ByteMWKey;
        *(_BYTE *)(v22 + 29) ^= *v22;
    } while (v21 < nXorCount);

    int nChildRet = DecodeMW((uint8 *)ab54ByteMWKey, (uint8 *)szOutMW);
    DecodeJYM((long)(pInDeviceMWJYMBuff + 0x1e), (uint8 *)szOutJYM);
    if (strlen(szOutJYM) > 30 || strlen(szOutJYM) < 16)
        return -13;
    if (nChildRet < 96 || nChildRet > 128)
        return -14;

    return 0;
}

uint8 DeviceTimeRandomSeed(long a1, long a2, int a3)
{
    int v3;
    long v4;
    uint8 result;
    v3 = 0;

    if (a3 > 0) {
        v4 = a1;
        do {
            result = (*(_BYTE *)(v3 + a2) & 0xF) + 10 * (*(_BYTE *)(v3 + a2) >> 4);
            ++v3;
            *(_BYTE *)v4 = result;
            *(_BYTE *)(v4 + 1) = 0;
            v4 += 2;
        } while (v3 < a3);
    }
    return result;
}

int sub_10006290(unsigned __int8 a1)
{
    uint8 *v1;
    int result;
    uint8 v3[32];
    strcpy((char *)v3, "0123456789+-*/<>");
    v1 = (uint8 *)strchr((char *)v3, a1);
    if (v1)
        result = v1 - v3;
    else
        result = 0;
    return result;
}


signed int BuildInv0x40(const void *a2, uint8 *a3)
{
    unsigned __int16 a4 = 0x6c;  //密文长度 108个字节
    BYTE v4[256] = "";
    signed int result;  // eax@5
    signed int v18;     // esi@13
    const void *v19;    // edi@14
    uint8 v20;          // al@15
    uint8 v21;          // dl@15
    uint8 v24;          // [sp+10h] [bp+4h]@15
    signed int v25;     // [sp+18h] [bp+Ch]@13
    v18 = 0;
    v25 = (unsigned int)a4 >> 1;
    result = v25;
    if (v25 > 0) {
        v19 = a2;
        do {
            v24 = sub_10006290(*((_BYTE *)v19 + 1));
            v20 = sub_10006290(*(_BYTE *)v19);
            v19 = (uint8 *)v19 + 2;
            v21 = 16 * v20 + v24;
            result = v25;
            *((_BYTE *)v4 + v18++ + 110) = v21;
        } while (v18 < v25);
    }
    memcpy(a3, (uint8 *)v4 + 110, 0x1Du);
    return result;
}

signed __int64 JE2InvRaw(long a1, double a2, int a3)
{
    signed __int64 result;
    signed int v4;
    result = (signed __int64)((double)a3 * fabs(a2) + 0.5);  // a2 有可能为负
    v4 = 0;
    do {
        *(_BYTE *)(v4 + a1) = result;
        result >>= 8;
        ++v4;
    } while (v4 < 5);
    return result;
}

double Check999TaxRate(uint8 *slv, int fplx_aisino, int zyfpLx)
{
    int flag = strlen((char *)slv) && !strcmp((char *)slv, "0.05") &&
               fplx_aisino == FPLX_AISINO_ZYFP && zyfpLx != ZYFP_HYSY;
    double double_ = (!strlen((char *)slv) || flag || zyfpLx == ZYFP_JYZS || zyfpLx == ZYFP_CEZS)
                         ? 9.99
                         : atof((char *)slv);
    return double_;
}

int NeedSpecialVCode(int fplx_aisino, uint8 b999TaxRate)
{
    int nC1 = sub_476AE90(fplx_aisino) == 1;
    int nC2 = b999TaxRate && (nC1 || (fplx_aisino >= 0x29u && fplx_aisino <= 0x3Cu));
    return nC2;
}

//这个函数参数比较杂
uint16 MakeJESLSE_VCodeByte(uint8 bAisinoFPLX, uint8 zyfpLx, double fJE, double fSL, double fSE,
                            uint16 wPreStr0x100x11)
{
    uint8 v71;
    signed __int64 v75;
    int v76;
    signed int v77;
    double v215;
    double v217;
    double v220;
    uint16 wVCode = 0;
    v217 = fabs(fJE);
    v215 = fabs(fSL);
    v220 = fabs(fSE);

    char szSlv[32] = "";
    sprintf(szSlv, "%f", fSL);
    double dTaxRate = Check999TaxRate((uint8 *)szSlv, bAisinoFPLX, zyfpLx);
    sprintf(szSlv, "%0.4f", dTaxRate);
    uint8 b999TaxRate = 0;
    if (!strncmp(szSlv, "9.99", 4))  //用字符串比对更准确
        b999TaxRate = 1;
    uint8 nNeedSpecialVCode = NeedSpecialVCode(bAisinoFPLX, b999TaxRate);

    v71 = bAisinoFPLX;
    if (v71 <= 2u || v71 >= 0xBu) {
        if (v71 <= 0x13u || v71 >= 0x33u) {
            if (nNeedSpecialVCode) {
                memcpy(&wVCode, &wPreStr0x100x11, 2);
                goto MakeJESLSE_VCodeByte_Finish;
            }
            v75 = ((signed __int64)(v215 * 1000.0 + 0.1) * (signed __int64)(v217 * 100.0 + 0.1) +
                   500) /
                  1000;
            v77 = (unsigned __int64)(v75 - (signed __int64)(v220 * 100.0 + 0.1)) >> 32;
            v76 = v75 - (signed __int64)(v220 * 100.0 + 0.1);
            if ((v75 - (signed __int64)(v220 * 100.0 + 0.1)) <= 127 && v77 >= -1 &&
                (v77 > -1 || v76 >= 0xFFFFFF80)) {
                wVCode = (uint8)(v76 - 128);
                goto MakeJESLSE_VCodeByte_Finish;
            }
            // error
        }
    }
    wVCode = 0;
MakeJESLSE_VCodeByte_Finish:
    return wVCode;
}

//---------------------------密文、校验码解密函数 结束-----------------------
//---------------------------开票数据包结束处8位验证码函数 开始-----------------------
int sub_10033160(uint8 *this)
{
    uint8 data[612] = {
        0x0E, 0x04, 0x0D, 0x01, 0x02, 0x0F, 0x0B, 0x08, 0x03, 0x0A, 0x06, 0x0C, 0x05, 0x09, 0x00,
        0x07, 0x00, 0x0F, 0x07, 0x04, 0x0E, 0x02, 0x0D, 0x01, 0x0A, 0x06, 0x0C, 0x0B, 0x09, 0x05,
        0x03, 0x08, 0x04, 0x01, 0x0E, 0x08, 0x0D, 0x06, 0x02, 0x0B, 0x0F, 0x0C, 0x09, 0x07, 0x03,
        0x0A, 0x05, 0x00, 0x0F, 0x0C, 0x08, 0x02, 0x04, 0x09, 0x01, 0x07, 0x05, 0x0B, 0x03, 0x0E,
        0x0A, 0x00, 0x06, 0x0D, 0x0F, 0x01, 0x08, 0x0E, 0x06, 0x0B, 0x03, 0x04, 0x09, 0x07, 0x02,
        0x0D, 0x0C, 0x00, 0x05, 0x0A, 0x03, 0x0D, 0x04, 0x07, 0x0F, 0x02, 0x08, 0x0E, 0x0C, 0x00,
        0x01, 0x0A, 0x06, 0x09, 0x0B, 0x05, 0x00, 0x0E, 0x07, 0x0B, 0x0A, 0x04, 0x0D, 0x01, 0x05,
        0x08, 0x0C, 0x06, 0x09, 0x03, 0x02, 0x0F, 0x0D, 0x08, 0x0A, 0x01, 0x03, 0x0F, 0x04, 0x02,
        0x0B, 0x06, 0x07, 0x0C, 0x00, 0x05, 0x0E, 0x09, 0x0A, 0x00, 0x09, 0x0E, 0x06, 0x03, 0x0F,
        0x05, 0x01, 0x0D, 0x0C, 0x07, 0x0B, 0x04, 0x02, 0x08, 0x0D, 0x07, 0x00, 0x09, 0x03, 0x04,
        0x06, 0x0A, 0x02, 0x08, 0x05, 0x0E, 0x0C, 0x0B, 0x0F, 0x01, 0x0D, 0x06, 0x04, 0x09, 0x08,
        0x0F, 0x03, 0x00, 0x0B, 0x01, 0x02, 0x0C, 0x05, 0x0A, 0x0E, 0x07, 0x01, 0x0A, 0x0D, 0x00,
        0x06, 0x09, 0x08, 0x07, 0x04, 0x0F, 0x0E, 0x03, 0x0B, 0x05, 0x02, 0x0C, 0x07, 0x0D, 0x0E,
        0x03, 0x00, 0x06, 0x09, 0x0A, 0x01, 0x02, 0x08, 0x05, 0x0B, 0x0C, 0x04, 0x0F, 0x0D, 0x08,
        0x0B, 0x05, 0x06, 0x0F, 0x00, 0x03, 0x04, 0x07, 0x02, 0x0C, 0x01, 0x0A, 0x0E, 0x09, 0x0A,
        0x06, 0x09, 0x00, 0x0C, 0x0B, 0x07, 0x0D, 0x0F, 0x01, 0x03, 0x0E, 0x05, 0x02, 0x08, 0x04,
        0x03, 0x0F, 0x00, 0x06, 0x0A, 0x01, 0x0D, 0x08, 0x09, 0x04, 0x05, 0x0B, 0x0C, 0x07, 0x02,
        0x0E, 0x02, 0x0C, 0x04, 0x01, 0x07, 0x0A, 0x0B, 0x06, 0x08, 0x05, 0x03, 0x0F, 0x0D, 0x00,
        0x0E, 0x09, 0x0E, 0x0B, 0x02, 0x0C, 0x04, 0x07, 0x0D, 0x01, 0x05, 0x00, 0x0F, 0x0A, 0x03,
        0x09, 0x08, 0x06, 0x04, 0x02, 0x01, 0x0B, 0x0A, 0x0D, 0x07, 0x08, 0x0F, 0x09, 0x0C, 0x05,
        0x06, 0x03, 0x00, 0x0E, 0x0B, 0x08, 0x0C, 0x07, 0x01, 0x0E, 0x02, 0x0D, 0x06, 0x0F, 0x00,
        0x09, 0x0A, 0x04, 0x05, 0x03, 0x0C, 0x01, 0x0A, 0x0F, 0x09, 0x02, 0x06, 0x08, 0x00, 0x0D,
        0x03, 0x04, 0x0E, 0x07, 0x05, 0x0B, 0x0A, 0x0F, 0x04, 0x02, 0x07, 0x0C, 0x09, 0x05, 0x06,
        0x01, 0x0D, 0x0E, 0x00, 0x0B, 0x03, 0x08, 0x09, 0x0E, 0x0F, 0x05, 0x02, 0x08, 0x0C, 0x03,
        0x07, 0x00, 0x04, 0x0A, 0x01, 0x0D, 0x0B, 0x06, 0x04, 0x03, 0x02, 0x0C, 0x09, 0x05, 0x0F,
        0x0A, 0x0B, 0x0E, 0x01, 0x07, 0x06, 0x00, 0x08, 0x0D, 0x04, 0x0B, 0x02, 0x0E, 0x0F, 0x00,
        0x08, 0x0D, 0x03, 0x0C, 0x09, 0x07, 0x05, 0x0A, 0x06, 0x01, 0x0D, 0x00, 0x0B, 0x07, 0x04,
        0x09, 0x01, 0x0A, 0x0E, 0x03, 0x05, 0x0C, 0x02, 0x0F, 0x08, 0x06, 0x01, 0x04, 0x0B, 0x0D,
        0x0C, 0x03, 0x07, 0x0E, 0x0A, 0x0F, 0x06, 0x08, 0x00, 0x05, 0x09, 0x02, 0x06, 0x0B, 0x0D,
        0x08, 0x01, 0x04, 0x0A, 0x07, 0x09, 0x05, 0x00, 0x0F, 0x0E, 0x02, 0x03, 0x0C, 0x0D, 0x02,
        0x08, 0x04, 0x06, 0x0F, 0x0B, 0x01, 0x0A, 0x09, 0x03, 0x0E, 0x05, 0x00, 0x0C, 0x07, 0x01,
        0x0F, 0x0D, 0x08, 0x0A, 0x03, 0x07, 0x04, 0x0C, 0x05, 0x06, 0x0B, 0x00, 0x0E, 0x09, 0x02,
        0x07, 0x0B, 0x04, 0x01, 0x09, 0x0C, 0x0E, 0x02, 0x00, 0x06, 0x0A, 0x0D, 0x0F, 0x03, 0x05,
        0x08, 0x02, 0x01, 0x0E, 0x07, 0x04, 0x0A, 0x08, 0x0D, 0x0F, 0x0C, 0x09, 0x00, 0x03, 0x05,
        0x06, 0x0B, 0x10, 0x07, 0x14, 0x15, 0x1D, 0x0C, 0x1C, 0x11, 0x01, 0x0F, 0x17, 0x1A, 0x05,
        0x12, 0x1F, 0x0A, 0x02, 0x08, 0x18, 0x0E, 0x20, 0x1B, 0x03, 0x09, 0x13, 0x0D, 0x1E, 0x06,
        0x16, 0x0B, 0x04, 0x19, 0x00, 0x33, 0x03, 0x10, 0x3A, 0x63, 0xAE, 0xDE, 0x09, 0x8C, 0x38,
        0xB4, 0xF9, 0x71, 0xCD, 0x7B, 0x65, 0xD9, 0x6C, 0xA0, 0x27, 0xBA, 0xE8, 0x12, 0xA5, 0x7C,
        0x84, 0xC8, 0x4A, 0x61, 0x6E, 0x46, 0x65, 0x62, 0x4D, 0x61, 0x72, 0x41, 0x70, 0x72, 0x4D,
        0x61, 0x79, 0x4A, 0x75, 0x6E, 0x4A, 0x75, 0x6C, 0x41, 0x75, 0x67, 0x53, 0x65, 0x70, 0x4F,
        0x63, 0x74, 0x4E, 0x6F, 0x76, 0x44, 0x65, 0x63, 0x00, 0x00, 0x00, 0x00};
    signed int v1;
    int v2;
    signed int v3;
    signed int v4;
    int v5;
    signed int v6;
    int result;
    uint8 *v8;
    uint8 *v9;
    uint8 v10[32];

    _BYTE *byte_1004CB8C = data;
    _BYTE *byte_1004CD8C = data + 0x200;

    v9 = (uint8 *)this;
    v1 = 0;
    do {
        v2 = 0;
        while (byte_1004CD8C[v2] - 1 != v1) {
            if (++v2 >= 32)
                goto LABEL_7;
        }
        v10[v1] = v2;
    LABEL_7:
        ++v1;
    } while (v1 < 32);
    v3 = 0;
    v8 = v10;
    do {
        v4 = 0;
        do {
            v5 = 0;
            v6 = 0;
            do {
                if ((8 >> v6) &
                    *(&byte_1004CB8C[(v4 & 0x20) | (16 * (v4 & 1)) | ((v4 >> 1) & 0xF)] + v3))
                    v5 |= 1 << (31 - v8[v6]);
                ++v6;
            } while (v6 < 4);
            result = v4++ + v3;
            *(int *)(v9 + result * 4 + 128) = v5;
        } while (v4 < 64);
        v3 += 64;
        v8 += 4;
    } while (v3 < 512);
    return result;
}

signed long sub_10033040(uint8 *this)
{
    uint8 byte_1004CA94[64] = {0x3A, 0x32, 0x2A, 0x22, 0x1A, 0x12, 0x0A, 0x02, 0x3C, 0x34, 0x2C,
                               0x24, 0x1C, 0x14, 0x0C, 0x04, 0x3E, 0x36, 0x2E, 0x26, 0x1E, 0x16,
                               0x0E, 0x06, 0x40, 0x38, 0x30, 0x28, 0x20, 0x18, 0x10, 0x08, 0x39,
                               0x31, 0x29, 0x21, 0x19, 0x11, 0x09, 0x01, 0x3B, 0x33, 0x2B, 0x23,
                               0x1B, 0x13, 0x0B, 0x03, 0x3D, 0x35, 0x2D, 0x25, 0x1D, 0x15, 0x0D,
                               0x05, 0x3F, 0x37, 0x2F, 0x27, 0x1F, 0x17, 0x0F, 0x07};
    // dword_10051430 dword_10051450
    uint8 data[48] = {0x80, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00,
                      0x10, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00,
                      0x02, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00,
                      0x04, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00};
    uint8 *v1;
    signed int v2;
    signed int v3;
    signed int v4;
    signed long result;
    int v6;

    _BYTE *dword_10051430 = data;
    _BYTE *dword_10051450 = data + 0x20;
    v1 = this;
    memset((void *)(this + 2176), 0, 0x800u);
    v6 = 0;
    v2 = 272;
    do {
        v3 = 0;
        do {
            v4 = 0;
            do {
                result = byte_1004CA94[v4] - 1;
                if (result >> 2 == v6) {
                    result &= 3u;
                    if (v3 & dword_10051450[result * 4]) {
                        result = (v4 >> 3) + (long)v1 + 8 * (v2 + v3);
                        *(_BYTE *)result |= dword_10051430[4 * (v4 & 7)];
                    }
                }
                ++v4;
            } while (v4 < 64);
            ++v3;
        } while (v3 < 16);
        v2 += 16;
        ++v6;
    } while (v2 < 528);
    return result;
}


signed long sub_100330D0(uint8 *this)
{
    // dword_10051430 dword_10051450
    uint8 data[48] = {0x80, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00,
                      0x10, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00,
                      0x02, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00,
                      0x04, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00};
    uint8 byte_1004CAD4[64] = {0x28, 0x08, 0x30, 0x10, 0x38, 0x18, 0x40, 0x20, 0x27, 0x07, 0x2F,
                               0x0F, 0x37, 0x17, 0x3F, 0x1F, 0x26, 0x06, 0x2E, 0x0E, 0x36, 0x16,
                               0x3E, 0x1E, 0x25, 0x05, 0x2D, 0x0D, 0x35, 0x15, 0x3D, 0x1D, 0x24,
                               0x04, 0x2C, 0x0C, 0x34, 0x14, 0x3C, 0x1C, 0x23, 0x03, 0x2B, 0x0B,
                               0x33, 0x13, 0x3B, 0x1B, 0x22, 0x02, 0x2A, 0x0A, 0x32, 0x12, 0x3A,
                               0x1A, 0x21, 0x01, 0x29, 0x09, 0x31, 0x11, 0x39, 0x19};

    uint8 *v1;
    signed int v2;
    signed int v3;
    signed int v4;
    signed long result;
    int v6;
    _BYTE *dword_10051430 = data;
    _BYTE *dword_10051450 = data + 0x20;

    v1 = this;
    memset((void *)(this + 4224), 0, 0x800u);
    v6 = 0;
    v2 = 528;
    do {
        v3 = 0;
        do {
            v4 = 0;
            do {
                result = byte_1004CAD4[v4] - 1;
                if (result >> 2 == v6) {
                    result &= 3u;
                    if (v3 & dword_10051450[result * 4]) {
                        result = (v4 >> 3) + (long)v1 + 8 * (v2 + v3);
                        *(_BYTE *)result |= dword_10051430[4 * (v4 & 7)];
                    }
                }
                ++v4;
            } while (v4 < 64);
            ++v3;
        } while (v3 < 16);
        v2 += 16;
        ++v6;
    } while (v2 < 784);
    return result;
}

uint8 sub_10032BF0(uint8 *this, long a2)
{
    // 100cb14 100cb4c 100cb5c
    uint8 data[120] = {
        0x39, 0x31, 0x29, 0x21, 0x19, 0x11, 0x09, 0x01, 0x3A, 0x32, 0x2A, 0x22, 0x1A, 0x12, 0x0A,
        0x02, 0x3B, 0x33, 0x2B, 0x23, 0x1B, 0x13, 0x0B, 0x03, 0x3C, 0x34, 0x2C, 0x24, 0x3F, 0x37,
        0x2F, 0x27, 0x1F, 0x17, 0x0F, 0x07, 0x3E, 0x36, 0x2E, 0x26, 0x1E, 0x16, 0x0E, 0x06, 0x3D,
        0x35, 0x2D, 0x25, 0x1D, 0x15, 0x0D, 0x05, 0x1C, 0x14, 0x0C, 0x04, 0x01, 0x02, 0x04, 0x06,
        0x08, 0x0A, 0x0C, 0x0E, 0x0F, 0x11, 0x13, 0x15, 0x17, 0x19, 0x1B, 0x1C, 0x0E, 0x11, 0x0B,
        0x18, 0x01, 0x05, 0x03, 0x1C, 0x0F, 0x06, 0x15, 0x0A, 0x17, 0x13, 0x0C, 0x04, 0x1A, 0x08,
        0x10, 0x07, 0x1B, 0x14, 0x0D, 0x02, 0x29, 0x34, 0x1F, 0x25, 0x2F, 0x37, 0x1E, 0x28, 0x33,
        0x2D, 0x21, 0x30, 0x2C, 0x31, 0x27, 0x38, 0x22, 0x35, 0x2E, 0x2A, 0x32, 0x24, 0x1D, 0x20};
    // dword_10051430
    uint8 dword_10051430[32] = {0x80, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00,
                                0x00, 0x10, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x04, 0x00,
                                0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00};
    uint8 *v2;
    signed int v3;
    signed int v4;
    signed int v5;
    signed int v6;
    int v7;
    signed int v8;
    uint8 result;
    uint8 *v10;
    uint8 aBlock[112] = "";
    uint8 *v11 = aBlock;
    uint8 *v12 = aBlock + 55;
    uint8 *v13 = aBlock + 56;
    _BYTE *byte_1004CB14 = data;
    _BYTE *byte_1004CB4C = data + 0x38;
    _BYTE *byte_1004CB5C = data + 0x48;

    v2 = this;
    memset(this, 0, 0x1880u);
    sub_10033160((uint8 *)this);
    sub_10033040((uint8 *)this);
    sub_100330D0((uint8 *)this);
    v3 = 0;
    do {
        v11[v3] = (*(_BYTE *)(((byte_1004CB14[v3] - 1) >> 3) + a2) &
                   dword_10051430[4 * ((byte_1004CB14[v3] - 1) & 7)]) != 0;
        ++v3;
    } while (v3 < 56);
    v4 = 0;
    do {
        v5 = byte_1004CB4C[v4];
        v6 = 0;
        do {
            v7 = v5;
            if (v5 >= (v6 >= 28 ? 56 : 28))
                v7 = v5 - 28;
            v13[v6++] = v11[v7];
            ++v5;
        } while (v6 < 56);
        v8 = 0;
        do {
            result = byte_1004CB5C[v8];
            if (*(v12 + result)) {
                v10 = &v2[8 * v4] + v8 / 6;
                result = (*(_DWORD *)&dword_10051430[4 * (v8 % 6)] >> 2) | *v10;
                *v10 = result;
            }
            ++v8;
        } while (v8 < 48);
        ++v4;
    } while (v4 < 16);
    return result;
}

int sub_10032DA0(uint8 *this, _BYTE *a2, uint8 *a3)
{
    _BYTE *v3;
    signed int v4;
    unsigned int v5;
    int v6;
    uint8 *v7;
    uint8 *v8;
    uint8 v9;
    uint8 v10;
    uint8 v11;
    uint8 v12;
    int result;
    uint8 *v14;
    v3 = a2;
    *(_DWORD *)a3 = 0;
    v14 = this;
    v4 = 0;
    *(_DWORD *)(a3 + 4) = 0;
    while (1) {
        v5 = v4 + ((unsigned int)*v3 >> 4);
        v6 = v4 + (*v3 & 0xF);
        v4 += 32;
        v7 = &this[8 * v5 + 2176];
        v8 = &this[8 * v6 + 2305];
        *(_BYTE *)a3 |= *v7++ | this[8 * v6 + 2304];
        *(_BYTE *)(a3 + 1) |= *v7 | *(_BYTE *)v8;
        v9 = v7[1] | this[8 * v6 + 2306];
        ++v8;
        ++v7;
        *(_BYTE *)(a3 + 2) |= v9;
        v10 = (v7++)[1] | *(_BYTE *)(v8++ + 1);
        *(_BYTE *)(a3 + 3) |= v10;
        v11 = (v7++)[1] | *(_BYTE *)(v8++ + 1);
        *(_BYTE *)(a3 + 4) |= v11;
        v12 = (v7++)[1] | *(_BYTE *)(v8++ + 1);
        *(_BYTE *)(a3 + 5) |= v12;
        *(_BYTE *)(a3 + 6) |= v7[1] | *(_BYTE *)(v8 + 1);
        result = *(a3 + 7);
        ++v3;
        *(_BYTE *)(a3 + 7) |= v7[2] | *(_BYTE *)(v8 + 2);
        if (v4 >= 256)
            break;
        this = v14;
    }
    return result;
}

int sub_10032F20(uint8 *this, unsigned int a2, long a3)
{
    uint8 *v3;  // esi@1
    int result;
    v3 = (uint8 *)(this + 384);
    result =
        (*(_DWORD *)&this[4 * (((a2 >> 23) & 0x3F) ^ (*(_BYTE *)(a3 + 1) & 0x3F)) + 384]) |
        (*(_DWORD *)&v3[4 * (((a2 >> 19) & 0x3F) ^ (*(_BYTE *)(a3 + 2) & 0x3F)) + 256]) |
        (*(_DWORD *)&v3[4 * (((a2 >> 15) & 0x3F) ^ (*(_BYTE *)(a3 + 3) & 0x3F)) + 512]) |
        (*(_DWORD *)&v3[4 * (((a2 >> 11) & 0x3F) ^ (*(_BYTE *)(a3 + 4) & 0x3F)) + 768]) |
        (*(_DWORD *)&this[4 * (((a2 >> 7) & 0x3F) ^ (*(_BYTE *)(a3 + 5) & 0x3F)) + 1408]) |
        (*(_DWORD *)&this[4 * (((a2 >> 3) & 0x3F) ^ (*(_BYTE *)(a3 + 6) & 0x3F)) + 1664]) |
        (*(_DWORD *)&this[4 * ((*(_BYTE *)(a3 + 7) & 0x3F) ^ ((a2 >> 31) | (2 * (a2 & 0x1F)))) +
                          1920]) |
        (*(_DWORD
               *)&this[4 * ((*(_BYTE *)a3 & 0x3F) ^ (32 * (a2 & 1) | ((a2 >> 27) & 0x3F))) + 128]);
    return result;
}

unsigned int sub_10032E60(uint8 *this, _BYTE *a2, long a3)
{
    _BYTE *v3;            // ebx@1
    signed int v4;        // edi@1
    unsigned int v5;      // edx@3
    int v6;               // eax@3
    uint8 *v7;            // edx@3
    long v8;              // esi@3
    uint8 v9;             // cl@3
    uint8 v10;            // cl@3
    uint8 v11;            // cl@3
    uint8 v12;            // cl@3
    unsigned int result;  // eax@3
    uint8 *v14;           // [sp+10h] [bp-4h]@1

    v3 = a2;
    *(_DWORD *)a3 = 0;
    v14 = this;
    v4 = 0;
    *(_DWORD *)(a3 + 4) = 0;
    while (1) {
        _BYTE bT = (_BYTE)*v3;
        v5 = v4 + (bT >> 4);
        v6 = v4 + (bT & 0xF);
        v4 += 32;
        v7 = &this[8 * v5 + 4224];
        v8 = (long)&this[8 * v6 + 4353];
        *(_BYTE *)a3 |= *v7++ | this[8 * v6 + 4352];
        *(_BYTE *)(a3 + 1) |= *v7 | *(_BYTE *)v8;
        v9 = v7[1] | this[8 * v6 + 4354];
        ++v8;
        ++v7;
        *(_BYTE *)(a3 + 2) |= v9;
        v10 = (v7++)[1] | *(_BYTE *)(v8++ + 1);
        *(_BYTE *)(a3 + 3) |= v10;
        v11 = (v7++)[1] | *(_BYTE *)(v8++ + 1);
        *(_BYTE *)(a3 + 4) |= v11;
        v12 = (v7++)[1] | *(_BYTE *)(v8++ + 1);
        *(_BYTE *)(a3 + 5) |= v12;
        *(_BYTE *)(a3 + 6) |= v7[1] | *(_BYTE *)(v8 + 1);
        result = a3 + 7;
        ++v3;
        *(_BYTE *)(a3 + 7) |= v7[2] | *(_BYTE *)(v8 + 2);
        if (v4 >= 256)
            break;
        this = v14;
    }
    return result;
}

int sub_10032CE0(void *this, long a2, long a3)
{
    unsigned int v3;   // esi@1
    unsigned int v4;   // edi@1
    int v5;            // ebp@1
    signed int v6;     // eax@1
    uint8 v7;          // cl@2
    unsigned int v8;   // edx@2
    long v9;           // ebx@3
    unsigned int v10;  // eax@4
    long v11;          // ebx@4
    int v12;           // ebp@5
    signed int v13;    // eax@5
    uint8 v14;         // cl@6
    uint8 *v16;        // [sp+Ch] [bp-Ch]@1
    uint8 abBlock[8] = "";
    uint8 *v17 = abBlock;
    uint8 *v18 = abBlock + 3;
    uint8 *v19 = abBlock + 4;
    signed int v20;  // [sp+1Ch] [bp+4h]@3

    v16 = (uint8 *)this;
    v3 = 0;
    v4 = 0;
    sub_10032DA0((uint8 *)this, (_BYTE *)a2, (uint8 *)v17);
    v5 = 0;
    v6 = 0;
    while (v6 < 32) {
        v7 = 24 - v6;
        v8 = (unsigned __int8)v17[v5] << (24 - v6);
        v6 += 8;
        v3 |= v8;
        v4 |= (unsigned __int8)v19[v5++] << v7;
    }
    v9 = (long)v16;
    v20 = 8;
    do {
        v10 = sub_10032F20(v16, v4, v9);
        v11 = v9 + 8;
        v3 ^= v10;
        v4 ^= sub_10032F20(v16, v3, v11);
        v9 = v11 + 8;
        --v20;
    } while (v20);
    v12 = 0;
    v13 = 0;
    do {
        v14 = 24 - v13;
        v13 += 8;
        v17[v12++] = v4 >> v14;
        *(v18 + v12) = v3 >> v14;
    } while (v13 < 32);
    return sub_10032E60(v16, (uint8 *)v17, a3);
}

int Build8BitMakeInvVCode(long a1, long a2, _BYTE *a3, signed int a4)
{
    _BYTE *v4;
    signed int v5;
    signed int v6;
    uint8 v7;
    signed int i;
    uint8 v9;
    int *v10;
    _BYTE *v11;
    long v13;
    uint8 this[0x2048] = "";  //必须8k以上
    int nT = 8;
    memcpy(this, &nT, 4);
    strcpy((char *)this + 4, "aeroinfo");
    v13 = (long)(this + 4);
    sub_10032BF0((uint8 *)(this + 12), a2);
    v4 = a3;
    v5 = a4;
    do {
        if (v5 < 8) {
            for (i = 0; i < v5; ++i) {
                v9 = *v4++ ^ *((_BYTE *)v13 + i);
                *((_BYTE *)v13 + i) = v9;
            }
            *((_BYTE *)v13 + v5) ^= 0x80u;
        } else {
            v6 = 0;
            do {
                v7 = *v4++ ^ *((_BYTE *)v13 + v6);
                *((_BYTE *)v13 + v6++) = v7;
            } while (v6 < 8);
        }
        sub_10032CE0(this + 12, (long)v13, (long)v13);
        v5 -= 8;
    } while (v5 >= 0);
    v10 = (int *)(a2 + 12);
    *(_DWORD *)a1 = *(_DWORD *)v13;
    v11 = (_BYTE *)(a2 + 12);
    int k = 0;
    do {
        *v11 ^= *(v11 - 4);
        ++v11;
    } while (k++ < 4);  //这里暂且认为是4
                        // result = *v10;
    *(_DWORD *)(a1 + 4) = *v10;
    return 0;
}
//---------------------------开票数据包结束处8位验证码函数 结束-----------------------


// bufIn输入参数 , bufOut输出参数
// bufIn为9位税号数组指针，bufOut输出16位伪随机数数组缓冲区指针
// 3209019999992070 -> 2C5036C627D099E1004818674B753514
int Makehashcode_GenProKey(uint8 *bufOut, uint8 *bufIn)
{
    uint8 v2 = 0;
    int *v3;
    _BYTE *v4;
    DWORD *v5;
    signed int v6;
    DWORD *v7;
    DWORD *v10;
    signed int v11;
    uint8 v12;
    unsigned __int8 v13;
    int *v14;
    signed int v15;
    uint8 *v20;
    unsigned __int8 *v21;
    int *v23;
    int *v24;
    uint8 *v27;
    unsigned __int8 v51;

    uint8 abLocalVarMem[52] = {0x00, 0xF6, 0x26, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0xF6, 0x26,
                               0x00, 0x68, 0x6C, 0xCA, 0x75, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
                               0x07, 0x08, 0x09, 0x00, 0x00, 0x10, 0x69, 0x73, 0x67, 0x6E, 0x6F,
                               0x67, 0x6E, 0x65, 0x66, 0x75, 0x67, 0x69, 0x78, 0x6E, 0x69, 0x78,
                               0x6E, 0x61, 0x69, 0x74, 0x67, 0x6E, 0x61, 0x68};
    uint8 *abLocalVar = (uint8 *)abLocalVarMem + 0x34;  // 0x34=52
    uint8 *TaxID = (uint8 *)abLocalVar - 0x24;
    memcpy(TaxID, bufIn, 9);
    v3 = (int *)(TaxID + 4);
    v4 = (_BYTE *)(TaxID + 8);

    if (*v4 & 0x80)
        BYTE3(*v3) = *v4 & 0x7F;
    else
        BYTE3(*v3) = 0;
    v5 = (DWORD *)TaxID;
    v6 = 8;
    do {
        v2 += *(_BYTE *)v5;
        v5 = (DWORD *)((uint8 *)v5 + 1);
        --v6;
    } while (v6);
    v7 = (DWORD *)((uint8 *)abLocalVar - 0x2c);
    v51 = (unsigned __int8)v2 % 24;
    //这里还有块复杂的算法，先不考虑
    //这里算法太复杂，涉及较多位移和参数设定，直接写死参数，后期如果有问题再修改
    uint8 var2C[] = {0x00, 0x48, 0x18, 0x67, 0x4B, 0x3D, 0x2D, 0x73};
    memcpy(v7, var2C, 8);

    v10 = (DWORD *)((uint8 *)abLocalVar - 0x28);
    v20 = (uint8 *)abLocalVar - 0x34;
    v11 = 4;
    do {
        v12 = *(_BYTE *)v10 + *v20;
        v10 = (DWORD *)((uint8 *)v10 + 1);
        --v11;
        *v20 = v12;
    } while (v11);
    v13 = 0;
    v21 = (uint8 *)abLocalVar - 0x30;
    v23 = (int *)((uint8 *)abLocalVar - 0x28);
    v24 = (int *)((uint8 *)abLocalVar - 0x24);
    v27 = (uint8 *)abLocalVar - 0x18;
    do {
        if (v13)
            *((_BYTE *)v24 + *v21) ^= *((_BYTE *)v23 + *v21 + 3);
        else
            LOBYTE(*v24) = *(v27 + v51) ^ *v24;
        ++v13;
        *((_BYTE *)v24 + *v21) ^= *(v27 + (unsigned __int8)((unsigned __int8)v12 % 16) + *v21);
        *v21 = v13;
    } while (v13 < 8u);
    v14 = (int *)v23;
    v15 = 4;
    do {
        *(_BYTE *)v14 ^= *((_BYTE *)v14 - 4);
        v14 = (int *)((uint8 *)v14 + 1);
        --v15;
    } while (v15);
    *(_DWORD *)(bufOut) = *(_DWORD *)((uint8 *)abLocalVar - 0x24);
    *(_DWORD *)(bufOut + 4) = *(_DWORD *)((uint8 *)abLocalVar - 0x20);
    *(_DWORD *)(bufOut + 8) = *(_DWORD *)((uint8 *)abLocalVar - 0x2c);
    *(_DWORD *)(bufOut + 12) = *(_DWORD *)((uint8 *)abLocalVar - 0x28);
    return 0;
}


uint8 sub_10039420(char *a1, unsigned __int8 a2)
{
    unsigned __int8 v2;
    signed __int8 result;
    int v4;
    uint8 v5;
    unsigned __int8 v6;
    unsigned __int8 v7;
    bool v8;
    signed __int8 v9;
    signed __int8 v10;
    signed __int8 v11;
    unsigned __int8 v12;

    v2 = a2;
    result = -1;
    v11 = 0;
    v9 = -1;
    v10 = -1;
    if (a2 > 0x14u)
        return -1;
    if (a2 < 0x10u)
        return -16;
    v4 = 0;
    v5 = 0;
    v12 = 0;
    if (a2) {
        do {
            v6 = *(_BYTE *)(v12 + a1);
            if (v6 < 0x30u || v6 > 0x39u) {
                if (v6 == 79 || v6 == 73 || v6 == 83 || v6 == 90)
                    v11 = 1;
                v5 = v12 + 1;
                v4 = 0;
            } else if (6 == ++v4) {
                if ((unsigned __int8)v5 <= 2u) {
                    if (result == -1)
                        v9 = v5;
                } else if (result == -1) {
                    return -1;
                }
            } else if (v4 == 14) {
                v10 = v5 + 16;
            }
            v2 = a2;
            v7 = v12 + 1;
            v12 = v7;
            v8 = v7 < a2;
            result = v9;
        } while (v8);
    }
    if (v2 >= 0x13u) {
        if (v10 != -1)
            return v10;
        if (result != -1 && !v11 && !(result & 0xF)) {
            result -= 64;
            return result;
        }
        return -1;
    }
    return result;
}

int sub_10038700(unsigned __int8 a1)
{
    uint8 a0123456789abcd[] = "0123456789ABCDEFGHJKLMNPQRTUVWXY";
    uint8 *v1;   // eax@1
    int result;  // eax@2
    v1 = (uint8 *)strchr((char *)a0123456789abcd, a1);
    if (v1)
        result = v1 - a0123456789abcd;
    else
        result = 255;
    return result;
}

// sub_10012780
int TaxID212Bytes(char *szTaxID, uint8 *abOut)
{
    //根据税号长度分三种情况[0,16)、(15,19)、19、20
    unsigned __int8 v4;
    unsigned int v5;
    uint8 v6;
    signed __int16 result;
    uint8 v8;
    uint8 v9;
    unsigned int v10;
    int v11;
    unsigned int v12;
    unsigned __int64 v13;
    signed int v14;
    signed int v15;
    signed int v16;
    unsigned __int8 v17;
    uint8 v18;
    int v19;
    _BYTE *v20;
    int v21;
    unsigned __int8 v22;
    uint8 v23;
    uint8 v24;
    uint8 v25;
    uint8 v26;
    __int64_t v27;
    int v28;
    int v29;
    __uint64_t v30;
    __int64_t v31;
    unsigned int v32;
    signed int v33;
    signed int v34;
    signed int v35;
    unsigned __int8 v36;
    uint8 v37;
    int v38;
    _BYTE *v39;
    int v40;
    unsigned __int8 v41;
    uint8 v42;
    uint8 v43;
    int v44;
    unsigned int v45;
    signed int v46;
    unsigned int v47;
    _BYTE *v48;
    signed int v49;
    signed int v50;
    unsigned __int8 v51;
    uint8 v52;
    uint8 v53;
    unsigned __int8 v54;
    unsigned __int8 v55;
    int v56;
    int v57;
    uint8 v58;
    signed int abOuta;
    int abOutb;
    signed int abOutc;
    int abOutd;
    int abOutf;
    signed int abOute;
    unsigned __int8 v71;

    long v3 = (long)abOut;
    *(_DWORD *)abOut = 0;
    *(_DWORD *)(abOut + 4) = 0;
    *(_DWORD *)(abOut + 8) = 0;
    v5 = strlen(szTaxID) + 1;
    v4 = v5 - 1;
    v6 = v5 - 1;
    v71 = v5 - 1;
    if ((unsigned __int8)(v5 - 1) > 0x14u)
        return 560;
    if (v4 < 0x10u) {
        sub_10038B00((_BYTE *)abOut, (uint8 *)szTaxID, v71);
        v8 = *(_BYTE *)(abOut + 11) | 0xF0;
        *(_BYTE *)(abOut + 11) = v8;
        if (v6 == 15) {
            *(_BYTE *)(abOut + 11) = v8 | 4;
            result = 0;
        } else {
            *(_BYTE *)(abOut + 10) |= 16 * v6;
            result = 0;
        }
        return result;
    }
    if (v4 > 0xFu && v4 < 0x13u) {
        v9 = sub_10039420(szTaxID, v71);
        if (v9 == 0xff)  //-1
            return 560;
        v54 = v9 & 0xF;
        if (v9 & 0x10)
            return 0;
        v10 = 0;
        v11 = v54;
        v12 = 0;
        v56 = v54 + 6;
        if (v54 < v56) {
            do {
                v13 = 10 * __PAIR__(v12, v10) + *((_BYTE *)szTaxID + v11++) - 48;
                v12 = HIDWORD(v13);
                v10 = v13;
            } while (v11 < v56);
        }
        *(_BYTE *)abOut = v10;
        *(_BYTE *)(abOut + 1) = BYTE1(v10);
        *(_BYTE *)(abOut + 2) = __PAIR__(v12, v10) >> 16;
        v14 = 2;
        v15 = 4;
        abOuta = 0;
        if ((signed int)v54 > 0) {
            v16 = 2;
            do {
                v17 = sub_10038750(*((_BYTE *)szTaxID + abOuta));
                v18 = v17 << v15;
                v15 += 6;
                v16 -= 6;
                *(_BYTE *)(v14 + v3) += v18;
                if (v15 > 8) {
                    v16 += 8;
                    v15 -= 8;
                    *(_BYTE *)(++v14 + v3) += v17 >> v16;
                }
                ++abOuta;
            } while (abOuta < v54);
        }
        v19 = v54 + 6;
        abOutb = v54 + 6;
        if (v56 < v71) {
            v20 = (_BYTE *)(v3 + v14);
            v21 = 6 - v15;
            while (1) {
                v22 = sub_10038750(*((_BYTE *)szTaxID + v19));
                v23 = v22 << v15;
                v15 += 6;
                v21 -= 6;
                *v20 += v23;
                if (v15 > 8) {
                    v21 += 8;
                    v15 -= 8;
                    *++v20 += v22 >> v21;
                }
                if (++abOutb >= v71)
                    break;
                v19 = abOutb;
            }
        }
        v24 = (v54 << 6) | *(_BYTE *)(v3 + 11);
        *(_BYTE *)(v3 + 11) = v24;
        if (v71 == 18) {
            *(_BYTE *)(v3 + 11) = v24 + 32;
            return 0;
        }
        *(_BYTE *)(v3 + 11) = v24 | ((v71 >> 2) & 0xF);
        v25 = v71 << 6;
        goto LABEL_55;
    }
    v26 = sub_10039420(szTaxID, v71);
    if (v26 == 0xff)  //-1
        return 560;
    v55 = v26 & 0xF;
    if (!(v26 & 0x10)) {
        v44 = 0;
        v45 = 0;
        v46 = 0;
        do {
            abOutf = *((_BYTE *)szTaxID + v46) - 48;
            v47 = (10 * __PAIR__(v45, v44) + abOutf) >> 32;
            ++v46;
            v44 = 10 * v44 + abOutf;
            v45 = v47;
        } while (v46 < 6);
        *(_BYTE *)v3 = v44;
        *(_BYTE *)(v3 + 1) = BYTE1(v44);
        v48 = (_BYTE *)(v3 + 2);
        *(_BYTE *)(v3 + 2) = __PAIR__(v47, v44) >> 16;
        v49 = 4;
        abOute = 6;
        if ((signed int)v71 > 6) {
            v50 = 1;
            do {
                v51 = sub_10038700(*((_BYTE *)szTaxID + abOute));
                v52 = v51 << v49;
                v49 += 5;
                v50 -= 5;
                *v48 += v52;
                if (v49 > 8) {
                    v50 += 8;
                    v49 -= 8;
                    *++v48 += v51 >> v50;
                }
                ++abOute;
            } while (abOute < v71);
        }
        v53 = *(_BYTE *)(v3 + 11) | 0xC0;
        *(_BYTE *)(v3 + 11) = v53;
        if (v71 >= 0x14u)
            return 0;
        *(_BYTE *)(v3 + 11) = v53 | ((unsigned __int8)((v71 & 0x58) | 0x20) >> 3);
        v25 = 32 * v71;
    LABEL_55:
        *(_BYTE *)(v3 + 10) |= v25;
        return 0;
    }
    v27 = 0;
    v28 = v55;
    v29 = 0;
    v58 = 0;
    v57 = v55 + 14;
    if (v55 < v57) {
        do {
            v30 = (10 * __PAIR__(v29, v27) + *((_BYTE *)szTaxID + v28) - 48) >> 32;
            v31 = 10 * v27 + *((_BYTE *)szTaxID + v28++) - 48;
            v29 = v30;
            v27 = v31;
        } while (v28 < v57);
        v58 = v30;
    }
    *(_BYTE *)abOut = v27;
    *(_BYTE *)(abOut + 1) = BYTE1(v27);
    *(_BYTE *)(abOut + 2) = __PAIR__(v29, v27) >> 16;
    *(_BYTE *)(abOut + 3) = __PAIR__(v29, v27) >> 24;
    v32 = v29 >> 8;
    v33 = 0;
    *(_BYTE *)(abOut + 4) = v58;
    *(_BYTE *)(abOut + 5) = v32;
    v34 = 6;
    abOutc = 0;
    if ((signed int)v55 > 0) {
        v35 = 6;
        do {
            v36 = sub_10038750(*((_BYTE *)szTaxID + abOutc));
            v37 = v36 << v33;
            v33 += 6;
            v35 -= 6;
            *(_BYTE *)(v34 + v3) += v37;
            if (v33 > 8) {
                v35 += 8;
                v33 -= 8;
                *(_BYTE *)(++v34 + v3) += v36 >> v35;
            }
            ++abOutc;
        } while (abOutc < v55);
    }
    v38 = v55 + 14;
    abOutd = v55 + 14;
    if (v57 < v71) {
        v39 = (_BYTE *)(v3 + v34);
        v40 = 6 - v33;
        while (1) {
            v41 = sub_10038750(*((_BYTE *)szTaxID + v38));
            v42 = v41 << v33;
            v33 += 6;
            v40 -= 6;
            *v39 += v42;
            if (v33 > 8) {
                v40 += 8;
                v33 -= 8;
                *++v39 += v41 >> v40;
            }
            if (++abOutd >= v71)
                break;
            v38 = abOutd;
        }
    }
    v43 = (v71 << 6) | *(_BYTE *)(v3 + 10);
    *(_BYTE *)(v3 + 11) |= ((v71 >> 2) & 0xF) | (unsigned __int8)((v55 << 6) + 16);
    *(_BYTE *)(v3 + 10) = v43;
    return 0;
}

// bIsMulitTaxRate 是否多税率发票,bNCP_SG是否农产品收购;开票时未考虑的选项可暂时填入false
int Get25ByteXor(bool bIsMulitTaxRate, bool bNCP_SG, uint8 *ab25, char *szCompressID6,
                 char *GFTaxID, char *XFTaxID)
{
    *(BYTE *)(ab25 + 23) = 0x41;
    uint8 abMem[34] = "";
    uint8 szXFTaxID12[16] = "";
    if (TaxID212Bytes(XFTaxID, (uint8 *)szXFTaxID12)) {
        logout(INFO, "TAXLIB", "发票开具", "TaxID212Bytes(XFTaxID, (uint8 *)szXFTaxID12) != 0\r\n");
        return -1;
    }
    memcpy(abMem, szXFTaxID12, 12);
    uint8 szGFTaxID12[16] = "";
    if (TaxID212Bytes(GFTaxID, (uint8 *)szGFTaxID12)) {
        logout(INFO, "TAXLIB", "发票开具", "TaxID212Bytes(GFTaxID, (uint8 *)szGFTaxID12) != 0\r\n");
        return -1;
    }
    memcpy(abMem + 22, szGFTaxID12, 12);
    unsigned int v10 = sub_10038990((uint8 *)szCompressID6, 6);
    memcpy(ab25 + 3, abMem, 11);
    *(WORD *)ab25 = (WORD)v10;
    BYTE *bV = (BYTE *)(abMem + 11);
    *(ab25 + 2) = (*bV * 16) | ((v10 >> 16) & 0xF);
    int v14 = 0;
    *(DWORD *)(ab25 + 15) = *(DWORD *)(abMem + 25);
    *(DWORD *)(ab25 + 19) = *(DWORD *)(abMem + 29);
    BYTE uB = *(BYTE *)(abMem + 33);
    BYTE uC = *(BYTE *)(ab25 + 23);
    *(BYTE *)(ab25 + 24) = uB & 0x0f;
    LOBYTE(v14) = (*bV & 0xF0) | ((unsigned __int8)uB >> 4);
    *(BYTE *)(ab25 + 14) = v14;
    *(BYTE *)(ab25 + 23) = (uint8)uC | 8;
    if (bIsMulitTaxRate)
        *(BYTE *)(ab25 + 23) &= 0xBFu;
    if (bNCP_SG)
        *(BYTE *)(ab25 + 23) |= 4u;
    *(BYTE *)(ab25 + 23) |= 0x20;
    return 0;
}

unsigned int sub_100341E0(_DWORD *this, int a2, _DWORD a3, _DWORD a4, _DWORD a5, signed int a6)
{
    uint8 byte_10051568[] = {0x07, 0x0c, 0x11, 0x16, 0x05, 0x09, 0x0e, 0x14,
                             0x04, 0x0b, 0x10, 0x17, 0x06, 0x0a, 0x0f, 0x15};
    uint8 data1[] = {
        0x78, 0xA4, 0x6A, 0xD7, 0x56, 0xB7, 0xC7, 0xE8, 0xDB, 0x70, 0x20, 0x24, 0xEE, 0xCE, 0xBD,
        0xC1, 0xAF, 0x0F, 0x7C, 0xF5, 0x2A, 0xC6, 0x87, 0x47, 0x13, 0x46, 0x30, 0xA8, 0x01, 0x95,
        0x46, 0xFD, 0xD8, 0x98, 0x80, 0x69, 0xAF, 0xF7, 0x44, 0x8B, 0xB1, 0x5B, 0xFF, 0xFF, 0xBE,
        0xD7, 0x5C, 0x89, 0x22, 0x11, 0x90, 0x6B, 0x93, 0x71, 0x98, 0xFD, 0x8E, 0x43, 0x79, 0xA6,
        0x21, 0x08, 0xB4, 0x49, 0x62, 0x25, 0x1E, 0xF6, 0x40, 0xB3, 0x40, 0xC0, 0x51, 0x5A, 0x5E,
        0x26, 0xAA, 0xC7, 0xB6, 0xE9, 0x5D, 0x10, 0x2F, 0xD6, 0x53, 0x14, 0x44, 0x02, 0x81, 0xE6,
        0xA1, 0xD8, 0xC8, 0xFB, 0xD3, 0xE7, 0xE6, 0xCD, 0xE1, 0x21, 0xD6, 0x07, 0x37, 0xC3, 0x87,
        0x0D, 0xD5, 0xF4, 0xED, 0x14, 0x5A, 0x45, 0x05, 0xE9, 0xE3, 0xA9, 0xF8, 0xA3, 0xEF, 0xFC,
        0xD9, 0x02, 0x6F, 0x67, 0x8A, 0x4C, 0x2A, 0x8D, 0x42, 0x39, 0xFA, 0xFF, 0x81, 0xF6, 0x71,
        0x87, 0x22, 0x61, 0x9D, 0x6D, 0x0C, 0x38, 0xE5, 0xFD, 0x44, 0xEA, 0xBE, 0xA4, 0xA9, 0xCF,
        0xDE, 0x4B, 0x60, 0x4B, 0xBB, 0xF6, 0x70, 0xBC, 0xBF, 0xBE, 0xC6, 0x7E, 0x9B, 0x28, 0xFA,
        0x27, 0xA1, 0xEA, 0x85, 0x30, 0xEF, 0xD4, 0x05, 0x1D, 0x88, 0x04, 0x39, 0xD0, 0xD4, 0xD9,
        0xE5, 0x99, 0xDB, 0xE6, 0xF8, 0x7C, 0xA2, 0x1F, 0x65, 0x56, 0xAC, 0xC4, 0x44, 0x22, 0x29,
        0xF4, 0x97, 0xFF, 0x2A, 0x43, 0xA7, 0x23, 0x94, 0xAB, 0x39, 0xA0, 0x93, 0xFC, 0xC3, 0x59,
        0x5B, 0x65, 0x92, 0xCC, 0x0C, 0x8F, 0x7D, 0xF4, 0xEF, 0xFF, 0xD1, 0x5D, 0x84, 0x85, 0x4F,
        0x7E, 0xA8, 0x6F, 0xE0, 0xE6, 0x2C, 0xFE, 0x14, 0x43, 0x01, 0xA3, 0xA1, 0x11, 0x08, 0x4E,
        0x82, 0x7E, 0x53, 0xF7, 0x35, 0xF2, 0x3A, 0xBD, 0xBB, 0xD2, 0xD7, 0x2A, 0x91, 0xD3, 0x86,
        0xEB};
    _DWORD *dword_10051468 = (_DWORD *)data1;
    _DWORD v6;
    _DWORD v7;
    _DWORD v8;
    uint8 v9;
    v6 = a6 & 0xF;

    switch (a6 >> 4) {
        case 0:
            v7 = this[v6] + ((a4 & a3) | (a5 & (~a3)));
            goto LABEL_6;
        case 1:
            v7 = this[(5 * v6 + 1) & 0xF] + ((a5 & a3) | (a4 & (~a5)));
            goto LABEL_6;
        case 2:
            v8 = (a5 ^ a4 ^ a3) + this[(3 * v6 + 5) & 0xF] + a2;
            goto LABEL_7;
        case 3:
            v7 = this[7 * (_BYTE)v6 & 0xF] + (a4 ^ (a3 | ~a5));
        LABEL_6:
            v8 = v7 + a2;
        LABEL_7:
            a2 = v8;
            break;
        default:
            break;
    }
    v9 = *(&byte_10051568[4 * (a6 >> 4)] + (a6 & 3));
    return a3 + (((a2 + dword_10051468[a6]) << v9) |
                 ((unsigned int)(a2 + dword_10051468[a6]) >> (32 - v9)));
}

uint8 *sub_10034010(uint8 *this)
{
    _DWORD *v1;     // esi@1
    signed int v2;  // ecx@1
    int *v3;        // ebx@1
    _DWORD *v4;     // eax@1
    signed int v5;  // edi@3
    uint8 *result;  // eax@10
    signed int v7;  // ecx@10

    v1 = (_DWORD *)this;
    v2 = 4;
    v3 = (int *)(this + 0x50);
    v4 = (_DWORD *)v3;
    do {
        *v4 = *(v4 - 4);
        ++v4;
        --v2;
    } while (v2);
    v5 = 0;
    do {
        switch (v5 & 3) {
            case 0:
                *v3 = sub_100341E0(v1, *v3, v1[21], v1[22], v1[23], v5);
                break;
            case 1:
                v1[23] = sub_100341E0(v1, v1[23], *v3, v1[21], v1[22], v5);
                break;
            case 2:
                v1[22] = sub_100341E0(v1, v1[22], v1[23], *v3, v1[21], v5);
                break;
            case 3:
                v1[21] = sub_100341E0(v1, v1[21], v1[22], v1[23], *v3, v5);
                break;
            default:
                break;
        }
        ++v5;
    } while (v5 < 64);
    result = (uint8 *)(v1 + 16);
    v7 = 4;
    do {
        *(_DWORD *)result += *((_DWORD *)result + 4);
        result += 4;
        --v7;
    } while (v7);
    return result;
}

//这个函数调用有前置内存条件，因此不可以随便调用
signed int sub_10034160(long this)
{
    long v2;          // esi@1
    int v3;           // eax@1
    unsigned int v6;  // eax@7
    v2 = this;
    v3 = 25;  // 25==0x19 25个字节xor，后面解密密文也有这个长度
    *(_BYTE *)(v3 + this) = 0x80u;
    memset((uint8 *)this + 0x1b, 0, 29);
    v6 = (unsigned int)v3;
    *(_DWORD *)(v2 + 56) = 8 * v6;
    *(_DWORD *)(v2 + 60) = v6 >> 29;
    sub_10034010((void *)v2);
    return 0;
}

int JE_SLV_Flag(bool bool_12, bool bool_13)
{
    bool bool_8 = true;
    int num = 80;
    if (bool_12) {
        num += (bool_8 ? -64 : 32);
    }
    if (bool_13) {
        num += 16;
    }
    return num;
}

uint8 SetPreInvoiceBuff0(float je)
{
    int nFlag = JE_SLV_Flag(false, je < 0) >> 4;
    return nFlag != 5 ? 31 : 15;
}

void CalcDZSYH_method_17(uint8 *byte_0, int nByte0Len, uint8 *pOut)
{
    int i = 0;
    for (i = 0; i < nByte0Len; i++) pOut[i] = byte_0[nByte0Len - i - 1];
}

int CalcDZSYH_method13(uint8 *szServerRep16Byte, int nByte0Len, int nDZSYH, uint8 *pOut)
{
    //官方的比较繁琐,可能涉及多票联报的,我们只考虑一次一张情况
    if (nByte0Len > 64)
        return -1;
    memcpy(pOut, szServerRep16Byte, nByte0Len);
    //分号分割的多票联报的情况暂时不考虑，每次只报一张发票，因此这里就填1
    pOut[nByte0Len] = 0x1;
    int nLen = nByte0Len + 1 + 4;
    uint8 *pEncDZSYHIndex = pOut + nByte0Len + 1;
    CalcDZSYH_method_17((uint8 *)&nDZSYH, 4, pEncDZSYHIndex);
    return nLen;
}

int sub_8B91C20(int a1, int a2)
{
    int v2;
    int result;
    v2 = a1;
    if (!(_WORD)a1) {
        v2 = a2;
        return 1 - v2;
    }
    if (!(_WORD)a2)
        return 1 - v2;
    uint32 nVar = (uint16)a2 * (uint16)a1;
    uint32 nVar2 = nVar >> 16;
    result = (uint16)nVar - (uint16)nVar2;
    if ((uint16)nVar < (uint16)nVar2)
        result += 1;
    return result;
}

int sub_8A5EAF0(long a3, long pDst, _WORD *pPreBuf)
{
    int a1;
    int a2;
    __int16 v5;
    _WORD *v6;
    int v7;
    long v8;
    int v9;
    int v10;
    int v11;
    int v12;
    int v13;
    int v14;
    unsigned __int16 v15;
    int v16;
    int v17;
    int v18;
    int v19;
    int v20;
    long v21;
    int v22;
    int v23;
    __int16 v24;
    __int16 v25;
    int result;
    long v27;
    __int16 v28;
    signed int v29;
    int v30;
    int v31;
    int v32;
    int v33;

    v5 = *(_WORD *)a3;
    LOWORD(a1) = *(_WORD *)(a3 + 2);
    v6 = pPreBuf;
    v7 = a3 + 4;
    v29 = 8;
    LOWORD(a2) = *(_WORD *)(a3 + 4);
    v28 = *(_WORD *)(a3 + 6);
    while (1) {
        LOWORD(v7) = *v6;
        v8 = (long)v6 + 2;
        v9 = sub_8B91C20(v5, v7);
        LOWORD(v10) = *(_WORD *)v8;
        LOWORD(v11) = *(_WORD *)(v8 + 2);
        v30 = v9;
        v8 += 4;
        v12 = v10 + a1;
        v13 = v11 + a2;
        LOWORD(v9) = *(_WORD *)v8;
        v8 += 2;
        v14 = sub_8B91C20(v28, v9);
        v15 = v14;
        LOWORD(v14) = *(_WORD *)v8;
        v33 = v13;
        v8 += 2;
        v16 = sub_8B91C20((unsigned __int16)v30 ^ (unsigned __int16)v13, v14);
        v17 = v16;
        LOWORD(v16) = *(_WORD *)v8;
        v6 = (_WORD *)(v8 + 2);
        v18 = sub_8B91C20((_WORD)v17 + ((unsigned __int16)v12 ^ v15), v16);
        v19 = v18 + v17;
        v31 = v18 ^ v30;
        v20 = v12;
        v28 = v19 ^ v15;
        a1 = v33 ^ v18;
        a2 = v20 ^ v19;
        v7 = v29-- - 1;
        if (!v29)
            break;
        v5 = v31;
    }
    LOWORD(v7) = *v6;
    v21 = (long)(v6 + 1);
    v22 = sub_8B91C20((unsigned __int16)v31, v7);
    LOWORD(v23) = *(_WORD *)(v21 + 4);
    v32 = v22;
    v24 = *(_WORD *)v21 + a2;
    v25 = *(_WORD *)(v21 + 2) + a1;
    result = sub_8B91C20(v28, v23);
    *(_WORD *)pDst = v32;
    *(_WORD *)(pDst + 2) = v24;
    v27 = pDst + 4;
    *(_WORD *)v27 = v25;
    *(_WORD *)(v27 + 2) = result;
    return result;
}

int sub_8A5EA90(uint8 *this, __int16 *a2)
{
    __int16 *v2;
    uint8 *v3;
    signed int v4;
    __int16 v5;
    int result;
    signed int v7;
    int v8;

    v2 = a2;
    v3 = this;
    v4 = 8;
    do {
        v5 = *v2;
        ++v2;
        *(_WORD *)v3 = v5;
        v3 += 2;
        --v4;
    } while (v4);
    result = 1;
    v7 = 44;
    do {
        v8 = result & 7;
        *(_WORD *)&this[2 * result + 14] =
            (*(_WORD *)&this[2 * v8] << 9) | (*(_WORD *)&this[2 * (((_BYTE)result + 1) & 7)] >> 7);
        --v7;
        this += 2 * (result & 8);
        result = v8 + 1;
    } while (v7);
    return result;
}

int BuildPubServiceRandom(char *szFPDM, char *szFPHM, char *szKPTimeStandard, char *szOut8ByteStr)
{
    // szKPStandard's format='19700101080808'
    if (14 != strlen(szKPTimeStandard) || (strlen(szFPDM) < 8 || strlen(szFPDM) > 12) ||
        (strlen(szFPHM) > 12 || strlen(szFPHM) < 8))
        return -1;
    unsigned char abStaticVar[16] = {0x1A, 0x68, 0x18, 0x66, 0x09, 0x6E, 0x51, 0x24,
                                     0x74, 0x9D, 0x5C, 0x4E, 0xA6, 0x73, 0x3A, 0xC1};
    uint8 abBuf[64] = {0}, abPreBuf[128] = {0}, abDst[64] = {0};
    uint8 *pMd5Dst = abBuf + 16, *pMd5XorDst = abBuf + 32;
    uint64 u64FPHMSwap = 0;
    char szKPDate[16] = "";
    int i = 0;

    strncpy(szKPDate, szKPTimeStandard, 8);
    InvTypeCode2Byte5(abBuf, szFPDM);
    u64FPHMSwap = atoll(szFPHM);
    memcpy(abBuf + 5, (uint8 *)&u64FPHMSwap, 4);
    if (!Str2Byte(abBuf + 9, szKPDate, 8))
        return -2;
    CalcMD5(abBuf, 0xd, pMd5Dst);
    for (i = 0; i < 16; i++) {
        pMd5XorDst[i] = pMd5Dst[i] ^ abStaticVar[i];
    }
    sub_8A5EA90(abPreBuf, (__int16 *)pMd5XorDst);
    sub_8A5EAF0((long)pMd5Dst, (long)abDst, (_WORD *)abPreBuf);
    Byte2Str(szOut8ByteStr, abDst, 4);
    mystrupr(szOut8ByteStr);
    return 0;
}

// pInputBuf's len must >=8;输出结果直接修改原buf
int AisinoDesEcbEncrypt(uint8 *pInputBuf)
{
    // https://www.ssleye.com/des_cipher.html
    // aes_64_ecb zeropadding key=password
    int nInputBufLen = 8;  //固定8字节
    EVP_CIPHER_CTX *ctx = NULL;
    char szKey[8] = "password";
    int len, nCopyLen = 8, nRet = -1;
    uint8 abInputBuf[100] = {0};  //原先只有8字节，太小会导致段错误

    // ZeroPadding
    memset(abInputBuf, 0, sizeof abInputBuf);
    if (nInputBufLen < nCopyLen)
        nCopyLen = nInputBufLen;
    memcpy(abInputBuf, pInputBuf, nCopyLen);

    if (!(ctx = EVP_CIPHER_CTX_new()))
        return -1;
    while (1) {
        if (!EVP_EncryptInit_ex(ctx, EVP_des_ecb(), NULL, (const unsigned char *)szKey, NULL))
            break;
        if (!EVP_EncryptUpdate(ctx, abInputBuf, &len, abInputBuf, 8))
            break;
        if (!EVP_EncryptFinal_ex(ctx, abInputBuf + len, &len))
            break;
        nRet = 0;
        break;
    }
    memcpy(pInputBuf, abInputBuf, 8);
    EVP_CIPHER_CTX_free(ctx);
    return nRet;
}

uint8 sub_8B1F970(uint8 a1)
{
    uint8 result;
    if (a1 < 11u || a1 > 19u) {
        if (a1 < 41u || (result = 4, a1 > 60u))
            result = 1;
    } else
        result = a1 - 8;
    return result;
}

//进销存buf还原算法
int RestoreAisinoMonthStatisticApiBuf(uint8 *pInBuf, uint8 *pOutMem, int nOutMemSize)
{
    int result;
    long v5;
    char *v6;
    signed int v7;
    _BYTE *v9;
    const void *v10;
    int v22;
    long v25;
    long v26;
    long v27;
    char *v12;
    long v13;
    signed int v14;
    char *v15;
    signed int v29;
    uint8 v31[55];
    uint8 *v17;

    bool v21;
    char *v23;
    signed int v24;
    signed __int64 v30;
    int v16 = 0;

    char a_2f[] = "%.2f";
    //实际使用大小为 121*8+41=1009
    if (nOutMemSize < 1280)
        return -1;
    v5 = (long)pOutMem + 41;
    v25 = (long)pInBuf;
    v22 = 8;
    do {
        *(_BYTE *)(v5 - 41) = -1;
        memset((void *)(v5 - 40), 0, 0x28u);
        v6 = (char *)v5;
        v7 = 4;
        do {
            sprintf(v6, a_2f, 0);
            v6 += 20;
            --v7;
        } while (v7);
        v5 += 121;
        --v22;
    } while (v22);
    v29 = 8;
    v9 = (uint8 *)pOutMem;
    v10 = (const void *)(v25 + 8);
    v27 = (long)pOutMem;
    v26 = v25 + 8;
    do {
        memcpy(v31, v10, 55);
        if (*v31 != 0xff) {
            *v9 = *v31;
            v12 = (char *)v31 + 1;
            v13 = (long)(v9 + 1);
            v14 = 10;
            do {
                v15 = v12;
                v12 += 3;
                v13 += 4;
                LOWORD(v16) = *(_WORD *)v15;
                --v14;
                HIWORD(v16) = (uint8)v15[2];
                *(_DWORD *)(v13 - 4) = v16;
            } while (v14);
            v17 = (uint8 *)v31 + 31;
            v23 = (char *)v9 + 41;
            v24 = 4;
            do {
                HIDWORD(v30) = 0;

                //蒙柏方修改--之前计算在A33有问题，修改方行，可能与浮点精度有关
                long long int t1 = (long long int)(v17[5]) * 0x10000000000;
                long long int t2 = (long long int)(v17[4]) * 0x100000000;
                long long int t3 = (long long int)(v17[3]) * 0x1000000;
                long long int t4 = (long long int)(v17[2]) * 0x10000;
                long long int t5 = (long long int)(v17[1]) * 0x100;
                long long int t6 = (long long int)(v17[0]) * 0x1;

                v30 = t1 + t2 + t3 + t4 + t5 + t6;
                v17 += 6;
                memset(v23, 0, 20);  // 4*20字节存储正数金额、负数金额、正数税额、负数税额
                sprintf(v23, a_2f, (long double)v30 * 0.01);
                v21 = v24 == 1;
                v23 += 20;
                --v24;
            } while (!v21);
            v9 = (_BYTE *)v27;
        }
        v10 = (const void *)(v26 + 55);
        v9 += 121;
        result = v29 - 1;
        v21 = v29 == 1;
        v26 += 55;
        v27 = (long)v9;
        --v29;
    } while (!v21);
    if (result == 0)
        result = (int)((long)v5 - (long)pOutMem);
    else
        result = -10;
    return result;
}

void BuildRedTzdbh6Bytes(uint8 *pOut6BytesBuf, char *sz16BytesTzdbh)
{
    int v2;
    signed int i;
    unsigned int v5;
    int v6;
    v6 = 0;
    v5 = 0;

    for (i = 0; i < 6; ++i) {
        v6 = 10 * v6 + *(_BYTE *)(i + sz16BytesTzdbh) - 48;
        v5 = 10 * v5 + *(_BYTE *)(i + sz16BytesTzdbh + 10) - 48;
    }
    v2 = 10 * (*(_BYTE *)(sz16BytesTzdbh + 8) - 48) +
         12 * (10 * (*(_BYTE *)(sz16BytesTzdbh + 6) - 48) + *(_BYTE *)(sz16BytesTzdbh + 7) - 55) +
         *(_BYTE *)(sz16BytesTzdbh + 9) - 49;
    *(_WORD *)pOut6BytesBuf = v6;
    *(_WORD *)(pOut6BytesBuf + 2) = ((v5 & 0xF) << 12) + 16 * v2 + HIWORD(v6);
    *(_WORD *)(pOut6BytesBuf + 4) = v5 >> 4;
}

int GetFpdmAndSpecialFlagFromBlock2(char *a1, char *a2)
{
    _BYTE *v4;
    char v5;
    int a3 = 2;
    strcpy(a2, "");

    v4 = (_BYTE *)a1;
    v5 = 1;
    while (1) {
        if ((unsigned __int8)v5 == a3)
            break;
        while (*v4 != 10) ++v4;
        ++v5;
        ++v4;
    }
    while (*v4 != 10) {
        *a2 = *v4;
        a2++;
        ++v4;
    }
    return strlen(a2);
}