/*****************************************************************************
File name:   cntax_algorithm.c
Description: 用于cntax税盘的所有算法
Author:      Zako
Version:     1.0
Date:        2019.09
History:
20200718     分离模块,将与税局的交互和具体业务功能全部归类于此
20210808     源码过大，分流非业务xml操作和xml实现过程至其他函数，只保留流程导航
*****************************************************************************/
#include "cntax_algorithm.h"

//返回0则正确，<0错误
int BuildMWFinal(char *szInOutMWBuf)
{
    char szBuf[256] = "";
    szBuf[0] = 0x00;
    strcpy(szBuf + 1, szInOutMWBuf);
    char *a6_4 = szBuf + 1;
    uint8 a6[32] = {0};
    *(long *)(a6 + 8) = (long)a6_4;
    *(long *)(a6 + 16) = 0x70;
    uint32 unk_8186428 = 0;
    long v21 = *(long *)(a6 + 16);
    char szDic[] = "0123456789*+<->/";
    int v22 = 0;
    long v23;
    uint8 v24;
    long v25;
    _BYTE *v26;
    long v27;
    long v28;
    uint8 v29;
    long v30;
    _BYTE *v31;
    long v32;
    long v33;
    uint8 v34;
    long v35;
    _BYTE *v36;
    long v37;
    long v38;
    uint8 v39;
    long v40;
    _BYTE *v41;
    long v49;
    long v50;
    uint8 v51;
    long v52;
    _BYTE *v53;
    _BYTE *v54;
    _BYTE *v55;
    long v56;
    long v57;
    uint8 v58;
    long v59;
    long v42;
    long v43;
    uint8 v44;
    long v45;
    _BYTE *v46;
    char v47;
    char *v48;

    do {
        v23 = *(long *)(a6 + 8);
        if (v23) {
            v24 = *(_BYTE *)(v23 - 1);
            if (v24 && v24 != 0xff)
                return -1;
            v25 = *(long *)(a6 + 8);
            if (v25)
                *(_BYTE *)(v25 - 1) = 0xff;
            v26 = (_BYTE *)(v22 + *(long *)(a6 + 8));
        } else {
            v26 = (uint8 *)&unk_8186428;
        }
        if (*v26 >= 48) {
            v27 = *(long *)(a6 + 16);
            if (v27 >= v22 && (v28 = *(long *)(a6 + 8)) != 0) {
                v29 = *(_BYTE *)(v28 - 1);
                if (v29 && v29 != 0xff)
                    return -2;
                v30 = *(long *)(a6 + 8);
                if (v30)
                    *(_BYTE *)(v30 - 1) = 0xff;
                v31 = (_BYTE *)(v22 + *(long *)(a6 + 8));
            } else {
                v31 = (uint8 *)&unk_8186428;
            }
            if (*v31 <= 57)
                goto LABEL_95;
        }
        v32 = *(long *)(a6 + 16);
        if (v32 >= v22 && (v33 = *(long *)(a6 + 8)) != 0) {
            v34 = *(_BYTE *)(v33 - 1);
            if (v34 && v34 != 0xff)
                return -3;
            v35 = *(long *)(a6 + 8);
            if (v35) {
                *(_BYTE *)(v35 - 1) = 0xff;
            }
            v36 = (_BYTE *)(v22 + *(long *)(a6 + 8));
        } else {
            v36 = (uint8 *)&unk_8186428;
        }
        if (*v36 < 97)
            goto LABEL_104;
        v37 = *(long *)(a6 + 16);
        if (v37 >= v22 && (v38 = *(long *)(a6 + 8)) != 0) {
            v39 = *(_BYTE *)(v38 - 1);
            if (v39 && v39 != 0xff)
                return -4;
            v40 = *(long *)(a6 + 8);
            if (v40)
                *(_BYTE *)(v40 - 1) = 0xff;
            v41 = (_BYTE *)(v22 + *(long *)(a6 + 8));
        } else {
            v41 = (uint8 *)&unk_8186428;
        }
        if (*v41 > 122) {  //全部大写字符进入该循环
        LABEL_104:
            v49 = *(long *)(a6 + 16);
            if (v49 >= v22 && (v50 = *(long *)(a6 + 8)) != 0) {
                v51 = *(_BYTE *)(v50 - 1);
                if (v51 && v51 != 0xff)
                    return -5;
                v52 = *(long *)(a6 + 8);
                if (v52)
                    *(_BYTE *)(v52 - 1) = 0xff;
                v53 = (_BYTE *)(v22 + *(long *)(a6 + 8));
            } else {
                v53 = (uint8 *)&unk_8186428;
            }
            if (*v53 < 65)
                goto LABEL_95;
            if (*(long *)(a6 + 16) >= v22 && *(long *)(a6 + 8)) {
                v54 = (_BYTE *)(v22 + *(long *)(a6 + 8));
            } else {
                v54 = (uint8 *)&unk_8186428;
            }
            if (*v54 > 90)
                goto LABEL_95;
            if (*(long *)(a6 + 16) >= v22 && *(long *)(a6 + 8)) {
                v55 = (_BYTE *)(v22 + *(long *)(a6 + 8));
            } else {
                v55 = (uint8 *)&unk_8186428;
            }
            v56 = *(long *)(a6 + 16);
            if (v56 >= v22 && (v57 = *(long *)(a6 + 8)) != 0) {
                v58 = *(_BYTE *)(v57 - 1);
                if (v58 && v58 != 0xff)
                    return -6;
                v59 = *(long *)(a6 + 8);
                if (v59)
                    *(_BYTE *)(v59 - 1) = 0xff;
                v48 = (char *)(v22 + *(long *)(a6 + 8));
            } else {
                v48 = (char *)(uint8 *)&unk_8186428;
            }
            v47 = szDic[*v55 - 0x37];  //   v47 = v63[*v55];
        } else {                       //全部小写字符进入该循环
            v42 = *(long *)(a6 + 16);
            if (v42 >= v22 && (v43 = *(long *)(a6 + 8)) != 0) {
                v44 = *(_BYTE *)(v43 - 1);
                if (v44 && v44 != 0xff)
                    return -7;
                v45 = *(long *)(a6 + 8);
                if (v45)
                    *(_BYTE *)(v45 - 1) = 0xff;
                v46 = (_BYTE *)(v22 + *(long *)(a6 + 8));
            } else {
                v46 = (uint8 *)&unk_8186428;
            }
            if (*(long *)(a6 + 16) >= v22 && *(long *)(a6 + 8)) {
                v47 = szDic[*v46 - 0x57];  // v47 = v62[*v46];
                v48 = (char *)(v22 + *(long *)(a6 + 8));
            } else {
                v48 = (char *)(uint8 *)&unk_8186428;
                v47 = szDic[*v46 - 0x57];  //  v47 = v62[*v46];
            }
        }
        *v48 = v47;
    LABEL_95:
        v21 = *(long *)(a6 + 16);
        ++v22;
    } while (v21 > v22);
    //覆盖输入缓冲
    strcpy(szInOutMWBuf, szBuf + 1);
    return 0;
};

//#38个bin，前31个已知(1+30USBBin)，最后7个怎么来的, param2 0x38字节疑为时钟密文长度
//第一个参数bIsRed暂时猜的，仍然不明确
void BuildMWPlaintext(uint8 bInvType, uint16 usMWBytesSize, uint8 *ab6BytesMWClearTail,
                      uint8 *p0x30USBRaw, char *szOut0x70)
{
    uint8 pOut0x38[0x48] = {0};
    char szBuf[128] = "";
    //无二维码等密文的固定密文长度（字节数）
    uint8 bPreMWSize = 0x38;
    //关于pOut0x38[0]首字节，百旺和航信的应该是留给是否红票的bIsRed，但是ukey并未设置，可能是遗漏，此处
    //暂且同样按照官方不设置，默认为0，跳过该参数
    Str2Byte(pOut0x38 + 1, (char *)p0x30USBRaw, usMWBytesSize * 2);
    memcpy(pOut0x38 + 0x31, ab6BytesMWClearTail, 3);
    // offset 0x35
    *(pOut0x38 + 0x34) = bInvType - 1;
    uint8 v13 = 0x35;
    // offset 0x36
    uint8 v14 = bPreMWSize - v13 - 2;
    if (v14) {
        uint8 *v15 = pOut0x38 + 1;
        do {
            uint8 v16 = *v15 + v15[1];
            v15 += 2;
            *(pOut0x38 + v13++) = v16;
            --v14;
        } while (v14);
    }
    Byte2Str(szBuf, pOut0x38, 0x36);
    mystrlwr(szBuf);
    // offset 0x37-38
    *(uint16 *)(pOut0x38 + 0x36) = bswap_16(NisecBuildVCode((uint8 *)szBuf, strlen(szBuf)));
    Byte2Str(szOut0x70, pOut0x38, 0x38);
    mystrupr(szOut0x70);
}

int CntaxMWDecAPI(uint8 bInvType, bool bIsRed, char *szDeviceID, char *szTaxAuthorityCodeEx,
                  char *kpsj_standard, char *xfsh, uint8 *pabUSBMWRawIn_0x30Bytes, char *szOutMW)
{
    int usMWBytesSize = 0x30;
    uint8 ab6BytesMWClearTail[8] = "";  //实际不足56字节
    if (Build6ByteMWPreTail(bIsRed, kpsj_standard, szDeviceID, szTaxAuthorityCodeEx, xfsh,
                            ab6BytesMWClearTail) < 0)
        return -1;
    // calc PreMW
    BuildMWPlaintext(bInvType, usMWBytesSize, ab6BytesMWClearTail, pabUSBMWRawIn_0x30Bytes,
                     szOutMW);
    if (BuildMWFinal(szOutMW))
        return -2;
    //目前不会出现不为0x70长度验证码
    if (strlen(szOutMW) != 0x70) {
        logout(INFO, "TAXLIB", "发票开具",
               "税盘开具结果查询返回信息中密文长度有误, szOutMW = %d\r\n", strlen(szOutMW));
        return -3;
    }
    return 0x70;
}

// nEncryptOrDecrypt  = 1 encrypt, = 0 decrypt
// 算法实现参考https://github.com/gityf/sm234
int SM4Algorithm(int nEncryptOrDecrypt, uint8 *pSrc, int nSrcLen, char *abIV, uint8 *pOutBuf)
{
    //加解密上下文
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    //加密算法初始化
    unsigned char iv[128] = {0};  // iv先设置空
    int nOutSize = 0;
    do {
        if (!EVP_CipherInit(ctx, EVP_sm4_ecb(), (uint8 *)abIV, iv, nEncryptOrDecrypt))
            break;
        //关闭自动填充，实际只要输入缓冲大小是8的整数，就不会触发填充
        //因为是通用函数，后续其他函数调用时可能有其他填充方式，会造成数据异常
        //因此由需要调用函数传入已填充好的数据
        EVP_CIPHER_CTX_set_padding(ctx, 0);
        if (!EVP_CipherUpdate(ctx, pOutBuf, &nOutSize, pSrc, nSrcLen))
            break;
        //取出最后一块数据（需要填充的），或者是padding补充的数据
        int padding_size = 0;
        if (!EVP_CipherFinal(ctx, pOutBuf + nOutSize, &padding_size))
            break;
        nOutSize += padding_size;
    } while (false);
    //释放上下文
    EVP_CIPHER_CTX_free(ctx);
    return nOutSize;
}

// nEncryptOrDecrypt  = 1 encrypt, = 0 decrypt
int EncryptDecryptV2String(int nEncryptOrDecrypt, uint8 *abInputBuf, int nInputSize,
                           uint8 *abOutBuf)
{
    uint8 abBuf[32] = {0};
    uint8 abIV[16] = {0};
    unsigned char abPreKey[16] = {0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18,
                                  0x19, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26};
    CalcSHA1(abPreKey, 16, abBuf);
    //获取IV  真*反人类变换算法
    memcpy(abIV + 11, abBuf, 5);
    memcpy(abIV + 5, abBuf + 7, 6);
    uint64 ulTmpNum = bswap_64((*(uint64 *)(abIV + 5)) << 16);
    memcpy(abIV + 5, &ulTmpNum, 6);
    memcpy(abIV, abBuf + 15, 5);
    //根据IV进行sm4
    return SM4Algorithm(nEncryptOrDecrypt, abInputBuf, nInputSize, (char *)abIV, abOutBuf);
}

//使用openssl计算sm3, cat foo.txt|openssl dgst -sm3
//算法实现参考https://github.com/gityf/sm234;
//多次EVP_DigestUpdate，可以当作pSrc的拼接，累加后进行总的SM3计算
int CalcSM3(uint8 *pSrc, int nSrcLen, uint8 *pOutDigest)
{
    int nDigestSize = 32;  // SM3_DIGEST_LENGTH
    EVP_MD_CTX *md_ctx = EVP_MD_CTX_new();
    EVP_DigestInit(md_ctx, EVP_sm3());
    EVP_DigestUpdate(md_ctx, pSrc, nSrcLen);
    EVP_DigestFinal(md_ctx, pOutDigest, (unsigned int *)&nDigestSize);
    EVP_MD_CTX_free(md_ctx);
    return nDigestSize;
}

//输入缓冲区实际大小最好不低于24，无数据置0,szDevIDEx为66devid格式
int CntaxBuild16ByteCryptKeyAnd4ByteVCode(char *szDevIDEx, char *szCertPwd, uint8 *abPreHashBegin16,
                                          uint8 *ab16BytesCommand, uint8 *ab16BytesParament,
                                          uint8 *abOutCryptKey, uint8 *abOutUSBCmd4ByteVCode)
{
    //所有都长度尽量都不低于64
    uint8 abPreHash[48] = {0}, abBuf[64] = {0}, abBuf2[64] = {0};
    if (strlen(szDevIDEx) != 14)
        return -1;
    CalcSM3((uint8 *)szCertPwd, strlen(szCertPwd), abPreHash);
    // sm3(20位hash-取前16位)16hash+(2+12 税号)+16设备应答=46bytes 取sm3
    memcpy(abPreHash + 16, szDevIDEx, 14);
    memcpy(abPreHash + 30, abPreHashBegin16, 16);
    CalcSM3(abPreHash, 46, abBuf);
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
    if (SymEncrypt5((long)abPreHash, (long)abBuf, 0x10, (long)abBuf2, (long)abPreHashBegin16))
        return -4;
    memset(abBuf, 0, sizeof abBuf);
    memcpy(abBuf, abOutCryptKey, 16);
    abBuf[16] = 0x80;  // 0x20时，必须
    if (SymEncrypt5((long)abPreHash, (long)abBuf, 0x20, (long)abOutUSBCmd4ByteVCode, (long)abBuf2))
        return -5;
    memmove(abOutUSBCmd4ByteVCode, abOutUSBCmd4ByteVCode + 16, 4);
    return 0;
}

//-------------pkcs11接口，回调hash asn长度计算源码-----------------------------------
int sub_ACC660(unsigned int a1)
{
    unsigned char byte_B4F530[256] = {
        0x00, 0x01, 0x02, 0x02, 0x03, 0x03, 0x03, 0x03, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
        0x04, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
        0x05, 0x05, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
        0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
        0x06, 0x06, 0x06, 0x06, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
        0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
        0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
        0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
        0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
        0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
        0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
        0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
        0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
        0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
        0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
        0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
        0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
        0x08};
    int result;

    if (a1 & 0xFFFF0000) {
        if (a1 & 0xFF000000)
            result = (unsigned __int8)byte_B4F530[a1 >> 24] + 24;
        else
            result = (unsigned __int8)byte_B4F530[a1 >> 16] + 16;
    } else if (a1 & 0xFF00) {
        result = (unsigned __int8)byte_B4F530[a1 >> 8] + 8;
    } else {
        result = (unsigned __int8)byte_B4F530[a1];
    }
    return result;
}

int sub_ACC6B0(long a1)
{
    int result;
    int v2;
    int v3;
    int v4;
    if (a1 && (v2 = *(uint32 *)(a1 + 8)) != 0) {
        uint32 *pDwordNum = (uint32 *)(*(long *)a1 + 4 * (v2 - 1));
        v3 = sub_ACC660(*pDwordNum);
        v4 = v2 - 1;
        result = 32 * v4 + v3;
    } else {
        result = 0;
    }
    return result;
}

//还有个参数a2，用不到
int CalcPkcsHashFormatSize(long a1, uint8 *a2)
{
    long v2;
    int result;
    int v4;
    int v6;
    v2 = a1;

    int nV = sub_ACC6B0(v2);
    v4 = (nV & 7) == 0;
    v6 = v4;
    result = v6 + (sub_ACC6B0(v2) + 7) / 8;
    return result;
}

// hash32Bytes为回调中，设备计算出来的hash翻转位序,原内存不动，获取格式化Asn长度
int DoCalcPkcsHashFormatSize(uint8 *hash32Bytes)
{
    uint8 abMemBufHash[64] = "";
    uint8 abMemBuf[64] = "";
    memcpy(abMemBufHash, hash32Bytes, 0x20);
    ByteReverse(abMemBufHash, 0x20);
    *(long *)abMemBuf = (long)&abMemBufHash;
    *(uint8 *)(abMemBuf + 8) = 0x8;  // 32位4字节，64位8字节；统一为8字节
    return CalcPkcsHashFormatSize((long)abMemBuf, NULL);
}

// nDstBufSize > 72;税局太缺德，不按常规出牌，留了个花样, 标准的asn1，非要搞成特殊的 :-(
int BuildCntaxPkcs11CallbackBuf(uint8 *abSM3HashRet, uint8 *dstBuf, int nDstBufSize)
{
    memset(dstBuf, 0, nDstBufSize);
    uint8 bASize = DoCalcPkcsHashFormatSize(abSM3HashRet);
    uint8 bBSize = DoCalcPkcsHashFormatSize(abSM3HashRet + 0x20);
    uint8 bSize = bASize + bBSize + 4;
    uint8 bIndex = 0;
    dstBuf[0] = 0x30;
    dstBuf[1] = bSize;
    dstBuf[2] = 0x2;
    dstBuf[3] = bASize;
    bIndex = bIndex + (bASize == 0x20 ? 4 : 5);
    memcpy(dstBuf + bIndex, abSM3HashRet, 0x20);
    bIndex += 0x20;
    dstBuf[bIndex] = 0x2;
    dstBuf[bIndex + 1] = bBSize;
    bIndex += (bBSize == 0x20 ? 2 : 3);
    memcpy(dstBuf + bIndex, abSM3HashRet + 0x20, 0x20);
    bIndex += 0x20;
    return bIndex;
}

//----------0002后续版本开票头部0x60长度加密算法 20220615新版本开票追加加密---------------------
unsigned int sub_47B170(uint8 *a1, unsigned int *a2, uint8 *a3)
{
    unsigned int result;
    unsigned int v4;
    unsigned int v5;
    char v6;
    char v7;
    uint8 v8;
    char v9;
    char v10;
    unsigned int v11;
    result = *a2;

    if (!(result & 1)) {
        v4 = *a2;
        result >>= 1;
        if (!*a2 || v4 > result) {
            v4 = result;
            *a2 = result;
        }
        v5 = 0;
        do {
            v6 = *(_BYTE *)(a1 + v5);
            v7 = v6 - 48;
            if ((unsigned __int8)(v6 - 48) > 9u) {
                v8 = v6 - 97;
                v9 = v6 - 87;
                v10 = v6 - 55;
                if (v8 > 5u)
                    v9 = v10;
                v7 = v9;
            }
            v11 = v5++;
            *(_BYTE *)(a3 + (v11 >> 1)) = v7 + 16 * *(_BYTE *)(a3 + (v11 >> 1));
            result = 2 * *a2;
        } while (v5 < result);
    }
    return result;
}

// szDevIDEx为66开头扩展设备号，返回值16字节,abOutBuf>16bytes
int GetHead0x60DevIdSeed(char *szDevID, uint8 *abOutBuf)
{
    char *v3;
    char v4;
    int result;
    uint32_t nSrcDevIdSize = strlen(szDevID);
    sub_47B170((uint8 *)szDevID, &nSrcDevIdSize, abOutBuf + 2);
    v3 = (char *)abOutBuf;
    do {
        v4 = *v3++;
        *(v3 + 7) = ~v4;
    } while (v3 != (char *)(abOutBuf + 8));
    result = 16;
    return result;
}

void Head0x60BytesSelfSeed(uint8 *v19)
{
    uint8 *v9 = NULL;
    int v8 = 0;
    do {
        v9 = &v19[v8];
        do {
            *v9 ^= v9[80 - v8];
            ++v9;
        } while (&v19[16 + v8] != v9);
        v8 += 16;
    } while (v8 != 80);
}

// rc5算法，openssl默认禁用，需要openssl支持必须修改openssl默认编译参数，只能自己实现了;a3>256字节
int sub_491C70(long a1, unsigned int a2, int *a3)
{
    int *v3;
    int v4;
    signed int v5;
    int result;
    unsigned int v7;
    unsigned int v8;
    uint32 v9;
    uint32 v10;
    int *v11;
    _DWORD *v12;
    int v13;
    int v14;

    v3 = a3;
    *a3 = 0xB7E15163;
    do {
        v4 = *v3;
        ++v3;
        *v3 = v4 - 0x61C88647;
    } while (v3 != a3 + 43);
    v5 = 44;
    if (a2 >= 0x2C)
        v5 = a2;
    result = 3 * v5;
    v14 = result;
    if (result) {
        v13 = 0;
        v7 = 0;
        v8 = 0;
        v9 = 0;
        v10 = 0;
        uint32 nNum = 0;
        do {
            ++v13;
            v11 = &a3[v8];
            nNum = v9 + *v11 + v10;
            v10 = __ROL4__(nNum, 3);
            *v11 = v10;
            v12 = (_DWORD *)(a1 + 4 * v7);
            nNum = v10 + *v12 + v9;
            v9 = __ROL4__(nNum, v10 + v9);
            *v12 = v9;
            result = v13;
            v8 = (v8 + 1) % 0x2C;
            v7 = (v7 + 1) % a2;
        } while (v13 != v14);
    }
    return result;
}

int sub_491D80(long a1, long a2)
{
    uint32 v2;
    uint32 v3;
    uint32 v4;
    long v5;
    uint32 v6;
    uint32 v7;
    uint32 v8;
    uint32 v9;
    uint32 v10;
    char v11;
    uint32 v12;
    uint32 v13;
    long v14;
    uint32 v15;
    int result;
    uint32 v17;
    uint32 v18;

    v2 = *(_DWORD *)(a2 + 4) + *(_DWORD *)a1;
    v3 = *(_DWORD *)a2;
    *(_DWORD *)(a2 + 4) = v2;
    v4 = *(_DWORD *)(a2 + 12) + *(_DWORD *)(a1 + 4);
    v18 = v2;
    v17 = *(_DWORD *)(a2 + 8);
    *(_DWORD *)(a2 + 12) = v4;
    v5 = a1 + 8;
    v6 = v2;
    v7 = v3;
    uint32 nNum = 0;
    while (1) {
        v5 += 8;
        nNum = v6 * (2 * v6 + 1);
        v8 = __ROL4__(nNum, 5);
        v9 = v8 ^ v7;
        nNum = v4 * (2 * v4 + 1);
        v10 = __ROL4__(nNum, 5);
        v11 = v10;
        v12 = v17 ^ v10;
        v17 = v4;
        v13 = __ROL4__(v9, v11);
        v14 = *(_DWORD *)(v5 - 8) + v13;
        v15 = __ROL4__(v12, v8);
        v7 = v18;
        *(_DWORD *)a2 = v14;
        v6 = *(_DWORD *)(v5 - 4) + v15;
        *(_DWORD *)(a2 + 8) = v4;
        *(_DWORD *)a2 = v18;
        *(_DWORD *)(a2 + 12) = v14;
        v18 = v6;
        *(_DWORD *)(a2 + 4) = v6;
        if (v5 == a1 + 168)
            break;
        v4 = v14;
    }
    result = a1;
    *(_DWORD *)a2 = *(_DWORD *)(a1 + 168) + v7;
    *(_DWORD *)(a2 + 8) = *(_DWORD *)(a1 + 172) + v4;
    return result;
}

int GetHeadBuffer0x60BytesSm4IV_IV(char *szDeviceId, uint8 *pOutBuf)
{
    uint8 keyRc5[] = {0x01, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00,
                      0x03, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00};
    uint8 szBuf[256] = "";
    GetHead0x60DevIdSeed(szDeviceId, pOutBuf);
    memset(szBuf, 0, sizeof(szBuf));
    sub_491C70((long)keyRc5, 4, (int *)szBuf);
    sub_491D80((long)szBuf, (long)pOutBuf);
    return 0;
}

//根据abBufInput前8字节算出后8字节，最终16字节
void XorTail8Bytes(uint8 *abBufInput)
{
    uint8 v7 = 0;
    uint8 *v6 = abBufInput;
    do {
        v7 = *v6++;
        v6[7] = ~v7;
    } while (v6 != (abBufInput + 8));
}

void GetHeadBuffer0x60BytesSm4IV_InputBuf(uint8 *pOutBuf, int nTableIndex)
{
    uint8 off_13846E0[48] = {0x70, 0xFA, 0x37, 0x1D, 0x06, 0x2E, 0x01, 0xD5, 0x72, 0xDF,
                             0xFD, 0xE7, 0x09, 0xF2, 0x7D, 0x7E, 0xDE, 0x68, 0xCB, 0xD0,
                             0xA6, 0x20, 0xB3, 0x26, 0x1C, 0x24, 0xA6, 0xC3, 0x2F, 0x8F,
                             0xB4, 0xE4, 0x55, 0x4B, 0x33, 0x8E, 0xF5, 0x47, 0xEF, 0x8D,
                             0xA4, 0x75, 0xF3, 0x1A, 0x6F, 0x01, 0xC7, 0x73};
    *(uint64 *)pOutBuf = *(uint64 *)(off_13846E0 + 8 * nTableIndex);
    XorTail8Bytes(pOutBuf);
}

int sub_492280(int a1, _BYTE *a2)
{
    _BYTE *v2;
    __int64 v4;
    signed int v5;
    uint16 usRandom = 0;
    v2 = a2;
    srand((unsigned int)time(NULL));
    do {
        v5 = 4;
        v4 = 0;
        do {
            usRandom = rand();
            v4 = (v4 << 16) + usRandom;
            --v5;
        } while (v5);
        *v2++ = v4;
        //此处可以直接替换随机数，控制变量，*v2++ = 0x11;
    } while (v2 != &a2[a1]);
    return v4;
}

// bGet0x32RandomOrBuildFinalHeadBuf=true，第一次发送设备算密钥，=false第二次最终生成头部buf
void GetEncryptBufferSm4IV(char *szDevId, uint8 *szOutBuf, int nTableIndex)
{
    uint8 abIV[32] = "", abBufInput[16] = {0};
    GetHeadBuffer0x60BytesSm4IV_IV(szDevId, abIV);
    GetHeadBuffer0x60BytesSm4IV_InputBuf(abBufInput, nTableIndex);
    SM4Algorithm(1, abBufInput, 16, (char *)abIV, szOutBuf);
}

// pOutHex0xc0StringBuf > c0+1
void GetHeadBuffer0x60BytesFinalBuf(char *szDevId, int nTableIndex, uint8 *pOrig0xc0HexString,
                                    uint8 *pOutHex0xc0StringBuf)
{
    uint8 szBuf[256] = "", szBuf2[256] = "", abIV[16] = {0};
    //排除头部长度
    Str2Byte(szBuf, (char *)pOrig0xc0HexString, 0xc0);
    Head0x60BytesSelfSeed(szBuf);
    //计算获取IV
    GetEncryptBufferSm4IV(szDevId, abIV, nTableIndex);
    //生成最终buf-bin
    memset(szBuf2, 0, sizeof(szBuf2));
    SM4Algorithm(1, szBuf, 0x60, (char *)abIV, szBuf2);
    memset(pOutHex0xc0StringBuf, 0, 0xc0 + 1);
    Byte2Str((char *)pOutHex0xc0StringBuf, szBuf2, 0x60);
}

void Get32bytesOperKeyVersionBuf_IV(char *szDevId, uint8 *szOutBuf, uint8 *abCleartypeIV)
{
    uint8 abIV[32] = "", abBufInput[16] = {0};
    GetHeadBuffer0x60BytesSm4IV_IV(szDevId, abIV);
    sub_492280(0x10, abBufInput);
    memcpy(abCleartypeIV, abBufInput, 0x10);
    XorTail8Bytes(abBufInput);
    SM4Algorithm(1, abBufInput, 16, (char *)abIV, szOutBuf);
}

void Get32bytesOperKeyVersionBuf_InputBuf(uint8 *abBuf)
{
    int i = 0;
    for (i = 0; i < 16; i++) abBuf[i] = 0xff;
    abBuf[0] = 0x00;
    sub_492280(0xd, abBuf + 1);
    // 0x00[0xd random]0xff0xff
    Get32bytesOperKeyVersionBuf_InputSeed(abBuf);
}

// abOutBuf>32bytes
void Get32bytesOperKeyVersionBuf(char *szDevId, uint8 *abOutBuf)
{
    uint8 abBuf1[256] = "", abCleartypeIV[16] = {0}, abIV[16] = {0};
    Get32bytesOperKeyVersionBuf_IV(szDevId, abIV, abCleartypeIV);
    Get32bytesOperKeyVersionBuf_InputBuf(abBuf1);
    SM4Algorithm(1, abBuf1, 0x10, (char *)abIV, abOutBuf);
    //内存分布 finalHash(0x10)+ClearTypeIV(0x10)
    memcpy(abOutBuf + 0x10, abCleartypeIV, 0x10);
}

void SwapSM4KeyRsBitAndIndex(SM4_KEY *key)
{
    uint32 *result;
    uint32 v4;
    uint32 v5;
    uint8 *a1 = (uint8 *)key;
    uint32 *v2 = (uint32 *)(a1 + 31 * 4);
    result = (uint32 *)a1;
    do {
        v4 = *result;
        v5 = *v2;
        --v2;
        ++result;
        *(result - 1) = v5;
        v2[1] = v4;
    } while (v2 != (uint32 *)(a1 + 15 * 4));
}

//此处需要在设置ctx后对ctx内SM4 subkeys进行异或，无法直接使用sm4计算函数及EVP系列函数
//因此只能使用底层SM4_系列函数
void CustomSM4WithKeyXor(uint8 *ab16bytesIv, uint8 *abInputOutput)
{
    SM4_KEY key;
    uint8 abOutBuf[16];
    SM4_set_key(ab16bytesIv, &key);
    SwapSM4KeyRsBitAndIndex(&key);
    SM4_encrypt((const unsigned char *)abInputOutput, abOutBuf, &key);
    memcpy(abInputOutput, abOutBuf, 16);
}

// abOutBuf>32bytes
int GetAlgIndexFromUsbReponse(char *szDevId, uint8 *abUsbResponse)
{
    uint8 abBuf1[128] = "", abBuf2[128] = "", abIV[16] = {0};
    uint8 bAlgNo = 0;
    //取IV值
    GetHeadBuffer0x60BytesSm4IV_IV(szDevId, abIV);
    //算第一次SM4
    memcpy(abBuf1, abUsbResponse + 0x10, 16);
    XorTail8Bytes(abBuf1);
    SM4Algorithm(1, abBuf1, 0x10, (char *)abIV, abBuf2);
    //第二次 xorSm4Key再算
    memcpy(abBuf1, abUsbResponse, 16);
    CustomSM4WithKeyXor(abBuf2, abBuf1);
    //计算最终index值
    memcpy(abBuf2, abBuf1, 16);
    Get32bytesOperKeyVersionBuf_InputSeed(abBuf2);
    if (memcmp(abBuf1 + 14, abBuf2 + 14, 2))
        return -1;  //此处算出来的16字节值最后2个字节应该一致，否则计算出错，官方是这么判定的
    bAlgNo = abBuf1[0];
    if (bAlgNo > 5)
        return -2;  //后续索引表只有48字节，5*8=40,==6时读取越界
    return bAlgNo;
}