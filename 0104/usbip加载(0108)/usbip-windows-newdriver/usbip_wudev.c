#include "usbip_wudev.h"

static BOOL
is_zero_class(usbip_wudev_t *wudev)
{
	if (wudev->bDeviceClass == 0 && wudev->bDeviceSubClass == 0 && wudev->bDeviceProtocol == 0)
		return TRUE;
	return FALSE;
}
static int usbip_net_xmit(SOCKET sockfd, void *buff, size_t bufflen, int sending)
{
	int total = 0;

	if (!bufflen) return 0;

	do {
		int nbytes;

		if (sending)
			nbytes = send(sockfd, buff, (int)bufflen, 0);
		else
			nbytes = recv(sockfd, buff, (int)bufflen, 0);

		if (nbytes <= 0)
			return -1;

		buff = (void *)((intptr_t)buff + nbytes);
		bufflen -= nbytes;
		total += nbytes;
	} while (bufflen > 0);

	return total;
}

int usbip_net_recv(SOCKET sockfd, void *buff, size_t bufflen)
{
	return usbip_net_xmit(sockfd, buff, bufflen, 0);
}
int usbip_net_send(SOCKET sockfd, void *buff, size_t bufflen)
{
	return usbip_net_xmit(sockfd, buff, bufflen, 1);
}
static int
fetch_conf_desc(SOCKET sockfd, unsigned devid, char *pdesc, unsigned desc_size)
{
	int ret,count;
	char c;
	struct usbip_header	uhdr;
	unsigned	alen;
	unsigned char send_new[1024] = {0};
	int send_len = 0;
	int i ;
	memset(&uhdr, 0, sizeof(uhdr));
	count = 0;
	uhdr.base.command = htonl(USBIP_CMD_SUBMIT);
	/* sufficient large enough seq used to avoid conflict with normal vhci operation */
	uhdr.base.seqnum = htonl(0x7fffffff);
	uhdr.base.direction = htonl(USBIP_DIR_IN);
	uhdr.base.devid = htonl(devid);

	uhdr.u.cmd_submit.transfer_buffer_length = htonl(desc_size);
	uhdr.u.cmd_submit.setup[0] = 0x80;	/* IN/control port */
	uhdr.u.cmd_submit.setup[1] = 6;		/* GetDescriptor */
	*(unsigned short *)(uhdr.u.cmd_submit.setup + 6) = (unsigned short)desc_size;	/* Length */
	uhdr.u.cmd_submit.setup[3] = 2;		/* Configuration Descriptor */

	send_len = sizeof(uhdr);
	memcpy(send_new,&uhdr,sizeof(uhdr));
	if (usbip_net_send(sockfd, send_new, send_len) < 0) {
		WriteLogFile("fetch_conf_desc: failed to send usbip header\n");
		return -1;
	}
	if (usbip_net_recv(sockfd, &uhdr, sizeof(uhdr)) < 0) {
		WriteLogFile("fetch_conf_desc: failed to recv usbip header\n");
		return -1;
	}
	if (uhdr.u.ret_submit.status != 0) {
		WriteLogFile("fetch_conf_desc: command submit error: %d\n", uhdr.u.ret_submit.status);
		return -1;
	}
	alen = ntohl(uhdr.u.ret_submit.actual_length);
	if (alen < desc_size) {
		WriteLogFile("new fetch_conf_desc: too short response: actual length: %d :%d\n", alen,desc_size);
		usbip_net_recv(sockfd, pdesc, alen);
		return -1;
	}
	if (usbip_net_recv(sockfd, pdesc, alen) < 0) {
		WriteLogFile("fetch_conf_desc: failed to recv usbip payload\n");
		return -1;
	}
	return 0;
}

/*
* Sadly, udev structure from linux does not have an interface descriptor.
* So we should get interface class number via GET_DESCRIPTOR usb command.
*/
static void
supplement_with_interface(SOCKET sockfd, usbip_wudev_t *wudev)
{
	unsigned char	buf[18] = {0};
	int i,ret;
	ret = fetch_conf_desc(sockfd, wudev->devid, buf, 18);
	if ( ret< 0) {
		WriteLogFile("failed to adjust device descriptor with interface descriptor\n");
		return;
	}
	wudev->bNumInterfaces = buf[4];
	for(i = 0; i < 18; i++)
	{
		WriteLogFile("%02x ",buf[i]);
	}
	WriteLogFile("\n");
	if (wudev->bNumInterfaces == 1){
		/* buf[4] holds the number of interfaces in USB configuration.
		* Supplement class/subclass/protocol only if there exists only single interface.
		* A device with multiple interfaces will be detected as a composite by vhci.
		*/
		wudev->bDeviceClass = buf[14]; 
		wudev->bDeviceSubClass = buf[15];
		wudev->bDeviceProtocol = buf[16];

	}
}

static void
setup_wudev_from_udev(usbip_wudev_t *wudev, struct usb_device *udev)
{
	int i;
	wudev->devid = udev->busnum << 16 | udev->devnum;

	wudev->idVendor = udev->idVendor;
	wudev->idProduct = udev->idProduct;
	wudev->bcdDevice = udev->bcdDevice;
	wudev->speed = udev->speed;

	wudev->bDeviceClass = udev->bDeviceClass;
	wudev->bDeviceSubClass =udev->bDeviceSubClass;
	wudev->bDeviceProtocol =  udev->bDeviceProtocol;
	wudev->bNumInterfaces = udev->bNumInterfaces;
	WriteLogFile("setup_wudev_from_udev:bDeviceClass:%02x -bDeviceSubClass:%02x -bDeviceProtocol:%02x -bNumInterfaces:%02x \n",udev->bDeviceClass,udev->bDeviceSubClass,udev->bDeviceProtocol,udev->bNumInterfaces);
}

void
get_wudev(SOCKET sockfd, usbip_wudev_t *wudev, struct usb_device *udev)
{
	setup_wudev_from_udev(wudev, udev);

	/* Many devices have 0 usb class number in a device descriptor.
	* 0 value means that class number is determined at interface level.
	* USB class, subclass and protocol numbers should be setup before importing.
	* Because windows vhci driver builds a device compatible id with those numbers.
	*/
	if (is_zero_class(wudev)) {
		supplement_with_interface(sockfd, wudev);
	}
}
