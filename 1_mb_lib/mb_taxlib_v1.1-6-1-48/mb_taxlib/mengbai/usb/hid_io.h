#ifndef HID_IO_H
#define HID_IO_H

#include <libusb-1.0/libusb.h>
#include "../../common/common.h"
//#include "../../../software/_algorithm.h"
//#include "../../../core/_port_core.h"
#ifdef hid_io_c
#ifdef DEBUG
#define out(s,arg...)       				log_out("all.c",s,##arg) 
#else
#define out(s,arg...)						log_out("all.c",s,##arg) 
#endif	
#endif
int base_err_to_errnum(unsigned char err,char *errinfo);
int usb_device_open_hid(HUSB hUSB, int nBusNum, int nDevNum);
//int usb_hid_open(struct libusb_device_handle **device, int nBusNum, int nDevNum);
int usb_data_write_read(HUSB husb, unsigned char *indata, int in_len, unsigned char *outdata, int *out_len, char *errinfo);
int usb_data_write(HUSB husb, unsigned char *indata, int in_len);
int usb_data_read(HUSB husb, unsigned char *outdata, int *out_len);
void usb_hid_close(HUSB husb);
#endif