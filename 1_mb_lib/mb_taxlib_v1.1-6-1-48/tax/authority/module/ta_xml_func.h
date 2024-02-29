#ifndef TA_XML_FUNC_H
#define TA_XML_FUNC_H
#include "../ta_iface.h"

int LoadZZSXMLDataNodeTemplate(mxml_node_t **pXMLRootOut, mxml_node_t **nodeDataNode,
                               char *szDataNodeName, char *szInputXMLAndOutputError);
int LoadZZSXMLDataNodeTemplateCntax(mxml_node_t **pXMLRootOut, mxml_node_t **nodeDataNode,
                                    char *szDataNodeName, char *szInputXMLAndOutputError);
int GetChildNodeValue(mxml_node_t *node, char *szChildKey, char *szOutValue);
int LoopFindChildNodeValue(mxml_node_t *nodeMain, char *szChildKey, char *szOutChildValue);
int FilterXMLResult(int nOpType, mxml_node_t *pXMLRoot, char *pDecPayload, int nDecPayloadLen);
bool FindXMLNodeByKey(mxml_node_t *pXMLRoot, mxml_node_t *nodedata, char *szKey, int nValueMinSize,
                      char *szOut);
#endif
