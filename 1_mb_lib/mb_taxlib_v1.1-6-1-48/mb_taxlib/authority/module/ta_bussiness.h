#ifndef TA_BUSSINESS_H
#define TA_BUSSINESS_H
#include "../ta_iface.h"

int SwitchPubservice(HDEV hDev, bool bEnableOrDisable, char *szOutSQXX,char *errinfo);
int ZZSXML_ReportClear(HDEV hDev, bool bReportOrClear, bool bDevLocked, bool bRepReached,
                       int nLockedDay, char *szHZSJHexStr_OutPutBuffer);

#endif
