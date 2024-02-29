#ifndef AISINO_USBIO_H
#define AISINO_USBIO_H
#include "../module/aisino_common.h"

int AisinoLogicIO(HUSB hUSB, uint8 *pInOutCmd, int nInCmdLen, int nInCmdBufLen,
                  uint8 *pTransferData, int nTransferDataLen);
int AisinoCryptIO(HUSB hUSB, bool bBigCryptIO, uint8 *TransferData, int nTransferLen,
                  uint8 *pRecvDataBuff, int nRecvDataBuffLen);
uint16 GetDeviceRepCode(uint8 *pAARep);

extern uint8 g_CmdE1AA[7];
extern uint8 g_CmdE1AB[7];

#endif
