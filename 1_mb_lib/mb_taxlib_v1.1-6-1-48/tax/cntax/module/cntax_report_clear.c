/*****************************************************************************
File name:   cntax_report_clear.c
Description: cntax�豸�����忨����ʵ��
Author:      Zako
Version:     1.0
Date:        2021.10.14
History:
20211014     �������ʵ��
20221201     cntax�����������⣬�޸����г����忨ģʽ���¾�2��ģʽ����
*****************************************************************************/
#include "cntax_report_clear.h"

int CntaxJudgeIsTimeForReport(HDEV hDev, char *szZxbsrq)
{
    //���ﱾ��Ӧ�û�Ҫ����Ƿ�˰�ڵģ��ſ��������������ϲ����ȥ����
    int nIntervalDay = 0;
    char szBuf[64];
    struct StMonitorInfo stMonitorInfo;
    HUSB hUSB = hDev->hUSB;

    if (CntaxGetMonitorInfo(hUSB, &stMonitorInfo) < 0)
        return -2;
    if (szZxbsrq)
        strcpy(szZxbsrq, stMonitorInfo.zxbsrq);
    //��ʬ��check
    strcpy(szBuf, stMonitorInfo.zxbsrq);
    strcat(szBuf, "000000");
    nIntervalDay = MinusTime(hDev->szDeviceTime, szBuf) / 3600 / 24;
    if (nIntervalDay > 90) {
        _WriteLog(LL_FATAL, "Out of date, zombie device has been locked");
        SetLastError(hDev->hUSB, ERR_LOGIC_CHECK_REPORT_TIME,
                     "�ж��Ƿ񵽳�������ʱ��ʧ��,��ʬ�豸,�ѳ�������ʱ��");
        return -3;
    }
    //��������
    strcpy(szBuf, stMonitorInfo.kpjzsj);
    strcat(szBuf, "000000");
    nIntervalDay = MinusTime(hDev->szDeviceTime, szBuf) / 3600 / 24;
    if (nIntervalDay > 0) {
        _WriteLog(LL_FATAL, "Out of date, device has been locked this month");
        SetLastError(hDev->hUSB, ERR_LOGIC_CHECK_REPORT_TIME,
                     "�ж��Ƿ񵽳�������ʱ��ʧ��,����������");
        return -4;
    }
    //��Ʊ��ֹʱ�䲻ͬ�£�˵�����忨
    strcpy(szBuf, stMonitorInfo.kpjzsj);
    strcat(szBuf, "000000");
    if (!IsTheSameMonth(hDev->szDeviceTime, szBuf)) {
        _WriteLog(LL_FATAL, "Timenow and kpjzsj in difference month, maybe has clearcarded");
        SetLastError(hDev->hUSB, ERR_LOGIC_CHECK_REPORT_TIME,
                     "�ж��Ƿ񵽳�������ʱ��ʧ��,�������뿨");
        return -6;
    }
    //��ȡ��ǰ˰�ڷ�Χ
    _WriteLog(LL_INFO, "Report tax period: %s-%s", stMonitorInfo.bsqsrq, stMonitorInfo.bszzrq);
    return 0;
}

//��ȡ������Ϣ/��Ʊ���ܱ�
#if CntaxReport
int BuildReportFphzXML(HDEV hDev, char *szHZxxXMLBuf, int bufSize)
{
    //<?xml version="1.0" encoding="gbk"?>
    // <business comment="��Ʊ������Ϣ�ϴ��ӿ�" id="GGFW_FPHZXXSC" jkbbh="1.0">
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
    //<?xml version="1.0" encoding="gbk"?><business comment="�����Ϣ�ش��ӿ�" id="GGFW_JKXXHC"
    // jkbbh="1.0"><body><tyxx><nsrsbh>91320118MA20LU6N3P</nsrsbh><sbbh>537100494968</sbbh><sblx>20</sblx><rjlx>01</rjlx><rjxh>100000000000</rjxh></tyxx><ywxxs><ywxx><fplxdm>007</fplxdm></ywxx></ywxxs></body></business>
    sprintf(szOutput, "<ywxxs><ywxx><fplxdm>%03d</fplxdm></ywxx></ywxxs>", hDev->bOperateInvType);
    return RET_SUCCESS;
}

//wang 
//С��ģ��˰�˿��Զ���忨
//int CntaxCardIO(HUSB hUSB, char *szFpjkmw)
int CntaxCardIO(HDEV hDev, char *szFpjkmw)
{
    int nChildRet = 0, nRet = ERR_GENERIC;
    int8 abFpjkmw[512] = {0};
    //�ٷ���bulkout����0xfe1a��device��bulkout�ظ��հף�����bulkout����160�ֽڣ��豸bulkout�ظ��հ�
    //���ų��ַ���:
    // 1)�ٷ��豸��bulkin�ظ���һ��USB Command passed
    // 2)�ҷ���bulkin����256�ֽ�req���豸��bulkin�ظ���һ��USB Command passed
    //�м����һ��bulkin usbreq�������libusb���Ͷ�ȡusb������������ʱ�޷���Ԥ��Ӧ��Ϊ��׼����

    // 20230209 !!!ע�⣬�˴��Ĵ�Сֵ����ֻ����64��̫���˻ᵼ�²����豸��237
    // 917��ͷ�豸�忨���⣿�Ȱ��ճ������⣬��512��Ϊ256->64���ԣ������������ٲ���������������
    uint8 cb[64] = "";  // libusb��buf��С����64�ֽ�ȡ������С64�ֽ�
    if (CntaxEntry(hDev->hUSB, NOP_OPEN_FULL) < 0)
        return ERR_DEVICE_OPEN;
    //�忨ʱ�����ַ�Ʊ���ͣ��˴�����Ҫѡ��Ʊ����CntaxSelectInvoiceType������ж����Ʊ��Ҫ�忨����ô���ں���һ���忨������
    do {
        nChildRet = Str2Byte((uint8 *)abFpjkmw, szFpjkmw, strlen(szFpjkmw));
        memcpy(cb, "\xfe\x1a", 2);
        if (CntaxLogicIO(hDev->hUSB, cb, 2, sizeof cb, (uint8 *)abFpjkmw, nChildRet) < 0) {
            nRet = ERR_LOGIC_REPORT_ERROR;
            break;
        }
        //��Ϊ������Ʊ�֣������޷��񳭱�һ������kpjzsj���ж��Ƿ���ɲ������������Ժ��ִ˴�����ȴ�
        nRet = RET_SUCCESS;
    } while (false);
    CntaxEntry(hDev->hUSB, NOP_CLOSE);
    return nRet;
}



//Զ�̳���=�ϱ����
//�˽ӿڰ����������ϱ����ܣ�ִ�е���һ���ֽ����򳭱����
// bOnlyPeriodTaxReport=true������
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
        //�Ƿ����忨
        if (CntaxGetMonitorInfo(hUSB, &stMonitorInfo) < 0)
            break;
        strcpy(szFPXTBuf, stMonitorInfo.kpjzsj);
        strcat(szFPXTBuf, "000000");
        if (!IsTheSameMonth(hDev->szDeviceTime, szFPXTBuf)) {
            _WriteLog(LL_FATAL, "Timenow and kpjzsj in difference month, maybe has clearcarded");
            break;
        }
        //��ʽ�忨
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
//////////////////////////////�µĹٷ������忨���̣���ʹ���豸��Ϊ237��ͷ���豸���ԣ�////////////////////////////////////////////
////////////////////////////////[�����ȶ��󣬽�ʹ���µĳ����忨�����滻�ϵ�����]////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//���ȴ�4s
bool WaitDeviceReportOperateFinish(HUSB hUSB, char *szOldZxbsrq,bool judge_flag)
{
    uint8 cb[256] = {0};
    char szZxbsrq[16] = "";
    int nChildRet = 0, i = 0, nRetry = 2, nInterval = 2;
    bool bOK = false;
    // ���zxbsrq
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

//wang 03 03 ���
//��ȡ������Ϣ/��Ʊ���ܱ�
int BuildReportFphzXML(HDEV hDev, uint8 bCommon_FPLX, char *szHZxxXMLBuf, int bufSize,bool judge_flag)
{
    HUSB hUSB = hDev->hUSB;
    //�˴�cb�ٷ�ֵ��СΪ256��ʵ�ʷ���200�ֽ�����,
    // 20221205 !!!ע�⣬�˴��Ĵ�Сֵ�������Ժ�ֻ����256��̫���˻ᵼ�²����豸��12��ͷ�豸��������
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
        //�������β�����һ�ο�������ɲ���Ӱ�죬���¿���
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
        buf[nChildRet - 8] = '\0';  //ȥ��У��ֵ?
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

//��ȡ��Ʊ�ֺ�һ�� ������Ϣ/��Ʊ���ܱ�
bool BuildReportFphzXMLAllInvType(HDEV hDev, char *szHZxxXMLBuf, int bufSize,bool judge_flag)
{
    //<?xml version="1.0" encoding="gbk"?>
    // <business comment="��Ʊ������Ϣ�ϴ��ӿ�" id="GGFW_FPHZXXSC" jkbbh="1.0">
    // <body><tyxx><nsrsbh>91320118MA20LU6N3P</nsrsbh><sbbh>537100494968</sbbh><sblx>20</sblx><rjlx>01</rjlx><rjxh>100000000000</rjxh></tyxx>
    // <ywxxs><ywxx><fplxdm>004</fplxdm><fphz>1B8FCC6990E0...........42A349F400E</fphz><qtxx></qtxx></ywxx>
    //<ywxx><fplxdm>007</fplxdm><fphz>C3A273DCE37FD...B0D</fphz><qtxx></qtxx></ywxx></ywxxs>
    // </body>
    // </business>
    char buf[1024] = {0};
    //������Ϊ���м���Ʊ������OK���ͳ�������Ʊ�֣�
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
    //���ȫ0��������ٳ���һ��Ʊ��
    return nReportDataOK > 0;
}

//Զ�̳���=�ϱ����
//�˽ӿڰ����������ϱ����ܣ�ִ�е���һ���ֽ����򳭱����
// bOnlyPeriodTaxReport=true������
int CntaxReportSummarytMain(HDEV hDev,bool judge_flag)
{
    //�����ֽ�300�ֽ����ң�һ��Ʊ��300�ֽڣ����8��Ʊ�֣��ϼ�3kbuf����16k�㹻
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
        //�Ƿ����忨
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
    //<?xml version="1.0" encoding="gbk"?><business comment="�����Ϣ�ش��ӿ�" id="GGFW_JKXXHC"
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

//С��ģ��˰�˿��Զ���忨
int CntaxCardIO_237(HUSB hUSB, char *szFpjkmw)
{
    int nChildRet = 0, nRet = ERR_GENERIC;
    int8 abFpjkmw[512] = {0};
    uint8 cb[512] = "";
    if (CntaxEntry(hUSB, NOP_OPEN_FULL) < 0)
        return ERR_DEVICE_OPEN;
    //ע�⣡�����忨ʱ�����ַ�Ʊ���ͣ��˴�����Ҫѡ��Ʊ����CntaxSelectInvoiceType
    //����ж����Ʊ��Ҫ�忨����ô���ں���һ���忨������
    do {
        nChildRet = Str2Byte((uint8 *)abFpjkmw, szFpjkmw, strlen(szFpjkmw));
        memcpy(cb, "\xfe\x1a", 2);
        if (CntaxLogicIO(hUSB, cb, 2, sizeof cb, (uint8 *)abFpjkmw, nChildRet) < 0) {
            nRet = ERR_LOGIC_REPORT_ERROR;
            break;
        }
        //��Ϊ������Ʊ�֣������޷��񳭱�һ������kpjzsj���ж��Ƿ���ɲ������������Ժ��ִ˴�����ȴ�
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
        //��ʽ�忨
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