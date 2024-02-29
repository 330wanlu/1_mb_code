/*****************************************************************************
File name:   ta_xml.c
Description: ����˰�ֵĽ����;���ҵ����,��ҪΪXML�߼�����swtich����
Author:      Zako
Version:     1.0
Date:        2019.09
History:
20200718     ����ģ��,����˰�ֵĽ����;���ҵ����ȫ�������ڴ�
20210808     Դ����󣬷�����ҵ��xml������xmlʵ�ֹ���������������ֻ�������̵���
*****************************************************************************/
#include "ta_xml.h"


// begin----------------------------------ҵ��㹲ͬ����------------------------------------------------------

int FetchBusinessContent(int nOpType, mxml_node_t *pXMLRoot, char *pDecPayload, int nDecPayloadLen, char *errinfo)
{
    int nRet = -1;
    mxml_node_t *node = NULL, *node2 = NULL;
    char *pPayLoad = NULL;

    if ((node = mxmlFindElement(pXMLRoot, pXMLRoot, "ZZSFPXT", NULL, NULL, MXML_DESCEND)) == NULL)
        return -5;
    if ((node2 = mxmlFindElement(node, pXMLRoot, "responseCode", NULL, NULL, MXML_DESCEND)) == NULL)
        return -6;
    if ((pPayLoad = (char *)mxmlGetOpaque(node2)) == NULL)
        return -7;
    int nRequestStatus = strtol(pPayLoad, NULL, 16);
    memset(pDecPayload, 0, nDecPayloadLen);
    if (nRequestStatus) {  //û����ȷ���أ�������Ϣ����
        if ((node2 = mxmlFindElement(node, pXMLRoot, "responseMessage", NULL, NULL,
                                     MXML_DESCEND)) == NULL)
            return -8;
        node = mxmlGetLastChild(node2);
        if ((pPayLoad = (char *)mxmlGetOpaque(node)) == NULL)
            return -9;
        strcpy(pDecPayload, pPayLoad);
        //_WriteLog(LL_WARN, "ZZSFP response failed,ta rep-message:+++[%s]+++", pPayLoad);
		printf("ZZSFP response failed,ta rep-message:+++[%s]+++\n", pPayLoad);
		EncordingConvert("utf-8", "gbk", pPayLoad, strlen(pPayLoad), errinfo, 1024);
		_WriteLog(LL_WARN, "ZZSFP response failed,ta rep-message:+++[%s]+++", errinfo);
        nRet = -1;
    } else  //��ȷ����,����xml
        nRet = FilterXMLResult(nOpType, pXMLRoot, pDecPayload, nDecPayloadLen);
    return nRet;
}

int FetchBusinessContent_DZ(int nOpType, mxml_node_t *pXMLRoot, char *pDecPayload,
                            int nDecPayloadLen,char *errinfo)
{
    int nRet = -1;
    mxml_node_t *node1 = NULL, *node2 = NULL;
    char *pPayload = NULL;
    int nRequestStatus = -1;
    char szReturnMsg[768] = "";
    //������ڲ��Ե�����output�ڵ�,��������������,�����º������кϲ�����,���������ж�
    while (1) {
        if ((node1 = mxmlFindElement(pXMLRoot, pXMLRoot, "business", NULL, NULL, MXML_DESCEND)) ==
            NULL)
            break;
        if ((node1 = mxmlFindElement(node1, pXMLRoot, "body", NULL, NULL, MXML_DESCEND)) == NULL) {
            nRet = -2;
            break;
        }
        if ((node2 = mxmlFindElement(node1, pXMLRoot, "output", NULL, NULL, MXML_DESCEND)) !=
            NULL) {  //�������ʱ����output�ֶ�,body��group֮���и�output�ֶ�
            // <business id="RESPONES" comment=""
            // serverName="hyclserver4����ʱ�䣺20191009100455"> <body>
            //     <output>
            //         <group>
            //             <returncode>91000000</returncode>
            //             <returnmsg>��Ʊ�Ѱ�ʧ�ܣ����յ��ı��ķǷ���</returnmsg>
            //         </group>
            //     </output>
            // </body>
            // </business>
            if ((node2 = mxmlFindElement(node2, pXMLRoot, "group", NULL, NULL, MXML_DESCEND)) ==
                NULL) {
                nRet = -3;
                break;
            }
            if ((node1 = mxmlFindElement(node2, pXMLRoot, "returncode", NULL, NULL,
                                         MXML_DESCEND)) == NULL) {
                nRet = -4;
                break;
            }
            if ((pPayload = (char *)mxmlGetOpaque(node1)) == NULL) {
                nRet = -5;
                break;
            }
            nRequestStatus = atoi(pPayload);
        } else {  //��ȷʱ��body��ֱ�Ӿ���group�ֶ�
                  // <business id="HX_FPMXSC" comment="��Ʊ��ϸ�ϴ�"
                  // serverName="hyclserver4����ʱ�䣺20191009 145849">
                  //     <body count="1" skph="661814461322" nsrsbh="92320105MA1WMUAP8T">
                  //         <group xh="1">
                  //             <data name="fplx_dm" value="026" />
                  //             <data name="returnCode" value="00" />
                  //             <data name="returnMessage" value="�ɹ�" />
                  //             <data name="slxlh"
                  //             value="f69b6411-5c00-445d-9701-3933adb896e220191009025849" />
                  //         </group>
                  //     </body>
                  // </business>
                  // slxlh,mxjgmw �ϴ������ظ�����һ��

            if ((node2 = mxmlFindElement(node1, pXMLRoot, "group", NULL, NULL, MXML_DESCEND)) ==
                NULL) {
                nRet = -6;
                break;
            }
            if ((node1 = mxmlGetFirstChild(node2)) == NULL) {
                nRet = -7;
                break;
            }
            int i = 0;
            char *pValue = NULL;
            char szResult[512] = "";
            for (i = 0; i < 10; i++) {
                if ((node1 = mxmlGetNextSibling(node1)) == NULL)
                    break;
                if ((pPayload = (char *)mxmlElementGetAttr(node1, "name")) == NULL)
                    break;
                if ((pValue = (char *)mxmlElementGetAttr(node1, "value")) == NULL)
                    break;
                // return code������Ŀ�����棬�϶��Ⱥ������Ŀ��ö�ٵ�
                if (!strcasecmp(pPayload, "returnCode"))
                    nRequestStatus = (int)strtol(pValue, NULL, 16);
                if ((nRet = BusinessContentChildPreDo(nOpType, pPayload, pValue, szResult,
                                              sizeof szResult,errinfo)) < 0) {
                    _WriteLog(LL_INFO, "BusinessContentChildPreDo failed nRet = %d", nRet);
                    snprintf(szReturnMsg, sizeof(szReturnMsg), "%s", szResult);
                    nRequestStatus = -1;
                    break;
                }
                // printf log
                if (!strcasecmp(pPayload, "returnMessage"))	{
					snprintf(szReturnMsg, sizeof(szReturnMsg), "%s", pValue);
					if (errinfo != NULL) {
						EncordingConvert("utf-8", "gbk", pValue, strlen(pValue), errinfo, 1024);
					}
				}
            }
            memset(pDecPayload, 0, nDecPayloadLen);
            strcpy(pDecPayload, szResult);  // szResult�п���Ϊ��
            // finish
        }
        if (0 != nRequestStatus) {
            _WriteLog(LL_WARN, "DZFP response failed,ta rep-message:+++[%s]+++", szReturnMsg);
            strcpy(pDecPayload, szReturnMsg);
            nRet = -10;
            break;
        }
        nRet = 0;
        break;
    }
    return nRet;
}

//ĳЩ����������xml��Ӧ��ͳһ����
int FetchBusinessContent_Special(int nOpType, mxml_node_t *pXMLRoot, char *pDecPayload,char *errinfo)
{
    int nRet = -1;
    int nRequestStatus = -1;
    char szBuf[512] = "";
    //������ڲ��Ե�����output�ڵ�,��������������,�����º������кϲ�����,���������ж�
    while (1) {
        if (GetChildNodeValue(pXMLRoot, "returncode", szBuf) < 0) {
            nRet = -2;
            break;
        }
        nRequestStatus = (int)strtol(szBuf, NULL, 16);
        memset(szBuf, 0, sizeof(szBuf));
        if (GetChildNodeValue(pXMLRoot, "returnmsg", szBuf) < 0) {
            nRet = -2;
            break;
        }
        //����ӿ�xml�ȽϹ淶���Ȱ�������־��¼���������ִ�гɹ�����ô�µĽ������ˢ��˰�ָ������
        strcpy(pDecPayload, szBuf);
        switch (nOpType) {
            case TACMD_AISINO_TZDBH_CANCEL:
            case TACMD_NISEC_TZDBH_CANCEL: {
                // <business comment="������Ϣ����" id="GGFW_HZXXBCX"><body><tyxx>
                // <nsrsbh>91320116MA1MKN3U59</nsrsbh><sbbh>661913412972</sbbh><sblx>10</sblx><returncode>B80046</returncode><returnmsg>���ݺ�����Ϣ���û�в�ѯ��������Ϣ��</returnmsg>
                // </tyxx></body></business></businessContent>

                // <business comment="������Ϣ����" id="GGFW_HZXXBCX"><body><tyxx>
                //<nsrsbh>91320118MA1X7GFL8Q</nsrsbh><sbbh>539912237528</sbbh><sblx>00</sblx><returncode>00</returncode><returnmsg>������Ϣ�������</returnmsg>
                //</tyxx></body></business>
                if (!nRequestStatus)
                    nRet = 0;
				else	{
					if (errinfo != NULL)
						EncordingConvert("utf-8", "gbk", pDecPayload, strlen(pDecPayload), errinfo, 1024);
				}
                break;
            }
            case TACMD_NISEC_PUBSERV_JOINCANCEL:
            case TACMD_AISINO_PUBSERV_JOINCANCEL: {
                //<business comment="���ӷ�Ʊ����ƽ̨����" id="GGFW_DZFPFWPTSQ"
                // jkbbh="1.0"><body><tyxx><returncode>00</returncode><returnmsg>�ɹ�</returnmsg><nsrsbh>92320105MA1WMUAP8T</nsrsbh><sbbh>661814461322</sbbh></tyxx><ywxx><fplxdm>026</fplxdm><xpmw>.....</xpmw></ywxx></body></business>
                if (GetChildNodeValue(pXMLRoot, "xpmw", szBuf) < 0)
                    break;
                // nisec��250+����
                if (strlen(szBuf) < 24 || strlen(szBuf) > 512)
                    break;
                strcpy(pDecPayload, szBuf);
                nRet = 0;
                break;
            }
            default:
                break;
        }
        break;
    }
    return nRet;
}

int FetchYwxx(int nOpType, mxml_node_t *pXMLRoot, char *pDecPayload, int nDecPayloadLen,char *errinfo)
{
    int nRet = -1;
    mxml_node_t *nodeTyxx = NULL, *nodeYwxx = NULL;
    int nRequestStatus = -1;
    char szReturnMsg[2048] = "";
    do {
        //<business comment="."
        // id="."><body><tyxx><nsrsbh>...</nsrsbh><returncode>00</returncode><returnmsg>����ɹ�</returnmsg><sbbh>...</sbbh></tyxx>
        //<ywxx><fplxdm>007</fplxdm><fxyjjb></fxyjjb><sllsh>...</sllsh></ywxx></body></business>
        if (!(nodeTyxx = mxmlFindElement(pXMLRoot, pXMLRoot, "tyxx", NULL, NULL, MXML_DESCEND))) {
            nRet = -2;
            break;
        }
        if (GetChildNodeValue(nodeTyxx, "returncode", szReturnMsg)) {
            nRet = -3;
            break;
        }
        nRequestStatus = (int)strtol(szReturnMsg, NULL, 16);
        if (GetChildNodeValue(nodeTyxx, "returnmsg", szReturnMsg)) {
            nRet = -4;
            break;
        }
        if (nRequestStatus) {
            strcpy(pDecPayload, szReturnMsg);
            _WriteLog(LL_WARN, "Cntax ywxx response failed,ta rep-message:+++[%s]+++", szReturnMsg);
			printf("szReturnMsg %s\n", szReturnMsg);
			if (errinfo != NULL)	{
				EncordingConvert("utf-8", "gbk", szReturnMsg, strlen(szReturnMsg), errinfo, 1024);
			}
			nRet = -5;
            break;
        }
        //������߼�����Ѱ��ywxxs���ҵ���Ѱ��ywxx���������Ѱ��ywxx��������Ѱ�Ҳ���������
        if (!(nodeYwxx = mxmlFindElement(pXMLRoot, pXMLRoot, "ywxxs", NULL, NULL, MXML_DESCEND)) &&
            !(nodeYwxx = mxmlFindElement(pXMLRoot, pXMLRoot, "ywxx", NULL, NULL, MXML_DESCEND))) {
            //��Щ����ֻ�ж�returncodeΪ0���ɣ���ywxx����Ŀ
            // TACMD_CNTAX_REDFORM_CANCEL
            //<body>/<tyxx>/<nsrsbh>...</nsrsbh><sbbh>...</sbbh><sblx>20</sblx><returncode>00</returncode><returnmsg>������Ϣ�������</returnmsg></tyxx></body>
            strcpy(pDecPayload, szReturnMsg);
            nRet = 0;
            break;
        }
        if (!(nRet = ZZSResultPreDo(nOpType, pXMLRoot, nodeYwxx, pDecPayload, nDecPayloadLen,
									pDecPayload, errinfo)))	{
            _WriteLog(LL_WARN, "ZZSResultPreDo failed nRet = %d", nRet);
            nRet = -6;
            break;
        }
        nRet = 0;
    } while (false);
    return nRet;
}

int ReponseHowToDo(int nOpType, mxml_node_t *pXMLRoot2, char *pDecPayload, int nDecPayloadLen,char *errinfo)
{
    int nChildRet = 0;
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
        case TACMD_NISEC_UPFP_ZZS:
        case TACMD_NISEC_REPORT_ZZS:
        case TACMD_NISEC_CLEAR_ZZS:
        case TACMD_NISEC_REDFORM_UPLOAD_ZZS:
        case TACMD_NISEC_REDFORM_DOWNLOAD_ZZS: {
                nChildRet = FetchBusinessContent(nOpType, pXMLRoot2, pDecPayload, nDecPayloadLen,errinfo);
            break;
        }
        case TACMD_AISINO_UPFP_DZ:
        case TACMD_AISINO_QUERY_FPUP_DZ:
        case TACMD_AISINO_REPORT_DZ:
        case TACMD_AISINO_CLEAR_DZ:
        case TACMD_NISEC_QUERY_FPUP_ZZS:
        case TACMD_NISEC_UPFP_DZ:
        case TACMD_NISEC_QUERY_FPUP_DZ:
        case TACMD_NISEC_REPORT_DZ:
        case TACMD_NISEC_CLEAR_DZ:
             nChildRet = FetchBusinessContent_DZ(nOpType, pXMLRoot2, pDecPayload, nDecPayloadLen,errinfo);
            break;
        case TACMD_AISINO_TZDBH_CANCEL:
        case TACMD_NISEC_TZDBH_CANCEL:
        case TACMD_AISINO_PUBSERV_JOINCANCEL:
        case TACMD_NISEC_PUBSERV_JOINCANCEL:
        	 nChildRet = FetchBusinessContent_Special(nOpType, pXMLRoot2, pDecPayload,errinfo);
            break;
        case TACMD_CNTAX_UPFP:
        case TACMD_CNTAX_QUERY_UPFP:
        case TACMD_CNTAX_REPORT:
        case TACMD_CNTAX_CLEAR:
        case TACMD_CNTAX_NETINV_WRITE:
        case TACMD_CNTAX_REDFORM_UPLOAD:
        case TACMD_CNTAX_REDFORM_DOWNLOAD:
        case TACMD_CNTAX_REDFORM_CANCEL:
        case TACMD_CNTAX_NETINV_CONFIRM:
            nChildRet = FetchYwxx(nOpType, pXMLRoot2, pDecPayload, nDecPayloadLen,errinfo);
            break;
        default:
            //����xml,�ɵ����ߴ���
            // TACMD_AISINO_REDFORM_DZCHECK TACMD_NISEC_NETINV_DO
            // TACMD_CNTAX_NETINV_QUERY TACMD_CNTAX_RAWXML
            // TACMD_CNTAX_SYNC_COMPANYINFO(������Ӧ���������ֲ������У���Ҫ���ú�������)
            nChildRet = 0;
            break;
    }
    return nChildRet;
}

//����ֵ������������������pDecPayload������ֵΪ��������ֵΪ�������˷���ֵ��
//��Ҫ�ֶ�freep DecBuff
int GetPayloadFromRep(int nOpType, char *szRep, char **pDecBuff,char *errinfo)
{
    mxml_node_t *node1 = NULL, *node2 = NULL, *node3 = NULL;
    char *pPayLoad = NULL;
    mxml_node_t *pXMLRoot1 = NULL, *pXMLRoot2 = NULL, *pXMLRoot3 = NULL;
    int nRet = -1;
    char *pDecPayload = NULL;
    int nRequestStatus = -1;
    int nDecPayloadLen = strlen(szRep) * 2 + 1024;  //�漰����ת�����˴��ڴ���Ŵ�
    int nChildRet = 0;
    while (1) {
        pDecPayload = calloc(1, nDecPayloadLen);
        if (!pDecPayload)
            break;
        if ((pXMLRoot1 = mxmlLoadString(NULL, szRep, MXML_OPAQUE_CALLBACK)) == NULL) {
            _WriteLog(LL_WARN, "GetPayloadFromRep mxmlLoadString1 failed");
            nRet = -2;
            break;
        }
        if ((node1 = mxmlFindElement(pXMLRoot1, pXMLRoot1, "tripPackage", NULL, NULL,
                                     MXML_DESCEND)) == NULL) {
            nRet = -3;
            break;
        }
        if ((node2 = mxmlFindElement(node1, pXMLRoot1, "technology", NULL, NULL, MXML_DESCEND)) ==
            NULL) {
            nRet = -4;
            break;
        }
        if ((pPayLoad = (char *)mxmlGetOpaque(node2)) == NULL) {
            nRet = -5;
            break;
        }
        if ((nChildRet = GBKBase64ToUTF8_NP(pPayLoad, pDecPayload, nDecPayloadLen)) < 0) {
            _WriteLog(LL_WARN, "GetPayloadFromRep GBKBase64ToUTF8_NP1 failed");
            nRet = -6;
            break;
        }
        if ((pXMLRoot3 = mxmlLoadString(NULL, pDecPayload, MXML_OPAQUE_CALLBACK)) == NULL) {
            _WriteLog(LL_WARN, "GetPayloadFromRep mxmlLoadString2 failed");
            nRet = -7;
            break;
        }
        if ((node2 = mxmlFindElement(pXMLRoot3, pXMLRoot3, "tripTechnologyPackage", NULL, NULL,
                                     MXML_DESCEND)) == NULL) {
            nRet = -8;
            break;
        }
        if ((node3 = mxmlFindElement(node2, pXMLRoot3, "responseCode", NULL, NULL, MXML_DESCEND)) ==
            NULL) {
            nRet = -9;
            break;
        }
        if ((pPayLoad = (char *)mxmlGetOpaque(node3)) == NULL) {
            nRet = -10;
            break;
        }
        nRequestStatus = atoi(pPayLoad);
        if (nRequestStatus < 0) {
            _WriteLog(LL_WARN, "GetPayloadFromRep reponse code:%d", nRequestStatus);
            nRet = -11;
            break;
        }
        if ((node2 = mxmlFindElement(node1, pXMLRoot1, "businessContent", NULL, NULL,
                                     MXML_DESCEND)) == NULL) {
            nRet = -12;
            break;
        }
        if ((pPayLoad = (char *)mxmlGetOpaque(node2)) == NULL) {
            nRet = -13;
            break;
        }
        memset(pDecPayload, 0, nDecPayloadLen);
        if (XmlBase64ToUTF8(pPayLoad, pDecPayload, nDecPayloadLen) < 0) {
            _WriteLog(LL_WARN, "GetPayloadFromRep GBKBase64ToUTF8_NP2 failed");
            nRet = -14;
            break;
        }
        char *pNoNewLine = StrReplace(pDecPayload, "\r\n", "");
        strcpy(pDecPayload, pNoNewLine);
        free(pNoNewLine);
        if ((pXMLRoot2 = mxmlLoadString(NULL, pDecPayload, MXML_OPAQUE_CALLBACK)) == NULL) {
            _WriteLog(LL_WARN, "GetPayloadFromRep mxmlLoadString3 failed");
            nRet = -15;
            break;
        }
        nRet = ReponseHowToDo(nOpType, pXMLRoot2, pDecPayload, nDecPayloadLen,errinfo);
        if (nRet < 0) {
            _WriteLog(LL_FATAL, "GetPayloadFromRep FetchBusinessContent failed, nChildRet:%d",
                      nRet);
            nRet = -16;
            break;
        }
        break;
    }
    *pDecBuff = pDecPayload;
    if (pXMLRoot1)
        mxmlDelete(pXMLRoot1);
    if (pXMLRoot2)
        mxmlDelete(pXMLRoot2);
    if (pXMLRoot3)
        mxmlDelete(pXMLRoot3);
    return nRet;
}

int AnalyzeUploadResult(int nOpType, char *pXML, char *pOut, int nBufSize,char *errinfo)
{
    mxml_node_t *pXMLRoot = NULL, *nodedata = NULL;
    int nRequestStatus = -1;
    int nRet = -1;
    while (1) {
        if ((nRequestStatus = LoadZZSXMLDataNodeTemplate(&pXMLRoot, &nodedata, "DATA", pXML)) < 0) {
            _WriteLog(LL_WARN, "AnalyzeUploadResult mxmlLoadString faild");
			if ((errinfo != NULL) && (pXML != NULL))
				EncordingConvert("utf-8", "gbk", pXML, strlen(pXML), errinfo, 1024);
            break;
        }
		uint8 bDetectOK = ZZSResultPreDo(nOpType, pXMLRoot, nodedata, pOut, nBufSize, pXML, errinfo);
        if (!bDetectOK) {
            _WriteLog(LL_WARN, "AnalyzeUploadResult ZZSResultPreDo faild bDetectOK = %d\n",
                      bDetectOK);
            break;
        }
        nRet = nRequestStatus;
        break;
    }
    if (pXMLRoot)
        mxmlDelete(pXMLRoot);
    return nRet;
}

//�����滻ΪTaNetIo�ӿ�
int CommunityMsg(HHTTP hi, char *szQueryPath, int nOpType, char *szPayload, char *pRepBuf,
                 int nRepBufLen)
{
    if (!hi || !hi->hDev)
        return -1;
    HDEV hDev = hi->hDev;
    char *pInfXML = NULL;
    char szTecBuf[1280] = "";
    int nBusBufLen = strlen(szPayload) * 3 + 1024;  // must more than base64 len
    char *szBusBuf = calloc(1, nBusBufLen);
    if (!szBusBuf)
        return -2;
    int nRet = -3;
    int nChildRet = -1;

    while (1) {
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
                // aisino ��ֵ˰�ӿ�
                nChildRet = AisinoBuildZZSXml(nOpType, szPayload, hDev, szBusBuf);
                break;
            case TACMD_NISEC_UPFP_ZZS:
            case TACMD_NISEC_REPORT_ZZS:
            case TACMD_NISEC_CLEAR_ZZS:
            case TACMD_NISEC_REDFORM_UPLOAD_ZZS:
            case TACMD_NISEC_REDFORM_DOWNLOAD_ZZS:
                // nisec ��ֵ˰�ӿ�
                nChildRet = NisecBuildZZSXml(nOpType, szPayload, hDev, szBusBuf);
                break;
            case TACMD_AISINO_UPFP_DZ:
            case TACMD_AISINO_QUERY_FPUP_DZ:
            case TACMD_AISINO_REPORT_DZ:
            case TACMD_AISINO_CLEAR_DZ:
            case TACMD_AISINO_REDFORM_DZCHECK:
            case TACMD_NISEC_QUERY_FPUP_ZZS:
            case TACMD_NISEC_UPFP_DZ:
            case TACMD_NISEC_QUERY_FPUP_DZ:
            case TACMD_AISINO_TZDBH_CANCEL:
            case TACMD_NISEC_REPORT_DZ:
            case TACMD_NISEC_CLEAR_DZ:
            case TACMD_NISEC_TZDBH_CANCEL:
            case TACMD_AISINO_PUBSERV_JOINCANCEL:
            case TACMD_NISEC_PUBSERV_JOINCANCEL:
            case TACMD_NISEC_NETINV_DO:
            case TACMD_NISEC_SYNC_COMPANYINFO:
                // aisino nisec ���õ��ӷ�Ʊ��װ�ӿ�
                nChildRet = BuildXML_DigitalEnvelope(hDev->bDeviceType, szPayload, szBusBuf);
                break;
            case TACMD_CNTAX_UPFP:
            case TACMD_CNTAX_QUERY_UPFP:
            case TACMD_CNTAX_REPORT:
            case TACMD_CNTAX_CLEAR:
            case TACMD_CNTAX_NETINV_QUERY:
            case TACMD_CNTAX_NETINV_WRITE:
            case TACMD_CNTAX_REDFORM_UPLOAD:
            case TACMD_CNTAX_REDFORM_DOWNLOAD:
            case TACMD_CNTAX_REDFORM_CANCEL:
            case TACMD_CNTAX_NETINV_CONFIRM:
            case TACMD_CNTAX_SYNC_COMPANYINFO:
                // cntax����XML����fpxt����dz����������tyxx��ywxx������ɣ��û����ô���ywxx����
                nChildRet = CntaxBuildTyxxXml(nOpType, szPayload, hDev, szBusBuf, nBusBufLen);
                break;
            default:  //�����xmlֱ����ΪszBusBuf
                      // TACMD_CNTAX_RAWXML TACMD_AISINO_CHECK_UPLOADSERVER
                strcpy(szBusBuf, szPayload);
                nChildRet = 0;
                break;
        }
        if (nChildRet < 0) {
            _WriteLog(LL_WARN, "CommunityMsg BuildZZSXml failed, nChildRet:%d", nChildRet);
            nRet = -4;
            break;
        }
        if (DEVICE_AISINO == hDev->bDeviceType)
            nChildRet = AisinoTechnologyXML(nOpType, hi->hDev, szTecBuf, sizeof(szTecBuf));
        else if (DEVICE_NISEC == hDev->bDeviceType)
            nChildRet = NisecTechnologyXML(nOpType, hi->hDev, szTecBuf);
        else if (DEVICE_CNTAX == hDev->bDeviceType)
            nChildRet = CntaxTechnologyXML(nOpType, hi->hDev, szTecBuf);
        else
            break;
        if (nChildRet < 0)  // szTecBuf
        {
            _WriteLog(LL_WARN, "CommunityMsg TechnologyXML failed, nChildRet:%d", nChildRet);
            nRet = -5;
            break;
        }
        if ((nChildRet = BuildInterfaceXML(hDev->bDeviceType, szTecBuf, szBusBuf, &pInfXML)) < 0) {
            _WriteLog(LL_WARN, "CommunityMsg BuildInterfaceXML failed, nChildRet:%d", nChildRet);
            nRet = -6;
            break;
        }
#ifdef _CONSOLE
        _WriteHexToDebugFile("CommunityMsg-payload-3.txt", (uint8 *)szPayload, strlen(szPayload));
        _WriteHexToDebugFile("CommunityMsg-bussiness-2.2.txt", (uint8 *)szBusBuf, strlen(szBusBuf));
        _WriteHexToDebugFile("CommunityMsg-tech-2.1.txt", (uint8 *)szTecBuf, strlen(szTecBuf));
        _WriteHexToDebugFile("CommunityMsg-interface-1.txt", (uint8 *)pInfXML, strlen(pInfXML));
#endif
        memset(pRepBuf, 0, nRepBufLen);

		//_WriteLog(LL_INFO, "�������ݣ�%s", pInfXML);
        if (ServerIO(hi, szQueryPath, (uint8 *)pInfXML, pRepBuf, nRepBufLen) < 0) {
            nRet = -7;
            break;
        }
		//_WriteLog(LL_INFO, "�������ݣ�%s", pRepBuf);
        _WriteHexToDebugFile("CommunityMsg-server-rep.txt", (uint8 *)pRepBuf, strlen(pRepBuf));
        nRet = 0;
        break;
    }
    if (pInfXML)
        free(pInfXML);
    free(szBusBuf);
    if (nRet < 0)
        _WriteLog(LL_WARN, "CommunityMsg failed ret:%d", nRet);
    return nRet;
}

// pInputAndRepBuf �����������������һ�㶼�з���ֵ�������п���Ϊ���ַ���
int TaNetIo(HHTTP hi, int nOpType, char *pInputAndRepBuf, int nRepBufLen)
{
    int nChildRet = -1;
    int nRet = -1;
    char *pDecRep = NULL;
    while (1) {
        if ((nChildRet = CommunityMsg(hi, DEF_SWZJ_INVOICE_GATEWAY, nOpType, pInputAndRepBuf,
                                      pInputAndRepBuf, nRepBufLen)) < 0) {
            SetLastError(hi->hDev->hUSB, ERR_TA_COMMUNITY_POST_ERROR,
                         "ҵ��������������ʧ��,nChildRet = %d", nChildRet);
            nRet = -2;
            _WriteLog(LL_FATAL, "TaNetIo CommunityMsg failed nChildRet:%d", nChildRet);
            break;
        }
		int nRepStatusCode = GetPayloadFromRep(nOpType, pInputAndRepBuf, &pDecRep, hi->hDev->hUSB->errinfo);
        if (nRepStatusCode) {
            nRet = -3;
            _WriteLog(LL_FATAL, "TaNetIo GetPayloadFromRep failed nChildRet:%d��errinfo [%s]\n",
				nRepStatusCode, hi->hDev->hUSB->errinfo);
            if (pDecRep)
                strcpy(pInputAndRepBuf, pDecRep);
            break;
        }
        //�����е��Ƿ��صļ�XML,ֻ��ȡһ����Ϣ,���ں����м��������ȡ����
        //���ʱ���صĸ���XML,�򷵻�XML,�ɵ��ú����Լ�����
        switch (nOpType) {
            case TACMD_AISINO_REPORT_DZ:  //�ɰط����������ڴ˴������ڲ��죬ԭ��Ϊ����ʧ������
                _WriteLog(LL_DEBUG, "TaNetIo server-echo:[%s]", pDecRep);
                strcpy(pInputAndRepBuf, pDecRep);
                char gbkDecRep[10240] = {0};
                U2G_UTF8Ignore(pDecRep, strlen(pDecRep), gbkDecRep, sizeof(gbkDecRep));
                if ((strcmp(pDecRep, "�ɹ�") != 0) && (strcmp(gbkDecRep, "�ɹ�") != 0)) {
                    printf("[-] ���ӷ�Ʊ�ϱ�����ʧ��,tmp_data = %s", pDecRep);
                    nChildRet = -1;
                    break;
                }
                nChildRet = 0;
                break;
            case TACMD_AISINO_REPORT_ZZS:
            case TACMD_AISINO_CLEAR_ZZS:
            case TACMD_AISINO_REDFORM_UPLOAD_ZZS:
            case TACMD_AISINO_REDFORM_DOWNLOAD_ZZS:
            case TACMD_AISINO_DOWNLOAD_NETINV_READY:
            case TACMD_NISEC_CLEAR_ZZS:
            case TACMD_NISEC_REPORT_ZZS:
            case TACMD_NISEC_REDFORM_UPLOAD_ZZS:
            case TACMD_NISEC_REDFORM_DOWNLOAD_ZZS:
            case TACMD_NISEC_UPFP_ZZS:
            case TACMD_AISINO_UPFP_ZZS:
            case TACMD_AISINO_QUERY_FPUP_ZZS:
            case TACMD_AISINO_QUERY_FPUP_DZ:
				nChildRet = AnalyzeUploadResult(nOpType, pDecRep, pInputAndRepBuf, nRepBufLen, hi->hDev->hUSB->errinfo);
                break;
            default:
                // ����Щδ֪�İ���δ���ݵģ���֪������
                // TACMD_AISINO_SYNC_COMPANYINFO
                // TACMD_NISEC_UPFP_DZ
                // TACMD_AISINO_UPFP_DZ
                // TACMD_AISINO_QUERY_FPUP_ZZS
                // TACMD_NISEC_SYNC_COMPANYINFO
                // TACMD_AISINO_REPORT_DZ
                strcpy(pInputAndRepBuf, pDecRep);
                break;
        }
        nRet = nChildRet;
        break;
    }
    if (pDecRep)
        free(pDecRep);
    if (nRet < 0) {
        //��¼�����ɰط�����ȫ������ײ�洢������룬�˴�����
/*		if (hi->hDev != NULL) {
			if (hi->hDev->hUSB != NULL) {
        		SetLastError(ErrorNET, hi->hDev->hUSB, pInputAndRepBuf);
			}
		}*/        
    }
    return nRet;
}

// szInputXMLStrAndOutputMW �����������������һ�㶼�з���ֵ�������п���Ϊ���ַ���
int TaNetIoOnce(HDEV hDev, int nOpType, char *szInputXMLStrAndOutputMW, int nIOBufSize)
{
    struct HTTP hi;
    int nRet = -2;
    int nRepBufSize = 512000 + nIOBufSize;
    char *response = calloc(1, nRepBufSize);
    if (!response) {
        logout(INFO, "TAXLIB", "����������", "�ڴ�����ʧ��\r\n");
        return -1;
    }
    while (1) {
        if ((nRet = CreateServerTalk(&hi, hDev)) < 0) {
            if (hDev->bBreakAllIO) {
                SetLastError(hDev->hUSB, ERR_IO_BREAK, "���������������б������ӿ��ж�");
                hDev->bBreakAllIO = 0;  //�������
                nRet = -3;
                break;
            }
            HTTPClose(&hi);
            if ((nRet = CreateServerTalk(&hi, hDev)) < 0) {
                _WriteLog(LL_FATAL, "TaNetIoOnce connect failed, nRet:%d,errinfo:%s", nRet,
                          hDev->hUSB->errinfo);
                break;
            }
            _WriteLog(LL_INFO, "��һ������ʧ�����Գɹ�");
            // ���Թ����з��ֻ����нϸ߼��������������ʧ�������ܳɹ�
        }
        strcpy(response, szInputXMLStrAndOutputMW);
        if ((nRet = TaNetIo(&hi, nOpType, response, nRepBufSize)) < 0) {
            if (strlen(response) < nIOBufSize)
                strcpy(szInputXMLStrAndOutputMW, response);
            else
                logout(INFO, "TAXLIB", "����������",
                       "������������Ϣ�������ջ���,�޷���������,nRet = %d\r\n", nRet);
            _WriteLog(LL_FATAL, "TaNetIo failed, nRet:%d", nRet);
            break;
        }
        if (strlen(response) > nIOBufSize) {
            _WriteLog(LL_FATAL, "TaNetIo failed, nIOBufSize too small");
            SetLastError(hDev->hUSB, ERR_BUF_OVERFLOW, "�������ݹ��󳬳��ڴ淶Χ");
            break;
        }
        strcpy(szInputXMLStrAndOutputMW, response);
        nRet = 0;
        break;
    }
    HTTPClose(&hi);
    free(response);
    return nRet;
}
// finish---------------------------------ҵ��㹲ͬ����------------------------------------------------------
