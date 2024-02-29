/*****************************************************************************
File name:   aisino_call.c
Description: 用于aisino模块中接口调用示例
Author:      Zako
Version:     1.0
Date:        2020.04
History:
*****************************************************************************/
#include "aisino_call.h"
#define aisino_call_c

//税号读取
int my_aisino_read_taxID(HUSB hUSB)
{
    char szComNameChar[256] = "";
    char szTaxID[32] = "";
    if (AisinoReadTaxIDandCustomName(hUSB, szTaxID, szComNameChar, sizeof(szComNameChar)) < 0) {
        _WriteLog(LL_FATAL, "my_read_taxID failed");
        return -1;
    }
    _WriteLog(LL_DEBUG,
              "\n----------------------------------------------------------------\n"
              "Aisino found, Device TaxID:%s ComName:%s\n"
              "----------------------------------------------------------------\n",
              szTaxID, szComNameChar);
    return 0;
}

//读取盘号
int my_aisino_read_ID(HUSB hUSB)
{
    char szDevID[32] = "";
    if (ReadTaxDeviceID(hUSB, szDevID) < 0)
        return -1;
    _WriteLog(LL_DEBUG,
              "----------------------------------------------------------------\n"
              "Aisino found, Device RawSN:%s, ID:44%s\n"
              "----------------------------------------------------------------\n",
              szDevID, szDevID);
    return 0;
}

//读取指定月份的所有发票 szFormatMonth example: "201805","201611"
int my_aisino_read_invoice(HDEV hDev, char *szFormatMonth)
{
    int nInvoiceCount = GetInvMonthTotal(hDev->hUSB, szFormatMonth);
    _WriteLog(LL_INFO, "Get month:%s, total invoice:%d\n", szFormatMonth, nInvoiceCount);
    if (nInvoiceCount <= 0) {
        _WriteLog(LL_FATAL, "Get invoice count Ret:%d, may be not found invoice,exit prog\n",
                  nInvoiceCount);
        return -1;
    }
    _WriteLog(LL_INFO, "Begin output invoice information\n");
    _WriteLog(LL_DEBUG, "----------------------------------------------------------------\n");
    int nMaxInvoiceLen = DEF_MAX_FPBUF_LEN;
    uint8 *pInvoiceBuff = (uint8 *)malloc(nMaxInvoiceLen);
    char szOutputPath[512];
    int i = 0;
    for (i = 0; i < nInvoiceCount; i++) {
        memset(pInvoiceBuff, 0, nMaxInvoiceLen);
        uint8 aisino_fplx;
        int nLen =
            GetInvDetail(hDev, i, szFormatMonth, &pInvoiceBuff, &nMaxInvoiceLen, &aisino_fplx);
        if (nLen < 768)  //发票长度不可能低于768字节
            continue;
        sprintf(szOutputPath, "%s-dec-%d.bin", szFormatMonth, i);
        //  _WriteHexToDebugFile(szOutputPath, pInvoiceBuff, nLen);
    }
    free(pInvoiceBuff);
    _WriteLog(LL_DEBUG, "----------------------------------------------------------------\n");
    return nInvoiceCount;
}

//读取盘的区域环境信息/监控信息
int my_aisino_read_taxInfo(HUSB hUSB)
{
    StateInfo info;
    if (GetStateInfo(hUSB, &info)) {
        _WriteLog(LL_INFO, "GetStateInfo failed");
        return -1;
    }
    int nCount = info.abInvTypeList[0];
    int i = 0;
    for (i = 0; i < nCount; i++) {
        uint8 bInvType = info.abInvTypeList[1 + i];
        struct stFPXE *pxe = FindStateFPXEByType(bInvType, 8, &info);
        if (!pxe)
            continue;
        // dispaly
        _WriteLog(
            LL_INFO,
            "FPLX:%d szInvAmountLimit:%s szOffLineAmoutLimit:%s szOffLineAmoutLimit_Surplus:%s",
            bInvType, pxe->szInvAmountLimit, pxe->szOffLineAmoutLimit,
            pxe->szOffLineAmoutLimit_Surplus);
    }
    _WriteLog(LL_INFO, "GetStateInfo finish");
    return 0;
}

//读取发票张数
int my_aisino_read_InvNumber(HUSB hUSB)
{
    unsigned char szRetBuff[768] = "";
    unsigned char *data2 = calloc(1, 8192);
    if (!data2)
        return -1;
    int nRet = GetInvStockBuf(hUSB, szRetBuff, data2);
    free(data2);

    if (0 == nRet)
        _WriteLog(LL_INFO, "my_read_InvNumber, no invoice avail in device");
    if (nRet < 0)
        _WriteLog(LL_INFO, "my_read_InvNumber failed");
    else {
        //_WriteHexToDebugFile("invnum.bin", (uint8 *)szRetBuff, nRet);
        _WriteLog(LL_INFO, "my_read_InvNumber OK");
    }
    return 0;
}

//开票入口
int my_aisino_make_invoice(HDEV hDev)
{
    int nRet = 0;
    HFPXX fpxx = MallocFpxxLinkDev(hDev);
    if (fpxx == NULL) {
        nRet = -1;
        goto FreeAndExit;
    }
    fpxx->isHzxxb = false;
    if (ImportJSONToFpxx(fpxx, hDev) < 0) {
        _WriteLog(LL_FATAL, "ImportJSONToFpxx failed\r\n");
        nRet = -2;
        goto FreeAndExit;
    }
    if (MakeInvoiceMain(fpxx) < 0) {
        _WriteLog(LL_FATAL, "MakeInvoiceMain failed!\r\n");
        nRet = -3;
        goto FreeAndExit;
    }

    nRet = 0;
FreeAndExit:
    FreeFpxx(fpxx);
    return nRet;
}

//可以指定发票号码代码上传，但是必须是本月税期内；或者不指定，读取第一张离线票上传
//用法1, szFPDMIn = "", 读取第一张离线票上传
//用法2,传入FPDM FPHM, DZSYH(可选),指定发票上传
int my_aisino_upload_OfflineInv(HDEV hDev, char *szFPDMIn, char *szFPHMIn, int nDZSYH)
{
    char szFPDM[32] = "", szFPHM[24] = "", szT2[128] = "";
    int nDzssy = 0;
    ClearLastError(hDev->hUSB);
    if (0 == strlen(szFPDMIn)) {
        int nChildRet = GetFirstOfflineInv(hDev, szFPDM, szFPHM, &nDzssy, szT2);
        if (nChildRet < 0) {
            _WriteLog(LL_FATAL, "GetFirstOfflineInv failed, nRet:%d", nChildRet);
            return -1;
        } else if (nChildRet == 0) {
            _WriteLog(LL_INFO, "No offline invoice found");
            return 0;
        }
        _WriteLog(LL_INFO, "Get offline-invoice: FPDM:%s FPHM:%s", szFPDM, szFPHM);
    } else {
        strcpy(szFPDM, szFPDMIn);
        strcpy(szFPHM, szFPHMIn);
    }
    HFPXX fpxx = MallocFpxxLinkDev(hDev);
    int nRet = -1;
    while (1) {
        if (QueryInvInfo(hDev, szFPDM, szFPHM, nDZSYH, fpxx) < 0) {
            nRet = -2;
            break;
        }
        if (InvUp_UploadAndUpdate(hDev, fpxx, true, true) < 0) {
            nRet = -3;
            break;
        }
        nRet = 1;
        break;
    }
    FreeFpxx(fpxx);
    return nRet;
}

//作废发票-指定发票号码 代码
int my_aisino_waste_inv(HDEV hDev, char *szFPDM, char *szFPHM)
{
    _WriteLog(LL_INFO, "Try Waste invoice: FPDM:%s FPHM:%s", szFPDM, szFPHM);
    HFPXX fpxx = MallocFpxxLinkDev(hDev);
    int nRet = -1;
    int nChildRet = -1;
    while (1) {
        if ((nChildRet = QueryInvInfo(hDev, szFPDM, szFPHM, 0, fpxx)) < 0) {
            _WriteLog(LL_DEBUG, "WasteInvoice QueryInvInfo failed, nRet:%d", nChildRet);
            break;
        }
        if (fpxx->zfbz) {
            _WriteLog(LL_WARN, "Invoice has been wasted,FPDM:%s FPHM:%s", szFPDM, szFPHM);
            nRet = 0;
            break;
        }
        if (WasteInvoice(fpxx) < 0)
            break;
        nRet = 0;
        break;
    }
    if (!nRet)
        _WriteLog(LL_INFO, "Waste invoice successful: FPDM:%s FPHM:%s", szFPDM, szFPHM);
    else
        _WriteLog(LL_FATAL, "Waste invoice failed: FPDM:%s FPHM:%s", szFPDM, szFPHM);
    FreeFpxx(fpxx);
    return 0;
}

// 1= yccb 上报汇总, 0 = qk
int my_aisino_report_clear(HDEV hDev, uint8 bReportOrClear)
{
    ClearLastError(hDev->hUSB);
    if (hDev->bTrainFPDiskType != 0) {
        _WriteLog(LL_WARN, "Test device cannot been sumary or clear card");
        return -1;
    }
    int nTypeCount = hDev->abSupportInvtype[0];
    uint8 bZZSHasDo = false;
    int i = 0;
    for (i = 0; i < nTypeCount; i++) {  //专普票视为专票，电子机动车视为机动车
        uint8 bAisino_FPLX = hDev->abSupportInvtype[i + 1];
        if (FPLX_AISINO_DZFP != bAisino_FPLX && FPLX_AISINO_ZYFP != bAisino_FPLX &&
            FPLX_AISINO_PTFP != bAisino_FPLX) {
            _WriteLog(LL_WARN, "Unsupport invoice's aisino-fplx:%d", bAisino_FPLX);
            continue;
        }
        if (FPLX_AISINO_PTFP == bAisino_FPLX || FPLX_AISINO_ZYFP == bAisino_FPLX) {
            if (bZZSHasDo)  //专普票视为一种，只报送一次
                continue;
            bZZSHasDo = true;
        }
        _WriteLog(LL_INFO, "Begin %s, fplx:%d", bReportOrClear ? "RemoteReport" : "ClearCard",
                  bAisino_FPLX);
        if (bReportOrClear) {
            _WriteLog(LL_INFO, "Begin reporsummaryt");
            ReporSummarytMain(hDev, bAisino_FPLX);
        } else {
            _WriteLog(LL_INFO, "Begin clearcard");
            ClearCardMain(hDev, bAisino_FPLX);
        }
    }
    return 0;
}

// func0 ZYFP 红字表申请并获取红字信息表编号
// func1 ZYFP Fpxx结构体信息对比认证
int my_aisino_upload_check_redinvform(HDEV hDev)
{
    uint8 *inv_info;
    uint8 bFuncNo = 0;
    int nRet = -10;
    char szTZDBH[36] = "";
    HFPXX fpxx = MallocFpxxLinkDev(hDev);
    if (!fpxx) {
        nRet = -1;
        goto FreeAndExit;
    }
    //借用KP的导入
    fpxx->isHzxxb = true;
    if (ImportJSONToFpxx(fpxx, hDev) < 0) {
        _WriteLog(LL_FATAL, "ImportJSONToFpxx failed\r\n");
        nRet = -2;
        goto FreeAndExit;
    }
    if (0 == bFuncNo) {
        if (UploadTZDBH(fpxx, szTZDBH) < 0) {
            _WriteLog(LL_FATAL, "UploadTZDBH failed!\r\n");
            nRet = -3;
            goto FreeAndExit;
        }
        _WriteLog(LL_INFO, "UploadTZDBH successful,  red invoice tzdbh:%s", szTZDBH);
    } else {
        if (RedInvOnlineCheck_ZY(fpxx, &inv_info) < 0) {
            _WriteLog(LL_FATAL, "RedInvOnlineCheck_ZY failed!\r\n");
            nRet = -3;
            goto FreeAndExit;
        }
        _WriteLog(LL_INFO, "RedInvOnlineCheck_ZY successful");
    }

    nRet = 0;
FreeAndExit:
    if (fpxx)
        FreeFpxx(fpxx);
    return nRet;
}

// nDZSYH可选参数, 主要用于电子和普通发票红票开票前校验
int my_aisino_redinv_check(HDEV hDev, uint8 bAisinoFPLX, char *szLZFPDM, char *szLZFPHM,
                           uint32 nDZSYH)
{
    char szRepMsg[512] = "";
    int nRet = RedInvOnlineCheck_PTDZ(hDev, bAisinoFPLX, szLZFPDM, szLZFPHM, nDZSYH, szRepMsg,
                                      sizeof(szRepMsg));

    if (nRet < 0) {
        _WriteLog(LL_INFO, "FPDM:%s FPHM:%s, RedInvOnlineCheck failed,  msg:[%s]", szLZFPDM,
                  szLZFPHM, szRepMsg);
        return -1;
    }
    _WriteLog(LL_INFO, "FPDM:%s FPHM:%s, RedInvOnlineCheck successful, result:[%s]", szLZFPDM,
              szLZFPHM, szRepMsg);

    return 0;
}

//注意!!!这是单独接口,只能打开USB handle后单独调用.不能融入主流程，因为修改完之后需要重新连接设备
int my_aisino_change_certpassword(HUSB hUsb, char *szOldPassword, char *szNewPassword)
{
    int nChildRet = -1;
    char szTaxID[256];
    bool bCryptBigIO = 0;
    _WriteLog(LL_INFO, "Try to set new cert's password:%s", szNewPassword);
    int nNewPwdLen = strlen(szNewPassword);
    if (nNewPwdLen < 8 || nNewPwdLen > 16) {
        _WriteLog(LL_FATAL, "New cert password's length must [8,16]");
        return -1;
    }
    if (!strcmp(szNewPassword, DEF_DEVICE_PASSWORD_DEFAULT)) {
        _WriteLog(LL_FATAL, "Can not set new password equal with defalt password '88888888'");
        return -2;
    }
    if (CheckHeadTailSpace(szNewPassword)) {
        _WriteLog(LL_FATAL, "Head and tail cannot been 0x20");
        return -3;
    }
    nChildRet = ReadTaxDeviceID(hUsb, szTaxID);
    if (nChildRet > 0)
        bCryptBigIO = true;
    else if (0 == nChildRet)
        bCryptBigIO = false;
    else
        return -4;
    if (SignAPI_OpenDevice(hUsb, bCryptBigIO, szOldPassword) <= 0) {
        _WriteLog(LL_FATAL, "Crypt login failed, cannot modify new password");
        return -1;
    }
    if (SignAPI_ChangePassword(hUsb, bCryptBigIO, szOldPassword, szNewPassword) < 0) {
        _WriteLog(LL_FATAL, "Crypt login failed, cannot modify new password");
        return -1;
    }
    _WriteLog(LL_INFO, "Cert's password has been set,old:[%s] -> new:[%s]", szOldPassword,
              szNewPassword);
    return 0;
}

// https://bswj.jiangsu.chinatax.gov.cn:7001/preview.html?code=032001700211_89027948_20200821_E62AA970
int my_aisino_get_pubserviceurl(HDEV hDev, char *szFPDM, char *szFPHM, int nDZSYH)
{
    HFPXX fpxx = MallocFpxxLinkDev(hDev);
    int nRet = -1;
    struct UploadAddrModel model;
    memset(&model, 0, sizeof(struct UploadAddrModel));
    char szOutURL[512] = "", szKPDate[16] = "", szRandomCode[64];
    while (1) {
        if (QueryInvInfo(hDev, szFPDM, szFPHM, nDZSYH, fpxx) < 0) {
            _WriteLog(LL_WARN, "Can not found specify inovce, please input DZSYH to try again");
            nRet = -2;
            break;
        }
        if (GetTaServerURL(TAADDR_PLATFORM, hDev->szRegCode, &model) != RET_SUCCESS) {
            nRet = -3;
            break;
        }
        strncpy(szKPDate, fpxx->kpsj_standard, 8);
        char *p = strrchr(fpxx->pubcode, ';');
        if (!p) {
            _WriteLog(LL_WARN, "This invoice maybe has no pubserver's information");
            nRet = -4;
            break;
        }
        strcpy(szRandomCode, p + 1);
        sprintf(szOutURL, "%s/preview.html?code=%s_%s_%s_%s", model.szTaxAuthorityURL, fpxx->fpdm,
                fpxx->fphm, szKPDate, szRandomCode);
        nRet = 0;
        break;
    }
    FreeFpxx(fpxx);
    _WriteLog(LL_INFO, "Get pubserver's url %s, fpdm:%s fphm:%s url:%s",
              !nRet ? "successful" : "failed", szFPDM, szFPHM, szOutURL);

    return nRet;
}

//前几次成功率较高，后续操作成功几率较低。因此不要频繁切换状态，调用时稍注意
int my_aisino_switch_pubservice(HDEV hDev, bool bEnableOrDisable)
{
    //税盘时间20200101之前不支持
    char szSQXX[512] = "";
    if (!CheckDeviceSupportInvType(hDev->abSupportInvtype, FPLX_AISINO_DZFP)) {
        _WriteLog(LL_FATAL, "Pubservice switch need dzfp support");
        return -1;
    }
    _WriteLog(LL_INFO, "Try to %s aisino's pubservice", bEnableOrDisable ? "enable" : "disable");
    int nRet = SwitchPubservice(hDev, bEnableOrDisable, szSQXX);
    if (nRet < 0) {
        _WriteLog(LL_FATAL, "AisinoSwitchPubservice failed");
        return -2;
    } else if (!nRet) {
        _WriteLog(LL_WARN, "Pubservice has already enabled/disable.Nothing to do...");
        return 0;
    }
    nRet = UpdateCompanyInfo(hDev->hUSB, szSQXX);
    //无论成功与否都强制与税局同步
    if (SyncWithTA(hDev) < 0) {
        _WriteLog(LL_DEBUG, "Sync with TA failed");
    } else {
        _WriteLog(LL_DEBUG, "Sync with TA Successful");
        nRet = 0;
    }
    if (nRet < 0) {
        _WriteLog(LL_FATAL, "my_aisino_switch_pubservice failed");
        return -3;
    }
    _WriteLog(LL_INFO,
              "AisinoSwitchPubservice successful to change status to :%s, SQXX's length:%d",
              bEnableOrDisable ? "enable" : "disable", strlen(szSQXX));
    return 0;
}

// szFormatYearMonth=202106
int my_aisino_month_statistic(HDEV hDev, char *szFormatYearMonth)
{
    struct StMonthStatistics stStatisticsRet;
    if (AisinoGetMonthStatistic(hDev->hUSB, szFormatYearMonth, &stStatisticsRet) < 0)
        return -1;
    struct StMonthStatistics *nextNode = stStatisticsRet.next;
    while (nextNode) {
        struct StMonthStatistics *tmp = nextNode->next;
        free(nextNode);
        nextNode = tmp;
    }
    return 0;
}
static int check_fpxx_aisino(struct Fpxx* stp_fpxx, struct _plate_infos *plate_infos, char *errinfo)
{
	int i_spsmmclen = 0;

	//char* pc_Tmp = NULL;
	int inv_type;
	if (!stp_fpxx || !plate_infos)
	{
		sprintf(errinfo, "发票信息数据结构体为空");
		return DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
	}
	//int i_RateFlag = 0;
	struct Spxx *stp_spxx = stp_fpxx->stp_MxxxHead->stp_next;
	struct Spxx *stp_spxx_tmp = stp_fpxx->stp_MxxxHead->stp_next;
	int i_FplxIndex = 0;
	//发票类型

	inv_type = stp_fpxx->fplx;

	find_inv_type_mem((unsigned char)inv_type, &i_FplxIndex);
	if (i_FplxIndex < 0 || i_FplxIndex> 7)
	{
		sprintf(errinfo, "发票类型错误");
		return DF_TAX_ERR_CODE_UNSUPPORT_INV_TYPE;
	}
	//查看是否超过单张发票限额
	long double dzfpkpxe;//单张发票限额
	long double fpzje;//当前开票总金额
	dzfpkpxe = strtold((char *)plate_infos->invoice_type_infos[i_FplxIndex].monitor_info.dzkpxe,NULL);
	fpzje = strtold(stp_fpxx->je,NULL);
	//_WriteLog(LL_INFO, "单张发票限额%4.2Lf,当前开票金额%%4.2Lf\n", dzfpkpxe, fpzje);
	if (fpzje > dzfpkpxe)
	{
		sprintf(errinfo, "超过单张发票限额,单张发票限额%4.2Lf,当前开票金额%4.2Lf", dzfpkpxe, fpzje);
		_WriteLog(LL_INFO, "%s\n", errinfo);
		return DF_TAX_ERR_CODE_INV_DATA_DISACCORD;
	}
///////////
	if (!stp_fpxx->hDev->bTrainFPDiskType)
	{
		long double lxzssyje;//离线正数剩余金额
		lxzssyje = strtold((char *)plate_infos->invoice_type_infos[i_FplxIndex].monitor_info.lxzssyje, NULL);
		if (fpzje > lxzssyje)
		{
			sprintf(errinfo, "超过离线正数剩余金额,离线正数剩余金额%4.2Lf,当前开票金额%4.2Lf", lxzssyje, fpzje);
			_WriteLog(LL_INFO, "%s\n", errinfo);
			return DF_TAX_ERR_CODE_INV_DATA_DISACCORD;
		}

		long double lxzsljje;//离线正数累计金额
		lxzsljje = strtold((char *)plate_infos->invoice_type_infos[i_FplxIndex].monitor_info.lxzsljje, NULL);

		if (lxzsljje <= 0)
		{
			sprintf(errinfo, "离线正数累计金额为0,不允许开票");
			_WriteLog(LL_INFO, "%s\n", errinfo);
			return DF_TAX_ERR_CODE_INV_DATA_DISACCORD;
		}

		if (plate_infos->plate_tax_info.off_inv_num != 0)
		{
			if (plate_infos->invoice_type_infos[i_FplxIndex].monitor_info.lxkpsc == 0)
			{
				sprintf(errinfo, "离线时长为0，不允许开具多张离线发票,请先上传离线发票在进行开具");
				_WriteLog(LL_INFO, "%s\n", errinfo);
				return DF_TAX_ERR_CODE_INV_DATA_DISACCORD;
			}
		}
	}
	//if (memcmp(stp_fpxx->hDev->szDeviceTime, plate_infos->invoice_type_infos[i_FplxIndex].monitor_info.kpjzsj, 8) >= 0)
	//{
	//	sprintf(errinfo, "超过开票截止日期,无法开票,开票截止日期 = %s,当前税盘时间 = %s", plate_infos->invoice_type_infos[i_FplxIndex].monitor_info.kpjzsj, stp_fpxx->hDev->szDeviceTime);
	//	return DF_TAX_ERR_CODE_EXCEED_KP_END_DATE;
	//}

	if ((memcmp(stp_fpxx->hDev->szDeviceTime, plate_infos->invoice_type_infos[i_FplxIndex].monitor_info.zxbsrq, 6) > 0) && \
		(memcmp(stp_fpxx->hDev->szDeviceTime, plate_infos->invoice_type_infos[i_FplxIndex].monitor_info.kpjzsj, 6) >= 0))//当前月大于最新报税日期月,且要小于开票截止日期,需要报税处理
	{
		sprintf(errinfo, "未完成抄税,不允许开票,最新报税日期：%s,当前税盘时间：%s", plate_infos->invoice_type_infos[i_FplxIndex].monitor_info.zxbsrq, stp_fpxx->hDev->szDeviceTime);
		return DF_TAX_ERR_CODE_INV_DATA_DISACCORD;
	}

	


	//销方税号
	if ((strlen((const char *)plate_infos->plate_basic_info.ca_number) == 0) || (strcmp((const char *)stp_fpxx->xfsh, (const char *)plate_infos->plate_basic_info.ca_number) != 0) || (strlen(stp_fpxx->xfsh) < 15) || (strlen(stp_fpxx->xfsh) > 20)){
		sprintf(errinfo, "销方税号信息与税盘不匹配");
		logout(INFO, "TAXLIB", "发票开具", "系统税号%s,发票数据名称%s\r\n", plate_infos->plate_basic_info.ca_number, stp_fpxx->xfsh);
		return DF_TAX_ERR_CODE_INV_DATA_DISACCORD;
	}
	//销方名称
	if ((strlen((const char *)plate_infos->plate_basic_info.ca_name) == 0) || (strcmp((const char *)stp_fpxx->xfmc, (const char *)plate_infos->plate_basic_info.ca_name)) != 0){
		sprintf(errinfo, "销方名称信息与税盘不匹配");
		logout(INFO, "TAXLIB", "发票开具", "系统名称%s,发票数据名称%s\r\n", plate_infos->plate_basic_info.ca_name, stp_fpxx->xfmc);
		return DF_TAX_ERR_CODE_INV_DATA_DISACCORD;
	}
	//盘号
	if ((strlen((const char *)plate_infos->plate_basic_info.plate_num) == 0) || (strcmp((const char *)stp_fpxx->jqbh, (const char *)plate_infos->plate_basic_info.plate_num) != 0)){
		sprintf(errinfo, "销方盘号信息与税盘不匹配,系统盘号%s,发票数据盘号%s", plate_infos->plate_basic_info.plate_num, stp_fpxx->jqbh);
		logout(INFO, "TAXLIB", "发票开具", "系统盘号%s,发票数据盘号%s\r\n", plate_infos->plate_basic_info.plate_num, stp_fpxx->jqbh);
		return DF_TAX_ERR_CODE_INV_DATA_DISACCORD;
	}
	/*主要商品税目名称*/
	if (strcmp(stp_fpxx->zyspsmmc, stp_spxx->spsmmc) != 0){
		_WriteLog(LL_INFO, "check_fpxx_aisino test4\n");
		sprintf(errinfo, "主要商品税目名称需要第一行商品税目名称一致");
		return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
	}
	/*主要商品税目名称*/
	if (strcmp(stp_fpxx->zyspmc, stp_spxx->spmc) != 0){
		_WriteLog(LL_INFO, "check_fpxx_aisino test5\n");
		_WriteLog(LL_INFO, "主要商品名称：%s\n", stp_fpxx->zyspmc);
		_WriteLog(LL_INFO, "第一行商品名称：%s\n", stp_spxx->spmc);
		sprintf(errinfo, "主要商品名称需要第一行商品名称一致");
		return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
	}

	/*商品税目名称*/
	while (stp_spxx_tmp){

		if ((strcmp(stp_spxx_tmp->spmc, "(详见对应正数发票清单)") != 0) && (strcmp(stp_spxx_tmp->spmc, "详见对应正数发票及清单") != 0))
		{
			i_spsmmclen = strlen(stp_spxx_tmp->spsmmc);
			if (i_spsmmclen < 3)
			{
				_WriteLog(LL_INFO, "test i_spsmmclen < 3,税目：%s，名称：%s\n", stp_spxx_tmp->spsmmc, stp_spxx_tmp->spmc);
				sprintf(errinfo, "商品税目名称规则错误");
				return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
			}
			if (stp_spxx_tmp->spsmmc[0] != '*')
			{
				if ((strcmp(stp_spxx_tmp->spsmmc, "原价合计") == 0) || (strcmp(stp_spxx_tmp->spsmmc, "折扣额合计") == 0))
				{
					stp_spxx_tmp = stp_spxx_tmp->stp_next;
					continue;
				}


				_WriteLog(LL_INFO, "test stp_spxx_tmp->spsmmc[0] != *,税目名：%s\n", stp_spxx_tmp->spsmmc);
				sprintf(errinfo, "商品税目名称规则错误");
				return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
			}

			if (stp_spxx_tmp->spsmmc[strlen(stp_spxx_tmp->spsmmc) - 1] != '*')
			{
				_WriteLog(LL_INFO, "test stp_spxx_tmp->spsmmc[strlen(stp_spxx_tmp->spsmmc) - 1] != *\n");
				sprintf(errinfo, "商品税目名称规则错误");
				return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
			}
		}
		stp_spxx_tmp = stp_spxx_tmp->stp_next;
	}

	//税号检查
	if (!strlen(stp_fpxx->xfsh) || !strlen(stp_fpxx->gfmc) || !strlen(stp_fpxx->xfmc)) {
		_WriteLog(LL_INFO, "[-] Need mc and sh");
		sprintf(errinfo, "销购方信息填写有误");
		return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
	}
	// 非专用发票gf税号可为空  与 importinv.c重复
	//if (stp_fpxx->fplx_aisino == FPLX_AISINO_ZYFP) {
	//	if (!strlen(stp_fpxx->gfsh) || !strlen(stp_fpxx->gfdzdh) || !strlen(stp_fpxx->gfyhzh)) {
	//		_WriteLog(LL_INFO, "[-] Zyfp gf info error");
	//		sprintf(errinfo, "增值税专用发票需填写购方税号、购方地址电话、购方银行账号");
	//		return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
	//	}
	//}
	//else {
	//	if (!strlen(stp_fpxx->gfsh))
	//		strcpy(stp_fpxx->gfsh, "000000000000000");
	//}


	//wang 04 03 添加购方税号长度非19判断
	//税控盘和uk的纸普007(i_FplxIndex=3)和电普026(i_FplxIndex=6)
	/*if((i_FplxIndex == 3) || (i_FplxIndex == 6))
	{
		if ((strlen(stp_fpxx->gfsh) != 0) && (strlen(stp_fpxx->gfsh) != 15) && (strlen(stp_fpxx->gfsh) != 17) &&
			(strlen(stp_fpxx->gfsh) != 18) && (strlen(stp_fpxx->gfsh) != 19) && (strlen(stp_fpxx->gfsh) != 20)) {
			printf("[-] TaxID's gfsh length must be [15,20]");
			sprintf(errinfo, "购方税号长度有误非0、15、17、18、19、20位");
			return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
		}
	}
	else 
	{
		if (!strlen(stp_fpxx->gfsh) && (strlen(stp_fpxx->gfsh) != 15) && (strlen(stp_fpxx->gfsh) != 17) &&
			(strlen(stp_fpxx->gfsh) != 18) && (strlen(stp_fpxx->gfsh) != 20)) {
			_WriteLog(LL_INFO, "[-] TaxID's gfsh length must be [15,20]");
			sprintf(errinfo, "购方税号长度有误非15、17、18、20位");
			return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
		}
	}*/
	if (!strlen(stp_fpxx->gfsh) && (strlen(stp_fpxx->gfsh) != 15) && (strlen(stp_fpxx->gfsh) != 17) &&
		(strlen(stp_fpxx->gfsh) != 18) && (strlen(stp_fpxx->gfsh) != 20)) {
		_WriteLog(LL_INFO, "[-] TaxID's gfsh length must be [15,20]");
		sprintf(errinfo, "购方税号长度有误非15、17、18、20位");
		return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
	}
	if ((strlen(stp_fpxx->xfsh) != 15) && (strlen(stp_fpxx->xfsh) != 17) && (strlen(stp_fpxx->xfsh) != 18) &&
		(strlen(stp_fpxx->xfsh) != 20)) {
		_WriteLog(LL_INFO, "[-] TaxID's xfsh length must be [15,20]");
		sprintf(errinfo, "销方税号长度有误非15、17、18、20位");
		return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
	}
	////购方税号(购方税号!=销方税号)
	//if (strcmp((const char *)stp_fpxx->gfsh, (const char *)plate_infos->plate_basic_info.ca_number) == 0){
	//	_WriteLog(LL_INFO, "gfsh = %s 等于 xfsh=%s", stp_fpxx->gfsh, plate_infos->plate_basic_info.ca_number);
	//	sprintf(errinfo, "购方税号不得等于销方税号");
	//	return DF_TAX_ERR_CODE_INV_DATA_DISACCORD;
	//}
	if (stp_fpxx->spsl > 2000)
	{
		sprintf(errinfo, "商品行数不得大于2000行");
		_WriteLog(LL_INFO, "商品数量不能超过2000行\n");
		return DF_TAX_ERR_CODE_INV_DATA_TOO_LARGE;
	}
	//if (i_RateFlag != 0)
	//{
	//	_WriteLog(LL_INFO, "test i_RateFlag != 0\n");
	//	return DF_TAX_ERR_CODE_TAX_RATE_UNSUPPORTED;
	//}
	return 0;
}

//还需测试，无离线发票情况；整合代码上报
static int aisino_upload_firstOfflineInv(HDEV hDev, struct _offline_upload_result *offline_upload_result)
{
	char szFPDM[32] = { 0 }, szFPHM[24] = { 0 };
	char szKPSJ[20] = "";
	int dzsyh;
	int nChildRet = GetFirstOfflineInv(hDev, szFPDM, szFPHM, &dzsyh, szKPSJ);
	if (nChildRet < 0) {
		_WriteLog(LL_FATAL, "[+] GetFirstOfflineInv failed, nRet:%d", nChildRet);
		return -1;
	}
	else if (nChildRet == 0) {
		_WriteLog(LL_INFO, "[+] No offline invoice found");
		return 100;
	}
	// strcpy(szFPDM, "032001700311");
	// strcpy(szFPHM, "57075814");
	_WriteLog(LL_INFO, "[+] Getfirstoffline invoice: FPDM:%s FPHM:%s\n", szFPDM, szFPHM);
	//此处追加证书及口令认证
	//AddLoadAisinoCert(pDevInfo, pDevInfo->hUSB);
	FillDevInfoCert(hDev, 1);
	HFPXX fpxx = MallocFpxx();
	fpxx->hDev = hDev;
	int nRet = -1;
	while (1) {
		if (QueryInvInfo(hDev, szFPDM, szFPHM, dzsyh, fpxx) < 0)
		{
			FreeFpxx(fpxx);
			_WriteLog(LL_INFO, "发票获取失败\n");
			break;
		}

		//_WriteLog(LL_INFO, "离线发票获取成功记录json文件\n");
		//char *fpxx_json;
		//fpxx_to_json_base_fpsjbbh_v102(fpxx, &fpxx_json,DF_FPQD_Y,DF_FP_BASE);
		//free(fpxx_json);
		_WriteLog(LL_INFO, "开票时间：%s，发票代码：%s，发票号码：%s，发票类型代码：%03d\n", fpxx->kpsj_F2, fpxx->fpdm, fpxx->fphm, fpxx->fplx);
		memcpy((char *)offline_upload_result->kpsj, fpxx->kpsj_F2, 19);
		memcpy((char *)offline_upload_result->fpdm, fpxx->fpdm, 12);
		memcpy((char *)offline_upload_result->fphm, fpxx->fphm, 12);
		offline_upload_result->fplxdm = fpxx->fplx;
		offline_upload_result->fpdzsyh = fpxx->dzsyh;

		if (InvUp_UploadAndUpdate(hDev, fpxx, true, 1) < 0)
		{
			FreeFpxx(fpxx);
			_WriteLog(LL_INFO, "盘号：%s,发票上传失败,错误信息【%s】\n", hDev->szDeviceID, hDev->hUSB->szLastErrorDescription);
			if (strlen(hDev->hUSB->szLastErrorDescription) < sizeof(offline_upload_result->errinfo))
				strcpy((char *)offline_upload_result->errinfo, hDev->hUSB->szLastErrorDescription);
			
			break;
		}
		FreeFpxx(fpxx);
		
		//_WriteLog(LL_INFO, "发票上传完成,拷贝数据\n");
		//_WriteLog(LL_INFO, "开票时间：%s，发票代码：%s，发票号码：%s，发票类型代码：%03d\n", fpxx->kpsj_F2, fpxx->fpdm, fpxx->fphm, fpxx->fplx);
		//memcpy((char *)make_invoice_result->kpsj, fpxx->kpsj_F2,19);
		//memcpy((char *)make_invoice_result->fpdm, fpxx->fpdm,12);
		//memcpy((char *)make_invoice_result->fphm, fpxx->fphm,12);
		//make_invoice_result->fplxdm= fpxx->fplx;
		//_WriteLog(LL_INFO, "数据拷贝完成\n");
		nRet = 0;
		break;
	}
	if (nRet == 0)
	{
		HFPXX stp_fpxx = MallocFpxx();
		stp_fpxx->hDev = hDev;
		if (QueryInvInfo(hDev, (char *)offline_upload_result->fpdm, (char *)offline_upload_result->fphm, offline_upload_result->fpdzsyh, stp_fpxx) < 0)
		{
			FreeFpxx(stp_fpxx);
			_WriteLog(LL_WARN, "盘号:%s,发票上传后查询发票失败, fpdm:%s fphm:%s", stp_fpxx->hDev->szDeviceID, offline_upload_result->fpdm, offline_upload_result->fphm);
			return -3;
		}
		if (stp_fpxx->bIsUpload != 1)
		{
			logout(INFO, "TAXLIB", "发票上传", "盘号:%s,发票上传后查询发票上传状态失败, fpdm:%s fphm:%s\r\n", stp_fpxx->hDev->szDeviceID, offline_upload_result->fpdm, offline_upload_result->fphm);
		}
		offline_upload_result->fplxdm = stp_fpxx->fplx;
		offline_upload_result->zfbz = stp_fpxx->zfbz;
		memset(offline_upload_result->fpdm, 0, sizeof(offline_upload_result->fpdm));
		memset(offline_upload_result->fphm, 0, sizeof(offline_upload_result->fphm));
		memset(offline_upload_result->kpsj, 0, sizeof(offline_upload_result->kpsj));
		memset(offline_upload_result->hjje, 0, sizeof(offline_upload_result->hjje));
		memset(offline_upload_result->hjse, 0, sizeof(offline_upload_result->hjse));
		memset(offline_upload_result->jshj, 0, sizeof(offline_upload_result->jshj));
		memset(offline_upload_result->jym, 0, sizeof(offline_upload_result->jym));
		memset(offline_upload_result->mwq, 0, sizeof(offline_upload_result->mwq));
		memset(offline_upload_result->bz, 0, sizeof(offline_upload_result->bz));

		memcpy(offline_upload_result->fpdm, stp_fpxx->fpdm, strlen(stp_fpxx->fpdm));
		memcpy(offline_upload_result->fphm, stp_fpxx->fphm, strlen(stp_fpxx->fphm));
		memcpy(offline_upload_result->kpsj, stp_fpxx->kpsj_F2, strlen(stp_fpxx->kpsj_F2));
		memcpy(offline_upload_result->hjje, stp_fpxx->je, strlen(stp_fpxx->je));
		memcpy(offline_upload_result->hjse, stp_fpxx->se, strlen(stp_fpxx->se));
		memcpy(offline_upload_result->jshj, stp_fpxx->jshj, strlen(stp_fpxx->jshj));
		memcpy(offline_upload_result->jym, stp_fpxx->jym, strlen(stp_fpxx->jym));
		memcpy(offline_upload_result->mwq, stp_fpxx->mw, strlen(stp_fpxx->mw));
		memcpy(offline_upload_result->bz, stp_fpxx->bz, strlen(stp_fpxx->bz));
		fpxx_to_json_base_fpsjbbh_v102(stp_fpxx, &offline_upload_result->scfpsj, DF_FPQD_Y, DF_FP_JSON);
		FreeFpxx(stp_fpxx);
		//_WriteLog(LL_INFO, "发票上传成功 FPDM:%s FPHM:%s\n", szFPDM, szFPHM);
		logout(INFO, "TAXLIB", "发票上传", "盘号:%s,离线发票上传成功,发票代码号码：%s_%s\r\n", hDev->szDeviceID, offline_upload_result->fpdm, offline_upload_result->fphm);
	}
	
	return nRet;
}

//获取发票卷数据
static int invoices_coil_data_to_struct(unsigned char fplxdm, unsigned char aisino_fplxdm, unsigned char *data, unsigned char *data2, struct _plate_infos *plate_infos)
{
	int num;
	//int len;
	int i;
	int coil_count = 0;
	int inv_type;
	int mem_i;
	int dqfphm = 0;
	int zsyfpfs = 0;
	char fplbdm[20] = { 0 };
	char fplbdm_s[20] = { 0 };
	unsigned short year;
	unsigned short month;
	unsigned short day;
	//struct _invoice_num invoice_num;
	find_inv_type_mem(fplxdm, &mem_i);
	//_WriteLog(LL_INFO, "获取发票卷总数\n");
	num = data[1] * 0x100 + data[0];
	//_WriteLog(LL_INFO, "获取发票卷总数,num = %d \n", num);
	if (num == 0)
	{
		//_WriteLog(LL_INFO, "无发票卷\n");
		return 0;
	}
	if (num > 20)
		num = 20;
	int counts = 0;
	//int index = 0;
	for (i = 0; i < num; i++)
	{
		int num2 = i * 28 + 2;
		int num3 = i * 25 + 2;

		inv_type = data[num2 + 19];
		//_WriteLog(LL_INFO, "invoice_num.InvType = %d,fp_type = %d\n", invoice_num.InvType, fp_type);
		if (inv_type != aisino_fplxdm)
			continue;
		//_WriteLog(LL_INFO, "发票类型代码为%03d\n", fplxdm);

		plate_infos->invoice_type_infos[mem_i].invoice_coils_info.invoice_coil[coil_count].fpqshm = data[num2 + 3] * 0x1000000 + data[num2 + 2] * 0x10000 + data[num2 + 1] * 0x100 + data[num2];
		plate_infos->invoice_type_infos[mem_i].invoice_coils_info.invoice_coil[coil_count].fpsyfs = data[num2 + 24] * 0x10000 + data[num2 + 5] * 0x100 + data[num2 + 4];

		zsyfpfs += plate_infos->invoice_type_infos[mem_i].invoice_coils_info.invoice_coil[coil_count].fpsyfs;
		year = data[num2 + 13] * 0x100 + data[num2 + 12];
		month = data[num2 + 7] * 0x100 + data[num2 + 6];
		day = data[num2 + 9] * 0x100 + data[num2 + 8];
		sprintf((char *)plate_infos->invoice_type_infos[mem_i].invoice_coils_info.invoice_coil[coil_count].fpgmsj, "%04d%02d%02d", year, month, day);
		plate_infos->invoice_type_infos[mem_i].invoice_coils_info.invoice_coil[coil_count].fpgmsl = data[num2 + 24]*0x10000 + data[num2 + 26] * 0x100 + data[num2 + 25];



		plate_infos->invoice_type_infos[mem_i].invoice_coils_info.invoice_coil[coil_count].fpzzhm = plate_infos->invoice_type_infos[mem_i].invoice_coils_info.invoice_coil[coil_count].fpqshm \
			+ plate_infos->invoice_type_infos[mem_i].invoice_coils_info.invoice_coil[coil_count].fpsyfs - 1;
		plate_infos->invoice_type_infos[mem_i].invoice_coils_info.invoice_coil[coil_count].state = 1;

		memset(fplbdm, 0, sizeof(fplbdm));
		memcpy(fplbdm, data2 + num3 + 16, 5);
		//_WriteLog(LL_INFO, "发票类别号码\n");
		//print_array(fplbdm, 6);
		//bcd2asc((unsigned char *)fplbdm, plate_infos->invoice_type_infos[mem_i].invoice_coils_info.invoice_coil[coil_count].fplbdm, 5);

		GetInvTypeCode(1, (uint8_t *)fplbdm, (char *)plate_infos->invoice_type_infos[mem_i].invoice_coils_info.invoice_coil[coil_count].fplbdm);
		//_WriteLog(LL_INFO, "发票类别代码为：%s\n", plate_infos->invoice_type_infos[mem_i].invoice_coils_info.invoice_coil[coil_count].fplbdm);

		if (coil_count == 0)
		{
			dqfphm = plate_infos->invoice_type_infos[mem_i].invoice_coils_info.invoice_coil[coil_count].fpqshm;
			memset(fplbdm_s, 0, sizeof(fplbdm_s));
			sprintf(fplbdm_s, "%s", plate_infos->invoice_type_infos[mem_i].invoice_coils_info.invoice_coil[coil_count].fplbdm);
		}



		//_WriteLog(LL_INFO, "发票卷%d的发票类别代码为%s\n", coil_count, plate_infos->invoice_type_infos[mem_i].invoice_coils_info.invoice_coil[coil_count].fplbdm);
		//_WriteLog(LL_INFO, "发票卷%d的发票起始号码为%d\n", coil_count, plate_infos->invoice_type_infos[mem_i].invoice_coils_info.invoice_coil[coil_count].fpqshm);
		//_WriteLog(LL_INFO, "发票卷%d的发票剩余份数为%d\n", coil_count, plate_infos->invoice_type_infos[mem_i].invoice_coils_info.invoice_coil[coil_count].fpsyfs);
		//_WriteLog(LL_INFO, "发票卷%d的发票购买时间为%d\n", coil_count, plate_infos->invoice_type_infos[mem_i].invoice_coils_info.invoice_coil[coil_count].fpgmsl);
		//_WriteLog(LL_INFO, "发票卷%d的发票终止号码为%d\n", coil_count, plate_infos->invoice_type_infos[mem_i].invoice_coils_info.invoice_coil[coil_count].fpzzhm);
		coil_count += 1;
	}
	memcpy(plate_infos->invoice_type_infos[mem_i].invoice_coils_info.dqfpdm, fplbdm_s, strlen(fplbdm_s));
	plate_infos->invoice_type_infos[mem_i].invoice_coils_info.dqfphm = dqfphm;
	plate_infos->invoice_type_infos[mem_i].invoice_coils_info.zsyfpfs = zsyfpfs;
	if (coil_count > DF_MAX_INV_COILS)
		coil_count = DF_MAX_INV_COILS;
	plate_infos->invoice_type_infos[mem_i].invoice_coils_info.wsyfpjsl = coil_count;
	plate_infos->invoice_type_infos[mem_i].invoice_coils_info.state = 1;
	//_WriteLog(LL_INFO, "当前发票代码%s\n", plate_infos->invoice_type_infos[mem_i].invoice_coils_info.dqfpdm);
	//_WriteLog(LL_INFO, "当前发票号码%d\n", plate_infos->invoice_type_infos[mem_i].invoice_coils_info.dqfphm);
	//_WriteLog(LL_INFO, "发票总剩余份数%d\n", plate_infos->invoice_type_infos[mem_i].invoice_coils_info.zsyfpfs);
	//_WriteLog(LL_INFO, "剩余发票卷数量%d\n", plate_infos->invoice_type_infos[mem_i].invoice_coils_info.wsyfpjsl);

	return counts;
}

int GetStateInfo2(HUSB hUSB, StateInfo *pRetObj, uint8 *abRawData)
{
	memset(pRetObj, 0, sizeof(StateInfo));
	uint8 bTypeCount = 0, bStaticMaxTypeCount = STATEINFO_FPXE_MAXCOUNT, bInvType;
	uint8 abInvBuf[256];
	int i, nIndex;
	char szInvTotalAmount[64], szInvAmount[64];

	//if (GetStateInfoBuf(device, (char *)abRawData) < 0) {
	//	_WriteLog(LL_FATAL, "Get stateinfo buf failed");
	//	return -1;
	//}
	//_WriteHexToDebugFile("stateinfo.bin", abRawData, 1024);
	pRetObj->IsLockReached = abRawData[1];
	pRetObj->IsRepReached = abRawData[2];
	pRetObj->LockedDays = abRawData[8];
	pRetObj->ICCardNo = abRawData[9];

    //wang 05 04
    uint8 szRegCodepin[2];

	logout(INFO, "TAXLIB", "地区编号", "读出来地区编号为 %02x%02x%02x\r\n", abRawData[36], abRawData[37], abRawData[38]);
    
	//printf("yuan abRawData[36] = %x\r\n",abRawData[36]);
    //printf("pRetObj->szRegCode = %02x%02x%02x", abRawData[36], abRawData[37], abRawData[38]);

    if((abRawData[36] == 0x91) || (abRawData[36] == 0x99)){          //重庆，单独处理
        abRawData[36] = 0x50;
    }
    else{
        if((abRawData[36]&0x0f) > 0x08){
            szRegCodepin[0] = (abRawData[36]&0x0f)%8;
        }else{
            szRegCodepin[0] = (abRawData[36]&0x0f);
        }
        if((abRawData[36]&0xf0) > 0x80){
            szRegCodepin[1] = (abRawData[36]&0xf0)%0x80;
        }else{
            szRegCodepin[1] = abRawData[36]&0xf0;
        }
        if((szRegCodepin[0]!=0)||(szRegCodepin[1]!=0)){
            abRawData[36] = szRegCodepin[0] + szRegCodepin[1];
            printf("abRawData[36] = %x\r\n",abRawData[36]);
        }
    }

	sprintf(pRetObj->szRegCode, "%02x%02x%02x", abRawData[36], abRawData[37], abRawData[38]);
	logout(INFO, "TAXLIB", "地区编号", "解密后地区编号为 %02x%02x%02x\r\n", abRawData[36], abRawData[37], abRawData[38]);


	for (i = 0; i < bStaticMaxTypeCount; i++) {
		uint8 *pType = abRawData + 39 + i * 26;
		struct stFPXE *pxe = &(pRetObj->fpType[i]);
		pxe->bFPLX = 0xff;
		if (*pType == 0xff)
			continue;
		pRetObj->abInvTypeList[1 + bTypeCount] = *pType;
		pxe->bFPLX = *pType;
		bTypeCount++;
		memcpy(abInvBuf, abRawData + 39 + i * 26, 26);
		//逐个发票解析
		sprintf(pxe->szBsDeadline, "%02x%02x%02x%02x", abInvBuf[14], abInvBuf[15], abInvBuf[16],
			abInvBuf[17]);
		sprintf(pxe->szKpDeadline, "%02x%02x%02x%02x", abInvBuf[18], abInvBuf[19], abInvBuf[20],
			abInvBuf[21]);
		sprintf(pxe->szBsLastestDate, "%02x%02x%02x%02x", abInvBuf[8], abInvBuf[9], abInvBuf[10],
			abInvBuf[11]);
		sprintf(pxe->szBsBeginDate, "%02x%02x%02x%02x", abInvBuf[8], abInvBuf[9], abInvBuf[10],
			abInvBuf[11]);
	}
	pRetObj->abInvTypeList[0] = bTypeCount;

	uint64 u64Num = 0;
	for (i = 0; i < bStaticMaxTypeCount; i++) {
		nIndex = i * 20;
		bInvType = abRawData[256 + nIndex];
		//单张开票限额
		u64Num = *(uint64 *)(abRawData + 256 + nIndex + 1);
		if (u64Num == 0)
			continue;  //空白，未设置，跳入下一个

		struct stFPXE *pxe = FindStateFPXEByType(bInvType, bStaticMaxTypeCount, pRetObj);
		if (!pxe)
			continue;
		PriceInt2DoubleStr(u64Num, pxe->szInvAmountLimit);
		// 离线剩余金额
		u64Num = *(uint64 *)(abRawData + 256 + nIndex + 7);
		PriceInt2DoubleStr(u64Num, pxe->szMonthAmountLimit);
		// 离线金额
		u64Num = *(uint64 *)(abRawData + 256 + nIndex + 13);
		PriceInt2DoubleStr(u64Num, pxe->szOffLineAmoutLimit);
	}
	//票种数据限额校正
	char *pNewConfigStr = (char *)abRawData + 500;
	NewDataModify(pRetObj, pNewConfigStr, bStaticMaxTypeCount);
	//税盘API数据限额校正
	for (i = 0; i < bStaticMaxTypeCount; i++) {
		uint8 bInvType = *(uint8 *)(abRawData + 39 + i * 26);
		if (bInvType == 0xff)
			continue;
		struct stFPXE *pxe = FindStateFPXEByType(bInvType, bStaticMaxTypeCount, pRetObj);
		if (!pxe)
			continue;
		AisinoGetOfflineAmount(hUSB, bInvType, szInvTotalAmount, szInvAmount);
		strcpy(pxe->szOffLineAmoutLimit, szInvTotalAmount);
		double dSurplus = atof(szInvTotalAmount) - atof(szInvAmount);
		sprintf(pxe->szOffLineAmoutLimit_Surplus, "%0.2lf", dSurplus);
		// dispaly
		//_WriteLog(
		//    LL_DEBUG,
		//    "FPLX:%d szInvAmountLimit:%s szOffLineAmoutLimit:%s szOffLineAmoutLimit_Surplus:%s",
		//    bInvType, pxe->szInvAmountLimit, pxe->szOffLineAmoutLimit,
		//    pxe->szOffLineAmoutLimit_Surplus);
	}

	return 0;
}

int aisino_read_monitor_info(HUSB hUSB, HDEV hDev, struct _plate_infos *plate_infos)
{
	char data[1024] = "";
	char data2[1024] = "";
	int type_count;
	int i, j;
	int mem_i;
	char bInvType;
	char fp_type_tmp;
	int available_count = 0;
	char type_tmp[20] = { 0 };
	char t_tmp[5000] = { 0 };
	int result;
	result = GetStateInfoBuf(hUSB, data);
	if (result < 0)
	{
		return result;
	}
	memcpy(data2,data,sizeof(data));
	//printf_array(data, result);

	unsigned short MachineNumber;
	MachineNumber = (unsigned short)data[9];
	plate_infos->plate_basic_info.extension = MachineNumber;
	//_WriteLog(LL_INFO, "获取到的分机号为%d\n", plate_infos->plate_basic_info.extension);


	//printf_array(data,100);//不准确尤其重庆地区,例661507483134,读取为910910实际为500910

	//memset(plate_infos->plate_tax_info.area_code, 0, sizeof(plate_infos->plate_tax_info.area_code));
	//sprintf((char *)plate_infos->plate_tax_info.area_code, "%02x%02x%02x", (unsigned short)data[36], (unsigned short)data[37], (unsigned short)data[38]);

	type_tmp[0] = data[39];
	type_tmp[1] = data[39 + 1 * 26];
	type_tmp[2] = data[39 + 2 * 26];
	type_tmp[3] = data[39 + 3 * 26];
	type_tmp[4] = data[39 + 4 * 26];
	type_tmp[5] = data[39 + 5 * 26];
	type_tmp[6] = data[39 + 6 * 26];
	type_tmp[7] = data[39 + 7 * 26];
	//_WriteLog(LL_INFO, "获取到的发票种类信息\n");
	//print_array((char *)tmp, 8);

	for (type_count = 0; type_count < 8; type_count++)
	{
		if (type_tmp[type_count] == 0xff)
			continue;
		switch (type_tmp[type_count])
		{
		case 0:
			//_WriteLog(LL_INFO, "发票类型为：专票\n");
			bInvType = 4;
			plate_infos->plate_tax_info.inv_type[available_count] = bInvType;
			available_count += 1;
			break;
		case 12:
			//_WriteLog(LL_INFO, "发票类型为：机动车销售统一发票\n");
			bInvType = 5;
			plate_infos->plate_tax_info.inv_type[available_count] = bInvType;
			available_count += 1;
			break;
		case 42:
			//_WriteLog(LL_INFO, "发票类型为：二手车销售统一发票\n");
			bInvType = 6;
			plate_infos->plate_tax_info.inv_type[available_count] = bInvType;
			available_count += 1;
			break;
		case 2:
			//_WriteLog(LL_INFO, "发票类型为：普票\n");
			bInvType = 7;
			plate_infos->plate_tax_info.inv_type[available_count] = bInvType;
			available_count += 1;
			break;
		case 41:
			//_WriteLog(LL_INFO, "发票类型为：增值税普通发票(卷票)\n");
			bInvType = 25;
			plate_infos->plate_tax_info.inv_type[available_count] = bInvType;
			available_count += 1;
			break;
		case 51:
			//_WriteLog(LL_INFO, "发票类型为：增值税电子普通发票\n");
			bInvType = 26;
			plate_infos->plate_tax_info.inv_type[available_count] = bInvType;
			available_count += 1;
			break;
		case 52:
			//_WriteLog(LL_INFO, "发票类型为：增值税电子专用发票\n");
			bInvType = 28;
			plate_infos->plate_tax_info.inv_type[available_count] = bInvType;
			available_count += 1;
			break;
		default:
			//_WriteLog(LL_INFO, "不支持的发票类型\n");
			continue;
		}
	}
	plate_infos->plate_tax_info.inv_type_num = available_count;
	//_WriteLog(LL_INFO, "获取区域代码:%s\n", plate_infos->plate_tax_info.area_code);


	for (i = 0; i < 8; i++)
	{
		memset(t_tmp, 0, sizeof(t_tmp));
		//_WriteLog(LL_INFO, "将票种数据单独拷贝至临时内存\n");
		memcpy(t_tmp, data + 39 + i * 26, 26);
		fp_type_tmp = t_tmp[0];
		switch (fp_type_tmp)
		{
		case 0:
			//_WriteLog(LL_INFO, "发票类型为：专票\n");
			bInvType = 4;
			mem_i = 0;
			break;
		case 12:
			//_WriteLog(LL_INFO, "发票类型为：机动车销售统一发票\n");
			bInvType = 5;
			mem_i = 1;
			break;
		case 42:
			//_WriteLog(LL_INFO, "发票类型为：二手车销售统一发票\n");
			bInvType = 6;
			mem_i = 2;
			break;
		case 2:
			//_WriteLog(LL_INFO, "发票类型为：普票\n");
			bInvType = 7;
			mem_i = 3;
			break;
		case 41:
			//_WriteLog(LL_INFO, "发票类型为：增值税普通发票(卷票)\n");
			bInvType = 25;
			mem_i = 5;
			break;
		case 51:
			//_WriteLog(LL_INFO, "发票类型为：增值税电子普通发票\n");
			bInvType = 26;
			mem_i = 6;
			break;
		case 52:
			//_WriteLog(LL_INFO, "发票类型为：增值税电子专用发票\n");
			bInvType = 28;
			mem_i = 7;
			break;
		default:
			//_WriteLog(LL_INFO, "不支持的发票类型\n");
			continue;
		}
		find_inv_type_mem(bInvType, &mem_i);

		char bszzrq[20] = { 0 };
		char kpjzsj[20] = { 0 };
		char zxbsrq[20] = { 0 };
		char bsqsrq[20] = { 0 };
		char tmp_t[8] = { 0 };

		plate_infos->invoice_type_infos[mem_i].monitor_info.lxkpsc = hDev->nMaxOfflineHour;

		memset(bszzrq, 0, sizeof(bszzrq));
		sprintf(bszzrq, "%02x%02x%02x%02x", t_tmp[14], t_tmp[15], t_tmp[16], t_tmp[17]);
		asc2bcd((unsigned char *)bszzrq, (unsigned char *)tmp_t, 12);
		back_date((unsigned char *)tmp_t);
		memset(bszzrq, 0, sizeof(bszzrq));
		bcd2asc((unsigned char *)tmp_t, (unsigned char *)bszzrq, 6);
		memcpy(plate_infos->invoice_type_infos[mem_i].monitor_info.bszzrq, bszzrq, 8);
		//_WriteLog(LL_INFO, "报送终止日期：%s\n", plate_infos->invoice_type_infos[mem_i].monitor_info.bszzrq);


		memset(kpjzsj, 0, sizeof(kpjzsj));
		sprintf(kpjzsj, "%02x%02x%02x%02x", t_tmp[18], t_tmp[19], t_tmp[20], t_tmp[21]);
		asc2bcd((unsigned char *)kpjzsj, (unsigned char *)tmp_t, 12);
		back_date((unsigned char *)tmp_t);
		memset(kpjzsj, 0, sizeof(kpjzsj));
		bcd2asc((unsigned char *)tmp_t, (unsigned char *)kpjzsj, 6);
		memcpy(plate_infos->invoice_type_infos[mem_i].monitor_info.kpjzsj, kpjzsj, 8);
		//_WriteLog(LL_INFO, "开票截止时间：%s\n", plate_infos->invoice_type_infos[mem_i].monitor_info.kpjzsj);


		memset(zxbsrq, 0, sizeof(zxbsrq));
		sprintf(zxbsrq, "%02x%02x%02x%02x", t_tmp[8], t_tmp[9], t_tmp[10], t_tmp[11]);
		memcpy(plate_infos->invoice_type_infos[mem_i].monitor_info.zxbsrq, zxbsrq, 8);
		//_WriteLog(LL_INFO, "最新报税日期：%s\n", plate_infos->invoice_type_infos[mem_i].monitor_info.zxbsrq);


		memset(bsqsrq, 0, sizeof(bsqsrq));
		sprintf(bsqsrq, "%02x%02x%02x%02x", t_tmp[8], t_tmp[9], t_tmp[10], t_tmp[11]);
		memcpy(plate_infos->invoice_type_infos[mem_i].monitor_info.bsqsrq, bsqsrq, 8);
		//_WriteLog(LL_INFO, "报税起始日期：%s\n", plate_infos->invoice_type_infos[mem_i].monitor_info.bsqsrq);


		if ((memcmp(hDev->szDeviceTime, plate_infos->invoice_type_infos[mem_i].monitor_info.zxbsrq, 6) > 0) && \
			(memcmp(hDev->szDeviceTime, plate_infos->invoice_type_infos[mem_i].monitor_info.kpjzsj, 6) >= 0))//当前月大于最新报税日期月,且要小于开票截止日期,需要报税处理
		{
			sprintf((char *)plate_infos->invoice_type_infos[mem_i].monitor_info.cbqkzt, "0");
		}
		else
		{
			sprintf((char *)plate_infos->invoice_type_infos[mem_i].monitor_info.cbqkzt, "1");
			if (memcmp(hDev->szDeviceTime, plate_infos->invoice_type_infos[mem_i].monitor_info.kpjzsj, 6) >= 0)
			{
				sprintf((char *)plate_infos->invoice_type_infos[mem_i].monitor_info.cbqkzt, "1");
			}
			else
			{
				sprintf((char *)plate_infos->invoice_type_infos[mem_i].monitor_info.cbqkzt, "2");
			}
		}
		

		for (j = 0; j < 8; j++)
		{
			int nIndex = j * 20;
			unsigned long long int  num32 = 0;
			unsigned long long lii = 0;
			unsigned long long rlt = 0;
			num32 = (unsigned  char)data[256 + nIndex + 8] * 0x100000000000000 + (unsigned  char)data[256 + nIndex + 7] * 0x1000000000000 + \
				(unsigned  char)data[256 + nIndex + 6] * 0x10000000000 + (unsigned  char)data[256 + nIndex + 5] * 0x100000000 + \
				(unsigned  char)data[256 + nIndex + 4] * 0x1000000 + (unsigned  char)data[256 + nIndex + 3] * 0x10000 + \
				(unsigned  char)data[256 + nIndex + 2] * 0x100 + (unsigned  char)data[256 + nIndex + 1];
			if ((num32 == 0) || (data[256 + nIndex] != fp_type_tmp))
			{
				//_WriteLog(LL_INFO, "查询离线限额发票类型不匹配\n");
				continue;
			}

			lii = 281474976710656;
			rlt = num32 % lii;
			double InvAmountLimit = (double)rlt / 100;
			sprintf((char *)plate_infos->invoice_type_infos[mem_i].monitor_info.dzkpxe, "%4.2f", InvAmountLimit);



			num32 = (unsigned  char)data[256 + nIndex + 20] * 0x100000000000000 + (unsigned  char)data[256 + nIndex + 19] * 0x1000000000000 + \
				(unsigned  char)data[256 + nIndex + 18] * 0x10000000000 + (unsigned  char)data[256 + nIndex + 17] * 0x100000000 + \
				(unsigned  char)data[256 + nIndex + 16] * 0x1000000 + (unsigned  char)data[256 + nIndex + 15] * 0x10000 + \
				(unsigned  char)data[256 + nIndex + 14] * 0x100 + (unsigned  char)data[256 + nIndex + 13];

			rlt = num32 % lii;
			double OffLineAmoutLimit = (double)rlt / 100;
			sprintf((char *)plate_infos->invoice_type_infos[mem_i].monitor_info.lxzsljje, "%4.2f", OffLineAmoutLimit);
			sprintf((char *)plate_infos->invoice_type_infos[mem_i].monitor_info.lxzssyje, "%4.2f", OffLineAmoutLimit);
			break;
		}
		
		plate_infos->invoice_type_infos[mem_i].monitor_info.state = 1;

		plate_infos->invoice_type_infos[mem_i].state = 1;
		plate_infos->invoice_type_infos[mem_i].fplxdm = bInvType;
		//_WriteLog(LL_INFO, "单张发票开票限额：%s\n", plate_infos->invoice_type_infos[mem_i].monitor_info.dzkpxe);
		//_WriteLog(LL_INFO, "离线发票开票限额：%s\n", plate_infos->invoice_type_infos[mem_i].monitor_info.lxzsljje);

		//_WriteLog(LL_INFO, "结构体状态：%d\n", plate_infos->invoice_type_infos[mem_i].monitor_info.state);
	}

	StateInfo stateinfo;
	if (GetStateInfo2(hDev->hUSB, &stateinfo, (uint8 *)data2) < 0)
	{
		_WriteLog(LL_INFO, "GetStateInfo failed\n");
	}


	//_WriteLog(LL_INFO, "GetStateInfo 是否锁定 %d\n", stateinfo.IsLockReached);
	//_WriteLog(LL_INFO, "GetStateInfo 是否到报税期 %d\n", stateinfo.IsRepReached);
	//_WriteLog(LL_INFO, "GetStateInfo 锁定日期 %d\n", stateinfo.LockedDays);
	//_WriteLog(LL_INFO, "GetStateInfo 分机号码 %s\n", stateinfo.szRegCode);
	//_WriteLog(LL_INFO, "GetStateInfo 支持的发票种类 %d\n", stateinfo.abInvTypeList[0]);
	//_WriteLog(LL_INFO, "GetStateInfo 支持的发票种类信息 %s\n", &stateinfo.abInvTypeList[1]);
	for (i = 0; i < STATEINFO_FPXE_MAXCOUNT; i++)
	{
		uint8 aisino_fplx;
		if (plate_infos->invoice_type_infos[i].state != 1)
		{
			//_WriteLog(LL_INFO, "此盘不支持 %d票种\n", plate_infos->invoice_type_infos[i].fplxdm);
			continue;
		}
		CommonFPLX2AisinoFPLX(plate_infos->invoice_type_infos[i].fplxdm, &aisino_fplx);
		//_WriteLog(LL_INFO, "此盘支持发票种类 %d转化成航信发票类型为%d\n", plate_infos->invoice_type_infos[i].fplxdm, aisino_fplx);
		for (j = 0; j < STATEINFO_FPXE_MAXCOUNT; j++)
		{
			if (stateinfo.fpType[j].bFPLX != aisino_fplx)
			{
				continue;
			}
			memset(plate_infos->invoice_type_infos[i].monitor_info.lxzsljje, 0, sizeof(plate_infos->invoice_type_infos[i].monitor_info.lxzsljje));
			memset(plate_infos->invoice_type_infos[i].monitor_info.lxzssyje, 0, sizeof(plate_infos->invoice_type_infos[i].monitor_info.lxzssyje));
			//memset(plate_infos->invoice_type_infos[i].monitor_info.dzkpxe, 0, sizeof(plate_infos->invoice_type_infos[i].monitor_info.dzkpxe));
			strcpy((char *)plate_infos->invoice_type_infos[i].monitor_info.lxzsljje, stateinfo.fpType[j].szOffLineAmoutLimit);
			strcpy((char *)plate_infos->invoice_type_infos[i].monitor_info.lxzssyje, stateinfo.fpType[j].szOffLineAmoutLimit_Surplus);
			//strcpy((char *)plate_infos->invoice_type_infos[i].monitor_info.dzkpxe, stateinfo.fpType[j].szInvAmountLimit);

			//_WriteLog(LL_INFO, "发票种类 %d\n", stateinfo.fpType[j].bFPLX);
			//_WriteLog(LL_INFO, "离线金额 %s\n", stateinfo.fpType[j].szOffLineAmoutLimit);
			//_WriteLog(LL_INFO, "月开票限额 %s\n", stateinfo.fpType[j].szMonthAmountLimit);
			//_WriteLog(LL_INFO, "单张开票限额 %s\n", stateinfo.fpType[j].szInvAmountLimit);
			//_WriteLog(LL_INFO, "不知道什么限额 %s\n", stateinfo.fpType[j].szReturnAmountLimit);
			//_WriteLog(LL_INFO, "离线剩余金额 %s\n", stateinfo.fpType[j].szOffLineAmoutLimit_Surplus);
			//_WriteLog(LL_INFO, "报送终止日期 %s\n", stateinfo.fpType[j].szBsDeadline);
			//_WriteLog(LL_INFO, "开票截止时间 %s\n", stateinfo.fpType[j].szKpDeadline);
			//_WriteLog(LL_INFO, "最新报税日期 %s\n", stateinfo.fpType[j].szBsLastestDate);
			//_WriteLog(LL_INFO, "报税起始日期 %s\n", stateinfo.fpType[j].szBsBeginDate);

			//_WriteLog(LL_INFO, "\n");
		}



		
	}

	return 0;
}

int aisino_read_inv_coil_number(HUSB hUSB, struct _plate_infos *plate_infos)
{
	unsigned char data[7680] = { 0 };
	unsigned char data2[9600] = { 0 };
	//int len;
	int i;
	unsigned char aisino_fplxdm;
	//_WriteLog(LL_INFO, "aisino_read_inv_coil_number enter\n");
	int result = GetInvStockBuf(hUSB, data, data2);
	if (result < 0)
	{
		return result;
	}
	//_WriteLog(LL_INFO, "获取到的发票卷数据为：\n");
	//printf_array(data, result);

	for (i = 0; i < 8; i++)
	{
		if (plate_infos->plate_tax_info.inv_type[i] == 0)
			continue;
		CommonFPLX2AisinoFPLX(plate_infos->plate_tax_info.inv_type[i], &aisino_fplxdm);
		//_WriteLog(LL_INFO, "将发票类型代码%d转换成航信专用类型代码%d\n", plate_infos->plate_tax_info.inv_type[i], aisino_fplxdm);
		invoices_coil_data_to_struct(plate_infos->plate_tax_info.inv_type[i], aisino_fplxdm, data, data2, plate_infos);

	}
	return 0;
}

/*获取税盘时间*/
int aisino_read_tax_time(HDEV hDev, char* outtime)
{
	int result = -1;
	uint8_t stand_time[20] = { 0 };
	uint8_t bcd_time[20] = { 0 };
	ClearLastError(hDev->hUSB);
	result = AisinoGetDeivceTime(hDev->hUSB, (char *)stand_time);
	if (result < 0)
	{
		return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
	}
	asc2bcd(stand_time, bcd_time, strlen((char *)stand_time));
	sprintf(outtime, "%02x%02x-%02x-%02x %02x:%02x:%02x", bcd_time[0], bcd_time[1], bcd_time[2], bcd_time[3], bcd_time[4], bcd_time[5], bcd_time[6]);
	//_WriteLog(LL_INFO, "金税盘当前时间：%s\n", outtime);
	return 0;
}

int aisino_get_inv_details_mqtt(HDEV hDev, unsigned char *month, void *function, void *arg, char *errinfo, int old_new, unsigned short *start_stop, int sfxqqd)
{
	int max_send_len = 4 * 1024 * 1024;//缓存区，超过即发送
	int max_fp_len = 4 * 1024 * 1024;		//4M单张发票长度
	int max_buf_len = 8 * 1024 * 1024;//最大发送缓存区
	int nInvoiceCount = -1;					//单月发票数量
	uint8 *pInvoiceBuff;					//单张发票数据
	int i;
	long nLen;
	char *encode_data = NULL;
	int result;
	unsigned char *send_data;
	//int max_len = 1 * 1024 * 1024;//最大数据超过1M即回调发送
	int over_time = 5;//发票数据获取时间超过5秒即回调发送
	long begin_time;
	long now_time;
	long index = 0;
	long inv_len;
	int now_count = 0;

	void *hook_arg;
	int(*hook)(unsigned char *s_data, int total_num, int now_num, void *hook_arg, char *plate_num);
	ClearLastError(hDev->hUSB);
	hook = function;
	hook_arg = arg;
	send_data = (uint8 *)malloc(max_buf_len); //发票数据缓冲区10M
	memset(send_data, 0, max_buf_len);

	nInvoiceCount = GetInvMonthTotal(hDev->hUSB, (char *)month);
	//printf("[+] my_read_single_invoice Get month:%s, total invoice:%d\n", szFormatMonth, nInvoiceCount);
	if (nInvoiceCount <= 0)
	{
		_WriteLog(LL_INFO, "[-] my_read_single_invoice Get invoice count Ret:%d, may be not found invoice,exit prog\n", nInvoiceCount);
		free(send_data);
		return DF_TAX_ERR_CODE_MONTH_HAVE_NO_INVPICE;
	}


	pInvoiceBuff = (uint8 *)malloc(max_fp_len);

	//printf("[+] my_read_single_invoice Begin output invoice information\n");
	*start_stop = 1;
	begin_time = get_time_sec();
	for (i = 0; i < nInvoiceCount; i++)
	{
		if (*start_stop != 1)
		{
			_WriteLog(LL_INFO, "停止正在查询%s月发票详情动作\n", (char *)month);
			break;
		}
		memset(pInvoiceBuff, 0, max_fp_len);
		uint8 aisino_fplx;
		_WriteLog(LL_INFO, "读取第%d张发票二进制数据", i+1);
		nLen = GetInvDetail(hDev, i, (char *)month, &pInvoiceBuff, (int *)&max_fp_len, &aisino_fplx);
		if (nLen < 200) //发票长度不可能低于768字节,实际测试中发现测试盘空白废仅732个字节
		{
			_WriteLog(LL_INFO, "%s税盘处理第 %d 张发票数据,%d发票类型,错误代码：%d\n", hDev->szCompanyName, i + 1, aisino_fplx, nLen);
			if ((aisino_fplx == FPLX_AISINO_HYFP) || (aisino_fplx == FPLX_AISINO_JDCFP) || (aisino_fplx == FPLX_AISINO_JSFP) || (aisino_fplx == FPLX_AISINO_ESC) || (aisino_fplx == FPLX_AISINO_DZZP))
			{
				//valid_num -= 1;
				if (aisino_fplx == FPLX_AISINO_HYFP) {
					_WriteLog(LL_INFO, "%s税盘处理第 %d 张发票数据,货运发票\n", hDev->szCompanyName, i + 1);
				}
				else if (aisino_fplx == FPLX_AISINO_JDCFP) {
					_WriteLog(LL_INFO, "%s税盘处理第 %d 张发票数据,不支持机动车发票\n", hDev->szCompanyName, i + 1);
				}
				else if (aisino_fplx == FPLX_AISINO_JSFP){
					_WriteLog(LL_INFO, "%s税盘处理第 %d 张发票数据,不支持卷式发票\n", hDev->szCompanyName, i + 1);
				}
				else if (aisino_fplx == FPLX_AISINO_ESC) {
					_WriteLog(LL_INFO, "%s税盘处理第 %d 张发票数据,不支持二手车发票\n", hDev->szCompanyName, i + 1);
				}
				else if (aisino_fplx == FPLX_AISINO_DZZP){
					_WriteLog(LL_INFO, "%s税盘处理第 %d 张发票数据,不支持电子专票发票\n", hDev->szCompanyName, i + 1);
				}
			}
			_WriteLog(LL_INFO, "%s税盘处理第 %d 张发票数据,二进制获取失败,ret = %d\n", hDev->szCompanyName, i + 1, nLen);
			continue;
		}


		if (old_new == 0)
		{
			result = invoice_bin_fpxx_json_base_old(hDev, (char *)pInvoiceBuff, &encode_data);
		
			//_WriteLog(LL_INFO, "invoice_bin_invdetail_json_base result = %d\n", result);
		}
		else
		{
			uint8 zfbz;
			result = invoice_bin_fpxx_json_base(hDev, (char *)pInvoiceBuff, &encode_data, sfxqqd, &zfbz);
			//_WriteLog(LL_INFO, "invoice_bin_fpxx_json_base result = %d\n",result );
		}
		if (result < 0)
		{
			_WriteLog(LL_INFO, "发票数据解析失败,result = %d\n", result);
			if (encode_data != NULL)
			{
				free(encode_data);
				encode_data = NULL;
				_WriteLog(LL_INFO, "处理第 %d 张发票数据解析失败，释放内存完成\n", i + 1);
			}
			continue;
		}

		send_data[index] = (((i + 1) >> 24) & 0xff); send_data[index + 1] = (((i + 1) >> 16) & 0xff);
		send_data[index + 2] = (((i + 1) >> 8) & 0xff); send_data[index + 3] = (((i + 1) >> 0) & 0xff);
		if (encode_data != NULL)
			inv_len = strlen((const char *)encode_data);
		send_data[index + 4] = ((inv_len >> 24) & 0xff); send_data[index + 5] = ((inv_len >> 16) & 0xff);
		send_data[index + 6] = ((inv_len >> 8) & 0xff); send_data[index + 7] = ((inv_len >> 0) & 0xff);
		memcpy(send_data + 8 + index, encode_data, inv_len);
		if (encode_data != NULL)
		{
			//_WriteLog(LL_INFO, "处理第 %d 张发票数据拷贝完成，释放内存完成,address = %08x\n", i + 1, encode_data);
			free(encode_data);
			encode_data = NULL;

		}
		index += 8 + inv_len;
		now_count += 1;
		//hook((unsigned char *)encode_data, nInvoiceCount, i + 1, hook_arg);
		//free(encode_data);
		_WriteLog(LL_INFO, "处理第 %d 张发票完成============================\n\n\n", i + 1);
		if (i == nInvoiceCount - 1)
		{
			//_WriteLog(LL_INFO, "准备回调发送发票数据\n");
			hook(send_data, nInvoiceCount, now_count, hook_arg, hDev->szDeviceID);
			//_WriteLog(LL_INFO, "所有发票数据读取完成\n");
			break;
		}
		if (index > max_send_len)
		{
			//_WriteLog(LL_INFO, "发票数据量大于1M\n");
			hook(send_data, nInvoiceCount, now_count, hook_arg, hDev->szDeviceID);
			memset(send_data, 0, max_buf_len);
			begin_time = get_time_sec();
			now_count = 0;
			index = 0;
		}
		now_time = get_time_sec();
		if (now_time - begin_time > over_time)
		{
			//_WriteLog(LL_INFO, "获取时间超过5秒\n");
			hook(send_data, nInvoiceCount, now_count, hook_arg, hDev->szDeviceID);
			memset(send_data, 0, max_buf_len);
			begin_time = get_time_sec();
			now_count = 0;
			index = 0;
		}
	}
	*start_stop = 0;
	free(pInvoiceBuff);
	free(send_data);

	return 0;
}



#ifdef DF_OLD_MSERVER
static int aisino_get_invoice_month_upload_hook(HDEV hDev, uint8 *month, void *function, void *arg, struct _upload_inv_num *upload_inv_num, unsigned short *start_stop, struct _inv_sum_data *now_month_sum)
{
	char errinfo[1024] = {0};
	int m_total_num = -1;					//单月发票数量
	uint8 *pInvoiceBuff;					//单张发票数据
	int inv_count = 0;
	char *encode_data = NULL;
	int result;
	unsigned char *send_data;
	unsigned char *lxp_data;
	int max_lxfp_len = 1 * 1024 * 1024;//最大数据超过2M即回调发送
	int max_send_len = 4 * 1024 * 1024;//缓存区，超过即发送
	int max_fp_len = 4 * 1024 * 1024;//单张发票最大长度
	int max_buf_len = 8 * 1024 * 1024;//最大发送缓存区
	int over_time = 10;//发票数据获取时间超过5秒即回调发送
	long begin_time;
	long now_time;
	long index = 0;
	long inv_len;
	int now_count = 0;
	int up_num = 0;
	int valid_num = 0;
	char today_month[10] = {0};
	char *year_month_data = NULL;
	int use_http_data = 0;
	void *hook_arg;
	int(*hook)(unsigned char *s_data, int complete_flag, int now_num, void *hook_arg, char *plate_num, unsigned char *s_month, char *statistics, int complete_err);
	struct _inv_sum_data inv_sum;
	memset(&inv_sum, 0, sizeof(struct _inv_sum_data));
	mb_get_today_month(today_month);
	hook = function;
	hook_arg = arg;	

	char stand_time[100] = {0};
	result = AisinoGetDeivceTime(hDev->hUSB, (char *)stand_time);
	if (result < 0)
	{
		_WriteLog(LL_INFO, "金税盘获取税盘时间失败\n");
		return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
	}

	if (memcmp(month, hDev->szDeviceEffectDate, 6) < 0)
	{
		//_WriteLog(LL_INFO, "%s税盘当月%s未到税盘启用时间:%s\n", device->szCompanyName, month, device->szDeviceEffectDate);
		result = hook(NULL, DF_INV_MONTH_COMPLETE, 0, hook_arg, (char *)hDev->szDeviceID, month, NULL, 0);
		if(result < 0)
		{
			_WriteLog(LL_INFO, "%s税盘回调同步发送数据超时\n", hDev->szCompanyName);
			return 	DF_TAX_ERR_CODE_HOOK_SERVER_TIME_OUT;
		}
		return 0;
	}

	//_WriteLog(LL_INFO, "读取%s月发票总数\n", (char *)month);
	m_total_num = GetInvMonthTotal(hDev->hUSB, (char *)month);
	if (m_total_num <= 0)
	{
		//_WriteLog(LL_INFO, "%s税盘当月%s无发票或查询失败 result:%d\n", hDev->szCompanyName, month, m_total_num);
		char *statistics = NULL;
		aisino_query_invoice_month_all_data(hDev, (char *)month, &statistics, errinfo);
		result = compare_inv_sum_statistics(&inv_sum, statistics);
		if (result == 0)
		{
			result = hook(NULL, DF_INV_MONTH_COMPLETE, 0, hook_arg, (char *)hDev->szDeviceID, month, statistics, 0);
			if (result < 0)
			{
				if (statistics != NULL)
					free(statistics);
				_WriteLog(LL_INFO, "%s税盘回调同步发送数据超时\n", hDev->szCompanyName);
				return 	DF_TAX_ERR_CODE_HOOK_SERVER_TIME_OUT;
			}
		}
		else
		{
			_WriteLog(LL_INFO, "%s税盘,发票读取月总数为0,汇总数据与发票明细数据对比不一致或对比出错,错误号：%d\r\n", hDev->szCompanyName, result);
			result = hook(NULL, DF_INV_MONTH_COMPLETE, 0, hook_arg, (char *)hDev->szDeviceID, month, statistics, 1);
			if (result < 0)
			{
				if (statistics != NULL)
					free(statistics);
				_WriteLog(LL_INFO, "%s税盘回调同步发送数据超时\n", hDev->szCompanyName);
				return 	DF_TAX_ERR_CODE_HOOK_SERVER_TIME_OUT;
			}
		}
		if (statistics != NULL)
			free(statistics);
		return 0;
	}
	if ((strcmp(upload_inv_num->year_month, today_month) == 0) && upload_inv_num->now_num > 0 && strlen(upload_inv_num->year_month_data)==0)
	{

		_WriteLog(LL_INFO, "%s税盘当月%s发票数据有%d张,可能存在不一致问题,需查询服务器再对齐\n", hDev->szCompanyName, month, upload_inv_num->now_num);
		result = get_plate_invs_sync(hDev->szDeviceID, &year_month_data);
		if (result < 0)
		{
			use_http_data = 0;
			_WriteLog(LL_INFO, "%s税盘当月%s发票数据有%d张,可能存在不一致问题,与服务器获取对齐失败\n", hDev->szCompanyName, month, upload_inv_num->now_num);
		}
		else
			use_http_data = 1;
				
	}

	valid_num = m_total_num;//初始有效发票数量 等于 读取到的发票总数（可能包含卷票、机动车票）
	//_WriteLog(LL_INFO, "%s 获取%s月发票总数 = %d\n", hDev->szCompanyName, month, m_total_num);
	send_data = (uint8 *)malloc(max_buf_len); //发票数据缓冲区10M
	memset(send_data, 0, max_buf_len);
	lxp_data = (uint8 *)malloc(max_lxfp_len);
	memset(lxp_data, 0, max_lxfp_len);
	pInvoiceBuff = (uint8 *)malloc(max_fp_len);
	
	//printf("[+] my_read_single_invoice Begin output invoice information\n");

	begin_time = get_time_sec();
	for (inv_count = 0; inv_count < m_total_num; inv_count++)
	{
		//_WriteLog(LL_INFO, "%s税盘处理第 %d 张发票数据开始\n", device->szCompanyName, inv_count + 1);
		if (*start_stop != 1)
		{
			_WriteLog(LL_INFO, "%s税盘停止正在查询%s月发票详情动作\n", hDev->szCompanyName, (char *)month);
			free(pInvoiceBuff);
			free(send_data);
			free(lxp_data);
			if (year_month_data != NULL)
			{
				free(year_month_data);
			}
			return -1;
		}
		memset(pInvoiceBuff, 0, max_fp_len);
		//_WriteLog(LL_INFO, "%s税盘处理第 %d 张发票数据\n", device->szCompanyName, inv_count + 1);
		uint8 aisino_fplx;
		long nLen = GetInvDetail(hDev, inv_count, (char *)month, &pInvoiceBuff, (int *)&max_fp_len, &aisino_fplx);
		if (nLen < 200) //发票长度不可能低于768字节,实际测试中发现测试盘空白废仅732个字节
		{
			_WriteLog(LL_INFO, "%s税盘处理第 %d 张发票数据,%d发票类型\n", hDev->szCompanyName, inv_count + 1, aisino_fplx);
			if ((aisino_fplx == FPLX_AISINO_HYFP) || (aisino_fplx == FPLX_AISINO_JDCFP) || (aisino_fplx == FPLX_AISINO_JSFP) || (aisino_fplx == FPLX_AISINO_ESC) || (aisino_fplx == FPLX_AISINO_DZZP))
			{
				valid_num -= 1;
				if (aisino_fplx == FPLX_AISINO_HYFP) {
					_WriteLog(LL_INFO, "%s税盘处理第 %d 张发票数据,货运发票\n", hDev->szCompanyName, inv_count + 1);
				}
				else if (aisino_fplx == FPLX_AISINO_JDCFP) {
					_WriteLog(LL_INFO, "%s税盘处理第 %d 张发票数据,不支持机动车发票\n", hDev->szCompanyName, inv_count + 1);
				}
				else if (aisino_fplx == FPLX_AISINO_JSFP){
					_WriteLog(LL_INFO, "%s税盘处理第 %d 张发票数据,不支持卷式发票\n", hDev->szCompanyName, inv_count + 1);
				}
				else if (aisino_fplx == FPLX_AISINO_ESC) {
					_WriteLog(LL_INFO, "%s税盘处理第 %d 张发票数据,不支持二手车发票\n", hDev->szCompanyName, inv_count + 1);
				}
				else if (aisino_fplx == FPLX_AISINO_DZZP){
					_WriteLog(LL_INFO, "%s税盘处理第 %d 张发票数据,不支持电子专票发票\n", hDev->szCompanyName, inv_count + 1);
				}
			}
			_WriteLog(LL_INFO, "%s税盘处理第 %d 张发票数据,二进制获取失败,ret = %d\n", hDev->szCompanyName, inv_count + 1, nLen);
			continue;
		}
		
		HFPXX fpxx = MallocFpxxLinkDev(hDev);
		result = invoice_bin_fpxx_json_base_outfpxx(hDev, (char *)pInvoiceBuff, &encode_data, 1, fpxx);
		if (result < 0)
		{
			_WriteLog(LL_INFO, "%s税盘处理第 %d 张发票数据,发票数据解析失败,result = %d\n", hDev->szCompanyName, inv_count + 1, result);
			//_WriteLog(LL_INFO, "发票数据解析失败,result = %d\n", result);
			if (encode_data != NULL)
			{
				free(encode_data);
				encode_data = NULL;
				//_WriteLog(LL_INFO, "处理第 %d 张发票数据解析失败，释放内存完成\n", inv_count + 1);
			}
			FreeFpxx(fpxx);
			continue;
		}
		add_inv_sum_data(fpxx, &inv_sum);
		//_WriteLog(LL_INFO, "处理第 %d 张发票数据,发票类型为%03d ,发票代码为%s 发票号码为 %s,作废标志 %d,上传标志 %d,开票时间 %s,作废时间 %s\n",
		//	inv_count + 1, fpxx->fplx, fpxx->fpdm, fpxx->fphm, fpxx->zfbz,fpxx->bIsUpload, fpxx->kpsj_F2, fpxx->zfsj_F2);
		if (fpxx->bIsUpload == 0)
		{
			if (strlen((char *)lxp_data) < max_lxfp_len - 100)
			{
				sprintf((char *)lxp_data + strlen((char *)lxp_data), "%s,%s;", fpxx->fpdm, fpxx->fphm);
			}
		}
		int need_up = jude_need_upload_inv(upload_inv_num, fpxx);
		if (use_http_data == 1)
		{
			if (year_month_data != NULL)
			{
				if ((need_up == 0) && (strlen(year_month_data) != 0))
				{
					//_WriteLog(LL_INFO, "处理第 %d 张发票数据,判断服务器端是否已经入库\n", inv_count + 1);
					char fpdm_fphm_up_off_str[50] = { 0 };
					memset(fpdm_fphm_up_off_str, 0, sizeof(fpdm_fphm_up_off_str));
					sprintf(fpdm_fphm_up_off_str, "%s,%s,%d,%d;", fpxx->fpdm, fpxx->fphm, fpxx->zfbz, fpxx->bIsUpload);

					if (str_replace(year_month_data, fpdm_fphm_up_off_str, ";") != 1)
					{
						_WriteLog(LL_INFO, "%s税盘处理第 %d 张发票数据,未找到服务器入库信息,可能存在错误需重新上传,%s\n", hDev->szCompanyName, inv_count + 1, fpdm_fphm_up_off_str);
						need_up = 1;
					}
				}
			}
		}
		else
		{
			if ((need_up == 0) && (strlen(upload_inv_num->year_month_data) != 0))
			{
				//_WriteLog(LL_INFO, "处理第 %d 张发票数据,判断服务器端是否已经入库\n", inv_count + 1);
				char fpdm_fphm_up_off_str[50] = { 0 };
				memset(fpdm_fphm_up_off_str, 0, sizeof(fpdm_fphm_up_off_str));
				sprintf(fpdm_fphm_up_off_str, "%s,%s,%d,%d;", fpxx->fpdm, fpxx->fphm, fpxx->zfbz, fpxx->bIsUpload);

				if (str_replace(upload_inv_num->year_month_data, fpdm_fphm_up_off_str, ";") != 1)
				{
					_WriteLog(LL_INFO, "%s税盘处理第 %d 张发票数据,未找到服务器入库信息,可能存在错误需重新上传,%s\n", hDev->szCompanyName, inv_count + 1, fpdm_fphm_up_off_str);
					need_up = 1;
				}
			}
		}
		if (need_up != 1)
		{
			if (encode_data != NULL)
			{
				free(encode_data);
				encode_data = NULL;
			}
			FreeFpxx(fpxx);
			continue;
		}
		//_WriteLog(LL_INFO, "处理第 %d 张发票数据判定该张发票需上传服务器，发票代码为%s 发票号码为 %s,index = %d ,inv_len = %d\n", inv_count + 1, fpxx->fpdm, fpxx->fphm, index, strlen((const char *)encode_data));

		FreeFpxx(fpxx);
		up_num += 1;
		send_data[index + 0] = (((inv_count + 1) >> 24) & 0xff); //发票序号
		send_data[index + 1] = (((inv_count + 1) >> 16) & 0xff);
		send_data[index + 2] = (((inv_count + 1) >> 8) & 0xff);
		send_data[index + 3] = (((inv_count + 1) >> 0) & 0xff);
		if (encode_data != NULL)
			inv_len = strlen((const char *)encode_data);
		send_data[index + 4] = ((inv_len >> 24) & 0xff); send_data[index + 5] = ((inv_len >> 16) & 0xff);//发票长度
		send_data[index + 6] = ((inv_len >> 8) & 0xff); send_data[index + 7] = ((inv_len >> 0) & 0xff);
		memcpy(send_data + 8 + index, encode_data, inv_len);
		if (encode_data != NULL)
		{
			free(encode_data);
			encode_data = NULL;

		}
		index += 8 + inv_len;
		now_count += 1;
		if (inv_count == m_total_num - 1)
		{
			result = hook(send_data, 0, now_count, hook_arg, hDev->szDeviceID, month, NULL, 0);
			if (result < 0)
			{
				if (year_month_data != NULL)
					free(year_month_data);
				free(pInvoiceBuff);
				free(send_data);
				free(lxp_data);		
				_WriteLog(LL_INFO, "%s税盘回调同步发送数据超时\n", hDev->szCompanyName);
				return 	DF_TAX_ERR_CODE_HOOK_SERVER_TIME_OUT;
			}
			up_num = 0;
			break;
		}
		if (index > max_send_len)
		{
			//_WriteLog(LL_INFO, "发票数据量大于1M\n");
			result = hook(send_data, 0, now_count, hook_arg, hDev->szDeviceID, month, NULL, 0);
			if (result < 0)
			{
				if (year_month_data != NULL)
					free(year_month_data);
				free(pInvoiceBuff);
				free(send_data);
				free(lxp_data);		
				_WriteLog(LL_INFO, "%s税盘回调同步发送数据超时\n", hDev->szCompanyName);
				return 	DF_TAX_ERR_CODE_HOOK_SERVER_TIME_OUT;
			}
			up_num = 0;
			memset(send_data, 0, max_buf_len);
			begin_time = get_time_sec();
			now_count = 0;
			index = 0;
		}
		now_time = get_time_sec();
		if (now_time - begin_time > over_time)
		{
			//_WriteLog(LL_INFO, "获取时间超过5秒,index = %d\n", index);
			result = hook(send_data, 0, now_count, hook_arg, hDev->szDeviceID, month, NULL, 0);
			if (result < 0)
			{
				if (year_month_data != NULL)
					free(year_month_data);
				free(pInvoiceBuff);
				free(send_data);
				free(lxp_data);		
				_WriteLog(LL_INFO, "%s税盘回调同步发送数据超时\n", hDev->szCompanyName);
				return 	DF_TAX_ERR_CODE_HOOK_SERVER_TIME_OUT;
			}
			up_num = 0;
			memset(send_data, 0, max_buf_len);
			begin_time = get_time_sec();
			now_count = 0;
			index = 0;
		}

	}
	if (up_num != 0)
	{
		result = hook(send_data, 0, now_count, hook_arg, hDev->szDeviceID, month, NULL, 0);
		if (result < 0)
		{
			if (year_month_data != NULL)
				free(year_month_data);
			free(pInvoiceBuff);
			free(send_data);
			free(lxp_data);			
			_WriteLog(LL_INFO, "%s税盘回调同步发送数据超时\n", hDev->szCompanyName);
			return 	DF_TAX_ERR_CODE_HOOK_SERVER_TIME_OUT;
		}
		up_num = 0;
	}

	//_WriteLog(LL_INFO, "%s税盘当前发票正常读取完成,非中断结束,通知M服务当前月读取完成,发票数量共%d张\n", hDev->szCompanyName, valid_num);
	if (memcmp(month, hDev->szDeviceTime, 6) == 0)
	{
		memcpy(now_month_sum, &inv_sum, sizeof(struct _inv_sum_data));
	}
	char *statistics = NULL;
	aisino_query_invoice_month_all_data(hDev, (char *)month, &statistics, errinfo);
	result = compare_inv_sum_statistics(&inv_sum, statistics);
	if (result == 0)
	{
		result = hook(lxp_data, DF_INV_MONTH_COMPLETE, valid_num, hook_arg, (char *)hDev->szDeviceID, month, statistics, 0);
		if (result < 0)
		{
			if (statistics != NULL)
				free(statistics);
			if (year_month_data != NULL)
				free(year_month_data);
			free(pInvoiceBuff);
			free(send_data);
			free(lxp_data);			
			_WriteLog(LL_INFO, "%s税盘回调同步发送数据超时\n", hDev->szCompanyName);
			return 	DF_TAX_ERR_CODE_HOOK_SERVER_TIME_OUT;
		}
	}
	else
	{
		_WriteLog(LL_INFO, "汇总数据与发票明细数据对比不一致或对比出错,错误号：%d\r\n", result);
		result = hook(lxp_data, DF_INV_MONTH_COMPLETE, valid_num, hook_arg, (char *)hDev->szDeviceID, month, statistics, 1);//与汇总数据不一致，可能存在问题
		if (result < 0)
		{
			if (statistics != NULL)
				free(statistics); 
			if (year_month_data != NULL)
				free(year_month_data);
			free(pInvoiceBuff);
			free(send_data);
			free(lxp_data);			
			_WriteLog(LL_INFO, "%s税盘回调同步发送数据超时\n", hDev->szCompanyName);
			return 	DF_TAX_ERR_CODE_HOOK_SERVER_TIME_OUT;
		}
	}
	if (statistics != NULL)
		free(statistics);
	up_num = 0;

	//*start_stop = 0;
	if (year_month_data != NULL)
		free(year_month_data);
	free(pInvoiceBuff);
	free(send_data);
	free(lxp_data);
	return 0;
}

static int aisino_get_invoice_single_upload_hook(HDEV hDev, void *function, void *arg, struct _upload_inv_num *upload_inv_num)
{
	//int m_total_num = -1;					//单月发票数量
	//uint8 *pInvoiceBuff;					//单张发票数据
	int inv_count = 0;
	char *encode_data = NULL;
	int result;
	unsigned char *send_data;
	int max_len = 1 * 1024 * 1024;//最大数据超过1M即回调发送
	long inv_len;
	//int now_count = 0;
	char now_month[20] = { 0 };


	void *hook_arg;
	int(*hook)(unsigned char *s_data, int complete_flag, int now_num, void *hook_arg, char *plate_num, unsigned char *s_month, char *statistics, int complete_err);

	hook = function;
	hook_arg = arg;

	for (inv_count = 0; inv_count < sizeof(upload_inv_num->need_upload_dmhm) / sizeof(upload_inv_num->need_upload_dmhm[0]); inv_count++)
	{
		if (upload_inv_num->need_upload_dmhm[inv_count].state != 1)
		{
			//_WriteLog(LL_INFO, "后续已无发票退出循环\n");
			break;
		}
		if (upload_inv_num->need_upload_dmhm[inv_count].up_flag == 1)
		{
			_WriteLog(LL_INFO, "该张发票已上传过无需重复上传\n");
			continue;
		}
		HFPXX fpxx = MallocFpxxLinkDev(hDev);
		if ((result = QueryInvInfo(hDev, upload_inv_num->need_upload_dmhm[inv_count].fpdm, upload_inv_num->need_upload_dmhm[inv_count].fphm, 0, fpxx)) < 0)
		{
			_WriteLog(LL_INFO, "发票获取失败,ret = %d\n", result);
			FreeFpxx(fpxx);
			continue;
		}
		fpxx_to_json_base_fpsjbbh_v102(fpxx, &encode_data, DF_FPQD_Y, DF_FP_BASE);
		memset(now_month, 0, sizeof(now_month));
		memcpy(now_month,fpxx->kpsj_standard,6);
		FreeFpxx(fpxx);
		send_data = (uint8 *)malloc(max_len * 10 * 1); //发票数据缓冲区10M
		memset(send_data, 0, max_len * 10);
		send_data[0] = (((inv_count + 1) >> 24) & 0xff); //发票序号
		send_data[1] = (((inv_count + 1) >> 16) & 0xff);
		send_data[2] = (((inv_count + 1) >> 8) & 0xff);
		send_data[3] = (((inv_count + 1) >> 0) & 0xff);
		if (encode_data != NULL)
			inv_len = strlen((const char *)encode_data);
		send_data[4] = ((inv_len >> 24) & 0xff); send_data[5] = ((inv_len >> 16) & 0xff);//发票长度
		send_data[6] = ((inv_len >> 8) & 0xff); send_data[7] = ((inv_len >> 0) & 0xff);
		memcpy(send_data + 8, encode_data, inv_len);
		if (encode_data != NULL)
		{
			free(encode_data);
			encode_data = NULL;
		}
		hook(send_data, 0, 1, hook_arg, hDev->szDeviceID, (uint8 *)now_month, NULL, 0);
		upload_inv_num->need_upload_dmhm[inv_count].up_flag = 1;
		free(send_data);
	}


	
	return 0;
}

int aisino_upload_m_server(HDEV hDev, unsigned short *start_stop, struct _upload_inv_num *upload_inv_num, void *function, void *arg, struct _inv_sum_data *now_month_sum)
{
	char errinfo[1024] = { 0 };
	int result = 0;
	char today_month[10] = { 0 };
	char year_month[100] = { 0 };
	char summary_month[100] = { 0 };
	void *hook_arg;
	int(*hook)(unsigned char *s_data, int total_num, int now_num, void *hook_arg, char *plate_num, uint8 *month, char *statistics, int complete_err);
	hook = function;
	hook_arg = arg;
	ClearLastError(hDev->hUSB);
	if (strcmp(upload_inv_num->plate_num, hDev->szDeviceID) != 0)
	{
		return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
	}
	if ((upload_inv_num->state == 0) && (upload_inv_num->need_fpdmhm_flag == 1))
	{
		//_WriteLog(LL_INFO,"需要按发票代码号码上传发票");
		/////后续上传操作
		result = aisino_get_invoice_single_upload_hook(hDev, function, arg, upload_inv_num);
		return result;

	}
	if (upload_inv_num->state != 1)
	{
		//_WriteLog(LL_INFO,"无需上传发票");
		return 0;
	}
	//_WriteLog(LL_INFO,"需要按时间段上传发票\n");
	mb_get_today_month(today_month);
	*start_stop = 1;
	strcpy(year_month, upload_inv_num->year_month);
	strcpy(summary_month, upload_inv_num->summary_month);
	if (strlen(summary_month) == 0 || (strcmp(summary_month, "201801") < 0))
	{
		memset(summary_month, 0, sizeof(summary_month));
		sprintf(summary_month, "201801");
		if (upload_inv_num->just_single_month == 1)
		{
			return 0;
		}
	}
	for (; strcmp(summary_month, today_month) <= 0; mb_get_next_month(summary_month))
	{
		//_WriteLog(LL_INFO, "\n读取%s月发票并上传", summary_month);
		if (*start_stop != 1)
		{
			_WriteLog(LL_INFO, "停止正在查询%s月发票详情动作\n", (char *)summary_month);
			return -1;
		}
		//_WriteLog(LL_INFO, "正在处理的汇总月份为%s，正在处理的发票月份为%s\n", (char *)summary_month, year_month);
		if (strcmp(summary_month, year_month) == 0)
		{
			result = aisino_get_invoice_month_upload_hook(hDev, (uint8 *)year_month, function, arg, upload_inv_num, start_stop, now_month_sum);
			if (result < 0)
			{
				_WriteLog(LL_INFO, "读取发票数据并回调上传过程中出现异常,停止正在查询%s月发票详情动作\n", (char *)summary_month);
				return result;
			}
			mb_get_next_month(year_month);
		}
		else //汇总月份比发票月份小，需补齐情况
		{
			char *statistics = NULL;
			result = aisino_query_invoice_month_all_data(hDev, (char *)summary_month, &statistics, errinfo);
			if (result < 0)
			{
				_WriteLog(LL_INFO, "读取发票汇总数据过程中出现异常,停止正在查询%s月发票详情动作\n", (char *)summary_month);
				continue;
			}
			result = hook(NULL, 1, -100, hook_arg, (char *)hDev->szDeviceID, (uint8 *)summary_month, statistics, 0);
			if (result < 0)
			{
				if (statistics != NULL)
					free(statistics);
				_WriteLog(LL_INFO, "回调上报发票汇总数据出现异常,停止正在查询%s月发票详情动作\n", (char *)summary_month);
				return result;
			}
			if (statistics != NULL)
				free(statistics);
		}

		if (upload_inv_num->just_single_month == 1)
		{
			return 0;
		}
	}
	return 0;
}
#else
static int aisino_get_invoice_month_upload_hook(HDEV hDev, char *month, void *function, void *arg, struct _upload_inv_num *upload_inv_num, unsigned short *start_stop, struct _inv_sum_data *now_month_sum)
{
	int all_inv_type_num = -1;
	uint8 *pInvoiceBuff;					
	int inv_count = 0;
	char *inv_data = NULL;
	int result;
	struct stat statbufs;
	int zip_data_len = 0;
	int max_fp_len = 4 * 1024 * 1024;
	int need_up;
	int need_up_count= 0;
	char today_month[10] = {0};
	struct _inv_sum_data inv_sum;
	char fp_zip_name[100] = { 0 };
	char fp_txt_name[100] = { 0 };
	char stand_time[100] = { 0 };
	char fphm_crc[50] = { 0 };
	zipFile zf = NULL;

	memset(&inv_sum, 0, sizeof(struct _inv_sum_data));
	mb_get_today_month(today_month);		
	sprintf(fp_zip_name, "/tmp/%s.zip", hDev->szDeviceID);
	result = aisino_read_tax_time(hDev, stand_time);
	if (result < 0)
	{
		_WriteLog(LL_INFO, "获取税盘时间失败\n");
		return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
	}



	if (memcmp(month, hDev->szDeviceEffectDate, 6) < 0)
	{
		_WriteLog(LL_INFO, "%s税盘当月%s未到税盘启用时间:%s\n", hDev->szCompanyName, month, hDev->szDeviceEffectDate);
		//result = hook(hook_arg, (char *)month, NULL, DF_UPLOAD_INV);
		result = send_zip_data_hook(hDev, month, function, arg, need_up_count, zf, fp_zip_name, zip_data_len, 1);
		if (result < 0)
		{
			_WriteLog(LL_INFO, "%s税盘回调同步发送数据超时\n", hDev->szCompanyName);
			return 	DF_TAX_ERR_CODE_HOOK_SERVER_TIME_OUT;
		}
		return 0;
	}


	//_WriteLog(LL_INFO, "读取%s月发票总数\n", (char *)month);
	all_inv_type_num = GetInvMonthTotal(hDev->hUSB, (char *)month);
	if (all_inv_type_num <= 0)
	{
		//_WriteLog(LL_INFO, "%s税盘当月%s无发票或查询失败 result:%d\n", hDev->szCompanyName, month, all_inv_type_num);
		result = send_zip_data_hook(hDev, month, function, arg, need_up_count, zf, fp_zip_name, zip_data_len, 1);
		if (result < 0)
		{
			_WriteLog(LL_INFO, "%s税盘回调同步发送数据超时\n", hDev->szCompanyName);
			return 	DF_TAX_ERR_CODE_HOOK_SERVER_TIME_OUT;
		}
		return 0;
	}

	if (file_exists(fp_zip_name) == 0)
	{
		_WriteLog(LL_INFO, "%s税盘处理发票数据前存在压缩文件先删除\n", hDev->szCompanyName);
		delete_file(fp_zip_name);
	}
	zf = zipOpen64(fp_zip_name, 0);
	if (zf == NULL)
	{
		_WriteLog(LL_INFO, "zipOpen64 compress file:%s fail!\n", fp_zip_name);
		return 	DF_TAX_ERR_CODE_SYSTEM_MEM_INSUFFICIENT;
	}



	//_WriteLog(LL_INFO, "%s 获取%s月发票总数 = %d\n", hDev->szCompanyName, month, all_inv_type_num);

	pInvoiceBuff = (uint8 *)malloc(max_fp_len);
	
	//printf("[+] my_read_single_invoice Begin output invoice information\n");

	for (inv_count = 0; inv_count < all_inv_type_num; inv_count++)
	{
		//_WriteLog(LL_INFO, "%s税盘处理第 %d 张发票数据开始\n", device->szCompanyName, inv_count + 1);
		if (*start_stop != 1)
		{
			_WriteLog(LL_INFO, "%s税盘停止正在查询%s月发票详情动作\n", hDev->szCompanyName, (char *)month);
			zipClose(zf, NULL);
			delete_file(fp_zip_name);
			free(pInvoiceBuff);
			return -1;
		}
		memset(pInvoiceBuff, 0, max_fp_len);
		//_WriteLog(LL_INFO, "%s税盘处理第 %d 张发票数据\n", device->szCompanyName, inv_count + 1);
		uint8 aisino_fplx;
		if(hDev == NULL)
		{
			_WriteLog(LL_INFO, "hDev == NULL,%s税盘停止正在查询%s月发票详情动作\n", hDev->szCompanyName, (char *)month);
			zipClose(zf, NULL);
			delete_file(fp_zip_name);
			free(pInvoiceBuff);
			return -1;
		}
		long nLen = GetInvDetail(hDev, inv_count, (char *)month, &pInvoiceBuff, (int *)&max_fp_len, &aisino_fplx);
		if (nLen < 200) //发票长度不可能低于768字节,实际测试中发现测试盘空白废仅732个字节
		{
			_WriteLog(LL_INFO, "%s税盘处理第 %d 张发票数据,%d发票类型\n", hDev->szCompanyName, inv_count + 1, aisino_fplx);
			if ((aisino_fplx == FPLX_AISINO_HYFP) || 
				(aisino_fplx == FPLX_AISINO_JDCFP) ||
				(aisino_fplx == FPLX_AISINO_JSFP) || 
				(aisino_fplx == FPLX_AISINO_ESC) ||
				(aisino_fplx == FPLX_AISINO_DZZP))
			{
				if (aisino_fplx == FPLX_AISINO_HYFP) {
					_WriteLog(LL_INFO, "%s税盘处理第 %d 张发票数据,货运发票\n", hDev->szCompanyName, inv_count + 1);
				}
				else if (aisino_fplx == FPLX_AISINO_JDCFP) {
					_WriteLog(LL_INFO, "%s税盘处理第 %d 张发票数据,不支持机动车发票\n", hDev->szCompanyName, inv_count + 1);
				}
				else if (aisino_fplx == FPLX_AISINO_JSFP){
					_WriteLog(LL_INFO, "%s税盘处理第 %d 张发票数据,不支持卷式发票\n", hDev->szCompanyName, inv_count + 1);
				}
				else if (aisino_fplx == FPLX_AISINO_ESC) {
					_WriteLog(LL_INFO, "%s税盘处理第 %d 张发票数据,不支持二手车发票\n", hDev->szCompanyName, inv_count + 1);
				}
				else if (aisino_fplx == FPLX_AISINO_DZZP){
					_WriteLog(LL_INFO, "%s税盘处理第 %d 张发票数据,不支持电子专票发票\n", hDev->szCompanyName, inv_count + 1);
				}
			}
			_WriteLog(LL_INFO, "%s税盘处理第 %d 张发票数据,二进制获取失败,ret = %d\n", hDev->szCompanyName, inv_count + 1, nLen);
			continue;
		}
		
		HFPXX fpxx = MallocFpxxLinkDev(hDev);
		result = invoice_bin_fpxx_json_base_outfpxx(hDev, (char *)pInvoiceBuff, &inv_data, 1, fpxx);
		if (result < 0)
		{
			_WriteLog(LL_INFO, "%s税盘处理第 %d 张发票数据,发票数据解析失败,result = %d\n", hDev->szCompanyName, inv_count + 1, result);
			//_WriteLog(LL_INFO, "发票数据解析失败,result = %d\n", result);
			if (inv_data != NULL)
			{
				free(inv_data);
				inv_data = NULL;
			}
			FreeFpxx(fpxx);
			continue;
		}

		add_inv_sum_data(fpxx, &inv_sum);
		//_WriteLog(LL_INFO, "处理第 %d 张发票数据,发票类型为%03d ,发票代码为%s 发票号码为 %s,作废标志 %d,上传标志 %d,开票时间 %s,作废时间 %s\n",
		//	inv_count + 1, fpxx->fplx, fpxx->fpdm, fpxx->fphm, fpxx->zfbz,fpxx->bIsUpload, fpxx->kpsj_F2, fpxx->zfsj_F2);

		uint16 crc = crc_8005((uint8 *)inv_data, strlen(inv_data), 0);
		char invoice_crc[10] = { 0 };
		sprintf(invoice_crc, "%04x", crc);

		need_up = 1;
	
		if (upload_inv_num->dmhmzfsbstr != NULL)
		{
			if (strlen(upload_inv_num->dmhmzfsbstr) != 0)
			{
				//_WriteLog(LL_INFO, "处理第 %d 张发票数据,判断服务器端是否已经入库\n", inv_count + 1);
				memset(fphm_crc, 0, sizeof(fphm_crc));
				sprintf(fphm_crc, "%s%s;", fpxx->fphm, invoice_crc);

				if (delete_str(upload_inv_num->dmhmzfsbstr, fphm_crc) == 1)
				{
					need_up = 0;
					//_WriteLog(LL_INFO, "查找%s数据成功,发票存在且无变动,不需重传发票\n", fphm_crc);
				}
			}
		}
		if (need_up != 1)
		{
			free(inv_data);
			inv_data = NULL;
			FreeFpxx(fpxx);
			continue;
		}

		need_up_count += 1;
		//_WriteLog(LL_INFO, "处理第 %d 张发票数据判定该张发票需上传服务器，发票代码为%s 发票号码为 %s,index = %d ,inv_len = %d\n", inv_count + 1, fpxx->fpdm, fpxx->fphm, index, strlen((const char *)inv_data));
		_WriteLog(LL_INFO, "查找%s_%s数据成功crc= %s,需重传发票\n", fpxx->fpdm, fpxx->fphm, fphm_crc);

		memset(fp_txt_name, 0, sizeof(fp_txt_name));
		sprintf(fp_txt_name, "%s_%s_%s.txt", fpxx->fpdm, fpxx->fphm, invoice_crc);

		zip_fileinfo zi;
		memset(&zi, 0, sizeof(zip_fileinfo));
		zipOpenNewFileInZip3_64(zf, fp_txt_name, &zi, NULL, 0, NULL, 0, NULL,Z_DEFLATED, 9, 0, -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY, NULL, 0, 0);
		zipWriteInFileInZip(zf, inv_data, strlen(inv_data));
		zipCloseFileInZip(zf);
		stat(fp_zip_name, &statbufs);
		zip_data_len = statbufs.st_size;
		_WriteLog(LL_INFO, "压缩第%d张发票后,压缩包大小为%d字节", inv_count+1, zip_data_len);

		
		free(inv_data);
		inv_data = NULL;
		FreeFpxx(fpxx);
		
		if (zip_data_len > 1024 * 1024)
		{
			//此情况不管是否失败，zip包及指针肯定都已经清理
			result = send_zip_data_hook(hDev, month, function, arg, need_up_count, zf, fp_zip_name, zip_data_len, 0);
			if (result < 0)
			{
				free(pInvoiceBuff);
				_WriteLog(LL_INFO, "%s税盘回调同步发送数据超时\n", hDev->szCompanyName);
				return 	DF_TAX_ERR_CODE_HOOK_SERVER_TIME_OUT;
			}
			zf = zipOpen64(fp_zip_name, 0);
			if (zf == NULL)
			{
				free(pInvoiceBuff);
				_WriteLog(LL_INFO, "zipOpen64 compress file:%s fail!\n", fp_zip_name);
				return 	DF_TAX_ERR_CODE_SYSTEM_MEM_INSUFFICIENT;
			}
			need_up_count = 0;//已上传完成，需上传发票数量清零
			continue;
		}
		
	}	
	free(pInvoiceBuff);
	result = send_zip_data_hook(hDev, month, function, arg, need_up_count, zf, fp_zip_name, zip_data_len, 1);
	if (result < 0)
	{
		if (file_exists(fp_zip_name) == 0)
		{
			zipClose(zf, NULL);
			delete_file(fp_zip_name);
		}
		_WriteLog(LL_INFO, "%s税盘回调同步发送数据超时\n", hDev->szCompanyName);
		return DF_TAX_ERR_CODE_HOOK_SERVER_TIME_OUT;
	}

	if (file_exists(fp_zip_name) == 0)
	{
		zipClose(zf, NULL);
		delete_file(fp_zip_name);
	}

	//_WriteLog(LL_INFO, "%s税盘当前发票正常读取完成,非中断结束,通知M服务当前月读取完成\n", hDev->szCompanyName);
	if (memcmp(month, hDev->szDeviceTime, 6) == 0)
	{
		memcpy(now_month_sum, &inv_sum, sizeof(struct _inv_sum_data));
	}	
	return 0;
}


int aisino_upload_m_server(HDEV hDev, unsigned short *start_stop, struct _upload_inv_num *upload_inv_num, void *function, void *arg, struct _inv_sum_data *now_month_sum)
{
	int result = 0;
	char today_month[10] = { 0 };
	char invoice_month[100] = { 0 };
	ClearLastError(hDev->hUSB);
	
	if (upload_inv_num->state == 0)
	{
		_WriteLog(LL_INFO, "[%s]无需上传发票", upload_inv_num->invoice_month);
		return 0;
	}

	//_WriteLog(LL_INFO,"需要按时间段上传发票\n");
	mb_get_today_month(today_month);
	*start_stop = 1;
	strcpy(invoice_month, upload_inv_num->invoice_month);
	if (strlen(invoice_month) == 0 || (strcmp(invoice_month, "201801") < 0))
	{
		memset(invoice_month, 0, sizeof(invoice_month));
		sprintf(invoice_month, "201801");
		if (upload_inv_num->just_single_month == 1)
		{
			return 0;
		}
	}
	for (; strcmp(invoice_month, today_month) <= 0; mb_get_next_month(invoice_month))
	{
		_WriteLog(LL_INFO, "\n读取%s月发票并上传", invoice_month);
		if (*start_stop != 1)
		{
			_WriteLog(LL_INFO, "停止正在查询%s月发票详情动作\n", (char *)invoice_month);
			return -1;
		}
		//_WriteLog(LL_INFO, "正在处理的汇总月份为%s，正在处理的发票月份为%s\n", (char *)summary_month, invoice_month);

		result = aisino_get_invoice_month_upload_hook(hDev, invoice_month, function, arg, upload_inv_num, start_stop, now_month_sum);
		if (result < 0)
		{
			_WriteLog(LL_INFO, "读取发票数据并回调上传过程中出现异常,停止正在查询%s月发票详情动作\n", (char *)invoice_month);
			return result;
		}
	
		if (upload_inv_num->just_single_month == 1)
		{
			return 0;
		}
	}
	return 0;
}

int aisino_upload_summary_m_server(HDEV hDev, unsigned short *start_stop, struct _upload_inv_num *upload_inv_num, void *function, void *arg, struct _inv_sum_data *now_month_sum)
{
	int result = 0;
	char today_month[10] = { 0 };
	char summary_month[100] = { 0 };
	void *hook_arg;
	int(*hook)(void *hook_arg, char *s_month, char *s_data, int inv_sum);
	hook = function;
	hook_arg = arg;
	ClearLastError(hDev->hUSB);

	if (upload_inv_num->state == 0)
	{
		_WriteLog(LL_INFO, "[%s]无需上传汇总", upload_inv_num->summary_month);
		return 0;
	}

	//_WriteLog(LL_INFO,"需要按时间段上传发票\n");
	mb_get_today_month(today_month);
	*start_stop = 1;
	strcpy(summary_month, upload_inv_num->summary_month);
	if (strlen(summary_month) == 0 || (strcmp(summary_month, "201801") < 0))
	{
		memset(summary_month, 0, sizeof(summary_month));
		sprintf(summary_month, "201801");
		if (upload_inv_num->just_single_month == 1)
		{
			return 0;
		}
	}
	for (; strcmp(summary_month, today_month) <= 0; mb_get_next_month(summary_month))
	{
		_WriteLog(LL_INFO, "\n读取%s月汇总并上传", summary_month);
		if (*start_stop != 1)
		{
			_WriteLog(LL_INFO, "停止正在查询%s月发票详情动作\n", (char *)summary_month);
			return -1;
		}

		char errinfo[1024] = {0};
		char *statistics = NULL;
		result = aisino_query_invoice_month_all_data(hDev, (char *)summary_month, &statistics, errinfo);
		if (result < 0)
		{
			//_WriteLog(LL_INFO, "读取发票汇总数据过程中出现异常,停止正在查询%s月发票详情动作\n", (char *)summary_month);
			continue;
		}
		_WriteLog(LL_INFO, "处理汇总信息数据\n");
		if (memcmp(summary_month, hDev->szDeviceTime, 6) == 0)
		{
			deal_plate_statistics_to_now_month_sum(statistics, now_month_sum);
		}
		result = hook(hook_arg, summary_month, statistics, DF_UPLOAD_SUM);
		if (statistics != NULL)
			free(statistics);
		if (result < 0)
		{
			_WriteLog(LL_INFO, "回调上报发票汇总数据出现异常,停止正在查询%s月发票详情动作\n", (char *)summary_month);
			return result;
		}

		if (upload_inv_num->just_single_month == 1)
		{
			return 0;
		}
	}
	return 0;
}
#endif

int aisino_is_exsist_offline_inv(HDEV hDev, char *fpdm, char *fphm, char *kpsj, int *dzsyh)
{
	int ret = -1;
	ClearLastError(hDev->hUSB);
	//_WriteLog(LL_INFO, "aisino_is_exsist_offline_inv 获取是否存在离线\n");
	char szFPDM[32] = { 0 }, szFPHM[24] = { 0 };
	char szKPSJ[20] = {0};
	int szDZSYH;
	int nChildRet = GetFirstOfflineInv(hDev, szFPDM, szFPHM, &szDZSYH, szKPSJ);
	if (nChildRet < 0) {
		_WriteLog(LL_INFO, "获取是否存在离线发票失败, nRet:%d\n", nChildRet);
		ret = -1;
		goto get_end;
	}
	else if (nChildRet == 0) {
		//_WriteLog(LL_INFO, "【%s】不存在离线发票\n", device->szCompanyName);
		ret = 100;
		goto get_end;
	}
	strcpy(fpdm, szFPDM);
	strcpy(fphm, szFPHM);
	strcpy(kpsj, szKPSJ);
	*dzsyh = szDZSYH;
	//_WriteLog(LL_INFO, "【%s】存在离线发票,nChildRet = %d\n", device->szCompanyName, nChildRet);
	ret = 0;
get_end:
	return ret;
}

int aisino_fpdm_fphm_get_invs(HDEV hDev, char  *fpdm, char *fphm, int dzsyh, char **inv_json)
{
	int ret = -1;
	ClearLastError(hDev->hUSB);
	HFPXX fpxx = MallocFpxx();
	fpxx->hDev = hDev;
	_WriteLog(LL_INFO, "通过发票代码%s,发票号码%s,获取发票数据\n", fpdm, fphm);
	if ((ret = QueryInvInfo(hDev, fpdm, fphm, dzsyh, fpxx)) < 0)
	{
		_WriteLog(LL_INFO, "发票获取失败,ret = %d\n", ret);
		goto get_end;
	}
	//_WriteLog(LL_INFO, "获取发票信息成功,组包到json\n");
	fpxx_to_json_base_fpsjbbh_v102_aison(fpxx, inv_json, DF_FPQD_Y, DF_FP_BASE);
	ret = 0;
get_end:
	if (fpxx)
	{
		//_WriteLog(LL_INFO, "释放发票结果体指针\n");
		FreeFpxx(fpxx);
	}

	return ret;
}

int aisino_fpdm_fphm_get_invs_kpstr(HDEV hDev, char  *fpdm, char *fphm, int dzsyh, char **inv_json, char **kp_bin, char **kp_str)
{
	int ret = -1;
	ClearLastError(hDev->hUSB);
	HFPXX fpxx = MallocFpxx();
	fpxx->hDev = hDev;
	_WriteLog(LL_INFO, "通过发票代码%s,发票号码%s,获取发票数据\n", fpdm, fphm);
	if ((ret = QueryInvInfoOutBinStr(hDev, fpdm, fphm, dzsyh, fpxx, kp_bin, kp_str)) < 0)
	{
		_WriteLog(LL_INFO, "发票获取失败,ret = %d\n", ret);
		goto get_end;
	}
	//_WriteLog(LL_INFO, "获取发票信息成功,组包到json\n");
	fpxx_to_json_base_fpsjbbh_v102(fpxx, inv_json, DF_FPQD_Y, DF_FP_BASE);
	ret = 0;
get_end:
	if (fpxx)
	{
		//_WriteLog(LL_INFO, "释放发票结果体指针\n");
		FreeFpxx(fpxx);
	}

	return ret;
}

int aisino_my_read_invoice_month_num(HUSB hUSB, char *szFormatMonth, unsigned int *Count, unsigned long *Size)
{
	int nInvoiceCount = -1;
	unsigned long nInvoiceSize = 0;
	ClearLastError(hUSB);
	nInvoiceCount = GetInvMonthTotal(hUSB, szFormatMonth);

	if (nInvoiceCount <= 0)
	{
		_WriteLog(LL_INFO, "[-] Get invoice count Ret:%d, may be not found invoice,exit prog\n", nInvoiceCount);
		return -1;
	}
	nInvoiceSize = 3000 * nInvoiceCount;
	*Count = (unsigned int)nInvoiceCount;
	*Size = nInvoiceSize;
	return nInvoiceCount;
}


int aisino_read_current_inv_code(HUSB hUSB, uint8_t bNeedType, char *szInvCurrentNum, char *szInvTypeCode)
{
	int result;
	uint8_t aisino_fplxdm;
	char szInvEndNum[100] = { 0 };
	ClearLastError(hUSB);
	CommonFPLX2AisinoFPLX(bNeedType, &aisino_fplxdm);
	result = AisinoGetCurrentInvCode(hUSB, aisino_fplxdm, szInvCurrentNum, szInvEndNum, szInvTypeCode);
	return result;
}
//int test_load = 0;

int aisino_inv_upload_server(HDEV hDev, struct _offline_upload_result *offline_upload_result)
{
	int result;
	ClearLastError(hDev->hUSB);
	//载入设备信息
	AisinoGetDeivceTime(hDev->hUSB, hDev->szDeviceTime); //更新税盘时间
	if (hDev->bTrainFPDiskType == 0)
	{
		result = aisino_upload_firstOfflineInv(hDev, offline_upload_result);
	}
	else
	{
		////////使用501测试盘模拟开票开具后，发票上传完成与M服务同步发票数据，需将发票代码号码修改待开代码号码

		//if (test_load >= 1)
		//{
		//	return 100;
		//}

		//HFPXX fpxx = MallocFpxx();
		//fpxx->hDev = hDev;
		//if (QueryInvInfo(hDev, "3200121620", "01900682", 0, fpxx) < 0)
		//{
		//	FreeFpxx(fpxx);
		//	_WriteLog(LL_INFO, "发票获取失败\n");
		//	return 99;
		//}

		//test_load += 1;

		//_WriteLog(LL_INFO, "开票时间：%s，发票代码：%s，发票号码：%s，发票类型代码：%03d\n", fpxx->kpsj_F2, fpxx->fpdm, fpxx->fphm, fpxx->fplx);
		//memcpy((char *)offline_upload_result->kpsj, fpxx->kpsj_F2, 19);
		//memcpy((char *)offline_upload_result->fpdm, fpxx->fpdm, 12);
		//memcpy((char *)offline_upload_result->fphm, fpxx->fphm, 12);
		//offline_upload_result->fplxdm = fpxx->fplx;
		//offline_upload_result->fpdzsyh = fpxx->dzsyh;
		//fpxx->bIsUpload = 1;
		//offline_upload_result->fplxdm = fpxx->fplx;
		//memset(offline_upload_result->fpdm, 0, sizeof(offline_upload_result->fpdm));
		//memset(offline_upload_result->fphm, 0, sizeof(offline_upload_result->fphm));
		//memset(offline_upload_result->kpsj, 0, sizeof(offline_upload_result->kpsj));
		//memset(offline_upload_result->hjje, 0, sizeof(offline_upload_result->hjje));
		//memset(offline_upload_result->hjse, 0, sizeof(offline_upload_result->hjse));
		//memset(offline_upload_result->jshj, 0, sizeof(offline_upload_result->jshj));
		//memset(offline_upload_result->jym, 0, sizeof(offline_upload_result->jym));
		//memset(offline_upload_result->mwq, 0, sizeof(offline_upload_result->mwq));
		//memset(offline_upload_result->bz, 0, sizeof(offline_upload_result->bz));
		//memcpy(offline_upload_result->fpdm, fpxx->fpdm, strlen(fpxx->fpdm));
		//memcpy(offline_upload_result->fphm, fpxx->fphm, strlen(fpxx->fphm));
		//memcpy(offline_upload_result->kpsj, fpxx->kpsj_F2, strlen(fpxx->kpsj_F2));
		//memcpy(offline_upload_result->hjje, fpxx->je, strlen(fpxx->je));
		//memcpy(offline_upload_result->hjse, fpxx->se, strlen(fpxx->se));
		//memcpy(offline_upload_result->jshj, fpxx->jshj, strlen(fpxx->jshj));
		//memcpy(offline_upload_result->jym, fpxx->jym, strlen(fpxx->jym));
		//memcpy(offline_upload_result->mwq, fpxx->mw, strlen(fpxx->mw));
		//memcpy(offline_upload_result->bz, fpxx->bz, strlen(fpxx->bz));
		//fpxx_to_json_base_fpsjbbh_v102(fpxx, &offline_upload_result->scfpsj, DF_FPQD_Y, DF_FP_BASE);
		//FreeFpxx(fpxx);
		//logout(INFO, "TAXLIB", "发票上传", "盘号:%s,离线发票上传成功,发票代码号码：%s_%s\r\n", hDev->szDeviceID, offline_upload_result->fpdm, offline_upload_result->fphm);
		//result = 1;
		

		sprintf((char *)offline_upload_result->errinfo, "测试盘不需要发票上传");
		result = 99;
	}
	//_WriteLog(LL_INFO, "发票上传完成,result = %d\n", result);
	return result;
}







int aisino_verify_cert_passwd(HDEV hDev, int *left_num)
{
	int nRet = -1;
	char errinfo[1024] = { 0 };
	ClearLastError(hDev->hUSB);
	AisinoGetDeivceTime(hDev->hUSB, hDev->szDeviceTime); //更新税盘时间
	while (1) {
		//载入盘配置信息
		X509 *pCert = NULL;
		if (!hDev->bTrainFPDiskType) {
			pCert = GetX509Cert(hDev->hUSB, hDev->bDeviceType, hDev->bCryptBigIO);
			if (!pCert) {
				//report_event(hDev->szDeviceID, "证书读取失败", "证书读取失败，可能原因为存在兼容问题", -1);
				_WriteLog(LL_FATAL, "Get cert failed!,last errinfo =　%s \r\n",hDev->hUSB->szLastErrorDescription);
				nRet = -1;
				break;
			}
			hDev->pX509Cert = pCert;
			//开票的签名时候需要输入密码去验证证书,如果不是测试盘需要打开税务证书，否则后面开票失败
			nRet = SignAPI_OpenDevice(hDev->hUSB, hDev->bCryptBigIO, DEF_CERT_PASSWORD_DEFAULT);
			if (nRet <= 0)
			{
				if (nRet > -100) {
					*left_num = -nRet;
					sprintf(errinfo, "证书口令有误，可能原因为未修改默认证书口令12345678,剩余尝试次数%d", *left_num);
					logout(INFO, "TAXLIB", "口令验证", "盘号：%s,错误信息：%s\r\n", hDev->szDeviceID, errinfo);
					//report_event(hDev->szDeviceID, "证书口令错误", errinfo, nRet);
					_WriteLog(LL_FATAL, "Device password incorrect\n");
					nRet = ERR_CERT_PASSWORD_ERR;
					break;
				}
				else 	{
					//report_event(hDev->szDeviceID, "证书口令验证失败", "证书口令验证失败，可能原因为底层USB操作失败", nRet);
					_WriteLog(LL_FATAL, "OpenDevice cert failed\n");
					nRet = DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
					break;
				}
			}
		}
		else {
			hDev->pX509Cert = NULL;
			//_WriteLog(LL_DEBUG, "Jump authenticate cert hDev->bTrainFPDiskType == %d && bAuthDev == %d", hDev->bTrainFPDiskType, bAuthDev);
		}
		//--载入静态变量
		nRet = 1;
		break;
	}
	return nRet;
}

//发票开具
int aisino_make_invoice(HDEV hDev, char* inv_data, struct _plate_infos *plate_infos, struct _make_invoice_result *make_invoice_result)
{
	int result;
	ClearLastError(hDev->hUSB);
	result = LoadAisinoDevInfo(hDev, hDev->hUSB, false);
	if (result < 0)
	{
		sprintf((char *)make_invoice_result->errinfo, "USB操作失败,税盘信息无法加载,请重试");
		return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
	}
	if (strcmp(hDev->szDeviceID, (char *)plate_infos->plate_basic_info.plate_num) != 0)
	{
		sprintf((char *)make_invoice_result->errinfo, "USB操作失败,税盘基础信息不一致,请重试");
		return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
	}

	//result = AisinoGetDeivceTime(device->hUSB, device->szDeviceTime); //更新税盘时间
	//if (result < 0)
	//{
	//	return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
	//}
	//str_replace(inv_data, "\\", "[@*br/*@]");//排查json中的\转义字符
	//result = check_invoice_str((unsigned char *)inv_data);
	//if (result < 0)
	//{
	//	sprintf((char *)make_invoice_result->errinfo, "发票JSON数据中存在特殊字符无法解析,例如&和\\");
	//	return DF_TAX_ERR_CODE_USE_ILL_CHAR_OR_UTF8;
	//}
	//_WriteLog(LL_INFO, "inv_data = %s\n", inv_data);
	//result = judge_code(inv_data, strlen(inv_data));
	//result = IsGBK(inv_data);
	//if (result != 1)
	//{
	//	sprintf((char *)make_invoice_result->errinfo, "发票JSON数据汉字编码格式错误,非GBK编码");
	//	return DF_TAX_ERR_CODE_USE_ILL_CHAR_OR_UTF8;//编码格式错误
	//}

	HFPXX stp_fpxx = MallocFpxxLinkDev(hDev);
	if (stp_fpxx == NULL) {
		printf("stp_fpxx malloc Err\n");
		sprintf((char *)make_invoice_result->errinfo, "系统内存申请失败,机柜可能存在故障,请重启机柜");
		return DF_TAX_ERR_CODE_SYSTEM_MEM_INSUFFICIENT;
	}
	stp_fpxx->hDev = hDev;
	stp_fpxx->isHzxxb = false;
	strcpy(stp_fpxx->tax_nature, (char *)plate_infos->plate_tax_info.tax_nature);
	//_WriteLog(LL_INFO, "解析发票JSON数据,%s\n", inv_data);
	if ((result = AnalyzeJsonBuff(hDev->bDeviceType, inv_data, stp_fpxx, (char *)make_invoice_result->errinfo)) != 0)
	{
		logout(INFO, "TAXLIB", "发票开具", "发票解析失败,错误代码%d\r\n", result);
		FreeFpxx(stp_fpxx);
		return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
	}
	UpdateDevInfoToFPXX(hDev, stp_fpxx);

	result = check_fpxx_aisino(stp_fpxx, plate_infos, (char *)make_invoice_result->errinfo);
	if (result < 0)
	{
		_WriteLog(LL_INFO, "check_fpxx_aisino failed\n");
		goto FreeAndExit;
	}

	// update by type
	if (DEVICE_AISINO == hDev->bDeviceType) {
		if (AisinoUpdateNodeInfo(stp_fpxx, hDev))
		{
			_WriteLog(LL_INFO, "替换测试盘开票数据出错\n");
			result = DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
			goto FreeAndExit;
		}
	}
	if (CheckFpxxNode(stp_fpxx, hDev, (char *)make_invoice_result->errinfo))
	{
		logout(INFO, "TAXLIB", "发票开具", "检验发票数据是否正确出错\r\n");
		result = DF_TAX_ERR_CODE_INV_DATA_DISACCORD;
		goto FreeAndExit;
	}
	//此处追加证书及口令认证
	//AddLoadAisinoCert(device, device->hUSB);
	FillDevInfoCert(hDev, 1);
	//if ((stp_fpxx->isRed) && (!device->bTrainFPDiskType))
	//{
	//	uint8 bAisinoFPLX;
	//	char szRepMsg[512] = "";
	//	CommonFPLX2AisinoFPLX(stp_fpxx->fplx, &bAisinoFPLX);
	//	int nRet = RedInvOnlineCheck_PTDZ(device, bAisinoFPLX, stp_fpxx->blueFpdm, stp_fpxx->blueFphm, 0, szRepMsg, sizeof(szRepMsg));
	//	if (nRet < 0) {
	//		printf("[-] FPDM:%s FPHM:%s, RedInvOnlineCheck failed,  msg:[%s]\n", stp_fpxx->blueFpdm,
	//			stp_fpxx->blueFphm, szRepMsg);
	//		EncordingConvert("utf-8", "gbk", szRepMsg, strlen(szRepMsg), (char *)make_invoice_result->errinfo, 1024);
	//		//_WriteLog(LL_INFO, "错误信息%s\n", errinfo);
	//		logout(INFO, "taxlib", "fpkj","红字发票开具蓝字发票查验失败,错误信息%s\r\n", make_invoice_result->errinfo);
	//		goto FreeAndExit;
	//	}
	//}
	if (make_invoice_result->test_only == 1)
	{
		logout(INFO, "TAXLIB", "发票开具", "测试接口不执行最终开具动作\r\n");
		sprintf((char *)make_invoice_result->errinfo,"开票数据及税盘环境校验均成功,测试接口不执行最终开具");
		result = DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
		goto FreeAndExit;
	}
	if ((stp_fpxx->isRed) &&
		(stp_fpxx->fplx == FPLX_COMMON_ZYFP || (stp_fpxx->fplx == FPLX_COMMON_DZZP)) &&
		strcmp(stp_fpxx->redNum, "0000000000000000") == 0)
	{
		logout(INFO, "TAXLIB", "发票开具", "专票红冲开具,信息表编号不能为0000000000000000\r\n");
		sprintf((char *)make_invoice_result->errinfo, "专票红冲信息表编号不能为0000000000000000");
		result = DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
		goto FreeAndExit;

	}
	logout(INFO, "TAXLIB", "发票开具", "盘号：%s,发票数据校验通过开始底层开具\r\n", hDev->szDeviceID);
	if ((result = MakeInvoiceMain(stp_fpxx)) < 0)
	{
		sprintf((char *)make_invoice_result->errinfo, "%s", stp_fpxx->hDev->hUSB->szLastErrorDescription);
		if (stp_fpxx->need_restart == 1)
		{
			make_invoice_result->need_restart = 1;
		}
		logout(INFO, "TAXLIB", "发票开具", "盘号：%s,最后一步开票底层操作失败,result = %d\r\n", hDev->szDeviceID, result);
		result = DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
		goto FreeAndExit;
	}
	logout(INFO, "TAXLIB", "发票开具", "盘号：%s,底层开具成功,发票代码：%s,发票号码：%s\r\n", hDev->szDeviceID, stp_fpxx->fpdm, stp_fpxx->fphm);
	make_invoice_result->fplxdm=stp_fpxx->fplx;
	memcpy(make_invoice_result->fpdm, stp_fpxx->fpdm, sizeof(make_invoice_result->fpdm));
	memcpy(make_invoice_result->fphm, stp_fpxx->fphm, sizeof(make_invoice_result->fphm));
	memcpy(make_invoice_result->kpsj, stp_fpxx->kpsj_F2, sizeof(make_invoice_result->kpsj));
	memcpy(make_invoice_result->hjje, stp_fpxx->je, sizeof(make_invoice_result->hjje));
	memcpy(make_invoice_result->hjse, stp_fpxx->se, sizeof(make_invoice_result->hjse));
	memcpy(make_invoice_result->jshj, stp_fpxx->jshj, sizeof(make_invoice_result->jshj));
	memcpy(make_invoice_result->jym, stp_fpxx->jym, sizeof(make_invoice_result->jym));
	memcpy(make_invoice_result->mwq, stp_fpxx->mw, sizeof(make_invoice_result->mwq));
	memcpy(make_invoice_result->fpqqlsh, stp_fpxx->fpqqlsh, sizeof(make_invoice_result->fpqqlsh));

	if (stp_fpxx->need_restart == 1)
	{
		make_invoice_result->need_restart = 1;
	}
	if ((stp_fpxx->fplx == FPLX_COMMON_DZFP) && (strlen(stp_fpxx->hDev->szPubServer)!=0))
	{
		if (strlen(stp_fpxx->pubcode) > 0)
		{
			char tqmxx[100] = { 0 };
			char *p;
			char url[5000] = { 0 };
			char time_str[20] = { 0 };
			p = strstr(stp_fpxx->pubcode, ";");
			if (p != NULL)
			{
				p = strstr(p + 1, ";");
				if (p != NULL)
				{
					memcpy(time_str, stp_fpxx->kpsj_standard, 8);
					memcpy(tqmxx, p + 1, strlen(stp_fpxx->pubcode) - (p - stp_fpxx->pubcode));
					//if (GetUPServerURL(1, stp_fpxx->hDev->szRegCode, szUPServURL) == 0)
					struct UploadAddrModel model;
					memset(&model,0,sizeof(struct UploadAddrModel));
					if (GetTaServerURL(TAADDR_PLATFORM, stp_fpxx->hDev->szRegCode, &model) == RET_SUCCESS)
					{
						sprintf(url, "%s/preview.html?code=%s_%s_%s_%s", model.szTaxAuthorityURL, stp_fpxx->fpdm, stp_fpxx->fphm, time_str, tqmxx);
						//_WriteLog(LL_FATAL, "url  %s\n", url);
						strcpy((char *)make_invoice_result->ofdurl, url);
					}
					else
					{
						logout(INFO, "TAXLIB", "发票开具", "发票开具成功，发票代码：%s,号码:%s,通过区域代码获取ofdurl地址失败\r\n", stp_fpxx->fpdm, stp_fpxx->fphm);
					}
				}
				else
				{
					logout(INFO, "TAXLIB", "发票开具", "发票开具成功，发票代码：%s,号码:%s,获取第二个；失败\r\n", stp_fpxx->fpdm, stp_fpxx->fphm);
				}
			}
			else
			{
				logout(INFO, "TAXLIB", "发票开具", "发票开具成功，发票代码：%s,号码:%s,获取第一个；失败\r\n", stp_fpxx->fpdm, stp_fpxx->fphm);
			}
		}
		else
		{
			logout(INFO, "TAXLIB", "发票开具", "发票开具成功，发票代码：%s,号码:%s,公共服务平台pubcode参数有误\r\n", stp_fpxx->fpdm, stp_fpxx->fphm);
		}
	}


	make_invoice_result->fpdzsyh = stp_fpxx->dzsyh;
	UTF8ToGBKBase64(stp_fpxx->bz, strlen(stp_fpxx->bz), (char *)make_invoice_result->bz);




	HFPXX stp_fpxx_tmp = MallocFpxx();
	stp_fpxx_tmp->hDev = hDev;
	if (QueryInvInfo(hDev, (char *)make_invoice_result->fpdm, (char *)make_invoice_result->fphm, make_invoice_result->fpdzsyh, stp_fpxx_tmp) < 0)
	{
		FreeFpxx(stp_fpxx_tmp);
		_WriteLog(LL_WARN, "盘号:%s,发票开具后查询发票失败, fpdm:%s fphm:%s", hDev->szDeviceID, make_invoice_result->fpdm, make_invoice_result->fphm);
	}
	else
	{
		fpxx_to_json_base_fpsjbbh_v102(stp_fpxx_tmp, &make_invoice_result->scfpsj, DF_FPQD_Y, DF_FP_JSON);
		FreeFpxx(stp_fpxx_tmp);
	}
	//memcpy(make_invoice_result->bz, stp_fpxx->bz, sizeof(make_invoice_result->bz));
	//_WriteLog(LL_INFO, "开票结果备注信息内容：%s\n", make_invoice_result->bz);
	result = 0;
	//_WriteLog(LL_INFO, "开票结果结构体赋值完成,scfpsj:%s\n", make_invoice_result->scfpsj);
FreeAndExit:
	if (stp_fpxx)
	{
		//_WriteLog(LL_INFO, "释放发票结构体\n");
		FreeFpxx(stp_fpxx);
	}
	//asleep(10);
	return result;

}

int aisino_waste_invoice(HDEV hDev, unsigned char inv_type, char  *fpdm, char *fphm, char *zfr, struct _cancel_invoice_result *cancel_invoice_result)
{
	//char errinfo[1024] = {0};
	int result = -1;
	ClearLastError(hDev->hUSB);
	result = LoadAisinoDevInfo(hDev, hDev->hUSB, false);
	if (result < 0)
	{
		return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
	}

	if ((inv_type != FPLX_COMMON_PTFP) && (inv_type != FPLX_COMMON_ZYFP))
	{
		sprintf((char *)cancel_invoice_result->errinfo, "发票作废仅支持普通发票和专用发票");
		return DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
	}

	//char szFPDM[32] = "3200142140", szFPHM[24] = "369595";
	_WriteLog(LL_INFO, "Try Waste invoice: FPDM:%s FPHM:%s", fpdm, fphm);
	HFPXX fpxx = MallocFpxx();
	fpxx->hDev = hDev;
	//此处追加证书及口令认证
	//AddLoadAisinoCert(device, device->hUSB);
	FillDevInfoCert(hDev, 1);
	int nRet = -1;
	int nChildRet = -1;
	while (1) {
		if ((nChildRet = QueryInvInfo(hDev, fpdm, fphm, 0, fpxx)) < 0) {
			_WriteLog(LL_DEBUG, "WasteInvoice QueryInvInfo failed, nRet:%d", nChildRet);
			sprintf((char *)cancel_invoice_result->errinfo, "根据发票代码和号码没有查询到发票相关信息");
			nRet = -1;
			break;
		}
		if (fpxx->zfbz) {
			_WriteLog(LL_WARN, "Invoice has been wasted,FPDM:%s FPHM:%s", fpdm, fphm);
			sprintf((char *)cancel_invoice_result->errinfo, "根据发票代码和号码没有查询到发票,或已经作废了,不允许重复作废");
			nRet = -2;
			break;
		}
		if (fpxx->fplx != inv_type)
		{
			_WriteLog(LL_DEBUG, "WasteInvoice QueryInvInfo failed");
			sprintf((char *)cancel_invoice_result->errinfo, "传入的发票类型与实际开具的发票类型不符");
			nRet = -1;
			break;
		}
		if (strcmp(fpxx->zyspsmmc, "*机动车*") == 0)
		{			
			sprintf((char *)cancel_invoice_result->errinfo, "机动车发票不允许作废");
			_WriteLog(LL_DEBUG, "%s", (char *)cancel_invoice_result->errinfo);
			nRet = -1;
			break;
		}
		struct Spxx *stp_Spxx = NULL;
		int i;
		stp_Spxx = fpxx->stp_MxxxHead->stp_next;
		//_WriteLog(LL_FATAL, "商品数量=%d\n", Spxx_num);
		for (i = 0; i < fpxx->spsl; i++)
		{
			//_WriteLog(LL_FATAL,"组包第%d行商品\n",i+1);
			if (stp_Spxx == NULL)
			{
				break;
			}
			if (memcmp(stp_Spxx->spbh, "10701010", strlen("10701010")) == 0) {
				sprintf((char *)cancel_invoice_result->errinfo, "开具成品油发票暂不支持作废");
				FreeFpxx(fpxx);
				return -1;
			}
			if ((memcmp(stp_Spxx->spbh, "1090305", strlen("1090305")) == 0) ||
				(memcmp(stp_Spxx->spbh, "1090306", strlen("1090306")) == 0) ||
				(memcmp(stp_Spxx->spbh, "1090307", strlen("1090307")) == 0) ||
				(memcmp(stp_Spxx->spbh, "1090309", strlen("1090309")) == 0) ||
				(memcmp(stp_Spxx->spbh, "1090312", strlen("1090312")) == 0) ||
				(memcmp(stp_Spxx->spbh, "1090315", strlen("1090315")) == 0)) {
				sprintf((char *)cancel_invoice_result->errinfo, "机动车发票不允许作废");
				FreeFpxx(fpxx);
				return -2;
			}
			if ((memcmp(stp_Spxx->spbh, "103040501", strlen("103040501")) == 0) ||
				(memcmp(stp_Spxx->spbh, "103040502", strlen("103040502")) == 0) ||
				(memcmp(stp_Spxx->spbh, "103040503", strlen("103040503")) == 0) ||
				(memcmp(stp_Spxx->spbh, "103040504", strlen("103040504")) == 0)) {
				sprintf((char *)cancel_invoice_result->errinfo, "电子烟发票不允许作废");
				FreeFpxx(fpxx);
				return -3;
			}
			//wang 02 01 稀土编码并入矿产品
			if ((memcmp(stp_Spxx->spbh, "102", strlen("102")) == 0)) {
				sprintf((char *)cancel_invoice_result->errinfo, "矿产品发票不允许作废");
				FreeFpxx(fpxx);
				return -4;
			}
			stp_Spxx = stp_Spxx->stp_next;
		}
		////测试并不作废
		//break;
		int result = WasteInvoice(fpxx);
		if (result< 0)
		{
			cancel_invoice_result->need_restart = 1;
			sprintf((char *)cancel_invoice_result->errinfo, "底层作废失败,%s", hDev->hUSB->szLastErrorDescription);
			//sprintf((char *)cancel_invoice_result->errinfo, "发票底层作废失败,错误排查代码：%s",hDev->hUSB->szLastErrorDescription);
			//report_event(fpxx->hDev->szDeviceID, "发票底层作废失败", (char *)cancel_invoice_result->errinfo, result);
			logout(INFO, "TAXLIB", "发票作废", "发票作废底层提示失败,FPDM:%s FPHM:%s,result = %d,%s\r\n", fpdm, fphm, result, (char *)cancel_invoice_result->errinfo);
			nRet = -3;
			break;
		}
		//if (device->bTrainFPDiskType == 0)
		//{
		//	_WriteLog(LL_INFO, "非测试盘需重新上传发票信息\n");
		//	//if (InvUp_UploadAndUpdate(device, fpxx, true, 1, errinfo) < 0)
		//	//{
		//	//	//nRet = 0;
		//	//	logout(INFO, "taxlib", "fpzf","发票作废成功,数据上传失败，等待下次上传,FPDM:%s FPHM:%s,失败原因【%s】\r\n", fpdm, fphm,errinfo);
		//	//	//break;
		//	//}
		//}
		QueryInvInfo(hDev, fpdm, fphm, 0, fpxx);
		cancel_invoice_result->fplxdm = inv_type;
		memcpy(cancel_invoice_result->fpdm, fpdm, strlen(fpdm));
		memcpy(cancel_invoice_result->fphm, fphm, strlen(fphm));
		//memcpy(cancel_invoice_result->zfsj, fpxx->zfsj, strlen(fpxx->zfsj));
		TranslateStandTime(2, fpxx->zfsj, (char *)cancel_invoice_result->zfsj);
		memcpy(cancel_invoice_result->kpsj, fpxx->kpsj_F2, strlen(fpxx->kpsj_F2));
		memcpy(cancel_invoice_result->hjje, fpxx->je, strlen(fpxx->je));
		memcpy(cancel_invoice_result->hjse, fpxx->se, strlen(fpxx->se));
		memcpy(cancel_invoice_result->jshj, fpxx->jshj, strlen(fpxx->jshj));
		memcpy(cancel_invoice_result->jym, fpxx->jym, strlen(fpxx->jym));
		memcpy(cancel_invoice_result->mwq, fpxx->mw, strlen(fpxx->mw));
		fpxx_to_json_base_fpsjbbh_v102(fpxx, &cancel_invoice_result->scfpsj, DF_FPQD_Y, DF_FP_JSON);
		nRet = 0;
		break;
	}
	FreeFpxx(fpxx);
	return nRet;
}

int aisino_cancel_null_invoice_to_plate(HDEV hDev, struct _cancel_invoice_result *cancel_invoice_result)
{
	//char errinfo[1024] = {0};
	int result = -1;
	ClearLastError(hDev->hUSB);
	result = LoadAisinoDevInfo(hDev, hDev->hUSB, false);
	if (result < 0)
	{
		return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
	}

	if ((cancel_invoice_result->fplxdm != FPLX_COMMON_PTFP) && (cancel_invoice_result->fplxdm != FPLX_COMMON_ZYFP))
	{
		sprintf((char *)cancel_invoice_result->errinfo, "发票作废仅支持普通发票和专用发票");
		return DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
	}
	if ((strlen((char *)cancel_invoice_result->zfr) == 0) || (strlen((char *)cancel_invoice_result->zfr) > 16))
	{
		sprintf((char *)cancel_invoice_result->errinfo, "作废人长度有误");
		return DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
	}
	result = IsGBK((char *)cancel_invoice_result->zfr);
	if (result != 1)
	{
		sprintf((char *)cancel_invoice_result->errinfo, "作废人编码格式有误");
		return DF_TAX_ERR_CODE_USE_ILL_CHAR_OR_UTF8;//编码格式错误
	}
	if (cancel_invoice_result->zfzs < 1)
	{
		sprintf((char *)cancel_invoice_result->errinfo, "作废总数有误");
		return DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
	}

	//char szFPDM[32] = "3200142140", szFPHM[24] = "369595";
	_WriteLog(LL_INFO, "Try Waste invoice: FPDM:%s FPHM:%s, ZFZS:%d", cancel_invoice_result->fpdm, cancel_invoice_result->fphm, cancel_invoice_result->zfzs);


	FillDevInfoCert(hDev, 1);

	uint8 bAisinoFPLX;
	CommonFPLX2AisinoFPLX(cancel_invoice_result->fplxdm, &bAisinoFPLX);


	result = AisinoBlankWaste(hDev, bAisinoFPLX, (char *)cancel_invoice_result->fpdm, (char *)cancel_invoice_result->fphm, 
		(char *)cancel_invoice_result->zfr, cancel_invoice_result->zfzs, &cancel_invoice_result->need_restart);
	if (result < 0)
	{
		sprintf((char *)cancel_invoice_result->errinfo, "%s", hDev->hUSB->szLastErrorDescription);
		return DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
	}
	return 0;
}

int aisino_read_tax_rate(HDEV hDev, struct _plate_infos *plate_infos)
{
	int nRet = -2;
	ClearLastError(hDev->hUSB);
	return nRet;
}


int aisino_copy_report_data(HDEV hDev, uint8 inv_type,char *errinfo)
{
	int result;
	uint8 bAisino_FPLX;
	uint8 bFplx_Report_Common = 0;
	uint8 bFplx_Report_Aisino = 0;
	ClearLastError(hDev->hUSB);
	result = LoadAisinoDevInfo(hDev, hDev->hUSB, false);
	if (result < 0)
	{
		return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
	}
	if (hDev->bTrainFPDiskType != 0)
	{
		_WriteLog(LL_WARN, "非正式盘不执行抄报清卡动作");
		sprintf(errinfo, "测试盘不进行抄报清卡");
		return -1;
	}
	CommonFPLX2AisinoFPLX(inv_type, &bAisino_FPLX);
	if (FPLX_AISINO_DZFP != bAisino_FPLX && FPLX_AISINO_ZYFP != bAisino_FPLX &&FPLX_AISINO_PTFP != bAisino_FPLX) 
	{
		_WriteLog(LL_WARN, "[+] Unsupport invoice's aisino-fplx:%d", bAisino_FPLX);
		return -1;
	}	
	AisinoFPLX2PrivateFPLX(bAisino_FPLX, &bFplx_Report_Common, &bFplx_Report_Aisino);
	if ((result = PeriodTaxReport(hDev->hUSB, bFplx_Report_Aisino)) < 0)
	{
		return -2;
	}
	if (result == 202)
	{
		return -3;
	}
	return 0;
}

int aisino_report_clear(HDEV hDev, uint8 inv_type,char *errinfo)
{
	int result;
	uint8 bAisino_FPLX;
	ClearLastError(hDev->hUSB);
	result = LoadAisinoDevInfo(hDev, hDev->hUSB, true);
	if (result < 0)
	{
		return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
	}
	if (hDev->bTrainFPDiskType != 0)
	{
		_WriteLog(LL_WARN, "非正式盘不执行抄报清卡动作");
		sprintf(errinfo,"测试盘不进行抄报清卡");
		return -1;
	}
	//_WriteLog(LL_WARN, "执行上报汇总前先加载证书");
	CommonFPLX2AisinoFPLX(inv_type, &bAisino_FPLX);
	if (FPLX_AISINO_DZFP != bAisino_FPLX && FPLX_AISINO_ZYFP != bAisino_FPLX &&FPLX_AISINO_PTFP != bAisino_FPLX)
	{
		_WriteLog(LL_WARN, "[-] Unsupport invoice's aisino-fplx:%d", bAisino_FPLX);
		sprintf(errinfo, "存在不支持的发票类型");
		return -2;
	}
	_WriteLog(LL_WARN, "[+] 开始执行上报汇总 aisino-fplx:%d", bAisino_FPLX);
	result = ReporSummarytMain(hDev, bAisino_FPLX);
	if (result < 0)
	{
		strcpy(errinfo, hDev->hUSB->errinfo);//网络报错存储在errinfo
		//report_event(hDev->szDeviceID, "上报汇总失败", (char *)errinfo, result);
		_WriteLog(LL_WARN, "[-] 上报汇总失败 aisino-fplx:%d", bAisino_FPLX);
		return -3;
	}
	//_WriteLog(LL_WARN, "[+] 开上报汇总成功 aisino-fplx:%d", bAisino_FPLX);
	//_WriteLog(LL_WARN, "[+] 开始执行清卡反写 aisino-fplx:%d", bAisino_FPLX);
	result = ClearCardMain(hDev, bAisino_FPLX);
	if (result < 0)
	{
		strcpy(errinfo, hDev->hUSB->errinfo);//网络报错存储在errinfo
		_WriteLog(LL_WARN, "[-] 清卡反写失败 aisino-fplx:%d,result = %d,errinfo = %s", bAisino_FPLX, result, errinfo);
		//report_event(hDev->szDeviceID, "清卡反写失败", (char *)errinfo, result);
		return -4;
	}
	return 0;
}

int aisino_fpdm_fphm_update_invs(HDEV hDev, char  *fpdm, char *fphm, int dzsyh, char **inv_json)
{
	int ret = -1;
	char errinfo[1024] = {0};
	int bUpdateFlag;
	ClearLastError(hDev->hUSB);
	FillDevInfoCert(hDev, 1);
	HFPXX fpxx = MallocFpxx();
	fpxx->hDev = hDev;
	_WriteLog(LL_INFO, "通过发票代码%s,发票号码%s,获取发票数据\n", fpdm, fphm);
	if ((ret = QueryInvInfo(hDev, fpdm, fphm, dzsyh, fpxx)) < 0)
	{
		_WriteLog(LL_INFO, "发票获取失败,ret = %d\n", ret);
		goto get_end;
	}
	if (fpxx->bIsUpload == 1)
	{
		_WriteLog(LL_INFO, "此票已上传，后续不再反写仅测试用\n");
		bUpdateFlag = 0;
	}
	else
	{
		_WriteLog(LL_INFO, "此票未上传，上传完成后反写\n");
		bUpdateFlag = 1;
	}
	fpxx_to_json_base_fpsjbbh_v102(fpxx, inv_json, DF_FPQD_Y, DF_FP_BASE);
	if (hDev->bTrainFPDiskType == 0)
	{
		if (InvUp_UploadAndUpdate(hDev, fpxx, true, bUpdateFlag) < 0)
		{
			_WriteLog(LL_INFO, "发票上传失败,错误信息【%s】\n",errinfo);
			
		}

		//_WriteLog(LL_INFO, "发票上传完成,拷贝数据\n");
		//_WriteLog(LL_INFO, "开票时间：%s，发票代码：%s，发票号码：%s，发票类型代码：%03d\n", fpxx->kpsj_F2, fpxx->fpdm, fpxx->fphm, fpxx->fplx);
		//struct _make_invoice_result make_invoice_result;
		//memcpy((char *)make_invoice_result.kpsj, fpxx->kpsj_F2,19);
		//memcpy((char *)make_invoice_result.fpdm, fpxx->fpdm,12);
		//memcpy((char *)make_invoice_result.fphm, fpxx->fphm,12);
		//make_invoice_result.fplxdm = fpxx->fplx;
		//_WriteLog(LL_INFO, "数据拷贝完成\n");
		
	}
	else
	{
		logout(INFO, "TAXLIB", "发票上传", "[+]测试盘不需要发票上传\r\n");
	}

	//_WriteLog(LL_INFO, "获取发票信息成功,组包到json\n");
	ret = 0;
get_end:
	if (fpxx)
	{
		//_WriteLog(LL_INFO, "释放发票结果体指针\n");
		FreeFpxx(fpxx);
	}

	return ret;
}

// func0 ZYFP 红字表申请并获取红字信息表编号
// func1 ZYFP Fpxx结构体信息对比认证
int aisino_upload_check_redinvform(HDEV hDev, uint8 inv_type, char* inv_data, struct _plate_infos *plate_infos, struct _askfor_tzdbh_result *askfor_tzdbh_result)
{
	int result;
	int count;
	//uint8 bFuncNo = 1;
	int nRet = -10;
	char szTZDBH[36] = "";
	ClearLastError(hDev->hUSB);
	result = AisinoGetDeivceTime(hDev->hUSB, hDev->szDeviceTime); //更新税盘时间
	if (result < 0)
	{
		logout(INFO, "TAXLIB", "红字信息表申请", "读取税盘时间异常%d\r\n", result);
		return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
	}
	HFPXX fpxx = MallocFpxxLinkDev(hDev);
	if (!fpxx) {
		nRet = -1;
		sprintf((char *)askfor_tzdbh_result->errinfo, "系统内存申请失败,机柜可能存在故障,请重启机柜");
		goto FreeAndExit;
	}
	if (inv_data != NULL)
	{
		str_replace(inv_data, "\\", "[@*br/*@]");//排查json中的\转义字符
		result = check_invoice_str((unsigned char *)inv_data);
		if (result < 0)
		{
			FreeFpxx(fpxx);
			sprintf((char *)askfor_tzdbh_result->errinfo, "发票JSON数据中存在特殊字符无法解析,例如&和\\");
			return DF_TAX_ERR_CODE_USE_ILL_CHAR_OR_UTF8;
		}
		//result = judge_code(inv_data, strlen(inv_data));
		result = IsGBK(inv_data);
		if (result != 1)
		{
			FreeFpxx(fpxx);
			sprintf((char *)askfor_tzdbh_result->errinfo, "发票JSON数据汉字编码格式错误,非GBK编码");
			return DF_TAX_ERR_CODE_USE_ILL_CHAR_OR_UTF8;//编码格式错误
		}

		fpxx->isHzxxb = true;
		//_WriteLog(LL_INFO, "解析发票JSON数据,%s\n", inv_data);
		if ((result = AnalyzeJsonBuff(hDev->bDeviceType, inv_data, fpxx, (char *)askfor_tzdbh_result->errinfo)) != 0)
		{
			logout(INFO, "TAXLIB", "红字信息表申请", "发票解析失败,错误代码%d\r\n", result);
			FreeFpxx(fpxx);
			return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
		}
		if (fpxx->hzxxbsqsm != 0)
		{
			//购买方申请红字新版交易销购方信息有误填反
			if ((strcmp(fpxx->xfsh, hDev->szCommonTaxID) == 0) || (strcmp(fpxx->xfmc, hDev->szCompanyName) == 0))			{
				logout(INFO, "TAXLIB", "红字信息表申请", "购买方发起红字信息表申请,销方购方信息有误\r\n", result);
				FreeFpxx(fpxx);
				return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
			}
			strcpy(fpxx->gfsh, hDev->szCommonTaxID);
			strcpy(fpxx->gfmc, hDev->szCompanyName);
		}
		else
		{
			strcpy(fpxx->xfsh, hDev->szCommonTaxID);
			strcpy(fpxx->xfmc, hDev->szCompanyName);
		}
		//更新必要税盘信息到发票信息
		fpxx->kpjh = hDev->uICCardNo;
		fpxx->hDev = hDev;
		//格式化各种格式时间
		UpdateFpxxAllTime(hDev->szDeviceTime, fpxx);

		//result = check_fpxx_aisino(fpxx, plate_infos, (char *)askfor_tzdbh_result->errinfo);
		//if (result < 0)
		//{
		//	_WriteLog(LL_INFO, "check_fpxx_aisino failed\n");
		//	goto FreeAndExit;
		//}
		//// update by type
		//if (DEVICE_AISINO == hDev->bDeviceType) {
		//	if (AisinoUpdateNodeInfo(fpxx, hDev))
		//	{
		//		_WriteLog(LL_INFO, "替换测试盘开票数据出错\n");
		//		result = DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
		//		goto FreeAndExit;
		//	}
		//}
		//if (CheckFpxxNode(fpxx, hDev, (char *)askfor_tzdbh_result->errinfo))
		//{
		//	_WriteLog(LL_INFO, "检验发票数据是否正确出错\n");
		//	result = DF_TAX_ERR_CODE_INV_DATA_DISACCORD;
		//	goto FreeAndExit;
		//}
		if (fpxx->dzsyh != 0)//未测试，目的兼容5%税率发票部分申请红字信息表，报征税模式不一致兼容
		{
			_WriteLog(LL_INFO, "查询蓝票数据并更新\n");
			HFPXX fpxx_tmp = MallocFpxxLinkDev(hDev);
			if ((result = QueryInvInfo(hDev, (char *)askfor_tzdbh_result->lzfpdm, (char *)askfor_tzdbh_result->lzfphm, askfor_tzdbh_result->dzsyh, fpxx_tmp)) < 0)
			{
				_WriteLog(LL_INFO, "发票获取失败,ret = %d\n", result);
				sprintf((char *)askfor_tzdbh_result->errinfo, "原发票代码号码查询失败");
				FreeFpxx(fpxx_tmp);
				goto FreeAndExit;
			}
			memset(fpxx->ssyf, 0, sizeof(fpxx->ssyf));
			memcpy(fpxx->ssyf, fpxx_tmp->kpsj_standard, 6);
			fpxx->zyfpLx = fpxx_tmp->zyfpLx;
			memset(fpxx->yysbz, 0, sizeof(fpxx->yysbz));
			strcpy(fpxx->yysbz, fpxx_tmp->yysbz);
			FreeFpxx(fpxx_tmp);
		}
		memcpy(askfor_tzdbh_result->lzfpdm, fpxx->blueFpdm, strlen(fpxx->blueFpdm));
		memcpy(askfor_tzdbh_result->lzfphm, fpxx->blueFphm, strlen(fpxx->blueFphm));
	}	
	else
	{
		fpxx->hDev = hDev;
		_WriteLog(LL_INFO, "通过发票代码%s,发票号码%s,获取发票数据\n", askfor_tzdbh_result->lzfpdm, askfor_tzdbh_result->lzfphm);
		if ((result = QueryInvInfo(hDev, (char *)askfor_tzdbh_result->lzfpdm, (char *)askfor_tzdbh_result->lzfphm, askfor_tzdbh_result->dzsyh, fpxx)) < 0)
		{
			_WriteLog(LL_INFO, "发票获取失败,ret = %d\n", result);
			sprintf((char *)askfor_tzdbh_result->errinfo, "通过发票代码%s,发票号码%s,获取发票数据失败", askfor_tzdbh_result->lzfpdm, askfor_tzdbh_result->lzfphm);
			goto FreeAndExit;
		}
		_WriteLog(LL_INFO,"QueryInvInfo 发票获取成功\n");
		
		//格式化各种格式时间

		memcpy(fpxx->ssyf, fpxx->kpsj_standard,6);
		
		memset(fpxx->kpsj_standard, 0, sizeof fpxx->kpsj_standard);
		memset(fpxx->kpsj_F1, 0, sizeof fpxx->kpsj_F1);
		memset(fpxx->kpsj_F2, 0, sizeof fpxx->kpsj_F2);
		memset(fpxx->kpsj_F3, 0, sizeof fpxx->kpsj_F3);

		UpdateFpxxAllTime(hDev->szDeviceTime, fpxx);		

		//_WriteLog(LL_INFO, "发票所属月份：%s\n", fpxx->ssyf);
		//_WriteLog(LL_INFO, "当前税盘时间：%s\n", fpxx->kpsj_standard);

		strcpy(fpxx->blueFpdm, (char *)askfor_tzdbh_result->lzfpdm);
		strcpy(fpxx->blueFphm, (char *)askfor_tzdbh_result->lzfphm);

		MemoryFormart(MF_TRANSLATE_RED, fpxx->je, strlen(fpxx->je));
		MemoryFormart(MF_TRANSLATE_RED, fpxx->se, strlen(fpxx->se));

		struct Spxx *stp_Spxx = NULL;
		if (strcmp(fpxx->qdbj, "Y") == 0)
		{
			stp_Spxx = fpxx->stp_MxxxHead;
			MemoryFormart(MF_TRANSLATE_RED, stp_Spxx->je, strlen(stp_Spxx->je));
			MemoryFormart(MF_TRANSLATE_RED, stp_Spxx->se, strlen(stp_Spxx->se));
			memset(stp_Spxx->spmc, 0, sizeof(stp_Spxx->spmc));
			strcpy(stp_Spxx->spmc, "详见对应正数发票及清单");
			memset(stp_Spxx->dj, 0, sizeof(stp_Spxx->dj));
			memset(stp_Spxx->sl, 0, sizeof(stp_Spxx->sl));
			memset(stp_Spxx->jldw, 0, sizeof(stp_Spxx->jldw));
			memset(stp_Spxx->ggxh, 0, sizeof(stp_Spxx->ggxh));
		}
		else
		{
			int i;
			int Spxx_num;
			
			stp_Spxx = fpxx->stp_MxxxHead->stp_next;
			Spxx_num = fpxx->spsl;

			//_WriteLog(LL_FATAL, "商品数量=%d\n", Spxx_num);
			for (i = 0; i < Spxx_num; i++)
			{
				//_WriteLog(LL_FATAL,"组包第%d行商品\n",i+1);
				if (stp_Spxx == NULL)
				{
					break;
				}

				if (strlen(stp_Spxx->dj)>0)
				{
					if (atoi(stp_Spxx->hsjbz) == 1)
					{
						char dj_tmp[20] = { 0 };
						PriceRemoveTax(stp_Spxx->dj, stp_Spxx->slv, 15, dj_tmp);  
						PriceRound(dj_tmp, 15, stp_Spxx->dj);
						memset(stp_Spxx->hsjbz, 0, sizeof(stp_Spxx->hsjbz));
						strcpy(stp_Spxx->hsjbz, "0");
					}
				}
				if (strlen(stp_Spxx->sl) > 0)
				{
					MemoryFormart(MF_TRANSLATE_RED, stp_Spxx->sl, strlen(stp_Spxx->sl));
				}
				MemoryFormart(MF_TRANSLATE_RED, stp_Spxx->je, strlen(stp_Spxx->je));
				MemoryFormart(MF_TRANSLATE_RED, stp_Spxx->se, strlen(stp_Spxx->se));

				if (atoi(stp_Spxx->fphxz) != 0)
				{
					sprintf((char *)askfor_tzdbh_result->errinfo,"暂不支持商品行非正常行发票整票红冲");
					_WriteLog(LL_INFO, "%s\n", askfor_tzdbh_result->errinfo);
					result = DF_TAX_ERR_CODE_INV_DATA_DISACCORD;
					goto FreeAndExit;
				}

				stp_Spxx = stp_Spxx->stp_next;
			}
		}
	}



	FillDevInfoCert(hDev, 1);

	//if (hDev->bTrainFPDiskType != 0)
	//{
	//	_WriteLog(LL_INFO, "测试盘无法申请红字信息表编号\n");
	//	sprintf((char *)askfor_tzdbh_result->errinfo, "测试盘无法申请红字信息表编号");
	//	result = DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
	//	goto FreeAndExit;
	//}

	char errinfo[1024] = { 0 };
	if ((result = UploadTZDBH(fpxx, szTZDBH)) < 0)
	{
		strcpy(errinfo, hDev->hUSB->errinfo);//网络出错存储在errinfo
		_WriteLog(LL_FATAL, "[-] UploadTZDBH failed!通知单编号申请失败,errinfo ：%s,result = %d\r\n",errinfo,result);
		if ((strcmp(errinfo, "红字信息表金额(税额)大于原蓝票可开具红字信息表的金额(税额)") == 0) ||
			(strcmp(errinfo, "红字发票信息表已成功上传,无需重复上传") == 0) || 
			(strcmp(errinfo, "业务层数据组包发送失败,nChildRet = -7") == 0)
			)
		{
			_WriteLog(LL_FATAL, "直接查询已申请红字信息表\r\n");
			result = RedInvOnlineCheck_TZD(fpxx,&askfor_tzdbh_result->red_inv);
			if (result == 0)
			{
				_WriteLog(LL_FATAL, "局端查询大于蓝票金额，直接返回红字信息表编号\r\n");				
				memcpy((char *)askfor_tzdbh_result->tzdbh, fpxx->redNum, strlen(fpxx->redNum));
				memcpy((char *)askfor_tzdbh_result->sqlsh, fpxx->redNum_serial, strlen(fpxx->redNum_serial));
				nRet = 0;
				goto FreeAndExit;
			}
		}
		sprintf((char *)askfor_tzdbh_result->errinfo, "通知单编号申请失败,结果%d,原因：%s", result, errinfo);
		nRet = DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
		goto FreeAndExit;
	}
	_WriteLog(LL_INFO, "[+] UploadTZDBH successful,通知单编号申请成功  red invoice tzdbh:%s", szTZDBH);
	memset(fpxx->redNum,0,sizeof(fpxx->redNum));
	memcpy(fpxx->redNum, szTZDBH,24);
	sleep(1);
	for (count = 0; count < 3; count++)  // 3次足够?,查询超时大约为18秒
	{
		sleep(count * 5);  //这里要等下服务器处理，如果是刚报送上传完就查询，肯定返回1=处理中
		memset(askfor_tzdbh_result->errinfo, 0, sizeof(askfor_tzdbh_result->errinfo));
		if (RedInvOnlineCheck_ZY(fpxx, &askfor_tzdbh_result->red_inv) < 0)
		{			
			sprintf((char *)askfor_tzdbh_result->errinfo, "红字信息表申请数据上传完成,红字信息表编号：%s,查询审核状态未成功,请核实!", szTZDBH);
			_WriteLog(LL_FATAL, "%s\r\n", askfor_tzdbh_result->errinfo);
			nRet = -4;
			continue;
		}
		nRet = 0;
		memcpy((char *)askfor_tzdbh_result->sqlsh, fpxx->redNum_serial, strlen(fpxx->redNum_serial));
		memcpy((char *)askfor_tzdbh_result->tzdbh, fpxx->redNum, strlen(fpxx->redNum));
		_WriteLog(LL_INFO, "[+] RedInvOnlineCheck_ZY successful 红字信息表编号审核通过");
		break;
	}
FreeAndExit:
	if (fpxx)
		FreeFpxx(fpxx);
	return nRet;
}

// nDZSYH可选参数, 主要用于电子和普通发票红票开票前校验
int aisino_redinv_check(HDEV hDev, uint8 inv_type, char *szLZFPDM, char *szLZFPHM, uint32 nDZSYH,char *errinfo)
{
	uint8 bAisinoFPLX;
	char szRepMsg[512] = "";
	ClearLastError(hDev->hUSB);
	FillDevInfoCert(hDev, 1);

	if (hDev->bTrainFPDiskType)
	{
		sprintf(errinfo,"测试盘无法校验蓝字发票正确性");
		return -1;
	}

	CommonFPLX2AisinoFPLX(inv_type, &bAisinoFPLX);

	int nRet = RedInvOnlineCheck_PTDZ(hDev, bAisinoFPLX, szLZFPDM, szLZFPHM, nDZSYH, szRepMsg,
		sizeof(szRepMsg));

	if (nRet < 0) {
		printf("[-] FPDM:%s FPHM:%s, RedInvOnlineCheck failed,  msg:[%s]\n", szLZFPDM,
			szLZFPHM, szRepMsg);
		EncordingConvert("utf-8", "gbk", szRepMsg, strlen(szRepMsg), errinfo, 1024);
		_WriteLog(LL_INFO, "错误信息%s\n", errinfo);
		//_WriteLog(LL_INFO, "[-] FPDM:%s FPHM:%s, RedInvOnlineCheck failed,  msg:[%s]", szLZFPDM,
		//	szLZFPHM, szRepMsg);
		return -1;
	}
	printf("[+] FPDM:%s FPHM:%s, RedInvOnlineCheck successful, result:[%s]\n", szLZFPDM,
		szLZFPHM, szRepMsg);

	
	//_WriteLog(LL_INFO, "[+] FPDM:%s FPHM:%s, RedInvOnlineCheck successful, result:[%s]", szLZFPDM,
	//	szLZFPHM, szRepMsg);

	return 0;
}

int aisino_download_hzxxb_from_rednum(HDEV hDev, char *redNum, uint8 **inv_json, char *errinfo)
{
	
	int result;
	ClearLastError(hDev->hUSB);
	if (hDev->bTrainFPDiskType)
	{
		sprintf(errinfo, "测试盘不支持红字信息表查询功能");//该错误描述不能修改,ty_usb_m中有使用
		_WriteLog(LL_WARN, "%s", errinfo);
		return -1;
	}
	if (strlen(redNum) != 16)
	{
		sprintf(errinfo, "填入的红字信息表数据有误");
		_WriteLog(LL_WARN, "%s", errinfo);
		return -2;
	}
	FillDevInfoCert(hDev, 1);
	result = GetRedInvFromRedNum(hDev, redNum, inv_json);
	if (result < 0)
	{
		strcpy(errinfo, hDev->hUSB->szLastErrorDescription);
		_WriteLog(LL_WARN, "%s", errinfo);
	}
	return result;
}



int aisino_change_certpassword(HUSB hUSB, char *szOldPassword, char *szNewPassword, char *errinfo)//注意!!!这是单独接口,只能打开USB handle后单独调用.不能融入主流程，因为修改完之后需要重新连接设备
//int my_change_certpassword(HUSB hUsb, char *szOldPassword, char *szNewPassword)
{
	int nChildRet = -1;
	char szTaxID[256] = { 0 };
	bool bCryptBigIO = 0;
	ClearLastError(hUSB);
	_WriteLog(LL_INFO, "Try to set new cert's password:%s", szNewPassword);
	int nNewPwdLen = strlen(szNewPassword);
	if (nNewPwdLen != 8 ) {
		_WriteLog(LL_FATAL, "New cert password's length must [8,16]");
		sprintf(errinfo,"输入新的密码长度错误");
		return -1;
	}
	if (strcmp(szNewPassword, "12345678")!=0) {
		_WriteLog(LL_FATAL, "Can not set new password equal with defalt password '88888888'");
		sprintf(errinfo, "当前系统仅允许修改密码为12345678");
		return -2;
	}
	if (strcmp(szOldPassword, szNewPassword)==0) {
		sprintf(errinfo, "新密码与旧密码一致");
		return -3;
	}


	if (CheckHeadTailSpace(szNewPassword)) {
		_WriteLog(LL_FATAL, "Head and tail cannot been 0x20");
		sprintf(errinfo, "密码前后不得有空格");
		return -4;
	}
	nChildRet = ReadTaxDeviceID(hUSB, szTaxID);
	if (nChildRet > 0)
		bCryptBigIO = true;
	else if (0 == nChildRet)
		bCryptBigIO = false;
	else
		return -5;
	if ((nChildRet = SignAPI_OpenDevice(hUSB, bCryptBigIO, szOldPassword)) <= 0) {
		if (nChildRet > -100)
		{
			_WriteLog(LL_FATAL, "Crypt login failed, cannot modify new password,left count = %d", -nChildRet);
			sprintf(errinfo, "旧证书口令验证失败，剩余尝试次数%d次\n", -nChildRet);
		}
		else 	
		{
			sprintf(errinfo, "证书口令验证失败，可能原因为底层USB操作失败\n");
			_WriteLog(LL_FATAL, "%s",errinfo);
		}
		return -6;
	}
	if (SignAPI_ChangePassword(hUSB, bCryptBigIO, szOldPassword, szNewPassword) < 0) {
		_WriteLog(LL_FATAL, "Crypt login failed, cannot modify new password");
		sprintf(errinfo,"证书口令修改失败\n");
		return -7;
	}
	_WriteLog(LL_INFO, "Cert's password has been set new:[%s]", szNewPassword);
	return 0;
}

int aisino_redinv_tzdbh_cancel(HDEV hDev, char *szTZDBH,char *errinfo)
{
	char szOutMsg[4096] = { 0 };
	int result;
	ClearLastError(hDev->hUSB);
	if (hDev->bTrainFPDiskType)
	{
		sprintf(errinfo, "测试盘不支持红字信息表撤销动作");
		return -1;
	}
	if (strlen(szTZDBH) != 16)
	{
		sprintf(errinfo, "填入的红字信息表数据有误");
		_WriteLog(LL_WARN, "%s", errinfo);
		return -2;
	}
	FillDevInfoCert(hDev, 1);
	result = RedinvTZDCancel(hDev, szTZDBH, szOutMsg, sizeof(szOutMsg));
	if (result < 0)
	{
		strcpy(errinfo, hDev->hUSB->errinfo);//网络报错存储在errinfo
	}
	_WriteLog(LL_INFO, "RedinvTZDCancel:%s TZDBH:%s retmsg:%s", result == 0 ? "OK" : "Fail",
		szTZDBH, errinfo);
	if (strcmp("信息表状态已经是撤销状态，不允许重复撤销", errinfo) == 0)
	{
		_WriteLog(LL_INFO, "重复撤销默认认为正常");
		return 0;
	}
	return result;
}


int aisino_download_hzxxb_from_date_range(HDEV hDev, char *date_range, uint8 **tzdbh_data, char *errinfo)
{
	//char szOutMsg[4096];
	int result;
	ClearLastError(hDev->hUSB);
	if (hDev->bTrainFPDiskType)
	{
		sprintf(errinfo, "测试盘不支持红字信息表查询功能");//该错误描述不能修改,ty_usb_m中有使用
		return -1;
	}
	if (strlen(date_range) != 17)
	{
		sprintf(errinfo, "日期范围数据格式有误");
		return -2;
	}
	FillDevInfoCert(hDev, 1);
	char *szBuf = calloc(1, DEF_MAX_FPBUF_LEN);
	result = DownloadTZDBH(hDev, date_range, "", "", szBuf, DEF_MAX_FPBUF_LEN);
	if (result < 0)
	{
		strcpy(errinfo, hDev->hUSB->szLastErrorDescription);
		//sprintf(errinfo, "%s未查询到有效的红字信息表信息", date_range);
		//EncordingConvert("utf-8", "gbk", szBuf, strlen(szBuf), (char *)errinfo, 1024);
		free(szBuf);
		return result;
	}
	uint8 * tzdbh_data_s = calloc(1, DEF_MAX_FPBUF_LEN);

	result = get_fpxx_from_hzxxb_data_aisino_nisec(hDev, szBuf, tzdbh_data_s, DEF_MAX_FPBUF_LEN, errinfo);
	if (result == 0)
	{
		sprintf(errinfo, "没有满足条件的待下载数据");
		free(szBuf);
		free(tzdbh_data_s);
		return -1;
	}
	*tzdbh_data = tzdbh_data_s;
	free(szBuf);
	return result;
}


int aisino_download_hzxxb_from_date_range_new(HDEV hDev, char *date_range,char *gfsh, uint8 **tzdbh_data, char *errinfo)
{
	//char szOutMsg[4096];
	int result;
	ClearLastError(hDev->hUSB);
	if (hDev->bTrainFPDiskType)
	{
		sprintf(errinfo, "测试盘不支持红字信息表查询功能");//该错误描述不能修改,ty_usb_m中有使用
		return -1;
	}
	if (strlen(date_range) != 17)
	{
		sprintf(errinfo, "日期范围数据格式有误");
		return -2;
	}
	FillDevInfoCert(hDev, 1);
	char *szBuf = calloc(1, DEF_MAX_FPBUF_LEN);
	result = DownloadTZDBH(hDev, date_range, gfsh, "", szBuf, DEF_MAX_FPBUF_LEN);
	if (result < 0)
	{
		strcpy(errinfo, hDev->hUSB->szLastErrorDescription);
		//sprintf(errinfo, "%s未查询到有效的红字信息表信息", date_range);
		//EncordingConvert("utf-8", "gbk", szBuf, strlen(szBuf), (char *)errinfo, 1024);
		free(szBuf);
		return result;
	}
	uint8 * tzdbh_data_s = calloc(1, DEF_MAX_FPBUF_LEN);

	result = get_fpxx_from_hzxxb_data_aisino_nisec(hDev, szBuf, tzdbh_data_s, DEF_MAX_FPBUF_LEN, errinfo);
	if (result == 0)
	{
		sprintf(errinfo, "没有满足条件的待下载数据");
		free(szBuf);
		free(tzdbh_data_s);
		return -1;
	}
	*tzdbh_data = tzdbh_data_s;
	free(szBuf);
	return result;
}


int aisino_query_net_invoice_coil(HDEV hDev, char *date_range, char *inv_data, char *errinfo)
{
	int result = -1;
	char *p;
	int index = 0;
	ClearLastError(hDev->hUSB);
	if (hDev->bTrainFPDiskType)
	{
		sprintf(errinfo, "测试盘不支持票源查询下载功能");//该错误描述不能修改,ty_usb_m中有使用
		return -1;
	}

	FillDevInfoCert(hDev, 1);
	char coil_data[1024] = {0};
	char tmp_data[200] = {0};

	//sprintf(coil_data,"2|032001900204|51496536|5\r\n0|032001900205|51496537|10\r\n51|032001900206|51496538|15\r\n");
	struct HTTP hi;
	if ((result = CreateServerTalk(&hi, hDev)) < 0)
	{
		if (hDev->bBreakAllIO) {
			SetLastError(hDev->hUSB, ERR_IO_BREAK, "服务器交互过程中被其他接口中断");
			hDev->bBreakAllIO = 0;  //清除缓存
			HTTPClose(&hi);
			return DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
		}
		HTTPClose(&hi);
		if ((result = CreateServerTalk(&hi, hDev)) < 0){
			_WriteLog(LL_FATAL, "金税盘票源下载连接服务器失败, err:%d", hDev->hUSB->szLastErrorDescription);
			HTTPClose(&hi);
			return DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
		}
		_WriteLog(LL_INFO, "第一次连接失败重试成功");
	}

	result = AisinoNetInvoiceQueryReadyEasy(&hi,hDev, date_range, coil_data);
	if (result < 0)
	{
		strcpy(errinfo, hDev->hUSB->szLastErrorDescription);
		HTTPClose(&hi);
		return DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
	}
	cJSON *root = cJSON_CreateObject();
	cJSON *data_array;
	cJSON_AddItemToObject(root, "inv_coils", data_array = cJSON_CreateArray());
	for (;;)
	{
		memset(tmp_data, 0, sizeof(tmp_data));
		p = strstr(coil_data + index, "\r\n");
		if (p == NULL)
		{
			break;
		}
		
		memcpy(tmp_data, coil_data + index, p - (coil_data+index));
		char dst[4][768];
		memset(dst, 0, sizeof(dst));
		int nSepCount = 0;
		if ((nSepCount = GetSplitStringSimple(tmp_data, "|", dst, 4)) != 4) {
			_WriteLog(LL_FATAL, "DownloadSpecifyInvoice, parament's count incorrect");
			HTTPClose(&hi);
			return -1;
		}
	
		uint8 fpzl;
		AisinoFPLX2CommonFPLX(atoi(dst[0]), &fpzl);

		sprintf(inv_data + strlen(inv_data), "%03d,%s,%s,%s;", fpzl, dst[1], dst[2], dst[3]);
		cJSON *data_layer;
		cJSON_AddItemToObject(data_array, "dira", data_layer = cJSON_CreateObject());

		char tmp_fpzl[20] = {0};
		sprintf(tmp_fpzl, "%03d", fpzl);
		cJSON_AddStringToObject(data_layer, "fplxdm", tmp_fpzl);
		cJSON_AddStringToObject(data_layer, "fpdm", dst[1]);
		cJSON_AddStringToObject(data_layer, "fpqshm", dst[2]);
		cJSON_AddStringToObject(data_layer, "fpzs", dst[3]);

		index = p - coil_data+2;
	}

	char *g_buf;
	g_buf = cJSON_Print(root);
	

	Base64_Encode(g_buf, strlen(g_buf), inv_data);
	free(g_buf);
	cJSON_Delete(root);
	_WriteLog(LL_FATAL, "inv_data :%s\n", inv_data);
	HTTPClose(&hi);
	return result;
}

int aisino_query_net_invoice_coil_download_unlock(HDEV hDev, char *date_range, uint8 fllxdm,char *fpdm,char *fpqshm,int fpzs, char *errinfo)
{
	char *p;
	int result;
	ClearLastError(hDev->hUSB);
	if (hDev->bTrainFPDiskType)
	{
		sprintf(errinfo, "测试盘不支持票源查询下载功能");//该错误描述不能修改,ty_usb_m中有使用
		return -1;
	}
	FillDevInfoCert(hDev, 1);
	char dowm_data[1024] = {0};
	char coil_data[1024] = { 0 };

	uint8 bAisino_FPLX;
	CommonFPLX2AisinoFPLX(fllxdm, &bAisino_FPLX);
	if (!CheckDeviceSupportInvType(hDev->abSupportInvtype, bAisino_FPLX)) {
		sprintf(errinfo, "税盘不支持此发票类型，请先完成税盘信息变更");
		return DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
	}

	struct HTTP hi;
	if ((result = CreateServerTalk(&hi, hDev)) < 0) 
	{
		if (hDev->bBreakAllIO) {
			SetLastError(hDev->hUSB, ERR_IO_BREAK, "服务器交互过程中被其他接口中断");
			hDev->bBreakAllIO = 0;  //清除缓存
			HTTPClose(&hi);
			return DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
		}
		HTTPClose(&hi);
		if ((result = CreateServerTalk(&hi, hDev)) < 0){
			_WriteLog(LL_FATAL, "金税盘票源下载连接服务器失败, err:%d", hDev->hUSB->szLastErrorDescription);
			HTTPClose(&hi);
			return DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
		}
		_WriteLog(LL_INFO, "第一次连接失败重试成功");
	}

	uint8 aisino_fplx;
	if ((fllxdm == FPLX_COMMON_ZYFP) || (fllxdm == FPLX_COMMON_PTFP))
	{
		CommonFPLX2AisinoFPLX(fllxdm, &aisino_fplx);
		sprintf(dowm_data, "%d|%s|%s|%d", aisino_fplx, fpdm, fpqshm, fpzs);

		_WriteLog(LL_FATAL, "先判断后下载 要下载的发票卷信息：%s \n", dowm_data);

		result = AisinoNetInvoiceQueryReadyEasy(&hi, hDev, date_range, coil_data);
		if (result < 0)
		{
			strcpy(errinfo, hDev->hUSB->szLastErrorDescription);

			if (strlen(errinfo) == 0)
			{
				sprintf(errinfo, "网上在线领票查询待下载信息失败");
			}
			HTTPClose(&hi);
			//sprintf(errinfo, "网上在线领票查询失败");
			return DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
		}
		ClearLastError(hDev->hUSB);
		int cmp_ok = -1;
		int index = 0;
		char tmp_data[200] = { 0 };
		for (;;)
		{
			memset(tmp_data, 0, sizeof(tmp_data));
			p = strstr(coil_data + index, "\r\n");
			if (p == NULL)
			{
				break;
			}
			memcpy(tmp_data, coil_data + index, p - (coil_data + index));
			char dst[4][768];
			memset(dst, 0, sizeof(dst));
			int nSepCount = 0;
			if ((nSepCount = GetSplitStringSimple(tmp_data, "|", dst, 4)) != 4) {
				_WriteLog(LL_FATAL, "DownloadSpecifyInvoice, parament's count incorrect");
				break;
			}
			uint8 net_fpzl = 0;
			char net_fpdm[20] = { 0 };
			char net_fphm[20] = { 0 };
			int net_fpzs = 0;
			AisinoFPLX2CommonFPLX(atoi(dst[0]), &net_fpzl);
			strcpy(net_fpdm, dst[1]);
			strcpy(net_fphm, dst[2]);
			net_fpzs = atoi(dst[3]);

			if ((net_fpzl == fllxdm) && (strcmp(net_fpdm, fpdm) == 0) && (strcmp(net_fphm, fpqshm) == 0) && (net_fpzs == fpzs))
			{
				cmp_ok = 0;
				break;
			}


			index = p - coil_data + 2;
		}

		if (cmp_ok != 0)
		{
			_WriteLog(LL_INFO, "coil_data = %s ,dowm_data = %s", coil_data,dowm_data);
			sprintf(errinfo, "要下载的发票与查询到的信息不一致");
			HTTPClose(&hi);
			return DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
		}
	}
	else if (fllxdm == FPLX_COMMON_DZFP)
	{
		if (fpzs <= 0)
		{
			sprintf(errinfo, "输入的发票张数错误");
			HTTPClose(&hi);
			return DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
		}
		sprintf(dowm_data, "%d|0|0|%d", FPLX_AISINO_DZFP, fpzs);
	}
	result = AisinoNetInvoiceDownloadUnlockEasy(&hi, hDev, dowm_data);
	if (result < 0)
	{
		strcpy(errinfo, hDev->hUSB->szLastErrorDescription);
		_WriteLog(LL_INFO, "下载失败,原因 %s", errinfo);
		if (strlen(errinfo) == 0)
		{
			sprintf(errinfo, "网上在线领票下载安装发票失败");
		}
		HTTPClose(&hi);
		return DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
	}
	HTTPClose(&hi);
	return result;
}

int aisino_check_server_address(HDEV hDev,char *errinfo)
{
	struct HTTP hi;	
	int result=0;
	ClearLastError(hDev->hUSB);
	memset(&hi, 0, sizeof(struct HTTP));
	if (hDev->bTrainFPDiskType)
	{
		sprintf(errinfo, "测试盘不支持测试服务器连接功能");
		return -1;
	}
	FillDevInfoCert(hDev, 1);
	if ((result = CreateServerTalk(&hi, hDev)) < 0) 
	{
		HTTPClose(&hi);
		_WriteLog(LL_DEBUG, "局端连接测试, 第一次不成功,重试第二次");
		if ((result = CreateServerTalk(&hi, hDev)) < 0)
		{
			sprintf(errinfo, "%s", hDev->hUSB->szLastErrorDescription);
			_WriteLog(LL_DEBUG, "局端连接测试, %s", errinfo);
			HTTPClose(&hi);
			return result;
		}
	}
	result = SyncCompanyInfo(&hi, true);
	if (result == 0)
	{
		_WriteLog(LL_DEBUG, "同步企业监控信息完成, %s", hDev->szCompanyName);
	}
	HTTPClose(&hi);
	return result;
}

int aisino_connect_pubservice(HDEV hDev, uint8 inv_type, char *errinfo)
{
	ClearLastError(hDev->hUSB);
	if (hDev->bTrainFPDiskType)
	{
		sprintf(errinfo, "测试盘不支持接入撤销公共服务平台");
		return -1;
	}
	FillDevInfoCert(hDev, 1);
	if (inv_type != FPLX_COMMON_DZFP)
	{
		sprintf(errinfo, "暂仅支持增值税电子普通发票");
		return -2;
	}
	//税盘时间20200101之前不支持
	char szSQXX[512] = "";
	if (!CheckDeviceSupportInvType(hDev->abSupportInvtype, FPLX_AISINO_DZFP)) {
		_WriteLog(LL_FATAL, "Pubservice switch need dzfp support");
		sprintf(errinfo, "此税盘不支持增值税电子普通发票");
		return -1;
	}
	int nRet = SwitchPubservice(hDev, true, szSQXX);
	if (nRet < 0) {
		strcpy(errinfo, hDev->hUSB->errinfo);//网络报错存储在errinfo
		_WriteLog(LL_FATAL, "AisinoSwitchPubservice failed");
		logout(INFO, "TAXLIB", "接入公共服务平台", "SwitchPubservice nRet = %d\r\n", nRet);
		return -2;
	}
	else if (!nRet) {
		sprintf(errinfo, "此税盘已接入公共服务平台,无需重复操作");
		_WriteLog(LL_WARN, "Pubservice has already enabled/disable.Nothing to do...");
		return 0;
	}
	// need to do
	if (UpdateCompanyInfo(hDev->hUSB, szSQXX) < 0) {
		sprintf(errinfo, "更新税盘状态信息失败,请稍后重试");
		_WriteLog(LL_FATAL, "Change pubservice status failed");
		return -1;
	}
	_WriteLog(LL_INFO, "AisinoSwitchPubservice change status to :%s",
		true ? "enable" : "disable");


	return 0;
}
int aisino_disconnect_pubservice(HDEV hDev, uint8 inv_type, char *errinfo)
{
	ClearLastError(hDev->hUSB);
	if (hDev->bTrainFPDiskType)
	{
		sprintf(errinfo, "测试盘不支持接入撤销公共服务平台");
		return -1;
	}
	FillDevInfoCert(hDev, 1);
	if (inv_type != FPLX_COMMON_DZFP)
	{
		sprintf(errinfo, "暂仅支持增值税电子普通发票");
		return -2;
	}
	//税盘时间20200101之前不支持
	char szSQXX[512] = "";
	if (!CheckDeviceSupportInvType(hDev->abSupportInvtype, FPLX_AISINO_DZFP)) {
		sprintf(errinfo, "此税盘不支持增值税电子普通发票");
		_WriteLog(LL_FATAL, "Pubservice switch need dzfp support");
		return -1;
	}
	int nRet = SwitchPubservice(hDev, false, szSQXX);
	if (nRet < 0) {
		strcpy(errinfo, hDev->hUSB->errinfo);//网络报错存储在errinfo
		_WriteLog(LL_FATAL, "AisinoSwitchPubservice failed");
		return -2;
	}
	else if (!nRet) {
		sprintf(errinfo, "此税盘已撤销接入公共服务平台,无需重复操作");
		_WriteLog(LL_WARN, "Pubservice has already enabled/disable.Nothing to do...");
		return 0;
	}
	// need to do
	if (UpdateCompanyInfo(hDev->hUSB, szSQXX) < 0) {
		sprintf(errinfo, "更新税盘状态信息失败,请稍后重试");
		_WriteLog(LL_FATAL, "Change pubservice status failed");
		return -1;
	}
	_WriteLog(LL_INFO, "AisinoSwitchPubservice change status to :%s",
		false ? "enable" : "disable");


	return 0;
}

int aisino_query_invoice_month_all_data(HDEV hDev, char *month, char **data_json, char *errinfo)
{
	int result;

	char tmp_s[50] = { 0 };
	char start_date[20] = { 0 };
	char end_date[20] = { 0 };
	//如果是当前月则到今天截止
	result = LoadAisinoDevInfo(hDev, hDev->hUSB, false);
	if (result < 0)
	{
		return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
	}
	turn_month_to_date_range(month, hDev->szDeviceTime, start_date, end_date);
	//turn_month_to_date_range(month, hDev->szDeviceTime, start_date, end_date);
	//_WriteLog(LL_INFO, "要查询的日期范围为：%s %s,当前时间为%s", start_date, end_date, hDev->szDeviceTime);




	struct StMonthStatistics stStatisticsRet;
	memset(&stStatisticsRet, 0, sizeof(struct StMonthStatistics));
	if (AisinoGetMonthStatistic(hDev->hUSB, month, &stStatisticsRet) < 0)
		return -1;

	cJSON *dir2, *dir3;		//组包使用
	cJSON *json = cJSON_CreateObject();
	cJSON_AddItemToObject(json, "statistics", dir2 = cJSON_CreateArray());

	if (1)
	{
		uint8 fplxdm;
		AisinoFPLX2CommonFPLX(stStatisticsRet.fplx, &fplxdm);
		memset(tmp_s, 0, sizeof(tmp_s));
		sprintf(tmp_s, "%03d", fplxdm);
		cJSON_AddItemToObject(dir2, "dira", dir3 = cJSON_CreateObject());
		cJSON_AddStringToObject(dir3, "fplxdm", tmp_s);
		cJSON *root = cJSON_CreateObject();
		cJSON_AddStringToObject(root, "qssj", start_date);//起始时间 yyyymmdd
		cJSON_AddStringToObject(root, "jzsj", end_date);//截止时间 yyyymmdd
		cJSON_AddStringToObject(root, "qckcfs", stStatisticsRet.qckcfs);//期初库存
		cJSON_AddStringToObject(root, "lgfpfs", stStatisticsRet.lgfpfs);//领购发票份数 
		cJSON_AddStringToObject(root, "thfpfs", stStatisticsRet.thfpfs);//退回发票份数 

		int zsfpfs = atoi(stStatisticsRet.zsfpfs) + atoi(stStatisticsRet.zffpfs);
		memset(tmp_s, 0, sizeof(tmp_s));
		sprintf(tmp_s, "%d", zsfpfs);
		cJSON_AddStringToObject(root, "zsfpfs", tmp_s);//正数发票份数 
		cJSON_AddStringToObject(root, "zffpfs", stStatisticsRet.zffpfs);//正废发票份数 

		int fsfpfs = atoi(stStatisticsRet.fsfpfs) + atoi(stStatisticsRet.fffpfs);
		memset(tmp_s, 0, sizeof(tmp_s));
		sprintf(tmp_s, "%d", fsfpfs);
		cJSON_AddStringToObject(root, "fsfpfs", tmp_s);//负数发票份数 

		cJSON_AddStringToObject(root, "fffpfs", stStatisticsRet.fffpfs);//负废发票份数 
		cJSON_AddStringToObject(root, "kffpfs", stStatisticsRet.kffpfs);//空废发票份数 
		cJSON_AddStringToObject(root, "qmkcfs", stStatisticsRet.qmkcfs);//期末库存份数 
		cJSON_AddStringToObject(root, "zsfpljje", stStatisticsRet.zsfpljje);//正数发票累计金额 
		cJSON_AddStringToObject(root, "zsfpljse", stStatisticsRet.zsfpljse);//正数发票累计税额 
		cJSON_AddStringToObject(root, "zffpljje", stStatisticsRet.zffpljje);//正废发票累计金额 
		cJSON_AddStringToObject(root, "zffpljse", stStatisticsRet.zffpljse);//正废发票累计税额 
		cJSON_AddStringToObject(root, "fsfpljje", stStatisticsRet.fsfpljje);//负数发票累计金额 
		cJSON_AddStringToObject(root, "fsfpljse", stStatisticsRet.fsfpljse);//负数发票累计税额 
		cJSON_AddStringToObject(root, "fffpljje", stStatisticsRet.fffpljje);//负废发票累计金额 
		cJSON_AddStringToObject(root, "fffpljse", stStatisticsRet.fffpljse);//负废发票累计税额 
		long double sjxsje = strtold(stStatisticsRet.zsfpljje, NULL) - strtold(stStatisticsRet.fsfpljje, NULL);
		//_WriteLog(LL_INFO, "发票类型：%03d,实际销售金额（浮点型）：%f", fplxdm,sjxsje);
		memset(tmp_s, 0, sizeof(tmp_s));
		sprintf(tmp_s, "%4.2Lf", sjxsje);
		//_WriteLog(LL_INFO, "发票类型：%03d,实际销售金额（字符型）：%s", fplxdm, tmp_s);
		//PriceRound(tmp_s, 2, tmp_s);
		cJSON_AddStringToObject(root, "sjxsje", tmp_s);//实际销售金额 
		long double sjxsse = strtold(stStatisticsRet.zsfpljse, NULL) - strtold(stStatisticsRet.fsfpljse, NULL);
		//_WriteLog(LL_INFO, "发票类型：%03d,实际销售税额（浮点型）：%f", fplxdm, sjxsse);
		memset(tmp_s, 0, sizeof(tmp_s));
		sprintf(tmp_s, "%4.2Lf", sjxsse);
		//_WriteLog(LL_INFO, "发票类型：%03d,实际销售税额（字符型）：%s", fplxdm, tmp_s);
		//PriceRound(tmp_s, 2, tmp_s);
		cJSON_AddStringToObject(root, "sjxsse", tmp_s);//实际销售税额 


		char *json_buf;
		char *base_buf;
		json_buf = cJSON_Print(root);
		//_WriteLog(LL_INFO, "发票类型%03d", fplxdm);
		//_WriteLog(LL_INFO, "%s", json_buf);
		base_buf = malloc(strlen(json_buf) * 2);
		Base64_Encode(json_buf, strlen(json_buf), base_buf);
		free(json_buf);
		cJSON_Delete(root);
		cJSON_AddStringToObject(dir3, "data", base_buf);
		free(base_buf);
	}



	struct StMonthStatistics *nextNode = stStatisticsRet.next;
	while (nextNode)
	{

		uint8 fplxdm;
		AisinoFPLX2CommonFPLX(nextNode->fplx, &fplxdm);
		memset(tmp_s, 0, sizeof(tmp_s));
		sprintf(tmp_s, "%03d", fplxdm);
		cJSON_AddItemToObject(dir2, "dira", dir3 = cJSON_CreateObject());
		cJSON_AddStringToObject(dir3, "fplxdm", tmp_s);
		cJSON *root = cJSON_CreateObject();
		cJSON_AddStringToObject(root, "qssj", start_date);//起始时间 yyyymmdd
		cJSON_AddStringToObject(root, "jzsj", end_date);//截止时间 yyyymmdd
		cJSON_AddStringToObject(root, "qckcfs", nextNode->qckcfs);//期初库存
		cJSON_AddStringToObject(root, "lgfpfs", nextNode->lgfpfs);//领购发票份数 
		cJSON_AddStringToObject(root, "thfpfs", nextNode->thfpfs);//退回发票份数 

		int zsfpfs = atoi(nextNode->zsfpfs) + atoi(nextNode->zffpfs);
		memset(tmp_s, 0, sizeof(tmp_s));
		sprintf(tmp_s, "%d", zsfpfs);
		cJSON_AddStringToObject(root, "zsfpfs", tmp_s);//正数发票份数 
		cJSON_AddStringToObject(root, "zffpfs", nextNode->zffpfs);//正废发票份数 

		int fsfpfs = atoi(nextNode->fsfpfs) + atoi(nextNode->fffpfs);
		memset(tmp_s, 0, sizeof(tmp_s));
		sprintf(tmp_s, "%d", fsfpfs);
		cJSON_AddStringToObject(root, "fsfpfs", tmp_s);//负数发票份数 

		cJSON_AddStringToObject(root, "fffpfs", nextNode->fffpfs);//负废发票份数 
		cJSON_AddStringToObject(root, "kffpfs", nextNode->kffpfs);//空废发票份数 
		cJSON_AddStringToObject(root, "qmkcfs", nextNode->qmkcfs);//期末库存份数 
		cJSON_AddStringToObject(root, "zsfpljje", nextNode->zsfpljje);//正数发票累计金额 
		cJSON_AddStringToObject(root, "zsfpljse", nextNode->zsfpljse);//正数发票累计税额 
		cJSON_AddStringToObject(root, "zffpljje", nextNode->zffpljje);//正废发票累计金额 
		cJSON_AddStringToObject(root, "zffpljse", nextNode->zffpljse);//正废发票累计税额 
		cJSON_AddStringToObject(root, "fsfpljje", nextNode->fsfpljje);//负数发票累计金额 
		cJSON_AddStringToObject(root, "fsfpljse", nextNode->fsfpljse);//负数发票累计税额 
		cJSON_AddStringToObject(root, "fffpljje", nextNode->fffpljje);//负废发票累计金额 
		cJSON_AddStringToObject(root, "fffpljse", nextNode->fffpljse);//负废发票累计税额 
		long double sjxsje = strtold(nextNode->zsfpljje, NULL) - strtold(nextNode->fsfpljje, NULL);
		//_WriteLog(LL_INFO, "发票类型：%03d,实际销售金额（浮点型）：%f", fplxdm, sjxsje);
		memset(tmp_s, 0, sizeof(tmp_s));
		sprintf(tmp_s, "%4.2Lf", sjxsje);
		//_WriteLog(LL_INFO, "发票类型：%03d,实际销售金额（字符型）：%s", fplxdm, tmp_s);
		//PriceRound(tmp_s, 2, tmp_s);
		cJSON_AddStringToObject(root, "sjxsje", tmp_s);//实际销售金额 
		long double sjxsse = strtold(nextNode->zsfpljse, NULL) - strtold(nextNode->fsfpljse, NULL);
		//_WriteLog(LL_INFO, "发票类型：%03d,实际销售税额（浮点型）：%f", fplxdm, sjxsse);
		memset(tmp_s, 0, sizeof(tmp_s));
		sprintf(tmp_s, "%4.2Lf", sjxsse);
		//_WriteLog(LL_INFO, "发票类型：%03d,实际销售税额（字符型）：%s", fplxdm, tmp_s);
		//PriceRound(tmp_s, 2, tmp_s);
		cJSON_AddStringToObject(root, "sjxsse", tmp_s);//实际销售税额 

		char *json_buf;
		char *base_buf;
		json_buf = cJSON_Print(root);
		//_WriteLog(LL_INFO, "发票类型%03d", fplxdm);
		//_WriteLog(LL_INFO, "%s", json_buf);
		base_buf = malloc(strlen(json_buf) * 2);
		Base64_Encode(json_buf, strlen(json_buf), base_buf);
		free(json_buf);
		cJSON_Delete(root);
		cJSON_AddStringToObject(dir3, "data", base_buf);
		free(base_buf);
		struct StMonthStatistics *tmp = nextNode->next;
		free(nextNode);
		nextNode = tmp;
	}
	char *g_buf;
	g_buf = cJSON_Print(json);
	//_WriteLog(LL_INFO, "%s", g_buf);
	char *base_buf;
	base_buf = malloc(strlen(g_buf) * 2);
	Base64_Encode(g_buf, strlen(g_buf), base_buf);
	free(g_buf);
	cJSON_Delete(json);
	*data_json = base_buf;
	return 0;

}

int aisino_client_hello(HDEV hDev, char *client_hello, char *errinfo)
{
	ClearLastError(hDev->hUSB);
	if (hDev->bTrainFPDiskType)
	{
		sprintf(errinfo, "测试盘不支持服务器握手Hello");
		return -1;
	}
	FillDevInfoCert(hDev, 1);
	//printf("	___________hDev->postMessage_flag = %d\r\n",hDev->postMessage_flag);
	if(hDev->postMessage_flag == 1)//电子税务局登录
	{
		if (GetTaClientHello(1,hDev, client_hello) < 0) {
			return ERR_TA_CLIENTHELLO;
		}
	}
	else
	{
		if (GetTaClientHello(0,hDev, client_hello) < 0) {
			return ERR_TA_CLIENTHELLO;
		}
	}
	//printf("aisino_client_hello:%s\n",client_hello);
	return 0;
}

int aisino_client_auth(HDEV hDev, char *server_hello, char *client_auth, char *errinfo)
{
	int auth_len = 5120;
	ClearLastError(hDev->hUSB);
	if (hDev->bTrainFPDiskType)
	{
		sprintf(errinfo, "测试盘不支持服务器认证Auth");
		return -1;
	}
	FillDevInfoCert(hDev, 1);
	if (BuildClientAuthCode((uint8 *)server_hello, strlen(server_hello), (uint8 *)client_auth, &auth_len, hDev) < 0) {
		return ERR_TA_AUTHCODE;
	}
	return 0;
}

int aisino_get_cert(HDEV hDev, char *cert_data, char *errinfo)
{
	X509 *x = NULL;
	int nIndex = 2;
	int i;
	void *pCerts = calloc(1, 20480);
	if (AisinoEnumCertsMem(hDev->hUSB, pCerts, hDev->bCryptBigIO) < 2)
	{
		free(pCerts);
		return -1;
	}
	uint16 *pNum = (uint16 *)pCerts;
	if (0 == *pNum) {
		free(pCerts);
		return -2;
	}
	for (i = 0; i < *pNum; i++) {
		uint16 *pCertLen = (uint16 *)(pCerts + nIndex);
		uint8 *pCert = (uint8 *)(pCerts + nIndex + 2);

		char cert_tmp[4096] = {0};
		Byte2Str(cert_tmp, pCert, *pCertLen);
		_WriteLog(LL_INFO, "i = %d,cert_data = %s", i, cert_tmp);


		if (!d2i_X509(&x, (const uint8 **)&pCert, *pCertLen))
		{
			_WriteLog(LL_INFO, "!d2i_X509");
			continue;
		}
		int crit = 0;
		BASIC_CONSTRAINTS *skid = NULL;
		skid = (BASIC_CONSTRAINTS *)X509_get_ext_d2i(x, NID_basic_constraints, &crit, NULL);
		if (!skid || skid->ca != 0)
		{
			nIndex += 2 + *pCertLen;
			X509_free(x);
			x = NULL;
			continue;
		}
		BASIC_CONSTRAINTS_free(skid);
		ASN1_BIT_STRING *lASN1UsageStr = NULL;
		lASN1UsageStr = (ASN1_BIT_STRING *)X509_get_ext_d2i(x, NID_key_usage, NULL, NULL);
		if (!lASN1UsageStr)
		{
			nIndex += 2 + *pCertLen;
			X509_free(x);
			x = NULL;
			continue;
		}
		uint16 usage = lASN1UsageStr->data[0];
		if (lASN1UsageStr->length > 1)
			usage |= lASN1UsageStr->data[1] << 8;

		ASN1_BIT_STRING_free(lASN1UsageStr);
		X509_free(x);
		x = NULL;
		if (usage & KU_KEY_ENCIPHERMENT)
		{
			_WriteLog(LL_INFO,"找到加密用证书");
			strcpy(cert_data,cert_tmp);
			_WriteLog(LL_INFO, "i = %d,cert_data = %s", i, cert_data);
			break;
		}		
		nIndex += 2 + *pCertLen;
	}
	free(pCerts);
	return 0;
}

int aisino_zhfwpt_server_address(HDEV hDev, char *errinfo)
{
	struct HTTP hi;
	int result = 0;
	ClearLastError(hDev->hUSB);
	memset(&hi, 0, sizeof(struct HTTP));
	if (hDev->bTrainFPDiskType)
	{
		sprintf(errinfo, "测试盘不支持测试服务器连接功能");
		return -1;
	}
	FillDevInfoCert(hDev, 1);
	if ((result = CreateConfirmTalk(&hi, hDev)) < 0)
	{
		HTTPClose(&hi);
		_WriteLog(LL_DEBUG, "局端连接测试, 第一次不成功,重试第二次");
		if ((result = CreateConfirmTalk(&hi, hDev)) < 0)
		{
			sprintf(errinfo, "%s", hDev->hUSB->szLastErrorDescription);
			_WriteLog(LL_DEBUG, "局端连接测试, %s", errinfo);
		}
	}
	HTTPClose(&hi);
	return result;
}

int aisino_update_summary_data(HDEV hDev, struct _plate_infos *plate_infos)
{
	int result;
	char errinfo[1024] = { 0 };
	char month[20] = {0};
	char *statistics = NULL;
	struct _inv_sum_data now_month_sum;
	memcpy(month, hDev->szDeviceTime,6);
	result = aisino_query_invoice_month_all_data(hDev, (char *)month, &statistics, errinfo);
	if (result < 0)
	{
		_WriteLog(LL_INFO, "读取发票汇总数据过程中出现异常\n");
		return -1;
	}

	deal_plate_statistics_to_now_month_sum(statistics, &now_month_sum);

	if (statistics != NULL)
	{
		free(statistics);
	}

	int sum_cout;
	uint8 mem_i;
	for (sum_cout = 0; sum_cout < sizeof(now_month_sum.type_sum) / sizeof(now_month_sum.type_sum[0]); sum_cout++)
	{
		if (sum_cout == 0)
			mem_i = 0;
		else if (sum_cout == 1)
			mem_i = 3;
		else if (sum_cout == 2)
			mem_i = 6;
		else if (sum_cout == 3)
			mem_i = 7;
		else
			return -1;
		if (plate_infos->invoice_type_infos[mem_i].state == 1)
		{
			//out("%03d票种，实际销售金额%4.2Lf,实际销售税额%4.2Lf,期初库存份数为%d\n", plate_infos->invoice_type_infos[mem_i].fplxdm, now_month_sum.type_sum[sum_cout].sjxsje, now_month_sum.type_sum[sum_cout].sjxsse, now_month_sum.type_sum[sum_cout].qckcfs);
			memset(&plate_infos->invoice_type_infos[mem_i].monitor_info.type_sum, 0, sizeof(struct _inv_type_sum_data));
			memcpy(&plate_infos->invoice_type_infos[mem_i].monitor_info.type_sum, &now_month_sum.type_sum[sum_cout], sizeof(struct _inv_type_sum_data));
		}
	}
	return 0;
}
