/*****************************************************************************
File name:   cntax_uploadinvoice.c
Description: cntax设备发票上传代码逻辑
Author:      Zako
Version:     1.0
Date:        2021.04.01
History:
20210401     最初代码实现
*****************************************************************************/
#include "cntax_uploadinvoice.h"

int CntaxPreInvoiceXML(HFPXX fpxx, char *pOutXMLBuf)
{
    int nRawFpLen = *(int *)fpxx->pRawFPBin;
    if (nRawFpLen < 0 || nRawFpLen > DEF_MAX_FPBUF_LEN)
        return -1;
    // mxysbz = "0",SelectInvoType之前设置的静态值，暂时不能理解其作用
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
        if (bStep == 1) {  // 0x1c 第一次上传确认密文
            memcpy(cb, "\xfe\x22\x01", 3);
        } else if (bStep == 2) {  //离线发票反写密文（通用反写密文）
            memcpy(cb, "\xfe\x22\x00", 3);
            //此处为发票上传后最后一步反写，此处break，就不再写入最后反写IO
            // break;
        }
        int nChildRet = CntaxLogicIO(hUsb, cb, 3, sizeof(cb), abTransferData, nTransferDataLen);
        if (nChildRet < 0) {
            logout(INFO, "TAXLIB", "发票上传",
                   "盘号：%s,发票上传反写第%d步失败,nChildRet = %d,errinfo = %s\r\n",
                   hDev->szDeviceID, bStep, nChildRet, hUsb->szLastErrorDescription);
            break;
        }
        logout(INFO, "TAXLIB", "发票上传", "盘号：%s,发票上传反写第%d步成功\r\n", hDev->szDeviceID,
               bStep);
        nRet = 0;
        break;
    }
    CntaxEntry(hUsb, NOP_CLOSE);
    return nRet;
}

//尚未接入Net_IO统一接口，因此暂不支持LastError
//返回值将在fpxx->szSLXLH中写入32位受理序列号，后期可以直接通过该序列号查询接口
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
        //如果正确，受理序列号拷贝出来
        strcpy(pRepBuf, szInvXML);
        nRet = 0;
    } while (false);
    free(szInvXML);
    return nRet;
}

//返回值<0 错误结果在fpxx->hDev->hUSBDevice->szLastErrorDescription中
int CntaxDownloadFPBySingle(HHTTP hi, HFPXX fpxx, char *pRepBuf, int nRepBufLen)
{
    char szInvXML[512] = "", szBuf[512000] = "";
    int nOpType = TACMD_CNTAX_QUERY_UPFP, i = 0, nRet = -1;
    sprintf(szInvXML, "<ywxx><fplxdm>%03d</fplxdm><sllsh>%s</sllsh></ywxx>", fpxx->fplx, pRepBuf);
    pRepBuf[0] = '\0';
    //第一次提交，第二次查询结果;如果之前没有提交需要2次，如果之前已经上传提交过了，执行一次就可以获取结果
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

//有第一次确认密文的才写入（当反写密文写入税盘后确认密文则消失，不会再获取到）
int WriteNewUploadInvoiceUploadMw(HFPXX fpxx)
{
    //这里有3种情况
    // 1)正常未上传发票，仅通过离线票接口查询出来，有abUploadSureMW10x1c
    // 2)尚未上传就作废的，由离线票查询接口查询出来的，有abUploadSureMW10x1c
    // 3)上传后再作废，由modify接口（即官方已上传发票接口）查询出来的，无abUploadSureMW10x1c信息
    // 4)发票全部同步完成后，由指定发票信息接口查询和月度恢复等接口查询出来的，皆无abUploadSureMW10x1c信息?
    //
    //实际离线票接口查出来的一般不会无abUploadSureMW10x1c信息，暂时都放过
    //如果有上传确认密文（第一次密文），但是上传后并写入上传反写密文（第二次密文）
    //但是确认密文未写入税盘，会造成税盘错误-最后一张离线票实际上传但是税盘认为未上传，需要去税局处理
    char szNoUploadSureMWFlag[] = "NoData!";  //自定义数据，无实际意义
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
        // if (fpxx->bIsUpload && fpxx->zfbz && !bHasUploadSureMw) {//老逻辑
        if (fpxx->zfbz && !bHasUploadSureMw) {  //此处以蒙柏方代码为准，不再修改
            //已上传的发票再作废，存在作废标志、存在未上传标志（但是不能通过未上传接口查询到）
            //且该发票必然无上传确认密文（已在第一次上传的时候反写过了），无需反写第一步上传密文
            nRet = RET_SUCCESS;
            break;
        }
        if (!bHasUploadSureMw) {
            //剩余无密文的都报错
            SetLastError(fpxx->hDev->hUSB, ERR_LOGIC_UPLOADINV_FAIL,
                         "非作废票未读到确认密码");  //此错误值上层判断不可任意修改
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
                         "发票上传过程中被其他接口中断[Place 1]");  //此错误值上层判断不可任意修改
            logout(INFO, "TAXLIB", "发票上传", "盘号：%s,正在上传发票,检测到外部要求中断,%s\r\n",
                   hDev->szDeviceID, hDev->hUSB->errinfo);
            hDev->bBreakAllIO = 0;  //清除缓存
            // report_event(fpxx->hDev->szDeviceID, "离线发票上传失败", errinfo, -2);
            nRet = ERR_IO_BREAK;
            free(response);
            return nRet;
        }
        if ((nChildRet = CreateServerTalk(&hi, hDev)) < 0) {

            if (hDev->bBreakAllIO) {
                SetLastError(
                    hDev->hUSB, ERR_IO_BREAK,
                    "发票上传过程中被其他接口中断[Place 2]");  //此错误值上层判断不可任意修改
                logout(INFO, "TAXLIB", "发票上传",
                       "盘号：%s,正在上传发票,检测到外部要求中断,%s\r\n", hDev->szDeviceID,
                       hDev->hUSB->errinfo);
                hDev->bBreakAllIO = 0;  //清除缓存
                // report_event(hDev->szDeviceID, "离线发票上传失败", errinfo, -2);
                nRet = ERR_IO_BREAK;
                break;
            }

            if ((strstr(hDev->hUSB->errinfo, "ERROR@98@证书已挂失") != NULL) ||
                (strstr(hDev->hUSB->errinfo, "ERROR@103@证书状态未知") != NULL) ||
                (strstr(hDev->hUSB->errinfo, "ERROR@97@证书已作废") != NULL)) {
                SetLastError(hDev->hUSB, ERR_TA_CERT_ERROR,
                             "证书已挂失或已作废或状态未知");  //此错误值上层判断不可任意修改
                logout(INFO, "TAXLIB", "发票上传", "盘号：%s,发票上传与局端连接异常,提示：%s\r\n",
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
                         "发票上传过程中被其他接口中断[Place 3]");  //此错误值上层判断不可任意修改
            logout(INFO, "TAXLIB", "发票上传", "盘号：%s,正在上传发票,检测到外部要求中断,%s\r\n",
                   hDev->szDeviceID, hDev->hUSB->errinfo);
            hDev->bBreakAllIO = 0;  //清除缓存
            // report_event(hDev->szDeviceID, "离线发票上传失败", errinfo, -2);
            nRet = ERR_IO_BREAK;
            break;
        }

        //_WriteLog(LL_INFO, "Prepare to upload invoice");
        if ((nChildRet = CntaxUploadFPBySingle(&hi, fpxx, response, nRepBufSize)) < 0) {
            _WriteLog(LL_WARN, "UploadInvoice failed, nChildRet:%d", nChildRet);
            nRet = ERR_TA_UPLOAD_INVOICE;
            break;
        }
        //如果上传没问题，则判断是否新上传发票。新上传发票需要写入上传确认密文
		if ((nChildRet = WriteNewUploadInvoiceUploadMw(fpxx)) < 0) {
            _WriteLog(LL_WARN, "Upload invoice, updateflag failed, step1");
			logout(INFO, "TAXLIB", "发票上传", "盘号：%s,WriteNewUploadInvoiceUploadMw nChildRet=%d\r\n", hDev->szDeviceID, nChildRet);
			nRet = ERR_TA_UPLOAD_INVOICE_UPDATE;
            break;
        }
        if (hDev->bBreakAllIO) {
            SetLastError(hDev->hUSB, ERR_IO_BREAK,
                         "发票上传过程中被其他接口中断[Place 4]");  //此错误值上层判断不可任意修改
            logout(INFO, "TAXLIB", "发票上传", "盘号：%s,正在上传发票,检测到外部要求中断,%s\r\n",
                   hDev->szDeviceID, hDev->hUSB->errinfo);
            hDev->bBreakAllIO = 0;  //清除缓存
            // report_event(hDev->szDeviceID, "离线发票上传失败", errinfo, -2);
            nRet = ERR_IO_BREAK;
            break;
        }
        sleep(3);  //不等待会出现发票尚未解析的错误
        _WriteLog(LL_DEBUG, "Upload invoice OK, wait for download...");
        if (hDev->bBreakAllIO) {
            SetLastError(hDev->hUSB, ERR_IO_BREAK,
                         "发票上传过程中被其他接口中断[Place 5]");  //此错误值上层判断不可任意修改
            logout(INFO, "TAXLIB", "发票上传", "盘号：%s,正在上传发票,检测到外部要求中断,%s\r\n",
                   hDev->szDeviceID, hDev->hUSB->errinfo);
            hDev->bBreakAllIO = 0;  //清除缓存
            // report_event(hDev->szDeviceID, "离线发票上传失败", hDev->hUSB->errinfo,
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
        //如果签名为空，需要补签名，完了才能上传
        if (!strlen(fpxx->sign)) {
            if (CntaxFpxxAppendSignIO(fpxx, false)) {
                _WriteLog(LL_FATAL, "Update invoice's sign failed");
                break;
            }
            _WriteLog(LL_INFO, "CntaxFpxxAppendSignIO success");
            //重新读取签名后的发票
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
        logout(INFO, "TAXLIB", "发票上传",
               "盘号：%s,CntaxUploadInvoiceCallBack 上传发票 fpdm = %s,fphm = %s 成功\r\n",
               fpxx->hDev->szDeviceID, fpxx->fpdm, fpxx->fphm);
    } else {
        logout(INFO, "TAXLIB", "发票上传",
               "盘号：%s,CntaxUploadInvoiceCallBack 上传发票 fpdm = %s,fphm = %s 失败\r\n",
               fpxx->hDev->szDeviceID, fpxx->fpdm, fpxx->fphm);
    }
    _WriteLog(LL_DEBUG, "Cntax upload invoice %s, fplx:%d fpdm:%s fpgm:%s",
              bOK ? "successful" : "failed", fpxx->fplx, fpxx->fpdm, fpxx->fphm);
    return bOK;  //已上报一张
}

////查询:1)已上传发票验签失败、2)解析有误、3)开具后作废的发票列表
//获取未上传发票信息及已上传未确认发票张数(包括正常发票上传尚未完成第二步的及已上传后再作废的)
//返回值<0 IO错误；==0无修改发票;>0已处理的修改发票张数
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
            //传出当前操作信息
            strcpy(szOutFPDM, fpxx->fpdm);
            strcpy(szOutFPHM, fpxx->fphm);
            //执行网络IO
            _WriteLog(LL_INFO, "Try to upload waste fp,fplx:%d fpdm:%s fphm:%s", fpxx->fplx,
                      fpxx->fpdm, fpxx->fphm);
            logout(INFO, "TAXLIB", "发票上传",
                   "盘号：%s,CntaxQueryModifyInvoice fpdm = %s,fphm = %s\r\n", hDev->szDeviceID,
                   fpxx->fpdm, fpxx->fphm);
            nRet = CntaxUploadInvoiceCallBack(fpxx);
            //不进行多次循环，一张张处理
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

//简单API，就不重新开关卡了，需要已开卡状态
int CntaxQueryOfflineInvoiceNum(HDEV hDev)
{
    //驱动版本号为"00020001220225"的盘暂且不知道返回值，遇到如果返回值不是4则报错
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

//查询完成后自动回调上传，返回值为处理的发票张数，一张张处理
int CntaxQueryOfflineInvoice(uint8 bInvType, HDEV hDev)
{
    int nRet = ERR_GENERIC, nChildRet = 0;
    //一次处理2张可以改成2，2张的需求不是很强烈，暂不加入测试
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
        // FPCX_COMMON_OFFLINE替换成FPCX_COMMON_LASTINV亦可
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
        //返回的发票buf只有一张的内存，因此也只处理一次，不会循环处理第二张
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
        // fe7f01 |fe0101(读取管理信息中也包含该信息)
        // 00010000191023
        memcpy(cb, "\xfe\x7f\x01", 3);
        if ((nChildRet = CntaxLogicIO(hDev->hUSB, cb, 3, sizeof(cb), NULL, 0)) < 7)
            break;
        Byte2Str(szTaVersion, cb, nChildRet);
        // API接口UK_ReadBSPVersion 请求fe7f0101，返回53010003191105
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

//检测设备COS，固件系统是否需要升级
int CheckFirewareUpdate(HHTTP hi)
{
    char szBuf[8192] = "";
    int nRet = ERR_PROGRAME_API;
    do {
        if ((nRet = BuildFirewareUpdateRequest(hi->hDev, szBuf)) < 0)
            break;
        if ((nRet = TaNetIo(hi, TACMD_CNTAX_RAWXML, szBuf, sizeof(szBuf))) < 0)
            break;
        // W20100时无新版本
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
    // bmtsqybs为何是dxqy(电信企业)，着实没想明白，但是逐层内存分析后确实是
    NewByteStringToXmlNode(xmlNode, "bmtsqybs", (uint8 *)&hDev->stDevExtend.dxqy);
    //小规模自开专票
    NewByteStringToXmlNode(xmlNode, "xgmzkzp", (uint8 *)&hDev->stDevExtend.xgmkjzpbs);
    //二手车企业标识
    NewByteStringToXmlNode(xmlNode, "escqybs", &hDev->stDevExtend.esjdcbs);
    //成品油企业类型
    NewByteStringToXmlNode(xmlNode, "cpyqylx", (uint8 *)&hDev->stDevExtend.cpybs);
    //成品油企业有效期
    mxmlNewText(mxmlNewElement(xmlNode, "cpyqyyxq"), 0, hDev->stDevExtend.cpybsyxq);
    //成品油白名单
    NewByteStringToXmlNode(xmlNode, "cpybmd", (uint8 *)&hDev->stDevExtend.cpybmdbs);
    //成品油白名单有效期
    mxmlNewText(mxmlNewElement(xmlNode, "cpybmdyxq"), 0, hDev->stDevExtend.cpybmdbsyxq);
    //卷烟企业标志
    NewByteStringToXmlNode(xmlNode, "jyqybz", (uint8 *)&hDev->stDevExtend.jyqy);
    //卷烟企业有效期
    mxmlNewText(mxmlNewElement(xmlNode, "jyqyyxq"), 0, hDev->stDevExtend.jyqyyxq);
    //机动车企业标志
    NewByteStringToXmlNode(xmlNode, "jdcqybz", (uint8 *)&hDev->stDevExtend.jdcqy);
    //机动车企业有效期
    mxmlNewText(mxmlNewElement(xmlNode, "jdcqyyxq"), 0, hDev->stDevExtend.jdcqyyxq);
    //机动车白名单标志
    NewByteStringToXmlNode(xmlNode, "jdcbmdbz", (uint8 *)&hDev->stDevExtend.jdcbmdbs);
    //机动车白名单有效期
    mxmlNewText(mxmlNewElement(xmlNode, "jdcbmdyxq"), 0, hDev->stDevExtend.jdcbmdyxq);
    //委托代开标志
    NewByteStringToXmlNode(xmlNode, "wtdkbz", (uint8 *)&hDev->stDevExtend.wtdkqy);
    //委托代开有效期
    mxmlNewText(mxmlNewElement(xmlNode, "wtdkyxq"), 0, hDev->stDevExtend.wtdkqyyxq);
    //登记注册类型。截止20220813，开票软件此处有个错误
    //--错误的标签导致XML标签不闭合，为:<djzclx>000</djzcclx>，目前我们先按照正确的来
    mxmlNewText(mxmlNewElement(xmlNode, "djzclx"), 0, hDev->stDevExtend.djzclx);
    //小规模专票ms标志
    NewByteStringToXmlNode(xmlNode, "xgmzpmsbz", (uint8 *)&hDev->stDevExtend.xgmjdczyms);
    //小规模专票ms标志有效期
    mxmlNewText(mxmlNewElement(xmlNode, "xgmzpmsbzyxq"), 0, hDev->stDevExtend.xgmjdczymsyxqzz);
    //纳税人名称
    mxmlNewText(mxmlNewElement(xmlNode, "nsrmc"), 0, hDev->szCompanyName);
    //特种企业(稀土)
    NewByteStringToXmlNode(xmlNode, "tzqy", (uint8 *)&hDev->stDevExtend.xtqy);
    //农产品企业
    NewByteStringToXmlNode(xmlNode, "ncpqy", (uint8 *)&hDev->stDevExtend.ncpqy);
    //特定企业标志
    NewByteStringToXmlNode(xmlNode, "tdqybz", (uint8 *)&hDev->stDevExtend.tdqy);
    //开票标志01禁止开票
    NewByteStringToXmlNode(xmlNode, "kpbz", (uint8 *)&hDev->stDevExtend.kpbz);

    //发票类型节点
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
        mxmlElementSetAttr(xmlNodeChild, "xh", szBuf);  //设置属性
        // 发票类型
        sprintf(szBuf, "%03d", invEnv->bInvType);
        mxmlNewText(mxmlNewElement(xmlNodeChild, "fplxdm"), 0, szBuf);
        //离线开票限额，不一定对（尚未验证）
        long double fNum =
            strtold(invEnv->monitor.lxzsljje, NULL) + strtold(invEnv->monitor.lxfsljje, NULL);
        memset(szBuf, 0, sizeof(szBuf));
        sprintf(szBuf, "%4.2Lf", fNum);  //蒙柏结论，感觉此处不是相加得出的，应该是lxzsljje
        mxmlNewText(mxmlNewElement(xmlNodeChild, "lxkpxe"), 0, invEnv->monitor.lxzsljje);
        //离线开票时限
        mxmlNewText(mxmlNewElement(xmlNodeChild, "lxkpsx"), 0, invEnv->monitor.lxkpsc);
        //单张开票限额
        mxmlNewText(mxmlNewElement(xmlNodeChild, "dzkpxe"), 0, invEnv->monitor.dzkpxe);
        //上传截止日期 1号
        mxmlNewText(mxmlNewElement(xmlNodeChild, "scjzrq"), 0, invEnv->monitor.scjzrq);
        //不含税税率，先填固定值
        GetTaxrate(invEnv->szTaxrate, false, szBuf);
        mxmlNewText(mxmlNewElement(xmlNodeChild, "bhssl"), 0, szBuf);
        //含税税率
        GetTaxrate(invEnv->szTaxrate, true, szBuf);
        if (strlen(szBuf) != 0)
            mxmlNewText(mxmlNewElement(xmlNodeChild, "hssl"), 0, szBuf);
        else
            mxmlNewElement(xmlNodeChild, "hssl");
        //平台类型
        memset(szBuf, 0, sizeof(szBuf));
        strncpy(szBuf, invEnv->monitor.ggfwpt, 2);
        if (strlen(szBuf) != 0)
            mxmlNewText(mxmlNewElement(xmlNodeChild, "ptlx"), 0, szBuf);
        else
            mxmlNewElement(xmlNodeChild, "ptlx");
        //平台序号
        memset(szBuf, 0, sizeof(szBuf));
        strcpy(szBuf, invEnv->monitor.ggfwpt + 2);
        if (strlen(szBuf) != 0)
            mxmlNewText(mxmlNewElement(xmlNodeChild, "ptxh"), 0, szBuf);
        else
            mxmlNewElement(xmlNodeChild, "ptxh");
    }
    return nRet;
}

//此处涉及同步的精准性，必须读取实时时钟和随机数，尽量不用HDEV中记录的静态数值
//后期分散因子用的多可以考虑加入hdev
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

//返回信息不为空，需要手动释放返回值
char *CntaxPreCompanyXml(HDEV hDev)
{
    char *pRet = NULL;
    mxml_node_t *root = NULL, *xml_root = NULL, *xmlNode = NULL;
    do {
        if (!(root = mxmlNewXML("1.0")))
            break;
        if (!(xml_root = mxmlNewElement(root, "ywxx")))
            break;
        // 1)企业参数下载
        mxmlNewElement(xml_root, "qycsxz");
        // 2)企业类型信息
        if (CntaxFillXmlCompnanyInfo(hDev, xml_root) < 0)
            break;
        // 3）远程变更查询
        mxmlNewText(mxmlNewElement(mxmlNewElement(xml_root, "ycbgcx"), "bglx"), 0, "01");
        // 4）hq时钟信息
        if (CntaxFillRtcTimeAndRand(hDev, xml_root))
            break;
        // 5）税收分类编码 ssflbm,分类编码太耗流量尽量不更新
        xmlNode = mxmlNewElement(xml_root, "ssflbm");
        mxmlNewText(mxmlNewElement(xmlNode, "version"), 0, "3.0");
        mxmlNewText(mxmlNewElement(xmlNode, "bbh"), 0, DEF_BMBBBH_DEFAULT);
        mxmlNewElement(xmlNode, "ssflbm");
        mxmlNewElement(xmlNode, "ssflmc");
        // 6）调整前税率授权信息 tzqslsq
        mxmlNewElement(xml_root, "tzqslsq");
        pRet = CntaxGetUtf8YwxxStringFromXmlRoot(root);
    } while (false);
    if (root)
        mxmlDelete(root);
    return pRet;
}
