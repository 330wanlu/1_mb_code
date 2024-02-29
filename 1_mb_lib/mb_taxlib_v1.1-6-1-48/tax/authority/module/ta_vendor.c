/*****************************************************************************
File name:   ta_invoice.c
Description: 用于税局端各个厂商xml构建逻辑；
Author:      Zako
Version:     1.0
Date:        2021.08
History:
20210808     最初代码实现及他处代码移入
20211210     文件版本格式修改,此文件要求必须为utf8编码，原因税务ukey使用接口名中存在汉字字符,
             且mxml在增加节点时仅支持utf8编码，cntax以utf8形式编码，再进行转码
*****************************************************************************/
#include "ta_vendor.h"

//****************************************************************************************************
//****************************************************************************************************
//----------------------------------------aisino------------------------------------------------------
//****************************************************************************************************
//****************************************************************************************************
int AisinoBuildZZSXml(int nOpType, char *szDataPayload, HDEV hDev, char *pOutBuf)
{
    int nRet = -3;
    char szBuffer[512] = {0};
    int nB64BufLen = strlen(szDataPayload) * 2 + 512;
    void *pBase64Buff = calloc(1, nB64BufLen);
    if (!pBase64Buff)
        return -2;
    mxml_node_t *xml_Element = NULL, *xml_Node = NULL, *xml_Node2 = NULL;
    mxml_node_t *root = mxmlNewXML("1.0");

    xml_Node = mxmlNewElement(root, "ZZSFPXT");
    if (!xml_Node)
        goto Finish;
    //税控类型
    xml_Element = mxmlNewElement(xml_Node, "SK_TYPE");
    if (!xml_Element)
        goto Finish;
    mxmlNewText(xml_Element, 0, "JSP");  //金税盘
    //类型
    xml_Element = mxmlNewElement(xml_Node, "OP_TYPE");
    if (!xml_Element)
        goto Finish;
    sprintf(szBuffer, "%04d", nOpType);
    mxmlNewText(xml_Element, 0, szBuffer);

    xml_Node2 = mxmlNewElement(xml_Node, "INPUT");
    if (!xml_Node2)
        goto Finish;
    //客户端类型
    xml_Element = mxmlNewElement(xml_Node2, "CLIENT_TYPE");
    if (!xml_Element)
        goto Finish;
    memset(pBase64Buff, 0, nB64BufLen);
    UTF8ToGBKBase64("KP", 2, pBase64Buff);
    mxmlNewText(xml_Element, 0, pBase64Buff);
    //客户端版本号(Base64)
    xml_Element = mxmlNewElement(xml_Node2, "CLIENT_VER");
    if (!xml_Element)
        goto Finish;
    memset(pBase64Buff, 0, nB64BufLen);
    GetAisinoKPRJVersion(hDev->nDriverVersionNum, szBuffer);
    UTF8ToGBKBase64(szBuffer, strlen(szBuffer), pBase64Buff);
    mxmlNewText(xml_Element, 0, pBase64Buff);
    //底层版本号(Base64)
    xml_Element = mxmlNewElement(xml_Node2, "BOTTOM_VER");
    if (!xml_Element)
        goto Finish;
    memset(pBase64Buff, 0, nB64BufLen);
    UTF8ToGBKBase64(hDev->szDriverVersion, strlen(hDev->szDriverVersion), pBase64Buff);
    mxmlNewText(xml_Element, 0, pBase64Buff);
    //编码表版本号(Base64)
    xml_Element = mxmlNewElement(xml_Node2, "BMB_VER");
    if (!xml_Element)
        goto Finish;
    memset(pBase64Buff, 0, nB64BufLen);
    UTF8ToGBKBase64(DEF_BMBBBH_DEFAULT, strlen(DEF_BMBBBH_DEFAULT), pBase64Buff);
    mxmlNewText(xml_Element, 0, pBase64Buff);
    //主板序列号(Base64)
    xml_Element = mxmlNewElement(xml_Node2, "ZBXLH");
    if (!xml_Element)
        goto Finish;
    memset(pBase64Buff, 0, nB64BufLen);
    memset(szBuffer, 0, sizeof(szBuffer));
    GetMotherBoardId(hDev->szCommonTaxID, szBuffer);
    UTF8ToGBKBase64(szBuffer, strlen(szBuffer), pBase64Buff);
    mxmlNewText(xml_Element, 0, pBase64Buff);
    //开票软件版本号(Base64)（即总局发布版本号）
    xml_Element = mxmlNewElement(xml_Node2, "KPRJ_VER");
    if (!xml_Element)
        goto Finish;
    memset(pBase64Buff, 0, nB64BufLen);
    UTF8ToGBKBase64((char *)defTaxAuthorityVersionAisino, strlen(defTaxAuthorityVersionAisino),
                    pBase64Buff);
    mxmlNewText(xml_Element, 0, pBase64Buff);
    memset(pBase64Buff, 0, nB64BufLen);
    // ip地址(Base64)
    xml_Element = mxmlNewElement(xml_Node2, "IP");
    if (!xml_Element)
        goto Finish;
    UTF8ToGBKBase64(hDev->szClientWanIP, strlen(hDev->szClientWanIP), pBase64Buff);
    mxmlNewText(xml_Element, 0, pBase64Buff);
    //端口?
    xml_Element = mxmlNewElement(xml_Node2, "YDK");
    if (!xml_Element)
        goto Finish;
    UTF8ToGBKBase64(hDev->szClientWanPort, strlen(hDev->szClientWanPort), pBase64Buff);
    mxmlNewText(xml_Element, 0, pBase64Buff);
    // MAC地址((Base64))
    xml_Element = mxmlNewElement(xml_Node2, "MAC");
    if (!xml_Element)
        goto Finish;
    memset(pBase64Buff, 0, nB64BufLen);
    GetNicMAC(hDev->szCommonTaxID, szBuffer, 0);
    UTF8ToGBKBase64(szBuffer, strlen(szBuffer), pBase64Buff);
    mxmlNewText(xml_Element, 0, pBase64Buff);
    xml_Element = mxmlNewElement(xml_Node2, "BDBZ");
    if (!xml_Element)
        goto Finish;
    char bdbz[10] = {0};
    //如果税号不等于15位或者税号下标89两位不是DK则为1，否则为0(DK应该为代开的意思)
    if ((strlen(hDev->szCommonTaxID) != 15) || (memcmp(hDev->szCommonTaxID + 8, "DK", 2) != 0)) {
        sprintf(bdbz, "1");
    } else {
        sprintf(bdbz, "0");
    }
    memset(pBase64Buff, 0, nB64BufLen);
    UTF8ToGBKBase64(bdbz, strlen(bdbz), pBase64Buff);
    mxmlNewText(xml_Element, 0, pBase64Buff);
    //纳税人识别号
    xml_Element = mxmlNewElement(xml_Node2, "NSRSBH");
    if (!xml_Element)
        goto Finish;
    mxmlNewText(xml_Element, 0, hDev->szCommonTaxID);
    //开票机号
    xml_Element = mxmlNewElement(xml_Node2, "KPJH");
    if (!xml_Element)
        goto Finish;
    sprintf(szBuffer, "%d", hDev->uICCardNo);
    mxmlNewText(xml_Element, 0, szBuffer);
    //设备编号
    xml_Element = mxmlNewElement(xml_Node2, "SBBH");
    if (!xml_Element)
        goto Finish;
    mxmlNewText(xml_Element, 0, hDev->szDeviceID);
    //压缩标志, 实际上是超过512K采用zip压缩，我们这边就不压缩了
    xml_Element = mxmlNewElement(xml_Node2, "YSBZ");
    if (!xml_Element)
        goto Finish;
    mxmlNewText(xml_Element, 0, "N");
    //数据载荷
    xml_Element = mxmlNewElement(xml_Node2, "DATA");
    if (!xml_Element)
        goto Finish;
    memset(pBase64Buff, 0, nB64BufLen);
    if (UTF8ToGBKBase64(szDataPayload, strlen(szDataPayload), pBase64Buff) < 0)
        goto Finish;
    mxmlNewText(xml_Element, 0, pBase64Buff);
    // MD5(发票信息xml字符串)
    xml_Element = mxmlNewElement(xml_Node2, "CRC");
    if (!xml_Element)
        goto Finish;
    // upload3 的xml头为GBK，因此要转GBK
    memset(pBase64Buff, 0, nB64BufLen);
    if (U2G_GBKIgnore(szDataPayload, strlen(szDataPayload), pBase64Buff, nB64BufLen) < 0)
        goto Finish;
    CalcMD5(pBase64Buff, strlen(pBase64Buff), (void *)szBuffer);
    char MD5_S[64] = {0};
    int i;
    for (i = 0; i < 16; i++) {
        sprintf(MD5_S + strlen(MD5_S), "%02X", (uint8)(szBuffer)[i]);
    }
    mxmlNewText(xml_Element, 0, MD5_S);
    char *ptr = mxmlSaveAllocString(root, MXML_NO_CALLBACK);
    int nStrLen = strlen(ptr);
    memcpy(pOutBuf, ptr, nStrLen);
    free(ptr);
    nRet = 0;
    //段尾调整
    if (pOutBuf[nStrLen - 1] == '\n')
        pOutBuf[nStrLen - 1] = '\0';

Finish:
    free(pBase64Buff);
    mxmlDelete(root);
    return nRet;
}

bool AisinoTechnologyXmlGetServiceId(int nOpType, char *szServiceID)
{
    bool ret = true;
    switch (nOpType) {
        case TACMD_AISINO_UPFP_ZZS:
        case TACMD_AISINO_SYNC_COMPANYINFO:
        case TACMD_AISINO_QUERY_FPUP_ZZS:
        case TACMD_AISINO_REPORT_ZZS:
        case TACMD_AISINO_CLEAR_ZZS:
        case TACMD_AISINO_REDFORM_UPLOAD_ZZS:
        case TACMD_AISINO_REDFORM_DOWNLOAD_ZZS:
        case TACMD_AISINO_REDFORM_PTCHECK:
        case TACMD_AISINO_QUERY_APPLICANT:
        case TACMD_AISINO_QUERY_INVTYPELIST:
        case TACMD_AISINO_QUERY_NETINV_READY:
        case TACMD_AISINO_DOWNLOAD_NETINV_READY:
            strcpy(szServiceID, "jsp_dzdz_cb_zzszp");
            break;
        case TACMD_AISINO_UPFP_DZ:
        case TACMD_AISINO_QUERY_FPUP_DZ:
        case TACMD_AISINO_REPORT_DZ:
        case TACMD_AISINO_CLEAR_DZ:
        case TACMD_AISINO_REDFORM_DZCHECK:
            strcpy(szServiceID, "jsp_dzfp_cb");
            break;
        case TACMD_AISINO_TZDBH_CANCEL:
        case TACMD_AISINO_PUBSERV_JOINCANCEL:
            strcpy(szServiceID, "dzdz_gpxx_shenqing_hyfp");
            break;
        default:
            ret = false;
            break;
    }
    return ret;
}

//新节点,目前目前测试仅仅点击开票软件测试上传服务器是否可用时会用到，目前我们现有业务永不上
bool AisinoTechnologyXmlAttachment(int nOpType, mxml_node_t *xml_root, HDEV hDev)
{
    mxml_node_t *xmlNodeChild = NULL, *xmlNodeKeyValue = NULL;
    if (nOpType == -1) {  // no use
        //<tripTechnologyPackage version="1.0"><identity>...</identity><attachment><Properties>
        //<Key Name="FRAMEWORK_BIZ_SUBTYPE">NETWORK_CONNECT_CHECK</Key>
        //<Key Name="FRAMEWORK_CURRENT_VERSION">V3.2.00.220328</Key>
        //</Properties>/</attachment>/</tripTechnologyPackage>
        xmlNodeChild = mxmlNewElement(xml_root, "attachment");
        xmlNodeChild = mxmlNewElement(xmlNodeChild, "Properties");
        // node1
        xmlNodeKeyValue = mxmlNewElement(xmlNodeChild, "Key");
        mxmlElementSetAttr(xmlNodeKeyValue, "Name", "FRAMEWORK_BIZ_SUBTYPE");
        mxmlNewText(xmlNodeKeyValue, 0, "NETWORK_CONNECT_CHECK");
        // node2
        xmlNodeKeyValue = mxmlNewElement(xmlNodeChild, "Key");
        mxmlElementSetAttr(xmlNodeKeyValue, "Name", "FRAMEWORK_CURRENT_VERSION");
        mxmlNewText(xmlNodeKeyValue, 0, "V3.2.00.220328");
    }
    return true;
}

int AisinoTechnologyXML(int nOpType, HDEV hDev, char *szOutBuf, int nSizeOutBuf)
{
    mxml_node_t *xml_root = NULL, *xmlNodeIdentity = NULL;
    char szBuff[128] = "";
    char szBuff2[64] = "";
    char szServiceID[32] = "";
    int nRet = ERR_GENERIC;

    mxml_node_t *root = mxmlNewXML("1.0U");
    do {
        if (!AisinoTechnologyXmlGetServiceId(nOpType, szServiceID))
            break;
        if (!(xml_root = mxmlNewElement(root, "tripTechnologyPackage")))
            break;
        mxmlElementSetAttr(xml_root, "version", "1.0");
        if (!(xmlNodeIdentity = mxmlNewElement(xml_root, "identity")))
            break;
        mxmlNewText(mxmlNewElement(xmlNodeIdentity, "applicationId"), 0, "zzs_fp_cgl_cj");
        mxmlNewText(mxmlNewElement(xmlNodeIdentity, "serviceId"), 0, szServiceID);
        mxmlNewText(mxmlNewElement(xmlNodeIdentity, "nsrsbh"), 0, hDev->szCommonTaxID);
        //++version 新版已经改为开票人名称
        mxmlNewText(mxmlNewElement(xmlNodeIdentity, "senderName"), 0, "KP_USER");
        GetOSTime(szBuff2);
        TranslateStandTime(4, szBuff2, szBuff);
        mxmlNewText(mxmlNewElement(xmlNodeIdentity, "sendTime"), 0, szBuff);
        GetRandom(szBuff2, 9, 10, NULL);
        sprintf(szBuff, "%s_%s", hDev->szCommonTaxID, szBuff2);
        mxmlNewText(mxmlNewElement(xmlNodeIdentity, "globalBusinessId"), 0, szBuff);
        mxmlNewText(mxmlNewElement(xmlNodeIdentity, "command"), 0, "GENERAL_COMMUNITY");
        mxmlNewText(mxmlNewElement(xmlNodeIdentity, "NewUpload"), 0, "0");
        if (!AisinoTechnologyXmlAttachment(nOpType, xml_root, hDev))
            break;

        char *ptr = mxmlSaveAllocString(root, MXML_NO_CALLBACK);  // MXML_NO_CALLBACK/XML_Formater
        if (strlen(ptr) > nSizeOutBuf - 1) {
            _WriteLog(LL_FATAL, "cmp xml data too large\n");
            free(ptr);
            break;
        }
        strcpy(szOutBuf, ptr);
        free(ptr);
        nRet = RET_SUCCESS;
    } while (false);
    if (root)
        mxmlDelete(root);
    return nRet;
}

int AisinoSyncCompanyInfo(HHTTP hi, bool bNeedUpdateIO)
{
    char szBuf[8192] = "";
    int nRet = RET_SUCCESS;
    char *pPayload = NULL;
    mxml_node_t *pXMLRoot = NULL, *node = NULL;
    do {
        if (AisinoPreCompanyXml(hi->hDev, szBuf) < 0) {
            nRet = -1;
            break;
        }
        if ((nRet = TaNetIo(hi, TACMD_AISINO_SYNC_COMPANYINFO, szBuf, sizeof(szBuf))) < 0) {
			if (bNeedUpdateIO == false){
				//部分税盘自身企业信息异常，但是不影响后续操作，只是不再同步企业信息，处理方式为：当作同步上传正确，但不进行同步反写
				nRet = RET_SUCCESS;
				_WriteLog(LL_WARN, "aisino企业信息同步异常， 参数下载null，视为正确");
			}
			else
				break;
        }
        //部分盘同步企业信息会失败，判断连接服务不用TaNetIo及后续
        if (bNeedUpdateIO == false)
            break;
        //<FPXT><OUTPUT><CODE>0000</CODE><MESS>成功</MESS><DATA><NSRSBH>...</NSRSBH><KPJH>0</KPJH><SBBH>...</SBBH><SQXX></SQXX>
        //<SCCS>11,1,1</SCCS><SPBMBBH></SPBMBBH><SPBM></SPBM><SPBMXX></SPBMXX><NSRXZ>1</NSRXZ><HYLX>0000000000</HYLX><TLQYBZ>0</TLQYBZ>
        //<XGMZKZPFLAG>0</XGMZKZPFLAG><BGXX><TZQSLSQBS/><KKJTZQSL_JZSJ/></BGXX><KPBB></KPBB><KPKZRQ></KPKZRQ><BMBB></BMBB>
        //<BMKZRQ></BMKZRQ><FXYJJB></FXYJJB></DATA></OUTPUT></FPXT>
        if (LoadZZSXMLDataNodeTemplate(&pXMLRoot, &node, "SQXX", szBuf) < 0) {
            nRet = -3;
            break;
        }
        //存在SQXX时说明税局对税盘进行了策略调整，需要尽快更新税盘授权信息，短期内不更新问题也不大
        if ((pPayload = (char *)mxmlGetOpaque(node)) != NULL) {
            _WriteLog(LL_WARN, "Aisino SyncCompanyInfo need update");
            if (!bNeedUpdateIO)
                break;
            //暂时去掉  lbc   2022-10-28
            // if (UpdateCompanyInfo(hi->hDev->hUSB, pPayload) < 0) {
            //     _WriteLog(LL_FATAL, "UpdateCompanyInfo failed");
            //     nRet = -4;
            //     break;
            // }
        }
    } while (false);
    return nRet;
}
//****************************************************************************************************
//****************************************************************************************************
//----------------------------------------nisec------------------------------------------------------
//****************************************************************************************************
//****************************************************************************************************
int NisecBuildZZSXml(int nOpType, char *szDataPayload, HDEV hDev, char *pOutBuf)
{
    int nRet = -3;
    char szBuffer[512] = {0};
    int nB64BufLen = strlen(szDataPayload) * 2 + 512;
    void *pBase64Buff = calloc(1, nB64BufLen);
    if (!pBase64Buff)
        return -2;
    mxml_node_t *xml_Element = NULL, *xml_Node = NULL, *xml_Node2 = NULL;
    mxml_node_t *root = mxmlNewXML("1.0");

    xml_Node = mxmlNewElement(root, "ZZSFPXT");
    if (!xml_Node)
        goto BuildZZSXml_Finish;

    //税控类型
    xml_Element = mxmlNewElement(xml_Node, "SK_TYPE");
    if (!xml_Element)
        goto BuildZZSXml_Finish;
    mxmlNewText(xml_Element, 0, "SKP");  //税控盘
    //类型
    xml_Element = mxmlNewElement(xml_Node, "OP_TYPE");
    if (!xml_Element)
        goto BuildZZSXml_Finish;
    sprintf(szBuffer, "%04d", nOpType);
    mxmlNewText(xml_Element, 0, szBuffer);
    xml_Node2 = mxmlNewElement(xml_Node, "INPUT");
    if (!xml_Node2)
        goto BuildZZSXml_Finish;
    //纳税人识别号
    mxmlNewText(mxmlNewElement(xml_Node2, "NSRSBH"), 0, hDev->szCommonTaxID);
    //开票机号
    sprintf(szBuffer, "%d", hDev->uICCardNo);
    mxmlNewText(mxmlNewElement(xml_Node2, "KPJH"), 0, szBuffer);
    //设备编号
    mxmlNewText(mxmlNewElement(xml_Node2, "SBBH"), 0, hDev->szDeviceID);
    //发票类型编码
    sprintf(szBuffer, "%03d", hDev->bOperateInvType);
    mxmlNewText(mxmlNewElement(xml_Node2, "FPLX_DM"), 0, szBuffer);
    // ip地址 20220629测试，官方已更改为 <IP>1.1.1.1:42445</IP>
    memset(szBuffer, 0, sizeof(szBuffer));
    sprintf(szBuffer, "%s:%s", hDev->szClientWanIP, hDev->szClientWanPort);
    mxmlNewText(mxmlNewElement(xml_Node2, "IP"), 0, szBuffer);
    // MAC地址
    GetNicMAC(hDev->szCommonTaxID, szBuffer, 1);
    mxmlNewText(mxmlNewElement(xml_Node2, "MAC"), 0, szBuffer);
    //总局发布版本号
    mxmlNewText(mxmlNewElement(xml_Node2, "RJBBH"), 0, defTaxAuthorityVersionNisec);
    //编码表版本号
    mxmlNewText(mxmlNewElement(xml_Node2, "BMBBH"), 0, DEF_BMBBBH_DEFAULT);
    // BDBZ
    xml_Element = mxmlNewElement(xml_Node2, "BDBZ");
    if (!xml_Element)
        goto BuildZZSXml_Finish;
    if ((strlen(hDev->szCommonTaxID) != 15) || (memcmp(hDev->szCommonTaxID + 8, "DK", 2) != 0))
        sprintf(szBuffer, "1");
    else
        sprintf(szBuffer, "0");
    mxmlNewText(xml_Element, 0, szBuffer);
    // 主板序列号
    memset(szBuffer, 0, sizeof(szBuffer));
    GetMotherBoardId(hDev->szCommonTaxID, szBuffer);
    mxmlNewText(mxmlNewElement(xml_Node2, "ZBXLH"), 0, szBuffer);
    // YDK  20220629测试，官方已更改为 <YDK>0</YDK>
    mxmlNewText(mxmlNewElement(xml_Node2, "YDK"), 0, "0");
    //数据载荷
    xml_Element = mxmlNewElement(xml_Node2, "DATA");
    if (!xml_Element)
        goto BuildZZSXml_Finish;
    memset(pBase64Buff, 0, nB64BufLen);
    if (UTF8ToGBKBase64(szDataPayload, strlen(szDataPayload), pBase64Buff) < 0)
        goto BuildZZSXml_Finish;
    mxmlNewText(xml_Element, 0, pBase64Buff);
    // MD5(发票信息xml字符串)
    xml_Element = mxmlNewElement(xml_Node2, "CRC");
    if (!xml_Element)
        goto BuildZZSXml_Finish;
    // upload3 的xml头为GBK，因此要转GBK
    memset(pBase64Buff, 0, nB64BufLen);
    if (U2G_GBKIgnore(szDataPayload, strlen(szDataPayload), pBase64Buff, nB64BufLen) < 0)
        goto BuildZZSXml_Finish;
    CalcMD5(pBase64Buff, strlen(pBase64Buff), (void *)szBuffer);
    char MD5_S[64] = {0};
    int i;
    for (i = 0; i < 16; i++) {
        sprintf(MD5_S + strlen(MD5_S), "%02X", (uint8)(szBuffer)[i]);
    }
    mxmlNewText(xml_Element, 0, MD5_S);
    // save
    char *ptr = mxmlSaveAllocString(root, MXML_NO_CALLBACK);
    int nStrLen = strlen(ptr);
    memcpy(pOutBuf, ptr, nStrLen);
    free(ptr);
    nRet = 0;
    //段尾调整
    if (pOutBuf[nStrLen - 1] == '\n')
        pOutBuf[nStrLen - 1] = '\0';

BuildZZSXml_Finish:
    free(pBase64Buff);
    mxmlDelete(root);
    return nRet;
}

//上传xml第一层的element1, pc_xml > 1280
int NisecTechnologyXML(int nOpType, HDEV hDev, char *pc_xml)
{
    mxml_node_t *xml_root = NULL, *xml_Node = NULL;
    char szBuff[128] = "";
    char szBuff2[64] = "";
    char szServiceID[32] = "";
    switch (nOpType) {
        //-----------------nisec-------------
        case TACMD_NISEC_QUERY_FPUP_ZZS:
        case TACMD_NISEC_TZDBH_CANCEL:
        case TACMD_NISEC_PUBSERV_JOINCANCEL:
        case TACMD_NISEC_SYNC_COMPANYINFO:
            strcpy(szServiceID, "skp_dzdz_cb_hyfp");
            break;
        case TACMD_NISEC_UPFP_ZZS:
        case TACMD_NISEC_REPORT_ZZS:
        case TACMD_NISEC_CLEAR_ZZS:
        case TACMD_NISEC_REDFORM_UPLOAD_ZZS:
        case TACMD_NISEC_REDFORM_DOWNLOAD_ZZS:
            strcpy(szServiceID, "skp_dzdz_cb_zzszp");
            break;
        case TACMD_NISEC_UPFP_DZ:
        case TACMD_NISEC_QUERY_FPUP_DZ:
        case TACMD_NISEC_REPORT_DZ:
        case TACMD_NISEC_CLEAR_DZ:
            strcpy(szServiceID, "skp_dzfp_cb");
            break;
        case TACMD_NISEC_NETINV_DO:
            strcpy(szServiceID, "dzdz_gpxx_xiazai_hyfp");
            break;
        default:
            return -1;
            break;
    }
    mxml_node_t *root = mxmlNewXML("1.0U");
    xml_root = mxmlNewElement(root, "tripTechnologyPackage");
    if (!xml_root) {
        mxmlDelete(root);
        return -1;
    }
    mxmlElementSetAttr(xml_root, "version", "1.0");
    xml_Node = mxmlNewElement(xml_root, "identity");
    mxmlNewText(mxmlNewElement(xml_Node, "applicationId"), 0, "hyfp_001");
    mxmlNewText(mxmlNewElement(xml_Node, "serviceId"), 0, szServiceID);
    mxmlNewText(mxmlNewElement(xml_Node, "nsrsbh"), 0, hDev->szCommonTaxID);
    mxmlNewElement(xml_Node, "password");
    mxmlNewText(mxmlNewElement(xml_Node, "senderName"), 0, hDev->szCommonTaxID);
    GetOSTime(szBuff2);
    TranslateStandTime(1, szBuff2, szBuff);
    mxmlNewText(mxmlNewElement(xml_Node, "sendTime"), 0, szBuff);
    GetRandom(szBuff2, 10, 10, NULL);
    sprintf(szBuff, "%s%s", hDev->szCommonTaxID, szBuff2);
    mxmlNewText(mxmlNewElement(xml_Node, "globalBusinessId"), 0, szBuff);
    mxmlNewText(mxmlNewElement(xml_Node, "command"), 0, "SEND");
    char *ptr = mxmlSaveAllocString(root, MXML_NO_CALLBACK);  // MXML_NO_CALLBACK/XML_Formater
    strcpy(pc_xml, ptr);
    free(ptr);
    mxmlDelete(root);
    return 0;
}

int NisecSyncCompanyInfo(HHTTP hi, bool bNeedUpdateIO)
{
    int nRet = ERR_GENERIC;
    int nIOBufSize = 819200;  //此处太小会返回失败
    char *pPreCompnany = NULL, *pIoBuf = NULL;
    mxml_node_t *pXMLRoot = NULL, *node = NULL;
    char szValue[256] = "";
    do {
        if (!(pIoBuf = calloc(1, nIOBufSize))) {
            nRet = ERR_BUF_ALLOC;
            break;
        }
        if (!(pPreCompnany = NisecPreCompanyXml(hi->hDev))) {
            nRet = ERR_BUF_CHECK;
            break;
        }
        if (strlen(pPreCompnany) > nIOBufSize - 1) {
            _WriteLog(LL_WARN, "PreCompanyXml ret buf too big");
            nRet = ERR_BUF_CHECK;
            break;
        }
        //_WriteHexToDebugFile("NisecSyncCompanyInfo.xml", (uint8 *)pPreCompnany,
        // strlen(pPreCompnany));
        strcpy(pIoBuf, pPreCompnany);
        if ((nRet = TaNetIo(hi, TACMD_NISEC_SYNC_COMPANYINFO, pIoBuf, nIOBufSize)) < 0)
            break;
        //部分盘同步企业信息会失败，判断连接服务不用TaNetIo及后续
        if (bNeedUpdateIO == false) {
            nRet = RET_SUCCESS;
            break;
        }
        //<body><returncode>00</returncode><returnmsg>成功</returnmsg><nsrsbh>91320118MA1X7GFL8Q</nsrsbh><sbbh>539912237528</sbbh><sblx>00</sblx><sbbbh>1105220316</sbbbh><qycsxz>
        //<returncode>00</returncode><returnmsg>成功</returnmsg><sczs>1</sczs><scsjjg>11</scsjjg><cs1>1</cs1><cs2></cs2></qycsxz><qylxxx><returncode>00</returncode><returnmsg>成功</returnmsg>
        //<kprjbb></kprjbb><ssbmbb></ssbmbb><kprjsjsj></kprjsjsj><ssbmsjsj></ssbmsjsj><fxyjjb></fxyjjb><xzkzszybz>0</xzkzszybz><fplx><group
        // xh="1"><fplxdm>004</fplxdm><lxkzxx>...</lxkzxx></group> <group
        // xh="2"><fplxdm>026</fplxdm><lxkzxx>...</lxkzxx></group></fplx></qylxxx><ycbgcx><returncode>01</returncode><returnmsg>没有查询到可变更项，不用变更</returnmsg></ycbgcx><spbm>
        //<returncode>00</returncode><returnmsg>成功</returnmsg><ysbz>1</ysbz><ysnr>...</ysnr></spbm><hqszxx><returncode>99800011</returncode><returnmsg>服务器、税控器具时间差小于30分钟,不用校准税控器具时钟</returnmsg>
        //</hqszxx><tzqslsq><returncode>90000001</returncode><returnmsg>无可下载的调整前税率授权信息</returnmsg></tzqslsq><qltkzxx><returncode>00</returncode><returnmsg>成功</returnmsg><fxyjjb></fxyjjb></qltkzxx>

        // load
        if ((pXMLRoot = mxmlLoadString(NULL, pIoBuf, MXML_OPAQUE_CALLBACK)) == NULL)
            break;
        //--fplx 更新监控信息,lxkzxx永远存在值，无论本地是否可远程一致    暂时去掉   lbc  2022-10-28
        // if (NisecUpdateMonitorFromTaSync(hi->hDev->hUSB, pXMLRoot))
        //     break;
        //--ycbgcx 远程变更，主要是税号、企业名称变化之类的，改动较大，暂不同步
        if ((node = mxmlFindElement(pXMLRoot, pXMLRoot, "ycbgcx", NULL, NULL, MXML_DESCEND)) ==
            NULL)
            break;
        if (GetChildNodeValue(node, "returncode", szValue) < 0)
            break;
        if (1 != atoi(szValue)) {  //存在需要远程变更
            if (GetChildNodeValue(node, "returnmsg", szValue) < 0)
                break;
            _WriteLog(LL_WARN,
                      "SyncCompanyInfo get TA command to modify device's envirnments, msg:[%s]",
                      szValue);
        }
        //--spbm 跳过分类编码更新和开票软件相关参数下载
        //
        //--hqszxx 时钟更新。如果时钟存在差异，我方不能修改，只提示去税局改
        if ((node = mxmlFindElement(pXMLRoot, pXMLRoot, "hqszxx", NULL, NULL, MXML_DESCEND)) ==
            NULL)
            break;
        if (GetChildNodeValue(node, "returncode", szValue) < 0)
            break;
        if (99800011 != atoi(szValue)) {
            if (GetChildNodeValue(node, "returnmsg", szValue) < 0)
                break;
            _WriteLog(LL_WARN, "SyncCompanyInfo found device's time incorrect, msg:[%s]", szValue);
        }
        //--tzqslsq 调整前税率，无环境跳过
        if ((node = mxmlFindElement(pXMLRoot, pXMLRoot, "tzqslsq", NULL, NULL, MXML_DESCEND)) ==
            NULL)
            break;
        if (GetChildNodeValue(node, "returncode", szValue) < 0)
            break;
        if (90000001 != atoi(szValue)) {
            if (GetChildNodeValue(node, "returnmsg", szValue) < 0)
                break;
            _WriteLog(LL_WARN, "SyncCompanyInfo found device's tzqslsq, msg:[%s]", szValue);
        }
        //--qltkzxx 未知
        if ((node = mxmlFindElement(pXMLRoot, pXMLRoot, "qltkzxx", NULL, NULL, MXML_DESCEND)) ==
            NULL)
            break;
        if (GetChildNodeValue(node, "returncode", szValue) < 0)
            break;
        if (0 != atoi(szValue)) {
            if (GetChildNodeValue(node, "returnmsg", szValue) < 0)
                break;
            _WriteLog(LL_WARN, "SyncCompanyInfo found device's qltkzxx, msg:[%s]", szValue);
        }
        nRet = RET_SUCCESS;
    } while (false);
    if (pPreCompnany)
        free(pPreCompnany);
    if (pIoBuf)
        free(pIoBuf);
    if (pXMLRoot)
        mxmlDelete(pXMLRoot);
    return nRet;
}

//****************************************************************************************************
//****************************************************************************************************
//----------------------------------------cntax------------------------------------------------------
//****************************************************************************************************
//****************************************************************************************************
int CntaxSyncCompanyInfo(HHTTP hi, bool bNeedUpdateIO)
{
    // cntax企业信息同步之前有个检测固件更新的，暂不考虑
    //<business id="CHECKUPDATE">/<body>/<nsrsbh>taxid</nsrsbh>/<csbs>53</csbs><sbbh>devid
    //</sbbh><gjbbh>ver1-ver2</gjbbh><yjbbh>ABCDEF</yjbbh></body></business>
    int nRet = ERR_GENERIC;
    int nIOBufSize = 819200;  //此处太小会返回失败
    char *pPreCompnany = NULL, *pIoBuf = NULL;
    mxml_node_t *pXMLRoot = NULL, *node = NULL;
    char szValue[256] = "";
    do {
        if (!(pIoBuf = calloc(1, nIOBufSize))) {
            nRet = ERR_BUF_ALLOC;
            break;
        }
        if (!(pPreCompnany = CntaxPreCompanyXml(hi->hDev))) {
            nRet = ERR_BUF_CHECK;
            break;
        }
        if (strlen(pPreCompnany) > nIOBufSize - 1) {
            _WriteLog(LL_WARN, "PreCompanyXml ret buf too big");
            nRet = ERR_BUF_CHECK;
            break;
        }
        //_WriteHexToDebugFile("CntaxSyncCompanyInfo.xml", (uint8 *)pPreCompnany,
        // strlen(pPreCompnany));
        strcpy(pIoBuf, pPreCompnany);
        if ((nRet = TaNetIo(hi, TACMD_CNTAX_SYNC_COMPANYINFO, pIoBuf, nIOBufSize)) < 0)
            break;
        //部分盘同步企业信息会失败，判断连接服务不用TaNetIo及后续
        if (bNeedUpdateIO == false) {
            nRet = RET_SUCCESS;
            break;
        }
        //<body><tyxx><nsrsbh>91320118MA20LU6N3P</nsrsbh><sbbh>537100494968</sbbh><returncode>00</returncode><returnmsg>成功</returnmsg></tyxx>
        //<ywxx><qycsxz><returncode>00</returncode><returnmsg>成功</returnmsg><sczs>1</sczs><scsjjg>11</scsjjg><cs1>1</cs1><cs2></cs2></qycsxz>
        //<qylxxx><returncode>00</returncode><returnmsg>成功</returnmsg><kprjbb></kprjbb><ssbmbb></ssbmbb><kprjsjsj></kprjsjsj><ssbmsjsj></ssbmsjsj>
        //<fxyjjb></fxyjjb><xzkzszybz>0</xzkzszybz><yhxx>...</yhxx><fplx><group
        // xh="1"><fplxdm>007</fplxdm><lxkzxx>...</lxkzxx></group> <group
        // xh="2"><fplxdm>026</fplxdm><lxkzxx>...</lxkzxx></group></fplx></qylxxx><ycbgcx><returncode>01</returncode><returnmsg>
        //没有查询到可变更项，不用变更</returnmsg></ycbgcx><ssflbm><returncode>00</returncode><returnmsg>成功</returnmsg><ysbz>1</ysbz><ysnr>...</ysnr>
        //</ssflbm><hqszxx><returncode>99800011</returncode><returnmsg>服务器、税控器具时间差小于30分钟,不用校准税控器具时钟</returnmsg></hqszxx><qltkzxx>
        //<returncode>00</returncode><returnmsg>成功</returnmsg><fxyjjb></fxyjjb></qltkzxx></ywxx></body></business>

        // load
        if ((pXMLRoot = mxmlLoadString(NULL, pIoBuf, MXML_OPAQUE_CALLBACK)) == NULL)
            break;

        //暂时去掉  lbc 20221028
        // if (CntaxUpdateMonitorFromTaSync(hi->hDev->hUSB, hi->hDev->abSupportInvtype[1], pXMLRoot))
        //     break;

        //--ycbgcx 远程变更，主要是税号、企业名称变化之类的，改动较大，暂不同步
        if ((node = mxmlFindElement(pXMLRoot, pXMLRoot, "ycbgcx", NULL, NULL, MXML_DESCEND)) ==
            NULL)
            break;
        if (GetChildNodeValue(node, "returncode", szValue) < 0)
            break;
        if (1 != atoi(szValue)) {  //存在需要远程变更
            if (GetChildNodeValue(node, "returnmsg", szValue) < 0)
                break;
            _WriteLog(LL_WARN,
                      "SyncCompanyInfo get TA command to modify device's envirnments, msg:[%s]",
                      szValue);
        }
        //--spbm 跳过分类编码更新和开票软件相关参数下载
        //
        //--hqszxx 时钟更新。如果时钟存在差异，我方不能修改，只提示去税局改
        if ((node = mxmlFindElement(pXMLRoot, pXMLRoot, "hqszxx", NULL, NULL, MXML_DESCEND)) ==
            NULL)
            break;
        if (GetChildNodeValue(node, "returncode", szValue) < 0)
            break;
        if (99800011 != atoi(szValue)) {
            if (GetChildNodeValue(node, "returnmsg", szValue) < 0)
                break;
            _WriteLog(LL_WARN, "SyncCompanyInfo found device's time incorrect, msg:[%s]", szValue);
        }
        //--qltkzxx 未知
        if ((node = mxmlFindElement(pXMLRoot, pXMLRoot, "qltkzxx", NULL, NULL, MXML_DESCEND)) ==
            NULL)
            break;
        if (GetChildNodeValue(node, "returncode", szValue) < 0)
            break;
        if (0 != atoi(szValue)) {
            if (GetChildNodeValue(node, "returnmsg", szValue) < 0)
                break;
            _WriteLog(LL_WARN, "SyncCompanyInfo found device's qltkzxx, msg:[%s]", szValue);
        }

        nRet = RET_SUCCESS;
    } while (false);
    if (pPreCompnany)
        free(pPreCompnany);
    if (pIoBuf)
        free(pIoBuf);
    if (pXMLRoot)
        mxmlDelete(pXMLRoot);
    return nRet;
}

int SetCntaxBusinessAttr(int nOpType, mxml_node_t *xmlBusiness)
{
    mxmlElementSetAttr(xmlBusiness, "jkbbh", "1.0");
    switch (nOpType) {
        case TACMD_CNTAX_UPFP:
            mxmlElementSetAttr(xmlBusiness, "comment", "发票上传接口");
            mxmlElementSetAttr(xmlBusiness, "id", "GGFW_FPSC");
            break;
        case TACMD_CNTAX_QUERY_UPFP:
            mxmlElementSetAttr(xmlBusiness, "comment", "获取发票上传结果接口");
            mxmlElementSetAttr(xmlBusiness, "id", "GGFW_HQFPSCJG");
            break;
        case TACMD_CNTAX_REPORT:
            mxmlElementSetAttr(xmlBusiness, "comment", "发票汇总信息上传接口");
            mxmlElementSetAttr(xmlBusiness, "id", "GGFW_FPHZXXSC");
            break;
        case TACMD_CNTAX_CLEAR:
            mxmlElementSetAttr(xmlBusiness, "comment", "监控信息回传接口");
            mxmlElementSetAttr(xmlBusiness, "id", "GGFW_JKXXHC");
            break;
        case TACMD_CNTAX_NETINV_QUERY:
            mxmlElementSetAttr(xmlBusiness, "comment", "网上领票信息查询");
            mxmlElementSetAttr(xmlBusiness, "id", "GGFW_WSLPXXCX");
            break;
        case TACMD_CNTAX_NETINV_WRITE:
            mxmlElementSetAttr(xmlBusiness, "comment", "领票信息申请");
            mxmlElementSetAttr(xmlBusiness, "id", "GGFW_WSLPXXSQ");
            break;
        case TACMD_CNTAX_REDFORM_UPLOAD:
            mxmlElementSetAttr(xmlBusiness, "comment", "红字信息表上传接口");
            mxmlElementSetAttr(xmlBusiness, "id", "GGFW_HZXXBSC");
            break;
        case TACMD_CNTAX_REDFORM_DOWNLOAD:
            mxmlElementSetAttr(xmlBusiness, "comment", "红字信息表下载接口");
            mxmlElementSetAttr(xmlBusiness, "id", "GGFW_HZXXBXZ");
            break;
        case TACMD_CNTAX_REDFORM_CANCEL:
            mxmlElementSetAttr(xmlBusiness, "comment", "红字信息表撤销");
            mxmlElementSetAttr(xmlBusiness, "id", "GGFW_HZXXBCX");
            break;
        case TACMD_CNTAX_NETINV_CONFIRM:
            mxmlElementSetAttr(xmlBusiness, "comment", "领票信息写盘结果确认");
            mxmlElementSetAttr(xmlBusiness, "id", "GGFW_LPXXXPJGQR");
            break;
        case TACMD_CNTAX_SYNC_COMPANYINFO:
            mxmlElementSetAttr(xmlBusiness, "comment", "企业信息下载接口");
            mxmlElementSetAttr(xmlBusiness, "id", "GGFW_QYXXXZ");
            break;
        default:
            return -1;
            break;
    }
    return 0;
}

int CntaxBuildTyxxXml(int nOpType, char *szYwxxPayload, HDEV hDev, char *pOutBuf, int nBusBufLen)
{
    int nRet = -3, nChildRet = 0;
    mxml_node_t *xmlYwxx = NULL, *xml_Node = NULL, *xmlTyxx = NULL;
    char szBuf[512] = "";
    mxml_node_t *root = mxmlNewXML("1.0");
    if (!root)
        return -1;
    xml_Node = mxmlNewElement(root, "business");
    if (!xml_Node || SetCntaxBusinessAttr(nOpType, xml_Node))
        goto BuildZZSXml_Finish;
    // body
    xml_Node = mxmlNewElement(xml_Node, "body");
    //类型
    xmlTyxx = mxmlNewElement(xml_Node, "tyxx");
    if (!xmlTyxx)
        goto BuildZZSXml_Finish;
    //纳税人识别号
    mxmlNewText(mxmlNewElement(xmlTyxx, "nsrsbh"), 0, hDev->szCommonTaxID);
    //设备编号
    mxmlNewText(mxmlNewElement(xmlTyxx, "sbbh"), 0, hDev->szDeviceID);
    //设备类型 static value
    mxmlNewText(mxmlNewElement(xmlTyxx, "sblx"), 0, "20");
    //软件类型 static value
    mxmlNewText(mxmlNewElement(xmlTyxx, "rjlx"), 0, "01");
    //软件序号 static value
    mxmlNewText(mxmlNewElement(xmlTyxx, "rjxh"), 0, "100000000000");
    switch (nOpType) {
        case TACMD_CNTAX_UPFP: {
            // ip地址,实际为税局反馈的开票软件连接的WanIP
            // 20211231蒙柏开票软件版本IP改为空,端口改为0，hDev->szClientWanIP
            mxmlNewElement(xmlTyxx, "ip");
            // MAC地址 'wmic nicconfig get macaddress,IPAddress'
            GetNicMAC(hDev->szCommonTaxID, szBuf, 2);
            mxmlNewText(mxmlNewElement(xmlTyxx, "mac"), 0, szBuf);
            //总局发布版本号
            mxmlNewText(mxmlNewElement(xmlTyxx, "rjbbh"), 0, defTaxAuthorityVersionCntax);
            //编码表版本号
            mxmlNewText(mxmlNewElement(xmlTyxx, "bmbbh"), 0, DEF_BMBBBH_DEFAULT);
            //主板序列号 'wmic Baseboard get serialnumber' example:PC1H0P9H
            memset(szBuf, 0, sizeof(szBuf));
            GetMotherBoardId(hDev->szCommonTaxID, szBuf);
            mxmlNewText(mxmlNewElement(xmlTyxx, "zbxlh"), 0, szBuf);
            // 远程连接端口，应该是与税局握手时给的TCP连接本地端口，组件直接填的静态值 0x802d
            //  20211231蒙柏开票软件版本IP改为空,端口改为0，hDev->szClientWanPort)
            mxmlNewText(mxmlNewElement(xmlTyxx, "ydk"), 0, "0");
            // CPUID 'wmic CPU get processorID' example:0F8BFBFF000906ED
            GetRandom(szBuf, 16, 16, hDev->szCommonTaxID);
            mxmlNewText(mxmlNewElement(xmlTyxx, "cpuid"), 0, szBuf);
            break;
        }
        default:
            // no do
            break;
    }
    //加载用户传入的ywxx <ywxx>...</ywxx> <ywxxs>...</ywxxs>
    if ((xmlYwxx = mxmlLoadString(NULL, szYwxxPayload, MXML_OPAQUE_CALLBACK)) == NULL) {
        _WriteLog(LL_WARN, "Load user's ywxx failed");
        goto BuildZZSXml_Finish;
    }
    mxmlAdd(xml_Node, MXML_ADD_AFTER, xmlTyxx, xmlYwxx);
    // save
    char *ptr = mxmlSaveAllocString(root, MXML_NO_CALLBACK);
    U2G_UTF8Ignore(ptr, strlen(ptr), pOutBuf, nBusBufLen);
    free(ptr);
    if (nChildRet < 0)
        goto BuildZZSXml_Finish;
    nRet = 0;
BuildZZSXml_Finish:
    mxmlDelete(root);
    return nRet;
}

int CntaxTechnologyXML(int nOpType, HDEV hDev, char *pc_xml)
{
    mxml_node_t *xml_root = NULL, *xml_Node = NULL;
    char szBuff[128] = "";
    char szBuff2[64] = "";
    char szServiceID[32] = "";
    switch (nOpType) {
        default:
            strcpy(szServiceID, "skxt2.0");
            break;
    }
    mxml_node_t *root = mxmlNewXML("1.0U");
    xml_root = mxmlNewElement(root, "tripTechnologyPackage");
    if (!xml_root) {
        mxmlDelete(root);
        return -1;
    }
    mxmlElementSetAttr(xml_root, "version", "1.0");
    xml_Node = mxmlNewElement(xml_root, "identity");
    mxmlNewText(mxmlNewElement(xml_Node, "applicationId"), 0, "ggfwpt_001");
    mxmlNewText(mxmlNewElement(xml_Node, "serviceId"), 0, szServiceID);
    mxmlNewText(mxmlNewElement(xml_Node, "nsrsbh"), 0, hDev->szCommonTaxID);
    mxmlNewElement(xml_Node, "password");
    mxmlNewText(mxmlNewElement(xml_Node, "senderName"), 0, hDev->szCommonTaxID);
    GetOSTime(szBuff2);
    TranslateStandTime(1, szBuff2, szBuff);
    mxmlNewText(mxmlNewElement(xml_Node, "sendTime"), 0, szBuff);
    GetRandom(szBuff2, 10, 10, NULL);
    sprintf(szBuff, "%s%s", hDev->szCommonTaxID, szBuff2);
    mxmlNewText(mxmlNewElement(xml_Node, "globalBusinessId"), 0, szBuff);
    mxmlNewText(mxmlNewElement(xml_Node, "command"), 0, "SEND");
    char *ptr = mxmlSaveAllocString(root, MXML_NO_CALLBACK);  // MXML_NO_CALLBACK/XML_Formater
    strcpy(pc_xml, ptr);
    free(ptr);
    mxmlDelete(root);
    return 0;
}