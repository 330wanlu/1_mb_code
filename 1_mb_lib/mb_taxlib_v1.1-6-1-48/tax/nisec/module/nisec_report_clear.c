/*****************************************************************************
File name:   nisec_report_clear.c
Description: ���ڰ����̵ĳ���˰���忨
Author:      Zako
Version:     1.0
Date:        2020.09
History:
20200901     �������ʵ��
*****************************************************************************/
#include "nisec_report_clear.h"

//���͸���˰����-��������ҵ��-�걨ģ��
int ZZSTaxDataFromHyXML(HDEV hDev, char *szHzxxConfigLines, char *szOutXML)
{
    //<taxDataFromHy><czlx>01</czlx><informationAboutTaxpayer><nsrsbh>913201067838061494</nsrsbh><kpjh>0</kpjh><bssj>202010010000</bssj><sksbh>539904612711</sksbh><sksblx>1102180608</sksblx><sfssq>N</sfssq><sfzq>Y</sfzq><bspz>02</bspz><qssj>20200901</qssj><jzsj>20200930</jzsj></informationAboutTaxpayer>
    //<summaryStockDataForPp><qckc>0</qckc><bqxg>45</bqxg><qmkc>36</qmkc><bqth>0</bqth><fgfc>0</fgfc><fcth>0</fcth><zczsje>456617.82</zczsje><zczsse>4566.18</zczsse><zcfsje>-200</zcfsje><zcfsse>-2</zcfsse><zfje>0</zfje><zfse>0</zfse><zcfs>9</zcfs><zczsfs>8</zczsfs><zcfsfs>1</zcfsfs><zffs>0</zffs></summaryStockDataForPp><skpData><fphz>BE01...B7E6</fphz><szfphz>3141...BA1A</szfphz><bbh>1</bbh></skpData></taxDataFromHy>
    int nRet = -1, nSepCount = 0, i = 0;
    char szBuf[256] = "";
    char lineHzxx[5][768];
    memset(lineHzxx, 0, sizeof(lineHzxx));
    mxml_node_t *root = NULL, *nodeMain = NULL, *xml_Node = NULL;
    uint8 bFplxAisino = 0;
    uint8 bFplx = hDev->bOperateInvType;
    struct StMonthStatistics statistics;

    if ((root = mxmlNewXML("1.0")) == NULL)
        goto Finish;
    if ((nodeMain = mxmlNewElement(root, "taxDataFromHy")) == NULL)
        goto Finish;
    mxmlNewText(mxmlNewElement(nodeMain, "czlx"), 0, "01");  // czlx static

    if ((xml_Node = mxmlNewElement(nodeMain, "informationAboutTaxpayer")) == NULL)
        goto Finish;
    mxmlNewText(mxmlNewElement(xml_Node, "nsrsbh"), 0, hDev->szCommonTaxID);  // nsrsbh
    sprintf(szBuf, "%d", hDev->uICCardNo);
    mxmlNewText(mxmlNewElement(xml_Node, "kpjh"), 0, szBuf);  // kpjh
    memset(szBuf, 0, sizeof(szBuf));
    strncpy(szBuf, hDev->szDeviceTime, 6);
    strcat(szBuf, "010000");
    mxmlNewText(mxmlNewElement(xml_Node, "bssj"), 0, szBuf);                    // ��˰ʱ��
    mxmlNewText(mxmlNewElement(xml_Node, "sksbh"), 0, hDev->szDeviceID);        // ˰���豸��
    mxmlNewText(mxmlNewElement(xml_Node, "sksblx"), 0, hDev->szDriverVersion);  // ˰���豸����
    mxmlNewText(mxmlNewElement(xml_Node, "sfssq"), 0, "N");  //�Ƿ�������?��1-30��
    mxmlNewText(mxmlNewElement(xml_Node, "sfzq"), 0, "Y");   //�Ƿ�����?���¸���1-15��
    CommonFPLX2AisinoFPLX(bFplx, &bFplxAisino);
    sprintf(szBuf, "%02d", bFplxAisino);
    mxmlNewText(mxmlNewElement(xml_Node, "bspz"), 0, szBuf);  //��˰Ʊ�� PTFP==2
    if ((nSepCount = GetSplitStringSimple(szHzxxConfigLines, "\r\n", lineHzxx, 5)) != 5)
        goto Finish;
    mxmlNewText(mxmlNewElement(xml_Node, "qssj"), 0, lineHzxx[0]);  // ˰����ʼʱ��
    mxmlNewText(mxmlNewElement(xml_Node, "jzsj"), 0, lineHzxx[1]);  // ˰�ڽ�ֹʱ��

    if (FPLX_COMMON_PTFP == bFplx) {
        if ((xml_Node = mxmlNewElement(nodeMain, "summaryStockDataForPp")) == NULL)
            goto Finish;
    } else {  // FPLX_COMMON_ZYFP
        if ((xml_Node = mxmlNewElement(nodeMain, "summaryStockDataForZp")) == NULL)
            goto Finish;
    }
    if (GetMonthStatistics(hDev->hUSB, lineHzxx[0], lineHzxx[1], &statistics) < 0)
        goto Finish;
    mxmlNewText(mxmlNewElement(xml_Node, "qckc"), 0, statistics.qckcfs);      //�ڳ����
    mxmlNewText(mxmlNewElement(xml_Node, "bqxg"), 0, statistics.lgfpfs);      //��Ʊ�칺��
    mxmlNewText(mxmlNewElement(xml_Node, "qmkc"), 0, statistics.qmkcfs);      //��ĩ���
    mxmlNewText(mxmlNewElement(xml_Node, "bqth"), 0, statistics.thfpfs);      //��Ʊ�˻أ�
    mxmlNewText(mxmlNewElement(xml_Node, "fgfc"), 0, "0");                    // static
    mxmlNewText(mxmlNewElement(xml_Node, "fcth"), 0, "0");                    // static
    mxmlNewText(mxmlNewElement(xml_Node, "zczsje"), 0, statistics.zsfpljje);  //�����������
    mxmlNewText(mxmlNewElement(xml_Node, "zczsse"), 0, statistics.zsfpljse);  //��������˰��
    sprintf(szBuf, "-%s", statistics.fsfpljje);
    mxmlNewText(mxmlNewElement(xml_Node, "zcfsje"), 0, szBuf);  //�����������
    sprintf(szBuf, "-%s", statistics.fsfpljse);
    mxmlNewText(mxmlNewElement(xml_Node, "zcfsse"), 0, szBuf);              //��������˰��
    mxmlNewText(mxmlNewElement(xml_Node, "zfje"), 0, statistics.zffpljje);  //���Ͻ��
    mxmlNewText(mxmlNewElement(xml_Node, "zfse"), 0, statistics.zffpljse);  //����˰��
    i = atoi(statistics.zsfpfs) + atoi(statistics.fsfpfs);
    sprintf(szBuf, "%d", i);
    mxmlNewText(mxmlNewElement(xml_Node, "zcfs"), 0, szBuf);  //��������=δ���Ϸ���=����+����
    mxmlNewText(mxmlNewElement(xml_Node, "zczsfs"), 0, statistics.zsfpfs);  //������������
    mxmlNewText(mxmlNewElement(xml_Node, "zcfsfs"), 0, statistics.fsfpfs);  //������������
    i = atoi(statistics.zffpfs) + atoi(statistics.fffpfs) + atoi(statistics.kffpfs);
    sprintf(szBuf, "%d", i);
    mxmlNewText(mxmlNewElement(xml_Node, "zffs"), 0, szBuf);  //���Ϸ���

    if ((xml_Node = mxmlNewElement(nodeMain, "skpData")) == NULL)
        goto Finish;
    mxmlNewText(mxmlNewElement(xml_Node, "fphz"), 0, lineHzxx[3]);    //��Ʊ����
    mxmlNewText(mxmlNewElement(xml_Node, "szfphz"), 0, lineHzxx[4]);  //����ʱ�ӷ�Ʊ����
    mxmlNewText(mxmlNewElement(xml_Node, "bbh"), 0, lineHzxx[2]);     //�汾��

    char *ptr = mxmlSaveAllocString(root, MXML_NO_CALLBACK);
    strcpy(szOutXML, ptr);
    free(ptr);
    nRet = 0;
Finish:
    mxmlDelete(root);
    return nRet;
}

//ע�⣬������ͨ�÷�Ʊ���ͣ�������aisino��FPLX
// szHzxxConfigLines == NULL ʱΪ���췴дxml��������Զ�̳���xml
int Nisec_DZXML_ReportClear(HDEV hDev, char *szHzxxConfigLines, char *szOut)
{
    //����
    //<?xml version="1.0" encoding="gbk"?><business id="KPXXSC" comment="��Ʊ��Ϣ�ϴ�"><body
    // count="1" skph="539912237528" nsrsbh="91320118MA1X7GFL8Q" bbh="0"><group xh="1"><data
    // name="fplx_dm" value="026"/><data name="sq" value="2020090120200930"/><data name="fpmx"
    // value=""/><data name="fpdxx" value="032...05"/><data name="fphz" value="4F...03"/><data
    // name="szfphz" value="C6...8E"/><data name="qtxx" value=""/></group></body></business>
    //��д
    //<business id="JKHCQQ" comment="��ػش�����"><body count="1" skph="539912237528"
    // nsrsbh="91320118MA1X7GFL8Q" bbh="0"><group xh="1"><data name="fplx_dm"
    // value="026"/></group></body></business>

    int nRet = -1;
    char szBuf[256] = "";
    mxml_node_t *root = NULL, *xml_Node = NULL, *xml_Node2 = NULL;
    SplitInfo sp;
    memset(&sp, 0, sizeof(SplitInfo));
    if (szHzxxConfigLines)
        GetSplitString(szHzxxConfigLines, "\r\n", &sp);

    if ((root = mxmlNewXML("1.0")) == NULL)
        return -2;
    xml_Node = mxmlNewElement(root, "business");
    if (szHzxxConfigLines) {
        mxmlElementSetAttr(xml_Node, "id", "KPXXSC");
        mxmlElementSetAttr(xml_Node, "comment", "��Ʊ��Ϣ�ϴ�");
    } else {
        mxmlElementSetAttr(xml_Node, "id", "JKHCQQ");
        mxmlElementSetAttr(xml_Node, "comment", "��ػش�����");
    }
    xml_Node = mxmlNewElement(xml_Node, "body");
    if (!xml_Node)
        goto Finish;
    mxmlElementSetAttr(xml_Node, "count", "1");
    mxmlElementSetAttr(xml_Node, "skph", hDev->szDeviceID);
    mxmlElementSetAttr(xml_Node, "nsrsbh", hDev->szCommonTaxID);
    if (szHzxxConfigLines) {  //��6���Ƿ�Ʊ��棬̫����
        SplitInfo *pIndx = GetSplitIndex(&sp, 2);
        mxmlElementSetAttr(xml_Node, "bbh", pIndx->info);
    } else {
        uint8 bVersion = GetDeviceBBH(hDev->szDriverVersion);
        sprintf(szBuf, "%d", bVersion);
        mxmlElementSetAttr(xml_Node, "bbh", szBuf);
    }
    xml_Node = mxmlNewElement(xml_Node, "group");
    if (!xml_Node)
        goto Finish;
    mxmlElementSetAttr(xml_Node, "xh", "1");
    xml_Node2 = mxmlNewElement(xml_Node, "data");
    mxmlElementSetAttr(xml_Node2, "name", "fplx_dm");
    sprintf(szBuf, "%03d", hDev->bOperateInvType);
    mxmlElementSetAttr(xml_Node2, "value", szBuf);
    if (szHzxxConfigLines) {
        SplitInfo *pIndx0 = GetSplitIndex(&sp, 0);
        SplitInfo *pIndx1 = GetSplitIndex(&sp, 1);
        // sq
        xml_Node2 = mxmlNewElement(xml_Node, "data");
        mxmlElementSetAttr(xml_Node2, "name", "sq");
        sprintf(szBuf, "%s%s", pIndx0->info, pIndx1->info);
        mxmlElementSetAttr(xml_Node2, "value", szBuf);
        // fpmx
        xml_Node2 = mxmlNewElement(xml_Node, "data");
        mxmlElementSetAttr(xml_Node2, "name", "fpmx");
        //���¿���Ʊʱ���Ƿ������ݴ���
        mxmlElementSetAttr(xml_Node2, "value", "");
        // fpdxx
        xml_Node2 = mxmlNewElement(xml_Node, "data");
        mxmlElementSetAttr(xml_Node2, "name", "fpdxx");
        pIndx0 = GetSplitIndex(&sp, 5);
        if (strlen(pIndx0->info) < 5)  // not N/A
            strcpy(pIndx0->info, "");
        mxmlElementSetAttr(xml_Node2, "value", pIndx0->info);
        // fphz
        xml_Node2 = mxmlNewElement(xml_Node, "data");
        mxmlElementSetAttr(xml_Node2, "name", "fphz");
        pIndx0 = GetSplitIndex(&sp, 3);
        mxmlElementSetAttr(xml_Node2, "value", pIndx0->info);
        // szfphz
        xml_Node2 = mxmlNewElement(xml_Node, "data");
        mxmlElementSetAttr(xml_Node2, "name", "szfphz");
        pIndx0 = GetSplitIndex(&sp, 4);
        mxmlElementSetAttr(xml_Node2, "value", pIndx0->info);
        // qtxx
        xml_Node2 = mxmlNewElement(xml_Node, "data");
        mxmlElementSetAttr(xml_Node2, "name", "qtxx");
        mxmlElementSetAttr(xml_Node2, "value", "");
    }
    TAXML_Formater(root, szOut);
    nRet = 0;
    // _WriteHexToDebugFile("kpxxsc.xml", (uint8 *)szOut, strlen(szOut));

Finish:
    SplitFree(&sp);
    mxmlDelete(root);
    return nRet;
}

//��ȡ������Ϣ/��Ʊ���ܱ�
int GetReportSignAndRTCData(HDEV hDev, char *szHzxxConfigLines)
{
    HUSB hUSB = hDev->hUSB;
    unsigned char abGetIB[64] = {0x80, 0x47, 0x45, 0x54, 0x49, 0x42, 0xff, 0xff, 0x00, 0x00, 0x00,
                                 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff};
    uint8 cb[1024] = {0}, abBuf[256] = {0};
    int nRet = -2;
    int nChildRet = -1;
    uint8 bVersion = -1;
    while (1) {
        // 1.��ȡ˰���̶��걨����ǩ������ֵ�̶� TC_GetSignature
        memset(cb, 0, 16);
        memcpy(cb, "\xfe\x0f", 2);
        //��ѯ��Χ��ʹ��szHzxxConfigLinesǰ���У������м�س�����
        bVersion = GetDeviceBBH(hDev->szDriverVersion);
        if (0 == bVersion) {
            Str2Byte(cb + 2, szHzxxConfigLines, 8);
            Str2Byte(cb + 6, szHzxxConfigLines + 10, 8);
            strcat(szHzxxConfigLines, "0");
        } else
            strcat(szHzxxConfigLines, "1");
        strcat(szHzxxConfigLines, "\r\n");
        if ((nChildRet = NisecLogicIO(hUSB, cb, 10, sizeof cb, NULL, 0)) < 64) {
            if (strstr(hUSB->szLastErrorDescription, "09d11b"))
                _WriteLog(
                    LL_WARN,
                    "Read sign of hzxx failed, maybe there is no hzxx in the device");  //�޻�������
            else
                _WriteLog(LL_FATAL, "Read sign of hzxx failed");
            break;
        }
        if (0 == bVersion)
            Byte2Str(szHzxxConfigLines + strlen(szHzxxConfigLines), cb, nChildRet);
        else
            Byte2Str(szHzxxConfigLines + strlen(szHzxxConfigLines), cb,
                     nChildRet - 4);  //�ų����4���ֽ�
        strcat(szHzxxConfigLines, "\r\n");
        // 2.˰���̶�ʱ�ӻ������ݣ���ֵ����ʱ�䶯̬�仯 TC_ReadRTCInvoice
        abGetIB[6] = 0x1;  // yylxdm Ӧ�����ʹ��� static
        abGetIB[7] = (uint8)hDev->bOperateInvType;
        sub_365670((long)abGetIB, 0x3e, (uint32 *)(abGetIB + 0x3e));
        memcpy(cb, "\xfe\x3b\x40", 3);  //ָ��
        if ((nChildRet = NisecLogicIO(hUSB, cb, 3, sizeof cb, abGetIB, sizeof(abGetIB))) < 0) {
            _WriteLog(LL_FATAL, "Read RTC failed 1");
            break;
        }
        memcpy(cb, "\xfe\x08", 2);  //��ȡ
        if ((nChildRet = NisecLogicIO(hUSB, cb, 2, sizeof cb, NULL, 0)) < 64) {
            _WriteLog(LL_FATAL, "Read RTC failed 2");
            break;
        }
        if (0x80 != cb[0])
            break;
        memset(abBuf, 0, sizeof(abBuf));
        Byte2Str((char *)abBuf, (uint8 *)cb + 1, 1);
        int nLen = atoi((char *)abBuf);
        if (!nLen)
            break;
        Byte2Str(szHzxxConfigLines + strlen(szHzxxConfigLines), cb + 3, nLen - 3);
        strcat(szHzxxConfigLines, "\r\n");
        nRet = 0;
        break;
    }
    return nRet;
}

int ZZSClearcardXML(HDEV hDev, char *szOutput)
{
    //<FPXT><INPUT><NSRSBH>91320105MA1MDPWR2N</NSRSBH><KPJH>0</KPJH><SBBH>499905080012</SBBH><HASH>91320105MA1MDPWR2N</HASH><CSSJ>202010010000</CSSJ><BSPZ>02</BSPZ></INPUT></FPXT>
    char szCSSJ[24] = "", szBSPZ[16] = "", szBuf[48] = "";
    struct StMonitorInfo stMonitorInfo;
    HUSB hUSB = hDev->hUSB;
    uint8 bFplxAisino = 0;
    if (GetMonitorInfo(hUSB, &stMonitorInfo) < 0)
        return -1;

    sprintf(szBuf, "%s000000", stMonitorInfo.kpjzsj);
    GetBJSJ(szBuf, szCSSJ, 2);
    CommonFPLX2AisinoFPLX(hDev->bOperateInvType, &bFplxAisino);
    sprintf(szBSPZ, "%02d", bFplxAisino);

    sprintf(szOutput,
            "<?xml version=\"1.0\" "
            "encoding=\"gbk\"?><FPXT><INPUT><NSRSBH>%s</NSRSBH><KPJH>%d</KPJH><SBBH>%s</"
            "SBBH><HASH>%s</HASH><CSSJ>%s</CSSJ><BSPZ>%s</BSPZ></INPUT></FPXT>",
            hDev->szCommonTaxID, hDev->uICCardNo, hDev->szDeviceID, hDev->szCommonTaxID, szCSSJ,
            szBSPZ);
    return 0;
}

int NisecJudgeIsTimeForReport(HDEV hDev, char *szHzxxConfigLines)
{
    //���ﱾ��Ӧ�û�Ҫ����Ƿ�˰�ڵģ��ſ��������������ϲ����ȥ����
    int nChildRet = 0, nIntervalDay = 0;
    uint8 cb[256] = "";
    char szTimeNow[24] = {0}, szBuf[64];
    struct StMonitorInfo stMonitorInfo;
    HUSB hUSB = hDev->hUSB;

    memcpy(cb, "\xfe\x04", 2);
    nChildRet = NisecLogicIO(hUSB, cb, 2, sizeof cb, NULL, 0);
    if (nChildRet != 7)
        return -1;
    sprintf((char *)szTimeNow, "%02x%02x%02x%02x%02x%02x%02x", cb[0], cb[1], cb[2], cb[3], cb[4],
            cb[5], cb[6]);
    if (GetMonitorInfo(hUSB, &stMonitorInfo) < 0)
        return -2;
    //��ʬ��check
    strcpy(szBuf, stMonitorInfo.zxbsrq);
    strcat(szBuf, "000000");
    nIntervalDay = MinusTime(szTimeNow, szBuf) / 3600 / 24;
    if (nIntervalDay > 90) {
        _WriteLog(LL_FATAL, "Out of date, zombie device has been locked");
        SetLastError(hDev->hUSB, ERR_LOGIC_CHECK_REPORT_TIME,
                     "�ж��Ƿ񵽳�������ʱ��ʧ��,���ܴ��ڶ���δ�����������");
        return -3;
    }
    //��������
    strcpy(szBuf, stMonitorInfo.kpjzsj);
    strcat(szBuf, "000000");
    nIntervalDay = MinusTime(szTimeNow, szBuf) / 3600 / 24;
    if (nIntervalDay > 0) {
        _WriteLog(LL_FATAL, "Out of date, device has been locked this month");
        SetLastError(hDev->hUSB, ERR_LOGIC_CHECK_REPORT_TIME,
                     "�ж��Ƿ񵽳�������ʱ��ʧ��,���ܴ��ڶ���δ�����������");
        return -4;
    }
    //��Ʊ��ֹʱ�䲻ͬ�£�˵�����忨
    strcpy(szBuf, stMonitorInfo.kpjzsj);
    strcat(szBuf, "000000");
    if (!IsTheSameMonth(szTimeNow, szBuf)) {
        SetLastError(hDev->hUSB, ERR_LOGIC_CHECK_REPORT_TIME,
                     "�ж��Ƿ񵽳�������ʱ��ʧ��,���ܴ��ڶ���δ�����������");
        _WriteLog(LL_FATAL, "Timenow and kpjzsj in difference month, maybe has clearcarded");
        return -6;
    }
    //��ȡ��ǰ˰�ڷ�Χ
    sprintf(szHzxxConfigLines, "%s\r\n%s\r\n", stMonitorInfo.bsqsrq, stMonitorInfo.bszzrq);
    _WriteLog(LL_INFO, "Report tax period: %s-%s", stMonitorInfo.bsqsrq, stMonitorInfo.bszzrq);
    return 0;
}

//С��ģ��˰�˿��Զ���忨
int NisecClearCardIO(HDEV hDev, char *szFpjkmw)
{
    unsigned char abSetIB[64] = {0x80, 0x53, 0x45, 0x54, 0x49, 0x42, 0x00, 0x00, 0x00, 0x00, 0x00,
                                 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1b, 0x2b};
    int nChildRet = 0, nFpjkmwLen = 0, nSzjkmwLen = 0;
    uint8 cb[512] = "";
    uint8 abFpjkmw[512] = {0};
    uint8 abSzjkmw[512] = {0};
    nSzjkmwLen = sizeof(abSetIB);
    if (FPLX_COMMON_ZYFP == hDev->bOperateInvType || FPLX_COMMON_PTFP == hDev->bOperateInvType) {
        if (strlen(szFpjkmw) != 224 * 2)
            return -1;
        //�ַ���96-112����仯
        nChildRet = Str2Byte(abFpjkmw, szFpjkmw, strlen(szFpjkmw));
        //��ʽΪ0-0xa8(168�ֽ�)Ϊ�忨���ģ�0xa9-0xe0(0x38,56�ֽ�)Ϊʱ������
        if (nChildRet != 224)
            return -2;
        memcpy(abSzjkmw, abSetIB, sizeof(abSetIB));
        //��У׼ʱ������,������ǰ6�����2���ֽ�
        memcpy(abSzjkmw + 6, abFpjkmw + 0xa8, nSzjkmwLen - 8);
        //��0����
        memset(abFpjkmw + 168, 0, 56);
        nFpjkmwLen = 168;
    } else {  // DZFP
        char dst[2][768];
        memset(dst, 0, sizeof(dst));
        int nSepCount = 0;
        if ((nSepCount = GetSplitStringSimple(szFpjkmw, "|||", dst, 2)) != 2)
            return -4;
        char *pSzjkmw = dst[1];
        if (strlen(pSzjkmw) != 112)
            return -5;
        _WriteLog(LL_DEBUG, "DZFP FPJKMW:%s SZJKMW:%s", dst[0], dst[1]);
        nFpjkmwLen = Str2Byte(abFpjkmw, dst[0], strlen(dst[0]));

        memcpy(abSzjkmw, abSetIB, sizeof(abSetIB));
        Str2Byte(abSzjkmw + 6, dst[1], strlen(dst[1]));
    }
    //������ʱ��
    memcpy(cb, "\xfe\x3b\x02", 3);
    if (NisecLogicIO(hDev->hUSB, cb, 3, sizeof cb, abSzjkmw, nSzjkmwLen) < 0)
        return -6;
    //���忨
    memcpy(cb, "\xfe\x10", 2);
    if (NisecLogicIO(hDev->hUSB, cb, 2, sizeof cb, abFpjkmw, nFpjkmwLen) < 0) {
        if (strstr(hDev->hUSB->szLastErrorDescription, "09d118")) {
            _WriteLog(LL_WARN, "Has already clearcard, repeat operate");
            return 0;
        }
        return -8;
    }
    return 0;
}

//Զ�̳���=�ϱ����
//�˽ӿڰ����������ϱ����ܣ�ִ�е���һ���ֽ����򳭱����
// bOnlyPeriodTaxReport=true������
int NisecReporSummarytMain(HDEV hDev, uint8 bFplx_Common)
{
    // szHzxxConfigLines������Ϣ����6�Σ��걨˰��begin+�걨˰��end+�汾��+�걨ǩ��+ʵʱʱ��+��Ʊ����Ϣ����Ʊ����ר�ã�
    // 20200901
    // 20200930
    // 0/1
    // BE0180705...2FB54B7E6
    // 96A53949770...AD88B1E
    // 03200170031...100050005
    HUSB hUSB = hDev->hUSB;
    if (NisecEntry(hUSB, NOP_OPEN_FULL) < 0)
        return -1;
    int nRet = -1, nOpType = 0, nBufLen = 65000;
    char *szHzxxConfigLines = calloc(1, nBufLen);
    char *szHZxxXMLBuf = calloc(1, nBufLen);
    char *szFPXTBuf = calloc(1, nBufLen);
    while (1) {
        if (!szHzxxConfigLines || !szHZxxXMLBuf || !szFPXTBuf)
            break;
        if (NisecSelectInvoiceType(hDev, bFplx_Common, SELECT_OPTION_TEMPLATE)) {
            _WriteLog(LL_FATAL, "NisecSelectInvoiceType, fplx:%d", bFplx_Common);
            break;
        }
        if (NisecJudgeIsTimeForReport(hDev, szHzxxConfigLines)) {
            _WriteLog(LL_FATAL, "NisecJudgeIsTimeForReport, failed");
            break;
        }
        if (GetReportSignAndRTCData(hDev, szHzxxConfigLines)) {
            _WriteLog(LL_FATAL, "GetReportSignAndRTCData, failed");
            break;
        }
        //������ɳ�˰����������ִ���ϱ�����
        if (FPLX_COMMON_PTFP == bFplx_Common || FPLX_COMMON_ZYFP == bFplx_Common) {
            mystrupr(szHzxxConfigLines);
            // _WriteLog(LL_DEBUG, "Get string of hzxx:%s", szHzxxConfigLines);
            if (ZZSTaxDataFromHyXML(hDev, szHzxxConfigLines, szHZxxXMLBuf)) {
                _WriteLog(LL_FATAL, "ZZSTaxDataFromHyXML, failed");
                break;
            }
            //_WriteLog(LL_DEBUG, "ZZSTaxDataFromHyXML:%s", szHZxxXMLBuf);
            Base64_Encode(szHZxxXMLBuf, strlen(szHZxxXMLBuf), szFPXTBuf);
            //����У�������³�����Ҫ��
            if (ZZSXML_ReportClear(hDev, true, false, true, 0, szFPXTBuf) < 0) {
                _WriteLog(LL_FATAL, "ZZSXML_ReportClear, failed");
                break;
            }
            nOpType = TACMD_NISEC_REPORT_ZZS;
        } else {  // DZFP
            //���ڲ����豸��Ʊ�֣����Ǵ���δ���Ʊ���������˸����ݿ���Ϊ�գ��������ò��������ôֱ�����5�ÿ�
            if (QueryInvoSection(hDev, szHzxxConfigLines + strlen(szHzxxConfigLines)) < 0)
                strcat(szHzxxConfigLines, "N/A");
            strcat(szHzxxConfigLines, "\r\n");
            mystrupr(szHzxxConfigLines);
            if (Nisec_DZXML_ReportClear(hDev, szHzxxConfigLines, szHZxxXMLBuf)) {
                _WriteLog(LL_FATAL, "Nisec_DZXML_ReportClear, failed,nRet = %d");
                break;
            }
            strcpy(szFPXTBuf, szHZxxXMLBuf);
            nOpType = TACMD_NISEC_REPORT_DZ;
        }
        if (TaNetIoOnce(hDev, nOpType, szFPXTBuf, nBufLen) < 0)
            break;
        // ZZS:202010010000 DZ:00
        if (!strstr(szFPXTBuf, "0000") && strcmp(szFPXTBuf, "00")) {
            _WriteLog(LL_FATAL, "TaNetIoOnce, failed2");
            break;
        }
        nRet = 0;
        break;
    }
    if (nRet < 0)
        _WriteLog(LL_FATAL, "Nisec ReportSummary failed, fplx:%d", bFplx_Common);
    else {
        _WriteLog(LL_INFO, "Nisec ReportSummary finish, result:successful, fplx:%d", bFplx_Common);
        _WriteLog(LL_INFO, "���ܳɹ� fplx:%d\n\n\n", bFplx_Common);
    }

    NisecEntry(hUSB, NOP_CLOSE);
    if (szHzxxConfigLines)
        free(szHzxxConfigLines);
    if (szHZxxXMLBuf)
        free(szHZxxXMLBuf);
    if (szFPXTBuf)
        free(szFPXTBuf);
    return nRet;
}

int NisecClearCardMain(HDEV hDev, uint8 bFplx_Common)
{
    char szFPXTBuf[1280] = {0}, szTimeNow[64] = "";
    uint8 cb[256];
    HUSB hUSB = hDev->hUSB;
    int nRet = -1, nOpType = 0, nChildRet = -1;
    struct StMonitorInfo stMonitorInfo;

    if (NisecEntry(hUSB, NOP_OPEN_FULL) < 0)
        return -1;
    while (1) {
        if (NisecSelectInvoiceType(hDev, bFplx_Common, SELECT_OPTION_TEMPLATE))
            break;
        //�Ƿ����忨
        memcpy(cb, "\xfe\x04", 2);
        nChildRet = NisecLogicIO(hUSB, cb, 2, sizeof cb, NULL, 0);
        if (nChildRet != 7)
            break;
        sprintf((char *)szTimeNow, "%02x%02x%02x%02x%02x%02x%02x", cb[0], cb[1], cb[2], cb[3],
                cb[4], cb[5], cb[6]);
        if (GetMonitorInfo(hUSB, &stMonitorInfo) < 0)
            break;
        strcpy(szFPXTBuf, stMonitorInfo.kpjzsj);
        strcat(szFPXTBuf, "000000");
        if (!IsTheSameMonth(szTimeNow, szFPXTBuf)) {
            _WriteLog(LL_FATAL, "Timenow and kpjzsj in difference month, maybe has clearcarded");
            break;
        }
        //��ʽ�忨
        if (FPLX_COMMON_PTFP == bFplx_Common || FPLX_COMMON_ZYFP == bFplx_Common) {
            if (ZZSClearcardXML(hDev, szFPXTBuf))
                break;
            nOpType = TACMD_NISEC_CLEAR_ZZS;
        } else {  // DZFP
            if (Nisec_DZXML_ReportClear(hDev, NULL, szFPXTBuf))
                break;
            nOpType = TACMD_NISEC_CLEAR_DZ;
        }
        if (TaNetIoOnce(hDev, nOpType, szFPXTBuf, sizeof(szFPXTBuf)) < 0)
            break;
        if (strlen(szFPXTBuf) < 256)
            break;
        if ((nChildRet = NisecClearCardIO(hDev, szFPXTBuf)) < 0) {
            _WriteLog(LL_FATAL, "Nisec clearcardio failed, ret:%d", nChildRet);
            break;
        }
        nRet = 0;
        break;
    }
    if (nRet < 0) {
        _WriteLog(LL_FATAL, "NisecClearCard failed, fplx:%d,errinfo %s", bFplx_Common,
                  hDev->hUSB->errinfo);
        return -2;
    } else
        _WriteLog(LL_INFO, "NisecClearCard finish, result:successful, fplx:%d", bFplx_Common);
    NisecEntry(hUSB, NOP_CLOSE);
    return nRet;
}