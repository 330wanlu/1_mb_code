#ifndef NISEC_REPORT_CLEAR_H
#define NISEC_REPORT_CLEAR_H
#include "nisec_common.h"

int NisecReporSummarytMain(HDEV hDev, uint8 bFplx_Common);
int NisecClearCardMain(HDEV hDev, uint8 bFplx_Common);
int GetMonthStatistics(HUSB hUSB, char *szDateBegin, char *szDateEnd,struct StMonthStatistics *pstStatistics);
#endif
