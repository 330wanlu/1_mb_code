#ifndef TA_XML_H
#define TA_XML_H
#include "ta_iface.h"

//˰���ܾ�����IOͨ����ȫ��װ�������ڲ��Զ�����HTTP������һ����IOͨ��(������)
int TaNetIoOnce(HDEV hDev, int nOpType, char *szInputXMLStrAndOutputMW, int nIOBufSize);
//�����ⲿ����ǰ������HTTP���ٴ������ӣ�����ʱ����HTTP����������ڶ��IO����(������)
int TaNetIo(HHTTP hi, int nOpType, char *pInputAndRepBuf, int nRepBufLen);

#endif  // TA_XML_H