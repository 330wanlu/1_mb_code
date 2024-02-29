#ifndef NISEC_USBIO_H
#define NISEC_USBIO_H
#include "../module/nisec_common.h"

int NisecLogicIO(HUSB hUSB, uint8 *pInOutCmd, int nInCmdLen, int nInCmdBufLen, uint8 *pTransferData,
                 int nTransferDataLen);
int NisecLogicIORaw(HUSB hUSB, uint8 *pInOutCmd, int nInCmdLen, int nInCmdBufLen,
                    uint8 *pTransferData, int nTransferDataLen);
int CntaxLogicIO(HUSB hUSB, uint8 *pInOutCmd, int nInCmdLen, int nInCmdBufLen, uint8 *pTransferData,
                 int nTransferDataLen);

#endif
