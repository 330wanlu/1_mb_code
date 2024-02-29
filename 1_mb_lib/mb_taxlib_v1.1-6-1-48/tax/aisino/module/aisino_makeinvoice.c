/*****************************************************************************
File name:   aisino_makeinvoice.c
Description: aisno��Ʊ��ش���
Author:      Zako
Version:     1.0
Date:        2019.10
History:
20200901     ��buildinv.c�ļ�����������ϲ�
20220222     ��Ʊ���220107�汾���룬��Ҫ�ǿ�Ʊ�ַ����Ķ��룬����#$`^�ָ��ַ�����
             �ϴ��ģ�Դ�ǰ���뿪Ʊ�ַ��������߼��Ƚϸ�������Ĳ��ֽ����и����;����߼�
20220414     ��20220401�汾�°汾���¼����RSA��Կ���ܸ���,�淶����Ʊ�˶�Ҫ�ص�
*****************************************************************************/
// һ�ŷ�Ʊ�Ĺؼ����������¼��㣬��Ҫ����ֽں˶ԣ����κ�һ���ֽڶ��ᵼ�����⣬������Ը���'aisino��Ʊ�˶�Ҫ��'����
#define make_invoice_c
#include "aisino_makeinvoice.h"

//˰�ʽṹ��
struct st_Slv {
    struct st_Slv *stp_next;
    char ca_slv[10];   //˰��
    char ca_zje[100];  //���
    char ca_zse[100];  //��˰��
};

//����˰�ʽ��
int InsertDict(struct st_Slv *pst_root, char *cp_slv, char *cp_je, char *cp_se)
{
    char ca_slv[10] = {0};
    if (pst_root == NULL || cp_slv == NULL || cp_je == NULL)
        return 1;
    struct st_Slv *stp_Tmp = pst_root, *stp_InsertNode = calloc(1, sizeof(struct st_Slv));
    if (stp_InsertNode == 0)
        return 1;
    //˰��ת��
    double d_slv = atof(cp_slv);
    if (d_slv != 0) {  //�Ƴ�˰�ʽ�β0��00������0.130,0.100
        sprintf(ca_slv, "%.3f", d_slv);
        MemoryFormart(MF_TRANSLATE_TAILZERO, ca_slv, strlen(ca_slv));
    } else  //������˰�ʱ��������λС��
        strcpy(ca_slv, "0.00");
    //��Ҫ������
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

//�������˰��
int AddDict(struct st_Slv *pst_root, char *cp_slv, char *pc_je, char *pc_se)
{
    double d_je = 0, d_zje = 0, d_se = 0, d_zse = 0;
    if (pst_root == NULL || cp_slv == NULL || pc_je == NULL)
        return 1;
    //����ͷ���
    struct st_Slv *stp_Tmp = pst_root->stp_next;
    while (stp_Tmp) {
        if (strcmp(stp_Tmp->ca_slv, cp_slv) == 0 || atof(stp_Tmp->ca_slv) == atof(cp_slv)) {
            //�ܽ��
            d_je = atof(pc_je);
            d_zje = atof(stp_Tmp->ca_zje);
            d_zje += d_je;
            sprintf(stp_Tmp->ca_zje, "%.2f", d_zje);
            //��˰��
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

//�ж�˰���Ƿ��Ѿ�����
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

//�˴��Ѿ��൱�ײ������ƴ�գ���˾�����Ҫ��������У��͵����������������import.c�н���
char *FillSPXX(HFPXX fpxx, struct Spxx *stp_Spxx, int nInvoiceNum)
{
    struct STSPXXv3 *spxx = (struct STSPXXv3 *)malloc(sizeof(struct STSPXXv3));
    char ca_spmc[256] = {0};
    memset(spxx, 0x20, sizeof(struct STSPXXv3));
    strcat(ca_spmc, stp_Spxx->spsmmc);
    strcat(ca_spmc, stp_Spxx->spmc);
    //��Ʒ����
    UTF8CopyGBK((uint8 *)spxx->SPMC, (uint8 *)ca_spmc, strlen(ca_spmc), sizeof(spxx->SPMC));
    //����ͺ�
    UTF8CopyGBK((uint8 *)spxx->GGXH, (uint8 *)stp_Spxx->ggxh, strlen(stp_Spxx->ggxh),
                sizeof(spxx->GGXH));
    //������λ
    UTF8CopyGBK((uint8 *)spxx->JLDW, (uint8 *)stp_Spxx->jldw, strlen(stp_Spxx->jldw),
                sizeof(spxx->JLDW));
    //����
    UTF8CopyGBK((uint8 *)spxx->SL, (uint8 *)stp_Spxx->sl, strlen(stp_Spxx->sl), sizeof(spxx->SL));
    //����
    UTF8CopyGBK((uint8 *)spxx->DJ, (uint8 *)stp_Spxx->dj, strlen(stp_Spxx->dj), sizeof(spxx->DJ));
    //���
    UTF8CopyGBK((uint8 *)spxx->JE, (uint8 *)stp_Spxx->je, strlen(stp_Spxx->je), sizeof(spxx->JE));
    //˰��
    UTF8CopyGBK((uint8 *)spxx->SLV, (uint8 *)stp_Spxx->slv, strlen(stp_Spxx->slv),
                sizeof(spxx->SLV));
    //˰��
    UTF8CopyGBK((uint8 *)spxx->SE, (uint8 *)stp_Spxx->se, strlen(stp_Spxx->se), sizeof(spxx->SE));
    //���
    char szTmp[16];
    sprintf(szTmp, "%d", nInvoiceNum);
    UTF8CopyGBK((uint8 *)spxx->NUM, (uint8 *)szTmp, strlen(szTmp), sizeof(spxx->NUM));
    // FPHXZ ��Ʊ������ import.c GetAvailImportData���ж�fphxz������������Ϊ��
    UTF8CopyGBK((uint8 *)spxx->FPHXZ, (uint8 *)stp_Spxx->fphxz, strlen(stp_Spxx->fphxz),
                sizeof(spxx->FPHXZ));
    //��˰�۱�־    
	if (strlen(stp_Spxx->hsjbz) == 0 || !strcmp(stp_Spxx->hsjbz, "0"))
		strcpy(stp_Spxx->hsjbz, "0");
	else
		strcpy(stp_Spxx->hsjbz, "1");
	// 20220908���²��Խ��
	// ���ӷ�Ʊ��˰����ʱ����˰�۱�־Ϊ1,ר��ƱΪ0,�����ڿ�Ʊ��������˰������񣬴˴���־��Զ�̶�
	if ((!strcmp(stp_Spxx->se, "0.00") &&
		IsAisino3Device(fpxx->hDev->nDriverVersionNum))) {
		//�ɱ�׼3%������˰��Ʊ���ߣ���˰�ʷ�Ʊ�����ԣ�
		if (fpxx->fplx_aisino == FPLX_AISINO_DZFP)
			strcpy(stp_Spxx->hsjbz, "1");
		else
			strcpy(stp_Spxx->hsjbz, "0");
	}
    UTF8CopyGBK((uint8 *)spxx->HSJBZ, (uint8 *)stp_Spxx->hsjbz, strlen(stp_Spxx->hsjbz),
                sizeof(spxx->HSJBZ));
    //��Ʒ˰Ŀ(spsm) �������(flbm) ��Ʒ���(spbh);�˴���Ʊ�ַ����������ΪЭ������Ʒ���
    UTF8CopyGBK((uint8 *)spxx->FLBM, (uint8 *)stp_Spxx->spbh, strlen(stp_Spxx->spbh),
                sizeof(spxx->FLBM));
    //��Ʒ���(�˴���Ʒ������,Ŀǰ����)
    UTF8CopyGBK((uint8 *)spxx->SPBH, (uint8 *)"", 0, sizeof(spxx->SPBH));
    //˰���Ż�
    UTF8CopyGBK((uint8 *)spxx->XSYH, (uint8 *)stp_Spxx->xsyh, strlen(stp_Spxx->xsyh),
                sizeof(spxx->XSYH));
    //�Ż�˵��
    UTF8CopyGBK((uint8 *)spxx->YHSM, (uint8 *)stp_Spxx->yhsm, strlen(stp_Spxx->yhsm),
                sizeof(spxx->YHSM));
    //��˰�ʱ�ʶ
    UTF8CopyGBK((uint8 *)spxx->LSLVBS, (uint8 *)stp_Spxx->lslvbs, strlen(stp_Spxx->lslvbs),
                sizeof(spxx->LSLVBS));
    return (char *)spxx;
}

struct st_Slv *GetTaxRateInfoFromString(HFPXX fpxx)
{
    struct st_Slv *stp_Slvzje = NULL;
    if ((stp_Slvzje = calloc(1, sizeof(struct st_Slv))) == NULL)
        return NULL;
    //��Ʊ��ϸ�а�˰�ʻ����ܽ���˰��
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
    ///��Ʊ������죬Ŀǰ��������Ʊ�ֽԴ��ڣ����ӷ�Ʊ˰��Ϊ0.01ʱ����Ʊ���Ϊ0.01\0.06���ҷ�Ϊ0.010\0.060
    //���ⲻ�����ҽ����ײ��Ը��֣������ɰط����ϲ����޸�
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
    //--���ʱ�����嵥��Ʊ���߶�˰�ʷ�Ʊ����˰��
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

//ͬʱ�����������汾��Ϣ
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
    //ר�÷�Ʊ_��Ʒ��
    if (fpxx->zyfpLx == ZYFP_CPY) {
        //�˴�1(��Ʒ��)����һ���ж�δ֪
        if ((1) && (fpxx->fplx_aisino == FPLX_AISINO_ZYFP) && (fpxx->isRed)) {
            memset(szBuf, 0, nBufSize);
            strcat((char *)szBuf, "VA");
        } else {
            memset(szBuf, 0, nBufSize);
            strcat((char *)szBuf, "V9");
        }
    }
    //ר�÷�Ʊ_ϡ��
    if (fpxx->fplx_aisino == FPLX_AISINO_ZYFP && fpxx->zyfpLx == ZYFP_XT_JSJHJ) {
        memset(szBuf, 0, nBufSize);
        strcat((char *)szBuf, "VE");
    } else if (fpxx->fplx_aisino == FPLX_AISINO_ZYFP && fpxx->zyfpLx == ZYFP_XT_CPJGF) {
        memset(szBuf, 0, nBufSize);
        strcat((char *)szBuf, "VF");
    }
    //�������FLBM�����汾��(ʵ����ȫ������)
    //�հ������ޱ����汾��
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
    //С������64K����������0x400K��ʣ��Ӧ�ø������µĽṹ���С����;����˰��201907����Ҫ������2000
    int num5 = 0x400000, nMaxInvoiceCount = 0, i_InvoiceNum = 1;
    struct Spxx *stp_Spxx = NULL;
    nMaxInvoiceCount = (num5 - nGBKOutBuffIndex) / sizeof(struct STSPXXv3);
    //�Ƿ����嵥
    if (strcmp(fpxx->qdbj, "Y") == 0) {
        //�����Ʒ��ϸͷ��stp_Spxx����ΪNULL
        if (!(stp_Spxx = fpxx->stp_MxxxHead))
            return ERR_BUF_CHECK;
        while (stp_Spxx && (i_InvoiceNum < nMaxInvoiceCount)) {
            int nIndex = i_InvoiceNum - 1;
            if (1 == i_InvoiceNum)
                nIndex = 1;  //�и���ͷ
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

// 20220401aisino�¼����㷨,ִ�гɹ���dstBuf����Ϊ������0x80��������
int RsaEncryptWithStaticPubkey(uint8 *pSrcBuf, int nSrcSize, uint8 *pDstBuf, int nDstSize)
{
    // Note:���Դ˶δ��룬������ϤRSA�ǶԳƼ�����ϸ���̼����ܹ������ڴ沼�ַ�ʽ�������ڴ�仯λ��
    // 1)��Ʊ��������ʽ
    //��Ʊ����˴���һ��RSA1024��Կ���ܹ��̣���ԿΪ��̬��Կ����ʽΪXML��Կ��ʽ���������¸�ʽ������ֱ������RSA֤��ת��XML->PEM��ʽת��Ϊopenssl
    // pemͨ�ø�ʽ <RSAKeyValue><Modulus>xA7S...</Modulus><Exponent>AQAB</Exponent>/RSAKeyValue>
    // openssl rsautl -encrypt -in input.txt -inkey pubkey.key -pubin -out hello.en
    // 2)RSA+PKCS1����㷨�ж�����
    //�Կ�Ʊ������ܳ��Ƚ����޸ģ����޸ĳ�����117�ֽ�ʱ������c++�쳣��128-117=11�ֽڷ��ϣ���PKCS#1�����padding��ռ����11���ֽڶ�Ӧ
    //�������pkcs#1�������
    //����ʱ����ֽ�����֤��λ��/8-11��2048λ��֤�飬֧�ֵ��������ֽ���2048/8-11=245
    //����:.NET�е�RSA�����㷨Ϊ����߰�ȫ�ԣ��ڴ���������ǰҪ���һЩ���������ˣ�ʹ��.NET�е�RSA�����㷨һ��������117�ֽ����ݣ�����117�ֽ���Ҫ��ֳɶ�ηֱ�������������������������ܺ�õ�һ������Ϊ128�ֽڵļ������ݡ�
    // 3)���RSA�淶
    //��������(BlockType)Ϊ02��ʹ��α�������ǰ�����ֽڽ�����䣬������ͬ���룬ÿ�μ��ܽ�����᲻һ����
    //��˽Կ������£�ͨ����������ܿ��ǣ��ù̶��ֽ����BT00/01����Կ���ڿͻ��ˣ���α��������BT02��
    //�ο�:https://www.jianshu.com/p/e300f7735c87
    // 4)����ֻ�й�Կ���޷��⿪���ܺ�����ݣ��޷�ֱ����֤����������ȷ�ԣ���˲��ù̶������䣬�����ҷ��������ݵĲ��԰취
    //���������ٷ���Ϊʹ��crypto++���ܿ��ϰ汾��C++�汾�㷨�⣩��pad�㷨PKCS_EncryptionPaddingScheme::Pad
    // opensslʹ��pad�㷨Ϊ RSA_padding_add_PKCS1_type_2
    //��opensslԴ����RSA_padding_add_PKCS1_type_2��memcpy֮������gdb�޸��ڴ�����������������в���
    /// tassl111bd/src/crypto/rsa/rsa_pk1.c��117�и��� RSA_padding_add_PKCS1_type_2������β��
    //-exec set {char[85]}(p-85) =
    //"000000000000000000000000000000000000000000000000000000000000000000000000000000000000"

    //˰�ָ����ľ�̬�ı���Կ֤��
    char szPubKey[] = "-----BEGIN PUBLIC KEY-----\n"
                      "MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQCvhgGyjmdmY1pjylEkVOFJB0+v\n"
                      "WbnhWy/+FQOBFJYsBwYV+gBXfeaHx9KOHXKn2s4uxZ0cPbMHvAvg51p6aRqGxpWj\n"
                      "E+JrisQaVQTcatyHMAPHgeiY61ItsiZfXgOsIOr8kh/NgSB+A0YE+QFpEuXgXuO+\n"
                      "EDhHsc55dG9NJKE8xQIDAQAB\n"
                      "-----END PUBLIC KEY-----";
    RSA *rsa = NULL;
    BIO *bufio = NULL;
    int ret = ERR_GENERIC;
    // rsa1024 �����������ܴ�С117�ֽ�,������岻�õ���0x80�ֽ�
    if (nSrcSize > 117 || nDstSize < 0x80)
        return false;
    do {
        if (!(bufio = BIO_new_mem_buf((void *)szPubKey, -1)))
            break;
        if (!(rsa = PEM_read_bio_RSA_PUBKEY(bufio, &rsa, NULL, NULL)))
            break;
        if (RSA_public_encrypt(nSrcSize, pSrcBuf, pDstBuf, rsa, RSA_PKCS1_PADDING) < 0)
            break;
        //���ɳ���=0x80�ֽ�
        ret = 0x80;
    } while (false);
    if (bufio)
        BIO_free_all(bufio);
    if (rsa)
        RSA_free(rsa);
    return ret;
}

// szOutBuf���Ȳ�����256�ֽ�
bool GetRsaEncrypInvoiceInformationBase64(HFPXX fpxx, char *szOutBuf)
{
    //[aisino��Ʊ�˶�Ҫ��2]
    char szBuf[512] = {0};
#ifdef _NODEVICE_TEST
    char szKpStr[2048] = "";
    if (_ReadHexFromDebugFile("kpstr_aisino_kprj.txt", (uint8 *)szKpStr, sizeof szKpStr) < 0)
        return false;
    //������Ҫ�������в����������ַ������ԾͲ���Ҫת��������
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

//����GBK���뿪Ʊ����
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
    //��Ʊ����
    char szStrFPLX[16] = "";
    AisinoFPLX2Str(fpxx->fplx_aisino, szStrFPLX);
    strcat(pUTF8OutBuff, szStrFPLX);
    strcat(pUTF8OutBuff, szNewlineSep);
    //��Ʊ����+ר�÷�Ʊ��ʶ+�������汾��Ϣ
    strcat(pUTF8OutBuff, fpxx->fpdm);
    GetKpstrSpecialInvoiceFlagString(fpxx, szBuf, sizeof(szBuf));
    strcat(pUTF8OutBuff, szBuf);
    strcat(pUTF8OutBuff, szNewlineSep);
    //��Ʊ����
    pUTF8OutBuff = strcat(pUTF8OutBuff, fpxx->fphm);
    //��˰��
    if (GetKpstrPriceTaxRate(fpxx, szBuf, sizeof(szBuf)) < 0)
        goto GetInvStr_Finish;
    if (strlen(szBuf)) {
        strcat(pUTF8OutBuff, "V");
        strcat(pUTF8OutBuff, szBuf);
    }
    pUTF8OutBuff = strcat(pUTF8OutBuff, szNewlineSep);
    //����
    strcat(pUTF8OutBuff, fpxx->kpsj_F1);
    strcat(pUTF8OutBuff, "@");
    //����汾��
    strcat(pUTF8OutBuff, fpxx->kprjbbh);
    strcat(pUTF8OutBuff, "@");
    //˰��Ӳ���汾��
    strcat(pUTF8OutBuff, fpxx->driverVersion);
    strcat(pUTF8OutBuff, szNewlineSep);
    //��������
    strcat(pUTF8OutBuff, fpxx->gfmc);
    strcat(pUTF8OutBuff, szNewlineSep);
    //����˰��
    strcat(pUTF8OutBuff, fpxx->gfsh);
    strcat(pUTF8OutBuff, szNewlineSep);
    //������ַ�绰
    strcat(pUTF8OutBuff, fpxx->gfdzdh);
    strcat(pUTF8OutBuff, szNewlineSep);
    //���������˺�
    strcat(pUTF8OutBuff, fpxx->gfyhzh);
    strcat(pUTF8OutBuff, szNewlineSep);
    //���ܰ汾��+�������+��������ƽ̨
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
        //���廧1% %3����˰��������,++todo(רƱ1% 3%�����ԣ����Ӧ���ǲ��������ߣ�ǿ��Ϊ0)
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
    //��Ҫ��Ʒ����
    strcat(pUTF8OutBuff, fpxx->zyspsmmc);
    strcat(pUTF8OutBuff, fpxx->zyspmc);
    strcat(pUTF8OutBuff, szNewlineSep);
    //��˰�ڱ�ǣ���ֵ����ᵼ���µ׻��ܴ���
    strcat(pUTF8OutBuff, fpxx->szPeriodCount);
    memset(szBuf, 0, sizeof(szBuf));
    if ((fpxx->fplx_aisino == FPLX_AISINO_ZYFP || fpxx->fplx_aisino == FPLX_AISINO_DZZP) &&
        fpxx->isRed) {
        if (strlen(fpxx->blueFpdm) && strlen(fpxx->blueFphm)) {
            //����רƱ������Ʊ-���۷����������Ϣ�������۷�����
            sprintf((char *)szBuf, "@%s@%08d", fpxx->blueFpdm, atoi(fpxx->blueFphm));
            strcat(pUTF8OutBuff, (char *)szBuf);
        } else {
            //�������������Ϣ�����������ߣ���ʱ��Ʊ�ַ����м���ԭ�������ߵ����ַ�Ʊ�������
        }
    }
    if (IsAisino3Device(fpxx->hDev->nDriverVersionNum)) {
        // XSDJBH(���۵��ݱ��?)��δ֪��������
        strcat(pUTF8OutBuff, defAisino3SplitFlag);
    }
    strcat(pUTF8OutBuff, szNewlineSep);
    //��������
    strcat(pUTF8OutBuff, fpxx->xfmc);
    strcat(pUTF8OutBuff, szNewlineSep);
    //����˰��
    strcat(pUTF8OutBuff, fpxx->xfsh);
    strcat(pUTF8OutBuff, szNewlineSep);
    //������ַ�绰
    strcat(pUTF8OutBuff, fpxx->xfdzdh);
    strcat(pUTF8OutBuff, szNewlineSep);
    //���������˺�
    strcat(pUTF8OutBuff, fpxx->xfyhzh);
    strcat(pUTF8OutBuff, szNewlineSep);
    //��Ʊ��
    strcat(pUTF8OutBuff, fpxx->kpr);
    strcat(pUTF8OutBuff, szNewlineSep);
    //������
    strcat(pUTF8OutBuff, fpxx->fhr);
    if (IsAisino3Device(fpxx->hDev->nDriverVersionNum)) {
        // ����������[defAisino3SplitFlag]026;20220121;2
        // LZFPZL(���ַ�Ʊ����),LZFPKJSJ�����ַ�Ʊ����ʱ�䣩��KJXXFSLYӦ���Ǻ��ַ�Ʊ����ѡ��
        // KJXXFSLY=1�����˻� 2��Ʊ���� 3������ֹ 4��������
        strcat(pUTF8OutBuff, defAisino3SplitFlag);
        if (fpxx->isRed && fpxx->blueFplx != FPLX_COMMON_DZZP &&
            fpxx->blueFplx != FPLX_COMMON_ZYFP && strlen(fpxx->blueKpsj) == 14)
            //ע�⣡�˴���Ʊ����Ϊͨ�����ͣ�����aisino���ͣ��˴�������Զ�� 2��Ʊ����
            //��Ʊ�͵�Ʊ�ĺ��ַ�Ʊ����
            sprintf(szBuf, "%03d;%.8s;%d", fpxx->blueFplx, fpxx->blueKpsj, fpxx->hcyy);
        else
            strcpy(szBuf, ";;");
        strcat(pUTF8OutBuff, szBuf);
    }
    strcat(pUTF8OutBuff, szNewlineSep);
    //�տ���
    strcat(pUTF8OutBuff, fpxx->skr);
    if (IsAisino3Device(fpxx->hDev->nDriverVersionNum)) {
        strcat(pUTF8OutBuff, defAisino3SplitFlag);
        if (!GetRsaEncrypInvoiceInformationBase64(fpxx, fpxx->kjbbh))
            goto GetInvStr_Finish;
        strcat(pUTF8OutBuff, fpxx->kjbbh);
    }
    strcat(pUTF8OutBuff, szNewlineSep);
    //��ע
    memset(szBuf, 0, sizeof(szBuf));
    UTF8ToGBKBase64(fpxx->bz, strlen(fpxx->bz), (char *)szBuf);
    strcat(pUTF8OutBuff, (char *)szBuf);
    strcat(pUTF8OutBuff, szNewlineSep);
    //���
    strcat(pUTF8OutBuff, fpxx->je);
    strcat(pUTF8OutBuff, szNewlineSep);
    //˰��
    strcat(pUTF8OutBuff, fpxx->se);
    strcat(pUTF8OutBuff, szNewlineSep);
    //�嵥���
    strcat(pUTF8OutBuff, fpxx->qdbj);
    strcat(pUTF8OutBuff, szNewlineSep);
    //=ԭUTF8bufת����GBK���������Ʒ��Ϣ
    int nGBKOutBuffIndex =
        UTF8CopyGBK((uint8 *)pGBKOutBuff, (uint8 *)pUTF8OutBuff, strlen(pUTF8OutBuff), 0);
    if (nGBKOutBuffIndex < 0)
        goto GetInvStr_Finish;
    if (fpxx->fpzt == FPSTATUS_BLANK_WASTE) {
        //��������Ʊ��Ϊ�հ����ϣ��հ�����Ʊ����Ʒ��Ϣ
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
    //[aisino��Ʊ�˶�Ҫ��1]
    _WriteHexToDebugFile("kpstr_aisino_my.txt", (uint8 *)pGBKOutBuff, strlen(pGBKOutBuff));
    return pGBKOutBuff;
}

//��Ҫע���£����ﴫ�����ͨ�����ͣ�������aisino����
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
    //��̬�Ĳ���仯�Ĳ���
    strcpy(fpxx->jmbbh, hDev->szJMBBH);
    strcpy(fpxx->driverVersion, hDev->szDriverVersion);
    strcpy(fpxx->kprjbbh, szKPRJVersion);
    strcpy(fpxx->taVersion, defTaxAuthorityVersionAisino);
    strcpy(fpxx->bmbbbh, DEF_BMBBBH_DEFAULT);
    //���ܸ���ʱ������ر仯�Ĳ���
    GetPeriodCount(hDev->hUSB, fpxx->fplx_aisino, fpxx->szPeriodCount);
}

int AisinoUpdateNodeInfo(HFPXX fpxx, HDEV hDev)
{
    if (hDev->bTrainFPDiskType == 1) {  //���ղ����̲ż���
        //�����̱�����ϣ����򿪳��ǲ�����Ϣ�ķ�Ʊ��˰�ֻỳ�ɣ�
        strcpy(fpxx->gfsh, "110101000000000");
        strcpy(fpxx->gfmc, "������ѵʹ��");
        strcpy(fpxx->gfdzdh, "������ѵʹ��");
        strcpy(fpxx->gfyhzh, "������ѵʹ��");
        strcpy(fpxx->bz, "������ѵʹ��");
    }
    if (fpxx->fplx_aisino == FPLX_AISINO_JDCFP || fpxx->fplx_aisino == FPLX_AISINO_DZFP ||
        fpxx->fplx_aisino == FPLX_AISINO_JSFP || fpxx->fplx_aisino == FPLX_AISINO_ESC) {
        strcpy(fpxx->jqbh, hDev->szDeviceID);
    } else {
        strcpy(fpxx->jqbh, "");
    }
    //���µײ�̼���Ϣ
    GetVersionInfo(fpxx, hDev);
    return 0;
}

// szRet һ����6-8k�����黺��������20k����
int EncodeInvoiceBlock2(char *pInvStr, int nInvBuffLen, char *szRet)
{
    BYTE *pSysMemBlock = NULL;
    //Ԥ���ռ�30k,ʵ����ԶԶ�ò�����ô���ڴ棬ʵ�����ڴ��ѹ����szRet���棬Ϊ�˺�Test����ͬ�����Ͷ������
    pSysMemBlock = calloc(1, nInvBuffLen * 15 + 30000);
    int nLen = InvEncryptBlock2((BYTE *)pSysMemBlock, (long *)pInvStr, nInvBuffLen, (long *)szRet);
    free(pSysMemBlock);
    return nLen;
}

// szInOutFPDM szInOutFPHM
// ���Բ�ָ��ֵ,ֱ���Զ�ѡ��һ�ŷ�Ʊ;���ָ��ֵ,��ô��������ָ����Ʊ��������Ƿ�Ϊ��һ��
int AisinoGetCurrentInvCode(HUSB hUSB, uint8 bNeedType, char *szInOutFPHM, char *szOptInvEndNum,
                            char *szInOutFPDM)
{
//������Ӧֵ
// 0040 0f 02 bd fd 04 00 e8 03 7e 32 b4 70 7e 32 da a0
// 0050 [32 00 13 46 50] 54 e7 29 00 f7
// �����ŵ�Ϊ��Ʊ���ʹ��룬���ܸ��ݲ�ͬ�ķ�Ʊ����������ͬ
//���ܶ�����Ҫ�ķ�Ʊ����Ļ��������ܣ�����ȫ�����껺������������ȡ��ȡ��ָ����Ʊ
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
        //����continue
        uint8 bContinue = cb[nRet + 1];
        BYTE bInvType = cb[1];
        if (bNeedType != bInvType)
            goto GetCurrentInvCode_LoopNext;
        //���涼�����������͵�
        int nInvCurrentNum = *(_DWORD *)(cb + 2) + *(_WORD *)(cb + 6) - *(_WORD *)(cb + 23);
        int nInvEndNum = *(_WORD *)(cb + 6) + *(_DWORD *)(cb + 2) - 1;
        pInvType5Byte = cb + 16;
        //������з�Ʊ��ö���꣬�����Ѿ���ȡ����Ҫ�ķ�Ʊ�����������ѭ��
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

    GetCurrentInvCode_LoopNext:  // goto ���߼�
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

//����1:��Ʊԭʼ���ݵ�200h�ֽ�
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
    // ��Ʊ����
    uint8 szOutTmp[128] = {0};
    int nChildRet = InvTypeCode2Byte5(szOutTmp, fpxx->fpdm);
    if (nChildRet != 5) {
        SetLastError(fpxx->hDev->hUSB, ERR_BUF_CHECK, "�ײ��������У��ʧ��,��Ʊ��������");
        logout(INFO, "TAXLIB", "��Ʊ����", "�̺�%s,�ײ㿪Ʊʧ�ܣ�%s\r\n", fpxx->hDev->szDeviceID,
               fpxx->hDev->hUSB->errinfo);
        return 0;
    }
    memcpy(pFormatBuff + nIndex, szOutTmp, nChildRet);
    nIndex += nChildRet;
    // 5λ,��Ʊ�ܼ۸�,��ʽ��
    double dF = atof(fpxx->je);
    long lAddr = (long)&dF;
    JE2InvRaw((long)(pFormatBuff + 0xb), *(double *)lAddr, 0x64);
    // 5λ,��Ʊ��˰���ʽ��
    dF = atof(fpxx->se);
    lAddr = (long)&dF;
    JE2InvRaw((long)(pFormatBuff + 0x10), *(double *)lAddr, 0x64);
    //˰�ʱ任 1byte
    if (!fpxx->isMultiTax) {  //�Ƕ�˰�ʲ�д��
        double fslv = atof(fpxx->slv);
        uint8 uSlv = (uint8)(fslv * 1000.0 + 0.1);
        pFormatBuff[0x15] = uSlv;
    }
    //����˰�ű任���� 12�ֽ�
    uint8 bpFormatBuffOffset0x77[1] = {0x04};
    nIndex = 0x19;
    memset(szOutTmp, 0, sizeof(szOutTmp));
    if (TaxID212Bytes(fpxx->gfsh, szOutTmp)) {
        // 9132D111MA20J2WK2L����˰�ŵ���ʧ��
        SetLastError(fpxx->hDev->hUSB, ERR_BUF_CHECK, "�ײ��������У��ʧ��,����˰������");
        logout(INFO, "TAXLIB", "��Ʊ����", "�̺�%s,�ײ㿪Ʊʧ�ܣ�%s\r\n", fpxx->hDev->szDeviceID,
               fpxx->hDev->hUSB->errinfo);
        return 0;
    }
    uint8 bIndex0x38 = *(pFormatBuff + 0x38);
    bpFormatBuffOffset0x77[0] = szOutTmp[2];
    uint8 bChar = (szOutTmp[2] & 0xF) | 0x80 | 0x10;
    if (sub_8B1F970(bIndex0x38) == 1 || bIndex0x38 == 11 ||
        (bIndex0x38 >= 42u && bIndex0x38 <= 60u))
        bChar |= 0x20;
    if (fpxx->isMultiTax)  //���ܻ��������������ƽ��룬����Ŀǰ���Ե���˰�ʿ϶��ǵ�
        bChar |= 0x40;
    szOutTmp[2] = bChar;
    memcpy(pFormatBuff + nIndex, szOutTmp, 12);
    // pFormatBuff 0x26-0x2b��
    // Ŀǰ����ֻ��רƱ������Ϣ�����Ʊ�и���Ϣ��רƱ������Ϣ�����Ʊ�Ƿ��иö���Ϣ������
    // �˴��ٷ��������ж��Ƿ�16��0������16λ֪ͨ����ţ�����������ղ�д�����ݣ�����д������
    if ((fpxx->fplx_aisino == FPLX_AISINO_ZYFP || fpxx->fplx_aisino == FPLX_AISINO_DZZP) &&
        fpxx->isRed && strlen(fpxx->redNum)) {
        //ʵ��ֻ�ж��Ƿ���֪ͨ����ż���
        uint8 *pTzdbh = pFormatBuff + 0x26;
        memset(pTzdbh, 0, 6);
        BuildRedTzdbh6Bytes(pTzdbh, fpxx->redNum);
    }
    // ��Ʊ����
    nIndex = 0x2e;
    pFormatBuff[nIndex] = fpxx->fplx_aisino;
    InvTypeCode2Byte5(pFormatBuff + nIndex + 1, fpxx->fpdm);
    // ��Ʊʱ���������������2���ֽ�
    uint8 szTimeBuff[24];
    if (TranslateStandTime(0, fpxx->kpsj_standard, (char *)szTimeBuff) < 0) {
        SetLastError(fpxx->hDev->hUSB, ERR_BUF_CHECK, "�ײ��������У��ʧ��,ʱ���ʽ����");
        logout(INFO, "TAXLIB", "��Ʊ����", "�̺�%s,�ײ㿪Ʊʧ�ܣ�%s\r\n", fpxx->hDev->szDeviceID,
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
    nIndex = 0x3a;  //���в����ݵ�?��ͷ��3,��ʱ���������
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
    // ˰���ۺ���֤��
    uint8 szMem[128] = "";
    uint8 sz01FF[] = {0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF,
                      0xFE, 0xDC, 0xBA, 0x98, 0x76, 0x54, 0x32, 0x10};
    memcpy(szMem + 0x40, sz01FF, sizeof(sz01FF));
    //ũ��Ʒ�չ���Ʊ���ߣ�����false
    if (Get25ByteXor(fpxx->isMultiTax, false, (uint8 *)szMem, fpxx->hDev->szCompressTaxID,
                     fpxx->gfsh, fpxx->xfsh)) {
        SetLastError(fpxx->hDev->hUSB, ERR_BUF_CHECK,
                     "�ײ��������У��ʧ��,��˰����������˰��У���㷨����");
        logout(INFO, "TAXLIB", "��Ʊ����", "�̺�%s,�ײ㿪Ʊʧ�ܣ�%s\r\n", fpxx->hDev->szDeviceID,
               fpxx->hDev->hUSB->errinfo);
        return 0;
    }
    sub_10034160((long)szMem);
    memcpy(pFormatBuff + 0x34, szMem + 0x40, 2);
    //����˰�ű任����ǰ2���ֽ�
    memcpy(pFormatBuff + 0x36, pFormatBuff + 0x19, 2);
    nIndex = 0x77;
    pFormatBuff[nIndex] = bpFormatBuffOffset0x77[0];
    // kp�ַ���
    nIndex = 0x98;
    int nSKPStrLen = strlen(szKPStr);
    nChildRet = EncodeInvoiceBlock2(szKPStr, nSKPStrLen, (char *)pFormatBuff + nIndex);
    *(uint16 *)(pFormatBuff + 0x96) = nChildRet;  //�ַ�������д��
    *(uint8 *)(pFormatBuff + 0x95) = nChildRet >> 16;
    strcpy((char *)pFormatBuff + 0x70, "Head");

    int v118 = (((nChildRet >> 16) & 0xFF) << 16) + (uint16)nChildRet;
    uint16 v221 = (uint16)(((uint32)(v118 + 0x9C) >> 9) + ((((uint16)v118 + 0x9C) & 0x1FF) != 0));
    pFormatBuff[0x74] = (BYTE)v221;
    pFormatBuff[0x2d] = (BYTE)(v221 >> 8);
    if (IsAisino3Device(fpxx->hDev->nDriverVersionNum)) {
        //���Ժ��ִ˴�aisino3����д�� 20220301
    } else {  //����aisino2
        uint16 uLenBuff = (uint16)nChildRet + 2;
        memcpy(pFormatBuff + 0x6e, &uLenBuff, 2);
    }
    pFormatBuff[0x76] = 0x00;
    return v221;  //��һ��������˽���
}

char *GetMWJYM_BuildSignB64(HUSB hUSB, uint8 *pInDeviceMWJYMBuff, HFPXX fpxx)
{
    // pInDeviceMWJYMBuffһ��Ϊ47���ֽڣ�ǰ29���ֽ�Ϊ����ǰһ��,����25���ֽ�ΪУ����
    //��Ʊʱ����һ������ԶΪ1,ũ��Ʒ�չ���Ʊ��־����Ϊfalse
    if (DecodeMWandJYM(1, fpxx->isMultiTax, false, (uint8 *)pInDeviceMWJYMBuff,
                       fpxx->hDev->szCompressTaxID, fpxx->gfsh, fpxx->xfsh, fpxx->mw,
                       fpxx->jym) < 0)
        return NULL;
    char *pRetSignBase64 = NULL;
    int nRetBufLen = 0;
    char *pSignGBKBuf = GetSignString(fpxx, &nRetBufLen);
    if (!pSignGBKBuf)
        return NULL;
    //[aisino��Ʊ�˶�Ҫ��2-���ڸô���������]
    //[aisino��Ʊ�˶�Ҫ��3.1-��ѡ]
    _WriteHexToDebugFile("signstr_aisino_my.txt", (uint8 *)pSignGBKBuf, strlen(pSignGBKBuf));
    if (fpxx->hDev->bTrainFPDiskType || !hUSB->handle) {
        // �˴����ڲ����̣�������ʽ��_NODEVICE_TEST�����õ�ʱ����
        uint8 abSignBin[512] = "";
        int nSignBinSize = 0;
        pRetSignBase64 = calloc(1, 1024);
        //[aisino��Ʊ�˶�Ҫ��4]
        if ((nSignBinSize = _ReadHexFromDebugFile("signout_aisino_kprj.bin", abSignBin,
                                                  sizeof(abSignBin))) == 327) {
            //�Զ���ǩ�����ݣ���˰��dump��ȡ
            nSignBinSize = Base64_Encode((const char *)abSignBin, nSignBinSize, pRetSignBase64);
        } else {
            //������ǩ��Base64,����ѵ����(Traindisk)ר��ǩ���������Ҫ�޸�����ǩ������Ҫע�Ͳ��������Base64Str
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
    //��д��Ļ������¶�ȡBlock2���ȣ������¼���uSepBlockNum,������п����⣬���������ϱ���������
    int nBlock2EncodeLen = *(uint16 *)(pFormatBuff + 0x96) + (*(uint8 *)(pFormatBuff + 0x95) << 16);
    //ԭ��Ϊ256���ַ�Ʊ�޷����ߣ����޸�Ϊ200 20210427;�հ�����ֻ��128����
    if (nBlock2EncodeLen <= 100) {
        _WriteLog(LL_FATAL, "BuildInvoice nBlock2EncodeLen = %d", nBlock2EncodeLen);
        return -1;
    }
    //ȡ���и����
    *uSepBlockNum = 0;
    *uSepBlockNum =
        (((((nBlock2EncodeLen >> 16) & 0xFF) << 16) + (unsigned __int16)nBlock2EncodeLen + 152) >>
         9) +
        ((((_WORD)nBlock2EncodeLen + 152) & 0x1FF) != 0);
    int nPacketNumMAX = DEF_MAX_FPBUF_LEN / 512 + 1;
    if (0 == *uSepBlockNum || *uSepBlockNum > nPacketNumMAX)
        return -2;
    //���ˣ�У�������֤���Ѿ���ȡ��ɣ��ú�����һ�������,���·��ͷ�Ʊ�ڶ���
    char outMW[512] = "";
    strcpy(outMW, fpxx->mw);
    strcpy(outMW + strlen(outMW) + 1, fpxx->jym);
    //�����0x6cӦ���Ǹ�����,�����������Ҫ���ܻ�Ҫ�޸�,0x6c ���������ĳ���
    BuildInv0x40(outMW, (uint8 *)pFormatBuff + 0x40);
    //������Ʊ��һ��ԭʼ��֤��16�ֽڵ��ڶ��ο�Ʊ����5e��
    memcpy(pFormatBuff + 0x5e, pInDeviceMWJYMBuff + 0x1e, 0x10);
    //��Ʊͷ����Ե�һ������Ĳ���
    pFormatBuff[0x76] = 0x01;  //������Ӧ����һ��������ͳ��
    pFormatBuff[0x74] = (BYTE)*uSepBlockNum + pFormatBuff[0x76];  //=��ʽ�������+0x76����
    //β��ǩ���㷨
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
    //д��Tail��ʶ
    int nFinishFlagOffset = pFormatBuff[0x76] << 9;
    strcpy((char *)pFormatBuff + nAvailBuffLen + nFinishFlagOffset - 4, "Tail");
    return nAvailBuffLen + nFinishFlagOffset;
}

//�����־λ���ô���ᵼ���µ��ϱ����ܳ��ֽ��ȶ�ʧ������?
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
    int nPackStaticEnd = 0x200;  //��Ʊ����趨���ģ�0x80*4=512�ֽ�
    pPreMakeBuff[nPackStaticEnd] = MakePackVerityFlag(pPreMakeBuff, nPackStaticEnd);
    nPackStaticEnd += 1;
    // IO comm
    uint8 cb[256];
    memcpy(cb, "\xfe\xff\xcc\xbc\x53\xe1\xdf\x00\x00\x00", 10);
    SetMakeInvoiceCommandByte7Flag(bAisinoFPLX, cb);
    //����У������֤������������������������
    int nChildRet = 0;
#ifndef _NODEVICE_TEST
    int ncbLen = sizeof cb;
    nChildRet = AisinoLogicIO(hUSB, cb, 10, ncbLen, pPreMakeBuff, nPackStaticEnd);
#else
    if (hUSB) {  //���뾯��
    }
    nChildRet = 48;
#endif
    //[aisino��Ʊ�˶�Ҫ��-��ѡ]
    _WriteHexToDebugFile("preinv_aisino_my.bin", pPreMakeBuff,
                         nPackStaticEnd - 1);  //У��λ��д��
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
    //���͵ڶ�������ʱ512�ֽں�Ჹһ��1+8�ֽ�У�����ݣ����е�һ���ֽڲ���
    //����8���ֽ���һ���㷨;
    uint8 abProcKey[32];
    memset(abProcKey, 0, 32);
    if (GetProcKey(fpxx->hDev->sz9ByteHashTaxID, abProcKey) < 0)
        return -1;

#ifdef _NODEVICE_TEST
    _WriteLog(LL_FATAL, "_NODEVICE_TEST Mode��Jump really make!");
    return 0;
#endif
    /////++debug �������Ʊ���Ƚ���
	//////_WriteLog(LL_FATAL, "��Ʊ���̻�������Ϊ��ֹʧ��Ʊ�����ѽ������տ�Ʊ�����ͣ�"
	//////                "���ֶ��ڴ�����ȡ��ע�Ͳ����Ʊģ��");
	//////SetLastError(fpxx->hDev->hUSB, ERR_PROGRAME_NEED_UNLOCK,"�ѽ��õײ����տ�Ʊ������,����ϵ������Ա");
	//////logout(INFO, "TAXLIB", "��Ʊ����","�̺�%s,�ײ㿪Ʊʧ�ܣ�%s\r\n", fpxx->hDev->szDeviceID, 
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
        if (i == 0) {                                            //��һ�����������Ĳ�һ��
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
        if (i == 0)  // i == 0; first send;��һ��������ʱ��˰�̻᷵�ش˴�Ʊ���ĵ�ַ�����ŵ���Ϣ
            GetInvoiceEnv(cb, fpxx);
    }
    return 0;
}

//��ȡУ���뼰���Ľӿ�
int BuildRawToSend(HUSB hUSB, HFPXX fpxx, char *szKPStr)
{
    int nBuffLen = strlen(szKPStr) * 4 + 128000;
    uint8 *pFormatBuff = (uint8 *)calloc(1, nBuffLen);
    //���뻺�������� ����ȫ��0
    int nRet = -1;
    uint8 abRepBuff[256];
    while (1) {
        unsigned __int16 uSepBlockNum = BuildPreInvoice(pFormatBuff, szKPStr, fpxx);
        if (0 == uSepBlockNum) {
            logout(INFO, "TAXLIB", "��Ʊ����", "BuildPreInvoice failed uSepBlockNum = %d\r\n",
                   uSepBlockNum);
            nRet = -100;  //�˴���ֵ���ϲ��ж��йأ����ɸ���
            break;
        }
        memset(abRepBuff, 0, sizeof(abRepBuff));
        if (SendPreInvoice(hUSB, fpxx->fplx_aisino, uSepBlockNum, pFormatBuff, abRepBuff) < 0) {
            nRet = -2;
            break;
        }
#ifdef _NODEVICE_TEST
        //[aisino��Ʊ�˶�Ҫ��2]
        if (47 != _ReadHexFromDebugFile("mwjym47bytes_aisino_kprj.bin", abRepBuff, 47)) {
            //�����47���ֽ�����˰�̼�ʱ���ɵģ�ÿ�ζ���һ�������ÿ�ζ�Ҫ������47���ֽ�
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
        //����ǩ��base64����Ʊ��Ϣ
        strcpy(fpxx->sign, pSignRepBase64);
        //_WriteLog(LL_DEBUG, "DeviceSN:%s InvoiceTime:%s MW:%s JYM:%s SignB64:%s",
        //          fpxx->hDev->szDeviceID, fpxx->kpsj_F1, fpxx->mw, fpxx->jym, fpxx->sign);
        //�����uSepBlockNum���ᱻ���¼��㸳ֵ
        int nChildRet = BuildInvoice(pFormatBuff, abRepBuff, &uSepBlockNum, fpxx, pSignRepBase64);
        free(pSignRepBase64);
        if (nChildRet < 0 || !uSepBlockNum) {
            _WriteLog(LL_FATAL, "BuildInvoice error result = %d\n", nChildRet);
            nRet = -4;
            break;
        }
        //[aisino��Ʊ�˶�Ҫ��5]
        _WriteHexToDebugFile("kpFinalIO_aisino_my.bin", pFormatBuff, nChildRet);
        int result;
        if ((result = SendInvoice(hUSB, fpxx, uSepBlockNum, pFormatBuff)) < 0) {
            _WriteLog(LL_FATAL, "SendInvoice error result = %d\n", result);
            if (result <= -100) {
                logout(INFO, "TAXLIB", "��Ʊ����", "SendInvoice failed result = %d\r\n", result);
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
	//501�����������жϴ˹���

	if (memcmp(p, "180104", 6) <= 0)	{
		nRet = SetLastError(hDev->hUSB, ERR_DEVICE_FIRMWARE_LOWVERSION,
			"�ײ������汾̫��,�ײ������汾̫���޷�֧�ֿ�Ʊ���������ײ�汾,driverVer:%s", hDev->szDriverVersion);		
	}
	else if (memcmp(p, "200812", 6) < 0)	{
		nRet = SetLastError(hDev->hUSB, ERR_DEVICE_FIRMWARE_LOWVERSION,
			"�ײ������汾̫��,ϵͳ����ͣ2.0˰��֧�֣��������ײ�汾,driverVer:%s", hDev->szDriverVersion);
	}
	//else if (memcmp(p, "220524", 6) >= 0)	{
	//	nRet = SetLastError(hDev->hUSB, ERR_DEVICE_FIRMWARE_HIGHVERSION,
	//		"�ײ������汾����,��ǰϵͳ�ݲ�֧�ִ˰汾������ϵ��ά��Ա,driverVer:%s", hDev->szDriverVersion);		
	//}
	if (nRet < 0){
		if (strcmp(hDev->szDeviceID, "661551502671") == 0)		{
			ClearLastError(hDev->hUSB);
			_WriteLog(LL_INFO,"�����̲����ĵײ�汾");
			nRet = 0;
		}else{
			//report_event(hDev->szDeviceID, "�ײ������汾̫��", hDev->hUSB->szLastErrorDescription, -1);
			logout(INFO, "TAXLIB", "ϵͳ�ж�", "�̺ţ�%s,������Ϣ��%s\r\n", hDev->szDeviceID, hDev->hUSB->szLastErrorDescription);
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
    // 1-������
    if ((nRet = CheckDriverVerSupport(hDev, fpxx->driverVersion)) < 0) {
        return nRet;
    }
    // 2-��������,ֻ�п�Ʊʱ�Զ����������·�
    memset(fpxx->ssyf, 0, sizeof fpxx->ssyf);
    strncpy(fpxx->ssyf, hDev->szDeviceTime, 6);
    // 2-��ȡ��Ʊ�����Ϣ
    if (AisinoGetCurrentInvCode(hUSB, fpxx->fplx_aisino, fpxx->fphm, fpxx->fpendhm, fpxx->fpdm) <
        0) {
        if ((strlen(fpxx->fphm) > 0) || (strlen(fpxx->fpdm) > 0))
            nRet =
                SetLastError(hUSB, ERR_DEVICE_CURRENT_INVOICE,
                             "��ǰ��Ʊ��������봫��ķ�Ʊ������벻һ��,���ٴ�ȷ�ϱ����ظ�����");
        else
            nRet = SetLastError(hUSB, ERR_DEVICE_CURRENT_INVOICE,
                                "��ȡ��ǰ��Ʊ�������ʧ��,���ٴ�ȷ�ϱ����ظ�����");
        return nRet;
    }
    // 3-�����������㹫������ƽ̨���
    if (strlen(hDev->szPubServer)) {
        char szPubPtlx[16];
        //�������
        if ((nChildRet =
                 BuildPubServiceRandom(fpxx->fpdm, fpxx->fphm, fpxx->kpsj_standard, szBuf)) < 0) {
            nRet = SetLastError(hUSB, ERR_LOGIC_COMMON, "����ƽ̨�����ʧ��");
            logout(INFO, "TAXLIB", "��Ʊ����", "�̺ţ�%s,����ƽ̨�����ʧ��,result = %d\r\n",
                   hDev->szDeviceID, nRet);
            return nRet;
        }
        memset(szPubPtlx, 0, sizeof(szPubPtlx));
        strncpy(szPubPtlx, hDev->szPubServer, 2);  // copy type 2bytes
        sprintf(fpxx->pubcode, "%s;%s;%s", szPubPtlx, hDev->szPubServer + 2, szBuf);
    }
    //�������÷�Ʊ��Ϣ
    _WriteLog(LL_DEBUG, "Get available invoice: FPDM:%s FPHM:%s \r\n", fpxx->fpdm, fpxx->fphm);
    // 4-���ɿ�Ʊ�ַ���
    char *pKPStr = (char *)GetInvString(fpxx);
    if (pKPStr == NULL) {
        nRet = SetLastError(hUSB, ERR_LOGIC_BUILD_FORMATBUF, "���ɷ�Ʊ����ʧ��");
        goto NextTry;
    }
    // 5-��ƱIO
    if ((nChildRet = BuildRawToSend(hUSB, fpxx, pKPStr)) < 0) {
        //�ɰش���
        logout(INFO, "TAXLIB", "��Ʊ����", "�̺ţ�%s,�ײ㿪Ʊʧ��,result = %d,LastError��%s��\r\n",
               hDev->szDeviceID, nChildRet, hDev->hUSB->errinfo);
        //�����ж�
        _WriteLog(LL_FATAL, "�ײ㿪��ʧ����Ҫ���Կ���,��%s��\n", hDev->hUSB->errinfo);
        if ((strstr(hDev->hUSB->errinfo, "[USB] 161") != NULL) ||
            (strstr(hDev->hUSB->errinfo, "[USB] 248") != NULL)) {
            _WriteLog(LL_FATAL, "˰����ȷ����,��������\n");
            fpxx->need_restart = 0;
        } else
            fpxx->need_restart = 1;
        nRet = nChildRet;
        goto NextTry;
    }
    nRet = RET_SUCCESS;
NextTry:
    if (strcmp(fpxx->fphm, fpxx->fpendhm) == 0) {
        _WriteLog(LL_FATAL, "��ʹ�õ���Ʊ�����һ�ŷ�Ʊ,������ɺ���Ҫ���жϵ����");
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
        hDev->szDeviceTime);  //����˰��ʱ��,�ɰط�Ϊ����֤����������Ƶ�����ü���IO�����ᵥ�����ô˺���
    while (1) {
        //������������Ϣ
        if (hDev->pX509Cert != NULL) {
            ;  // _WriteLog(LL_FATAL, "ԭx509֤�������������ٴζ�ȡ\r\n");//��������x509й¶
        } else if (!hDev->bTrainFPDiskType && bAuthDev) {
            X509 *pCert = NULL;
            pCert = GetX509Cert(hDev->hUSB, hDev->bDeviceType, hDev->bCryptBigIO);
            if (!pCert) {
                _WriteLog(LL_FATAL, "Get cert failed!,last errinfo =��%s \r\n",
                          hDev->hUSB->errinfo);
                nRet = -1;
                break;
            }
            hDev->pX509Cert = pCert;
            //��Ʊ��ǩ��ʱ����Ҫ��������ȥ��֤֤��,������ǲ�������Ҫ��˰��֤�飬������濪Ʊʧ��
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
        //--���뾲̬����
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
    //ѹ��˰��
    if (GetCompressTaxCode(hUSB, hDev->szCompressTaxID) < 0) {
        nRet = -2;
        goto LoadAisinoDevInfo_Finish;
    }
    // 9λhash˰��
    if (Get9BitHashTaxCode(hUSB, hDev->sz9ByteHashTaxID) < 0) {
        nRet = -3;
        goto LoadAisinoDevInfo_Finish;
    }
    if (AisinoGetDeivceTime(hUSB, hDev->szDeviceTime) < 0) {
        nRet = -5;
        goto LoadAisinoDevInfo_Finish;
    }
    //�豸���SN��������IO�ж�
    nChildRet = ReadTaxDeviceID(hUSB, hDev->szDeviceID);
    if (nChildRet > 0)
        hDev->bCryptBigIO = 1;
    else if (0 == nChildRet)
        hDev->bCryptBigIO = 0;
    else {  //< 0
        nRet = -6;
        goto LoadAisinoDevInfo_Finish;
    }
    //ͨ����ʾ˰�� ˰�̹�˾����
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
    //�����Ϣ
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

//----�հ����ϴ���begin
bool AisinoBuildBlankWasteFpxx(HDEV hDev, HFPXX fpxx, uint8 bNeedType)
{
    // szFPDM szFPHM szKpr,�ⲿ�Ѿ���ʼ��
    if (!strlen(fpxx->fpdm) || !strlen(fpxx->fphm) || !strlen(fpxx->kpr))
        return false;
    //רƱ��ǩ�������ϱ�־ΪN������ʱż��������ƱΪY�������޷��ٸ��֣����ɺͿ�Ʊ���δ��ʼ���й�
    //��������հ�����ǩ���������⣬�����ȴӴ˴��Ų�
    //
    //����רƱ��ȡ�����ı�־λ��������Ʊ������������ʷ��������
    //ǩ���㷨ûʲô����ԭ��϶��ǲ��ܶ��ģ�Ϊ��СС�Ŀհ����ϣ�ֻ���ڴ˴��Լ�������
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
    //��д�뿪Ʊ�ַ���������д�뷢Ʊrawͷ����0.13˰�ʣ���Ҳ����� :-)
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
            _WriteLog(LL_INFO, "�հ����Ϸ�Ʊ����%s ��Ʊ����%s", fpxx->fpdm, fpxx->fphm);
            if (!AisinoBuildBlankWasteFpxx(fpxx->hDev, fpxx, bInvoiceType))
                break;
            // ���ɿ�Ʊ�ַ���
            pKPStr = (char *)GetInvString(fpxx);
            if (pKPStr == NULL)
                break;
            // �հ�����ʹ�õ��ǿ�ƱIO
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
                            "��Ʊ���ʹ���������Ϸ�Ʊֻ��Ϊֽר��ֽ��");
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
                                "��Ʊ�������ָ�����󣬱����Ƿ�Ʊ��ǰͷ���������");
            break;
        }
        if (!CheckFphmRangeIncorrect(fpxx->fphm, fpxx->fpendhm, nCount)) {
            nRet = SetLastError(hDev->hUSB, ERR_PARM_CHECK, "��Ʊ������뷶Χ���󣬳�����Ʊ��Χ");
            break;
        }
        if ((nRet = AisinoLoopBlankWaste(fpxx, bInvoiceType, nCount)) < 0)
            break;
        nRet = RET_SUCCESS;
    } while (false);
    if (strcmp(fpxx->fphm, fpxx->fpendhm) == 0) {
        _WriteLog(LL_WARN, "��ʹ�õ���Ʊ�����һ�ŷ�Ʊ,������ɺ���Ҫ���жϵ����");
        fpxx->need_restart = 1;
        *need_restart = 1;
    }
    FreeFpxx(fpxx);
    return nRet;
}
//----�հ����ϴ���finish