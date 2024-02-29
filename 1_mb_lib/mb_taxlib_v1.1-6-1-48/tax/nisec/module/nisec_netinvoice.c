/*****************************************************************************
File name:   nisec_netinvoice.c
Description: �������緢Ʊ�Ļ�ȡ\ȷ��\�ַ������ӹ�������ƽ̨������������ء���Ʊ��ش���
Author:      Zako
Version:     1.0
Date:        2020.12
History:
20201218     �������ʵ��
20210618     ����������
*****************************************************************************/
#include "nisec_netinvoice.h"

//-------------------------------��������ƽ̨���� begin----------------------------------------
int NisecUpdateCompanyInfo(HUSB hUSB, char *szNewCompanyInfoBinStr)
{
    uint8 cb[512], abCompanyInfoBuf[4096];
    if ((strlen(szNewCompanyInfoBinStr) / 2 + 1) > sizeof(abCompanyInfoBuf))
        return ERR_BUF_CHECK;
    int nCIBufLength =
        Str2Byte(abCompanyInfoBuf, szNewCompanyInfoBinStr, strlen(szNewCompanyInfoBinStr));
    memcpy(cb, "\xfe\x17", 2);
    if (NisecLogicIO(hUSB, cb, 2, sizeof cb, abCompanyInfoBuf, nCIBufLength) < 0)
        return ERR_IO_FAIL;
    return RET_SUCCESS;
}
//-------------------------------��������ƽ̨���� Finish----------------------------------------
//--------------------------------���Ϲ�Ʊ���� Begin--------------------------------------------
int AppendNodeGpxx(mxml_node_t *nodeResultChildSingle, char *szOutFormatStr)
{
    //<group
    // xh="1"><fplx_dm>026</fplx_dm><fpdm>032002000311</fpdm><fphm_q>95087285</fphm_q><fphm_z>95087288</fphm_z><fs>4</fs><gpzt>0</gpzt></group>
    int count = 0;
    char szFPZL[16], szTypeCode[24], szFPHMHead[24], szFPCount[24], szGpzt[24], szLine[512];
    do {
        if (GetChildNodeValue(nodeResultChildSingle, "fplx_dm", szFPZL) < 0)
            break;
        if (GetChildNodeValue(nodeResultChildSingle, "fpdm", szTypeCode) < 0)
            break;
        if (GetChildNodeValue(nodeResultChildSingle, "fphm_q", szFPHMHead) < 0)
            break;
        if (GetChildNodeValue(nodeResultChildSingle, "fs", szFPCount) < 0)
            break;
        if (GetChildNodeValue(nodeResultChildSingle, "gpzt", szGpzt) < 0)
            break;
        if (!strcmp(szGpzt, "2"))
            continue;
        if (!strcmp(szGpzt, "1")) {
            _WriteLog(LL_WARN, "Found not unlock invoce -- FPZL:%s LBDM:%s QSHM:%s FPFS:%s", szFPZL,
                      szTypeCode, szFPHMHead, szFPCount);
        }
        //���ﻹ�и���Ʊ״̬gpzt������Ʊ�����ص��豸��������δ����������˰��ʱ����ֵΪ1������Ϊ0
        _WriteLog(LL_INFO, "NetInvoiceGet, TA invlist -- FPZL:%s LBDM:%s QSHM:%s FPFS:%s GPZT:%s",
                  szFPZL, szTypeCode, szFPHMHead, szFPCount, szGpzt);
        sprintf(szLine, "%s|%s|%s|%s|%s\r\n", szFPZL, szTypeCode, szFPHMHead, szFPCount, szGpzt);
        count++;
        strcat(szOutFormatStr, szLine);
    } while ((nodeResultChildSingle = mxmlGetNextSibling(nodeResultChildSingle)) != NULL);
    return count;
}

//�Ȳ�ѯ������д��,����֪ͨ˰�ֽ�� bOpType =0 ��ѯ��ù�Ʊ,=1
//���빺Ʊ��=2�ϴ���Ʊ���ͬʱ˰�ַ��ع�Ʊ���
int NisecQueryNetInvoiceReady(uint8 bOpType, HHTTP hi, char *szInputOrOutFormatStr)
{
    HDEV hDev = hi->hDev;
    char szXML[9999] = "", dst[4][768], szZzhm[24] = "";
    memset(dst, 0, sizeof(dst));
    int nRet = -3, nChildRet = -1;
    mxml_node_t *pXMLRoot = NULL, *node1 = NULL;
    char *pPayLoad = NULL;
    int nSepCount = 0, nFpfs = 0;
    long lQshm = 0, lZzhm = 0;
    if (bOpType == 1 || bOpType == 2) {
        if ((nSepCount = GetSplitStringSimple(szInputOrOutFormatStr, "|", dst, 4)) != 4) {
            _WriteLog(LL_FATAL, "DownloadSpecifyInvoice, parament's count incorrect");
            return -1;
        }
        sscanf(dst[2], "%010ld", &lQshm);
        sscanf(dst[3], "%d", &nFpfs);
        lZzhm = lQshm + nFpfs - 1;
        sprintf(szZzhm, "%0*ld", (int)strlen(dst[2]), lZzhm);
    }

    if (bOpType == 0)
        sprintf(szXML,
                "<?xml version=\"1.0\" encoding=\"gbk\"?><business id=\"gpxx_cx\" "
                "comment=\"��Ʊ��Ϣ��ѯ\"><body count=\"0\" skph=\"%s\" nsrsbh=\"%s\"/></business>",
                hDev->szDeviceID, hDev->szCommonTaxID);
    else if (bOpType == 1) {
        //<business id="gpxx_sq" comment="��Ʊ��Ϣ����"><body count="1" skph="539912237528"
        // nsrsbh="91320118MA1X7GFL8Q" bbh="0"><group
        // xh="1"><fplx_dm>026</fplx_dm><fpdm>032002000311</fpdm><fphm_q>95087285</fphm_q><fphm_z>95087285</fphm_z><fs>1</fs></group></body></business>
        uint8 bVersion = GetDeviceBBH(hDev->szDriverVersion);
        sprintf(szXML,
                "<?xml version=\"1.0\" encoding=\"gbk\"?><business id=\"gpxx_sq\" "
                "comment=\"��Ʊ��Ϣ����\"><body count=\"1\" skph=\"%s\" nsrsbh=\"%s\" "
                "bbh=\"%d\"><group "
                "xh=\"1\"><fplx_dm>%s</fplx_dm><fpdm>%s</fpdm><fphm_q>%s</fphm_q><fphm_z>%s</"
                "fphm_z><fs>%s</fs></group></body></business>",
                hDev->szDeviceID, hDev->szCommonTaxID, bVersion, dst[0], dst[1], dst[2], szZzhm,
                dst[3]);
        hDev->bOperateInvType = atoi(dst[0]);
    } else if (bOpType == 2)  //==2
    {
        //<business id="gpxx_cg" comment="��Ʊ��Ϣ�ɹ�"><body count="1" skph="539912237528"
        // nsrsbh="91320118MA1X7GFL8Q"><group
        // xh="1"><fplx_dm>026</fplx_dm><fpdm>032002000311</fpdm><fphm_q>95087285</fphm_q><fphm_z>95087285</fphm_z><returnCode>09d119</returnCode><returnMessage>��Ʊ��Ϣд��ʧ�ܣ���Ʊ�칺��Ϣ�Ѵ���(09d119)</returnMessage></group></body></business>
        sprintf(szXML,
                "<?xml version=\"1.0\" encoding=\"gbk\"?><business id=\"gpxx_cg\" "
                "comment=\"��Ʊ��Ϣ�ɹ�\"><body count=\"1\" skph=\"%s\" "
                "nsrsbh=\"%s\"><group "
                "xh=\"1\"><fplx_dm>%s</fplx_dm><fpdm>%s</fpdm><fphm_q>%s</"
                "fphm_q><fphm_z>%s</fphm_z><returnCode>00</"
                "returnCode><returnMessage>��Ʊ��Ϣд�̳ɹ�</"
                "returnMessage></group></body></business>",
                hDev->szDeviceID, hDev->szCommonTaxID, dst[0], dst[1], dst[2], szZzhm);
    } else {
        return -2;
    }
    if (TaNetIo(hi, TACMD_NISEC_NETINV_DO, szXML, sizeof szXML) < 0) {
        _WriteLog(LL_FATAL, "TaNetIo failed, nRet:%d", nRet);
        return -1;
    }
    if ((pXMLRoot = mxmlLoadString(NULL, szXML, MXML_OPAQUE_CALLBACK)) == NULL) {
        _WriteLog(LL_FATAL, "NetInvoiceGet_QueryReady failed, Msg:%s", szXML);
        return -3;
    }
    while (true) {
        if ((node1 = mxmlFindElement(pXMLRoot, pXMLRoot, "returnCode", NULL, NULL, MXML_DESCEND)) ==
            NULL)
            break;
        if ((pPayLoad = (char *)mxmlGetOpaque(node1)) == NULL) {
            nRet = -10;
            break;
        }
        int nRequestStatus = atoi(pPayLoad);
        if (nRequestStatus != 0) {
            //<business id="gpxx_cx" comment="��Ʊ��Ϣ��ѯ"><body count="0"
            // nsrsbh="91320118MA1X7GFL8Q"
            // skph="539912237528"><returnCode>01</returnCode><returnMessage>û�в�ѯ��������ȡ�ķ�Ʊ</returnMessage></body></business>
            char szMsg[1024] = {0};
            char szMsg_error[1024] = {0};
            GetChildNodeValue(pXMLRoot, "returnMessage", szMsg);
            U2G_UTF8Ignore(szMsg, strlen(szMsg), szMsg_error, sizeof(szMsg_error));
            SetLastError(hDev->hUSB, ERR_TA_REPONSE_CHECK, "%s", szMsg_error);
            _WriteLog(LL_WARN,
                      "Upload nisec invoice finish, but it return failed, upload "
                      "terminate,server-msg:[%s]",
                      hDev->hUSB->errinfo);

            if (nRequestStatus == 1)
                nRet = 0;  // 01ʱ ��Ʊ�����������
            break;
        }
        if (bOpType == 0) {
            //#query response
            //<business id="gpxx_cx" comment="��Ʊ��Ϣ��ѯ"><body count="1"
            // nsrsbh="91320118MA1X7GFL8Q"
            // skph="539912237528"><returnCode>00</returnCode><returnMessage>�����ɹ�</returnMessage><group
            // xh="1"><fplx_dm>026</fplx_dm><fpdm>032002000311</fpdm><fphm_q>95087285</fphm_q><fphm_z>95087288</fphm_z><fs>4</fs><gpzt>0</gpzt></group></body></business>
            //
            if ((node1 = mxmlFindElement(pXMLRoot, pXMLRoot, "group", NULL, NULL, MXML_DESCEND)) ==
                NULL)
                break;
            // szInputOrOutFormatStr�����5������һ����Ʊ״̬��0-1-2
            nChildRet = AppendNodeGpxx(node1, szInputOrOutFormatStr);
            if (nChildRet < 0) {
                break;
            }
            _WriteLog(LL_INFO, "Get available invoice count:%d", nChildRet);
        } else if (bOpType == 1) {
            //#request response
            //<business id="gpxx_sq" comment="��Ʊ��Ϣ����"><body count="1"
            // nsrsbh="91320118MA1X7GFL8Q"
            // skph="539912237528"><returnCode>00</returnCode><returnMessage>�����ɹ�</returnMessage><group
            // xh="1"><returnCode>00</returnCode><returnMessage>�����ɹ�</returnMessage><fplx_dm>026</fplx_dm><gpxxmw>2279BF8F3C741CFF93801FE2F082761F93801FE2F082761F6B40904A6EF975793351C3B1DBFDC47E</gpxxmw><show>032002000311#95087285#95087285</show></group></body></business>
            szInputOrOutFormatStr[0] = '\0';
            if (GetChildNodeValue(pXMLRoot, "gpxxmw", szInputOrOutFormatStr) < 0)
                break;
            if (strlen(szInputOrOutFormatStr) < 64)
                break;  //ͨ����80�ֽ�
        } else if (bOpType == 2) {
            //��Ʊ״̬Ϊ2ʱ����Ʊ���̽���
            //
            //<?xml version="1.0" encoding="GBk"?><business id="gpxx_cg"
            // comment="��Ʊ��Ϣ�ɹ�"><body count="1" nsrsbh="91320118MA1X7GFL8Q"
            // skph="539912237528"><returnCode>00</returnCode><returnMessage>�����ɹ�</returnMessage><group
            // xh="1"><fplx_dm>026</fplx_dm><fpdm>032002000311</fpdm><fphm_q>95087285</fphm_q><fphm_z>95087285</fphm_z><gpzt>2</gpzt></group></body></business>

            _WriteLog(LL_INFO, "Nisec buy invoice all finish, formatStr:%s", szInputOrOutFormatStr);
        } else {  //==3
        }
        nRet = 0;
        break;
    }
    if (pXMLRoot)
        mxmlDelete(pXMLRoot);
    return nRet;
}

//������ɵ�FPд���豸,д����ɺ���Ҫ����
// checkConfig Ϊfplx|fpdm|qshm|fpfs
int WriteGpxxmwToDevice(HDEV hDev, char *szGpxxmw, char *szSingleFormatLine)
{
    uint8 cb[512] = {0}, abWrite[512] = {0};
    char dst[4][768];
    memset(dst, 0, sizeof(dst));
    int nSepCount = 0;

    if ((nSepCount = GetSplitStringSimple(szSingleFormatLine, "|", dst, 4)) != 4)
        return -1;
    int nChildRet = Str2Byte(abWrite, szGpxxmw, strlen(szGpxxmw));
    if (nChildRet <= 0)
        return -2;
    memcpy(cb, "\xfe\x0b\x00\x01", 4);
    if (NisecLogicIO(hDev->hUSB, cb, 4, sizeof cb, abWrite, nChildRet) < 0)
        return -3;
    //д���ټ��һ��
    if (CheckInvoiceExistAtDevice(hDev, dst) <= 0)
        return -4;
    return 0;
}

// szOutFormatStr�����������㹻�������ʽ����
// szFPZL|szTypeCode|szFPHMHead|szFPCount|szGpzt\r\nszFPZL|szTypeCode|szFPHMHead|szFPCount|szGpzt\r\n....
int NisecNetInvoiceQueryReady(HDEV hDev, char *szOutFormatStrMultiLine)
{
    struct HTTP hi;
    int nRet = -1, nChildRet = -1;
    if ((nChildRet = CreateServerTalk(&hi, hDev)) < 0) {
        _WriteLog(LL_DEBUG, "NisecNetInvoiceQueryReady conn failed");
        goto FreeAndExit;
    }
    if ((nChildRet = NisecQueryNetInvoiceReady(0, &hi, szOutFormatStrMultiLine)) < 0) {
        _WriteLog(LL_FATAL, "NetInvoice_QueryReady, IO failed");
        goto FreeAndExit;
    }
    if (!strlen(szOutFormatStrMultiLine)) {
        _WriteLog(LL_FATAL, "NetInvoice_QueryReady, not found avail invoice to download");
        goto FreeAndExit;
    }
    nRet = 0;
FreeAndExit:
    HTTPClose(&hi);
    return nRet;
}

//���豸�������أ�������δ�����ķ�Ʊ���в�ѯ��������˰�ֲ�ѯ�ȽϾ�׼
bool UnlockReadyInvoiceInDevice(void *ptrHHTTP)
{
    HHTTP hi = (HHTTP)ptrHHTTP;
    HDEV hDev = hi->hDev;
    char szBuf[8192] = "";
    int nChildRet = 0;

    if ((nChildRet = NisecQueryNetInvoiceReady(0, hi, szBuf)) < 0) {
        _WriteLog(LL_FATAL, "NetInvoice_QueryReady, IO failed");
        return false;
    }
    bool ret = true;
    bool bDeviceOpen = false;
    SplitInfo spOut;
    memset(&spOut, 0, sizeof(SplitInfo));
    GetSplitString(szBuf, "\r\n", &spOut);
    SplitInfo *pOut = &spOut;
    int i;
    for (i = 0; i < spOut.num; i++) {
        pOut = pOut->next;
        char *pChild = pOut->info;

        char dst[5][768];
        memset(dst, 0, sizeof(dst));
        char szSingleFormatLine[4096] = "";
        int nSepCount = 0;
        if ((nSepCount = GetSplitStringSimple(pChild, "|", dst, 5)) != 5) {
            _WriteLog(LL_FATAL, "DownloadSpecifyInvoice, parament's count incorrect");
            ret = false;
            break;
        }
        if (strcmp(dst[4], "1"))
            continue;
        // need unlock found
        sprintf(szSingleFormatLine, "%s|%s|%s|%s", dst[0], dst[1], dst[2], dst[3]);
        strcpy(szBuf, szSingleFormatLine);
        if (NisecQueryNetInvoiceReady(1, hi, szBuf) < 0)
            continue;
        if (!hDev->bOperateInvType) {
            _WriteLog(LL_FATAL, "Write gpxxmw not set bOperateInvType");
            break;
        }
        // open device
        if (NisecEntry(hDev->hUSB, NOP_OPEN_FULL) < 0) {
            break;
        }
        bDeviceOpen = true;
        NisecSelectInvoiceType(hDev, hDev->bOperateInvType, SELECT_OPTION_TEMPLATE);
        // check exist
        bool existAtDevice = CheckInvoiceExistAtDevice(hDev, dst) > 0;
        if (!existAtDevice && WriteGpxxmwToDevice(hDev, szBuf, szSingleFormatLine) < 0) {
            _WriteLog(LL_WARN, "UnlockReadyInvoiceInDevice WriteGpxxmwToDevice failed");
            ret = false;
            break;
        }
        if ((nChildRet = NisecQueryNetInvoiceReady(2, hi, szSingleFormatLine)) < 0) {
            _WriteLog(LL_WARN, "UnlockReadyInvoiceInDevice result to TA failed");
            ret = false;
            break;
        }
    }

    SplitFree(&spOut);
    if (bDeviceOpen)
        NisecEntry(hDev->hUSB, NOP_CLOSE);
    return ret;
}

// szInputFormatSingleLine example:026|032002000311|95087285|1
//�������szInputFormatSingleLine==0ʱ��ͬ���豸��˰�ֹ�Ʊ��Ϣ����������Ʊ�ѳɹ��Ѵ��ڵ���˰����δͬ��ɾ����Ʊ���
int NisecNetInvoiceDownloadWrite(HDEV hDev, char *szInputFormatSingleLine)
{
    struct HTTP hi;
    char szOutMW[512] = "";
    int nRet = -1, nChildRet = -1;

    if ((nChildRet = CreateServerTalk(&hi, hDev)) < 0) {
        goto FreeAndExit;
    }
    if (strlen(szInputFormatSingleLine) == 0) {
        //��ѯ˰�֌����ڣ�����ʵ�����з�Ʊ��д�벢�ҿɿ�Ʊ������ͬ����ɾ���ֶ���Ϣ
        if ((nChildRet = UnlockReadyInvoiceInDevice(&hi)) < 0) {
            _WriteLog(LL_FATAL, "UnlockReadyInvoiceInDevice failed");
        } else {
            nRet = 0;
        }
        goto FreeAndExit;
    }
    if (strlen(szInputFormatSingleLine) > sizeof(szOutMW)) {
        return -1;
    }
    strcpy(szOutMW, szInputFormatSingleLine);
    if ((nChildRet = NisecQueryNetInvoiceReady(1, &hi, szOutMW)) < 0) {
        _WriteLog(LL_FATAL, "NisecQueryNetInvoiceReady failed");
        goto FreeAndExit;
    }
    // open device
    if (NisecEntry(hDev->hUSB, NOP_OPEN_FULL) < 0) {
        goto FreeAndExit;
    }
    NisecSelectInvoiceType(hDev, hDev->bOperateInvType, SELECT_OPTION_TEMPLATE);
    nChildRet = WriteGpxxmwToDevice(hDev, szOutMW, szInputFormatSingleLine);
    NisecEntry(hDev->hUSB, NOP_CLOSE);
    if (nChildRet < 0) {
        _WriteLog(LL_FATAL, "WriteGpxxmwToDevice failed,last errinfo = %s", hDev->hUSB->errinfo);
        goto FreeAndExit;
    }
    _WriteLog(LL_INFO, "Invoice info has writed to device, notify TA now...");
    if ((nChildRet = NisecQueryNetInvoiceReady(2, &hi, szInputFormatSingleLine)) < 0) {
        _WriteLog(LL_FATAL, "Gpxx write ok, notify TA failed");
        goto FreeAndExit;
    }
    _WriteLog(LL_INFO, "Nisec buy invoice finish");
    nRet = 0;
FreeAndExit:
    HTTPClose(&hi);
    return nRet;
}
//--------------------------------���Ϲ�Ʊ���� Finish--------------------------------------------
//---------------------------------��Ʊ�������� begin----------------------------------------
//
//��Ʊ���ִ����aisino���ã������authorityĿ¼�£�authority/module/ta_vendor_common.c
//
//---------------------------------��Ʊ�������� finish---------------------------------------