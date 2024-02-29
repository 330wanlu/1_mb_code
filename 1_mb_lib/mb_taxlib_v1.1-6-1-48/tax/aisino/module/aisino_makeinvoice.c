/*****************************************************************************
File name:   aisino_makeinvoice.c
Description: aisno开票相关代码
Author:      Zako
Version:     1.0
Date:        2019.10
History:
20200901     与buildinv.c文件功能相近，合并
20220222     开票软件220107版本对齐，主要是开票字符串的对齐，引入#$`^分割字符串；
             较大规模对此前代码开票字符串部分逻辑比较复杂冗余的部分进行切割分离和精简逻辑
20220414     对20220401版本新版本中新加入的RSA公钥加密跟进,规范化发票核对要素点
*****************************************************************************/
// 一张发票的关键因素有如下几点，需要逐个字节核对，错任何一个字节都会导致问题，具体可以跟踪'aisino开票核对要素'字样
#define make_invoice_c
#include "aisino_makeinvoice.h"

//税率结构体
struct st_Slv {
    struct st_Slv *stp_next;
    char ca_slv[10];   //税率
    char ca_zje[100];  //金额
    char ca_zse[100];  //总税额
};

//插入税率结点
int InsertDict(struct st_Slv *pst_root, char *cp_slv, char *cp_je, char *cp_se)
{
    char ca_slv[10] = {0};
    if (pst_root == NULL || cp_slv == NULL || cp_je == NULL)
        return 1;
    struct st_Slv *stp_Tmp = pst_root, *stp_InsertNode = calloc(1, sizeof(struct st_Slv));
    if (stp_InsertNode == 0)
        return 1;
    //税率转换
    double d_slv = atof(cp_slv);
    if (d_slv != 0) {  //移除税率结尾0和00，例如0.130,0.100
        sprintf(ca_slv, "%.3f", d_slv);
        MemoryFormart(MF_TRANSLATE_TAILZERO, ca_slv, strlen(ca_slv));
    } else  //特殊零税率保留最后两位小数
        strcpy(ca_slv, "0.00");
    //三要素填入
    memset(stp_InsertNode, 0, sizeof(struct st_Slv));
    memcpy(stp_InsertNode->ca_slv, ca_slv, strlen(ca_slv));
    memcpy(stp_InsertNode->ca_zje, cp_je, strlen(cp_je));
    memcpy(stp_InsertNode->ca_zse, cp_se, strlen(cp_se));
    while (stp_Tmp->stp_next) {
        stp_Tmp = stp_Tmp->stp_next;
    }
    stp_Tmp->stp_next = stp_InsertNode;
    return 0;
}

//添加已有税率
int AddDict(struct st_Slv *pst_root, char *cp_slv, char *pc_je, char *pc_se)
{
    double d_je = 0, d_zje = 0, d_se = 0, d_zse = 0;
    if (pst_root == NULL || cp_slv == NULL || pc_je == NULL)
        return 1;
    //跳过头结点
    struct st_Slv *stp_Tmp = pst_root->stp_next;
    while (stp_Tmp) {
        if (strcmp(stp_Tmp->ca_slv, cp_slv) == 0 || atof(stp_Tmp->ca_slv) == atof(cp_slv)) {
            //总金额
            d_je = atof(pc_je);
            d_zje = atof(stp_Tmp->ca_zje);
            d_zje += d_je;
            sprintf(stp_Tmp->ca_zje, "%.2f", d_zje);
            //总税额
            d_se = atof(pc_se);
            d_zse = atof(stp_Tmp->ca_zse);
            d_zse += d_se;
            sprintf(stp_Tmp->ca_zse, "%.2f", d_zse);
            return 0;
        }
        stp_Tmp = stp_Tmp->stp_next;
    }
    return 1;
}

//判断税率是否已经存在
int JudgeSlvExist(struct st_Slv *pst_root, uint8 *cp_slv)
{
    struct st_Slv *stp_Tmp = pst_root->stp_next;
    while (stp_Tmp) {
        if (strcmp(stp_Tmp->ca_slv, (const char *)cp_slv) == 0 ||
            atof(stp_Tmp->ca_slv) == atof((const char *)cp_slv)) {
            return 0;
        }
        stp_Tmp = stp_Tmp->stp_next;
    }
    return 1;
}

int FreeSlvDict(struct st_Slv *pst_root)
{
    struct st_Slv *stp_Tmp = pst_root, *stp_DelNode = NULL;
    while (stp_Tmp) {
        stp_DelNode = stp_Tmp;
        stp_Tmp = stp_Tmp->stp_next;
        free(stp_DelNode);
    }
    return 0;
}

//此处已经相当底层的数据拼凑，因此尽量不要进行数据校验和调整，如需操作请在import.c中进行
char *FillSPXX(HFPXX fpxx, struct Spxx *stp_Spxx, int nInvoiceNum)
{
    struct STSPXXv3 *spxx = (struct STSPXXv3 *)malloc(sizeof(struct STSPXXv3));
    char ca_spmc[256] = {0};
    memset(spxx, 0x20, sizeof(struct STSPXXv3));
    strcat(ca_spmc, stp_Spxx->spsmmc);
    strcat(ca_spmc, stp_Spxx->spmc);
    //商品名称
    UTF8CopyGBK((uint8 *)spxx->SPMC, (uint8 *)ca_spmc, strlen(ca_spmc), sizeof(spxx->SPMC));
    //规格型号
    UTF8CopyGBK((uint8 *)spxx->GGXH, (uint8 *)stp_Spxx->ggxh, strlen(stp_Spxx->ggxh),
                sizeof(spxx->GGXH));
    //计量单位
    UTF8CopyGBK((uint8 *)spxx->JLDW, (uint8 *)stp_Spxx->jldw, strlen(stp_Spxx->jldw),
                sizeof(spxx->JLDW));
    //数量
    UTF8CopyGBK((uint8 *)spxx->SL, (uint8 *)stp_Spxx->sl, strlen(stp_Spxx->sl), sizeof(spxx->SL));
    //单价
    UTF8CopyGBK((uint8 *)spxx->DJ, (uint8 *)stp_Spxx->dj, strlen(stp_Spxx->dj), sizeof(spxx->DJ));
    //金额
    UTF8CopyGBK((uint8 *)spxx->JE, (uint8 *)stp_Spxx->je, strlen(stp_Spxx->je), sizeof(spxx->JE));
    //税率
    UTF8CopyGBK((uint8 *)spxx->SLV, (uint8 *)stp_Spxx->slv, strlen(stp_Spxx->slv),
                sizeof(spxx->SLV));
    //税额
    UTF8CopyGBK((uint8 *)spxx->SE, (uint8 *)stp_Spxx->se, strlen(stp_Spxx->se), sizeof(spxx->SE));
    //序号
    char szTmp[16];
    sprintf(szTmp, "%d", nInvoiceNum);
    UTF8CopyGBK((uint8 *)spxx->NUM, (uint8 *)szTmp, strlen(szTmp), sizeof(spxx->NUM));
    // FPHXZ 发票行性质 import.c GetAvailImportData已判断fphxz参数，不可能为空
    UTF8CopyGBK((uint8 *)spxx->FPHXZ, (uint8 *)stp_Spxx->fphxz, strlen(stp_Spxx->fphxz),
                sizeof(spxx->FPHXZ));
    //含税价标志    
	if (strlen(stp_Spxx->hsjbz) == 0 || !strcmp(stp_Spxx->hsjbz, "0"))
		strcpy(stp_Spxx->hsjbz, "0");
	else
		strcpy(stp_Spxx->hsjbz, "1");
	// 20220908最新测试结果
	// 电子发票免税开具时，含税价标志为1,专普票为0,无论在开票界面点击含税开具与否，此处标志永远固定
	if ((!strcmp(stp_Spxx->se, "0.00") &&
		IsAisino3Device(fpxx->hDev->nDriverVersionNum))) {
		//旧标准3%按照免税发票开具（零税率发票待测试）
		if (fpxx->fplx_aisino == FPLX_AISINO_DZFP)
			strcpy(stp_Spxx->hsjbz, "1");
		else
			strcpy(stp_Spxx->hsjbz, "0");
	}
    UTF8CopyGBK((uint8 *)spxx->HSJBZ, (uint8 *)stp_Spxx->hsjbz, strlen(stp_Spxx->hsjbz),
                sizeof(spxx->HSJBZ));
    //商品税目(spsm) 分类编码(flbm) 商品编号(spbh);此处开票字符串分类编码为协议中商品编号
    UTF8CopyGBK((uint8 *)spxx->FLBM, (uint8 *)stp_Spxx->spbh, strlen(stp_Spxx->spbh),
                sizeof(spxx->FLBM));
    //商品编号(此处商品编号填空,目前废弃)
    UTF8CopyGBK((uint8 *)spxx->SPBH, (uint8 *)"", 0, sizeof(spxx->SPBH));
    //税率优惠
    UTF8CopyGBK((uint8 *)spxx->XSYH, (uint8 *)stp_Spxx->xsyh, strlen(stp_Spxx->xsyh),
                sizeof(spxx->XSYH));
    //优惠说明
    UTF8CopyGBK((uint8 *)spxx->YHSM, (uint8 *)stp_Spxx->yhsm, strlen(stp_Spxx->yhsm),
                sizeof(spxx->YHSM));
    //零税率标识
    UTF8CopyGBK((uint8 *)spxx->LSLVBS, (uint8 *)stp_Spxx->lslvbs, strlen(stp_Spxx->lslvbs),
                sizeof(spxx->LSLVBS));
    return (char *)spxx;
}

struct st_Slv *GetTaxRateInfoFromString(HFPXX fpxx)
{
    struct st_Slv *stp_Slvzje = NULL;
    if ((stp_Slvzje = calloc(1, sizeof(struct st_Slv))) == NULL)
        return NULL;
    //发票明细中按税率划分总金额、总税额
    struct Spxx *stp_TmpSpxx = fpxx->stp_MxxxHead->stp_next;
    while (stp_TmpSpxx) {
        if (atoi(stp_TmpSpxx->fphxz) != FPHXZ_AISINO_XHQDZK) {
            if (JudgeSlvExist(stp_Slvzje, (uint8 *)stp_TmpSpxx->slv) == 0) {
                if (AddDict(stp_Slvzje, stp_TmpSpxx->slv, stp_TmpSpxx->je, stp_TmpSpxx->se) != 0) {
                    FreeSlvDict(stp_Slvzje);
                    stp_Slvzje = NULL;
                    break;
                }
            } else {
                if (InsertDict(stp_Slvzje, stp_TmpSpxx->slv, stp_TmpSpxx->je, stp_TmpSpxx->se) !=
                    0) {
                    FreeSlvDict(stp_Slvzje);
                    stp_Slvzje = NULL;
                    break;
                }
            }
        }
        stp_TmpSpxx = stp_TmpSpxx->stp_next;
    }
    return stp_Slvzje;
}

int GetKpstrPriceTaxRate(HFPXX fpxx, char *szBuf, int nBufSize)
{
    memset(szBuf, 0, nBufSize);
    struct st_Slv *stp_Slvzje = GetTaxRateInfoFromString(fpxx);
    if (!stp_Slvzje)
        return ERR_BUF_ALLOC;
    ///开票软件差异，目前测试所有票种皆存在，电子发票税率为0.01时，开票软件为0.01\0.06，我方为0.010\0.060
    //问题不严重且较容易测试复现，交由蒙柏方线上测试修改
    struct st_Slv *stp_slvTmp = stp_Slvzje->stp_next;
    while (stp_slvTmp) {
        strcat((char *)szBuf, stp_slvTmp->ca_slv);
        strcat((char *)szBuf, ",");
        strcat((char *)szBuf, stp_slvTmp->ca_zje);
        strcat((char *)szBuf, ",");
        strcat((char *)szBuf, stp_slvTmp->ca_zse);
        strcat((char *)szBuf, ";");
        stp_slvTmp = stp_slvTmp->stp_next;
    }
    //--如果时红字清单发票或者多税率发票重设税率
    if ((fpxx->isRedDetailList == 1) && (fpxx->isMultiTax == 1)) {
        memset(szBuf, 0, nBufSize);
        strcat((char *)szBuf, ",");
        strcat((char *)szBuf, fpxx->stp_MxxxHead->stp_next->je);
        strcat((char *)szBuf, ",");
        strcat((char *)szBuf, fpxx->stp_MxxxHead->stp_next->se);
        strcat((char *)szBuf, ";");
    }
    if (stp_Slvzje)
        FreeSlvDict(stp_Slvzje);
    return RET_SUCCESS;
}

//同时包含分类编码版本信息
int GetKpstrSpecialInvoiceFlagString(HFPXX fpxx, char *szBuf, int nBufSize)
{
    uint8 bZyfpFlag = 0;
    bool bIsMajorInvoiceType = false;
    strcpy(szBuf, "V1");  // default
    if ((fpxx->fplx_aisino == FPLX_AISINO_ZYFP) && (fpxx->zyfpLx == ZYFP_XT_YCL)) {
        bZyfpFlag = 1;
    } else if ((fpxx->fplx_aisino == FPLX_AISINO_ZYFP) && (fpxx->zyfpLx == ZYFP_XT_CCP)) {
        bZyfpFlag = 2;
    } else if ((fpxx->fplx_aisino == FPLX_AISINO_ZYFP) &&
               (fpxx->zyfpLx == ZYFP_SNY || fpxx->zyfpLx == ZYFP_SNY_DDZG ||
                fpxx->zyfpLx == ZYFP_RLY || fpxx->zyfpLx == ZYFP_RLY_DDZG)) {
        bZyfpFlag = 3;
    }
    if (bZyfpFlag != 0) {
        memset(szBuf, 0, nBufSize);
        sprintf((char *)szBuf, "V%d", bZyfpFlag + 1);
    }
    if (fpxx->zyfpLx == ZYFP_NCP_XS) {
        memset(szBuf, 0, nBufSize);
        strcat((char *)szBuf, "V5");
    } else if (fpxx->zyfpLx == ZYFP_NCP_SG) {
        memset(szBuf, 0, nBufSize);
        strcat((char *)szBuf, "V6");
    }
    //专用发票_成品油
    if (fpxx->zyfpLx == ZYFP_CPY) {
        //此处1(成品油)还有一个判断未知
        if ((1) && (fpxx->fplx_aisino == FPLX_AISINO_ZYFP) && (fpxx->isRed)) {
            memset(szBuf, 0, nBufSize);
            strcat((char *)szBuf, "VA");
        } else {
            memset(szBuf, 0, nBufSize);
            strcat((char *)szBuf, "V9");
        }
    }
    //专用发票_稀土
    if (fpxx->fplx_aisino == FPLX_AISINO_ZYFP && fpxx->zyfpLx == ZYFP_XT_JSJHJ) {
        memset(szBuf, 0, nBufSize);
        strcat((char *)szBuf, "VE");
    } else if (fpxx->fplx_aisino == FPLX_AISINO_ZYFP && fpxx->zyfpLx == ZYFP_XT_CPJGF) {
        memset(szBuf, 0, nBufSize);
        strcat((char *)szBuf, "VF");
    }
    //如果启用FLBM则加入版本号(实际已全面上线)
    //空白作废无编码表版本号
    if (fpxx->fpzt != FPSTATUS_BLANK_WASTE) {
        strcat((char *)szBuf, "B");
        strcat((char *)szBuf, fpxx->bmbbbh);
    }
    if (fpxx->fplx_aisino == FPLX_AISINO_ZYFP || fpxx->fplx_aisino == FPLX_AISINO_PTFP ||
        fpxx->fplx_aisino == FPLX_AISINO_DZFP || fpxx->fplx_aisino == FPLX_AISINO_DZZP)
        bIsMajorInvoiceType = true;
    if (bIsMajorInvoiceType && fpxx->zyfpLx != ZYFP_HYSY && fpxx->zyfpLx != ZYFP_JYZS)
        strcat((char *)szBuf, "H");
    if (bIsMajorInvoiceType && fpxx->zyfpLx == ZYFP_JYZS)
        strcat((char *)szBuf, "J");
    if (bIsMajorInvoiceType && fpxx->zyfpLx == ZYFP_CEZS)
        strcat((char *)szBuf, "C");
    if (!strcmp(fpxx->yddk, "1"))
        strcat((char *)szBuf, "YD");
    if (!strcmp(fpxx->dkbdbs, "1"))
        strcat((char *)szBuf, "YS");
    return RET_SUCCESS;
}

int GetGbkSpxxString(HFPXX fpxx, char *pGBKOutBuff, int nGBKOutBuffIndex)
{
    //小容量盘64K，大容量盘0x400K；剩余应该根据最新的结构体大小计算;但是税局201907最新要求行数2000
    int num5 = 0x400000, nMaxInvoiceCount = 0, i_InvoiceNum = 1;
    struct Spxx *stp_Spxx = NULL;
    nMaxInvoiceCount = (num5 - nGBKOutBuffIndex) / sizeof(struct STSPXXv3);
    //是否有清单
    if (strcmp(fpxx->qdbj, "Y") == 0) {
        //添加商品明细头，stp_Spxx不会为NULL
        if (!(stp_Spxx = fpxx->stp_MxxxHead))
            return ERR_BUF_CHECK;
        while (stp_Spxx && (i_InvoiceNum < nMaxInvoiceCount)) {
            int nIndex = i_InvoiceNum - 1;
            if (1 == i_InvoiceNum)
                nIndex = 1;  //有个表头
            char *p = FillSPXX(fpxx, stp_Spxx, nIndex);
            memcpy(pGBKOutBuff + nGBKOutBuffIndex, p, sizeof(struct STSPXXv3));
            nGBKOutBuffIndex += sizeof(struct STSPXXv3);
            if (1 == i_InvoiceNum) {
                pGBKOutBuff[nGBKOutBuffIndex] = '\n';
                nGBKOutBuffIndex += 1;
            }
            free(p);
            stp_Spxx = stp_Spxx->stp_next;
            i_InvoiceNum++;
        }
    } else {
        if (!(stp_Spxx = fpxx->stp_MxxxHead->stp_next))
            return ERR_BUF_CHECK;
        while (stp_Spxx && (i_InvoiceNum < nMaxInvoiceCount)) {
            char *p = FillSPXX(fpxx, stp_Spxx, i_InvoiceNum);
            memcpy(pGBKOutBuff + nGBKOutBuffIndex, p, sizeof(struct STSPXXv3));
            nGBKOutBuffIndex += sizeof(struct STSPXXv3);
            free(p);
            stp_Spxx = stp_Spxx->stp_next;
            i_InvoiceNum++;
        }
    }
    return nGBKOutBuffIndex;
}

// 20220401aisino新加入算法,执行成功后dstBuf返回为二进制0x80长度数据
int RsaEncryptWithStaticPubkey(uint8 *pSrcBuf, int nSrcSize, uint8 *pDstBuf, int nDstSize)
{
    // Note:调试此段代码，必须熟悉RSA非对称加密详细过程及加密过程中内存布局方式和最终内存变化位置
    // 1)开票软件输入格式
    //开票软件此处是一个RSA1024公钥加密过程，公钥为静态公钥，格式为XML公钥格式类似于如下格式，可以直接利用RSA证书转换XML->PEM格式转换为openssl
    // pem通用格式 <RSAKeyValue><Modulus>xA7S...</Modulus><Exponent>AQAB</Exponent>/RSAKeyValue>
    // openssl rsautl -encrypt -in input.txt -inkey pubkey.key -pubin -out hello.en
    // 2)RSA+PKCS1填充算法判断理由
    //对开票软件加密长度进行修改，当修改长度至117字节时，程序c++异常，128-117=11字节符合，与PKCS#1建议的padding就占用了11个字节对应
    //因此利用pkcs#1进行填充
    //加密时最大字节数：证书位数/8-11（2048位的证书，支持的最大加密字节数2048/8-11=245
    //例如:.NET中的RSA加密算法为了提高安全性，在待加密数据前要添加一些随机数，因此，使用.NET中的RSA加密算法一次最多加密117字节数据（多于117字节需要拆分成多段分别加密再连接起来），经过加密后得到一个长度为128字节的加密数据。
    // 3)填充RSA规范
    //填充块类型(BlockType)为02，使用伪随机数对前两个字节进行填充，所以相同输入，每次加密结果都会不一样。
    //在私钥的情况下，通常服务端性能考虑，用固定字节填充BT00/01。公钥用于客户端，用伪随机数填充BT02。
    //参考:https://www.jianshu.com/p/e300f7735c87
    // 4)由于只有公钥，无法解开加密后的数据，无法直接验证加密数据正确性，因此采用固定随机填充，测试我方加密数据的测试办法
    //深入分析后官方疑为使用crypto++加密库老版本（C++版本算法库），pad算法PKCS_EncryptionPaddingScheme::Pad
    // openssl使用pad算法为 RSA_padding_add_PKCS1_type_2
    //在openssl源码中RSA_padding_add_PKCS1_type_2，memcpy之后利用gdb修改内存对随机数进行填充后进行测试
    /// tassl111bd/src/crypto/rsa/rsa_pk1.c：117行附近 RSA_padding_add_PKCS1_type_2函数结尾处
    //-exec set {char[85]}(p-85) =
    //"000000000000000000000000000000000000000000000000000000000000000000000000000000000000"

    //税局给定的静态文本公钥证书
    char szPubKey[] = "-----BEGIN PUBLIC KEY-----\n"
                      "MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQCvhgGyjmdmY1pjylEkVOFJB0+v\n"
                      "WbnhWy/+FQOBFJYsBwYV+gBXfeaHx9KOHXKn2s4uxZ0cPbMHvAvg51p6aRqGxpWj\n"
                      "E+JrisQaVQTcatyHMAPHgeiY61ItsiZfXgOsIOr8kh/NgSB+A0YE+QFpEuXgXuO+\n"
                      "EDhHsc55dG9NJKE8xQIDAQAB\n"
                      "-----END PUBLIC KEY-----";
    RSA *rsa = NULL;
    BIO *bufio = NULL;
    int ret = ERR_GENERIC;
    // rsa1024 不分组最大加密大小117字节,输出缓冲不得低于0x80字节
    if (nSrcSize > 117 || nDstSize < 0x80)
        return false;
    do {
        if (!(bufio = BIO_new_mem_buf((void *)szPubKey, -1)))
            break;
        if (!(rsa = PEM_read_bio_RSA_PUBKEY(bufio, &rsa, NULL, NULL)))
            break;
        if (RSA_public_encrypt(nSrcSize, pSrcBuf, pDstBuf, rsa, RSA_PKCS1_PADDING) < 0)
            break;
        //生成长度=0x80字节
        ret = 0x80;
    } while (false);
    if (bufio)
        BIO_free_all(bufio);
    if (rsa)
        RSA_free(rsa);
    return ret;
}

// szOutBuf长度不低于256字节
bool GetRsaEncrypInvoiceInformationBase64(HFPXX fpxx, char *szOutBuf)
{
    //[aisino开票核对要素2]
    char szBuf[512] = {0};
#ifdef _NODEVICE_TEST
    char szKpStr[2048] = "";
    if (_ReadHexFromDebugFile("kpstr_aisino_kprj.txt", (uint8 *)szKpStr, sizeof szKpStr) < 0)
        return false;
    //我们需要的数据中不包含中文字符，所以就不需要转换编码了
    char *arrays[32] = {NULL};
    int nLines = SplitInvoiceDetail((uint8 *)szKpStr, (uint8 **)arrays);
    if (nLines < 23) {
        Free2dArray((uint8 **)arrays, 32);
        return false;
    }
    char *pAisino3SplitFlag = strstr(arrays[17], defAisino3SplitFlag);
    if (!pAisino3SplitFlag) {
        Free2dArray((uint8 **)arrays, 32);
        return false;
    }
    pAisino3SplitFlag += strlen(defAisino3SplitFlag);
    memmove(szBuf, pAisino3SplitFlag, strlen(arrays[17]) - (pAisino3SplitFlag - arrays[17]));
    strcpy(szOutBuf, szBuf);
#else
    sprintf(szBuf, "%s@%s@%s", fpxx->fpdm, fpxx->fphm, defTaxAuthorityVersionAisino);
    uint8 abDst[256] = "";
    int nChildRet = -1;
    if ((nChildRet =
             RsaEncryptWithStaticPubkey((uint8 *)szBuf, strlen(szBuf), abDst, sizeof abDst)) > 0) {
        memset(szBuf, 0, sizeof(szBuf));
        Base64_Encode((const char *)abDst, nChildRet, szBuf);
        strcpy(szOutBuf, szBuf);
    }
#endif
    return true;
}

//返回GBK编码开票数据
char *GetInvString(HFPXX fpxx)
{
    int nRet = -1;
    char szBuf[512] = {0}, szNewlineSep[] = "\n";
    int nInvBuffLen = CalcAllocBuffLen(fpxx);
    if (fpxx == NULL)
        return NULL;
    if ((fpxx->fplx_aisino != FPLX_AISINO_ZYFP) && (fpxx->fplx_aisino != FPLX_AISINO_PTFP) &&
        (fpxx->fplx_aisino != FPLX_AISINO_DZFP)) {
        _WriteLog(LL_FATAL, "Unsupport invoicetype");
        return NULL;
    }
    char *pUTF8OutBuff = calloc(1, nInvBuffLen);
    if (!pUTF8OutBuff)
        return NULL;
    char *pGBKOutBuff = calloc(1, nInvBuffLen);
    if (!pGBKOutBuff) {
        free(pUTF8OutBuff);
        return NULL;
    }
    //发票类型
    char szStrFPLX[16] = "";
    AisinoFPLX2Str(fpxx->fplx_aisino, szStrFPLX);
    strcat(pUTF8OutBuff, szStrFPLX);
    strcat(pUTF8OutBuff, szNewlineSep);
    //发票代码+专用发票标识+分类编码版本信息
    strcat(pUTF8OutBuff, fpxx->fpdm);
    GetKpstrSpecialInvoiceFlagString(fpxx, szBuf, sizeof(szBuf));
    strcat(pUTF8OutBuff, szBuf);
    strcat(pUTF8OutBuff, szNewlineSep);
    //发票号码
    pUTF8OutBuff = strcat(pUTF8OutBuff, fpxx->fphm);
    //金额、税率
    if (GetKpstrPriceTaxRate(fpxx, szBuf, sizeof(szBuf)) < 0)
        goto GetInvStr_Finish;
    if (strlen(szBuf)) {
        strcat(pUTF8OutBuff, "V");
        strcat(pUTF8OutBuff, szBuf);
    }
    pUTF8OutBuff = strcat(pUTF8OutBuff, szNewlineSep);
    //日期
    strcat(pUTF8OutBuff, fpxx->kpsj_F1);
    strcat(pUTF8OutBuff, "@");
    //软件版本号
    strcat(pUTF8OutBuff, fpxx->kprjbbh);
    strcat(pUTF8OutBuff, "@");
    //税盘硬件版本号
    strcat(pUTF8OutBuff, fpxx->driverVersion);
    strcat(pUTF8OutBuff, szNewlineSep);
    //购方名称
    strcat(pUTF8OutBuff, fpxx->gfmc);
    strcat(pUTF8OutBuff, szNewlineSep);
    //购方税号
    strcat(pUTF8OutBuff, fpxx->gfsh);
    strcat(pUTF8OutBuff, szNewlineSep);
    //购方地址电话
    strcat(pUTF8OutBuff, fpxx->gfdzdh);
    strcat(pUTF8OutBuff, szNewlineSep);
    //购方银行账号
    strcat(pUTF8OutBuff, fpxx->gfyhzh);
    strcat(pUTF8OutBuff, szNewlineSep);
    //加密版本号+机器编号+公共服务平台
    if (fpxx->fplx_aisino == FPLX_AISINO_PTFP || fpxx->fplx_aisino == FPLX_AISINO_ZYFP) {
        strcat(pUTF8OutBuff, fpxx->jmbbh);
    } else {
        // DZFP DZZP JSFP
        strcat(pUTF8OutBuff, fpxx->jqbh);
        strcat(pUTF8OutBuff, "@");
        if (strlen(fpxx->hDev->szPubServer))
            strcat(pUTF8OutBuff, fpxx->pubcode);  // ptlx;ptxh;pthm
        else
            strcat(pUTF8OutBuff, ";;");
    }
    if (IsAisino3Device(fpxx->hDev->nDriverVersionNum)) {
        //个体户1% %3减免税开具理由,++todo(专票1% 3%待测试，结果应该是不享受政策，强制为0)
        strcat(pUTF8OutBuff, defAisino3SplitFlag);
            if (!strcmp(fpxx->slv, "0.01"))
            {   
                //fpxx->sslkjly = 0;
                strcpy(szBuf,"0");
            }
            else if(!strcmp(fpxx->slv, "0.03"))
            {
                if(fpxx->sslkjly != 2 && fpxx->sslkjly != 3)
                    strcpy(szBuf,"2");
                else
                    sprintf(szBuf, "%d", fpxx->sslkjly);

                if (fpxx->fplx == 7 || fpxx->fplx == 26)
                {
                    //fpxx->sslkjly = 2;
                    strcpy(szBuf,"2");
                }
                              
            }
        strcat(pUTF8OutBuff, szBuf);
    }
    strcat(pUTF8OutBuff, szNewlineSep);
    //主要商品名称
    strcat(pUTF8OutBuff, fpxx->zyspsmmc);
    strcat(pUTF8OutBuff, fpxx->zyspmc);
    strcat(pUTF8OutBuff, szNewlineSep);
    //报税期标记，该值错误会导致月底汇总错误
    strcat(pUTF8OutBuff, fpxx->szPeriodCount);
    memset(szBuf, 0, sizeof(szBuf));
    if ((fpxx->fplx_aisino == FPLX_AISINO_ZYFP || fpxx->fplx_aisino == FPLX_AISINO_DZZP) &&
        fpxx->isRed) {
        if (strlen(fpxx->blueFpdm) && strlen(fpxx->blueFphm)) {
            //常规专票销方红票-销售方申请红字信息表由销售方开具
            sprintf((char *)szBuf, "@%s@%08d", fpxx->blueFpdm, atoi(fpxx->blueFphm));
            strcat(pUTF8OutBuff, (char *)szBuf);
        } else {
            //购方申请红字信息表由销方开具，此时开票字符串中间无原销方开具的蓝字发票代码号码
        }
    }
    if (IsAisino3Device(fpxx->hDev->nDriverVersionNum)) {
        // XSDJBH(销售单据编号?)，未知，待测试
        strcat(pUTF8OutBuff, defAisino3SplitFlag);
    }
    strcat(pUTF8OutBuff, szNewlineSep);
    //销方名称
    strcat(pUTF8OutBuff, fpxx->xfmc);
    strcat(pUTF8OutBuff, szNewlineSep);
    //销方税号
    strcat(pUTF8OutBuff, fpxx->xfsh);
    strcat(pUTF8OutBuff, szNewlineSep);
    //销方地址电话
    strcat(pUTF8OutBuff, fpxx->xfdzdh);
    strcat(pUTF8OutBuff, szNewlineSep);
    //销方银行账号
    strcat(pUTF8OutBuff, fpxx->xfyhzh);
    strcat(pUTF8OutBuff, szNewlineSep);
    //开票人
    strcat(pUTF8OutBuff, fpxx->kpr);
    strcat(pUTF8OutBuff, szNewlineSep);
    //复核人
    strcat(pUTF8OutBuff, fpxx->fhr);
    if (IsAisino3Device(fpxx->hDev->nDriverVersionNum)) {
        // 复核人名称[defAisino3SplitFlag]026;20220121;2
        // LZFPZL(蓝字发票种类),LZFPKJSJ（蓝字发票开具时间），KJXXFSLY应该是红字发票必填选项
        // KJXXFSLY=1销货退回 2开票有误 3服务中止 4销售折让
        strcat(pUTF8OutBuff, defAisino3SplitFlag);
        if (fpxx->isRed && fpxx->blueFplx != FPLX_COMMON_DZZP &&
            fpxx->blueFplx != FPLX_COMMON_ZYFP && strlen(fpxx->blueKpsj) == 14)
            //注意！此处发票类型为通用类型，而非aisino类型；此处理由永远是 2开票有误
            //普票和电票的红字发票才有
            sprintf(szBuf, "%03d;%.8s;%d", fpxx->blueFplx, fpxx->blueKpsj, fpxx->hcyy);
        else
            strcpy(szBuf, ";;");
        strcat(pUTF8OutBuff, szBuf);
    }
    strcat(pUTF8OutBuff, szNewlineSep);
    //收款人
    strcat(pUTF8OutBuff, fpxx->skr);
    if (IsAisino3Device(fpxx->hDev->nDriverVersionNum)) {
        strcat(pUTF8OutBuff, defAisino3SplitFlag);
        if (!GetRsaEncrypInvoiceInformationBase64(fpxx, fpxx->kjbbh))
            goto GetInvStr_Finish;
        strcat(pUTF8OutBuff, fpxx->kjbbh);
    }
    strcat(pUTF8OutBuff, szNewlineSep);
    //备注
    memset(szBuf, 0, sizeof(szBuf));
    UTF8ToGBKBase64(fpxx->bz, strlen(fpxx->bz), (char *)szBuf);
    strcat(pUTF8OutBuff, (char *)szBuf);
    strcat(pUTF8OutBuff, szNewlineSep);
    //金额
    strcat(pUTF8OutBuff, fpxx->je);
    strcat(pUTF8OutBuff, szNewlineSep);
    //税额
    strcat(pUTF8OutBuff, fpxx->se);
    strcat(pUTF8OutBuff, szNewlineSep);
    //清单标记
    strcat(pUTF8OutBuff, fpxx->qdbj);
    strcat(pUTF8OutBuff, szNewlineSep);
    //=原UTF8buf转换成GBK继续填充商品信息
    int nGBKOutBuffIndex =
        UTF8CopyGBK((uint8 *)pGBKOutBuff, (uint8 *)pUTF8OutBuff, strlen(pUTF8OutBuff), 0);
    if (nGBKOutBuffIndex < 0)
        goto GetInvStr_Finish;
    if (fpxx->fpzt == FPSTATUS_BLANK_WASTE) {
        //开具作废票即为空白作废，空白作废票无商品信息
        nRet = 0;
        goto GetInvStr_Finish;
    }
    if ((nGBKOutBuffIndex = GetGbkSpxxString(fpxx, pGBKOutBuff, nGBKOutBuffIndex)) < 0)
        goto GetInvStr_Finish;

    nRet = 0;
GetInvStr_Finish:
    free(pUTF8OutBuff);
    if (nRet < 0) {
        free(pGBKOutBuff);
        return NULL;
    }
    //[aisino开票核对要素1]
    _WriteHexToDebugFile("kpstr_aisino_my.txt", (uint8 *)pGBKOutBuff, strlen(pGBKOutBuff));
    return pGBKOutBuff;
}

//需要注意下，这里传入的是通用类型，而不是aisino类型
int GetPeriodCount(HUSB hUSB, uint8 bAisinoFPLX, char *szRet)
{
    uint8 abRaw[8] = {0};
#ifdef _NODEVICE_TEST
    strcpy(szRet, "1");
    return 0;
#endif
    uint8 cbTransfer[16] = {0};
    uint8 cb[16] = {0};
    int ncbLen = sizeof cb;
    memcpy(cb, "\xfe\xff\xcc\xbc\x53\xe1\xe3\x00\x31\x00", 10);
    cbTransfer[0] = bAisinoFPLX;
    cbTransfer[1] = MakePackVerityFlag(cbTransfer, 1);
    if (AisinoLogicIO(hUSB, cb, 10, ncbLen, cbTransfer, 2) < 2)
        return -1;
    abRaw[1] = cb[0];
    memset(cb, 0, sizeof(cb));
    memcpy(cb, "\xfe\xff\xcc\xbc\x53\xe1\xe3\x00\x32\x00", 10);
    if (AisinoLogicIO(hUSB, cb, 10, ncbLen, cbTransfer, 2) < 2)
        return -2;
    abRaw[0] = cb[0];
    sprintf(szRet, "%d", abRaw[1]);
    return 0;
}

void GetVersionInfo(HFPXX fpxx, HDEV hDev)
{
    char szKPRJVersion[24];
    GetAisinoKPRJVersion(hDev->nDriverVersionNum, szKPRJVersion);
    //静态的不会变化的参数
    strcpy(fpxx->jmbbh, hDev->szJMBBH);
    strcpy(fpxx->driverVersion, hDev->szDriverVersion);
    strcpy(fpxx->kprjbbh, szKPRJVersion);
    strcpy(fpxx->taVersion, defTaxAuthorityVersionAisino);
    strcpy(fpxx->bmbbbh, DEF_BMBBBH_DEFAULT);
    //可能跟随时间等因素变化的参数
    GetPeriodCount(hDev->hUSB, fpxx->fplx_aisino, fpxx->szPeriodCount);
}

int AisinoUpdateNodeInfo(HFPXX fpxx, HDEV hDev)
{
    if (hDev->bTrainFPDiskType == 1) {  //江苏测试盘才加上
        //测试盘必须加上，否则开出非测试信息的发票，税局会怀疑？
        strcpy(fpxx->gfsh, "110101000000000");
        strcpy(fpxx->gfmc, "仅供培训使用");
        strcpy(fpxx->gfdzdh, "仅供培训使用");
        strcpy(fpxx->gfyhzh, "仅供培训使用");
        strcpy(fpxx->bz, "仅供培训使用");
    }
    if (fpxx->fplx_aisino == FPLX_AISINO_JDCFP || fpxx->fplx_aisino == FPLX_AISINO_DZFP ||
        fpxx->fplx_aisino == FPLX_AISINO_JSFP || fpxx->fplx_aisino == FPLX_AISINO_ESC) {
        strcpy(fpxx->jqbh, hDev->szDeviceID);
    } else {
        strcpy(fpxx->jqbh, "");
    }
    //更新底层固件信息
    GetVersionInfo(fpxx, hDev);
    return 0;
}

// szRet 一般在6-8k，建议缓冲区最少20k以上
int EncodeInvoiceBlock2(char *pInvStr, int nInvBuffLen, char *szRet)
{
    BYTE *pSysMemBlock = NULL;
    //预留空间30k,实际上远远用不了这么多内存，实际上内存解压是在szRet里面，为了和Test方法同步，就多分配点吧
    pSysMemBlock = calloc(1, nInvBuffLen * 15 + 30000);
    int nLen = InvEncryptBlock2((BYTE *)pSysMemBlock, (long *)pInvStr, nInvBuffLen, (long *)szRet);
    free(pSysMemBlock);
    return nLen;
}

// szInOutFPDM szInOutFPHM
// 可以不指定值,直接自动选择一张发票;如果指定值,那么则便历检测指定发票代码号码是否为第一张
int AisinoGetCurrentInvCode(HUSB hUSB, uint8 bNeedType, char *szInOutFPHM, char *szOptInvEndNum,
                            char *szInOutFPDM)
{
//返回响应值
// 0040 0f 02 bd fd 04 00 e8 03 7e 32 b4 70 7e 32 da a0
// 0050 [32 00 13 46 50] 54 e7 29 00 f7
// 方括号的为发票类型代码，可能根据不同的发票类型有所不同
//不能读到想要的发票后面的缓冲区不管，必须全部读完缓冲区，哪怕提取获取到指定发票
#ifdef _NODEVICE_TEST
    PTData pTD = (PTData)hUSB->pTestData;
    strcpy(szInOutFPDM, pTD->szFPDM);
    strcpy(szInOutFPHM, pTD->szFPHM);
    return 0;
#endif
    uint8 cb[256] = {0};
    uint8 *pInvType5Byte = NULL;
    int ncbLen = sizeof cb, nOK = -1;
    bool bGiveFPDMHM = false;
    char szFPDM[32] = {0}, szFPHM[32] = {0}, szFPHMEnd[32] = {0};
    if (strlen(szInOutFPHM) || strlen(szInOutFPDM)) {
        bGiveFPDMHM = true;
        _WriteLog(LL_INFO,
                  "Check input fpdm and fphm is the top of invoice volume, fpdm:%s fphm:%s",
                  szInOutFPDM, szInOutFPHM);
    }
    while (1) {
        memcpy(cb, "\xfe\xff\xcc\xbc\x53\xe1\xe3\x00\x1a\x00", 10);
        int nRet = AisinoLogicIO(hUSB, cb, 10, ncbLen, NULL, 0);
        if (nRet < 20 || nRet > 32) {
            _WriteLog(LL_DEBUG, "AisinoGetCurrentInvCode AisinoLogicIO failed nRet = %d", nRet);
            break;
        }
        //设置continue
        uint8 bContinue = cb[nRet + 1];
        BYTE bInvType = cb[1];
        if (bNeedType != bInvType)
            goto GetCurrentInvCode_LoopNext;
        //下面都是所需求类型的
        int nInvCurrentNum = *(_DWORD *)(cb + 2) + *(_WORD *)(cb + 6) - *(_WORD *)(cb + 23);
        int nInvEndNum = *(_WORD *)(cb + 6) + *(_DWORD *)(cb + 2) - 1;
        pInvType5Byte = cb + 16;
        //如果已有发票卷枚举完，或者已经获取到需要的发票，则进行下轮循环
        if (nInvCurrentNum > nInvEndNum || !nOK) {
            goto GetCurrentInvCode_LoopNext;
        }
        memset(szFPDM, 0, 16);
        GetInvTypeCode(nInvCurrentNum, pInvType5Byte, szFPDM);
        sprintf(szFPHM, "%08d", nInvCurrentNum);
        sprintf(szFPHMEnd, "%08d", nInvEndNum);
        _WriteLog(LL_DEBUG, "AisinoGetCurrentInvCode enum fpdm:%s fphm:%s fpend:%s", szFPDM, szFPHM,
                  szFPHMEnd);
        if (bGiveFPDMHM) {
            if (strcmp(szFPDM, szInOutFPDM))
                goto GetCurrentInvCode_LoopNext;
            if (strcmp(szFPHM, szInOutFPHM))
                goto GetCurrentInvCode_LoopNext;
        } else {
            strcpy(szInOutFPDM, szFPDM);
            strcpy(szInOutFPHM, szFPHM);
        }
        if (szOptInvEndNum)
            strcpy(szOptInvEndNum, szFPHMEnd);
        nOK = 0;

    GetCurrentInvCode_LoopNext:  // goto 简化逻辑
        if (!bContinue)
            break;
    }
    return nOK;
}

int GetProcKey(char *sz9ByteHashTaxID, uint8 *abProcKey)
{
    uint8 ab9BitTaxCode[24];
    memset(ab9BitTaxCode, 0, 24);
    Str2Byte(ab9BitTaxCode, sz9ByteHashTaxID, 18);
    Makehashcode_GenProKey(abProcKey, ab9BitTaxCode);
    return 0;
}

int Packet8ByteVCode_Seed(uint8 *pInvData, uint8 *abProcKey, uint8 *dstBuff)
{
    int nInvBlockNum = 3;  // 3 or 0
    uint8 *v8 = pInvData;
    int nOffset = ((nInvBlockNum & 0xffff) - 1);
    uint8 *v9 = dstBuff + 8;
    char *v10 = (char *)abProcKey + 8;
    int i = 0;
    while (i++ < 8) {
        _BYTE v11 = v8[nOffset] ^ *(_BYTE *)(v10++ - 8);
        *(v9 - 8) = v11;
        *v9++ = *(_BYTE *)(v10 - 1);
    }
    return 0;
}

//参数1:发票原始数据的200h字节
int Packet8ByteVCode(uint8 *abProcKey, uint8 *pInvData, uint8 *dstBuff)
{
    uint8 abSeed[32];
    memset(abSeed, 0, sizeof(abSeed));
    Packet8ByteVCode_Seed(pInvData, abProcKey, abSeed);
    Build8BitMakeInvVCode((long)dstBuff, (long)abSeed, pInvData, 0x200);
    return 0;
}

unsigned __int16 BuildPreInvoice(uint8 *pFormatBuff, char *szKPStr, HFPXX fpxx)
{
    int nIndex = 0;
    pFormatBuff[0] = SetPreInvoiceBuff0(atof(fpxx->je));
    pFormatBuff[1] = fpxx->fplx_aisino;  // FPLX
    int nCurrentInvNum = atoi(fpxx->fphm);
    memcpy(pFormatBuff + 2, &nCurrentInvNum, 4);
    nIndex = 6;
    // 发票类型
    uint8 szOutTmp[128] = {0};
    int nChildRet = InvTypeCode2Byte5(szOutTmp, fpxx->fpdm);
    if (nChildRet != 5) {
        SetLastError(fpxx->hDev->hUSB, ERR_BUF_CHECK, "底层数据组包校验失败,发票代码有误");
        logout(INFO, "TAXLIB", "发票开具", "盘号%s,底层开票失败：%s\r\n", fpxx->hDev->szDeviceID,
               fpxx->hDev->hUSB->errinfo);
        return 0;
    }
    memcpy(pFormatBuff + nIndex, szOutTmp, nChildRet);
    nIndex += nChildRet;
    // 5位,发票总价格,格式化
    double dF = atof(fpxx->je);
    long lAddr = (long)&dF;
    JE2InvRaw((long)(pFormatBuff + 0xb), *(double *)lAddr, 0x64);
    // 5位,发票总税额，格式化
    dF = atof(fpxx->se);
    lAddr = (long)&dF;
    JE2InvRaw((long)(pFormatBuff + 0x10), *(double *)lAddr, 0x64);
    //税率变换 1byte
    if (!fpxx->isMultiTax) {  //非多税率才写入
        double fslv = atof(fpxx->slv);
        uint8 uSlv = (uint8)(fslv * 1000.0 + 0.1);
        pFormatBuff[0x15] = uSlv;
    }
    //购方税号变换生成 12字节
    uint8 bpFormatBuffOffset0x77[1] = {0x04};
    nIndex = 0x19;
    memset(szOutTmp, 0, sizeof(szOutTmp));
    if (TaxID212Bytes(fpxx->gfsh, szOutTmp)) {
        // 9132D111MA20J2WK2L错误税号导致失败
        SetLastError(fpxx->hDev->hUSB, ERR_BUF_CHECK, "底层数据组包校验失败,购方税号有误");
        logout(INFO, "TAXLIB", "发票开具", "盘号%s,底层开票失败：%s\r\n", fpxx->hDev->szDeviceID,
               fpxx->hDev->hUSB->errinfo);
        return 0;
    }
    uint8 bIndex0x38 = *(pFormatBuff + 0x38);
    bpFormatBuffOffset0x77[0] = szOutTmp[2];
    uint8 bChar = (szOutTmp[2] & 0xF) | 0x80 | 0x10;
    if (sub_8B1F970(bIndex0x38) == 1 || bIndex0x38 == 11 ||
        (bIndex0x38 >= 42u && bIndex0x38 <= 60u))
        bChar |= 0x20;
    if (fpxx->isMultiTax)  //可能还有其他参数控制进入，但是目前测试到多税率肯定是的
        bChar |= 0x40;
    szOutTmp[2] = bChar;
    memcpy(pFormatBuff + nIndex, szOutTmp, 12);
    // pFormatBuff 0x26-0x2b处
    // 目前测试只有专票购方信息表红字票有该信息，专票销方信息表红字票是否有该段信息待测试
    // 此处官方做法是判断是否16个0或者是16位通知单编号，如果是则留空不写入数据，否则写入数据
    if ((fpxx->fplx_aisino == FPLX_AISINO_ZYFP || fpxx->fplx_aisino == FPLX_AISINO_DZZP) &&
        fpxx->isRed && strlen(fpxx->redNum)) {
        //实际只判断是否有通知单编号即可
        uint8 *pTzdbh = pFormatBuff + 0x26;
        memset(pTzdbh, 0, 6);
        BuildRedTzdbh6Bytes(pTzdbh, fpxx->redNum);
    }
    // 发票类型
    nIndex = 0x2e;
    pFormatBuff[nIndex] = fpxx->fplx_aisino;
    InvTypeCode2Byte5(pFormatBuff + nIndex + 1, fpxx->fpdm);
    // 开票时间随机数种子生成2个字节
    uint8 szTimeBuff[24];
    if (TranslateStandTime(0, fpxx->kpsj_standard, (char *)szTimeBuff) < 0) {
        SetLastError(fpxx->hDev->hUSB, ERR_BUF_CHECK, "底层数据组包校验失败,时间格式有误");
        logout(INFO, "TAXLIB", "发票开具", "盘号%s,底层开票失败：%s\r\n", fpxx->hDev->szDeviceID,
               fpxx->hDev->hUSB->errinfo);
        return 0;
    }
    uint8 szTarget[32] = "";
    DeviceTimeRandomSeed((long)szTarget, (long)szTimeBuff, 7);
    long v66 = (long)szTarget;
    int v67 = 0;
    int v68 = 0;
    int v69 = 0;
    int v70 = 0;
    LOWORD(v67) = *(uint8 *)v66;
    v68 = 3 * v67;
    LOWORD(v68) = *(uint8 *)(v66 + 2);
    v69 = v68 + 100 * v67;
    LOWORD(v68) = *(uint8 *)(v66 + 4);
    v70 = v68 + 12 * v69 - 23880;
    uint8 b1 = v70;
    uint8 b2 = BYTE1(v70) | 8 * *(uint8 *)((long)(szTarget) + 6);
    nIndex = 0x3a;  //还有不兼容的?汕头盘3,有时间继续测试
    pFormatBuff[nIndex] = b1;
    pFormatBuff[nIndex + 1] = b2;
    uint16 *pwPreStr0x100x11 = (uint16 *)(pFormatBuff + 0x10);
    uint16 wVCode = 0;
    if (fpxx->isMultiTax)
        wVCode = *pwPreStr0x100x11;
    else
        wVCode = MakeJESLSE_VCodeByte(fpxx->fplx_aisino, fpxx->zyfpLx, atof(fpxx->je),
                                      atof(fpxx->slv), atof(fpxx->se), *pwPreStr0x100x11);
    *(uint16 *)(pFormatBuff + nIndex + 2) = wVCode;
    *(WORD *)(pFormatBuff + nIndex + 4) = 0x200;
    // 税号综合验证码
    uint8 szMem[128] = "";
    uint8 sz01FF[] = {0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF,
                      0xFE, 0xDC, 0xBA, 0x98, 0x76, 0x54, 0x32, 0x10};
    memcpy(szMem + 0x40, sz01FF, sizeof(sz01FF));
    //农产品收购发票开具，暂且false
    if (Get25ByteXor(fpxx->isMultiTax, false, (uint8 *)szMem, fpxx->hDev->szCompressTaxID,
                     fpxx->gfsh, fpxx->xfsh)) {
        SetLastError(fpxx->hDev->hUSB, ERR_BUF_CHECK,
                     "底层数据组包校验失败,多税率与销购方税号校验算法有误");
        logout(INFO, "TAXLIB", "发票开具", "盘号%s,底层开票失败：%s\r\n", fpxx->hDev->szDeviceID,
               fpxx->hDev->hUSB->errinfo);
        return 0;
    }
    sub_10034160((long)szMem);
    memcpy(pFormatBuff + 0x34, szMem + 0x40, 2);
    //购方税号变换生成前2个字节
    memcpy(pFormatBuff + 0x36, pFormatBuff + 0x19, 2);
    nIndex = 0x77;
    pFormatBuff[nIndex] = bpFormatBuffOffset0x77[0];
    // kp字符串
    nIndex = 0x98;
    int nSKPStrLen = strlen(szKPStr);
    nChildRet = EncodeInvoiceBlock2(szKPStr, nSKPStrLen, (char *)pFormatBuff + nIndex);
    *(uint16 *)(pFormatBuff + 0x96) = nChildRet;  //字符串长度写入
    *(uint8 *)(pFormatBuff + 0x95) = nChildRet >> 16;
    strcpy((char *)pFormatBuff + 0x70, "Head");

    int v118 = (((nChildRet >> 16) & 0xFF) << 16) + (uint16)nChildRet;
    uint16 v221 = (uint16)(((uint32)(v118 + 0x9C) >> 9) + ((((uint16)v118 + 0x9C) & 0x1FF) != 0));
    pFormatBuff[0x74] = (BYTE)v221;
    pFormatBuff[0x2d] = (BYTE)(v221 >> 8);
    if (IsAisino3Device(fpxx->hDev->nDriverVersionNum)) {
        //测试后发现此处aisino3不再写入 20220301
    } else {  //兼容aisino2
        uint16 uLenBuff = (uint16)nChildRet + 2;
        memcpy(pFormatBuff + 0x6e, &uLenBuff, 2);
    }
    pFormatBuff[0x76] = 0x00;
    return v221;  //第一个组包到此结束
}

char *GetMWJYM_BuildSignB64(HUSB hUSB, uint8 *pInDeviceMWJYMBuff, HFPXX fpxx)
{
    // pInDeviceMWJYMBuff一般为47个字节，前29个字节为密文前一半,后面25个字节为校验码
    //开票时，第一参数永远为1,农产品收购发票标志暂且为false
    if (DecodeMWandJYM(1, fpxx->isMultiTax, false, (uint8 *)pInDeviceMWJYMBuff,
                       fpxx->hDev->szCompressTaxID, fpxx->gfsh, fpxx->xfsh, fpxx->mw,
                       fpxx->jym) < 0)
        return NULL;
    char *pRetSignBase64 = NULL;
    int nRetBufLen = 0;
    char *pSignGBKBuf = GetSignString(fpxx, &nRetBufLen);
    if (!pSignGBKBuf)
        return NULL;
    //[aisino开票核对要素2-不在该代码中体现]
    //[aisino开票核对要素3.1-可选]
    _WriteHexToDebugFile("signstr_aisino_my.txt", (uint8 *)pSignGBKBuf, strlen(pSignGBKBuf));
    if (fpxx->hDev->bTrainFPDiskType || !hUSB->handle) {
        // 此处仅在测试盘，或者正式盘_NODEVICE_TEST宏启用的时进入
        uint8 abSignBin[512] = "";
        int nSignBinSize = 0;
        pRetSignBase64 = calloc(1, 1024);
        //[aisino开票核对要素4]
        if ((nSignBinSize = _ReadHexFromDebugFile("signout_aisino_kprj.bin", abSignBin,
                                                  sizeof(abSignBin))) == 327) {
            //自定义签名数据，由税盘dump获取
            nSignBinSize = Base64_Encode((const char *)abSignBin, nSignBinSize, pRetSignBase64);
        } else {
            //测试用签名Base64,测试训练盘(Traindisk)专用签名，如果需要修改其他签名，需要注释并保留这个Base64Str
            char szSignStr[] =
                "MIIBOgYJKoZIhvcNAQcCoIIBKzCCAScCAQExCzAJBgUrDgMCGgUAMAsGCSqGSIb3DQEHAjGCAQY"
                "wggECAgEBMGAwVTELMAkGA1UEBhMCY24xDTALBgNVBAseBABDAEExDTALBgNVBAgeBFMXTqwxGT"
                "AXBgNVBAMeEE4tVv16DlKhi6SLwU4tX8MxDTALBgNVBAceBFMXTqwCBwIBAAAAJEcwCQYFKw4DA"
                "hoFADANBgkqhkiG9w0BAQEFAASBgEnhTAZe2ygpABXdBcEkLoiDPuMfbEPKysACRJqmLRzjyykB"
                "gIfetmek+Iz43YnOX/LX3+OYplTn1hgCTTbLjbbefgaplmsD0p7EgIxU3yGvJyURK4ndVpk9/"
                "cVivoyx3yIyKot3cS6N9ZClI1R9NgwHhTwciDFjfihaRF94wICl";
            strcpy(pRetSignBase64, szSignStr);
        }
    } else {
        pRetSignBase64 = GetDeviceSignHashBase64(fpxx->hDev, pSignGBKBuf, nRetBufLen);
    }
    free(pSignGBKBuf);
    if (!pRetSignBase64)
        return NULL;
    return pRetSignBase64;
}

int BuildInvoice(uint8 *pFormatBuff, uint8 *pInDeviceMWJYMBuff, uint16 *uSepBlockNum, HFPXX fpxx,
                 char *pSignRepBase64)
{
    //从写入的缓冲重新读取Block2长度，并重新计算uSepBlockNum,这里的切块问题，容易引起上报汇总问题
    int nBlock2EncodeLen = *(uint16 *)(pFormatBuff + 0x96) + (*(uint8 *)(pFormatBuff + 0x95) << 16);
    //原先为256部分发票无法开具，后修改为200 20210427;空白作废只有128左右
    if (nBlock2EncodeLen <= 100) {
        _WriteLog(LL_FATAL, "BuildInvoice nBlock2EncodeLen = %d", nBlock2EncodeLen);
        return -1;
    }
    //取得切割份数
    *uSepBlockNum = 0;
    *uSepBlockNum =
        (((((nBlock2EncodeLen >> 16) & 0xFF) << 16) + (unsigned __int16)nBlock2EncodeLen + 152) >>
         9) +
        ((((_WORD)nBlock2EncodeLen + 152) & 0x1FF) != 0);
    int nPacketNumMAX = DEF_MAX_FPBUF_LEN / 512 + 1;
    if (0 == *uSepBlockNum || *uSepBlockNum > nPacketNumMAX)
        return -2;
    //至此，校验码和验证码已经获取完成，该函数第一部分完成,如下发送发票第二块
    char outMW[512] = "";
    strcpy(outMW, fpxx->mw);
    strcpy(outMW + strlen(outMW) + 1, fpxx->jym);
    //这里的0x6c应该是个变量,后期如果有需要可能还要修改,0x6c 可能是密文长度
    BuildInv0x40(outMW, (uint8 *)pFormatBuff + 0x40);
    //拷贝开票第一次原始验证码16字节到第二次开票请求5e处
    memcpy(pFormatBuff + 0x5e, pInDeviceMWJYMBuff + 0x1e, 0x10);
    //发票头部针对第一次请求的补充
    pFormatBuff[0x76] = 0x01;  //该数字应该是一个个数的统计
    pFormatBuff[0x74] = (BYTE)*uSepBlockNum + pFormatBuff[0x76];  //=格式化块份数+0x76个数
    //尾部签名算法
    int nAvailBuffLen = (*uSepBlockNum & 0xffff) * 512;
    pFormatBuff[nAvailBuffLen] = 0xff;
    pFormatBuff[nAvailBuffLen + 1] = pFormatBuff[0x76];
    memcpy(pFormatBuff + nAvailBuffLen + 2, pFormatBuff + 2, 9);
    int nIndex = 0x13;
    strcpy((char *)pFormatBuff + nAvailBuffLen + nIndex, "Sign");
    uint8 *abSignData = calloc(1, 2 * strlen(pSignRepBase64) + 256);
    if (!abSignData)
        return -3;
    int nOutLen = Base64_Decode(pSignRepBase64, strlen(pSignRepBase64), (char *)abSignData);
    if (nOutLen <= 0) {
        free(abSignData);
        return -4;
    }
    unsigned __int16 uSignDataLen = (unsigned __int16)nOutLen;
    nIndex = 0x1c;
    memcpy(pFormatBuff + nAvailBuffLen + nIndex, &uSignDataLen, 2);
    nIndex = 0x1e;
    memcpy(pFormatBuff + nAvailBuffLen + nIndex, abSignData, uSignDataLen);
    free(abSignData);
    //写入Tail标识
    int nFinishFlagOffset = pFormatBuff[0x76] << 9;
    strcpy((char *)pFormatBuff + nAvailBuffLen + nFinishFlagOffset - 4, "Tail");
    return nAvailBuffLen + nFinishFlagOffset;
}

//这个标志位设置错误会导致月底上报汇总出现金额比对失败问题?
void SetMakeInvoiceCommandByte7Flag(uint8 bAisinoFPLX, uint8 *pIOCommand)
{
    uint8 bFPLXIOFlag = 0;
    switch (bAisinoFPLX) {
        case FPLX_AISINO_HYFP:
        case FPLX_AISINO_DZFP:
        case FPLX_AISINO_ESC:
            bFPLXIOFlag = 0x1;
            break;
        case FPLX_AISINO_JDCFP:
            bFPLXIOFlag = 0x2;
            break;
        case FPLX_AISINO_JSFP:
            bFPLXIOFlag = 0xC1u;
            break;
        default:  // FPLX_AISINO_ZYFP,FPLX_AISINO_PTFP
            bFPLXIOFlag = 0;
    }
    pIOCommand[7] = bFPLXIOFlag;
}

// abRepBuff's buff len must > 256
int SendPreInvoice(HUSB hUSB, uint8 bAisinoFPLX, unsigned __int16 uSepBlockNum, uint8 *pFormatBuff,
                   uint8 *abRepBuff)
{
    int nAvailBuffLen = (uSepBlockNum & 0xffff) * 512;
    uint8 *pPreMakeBuff = (uint8 *)calloc(1, nAvailBuffLen + 1);
    memcpy(pPreMakeBuff, pFormatBuff, nAvailBuffLen);
    int nTailFlagIndex = nAvailBuffLen - 4;
    strcpy((char *)pPreMakeBuff + nTailFlagIndex, "Tail");
    // packet verity flag
    int nPackStaticEnd = 0x200;  //开票软件设定死的，0x80*4=512字节
    pPreMakeBuff[nPackStaticEnd] = MakePackVerityFlag(pPreMakeBuff, nPackStaticEnd);
    nPackStaticEnd += 1;
    // IO comm
    uint8 cb[256];
    memcpy(cb, "\xfe\xff\xcc\xbc\x53\xe1\xdf\x00\x00\x00", 10);
    SetMakeInvoiceCommandByte7Flag(bAisinoFPLX, cb);
    //发送校验码验证码请求参数，返回这两个结果
    int nChildRet = 0;
#ifndef _NODEVICE_TEST
    int ncbLen = sizeof cb;
    nChildRet = AisinoLogicIO(hUSB, cb, 10, ncbLen, pPreMakeBuff, nPackStaticEnd);
#else
    if (hUSB) {  //编译警告
    }
    nChildRet = 48;
#endif
    //[aisino开票核对要素-可选]
    _WriteHexToDebugFile("preinv_aisino_my.bin", pPreMakeBuff,
                         nPackStaticEnd - 1);  //校验位不写入
    free(pPreMakeBuff);
    if (nChildRet < 36 || nChildRet > 256)
        return -1;
    memcpy(abRepBuff, cb, nChildRet);
    return 0;
}

void GetInvoiceEnv(uint8 *pRep, HFPXX fpxx)
{
    uint32 dwDZSYH = bswap_32(*(uint32 *)(pRep + 0x2e));
    fpxx->dzsyh = dwDZSYH;
}

int SendInvoice(HUSB hUSB, HFPXX fpxx, unsigned __int16 uSepBlockNum, uint8 *pFormatBuff)
{
    //发送第二次请求时512字节后会补一段1+8字节校验数据，其中第一个字节不动
    //后面8个字节有一个算法;
    uint8 abProcKey[32];
    memset(abProcKey, 0, 32);
    if (GetProcKey(fpxx->hDev->sz9ByteHashTaxID, abProcKey) < 0)
        return -1;

#ifdef _NODEVICE_TEST
    _WriteLog(LL_FATAL, "_NODEVICE_TEST Mode，Jump really make!");
    return 0;
#endif
    /////++debug 避免错误开票，先禁用
	//////_WriteLog(LL_FATAL, "开票过程基本无误，为防止失误开票，现已禁用最终开票包发送，"
	//////                "请手动在代码中取消注释并激活开票模块");
	//////SetLastError(fpxx->hDev->hUSB, ERR_PROGRAME_NEED_UNLOCK,"已禁用底层最终开票包发送,请联系开发人员");
	//////logout(INFO, "TAXLIB", "发票开具","盘号%s,底层开票失败：%s\r\n", fpxx->hDev->szDeviceID, 
	//////       fpxx->hDev->hUSB->errinfo);
	//////return -100;

    uSepBlockNum++;
    uint8 abSendCopy[768];
    int i = 0;
    for (i = 0; i < uSepBlockNum; i++) {
        uint8 cb[256];
        int ncbLen = sizeof cb;
        uint8 *pSendIndex = pFormatBuff + i * 512;
        memset(abSendCopy, 0, sizeof(abSendCopy));
        memcpy(abSendCopy, pSendIndex, 0x200);
        abSendCopy[0x200] = 0x00;
        Packet8ByteVCode(abProcKey, abSendCopy, abSendCopy + 0x201);
        int nPackStaticEnd = 0x209;
        abSendCopy[nPackStaticEnd] = MakePackVerityFlag(abSendCopy, nPackStaticEnd);
        nPackStaticEnd += 1;
        if (i == 0) {                                            //第一个包和其他的不一样
            if (IsAisino3Device(fpxx->hDev->nDriverVersionNum))  // 1521 error
                memcpy(cb, "\xfe\xff\xcc\xbc\x53\xe1\xd4\x00\x00\x00", 10);
            else
                memcpy(cb, "\xfe\xff\xcc\xbc\x53\xe1\xe0\x00\x00\x00", 10);
            SetMakeInvoiceCommandByte7Flag(fpxx->fplx_aisino, cb);
        } else
            memcpy(cb, "\xfe\xff\xcc\xbc\x53\xe1\xec\x00\x00\x00", 10);
        int nChildRet = AisinoLogicIO(hUSB, cb, 10, ncbLen, abSendCopy, nPackStaticEnd);
        if (nChildRet < 0)
            return -2;
        if (i == 0)  // i == 0; first send;第一个包发送时，税盘会返回此次票开的地址索引号等信息
            GetInvoiceEnv(cb, fpxx);
    }
    return 0;
}

//获取校验码及密文接口
int BuildRawToSend(HUSB hUSB, HFPXX fpxx, char *szKPStr)
{
    int nBuffLen = strlen(szKPStr) * 4 + 128000;
    uint8 *pFormatBuff = (uint8 *)calloc(1, nBuffLen);
    //输入缓冲区内容 必须全置0
    int nRet = -1;
    uint8 abRepBuff[256];
    while (1) {
        unsigned __int16 uSepBlockNum = BuildPreInvoice(pFormatBuff, szKPStr, fpxx);
        if (0 == uSepBlockNum) {
            logout(INFO, "TAXLIB", "发票开具", "BuildPreInvoice failed uSepBlockNum = %d\r\n",
                   uSepBlockNum);
            nRet = -100;  //此错误值与上层判断有关，不可更改
            break;
        }
        memset(abRepBuff, 0, sizeof(abRepBuff));
        if (SendPreInvoice(hUSB, fpxx->fplx_aisino, uSepBlockNum, pFormatBuff, abRepBuff) < 0) {
            nRet = -2;
            break;
        }
#ifdef _NODEVICE_TEST
        //[aisino开票核对要素2]
        if (47 != _ReadHexFromDebugFile("mwjym47bytes_aisino_kprj.bin", abRepBuff, 47)) {
            //这里的47个字节是由税盘即时生成的，每次都不一样，因此每次都要更换这47个字节
            unsigned char data[47] = {0x10, 0xE9, 0x8C, 0x14, 0x6B, 0xD7, 0xE9, 0x9B, 0x93, 0x9F,
                                      0x89, 0x82, 0x6C, 0x02, 0x83, 0x62, 0x11, 0xD9, 0x68, 0xE5,
                                      0xEC, 0x41, 0x67, 0x0F, 0xB2, 0x0C, 0x60, 0x81, 0x08, 0x00,
                                      0x6A, 0x10, 0xE9, 0x44, 0x29, 0xAF, 0x67, 0x4F, 0xC2, 0xBA,
                                      0x23, 0x2E, 0xDE, 0x5E, 0xFF, 0x0A, 0xB7};
            memcpy(abRepBuff, data, 47);
        }
#endif

        char *pSignRepBase64 = GetMWJYM_BuildSignB64(hUSB, abRepBuff, fpxx);
        if (!pSignRepBase64 || strlen(pSignRepBase64) < 64 ||
            strlen(pSignRepBase64) > sizeof(fpxx->sign)) {
            nRet = -3;
            break;
        }
        //保存签名base64至开票信息
        strcpy(fpxx->sign, pSignRepBase64);
        //_WriteLog(LL_DEBUG, "DeviceSN:%s InvoiceTime:%s MW:%s JYM:%s SignB64:%s",
        //          fpxx->hDev->szDeviceID, fpxx->kpsj_F1, fpxx->mw, fpxx->jym, fpxx->sign);
        //这里的uSepBlockNum将会被重新计算赋值
        int nChildRet = BuildInvoice(pFormatBuff, abRepBuff, &uSepBlockNum, fpxx, pSignRepBase64);
        free(pSignRepBase64);
        if (nChildRet < 0 || !uSepBlockNum) {
            _WriteLog(LL_FATAL, "BuildInvoice error result = %d\n", nChildRet);
            nRet = -4;
            break;
        }
        //[aisino开票核对要素5]
        _WriteHexToDebugFile("kpFinalIO_aisino_my.bin", pFormatBuff, nChildRet);
        int result;
        if ((result = SendInvoice(hUSB, fpxx, uSepBlockNum, pFormatBuff)) < 0) {
            _WriteLog(LL_FATAL, "SendInvoice error result = %d\n", result);
            if (result <= -100) {
                logout(INFO, "TAXLIB", "发票开具", "SendInvoice failed result = %d\r\n", result);
                nRet = result;
            } else
                nRet = -5;
            break;
        }
        nRet = 0;
        break;
    }
    if (nRet == 0)
        _WriteLog(LL_INFO, "Makeinvoice successful, Type:%d(%d) FPDM:%s FPHM:%s\r\n", fpxx->fplx,
                  fpxx->fplx_aisino, fpxx->fpdm, fpxx->fphm);
    else
        _WriteLog(LL_FATAL, "Makeinvoice failed, Type:%d(%d) FPDM:%s FPHM:%s\r\n", fpxx->fplx,
                  fpxx->fplx_aisino, fpxx->fpdm, fpxx->fphm);
    free(pFormatBuff);
    return nRet;
}

int CheckDriverVerSupport(HDEV hDev, char *szDriverVersion)
{
	int nRet = RET_SUCCESS;
    char *p = strchr(szDriverVersion, '-');
    if (!p)
        return -1;
    p += 1;
	//501测试盘无需判断此规则

	if (memcmp(p, "180104", 6) <= 0)	{
		nRet = SetLastError(hDev->hUSB, ERR_DEVICE_FIRMWARE_LOWVERSION,
			"底层驱动版本太低,底层驱动版本太低无法支持开票，需升级底层版本,driverVer:%s", hDev->szDriverVersion);		
	}
	else if (memcmp(p, "200812", 6) < 0)	{
		nRet = SetLastError(hDev->hUSB, ERR_DEVICE_FIRMWARE_LOWVERSION,
			"底层驱动版本太低,系统已暂停2.0税盘支持，需升级底层版本,driverVer:%s", hDev->szDriverVersion);
	}
	//else if (memcmp(p, "220524", 6) >= 0)	{
	//	nRet = SetLastError(hDev->hUSB, ERR_DEVICE_FIRMWARE_HIGHVERSION,
	//		"底层驱动版本过高,当前系统暂不支持此版本，请联系运维人员,driverVer:%s", hDev->szDriverVersion);		
	//}
	if (nRet < 0){
		if (strcmp(hDev->szDeviceID, "661551502671") == 0)		{
			ClearLastError(hDev->hUSB);
			_WriteLog(LL_INFO,"测试盘不关心底层版本");
			nRet = 0;
		}else{
			//report_event(hDev->szDeviceID, "底层驱动版本太低", hDev->hUSB->szLastErrorDescription, -1);
			logout(INFO, "TAXLIB", "系统判断", "盘号：%s,错误信息：%s\r\n", hDev->szDeviceID, hDev->hUSB->szLastErrorDescription);
		}
	}

	return nRet;
}

int MakeInvoiceMain(HFPXX fpxx)
{
    int nRet = ERR_GENERIC;
    int nChildRet = 0;
    HDEV hDev = fpxx->hDev;
    HUSB hUSB = fpxx->hDev->hUSB;
    char szBuf[64] = "";
    // 1-检查参数
    if ((nRet = CheckDriverVerSupport(hDev, fpxx->driverVersion)) < 0) {
        return nRet;
    }
    // 2-防呆代码,只有开票时自动填入所属月份
    memset(fpxx->ssyf, 0, sizeof fpxx->ssyf);
    strncpy(fpxx->ssyf, hDev->szDeviceTime, 6);
    // 2-获取开票序号信息
    if (AisinoGetCurrentInvCode(hUSB, fpxx->fplx_aisino, fpxx->fphm, fpxx->fpendhm, fpxx->fpdm) <
        0) {
        if ((strlen(fpxx->fphm) > 0) || (strlen(fpxx->fpdm) > 0))
            nRet =
                SetLastError(hUSB, ERR_DEVICE_CURRENT_INVOICE,
                             "当前发票代码号码与传入的发票代码号码不一致,请再次确认避免重复开具");
        else
            nRet = SetLastError(hUSB, ERR_DEVICE_CURRENT_INVOICE,
                                "获取当前发票代码号码失败,请再次确认避免重复开具");
        return nRet;
    }
    // 3-如果启用则计算公共服务平台编号
    if (strlen(hDev->szPubServer)) {
        char szPubPtlx[16];
        //如果启用
        if ((nChildRet =
                 BuildPubServiceRandom(fpxx->fpdm, fpxx->fphm, fpxx->kpsj_standard, szBuf)) < 0) {
            nRet = SetLastError(hUSB, ERR_LOGIC_COMMON, "生成平台随机数失败");
            logout(INFO, "TAXLIB", "发票开具", "盘号：%s,生成平台随机数失败,result = %d\r\n",
                   hDev->szDeviceID, nRet);
            return nRet;
        }
        memset(szPubPtlx, 0, sizeof(szPubPtlx));
        strncpy(szPubPtlx, hDev->szPubServer, 2);  // copy type 2bytes
        sprintf(fpxx->pubcode, "%s;%s;%s", szPubPtlx, hDev->szPubServer + 2, szBuf);
    }
    //给出可用发票信息
    _WriteLog(LL_DEBUG, "Get available invoice: FPDM:%s FPHM:%s \r\n", fpxx->fpdm, fpxx->fphm);
    // 4-生成开票字符串
    char *pKPStr = (char *)GetInvString(fpxx);
    if (pKPStr == NULL) {
        nRet = SetLastError(hUSB, ERR_LOGIC_BUILD_FORMATBUF, "生成发票数据失败");
        goto NextTry;
    }
    // 5-开票IO
    if ((nChildRet = BuildRawToSend(hUSB, fpxx, pKPStr)) < 0) {
        //蒙柏错误
        logout(INFO, "TAXLIB", "发票开具", "盘号：%s,底层开票失败,result = %d,LastError【%s】\r\n",
               hDev->szDeviceID, nChildRet, hDev->hUSB->errinfo);
        //错误判断
        _WriteLog(LL_FATAL, "底层开具失败需要重试开具,【%s】\n", hDev->hUSB->errinfo);
        if ((strstr(hDev->hUSB->errinfo, "[USB] 161") != NULL) ||
            (strstr(hDev->hUSB->errinfo, "[USB] 248") != NULL)) {
            _WriteLog(LL_FATAL, "税盘明确报错,不在重试\n");
            fpxx->need_restart = 0;
        } else
            fpxx->need_restart = 1;
        nRet = nChildRet;
        goto NextTry;
    }
    nRet = RET_SUCCESS;
NextTry:
    if (strcmp(fpxx->fphm, fpxx->fpendhm) == 0) {
        _WriteLog(LL_FATAL, "已使用到发票卷最后一张发票,开具完成后需要进行断电操作");
        fpxx->need_restart = 1;
    }
    if (NULL != pKPStr)
        free(pKPStr);
    return nRet;
}

int FillDevInfoCert(HDEV hDev, uint8 bAuthDev)
{
    int nRet = -1;
    AisinoGetDeivceTime(
        hDev->hUSB,
        hDev->szDeviceTime);  //更新税盘时间,蒙柏方为避免证书锁死，和频繁调用减少IO操作会单独调用此函数
    while (1) {
        //载入盘配置信息
        if (hDev->pX509Cert != NULL) {
            ;  // _WriteLog(LL_FATAL, "原x509证书句柄存在无需再次读取\r\n");//否则会存在x509泄露
        } else if (!hDev->bTrainFPDiskType && bAuthDev) {
            X509 *pCert = NULL;
            pCert = GetX509Cert(hDev->hUSB, hDev->bDeviceType, hDev->bCryptBigIO);
            if (!pCert) {
                _WriteLog(LL_FATAL, "Get cert failed!,last errinfo =　%s \r\n",
                          hDev->hUSB->errinfo);
                nRet = -1;
                break;
            }
            hDev->pX509Cert = pCert;
            //开票的签名时候需要输入密码去验证证书,如果不是测试盘需要打开税务证书，否则后面开票失败
            nRet = SignAPI_OpenDevice(hDev->hUSB, hDev->bCryptBigIO, DEF_CERT_PASSWORD_DEFAULT);
            if (nRet <= 0 && nRet > -100) {
                _WriteLog(LL_FATAL, "Device password incorrect\n");
                nRet = ERR_CERT_PASSWORD_ERR;
                break;
            } else if (nRet < -100) {
                _WriteLog(LL_FATAL, "OpenDevice cert failed\n");
                nRet = DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
                break;
            }
        } else {
            hDev->pX509Cert = NULL;
        }
        //--载入静态变量
        nRet = 0;
        break;
    }
    return nRet;
}

int LoadAisinoDevInfo(HDEV hDev, HUSB hUSB, uint8 bAuthDev)
{
    int nRet = -1, nChildRet = 0;
    hDev->hUSB = hUSB;
    hDev->bDeviceType = DEVICE_AISINO;
#ifdef _NODEVICE_TEST
    if (LoadNoDeviceJson(hDev) < 0)
        goto LoadAisinoDevInfo_Finish;
    nRet = 0;
    goto LoadAisinoDevInfo_Finish;
#endif
    //压缩税号
    if (GetCompressTaxCode(hUSB, hDev->szCompressTaxID) < 0) {
        nRet = -2;
        goto LoadAisinoDevInfo_Finish;
    }
    // 9位hash税号
    if (Get9BitHashTaxCode(hUSB, hDev->sz9ByteHashTaxID) < 0) {
        nRet = -3;
        goto LoadAisinoDevInfo_Finish;
    }
    if (AisinoGetDeivceTime(hUSB, hDev->szDeviceTime) < 0) {
        nRet = -5;
        goto LoadAisinoDevInfo_Finish;
    }
    //设备编号SN及加密区IO判断
    nChildRet = ReadTaxDeviceID(hUSB, hDev->szDeviceID);
    if (nChildRet > 0)
        hDev->bCryptBigIO = 1;
    else if (0 == nChildRet)
        hDev->bCryptBigIO = 0;
    else {  //< 0
        nRet = -6;
        goto LoadAisinoDevInfo_Finish;
    }
    //通用显示税号 税盘公司名称
    if (AisinoReadTaxIDandCustomName(hUSB, hDev->szCommonTaxID, hDev->szCompanyName,
                                     sizeof(hDev->szCompanyName)) < 0) {
        nRet = -7;
        goto LoadAisinoDevInfo_Finish;
    }
    // driver ver
    if (GetDriverVersion(hUSB, hDev->szDriverVersion, &hDev->nDriverVersionNum) < 0) {
        nRet = -8;
        goto LoadAisinoDevInfo_Finish;
    }
    // jmbmh
    if (GetJMBBH(hUSB, hDev->szJMBBH) < 0) {
        nRet = -8;
        goto LoadAisinoDevInfo_Finish;
    }
    // trandisk check
    hDev->bTrainFPDiskType = GetTrainDeviceType(hUSB, hDev);
    //监控信息
    if (AisinoStateInfoToHDEV(hDev) < 0) {
        nRet = -9;
        goto LoadAisinoDevInfo_Finish;
    }
    // extra info
    if (AisinoFillExtraDeviceInfo(hDev) < 0) {
        nRet = -10;
        goto LoadAisinoDevInfo_Finish;
    }

    if (!hUSB) {  // test mode
        hDev->bTrainFPDiskType = 0;
        hDev->pX509Cert = NULL;
        nRet = 0;
    } else {
        nRet = FillDevInfoCert(hDev, bAuthDev);
    }
LoadAisinoDevInfo_Finish:
    if (nRet)
        _WriteLog(LL_FATAL, "LoadDevInfo failed,nRet = %d\n", nRet);
#ifdef _CONSOLE
    else
        _WriteLog(LL_DEBUG,
                  "DevInfo, region:%s devid:%s taxidstr:%s "
                  "cusname:%s taxauth:%s NatureOfTaxpayer:%d",
                  hDev->szRegCode, hDev->szDeviceID, hDev->szCommonTaxID, hDev->szCompanyName,
                  hDev->szTaxAuthorityName, hDev->bNatureOfTaxpayer);
#endif
    return nRet;
}

//----空白作废代码begin
bool AisinoBuildBlankWasteFpxx(HDEV hDev, HFPXX fpxx, uint8 bNeedType)
{
    // szFPDM szFPHM szKpr,外部已经初始化
    if (!strlen(fpxx->fpdm) || !strlen(fpxx->fphm) || !strlen(fpxx->kpr))
        return false;
    //专票在签名中作废标志为N，测试时偶尔复现普票为Y，后续无法再复现，怀疑和开票软件未初始化有关
    //如果后续空白作废签名出现问题，可以先从此处排查
    //
    //但是专票读取出来的标志位又是作废票，可能是其历史遗留问题
    //签名算法没什么特殊原因肯定是不能动的，为了小小的空白作废，只能在此处自己兼容了
    // if (bNeedType == FPLX_AISINO_PTFP)
    //     fpxx->zfbz = true;
    fpxx->fpzt = (uint8)FPSTATUS_BLANK_WASTE;
    fpxx->fplx_aisino = bNeedType;
    AisinoFPLX2CommonFPLX(fpxx->fplx_aisino, &fpxx->fplx);
    UpdateFpxxAllTime(hDev->szDeviceTime, fpxx);
    GetVersionInfo(fpxx, hDev);
    strcpy(fpxx->gfmc, "0");
    strcpy(fpxx->gfsh, "000000000000000");
    strcpy(fpxx->xfmc, hDev->szCompanyName);
    strcpy(fpxx->xfsh, hDev->szCommonTaxID);
    strcpy(fpxx->je, "0.00");
    strcpy(fpxx->se, "0.00");
    //不写入开票字符串，但是写入发票raw头部，0.13税率，我也很奇怪 :-)
    strcpy(fpxx->slv, "0.13");
    strcpy(fpxx->qdbj, "N");
    return true;
}

int AisinoLoopBlankWaste(HFPXX fpxx, uint8 bInvoiceType, int nCount)
{
    int nRet = ERR_GENERIC;
    int i = 0;
    for (i = 0; i < nCount; i++) {
        char *pKPStr = NULL;
        int nChildRet = ERR_GENERIC;
        do {
            _WriteLog(LL_INFO, "空白作废发票代码%s 发票号码%s", fpxx->fpdm, fpxx->fphm);
            if (!AisinoBuildBlankWasteFpxx(fpxx->hDev, fpxx, bInvoiceType))
                break;
            // 生成开票字符串
            pKPStr = (char *)GetInvString(fpxx);
            if (pKPStr == NULL)
                break;
            // 空白作废使用的是开票IO
            if ((nChildRet = BuildRawToSend(fpxx->hDev->hUSB, fpxx, pKPStr)) < 0)
                break;
            nChildRet = RET_SUCCESS;
        } while (false);
        nRet = nChildRet;
        if (pKPStr)
            free(pKPStr);
        if (nChildRet < 0)
            break;
        if (i != nCount - 1)
            StringNumIncrement(fpxx->fphm);
    }
    _WriteLog(LL_INFO, "BlankWaste invoice count:%d %s", nCount,
              nRet == 0 ? "successful" : "failed");
    return nRet;
}

int AisinoBlankWaste(HDEV hDev, uint8 bInvoiceType, char *szFPDM, char *szFPHM, char *zfr,
                     int nCount, int *need_restart)
{
    int nRet = ERR_GENERIC;
    if (bInvoiceType != FPLX_AISINO_ZYFP && bInvoiceType != FPLX_AISINO_PTFP) {
        return SetLastError(hDev->hUSB, ERR_LOGIC_DZFP_CANNOT_WASTE,
                            "发票类型代码错误，作废发票只能为纸专、纸普");
    }
    if ((nRet = CheckDriverVerSupport(hDev, hDev->szDriverVersion)) < 0) {
        return nRet;
    }
    HFPXX fpxx = MallocFpxxLinkDev(hDev);
    do {
        strcpy(fpxx->fpdm, szFPDM);
        strcpy(fpxx->fphm, szFPHM);
        strcpy(fpxx->kpr, zfr);
        if (AisinoGetCurrentInvCode(hDev->hUSB, bInvoiceType, fpxx->fphm, fpxx->fpendhm,
                                    fpxx->fpdm)) {
            nRet = SetLastError(hDev->hUSB, ERR_PARM_CHECK,
                                "发票代码号码指定错误，必须是发票卷当前头部代码号码");
            break;
        }
        if (!CheckFphmRangeIncorrect(fpxx->fphm, fpxx->fpendhm, nCount)) {
            nRet = SetLastError(hDev->hUSB, ERR_PARM_CHECK, "发票代码号码范围错误，超过发票卷范围");
            break;
        }
        if ((nRet = AisinoLoopBlankWaste(fpxx, bInvoiceType, nCount)) < 0)
            break;
        nRet = RET_SUCCESS;
    } while (false);
    if (strcmp(fpxx->fphm, fpxx->fpendhm) == 0) {
        _WriteLog(LL_WARN, "已使用到发票卷最后一张发票,开具完成后需要进行断电操作");
        fpxx->need_restart = 1;
        *need_restart = 1;
    }
    FreeFpxx(fpxx);
    return nRet;
}
//----空白作废代码finish