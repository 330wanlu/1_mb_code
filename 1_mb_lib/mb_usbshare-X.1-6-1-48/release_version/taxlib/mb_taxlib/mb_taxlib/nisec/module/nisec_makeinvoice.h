#ifndef NISEC_MAKEINVOICE_H
#define NISEC_MAKEINVOICE_H
#include "nisec_common.h"

int NisecMakeInvoiceMain(HFPXX fpxx);
void GetFotmatJEStr(char *je, char *szOut);
int FillTwocharRepeat(char *pBuff, char *szTwocharStr, int nRepeatCount);
int NisecDecKPRep(HFPXX hFpxx, uint8 *pUSBRaw, int nUSBRawLen);
int StrToNisecTemplate(bool bUtf8Buf, char *pDstBuf, char *abInput, int nInputLen);
int NisecUpdateNodeInfo(HFPXX fpxx);
int GetDevice128ByteSignRep(HDEV hDev, const char *sSignStrGBKBuff, int nGBKBuffLen,
                            char *pRecvBuff, int nRecvBuffLen);
char *GetSignBase64(HFPXX fpxx);
int NisecCryptHash(HUSB hUSB, uint8 bHashType, uint8 *abSha1Hash, int nSignBuflen, char *pRecvBuff,
                   int nRecvBuffLen);
int NisecFpxxAppendSignIO(HFPXX fpxx, bool bEraseSign);
int NisecCheckToReSign(HDEV hDev, uint8 bFplx, char *szFpdm, char *szFphm);
int BuildQTXM(bool bUtf8Buf, char *pBuff, HFPXX hFpxx);

#endif
