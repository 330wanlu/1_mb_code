#ifndef CNTAX_MAKEINVOICE_H
#define CNTAX_MAKEINVOICE_H
#include "cntax_common.h"

int CntaxMakeInvoiceMain(HFPXX fpxx);
int CntaxGetDevice64ByteSignRep(HDEV hDev, const char *sSignStrGBKBuff, int nGBKBuffLen,
                                char *pRecvBuff, int nRecvBuffLen);
int CntaxGetDevice64ByteSignRepAsn1(HDEV hDev, bool bHeadWith0x3044, const char *sSignStrGBKBuff,
                                    int nGBKBuffLen, char *pRecvBuff, int nRecvBuffLen);
int CntaxCheckToReSign(HDEV hDev, uint8 bFplx, char *szFpdm, char *szFphm);
int CntaxFpxxAppendSignIO(HFPXX fpxx, bool bEraseSign);
int CntaxUpdateNodeInfo(HFPXX fpxx);

#endif
