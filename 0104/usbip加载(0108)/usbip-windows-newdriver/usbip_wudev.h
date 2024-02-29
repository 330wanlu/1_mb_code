#ifndef _USBIP_WUDEV_H_
#define _USBIP_WUDEV_H_

#include "usbip_protocol.h"

#include "win_stub.h"
#include <winsock2.h>
#include "mylog.h"
typedef struct {
	unsigned	devid;
	uint32_t	speed;

	uint16_t	idVendor;
	uint16_t	idProduct;
	uint16_t	bcdDevice;

	uint8_t		bDeviceClass;
	uint8_t		bDeviceSubClass;
	uint8_t		bDeviceProtocol;

	uint8_t		bNumInterfaces;
} usbip_wudev_t;

extern void get_wudev(SOCKET sockfd, usbip_wudev_t *uwdev, struct usb_device *udev);

#endif /* _USBIP_WUDEV_H_ */
