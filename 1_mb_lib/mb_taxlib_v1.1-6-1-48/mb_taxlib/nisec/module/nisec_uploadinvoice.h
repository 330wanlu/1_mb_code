#ifndef NISEC_UPLOAD_INVOICE_H
#define NISEC_UPLOAD_INVOICE_H
#include "nisec_common.h"

int UploadInvoice_CallBack(HFPXX fpxx);
int QueryModifyInvoice(uint8 bInvType, HDEV hDev, char *szCurFPDM, char *szCurFPHM);
char *NisecPreCompanyXml(HDEV hDev);

// mengbai USED need declare
int NisecInvoiceUploadAndDownload(HFPXX fpxx, char *szOutDZSYH);
int NisecUpdateInvUploadFlag(HFPXX fpxx, char *szDZSYH);
#endif
