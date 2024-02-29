#ifndef CNTAX_GETMONITOR_H
#define CNTAX_GETMONITOR_H
#include "cntax_common.h"

int CntaxGetMonitorInfo(HUSB hUSB, struct StMonitorInfo *stMIInfo);
int CntaxGetStockInfo(HDEV hDev, struct StStockInfoHead *stSKInfo);
int CntaxGetMonthStatistics(HUSB hUSB, bool bPeriodAll, struct StMonthStatistics *pstStatistics);
int CntaxGetTaxrateStrings(HUSB hUsb, char *szOut, int nOutBufSize);
int CntaxUpdateMonitorFromTaSync(HUSB hUSB, uint8 bDefaultFplx, mxml_node_t *pXMLRoot);

#endif
