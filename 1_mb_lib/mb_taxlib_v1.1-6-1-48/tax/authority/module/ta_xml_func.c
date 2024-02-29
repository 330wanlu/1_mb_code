/*****************************************************************************
File name:   ta_xml_func.c
Description: ����˰�ֶ�ta_xml xml�����ַ���
             ��Ҫ������ʵ��xml��ҵ����ز���������
Author:      Zako
Version:     1.0
Date:        2021.08
History:
20210808     �������ʵ�ּ�������������
*****************************************************************************/
#include "ta_xml_func.h"

// begin----------------------------------�����Դ���------------------------------------------------------
const char *XML_Formater(mxml_node_t *node, int where)
{
    //ԭ����mxml�����Ƿ��ؾ�̬�ַ������и�ʽ����������ȫ��������xml��ʽ����ʾ����ģ���˶�mxml�⺯���������޸�
    //���mxml��������ҷ����ã�������ʹ�ùٷ���mxml���ˣ����ܽ�������
    //�������Ժ�һ�㲻��Ҫ�ûص��������и�ʽ��Ҳ����ͨ�ţ������ں��ڵ��Ի���hash�ȶ����øú���
    //ʹ�ò�������ѹ����xml���Խ�ʡ��������;�����п��Լ��������ĵط���ע�����,����'MXML_NO_CALLBACK/XML_Formater'����
    //
    // ���ĸ������ֱ����һ��XML���ĸ�λ�ã���<tmp>sample</tmp>����˵����
    // MXML_WS_BEFORE_OPEN����<tmp>��ǰ�棬������'\t����\n"
    // MXML_WS_BEFORE_CLOSE����<tmp>�ĺ��棬������"\n"
    // MXML_WS_AFTER_OPEN����</tmp>��ǰ�棬�����ǡ�\n"
    // MXML_WS_AFTER_CLOSE����</tmp>�ĺ��棬������"\n"
    const char *name;
    name = mxmlGetElement(node);  //�����utf8��xmlͷ����Ҫ��mxml newxml��Ӧ�������쳣
    if (!strcmp(name, "?xml version=\"1.0\" encoding=\"UTF-8\" ?") ||
        !strcmp(name, "?xml version=\"1.0\" encoding=\"GBK\"?")) {
        return NULL;
    }
    //ȡ�ò㼶
    int nLevel = -1;
    mxml_node_t *nodeParent = node;
    while (1) {
        nodeParent = mxmlGetParent(nodeParent);
        if (NULL == nodeParent)
            break;
        nLevel++;
    }

    int nCond = 0;
    if (where == MXML_WS_BEFORE_CLOSE) {
        //�������ҹ���XML����ʱ�����޸�ΪmxmlGetOpaque
        char *p = (char *)mxmlGetText(node, 0);
        if (p == NULL) {
            nCond = 1;
        }
    }
    //���͸�ʽ��
    char *szBuffer = calloc(1, 128);
    if (where == MXML_WS_BEFORE_OPEN || nCond) {
        strcat(szBuffer, "\r\n");
        int i = 0;
        for (i = 0; i < nLevel; i++) {
            strcat(szBuffer, "    ");
        }
    }
    //���ص��ڴ潫��mxml����
    return (const char *)(szBuffer);
}

//�ж�XML���,������DATA�ڵ��Թ�ö���ӽڵ�
//���ִ�д���,��szInputXMLAndOutputError���ش�����Ϣ
// pXMLRootOut��nodeDataNode�ɹ�ʱ�������ͷ�,���������ͷ�
int LoadZZSXMLDataNodeTemplate(mxml_node_t **pXMLRootOut, mxml_node_t **nodeDataNode,
                               char *szDataNodeName, char *szInputXMLAndOutputError)
{
    mxml_node_t *node1 = NULL, *node2 = NULL;
    char *pPayload = NULL;
    int nRet = -1;
    // <?xml version="1.0"
    // encoding="gbk"?><FPXT><OUTPUT><CODE>0000</CODE><MESS>�ɹ�</MESS><DATA></DATA><x1></x1></OUTPUT></FPXT>
    if ((*pXMLRootOut = mxmlLoadString(NULL, szInputXMLAndOutputError, MXML_OPAQUE_CALLBACK)) ==
        NULL)
        return -1;
    while (1) {
        if ((node1 = mxmlFindElement(*pXMLRootOut, *pXMLRootOut, "FPXT", NULL, NULL,
                                     MXML_DESCEND)) == NULL)
            break;
        if ((node1 = mxmlFindElement(node1, *pXMLRootOut, "OUTPUT", NULL, NULL, MXML_DESCEND)) ==
            NULL)
            break;
        if ((node2 = mxmlFindElement(node1, *pXMLRootOut, "CODE", NULL, NULL, MXML_DESCEND)) ==
            NULL)
            break;
        if ((pPayload = (char *)mxmlGetOpaque(node2)) == NULL)
            break;
        if (!strcmp(pPayload, "0000"))
            nRet = 0;
        else
            nRet = -1;
        if (0 != nRet) {
            if ((node2 = mxmlFindElement(node1, *pXMLRootOut, "MESS", NULL, NULL, MXML_DESCEND)) ==
                NULL)
                break;
            if ((pPayload = (char *)mxmlGetOpaque(node2)) == NULL)
                break;
            strcpy(szInputXMLAndOutputError, pPayload);
            break;
        }
        if ((node2 = mxmlFindElement(node1, *pXMLRootOut, szDataNodeName, NULL, NULL,
                                     MXML_DESCEND)) == NULL) {
            nRet = -2;
            break;
        }
        *nodeDataNode = node2;
        nRet = 0;
        break;
    }
    if (nRet) {
        mxmlDelete(*pXMLRootOut);
        *pXMLRootOut = NULL;
        return -10;
    }
    return nRet;
}

int LoadZZSXMLDataNodeTemplateCntax(mxml_node_t **pXMLRootOut, mxml_node_t **nodeDataNode,
                                    char *szDataNodeName, char *szInputXMLAndOutputError)
{
    mxml_node_t *node1 = NULL, *node2 = NULL;
    char *pPayload = NULL;
    int nRet = -1;
    // <?xml version="1.0"
    // encoding="gbk"?><FPXT><OUTPUT><CODE>0000</CODE><MESS>�ɹ�</MESS><DATA></DATA><x1></x1></OUTPUT></FPXT>
    if ((*pXMLRootOut = mxmlLoadString(NULL, szInputXMLAndOutputError, MXML_OPAQUE_CALLBACK)) ==
        NULL)
        return -1;
    while (1) {
        if ((node1 = mxmlFindElement(*pXMLRootOut, *pXMLRootOut, "body", NULL, NULL,
                                     MXML_DESCEND)) == NULL) {
            nRet = -2;
            break;
        }
        if ((node1 = mxmlFindElement(node1, *pXMLRootOut, "tyxx", NULL, NULL, MXML_DESCEND)) ==
            NULL) {
            nRet = -3;
            break;
        }
        if ((node2 = mxmlFindElement(node1, *pXMLRootOut, "returncode", NULL, NULL,
                                     MXML_DESCEND)) == NULL) {
            nRet = -4;
            break;
        }
        if ((pPayload = (char *)mxmlGetOpaque(node2)) == NULL) {
            nRet = -5;
            break;
        }
        if (!strcmp(pPayload, "00"))
            nRet = 0;
        else {
            nRet = -6;
        }
        if (0 != nRet) {
            if ((node2 = mxmlFindElement(node1, *pXMLRootOut, "returnmsg", NULL, NULL,
                                         MXML_DESCEND)) == NULL) {
                nRet = -7;
                break;
            }
            if ((pPayload = (char *)mxmlGetOpaque(node2)) == NULL) {
                nRet = -8;
                break;
            }
            strcpy(szInputXMLAndOutputError, pPayload);
            {
                nRet = -9;
                break;
            }
        }
        if ((node2 = mxmlFindElement(node1, *pXMLRootOut, szDataNodeName, NULL, NULL,
                                     MXML_DESCEND)) == NULL) {
            nRet = -10;
            break;
        }
        *nodeDataNode = node2;
        nRet = 0;
        break;
    }
    if (nRet) {
        mxmlDelete(*pXMLRootOut);
        *pXMLRootOut = NULL;
        return nRet;
    }
    return nRet;
}

// szOutValue must be enough
// szChildKey's value canbe emptystring
int GetChildNodeValue(mxml_node_t *node, char *szChildKey, char *szOutValue)
{
    mxml_node_t *node2 = NULL;
    char *pPayload = NULL;
    if ((node2 = mxmlFindElement(node, node, szChildKey, NULL, NULL, MXML_DESCEND)) == NULL)
        return -1;
    if ((pPayload = (char *)mxmlGetOpaque(node2)) != NULL)
        strcpy(szOutValue, pPayload);
    else  //�˴��Ķ���Ϊ��Ҫ!!!���<key></key>�����Σ�valueֵΪ"",Ӧ����0 buf����������Ұ�ڴ�
        strcpy(szOutValue, "");
    return 0;
}

//���ã�����ʹ��GetChildNodeValue
bool FindXMLNodeByKey(mxml_node_t *pXMLRoot, mxml_node_t *nodedata, char *szKey, int nValueMinSize,
                      char *szOut)
{
    mxml_node_t *node1 = NULL;
    char *pPayload = NULL;
    if ((node1 = mxmlFindElement(nodedata, pXMLRoot, szKey, NULL, NULL, MXML_DESCEND)) == NULL)
        return false;
    if ((pPayload = (char *)mxmlGetOpaque(node1)) == NULL)
        return false;  //<key></key>����
    if (strlen(pPayload) < nValueMinSize)
        return false;
    strcpy(szOut, pPayload);
    return true;
}

int LoopFindChildNodeValue(mxml_node_t *nodeMain, char *szChildKey, char *szOutChildValue)
{
    //<business><body><returnCode>0</returnCode><returnMessage>XX</returnMessage><group
    // xh="1"><data name="fplx_dm" value="026"/><data name="yfpdm"
    // value="032001700211"/></group></body></business>
    mxml_node_t *node = NULL;
    if ((node = mxmlGetFirstChild(nodeMain)) == NULL)
        return -1;
    int i = 0;
    char *pValue = NULL;
    bool bFind = false;
    for (i = 0; i < 30; i++) {
        if ((node = mxmlGetNextSibling(node)) == NULL)
            break;
        if ((pValue = (char *)mxmlElementGetAttr(node, "name")) == NULL)
            continue;
        if (strcmp(pValue, szChildKey))
            continue;
        if ((pValue = (char *)mxmlElementGetAttr(node, "value")) == NULL)
            break;
        strcpy(szOutChildValue, pValue);
        bFind = true;
        break;
    }
    if (bFind)
        return 0;
    else
        return -2;
}

int FilterXMLResult(int nOpType, mxml_node_t *pXMLRoot, char *pDecPayload, int nDecPayloadLen)
{
    int nRet = -10;
    mxml_node_t *node = NULL, *node2 = NULL;
    char *pPayLoad = NULL;
    switch (nOpType) {
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
        case TACMD_NISEC_REDFORM_DOWNLOAD_ZZS:
            if ((node = mxmlFindElement(pXMLRoot, pXMLRoot, "INPUT", NULL, NULL, MXML_DESCEND)) ==
                NULL)
                return -1;
            break;
        default:
            if ((node = mxmlFindElement(pXMLRoot, pXMLRoot, "OUTPUT", NULL, NULL, MXML_DESCEND)) ==
                NULL)
                return -1;
            break;
    }
    if ((node2 = mxmlFindElement(node, pXMLRoot, "DATA", NULL, NULL, MXML_DESCEND)) == NULL)
        return -2;
    if ((pPayLoad = (char *)mxmlGetOpaque(node2)) == NULL)
        return -3;
    if (GBKBase64ToUTF8_NP(pPayLoad, pDecPayload, nDecPayloadLen) < 0) {
        return -4;
    }
    nRet = 0;
    return nRet;
}

// finish----------------------------------�����Դ���------------------------------------------------------