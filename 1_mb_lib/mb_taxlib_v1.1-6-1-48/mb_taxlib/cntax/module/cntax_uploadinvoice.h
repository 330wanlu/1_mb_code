#ifndef CNTAX_UPLOAD_INVOICE_H
#define CNTAX_UPLOAD_INVOICE_H
#include "cntax_common.h"

int CntaxUploadInvoiceCallBack(HFPXX fpxx);
int CntaxQueryModifyInvoice(uint8 bInvType, HDEV hDev, char *szOutFPDM, char *szOutFPHM);
int CntaxQueryOfflineInvoice(uint8 bInvType, HDEV hDev);
int CheckFirewareUpdate(HHTTP hi);
char *CntaxPreCompanyXml(HDEV hDev);

// mengbai USED need declare
int CntaxInvoiceUploadAndDownload(HFPXX fpxx, char *szOutDZSYH);
int CntaxUpdateInvUploadFlag(HDEV hDev, uint8 bStep, uint8 *abTransferData, int nTransferDataLen);

#endif
