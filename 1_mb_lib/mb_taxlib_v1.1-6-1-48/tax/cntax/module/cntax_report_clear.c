/*****************************************************************************
File name:   cntax_report_clear.c
Description: cntax设备抄报清卡代码实现
Author:      Zako
Version:     1.0
Date:        2021.10.14
History:
20211014     最初代码实现
20221201     cntax抄报错误问题，修改现有抄报清卡模式，新旧2个模式并行
*****************************************************************************/
#include "cntax_report_clear.h"

int CntaxJudgeIsTimeForReport(HDEV hDev, char *szZxbsrq)
{
    //这里本来应该还要检测是否报税期的，放宽检测条件，交由上层调用去处理
    int nIntervalDay = 0;
    char szBuf[64];
    struct StMonitorInfo stMonitorInfo;
    HUSB hUSB = hDev->hUSB;

    if (CntaxGetMonitorInfo(hUSB, &stMonitorInfo) < 0)
        return -2;
    if (szZxbsrq)
        strcpy(szZxbsrq, stMonitorInfo.zxbsrq);
    //僵尸盘check
    strcpy(szBuf, stMonitorInfo.zxbsrq);
    strcat(szBuf, "000000");
    nIntervalDay = MinusTime(hDev->szDeviceTime, szBuf) / 3600 / 24;
    if (nIntervalDay > 90) {
        _WriteLog(LL_FATAL, "Out of date, zombie device has been locked");
        SetLastError(hDev->hUSB, ERR_LOGIC_CHECK_REPORT_TIME,
                     "判断是否到抄报汇总时间失败,僵尸设备,已超过抄报时间");
        return -3;
    }
    //本月锁死
    strcpy(szBuf, stMonitorInfo.kpjzsj);
    strcat(szBuf, "000000");
    nIntervalDay = MinusTime(hDev->szDeviceTime, szBuf) / 3600 / 24;
    if (nIntervalDay > 0) {
        _WriteLog(LL_FATAL, "Out of date, device has been locked this month");
        SetLastError(hDev->hUSB, ERR_LOGIC_CHECK_REPORT_TIME,
                     "判断是否到抄报汇总时间失败,本月已锁死");
        return -4;
    }
    //开票截止时间不同月，说明已清卡
    strcpy(szBuf, stMonitorInfo.kpjzsj);
    strcat(szBuf, "000000");
    if (!IsTheSameMonth(hDev->szDeviceTime, szBuf)) {
        _WriteLog(LL_FATAL, "Timenow and kpjzsj in difference month, maybe has clearcarded");
        SetLastError(hDev->hUSB, ERR_LOGIC_CHECK_REPORT_TIME,
                     "判断是否到抄报汇总时间失败,可能已请卡");
        return -6;
    }
    //获取当前税期范围
    _WriteLog(LL_INFO, "Report tax period: %s-%s", stMonitorInfo.bsqsrq, stMonitorInfo.bszzrq);
    return 0;
}

//获取汇总信息/发票汇总表
#if CntaxReport
int BuildReportFphzXML(HDEV hDev, char *szHZxxXMLBuf, int bufSize)
{
    //<?xml version="1.0" encoding="gbk"?>
    // <business comment="发票汇总信息上传接口" id="GGFW_FPHZXXSC" jkbbh="1.0">
    // <body><tyxx><nsrsbh>91320118MA20LU6N3P</nsrsbh><sbbh>537100494968</sbbh><sblx>20</sblx><rjlx>01</rjlx><rjxh>100000000000</rjxh></tyxx>
    // <ywxxs><ywxx><fplxdm>007</fplxdm><fphz>C3A273DCE37FD...B0D</fphz><qtxx></qtxx></ywxx></ywxxs>
    // </body>
    // </business>
    HUSB hUSB = hDev->hUSB;
    uint8 cb[1024] = {0}, buf[1024] = {0};
    int nChildRet = 0, nRet = ERR_GENERIC;
    do {
        memcpy(cb, "\xfe\x19", 2);
        if ((nChildRet = CntaxLogicIO(hUSB, cb, 2, sizeof cb, NULL, 0)) < 0) {
            _WriteLog(LL_FATAL, "Read report summary failed");
            break;
        }
        if ((nChildRet = Byte2Str((char *)buf, cb, nChildRet)) < 64) {
            break;
        }
        buf[nChildRet - 8] = '\0';
        mystrupr((char *)buf);
        snprintf(szHZxxXMLBuf, bufSize,
                 "<ywxxs><ywxx><fplxdm>%03d</fplxdm><fphz>%s</fphz><qtxx></qtxx></ywxx></ywxxs>",
                 hDev->bOperateInvType, buf);
        nRet = RET_SUCCESS;
    } while (false);
    return nRet;
}
#endif

int BuildClearCardXML(HDEV hDev, char *szOutput)
{
    //<?xml version="1.0" encoding="gbk"?><business comment="监控信息回传接口" id="GGFW_JKXXHC"
    // jkbbh="1.0"><body><tyxx><nsrsbh>91320118MA20LU6N3P</nsrsbh><sbbh>537100494968</sbbh><sblx>20</sblx><rjlx>01</rjlx><rjxh>100000000000</rjxh></tyxx><ywxxs><ywxx><fplxdm>007</fplxdm></ywxx></ywxxs></body></business>
    sprintf(szOutput, "<ywxxs><ywxx><fplxdm>%03d</fplxdm></ywxx></ywxxs>", hDev->bOperateInvType);
    return RET_SUCCESS;
}

//wang 
//小规模纳税人可以多次清卡
//int CntaxCardIO(HUSB hUSB, char *szFpjkmw)
int CntaxCardIO(HDEV hDev, char *szFpjkmw)
{
    int nChildRet = 0, nRet = ERR_GENERIC;
    int8 abFpjkmw[512] = {0};
    //官方在bulkout发送0xfe1a，device在bulkout回复空白，主机bulkout发送160字节，设备bulkout回复空白
    //接着出现分歧:
    // 1)官方设备在bulkin回复了一个USB Command passed
    // 2)我方在bulkin发送256字节req，设备在bulkin回复了一个USB Command passed
    //中间多了一个bulkin usbreq，这个是libusb发送读取usb命令附带的命令，暂时无法干预，应该为标准操作

    // 20230209 !!!注意，此处的大小值疑似只能是64，太大了会导致部分设备号237
    // 917开头设备清卡问题？先按照抄报问题，从512改为256->64测试，后续不不行再测试如上歧义问题
    uint8 cb[64] = "";  // libusb该buf大小向上64字节取整，最小64字节
    if (CntaxEntry(hDev->hUSB, NOP_OPEN_FULL) < 0)
        return ERR_DEVICE_OPEN;
    //清卡时不区分发票类型，此处不需要选择发票类型CntaxSelectInvoiceType；如果有多个发票需要清卡，那么都融合在一个清卡密文中
    do {
        nChildRet = Str2Byte((uint8 *)abFpjkmw, szFpjkmw, strlen(szFpjkmw));
        memcpy(cb, "\xfe\x1a", 2);
        if (CntaxLogicIO(hDev->hUSB, cb, 2, sizeof cb, (uint8 *)abFpjkmw, nChildRet) < 0) {
            nRet = ERR_LOGIC_REPORT_ERROR;
            break;
        }
        //因为不区分票种，所有无法像抄报一样根据kpjzsj来判断是否完成操作，大量测试后发现此处无需等待
        nRet = RET_SUCCESS;
    } while (false);
    CntaxEntry(hDev->hUSB, NOP_CLOSE);
    return nRet;
}



//远程抄报=上报监控
//此接口包含抄报和上报汇总，执行到第一部分结束则抄报完成
// bOnlyPeriodTaxReport=true仅抄报
#if CntaxReport
int CntaxReportSummarytMain(HDEV hDev, uint8 bFplx_Common)
{
    HUSB hUSB = hDev->hUSB;
    if (CntaxEntry(hUSB, NOP_OPEN_FULL) < 0)
        return -1;
    int nRet = -1, nOpType = 0, nBufLen = 65000;
    char *szHZxxXMLBuf = calloc(1, nBufLen);
    while (1) {
        if (!szHZxxXMLBuf)
            break;
        if (CntaxSelectInvoiceType(hDev, bFplx_Common, SELECT_OPTION_TEMPLATE | SELECT_OPTION_TIME))
            break;
        if (CntaxJudgeIsTimeForReport(hDev, NULL))
            break;
        if (BuildReportFphzXML(hDev, szHZxxXMLBuf, nBufLen))
            break;
        nOpType = TACMD_CNTAX_REPORT;
        if (TaNetIoOnce(hDev, nOpType, szHZxxXMLBuf, nBufLen) < 0)
            break;
        if (strcmp(szHZxxXMLBuf, "00"))
            break;
        nRet = 0;
        break;
    }
    if (nRet < 0)
        _WriteLog(LL_FATAL, "Cntax ReportSummary failed, fplx:%d", bFplx_Common);
    else
        _WriteLog(LL_INFO, "Cntax ReportSummary finish, result:successful, fplx:%d", bFplx_Common);

    CntaxEntry(hUSB, NOP_CLOSE);
    if (szHZxxXMLBuf)
        free(szHZxxXMLBuf);
    return nRet;
}


int CntaxClearCardMain(HDEV hDev, uint8 bFplx_Common)
{
    char szFPXTBuf[1280] = {0};
    HUSB hUSB = hDev->hUSB;
    int nRet = -1, nOpType = TACMD_CNTAX_CLEAR, nChildRet = -1;
    struct StMonitorInfo stMonitorInfo;

    if (CntaxEntry(hUSB, NOP_OPEN_FULL) < 0)
        return -1;
    while (1) {
        if (CntaxSelectInvoiceType(hDev, bFplx_Common, SELECT_OPTION_TEMPLATE | SELECT_OPTION_TIME))
            break;
        //是否已清卡
        if (CntaxGetMonitorInfo(hUSB, &stMonitorInfo) < 0)
            break;
        strcpy(szFPXTBuf, stMonitorInfo.kpjzsj);
        strcat(szFPXTBuf, "000000");
        if (!IsTheSameMonth(hDev->szDeviceTime, szFPXTBuf)) {
            _WriteLog(LL_FATAL, "Timenow and kpjzsj in difference month, maybe has clearcarded");
            break;
        }
        //正式清卡
        BuildClearCardXML(hDev, szFPXTBuf);
        if (TaNetIoOnce(hDev, nOpType, szFPXTBuf, sizeof(szFPXTBuf)) < 0)
            break;
        if (strlen(szFPXTBuf) < 64)
            break;
        if ((nChildRet = CntaxCardIO(hDev, szFPXTBuf)) < 0) {
            _WriteLog(LL_FATAL, "Cntax clearcardio failed, ret:%d", nChildRet);
            break;
        }
        nRet = 0;
        break;
    }
    if (nRet < 0) {
        _WriteLog(LL_FATAL, "CntaxClearCard failed, fplx:%d", bFplx_Common);
    } else
        _WriteLog(LL_INFO, "CntaxClearCard finish, result:successful, fplx:%d", bFplx_Common);
    CntaxEntry(hUSB, NOP_CLOSE);
    return nRet;
}
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////新的官方抄报清卡流程（先使用设备号为237开头的设备测试）////////////////////////////////////////////
////////////////////////////////[后续稳定后，将使用新的抄报清卡流程替换老的流程]////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//最大等待4s
bool WaitDeviceReportOperateFinish(HUSB hUSB, char *szOldZxbsrq,bool judge_flag)
{
    uint8 cb[256] = {0};
    char szZxbsrq[16] = "";
    int nChildRet = 0, i = 0, nRetry = 2, nInterval = 2;
    bool bOK = false;
    // 检查zxbsrq
    for (i = 0; i < nRetry; i++) {
        memcpy(cb, "\xfe\x18\x00", 3);
        if ((nChildRet = CntaxLogicIO(hUSB, cb, 3, sizeof(cb), NULL, 0)) < 4)
            break;
        Byte2Str(szZxbsrq, cb, 4);
        if (strcmp(szOldZxbsrq, szZxbsrq) || false == judge_flag) {
            bOK = true;
            break;
        }
        sleep(nInterval);
    }
    if (!bOK)
        sleep((nRetry - i) * nInterval);
    return bOK;
}

//wang 03 03 外包
//获取汇总信息/发票汇总表
int BuildReportFphzXML(HDEV hDev, uint8 bCommon_FPLX, char *szHZxxXMLBuf, int bufSize,bool judge_flag)
{
    HUSB hUSB = hDev->hUSB;
    //此处cb官方值大小为256，实际返回200字节左右,
    // 20221205 !!!注意，此处的大小值经过测试后只能是256，太大了会导致部分设备号12开头设备抄报问题
    // uint8 cb[256] = {0}, buf[512] = {0};
    uint8 cb[100] = {0}, buf[512] = {0};  //++debug
    char szOldZxbsrq[16] = "";
    int nChildRet = 0, nRet = ERR_GENERIC;
    if (CntaxEntry(hUSB, NOP_OPEN_FULL) < 0)
        return ERR_DEVICE_OPEN;
    do {
        if (CntaxSelectInvoiceType(hDev, bCommon_FPLX, SELECT_OPTION_NULL))
            break;
        if (CntaxJudgeIsTimeForReport(hDev, szOldZxbsrq))
            break;
        CntaxEntry(hUSB, NOP_CLOSE);
        //避免两次操作在一次开卡中造成不当影响，重新开卡
        if (CntaxEntry(hUSB, NOP_OPEN_FULL) < 0)
            break;
        if (CntaxSelectInvoiceType(hDev, bCommon_FPLX, SELECT_OPTION_TEMPLATE))
            break;
        memcpy(cb, "\xfe\x19", 2);
        if ((nChildRet = CntaxLogicIO(hUSB, cb, 2, sizeof cb, NULL, 0)) != sizeof(cb)) {
            _WriteLog(LL_FATAL, "Read report summary failed");
            break;
        }
        if ((nChildRet = Byte2Str((char *)buf, cb, nChildRet)) != 2 * sizeof(cb))
            break;
        buf[nChildRet - 8] = '\0';  //去掉校验值?
        mystrupr((char *)buf);
        _WriteLog(LL_DEBUG, "CntaxFphz:%s", buf);
        snprintf(szHZxxXMLBuf, bufSize,
                 "<ywxx><fplxdm>%03d</fplxdm><fphz>%s</fphz><qtxx></qtxx></ywxx>",
                 hDev->bOperateInvType, buf);
        nRet =
            WaitDeviceReportOperateFinish(hUSB, szOldZxbsrq,judge_flag) ? RET_SUCCESS : ERR_LOGIC_REPORT_ERROR;
    } while (false);
    CntaxEntry(hUSB, NOP_CLOSE);
    return nRet;
}

//获取多票种合一的 汇总信息/发票汇总表
bool BuildReportFphzXMLAllInvType(HDEV hDev, char *szHZxxXMLBuf, int bufSize,bool judge_flag)
{
    //<?xml version="1.0" encoding="gbk"?>
    // <business comment="发票汇总信息上传接口" id="GGFW_FPHZXXSC" jkbbh="1.0">
    // <body><tyxx><nsrsbh>91320118MA20LU6N3P</nsrsbh><sbbh>537100494968</sbbh><sblx>20</sblx><rjlx>01</rjlx><rjxh>100000000000</rjxh></tyxx>
    // <ywxxs><ywxx><fplxdm>004</fplxdm><fphz>1B8FCC6990E0...........42A349F400E</fphz><qtxx></qtxx></ywxx>
    //<ywxx><fplxdm>007</fplxdm><fphz>C3A273DCE37FD...B0D</fphz><qtxx></qtxx></ywxx></ywxxs>
    // </body>
    // </business>
    char buf[1024] = {0};
    //尽力而为，有几个票种数据OK，就抄报几个票种，
    strcpy(szHZxxXMLBuf, "<ywxxs>");
    int nTypeCount = hDev->abSupportInvtype[0];
    int i = 0, nReportDataOK = 0;
    for (i = 0; i < nTypeCount; i++) {
        uint8 bCommon_FPLX = hDev->abSupportInvtype[i + 1];
        if (BuildReportFphzXML(hDev, bCommon_FPLX, buf, sizeof(buf),judge_flag))
            continue;
        _WriteLog(LL_INFO, "Cntax get summary data successful, fplx:%d", bCommon_FPLX);
        strcat(szHZxxXMLBuf, buf);
        nReportDataOK++;
    }
    strcat(szHZxxXMLBuf, "</ywxxs>");
    //如果全0则错误，至少抄报一个票种
    return nReportDataOK > 0;
}

//远程抄报=上报监控
//此接口包含抄报和上报汇总，执行到第一部分结束则抄报完成
// bOnlyPeriodTaxReport=true仅抄报
int CntaxReportSummarytMain(HDEV hDev,bool judge_flag)
{
    //额外字节300字节左右，一个票种300字节，最大8个票种，合计3kbuf，给16k足够
    int nRet = -1, nOpType = 0, nBufLen = 16000;
    char *szHZxxXMLBuf = calloc(1, nBufLen);
    while (1) {
        if (!szHZxxXMLBuf)
            break;
        if (!BuildReportFphzXMLAllInvType(hDev, szHZxxXMLBuf, nBufLen,judge_flag))
            break;
        nOpType = TACMD_CNTAX_REPORT;
        if (TaNetIoOnce(hDev, nOpType, szHZxxXMLBuf, nBufLen) < 0)
            break;
        if (strcmp(szHZxxXMLBuf, "00"))
            break;
        nRet = 0;
        break;
    }
    _WriteLog(LL_INFO, "Cntax ReportSummary finish, result:%d", nRet);
    if (szHZxxXMLBuf)
        free(szHZxxXMLBuf);
    return nRet;
}

int CntaxCheckInvTypeClearCard(HDEV hDev, uint8 bFplx_Common)
{
    HUSB hUSB = hDev->hUSB;
    int nRet = ERR_GENERIC;
    struct StMonitorInfo stMonitorInfo;
    char szKpjzsj[64] = "";

    if (CntaxEntry(hUSB, NOP_OPEN_FULL) < 0)
        return -1;
    while (1) {
        if (CntaxSelectInvoiceType(hDev, bFplx_Common, SELECT_OPTION_NULL))
            break;
        //是否已清卡
        if (CntaxGetMonitorInfo(hUSB, &stMonitorInfo) < 0)
            break;
        strcpy(szKpjzsj, stMonitorInfo.kpjzsj);
        strcat(szKpjzsj, "000000");
        if (!IsTheSameMonth(hDev->szDeviceTime, szKpjzsj)) {
            _WriteLog(
                LL_WARN,
                "Fplx:%d Timenow and kpjzsj in difference month, device may have been cleared",
                bFplx_Common);
            break;
        }
        nRet = RET_SUCCESS;
        break;
    }
    CntaxEntry(hUSB, NOP_CLOSE);
    return nRet;
}

void BuildAllInvTypeClearCardXML(HDEV hDev, int nTypeCount, uint8 *abSupportInvtype, char *szOutput)
{
    //<?xml version="1.0" encoding="gbk"?><business comment="监控信息回传接口" id="GGFW_JKXXHC"
    // jkbbh="1.0"><body><tyxx><nsrsbh>91320118MA20LU6N3P</nsrsbh><sbbh>537100494968</sbbh><sblx>20</sblx><rjlx>01</rjlx><rjxh>100000000000</rjxh></tyxx><ywxxs><ywxx><fplxdm>004</fplxdm></ywxx><ywxx><fplxdm>007</fplxdm></ywxx></ywxxs></body></business>
    char szBuf[64] = "";
    strcpy(szOutput, "<ywxxs>");
    int i = 0;
    for (i = 0; i < nTypeCount; i++) {
        uint8 bCommon_FPLX = abSupportInvtype[i];
        sprintf(szBuf, "<ywxx><fplxdm>%03d</fplxdm></ywxx>", bCommon_FPLX);
        strcat(szOutput, szBuf);
    }
    strcat(szOutput, "</ywxxs>");
}

//小规模纳税人可以多次清卡
int CntaxCardIO_237(HUSB hUSB, char *szFpjkmw)
{
    int nChildRet = 0, nRet = ERR_GENERIC;
    int8 abFpjkmw[512] = {0};
    uint8 cb[512] = "";
    if (CntaxEntry(hUSB, NOP_OPEN_FULL) < 0)
        return ERR_DEVICE_OPEN;
    //注意！！！清卡时不区分发票类型，此处不需要选择发票类型CntaxSelectInvoiceType
    //如果有多个发票需要清卡，那么都融合在一个清卡密文中
    do {
        nChildRet = Str2Byte((uint8 *)abFpjkmw, szFpjkmw, strlen(szFpjkmw));
        memcpy(cb, "\xfe\x1a", 2);
        if (CntaxLogicIO(hUSB, cb, 2, sizeof cb, (uint8 *)abFpjkmw, nChildRet) < 0) {
            nRet = ERR_LOGIC_REPORT_ERROR;
            break;
        }
        //因为不区分票种，所有无法像抄报一样根据kpjzsj来判断是否完成操作，大量测试后发现此处无需等待
        nRet = RET_SUCCESS;
    } while (false);
    CntaxEntry(hUSB, NOP_CLOSE);
    return nRet;
}

int CntaxClearCardMain(HDEV hDev)
{
    char szFPXTBuf[512] = {0};
    int nRet = -1;
    uint8 abNeedClearCardInvType[16] = {0};
    uint8 bNeedClearCardInvNum = 0;

    while (1) {
        int i = 0, nTypeCount = hDev->abSupportInvtype[0];
        for (i = 0; i < nTypeCount; i++) {
            uint8 bCommon_FPLX = hDev->abSupportInvtype[i + 1];
            if (!CntaxCheckInvTypeClearCard(hDev, bCommon_FPLX)) {
                abNeedClearCardInvType[bNeedClearCardInvNum] = bCommon_FPLX;
                bNeedClearCardInvNum++;
                continue;
            }
        }
        if (!bNeedClearCardInvNum) {
            _WriteLog(LL_INFO, "No more invtype to clearcard");
            break;
        }
        //正式清卡
        BuildAllInvTypeClearCardXML(hDev, bNeedClearCardInvNum, abNeedClearCardInvType, szFPXTBuf);
        if (TaNetIoOnce(hDev, TACMD_CNTAX_CLEAR, szFPXTBuf, sizeof(szFPXTBuf)) < 0)
            break;
        if (strlen(szFPXTBuf) < 64)
            break;
        _WriteLog(LL_DEBUG, "Cntax clearcard mw:%s", szFPXTBuf);
        if (CntaxCardIO_237(hDev->hUSB, szFPXTBuf) < 0) {
            _WriteLog(LL_FATAL, "Cntax clearcardio failed");
            break;
        }
        nRet = RET_SUCCESS;
        break;
    }
    _WriteLog(LL_INFO, "CntaxClearCard finish, result:%d", nRet);
    return nRet;
}