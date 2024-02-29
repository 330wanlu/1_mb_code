/*****************************************************************************
File name:   ta_xml_func.c
Description: 用于税局端ta_xml xml处理部分分流
             主要功能是实现xml非业务相关操作函数；
Author:      Zako
Version:     1.0
Date:        2021.08
History:
20210808     最初代码实现及他处代码移入
*****************************************************************************/
#include "ta_xml_func.h"

// begin----------------------------------功能性代码------------------------------------------------------
const char *XML_Formater(mxml_node_t *node, int where)
{
    //原生的mxml函数是返回静态字符串进行格式化，这是完全不能满足xml格式化显示需求的，因此对mxml库函数进行了修改
    //因此mxml库必须是我方自用，不能再使用官方的mxml库了，不能进行升级
    //经过测试后，一般不需要该回调函数进行格式化也可以通信，可以在后期调试或者hash比对启用该函数
    //使用不美化，压缩的xml可以节省网络流量;代码中可以加上美化的地方留注标记了,搜索'MXML_NO_CALLBACK/XML_Formater'即可
    //
    // 这四个参数分别代表一行XML的四个位置，以<tmp>sample</tmp>进行说明：
    // MXML_WS_BEFORE_OPEN代表<tmp>的前面，可以是'\t”或“\n"
    // MXML_WS_BEFORE_CLOSE代表<tmp>的后面，可以是"\n"
    // MXML_WS_AFTER_OPEN代表</tmp>的前面，可以是”\n"
    // MXML_WS_AFTER_CLOSE代表</tmp>的后面，可以是"\n"
    const char *name;
    name = mxmlGetElement(node);  //这里的utf8的xml头，需要和mxml newxml对应，否则异常
    if (!strcmp(name, "?xml version=\"1.0\" encoding=\"UTF-8\" ?") ||
        !strcmp(name, "?xml version=\"1.0\" encoding=\"GBK\"?")) {
        return NULL;
    }
    //取得层级
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
        //这是自我构造XML，暂时不用修改为mxmlGetOpaque
        char *p = (char *)mxmlGetText(node, 0);
        if (p == NULL) {
            nCond = 1;
        }
    }
    //树型格式化
    char *szBuffer = calloc(1, 128);
    if (where == MXML_WS_BEFORE_OPEN || nCond) {
        strcat(szBuffer, "\r\n");
        int i = 0;
        for (i = 0; i < nLevel; i++) {
            strcat(szBuffer, "    ");
        }
    }
    //返回的内存将由mxml回收
    return (const char *)(szBuffer);
}

//判断XML结果,并返回DATA节点以供枚举子节点
//如果执行错误,在szInputXMLAndOutputError返回错误消息
// pXMLRootOut和nodeDataNode成功时调用者释放,错误则不用释放
int LoadZZSXMLDataNodeTemplate(mxml_node_t **pXMLRootOut, mxml_node_t **nodeDataNode,
                               char *szDataNodeName, char *szInputXMLAndOutputError)
{
    mxml_node_t *node1 = NULL, *node2 = NULL;
    char *pPayload = NULL;
    int nRet = -1;
    // <?xml version="1.0"
    // encoding="gbk"?><FPXT><OUTPUT><CODE>0000</CODE><MESS>成功</MESS><DATA></DATA><x1></x1></OUTPUT></FPXT>
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
    // encoding="gbk"?><FPXT><OUTPUT><CODE>0000</CODE><MESS>成功</MESS><DATA></DATA><x1></x1></OUTPUT></FPXT>
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
    else  //此处改动较为重要!!!如果<key></key>该情形，value值为"",应该置0 buf，否则会出现野内存
        strcpy(szOutValue, "");
    return 0;
}

//少用，尽量使用GetChildNodeValue
bool FindXMLNodeByKey(mxml_node_t *pXMLRoot, mxml_node_t *nodedata, char *szKey, int nValueMinSize,
                      char *szOut)
{
    mxml_node_t *node1 = NULL;
    char *pPayload = NULL;
    if ((node1 = mxmlFindElement(nodedata, pXMLRoot, szKey, NULL, NULL, MXML_DESCEND)) == NULL)
        return false;
    if ((pPayload = (char *)mxmlGetOpaque(node1)) == NULL)
        return false;  //<key></key>情形
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

// finish----------------------------------功能性代码------------------------------------------------------