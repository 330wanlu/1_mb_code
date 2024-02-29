#ifndef USB_DEVICE_H
#define USB_DEVICE_H
#include "common.h"

#define MSC_DIR_D2H (0x80)
#define MSC_DIR_H2D (0x00)

int usb_device_open(HUSB hUSB, int nBusNum, int nDevNum);
void usb_device_close(HUSB hUSB);
int usb_bulk_read(HUSB hUSB, void *data, int length);
int usb_bulk_write(HUSB hUSB, const void *data, int length);
int mass_storage_send_command(HUSB hUSB, const void *command_block, int command_block_length,
                              uint8 direction, int transfer_length);
int mass_storage_io(HUSB hUSB, uint8 *pSendCmd, int nCmdLen, int UMSHeader_nDataTransferLen,
                    uint8 UMSHeader_direction, uint8 *pTransferData, int nTransferLen,
                    uint8 *pRecvDataBuff, int nRecvDataBuffLen);
#endif
