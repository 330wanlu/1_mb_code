#ifndef TA_XML_IFACE_H
#define TA_XML_IFACE_H
#include "../ta_iface.h"

int BuildXML_DigitalEnvelope(uint8 bDeviceType, char *szDataPayload, char *pOutBuf);
int BuildInterfaceXML(uint8 bDeviceType, char *szTecBuf, char *szBussines, char **pOut);

int BusinessContentChildPreDo(int nOpType, char *pKey, char *pValue, char *szResult,
                              int nResultSize,char *errinfo);
int ZZSResultPreDo(int nOpType, mxml_node_t *pXMLRoot, mxml_node_t *nodedata, char *pOut,
                   int nBufSize, char *pXML,char *errinfo);

#endif
