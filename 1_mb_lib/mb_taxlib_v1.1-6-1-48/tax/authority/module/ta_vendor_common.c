/*****************************************************************************
File name:   ta_vendor_common.c
Description: ����˰�ֶ�aisino\nisec\cntax����ҵ���߼�����
             ���緢Ʊ�Ļ�ȡ\ȷ��\�ַ�;���ӹ�������ƽ̨����������;�����忨;���,
             ��Ʊ�ϴ���ز��֣�����ɣ������ַ�Ʊ���볷����ѯ��
Author:      Zako
Version:     1.0
Date:        2020.12
History:
20201218     �������ʵ��
20210808     ���ִ���ϲ�
*****************************************************************************/
#include "ta_vendor_common.h"

//---------------------------��������ƽ̨���д��� begin------------------------------------
//����<0����=0�������,>0 ��Ȩ��Ϣb64����
// bEnableOrDisable == true ���룻bEnableOrDisable == false ������
int SwitchPubservice(HDEV hDev, bool bEnableOrDisable, char *szOutSQXX)
{
    //"<?xml version=\"1.0\" encoding=\"gbk\"?><business comment=\"���ӷ�Ʊ����ƽ̨����\"
    // id=\"GGFW_DZFPFWPTSQ\"
    // jkbbh=\"1.0\"><body><tyxx><nsrsbh>92320105MA1WMUAP8T</nsrsbh><sbbh>661814461322</sbbh><sblx>10</sblx><rjlx
    ///><rjxh
    ///></tyxx><ywxx><fplxdm>026</fplxdm><czlx>1</czlx><ptlx>01</ptlx><ptxh>100000000000</ptxh><hssh>3201003440893140</hssh><kpjh>0</kpjh></ywxx></body></business>"
    char szOutBuf[2048] = "", szSblx[8] = "", szHssh[24] = "";
    int nOpType = 0, nRet = -2;

    if ((bEnableOrDisable && strlen(hDev->szPubServer)) ||
        (!bEnableOrDisable && !strlen(hDev->szPubServer)))
        return 0;  // nothing to do...

    if (DEVICE_AISINO == hDev->bDeviceType) {
        strcpy(szSblx, "10");
        nOpType = TACMD_AISINO_PUBSERV_JOINCANCEL;
        GetHashTaxCode(szHssh, hDev->sz9ByteHashTaxID);
        sprintf(
            szOutBuf,
            "<?xml version=\"1.0\" encoding=\"gbk\"?><business comment=\"���ӷ�Ʊ����ƽ̨����\" "
            "id=\"GGFW_DZFPFWPTSQ\" "
            "jkbbh=\"1.0\"><body><tyxx><nsrsbh>%s</nsrsbh><sbbh>%s</"
            "sbbh><sblx>%s</sblx><rjlx /><rjxh "
            "/></tyxx><ywxx><fplxdm>026</fplxdm><czlx>%s</czlx><ptlx>01</ptlx><ptxh>100000000000</"
            "ptxh><hssh>%s</hssh><kpjh>%d</kpjh></ywxx></body></business>",
            hDev->szCommonTaxID, hDev->szDeviceID, szSblx, bEnableOrDisable ? "1" : "2", szHssh,
            hDev->uICCardNo);
    } else if (DEVICE_NISEC == hDev->bDeviceType) {  //�˴��²����
        sprintf(szSblx, "00");
        nOpType = TACMD_NISEC_PUBSERV_JOINCANCEL;
        sprintf(szOutBuf,
                "<?xml version=\"1.0\" encoding=\"gbk\"?><business id=\"GGFW_DZFPFWPTSQ\" "
                "jkbbh=\"1.0\" comment=\"���ӷ�Ʊ����ƽ̨����\">"
                "<body><tyxx><nsrsbh>%s</nsrsbh><sbbh>%s</sbbh><sblx>%s</sblx><rjlx>01</"
                "rjlx><rjxh>100000000000</rjxh></tyxx><ywxx><fplxdm>026</fplxdm>"
                "<czlx>%s</czlx><ptlx>01</ptlx><ptxh>100000000000</ptxh></ywxx></body></business>",
                hDev->szCommonTaxID, hDev->szDeviceID, szSblx, bEnableOrDisable ? "1" : "2");
    } else {
        logout(INFO, "TAXLIB", "���빫������ƽ̨", "˰�����Ͳ�֧��\r\n");
        return -1;
    }

    nRet = TaNetIoOnce(hDev, nOpType, szOutBuf, sizeof(szOutBuf));
    if (!nRet) {
        nRet = strlen(szOutBuf);
        strcpy(szOutSQXX, szOutBuf);
    } else {
        logout(INFO, "TAXLIB", "���빫������ƽ̨", "������ͨѶʧ��,nRet = %d,������Ϣ��%s��\r\n",
               nRet, szOutBuf);
    }
    return nRet;
}
//--------------------------��������ƽ̨���д��� Finish---------------------------------

//--------------------------�����忨���ֹ��д��� begin----------------------------------
// bReportOrClear==falseʱ�� bDevLocked,bRepReached,
// nLockedDay��szHZSJHexStr_OutPutBuffer������ֵ������
int ZZSXML_ReportClear(HDEV hDev, bool bReportOrClear, bool bDevLocked, bool bRepReached,
                       int nLockedDay, char *szHZSJHexStr_OutPutBuffer)
{
    // aisino����˰��ȡ������16�����ַ�����nisec�����Լ��齨��XML Base64
    // aisino
    //<?xml version="1.0" encoding="GBK"?><FPXT><INPUT
    ///><NSRSBH>92320115MA1TAWE49D</NSRSBH><KPJH>0</KPJH><SBBH>661917845089</SBBH><HASH>3201007445689390</HASH>
    //<ISLOCK>N</ISLOCK><SSQ>17</SSQ><ZQBZ>N</ZQBZ><HZXX>971D1...</HZXX><CSSJ>202002010000</CSSJ></FPXT>
    // nisec
    //<?xml version="1.0" encoding="GBK"?><FPXT><INPUT
    ///><NSRSBH>913201067838061494</NSRSBH><KPJH>0</KPJH><SBBH>539904612711</SBBH>
    //<HASH>913201067838061494</HASH><ISLOCK>N</ISLOCK><ZQBZ>Y</ZQBZ><HZXX>PD...Cg==</HZXX><CSSJ>202010010000</CSSJ></FPXT>
    int nRet = -1;
    char szBuf[256] = "";
    mxml_node_t *xml_Node = NULL;
    mxml_node_t *root = mxmlNewXML("1.0");
    if (!root)
        goto Finish;
    xml_Node = mxmlNewElement(root, "FPXT");
    // aisino��INPUT�ǿհ�
    if (DEVICE_AISINO == hDev->bDeviceType)
        mxmlNewElement(xml_Node, "INPUT");
    else  // DEVICE_NISEC��nisec��INPUT�����ڵ�
        xml_Node = mxmlNewElement(xml_Node, "INPUT");

    mxmlNewText(mxmlNewElement(xml_Node, "NSRSBH"), 0, hDev->szCommonTaxID);
    sprintf(szBuf, "%d", hDev->uICCardNo);
    mxmlNewText(mxmlNewElement(xml_Node, "KPJH"), 0, szBuf);
    mxmlNewText(mxmlNewElement(xml_Node, "SBBH"), 0, hDev->szDeviceID);
    if (DEVICE_AISINO == hDev->bDeviceType) {
        GetHashTaxCode(szBuf, hDev->sz9ByteHashTaxID);
        mxmlNewText(mxmlNewElement(xml_Node, "HASH"), 0, szBuf);
    } else {  // DEVICE_NISEC
        mxmlNewText(mxmlNewElement(xml_Node, "HASH"), 0, hDev->szCommonTaxID);
    }
    if (bReportOrClear) {
        mxmlNewText(mxmlNewElement(xml_Node, "ISLOCK"), 0, bDevLocked ? "Y" : "N");
        if (DEVICE_AISINO == hDev->bDeviceType) {
            sprintf(szBuf, "%d", nLockedDay);
            mxmlNewText(mxmlNewElement(xml_Node, "SSQ"), 0, szBuf);
        }
        mxmlNewText(mxmlNewElement(xml_Node, "ZQBZ"), 0, bRepReached ? "Y" : "N");
        mxmlNewText(mxmlNewElement(xml_Node, "HZXX"), 0, szHZSJHexStr_OutPutBuffer);
    }
    GetBJSJ(hDev->szDeviceTime, szBuf, 2);
    mxmlNewText(mxmlNewElement(xml_Node, "CSSJ"), 0, szBuf);
    nRet = TAXML_Formater(root, szHZSJHexStr_OutPutBuffer);
    //_WriteHexToDebugFile("zzs_report.xml", (uint8 *)szHZSJHexStr_OutPutBuffer, nRet);

Finish:
    mxmlDelete(root);
    return nRet;
}
//---------------------------�����忨���ֹ��д��� finish------------------------------

//----------------------���ַ�Ʊ���볷����ѯ���д��� begin-----------------------------
void InsertRedSPXX(struct Spxx *spxx, mxml_node_t *xnode_spxx)
{
    mxml_node_t *xml_Node = NULL;
    char szBuf[256] = "";
    // GoodsName
    sprintf(szBuf, "%s%s", spxx->spsmmc, spxx->spmc);
    xml_Node = mxmlNewElement(xnode_spxx, "GoodsName");
    mxmlNewText(xml_Node, 0, szBuf);
    // GoodsUnit
    xml_Node = mxmlNewElement(xnode_spxx, "GoodsUnit");
    mxmlNewText(xml_Node, 0, spxx->jldw);
    // GoodsPrice
    xml_Node = mxmlNewElement(xnode_spxx, "GoodsPrice");
    memset(szBuf, 0, sizeof szBuf);
    if (!strcmp(spxx->hsjbz, "1")) {
        PriceRemoveTax(spxx->dj, spxx->slv, 15, szBuf);
        mxmlNewText(xml_Node, 0, szBuf);
    } else
        mxmlNewText(xml_Node, 0, spxx->dj);
    // GoodsTaxRate
    xml_Node = mxmlNewElement(xnode_spxx, "GoodsTaxRate");
    mxmlNewText(xml_Node, 0, spxx->slv);
    // GoodsGgxh
    xml_Node = mxmlNewElement(xnode_spxx, "GoodsGgxh");
    mxmlNewText(xml_Node, 0, spxx->ggxh);
    // GoodsNum
    xml_Node = mxmlNewElement(xnode_spxx, "GoodsNum");
    mxmlNewText(xml_Node, 0, spxx->sl);
    // GoodsJE
    xml_Node = mxmlNewElement(xnode_spxx, "GoodsJE");
    mxmlNewText(xml_Node, 0, spxx->je);
    // GoodsSE
    xml_Node = mxmlNewElement(xnode_spxx, "GoodsSE");
    mxmlNewText(xml_Node, 0, spxx->se);
    // HS_BZ;ֻ֧�ֲ���˰
    xml_Node = mxmlNewElement(xnode_spxx, "HS_BZ");
    mxmlNewText(xml_Node, 0, "N");
    // SPBM
    xml_Node = mxmlNewElement(xnode_spxx, "SPBM");
    mxmlNewText(xml_Node, 0, spxx->spbh);
    // QYSPBM
    xml_Node = mxmlNewElement(xnode_spxx, "QYSPBM");
    mxmlNewText(xml_Node, 0, spxx->zxbh);
    // SYYHZCBZ;�Ż����߱�ʶ,�Ƿ�ʹ���Ż����߱�ʶ0����ʹ�ã�1��ʹ��
    xml_Node = mxmlNewElement(xnode_spxx, "SYYHZCBZ");
    if (strlen(spxx->yhsm) > 0)
        mxmlNewText(xml_Node, 0, "1");
    else
        mxmlNewText(xml_Node, 0, "0");
    // YHZC;�Ż���������
    xml_Node = mxmlNewElement(xnode_spxx, "YHZC");
    mxmlNewText(xml_Node, 0, spxx->yhsm);
    // LSLBZ; ��˰�ʱ�ʶ,�գ�����˰�ʣ�0��������˰��1����˰��2�������գ�3��ͨ��˰��
    xml_Node = mxmlNewElement(xnode_spxx, "LSLBZ");
    mxmlNewText(xml_Node, 0, spxx->lslvbs);
}

// Aisino.Fwkp.Hzfp.Form.SqdChaXun.CreateSingleTableNodeInfo
int BuildUploadFormXML(HFPXX fpxx, char **szOut)
{
    char szBuf[256] = "";
    mxml_node_t *xml_root = NULL, *xml_Node = NULL;
    mxml_node_t *root = mxmlNewXML("1.0");
    if (!root)
        return -1;
    if (!(xml_root = mxmlNewElement(root, "FPXT")))
        goto Finish;
    if (!(xml_root = mxmlNewElement(xml_root, "INPUT")))
        goto Finish;
    if (DEVICE_NISEC == fpxx->hDev->bDeviceType)
        mxmlNewText(mxmlNewElement(xml_root, "Version"), 0, "2.0");
    if (!(xml_root = mxmlNewElement(xml_root, "RedInvReqBill")))
        goto Finish;
    // ReqNsrsbh(�Ե�ǰ�豸����˰��Ϊ׼)
    xml_Node = mxmlNewElement(xml_root, "ReqNsrsbh");
    mxmlNewText(xml_Node, 0, fpxx->hDev->szCommonTaxID);
    // Kpjh
    xml_Node = mxmlNewElement(xml_root, "Kpjh");
    memset(szBuf, 0, sizeof(szBuf));
    sprintf(szBuf, "%d", fpxx->kpjh);
    mxmlNewText(xml_Node, 0, szBuf);
    // Sbbh
    xml_Node = mxmlNewElement(xml_root, "Sbbh");
    mxmlNewText(xml_Node, 0, fpxx->hDev->szDeviceID);
    // ReqBillNo; ���뵥�� SN+yyMMddHHmmss
    memset(szBuf, 0, sizeof(szBuf));
    sprintf(szBuf, "%s%s", fpxx->hDev->szDeviceID, fpxx->hDev->szDeviceTime + 2);
    xml_Node = mxmlNewElement(xml_root, "ReqBillNo");
    mxmlNewText(xml_Node, 0, szBuf);
    // BillType; ��Ϣ������ 0��������1������
    xml_Node = mxmlNewElement(xml_root, "BillType");
    mxmlNewText(xml_Node, 0, "0");
    //�����͹���δ�ֿ���Ҫ���뷢Ʊ������뼴��Ʊ����
    if (fpxx->hzxxbsqsm != 1) {
        // bluefpdm bluefpgm
        xml_Node = mxmlNewElement(xml_root, "TypeCode");
        mxmlNewText(xml_Node, 0, fpxx->blueFpdm);
        xml_Node = mxmlNewElement(xml_root, "InvNo");
        mxmlNewText(xml_Node, 0, fpxx->blueFphm);
        // kprq ssyf
        xml_Node = mxmlNewElement(xml_root, "Kprq");
        mxmlNewText(xml_Node, 0, fpxx->ssyf);
    } else {                                   //�ѵֿ�����
        mxmlNewElement(xml_root, "TypeCode");  // bluefpdm bluefpgm
        mxmlNewElement(xml_root, "InvNo");
        //����kprq
    }
    // Szlb; ˰����� 0��Ӫҵ˰��1����ֵ˰��Ĭ�ϣ�
    xml_Node = mxmlNewElement(xml_root, "Szlb");
    mxmlNewText(xml_Node, 0, "1");
    // IsMutiRate
    xml_Node = mxmlNewElement(xml_root, "IsMutiRate");
    mxmlNewText(xml_Node, 0, fpxx->isMultiTax ? "1" : "0");
    // Date
    memset(szBuf, 0, sizeof(szBuf));
    xml_Node = mxmlNewElement(xml_root, "Date");
    TranslateStandTime(2, fpxx->kpsj_standard, szBuf);
    szBuf[10] = '\0';
    mxmlNewText(xml_Node, 0, szBuf);
    // BuyerName;
    xml_Node = mxmlNewElement(xml_root, "BuyerName");
    mxmlNewText(xml_Node, 0, fpxx->gfmc);
    // BuyTaxCode;
    xml_Node = mxmlNewElement(xml_root, "BuyTaxCode");
    mxmlNewText(xml_Node, 0, fpxx->gfsh);
    // SellerName;
    xml_Node = mxmlNewElement(xml_root, "SellerName");
    mxmlNewText(xml_Node, 0, fpxx->xfmc);
    // SellTaxCode;
    xml_Node = mxmlNewElement(xml_root, "SellTaxCode");
    mxmlNewText(xml_Node, 0, fpxx->xfsh);
    // Amount
    xml_Node = mxmlNewElement(xml_root, "Amount");
    mxmlNewText(xml_Node, 0, fpxx->je);
    // TaxRate,nisec�в�д��˰�ʣ���ʱ��д��
    if (!fpxx->isMultiTax) {
        xml_Node = mxmlNewElement(xml_root, "TaxRate");
        mxmlNewText(xml_Node, 0, fpxx->slv);
    } else {
        xml_Node = mxmlNewElement(xml_root, "TaxRate");
        mxmlNewText(xml_Node, 0, "");
    }
    // Tax
    xml_Node = mxmlNewElement(xml_root, "Tax");
    mxmlNewText(xml_Node, 0, fpxx->se);
    // ReqMemo;����˵�� 11λ;
    //���������ѵֿ�:11000000000 ��������δ�ֿ�:10100000000
    //��������:00000001000
    if (fpxx->hzxxbsqsm == 0) {
        mxmlNewText(mxmlNewElement(xml_root, "ReqMemo"), 0, "0000000100");
    } else if (fpxx->hzxxbsqsm == 1) {
        mxmlNewText(mxmlNewElement(xml_root, "ReqMemo"), 0, "1100000000");
    } else if (fpxx->hzxxbsqsm == 2) {
        mxmlNewText(mxmlNewElement(xml_root, "ReqMemo"), 0, "1010000000");
    }
    // SPBMBBH
    xml_Node = mxmlNewElement(xml_root, "SPBMBBH");
    mxmlNewText(xml_Node, 0, fpxx->bmbbbh);
    // SLBZ;0��������Ʊ  1����������  2���������
    xml_Node = mxmlNewElement(xml_root, "SLBZ");
    if (!strcmp(fpxx->slv, "0.015") ||
        (fpxx->yysbz[8] == '0' && (!strcmp(fpxx->slv, "0.05") || !strcmp(fpxx->slv, "0.050"))))
        mxmlNewText(xml_Node, 0, "1");
    else if (ZYFP_CEZS == fpxx->zyfpLx)
        mxmlNewText(xml_Node, 0, "2");
    else
        mxmlNewText(xml_Node, 0, "0");
    // refresh root node
    if (!(xml_root = mxmlNewElement(xml_root, "RedInvReqBillMx")))
        goto Finish;

    if (strcmp(fpxx->qdbj, "Y") == 0) {
        xml_Node = mxmlNewElement(xml_root, "GoodsMx");
        InsertRedSPXX(fpxx->stp_MxxxHead, xml_Node);
    } else {
        struct Spxx *stp_Spxx = fpxx->stp_MxxxHead->stp_next;
        while (stp_Spxx) {
            xml_Node = mxmlNewElement(xml_root, "GoodsMx");
            InsertRedSPXX(stp_Spxx, xml_Node);
            stp_Spxx = stp_Spxx->stp_next;
        }
    }
    *szOut = mxmlSaveAllocString(root, MXML_NO_CALLBACK);

Finish:
    mxmlDelete(root);
    return 0;
}

int RedFormVerify(HFPXX fpxx)
{
    int nSpxxNum = 0;
    struct Spxx *stp_Spxx = fpxx->stp_MxxxHead;
    if (!stp_Spxx->stp_next) {
        _WriteLog(LL_FATAL, "RedFormVerify spxx cannot null");
        return -1;
    }
    if (stp_Spxx) {
        while (stp_Spxx && (nSpxxNum < 2000)) {
            stp_Spxx = stp_Spxx->stp_next;
            nSpxxNum++;
        }
    }
    //���ֱ����� �������ѵֿ�����ssyf����������Ҫ
    if (fpxx->hzxxbsqsm != 1) {
        if (strlen(fpxx->ssyf) != 6) {
            _WriteLog(LL_FATAL, "Muset fill ssyf");
            return -3;
        }
    }
    return 0;
}

// szOutTZDBH�ں�������0��ִ����ȷ����ʱ�����ֵ������ʱ�������ݼ�szLastError
// aisino��nisec��ͬʹ��,����ֵΪ|������ĸ��ַ���
// ��Ʊ������ˮ��|��Ʊ֪ͨ�����|��Ʊ����״̬����|��Ʊ����״̬����
int UploadTZDBH(HFPXX fpxx, char *szOutTZDBH)
{
    if (RedFormVerify(fpxx) < 0) {
        SetLastError(fpxx->hDev->hUSB, ERR_BUF_CHECK, "������Ϣ������������֤ʧ��");
        return -1;
    }
    char *szOut = NULL;
    // _WriteHexToDebugFile("red.xml", (uint8 *)szOut, strlen(szOut));
    int nOpType = 0;
    if (DEVICE_AISINO == fpxx->hDev->bDeviceType)
        nOpType = TACMD_AISINO_REDFORM_UPLOAD_ZZS;
    else if (DEVICE_NISEC == fpxx->hDev->bDeviceType)
        nOpType = TACMD_NISEC_REDFORM_UPLOAD_ZZS;
    else if (DEVICE_CNTAX == fpxx->hDev->bDeviceType)
        nOpType = TACMD_CNTAX_REDFORM_UPLOAD;
    else
        return -2;
    if (nOpType == TACMD_AISINO_REDFORM_UPLOAD_ZZS || nOpType == TACMD_NISEC_REDFORM_UPLOAD_ZZS) {
        if (BuildUploadFormXML(fpxx, &szOut) < 0) {
            SetLastError(fpxx->hDev->hUSB, ERR_BUF_CHECK, "���������Ϣ���������ʧ��");
            return -3;
        }
    } else {  // TACMD_CNTAX_REDFORM_UPLOAD ueky
        if (BuildCntaxUploadRedFormXML(fpxx, &szOut) < 0) {
            SetLastError(fpxx->hDev->hUSB, ERR_BUF_CHECK, "���������Ϣ���������ʧ��");
            return -4;
        }
    }
    _WriteHexToDebugFile("redform_req.xml", (uint8 *)szOut, strlen(szOut));
    int nRet = TaNetIoOnce(fpxx->hDev, nOpType, szOut, strlen(szOut));
    if (nRet < 0) {
        logout(INFO, "TAXLIB", "������Ϣ���ϴ�", "������ͨѶʧ��,nRet = %d,������Ϣ��%s��\r\n",
               nRet, szOut);
        free(szOut);
        return nRet;
    }
    strcpy(szOutTZDBH, szOut);
    free(szOut);
    return 0;
}

int RedinvTZDCancel(HDEV hDev, char *szTZDBH, char *szOutMsg, int nOutMsgBuffSize)
{
    char szSblx[8] = "";
    int nRet = 0;
    int nOpType = 0;
    if (!CheckTzdh(szTZDBH, "s")) {
        _WriteLog(LL_FATAL, "CheckTzdh failed");
        SetLastError(hDev->hUSB, ERR_BUF_CHECK, "����ĺ�����Ϣ���Ÿ�ʽ����");
        return -2;
    }
    if (DEVICE_AISINO == hDev->bDeviceType) {
        strcpy(szSblx, "10");
        nOpType = TACMD_AISINO_TZDBH_CANCEL;
        sprintf(szOutMsg,
                "<?xml version=\"1.0\" encoding=\"gbk\"?><business comment=\"������Ϣ����\" "
                "id=\"GGFW_HZXXBCX\" jkbbh=\"1.0\"><body><tyxx><nsrsbh>%s</nsrsbh><sbbh>%s</"
                "sbbh><sblx>%s</sblx><rjlx /><rjxh "
                "/></tyxx><ywxx><hzxxbbh>%s</hzxxbbh></ywxx></body></business>",
                hDev->szCommonTaxID, hDev->szDeviceID, szSblx, szTZDBH);
    } else if (DEVICE_NISEC == hDev->bDeviceType) {  //�˴��²����
        strcpy(szSblx, "00");
        nOpType = TACMD_NISEC_TZDBH_CANCEL;
        sprintf(szOutMsg,
                "<?xml version=\"1.0\" encoding=\"gbk\"?><business id=\"GGFW_HZXXBCX\" "
                "comment=\"������Ϣ����\" jkbbh=\"1.0\">"
                "<body><tyxx><nsrsbh>%s</nsrsbh><sbbh>%s</sbbh><sblx>%s</sblx><rjlx>01</"
                "rjlx><rjxh>100000000000</rjxh></tyxx><ywxx><hzxxbbh>%s</hzxxbbh></ywxx></body></"
                "business>",
                hDev->szCommonTaxID, hDev->szDeviceID, szSblx, szTZDBH);
    } else if (DEVICE_CNTAX == hDev->bDeviceType) {  //�˴��²����
        nOpType = TACMD_CNTAX_REDFORM_CANCEL;
        sprintf(szOutMsg, "<ywxx><hzxxbbh>%s</hzxxbbh></ywxx>", szTZDBH);
    } else
        return -1;
    nRet = TaNetIoOnce(hDev, nOpType, szOutMsg, nOutMsgBuffSize);
    _WriteLog(LL_INFO, "RedinvTZDCancel:%s TZDBH:%s nRet:%d retmsg:%s", nRet == 0 ? "OK" : "Fail",
              szTZDBH, nRet, szOutMsg);
    // printf("szOutMsg:%s\n", szOutMsg);
    return nRet;
}

//ֻ�ṩ������Ҫ�����ӿ�,������������Ҫ,ִ�гɹ�szOut�ڴ�����߻���
// szQueryDateRange=20200401-20200501;szGfsh szXXBBH��ѡ
int BuildDownloadFormXML(HDEV hDev, char *szQueryDateRange, char *szGfsh, char *szXXBBH,
                         char **szOut)
{
    if (strlen(szQueryDateRange) > 0 &&
        (17 != strlen(szQueryDateRange) || !strchr(szQueryDateRange, '-') ||
         '-' != szQueryDateRange[8])) {
        _WriteLog(LL_FATAL, "QueryDateRange format incorrect, example: 20200401-20200501");
        return -1;
    }
    /*if (!szGfsh || !szXXBBH) {
        _WriteLog(LL_FATAL, "Parament mustbe '', canot be NULL");
        return -2;
    }*/
    //strcpy(szGfsh,"91320191MA1N8ERL56");

    //printf("11111111111111111111111111111111111111111111szGfsh=%s\r\n",szGfsh);
    
    char szBuf[256] = "";
    char szBuf2[256] = "";
    mxml_node_t *xml_root = NULL, *xml_Node = NULL;
    mxml_node_t *root = mxmlNewXML("1.0");
    if (!root)
        return -3;
    int nRet = -4;
    if (!(xml_root = mxmlNewElement(root, "FPXT")))
        goto Finish;
    if (!(xml_root = mxmlNewElement(xml_root, "INPUT")))
        goto Finish;
    if (DEVICE_NISEC == hDev->bDeviceType)
        mxmlNewText(mxmlNewElement(xml_root, "Version"), 0, "2.0");
    // NSRSBH
    xml_Node = mxmlNewElement(xml_root, "NSRSBH");
    mxmlNewText(xml_Node, 0, hDev->szCommonTaxID);
    // SBBH
    xml_Node = mxmlNewElement(xml_root, "SBBH");
    mxmlNewText(xml_Node, 0, hDev->szDeviceID);
    // KPJH
    sprintf(szBuf, "%d", hDev->uICCardNo);
    xml_Node = mxmlNewElement(xml_root, "KPJH");
    mxmlNewText(xml_Node, 0, szBuf);
    // YQZT
    xml_Node = mxmlNewElement(xml_root, "YQZT");
    mxmlNewText(xml_Node, 0, "N");
    if (strlen(szQueryDateRange)) {
        // TKRQ_Q
        strcpy(szBuf, szQueryDateRange);
        szBuf[8] = '\0';
        if (DEVICE_AISINO == hDev->bDeviceType) {
            strcat(szBuf, "000000");
            TranslateStandTime(5, szBuf, szBuf2);
        } else  // NISEC
            strcpy(szBuf2, szBuf);
        xml_Node = mxmlNewElement(xml_root, "TKRQ_Q");
        mxmlNewText(xml_Node, 0, szBuf2);
        // TKRQ_Z
        strcpy(szBuf, szQueryDateRange + 9);
        if (DEVICE_AISINO == hDev->bDeviceType) {
            strcat(szBuf, "000000");
            TranslateStandTime(5, szBuf, szBuf2);
        } else  // nisec
            strcpy(szBuf2, szBuf);
        xml_Node = mxmlNewElement(xml_root, "TKRQ_Z");
        mxmlNewText(xml_Node, 0, szBuf2);
    } else {
        xml_Node = mxmlNewElement(xml_root, "TKRQ_Q");
        xml_Node = mxmlNewElement(xml_root, "TKRQ_Z");
    }
    // GFSH
    xml_Node = mxmlNewElement(xml_root, "GFSH");
    mxmlNewText(xml_Node, 0, szGfsh);
    // XFSH
    xml_Node = mxmlNewElement(xml_root, "XFSH");
    mxmlNewText(xml_Node, 0, "");
    // XXBBH ���ַ�Ʊ��Ϣ����
    xml_Node = mxmlNewElement(xml_root, "XXBBH");
    mxmlNewText(xml_Node, 0, szXXBBH);
    // XXBFW ��Ϣ��Χ��0 ����  1����ҵ����  2 ����ҵ���գ�
    xml_Node = mxmlNewElement(xml_root, "XXBFW");
    mxmlNewText(xml_Node, 0, "0");
    // PAGENO,ֻ��ѯ��һҳ,���1000��
    xml_Node = mxmlNewElement(xml_root, "PAGENO");
    mxmlNewText(xml_Node, 0, "1");
    // PAGESIZE
    xml_Node = mxmlNewElement(xml_root, "PAGESIZE");
    mxmlNewText(xml_Node, 0, "1000");

    *szOut = mxmlSaveAllocString(root, MXML_NO_CALLBACK);
    nRet = 0;

Finish:
    mxmlDelete(root);
    return nRet;
}

// szXXBBH��ѯ������Ψһ�ģ������ķ��ؿ����Ƕ���
int DownloadTZDBH(HDEV hDev, char *szQueryDateRange, char *szGfsh, char *szXXBBH, char *szRetBuf,
                  int nBufSize)
{
    int nOpType = 0;
    if (DEVICE_AISINO == hDev->bDeviceType)
        nOpType = TACMD_AISINO_REDFORM_DOWNLOAD_ZZS;
    else if (DEVICE_NISEC == hDev->bDeviceType)
        nOpType = TACMD_NISEC_REDFORM_DOWNLOAD_ZZS;
    else if (DEVICE_CNTAX == hDev->bDeviceType)
        nOpType = TACMD_CNTAX_REDFORM_DOWNLOAD;
    else
        return -1;
    char *szOut = NULL;
    if (nOpType == TACMD_AISINO_REDFORM_DOWNLOAD_ZZS ||
        nOpType == TACMD_NISEC_REDFORM_DOWNLOAD_ZZS) {
        if (BuildDownloadFormXML(hDev, szQueryDateRange, szGfsh, szXXBBH, &szOut) < 0) {
            SetLastError(hDev->hUSB, ERR_BUF_CHECK, "��ѯ���غ�����Ϣ���������ʧ��");
            return -2;
        }
    } else {  // cntax
        if (CntaxBuildDownloadFormXML(hDev, szQueryDateRange, szGfsh, szXXBBH, &szOut) < 0) {
            SetLastError(hDev->hUSB, ERR_BUF_CHECK, "��ѯ���غ�����Ϣ���������ʧ��");
            return -3;
        }
    }
    //_WriteHexToDebugFile("red.xml", (uint8 *)szOut, strlen(szOut));
    strcpy(szRetBuf, szOut);
    free(szOut);
    int nRet = TaNetIoOnce(hDev, nOpType, szRetBuf, nBufSize);
    if (nRet < 0) {
        _WriteLog(LL_FATAL, "DownloadTZDBH TaNetIoOnce failed! Error:%s",
                  hDev->hUSB->szLastErrorDescription);
        return -3;
    }
    // not match;��һ����aisino��nisec���ڶ���cntax
    if (strstr(szRetBuf, "<ALLCOUNT>0</ALLCOUNT>") || strstr(szRetBuf, "<fhtjjls>0</fhtjjls>")) {
        _WriteLog(LL_WARN, "DownloadTZDBH no match result!");
        SetLastError(hDev->hUSB, ERR_LOGIC_NOTFOUND,
                     "û�����������Ĵ���������");  //�ô������������޸�,ty_usb_m����ʹ��
        return -4;
    }
    return 0;
}
//----------------------���ַ�Ʊ���볷����ѯ���д��� finish-----------------------------
//---------------------------��Ʊ�ϴ����д��� begin-----------------------------------
// DZFP�ϴ������һ��
int AddDZFPXMLPackage(char *szDataPayload, bool bPayloadNeedBase64, HDEV hDev, char *pOutBuf)
{
    char szBuf[256] = "";
    if (!hDev || !pOutBuf)
        return -1;
    int nRet = -3;
    int nB64BufLen = strlen(szDataPayload) * 2 + 512;
    void *pBase64Buff = calloc(1, nB64BufLen);
    if (!pBase64Buff)
        return -2;
    if (bPayloadNeedBase64) {
        if (UTF8ToGBKBase64(szDataPayload, strlen(szDataPayload), pBase64Buff) < 0)
            goto Finish;
    } else {
        U2G_GBKIgnore(szDataPayload, strlen(szDataPayload), pBase64Buff, nB64BufLen);
    }
    mxml_node_t *xml_Node = NULL, *xml_Node2 = NULL;
    mxml_node_t *root = mxmlNewXML("1.0");
    if (!root)
        goto Finish;

    xml_Node = mxmlNewElement(root, "business");
    if (!xml_Node)
        goto Finish;
    if (DEVICE_AISINO == hDev->bDeviceType)
        mxmlElementSetAttr(xml_Node, "id", "HX_FPMXSC");
    else if (DEVICE_NISEC == hDev->bDeviceType)
        mxmlElementSetAttr(xml_Node, "id", "FPMXSC");
    else
        goto Finish;
    mxmlElementSetAttr(xml_Node, "comment", "��Ʊ��ϸ�ϴ�");
    mxmlElementSetAttr(xml_Node, "version", "1.0");
    xml_Node = mxmlNewElement(xml_Node, "body");
    if (!xml_Node)
        goto Finish;
    mxmlElementSetAttr(xml_Node, "count", "1");
    mxmlElementSetAttr(xml_Node, "skph", hDev->szDeviceID);
    mxmlElementSetAttr(xml_Node, "nsrsbh", hDev->szCommonTaxID);
    if (DEVICE_AISINO == hDev->bDeviceType)
        mxmlElementSetAttr(xml_Node, "ip", hDev->szClientWanIP);
    else {
        memset(szBuf, 0, sizeof(szBuf));
        sprintf(szBuf, "%s:%s", hDev->szClientWanIP, hDev->szClientWanPort);
        mxmlElementSetAttr(xml_Node, "ip", szBuf);
    }

    memset(szBuf, 0, sizeof(szBuf));

    if (DEVICE_AISINO == hDev->bDeviceType) {
        GetNicMAC(hDev->szCommonTaxID, szBuf, 0);
        mxmlElementSetAttr(xml_Node, "rjbbh", defTaxAuthorityVersionAisino);
    } else {
        GetNicMAC(hDev->szCommonTaxID, szBuf, 1);
        mxmlElementSetAttr(xml_Node, "rjbbh", defTaxAuthorityVersionNisec);
    }
    mxmlElementSetAttr(xml_Node, "mac", szBuf);
    mxmlElementSetAttr(xml_Node, "bmbbh", DEF_BMBBBH_DEFAULT);
    mxmlElementSetAttr(xml_Node, "bdbz", "1");
    if (DEVICE_AISINO == hDev->bDeviceType)
        mxmlElementSetAttr(xml_Node, "ydk", hDev->szClientWanPort);
    else
        mxmlElementSetAttr(xml_Node, "ydk", "0");
    memset(szBuf, 0, sizeof(szBuf));
    GetMotherBoardId(hDev->szCommonTaxID, szBuf);
    mxmlElementSetAttr(xml_Node, "zbxlh", szBuf);

    xml_Node = mxmlNewElement(xml_Node, "group");
    if (!xml_Node)
        goto Finish;
    mxmlElementSetAttr(xml_Node, "xh", "1");

    xml_Node2 = mxmlNewElement(xml_Node, "data");
    mxmlElementSetAttr(xml_Node2, "name", "fplx_dm");
    mxmlElementSetAttr(xml_Node2, "value", "026");

    xml_Node2 = mxmlNewElement(xml_Node, "data");
    mxmlElementSetAttr(xml_Node2, "name", "fpmx");
    mxmlElementSetAttr(xml_Node2, "value", pBase64Buff);

    //����͸�ʽ����
    char *ptr = mxmlSaveAllocString(root, MXML_NO_CALLBACK);
    char *pNew = StrReplace(ptr, "\n", " ");
    memcpy(pOutBuf, pNew, strlen(pNew));
    int nStrLen = strlen(pOutBuf);
    if (pOutBuf[nStrLen - 1] == ' ')
        pOutBuf[nStrLen - 1] = '\0';
    free(pNew);
    free(ptr);
    nRet = 0;

Finish:
    free(pBase64Buff);
    if (root)
        mxmlDelete(root);
    return nRet;
}
//---------------------------��Ʊ�ϴ����д��� finish-----------------------------------