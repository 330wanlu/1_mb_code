#ifndef CNTAX_NETINVOICE_H
#define CNTAX_NETINVOICE_H
#include "cntax_common.h"

int CntaxNetInvoiceQueryReady(HDEV hDev, char *szOutFormatStrMultiLine);
int BuildCntaxUploadRedFormXML(HFPXX fpxx, char **szOut);
int CntaxBuildDownloadFormXML(HDEV hDev, char *szQueryDateRange, char *szGfsh, char *szXXBBH,
                              char **szOut);
int CntaxNetInvoiceDownloadWrite(HDEV hDev, char *szInputFormatSingleLine);

// mengbai USED need declare
int CntaxQueryNetInvoiceReady(uint8 bOpType, HHTTP hi, char *szInputOrOutFormatStr);
bool CntaxUnlockReadyInvoiceInDevice(void *ptrHHTTP);
int CntaxWriteGpxxmwToDevice(HDEV hDev, char *szGpxxmw, char *szSingleFormatLine);
#endif
