/*************************************************
File name:   ta_algorithm.c
Description: 该文件主要存放软件中逆向出来税务总局和各开票软件支撑的的算法结构，
             统一存放也便于其他模块调用，该文件中不得存放非算法函数
Author:      Zako
Version:     1.0
Date:        2020.04
History:
20221008     针对上海电子税务局ClientHello算法追加SHA1算法，进行改动
*************************************************/
#include "ta_algorithm.h"

void FPHM_SignStrFormat(char *fphm, char *pc_OutFphm)
{
    if (strlen(fphm) == 0) {
        memset(pc_OutFphm, '0', 8);
        return;
    }
    int i_fphmlen = strlen(fphm);
    // fphm长度小于8
    if (i_fphmlen < 8) {
        if (1) {
            //前面补零
            memset(pc_OutFphm, '0', 8);
            memcpy(pc_OutFphm + 8 - strlen(fphm), fphm, strlen(fphm));
            return;
        } else {
            //后面补零
            memset(pc_OutFphm, '0', 8);
            memcpy(pc_OutFphm, fphm, strlen(fphm));
            return;
        }
    } else {
        // fphm长度大于8 截取前8位
        memcpy(pc_OutFphm, fphm, 8);
        return;
    }
}

//填充签名商品信息
void FillSignSpxx(char *szSignStr, struct Spxx *spxx, int num)
{
    char szBuf[512] = {0};
    char spsm_spmc[512] = {0};
    //序号
    memset(szBuf, 0, sizeof(szBuf));
    sprintf(szBuf, "%d", num);
    strcat(szSignStr, szBuf);
    //商品名称
    memset(spsm_spmc, 0, sizeof(spsm_spmc));
    sprintf(spsm_spmc, "%s%s", spxx->spsmmc, spxx->spmc);
    strcat(szSignStr, spsm_spmc);
    //规格型号
    strcat(szSignStr, spxx->ggxh);
    //计量单位
    strcat(szSignStr, spxx->jldw);
    //数量
    if (strlen(spxx->sl) == 0)
        strcat(szSignStr, "0.00000000");
    else {
        PriceRound(spxx->sl, 8, szBuf);
        strcat(szSignStr, szBuf);
    }
    //单价
    if (strlen(spxx->dj) == 0)
        strcat(szSignStr, "0.00000000");
    else {
        if (!strlen(spxx->hsjbz) || !strcmp("0", spxx->hsjbz))  // no tax
            PriceRound(spxx->dj, 8, szBuf);
        else
            PriceRemoveTax(spxx->dj, spxx->slv, 8, szBuf);
        strcat(szSignStr, szBuf);
    }
    //金额
    if (strlen(spxx->je) == 0)
        strcat(szSignStr, "0.00");
    else {
        PriceRound(spxx->je, 2, szBuf);
        strcat(szSignStr, szBuf);
    }
    //税率
    if (strlen(spxx->slv) == 0)
        strcat(szSignStr, "0.00");
    else {
        PriceRound(spxx->slv, 2, szBuf);
        strcat(szSignStr, szBuf);
    }
    //税额
    if (strlen(spxx->se) == 0)
        strcat(szSignStr, "0.00");
    else {
        PriceRound(spxx->se, 2, szBuf);
        strcat(szSignStr, szBuf);
    }
}

char *GetSignString(HFPXX fpxx, int *pnRetBufLen)
{
    // 20211108 电子专票部分签名为蒙柏方临时测试后加入，待后续使用测试稳定后删除该注释
    struct Spxx *spxx = NULL;
    char szBuf[512] = {0};  //不能低于512
    int nInvBuffLen = CalcAllocBuffLen(fpxx);
    int nChildRet = 0;
    char *pOrigSignString = calloc(1, nInvBuffLen);
    if (!pOrigSignString)
        return NULL;
    char *pRetSignString = calloc(1, nInvBuffLen);
    if (!pRetSignString) {
        free(pOrigSignString);
        return NULL;
    }
    //发票代码
    strcat(pOrigSignString, fpxx->fpdm);
    //发票号码
    memset(szBuf, 0, sizeof(szBuf));
    FPHM_SignStrFormat(fpxx->fphm, szBuf);
    strcat(pOrigSignString, szBuf);
    //开票日期(必须保证yyyyMMdd格式8个字节)
    memset(szBuf, 0, sizeof(szBuf));
    memcpy(szBuf, fpxx->kpsj_F1, 8);
    strcat(pOrigSignString, szBuf);
    //密文
    strcat(pOrigSignString, fpxx->mw);
    if (fpxx->fplx_aisino == FPLX_AISINO_PTFP || fpxx->fplx_aisino == FPLX_AISINO_DZFP ||
        fpxx->fplx_aisino == FPLX_AISINO_DZZP)
        strcat(pOrigSignString, fpxx->jym);  //校验码
    //机器编号
    strcat(pOrigSignString, fpxx->jqbh);
    //购方名称
    strcat(pOrigSignString, fpxx->gfmc);
    //购方税号
    strcat(pOrigSignString, fpxx->gfsh);
    //购方地址电话
    strcat(pOrigSignString, fpxx->gfdzdh);
    //购方银行账号
    strcat(pOrigSignString, fpxx->gfyhzh);
    //金额
    PriceRound(fpxx->je, 2, szBuf);
    strcat(pOrigSignString, szBuf);
    //税额
    PriceRound(fpxx->se, 2, szBuf);
    strcat(pOrigSignString, szBuf);
    //价税合计
    long double fPrice = strtold(fpxx->je, NULL) + strtold(fpxx->se, NULL);
    sprintf(szBuf, "%.24Lf", fPrice);
    PriceRound(szBuf, 2, szBuf);
    strcat(pOrigSignString, szBuf);
    //销方名称
    strcat(pOrigSignString, fpxx->xfmc);
    //销方税号
    strcat(pOrigSignString, fpxx->xfsh);
    //销方地址电话
    strcat(pOrigSignString, fpxx->xfdzdh);
    //销方银行账号
    strcat(pOrigSignString, fpxx->xfyhzh);
    //收款人
    strcat(pOrigSignString, fpxx->skr);
    //复核人
    strcat(pOrigSignString, fpxx->fhr);
    //开票人
    strcat(pOrigSignString, fpxx->kpr);
    //备注
    strcat(pOrigSignString, fpxx->bz);
    //判断是否是红票
    if (fpxx->isRed) {
        if (fpxx->fplx_aisino == FPLX_AISINO_ZYFP || fpxx->fplx_aisino == FPLX_AISINO_HYFP ||
            fpxx->fplx_aisino == FPLX_AISINO_DZZP)
            strcat(pOrigSignString, fpxx->redNum);  // ZYFP
        else {
            strcat(pOrigSignString, fpxx->blueFpdm);
            memset(szBuf, 0, sizeof(szBuf));
            FPHM_SignStrFormat(fpxx->blueFphm, szBuf);
            strcat(pOrigSignString, szBuf);
        }
    }
    //是否是作废
    if (fpxx->zfbz)
        strcat(pOrigSignString, "Y");
    else
        strcat(pOrigSignString, "N");

    if (fpxx->zyfpLx == ZYFP_NCP_XS) {
        strcat(pOrigSignString, "01");
    } else if (fpxx->zyfpLx == ZYFP_NCP_SG) {
        strcat(pOrigSignString, "02");
    } else {
        if (fpxx->zyfpLx != ZYFP_XT_CCP && fpxx->zyfpLx != ZYFP_XT_YCL &&
            fpxx->zyfpLx != ZYFP_XT_JSJHJ) {
            if (fpxx->zyfpLx != ZYFP_XT_CPJGF) {
                if (fpxx->zyfpLx != ZYFP_CPY) {
                    goto HANDLE1;
                } else if (fpxx->zyfpLx == FPLX_AISINO_ZYFP && fpxx->isRed) {
                    strcat(pOrigSignString, "18");
                    goto HANDLE1;
                }
                strcat(pOrigSignString, "08");
                goto HANDLE1;
            }
        }
        strcat(pOrigSignString, "03");
    }
HANDLE1:
    spxx = NULL;
    if ((fpxx->fplx_aisino == FPLX_AISINO_DZFP) || (fpxx->fplx_aisino == FPLX_AISINO_DZZP)) {
        //商品明细
        int i_InvoiceNum = 1;
        spxx = fpxx->stp_MxxxHead->stp_next;
        if (spxx != NULL) {
            while (spxx) {
                FillSignSpxx(pOrigSignString, spxx, i_InvoiceNum);
                spxx = spxx->stp_next;
                i_InvoiceNum++;
            }
        }
    } else if ((fpxx->fplx_aisino == FPLX_AISINO_PTFP) || (fpxx->fplx_aisino == FPLX_AISINO_ZYFP)) {
        //添加商品清单头
        if (strcmp(fpxx->qdbj, "Y") == 0) {
            spxx = fpxx->stp_MxxxHead;
            FillSignSpxx(pOrigSignString, spxx, 1);
            spxx = spxx->stp_next;
        } else {
            spxx = fpxx->stp_MxxxHead->stp_next;
        }
        //商品明细
        int i_InvoiceNum = 1;
        if (spxx != NULL) {
            while (spxx) {
                FillSignSpxx(pOrigSignString, spxx, i_InvoiceNum);
                spxx = spxx->stp_next;
                i_InvoiceNum++;
            }
        }
    }
    //签名不可能有\0字符，如果发票内容是UTF8则签名也必须为UTF8
    if (fpxx->utf8Invoice)
        nChildRet = G2U_UTF8Ignore((char *)pOrigSignString, strlen(pOrigSignString),
                                   (char *)pRetSignString, nInvBuffLen);
    else
        nChildRet = U2G_GBKIgnore((char *)pOrigSignString, strlen(pOrigSignString),
                                  (char *)pRetSignString, nInvBuffLen);
    if (nChildRet < 0) {
        free(pOrigSignString);
        free(pRetSignString);
        return NULL;
    }
    *pnRetBufLen = nChildRet;
    free(pOrigSignString);
    return pRetSignString;
}

//目前采用的是模板套用，替换变量的方式获取，后期有需求有条件的可以将这些静态替换的方法改为由x509证书计算出来
int SelectRetSignFormatModel(X509 *pX509Cert, char *pFetchBuff)
{
    // get sn
    uint8 sz7ByteSN[32];
    if (GetCertSNByteOrStr(pX509Cert, (char *)sz7ByteSN, 1) < 0)
        return -1;
    X509_NAME *pCommonName = X509_get_issuer_name(pX509Cert);
    if (!pCommonName)
        return -1;
    int nNameLen = 64;
    char csCommonName[64] = {0};
    nNameLen = X509_NAME_get_text_by_NID(pCommonName, NID_commonName, csCommonName, nNameLen);
    if (-1 == nNameLen)
        return -2;
    if (!memcmp(csCommonName, "\x4e\x2d\x56\xfd\x7a\x0e\x52\xa1\x8b\xa4\x8b\xc1\x4e\x2d\x5f\xc3",
                16)) {
        //中国税务认证中心模板 L = 北京 CN = 中国税务认证中心 S = 北京 OU = CA C = cn
        uint8 data[318] = {
            0x30, 0x82, 0x01, 0x3A, 0x06, 0x09, 0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01, 0x07,
            0x02, 0xA0, 0x82, 0x01, 0x2B, 0x30, 0x82, 0x01, 0x27, 0x02, 0x01, 0x01, 0x31, 0x0B,
            0x30, 0x09, 0x06, 0x05, 0x2B, 0x0E, 0x03, 0x02, 0x1A, 0x05, 0x00, 0x30, 0x0B, 0x06,
            0x09, 0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01, 0x07, 0x01, 0x31, 0x82, 0x01, 0x06,
            0x30, 0x82, 0x01, 0x02, 0x02, 0x01, 0x01, 0x30, 0x60, 0x30, 0x55, 0x31, 0x0B, 0x30,
            0x09, 0x06, 0x03, 0x55, 0x04, 0x06, 0x13, 0x02, 0x63, 0x6E, 0x31, 0x0D, 0x30, 0x0B,
            0x06, 0x03, 0x55, 0x04, 0x0B, 0x1E, 0x04, 0x00, 0x43, 0x00, 0x41, 0x31, 0x0D, 0x30,
            0x0B, 0x06, 0x03, 0x55, 0x04, 0x08, 0x1E, 0x04, 0x53, 0x17, 0x4E, 0xAC, 0x31, 0x19,
            0x30, 0x17, 0x06, 0x03, 0x55, 0x04, 0x03, 0x1E, 0x10, 0x4E, 0x2D, 0x56, 0xFD, 0x7A,
            0x0E, 0x52, 0xA1, 0x8B, 0xA4, 0x8B, 0xC1, 0x4E, 0x2D, 0x5F, 0xC3, 0x31, 0x0D, 0x30,
            0x0B, 0x06, 0x03, 0x55, 0x04, 0x07, 0x1E, 0x04, 0x53, 0x17, 0x4E, 0xAC, 0x02, 0x07,
            0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x30, 0x09, 0x06, 0x05, 0x2B, 0x0E, 0x03,
            0x02, 0x1A, 0x05, 0x00, 0x30, 0x0D, 0x06, 0x09, 0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D,
            0x01, 0x01, 0x01, 0x05, 0x00, 0x04, 0x81, 0x80, 0xD7, 0xB0, 0x60, 0xFF, 0x5E, 0x39,
            0x66, 0x0E, 0xB4, 0x73, 0xB3, 0xE9, 0x97, 0x1D, 0xE6, 0x09, 0x7F, 0x1A, 0x35, 0x74,
            0xF7, 0x33, 0x5F, 0xC8, 0xC6, 0x41, 0xA6, 0x80, 0x68, 0xC8, 0x1D, 0x82, 0xED, 0x7C,
            0xED, 0x3A, 0x65, 0xB3, 0x00, 0xC4, 0x16, 0xC6, 0xC6, 0x45, 0x84, 0xB4, 0x07, 0x6F,
            0xA0, 0xF3, 0x4F, 0x7D, 0xFA, 0x83, 0x7D, 0x6F, 0x43, 0xDC, 0x66, 0x6F, 0x4E, 0xDB,
            0x55, 0x37, 0x02, 0xA8, 0x6F, 0x6D, 0x3A, 0x76, 0xC5, 0x46, 0xAF, 0x53, 0xD5, 0xD6,
            0x55, 0x20, 0xC9, 0x37, 0x90, 0x43, 0x73, 0x32, 0xEF, 0x2F, 0xBE, 0xB4, 0xF6, 0x51,
            0xAE, 0x55, 0x70, 0x90, 0xCC, 0x65, 0x56, 0x8E, 0x46, 0x6E, 0xC7, 0x88, 0xA5, 0x42,
            0xFD, 0xD2, 0x1C, 0x8A, 0x8E, 0xD7, 0xAC, 0xE5, 0x3D, 0x00, 0xFE, 0x6C, 0x07, 0x69,
            0x1D, 0x8C, 0x9C, 0xE3, 0xBB, 0xDF, 0xF7, 0x12, 0x6C, 0xC6};
        // 0x11,为填充字符,将会被替换,格式为asn1;最后0x80字节hash将会被替换
        memcpy(data + 0x9a, sz7ByteSN, 7);
        memcpy(pFetchBuff, data, sizeof(data));
        return sizeof(data);
    } else if (!memcmp(csCommonName,
                       "\x7a\x0e\x52\xa1\x75\x35\x5b\x50\x8b\xc1\x4e\x66\x7b\xa1\x74\x06\x4e\x2d"
                       "\x5f\xc3",
                       20)) {
        //税务电子证书管理中心模板, # Description = ca100002 CN = 税务电子证书管理中心 OU =
        //国家税务总局 C=cn
        uint8 data[327] = {
            0x30, 0x82, 0x01, 0x43, 0x06, 0x09, 0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01, 0x07,
            0x02, 0xA0, 0x82, 0x01, 0x34, 0x30, 0x82, 0x01, 0x30, 0x02, 0x01, 0x01, 0x31, 0x0B,
            0x30, 0x09, 0x06, 0x05, 0x2B, 0x0E, 0x03, 0x02, 0x1A, 0x05, 0x00, 0x30, 0x0B, 0x06,
            0x09, 0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01, 0x07, 0x01, 0x31, 0x82, 0x01, 0x0F,
            0x30, 0x82, 0x01, 0x0B, 0x02, 0x01, 0x01, 0x30, 0x69, 0x30, 0x5E, 0x31, 0x0B, 0x30,
            0x09, 0x06, 0x03, 0x55, 0x04, 0x06, 0x13, 0x02, 0x63, 0x6E, 0x31, 0x15, 0x30, 0x13,
            0x06, 0x03, 0x55, 0x04, 0x0B, 0x1E, 0x0C, 0x56, 0xFD, 0x5B, 0xB6, 0x7A, 0x0E, 0x52,
            0xA1, 0x60, 0x3B, 0x5C, 0x40, 0x31, 0x1D, 0x30, 0x1B, 0x06, 0x03, 0x55, 0x04, 0x03,
            0x1E, 0x14, 0x7A, 0x0E, 0x52, 0xA1, 0x75, 0x35, 0x5B, 0x50, 0x8B, 0xC1, 0x4E, 0x66,
            0x7B, 0xA1, 0x74, 0x06, 0x4E, 0x2D, 0x5F, 0xC3, 0x31, 0x19, 0x30, 0x17, 0x06, 0x03,
            0x55, 0x04, 0x0D, 0x1E, 0x10, 0x00, 0x63, 0x00, 0x61, 0x00, 0x31, 0x00, 0x30, 0x00,
            0x30, 0x00, 0x30, 0x00, 0x30, 0x00, 0x32, 0x02, 0x07, 0x11, 0x11, 0x11, 0x11, 0x11,
            0x11, 0x11, 0x30, 0x09, 0x06, 0x05, 0x2B, 0x0E, 0x03, 0x02, 0x1A, 0x05, 0x00, 0x30,
            0x0D, 0x06, 0x09, 0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01, 0x01, 0x01, 0x05, 0x00,
            0x04, 0x81, 0x80, 0x83, 0x6B, 0x84, 0x64, 0xFD, 0xD7, 0xFF, 0xDC, 0x6A, 0xB5, 0xF6,
            0xE2, 0x75, 0x09, 0x97, 0xB3, 0x57, 0x54, 0xD9, 0x09, 0xDF, 0x5C, 0x51, 0xBB, 0xE7,
            0xCB, 0x98, 0x6B, 0xD1, 0x13, 0x11, 0xF8, 0xA2, 0x68, 0xE8, 0xFF, 0x09, 0x69, 0x60,
            0x12, 0x44, 0x2C, 0x92, 0x7E, 0x0C, 0x57, 0x4C, 0x44, 0x9F, 0x6A, 0x61, 0xA1, 0x53,
            0xF5, 0x13, 0x25, 0xFE, 0x9B, 0xA7, 0x5D, 0x80, 0xE9, 0x21, 0xE0, 0xA2, 0x3A, 0x44,
            0xF9, 0xDF, 0x2B, 0xA9, 0x88, 0xF5, 0x59, 0x64, 0x21, 0x7D, 0x7E, 0x84, 0x95, 0xCE,
            0x65, 0xD7, 0xAF, 0x84, 0xD6, 0x28, 0x43, 0xC2, 0x5A, 0xAE, 0xA0, 0xC7, 0x1B, 0xA1,
            0x50, 0xF0, 0x45, 0xDF, 0xB6, 0xDF, 0x31, 0xA8, 0x2A, 0xB5, 0xB8, 0x0E, 0xC1, 0xBF,
            0xE6, 0xE3, 0xE5, 0x4B, 0x75, 0xE8, 0xCE, 0xA9, 0x0F, 0x9E, 0x15, 0x15, 0x40, 0xF6,
            0x45, 0x47, 0xC2, 0x6E, 0x70};
        // 0x11,为填充字符,将会被替换,格式为asn1;最后0x80字节hash将会被替换
        memcpy(data + 163, sz7ByteSN, 7);
        memcpy(pFetchBuff, data, sizeof(data));
        return sizeof(data);
    } else if (!memcmp(csCommonName,
                       "\xe7\xa8\x8e\xe5\x8a\xa1\xe7\x94\xb5\xe5\xad\x90\xe8\xaf\x81\xe4\xb9\xa6"
                       "\xe7\xae",
                       20)) {
        //税务电子证书管理中心(SM2)模板, # CN = 税务电子证书管理中心(SM2) OU = 国家税务总局 C=CN
        unsigned char data[265] = {
            0x30, 0x82, 0x01, 0x05, 0x06, 0x0A, 0x2A, 0x81, 0x1C, 0xCF, 0x55, 0x06, 0x01, 0x04,
            0x02, 0x02, 0xA0, 0x81, 0xF6, 0x30, 0x81, 0xF3, 0x02, 0x01, 0x01, 0x31, 0x0E, 0x30,
            0x0C, 0x06, 0x08, 0x2A, 0x81, 0x1C, 0xCF, 0x55, 0x01, 0x83, 0x11, 0x05, 0x00, 0x30,
            0x0C, 0x06, 0x0A, 0x2A, 0x81, 0x1C, 0xCF, 0x55, 0x06, 0x01, 0x04, 0x02, 0x01, 0x31,
            0x81, 0xCF, 0x30, 0x81, 0xCC, 0x02, 0x01, 0x01, 0x30, 0x63, 0x30, 0x58, 0x31, 0x0B,
            0x30, 0x09, 0x06, 0x03, 0x55, 0x04, 0x06, 0x13, 0x02, 0x43, 0x4E, 0x31, 0x1B, 0x30,
            0x19, 0x06, 0x03, 0x55, 0x04, 0x0B, 0x0C, 0x12, 0xE5, 0x9B, 0xBD, 0xE5, 0xAE, 0xB6,
            0xE7, 0xA8, 0x8E, 0xE5, 0x8A, 0xA1, 0xE6, 0x80, 0xBB, 0xE5, 0xB1, 0x80, 0x31, 0x2C,
            0x30, 0x2A, 0x06, 0x03, 0x55, 0x04, 0x03, 0x0C, 0x23, 0xE7, 0xA8, 0x8E, 0xE5, 0x8A,
            0xA1, 0xE7, 0x94, 0xB5, 0xE5, 0xAD, 0x90, 0xE8, 0xAF, 0x81, 0xE4, 0xB9, 0xA6, 0xE7,
            0xAE, 0xA1, 0xE7, 0x90, 0x86, 0xE4, 0xB8, 0xAD, 0xE5, 0xBF, 0x83, 0x28, 0x53, 0x4D,
            0x32, 0x29, 0x02, 0x07, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x30, 0x0C, 0x06,
            0x08, 0x2A, 0x81, 0x1C, 0xCF, 0x55, 0x01, 0x83, 0x11, 0x05, 0x00, 0x30, 0x0C, 0x06,
            0x08, 0x2A, 0x81, 0x1C, 0xCF, 0x55, 0x01, 0x83, 0x75, 0x05, 0x00, 0x04, 0x46, 0x30,
            0x44, 0x02, 0x20, 0x50, 0x5E, 0xCD, 0x53, 0xA7, 0x46, 0x04, 0x5B, 0x79, 0xA0, 0x57,
            0x0C, 0x87, 0x23, 0xCC, 0xE1, 0x8D, 0xCC, 0xCE, 0xAF, 0x94, 0x52, 0xCA, 0x3B, 0x7C,
            0xC6, 0x87, 0xF3, 0xA9, 0x80, 0x63, 0x15, 0x02, 0x20, 0x3E, 0xA7, 0xD0, 0xBE, 0x7B,
            0xE7, 0xBE, 0x67, 0x0D, 0x9A, 0xB3, 0x6A, 0xEC, 0x25, 0x0D, 0xC1, 0xB4, 0x0D, 0x2F,
            0x40, 0x41, 0x26, 0xB3, 0x04, 0x5C, 0x51, 0xAC, 0xEF, 0x84, 0x0C, 0xD6, 0x56};
        // 0x11,为填充字符,将会被替换,格式为asn1;最后0x2+0x20+0x2+0x20=0x44字节hash将会被替换
        memcpy(data + 158, sz7ByteSN, 7);
        memcpy(pFetchBuff, data, sizeof(data));
        return sizeof(data);
    } else
        return -10;
}

//生成开票字符串hash，并将使用证书的SN值填进去，最终生成可用开票签名；如果是批量开票，这里的SN不需要每次都获取，可以在初始化的时候获取一次，保存下来就可以
//记得返回的内存需要及时free
//注意！！sSignStrGBKBuff不会是一个简单的字符串，有可能是带\0的截断buff,因此需要配合nGBKBuffLen
char *GetDeviceSignHashBase64(HDEV hDev, const char *sSignStrGBKBuff, int nGBKBuffLen)
{
    char szSignHash[0x100] = "";  //可以为SHA1Hash 0x80字节，SM3Hash0x40字节，多设置点
    int nRecvBuffLen = 2048;
    int nDeviceSignRepLen = 0;
    char *pRecvBuff = (char *)calloc(1, nRecvBuffLen);
    char *pRetB64Buf = NULL;
    if (!pRecvBuff)
        return NULL;
    if (DEVICE_AISINO == hDev->bDeviceType) {
        nDeviceSignRepLen = 0x80;
        if (InvSignStrToDev128ByteRep(hDev->hUSB, hDev->bCryptBigIO, sSignStrGBKBuff, nGBKBuffLen,
                                      pRecvBuff, nRecvBuffLen) < 0)
            goto SignHash_Finish;
    } else if (DEVICE_NISEC == hDev->bDeviceType) {
        nDeviceSignRepLen = 0x80;
        if (GetDevice128ByteSignRep(hDev, sSignStrGBKBuff, nGBKBuffLen, pRecvBuff, nRecvBuffLen) <
            0)
            goto SignHash_Finish;
    } else if (DEVICE_CNTAX == hDev->bDeviceType) {
        nDeviceSignRepLen = 0x44;
        memset(pRecvBuff, 0, nRecvBuffLen);
        if (CntaxGetDevice64ByteSignRepAsn1(hDev, false, sSignStrGBKBuff, nGBKBuffLen,
                                            pRecvBuff + 3, nRecvBuffLen - 3) < 0)
            goto SignHash_Finish;
        //这里需要fix下 返回的64字节，但是需要适当变换下，64字节分成两段
        //官方在实现时感觉有问题？偶尔有0插入这两段的首字节，长度为64-66字节不等，我们取最标准长度64
    } else {
        free(pRecvBuff);
        return NULL;
    }
    //默认偏移3字节
    memcpy(szSignHash, pRecvBuff + 3, nDeviceSignRepLen);
    int nModelLen = SelectRetSignFormatModel(hDev->pX509Cert, pRecvBuff);
    if (nModelLen < 0)
        goto SignHash_Finish;
    memcpy(pRecvBuff + nModelLen - nDeviceSignRepLen, szSignHash, nDeviceSignRepLen);
    pRetB64Buf = (char *)calloc(1, nModelLen * 2 + 256);
    if (!pRetB64Buf)
        goto SignHash_Finish;
    _WriteHexToDebugFile("mysign-bin.txt", (uint8 *)pRecvBuff, nModelLen);
    Base64_Encode((const char *)pRecvBuff, nModelLen, (char *)pRetB64Buf);

SignHash_Finish:
    free(pRecvBuff);
    return pRetB64Buf;
}

//--------------------Begin-----------上报发票证书税盘auth算法-------------------------
//----------------------cntax 税局通信算法-----------------------------------
int CheckBufferLength(long a1, int32 *a2, unsigned int a3)
{
    int result;
    unsigned int v4;
    result = (int)*a2;
    if (a2) {
        if (a1) {
            v4 = *a2;
            *a2 = a3;
            if (v4 >= a3)
                result = -1;
            else
                result = 8;  // dst buffer to small
        } else {
            *a2 = a3;
            result = 0;
        }
    }
    return result;
}

int sub_E347E70(uint8 *a1, int v4, uint8 *a3, int *a4)
{
    int v5;
    _WORD *v6;
    v5 = 0;
    if (v4 > 0) {
        v6 = (_WORD *)a3;
        do {
            sprintf((char *)v6, "%02x", *(_BYTE *)(v5 + a1));
            ++v5;
            ++v6;
        } while (v5 < v4);
    }
    *(_DWORD *)a4 = 2 * v4;
    return 0;
}

uint32 GetMd5SumHeadDword(void *pSrc, int nSrcLen)
{
    uint8 szDst[24] = "";  // just 16
    CalcMD5(pSrc, nSrcLen, szDst);
    uint32 *dRet = (uint32 *)szDst;
    return *dRet;
}

int sub_1CB57C0(int a2, long a3, _DWORD *a4, LPVOID *a6);
//两个小模块通用算法结束

// cryp_api.dll#Crypt_ClientAuth Main algo
// Ret 返回值至少在1k以上，如果低于这个值，就是错的
//至少支持两个省份以上，才可以
int CntaxBuildAuthCode(uint8 bDeviceType, int a1, long a2, long lpMem, long a4, long a5, uint8 *a6,
                       int a7, int a8, int a9, uint8 *a10, int a11, uint8 *a12, int a13,
                       void *dstBuf, int32 *a15)
{
    uint8 aAuthclientauth[] = "AUTHCLIENTAUTH_2";
    unsigned int v51 = 0;
    int v34;
    long l54 = 0;
    int v16;
    int v17;
    int v19;
    void *dstBufRef;
    long v53;
    int nChildRet = 0;
    int v46;
    void *dstBufRefv21;
    int v23;
    bool v24;
    int v15 = 0;
    int v29;
    int v30;
    int v33;

    long v36;
    long v37;
    _DWORD *v38;
    int v39;
    _DWORD *v52;
    _DWORD *v41;
    void *v50 = NULL;
    int v42;
    long v43;
    long v44;
    int v45;

    if ((unsigned int)a1 > 2) {
        if (a7 == 36) {
            v51 = 16;
            l54 = (long)a6 + 20;
        }
        v16 = v51 + 1;
    }
    if ((unsigned int)a7 > 0x14)
        a7 = 20;
    v17 = a9;
    int n1 = a13;
    int v18 = sub_1CB57C0(100, (long)a12, (uint32 *)&n1, (void *)&v50);
    if (v18)
        return -1;
    v19 = a13 + a11 + a7 + a5 + lpMem + v16;
    if (v17 == 32)
        v19 += 44;
    dstBufRef = dstBuf;
    v53 = v19 + 42;
    nChildRet = CheckBufferLength((long)dstBuf, a15, v19 + 42);
    if ((long)nChildRet >= 0) {
        return -1;  //输出缓冲太小
    }
    memset(dstBufRef, 0, v19 + 42);
    dstBufRefv21 = (uint8 *)dstBufRef + 42;
    dstBuf = (uint8 *)dstBufRef + 42;
    if (a9 == 32) {
        v52 = (uint32 *)((uint8 *)dstBufRef + 42);
        dstBufRefv21 = (uint8 *)dstBufRef + 86;
        dstBuf = (uint8 *)dstBufRef + 86;
    }
    memcpy(dstBufRef, aAuthclientauth, strlen((char *)aAuthclientauth));
    *((_WORD *)dstBufRef + 20) = v19;
    *((_WORD *)dstBufRef + 8) = a1;
    v23 = lpMem;
    v24 = lpMem == 0;
    *((_WORD *)dstBufRef + 9) = lpMem;
    if (!v24) {
        v15 = v23;
        memcpy(dstBufRefv21, (const void *)a2, v23);
    }
    v24 = a5 == 0;
    *((_WORD *)dstBufRef + 10) = a5;
    if (!v24) {
        memcpy((uint8 *)dstBufRefv21 + v15, (const void *)a4, a5);
        v15 += a5;
    };
    *((_WORD *)dstBufRef + 11) = a7;
    memcpy((uint8 *)dstBufRefv21 + v15, a6, a7);
    v29 = a11;
    v30 = a7 + v15;
    *((_WORD *)dstBufRef + 16) = a11;
    v29 = (v29 >> 2) * 4;
    memcpy((uint8 *)dstBufRefv21 + v30, a10, v29);
    if (bDeviceType == DEVICE_CNTAX) {
        int nV3 = a11 & 3;
        memcpy(dstBufRefv21 + v30 + v29, a10 + v29, nV3);
        v29 += 2;
    }
    v33 = a13;
    v34 = v29 + v30;
    *((_WORD *)dstBufRef + 17) = a13;
    v33 = (v33 >> 2) * 4 + (a13 & 0x3);
    memcpy((uint8 *)dstBufRefv21 + v34, a12, v33);
    *((_DWORD *)dstBufRef + 6) = a8;
    v24 = a9 == 32;
    *((_DWORD *)dstBufRef + 7) = a9;
    if (!v24)
        return -2;
    a7 = v33;
    a5 = 0;
    //--new code
    nChildRet = sub_1CB57C0(1, (long)a12, (_DWORD *)&a7, (LPVOID *)&lpMem);
    v36 = (long)a12;
    v37 = lpMem;
    v38 = v52;
    v39 = (long)a12 << 16;
    //这里算出来与之际值相差4，可能因为系统指针不一样原因？第三字节差4，先手工平一下，后期其他盘再测试
    *v52 = *(_DWORD *)lpMem + ((v34 + 0xFFFF * (long)a12 + *(_DWORD *)(lpMem + 4)) << 16) +
           0x40000;  // + 0x40000;
    v38[10] = *(_DWORD *)(v37 + 20) + ((v34 + v39 - v36 + *(_DWORD *)(v37 + 24)) << 16);
    v38[9] = 6;  // MD5withRSA
    if (lpMem) {
        free((void *)lpMem);
        lpMem = 0;
    }
    nChildRet = sub_1CB57C0(19, (long)a12, (uint32 *)&a7, (void *)&a5);
    if (bDeviceType == DEVICE_CNTAX) {
        // v34为根据签名长度计算出来的值，与开票软件实际计算相差1，手动-1参与计算
        int nV34minusOne = v34 - 1;
        int n0x41 = *((uint8 *)v50 + 0x44);  // 0x41
        // v36 unknow
        int nAlgmIDAndSignID =
            ((nV34minusOne + 0xFFFF * v36 + *((_DWORD *)v50 + 18)) << 16) + n0x41 + 0xFFFF;
        v38[8] = nAlgmIDAndSignID;
    } else {  // nisec aisino
        v38[8] = *(_DWORD *)a5 + ((v34 + 0xFFFF * v36 + *(_DWORD *)(a5 + 4)) << 16);
    }
    if (a5) {
        free((void *)a5);
        a5 = 0;
    }
    v41 = v50;
    v42 = v41[2];
    v43 = (long)(v41 + 8);
    v38[1] = *(_DWORD *)(v43 - 28) + ((v34 + 0xFFFF * v36 + v42) << 16);
    v38[6] = *(_DWORD *)(v43 - 8) + ((v34 + 0xFFFF * v36 + *(_DWORD *)(v43 - 4)) << 16);
    v38[7] = *(_DWORD *)(v43 + 16) + ((v34 + 0xFFFF * v36 + *(_DWORD *)(v43 + 20)) << 16);
    v44 = (long)(v38 + 2);
    *(_DWORD *)v44 = *(_DWORD *)v43;
    *(_DWORD *)(v44 + 4) = *(_DWORD *)(v43 + 4);
    *(_DWORD *)(v44 + 8) = *(_DWORD *)(v43 + 8);
    v45 = *(_DWORD *)(v43 + 12);
    dstBufRefv21 = dstBuf;
    *(_DWORD *)(v44 + 12) = v45;
    //--new code finish
    dstBufRefv21 = dstBuf;
    v46 = a13 + v34;
    if ((unsigned int)a1 > 2) {
        v24 = v51 == 0;
        *((_BYTE *)dstBufRefv21 + v46) = v51;
        if (!v24)
            memcpy((uint8 *)dstBufRefv21 + v46 + 1, (void *)l54, v51);
    }
    //上面计算部分是错的，直接采用标准静态值覆盖可能错误的值
    // ERROR@103@证书状态未知
    ((uint8 *)dstBufRef)[0x30] = 0x69;
    ((uint8 *)dstBufRef)[0x31] = 0x00;
    // ERROR@114@CA未被信任
    ((uint8 *)dstBufRef)[0x42] = 0x5a;
    ((uint8 *)dstBufRef)[0x44] = 0x7e;
    // unknow
    ((uint8 *)dstBufRef)[0x46] = 0x88;
    // ERROR@57@密码算法ID错误 我方计算OK
    //((uint8 *)dstBufRef)[0x4a] = 0x40;
    // ERROR@39@签名验证错误
    //((uint8 *)dstBufRef)[0x4c] = 0x9b;
    //((uint8 *)dstBufRef)[0x4d] = 0x01;
    // unknow，税局好像也不验证，不验证或者不出错的就不强行修改了
    ((uint8 *)dstBufRef)[0x4e] = 0x17;
    ((uint8 *)dstBufRef)[0x52] = 0x46;
    ((uint8 *)dstBufRef)[0x54] = 0x99;
    *((_DWORD *)dstBufRef + 9) = GetMd5SumHeadDword(dstBufRef, v53);
    if (v50)
        free(v50);
    return *a15;
}

//----------------------------aisino nisec 税局通信算法---------------------------------------
//没啥用，就是过程太长了，单独建个函数，便于分析

int Fillv50(X509 *p509, uint8 *a3, _DWORD *a4, int nDW, LPVOID *a6, uint8 **pPKOut)
{
    uint8 *pSN = NULL;
    uint8 *pIssue = NULL;
    uint8 *pUser = NULL;
    uint8 *pPubKey = NULL;
    // NID_sm3WithSM2Sign              1195
    // NID_sha1WithRSAEncryption 65
    bool bSha1SignCert = X509_get_signature_nid(p509) == NID_sha1WithRSAEncryption;
    while (1) {
        ASN1_INTEGER *asn1_i = NULL;
        uint8 abSN[32] = "";
        BIGNUM *bignum = NULL;
        uint8 *serial = NULL;
        asn1_i = X509_get_serialNumber(p509);
        bignum = ASN1_INTEGER_to_BN(asn1_i, NULL);
        if (bignum == NULL)
            break;
        serial = (uint8 *)BN_bn2hex(bignum);
        BN_free(bignum);
        if (serial == NULL)
            break;
        Str2Byte(abSN, (char *)serial, strlen((char *)serial));
        OPENSSL_free(serial);
        pSN = MyMemSearch(a3, a3 + *a4, abSN, 7);
        if (!pSN)
            break;
        //颁发者指针
        if (bSha1SignCert) {
            // 1.2.840.113549.1.1.5 sha1RSA
            uint8 sz[] = "\x2a\x86\x48\x86\xf7\x0d\x01\x01";
            //因为颁发者节点无明显查找特征，只能通过查找sha1rsa标识，后移
            pIssue = MyMemSearch(a3, a3 + *a4, sz, sizeof(sz) - 1);
            if (!pIssue)
                break;
            pIssue = pIssue + sizeof(sz);
        } else {
            // 1.2.156.10197.1.501 sm3?
            uint8 sz[] = "\x2a\x81\x1c\xcf\x55\x01\x83\x75";
            //因为颁发者节点无明显查找特征，只能通过查找sha1rsa标识，后移
            pIssue = MyMemSearch(a3, a3 + *a4, sz, sizeof(sz) - 1);
            if (!pIssue)
                break;
            pIssue = pIssue + sizeof(sz);
        }
        //使用者指针
        ASN1_TIME *end = X509_get_notAfter(p509);
        pUser = MyMemSearch(a3, a3 + *a4, (uint8 *)end->data, 13);
        if (!pUser)
            break;
        pUser += 13;
        //公钥
        ASN1_BIT_STRING *pub_key_bit_str = NULL;
        if (!(pub_key_bit_str = X509_get0_pubkey_bitstr(p509)))
            break;
        int nEncLen = i2d_ASN1_BIT_STRING(pub_key_bit_str, (uint8 **)pPKOut);
        if (nEncLen <= 0 || nEncLen > 200) {
            break;
        }
        pPubKey = MyMemSearch(a3, a3 + *a4, *pPKOut, nEncLen);
        if (!pPubKey)
            break;
        pPubKey -= 2;
        break;
    }
    if (!pSN || !pIssue || !pUser || !pPubKey)
        return -1;
    if (*a6)
        free(*a6);
    *a6 = (LPVOID)calloc(1, 256);
    uint8 *pBuff = *a6;
    if (bSha1SignCert) {
        uint8 dataOID[48] = {0x31, 0x2E, 0x32, 0x2E, 0x38, 0x34, 0x30, 0x2E, 0x31, 0x31,
                             0x33, 0x35, 0x34, 0x39, 0x2E, 0x31, 0x2E, 0x31, 0x2E, 0x35,
                             0x00, 0xF0, 0xAD, 0xBA, 0x31, 0x2E, 0x32, 0x2E, 0x38, 0x34,
                             0x30, 0x2E, 0x31, 0x31, 0x33, 0x35, 0x34, 0x39, 0x2E, 0x31,
                             0x2E, 0x31, 0x2E, 0x31, 0x00, 0xF0, 0xAD, 0xBA};
        memcpy(pBuff + 28 * nDW, dataOID, sizeof(dataOID));  // 0x70
    } else {
        unsigned char dataOID[48] = {0x31, 0x2E, 0x32, 0x2E, 0x31, 0x35, 0x36, 0x2E, 0x31, 0x30,
                                     0x31, 0x39, 0x37, 0x2E, 0x31, 0x2E, 0x35, 0x30, 0x31, 0x00,
                                     0x31, 0x2E, 0x32, 0x2E, 0x38, 0x34, 0x30, 0x2E, 0x31, 0x30,
                                     0x30, 0x34, 0x35, 0x2E, 0x32, 0x2E, 0x31, 0x00, 0xAD, 0xBA,
                                     0x0D, 0xF0, 0xAD, 0xBA, 0x0D, 0xF0, 0xAD, 0xBA};
        memcpy(pBuff + 28 * nDW, dataOID, sizeof(dataOID));  // 0x70
    }
    long lCopy = 0;
    lCopy = 0x2;
    memcpy(pBuff + 0 * nDW, &lCopy, nDW);
    lCopy = 0x7;
    memcpy(pBuff + 1 * nDW, &lCopy, nDW);
    lCopy = (long)pSN;
    memcpy(pBuff + 2 * nDW, &lCopy, nDW);
    lCopy = (long)pBuff + 28 * nDW;
    memcpy(pBuff + 3 * nDW, &lCopy, nDW);
    lCopy = 0x2;
    memcpy(pBuff + 4 * nDW, &lCopy, nDW);
    lCopy = (long)pIssue;
    memcpy(pBuff + 5 * nDW, &lCopy, nDW);
    lCopy = 0x60;
    memcpy(pBuff + 6 * nDW, &lCopy, nDW);
    lCopy = (long)pIssue + 2;
    memcpy(pBuff + 7 * nDW, &lCopy, nDW);
    if (bSha1SignCert) {
        uint8 sz2[] = "\xE3\x07\x07\x12\x00\x00\x00\x00\xED\x07\x07\x12\x00\x00\x00\x00";
        memcpy(pBuff + 8 * nDW, sz2, 16);  // 0x20
    } else {
        unsigned char sz2[16] = {0xE4, 0x07, 0x01, 0x0A, 0x00, 0x00, 0x00, 0x00,
                                 0xEE, 0x07, 0x01, 0x0A, 0x00, 0x00, 0x00, 0x00};
        memcpy(pBuff + 8 * nDW, sz2, 16);  // 0x20
    }
    lCopy = 0x79;
    memcpy(pBuff + 12 * nDW, &lCopy, nDW);
    lCopy = (long)pUser;
    memcpy(pBuff + 13 * nDW, &lCopy, nDW);
    lCopy = (long)pBuff + 28 * nDW + 0x18;
    memcpy(pBuff + 14 * nDW, &lCopy, nDW);
    lCopy = 0x2;
    memcpy(pBuff + 15 * nDW, &lCopy, nDW);
    lCopy = (long)pPubKey;
    memcpy(pBuff + 16 * nDW, &lCopy, nDW);
    if (bSha1SignCert)
        lCopy = 0x8c;
    else
        lCopy = 0x41;
    memcpy(pBuff + 17 * nDW, &lCopy, nDW);
    lCopy = (long)pPubKey + 6;
    memcpy(pBuff + 18 * nDW, &lCopy, nDW);
    return 0;
}

int sub_1CB57C0(int a2, long a3, _DWORD *a4, LPVOID *a6)
{
    int nRet = -1;
    uint8 *pCert = (uint8 *)a3;
    uint8 *pPKOut = NULL;
    X509 *p509 = NULL;
    if (!d2i_X509(&p509, (const uint8 **)&pCert, *a4))
        return -1;
    // NID_sm3WithSM2Sign              1195
    // NID_sha1WithRSAEncryption 65
    bool bSha1SignCert = X509_get_signature_nid(p509) == NID_sha1WithRSAEncryption;
    //为什么64位linux分配的堆地址是32位的，为了尽量减少逆向工作量和安全稳定，先用32位的测试，后期如果有问题再修改,如果这里存储地址4转8位，那么外面的地址都需要大改动
    //   int nDW = sizeof(long);
    int nDW = 4;
    //这里应该是根据证书生成一段大约0x30字节的内存，因为涉及底层x509太多，无法全部还原，
    //目前只能按照x509的oid来分别返回内存，后期如果证书变化，这里的内存可能变化？
    if (a2 == 1) {
        if (*a6)
            free(*a6);
        *a6 = (LPVOID)calloc(1, 256);
        uint8 *pBuff = *a6;
        if (bSha1SignCert)
            strcpy((char *)pBuff + 8 * nDW,
                   "1.2.840.113549.1.1.5");  //前64位为32和和64位变长，64位之后固定
        else
            strcpy((char *)pBuff + 8 * nDW, "1.2.156.10197.1.501");
        long lCopy = 0;
        if (bSha1SignCert)
            lCopy = 0x04ff;
        else
            lCopy = 0x052a;
        memcpy(pBuff + 0 * nDW, &lCopy, nDW);
        lCopy = a3;
        memcpy(pBuff + 1 * nDW, &lCopy, nDW);
        lCopy = (long)(pBuff + 8 * nDW);
        memcpy(pBuff + 2 * nDW, &lCopy, nDW);
        lCopy = 0x02;
        memcpy(pBuff + 3 * nDW, &lCopy, nDW);
        lCopy = a3 + *a4 - 0x86;
        memcpy(pBuff + 4 * nDW, &lCopy, nDW);
        lCopy = 0x80;
        memcpy(pBuff + 5 * nDW, &lCopy, nDW);
        lCopy = a3 + *a4 - 0x80;
        memcpy(pBuff + 6 * nDW, &lCopy, nDW);
        nRet = 0;
    } else if (a2 == 19) {
        ASN1_BIT_STRING *pub_key_bit_str = NULL;
        uint8 *p = NULL;
        while (1) {
            if (!(pub_key_bit_str = X509_get0_pubkey_bitstr(p509)))
                break;
            int nEncLen = i2d_ASN1_BIT_STRING(pub_key_bit_str, (uint8 **)&pPKOut);
            if (nEncLen <= 0 || nEncLen > 200) {
                break;
            }
            p = (uint8 *)MyMemSearch((uint8 *)a3, (uint8 *)(a3 + *a4), (uint8 *)pPKOut, nEncLen);
            if (!p)
                break;
            break;
        }
        if (!p)
            goto sub_1CB57C0_Finish;
        if (*a6)
            free(*a6);
        *a6 = (LPVOID)calloc(1, 128);
        uint8 *pBuff = *a6;
        long lCopy = 0;
        lCopy = 0x80;
        memcpy(pBuff + 0 * nDW, &lCopy, nDW);
        lCopy = (long)p + 11;
        memcpy(pBuff + 1 * nDW, &lCopy, nDW);
        lCopy = 0x3;
        memcpy(pBuff + 2 * nDW, &lCopy, nDW);
        lCopy = (long)p + 11 + 0x82;
        memcpy(pBuff + 3 * nDW, &lCopy, nDW);
        nRet = 0;
    } else if (a2 == 100) {  //自定义
        if (Fillv50(p509, (uint8 *)a3, a4, nDW, a6, &pPKOut) < 0)
            goto sub_1CB57C0_Finish;
        nRet = 0;
    }

sub_1CB57C0_Finish:
    if (pPKOut)
        free(pPKOut);
    if (p509)
        X509_free(p509);
    return nRet;
}

// cryp_api.dll#Crypt_ClientAuth Main algo
// Ret 返回值至少在1k以上，如果低于这个值，就是错的
int CommmonBuildAuthCode(uint8 bDeviceType, int a1, long a2, long lpMem, long a4, long a5,
                         uint8 *a6, int a7, int a8, int a9, uint8 *a10, int a11, uint8 *a12,
                         int a13, void *a14, int32 *a15)
{
    int v15 = 0;
    int v16;
    int v17;
    int v18 = 0;
    int v19;
    void *v20;
    void *v21;
    int v23;
    bool v24;
    int v25;
    int v26;
    const void *v27;
    int v28;
    int v29;
    int v30;
    const void *v31;
    int v32;
    int v33;
    int v34;
    const void *v35;
    long v36;
    long v37;
    _DWORD *v38;
    int v39;
    _DWORD *v41;
    int v42;
    long v43;
    long v44;
    int v45;
    int v46;
    unsigned int v47;
    long v48;
    void *v50;
    unsigned int v51;
    _DWORD *v52;
    long v53;
    long l54 = 0;
    int nChildRet = 0;
    uint8 aAuthclientauth[] = "AUTHCLIENTAUTH_2";
    v16 = 0;
    v50 = 0;
    int nFalse = 0;

    v51 = 0;
    if ((unsigned int)a1 > 2) {
        if (a7 == 36) {
            v51 = 16;
            l54 = (long)a6 + 20;
        }
        v16 = v51 + 1;
    }
    if ((unsigned int)a7 > 0x14)
        a7 = 20;
    v17 = a9;
    if (a9 != 32) {
        if (a9 != 64)
            return 88;
    LABEL_11:
        v19 = a13 + a11 + a7 + a5 + lpMem + v16;
        if (v17 == 32)
            v19 += 44;
        v20 = a14;
        v53 = v19 + 42;
        nChildRet = CheckBufferLength((long)a14, a15, v19 + 42);
        if ((long)nChildRet >= 0) {
            nFalse = 1;
            goto LABEL_32;
        }
        memset(v20, 0, v19 + 42);
        v21 = (uint8 *)v20 + 42;
        a14 = (uint8 *)v20 + 42;
        if (a9 == 32) {
            v52 = (uint32 *)((uint8 *)v20 + 42);
            v21 = (uint8 *)v20 + 86;
            a14 = (uint8 *)v20 + 86;
        }
        memcpy(v20, aAuthclientauth, strlen((char *)aAuthclientauth));
        *((_WORD *)v20 + 20) = v19;
        *((_WORD *)v20 + 8) = a1;
        v23 = lpMem;
        v24 = lpMem == 0;
        *((_WORD *)v20 + 9) = lpMem;
        if (!v24) {
            v15 = v23;
            memcpy(v21, (const void *)a2, v23);
        }
        v25 = a5;
        v24 = a5 == 0;
        *((_WORD *)v20 + 10) = a5;
        if (!v24) {
            memcpy((uint8 *)v21 + v15, (const void *)a4, v25);
            v15 += v25;
        }
        v26 = a7;
        v27 = (const void *)a6;
        *((_WORD *)v20 + 11) = a7;
        v28 = v26;
        memcpy((uint8 *)v21 + v15, v27, v26);
        v29 = a11;
        v30 = v28 + v15;
        v31 = (const void *)a10;
        *((_WORD *)v20 + 16) = a11;
        v32 = v29;
        v29 = (v29 >> 2) * 4;
        memcpy((uint8 *)v21 + v30, v31, v29);
        v33 = a13;
        v34 = v32 + v30;
        v35 = (const void *)a12;
        *((_WORD *)v20 + 17) = a13;
        v33 = (v33 >> 2) * 4 + (a13 & 0x3);
        memcpy((uint8 *)v21 + v34, v35, v33);
        *((_DWORD *)v20 + 6) = a8;
        v24 = a9 == 32;
        *((_DWORD *)v20 + 7) = a9;
        if (!v24)
            goto LABEL_28;
        a7 = v33;
        lpMem = 0;
        a5 = 0;
        nChildRet = sub_1CB57C0(1, (long)a12, (_DWORD *)&a7, (LPVOID *)&lpMem);
        if (!nChildRet) {
            v36 = (long)a12;
            v37 = lpMem;
            v38 = v52;
            v39 = (long)a12 << 16;
            //这里算出来与之际值相差4，可能因为系统指针不一样原因？第三字节差4，先手工平一下，后期其他盘再测试
            *v52 = *(_DWORD *)lpMem + ((v34 + 0xFFFF * (long)a12 + *(_DWORD *)(lpMem + 4)) << 16) +
                   0x40000;  // + 0x40000;
            v38[10] = *(_DWORD *)(v37 + 20) + ((v34 + v39 - v36 + *(_DWORD *)(v37 + 24)) << 16);
            v38[9] = 6;  // MD5withRSA
            if (lpMem) {
                free((void *)lpMem);
                lpMem = 0;
            }
            if (!nChildRet) {
                nChildRet = sub_1CB57C0(19, (long)a12, (uint32 *)&a7, (void *)&a5);
                if (!nChildRet) {
                    v38[8] = *(_DWORD *)a5 + ((v34 + 0xFFFF * v36 + *(_DWORD *)(a5 + 4)) << 16);
                    if (a5) {
                        free((void *)a5);
                        a5 = 0;
                    }
                    v41 = v50;
                    v42 = v41[2];
                    v43 = (long)(v41 + 8);
                    v38[1] = *(_DWORD *)(v43 - 28) + ((v34 + 0xFFFF * v36 + v42) << 16);
                    v38[6] =
                        *(_DWORD *)(v43 - 8) + ((v34 + 0xFFFF * v36 + *(_DWORD *)(v43 - 4)) << 16);
                    v38[7] = *(_DWORD *)(v43 + 16) +
                             ((v34 + 0xFFFF * v36 + *(_DWORD *)(v43 + 20)) << 16);
                    v44 = (long)(v38 + 2);
                    *(_DWORD *)v44 = *(_DWORD *)v43;
                    *(_DWORD *)(v44 + 4) = *(_DWORD *)(v43 + 4);
                    *(_DWORD *)(v44 + 8) = *(_DWORD *)(v43 + 8);
                    v45 = *(_DWORD *)(v43 + 12);
                    v21 = a14;
                    *(_DWORD *)(v44 + 12) = v45;
                LABEL_28:
                    v46 = a13 + v34;
                    if ((unsigned int)a1 > 2) {
                        v47 = v51;
                        v24 = v51 == 0;
                        *((_BYTE *)v21 + v46) = v51;
                        if (!v24)
                            memcpy((uint8 *)v21 + v46 + 1, (void *)l54, v47);
                    }
                    v48 = v53;
                    *((_DWORD *)v20 + 9) = GetMd5SumHeadDword(v20, v48);
                    goto LABEL_32;
                }
            }
        }
    LABEL_32:
        v18 = *a15;
        goto LABEL_33;
    }
    //自己加的,初始化v50
    int n1 = a13;
    v18 = sub_1CB57C0(100, (long)a12, (uint32 *)&n1, (void *)&v50);
    if (!v18)
        goto LABEL_11;
LABEL_33:
    if (v50)
        free(v50);
    if (nFalse)
        return -1;
    return v18;
}

int Crypt_ClientAuth(uint8 *a3, int a4, uint8 *szDst, int32 *pnRetDstLen, HDEV hDev)
{
    if (strncmp((char *)a3, "AUTHSERVERHELLO2", 16))
        return -1;
    //经典认证44字节，电子税务局系统认证1400字节左右   1560  最大能达到多少
    // if ((a4 != 0x44 && a4 < 1280) || a4 > 1560)
    if (a4 < 0x44)
        return -2;
    uint8 *pRepBody = (uint8 *)a3 + 0x20, *pSignAddr = NULL;
    int nRepBodyLen = 0x24;  // sha1 20 + md5 16
    uint8 signRet[256] = "";
    int nSignBuffLen = sizeof(signRet), nCerDerLen = 0, a8 = 0, a11SignSize = 0, nRet = 0;
    uint8 *pCertDer = NULL;

    if (DEVICE_AISINO == hDev->bDeviceType) {
        //这里存在一个问题，税盘返回签名后128字节后，再去读取时应该还有一个success的反馈，但是这里没有；那么先不管这个，不对后续反馈响应进行判断，只判断内容
        if (InvSignStrToDev128ByteRep(hDev->hUSB, hDev->bCryptBigIO, (char *)pRepBody, 0x14,
                                      (char *)signRet, nSignBuffLen) < 0)
            return -3;
        a8 = 0x6;
        a11SignSize = 0x80;
        pSignAddr = (uint8 *)signRet + 3;
    } else if (DEVICE_NISEC == hDev->bDeviceType) {
        if (GetDevice128ByteSignRep(hDev, (char *)pRepBody, 0x14, (char *)signRet, nSignBuffLen) <
            0)
            return -3;
        a8 = 0x6;
        a11SignSize = 0x80;
        pSignAddr = (uint8 *)signRet + 3;
    } else if (DEVICE_CNTAX == hDev->bDeviceType) {
        //和其他两个设备一样，合计0x20字节发送至设备签名
        if (CntaxGetDevice64ByteSignRepAsn1(hDev, true, (char *)pRepBody, 0x14, (char *)signRet,
                                            nSignBuffLen) < 0)
            return -3;
        //返回值一般为0x46，70个字节
        a8 = 0x17;
        a11SignSize = 0x46;
        pSignAddr = (uint8 *)signRet;
    } else
        return -3;
    X509 *p509 = hDev->pX509Cert;
    nCerDerLen = i2d_X509(p509, &pCertDer);
    if (DEVICE_AISINO == hDev->bDeviceType || DEVICE_NISEC == hDev->bDeviceType) {
        // signRet 头部默认空白偏移3字节
        nRet =
            CommmonBuildAuthCode(hDev->bDeviceType, 3, 0, 0, 0, 0, pRepBody, nRepBodyLen, a8, 0x20,
                                 pSignAddr, a11SignSize, pCertDer, nCerDerLen, szDst, pnRetDstLen);
    } else {  // cntax
        nRet = CntaxBuildAuthCode(hDev->bDeviceType, 3, 0, 0, 0, 0, pRepBody, nRepBodyLen, a8, 0x20,
                                  pSignAddr, a11SignSize, pCertDer, nCerDerLen, szDst, pnRetDstLen);
    }
    free(pCertDer);
    if (nRet < 1024)
        return -5;
    return 0;
}

int BuildClientAuthCode(uint8 *pRepRadom1, int nRepRadom1Size, uint8 *pOutBuf, int *pnOutSize,
                        HDEV hDev)
{
    int v16;
    int nOutSize;
    int nRandomSize;
    int v39;

    uint8 abRandomBuf[5120] = "";//目前测试到有存在pRepRadom1超过2236字节情况，此处内存小会造成内存问题。
    nRandomSize = Str2Byte(abRandomBuf, (char *)pRepRadom1, nRepRadom1Size);
    v16 = Crypt_ClientAuth((uint8 *)abRandomBuf, nRandomSize, (uint8 *)pOutBuf, pnOutSize, hDev);
    if (v16 < 0)
        return -1;
    nOutSize = *(int *)pnOutSize;
    if (nOutSize < 0 || nOutSize > 500 * 1024)
        return -2;
    v39 = nOutSize * 3 + 1024;
    uint8 *v43 = calloc(1, v39);
    sub_E347E70(pOutBuf, nOutSize, (uint8 *)v43, &v39);
    memcpy(pOutBuf, v43, v39);
    free(v43);
    *pnOutSize = v39;
    return 0;
}

//--------------------Finish-----------上报发票证书税盘auth算法-------------------------
//--------------------Begin-----------Html转义算法-------------------------
// libevent中存在HTML转义，可以尝试替换++todo,evhttp_htmlescape
#if __GNUC__ >= 3
#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)
#else
#define likely(x) (x)
#define unlikely(x) (x)
#endif

const char *ESCAPED_STRING[] = {
    "", "&quot;", "&amp;", "&#39;", "&lt;", "&gt;",
};

// This is strlen(ESCAPED_STRING[x]) optimized specially.
// Mapping: 1 => 6, 2 => 5, 3 => 5, 4 => 4, 5 => 4
#define ESC_LEN(x) ((13 - x) / 2)

//   Given ASCII-compatible character, return index of ESCAPED_STRING.
//  " (34) => 1 (&quot;)
//  & (38) => 2 (&amp;)
//  ' (39) => 3 (&#39;)
//  < (60) => 4 (&lt;)
//  > (62) => 5 (&gt;)
static const char HTML_ESCAPE_TABLE[] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 1, 0, 0, 0, 2, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 0, 5, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

uint8 *ensure_allocated(uint8 *buf, size_t size, size_t *asize)
{
    if (size < *asize)
        return buf;
    size_t new_size;
    if (*asize == 0) {
        new_size = size;
    } else {
        new_size = *asize;
    }
    // Increase buffer size by 1.5x if realloced multiple times.
    while (new_size < size) new_size = (new_size << 1) - (new_size >> 1);
    *asize = new_size;
    return realloc(buf, new_size);
}

size_t append_pending_buf(uint8 *rbuf, size_t rbuf_i, const uint8 *buf, size_t buf_i, size_t esize)
{
    memcpy(rbuf + rbuf_i, buf + (rbuf_i - esize), buf_i - (rbuf_i - esize));
    return buf_i + esize;
}

size_t append_escaped_buf(uint8 *rbuf, size_t rbuf_i, size_t esc_i, size_t *esize)
{
    memcpy(rbuf + rbuf_i, ESCAPED_STRING[esc_i], ESC_LEN(esc_i));
    *esize += ESC_LEN(esc_i) - 1;
    return rbuf_i + ESC_LEN(esc_i);
}

//无转义操作返回值为0，无需释放dest；返回值不为0，则需要释放
size_t HtmlEscape(uint8 **dest, const uint8 *buf, size_t size)
{
    size_t asize = 0, esc_i, esize = 0, i = 0, rbuf_i = 0;
    uint8 *rbuf = NULL;
    while (i < size) {
        // Loop here to skip non-escaped characters fast.
        while (i < size && (esc_i = HTML_ESCAPE_TABLE[buf[i]]) == 0) i++;

        if (esc_i) {
            rbuf =
                ensure_allocated(rbuf, sizeof(uint8) * (size + esize + ESC_LEN(esc_i) + 1), &asize);
            rbuf_i = append_pending_buf(rbuf, rbuf_i, buf, i, esize);
            rbuf_i = append_escaped_buf(rbuf, rbuf_i, esc_i, &esize);
        }
        i++;
    }
    if (rbuf_i == 0) {
        // Return given buf and size if there are no escaped characters.
        *dest = (uint8 *)buf;
        return 0;
    } else {
        append_pending_buf(rbuf, rbuf_i, buf, size, esize);
        rbuf[size + esize] = '\0';
        *dest = rbuf;
        return size + esize;
    }
}
//--------------------finish-----------Html转义算法-------------------------
#define IN_URANGE(a, b, c) (a >= (unsigned int)b && a <= (unsigned int)c)

static inline int needs_encoding(char ch, char next)
{
    if (IN_URANGE(ch, 0xD800, 0xDBFF)) {
        if (!IN_URANGE(next, 0xDC00, 0xDFFF)) {
            return -1;
        }
    }
    // alpha capital/small
    if (IN_URANGE(ch, 0x0041, 0x005A) || IN_URANGE(ch, 0x061, 0x007A)) {
        return 0;
    }
    // decimal digits
    if (IN_URANGE(ch, 0x0030, 0x0039)) {
        return 0;
    }
    // reserved chars
    // - _ . ! ~ * ' ( )
    if ('-' == ch || '_' == ch || '.' == ch || '!' == ch || '~' == ch || '*' == ch || '(' == ch ||
        ')' == ch) {
        return 0;
    }
    return 1;
}

char *UriEncode(const char *src)
{
    int i = 0;
    size_t size = 0;
    size_t len = 0;
    size_t msize = 0;
    char *enc = NULL;
    char tmp[4];
    char ch = 0;
    // chars length
    len = strlen(src);
#define push(c) (enc[size++] = c)
    for (; i < len; ++i) {
        switch (needs_encoding(src[i], src[i + 1])) {
            case -1:
                // @TODO - handle with uri error
                free(enc);
                return NULL;

            case 0:
                msize++;
                break;

            case 1:
                msize = (msize + 3);  // %XX
                break;
        }
    }
    // alloc with probable size
    enc = (char *)malloc((sizeof(char) * msize) + 1);
    if (NULL == enc) {
        return NULL;
    }
    // reset
    i = 0;
    // encode
    while (len--) {
        ch = src[i++];
        // if high surrogate ensure next is
        // low surrogate for valid surrogate
        // pair
        if (needs_encoding(ch, src[i])) {
            // encode
            sprintf(tmp, "%%%x", ch & 0xff);
            // prefix
            push(tmp[0]);
            // codec
            push(toupper(tmp[1]));
            push(toupper(tmp[2]));
        } else {
            push(ch);
        }
    }
    enc[size] = '\0';
#undef push
    return enc;
}

char *UriDecode(const char *src)
{
    int i = 0;
    size_t size = 0;
    size_t len = 0;
    char *dec = NULL;
    char tmp[3];
    char ch = 0;
    // chars len
    len = strlen(src);
    // alloc
    dec = (char *)malloc(len + 1);
#define push(c) (dec[size++] = c)
    // decode
    while (len--) {
        ch = src[i++];
        // if prefix `%' then read byte and decode
        if ('%' == ch) {
            tmp[0] = src[i++];
            tmp[1] = src[i++];
            tmp[2] = '\0';
            push(strtol(tmp, NULL, 16));
        } else {
            push(ch);
        }
    }
    dec[size] = '\0';
#undef push
    return dec;
}
//--------------------finish-----------Uri encodeURIComponent算法-------------------------

// https://www.zytrax.com/books/ldap/apb/asn1.pdf
void BuildClientHelloString(uint8 bDeviceType, uint8 *pIn7BytesCertSN,
                            uint8 *pInRandomSHA1OptionalNode, char *pOutClientHelloString)
{
    int nAsnModel = 0;
    // 标准无web扩展SHA1节点的asn1模板,利用模板直接加载，避免一个个单独push节点，比较麻烦
    uint8 abAsn1Model[768] = "";
    if (bDeviceType == DEVICE_AISINO || bDeviceType == DEVICE_NISEC) {
        //  "cn"\"国家税务总局"\"税务电子证书管理中心"\"ca100002"
        //
        // aisino-web-example(bNeedType=1):30818f020103305e310b300906035504061302636e31153013060355040b1e0c56fd5bb67a0e52a1603b5c40311d301b06035504031e147a0e52a175355b508bc14e667ba174064e2d5fc331193017060355040d1e100063006100310030003000300030003202071401000081aeaba016041419736ff3d2b5c8bb82f9d9befd81bd9770c53a2ea209310702010102020402
        // aisino-classical-example(bNeedType=0):3077020103305e310b300906035504061302636e31153013060355040b1e0c56fd5bb67a0e52a1603b5c40311d301b06035504031e147a0e52a175355b508bc14e667ba174064e2d5fc331193017060355040d1e100063006100310030003000300030003202071401000081aeaba209310702010102020402
        unsigned char data[121] = {
            0x30, 0x77, 0x02, 0x01, 0x03, 0x30, 0x5E, 0x31, 0x0B, 0x30, 0x09, 0x06, 0x03, 0x55,
            0x04, 0x06, 0x13, 0x02, 0x63, 0x6E, 0x31, 0x15, 0x30, 0x13, 0x06, 0x03, 0x55, 0x04,
            0x0B, 0x1E, 0x0C, 0x56, 0xFD, 0x5B, 0xB6, 0x7A, 0x0E, 0x52, 0xA1, 0x60, 0x3B, 0x5C,
            0x40, 0x31, 0x1D, 0x30, 0x1B, 0x06, 0x03, 0x55, 0x04, 0x03, 0x1E, 0x14, 0x7A, 0x0E,
            0x52, 0xA1, 0x75, 0x35, 0x5B, 0x50, 0x8B, 0xC1, 0x4E, 0x66, 0x7B, 0xA1, 0x74, 0x06,
            0x4E, 0x2D, 0x5F, 0xC3, 0x31, 0x19, 0x30, 0x17, 0x06, 0x03, 0x55, 0x04, 0x0D, 0x1E,
            0x10, 0x00, 0x63, 0x00, 0x61, 0x00, 0x31, 0x00, 0x30, 0x00, 0x30, 0x00, 0x30, 0x00,
            0x30, 0x00, 0x32, 0x02, 0x07, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0xA2, 0x09,
            0x31, 0x07, 0x02, 0x01, 0x01, 0x02, 0x02, 0x04, 0x02};
        nAsnModel = sizeof(data);
        memcpy(abAsn1Model, data, nAsnModel);
    } else if (bDeviceType == DEVICE_CNTAX) {
        //  "cn"\"国家税务总局"\"税务电子证书管理中心(SM2)"
        //
        // cntax-web-example(bNeedType=1):3081890201033058310b300906035504061302434e311b3019060355040b0c12e59bbde5aeb6e7a88ee58aa1e680bbe5b180312c302a06035504030c23e7a88ee58aa1e794b5e5ad90e8af81e4b9a6e7aea1e79086e4b8ade5bf8328534d322902071403000001ec79a0160414c278c6622b6de4cc24ed9308dea22fee761545d5a209310702011002020402
        // cntax-classical-example(bNeedType=0):30710201033058310b300906035504061302434e311b3019060355040b0c12e59bbde5aeb6e7a88ee58aa1e680bbe5b180312c302a06035504030c23e7a88ee58aa1e794b5e5ad90e8af81e4b9a6e7aea1e79086e4b8ade5bf8328534d322902071403000001ec79a209310702011002020402
        unsigned char data[115] = {
            0x30, 0x71, 0x02, 0x01, 0x03, 0x30, 0x58, 0x31, 0x0B, 0x30, 0x09, 0x06, 0x03,
            0x55, 0x04, 0x06, 0x13, 0x02, 0x43, 0x4E, 0x31, 0x1B, 0x30, 0x19, 0x06, 0x03,
            0x55, 0x04, 0x0B, 0x0C, 0x12, 0xE5, 0x9B, 0xBD, 0xE5, 0xAE, 0xB6, 0xE7, 0xA8,
            0x8E, 0xE5, 0x8A, 0xA1, 0xE6, 0x80, 0xBB, 0xE5, 0xB1, 0x80, 0x31, 0x2C, 0x30,
            0x2A, 0x06, 0x03, 0x55, 0x04, 0x03, 0x0C, 0x23, 0xE7, 0xA8, 0x8E, 0xE5, 0x8A,
            0xA1, 0xE7, 0x94, 0xB5, 0xE5, 0xAD, 0x90, 0xE8, 0xAF, 0x81, 0xE4, 0xB9, 0xA6,
            0xE7, 0xAE, 0xA1, 0xE7, 0x90, 0x86, 0xE4, 0xB8, 0xAD, 0xE5, 0xBF, 0x83, 0x28,
            0x53, 0x4D, 0x32, 0x29, 0x02, 0x07, 0x14, 0x03, 0x00, 0x00, 0x01, 0xEC, 0x79,
            0xA2, 0x09, 0x31, 0x07, 0x02, 0x01, 0x10, 0x02, 0x02, 0x04, 0x02};
        nAsnModel = sizeof(data);
        memcpy(abAsn1Model, data, nAsnModel);
    }
    const uint8 *pabAsn1Model = abAsn1Model;
    ASN1_SEQUENCE_ANY *skAsnTypeMy = sk_ASN1_TYPE_new_null();
    ASN1_SEQUENCE_ANY *skAsnType = d2i_ASN1_SEQUENCE_ANY(NULL, &pabAsn1Model, nAsnModel);
    int nskAsnTypeNum = sk_ASN1_TYPE_num(skAsnType);
    int i = 0;
    for (i = 0; i < nskAsnTypeNum; i++) {
        bool bNeedDefaultPushItem = true;
        ASN1_TYPE *asnTypeItem = sk_ASN1_TYPE_value(skAsnType, i);
        do {
            int type = ASN1_TYPE_get(asnTypeItem);
            if (V_ASN1_INTEGER != type)
                break;
            ASN1_INTEGER *asn1Int = asnTypeItem->value.integer;
            if (asn1Int->length != 7)
                break;
            bNeedDefaultPushItem = false;
            //--用设备自身sn替换模板中的sn
            // {0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11};
            memcpy(asn1Int->data, pIn7BytesCertSN, asn1Int->length);
            sk_ASN1_TYPE_push(skAsnTypeMy, asnTypeItem);
            //--只有web接口（电子税务局接口），才需要该字段
            //针对web接口，官方必须跟随在序列号后面(push到后面)，额外添加了sha1随机数,2+2+0x14字节，后续0x14字节需要靠自行填空
            if (pInRandomSHA1OptionalNode[0] != 0x00) {
                // {0xa0, 0x16, 0x04, 0x14, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11,
                // 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11}
                const uint8 *pabPkcs7Sha1HashFormatModel = (const uint8 *)pInRandomSHA1OptionalNode;
                ASN1_TYPE *at = d2i_ASN1_TYPE(NULL, &pabPkcs7Sha1HashFormatModel, 24);
                sk_ASN1_TYPE_push(skAsnTypeMy, at);
            }
        } while (false);
        if (bNeedDefaultPushItem)
            sk_ASN1_TYPE_push(skAsnTypeMy, asnTypeItem);
    }
    uint8 *pOutSeq = NULL;
    int nOutSeq = i2d_ASN1_SEQUENCE_ANY(skAsnTypeMy, &pOutSeq);
    Byte2Str(pOutClientHelloString, pOutSeq, nOutSeq);
    // clear
    sk_ASN1_TYPE_free(skAsnType);
    OPENSSL_free(pOutSeq);
    sk_ASN1_TYPE_pop_free(skAsnTypeMy, ASN1_TYPE_free);
}

// bNeedType,0为传统税务总局发票上传接口\发票确认平台接口，1为电子税务局接口(多出24字节SHA1 Hash)
// szOutBuf缓冲区长度最少需要768字节;
int GetTaClientHello(uint8 bNeedType, HDEV hDev, char *szOutBuf)
{
    uint8 abCertSN[32] = {0}, abRandomSHA1OptionalNode[64] = {0};
    if (GetCertSNByteOrStr(hDev->pX509Cert, (char *)abCertSN, 1) < 0)
        return ERR_DEVICE_CERT_CHECK;
    if (bNeedType == 1) {
        memcpy(abRandomSHA1OptionalNode, "\xa0\x16\x04\x14", 4);
        srand((unsigned int)time(NULL));
        uint32 uRandomNum = rand() % 2048;
        CalcSHA1(&uRandomNum, 4, abRandomSHA1OptionalNode + 4);
    }
    BuildClientHelloString(hDev->bDeviceType, abCertSN, abRandomSHA1OptionalNode, szOutBuf);
    return RET_SUCCESS;
}