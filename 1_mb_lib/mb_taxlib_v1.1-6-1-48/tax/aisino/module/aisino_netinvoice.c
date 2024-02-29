/*****************************************************************************
File name:   aisino_netinvoice.c
Description: �������緢Ʊ�Ļ�ȡ\ȷ��\�ַ������ӹ�������ƽ̨������������أ���Ʊ����У�顢���롢����
Author:      Zako
Version:     1.0
Date:        2020.05
History:
20200514     �������ʵ��
20200914     ��Ʊ���ýӿڷ����������ӿ�
20201121     �����Ʊ����
20201215     ���빫������ƽ̨��ش���
20211122     ��Ʊ�������ִ�����ൽ�ˣ��ϲ�Ϊ���緢Ʊ����
*****************************************************************************/
#include "aisino_netinvoice.h"

//------------------------------------����ɴ��� Begin----------------------------------------
int GetApplicatantInfo(HHTTP hi, char *szCommonTaxID)
{
    char szXML[2048] = "";
    char szBuf[512] = "";
    int nRet = -1;
    mxml_node_t *pXMLRoot = NULL, *node1 = NULL;
    sprintf(
        szXML,
        "<?xml version=\"1.0\" encoding=\"GBK\"?><FPXT><INPUT><NSRSBH>%s</NSRSBH></INPUT></FPXT>",
        szCommonTaxID);
    if ((nRet = TaNetIo(hi, TACMD_AISINO_QUERY_APPLICANT, szXML, sizeof szXML)) < 0) {
        return -1;
    }
    // <?xml version="1.0" encoding="gbk"?>
    //<FPXT><OUTPUT><CODE>0000</CODE><MESS>�ɹ�</MESS><LPRXX><XM>�ΰ��</XM><YDDH>13770851381</YDDH><GDDH>13770851381</GDDH><ZJLX>201</ZJLX>
    //<ZJHM>32118119850525677X</ZJHM><JYDZ>�Ͼ����껨̨���껨��·65��-9����</JYDZ><YZBM>210000</YZBM></LPRXX></OUTPUT></FPXT>
    if (LoadZZSXMLDataNodeTemplate(&pXMLRoot, &node1, "LPRXX", szXML) < 0) {
        _WriteLog(LL_FATAL, "GetAndVerifyInvoiceType2 failed, Msg:%s", szXML);
        goto FreeAndExit;
    }
    if (GetChildNodeValue(node1, "XM", szBuf) < 0)
        goto FreeAndExit;
    _WriteLog(LL_INFO, "GetApplicatantInfo XM:%s", szBuf);
    // todo...
    //...
FreeAndExit:
    if (pXMLRoot)
        mxmlDelete(pXMLRoot);
    return nRet;
}

int GetAndVerifyInvoiceType(HHTTP hi, char *szFPZLDM, char *szOutFPZL)
{
    if (strlen(szFPZLDM) < 2)
        return -1;
    char szXML[8192] = "";  // buff must > 4096
    char szBuf[512] = "";
    char szBuf2[512] = "";
    int nRet = -1;
    mxml_node_t *pXMLRoot = NULL, *node1 = NULL, *nodeResultChildSingle = NULL;
    HDEV hDev = hi->hDev;
    // request
    //<?xml version=\"1.0\"
    // encoding=\"GBK\"?><FPXT><INPUT><NSRSBH>91320116MA1MKN3U59</NSRSBH><KPJH>0</KPJH><SBBH>661913412972</SBBH><DCBB>SKN6K08L1-181018</DCBB></INPUT></FPXT>
    sprintf(szXML,
            "<?xml version=\"1.0\" "
            "encoding=\"GBK\"?><FPXT><INPUT><NSRSBH>%s</NSRSBH><KPJH>%d</KPJH><SBBH>%s</"
            "SBBH><DCBB>%s</DCBB></INPUT></FPXT>",
            hDev->szCommonTaxID, hDev->uICCardNo, hDev->szDeviceID, hDev->szDriverVersion);
    if ((nRet = TaNetIo(hi, TACMD_AISINO_QUERY_INVTYPELIST, szXML, sizeof szXML)) < 0) {
        _WriteLog(LL_FATAL, "GetAndVerifyInvoiceType1 failed, nRet:%d", nRet);
        return -1;
    }
    // reponse
    //<?xml version=\"1.0\"
    // encoding=\"gbk\"?>\n<FPXT><OUTPUT><CODE>0000</CODE><MESS>�ɹ�</MESS><DATA><NSRSBH>91320116MA1MKN3U59</NSRSBH><KPJH>0</KPJH><SBBH>661913412972</SBBH>
    //<PZHDXX><FPZL>2</FPZL><FPZLDM>000008101200</FPZLDM><FPZLMC>2008����ֵ˰��ͨ��Ʊ�������޽�����ư棩</FPZLMC><MCFPXE>25</MCFPXE><MYFPXE>25</MYFPXE><ZGCPL>25</ZGCPL></PZHDXX>
    //<PZHDXX><FPZL>2</FPZL><FPZLDM>04</FPZLDM><FPZLMC>2016����ֵ˰��ͨ��Ʊ�������۵�Ʊ��</FPZLMC><MCFPXE>25</MCFPXE><MYFPXE>25</MYFPXE><ZGCPL>25</ZGCPL></PZHDXX>
    //<PZHDXX><FPZL>0</FPZL><FPZLDM>1130</FPZLDM><FPZLMC>��ֵ˰ר�÷�Ʊ�����������޽�����ư棩</FPZLMC><MCFPXE>25</MCFPXE><MYFPXE>25</MYFPXE><ZGCPL>25</ZGCPL></PZHDXX></DATA></OUTPUT></FPXT>
    if (LoadZZSXMLDataNodeTemplate(&pXMLRoot, &node1, "DATA", szXML) < 0) {
        _WriteLog(LL_FATAL, "GetAndVerifyInvoiceType2 failed, Msg:%s", szXML);
        goto FreeAndExit;
    }
    if ((nodeResultChildSingle = mxmlGetFirstChild(node1)) == NULL)
        goto FreeAndExit;
    bool bHasFind = false;
    nRet = 0;
    do {
        if (GetChildNodeValue(nodeResultChildSingle, "FPZLDM", szBuf) < 0)
            continue;
        if (strcmp(szBuf, szFPZLDM))
            continue;  // not match
        if (GetChildNodeValue(nodeResultChildSingle, "FPZLMC", szBuf) < 0)
            continue;
        if (GetChildNodeValue(nodeResultChildSingle, "MCFPXE", szBuf2) < 0)
            continue;
        if (GetChildNodeValue(nodeResultChildSingle, "FPZL", szOutFPZL) < 0)
            continue;
        _WriteLog(LL_DEBUG, "Select TA invoice type -- FPZL:%s FPZLMC:%s MCFPXE:%s", szOutFPZL,
                  szBuf, szBuf2);
        bHasFind = true;
        nRet = atoi(szBuf2);  // xe
        break;
    } while ((nodeResultChildSingle = mxmlGetNextSibling(nodeResultChildSingle)) != NULL);
    if (!bHasFind)
        _WriteLog(LL_FATAL, "Not found TA invoice type:%s", szFPZLDM);
FreeAndExit:
    if (pXMLRoot)
        mxmlDelete(pXMLRoot);
    return nRet;
}

int BuildReqInvXML(HDEV hDev)
{
    //<?xml version=\"1.0\" encoding=\"GBK\"?><business id=\"fp_sl\"
    // comment=\"��Ʊ����\"><body><nsrsbh>91320116MA1MKN3U59</nsrsbh><ph>661913412972</ph><fplx_dm>026</fplx_dm><slsl>5</slsl><slsj>20200427014059</slsj>
    //<fpzl_dm>11</fpzl_dm><zjlx>201</zjlx><zjhm>32118119850525677X</zjhm><jbrxm>�ΰ��</jbrxm><slsm></slsm><lpfs>1</lpfs></body></business>
    int nRet = -1;
    mxml_node_t *xml_Node = NULL;
    mxml_node_t *root = mxmlNewXML("1.0");
    if (!root)
        goto Finish;
    xml_Node = mxmlNewElement(root, "business");
    mxmlElementSetAttr(xml_Node, "id", "fp_sl");
    mxmlElementSetAttr(xml_Node, "comment", "��Ʊ����");
    xml_Node = mxmlNewElement(xml_Node, "body");
    if (!xml_Node)
        goto Finish;
    mxmlNewText(mxmlNewElement(xml_Node, "nsrsbh"), 0, hDev->szCommonTaxID);
    mxmlNewText(mxmlNewElement(xml_Node, "ph"), 0, hDev->szDeviceID);
    // todo
    //...
    //...
    char *ptr = mxmlSaveAllocString(root, MXML_NO_CALLBACK);
    free(ptr);
    nRet = 0;
Finish:
    mxmlDelete(root);
    return nRet;
}

//��Ʊ����,���ڲ���ʡ�ݽӿ���δ��ͨ,������ʱ�����API�ӿ�,�������������ʡ�ݽӿں��ټ�����ɺ�������
int NetInvoiceApplicate(HDEV hDev, char *szNeedFPZLDM, int nNeedCount)
{
    struct HTTP hi;
    memset(&hi, 0, sizeof(struct HTTP));
    int nRet = -1;
    if ((nRet = CreateServerTalk(&hi, hDev)) < 0)
        goto FreeAndExit;
    char szFPZL[128] = "";
    if ((nRet = GetAndVerifyInvoiceType(&hi, szNeedFPZLDM, szFPZL)) <= 0)
        goto FreeAndExit;
    if (nRet < nNeedCount) {  //�����̫��
        _WriteLog(LL_FATAL, "Not enough invoice to applicate");
        goto FreeAndExit;
    }
    GetApplicatantInfo(&hi, hDev->szCommonTaxID);
    // todo
    //...
    //...
FreeAndExit:
    HTTPClose(&hi);
    return nRet;
}
//------------------------------------����ɴ��� Finish----------------------------------------
//-------------------------------���繺Ʊ�������� begin----------------------------------------
void GetDecimalStr(char *szHashInput, char *szOut)
{
    uint8 ab9ByteHash[16] = {0};
    int nHashLen = Str2Byte(ab9ByteHash, szHashInput, strlen(szHashInput));
    int i = 0;
    char szTmp[16];
    for (i = 0; i < nHashLen; i++) {
        uint8 b = ab9ByteHash[i];
        sprintf(szTmp, "%03d", b);
        strcat(szOut, szTmp);
    }
}

//��ȡFP������Ȩ����
int ReadInvGetAuthorization(HUSB hUSB, char *szOutInfo)
{
    uint8 cb[768];
    int ncbLen = sizeof cb;
    memcpy(cb, "\xfe\xff\xcc\xbc\x53\xe1\xe1\x10\x08\x00", 10);
    int nRet = AisinoLogicIO(hUSB, cb, 10, ncbLen, NULL, 0);
    if (nRet != 17)
        return -1;
    return Base64_Encode((const char *)cb, nRet - 1, szOutInfo);
}

//������ɵ�FPд���豸,д����ɺ���Ҫ����
int WriteDownloadInvDataToDevice(HUSB hUSB, uint8 bAisino_FPLX, char *szInvBase64)
{
    uint8 abTransferData[512];
    uint8 cb[512];
    int nRet = Base64_Decode(szInvBase64, strlen(szInvBase64), (char *)abTransferData);
    if (nRet < 0 || nRet != 0x90) {
        SetLastError(hUSB, ERR_BUF_CHECK, "�������ݽ���ʧ��");
        return -1;
    }
    abTransferData[nRet] = bAisino_FPLX;
    abTransferData[nRet + 1] = MakePackVerityFlag(abTransferData, nRet + 1);
    int nTransferDataLen = nRet + 2;
    memcpy(cb, "\xfe\xff\xcc\xbc\x53\xe1\xe1\x10\x06\x00", 10);
    nRet = AisinoLogicIO(hUSB, cb, 10, sizeof cb, abTransferData, nTransferDataLen);
    if (nRet < 0)  // orig < 12,ʵ���Ϊ <0
        return -2;
    return 0;
}

int NInvWirteConfirmResult(HUSB hUSB, char *szInvBase64, uint8 bFPZLOperateType)
{
    uint8 abTransferData[512];
    uint8 cb[512];
    int nRet = Base64_Decode(szInvBase64, strlen(szInvBase64), (char *)abTransferData);
    if (nRet < 0 || nRet != 40) {
        SetLastError(hUSB, ERR_BUF_CHECK, "�������ݽ���ʧ��");
        return -1;
    }
    abTransferData[nRet] = bFPZLOperateType;  // ZZS==0x1 DZ==0x4
    abTransferData[nRet + 1] = MakePackVerityFlag(abTransferData, nRet + 1);
    int nTransferDataLen = nRet + 2;
    memcpy(cb, "\xfe\xff\xcc\xbc\x53\xe1\xe1\x00\x07\x00", 10);
    nRet = AisinoLogicIO(hUSB, cb, 10, sizeof cb, abTransferData, nTransferDataLen);
    if (nRet != 0) {
        logout(INFO, "TAXLIB", "��Ʊ������", "��Ʊ���� nRet = %d,errinfo��%s��\r\n", nRet,
               hUSB->errinfo);
        return -1;
    }
    return 0;
}

int NetInvoice_DownloadOrUnlockAPI(HHTTP hi, struct InvoiceVol *pInvv)
{
    // ta_cmd=20
    // Aisino.Fwkp.Fplygl.Controller.WebDownload.WebDownloadController.RequestDownloadInput
    // https://github.com/zhangchaoyangaisino/SKJ_FWSK_1.0000
    //<FPXT><INPUT><TYPE>1</TYPE><PROTOCOL>0</PROTOCOL><XZFS>0</XZFS><NSRSBH>91320116MA1MKN3U59</NSRSBH><HASH>050001000037025032034128000</HASH>
    //<YSSH>320100251920228</YSSH><DQBH>320100</DQBH><KPJH>0</KPJH><SBBH>661913412972</SBBH><DCBB>SKN6K08L1-181018</DCBB><FPXZ><FPZL>0</FPZL>
    //<LBDM>3200194130</LBDM><QSHM>23272956</QSHM><FPFS>40</FPFS><GPSQ>3TSHJE4SsqiICpWNBI6rTA==</GPSQ></FPXZ></INPUT></FPXT>
    HDEV hDev = hi->hDev;
    char szBuf[40960] = "";
    mxml_node_t *xml_root = NULL, *xml_Node = NULL;
    mxml_node_t *root = mxmlNewXML("1.0");
    int nRet = -4;
    bool bOnlineDown = false;  //��Ʊ��������ֱ�����ط�Ʊ������Ҫǰ���ȹ�Ʊ����

    if (!root)
        return -3;
    if (!(xml_root = mxmlNewElement(root, "FPXT")))
        goto Finish;
    if (!(xml_root = mxmlNewElement(xml_root, "INPUT")))
        goto Finish;
    // TYPE static
    mxmlNewText(mxmlNewElement(xml_root, "TYPE"), 0, "1");
    // PROTOCOL ?
    mxmlNewText(mxmlNewElement(xml_root, "PROTOCOL"), 0, "0");
    // XZFS ���ط�ʽ
    sprintf(szBuf, "%d", FPLX_AISINO_DZFP);
    if (!strcmp(pInvv->szFPZL, szBuf)) {
        bOnlineDown = true;
        mxmlNewText(mxmlNewElement(xml_root, "XZFS"), 0, "1");
    } else
        mxmlNewText(mxmlNewElement(xml_root, "XZFS"), 0, "0");
    // NSRSBH
    mxmlNewText(mxmlNewElement(xml_root, "NSRSBH"), 0, hDev->szCommonTaxID);
    // 9bit HASH
    memset(szBuf, 0, sizeof szBuf);
    GetDecimalStr(hDev->sz9ByteHashTaxID, szBuf);
    mxmlNewText(mxmlNewElement(xml_root, "HASH"), 0, szBuf);
    // szCompressTaxID
    mxmlNewText(mxmlNewElement(xml_root, "YSSH"), 0, hDev->szCompressTaxID);
    // DQBH �������
    mxmlNewText(mxmlNewElement(xml_root, "DQBH"), 0, hDev->szRegCode);
    // KPJH
    sprintf(szBuf, "%d", hDev->uICCardNo);
    mxmlNewText(mxmlNewElement(xml_root, "KPJH"), 0, szBuf);
    // SBBH
    mxmlNewText(mxmlNewElement(xml_root, "SBBH"), 0, hDev->szDeviceID);
    // DCBB �ײ�汾
    mxmlNewText(mxmlNewElement(xml_root, "DCBB"), 0, hDev->szDriverVersion);
    // +FPXZ or fpjs
    if (pInvv->bDownloadOrUnlock) {
        xml_Node = mxmlNewElement(xml_root, "FPXZ");
        mxmlNewText(mxmlNewElement(xml_Node, "GPSQ"), 0, pInvv->szB64Buf);
    } else {
        xml_Node = mxmlNewElement(xml_root, "FPJS");
        mxmlNewText(mxmlNewElement(xml_Node, "FLAG"), 0, pInvv->szB64Buf);
    }
    // LGXX
    mxmlNewText(mxmlNewElement(xml_Node, "FPZL"), 0, pInvv->szFPZL);
    if (bOnlineDown && pInvv->bDownloadOrUnlock) {
        mxmlNewText(mxmlNewElement(xml_Node, "LBDM"), 0, "");
        mxmlNewText(mxmlNewElement(xml_Node, "QSHM"), 0, "0");
    } else {
        mxmlNewText(mxmlNewElement(xml_Node, "LBDM"), 0, pInvv->szTypeCode);
        mxmlNewText(mxmlNewElement(xml_Node, "QSHM"), 0, pInvv->szFPHMHead);
    }
    mxmlNewText(mxmlNewElement(xml_Node, "FPFS"), 0, pInvv->szFPCount);

    char *ptr = mxmlSaveAllocString(root, MXML_NO_CALLBACK);
    strcpy(szBuf, ptr);
    free(ptr);
    if ((nRet = TaNetIo(hi, TACMD_AISINO_DOWNLOAD_NETINV_READY, szBuf, sizeof szBuf)) < 0) {
        _WriteLog(LL_FATAL, "TaNetIo failed, nRet:%d", nRet);
        goto Finish;
    }
    if (pInvv->bDownloadOrUnlock) {  // download
        //��Ҫ����մ�ǰ�����ķ�Ʊ��,�����޷����ط�Ʊ�µķ�Ʊ��
        //++issue WriteDownloadInvDataToDevice(hDev->hUSB, pInvv->bFPZLOperateType,szBuf)
        if ((nRet = WriteDownloadInvDataToDevice(hDev->hUSB, atoi(pInvv->szFPZL), szBuf)) < 0) {
            logout(INFO, "TAXLIB", "��Ʊ������",
                   "WriteDownloadInvDataToDevice nRet = %d,errinfo��%s��\r\n", nRet,
                   hDev->hUSB->errinfo);
            _WriteLog(LL_FATAL, "WriteDownloadInvDataToDevice failed, nRet:%d", nRet);
            goto Finish;
        }
    } else {  // unlock
        if ((nRet = NInvWirteConfirmResult(hDev->hUSB, szBuf, pInvv->bFPZLOperateType)) < 0) {
            logout(INFO, "TAXLIB", "��Ʊ������",
                   "NInvWirteConfirmResult nRet = %d,errinfo��%s��\r\n", nRet, hDev->hUSB->errinfo);
            _WriteLog(LL_FATAL, "NInvWirteConfirmResult failed, nRet:%d", nRet);
            goto Finish;
        }
    }
    nRet = 0;
Finish:
    mxmlDelete(root);
    return nRet;
}

//��ȡ��δ������FP��Ϣ,���ڽ���˰�ֻ�ȡ������Ϣ
//���ڵ�һ����Ʊδ����,�ڶ�����Ʊ�޷�����,���ֻ����һ�ַ�Ʊδ�������
int NInvGetUnlockInvoice(HHTTP hi, struct InvoiceVol *pInvv)
{  // bQueryType ��ֵ˰1����Ʊ4
    uint8 abTransferData[4], cb[512], abBuf[256];
    memcpy(abTransferData, &pInvv->bFPZLOperateType, 1);
    abTransferData[1] = MakePackVerityFlag(abTransferData, 1);
    memcpy(cb, "\xfe\xff\xcc\xbc\x53\xe1\xe3\x00\x90\x00", 10);
    int nRet = AisinoLogicIO(hi->hDev->hUSB, cb, 10, sizeof cb, abTransferData, 2);
    if (nRet < 24)
        return -1;
    if (0xff == cb[0] && 0xff == cb[1]) {
        _WriteLog(LL_INFO, "NO unlock invoice to unlock");
        return 0;  // no tounlock invoice
    }
    pInvv->bDownloadOrUnlock = false;
    sprintf(pInvv->szFPZL, "%d", cb[0]);   //???��ȷ��,������
    uint32 nNumber = *(uint32 *)(cb + 1);  //��ʼ����
    sprintf(pInvv->szFPHMHead, "%d", nNumber);
    nNumber = *(uint16 *)(cb + 5);  //����
    sprintf(pInvv->szFPCount, "%d", nNumber);
    GetInvTypeCode(9999, cb + 7, pInvv->szTypeCode);
    // FPDM��ȡ�Ƚϸ���,�ݲ�ʵ��
    _WriteLog(LL_DEBUG, "Find to be unlock inv, type:%s FPDM:%s FPHM-head:%s count:%s",
              pInvv->szFPZL, pInvv->szTypeCode, pInvv->szFPHMHead, pInvv->szFPCount);
    //��Ʊ������Ȩ��̶�40�ֽ�
    memcpy(abBuf, cb + 12, 40);
    Base64_Encode((char *)abBuf, 40, pInvv->szB64Buf);
    return nNumber;
}

// szQueryDateRange=20200501-20200530
int QueryNetInvoiceReady(HHTTP hi, char *szQueryDateRange, char *szOutFormatStr)
{
    HDEV hDev = hi->hDev;
    char szInvAuthB64[256] = "", szXML[81920] = "", szGMRQ[256] = "", szLine[128] = "";

    char szBuf[256] = "";
    if (17 != strlen(szQueryDateRange) || !strchr(szQueryDateRange, '-') ||
        '-' != szQueryDateRange[8]) {
        SetLastError(hi->hDev->hUSB, ERR_PARM_FORMAT, "ʱ���ʽ����");
        _WriteLog(LL_FATAL, "QueryDateRange format incorrect, example: 20200401-20200501");
        return -1;
    }
    strcpy(szBuf, szQueryDateRange);
    szBuf[8] = '\0';
    char pDateQ_t[100] = {0};  // szBuf;
    char pDateZ_t[100] = {0};  // szBuf + 9;
    memcpy(pDateQ_t, szBuf, 8);
    memcpy(pDateZ_t, szBuf + 9, 8);

    char pDateQ[100] = {0};
    char pDateZ[100] = {0};
    memcpy(pDateQ, hDev->szDeviceEffectDate, 8);  //ֻȡǰ8λ��Чֵ
    memcpy(pDateZ, hDev->szDeviceTime, 8);
    if (memcmp(pDateQ_t, pDateQ, 8) < 0) {
        memset(pDateQ, 0, sizeof(pDateQ));
        memcpy(pDateQ, pDateQ_t, 8);
    }

    if (memcmp(pDateZ_t, pDateZ, 8) > 0) {
        memset(pDateZ, 0, sizeof(pDateZ));
        memcpy(pDateZ, pDateZ_t, 8);
    }
    _WriteLog(LL_INFO, "��ѯ��ʼʱ�䣺%s,��ѯ����ʱ�䣺%s\n", pDateQ, pDateZ);
    int nRet = -3;
    mxml_node_t *pXMLRoot = NULL, *node1 = NULL, *nodeResultChildSingle = NULL;
    int nAllAvailCount = 0;

    if (ReadInvGetAuthorization(hDev->hUSB, szInvAuthB64) < 0) {
        SetLastError(hi->hDev->hUSB, ERR_BUF_CHECK, "��Ʊ������Ȩ���ȡʧ��");
        _WriteLog(LL_FATAL, "ReadInvGetAuthorization failed");
        return -1;
    }
    sprintf(szXML,
            "<?xml version=\"1.0\" "
            "encoding=\"GBK\"?><FPXT><INPUT><TYPE>1</TYPE><PROTOCOL>1</"
            "PROTOCOL><NSRSBH>%s</NSRSBH><KPJH>%d</KPJH><SBBH>%s</"
            "SBBH><DCBB>%s</DCBB><FSSJ_Q>%s</FSSJ_Q><FSSJ_Z>%s</"
            "FSSJ_Z><QYXZ>Y</QYXZ></INPUT></FPXT>",
            hDev->szCommonTaxID, hDev->uICCardNo, hDev->szDeviceID, hDev->szDriverVersion, pDateQ,
            pDateZ);
    if ((nRet = TaNetIo(hi, TACMD_AISINO_QUERY_NETINV_READY, szXML, sizeof szXML)) < 0) {
        _WriteLog(LL_FATAL, "TaNetIo failed, nRet:%d", nRet);
        return -1;
    }
    // <?xml version=\"1.0\" encoding=\"GBK\" standalone=\"yes\"?>\n<FPXT>\n    <OUTPUT>\n
    // <CODE>0000</CODE>\n        <MESS></MESS>\n        <DATA>\n
    // <NSRSBH>91320116MA1MKN3U59</NSRSBH>\n            <KPJH>0</KPJH>\n
    // <SBBH>661913412972</SBBH>\n
    // </DATA>\n    </OUTPUT>\n</FPXT>\n
    if (LoadZZSXMLDataNodeTemplate(&pXMLRoot, &node1, "DATA", szXML) < 0) {
        _WriteLog(LL_FATAL, "NetInvoiceGet_QueryReady failed, Msg");
        SetLastError(hi->hDev->hUSB, ERR_TA_REPONSE_CHECK, "�������������ݽ���ʧ��");
        goto FreeAndExit;
    }
    if ((node1 = mxmlFindElement(node1, pXMLRoot, "FPMX", NULL, NULL, MXML_DESCEND)) == NULL)
        goto FreeAndExit;
    nodeResultChildSingle = node1;
    //<FPMX><FPZL>2</FPZL><LBDM>032001900204</LBDM><QSHM>14407631</QSHM><FPFS>40</FPFS><GMRQ>20200429155126</GMRQ></FPMX>
    do {
        struct InvoiceVol invv;
        invv.bDownloadOrUnlock = true;
        strcpy(invv.szB64Buf, szInvAuthB64);
        if (GetChildNodeValue(nodeResultChildSingle, "FPZL", invv.szFPZL) < 0)
            continue;
        if (!strcmp(invv.szFPZL, "51"))  // Jump DZFP
        {
            _WriteLog(LL_DEBUG, "Jump DZFP online buy");
            continue;
        }
        if (GetChildNodeValue(nodeResultChildSingle, "LBDM", invv.szTypeCode) < 0)
            continue;
        if (GetChildNodeValue(nodeResultChildSingle, "QSHM", invv.szFPHMHead) < 0)
            continue;
        if (GetChildNodeValue(nodeResultChildSingle, "FPFS", invv.szFPCount) < 0)
            continue;
        if (GetChildNodeValue(nodeResultChildSingle, "GMRQ", szGMRQ) < 0)
            continue;
        _WriteLog(LL_INFO, "NetInvoiceGet, TA invlist -- FPZL:%s LBDM:%s QSHM:%s FPFS:%s GMRQ:%s",
                  invv.szFPZL, invv.szTypeCode, invv.szFPHMHead, invv.szFPCount, szGMRQ);
        nAllAvailCount++;

        sprintf(szLine, "%s|%s|%s|%s\r\n", invv.szFPZL, invv.szTypeCode, invv.szFPHMHead,
                invv.szFPCount);
        strcat(szOutFormatStr, szLine);
    } while ((nodeResultChildSingle = mxmlGetNextSibling(nodeResultChildSingle)) != NULL);
    nRet = 0;

FreeAndExit:
    _WriteLog(LL_INFO, "Get available invoice count:%d", nAllAvailCount);
    return nRet;
}

int ClearLockedInvoiceVolume(HHTTP hi)
{
    //�˴�����һ�ַ�Ʊ��ȡ���˻ز��ֵ���������������������ʣ���˻ط�Ʊ�����֮���ٽ�����Ʊ
    //Ŀǰ��Ʊ����ѽ��øýӿڣ�����Ҳ�����ٿ����������
    uint8 abQueryType[2] = {0x01, 0x04};  // ZZS==0x1 DZ==0x4
    int nTypeCount = sizeof(abQueryType), i = 0, nChildRet = 0;
    struct InvoiceVol invv;
    for (i = 0; i < nTypeCount; i++) {
        while (true) {
            memset(&invv, 0, sizeof(struct InvoiceVol));
            invv.bFPZLOperateType = abQueryType[i];
            nChildRet = NInvGetUnlockInvoice(hi, &invv);
            if (nChildRet < 0) {
                _WriteLog(LL_FATAL,
                          "ClearLockedInvoiceVolume NInvGetUnlockInvoice failed nChildRet = %d",
                          nChildRet);
                return -1;  // IO error
            } else if (nChildRet > 0) {
                //����δ������,��Ҫ�����δ����
                if (NetInvoice_DownloadOrUnlockAPI(hi, &invv) < 0)
                    return -2;
                continue;
            } else  //��ǰ���಻����δ������
                break;
        }
    }
    return 0;
}

// szInFormatStrĩβ��Ҫ��\r\n
int DownloadSpecifyInvoice(HHTTP hi, char *szInFormatStr)
{
    char dst[4][768];
    memset(dst, 0, sizeof(dst));
    int nSepCount = 0;
    if ((nSepCount = GetSplitStringSimple(szInFormatStr, "|", dst, 4)) != 4) {
        _WriteLog(LL_FATAL, "DownloadSpecifyInvoice, parament's count incorrect");
        SetLastError(hi->hDev->hUSB, ERR_PARM_CHECK, "����Ĳ�����Ϣ����");
        return -1;
    }
    struct InvoiceVol invv;
    strcpy(invv.szFPZL, dst[0]);
    strcpy(invv.szTypeCode, dst[1]);
    strcpy(invv.szFPHMHead, dst[2]);
    strcpy(invv.szFPCount, dst[3]);
    // DownloadSpecifyInvoice ר��Ϊ1��DZ==4
    invv.bFPZLOperateType = 0x01;
    // if (!strcmp(invv.szFPZL, "51"))
    //    invv.bFPZLOperateType = 0x04;  //�¼��� ++issue ������
    if (!strlen(invv.szFPZL) || !strlen(invv.szTypeCode) || !strlen(invv.szFPHMHead) ||
        !strlen(invv.szFPCount)) {
        _WriteLog(LL_FATAL, "DownloadSpecifyInvoice, parament's content is empty");
        SetLastError(hi->hDev->hUSB, ERR_PARM_CHECK, "�����ط�Ʊ��������Ϊ��");
        return -2;
    }
    if (strlen(invv.szFPZL) > 3 || strlen(invv.szTypeCode) > 14 || strlen(invv.szFPHMHead) > 12 ||
        strlen(invv.szFPCount) > 10) {
        _WriteLog(LL_FATAL, "DownloadSpecifyInvoice, parament's content is incorrect");
        SetLastError(hi->hDev->hUSB, ERR_PARM_CHECK, "�����ط�Ʊ������������");
        return -3;
    }
    if (ReadInvGetAuthorization(hi->hDev->hUSB, invv.szB64Buf) < 0) {
        SetLastError(hi->hDev->hUSB, ERR_BUF_CHECK, "��Ʊ������Ȩ���ȡʧ��");
        _WriteLog(LL_FATAL, "ReadInvGetAuthorization failed");
        return -4;
    }
    invv.bDownloadOrUnlock = true;
    if (NetInvoice_DownloadOrUnlockAPI(hi, &invv) < 0) {
        _WriteLog(LL_WARN, "Download netinv failed");
        return -5;
    }
    _WriteLog(LL_INFO, "Download invoice,zl:%s lx:%s hmhead:%s count:%s", invv.szFPZL,
              invv.szTypeCode, invv.szFPHMHead, invv.szFPCount);
    return 0;
}

// szOutFormatStr�����������㹻�������ʽ����
// szFPZL|szTypeCode|szFPHMHead|szFPCount\r\nszFPZL|szTypeCode|szFPHMHead|szFPCount\r\n....szFPZL|szTypeCode|szFPHMHead|szFPCount\r\n
//��APIֻ������ר��Ʊ��ѯ����ѯ��Ϻ������ط�д
//��Ʊ���øò�ѯ���裬ֱ������
int NetInvoice_QueryReady(HDEV hDev, char *szInputQueryDateRange, char *szOutFormatStrMultiLine)
{
    struct HTTP hi;
    memset(&hi, 0, sizeof(struct HTTP));
    int nRet = -1, nChildRet = -1;
    if ((nChildRet = CreateServerTalk(&hi, hDev)) < 0) {
        _WriteLog(LL_FATAL, "NetInvoice_QueryReady, conn failed, err:%d", hDev->hUSB->errinfo);
        goto FreeAndExit;
    }
    if ((nChildRet = QueryNetInvoiceReady(&hi, szInputQueryDateRange, szOutFormatStrMultiLine)) <
        0) {
        _WriteLog(LL_FATAL, "NetInvoice_QueryReady, IO failed");
        goto FreeAndExit;
    }
    if (!strlen(szOutFormatStrMultiLine)) {
        SetLastError(hDev->hUSB, ERR_TA_REPONSE_FAILE, "û�в�ѯ��������ȡ�ķ�Ʊ");
        _WriteLog(LL_FATAL, "NetInvoice_QueryReady, not found avail invoice to download");
        goto FreeAndExit;
    }
    //  _WriteLog(LL_DEBUG, "%s", szOutFormatStrMultiLine);
    nRet = 0;
FreeAndExit:
    HTTPClose(&hi);
    return nRet;
}

// szInputFormatStrΪNetInvoice_QueryReady�ӿڲ�ѯ���صĸ�ʽ��������
// szInputFormatStrSingleLine == 2\|032001900204\|51496536\|5
//��Ʊֱ�����أ���Ʊ����������ʼ�������ֱ����0 51\|0\|0\|5;ר��Ʊ��Ҫ�Ȳ�ѯ����ѯ���˲ſ�������
int NetInvoice_DownloadUnlock(HDEV hDev, char *szInputFormatStrSingleLine)
{
    struct HTTP hi;
    memset(&hi, 0, sizeof(struct HTTP));
    int nRet = -1, nChildRet = -1;
    while (1) {
        if ((nChildRet = CreateServerTalk(&hi, hDev)) < 0) {
            _WriteLog(LL_FATAL, "NetInvoice_DownloadUnlock, conn failed, err:%s",
                      hDev->hUSB->errinfo);
            break;
        }
        //���֮ǰ���������ˣ�������δ�����ģ������޷�������һ��
        if ((nChildRet = ClearLockedInvoiceVolume(&hi)) < 0) {
            SetLastError(hDev->hUSB, ERR_LOGIC_CLEAR_LOCKED_INVOICE, "��װ��ʷ��Ʊ���ɹ�");
            _WriteLog(LL_FATAL, "NetInvoice_DownloadUnlock, failed 1,nChildRet = %d", nChildRet);
            break;
        }
        //����ָ����
        if ((nChildRet = DownloadSpecifyInvoice(&hi, szInputFormatStrSingleLine)) < 0) {
            _WriteLog(LL_FATAL, "NetInvoice_DownloadUnlock, failed 2,nChildRet = %d", nChildRet);
            break;
        }
        //���������ص�
        if ((nChildRet = ClearLockedInvoiceVolume(&hi)) < 0) {
            SetLastError(hDev->hUSB, ERR_LOGIC_CLEAR_LOCKED_INVOICE, "��װ��Ʊʧ��");
            _WriteLog(LL_FATAL, "NetInvoice_DownloadUnlock, failed 3,nChildRet = %d", nChildRet);
            break;
        }
        nRet = 0;
        break;
    }
    HTTPClose(&hi);
    return nRet;
}
//-------------------------------���繺Ʊ�������� finish----------------------------------------
//-------------------------------��������ƽ̨���� begin----------------------------------------
//ע�⣡��ʱ���ڿ��ٸ�����ҵ��Ϣ���ײ��ɹ������β�����΢�Ȼ�
int UpdateCompanyInfo(HUSB hUSB, char *szNewCompanyInfoB64)
{
    uint8 cb[512], abCompanyInfoBuf[512];
    if ((strlen(szNewCompanyInfoB64) * 0.75) > sizeof(abCompanyInfoBuf))
        return ERR_BUF_CHECK;
    int nCIBufLength =
        Base64_Decode(szNewCompanyInfoB64, strlen(szNewCompanyInfoB64), (char *)abCompanyInfoBuf);

    abCompanyInfoBuf[nCIBufLength] = MakePackVerityFlag(abCompanyInfoBuf, nCIBufLength);
    nCIBufLength += 1;
    memcpy(cb, "\xfe\xff\xcc\xbc\x53\xe1\xe5\x00\x04\x00", 10);
    if (AisinoLogicIO(hUSB, cb, 10, sizeof cb, abCompanyInfoBuf, nCIBufLength) < 0)
        return ERR_IO_FAIL;
    return RET_SUCCESS;
}
//-------------------------------��������ƽ̨���� Finish----------------------------------------
//-------------------------------��Ʊ�������� begin----------------------------------------

// nDZSYH��ѡ����������
bool LocalCheckCanbeRed(HDEV hDev, char *szLZFPDM, char *szLZFPHM, uint32 nDZSYH)
{
    bool bRet = false;
    HFPXX bluefpxx = MallocFpxxLinkDev(hDev);
    if (!bluefpxx)
        return false;
    //���ṩDZSYH�������˾Ͳ�һ���ܲ鵽������ܲ鵽��У��
    int nChildRet = QueryInvInfo(hDev, szLZFPDM, szLZFPHM, nDZSYH, bluefpxx);
    if (!nChildRet) {  //�鵽��Ʊ
        if (bluefpxx->isRed) {
            _WriteLog(LL_FATAL, "Blule-invoice is red, failed");
            goto Finish;
        }
        if (bluefpxx->zfbz) {
            _WriteLog(LL_FATAL, "Blule-invoice is wasted, failed");
            goto Finish;
        }
    } else {  //����ṩ����Ż��ǲ鲻�����򱨴�;�����Ǵ������Ҫ�أ�Ҳ��������Ϊ��Ʊʱ�䳬��6����
        if (nDZSYH > 0) {
            _WriteLog(LL_FATAL,
                      "Cannot find invoice, maybe an error DZSYH or kpsj > 6month? DZSYH:%d",
                      nDZSYH);
            goto Finish;
        }
    }
    bRet = true;
Finish:
    FreeFpxx(bluefpxx);
    return bRet;
}

int HZXMLNodeMx2FPXX(mxml_node_t *pXMLChildRoot, struct Spxx *spxx)
{
    char szBuf[512] = "";
    char tmp_utf8[512] = {0};
    char spsmmc[200] = {0};
    char spmc[200] = {0};
    memset(tmp_utf8, 0, sizeof(tmp_utf8));
    if (GetChildNodeValue(pXMLChildRoot, "GoodsName", tmp_utf8) < 0)
        return -1;
    GetSpsmmcSpmc(tmp_utf8, spsmmc, spmc);
    EncordingConvert("utf-8", "gbk", spsmmc, strlen(spsmmc), spxx->spsmmc, sizeof(spxx->spsmmc));
    EncordingConvert("utf-8", "gbk", spmc, strlen(spmc), spxx->spmc, sizeof(spmc));
    memset(tmp_utf8, 0, sizeof(tmp_utf8));
    GetChildNodeValue(pXMLChildRoot, "GoodsUnit", tmp_utf8);
    EncordingConvert("utf-8", "gbk", tmp_utf8, strlen(tmp_utf8), spxx->jldw, sizeof(spxx->jldw));
    memset(tmp_utf8, 0, sizeof(tmp_utf8));
    GetChildNodeValue(pXMLChildRoot, "GoodsPrice", tmp_utf8);
    EncordingConvert("utf-8", "gbk", tmp_utf8, strlen(tmp_utf8), spxx->dj, sizeof(spxx->dj));
    memset(tmp_utf8, 0, sizeof(tmp_utf8));
    GetChildNodeValue(pXMLChildRoot, "GoodsTaxRate", tmp_utf8);
    EncordingConvert("utf-8", "gbk", tmp_utf8, strlen(tmp_utf8), spxx->slv, sizeof(spxx->slv));
    memset(tmp_utf8, 0, sizeof(tmp_utf8));
    GetChildNodeValue(pXMLChildRoot, "GoodsGgxh", tmp_utf8);
    EncordingConvert("utf-8", "gbk", tmp_utf8, strlen(tmp_utf8), spxx->ggxh, sizeof(spxx->ggxh));
    memset(tmp_utf8, 0, sizeof(tmp_utf8));
    GetChildNodeValue(pXMLChildRoot, "GoodsNum", tmp_utf8);
    EncordingConvert("utf-8", "gbk", tmp_utf8, strlen(tmp_utf8), spxx->sl, sizeof(spxx->sl));
    memset(tmp_utf8, 0, sizeof(tmp_utf8));
    GetChildNodeValue(pXMLChildRoot, "GoodsJE", tmp_utf8);
    EncordingConvert("utf-8", "gbk", tmp_utf8, strlen(tmp_utf8), spxx->je, sizeof(spxx->je));
    memset(tmp_utf8, 0, sizeof(tmp_utf8));
    GetChildNodeValue(pXMLChildRoot, "GoodsSE", tmp_utf8);
    EncordingConvert("utf-8", "gbk", tmp_utf8, strlen(tmp_utf8), spxx->se, sizeof(spxx->se));
    memset(tmp_utf8, 0, sizeof(tmp_utf8));
    GetChildNodeValue(pXMLChildRoot, "HS_BZ", tmp_utf8);
    EncordingConvert("utf-8", "gbk", tmp_utf8, strlen(tmp_utf8), szBuf, sizeof(szBuf));
    if (!strcmp(szBuf, "N"))
        strcpy(spxx->hsjbz, "0");
    else
        strcpy(spxx->hsjbz, "1");
    memset(tmp_utf8, 0, sizeof(tmp_utf8));
    GetChildNodeValue(pXMLChildRoot, "SPBM", tmp_utf8);
    EncordingConvert("utf-8", "gbk", tmp_utf8, strlen(tmp_utf8), spxx->spbh, sizeof(spxx->spbh));
	memset(tmp_utf8, 0, sizeof(tmp_utf8));
	GetChildNodeValue(pXMLChildRoot, "SYYHZCBZ", tmp_utf8);
	EncordingConvert("utf-8", "gbk", tmp_utf8, strlen(tmp_utf8), spxx->xsyh, sizeof(spxx->xsyh));
    memset(tmp_utf8, 0, sizeof(tmp_utf8));
    GetChildNodeValue(pXMLChildRoot, "YHZC", tmp_utf8);
    EncordingConvert("utf-8", "gbk", tmp_utf8, strlen(tmp_utf8), spxx->yhsm, sizeof(spxx->yhsm));
    memset(tmp_utf8, 0, sizeof(tmp_utf8));
    GetChildNodeValue(pXMLChildRoot, "LSLBZ", tmp_utf8);
    EncordingConvert("utf-8", "gbk", tmp_utf8, strlen(tmp_utf8), spxx->lslvbs,
                     sizeof(spxx->lslvbs));
    return 0;
}

int HZXMLNodeMx2FPXXCntax(mxml_node_t *pXMLChildRoot, struct Spxx *spxx)
{
    char szBuf[512] = "";
    char tmp_utf8[512] = {0};
    char spsmmc[200] = {0};
    char spmc[200] = {0};
    memset(tmp_utf8, 0, sizeof(tmp_utf8));
    if (GetChildNodeValue(pXMLChildRoot, "xmmc", tmp_utf8) < 0)
        return -1;
    GetSpsmmcSpmc(tmp_utf8, spsmmc, spmc);
    EncordingConvert("utf-8", "gbk", spsmmc, strlen(spsmmc), spxx->spsmmc, sizeof(spxx->spsmmc));
    EncordingConvert("utf-8", "gbk", spmc, strlen(spmc), spxx->spmc, sizeof(spmc));
    memset(tmp_utf8, 0, sizeof(tmp_utf8));
    GetChildNodeValue(pXMLChildRoot, "xmdw", tmp_utf8);
    EncordingConvert("utf-8", "gbk", tmp_utf8, strlen(tmp_utf8), spxx->jldw, sizeof(spxx->jldw));
    memset(tmp_utf8, 0, sizeof(tmp_utf8));
    GetChildNodeValue(pXMLChildRoot, "xmdj", tmp_utf8);
    EncordingConvert("utf-8", "gbk", tmp_utf8, strlen(tmp_utf8), spxx->dj, sizeof(spxx->dj));
    memset(tmp_utf8, 0, sizeof(tmp_utf8));
    GetChildNodeValue(pXMLChildRoot, "xmslv", tmp_utf8);
    EncordingConvert("utf-8", "gbk", tmp_utf8, strlen(tmp_utf8), spxx->slv, sizeof(spxx->slv));
    memset(tmp_utf8, 0, sizeof(tmp_utf8));
    GetChildNodeValue(pXMLChildRoot, "xmggxh", tmp_utf8);
    EncordingConvert("utf-8", "gbk", tmp_utf8, strlen(tmp_utf8), spxx->ggxh, sizeof(spxx->ggxh));
    memset(tmp_utf8, 0, sizeof(tmp_utf8));
    GetChildNodeValue(pXMLChildRoot, "xmsl", tmp_utf8);
    EncordingConvert("utf-8", "gbk", tmp_utf8, strlen(tmp_utf8), spxx->sl, sizeof(spxx->sl));
    memset(tmp_utf8, 0, sizeof(tmp_utf8));
    GetChildNodeValue(pXMLChildRoot, "xmje", tmp_utf8);
    EncordingConvert("utf-8", "gbk", tmp_utf8, strlen(tmp_utf8), spxx->je, sizeof(spxx->je));
    memset(tmp_utf8, 0, sizeof(tmp_utf8));
    GetChildNodeValue(pXMLChildRoot, "xmse", tmp_utf8);
    EncordingConvert("utf-8", "gbk", tmp_utf8, strlen(tmp_utf8), spxx->se, sizeof(spxx->se));
    memset(tmp_utf8, 0, sizeof(tmp_utf8));
    GetChildNodeValue(pXMLChildRoot, "hsbz", tmp_utf8);
    EncordingConvert("utf-8", "gbk", tmp_utf8, strlen(tmp_utf8), szBuf, sizeof(szBuf));
    if (!strcmp(szBuf, "N"))
        strcpy(spxx->hsjbz, "0");
    else
        strcpy(spxx->hsjbz, "1");
    memset(tmp_utf8, 0, sizeof(tmp_utf8));
    GetChildNodeValue(pXMLChildRoot, "ssflbm", tmp_utf8);
    EncordingConvert("utf-8", "gbk", tmp_utf8, strlen(tmp_utf8), spxx->spbh, sizeof(spxx->spbh));
    memset(tmp_utf8, 0, sizeof(tmp_utf8));
    GetChildNodeValue(pXMLChildRoot, "zzstsgl", tmp_utf8);
    EncordingConvert("utf-8", "gbk", tmp_utf8, strlen(tmp_utf8), spxx->yhsm, sizeof(spxx->yhsm));
    memset(tmp_utf8, 0, sizeof(tmp_utf8));
    GetChildNodeValue(pXMLChildRoot, "lslbz", tmp_utf8);
    EncordingConvert("utf-8", "gbk", tmp_utf8, strlen(tmp_utf8), spxx->lslvbs,
                     sizeof(spxx->lslvbs));
    return 0;
}

int HZXMLNode2FPXX(mxml_node_t *pXMLChildRoot, HFPXX fpxx)
{
    mxml_node_t *node = NULL;
    char tmp_xfmc[200];
    char tmp_gfmc[200];
    if (GetChildNodeValue(pXMLChildRoot, "TypeCode", fpxx->blueFpdm) < 0)
        return -1;
    if (GetChildNodeValue(pXMLChildRoot, "InvNo", fpxx->blueFphm) < 0)
        return -2;

    char date[20] = {0};
    char tmp_ztmc[200] = {0};
    GetChildNodeValue(pXMLChildRoot, "Date", date);
    sprintf(fpxx->kpsj_F2, "%s 00:00:00", date);
    fpxx->fplx = FPLX_COMMON_ZYFP;
    sprintf(fpxx->qdbj, "N");
    GetChildNodeValue(pXMLChildRoot, "ResBillNo", fpxx->redNum);
    GetChildNodeValue(pXMLChildRoot, "ReqBillNo", fpxx->redNum_serial);
    GetChildNodeValue(pXMLChildRoot, "StatusDM", fpxx->redNum_ztdm);
    GetChildNodeValue(pXMLChildRoot, "StatusMC", tmp_ztmc);
    EncordingConvert("utf-8", "gbk", tmp_ztmc, strlen(tmp_ztmc), fpxx->redNum_ztmc,
                     sizeof(fpxx->redNum_ztmc));
    GetChildNodeValue(pXMLChildRoot, "BuyerName", tmp_gfmc);
    EncordingConvert("utf-8", "gbk", tmp_gfmc, strlen(tmp_gfmc), fpxx->gfmc, sizeof(fpxx->gfmc));
    GetChildNodeValue(pXMLChildRoot, "BuyTaxCode", fpxx->gfsh);
    if (GetChildNodeValue(pXMLChildRoot, "SellerName", tmp_xfmc) < 0)
        return -3;
    EncordingConvert("utf-8", "gbk", tmp_xfmc, strlen(tmp_xfmc), fpxx->xfmc, sizeof(fpxx->gfmc));
    if (GetChildNodeValue(pXMLChildRoot, "SellTaxCode", fpxx->xfsh) < 0)
        return -4;
    GetChildNodeValue(pXMLChildRoot, "Amount", fpxx->je);
    GetChildNodeValue(pXMLChildRoot, "TaxRate", fpxx->slv);
    GetChildNodeValue(pXMLChildRoot, "Tax", fpxx->se);
	GetChildNodeValue(pXMLChildRoot, "ReqMemo", fpxx->redNum_sqsm);
    GetChildNodeValue(pXMLChildRoot, "SPBMBBH", fpxx->bmbbbh);

    float jshj;
    jshj = atof(fpxx->je) + atof(fpxx->se);
    sprintf(fpxx->jshj, "%4.2f", jshj);
    // spmx
    if ((node = mxmlFindElement(pXMLChildRoot, pXMLChildRoot, "RedInvReqBillMx", NULL, NULL,
                                MXML_DESCEND)) == NULL)
        return -5;
    if ((node = mxmlGetFirstChild(node)) == NULL)
        return -6;
    while (1) {
        if ((node = mxmlGetNextSibling(node)) == NULL)
            break;
        fpxx->spsl += 1;
        struct Spxx *spxx = calloc(1, sizeof(struct Spxx));
        if (HZXMLNodeMx2FPXX(node, spxx) < 0) {
            free(spxx);
            break;
        }
        InsertMxxx(fpxx, spxx);
        if ((node = mxmlGetNextSibling(node)) == NULL)
            break;
    }
    return 0;
}

int HZXMLNode2FPXXCntax(mxml_node_t *pXMLChildRoot, HFPXX fpxx)
{
    mxml_node_t *node = NULL;
    char tmp_xfmc[200];
    char tmp_gfmc[200];
    char tmp_gfdzdh[200];
    char tmp_gfyhzh[200];
    if (GetChildNodeValue(pXMLChildRoot, "yfpdm", fpxx->blueFpdm) < 0)
        return -1;
    if (GetChildNodeValue(pXMLChildRoot, "yfphm", fpxx->blueFphm) < 0)
        return -2;

    char date[20] = {0};
    char tmp_ztmc[200] = {0};
    GetChildNodeValue(pXMLChildRoot, "tkrq", date);
    sprintf(fpxx->kpsj_F2, "%s 00:00:00", date);
    fpxx->fplx = FPLX_COMMON_ZYFP;
    sprintf(fpxx->qdbj, "N");
    GetChildNodeValue(pXMLChildRoot, "xxbbh", fpxx->redNum);
    GetChildNodeValue(pXMLChildRoot, "sqdbh", fpxx->redNum_serial);
    GetChildNodeValue(pXMLChildRoot, "xxbztdm", fpxx->redNum_ztdm);
    GetChildNodeValue(pXMLChildRoot, "xxbztms", tmp_ztmc);
    EncordingConvert("utf-8", "gbk", tmp_ztmc, strlen(tmp_ztmc), fpxx->redNum_ztmc,
                     sizeof(fpxx->redNum_ztmc));
    GetChildNodeValue(pXMLChildRoot, "gmfmc", tmp_gfmc);
    EncordingConvert("utf-8", "gbk", tmp_gfmc, strlen(tmp_gfmc), fpxx->gfmc, sizeof(fpxx->gfmc));
    GetChildNodeValue(pXMLChildRoot, "gmfdzdh", tmp_gfdzdh);
    EncordingConvert("utf-8", "gbk", tmp_gfdzdh, strlen(tmp_gfdzdh), fpxx->gfdzdh,
                     sizeof(fpxx->gfdzdh));
    GetChildNodeValue(pXMLChildRoot, "gmfyhzh", tmp_gfyhzh);
    EncordingConvert("utf-8", "gbk", tmp_gfyhzh, strlen(tmp_gfyhzh), fpxx->gfyhzh,
                     sizeof(fpxx->gfyhzh));
    GetChildNodeValue(pXMLChildRoot, "gmfsbh", fpxx->gfsh);
    if (GetChildNodeValue(pXMLChildRoot, "xsfmc", tmp_xfmc) < 0)
        return -3;
    EncordingConvert("utf-8", "gbk", tmp_xfmc, strlen(tmp_xfmc), fpxx->xfmc, sizeof(fpxx->gfmc));
    if (GetChildNodeValue(pXMLChildRoot, "xsfsbh", fpxx->xfsh) < 0)
        return -4;
    GetChildNodeValue(pXMLChildRoot, "hjje", fpxx->je);
    GetChildNodeValue(pXMLChildRoot, "slv", fpxx->slv);
    GetChildNodeValue(pXMLChildRoot, "hjse", fpxx->se);
	GetChildNodeValue(pXMLChildRoot, "sqsm", fpxx->redNum_sqsm);
    GetChildNodeValue(pXMLChildRoot, "ssflbmbbh", fpxx->bmbbbh);

    float jshj;
    jshj = atof(fpxx->je) + atof(fpxx->se);
    sprintf(fpxx->jshj, "%4.2f", jshj);
    // spmx
    if ((node = mxmlFindElement(pXMLChildRoot, pXMLChildRoot, "xmmxs", NULL, NULL, MXML_DESCEND)) ==
        NULL)
        return -5;
    if ((node = mxmlGetFirstChild(node)) == NULL)
        return -6;
    while (1) {
        if ((node = mxmlGetNextSibling(node)) == NULL)
            break;
        fpxx->spsl += 1;
        struct Spxx *spxx = calloc(1, sizeof(struct Spxx));
        if (HZXMLNodeMx2FPXXCntax(node, spxx) < 0) {
            free(spxx);
            break;
        }
        InsertMxxx(fpxx, spxx);
        if ((node = mxmlGetNextSibling(node)) == NULL)
            break;
    }
    return 0;
}

// fpxx need init by caller, free by caller
int GetFpxxFromRepByRedNum(char *szRep, char *szRedNum, HFPXX fpxx)
{
    // 3201142004040539
    mxml_node_t *pXMLRoot = NULL, *node = NULL, *nodeResultChildSingle = NULL;
    int nRet = -10, i = 0, nCount = 0;
    char *pPayload = NULL, szBuf[512] = "";
    if (LoadZZSXMLDataNodeTemplate(&pXMLRoot, &node, "ALLCOUNT", szRep) < 0) {
        _WriteLog(LL_FATAL, "LoadZZSXMLDataNodeTemplate failed, Msg:%s", szRep);
        return -1;
    }
    if ((pPayload = (char *)mxmlGetOpaque(node)) == NULL) {
        nRet = -6;
        goto Finish;
    }
    nCount = atoi(pPayload);
    nodeResultChildSingle = node;
    // node1 node2 û���˿��Ա���Ϊ����ֵ
    for (i = 0; i < nCount * 2; i++) {
        //��Ҫ���Σ������Ϊʲô
        if ((nodeResultChildSingle = mxmlGetNextSibling(nodeResultChildSingle)) == NULL) {
            nRet = -5;
            goto Finish;
        }
        if ((nodeResultChildSingle = mxmlGetNextSibling(nodeResultChildSingle)) == NULL) {
            nRet = -6;
            goto Finish;
        }
        if ((node = mxmlFindElement(nodeResultChildSingle, pXMLRoot, "ResBillNo", NULL, NULL,
                                    MXML_DESCEND)) == NULL)
            continue;
        if ((pPayload = (char *)mxmlGetOpaque(node)) == NULL)
            continue;
        if (strcmp(szRedNum, pPayload))
            continue;  // not match
        if (GetChildNodeValue(nodeResultChildSingle, "StatusDM", szBuf) < 0) {
            nRet = -7;
            goto Finish;
        }
        // if (strcmp(szBuf, "TZD0000"))
        //{
        //    _WriteLog(LL_WARN, "Found red tzdbh, but server say it is failed");
        //	if (errinfo != NULL)
        //		sprintf(errinfo, "���״̬Ϊδͨ��,���룺%s", szBuf);
        //    nRet = -100;
        //    goto Finish;
        //}
        nRet = HZXMLNode2FPXX(nodeResultChildSingle, fpxx);
        break;
    }

Finish:
    if (pXMLRoot)
        mxmlDelete(pXMLRoot);
    return nRet;
}

// fpxx need init by caller, free by caller
int GetFpxxFromRepByRedNumCntax(char *szRep, char *szRedNum, HFPXX fpxx)
{
    // 3201142004040539
    mxml_node_t *pXMLRoot = NULL, *node = NULL, *nodeResultChildSingle = NULL;
    int nRet = -10, i = 0, nCount = 0;
    char *pPayload = NULL, szBuf[512] = "";
    if (LoadZZSXMLDataNodeTemplateCntax(&pXMLRoot, &node, "fhtjjls", szRep) < 0) {
        _WriteLog(LL_FATAL, "LoadZZSXMLDataNodeTemplate failed, Msg:%s", szRep);
        return -1;
    }
    if ((pPayload = (char *)mxmlGetOpaque(node)) == NULL) {
        nRet = -6;
        goto Finish;
    }
    nCount = atoi(pPayload);
    nodeResultChildSingle = node;
    // node1 node2 û���˿��Ա���Ϊ����ֵ
    for (i = 0; i < nCount * 2; i++) {
        //��Ҫ���Σ������Ϊʲô
        if ((nodeResultChildSingle = mxmlGetNextSibling(nodeResultChildSingle)) == NULL) {
            nRet = -5;
            goto Finish;
        }
        if ((nodeResultChildSingle = mxmlGetNextSibling(nodeResultChildSingle)) == NULL) {
            nRet = -6;
            goto Finish;
        }
        if ((node = mxmlFindElement(nodeResultChildSingle, pXMLRoot, "xxbbh", NULL, NULL,
                                    MXML_DESCEND)) == NULL)
            continue;
        if ((pPayload = (char *)mxmlGetOpaque(node)) == NULL)
            continue;
        if (strcmp(szRedNum, pPayload))
            continue;  // not match
        if (GetChildNodeValue(nodeResultChildSingle, "xxbztdm", szBuf) < 0) {
            nRet = -7;
            goto Finish;
        }
        // if (strcmp(szBuf, "TZD0000"))
        //{
        //    _WriteLog(LL_WARN, "Found red tzdbh, but server say it is failed");
        //	if (errinfo != NULL)
        //		sprintf(errinfo, "���״̬Ϊδͨ��,���룺%s", szBuf);
        //    nRet = -100;
        //    goto Finish;
        //}
        nRet = HZXMLNode2FPXXCntax(nodeResultChildSingle, fpxx);
        break;
    }

Finish:
    if (pXMLRoot)
        mxmlDelete(pXMLRoot);
    return nRet;
}

int AnalyzeZYDZResult(uint8 bAisinoFPLX, char *szResult, char *szOutMsg)
{
    mxml_node_t *root = NULL, *node1 = NULL;
    char *pPayload = NULL;
    int nRet = -2;
    char szFlag[128] = "";
    if ((root = mxmlLoadString(NULL, szResult, MXML_OPAQUE_CALLBACK)) == NULL)
        return -1;
    if (FPLX_AISINO_PTFP == bAisinoFPLX) {
        // PT
        //��Ʊ�������
        //<FPXT><OUTPUT><CODE>0000</CODE><MESS>�ɹ�</MESS><HZJE>-0</HZJE></OUTPUT></FPXT>
        //�鲻��������������?
        //<MESS>xxxxxxxxxx</MESS><HZJE></HZJE>
        while (1) {
            if ((node1 = mxmlFindElement(root, root, "HZJE", NULL, NULL, MXML_DESCEND)) == NULL) {
                nRet = -3;
                break;
            }
            if ((pPayload = (char *)mxmlGetOpaque(node1)) == NULL) {
                if ((node1 = mxmlFindElement(root, root, "MESS", NULL, NULL, MXML_DESCEND)) ==
                    NULL) {
                    nRet = -4;
                    break;
                }
                if ((pPayload = (char *)mxmlGetOpaque(node1)) == NULL) {
                    nRet = -5;
                    break;
                }
                strcpy(szOutMsg, pPayload);
                nRet = -6;
                break;
            }
            if (!strcmp(pPayload, "-0")) {
                strcpy(szOutMsg, "LZFPDMM/LZFPHM not found, please check");
                nRet = -7;
                break;
            }
            szOutMsg[0] = '\0';
            nRet = 0;
            break;
        }
        goto Finish;
    }
    // DZ
    //�鲻��������������?
    //<?xml version="1.0" encoding="gbk" standalone="yes"?><business id="hzkpje"
    // comment="������ͨ��Ʊ����У��"><body count="1" nsrsbh="91320116MA1MKN3U59"
    // skph="661913412972"><returnCode>0</returnCode><returnMessage>δ�ҵ�ԭ��Ʊ</returnMessage><group
    // xh="1"><data name="fplx_dm" value="026"/><data name="yfpdm" value="032001700211"/><data
    // name="yfphm" value="22222222"/><data name="hzje" value=""/><data name="cpyxx"
    // value=""/></group></body></business>
    //��õ�������,successful
    // returnMessage = �ɹ�, hzje = "XX"
    //��Ʊ�������
    // returnMessage = �ɹ�, hzje = "-0"
    while (1) {
        if ((node1 = mxmlFindElement(root, root, "group", NULL, NULL, MXML_DESCEND)) == NULL) {
            nRet = -7;
            break;
        }
        if (LoopFindChildNodeValue(node1, "hzje", szFlag)) {
            nRet = -8;
            break;
        }
        if (!strcmp(szFlag, "-0")) {
            strcpy(szOutMsg, "LZFPDM/LZFPHM not found, please check");
            nRet = -9;
            break;
        }
        if (!strcmp(szFlag, "")) {
            if ((node1 = mxmlFindElement(root, root, "returnMessage", NULL, NULL, MXML_DESCEND)) ==
                NULL) {
                nRet = -10;
                break;
            }
            if ((pPayload = (char *)mxmlGetOpaque(node1)) == NULL) {
                nRet = -11;
                break;
            }
            strcpy(szOutMsg, pPayload);
            nRet = -12;
            break;
        }
        szOutMsg[0] = '\0';
        nRet = 0;
        break;
    }
Finish:
    if (root)
        mxmlDelete(root);
    return nRet;
}

// nDZSYH��ѡ����(0=���ṩ����>0)������ṩ��϶��ܲ鵽�������������ݣ�����+����˫У�飻������ṩ��ֻ��������У��
//��Ҫ���ڵ��Ӻ���ͨ��Ʊ��Ʊ��ƱǰУ��
//!!!ע��,�����ṩ�����������ĺϷ���Ʊ����,��Ʊ������Դ���;����ṩ����ķ�Ʊ����,�������᷵�ش������ʾ-"�ɹ�",������
int RedInvOnlineCheck_PTDZ(HDEV hDev, uint8 bAisinoFPLX, char *szLZFPDM, char *szLZFPHM,
                           uint32 nDZSYH, char *szOutMsg, int nOutMsgBuffSize)
{
    if (strlen(szLZFPDM) < 6 || strlen(szLZFPHM) < 6) {
        _WriteLog(LL_FATAL, "Incorrect fpdm/fphm");
        return -1;
    }
    if (FPLX_AISINO_PTFP != bAisinoFPLX && FPLX_AISINO_DZFP != bAisinoFPLX) {
        _WriteLog(LL_FATAL, "Just query DZFP/PTFP");
        return -2;
    }
    int nRet = -3;
    char szBuf[512] = "";
    mxml_node_t *xml_root = NULL, *xml_Node = NULL, *xml_Node2 = NULL;
    mxml_node_t *root = mxmlNewXML("1.0");
    if (!root)
        goto Finish;
    if (!LocalCheckCanbeRed(hDev, szLZFPDM, szLZFPHM, nDZSYH))
        goto Finish;
    if (FPLX_AISINO_PTFP == bAisinoFPLX) {
        // request
        //<?xml version=\"1.0\"
        // encoding=\"GBK\"?><FPXT><INPUT><NSRSBH>91320116MA1MKN3U59</NSRSBH><KPJH>0</KPJH><SBBH>661913412972</SBBH>
        //<LZFPDM>032001700211</LZFPDM><LZFPHM>22222222</LZFPHM><FPZL>p</FPZL></INPUT></FPXT>
        //---XML Operate
        if (!(xml_root = mxmlNewElement(root, "FPXT")))
            goto Finish;
        if (!(xml_root = mxmlNewElement(xml_root, "INPUT")))
            goto Finish;
        // NSRSBH
        xml_Node = mxmlNewElement(xml_root, "NSRSBH");
        mxmlNewText(xml_Node, 0, hDev->szCommonTaxID);
        // KPJH
        sprintf(szBuf, "%d", hDev->uICCardNo);
        xml_Node = mxmlNewElement(xml_root, "KPJH");
        mxmlNewText(xml_Node, 0, szBuf);
        // SBBH
        xml_Node = mxmlNewElement(xml_root, "SBBH");
        mxmlNewText(xml_Node, 0, hDev->szDeviceID);
        // LZFPDM
        xml_Node = mxmlNewElement(xml_root, "LZFPDM");
        mxmlNewText(xml_Node, 0, szLZFPDM);
        // LZFPHM
        xml_Node = mxmlNewElement(xml_root, "LZFPHM");
        mxmlNewText(xml_Node, 0, szLZFPHM);
        // FPZL
        AisinoFPLX2Str(bAisinoFPLX, szBuf);
        xml_Node = mxmlNewElement(xml_root, "FPZL");
        mxmlNewText(xml_Node, 0, szBuf);
    } else if (FPLX_AISINO_DZFP == bAisinoFPLX) {
        //<?xml version="1.0" encoding="GBK"?><business id="hzkpje"
        // comment="������ͨ��Ʊ(����)����У��"><body count="1" skph="661913412972"
        // nsrsbh="91320116MA1MKN3U59"><group xh="1"><data name="fplx_dm" value="026" /><data
        // name="yfpdm" value="032001700211" /><data name="yfphm" value="22222222"
        ///></group></body></business>
        xml_Node = mxmlNewElement(root, "business");
        mxmlElementSetAttr(xml_Node, "id", "hzkpje");
        //     mxmlElementSetAttr(xml_Node, "comment", "������ͨ��Ʊ(����)����У��");
        xml_Node = mxmlNewElement(xml_Node, "body");
        if (!xml_Node)
            goto Finish;
        mxmlElementSetAttr(xml_Node, "count", "1");
        mxmlElementSetAttr(xml_Node, "skph", hDev->szDeviceID);
        mxmlElementSetAttr(xml_Node, "nsrsbh", hDev->szCommonTaxID);
        // group
        xml_Node = mxmlNewElement(xml_Node, "group");
        mxmlElementSetAttr(xml_Node, "xh", "1");
        // data1
        xml_Node2 = mxmlNewElement(xml_Node, "data");
        mxmlElementSetAttr(xml_Node2, "name", "fplx_dm");
        mxmlElementSetAttr(xml_Node2, "value", "026");
        // data2
        xml_Node2 = mxmlNewElement(xml_Node, "data");
        mxmlElementSetAttr(xml_Node2, "name", "yfpdm");
        mxmlElementSetAttr(xml_Node2, "value", szLZFPDM);
        // data3
        xml_Node2 = mxmlNewElement(xml_Node, "data");
        mxmlElementSetAttr(xml_Node2, "name", "yfphm");
        mxmlElementSetAttr(xml_Node2, "value", szLZFPHM);
    } else {
        _WriteLog(LL_FATAL, "Just query DZFP/PTFP");
        goto Finish;
    }
    char *ptr = mxmlSaveAllocString(root, MXML_NO_CALLBACK);
    char *pNew = StrReplace(ptr, "\n", " ");
    strcpy(szOutMsg, pNew);  //�����»�����
    free(ptr);
    free(pNew);
    if (FPLX_AISINO_PTFP == bAisinoFPLX)
        nRet = TaNetIoOnce(hDev, TACMD_AISINO_REDFORM_PTCHECK, szOutMsg, nOutMsgBuffSize);
    else if (FPLX_AISINO_DZFP == bAisinoFPLX)
        nRet = TaNetIoOnce(hDev, TACMD_AISINO_REDFORM_DZCHECK, szOutMsg, nOutMsgBuffSize);
    if (nRet < 0) {
        _WriteLog(LL_FATAL, "NetIO failed");
        goto Finish;
    }
    nRet = AnalyzeZYDZResult(bAisinoFPLX, szOutMsg, szOutMsg);  // 0=OK
    _WriteLog(LL_INFO, "[%s] RedInvOnlineCheck=%s FPLX:%d LZFPDM:%s LZFPHM:%s, retmsg:%s",
              nRet == 0 ? "+" : "-", nRet == 0 ? "True" : "False", bAisinoFPLX, szLZFPDM, szLZFPHM,
              szOutMsg);
Finish:
    mxmlDelete(root);
    return nRet;
}

// mini�򵥶Ա�
int ZYRedSimpleCompare(HFPXX fpLocal, HFPXX fpRemote)
{
    int spsl;
    int i;
    if (strcmp(fpLocal->xfmc, fpRemote->xfmc) || strcmp(fpLocal->xfsh, fpRemote->xfsh)) {
        _WriteLog(LL_FATAL,
                  "ZYRedSimpleCompare xf not match,fpLocal->xfmc = %s,fpRemote->xfmc = %s",
                  fpLocal->xfmc, fpRemote->xfmc);
        _WriteLog(LL_FATAL,
                  "ZYRedSimpleCompare xf not match,fpLocal->xfsh = %s,fpRemote->xfsh = %s",
                  fpLocal->xfsh, fpRemote->xfsh);
        return -1;
    }
    if (strcmp(fpLocal->gfmc, fpRemote->gfmc) || strcmp(fpLocal->gfsh, fpRemote->gfsh)) {
        _WriteLog(LL_FATAL,
                  "ZYRedSimpleCompare gf not match,fpLocal->gfmc = %s,fpRemote->gfmc = %s",
                  fpLocal->gfmc, fpRemote->gfmc);
        _WriteLog(LL_FATAL,
                  "ZYRedSimpleCompare gf not match,fpLocal->gfsh = %s,fpRemote->gfsh = %s",
                  fpLocal->gfsh, fpRemote->gfsh);
        return -2;
    }
    if (strcmp(fpLocal->je, fpRemote->je) || strcmp(fpLocal->slv, fpRemote->slv) ||
        strcmp(fpLocal->se, fpRemote->se)) {
        _WriteLog(LL_FATAL, "ZYRedSimpleCompare je/slv/se not match");
        return -3;
    }
    spsl = fpLocal->spsl;  // local����
    struct Spxx *spxxLocal = fpLocal->stp_MxxxHead;
    struct Spxx *spxxRemote = fpRemote->stp_MxxxHead;
    for (i = 0; i < spsl; i++) {

        spxxLocal = spxxLocal->stp_next;
        spxxRemote = spxxRemote->stp_next;
        if (!spxxLocal || !spxxRemote) {
            _WriteLog(LL_FATAL, "ZYRedSimpleCompare spxx's num not match, line:%d", i + 1);
            return -10;
        }
        if (strcmp(spxxLocal->spsmmc, spxxRemote->spsmmc) ||
            strcmp(spxxLocal->spmc, spxxRemote->spmc)) {
            _WriteLog(LL_FATAL, "ZYRedSimpleCompare spsmmc/spmc not match, line:%d", i + 1);
            _WriteLog(LL_FATAL,
                      "ZYRedSimpleCompare spxxLocal->spsmmc = %s, spxxRemote->spsmmc = %s, line:%d",
                      spxxLocal->spsmmc, spxxRemote->spsmmc, i + 1);
            _WriteLog(LL_FATAL,
                      "ZYRedSimpleCompare spxxLocal->spmc = %s, spxxRemote->spmc = %s, line:%d",
                      spxxLocal->spmc, spxxRemote->spmc, i + 1);
            return -11;
        }
        if (strcmp(spxxLocal->jldw, spxxRemote->jldw) ||
            strcmp(spxxLocal->ggxh, spxxRemote->ggxh)) {
            _WriteLog(LL_FATAL, "ZYRedSimpleCompare jldw/ggxh not match, line:%d", i + 1);
            _WriteLog(LL_FATAL,
                      "ZYRedSimpleCompare spxxLocal->jldw = %s, spxxRemote->jldw = %s, line:%d",
                      spxxLocal->jldw, spxxRemote->jldw, i + 1);
            _WriteLog(LL_FATAL,
                      "ZYRedSimpleCompare spxxLocal->ggxh = %s, spxxRemote->ggxh = %s, line:%d",
                      spxxLocal->ggxh, spxxRemote->ggxh, i + 1);
            return -12;
        }
        if (strcmp(spxxLocal->slv, spxxRemote->slv) || strcmp(spxxLocal->sl, spxxRemote->sl) ||
            strcmp(spxxLocal->je, spxxRemote->je)) {
            _WriteLog(LL_FATAL, "ZYRedSimpleCompare dj/slv/sl/je not match, line:%d", i + 1);
            _WriteLog(LL_FATAL,
                      "ZYRedSimpleCompare spxxLocal->dj = %s, spxxRemote->dj = %s, line:%d",
                      spxxLocal->dj, spxxRemote->dj, i + 1);
            _WriteLog(LL_FATAL,
                      "ZYRedSimpleCompare spxxLocal->slv = %s, spxxRemote->slv = %s, line:%d",
                      spxxLocal->slv, spxxRemote->slv, i + 1);
            _WriteLog(LL_FATAL,
                      "ZYRedSimpleCompare spxxLocal->sl = %s, spxxRemote->sl = %s, line:%d",
                      spxxLocal->sl, spxxRemote->sl, i + 1);
            _WriteLog(LL_FATAL,
                      "ZYRedSimpleCompare spxxLocal->je = %s, spxxRemote->je = %s, line:%d",
                      spxxLocal->ggxh, spxxRemote->ggxh, i + 1);
            return -13;
        }
        if (atoi(spxxRemote->hsjbz) != 0) {
            _WriteLog(LL_FATAL, "ZYRedSimpleCompare hsjbz not match, line:%d", i + 1);
            _WriteLog(LL_FATAL, "ZYRedSimpleCompare ������Ϣ����Ĭ��ʹ�ò���˰���� line:%d",
                      spxxLocal->hsjbz, spxxRemote->hsjbz, i + 1);
            return -14;
        }
        if (strcmp(spxxLocal->spbh, spxxRemote->spbh)) {
            _WriteLog(LL_FATAL, "ZYRedSimpleCompare spbh not match, line:%d", i + 1);
            _WriteLog(LL_FATAL,
                      "ZYRedSimpleCompare spxxLocal->spbh = %s, spxxRemote->spbh = %s, line:%d",
                      spxxLocal->spbh, spxxRemote->spbh, i + 1);
            return -15;
        }
        if (strcmp(spxxLocal->yhsm, spxxRemote->yhsm)) {
            _WriteLog(LL_FATAL, "ZYRedSimpleCompare yhsm not match, line:%d", i + 1);
            _WriteLog(LL_FATAL,
                      "ZYRedSimpleCompare spxxLocal->yhsm = %s, spxxRemote->yhsm = %s, line:%d",
                      spxxLocal->yhsm, spxxRemote->yhsm, i + 1);
            return -16;
        }
        if (strcmp(spxxLocal->lslvbs, spxxRemote->lslvbs)) {
            _WriteLog(LL_FATAL, "ZYRedSimpleCompare lslbz not match, line:%d", i + 1);
            _WriteLog(LL_FATAL,
                      "ZYRedSimpleCompare spxxLocal->lslvbs = %s, spxxRemote->lslvbs = %s, line:%d",
                      spxxLocal->lslvbs, spxxRemote->lslvbs, i + 1);
            return -17;
        }
        if (!strcmp(spxxLocal->hsjbz, "1")) {
            char local_dj[48] = {0};
            PriceRemoveTax(spxxLocal->dj, spxxLocal->slv, 15, local_dj);
            if (strcmp(local_dj, spxxRemote->dj)) {
                _WriteLog(LL_FATAL, "ZYRedSimpleCompare dj/slv/sl/je not match, line:%d", i + 1);
                _WriteLog(LL_FATAL,
                          "ZYRedSimpleCompare spxxLocal->dj = %s, spxxRemote->dj = %s, line:%d",
                          local_dj, spxxRemote->dj, i + 1);
                return -18;
            }
        }
    }
    return 0;
}

int RedInvOnlineCheck_ZY(HFPXX fpxx, uint8 **inv_json)
{
    if ((fpxx->fplx_aisino != FPLX_AISINO_ZYFP) && ((fpxx->fplx_aisino != FPLX_AISINO_DZZP))) {
        _WriteLog(LL_FATAL, "Just query ZYFP");
        return -1;
    }
    if (!CheckTzdh(fpxx->redNum, "s")) {
        _WriteLog(LL_FATAL, "CheckTzdh1 failed");
        return -2;
    }
    //����ʱ������16��0��У��ʱ������16��0��
    if (!strcmp(fpxx->redNum, "0000000000000000")) {
        _WriteLog(LL_FATAL, "CheckTzdh2 failed");
        return -3;
    }
    if (!fpxx->hDev)
        return -4;
    int nRet = -10;
    int nChildRet = -1;
    int nBufSize = CalcAllocBuffLen(fpxx);
    char *szBuf = calloc(1, nBufSize);
    HFPXX fpxxRepMini = MallocFpxx();
    if (!szBuf || !fpxxRepMini)
        goto FreeAndExit;
    if (!LocalCheckCanbeRed(fpxx->hDev, fpxx->blueFpdm, fpxx->blueFphm, fpxx->dzsyh))
        goto FreeAndExit;
    if (DownloadTZDBH(fpxx->hDev, "", "", fpxx->redNum, szBuf, nBufSize) < 0)
        return -5;
    //_WriteHexToDebugFile("red-dl-rep.xml", (uint8 *)szBuf, strlen(szBuf));

    if ((fpxx->hDev->bDeviceType == DEVICE_AISINO) || (fpxx->hDev->bDeviceType == DEVICE_NISEC)) {
        if (GetFpxxFromRepByRedNum(szBuf, fpxx->redNum, fpxxRepMini) < 0) {
            _WriteLog(LL_FATAL, "GetFpxxFromRepByRedNum failed\r\n");
            nRet = -6;
            goto FreeAndExit;
        }
    } else if (fpxx->hDev->bDeviceType == DEVICE_CNTAX) {
        if (GetFpxxFromRepByRedNumCntax(szBuf, fpxx->redNum, fpxxRepMini) < 0) {
            _WriteLog(LL_FATAL, "GetFpxxFromRepByRedNumCntax failed\r\n");
            nRet = -6;
            goto FreeAndExit;
        }
    }
    if ((nChildRet = ZYRedSimpleCompare(fpxx, fpxxRepMini)) < 0) {
        _WriteLog(LL_FATAL, "ZYRedCompareMini failed,nRet:%d\r\n", nChildRet);
        nRet = -7;
        goto FreeAndExit;
    }

    fpxxRepMini->hDev = fpxx->hDev;
    fpxx_to_json_rednum_data_v102(fpxxRepMini, inv_json, 1);
    strcpy(fpxx->redNum_serial, fpxxRepMini->redNum_serial);
    nRet = 0;

FreeAndExit:
    if (fpxxRepMini)
        FreeFpxx(fpxxRepMini);
    if (szBuf)
        free(szBuf);

    return nRet;
}
//-------------------------------��Ʊ�������� finish----------------------------------------