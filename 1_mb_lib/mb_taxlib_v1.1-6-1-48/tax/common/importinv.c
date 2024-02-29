/*****************************************************************************
File name:   importinv.c
Description: 用于导入json结构，并将其转换为FPXX结构体工作
Author:      Zako
Version:     1.0
Date:        2019.07
History:
20200723     代码对齐,加强检测,所有json输入字段检查前后空格
20201124     蒙柏对齐
20201216     无关输入的json处理代码移出；代码拆分，将不同设备代码分离处理
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

//分配发票信息结构体空间
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


//分配发票信息结构体空间
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



//释放发票信息(包括商品明细)结构体
int FreeFpxx(HFPXX stp_Root)
{
    if (stp_Root == NULL)
        return -1;
    //循环释放每张发票的明细
    HFPXX stp_FpxxNode = stp_Root;
    if (!stp_FpxxNode)
        return -2;
    struct Spxx *stp_MxxxNode = stp_FpxxNode->stp_MxxxHead, *stp_TmpSpxxNode = NULL;
    //释放商品明细
    while (stp_MxxxNode) {
        stp_TmpSpxxNode = stp_MxxxNode->stp_next;
        free(stp_MxxxNode);
        stp_MxxxNode = stp_TmpSpxxNode;
    }
    //如果包含了原始发票信息，则释放
    if (stp_FpxxNode->pRawFPBin)
        free(stp_FpxxNode->pRawFPBin);
    //释放商品明细头
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
        _WriteLog(LL_INFO, "传入的数据全部为不可见字符或空格,数据：%s,%02x", strIn, strIn[0]);
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

//格式化导入json数据
int GetAvailImportData(cJSON *jsonInputParentNode, char *szGetNodeKey, char *szOut, int nOutMaxLen,
                       bool bCannotNull, char *json_err)
{
    memset(szOut, 0, nOutMaxLen);

    char key[100] = {0};
    strcpy(key, szGetNodeKey);
    mystrlwr(key);
    int nChildRet = 0;
    // jsonNode 有可能为空,因此不判断返回
    cJSON *jsonNode = cJSON_GetObjectItem(jsonInputParentNode, szGetNodeKey);

    if (bCannotNull)
        nChildRet = get_json_value_can_not_null(jsonNode, szOut, 1, nOutMaxLen);
    else
        nChildRet = get_json_value_can_null(jsonNode, szOut, 0, nOutMaxLen);
    if (nChildRet < 0) {

        if (json_err != NULL) {
            if (nChildRet == -1) {
                sprintf(json_err, "%s,字段缺失", key);
            } else if (nChildRet == -2) {
                sprintf(json_err, "%s,字段长度有误", key);
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

            sprintf(json_err, "%s,中不得存在回车、换行", key);
            _WriteLog(LL_FATAL, "JSON数据[%s]中不得存在回车、换行等特殊字符\n", szGetNodeKey);
            return -2;
        }
    }
    if ((szOut[strlen(szOut) - 2] == 0x5c) && (szOut[strlen(szOut) - 1] == 0x6e)) {
        sprintf(json_err, "%s,中不得存在\n字符", key);
        _WriteLog(LL_FATAL, "JSON数据[%s]中不得存在\"\\n\"字符\n", szGetNodeKey);
        return -2;
    }

    return 0;
}

int InsertMxxx(HFPXX stp_Root, struct Spxx *stp_InsertNode)
{
    if (stp_Root == NULL || stp_InsertNode == NULL)
        return -1;
    //头结点
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
    //头结点
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

//判断小数点位数
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
            //去除I、O、Z、S、V校验
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

//是否商品折扣,ture or false
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
    sprintf(szOut, "对应正数发票代码:%s号码:%s", string_0, string_1);
}

//检查红票通知单编号是否合法
bool CheckTzdh(char *szRedNum, char *szInvFlag)
{
    if (szRedNum == NULL)
        return false;
    if (strlen(szRedNum) != 16)
        return false;
    if (!strcmp(szRedNum, "0000000000000000"))
        return true;  //无需校验
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

//获取格式化的ZYFP红票备注
int GetZYFPRedNotes(uint8 bDeviceType, char *szRedNum, char *szInvFlag, char *szOut)
{
    if (!CheckTzdh(szRedNum, szInvFlag))
        return -1;
    if (DEVICE_AISINO == bDeviceType) {
        if (szInvFlag[0] == 's')
            sprintf(szOut, "%s%s", "开具红字增值税专用发票信息表编号", szRedNum);
        else if (szInvFlag[0] == 'f')
            sprintf(szOut, "%s%s", "开具红字货物运输业增值税专用发票信息表编号", szRedNum);
        else
            return -2;
    } else if ((DEVICE_NISEC == bDeviceType) ||
               (DEVICE_CNTAX == bDeviceType)) {  //税务ukey专用发票红字发票备注暂且参考税控盘
        sprintf(szOut, "%s%s", "红字发票信息表编号", szRedNum);
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
    //实际nisec软件中,spsl也是被截断的,但是考虑计算准确性,暂时不向开票软件对齐,有问题再解决
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
                               dj_tmp);  // nisec电票标准转换位6位精度
                PriceRound(dj_tmp, 6, spxx->dj);
            }
        }
    }
    //复杂情况单价计算待完善 差额征收 扣除额
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
                               dj_tmp);  // nisec专普票标准转换位长度共计16位
                p_point = strstr(dj_tmp, ".");
                int_len = strlen(dj_tmp) - strlen(p_point + 1);
                PriceRound(dj_tmp, 16 - int_len, spxx->dj);
            }
        }
    }

    //覆盖
    strcpy(fpxx->hsjbz, "0");
    strcpy(spxx->hsjbz, "0");
    return;
}

int check_spxx_hsjbz(struct Fpxx *stp_fpxx, struct Spxx *stp_Spxx, char *errinfo)
{
    if (strcmp(stp_Spxx->hsjbz, stp_fpxx->hsjbz) != 0) {
        sprintf(errinfo,
                "商品信息中含税价标志与发票中含税价标志不一致,商品中含税价标志%s,"
                "发票中含税价标志%s",
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
        sprintf(errinfo, "商品信息中商品编号格式错误,长度应为19位");
        return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
    }

    if (memcmp(spbh, "10701010", strlen("10701010")) == 0) {
        sprintf(errinfo, "暂不支持开具成品油发票");
        return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
    }
    if ((memcmp(spbh, "1090305", strlen("1090305")) == 0) ||
        (memcmp(spbh, "1090306", strlen("1090306")) == 0) ||
        (memcmp(spbh, "1090307", strlen("1090307")) == 0) ||
        (memcmp(spbh, "1090309", strlen("1090309")) == 0) ||
        (memcmp(spbh, "1090312", strlen("1090312")) == 0) ||
        (memcmp(spbh, "1090315", strlen("1090315")) == 0)) {
        sprintf(errinfo, "暂不支持开具机动车发票");
        return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
    }
    if ((memcmp(spbh, "103040501", strlen("103040501")) == 0) ||
        (memcmp(spbh, "103040502", strlen("103040502")) == 0) ||
        (memcmp(spbh, "103040503", strlen("103040503")) == 0) ||
        (memcmp(spbh, "103040504", strlen("103040504")) == 0)){
        sprintf(errinfo, "暂不支持开具电子烟发票");
        return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
    }
    //wang 02 01 稀土编码并入矿产品
    if (memcmp(spbh, "102", strlen("102")) == 0) {
        sprintf(errinfo, "暂不支持开具矿产品发票");
        return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
    }
    for (i = 0; i < strlen(spbh); i++) {
        if ((spbh[i] < '0') || (spbh[i] > '9')) {
            sprintf(errinfo, "商品信息中商品编号格式错误,数据应为纯数字");
            return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
        }
    }
    return 0;
}

int check_spxx_lslvbz(struct Spxx *stp_Spxx, char *errinfo)
{
    if (strlen(stp_Spxx->slv) < 4) {
        sprintf(errinfo, "商品信息中税率长度有误,应填入0.00、0.01、0.03格式");
        _WriteLog(LL_FATAL, "%s\n", errinfo);
        return DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
    }
	if ((strcmp(stp_Spxx->xsyh, "0") != 0) &&
		(strcmp(stp_Spxx->xsyh, "1") != 0))	{
		sprintf(errinfo, "商品信息中优惠政策标识仅支持0不使用优惠政策 1使用优惠政策");
		_WriteLog(LL_FATAL, "%s\n", errinfo);
		return DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
	}

    if (strlen(stp_Spxx->lslvbs) != 0) {
        int lslbz = atoi(stp_Spxx->lslvbs);
		if ((strcmp(stp_Spxx->lslvbs, "0") != 0) &&
			(strcmp(stp_Spxx->lslvbs, "1") != 0) &&
			(strcmp(stp_Spxx->lslvbs, "2") != 0) &&
			(strcmp(stp_Spxx->lslvbs, "3") != 0)){
			sprintf(errinfo, "商品信息中税率标识仅支持0出口退税 1免税 2不征收 3普通零税率和空");
			_WriteLog(LL_FATAL, "%s\n", errinfo);
			return DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
		}


        if (lslbz == 0) {
            sprintf(errinfo, "出口退税方式请使用零税率开具,并在备注中填入出口退税相关信息");
            _WriteLog(LL_FATAL, "%s\n", errinfo);
            return DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        } else if ((lslbz == 1) && (strcmp(stp_Spxx->slv, "0.00") != 0)) {
            sprintf(errinfo, "商品信息中税率标识为1免税，但税率非0.00,税率：%s", stp_Spxx->slv);
            _WriteLog(LL_FATAL, "%s\n", errinfo);
            return DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        } else if ((lslbz == 2) && (strcmp(stp_Spxx->slv, "0.00") != 0)) {
            sprintf(errinfo, "商品信息中税率标识为2不征收，但税率非0.00,税率：%s", stp_Spxx->slv);
            _WriteLog(LL_FATAL, "%s\n", errinfo);
            return DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        } else if ((lslbz == 3) && (strcmp(stp_Spxx->slv, "0.00") != 0)) {
            sprintf(errinfo, "商品信息中税率标识为3普通零税率，但税率非0.00,税率：%s",
                    stp_Spxx->slv);
            _WriteLog(LL_FATAL, "%s\n", errinfo);
            return DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        } else if ((lslbz == 3) && (atoi(stp_Spxx->xsyh) != 0)) {
            sprintf(errinfo, "商品信息中税率标识为3普通零税率，但优惠政策标识非0，优惠政策标识：%s",
                    stp_Spxx->xsyh);
            _WriteLog(LL_FATAL, "%s\n", errinfo);
            return DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        } else {
            strcpy(stp_Spxx->slv, "0");
        }
    }
    if ((atoi(stp_Spxx->xsyh) == 0) && (strcmp(stp_Spxx->slv, "0.00") == 0) &&
        (atoi(stp_Spxx->lslvbs) != 3)) {
        sprintf(errinfo, "税率为0.00且优惠政策为0时,税率标识应为3");
        _WriteLog(LL_FATAL, "%s\n", errinfo);
        return DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
    }
    if ((atof(stp_Spxx->slv) != 0) && (strlen(stp_Spxx->lslvbs) != 0)) {
        _WriteLog(LL_WARN, "商品税率%s,商品金额%s", stp_Spxx->slv, stp_Spxx->je);
        sprintf(errinfo, "商品非零税率,税率标识应为空");
        _WriteLog(LL_FATAL, "%s\n", errinfo);
        return DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
    }
    if ((atoi(stp_Spxx->lslvbs) == 1) || ((atoi(stp_Spxx->lslvbs) == 2))) {
        if (atoi(stp_Spxx->xsyh) != 1) {
            sprintf(errinfo, "零税率标识为1或2时,优惠政策标识应为1");
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
        sprintf(errinfo, "优惠政策标识填写有误");
        _WriteLog(LL_FATAL, "%s\n", errinfo);
        return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
    }
    // int fphxz = IsZKXX(bDeviceType, stp_Spxx->fphxz);
    if (atof(stp_Spxx->xsyh) == 1) {
        if (strlen(stp_Spxx->yhsm) < 4)  //最少为免税，4个字符
        {
            sprintf(errinfo, "使用优惠政策标识,需正确填写优惠说明，即ZZSTSGL");
            _WriteLog(LL_FATAL, "%s\n", errinfo);
            return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
        }
        if (strcmp(stp_Spxx->yhsm, "按3%简易征收") == 0) {
            if (strcmp(stp_Spxx->slv, "0.03") != 0) {
                sprintf(errinfo, "按3%%简易征收，税率需为0.03");
                _WriteLog(LL_FATAL, "%s\n", errinfo);
                return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
            }
            _WriteLog(LL_FATAL, "按3%%简易征收\n");
        } else if (strcmp(stp_Spxx->yhsm, "按5%简易征收") == 0) {
            if (strcmp(stp_Spxx->slv, "0.05") != 0) {
                sprintf(errinfo, "按5%%简易征收，税率需为0.05");
                _WriteLog(LL_FATAL, "%s\n", errinfo);
                return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
            }
            _WriteLog(LL_FATAL, "按5%%简易征收\n");
        } else if (strcmp(stp_Spxx->yhsm, "免税") == 0) {
            // _WriteLog(LL_FATAL, "免税\n");
            if ((atof(stp_Spxx->slv) != 0) || (strcmp(stp_Spxx->se, "0.00") != 0)) {
                sprintf(errinfo, "免税要求商品税率和税额为0.00");
                _WriteLog(LL_FATAL, "%s\n", errinfo);
                return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
            }
            strcpy(stp_Spxx->slv, "0");
            strcpy(stp_Spxx->lslvbs, "1");
        } else if (strcmp(stp_Spxx->yhsm, "不征税") == 0) {
            if ((atof(stp_Spxx->slv) != 0) || (strcmp(stp_Spxx->se, "0.00") != 0)) {
                sprintf(errinfo, "免税要求商品税率和税额为0.00");
                _WriteLog(LL_FATAL, "%s\n", errinfo);
                return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
            }
            strcpy(stp_Spxx->slv, "0");
            strcpy(stp_Spxx->lslvbs, "2");
        } else if (strcmp(stp_Spxx->yhsm, "按5%简易征收减按1.5%计征") == 0) {
            sprintf(errinfo, "按5%%简易征收减按1.5%%计征,暂不支持");
            _WriteLog(LL_FATAL, "%s\n", errinfo);
            return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
        } else if (strcmp(stp_Spxx->yhsm, "按5%简易征收减按3%计征") == 0) {

            sprintf(errinfo, "按5%%简易征收减按3%%计征,暂不支持");
            _WriteLog(LL_FATAL, "%s\n", errinfo);
            return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
        } else if (strcmp(stp_Spxx->yhsm, "超税负3%即征即退") == 0) {
            _WriteLog(LL_FATAL, "超税负3%%即征即退\n");
        } else if (strcmp(stp_Spxx->yhsm, "超税负8%即征即退") == 0) {
            _WriteLog(LL_FATAL, "超税负8%%即征即退\n");
        } else if (strcmp(stp_Spxx->yhsm, "超税负12%即征即退") == 0) {
            _WriteLog(LL_FATAL, "超税负12%%即征即退\n");
        } else {
            sprintf(errinfo, "%s,暂不支持", stp_Spxx->yhsm);
            _WriteLog(LL_FATAL, "%s\n", errinfo);
            return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
        }
    } else {
        if (strlen(stp_Spxx->yhsm) > 0) {
            sprintf(errinfo, "优惠政策标识为0时,不能填入增值税特殊管理说明(即优惠说明)");
            _WriteLog(LL_FATAL, "%s\n", errinfo);
            return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
        }
    }

    if ((stp_fpxx->zyfpLx == ZYFP_CEZS) || (strlen(stp_Spxx->yhsm) != 0)) {
        // if ((fphxz != 0) && (bDeviceType == DEVICE_NISEC)) {
        //    sprintf(errinfo, "税控盘不支持,发票行性质不为正常行,无法开具差额征收或减按计征发票");
        //    _WriteLog(LL_FATAL, "%s\n", errinfo);
        //    return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
        //}
        // if ((fphxz != 0) && (bDeviceType == DEVICE_CNTAX)) {
        //    sprintf(errinfo,
        //    "税务UKEY不支持,发票行性质不为正常行,无法开具差额征收或减按计征发票");
        //    _WriteLog(LL_FATAL, "%s\n", errinfo);
        //    return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
        //}
        if (stp_fpxx->zyfpLx == ZYFP_CEZS) {
            if (stp_fpxx->spsl != 1) {
                sprintf(errinfo, "商品数量超过1项,无法开具差额征收");
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
//        //单价未填入情况不校验
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
//                sprintf(errinfo, "金税盘发票填入的单价%s的小数点不得小于2位或超过15位长度",
//                        stp_Spxx->dj);
//                logout(INFO, "TAXLIB", "发票开具解析","%s\n", errinfo);
//                return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
//            }
//            if (JudgeDecimalPoint(stp_Spxx->sl, 0, 15) != 0) {
//                sprintf(errinfo, "金税盘发票填入的数量%s的小数点不得超过15位长度", stp_Spxx->sl);
//                logout(INFO, "TAXLIB", "发票开具解析","%s\n", errinfo);
//                return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
//            }
//            if (strlen(stp_Spxx->dj) > 20) {
//                sprintf(errinfo, "金税盘发票填入的单价%s的总长度不得超过20位长度", stp_Spxx->dj);
//                logout(INFO, "TAXLIB", "发票开具解析","%s\n", errinfo);
//                return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
//            }
//			if (strlen(stp_Spxx->sl) > 20) {
//                sprintf(errinfo, "金税盘发票填入的数量%s的总长度不得超过20位长度", stp_Spxx->sl);
//                logout(INFO, "TAXLIB", "发票开具解析","%s\n", errinfo);
//                return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
//            }
//        }
//    } else if ((bDeviceType == DEVICE_NISEC) || (bDeviceType == DEVICE_CNTAX)) {
//        if (stp_fpxx->fplx == 26) {
//            _WriteLog(LL_FATAL, "税控盘电票最多保留小数点6位\n");
//            if (JudgeDecimalPoint(stp_Spxx->dj, 0, 6) != 0) {
//                if (JudgeDecimalPoint(stp_Spxx->dj, 2, 8) == 0) //兼容金税盘8位，直接默认修改为6位
//                {
//                    double dj_doub;
//                    char dj_str[48] = {0};
//                    dj_doub = atof(stp_Spxx->dj);
//                    sprintf(dj_str, "%.6f", dj_doub);
//                    logout(INFO, "TAXLIB",
//                    "发票开具解析","单价超过小数6位【%s】,默认直接修改成小数6位【%s】\n",
//                    stp_Spxx->dj,
//                              dj_str);
//                    memset(stp_Spxx->dj, 0, sizeof(stp_Spxx->dj));
//                    memcpy(stp_Spxx->dj, dj_str, strlen(dj_str));
//                    return 0;
//                }
//
//                sprintf(errinfo, "税控盘电票填入的单价%s的小数点不得小于2位或超过6位长度",
//                        stp_Spxx->dj);
//                logout(INFO, "TAXLIB", "发票开具解析","%s\n", errinfo);
//                return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
//            }
//
//			if (JudgeDecimalPoint(stp_Spxx->sl, 0, 6) != 0) {
//				sprintf(errinfo, "税控盘电票填入的数量%s的小数点不得超过6位长度",
// stp_Spxx->sl); 				logout(INFO, "TAXLIB", "发票开具解析","%s\n",
// errinfo); return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
//			}
//
//            if (strlen(stp_Spxx->dj) > 16) {
//                sprintf(errinfo, "税控盘电票填入的单价%s的总长度不得超过16位长度", stp_Spxx->dj);
//                logout(INFO, "TAXLIB", "发票开具解析","%s\n", errinfo);
//                return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
//            }
//
//			if (strlen(stp_Spxx->sl) > 16 + add_len_sl) {
//                sprintf(errinfo, "税控盘电票填入的数量%s的总长度不得超过16位长度", stp_Spxx->sl);
//                logout(INFO, "TAXLIB", "发票开具解析","%s\n", errinfo);
//                return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
//            }
//        } else if ((stp_fpxx->fplx == 4) || (stp_fpxx->fplx == 7)) {
//
//            if ((stp_Spxx->sl[0] == '0') && (stp_Spxx->sl[1] == '.')) {
//                if (JudgeDecimalPoint(stp_Spxx->sl, 0, 15) != 0) {
//                    sprintf(errinfo, "税控盘专普票填入的数量%s的小数点不得超过15位长度",
//                            stp_Spxx->sl);
//                    logout(INFO, "TAXLIB", "发票开具解析","%s\n", errinfo);
//                    return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
//                }
//				if (strlen(stp_Spxx->sl) > 17 + add_len_sl) {
//                    sprintf(errinfo, "税控盘专普票填入的数量%s不得超过总长度17字节",
//                    stp_Spxx->sl); logout(INFO, "TAXLIB", "发票开具解析","%s\n", errinfo);
//                    return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
//                }
//            } else {
//                if (JudgeDecimalPoint(stp_Spxx->sl, 0, 14) != 0) {
//                    sprintf(errinfo, "税控盘专普票填入的数量%s的小数点不得超过14位长度",
//                            stp_Spxx->sl);
//                    logout(INFO, "TAXLIB", "发票开具解析","%s\n", errinfo);
//                    return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
//                }
//				if (strlen(stp_Spxx->sl) > 16 + add_len_sl) {
//                    sprintf(errinfo, "税控盘专普票填入的数量%s不得超过总长度16字节",
//                    stp_Spxx->sl); logout(INFO, "TAXLIB", "发票开具解析","%s\n", errinfo);
//                    return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
//                }
//            }
//            if ((stp_Spxx->dj[0] == '0') && (stp_Spxx->dj[1] == '.')) {
//                if (JudgeDecimalPoint(stp_Spxx->dj, 0, 15) != 0) {
//                    sprintf(errinfo, "税控盘专普票填入的单价%s的小数点不得超过15位长度",
//                            stp_Spxx->dj);
//                    logout(INFO, "TAXLIB", "发票开具解析","%s\n", errinfo);
//                    return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
//                }
//                if (strlen(stp_Spxx->dj) > 17) {
//                    sprintf(errinfo, "税控盘专普票填入的单价%s不得超过总长度17字节",
//                    stp_Spxx->dj); logout(INFO, "TAXLIB", "发票开具解析","%s\n", errinfo);
//                    return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
//                }
//            } else {
//                if (JudgeDecimalPoint(stp_Spxx->dj, 0, 14) != 0) {
//                    sprintf(errinfo, "税控盘专普票填入的单价%s的小数点不得超过14位长度",
//                            stp_Spxx->dj);
//                    logout(INFO, "TAXLIB", "发票开具解析","%s\n", errinfo);
//                    return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
//                }
//                if (strlen(stp_Spxx->dj) > 16) {
//                    sprintf(errinfo, "税控盘专普票填入的单价%s不得超过总长度16字节",
//                    stp_Spxx->dj); logout(INFO, "TAXLIB", "发票开具解析","%s\n", errinfo);
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
        //单价未填入情况不校验
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
            //_WriteLog(LL_INFO, "税控盘电票最多保留小数点6位");
            if (JudgeDecimalPoint(stp_Spxx->dj, 2, 8) == 0)  //兼容金税盘8位，直接默认修改为6位
            {
                double dj_doub;
                char dj_str[48] = {0};
                dj_doub = atof(stp_Spxx->dj);
                sprintf(dj_str, "%.6f", dj_doub);
                // logout(INFO, "TAXLIB",
                // "发票开具解析","单价超过小数6位【%s】,默认直接修改成小数6位【%s】\r\n",
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
                       (stp_Spxx->sl[2] == '.'))  //数量可能存在负数，单价不存在
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
        sprintf(errinfo, "税盘发票填入的数量%s的小数点精度有误", stp_Spxx->sl);
        _WriteLog(LL_INFO, "%s\n", errinfo);
        return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
    }

    if (JudgeDecimalPoint(stp_Spxx->dj, dj_point_min_len, dj_point_max_len) != 0) {
        sprintf(errinfo, "税盘发票填入的单价%s的小数点不得小于%d位或超过%d位长度", stp_Spxx->dj,
                dj_point_min_len, dj_point_max_len);
        _WriteLog(LL_INFO, "%s\n", errinfo);
        return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
    }
    if (JudgeDecimalPoint(stp_Spxx->sl, 0, sl_point_max_len) != 0) {
        sprintf(errinfo, "税盘发票填入的数量%s的小数点不得超过%d位长度", stp_Spxx->sl,
                sl_point_max_len);
        _WriteLog(LL_INFO, "%s\n", errinfo);
        return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
    }
    if (strlen(stp_Spxx->dj) > dj_str_max_len) {
        sprintf(errinfo, "税盘发票填入的单价%s的总长度不得超过%d位长度", stp_Spxx->dj,
                dj_str_max_len);
        _WriteLog(LL_INFO, "%s\n", errinfo);
        return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
    }
    if (strlen(stp_Spxx->sl) > sl_str_max_len) {
        sprintf(errinfo, "税盘发票填入的数量%s的总长度不得超过%d位长度", stp_Spxx->sl,
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
        sprintf(errinfo, "红字发票填入的商品数量应为负数");
        _WriteLog(LL_FATAL, "%s\n", errinfo);
        return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
    }
    if ((stp_fpxx->isRed == 1) && (atof(stp_Spxx->je) > 0)) {
        sprintf(errinfo, "红字发票填入的商品金额应为负数");
        _WriteLog(LL_FATAL, "%s\n", errinfo);
        return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
    }
    if ((stp_fpxx->isRed == 1) && (atof(stp_Spxx->se) > 0)) {
        sprintf(errinfo, "红字发票填入的商品税额应为负数");
        _WriteLog(LL_FATAL, "%s\n", errinfo);
        return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
    }
    // nisec中100.00转换为100；cntax不转换，转了影响也不大(蒙柏方已确认，税务ukey开票软件也是将100.00转换成100)
    if ((bDeviceType == DEVICE_NISEC) || (bDeviceType == DEVICE_CNTAX)) {
        MemoryFormart(MF_TRANSLATE_TAILZERO, stp_Spxx->dj, strlen(stp_Spxx->dj));
        if (stp_Spxx->dj[strlen(stp_Spxx->dj) - 1] == '.') {
            stp_Spxx->dj[strlen(stp_Spxx->dj) - 1] = '\0';
        }
        //_WriteLog(LL_INFO, "转换后的单价为为：%s", stp_Spxx->dj);
    }
    if (JudgePrice(stp_Spxx->dj, sizeof(stp_Spxx->dj)) < 0) {
        sprintf(errinfo, "填入的商品单价存在非法字符");
        _WriteLog(LL_FATAL, "%s\n", errinfo);
        return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
    }
    if (JudgePrice(stp_Spxx->sl, sizeof(stp_Spxx->sl)) < 0) {
        sprintf(errinfo, "填入的商品数量存在非法字符");
        _WriteLog(LL_FATAL, "%s\n", errinfo);
        return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
    }
    if (JudgePrice(stp_Spxx->se, sizeof(stp_Spxx->se)) < 0) {
        sprintf(errinfo, "填入的商品税额存在非法字符");
        _WriteLog(LL_FATAL, "%s\n", errinfo);
        return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
    }
    if (JudgePrice(stp_Spxx->je, sizeof(stp_Spxx->je)) < 0) {
        sprintf(errinfo, "填入的商品金额存在非法字符");
        _WriteLog(LL_FATAL, "%s\n", errinfo);
        return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
    }
    if (JudgePrice(stp_Spxx->slv, sizeof(stp_Spxx->slv)) < 0) {
        sprintf(errinfo, "填入的商品税率存在非法字符");
        _WriteLog(LL_FATAL, "%s\n", errinfo);
        return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
    }

    if (strcmp(stp_Spxx->je, "0.00") == 0) {
        sprintf(errinfo, "不允许开具金额为0的商品");
        _WriteLog(LL_FATAL, "%s\n", errinfo);
        return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
    }
    if ((stp_fpxx->zyfpLx == ZYFP_CEZS) && (atoi(stp_Spxx->hsjbz) != 0)) {
        sprintf(errinfo, "不允许含税模式开具差额征收");
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
        sprintf(errinfo, "填入的单价、金额、税额不能为-0.00");
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
    if (fphxz == 0 || fphxz == 2)  //不为折扣行
    {
        f_sl = atof(stp_Spxx->sl);     //数量
        f_slv = atof(stp_Spxx->slv);   //税率
        f_bhsje = atof(stp_Spxx->je);  //金额
        f_kce = atof(stp_Spxx->kce);   //差额
        f_se = atof(stp_Spxx->se);     //税额
        f_dj = atof(stp_Spxx->dj);     //单价

        if ((dj_exist == 0) && (sl_exist == 1)) {
            sprintf(errinfo, "仅填入数量未填入单价");
            _WriteLog(LL_FATAL, "%s\n", errinfo);
            return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
        } else if ((dj_exist == 1) && (sl_exist == 0)) {
            sprintf(errinfo, "仅填入单价未填入数量");
            _WriteLog(LL_FATAL, "%s\n", errinfo);
            return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
        } else if ((dj_exist == 0) && (sl_exist == 0))  //不填入单价数量模式
        {
            //_WriteLog(LL_INFO, "stp_Spxx->hsje = %s", stp_Spxx->hsje);
            if (strlen(stp_Spxx->hsje) != 0)  //传入含税金额方式
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
                _WriteLog(LL_FATAL, "差额征收红字发票不校验税额");
            } else {
                if (strcmp(stp_Spxx->se, s_cal_se) != 0) {
                    if (stp_fpxx->allow_error > 0) {

                        se_diff = abs(f_cal_se * 100 - f_se * 100);
                        if (se_diff > stp_fpxx->allow_error) {
                            sprintf(errinfo,
                                    "1无单价模式，在允许误差为%"
                                    "d模式下，填入的税额与金额计算的税额存在较大误差计算税额%s,"
                                    "填入的税额%s",
                                    stp_fpxx->allow_error, s_cal_se, stp_Spxx->se);
                            _WriteLog(LL_FATAL, "%s\n", errinfo);
                            return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
                        }
                        if (stp_fpxx->allow_error > 1) {

                            long double d_se_diff = fabs(f_cal_se - f_se);
                            long double d_allow_error =
                                (long double)((long double)stp_fpxx->allow_error / 100);
                            _WriteLog(LL_FATAL, "当前计算差绝对值 = %f 允许误差 = %f", d_se_diff,
                                      d_allow_error);
                            if (d_se_diff > d_allow_error) {
                                sprintf(errinfo,
                                        "2无单价模式，在允许误差为%"
                                        "d模式下，填入的税额与金额计算的税额存在误差计算税额%s,"
                                        "填入的税额%s",
                                        stp_fpxx->allow_error, s_cal_se, stp_Spxx->se);
                                _WriteLog(LL_FATAL, "%s\n", errinfo);
                                return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
                            }
                        }

                        stp_fpxx->d_zsewc += (atof(stp_Spxx->se) - atof(s_cal_se));
                        _WriteLog(LL_INFO,
                                  "无单价模式,税额不等，但在误差允许范围内,计算税额%s,填入的税额%s,"
                                  "允许误差%d",
                                  s_cal_se, stp_Spxx->se, stp_fpxx->allow_error);
                    } else {
                        sprintf(
                            errinfo,
                            "无单价模式，填入的税额与金额计算的税额存在误差计算税额%s,填入的税额%s",
                            s_cal_se, stp_Spxx->se);
                        _WriteLog(LL_FATAL, "%s\n", errinfo);
                        return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
                    }
                }
            }
        } else  //填入单价数量模式
        {
            if (strlen(stp_Spxx->hsje) != 0)  //传入含税金额方式
            {
                f_cal_hsje = atof(stp_Spxx->hsje);
                f_cal_bhsje = (f_cal_hsje - f_kce) / (1 + f_slv) + f_kce;
            } else  //不传含税金额模式
            {
                f_cal_hsje = f_bhsje + f_se;
                if (atoi(stp_Spxx->hsjbz) == 0)  //商品不含税模式
                {
                    f_cal_bhsje = f_dj * f_sl;
                } else  //商品含税模式
                {
                    f_cal_hsje = f_dj * f_sl;
                    char s_cal_hsje[50] = {0};
                    sprintf(s_cal_hsje, "%Lf", f_cal_hsje);
                    //_WriteLog(LL_FATAL, "1计算含税金额%s", s_cal_hsje);
                    memset(tmp_price, 0, sizeof(tmp_price));

                    PriceRound(s_cal_hsje, 2, tmp_price);
                    //_WriteLog(LL_FATAL, "2计算含税金额%s", tmp_price);
                    if (strcmp(tmp_price, "-0.00") == 0) {
                        memset(s_cal_hsje, 0, sizeof(s_cal_hsje));
                        strcpy(s_cal_hsje, "0.00");
                    } else {
                        memset(s_cal_hsje, 0, sizeof(s_cal_hsje));
                        strcpy(s_cal_hsje, tmp_price);
                    }
                    f_cal_hsje = atof(s_cal_hsje);
                    //_WriteLog(LL_FATAL, "3计算含税金额%f", f_cal_hsje);
                    //_WriteLog(LL_FATAL, "4计算含税金额%s", s_cal_hsje);
                    f_cal_bhsje = f_cal_hsje - f_se;
                    //_WriteLog(LL_FATAL, "商品含税模式，计算含税金额%f", f_cal_bhsje);
                }
            }

            if (DEVICE_AISINO == bDeviceType) {
                if (JudgeDecimalPoint(stp_Spxx->dj, 2, 18) != 0) {
                    sprintf(errinfo, "金税盘单价小数点后位数小于2位或大于18位,单价%s",
                            stp_Spxx->dj);
                    _WriteLog(LL_FATAL, "%s\n", errinfo);
                    return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
                }
            } else {
                if (JudgeDecimalPoint(stp_Spxx->dj, 0, 18) != 0) {
                    sprintf(errinfo, "税控盘/税务ukey单价小数点后位数小于0位或大于18位,单价%s",
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
                                      "税控盘税务Ukey电子普票单价精度位较低,再次校验单价误差");
                            f_cal_dj = f_cal_bhsje / f_sl;
                            char s_cal_dj[50] = {0};
                            sprintf(s_cal_dj, "%Lf", f_cal_dj);
                            memset(tmp_price, 0, sizeof(tmp_price));
                            PriceRound(s_cal_dj, 6, tmp_price);
                            if (strcmp(stp_Spxx->dj, s_cal_dj) != 0) {
                                _WriteLog(LL_INFO,
                                          "税控盘税务Ukey电子普票单价精度位较低,"
                                          "再次校验单价误差仍然存在误差,计算单价%s,填入的单价%s",
                                          s_cal_dj, stp_Spxx->dj);
                                sprintf(errinfo,
                                        "有单价模式,"
                                        "在允许误差为%"
                                        "d模式下，填入的金额与计算金额存在较大误差计算金额%s,"
                                        "填入的金额%s,计算金额%s,填入的金额%s",
                                        stp_fpxx->allow_error, s_cal_je, stp_Spxx->je, s_cal_je,
                                        stp_Spxx->je);
                                _WriteLog(LL_FATAL, "%s\n", errinfo);
                                return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
                            } else {
                                _WriteLog(LL_INFO,
                                          "税控盘税务Ukey电子普票单价精度位较低,再次校验单价通过,"
                                          "计算单价%s,填入的单价%s",
                                          s_cal_dj, stp_Spxx->dj);
                            }
                        } else {
                            sprintf(errinfo,
                                    "有单价模式,"
                                    "在允许误差模式下，填入的金额与计算金额存在较大误差计算金额%s,"
                                    "填入的金额%s",
                                    s_cal_je, stp_Spxx->je);
                            _WriteLog(LL_FATAL, "%s\n", errinfo);
                            return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
                        }
                    }
                } else {
                    sprintf(errinfo,
                            "有单价模式,填入的单价与数量乘积超过1分钱误差,商品行数%s,商品名称%s,"
                            "传入金额%s,计算金额%s",
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

            //_WriteLog(LL_FATAL, "计算税额%s,填入的税额%s", s_cal_se, stp_Spxx->se);
            // se_diff = abs(f_cal_se * 100 - f_se * 100);
            // if (se_diff != 0) {
            if ((stp_fpxx->zyfpLx == ZYFP_CEZS) && (stp_fpxx->isRed == 1)) {
                _WriteLog(LL_FATAL, "差额征收红字发票不校验税额");
            } else {
                if (strcmp(stp_Spxx->se, s_cal_se) != 0) {
                    if (stp_fpxx->allow_error > 0) {
                        se_diff = abs(f_cal_se * 100 - f_se * 100);
                        if (se_diff > stp_fpxx->allow_error) {
                            sprintf(errinfo,
                                    "1有单价模式，在允许误差为%"
                                    "d模式下，填入的税额与金额计算的税额存在误差计算税额%s,"
                                    "填入的税额%s",
                                    stp_fpxx->allow_error, s_cal_se, stp_Spxx->se);
                            _WriteLog(LL_FATAL, "%s\n", errinfo);
                            return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
                        }
                        if (stp_fpxx->allow_error > 1) {

                            long double d_se_diff = fabs(f_cal_se - f_se);
                            long double d_allow_error =
                                (long double)((long double)stp_fpxx->allow_error / 100);
                            _WriteLog(LL_FATAL, "当前计算差绝对值 = %f 允许误差 = %f", d_se_diff,
                                      d_allow_error);
                            if (d_se_diff > d_allow_error) {
                                sprintf(errinfo,
                                        "2有单价模式，在允许误差为%"
                                        "d模式下，填入的税额与金额计算的税额存在误差计算税额%s,"
                                        "填入的税额%s",
                                        stp_fpxx->allow_error, s_cal_se, stp_Spxx->se);
                                _WriteLog(LL_FATAL, "%s\n", errinfo);
                                return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
                            }
                        }
                        stp_fpxx->d_zsewc += (atof(stp_Spxx->se) - atof(s_cal_se));
                        //_WriteLog(LL_INFO, "有单价模式,税额不等，但在误差允许范围内");
                        _WriteLog(LL_INFO,
                                  "有单价模式,税额不等，但在误差允许范围内,计算税额%s,填入的税额%s,"
                                  "允许误差%d,误差绝对值：%d",
                                  s_cal_se, stp_Spxx->se, stp_fpxx->allow_error, se_diff);
                    } else {
                        sprintf(
                            errinfo,
                            "有单价模式，填入的税额与金额计算的税额存在误差计算税额%s,填入的税额%s",
                            s_cal_se, stp_Spxx->se);
                        _WriteLog(LL_FATAL, "%s\n", errinfo);
                        return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
                    }
                }
            }
            //_WriteLog(LL_FATAL, "商品【%s%s】校验成功\n", stp_Spxx->spsmmc, stp_Spxx->spmc);
        }
        result = check_dj_len(bDeviceType, stp_fpxx, stp_Spxx, errinfo);
        if (result < 0) {
            return result;
        }
    } else if (fphxz == 1)  //折扣行
    {
        if (stp_fpxx->isRed)  //红字发票暂不支持折扣
        {
            sprintf(errinfo, "暂不支持红字发票折扣行");
            _WriteLog(LL_FATAL, "%s\n", errinfo);
            return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
        }

        if ((dj_exist != 0) || (sl_exist != 0)) {
            sprintf(errinfo, "折扣行无需填入单价和数量");
            _WriteLog(LL_FATAL, "%s\n", errinfo);
            return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
        }
        if ((strlen(stp_Spxx->ggxh) != 0) || (strlen(stp_Spxx->jldw) != 0)) {
            sprintf(errinfo, "折扣行无需填入规格型号和计量单位");
            _WriteLog(LL_FATAL, "%s\n", errinfo);
            return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
        }

        f_bhsje = atof(stp_Spxx->je);  //金额
        f_se = atof(stp_Spxx->se);     //金额
		f_slv = atof(stp_Spxx->slv);   //税率

		if ((f_bhsje >= 0) || (f_se > 0)) {
			sprintf(errinfo, "折扣行金额应为负数，税额应为负数或0");
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

		//_WriteLog(LL_FATAL, "计算税额%s,填入的税额%s", s_cal_se, stp_Spxx->se);
		// se_diff = abs(f_cal_se * 100 - f_se * 100);
		// if (se_diff != 0) {

		if (strcmp(stp_Spxx->se, s_cal_se) != 0) {
			if (stp_fpxx->allow_error > 0) {
				se_diff = abs(f_cal_se * 100 - f_se * 100);
				if (se_diff > stp_fpxx->allow_error) {
					sprintf(errinfo,"折扣行，在允许误差为%d模式下，填入的税额与金额计算的税额存在误差计算税额%s,填入的税额%s",stp_fpxx->allow_error, s_cal_se, stp_Spxx->se);
					_WriteLog(LL_FATAL, "%s\n", errinfo);
					return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
				}
				if (stp_fpxx->allow_error > 1) {
					long double d_se_diff = fabs(f_cal_se - f_se);
					long double d_allow_error =(long double)((long double)stp_fpxx->allow_error / 100);
					_WriteLog(LL_FATAL, "当前计算差绝对值 = %f 允许误差 = %f", d_se_diff,d_allow_error);
					if (d_se_diff > d_allow_error) {
						sprintf(errinfo,"折扣行，在允许误差为%d模式下，填入的税额与金额计算的税额存在误差计算税额%s,填入的税额%s",stp_fpxx->allow_error, s_cal_se, stp_Spxx->se);
						_WriteLog(LL_FATAL, "%s\n", errinfo);
						return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
					}
				}
				stp_fpxx->d_zsewc += (atof(stp_Spxx->se) - atof(s_cal_se));
				//_WriteLog(LL_INFO, "有单价模式,税额不等，但在误差允许范围内");
				_WriteLog(LL_INFO,",税额不等，但在误差允许范围内,计算税额%s,填入的税额%s,允许误差%d,误差绝对值：%d",s_cal_se, stp_Spxx->se, stp_fpxx->allow_error, se_diff);
			}else {
				sprintf(errinfo,"折扣行，填入的税额与金额计算的税额存在误差计算税额%s,填入的税额%s",s_cal_se, stp_Spxx->se);
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
        sprintf(errinfo, "专用发票不允许开具零税率发票");
        _WriteLog(LL_FATAL, "%s\n", errinfo);
        return DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
    }
    if (bDeviceType == DEVICE_AISINO) {
        if (strlen(stp_fpxx->zyspsmmc) + strlen(stp_fpxx->zyspmc) > 92) {
            sprintf(errinfo, "金税盘主要商品税目+主要商品名称长度超过92字节错误");
            _WriteLog(LL_FATAL, "%s\n", errinfo);
            return DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        }
        if (strlen(stp_Spxx->spsmmc) + strlen(stp_Spxx->spmc) > 92) {
            sprintf(errinfo, "金税盘商品税目+商品名称长度超过92字节错误");
            _WriteLog(LL_FATAL, "%s\n", errinfo);
            return DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        }
    } else if ((bDeviceType == DEVICE_NISEC) || (bDeviceType == DEVICE_CNTAX)) {

        if (strlen(stp_fpxx->zyspsmmc) + strlen(stp_fpxx->zyspmc) > 100) {
            sprintf(errinfo, "税控盘主要商品税目+主要商品名称长度超过100字节错误");
            _WriteLog(LL_FATAL, "%s\n", errinfo);
            return DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        }
        if (strlen(stp_Spxx->spsmmc) + strlen(stp_Spxx->spmc) > 100) {
            sprintf(errinfo, "税控盘商品税目+商品名称长度超过100字节错误");
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
        sprintf(errinfo, "发票综合税率中包含非法字符");
        return DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
    }
    if (JudgePrice(stp_fpxx->je, sizeof(stp_fpxx->je)) < 0) {
        sprintf(errinfo, "发票合计金额中包含非法字符");
        return DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
    }
    if (JudgePrice(stp_fpxx->se, sizeof(stp_fpxx->se)) < 0) {
        sprintf(errinfo, "发票合计税额包含非法字符");
        return DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
    }
    if (JudgePrice(stp_fpxx->jshj, sizeof(stp_fpxx->jshj)) < 0) {
        sprintf(errinfo, "发票价税合计包含非法字符");
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

//红蓝发票比较
int Invoice_RBCompare(HFPXX red_fpxx, HFPXX bule_fpxx, char *errifno)
{
    int spsl;
    int i;
    if ((strcmp(red_fpxx->xfmc, bule_fpxx->xfmc) != 0) ||
        (strcmp(red_fpxx->xfsh, bule_fpxx->xfsh) != 0) ||
        (strcmp(red_fpxx->xfdzdh, bule_fpxx->xfdzdh) != 0) ||
        (strcmp(red_fpxx->xfyhzh, bule_fpxx->xfyhzh) != 0)) {
        sprintf(errifno, "红字发票与蓝字发票销方信息匹配不一致");
        return -1;
    }
    if ((strcmp(red_fpxx->gfmc, bule_fpxx->gfmc) != 0) ||
        (strcmp(red_fpxx->gfsh, bule_fpxx->gfsh) != 0) ||
        (strcmp(red_fpxx->gfdzdh, bule_fpxx->gfdzdh) != 0) ||
        (strcmp(red_fpxx->gfyhzh, bule_fpxx->gfyhzh) != 0)) {
        sprintf(errifno, "红字发票与蓝字发票购方信息匹配不一致");
        return -2;
    }
    if (red_fpxx->fplx != bule_fpxx->fplx) {
        sprintf(errifno, "红字发票与蓝字发票发票种类不一致");
        return -3;
    }
    float cal_je;
    float cal_se;
    float cal_jshj;
    cal_je = atof(red_fpxx->je) + atof(bule_fpxx->je);
    cal_se = atof(red_fpxx->se) + atof(bule_fpxx->se);
    cal_jshj = atof(red_fpxx->jshj) + atof(bule_fpxx->jshj);
    if ((cal_je != 0) || (cal_se != 0) || (cal_jshj != 0)) {
        sprintf(errifno, "红字发票与蓝字发票金额、税额、合计金额不匹配");
        return -4;
    }
    if ((strcmp(red_fpxx->skr, bule_fpxx->skr) != 0) ||
        (strcmp(red_fpxx->fhr, bule_fpxx->fhr) != 0) ||
        (strcmp(red_fpxx->kpr, bule_fpxx->kpr) != 0)) {
        sprintf(errifno, "红字发票与蓝字发票收款人、复核人、开票人信息不一致");
        return -5;
    }
    if ((strcmp(red_fpxx->zyspsmmc, bule_fpxx->zyspsmmc) != 0) ||
        (strcmp(red_fpxx->zyspmc, bule_fpxx->zyspmc) != 0)) {
        sprintf(errifno, "红字发票与蓝字发票主要商品税目、名称不一致");
        return -6;
    }
    if (red_fpxx->spsl != bule_fpxx->spsl) {
        sprintf(errifno, "红字发票与蓝字发票商品行数量不一致");
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
            sprintf(errifno, "红字发票与蓝字发票的第%d行商品的商品税目名称不一致", i + 1);
            return -8;
        }
        if ((strcmp(spxx_red->spmc, spxx_blue->spmc) != 0)) {
            sprintf(errifno, "红字发票与蓝字发票的第%d行商品的商品名称不一致", i + 1);
            return -8;
        }
        if (strcmp(spxx_red->spbh, spxx_blue->spbh) != 0) {
            sprintf(errifno, "红字发票与蓝字发票的第%d行商品的商品编码不一致", i + 1);
            return -9;
        }
        if (strcmp(spxx_red->fphxz, spxx_blue->fphxz) != 0) {
            sprintf(errifno, "红字发票与蓝字发票的第%d行商品的发票行性质不一致", i + 1);
            return -10;
        }

        char szDJ[64] = "";
        if (atoi(spxx_blue->hsjbz))
            PriceRemoveTax(spxx_blue->dj, spxx_blue->slv, 8, szDJ);
        else
            strcpy(szDJ, spxx_blue->dj);
        if ((strcmp(spxx_red->dj, szDJ) != 0) || (strcmp(spxx_red->slv, spxx_blue->slv) != 0)) {
            sprintf(errifno, "红字发票与蓝字发票的第%d行商品的单价、税率不一致", i + 1);
            return -11;
        }
        cal_spsl = atof(spxx_red->sl) + atof(spxx_blue->sl);
        cal_spje = atof(spxx_red->je) + atof(spxx_blue->je);
        cal_spse = atof(spxx_red->se) + atof(spxx_blue->se);
        cal_kce = atof(spxx_red->kce) + atof(spxx_blue->kce);
        if ((cal_spsl != 0) || (cal_spje != 0) || (cal_spse != 0) || (cal_kce != 0)) {
            sprintf(errifno,
                    "红字发票与蓝字发票的第%"
                    "d行商品的数量、金额、税额、差额征收扣除额不匹配",
                    i + 1);
            return -12;
        }
        if ((strcmp(spxx_red->ggxh, spxx_blue->ggxh) != 0) ||
            (strcmp(spxx_red->jldw, spxx_blue->jldw) != 0)) {
            sprintf(errifno, "红字发票与蓝字发票的第%d行商品的规格型号、计量单位不一致", i + 1);
            return -13;
        }
        if ((strcmp(spxx_red->xsyh, spxx_blue->xsyh) != 0) ||
            (strcmp(spxx_red->lslvbs, spxx_blue->lslvbs) != 0) ||
            (strcmp(spxx_red->yhsm, spxx_blue->yhsm) != 0)) {
            sprintf(errifno,
                    "红字发票与蓝字发票的第%"
                    "d行商品的优惠政策标识、零税率标识、增值税特殊管理不一致",
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
                            "发票综合税率中包含非法字符");
    if (JudgePrice(fpxx->je, sizeof(fpxx->je)) < 0)
        return SetLastError(fpxx->hDev->hUSB, DF_TAX_ERR_CODE_DATA_PARSING_FAILED,
                            "发票合计金额中包含非法字符");
    if (JudgePrice(fpxx->se, sizeof(fpxx->se)) < 0)
        return SetLastError(fpxx->hDev->hUSB, DF_TAX_ERR_CODE_DATA_PARSING_FAILED,
                            "发票合计税额包含非法字符");
    if (JudgePrice(fpxx->jshj, sizeof(fpxx->jshj)) < 0)
        return SetLastError(fpxx->hDev->hUSB, DF_TAX_ERR_CODE_DATA_PARSING_FAILED,
                            "发票价税合计包含非法字符");
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
    //上方为针对不同设备检测，下方为通用检测
    if ((fpxx->fplx == FPLX_COMMON_ZYFP) || (fpxx->fplx == FPLX_COMMON_DZZP)) {  //专票
        if (!fpxx->isRed) {                                                      //开蓝票
            if (!strlen(fpxx->gfsh) || !strlen(fpxx->gfdzdh) || !strlen(fpxx->xfdzdh)) {
                sprintf(errifno, "增值税专用发票需填写购方税号、购方地址电话、销方地址电话");
                _WriteLog(LL_WARN, "Zyfp gf info error");
                return -1;
            }
        } else {  //红票（开具及申请红字信息表）
            if (!strlen(fpxx->gfsh)) {
                sprintf(errifno, "需正确填入购买方税号");
                _WriteLog(LL_WARN, "%s", errifno);
                return -1;
            }
            if (fpxx->hzxxbsqsm != 1) {
                //仅购买申请已抵扣红字信息表和销售方开具购方已抵扣红字信息表时无蓝字发票代码号码
                if (!strlen(fpxx->blueFpdm) || !strlen(fpxx->blueFphm)) {
                    sprintf(errifno, "需正确填入蓝字发票代码号码");
                    _WriteLog(LL_WARN, "%s", errifno);
                    return -1;
                }
            }
            if (!fpxx->isHzxxb) {  //(非申请红字信息表)
                if (!strlen(fpxx->xfdzdh)) {
                    sprintf(errifno, "需正确填入销方地址电话");
                    _WriteLog(LL_WARN, "%s", errifno);
                    return -1;
                }
                if ((DEVICE_NISEC == bDeviceType) ||
                    (DEVICE_CNTAX ==
                     bDeviceType)) {  //百旺系税盘要求红票或蓝票开具均要填写购方地址电话
                    if (!strlen(fpxx->gfdzdh)) {
                        sprintf(errifno, "需正确填入购方地址电话");
                        _WriteLog(LL_WARN, "%s", errifno);
                        return -1;
                    }
                    /*if (fpxx->hzxxbsqsm != 0) {
                            sprintf(errifno,
                    "暂不支持税控盘税务ukey开具购买方申请的红字信息表发票"); _WriteLog(LL_WARN,
                    "%s", errifno); return -1;
                    }*/
                }
            }
        }

    } else {  //普票/电票
        // 非专用发票gf税号可为空,目前已测试aisino覆盖15个0，nisec置空，ukey尚未测试（但是15个0时也正常，暂且不处理）
        if (!strlen(fpxx->gfsh) && DEVICE_NISEC != bDeviceType)
            strcpy(fpxx->gfsh, "000000000000000");

        if ((DEVICE_NISEC == bDeviceType) ||
            (DEVICE_CNTAX == bDeviceType)) {  //百旺系税盘要求红票或蓝票开具均要填写销方地址电话
            if (!strlen(fpxx->xfdzdh)) {
                sprintf(errifno, "需正确填入销方地址电话");
                _WriteLog(LL_WARN, "%s", errifno);
                return -1;
            }
        }

        if (fpxx->isRed) {  //开红票票
            if (!strlen(fpxx->blueFpdm) || !strlen(fpxx->blueFphm)) {
                sprintf(errifno, "需正确填入蓝字发票代码号码");
                _WriteLog(LL_WARN, "%s", errifno);
                return -1;
            }
        }
    }

    if (!fpxx->isHzxxb) {  //(非申请红字信息表)
        if (!strlen(fpxx->kpr)) {
            sprintf(errifno, "需正确填入开票人名称");
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
        sprintf(errifno, "未知设备!");
        _WriteLog(LL_WARN, "Unknow device");
        return -1;
    }
    if (fpxx->fplx == FPLX_COMMON_DZFP) {
        if (fpxx->need_ofdurl == 1) {
            if (strlen(fpxx->hDev->szPubServer) == 0) {
                sprintf(errifno, "未开通电子发票公共服务平台,无法开具公共服务平台电票");
                _WriteLog(LL_WARN, "%s", errifno);
                return -2;
            }
        }
    }
    if ((fpxx->fplx == FPLX_COMMON_DZZP) && (DEVICE_CNTAX != hDev->bDeviceType)) {
        sprintf(errifno, "除税务ukey其他税盘暂不支持电子专票开具");
        return -1;
    }

    if (((DEVICE_NISEC == hDev->bDeviceType) || (DEVICE_CNTAX == hDev->bDeviceType)) &&
        !strcmp(fpxx->slv, "99.01")) {
        _WriteLog(LL_DEBUG, "Nisec multi slv, force set slv to 99.01");
    } else if (atof(fpxx->slv) > 0.17) {
        _WriteLog(LL_WARN, "Slv > 0.17");
        sprintf(errifno, "发票税率不得超过17%%");
        return -2;
    }
    struct Spxx *spxxchild = fpxx->stp_MxxxHead;
    while (spxxchild->stp_next) {
        spxxchild = spxxchild->stp_next;
        if (strlen(spxxchild->spmc) == 0) {

            if ((strcmp(spxxchild->spsmmc, "原价合计") == 0) ||
                (strcmp(spxxchild->spsmmc, "折扣额合计") == 0)) {
                ;
            } else {
                _WriteLog(LL_WARN, "Spmc must be filled");
                sprintf(errifno, "商品名称必现正确填写");
                return -3;
            }
        }
        if ((strcmp(spxxchild->spmc, "(详见对应正数发票清单)") != 0) &&
            (strcmp(spxxchild->spmc, "详见对应正数发票及清单") != 0)) {
            if (strlen(spxxchild->spsmmc) == 0) {


                _WriteLog(LL_WARN, "spsmmc must be filled");
                sprintf(errifno, "商品税目名称必现正确填写");
                return -4;
            }
            if (spxxchild->spsmmc[0] != '*' ||
                spxxchild->spsmmc[strlen(spxxchild->spsmmc) - 1] != '*') {

                if ((strcmp(spxxchild->spsmmc, "原价合计") == 0) ||
                    (strcmp(spxxchild->spsmmc, "折扣额合计") == 0)) {
                    ;
                } else {
                    _WriteLog(LL_WARN, "spsmmc must be format of '*XXX*'");
                    sprintf(errifno, "商品名称必现正确填写,*税目名称*");
                    return -4;
                }
            }
        }

        if (strlen(spxxchild->slv) == 0) {
            if ((strcmp(spxxchild->spsmmc, "原价合计") == 0) ||
                (strcmp(spxxchild->spsmmc, "折扣额合计") == 0) ||
                (strcmp(spxxchild->spmc, "详见对应正数发票及清单") == 0)) {
                ;
            } else {
                sprintf(errifno, "发票行信息中需填入税率");
                _WriteLog(LL_WARN, "Slv must be filled");
                return -7;
            }
        }
        if (strlen(spxxchild->se) == 0) {
            sprintf(errifno, "发票行信息中需填入税额");
            _WriteLog(LL_WARN, "Se must be filled");
            return -8;
        }
        if (strlen(spxxchild->je) == 0) {
            sprintf(errifno, "发票行信息中需填入金额");
            _WriteLog(LL_WARN, "Je must be filled");
            return -9;
        }
        //红冲发票校验
        if (fpxx->isRed) {
            if (atoi(spxxchild->sl) > 0) {
                sprintf(errifno, "红冲发票数量应该为负数");
                _WriteLog(LL_WARN, "Red invoice Sl must be <0");
                return -6;
            }
            if (atof(spxxchild->se) > 0) {
                sprintf(errifno, "红冲发票税额应该为负数");
                _WriteLog(LL_WARN, "Red invoice SE must be <0");
                return -8;
            }
            if (atof(spxxchild->je) > 0) {
                sprintf(errifno, "红冲发票金额应该为负数");
                _WriteLog(LL_WARN, "Red invoice JE must be <0");
                return -9;
            }
        }
        // if (spxxchild->stp_next == NULL)
        //     break;
    }
    if (!strlen(fpxx->xfsh) || !strlen(fpxx->gfmc) || !strlen(fpxx->xfmc)) {
        sprintf(errifno, "未正确填写销购方信息");
        _WriteLog(LL_WARN, "Need mc and sh");
        return -10;
    }

    //如果有购方税号才判定
    if (strlen(fpxx->gfsh) && ((strlen(fpxx->gfsh) < 15) || strlen(fpxx->gfsh) > 20)) {
        sprintf(errifno, "购方信息税号长度错误");
        _WriteLog(LL_WARN, "TaxID's gfsh length must be [15,20]");
        return -10;
    }

    if ((strlen(fpxx->xfsh) != 15) && (strlen(fpxx->xfsh) != 17) && (strlen(fpxx->xfsh) != 18) &&
        (strlen(fpxx->xfsh) != 20)) {
        sprintf(errifno, "销方信息税号长度错误");
        _WriteLog(LL_WARN, "TaxID's xfsh length must be [15,20]");
        return -10;
    }
    //购方税号 ！= 销方税号
    // if (strcmp((const char *)fpxx->gfsh, (const char *)hDev->szCommonTaxID) == 0) {
    //    _WriteLog(LL_DEBUG, "gfsh = %s xfsh=%s", fpxx->gfsh, hDev->szCommonTaxID);
    //    sprintf(errifno, "购方信息不得等于销方信息");
    //    return -10;
    //}
    // if (!strlen(fpxx->kpr) || strlen(fpxx->kpr) > 15) {
    //    sprintf(errifno, "开票人长度错误,5个汉字");
    //    _WriteLog(LL_WARN, "kpr incorrect, 5word");  // kpr 5 hanzi max
    //    return -11;
    //}
    if (fpxx->isHzxxb == true && fpxx->hzxxbsqsm != 0) {
        _WriteLog(LL_WARN, "红字信息表申请时购买方申请无需校验红字销方信息是否匹配");
    } else {
        if (strcmp(fpxx->xfsh, hDev->szCommonTaxID) || strcmp(fpxx->xfmc, hDev->szCompanyName)) {
            sprintf(errifno, "销方信息与设备信息不匹配");
            _WriteLog(LL_WARN, "Import XFSH or XFTaxID not match the device's config");
            return -12;
        }
    }
    if (!strlen(fpxx->zyspmc)) {
        sprintf(errifno, "主要商品名称不得为空");
        _WriteLog(LL_WARN, "Zyspmc == null");
        return -13;
    }
    if (strlen(fpxx->bz) > 230 || strlen(fpxx->gfdzdh) > 100 || strlen(fpxx->gfyhzh) > 100) {
        sprintf(errifno,
                "备注信息或购方地址电话、银行账号数据长度过长,底层规定：备注230,"
                "购方地址100,购方银行100,实际长度：备注%d,购方地址%d,购方银行%d",
                (int)strlen(fpxx->bz), (int)strlen(fpxx->gfdzdh), (int)strlen(fpxx->gfyhzh));
        _WriteLog(LL_WARN, "bz gfdzdh gfyhzh length error");
        return -14;
    }
    //红冲发票校验
    double nC1, nC2, nC3, nC4;
    nC1 = atof(fpxx->je);
    nC2 = atof(fpxx->se);
    nC3 = atof(fpxx->jshj);
    nC4 = atof(fpxx->slv);
    if (fpxx->isRed) {
        if (nC1 >= 0 || nC2 > 0 || nC3 >= 0 || nC4 < 0) {
            sprintf(errifno, "红字发票金额税额价税合计应小于等于零,税率应大于零");
            _WriteLog(LL_WARN, "red invoice je se slv check failed");
            return -15;
        }
    } else {
        if (nC1 < 0 || nC2 < 0 || nC3 < 0 || nC4 < 0) {
            sprintf(errifno, "蓝字发票金额税额价税合计应大于等于零");
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
//-----------------------------------------↑↑↑↑↑↑导入依赖函数↑↑↑↑↑↑------------------------------------------------------
//-----------------------------------------------华丽的分割线------------------------------------------------------------
//-----------------------------------------↓↓↓↓导入分析监测过程↓↓↓↓↓------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------------


//这4个check函数只作检测，不对fpxx内容做改动

//！！！注意
//该函数内部不要动单价，因为外面已经开始计算了
//目前先把框架定下来，后期剩余代码再慢慢适配进来
int CheckNisecSpxx(HFPXX stp_Root, struct Spxx *stp_InsertNode, char *errinfo)
{
    if (stp_Root || stp_InsertNode || errinfo) {
    }
    return 0;
}

//！！！注意
//该函数内部不要动单价，因为外面已经开始计算了
//目前先把框架定下来，后期剩余代码再慢慢适配进来
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
    //    sprintf(errinfo, "税控盘收款人、复核人、开票人最大长度不得超过16字节");
    //    goto Err;
    //}

    // if (strlen(stp_fpxx->zyspsmmc) + strlen(stp_fpxx->zyspmc) > 100) {
    //    sprintf(errinfo, "税控盘主要商品税目+主要商品名称长度超过100字节错误");
    //    goto Err;
    //}
    // if (stp_fpxx->fplx_aisino == FPLX_AISINO_ZYFP) {
    //    if (strlen(stp_fpxx->bz) > 184) {
    //        sprintf(errinfo, "税控盘专用发票备注不得大于184字节");
    //        goto Err;
    //    }
    //} else if (stp_fpxx->fplx_aisino == FPLX_AISINO_PTFP) {
    //    if (strlen(stp_fpxx->bz) > 138) {
    //        sprintf(errinfo, "税控盘普通发票备注不得大于138字节");
    //
    //        goto Err;
    //    }
    //} else if (stp_fpxx->fplx_aisino == FPLX_AISINO_DZFP) {
    //    if (strlen(stp_fpxx->bz) > 130) {
    //        sprintf(errinfo, "税控盘电子发票备注不得大于130字节");
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
    //    sprintf(errinfo, "税控盘收款人、复核人、开票人最大长度不得超过16字节");
    //    goto Err;
    //}

    // if (strlen(stp_fpxx->zyspsmmc) + strlen(stp_fpxx->zyspmc) > 100) {
    //    sprintf(errinfo, "税控盘主要商品税目+主要商品名称长度超过100字节错误");
    //    goto Err;
    //}
    // if (stp_fpxx->fplx_aisino == FPLX_AISINO_ZYFP) {
    //    if (strlen(stp_fpxx->bz) > 184) {
    //        sprintf(errinfo, "税控盘专用发票备注不得大于184字节");
    //        goto Err;
    //    }
    //} else if (stp_fpxx->fplx_aisino == FPLX_AISINO_PTFP) {
    //    if (strlen(stp_fpxx->bz) > 138) {
    //        sprintf(errinfo, "税控盘普通发票备注不得大于138字节");
    //
    //        goto Err;
    //    }
    //} else if (stp_fpxx->fplx_aisino == FPLX_AISINO_DZFP) {
    //    if (strlen(stp_fpxx->bz) > 130) {
    //        sprintf(errinfo, "税控盘电子发票备注不得大于130字节");
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
    //    sprintf(errinfo, "金税盘主要商品税目+主要商品名称长度超过92字节错误");
    //    goto Err;
    //}
    // if (strlen(stp_fpxx->bz) > 230) {
    //    sprintf(errinfo, "金税盘发票备注不得大于230字节");
    //    goto Err;
    //}
    nRet = 0;
    // Err:
    return nRet;
}

//解析发票json数据
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
        return -1;  //参数错误
    root = cJSON_Parse((const char *)json_data);
    if (!root) {
        _WriteLog(LL_FATAL, "JSON Parse json_data failed\n");
        sprintf(errinfo, "发票JSON数据格式存在问题");
        i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        goto Err;
    }
    //发票类型
    memset(szBuf, 0, sizeof szBuf);
    if (GetAvailImportData(root, "FPZLS", szBuf, 3, true, json_err) < 0) {
        _WriteLog(LL_FATAL, "JSON Parse FPZLS failed\n");
        sprintf(errinfo, "发票JSON数据中发票种类(fpzls)解析出错,%s", json_err);
        i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        goto Err;
    }
    if ((strcmp(szBuf, "004") != 0) && (strcmp(szBuf, "007") != 0) && (strcmp(szBuf, "026") != 0) &&
        (strcmp(szBuf, "028") != 0)) {
        if (!strcmp(szBuf, "028") && bDeviceType != DEVICE_CNTAX) {
            _WriteLog(LL_FATAL, "028 Just support cntax");
            sprintf(errinfo, "电子专票目前仅支持Ukey");
            i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
            goto Err;
        }
        _WriteLog(LL_FATAL, "JSON Parse FPZLS did not 004 007 026 028 failed\n");
        sprintf(errinfo,
                "发票种类目前仅支持专票、普票、电普、电专（004 007 026 028）四种类型发票开具");
        i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        goto Err;
    }
    stp_fpxx->fplx = atoi(szBuf);
    //考虑兼容，同时填入aisino发票种类信息
    //另：由于研究最早以aisino为切入点，代码已经成型，因此该函数后面所有的发票判断都是以aisino发票类型为主要判断依据，不要再加入通用fplx的判断
    CommonFPLX2AisinoFPLX(stp_fpxx->fplx, &stp_fpxx->fplx_aisino);
    //发票请求流水号
    if (GetAvailImportData(root, "FPQQLSH", stp_fpxx->fpqqlsh, 120, false, json_err) < 0) {
        _WriteLog(LL_FATAL, "JSON Parse FPQQLSH failed\n");
        sprintf(errinfo, "发票JSON数据中发票请求流水号(fpqqlsh)解析出错,%s", json_err);
        i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        goto Err;
    }
    memset(szBuf, 0, sizeof szBuf);
    if (GetAvailImportData(root, "ofdurl", szBuf, 3, false, json_err) < 0) {
        stp_fpxx->need_ofdurl = 0;
    } else {
        stp_fpxx->need_ofdurl = atoi(szBuf);
    }

    //开票软件版本号
    if (GetAvailImportData(root, "KPRJBBH", stp_fpxx->kprjbbh, 31, false, json_err) < 0) {
        _WriteLog(LL_FATAL, "JSON Parse KPRJBBH failed\n");
        sprintf(errinfo, "发票JSON数据中开票软件版本号(kprjbbh)解析出错,%s", json_err);
        i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        goto Err;
    }
    //税盘编号 jqbh
    if (GetAvailImportData(root, "SPBH", stp_fpxx->jqbh, 12, true, json_err) < 0) {
        _WriteLog(LL_FATAL, "JSON Parse jqbh-SPBH\n");
        sprintf(errinfo, "发票JSON数据中税盘编号(spbh)解析出错,%s", json_err);
        i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        goto Err;
    }
    //开票类型(正数、负数)
    memset(szBuf, 0, sizeof szBuf);
    if (GetAvailImportData(root, "KPLX", szBuf, 3, true, json_err) < 0) {
        _WriteLog(LL_FATAL, "JSON Parse KPLX failed\n");
        sprintf(errinfo, "发票JSON数据中开票类型(kplx)解析出错,%s", json_err);
        i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        goto Err;
    }
    if (strcasecmp(szBuf, "0") && strcasecmp(szBuf, "1")) {
        _WriteLog(LL_FATAL, "KPLX 0/1, blue or red\n");
        sprintf(errinfo, "开票类型错误，非蓝字发票、非红字发票");
        i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        goto Err;
    }
    strcpy(stp_fpxx->kplx, szBuf);  //兼容代码
    stp_fpxx->isRed = atoi(szBuf);
    //编码表版本号
    if (GetAvailImportData(root, "BMBBBH", stp_fpxx->bmbbbh, 8, true, json_err) < 0) {
        _WriteLog(LL_FATAL, "JSON Parse BMBBBH failed\n");
        sprintf(errinfo, "发票JSON数据中编码表版本号(bmbbbh)解析出错,%s", json_err);
        i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        goto Err;
    }
    //综合税率导入接口,不能修改只能保持zhsl,内部值改为slv
    if (GetAvailImportData(root, "ZHSL", stp_fpxx->slv, 10, false, json_err) < 0) {
        _WriteLog(LL_FATAL, "JSON Parse SLV failed\n");
        sprintf(errinfo, "发票JSON数据中综合税率(zhsl)解析出错,%s", json_err);
        i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        goto Err;
    }
    //红字信息表申请说明
    GetAvailImportData(root, "HZXXBSQSM", szBuf, 3, false, json_err);
    stp_fpxx->hzxxbsqsm = atoi(szBuf);

    if ((stp_fpxx->hzxxbsqsm != 0) && (stp_fpxx->isHzxxb == true)) {
        if (stp_fpxx->isRed != 1) {
            sprintf(errinfo, "仅红字信息表申请或红冲开具时填入红字信息表申请说明");
            i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
            goto Err;
        }
        if ((stp_fpxx->fplx != FPLX_COMMON_ZYFP) && (stp_fpxx->fplx != FPLX_COMMON_DZZP)) {
            sprintf(errinfo, "仅纸质专票或电子专票填入红字信息表申请说明");
            i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
            goto Err;
        }
    }
    //购方名称
    if (GetAvailImportData(root, "GFMC", stp_fpxx->gfmc, 100, true, json_err) < 0) {
        _WriteLog(LL_FATAL, "JSON Parse GFMC failed\n");
        sprintf(errinfo, "发票JSON数据中购方名称(gfmc)解析出错,%s", json_err);
        i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        goto Err;
    }
    if ((stp_fpxx->fplx == FPLX_COMMON_ZYFP) || (stp_fpxx->fplx == FPLX_COMMON_DZZP)) {
        //购方税号
        if (GetAvailImportData(root, "GFSH", stp_fpxx->gfsh, 20, true, json_err) < 0) {
            _WriteLog(LL_FATAL, "JSON Parse GFSH failed\n");
            sprintf(errinfo, "发票JSON数据中购方税号(gfsh)解析出错,%s", json_err);
            i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
            goto Err;
        }
        if ((strlen(stp_fpxx->gfsh) != 15) && (strlen(stp_fpxx->gfsh) != 17) &&
            (strlen(stp_fpxx->gfsh) != 18) && (strlen(stp_fpxx->gfsh) != 20)) {
            _WriteLog(LL_FATAL, "JSON Parse GFSH len error failed\n");
            sprintf(errinfo, "发票JSON数据中购方税号非15、17、18、20位");
            i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
            goto Err;
        }
        if (JudgeTaxID(stp_fpxx->gfsh) < 0) {
            _WriteLog(LL_FATAL, "JSON Parse GFSH len error failed\n");
            sprintf(errinfo, "发票JSON数据中购方税号规则存在错误,"
                             "检测到存在除大写字母及数字外的其他字符");
            i_ErrNum = DF_TAX_ERR_CODE_CAN_NOT_USE_IOZSV;
            goto Err;
        }
    } else if ((stp_fpxx->fplx == FPLX_COMMON_PTFP) ||
               (stp_fpxx->fplx ==
                FPLX_COMMON_DZFP))  //普票及电票购方税号、地址电话、银行账号可为空（暂不支持税号空）
    {
        //购方税号
        if (GetAvailImportData(root, "GFSH", stp_fpxx->gfsh, 20, false, json_err) < 0) {
            _WriteLog(LL_FATAL, "JSON Parse GFSH failed\n");
            sprintf(errinfo, "发票JSON数据中购方税号(gfsh)解析出错,%s", json_err);
            i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
            goto Err;
        }
        if (strlen(stp_fpxx->gfsh) != 0) {
            // aisino的支持非标准税号开具(个人名称+手机号码)，其他税盘暂不支持
            // if (DEVICE_AISINO != bDeviceType && (strlen(stp_fpxx->gfsh) != 15) &&
            //    (strlen(stp_fpxx->gfsh) != 17) && (strlen(stp_fpxx->gfsh) != 18) &&
            //    (strlen(stp_fpxx->gfsh) != 20)) {
            //    _WriteLog(LL_FATAL, "JSON Parse GFSH len error failed\n");
            //    sprintf(errinfo, "发票JSON数据中购方税号非15、17、18、20位");
            //    i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
            //    goto Err;
            //}
            if (JudgeTaxID(stp_fpxx->gfsh) < 0) {
                _WriteLog(LL_FATAL, "JSON Parse GFSH len error failed\n");
                sprintf(errinfo, "发票JSON数据中购方税号规则存在错误,"
                                 "检测到存在除大写字母及数字外的其他字符");
                i_ErrNum = DF_TAX_ERR_CODE_CAN_NOT_USE_IOZSV;
                goto Err;
            }
        }
    }

    //购方地址电话
    if (GetAvailImportData(root, "GFDZDH", stp_fpxx->gfdzdh, 100, false, json_err) < 0) {
        _WriteLog(LL_FATAL, "JSON Parse GFDZDH failed\n");
        sprintf(errinfo, "发票JSON数据中购方地址电话(gfdzdh)解析出错,%s", json_err);
        i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        goto Err;
    }
    //购方银行账号
    if (GetAvailImportData(root, "GFYHZH", stp_fpxx->gfyhzh, 100, false, json_err) < 0) {
        _WriteLog(LL_FATAL, "JSON Parse GFYHZH failed\n");
        sprintf(errinfo, "发票JSON数据中购方银行账号(gfyhzh)解析出错,%s", json_err);
        i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        goto Err;
    }
    //销方名称
    if (GetAvailImportData(root, "XFMC", stp_fpxx->xfmc, 100, true, json_err) < 0) {
        _WriteLog(LL_FATAL, "JSON Parse XFMC failed\n");
        sprintf(errinfo, "发票JSON数据中销方名称(xfmc)解析出错,%s", json_err);
        i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        goto Err;
    }
    if (strlen(stp_fpxx->xfmc) < 1) {
        sprintf(errinfo, "发票JSON数据中销方名称长度错误");
        i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        goto Err;
    }
    //销方税号
    if (GetAvailImportData(root, "XFSH", stp_fpxx->xfsh, 20, true, json_err) < 0) {
        _WriteLog(LL_FATAL, "JSON Parse XFSH failed\n");
        sprintf(errinfo, "发票JSON数据中销方税号(xfsh)解析出错,%s", json_err);
        i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        goto Err;
    }
    //销方地址电话
    if (GetAvailImportData(root, "XFDZDH", stp_fpxx->xfdzdh, 100, false, json_err) < 0) {
        _WriteLog(LL_FATAL, "JSON Parse XFDZDH failed\n");
        sprintf(errinfo, "发票JSON数据中销方地址电话(xfdzdh)解析出错,%s", json_err);
        i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        goto Err;
    }
    //销方银行账号
    if (GetAvailImportData(root, "XFYHZH", stp_fpxx->xfyhzh, 100, false, json_err) < 0) {
        _WriteLog(LL_FATAL, "JSON Parse XFYHZH failed\n");
        sprintf(errinfo, "发票JSON数据中销方银行账号(xfyhzh)解析出错,%s", json_err);
        i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        goto Err;
    }
    //营业税标识，先全置0
    strcpy(stp_fpxx->yysbz, "00000000000000000000");
    //含税价标志 0/1/2
    if (GetAvailImportData(root, "HSJBZ", stp_fpxx->hsjbz, 3, true, json_err) < 0) {
        _WriteLog(LL_FATAL, "JSON Parse HSJBZ failed\n");
        sprintf(errinfo, "发票JSON数据中含税价标志(hsjbz)解析出错,%s", json_err);
        i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        goto Err;
    }
    if (!strlen(stp_fpxx->hsjbz))
        strcpy(stp_fpxx->hsjbz, "0");
    i_hsjbz = atoi(stp_fpxx->hsjbz);
    if ((i_hsjbz != 0) && (i_hsjbz != 1)) {
        sprintf(errinfo, "目前仅支持商品全部含税或不含税开票");
        _WriteLog(LL_FATAL, "JSON Parse HSJBZ 目前仅支持商品全部含税或不含税开票\n");
        i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        goto Err;
    }
    //合计金额
    if (GetAvailImportData(root, "HJJE", stp_fpxx->je, 20, true, json_err) < 0) {
        _WriteLog(LL_FATAL, "JSON Parse HJJE failed\n");
        sprintf(errinfo, "发票JSON数据中合计金额(hjje)解析出错,%s", json_err);
        i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        goto Err;
    }
    //合计税额
    if (GetAvailImportData(root, "HJSE", stp_fpxx->se, 20, true, json_err) < 0) {
        _WriteLog(LL_FATAL, "JSON Parse HJSE failed\n");
        sprintf(errinfo, "发票JSON数据中合计税额(hjse)解析出错,%s", json_err);
        i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        goto Err;
    }
    //价税合计
    if (GetAvailImportData(root, "JSHJ", stp_fpxx->jshj, 20, true, json_err) < 0) {
        _WriteLog(LL_FATAL, "JSON Parse JSHJ failed\n");
        sprintf(errinfo, "发票JSON数据中价税合计(jshj)解析出错,%s", json_err);
        i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        goto Err;
    }

    if (check_fpxx_info(stp_fpxx, errinfo) < 0) {
        i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        goto Err;
    }

    //清单标志
    if (GetAvailImportData(root, "QDBZ", stp_fpxx->qdbj, 3, true, json_err) < 0) {
        _WriteLog(LL_FATAL, "JSON Parse QDBZ failed\n");
        sprintf(errinfo, "发票JSON数据中清单标志(qdbz)解析出错,%s", json_err);
        i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        goto Err;
    }

    if (GetAvailImportData(root, "SYFPDMFPHM", szBuf, 3, false, json_err) < 0) {
        use_fpdmhm = 0;
    }
    if (strcmp(szBuf, "1") == 0)
        use_fpdmhm = 1;
    if (use_fpdmhm == 1) {
        //指定发票卷首张开票,而非自动选
        if (GetAvailImportData(root, "DKFPDM", stp_fpxx->fpdm, 12, true, json_err) < 0) {
            _WriteLog(LL_FATAL, "JSON Parse DKFPDM failed\n");
            sprintf(errinfo, "发票JSON数据中(dkfpdm)解析出错,%s", json_err);
            i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
            goto Err;
        }
        if (GetAvailImportData(root, "DKFPHM", stp_fpxx->fphm, 12, true, json_err) < 0) {
            _WriteLog(LL_FATAL, "JSON Parse DKFPHM failed\n");
            sprintf(errinfo, "发票JSON数据中(dkfphm)解析出错,%s", json_err);
            i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
            goto Err;
        }

        if ((strlen(stp_fpxx->fpdm) != 10) && (strlen(stp_fpxx->fpdm) != 12)) {
            sprintf(errinfo, "填入的代开发票代码长度有误");
            i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
            goto Err;
        }
        if (strlen(stp_fpxx->fphm) != 8) {
            sprintf(errinfo, "填入的代开发票号码长度有误");
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
        sprintf(errinfo, "专用发票类型暂只支持一般专普票和差额征收模式");
        i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        goto Err;
    }
    if (stp_fpxx->isRed) {
        //原发票代码
        if (GetAvailImportData(root, "YFPDM", stp_fpxx->blueFpdm, 12, false, json_err) < 0) {
            _WriteLog(LL_FATAL, "JSON Parse YFPDM failed\n");
            sprintf(errinfo, "发票JSON数据中红字发票原发票代码(yfpdm)解析出错,%s", json_err);
            i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
            goto Err;
        }
        //原发票号码
        if (GetAvailImportData(root, "YFPHM", stp_fpxx->blueFphm, 12, false, json_err) < 0) {
            _WriteLog(LL_FATAL, "JSON Parse YFPHM failed\n");
            sprintf(errinfo, "发票JSON数据中红字发票原发票号码(yfphm)解析出错,%s", json_err);
            i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
            goto Err;
        }
        //原发票开具时间
        GetAvailImportData(root, "YFPKJSJ", stp_fpxx->blueKpsj, 19, false, json_err);
        if (strlen(stp_fpxx->blueKpsj) > 0) {
            if (strlen(stp_fpxx->blueKpsj) != 14) {
                sprintf(errinfo, "发票JSON数据中红字发票原发票开票时间长度有误");
                i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
                goto Err;
            }
            int sj_count;
            for (sj_count = 0; sj_count < strlen(stp_fpxx->blueKpsj); sj_count++) {
                if ((stp_fpxx->blueKpsj[sj_count] < '0') || (stp_fpxx->blueKpsj[sj_count] > '9')) {
                    sprintf(errinfo, "发票JSON数据中红字发票原发票开票时间格式有误");
                    return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
                }
            }
        }

        //原发票种类
        GetAvailImportData(root, "YFPZLS", szBuf, 4, false, json_err);
        if ((strlen(szBuf) > 0)) {
            stp_fpxx->blueFplx = atoi(szBuf);
            if ((strcmp(szBuf, "004") != 0) && (strcmp(szBuf, "007") != 0) &&
                (strcmp(szBuf, "026") != 0) && (strcmp(szBuf, "028") != 0)) {
                if (!strcmp(szBuf, "028") && bDeviceType != DEVICE_CNTAX) {
                    _WriteLog(LL_FATAL, "028 Just support cntax");
                    sprintf(errinfo, "电子专票目前仅支持Ukey");
                    i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
                    goto Err;
                }
                _WriteLog(LL_FATAL, "JSON Parse FPZLS did not 004 007 026 028 failed\n");
                sprintf(
                    errinfo,
                    "发票种类目前仅支持专票、普票、电普、电专（004 007 026 028）四种类型发票开具");
                i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
                goto Err;
            }

            if ((stp_fpxx->fplx == FPLX_COMMON_ZYFP) && (stp_fpxx->blueFplx != FPLX_COMMON_ZYFP)) {
                sprintf(errinfo, "专票不支持跨票种红冲");
                i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
                goto Err;
            }
            if ((stp_fpxx->fplx == FPLX_COMMON_DZZP) && (stp_fpxx->blueFplx != FPLX_COMMON_DZZP)) {
                sprintf(errinfo, "电专票不支持跨票种红冲");
                i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
                goto Err;
            }
            if ((stp_fpxx->fplx == FPLX_COMMON_PTFP) &&
                ((stp_fpxx->blueFplx != FPLX_COMMON_PTFP) &&
                 (stp_fpxx->blueFplx != FPLX_COMMON_DZFP) &&
                 (stp_fpxx->blueFplx != FPLX_COMMON_JSFP))) {
                sprintf(errinfo, "普票只可以对普票、电普、卷票红冲");
                i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
                goto Err;
            }
            if ((stp_fpxx->fplx == FPLX_COMMON_DZFP) &&
                ((stp_fpxx->blueFplx != FPLX_COMMON_PTFP) &&
                 (stp_fpxx->blueFplx != FPLX_COMMON_DZFP) &&
                 (stp_fpxx->blueFplx != FPLX_COMMON_JSFP))) {
                sprintf(errinfo, "电普只可以对普票、电普、卷票红冲");
                i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
                goto Err;
            }

        } else
            stp_fpxx->blueFplx = stp_fpxx->fplx;


        //红冲原因
        GetAvailImportData(root, "HCYY", szBuf, 3, false, json_err);

        if (strlen(szBuf) > 0) {
            stp_fpxx->hcyy = atoi(szBuf);
            if ((stp_fpxx->hcyy != 1) && (stp_fpxx->hcyy != 2) && (stp_fpxx->hcyy != 3) &&
                (stp_fpxx->hcyy != 4)) {
                sprintf(errinfo, "红冲原因填入的数值有误\n");
                i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
                goto Err;
            }
        } else
            stp_fpxx->hcyy = 2;
        _WriteLog(LL_INFO,
                  "开具红字发票,蓝字发票代码:%s 号码:%s 开具时间:%s 原蓝票类型：%03d 红冲原因：%d",
                  stp_fpxx->blueFpdm, stp_fpxx->blueFphm, stp_fpxx->blueKpsj, stp_fpxx->blueFplx,
                  stp_fpxx->hcyy);
    }

    //特殊税率填开理由
    GetAvailImportData(root, "SSLKJLY", szBuf, 3, false, json_err);
    if (strlen(szBuf) > 0) {
        stp_fpxx->sslkjly = atoi(szBuf);
    }

    //地址索引号
    GetAvailImportData(root, "DZSYH", szBuf, 24, false, json_err);
    stp_fpxx->dzsyh = atoi(szBuf);

    //允许误差值
    GetAvailImportData(root, "YXWCZ", szBuf, 3, false, json_err);
    stp_fpxx->allow_error = atoi(szBuf);

    if ((stp_fpxx->allow_error > 6) || (stp_fpxx->allow_error < 1)) {
        stp_fpxx->allow_error = 1;
    }
    //收款人
    if (GetAvailImportData(root, "SKR", stp_fpxx->skr, 20, false, json_err) < 0) {
        _WriteLog(LL_FATAL, "JSON Parse SKR failed\n");
        sprintf(errinfo, "发票JSON数据中收款人(skr)解析出错,%s", json_err);
        i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        goto Err;
    }
    //复核人
    if (GetAvailImportData(root, "FHR", stp_fpxx->fhr, 20, false, json_err) < 0) {
        _WriteLog(LL_FATAL, "JSON Parse FHR failed\n");
        sprintf(errinfo, "发票JSON数据中复核人(fhr)解析出错,%s", json_err);
        i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        goto Err;
    }

    if (strstr(stp_fpxx->fhr, defAisino3SplitFlag)) {
        _WriteLog(LL_FATAL, "JSON Parse FHR failed\n");
        sprintf(errinfo, "发票JSON数据中复核人(fhr)信息有误存在非法字符");
        i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        goto Err;
    }


    //开票人
    if (GetAvailImportData(root, "KPR", stp_fpxx->kpr, 20, false, json_err) < 0) {
        _WriteLog(LL_FATAL, "JSON Parse KPR failed\n");
        sprintf(errinfo, "发票JSON数据中开票人(kpr)解析出错,%s", json_err);
        i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        goto Err;
    }

    if (CheckFpxxMandatoryField(stp_fpxx, bDeviceType, errinfo) < 0) {
        i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        goto Err;
    }

    //主要商品名称
    if (GetAvailImportData(root, "ZYSPMC", stp_fpxx->zyspmc, 100, true, json_err) < 0) {
        _WriteLog(LL_FATAL, "JSON Parse ZYSPMC failed\n");
        sprintf(errinfo, "发票JSON数据中主要商品名称(zyspmc)解析出错,%s", json_err);
        i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        goto Err;
    }
    //商品税目
    if (GetAvailImportData(root, "ZYSPSMMC", stp_fpxx->zyspsmmc, 100, true, json_err) < 0) {
        _WriteLog(LL_FATAL, "JSON Parse ZYSPSMMC failed\n");
        sprintf(errinfo, "发票JSON数据中主要商品税目名称(zyspsmmc)解析出错,%s", json_err);
        i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        goto Err;
    }

    if (bDeviceType == DEVICE_AISINO) {
        if (strlen(stp_fpxx->zyspsmmc) + strlen(stp_fpxx->zyspmc) > 92) {
            sprintf(errinfo, "金税盘主要商品税目+主要商品名称长度超过92字节错误");
            _WriteLog(LL_FATAL, "%s\n", errinfo);
            i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
            goto Err;
        }
    } else if ((bDeviceType == DEVICE_NISEC) || (bDeviceType == DEVICE_CNTAX)) {
        if ((strlen(stp_fpxx->skr) > 16) || (strlen(stp_fpxx->fhr) > 16) ||
            (strlen(stp_fpxx->kpr) > 16)) {
            sprintf(errinfo, "税控盘收款人、复核人、开票人最大长度不得超过16字节");
            i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
            goto Err;
        }

        if (strlen(stp_fpxx->zyspsmmc) + strlen(stp_fpxx->zyspmc) > 100) {
            sprintf(errinfo, "税控盘主要商品税目+主要商品名称长度超过100字节错误");
            _WriteLog(LL_FATAL, "%s\n", errinfo);
            i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
            goto Err;
        }
    }

    //所属月份,此校验主要用于红字信息表接口
    if (GetAvailImportData(root, "SSYF", stp_fpxx->ssyf, 10, true, json_err) < 0) {
        _WriteLog(LL_FATAL, "JSON Parse SSYF failed\n");
        sprintf(errinfo, "发票JSON数据中所属月份(ssyf)解析出错,%s", json_err);
        i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        goto Err;
    }
    if (strlen(stp_fpxx->ssyf) != 6) {
        sprintf(errinfo, "发票JSON数据中所属月份格式错误");
        _WriteLog(LL_FATAL, "JSON Parse SSYF len error did not YYYYMM\n");
    }
    //通知单编号,example:1000123410666662,1000123410666673
    //通知单编号,放到后面代码去校验
    if (GetAvailImportData(root, "TZDBH", stp_fpxx->redNum, 20, false, json_err) < 0) {
        _WriteLog(LL_FATAL, "JSON Parse TZDBH failed\n");
        sprintf(errinfo, "发票JSON数据中通知单编号(tzdbh)解析出错,%s", json_err);
        i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        goto Err;
    }
#ifndef _NODEVICE_TEST
    //直接强制更新开票软件版本号及编码表版本号
    memset(stp_fpxx->kprjbbh, 0, sizeof(stp_fpxx->kprjbbh));
    memset(stp_fpxx->bmbbbh, 0, sizeof(stp_fpxx->bmbbbh));
    strcpy(stp_fpxx->bmbbbh, DEF_BMBBBH_DEFAULT);
    // strcpy(stp_fpxx->kprjbbh, AISINO_INNER_VERSION_DEFAULT);

#endif
    //商品明细
    jsp_ArraySpxx = cJSON_GetObjectItem(root, "FPMX");
    if (!jsp_ArraySpxx) {
        _WriteLog(LL_FATAL, "JSON Parse FPMX failed\n");
        i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        goto Err;
    }
    int i_SpxxNum = cJSON_GetArraySize(jsp_ArraySpxx);
    if (i_SpxxNum > 2000) {
        sprintf(errinfo, "商品条数不得超过2000行");
        _WriteLog(LL_FATAL, "JSON Parse jsp_ArraySpxx failed\n");
        i_ErrNum = DF_TAX_ERR_CODE_INV_DATA_TOO_LARGE;  //航信不超过2000商品
        goto Err;
    }
	
	if ((DEVICE_AISINO != bDeviceType)&&((stp_fpxx->fplx == FPLX_COMMON_DZFP) || (stp_fpxx->fplx == FPLX_COMMON_DZZP)) &&
		(i_SpxxNum > 100)) {
		sprintf(errinfo, "电子发票商品条数不得超过100行");
		_WriteLog(LL_FATAL, "JSON Parse jsp_ArraySpxx failed\n");
		i_ErrNum = DF_TAX_ERR_CODE_INV_DATA_TOO_LARGE;  //非航信电票不超过100商品
		goto Err;
	}
	

    if (strcmp(stp_fpxx->gfsh, "000000123456789") ==
        0)  //金税盘不允许开具此特殊税号,可能与发票蓝冲有关
    {
        sprintf(errinfo, "特殊税号不允许开具");
        i_ErrNum = DF_TAX_ERR_CODE_INV_DATA_TOO_LARGE;  //航信电票不超过100商品
        goto Err;
    }

    //分设备检测纠正，下面的代码执行中对结构体内的修改，就管不了
    //后期需要对下面的代码逐渐融入该框架内部
    if (DEVICE_AISINO == bDeviceType) {
        if (CheckAisinoFpxx(stp_fpxx, errinfo) < 0) {
            i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
            // sprintf(errinfo, "分设备检测不通过");
            goto Err;
        }
    } else if (DEVICE_NISEC == bDeviceType) {
        if (CheckNisecFpxx(stp_fpxx, errinfo) < 0) {
            i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
            // sprintf(errinfo, "分设备检测不通过");
            goto Err;
        }
    } else if (DEVICE_CNTAX == bDeviceType) {
        if (CheckCntaxFpxx(stp_fpxx, errinfo) < 0) {
            i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
            // sprintf(errinfo, "分设备检测不通过");
            goto Err;
        }
    }

    stp_fpxx->spsl = i_SpxxNum;
    int last_fphxz = -1, nAvailSpxxIndex = 0;
    for (i_count = 0; i_count < i_SpxxNum; i_count++) {
        nAvailSpxxIndex++;
        stp_Spxx = malloc(sizeof(struct Spxx));
        if (!stp_Spxx) {
            sprintf(errinfo, "商品信息结构体系统内存申请失败");
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
            sprintf(errinfo, "商品信息JSON节点解析出错");
            i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
            goto Err;
        }
        //商品序号
        if (!GetAvailImportData(cjson_tmp, "XH", stp_Spxx->xh, 6, false, json_err)) {
            if (!strcmp(
                    stp_Spxx->xh,
                    "0")) {  //清单头，跳过[外包方导入开票会使用，xh0为详解销货清单行，此处直接跳过]
                free(stp_Spxx);
                nAvailSpxxIndex--;
                continue;
            }
            if (atoi(stp_Spxx->xh) != nAvailSpxxIndex) {
                if (stp_Spxx)
                    free(stp_Spxx);
                sprintf(errinfo, "商品信息中商品序号与json中顺序不一致,商品序号 %d",
                        nAvailSpxxIndex);
                i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
                goto Err;
            }
        }
        //商品名称
        if (GetAvailImportData(cjson_tmp, "SPMC", stp_Spxx->spmc, 100, true, json_err) < 0) {
            _WriteLog(LL_FATAL, "JSON Parse SPMC failed\n");
            if (stp_Spxx)
                free(stp_Spxx);
            sprintf(errinfo, "商品信息中商品名称(spmc)解析出错,%s,商品序号 %d", json_err,
                    nAvailSpxxIndex);
            i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
            goto Err;
        }

        if ((stp_fpxx->isRed == 1) &&
            (strcasecmp(stp_fpxx->qdbj, "Y") == 0 ||
             strcmp(stp_fpxx->qdbj, "1") == 0))  //如果是红票清单，商品为详见销货清单，直接不解析
        {
            if (strcmp(stp_Spxx->spmc, "(详见销货清单)") == 0) {
                free(stp_Spxx);
                nAvailSpxxIndex--;
                continue;
            }
        }

        //商品税目名称
        if (GetAvailImportData(cjson_tmp, "SPSMMC", stp_Spxx->spsmmc, 100, true, json_err) < 0) {
            _WriteLog(LL_FATAL, "JSON Parse SPSMMC failed\n");
            if (stp_Spxx)
                free(stp_Spxx);
            sprintf(errinfo, "商品信息中商品税目名称(spsmmc)解析出错,%s,商品序号 %d", json_err,
                    nAvailSpxxIndex);
            i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
            goto Err;
        }
        //商品编号
        if (GetAvailImportData(cjson_tmp, "SPBH", stp_Spxx->spbh, 20, true, json_err) < 0) {
            _WriteLog(LL_FATAL, "JSON Parse SPBH failed\n");
            if (stp_Spxx)
                free(stp_Spxx);
            sprintf(errinfo, "商品信息中商品分类编码(spbh)解析出错,%s,商品序号 %d", json_err,
                    nAvailSpxxIndex);
            i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
            goto Err;
        }
#ifdef _CONSOLE  //不要删除此代码，所有的开票软件全部都使用该字段并且有值，核对IO时需要该字段
        //自行编号
        GetAvailImportData(cjson_tmp, "ZXBH", stp_Spxx->zxbh, 50, false, json_err);
        if (strlen(stp_Spxx->zxbh) > 20) {
            _WriteLog(LL_FATAL, "JSON Parse zxbh failed\n");
            if (stp_Spxx)
                free(stp_Spxx);
            sprintf(errinfo, "商品信息中自行编码长度错误");
            i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
            goto Err;
        }
#endif
        //发票行性质
        if (GetAvailImportData(cjson_tmp, "FPHXZ", stp_Spxx->fphxz, 3, true, json_err) < 0) {
            _WriteLog(LL_FATAL, "JSON Parse FPHXZ failed\n");
            if (stp_Spxx)
                free(stp_Spxx);
            sprintf(errinfo, "商品信息中商品行性质(fphxz)解析出错,%s,商品序号 %d", json_err,
                    nAvailSpxxIndex);
            i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
            goto Err;
        }
        nTmpVar = atoi(stp_Spxx->fphxz);
		if ((strcmp(stp_Spxx->fphxz, "0") != 0) &&
			(strcmp(stp_Spxx->fphxz, "1") != 0) &&
			(strcmp(stp_Spxx->fphxz, "2") != 0))	{
            sprintf(errinfo, "商品信息中商品行性质目前仅支持 0 1 2 三种");
            _WriteLog(LL_FATAL, "%s\n", errinfo);
            if (stp_Spxx)
                free(stp_Spxx);
            i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
            goto Err;
        }
        if (nTmpVar == 0) {
            if (last_fphxz == 2) {
                sprintf(errinfo, "商品信息中本行（%d）商品为正常行，而上一行为被折扣行错误",
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
                sprintf(errinfo, "商品信息中折扣行需在被折扣行之后,商品行数：%d", nAvailSpxxIndex);
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
            exsit_zkxx = 1;  //存在折扣信息
        //单价
        if (GetAvailImportData(cjson_tmp, "DJ", stp_Spxx->dj, 21, false, json_err) < 0) {
            _WriteLog(LL_FATAL, "JSON Parse DJ failed\n");
            if (stp_Spxx)
                free(stp_Spxx);
            sprintf(errinfo, "商品信息中单价(dj)解析出错,%s,商品序号 %d", json_err,
                    nAvailSpxxIndex);
            i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
            goto Err;
        }
        //计量单位
        if (GetAvailImportData(cjson_tmp, "JLDW", stp_Spxx->jldw, 22, false, json_err) < 0) {
            _WriteLog(LL_FATAL, "JSON Parse JLDW failed\n");
            if (stp_Spxx)
                free(stp_Spxx);
            sprintf(errinfo, "商品信息中计量单位(jldw)解析出错,%s,商品序号 %d", json_err,
                    nAvailSpxxIndex);
            i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
            goto Err;
        }
        //规格型号
        if (GetAvailImportData(cjson_tmp, "GGXH", stp_Spxx->ggxh, 40, false, json_err) < 0) {
            _WriteLog(LL_FATAL, "JSON Parse GGXH failed\n");
            if (stp_Spxx)
                free(stp_Spxx);
            sprintf(errinfo, "商品信息中规格型号(ggxh)解析出错,%s,商品序号 %d", json_err,
                    nAvailSpxxIndex);
            i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
            goto Err;
        }
        if ((bDeviceType == DEVICE_NISEC) || (bDeviceType == DEVICE_CNTAX)) {
            if (strlen(stp_Spxx->ggxh) > 36) {
                sprintf(errinfo, "税控盘商品信息中规格型号不得超过36字节,商品序号 %d",
                        nAvailSpxxIndex);
                i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
                goto Err;
            }
        }
        //数量
        if (GetAvailImportData(cjson_tmp, "SL", stp_Spxx->sl, 21, false, json_err) < 0) {
            _WriteLog(LL_FATAL, "JSON Parse SL failed\n");
            if (stp_Spxx)
                free(stp_Spxx);
            sprintf(errinfo, "商品信息中数量(sl)解析出错,%s,商品序号 %d", json_err,
                    nAvailSpxxIndex);
            i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
            goto Err;
        }
        //税额
        if (GetAvailImportData(cjson_tmp, "SE", stp_Spxx->se, 18, true, json_err) < 0) {
            _WriteLog(LL_FATAL, "JSON Parse SE failed\n");
            if (stp_Spxx)
                free(stp_Spxx);
            sprintf(errinfo, "商品信息中税额(se)解析出错,%s,商品序号 %d", json_err,
                    nAvailSpxxIndex);
            i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
            goto Err;
        }
        d_zse += atof(stp_Spxx->se);
        if ((fphxz == 0) || (fphxz == 2))
            d_yjhjse += atof(stp_Spxx->se);  //取原价合计税额
        else
            d_zkehjse += atof(stp_Spxx->se);  //取折扣额合计税额

        //金额
        if (GetAvailImportData(cjson_tmp, "JE", stp_Spxx->je, 18, true, json_err) < 0) {
            _WriteLog(LL_FATAL, "JSON Parse JE failed\n");
            if (stp_Spxx)
                free(stp_Spxx);
            sprintf(errinfo, "商品信息中金额(je)解析出错,%s,商品序号 %d", json_err,
                    nAvailSpxxIndex);
            i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
            goto Err;
        }
        d_zje += atof(stp_Spxx->je);
        if ((fphxz == 0) || (fphxz == 2))
            d_yjhjje += atof(stp_Spxx->je);  //取原价合计金额
        else
            d_zkehjje += atof(stp_Spxx->je);  //取折扣额合计金额

        GetAvailImportData(cjson_tmp, "HSJE", stp_Spxx->hsje, 18, false, json_err);

        //税率
        if (GetAvailImportData(cjson_tmp, "SLV", stp_Spxx->slv, 6, true, json_err) < 0) {
            _WriteLog(LL_FATAL, "JSON Parse SLV failed\n");
            if (stp_Spxx)
                free(stp_Spxx);
            sprintf(errinfo, "商品信息中税率(slv)解析出错,%s,商品序号 %d", json_err,
                    nAvailSpxxIndex);
            i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
            goto Err;
        }

        if (strcmp(stp_Spxx->slv, "0.015") == 0) {
            stp_fpxx->zyfpLx = ZYFP_JYZS;
        }
        if (!strcmp(stp_Spxx->slv, "0.03")) {
            //如果是1%和3%政策性减税和免税，默认填入第二个选择项目
            stp_fpxx->specialFlag = TSFP_XGM_SPECIAL_TAX;
            if (stp_fpxx->sslkjly == 0)  //未填入则默认为3
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
        //优惠政策标识
        if (GetAvailImportData(cjson_tmp, "YHZCBS", stp_Spxx->xsyh, 3, true, json_err) < 0) {
            _WriteLog(LL_FATAL, "JSON Parse YHZCBS failed\n");
            if (stp_Spxx)
                free(stp_Spxx);
            sprintf(errinfo, "商品信息中优惠政策标识(yhzcbs)解析出错,%s,商品序号 %d", json_err,
                    nAvailSpxxIndex);
            i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
            goto Err;
        }
        //税率标识
        if (GetAvailImportData(cjson_tmp, "SLBS", stp_Spxx->lslvbs, 3, false, json_err) < 0) {
            _WriteLog(LL_FATAL, "JSON Parse SLBS failed\n");
            if (stp_Spxx)
                free(stp_Spxx);
            sprintf(errinfo, "商品信息中税率标识(slbs)解析出错,%s,商品序号 %d", json_err,
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

        if ((strcmp(stp_fpxx->tax_nature, "小规模纳税人") == 0) && (atoi(stp_Spxx->lslvbs) == 3)) {
            if (stp_Spxx)
                free(stp_Spxx);
            sprintf(errinfo, "小规模纳税人不允许开具普通零税率发票");
            i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
            goto Err;
        }
        //增值税特殊管理
        if (GetAvailImportData(cjson_tmp, "ZZSTSGL", stp_Spxx->yhsm, 50, false, json_err) < 0) {
            _WriteLog(LL_FATAL, "JSON Parse ZZSTSGL failed\n");
            if (stp_Spxx)
                free(stp_Spxx);
            sprintf(errinfo, "商品信息中增值税特殊管理(zzstsgl)解析出错,%s,商品序号 %d", json_err,
                    nAvailSpxxIndex);
            i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
            goto Err;
        }
        //扣除金额 差额征税扣除额
        if (GetAvailImportData(cjson_tmp, "CEZSKCE", stp_Spxx->kce, 20, false, json_err) < 0) {
            _WriteLog(LL_FATAL, "JSON Parse CEZSKCE failed\n");
            if (stp_Spxx)
                free(stp_Spxx);
            sprintf(errinfo, "商品信息中差额征税扣除额(cezskce)解析出错,%s,商品序号 %d", json_err,
                    nAvailSpxxIndex);
            i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
            goto Err;
        }
        if (strlen(stp_Spxx->kce) > 0)
            stp_fpxx->zyfpLx = ZYFP_CEZS;

        //含税价标志
        if (GetAvailImportData(cjson_tmp, "HSJBZ", stp_Spxx->hsjbz, 3, false, json_err) < 0) {
            memcpy(stp_Spxx->hsjbz, stp_fpxx->hsjbz, strlen(stp_fpxx->hsjbz));
        }
        // --nisec盘更新含税价为不含税价
        SpxxPriceRemoveTaxContition(bDeviceType, stp_fpxx, stp_Spxx);
        if (check_spxx_info(bDeviceType, stp_fpxx, stp_Spxx, errinfo) < 0) {
            _WriteLog(LL_FATAL, "商品信息参数检测存在错误\n");
            if (stp_Spxx)
                free(stp_Spxx);
            i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
            goto Err;
        }
        // //--如果是aisino盘
        // if (DEVICE_AISINO == bDeviceType) {
        //     if (strlen(stp_Spxx->hsjbz) == 0 || !strcmp(stp_Spxx->hsjbz, "0"))
        //         strcpy(stp_Spxx->hsjbz, "0");
        //     else
        //         strcpy(stp_Spxx->hsjbz, "1");
        //     // 20220908最新测试结果
        //     // 电子发票免税开具时，含税价标志为1,专普票为0,无论在开票界面点击含税开具与否，此处标志永远固定
        //     if ((!strcmp(stp_Spxx->se, "0.00") &&
        //          IsAisino3Device(stp_fpxx->hDev->nDriverVersionNum))) {
        //         //旧标准3%按照免税发票开具（零税率发票待测试）
        //         if (stp_fpxx->fplx_aisino == FPLX_AISINO_DZFP)
        //             strcpy(stp_Spxx->hsjbz, "1");
        //         else
        //             strcpy(stp_Spxx->hsjbz, "0");
        //     }
        // }
        //插入结构体链表
        if (InsertMxxx(stp_fpxx, stp_Spxx) != 0) {
            _WriteLog(LL_FATAL, "JSON Parse 插入结构体链表failed\n");
            if (stp_Spxx)
                free(stp_Spxx);
            sprintf(errinfo, "系统错误，商品数据无法插入链表");
            i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
            goto Err;
        }
        //分设备商品信息处理
        if (DEVICE_AISINO == bDeviceType) {
            if (CheckAisinoSpxx(stp_fpxx, stp_Spxx, errinfo) < 0) {
                i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
                sprintf(errinfo, "分设备检测不通过");
                goto Err;
            }
        } else if (DEVICE_NISEC == bDeviceType) {
            if (CheckNisecSpxx(stp_fpxx, stp_Spxx, errinfo) < 0) {
                i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
                sprintf(errinfo, "分设备检测不通过");
                goto Err;
            }
        } else if (DEVICE_CNTAX == bDeviceType) {
            // if (strlen(stp_Spxx->kce) > 0) {
            //     i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
            //     sprintf(errinfo, "暂不支持税务ukey开具差额征收发票");
            //     goto Err;
            // }
            // if (strlen(stp_Spxx->yhsm) > 0) {
            //     i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
            //     sprintf(errinfo, "暂不支持税务ukey开具简易征税发票");
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
        sprintf(errinfo, "所有商品行统计的税额误差超过1.27,不允许开票");
        i_ErrNum = DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
        goto Err;
    }

    //价税合计
    d_jshj = d_zje + d_zse;
    memset(szBuf, 0, sizeof(szBuf));
    sprintf(szBuf, "%4.2f", d_jshj);
    if (strcmp(szBuf, stp_fpxx->jshj)) {
        _WriteLog(LL_FATAL,
                  "JSON judge d_jshj,计算价税合计 = %s,填入价税合计 = "
                  "%s,计算总金额为：%4.2f,填入总金额为：%s,计算总税额为：%4.2f,填入总税额为:%s\n",
                  szBuf, stp_fpxx->jshj, d_zje, stp_fpxx->je, d_zse, stp_fpxx->se);
        sprintf(errinfo, "所有商品行统计的价税合计不一致");
        i_ErrNum = DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
        goto Err;
    }
    //总金额
    memset(szBuf, 0, sizeof(szBuf));
    sprintf(szBuf, "%4.2f", d_zje);
    if (strcmp(szBuf, stp_fpxx->je)) {
        _WriteLog(LL_FATAL, "JSON judge d_zje failed\n");
        sprintf(errinfo, "所有商品行统计的总金额不一致");
        i_ErrNum = DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
        goto Err;
    }
    memcpy(stp_fpxx->stp_MxxxHead->je, szBuf, strlen(szBuf));
    //总税额
    memset(szBuf, 0, sizeof(szBuf));
    sprintf(szBuf, "%4.2f", d_zse);
    if (strcmp(szBuf, stp_fpxx->se)) {
        _WriteLog(LL_FATAL, "JSON judge d_zse failed\n");
        sprintf(errinfo, "所有商品行统计的总税额不一致,合计税为%s,累计税额为%s", stp_fpxx->se,
                szBuf);
        i_ErrNum = DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
        goto Err;
    }
    memcpy(stp_fpxx->stp_MxxxHead->se, szBuf, strlen(szBuf));
    //判断是否全部为同一税率,如果是才写入综合税率,留空也可以
    stp_fpxx->isMultiTax = 0;
    if (SamSlvFlag) {
        memset(szBuf, 0, sizeof(szBuf));
        sprintf(szBuf, "%4.3f", d_slv);
        if (strncmp(stp_fpxx->slv, szBuf, 4)) {  // 0.01
            _WriteLog(LL_FATAL, "JSON judge d_zhsl failed\n");
            sprintf(errinfo, "所有商品行统计的综合税率匹配不一致,综合税率为%s,累计税率为%s",
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
            sprintf(errinfo, "使用0.015税率的商品不支持多税率填开");
            i_ErrNum = DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
            goto Err;
        }
    }
    //大于8条明细 有清单 详见销货清单
    bool bZZSRedDetailList = false;  //增值税红票清单，不能插入节点信息
    if ((i_SpxxNum > 8 || strcasecmp(stp_fpxx->qdbj, "Y") == 0 ||
         strcmp(stp_fpxx->qdbj, "1") == 0) &&
        (stp_fpxx->fplx_aisino != FPLX_AISINO_DZFP && stp_fpxx->fplx_aisino != FPLX_AISINO_DZZP)) {
        _WriteLog(LL_INFO, "清单开票,清单标志为%s\n", stp_fpxx->qdbj);

        if (stp_fpxx->zyfpLx == ZYFP_CEZS) {
            sprintf(errinfo, "差额征收不支持清单开票");
            _WriteLog(LL_FATAL, "%s\n", errinfo);
            i_ErrNum = DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
            goto Err;
        }
        if (!stp_fpxx->isRed) {  //蓝票使用清单开票
            strcpy(stp_fpxx->qdbj, "Y");
            if (DEVICE_AISINO == bDeviceType) {
                strcpy(stp_fpxx->stp_MxxxHead->spmc, "(详见销货清单)");
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
                strcpy(stp_fpxx->stp_MxxxHead->spmc, "（详见销货清单）");
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
        } else {  //红票使用清单开票，专普票需要特殊处理
            bZZSRedDetailList = true;
            stp_fpxx->isRedDetailList = 1;
            strcpy(stp_fpxx->qdbj, "Y");
            strcpy(stp_fpxx->stp_MxxxHead->spmc, "详见对应正数发票及清单");
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
                //红票清单只有专票和普票有
            }
        }

        if (exsit_zkxx == 1) {
            if ((bDeviceType == DEVICE_NISEC) || (bDeviceType == DEVICE_CNTAX)) {
                // nisec已测试无误
                //
                // sprintf(errinfo, "暂不支持税控盘折扣清单发票开具");
                // _WriteLog(LL_FATAL, "%s\n", errinfo);
                // i_ErrNum = DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
                // goto Err;
            } else {
                // aisino
                _WriteLog(LL_WARN, "清单商品中存在折扣信息\n");
                _WriteLog(LL_WARN, "添加原价合计\n");
                stp_Spxx = malloc(sizeof(struct Spxx));
                if (!stp_Spxx) {
                    sprintf(errinfo, "商品信息结构体中原价合计系统内存申请失败");
                    _WriteLog(LL_FATAL, "%s\n", errinfo);
                    i_ErrNum = DF_TAX_ERR_CODE_SYSTEM_MEM_INSUFFICIENT;
                    goto Err;
                }
                memset(stp_Spxx, 0, sizeof(struct Spxx));
                strcpy(stp_Spxx->spsmmc, "原价合计");
                sprintf(stp_Spxx->fphxz, "%d", FPHXZ_AISINO_XHQDZK);
                sprintf(stp_Spxx->je, "%4.2f", d_yjhjje);
                sprintf(stp_Spxx->se, "%4.2f", d_yjhjse);
                strcpy(stp_Spxx->hsjbz, stp_fpxx->hsjbz);
                sprintf(stp_Spxx->xsyh, "0");
                strcpy(stp_Spxx->slv, stp_fpxx->stp_MxxxHead->slv);
                if (InsertMxxx(stp_fpxx, stp_Spxx) != 0) {
                    _WriteLog(LL_FATAL, "JSON Parse 插入结构体链表failed\n");
                    if (stp_Spxx)
                        free(stp_Spxx);
                    sprintf(errinfo, "系统错误，原价合计商品数据无法插入链表");
                    i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
                    goto Err;
                }

                _WriteLog(LL_WARN, "添加折扣额合计\n");
                stp_Spxx = malloc(sizeof(struct Spxx));
                if (!stp_Spxx) {
                    sprintf(errinfo, "商品信息结构体中原价合计系统内存申请失败");
                    _WriteLog(LL_FATAL, "%s\n", errinfo);
                    i_ErrNum = DF_TAX_ERR_CODE_SYSTEM_MEM_INSUFFICIENT;
                    goto Err;
                }
                memset(stp_Spxx, 0, sizeof(struct Spxx));
                strcpy(stp_Spxx->spsmmc, "折扣额合计");
                sprintf(stp_Spxx->fphxz, "%d", FPHXZ_AISINO_XHQDZK);
                sprintf(stp_Spxx->je, "%4.2f", d_zkehjje);
                sprintf(stp_Spxx->se, "%4.2f", d_zkehjse);
                strcpy(stp_Spxx->hsjbz, stp_fpxx->hsjbz);
                sprintf(stp_Spxx->xsyh, "0");
                strcpy(stp_Spxx->slv, stp_fpxx->stp_MxxxHead->slv);
                if (InsertMxxx(stp_fpxx, stp_Spxx) != 0) {
                    _WriteLog(LL_FATAL, "JSON Parse 插入结构体链表failed\n");
                    if (stp_Spxx)
                        free(stp_Spxx);
                    sprintf(errinfo, "系统错误，原价合计商品数据无法插入链表");
                    i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
                    goto Err;
                }
            }
        }

        if (bZZSRedDetailList) {
            //移除所有spxx除head之外的节点
            struct Spxx *stp_MxxxNode = stp_fpxx->stp_MxxxHead->stp_next, *stp_TmpSpxxNode = NULL;
            //释放商品明细
            while (stp_MxxxNode) {
                stp_TmpSpxxNode = stp_MxxxNode->stp_next;
                free(stp_MxxxNode);
                stp_MxxxNode = stp_TmpSpxxNode;
            }
            //修复链表，说明信息当作商品信息
            stp_fpxx->stp_MxxxHead->stp_next = malloc(sizeof(struct Spxx));
            memcpy(stp_fpxx->stp_MxxxHead->stp_next, stp_fpxx->stp_MxxxHead, sizeof(struct Spxx));
            stp_TmpSpxxNode = stp_fpxx->stp_MxxxHead->stp_next;
            stp_TmpSpxxNode->stp_next = NULL;
            strcpy(stp_TmpSpxxNode->hsjbz, "0");
            strcpy(stp_TmpSpxxNode->xsyh, "0");
            stp_fpxx->spsl = 1;
            //‘详见清单‘刷新至主要商品名称
            strcpy(stp_fpxx->zyspmc, stp_fpxx->stp_MxxxHead->spmc);
            strcpy(stp_fpxx->zyspsmmc, "");
            strcpy(stp_fpxx->qdbj, "N");
            if (DEVICE_AISINO == bDeviceType) {
                //红字清单
                sprintf(stp_fpxx->stp_MxxxHead->stp_next->fphxz, "%d", FPHXZ_AISINO_XJDYZSFPQD);
            }
        }
    } else {
        memcpy(stp_fpxx->qdbj, "N", strlen("N"));
    }
    //备注-很多小种类发票都是在备注中进行标识，因此备注依赖很多参数，需要在最后导入
    char user_bz[240] = {0};
    char tmp_bz[512] = {0};
    memset(user_bz, 0, sizeof(user_bz));
    memset(tmp_bz, 0, sizeof(tmp_bz));

    if (GetAvailImportData(root, "BZ", user_bz, 230, false, json_err) < 0) {
        _WriteLog(LL_FATAL, "JSON Parse BZ\n");
        sprintf(errinfo, "发票JSON数据中销方名称(bz)解析出错,%s", json_err);
        i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        goto Err;
    }
    //如需换行，请在应用层进行兼容，这里对应用层要求换行进行替换成底层
    if (strlen(user_bz) != 0)
        str_replace(user_bz, "<br/>", "\r\n");
    // tmp_bz和user_bz，同时保存原始备注,但是最终user_bz将生成最终版明文备注
    // user_bz除了生成最终备注时改动，中间不再改动；tmp_bz作为缓冲区，会被改变
    if ((DEVICE_AISINO == bDeviceType) || (DEVICE_MENGBAI == bDeviceType)) {
        if (strlen(user_bz) > 230) {
            sprintf(errinfo, "金税盘发票备注不得大于230字节");
            i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
            goto Err;
        }
    } else if (DEVICE_NISEC == bDeviceType) {
        if (stp_fpxx->fplx_aisino == FPLX_AISINO_ZYFP) {
            if (strlen(user_bz) > 184) {
                sprintf(errinfo, "税控盘专用发票备注不得大于184字节");
                i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
                goto Err;
            }
        } else if (stp_fpxx->fplx_aisino == FPLX_AISINO_PTFP) {
            if (strlen(user_bz) > 138) {
                sprintf(errinfo, "税控盘普通发票备注不得大于138字节");
                i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
                goto Err;
            }
        } else if (stp_fpxx->fplx_aisino == FPLX_AISINO_DZFP) {
            if (strlen(user_bz) > 130) {
                sprintf(errinfo, "税控盘电子发票备注不得大于130字节");
                i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
                goto Err;
            }
        }
    } else if (DEVICE_CNTAX == bDeviceType) {
        if (stp_fpxx->fplx_aisino == FPLX_AISINO_ZYFP) {
            if (strlen(user_bz) > 184) {
                sprintf(errinfo, "税务ukey专用发票备注不得大于184字节");
                i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
                goto Err;
            }
        } else if (stp_fpxx->fplx_aisino == FPLX_AISINO_PTFP) {
            if (strlen(user_bz) > 138) {
                sprintf(errinfo, "税务ukey普通发票备注不得大于138字节");
                i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
                goto Err;
            }
        } else if (stp_fpxx->fplx_aisino == FPLX_AISINO_DZFP) {
            if (strlen(user_bz) > 130) {
                sprintf(errinfo, "税务ukey盘电子普票备注不得大于130字节");
                i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
                goto Err;
            }
        } else if (stp_fpxx->fplx_aisino == FPLX_AISINO_DZZP) {
            if (strlen(user_bz) > 230) {
                sprintf(errinfo, "税务ukey电子专票备注不得大于230字节");
                i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
                goto Err;
            }
        }
    }
    strcpy(tmp_bz, user_bz);
    if (strstr(user_bz, "对应正数发票代码:") || strstr(user_bz, "差额征税：") ||
        strstr(user_bz, "发票信息表编号")) {
        _WriteLog(LL_FATAL, "不能出现保留关键字和关键语句，'对应正数发票代码：'.'差额征税：',"
                            "'发票信息表编号'\n");
        sprintf(errinfo, "不能出现保留关键字和关键语句,'对应正数发票代码：'、'"
                         "差额征税：','发票信息表编号'");
        i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        goto Err;
    }
    if (stp_fpxx->isRed) {  //红票备注替换
        if (stp_fpxx->isHzxxb == false && stp_fpxx->hzxxbsqsm != 1 &&
            (!strlen(stp_fpxx->blueFpdm) ||
             !strlen(stp_fpxx->blueFphm))) {  //不能加蓝票开票时间，否则影响现在开票业务
            //除购方已抵扣外，全部需要蓝字发票号码代码
            sprintf(errinfo, "红字发票必现填入完整的原发票代码和原发票号");
            _WriteLog(LL_FATAL, "%s\n", errinfo);
            i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
            goto Err;
        }
        char szRedStr[128] = {0};
        if ((stp_fpxx->fplx_aisino == FPLX_AISINO_ZYFP) ||
            (stp_fpxx->fplx_aisino == FPLX_AISINO_DZZP)) {
            //红字发票通知单编号共16位,校验位为最后一位，是前15位号码之和的个位数
            if (strlen(stp_fpxx->redNum) == 0 || strlen(stp_fpxx->redNum) != 16) {
                sprintf(errinfo, "专用发票红字发票,红字信息表长度错误");
                i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
                goto Err;
            }
            if (GetZYFPRedNotes(bDeviceType, stp_fpxx->redNum, "s", szRedStr) < 0) {
                sprintf(errinfo, "红字发票红字信息表（通知单编号）校验失败");
                i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
                goto Err;
            }
        } else
            GetRedInvNotes(stp_fpxx->blueFpdm, stp_fpxx->blueFphm, szRedStr);
        //如果原始备注为空，则不接备注；如果原始备注不为空，则需要接空格+原始备注


        if (strlen(user_bz)) {
            // if (DEVICE_AISINO == bDeviceType)
            //    sprintf(tmp_bz, " %s", user_bz);
            // else if (DEVICE_NISEC == bDeviceType)
            //    sprintf(tmp_bz, "\n%s", user_bz);
            strcpy(tmp_bz, user_bz);
        }
        // user_bz为空，上面有个strcpy，tmp_bz也为空

        if (stp_fpxx->zyfpLx == ZYFP_CEZS) {
            //税控盘备注：对应正数发票代码号码 + 换行 + 差额征税 + 空格 + 用户备注信息
            //金税盘备注：差额征税。+ 对应正数发票代码号码 + 空格 + 用户备注信息             未确认
            if ((bDeviceType == DEVICE_NISEC) || ((bDeviceType == DEVICE_CNTAX))) {
                if (strlen(szRedStr) != 0)  //默认加入备注后固定跟“差额征税”字样，中间直接加入\n
                {
                    strcat(szRedStr, "\n");
                }
                if (strlen(tmp_bz) != 0)
                    snprintf(user_bz, sizeof(user_bz), "%s差额征税 %s", szRedStr, tmp_bz);
                else
                    snprintf(user_bz, sizeof(user_bz), "%s差额征税", szRedStr);
            } else {
                if (strlen(tmp_bz) != 0)
                    snprintf(user_bz, sizeof(user_bz), "差额征税。%s %s", szRedStr, tmp_bz);
                else
                    snprintf(user_bz, sizeof(user_bz), "差额征税。%s", szRedStr);
            }
        } else {
            //税控盘备注：对应正数发票代码号码 + 换行 + 用户备注信息
            //金税盘备注：对应正数发票代码号码 + 空格 + 用户备注信息             未确认
            if ((bDeviceType == DEVICE_NISEC) || (bDeviceType == DEVICE_CNTAX)) {
                if ((strlen(szRedStr) != 0) &&
                    (strlen(tmp_bz) != 0))  //默认加入备注及用户备注均不为空的时候中间加入\n
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
    } else if (stp_fpxx->zyfpLx == ZYFP_CEZS) {  //差额备注替换
        double fkce = atof(stp_Spxx->kce);
        // char szNlCr[16] = "";
        if (!fkce) {
            _WriteLog(LL_FATAL, "差额征收发票扣除金额填写错误\n");
            sprintf(errinfo, "差额征收发票扣除金额填写错误");
            i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
            goto Err;
        }
        struct Spxx *spxxFirst = (struct Spxx *)stp_fpxx->stp_MxxxHead->stp_next;
        if (spxxFirst->stp_next != NULL) {
            _WriteLog(LL_FATAL, "ZYFP_CEZS, spxx must be one line\n");
            sprintf(errinfo, "差额征收仅支持单行商品");
            i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
            goto Err;
        }
        // if (bDeviceType == DEVICE_CNTAX)
        // strcpy(szNlCr, "\r\n");//(蒙柏方测试此处多余，开票软件没有）
        // cntax是含税价差额
        snprintf(user_bz, sizeof(user_bz), "差额征税：%.2f。%s", fkce, tmp_bz);
    }

    if (strlen(user_bz) != 0)
        strcpy(stp_fpxx->bz, user_bz);
    if ((stp_fpxx->bz[strlen(stp_fpxx->bz) - 2] == 0x0d) &&
        (stp_fpxx->bz[strlen(stp_fpxx->bz) - 1] == 0x0a)) {
        sprintf(errinfo, "备注信息末尾不得输入回车换行即<br/>");
        i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        goto Err;
    }
    if ((stp_fpxx->bz[0] == 0x0d) && (stp_fpxx->bz[1] == 0x0a)) {
        sprintf(errinfo, "备注信息头部不得输入回车换行即<br/>");
        i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        goto Err;
    }
    if (stp_fpxx->bz[strlen(stp_fpxx->bz) - 1] == 0x0a) {
        sprintf(errinfo, "备注信息末尾不得输入回车\n");
        i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        goto Err;
    }
    if (stp_fpxx->bz[0] == 0x0a) {
        sprintf(errinfo, "备注信息头部不得输入回车\n");
        i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        goto Err;
    }

    //此处需另加判断，json解析中单独剔除了bz字段，因为bz字段存在后续拼接情况
    if ((stp_fpxx->bz[0] == 0x20) || (stp_fpxx->bz[0] >= 0x09 && stp_fpxx->bz[0] <= 0x0d)) {
        sprintf(errinfo, "备注信息头部不得输入空格等特殊字符");
        i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        goto Err;
    }
    if ((stp_fpxx->bz[strlen(stp_fpxx->bz) - 1] == 0x20) ||
        (stp_fpxx->bz[strlen(stp_fpxx->bz) - 1] >= 0x09 &&
         stp_fpxx->bz[strlen(stp_fpxx->bz) - 1] <= 0x0d)) {

        sprintf(errinfo, "备注信息末尾不得输入空格等特殊字符\n");
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

//导入当前bin文件同目录下import.json
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
    //载入Json
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