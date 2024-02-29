/*****************************************************************************
File name:   cntax_call.c
Description: ����cntaxģ���нӿڵ���ʾ��
Author:      Zako
Version:     1.0
Date:        2021.02
History:
*****************************************************************************/
#include "cntax_call.h"

int my_cntax_read_id(HDEV hDev)
{
    char szSKPBH[32] = "";
    char szTaxID[64] = "";
    char szNSRMC[256] = "";
    if (CntaxGetBasicInfo(hDev, szSKPBH, szTaxID, szNSRMC) < 0) {
        _WriteLog(LL_FATAL, "Nisec get bh|taxid|mc failed");
        return -1;
    }
    printf("----------------------------------------------------------------\n"
           "Chinatax ueky found, Device SN:%s NSRID:%s NSRName:%s\n"
           "----------------------------------------------------------------\n",
           szSKPBH, szTaxID, szNSRMC);
    return 0;
}

int my_cntax_read_taxinfo_alldisplay(HUSB hUSB)
{
    if (CntaxEntry(hUSB, NOP_OPEN_FULL) < 0)
        return -1;
    struct StMonitorInfo *stMIInf = (struct StMonitorInfo *)malloc(sizeof(struct StMonitorInfo));
    uint8 abInvoiceTypes[] = {4, 5, 6, 7, 25, 26, 28};
    int i = 0;
    for (i = 0; i < sizeof(abInvoiceTypes); i++) {
        uint8 bInvType = abInvoiceTypes[i];
        memset(stMIInf, 0, sizeof(struct StMonitorInfo));
        if (CntaxSelectInvoiceTypeMini(hUSB, bInvType) < 0)
            continue;
        int nRet = CntaxGetMonitorInfo(hUSB, stMIInf);
        if (nRet < 0)
            continue;
        printf("��Ʊ����:\t\t\t%d\n��Ʊ��ֹʱ��(kpjzsj):\t\t%s\n���ݱ�����ʼ����("
               "bsqsrq):\t%"
               "s\n���ݱ�����ֹ����(bszzrq):\t%s\n���ŷ�Ʊ��Ʊ����޶�(dzkpxe):\t%"
               "s\n�����ۼ��޶�(zsljxe):\t\t%s\n�����ۼ��޶�(fsljxe):\t\t%"
               "s\n���±�˰����(zxbsrq):\t\t%s\n"
               "�ϴ���ֹ����(scjzrq):\t\t%s\n"
               "���߿�Ʊʱ��(lxkpsc):\t\t%s\n���߿�Ʊ����(lxkpzs):\t\t%"
               "s\n���������ۼƽ��(lxzsljje):\t%s\n���߸����ۼƽ��(lxfsljje):"
               "\t%"
               "s\n",
               bInvType, stMIInf->kpjzsj, stMIInf->bsqsrq, stMIInf->bszzrq, stMIInf->dzkpxe,
               stMIInf->zsljxe, stMIInf->fsljxe, stMIInf->zxbsrq, stMIInf->scjzrq, stMIInf->lxkpsc,
               stMIInf->lxkpzs, stMIInf->lxzsljje, stMIInf->lxfsljje);
    }
    free(stMIInf);
    if (CntaxEntry(hUSB, NOP_CLOSE) < 0)
        return -2;
    return 0;
}

int my_cntax_read_buyinvinfo_alldisplay(HDEV hDev)
{
    HUSB hUSB = hDev->hUSB;
    struct StStockInfoHead *stMIInf =
        (struct StStockInfoHead *)malloc(sizeof(struct StStockInfoHead));
    int nTypeCount = hDev->abSupportInvtype[0];
    int i = 0;
    for (i = 0; i < nTypeCount; i++) {
        uint8 bInvType = hDev->abSupportInvtype[i + 1];
        memset(stMIInf, 0, sizeof(struct StStockInfoHead));
        if (CntaxEntry(hUSB, NOP_OPEN_FULL) < 0)
            break;
        if (CntaxSelectInvoiceType(hDev, bInvType, SELECT_OPTION_TEMPLATE) ||
            CntaxGetStockInfo(hDev, stMIInf)) {
            CntaxEntry(hUSB, NOP_CLOSE);
            continue;
        }
        printf("\n��Ʊ����:\t\t\t%d\n��ǰ��Ʊ����(dqfpdm):\t\t%s\n��ǰ��Ʊ����(dqfphm)"
               ":\t\t%"
               "s\n��Ʊʣ����Ŀ(zsyfs):\t\t%d\n��Ʊ����Ŀ(nGroupCount):\t%d\n"
               "-----------------------------------------------------------------\n",
               bInvType, stMIInf->dqfpdm, stMIInf->dqfphm, stMIInf->zsyfs, stMIInf->nGroupCount);
        uint8 *pArrayCount = stMIInf->pArray_StStockInfoGroup;
        int i = 0;
        for (i = 0; i < stMIInf->nGroupCount; i++) {
            struct StStockInfoGroup *pSI =
                (struct StStockInfoGroup *)(pArrayCount + i * sizeof(struct StStockInfoGroup));
            printf("���:%d\n��Ʊ����(fpdm):%s\n��ʼ����(qshm):%"
                   "s\n��ֹ����(zzhm):%s\n��Ʊ����(fpfs):%d\nʣ�����(syfs):%"
                   "d\n�칺����(lgrq):%s\n�칺��Ա(lgry):%s\n\n",
                   i + 1, pSI->fpdm, pSI->qshm, pSI->zzhm, pSI->fpfs, pSI->syfs, pSI->lgrq,
                   pSI->lgry);
        }
        printf("================================================================\n");
        if (NULL != stMIInf->pArray_StStockInfoGroup)
            free(stMIInf->pArray_StStockInfoGroup);
        if (CntaxEntry(hUSB, NOP_CLOSE) < 0)
            break;
    }
    free(stMIInf);
    return 0;
}

// input example: "20180501-20190220"
int my_cntax_read_invoice(HDEV hDev, char *szQueryDateRange)
{
    uint8 abQueryDataRange[64] = {0};
    if (GetInvMonthFormat(szQueryDateRange, abQueryDataRange)) {
        _WriteLog(LL_FATAL, "Input dataformat error");
        return -1;
    }
    int nTypeCount = hDev->abSupportInvtype[0];
    int i = 0, nRet = -10;
    uint8 *pRetBuff = NULL;
    for (i = 0; i < nTypeCount; i++) {
        uint8 bInvType = hDev->abSupportInvtype[i + 1];
        if (CntaxEntry(hDev->hUSB, NOP_OPEN_FULL) < 0)
            return -1;
        if (CntaxSelectInvoiceType(hDev, bInvType, SELECT_OPTION_TEMPLATE) < 0) {
            if (CntaxEntry(hDev->hUSB, NOP_CLOSE) < 0)
                return -2;
            return -3;
        }
        int nRetBuffLen = 0;
        int nChildRet = CntaxGetInvBuff(hDev->hUSB, FPCX_COMMON_MONTH, &pRetBuff, &nRetBuffLen,
                                        abQueryDataRange, 8, NULL);
        CntaxEntry(hDev->hUSB, NOP_CLOSE);
        if (nChildRet < 0)
            goto NextFPType;
        _WriteHexToDebugFile("cntax_usbraw.bin", pRetBuff, nChildRet);
        nRet = CntaxGetInvDetail(hDev, FPCX_COMMON_MONTH, pRetBuff, nChildRet, NULL);
    NextFPType:
        if (pRetBuff) {
            free(pRetBuff);
            pRetBuff = NULL;
        }
    }
    return nRet;
}

//��Ʊ���
int my_cntax_make_invoice(HDEV hDev)
{
    int nRet = 0;
    HFPXX fpxx = MallocFpxxLinkDev(hDev);
    if (fpxx == NULL) {
        nRet = -1;
        goto FreeAndExit;
    }
    //����json config
    fpxx->isHzxxb = false;
    if (ImportJSONToFpxx(fpxx, hDev) < 0) {
        _WriteLog(LL_FATAL, "ImportJSONToFpxx failed");
        nRet = -2;
        goto FreeAndExit;
    }
    if ((nRet = CntaxMakeInvoiceMain(fpxx)) < 0) {
        _WriteLog(LL_FATAL, "MakeInvoiceMain failed!,nRet = %d", nRet);
        nRet = -3;
        goto FreeAndExit;
    }

    nRet = 0;
FreeAndExit:
    FreeFpxx(fpxx);
    return nRet;
}

//ע��!!!���ǵ����ӿ�,ֻ�ܴ�USB handle�󵥶�����.�������������̣���Ϊ�޸���֮����Ҫ���������豸
//֤�������޸�һ�㲻���ã���Ϊ֤�������ѱ���Ʊ������ԣ��޸�֤��Ľӿڽ����ڿͻ�ͳһ��������ʹ��
//һ��ʹ��bDeviceOrCert=true
int my_cntax_change_password(HDEV hDev, bool bDeviceOrCert, char *szOldPassword,
                             char *szNewPassword)
{
    //����cntax��20210202�汾֮��ǿ��Ҫ��ͳһ֤����豸�����˵��
    //������ָ���汾֮���豸����ʱ��֤�豸��֤������Ƿ�һ�£������һ����ǿ��Ҫ���û��޸�����
    //�޸�ͳһ��������2��������Ŀ򣬷ֱ�Ϊ��֤������豸���룬ʵ����֤������������Ǳ����Եģ�ֻ�����豸��������Ч��
    // cntax��ǿ�ƽ�֤�������Ϊ���豸����һ�£������豸������Ϊ��ʵ�豸���룬������Իͳһ�豸����

    _WriteLog(LL_INFO, "Try to set new cert's password:%s", szNewPassword);
    int nNewPwdLen = strlen(szNewPassword);
    if (nNewPwdLen < 6 || nNewPwdLen > 8) {
        _WriteLog(LL_FATAL, "New cert password's length must [6,8]");
        return -1;
    }
    if (!strcmp(szNewPassword, DEF_DEVICE_PASSWORD_DEFAULT)) {
        _WriteLog(LL_FATAL, "Can not set new password equal with defalt password '88888888'");
        return -2;
    }
    //ʵ������Ϊ0-9�����ж��¾���
    if (!CheckXDigit(szNewPassword, strlen(szNewPassword))) {
        _WriteLog(LL_FATAL, "New password must be [0-9]");
        return -2;
    }
    if (CheckHeadTailSpace(szNewPassword)) {
        _WriteLog(LL_FATAL, "Head and tail cannot been 0x20");
        return -3;
    }
    if (bDeviceOrCert) {
    } else {
        if (EncryptOpenDevice(hDev->hUSB, hDev->bDeviceType, hDev->szDeviceID) < 0) {
            _WriteLog(LL_FATAL, "EncryptOpenDevice failed");
            return -1;
        }
        if (CntaxChangeCertPassword(hDev->hUSB, hDev->szDeviceID, szOldPassword, szNewPassword) <
            0) {
            _WriteLog(LL_FATAL, "Crypt login failed, cannot modify new password");
            return -1;
        }
        _WriteLog(LL_INFO, "Cert's password has been set,old:[%s] -> new:[%s]", szOldPassword,
                  szNewPassword);
    }
    return 0;
}

//��demo��ͳ�Ƶ��£������Զ�����ÿ���ֱ�ӵ��ö�ӦAPI
int my_cntax_get_Statistics(HDEV hDev)
{
    HUSB hUSB = hDev->hUSB;
    struct StMonthStatistics st;
    int nTypeCount = hDev->abSupportInvtype[0];
    int i = 0;
    for (i = 0; i < nTypeCount; i++) {
        uint8 bInvType = hDev->abSupportInvtype[i + 1];
        struct StMonthStatistics *nextNode = NULL;
        if (CntaxEntry(hUSB, NOP_OPEN_FULL) < 0)
            return -1;
        if (CntaxSelectInvoiceType(hDev, bInvType, SELECT_OPTION_TEMPLATE) < 0)
            goto Next;
        memset(&st, 0, sizeof(struct StMonthStatistics));
        if (CntaxGetMonthStatistics(hDev->hUSB, false, &st) < 0) {
            _WriteLog(LL_WARN, "GetMonthStatistics failed, fplx:%d", bInvType);
        }
        nextNode = st.next;
        while (nextNode) {
            struct StMonthStatistics *tmp = nextNode->next;
            free(nextNode);
            nextNode = tmp;
        }
    Next:
        if (CntaxEntry(hUSB, NOP_CLOSE) < 0)
            break;
    }
    return 0;
}

//<0�����账������ֵ==0,������Ҫ�ϱ���Ʊ�������ϱ�ʧ�ܣ�>0 ����һ�ţ��ϱ����
int my_cntax_read_upload_invoice(HDEV hDev)
{
    uint8 abInvoiceTypes[] = {4, 7, 26, 28};
    int i = 0, nRet = 0, nChildRet = 0;
    uint8 bInvType = 0;
    char szCurFPDM[64] = "", szCurFPHM[64] = "";
    //ÿһ�ֽӿڶ�ֻ����һ�ţ�����ÿһ���ӿڶ�����һ��
    for (i = 0; i < sizeof(abInvoiceTypes); i++) {
        bInvType = abInvoiceTypes[i];
        if (!CheckDeviceSupportInvType(hDev->abSupportInvtype, bInvType))
            continue;
        //�ȴ������ϴ������֣����ϡ�ǩ��ʧ�ܵ�ԭ��ķ�Ʊ��
        if ((nChildRet = CntaxQueryModifyInvoice(bInvType, hDev, szCurFPDM, szCurFPHM)) < 0)
            break;
        if (nChildRet > 0)
            nRet++;
        //���û������Ʊ����������̼��δ�ϴ���Ʊ
        if ((nChildRet = CntaxQueryOfflineInvoice(bInvType, hDev)) < 0)
            break;
        if (nChildRet > 0)
            nRet++;
        //�ٷ��˴����нϵ͵ĸ���ʹ��FPCX_COMMON_LASTINV��ȡ��һ�ŷ�Ʊ�������һ�ŷ�Ʊ�Ƿ�δ�ϴ�
        //��Ϊ�漰�϶��IO��Ŀǰ�ݲ�����
        //����������ַ�Ʊ��ʹ��ȡlast��Ʊ�ж��Ƿ���bIsUpload��־λ�����ٴ��ϴ�

        //��������Ʊ��
    }
    return nRet;
}

// ukey���Ʋ�֧��ָ����Ʊ�ϱ����ýӿڷ���
// int my_cntax_upload_invoice_condition(HDEV hDev, uint8 bInvType, char *szFPDM, char *szFPHM)

// 1= yccb �ϱ�����, 0 = qk
//wang 04 03 ȡ�������
#if CntaxReport 
int my_cntax_report_clear(HDEV hDev, uint8 bReportOrClear)
{
    ClearLastError(hDev->hUSB);
    if (hDev->bTrainFPDiskType != 0) {
        _WriteLog(LL_WARN, "����ʽ�̲�ִ�г����忨����");
        return -1;
    }
    int nTypeCount = hDev->abSupportInvtype[0];
    int i = 0, nRet = 0;
    for (i = 0; i < nTypeCount; i++) {
        uint8 bCommon_FPLX = hDev->abSupportInvtype[i + 1];
        // cntax�����忨���ƽ�Ϊ�򵥣�Ĭ��֧������Ʊ�ֳ����忨����һ�����̣���������Ʊ����δ���Թ���ֻ������4��
        //ע�⣬��Ҫȫ��Ʊ�ֶ������������忨�����򵥸�Ʊ�ֳ������忨˰�ֻ���ʾδ����
        if (FPLX_COMMON_DZFP != bCommon_FPLX && FPLX_COMMON_ZYFP != bCommon_FPLX &&
            FPLX_COMMON_PTFP != bCommon_FPLX && FPLX_COMMON_DZZP != bCommon_FPLX) {
            _WriteLog(LL_WARN, "Unsupport invoice's common-fplx:%d", bCommon_FPLX);
            continue;
        }
        _WriteLog(LL_INFO, "Begin %s, fplx:%d", bReportOrClear ? "RemoteReport" : "ClearCard",
                  bCommon_FPLX);
        if (bReportOrClear) {
            _WriteLog(LL_INFO, "Begin cntax reportsummary");
            if (CntaxReportSummarytMain(hDev, bCommon_FPLX) < 0) {
                continue;
            }
        } else {
            _WriteLog(LL_INFO, "Begin cntax clearcard");
            if (CntaxClearCardMain(hDev, bCommon_FPLX) < 0) {
                continue;
            }
        }
    }
    return nRet;
}
#endif

int my_cntax_redinv_operate(HDEV hDev)
{
    uint8 bFuncNo = 1;  // 0 == upload;1==query;2==cancel
    int nRet = -10, nChildRet = -1;
    int nBufSize = DEF_MAX_FPBUF_LEN;
    char *szBuf = calloc(1, nBufSize);
    HFPXX fpxx = MallocFpxxLinkDev(hDev);
    if (!fpxx || !szBuf) {
        nRet = -1;
        goto FreeAndExit;
    }
    if (0 == bFuncNo) {
        char szTZDBH[36] = "";
        //����KP�ĵ���
        fpxx->isHzxxb = true;
        if (ImportJSONToFpxx(fpxx, hDev) < 0) {
            _WriteLog(LL_FATAL, "ImportJSONToFpxx failed");
            nRet = -2;
            goto FreeAndExit;
        }
        if (UploadTZDBH(fpxx, szTZDBH) < 0) {
            _WriteLog(LL_FATAL, "UploadTZDBH failed,result:%s!",
                      hDev->hUSB->szLastErrorDescription);
            nRet = -3;
            goto FreeAndExit;
        }
        _WriteLog(LL_INFO, "UploadTZDBH successful,  red invoice information:%s", szTZDBH);
    } else if (1 == bFuncNo) {
        if (DownloadTZDBH(fpxx->hDev, "20211120-20211230", "", "", szBuf, nBufSize) < 0) {
            // if (DownloadTZDBH(hDev, "", "", "3201122111012345", szBuf, nBufSize) < 0) {
            _WriteLog(LL_FATAL, "DownloadTZDBH failed,result:%s!",
                      hDev->hUSB->szLastErrorDescription);
            nRet = -3;
            goto FreeAndExit;
        }
        _WriteLog(LL_INFO, "%s", szBuf);
    } else if (2 == bFuncNo) {
        char szTZDBH[] = "3201122111012345";
        nChildRet = RedinvTZDCancel(hDev, szTZDBH, szBuf, nBufSize);
        _WriteLog(LL_INFO, "RedinvTZDCancel:%s TZDBH:%s retmsg:%s", nChildRet == 0 ? "OK" : "Fail",
                  szTZDBH, szBuf);
    }
    nRet = 0;
FreeAndExit:
    if (szBuf)
        free(szBuf);
    if (fpxx)
        FreeFpxx(fpxx);
    return nRet;
}
int cntax_get_state_info(HDEV hDev, struct _plate_infos *plate_info)
{
	if (CntaxEntry(hDev->hUSB, NOP_OPEN_FULL) < 0)
		return -1;
	int j = 0;
	struct StMonitorInfo *stMIInf = (struct StMonitorInfo *)malloc(sizeof(struct StMonitorInfo));
	uint8_t abInvoiceTypes[] = { 4, 5, 6, 7, 25, 26 ,28};
	int i = 0;
	for (i = 0; i < sizeof(abInvoiceTypes); i++) {

		uint8 bInvType = abInvoiceTypes[i];

		if (!CheckDeviceSupportInvType(hDev->abSupportInvtype, bInvType))
			continue;  // not support

		memset(stMIInf, 0, sizeof(struct StMonitorInfo));
		if (CntaxSelectInvoiceType(hDev, bInvType, SELECT_OPTION_TEMPLATE) < 0)
			continue;

		int nRet = CntaxGetMonitorInfo(hDev->hUSB, stMIInf);
		if (nRet < 0)
			continue;

		plate_info->plate_tax_info.inv_type[j] = bInvType;
		j++;

		int mem_i;
		find_inv_type_mem(bInvType, &mem_i);

		//out("��Ʊ����%3d���ڴ����%d\n", bInvType, mem_i);

		plate_info->invoice_type_infos[mem_i].state = 1;
		plate_info->invoice_type_infos[mem_i].fplxdm = abInvoiceTypes[i];
		plate_info->invoice_type_infos[mem_i].monitor_info.state = 1;

		uint8 zxbsrq[20] = { 0 };
		uint8 tmp_t[8] = { 0 };
		//out("���±�˰���ڸ���ǰ��%s\n", stMIInf->zxbsrq);
		memset(zxbsrq, 0, sizeof(zxbsrq));
		memcpy(zxbsrq, stMIInf->zxbsrq, 8);
		asc2bcd(zxbsrq, tmp_t, 12);
		next_date(tmp_t);
		memset(zxbsrq, 0, sizeof(zxbsrq));
		bcd2asc(tmp_t, zxbsrq, 6);
		//out("���±�˰���ڸ��ĺ�%s\n", zxbsrq);
		memcpy(plate_info->invoice_type_infos[mem_i].monitor_info.zxbsrq, zxbsrq, 8);

		memcpy(plate_info->invoice_type_infos[mem_i].monitor_info.bsqsrq, stMIInf->bsqsrq, strlen(stMIInf->bsqsrq));
		memcpy(plate_info->invoice_type_infos[mem_i].monitor_info.bszzrq, stMIInf->bszzrq, strlen(stMIInf->bszzrq));
		//memcpy(state->port_info.tax_infos[i].zxbsrq, stMIInf.zxbsrq, sizeof(state->port_info.tax_infos[i].zxbsrq));
		memcpy(plate_info->invoice_type_infos[mem_i].monitor_info.kpjzsj, stMIInf->kpjzsj, strlen(stMIInf->kpjzsj));
		memcpy(plate_info->invoice_type_infos[mem_i].monitor_info.dzkpxe, stMIInf->dzkpxe, strlen(stMIInf->dzkpxe));
		memcpy(plate_info->invoice_type_infos[mem_i].monitor_info.lxzsljje, stMIInf->lxzsljje, strlen(stMIInf->lxzsljje));
		plate_info->invoice_type_infos[mem_i].monitor_info.lxkpsc = atoi(stMIInf->lxkpsc);

		if ((memcmp(hDev->szDeviceTime, plate_info->invoice_type_infos[mem_i].monitor_info.zxbsrq, 6) > 0) && \
			(memcmp(hDev->szDeviceTime, plate_info->invoice_type_infos[mem_i].monitor_info.kpjzsj, 6) >= 0))//��ǰ�´������±�˰������,��ҪС�ڿ�Ʊ��ֹ����,��Ҫ��˰����
		{
			sprintf((char *)plate_info->invoice_type_infos[mem_i].monitor_info.cbqkzt, "0");
		}
		else
		{
			sprintf((char *)plate_info->invoice_type_infos[mem_i].monitor_info.cbqkzt, "1");
			if (memcmp(hDev->szDeviceTime, plate_info->invoice_type_infos[mem_i].monitor_info.kpjzsj, 6) >= 0)
			{
				sprintf((char *)plate_info->invoice_type_infos[mem_i].monitor_info.cbqkzt, "1");
			}
			else
			{
				sprintf((char *)plate_info->invoice_type_infos[mem_i].monitor_info.cbqkzt, "2");
			}
		}



	}
	free(stMIInf);
	CntaxEntry(hDev->hUSB, NOP_CLOSE);
	plate_info->plate_tax_info.inv_type_num = j;	
	//_WriteLog(LL_INFO,"cntax_get_state_info Ʊ��������%d\n", plate_info->plate_tax_info.inv_type_num);
	return plate_info->plate_tax_info.inv_type_num;
}

int cntax_read_inv_coil_number(HDEV hDev, struct _plate_infos *plate_info)
{
	HUSB hUSB = hDev->hUSB;
	struct StStockInfoHead *stMIInf =
		(struct StStockInfoHead *)malloc(sizeof(struct StStockInfoHead));
	uint8_t abInvoiceTypes[] = { 4, 5, 6, 7, 25, 26 ,28};
	//int i = 0;
	int type_count;
	int coil_count;
	if (CntaxEntry(hUSB, NOP_OPEN_FULL) < 0)
	{	
		free(stMIInf);
		return -1;
	}
	for (type_count = 0; type_count < sizeof(abInvoiceTypes); type_count++) {
		uint8_t bInvType = abInvoiceTypes[type_count];
		if (!CheckDeviceSupportInvType(hDev->abSupportInvtype, bInvType))
			continue;  // not support

		if (CntaxSelectInvoiceType(hDev, bInvType, SELECT_OPTION_TEMPLATE | SELECT_OPTION_MONITOR | SELECT_OPTION_TIME) < 0)
			continue;
		memset(stMIInf, 0, sizeof(struct StStockInfoHead));
		int nRet = CntaxGetStockInfo(hDev, stMIInf);
		if (nRet < 0) {
			//printf("GetStockInfo-type:%d nRet:%d\n", bInvType, nRet);
			continue;
		}
		int mem_i;
		find_inv_type_mem(bInvType, &mem_i);

		//_WriteLog(LL_INFO,"��Ʊ����%3d���ڴ����%d\n", bInvType, mem_i);
		//printf("\n��Ʊ����:\t\t\t%d\n��ǰ��Ʊ����(dqfpdm):\t\t%s\n��ǰ��Ʊ����(dqfphm)"
		//	":\t\t%"
		//	"s\n��Ʊʣ����Ŀ(zsyfs):\t\t%d\n���ط�Ʊ��Ŀ(nGroupCount):\t%d\n"
		//	"-----------------------------------------------------------------\n",
		//	bInvType, stMIInf->dqfpdm, stMIInf->dqfphm, stMIInf->zsyfs, stMIInf->nGroupCount);
		plate_info->invoice_type_infos[mem_i].invoice_coils_info.state = 1;
		strcpy((char *)plate_info->invoice_type_infos[mem_i].invoice_coils_info.dqfpdm, stMIInf->dqfpdm);
		plate_info->invoice_type_infos[mem_i].invoice_coils_info.dqfphm = atoi(stMIInf->dqfphm);
		plate_info->invoice_type_infos[mem_i].invoice_coils_info.zsyfpfs = stMIInf->zsyfs;

		uint8 *pArrayCount = stMIInf->pArray_StStockInfoGroup;
		int count = 0;
		for (coil_count = 0; coil_count < stMIInf->nGroupCount; coil_count++) {
			struct StStockInfoGroup *pSI =
				(struct StStockInfoGroup *)(pArrayCount + coil_count * sizeof(struct StStockInfoGroup));
			/*printf("���:%d\n��Ʊ����(fpdm):%s\n��ʼ����(qshm):%"
			"s\n��ֹ����(zzhm):%s\n��Ʊ����(fpfs):%d\nʣ�����(syfs):%"
			"d\n�칺����(lgrq):%s\n�칺��Ա(lgry):%s\n\n",
			count + 1, pSI->fpdm, pSI->qshm, pSI->zzhm, pSI->fpfs, pSI->syfs, pSI->lgrq,
			pSI->lgry);*/
			if (pSI->syfs == 0)//��ʹ����ķ�Ʊ��
				continue;
			//_WriteLog(LL_INFO, "���:%d\n", coil_count + 1);
			//_WriteLog(LL_INFO, "��Ʊ����(fpdm):%s\n", pSI->fpdm);
			//_WriteLog(LL_INFO, "��ʼ����(qshm):%s\n", pSI->qshm);
			//_WriteLog(LL_INFO, "��ֹ����(zzhm):%s\n", pSI->zzhm);
			//_WriteLog(LL_INFO, "��Ʊ����(fpfs):%d\n", pSI->fpfs);
			//_WriteLog(LL_INFO, "ʣ�����(syfs):%d\n", pSI->syfs);
			//_WriteLog(LL_INFO, "�칺����(lgrq):%s\n", pSI->lgrq);
			//_WriteLog(LL_INFO, "�칺��Ա(lgry):%s\n", pSI->lgry);
			plate_info->invoice_type_infos[mem_i].invoice_coils_info.invoice_coil[count].state = 1;
			plate_info->invoice_type_infos[mem_i].invoice_coils_info.invoice_coil[count].fpsyfs = pSI->syfs;
			plate_info->invoice_type_infos[mem_i].invoice_coils_info.invoice_coil[count].fpgmsl = pSI->fpfs;


			sprintf((char *)plate_info->invoice_type_infos[mem_i].invoice_coils_info.invoice_coil[count].fplbdm, "%s", pSI->fpdm);
			plate_info->invoice_type_infos[mem_i].invoice_coils_info.invoice_coil[count].fpqshm = atoi(pSI->qshm);
			plate_info->invoice_type_infos[mem_i].invoice_coils_info.invoice_coil[count].fpzzhm = plate_info->invoice_type_infos[mem_i].invoice_coils_info.invoice_coil[count].fpqshm + pSI->fpfs - 1;
			sprintf((char *)plate_info->invoice_type_infos[mem_i].invoice_coils_info.invoice_coil[count].fpgmsj, "%s", pSI->lgrq);
			count += 1;
			if (count >= DF_MAX_INV_COILS)
				break;
		}
		if (coil_count > DF_MAX_INV_COILS)
			coil_count = DF_MAX_INV_COILS;
		plate_info->invoice_type_infos[mem_i].invoice_coils_info.wsyfpjsl = count;
		//out("�÷�Ʊ��������з�Ʊ���Ѷ���,��Ʊ��������%d\n", plate_info->invoice_type_infos[mem_i].invoice_coils_info.wsyfpjsl);
		//printf("================================================================\n");
		if (NULL != stMIInf->pArray_StStockInfoGroup)
			free(stMIInf->pArray_StStockInfoGroup);
	}

	free(stMIInf);
	if (CntaxEntry(hUSB, NOP_CLOSE) < 0)
		return -2;
	return 0;

}

int cntax_get_state_info_and_coil_info(HDEV hDev, HUSB hUSB, struct _plate_infos *plate_info)
{
	int j = 0;
	struct StMonitorInfo *stateInf = (struct StMonitorInfo *)malloc(sizeof(struct StMonitorInfo));
	struct StStockInfoHead *stMIInf = (struct StStockInfoHead *)malloc(sizeof(struct StStockInfoHead));
	uint8_t abInvoiceTypes[] = { 4, 5, 6, 7, 25, 26 };
	//int nRet;
	int type_count;
	int coil_count;
	if (CntaxEntry(hUSB, NOP_OPEN_FULL) < 0)
		return -1;
	for (type_count = 0; type_count < sizeof(abInvoiceTypes); type_count++) {

		uint8 bInvType = abInvoiceTypes[type_count];
		memset(stateInf, 0, sizeof(struct StMonitorInfo));
		memset(stMIInf, 0, sizeof(struct StStockInfoHead));
		if (CntaxSelectInvoiceType(hDev, bInvType, SELECT_OPTION_TEMPLATE) < 0)
			continue;
		int nRet = CntaxGetMonitorInfo(hUSB, stateInf);
		if (nRet < 0)
		{
			_WriteLog(LL_INFO,"GetMonitorInfo < 0 bInvType = %d\n", bInvType);
			continue;
		}


		plate_info->plate_tax_info.inv_type[j] = bInvType;
		j++;
		int mem_i;
		find_inv_type_mem(bInvType, &mem_i);

		plate_info->invoice_type_infos[mem_i].state = 1;
		plate_info->invoice_type_infos[mem_i].fplxdm = bInvType;
		plate_info->invoice_type_infos[mem_i].monitor_info.state = 1;
		uint8 zxbsrq[20] = { 0 };
		uint8 tmp_t[8] = { 0 };
		memset(zxbsrq, 0, sizeof(zxbsrq));
		memcpy(zxbsrq, stateInf->zxbsrq, 8);
		asc2bcd(zxbsrq, tmp_t, 12);
		next_date(tmp_t);
		memset(zxbsrq, 0, sizeof(zxbsrq));
		bcd2asc(tmp_t, zxbsrq, 6);
		memcpy(plate_info->invoice_type_infos[mem_i].monitor_info.zxbsrq, zxbsrq, 8);
		memcpy(plate_info->invoice_type_infos[mem_i].monitor_info.bsqsrq, stateInf->bsqsrq, strlen(stateInf->bsqsrq));
		memcpy(plate_info->invoice_type_infos[mem_i].monitor_info.bszzrq, stateInf->bszzrq, strlen(stateInf->bszzrq));
		memcpy(plate_info->invoice_type_infos[mem_i].monitor_info.kpjzsj, stateInf->kpjzsj, strlen(stateInf->kpjzsj));
		memcpy(plate_info->invoice_type_infos[mem_i].monitor_info.dzkpxe, stateInf->dzkpxe, strlen(stateInf->dzkpxe));
		memcpy(plate_info->invoice_type_infos[mem_i].monitor_info.lxzsljje, stateInf->lxzsljje, strlen(stateInf->lxzsljje));
		plate_info->invoice_type_infos[mem_i].monitor_info.lxkpsc = atoi(stateInf->lxkpsc);

		if ((memcmp(hDev->szDeviceTime, plate_info->invoice_type_infos[mem_i].monitor_info.zxbsrq, 6) > 0) && \
			(memcmp(hDev->szDeviceTime, plate_info->invoice_type_infos[mem_i].monitor_info.kpjzsj, 6) >= 0))//��ǰ�´������±�˰������,��ҪС�ڿ�Ʊ��ֹ����,��Ҫ��˰����
		{
			sprintf((char *)plate_info->invoice_type_infos[mem_i].monitor_info.cbqkzt, "0");
		}
		else
		{
			sprintf((char *)plate_info->invoice_type_infos[mem_i].monitor_info.cbqkzt, "1");
			if (memcmp(hDev->szDeviceTime, plate_info->invoice_type_infos[mem_i].monitor_info.kpjzsj, 6) >= 0)
			{
				sprintf((char *)plate_info->invoice_type_infos[mem_i].monitor_info.cbqkzt, "1");
			}
			else
			{
				sprintf((char *)plate_info->invoice_type_infos[mem_i].monitor_info.cbqkzt, "2");
			}
		}
		_WriteLog(LL_INFO, "test 1\n");
		nRet = CntaxGetStockInfo(hDev, stMIInf);
		if (nRet < 0) {
			//printf("GetStockInfo-type:%d nRet:%d\n", bInvType, nRet);
			continue;
		}

		_WriteLog(LL_INFO, "\n��Ʊ����:\t\t\t%d\n��ǰ��Ʊ����(dqfpdm):\t\t%s\n��ǰ��Ʊ����(dqfphm)"
			":\t\t%"
			"s\n��Ʊʣ����Ŀ(zsyfs):\t\t%d\n��Ʊ����Ŀ(nGroupCount):\t%d\n"
			"-----------------------------------------------------------------\n",
			bInvType, stMIInf->dqfpdm, stMIInf->dqfphm, stMIInf->zsyfs, stMIInf->nGroupCount);
		plate_info->invoice_type_infos[mem_i].invoice_coils_info.state = 1;

		//if (bInvType == FPLX_COMMON_ZYFP)
		//	sprintf((char *)plate_info->invoice_type_infos[mem_i].invoice_coils_info.dqfpdm, "%010llu", atoll(stMIInf->dqfpdm));
		//else
		//	sprintf((char *)plate_info->invoice_type_infos[mem_i].invoice_coils_info.dqfpdm, "%012llu", atoll(stMIInf->dqfpdm));
		strcpy((char *)plate_info->invoice_type_infos[mem_i].invoice_coils_info.dqfpdm, stMIInf->dqfpdm);
		//sprintf((char *)plate_info->invoice_type_infos[type_count].invoice_coils_info.dqfphm, "%08llu", atoll(coilInf->dqfphm));
		//sprintf((char *)plate_info->invoice_type_infos[type_count].invoice_coils_info.dqfpdm, "%s", coilInf->dqfpdm);

		plate_info->invoice_type_infos[mem_i].invoice_coils_info.dqfphm = atoi(stMIInf->dqfphm);
		//sprintf((char *)plate_info->invoice_type_infos[i].invoice_coils_info.dqfphm, "%s", coilInf->dqfphm);
		plate_info->invoice_type_infos[mem_i].invoice_coils_info.zsyfpfs = stMIInf->zsyfs;

		uint8_t *pArrayCount = stMIInf->pArray_StStockInfoGroup;
		int count = 0;
		for (coil_count = 0; coil_count < stMIInf->nGroupCount; coil_count++) {
			struct StStockInfoGroup *pSI =
				(struct StStockInfoGroup *)(pArrayCount + coil_count * sizeof(struct StStockInfoGroup));
			_WriteLog(LL_INFO, "���:%d\n��Ʊ����(fpdm):%s\n��ʼ����(qshm):%"
				"s\n��ֹ����(zzhm):%s\n��Ʊ����(fpfs):%d\nʣ�����(syfs):%"
				"d\n�칺����(lgrq):%s\n�칺��Ա(lgry):%s\n\n",
				coil_count + 1, pSI->fpdm, pSI->qshm, pSI->zzhm, pSI->fpfs, pSI->syfs, pSI->lgrq,
				pSI->lgry);
			/*printf("���:%d\n��Ʊ����(fpdm):%s\n��ʼ����(qshm):%"
			"s\n��ֹ����(zzhm):%s\n��Ʊ����(fpfs):%d\nʣ�����(syfs):%"
			"d\n�칺����(lgrq):%s\n�칺��Ա(lgry):%s\n\n",
			count + 1, pSI->fpdm, pSI->qshm, pSI->zzhm, pSI->fpfs, pSI->syfs, pSI->lgrq,
			pSI->lgry);*/
			if (pSI->syfs == 0)//��ʹ����ķ�Ʊ��
				continue;
			//out("���:%d\n", coil_count + 1);
			//out("��Ʊ����(fpdm):%s\n", pSI->fpdm);
			//out("��ʼ����(qshm):%s\n", pSI->qshm);
			//out("��ֹ����(zzhm):%s\n", pSI->zzhm);
			//out("��Ʊ����(fpfs):%d\n", pSI->fpfs);
			//out("ʣ�����(syfs):%d\n", pSI->syfs);
			//out("�칺����(lgrq):%s\n", pSI->lgrq);
			//out("�칺��Ա(lgry):%s\n", pSI->lgry);
			plate_info->invoice_type_infos[mem_i].invoice_coils_info.invoice_coil[count].state = 1;
			plate_info->invoice_type_infos[mem_i].invoice_coils_info.invoice_coil[count].fpsyfs = pSI->syfs;
			plate_info->invoice_type_infos[mem_i].invoice_coils_info.invoice_coil[count].fpgmsl = pSI->fpfs;


			sprintf((char *)plate_info->invoice_type_infos[mem_i].invoice_coils_info.invoice_coil[count].fplbdm, "%s", pSI->fpdm);
			plate_info->invoice_type_infos[mem_i].invoice_coils_info.invoice_coil[count].fpqshm = atoi(pSI->qshm);
			plate_info->invoice_type_infos[mem_i].invoice_coils_info.invoice_coil[count].fpzzhm = plate_info->invoice_type_infos[mem_i].invoice_coils_info.invoice_coil[count].fpqshm + pSI->fpfs - 1;
			sprintf((char *)plate_info->invoice_type_infos[mem_i].invoice_coils_info.invoice_coil[count].fpgmsj, "%s", pSI->lgrq);
			count += 1;
			if (count >= DF_MAX_INV_COILS)
				break;
		}
		if (coil_count > DF_MAX_INV_COILS)
			coil_count = DF_MAX_INV_COILS;
		plate_info->invoice_type_infos[mem_i].invoice_coils_info.wsyfpjsl = count;
		//out("�÷�Ʊ��������з�Ʊ���Ѷ���\n");
		//printf("================================================================\n");
		if (NULL != stMIInf->pArray_StStockInfoGroup)
			free(stMIInf->pArray_StStockInfoGroup);
	}
	plate_info->plate_tax_info.inv_type_num = j;
	free(stMIInf);
	free(stateInf);
	CntaxEntry(hDev->hUSB, NOP_CLOSE);

	return 0;
}
#ifdef DF_OLD_MSERVER
static int cntax_get_invoice_month_upload_hook(HDEV hDev, uint8 *month, void *function, void *arg, struct _upload_inv_num *upload_inv_num, unsigned short *start_stop, struct _inv_sum_data *now_month_sum)
{
	char errinfo[1024] = { 0 };
	int result;
	char date_range[20] = { 0 };
	long index = 0;
	unsigned char *send_data;
	char *encode_data = NULL;
	int now_count = 0;
	int hook_count = 0;
	long begin_time;
	long now_time;
	long inv_len;
	int inv_count = 0;
	int up_num = 0;
	int max_len = 1 * 1024 * 1024;//������ݳ���1M���ص�����
	int over_time = 5;//��Ʊ���ݻ�ȡʱ�䳬��5�뼴�ص�����
	unsigned char *lxp_data;
	char today_month[10] = { 0 };
	char *year_month_data = NULL;
	int use_http_data = 0;
	void *hook_arg;
	int(*hook)(unsigned char *s_data, int total_num, int now_num, void *hook_arg, char *plate_num, uint8 *month, char *statistics, int complete_err);
	struct _inv_sum_data inv_sum;
	memset(&inv_sum, 0, sizeof(struct _inv_sum_data));
	mb_get_today_month(today_month);
	hook = function;
	hook_arg = arg;
	char stand_time[100] = { 0 };
	result = cntax_read_tax_time(hDev, stand_time);
	if (result < 0)
	{
		_WriteLog(LL_INFO,"˰��UKEY��ȡ˰��ʱ��ʧ��\n");
		return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
	}

	char least_month[10] = { 0 };
	memcpy(least_month, hDev->szDeviceEffectDate, 6);
	if (strcmp((char *)month, least_month) < 0)
	{
		result = hook(NULL, DF_INV_MONTH_COMPLETE, 0, hook_arg, (char *)hDev->szDeviceID, month, NULL, 0);
		if (result < 0)
		{
			_WriteLog(LL_INFO, "%s˰�̻ص�ͬ���������ݳ�ʱ\n", hDev->szCompanyName);
			return DF_TAX_ERR_CODE_HOOK_SERVER_TIME_OUT;
		}
		return 0;
	}
	if ((strcmp(upload_inv_num->year_month, today_month) == 0) && upload_inv_num->now_num > 0 && strlen(upload_inv_num->year_month_data) == 0)
	{

		_WriteLog(LL_INFO, "%s˰�̵���%s��Ʊ������%d��,���ܴ��ڲ�һ������,���ѯ�������ٶ���\n", hDev->szCompanyName, month, upload_inv_num->now_num);
		result = get_plate_invs_sync(hDev->szDeviceID, &year_month_data);
		if (result < 0)
		{
			use_http_data = 0;
			_WriteLog(LL_INFO, "%s˰�̵���%s��Ʊ������%d��,���ܴ��ڲ�һ������,���������ȡ����ʧ��\n", hDev->szCompanyName, month, upload_inv_num->now_num);
		}
		else
			use_http_data = 1;

	}
	send_data = (uint8 *)malloc(max_len * 10 * 1); //��Ʊ���ݻ�����10M
	memset(send_data, 0, max_len * 10);
	lxp_data = (uint8 *)malloc(max_len);
	memset(lxp_data, 0, max_len);

	turn_month_to_range((char *)month, date_range);
	//out("ת���·ݳ�ʱ��� = %s\n", date_range);

	uint8 abQueryDataRange[8] = { 0 };
	if (GetInvMonthFormat(date_range, abQueryDataRange))
	{
		_WriteLog(LL_FATAL, "Input dataformat error");
		if (year_month_data != NULL)
			free(year_month_data);
		free(send_data);
		free(lxp_data);
		return -1;
	}

	uint8 abInvoiceTypes[] = { 4, 7, 26, 28 };
	int i = 0;
	*start_stop = 1;
	begin_time = get_time_sec();
	for (i = 0; i < sizeof(abInvoiceTypes); i++)
	{
		if (*start_stop != 1)
		{
			_WriteLog(LL_INFO, "ֹͣ���ڲ�ѯ%s�·�Ʊ���鶯��\n", (char *)month);
			if (year_month_data != NULL)
				free(year_month_data);
			free(send_data);
			free(lxp_data);
			return -1;
		}
		uint8 bInvType = abInvoiceTypes[i];
		if (!CheckDeviceSupportInvType(hDev->abSupportInvtype, bInvType))
			continue;  // not support
		if (CntaxEntry(hDev->hUSB, NOP_OPEN_FULL) < 0)
		{
			if (year_month_data != NULL)
				free(year_month_data);
			free(send_data);
			free(lxp_data);
			return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
		}
		if (CntaxSelectInvoiceType(hDev, bInvType, SELECT_OPTION_TEMPLATE) < 0) {
			CntaxEntry(hDev->hUSB, NOP_CLOSE);
			continue;
		}
		uint8 *pRetBuff = NULL;
		int nRetBuffLen = 0;
		//_WriteLog(LL_INFO,"��Ʊ����Ϊ%03d\n", bInvType);
		result = CntaxGetInvBuff(hDev->hUSB, 0, &pRetBuff, &nRetBuffLen, abQueryDataRange, sizeof abQueryDataRange,NULL);
		if (result < 0)
		{
			//_WriteLog(LL_INFO, "����%s  %03d��Ʊ�����޷�Ʊ���ѯʧ��,result = %d\n", month, bInvType, nInvNum);
			continue;
		}
		int nInvNum = CntaxGetInvDetail(hDev, FPCX_COMMON_MONTH, pRetBuff, result, NULL);
		if (nInvNum <= 0)
		{
			//_WriteLog(LL_INFO, "����%s  %03d��Ʊ�����޷�Ʊ���ѯʧ��,result = %d\n", month, bInvType, nInvNum);
			CntaxEntry(hDev->hUSB, NOP_CLOSE);
			continue;
		}
		//_WriteLog(LL_INFO, "cntax_get_invoice_month_upload_hook ��ȡ%s�·�Ʊ���� = %d\n", month, nInvNum);
		CntaxEntry(hDev->hUSB, NOP_CLOSE);

		int nBlockIndex = 0;

		//_WriteLog(LL_INFO,"����Ʊ���ݵ��Ž����󲢻ص�����\n");
		for (inv_count = 0; inv_count < nInvNum; inv_count++)
		{
			if (*start_stop != 1)
			{
				_WriteLog(LL_INFO, "ֹͣ���ڲ�ѯ%s�·�Ʊ���鶯��\n", (char *)month);
				if (year_month_data != NULL)
					free(year_month_data);
				free(send_data);
				free(lxp_data);
				free(pRetBuff);
				return -1;
			}
			HFPXX fpxx = MallocFpxxLinkDev(hDev);

			uint8 *pBlockBin = pRetBuff + nBlockIndex;
			int index_len;
			int nChildRet = CntaxInvRaw2Fpxx(hDev, FPCX_COMMON_MONTH, pBlockBin, fpxx, &index_len);
			if (nChildRet < 0)
			{
				FreeFpxx(fpxx);
				_WriteLog(LL_INFO, "��Ʊ�����������޷�����nChildRet = %d\n", nChildRet);
				break;
			}
			add_inv_sum_data(fpxx, &inv_sum);
			nBlockIndex += nChildRet;
			result = fpxx_to_json_base_fpsjbbh_v102(fpxx, &encode_data, DF_FPQD_Y, DF_FP_BASE);
			//out("invoice_bin_fpxx_json_base result = %d\n",result );
			if (result < 0)
			{
				_WriteLog(LL_INFO, "��Ʊ���ݽ���ʧ��,result = %d\n", result);
				if (encode_data != NULL)
				{
					free(encode_data);
					encode_data = NULL;
					_WriteLog(LL_INFO, "����� %d �ŷ�Ʊ���ݽ���ʧ�ܣ��ͷ��ڴ����\n", i + 1);
				}
				FreeFpxx(fpxx);
				continue;
			}

			hook_count += 1;

			if (fpxx->bIsUpload == 0)
			{
				if (strlen((char *)lxp_data) < max_len - 100)
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
						//_WriteLog(LL_INFO, "����� %d �ŷ�Ʊ����,�жϷ��������Ƿ��Ѿ����\n", inv_count + 1);
						char fpdm_fphm_up_off_str[50] = { 0 };
						memset(fpdm_fphm_up_off_str, 0, sizeof(fpdm_fphm_up_off_str));
						sprintf(fpdm_fphm_up_off_str, "%s,%s,%d,%d;", fpxx->fpdm, fpxx->fphm, fpxx->zfbz, fpxx->bIsUpload);

						if (str_replace(year_month_data, fpdm_fphm_up_off_str, ";") != 1)
						{
							_WriteLog(LL_INFO, "%s˰�̴���� %d �ŷ�Ʊ����,δ�ҵ������������Ϣ,���ܴ��ڴ����������ϴ�,%s\n", hDev->szCompanyName, inv_count + 1, fpdm_fphm_up_off_str);
							need_up = 1;
						}
					}
				}
			}
			else
			{
				if ((need_up == 0) && (strlen(upload_inv_num->year_month_data) != 0))
				{
					//_WriteLog(LL_INFO, "����� %d �ŷ�Ʊ����,�жϷ��������Ƿ��Ѿ����\n", inv_count + 1);
					char fpdm_fphm_up_off_str[50] = { 0 };
					memset(fpdm_fphm_up_off_str, 0, sizeof(fpdm_fphm_up_off_str));
					sprintf(fpdm_fphm_up_off_str, "%s,%s,%d,%d;", fpxx->fpdm, fpxx->fphm, fpxx->zfbz, fpxx->bIsUpload);

					if (str_replace(upload_inv_num->year_month_data, fpdm_fphm_up_off_str, ";") != 1)
					{
						_WriteLog(LL_INFO, "����� %d �ŷ�Ʊ����,δ�ҵ������������Ϣ,���ܴ��ڴ����������ϴ�,%s\n", inv_count + 1, fpdm_fphm_up_off_str);
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
			//_WriteLog(LL_INFO, "����%03d��Ʊ����ĵ� %d �ŷ�Ʊ�����ж����ŷ�Ʊ���ϴ�����������Ʊ����Ϊ%s ��Ʊ����Ϊ %s\n", bInvType, inv_count + 1, fpxx->fpdm, fpxx->fphm);

			FreeFpxx(fpxx);
			up_num += 1;
			//_WriteLog(LL_INFO, "�������׼������\n");
			send_data[index] = (((hook_count) >> 24) & 0xff); send_data[index + 1] = (((hook_count) >> 16) & 0xff);
			send_data[index + 2] = (((hook_count) >> 8) & 0xff); send_data[index + 3] = (((hook_count) >> 0) & 0xff);
			if (encode_data != NULL)
				inv_len = strlen((const char *)encode_data);
			send_data[index + 4] = ((inv_len >> 24) & 0xff); send_data[index + 5] = ((inv_len >> 16) & 0xff);
			send_data[index + 6] = ((inv_len >> 8) & 0xff); send_data[index + 7] = ((inv_len >> 0) & 0xff);
			memcpy(send_data + 8 + index, encode_data, inv_len);
			if (encode_data != NULL)
			{
				//out("����� %d �ŷ�Ʊ���ݿ�����ɣ��ͷ��ڴ����,address = %08x\n", i + 1, encode_data);
				free(encode_data);
				encode_data = NULL;

			}
			index += 8 + inv_len;
			now_count += 1;

			//_WriteLog(LL_INFO, "����%03d��Ʊ����� %d �ŷ�Ʊ���", bInvType, now_count);
			if (inv_count == nInvNum - 1)
			{
				//_WriteLog(LL_INFO, "��ǰƱ�����һ�ŷ�Ʊ��ȡ�ص�����\n");
				result = hook(send_data, 0, now_count, hook_arg, hDev->szDeviceID, month, NULL, 0);
				if (result < 0)
				{
					if (year_month_data != NULL)
						free(year_month_data);
					free(send_data);
					free(lxp_data);
					free(pRetBuff);
					_WriteLog(LL_INFO, "%s˰�̻ص�ͬ���������ݳ�ʱ\n", hDev->szCompanyName);
					return DF_TAX_ERR_CODE_HOOK_SERVER_TIME_OUT;
				}
				up_num = 0;
				//_WriteLog(LL_INFO, "��ǰƱ�����һ�ŷ�Ʊ��ȡ�ص��������\n");
				memset(send_data, 0, max_len * 10);
				begin_time = get_time_sec();
				index = 0;
				now_count = 0;
				break;
			}

			if (index > max_len)
			{
				//_WriteLog(LL_INFO, "��Ʊ����������1M\n");
				result = hook(send_data, 0, now_count, hook_arg, hDev->szDeviceID, month, NULL, 0);
				if (result < 0)
				{
					if (year_month_data != NULL)
						free(year_month_data);
					free(send_data);
					free(lxp_data);
					free(pRetBuff);
					_WriteLog(LL_INFO, "%s˰�̻ص�ͬ���������ݳ�ʱ\n", hDev->szCompanyName);
					return DF_TAX_ERR_CODE_HOOK_SERVER_TIME_OUT;
				}
				up_num = 0;
				memset(send_data, 0, max_len * 10);
				begin_time = get_time_sec();
				index = 0;
				now_count = 0;
			}
			now_time = get_time_sec();
			if (now_time - begin_time > over_time)
			{
				//_WriteLog(LL_INFO, "��ȡʱ�䳬��5��\n");
				result = hook(send_data, 0, now_count, hook_arg, hDev->szDeviceID, month, NULL, 0);
				if (result < 0)
				{
					if (year_month_data != NULL)
						free(year_month_data);
					free(send_data);
					free(lxp_data);
					free(pRetBuff);
					_WriteLog(LL_INFO, "%s˰�̻ص�ͬ���������ݳ�ʱ\n", hDev->szCompanyName);
					return DF_TAX_ERR_CODE_HOOK_SERVER_TIME_OUT;
				}
				up_num = 0;
				memset(send_data, 0, max_len * 10);
				begin_time = get_time_sec();
				index = 0;
				now_count = 0;
			}
			continue;
		}
		if (up_num != 0)
		{
			result = hook(send_data, 0, now_count, hook_arg, hDev->szDeviceID, month, NULL, 0);
			if (result < 0)
			{
				if (year_month_data != NULL)
					free(year_month_data);
				free(send_data);
				free(lxp_data);
				free(pRetBuff);
				_WriteLog(LL_INFO, "%s˰�̻ص�ͬ���������ݳ�ʱ\n", hDev->szCompanyName);
				return DF_TAX_ERR_CODE_HOOK_SERVER_TIME_OUT;
			}
		}
		up_num = 0;
		free(pRetBuff);
	}

	if (hook_count == 0)
	{
		//_WriteLog(LL_INFO, "����%s���з�Ʊ�����޷�Ʊ���ѯʧ��\n", month);
		char *statistics = NULL;
		cntax_query_invoice_month_all_data(hDev, (char *)month, &statistics, errinfo);
		result = compare_inv_sum_statistics(&inv_sum, statistics);
		if (result == 0)
		{
			result = hook(NULL, DF_INV_MONTH_COMPLETE, 0, hook_arg, (char *)hDev->szDeviceID, month, statistics, 0);
			if (result < 0)
			{
				if (statistics != NULL)
					free(statistics);
				if (year_month_data != NULL)
					free(year_month_data);
				free(send_data);
				free(lxp_data);
				_WriteLog(LL_INFO, "%s˰�̻ص�ͬ���������ݳ�ʱ\n", hDev->szCompanyName);
				return DF_TAX_ERR_CODE_HOOK_SERVER_TIME_OUT;
			}
		}
		else
		{
			logout(INFO, "TAXLIB", "��Ʊͬ��", "��Ʊ��ȡ������Ϊ0,���������뷢Ʊ��ϸ���ݶԱȲ�һ�»�Աȳ���,����ţ�%d\r\n", result);
			result = hook(NULL, DF_INV_MONTH_COMPLETE, 0, hook_arg, (char *)hDev->szDeviceID, month, statistics, 1);//��������ݲ�һ�£����ܴ�������
			if (result < 0)
			{
				if (statistics != NULL)
					free(statistics);
				if (year_month_data != NULL)
					free(year_month_data);
				free(send_data);
				free(lxp_data);
				_WriteLog(LL_INFO, "%s˰�̻ص�ͬ���������ݳ�ʱ\n", hDev->szCompanyName);
				return DF_TAX_ERR_CODE_HOOK_SERVER_TIME_OUT;
			}
		}
		if (statistics != NULL)
			free(statistics);
		if (year_month_data != NULL)
			free(year_month_data);
		free(send_data);
		free(lxp_data);
		return 0;
	}

	if (*start_stop == 1)
	{
		if (memcmp(month, hDev->szDeviceTime, 6) == 0)
		{
			memcpy(now_month_sum, &inv_sum, sizeof(struct _inv_sum_data));
		}
		char *statistics = NULL;
		cntax_query_invoice_month_all_data(hDev, (char *)month, &statistics, errinfo);
		result = compare_inv_sum_statistics(&inv_sum, statistics);
		if (result == 0)
		{
			result = hook(lxp_data, DF_INV_MONTH_COMPLETE, hook_count, hook_arg, (char *)hDev->szDeviceID, month, statistics, 0);
			if (result < 0)
			{
				if (statistics != NULL)
					free(statistics);
				if (year_month_data != NULL)
					free(year_month_data);
				free(send_data);
				free(lxp_data);
				_WriteLog(LL_INFO, "%s˰�̻ص�ͬ���������ݳ�ʱ\n", hDev->szCompanyName);
				return DF_TAX_ERR_CODE_HOOK_SERVER_TIME_OUT;
			}
		}
		else
		{
			logout(INFO, "TAXLIB", "��Ʊͬ��", "���������뷢Ʊ��ϸ���ݶԱȲ�һ�»�Աȳ���,����ţ�%d\r\n", result);
			result = hook(lxp_data, DF_INV_MONTH_COMPLETE, hook_count, hook_arg, (char *)hDev->szDeviceID, month, statistics, 1);//��������ݲ�һ�£����ܴ�������
			if (result < 0)
			{
				if (statistics != NULL)
					free(statistics);
				if (year_month_data != NULL)
					free(year_month_data);
				free(send_data);
				free(lxp_data);
				_WriteLog(LL_INFO, "%s˰�̻ص�ͬ���������ݳ�ʱ\n", hDev->szCompanyName);
				return DF_TAX_ERR_CODE_HOOK_SERVER_TIME_OUT;
			}
		}
		//out("��ǰ��Ʊ������ȡ���,���жϽ���,֪ͨM����ǰ�¶�ȡ���\n");
		if (statistics != NULL)
			free(statistics);
	}
	//out("[%s]��Ʊ��ѯ�ϴ�����\n", hDev->szCompanyName);
	if (year_month_data != NULL)
		free(year_month_data);
	free(send_data);
	free(lxp_data);
	return 0;
}


static int cntax_fpdm_fphm_get_invs_json_base(HDEV hDev, char  *fpdm, char *fphm, HFPXX fpxx)
{
	int ret = -1;
	uint8 abInvoiceTypes[] = { 4, 7, 26,28};
	int i = 0;
	for (i = 0; i < sizeof(abInvoiceTypes); i++)
	{
		uint8 bInvType = abInvoiceTypes[i];
		fpxx->hDev = hDev;
		_WriteLog(LL_INFO,"ͨ����Ʊ����%s,��Ʊ����%s,��ȡ��Ʊ����\n", fpdm, fphm);
		if ((ret = CntaxQueryInvInfo(hDev, bInvType, fpdm, fphm, fpxx)) < 0)
		{
			_WriteLog(LL_INFO, "%d��Ʊ����,��Ʊ��ȡʧ��,�л���һƱ�ֲ�ѯ,ret = %d\n", bInvType, ret);
			continue;
		}
		//out("��ȡ��Ʊ��Ϣ�ɹ�,�����json\n");
		ret = 0;
		break;
	}
	return ret;
}

static int cntax_get_invoice_single_upload_hook(HDEV hDev, void *function, void *arg, struct _upload_inv_num *upload_inv_num)
{
	//int m_total_num = -1;					//���·�Ʊ����
	//uint8 *pInvoiceBuff;					//���ŷ�Ʊ����
	int inv_count = 0;
	char *encode_data = NULL;
	int result;
	unsigned char *send_data;
	int max_len = 1 * 1024 * 1024;//������ݳ���1M���ص�����
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
			//_WriteLog(LL_INFO, "�������޷�Ʊ�˳�ѭ��\n");
			break;
		}
		if (upload_inv_num->need_upload_dmhm[inv_count].up_flag == 1)
		{
			_WriteLog(LL_INFO, "���ŷ�Ʊ���ϴ��������ظ��ϴ�\n");
			continue;
		}
		HFPXX fpxx = MallocFpxx();
		fpxx->hDev = hDev;

		if ((result = cntax_fpdm_fphm_get_invs_json_base(hDev, upload_inv_num->need_upload_dmhm[inv_count].fpdm, upload_inv_num->need_upload_dmhm[inv_count].fphm, fpxx)) < 0)
		{
			_WriteLog(LL_INFO, "��Ʊ��ȡʧ��,ret = %d\n", result);
			free(fpxx);
			continue;
		}
		fpxx_to_json_base_fpsjbbh_v102(fpxx, &encode_data, DF_FPQD_Y, DF_FP_BASE);
		memset(now_month, 0, sizeof(now_month));
		memcpy(now_month, fpxx->kpsj_standard, 6);
		free(fpxx);
		send_data = (uint8 *)malloc(max_len * 10 * 1); //��Ʊ���ݻ�����10M
		memset(send_data, 0, max_len * 10);
		send_data[0] = (((inv_count + 1) >> 24) & 0xff); //��Ʊ���
		send_data[1] = (((inv_count + 1) >> 16) & 0xff);
		send_data[2] = (((inv_count + 1) >> 8) & 0xff);
		send_data[3] = (((inv_count + 1) >> 0) & 0xff);
		if (encode_data != NULL)
			inv_len = strlen((const char *)encode_data);
		send_data[4] = ((inv_len >> 24) & 0xff); send_data[5] = ((inv_len >> 16) & 0xff);//��Ʊ����
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

int cntax_upload_m_server(HDEV hDev, unsigned short *start_stop, struct _upload_inv_num *upload_inv_num, void *function, void *arg, struct _inv_sum_data *now_month_sum)
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
		//_WriteLog(LL_INFO,"��Ҫ����Ʊ��������ϴ���Ʊ");
		/////�����ϴ�����
		result = cntax_get_invoice_single_upload_hook(hDev, function, arg, upload_inv_num);
		return result;

	}
	if (upload_inv_num->state != 1)
	{
		//_WriteLog(LL_INFO,"�����ϴ���Ʊ");
		return 0;
	}
	//_WriteLog(LL_INFO,"��Ҫ��ʱ����ϴ���Ʊ\n");
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
		//_WriteLog(LL_INFO, "\n��ȡ%s�·�Ʊ���ϴ�", summary_month);
		if (*start_stop != 1)
		{
			_WriteLog(LL_INFO, "ֹͣ���ڲ�ѯ%s�·�Ʊ���鶯��\n", (char *)summary_month);
			return -1;
		}
		//_WriteLog(LL_INFO, "���ڴ���Ļ����·�Ϊ%s�����ڴ���ķ�Ʊ�·�Ϊ%s\n", (char *)summary_month, year_month);
		if (strcmp(summary_month, year_month) == 0)
		{
			result = cntax_get_invoice_month_upload_hook(hDev, (uint8 *)year_month, function, arg, upload_inv_num, start_stop, now_month_sum);
			if (result < 0)
			{
				_WriteLog(LL_INFO, "��ȡ��Ʊ���ݲ��ص��ϴ������г����쳣,ֹͣ���ڲ�ѯ%s�·�Ʊ���鶯��\n", (char *)summary_month);
				return result;
			}
			mb_get_next_month(year_month);
		}
		else //�����·ݱȷ�Ʊ�·�С���貹�����
		{
			char *statistics = NULL;
			result = cntax_query_invoice_month_all_data(hDev, (char *)summary_month, &statistics, errinfo);
			if (result < 0)
			{
				_WriteLog(LL_INFO, "��ȡ��Ʊ�������ݹ����г����쳣,ֹͣ���ڲ�ѯ%s�·�Ʊ���鶯��\n", (char *)summary_month);
				continue;
			}
			result = hook(NULL, 1, -100, hook_arg, (char *)hDev->szDeviceID, (uint8 *)summary_month, statistics, 0);
			if (result < 0)
			{
				if (statistics != NULL)
					free(statistics);
				_WriteLog(LL_INFO, "�ص��ϱ���Ʊ�������ݳ����쳣,ֹͣ���ڲ�ѯ%s�·�Ʊ���鶯��\n", (char *)summary_month);
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
static int cntax_get_invoice_month_upload_hook(HDEV hDev, char *month, void *function, void *arg, struct _upload_inv_num *upload_inv_num, unsigned short *start_stop, struct _inv_sum_data *now_month_sum)
{
	int one_inv_type_num = -1;
	int inv_count = 0;
	char *inv_data = NULL;
	int result;
	char date_range[20] = { 0 };
	struct stat statbufs;
	int zip_data_len = 0;
	int need_up_count = 0;
	int need_up;
	char today_month[10] = { 0 };
	struct _inv_sum_data inv_sum;
	char fp_zip_name[100] = { 0 };
	char fp_txt_name[100] = { 0 };
	char stand_time[100] = { 0 };
	char fphm_crc[50] = { 0 };
	zipFile zf=NULL;

	memset(&inv_sum, 0, sizeof(struct _inv_sum_data));
	mb_get_today_month(today_month);
	sprintf(fp_zip_name, "/tmp/%s.zip", hDev->szDeviceID);
	result = cntax_read_tax_time(hDev, stand_time);
	if (result < 0)
	{
		_WriteLog(LL_INFO, "��ȡ˰��ʱ��ʧ��\n");
		return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
	}
	if (memcmp(month, hDev->szDeviceEffectDate, 6) < 0)
	{
		_WriteLog(LL_INFO, "%s˰�̵���%sδ��˰������ʱ��:%s\n", hDev->szCompanyName, month, hDev->szDeviceEffectDate);
		//result = hook(hook_arg, (char *)month, NULL, DF_UPLOAD_INV);
		result = send_zip_data_hook(hDev, month, function, arg, need_up_count, zf, fp_zip_name, zip_data_len, 1);
		if (result < 0)
		{
			_WriteLog(LL_INFO, "%s˰�̻ص�ͬ���������ݳ�ʱ\n", hDev->szCompanyName);
			return 	DF_TAX_ERR_CODE_HOOK_SERVER_TIME_OUT;
		}
		return 0;
	}
	turn_month_to_range((char *)month, date_range);
	uint8 abQueryDataRange[8] = { 0 };
	GetInvMonthFormat(date_range, abQueryDataRange);


	if (file_exists(fp_zip_name) == 0)
	{
		_WriteLog(LL_INFO, "%s˰�̴���Ʊ����ǰ����ѹ���ļ���ɾ��\n", hDev->szCompanyName);
		delete_file(fp_zip_name);
	}
	zf = zipOpen64(fp_zip_name, 0);
	if (zf == NULL)
	{
		_WriteLog(LL_INFO, "zipOpen64 compress file:%s fail!\n", fp_zip_name);
		return 	DF_TAX_ERR_CODE_SYSTEM_MEM_INSUFFICIENT;
	}


	uint8 abInvoiceTypes[] = { 4, 7, 26, 28 };
	int i = 0;
	*start_stop = 1;
	for (i = 0; i < sizeof(abInvoiceTypes); i++)
	{
		if (*start_stop != 1)
		{
			_WriteLog(LL_INFO, "%s˰��ֹͣ���ڲ�ѯ%s�·�Ʊ���鶯��\n", hDev->szCompanyName, (char *)month);
			zipClose(zf, NULL);
			delete_file(fp_zip_name);
			return -1;
		}
		uint8 bInvType = abInvoiceTypes[i];
		if (!CheckDeviceSupportInvType(hDev->abSupportInvtype, bInvType))
			continue;  // not support
		if (CntaxEntry(hDev->hUSB, NOP_OPEN_FULL) < 0)
		{
			zipClose(zf, NULL);
			delete_file(fp_zip_name);
			return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
		}
		if (CntaxSelectInvoiceType(hDev, bInvType, SELECT_OPTION_TEMPLATE) < 0) {
			CntaxEntry(hDev->hUSB, NOP_CLOSE);
			continue;
		}
		uint8 *pInvoiceBuff = NULL;
		int nRetBuffLen = 0;
		//_WriteLog(LL_INFO,"��Ʊ����Ϊ%03d\n", bInvType);
		result = CntaxGetInvBuff(hDev->hUSB, 0, &pInvoiceBuff, &nRetBuffLen, abQueryDataRange, sizeof abQueryDataRange,NULL);
		if (result < 0)
		{
			//_WriteLog(LL_INFO, "����%s  %03d��Ʊ�����޷�Ʊ���ѯʧ��,result = %d\n", month, bInvType, nInvNum);
			CntaxEntry(hDev->hUSB, NOP_CLOSE);
			continue;
		}
		one_inv_type_num = CntaxGetInvDetail(hDev, FPCX_COMMON_MONTH, pInvoiceBuff, result, NULL);
		if (one_inv_type_num <= 0)
		{
			//_WriteLog(LL_INFO, "����%s  %03d��Ʊ�����޷�Ʊ���ѯʧ��,result = %d\n", month, bInvType, nInvNum);
			CntaxEntry(hDev->hUSB, NOP_CLOSE);
			continue;
		}
		//_WriteLog(LL_INFO, "%s ��ȡ%s�·�Ʊ���� = %d\n", hDev->szCompanyName, month, one_inv_type_num);
		CntaxEntry(hDev->hUSB, NOP_CLOSE);

		int nBlockIndex = 0;

		//_WriteLog(LL_INFO,"����Ʊ���ݵ��Ž����󲢻ص�����\n");
		for (inv_count = 0; inv_count < one_inv_type_num; inv_count++)
		{
			if (*start_stop != 1)
			{
				_WriteLog(LL_INFO, "ֹͣ���ڲ�ѯ%s�·�Ʊ���鶯��\n", (char *)month);
				zipClose(zf, NULL);
				delete_file(fp_zip_name);
				free(pInvoiceBuff);
				return -1;
			}
			HFPXX fpxx = MallocFpxxLinkDev(hDev);

			uint8 *pBlockBin = pInvoiceBuff + nBlockIndex;
			int index_len;
			int nChildRet = CntaxInvRaw2Fpxx(hDev, FPCX_COMMON_MONTH, pBlockBin, fpxx, &index_len);
			if (nChildRet < 0)
			{
				FreeFpxx(fpxx);
				_WriteLog(LL_INFO, "��Ʊ�����������޷�����nChildRet = %d\n", nChildRet);
				break;
			}

			nBlockIndex += nChildRet;
			result = fpxx_to_json_base_fpsjbbh_v102(fpxx, &inv_data, DF_FPQD_Y, DF_FP_JSON);
			//_WriteLog(LL_INFO,"invoice_bin_fpxx_json_base result = %d\n",result );
			if (result < 0)
			{
				_WriteLog(LL_INFO, "%s˰�̴���� %d �ŷ�Ʊ����,��Ʊ���ݽ���ʧ��,result = %d\n", hDev->szCompanyName, inv_count + 1, result);
				if (inv_data != NULL)
				{
					free(inv_data);
					inv_data = NULL;
				}
				FreeFpxx(fpxx);
				continue;
			}
			add_inv_sum_data(fpxx, &inv_sum);
			//_WriteLog(LL_INFO, "����� %d �ŷ�Ʊ����,��Ʊ����Ϊ%03d ,��Ʊ����Ϊ%s ��Ʊ����Ϊ %s,���ϱ�־ %d,�ϴ���־ %d,��Ʊʱ�� %s,����ʱ�� %s\n",
			//	inv_count + 1, fpxx->fplx, fpxx->fpdm, fpxx->fphm, fpxx->zfbz,fpxx->bIsUpload, fpxx->kpsj_F2, fpxx->zfsj_F2);
			uint16 crc = crc_8005((uint8 *)inv_data, strlen(inv_data), 0);
			char invoice_crc[10] = { 0 };
			sprintf(invoice_crc, "%04x", crc);

			need_up = 1;

			if (upload_inv_num->dmhmzfsbstr != NULL)
			{
				if (strlen(upload_inv_num->dmhmzfsbstr) != 0)
				{
					//_WriteLog(LL_INFO, "����� %d �ŷ�Ʊ����,�жϷ��������Ƿ��Ѿ����\n", inv_count + 1);
					memset(fphm_crc, 0, sizeof(fphm_crc));
					sprintf(fphm_crc, "%s%s;", fpxx->fphm, invoice_crc);

					if (delete_str(upload_inv_num->dmhmzfsbstr, fphm_crc) == 1)
					{
						need_up = 0;
						//_WriteLog(LL_INFO, "����%s���ݳɹ�,��Ʊ�������ޱ䶯,�����ش���Ʊ\n", fphm_crc);
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
			//_WriteLog(LL_INFO, "����%03d��Ʊ����ĵ� %d �ŷ�Ʊ�����ж����ŷ�Ʊ���ϴ�����������Ʊ����Ϊ%s ��Ʊ����Ϊ %s\n", bInvType, inv_count + 1, fpxx->fpdm, fpxx->fphm);
			_WriteLog(LL_INFO, "����%s_%s���ݳɹ�crc= %s,���ش���Ʊ\n", fpxx->fpdm, fpxx->fphm, fphm_crc);


			memset(fp_txt_name, 0, sizeof(fp_txt_name));
			sprintf(fp_txt_name, "%s_%s_%s.txt", fpxx->fpdm, fpxx->fphm, invoice_crc);

			zip_fileinfo zi;
			memset(&zi, 0, sizeof(zip_fileinfo));
			zipOpenNewFileInZip3_64(zf, fp_txt_name, &zi, NULL, 0, NULL, 0, NULL, Z_DEFLATED, 9, 0, -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY, NULL, 0, 0);
			zipWriteInFileInZip(zf, inv_data, strlen(inv_data));
			zipCloseFileInZip(zf);
			stat(fp_zip_name, &statbufs);
			zip_data_len = statbufs.st_size;
			_WriteLog(LL_INFO, "ѹ����%d�ŷ�Ʊ��,ѹ������СΪ%d�ֽ�", inv_count + 1, zip_data_len);

			free(inv_data);
			inv_data = NULL;
			FreeFpxx(fpxx);


			if (zip_data_len > 1024 * 1024)
			{
				//����������Ƿ�ʧ�ܣ�zip����ָ��϶����Ѿ�����
				result = send_zip_data_hook(hDev, month, function, arg, need_up_count, zf, fp_zip_name, zip_data_len, 0);
				if (result < 0)
				{
					free(pInvoiceBuff);
					_WriteLog(LL_INFO, "%s˰�̻ص�ͬ���������ݳ�ʱ\n", hDev->szCompanyName);
					return 	DF_TAX_ERR_CODE_HOOK_SERVER_TIME_OUT;
				}
				zf = zipOpen64(fp_zip_name, 0);
				if (zf == NULL)
				{
					free(pInvoiceBuff);
					_WriteLog(LL_INFO, "zipOpen64 compress file:%s fail!\n", fp_zip_name);
					return 	DF_TAX_ERR_CODE_SYSTEM_MEM_INSUFFICIENT;
				}
				need_up_count = 0;//���ϴ���ɣ����ϴ���Ʊ��������
				continue;
			}
		}
		free(pInvoiceBuff);
	}


	//�������޷�Ʊ�����ô˽ӿڷ�������
	result = send_zip_data_hook(hDev, month, function, arg, need_up_count, zf, fp_zip_name, zip_data_len, 1);
	if (result < 0)
	{
		if (file_exists(fp_zip_name) == 0)
		{
			zipClose(zf, NULL);
			delete_file(fp_zip_name);
		}
		_WriteLog(LL_INFO, "%s˰�̻ص�ͬ���������ݳ�ʱ\n", hDev->szCompanyName);
		return DF_TAX_ERR_CODE_HOOK_SERVER_TIME_OUT;
	}

	if (file_exists(fp_zip_name) == 0)
	{
		zipClose(zf, NULL);
		delete_file(fp_zip_name);
	}
	//_WriteLog(LL_INFO, "%s˰�̵�ǰ��Ʊ������ȡ���,���жϽ���,֪ͨM����ǰ�¶�ȡ���\n", hDev->szCompanyName);
	if (memcmp(month, hDev->szDeviceTime, 6) == 0)
	{
		memcpy(now_month_sum, &inv_sum, sizeof(struct _inv_sum_data));
	}
	return 0;
}




int cntax_upload_m_server(HDEV hDev, unsigned short *start_stop, struct _upload_inv_num *upload_inv_num, void *function, void *arg, struct _inv_sum_data *now_month_sum)
{
	int result = 0;
	char today_month[10] = { 0 };
	char invoice_month[100] = { 0 };
	ClearLastError(hDev->hUSB);

	if (upload_inv_num->state == 0)
	{
		_WriteLog(LL_INFO, "[%s]�����ϴ���Ʊ", upload_inv_num->invoice_month);
		return 0;
	}

	//_WriteLog(LL_INFO,"��Ҫ��ʱ����ϴ���Ʊ\n");
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
		_WriteLog(LL_INFO, "\n��ȡ%s�·�Ʊ���ϴ�", invoice_month);
		if (*start_stop != 1)
		{
			_WriteLog(LL_INFO, "ֹͣ���ڲ�ѯ%s�·�Ʊ���鶯��\n", (char *)invoice_month);
			return -1;
		}
		//_WriteLog(LL_INFO, "���ڴ���Ļ����·�Ϊ%s�����ڴ���ķ�Ʊ�·�Ϊ%s\n", (char *)summary_month, invoice_month);

		result = cntax_get_invoice_month_upload_hook(hDev, invoice_month, function, arg, upload_inv_num, start_stop, now_month_sum);
		if (result < 0)
		{
			_WriteLog(LL_INFO, "��ȡ��Ʊ���ݲ��ص��ϴ������г����쳣,ֹͣ���ڲ�ѯ%s�·�Ʊ���鶯��\n", (char *)invoice_month);
			return result;
		}

		if (upload_inv_num->just_single_month == 1)
		{
			return 0;
		}
	}
	return 0;
}

int cntax_upload_summary_m_server(HDEV hDev, unsigned short *start_stop, struct _upload_inv_num *upload_inv_num, void *function, void *arg, struct _inv_sum_data *now_month_sum)
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
		_WriteLog(LL_INFO, "[%s]�����ϴ�����", upload_inv_num->summary_month);
		return 0;
	}

	//_WriteLog(LL_INFO,"��Ҫ��ʱ����ϴ���Ʊ\n");
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
		_WriteLog(LL_INFO, "\n��ȡ%s�»��ܲ��ϴ�", summary_month);
		if (*start_stop != 1)
		{
			_WriteLog(LL_INFO, "ֹͣ���ڲ�ѯ%s�·�Ʊ���鶯��\n", (char *)summary_month);
			return -1;
		}

		char errinfo[1024] = { 0 };
		char *statistics = NULL;
		result = cntax_query_invoice_month_all_data(hDev, (char *)summary_month, &statistics, errinfo);
		if (result < 0)
		{
			_WriteLog(LL_INFO, "��ȡ��Ʊ�������ݹ����г����쳣,ֹͣ���ڲ�ѯ%s�·�Ʊ���鶯��\n", (char *)summary_month);
			continue;
		}
		if (memcmp(summary_month, hDev->szDeviceTime, 6) == 0)
		{
			deal_plate_statistics_to_now_month_sum(statistics, now_month_sum);
		}
		result = hook(hook_arg, summary_month, statistics, DF_UPLOAD_SUM);
		if (statistics != NULL)
			free(statistics);
		if (result < 0)
		{
			_WriteLog(LL_INFO, "�ص��ϱ���Ʊ�������ݳ����쳣,ֹͣ���ڲ�ѯ%s�·�Ʊ���鶯��\n", (char *)summary_month);
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

int cntax_my_read_invoice_num(HDEV hDev, char *month, unsigned int *count, unsigned long *size)
{
	int i = 0;
	int result;
	char date_range[20] = { 0 };
	turn_month_to_range((char *)month, date_range);
	ClearLastError(hDev->hUSB);
	uint8 abQueryDataRange[8] = { 0 };
	if (GetInvMonthFormat(date_range, abQueryDataRange)) {
		_WriteLog(LL_FATAL, "Input dataformat error");
		return -1;
	}


	//uint8 abQueryDataRange[8] = { 0x20, 0x14, 0x01, 0x01, 0x20, 0x99, 0x12, 0x31 };


	int total_num = 0;
	//int nUploadOK = 0;


	// uint8 abInvoiceTypes[] = {4, 7, 26};
	int nTypeCount = hDev->abSupportInvtype[0];
	for (i = 0; i < nTypeCount; i++)
	{
		uint8 bInvType = hDev->abSupportInvtype[i + 1];

		if (CntaxEntry(hDev->hUSB, NOP_OPEN_FULL) < 0)
			return -1;

		if (CntaxSelectInvoiceType(hDev, bInvType, SELECT_OPTION_TEMPLATE) < 0) {
			if (CntaxEntry(hDev->hUSB, NOP_CLOSE) < 0)
				return -2;
			return -3;
		}
		uint8 *pRetBuff = NULL;
		int nRetBuffLen = 0;
		result = CntaxGetInvBuff(hDev->hUSB, 0, &pRetBuff, &nRetBuffLen,
			abQueryDataRange, sizeof abQueryDataRange, NULL);
		if (result < 0)
		{
			//_WriteLog(LL_INFO, "����%s  %03d��Ʊ�����޷�Ʊ���ѯʧ��,result = %d\n", month, bInvType, nInvNum);
			continue;
		}
		int nInvNum = CntaxGetInvDetail(hDev, FPCX_COMMON_MONTH, pRetBuff, result, NULL);
		if (nInvNum <= 0)
		{
			//_WriteLog(LL_INFO, "����%s  %03d��Ʊ�����޷�Ʊ���ѯʧ��,result = %d\n", month, bInvType, nInvNum);
			continue;
		}
		if (nInvNum > 0)
		{
			total_num += nInvNum;
		}
		//_WriteLog(LL_INFO, "cntax_my_read_invoice_num ��ȡ%s�·�Ʊ���� = %d\n", month, nInvNum);
		CntaxEntry(hDev->hUSB, NOP_CLOSE);
	}
	//_WriteLog(LL_FATAL, "˰����������������Ϊ%d\n", offlinenum);
	*count = total_num;
	*size = total_num * 3000;
	return total_num;
}

int cntax_get_inv_details_mqtt(HDEV hDev, unsigned char *month, void *function, void *arg, char *errinfo, int old_new, unsigned short *start_stop, int sfxqqd)
{
	int result;
	unsigned int total_count = 0;
	//int now_count = 0;
	//int com_count=0;
	unsigned long total_size = 0;
	char date_range[20] = { 0 };
	void *hook_arg;
	long index = 0;
	int(*hook)(unsigned char *s_data, int total_num, int now_num, void *hook_arg, char *plate_num);
	unsigned char *send_data;
	char *encode_data = NULL;
	int now_count = 0;
	long begin_time;
	long now_time;
	int inv_count = 0;
	int hook_count = 0;
	int max_len = 1 * 1024 * 1024;//������ݳ���1M���ص�����
	int over_time = 5;//��Ʊ���ݻ�ȡʱ�䳬��5�뼴�ص�����
	hook = function;
	hook_arg = arg;
	send_data = (uint8 *)malloc(max_len * 10 * 1); //��Ʊ���ݻ�����10M
	memset(send_data, 0, max_len * 10);
	ClearLastError(hDev->hUSB);
	_WriteLog(LL_INFO, "��ѯ�·�szQueryDateRange = %s\n", month);

	cntax_my_read_invoice_num(hDev, (char *)month, &total_count, &total_size);

	turn_month_to_range((char *)month, date_range);
	_WriteLog(LL_INFO, "ת���·ݳ�ʱ��� = %s\n", date_range);

	uint8 abQueryDataRange[8] = { 0 };
	if (GetInvMonthFormat(date_range, abQueryDataRange))
	{
		_WriteLog(LL_FATAL, "Input dataformat error");
		free(send_data);
		return -1;
	}

	// uint8 abInvoiceTypes[] = {4, 7, 26};
	uint8 abInvoiceTypes[] = { 4, 7, 26, 28 };
	//int nTypeCount = hDev->abSupportInvtype[0];
	//_WriteLog(LL_INFO, "nTypeCount = %d\n", nTypeCount);
	int i = 0;
	*start_stop = 1;
	begin_time = get_time_sec();
	for (i = 0; i < sizeof(abInvoiceTypes); i++)
	{
		if (*start_stop != 1)
		{
			_WriteLog(LL_INFO, "ֹͣ���ڲ�ѯ%s�·�Ʊ���鶯��\n", (char *)month);
			break;
		}
		uint8 bInvType = abInvoiceTypes[i];
		if (!CheckDeviceSupportInvType(hDev->abSupportInvtype, bInvType))
			continue;  // not support

		_WriteLog(LL_INFO, "\r\n\r\n����%s  %03d��Ʊ�����ѯ\n", month, bInvType);
		if (CntaxEntry(hDev->hUSB, NOP_OPEN_FULL) < 0)
			break;

		if (CntaxSelectInvoiceType(hDev, bInvType, SELECT_OPTION_TEMPLATE) < 0) {
			CntaxEntry(hDev->hUSB, NOP_CLOSE);
			continue;
		}
		uint8 *pRetBuff = NULL;
		int nRetBuffLen = 0;
		result = CntaxGetInvBuff(hDev->hUSB, 0, &pRetBuff, &nRetBuffLen,
			abQueryDataRange, sizeof abQueryDataRange, NULL);
		if (result < 0)
		{
			_WriteLog(LL_INFO, "����%s  %03d��Ʊ�����޷�Ʊ���ѯʧ��1,result = %d\n", month, bInvType, result);
			continue;
		}
		int nInvNum = CntaxGetInvDetail(hDev, FPCX_COMMON_MONTH, pRetBuff, result, NULL);
		if (nInvNum <= 0)
		{
			_WriteLog(LL_INFO, "����%s  %03d��Ʊ�����޷�Ʊ���ѯʧ��2,result = %d\n", month, bInvType, nInvNum);
			continue;
		}
		//_WriteLog(LL_INFO, "cntax_get_inv_details_mqtt ��ȡ%s�·�Ʊ���� = %d\n", month, nInvNum);
		CntaxEntry(hDev->hUSB, NOP_CLOSE);



		long inv_len;

		int nBlockIndex = 0;

		//out("����Ʊ���ݵ��Ž����󲢻ص�����\n");
		for (inv_count = 0; inv_count < nInvNum; inv_count++)
		{
			//out("��%d�λ�ȡ��Ʊ\n",inv_count+1);
			if (*start_stop != 1)
			{
				_WriteLog(LL_INFO, "ֹͣ���ڲ�ѯ%s�·�Ʊ���鶯��\n", (char *)month);
				break;
			}
			HFPXX fpxx = MallocFpxxLinkDev(hDev);
			if (!fpxx)
				break;
			uint8 *pBlockBin = pRetBuff + nBlockIndex;
			int index_len;
			int nChildRet = CntaxInvRaw2Fpxx(hDev, FPCX_COMMON_MONTH, pBlockBin, fpxx, &index_len);
			if (nChildRet < 0)
			{
				FreeFpxx(fpxx);
				_WriteLog(LL_INFO, "��Ʊ���ݽ���ʧ��nChildRet = %d\n", nChildRet);
				break;
			}
			nBlockIndex += nChildRet;
			_WriteLog(LL_INFO, "��Ʊ���ݽ����ɹ�,��Ʊ���ͣ�%03d,��Ʊ���룺%s,��Ʊ���룺%s\n", fpxx->fplx,fpxx->fpdm,fpxx->fphm);
			result = fpxx_to_json_base_fpsjbbh_v102(fpxx, &encode_data, sfxqqd,DF_FP_BASE);
			
			if (result < 0)
			{
				_WriteLog(LL_INFO, "��Ʊ���ݽ���ʧ��,result = %d\n", result);
				if (encode_data != NULL)
				{
					free(encode_data);
					encode_data = NULL;
					_WriteLog(LL_INFO, "����� %d �ŷ�Ʊ���ݽ���ʧ�ܣ��ͷ��ڴ����\n", i + 1);
				}
				FreeFpxx(fpxx);
				continue;
			}
			FreeFpxx(fpxx);
			hook_count += 1;
			//out("�������׼������\n");
			send_data[index] = (((hook_count) >> 24) & 0xff); send_data[index + 1] = (((hook_count) >> 16) & 0xff);
			send_data[index + 2] = (((hook_count) >> 8) & 0xff); send_data[index + 3] = (((hook_count) >> 0) & 0xff);
			if (encode_data != NULL)
				inv_len = strlen((const char *)encode_data);
			send_data[index + 4] = ((inv_len >> 24) & 0xff); send_data[index + 5] = ((inv_len >> 16) & 0xff);
			send_data[index + 6] = ((inv_len >> 8) & 0xff); send_data[index + 7] = ((inv_len >> 0) & 0xff);
			memcpy(send_data + 8 + index, encode_data, inv_len);
			if (encode_data != NULL)
			{
				//out("����� %d �ŷ�Ʊ���ݿ�����ɣ��ͷ��ڴ����,address = %08x\n", i + 1, encode_data);
				free(encode_data);
				encode_data = NULL;

			}
			index += 8 + inv_len;
			now_count += 1;

			//out("����%03d��Ʊ����� %d �ŷ�Ʊ���============================\n", bInvType, now_count);
			if (inv_count == nInvNum - 1)
			{
				//out("��ǰƱ�����һ�ŷ�Ʊ��ȡ�ص�����\n");
				hook(send_data, total_count, now_count, hook_arg, hDev->szDeviceID);
				//out("��ǰƱ�����һ�ŷ�Ʊ��ȡ�ص��������\n");
				memset(send_data, 0, max_len * 10);
				begin_time = get_time_sec();
				index = 0;
				now_count = 0;
				break;
			}

			if (index > max_len)
			{
				//_WriteLog(LL_INFO, "��Ʊ����������1M\n");
				hook(send_data, total_count, now_count, hook_arg, hDev->szDeviceID);
				memset(send_data, 0, max_len * 10);
				begin_time = get_time_sec();
				index = 0;
				now_count = 0;
			}
			now_time = get_time_sec();
			if (now_time - begin_time > over_time)
			{
				//_WriteLog(LL_INFO, "��ȡʱ�䳬��5��\n");
				hook(send_data, total_count, now_count, hook_arg, hDev->szDeviceID);
				memset(send_data, 0, max_len * 10);
				begin_time = get_time_sec();
				index = 0;
				now_count = 0;
			}
			continue;
		}

		free(pRetBuff);
	}
	free(send_data);
	*start_stop = 0;
	return 0;
}

int cntax_read_tax_time(HDEV hDev, char *outtime)
{
	char time_tmp[20] = { 0 };
	ClearLastError(hDev->hUSB);
	if (CntaxEntry(hDev->hUSB, NOP_OPEN_FULL) < 0)
		return -1;
	if (CntaxGetDeviceTime(hDev->hUSB, hDev->szDeviceTime) < 0) {
		CntaxEntry(hDev->hUSB, NOP_CLOSE);
		return -1;
	}
	//_WriteLog(LL_INFO, "NISEC TIME = %s\n", h_dev->szDeviceTime);
	asc2bcd((uint8 *)hDev->szDeviceTime, (uint8 *)time_tmp, strlen(hDev->szDeviceTime));
	sprintf(outtime, "%02x%02x-%02x-%02x %02x:%02x:%02x", time_tmp[0], time_tmp[1], time_tmp[2], time_tmp[3], time_tmp[4], time_tmp[5], time_tmp[6]);
	CntaxEntry(hDev->hUSB, NOP_CLOSE);
	return 0;
}


int cntax_fpdm_fphm_get_invs(HDEV hDev, char  *fpdm, char *fphm, char **inv_json)
{
	int ret = -1;
	uint8 abInvoiceTypes[] = { 4, 7, 26,28 };
	int i = 0;
	HFPXX fpxx = MallocFpxx();
	ClearLastError(hDev->hUSB);
	for (i = 0; i < sizeof(abInvoiceTypes); i++)
	{
		uint8 bInvType = abInvoiceTypes[i];
		if (!CheckDeviceSupportInvType(hDev->abSupportInvtype, bInvType))
			continue;  // not support
		fpxx->hDev = hDev;
		_WriteLog(LL_INFO,"ͨ����Ʊ����%s,��Ʊ����%s,��ȡ��Ʊ����\n", fpdm, fphm);
		if ((ret = CntaxQueryInvInfo(hDev, bInvType, fpdm, fphm, fpxx)) < 0)
		{
			_WriteLog(LL_INFO, "%d��Ʊ����,��Ʊ��ȡʧ��,�л���һƱ�ֲ�ѯ,ret = %d\n", bInvType, ret);
			continue;
		}
		fpxx_to_json_base_fpsjbbh_v102(fpxx, inv_json, DF_FPQD_Y, DF_FP_BASE);
		ret = 0;
		break;
	}
	if (fpxx)
	{
		FreeFpxx(fpxx);
	}
	return ret;
}

int cntax_read_current_inv_code(HDEV hDev, uint8_t bNeedType, char *szInvCurrentNum, char *szInvTypeCode)
{
	char szFPZZHM[24] = "";
	ClearLastError(hDev->hUSB);
	//char szInvCurrentNum_t[100] = { 0 }, szInvTypeCode_t[100] = {0};
	if (CntaxEntry(hDev->hUSB, NOP_OPEN_FULL) < 0)
		return -1;
	if (!CheckDeviceSupportInvType(hDev->abSupportInvtype, bNeedType)) {
		_WriteLog(LL_FATAL, "Device not has this invoice-type, type:%d", bNeedType);
		return -2;
	}
	if (CntaxSelectInvoiceType(hDev, bNeedType, SELECT_OPTION_TEMPLATE) < 0) {
		return -3;
	}
	if (CommonGetCurrentInvCode(hDev, szInvTypeCode, szInvCurrentNum, szFPZZHM) < 0) {
		return -4;
	}
	if (CntaxEntry(hDev->hUSB, NOP_CLOSE) < 0)
		return -5;
	_WriteLog(LL_FATAL, "��Ʊ����:%03d����Ʊ���룺%s����Ʊ���룺%s", bNeedType, szInvCurrentNum, szInvTypeCode);
	return 0;
}
static int check_fpxx_cntax(struct Fpxx* stp_fpxx, struct _plate_infos *plate_infos, char *errinfo)
{
	int i_spsmmclen = 0;

	//char* pc_Tmp = NULL;
	int inv_type;
	if (!stp_fpxx || !plate_infos)
	{
		sprintf(errinfo, "��Ʊ��Ϣ���ݽṹ��Ϊ��");
		return DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
	}
	//int i_RateFlag = 0;
	struct Spxx *stp_spxx = stp_fpxx->stp_MxxxHead->stp_next;
	struct Spxx *stp_spxx_tmp = stp_fpxx->stp_MxxxHead->stp_next;
	int i_FplxIndex = 0;
	//��Ʊ����

	inv_type = stp_fpxx->fplx;

	find_inv_type_mem((unsigned char)inv_type, &i_FplxIndex);
	if (i_FplxIndex < 0 || i_FplxIndex> 7)
	{
		sprintf(errinfo, "��Ʊ���ʹ���");
		return DF_TAX_ERR_CODE_UNSUPPORT_INV_TYPE;
	}

	//�鿴�Ƿ񳬹����ŷ�Ʊ�޶�
	long double dzfpkpxe;//���ŷ�Ʊ�޶�
	long double fpzje;//��ǰ��Ʊ�ܽ��

	dzfpkpxe = strtold((char *)plate_infos->invoice_type_infos[i_FplxIndex].monitor_info.dzkpxe,NULL);
	fpzje = strtold(stp_fpxx->je,NULL);
	_WriteLog(LL_INFO, "���ŷ�Ʊ�޶�%4.2Lf,��ǰ��Ʊ���%4.2Lf  i_FplxIndex = %d\n", dzfpkpxe, fpzje, i_FplxIndex);
	if (fpzje > dzfpkpxe)
	{
		_WriteLog(LL_INFO, "�������ŷ�Ʊ�޶�\n");
		sprintf(errinfo, "�������ŷ�Ʊ�޶�,���ŷ�Ʊ�޶�%4.2Lf,��ǰ��Ʊ���%4.2Lf", dzfpkpxe, fpzje);
		return DF_TAX_ERR_CODE_INV_DATA_DISACCORD;
	}

	long double lxzsljje;//���������ۼƽ��
	lxzsljje = strtold((char *)plate_infos->invoice_type_infos[i_FplxIndex].monitor_info.lxzsljje, NULL);

	if (lxzsljje <= 0)
	{
		sprintf(errinfo, "���������ۼƽ��Ϊ0,������Ʊ");
		_WriteLog(LL_INFO, "%s\n", errinfo);
		return DF_TAX_ERR_CODE_INV_DATA_DISACCORD;
	}

	if (plate_infos->plate_tax_info.off_inv_num != 0)
	{
		if (plate_infos->invoice_type_infos[i_FplxIndex].monitor_info.lxkpsc == 0)
		{
			sprintf(errinfo, "����ʱ��Ϊ0���������߶������߷�Ʊ,�����ϴ����߷�Ʊ�ڽ��п���");
			return DF_TAX_ERR_CODE_INV_DATA_DISACCORD;
		}
	}

	//if (memcmp(stp_fpxx->hDev->szDeviceTime, plate_infos->invoice_type_infos[i_FplxIndex].monitor_info.kpjzsj, 8) >= 0)
	//{
	//	sprintf(errinfo, "������Ʊ��ֹ����,�޷���Ʊ,��Ʊ��ֹ���� = %s,��ǰ˰��ʱ�� = %s", plate_infos->invoice_type_infos[i_FplxIndex].monitor_info.kpjzsj, stp_fpxx->hDev->szDeviceTime);
	//	return DF_TAX_ERR_CODE_EXCEED_KP_END_DATE;
	//}

	//if ((memcmp(stp_fpxx->hDev->szDeviceTime, plate_infos->invoice_type_infos[i_FplxIndex].monitor_info.zxbsrq, 6) > 0) && 
	//	(memcmp(stp_fpxx->hDev->szDeviceTime, plate_infos->invoice_type_infos[i_FplxIndex].monitor_info.kpjzsj, 6) >= 0))//��ǰ�´������±�˰������,��ҪС�ڿ�Ʊ��ֹ����,��Ҫ��˰����
	//{
	//	sprintf(errinfo, "δ��ɳ�˰,������Ʊ,���±�˰���ڣ�%s,��ǰ˰��ʱ�䣺%s", plate_infos->invoice_type_infos[i_FplxIndex].monitor_info.zxbsrq, stp_fpxx->hDev->szDeviceTime);
	//	return DF_TAX_ERR_CODE_INV_DATA_DISACCORD;
	//}

	//����˰��
	if ((strlen((const char *)plate_infos->plate_basic_info.ca_number) == 0) || (strcmp((const char *)stp_fpxx->xfsh, (const char *)plate_infos->plate_basic_info.ca_number) != 0) || (strlen(stp_fpxx->xfsh) < 15) || (strlen(stp_fpxx->xfsh) > 20)){
		sprintf(errinfo, "����˰����Ϣ��˰�̲�ƥ��");
		logout(INFO, "TAXLIB", "��Ʊ����", "ϵͳ˰��%s,��Ʊ��������%s\r\n", plate_infos->plate_basic_info.ca_number, stp_fpxx->xfsh);
		return DF_TAX_ERR_CODE_INV_DATA_DISACCORD;
	}
	//��������
	if ((strlen((const char *)plate_infos->plate_basic_info.ca_name) == 0) || (strcmp((const char *)stp_fpxx->xfmc, (const char *)plate_infos->plate_basic_info.ca_name)) != 0){
		sprintf(errinfo, "����������Ϣ��˰�̲�ƥ��");
		logout(INFO, "TAXLIB", "��Ʊ����", "ϵͳ����%s,��Ʊ��������%s\r\n", plate_infos->plate_basic_info.ca_name, stp_fpxx->xfmc);
		return DF_TAX_ERR_CODE_INV_DATA_DISACCORD;
	}
	//�̺�
	if ((strlen((const char *)plate_infos->plate_basic_info.plate_num) == 0) || (strcmp((const char *)stp_fpxx->jqbh, (const char *)plate_infos->plate_basic_info.plate_num) != 0)){
		sprintf(errinfo, "�����̺���Ϣ��˰�̲�ƥ��,ϵͳ�̺�%s,��Ʊ�����̺�%s", plate_infos->plate_basic_info.plate_num, stp_fpxx->jqbh);
		logout(INFO, "TAXLIB", "��Ʊ����", "ϵͳ�̺�%s,��Ʊ�����̺�%s\r\n", plate_infos->plate_basic_info.plate_num, stp_fpxx->jqbh);
		return DF_TAX_ERR_CODE_INV_DATA_DISACCORD;
	}
	/*��Ҫ��Ʒ˰Ŀ����*/
	if (strcmp(stp_fpxx->zyspsmmc, stp_spxx->spsmmc) != 0){
		printf("check_fpxx_aisino test4\n");
		sprintf(errinfo, "��Ҫ��Ʒ˰Ŀ������Ҫ��һ����Ʒ˰Ŀ����һ��");
		return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
	}
	/*��Ҫ��Ʒ˰Ŀ����*/
	if (strcmp(stp_fpxx->zyspmc, stp_spxx->spmc) != 0){
		printf("check_fpxx_aisino test5\n");
		printf("��Ҫ��Ʒ���ƣ�%s\n", stp_fpxx->zyspmc);
		printf("��һ����Ʒ���ƣ�%s\n", stp_spxx->spmc);
		sprintf(errinfo, "��Ҫ��Ʒ������Ҫ��һ����Ʒ����һ��");
		return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
	}

	/*��Ʒ˰Ŀ����*/
	while (stp_spxx_tmp){
		if ((strcmp(stp_spxx_tmp->spmc, "(�����Ӧ������Ʊ�嵥)") != 0) && (strcmp(stp_spxx_tmp->spmc, "�����Ӧ������Ʊ���嵥") != 0))
		{
			i_spsmmclen = strlen(stp_spxx_tmp->spsmmc);
			if (i_spsmmclen < 3)
			{
				printf("test i_spsmmclen < 3\n");
				sprintf(errinfo, "��Ʒ˰Ŀ���ƹ������");
				return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
			}
			if (stp_spxx_tmp->spsmmc[0] != '*')
			{
				if ((strcmp(stp_spxx_tmp->spsmmc, "ԭ�ۺϼ�") == 0) || (strcmp(stp_spxx_tmp->spsmmc, "�ۿ۶�ϼ�") == 0))
				{
					stp_spxx_tmp = stp_spxx_tmp->stp_next;
					continue;
				}

				printf("test stp_spxx_tmp->spsmmc[0] != *,˰Ŀ����%s\n", stp_spxx_tmp->spsmmc);
				sprintf(errinfo, "��Ʒ˰Ŀ���ƹ������");
				return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
			}

			if (stp_spxx_tmp->spsmmc[strlen(stp_spxx_tmp->spsmmc) - 1] != '*')
			{
				printf("test stp_spxx_tmp->spsmmc[strlen(stp_spxx_tmp->spsmmc) - 1] != *\n");
				sprintf(errinfo, "��Ʒ˰Ŀ���ƹ������");
				return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
			}
		}

		stp_spxx_tmp = stp_spxx_tmp->stp_next;
	}

	//˰�ż��
	if (!strlen(stp_fpxx->xfsh) || !strlen(stp_fpxx->gfmc) || !strlen(stp_fpxx->xfmc)) {
		printf("[-] Need mc and sh");
		sprintf(errinfo, "��������Ϣ��д����");
		return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
	}
	//// ��ר�÷�Ʊgf˰�ſ�Ϊ��
	//if ((stp_fpxx->fplx_aisino == FPLX_AISINO_ZYFP) || (stp_fpxx->fplx_aisino == FPLX_AISINO_DZZP)) {
	//	if (!strlen(stp_fpxx->gfsh) || !strlen(stp_fpxx->gfdzdh) || !strlen(stp_fpxx->gfyhzh)) {
	//		printf("[-] Zyfp gf info error");
	//		sprintf(errinfo, "��ֵ˰ר�÷�Ʊ����д����˰�š�������ַ�绰�����������˺�");
	//		return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
	//	}
	//}
	//else {
	//	if (!strlen(stp_fpxx->gfsh))
	//		strcpy(stp_fpxx->gfsh, "000000000000000");
	//}
	//wang 04 03 ��ӹ���˰�ų��ȷ�19�ж�
	//˰���̺�uk��ֽ��007(i_FplxIndex=3)�͵���026(i_FplxIndex=6)
	/*if((i_FplxIndex == 3) || (i_FplxIndex == 6))
	{
		if ((strlen(stp_fpxx->gfsh) != 0) && (strlen(stp_fpxx->gfsh) != 15) && (strlen(stp_fpxx->gfsh) != 17) &&
			(strlen(stp_fpxx->gfsh) != 18) && (strlen(stp_fpxx->gfsh) != 19) && (strlen(stp_fpxx->gfsh) != 20)) {
			printf("[-] TaxID's gfsh length must be [15,20]");
			sprintf(errinfo, "����˰�ų��������0��15��17��18��19��20λ");
			return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
		}
	}
	else 
	{
		if (!strlen(stp_fpxx->gfsh) && (strlen(stp_fpxx->gfsh) != 15) && (strlen(stp_fpxx->gfsh) != 17) &&
			(strlen(stp_fpxx->gfsh) != 18) && (strlen(stp_fpxx->gfsh) != 20)) {
			printf("[-] TaxID's gfsh length must be [15,20]");
			sprintf(errinfo, "����˰�ų��������15��17��18��20λ");
			return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
		}
	}*/
	if (!strlen(stp_fpxx->gfsh) && (strlen(stp_fpxx->gfsh) != 15) && (strlen(stp_fpxx->gfsh) != 17) &&
		(strlen(stp_fpxx->gfsh) != 18) && (strlen(stp_fpxx->gfsh) != 20)) {
		printf("[-] TaxID's gfsh length must be [15,20]");
		sprintf(errinfo, "����˰�ų��������15��17��18��20λ");
		return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
	}
	if ((strlen(stp_fpxx->xfsh) != 15) && (strlen(stp_fpxx->xfsh) != 17) && (strlen(stp_fpxx->xfsh) != 18) &&
		(strlen(stp_fpxx->xfsh) != 20)) {
		printf("[-] TaxID's xfsh length must be [15,20]");
		sprintf(errinfo, "����˰�ų��������15��17��18��20λ");
		return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
	}
	////����˰��(����˰��!=����˰��)
	//if (strcmp((const char *)stp_fpxx->gfsh, (const char *)plate_infos->plate_basic_info.ca_number) == 0){
	//	printf("gfsh = %s ���� xfsh=%s", stp_fpxx->gfsh, plate_infos->plate_basic_info.ca_number);
	//	sprintf(errinfo, "����˰�Ų��õ�������˰��");
	//	return DF_TAX_ERR_CODE_INV_DATA_DISACCORD;
	//}
	if (stp_fpxx->spsl > 2000)
	{
		sprintf(errinfo, "��Ʒ�������ô���2000��");
		printf("��Ʒ�������ܳ���2000��\n");
		return DF_TAX_ERR_CODE_INV_DATA_TOO_LARGE;
	}
	//if (i_RateFlag != 0)
	//{
	//	out("test i_RateFlag != 0\n");
	//	return DF_TAX_ERR_CODE_TAX_RATE_UNSUPPORTED;
	//}
	return 0;
}
int cntax_make_invoice(HDEV hDev, char* inv_data, struct _plate_infos *plate_infos, struct _make_invoice_result *make_invoice_result)
{
	//int nRet = 0;
	int result = -1;
	ClearLastError(hDev->hUSB);
	result = LoadCntaxDevInfo(hDev, hDev->hUSB, true);
	if (result < 0)
	{
		sprintf((char *)make_invoice_result->errinfo, "USB����ʧ��,˰����Ϣ�޷�����,������");
		return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
	}
	if (strcmp(hDev->szDeviceID, (char *)plate_infos->plate_basic_info.plate_num) != 0)
	{
		sprintf((char *)make_invoice_result->errinfo, "USB����ʧ��,˰�̻�����Ϣ��һ��,������");
		return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
	}
	//struct Fpxx *stp_fpxx = NULL;
	//str_replace(inv_data, "\\", "[@*br/*@]");//�Ų�json�е�\ת���ַ�
	//result = check_invoice_str((unsigned char *)inv_data);
	//if (result < 0)
	//{
	//	sprintf((char *)make_invoice_result->errinfo, "��ƱJSON�����д��������ַ��޷�����,����&��\\");
	//	return DF_TAX_ERR_CODE_USE_ILL_CHAR_OR_UTF8;
	//}
	//result = judge_code(inv_data, strlen(inv_data));
	//result = IsGBK(inv_data);
	//if (result != 1)
	//{
	//	sprintf((char *)make_invoice_result->errinfo, "��ƱJSON���ݺ��ֱ����ʽ����,��GBK����");
	//	return DF_TAX_ERR_CODE_USE_ILL_CHAR_OR_UTF8;//�����ʽ����
	//}

	HFPXX stp_fpxx = MallocFpxxLinkDev(hDev);
	if (stp_fpxx == NULL) {
		printf("stp_fpxx malloc Err\n");
		sprintf((char *)make_invoice_result->errinfo, "ϵͳ�ڴ�����ʧ��,������ܴ��ڹ���,����������");
		return DF_TAX_ERR_CODE_SYSTEM_MEM_INSUFFICIENT;
	}
	stp_fpxx->isHzxxb = false;
	stp_fpxx->hDev = hDev;
	strcpy(stp_fpxx->tax_nature, (char *)plate_infos->plate_tax_info.tax_nature);
	////����json config
	//if (ImportJSONToFpxx(fpxx, hDev) < 0) {
	//	_WriteLog(LL_FATAL, "[-] ImportJSONToFpxx failed");
	//	nRet = -2;
	//	goto FreeAndExit;
	//}
	//out("������ƱJSON����\n");
	_WriteHexToDebugFile("import.json", (uint8 *)inv_data, strlen(inv_data));
	if ((result = AnalyzeJsonBuff(hDev->bDeviceType, inv_data, stp_fpxx, (char *)make_invoice_result->errinfo)) != 0)
	{
		logout(INFO, "TAXLIB", "��Ʊ����", "��Ʊ����ʧ��,�������%d\r\n", result);
		FreeFpxx(stp_fpxx);
		return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
	}
	UpdateDevInfoToFPXX(hDev, stp_fpxx);
	//_WriteLog(LL_INFO, "1��Ʊʱ�䣺%s,˰��ʱ�䣺%s\n", stp_fpxx->kpsj_F2, hDev->szDeviceTime);
	if (CheckFpxxNode(stp_fpxx, hDev, (char *)make_invoice_result->errinfo))
	{
		logout(INFO, "TAXLIB", "��Ʊ����", "���鷢Ʊ�����Ƿ���ȷ����\r\n");
		result = DF_TAX_ERR_CODE_INV_DATA_DISACCORD;
		goto FreeAndExit;
	}
	if (CntaxUpdateNodeInfo(stp_fpxx))
	{
		logout(INFO, "TAXLIB", "��Ʊ����", "���鷢Ʊ�����Ƿ���ȷ����2\r\n");
		result = DF_TAX_ERR_CODE_INV_DATA_DISACCORD;
		goto FreeAndExit;
	}
	//_WriteLog(LL_INFO, "2��Ʊʱ�䣺%s,˰��ʱ�䣺%s\n", stp_fpxx->kpsj_F2, hDev->szDeviceTime);
	result = check_fpxx_cntax(stp_fpxx, plate_infos, (char *)make_invoice_result->errinfo);
	if (result < 0)
	{
		printf("check_fpxx_aisino failed\n");
		goto FreeAndExit;
	}
	//_WriteLog(LL_INFO, "3��Ʊʱ�䣺%s,˰��ʱ�䣺%s\n", stp_fpxx->kpsj_F2, hDev->szDeviceTime);
	if (make_invoice_result->test_only == 1)
	{
		logout(INFO, "TAXLIB", "��Ʊ����", "���Խӿڲ�ִ�����տ��߶���\r\n");
		sprintf((char *)make_invoice_result->errinfo, "��Ʊ���ݼ�˰�̻���У����ɹ�,���Խӿڲ�ִ�����տ���");
		result = DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
		goto FreeAndExit;
	}
	if ((stp_fpxx->isRed) && 
		(stp_fpxx->fplx == FPLX_COMMON_ZYFP || (stp_fpxx->fplx == FPLX_COMMON_DZZP)) && 
		strcmp(stp_fpxx->redNum,"0000000000000000")==0)
	{
		logout(INFO, "TAXLIB", "��Ʊ����", "רƱ��忪��,��Ϣ���Ų���Ϊ0000000000000000\r\n");
		sprintf((char *)make_invoice_result->errinfo, "רƱ�����Ϣ���Ų���Ϊ0000000000000000");
		result = DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
		goto FreeAndExit;

	}

	//_WriteLog(LL_INFO, "4��Ʊʱ�䣺%s,˰��ʱ�䣺%s\n", stp_fpxx->kpsj_F2, hDev->szDeviceTime);
	if ((result = CntaxMakeInvoiceMain(stp_fpxx)) < 0) {

		sprintf((char *)make_invoice_result->errinfo, "%s", hDev->hUSB->szLastErrorDescription);
		logout(INFO, "TAXLIB", "��Ʊ����", "���һ����Ʊ�ײ����ʧ��,result = %d\r\n", result);
		//stp_fpxx_json_nisec(stp_fpxx, &make_invoice_result->scfpsj, 1);
		//fpxx_to_json_base_fpsjbbh_v102(stp_fpxx, &make_invoice_result->scfpsj,DF_FPQD_Y,DF_FP_BASE);
		_WriteLog(LL_INFO,"make_invoice_result->scfpsj = %s\n", make_invoice_result->scfpsj);
		result = DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
		goto FreeAndExit;
	}
	//_WriteLog(LL_INFO, "5��Ʊʱ�䣺%s,˰��ʱ�䣺%s\n", stp_fpxx->kpsj_F2, hDev->szDeviceTime);
	make_invoice_result->fplxdm = stp_fpxx->fplx;
	//if (make_invoice_result->fplxdm == FPLX_COMMON_ZYFP)
	//	sprintf((char *)make_invoice_result->fpdm, "%010llu", atoll(stp_fpxx->fpdm));
	//else
	//	sprintf((char *)make_invoice_result->fpdm, "%012llu", atoll(stp_fpxx->fpdm));
	//sprintf((char *)make_invoice_result->fphm, "%08llu", atoll(stp_fpxx->fphm));

	strcpy((char *)make_invoice_result->fpdm, stp_fpxx->fpdm);
	strcpy((char *)make_invoice_result->fphm, stp_fpxx->fphm);

	memcpy(make_invoice_result->kpsj, stp_fpxx->kpsj_F2, sizeof(make_invoice_result->kpsj));
	memcpy(make_invoice_result->hjje, stp_fpxx->je, sizeof(make_invoice_result->hjje));
	memcpy(make_invoice_result->hjse, stp_fpxx->se, sizeof(make_invoice_result->hjse));
	memcpy(make_invoice_result->jshj, stp_fpxx->jshj, sizeof(make_invoice_result->jshj));
	memcpy(make_invoice_result->jym, stp_fpxx->jym, sizeof(make_invoice_result->jym));
	memcpy(make_invoice_result->mwq, stp_fpxx->mw, strlen(stp_fpxx->mw));
	memcpy(make_invoice_result->fpqqlsh, stp_fpxx->fpqqlsh, sizeof(make_invoice_result->fpqqlsh));
	make_invoice_result->fpdzsyh = stp_fpxx->dzsyh;
	UTF8ToGBKBase64(stp_fpxx->bz, strlen(stp_fpxx->bz), (char *)make_invoice_result->bz);


	if (stp_fpxx->need_restart == 1)
	{
		make_invoice_result->need_restart = 1;
	}
	if (((stp_fpxx->fplx == FPLX_COMMON_DZFP) || stp_fpxx->fplx == FPLX_COMMON_DZZP) && (strlen(stp_fpxx->hDev->szPubServer) != 0))
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
				}
			}
		}
	}

	fpxx_to_json_base_fpsjbbh_v102(stp_fpxx, &make_invoice_result->scfpsj, DF_FPQD_Y, DF_FP_JSON);
	//stp_fpxx_json_nisec(stp_fpxx, &make_invoice_result->scfpsj, 1);
	//memcpy(make_invoice_result->bz, stp_fpxx->bz, sizeof(make_invoice_result->bz));
	//out("��Ʊ�����������Ϣ���ݣ�%s\n", make_invoice_result->mwq);
	result = 0;
	//out("��Ʊ����ṹ�帳ֵ���,scfpsj:%s\n", make_invoice_result->scfpsj);
FreeAndExit:
	if (stp_fpxx)
	{
		//out("�ͷŷ�Ʊ�ṹ��\n");
		FreeFpxx(stp_fpxx);
	}
	//asleep(10);
	return result;

}





int cntax_verify_cert_passwd(HDEV hDev, int *left_num)
{
	ClearLastError(hDev->hUSB);
	//�����ǰloaddev��ʱ��crypt_deviceopen��δִ�У�������ܻ���Ҫ����ִ��һ��
	uint8 cb[512] = { 0 }, abPreHashBegin16[24] = { 0 }, abStep2_16BytesCMD[24] = { 0 },
		abStep2_16bytesParament[24] = { 0 }, abCryptKey[48] = { 0 }, abUSBCmdVCode[32] = { 0 };
	char szBuf[64] = "";
	int nChildRet = -1;
	int nRet = -100;

	while (1) {
		memcpy(cb, "\xfe\x6f\x04\x10", 4);  // 1
		if ((nChildRet = NisecLogicIORaw(hDev->hUSB, cb, 4, sizeof(cb), NULL, 0)) != 16)
		{
			nRet = DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
			break;
		}
		memcpy(abPreHashBegin16, cb, 16);
		memcpy(abStep2_16BytesCMD, "\xfe\x6f\x07\x01", 4);
		memcpy(abStep2_16bytesParament, "\x00\x00\x80", 3);

		sprintf(szBuf, "66%s", hDev->szDeviceID);
		if (CntaxBuild16ByteCryptKeyAnd4ByteVCode(szBuf, DEF_CERT_PASSWORD_DEFAULT, abPreHashBegin16,
			abStep2_16BytesCMD, abStep2_16bytesParament,
			abCryptKey, abUSBCmdVCode) < 0)
		{
			nRet = DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
			break;
		}

		memset(cb, 0, sizeof(cb));
		memcpy(cb, abStep2_16BytesCMD, 16);
		memcpy(cb + 11, abUSBCmdVCode, 4);
		ClearLastError(hDev->hUSB);
		if ((nChildRet = NisecLogicIORaw(hDev->hUSB, cb, 16, sizeof(cb), abCryptKey, 0x10)) < 0) {
			nRet = 0 - PasswordNotice(false, hDev->hUSB->szLastErrorDescription);
			*left_num = -nRet;
			SetLastError(hDev->hUSB, ERR_CERT_PASSWORD_ERR,
				"֤��������󣬿���ԭ��Ϊδ�޸�Ĭ��֤�����12345678,ʣ�ೢ�Դ���%d", *left_num);

			logout(INFO, "TAXLIB", "������֤", "�̺ţ�%s,������Ϣ��%s\r\n", hDev->szDeviceID,
				hDev->hUSB->errinfo);
			//report_event(hDev->szDeviceID, "֤��������", hDev->hUSB->errinfo, nRet);
			nRet = ERR_CERT_PASSWORD_ERR;
			break;
		}
		nRet = 1;
		break;
	}
	if (nRet < 0)
		_WriteLog(LL_FATAL, "CNTAX_Crypt_Password failed");
	return nRet;
}



//ע��!!!���ǵ����ӿ�,ֻ�ܴ�USB handle�󵥶�����.�������������̣���Ϊ�޸���֮����Ҫ���������豸
//֤�������޸�һ�㲻���ã���Ϊ֤�������ѱ���Ʊ������ԣ��޸�֤��Ľӿڽ����ڿͻ�ͳһ��������ʹ��
//һ��ʹ��bDeviceOrCert=true
int cntax_change_certpassword(HDEV hDev, char *szOldPassword, char *szNewPassword, char *errinfo)//ע��!!!���ǵ����ӿ�,ֻ�ܴ�USB handle�󵥶�����.�������������̣���Ϊ�޸���֮����Ҫ���������豸
{
	//����cntax��20210202�汾֮��ǿ��Ҫ��ͳһ֤����豸�����˵��
	//������ָ���汾֮���豸����ʱ��֤�豸��֤������Ƿ�һ�£������һ����ǿ��Ҫ���û��޸�����
	//�޸�ͳһ��������2��������Ŀ򣬷ֱ�Ϊ��֤������豸���룬ʵ����֤������������Ǳ����Եģ�ֻ�����豸��������Ч��
	// cntax��ǿ�ƽ�֤�������Ϊ���豸����һ�£������豸������Ϊ��ʵ�豸���룬������Իͳһ�豸����
	_WriteLog(LL_INFO, "Try to set new cert's password:%s", szNewPassword);
	ClearLastError(hDev->hUSB);
	if (strcmp(szNewPassword, DEF_CERT_PASSWORD_DEFAULT)!=0) {
		_WriteLog(LL_FATAL, "Can not set new password equal with defalt password '88888888'");
		sprintf(errinfo, "��ǰϵͳ�������޸�����Ϊ12345678");
		return -1;
	}
	if (strcmp(szOldPassword, szNewPassword) == 0) {
		sprintf(errinfo, "�������������һ��");
		return -2;
	}
	if (CheckHeadTailSpace(szNewPassword)) {
		_WriteLog(LL_FATAL, "Head and tail cannot been 0x20");
		sprintf(errinfo, "����ǰ�󲻵��пո�");
		return -4;
	}

	if (EncryptOpenDevice(hDev->hUSB, hDev->bDeviceType, hDev->szDeviceID) < 0) {
		_WriteLog(LL_FATAL, "EncryptOpenDevice failed");
		return -1;
	}
	if (CntaxChangeCertPassword(hDev->hUSB, hDev->szDeviceID, szOldPassword,	szNewPassword) < 0) {
			_WriteLog(LL_FATAL, "Crypt login failed, cannot modify new password");
			return -1;
	}
	_WriteLog(LL_INFO, "Cert's password has been set,old:[%s] -> new:[%s]", szOldPassword,szNewPassword);
	return 0;

}

int cntax_read_inv_coil_number_history(HDEV hDev, char **data_json, char *errinfo)
{
	HUSB hUSB = hDev->hUSB;
	char tmp_s[50] = {0};
	struct StStockInfoHead *stMIInf =
		(struct StStockInfoHead *)malloc(sizeof(struct StStockInfoHead));
	int nTypeNum = hDev->abSupportInvtype[0];
	//int i = 0;
	int type_count;
	int coil_count;
	ClearLastError(hDev->hUSB);
	if (CntaxEntry(hUSB, NOP_OPEN_FULL) < 0)
	{
		free(stMIInf);
		return -1;
	}

	cJSON *json = cJSON_CreateObject();
	cJSON *data_array ,*coils_array;		//���ʹ��
	cJSON_AddItemToObject(json, "coils_histroy", data_array = cJSON_CreateArray());
	for (type_count = 0; type_count < nTypeNum; type_count++)
	{
		
		cJSON *data;
		cJSON_AddItemToObject(data_array, "dira", data = cJSON_CreateObject());
		uint8 bInvType = hDev->abSupportInvtype[type_count + 1];
		memset(tmp_s, 0, sizeof(tmp_s));
		sprintf(tmp_s, "%03d", bInvType);
		cJSON_AddStringToObject(data, "fplxdm", tmp_s);


		cJSON_AddItemToObject(data, "coils", coils_array = cJSON_CreateArray());

		if (CntaxSelectInvoiceType(hDev, bInvType, SELECT_OPTION_TEMPLATE) < 0)
		{
			continue;
		}
		memset(stMIInf, 0, sizeof(struct StStockInfoHead));
		int nRet = CntaxGetStockInfo(hDev, stMIInf);
		if (nRet < 0) 
		{
			//printf("GetStockInfo-type:%d nRet:%d\n", bInvType, nRet);
			continue;
		}


		//out("��Ʊ����%3d���ڴ����%d\n", bInvType, mem_i);
		//printf("\n��Ʊ����:\t\t\t%d\n��ǰ��Ʊ����(dqfpdm):\t\t%s\n��ǰ��Ʊ����(dqfphm)"
		//	":\t\t%"
		//	"s\n��Ʊʣ����Ŀ(zsyfs):\t\t%d\n���ط�Ʊ��Ŀ(nGroupCount):\t%d\n"
		//	"-----------------------------------------------------------------\n",
		//	bInvType, stMIInf->dqfpdm, stMIInf->dqfphm, stMIInf->zsyfs, stMIInf->nGroupCount);

		
		uint8 *pArrayCount = stMIInf->pArray_StStockInfoGroup;
		for (coil_count = 0; coil_count < stMIInf->nGroupCount; coil_count++) {

			cJSON *coils;
			cJSON_AddItemToObject(coils_array, "dira", coils = cJSON_CreateObject());
			struct StStockInfoGroup *pSI =
				(struct StStockInfoGroup *)(pArrayCount + coil_count * sizeof(struct StStockInfoGroup));
			/*printf("���:%d\n��Ʊ����(fpdm):%s\n��ʼ����(qshm):%"
			"s\n��ֹ����(zzhm):%s\n��Ʊ����(fpfs):%d\nʣ�����(syfs):%"
			"d\n�칺����(lgrq):%s\n�칺��Ա(lgry):%s\n\n",
			count + 1, pSI->fpdm, pSI->qshm, pSI->zzhm, pSI->fpfs, pSI->syfs, pSI->lgrq,
			pSI->lgry);*/

			//_WriteLog(LL_INFO, "���:%d\n", coil_count + 1);
			//_WriteLog(LL_INFO, "��Ʊ����(fpdm):%s\n", pSI->fpdm);
			//_WriteLog(LL_INFO, "��ʼ����(qshm):%s\n", pSI->qshm);
			//_WriteLog(LL_INFO, "��ֹ����(zzhm):%s\n", pSI->zzhm);
			//_WriteLog(LL_INFO, "��Ʊ����(fpfs):%d\n", pSI->fpfs);
			//_WriteLog(LL_INFO, "ʣ�����(syfs):%d\n", pSI->syfs);
			//_WriteLog(LL_INFO, "�칺����(lgrq):%s\n", pSI->lgrq);
			//_WriteLog(LL_INFO, "�칺��Ա(lgry):%s\n", pSI->lgry);

			cJSON *root = cJSON_CreateObject();
			memset(tmp_s, 0, sizeof(tmp_s));			sprintf(tmp_s, "%d", coil_count + 1);
			cJSON_AddStringToObject(root, "fpjh", tmp_s);
			cJSON_AddStringToObject(root, "fplbdm", pSI->fpdm);
			cJSON_AddStringToObject(root, "fpqshm", pSI->qshm);

			unsigned int fpqshm,fpzzhm;
			fpqshm = atoi(pSI->qshm);
			fpzzhm = fpqshm + pSI->fpfs - 1;
			memset(tmp_s, 0, sizeof(tmp_s));			sprintf(tmp_s, "%08d", fpzzhm);
			cJSON_AddStringToObject(root, "fpzzhm", tmp_s);
			memset(tmp_s, 0, sizeof(tmp_s));			sprintf(tmp_s, "%d", pSI->fpfs);
			cJSON_AddStringToObject(root, "fpgmsl", tmp_s);
			memset(tmp_s, 0, sizeof(tmp_s));			sprintf(tmp_s, "%d", pSI->syfs);
			cJSON_AddStringToObject(root, "fpsyfs", tmp_s);
			cJSON_AddStringToObject(root, "fpgmsj", pSI->lgrq);
			cJSON_AddStringToObject(root, "fpgmry", pSI->lgry);

			char *json_buf;
			char *base_buf;
			json_buf = cJSON_Print(root);
			//_WriteLog(LL_INFO, "%s", json_buf);
			base_buf = malloc(strlen(json_buf) * 2);
			Base64_Encode(json_buf, strlen(json_buf), base_buf);
			free(json_buf);
			cJSON_Delete(root);
			cJSON_AddStringToObject(coils, "data", base_buf);
			free(base_buf);

		}
		//out("�÷�Ʊ��������з�Ʊ���Ѷ���,��Ʊ��������%d\n", plate_info->invoice_type_infos[mem_i].invoice_coils_info.wsyfpjsl);
		//printf("================================================================\n");
		if (NULL != stMIInf->pArray_StStockInfoGroup)
			free(stMIInf->pArray_StStockInfoGroup);
	}

	char *g_buf;
	g_buf = cJSON_Print(json);
	_WriteLog(LL_INFO,"�������ݣ�%s",g_buf);
	char *base_buf;
	base_buf = malloc(strlen(g_buf) * 2);
	Base64_Encode(g_buf, strlen(g_buf), base_buf);
	free(g_buf);
	cJSON_Delete(json);
	*data_json = base_buf;

	free(stMIInf);
	if (CntaxEntry(hUSB, NOP_CLOSE) < 0)
		return -2;
	return 0;

}


int cntax_query_invoice_month_all_data(HDEV hDev, char *month, char **data_json, char *errinfo)
{
	int result;
	char tmp_s[50] = { 0 };
	char start_date[20] = { 0 };
	char end_date[20] = { 0 };
	ClearLastError(hDev->hUSB);
	//����ǵ�ǰ���򵽽����ֹ
	result = LoadCntaxDevInfo(hDev, hDev->hUSB, true);
	if (result < 0)
	{
		return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
	}
	turn_month_to_date_range(month, hDev->szDeviceTime, start_date, end_date);
	//_WriteLog(LL_INFO, "Ҫ��ѯ�����ڷ�ΧΪ��%s %s,��ǰʱ��Ϊ%s", start_date, end_date, hDev->szDeviceTime);

	cJSON *dir2, *dir3;		//���ʹ��
	cJSON *json = cJSON_CreateObject();
	cJSON_AddItemToObject(json, "statistics", dir2 = cJSON_CreateArray());
	//cJSON_AddItemToObject(dir2, "dira", dir3 = cJSON_CreateObject());

	struct StMonthStatistics *st_ok = NULL;
	struct StMonthStatistics st_now;
	struct StMonthStatistics st;
	int nTypeCount = hDev->abSupportInvtype[0];
	int i = 0;
	for (i = 0; i < nTypeCount; i++)
	{  //ר��Ʊ��ΪרƱ�����ӻ�������Ϊ������
		uint8 bInvType = hDev->abSupportInvtype[i + 1];
		struct StMonthStatistics *nextNode = NULL;

		memset(tmp_s, 0, sizeof(tmp_s));
		sprintf(tmp_s, "%03d", bInvType);
		cJSON_AddItemToObject(dir2, "dira", dir3 = cJSON_CreateObject());
		cJSON_AddStringToObject(dir3, "fplxdm", tmp_s);

		if (CntaxEntry(hDev->hUSB, NOP_OPEN_FULL) < 0)
		{
			_WriteLog(LL_INFO, "����ʧ��\n");
			cJSON_AddStringToObject(dir3, "data", "");
			continue;
		}
		if (CntaxSelectInvoiceType(hDev, bInvType, SELECT_OPTION_TEMPLATE) < 0)
		{
			cJSON_AddStringToObject(dir3, "data", "");
			continue;
		}

		memset(&st_now, 0, sizeof(struct StMonthStatistics));
		memset(&st, 0, sizeof(struct StMonthStatistics));
		//_WriteLog(LL_INFO, "��ȡ%03d��Ʊ���ͻ�������\n", bInvType);
	

		if (CntaxGetMonthStatistics(hDev->hUSB, false, &st_now) < 0) {
			_WriteLog(LL_WARN, "GetMonthStatistics failed, fplx:%d", bInvType);
		}

		if (CntaxGetMonthStatistics(hDev->hUSB, true, &st) < 0) {
			_WriteLog(LL_WARN, "GetMonthStatistics failed, fplx:%d", bInvType);
		}

		if ((memcmp(start_date, st_now.qssj, 6) == 0) && (memcmp(end_date, st_now.jzsj, 6) == 0))
		{
			st_ok = &st_now;
		}
		//_WriteLog(LL_INFO, "Ҫ��ѯ�ĵ��ڷ�ΧΪ��%s %s", st.qssj, st.jzsj);
		else if ((memcmp(start_date, st.qssj, 6) == 0) && (memcmp(end_date, st.jzsj,6) == 0))
		{
			st_ok = &st;
		}
		else
		{
			nextNode = st.next;
			while (nextNode)
			{
				//_WriteLog(LL_WARN, "��ֹʱ��%s %s", nextNode->qssj, nextNode->jzsj);
				if ((memcmp(start_date, nextNode->qssj,6) == 0) && (memcmp(end_date, nextNode->jzsj,6) == 0))
				{
					_WriteLog(LL_WARN, "�ҵ���Ӧ�·����ݣ���ֹʱ��%s %s", nextNode->qssj, nextNode->jzsj);
					st_ok = nextNode;
					break;
				}
				struct StMonthStatistics *tmp = nextNode->next;
				nextNode = tmp;
			}
		}
		CntaxEntry(hDev->hUSB, NOP_CLOSE);

		

		if (st_ok != NULL)
		{
			cJSON *root = cJSON_CreateObject();
			cJSON_AddStringToObject(root, "qssj", start_date);//��ʼʱ�� yyyymmdd
			cJSON_AddStringToObject(root, "jzsj", end_date);//��ֹʱ�� yyyymmdd
			cJSON_AddStringToObject(root, "qckcfs", st_ok->qckcfs);//�ڳ����
			cJSON_AddStringToObject(root, "lgfpfs", st_ok->lgfpfs);//�칺��Ʊ���� 
			cJSON_AddStringToObject(root, "thfpfs", st_ok->thfpfs);//�˻ط�Ʊ���� 

			int zsfpfs = atoi(st_ok->zsfpfs) + atoi(st_ok->zffpfs) + atoi(st_ok->kffpfs);
			memset(tmp_s, 0, sizeof(tmp_s));
			sprintf(tmp_s, "%d", zsfpfs);
			cJSON_AddStringToObject(root, "zsfpfs", tmp_s);//������Ʊ���� 
			//cJSON_AddStringToObject(root, "zsfpfs", st_ok->zsfpfs);//������Ʊ���� 


			int zffpfs = atoi(st_ok->zffpfs) + atoi(st_ok->kffpfs);
			memset(tmp_s, 0, sizeof(tmp_s));
			sprintf(tmp_s, "%d", zffpfs);
			cJSON_AddStringToObject(root, "zffpfs", tmp_s);//���Ϸ�Ʊ���� 

			int fsfpfs = atoi(st_ok->fsfpfs) + atoi(st_ok->fffpfs);
			memset(tmp_s, 0, sizeof(tmp_s));
			sprintf(tmp_s, "%d", fsfpfs);
			cJSON_AddStringToObject(root, "fsfpfs", tmp_s);//������Ʊ���� 

			//cJSON_AddStringToObject(root, "fsfpfs", st_ok->fsfpfs);//������Ʊ���� 
			cJSON_AddStringToObject(root, "fffpfs", st_ok->fffpfs);//���Ϸ�Ʊ���� 
			cJSON_AddStringToObject(root, "kffpfs", st_ok->kffpfs);//�շϷ�Ʊ���� 
			cJSON_AddStringToObject(root, "qmkcfs", st_ok->qmkcfs);//��ĩ������ 
			cJSON_AddStringToObject(root, "zsfpljje", st_ok->zsfpljje);//������Ʊ�ۼƽ�� 
			cJSON_AddStringToObject(root, "zsfpljse", st_ok->zsfpljse);//������Ʊ�ۼ�˰�� 
			cJSON_AddStringToObject(root, "zffpljje", st_ok->zffpljje);//���Ϸ�Ʊ�ۼƽ�� 
			cJSON_AddStringToObject(root, "zffpljse", st_ok->zffpljse);//���Ϸ�Ʊ�ۼ�˰�� 
			cJSON_AddStringToObject(root, "fsfpljje", st_ok->fsfpljje);//������Ʊ�ۼƽ�� 
			cJSON_AddStringToObject(root, "fsfpljse", st_ok->fsfpljse);//������Ʊ�ۼ�˰�� 
			cJSON_AddStringToObject(root, "fffpljje", st_ok->fffpljje);//���Ϸ�Ʊ�ۼƽ�� 
			cJSON_AddStringToObject(root, "fffpljse", st_ok->fffpljse);//���Ϸ�Ʊ�ۼ�˰�� 

			long double sjxsje = strtold(st_ok->zsfpljje, NULL) - strtold(st_ok->fsfpljje, NULL);
			memset(tmp_s, 0, sizeof(tmp_s));
			sprintf(tmp_s, "%4.2Lf", sjxsje);
			//PriceRound(tmp_s, 2, tmp_s);
			//_WriteLog(LL_INFO, "ʵ�����۽��%s    %f", tmp_s, sjxsje);
			cJSON_AddStringToObject(root, "sjxsje", tmp_s);//ʵ�����۽�� 
			long double sjxsse = strtold(st_ok->zsfpljse, NULL) - strtold(st_ok->fsfpljse, NULL);
			memset(tmp_s, 0, sizeof(tmp_s));
			sprintf(tmp_s, "%4.2Lf", sjxsse);
			//PriceRound(tmp_s, 2, tmp_s);
			cJSON_AddStringToObject(root, "sjxsse", tmp_s);//ʵ������˰�� 

			char *json_buf;
			char *base_buf;
			json_buf = cJSON_Print(root);
			//_WriteLog(LL_INFO, "%s", json_buf);
			base_buf = malloc(strlen(json_buf) * 2);
			Base64_Encode(json_buf, strlen(json_buf), base_buf);
			free(json_buf);
			cJSON_Delete(root);
			cJSON_AddStringToObject(dir3, "data", base_buf);
			free(base_buf);
			st_ok = NULL;
		}
		else
		{
			cJSON_AddStringToObject(dir3, "data", "");
		}
		nextNode = st.next;
		while (nextNode)
		{
			struct StMonthStatistics *tmp = nextNode->next;
			free(nextNode);
			nextNode = tmp;
		}

	}
	char *g_buf;
	g_buf = cJSON_Print(json);
	//_WriteLog(LL_INFO,"�������ݣ�%s",g_buf);
	char *base_buf;
	base_buf = malloc(strlen(g_buf) * 2);
	Base64_Encode(g_buf, strlen(g_buf), base_buf);
	free(g_buf);
	cJSON_Delete(json);
	*data_json = base_buf;
	return 0;
}

int cntax_check_server_address(HDEV hDev,char **splxxx, char *errinfo)
{
	struct HTTP hi;
	int result = 0;
	memset(&hi, 0, sizeof(struct HTTP));
	ClearLastError(hDev->hUSB);
	LoadCntaxDevInfo(hDev, hDev->hUSB, 1);
	if (hDev->bTrainFPDiskType)
	{
		sprintf(errinfo, "�����̲�֧�ֲ��Է��������ӹ���");
		return -1;
	}
	if ((result = CreateServerTalk(&hi, hDev)) < 0)
	{
		HTTPClose(&hi);
		_WriteLog(LL_DEBUG, "�ֶ����Ӳ���, ��һ�β��ɹ�,���Եڶ���");
		if ((result = CreateServerTalk(&hi, hDev)) < 0)
		{
			sprintf(errinfo, "%s", hDev->hUSB->szLastErrorDescription);
			_WriteLog(LL_DEBUG, "�ֶ����Ӳ���, %s", errinfo);
		}
	}
#ifndef MIPS_MTK
	_WriteLog(LL_DEBUG, "������ҵ��Ϣͬ��");
	result = SyncCompanyInfo(&hi, true);
	if (result == 0)
	{
		_WriteLog(LL_DEBUG, "ͬ����ҵ�����Ϣ���, %s", hDev->szCompanyName);
	}

	char *tmp = CntaxPreCompanyXml(hDev);
	if (tmp != NULL)
	{
		char *tmp_gbk;
		int len = strlen(tmp) * 2 + 1024;
		tmp_gbk = calloc(1, len);

		EncordingConvert("utf-8", "gbk", tmp, strlen(tmp), tmp_gbk, len);
		free(tmp);

		char *tmp_base;
		len = strlen(tmp_gbk) * 2 + 1024;
		tmp_base = calloc(1, len);

		
		Base64_Encode(tmp_gbk, strlen(tmp_gbk), tmp_base);
		free(tmp_gbk);
		*splxxx = tmp_base;
	}
#endif

	HTTPClose(&hi);
	return result;
}

int cntax_is_exsist_offline_inv(HDEV hDev, char *fpdm, char *fphm, char *kpsj, int *dzsyh)
{
	//printf_array(abQueryDataRange,8);
	ClearLastError(hDev->hUSB);
	uint8 abQueryDataCount[4] = { 0x00, 0x00, 0x00, 0x01 };
	uint8 abInvoiceTypes[] = { 4, 7, 26,28};
	int i = 0;
	uint8 *pRetBuff = NULL;
	int offlinenum = 0;
	//int nUploadOK = 0;
	uint8 bInvType = 0;
	//out("��ʼ��ȡ˰��������Ʊ����\n");
	if (CntaxEntry(hDev->hUSB, NOP_OPEN_FULL) < 0)
		return -1;
	for (i = 0; i < sizeof(abInvoiceTypes); i++)
	{
		bInvType = abInvoiceTypes[i];
		if (!CheckDeviceSupportInvType(hDev->abSupportInvtype, bInvType))
			continue;  // not support
		//�ȴ������ϣ����û������Ʊ����������̼��δ�ϴ���Ʊ		

		if (CntaxSelectInvoiceType(hDev, bInvType, SELECT_OPTION_TEMPLATE) < 0) {
			break;
		}
		int nRetBuffLen = 0;

		int nInvNum = CntaxGetInvBuff(hDev->hUSB, 2, &pRetBuff, &nRetBuffLen,
			abQueryDataCount, sizeof abQueryDataCount, NULL);
		//_WriteLog(LL_FATAL, "%03dƱ�ֶ�ȡ�����������,��������Ϊ%d\n", bInvType, nInvNum);

		if (nInvNum > 0)
		{
			offlinenum = 1;
			break;
		}



		uint8 cb[256] = { 0 };
		int nChildRet = 0;
		memcpy(cb, "\xfe\x17\x30", 3);
		nChildRet = CntaxLogicIO(hDev->hUSB, cb, 3, sizeof(cb), NULL, 0);
		if (nChildRet < 15)
			continue;
		nInvNum = bswap_32(*(uint32 *)cb);
		if (nInvNum>0)
		{
			_WriteLog(LL_FATAL, "%03dƱ�ִ�������δ�ϴ���Ʊ,��������Ϊ%d\n", bInvType, nInvNum);
			offlinenum = 1;
			break;
		}

	}
	if (pRetBuff != NULL)
		free(pRetBuff);
	CntaxEntry(hDev->hUSB, NOP_CLOSE);
	//_WriteLog(LL_FATAL, "˰����������������Ϊ%d\n", offlinenum);
	return offlinenum;
}





//<0�����账������ֵ==0,������Ҫ�ϱ���Ʊ�������ϱ�ʧ�ܣ�>0 ����һ�ţ��ϱ����
int cntax_inv_upload_server(HDEV hDev, struct _offline_upload_result *offline_upload_result)
{
	int result = -1;
	ClearLastError(hDev->hUSB);
	result = LoadCntaxDevInfo(hDev, hDev->hUSB, true);
	if (result < 0)
	{
		_WriteLog(LL_INFO, "LoadCntaxDevInfo result = %d\n", result);
		return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
	}
	uint8 abQueryDataCount[4] = { 0x00, 0x00, 0x00, 0x01 };
	// uint8 abInvoiceTypes[] = {4, 7, 26};
	uint8 abInvoiceTypes[] = { 4, 7, 26,28 };
	int i = 0, nRet = -10;
	uint8 *pRetBuff = NULL;
	uint8 bInvType = 0;
	char  szCurFPDM[64] = "", szCurFPHM[64] = "", szResultDes[128] = "";
	if (hDev->bTrainFPDiskType != 0)
	{
		_WriteLog(LL_INFO, "[+]�����̲���Ҫ��Ʊ�ϴ�\n\n\n");
		sprintf((char *)offline_upload_result->errinfo, "�����̲���Ҫ��Ʊ�ϴ�");
		return 99;
	}
	for (i = 0; i < sizeof(abInvoiceTypes); i++)
	{
		bInvType = abInvoiceTypes[i];
		if (!CheckDeviceSupportInvType(hDev->abSupportInvtype, bInvType))
			continue;  // not support
		_WriteLog(LL_FATAL, "nisec_inv_upload_server ֻҪ���������������۳ɹ��������ѭ��");
		if ((nRet = CntaxQueryModifyInvoice(bInvType, hDev, szCurFPDM, szCurFPHM)) >= 0)
		{
			_WriteLog(LL_FATAL, "�������Ϸ�Ʊ���ϴ��ɹ�\n");
			offline_upload_result->fplxdm = bInvType;
			memcpy(offline_upload_result->fpdm, szCurFPDM, strlen(szCurFPDM));
			memcpy(offline_upload_result->fphm, szCurFPHM, strlen(szCurFPHM));
			break;
		}
		nRet = -10;
		//�ȴ������ϣ����û������Ʊ����������̼��δ�ϴ���Ʊ
		if (CntaxEntry(hDev->hUSB, NOP_OPEN_FULL) < 0)
			break;
		if (CntaxSelectInvoiceType(hDev, bInvType, SELECT_OPTION_TEMPLATE) < 0) {
			CntaxEntry(hDev->hUSB, NOP_CLOSE);
			break;
		}

		int nRetBuffLen = 0;
		int nInvNum = CntaxGetInvBuff(hDev->hUSB, 2, &pRetBuff, &nRetBuffLen,
			abQueryDataCount, sizeof abQueryDataCount, NULL);
		_WriteLog(LL_FATAL, "ȡ��Ʊ���������ݳɹ�\n");
		CntaxEntry(hDev->hUSB, NOP_CLOSE);
		if (0 == nInvNum) {
			_WriteLog(LL_DEBUG, "No invoice in this range of date, Invoice's type:%d", bInvType);
			nRet = 100;
			goto NextFPType;
		}
		if (nInvNum < 0) {
			if (strstr(hDev->hUSB->szLastErrorDescription, "09d106"))
			{
				_WriteLog(LL_INFO, "No invoice to upload, Invoice's type:%d", bInvType);
				nRet = 100;
			}
			else
				_WriteLog(LL_FATAL, "CntaxGetInvBuff GetInvBuff-type:%d nRet:%d\n", bInvType, nInvNum);
			goto NextFPType;
		}
		//   _WriteHexToDebugFile("nisec_usbraw.bin", pRetBuff, nRetBuffLen);
		//nRet = CntaxGetInvDetail(hDev, FPCX_COMMON_UPLOAD, pRetBuff, 1, CntaxUploadInvoiceCallBack);

		HFPXX fpxx = MallocFpxxLinkDev(hDev);
		if (!fpxx)
		{
			_WriteLog(LL_FATAL, "MallocFpxxLinkDev fpxx mem errfo\n");
			goto NextFPType;
		}
		uint8 *pBlockBin = pRetBuff;
		int index_len;
		int nChildRet = CntaxInvRaw2Fpxx(hDev, FPCX_COMMON_OFFLINE, pBlockBin, fpxx, &index_len);
		if (nChildRet < 0) {
			_WriteLog(LL_FATAL, "���߷�Ʊ���ݽ���ʧ��,nChildRet = %d\n", nChildRet);
			FreeFpxx(fpxx);
			goto NextFPType;
		}

		char szDZSYH[256] = "";
		uint8 abTransferData[256] = { 0 };
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
			_WriteLog(LL_FATAL, "Update invoice's sign success");
			if (CntaxInvoiceUploadAndDownload(fpxx, szDZSYH)) {
				if (strlen(hDev->hUSB->szLastErrorDescription) < sizeof(offline_upload_result->errinfo))
					strcpy((char *)offline_upload_result->errinfo, hDev->hUSB->szLastErrorDescription);
				_WriteLog(LL_FATAL, "CntaxInvoiceUploadAndDownload,netio failed, step1,nRet = %d,errinfo [%s]", nRet,offline_upload_result->errinfo);
				nRet = -1;
				
				if (strstr((char *)offline_upload_result->errinfo, "������Ʊδ����ȷ������"))
				{
					logout(INFO, "TAXLIB", "��Ʊ�ϴ�", "�̺ţ�%s,������Ϣ��%s\r\n", fpxx->hDev->szDeviceID, (char *)offline_upload_result->errinfo);
				}

				if (strstr((char *)offline_upload_result->errinfo, "ǩ����֤����"))
				{
					char report_data[1024] = { 0 };
					sprintf(report_data, "ǩ����֤����,��Ʊ���룺%s,��Ʊ���룺%s,��Ʊʱ�䣺%s,����Ʊ�֣�%d", fpxx->fpdm, fpxx->fphm, fpxx->kpsj_F2, fpxx->zyfpLx);
					logout(INFO, "TAXLIB", "��Ʊ�ϴ�", "�̺ţ�%s,������Ϣ��%s\r\n", fpxx->hDev->szDeviceID, report_data);
					report_event(fpxx->hDev->szDeviceID, "��ǩʧ��", report_data, -1);
				}
				break;
			}
			//_WriteLog(LL_FATAL, "�����ݲ�����step 2��errinfo =%s", offline_upload_result->errinfo);
			_WriteLog(LL_WARN, "Nisec upload invoice,NisecInvoiceUploadAndDownload, step1 success");
			int nTransferDataLen = Str2Byte(abTransferData, szDZSYH, strlen(szDZSYH));
			if (CntaxUpdateInvUploadFlag(fpxx->hDev, 2, abTransferData, nTransferDataLen) < 0) {
				_WriteLog(LL_WARN, "Upload invoice, updateflag failed, step2");
				break;
			}
			logout(INFO, "TAXLIB", "��Ʊ�ϴ�", "�̺�:%s,���߷�Ʊ�ϴ��ɹ�2,��Ʊ������룺%s_%s\r\n", fpxx->hDev->szDeviceID, fpxx->fpdm, fpxx->fphm);
			nRet = 1;
			break;
		}

		offline_upload_result->fplxdm = fpxx->fplx;
		offline_upload_result->zfbz = fpxx->zfbz;
		memcpy(offline_upload_result->fpdm, fpxx->fpdm, strlen(fpxx->fpdm));
		memcpy(offline_upload_result->fphm, fpxx->fphm, strlen(fpxx->fphm));
		//memcpy(offline_upload_result->kpsj, fpxx->kpsj_F2, strlen(fpxx->kpsj_F2));
		//memcpy(offline_upload_result->hjje, fpxx->je, strlen(fpxx->je));
		//memcpy(offline_upload_result->hjse, fpxx->se, strlen(fpxx->se));
		//memcpy(offline_upload_result->jshj, fpxx->jshj, strlen(fpxx->jshj));
		//memcpy(offline_upload_result->jym, fpxx->jym, strlen(fpxx->jym));
		//memcpy(offline_upload_result->mwq, fpxx->mw, strlen(fpxx->mw));
		//memcpy(offline_upload_result->bz, fpxx->bz, strlen(fpxx->bz));

		_WriteLog(LL_DEBUG, "Cntax upload invoice %s, fplx:%d fpdm:%s fphm:%s", nRet ? "successful" : "failed", fpxx->fplx, fpxx->fpdm, fpxx->fphm);
		//logout(INFO, "TAXLIB", "��Ʊ�ϴ�","nisec_inv_upload_server �̺�:%s,���߷�Ʊ�ϴ��ɹ�2,��Ʊ������룺%s_%s\n", fpxx->hDev->szDeviceID, offline_upload_result->fpdm, offline_upload_result->fphm);
		FreeFpxx(fpxx);
		//nRet = CntaxGetInvDetailUpload(hDev, bInvType, pRetBuff, 1, UploadInvoice_CallBack, offline_upload_result);
		//_WriteLog(LL_FATAL, "CntaxGetInvDetail nRet:%d\n", nRet);
		free(pRetBuff);
		pRetBuff = NULL;
		break;
	NextFPType:
		if (pRetBuff) {
			free(pRetBuff);
			pRetBuff = NULL;
		}
	}
	//nRet<0ʧ��   ==0��Ҫ����   == 100������Ʊ  ==1�ɹ�

	if (nRet <= 0)
	{
		strcpy(szResultDes, "Get inv or Update failed");
		if (strstr((char *)offline_upload_result->errinfo, "��Ʊ�ϴ������б������ӿ��ж�"))
		{
			_WriteLog(LL_INFO, "�ӿ��жϲ�����ȡ�������ϴ�\n");
		}
		else
		{
			offline_upload_result->need_retryup += 1;//�����ϴ�
		}
		nRet = -1;
	}
	else if (100 == nRet)
		strcpy(szResultDes, "Have no invoice");
	else  //== 1
	{
		strcpy(szResultDes, "successful");
		_WriteLog(LL_WARN, "��Ʊ�ϴ��ɹ�, fpdm:%s fphm:%s,�ٴβ�ѯ��У���ϴ���־", offline_upload_result->fpdm, offline_upload_result->fphm);



		HFPXX stp_fpxx = MallocFpxxLinkDev(hDev);
		int nChildRet = CntaxQueryInvInfo(hDev, offline_upload_result->fplxdm, (char *)offline_upload_result->fpdm, (char *)offline_upload_result->fphm, stp_fpxx);
		if (nChildRet < 0)
		{
			FreeFpxx(stp_fpxx);
			_WriteLog(LL_WARN, "�̺�:%s,��Ʊ�ϴ����ѯ��Ʊʧ��nChildRet: %d, fpdm:%s fphm:%s", hDev->szDeviceID, nChildRet, offline_upload_result->fpdm, offline_upload_result->fphm);
			return -3;
		}
		if (stp_fpxx->bIsUpload != 1)
		{
			sprintf((char *)offline_upload_result->errinfo,"�ϴ���־δ����");
			logout(INFO, "TAXLIB", "��Ʊ�ϴ�", "�̺�:%s,��Ʊ�ϴ����ѯ��Ʊ�ϴ�״̬ʧ��,�ϴ���־δ����, fpdm:%s fphm:%s\r\n", hDev->szDeviceID, offline_upload_result->fpdm, offline_upload_result->fphm);
			nRet = -1;
		}
		offline_upload_result->fplxdm = stp_fpxx->fplx;
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

	}
	_WriteLog(LL_INFO, "cntax_inv_upload_server Cntax upload single invoice finish,��%s�� fplx:%d fpdm:%s fphm:%s result:%s", hDev->szCompanyName, bInvType, offline_upload_result->fpdm, offline_upload_result->fphm, szResultDes);
	return nRet;
}



int cntax_fpdm_fphm_update_invs(HDEV hDev, char  *fpdm, char *fphm, char **inv_json)
{
	int result = -1;
	ClearLastError(hDev->hUSB);
	result = LoadCntaxDevInfo(hDev, hDev->hUSB, true);
	if (result < 0)
	{
		return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
	}
	int ret = -1;
	uint8 abInvoiceTypes[] = { 4, 7, 26,28 };
	int i = 0;
	HFPXX fpxx = MallocFpxx();
	for (i = 0; i < sizeof(abInvoiceTypes); i++)
	{
		uint8 bInvType = abInvoiceTypes[i];
		fpxx->hDev = hDev;

		//int nChildRet = CntaxQueryInvInfo(hDev, bInvType, fpdm, fphm, fpxx);
		//_WriteLog(LL_INFO,"ͨ����Ʊ����%s,��Ʊ����%s,��ȡ��Ʊ����\n", fpdm, fphm);
		if ((ret = CntaxQueryInvInfo(hDev, bInvType, fpdm, fphm, fpxx)) < 0)
		{
			//_WriteLog(LL_INFO,"%d��Ʊ����,��Ʊ��ȡʧ��,�л���һƱ�ֲ�ѯ,ret = %d\n", bInvType, ret);
			continue;
		}

		_WriteLog(LL_INFO, "��ȡ��Ʊ��Ϣ�ɹ�,fpxx->aisino = %d\n", fpxx->fplx_aisino);
		fpxx_to_json_base_fpsjbbh_v102(fpxx, inv_json, DF_FPQD_Y, DF_FP_BASE);
		ret = 0;
		break;
	}
	if (ret != 0)
	{
		_WriteLog(LL_INFO, "δ��ѯ����Ʊ,��Ʊ����%s,��Ʊ����%s\n", fpdm, fphm);
		if (fpxx)
		{
			//_WriteLog(LL_INFO,"�ͷŷ�Ʊ�����ָ��\n");
			FreeFpxx(fpxx);
		}
	}
	else
	{

		char szDZSYH[256] = "";
		uint8 abTransferData[256] = { 0 };
		int bOK = 0;
		while (1) {
			if (!strlen(fpxx->sign)) {
				if (CntaxFpxxAppendSignIO(fpxx, true)) {
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
			_WriteLog(LL_FATAL, "Update invoice's sign success");
			if (CntaxInvoiceUploadAndDownload(fpxx, szDZSYH)) {
				_WriteLog(LL_WARN, "Cntax upload invoice,netio failed, step1,errinfo [%s]", fpxx->hDev->hUSB->szLastErrorDescription);
				break;
			}
			_WriteLog(LL_WARN, "Cntax upload invoice,NisecInvoiceUploadAndDownload, step1 success");
			int nTransferDataLen = Str2Byte(abTransferData, szDZSYH, strlen(szDZSYH));
			if (CntaxUpdateInvUploadFlag(fpxx->hDev, 2, abTransferData, nTransferDataLen) < 0) {
				_WriteLog(LL_WARN, "Cntax upload invoice, updateflag failed, step2");
				break;
			}
			bOK = 1;
			break;
		}

		_WriteLog(LL_DEBUG, "Cntax upload invoice %s, fplx:%d fpdm:%s fpgm:%s",
			bOK ? "successful" : "failed", fpxx->fplx, fpxx->fpdm, fpxx->fphm);

		if (fpxx)
		{
			//_WriteLog(LL_INFO,"�ͷŷ�Ʊ�����ָ��\n");
			FreeFpxx(fpxx);
		}
	}
	return ret;
}





//���Ϸ�Ʊ-ָ����Ʊ���� ����
int cntax_waste_invoice(HDEV hDev, uint8 bInvType, char *szFPDM, char *szFPHM, struct _cancel_invoice_result *cancel_invoice_result)
{
	int result = -1;
	ClearLastError(hDev->hUSB);
	result = LoadCntaxDevInfo(hDev, hDev->hUSB, true);
	if (result < 0)
	{
		return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
	}
	_WriteLog(LL_INFO, "[+] Try Waste invoice: FPDM:%s FPHM:%s", szFPDM, szFPHM);
	HFPXX fpxx = MallocFpxxLinkDev(hDev);
	int nRet = -1;
	int nChildRet = -1;
	if ((bInvType != 4) && (bInvType != 7))
	{
		sprintf((char *)cancel_invoice_result->errinfo, "��Ʊ���Ͻ�֧����ͨ��Ʊ��ר�÷�Ʊ");
		return DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
	}
	while (1) {

		_WriteLog(LL_INFO, "���Ϸ�Ʊ-ָ����Ʊ������뿪ʼ��Ʊ����\n");
		if ((nChildRet = CntaxWasteInvoice(hDev, bInvType, false, szFPDM, szFPHM,fpxx->kpr, 1,&cancel_invoice_result->need_restart)) < 0)
		{
			_WriteLog(LL_INFO, "[-] Waste invoice failed: FPDM:%s FPHM:%s nChildRet = %d", szFPDM, szFPHM, nChildRet);
			sprintf((char *)cancel_invoice_result->errinfo, "�ײ�����ʧ��,%s", hDev->hUSB->szLastErrorDescription);
			FreeFpxx(fpxx);
			return DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
		}
		_WriteLog(LL_INFO, "���Ϸ�Ʊ-ָ����Ʊ������뷢Ʊ�������\n");
		nRet = 0;
		break;
	}
	if (!nRet)
		_WriteLog(LL_INFO, "[+] Waste invoice successful: FPDM:%s FPHM:%s", szFPDM, szFPHM);
	else
		_WriteLog(LL_INFO, "[+] Waste invoice failed: FPDM:%s FPHM:%s", szFPDM, szFPHM);

	if ((nChildRet = CntaxQueryInvInfo(hDev, bInvType, szFPDM, szFPHM, fpxx)) < 0) {
		sprintf((char *)cancel_invoice_result->errinfo, "������ɺ��ٴβ�ѯ��Ʊʧ��,������Ʊ�����ѯ�Ƿ����ϳɹ�");
		FreeFpxx(fpxx);
		return DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
	}
	cancel_invoice_result->fplxdm = bInvType;
	memcpy(cancel_invoice_result->fpdm, szFPDM, strlen(szFPDM));
	memcpy(cancel_invoice_result->fphm, szFPHM, strlen(szFPHM));
	memcpy(cancel_invoice_result->zfsj, fpxx->zfsj_F2, strlen(fpxx->zfsj_F2));
	memcpy(cancel_invoice_result->kpsj, fpxx->kpsj_F2, strlen(fpxx->kpsj_F2));
	//TranslateStandTime(2, fpxx->zfsj, (char *)cancel_invoice_result->zfsj);
	memcpy(cancel_invoice_result->hjje, fpxx->je, strlen(fpxx->je));
	memcpy(cancel_invoice_result->hjse, fpxx->se, strlen(fpxx->se));
	memcpy(cancel_invoice_result->jshj, fpxx->jshj, strlen(fpxx->jshj));
	memcpy(cancel_invoice_result->jym, fpxx->jym, strlen(fpxx->jym));
	memcpy(cancel_invoice_result->mwq, fpxx->mw, strlen(fpxx->mw));
	fpxx_to_json_base_fpsjbbh_v102(fpxx, &cancel_invoice_result->scfpsj, DF_FPQD_Y, DF_FP_JSON);
	FreeFpxx(fpxx);
	fpxx = NULL;
	return 0;
}

int cntax_cancel_null_invoice_to_plate(HDEV hDev, struct _cancel_invoice_result *cancel_invoice_result)
{
	int result = -1;
	ClearLastError(hDev->hUSB);
	result = LoadCntaxDevInfo(hDev, hDev->hUSB, true);
	if (result < 0)
	{
		return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
	}

	if ((cancel_invoice_result->fplxdm != FPLX_COMMON_PTFP) && (cancel_invoice_result->fplxdm != FPLX_COMMON_ZYFP))
	{
		sprintf((char *)cancel_invoice_result->errinfo, "��Ʊ���Ͻ�֧����ͨ��Ʊ��ר�÷�Ʊ");
		return DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
	}
	if ((strlen((char *)cancel_invoice_result->zfr) == 0) || (strlen((char *)cancel_invoice_result->zfr) > 16))
	{
		sprintf((char *)cancel_invoice_result->errinfo, "�����˳�������");
		return DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
	}
	result = IsGBK((char *)cancel_invoice_result->zfr);
	if (result != 1)
	{
		sprintf((char *)cancel_invoice_result->errinfo, "�����˱����ʽ����");
		return DF_TAX_ERR_CODE_USE_ILL_CHAR_OR_UTF8;//�����ʽ����
	}
	if (cancel_invoice_result->zfzs < 1)
	{
		sprintf((char *)cancel_invoice_result->errinfo, "������������");
		return DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
	}

	//char szFPDM[32] = "3200142140", szFPHM[24] = "369595";
	_WriteLog(LL_INFO, "Try Waste invoice: FPDM:%s FPHM:%s, ZFZS:%d", cancel_invoice_result->fpdm, cancel_invoice_result->fphm, cancel_invoice_result->zfzs);

	if ((result = CntaxWasteInvoice(hDev, cancel_invoice_result->fplxdm, true, (char *)cancel_invoice_result->fpdm, (char *)cancel_invoice_result->fphm, (char *)cancel_invoice_result->zfr, cancel_invoice_result->zfzs,&cancel_invoice_result->need_restart)) < 0)
	{
		sprintf((char *)cancel_invoice_result->errinfo, "�ײ�����ʧ��,%s", hDev->hUSB->szLastErrorDescription);
		//sprintf((char *)cancel_invoice_result->errinfo, "�ײ��������ʧ��");
		return DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
	}
	_WriteLog(LL_INFO, "���Ϸ�Ʊ-ָ����Ʊ������뷢Ʊ�������\n");

	return 0;
}

int cntax_copy_report_data(HDEV hDev, uint8 inv_type, char *errinfo)
{
	int result;
	ClearLastError(hDev->hUSB);
	if (hDev->bTrainFPDiskType != 0)
	{
		_WriteLog(LL_WARN, "����ʽ�̲�ִ�г����忨����");
		sprintf(errinfo, "�����̲����г����忨");
		return -1;
	}
	// if (FPLX_COMMON_ZYFP != inv_type &&	FPLX_COMMON_PTFP != inv_type
	// 	&&	FPLX_COMMON_DZFP != inv_type&&	FPLX_COMMON_DZZP != inv_type)
	// {
	// 	_WriteLog(LL_WARN, "�ݲ�֧��ר�յ�Ʊ֮���Ʊ�ֳ���,inv_type = %03d", inv_type);
	// 	sprintf(errinfo, "�ݲ�֧��ר�յ�Ʊ֮���Ʊ�ֳ���");
	// 	return -2;
	// }
	result = LoadCntaxDevInfo(hDev, hDev->hUSB, true);
	if (result < 0)
	{
		return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
	}

	//_WriteLog(LL_WARN, "��ʼִ��%03d��Ʊ����\n\n\n\n", inv_type);
			
	if ((result = CntaxReportSummarytMain(hDev,true)) < 0) {
		strcpy(errinfo, hDev->hUSB->errinfo);//�������洢��errinfo
		_WriteLog(LL_WARN, "��������ʧ��result = %d,errinfo = %s\n", result, errinfo);
		//report_event(hDev->szDeviceID, "��������ʧ��", (char *)errinfo, result);
		return -3;
	}

	// if ((result = CntaxReportSummarytMain(hDev, inv_type)) < 0) {
	// 	strcpy(errinfo, hDev->hUSB->errinfo);//�������洢��errinfo
	// 	_WriteLog(LL_WARN, "��������ʧ��result = %d,errinfo = %s\n", result, errinfo);
	// 	//report_event(hDev->szDeviceID, "��������ʧ��", (char *)errinfo, result);
	// 	return -3;
	// }
	return 0;
}

int cntax_report_clear(HDEV hDev, uint8 inv_type, char *errinfo)
{
	int result;
	ClearLastError(hDev->hUSB);
	if (hDev->bTrainFPDiskType != 0)
	{
		_WriteLog(LL_WARN, "�����̲����г����忨");
		sprintf(errinfo, "�����̲����г����忨");
		return -1;
	}
	// if (FPLX_COMMON_ZYFP != inv_type &&	FPLX_COMMON_PTFP != inv_type
	// 	&&	FPLX_COMMON_DZFP != inv_type&&	FPLX_COMMON_DZZP != inv_type)
	// {
	// 	_WriteLog(LL_WARN, "�ݲ�֧��ר��Ʊ��֮���Ʊ�ֳ���");
	// 	sprintf(errinfo, "�ݲ�֧��ר��Ʊ��֮���Ʊ�ֳ���");
	// 	return -2;
	// }
	result = LoadCntaxDevInfo(hDev, hDev->hUSB, true);
	if (result < 0)
	{
		return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
	}

	//�³����忨
	if ((result = CntaxReportSummarytMain(hDev,false)) < 0) {
		strcpy(errinfo, hDev->hUSB->errinfo);//�������洢��errinfo
		_WriteLog(LL_WARN, "��������ʧ��result = %d,errinfo = %s\n", result, errinfo);
		//report_event(hDev->szDeviceID, "��������ʧ��", (char *)errinfo, result);
		return -3;
	}

	//�ϵĳ����忨
	// if ((result = CntaxReportSummarytMain(hDev, inv_type)) < 0) {
	// 	strcpy(errinfo, hDev->hUSB->errinfo);//�������洢��errinfo
	// 	_WriteLog(LL_WARN, "��������ʧ��result = %d,errinfo = %s\n", result, errinfo);
	// 	//report_event(hDev->szDeviceID, "��������ʧ��", (char *)errinfo, result);
	// 	return -3;
	// }
	ClearLastError(hDev->hUSB);
	//�µ��忨
	if ((result = CntaxClearCardMain(hDev)) < 0) {
		strcpy(errinfo, hDev->hUSB->errinfo);//�������洢��errinfo
		_WriteLog(LL_WARN, "�忨ʧ��result = %d,errinfo = %s\n", result, errinfo);
		//report_event(hDev->szDeviceID, "�忨��дʧ��", (char *)errinfo, result);
		return -4;
	}

	//�ϵ��忨
	// if ((result = CntaxClearCardMain(hDev, inv_type)) < 0) {
	// 	strcpy(errinfo, hDev->hUSB->errinfo);//�������洢��errinfo
	// 	_WriteLog(LL_WARN, "�忨ʧ��result = %d,errinfo = %s\n", result, errinfo);
	// 	//report_event(hDev->szDeviceID, "�忨��дʧ��", (char *)errinfo, result);
	// 	return -4;
	// }
	_WriteLog(LL_WARN, "�忨�ɹ� \n\n\n", result);
	return 0;
}

// func0 ZYFP ���ֱ����벢��ȡ������Ϣ����
// func1 ZYFP Fpxx�ṹ����Ϣ�Ա���֤
int cntax_upload_check_redinvform(HDEV hDev, uint8 inv_type, char* inv_data, struct _plate_infos *plate_infos, struct _askfor_tzdbh_result *askfor_tzdbh_result)
{
	int result;
	int count;
	//uint8 bFuncNo = 1;
	int nRet = -10;
	char szTZDBH[36] = "";
	//int left_num = -1;
	ClearLastError(hDev->hUSB);
	HFPXX fpxx = MallocFpxxLinkDev(hDev);
	if (!fpxx) {
		nRet = -1;
		sprintf((char *)askfor_tzdbh_result->errinfo, "ϵͳ�ڴ�����ʧ��,������ܴ��ڹ���,����������");
		goto FreeAndExit;
	}
	if (inv_data != NULL)
	{
		str_replace(inv_data, "\\", "[@*br/*@]");//�Ų�json�е�\ת���ַ�
		result = check_invoice_str((unsigned char *)inv_data);
		if (result < 0)
		{
			FreeFpxx(fpxx);
			sprintf((char *)askfor_tzdbh_result->errinfo, "��ƱJSON�����д��������ַ��޷�����,����&��\\");
			return DF_TAX_ERR_CODE_USE_ILL_CHAR_OR_UTF8;
		}
		//result = judge_code(inv_data, strlen(inv_data));
		result = IsGBK(inv_data);
		if (result != 1)
		{
			FreeFpxx(fpxx);
			sprintf((char *)askfor_tzdbh_result->errinfo, "��ƱJSON���ݺ��ֱ����ʽ����,��GBK����");
			return DF_TAX_ERR_CODE_USE_ILL_CHAR_OR_UTF8;//�����ʽ����
		}

		fpxx->isHzxxb = true;
		//_WriteLog(LL_INFO, "������ƱJSON����,%s\n", inv_data);
		if ((result = AnalyzeJsonBuff(hDev->bDeviceType, inv_data, fpxx, (char *)askfor_tzdbh_result->errinfo)) != 0)
		{
			logout(INFO, "TAXLIB", "������Ϣ������", "��Ʊ����ʧ��,�������%d\r\n", result);
			FreeFpxx(fpxx);
			return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
		}
		if (fpxx->hzxxbsqsm != 0)
		{
			//������������°潻����������Ϣ�����
			if ((strcmp(fpxx->xfsh, hDev->szCommonTaxID) == 0) || (strcmp(fpxx->xfmc, hDev->szCompanyName) == 0))			{
				logout(INFO, "TAXLIB", "������Ϣ������", "���򷽷��������Ϣ������,����������Ϣ����\r\n", result);
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
		//���±�Ҫ˰����Ϣ����Ʊ��Ϣ
		fpxx->kpjh = hDev->uICCardNo;
		fpxx->hDev = hDev;
		strcpy(fpxx->jqbh, hDev->szDeviceID);
		//��ʽ�����ָ�ʽʱ��
		UpdateFpxxAllTime(hDev->szDeviceTime, fpxx);

		//result = check_fpxx_aisino(fpxx, plate_infos, (char *)askfor_tzdbh_result->errinfo);
		//if (result < 0)
		//{
		//	_WriteLog(LL_INFO, "check_fpxx_aisino failed\n");
		//	goto FreeAndExit;
		//}
		// update by type
		//if (DEVICE_AISINO == hDev->bDeviceType) {
		//	if (AisinoUpdateNodeInfo(fpxx, hDev))
		//	{
		//		_WriteLog(LL_INFO, "�滻�����̿�Ʊ���ݳ���\n");
		//		result = DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
		//		goto FreeAndExit;
		//	}
		//}
		//if (CheckFpxxNode(fpxx, hDev, (char *)askfor_tzdbh_result->errinfo))
		//{
		//	_WriteLog(LL_INFO, "���鷢Ʊ�����Ƿ���ȷ����\n");
		//	result = DF_TAX_ERR_CODE_INV_DATA_DISACCORD;
		//	goto FreeAndExit;
		//}
		memcpy(askfor_tzdbh_result->lzfpdm, fpxx->blueFpdm, strlen(fpxx->blueFpdm));
		memcpy(askfor_tzdbh_result->lzfphm, fpxx->blueFphm, strlen(fpxx->blueFphm));
	}
	else
	{
		fpxx->hDev = hDev;
		_WriteLog(LL_INFO, "ͨ����Ʊ����%s,��Ʊ����%s,��ȡ��Ʊ����\n", askfor_tzdbh_result->lzfpdm, askfor_tzdbh_result->lzfphm);
		if ((result = CntaxQueryInvInfo(hDev, inv_type, (char *)askfor_tzdbh_result->lzfpdm, (char *)askfor_tzdbh_result->lzfphm, fpxx)) < 0)
		{
			FreeFpxx(fpxx);
			_WriteLog(LL_INFO, "��Ʊ��ȡʧ��,ret = %d\n", result);
			sprintf((char *)askfor_tzdbh_result->errinfo, "δ�ҵ���Ʊ���룺%s,��Ʊ����%s,��Ӧ�ķ�Ʊ", askfor_tzdbh_result->lzfpdm, askfor_tzdbh_result->lzfphm);
			return DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
		}
		_WriteLog(LL_INFO, "QueryInvInfo ��Ʊ��ȡ�ɹ�\n");

		//��ʽ�����ָ�ʽʱ��

		memcpy(fpxx->ssyf, fpxx->kpsj_standard, 6);

		memset(fpxx->kpsj_standard, 0, sizeof fpxx->kpsj_standard);
		memset(fpxx->kpsj_F1, 0, sizeof fpxx->kpsj_F1);
		memset(fpxx->kpsj_F2, 0, sizeof fpxx->kpsj_F2);
		memset(fpxx->kpsj_F3, 0, sizeof fpxx->kpsj_F3);

		UpdateFpxxAllTime(hDev->szDeviceTime, fpxx);

		//_WriteLog(LL_INFO, "��Ʊ�����·ݣ�%s\n", fpxx->ssyf);
		//_WriteLog(LL_INFO, "��ǰ˰��ʱ�䣺%s\n", fpxx->kpsj_standard);

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
			strcpy(stp_Spxx->spmc, "�����Ӧ������Ʊ���嵥");
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

			//_WriteLog(LL_FATAL, "��Ʒ����=%d\n", Spxx_num);
			for (i = 0; i < Spxx_num; i++)
			{
				//_WriteLog(LL_FATAL,"�����%d����Ʒ\n",i+1);
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
					FreeFpxx(fpxx);
					_WriteLog(LL_INFO, "�ݲ�֧����Ʒ�з������з�Ʊ��Ʊ���\n");
					return DF_TAX_ERR_CODE_INV_DATA_DISACCORD;
				}

				stp_Spxx = stp_Spxx->stp_next;
			}
		}
	}



	result = LoadCntaxDevInfo(hDev, hDev->hUSB, true);
	if (result < 0)
	{
		return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
	}

	//if (hDev->bTrainFPDiskType != 0)
	//{
	//	_WriteLog(LL_INFO, "�������޷����������Ϣ����\n");
	//	sprintf((char *)askfor_tzdbh_result->errinfo, "�������޷����������Ϣ����");
	//	result = DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
	//	goto FreeAndExit;
	//}

	//�޸�Ӫҵ˰��־
	//strcpy(fpxx->yysbz, "00000000000000000000");
	//if (fpxx->zyfpLx == ZYFP_CEZS)
	//	fpxx->yysbz[8] = '2';

	char errinfo[1024] = { 0 };
	if ((result = UploadTZDBH(fpxx, szTZDBH)) < 0)
	{
		strcpy(errinfo, hDev->hUSB->errinfo);//�������洢��errinfo
		_WriteLog(LL_FATAL, "[-] UploadTZDBH failed!֪ͨ���������ʧ��,result =%d\r\n", result);
		if ((strcmp(errinfo, "������Ϣ����(˰��)����ԭ��Ʊ�ɿ��ߺ�����Ϣ��Ľ��(˰��)") == 0) ||
			(strcmp(errinfo, "���ַ�Ʊ��Ϣ���ѳɹ��ϴ�,�����ظ��ϴ�") == 0) ||
			(strcmp(errinfo, "ҵ��������������ʧ��,nChildRet = -7") == 0))
		{
			memset(szTZDBH, 0, sizeof(szTZDBH));
			result = RedInvOnlineCheck_TZD(fpxx, &askfor_tzdbh_result->red_inv);
			if (result == 0)
			{
				_WriteLog(LL_FATAL, "�ֶ˲�ѯ������Ʊ��ֱ�ӷ��غ�����Ϣ����\r\n");
				memcpy((char *)askfor_tzdbh_result->tzdbh, fpxx->redNum, strlen(fpxx->redNum));
				memcpy((char *)askfor_tzdbh_result->sqlsh, fpxx->redNum_serial, strlen(fpxx->redNum_serial));
				nRet = 0;
				goto FreeAndExit;
			}
		}
		sprintf((char *)askfor_tzdbh_result->errinfo, "֪ͨ���������ʧ��,���%d,ԭ��%s", result, errinfo);
		nRet = DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
		goto FreeAndExit;
	}
	_WriteLog(LL_INFO, "[+] UploadTZDBH successful,֪ͨ���������ɹ�  red invoice tzdbh:%s", szTZDBH);
	memset(fpxx->redNum, 0, sizeof(fpxx->redNum));
	memcpy(fpxx->redNum, szTZDBH, 24);
	sleep(1);
	for (count = 0; count < 3; count++)  // 3���㹻?,��ѯ��ʱ��ԼΪ18��
	{
		sleep(count * 5);  //����Ҫ���·�������������Ǹձ����ϴ���Ͳ�ѯ���϶�����1=������
		memset(askfor_tzdbh_result->errinfo, 0, sizeof(askfor_tzdbh_result->errinfo));
		if (RedInvOnlineCheck_ZY(fpxx, &askfor_tzdbh_result->red_inv) < 0)
		{
			sprintf((char *)askfor_tzdbh_result->errinfo, "������Ϣ�����������ϴ����,������Ϣ���ţ�%s,��ѯ���״̬δ�ɹ�,���ʵ!", szTZDBH);
			_WriteLog(LL_FATAL, "%s\r\n", askfor_tzdbh_result->errinfo);
			nRet = -4;
			continue;
		}
		nRet = 0;
		memcpy((char *)askfor_tzdbh_result->tzdbh, fpxx->redNum, strlen(fpxx->redNum));
		memcpy((char *)askfor_tzdbh_result->sqlsh, fpxx->redNum_serial, strlen(fpxx->redNum_serial));
		_WriteLog(LL_INFO, "[+] RedInvOnlineCheck_ZY successful ������Ϣ�������ͨ��");
		break;
	}
FreeAndExit:
	if (fpxx)
		FreeFpxx(fpxx);
	return nRet;
}

int cntax_download_hzxxb_from_rednum(HDEV hDev, char *redNum, uint8 **inv_json, char *errinfo)
{
	int result;
	ClearLastError(hDev->hUSB);
	if (hDev->bTrainFPDiskType != 0)
	{
		sprintf(errinfo, "�����̲�֧�ֺ�����Ϣ���ѯ����");//�ô������������޸�,ty_usb_m����ʹ��
		_WriteLog(LL_WARN, "%s", errinfo);
		return -1;
	}
	if (strlen(redNum) != 16)
	{
		sprintf(errinfo, "����ĺ�����Ϣ����������");
		_WriteLog(LL_WARN, "%s", errinfo);
		return -2;
	}
	result = LoadCntaxDevInfo(hDev, hDev->hUSB, true);
	if (result < 0)
	{
		return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
	}
	result = GetRedInvFromRedNum(hDev, redNum, inv_json);
	if (result < 0)
	{
		strcpy(errinfo, hDev->hUSB->szLastErrorDescription);
		_WriteLog(LL_WARN, "%s", errinfo);
	}
	return result;
}

int cntax_download_hzxxb_from_date_range(HDEV hDev, char *date_range, uint8 **tzdbh_data, char *errinfo)
{
	int result;
	int nBufSize = DEF_MAX_FPBUF_LEN;
	char *szBuf = calloc(1, nBufSize);
	ClearLastError(hDev->hUSB);
	if (hDev->bTrainFPDiskType != 0)
	{
		free(szBuf);
		sprintf(errinfo, "�����̲�֧�ֺ�����Ϣ���ѯ����");//�ô������������޸�,ty_usb_m����ʹ��
		_WriteLog(LL_WARN, "%s", errinfo);
		return -1;
	}
	if (strlen(date_range) != 17)
	{
		free(szBuf);
		sprintf(errinfo, "���ڷ�Χ���ݸ�ʽ����");
		return -2;
	}
	result = LoadCntaxDevInfo(hDev, hDev->hUSB, true);
	if (result < 0)
	{
		free(szBuf);
		return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
	}
	if ((result = DownloadTZDBH(hDev, date_range, "", "", szBuf, nBufSize)) < 0) {
		strcpy(errinfo, hDev->hUSB->szLastErrorDescription);
		// if (DownloadTZDBH(hDev, "", "", "3201252012000783", szBuf, nBufSize) < 0) {
		//sprintf(errinfo, "%sʱ��η�Ʊ����ʧ��,ԭ��:%s", date_range, errinfo);
		//EncordingConvert("utf-8", "gbk", szBuf, strlen(szBuf), (char *)errinfo, 1024);
		free(szBuf);
		return result;
	}
#ifdef NEED_FP_DATA
	char name[200] = { 0 };
	memset(name, 0, sizeof(name));
	sprintf(name, "/mnt/masstore/get_redxml.txt");
	write_file(name, (char *)szBuf, strlen(szBuf));

#endif
	uint8 * tzdbh_data_s = calloc(1, DEF_MAX_FPBUF_LEN);
	result = get_fpxx_from_hzxxb_data_cntax(hDev, szBuf, tzdbh_data_s, DEF_MAX_FPBUF_LEN, errinfo);
	if (result == 0)
	{
		sprintf(errinfo, "û�����������Ĵ���������");
		free(szBuf);
		free(tzdbh_data_s);
		return -1;
	}
	//printf("szBuf :%s\n", szBuf);
	*tzdbh_data = tzdbh_data_s;
	free(szBuf);
	return result;
}


int cntax_redinv_tzdbh_cancel(HDEV hDev, char *redNum, char *errinfo)
{
	int nChildRet = -1;
	int nBufSize = DEF_MAX_FPBUF_LEN;
	char *szBuf = calloc(1, nBufSize);
	ClearLastError(hDev->hUSB);
	if (hDev->bTrainFPDiskType != 0)
	{
		free(szBuf);
		sprintf(errinfo, "�����̲�֧�ֺ�����Ϣ��������");
		_WriteLog(LL_WARN, "%s", errinfo);
		return -1;
	}
	if (strlen(redNum) != 16)
	{
		free(szBuf);
		sprintf(errinfo, "����ĺ�����Ϣ����������");
		_WriteLog(LL_WARN, "%s", errinfo);
		return -2;
	}
	nChildRet = LoadCntaxDevInfo(hDev, hDev->hUSB, true);
	if (nChildRet < 0)
	{
		free(szBuf);
		return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
	}

	nChildRet = RedinvTZDCancel(hDev, redNum, szBuf, nBufSize);
	if (nChildRet < 0)
	{
		strcpy(errinfo, hDev->hUSB->errinfo);
		if (strlen(errinfo) == 0)
		{
			sprintf(errinfo, "����������Ϣ��ֶ˽���ʧ��,������");
		}
	}
	_WriteLog(LL_INFO, "RedinvTZDCancel:%s TZDBH:%s retmsg:%s", nChildRet == 0 ? "OK" : "Fail",
		redNum, errinfo);
	if (strcmp("��Ϣ��״̬�Ѿ��ǳ���״̬���������ظ�����", errinfo) == 0)
	{
		free(szBuf);
		_WriteLog(LL_INFO, "�ظ�����Ĭ����Ϊ����");
		return 0;
	}
	free(szBuf);
	return nChildRet;
}

int cntax_query_net_invoice_coil(HDEV hDev, char *date_range, char *inv_data, char *errinfo)
{
	int result = -1;
	char *p;
	int index = 0;
	//int left_num = -1;
	ClearLastError(hDev->hUSB);
	if (hDev->bTrainFPDiskType)
	{
		sprintf(errinfo, "�����̲�֧��ƱԴ��ѯ���ع���");//�ô������������޸�,ty_usb_m����ʹ��
		_WriteLog(LL_WARN, "%s", errinfo);
		return -1;
	}

	result = LoadCntaxDevInfo(hDev, hDev->hUSB, true);
	if (result < 0)
	{
		return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
	}
	char coil_data[4096] = { 0 };
	char tmp_data[200] = { 0 };

	struct HTTP hi;
	if ((result = CreateServerTalk(&hi, hDev)) < 0)
	{
		if (hDev->bBreakAllIO) {
			SetLastError(hDev->hUSB, ERR_IO_BREAK, "���������������б������ӿ��ж�");
			hDev->bBreakAllIO = 0;  //�������
			HTTPClose(&hi);
			return DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
		}
		HTTPClose(&hi);
		if ((result = CreateServerTalk(&hi, hDev)) < 0){
			_WriteLog(LL_FATAL, "˰��ukeyƱԴ�������ӷ�����ʧ��, err:%d", hDev->hUSB->szLastErrorDescription);
			HTTPClose(&hi);
			return DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
		}
		_WriteLog(LL_INFO, "��һ������ʧ�����Գɹ�");
	}
	//sprintf(coil_data,"2|032001900204|51496536|5\r\n0|032001900205|51496537|10\r\n51|032001900206|51496538|15\r\n");
	result = CntaxNetInvoiceQueryReadyEasy(&hi,hDev, coil_data);
	if (result < 0)
	{
		strcpy(errinfo, hDev->hUSB->szLastErrorDescription);
		HTTPClose(&hi);
		return DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
	}
	_WriteLog(LL_FATAL, "nisec_query_net_invoice_coil : %s", coil_data);
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

		memcpy(tmp_data, coil_data + index, p - (coil_data + index));
		char dst[5][768];
		memset(dst, 0, sizeof(dst));
		int nSepCount = 0;
		if ((nSepCount = GetSplitStringSimple(tmp_data, "|", dst, 5)) != 5) {
			_WriteLog(LL_FATAL, "DownloadSpecifyInvoice, parament's count incorrect,nSepCount = %d", nSepCount);
			HTTPClose(&hi);
			return -1;
		}



		sprintf(inv_data + strlen(inv_data), "%s,%s,%s,%s;", dst[0], dst[1], dst[2], dst[3]);
		cJSON *data_layer;
		cJSON_AddItemToObject(data_array, "dira", data_layer = cJSON_CreateObject());

		cJSON_AddStringToObject(data_layer, "fplxdm", dst[0]);
		cJSON_AddStringToObject(data_layer, "fpdm", dst[1]);
		cJSON_AddStringToObject(data_layer, "fpqshm", dst[2]);
		cJSON_AddStringToObject(data_layer, "fpzs", dst[3]);

		index = p - coil_data + 2;
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

int cntax_query_net_invoice_coil_download_unlock(HDEV hDev, char *date_range, uint8 fllxdm, char *fpdm, char *fpqshm, int fpzs, char *errinfo)
{
	char *p;
	int result;
	ClearLastError(hDev->hUSB);
	if (hDev->bTrainFPDiskType)
	{
		sprintf(errinfo, "�����̲�֧��ƱԴ��ѯ���ع���");//�ô������������޸�,ty_usb_m����ʹ��
		_WriteLog(LL_WARN, "%s", errinfo);
		return -1;
	}
	result = LoadCntaxDevInfo(hDev, hDev->hUSB, true);
	if (result < 0)
	{
		return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
	}
	char dowm_data[4096] = { 0 };
	char coil_data[1024] = { 0 };
	//uint8 aisino_fplx;
	if (!CheckDeviceSupportInvType(hDev->abSupportInvtype, fllxdm)) {
		sprintf(errinfo, "˰�̲�֧�ִ˷�Ʊ���ͣ��������˰����Ϣ���");
		return DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
	}

	sprintf(dowm_data, "%03d|%s|%s|%d", fllxdm, fpdm, fpqshm, fpzs);

	_WriteLog(LL_FATAL, "���жϺ����� Ҫ���صķ�Ʊ����Ϣ��%s \n", dowm_data);

	struct HTTP hi;
	if ((result = CreateServerTalk(&hi, hDev)) < 0)
	{
		if (hDev->bBreakAllIO) {
			SetLastError(hDev->hUSB, ERR_IO_BREAK, "���������������б������ӿ��ж�");
			hDev->bBreakAllIO = 0;  //�������
			HTTPClose(&hi);
			return DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
		}
		HTTPClose(&hi);
		if ((result = CreateServerTalk(&hi, hDev)) < 0){
			_WriteLog(LL_FATAL, "˰��ukeyƱԴ�������ӷ�����ʧ��, err:%d", hDev->hUSB->szLastErrorDescription);
			HTTPClose(&hi);
			return DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
		}
		_WriteLog(LL_INFO, "��һ������ʧ�����Գɹ�");
	}

	result = CntaxNetInvoiceQueryReadyEasy(&hi,hDev, coil_data);
	if (result < 0)
	{
		strcpy(errinfo, hDev->hUSB->szLastErrorDescription);
		//sprintf(errinfo, "����������Ʊ��ѯʧ��");
		if (strlen(errinfo) == 0)
		{
			sprintf(errinfo, "����������Ʊ��ѯ��������Ϣʧ��");
		}
		HTTPClose(&hi);
		return DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
	}

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
		char dst[5][768];
		memset(dst, 0, sizeof(dst));
		int nSepCount = 0;
		if ((nSepCount = GetSplitStringSimple(tmp_data, "|", dst, 5)) != 5) {
			_WriteLog(LL_FATAL, "DownloadSpecifyInvoice, parament's count incorrect");
			break;
		}
		uint8 net_fpzl = 0;
		char net_fpdm[20] = { 0 };
		char net_fphm[20] = { 0 };
		int net_fpzs = 0;
		net_fpzl = atoi(dst[0]);
		strcpy(net_fpdm, dst[1]);
		strcpy(net_fphm, dst[2]);
		net_fpzs = atoi(dst[3]);

		if ((net_fpzl == fllxdm) && (strcmp(net_fpdm, fpdm) == 0) && (strcmp(net_fphm, fpqshm) == 0) && (net_fpzs >= fpzs) && (fpzs >= 1))
		{
			cmp_ok = 0;
			break;
		}


		index = p - coil_data + 2;
	}

	if (cmp_ok != 0)
	{
		_WriteLog(LL_INFO, "coil_data = %s ,dowm_data = %s", coil_data, dowm_data);
		sprintf(errinfo, "Ҫ���صķ�Ʊ���ѯ������Ϣ��һ��");
		HTTPClose(&hi);
		return DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
	}


	result = CntaxNetInvoiceDownloadWriteEasy(&hi,hDev, dowm_data);
	if (result < 0)
	{
		strcpy(errinfo, hDev->hUSB->szLastErrorDescription);
		if (strlen(errinfo) == 0)
		{
			sprintf(errinfo, "����������Ʊ���ذ�װ��Ʊʧ��");
		}
		_WriteLog(LL_INFO, "����ʧ��,ԭ�� %s", errinfo);
		HTTPClose(&hi);
		return DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
	}
	HTTPClose(&hi);
	return result;
}
int cntax_client_hello(HDEV hDev, char *client_hello, char *errinfo)
{
	int result;
	ClearLastError(hDev->hUSB);
	if (hDev->bTrainFPDiskType)
	{
		sprintf(errinfo, "�����̲�֧�ַ���������Hello");
		return -1;
	}
	result = LoadCntaxDevInfo(hDev, hDev->hUSB, true);
	if (result < 0)
	{
		return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
	}
	if(hDev->postMessage_flag == 1)//����˰��ֵ�¼
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
	return 0;
}

int cntax_client_auth(HDEV hDev, char *server_hello, char *client_auth, char *errinfo)
{
	int result;
	int auth_len = 5120;
	ClearLastError(hDev->hUSB);
	if (hDev->bTrainFPDiskType)
	{
		sprintf(errinfo, "�����̲�֧�ַ�������֤Auth");
		return -1;
	}
	result = LoadCntaxDevInfo(hDev, hDev->hUSB, true);
	if (result < 0)
	{
		return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
	}
	if (BuildClientAuthCode((uint8 *)server_hello, strlen(server_hello), (uint8 *)client_auth, &auth_len, hDev) < 0) {
		return ERR_TA_AUTHCODE;
	}
	return 0;
}

int cntax_get_cert(HDEV hDev, char *cert_data, char *errinfo)
{
	X509 *x = NULL;
	int nIndex = 2;
	int i;
	void *pCerts = calloc(1, 20480);
	if (CntaxEnumCertsMem(hDev->hUSB, pCerts) < 2)
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

		char cert_tmp[4096] = { 0 };
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
			_WriteLog(LL_INFO, "�ҵ�������֤��");
			strcpy(cert_data, cert_tmp);
			_WriteLog(LL_INFO, "i = %d,cert_data = %s", i, cert_data);
			break;
		}
		nIndex += 2 + *pCertLen;
	}
	free(pCerts);
	return 0;
}

int cntax_update_summary_data(HDEV hDev, struct _plate_infos *plate_infos)
{
	int result;
	char errinfo[1024] = { 0 };
	char month[20] = { 0 };
	char *statistics = NULL;
	struct _inv_sum_data now_month_sum;
	memcpy(month, hDev->szDeviceTime, 6);
	result = cntax_query_invoice_month_all_data(hDev, (char *)month, &statistics, errinfo);
	if (result < 0)
	{
		_WriteLog(LL_INFO, "��ȡ��Ʊ�������ݹ����г����쳣\n");
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
			//out("%03dƱ�֣�ʵ�����۽��%4.2Lf,ʵ������˰��%4.2Lf,�ڳ�������Ϊ%d\n", plate_infos->invoice_type_infos[mem_i].fplxdm, now_month_sum.type_sum[sum_cout].sjxsje, now_month_sum.type_sum[sum_cout].sjxsse, now_month_sum.type_sum[sum_cout].qckcfs);
			memset(&plate_infos->invoice_type_infos[mem_i].monitor_info.type_sum, 0, sizeof(struct _inv_type_sum_data));
			memcpy(&plate_infos->invoice_type_infos[mem_i].monitor_info.type_sum, &now_month_sum.type_sum[sum_cout], sizeof(struct _inv_type_sum_data));
		}
	}
	return 0;
}