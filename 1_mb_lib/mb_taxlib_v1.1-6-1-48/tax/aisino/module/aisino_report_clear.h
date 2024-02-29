#ifndef AISINO_RPORTCLEAR_H
#define AISINO_RPORTCLEAR_H
#include "aisino_common.h"

int ReporSummarytMain(HDEV hDev, uint8 bFplx_Aisino);
int ClearCardMain(HDEV hDev, uint8 bFplx_Aisino);

void AisinoFPLX2PrivateFPLX(uint8 bFplx_Aisino, uint8 *bFplx_Report_Common,
                            uint8 *bFplx_Report_Aisino);
int PeriodTaxReport(HUSB hUsb, uint8 bFplx_Report_Aisino);

#endif
