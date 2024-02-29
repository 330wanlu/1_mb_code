#ifndef NISEC_NETINVOICE_H
#define NISEC_NETINVOICE_H
#include "nisec_common.h"

int NisecUpdateCompanyInfo(HUSB hUSB, char *szNewCompanyInfoBinStr);
int NisecNetInvoiceQueryReady(HDEV hDev, char *szOutFormatStrMultiLine);
int NisecNetInvoiceDownloadWrite(HDEV hDev, char *szInputFormatSingleLine);
bool UnlockReadyInvoiceInDevice(void *ptrHHTTP);

// mengbai USED need declare
int NisecQueryNetInvoiceReady(uint8 bOpType, HHTTP hi, char *szInputOrOutFormatStr);
int WriteGpxxmwToDevice(HDEV hDev, char *szGpxxmw, char *szSingleFormatLine);
#endif
