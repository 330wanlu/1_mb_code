#ifndef TA_INVOICE_H
#define TA_INVOICE_H
#include "../ta_iface.h"

//红字申请表上传
int UploadTZDBH(HFPXX fpxx, char *szOutTZDBH, char *errinfo);
//红字通知单撤销
int RedinvTZDCancel(HDEV hDev, char *szTZDBH, char *szOutMsg, int nOutMsgBuffSize,char *errinfo);
//红字通知单下载
int DownloadTZDBH(HDEV hDev, char *szQueryDateRange, char *szGfsh, char *szXXBBH, char *szRetBuf,int nBufSize,char *errinfo);

int AddDZFPXMLPackage(char *szDataPayload, bool bPayloadNeedBase64, HDEV hDev, char *pOutBuf);
#endif
