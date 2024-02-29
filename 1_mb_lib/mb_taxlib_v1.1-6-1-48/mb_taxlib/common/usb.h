#ifndef COMMON_USB_H
#define COMMON_USB_H
#include "common.h"

#define MSC_DIR_D2H (0x80)
#define MSC_DIR_H2D (0x00)

enum enDeviceOpenType { OPEN_DEV_AUTO = 1, OPEN_DEV_ID, OPEN_DEV_ENUM };

int usb_device_open(HUSB hUSB, int nBusNum, int nDevNum);
void usb_device_close(HUSB hUSB);
int usb_bulk_read(HUSB hUSB, void *data, int length);
int usb_bulk_write(HUSB hUSB, const void *data, int length);
int mass_storage_send_command(HUSB hUSB, const void *command_block, int command_block_length,
                              uint8 direction, int transfer_length);
int usb_device_open_raw(HUSB hUSB, uint8 bMethod, int nBusNum, int nDevNum,
                        int (*pCallBack)(int nDeviceType, HUSB hUSB));
#endif
