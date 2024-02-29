#ifndef CNTAX_GETINVOICE_H
#define CNTAX_GETINVOICE_H
#include "cntax_common.h"

int CntaxGetInvBuff(HUSB hUSB, uint8 bOPType, uint8 **pRetBuff, int *nRetBuffLen,
                    uint8 *abQueryInput, int nQueryInputSize, char *szDriverVersion);
int CntaxGetInvDetail(HDEV hDev, uint8 bBlockBufType, uint8 *pBuf, int nBufSize,
                      int (*pCallBack)(HFPXX));
int CntaxQueryInvInfo(HDEV hDev, uint8 bInvType, char *szFPDM, char *szFPHM, HFPXX pOutFpxx);
int CntaxWasteInvoice(HDEV hDev, uint8 bInvType, bool bBlankWaste, char *szFPDM, char *szFPHM,
                      char *zfr, int nCount,int *need_restart);
int CntaxGetTailInvVersion(HDEV hDev, char *szOutVersion);

// mengbai USED need declare
int CntaxInvRaw2Fpxx(HDEV hDev, uint8 bBlockBufType, uint8 *pBlockBin, HFPXX pOutFpxx,
                     int *pnBlockBinLen);
int TransferFPDMAndFPHMToBytes(char *szInFPDM, char *szInFPHMBegin, int nQueryCount,
                               uint8 *pOutBuf);
#endif
