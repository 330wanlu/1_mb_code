/*****************************************************************************
File name:   function.c
Description: 用于航信、百旺、UKey通用函数和一些非业务相关的通用函数
Author:      Zako
Version:     1.0
Date:        2019.12
History:
20220109        原代码行数已超过2000多行，减负分离所有字符和编码转换
                等方法大约1000行代码另立character.c
*****************************************************************************/
#include "function.h"
#include <openssl/obj_mac.h>
#include <openssl/ossl_typ.h>
#include <openssl/x509v3.h>

extern uint8 g_bLogLevel;

#ifdef _CONSOLE
void _WriteLog(uint8 bLoglevl, char *msg, ...)
{
    if (bLoglevl > g_bLogLevel)
        return;
    va_list val = {0};
    char temp[51200] = "";
    va_start(val, msg);
    vsprintf(temp, msg, val);
    va_end(val);

    char szSep[3] = "";
    if (temp[strlen(temp) - 1] != '\n')
        strcpy(szSep, "\r\n");
    time_t b_time;
    struct tm *tim;
    b_time = time(NULL);
    tim = localtime(&b_time);
    printf("[%d-%d %d:%d:%d]%s%s", tim->tm_mon + 1, tim->tm_mday, tim->tm_hour, tim->tm_min,
           tim->tm_sec, temp, szSep);

    FILE *fp = fopen("/tmp/aisino.log", "a");
    if (fp == NULL)
        return;
    fprintf(fp, "[%d-%d %d:%d:%d]%s%s", tim->tm_mon + 1, tim->tm_mday, tim->tm_hour, tim->tm_min,
            tim->tm_sec, temp, szSep);

    fflush(fp);
    fclose(fp);
    return;
}
#else
void _WriteLog(uint8 bLoglevl, char *msg, ...)
{
	va_list args;
	char buf[204800];
	char buf1[204800];
	//char time_buf[20];
	uint8 timer[8];
	uint8 timer_asc[50];
	//int i;
	//char *pbuf;
	
	va_start(args, msg);
	memset(buf1, 0, sizeof(buf1));
	vsprintf(buf1, msg, args);
	va_end(args);

	char end[50] = { 0 };
	if (buf1[strlen(buf1) - 1] != '\n')
		strcpy(end, "\r\n");

	memset(buf, 0, sizeof(buf));
	clock_get(timer);
	memset(timer_asc, 0, sizeof(timer_asc));
	time_asc(timer, timer_asc, 7);
	//print_time((uint8 *)time_buf);
	char time_m[100];
	memset(time_m, 0, sizeof(time_m));
	timer_read_asc_ms(time_m);
	//printf("%s\n", time_m);
	//sprintf(buf,"%s:---%s",timer_asc,buf1);
	sprintf(buf, "%s", buf1);

	memset(buf1, 0, sizeof(buf1));
	G2U_GBKIgnore(buf, strlen(buf), buf1, sizeof(buf1));

	EncordingConvert("gbk", "utf-8", buf, strlen(buf), buf1, sizeof(buf1));
	printf("%s %s%s", time_m, buf1,end);
	
}
#endif

////老算法，同一个税号多次开票网卡变化，可能被税局检测到，使用税号或者设备号做种子可以保证网卡唯一
//但是生成的网卡地址可能是非注册的地址，需要正规MAC必须前6位固定，暂且不考虑
// bSplitFormat 0==aisino;1==nisec;2==cntax
void GetNicMAC(char *szTaxID, char *szOut, uint8 bSplitFormat)
{
    //"1831BF" 华硕前缀ASUSTek COMPUTER INC.
    if (bSplitFormat == 0) {  //-分割，aisino风格;实测aisino电票为"00-0C-29-C8-B4-9C"格式
        char szBuf1[8] = "", szBuf2[8] = "", szBuf3[8] = "";
        strcpy(szOut, "18-31-BF");
        GetRandom(szBuf1, 2, 16, szTaxID + 1);
        GetRandom(szBuf2, 2, 16, szTaxID + 2);
        GetRandom(szBuf3, 2, 16, szTaxID + 3);
        sprintf(szOut + 8, "-%s-%s-%s", szBuf1, szBuf2, szBuf3);
    } else {  // 实测nisec mac="000C298205A9" 格式， 实测cntax和nisec一致
        strcpy(szOut, "1831BF");
        GetRandom(szOut + 6, 6, 16, szTaxID);
    }
}

void GetMotherBoardId(char *szTaxID, char *szOut)
{
    // example:PC1H0P9H
    strcpy(szOut, "PC");
    GetRandom(szOut + 2, 6, 16, szTaxID);
}

void FreeDevInfo(HDEV hDev)
{
    if (!hDev)
        return;
    if (hDev->pX509Cert != NULL) {
        X509_free(hDev->pX509Cert);
        hDev->pX509Cert = NULL;
    }
    if (hDev->lstHeadDiti != NULL) {
        free(hDev->lstHeadDiti);
        hDev->lstHeadDiti = NULL;
    }
    free(hDev);
}

HDEV MallocDevInfo()
{
    HDEV hDev = calloc(1, sizeof(struct StaticDeviceInfo));
    if (!hDev)
        return NULL;
    if (!(hDev->lstHeadDiti = calloc(FPLX_MAX_NUM, sizeof(struct DeviceInvoiceTypeInfo)))) {
        free(hDev);
        return NULL;
    }
    return hDev;
}

HUSB MallocUSBSession()
{
    int ret = 0;
    HUSB husb = calloc(1, sizeof(struct USBSession));
    if (!husb)
        return NULL;
    ret = libusb_init(&husb->context);
    if(ret != 0){                               	                //wang 02 07 尝试解决机柜重启问题
        printf("MallocUSBSession libusb_init error%d!!!!!!!!!!!!!\r\n",ret);    
        free(husb);
        return NULL;
    }
    //libusb_set_debug(husb->context, LIBUSB_LOG_LEVEL_DEBUG);        //wang 02 07 尝试解决机柜重启问题
    //libusb_set_debug(husb->context, LIBUSB_LOG_LEVEL_INFO);  // LIBUSB_LOG_LEVEL_DEBUG
#ifdef _NODEVICE_TEST
    husb->pTestData = calloc(1, sizeof(struct stTestData));
#endif
    return husb;
}

/*
HUSB MallocUSBSession()
{
    HUSB husb = calloc(1, sizeof(struct USBSession));
    if (!husb)
        return NULL;
    libusb_init(&husb->context);
    // libusb_set_debug(husb->context, LIBUSB_LOG_LEVEL_INFO);  // LIBUSB_LOG_LEVEL_DEBUG
#ifdef _NODEVICE_TEST
    husb->pTestData = calloc(1, sizeof(struct stTestData));
#endif
    return husb;
}
*/


void FreeUSBSession(HUSB husb)
{
    // udev's cache 8192bytes 全局leak，不会产生循环leak
    if (!husb)
        return;
    if (husb->handle)
        usb_device_close(husb);
    husb->handle = NULL;
    libusb_exit(husb->context);
    husb->context = NULL;
#ifdef _NODEVICE_TEST
    free(husb->pTestData);
#endif
    free(husb);
    husb = NULL;
}

//------------------------------浮点计算支持 begin-----------------
//高精度小数点金额截断并进行四舍五入计算,szOutPrice可以是原输入缓冲也可以单独缓冲
void PriceRound(char *szPrice, int nRoundSize, char *szOutPrice)
{
    //空字符串不处理
    if (!strlen(szPrice)) {
        strcpy(szOutPrice, "");
        return;
    }
    //变量初始化
    mpfr_t mpf1, mpf2, mpf3;
    mpfr_init2(mpf1, 128);
    mpfr_init2(mpf2, 128);
    mpfr_init2(mpf3, 128);
    mpfr_set_str(mpf1, szPrice, 10, MPFR_RNDA);
    mpfr_set_d(mpf2, pow(10, nRoundSize), MPFR_RNDA);
    //四舍五入计算
    mpfr_mul(mpf3, mpf1, mpf2, MPFR_RNDA);  // mpf3=mpf1*mpf2
    mpfr_round(mpf1, mpf3);                 // mpf1=round(mpf3)
    mpfr_div(mpf3, mpf1, mpf2, MPFR_RNDA);  // mpf3=mpf1/mpf2
    mpfr_sprintf(szOutPrice, "%.*Rf", nRoundSize, mpf3);
    //清除释放
    mpfr_clear(mpf1);
    mpfr_clear(mpf2);
    mpfr_clear(mpf3);
    mpfr_free_cache();
}

//高精度含税价格转不含税价格，nRoundSize为指定的小数点后精度长度，szOutPriceWithTax为输出无税价格；截断为根据后一位四舍五入
int PriceRemoveTax(char *szDJ, char *szSLV, int nRoundSize, char *szOutPriceWithTax)
{
    if (!strlen(szDJ) || !strlen(szSLV))
        return -1;
    if (nRoundSize < 0 || nRoundSize > 100)
        return -2;
    // MPFR_RNDN：四舍五入（IEEE 754-2008中的四舍五入）;四舍六入五取偶？
    // MPFR_RNDZ：向零舍入（IEEE 754-2008中向零舍入）；直接舍去？
    // MPFR_RNDU：向正无穷大舍入（IEEE 754-2008中向正舍入）；
    // MPFR_RNDD：向负无穷大舍入（在IEEE 754-2008中向负舍入）‘
    // MPFR_RNDA:从零开始四舍五入；向零四舍五入
    // MPFR_RNDF：精确舍入。此功能目前正在试验中。特定支持
    //初始化变量
    mpfr_t mpf1, mpf2, mpf3;
    mpfr_init2(mpf1, 128);
    mpfr_init2(mpf2, 128);
    mpfr_init2(mpf3, 128);
    //计算无税单价
    mpfr_set_str(mpf1, szDJ, 10, MPFR_RNDA);
    mpfr_set_str(mpf2, szSLV, 10, MPFR_RNDA);
    mpfr_set_str(mpf3, "1", 10, MPFR_RNDA);
    mpfr_add(mpf2, mpf2, mpf3, MPFR_RNDA);
    mpfr_div(mpf3, mpf1, mpf2, MPFR_RNDA);  // mpf3 已经是高精度无税单价
    //无税单价进行指定位数四舍五入
    mpfr_set_d(mpf2, pow(10, nRoundSize), MPFR_RNDA);
    mpfr_mul(mpf1, mpf3, mpf2, MPFR_RNDA);  // mpf1=mpf3*mpf2
    mpfr_round(mpf3, mpf1);                 // mpf3=round(mpf1)
    mpfr_div(mpf1, mpf3, mpf2, MPFR_RNDA);  // mpf1=mpf3/mpf2
    mpfr_sprintf(szOutPriceWithTax, "%.*Rf", nRoundSize, mpf1);
    //清除释放
    mpfr_clear(mpf1);
    mpfr_clear(mpf2);
    mpfr_clear(mpf3);
    mpfr_free_cache();
    return 0;
}
//------------------------------浮点计算支持 finish--------------------

// 0 or 1
int ProgInitDestory(uint8 bInitOrDestory)
{
    if (bInitOrDestory) {
        //首先环境检查
        setenv("TZ", "GMT-8", 1);
        char szUG[16] = "", szBuf[32] = "",
             abIn[] = {0x77, 0x00, 0x69, 0x00, 0x6e, 0x00, 0x64, 0x00, 0x6f, 0x00, 0x77, 0x00};
        if (GBKBase64ToUTF8_NP("zPW8/rLiytQ=", szUG, sizeof(szUG)) < 0 ||
            Base64_Decode("5p2h5Lu25rWL6K+V", 16, szBuf) < 0 || strcmp(szUG, szBuf)) {
            _WriteLog(LL_FATAL, "UTF8-GBK change envirment check failed, run env command "
                                "first.\r\nexport LD_PRELOAD=/lib/preloadable_libiconv.so");
            return -1;
        }
        if (6 != EncordingConvert("unicodelittle", "utf-8", abIn, 12, szBuf, sizeof(szBuf))) {
            _WriteLog(LL_FATAL, "Library iconv envirment incorrect");
            return -2;
        }
        //初始化SSL库,添加SSL握手时所有支持的算法
        SSL_library_init();
        ERR_load_BIO_strings();
        SSL_load_error_strings();
        OpenSSL_add_ssl_algorithms();
        return 1;
    } else {
        // clear all resource
        CRYPTO_cleanup_all_ex_data();
        CONF_modules_free();
        CONF_modules_unload(1);
        ERR_free_strings();
        EVP_cleanup();
        mpfr_free_cache();
        return 1;
    }
}



// 1= bin, 2=str
int GetCertSNByteOrStr(X509 *pCert, char *retSN, uint8 bRetBinOrStr)
{
    ASN1_INTEGER *asn1_i = X509_get_serialNumber(pCert);
    BIGNUM *bnser = ASN1_INTEGER_to_BN(asn1_i, NULL);
    int nLen = BN_bn2bin(bnser, (uint8 *)retSN);
    BN_free(bnser);
    if (nLen != 7)
        return -1;
    if (bRetBinOrStr)
        return 0;  // return bin
    char szBuf[16] = "";
    Byte2Str(szBuf, (uint8 *)retSN, 7);
    strcpy(retSN, szBuf);
    return 0;  // return str
}

uint32 GetU32IdFromString(char *src)
{
    uint8 dst[16] = "";
    CalcMD5(src, strlen(src), (void *)dst);
    uint32 ret = *(uint32 *)dst;
    ret = htonl(ret);
    return ret;
}

void GetRandom(char *szOut, int nInNeedLen, int nMod, char *szSeedString)
{
    if (!szSeedString)
        srand((unsigned int)time(NULL));
    else
        srand(GetU32IdFromString(szSeedString));
    int i;
    for (i = 0; i < nInNeedLen; i++) {
        sprintf(szOut + i, "%X", rand() % nMod);
    }
}

//实现不一定完全，够用就行
// szStandTimeIn 输入格式yyyymmddhhmmss
int MyStrptime(char *szStandTimeIn, struct tm *tt0)
{
    if (strlen(szStandTimeIn) != 14)
        return -1;
    unsigned int nY = 0, nM = 0, nD = 0, nH = 0, nMn = 0, nS = 0;
    sscanf((char *)szStandTimeIn, "%04d%02d%02d%02d%02d%02d", &nY, &nM, &nD, &nH, &nMn, &nS);
    memset(tt0, 0, sizeof(struct tm));  //!!!特别注意，tt0最好赋初值，否则某些量容易出错
    tt0->tm_sec = nS;
    tt0->tm_min = nMn;
    tt0->tm_hour = nH;
    tt0->tm_mday = nD;
    tt0->tm_mon = nM - 1;
    tt0->tm_year = nY - 1900;
    return 0;
}

//这里没法做返回值了，因此输入参数一定要对
// 20190101151515
int MinusTime(char *szTime1, char *szTime2)
{
    struct tm tmTime;
    MyStrptime(szTime1, &tmTime);
    time_t tTime1 = mktime(&tmTime);
    MyStrptime(szTime2, &tmTime);
    time_t tTime2 = mktime(&tmTime);
    int nRet = tTime1 - tTime2;
    return nRet;
}

//判断当前操作是否超出临界值,返回值为0，才是符合条件的
int TimeOutMonthDeadline(char *szBaseTime, char *szTimeDev)
{
    if (strlen(szTimeDev) != 14 || strlen(szBaseTime) != 14)
        return -1;
    struct tm tmTime;
    MyStrptime(szTimeDev, &tmTime);
    time_t tTimeNow = mktime(&tmTime) + 300;  //多加上预留5分钟

    MyStrptime(szBaseTime, &tmTime);
    int nNewY = tmTime.tm_year + 1900;
    int nNewM = tmTime.tm_mon + 1;
    if (nNewM == 12) {
        nNewY += 1;
        nNewM = 1;
    } else
        nNewM += 1;

    char szDeadline[24] = "";
    sprintf(szDeadline, "%04d%02d01000000", nNewY, nNewM);
    MyStrptime(szDeadline, &tmTime);
    time_t tDeadline = mktime(&tmTime);

    if (tTimeNow - tDeadline >= 0)
        return 1;
    return 0;
}

bool IsTheSameMonth(char *szTime1, char *szTime2)
{
    if (strlen(szTime1) != 14 || strlen(szTime2) != 14)
        return false;
    if (!strncmp(szTime1, szTime2, 6))
        return true;
    return false;
}


//高精度计算，含税价不含税价格互相转换
// bPriceWithTax==true 高精度含税价格转不含税价格;== false 高精度不含税价格转含税价格
// nRoundSize为指定的小数点后精度长度，szOutPrice为输出价格；截断为根据后一位四舍五入//old:PriceRemoveTax
int CalcTaxratePrice(bool bPriceWithTax, char *szDJ, char *szSLV, int nRoundSize, char *szOutPrice)
{
    if (!strlen(szDJ) || !strlen(szSLV))
        return -1;
    if (nRoundSize < 0 || nRoundSize > 100)
        return -2;
    // MPFR_RNDN：四舍五入（IEEE 754-2008中的四舍五入）;四舍六入五取偶？
    // MPFR_RNDZ：向零舍入（IEEE 754-2008中向零舍入）；直接舍去？
    // MPFR_RNDU：向正无穷大舍入（IEEE 754-2008中向正舍入）；
    // MPFR_RNDD：向负无穷大舍入（在IEEE 754-2008中向负舍入）‘
    // MPFR_RNDA:从零开始四舍五入；向零四舍五入
    // MPFR_RNDF：精确舍入。此功能目前正在试验中。特定支持
    //初始化变量
    mpfr_t mpf1, mpf2, mpf3;
    mpfr_init2(mpf1, 128);
    mpfr_init2(mpf2, 128);
    mpfr_init2(mpf3, 128);
    //计算目标价格
    mpfr_set_str(mpf1, szDJ, 10, MPFR_RNDA);
    mpfr_set_str(mpf2, szSLV, 10, MPFR_RNDA);
    mpfr_set_str(mpf3, "1", 10, MPFR_RNDA);
    mpfr_add(mpf2, mpf2, mpf3, MPFR_RNDA);
    if (bPriceWithTax)  //含税价格/(税率+1)=不含税价格
        mpfr_div(mpf3, mpf1, mpf2, MPFR_RNDA);
    else  //不含税价格*(税率+1)=含税价格
        mpfr_mul(mpf3, mpf1, mpf2, MPFR_RNDA);
    // mpf3为目标价格
    //目标价格进行指定位数四舍五入
    mpfr_set_d(mpf2, pow(10, nRoundSize), MPFR_RNDA);
    mpfr_mul(mpf1, mpf3, mpf2, MPFR_RNDA);  // mpf1=mpf3*mpf2
    mpfr_round(mpf3, mpf1);                 // mpf3=round(mpf1)
    mpfr_div(mpf1, mpf3, mpf2, MPFR_RNDA);  // mpf1=mpf3/mpf2
    mpfr_sprintf(szOutPrice, "%.*Rf", nRoundSize, mpf1);
    //清除释放
    mpfr_clear(mpf1);
    mpfr_clear(mpf2);
    mpfr_clear(mpf3);
    mpfr_free_cache();
    return 0;
}

int GetRandNum(int nMin, int nMax)
{
    int result = 0, low_num = 0, hi_num = 0;
    if (nMin < nMax) {
        low_num = nMin;
        hi_num = nMax + 1;
    } else {
        low_num = nMax + 1;
        hi_num = nMin;
    }
    uint32 nSeed = 0;
    RAND_bytes((uint8 *)&nSeed, 4);
    srand(nSeed);
    result = (rand() % (hi_num - low_num)) + low_num;
    return result;
}


int GetSelfAbsolutePath(char *buf, int nBufflen)
{
    int count;
    count = readlink("/proc/self/exe", buf, nBufflen);
    if (count < 0 || count >= nBufflen)
        return -1;
    buf[count] = '\0';
    return 0;
}

void CalcMD5(void *pSrc, int nSrcLen, void *pDst16)
{
    MD5_CTX ctx;
    MD5_Init(&ctx);
    MD5_Update(&ctx, pSrc, nSrcLen);
    MD5_Final(pDst16, &ctx);
}

// digest's len=SHA_DIGEST_LENGTH > 20
int CalcSHA1(void *pSrc, int nSrcLen, void *pOutDigest)
{
    SHA_CTX ctx;
    if (!SHA1_Init(&ctx))
        return -1;
    if (!SHA1_Update(&ctx, pSrc, nSrcLen))
        return -2;
    if (!SHA1_Final(pOutDigest, &ctx))
        return -3;
    return 0;
}

uint8 *CalcHmacSha256(const void *key, int keylen, const uint8 *data, int datalen, uint8 *result,
                      uint32 *resultlen)
{
    return HMAC(EVP_sha256(), key, keylen, data, datalen, result, resultlen);
}

//标准时间转换各种其他格式
// 0=0x20 0x19 0x01 0x01 0x00 0x00 0xx00   7位二进制格式
// 1=20190101 00:00:00
// 2=2019-05-01 15:00:01
// 3=201901011200  无秒
// 4=20190101 121212
// 5=2019-05-01   截取日期
int TranslateStandTime(int nType, char *szStandTimeIn, char *szOutTime)
{
    if (strlen(szStandTimeIn) != 14)
        return -1;
    uint8 cb[12] = {0};
    sscanf((char *)szStandTimeIn, "%02x%02x%02x%02x%02x%02x%02x", (unsigned int *)&cb[0],
           (unsigned int *)&cb[1], (unsigned int *)&cb[2], (unsigned int *)&cb[3],
           (unsigned int *)&cb[4], (unsigned int *)&cb[5], (unsigned int *)&cb[6]);
    if (cb[0] > 0x30 || cb[0] < 0x10 || cb[2] > 0x12 || cb[3] > 0x31 || cb[4] > 0x24 ||
        cb[5] > 0x60 || cb[6] > 0x60)
        return -2;
    switch (nType) {
        case 0:
            memcpy(szOutTime, cb, 7);
            break;
        case 1:
            sprintf(szOutTime, "%02x%02x%02x%02x %02x:%02x:%02x", cb[0], cb[1], cb[2], cb[3], cb[4],
                    cb[5], cb[6]);
            break;
        case 2:
            sprintf(szOutTime, "%02x%02x-%02x-%02x %02x:%02x:%02x", cb[0], cb[1], cb[2], cb[3],
                    cb[4], cb[5], cb[6]);
            break;
        case 3:
            sprintf(szOutTime, "%02x%02x%02x%02x%02x%02x", cb[0], cb[1], cb[2], cb[3], cb[4],
                    cb[5]);
            break;
        case 4:
            sprintf(szOutTime, "%02x%02x%02x%02x %02x%02x%02x", cb[0], cb[1], cb[2], cb[3], cb[4],
                    cb[5], cb[6]);
            break;
        case 5:
            sprintf(szOutTime, "%02x%02x-%02x-%02x", cb[0], cb[1], cb[2], cb[3]);
            break;
        default:
            return -2;
            break;
    }
    return 0;
}

void UpdateDevInfoToFPXX(HDEV hDev, HFPXX fpxx)
{
    //强制税盘开票人税号和开票人名称更新开票信息
    if (fpxx->isHzxxb == true && fpxx->hzxxbsqsm != 0) {
        _WriteLog(LL_INFO, "购买方红字信息表时强制修改购方信息");
        strcpy(fpxx->gfsh, hDev->szCommonTaxID);
        strcpy(fpxx->gfmc, hDev->szCompanyName);
    } else {
        strcpy(fpxx->xfsh, hDev->szCommonTaxID);
        strcpy(fpxx->xfmc, hDev->szCompanyName);
    }
    strcpy(fpxx->jqbh, hDev->szDeviceID);
    //更新必要税盘信息到发票信息
    fpxx->kpjh = hDev->uICCardNo;
    fpxx->hDev = hDev;
    //格式化各种格式时间
    UpdateFpxxAllTime(hDev->szDeviceTime, fpxx);
    //红字信息表不强制更新，防止将原发票信息替换成现版本信息，以原发票为准
    if (fpxx->isHzxxb == false) {
        // ssyf
        memset(fpxx->ssyf, 0, sizeof(fpxx->ssyf));
        strncpy(fpxx->ssyf, hDev->szDeviceTime, 6);
    }

#ifdef _NODEVICE_TEST
    //以import.json中指定版本和编码表版本号
#else
    //以代码中值强制更新
    switch (hDev->bDeviceType) {
        case DEVICE_AISINO:
            strcpy(fpxx->taVersion, defTaxAuthorityVersionAisino);
            break;
        case DEVICE_NISEC:
            strcpy(fpxx->taVersion, defTaxAuthorityVersionNisec);
            break;
        case DEVICE_CNTAX:
            strcpy(fpxx->taVersion, defTaxAuthorityVersionCntax);
            break;
        default:
            break;
    }
    //不得开具低于当前最低版本号，否则容易被税局发现问题
    strcpy(fpxx->bmbbbh, DEF_BMBBBH_DEFAULT);
#endif
}

//四舍五入*100,取绝对值
inline int GetFotmatJE100(char *je)
{
    int f = abs(round(atof(je) * 100));
    return f;
}

// GetFotmatJEdouble,转字符串
inline void GetFotmatJEStr(char *je, char *szOut)
{
    int64 lJENew = (int64)GetFotmatJE100(je);
    sprintf(szOut, "%012llx", lJENew);
}

// old:Get_spsmmc_spmc
int GetSpsmmcSpmc(char *szSmmcMc, char *spsmmc, char *spmc)
{
    char szBuf[200] = "";
    strcpy(szBuf, szSmmcMc);
    if (!strchr(szBuf, '*')) {
        strcpy(spmc, szSmmcMc);
        return 0;
    }
    if (szBuf[0] != '*') {
        strcpy(spmc, szSmmcMc);
        return 0;
    }
    char *p2rdFlag = strstr(szBuf + 1, "*");
    if (p2rdFlag == NULL) {
        strcpy(spmc, szSmmcMc);
        return 0;
    }
    p2rdFlag += 1;
    memcpy(spsmmc, szBuf, p2rdFlag - szBuf);
    memcpy(spmc, p2rdFlag, strlen(szBuf) - strlen(spsmmc));
    return 0;
}

int GetSignatureCert(X509 *pX509Cert)
{
    int nRet = 0;
    BASIC_CONSTRAINTS *skid = NULL;
    ASN1_BIT_STRING *lASN1UsageStr = NULL;
    while (1) {
        char oid[64] = {0};
        ASN1_OBJECT *algo = X509_get0_tbs_sigalg(pX509Cert)->algorithm;
        int nChildRet = OBJ_obj2txt(oid, sizeof(oid), algo, 1);
        if (nChildRet < 8)
            break;
        if (strcmp(oid, "1.2.840.113549.1.1.5") &&  //检测签名算法是否是sha1rsa
            strcmp(oid, "1.2.156.10197.1.501"))  //检测签名算法是否是基于SM3的SM2签名算法
            break;

        //根据openssl文档，单项目结构体指针结构好像不用free
        // check 2; jump CA cert
        int crit = 0;
        skid = (BASIC_CONSTRAINTS *)X509_get_ext_d2i(pX509Cert, NID_basic_constraints, &crit, NULL);
        if (!skid || skid->ca != 0)
            break;
        // check 3;jump not signature cert
        lASN1UsageStr = (ASN1_BIT_STRING *)X509_get_ext_d2i(pX509Cert, NID_key_usage, NULL, NULL);
        if (!lASN1UsageStr)
            break;
        uint16 usage = lASN1UsageStr->data[0];
        if (lASN1UsageStr->length > 1)
            usage |= lASN1UsageStr->data[1] << 8;
        if (!(usage & KU_DIGITAL_SIGNATURE && usage & KU_NON_REPUDIATION))
            break;

        nRet = 1;
        break;
    }

    BASIC_CONSTRAINTS_free(skid);
    ASN1_BIT_STRING_free(lASN1UsageStr);
    return nRet;
}

//如果是批量开票，这里的开票使用的证书句柄值，应该保存下来,不用时候应该尽快释放
X509 *GetX509Cert(HUSB hUSB, uint8 bDeviceType, bool bBigCryptIO)
{
    X509 *x = NULL;
    int i = 0;
    int nIndex = 2;
    void *pCerts = calloc(1, 20480);
    switch (bDeviceType) {
        case DEVICE_AISINO: {
            if (AisinoEnumCertsMem(hUSB, pCerts, bBigCryptIO) < 2)
                goto CheckCertFinish;
            break;
        }
        case DEVICE_NISEC: {
            if (NisecEnumCertsMem(hUSB, pCerts) < 2)
                goto CheckCertFinish;
            break;
        }
        case DEVICE_CNTAX: {
            if (CntaxEnumCertsMem(hUSB, pCerts) < 2)
                goto CheckCertFinish;
            break;
        }
        default:
            goto CheckCertFinish;
            break;
    }
    uint16 *pNum = (uint16 *)pCerts;
    if (0 == *pNum) {
        goto CheckCertFinish;
    }
    for (i = 0; i < *pNum; i++) {
        uint16 *pCertLen = (uint16 *)(pCerts + nIndex);
        uint8 *pCert = (uint8 *)(pCerts + nIndex + 2);
        //保存所有的证书至本地
        // char szPath[128] = "";
        // sprintf(szPath, "cert-%d.der", i);
        // _WriteHexToDebugFile(szPath, pCert, *pCertLen);

        if (!d2i_X509(&x, (const uint8 **)&pCert, *pCertLen))
            continue;
        if (GetSignatureCert(x)) {
            break;
        } else {
            X509_free(x);
            x = NULL;
        }
        nIndex += 2 + *pCertLen;
    }
CheckCertFinish:
    free(pCerts);
    return x;
}

int CheckAndAppendCertBuf(uint8 *pRetBuff, int nRetBuffIndex, uint8 *pCertBuff, int nCertLen)
{
    if (nCertLen < 0 || nCertLen > 2048 || nCertLen < 768)
        return 0;
    if (NULL == MyMemSearch(pCertBuff, pCertBuff + nCertLen, (uint8 *)"\x30\x82", 2))
        return 0;  //没有证书标识返回
    memcpy(pRetBuff + nRetBuffIndex, &nCertLen, 2);
    memcpy(pRetBuff + nRetBuffIndex + 2, pCertBuff, nCertLen);
    nRetBuffIndex += 2 + nCertLen;
    return nRetBuffIndex;
}

void GetBJSJ(char *szDeviceTime, char *szOut, uint8 bStyle)
{
    char szBuf[64] = "";
    TranslateStandTime(2, szDeviceTime, szOut);
    memset(szBuf, 0, sizeof(szBuf));
    strncpy(szBuf, szOut, 7);
    if (0 == bStyle)  // 2020-01-01 00:00
        strcat(szBuf, "-01 00:00");
    else if (1 == bStyle)  // 2020-01-01 00:00:00
        strcat(szBuf, "-01 00:00:00");
    else if (2 == bStyle) {  // 202001010000
        memset(szBuf, 0, sizeof(szBuf));
        strncpy(szBuf, szDeviceTime, 6);
        strcat(szBuf, "010000");
    }
    strcpy(szOut, szBuf);
}

int TAXML_Formater(mxml_node_t *root, char *szOut)
{
    char *ptr = mxmlSaveAllocString(root, MXML_NO_CALLBACK);
    char *pNewFormat = StrReplace(ptr, "\n", " ");
    //段尾调整
    int nXMLLen = strlen(pNewFormat);
    if (pNewFormat[nXMLLen - 1] == ' ')
        pNewFormat[nXMLLen - 1] = '\0';
    strcpy(szOut, pNewFormat);
    free(ptr);
    free(pNewFormat);
    return strlen(szOut);
}

void UpdateFpxxAllTime(char *szStandTimeInput, HFPXX fpxx)
{
    strcpy(fpxx->kpsj_standard, szStandTimeInput);
    TranslateStandTime(1, szStandTimeInput, fpxx->kpsj_F1);
    TranslateStandTime(2, szStandTimeInput, fpxx->kpsj_F2);
    TranslateStandTime(3, szStandTimeInput, fpxx->kpsj_F3);
}

//打印该机柜设备上所有设备类型、设备号、税号、公司名称，用于找盘  自己无需使用
//int EnumAllUSBDevice_CallBack(int nDeviceType, HUSB hUSB)
//{
//    bool bRet = false;
//    libusb_device *device = libusb_get_device(hUSB->handle);
//    int nBusID = libusb_get_bus_number(device);
//    int nDevID = libusb_get_device_address(device);
//    char szDeviceType[16] = "", szCommonTaxID[24] = "", szCompanyName[256] = "",
//         szDeviceID[24] = "";
//    switch (nDeviceType) {
//        case DEVICE_AISINO: {
//            strcpy(szDeviceType, "A");
//            if (AisinoOpen(hUSB) < 0)
//                break;
//            if (AisinoReadTaxIDandCustomName(hUSB, szCommonTaxID, szCompanyName,
//                                             sizeof(szCompanyName)) < 0)
//                break;
//            ReadTaxDeviceID(hUSB, szDeviceID);
//            AisinoClose(hUSB);
//            bRet = true;
//            break;
//        }
//        case DEVICE_NISEC: {
//            strcpy(szDeviceType, "N");
//            HDEV hDev = MallocDevInfo();
//            if (!hDev)
//                break;
//            hDev->hUSB = hUSB;
//            if (NisecEntry(hUSB, NOP_OPEN_FULL) < 0)
//                break;
//            GetDiskEnvFromManageData(hDev);
//            strcpy(szCommonTaxID, hDev->szCommonTaxID);
//            strcpy(szCompanyName, hDev->szCompanyName);
//            strcpy(szDeviceID, hDev->szDeviceID);
//            FreeDevInfo(hDev);
//            bRet = true;
//            break;
//        }
//        case DEVICE_CNTAX: {
//            strcpy(szDeviceType, "C");
//            HDEV hDev = MallocDevInfo();
//            if (!hDev)
//                break;
//            hDev->hUSB = hUSB;
//
//            uint8 abDeviceOpenInfo[64] = {0};
//            if (CntaxEntryRetInfo(hUSB, NOP_OPEN_FULL, abDeviceOpenInfo) < 0)
//                break;
//            if (CnTaxGetDeviceEnv(hDev, hUSB, abDeviceOpenInfo) < 0) {
//                CntaxEntry(hUSB, NOP_CLOSE);
//                break;
//            }
//            CntaxEntry(hUSB, NOP_CLOSE);
//            strcpy(szCommonTaxID, hDev->szCommonTaxID);
//            strcpy(szCompanyName, hDev->szCompanyName);
//            strcpy(szDeviceID, hDev->szDeviceID);
//            FreeDevInfo(hDev);
//            bRet = true;
//            break;
//        }
//        default:
//            _WriteLog(LL_WARN, "Unsupport device type");
//            break;
//    }
//    if (!bRet)
//        _WriteLog(LL_WARN, "Enum device failed, busid:03%d devid:03%d", nBusID, nDevID);
//    else {
//        _WriteLog(LL_INFO, "bus:%03d dev:%03d %s %s %s %s", nBusID, nDevID, szDeviceType,
//                  szDeviceID, szCommonTaxID, szCompanyName);
//    }
//    return 0;
//}

//默认值为0，设置true时所有流程全部快速中断退出，记得及时恢复为false
//通过伪全局变量，通知工作线程中正在处于IO状态的设备取消循环、阻塞状态
void DevIOCtrl(HDEV hDev, bool bStatus)
{
    hDev->bBreakAllIO = bStatus;
}

//根据静态税号来判断是否测试盘
uint8 StaticCheckTrainDeviceByDeviceID(char *szDeviceID)
{
    // clang-format off
    char szGroup[][24] = {
	//"xxxxxxxxxxxx",//--ff--ffffffffffffff

	//金税盘测试盘
    "661551502671",//蒙柏--航信培训企业--320102999999501
	"661551338399",//蒙柏--航信培训企业--320901999999205
	"661551275480",//蒙柏--航信培训企业--320901999999207
	"661401824869",//蒙柏--航信培训企业--320901999999208	
	"661401813537",//蒙柏--航信培训企业--320901999999209
	"661565671563",//票通--500102192801051381--500102192801051381
	"661707894780",//--航信培训企业--339901999999209
	"661551401482",//--航信培训企业--440002999999075
	"661500152244",//--航信培训企业--320501999999098
	"661711123478",//--航信培训企业--339901999999502
	"661617935445",//--航信培训企业--150105999999501	
	"661512381378",//--test--91500020020718697X
	"661600082159",//--400102204710268392--400102204710268392
	"661612339468",//--422010201709012004--422010201709012004
	"661612339265",//--422010201709012004--422010201709012004
	"661840689777",//--5000002020112501A--5000002020112501A	
	"661565723941",//--500102201007206608--500102201007206608
	"661504734698",//--915001021210000032--915001021210000032
	"661504734680",//--915001021210000034--915001021210000034
    "661731475551",

	//税控盘测试盘，！！！注意！！！注意！！！注意，百旺的测试盘是正式证书的，不能加入这里，否则会导致qmcs为01a8，签名错误
	"499000146849",//蒙柏--1--123456789098765	
	"499000144018",//--rrr--111222333456111	
	"499000138591",//--test12345--500102010004050
	"499000156502",//票通--测试用户--91142010202002279F
	"499000156570",//票通--测试用户--91142010202002270F      //在线
	"499000146857",//--百旺测试企业--91510198766347M
	"539900210491",//--测试用户7891--913201234567891
	"499000132869",//--测试用户3176--500102010003176	
	"539900211021",//--升级版测试用户226--500102010000226
	"499000115225",//--升级版测试用户1412--500102010001412	
	"499000135074",//--升级版测试用户3697--500102010003697
	"499000135082",//--升级版测试用户3698--500102010003698		
	"499000135091",//--升级版测试用户3699--500102010003699
	"499000136051",//--升级版测试用户3782--500102010003782
	"499000135947",//--升级版测试用户3792--500102010003792
	"539990020041",//--江苏一九年测试用户2--903201111111112
	"499000134531",//--上海百旺测试3643--500102010003643
	"499000160309",//--云票测试一零九--110101201801010109
	"499000152421",//--云票测试五十四--110101201707010054	
	"499000140033",//--测试--500102010088889
	"499000140025",//蒙柏--测试2019--9131000032955577SC
	"499000143277",//--测试3277--410104999000143277
	"499000107583",//--测试用户0424--500102010000424
	"499000155614",//--测试盘5614--91500102000005614Y
	"499000155690",//--测试盘5690--91500102010005690X
	"499000140711",//蒙柏--贵州百旺测试毕节1--91520103063000BJ1
	"499000143584",//--增值税发票测试3266--400811888888883266
	"499000166840",//--宁夏简易税控盘测试02--151000000000000123
	"889999999583",//--贵州百旺测试盘铜仁分公司--500102019999999
	"499000139729",//--贵州百旺简易测试盘贾维强--520681234500005
    //"929910415654",//河南电子发票公共服务平台测试3 914101050260260263 #不能作为测试盘，否则qmcs为01a8，签名错误
	"499000177611",//--贵州百望测试8--

	//ukey测试盘
	"587003206501", //蒙柏--河南恩博商贸有限公司--91410102075443367X
	"587003107634"//蒙柏--滑县戴瑞皮具有限责任公司--410526582890516
	};
    // clang-format on
    int cmdNum = sizeof(szGroup) / sizeof(szGroup[0]);
    int i = 0;
    for (i = 0; i < cmdNum; i++) {
        if (!strcmp(szGroup[i], szDeviceID)) {
            return 1;
        }
    }
    return 0;
}

//返回值为剩余尝试此处，<0则跳过函数执行，可以忽略返回值
int PasswordNotice(bool bDeviceOrCert, char *szLastErrorDescription)
{
    char szTmpBuf[16] = "";
    char *pErr = NULL;
    if (!strlen(szLastErrorDescription))
        return 0;
    if (!(pErr = strstr(szLastErrorDescription, "ECode:0x")))
        return 0;
    pErr += 8;
    // ukey的没有实盘测试是否为该值，后续如果有锁死的盘记得测试下
    if (!strcmp(pErr, "96983"))
        strcpy(szTmpBuf, "0");
    else {
        char t = pErr[strlen(pErr) - 1];
        sprintf(szTmpBuf, "%c", t);
    }
    _WriteLog(LL_FATAL,
              "%s password incorrect, please check. Remaining "
              "[\033[1;31m--%s--\033[0m] "
              "times to retry",
              bDeviceOrCert ? "Device" : "Certificate", szTmpBuf);
    return atoi(szTmpBuf);
}

HDITI GetInvoiceTypeEnvirnment(HDEV hDev, uint8 bCommmonInvType)
{
    HDITI ret = NULL;
    int nTypeCount = hDev->abSupportInvtype[0], i = 0;
    struct DeviceInvoiceTypeInfo *node = (struct DeviceInvoiceTypeInfo *)hDev->lstHeadDiti;

    if (hDev->lstHeadDiti == NULL)
        return ret;
    while (i++ < nTypeCount) {
        if (node->bInvType == bCommmonInvType) {
            ret = node;
            break;
        }
        node += 1;
    }
    return ret;
}

int LoadAllInvoiceTypeProperty(HDEV hDev)
{
    int nTypeCount = hDev->abSupportInvtype[0];
    int i = 0, nRet = RET_SUCCESS;
    struct DeviceInvoiceTypeInfo *node = hDev->lstHeadDiti;

    for (i = 0; i < nTypeCount; i++) {
        uint8 bInvType = hDev->abSupportInvtype[i + 1];
        if (hDev->bDeviceType == DEVICE_CNTAX) {
            if ((nRet = GetCntaxInvoiceTypeProperty(bInvType, hDev->hUSB, node)) < 0)
                break;
        } else if (hDev->bDeviceType == DEVICE_NISEC) {
            if ((nRet = GetNisecInvoiceTypeProperty(bInvType, hDev, node)) < 0)
                break;
        } else {
            nRet = ERR_DEVICE_UNSUPPORT;
            break;
        }
        node += 1;
    }
    return nRet;
}

int GetLastErrorType(enum ErrorCode bErrorCode)
{
    if ((bErrorCode >= ERR_TA_COMMON) && (bErrorCode < ErrorNET)) {
        return ErrorNET;
    } else if ((bErrorCode >= ERR_AISINO_COMMON) && (bErrorCode < ErrorUSB)) {
        return ErrorUSB;
    }
    return ErrorDev;
}

int SetLastError(HUSB hUSB, enum ErrorCode bErrorCode, char *msg, ...)
{
    if (hUSB == NULL)
        return ERR_GENERIC;
    va_list val = {0};
    int bErrorType = GetLastErrorType(bErrorCode);
    memset(hUSB->errinfo, 0, sizeof(hUSB->errinfo));
    switch (bErrorType) {
        case ErrorUSB:
            strcpy(hUSB->errinfo, "[USB] ");  // USB层错误
            break;
        case ErrorDev:
            strcpy(hUSB->errinfo, "[DEV] ");  //设备层错误
            break;
        case ErrorNET:
            strcpy(hUSB->errinfo, "[NET] ");  //网络层错误
            break;
        default:
            strcpy(hUSB->errinfo, "[UNK] ");  //未知
            break;
    }
    va_start(val, msg);
    hUSB->errcode = bErrorType;
    hUSB->nLastErrorCode = bErrorCode;
    vsnprintf(hUSB->szLastErrorDescription, sizeof(hUSB->szLastErrorDescription), msg, val);
    memcpy(hUSB->errinfo + 6, hUSB->szLastErrorDescription, strlen(hUSB->szLastErrorDescription));
    va_end(val);
    //找不到数据的错误太常见了，不打印
    if (hUSB->nLastErrorCode != ERR_NISEC_NO_DATA)
        _WriteLog(LL_FATAL, hUSB->szLastErrorDescription);
    return bErrorCode;
}

void ClearLastError(HUSB hUSB)
{
    if (hUSB == NULL)
        return;
    hUSB->errcode = 0;
    hUSB->nLastErrorCode = 0;
    memset(hUSB->errinfo, 0, sizeof(hUSB->errinfo));
    memset(hUSB->szLastErrorDescription, 0, sizeof(hUSB->szLastErrorDescription));
}

int GetHostAndPortFromEvUri(struct evhttp_uri *uri, char *szHostOut, int *nPortOut)
{
    const char *scheme = evhttp_uri_get_scheme(uri);
    if (scheme == NULL || (strcasecmp(scheme, "https") != 0 && strcasecmp(scheme, "http") != 0))
        return ERR_PARM_CHECK;
    const char *host = evhttp_uri_get_host(uri);
    if (host == NULL)
        return ERR_PARM_CHECK;
    strcpy(szHostOut, host);
    *nPortOut = evhttp_uri_get_port(uri);
    if (*nPortOut == -1) {
        *nPortOut = (strcasecmp(scheme, "http") == 0) ? 80 : 443;
    }
    return RET_SUCCESS;
}

mxml_node_t *NewByteStringToXmlNode(mxml_node_t *xmlNode, char *key, uint8 *flag)
{
    char szBuf[4] = "";
    sprintf(szBuf, "%02u", *flag);
    return mxmlNewText(mxmlNewElement(xmlNode, key), 0, szBuf);
}

bool IsPrivateIPv4(char *szIP)
{
    bool ret = true;
    do {
        char szIPBuf[64] = "";
        uint8 abIP[4];
        // 1.1.1.1 111.111.111.111
        if (!strchr(szIP, '.') || strlen(szIP) < 7 || strlen(szIP) > 15)
            break;
        strcpy(szIPBuf, szIP);
        if (4 != sscanf(szIPBuf, "%u.%u.%u.%u", (uint32 *)&abIP[0], (uint32 *)&abIP[1],
                        (uint32 *)&abIP[2], (uint32 *)&abIP[3]))
            break;
        // 0.0.0.0 127.0.0.1 10.0.0.0 255.0.0.0
        if (abIP[0] == 0 || abIP[0] == 10 || abIP[0] == 127 || abIP[0] == 255)
            break;
        // 169.254.0.0
        if (abIP[0] == 169 && abIP[1] == 254)
            break;
        // 192.168.0.0
        if (abIP[0] == 192 && abIP[1] == 168)
            break;
        // 172.16.0.0-172.31.255.255
        if ((abIP[0] == 172) && (abIP[1] >= 16) && (abIP[1] <= 31))
            break;
        // 100.64.0.0-100.127.255.255
        if ((abIP[0] == 100) && (abIP[1] >= 64) && (abIP[1] <= 127))
            break;
        //特殊地址 224.0.0.0-239.255.255.255 多点广播;240.0.0.0-255.255.255.254 E类保留
        if (abIP[0] >= 224 && abIP[0] < 255)
            break;
        ret = false;
    } while (false);
    return ret;
}

void Free2dArray(uint8 *array[], int nArrayNum)
{
    int i = 0;
    for (i = 0; i < nArrayNum; i++) {
        if (array[i] != NULL)
            free(array[i]);
    }
}

void Zyfplx2String(bool bStr2Byte, char *szStr, uint8 *pbZyfplxByte)
{
    if (bStr2Byte) {
        if (!strcmp(szStr, "01"))
            *pbZyfplxByte = ZYFP_NCP_XS;
        else if (!strcmp(szStr, "02"))
            *pbZyfplxByte = ZYFP_NCP_SG;
        else if (!strcmp(szStr, "03"))
            *pbZyfplxByte = ZYFP_XT_YCL;
        else if (!strcmp(szStr, "04"))
            *pbZyfplxByte = ZYFP_XT_CCP;
        else if (!strcmp(szStr, "05"))
            *pbZyfplxByte = ZYFP_SNY;
        else if (!strcmp(szStr, "08"))
            *pbZyfplxByte = ZYFP_CPY;
        else
            *pbZyfplxByte = ZYFP_NULL;
    } else {
        switch (*pbZyfplxByte) {
            case ZYFP_NCP_XS:
                strcpy(szStr, "01");
                break;
            case ZYFP_NCP_SG:
                strcpy(szStr, "02");
                break;
            case ZYFP_XT_YCL:
                strcpy(szStr, "03");
                break;
            case ZYFP_XT_CCP:
                strcpy(szStr, "04");
                break;
            case ZYFP_SNY:
                strcpy(szStr, "05");
                break;
            case ZYFP_CPY:
                strcpy(szStr, "08");
                break;
            default:
                strcpy(szStr, "");
                break;
        }
    }
}

//域名查询阻塞调用，目前代码中已无关联，仅仅作备份
int Domain2IP(char *host, char *ip)
{
    char buf[100];
    struct addrinfo hints;
    struct addrinfo *res = NULL, *curr;
    struct sockaddr_in *sa;
    int ret;
    if (!strcasecmp(host, "127.0.0.1") || !strcasecmp(host, "localhost")) {
        strcpy(ip, "127.0.0.1");
        return strlen(ip);
    }
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_flags = AI_CANONNAME;
    hints.ai_socktype = SOCK_DGRAM;
    ret = getaddrinfo(host, NULL, &hints, &res);
    if (ret != 0) {
        if (res)
            freeaddrinfo(res);
        return -1;
    }
    curr = res;
    while (curr && curr->ai_canonname) {
        sa = (struct sockaddr_in *)curr->ai_addr;
        sprintf(ip, "%s", inet_ntop(AF_INET, &sa->sin_addr.s_addr, buf, sizeof(buf)));
        curr = curr->ai_next;
    }
    freeaddrinfo(res);
    return strlen(ip);
}

//此处主要时socket超时，关键超时函数通过evhttp_connection_set_timeout设置
void SetSocketOption(int s, int nTimeoutSec)
{
    struct timeval timeo = {nTimeoutSec, 0};
    socklen_t len = sizeof(timeo);
    setsockopt(s, SOL_SOCKET, SO_SNDTIMEO, &timeo, len);
    setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &timeo, len);
}

bool CheckFphmRangeIncorrect(char *szFPHMHead, char *szFPHMTail, int nWantCount)
{
    long long lFphmHead = atoll(szFPHMHead);
    long long lFphmTail = atoll(szFPHMTail);
    if ((lFphmTail - lFphmHead + 1) < nWantCount)
        return false;
    return true;
}

int CalcAllocBuffLen(HFPXX hFpxx)
{
    int nSpxxNum = 0;
    struct Spxx *stp_Spxx = hFpxx->stp_MxxxHead;
    if (stp_Spxx) {
        while (stp_Spxx && (nSpxxNum < 2000)) {
            stp_Spxx = stp_Spxx->stp_next;
            nSpxxNum++;
        }
    }
    //(基础信息长度+(明细数量*1024))*Bin2Hex[2]
    int nBinLen = 8192 + nSpxxNum * (sizeof(struct Spxx) + 256);
    return nBinLen * 2;
}

void RemoveVersionSpecialChar(char *szSoftVersion, char *szDeviceDeclareVersion, char *old,
                              char *new)
{
    char *p = NULL;
    if ((p = StrReplace(szSoftVersion, old, new))) {
        strcpy(szSoftVersion, p);
        free(p);
    };
    if ((p = StrReplace(szDeviceDeclareVersion, old, new))) {
        strcpy(szDeviceDeclareVersion, p);
        free(p);
    };
}

//由于税局限制，不允许低版本覆盖高版本发票开具，因此需要校验设备是否已经在高版本开票软件使用
//返回值ture(符合条件)或者false，同时输出当前设备最新版本号(该版本号有可能因为IO错误返回空值)
bool CheckDeviceVersionMatch(HDEV hDev, char *szOutDeviceDeclareVersion)
{
    //先取各自版本号
    bool bRet = false;
    char szSoftVersion[32] = "", szDeviceDeclareVersion[32] = "", *p = NULL;
    bool bDeviceVersionGreaterthanSoftVersion = false;
    int nVersionCount = 4, i = 0;
    char abSoftWare[nVersionCount][768], abDevice[nVersionCount][768];
    strcpy(szOutDeviceDeclareVersion, "");

    switch (hDev->bDeviceType) 
    {
        case DEVICE_AISINO:
            GetAisinoKPRJVersion(hDev->nDriverVersionNum, szSoftVersion);
            if (AisinoGetTailInvVersion(hDev, szDeviceDeclareVersion))
                break;
            // V3.2.00.220507 aisino私有版本
            strcpy(szOutDeviceDeclareVersion, szDeviceDeclareVersion);
            bRet = true;
            break;
        case DEVICE_NISEC:
            if (NisecGetTailInvVersion(hDev, szDeviceDeclareVersion))
                break;
            // V2.0.49_ZS_20220630 税务总局版本
            strcpy(szSoftVersion, defTaxAuthorityVersionNisec);
            strcpy(szOutDeviceDeclareVersion, szDeviceDeclareVersion);
            bRet = true;
            break;
        case DEVICE_CNTAX:
            if (CntaxGetTailInvVersion(hDev, szDeviceDeclareVersion))
                break;
            // V1.0.17_ZS_20220429 税务总局cntax版本
            strcpy(szSoftVersion, defTaxAuthorityVersionCntax);
            strcpy(szOutDeviceDeclareVersion, szDeviceDeclareVersion);
            bRet = true;
            break;
        default:
            break;
    }
    logout(INFO, "TAXLIB", "税盘读取版本号", "税盘读取的到版本:%s 动态库读取到的版本:%s\r\n", szDeviceDeclareVersion,szSoftVersion);
    if (!bRet)
        return true;
    //替换掉非数字标识，统一为4等分版本号比较
    if ((p = StrReplace(szSoftVersion, "_ZS_", "."))) {  //正式
        strcpy(szSoftVersion, p);
        free(p);
    };
    if ((p = StrReplace(szSoftVersion, "_LS_", "."))) {  //临时
        strcpy(szSoftVersion, p);
        free(p);
    };
    if ((p = StrReplace(szDeviceDeclareVersion, "_ZS_", "."))) {  //正式
        strcpy(szDeviceDeclareVersion, p);
        free(p);
    };
    if ((p = StrReplace(szDeviceDeclareVersion, "_LS_", "."))) {  //临时
        strcpy(szDeviceDeclareVersion, p);
        free(p);
    };
    bRet = false;
    do {
        memset(abSoftWare, 0, sizeof(abSoftWare));
        memset(abDevice, 0, sizeof(abDevice));
        if (!strchr(szSoftVersion, '.') || !strchr(szDeviceDeclareVersion, '.'))
            break;
        if (GetSplitStringSimple(szSoftVersion, ".", abSoftWare, nVersionCount) != nVersionCount)
            break;
        if (GetSplitStringSimple(szDeviceDeclareVersion, ".", abDevice, nVersionCount) !=
            nVersionCount)
            break;
        for (i = 0; i < nVersionCount; i++) {
            // int memcmp(const void *str1, const void *str2, size_t n));
            // 如果返回值 < 0，则表示 str1 小于 str2。
            // 如果返回值 > 0，则表示 str2 小于 str1。
            // 如果返回值 = 0，则表示 str1 等于 str2
            //_WriteLog(LL_DEBUG, "abDevice[%d]=%s,abSoftWare[%d]=%s",i,abDevice[i],i,abSoftWare[i]);
            if(strlen(abDevice[i]) > strlen(abSoftWare[i]))
            {
                bDeviceVersionGreaterthanSoftVersion = true;
                break;
            }
            else if(strlen(abDevice[i]) < strlen(abSoftWare[i]))
            {
                bDeviceVersionGreaterthanSoftVersion = false;
                break;
            }
            else
            {
                if (memcmp(abDevice[i], abSoftWare[i], strlen(abDevice[i])) < 0) 
                {
                    bDeviceVersionGreaterthanSoftVersion = false;
                    break;
                }
                else if(memcmp(abDevice[i], abSoftWare[i], strlen(abDevice[i])) == 0)
                {
                    continue;
                }
                else
                {
                    bDeviceVersionGreaterthanSoftVersion = true;
                    break;
                }
            }
        }
    } while (false);
    bRet = !bDeviceVersionGreaterthanSoftVersion;
    _WriteLog(LL_DEBUG, "TailInvoiceVersion:%s SoftwareVersion:%s,Match:%d", szDeviceDeclareVersion,
              szSoftVersion, bRet);
    logout(INFO, "TAXLIB", "税盘比较版本号", "税盘读取的到版本:%s 动态库读取到的版本:%s,Match:%d", szDeviceDeclareVersion,szSoftVersion, bRet);
    return bRet;
}

////格式化获取软件所运行linux设备时间
//返回系统时间，标准格式20190101151515,szOut缓冲区至少16字节
void GetOSTime(char *szOutTime)
{
    memset(szOutTime, 0, 16);
    time_t now;
    struct tm *tm_now;
    time(&now);
    tm_now = localtime(&now);
    strftime(szOutTime, 16, "%04Y%02m%02d%02H%02M%02S", tm_now);  // format date and time.
}


int EvpCipherAlgorithm(bool bEncOrDec, const EVP_CIPHER *cipher, int padding, uint8 *pSrc,
                       int nSrcLen, uint8 *abKey, uint8 *pOutBuf)
{
    //加解密上下文
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    //加密算法初始化
    unsigned char iv[128] = {0};  // iv先设置空
    int nOutSize = 0;
    do {
        if (!EVP_CipherInit(ctx, cipher, abKey, iv, bEncOrDec))
            break;
        EVP_CIPHER_CTX_set_padding(ctx, padding);
        if (!EVP_CipherUpdate(ctx, pOutBuf, &nOutSize, pSrc, nSrcLen))
            break;
        //取出最后一块数据（需要填充的），或者是padding补充的数据
        int nPaddingSize = 0;
        if (!EVP_CipherFinal(ctx, pOutBuf + nOutSize, &nPaddingSize))
            break;
        nOutSize += nPaddingSize;
    } while (false);
    //释放上下文
    EVP_CIPHER_CTX_free(ctx);
    return nOutSize;
}

int SM2EncryptAlgorithm(uint8 *pSrc, int nSrcLen, char *szPubKeyHextString, uint8 *pBufOut,
                        int nBufOutSize)
{
    // pBufOut为如下asn1结构返回
    // 00000000  30 79 02 21 00 ee fc 6a  79 b5 94 27 89 10 6e e7  |0y.!...jy..'..n.|
    // 00000010  ba cb c2 2b ba cb 9c c0  09 e1 db f6 71 d0 97 87  |...+........q...|
    // 00000020  3f cc 65 0e de 02 20 2d  a4 a7 11 20 ff 7d 76 7c  |?.e... -... .}v||
    // 00000030  8c 39 fc ee c7 f0 6a 0f  f1 5d b6 af 5d 8e 49 da  |.9....j..]..].I.|
    // 00000040  59 0f d9 5a 69 9b b3 04  20 b9 45 a9 21 86 fa 32  |Y..Zi... .E.!..2|
    // 00000050  30 b4 02 66 1c 6c 09 ab  b8 85 7e 24 66 f1 14 2f  |0..f.l....~$f../|
    // 00000060  9b 97 59 25 aa 46 d6 30  f9 04 10 af bd e1 bc 06  |..Y%.F.0........|
    // 00000070  22 03 13 fa 4f de 77 62  8a 79 bc                 |"...O.wb.y.|
    int nRet = ERR_GENERIC;
    EC_KEY *ecKey = NULL;
    EVP_PKEY *pKey = NULL;
    EVP_PKEY_CTX *ctx = NULL;
    EC_GROUP *group = NULL;
    EC_POINT *pt = NULL;
    size_t nInOutBufOutSize = nBufOutSize;

    do {
        if (!pSrc || nSrcLen <= 0 || strlen(szPubKeyHextString) != 130 || !pBufOut ||
            nBufOutSize < 16)
            break;
        if (!(group = EC_GROUP_new_by_curve_name(NID_sm2)))
            break;
        if (!(pt = EC_POINT_hex2point(group, (const char *)szPubKeyHextString, NULL, NULL)))
            break;
        if (!(ecKey = EC_KEY_new()))
            break;
        if (!EC_KEY_set_group(ecKey, group) || !EC_KEY_set_public_key(ecKey, pt))
            break;
        if (!(pKey = EVP_PKEY_new()))
            break;
        if (!EVP_PKEY_set1_EC_KEY(pKey, ecKey) || !EVP_PKEY_set_alias_type(pKey, EVP_PKEY_SM2))
            break;
        if (!(ctx = EVP_PKEY_CTX_new(pKey, NULL)))
            break;
        if (EVP_PKEY_encrypt_init(ctx) <= 0)
            break;
        if (EVP_PKEY_encrypt(ctx, pBufOut, &nInOutBufOutSize, pSrc, nSrcLen) <= 0)
            break;
        nRet = nInOutBufOutSize;
    } while (false);
    if (pKey)
        EVP_PKEY_free(pKey);
    if (ctx)
        EVP_PKEY_CTX_free(ctx);
    if (ecKey)
        EC_KEY_free(ecKey);
    if (group)
        EC_GROUP_free(group);
    if (pt)
        EC_POINT_free(pt);
    return nRet;
}

// 输出肯定小于输入，因此直接覆盖输入
int SM2EncryptDataRemoveAsn1(uint8 *pSrcAsn1FormatSm2Result, int nSrcLen)
{
    //输入asn1格式，输出覆盖为sm2"密文"裸格式
    //裸密文格式解读
    //
    // SM2公钥格式：公钥：04|X|Y，共65字节；私钥格式：整数，共32字节。密文格式结构体为
    // typedef struct SM2Cipher_st {
    // ASN1_INTEGER *xCoordinate;                                // x分量(随机，非公钥)，32字节
    // ASN1_INTEGER *yCoordinate;                                // y分量(随机，非公钥)，32字节
    // ASN1_OCTET_STRING *hash;                                  // 杂凑值，32字节
    // ASN1_OCTET_STRING *cipherText;                            // 真密文，等于明文长度
    // } SM2Cipher;
    // 密文结构体x、y分量为C1，杂凑值为C3，密文数据为C2，新标准密文结构为C1C3C2。根据密文机构以及各个成员变量长度可以知道，明文长度等于密文结构体长度-96。
    int nskAsnTypeNum = 0, nChildRet = 0, nRet = ERR_GENERIC;
    ASN1_TYPE *asnTypeItem = NULL;
    ASN1_SEQUENCE_ANY *skAsnType = NULL;
    uint8 *pOut = pSrcAsn1FormatSm2Result;
    do {
        //如果需要asn1结构则推出，否则继续
        skAsnType =
            d2i_ASN1_SEQUENCE_ANY(NULL, (const unsigned char **)&pSrcAsn1FormatSm2Result, nSrcLen);
        if (4 != (nskAsnTypeNum = sk_ASN1_TYPE_num(skAsnType)))
            break;
        // 0
        asnTypeItem = sk_ASN1_TYPE_value(skAsnType, 0);
        if (V_ASN1_INTEGER != ASN1_TYPE_get(asnTypeItem))
            break;
        memcpy(pOut, asnTypeItem->value.integer->data, asnTypeItem->value.integer->length);
        nChildRet += asnTypeItem->value.integer->length;
        ASN1_TYPE_free(asnTypeItem);
        // 1
        asnTypeItem = sk_ASN1_TYPE_value(skAsnType, 1);
        if (V_ASN1_INTEGER != ASN1_TYPE_get(asnTypeItem))
            break;
        memcpy(pOut + nChildRet, asnTypeItem->value.integer->data,
               asnTypeItem->value.integer->length);
        nChildRet += asnTypeItem->value.integer->length;
        ASN1_TYPE_free(asnTypeItem);
        // 2
        asnTypeItem = sk_ASN1_TYPE_value(skAsnType, 2);
        if (V_ASN1_OCTET_STRING != ASN1_TYPE_get(asnTypeItem))
            break;
        memcpy(pOut + nChildRet, asnTypeItem->value.octet_string->data,
               asnTypeItem->value.octet_string->length);
        nChildRet += asnTypeItem->value.octet_string->length;
        ASN1_TYPE_free(asnTypeItem);
        // 3
        asnTypeItem = sk_ASN1_TYPE_value(skAsnType, 3);
        if (V_ASN1_OCTET_STRING != ASN1_TYPE_get(asnTypeItem))
            break;
        memcpy(pOut + nChildRet, asnTypeItem->value.octet_string->data,
               asnTypeItem->value.octet_string->length);
        nChildRet += asnTypeItem->value.octet_string->length;
        ASN1_TYPE_free(asnTypeItem);

        nRet = nChildRet;
    } while (false);
    if (skAsnType)
        sk_ASN1_TYPE_free(skAsnType);
    return nRet;
}

// nEncryptOrDecrypt  = 1 encrypt, = 0 decrypt
// 算法实现参考https://github.com/gityf/sm234
int EtaxSM4Algorithm(bool bEncOrDec, int padding, uint8 *pSrc, int nSrcLen, uint8 *abIV, uint8 *pOutBuf)
{
    return EvpCipherAlgorithm(bEncOrDec, EVP_sm4_ecb(), padding, pSrc, nSrcLen, abIV, pOutBuf);
}

// padding==0时，为无填充; ==1 PKCS7
int AesAlgorithm(bool bEncOrDec, const EVP_CIPHER *cipher, int padding, uint8 *todoData,
                 int nTodoDataSize, uint8 *inKey, uint8 *abOut)
{
    return EvpCipherAlgorithm(bEncOrDec, cipher, padding, todoData, nTodoDataSize, inKey, abOut);
}

int GzipDecompress(struct evbuffer *evbuf)
{
    struct evbuffer *tmp = NULL;
    z_stream stream = {};
    uint8 buffer[2048];
    int status, done = 0;
    do {
        if (!(tmp = evbuffer_new()))
            break;
        //此处为gzip数据解压，因此需要inflateInit2。普通zlib解压为inflateInit(&stream)?
        if (Z_OK != inflateInit2(&stream, MAX_WBITS + 16))
            break;
        stream.next_in = evbuffer_pullup(evbuf, -1);
        stream.avail_in = evbuffer_get_length(evbuf);
        do {
            stream.next_out = buffer;
            stream.avail_out = sizeof(buffer);
            status = inflate(&stream, Z_FULL_FLUSH);
            switch (status) {
                case Z_OK:
                    evbuffer_add(tmp, buffer, sizeof(buffer) - stream.avail_out);
                    break;
                case Z_STREAM_END:
                    evbuffer_add(tmp, buffer, sizeof(buffer) - stream.avail_out);
                    done = evbuffer_get_length(tmp);
                    break;
                default:
                    done = -1;
                    break;
            }
        } while (!done);
        inflateEnd(&stream);
        if (done <= 0)
            break;
        evbuffer_drain(evbuf, evbuffer_get_length(evbuf));
        evbuffer_add_buffer(evbuf, tmp);
    } while (false);
    if (tmp)
        evbuffer_free(tmp);
    return done;
}


// 该函数只生成16进制字符串函数,例如0F8BFBFF000906ED;
// GetRandomBytes不自己实现，使用Openssl的RAND_bytes,nMod=10 0-9，nMod=16 0-f
void GetRandHexString(char *szOut, int nInNeedLen, int nMod, char *szSeedString)
{
    if (!szSeedString) {
        //通过time随机数种子执行快点，经常重复，换成openssl随机数
        uint32 nSeed = 0;
        RAND_bytes((uint8 *)&nSeed, 4);
        srand(nSeed);
    } else
        srand(GetU32IdFromString(szSeedString));
    int i;
    for (i = 0; i < nInNeedLen; i++) {
        sprintf(szOut + i, "%X", rand() % nMod);
    }
}

void GetRandString(char *szOut, int nInNeedLen)
{
    const char allChar[] = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    int cnt, randNo, nAllCharSize;
    uint32 nSeed = 0;
    RAND_bytes((uint8 *)&nSeed, 4);
    srand(nSeed);
    nAllCharSize = sizeof(allChar) - 1;

    for (cnt = 0; cnt < nInNeedLen; cnt++) {
        randNo = rand() % nAllCharSize;
        *szOut = allChar[randNo];
        szOut++;
    }
    *szOut = '\0';
}

//返回值<0，设备IO出错,-100为USB SCSI协议返回的执行状态代码不为0（错误）
//返回值=0，无额外数据可读
//返回值>0，有可用数据长度;pRecvDataBuff存放可用数据缓存
int TaxMassStorageIo(HUSB hUsb, uint8 *pSendCmd, int nCmdLen, int UMSHeader_nDataTransferLen,
                     uint8 UMSHeader_direction, uint8 *pTransferData, int nTransferLen,
                     uint8 *pRecvDataBuff, int nRecvDataBuffLen)
{
    int nReallyTransferLen = UMSHeader_nDataTransferLen;
    if (0 != pTransferData)
        nReallyTransferLen = nTransferLen;
    int result =
        mass_storage_send_command(hUsb, pSendCmd, nCmdLen, UMSHeader_direction, nReallyTransferLen);
    if (result < 0)
        return -1;
    if (0 != pTransferData) {
        result = usb_bulk_write(hUsb, pTransferData, nTransferLen);
        if (result < 0)
            return -2;
    }
    memset(pRecvDataBuff, 0, nRecvDataBuffLen);
    result = usb_bulk_read(hUsb, pRecvDataBuff, nRecvDataBuffLen);
    if (result < 0)
        return -3;
    if (0x55 == pRecvDataBuff[0] && 0x53 == pRecvDataBuff[1] && 0x42 == pRecvDataBuff[2]) {
        ///先USBflag，后数据;出现USBFlag，说明执行已经完成，理论上已经可以直接取执行结果
        if (13 != result)
            return -3;
        uint8 bOK = pRecvDataBuff[12];
        if (0 != bOK)
            return -100;  //这个返回值不能改动
        // All CSW transfers shall be ordered with the LSB (byte 0) first (little endian).
        //具体参考第15页有关CSW介绍 https://www.usb.org/sites/default/files/usbmassbulk_10.pdf
        //此处实际因为小端序，此处早期aisino和nisec代码实现时可能有误，实际测试后发现此处实际税盘测试，uFollowDataLen全部为0，此处条件检测也无效
        // uint32 uFollowDataLen = ntohl(*(uint32 *)(pRecvDataBuff + 8));
        uint32 uFollowDataLen = *(uint32 *)(pRecvDataBuff + 8);
        if (uFollowDataLen > nRecvDataBuffLen)
            return -5;
        // 20230119 实际测试都是此处uFollowDataLen全部为0，直接返回
        //后续else流程无效。后续逻辑一般执行不到，仅仅保留代码
        if (0 == uFollowDataLen)
            return 0;
        memset(pRecvDataBuff, 0, nRecvDataBuffLen);
        result = usb_bulk_read(hUsb, pRecvDataBuff, nRecvDataBuffLen);
        if (result < 0)
            return -4;
        return result;
    } else {  //先数据已经被上面的read读掉了，剩余只剩下后USBflag(爱信诺)；百旺会出现read返回为0，执行结果没出来，需要等会再读一次
        uint8 abNew[512];
        int i = 0;
        for (i = 0; i < 30; i++) {  //返回结果超时3秒读取
            memset(abNew, 0, 512);
            int nNew = usb_bulk_read(hUsb, abNew, 512);
            if (nNew < 0)
                break;
            if (0 == nNew) {
                usleep(100000);  // 100ms
                continue;
            }
            if (13 != nNew)
                break;
            uint8 bOK = abNew[12];
            if (0 != bOK)
                return -100;  //这个返回值不能改动
            return result;
        }
        return -6;
    }
    return 0;
}