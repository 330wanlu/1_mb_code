#ifndef AISINO_MAKEINVOICE_H
#define AISINO_MAKEINVOICE_H

#include "aisino_common.h"

char *GetInvString(HFPXX fpxx);
char *FillSPXX(HFPXX fpxx, struct Spxx *stp_Spxx, int nInvoiceNum);
int LoadAisinoDevInfo(HDEV hDev, HUSB hUSB, uint8 bAuthDev);
int AisinoGetCurrentInvCode(HUSB hUSB, uint8 bNeedType, char *szInOutFPHM, char *szOptInvEndNum,
                            char *szInOutFPDM);
int AisinoUpdateNodeInfo(HFPXX fpxx, HDEV hDev);
int Packet8ByteVCode(uint8 *abProcKey, uint8 *pInvData, uint8 *dstBuff);
int GetProcKey(char *sz9ByteHashTaxID, uint8 *abProcKey);
int MakeInvoiceMain(HFPXX fpxx);
int FillDevInfoCert(HDEV hDev, uint8 bAuthDev);
int AisinoBlankWaste(HDEV hDev, uint8 bInvoiceType, char *szFPDM, char *szFPHM, char *zfr,
                     int nCount, int *need_restart);

// mengbai USED need declare
int CheckDriverVerSupport(HDEV hDev, char *szDriverVersion);

#endif
