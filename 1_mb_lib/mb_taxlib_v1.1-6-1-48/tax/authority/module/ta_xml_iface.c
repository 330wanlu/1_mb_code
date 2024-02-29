/*****************************************************************************
File name:   ta_xml_func.c
Description: 用于税局端ta_xml分流
             主要功能是实现xml接口xml构建实现；
             实现xml业务具体流程实现，各税盘不同的switch综合处理方式；
Author:      Zako
Version:     1.0
Date:        2021.08
History:
20210808     最初代码实现及他处代码移入
*****************************************************************************/
#include "ta_xml_iface.h"

//第二层XML,构建电子发票额外包裹xml
int BuildXML_DigitalEnvelope(uint8 bDeviceType, char *szDataPayload, char *pOutBuf)
{
    int nRet = ERR_GENERIC;
    void *pBase64Buff = NULL;
    mxml_node_t *root = NULL, *xml_Node = NULL, *xml_Node2 = NULL;
    int nB64BufLen = strlen(szDataPayload) * 3 + 10240;
    do {
        if (!(pBase64Buff = calloc(1, nB64BufLen)))
            break;
        if (UTF8ToGBKBase64(szDataPayload, strlen(szDataPayload), pBase64Buff) < 0)
            break;
        if (!(root = mxmlNewXML("1.0")))
            break;
        if (!(xml_Node = mxmlNewElement(root, "root")))
            break;
        if (DEVICE_AISINO == bDeviceType)
            mxmlElementSetAttr(xml_Node, "id", "hx");
        else
            mxmlElementSetAttr(xml_Node, "id", "");  // NISEC
        xml_Node2 = mxmlNewElement(xml_Node, "digitalEnvelope");
        mxmlNewText(xml_Node2, 0, "0");
        xml_Node2 = mxmlNewElement(xml_Node, "zip");
        mxmlNewText(xml_Node2, 0, "0");
        xml_Node2 = mxmlNewElement(xml_Node, "context");
        mxmlNewText(xml_Node2, 0, pBase64Buff);
        // get string
        char *ptr = mxmlSaveAllocString(root, MXML_NO_CALLBACK);
        int nStrLen = strlen(ptr);
        memcpy(pOutBuf, ptr, nStrLen);
        free(ptr);
        nRet = RET_SUCCESS;
        //段尾调整
        if (pOutBuf[nStrLen - 1] == '\n')
            pOutBuf[nStrLen - 1] = '\0';
    } while (false);
    if (pBase64Buff)
        free(pBase64Buff);
    if (root)
        mxmlDelete(root);
    return nRet;
}

// szBussines为承载内容
int BuildInterfaceXML(uint8 bDeviceType, char *szTecBuf, char *szBussines, char **pOut)
{
    int nRet = ERR_GENERIC;
    mxml_node_t *root = NULL, *xml_root = NULL, *xml_Node = NULL, *xml_Node2 = NULL;
    char *pB64Tec = NULL, *pB64Bus = NULL;
    do {
        // Calc SHA1 digest
        char szDigest[64] = "";
        if (PreMessageDigest(bDeviceType, szTecBuf, szBussines, szDigest) < 0) {
            nRet = ERR_BUF_CONVERT;
            break;
        }
        // build interface xml
        pB64Tec = (char *)calloc(1, strlen(szTecBuf) * 2 + 512);
        pB64Bus = (char *)calloc(1, strlen(szBussines) * 2 + 512);
        if (!pB64Tec || !pB64Bus) {
            nRet = ERR_BUF_ALLOC;
            break;
        }
        if (UTF8ToGBKBase64(szTecBuf, strlen(szTecBuf), pB64Tec) < 0) {
            nRet = ERR_BUF_CONVERT;
            break;
        }
        if (UTF8ToGBKBase64(szBussines, strlen(szBussines), pB64Bus) < 0) {
            nRet = ERR_BUF_CONVERT;
            break;
        }
        if (!(root = mxmlNewXML("1.0U")))
            break;
        xml_root = mxmlNewElement(root, "tripPackage");
        mxmlElementSetAttr(xml_root, "version", "1.0");
        if (DEVICE_AISINO == bDeviceType)
            mxmlElementSetAttr(xml_root, "ssl", "TA");
        else
            mxmlElementSetAttr(xml_root, "ssl", "OA");  // NISEC CNTAX
        if (!(xml_Node = mxmlNewElement(xml_root, "technology")))
            break;
        mxmlNewText(xml_Node, 0, pB64Tec);
        if (!(xml_Node = mxmlNewElement(xml_root, "businessContent")))
            break;
        mxmlNewText(xml_Node, 0, pB64Bus);
        if (!(xml_Node = mxmlNewElement(xml_root, "signature")))
            break;
        if (!(xml_Node2 = mxmlNewElement(xml_Node, "messageDigest")))
            break;
        mxmlNewText(xml_Node2, 0, szDigest);
        *pOut = mxmlSaveAllocString(root, MXML_NO_CALLBACK);  // MXML_NO_CALLBACK/XML_Formater
        nRet = RET_SUCCESS;
    } while (false);
    if (pB64Bus)
        free(pB64Bus);
    if (pB64Tec)
        free(pB64Tec);
    if (root)
        mxmlDelete(root);
    return nRet;
}

//返回值>0 正确找到；=0不处理；<0错误
int BusinessContentChildPreDo(int nOpType, char *pKey, char *pValue, char *szResult,
                              int nResultSize,char *errinfo)
{
    int nRet = 0;  //默认都对，只有错误的才返回<0
    switch (nOpType) {
        case TACMD_AISINO_UPFP_DZ: {
            if (strcasecmp(pKey, "slxlh"))
                break;
            strcpy(szResult, pValue);
            nRet = 1;
            break;
        }
        case TACMD_AISINO_QUERY_FPUP_DZ: {
            if (strcasecmp(pKey, "mxjgmw"))
                break;
            if (GBKBase64ToUTF8_NP(pValue, szResult, nResultSize) < 0) {
                nRet = -1;
                break;
            }
            nRet = 1;
            break;
        }
        case TACMD_NISEC_QUERY_FPUP_ZZS:
        case TACMD_NISEC_QUERY_FPUP_DZ: {  //只测试了电票，专普票应该一样
            //错误签名返回XML
            //
            //<business id="FPMXJG" comment="发票明细结果"><body count="1" skph="539912237528"
            // nsrsbh="91320118MA1X7GFL8Q" fplx_dm="026"><group xh="1"><data name="fplx_dm"
            // value="026"/><data name="returnCode" value="00"/><data name="returnMessage"
            // value="请求成功"/><data name="mxjgmw" value="...."/><data name="slxlh"
            // value=""/><data name="yqjg" value=""/><data name="yqzt" value=""/><data name="jgmwsm"
            // value="032001700311002071212005#签名验证错误¤"/><data name="hxcwdm" value=""/><data
            // name="ddsj" value=""/><data name="hycwdm" value=""/></group></body></business>
            //
            //正确无误返回XML 也有jgmwsm，但是值为空
            if (!strcasecmp(pKey, "mxjgmw")) {
                //签名错误时也含有该值
                if (strlen(pValue) > nResultSize) {
                    nRet = -1;
                    break;
                }
                strcpy(szResult, pValue);
                nRet = 1;
            }
            if (!strcasecmp(pKey, "jgmwsm")) {
                //如果存在该键值且不为空，那么签名肯定有问题，将返回的mxjgmw清空
                //这样可以防止反写发票，待调试找出问题重新签名发票再上传
                if (!strlen(pValue)) {
                    nRet = 0;  //返回值不能是-1，否则税控盘离线发票上传一直不能成功
                    break;
                }
                strcpy(szResult, "");
                _WriteLog(LL_WARN,
                          "Upload nisec invoice finish, but it return failed, upload "
                          "terminate,server-msg:[%s]",
                          pValue);
				if (errinfo != NULL)
				{
					EncordingConvert("utf-8", "gbk", pValue, strlen(pValue), errinfo, 1024);
					_WriteLog(LL_WARN,"Upload nisec invoice finish, but it return failed, upload terminate,server-msg:[%s]",errinfo);
				}
                strcpy(szResult, pValue);
                nRet = 1;
            }
            break;
        }
        case TACMD_AISINO_REPORT_DZ: {
            if (strcasecmp(pKey, "returnMessage"))
                break;
            strcpy(szResult, pValue);
            nRet = 1;
            break;
        }
        case TACMD_NISEC_CLEAR_DZ: {
            //<business id="JKHCQQ" comment="xxx"><body count="1" skph="539912237528"
            // nsrsbh="91320118MA1X7GFL8Q"><group xh="1"><data name="fplx_dm" value="026"/><data
            // name="fpjkmw" value="4CD78E39"/><data name="szjkmw" value="A3780"/><data
            // name="returnCode" value="00"/><data name="returnMessage"
            // value="??"/></group></body></business>

            //应该是发票密文和时钟密文两段，为了处理方便直接合并
            //最终格式为 fpjkmw|||szjkmw|||
            if (strcasecmp(pKey, "fpjkmw") && strcasecmp(pKey, "szjkmw"))
                break;
            if (strlen(pValue) == 0)
                break;
            strcat(szResult, pValue);
            strcat(szResult, "|||");
            nRet = 1;
            break;
        }
        case TACMD_AISINO_CLEAR_DZ: {
            if (strcasecmp(pKey, "fpjkmw"))
                break;
            strcpy(szResult, pValue);
            nRet = 1;
            break;
        }
        case TACMD_NISEC_UPFP_DZ:
        case TACMD_NISEC_REPORT_DZ: {
            if (strcasecmp(pKey, "returnCode"))
                break;
            if (!strcmp(pValue, "00"))
                strcpy(szResult, "00");
            nRet = 1;
            break;
        }
    }
    return nRet;
}

//！！！注意下，原返回值蒙柏改为gbk，考虑代码一致性，麻烦更改下，返回内容统一为UTF8
bool RedFormUploadResult(int nOpType, mxml_node_t *nodeData, char *pOut)
{
    bool bRet = false;
    char szBuf[256] = "";
    char StatusMC[1024] = "";

    do {
        if (nOpType == TACMD_AISINO_REDFORM_UPLOAD_ZZS ||
            nOpType == TACMD_NISEC_REDFORM_UPLOAD_ZZS) {
            // <FPXT><OUTPUT><CODE>0000</CODE><MESS>操作成功</MESS><DATA><RedInvReqBill>
            // <ReqBillNo>661913412972200409205804</ReqBillNo>
            // <ResBillNo></ResBillNo>
            // <YDDK></YDDK>
            // <DKBDBS></DKBDBS>
            // <StatusDM>TZD0100</StatusDM>
            // <StatusMC>红字信息表金额(税额)大于原蓝票可开具红字信息表的金额(税额)</StatusMC>
            //             </RedInvReqBill></DATA></OUTPUT> </FPXT>
            //
            //  <RedInvReqBill>
            //     <ReqBillNo>539912237528201201152650</ReqBillNo>
            //     <ResBillNo>3201252012000783</ResBillNo>
            //     <YDDK></YDDK>
            //     <DKBDBS></DKBDBS>
            //     <StatusDM>TZD0000</StatusDM>
            //     <StatusMC>审核通过</StatusMC>
            // </RedInvReqBill>
            //红票申请状态名称
            if (GetChildNodeValue(nodeData, "StatusMC", StatusMC) < 0)
                break;
            //红票申请状态代码
            if (GetChildNodeValue(nodeData, "StatusDM", szBuf) < 0)
                break;
            if (!strcmp(szBuf, "TZD0000")) {
                //红票通知单编号
                memset(szBuf, 0, sizeof(szBuf));
                if (GetChildNodeValue(nodeData, "ResBillNo", szBuf) < 0)
                    break;
                strcpy(pOut, szBuf);
                bRet = true;
            }
        } else {  // cntax TACMD_CNTAX_REDFORM_UPLOAD
                  //<body><tyxx>...</tyxx><ywxxs><ywxx>...
            //<xxbbh>3201122111051358</xxbbh><sqdbh>917004614901211126222816</sqdbh><xxbztdm>TZD0000</xxbztdm><xxbztms>审核通过</xxbztms>
            //</ywxx></ywxxs></body>
            //红票申请状态名称
            if (GetChildNodeValue(nodeData, "xxbztms", StatusMC) < 0) {
                _WriteLog(LL_INFO, "获取信息表状态描述失败");
                break;
            }
            //红票申请状态代码
            if (GetChildNodeValue(nodeData, "xxbztdm", szBuf) < 0) {
                _WriteLog(LL_INFO, "获取信息表状态代码失败");
                break;
            }
            if (!strcmp(szBuf, "TZD0000")) {
                //红票通知单编号
                memset(szBuf, 0, sizeof(szBuf));
                if (GetChildNodeValue(nodeData, "xxbbh", szBuf) < 0) {
                    _WriteLog(LL_INFO, "获取信息表变化失败");
                    break;
                }
                strcpy(pOut, szBuf);
                bRet = true;
            }
        }
    } while (false);

    if (!bRet) {
        //如果出错，统一将错误信息拷贝进去
        strcpy(pOut, StatusMC);
    }
    return bRet;
}

//响应中简单的进行预处理,避免调用者再解析XML
int ZZSResultPreDo(int nOpType, mxml_node_t *pXMLRoot, mxml_node_t *nodeData, char *pOut,
                   int nBufSize, char *pXML,char *errinfo)
{
    mxml_node_t *node1 = NULL, *node2 = NULL;
    bool bDetectOK = false;
    char *pPayload = NULL;
    char *pPayloadError = NULL;
    char szBuf1[256];
    switch (nOpType) {
        case TACMD_AISINO_REPORT_ZZS:
        case TACMD_NISEC_REPORT_ZZS: {
            //<FPXT><OUTPUT><CODE>0000</CODE><MESS>成功</MESS><DATA><NSRSBH>91320116MA1MKN3U59</NSRSBH><KPJH>0</KPJH><SBBH>661913412972</SBBH>
            //<HASH>3201002519202280</HASH><CSSJ>202006010000</CSSJ></DATA></OUTPUT></FPXT>
            bDetectOK = FindXMLNodeByKey(pXMLRoot, nodeData, "CSSJ", 10, pOut);
            break;
        }
        case TACMD_AISINO_CLEAR_ZZS:
        case TACMD_NISEC_CLEAR_ZZS: {
            //<FPXT><OUTPUT><CODE>0000</CODE><MESS>成功</MESS><DATA><NSRSBH>91320118MA1X7GFL8Q</NSRSBH><KPJH>0</KPJH><SBBH>539912237528</SBBH><HASH>91320118MA1X7GFL8Q</HASH><CSSJ>202010010000</CSSJ><QKXX>4CD...80</QKXX></DATA></OUTPUT></FPXT>
            bDetectOK = FindXMLNodeByKey(pXMLRoot, nodeData, "QKXX", 64, pOut);
            break;
        }
        case TACMD_AISINO_UPFP_ZZS:
        case TACMD_NISEC_UPFP_ZZS: {  //受理序列号
            bDetectOK = FindXMLNodeByKey(pXMLRoot, nodeData, "SLXLH", 16, pOut);
            break;
        }
        case TACMD_CNTAX_UPFP: {  //受理流水号
            //<?xml version="1.0" encoding="gbk" standalone="yes"?><business comment="发票上传接口"
            // id="GGFW_FPSC"><body><tyxx><nsrsbh>91320118MA20LU6N3P</nsrsbh><returncode>00</returncode><returnmsg>请求成功</returnmsg><sbbh>537100494968</sbbh></tyxx><ywxx><fplxdm>007</fplxdm><fxyjjb></fxyjjb><sllsh>d63ed75c955f49a0bccd216bf20f2cbd</sllsh></ywxx></body></business>
            bDetectOK = FindXMLNodeByKey(pXMLRoot, nodeData, "sllsh", 16, pOut);
            break;
        }
        case TACMD_CNTAX_REPORT: {
            //<business comment="发票汇总信息上传接口" id="GGFW_FPHZXXSC"
            // jkbbh="1.0"><body><tyxx><nsrsbh>91320118MA20LU6N3P</nsrsbh><sbbh>537100494968</sbbh><returncode>00</returncode><returnmsg>成功</returnmsg></tyxx><ywxxs><ywxx><fplxdm>007</fplxdm><returncode>00</returncode><returnmsg>成功</returnmsg></ywxx></ywxxs></body></business>
            if (!FindXMLNodeByKey(pXMLRoot, nodeData, "returncode", 2, pOut))
                break;
            if (strcmp(pOut, "00"))
                break;
            bDetectOK = true;
            break;
        }
        case TACMD_CNTAX_CLEAR: {
            //<business comment="监控信息回传接口" id="GGFW_JKXXHC"
            // jkbbh="1.0"><body><tyxx><nsrsbh>91320118MA20LU6N3P</nsrsbh><sbbh>537100494968</sbbh><returncode>00</returncode><returnmsg>成功</returnmsg></tyxx><ywxx><fpjkmw>2C18..4E73</fpjkmw></ywxx></body></business>
            bDetectOK = FindXMLNodeByKey(pXMLRoot, nodeData, "fpjkmw", 64, pOut);
            break;
        }
        case TACMD_CNTAX_QUERY_UPFP: {
            //对返回值不判断，有这个key就行；由应用层判断值是否密文有效，如果为空服务器可能正在处理中需要重试
            char jgmwsm[1024] = {0};
            FindXMLNodeByKey(pXMLRoot, nodeData, "jgmwsm", 0, jgmwsm);
            if (strlen(jgmwsm) != 0) {
                _WriteLog(LL_WARN, "判断到发票上传后提示验签失败 jgmwsm = %s", jgmwsm);
				if (errinfo != NULL)
				{
					EncordingConvert("utf-8", "gbk", jgmwsm, strlen(jgmwsm), errinfo, 1024);
					_WriteLog(LL_WARN, "Upload nisec invoice finish, but it return failed, upload terminate,server-msg:[%s]", errinfo);
				}
                bDetectOK = false;
                break;
            }
            bDetectOK = FindXMLNodeByKey(pXMLRoot, nodeData, "mxjgmw", 16, pOut);
            if (!bDetectOK) {
                //第一次提交
                pOut[0] = '\0';
                bDetectOK = true;
                break;
            }
            //第二次查询
            break;
        }
        case TACMD_AISINO_REDFORM_DOWNLOAD_ZZS:
        case TACMD_NISEC_REDFORM_DOWNLOAD_ZZS:
        case TACMD_CNTAX_REDFORM_DOWNLOAD: {
            // aisino nisec
            //<FPXT>/<OUTPUT>/<CODE>0000</CODE>/<MESS>处理成功</MESS>/<DATA>/<ALLCOUNT>N</ALLCOUNT>
            //<RedInvReqBill>...<RedInvReqBillMx><GoodsMx>...</GoodsMx></RedInvReqBillMx></RedInvReqBill>
            //<RedInvReqBill>...<RedInvReqBillMx><GoodsMx>...</GoodsMx></RedInvReqBillMx></RedInvReqBill>
            //</DATA>/</OUTPUT>/</FPXT>
            //
            // cntax
            //<body>/<tyxx>...</tyxx><ywxxs><fhtjjls>1</fhtjjls><ywxx>
            //<sqdbh>917004614901211126202026</sqdbh><xxbbh>3201122111051336</xxbbh><xxbztdm>TZD0000</xxbztdm><xxbztms>审核通过</xxbztms>
            //...</ywxx></ywxxs></body>
            if (strlen(pXML) >= nBufSize) {
                strcpy(pOut, "Query return need a huge buffer, please narrow query "
                             "scope or increase buffer!");
                _WriteLog(LL_FATAL, "Query return need a huge buffer, please narrow query "
                                    "scope or increase buffer!");
                break;
            }
            strcpy(pOut, pXML);
            bDetectOK = true;
            break;
        }
        case TACMD_AISINO_REDFORM_UPLOAD_ZZS:
        case TACMD_NISEC_REDFORM_UPLOAD_ZZS:
        case TACMD_CNTAX_REDFORM_UPLOAD: {
            bDetectOK = RedFormUploadResult(nOpType, nodeData, pOut);
			if (bDetectOK == false)
			{
				EncordingConvert("utf-8", "gbk", pOut, strlen(pOut), errinfo, 1024);
			}
            break;
        }
        case TACMD_AISINO_UPFP_DZ:
        case TACMD_AISINO_QUERY_FPUP_DZ:
        case TACMD_AISINO_QUERY_FPUP_ZZS: {
            if ((node1 = mxmlFindElement(nodeData, pXMLRoot, "FP_SUCC", NULL, NULL,
                                         MXML_DESCEND)) == NULL)
                break;
            if ((node2 = mxmlFindElement(node1, pXMLRoot, "FLAG", NULL, NULL, MXML_DESCEND)) ==
                NULL)
                break;
            if ((pPayload = (char *)mxmlGetOpaque(node2)) == NULL) {
                //无结果，错了，查询错误
                if ((node1 = mxmlFindElement(nodeData, pXMLRoot, "FP_ERR", NULL, NULL,
                                             MXML_DESCEND)) == NULL)
                    break;
                if ((node2 = mxmlFindElement(node1, pXMLRoot, "FP", NULL, NULL, MXML_DESCEND)) ==
                    NULL)
                    break;
                if ((node1 = mxmlFindElement(node2, pXMLRoot, "MESS", NULL, NULL, MXML_DESCEND)) ==
                    NULL)
                    break;
                if ((pPayloadError = (char *)mxmlGetOpaque(node1)) == NULL)
                    break;               
				if (errinfo != NULL)
				{
					EncordingConvert("utf-8", "gbk", pPayloadError, strlen(pPayloadError), errinfo, 1024);
					_WriteLog(LL_FATAL, "TACMD_AISINO_QUERY_FPUP_ZZS Upload invoice failed, retmsg:%s",
						errinfo);
				}
                break;
            }
            if (strlen(pPayload) > 128)
                break;
            strcpy(pOut, pPayload);
            bDetectOK = true;
            break;
        }
        case TACMD_AISINO_DOWNLOAD_NETINV_READY: {
            // <FPXT><OUTPUT><CODE>0000</CODE><MESS>下载解锁成功</MESS>
            // <DATA><NSRSBH>91320116MA1MKN3U59</NSRSBH><KPJH>0</KPJH><SBBH>661913412972</SBBH><FPXZ><FPZL>0</FPZL><FPJMW>XXXX</FPJMW></FPXZ>
            // </DATA></OUTPUT></FPXT>
            if (GetChildNodeValue(pXMLRoot, "FPZL", szBuf1) < 0)
                break;
            if (!GetChildNodeValue(pXMLRoot, "FPJMW", pOut) ||
                !GetChildNodeValue(pXMLRoot, "FPJJSMW", pOut))
                bDetectOK = true;  // FPJMW\FPJJSMW 读取到任意一个都行
            break;
        }
        case TACMD_CNTAX_NETINV_WRITE: {
            //<body><tyxx>...</tyxx><ywxxs><ywxx><returncode>00</returncode><returnmsg>操作成功</returnmsg><fplxdm>026</fplxdm>
            //<lpxxmw>CB7880A..6EEB</lpxxmw><lpxxzs>032002100211#66636707#66636707</lpxxzs></ywxx></ywxxs></body>
            if (!GetChildNodeValue(pXMLRoot, "lpxxmw", pOut) && strlen(pOut) > 100)
                bDetectOK = true;
            break;
        }
    }
    return bDetectOK;
}