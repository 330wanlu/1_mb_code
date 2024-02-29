/*****************************************************************************
File name:   nisec_uploadinvoice.c
Description: ���ڰ����̵ķ�Ʊ�ϴ�ҵ��
Author:      Zako
Version:     1.0
Date:        2020.05
History:
20200522     �������ʵ��
*****************************************************************************/
#include "nisec_uploadinvoice.h"

//������ӿ� ������Ʊ��Ϣxml
int NisecPreInvoiceXML(HFPXX fpxx, char *pOutXMLBuf)
{
    char szBuf[64] = "";
    int nRawFpLen = *(int *)fpxx->pRawFPBin;
    // str:SN(12)+FPLX+?(6)
    if (FPLX_COMMON_DZFP == fpxx->fplx) {
        memcpy(pOutXMLBuf, fpxx->pRawFPBin + 4, nRawFpLen);
    } else if (FPLX_COMMON_PTFP == fpxx->fplx || FPLX_COMMON_ZYFP == fpxx->fplx) {
        sprintf(szBuf, "%s%03d000", fpxx->hDev->szDeviceID, fpxx->fplx);
        strncpy((char *)pOutXMLBuf, szBuf, 18);
        memcpy(pOutXMLBuf + 18, fpxx->pRawFPBin + 4, nRawFpLen);
        _WriteHexToDebugFile("nisec_uploadinv_my.bin", (uint8 *)pOutXMLBuf, nRawFpLen + 18);
    } else
        return -1;
    return 0;
}

//��δ����Net_IOͳһ�ӿڣ�����ݲ�֧��LastError
//����ֵ����fpxx->szSLXLH��д��32λ�������кţ����ڿ���ֱ��ͨ�������кŲ�ѯ�ӿ�
int NisecUploadFPBySingle(HHTTP hi, HFPXX fpxx, char *pRepBuf, int nRepBufLen)
{
    int nRet = ERR_LOGIC_UPLOADINV_FAIL;
    int nOpType = 0;
    do {
        if (!strlen(fpxx->xfsh))
            break;
        if (fpxx->fplx == FPLX_COMMON_ZYFP || fpxx->fplx == FPLX_COMMON_PTFP)
            nOpType = TACMD_NISEC_UPFP_ZZS;
        else if (fpxx->fplx == FPLX_COMMON_DZFP)
            nOpType = TACMD_NISEC_UPFP_DZ;
        else
            break;
        if (NisecPreInvoiceXML(fpxx, pRepBuf) < 0) {
            nRet = ERR_LOGIC_UPLOADINV_BUILD_PREXML;
            break;
        }
        //���ӽӿ���Ҫ������һ��ZZS
        if (FPLX_COMMON_DZFP == fpxx->fplx &&
            AddDZFPXMLPackage(pRepBuf, false, fpxx->hDev, pRepBuf) < 0) {
            nRet = ERR_TA_DZFPXML_PACKAGE;
            break;
        }
        if (TaNetIo(hi, nOpType, pRepBuf, nRepBufLen) < 0) {
            nRet = ERR_IO_FAIL;
            break;
        }
        if (FPLX_COMMON_DZFP == fpxx->fplx && !strcmp(pRepBuf, "00")) {
            nRet = RET_SUCCESS;  //��Ʊ2 "00"
        } else if (strlen(pRepBuf) < 40 && strlen(pRepBuf) > 32 && strchr(pRepBuf, '-')) {
            nRet = RET_SUCCESS;  // ר��Ʊ����36 38b28411-3f81-4fbc-90bb-822e141e4440
        } else
            break;
    } while (false);
    return nRet;
}


////��ѯ�Ƿ������ϴ��ϴ��ķ�Ʊ
//<0�����账������ֵ==0,������Ҫ�ϱ���Ʊ�������ϱ�ʧ�ܣ�>0 ����һ�ţ��ϱ����
// szCurFPDM szCurFPHM ret < 0�п���Ϊ��
int QueryModifyInvoice(uint8 bInvType, HDEV hDev, char *szCurFPDM, char *szCurFPHM)
{
    HUSB hUsb = hDev->hUSB;
    uint8 cb[256];
    char szFPDM[64], szFPHM[64];
    int nChildRet = 0, i = 0, nRet = -1;
    // 20220729ʵ�� 84 88��������Ʊ��ѯ�ӿ�
    // ��Query-ModifyInvoice�ӿڲ��ɺ�cntaxһ����Get-InvBuf�ӿںϲ�
    //\xfe\x2f\x84 ??
    //\xfe\x2f\x86 ����δ�ϴ�
    //\xfe\x2f\x88 ??
    while (1) {
        // query buf
        if (NisecEntry(hUsb, NOP_OPEN_FULL) < 0)
            break;
        if (NisecSelectInvoiceTypeMini(hUsb, bInvType) < 0) {
            NisecEntry(hDev->hUSB, NOP_CLOSE);
            break;
        }
        memcpy(cb, "\xfe\x2f\x86\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01", 16);
        nChildRet = NisecLogicIO(hUsb, cb, 16, sizeof(cb), NULL, 0);
        NisecEntry(hDev->hUSB, NOP_CLOSE);
        if (nChildRet < 15)
            break;
        uint32 nInvNum = bswap_32(*(uint32 *)cb);
        int nChildIndex = 4;
        for (i = 0; i < nInvNum; i++) {
            Transfer9ByteToFPDMAndFPHM(cb + nChildIndex, szFPDM, szFPHM);
            HFPXX fpxx = MallocFpxxLinkDev(hDev);
            int nChildRet = NisecQueryInvInfo(hDev, bInvType, szFPDM, szFPHM, fpxx);
            if (nChildRet < 0) {
                _WriteLog(LL_WARN, "Nisec QueryModifyInvoice failed, jump it, fpdm:%s fphm:%s",
                          fpxx->fpdm, fpxx->fphm);
                goto NextLoop;
            }
            if (fpxx->bIsUpload)
                goto NextLoop;
            //������ǰ������Ϣ
            strcpy(szCurFPDM, fpxx->fpdm);
            strcpy(szCurFPHM, fpxx->fphm);
            //ִ������IO
            _WriteLog(LL_INFO, "Try to upload waste fp,fplx:%d fpdm:%s fphm:%s", fpxx->fplx,
                      fpxx->fpdm, fpxx->fphm);

            nRet = UploadInvoice_CallBack(fpxx);
            //�����ж��ѭ����һ���Ŵ���
            break;
        NextLoop:
            FreeFpxx(fpxx);
            nChildIndex += 13;
        }
        break;
    }
    return nRet;
}

//�ϴ�xml������, pc_xml > 768
void NisecPreDownloadFPXML(HFPXX fpxx, char *szOut)
{
    sprintf(szOut,
            "<?xml version=\"1.0\" encoding=\"gbk\"?><business id=\"FPMXJG\" "
            "comment=\"��Ʊ��ϸ���\"><body count=\"1\" skph=\"%s\" "
            "nsrsbh=\"%s\" bbh=\"%d\"><group xh=\"1\"><data name=\"fplx_dm\" "
            "value=\"%03d\"/></group></body></business>",
            fpxx->hDev->szDeviceID, fpxx->hDev->szCommonTaxID,
            GetDeviceBBH(fpxx->hDev->szDriverVersion), fpxx->hDev->bOperateInvType);
}

//��δ����Net_IOͳһ�ӿڣ�����ݲ�֧��LastError
int NisecDownloadFPBySingle(HHTTP hi, HFPXX fpxx, char *pRepBuf, int nRepBufLen)
{
    int nRet = ERR_LOGIC_UPLOADINV_FAIL, nOpType = 0;
    do {
        if (fpxx->fplx == FPLX_COMMON_ZYFP || fpxx->fplx == FPLX_COMMON_PTFP)
            nOpType = TACMD_NISEC_QUERY_FPUP_ZZS;
        else if (fpxx->fplx == FPLX_COMMON_DZFP)
            nOpType = TACMD_NISEC_QUERY_FPUP_DZ;
        else
            break;
        NisecPreDownloadFPXML(fpxx, pRepBuf);
        if (TaNetIo(hi, nOpType, pRepBuf, nRepBufLen) < 0) {
            nRet = ERR_IO_FAIL;
            break;
        }
        nRet = RET_SUCCESS;
    } while (false);
    return nRet;
}

//ת���ֽ����У��¸�ʽ���������
int GetNewByteOrderDZSYH(HFPXX fpxx, char *szDZSYH, uint8 *abOutDZSYH)
{
    //ȷ����Ϣ���ģ�32B��HEX������Ʊ������4B��HEX����������ǩ��Ϣ����1(FPDM-ZYFP5bytes)��[������ǩ��Ϣ����2(FPHM-4bytes)]+��Ʊʱ��=51bytes
    char szFPDM[32] = "", szFPHM[32] = "", szFPDMNoPreZero[32] = "";
    int nFPDMLen = 12, nRet = 0;
    uint8 abBuf[512] = {0};
    char *p = NULL, *p2 = NULL;

    nRet = Str2Byte(abBuf, szDZSYH, strlen(szDZSYH));
    if (!nRet)
        return -1;
    memcpy(abOutDZSYH, abBuf, 36);
    // check fpdm hm match
    p = szDZSYH + 36 * 2;  // static
    // Find FPDM
    strcpy(szFPDMNoPreZero, fpxx->fpdm);
    MemoryFormart(MF_REMOVE_HEADZERO, szFPDMNoPreZero, strlen(szFPDMNoPreZero));
    if (!(p2 = strstr(p, szFPDMNoPreZero)))
        return -2;
    nFPDMLen = p2 + strlen(szFPDMNoPreZero) - p;
    strncpy(szFPDM, p, nFPDMLen);
    _WriteLog(LL_DEBUG, "GetNewByteOrderDZSYH analyze length of fpdm:%d, content:%s", nFPDMLen,
              szFPDM);
    p += nFPDMLen;
    strncpy(szFPHM, p, 8);
    if (atoll(fpxx->fpdm) != atoll(szFPDM) || atoll(fpxx->fphm) != atoll(szFPHM))
        _WriteLog(LL_WARN,
                  "abInvTemplate fpdm DZSYH not match orig's fpdmhm, "
                  "orig:%s %s, dzsyh:%s %s",
                  fpxx->fpdm, fpxx->fphm, szFPDM, szFPHM);

    TransferFPDMAndFPHMTo9Byte(szFPDM, szFPHM, abOutDZSYH + 36);
    memcpy(abOutDZSYH + 45, abBuf + 36 + (nFPDMLen / 2 + 4), 6);  // copy tail's date
    return 51;
}

int NisecUpdateInvUploadFlag(HFPXX fpxx, char *szDZSYH)
{
    HUSB hUsb = fpxx->hDev->hUSB;
    uint8 bInvType = fpxx->fplx;
    int nRet = -3, nTransferDataLen = 0;
    uint8 cb[1024] = {0}, abTransferData[512] = {0};
    nTransferDataLen = GetNewByteOrderDZSYH(fpxx, szDZSYH, abTransferData);
    if (nTransferDataLen < 0)
        return -1;
    if (NisecEntry(hUsb, NOP_OPEN_FULL) < 0)
        return -2;
    while (1) {
        if (NisecSelectInvoiceType(fpxx->hDev, bInvType, SELECT_OPTION_TEMPLATE) < 0)
            break;
        memcpy(cb, "\xfe\x2f\x01", 3);
        int nChildRet = NisecLogicIO(hUsb, cb, 3, sizeof(cb), abTransferData, nTransferDataLen);
        if (nChildRet < 0)
            break;
        nRet = 0;
        break;
    }
    NisecEntry(hUsb, NOP_CLOSE);
    return nRet;
}

int NisecInvoiceUploadAndDownload(HFPXX fpxx, char *szOutDZSYH)
{
    struct HTTP hi;
    int nRet = ERR_GENERIC, nRepBufSize = CalcAllocBuffLen(fpxx), nChildRet = -1;
    HDEV hDev = fpxx->hDev;
    char *response = calloc(1, nRepBufSize);
    if (!response)
        return ERR_BUF_ALLOC;
    while (1) {
        if (hDev->bBreakAllIO) {
            SetLastError(hDev->hUSB, ERR_IO_BREAK,
                         "��Ʊ�ϴ������б������ӿ��ж�[Place 1]");  //�˴���ֵ�ϲ��жϲ��������޸�
            logout(INFO, "TAXLIB", "��Ʊ�ϴ�", "�̺ţ�%s,�����ϴ���Ʊ,��⵽�ⲿҪ���ж�,%s\r\n",
                   hDev->szDeviceID, hDev->hUSB->errinfo);
            hDev->bBreakAllIO = 0;  //�������
			nRet = ERR_IO_BREAK;
            free(response);
            return nRet;
        }
        if ((nChildRet = CreateServerTalk(&hi, hDev)) < 0) {
            if (hDev->bBreakAllIO) {
                SetLastError(
                    hDev->hUSB, ERR_IO_BREAK,
                    "��Ʊ�ϴ������б������ӿ��ж�[Place 2]");  //�˴���ֵ�ϲ��жϲ��������޸�
                logout(INFO, "TAXLIB", "��Ʊ�ϴ�",
                       "�̺ţ�%s,�����ϴ���Ʊ,��⵽�ⲿҪ���ж�,%s\r\n", hDev->szDeviceID,
                       hDev->hUSB->errinfo);
                hDev->bBreakAllIO = 0;  //�������
				nRet = ERR_IO_BREAK;
                break;
            }
            if ((strstr(hDev->hUSB->errinfo, "ERROR@98@֤���ѹ�ʧ") != NULL) ||
                (strstr(hDev->hUSB->errinfo, "ERROR@103@֤��״̬δ֪") != NULL) ||
                (strstr(hDev->hUSB->errinfo, "ERROR@97@֤��������") != NULL)) {
                SetLastError(hDev->hUSB, ERR_TA_CERT_ERROR,"֤���ѹ�ʧ�������ϻ�״̬δ֪");  //�˴���ֵ�ϲ��жϲ��������޸�
                logout(INFO, "TAXLIB", "��Ʊ�ϴ�", "�̺ţ�%s,��Ʊ�ϴ���ֶ������쳣,��ʾ��%s\r\n",
                       hDev->szDeviceID, hDev->hUSB->errinfo);
				nRet = ERR_TA_CERT_ERROR;
                break;
            } else {
				nRet = ERR_TA_HTTPS_OPEN;
                break;
            }
        }
        if (hDev->bBreakAllIO) {
            SetLastError(hDev->hUSB, ERR_IO_BREAK,
                         "��Ʊ�ϴ������б������ӿ��ж�[Place 3]");  //�˴���ֵ�ϲ��жϲ��������޸�
            logout(INFO, "TAXLIB", "��Ʊ�ϴ�", "�̺ţ�%s,�����ϴ���Ʊ,��⵽�ⲿҪ���ж�,%s\r\n",
                   hDev->szDeviceID, hDev->hUSB->errinfo);
            hDev->bBreakAllIO = 0;  //�������
			nRet = ERR_IO_BREAK;
            break;
        }
        if ((nChildRet = NisecUploadFPBySingle(&hi, fpxx, response, nRepBufSize)) < 0) {
            _WriteLog(LL_WARN, "UploadInvoice failed, nChildRet:%d", nChildRet);
            nRet = ERR_TA_UPLOAD_INVOICE;
            break;
        }
        _WriteLog(LL_DEBUG, "Upload invoice OK, wait for download...");
        //���ϴ�Ҫ�Ȼ���������
        if (hDev->bBreakAllIO) {
            SetLastError(hDev->hUSB, ERR_IO_BREAK,
                         "��Ʊ�ϴ������б������ӿ��ж�[Place 4]");  //�˴���ֵ�ϲ��жϲ��������޸�
            logout(INFO, "TAXLIB", "��Ʊ�ϴ�", "�̺ţ�%s,�����ϴ���Ʊ,��⵽�ⲿҪ���ж�,%s\r\n",
                   hDev->szDeviceID, hDev->hUSB->errinfo);
            hDev->bBreakAllIO = 0;  //�������
			nRet = ERR_IO_BREAK;
            break;
        }
        sleep(2);
        if (hDev->bBreakAllIO) {
            SetLastError(hDev->hUSB, ERR_IO_BREAK,
                         "��Ʊ�ϴ������б������ӿ��ж�[Place 5]");  //�˴���ֵ�ϲ��жϲ��������޸�
            logout(INFO, "TAXLIB", "��Ʊ�ϴ�", "�̺ţ�%s,�����ϴ���Ʊ,��⵽�ⲿҪ���ж�,%s\r\n",
                   hDev->szDeviceID, hDev->hUSB->errinfo);
            hDev->bBreakAllIO = 0;  //�������
			nRet = ERR_IO_BREAK;
            break;
        }
        if ((nChildRet = NisecDownloadFPBySingle(&hi, fpxx, response, nRepBufSize)) < 0) {
            _WriteLog(LL_WARN, "QueryResult download failed, nChildRet:%d", nChildRet);
            nRet = ERR_TA_UPLOAD_INVOICE_DOWNLOAD;
            break;
        }
        nChildRet = strlen(response);
        if (nChildRet < 90 || nChildRet > 110) {
            _WriteLog(LL_WARN, "Upload nisec invoice failed, PrefixDZSYH's len:%d,errinfo = %s",
                      nChildRet, hDev->hUSB->errinfo);
            nRet = ERR_TA_DATA_ERROR;
            break;
        }
        _WriteLog(LL_DEBUG, "Download invoice's PrefixDZSYH OK, PrefixDZSYH:%s", response);
        strcpy(szOutDZSYH, response);
        nRet = RET_SUCCESS;
        break;
    }
    HTTPClose(&hi);
    free(response);
    return nRet;
}

// ZssLd.dll!PostFileToSSLAndSaveFileEx
int UploadInvoice_CallBack(HFPXX fpxx)
{
    char szDZSYH[256] = "";
    int bOK = 0;
    while (1) {
        //���ǩ��Ϊ�գ���Ҫ��ǩ�������˲����ϴ�
        if (!strlen(fpxx->sign)) {
            if (NisecFpxxAppendSignIO(fpxx, false)) {
                _WriteLog(LL_FATAL, "Update invoice's sign failed");
                break;
            }
            //���¶�ȡǩ����ķ�Ʊ
            if (NisecQueryInvInfo(fpxx->hDev, fpxx->fplx, fpxx->fpdm, fpxx->fphm, fpxx)) {
                _WriteLog(LL_FATAL, "Can not find this invoice %s %s", fpxx->fpdm, fpxx->fphm);
                break;
            }
        }
        if (NisecInvoiceUploadAndDownload(fpxx, szDZSYH)) {
            _WriteLog(LL_FATAL, "Nisec upload invoice,netio failed, step1");
            break;
        }
        if (NisecUpdateInvUploadFlag(fpxx, szDZSYH) < 0) {
            _WriteLog(LL_WARN, "Nisec upload invoice, updateflag failed, step2");
            break;
        }
        logout(INFO, "TAXLIB", "��Ʊ�ϴ�",
               "UploadInvoice_CallBack �̺�:%s,���߷�Ʊ�ϴ��ɹ�,��Ʊ������룺%s_%s\r\n",
               fpxx->hDev->szDeviceID, fpxx->fpdm, fpxx->fphm);
        bOK = 1;
        break;
    }
    _WriteLog(LL_DEBUG, "Nisec upload invoice %s, fplx:%d fpdm:%s fpgm:%s",
              bOK ? "successful" : "failed", fpxx->fplx, fpxx->fpdm, fpxx->fphm);
    return bOK;  //���ϱ�һ��
}

int NisecFillXmlCompnanyInfo(HDEV hDev, mxml_node_t *xml_root)
{
    // <qylxxx><nsrxz>01</nsrxz><qysj>20200522000000</qysj><swjgdm>13201252600</swjgdm><swjgmc>����˰���ܾ��Ͼ��иߴ���˰��ֹŰ�˰����</swjgmc><fplx>
    //<group
    // xh="1"><fplxdm>004</fplxdm><lxkpxe>100000</lxkpxe><lxkpsx>0</lxkpsx><scjzrq>1</scjzrq><bhssl>...</bhssl><hssl>0.05000</hssl><ptlx></ptlx><ptxh></ptxh></group>
    // <group xh="2">...</group></fplx>
    // <qybzfwsk><bmtsqybs>00</bmtsqybs><xgmzkzp>00</xgmzkzp><escqybs>00</escqybs><cpyqylx>00</cpyqylx><cpyqyyxq>0000000000000000</cpyqyyxq><cpybmd>00</cpybmd><cpybmdyxq>0000000000000000</cpybmdyxq></qybzfwsk>
    // <qybzskxt>(��qybzfwsk����Ŀ�ṹһ��)</qybzskxt></qylxxx>
    char szBuf[512] = "";
    int nRet = RET_SUCCESS;
    mxml_node_t *xmlNode = NULL, *xmlNode2 = NULL, *xmlNodeChild = NULL;
    xmlNode = mxmlNewElement(xml_root, "qylxxx");
    GetNatureOfTaxpayerStr(hDev->bNatureOfTaxpayer, szBuf);
    mxmlNewText(mxmlNewElement(xmlNode, "nsrxz"), 0, szBuf);
    mxmlNewText(mxmlNewElement(xmlNode, "qysj"), 0, hDev->szDeviceEffectDate);
    mxmlNewText(mxmlNewElement(xmlNode, "swjgdm"), 0, hDev->szTaxAuthorityCode);
    mxmlNewText(mxmlNewElement(xmlNode, "swjgmc"), 0, hDev->szTaxAuthorityName);
    //��Ʊ���ͽڵ�
    xmlNode2 = mxmlNewElement(xmlNode, "fplx");
    //�����Ϣ��������չ��Ϣ��Ŀǰ���Զ�Ʊ��һ�£���˱������һ��Ʊ�ּ����Ϣ��¼��qybzskxt��qybzfwskʹ��
    //����������������˰�ֵ������ٵ���
    HDITI invEnv = NULL;
    int i = 0;
    uint8 bSupportNum = hDev->abSupportInvtype[0];
    for (i = 0; i < bSupportNum; i++) {
        if (!(invEnv = GetInvoiceTypeEnvirnment(hDev, hDev->abSupportInvtype[1 + i]))) {
            nRet = ERR_DEVICE_GET_ENV;
            break;
        }
        // new
        xmlNodeChild = mxmlNewElement(xmlNode2, "group");
        sprintf(szBuf, "%d", i + 1);
        mxmlElementSetAttr(xmlNodeChild, "xh", szBuf);  //��������
        // ��Ʊ����
        sprintf(szBuf, "%03d", invEnv->bInvType);
        mxmlNewText(mxmlNewElement(xmlNodeChild, "fplxdm"), 0, szBuf);
        //���ſ�Ʊ�޶�
        mxmlNewText(mxmlNewElement(xmlNodeChild, "dzkpxe"), 0, invEnv->monitor.dzkpxe);
        //���߿�Ʊ�޶�
        long double fNum =
            strtold(invEnv->monitor.lxzsljje, NULL) + strtold(invEnv->monitor.lxfsljje, NULL);
        memset(szBuf, 0, sizeof(szBuf));
        sprintf(szBuf, "%4.2Lf", fNum);
        mxmlNewText(mxmlNewElement(xmlNodeChild, "lxkpxe"), 0, invEnv->monitor.lxzsljje);
        //���߿�Ʊʱ��
        mxmlNewText(mxmlNewElement(xmlNodeChild, "lxkpsx"), 0, invEnv->monitor.lxkpsc);
        //�ϴ���ֹ���� 1��
        mxmlNewText(mxmlNewElement(xmlNodeChild, "scjzrq"), 0, invEnv->monitor.scjzrq);
        //����˰˰�ʣ�����̶�ֵ
        GetTaxrate(invEnv->szTaxrate, false, szBuf);
        mxmlNewText(mxmlNewElement(xmlNodeChild, "bhssl"), 0, szBuf);
        //��˰˰��
        GetTaxrate(invEnv->szTaxrate, true, szBuf);
        if (strlen(szBuf) != 0)
            mxmlNewText(mxmlNewElement(xmlNodeChild, "hssl"), 0, szBuf);
        else
            mxmlNewElement(xmlNodeChild, "hssl");
        //ƽ̨����
        memset(szBuf, 0, sizeof(szBuf));
        strncpy(szBuf, invEnv->monitor.ggfwpt, 2);
        if (strlen(szBuf) != 0)
            mxmlNewText(mxmlNewElement(xmlNodeChild, "ptlx"), 0, szBuf);
        else
            mxmlNewElement(xmlNodeChild, "ptlx");
        //ƽ̨���
        memset(szBuf, 0, sizeof(szBuf));
        strcpy(szBuf, invEnv->monitor.ggfwpt + 2);
        if (strlen(szBuf) != 0)
            mxmlNewText(mxmlNewElement(xmlNodeChild, "ptxh"), 0, szBuf);
        else
            mxmlNewElement(xmlNodeChild, "ptxh");
    }
    // qybzskxt
    xmlNodeChild = mxmlNewElement(xmlNode, "qybzskxt");
    // bmtsqybs
    NewByteStringToXmlNode(xmlNodeChild, "bmtsqybs", (uint8 *)&hDev->stDevExtend.bmtsqybs);
    //С��ģ�Կ�רƱ
    if (hDev->bNatureOfTaxpayer == 3)
        NewByteStringToXmlNode(xmlNodeChild, "xgmzkzp", (uint8 *)&hDev->stDevExtend.xgmkjzpbs);
    else
        mxmlNewElement(xmlNodeChild, "xgmzkzp");
    //���ֳ���ҵ��ʶ
    NewByteStringToXmlNode(xmlNodeChild, "escqybs", (uint8 *)&hDev->stDevExtend.esjdcbs);
    //��Ʒ����ҵ����
    NewByteStringToXmlNode(xmlNodeChild, "cpyqylx", (uint8 *)&hDev->stDevExtend.cpybs);
    //��Ʒ����ҵ��Ч��
    mxmlNewText(mxmlNewElement(xmlNodeChild, "cpyqyyxq"), 0, hDev->stDevExtend.cpybsyxq);
    //��Ʒ�Ͱ�����
    NewByteStringToXmlNode(xmlNodeChild, "cpybmd", (uint8 *)&hDev->stDevExtend.cpybmdbs);
    //��Ʒ�Ͱ�������Ч��
    mxmlNewText(mxmlNewElement(xmlNodeChild, "cpybmdyxq"), 0, hDev->stDevExtend.cpybmdbsyxq);
    //�ض���ҵ
    NewByteStringToXmlNode(xmlNodeChild, "tzqy", (uint8 *)&hDev->stDevExtend.tdqy);
    //������ҵ
    NewByteStringToXmlNode(xmlNodeChild, "jyqybz", (uint8 *)&hDev->stDevExtend.jyqy);
    //������ҵ��Ч��
    mxmlNewText(mxmlNewElement(xmlNodeChild, "jyqyyxq"), 0, hDev->stDevExtend.jyqyyxq);
    //��������ҵ
    NewByteStringToXmlNode(xmlNodeChild, "jdcqybz", (uint8 *)&hDev->stDevExtend.jdcqy);
    //��������ҵ��Ч��
    mxmlNewText(mxmlNewElement(xmlNodeChild, "jdcqyyxq"), 0, hDev->stDevExtend.jdcqyyxq);
    //��������������ʶ
    NewByteStringToXmlNode(xmlNodeChild, "jdcbmdbz", (uint8 *)&hDev->stDevExtend.jdcbmdbs);
    //��������������Ч��
    mxmlNewText(mxmlNewElement(xmlNodeChild, "jdcbmdyxq"), 0, hDev->stDevExtend.jdcbmdyxq);
    //ί�д�����ҵ
    NewByteStringToXmlNode(xmlNodeChild, "wtdkbz", (uint8 *)&hDev->stDevExtend.wtdkqy);
    //ί�д�����ҵ��Ч��
    mxmlNewText(mxmlNewElement(xmlNodeChild, "wtdkyxq"), 0, hDev->stDevExtend.wtdkqyyxq);
    //�Ǽ���������
    mxmlNewText(mxmlNewElement(xmlNodeChild, "djzclx"), 0, hDev->stDevExtend.djzclx);
    // qybzfwsk,һ�㶼Ϊ��
    xmlNodeChild = mxmlNewElement(xmlNode, "qybzfwsk");
    mxmlNewElement(xmlNodeChild, "bmtsqybs");
    mxmlNewElement(xmlNodeChild, "xgmzkzp");
    mxmlNewElement(xmlNodeChild, "escqybs");
    mxmlNewElement(xmlNodeChild, "cpyqylx");
    mxmlNewElement(xmlNodeChild, "cpyqyyxq");
    mxmlNewElement(xmlNodeChild, "cpybmd");
    mxmlNewElement(xmlNodeChild, "cpybmdyxq");
    mxmlNewElement(xmlNodeChild, "tzqy");
    return nRet;
}

//�˴��漰ͬ���ľ�׼�ԣ������ȡʵʱʱ�Ӻ����������������HDEV�м�¼�ľ�̬��ֵ
//���ڷ�ɢ�����õĶ���Կ��Ǽ���hdev
int NisecFillRtcTimeAndRand(HDEV hDev, mxml_node_t *xml_root)
{
    // char szFsyz[64] = "";
    int nRet = ERR_GENERIC;
    mxml_node_t *xmlNode = mxmlNewElement(xml_root, "hqszxx");
    do {
        // if (NisecEntry(hDev->hUSB, NOP_OPEN))
        //    break;
        // _WriteLog(LL_DEBUG, "lbc--------------test NisecFillRtcTimeAndRand1111  hDev->szDeviceTime=%s", hDev->szDeviceTime);
        // if (NisecGetDeviceTime(hDev->hUSB, hDev->szDeviceTime))
        //    break;
        // _WriteLog(LL_DEBUG, "lbc--------------test NisecFillRtcTimeAndRand2222  hDev->szDeviceTime=%s", hDev->szDeviceTime);
        mxmlNewText(mxmlNewElement(xmlNode, "skpsj"), 0, hDev->szDeviceTime);
        // if (NisecGetDeviceFsyz(hDev->hUSB, szFsyz))
        //     break;
        // _WriteLog(LL_DEBUG, "lbc--------------test NisecFillRtcTimeAndRand szFsyz=%s", szFsyz);
        mxmlNewText(mxmlNewElement(xmlNode, "fsyz"), 0, hDev->szFsyz);
        nRet = RET_SUCCESS;
    } while (false);
    //NisecEntry(hDev->hUSB, NOP_CLOSE);
    return nRet;
}

//������Ϣ��Ϊ�գ���Ҫ�ֶ��ͷŷ���ֵ
char *NisecPreCompanyXml(HDEV hDev)
{
    char *pRet = NULL;
    mxml_node_t *root = NULL, *xml_root = NULL, *xmlNode = NULL;
    do {
        if (!(root = mxmlNewXML("1.0")))
            break;
        if (!(xml_root = mxmlNewElement(root, "business")))
            break;
        mxmlElementSetAttr(xml_root, "id", "QYXXXZ");
        mxmlElementSetAttr(xml_root, "comment", "��ҵ��Ϣ���ؽӿ�");
        xml_root = mxmlNewElement(xml_root, "body");
        mxmlNewText(mxmlNewElement(xml_root, "nsrsbh"), 0, hDev->szCommonTaxID);
        mxmlNewText(mxmlNewElement(xml_root, "sblx"), 0, "00");  // static
        mxmlNewText(mxmlNewElement(xml_root, "sbbh"), 0, hDev->szDeviceID);
        mxmlNewText(mxmlNewElement(xml_root, "sbbbh"), 0, hDev->szDriverVersion);
        // 1)��ҵ�������� qycsxz
        mxmlNewElement(xml_root, "qycsxz");
        // 2)��ҵ������Ϣ,�ص�ͬ������
        if (NisecFillXmlCompnanyInfo(hDev, xml_root) < 0)
            break;
        // 3��Զ�̱����ѯ
        xmlNode = mxmlNewElement(xml_root, "ycbgcx");
        mxmlNewText(mxmlNewElement(xmlNode, "bglx"), 0, "01");
        mxmlNewText(mxmlNewElement(xmlNode, "pyhlx"), 0, "01");
        // 4��ʱ����Ϣ,��ɢ����
        if (NisecFillRtcTimeAndRand(hDev, xml_root))
            break;
        // 5����������˽�а汾
        xmlNode = mxmlNewElement(xml_root, "spbm");
        mxmlNewText(mxmlNewElement(xmlNode, "version"), 0, "3.0");
        mxmlNewElement(xmlNode, "ysbz");
        mxmlNewText(mxmlNewElement(xmlNode, "bbh"), 0, DEF_BMBBBH_DEFAULT);
        mxmlNewElement(xmlNode, "spbm");
        mxmlNewElement(xmlNode, "spmc");
        //��˰��ͳһ�汾��nisec˽�а汾��
        mxmlNewText(mxmlNewElement(xmlNode, "kprjbb"), 0, "V2.0.47");
        mxmlNewText(mxmlNewElement(xmlNode, "rjlx"), 0, "01");
        // 6������ǰ˰����Ȩ��Ϣ,��Ҫ�ڿ�Ʊ����豸����Ҳ������Ȩ����Ч��С�������ݲ����Ǽ���
        mxmlNewElement(xml_root, "tzqslsq");
        pRet = mxmlSaveAllocString(root, MXML_NO_CALLBACK);
    } while (false);
    mxmlDelete(root);
    return pRet;
}