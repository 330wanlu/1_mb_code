/*****************************************************************************
File name:   cntax_netinvoice.c
Description: 用于网络发票的获取\确认\分发，电子公共服务平台等网络设置相关
Author:      Zako
Version:     1.0
Date:        2021.11
History:
20211122     最初代码实现
*****************************************************************************/
#include "cntax_netinvoice.h"

int CntaxAppendNodeGpxx(mxml_node_t *nodeResultChildSingle, char *szOutFormatStr)
{
    //<ywxxs><ywxx><fplxdm>026</fplxdm><fpdm>032002100211</fpdm><qshm>66422957</qshm><zzhm>66422961</zzhm><fs>5</fs><lpzt>2</lpzt><sbbh>537100494968</sbbh></ywxx>
    //<ywxx><fplxdm>004</fplxdm><fpdm>3200211130</fpdm><qshm>50068726</qshm><zzhm>50068730</zzhm><fs>5</fs><lpzt>0</lpzt><sbbh></sbbh></ywxx>
    //<ywxx><fplxdm>007</fplxdm><fpdm>032002100204</fpdm><qshm>10614561</qshm><zzhm>10614565</zzhm><fs>5</fs><lpzt>2</lpzt><sbbh>537100494968</sbbh></ywxx>
    //</ywxxs>
    int count = 0;
    char szFPZL[16], szTypeCode[24], szFPHMHead[24], szFPCount[24], szGpzt[24], szLine[512];
    do {
        if (GetChildNodeValue(nodeResultChildSingle, "fplxdm", szFPZL) < 0)
            break;
        if (GetChildNodeValue(nodeResultChildSingle, "fpdm", szTypeCode) < 0)
            break;
        if (GetChildNodeValue(nodeResultChildSingle, "qshm", szFPHMHead) < 0)
            break;
        // zzhm 终止号码，暂不处理
        if (GetChildNodeValue(nodeResultChildSingle, "fs", szFPCount) < 0)
            break;
        if (GetChildNodeValue(nodeResultChildSingle, "lpzt", szGpzt) < 0)
            break;
        if (!strcmp(szGpzt, "2"))
            continue;
        if (!strcmp(szGpzt, "1")) {
            _WriteLog(LL_WARN, "Found not unlock invoce -- FPZL:%s LBDM:%s QSHM:%s FPFS:%s", szFPZL,
                      szTypeCode, szFPHMHead, szFPCount);
        }
        //这里还有个购票状态gpzt，当发票已下载到设备，但是尚未解锁并反馈税局时，该值为1，否则为0
        _WriteLog(LL_INFO, "NetInvoiceGet, TA invlist -- FPZL:%s LBDM:%s QSHM:%s FPFS:%s GPZT:%s",
                  szFPZL, szTypeCode, szFPHMHead, szFPCount, szGpzt);
        sprintf(szLine, "%s|%s|%s|%s|%s\r\n", szFPZL, szTypeCode, szFPHMHead, szFPCount, szGpzt);
        count++;
        strcat(szOutFormatStr, szLine);
    } while ((nodeResultChildSingle = mxmlGetNextSibling(nodeResultChildSingle)) != NULL);
    return count;
}

int CntaxQueryGpxxResult(HDEV hDev, char *szXML, char *szInputOrOutFormatStr)
{
    int nRet = -1;
    char szBuf[64] = "";
    mxml_node_t *pXMLRoot = NULL, *node1 = NULL;
    if ((pXMLRoot = mxmlLoadString(NULL, szXML, MXML_OPAQUE_CALLBACK)) == NULL) {
        _WriteLog(LL_FATAL, "NetInvoiceGet_QueryReady failed, Msg:%s", szXML);
        return -3;
    }
    do {
        if (!FindXMLNodeByKey(pXMLRoot, pXMLRoot, "returncode", 1, szBuf))
            break;
        //<business id="GGFW_WSLPXXCX"
        // comment="网上领票信息查询"><body><tyxx><returncode>00</returncode><returnmsg>操作成功</returnmsg><nsrsbh>91320118MA20LU6N3P</nsrsbh><sbbh>537100494968</sbbh></tyxx>
        //<ywxxs><ywxx><fplxdm>026</fplxdm><fpdm>032002100211</fpdm><qshm>66422957</qshm><zzhm>66422961</zzhm><fs>5</fs><lpzt>2</lpzt><sbbh>537100494968</sbbh></ywxx>
        //<ywxx><fplxdm>004</fplxdm><fpdm>3200211130</fpdm><qshm>50068726</qshm><zzhm>50068730</zzhm><fs>5</fs><lpzt>0</lpzt><sbbh></sbbh></ywxx>
        //<ywxx><fplxdm>007</fplxdm><fpdm>032002100204</fpdm><qshm>10614561</qshm><zzhm>10614565</zzhm><fs>5</fs><lpzt>2</lpzt><sbbh>537100494968</sbbh></ywxx>
        //</ywxxs></body></business>
        int nRequestStatus = atoi(szBuf);
        if (nRequestStatus != 0) {
            char szMsg[512] = "";
            GetChildNodeValue(pXMLRoot, "returnmsg", szMsg);
            SetLastError(hDev->hUSB, ERR_TA_DATA_ERROR, szMsg);
            _WriteLog(LL_WARN, "Cntax netinvoice failed, %s", hDev->hUSB->szLastErrorDescription);
            if (nRequestStatus == 1)  //尚未测试
                nRet = 0;             // 01时 无票，不能算错误
            break;
        }
        if ((node1 = mxmlFindElement(pXMLRoot, pXMLRoot, "ywxx", NULL, NULL, MXML_DESCEND)) == NULL)
            break;
        // szInputOrOutFormatStr输出有5个，多一个发票状态，0-1-2
        nRet = CntaxAppendNodeGpxx(node1, szInputOrOutFormatStr);
        if (nRet < 0) {
            break;
        }
        _WriteLog(LL_INFO, "Get available invoice count:%d", nRet);
    } while (false);
    if (pXMLRoot)
        mxmlDelete(pXMLRoot);
    return nRet;
}

//先查询再申请写入,最终通知税局结果 bOpType =0 查询获得购票,=1
//申请购票，=2上传购票结果同时税局返回购票结果
int CntaxQueryNetInvoiceReady(uint8 bOpType, HHTTP hi, char *szInputOrOutFormatStr)
{
    HDEV hDev = hi->hDev;
    char szXML[9999] = "", dst[4][768], szZzhm[24] = "";
    memset(dst, 0, sizeof(dst));
    int nRet = -3, nOpType = 0;

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
    if (bOpType == 0) {
        sprintf(szXML, "<ywxxs><cxbz/></ywxxs>");
        nOpType = TACMD_CNTAX_NETINV_QUERY;
    } else if (bOpType == 1) {
        nOpType = TACMD_CNTAX_NETINV_WRITE;
        sprintf(szXML,
                "<ywxxs><cxbz></cxbz><ywxx><fplxdm>%s</fplxdm><fpdm>%s</"
                "fpdm><qshm>%s</qshm><zzhm>%s</zzhm><fs>%s</fs></ywxx></ywxxs>",
                dst[0], dst[1], dst[2], szZzhm, dst[3]);
        hDev->bOperateInvType = atoi(dst[0]);
    } else if (bOpType == 2)  //==2
    {
        nOpType = TACMD_CNTAX_NETINV_CONFIRM;
        sprintf(szXML,
                "<ywxxs><ywxx><fplxdm>%s</fplxdm><fpdm>%s</fpdm><qshm>%s</"
                "qshm><zzhm>%s</zzhm></ywxx></ywxxs>",
                dst[0], dst[1], dst[2], szZzhm);
    } else {
        return -2;
    }
    if (TaNetIo(hi, nOpType, szXML, sizeof szXML) < 0) {
        _WriteLog(LL_FATAL, "TaNetIo failed, nRet:%d", nRet);
        return -1;
    }
    if (bOpType == 0) {
        nRet = CntaxQueryGpxxResult(hDev, szXML, szInputOrOutFormatStr);
    } else if (bOpType == 1) {
        if (strlen(szXML) > 100) {
            strcpy(szInputOrOutFormatStr, szXML);
            nRet = 0;
        }
    } else if (bOpType == 2) {
        _WriteLog(LL_INFO, "Buy invoice all finish, result:[%s]", szXML);
    } else {  //==3
    }
    nRet = 0;
    return nRet;
}

// szOutFormatStr缓冲区必须足够大，输出格式如下
// szFPZL|szTypeCode|szFPHMHead|szFPCount|szGpzt\r\nszFPZL|szTypeCode|szFPHMHead|szFPCount|szGpzt\r\n....
int CntaxNetInvoiceQueryReady(HDEV hDev, char *szOutFormatStrMultiLine)
{
    struct HTTP hi;
    int nRet = -1, nChildRet = -1;
    if ((nChildRet = CreateServerTalk(&hi, hDev)) < 0) {
        _WriteLog(LL_DEBUG, "CntaxNetInvoiceQueryReady, conn failed");
        goto FreeAndExit;
    }
    if ((nChildRet = CntaxQueryNetInvoiceReady(0, &hi, szOutFormatStrMultiLine)) < 0) {
        _WriteLog(LL_FATAL, "CntaxNetInvoiceQueryReady, IO failed");
        goto FreeAndExit;
    }
    if (!strlen(szOutFormatStrMultiLine)) {
        _WriteLog(LL_FATAL, "CntaxNetInvoiceQueryReady, not found avail invoice to download");
        SetLastError(hDev->hUSB, ERR_TA_REPONSE_FAILE, "%s", "没有查询到可以领取的发票");
        goto FreeAndExit;
    }
    nRet = 0;
FreeAndExit:
    HTTPClose(&hi);
    return nRet;
}

//下载完成的FP写入设备,写入完成后还需要解锁
// checkConfig 为fplx|fpdm|qshm|fpfs
int CntaxWriteGpxxmwToDevice(HDEV hDev, char *szGpxxmw, char *szSingleFormatLine)
{
    uint8 cb[512], abWrite[512] = {0};
    char dst[4][768];
    memset(dst, 0, sizeof(dst));
    int nSepCount = 0;
    if ((nSepCount = GetSplitStringSimple(szSingleFormatLine, "|", dst, 4)) != 4)
        return -1;
    int nChildRet = Str2Byte(abWrite, szGpxxmw, strlen(szGpxxmw));
    if (nChildRet <= 0)
        return -2;
    memcpy(cb, "\xfe\x1b", 2);
    if (CntaxLogicIO(hDev->hUSB, cb, 2, sizeof cb, abWrite, nChildRet) < 0)
        return -3;
    //写完再检测一次
    if (CheckInvoiceExistAtDevice(hDev, dst) <= 0)
        return -4;
    return 0;
}

//将设备中已下载，但是尚未解锁的发票进行查询解锁，从税局查询比较精准
bool CntaxUnlockReadyInvoiceInDevice(void *ptrHHTTP)
{
    HHTTP hi = (HHTTP)ptrHHTTP;
    HDEV hDev = hi->hDev;
    char szBuf[8192] = "";
    int nChildRet = 0;

    if ((nChildRet = CntaxQueryNetInvoiceReady(0, hi, szBuf)) < 0) {
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
        if (CntaxQueryNetInvoiceReady(1, hi, szBuf) < 0)
            continue;
        if (!hDev->bOperateInvType) {
            _WriteLog(LL_FATAL, "Write gpxxmw not set bOperateInvType");
            break;
        }
        // open device
        if (CntaxEntry(hDev->hUSB, NOP_OPEN_FULL) < 0) {
            break;
        }
        bDeviceOpen = true;
        CntaxSelectInvoiceType(hDev, hDev->bOperateInvType, SELECT_OPTION_TEMPLATE);
        // check exist
        bool existAtDevice = CheckInvoiceExistAtDevice(hDev, dst) > 0;
        if (!existAtDevice && CntaxWriteGpxxmwToDevice(hDev, szBuf, szSingleFormatLine) < 0) {
            _WriteLog(LL_WARN, "UnlockReadyInvoiceInDevice CntaxWriteGpxxmwToDevice failed");
            ret = false;
            break;
        }
        if ((nChildRet = CntaxQueryNetInvoiceReady(2, hi, szSingleFormatLine)) < 0) {
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
//输入参数szInputFormatSingleLine==0时，同步设备与税局购票信息，将盘中领票已成功已存在但是税局尚未同步删除的票清除
int CntaxNetInvoiceDownloadWrite(HDEV hDev, char *szInputFormatSingleLine)
{
    struct HTTP hi;
    char szOutMW[512] = "";
    int nRet = -1, nChildRet = -1;
    do {
        if ((nChildRet = CreateServerTalk(&hi, hDev)) < 0)
            break;
        if (strlen(szInputFormatSingleLine) == 0) {
            //查询税局尙存在，但是实际盘中发票已写入并且可开票，尝试同步并删除局端信息
            if ((nChildRet = CntaxUnlockReadyInvoiceInDevice(&hi)) < 0) {
                _WriteLog(LL_FATAL, "UnlockReadyInvoiceInDevice failed");
            } else {
                nRet = 0;
            }
            break;
        }
        if (strlen(szInputFormatSingleLine) > sizeof(szOutMW))
            break;
        strcpy(szOutMW, szInputFormatSingleLine);
        if ((nChildRet = CntaxQueryNetInvoiceReady(1, &hi, szOutMW)) < 0) {
            _WriteLog(LL_FATAL, "QueryNetInvoiceReady failed");
            break;
        }
        // open device
        if (CntaxEntry(hDev->hUSB, NOP_OPEN_FULL) < 0)
            break;
        CntaxSelectInvoiceType(hDev, hDev->bOperateInvType, SELECT_OPTION_TEMPLATE);
        nChildRet = CntaxWriteGpxxmwToDevice(hDev, szOutMW, szInputFormatSingleLine);
        CntaxEntry(hDev->hUSB, NOP_CLOSE);
        if (nChildRet < 0) {
            _WriteLog(LL_FATAL, "WriteGpxxmwToDevice failed");
            break;
        }
        _WriteLog(LL_INFO, "Invoice info has writed to device, notify TA now...");
        if ((nChildRet = CntaxQueryNetInvoiceReady(2, &hi, szInputFormatSingleLine)) < 0) {
            _WriteLog(LL_FATAL, "Gpxx write ok, notify TA failed");
            break;
        }
        _WriteLog(LL_INFO, "CntaxNetInvoiceDownloadWrite finish");
        nRet = 0;
    } while (false);

    // FreeAndExit:
    HTTPClose(&hi);
    return nRet;
}

//---------------------------------红票操作代码 begin----------------------------------------
void CntaxInsertRedSPXX(struct Spxx *spxx, mxml_node_t *xnode_spxx)
{
    mxml_node_t *xml_Node = NULL;
    char szBuf[256] = "";
    // xmmc
    sprintf(szBuf, "%s%s", spxx->spsmmc, spxx->spmc);
    xml_Node = mxmlNewElement(xnode_spxx, "xmmc");
    mxmlNewText(xml_Node, 0, szBuf);
    // xmdw
    xml_Node = mxmlNewElement(xnode_spxx, "xmdw");
    if (strlen(spxx->jldw) != 0)
        mxmlNewText(xml_Node, 0, spxx->jldw);
    // xmdj
    xml_Node = mxmlNewElement(xnode_spxx, "xmdj");
    memset(szBuf, 0, sizeof szBuf);
    if (!strcmp(spxx->hsjbz, "1")) {
        PriceRemoveTax(spxx->dj, spxx->slv, 15, szBuf);
        if (strlen(szBuf) != 0)
            mxmlNewText(xml_Node, 0, szBuf);
    } else {
        if (strlen(spxx->dj) != 0)
            mxmlNewText(xml_Node, 0, spxx->dj);
    }
    // xmslv
    xml_Node = mxmlNewElement(xnode_spxx, "xmslv");
    mxmlNewText(xml_Node, 0, spxx->slv);
    // xmggxh
    xml_Node = mxmlNewElement(xnode_spxx, "xmggxh");
    if (strlen(spxx->ggxh) != 0)
        mxmlNewText(xml_Node, 0, spxx->ggxh);
    // xmsl
    xml_Node = mxmlNewElement(xnode_spxx, "xmsl");
    if (strlen(spxx->sl) != 0)
        mxmlNewText(xml_Node, 0, spxx->sl);
    // xmje
    xml_Node = mxmlNewElement(xnode_spxx, "xmje");
    mxmlNewText(xml_Node, 0, spxx->je);
    // xmse
    xml_Node = mxmlNewElement(xnode_spxx, "xmse");
    mxmlNewText(xml_Node, 0, spxx->se);
    // hsbz;只支持不含税
    xml_Node = mxmlNewElement(xnode_spxx, "hsbz");
    mxmlNewText(xml_Node, 0, "N");
    // ssflbm
    xml_Node = mxmlNewElement(xnode_spxx, "ssflbm");
    if (strlen(spxx->spbh) != 0)
        mxmlNewText(xml_Node, 0, spxx->spbh);
    // qyzdybm
    xml_Node = mxmlNewElement(xnode_spxx, "qyzdybm");
    if (strlen(spxx->zxbh) != 0)
        mxmlNewText(xml_Node, 0, spxx->zxbh);
    // yhzcbz;优惠政策标识,是否使用优惠政策标识0：不使用，1：使用
    xml_Node = mxmlNewElement(xnode_spxx, "yhzcbz");
    if (strlen(spxx->yhsm) > 0)
        mxmlNewText(xml_Node, 0, "1");
    else
        mxmlNewText(xml_Node, 0, "0");
    // lslbz; 零税率标识,空：非零税率，0：出口退税，1：免税，2：不征收，3普通零税率
    xml_Node = mxmlNewElement(xnode_spxx, "lslbz");
    if (strlen(spxx->lslvbs) != 0)
        mxmlNewText(xml_Node, 0, spxx->lslvbs);
    // zzstsgl;优惠政策名字
    xml_Node = mxmlNewElement(xnode_spxx, "zzstsgl");
    if (strlen(spxx->yhsm) != 0)
        mxmlNewText(xml_Node, 0, spxx->yhsm);
}


int BuildCntaxUploadRedFormXML(HFPXX fpxx, char **szOut)
{
    //<tyxx>...</tyxx><ywxxs><ywxx><sqfnsrsbh>91320191MA1N8ERL56</sqfnsrsbh><sbbh>917004614901</sbbh><sqdbh>917004614901211126221241</sqdbh><xxblx>0</xxblx><yfpdm>3200203130</yfpdm><yfphm>41092939</yfphm><ykprq>202111</ykprq><szlb>1</szlb><dslbz>0</dslbz><tkrq>2021-11-26</tkrq><gmfmc>南京蒙柏信息产业有限公司</gmfmc><gmfsbh>91320116MA1MKN3U59</gmfsbh><xsfmc>南京学春耕企业管理咨询有限公司</xsfmc><xsfsbh>91320191MA1N8ERL56</xsfsbh><hjje>-100.00</hjje><slv>0.01</slv><hjse>-1.00</hjse><sqsm>0000000100</sqsm><ssflbmbbh>39.0</ssflbmbbh><slbz>0</slbz><xmmxs><mx><xmmc>*研发和技术服务*服务费6%</xmmc><xmdw/><xmdj/><xmslv>0.01</xmslv><xmggxh/><xmsl/><xmje>-100</xmje><xmse>-1.00</xmse><hsbz>N</hsbz><ssflbm>3040101000000000000</ssflbm><qyzdybm/><yhzcbz>0</yhzcbz><lslbz/><zzstsgl/></mx></xmmxs></ywxx></ywxxs></body>
    HDEV hDev = fpxx->hDev;
    char szBuf[256] = "";
    int nRet = ERR_GENERIC;
    mxml_node_t *root = NULL, *xml_root = NULL, *xml_Node = NULL;
    do {
        if (!(root = mxmlNewXML("1.0")))
            break;
        if (!(xml_root = mxmlNewElement(root, "ywxxs")))
            break;
        if (!(xml_root = mxmlNewElement(xml_root, "ywxx")))
            break;
        mxmlNewText(mxmlNewElement(xml_root, "sqfnsrsbh"), 0, hDev->szCommonTaxID);
        mxmlNewText(mxmlNewElement(xml_root, "sbbh"), 0, hDev->szDeviceID);
        // sqdbh; 申请单号 SN+yyMMddHHmmss
        memset(szBuf, 0, sizeof(szBuf));
        sprintf(szBuf, "%s%s", fpxx->hDev->szDeviceID, fpxx->hDev->szDeviceTime + 2);
        mxmlNewText(mxmlNewElement(xml_root, "sqdbh"), 0, szBuf);
        //信息表类型 0正常，1逾期
        mxmlNewText(mxmlNewElement(xml_root, "xxblx"), 0, "0");

        if (fpxx->hzxxbsqsm != 1) {
            // bluefpdm bluefpgm
            mxmlNewText(mxmlNewElement(xml_root, "yfpdm"), 0, fpxx->blueFpdm);
            mxmlNewText(mxmlNewElement(xml_root, "yfphm"), 0, fpxx->blueFphm);
            mxmlNewText(mxmlNewElement(xml_root, "ykprq"), 0, fpxx->ssyf);
        } else {
            mxmlNewElement(xml_root, "yfpdm");
            mxmlNewElement(xml_root, "yfphm");
            //无需kprq
        }
        //税种类别 0：营业税，1：增值税（默认）
        mxmlNewText(mxmlNewElement(xml_root, "szlb"), 0, "1");
        //多税率标志 0：一票一税率，1：一票多税率
        mxmlNewText(mxmlNewElement(xml_root, "dslbz"), 0, fpxx->isMultiTax ? "1" : "0");
        //填开日期
        TranslateStandTime(5, fpxx->kpsj_standard, szBuf);
        mxmlNewText(mxmlNewElement(xml_root, "tkrq"), 0, szBuf);
        // gf xf info
        mxmlNewText(mxmlNewElement(xml_root, "gmfmc"), 0, fpxx->gfmc);
        mxmlNewText(mxmlNewElement(xml_root, "gmfsbh"), 0, fpxx->gfsh);
        mxmlNewText(mxmlNewElement(xml_root, "xsfmc"), 0, fpxx->xfmc);
        mxmlNewText(mxmlNewElement(xml_root, "xsfsbh"), 0, fpxx->xfsh);
        // 价格信息
        mxmlNewText(mxmlNewElement(xml_root, "hjje"), 0, fpxx->je);
        mxmlNewText(mxmlNewElement(xml_root, "slv"), 0, fpxx->slv);
        mxmlNewText(mxmlNewElement(xml_root, "hjse"), 0, fpxx->se);
        //申请说明10位;0销方申请:0000000100 1购方申请已抵扣:1100000000 2购方申请未抵扣:1010000000
        if (fpxx->hzxxbsqsm == 0) {
            mxmlNewText(mxmlNewElement(xml_root, "sqsm"), 0, "0000000100");
        } else if (fpxx->hzxxbsqsm == 1) {
            mxmlNewText(mxmlNewElement(xml_root, "sqsm"), 0, "1100000000");
        } else if (fpxx->hzxxbsqsm == 2) {
            mxmlNewText(mxmlNewElement(xml_root, "sqsm"), 0, "1010000000");
        }
        // SPBMBBH
        mxmlNewText(mxmlNewElement(xml_root, "ssflbmbbh"), 0, fpxx->bmbbbh);
        // SLBZ;0：正常发票  1：减按计征  2：差额征收
        xml_Node = mxmlNewElement(xml_root, "slbz");
        if (!strcmp(fpxx->slv, "0.015") ||
            (fpxx->yysbz[8] == '0' && (!strcmp(fpxx->slv, "0.05") || !strcmp(fpxx->slv, "0.050"))))
            mxmlNewText(xml_Node, 0, "1");
        else if (ZYFP_CEZS == fpxx->zyfpLx)
            mxmlNewText(xml_Node, 0, "2");
        else
            mxmlNewText(xml_Node, 0, "0");
        // refresh root node
        if (!(xml_root = mxmlNewElement(xml_root, "xmmxs")))
            break;
        if (strcmp(fpxx->qdbj, "Y") == 0) {
            xml_Node = mxmlNewElement(xml_root, "mx");
            CntaxInsertRedSPXX(fpxx->stp_MxxxHead, xml_Node);
        } else {

            if (fpxx->spsl > 8) {
                strcpy(fpxx->stp_MxxxHead->spmc, "详见对应正数发票及清单");
                sprintf(fpxx->stp_MxxxHead->fphxz, "%d", FPHXZ_COMMON_SPXX);
                if (fpxx->isMultiTax == 0) {
                    strcpy(fpxx->stp_MxxxHead->slv, fpxx->stp_MxxxHead->stp_next->slv);
                    strcpy(fpxx->stp_MxxxHead->lslvbs, fpxx->stp_MxxxHead->stp_next->lslvbs);
                } else {
                    memset(fpxx->stp_MxxxHead->slv, 0, sizeof(fpxx->stp_MxxxHead->slv));
                    memset(fpxx->stp_MxxxHead->lslvbs, 0, sizeof(fpxx->stp_MxxxHead->lslvbs));
                }
                strcpy(fpxx->stp_MxxxHead->je, fpxx->je);
                strcpy(fpxx->stp_MxxxHead->se, fpxx->se);
                xml_Node = mxmlNewElement(xml_root, "mx");
                CntaxInsertRedSPXX(fpxx->stp_MxxxHead, xml_Node);

            } else {
                struct Spxx *stp_Spxx = fpxx->stp_MxxxHead->stp_next;
                while (stp_Spxx) {
                    xml_Node = mxmlNewElement(xml_root, "mx");
                    CntaxInsertRedSPXX(stp_Spxx, xml_Node);
                    stp_Spxx = stp_Spxx->stp_next;
                }
            }
        }
        *szOut = CntaxGetUtf8YwxxStringFromXmlRoot(root);
        if (*szOut == NULL)
            break;
        /*
        char *szoutTmp = mxmlSaveAllocString(root, MXML_NO_CALLBACK);
        //需要前置截断，去掉头部
        if (RemoveFullXmlHead(szoutTmp) < 0)
            break;
        int szoutlen = strlen(szoutTmp) * 2 + 1024;
        *szOut = calloc(1, szoutlen);
        G2U_UTF8Ignore(szoutTmp, strlen(szoutTmp), *szOut,
                        szoutlen);  //蒙柏的gbk模拟需要转换utf8兼容
        free(szoutTmp);
            //_WriteHexToDebugFile("reqRed-cntax.xml", *szOut, strlen(*szOut));
        */
        nRet = RET_SUCCESS;
    } while (false);

    mxmlDelete(root);
    return nRet;
}

int CntaxBuildDownloadFormXML(HDEV hDev, char *szQueryDateRange, char *szGfsh, char *szXXBBH,
                              char **szOut)
{
    //与组件文档提供的接口差异较大
    //
    //<business id="GGFW_HZXXBXZ" comment="红字信息表下载接口" jkbbh="1.0">
    //   <body><tyxx>...</tyxx><ywxx>
    //       <nsrsbh>91320191MA1N8ERL56</nsrsbh>
    //       <sbbh>917004614901</sbbh>
    //       <tkrqq>20211101</tkrqq>
    //       <tkrqz>20211126</tkrqz>
    //       <gmfsbh/>
    //       <xsfsbh/>
    //       <xxbbh/>
    //       <xxbfw>0</xxbfw>
    //       <yh>1</yh>
    //       <myjls/>
    //       <version>2.0</version>
    //     </ywxx></body></business>
    char szTKRQ_Q[64] = "", szTKRQ_Z[64] = "";
    if (strlen(szQueryDateRange) > 0 &&
        (17 != strlen(szQueryDateRange) || !strchr(szQueryDateRange, '-') ||
         '-' != szQueryDateRange[8])) {
        _WriteLog(LL_FATAL, "QueryDateRange format incorrect, example: 20200401-20200501");
        return ERR_PARM_CHECK;
    }
    /*if (!szGfsh || !szXXBBH) {
        _WriteLog(LL_FATAL, "Parament mustbe '', canot be NULL");
        return ERR_PARM_NEEDMORE;
    }*/
    //printf("11111111111111111111111111111111111111111112szGfsh=%s\r\n",szGfsh);
    *szOut = calloc(1, 1024);
    if (*szOut == NULL) {
        return ERR_BUF_ALLOC;
    }
    if (strlen(szQueryDateRange)) {
        // TKRQ_Q
        strcpy(szTKRQ_Q, szQueryDateRange);
        szTKRQ_Q[8] = '\0';
        // TKRQ_Z
        strcpy(szTKRQ_Z, szQueryDateRange + 9);
    }
    // XXBBH 红字发票信息表编号
    // XXBFW 信息表范围（0 所有  1本企业申请  2 本企业接收）
    sprintf(*szOut,
            "<ywxx><nsrsbh>%s</nsrsbh><sbbh>%s</sbbh><tkrqq>%s</tkrqq><tkrqz>%s</tkrqz><gmfsbh>%s</"
            "gmfsbh><xsfsbh/><xxbbh>%s</xxbbh><xxbfw>0</xxbfw><yh>1</yh><myjls/><version>2.0</"
            "version></ywxx>",
            hDev->szCommonTaxID, hDev->szDeviceID, szTKRQ_Q, szTKRQ_Z, szGfsh, szXXBBH);
    return RET_SUCCESS;
}
//---------------------------------红票操作代码 finish----------------------------------------