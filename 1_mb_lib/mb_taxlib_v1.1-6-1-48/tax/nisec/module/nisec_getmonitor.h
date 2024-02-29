#ifndef NISEC_GETMONITOR_H
#define NISEC_GETMONITOR_H
#include "nisec_common.h"

int GetMonitorInfo(HUSB hUSB, struct StMonitorInfo *stMIInfo);
int GetStockInfo(HDEV hDev, struct StStockInfoHead *stSKInfo);
int QueryInvoSection(HDEV hDev, char *szOutputHexStr);
int GetMonthStatistics(HUSB hUSB, char *szDateBegin, char *szDateEnd,
                       struct StMonthStatistics *pstStatistics);
int NisecGetTaxrateStrings(HUSB hUsb, char *szOut, int nOutBufSize);
int NisecUpdateMonitorFromTaSync(HUSB hUSB, mxml_node_t *pXMLRoot);

#endif
