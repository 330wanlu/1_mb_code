#ifndef NISEC_GETINVOICE_H
#define NISEC_GETINVOICE_H
#include "nisec_common.h"

int GetInvMonthFormat(char *szFormatYearMonth, uint8 *szRetBuff);
int GetInvBuff(HUSB hUSB, uint8 bOPType, uint8 **pRetBuff, int *nRetBuffLen, uint8 *abQueryInput,
               int nQueryInputSize, bool bReadOrPeek);
int NisecGetInvDetail(HDEV hDev, uint8 bInvType, uint8 *pBuf, int nInvNum, int (*pCallBack)(HFPXX));
int NisecQueryInvInfo(HDEV hDev, uint8 bInvType, char *szFPDM, char *szFPHM, HFPXX fpxx);
int NisecWasteInvoice(HDEV hDev, uint8 bInvType, bool bBlankWaste, char *szFPDM, char *szFPHM,
                      char *zfr, int nCount,int *need_restart);//wang 03 07 空白发空白发票最后一张
int NisecGetFPHumanceBuf(HDEV hDev, uint8 bInvType, uint8 *pBlockBin);
int NisecInvRaw2Fpxx(HDEV hDev, uint8 bInvType, uint8 *pBlockBin, HFPXX fpxx, int *pnBlockBinLen);
int GetFYXMorQDXM(HFPXX fpxx, bool bUtf8Buf, bool bFYorQD, char *pDetailBuff);
int GetQTXM(bool bUtf8Buf, char *pDetailBuff);
int Decode2020YearBuf(HFPXX fpxx, bool bUtf8Buf, char *pDetailBuff, int nIndex);
int NisecGetTailInvVersion(HDEV hDev, char *szOutVersion);

#endif
