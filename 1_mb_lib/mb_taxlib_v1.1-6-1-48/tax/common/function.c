/*****************************************************************************
File name:   function.c
Description: ���ں��š�������UKeyͨ�ú�����һЩ��ҵ����ص�ͨ�ú���
Author:      Zako
Version:     1.0
Date:        2019.12
History:
20220109        ԭ���������ѳ���2000���У��������������ַ��ͱ���ת��
                �ȷ�����Լ1000�д�������character.c
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

////���㷨��ͬһ��˰�Ŷ�ο�Ʊ�����仯�����ܱ�˰�ּ�⵽��ʹ��˰�Ż����豸�������ӿ��Ա�֤����Ψһ
//�������ɵ�������ַ�����Ƿ�ע��ĵ�ַ����Ҫ����MAC����ǰ6λ�̶������Ҳ�����
// bSplitFormat 0==aisino;1==nisec;2==cntax
void GetNicMAC(char *szTaxID, char *szOut, uint8 bSplitFormat)
{
    //"1831BF" ��˶ǰ׺ASUSTek COMPUTER INC.
    if (bSplitFormat == 0) {  //-�ָaisino���;ʵ��aisino��ƱΪ"00-0C-29-C8-B4-9C"��ʽ
        char szBuf1[8] = "", szBuf2[8] = "", szBuf3[8] = "";
        strcpy(szOut, "18-31-BF");
        GetRandom(szBuf1, 2, 16, szTaxID + 1);
        GetRandom(szBuf2, 2, 16, szTaxID + 2);
        GetRandom(szBuf3, 2, 16, szTaxID + 3);
        sprintf(szOut + 8, "-%s-%s-%s", szBuf1, szBuf2, szBuf3);
    } else {  // ʵ��nisec mac="000C298205A9" ��ʽ�� ʵ��cntax��nisecһ��
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
    if(ret != 0){                               	                //wang 02 07 ���Խ��������������
        printf("MallocUSBSession libusb_init error%d!!!!!!!!!!!!!\r\n",ret);    
        free(husb);
        return NULL;
    }
    //libusb_set_debug(husb->context, LIBUSB_LOG_LEVEL_DEBUG);        //wang 02 07 ���Խ��������������
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
    // udev's cache 8192bytes ȫ��leak���������ѭ��leak
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

//------------------------------�������֧�� begin-----------------
//�߾���С������ضϲ����������������,szOutPrice������ԭ���뻺��Ҳ���Ե�������
void PriceRound(char *szPrice, int nRoundSize, char *szOutPrice)
{
    //���ַ���������
    if (!strlen(szPrice)) {
        strcpy(szOutPrice, "");
        return;
    }
    //������ʼ��
    mpfr_t mpf1, mpf2, mpf3;
    mpfr_init2(mpf1, 128);
    mpfr_init2(mpf2, 128);
    mpfr_init2(mpf3, 128);
    mpfr_set_str(mpf1, szPrice, 10, MPFR_RNDA);
    mpfr_set_d(mpf2, pow(10, nRoundSize), MPFR_RNDA);
    //�����������
    mpfr_mul(mpf3, mpf1, mpf2, MPFR_RNDA);  // mpf3=mpf1*mpf2
    mpfr_round(mpf1, mpf3);                 // mpf1=round(mpf3)
    mpfr_div(mpf3, mpf1, mpf2, MPFR_RNDA);  // mpf3=mpf1/mpf2
    mpfr_sprintf(szOutPrice, "%.*Rf", nRoundSize, mpf3);
    //����ͷ�
    mpfr_clear(mpf1);
    mpfr_clear(mpf2);
    mpfr_clear(mpf3);
    mpfr_free_cache();
}

//�߾��Ⱥ�˰�۸�ת����˰�۸�nRoundSizeΪָ����С����󾫶ȳ��ȣ�szOutPriceWithTaxΪ�����˰�۸񣻽ض�Ϊ���ݺ�һλ��������
int PriceRemoveTax(char *szDJ, char *szSLV, int nRoundSize, char *szOutPriceWithTax)
{
    if (!strlen(szDJ) || !strlen(szSLV))
        return -1;
    if (nRoundSize < 0 || nRoundSize > 100)
        return -2;
    // MPFR_RNDN���������루IEEE 754-2008�е��������룩;����������ȡż��
    // MPFR_RNDZ���������루IEEE 754-2008���������룩��ֱ����ȥ��
    // MPFR_RNDU��������������루IEEE 754-2008���������룩��
    // MPFR_RNDD������������루��IEEE 754-2008�������룩��
    // MPFR_RNDA:���㿪ʼ�������룻������������
    // MPFR_RNDF����ȷ���롣�˹���Ŀǰ���������С��ض�֧��
    //��ʼ������
    mpfr_t mpf1, mpf2, mpf3;
    mpfr_init2(mpf1, 128);
    mpfr_init2(mpf2, 128);
    mpfr_init2(mpf3, 128);
    //������˰����
    mpfr_set_str(mpf1, szDJ, 10, MPFR_RNDA);
    mpfr_set_str(mpf2, szSLV, 10, MPFR_RNDA);
    mpfr_set_str(mpf3, "1", 10, MPFR_RNDA);
    mpfr_add(mpf2, mpf2, mpf3, MPFR_RNDA);
    mpfr_div(mpf3, mpf1, mpf2, MPFR_RNDA);  // mpf3 �Ѿ��Ǹ߾�����˰����
    //��˰���۽���ָ��λ����������
    mpfr_set_d(mpf2, pow(10, nRoundSize), MPFR_RNDA);
    mpfr_mul(mpf1, mpf3, mpf2, MPFR_RNDA);  // mpf1=mpf3*mpf2
    mpfr_round(mpf3, mpf1);                 // mpf3=round(mpf1)
    mpfr_div(mpf1, mpf3, mpf2, MPFR_RNDA);  // mpf1=mpf3/mpf2
    mpfr_sprintf(szOutPriceWithTax, "%.*Rf", nRoundSize, mpf1);
    //����ͷ�
    mpfr_clear(mpf1);
    mpfr_clear(mpf2);
    mpfr_clear(mpf3);
    mpfr_free_cache();
    return 0;
}
//------------------------------�������֧�� finish--------------------

// 0 or 1
int ProgInitDestory(uint8 bInitOrDestory)
{
    if (bInitOrDestory) {
        //���Ȼ������
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
        //��ʼ��SSL��,���SSL����ʱ����֧�ֵ��㷨
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

//ʵ�ֲ�һ����ȫ�����þ���
// szStandTimeIn �����ʽyyyymmddhhmmss
int MyStrptime(char *szStandTimeIn, struct tm *tt0)
{
    if (strlen(szStandTimeIn) != 14)
        return -1;
    unsigned int nY = 0, nM = 0, nD = 0, nH = 0, nMn = 0, nS = 0;
    sscanf((char *)szStandTimeIn, "%04d%02d%02d%02d%02d%02d", &nY, &nM, &nD, &nH, &nMn, &nS);
    memset(tt0, 0, sizeof(struct tm));  //!!!�ر�ע�⣬tt0��ø���ֵ������ĳЩ�����׳���
    tt0->tm_sec = nS;
    tt0->tm_min = nMn;
    tt0->tm_hour = nH;
    tt0->tm_mday = nD;
    tt0->tm_mon = nM - 1;
    tt0->tm_year = nY - 1900;
    return 0;
}

//����û��������ֵ�ˣ�����������һ��Ҫ��
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

//�жϵ�ǰ�����Ƿ񳬳��ٽ�ֵ,����ֵΪ0�����Ƿ���������
int TimeOutMonthDeadline(char *szBaseTime, char *szTimeDev)
{
    if (strlen(szTimeDev) != 14 || strlen(szBaseTime) != 14)
        return -1;
    struct tm tmTime;
    MyStrptime(szTimeDev, &tmTime);
    time_t tTimeNow = mktime(&tmTime) + 300;  //�����Ԥ��5����

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


//�߾��ȼ��㣬��˰�۲���˰�۸���ת��
// bPriceWithTax==true �߾��Ⱥ�˰�۸�ת����˰�۸�;== false �߾��Ȳ���˰�۸�ת��˰�۸�
// nRoundSizeΪָ����С����󾫶ȳ��ȣ�szOutPriceΪ����۸񣻽ض�Ϊ���ݺ�һλ��������//old:PriceRemoveTax
int CalcTaxratePrice(bool bPriceWithTax, char *szDJ, char *szSLV, int nRoundSize, char *szOutPrice)
{
    if (!strlen(szDJ) || !strlen(szSLV))
        return -1;
    if (nRoundSize < 0 || nRoundSize > 100)
        return -2;
    // MPFR_RNDN���������루IEEE 754-2008�е��������룩;����������ȡż��
    // MPFR_RNDZ���������루IEEE 754-2008���������룩��ֱ����ȥ��
    // MPFR_RNDU��������������루IEEE 754-2008���������룩��
    // MPFR_RNDD������������루��IEEE 754-2008�������룩��
    // MPFR_RNDA:���㿪ʼ�������룻������������
    // MPFR_RNDF����ȷ���롣�˹���Ŀǰ���������С��ض�֧��
    //��ʼ������
    mpfr_t mpf1, mpf2, mpf3;
    mpfr_init2(mpf1, 128);
    mpfr_init2(mpf2, 128);
    mpfr_init2(mpf3, 128);
    //����Ŀ��۸�
    mpfr_set_str(mpf1, szDJ, 10, MPFR_RNDA);
    mpfr_set_str(mpf2, szSLV, 10, MPFR_RNDA);
    mpfr_set_str(mpf3, "1", 10, MPFR_RNDA);
    mpfr_add(mpf2, mpf2, mpf3, MPFR_RNDA);
    if (bPriceWithTax)  //��˰�۸�/(˰��+1)=����˰�۸�
        mpfr_div(mpf3, mpf1, mpf2, MPFR_RNDA);
    else  //����˰�۸�*(˰��+1)=��˰�۸�
        mpfr_mul(mpf3, mpf1, mpf2, MPFR_RNDA);
    // mpf3ΪĿ��۸�
    //Ŀ��۸����ָ��λ����������
    mpfr_set_d(mpf2, pow(10, nRoundSize), MPFR_RNDA);
    mpfr_mul(mpf1, mpf3, mpf2, MPFR_RNDA);  // mpf1=mpf3*mpf2
    mpfr_round(mpf3, mpf1);                 // mpf3=round(mpf1)
    mpfr_div(mpf1, mpf3, mpf2, MPFR_RNDA);  // mpf1=mpf3/mpf2
    mpfr_sprintf(szOutPrice, "%.*Rf", nRoundSize, mpf1);
    //����ͷ�
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

//��׼ʱ��ת������������ʽ
// 0=0x20 0x19 0x01 0x01 0x00 0x00 0xx00   7λ�����Ƹ�ʽ
// 1=20190101 00:00:00
// 2=2019-05-01 15:00:01
// 3=201901011200  ����
// 4=20190101 121212
// 5=2019-05-01   ��ȡ����
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
    //ǿ��˰�̿�Ʊ��˰�źͿ�Ʊ�����Ƹ��¿�Ʊ��Ϣ
    if (fpxx->isHzxxb == true && fpxx->hzxxbsqsm != 0) {
        _WriteLog(LL_INFO, "���򷽺�����Ϣ��ʱǿ���޸Ĺ�����Ϣ");
        strcpy(fpxx->gfsh, hDev->szCommonTaxID);
        strcpy(fpxx->gfmc, hDev->szCompanyName);
    } else {
        strcpy(fpxx->xfsh, hDev->szCommonTaxID);
        strcpy(fpxx->xfmc, hDev->szCompanyName);
    }
    strcpy(fpxx->jqbh, hDev->szDeviceID);
    //���±�Ҫ˰����Ϣ����Ʊ��Ϣ
    fpxx->kpjh = hDev->uICCardNo;
    fpxx->hDev = hDev;
    //��ʽ�����ָ�ʽʱ��
    UpdateFpxxAllTime(hDev->szDeviceTime, fpxx);
    //������Ϣ��ǿ�Ƹ��£���ֹ��ԭ��Ʊ��Ϣ�滻���ְ汾��Ϣ����ԭ��ƱΪ׼
    if (fpxx->isHzxxb == false) {
        // ssyf
        memset(fpxx->ssyf, 0, sizeof(fpxx->ssyf));
        strncpy(fpxx->ssyf, hDev->szDeviceTime, 6);
    }

#ifdef _NODEVICE_TEST
    //��import.json��ָ���汾�ͱ����汾��
#else
    //�Դ�����ֵǿ�Ƹ���
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
    //���ÿ��ߵ��ڵ�ǰ��Ͱ汾�ţ��������ױ�˰�ַ�������
    strcpy(fpxx->bmbbbh, DEF_BMBBBH_DEFAULT);
#endif
}

//��������*100,ȡ����ֵ
inline int GetFotmatJE100(char *je)
{
    int f = abs(round(atof(je) * 100));
    return f;
}

// GetFotmatJEdouble,ת�ַ���
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
        if (strcmp(oid, "1.2.840.113549.1.1.5") &&  //���ǩ���㷨�Ƿ���sha1rsa
            strcmp(oid, "1.2.156.10197.1.501"))  //���ǩ���㷨�Ƿ��ǻ���SM3��SM2ǩ���㷨
            break;

        //����openssl�ĵ�������Ŀ�ṹ��ָ��ṹ������free
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

//�����������Ʊ������Ŀ�Ʊʹ�õ�֤����ֵ��Ӧ�ñ�������,����ʱ��Ӧ�þ����ͷ�
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
        //�������е�֤��������
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
        return 0;  //û��֤���ʶ����
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
    //��β����
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

//��ӡ�û����豸�������豸���͡��豸�š�˰�š���˾���ƣ���������  �Լ�����ʹ��
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

//Ĭ��ֵΪ0������trueʱ��������ȫ�������ж��˳����ǵü�ʱ�ָ�Ϊfalse
//ͨ��αȫ�ֱ�����֪ͨ�����߳������ڴ���IO״̬���豸ȡ��ѭ��������״̬
void DevIOCtrl(HDEV hDev, bool bStatus)
{
    hDev->bBreakAllIO = bStatus;
}

//���ݾ�̬˰�����ж��Ƿ������
uint8 StaticCheckTrainDeviceByDeviceID(char *szDeviceID)
{
    // clang-format off
    char szGroup[][24] = {
	//"xxxxxxxxxxxx",//--ff--ffffffffffffff

	//��˰�̲�����
    "661551502671",//�ɰ�--������ѵ��ҵ--320102999999501
	"661551338399",//�ɰ�--������ѵ��ҵ--320901999999205
	"661551275480",//�ɰ�--������ѵ��ҵ--320901999999207
	"661401824869",//�ɰ�--������ѵ��ҵ--320901999999208	
	"661401813537",//�ɰ�--������ѵ��ҵ--320901999999209
	"661565671563",//Ʊͨ--500102192801051381--500102192801051381
	"661707894780",//--������ѵ��ҵ--339901999999209
	"661551401482",//--������ѵ��ҵ--440002999999075
	"661500152244",//--������ѵ��ҵ--320501999999098
	"661711123478",//--������ѵ��ҵ--339901999999502
	"661617935445",//--������ѵ��ҵ--150105999999501	
	"661512381378",//--test--91500020020718697X
	"661600082159",//--400102204710268392--400102204710268392
	"661612339468",//--422010201709012004--422010201709012004
	"661612339265",//--422010201709012004--422010201709012004
	"661840689777",//--5000002020112501A--5000002020112501A	
	"661565723941",//--500102201007206608--500102201007206608
	"661504734698",//--915001021210000032--915001021210000032
	"661504734680",//--915001021210000034--915001021210000034
    "661731475551",

	//˰���̲����̣�������ע�⣡����ע�⣡����ע�⣬�����Ĳ���������ʽ֤��ģ����ܼ����������ᵼ��qmcsΪ01a8��ǩ������
	"499000146849",//�ɰ�--1--123456789098765	
	"499000144018",//--rrr--111222333456111	
	"499000138591",//--test12345--500102010004050
	"499000156502",//Ʊͨ--�����û�--91142010202002279F
	"499000156570",//Ʊͨ--�����û�--91142010202002270F      //����
	"499000146857",//--����������ҵ--91510198766347M
	"539900210491",//--�����û�7891--913201234567891
	"499000132869",//--�����û�3176--500102010003176	
	"539900211021",//--����������û�226--500102010000226
	"499000115225",//--����������û�1412--500102010001412	
	"499000135074",//--����������û�3697--500102010003697
	"499000135082",//--����������û�3698--500102010003698		
	"499000135091",//--����������û�3699--500102010003699
	"499000136051",//--����������û�3782--500102010003782
	"499000135947",//--����������û�3792--500102010003792
	"539990020041",//--����һ��������û�2--903201111111112
	"499000134531",//--�Ϻ���������3643--500102010003643
	"499000160309",//--��Ʊ����һ���--110101201801010109
	"499000152421",//--��Ʊ������ʮ��--110101201707010054	
	"499000140033",//--����--500102010088889
	"499000140025",//�ɰ�--����2019--9131000032955577SC
	"499000143277",//--����3277--410104999000143277
	"499000107583",//--�����û�0424--500102010000424
	"499000155614",//--������5614--91500102000005614Y
	"499000155690",//--������5690--91500102010005690X
	"499000140711",//�ɰ�--���ݰ������ԱϽ�1--91520103063000BJ1
	"499000143584",//--��ֵ˰��Ʊ����3266--400811888888883266
	"499000166840",//--���ļ���˰���̲���02--151000000000000123
	"889999999583",//--���ݰ���������ͭ�ʷֹ�˾--500102019999999
	"499000139729",//--���ݰ������ײ����̼�άǿ--520681234500005
    //"929910415654",//���ϵ��ӷ�Ʊ��������ƽ̨����3 914101050260260263 #������Ϊ�����̣�����qmcsΪ01a8��ǩ������
	"499000177611",//--���ݰ�������8--

	//ukey������
	"587003206501", //�ɰ�--���϶�����ó���޹�˾--91410102075443367X
	"587003107634"//�ɰ�--���ش���Ƥ���������ι�˾--410526582890516
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

//����ֵΪʣ�ೢ�Դ˴���<0����������ִ�У����Ժ��Է���ֵ
int PasswordNotice(bool bDeviceOrCert, char *szLastErrorDescription)
{
    char szTmpBuf[16] = "";
    char *pErr = NULL;
    if (!strlen(szLastErrorDescription))
        return 0;
    if (!(pErr = strstr(szLastErrorDescription, "ECode:0x")))
        return 0;
    pErr += 8;
    // ukey��û��ʵ�̲����Ƿ�Ϊ��ֵ������������������̼ǵò�����
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
            strcpy(hUSB->errinfo, "[USB] ");  // USB�����
            break;
        case ErrorDev:
            strcpy(hUSB->errinfo, "[DEV] ");  //�豸�����
            break;
        case ErrorNET:
            strcpy(hUSB->errinfo, "[NET] ");  //��������
            break;
        default:
            strcpy(hUSB->errinfo, "[UNK] ");  //δ֪
            break;
    }
    va_start(val, msg);
    hUSB->errcode = bErrorType;
    hUSB->nLastErrorCode = bErrorCode;
    vsnprintf(hUSB->szLastErrorDescription, sizeof(hUSB->szLastErrorDescription), msg, val);
    memcpy(hUSB->errinfo + 6, hUSB->szLastErrorDescription, strlen(hUSB->szLastErrorDescription));
    va_end(val);
    //�Ҳ������ݵĴ���̫�����ˣ�����ӡ
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
        //�����ַ 224.0.0.0-239.255.255.255 ���㲥;240.0.0.0-255.255.255.254 E�ౣ��
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

//������ѯ�������ã�Ŀǰ���������޹���������������
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

//�˴���Ҫʱsocket��ʱ���ؼ���ʱ����ͨ��evhttp_connection_set_timeout����
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
    //(������Ϣ����+(��ϸ����*1024))*Bin2Hex[2]
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

//����˰�����ƣ�������Ͱ汾���Ǹ߰汾��Ʊ���ߣ������ҪУ���豸�Ƿ��Ѿ��ڸ߰汾��Ʊ���ʹ��
//����ֵture(��������)����false��ͬʱ�����ǰ�豸���°汾��(�ð汾���п�����ΪIO���󷵻ؿ�ֵ)
bool CheckDeviceVersionMatch(HDEV hDev, char *szOutDeviceDeclareVersion)
{
    //��ȡ���԰汾��
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
            // V3.2.00.220507 aisino˽�а汾
            strcpy(szOutDeviceDeclareVersion, szDeviceDeclareVersion);
            bRet = true;
            break;
        case DEVICE_NISEC:
            if (NisecGetTailInvVersion(hDev, szDeviceDeclareVersion))
                break;
            // V2.0.49_ZS_20220630 ˰���ְܾ汾
            strcpy(szSoftVersion, defTaxAuthorityVersionNisec);
            strcpy(szOutDeviceDeclareVersion, szDeviceDeclareVersion);
            bRet = true;
            break;
        case DEVICE_CNTAX:
            if (CntaxGetTailInvVersion(hDev, szDeviceDeclareVersion))
                break;
            // V1.0.17_ZS_20220429 ˰���ܾ�cntax�汾
            strcpy(szSoftVersion, defTaxAuthorityVersionCntax);
            strcpy(szOutDeviceDeclareVersion, szDeviceDeclareVersion);
            bRet = true;
            break;
        default:
            break;
    }
    logout(INFO, "TAXLIB", "˰�̶�ȡ�汾��", "˰�̶�ȡ�ĵ��汾:%s ��̬���ȡ���İ汾:%s\r\n", szDeviceDeclareVersion,szSoftVersion);
    if (!bRet)
        return true;
    //�滻�������ֱ�ʶ��ͳһΪ4�ȷְ汾�űȽ�
    if ((p = StrReplace(szSoftVersion, "_ZS_", "."))) {  //��ʽ
        strcpy(szSoftVersion, p);
        free(p);
    };
    if ((p = StrReplace(szSoftVersion, "_LS_", "."))) {  //��ʱ
        strcpy(szSoftVersion, p);
        free(p);
    };
    if ((p = StrReplace(szDeviceDeclareVersion, "_ZS_", "."))) {  //��ʽ
        strcpy(szDeviceDeclareVersion, p);
        free(p);
    };
    if ((p = StrReplace(szDeviceDeclareVersion, "_LS_", "."))) {  //��ʱ
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
            // �������ֵ < 0�����ʾ str1 С�� str2��
            // �������ֵ > 0�����ʾ str2 С�� str1��
            // �������ֵ = 0�����ʾ str1 ���� str2
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
    logout(INFO, "TAXLIB", "˰�̱Ƚϰ汾��", "˰�̶�ȡ�ĵ��汾:%s ��̬���ȡ���İ汾:%s,Match:%d", szDeviceDeclareVersion,szSoftVersion, bRet);
    return bRet;
}

////��ʽ����ȡ���������linux�豸ʱ��
//����ϵͳʱ�䣬��׼��ʽ20190101151515,szOut����������16�ֽ�
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
    //�ӽ���������
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    //�����㷨��ʼ��
    unsigned char iv[128] = {0};  // iv�����ÿ�
    int nOutSize = 0;
    do {
        if (!EVP_CipherInit(ctx, cipher, abKey, iv, bEncOrDec))
            break;
        EVP_CIPHER_CTX_set_padding(ctx, padding);
        if (!EVP_CipherUpdate(ctx, pOutBuf, &nOutSize, pSrc, nSrcLen))
            break;
        //ȡ�����һ�����ݣ���Ҫ���ģ���������padding���������
        int nPaddingSize = 0;
        if (!EVP_CipherFinal(ctx, pOutBuf + nOutSize, &nPaddingSize))
            break;
        nOutSize += nPaddingSize;
    } while (false);
    //�ͷ�������
    EVP_CIPHER_CTX_free(ctx);
    return nOutSize;
}

int SM2EncryptAlgorithm(uint8 *pSrc, int nSrcLen, char *szPubKeyHextString, uint8 *pBufOut,
                        int nBufOutSize)
{
    // pBufOutΪ����asn1�ṹ����
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

// ����϶�С�����룬���ֱ�Ӹ�������
int SM2EncryptDataRemoveAsn1(uint8 *pSrcAsn1FormatSm2Result, int nSrcLen)
{
    //����asn1��ʽ���������Ϊsm2"����"���ʽ
    //�����ĸ�ʽ���
    //
    // SM2��Կ��ʽ����Կ��04|X|Y����65�ֽڣ�˽Կ��ʽ����������32�ֽڡ����ĸ�ʽ�ṹ��Ϊ
    // typedef struct SM2Cipher_st {
    // ASN1_INTEGER *xCoordinate;                                // x����(������ǹ�Կ)��32�ֽ�
    // ASN1_INTEGER *yCoordinate;                                // y����(������ǹ�Կ)��32�ֽ�
    // ASN1_OCTET_STRING *hash;                                  // �Ӵ�ֵ��32�ֽ�
    // ASN1_OCTET_STRING *cipherText;                            // �����ģ��������ĳ���
    // } SM2Cipher;
    // ���Ľṹ��x��y����ΪC1���Ӵ�ֵΪC3����������ΪC2���±�׼���ĽṹΪC1C3C2���������Ļ����Լ�������Ա�������ȿ���֪�������ĳ��ȵ������Ľṹ�峤��-96��
    int nskAsnTypeNum = 0, nChildRet = 0, nRet = ERR_GENERIC;
    ASN1_TYPE *asnTypeItem = NULL;
    ASN1_SEQUENCE_ANY *skAsnType = NULL;
    uint8 *pOut = pSrcAsn1FormatSm2Result;
    do {
        //�����Ҫasn1�ṹ���Ƴ����������
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
// �㷨ʵ�ֲο�https://github.com/gityf/sm234
int EtaxSM4Algorithm(bool bEncOrDec, int padding, uint8 *pSrc, int nSrcLen, uint8 *abIV, uint8 *pOutBuf)
{
    return EvpCipherAlgorithm(bEncOrDec, EVP_sm4_ecb(), padding, pSrc, nSrcLen, abIV, pOutBuf);
}

// padding==0ʱ��Ϊ�����; ==1 PKCS7
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
        //�˴�Ϊgzip���ݽ�ѹ�������ҪinflateInit2����ͨzlib��ѹΪinflateInit(&stream)?
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


// �ú���ֻ����16�����ַ�������,����0F8BFBFF000906ED;
// GetRandomBytes���Լ�ʵ�֣�ʹ��Openssl��RAND_bytes,nMod=10 0-9��nMod=16 0-f
void GetRandHexString(char *szOut, int nInNeedLen, int nMod, char *szSeedString)
{
    if (!szSeedString) {
        //ͨ��time���������ִ�п�㣬�����ظ�������openssl�����
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

//����ֵ<0���豸IO����,-100ΪUSB SCSIЭ�鷵�ص�ִ��״̬���벻Ϊ0������
//����ֵ=0���޶������ݿɶ�
//����ֵ>0���п������ݳ���;pRecvDataBuff��ſ������ݻ���
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
        ///��USBflag��������;����USBFlag��˵��ִ���Ѿ���ɣ��������Ѿ�����ֱ��ȡִ�н��
        if (13 != result)
            return -3;
        uint8 bOK = pRecvDataBuff[12];
        if (0 != bOK)
            return -100;  //�������ֵ���ܸĶ�
        // All CSW transfers shall be ordered with the LSB (byte 0) first (little endian).
        //����ο���15ҳ�й�CSW���� https://www.usb.org/sites/default/files/usbmassbulk_10.pdf
        //�˴�ʵ����ΪС���򣬴˴�����aisino��nisec����ʵ��ʱ��������ʵ�ʲ��Ժ��ִ˴�ʵ��˰�̲��ԣ�uFollowDataLenȫ��Ϊ0���˴��������Ҳ��Ч
        // uint32 uFollowDataLen = ntohl(*(uint32 *)(pRecvDataBuff + 8));
        uint32 uFollowDataLen = *(uint32 *)(pRecvDataBuff + 8);
        if (uFollowDataLen > nRecvDataBuffLen)
            return -5;
        // 20230119 ʵ�ʲ��Զ��Ǵ˴�uFollowDataLenȫ��Ϊ0��ֱ�ӷ���
        //����else������Ч�������߼�һ��ִ�в�����������������
        if (0 == uFollowDataLen)
            return 0;
        memset(pRecvDataBuff, 0, nRecvDataBuffLen);
        result = usb_bulk_read(hUsb, pRecvDataBuff, nRecvDataBuffLen);
        if (result < 0)
            return -4;
        return result;
    } else {  //�������Ѿ��������read�����ˣ�ʣ��ֻʣ�º�USBflag(����ŵ)�����������read����Ϊ0��ִ�н��û��������Ҫ�Ȼ��ٶ�һ��
        uint8 abNew[512];
        int i = 0;
        for (i = 0; i < 30; i++) {  //���ؽ����ʱ3���ȡ
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
                return -100;  //�������ֵ���ܸĶ�
            return result;
        }
        return -6;
    }
    return 0;
}