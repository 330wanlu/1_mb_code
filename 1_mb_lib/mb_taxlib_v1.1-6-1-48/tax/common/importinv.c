/*****************************************************************************
File name:   importinv.c
Description: ���ڵ���json�ṹ��������ת��ΪFPXX�ṹ�幤��
Author:      Zako
Version:     1.0
Date:        2019.07
History:
20200723     �������,��ǿ���,����json�����ֶμ��ǰ��ո�
20201124     �ɰض���
20201216     �޹������json��������Ƴ��������֣�����ͬ�豸������봦��
*****************************************************************************/
#define importinv_c
#include "importinv.h"

HFPXX MallocFpxxLinkDev(HDEV hDev)
{
    HFPXX fpxx = MallocFpxx();
    if (!fpxx)
        return NULL;
    fpxx->hDev = hDev;
    return fpxx;
}

//���䷢Ʊ��Ϣ�ṹ��ռ�
HFPXX MallocFpxx()
{
    HFPXX stp_Fpxx = malloc(sizeof(struct Fpxx));
    if (stp_Fpxx == NULL)
        return NULL;
    memset(stp_Fpxx, 0, sizeof(struct Fpxx));
    stp_Fpxx->stp_MxxxHead = malloc(sizeof(struct Spxx));
    if (stp_Fpxx->stp_MxxxHead == NULL) {
        free(stp_Fpxx);
        return NULL;
    }
    memset(stp_Fpxx->stp_MxxxHead, 0, sizeof(struct Spxx));
    return stp_Fpxx;
}


//���䷢Ʊ��Ϣ�ṹ��ռ�
EHFPXX EMallocFpxx()
{
    EHFPXX fpxx = malloc(sizeof(struct EFpxx));
    if (fpxx == NULL)
        return NULL;
    memset(fpxx, 0, sizeof(struct EFpxx));
    fpxx->stp_MxxxHead = malloc(sizeof(struct ESpxx));
    if (fpxx->stp_MxxxHead == NULL) {
        free(fpxx);
        return NULL;
    }
    memset(fpxx->stp_MxxxHead, 0, sizeof(struct ESpxx));
    return fpxx;
}

EHFPXX EMallocFpxxLinkDev(HDEV hDev)
{
    EHFPXX fpxx = EMallocFpxx();
    if (!fpxx)
        return NULL;
    fpxx->hDev = hDev;
    return fpxx;
}



//�ͷŷ�Ʊ��Ϣ(������Ʒ��ϸ)�ṹ��
int FreeFpxx(HFPXX stp_Root)
{
    if (stp_Root == NULL)
        return -1;
    //ѭ���ͷ�ÿ�ŷ�Ʊ����ϸ
    HFPXX stp_FpxxNode = stp_Root;
    if (!stp_FpxxNode)
        return -2;
    struct Spxx *stp_MxxxNode = stp_FpxxNode->stp_MxxxHead, *stp_TmpSpxxNode = NULL;
    //�ͷ���Ʒ��ϸ
    while (stp_MxxxNode) {
        stp_TmpSpxxNode = stp_MxxxNode->stp_next;
        free(stp_MxxxNode);
        stp_MxxxNode = stp_TmpSpxxNode;
    }
    //���������ԭʼ��Ʊ��Ϣ�����ͷ�
    if (stp_FpxxNode->pRawFPBin)
        free(stp_FpxxNode->pRawFPBin);
    //�ͷ���Ʒ��ϸͷ
    free(stp_FpxxNode);
    return 0;
}

void trim(char *strIn, char *strOut)
{
    int i, j;
    i = 0;
    j = strlen(strIn) - 1;

    int k = 0;
    for (k = 0; k < strlen(strIn); k++) {
        if ((strIn[k] == 0x20) || (strIn[k] >= 0x09 && strIn[k] <= 0x0d))
            continue;
        break;
    }
    if (k == strlen(strIn)) {
        _WriteLog(LL_INFO, "���������ȫ��Ϊ���ɼ��ַ���ո�,���ݣ�%s,%02x", strIn, strIn[0]);
        return;
    }

    while ((strIn[i] == 0x20) || (strIn[i] >= 0x09 && strIn[i] <= 0x0d)) {
        ++i;
    }
    while ((strIn[j] == 0x20) || (strIn[j] >= 0x09 && strIn[j] <= 0x0d)) {
        --j;
    }
    if (strlen(strIn) != 0) {
        strncpy(strOut, strIn + i, j - i + 1);
        strOut[j - i + 1] = '\0';
    }
}

//��ʽ������json����
int GetAvailImportData(cJSON *jsonInputParentNode, char *szGetNodeKey, char *szOut, int nOutMaxLen,
                       bool bCannotNull, char *json_err)
{
    memset(szOut, 0, nOutMaxLen);

    char key[100] = {0};
    strcpy(key, szGetNodeKey);
    mystrlwr(key);
    int nChildRet = 0;
    // jsonNode �п���Ϊ��,��˲��жϷ���
    cJSON *jsonNode = cJSON_GetObjectItem(jsonInputParentNode, szGetNodeKey);

    if (bCannotNull)
        nChildRet = get_json_value_can_not_null(jsonNode, szOut, 1, nOutMaxLen);
    else
        nChildRet = get_json_value_can_null(jsonNode, szOut, 0, nOutMaxLen);
    if (nChildRet < 0) {

        if (json_err != NULL) {
            if (nChildRet == -1) {
                sprintf(json_err, "%s,�ֶ�ȱʧ", key);
            } else if (nChildRet == -2) {
                sprintf(json_err, "%s,�ֶγ�������", key);
            }
        }

        return -1;
    }
    str_replace(szOut, "<br/>", "\r\n");
    if ((szOut[0] == 0x20) || (szOut[0] >= 0x09 && szOut[0] <= 0x0d)) {
        if (strcmp(szGetNodeKey, "BZ") != 0) {
            if (strlen(szOut) > 1)
                trim(szOut, szOut);
            else
                szOut[0] = 0x00;
        }
    }
    if ((szOut[strlen(szOut) - 1] == 0x20) ||
        (szOut[strlen(szOut) - 1] >= 0x09 && szOut[strlen(szOut) - 1] <= 0x0d)) {
        if (strlen(szOut) > 1)
            trim(szOut, szOut);
        else
            szOut[0] = 0x00;
    }
    if (strcmp(szGetNodeKey, "BZ") != 0) {
        if (strstr(szOut, "\r\n") != NULL) {

            sprintf(json_err, "%s,�в��ô��ڻس�������", key);
            _WriteLog(LL_FATAL, "JSON����[%s]�в��ô��ڻس������е������ַ�\n", szGetNodeKey);
            return -2;
        }
    }
    if ((szOut[strlen(szOut) - 2] == 0x5c) && (szOut[strlen(szOut) - 1] == 0x6e)) {
        sprintf(json_err, "%s,�в��ô���\n�ַ�", key);
        _WriteLog(LL_FATAL, "JSON����[%s]�в��ô���\"\\n\"�ַ�\n", szGetNodeKey);
        return -2;
    }

    return 0;
}

int InsertMxxx(HFPXX stp_Root, struct Spxx *stp_InsertNode)
{
    if (stp_Root == NULL || stp_InsertNode == NULL)
        return -1;
    //ͷ���
    struct Spxx *stp_MxxxNode = stp_Root->stp_MxxxHead;
    while (stp_MxxxNode->stp_next) {
        stp_MxxxNode = stp_MxxxNode->stp_next;
    }
    stp_MxxxNode->stp_next = stp_InsertNode;
    stp_MxxxNode->stp_next->stp_next = NULL;
    return 0;
}

int EInsertMxxx(EHFPXX fpxx, struct ESpxx *newSpxx)
{
    if (fpxx == NULL || newSpxx == NULL)
        return -1;
    //ͷ���
    struct ESpxx *nodeMxxx = fpxx->stp_MxxxHead;
    while (nodeMxxx->stp_next) {
        nodeMxxx = nodeMxxx->stp_next;
    }
    nodeMxxx->stp_next = newSpxx;
    nodeMxxx->stp_next->stp_next = NULL;
    return RET_SUCCESS;
}

int AisinoFPLX2Str(enum enFPLX fplx, char *szRet)
{
    switch (fplx) {
        case FPLX_AISINO_ZYFP:
            strcpy(szRet, "s");
            break;
        case FPLX_AISINO_PTFP:
            strcpy(szRet, "c");
            break;
        case FPLX_AISINO_HYFP:
            strcpy(szRet, "f");
            break;
        case FPLX_AISINO_JDCFP:
            strcpy(szRet, "j");
            break;
        case FPLX_AISINO_JSFP:
            strcpy(szRet, "q");
            break;
        case FPLX_AISINO_DZFP:
            strcpy(szRet, "p");
            break;
        default:
            if (fplx == FPLX_AISINO_ESC) {
                strcpy(szRet, "s");
                break;
            }
    }
    if (strlen(szRet) == 0) {
        strcpy(szRet, "s");
    }
    return 0;
}

//�ж�С����λ��
int JudgeDecimalPoint(char *pc_jg, int min_num, int max_num)
{
    int i = 0;
    int len;
    char *pc_tmp = NULL;
    if (!pc_jg) {
        return -1;
    }
    pc_tmp = strstr(pc_jg, ".");
    if (!pc_tmp) {
        if (min_num == 0) {
            return 0;
        }
        return -2;
    }
    len = pc_tmp - pc_jg;
    i = strlen(pc_jg) - len - 1;
    if (i >= min_num && i <= max_num)
        return 0;
    return -3;
}

int AisinoFPLXFlag2CommonFPLX(uint8 *fpzl, uint8 *fplxdm)
{
    switch (fpzl[0]) {
        case 's':
            *fplxdm = FPLX_COMMON_ZYFP;
            break;
        case 'j':
            *fplxdm = FPLX_COMMON_JDCFP;
            break;
        case 'c':
            *fplxdm = FPLX_COMMON_PTFP;
            break;
        case 'f':
            *fplxdm = FPLX_COMMON_HYFP;
            break;
        case 'q':
            *fplxdm = FPLX_COMMON_JSFP;
            break;
        case 'p':
            *fplxdm = FPLX_COMMON_DZFP;
            break;
        case 'e':
            *fplxdm = FPLX_COMMON_ESC;
            break;
        case 'd':
            *fplxdm = FPLX_COMMON_DZZP;
            break;
        default:
            *fplxdm = 0;
            break;
    }
    return 0;
}

int JudgeTaxID(char *szTaxID)
{
    int i;
    for (i = 0; i < strlen(szTaxID); i++) {
        if ((('0' <= szTaxID[i]) && (szTaxID[i] <= '9')) ||
            (('A' <= szTaxID[i]) && (szTaxID[i] <= 'Z'))) {
            //ȥ��I��O��Z��S��VУ��
            continue;
        }
        return -1;
    }
    return 0;
}

int JudgePrice(char *price, int len)
{
    int i;
    for (i = 0; i < len; i++) {
        if ((('0' <= price[i]) && (price[i] <= '9')) || ('.' == price[i]) || (('-' == price[i])) ||
            (0x00 == price[i])) {
            continue;
        }
        return -1;
    }
    return 0;
}

int JudgeNumber(char *Number, int len)
{
    int i;
    for (i = 0; i < len; i++) {
        if ((('0' <= Number[i]) && (Number[i] <= '9')) || (0x00 == Number[i])) {
            continue;
        }
        return -1;
    }
    return 0;
}

//�Ƿ���Ʒ�ۿ�,ture or false
uint8 IsZKXX(uint8 bDeviceType, char *szFphxz)
{
    if (DEVICE_AISINO == bDeviceType) {
        switch (atoi(szFphxz)) {
            case FPHXZ_AISINO_SPXX:
                return 0;
            case FPHXZ_AISINO_ZKXX:
                return 1;
            case FPHXZ_AISINO_SPXX_ZK:
                return 2;
        }
    } else
        return atoi(szFphxz);
    return 0;
}

void GetRedInvNotes(char *string_0, char *string_1, char *szOut)
{
    if (string_0 != NULL && strlen(string_1) < 8) {
        int nNum = atoi(string_1);
        sprintf(string_1, "%08d", nNum);
    }
    sprintf(szOut, "��Ӧ������Ʊ����:%s����:%s", string_0, string_1);
}

//����Ʊ֪ͨ������Ƿ�Ϸ�
bool CheckTzdh(char *szRedNum, char *szInvFlag)
{
    if (szRedNum == NULL)
        return false;
    if (strlen(szRedNum) != 16)
        return false;
    if (!strcmp(szRedNum, "0000000000000000"))
        return true;  //����У��
    if (strspn(szRedNum, "0123456789") != strlen(szRedNum))
        return false;
    char szTmp[16];
    memset(szTmp, 0, 16);
    strncpy(szTmp, szRedNum + 6, 2);
    int num = atoi(szTmp);
    if (num < 7)
        return false;
    memset(szTmp, 0, 16);
    strncpy(szTmp, szRedNum + 8, 2);
    int num2 = atoi(szTmp);
    if (szInvFlag[0] != 's' && szInvFlag[0] != 'f')
        return false;
    if (szInvFlag[0] == 's' && (num2 < 1 || num2 > 12))
        return false;
    if (szInvFlag[0] == 'f' && (num2 < 13 || num2 > 24))
        return false;
    int num3 = 0;
    int i = 0;
    memset(szTmp, 0, 16);
    for (i = 0; i < strlen(szRedNum) - 1; i++) {
        strncpy(szTmp, szRedNum + i, 1);
        num3 += atoi(szTmp);
    }
    strncpy(szTmp, szRedNum + strlen(szRedNum) - 1, 1);
    int nVNum = atoi(szTmp);
    bool bRet = num3 % 10 == nVNum;
    return bRet;
}

//��ȡ��ʽ����ZYFP��Ʊ��ע
int GetZYFPRedNotes(uint8 bDeviceType, char *szRedNum, char *szInvFlag, char *szOut)
{
    if (!CheckTzdh(szRedNum, szInvFlag))
        return -1;
    if (DEVICE_AISINO == bDeviceType) {
        if (szInvFlag[0] == 's')
            sprintf(szOut, "%s%s", "���ߺ�����ֵ˰ר�÷�Ʊ��Ϣ����", szRedNum);
        else if (szInvFlag[0] == 'f')
            sprintf(szOut, "%s%s", "���ߺ��ֻ�������ҵ��ֵ˰ר�÷�Ʊ��Ϣ����", szRedNum);
        else
            return -2;
    } else if ((DEVICE_NISEC == bDeviceType) ||
               (DEVICE_CNTAX == bDeviceType)) {  //˰��ukeyר�÷�Ʊ���ַ�Ʊ��ע���Ҳο�˰����
        sprintf(szOut, "%s%s", "���ַ�Ʊ��Ϣ����", szRedNum);
    } else
        return -3;
    return 0;
}

void SpxxPriceRemoveTaxContition(uint8 bDeviceType, struct Fpxx *fpxx, struct Spxx *spxx)
{
    if (DEVICE_AISINO == bDeviceType)
        return;
    if (strlen(spxx->hsjbz) == 0 || !strcmp(spxx->hsjbz, "0"))
        return;

    char dj_tmp[20] = {0};
    int int_len;
    char *p_point;

    int nRoundSize = 14;
    //ʵ��nisec�����,spslҲ�Ǳ��ضϵ�,���ǿ��Ǽ���׼ȷ��,��ʱ����Ʊ�������,�������ٽ��
    if (FPLX_COMMON_DZFP == fpxx->fplx) {
        nRoundSize = 6;
        if (strlen(spxx->dj) != 0) {
            if (fpxx->zyfpLx == ZYFP_CEZS) {
                double bhje;
                double bhsdj;
                char bhsdj_s[50] = {0};
                char bhsje_s[50] = {0};
                char tmp_s[50] = {0};
                bhje = (atof(spxx->dj) * atof(spxx->sl) - atof(spxx->kce)) / (1 + atof(spxx->slv)) +
                       atof(spxx->kce);
                sprintf(bhsje_s, "%.16f", bhje);
                PriceRound(bhsje_s, 2, tmp_s);
                bhje = atof(tmp_s);
                bhsdj = bhje / atof(spxx->sl);
                sprintf(bhsdj_s, "%.16f", bhsdj);
                PriceRound(bhsdj_s, 6, spxx->dj);
            } else {
                PriceRemoveTax(spxx->dj, spxx->slv, nRoundSize,
                               dj_tmp);  // nisec��Ʊ��׼ת��λ6λ����
                PriceRound(dj_tmp, 6, spxx->dj);
            }
        }
    }
    //����������ۼ�������� ������� �۳���
    else {
        if (strlen(spxx->dj) != 0) {
            if (fpxx->zyfpLx == ZYFP_CEZS) {
                double bhje;
                double bhsdj;
                char bhsdj_s[50] = {0};
                char bhsje_s[50] = {0};
                char tmp_s[50] = {0};
                bhje = (atof(spxx->dj) * atof(spxx->sl) - atof(spxx->kce)) / (1 + atof(spxx->slv)) +
                       atof(spxx->kce);
                sprintf(bhsje_s, "%.16f", bhje);
                PriceRound(bhsje_s, 2, tmp_s);
                bhje = atof(tmp_s);
                bhsdj = bhje / atof(spxx->sl);
                sprintf(bhsdj_s, "%.16f", bhsdj);

                p_point = strstr(bhsdj_s, ".");
                int_len = strlen(bhsdj_s) - strlen(p_point + 1);
                PriceRound(bhsdj_s, 16 - int_len, spxx->dj);
            } else {
                PriceRemoveTax(spxx->dj, spxx->slv, nRoundSize,
                               dj_tmp);  // nisecר��Ʊ��׼ת��λ���ȹ���16λ
                p_point = strstr(dj_tmp, ".");
                int_len = strlen(dj_tmp) - strlen(p_point + 1);
                PriceRound(dj_tmp, 16 - int_len, spxx->dj);
            }
        }
    }

    //����
    strcpy(fpxx->hsjbz, "0");
    strcpy(spxx->hsjbz, "0");
    return;
}

int check_spxx_hsjbz(struct Fpxx *stp_fpxx, struct Spxx *stp_Spxx, char *errinfo)
{
    if (strcmp(stp_Spxx->hsjbz, stp_fpxx->hsjbz) != 0) {
        sprintf(errinfo,
                "��Ʒ��Ϣ�к�˰�۱�־�뷢Ʊ�к�˰�۱�־��һ��,��Ʒ�к�˰�۱�־%s,"
                "��Ʊ�к�˰�۱�־%s",
                stp_Spxx->hsjbz, stp_fpxx->hsjbz);
        _WriteLog(LL_FATAL, "%s\n", errinfo);
        return DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
    }
    return 0;
}

int check_spxx_spbh(char *spbh, char *errinfo)
{
    int i = 0;
    if (strlen(spbh) < 19) {
        sprintf(errinfo, "��Ʒ��Ϣ����Ʒ��Ÿ�ʽ����,����ӦΪ19λ");
        return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
    }

    if (memcmp(spbh, "10701010", strlen("10701010")) == 0) {
        sprintf(errinfo, "�ݲ�֧�ֿ��߳�Ʒ�ͷ�Ʊ");
        return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
    }
    if ((memcmp(spbh, "1090305", strlen("1090305")) == 0) ||
        (memcmp(spbh, "1090306", strlen("1090306")) == 0) ||
        (memcmp(spbh, "1090307", strlen("1090307")) == 0) ||
        (memcmp(spbh, "1090309", strlen("1090309")) == 0) ||
        (memcmp(spbh, "1090312", strlen("1090312")) == 0) ||
        (memcmp(spbh, "1090315", strlen("1090315")) == 0)) {
        sprintf(errinfo, "�ݲ�֧�ֿ��߻�������Ʊ");
        return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
    }
    if ((memcmp(spbh, "103040501", strlen("103040501")) == 0) ||
        (memcmp(spbh, "103040502", strlen("103040502")) == 0) ||
        (memcmp(spbh, "103040503", strlen("103040503")) == 0) ||
        (memcmp(spbh, "103040504", strlen("103040504")) == 0)){
        sprintf(errinfo, "�ݲ�֧�ֿ��ߵ����̷�Ʊ");
        return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
    }
    //wang 02 01 ϡ�����벢����Ʒ
    if (memcmp(spbh, "102", strlen("102")) == 0) {
        sprintf(errinfo, "�ݲ�֧�ֿ��߿��Ʒ��Ʊ");
        return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
    }
    for (i = 0; i < strlen(spbh); i++) {
        if ((spbh[i] < '0') || (spbh[i] > '9')) {
            sprintf(errinfo, "��Ʒ��Ϣ����Ʒ��Ÿ�ʽ����,����ӦΪ������");
            return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
        }
    }
    return 0;
}

int check_spxx_lslvbz(struct Spxx *stp_Spxx, char *errinfo)
{
    if (strlen(stp_Spxx->slv) < 4) {
        sprintf(errinfo, "��Ʒ��Ϣ��˰�ʳ�������,Ӧ����0.00��0.01��0.03��ʽ");
        _WriteLog(LL_FATAL, "%s\n", errinfo);
        return DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
    }
	if ((strcmp(stp_Spxx->xsyh, "0") != 0) &&
		(strcmp(stp_Spxx->xsyh, "1") != 0))	{
		sprintf(errinfo, "��Ʒ��Ϣ���Ż����߱�ʶ��֧��0��ʹ���Ż����� 1ʹ���Ż�����");
		_WriteLog(LL_FATAL, "%s\n", errinfo);
		return DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
	}

    if (strlen(stp_Spxx->lslvbs) != 0) {
        int lslbz = atoi(stp_Spxx->lslvbs);
		if ((strcmp(stp_Spxx->lslvbs, "0") != 0) &&
			(strcmp(stp_Spxx->lslvbs, "1") != 0) &&
			(strcmp(stp_Spxx->lslvbs, "2") != 0) &&
			(strcmp(stp_Spxx->lslvbs, "3") != 0)){
			sprintf(errinfo, "��Ʒ��Ϣ��˰�ʱ�ʶ��֧��0������˰ 1��˰ 2������ 3��ͨ��˰�ʺͿ�");
			_WriteLog(LL_FATAL, "%s\n", errinfo);
			return DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
		}


        if (lslbz == 0) {
            sprintf(errinfo, "������˰��ʽ��ʹ����˰�ʿ���,���ڱ�ע�����������˰�����Ϣ");
            _WriteLog(LL_FATAL, "%s\n", errinfo);
            return DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        } else if ((lslbz == 1) && (strcmp(stp_Spxx->slv, "0.00") != 0)) {
            sprintf(errinfo, "��Ʒ��Ϣ��˰�ʱ�ʶΪ1��˰����˰�ʷ�0.00,˰�ʣ�%s", stp_Spxx->slv);
            _WriteLog(LL_FATAL, "%s\n", errinfo);
            return DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        } else if ((lslbz == 2) && (strcmp(stp_Spxx->slv, "0.00") != 0)) {
            sprintf(errinfo, "��Ʒ��Ϣ��˰�ʱ�ʶΪ2�����գ���˰�ʷ�0.00,˰�ʣ�%s", stp_Spxx->slv);
            _WriteLog(LL_FATAL, "%s\n", errinfo);
            return DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        } else if ((lslbz == 3) && (strcmp(stp_Spxx->slv, "0.00") != 0)) {
            sprintf(errinfo, "��Ʒ��Ϣ��˰�ʱ�ʶΪ3��ͨ��˰�ʣ���˰�ʷ�0.00,˰�ʣ�%s",
                    stp_Spxx->slv);
            _WriteLog(LL_FATAL, "%s\n", errinfo);
            return DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        } else if ((lslbz == 3) && (atoi(stp_Spxx->xsyh) != 0)) {
            sprintf(errinfo, "��Ʒ��Ϣ��˰�ʱ�ʶΪ3��ͨ��˰�ʣ����Ż����߱�ʶ��0���Ż����߱�ʶ��%s",
                    stp_Spxx->xsyh);
            _WriteLog(LL_FATAL, "%s\n", errinfo);
            return DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        } else {
            strcpy(stp_Spxx->slv, "0");
        }
    }
    if ((atoi(stp_Spxx->xsyh) == 0) && (strcmp(stp_Spxx->slv, "0.00") == 0) &&
        (atoi(stp_Spxx->lslvbs) != 3)) {
        sprintf(errinfo, "˰��Ϊ0.00���Ż�����Ϊ0ʱ,˰�ʱ�ʶӦΪ3");
        _WriteLog(LL_FATAL, "%s\n", errinfo);
        return DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
    }
    if ((atof(stp_Spxx->slv) != 0) && (strlen(stp_Spxx->lslvbs) != 0)) {
        _WriteLog(LL_WARN, "��Ʒ˰��%s,��Ʒ���%s", stp_Spxx->slv, stp_Spxx->je);
        sprintf(errinfo, "��Ʒ����˰��,˰�ʱ�ʶӦΪ��");
        _WriteLog(LL_FATAL, "%s\n", errinfo);
        return DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
    }
    if ((atoi(stp_Spxx->lslvbs) == 1) || ((atoi(stp_Spxx->lslvbs) == 2))) {
        if (atoi(stp_Spxx->xsyh) != 1) {
            sprintf(errinfo, "��˰�ʱ�ʶΪ1��2ʱ,�Ż����߱�ʶӦΪ1");
            _WriteLog(LL_FATAL, "%s\n", errinfo);
            return DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        }
    }
    return 0;
}

int check_spxx_yhzcbs_zsfs(uint8 bDeviceType, struct Fpxx *stp_fpxx, struct Spxx *stp_Spxx,
                           char *errinfo)
{
    if ((atof(stp_Spxx->xsyh) != 1) && (atof(stp_Spxx->xsyh) != 0)) {
        sprintf(errinfo, "�Ż����߱�ʶ��д����");
        _WriteLog(LL_FATAL, "%s\n", errinfo);
        return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
    }
    // int fphxz = IsZKXX(bDeviceType, stp_Spxx->fphxz);
    if (atof(stp_Spxx->xsyh) == 1) {
        if (strlen(stp_Spxx->yhsm) < 4)  //����Ϊ��˰��4���ַ�
        {
            sprintf(errinfo, "ʹ���Ż����߱�ʶ,����ȷ��д�Ż�˵������ZZSTSGL");
            _WriteLog(LL_FATAL, "%s\n", errinfo);
            return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
        }
        if (strcmp(stp_Spxx->yhsm, "��3%��������") == 0) {
            if (strcmp(stp_Spxx->slv, "0.03") != 0) {
                sprintf(errinfo, "��3%%�������գ�˰����Ϊ0.03");
                _WriteLog(LL_FATAL, "%s\n", errinfo);
                return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
            }
            _WriteLog(LL_FATAL, "��3%%��������\n");
        } else if (strcmp(stp_Spxx->yhsm, "��5%��������") == 0) {
            if (strcmp(stp_Spxx->slv, "0.05") != 0) {
                sprintf(errinfo, "��5%%�������գ�˰����Ϊ0.05");
                _WriteLog(LL_FATAL, "%s\n", errinfo);
                return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
            }
            _WriteLog(LL_FATAL, "��5%%��������\n");
        } else if (strcmp(stp_Spxx->yhsm, "��˰") == 0) {
            // _WriteLog(LL_FATAL, "��˰\n");
            if ((atof(stp_Spxx->slv) != 0) || (strcmp(stp_Spxx->se, "0.00") != 0)) {
                sprintf(errinfo, "��˰Ҫ����Ʒ˰�ʺ�˰��Ϊ0.00");
                _WriteLog(LL_FATAL, "%s\n", errinfo);
                return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
            }
            strcpy(stp_Spxx->slv, "0");
            strcpy(stp_Spxx->lslvbs, "1");
        } else if (strcmp(stp_Spxx->yhsm, "����˰") == 0) {
            if ((atof(stp_Spxx->slv) != 0) || (strcmp(stp_Spxx->se, "0.00") != 0)) {
                sprintf(errinfo, "��˰Ҫ����Ʒ˰�ʺ�˰��Ϊ0.00");
                _WriteLog(LL_FATAL, "%s\n", errinfo);
                return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
            }
            strcpy(stp_Spxx->slv, "0");
            strcpy(stp_Spxx->lslvbs, "2");
        } else if (strcmp(stp_Spxx->yhsm, "��5%�������ռ���1.5%����") == 0) {
            sprintf(errinfo, "��5%%�������ռ���1.5%%����,�ݲ�֧��");
            _WriteLog(LL_FATAL, "%s\n", errinfo);
            return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
        } else if (strcmp(stp_Spxx->yhsm, "��5%�������ռ���3%����") == 0) {

            sprintf(errinfo, "��5%%�������ռ���3%%����,�ݲ�֧��");
            _WriteLog(LL_FATAL, "%s\n", errinfo);
            return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
        } else if (strcmp(stp_Spxx->yhsm, "��˰��3%��������") == 0) {
            _WriteLog(LL_FATAL, "��˰��3%%��������\n");
        } else if (strcmp(stp_Spxx->yhsm, "��˰��8%��������") == 0) {
            _WriteLog(LL_FATAL, "��˰��8%%��������\n");
        } else if (strcmp(stp_Spxx->yhsm, "��˰��12%��������") == 0) {
            _WriteLog(LL_FATAL, "��˰��12%%��������\n");
        } else {
            sprintf(errinfo, "%s,�ݲ�֧��", stp_Spxx->yhsm);
            _WriteLog(LL_FATAL, "%s\n", errinfo);
            return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
        }
    } else {
        if (strlen(stp_Spxx->yhsm) > 0) {
            sprintf(errinfo, "�Ż����߱�ʶΪ0ʱ,����������ֵ˰�������˵��(���Ż�˵��)");
            _WriteLog(LL_FATAL, "%s\n", errinfo);
            return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
        }
    }

    if ((stp_fpxx->zyfpLx == ZYFP_CEZS) || (strlen(stp_Spxx->yhsm) != 0)) {
        // if ((fphxz != 0) && (bDeviceType == DEVICE_NISEC)) {
        //    sprintf(errinfo, "˰���̲�֧��,��Ʊ�����ʲ�Ϊ������,�޷����߲�����ջ����������Ʊ");
        //    _WriteLog(LL_FATAL, "%s\n", errinfo);
        //    return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
        //}
        // if ((fphxz != 0) && (bDeviceType == DEVICE_CNTAX)) {
        //    sprintf(errinfo,
        //    "˰��UKEY��֧��,��Ʊ�����ʲ�Ϊ������,�޷����߲�����ջ����������Ʊ");
        //    _WriteLog(LL_FATAL, "%s\n", errinfo);
        //    return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
        //}
        if (stp_fpxx->zyfpLx == ZYFP_CEZS) {
            if (stp_fpxx->spsl != 1) {
                sprintf(errinfo, "��Ʒ��������1��,�޷����߲������");
                _WriteLog(LL_FATAL, "%s\n", errinfo);
                return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
            }
        }
    }
    return 0;
}


// int check_dj_len(uint8 bDeviceType, struct Fpxx *stp_fpxx, struct Spxx *stp_Spxx, char *errinfo)
//{
//    if (strlen(stp_Spxx->dj) == 0) {
//        //����δ���������У��
//        return 0;
//    }
//
//	int add_len_sl = 0;
//	if (stp_fpxx->isRed == 1)
//	{
//		add_len_sl = 1;
//	}
//
//    if (bDeviceType == DEVICE_AISINO) {
//        if ((stp_fpxx->fplx == 4) || (stp_fpxx->fplx == 7) || (stp_fpxx->fplx == 26)) {
//            if (JudgeDecimalPoint(stp_Spxx->dj, 2, 15) != 0) {
//                sprintf(errinfo, "��˰�̷�Ʊ����ĵ���%s��С���㲻��С��2λ�򳬹�15λ����",
//                        stp_Spxx->dj);
//                logout(INFO, "TAXLIB", "��Ʊ���߽���","%s\n", errinfo);
//                return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
//            }
//            if (JudgeDecimalPoint(stp_Spxx->sl, 0, 15) != 0) {
//                sprintf(errinfo, "��˰�̷�Ʊ���������%s��С���㲻�ó���15λ����", stp_Spxx->sl);
//                logout(INFO, "TAXLIB", "��Ʊ���߽���","%s\n", errinfo);
//                return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
//            }
//            if (strlen(stp_Spxx->dj) > 20) {
//                sprintf(errinfo, "��˰�̷�Ʊ����ĵ���%s���ܳ��Ȳ��ó���20λ����", stp_Spxx->dj);
//                logout(INFO, "TAXLIB", "��Ʊ���߽���","%s\n", errinfo);
//                return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
//            }
//			if (strlen(stp_Spxx->sl) > 20) {
//                sprintf(errinfo, "��˰�̷�Ʊ���������%s���ܳ��Ȳ��ó���20λ����", stp_Spxx->sl);
//                logout(INFO, "TAXLIB", "��Ʊ���߽���","%s\n", errinfo);
//                return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
//            }
//        }
//    } else if ((bDeviceType == DEVICE_NISEC) || (bDeviceType == DEVICE_CNTAX)) {
//        if (stp_fpxx->fplx == 26) {
//            _WriteLog(LL_FATAL, "˰���̵�Ʊ��ౣ��С����6λ\n");
//            if (JudgeDecimalPoint(stp_Spxx->dj, 0, 6) != 0) {
//                if (JudgeDecimalPoint(stp_Spxx->dj, 2, 8) == 0) //���ݽ�˰��8λ��ֱ��Ĭ���޸�Ϊ6λ
//                {
//                    double dj_doub;
//                    char dj_str[48] = {0};
//                    dj_doub = atof(stp_Spxx->dj);
//                    sprintf(dj_str, "%.6f", dj_doub);
//                    logout(INFO, "TAXLIB",
//                    "��Ʊ���߽���","���۳���С��6λ��%s��,Ĭ��ֱ���޸ĳ�С��6λ��%s��\n",
//                    stp_Spxx->dj,
//                              dj_str);
//                    memset(stp_Spxx->dj, 0, sizeof(stp_Spxx->dj));
//                    memcpy(stp_Spxx->dj, dj_str, strlen(dj_str));
//                    return 0;
//                }
//
//                sprintf(errinfo, "˰���̵�Ʊ����ĵ���%s��С���㲻��С��2λ�򳬹�6λ����",
//                        stp_Spxx->dj);
//                logout(INFO, "TAXLIB", "��Ʊ���߽���","%s\n", errinfo);
//                return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
//            }
//
//			if (JudgeDecimalPoint(stp_Spxx->sl, 0, 6) != 0) {
//				sprintf(errinfo, "˰���̵�Ʊ���������%s��С���㲻�ó���6λ����",
// stp_Spxx->sl); 				logout(INFO, "TAXLIB", "��Ʊ���߽���","%s\n",
// errinfo); return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
//			}
//
//            if (strlen(stp_Spxx->dj) > 16) {
//                sprintf(errinfo, "˰���̵�Ʊ����ĵ���%s���ܳ��Ȳ��ó���16λ����", stp_Spxx->dj);
//                logout(INFO, "TAXLIB", "��Ʊ���߽���","%s\n", errinfo);
//                return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
//            }
//
//			if (strlen(stp_Spxx->sl) > 16 + add_len_sl) {
//                sprintf(errinfo, "˰���̵�Ʊ���������%s���ܳ��Ȳ��ó���16λ����", stp_Spxx->sl);
//                logout(INFO, "TAXLIB", "��Ʊ���߽���","%s\n", errinfo);
//                return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
//            }
//        } else if ((stp_fpxx->fplx == 4) || (stp_fpxx->fplx == 7)) {
//
//            if ((stp_Spxx->sl[0] == '0') && (stp_Spxx->sl[1] == '.')) {
//                if (JudgeDecimalPoint(stp_Spxx->sl, 0, 15) != 0) {
//                    sprintf(errinfo, "˰����ר��Ʊ���������%s��С���㲻�ó���15λ����",
//                            stp_Spxx->sl);
//                    logout(INFO, "TAXLIB", "��Ʊ���߽���","%s\n", errinfo);
//                    return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
//                }
//				if (strlen(stp_Spxx->sl) > 17 + add_len_sl) {
//                    sprintf(errinfo, "˰����ר��Ʊ���������%s���ó����ܳ���17�ֽ�",
//                    stp_Spxx->sl); logout(INFO, "TAXLIB", "��Ʊ���߽���","%s\n", errinfo);
//                    return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
//                }
//            } else {
//                if (JudgeDecimalPoint(stp_Spxx->sl, 0, 14) != 0) {
//                    sprintf(errinfo, "˰����ר��Ʊ���������%s��С���㲻�ó���14λ����",
//                            stp_Spxx->sl);
//                    logout(INFO, "TAXLIB", "��Ʊ���߽���","%s\n", errinfo);
//                    return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
//                }
//				if (strlen(stp_Spxx->sl) > 16 + add_len_sl) {
//                    sprintf(errinfo, "˰����ר��Ʊ���������%s���ó����ܳ���16�ֽ�",
//                    stp_Spxx->sl); logout(INFO, "TAXLIB", "��Ʊ���߽���","%s\n", errinfo);
//                    return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
//                }
//            }
//            if ((stp_Spxx->dj[0] == '0') && (stp_Spxx->dj[1] == '.')) {
//                if (JudgeDecimalPoint(stp_Spxx->dj, 0, 15) != 0) {
//                    sprintf(errinfo, "˰����ר��Ʊ����ĵ���%s��С���㲻�ó���15λ����",
//                            stp_Spxx->dj);
//                    logout(INFO, "TAXLIB", "��Ʊ���߽���","%s\n", errinfo);
//                    return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
//                }
//                if (strlen(stp_Spxx->dj) > 17) {
//                    sprintf(errinfo, "˰����ר��Ʊ����ĵ���%s���ó����ܳ���17�ֽ�",
//                    stp_Spxx->dj); logout(INFO, "TAXLIB", "��Ʊ���߽���","%s\n", errinfo);
//                    return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
//                }
//            } else {
//                if (JudgeDecimalPoint(stp_Spxx->dj, 0, 14) != 0) {
//                    sprintf(errinfo, "˰����ר��Ʊ����ĵ���%s��С���㲻�ó���14λ����",
//                            stp_Spxx->dj);
//                    logout(INFO, "TAXLIB", "��Ʊ���߽���","%s\n", errinfo);
//                    return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
//                }
//                if (strlen(stp_Spxx->dj) > 16) {
//                    sprintf(errinfo, "˰����ר��Ʊ����ĵ���%s���ó����ܳ���16�ֽ�",
//                    stp_Spxx->dj); logout(INFO, "TAXLIB", "��Ʊ���߽���","%s\n", errinfo);
//                    return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
//                }
//            }
//        }
//    }
//    return 0;
//}

int check_dj_len(uint8 bDeviceType, struct Fpxx *stp_fpxx, struct Spxx *stp_Spxx, char *errinfo)
{
    if (strlen(stp_Spxx->dj) == 0) {
        //����δ���������У��
        return 0;
    }

    int add_len_sl = 0;
    if (stp_fpxx->isRed == 1) {
        add_len_sl = 1;
    }

    int sl_point_max_len = 0;
    int dj_point_max_len = 0;
    int dj_point_min_len = 0;
    int sl_str_max_len = 0;
    int dj_str_max_len = 0;

    if ((bDeviceType == DEVICE_AISINO) || (bDeviceType == DEVICE_MENGBAI)) {
        if ((stp_fpxx->fplx == FPLX_COMMON_ZYFP) || (stp_fpxx->fplx == FPLX_COMMON_PTFP) ||
            (stp_fpxx->fplx == FPLX_COMMON_DZFP)) {

            sl_point_max_len = 15;
            dj_point_max_len = 15;
            sl_str_max_len = 20 + add_len_sl;
            dj_str_max_len = 20;
            dj_point_min_len = 2;
        }
    } else if ((bDeviceType == DEVICE_NISEC) || (bDeviceType == DEVICE_CNTAX)) {
        if (stp_fpxx->fplx == FPLX_COMMON_DZFP) {
            //_WriteLog(LL_INFO, "˰���̵�Ʊ��ౣ��С����6λ");
            if (JudgeDecimalPoint(stp_Spxx->dj, 2, 8) == 0)  //���ݽ�˰��8λ��ֱ��Ĭ���޸�Ϊ6λ
            {
                double dj_doub;
                char dj_str[48] = {0};
                dj_doub = atof(stp_Spxx->dj);
                sprintf(dj_str, "%.6f", dj_doub);
                // logout(INFO, "TAXLIB",
                // "��Ʊ���߽���","���۳���С��6λ��%s��,Ĭ��ֱ���޸ĳ�С��6λ��%s��\r\n",
                // stp_Spxx->dj,dj_str);
                memset(stp_Spxx->dj, 0, sizeof(stp_Spxx->dj));
                memcpy(stp_Spxx->dj, dj_str, strlen(dj_str));
            }
            sl_point_max_len = 6;
            dj_point_max_len = 6;
            sl_str_max_len = 16 + add_len_sl;
            dj_str_max_len = 16;
        } else if ((stp_fpxx->fplx == FPLX_COMMON_ZYFP) || (stp_fpxx->fplx == FPLX_COMMON_DZZP) ||
                   (stp_fpxx->fplx == FPLX_COMMON_PTFP)) {
            if ((stp_Spxx->sl[0] == '0') && (stp_Spxx->sl[1] == '.')) {
                sl_point_max_len = 15;
                sl_str_max_len = 17 + add_len_sl;
            } else if ((stp_Spxx->sl[0] == '-') && (stp_Spxx->sl[1] == '0') &&
                       (stp_Spxx->sl[2] == '.'))  //�������ܴ��ڸ��������۲�����
            {
                sl_point_max_len = 15;
                sl_str_max_len = 17 + add_len_sl;
            } else {
                sl_point_max_len = 14;
                sl_str_max_len = 16 + add_len_sl;
            }
            if ((stp_Spxx->dj[0] == '0') && (stp_Spxx->dj[1] == '.')) {
                dj_point_max_len = 15;
                dj_str_max_len = 17;
            } else {
                dj_point_max_len = 14;
                dj_str_max_len = 16;
            }
        }
        dj_point_min_len = 0;
    }

    int last_len = strlen(stp_Spxx->sl) - 1;
    if (stp_Spxx->sl[last_len] == '.') {
        sprintf(errinfo, "˰�̷�Ʊ���������%s��С���㾫������", stp_Spxx->sl);
        _WriteLog(LL_INFO, "%s\n", errinfo);
        return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
    }

    if (JudgeDecimalPoint(stp_Spxx->dj, dj_point_min_len, dj_point_max_len) != 0) {
        sprintf(errinfo, "˰�̷�Ʊ����ĵ���%s��С���㲻��С��%dλ�򳬹�%dλ����", stp_Spxx->dj,
                dj_point_min_len, dj_point_max_len);
        _WriteLog(LL_INFO, "%s\n", errinfo);
        return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
    }
    if (JudgeDecimalPoint(stp_Spxx->sl, 0, sl_point_max_len) != 0) {
        sprintf(errinfo, "˰�̷�Ʊ���������%s��С���㲻�ó���%dλ����", stp_Spxx->sl,
                sl_point_max_len);
        _WriteLog(LL_INFO, "%s\n", errinfo);
        return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
    }
    if (strlen(stp_Spxx->dj) > dj_str_max_len) {
        sprintf(errinfo, "˰�̷�Ʊ����ĵ���%s���ܳ��Ȳ��ó���%dλ����", stp_Spxx->dj,
                dj_str_max_len);
        _WriteLog(LL_INFO, "%s\n", errinfo);
        return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
    }
    if (strlen(stp_Spxx->sl) > sl_str_max_len) {
        sprintf(errinfo, "˰�̷�Ʊ���������%s���ܳ��Ȳ��ó���%dλ����", stp_Spxx->sl,
                sl_str_max_len);
        _WriteLog(LL_INFO, "%s\n", errinfo);
        return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
    }
    return 0;
}

int check_spxx_dj_sl_je_se(uint8 bDeviceType, struct Fpxx *stp_fpxx, struct Spxx *stp_Spxx,
                           char *errinfo)
{
    long double f_sl = 0, f_slv = 0, f_bhsje = 0, f_kce = 0, f_se = 0, f_dj = 0;
    long double f_cal_se = 0, f_cal_bhsje = 0, f_cal_hsje = 0, f_cal_dj = 0;
    int je_diff, se_diff;
    int dj_exist = 1, sl_exist = 1;
    int result;

    if ((stp_fpxx->isRed == 1) && (atof(stp_Spxx->sl) > 0)) {
        sprintf(errinfo, "���ַ�Ʊ�������Ʒ����ӦΪ����");
        _WriteLog(LL_FATAL, "%s\n", errinfo);
        return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
    }
    if ((stp_fpxx->isRed == 1) && (atof(stp_Spxx->je) > 0)) {
        sprintf(errinfo, "���ַ�Ʊ�������Ʒ���ӦΪ����");
        _WriteLog(LL_FATAL, "%s\n", errinfo);
        return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
    }
    if ((stp_fpxx->isRed == 1) && (atof(stp_Spxx->se) > 0)) {
        sprintf(errinfo, "���ַ�Ʊ�������Ʒ˰��ӦΪ����");
        _WriteLog(LL_FATAL, "%s\n", errinfo);
        return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
    }
    // nisec��100.00ת��Ϊ100��cntax��ת����ת��Ӱ��Ҳ����(�ɰط���ȷ�ϣ�˰��ukey��Ʊ���Ҳ�ǽ�100.00ת����100)
    if ((bDeviceType == DEVICE_NISEC) || (bDeviceType == DEVICE_CNTAX)) {
        MemoryFormart(MF_TRANSLATE_TAILZERO, stp_Spxx->dj, strlen(stp_Spxx->dj));
        if (stp_Spxx->dj[strlen(stp_Spxx->dj) - 1] == '.') {
            stp_Spxx->dj[strlen(stp_Spxx->dj) - 1] = '\0';
        }
        //_WriteLog(LL_INFO, "ת����ĵ���ΪΪ��%s", stp_Spxx->dj);
    }
    if (JudgePrice(stp_Spxx->dj, sizeof(stp_Spxx->dj)) < 0) {
        sprintf(errinfo, "�������Ʒ���۴��ڷǷ��ַ�");
        _WriteLog(LL_FATAL, "%s\n", errinfo);
        return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
    }
    if (JudgePrice(stp_Spxx->sl, sizeof(stp_Spxx->sl)) < 0) {
        sprintf(errinfo, "�������Ʒ�������ڷǷ��ַ�");
        _WriteLog(LL_FATAL, "%s\n", errinfo);
        return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
    }
    if (JudgePrice(stp_Spxx->se, sizeof(stp_Spxx->se)) < 0) {
        sprintf(errinfo, "�������Ʒ˰����ڷǷ��ַ�");
        _WriteLog(LL_FATAL, "%s\n", errinfo);
        return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
    }
    if (JudgePrice(stp_Spxx->je, sizeof(stp_Spxx->je)) < 0) {
        sprintf(errinfo, "�������Ʒ�����ڷǷ��ַ�");
        _WriteLog(LL_FATAL, "%s\n", errinfo);
        return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
    }
    if (JudgePrice(stp_Spxx->slv, sizeof(stp_Spxx->slv)) < 0) {
        sprintf(errinfo, "�������Ʒ˰�ʴ��ڷǷ��ַ�");
        _WriteLog(LL_FATAL, "%s\n", errinfo);
        return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
    }

    if (strcmp(stp_Spxx->je, "0.00") == 0) {
        sprintf(errinfo, "�������߽��Ϊ0����Ʒ");
        _WriteLog(LL_FATAL, "%s\n", errinfo);
        return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
    }
    if ((stp_fpxx->zyfpLx == ZYFP_CEZS) && (atoi(stp_Spxx->hsjbz) != 0)) {
        sprintf(errinfo, "������˰ģʽ���߲������");
        _WriteLog(LL_FATAL, "%s\n", errinfo);
        return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
    }

    char tmp_price[200];
    memset(tmp_price, 0, sizeof(tmp_price));
    PriceRound(stp_Spxx->se, 2, tmp_price);
    memset(stp_Spxx->se, 0, sizeof(stp_Spxx->se));
    strcpy(stp_Spxx->se, tmp_price);

    memset(tmp_price, 0, sizeof(tmp_price));
    PriceRound(stp_Spxx->je, 2, tmp_price);
    memset(stp_Spxx->je, 0, sizeof(stp_Spxx->je));
    strcpy(stp_Spxx->je, tmp_price);

    if ((strcmp(stp_Spxx->dj, "-0.00") == 0) || (strcmp(stp_Spxx->je, "-0.00") == 0) ||
        (strcmp(stp_Spxx->se, "-0.00") == 0)) {
        sprintf(errinfo, "����ĵ��ۡ���˰���Ϊ-0.00");
        _WriteLog(LL_FATAL, "%s\n", errinfo);
        return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
    }
    char cal_tmp[200] = {0};
    memset(cal_tmp, 0, sizeof(cal_tmp));
    if (memcmp(stp_Spxx->dj, cal_tmp, sizeof(stp_Spxx->dj)) == 0)
        dj_exist = 0;
    if (memcmp(stp_Spxx->sl, cal_tmp, sizeof(stp_Spxx->sl)) == 0)
        sl_exist = 0;

    int fphxz = IsZKXX(bDeviceType, stp_Spxx->fphxz);
    if (fphxz == 0 || fphxz == 2)  //��Ϊ�ۿ���
    {
        f_sl = atof(stp_Spxx->sl);     //����
        f_slv = atof(stp_Spxx->slv);   //˰��
        f_bhsje = atof(stp_Spxx->je);  //���
        f_kce = atof(stp_Spxx->kce);   //���
        f_se = atof(stp_Spxx->se);     //˰��
        f_dj = atof(stp_Spxx->dj);     //����

        if ((dj_exist == 0) && (sl_exist == 1)) {
            sprintf(errinfo, "����������δ���뵥��");
            _WriteLog(LL_FATAL, "%s\n", errinfo);
            return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
        } else if ((dj_exist == 1) && (sl_exist == 0)) {
            sprintf(errinfo, "�����뵥��δ��������");
            _WriteLog(LL_FATAL, "%s\n", errinfo);
            return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
        } else if ((dj_exist == 0) && (sl_exist == 0))  //�����뵥������ģʽ
        {
            //_WriteLog(LL_INFO, "stp_Spxx->hsje = %s", stp_Spxx->hsje);
            if (strlen(stp_Spxx->hsje) != 0)  //���뺬˰��ʽ
            {
                f_cal_hsje = atof(stp_Spxx->hsje);
                f_cal_bhsje = (f_cal_hsje - f_kce) / (1 + f_slv) + f_kce;
            } else {
                f_cal_hsje = f_bhsje + f_se;
                f_cal_bhsje = f_bhsje;
            }
            //_WriteLog(LL_INFO, "f_cal_hsje = %4.2f", f_cal_hsje);
            //_WriteLog(LL_INFO, "f_cal_bhsje = %4.2f", f_cal_bhsje);
            f_cal_se = ((f_cal_hsje - f_kce) / (1 + f_slv)) * f_slv;
            //_WriteLog(LL_INFO, "f_cal_se = %4.2f", f_cal_se);
            char s_cal_se[50] = {0};
            sprintf(s_cal_se, "%Lf", f_cal_se);

            memset(tmp_price, 0, sizeof(tmp_price));
            PriceRound(s_cal_se, 2, tmp_price);

            if (strcmp(tmp_price, "-0.00") == 0) {
                memset(s_cal_se, 0, sizeof(s_cal_se));
                strcpy(s_cal_se, "0.00");
            } else {
                memset(s_cal_se, 0, sizeof(s_cal_se));
                strcpy(s_cal_se, tmp_price);
            }
            if ((stp_fpxx->zyfpLx == ZYFP_CEZS) && (stp_fpxx->isRed == 1)) {
                _WriteLog(LL_FATAL, "������պ��ַ�Ʊ��У��˰��");
            } else {
                if (strcmp(stp_Spxx->se, s_cal_se) != 0) {
                    if (stp_fpxx->allow_error > 0) {

                        se_diff = abs(f_cal_se * 100 - f_se * 100);
                        if (se_diff > stp_fpxx->allow_error) {
                            sprintf(errinfo,
                                    "1�޵���ģʽ�����������Ϊ%"
                                    "dģʽ�£������˰����������˰����ڽϴ�������˰��%s,"
                                    "�����˰��%s",
                                    stp_fpxx->allow_error, s_cal_se, stp_Spxx->se);
                            _WriteLog(LL_FATAL, "%s\n", errinfo);
                            return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
                        }
                        if (stp_fpxx->allow_error > 1) {

                            long double d_se_diff = fabs(f_cal_se - f_se);
                            long double d_allow_error =
                                (long double)((long double)stp_fpxx->allow_error / 100);
                            _WriteLog(LL_FATAL, "��ǰ��������ֵ = %f ������� = %f", d_se_diff,
                                      d_allow_error);
                            if (d_se_diff > d_allow_error) {
                                sprintf(errinfo,
                                        "2�޵���ģʽ�����������Ϊ%"
                                        "dģʽ�£������˰����������˰�����������˰��%s,"
                                        "�����˰��%s",
                                        stp_fpxx->allow_error, s_cal_se, stp_Spxx->se);
                                _WriteLog(LL_FATAL, "%s\n", errinfo);
                                return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
                            }
                        }

                        stp_fpxx->d_zsewc += (atof(stp_Spxx->se) - atof(s_cal_se));
                        _WriteLog(LL_INFO,
                                  "�޵���ģʽ,˰��ȣ������������Χ��,����˰��%s,�����˰��%s,"
                                  "�������%d",
                                  s_cal_se, stp_Spxx->se, stp_fpxx->allow_error);
                    } else {
                        sprintf(
                            errinfo,
                            "�޵���ģʽ�������˰����������˰�����������˰��%s,�����˰��%s",
                            s_cal_se, stp_Spxx->se);
                        _WriteLog(LL_FATAL, "%s\n", errinfo);
                        return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
                    }
                }
            }
        } else  //���뵥������ģʽ
        {
            if (strlen(stp_Spxx->hsje) != 0)  //���뺬˰��ʽ
            {
                f_cal_hsje = atof(stp_Spxx->hsje);
                f_cal_bhsje = (f_cal_hsje - f_kce) / (1 + f_slv) + f_kce;
            } else  //������˰���ģʽ
            {
                f_cal_hsje = f_bhsje + f_se;
                if (atoi(stp_Spxx->hsjbz) == 0)  //��Ʒ����˰ģʽ
                {
                    f_cal_bhsje = f_dj * f_sl;
                } else  //��Ʒ��˰ģʽ
                {
                    f_cal_hsje = f_dj * f_sl;
                    char s_cal_hsje[50] = {0};
                    sprintf(s_cal_hsje, "%Lf", f_cal_hsje);
                    //_WriteLog(LL_FATAL, "1���㺬˰���%s", s_cal_hsje);
                    memset(tmp_price, 0, sizeof(tmp_price));

                    PriceRound(s_cal_hsje, 2, tmp_price);
                    //_WriteLog(LL_FATAL, "2���㺬˰���%s", tmp_price);
                    if (strcmp(tmp_price, "-0.00") == 0) {
                        memset(s_cal_hsje, 0, sizeof(s_cal_hsje));
                        strcpy(s_cal_hsje, "0.00");
                    } else {
                        memset(s_cal_hsje, 0, sizeof(s_cal_hsje));
                        strcpy(s_cal_hsje, tmp_price);
                    }
                    f_cal_hsje = atof(s_cal_hsje);
                    //_WriteLog(LL_FATAL, "3���㺬˰���%f", f_cal_hsje);
                    //_WriteLog(LL_FATAL, "4���㺬˰���%s", s_cal_hsje);
                    f_cal_bhsje = f_cal_hsje - f_se;
                    //_WriteLog(LL_FATAL, "��Ʒ��˰ģʽ�����㺬˰���%f", f_cal_bhsje);
                }
            }

            if (DEVICE_AISINO == bDeviceType) {
                if (JudgeDecimalPoint(stp_Spxx->dj, 2, 18) != 0) {
                    sprintf(errinfo, "��˰�̵���С�����λ��С��2λ�����18λ,����%s",
                            stp_Spxx->dj);
                    _WriteLog(LL_FATAL, "%s\n", errinfo);
                    return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
                }
            } else {
                if (JudgeDecimalPoint(stp_Spxx->dj, 0, 18) != 0) {
                    sprintf(errinfo, "˰����/˰��ukey����С�����λ��С��0λ�����18λ,����%s",
                            stp_Spxx->dj);
                    _WriteLog(LL_FATAL, "%s\n", errinfo);
                    return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
                }
            }

            char s_cal_je[50] = {0};
            sprintf(s_cal_je, "%Lf", f_cal_bhsje);
            memset(tmp_price, 0, sizeof(tmp_price));
            PriceRound(s_cal_je, 2, tmp_price);
            if (strcmp(tmp_price, "-0.00") == 0) {
                memset(s_cal_je, 0, sizeof(s_cal_je));
                strcpy(s_cal_je, "0.00");
            } else {
                memset(s_cal_je, 0, sizeof(s_cal_je));
                strcpy(s_cal_je, tmp_price);
            }
            if (strcmp(stp_Spxx->je, s_cal_je) != 0) {
                if (stp_fpxx->allow_error > 0) {
                    je_diff = abs(f_bhsje * 100 - f_cal_bhsje * 100);
                    if (je_diff > stp_fpxx->allow_error) {

                        if (((DEVICE_NISEC == bDeviceType) || (DEVICE_CNTAX == bDeviceType)) &&
                            (stp_fpxx->fplx == FPLX_COMMON_DZFP)) {
                            _WriteLog(LL_INFO,
                                      "˰����˰��Ukey������Ʊ���۾���λ�ϵ�,�ٴ�У�鵥�����");
                            f_cal_dj = f_cal_bhsje / f_sl;
                            char s_cal_dj[50] = {0};
                            sprintf(s_cal_dj, "%Lf", f_cal_dj);
                            memset(tmp_price, 0, sizeof(tmp_price));
                            PriceRound(s_cal_dj, 6, tmp_price);
                            if (strcmp(stp_Spxx->dj, s_cal_dj) != 0) {
                                _WriteLog(LL_INFO,
                                          "˰����˰��Ukey������Ʊ���۾���λ�ϵ�,"
                                          "�ٴ�У�鵥�������Ȼ�������,���㵥��%s,����ĵ���%s",
                                          s_cal_dj, stp_Spxx->dj);
                                sprintf(errinfo,
                                        "�е���ģʽ,"
                                        "���������Ϊ%"
                                        "dģʽ�£�����Ľ�����������ڽϴ���������%s,"
                                        "����Ľ��%s,������%s,����Ľ��%s",
                                        stp_fpxx->allow_error, s_cal_je, stp_Spxx->je, s_cal_je,
                                        stp_Spxx->je);
                                _WriteLog(LL_FATAL, "%s\n", errinfo);
                                return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
                            } else {
                                _WriteLog(LL_INFO,
                                          "˰����˰��Ukey������Ʊ���۾���λ�ϵ�,�ٴ�У�鵥��ͨ��,"
                                          "���㵥��%s,����ĵ���%s",
                                          s_cal_dj, stp_Spxx->dj);
                            }
                        } else {
                            sprintf(errinfo,
                                    "�е���ģʽ,"
                                    "���������ģʽ�£�����Ľ�����������ڽϴ���������%s,"
                                    "����Ľ��%s",
                                    s_cal_je, stp_Spxx->je);
                            _WriteLog(LL_FATAL, "%s\n", errinfo);
                            return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
                        }
                    }
                } else {
                    sprintf(errinfo,
                            "�е���ģʽ,����ĵ����������˻�����1��Ǯ���,��Ʒ����%s,��Ʒ����%s,"
                            "������%s,������%s",
                            stp_Spxx->xh, stp_Spxx->spmc, stp_Spxx->je, s_cal_je);
                    _WriteLog(LL_FATAL, "%s\n", errinfo);
                    return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
                }
            }

            f_cal_se = ((f_cal_hsje - f_kce) / (1 + f_slv)) * f_slv;

            char s_cal_se[50] = {0};
            sprintf(s_cal_se, "%Lf", f_cal_se);
            memset(tmp_price, 0, sizeof(tmp_price));
            PriceRound(s_cal_se, 2, tmp_price);

            if (strcmp(tmp_price, "-0.00") == 0) {
                memset(s_cal_se, 0, sizeof(s_cal_se));
                strcpy(s_cal_se, "0.00");
            } else {
                memset(s_cal_se, 0, sizeof(s_cal_se));
                strcpy(s_cal_se, tmp_price);
            }

            //_WriteLog(LL_FATAL, "����˰��%s,�����˰��%s", s_cal_se, stp_Spxx->se);
            // se_diff = abs(f_cal_se * 100 - f_se * 100);
            // if (se_diff != 0) {
            if ((stp_fpxx->zyfpLx == ZYFP_CEZS) && (stp_fpxx->isRed == 1)) {
                _WriteLog(LL_FATAL, "������պ��ַ�Ʊ��У��˰��");
            } else {
                if (strcmp(stp_Spxx->se, s_cal_se) != 0) {
                    if (stp_fpxx->allow_error > 0) {
                        se_diff = abs(f_cal_se * 100 - f_se * 100);
                        if (se_diff > stp_fpxx->allow_error) {
                            sprintf(errinfo,
                                    "1�е���ģʽ�����������Ϊ%"
                                    "dģʽ�£������˰����������˰�����������˰��%s,"
                                    "�����˰��%s",
                                    stp_fpxx->allow_error, s_cal_se, stp_Spxx->se);
                            _WriteLog(LL_FATAL, "%s\n", errinfo);
                            return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
                        }
                        if (stp_fpxx->allow_error > 1) {

                            long double d_se_diff = fabs(f_cal_se - f_se);
                            long double d_allow_error =
                                (long double)((long double)stp_fpxx->allow_error / 100);
                            _WriteLog(LL_FATAL, "��ǰ��������ֵ = %f ������� = %f", d_se_diff,
                                      d_allow_error);
                            if (d_se_diff > d_allow_error) {
                                sprintf(errinfo,
                                        "2�е���ģʽ�����������Ϊ%"
                                        "dģʽ�£������˰����������˰�����������˰��%s,"
                                        "�����˰��%s",
                                        stp_fpxx->allow_error, s_cal_se, stp_Spxx->se);
                                _WriteLog(LL_FATAL, "%s\n", errinfo);
                                return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
                            }
                        }
                        stp_fpxx->d_zsewc += (atof(stp_Spxx->se) - atof(s_cal_se));
                        //_WriteLog(LL_INFO, "�е���ģʽ,˰��ȣ������������Χ��");
                        _WriteLog(LL_INFO,
                                  "�е���ģʽ,˰��ȣ������������Χ��,����˰��%s,�����˰��%s,"
                                  "�������%d,������ֵ��%d",
                                  s_cal_se, stp_Spxx->se, stp_fpxx->allow_error, se_diff);
                    } else {
                        sprintf(
                            errinfo,
                            "�е���ģʽ�������˰����������˰�����������˰��%s,�����˰��%s",
                            s_cal_se, stp_Spxx->se);
                        _WriteLog(LL_FATAL, "%s\n", errinfo);
                        return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
                    }
                }
            }
            //_WriteLog(LL_FATAL, "��Ʒ��%s%s��У��ɹ�\n", stp_Spxx->spsmmc, stp_Spxx->spmc);
        }
        result = check_dj_len(bDeviceType, stp_fpxx, stp_Spxx, errinfo);
        if (result < 0) {
            return result;
        }
    } else if (fphxz == 1)  //�ۿ���
    {
        if (stp_fpxx->isRed)  //���ַ�Ʊ�ݲ�֧���ۿ�
        {
            sprintf(errinfo, "�ݲ�֧�ֺ��ַ�Ʊ�ۿ���");
            _WriteLog(LL_FATAL, "%s\n", errinfo);
            return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
        }

        if ((dj_exist != 0) || (sl_exist != 0)) {
            sprintf(errinfo, "�ۿ����������뵥�ۺ�����");
            _WriteLog(LL_FATAL, "%s\n", errinfo);
            return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
        }
        if ((strlen(stp_Spxx->ggxh) != 0) || (strlen(stp_Spxx->jldw) != 0)) {
            sprintf(errinfo, "�ۿ��������������ͺźͼ�����λ");
            _WriteLog(LL_FATAL, "%s\n", errinfo);
            return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
        }

        f_bhsje = atof(stp_Spxx->je);  //���
        f_se = atof(stp_Spxx->se);     //���
		f_slv = atof(stp_Spxx->slv);   //˰��

		if ((f_bhsje >= 0) || (f_se > 0)) {
			sprintf(errinfo, "�ۿ��н��ӦΪ������˰��ӦΪ������0");
			_WriteLog(LL_FATAL, "%s\n", errinfo);
			return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
		}


		f_cal_se = f_bhsje * f_slv;
		//_WriteLog(LL_INFO, "f_cal_se = %4.2f", f_cal_se);



		char s_cal_se[50] = { 0 };
		sprintf(s_cal_se, "%Lf", f_cal_se);
		memset(tmp_price, 0, sizeof(tmp_price));
		PriceRound(s_cal_se, 2, tmp_price);

		if (strcmp(tmp_price, "-0.00") == 0) {
			memset(s_cal_se, 0, sizeof(s_cal_se));
			strcpy(s_cal_se, "0.00");
		}
		else {
			memset(s_cal_se, 0, sizeof(s_cal_se));
			strcpy(s_cal_se, tmp_price);
		}

		//_WriteLog(LL_FATAL, "����˰��%s,�����˰��%s", s_cal_se, stp_Spxx->se);
		// se_diff = abs(f_cal_se * 100 - f_se * 100);
		// if (se_diff != 0) {

		if (strcmp(stp_Spxx->se, s_cal_se) != 0) {
			if (stp_fpxx->allow_error > 0) {
				se_diff = abs(f_cal_se * 100 - f_se * 100);
				if (se_diff > stp_fpxx->allow_error) {
					sprintf(errinfo,"�ۿ��У����������Ϊ%dģʽ�£������˰����������˰�����������˰��%s,�����˰��%s",stp_fpxx->allow_error, s_cal_se, stp_Spxx->se);
					_WriteLog(LL_FATAL, "%s\n", errinfo);
					return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
				}
				if (stp_fpxx->allow_error > 1) {
					long double d_se_diff = fabs(f_cal_se - f_se);
					long double d_allow_error =(long double)((long double)stp_fpxx->allow_error / 100);
					_WriteLog(LL_FATAL, "��ǰ��������ֵ = %f ������� = %f", d_se_diff,d_allow_error);
					if (d_se_diff > d_allow_error) {
						sprintf(errinfo,"�ۿ��У����������Ϊ%dģʽ�£������˰����������˰�����������˰��%s,�����˰��%s",stp_fpxx->allow_error, s_cal_se, stp_Spxx->se);
						_WriteLog(LL_FATAL, "%s\n", errinfo);
						return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
					}
				}
				stp_fpxx->d_zsewc += (atof(stp_Spxx->se) - atof(s_cal_se));
				//_WriteLog(LL_INFO, "�е���ģʽ,˰��ȣ������������Χ��");
				_WriteLog(LL_INFO,",˰��ȣ������������Χ��,����˰��%s,�����˰��%s,�������%d,������ֵ��%d",s_cal_se, stp_Spxx->se, stp_fpxx->allow_error, se_diff);
			}else {
				sprintf(errinfo,"�ۿ��У������˰����������˰�����������˰��%s,�����˰��%s",s_cal_se, stp_Spxx->se);
				_WriteLog(LL_FATAL, "%s\n", errinfo);
				return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
			}
		}
    }
    return 0;
}

int check_spxx_info(uint8 bDeviceType, struct Fpxx *stp_fpxx, struct Spxx *stp_Spxx, char *errinfo)
{
    int result;
    if (((stp_fpxx->fplx == FPLX_COMMON_ZYFP) || (stp_fpxx->fplx == FPLX_COMMON_DZZP)) &&
        (strcmp(stp_Spxx->slv, "0.00") == 0)) {
        sprintf(errinfo, "ר�÷�Ʊ����������˰�ʷ�Ʊ");
        _WriteLog(LL_FATAL, "%s\n", errinfo);
        return DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
    }
    if (bDeviceType == DEVICE_AISINO) {
        if (strlen(stp_fpxx->zyspsmmc) + strlen(stp_fpxx->zyspmc) > 92) {
            sprintf(errinfo, "��˰����Ҫ��Ʒ˰Ŀ+��Ҫ��Ʒ���Ƴ��ȳ���92�ֽڴ���");
            _WriteLog(LL_FATAL, "%s\n", errinfo);
            return DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        }
        if (strlen(stp_Spxx->spsmmc) + strlen(stp_Spxx->spmc) > 92) {
            sprintf(errinfo, "��˰����Ʒ˰Ŀ+��Ʒ���Ƴ��ȳ���92�ֽڴ���");
            _WriteLog(LL_FATAL, "%s\n", errinfo);
            return DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        }
    } else if ((bDeviceType == DEVICE_NISEC) || (bDeviceType == DEVICE_CNTAX)) {

        if (strlen(stp_fpxx->zyspsmmc) + strlen(stp_fpxx->zyspmc) > 100) {
            sprintf(errinfo, "˰������Ҫ��Ʒ˰Ŀ+��Ҫ��Ʒ���Ƴ��ȳ���100�ֽڴ���");
            _WriteLog(LL_FATAL, "%s\n", errinfo);
            return DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        }
        if (strlen(stp_Spxx->spsmmc) + strlen(stp_Spxx->spmc) > 100) {
            sprintf(errinfo, "˰������Ʒ˰Ŀ+��Ʒ���Ƴ��ȳ���100�ֽڴ���");
            _WriteLog(LL_FATAL, "%s\n", errinfo);
            return DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        }
    }
    if ((result = check_spxx_lslvbz(stp_Spxx, errinfo)) < 0) {
        return result;
    }
    if ((result = check_spxx_hsjbz(stp_fpxx, stp_Spxx, errinfo)) < 0) {
        return result;
    }
    if ((result = check_spxx_spbh(stp_Spxx->spbh, errinfo)) < 0) {
        return result;
    }
    if ((result = check_spxx_dj_sl_je_se(bDeviceType, stp_fpxx, stp_Spxx, errinfo)) < 0) {
        return result;
    }
    if ((result = check_spxx_yhzcbs_zsfs(bDeviceType, stp_fpxx, stp_Spxx, errinfo)) < 0) {
        return result;
    }
    return 0;
}

int check_fpxx_info(struct Fpxx *stp_fpxx, char *errinfo)
{
    // int result;
    if (JudgePrice(stp_fpxx->slv, sizeof(stp_fpxx->slv)) < 0) {
        sprintf(errinfo, "��Ʊ�ۺ�˰���а����Ƿ��ַ�");
        return DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
    }
    if (JudgePrice(stp_fpxx->je, sizeof(stp_fpxx->je)) < 0) {
        sprintf(errinfo, "��Ʊ�ϼƽ���а����Ƿ��ַ�");
        return DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
    }
    if (JudgePrice(stp_fpxx->se, sizeof(stp_fpxx->se)) < 0) {
        sprintf(errinfo, "��Ʊ�ϼ�˰������Ƿ��ַ�");
        return DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
    }
    if (JudgePrice(stp_fpxx->jshj, sizeof(stp_fpxx->jshj)) < 0) {
        sprintf(errinfo, "��Ʊ��˰�ϼư����Ƿ��ַ�");
        return DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
    }

    char tmp_price[200];
    memset(tmp_price, 0, sizeof(tmp_price));
    PriceRound(stp_fpxx->se, 2, tmp_price);
    memset(stp_fpxx->se, 0, sizeof(stp_fpxx->se));
    strcpy(stp_fpxx->se, tmp_price);

    memset(tmp_price, 0, sizeof(tmp_price));
    PriceRound(stp_fpxx->je, 2, tmp_price);
    memset(stp_fpxx->je, 0, sizeof(stp_fpxx->je));
    strcpy(stp_fpxx->je, tmp_price);

    memset(tmp_price, 0, sizeof(tmp_price));
    PriceRound(stp_fpxx->jshj, 2, tmp_price);
    memset(stp_fpxx->jshj, 0, sizeof(stp_fpxx->jshj));
    strcpy(stp_fpxx->jshj, tmp_price);

    return 0;
}

//������Ʊ�Ƚ�
int Invoice_RBCompare(HFPXX red_fpxx, HFPXX bule_fpxx, char *errifno)
{
    int spsl;
    int i;
    if ((strcmp(red_fpxx->xfmc, bule_fpxx->xfmc) != 0) ||
        (strcmp(red_fpxx->xfsh, bule_fpxx->xfsh) != 0) ||
        (strcmp(red_fpxx->xfdzdh, bule_fpxx->xfdzdh) != 0) ||
        (strcmp(red_fpxx->xfyhzh, bule_fpxx->xfyhzh) != 0)) {
        sprintf(errifno, "���ַ�Ʊ�����ַ�Ʊ������Ϣƥ�䲻һ��");
        return -1;
    }
    if ((strcmp(red_fpxx->gfmc, bule_fpxx->gfmc) != 0) ||
        (strcmp(red_fpxx->gfsh, bule_fpxx->gfsh) != 0) ||
        (strcmp(red_fpxx->gfdzdh, bule_fpxx->gfdzdh) != 0) ||
        (strcmp(red_fpxx->gfyhzh, bule_fpxx->gfyhzh) != 0)) {
        sprintf(errifno, "���ַ�Ʊ�����ַ�Ʊ������Ϣƥ�䲻һ��");
        return -2;
    }
    if (red_fpxx->fplx != bule_fpxx->fplx) {
        sprintf(errifno, "���ַ�Ʊ�����ַ�Ʊ��Ʊ���಻һ��");
        return -3;
    }
    float cal_je;
    float cal_se;
    float cal_jshj;
    cal_je = atof(red_fpxx->je) + atof(bule_fpxx->je);
    cal_se = atof(red_fpxx->se) + atof(bule_fpxx->se);
    cal_jshj = atof(red_fpxx->jshj) + atof(bule_fpxx->jshj);
    if ((cal_je != 0) || (cal_se != 0) || (cal_jshj != 0)) {
        sprintf(errifno, "���ַ�Ʊ�����ַ�Ʊ��˰��ϼƽ�ƥ��");
        return -4;
    }
    if ((strcmp(red_fpxx->skr, bule_fpxx->skr) != 0) ||
        (strcmp(red_fpxx->fhr, bule_fpxx->fhr) != 0) ||
        (strcmp(red_fpxx->kpr, bule_fpxx->kpr) != 0)) {
        sprintf(errifno, "���ַ�Ʊ�����ַ�Ʊ�տ��ˡ������ˡ���Ʊ����Ϣ��һ��");
        return -5;
    }
    if ((strcmp(red_fpxx->zyspsmmc, bule_fpxx->zyspsmmc) != 0) ||
        (strcmp(red_fpxx->zyspmc, bule_fpxx->zyspmc) != 0)) {
        sprintf(errifno, "���ַ�Ʊ�����ַ�Ʊ��Ҫ��Ʒ˰Ŀ�����Ʋ�һ��");
        return -6;
    }
    if (red_fpxx->spsl != bule_fpxx->spsl) {
        sprintf(errifno, "���ַ�Ʊ�����ַ�Ʊ��Ʒ��������һ��");
        return -7;
    }
    spsl = red_fpxx->spsl;
    struct Spxx *spxx_red = red_fpxx->stp_MxxxHead;
    struct Spxx *spxx_blue = bule_fpxx->stp_MxxxHead;
    for (i = 0; i < spsl; i++) {
        float cal_spsl;
        float cal_spje;
        float cal_spse;
        float cal_kce;
        spxx_red = spxx_red->stp_next;
        spxx_blue = spxx_blue->stp_next;
        if ((strcmp(spxx_red->spsmmc, spxx_blue->spsmmc) != 0)) {
            sprintf(errifno, "���ַ�Ʊ�����ַ�Ʊ�ĵ�%d����Ʒ����Ʒ˰Ŀ���Ʋ�һ��", i + 1);
            return -8;
        }
        if ((strcmp(spxx_red->spmc, spxx_blue->spmc) != 0)) {
            sprintf(errifno, "���ַ�Ʊ�����ַ�Ʊ�ĵ�%d����Ʒ����Ʒ���Ʋ�һ��", i + 1);
            return -8;
        }
        if (strcmp(spxx_red->spbh, spxx_blue->spbh) != 0) {
            sprintf(errifno, "���ַ�Ʊ�����ַ�Ʊ�ĵ�%d����Ʒ����Ʒ���벻һ��", i + 1);
            return -9;
        }
        if (strcmp(spxx_red->fphxz, spxx_blue->fphxz) != 0) {
            sprintf(errifno, "���ַ�Ʊ�����ַ�Ʊ�ĵ�%d����Ʒ�ķ�Ʊ�����ʲ�һ��", i + 1);
            return -10;
        }

        char szDJ[64] = "";
        if (atoi(spxx_blue->hsjbz))
            PriceRemoveTax(spxx_blue->dj, spxx_blue->slv, 8, szDJ);
        else
            strcpy(szDJ, spxx_blue->dj);
        if ((strcmp(spxx_red->dj, szDJ) != 0) || (strcmp(spxx_red->slv, spxx_blue->slv) != 0)) {
            sprintf(errifno, "���ַ�Ʊ�����ַ�Ʊ�ĵ�%d����Ʒ�ĵ��ۡ�˰�ʲ�һ��", i + 1);
            return -11;
        }
        cal_spsl = atof(spxx_red->sl) + atof(spxx_blue->sl);
        cal_spje = atof(spxx_red->je) + atof(spxx_blue->je);
        cal_spse = atof(spxx_red->se) + atof(spxx_blue->se);
        cal_kce = atof(spxx_red->kce) + atof(spxx_blue->kce);
        if ((cal_spsl != 0) || (cal_spje != 0) || (cal_spse != 0) || (cal_kce != 0)) {
            sprintf(errifno,
                    "���ַ�Ʊ�����ַ�Ʊ�ĵ�%"
                    "d����Ʒ����������˰�������տ۳��ƥ��",
                    i + 1);
            return -12;
        }
        if ((strcmp(spxx_red->ggxh, spxx_blue->ggxh) != 0) ||
            (strcmp(spxx_red->jldw, spxx_blue->jldw) != 0)) {
            sprintf(errifno, "���ַ�Ʊ�����ַ�Ʊ�ĵ�%d����Ʒ�Ĺ���ͺš�������λ��һ��", i + 1);
            return -13;
        }
        if ((strcmp(spxx_red->xsyh, spxx_blue->xsyh) != 0) ||
            (strcmp(spxx_red->lslvbs, spxx_blue->lslvbs) != 0) ||
            (strcmp(spxx_red->yhsm, spxx_blue->yhsm) != 0)) {
            sprintf(errifno,
                    "���ַ�Ʊ�����ַ�Ʊ�ĵ�%"
                    "d����Ʒ���Ż����߱�ʶ����˰�ʱ�ʶ����ֵ˰�������һ��",
                    i + 1);
            return -14;
        }
    }
    return 0;
}

int CheckFpxx(struct EFpxx *fpxx)
{
    if (JudgePrice(fpxx->slv, sizeof(fpxx->slv)) < 0)
        return SetLastError(fpxx->hDev->hUSB, DF_TAX_ERR_CODE_DATA_PARSING_FAILED,
                            "��Ʊ�ۺ�˰���а����Ƿ��ַ�");
    if (JudgePrice(fpxx->je, sizeof(fpxx->je)) < 0)
        return SetLastError(fpxx->hDev->hUSB, DF_TAX_ERR_CODE_DATA_PARSING_FAILED,
                            "��Ʊ�ϼƽ���а����Ƿ��ַ�");
    if (JudgePrice(fpxx->se, sizeof(fpxx->se)) < 0)
        return SetLastError(fpxx->hDev->hUSB, DF_TAX_ERR_CODE_DATA_PARSING_FAILED,
                            "��Ʊ�ϼ�˰������Ƿ��ַ�");
    if (JudgePrice(fpxx->jshj, sizeof(fpxx->jshj)) < 0)
        return SetLastError(fpxx->hDev->hUSB, DF_TAX_ERR_CODE_DATA_PARSING_FAILED,
                            "��Ʊ��˰�ϼư����Ƿ��ַ�");
    char tmp_price[200];
    memset(tmp_price, 0, sizeof(tmp_price));
    PriceRound(fpxx->se, 2, tmp_price);
    memset(fpxx->se, 0, sizeof(fpxx->se));
    strcpy(fpxx->se, tmp_price);

    memset(tmp_price, 0, sizeof(tmp_price));
    PriceRound(fpxx->je, 2, tmp_price);
    memset(fpxx->je, 0, sizeof(fpxx->je));
    strcpy(fpxx->je, tmp_price);

    memset(tmp_price, 0, sizeof(tmp_price));
    PriceRound(fpxx->jshj, 2, tmp_price);
    memset(fpxx->jshj, 0, sizeof(fpxx->jshj));
    strcpy(fpxx->jshj, tmp_price);

    return RET_SUCCESS;
}


int CheckFpxxMandatoryField(HFPXX fpxx, uint8 bDeviceType, char *errifno)
{
    //�Ϸ�Ϊ��Բ�ͬ�豸��⣬�·�Ϊͨ�ü��
    if ((fpxx->fplx == FPLX_COMMON_ZYFP) || (fpxx->fplx == FPLX_COMMON_DZZP)) {  //רƱ
        if (!fpxx->isRed) {                                                      //����Ʊ
            if (!strlen(fpxx->gfsh) || !strlen(fpxx->gfdzdh) || !strlen(fpxx->xfdzdh)) {
                sprintf(errifno, "��ֵ˰ר�÷�Ʊ����д����˰�š�������ַ�绰��������ַ�绰");
                _WriteLog(LL_WARN, "Zyfp gf info error");
                return -1;
            }
        } else {  //��Ʊ�����߼����������Ϣ��
            if (!strlen(fpxx->gfsh)) {
                sprintf(errifno, "����ȷ���빺��˰��");
                _WriteLog(LL_WARN, "%s", errifno);
                return -1;
            }
            if (fpxx->hzxxbsqsm != 1) {
                //�����������ѵֿۺ�����Ϣ������۷����߹����ѵֿۺ�����Ϣ��ʱ�����ַ�Ʊ�������
                if (!strlen(fpxx->blueFpdm) || !strlen(fpxx->blueFphm)) {
                    sprintf(errifno, "����ȷ�������ַ�Ʊ�������");
                    _WriteLog(LL_WARN, "%s", errifno);
                    return -1;
                }
            }
            if (!fpxx->isHzxxb) {  //(�����������Ϣ��)
                if (!strlen(fpxx->xfdzdh)) {
                    sprintf(errifno, "����ȷ����������ַ�绰");
                    _WriteLog(LL_WARN, "%s", errifno);
                    return -1;
                }
                if ((DEVICE_NISEC == bDeviceType) ||
                    (DEVICE_CNTAX ==
                     bDeviceType)) {  //����ϵ˰��Ҫ���Ʊ����Ʊ���߾�Ҫ��д������ַ�绰
                    if (!strlen(fpxx->gfdzdh)) {
                        sprintf(errifno, "����ȷ���빺����ַ�绰");
                        _WriteLog(LL_WARN, "%s", errifno);
                        return -1;
                    }
                    /*if (fpxx->hzxxbsqsm != 0) {
                            sprintf(errifno,
                    "�ݲ�֧��˰����˰��ukey���߹�������ĺ�����Ϣ��Ʊ"); _WriteLog(LL_WARN,
                    "%s", errifno); return -1;
                    }*/
                }
            }
        }

    } else {  //��Ʊ/��Ʊ
        // ��ר�÷�Ʊgf˰�ſ�Ϊ��,Ŀǰ�Ѳ���aisino����15��0��nisec�ÿգ�ukey��δ���ԣ�����15��0ʱҲ���������Ҳ�����
        if (!strlen(fpxx->gfsh) && DEVICE_NISEC != bDeviceType)
            strcpy(fpxx->gfsh, "000000000000000");

        if ((DEVICE_NISEC == bDeviceType) ||
            (DEVICE_CNTAX == bDeviceType)) {  //����ϵ˰��Ҫ���Ʊ����Ʊ���߾�Ҫ��д������ַ�绰
            if (!strlen(fpxx->xfdzdh)) {
                sprintf(errifno, "����ȷ����������ַ�绰");
                _WriteLog(LL_WARN, "%s", errifno);
                return -1;
            }
        }

        if (fpxx->isRed) {  //����ƱƱ
            if (!strlen(fpxx->blueFpdm) || !strlen(fpxx->blueFphm)) {
                sprintf(errifno, "����ȷ�������ַ�Ʊ�������");
                _WriteLog(LL_WARN, "%s", errifno);
                return -1;
            }
        }
    }

    if (!fpxx->isHzxxb) {  //(�����������Ϣ��)
        if (!strlen(fpxx->kpr)) {
            sprintf(errifno, "����ȷ���뿪Ʊ������");
            _WriteLog(LL_WARN, "%s", errifno);
            return -1;
        }
    }


    return 0;
}

int CheckFpxxNode(HFPXX fpxx, HDEV hDev, char *errifno)
{
    if (DEVICE_AISINO == hDev->bDeviceType) {          // aisino
    } else if (DEVICE_NISEC == hDev->bDeviceType) {    // nisec
    } else if (DEVICE_CNTAX == hDev->bDeviceType) {    // cntax
    } else if (DEVICE_MENGBAI == hDev->bDeviceType) {  // mengbai
    } else {
        sprintf(errifno, "δ֪�豸!");
        _WriteLog(LL_WARN, "Unknow device");
        return -1;
    }
    if (fpxx->fplx == FPLX_COMMON_DZFP) {
        if (fpxx->need_ofdurl == 1) {
            if (strlen(fpxx->hDev->szPubServer) == 0) {
                sprintf(errifno, "δ��ͨ���ӷ�Ʊ��������ƽ̨,�޷����߹�������ƽ̨��Ʊ");
                _WriteLog(LL_WARN, "%s", errifno);
                return -2;
            }
        }
    }
    if ((fpxx->fplx == FPLX_COMMON_DZZP) && (DEVICE_CNTAX != hDev->bDeviceType)) {
        sprintf(errifno, "��˰��ukey����˰���ݲ�֧�ֵ���רƱ����");
        return -1;
    }

    if (((DEVICE_NISEC == hDev->bDeviceType) || (DEVICE_CNTAX == hDev->bDeviceType)) &&
        !strcmp(fpxx->slv, "99.01")) {
        _WriteLog(LL_DEBUG, "Nisec multi slv, force set slv to 99.01");
    } else if (atof(fpxx->slv) > 0.17) {
        _WriteLog(LL_WARN, "Slv > 0.17");
        sprintf(errifno, "��Ʊ˰�ʲ��ó���17%%");
        return -2;
    }
    struct Spxx *spxxchild = fpxx->stp_MxxxHead;
    while (spxxchild->stp_next) {
        spxxchild = spxxchild->stp_next;
        if (strlen(spxxchild->spmc) == 0) {

            if ((strcmp(spxxchild->spsmmc, "ԭ�ۺϼ�") == 0) ||
                (strcmp(spxxchild->spsmmc, "�ۿ۶�ϼ�") == 0)) {
                ;
            } else {
                _WriteLog(LL_WARN, "Spmc must be filled");
                sprintf(errifno, "��Ʒ���Ʊ�����ȷ��д");
                return -3;
            }
        }
        if ((strcmp(spxxchild->spmc, "(�����Ӧ������Ʊ�嵥)") != 0) &&
            (strcmp(spxxchild->spmc, "�����Ӧ������Ʊ���嵥") != 0)) {
            if (strlen(spxxchild->spsmmc) == 0) {


                _WriteLog(LL_WARN, "spsmmc must be filled");
                sprintf(errifno, "��Ʒ˰Ŀ���Ʊ�����ȷ��д");
                return -4;
            }
            if (spxxchild->spsmmc[0] != '*' ||
                spxxchild->spsmmc[strlen(spxxchild->spsmmc) - 1] != '*') {

                if ((strcmp(spxxchild->spsmmc, "ԭ�ۺϼ�") == 0) ||
                    (strcmp(spxxchild->spsmmc, "�ۿ۶�ϼ�") == 0)) {
                    ;
                } else {
                    _WriteLog(LL_WARN, "spsmmc must be format of '*XXX*'");
                    sprintf(errifno, "��Ʒ���Ʊ�����ȷ��д,*˰Ŀ����*");
                    return -4;
                }
            }
        }

        if (strlen(spxxchild->slv) == 0) {
            if ((strcmp(spxxchild->spsmmc, "ԭ�ۺϼ�") == 0) ||
                (strcmp(spxxchild->spsmmc, "�ۿ۶�ϼ�") == 0) ||
                (strcmp(spxxchild->spmc, "�����Ӧ������Ʊ���嵥") == 0)) {
                ;
            } else {
                sprintf(errifno, "��Ʊ����Ϣ��������˰��");
                _WriteLog(LL_WARN, "Slv must be filled");
                return -7;
            }
        }
        if (strlen(spxxchild->se) == 0) {
            sprintf(errifno, "��Ʊ����Ϣ��������˰��");
            _WriteLog(LL_WARN, "Se must be filled");
            return -8;
        }
        if (strlen(spxxchild->je) == 0) {
            sprintf(errifno, "��Ʊ����Ϣ����������");
            _WriteLog(LL_WARN, "Je must be filled");
            return -9;
        }
        //��巢ƱУ��
        if (fpxx->isRed) {
            if (atoi(spxxchild->sl) > 0) {
                sprintf(errifno, "��巢Ʊ����Ӧ��Ϊ����");
                _WriteLog(LL_WARN, "Red invoice Sl must be <0");
                return -6;
            }
            if (atof(spxxchild->se) > 0) {
                sprintf(errifno, "��巢Ʊ˰��Ӧ��Ϊ����");
                _WriteLog(LL_WARN, "Red invoice SE must be <0");
                return -8;
            }
            if (atof(spxxchild->je) > 0) {
                sprintf(errifno, "��巢Ʊ���Ӧ��Ϊ����");
                _WriteLog(LL_WARN, "Red invoice JE must be <0");
                return -9;
            }
        }
        // if (spxxchild->stp_next == NULL)
        //     break;
    }
    if (!strlen(fpxx->xfsh) || !strlen(fpxx->gfmc) || !strlen(fpxx->xfmc)) {
        sprintf(errifno, "δ��ȷ��д��������Ϣ");
        _WriteLog(LL_WARN, "Need mc and sh");
        return -10;
    }

    //����й���˰�Ų��ж�
    if (strlen(fpxx->gfsh) && ((strlen(fpxx->gfsh) < 15) || strlen(fpxx->gfsh) > 20)) {
        sprintf(errifno, "������Ϣ˰�ų��ȴ���");
        _WriteLog(LL_WARN, "TaxID's gfsh length must be [15,20]");
        return -10;
    }

    if ((strlen(fpxx->xfsh) != 15) && (strlen(fpxx->xfsh) != 17) && (strlen(fpxx->xfsh) != 18) &&
        (strlen(fpxx->xfsh) != 20)) {
        sprintf(errifno, "������Ϣ˰�ų��ȴ���");
        _WriteLog(LL_WARN, "TaxID's xfsh length must be [15,20]");
        return -10;
    }
    //����˰�� ��= ����˰��
    // if (strcmp((const char *)fpxx->gfsh, (const char *)hDev->szCommonTaxID) == 0) {
    //    _WriteLog(LL_DEBUG, "gfsh = %s xfsh=%s", fpxx->gfsh, hDev->szCommonTaxID);
    //    sprintf(errifno, "������Ϣ���õ���������Ϣ");
    //    return -10;
    //}
    // if (!strlen(fpxx->kpr) || strlen(fpxx->kpr) > 15) {
    //    sprintf(errifno, "��Ʊ�˳��ȴ���,5������");
    //    _WriteLog(LL_WARN, "kpr incorrect, 5word");  // kpr 5 hanzi max
    //    return -11;
    //}
    if (fpxx->isHzxxb == true && fpxx->hzxxbsqsm != 0) {
        _WriteLog(LL_WARN, "������Ϣ������ʱ������������У�����������Ϣ�Ƿ�ƥ��");
    } else {
        if (strcmp(fpxx->xfsh, hDev->szCommonTaxID) || strcmp(fpxx->xfmc, hDev->szCompanyName)) {
            sprintf(errifno, "������Ϣ���豸��Ϣ��ƥ��");
            _WriteLog(LL_WARN, "Import XFSH or XFTaxID not match the device's config");
            return -12;
        }
    }
    if (!strlen(fpxx->zyspmc)) {
        sprintf(errifno, "��Ҫ��Ʒ���Ʋ���Ϊ��");
        _WriteLog(LL_WARN, "Zyspmc == null");
        return -13;
    }
    if (strlen(fpxx->bz) > 230 || strlen(fpxx->gfdzdh) > 100 || strlen(fpxx->gfyhzh) > 100) {
        sprintf(errifno,
                "��ע��Ϣ�򹺷���ַ�绰�������˺����ݳ��ȹ���,�ײ�涨����ע230,"
                "������ַ100,��������100,ʵ�ʳ��ȣ���ע%d,������ַ%d,��������%d",
                (int)strlen(fpxx->bz), (int)strlen(fpxx->gfdzdh), (int)strlen(fpxx->gfyhzh));
        _WriteLog(LL_WARN, "bz gfdzdh gfyhzh length error");
        return -14;
    }
    //��巢ƱУ��
    double nC1, nC2, nC3, nC4;
    nC1 = atof(fpxx->je);
    nC2 = atof(fpxx->se);
    nC3 = atof(fpxx->jshj);
    nC4 = atof(fpxx->slv);
    if (fpxx->isRed) {
        if (nC1 >= 0 || nC2 > 0 || nC3 >= 0 || nC4 < 0) {
            sprintf(errifno, "���ַ�Ʊ���˰���˰�ϼ�ӦС�ڵ�����,˰��Ӧ������");
            _WriteLog(LL_WARN, "red invoice je se slv check failed");
            return -15;
        }
    } else {
        if (nC1 < 0 || nC2 < 0 || nC3 < 0 || nC4 < 0) {
            sprintf(errifno, "���ַ�Ʊ���˰���˰�ϼ�Ӧ���ڵ�����");
            _WriteLog(LL_WARN, "blue invoice je se slv check failed");
            return -18;
        }
    }
    return 0;
}

//---------------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------------
//-----------------------------------------������������������������������������------------------------------------------------------
//-----------------------------------------------�����ķָ���------------------------------------------------------------
//-----------------------------------------��������������������̡���������------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------------


//��4��check����ֻ����⣬����fpxx�������Ķ�

//������ע��
//�ú����ڲ���Ҫ�����ۣ���Ϊ�����Ѿ���ʼ������
//Ŀǰ�Ȱѿ�ܶ�����������ʣ������������������
int CheckNisecSpxx(HFPXX stp_Root, struct Spxx *stp_InsertNode, char *errinfo)
{
    if (stp_Root || stp_InsertNode || errinfo) {
    }
    return 0;
}

//������ע��
//�ú����ڲ���Ҫ�����ۣ���Ϊ�����Ѿ���ʼ������
//Ŀǰ�Ȱѿ�ܶ�����������ʣ������������������
int CheckAisinoSpxx(HFPXX stp_Root, struct Spxx *stp_InsertNode, char *errinfo)
{
    if (stp_Root || stp_InsertNode || errinfo) {
    }
    return 0;
}

int CheckCntaxFpxx(HFPXX stp_fpxx, char *errinfo)
{
    int nRet = -1;
    // if ((strlen(stp_fpxx->skr) > 16) || (strlen(stp_fpxx->fhr) > 16) ||
    //    (strlen(stp_fpxx->kpr) > 16)) {
    //    sprintf(errinfo, "˰�����տ��ˡ������ˡ���Ʊ����󳤶Ȳ��ó���16�ֽ�");
    //    goto Err;
    //}

    // if (strlen(stp_fpxx->zyspsmmc) + strlen(stp_fpxx->zyspmc) > 100) {
    //    sprintf(errinfo, "˰������Ҫ��Ʒ˰Ŀ+��Ҫ��Ʒ���Ƴ��ȳ���100�ֽڴ���");
    //    goto Err;
    //}
    // if (stp_fpxx->fplx_aisino == FPLX_AISINO_ZYFP) {
    //    if (strlen(stp_fpxx->bz) > 184) {
    //        sprintf(errinfo, "˰����ר�÷�Ʊ��ע���ô���184�ֽ�");
    //        goto Err;
    //    }
    //} else if (stp_fpxx->fplx_aisino == FPLX_AISINO_PTFP) {
    //    if (strlen(stp_fpxx->bz) > 138) {
    //        sprintf(errinfo, "˰������ͨ��Ʊ��ע���ô���138�ֽ�");
    //
    //        goto Err;
    //    }
    //} else if (stp_fpxx->fplx_aisino == FPLX_AISINO_DZFP) {
    //    if (strlen(stp_fpxx->bz) > 130) {
    //        sprintf(errinfo, "˰���̵��ӷ�Ʊ��ע���ô���130�ֽ�");
    //        goto Err;
    //    }
    //}
    nRet = 0;
    // Err:
    return nRet;
}


int CheckNisecFpxx(HFPXX stp_fpxx, char *errinfo)
{
    int nRet = -1;
    // if ((strlen(stp_fpxx->skr) > 16) || (strlen(stp_fpxx->fhr) > 16) ||
    //    (strlen(stp_fpxx->kpr) > 16)) {
    //    sprintf(errinfo, "˰�����տ��ˡ������ˡ���Ʊ����󳤶Ȳ��ó���16�ֽ�");
    //    goto Err;
    //}

    // if (strlen(stp_fpxx->zyspsmmc) + strlen(stp_fpxx->zyspmc) > 100) {
    //    sprintf(errinfo, "˰������Ҫ��Ʒ˰Ŀ+��Ҫ��Ʒ���Ƴ��ȳ���100�ֽڴ���");
    //    goto Err;
    //}
    // if (stp_fpxx->fplx_aisino == FPLX_AISINO_ZYFP) {
    //    if (strlen(stp_fpxx->bz) > 184) {
    //        sprintf(errinfo, "˰����ר�÷�Ʊ��ע���ô���184�ֽ�");
    //        goto Err;
    //    }
    //} else if (stp_fpxx->fplx_aisino == FPLX_AISINO_PTFP) {
    //    if (strlen(stp_fpxx->bz) > 138) {
    //        sprintf(errinfo, "˰������ͨ��Ʊ��ע���ô���138�ֽ�");
    //
    //        goto Err;
    //    }
    //} else if (stp_fpxx->fplx_aisino == FPLX_AISINO_DZFP) {
    //    if (strlen(stp_fpxx->bz) > 130) {
    //        sprintf(errinfo, "˰���̵��ӷ�Ʊ��ע���ô���130�ֽ�");
    //        goto Err;
    //    }
    //}
    nRet = 0;
    // Err:
    return nRet;
}

int CheckAisinoFpxx(HFPXX stp_fpxx, char *errinfo)
{
    int nRet = -1;


    // if (strlen(stp_fpxx->zyspsmmc) + strlen(stp_fpxx->zyspmc) > 92) {
    //    sprintf(errinfo, "��˰����Ҫ��Ʒ˰Ŀ+��Ҫ��Ʒ���Ƴ��ȳ���92�ֽڴ���");
    //    goto Err;
    //}
    // if (strlen(stp_fpxx->bz) > 230) {
    //    sprintf(errinfo, "��˰�̷�Ʊ��ע���ô���230�ֽ�");
    //    goto Err;
    //}
    nRet = 0;
    // Err:
    return nRet;
}

//������Ʊjson����
int AnalyzeJsonBuff(uint8 bDeviceType, char *json_data, HFPXX stp_fpxx, char *errinfo)
{
    cJSON *root = NULL, *jsp_ArraySpxx = NULL;
    struct Spxx *stp_Spxx = NULL;
    int i_ErrNum = 0, d_slv_first = -1, i_count, i_hsjbz = 0, nTmpVar = 0;
    double d_slv = 0, d_zje = 0, d_zse = 0, d_jshj = 0, d_yjhjje = 0, d_yjhjse = 0, d_zkehjje = 0,
           d_zkehjse = 0;
    char SamSlvFlag = 1, exsit_zkxx = 0, use_fpdmhm = 0;
    int d_lslv_first = -1, d_lslv = 0;
    char SamLSlvFlag = 1;
    char szBuf[512] = "";
    char json_err[100] = {0};
    if (!json_data || !stp_fpxx)
        return -1;  //��������
    root = cJSON_Parse((const char *)json_data);
    if (!root) {
        _WriteLog(LL_FATAL, "JSON Parse json_data failed\n");
        sprintf(errinfo, "��ƱJSON���ݸ�ʽ��������");
        i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        goto Err;
    }
    //��Ʊ����
    memset(szBuf, 0, sizeof szBuf);
    if (GetAvailImportData(root, "FPZLS", szBuf, 3, true, json_err) < 0) {
        _WriteLog(LL_FATAL, "JSON Parse FPZLS failed\n");
        sprintf(errinfo, "��ƱJSON�����з�Ʊ����(fpzls)��������,%s", json_err);
        i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        goto Err;
    }
    if ((strcmp(szBuf, "004") != 0) && (strcmp(szBuf, "007") != 0) && (strcmp(szBuf, "026") != 0) &&
        (strcmp(szBuf, "028") != 0)) {
        if (!strcmp(szBuf, "028") && bDeviceType != DEVICE_CNTAX) {
            _WriteLog(LL_FATAL, "028 Just support cntax");
            sprintf(errinfo, "����רƱĿǰ��֧��Ukey");
            i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
            goto Err;
        }
        _WriteLog(LL_FATAL, "JSON Parse FPZLS did not 004 007 026 028 failed\n");
        sprintf(errinfo,
                "��Ʊ����Ŀǰ��֧��רƱ����Ʊ�����ա���ר��004 007 026 028���������ͷ�Ʊ����");
        i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        goto Err;
    }
    stp_fpxx->fplx = atoi(szBuf);
    //���Ǽ��ݣ�ͬʱ����aisino��Ʊ������Ϣ
    //�������о�������aisinoΪ����㣬�����Ѿ����ͣ���˸ú����������еķ�Ʊ�ж϶�����aisino��Ʊ����Ϊ��Ҫ�ж����ݣ���Ҫ�ټ���ͨ��fplx���ж�
    CommonFPLX2AisinoFPLX(stp_fpxx->fplx, &stp_fpxx->fplx_aisino);
    //��Ʊ������ˮ��
    if (GetAvailImportData(root, "FPQQLSH", stp_fpxx->fpqqlsh, 120, false, json_err) < 0) {
        _WriteLog(LL_FATAL, "JSON Parse FPQQLSH failed\n");
        sprintf(errinfo, "��ƱJSON�����з�Ʊ������ˮ��(fpqqlsh)��������,%s", json_err);
        i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        goto Err;
    }
    memset(szBuf, 0, sizeof szBuf);
    if (GetAvailImportData(root, "ofdurl", szBuf, 3, false, json_err) < 0) {
        stp_fpxx->need_ofdurl = 0;
    } else {
        stp_fpxx->need_ofdurl = atoi(szBuf);
    }

    //��Ʊ����汾��
    if (GetAvailImportData(root, "KPRJBBH", stp_fpxx->kprjbbh, 31, false, json_err) < 0) {
        _WriteLog(LL_FATAL, "JSON Parse KPRJBBH failed\n");
        sprintf(errinfo, "��ƱJSON�����п�Ʊ����汾��(kprjbbh)��������,%s", json_err);
        i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        goto Err;
    }
    //˰�̱�� jqbh
    if (GetAvailImportData(root, "SPBH", stp_fpxx->jqbh, 12, true, json_err) < 0) {
        _WriteLog(LL_FATAL, "JSON Parse jqbh-SPBH\n");
        sprintf(errinfo, "��ƱJSON������˰�̱��(spbh)��������,%s", json_err);
        i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        goto Err;
    }
    //��Ʊ����(����������)
    memset(szBuf, 0, sizeof szBuf);
    if (GetAvailImportData(root, "KPLX", szBuf, 3, true, json_err) < 0) {
        _WriteLog(LL_FATAL, "JSON Parse KPLX failed\n");
        sprintf(errinfo, "��ƱJSON�����п�Ʊ����(kplx)��������,%s", json_err);
        i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        goto Err;
    }
    if (strcasecmp(szBuf, "0") && strcasecmp(szBuf, "1")) {
        _WriteLog(LL_FATAL, "KPLX 0/1, blue or red\n");
        sprintf(errinfo, "��Ʊ���ʹ��󣬷����ַ�Ʊ���Ǻ��ַ�Ʊ");
        i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        goto Err;
    }
    strcpy(stp_fpxx->kplx, szBuf);  //���ݴ���
    stp_fpxx->isRed = atoi(szBuf);
    //�����汾��
    if (GetAvailImportData(root, "BMBBBH", stp_fpxx->bmbbbh, 8, true, json_err) < 0) {
        _WriteLog(LL_FATAL, "JSON Parse BMBBBH failed\n");
        sprintf(errinfo, "��ƱJSON�����б����汾��(bmbbbh)��������,%s", json_err);
        i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        goto Err;
    }
    //�ۺ�˰�ʵ���ӿ�,�����޸�ֻ�ܱ���zhsl,�ڲ�ֵ��Ϊslv
    if (GetAvailImportData(root, "ZHSL", stp_fpxx->slv, 10, false, json_err) < 0) {
        _WriteLog(LL_FATAL, "JSON Parse SLV failed\n");
        sprintf(errinfo, "��ƱJSON�������ۺ�˰��(zhsl)��������,%s", json_err);
        i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        goto Err;
    }
    //������Ϣ������˵��
    GetAvailImportData(root, "HZXXBSQSM", szBuf, 3, false, json_err);
    stp_fpxx->hzxxbsqsm = atoi(szBuf);

    if ((stp_fpxx->hzxxbsqsm != 0) && (stp_fpxx->isHzxxb == true)) {
        if (stp_fpxx->isRed != 1) {
            sprintf(errinfo, "��������Ϣ��������忪��ʱ���������Ϣ������˵��");
            i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
            goto Err;
        }
        if ((stp_fpxx->fplx != FPLX_COMMON_ZYFP) && (stp_fpxx->fplx != FPLX_COMMON_DZZP)) {
            sprintf(errinfo, "��ֽ��רƱ�����רƱ���������Ϣ������˵��");
            i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
            goto Err;
        }
    }
    //��������
    if (GetAvailImportData(root, "GFMC", stp_fpxx->gfmc, 100, true, json_err) < 0) {
        _WriteLog(LL_FATAL, "JSON Parse GFMC failed\n");
        sprintf(errinfo, "��ƱJSON�����й�������(gfmc)��������,%s", json_err);
        i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        goto Err;
    }
    if ((stp_fpxx->fplx == FPLX_COMMON_ZYFP) || (stp_fpxx->fplx == FPLX_COMMON_DZZP)) {
        //����˰��
        if (GetAvailImportData(root, "GFSH", stp_fpxx->gfsh, 20, true, json_err) < 0) {
            _WriteLog(LL_FATAL, "JSON Parse GFSH failed\n");
            sprintf(errinfo, "��ƱJSON�����й���˰��(gfsh)��������,%s", json_err);
            i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
            goto Err;
        }
        if ((strlen(stp_fpxx->gfsh) != 15) && (strlen(stp_fpxx->gfsh) != 17) &&
            (strlen(stp_fpxx->gfsh) != 18) && (strlen(stp_fpxx->gfsh) != 20)) {
            _WriteLog(LL_FATAL, "JSON Parse GFSH len error failed\n");
            sprintf(errinfo, "��ƱJSON�����й���˰�ŷ�15��17��18��20λ");
            i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
            goto Err;
        }
        if (JudgeTaxID(stp_fpxx->gfsh) < 0) {
            _WriteLog(LL_FATAL, "JSON Parse GFSH len error failed\n");
            sprintf(errinfo, "��ƱJSON�����й���˰�Ź�����ڴ���,"
                             "��⵽���ڳ���д��ĸ��������������ַ�");
            i_ErrNum = DF_TAX_ERR_CODE_CAN_NOT_USE_IOZSV;
            goto Err;
        }
    } else if ((stp_fpxx->fplx == FPLX_COMMON_PTFP) ||
               (stp_fpxx->fplx ==
                FPLX_COMMON_DZFP))  //��Ʊ����Ʊ����˰�š���ַ�绰�������˺ſ�Ϊ�գ��ݲ�֧��˰�ſգ�
    {
        //����˰��
        if (GetAvailImportData(root, "GFSH", stp_fpxx->gfsh, 20, false, json_err) < 0) {
            _WriteLog(LL_FATAL, "JSON Parse GFSH failed\n");
            sprintf(errinfo, "��ƱJSON�����й���˰��(gfsh)��������,%s", json_err);
            i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
            goto Err;
        }
        if (strlen(stp_fpxx->gfsh) != 0) {
            // aisino��֧�ַǱ�׼˰�ſ���(��������+�ֻ�����)������˰���ݲ�֧��
            // if (DEVICE_AISINO != bDeviceType && (strlen(stp_fpxx->gfsh) != 15) &&
            //    (strlen(stp_fpxx->gfsh) != 17) && (strlen(stp_fpxx->gfsh) != 18) &&
            //    (strlen(stp_fpxx->gfsh) != 20)) {
            //    _WriteLog(LL_FATAL, "JSON Parse GFSH len error failed\n");
            //    sprintf(errinfo, "��ƱJSON�����й���˰�ŷ�15��17��18��20λ");
            //    i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
            //    goto Err;
            //}
            if (JudgeTaxID(stp_fpxx->gfsh) < 0) {
                _WriteLog(LL_FATAL, "JSON Parse GFSH len error failed\n");
                sprintf(errinfo, "��ƱJSON�����й���˰�Ź�����ڴ���,"
                                 "��⵽���ڳ���д��ĸ��������������ַ�");
                i_ErrNum = DF_TAX_ERR_CODE_CAN_NOT_USE_IOZSV;
                goto Err;
            }
        }
    }

    //������ַ�绰
    if (GetAvailImportData(root, "GFDZDH", stp_fpxx->gfdzdh, 100, false, json_err) < 0) {
        _WriteLog(LL_FATAL, "JSON Parse GFDZDH failed\n");
        sprintf(errinfo, "��ƱJSON�����й�����ַ�绰(gfdzdh)��������,%s", json_err);
        i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        goto Err;
    }
    //���������˺�
    if (GetAvailImportData(root, "GFYHZH", stp_fpxx->gfyhzh, 100, false, json_err) < 0) {
        _WriteLog(LL_FATAL, "JSON Parse GFYHZH failed\n");
        sprintf(errinfo, "��ƱJSON�����й��������˺�(gfyhzh)��������,%s", json_err);
        i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        goto Err;
    }
    //��������
    if (GetAvailImportData(root, "XFMC", stp_fpxx->xfmc, 100, true, json_err) < 0) {
        _WriteLog(LL_FATAL, "JSON Parse XFMC failed\n");
        sprintf(errinfo, "��ƱJSON��������������(xfmc)��������,%s", json_err);
        i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        goto Err;
    }
    if (strlen(stp_fpxx->xfmc) < 1) {
        sprintf(errinfo, "��ƱJSON�������������Ƴ��ȴ���");
        i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        goto Err;
    }
    //����˰��
    if (GetAvailImportData(root, "XFSH", stp_fpxx->xfsh, 20, true, json_err) < 0) {
        _WriteLog(LL_FATAL, "JSON Parse XFSH failed\n");
        sprintf(errinfo, "��ƱJSON����������˰��(xfsh)��������,%s", json_err);
        i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        goto Err;
    }
    //������ַ�绰
    if (GetAvailImportData(root, "XFDZDH", stp_fpxx->xfdzdh, 100, false, json_err) < 0) {
        _WriteLog(LL_FATAL, "JSON Parse XFDZDH failed\n");
        sprintf(errinfo, "��ƱJSON������������ַ�绰(xfdzdh)��������,%s", json_err);
        i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        goto Err;
    }
    //���������˺�
    if (GetAvailImportData(root, "XFYHZH", stp_fpxx->xfyhzh, 100, false, json_err) < 0) {
        _WriteLog(LL_FATAL, "JSON Parse XFYHZH failed\n");
        sprintf(errinfo, "��ƱJSON���������������˺�(xfyhzh)��������,%s", json_err);
        i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        goto Err;
    }
    //Ӫҵ˰��ʶ����ȫ��0
    strcpy(stp_fpxx->yysbz, "00000000000000000000");
    //��˰�۱�־ 0/1/2
    if (GetAvailImportData(root, "HSJBZ", stp_fpxx->hsjbz, 3, true, json_err) < 0) {
        _WriteLog(LL_FATAL, "JSON Parse HSJBZ failed\n");
        sprintf(errinfo, "��ƱJSON�����к�˰�۱�־(hsjbz)��������,%s", json_err);
        i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        goto Err;
    }
    if (!strlen(stp_fpxx->hsjbz))
        strcpy(stp_fpxx->hsjbz, "0");
    i_hsjbz = atoi(stp_fpxx->hsjbz);
    if ((i_hsjbz != 0) && (i_hsjbz != 1)) {
        sprintf(errinfo, "Ŀǰ��֧����Ʒȫ����˰�򲻺�˰��Ʊ");
        _WriteLog(LL_FATAL, "JSON Parse HSJBZ Ŀǰ��֧����Ʒȫ����˰�򲻺�˰��Ʊ\n");
        i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        goto Err;
    }
    //�ϼƽ��
    if (GetAvailImportData(root, "HJJE", stp_fpxx->je, 20, true, json_err) < 0) {
        _WriteLog(LL_FATAL, "JSON Parse HJJE failed\n");
        sprintf(errinfo, "��ƱJSON�����кϼƽ��(hjje)��������,%s", json_err);
        i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        goto Err;
    }
    //�ϼ�˰��
    if (GetAvailImportData(root, "HJSE", stp_fpxx->se, 20, true, json_err) < 0) {
        _WriteLog(LL_FATAL, "JSON Parse HJSE failed\n");
        sprintf(errinfo, "��ƱJSON�����кϼ�˰��(hjse)��������,%s", json_err);
        i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        goto Err;
    }
    //��˰�ϼ�
    if (GetAvailImportData(root, "JSHJ", stp_fpxx->jshj, 20, true, json_err) < 0) {
        _WriteLog(LL_FATAL, "JSON Parse JSHJ failed\n");
        sprintf(errinfo, "��ƱJSON�����м�˰�ϼ�(jshj)��������,%s", json_err);
        i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        goto Err;
    }

    if (check_fpxx_info(stp_fpxx, errinfo) < 0) {
        i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        goto Err;
    }

    //�嵥��־
    if (GetAvailImportData(root, "QDBZ", stp_fpxx->qdbj, 3, true, json_err) < 0) {
        _WriteLog(LL_FATAL, "JSON Parse QDBZ failed\n");
        sprintf(errinfo, "��ƱJSON�������嵥��־(qdbz)��������,%s", json_err);
        i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        goto Err;
    }

    if (GetAvailImportData(root, "SYFPDMFPHM", szBuf, 3, false, json_err) < 0) {
        use_fpdmhm = 0;
    }
    if (strcmp(szBuf, "1") == 0)
        use_fpdmhm = 1;
    if (use_fpdmhm == 1) {
        //ָ����Ʊ�����ſ�Ʊ,�����Զ�ѡ
        if (GetAvailImportData(root, "DKFPDM", stp_fpxx->fpdm, 12, true, json_err) < 0) {
            _WriteLog(LL_FATAL, "JSON Parse DKFPDM failed\n");
            sprintf(errinfo, "��ƱJSON������(dkfpdm)��������,%s", json_err);
            i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
            goto Err;
        }
        if (GetAvailImportData(root, "DKFPHM", stp_fpxx->fphm, 12, true, json_err) < 0) {
            _WriteLog(LL_FATAL, "JSON Parse DKFPHM failed\n");
            sprintf(errinfo, "��ƱJSON������(dkfphm)��������,%s", json_err);
            i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
            goto Err;
        }

        if ((strlen(stp_fpxx->fpdm) != 10) && (strlen(stp_fpxx->fpdm) != 12)) {
            sprintf(errinfo, "����Ĵ�����Ʊ���볤������");
            i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
            goto Err;
        }
        if (strlen(stp_fpxx->fphm) != 8) {
            sprintf(errinfo, "����Ĵ�����Ʊ���볤������");
            i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
            goto Err;
        }
    }

    if (GetAvailImportData(root, "tspz", szBuf, 3, false, json_err) < 0) {
        stp_fpxx->zyfpLx = 0;
    } else {
        stp_fpxx->zyfpLx = atoi(szBuf);
    }
    if ((stp_fpxx->zyfpLx != ZYFP_CEZS) && (stp_fpxx->zyfpLx != ZYFP_NULL)) {
        sprintf(errinfo, "ר�÷�Ʊ������ֻ֧��һ��ר��Ʊ�Ͳ������ģʽ");
        i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        goto Err;
    }
    if (stp_fpxx->isRed) {
        //ԭ��Ʊ����
        if (GetAvailImportData(root, "YFPDM", stp_fpxx->blueFpdm, 12, false, json_err) < 0) {
            _WriteLog(LL_FATAL, "JSON Parse YFPDM failed\n");
            sprintf(errinfo, "��ƱJSON�����к��ַ�Ʊԭ��Ʊ����(yfpdm)��������,%s", json_err);
            i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
            goto Err;
        }
        //ԭ��Ʊ����
        if (GetAvailImportData(root, "YFPHM", stp_fpxx->blueFphm, 12, false, json_err) < 0) {
            _WriteLog(LL_FATAL, "JSON Parse YFPHM failed\n");
            sprintf(errinfo, "��ƱJSON�����к��ַ�Ʊԭ��Ʊ����(yfphm)��������,%s", json_err);
            i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
            goto Err;
        }
        //ԭ��Ʊ����ʱ��
        GetAvailImportData(root, "YFPKJSJ", stp_fpxx->blueKpsj, 19, false, json_err);
        if (strlen(stp_fpxx->blueKpsj) > 0) {
            if (strlen(stp_fpxx->blueKpsj) != 14) {
                sprintf(errinfo, "��ƱJSON�����к��ַ�Ʊԭ��Ʊ��Ʊʱ�䳤������");
                i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
                goto Err;
            }
            int sj_count;
            for (sj_count = 0; sj_count < strlen(stp_fpxx->blueKpsj); sj_count++) {
                if ((stp_fpxx->blueKpsj[sj_count] < '0') || (stp_fpxx->blueKpsj[sj_count] > '9')) {
                    sprintf(errinfo, "��ƱJSON�����к��ַ�Ʊԭ��Ʊ��Ʊʱ���ʽ����");
                    return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
                }
            }
        }

        //ԭ��Ʊ����
        GetAvailImportData(root, "YFPZLS", szBuf, 4, false, json_err);
        if ((strlen(szBuf) > 0)) {
            stp_fpxx->blueFplx = atoi(szBuf);
            if ((strcmp(szBuf, "004") != 0) && (strcmp(szBuf, "007") != 0) &&
                (strcmp(szBuf, "026") != 0) && (strcmp(szBuf, "028") != 0)) {
                if (!strcmp(szBuf, "028") && bDeviceType != DEVICE_CNTAX) {
                    _WriteLog(LL_FATAL, "028 Just support cntax");
                    sprintf(errinfo, "����רƱĿǰ��֧��Ukey");
                    i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
                    goto Err;
                }
                _WriteLog(LL_FATAL, "JSON Parse FPZLS did not 004 007 026 028 failed\n");
                sprintf(
                    errinfo,
                    "��Ʊ����Ŀǰ��֧��רƱ����Ʊ�����ա���ר��004 007 026 028���������ͷ�Ʊ����");
                i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
                goto Err;
            }

            if ((stp_fpxx->fplx == FPLX_COMMON_ZYFP) && (stp_fpxx->blueFplx != FPLX_COMMON_ZYFP)) {
                sprintf(errinfo, "רƱ��֧�ֿ�Ʊ�ֺ��");
                i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
                goto Err;
            }
            if ((stp_fpxx->fplx == FPLX_COMMON_DZZP) && (stp_fpxx->blueFplx != FPLX_COMMON_DZZP)) {
                sprintf(errinfo, "��רƱ��֧�ֿ�Ʊ�ֺ��");
                i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
                goto Err;
            }
            if ((stp_fpxx->fplx == FPLX_COMMON_PTFP) &&
                ((stp_fpxx->blueFplx != FPLX_COMMON_PTFP) &&
                 (stp_fpxx->blueFplx != FPLX_COMMON_DZFP) &&
                 (stp_fpxx->blueFplx != FPLX_COMMON_JSFP))) {
                sprintf(errinfo, "��Ʊֻ���Զ���Ʊ�����ա���Ʊ���");
                i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
                goto Err;
            }
            if ((stp_fpxx->fplx == FPLX_COMMON_DZFP) &&
                ((stp_fpxx->blueFplx != FPLX_COMMON_PTFP) &&
                 (stp_fpxx->blueFplx != FPLX_COMMON_DZFP) &&
                 (stp_fpxx->blueFplx != FPLX_COMMON_JSFP))) {
                sprintf(errinfo, "����ֻ���Զ���Ʊ�����ա���Ʊ���");
                i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
                goto Err;
            }

        } else
            stp_fpxx->blueFplx = stp_fpxx->fplx;


        //���ԭ��
        GetAvailImportData(root, "HCYY", szBuf, 3, false, json_err);

        if (strlen(szBuf) > 0) {
            stp_fpxx->hcyy = atoi(szBuf);
            if ((stp_fpxx->hcyy != 1) && (stp_fpxx->hcyy != 2) && (stp_fpxx->hcyy != 3) &&
                (stp_fpxx->hcyy != 4)) {
                sprintf(errinfo, "���ԭ���������ֵ����\n");
                i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
                goto Err;
            }
        } else
            stp_fpxx->hcyy = 2;
        _WriteLog(LL_INFO,
                  "���ߺ��ַ�Ʊ,���ַ�Ʊ����:%s ����:%s ����ʱ��:%s ԭ��Ʊ���ͣ�%03d ���ԭ��%d",
                  stp_fpxx->blueFpdm, stp_fpxx->blueFphm, stp_fpxx->blueKpsj, stp_fpxx->blueFplx,
                  stp_fpxx->hcyy);
    }

    //����˰�������
    GetAvailImportData(root, "SSLKJLY", szBuf, 3, false, json_err);
    if (strlen(szBuf) > 0) {
        stp_fpxx->sslkjly = atoi(szBuf);
    }

    //��ַ������
    GetAvailImportData(root, "DZSYH", szBuf, 24, false, json_err);
    stp_fpxx->dzsyh = atoi(szBuf);

    //�������ֵ
    GetAvailImportData(root, "YXWCZ", szBuf, 3, false, json_err);
    stp_fpxx->allow_error = atoi(szBuf);

    if ((stp_fpxx->allow_error > 6) || (stp_fpxx->allow_error < 1)) {
        stp_fpxx->allow_error = 1;
    }
    //�տ���
    if (GetAvailImportData(root, "SKR", stp_fpxx->skr, 20, false, json_err) < 0) {
        _WriteLog(LL_FATAL, "JSON Parse SKR failed\n");
        sprintf(errinfo, "��ƱJSON�������տ���(skr)��������,%s", json_err);
        i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        goto Err;
    }
    //������
    if (GetAvailImportData(root, "FHR", stp_fpxx->fhr, 20, false, json_err) < 0) {
        _WriteLog(LL_FATAL, "JSON Parse FHR failed\n");
        sprintf(errinfo, "��ƱJSON�����и�����(fhr)��������,%s", json_err);
        i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        goto Err;
    }

    if (strstr(stp_fpxx->fhr, defAisino3SplitFlag)) {
        _WriteLog(LL_FATAL, "JSON Parse FHR failed\n");
        sprintf(errinfo, "��ƱJSON�����и�����(fhr)��Ϣ������ڷǷ��ַ�");
        i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        goto Err;
    }


    //��Ʊ��
    if (GetAvailImportData(root, "KPR", stp_fpxx->kpr, 20, false, json_err) < 0) {
        _WriteLog(LL_FATAL, "JSON Parse KPR failed\n");
        sprintf(errinfo, "��ƱJSON�����п�Ʊ��(kpr)��������,%s", json_err);
        i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        goto Err;
    }

    if (CheckFpxxMandatoryField(stp_fpxx, bDeviceType, errinfo) < 0) {
        i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        goto Err;
    }

    //��Ҫ��Ʒ����
    if (GetAvailImportData(root, "ZYSPMC", stp_fpxx->zyspmc, 100, true, json_err) < 0) {
        _WriteLog(LL_FATAL, "JSON Parse ZYSPMC failed\n");
        sprintf(errinfo, "��ƱJSON��������Ҫ��Ʒ����(zyspmc)��������,%s", json_err);
        i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        goto Err;
    }
    //��Ʒ˰Ŀ
    if (GetAvailImportData(root, "ZYSPSMMC", stp_fpxx->zyspsmmc, 100, true, json_err) < 0) {
        _WriteLog(LL_FATAL, "JSON Parse ZYSPSMMC failed\n");
        sprintf(errinfo, "��ƱJSON��������Ҫ��Ʒ˰Ŀ����(zyspsmmc)��������,%s", json_err);
        i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        goto Err;
    }

    if (bDeviceType == DEVICE_AISINO) {
        if (strlen(stp_fpxx->zyspsmmc) + strlen(stp_fpxx->zyspmc) > 92) {
            sprintf(errinfo, "��˰����Ҫ��Ʒ˰Ŀ+��Ҫ��Ʒ���Ƴ��ȳ���92�ֽڴ���");
            _WriteLog(LL_FATAL, "%s\n", errinfo);
            i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
            goto Err;
        }
    } else if ((bDeviceType == DEVICE_NISEC) || (bDeviceType == DEVICE_CNTAX)) {
        if ((strlen(stp_fpxx->skr) > 16) || (strlen(stp_fpxx->fhr) > 16) ||
            (strlen(stp_fpxx->kpr) > 16)) {
            sprintf(errinfo, "˰�����տ��ˡ������ˡ���Ʊ����󳤶Ȳ��ó���16�ֽ�");
            i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
            goto Err;
        }

        if (strlen(stp_fpxx->zyspsmmc) + strlen(stp_fpxx->zyspmc) > 100) {
            sprintf(errinfo, "˰������Ҫ��Ʒ˰Ŀ+��Ҫ��Ʒ���Ƴ��ȳ���100�ֽڴ���");
            _WriteLog(LL_FATAL, "%s\n", errinfo);
            i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
            goto Err;
        }
    }

    //�����·�,��У����Ҫ���ں�����Ϣ��ӿ�
    if (GetAvailImportData(root, "SSYF", stp_fpxx->ssyf, 10, true, json_err) < 0) {
        _WriteLog(LL_FATAL, "JSON Parse SSYF failed\n");
        sprintf(errinfo, "��ƱJSON�����������·�(ssyf)��������,%s", json_err);
        i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        goto Err;
    }
    if (strlen(stp_fpxx->ssyf) != 6) {
        sprintf(errinfo, "��ƱJSON�����������·ݸ�ʽ����");
        _WriteLog(LL_FATAL, "JSON Parse SSYF len error did not YYYYMM\n");
    }
    //֪ͨ�����,example:1000123410666662,1000123410666673
    //֪ͨ�����,�ŵ��������ȥУ��
    if (GetAvailImportData(root, "TZDBH", stp_fpxx->redNum, 20, false, json_err) < 0) {
        _WriteLog(LL_FATAL, "JSON Parse TZDBH failed\n");
        sprintf(errinfo, "��ƱJSON������֪ͨ�����(tzdbh)��������,%s", json_err);
        i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        goto Err;
    }
#ifndef _NODEVICE_TEST
    //ֱ��ǿ�Ƹ��¿�Ʊ����汾�ż������汾��
    memset(stp_fpxx->kprjbbh, 0, sizeof(stp_fpxx->kprjbbh));
    memset(stp_fpxx->bmbbbh, 0, sizeof(stp_fpxx->bmbbbh));
    strcpy(stp_fpxx->bmbbbh, DEF_BMBBBH_DEFAULT);
    // strcpy(stp_fpxx->kprjbbh, AISINO_INNER_VERSION_DEFAULT);

#endif
    //��Ʒ��ϸ
    jsp_ArraySpxx = cJSON_GetObjectItem(root, "FPMX");
    if (!jsp_ArraySpxx) {
        _WriteLog(LL_FATAL, "JSON Parse FPMX failed\n");
        i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        goto Err;
    }
    int i_SpxxNum = cJSON_GetArraySize(jsp_ArraySpxx);
    if (i_SpxxNum > 2000) {
        sprintf(errinfo, "��Ʒ�������ó���2000��");
        _WriteLog(LL_FATAL, "JSON Parse jsp_ArraySpxx failed\n");
        i_ErrNum = DF_TAX_ERR_CODE_INV_DATA_TOO_LARGE;  //���Ų�����2000��Ʒ
        goto Err;
    }
	
	if ((DEVICE_AISINO != bDeviceType)&&((stp_fpxx->fplx == FPLX_COMMON_DZFP) || (stp_fpxx->fplx == FPLX_COMMON_DZZP)) &&
		(i_SpxxNum > 100)) {
		sprintf(errinfo, "���ӷ�Ʊ��Ʒ�������ó���100��");
		_WriteLog(LL_FATAL, "JSON Parse jsp_ArraySpxx failed\n");
		i_ErrNum = DF_TAX_ERR_CODE_INV_DATA_TOO_LARGE;  //�Ǻ��ŵ�Ʊ������100��Ʒ
		goto Err;
	}
	

    if (strcmp(stp_fpxx->gfsh, "000000123456789") ==
        0)  //��˰�̲������ߴ�����˰��,�����뷢Ʊ�����й�
    {
        sprintf(errinfo, "����˰�Ų�������");
        i_ErrNum = DF_TAX_ERR_CODE_INV_DATA_TOO_LARGE;  //���ŵ�Ʊ������100��Ʒ
        goto Err;
    }

    //���豸������������Ĵ���ִ���жԽṹ���ڵ��޸ģ��͹ܲ���
    //������Ҫ������Ĵ���������ÿ���ڲ�
    if (DEVICE_AISINO == bDeviceType) {
        if (CheckAisinoFpxx(stp_fpxx, errinfo) < 0) {
            i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
            // sprintf(errinfo, "���豸��ⲻͨ��");
            goto Err;
        }
    } else if (DEVICE_NISEC == bDeviceType) {
        if (CheckNisecFpxx(stp_fpxx, errinfo) < 0) {
            i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
            // sprintf(errinfo, "���豸��ⲻͨ��");
            goto Err;
        }
    } else if (DEVICE_CNTAX == bDeviceType) {
        if (CheckCntaxFpxx(stp_fpxx, errinfo) < 0) {
            i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
            // sprintf(errinfo, "���豸��ⲻͨ��");
            goto Err;
        }
    }

    stp_fpxx->spsl = i_SpxxNum;
    int last_fphxz = -1, nAvailSpxxIndex = 0;
    for (i_count = 0; i_count < i_SpxxNum; i_count++) {
        nAvailSpxxIndex++;
        stp_Spxx = malloc(sizeof(struct Spxx));
        if (!stp_Spxx) {
            sprintf(errinfo, "��Ʒ��Ϣ�ṹ��ϵͳ�ڴ�����ʧ��");
            _WriteLog(LL_FATAL, "malloc struct Spxx failed\n");
            i_ErrNum = DF_TAX_ERR_CODE_SYSTEM_MEM_INSUFFICIENT;
            goto Err;
        }
        memset(stp_Spxx, 0, sizeof(struct Spxx));
        cJSON *cjson_tmp = cJSON_GetArrayItem(jsp_ArraySpxx, i_count);
        if (!cjson_tmp) {
            _WriteLog(LL_FATAL, "JSON Parse cjson_tmp failed\n");
            if (stp_Spxx)
                free(stp_Spxx);
            sprintf(errinfo, "��Ʒ��ϢJSON�ڵ��������");
            i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
            goto Err;
        }
        //��Ʒ���
        if (!GetAvailImportData(cjson_tmp, "XH", stp_Spxx->xh, 6, false, json_err)) {
            if (!strcmp(
                    stp_Spxx->xh,
                    "0")) {  //�嵥ͷ������[��������뿪Ʊ��ʹ�ã�xh0Ϊ��������嵥�У��˴�ֱ������]
                free(stp_Spxx);
                nAvailSpxxIndex--;
                continue;
            }
            if (atoi(stp_Spxx->xh) != nAvailSpxxIndex) {
                if (stp_Spxx)
                    free(stp_Spxx);
                sprintf(errinfo, "��Ʒ��Ϣ����Ʒ�����json��˳��һ��,��Ʒ��� %d",
                        nAvailSpxxIndex);
                i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
                goto Err;
            }
        }
        //��Ʒ����
        if (GetAvailImportData(cjson_tmp, "SPMC", stp_Spxx->spmc, 100, true, json_err) < 0) {
            _WriteLog(LL_FATAL, "JSON Parse SPMC failed\n");
            if (stp_Spxx)
                free(stp_Spxx);
            sprintf(errinfo, "��Ʒ��Ϣ����Ʒ����(spmc)��������,%s,��Ʒ��� %d", json_err,
                    nAvailSpxxIndex);
            i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
            goto Err;
        }

        if ((stp_fpxx->isRed == 1) &&
            (strcasecmp(stp_fpxx->qdbj, "Y") == 0 ||
             strcmp(stp_fpxx->qdbj, "1") == 0))  //����Ǻ�Ʊ�嵥����ƷΪ��������嵥��ֱ�Ӳ�����
        {
            if (strcmp(stp_Spxx->spmc, "(��������嵥)") == 0) {
                free(stp_Spxx);
                nAvailSpxxIndex--;
                continue;
            }
        }

        //��Ʒ˰Ŀ����
        if (GetAvailImportData(cjson_tmp, "SPSMMC", stp_Spxx->spsmmc, 100, true, json_err) < 0) {
            _WriteLog(LL_FATAL, "JSON Parse SPSMMC failed\n");
            if (stp_Spxx)
                free(stp_Spxx);
            sprintf(errinfo, "��Ʒ��Ϣ����Ʒ˰Ŀ����(spsmmc)��������,%s,��Ʒ��� %d", json_err,
                    nAvailSpxxIndex);
            i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
            goto Err;
        }
        //��Ʒ���
        if (GetAvailImportData(cjson_tmp, "SPBH", stp_Spxx->spbh, 20, true, json_err) < 0) {
            _WriteLog(LL_FATAL, "JSON Parse SPBH failed\n");
            if (stp_Spxx)
                free(stp_Spxx);
            sprintf(errinfo, "��Ʒ��Ϣ����Ʒ�������(spbh)��������,%s,��Ʒ��� %d", json_err,
                    nAvailSpxxIndex);
            i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
            goto Err;
        }
#ifdef _CONSOLE  //��Ҫɾ���˴��룬���еĿ�Ʊ���ȫ����ʹ�ø��ֶβ�����ֵ���˶�IOʱ��Ҫ���ֶ�
        //���б��
        GetAvailImportData(cjson_tmp, "ZXBH", stp_Spxx->zxbh, 50, false, json_err);
        if (strlen(stp_Spxx->zxbh) > 20) {
            _WriteLog(LL_FATAL, "JSON Parse zxbh failed\n");
            if (stp_Spxx)
                free(stp_Spxx);
            sprintf(errinfo, "��Ʒ��Ϣ�����б��볤�ȴ���");
            i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
            goto Err;
        }
#endif
        //��Ʊ������
        if (GetAvailImportData(cjson_tmp, "FPHXZ", stp_Spxx->fphxz, 3, true, json_err) < 0) {
            _WriteLog(LL_FATAL, "JSON Parse FPHXZ failed\n");
            if (stp_Spxx)
                free(stp_Spxx);
            sprintf(errinfo, "��Ʒ��Ϣ����Ʒ������(fphxz)��������,%s,��Ʒ��� %d", json_err,
                    nAvailSpxxIndex);
            i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
            goto Err;
        }
        nTmpVar = atoi(stp_Spxx->fphxz);
		if ((strcmp(stp_Spxx->fphxz, "0") != 0) &&
			(strcmp(stp_Spxx->fphxz, "1") != 0) &&
			(strcmp(stp_Spxx->fphxz, "2") != 0))	{
            sprintf(errinfo, "��Ʒ��Ϣ����Ʒ������Ŀǰ��֧�� 0 1 2 ����");
            _WriteLog(LL_FATAL, "%s\n", errinfo);
            if (stp_Spxx)
                free(stp_Spxx);
            i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
            goto Err;
        }
        if (nTmpVar == 0) {
            if (last_fphxz == 2) {
                sprintf(errinfo, "��Ʒ��Ϣ�б��У�%d����ƷΪ�����У�����һ��Ϊ���ۿ��д���",
                        nAvailSpxxIndex);
                _WriteLog(LL_FATAL, "%s\n", errinfo);
                if (stp_Spxx)
                    free(stp_Spxx);
                i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
                goto Err;
            }
        }
        if (nTmpVar == 1) {
            if (last_fphxz != 2) {
                sprintf(errinfo, "��Ʒ��Ϣ���ۿ������ڱ��ۿ���֮��,��Ʒ������%d", nAvailSpxxIndex);
                _WriteLog(LL_FATAL, "%s\n", errinfo);
                if (stp_Spxx)
                    free(stp_Spxx);
                i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
                goto Err;
            }
        }
        last_fphxz = nTmpVar;

        // aisino need to convert
        if (DEVICE_AISINO == bDeviceType) {
            switch (nTmpVar) {
                case 0:
                    nTmpVar = FPHXZ_AISINO_SPXX;
                    break;
                case 1:
                    nTmpVar = FPHXZ_AISINO_ZKXX;
                    break;
                case 2:
                    nTmpVar = FPHXZ_AISINO_SPXX_ZK;
                    break;
            }
        }
        memset(stp_Spxx->fphxz, 0, sizeof(stp_Spxx->fphxz));
        sprintf(stp_Spxx->fphxz, "%d", nTmpVar);

        int fphxz = IsZKXX(bDeviceType, stp_Spxx->fphxz);
        if (fphxz == 1)
            exsit_zkxx = 1;  //�����ۿ���Ϣ
        //����
        if (GetAvailImportData(cjson_tmp, "DJ", stp_Spxx->dj, 21, false, json_err) < 0) {
            _WriteLog(LL_FATAL, "JSON Parse DJ failed\n");
            if (stp_Spxx)
                free(stp_Spxx);
            sprintf(errinfo, "��Ʒ��Ϣ�е���(dj)��������,%s,��Ʒ��� %d", json_err,
                    nAvailSpxxIndex);
            i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
            goto Err;
        }
        //������λ
        if (GetAvailImportData(cjson_tmp, "JLDW", stp_Spxx->jldw, 22, false, json_err) < 0) {
            _WriteLog(LL_FATAL, "JSON Parse JLDW failed\n");
            if (stp_Spxx)
                free(stp_Spxx);
            sprintf(errinfo, "��Ʒ��Ϣ�м�����λ(jldw)��������,%s,��Ʒ��� %d", json_err,
                    nAvailSpxxIndex);
            i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
            goto Err;
        }
        //����ͺ�
        if (GetAvailImportData(cjson_tmp, "GGXH", stp_Spxx->ggxh, 40, false, json_err) < 0) {
            _WriteLog(LL_FATAL, "JSON Parse GGXH failed\n");
            if (stp_Spxx)
                free(stp_Spxx);
            sprintf(errinfo, "��Ʒ��Ϣ�й���ͺ�(ggxh)��������,%s,��Ʒ��� %d", json_err,
                    nAvailSpxxIndex);
            i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
            goto Err;
        }
        if ((bDeviceType == DEVICE_NISEC) || (bDeviceType == DEVICE_CNTAX)) {
            if (strlen(stp_Spxx->ggxh) > 36) {
                sprintf(errinfo, "˰������Ʒ��Ϣ�й���ͺŲ��ó���36�ֽ�,��Ʒ��� %d",
                        nAvailSpxxIndex);
                i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
                goto Err;
            }
        }
        //����
        if (GetAvailImportData(cjson_tmp, "SL", stp_Spxx->sl, 21, false, json_err) < 0) {
            _WriteLog(LL_FATAL, "JSON Parse SL failed\n");
            if (stp_Spxx)
                free(stp_Spxx);
            sprintf(errinfo, "��Ʒ��Ϣ������(sl)��������,%s,��Ʒ��� %d", json_err,
                    nAvailSpxxIndex);
            i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
            goto Err;
        }
        //˰��
        if (GetAvailImportData(cjson_tmp, "SE", stp_Spxx->se, 18, true, json_err) < 0) {
            _WriteLog(LL_FATAL, "JSON Parse SE failed\n");
            if (stp_Spxx)
                free(stp_Spxx);
            sprintf(errinfo, "��Ʒ��Ϣ��˰��(se)��������,%s,��Ʒ��� %d", json_err,
                    nAvailSpxxIndex);
            i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
            goto Err;
        }
        d_zse += atof(stp_Spxx->se);
        if ((fphxz == 0) || (fphxz == 2))
            d_yjhjse += atof(stp_Spxx->se);  //ȡԭ�ۺϼ�˰��
        else
            d_zkehjse += atof(stp_Spxx->se);  //ȡ�ۿ۶�ϼ�˰��

        //���
        if (GetAvailImportData(cjson_tmp, "JE", stp_Spxx->je, 18, true, json_err) < 0) {
            _WriteLog(LL_FATAL, "JSON Parse JE failed\n");
            if (stp_Spxx)
                free(stp_Spxx);
            sprintf(errinfo, "��Ʒ��Ϣ�н��(je)��������,%s,��Ʒ��� %d", json_err,
                    nAvailSpxxIndex);
            i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
            goto Err;
        }
        d_zje += atof(stp_Spxx->je);
        if ((fphxz == 0) || (fphxz == 2))
            d_yjhjje += atof(stp_Spxx->je);  //ȡԭ�ۺϼƽ��
        else
            d_zkehjje += atof(stp_Spxx->je);  //ȡ�ۿ۶�ϼƽ��

        GetAvailImportData(cjson_tmp, "HSJE", stp_Spxx->hsje, 18, false, json_err);

        //˰��
        if (GetAvailImportData(cjson_tmp, "SLV", stp_Spxx->slv, 6, true, json_err) < 0) {
            _WriteLog(LL_FATAL, "JSON Parse SLV failed\n");
            if (stp_Spxx)
                free(stp_Spxx);
            sprintf(errinfo, "��Ʒ��Ϣ��˰��(slv)��������,%s,��Ʒ��� %d", json_err,
                    nAvailSpxxIndex);
            i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
            goto Err;
        }

        if (strcmp(stp_Spxx->slv, "0.015") == 0) {
            stp_fpxx->zyfpLx = ZYFP_JYZS;
        }
        if (!strcmp(stp_Spxx->slv, "0.03")) {
            //�����1%��3%�����Լ�˰����˰��Ĭ������ڶ���ѡ����Ŀ
            stp_fpxx->specialFlag = TSFP_XGM_SPECIAL_TAX;
            if (stp_fpxx->sslkjly == 0)  //δ������Ĭ��Ϊ3
                //stp_fpxx->sslkjly = 3;
                stp_fpxx->sslkjly = 2;
        }
        else if(!strcmp(stp_Spxx->slv, "0.01"))
        {
            stp_fpxx->sslkjly = 0;
        }

        if (d_slv_first == -1) {
            d_slv = atof(stp_Spxx->slv);
            d_slv_first = 0;
        } else {
            if (d_slv != atof(stp_Spxx->slv)) {
                SamSlvFlag = 0;
            }
        }
        //�Ż����߱�ʶ
        if (GetAvailImportData(cjson_tmp, "YHZCBS", stp_Spxx->xsyh, 3, true, json_err) < 0) {
            _WriteLog(LL_FATAL, "JSON Parse YHZCBS failed\n");
            if (stp_Spxx)
                free(stp_Spxx);
            sprintf(errinfo, "��Ʒ��Ϣ���Ż����߱�ʶ(yhzcbs)��������,%s,��Ʒ��� %d", json_err,
                    nAvailSpxxIndex);
            i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
            goto Err;
        }
        //˰�ʱ�ʶ
        if (GetAvailImportData(cjson_tmp, "SLBS", stp_Spxx->lslvbs, 3, false, json_err) < 0) {
            _WriteLog(LL_FATAL, "JSON Parse SLBS failed\n");
            if (stp_Spxx)
                free(stp_Spxx);
            sprintf(errinfo, "��Ʒ��Ϣ��˰�ʱ�ʶ(slbs)��������,%s,��Ʒ��� %d", json_err,
                    nAvailSpxxIndex);
            i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
            goto Err;
        }

        if (d_lslv_first == -1) {
            d_lslv = atoi(stp_Spxx->lslvbs);
            d_lslv_first = 0;
        } else {
            if (d_lslv != atoi(stp_Spxx->lslvbs)) {
                SamLSlvFlag = 0;
            }
        }

        if ((strcmp(stp_fpxx->tax_nature, "С��ģ��˰��") == 0) && (atoi(stp_Spxx->lslvbs) == 3)) {
            if (stp_Spxx)
                free(stp_Spxx);
            sprintf(errinfo, "С��ģ��˰�˲���������ͨ��˰�ʷ�Ʊ");
            i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
            goto Err;
        }
        //��ֵ˰�������
        if (GetAvailImportData(cjson_tmp, "ZZSTSGL", stp_Spxx->yhsm, 50, false, json_err) < 0) {
            _WriteLog(LL_FATAL, "JSON Parse ZZSTSGL failed\n");
            if (stp_Spxx)
                free(stp_Spxx);
            sprintf(errinfo, "��Ʒ��Ϣ����ֵ˰�������(zzstsgl)��������,%s,��Ʒ��� %d", json_err,
                    nAvailSpxxIndex);
            i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
            goto Err;
        }
        //�۳���� �����˰�۳���
        if (GetAvailImportData(cjson_tmp, "CEZSKCE", stp_Spxx->kce, 20, false, json_err) < 0) {
            _WriteLog(LL_FATAL, "JSON Parse CEZSKCE failed\n");
            if (stp_Spxx)
                free(stp_Spxx);
            sprintf(errinfo, "��Ʒ��Ϣ�в����˰�۳���(cezskce)��������,%s,��Ʒ��� %d", json_err,
                    nAvailSpxxIndex);
            i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
            goto Err;
        }
        if (strlen(stp_Spxx->kce) > 0)
            stp_fpxx->zyfpLx = ZYFP_CEZS;

        //��˰�۱�־
        if (GetAvailImportData(cjson_tmp, "HSJBZ", stp_Spxx->hsjbz, 3, false, json_err) < 0) {
            memcpy(stp_Spxx->hsjbz, stp_fpxx->hsjbz, strlen(stp_fpxx->hsjbz));
        }
        // --nisec�̸��º�˰��Ϊ����˰��
        SpxxPriceRemoveTaxContition(bDeviceType, stp_fpxx, stp_Spxx);
        if (check_spxx_info(bDeviceType, stp_fpxx, stp_Spxx, errinfo) < 0) {
            _WriteLog(LL_FATAL, "��Ʒ��Ϣ���������ڴ���\n");
            if (stp_Spxx)
                free(stp_Spxx);
            i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
            goto Err;
        }
        // //--�����aisino��
        // if (DEVICE_AISINO == bDeviceType) {
        //     if (strlen(stp_Spxx->hsjbz) == 0 || !strcmp(stp_Spxx->hsjbz, "0"))
        //         strcpy(stp_Spxx->hsjbz, "0");
        //     else
        //         strcpy(stp_Spxx->hsjbz, "1");
        //     // 20220908���²��Խ��
        //     // ���ӷ�Ʊ��˰����ʱ����˰�۱�־Ϊ1,ר��ƱΪ0,�����ڿ�Ʊ��������˰������񣬴˴���־��Զ�̶�
        //     if ((!strcmp(stp_Spxx->se, "0.00") &&
        //          IsAisino3Device(stp_fpxx->hDev->nDriverVersionNum))) {
        //         //�ɱ�׼3%������˰��Ʊ���ߣ���˰�ʷ�Ʊ�����ԣ�
        //         if (stp_fpxx->fplx_aisino == FPLX_AISINO_DZFP)
        //             strcpy(stp_Spxx->hsjbz, "1");
        //         else
        //             strcpy(stp_Spxx->hsjbz, "0");
        //     }
        // }
        //����ṹ������
        if (InsertMxxx(stp_fpxx, stp_Spxx) != 0) {
            _WriteLog(LL_FATAL, "JSON Parse ����ṹ������failed\n");
            if (stp_Spxx)
                free(stp_Spxx);
            sprintf(errinfo, "ϵͳ������Ʒ�����޷���������");
            i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
            goto Err;
        }
        //���豸��Ʒ��Ϣ����
        if (DEVICE_AISINO == bDeviceType) {
            if (CheckAisinoSpxx(stp_fpxx, stp_Spxx, errinfo) < 0) {
                i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
                sprintf(errinfo, "���豸��ⲻͨ��");
                goto Err;
            }
        } else if (DEVICE_NISEC == bDeviceType) {
            if (CheckNisecSpxx(stp_fpxx, stp_Spxx, errinfo) < 0) {
                i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
                sprintf(errinfo, "���豸��ⲻͨ��");
                goto Err;
            }
        } else if (DEVICE_CNTAX == bDeviceType) {
            // if (strlen(stp_Spxx->kce) > 0) {
            //     i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
            //     sprintf(errinfo, "�ݲ�֧��˰��ukey���߲�����շ�Ʊ");
            //     goto Err;
            // }
            // if (strlen(stp_Spxx->yhsm) > 0) {
            //     i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
            //     sprintf(errinfo, "�ݲ�֧��˰��ukey���߼�����˰��Ʊ");
            //     goto Err;
            // }
        }
    }

    char tmp_zsewc[50] = {0};
    char s_d_zsewc[50] = {0};
    memset(tmp_zsewc, 0, sizeof(tmp_zsewc));
    memset(s_d_zsewc, 0, sizeof(s_d_zsewc));
    sprintf(tmp_zsewc, "%Lf", stp_fpxx->d_zsewc);
    PriceRound(tmp_zsewc, 2, s_d_zsewc);

    //_WriteLog(LL_FATAL, "s_d_zsewc = %s\n", s_d_zsewc);

    long double d_zsewc;
    d_zsewc = atof(s_d_zsewc);

    //_WriteLog(LL_FATAL, "d_zsewc = %f\n", d_zsewc);
    if (fabs(d_zsewc) > 1.27) {
        sprintf(errinfo, "������Ʒ��ͳ�Ƶ�˰������1.27,������Ʊ");
        i_ErrNum = DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
        goto Err;
    }

    //��˰�ϼ�
    d_jshj = d_zje + d_zse;
    memset(szBuf, 0, sizeof(szBuf));
    sprintf(szBuf, "%4.2f", d_jshj);
    if (strcmp(szBuf, stp_fpxx->jshj)) {
        _WriteLog(LL_FATAL,
                  "JSON judge d_jshj,�����˰�ϼ� = %s,�����˰�ϼ� = "
                  "%s,�����ܽ��Ϊ��%4.2f,�����ܽ��Ϊ��%s,������˰��Ϊ��%4.2f,������˰��Ϊ:%s\n",
                  szBuf, stp_fpxx->jshj, d_zje, stp_fpxx->je, d_zse, stp_fpxx->se);
        sprintf(errinfo, "������Ʒ��ͳ�Ƶļ�˰�ϼƲ�һ��");
        i_ErrNum = DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
        goto Err;
    }
    //�ܽ��
    memset(szBuf, 0, sizeof(szBuf));
    sprintf(szBuf, "%4.2f", d_zje);
    if (strcmp(szBuf, stp_fpxx->je)) {
        _WriteLog(LL_FATAL, "JSON judge d_zje failed\n");
        sprintf(errinfo, "������Ʒ��ͳ�Ƶ��ܽ�һ��");
        i_ErrNum = DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
        goto Err;
    }
    memcpy(stp_fpxx->stp_MxxxHead->je, szBuf, strlen(szBuf));
    //��˰��
    memset(szBuf, 0, sizeof(szBuf));
    sprintf(szBuf, "%4.2f", d_zse);
    if (strcmp(szBuf, stp_fpxx->se)) {
        _WriteLog(LL_FATAL, "JSON judge d_zse failed\n");
        sprintf(errinfo, "������Ʒ��ͳ�Ƶ���˰�һ��,�ϼ�˰Ϊ%s,�ۼ�˰��Ϊ%s", stp_fpxx->se,
                szBuf);
        i_ErrNum = DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
        goto Err;
    }
    memcpy(stp_fpxx->stp_MxxxHead->se, szBuf, strlen(szBuf));
    //�ж��Ƿ�ȫ��Ϊͬһ˰��,����ǲ�д���ۺ�˰��,����Ҳ����
    stp_fpxx->isMultiTax = 0;
    if (SamSlvFlag) {
        memset(szBuf, 0, sizeof(szBuf));
        sprintf(szBuf, "%4.3f", d_slv);
        if (strncmp(stp_fpxx->slv, szBuf, 4)) {  // 0.01
            _WriteLog(LL_FATAL, "JSON judge d_zhsl failed\n");
            sprintf(errinfo, "������Ʒ��ͳ�Ƶ��ۺ�˰��ƥ�䲻һ��,�ۺ�˰��Ϊ%s,�ۼ�˰��Ϊ%s",
                    stp_fpxx->slv, szBuf);
            i_ErrNum = DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
            goto Err;
        }
        if ('0' == szBuf[strlen(szBuf) - 1])
            szBuf[strlen(szBuf) - 1] = '\0';
        strcpy(stp_fpxx->slv, szBuf);
        strcpy(stp_fpxx->stp_MxxxHead->slv, szBuf);
    } else {
        stp_fpxx->isMultiTax = 1;
        if (stp_fpxx->zyfpLx == ZYFP_JYZS) {
            sprintf(errinfo, "ʹ��0.015˰�ʵ���Ʒ��֧�ֶ�˰���");
            i_ErrNum = DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
            goto Err;
        }
    }
    //����8����ϸ ���嵥 ��������嵥
    bool bZZSRedDetailList = false;  //��ֵ˰��Ʊ�嵥�����ܲ���ڵ���Ϣ
    if ((i_SpxxNum > 8 || strcasecmp(stp_fpxx->qdbj, "Y") == 0 ||
         strcmp(stp_fpxx->qdbj, "1") == 0) &&
        (stp_fpxx->fplx_aisino != FPLX_AISINO_DZFP && stp_fpxx->fplx_aisino != FPLX_AISINO_DZZP)) {
        _WriteLog(LL_INFO, "�嵥��Ʊ,�嵥��־Ϊ%s\n", stp_fpxx->qdbj);

        if (stp_fpxx->zyfpLx == ZYFP_CEZS) {
            sprintf(errinfo, "������ղ�֧���嵥��Ʊ");
            _WriteLog(LL_FATAL, "%s\n", errinfo);
            i_ErrNum = DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
            goto Err;
        }
        if (!stp_fpxx->isRed) {  //��Ʊʹ���嵥��Ʊ
            strcpy(stp_fpxx->qdbj, "Y");
            if (DEVICE_AISINO == bDeviceType) {
                strcpy(stp_fpxx->stp_MxxxHead->spmc, "(��������嵥)");
                sprintf(stp_fpxx->stp_MxxxHead->fphxz, "%d", FPHXZ_AISINO_XJXHQD);
                sprintf(stp_fpxx->stp_MxxxHead->xsyh, "0");
                if (SamSlvFlag)
                    strcpy(stp_fpxx->stp_MxxxHead->slv, stp_fpxx->stp_MxxxHead->stp_next->slv);
                else
                    memset(stp_fpxx->stp_MxxxHead->slv, 0, sizeof(stp_fpxx->stp_MxxxHead->slv));
                if (SamLSlvFlag)
                    strcpy(stp_fpxx->stp_MxxxHead->lslvbs,
                           stp_fpxx->stp_MxxxHead->stp_next->lslvbs);
                else
                    memset(stp_fpxx->stp_MxxxHead->lslvbs, 0,
                           sizeof(stp_fpxx->stp_MxxxHead->lslvbs));
            } else {  // nisec/cntax
                strcpy(stp_fpxx->stp_MxxxHead->spmc, "����������嵥��");
                sprintf(stp_fpxx->stp_MxxxHead->fphxz, "%d", FPHXZ_COMMON_XJXHQD);
                sprintf(stp_fpxx->stp_MxxxHead->hsjbz, "0");

                if (SamSlvFlag) {
                    strcpy(stp_fpxx->stp_MxxxHead->slv, stp_fpxx->stp_MxxxHead->stp_next->slv);
                    strcpy(stp_fpxx->stp_MxxxHead->lslvbs,
                           stp_fpxx->stp_MxxxHead->stp_next->lslvbs);
                    strcpy(stp_fpxx->stp_MxxxHead->xsyh, stp_fpxx->stp_MxxxHead->stp_next->xsyh);
                    strcpy(stp_fpxx->stp_MxxxHead->yhsm, stp_fpxx->stp_MxxxHead->stp_next->yhsm);
                } else {
                    memset(stp_fpxx->stp_MxxxHead->slv, 0, sizeof(stp_fpxx->stp_MxxxHead->slv));
                    memset(stp_fpxx->stp_MxxxHead->lslvbs, 0,
                           sizeof(stp_fpxx->stp_MxxxHead->lslvbs));
                    memset(stp_fpxx->stp_MxxxHead->xsyh, 0, sizeof(stp_fpxx->stp_MxxxHead->xsyh));
                    memset(stp_fpxx->stp_MxxxHead->yhsm, 0, sizeof(stp_fpxx->stp_MxxxHead->yhsm));
                }
            }
        } else {  //��Ʊʹ���嵥��Ʊ��ר��Ʊ��Ҫ���⴦��
            bZZSRedDetailList = true;
            stp_fpxx->isRedDetailList = 1;
            strcpy(stp_fpxx->qdbj, "Y");
            strcpy(stp_fpxx->stp_MxxxHead->spmc, "�����Ӧ������Ʊ���嵥");
            if (DEVICE_AISINO == bDeviceType) {
                sprintf(stp_fpxx->stp_MxxxHead->fphxz, "%d", FPHXZ_AISINO_XJDYZSFPQD);

            } else {
                sprintf(stp_fpxx->stp_MxxxHead->fphxz, "%d", FPHXZ_COMMON_SPXX);
                if (SamSlvFlag) {
                    strcpy(stp_fpxx->stp_MxxxHead->slv, stp_fpxx->stp_MxxxHead->stp_next->slv);
                    strcpy(stp_fpxx->stp_MxxxHead->lslvbs,
                           stp_fpxx->stp_MxxxHead->stp_next->lslvbs);
                } else {
                    memset(stp_fpxx->stp_MxxxHead->slv, 0, sizeof(stp_fpxx->stp_MxxxHead->slv));
                    memset(stp_fpxx->stp_MxxxHead->lslvbs, 0,
                           sizeof(stp_fpxx->stp_MxxxHead->lslvbs));
                }
                //��Ʊ�嵥ֻ��רƱ����Ʊ��
            }
        }

        if (exsit_zkxx == 1) {
            if ((bDeviceType == DEVICE_NISEC) || (bDeviceType == DEVICE_CNTAX)) {
                // nisec�Ѳ�������
                //
                // sprintf(errinfo, "�ݲ�֧��˰�����ۿ��嵥��Ʊ����");
                // _WriteLog(LL_FATAL, "%s\n", errinfo);
                // i_ErrNum = DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
                // goto Err;
            } else {
                // aisino
                _WriteLog(LL_WARN, "�嵥��Ʒ�д����ۿ���Ϣ\n");
                _WriteLog(LL_WARN, "���ԭ�ۺϼ�\n");
                stp_Spxx = malloc(sizeof(struct Spxx));
                if (!stp_Spxx) {
                    sprintf(errinfo, "��Ʒ��Ϣ�ṹ����ԭ�ۺϼ�ϵͳ�ڴ�����ʧ��");
                    _WriteLog(LL_FATAL, "%s\n", errinfo);
                    i_ErrNum = DF_TAX_ERR_CODE_SYSTEM_MEM_INSUFFICIENT;
                    goto Err;
                }
                memset(stp_Spxx, 0, sizeof(struct Spxx));
                strcpy(stp_Spxx->spsmmc, "ԭ�ۺϼ�");
                sprintf(stp_Spxx->fphxz, "%d", FPHXZ_AISINO_XHQDZK);
                sprintf(stp_Spxx->je, "%4.2f", d_yjhjje);
                sprintf(stp_Spxx->se, "%4.2f", d_yjhjse);
                strcpy(stp_Spxx->hsjbz, stp_fpxx->hsjbz);
                sprintf(stp_Spxx->xsyh, "0");
                strcpy(stp_Spxx->slv, stp_fpxx->stp_MxxxHead->slv);
                if (InsertMxxx(stp_fpxx, stp_Spxx) != 0) {
                    _WriteLog(LL_FATAL, "JSON Parse ����ṹ������failed\n");
                    if (stp_Spxx)
                        free(stp_Spxx);
                    sprintf(errinfo, "ϵͳ����ԭ�ۺϼ���Ʒ�����޷���������");
                    i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
                    goto Err;
                }

                _WriteLog(LL_WARN, "����ۿ۶�ϼ�\n");
                stp_Spxx = malloc(sizeof(struct Spxx));
                if (!stp_Spxx) {
                    sprintf(errinfo, "��Ʒ��Ϣ�ṹ����ԭ�ۺϼ�ϵͳ�ڴ�����ʧ��");
                    _WriteLog(LL_FATAL, "%s\n", errinfo);
                    i_ErrNum = DF_TAX_ERR_CODE_SYSTEM_MEM_INSUFFICIENT;
                    goto Err;
                }
                memset(stp_Spxx, 0, sizeof(struct Spxx));
                strcpy(stp_Spxx->spsmmc, "�ۿ۶�ϼ�");
                sprintf(stp_Spxx->fphxz, "%d", FPHXZ_AISINO_XHQDZK);
                sprintf(stp_Spxx->je, "%4.2f", d_zkehjje);
                sprintf(stp_Spxx->se, "%4.2f", d_zkehjse);
                strcpy(stp_Spxx->hsjbz, stp_fpxx->hsjbz);
                sprintf(stp_Spxx->xsyh, "0");
                strcpy(stp_Spxx->slv, stp_fpxx->stp_MxxxHead->slv);
                if (InsertMxxx(stp_fpxx, stp_Spxx) != 0) {
                    _WriteLog(LL_FATAL, "JSON Parse ����ṹ������failed\n");
                    if (stp_Spxx)
                        free(stp_Spxx);
                    sprintf(errinfo, "ϵͳ����ԭ�ۺϼ���Ʒ�����޷���������");
                    i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
                    goto Err;
                }
            }
        }

        if (bZZSRedDetailList) {
            //�Ƴ�����spxx��head֮��Ľڵ�
            struct Spxx *stp_MxxxNode = stp_fpxx->stp_MxxxHead->stp_next, *stp_TmpSpxxNode = NULL;
            //�ͷ���Ʒ��ϸ
            while (stp_MxxxNode) {
                stp_TmpSpxxNode = stp_MxxxNode->stp_next;
                free(stp_MxxxNode);
                stp_MxxxNode = stp_TmpSpxxNode;
            }
            //�޸�����˵����Ϣ������Ʒ��Ϣ
            stp_fpxx->stp_MxxxHead->stp_next = malloc(sizeof(struct Spxx));
            memcpy(stp_fpxx->stp_MxxxHead->stp_next, stp_fpxx->stp_MxxxHead, sizeof(struct Spxx));
            stp_TmpSpxxNode = stp_fpxx->stp_MxxxHead->stp_next;
            stp_TmpSpxxNode->stp_next = NULL;
            strcpy(stp_TmpSpxxNode->hsjbz, "0");
            strcpy(stp_TmpSpxxNode->xsyh, "0");
            stp_fpxx->spsl = 1;
            //������嵥��ˢ������Ҫ��Ʒ����
            strcpy(stp_fpxx->zyspmc, stp_fpxx->stp_MxxxHead->spmc);
            strcpy(stp_fpxx->zyspsmmc, "");
            strcpy(stp_fpxx->qdbj, "N");
            if (DEVICE_AISINO == bDeviceType) {
                //�����嵥
                sprintf(stp_fpxx->stp_MxxxHead->stp_next->fphxz, "%d", FPHXZ_AISINO_XJDYZSFPQD);
            }
        }
    } else {
        memcpy(stp_fpxx->qdbj, "N", strlen("N"));
    }
    //��ע-�ܶ�С���෢Ʊ�����ڱ�ע�н��б�ʶ����˱�ע�����ܶ��������Ҫ�������
    char user_bz[240] = {0};
    char tmp_bz[512] = {0};
    memset(user_bz, 0, sizeof(user_bz));
    memset(tmp_bz, 0, sizeof(tmp_bz));

    if (GetAvailImportData(root, "BZ", user_bz, 230, false, json_err) < 0) {
        _WriteLog(LL_FATAL, "JSON Parse BZ\n");
        sprintf(errinfo, "��ƱJSON��������������(bz)��������,%s", json_err);
        i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        goto Err;
    }
    //���軻�У�����Ӧ�ò���м��ݣ������Ӧ�ò�Ҫ���н����滻�ɵײ�
    if (strlen(user_bz) != 0)
        str_replace(user_bz, "<br/>", "\r\n");
    // tmp_bz��user_bz��ͬʱ����ԭʼ��ע,��������user_bz���������հ����ı�ע
    // user_bz�����������ձ�עʱ�Ķ����м䲻�ٸĶ���tmp_bz��Ϊ���������ᱻ�ı�
    if ((DEVICE_AISINO == bDeviceType) || (DEVICE_MENGBAI == bDeviceType)) {
        if (strlen(user_bz) > 230) {
            sprintf(errinfo, "��˰�̷�Ʊ��ע���ô���230�ֽ�");
            i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
            goto Err;
        }
    } else if (DEVICE_NISEC == bDeviceType) {
        if (stp_fpxx->fplx_aisino == FPLX_AISINO_ZYFP) {
            if (strlen(user_bz) > 184) {
                sprintf(errinfo, "˰����ר�÷�Ʊ��ע���ô���184�ֽ�");
                i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
                goto Err;
            }
        } else if (stp_fpxx->fplx_aisino == FPLX_AISINO_PTFP) {
            if (strlen(user_bz) > 138) {
                sprintf(errinfo, "˰������ͨ��Ʊ��ע���ô���138�ֽ�");
                i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
                goto Err;
            }
        } else if (stp_fpxx->fplx_aisino == FPLX_AISINO_DZFP) {
            if (strlen(user_bz) > 130) {
                sprintf(errinfo, "˰���̵��ӷ�Ʊ��ע���ô���130�ֽ�");
                i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
                goto Err;
            }
        }
    } else if (DEVICE_CNTAX == bDeviceType) {
        if (stp_fpxx->fplx_aisino == FPLX_AISINO_ZYFP) {
            if (strlen(user_bz) > 184) {
                sprintf(errinfo, "˰��ukeyר�÷�Ʊ��ע���ô���184�ֽ�");
                i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
                goto Err;
            }
        } else if (stp_fpxx->fplx_aisino == FPLX_AISINO_PTFP) {
            if (strlen(user_bz) > 138) {
                sprintf(errinfo, "˰��ukey��ͨ��Ʊ��ע���ô���138�ֽ�");
                i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
                goto Err;
            }
        } else if (stp_fpxx->fplx_aisino == FPLX_AISINO_DZFP) {
            if (strlen(user_bz) > 130) {
                sprintf(errinfo, "˰��ukey�̵�����Ʊ��ע���ô���130�ֽ�");
                i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
                goto Err;
            }
        } else if (stp_fpxx->fplx_aisino == FPLX_AISINO_DZZP) {
            if (strlen(user_bz) > 230) {
                sprintf(errinfo, "˰��ukey����רƱ��ע���ô���230�ֽ�");
                i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
                goto Err;
            }
        }
    }
    strcpy(tmp_bz, user_bz);
    if (strstr(user_bz, "��Ӧ������Ʊ����:") || strstr(user_bz, "�����˰��") ||
        strstr(user_bz, "��Ʊ��Ϣ����")) {
        _WriteLog(LL_FATAL, "���ܳ��ֱ����ؼ��ֺ͹ؼ���䣬'��Ӧ������Ʊ���룺'.'�����˰��',"
                            "'��Ʊ��Ϣ����'\n");
        sprintf(errinfo, "���ܳ��ֱ����ؼ��ֺ͹ؼ����,'��Ӧ������Ʊ���룺'��'"
                         "�����˰��','��Ʊ��Ϣ����'");
        i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        goto Err;
    }
    if (stp_fpxx->isRed) {  //��Ʊ��ע�滻
        if (stp_fpxx->isHzxxb == false && stp_fpxx->hzxxbsqsm != 1 &&
            (!strlen(stp_fpxx->blueFpdm) ||
             !strlen(stp_fpxx->blueFphm))) {  //���ܼ���Ʊ��Ʊʱ�䣬����Ӱ�����ڿ�Ʊҵ��
            //�������ѵֿ��⣬ȫ����Ҫ���ַ�Ʊ�������
            sprintf(errinfo, "���ַ�Ʊ��������������ԭ��Ʊ�����ԭ��Ʊ��");
            _WriteLog(LL_FATAL, "%s\n", errinfo);
            i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
            goto Err;
        }
        char szRedStr[128] = {0};
        if ((stp_fpxx->fplx_aisino == FPLX_AISINO_ZYFP) ||
            (stp_fpxx->fplx_aisino == FPLX_AISINO_DZZP)) {
            //���ַ�Ʊ֪ͨ����Ź�16λ,У��λΪ���һλ����ǰ15λ����֮�͵ĸ�λ��
            if (strlen(stp_fpxx->redNum) == 0 || strlen(stp_fpxx->redNum) != 16) {
                sprintf(errinfo, "ר�÷�Ʊ���ַ�Ʊ,������Ϣ���ȴ���");
                i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
                goto Err;
            }
            if (GetZYFPRedNotes(bDeviceType, stp_fpxx->redNum, "s", szRedStr) < 0) {
                sprintf(errinfo, "���ַ�Ʊ������Ϣ��֪ͨ����ţ�У��ʧ��");
                i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
                goto Err;
            }
        } else
            GetRedInvNotes(stp_fpxx->blueFpdm, stp_fpxx->blueFphm, szRedStr);
        //���ԭʼ��עΪ�գ��򲻽ӱ�ע�����ԭʼ��ע��Ϊ�գ�����Ҫ�ӿո�+ԭʼ��ע


        if (strlen(user_bz)) {
            // if (DEVICE_AISINO == bDeviceType)
            //    sprintf(tmp_bz, " %s", user_bz);
            // else if (DEVICE_NISEC == bDeviceType)
            //    sprintf(tmp_bz, "\n%s", user_bz);
            strcpy(tmp_bz, user_bz);
        }
        // user_bzΪ�գ������и�strcpy��tmp_bzҲΪ��

        if (stp_fpxx->zyfpLx == ZYFP_CEZS) {
            //˰���̱�ע����Ӧ������Ʊ������� + ���� + �����˰ + �ո� + �û���ע��Ϣ
            //��˰�̱�ע�������˰��+ ��Ӧ������Ʊ������� + �ո� + �û���ע��Ϣ             δȷ��
            if ((bDeviceType == DEVICE_NISEC) || ((bDeviceType == DEVICE_CNTAX))) {
                if (strlen(szRedStr) != 0)  //Ĭ�ϼ��뱸ע��̶����������˰���������м�ֱ�Ӽ���\n
                {
                    strcat(szRedStr, "\n");
                }
                if (strlen(tmp_bz) != 0)
                    snprintf(user_bz, sizeof(user_bz), "%s�����˰ %s", szRedStr, tmp_bz);
                else
                    snprintf(user_bz, sizeof(user_bz), "%s�����˰", szRedStr);
            } else {
                if (strlen(tmp_bz) != 0)
                    snprintf(user_bz, sizeof(user_bz), "�����˰��%s %s", szRedStr, tmp_bz);
                else
                    snprintf(user_bz, sizeof(user_bz), "�����˰��%s", szRedStr);
            }
        } else {
            //˰���̱�ע����Ӧ������Ʊ������� + ���� + �û���ע��Ϣ
            //��˰�̱�ע����Ӧ������Ʊ������� + �ո� + �û���ע��Ϣ             δȷ��
            if ((bDeviceType == DEVICE_NISEC) || (bDeviceType == DEVICE_CNTAX)) {
                if ((strlen(szRedStr) != 0) &&
                    (strlen(tmp_bz) != 0))  //Ĭ�ϼ��뱸ע���û���ע����Ϊ�յ�ʱ���м����\n
                {
                    strcat(szRedStr, "\n");
                }
                snprintf(user_bz, sizeof(user_bz), "%s%s", szRedStr, tmp_bz);
            } else {
                if (strlen(tmp_bz) != 0)
                    snprintf(user_bz, sizeof(user_bz), "%s %s", szRedStr, tmp_bz);
                else
                    snprintf(user_bz, sizeof(user_bz), "%s", szRedStr);
            }
        }
    } else if (stp_fpxx->zyfpLx == ZYFP_CEZS) {  //��ע�滻
        double fkce = atof(stp_Spxx->kce);
        // char szNlCr[16] = "";
        if (!fkce) {
            _WriteLog(LL_FATAL, "������շ�Ʊ�۳������д����\n");
            sprintf(errinfo, "������շ�Ʊ�۳������д����");
            i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
            goto Err;
        }
        struct Spxx *spxxFirst = (struct Spxx *)stp_fpxx->stp_MxxxHead->stp_next;
        if (spxxFirst->stp_next != NULL) {
            _WriteLog(LL_FATAL, "ZYFP_CEZS, spxx must be one line\n");
            sprintf(errinfo, "������ս�֧�ֵ�����Ʒ");
            i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
            goto Err;
        }
        // if (bDeviceType == DEVICE_CNTAX)
        // strcpy(szNlCr, "\r\n");//(�ɰط����Դ˴����࣬��Ʊ���û�У�
        // cntax�Ǻ�˰�۲��
        snprintf(user_bz, sizeof(user_bz), "�����˰��%.2f��%s", fkce, tmp_bz);
    }

    if (strlen(user_bz) != 0)
        strcpy(stp_fpxx->bz, user_bz);
    if ((stp_fpxx->bz[strlen(stp_fpxx->bz) - 2] == 0x0d) &&
        (stp_fpxx->bz[strlen(stp_fpxx->bz) - 1] == 0x0a)) {
        sprintf(errinfo, "��ע��Ϣĩβ��������س����м�<br/>");
        i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        goto Err;
    }
    if ((stp_fpxx->bz[0] == 0x0d) && (stp_fpxx->bz[1] == 0x0a)) {
        sprintf(errinfo, "��ע��Ϣͷ����������س����м�<br/>");
        i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        goto Err;
    }
    if (stp_fpxx->bz[strlen(stp_fpxx->bz) - 1] == 0x0a) {
        sprintf(errinfo, "��ע��Ϣĩβ��������س�\n");
        i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        goto Err;
    }
    if (stp_fpxx->bz[0] == 0x0a) {
        sprintf(errinfo, "��ע��Ϣͷ����������س�\n");
        i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        goto Err;
    }

    //�˴�������жϣ�json�����е����޳���bz�ֶΣ���Ϊbz�ֶδ��ں���ƴ�����
    if ((stp_fpxx->bz[0] == 0x20) || (stp_fpxx->bz[0] >= 0x09 && stp_fpxx->bz[0] <= 0x0d)) {
        sprintf(errinfo, "��ע��Ϣͷ����������ո�������ַ�");
        i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        goto Err;
    }
    if ((stp_fpxx->bz[strlen(stp_fpxx->bz) - 1] == 0x20) ||
        (stp_fpxx->bz[strlen(stp_fpxx->bz) - 1] >= 0x09 &&
         stp_fpxx->bz[strlen(stp_fpxx->bz) - 1] <= 0x0d)) {

        sprintf(errinfo, "��ע��Ϣĩβ��������ո�������ַ�\n");
        i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        goto Err;
    }
    cJSON_Delete(root);
    return 0;
Err:
    _WriteLog(LL_FATAL, "JSON Parse Err,%s", errinfo);
    cJSON_Delete(root);
    return i_ErrNum;
}

int ImportJsonFromFile(uint8 bDeviceType, uint8 *szJsonPath, HFPXX fpxx)
{
    struct stat statbuf;
    char errinfo[500] = {0};
    if (stat((const char *)szJsonPath, &statbuf) < 0)
        return -1;
    int nFileSize = statbuf.st_size;
    if (nFileSize <= 256)
        return -2;
    char *szJsonBuff = (char *)calloc(1, nFileSize);
    if (!szJsonBuff)
        return -3;
    FILE *fp = fopen((const char *)szJsonPath, "r");
    if (!fp) {
        free(szJsonBuff);
        return -4;
    }
    int nRead = fread(szJsonBuff, 1, nFileSize, fp);
    fclose(fp);
    if (nRead != nFileSize) {
        free(szJsonBuff);
        return -5;
    }
    if (CheckStrEncoding(szJsonBuff))
        return -6;
    int nRet = AnalyzeJsonBuff(bDeviceType, szJsonBuff, fpxx, errinfo);
    free(szJsonBuff);
    return nRet;
}

//���뵱ǰbin�ļ�ͬĿ¼��import.json
int ImportJSONToFpxx(HFPXX fpxx, HDEV hDev)
{
    char szJsonPath[256] = "";
    char errinfo[1024] = "";
    if (GetSelfAbsolutePath(szJsonPath, sizeof(szJsonPath)) < 0)
        return -1;
    *strrchr(szJsonPath, '/') = '\0';
    strcat(szJsonPath, "/import.json");
    struct stat statbuf;
    if (stat(szJsonPath, &statbuf) < 0) {
        _WriteLog(LL_FATAL, "No import.json found:%s", szJsonPath);
        return -1;
    }
    int nFileSize = statbuf.st_size;
    if (nFileSize == 0)
        return -2;
    //����Json
    if (ImportJsonFromFile(hDev->bDeviceType, (uint8 *)szJsonPath, fpxx) < 0)
        return -3;
    // update common first
    UpdateDevInfoToFPXX(hDev, fpxx);
    // update by device's type
    if (DEVICE_AISINO == hDev->bDeviceType) {
        if (AisinoUpdateNodeInfo(fpxx, hDev))
            return -4;
    } else if (DEVICE_NISEC == hDev->bDeviceType) {
        if (NisecUpdateNodeInfo(fpxx))
            return -4;
    } else if (DEVICE_CNTAX == hDev->bDeviceType) {
        if (CntaxUpdateNodeInfo(fpxx))
            return -4;
    } else {
        _WriteLog(LL_WARN, "Unknow device");
        return -1;
    }
    if (CheckFpxxNode(fpxx, hDev, errinfo))
        return -5;
    return 0;
}