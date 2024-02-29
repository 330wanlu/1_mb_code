/*****************************************************************************
File name:   fpxx_json.c
Description: 用于导入fpxx json转换
Author:      Zako
Version:     1.0
Date:        2020.12
History:
20201216     初始化
*****************************************************************************/
#include "fpxx_json.h"

//返回的内存需要free
int FPXX2Json(HFPXX fpxx, char **stp_fpxx_json)
{
    int i;
    char szBuf[512];
    int Spxx_num;
    cJSON *dir1;
    cJSON *json = cJSON_CreateObject();
    cJSON_AddStringToObject(json, "bmbbbh", fpxx->bmbbbh);
    cJSON_AddStringToObject(json, "bz", fpxx->bz);
    cJSON_AddStringToObject(json, "fhr", fpxx->fhr);
    cJSON_AddStringToObject(json, "fpdm", fpxx->fpdm);
    cJSON_AddStringToObject(json, "fphm", fpxx->fphm);
    sprintf(szBuf, "%03d", fpxx->fplx);
    cJSON_AddStringToObject(json, "fpzls", szBuf);
    // sprintf(szBuf, "%d", fpxx->fplx_aisino);
    // cJSON_AddStringToObject(json, "fplx_aisino", szBuf);
    cJSON_AddStringToObject(json, "gfdzdh", fpxx->gfdzdh);
    cJSON_AddStringToObject(json, "gfmc", fpxx->gfmc);
    cJSON_AddStringToObject(json, "gfsh", fpxx->gfsh);
    cJSON_AddStringToObject(json, "gfyhzh", fpxx->gfyhzh);
    cJSON_AddStringToObject(json, "hjje", fpxx->je);
    // cJSON_AddStringToObject(json, "jqbh", fpxx->jqbh);
    cJSON_AddStringToObject(json, "spbh", fpxx->hDev->szDeviceID);
    cJSON_AddStringToObject(json, "jym", fpxx->jym);
    cJSON_AddStringToObject(json, "kpr", fpxx->kpr);
    cJSON_AddStringToObject(json, "kpsj_standard", fpxx->kpsj_standard);
    // cJSON_AddStringToObject(json, "kpsj_F1", fpxx->kpsj_F1);
    // cJSON_AddStringToObject(json, "kpsj_F2", fpxx->kpsj_F2);
    // cJSON_AddStringToObject(json, "kpsj_F3", fpxx->kpsj_F3);
    cJSON_AddStringToObject(json, "mw", fpxx->mw);
    cJSON_AddStringToObject(json, "hjse", fpxx->se);
    cJSON_AddStringToObject(json, "sign", fpxx->sign);
    cJSON_AddStringToObject(json, "skr", fpxx->skr);
    memset(szBuf, 0, sizeof(szBuf));
    if (strcmp(fpxx->slv, "99.01"))
        strcpy(szBuf, fpxx->slv);
    cJSON_AddStringToObject(json, "zhsl", szBuf);

    cJSON_AddStringToObject(json, "ssyf", fpxx->ssyf);
    cJSON_AddStringToObject(json, "xfdzdh", fpxx->xfdzdh);
    cJSON_AddStringToObject(json, "xfmc", fpxx->xfmc);
    cJSON_AddStringToObject(json, "xfsh", fpxx->xfsh);
    cJSON_AddStringToObject(json, "xfyhzh", fpxx->xfyhzh);
    sprintf(szBuf, "%d", fpxx->zyfpLx);
    cJSON_AddStringToObject(json, "zyfpLx", szBuf);
    cJSON_AddStringToObject(json, "zyspmc", fpxx->zyspmc);
    cJSON_AddStringToObject(json, "kprjbbh", fpxx->kprjbbh);
    cJSON_AddStringToObject(json, "qdbz", fpxx->qdbj);
    sprintf(szBuf, "%d", fpxx->isRed);
    cJSON_AddStringToObject(json, "kplx", szBuf);
    sprintf(szBuf, "%d", fpxx->isMultiTax);
    cJSON_AddStringToObject(json, "isMultiTax", szBuf);
    cJSON_AddStringToObject(json, "tzdbh", fpxx->redNum);
    sprintf(szBuf, "%d", fpxx->zfbz);
    cJSON_AddStringToObject(json, "zfbz", szBuf);
    cJSON_AddStringToObject(json, "yfpdm", fpxx->blueFpdm);
    cJSON_AddStringToObject(json, "yfphm", fpxx->blueFphm);
    cJSON_AddStringToObject(json, "jshj", fpxx->jshj);
    if (!strcmp(fpxx->hsjbz, "0") || !strlen(fpxx->hsjbz))
        cJSON_AddStringToObject(json, "hsjbz", "0");
    else
        cJSON_AddStringToObject(json, "hsjbz", "1");
    cJSON_AddStringToObject(json, "zyspsmmc", fpxx->zyspsmmc);
    sprintf(szBuf, "%d", fpxx->spsl);
    cJSON_AddStringToObject(json, "spsl", szBuf);
    cJSON_AddStringToObject(json, "fpqqlsh", fpxx->fpqqlsh);
    cJSON_AddStringToObject(json, "pubcode", fpxx->pubcode);
    // cJSON_AddStringToObject(json, "yysbz", fpxx->yysbz);
    cJSON_AddStringToObject(json, "zfsj", fpxx->zfsj);
    sprintf(szBuf, "%d", fpxx->kpjh);
    cJSON_AddStringToObject(json, "kpjh", szBuf);
    // cJSON_AddStringToObject(json, "slxlh", fpxx->slxlh);
    sprintf(szBuf, "%d", fpxx->dzsyh);
    cJSON_AddStringToObject(json, "dzsyh", szBuf);
    cJSON_AddItemToObject(json, "fpmx", dir1 = cJSON_CreateArray());

    struct Spxx *stp_Spxx = NULL;
    if (strcmp(fpxx->qdbj, "Y") == 0) {
        stp_Spxx = fpxx->stp_MxxxHead;
        Spxx_num = fpxx->spsl + 1;
    } else {
        stp_Spxx = fpxx->stp_MxxxHead->stp_next;
        Spxx_num = fpxx->spsl;
    }
    for (i = 0; i < Spxx_num; i++) {
        if (stp_Spxx == NULL) {
            break;
        }
        cJSON *dir2 = NULL;
        if (!strcmp(fpxx->qdbj, "Y") && !i) {
            //实现导出的json可以再导入开票，并兼容蒙柏代码，清单首节点'（详见销货清单）'不记录
            stp_Spxx = stp_Spxx->stp_next;
            continue;
        } else {
            cJSON_AddItemToObject(dir1, "dira", dir2 = cJSON_CreateObject());
            cJSON_AddStringToObject(dir2, "xh", stp_Spxx->xh);
        }
        cJSON_AddStringToObject(dir2, "spmc", stp_Spxx->spmc);
        cJSON_AddStringToObject(dir2, "spbh", stp_Spxx->spbh);
        cJSON_AddStringToObject(dir2, "spsm", stp_Spxx->spsm);
        cJSON_AddStringToObject(dir2, "ggxh", stp_Spxx->ggxh);
        cJSON_AddStringToObject(dir2, "jldw", stp_Spxx->jldw);
        cJSON_AddStringToObject(dir2, "dj", stp_Spxx->dj);
        cJSON_AddStringToObject(dir2, "sl", stp_Spxx->sl);
        cJSON_AddStringToObject(dir2, "je", stp_Spxx->je);
        cJSON_AddStringToObject(dir2, "slv", stp_Spxx->slv);
        cJSON_AddStringToObject(dir2, "se", stp_Spxx->se);
        cJSON_AddStringToObject(dir2, "zxbh", stp_Spxx->zxbh);
        cJSON_AddStringToObject(dir2, "fphxz", stp_Spxx->fphxz);
        cJSON_AddStringToObject(dir2, "hsjbz", stp_Spxx->hsjbz);
        cJSON_AddStringToObject(dir2, "yhzcbs", stp_Spxx->xsyh);
        cJSON_AddStringToObject(dir2, "zzstsgl", stp_Spxx->yhsm);
        cJSON_AddStringToObject(dir2, "slbs", stp_Spxx->lslvbs);
        cJSON_AddStringToObject(dir2, "cezskce", stp_Spxx->kce);
        cJSON_AddStringToObject(dir2, "spsmmc", stp_Spxx->spsmmc);
        stp_Spxx = stp_Spxx->stp_next;
    }
    char *pJsonBuf;
    pJsonBuf = cJSON_Print(json);
    *stp_fpxx_json = pJsonBuf;
    cJSON_Delete(json);
    return 0;
}

//解析发票上传json数据
int Json2FPXX(char *json_data, HFPXX fpxx)
{
    cJSON *root = NULL, *jsp_ArraySpxx = NULL;
    struct Spxx *stp_Spxx = NULL;
    int i_ErrNum = 0;
    char szTmp[64] = {0};
    int i;
    if (!json_data || !fpxx)
        return -1;  //参数错误
    root = cJSON_Parse((const char *)json_data);
    if (!root) {
        i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        goto JSon2FPXX_Err;
    }
    // bmbbbh
    if (GetAvailImportData(root, "bmbbbh", fpxx->bmbbbh, sizeof(fpxx->bmbbbh), true, NULL) < 0) {
        _WriteLog(LL_FATAL, "JSON Parse bmbbbh failed\n");
        i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        goto JSon2FPXX_Err;
    }
    // bz
    if (GetAvailImportData(root, "bz", fpxx->bz, sizeof(fpxx->bz), true, NULL) < 0) {
        _WriteLog(LL_FATAL, "JSON Parse bz failed\n");
        i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        goto JSon2FPXX_Err;
    }
    // fhr
    if (GetAvailImportData(root, "fhr", fpxx->fhr, sizeof(fpxx->fhr), true, NULL) < 0) {
        _WriteLog(LL_FATAL, "JSON Parse fhr failed\n");
        i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        goto JSon2FPXX_Err;
    }
    // fpdm
    if (GetAvailImportData(root, "fpdm", fpxx->fpdm, sizeof(fpxx->fpdm), true, NULL) < 0) {
        _WriteLog(LL_FATAL, "JSON Parse fpdm failed\n");
        i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        goto JSon2FPXX_Err;
    }
    // fphm
    if (GetAvailImportData(root, "fphm", fpxx->fphm, sizeof(fpxx->fphm), true, NULL) < 0) {
        _WriteLog(LL_FATAL, "JSON Parse fphm failed\n");
        i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        goto JSon2FPXX_Err;
    }
    // fplx
    if (GetAvailImportData(root, "fpzls", szTmp, sizeof(szTmp), true, NULL) < 0) {
        _WriteLog(LL_FATAL, "JSON Parse fplx failed\n");
        i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        goto JSon2FPXX_Err;
    }
    fpxx->fplx = atoi(szTmp);
    // fplx_aisno
    if (GetAvailImportData(root, "fplx_aisino", szTmp, sizeof(szTmp), true, NULL) < 0) {
        _WriteLog(LL_FATAL, "JSON Parse fplx_aisino failed\n");
        i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        goto JSon2FPXX_Err;
    }
    fpxx->fplx_aisino = atoi(szTmp);
    // gfdzdh
    if (GetAvailImportData(root, "gfdzdh", fpxx->gfdzdh, sizeof(fpxx->gfdzdh), true, NULL) < 0) {
        _WriteLog(LL_FATAL, "JSON Parse gfdzdh failed\n");
        i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        goto JSon2FPXX_Err;
    }
    // gfmc
    if (GetAvailImportData(root, "gfmc", fpxx->gfmc, sizeof(fpxx->gfmc), true, NULL) < 0) {
        _WriteLog(LL_FATAL, "JSON Parse gfmc failed\n");
        i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        goto JSon2FPXX_Err;
    }
    // gfsh
    if (GetAvailImportData(root, "gfsh", fpxx->gfsh, sizeof(fpxx->gfsh), true, NULL) < 0) {
        _WriteLog(LL_FATAL, "JSON Parse gfsh failed\n");
        i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        goto JSon2FPXX_Err;
    }
    // gfyhzh
    if (GetAvailImportData(root, "gfyhzh", fpxx->gfyhzh, sizeof(fpxx->gfyhzh), true, NULL) < 0) {
        _WriteLog(LL_FATAL, "JSON Parse gfyhzh failed\n");
        i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        goto JSon2FPXX_Err;
    }
    // je
    if (GetAvailImportData(root, "je", fpxx->je, sizeof(fpxx->je), true, NULL) < 0) {
        _WriteLog(LL_FATAL, "JSON Parse je failed\n");
        i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        goto JSon2FPXX_Err;
    }
    // jqbh
    if (GetAvailImportData(root, "jqbh", fpxx->jqbh, sizeof(fpxx->jqbh), true, NULL) < 0) {
        _WriteLog(LL_FATAL, "JSON Parse jqbh failed\n");
        i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        goto JSon2FPXX_Err;
    }
    // jym
    if (GetAvailImportData(root, "jym", fpxx->jym, sizeof(fpxx->jym), true, NULL) < 0) {
        _WriteLog(LL_FATAL, "JSON Parse jym failed\n");
        i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        goto JSon2FPXX_Err;
    }
    // kpr
    if (GetAvailImportData(root, "kpr", fpxx->kpr, sizeof(fpxx->kpr), true, NULL) < 0) {
        _WriteLog(LL_FATAL, "JSON Parse kpr failed\n");
        i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        goto JSon2FPXX_Err;
    }
    // kpsj_standard
    if (GetAvailImportData(root, "kpsj_standard", fpxx->kpsj_standard, sizeof(fpxx->kpsj_standard),
                           true, NULL) < 0) {
        _WriteLog(LL_FATAL, "JSON Parse kpsj_standard failed\n");
        i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        goto JSon2FPXX_Err;
    }
    // kpsj_F1
    if (GetAvailImportData(root, "kpsj_F1", fpxx->kpsj_F1, sizeof(fpxx->kpsj_F1), true, NULL) < 0) {
        _WriteLog(LL_FATAL, "JSON Parse kpsj_F1 failed\n");
        i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        goto JSon2FPXX_Err;
    }
    // kpsj_F2
    if (GetAvailImportData(root, "kpsj_F2", fpxx->kpsj_F2, sizeof(fpxx->kpsj_F2), true, NULL) < 0) {
        _WriteLog(LL_FATAL, "JSON Parse kpsj_F2 failed\n");
        i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        goto JSon2FPXX_Err;
    }
    // kpsj_F3
    if (GetAvailImportData(root, "kpsj_F3", fpxx->kpsj_F3, sizeof(fpxx->kpsj_F3), true, NULL) < 0) {
        _WriteLog(LL_FATAL, "JSON Parse kpsj_F3 failed\n");
        i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        goto JSon2FPXX_Err;
    }
    // mw
    if (GetAvailImportData(root, "mw", fpxx->mw, sizeof(fpxx->mw), true, NULL) < 0) {
        _WriteLog(LL_FATAL, "JSON Parse mw failed\n");
        i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        goto JSon2FPXX_Err;
    }
    // se
    if (GetAvailImportData(root, "se", fpxx->se, sizeof(fpxx->se), true, NULL) < 0) {
        _WriteLog(LL_FATAL, "JSON Parse se failed\n");
        i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        goto JSon2FPXX_Err;
    }
    // sign
    if (GetAvailImportData(root, "sign", fpxx->sign, sizeof(fpxx->sign), true, NULL) < 0) {
        _WriteLog(LL_FATAL, "JSON Parse sign failed\n");
        i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        goto JSon2FPXX_Err;
    }
    // skr
    if (GetAvailImportData(root, "skr", fpxx->skr, sizeof(fpxx->skr), true, NULL) < 0) {
        _WriteLog(LL_FATAL, "JSON Parse skr failed\n");
        i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        goto JSon2FPXX_Err;
    }
    // slv
    if (GetAvailImportData(root, "slv", fpxx->slv, sizeof(fpxx->slv), true, NULL) < 0) {
        _WriteLog(LL_FATAL, "JSON Parse slv failed\n");
        i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        goto JSon2FPXX_Err;
    }
    // ssyf
    if (GetAvailImportData(root, "ssyf", fpxx->ssyf, sizeof(fpxx->ssyf), true, NULL) < 0) {
        _WriteLog(LL_FATAL, "JSON Parse ssyf failed\n");
        i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        goto JSon2FPXX_Err;
    }
    // xfdzdh
    if (GetAvailImportData(root, "xfdzdh", fpxx->xfdzdh, sizeof(fpxx->xfdzdh), true, NULL) < 0) {
        _WriteLog(LL_FATAL, "JSON Parse xfdzdh failed\n");
        i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        goto JSon2FPXX_Err;
    }
    // xfmc
    if (GetAvailImportData(root, "xfmc", fpxx->xfmc, sizeof(fpxx->xfmc), true, NULL) < 0) {
        _WriteLog(LL_FATAL, "JSON Parse xfmc failed\n");
        i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        goto JSon2FPXX_Err;
    }
    // xfsh
    if (GetAvailImportData(root, "xfsh", fpxx->xfsh, sizeof(fpxx->xfsh), true, NULL) < 0) {
        _WriteLog(LL_FATAL, "JSON Parse xfsh failed\n");
        i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        goto JSon2FPXX_Err;
    }
    // xfyhzh
    if (GetAvailImportData(root, "xfyhzh", fpxx->xfyhzh, sizeof(fpxx->xfyhzh), true, NULL) < 0) {
        _WriteLog(LL_FATAL, "JSON Parse xfyhzh failed\n");
        i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        goto JSon2FPXX_Err;
    }
    // zyfpLx
    if (GetAvailImportData(root, "zyfpLx", szTmp, sizeof(szTmp), true, NULL) < 0) {
        _WriteLog(LL_FATAL, "JSON Parse zyfpLx failed\n");
        i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        goto JSon2FPXX_Err;
    }
    fpxx->zyfpLx = atoi(szTmp);
    // zyspmc
    if (GetAvailImportData(root, "zyspmc", fpxx->zyspmc, sizeof(fpxx->zyspmc), true, NULL) < 0) {
        _WriteLog(LL_FATAL, "JSON Parse zyspmc failed\n");
        i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        goto JSon2FPXX_Err;
    }
    // kprjbbh
    if (GetAvailImportData(root, "kprjbbh", fpxx->kprjbbh, sizeof(fpxx->kprjbbh), true, NULL) < 0) {
        _WriteLog(LL_FATAL, "JSON Parse kprjbbh failed\n");
        i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        goto JSon2FPXX_Err;
    }
    // qdbj
    if (GetAvailImportData(root, "qdbj", fpxx->qdbj, sizeof(fpxx->qdbj), true, NULL) < 0) {
        _WriteLog(LL_FATAL, "JSON Parse qdbj failed\n");
        i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        goto JSon2FPXX_Err;
    }
    // isRed
    if (GetAvailImportData(root, "isRed", szTmp, sizeof(szTmp), true, NULL) < 0) {
        _WriteLog(LL_FATAL, "JSON Parse isRed failed\n");
        i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        goto JSon2FPXX_Err;
    }
    fpxx->isRed = atoi(szTmp);
    // isMultiTax
    if (GetAvailImportData(root, "isMultiTax", szTmp, sizeof(szTmp), true, NULL) < 0) {
        _WriteLog(LL_FATAL, "JSON Parse isRed failed\n");
        i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        goto JSon2FPXX_Err;
    }
    fpxx->isMultiTax = atoi(szTmp);
    // redNum
    if (GetAvailImportData(root, "redNum", fpxx->redNum, sizeof(fpxx->redNum), true, NULL) < 0) {
        _WriteLog(LL_FATAL, "JSON Parse redNum failed\n");
        i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        goto JSon2FPXX_Err;
    }
    // zfbz
    if (GetAvailImportData(root, "zfbz", szTmp, sizeof(szTmp), true, NULL) < 0) {
        _WriteLog(LL_FATAL, "JSON Parse zfbz failed\n");
        i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        goto JSon2FPXX_Err;
    }
    fpxx->zfbz = atoi(szTmp);
    // blueFpdm
    if (GetAvailImportData(root, "blueFpdm", fpxx->blueFpdm, sizeof(fpxx->blueFpdm), true, NULL) <
        0) {
        _WriteLog(LL_FATAL, "JSON Parse blueFpdm failed\n");
        i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        goto JSon2FPXX_Err;
    }
    // blueFphm
    if (GetAvailImportData(root, "blueFphm", fpxx->blueFphm, sizeof(fpxx->blueFphm), true, NULL) <
        0) {
        _WriteLog(LL_FATAL, "JSON Parse blueFphm failed\n");
        i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        goto JSon2FPXX_Err;
    }
    // jshj
    if (GetAvailImportData(root, "jshj", fpxx->jshj, sizeof(fpxx->jshj), true, NULL) < 0) {
        _WriteLog(LL_FATAL, "JSON Parse jshj failed\n");
        i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        goto JSon2FPXX_Err;
    }
    // hsjbz
    if (GetAvailImportData(root, "hsjbz", fpxx->hsjbz, sizeof(fpxx->hsjbz), true, NULL) < 0) {
        _WriteLog(LL_FATAL, "JSON Parse hsjbz failed\n");
        i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        goto JSon2FPXX_Err;
    }
    // zyspsmmc
    if (GetAvailImportData(root, "zyspsmmc", fpxx->zyspsmmc, sizeof(fpxx->zyspsmmc), true, NULL) <
        0) {
        _WriteLog(LL_FATAL, "JSON Parse zyspsmmc failed\n");
        i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        goto JSon2FPXX_Err;
    }
    // spsl
    if (GetAvailImportData(root, "spsl", szTmp, sizeof(szTmp), true, NULL) < 0) {
        _WriteLog(LL_FATAL, "JSON Parse spsl failed\n");
        i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        goto JSon2FPXX_Err;
    }
    fpxx->spsl = atoi(szTmp);
    // fpqqlsh
    if (GetAvailImportData(root, "fpqqlsh", fpxx->fpqqlsh, sizeof(fpxx->fpqqlsh), true, NULL) < 0) {
        _WriteLog(LL_FATAL, "JSON Parse fpqqlsh failed\n");
        i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        goto JSon2FPXX_Err;
    }
    // pubcode
    if (GetAvailImportData(root, "pubcode", fpxx->pubcode, sizeof(fpxx->pubcode), false, NULL) <
        0) {
        _WriteLog(LL_FATAL, "JSON Parse pubcode failed\n");
        i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        goto JSon2FPXX_Err;
    }
    // kplx
    // if (GetAvailImportData(root, "kplx", fpxx->kplx, sizeof(fpxx->kplx), true) < 0) {
    //     _WriteLog(LL_FATAL, "JSON Parse kplx failed\n");
    //     i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
    //     goto JSon2FPXX_Err;
    // }
    // yysbz
    if (GetAvailImportData(root, "yysbz", fpxx->yysbz, sizeof(fpxx->yysbz), true, NULL) < 0) {
        _WriteLog(LL_FATAL, "JSON Parse yysbz failed\n");
        i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        goto JSon2FPXX_Err;
    }
    // zfsj
    if (GetAvailImportData(root, "zfsj", fpxx->zfsj, sizeof(fpxx->zfsj), true, NULL) < 0) {
        _WriteLog(LL_FATAL, "JSON Parse zfsj failed\n");
        i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        goto JSon2FPXX_Err;
    }
    // kpjh
    if (GetAvailImportData(root, "kpjh", szTmp, sizeof(szTmp), true, NULL) < 0) {
        _WriteLog(LL_FATAL, "JSON Parse kpjh failed\n");
        i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        goto JSon2FPXX_Err;
    }
    fpxx->kpjh = atoi(szTmp);
    // slxlh
    if (GetAvailImportData(root, "slxlh", fpxx->slxlh, sizeof(fpxx->slxlh), true, NULL) < 0) {
        _WriteLog(LL_FATAL, "JSON Parse slxlh failed\n");
        i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        goto JSon2FPXX_Err;
    }
    // dzsyh
    if (GetAvailImportData(root, "dzsyh", szTmp, sizeof(szTmp), true, NULL) < 0) {
        _WriteLog(LL_FATAL, "JSON Parse dzsyh failed\n");
        i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        goto JSon2FPXX_Err;
    }
    fpxx->dzsyh = atoi(szTmp);
    //商品明细
    jsp_ArraySpxx = cJSON_GetObjectItem(root, "fpmx");
    if (!jsp_ArraySpxx) {
        _WriteLog(LL_FATAL, "JSON Parse FPMX failed\n");
        i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
        goto JSon2FPXX_Err;
    }
    int i_SpxxNum = cJSON_GetArraySize(jsp_ArraySpxx);
    for (i = 0; i < i_SpxxNum; i++) {
        stp_Spxx = malloc(sizeof(struct Spxx));
        if (!stp_Spxx) {
            _WriteLog(LL_FATAL, "malloc struct Spxx failed\n");
            i_ErrNum = DF_TAX_ERR_CODE_SYSTEM_MEM_INSUFFICIENT;
            goto JSon2FPXX_Err;
        }
        memset(stp_Spxx, 0, sizeof(struct Spxx));
        cJSON *cjson_tmp = cJSON_GetArrayItem(jsp_ArraySpxx, i);
        if (!cjson_tmp) {
            _WriteLog(LL_FATAL, "JSON Parse cjson_tmp failed\n");
            if (stp_Spxx)
                free(stp_Spxx);
            i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
            goto JSon2FPXX_Err;
        }
        // spmc
        if (GetAvailImportData(cjson_tmp, "spmc", stp_Spxx->spmc, sizeof(stp_Spxx->spmc), true,
                               NULL) < 0) {
            _WriteLog(LL_FATAL, "JSON Parse spmc failed\n");
            if (stp_Spxx)
                free(stp_Spxx);
            i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
            goto JSon2FPXX_Err;
        }
        // spbh
        if (GetAvailImportData(cjson_tmp, "spbh", stp_Spxx->spbh, sizeof(stp_Spxx->spbh), true,
                               NULL) < 0) {
            _WriteLog(LL_FATAL, "JSON Parse spbh failed\n");
            if (stp_Spxx)
                free(stp_Spxx);
            i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
            goto JSon2FPXX_Err;
        }
        // spsm
        if (GetAvailImportData(cjson_tmp, "spsm", stp_Spxx->spsm, sizeof(stp_Spxx->spsm), true,
                               NULL) < 0) {
            _WriteLog(LL_FATAL, "JSON Parse spsm failed\n");
            if (stp_Spxx)
                free(stp_Spxx);
            i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
            goto JSon2FPXX_Err;
        }
        // ggxh
        if (GetAvailImportData(cjson_tmp, "ggxh", stp_Spxx->ggxh, sizeof(stp_Spxx->ggxh), true,
                               NULL) < 0) {
            _WriteLog(LL_FATAL, "JSON Parse ggxh failed\n");
            if (stp_Spxx)
                free(stp_Spxx);
            i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
            goto JSon2FPXX_Err;
        }
        // jldw
        if (GetAvailImportData(cjson_tmp, "jldw", stp_Spxx->jldw, sizeof(stp_Spxx->jldw), true,
                               NULL) < 0) {
            _WriteLog(LL_FATAL, "JSON Parse jldw failed\n");
            if (stp_Spxx)
                free(stp_Spxx);
            i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
            goto JSon2FPXX_Err;
        }
        // dj
        if (GetAvailImportData(cjson_tmp, "dj", stp_Spxx->dj, sizeof(stp_Spxx->dj), true, NULL) <
            0) {
            _WriteLog(LL_FATAL, "JSON Parse dj failed\n");
            if (stp_Spxx)
                free(stp_Spxx);
            i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
            goto JSon2FPXX_Err;
        }
        // sl
        if (GetAvailImportData(cjson_tmp, "sl", stp_Spxx->sl, sizeof(stp_Spxx->sl), true, NULL) <
            0) {
            _WriteLog(LL_FATAL, "JSON Parse sl failed\n");
            if (stp_Spxx)
                free(stp_Spxx);
            i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
            goto JSon2FPXX_Err;
        }
        // je
        if (GetAvailImportData(cjson_tmp, "je", stp_Spxx->je, sizeof(stp_Spxx->je), true, NULL) <
            0) {
            _WriteLog(LL_FATAL, "JSON Parse je failed\n");
            if (stp_Spxx)
                free(stp_Spxx);
            i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
            goto JSon2FPXX_Err;
        }
        // slv
        if (GetAvailImportData(cjson_tmp, "slv", stp_Spxx->slv, sizeof(stp_Spxx->slv), true, NULL) <
            0) {
            _WriteLog(LL_FATAL, "JSON Parse slv failed\n");
            if (stp_Spxx)
                free(stp_Spxx);
            i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
            goto JSon2FPXX_Err;
        }
        // se
        if (GetAvailImportData(cjson_tmp, "se", stp_Spxx->se, sizeof(stp_Spxx->se), true, NULL) <
            0) {
            _WriteLog(LL_FATAL, "JSON Parse se failed\n");
            if (stp_Spxx)
                free(stp_Spxx);
            i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
            goto JSon2FPXX_Err;
        }
        // fphxz
        if (GetAvailImportData(cjson_tmp, "fphxz", stp_Spxx->fphxz, sizeof(stp_Spxx->fphxz), true,
                               NULL) < 0) {
            _WriteLog(LL_FATAL, "JSON Parse fphxz failed\n");
            if (stp_Spxx)
                free(stp_Spxx);
            i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
            goto JSon2FPXX_Err;
        }
        // hsjbz
        if (GetAvailImportData(cjson_tmp, "hsjbz", stp_Spxx->hsjbz, sizeof(stp_Spxx->hsjbz), true,
                               NULL) < 0) {
            _WriteLog(LL_FATAL, "JSON Parse hsjbz failed\n");
            if (stp_Spxx)
                free(stp_Spxx);
            i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
            goto JSon2FPXX_Err;
        }
        // xh
        if (GetAvailImportData(cjson_tmp, "xh", stp_Spxx->xh, sizeof(stp_Spxx->xh), true, NULL) <
            0) {
            _WriteLog(LL_FATAL, "JSON Parse xh failed\n");
            if (stp_Spxx)
                free(stp_Spxx);
            i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
            goto JSon2FPXX_Err;
        }
        // xsyh
        if (GetAvailImportData(cjson_tmp, "yhzcbs", stp_Spxx->xsyh, sizeof(stp_Spxx->xsyh), true,
                               NULL) < 0) {
            _WriteLog(LL_FATAL, "JSON Parse yhzcbs failed\n");
            if (stp_Spxx)
                free(stp_Spxx);
            i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
            goto JSon2FPXX_Err;
        }
        // yhsm
        if (GetAvailImportData(cjson_tmp, "zzstsgl", stp_Spxx->yhsm, sizeof(stp_Spxx->yhsm), true,
                               NULL) < 0) {
            _WriteLog(LL_FATAL, "JSON Parse yhsm failed\n");
            if (stp_Spxx)
                free(stp_Spxx);
            i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
            goto JSon2FPXX_Err;
        }
        // lslvbs
        if (GetAvailImportData(cjson_tmp, "lslvbs", stp_Spxx->lslvbs, sizeof(stp_Spxx->lslvbs),
                               true, NULL) < 0) {
            _WriteLog(LL_FATAL, "JSON Parse lslvbs failed\n");
            if (stp_Spxx)
                free(stp_Spxx);
            i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
            goto JSon2FPXX_Err;
        }
        // kce
        if (GetAvailImportData(cjson_tmp, "kce", stp_Spxx->kce, sizeof(stp_Spxx->kce), true, NULL) <
            0) {
            _WriteLog(LL_FATAL, "JSON Parse kce failed\n");
            if (stp_Spxx)
                free(stp_Spxx);
            i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
            goto JSon2FPXX_Err;
        }
        // spsmmc
        if (GetAvailImportData(cjson_tmp, "spsmmc", stp_Spxx->spsmmc, sizeof(stp_Spxx->spsmmc),
                               true, NULL) < 0) {
            _WriteLog(LL_FATAL, "JSON Parse spsmmc failed\n");
            if (stp_Spxx)
                free(stp_Spxx);
            i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
            goto JSon2FPXX_Err;
        }
        //插入结构体链表
        if (InsertMxxx(fpxx, stp_Spxx) != 0) {
            if (stp_Spxx)
                free(stp_Spxx);
            i_ErrNum = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
            goto JSon2FPXX_Err;
        }
    }
    cJSON_Delete(root);
    return 0;
JSon2FPXX_Err:
    _WriteLog(LL_FATAL, "Upateload JSON Parse Err\n");
    cJSON_Delete(root);
    return i_ErrNum;
}

int fpxx_to_json_rednum_data_v102(HFPXX stp_fpxx, uint8 **fpxx_json, int sfxqqd)
{
    int i;
    char tmp_s[200] = {0};
    int Spxx_num;
    cJSON *dir1;
    cJSON *json = cJSON_CreateObject();
    cJSON_AddStringToObject(json, "fpsjbbh", "V1.0.2");
    cJSON_AddStringToObject(json, "fpqqlsh", "");
    memset(tmp_s, 0, sizeof(tmp_s));
    sprintf(tmp_s, "%d", stp_fpxx->hDev->bDeviceType);
    cJSON_AddStringToObject(json, "splx", tmp_s);
    cJSON_AddStringToObject(json, "hzxxbsqlsh", stp_fpxx->redNum_serial);
    cJSON_AddStringToObject(json, "hzxxbsqztdm", stp_fpxx->redNum_ztdm);
    cJSON_AddStringToObject(json, "hzxxbsqztmc", stp_fpxx->redNum_ztmc);
    cJSON_AddStringToObject(json, "hzxxbbh", stp_fpxx->redNum);

    if (strcmp(stp_fpxx->redNum_sqsm, "Y") == 0)
        cJSON_AddStringToObject(json, "hzxxbsqf", "1");  //购方
    else if (strcmp(stp_fpxx->redNum_sqsm, "N1") == 0)
        cJSON_AddStringToObject(json, "hzxxbsqf", "1");  //购方
    else if (strcmp(stp_fpxx->redNum_sqsm, "N5") == 0)
        cJSON_AddStringToObject(json, "hzxxbsqf", "0");  //销方
    else
        cJSON_AddStringToObject(json, "hzxxbsqf", "");  //未知

    cJSON_AddStringToObject(json, "reqmemo", stp_fpxx->redNum_sqsm);

    cJSON_AddStringToObject(json, "spbh", stp_fpxx->hDev->szDeviceID);
    memset(tmp_s, 0, sizeof(tmp_s));
    sprintf(tmp_s, "%d", stp_fpxx->kpjh);
    cJSON_AddStringToObject(json, "kpjh", tmp_s);
    memset(tmp_s, 0, sizeof(tmp_s));
    sprintf(tmp_s, "%d", stp_fpxx->zfbz);
    cJSON_AddStringToObject(json, "zfbz", tmp_s);
    memset(tmp_s, 0, sizeof(tmp_s));
    sprintf(tmp_s, "%d", stp_fpxx->bIsUpload);
    cJSON_AddStringToObject(json, "sbbz", tmp_s);
    cJSON_AddStringToObject(json, "fpdm", stp_fpxx->fpdm);
    cJSON_AddStringToObject(json, "fphm", stp_fpxx->fphm);
    cJSON_AddStringToObject(json, "kpsj", stp_fpxx->kpsj_F2);
    memset(tmp_s, 0, sizeof(tmp_s));
    sprintf(tmp_s, "%d", stp_fpxx->isRed);
    cJSON_AddStringToObject(json, "kplx", tmp_s);
    memset(tmp_s, 0, sizeof(tmp_s));
    sprintf(tmp_s, "%03d", stp_fpxx->fplx);
    cJSON_AddStringToObject(json, "fpzls", tmp_s);
    memset(tmp_s, 0, sizeof(tmp_s));
    sprintf(tmp_s, "%d", stp_fpxx->zyfpLx);
    cJSON_AddStringToObject(json, "tspz", tmp_s);
    cJSON_AddStringToObject(json, "bmbbbh", stp_fpxx->bmbbbh);
    cJSON_AddStringToObject(json, "gfmc", stp_fpxx->gfmc);
    cJSON_AddStringToObject(json, "gfsh", stp_fpxx->gfsh);
    cJSON_AddStringToObject(json, "gfdzdh", stp_fpxx->gfdzdh);
    cJSON_AddStringToObject(json, "gfyhzh", stp_fpxx->gfyhzh);
    cJSON_AddStringToObject(json, "xfmc", stp_fpxx->xfmc);
    cJSON_AddStringToObject(json, "xfsh", stp_fpxx->xfsh);
    cJSON_AddStringToObject(json, "xfdzdh", stp_fpxx->xfdzdh);
    cJSON_AddStringToObject(json, "xfyhzh", stp_fpxx->xfyhzh);
    cJSON_AddStringToObject(json, "hsjbz", stp_fpxx->hsjbz);
    cJSON_AddStringToObject(json, "hjje", stp_fpxx->je);
    cJSON_AddStringToObject(json, "hjse", stp_fpxx->se);
    cJSON_AddStringToObject(json, "jshj", stp_fpxx->jshj);
    cJSON_AddStringToObject(json, "qdbz", stp_fpxx->qdbj);
    char bz_s[5120] = {0};
    sprintf(bz_s, "%s", stp_fpxx->bz);
    str_replace(bz_s, "\r\n", "<br/>");
    cJSON_AddStringToObject(json, "bz", bz_s);
    cJSON_AddStringToObject(json, "skr", stp_fpxx->skr);
    cJSON_AddStringToObject(json, "fhr", stp_fpxx->fhr);
    cJSON_AddStringToObject(json, "kpr", stp_fpxx->kpr);
    cJSON_AddStringToObject(json, "zyspsmmc", stp_fpxx->zyspsmmc);
    cJSON_AddStringToObject(json, "zyspmc", stp_fpxx->zyspmc);

    int tmp_time[20] = {0};
    sscanf(stp_fpxx->kpsj_F2, "%02x%02x-%02x-%02x %02x:%02x:%02x", &tmp_time[0], &tmp_time[1],
           &tmp_time[2], &tmp_time[3], &tmp_time[4], &tmp_time[5], &tmp_time[6]);
    sprintf(stp_fpxx->ssyf, "%02x%02x%02x", tmp_time[0], tmp_time[1], tmp_time[2]);


    cJSON_AddStringToObject(json, "ssyf", stp_fpxx->ssyf);
    cJSON_AddStringToObject(json, "tzdbh", stp_fpxx->redNum);
    cJSON_AddStringToObject(json, "yfpdm", stp_fpxx->blueFpdm);
    cJSON_AddStringToObject(json, "yfphm", stp_fpxx->blueFphm);
    // cJSON_AddStringToObject(json, "zfrq", stp_fpxx->zfrq);
    cJSON_AddStringToObject(json, "zfsj", stp_fpxx->zfsj_F2);
    cJSON_AddStringToObject(json, "jym", stp_fpxx->jym);
    cJSON_AddStringToObject(json, "mw", stp_fpxx->mw);

    //_WriteLog(LL_FATAL,"发票信息组包完成\n");
    cJSON_AddItemToObject(json, "fpmx", dir1 = cJSON_CreateArray());
    struct Spxx *stp_Spxx = NULL;
    if (strcmp(stp_fpxx->qdbj, "Y") == 0) {
        stp_Spxx = stp_fpxx->stp_MxxxHead;
        Spxx_num = stp_fpxx->spsl + 1;
    } else {
        stp_Spxx = stp_fpxx->stp_MxxxHead->stp_next;
        Spxx_num = stp_fpxx->spsl;
    }
    //_WriteLog(LL_FATAL, "商品数量=%d\n", Spxx_num);
    for (i = 0; i < Spxx_num; i++) {
        //_WriteLog(LL_FATAL,"组包第%d行商品\n",i+1);
        if (stp_Spxx == NULL) {
            break;
        }
        cJSON *dir2;
        cJSON_AddItemToObject(dir1, "dira", dir2 = cJSON_CreateObject());
        if ((i == 0) && (strcmp(stp_fpxx->qdbj, "Y") == 0)) {
            cJSON_AddStringToObject(dir2, "spsmmc", "");
            cJSON_AddStringToObject(dir2, "spmc", stp_Spxx->spmc);
            cJSON_AddStringToObject(dir2, "spbh", stp_Spxx->spbh);
            cJSON_AddStringToObject(dir2, "fphxz", stp_Spxx->fphxz);
            cJSON_AddStringToObject(dir2, "hsjbz", stp_Spxx->hsjbz);
            cJSON_AddStringToObject(dir2, "dj", stp_Spxx->dj);
            cJSON_AddStringToObject(dir2, "jldw", stp_Spxx->jldw);
            cJSON_AddStringToObject(dir2, "ggxh", stp_Spxx->ggxh);
            cJSON_AddStringToObject(dir2, "se", stp_Spxx->se);
            cJSON_AddStringToObject(dir2, "je", stp_Spxx->je);
            cJSON_AddStringToObject(dir2, "slv", stp_Spxx->slv);
            cJSON_AddStringToObject(dir2, "sl", stp_Spxx->sl);
            cJSON_AddStringToObject(dir2, "yhzcbs", stp_Spxx->xsyh);
            cJSON_AddStringToObject(dir2, "slbs", stp_Spxx->lslvbs);
            // cJSON_AddStringToObject(dir2, "zzstsgl", "");
            cJSON_AddStringToObject(dir2, "zzstsgl", stp_Spxx->yhsm);
            cJSON_AddStringToObject(dir2, "zsfs", "0");
            cJSON_AddStringToObject(dir2, "cezskce", stp_Spxx->kce);
            if (sfxqqd == 0)
                break;
        } else {
            // memset(spsmmc, 0, sizeof(spsmmc));
            // memset(spmc, 0, sizeof(spmc));
            // GetSpsmmcSpmc(stp_Spxx->spmc, spsmmc, spmc);
            cJSON_AddStringToObject(dir2, "spsmmc", stp_Spxx->spsmmc);
            cJSON_AddStringToObject(dir2, "spmc", stp_Spxx->spmc);
            cJSON_AddStringToObject(dir2, "spbh", stp_Spxx->spbh);
            cJSON_AddStringToObject(dir2, "fphxz", stp_Spxx->fphxz);
            cJSON_AddStringToObject(dir2, "hsjbz", stp_Spxx->hsjbz);
            cJSON_AddStringToObject(dir2, "dj", stp_Spxx->dj);
            cJSON_AddStringToObject(dir2, "jldw", stp_Spxx->jldw);
            cJSON_AddStringToObject(dir2, "ggxh", stp_Spxx->ggxh);
            cJSON_AddStringToObject(dir2, "se", stp_Spxx->se);
            cJSON_AddStringToObject(dir2, "je", stp_Spxx->je);
            cJSON_AddStringToObject(dir2, "slv", stp_Spxx->slv);
            cJSON_AddStringToObject(dir2, "sl", stp_Spxx->sl);
            cJSON_AddStringToObject(dir2, "yhzcbs", stp_Spxx->xsyh);
            cJSON_AddStringToObject(dir2, "slbs", stp_Spxx->lslvbs);
            // cJSON_AddStringToObject(dir2, "zzstsgl", "");
            cJSON_AddStringToObject(dir2, "zzstsgl", stp_Spxx->yhsm);
            cJSON_AddStringToObject(dir2, "zsfs", "0");
            cJSON_AddStringToObject(dir2, "cezskce", stp_Spxx->kce);
        }
        stp_Spxx = stp_Spxx->stp_next;
    }
    //_WriteLog(LL_FATAL,"商品信息组包完成\n");
    int g_len;
    char *json_buf;
    char *g_buf;
    char *base_buf;
    json_buf = cJSON_Print(json);
    g_len = strlen(json_buf) * 2;
    g_buf = malloc(g_len);
    memset(g_buf, 0, g_len);
    memcpy(g_buf, json_buf, strlen(json_buf));
    free(json_buf);
    // str_replace(g_buf, "\\", "\\\\");
    str_replace(g_buf, "\\\\", "[@*br/*@]");
    str_replace(g_buf, "[@*br/*@]", "\\");
    //_WriteLog(LL_FATAL,"%s\n",g_buf);
#ifdef NEED_FP_DATA
    char name[200] = {0};
    memset(name, 0, sizeof(name));
    sprintf(name, "/mnt/masstore/get_rednum_kpjson%s_%s.txt", stp_fpxx->fpdm, stp_fpxx->fphm);
    write_file(name, (char *)g_buf, strlen(g_buf));

#endif

    base_buf = malloc(strlen(g_buf) * 2);
    memset(base_buf, 0, sizeof(strlen(g_buf) * 2));
    Base64_Encode(g_buf, strlen(g_buf), base_buf);
    //_WriteLog(LL_FATAL,"发票json数据，压缩前长度%d,压缩后长度%d\n", strlen(g_buf),
    // strlen(base_buf));
    *fpxx_json = (uint8 *)base_buf;
    free(g_buf);
    g_buf = NULL;
    cJSON_Delete(json);
    return 0;
}