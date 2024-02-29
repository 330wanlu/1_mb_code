#ifndef RED_INVOICE_H
#define RED_INVOICE_H
#include "aisino_common.h"

int RedInvOnlineCheck_PTDZ(HDEV hDev, uint8 bAisinoFPLX, char *szLZFPDM, char *szLZFPHM,
                           uint32 nDZSYH, char *szOutMsg, int nOutMsgBuffSize);
int RedInvOnlineCheck_ZY(HFPXX fpxx,uint8 **inv_json);

//mengbai USED need declare
int HZXMLNode2FPXX(mxml_node_t *pXMLChildRoot, HFPXX fpxx);
int GetFpxxFromRepByRedNum(char *szRep, char *szRedNum, HFPXX fpxx);
#endif
