/*****************************************************************************
File name:   cntax_uploadinvoice.c
Description: cntax�豸��Ʊ�ϴ������߼�
Author:      Zako
Version:     1.0
Date:        2021.04.01
History:
20210401     �������ʵ��
*****************************************************************************/
#include "cntax_uploadinvoice.h"

int CntaxPreInvoiceXML(HFPXX fpxx, char *pOutXMLBuf)
{
    int nRawFpLen = *(int *)fpxx->pRawFPBin;
    if (nRawFpLen < 0 || nRawFpLen > DEF_MAX_FPBUF_LEN)
        return -1;
    // mxysbz = "0",SelectInvoType֮ǰ���õľ�ֵ̬����ʱ�������������
    uint8 *szEscape = NULL;
    int nChildRet = 0;
    do {
        nChildRet = HtmlEscape(&szEscape, fpxx->pRawFPBin + 4, nRawFpLen);
        sprintf(pOutXMLBuf, "<ywxx><fplxdm>%03d</fplxdm><fpmx>%s</fpmx><mxysbz>0</mxysbz></ywxx>",
                fpxx->fplx, szEscape);
    } while (false);
    if (nChildRet)
        free(szEscape);
    return 0;
}

int CntaxUpdateInvUploadFlag(HDEV hDev, uint8 bStep, uint8 *abTransferData, int nTransferDataLen)
{
    HUSB hUsb = hDev->hUSB;
    int nRet = -1;
    uint8 cb[1024] = {0};
    if (CntaxEntry(hUsb, NOP_OPEN_FULL) < 0)
        return -2;
    while (1) {
        if (CntaxSelectInvoiceType(hDev, hDev->bOperateInvType, SELECT_OPTION_TEMPLATE) < 0)
            break;
        if (bStep == 1) {  // 0x1c ��һ���ϴ�ȷ������
            memcpy(cb, "\xfe\x22\x01", 3);
        } else if (bStep == 2) {  //���߷�Ʊ��д���ģ�ͨ�÷�д���ģ�
            memcpy(cb, "\xfe\x22\x00", 3);
            //�˴�Ϊ��Ʊ�ϴ������һ����д���˴�break���Ͳ���д�����дIO
            // break;
        }
        int nChildRet = CntaxLogicIO(hUsb, cb, 3, sizeof(cb), abTransferData, nTransferDataLen);
        if (nChildRet < 0) {
            logout(INFO, "TAXLIB", "��Ʊ�ϴ�",
                   "�̺ţ�%s,��Ʊ�ϴ���д��%d��ʧ��,nChildRet = %d,errinfo = %s\r\n",
                   hDev->szDeviceID, bStep, nChildRet, hUsb->szLastErrorDescription);
            break;
        }
        logout(INFO, "TAXLIB", "��Ʊ�ϴ�", "�̺ţ�%s,��Ʊ�ϴ���д��%d���ɹ�\r\n", hDev->szDeviceID,
               bStep);
        nRet = 0;
        break;
    }
    CntaxEntry(hUsb, NOP_CLOSE);
    return nRet;
}

//��δ����Net_IOͳһ�ӿڣ�����ݲ�֧��LastError
//����ֵ����fpxx->szSLXLH��д��32λ�������кţ����ڿ���ֱ��ͨ�������кŲ�ѯ�ӿ�
int CntaxUploadFPBySingle(HHTTP hi, HFPXX fpxx, char *pRepBuf, int nRepBufLen)
{
    int nBufSzie = CalcAllocBuffLen(fpxx), nRet = -3;
    char *szInvXML = calloc(1, nBufSzie);
    if (!szInvXML)
        return -2;
    do {
        if (CntaxPreInvoiceXML(fpxx, szInvXML) < 0)
            break;
        if ((nRet = TaNetIo(hi, TACMD_CNTAX_UPFP, szInvXML, nBufSzie)) < 0) {
            _WriteLog(LL_FATAL, "TaNetIo failed, nRet:%d", nRet);
            break;
        }
        //�����ȷ���������кſ�������
        strcpy(pRepBuf, szInvXML);
        nRet = 0;
    } while (false);
    free(szInvXML);
    return nRet;
}

//����ֵ<0 ��������fpxx->hDev->hUSBDevice->szLastErrorDescription��
int CntaxDownloadFPBySingle(HHTTP hi, HFPXX fpxx, char *pRepBuf, int nRepBufLen)
{
    char szInvXML[512] = "", szBuf[512000] = "";
    int nOpType = TACMD_CNTAX_QUERY_UPFP, i = 0, nRet = -1;
    sprintf(szInvXML, "<ywxx><fplxdm>%03d</fplxdm><sllsh>%s</sllsh></ywxx>", fpxx->fplx, pRepBuf);
    pRepBuf[0] = '\0';
    //��һ���ύ���ڶ��β�ѯ���;���֮ǰû���ύ��Ҫ2�Σ����֮ǰ�Ѿ��ϴ��ύ���ˣ�ִ��һ�ξͿ��Ի�ȡ���
    for (i = 0; i < 2; i++) {
        strcpy(szBuf, szInvXML);
        if ((nRet = TaNetIo(hi, nOpType, szBuf, sizeof(szBuf))) < 0) {
            _WriteLog(LL_FATAL, "TaNetIo failed, nRet:%d", nRet);
            break;
        }
        if (strlen(szBuf)) {
            strcpy(pRepBuf, szBuf);
            nRet = 0;
            break;
        }
    }
    return nRet;
}

//�е�һ��ȷ�����ĵĲ�д�루����д����д��˰�̺�ȷ����������ʧ�������ٻ�ȡ����
int WriteNewUploadInvoiceUploadMw(HFPXX fpxx)
{
    //������3�����
    // 1)����δ�ϴ���Ʊ����ͨ������Ʊ�ӿڲ�ѯ��������abUploadSureMW10x1c
    // 2)��δ�ϴ������ϵģ�������Ʊ��ѯ�ӿڲ�ѯ�����ģ���abUploadSureMW10x1c
    // 3)�ϴ��������ϣ���modify�ӿڣ����ٷ����ϴ���Ʊ�ӿڣ���ѯ�����ģ���abUploadSureMW10x1c��Ϣ
    // 4)��Ʊȫ��ͬ����ɺ���ָ����Ʊ��Ϣ�ӿڲ�ѯ���¶Ȼָ��Ƚӿڲ�ѯ�����ģ�����abUploadSureMW10x1c��Ϣ?
    //
    //ʵ������Ʊ�ӿڲ������һ�㲻����abUploadSureMW10x1c��Ϣ����ʱ���Ź�
    //������ϴ�ȷ�����ģ���һ�����ģ��������ϴ���д���ϴ���д���ģ��ڶ������ģ�
    //����ȷ������δд��˰�̣������˰�̴���-���һ������Ʊʵ���ϴ�����˰����Ϊδ�ϴ�����Ҫȥ˰�ִ���
    char szNoUploadSureMWFlag[] = "NoData!";  //�Զ������ݣ���ʵ������
    uint8 abUploadSureMW10x1c[64] = {0};
    int nRawFpLen = *(int *)fpxx->pRawFPBin, nRet = ERR_GENERIC;
    bool bHasUploadSureMw = false;
    do {
        if (nRawFpLen < 0 || nRawFpLen > DEF_MAX_FPBUF_LEN) {
            nRet = ERR_BUF_CHECK;
            break;
        }
        memcpy(abUploadSureMW10x1c, fpxx->pRawFPBin + 4 + nRawFpLen, 0x1c);
        bHasUploadSureMw = strcmp((char *)abUploadSureMW10x1c, szNoUploadSureMWFlag);
        // if (fpxx->bIsUpload && fpxx->zfbz && !bHasUploadSureMw) {//���߼�
        if (fpxx->zfbz && !bHasUploadSureMw) {  //�˴����ɰط�����Ϊ׼�������޸�
            //���ϴ��ķ�Ʊ�����ϣ��������ϱ�־������δ�ϴ���־�����ǲ���ͨ��δ�ϴ��ӿڲ�ѯ����
            //�Ҹ÷�Ʊ��Ȼ���ϴ�ȷ�����ģ����ڵ�һ���ϴ���ʱ��д���ˣ������跴д��һ���ϴ�����
            nRet = RET_SUCCESS;
            break;
        }
        if (!bHasUploadSureMw) {
            //ʣ�������ĵĶ�����
            SetLastError(fpxx->hDev->hUSB, ERR_LOGIC_UPLOADINV_FAIL,
                         "������Ʊδ����ȷ������");  //�˴���ֵ�ϲ��жϲ��������޸�
            nRet = ERR_BUF_CHECK;
            break;
        }
        if (CntaxUpdateInvUploadFlag(fpxx->hDev, 1, abUploadSureMW10x1c, 0x1c) < 0) {
            nRet = ERR_IO_FAIL;
            break;
        }
        nRet = RET_SUCCESS;
    } while (false);
    return nRet;
}

int CntaxInvoiceUploadAndDownload(HFPXX fpxx, char *szOutDZSYH)
{
    struct HTTP hi;
    memset(&hi, 0, sizeof(struct HTTP));
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
            // report_event(fpxx->hDev->szDeviceID, "���߷�Ʊ�ϴ�ʧ��", errinfo, -2);
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
                // report_event(hDev->szDeviceID, "���߷�Ʊ�ϴ�ʧ��", errinfo, -2);
                nRet = ERR_IO_BREAK;
                break;
            }

            if ((strstr(hDev->hUSB->errinfo, "ERROR@98@֤���ѹ�ʧ") != NULL) ||
                (strstr(hDev->hUSB->errinfo, "ERROR@103@֤��״̬δ֪") != NULL) ||
                (strstr(hDev->hUSB->errinfo, "ERROR@97@֤��������") != NULL)) {
                SetLastError(hDev->hUSB, ERR_TA_CERT_ERROR,
                             "֤���ѹ�ʧ�������ϻ�״̬δ֪");  //�˴���ֵ�ϲ��жϲ��������޸�
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
            // report_event(hDev->szDeviceID, "���߷�Ʊ�ϴ�ʧ��", errinfo, -2);
            nRet = ERR_IO_BREAK;
            break;
        }

        //_WriteLog(LL_INFO, "Prepare to upload invoice");
        if ((nChildRet = CntaxUploadFPBySingle(&hi, fpxx, response, nRepBufSize)) < 0) {
            _WriteLog(LL_WARN, "UploadInvoice failed, nChildRet:%d", nChildRet);
            nRet = ERR_TA_UPLOAD_INVOICE;
            break;
        }
        //����ϴ�û���⣬���ж��Ƿ����ϴ���Ʊ�����ϴ���Ʊ��Ҫд���ϴ�ȷ������
		if ((nChildRet = WriteNewUploadInvoiceUploadMw(fpxx)) < 0) {
            _WriteLog(LL_WARN, "Upload invoice, updateflag failed, step1");
			logout(INFO, "TAXLIB", "��Ʊ�ϴ�", "�̺ţ�%s,WriteNewUploadInvoiceUploadMw nChildRet=%d\r\n", hDev->szDeviceID, nChildRet);
			nRet = ERR_TA_UPLOAD_INVOICE_UPDATE;
            break;
        }
        if (hDev->bBreakAllIO) {
            SetLastError(hDev->hUSB, ERR_IO_BREAK,
                         "��Ʊ�ϴ������б������ӿ��ж�[Place 4]");  //�˴���ֵ�ϲ��жϲ��������޸�
            logout(INFO, "TAXLIB", "��Ʊ�ϴ�", "�̺ţ�%s,�����ϴ���Ʊ,��⵽�ⲿҪ���ж�,%s\r\n",
                   hDev->szDeviceID, hDev->hUSB->errinfo);
            hDev->bBreakAllIO = 0;  //�������
            // report_event(hDev->szDeviceID, "���߷�Ʊ�ϴ�ʧ��", errinfo, -2);
            nRet = ERR_IO_BREAK;
            break;
        }
        sleep(3);  //���ȴ�����ַ�Ʊ��δ�����Ĵ���
        _WriteLog(LL_DEBUG, "Upload invoice OK, wait for download...");
        if (hDev->bBreakAllIO) {
            SetLastError(hDev->hUSB, ERR_IO_BREAK,
                         "��Ʊ�ϴ������б������ӿ��ж�[Place 5]");  //�˴���ֵ�ϲ��жϲ��������޸�
            logout(INFO, "TAXLIB", "��Ʊ�ϴ�", "�̺ţ�%s,�����ϴ���Ʊ,��⵽�ⲿҪ���ж�,%s\r\n",
                   hDev->szDeviceID, hDev->hUSB->errinfo);
            hDev->bBreakAllIO = 0;  //�������
            // report_event(hDev->szDeviceID, "���߷�Ʊ�ϴ�ʧ��", hDev->hUSB->errinfo,
            // -2);
            nRet = ERR_IO_BREAK;
            break;
        }
        if ((nChildRet = CntaxDownloadFPBySingle(&hi, fpxx, response, nRepBufSize)) < 0) {
            _WriteLog(LL_WARN, "QueryResult download failed, nChildRet:%d", nChildRet);
            nRet = ERR_TA_UPLOAD_INVOICE_DOWNLOAD;
            break;
        }

        nChildRet = strlen(response);
        if (nChildRet < 90 || nChildRet > 110) {
            _WriteLog(LL_WARN, "Upload invoice failed, mxjgmw's len:%d,errinfo = %s", nChildRet,
                      hDev->hUSB->errinfo);
            nRet = ERR_TA_DATA_ERROR;
            break;
        }
        _WriteLog(LL_DEBUG, "Download invoice's mxjgmw OK, mw:%s\n", response);
        strcpy(szOutDZSYH, response);
        nRet = RET_SUCCESS;
        break;
    }
    HTTPClose(&hi);
    free(response);
    return nRet;
}

int CntaxUploadInvoiceCallBack(HFPXX fpxx)
{
    char szDZSYH[256] = "";
    uint8 abTransferData[256] = {0};
    int bOK = 0;
    while (1) {
        //���ǩ��Ϊ�գ���Ҫ��ǩ�������˲����ϴ�
        if (!strlen(fpxx->sign)) {
            if (CntaxFpxxAppendSignIO(fpxx, false)) {
                _WriteLog(LL_FATAL, "Update invoice's sign failed");
                break;
            }
            _WriteLog(LL_INFO, "CntaxFpxxAppendSignIO success");
            //���¶�ȡǩ����ķ�Ʊ
            if (CntaxQueryInvInfo(fpxx->hDev, fpxx->fplx, fpxx->fpdm, fpxx->fphm, fpxx)) {
                _WriteLog(LL_FATAL, "Can not find this invoice %s %s", fpxx->fpdm, fpxx->fphm);
                break;
            }
            _WriteLog(LL_INFO, "CntaxQueryInvInfo success");
        }
        if (CntaxInvoiceUploadAndDownload(fpxx, szDZSYH)) {
            _WriteLog(LL_FATAL, "CntaxInvoiceUploadAndDownload,netio failed");
            break;
        }
        int nTransferDataLen = Str2Byte(abTransferData, szDZSYH, strlen(szDZSYH));
        if (CntaxUpdateInvUploadFlag(fpxx->hDev, 2, abTransferData, nTransferDataLen) < 0) {
            _WriteLog(LL_WARN, "Upload invoice, updateflag failed, step2");
            break;
        }
        _WriteLog(LL_INFO, "CntaxUpdateInvUploadFlag success");
        bOK = 1;
        break;
    }
    if (bOK == 1) {
        logout(INFO, "TAXLIB", "��Ʊ�ϴ�",
               "�̺ţ�%s,CntaxUploadInvoiceCallBack �ϴ���Ʊ fpdm = %s,fphm = %s �ɹ�\r\n",
               fpxx->hDev->szDeviceID, fpxx->fpdm, fpxx->fphm);
    } else {
        logout(INFO, "TAXLIB", "��Ʊ�ϴ�",
               "�̺ţ�%s,CntaxUploadInvoiceCallBack �ϴ���Ʊ fpdm = %s,fphm = %s ʧ��\r\n",
               fpxx->hDev->szDeviceID, fpxx->fpdm, fpxx->fphm);
    }
    _WriteLog(LL_DEBUG, "Cntax upload invoice %s, fplx:%d fpdm:%s fpgm:%s",
              bOK ? "successful" : "failed", fpxx->fplx, fpxx->fpdm, fpxx->fphm);
    return bOK;  //���ϱ�һ��
}

////��ѯ:1)���ϴ���Ʊ��ǩʧ�ܡ�2)��������3)���ߺ����ϵķ�Ʊ�б�
//��ȡδ�ϴ���Ʊ��Ϣ�����ϴ�δȷ�Ϸ�Ʊ����(����������Ʊ�ϴ���δ��ɵڶ����ļ����ϴ��������ϵ�)
//����ֵ<0 IO����==0���޸ķ�Ʊ;>0�Ѵ�����޸ķ�Ʊ����
int CntaxQueryModifyInvoice(uint8 bInvType, HDEV hDev, char *szOutFPDM, char *szOutFPHM)
{
    HUSB hUsb = hDev->hUSB;
    uint8 cb[256] = {0};
    char szFPDM[64] = "", szFPHM[64] = "";
    int nChildRet = 0, nChildRet2 = 0, i = 0, nRet = ERR_GENERIC;
    while (true) {
        // query buf
        if (CntaxEntry(hUsb, NOP_OPEN_FULL) < 0)
            break;
        if (CntaxSelectInvoiceTypeMini(hUsb, bInvType) < 0) {
            CntaxEntry(hDev->hUSB, NOP_CLOSE);
            break;
        }
        uint8 *pRetBuff = NULL;
        nChildRet =
            CntaxGetInvBuff(hDev->hUSB, FPCX_COMMON_MODIFY, &pRetBuff, &nChildRet2, NULL, 0, NULL);
        if (pRetBuff) {
            if (nChildRet > 0)
                memcpy(cb, pRetBuff, nChildRet);
            free(pRetBuff);
        }
        CntaxEntry(hDev->hUSB, NOP_CLOSE);
        if (nChildRet < 15)
            break;
        uint32 nInvNum = bswap_32(*(uint32 *)cb);
        int nChildIndex = 4;
        for (i = 0; i < nInvNum; i++) {
            Transfer10ByteToFPDMAndFPHM(cb + nChildIndex, szFPDM, szFPHM);
            HFPXX fpxx = MallocFpxxLinkDev(hDev);
            int nChildRet = CntaxQueryInvInfo(hDev, bInvType, szFPDM, szFPHM, fpxx);
            if (nChildRet < 0) {
                _WriteLog(LL_WARN, "Nisec QueryModifyInvoice failed, jump it, fpdm:%s fphm:%s",
                          fpxx->fpdm, fpxx->fphm);
                goto NextLoop;
            }
            if (fpxx->bIsUpload)
                goto NextLoop;
            //������ǰ������Ϣ
            strcpy(szOutFPDM, fpxx->fpdm);
            strcpy(szOutFPHM, fpxx->fphm);
            //ִ������IO
            _WriteLog(LL_INFO, "Try to upload waste fp,fplx:%d fpdm:%s fphm:%s", fpxx->fplx,
                      fpxx->fpdm, fpxx->fphm);
            logout(INFO, "TAXLIB", "��Ʊ�ϴ�",
                   "�̺ţ�%s,CntaxQueryModifyInvoice fpdm = %s,fphm = %s\r\n", hDev->szDeviceID,
                   fpxx->fpdm, fpxx->fphm);
            nRet = CntaxUploadInvoiceCallBack(fpxx);
            //�����ж��ѭ����һ���Ŵ���
            FreeFpxx(fpxx);
            break;

        NextLoop:
            FreeFpxx(fpxx);
            nChildIndex += 13;
        }
        break;
    }
    return nRet;
}

//��API���Ͳ����¿��ؿ��ˣ���Ҫ�ѿ���״̬
int CntaxQueryOfflineInvoiceNum(HDEV hDev)
{
    //�����汾��Ϊ"00020001220225"�������Ҳ�֪������ֵ�������������ֵ����4�򱨴�
    uint8 *pRetBuff = NULL;
    int nRet = 0;
    do {
        if (CntaxGetInvBuff(hDev->hUSB, FPCX_COMMON_OFFLINE_NUM, &pRetBuff, &nRet, NULL, 0,
                            hDev->szDriverVersion) != 4) {
            nRet = ERR_IO_REPCHECK;
            break;
        }
        uint32 num = *(uint32 *)pRetBuff;
        nRet = ntohl(num);
    } while (false);
    if (pRetBuff)
        free(pRetBuff);
    return nRet;
}

//��ѯ��ɺ��Զ��ص��ϴ�������ֵΪ����ķ�Ʊ������һ���Ŵ���
int CntaxQueryOfflineInvoice(uint8 bInvType, HDEV hDev)
{
    int nRet = ERR_GENERIC, nChildRet = 0;
    //һ�δ���2�ſ��Ըĳ�2��2�ŵ������Ǻ�ǿ�ң��ݲ��������
    uint8 abQueryDataCount[4] = {0x00, 0x00, 0x00, 0x01};
    uint8 *pRetBuff = NULL;
    if (CntaxEntry(hDev->hUSB, NOP_OPEN_FULL) < 0)
        return ERR_DEVICE_OPEN;
    do {
        if (CntaxSelectInvoiceType(hDev, bInvType, SELECT_OPTION_TEMPLATE) < 0)
            break;
        nChildRet = CntaxQueryOfflineInvoiceNum(hDev);
        if (nChildRet <= 0) {
            if (nChildRet < 0)
                nRet = ERR_IO_FAIL;
            else {
                nRet = 0;
                _WriteLog(LL_INFO, "Invoice no need upload, inv-type:%d", bInvType);
            }
            break;
        }
        _WriteLog(LL_DEBUG, "Invoice need upload, inv-type:%d, num:%d", bInvType, nChildRet);
        nChildRet = 0;
        // FPCX_COMMON_OFFLINE�滻��FPCX_COMMON_LASTINV���
        int nInvBufSize = CntaxGetInvBuff(hDev->hUSB, FPCX_COMMON_OFFLINE, &pRetBuff, &nChildRet,
                                          abQueryDataCount, sizeof abQueryDataCount, NULL);
        if (nInvBufSize <= 0) {
            if (hDev->hUSB->nLastErrorCode == ERR_NISEC_NO_DATA) {
                nRet = 0;
                _WriteLog(LL_INFO, "No invoice to upload, Invoice's type:%d", bInvType);
            } else
                nRet = ERR_IO_FAIL;
            break;
        }
        _WriteHexToDebugFile("cntax_inv_raw.bin", pRetBuff, nChildRet);
        //���صķ�Ʊbufֻ��һ�ŵ��ڴ棬���Ҳֻ����һ�Σ�����ѭ������ڶ���
        nRet = CntaxGetInvDetail(hDev, FPCX_COMMON_OFFLINE, pRetBuff, nInvBufSize,
                                 CntaxUploadInvoiceCallBack);
    } while (false);
    CntaxEntry(hDev->hUSB, NOP_CLOSE);
    if (pRetBuff)
        free(pRetBuff);
    return nRet;
}

int BuildFirewareUpdateRequest(HDEV hDev, char *szOut)
{
    //<?xml version="1.0" encoding="UTF-8"?><business
    // id="CHECKUPDATE"><body><nsrsbh>91320118MA20LU6N3P</nsrsbh><csbs>53</csbs><sbbh>537100494968</sbbh>
    //<gjbbh>00010000191023-53010003191105</gjbbh>/<yjbbh>ABCDEF</yjbbh>/</body></business>
    uint8 cb[256] = {0};
    char szTaVersion[64] = "", szVendorVersion[64] = "", szGjbbh[128] = "", szVendorCode[16] = "";
    int nChildRet = 0, nRet = ERR_PROGRAME_API;
    do {
        // fe7f01 |fe0101(��ȡ������Ϣ��Ҳ��������Ϣ)
        // 00010000191023
        memcpy(cb, "\xfe\x7f\x01", 3);
        if ((nChildRet = CntaxLogicIO(hDev->hUSB, cb, 3, sizeof(cb), NULL, 0)) < 7)
            break;
        Byte2Str(szTaVersion, cb, nChildRet);
        // API�ӿ�UK_ReadBSPVersion ����fe7f0101������53010003191105
        memcpy(cb, "\xfe\x7f\x01\x01", 4);
        if ((nChildRet = CntaxLogicIO(hDev->hUSB, cb, 4, sizeof(cb), NULL, 0)) < 7)
            break;
        Byte2Str(szVendorVersion, cb, nChildRet);
        sprintf(szGjbbh, "%s-%s", szTaVersion, szVendorVersion);
        Byte2Str(szVendorCode, cb, 1);
        sprintf(szOut,
                "<?xml version=\"1.0\" encoding=\"UTF-8\"?><business "
                "id=\"CHECKUPDATE\"><body><nsrsbh>%s</nsrsbh><csbs>%s</"
                "csbs><sbbh>%s</sbbh><gjbbh>%s</"
                "gjbbh><yjbbh>ABCDEF</yjbbh></body></business>",
                hDev->szCommonTaxID, szVendorCode, hDev->szDeviceID, szGjbbh);
        nRet = RET_SUCCESS;
    } while (false);
    return nRet;
}

//����豸COS���̼�ϵͳ�Ƿ���Ҫ����
int CheckFirewareUpdate(HHTTP hi)
{
    char szBuf[8192] = "";
    int nRet = ERR_PROGRAME_API;
    do {
        if ((nRet = BuildFirewareUpdateRequest(hi->hDev, szBuf)) < 0)
            break;
        if ((nRet = TaNetIo(hi, TACMD_CNTAX_RAWXML, szBuf, sizeof(szBuf))) < 0)
            break;
        // W20100ʱ���°汾
        if (!strstr(szBuf, "<returncode>W20100</returncode>")) {
            _WriteLog(LL_WARN, "Maybe need to update cntax's eos fireware");
            nRet = ERR_DEVICE_NEEDUPDATE;
        }
        nRet = RET_SUCCESS;
    } while (false);
    return nRet;
}

int CntaxFillXmlCompnanyInfo(HDEV hDev, mxml_node_t *xml_root)
{
    int nRet = RET_SUCCESS;
    char szBuf[512] = "";
    mxml_node_t *xmlNode = NULL, *xmlNode2 = NULL, *xmlNodeChild = NULL;
    xmlNode = mxmlNewElement(xml_root, "qylxxx");
    GetNatureOfTaxpayerStr(hDev->bNatureOfTaxpayer, szBuf);
    mxmlNewText(mxmlNewElement(xmlNode, "nsrxz"), 0, szBuf);
    mxmlNewText(mxmlNewElement(xmlNode, "qysj"), 0, hDev->szDeviceEffectDate);
    mxmlNewText(mxmlNewElement(xmlNode, "swjgdm"), 0, hDev->szTaxAuthorityCode);
    mxmlNewText(mxmlNewElement(xmlNode, "swjgmc"), 0, hDev->szTaxAuthorityName);
    // bmtsqybsΪ����dxqy(������ҵ)����ʵû�����ף���������ڴ������ȷʵ��
    NewByteStringToXmlNode(xmlNode, "bmtsqybs", (uint8 *)&hDev->stDevExtend.dxqy);
    //С��ģ�Կ�רƱ
    NewByteStringToXmlNode(xmlNode, "xgmzkzp", (uint8 *)&hDev->stDevExtend.xgmkjzpbs);
    //���ֳ���ҵ��ʶ
    NewByteStringToXmlNode(xmlNode, "escqybs", &hDev->stDevExtend.esjdcbs);
    //��Ʒ����ҵ����
    NewByteStringToXmlNode(xmlNode, "cpyqylx", (uint8 *)&hDev->stDevExtend.cpybs);
    //��Ʒ����ҵ��Ч��
    mxmlNewText(mxmlNewElement(xmlNode, "cpyqyyxq"), 0, hDev->stDevExtend.cpybsyxq);
    //��Ʒ�Ͱ�����
    NewByteStringToXmlNode(xmlNode, "cpybmd", (uint8 *)&hDev->stDevExtend.cpybmdbs);
    //��Ʒ�Ͱ�������Ч��
    mxmlNewText(mxmlNewElement(xmlNode, "cpybmdyxq"), 0, hDev->stDevExtend.cpybmdbsyxq);
    //������ҵ��־
    NewByteStringToXmlNode(xmlNode, "jyqybz", (uint8 *)&hDev->stDevExtend.jyqy);
    //������ҵ��Ч��
    mxmlNewText(mxmlNewElement(xmlNode, "jyqyyxq"), 0, hDev->stDevExtend.jyqyyxq);
    //��������ҵ��־
    NewByteStringToXmlNode(xmlNode, "jdcqybz", (uint8 *)&hDev->stDevExtend.jdcqy);
    //��������ҵ��Ч��
    mxmlNewText(mxmlNewElement(xmlNode, "jdcqyyxq"), 0, hDev->stDevExtend.jdcqyyxq);
    //��������������־
    NewByteStringToXmlNode(xmlNode, "jdcbmdbz", (uint8 *)&hDev->stDevExtend.jdcbmdbs);
    //��������������Ч��
    mxmlNewText(mxmlNewElement(xmlNode, "jdcbmdyxq"), 0, hDev->stDevExtend.jdcbmdyxq);
    //ί�д�����־
    NewByteStringToXmlNode(xmlNode, "wtdkbz", (uint8 *)&hDev->stDevExtend.wtdkqy);
    //ί�д�����Ч��
    mxmlNewText(mxmlNewElement(xmlNode, "wtdkyxq"), 0, hDev->stDevExtend.wtdkqyyxq);
    //�Ǽ�ע�����͡���ֹ20220813����Ʊ����˴��и�����
    //--����ı�ǩ����XML��ǩ���պϣ�Ϊ:<djzclx>000</djzcclx>��Ŀǰ�����Ȱ�����ȷ����
    mxmlNewText(mxmlNewElement(xmlNode, "djzclx"), 0, hDev->stDevExtend.djzclx);
    //С��ģרƱms��־
    NewByteStringToXmlNode(xmlNode, "xgmzpmsbz", (uint8 *)&hDev->stDevExtend.xgmjdczyms);
    //С��ģרƱms��־��Ч��
    mxmlNewText(mxmlNewElement(xmlNode, "xgmzpmsbzyxq"), 0, hDev->stDevExtend.xgmjdczymsyxqzz);
    //��˰������
    mxmlNewText(mxmlNewElement(xmlNode, "nsrmc"), 0, hDev->szCompanyName);
    //������ҵ(ϡ��)
    NewByteStringToXmlNode(xmlNode, "tzqy", (uint8 *)&hDev->stDevExtend.xtqy);
    //ũ��Ʒ��ҵ
    NewByteStringToXmlNode(xmlNode, "ncpqy", (uint8 *)&hDev->stDevExtend.ncpqy);
    //�ض���ҵ��־
    NewByteStringToXmlNode(xmlNode, "tdqybz", (uint8 *)&hDev->stDevExtend.tdqy);
    //��Ʊ��־01��ֹ��Ʊ
    NewByteStringToXmlNode(xmlNode, "kpbz", (uint8 *)&hDev->stDevExtend.kpbz);

    //��Ʊ���ͽڵ�
    xmlNode2 = mxmlNewElement(xmlNode, "fplx");
    int i = 0;
    uint8 bSupportNum = hDev->abSupportInvtype[0];
    for (i = 0; i < bSupportNum; i++) {
        HDITI invEnv = NULL;
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
        //���߿�Ʊ�޶��һ���ԣ���δ��֤��
        long double fNum =
            strtold(invEnv->monitor.lxzsljje, NULL) + strtold(invEnv->monitor.lxfsljje, NULL);
        memset(szBuf, 0, sizeof(szBuf));
        sprintf(szBuf, "%4.2Lf", fNum);  //�ɰؽ��ۣ��о��˴�������ӵó��ģ�Ӧ����lxzsljje
        mxmlNewText(mxmlNewElement(xmlNodeChild, "lxkpxe"), 0, invEnv->monitor.lxzsljje);
        //���߿�Ʊʱ��
        mxmlNewText(mxmlNewElement(xmlNodeChild, "lxkpsx"), 0, invEnv->monitor.lxkpsc);
        //���ſ�Ʊ�޶�
        mxmlNewText(mxmlNewElement(xmlNodeChild, "dzkpxe"), 0, invEnv->monitor.dzkpxe);
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
    return nRet;
}

//�˴��漰ͬ���ľ�׼�ԣ������ȡʵʱʱ�Ӻ����������������HDEV�м�¼�ľ�̬��ֵ
//���ڷ�ɢ�����õĶ���Կ��Ǽ���hdev
int CntaxFillRtcTimeAndRand(HDEV hDev, mxml_node_t *xml_root)
{
    //char szFsyz[64] = "";
    int nRet = ERR_GENERIC;
    mxml_node_t *xmlNode = mxmlNewElement(xml_root, "hqszxx");
    do {
        // if (CntaxEntry(hDev->hUSB, NOP_OPEN))
        //    break;
        // _WriteLog(LL_DEBUG, "lbc--------------test CntaxFillRtcTimeAndRand1111  hDev->szDeviceTime=%s", hDev->szDeviceTime);
        // if (CntaxGetDeviceTime(hDev->hUSB, hDev->szDeviceTime))
        //    break;
        //_WriteLog(LL_DEBUG, "lbc--------------test CntaxFillRtcTimeAndRand2222  hDev->szDeviceTime=%s", hDev->szDeviceTime);
        mxmlNewText(mxmlNewElement(xmlNode, "skpsj"), 0, hDev->szDeviceTime);
        // if (CntaxGetDeviceFsyz(hDev->hUSB, szFsyz))
        //     break;
        //strcpy(szFsyz,"1087cae9cb3bb63d4e1eb55ed209330f");
        //memcpy(szFsyz+2,hDev->szDeviceTime,strlen(hDev->szDeviceTime));
        //_WriteLog(LL_DEBUG, "lbc--------------test CntaxFillRtcTimeAndRand hDev->szFsyz=%s", hDev->szFsyz);
        mxmlNewText(mxmlNewElement(xmlNode, "fsyz"), 0, hDev->szFsyz);
        nRet = RET_SUCCESS;
    } while (false);
    //  CntaxEntry(hDev->hUSB, NOP_CLOSE);
    return nRet;
}

//������Ϣ��Ϊ�գ���Ҫ�ֶ��ͷŷ���ֵ
char *CntaxPreCompanyXml(HDEV hDev)
{
    char *pRet = NULL;
    mxml_node_t *root = NULL, *xml_root = NULL, *xmlNode = NULL;
    do {
        if (!(root = mxmlNewXML("1.0")))
            break;
        if (!(xml_root = mxmlNewElement(root, "ywxx")))
            break;
        // 1)��ҵ��������
        mxmlNewElement(xml_root, "qycsxz");
        // 2)��ҵ������Ϣ
        if (CntaxFillXmlCompnanyInfo(hDev, xml_root) < 0)
            break;
        // 3��Զ�̱����ѯ
        mxmlNewText(mxmlNewElement(mxmlNewElement(xml_root, "ycbgcx"), "bglx"), 0, "01");
        // 4��hqʱ����Ϣ
        if (CntaxFillRtcTimeAndRand(hDev, xml_root))
            break;
        // 5��˰�շ������ ssflbm,�������̫����������������
        xmlNode = mxmlNewElement(xml_root, "ssflbm");
        mxmlNewText(mxmlNewElement(xmlNode, "version"), 0, "3.0");
        mxmlNewText(mxmlNewElement(xmlNode, "bbh"), 0, DEF_BMBBBH_DEFAULT);
        mxmlNewElement(xmlNode, "ssflbm");
        mxmlNewElement(xmlNode, "ssflmc");
        // 6������ǰ˰����Ȩ��Ϣ tzqslsq
        mxmlNewElement(xml_root, "tzqslsq");
        pRet = CntaxGetUtf8YwxxStringFromXmlRoot(root);
    } while (false);
    if (root)
        mxmlDelete(root);
    return pRet;
}
