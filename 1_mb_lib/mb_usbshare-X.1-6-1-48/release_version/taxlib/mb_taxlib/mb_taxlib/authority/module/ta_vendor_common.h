#ifndef TA_VENDOR_COMMON_H
#define TA_VENDOR_COMMON_H
#include "../ta_iface.h"

int SwitchPubservice(HDEV hDev, bool bEnableOrDisable, char *szOutSQXX);
int ZZSXML_ReportClear(HDEV hDev, bool bReportOrClear, bool bDevLocked, bool bRepReached,
                       int nLockedDay, char *szHZSJHexStr_OutPutBuffer);
//����������ϴ�
int UploadTZDBH(HFPXX fpxx, char *szOutTZDBH);
//����֪ͨ������
int RedinvTZDCancel(HDEV hDev, char *szTZDBH, char *szOutMsg, int nOutMsgBuffSize);
//����֪ͨ������
int DownloadTZDBH(HDEV hDev, char *szQueryDateRange, char *szGfsh, char *szXXBBH, char *szRetBuf,
                  int nBufSize);
int AddDZFPXMLPackage(char *szDataPayload, bool bPayloadNeedBase64, HDEV hDev, char *pOutBuf);
#endif
