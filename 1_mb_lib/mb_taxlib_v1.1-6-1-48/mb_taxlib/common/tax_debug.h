#ifndef TAX_DEBUG_H
#define TAX_DEBUG_H
#include "tax_common.h"

int SaveNoDeviceJson(HDEV hDev, uint8 bCommonFplx);
int LoadNoDeviceJson(HDEV hDev);
void SaveFpxxToDisk(HFPXX fpxx);
void SaveFpxxByDMHM(HDEV hDev, char *szFPDM, char *szFPHM);

#endif