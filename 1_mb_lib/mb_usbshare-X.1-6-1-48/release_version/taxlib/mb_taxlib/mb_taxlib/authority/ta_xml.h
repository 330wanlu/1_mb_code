#ifndef TA_XML_H
#define TA_XML_H
#include "ta_iface.h"

//税务总局网络IO通信完全封装，函数内部自动建立HTTP，用于一次性IO通信(短连接)
int TaNetIoOnce(HDEV hDev, int nOpType, char *szInputXMLStrAndOutputMW, int nIOBufSize);
//函数外部调用前先申请HTTP，再创建连接，调用时传入HTTP句柄，可用于多次IO操作(长连接)
int TaNetIo(HHTTP hi, int nOpType, char *pInputAndRepBuf, int nRepBufLen);

#endif  // TA_XML_H