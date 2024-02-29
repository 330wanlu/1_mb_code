/*****************************************************************************
File name:   usb_device.c
Description: 整个工程大容量存储底层IO通用函数
Author:      Zako
Version:     1.0
Date:        2019.12
History:
20191214     单实例修改为会话模式，支持多线程
20201031     添加usb_device_open_raw接口
20220628     usb_device_open_raw逻辑精简，分离设备厂商识别代码
*****************************************************************************/
#include "usb_device.h"

#define LIBUSB_CHECK(action)                                                                       \
    do {                                                                                           \
        int ret = (action);                                                                        \
        if (ret != LIBUSB_SUCCESS) {                                                               \
            return -1;                                                                             \
        }                                                                                          \
        break;                                                                                     \
    } while (0)

//返回值<0 错误;0 未知设备；1 aisino; 2 nisec
int usb_device_open(HUSB hUSB, int nBusNum, int nDevNum)
{
	//if ((nBusNum<1) || (nBusNum>127))
	//{
	//	printf("nBusNum range error\n");
	//	
	//	return -1;
	//}
	//if ((nDevNum<1) || (nDevNum>127))
	//{
	//	printf("nDevNum range error\n");
	//	libusb_exit(hUSB->context);
	//	return -1;
	//}


	//uint16 payload_d2h = 0;
	//uint16 payload_h2d = 0;
	libusb_device **device_list = NULL;
	int device_num = libusb_get_device_list(hUSB->context, &device_list);
	if (device_num <= 0) {
		libusb_free_device_list(device_list, 1);
		fprintf(stderr, "[Device] No device found on the bus.\n");
		libusb_exit(hUSB->context);
		return -1;
	}
	

	int i = 0;
	for (i = 0; i < device_num; i++) 
	{
		struct libusb_device_descriptor desc;
		LIBUSB_CHECK(libusb_get_device_descriptor(device_list[i], &desc));
		int nNowBus = libusb_get_bus_number(device_list[i]);
		int nNowDev = libusb_get_device_address(device_list[i]);
		if (nBusNum != 0 || nDevNum != 0) 
		{
			if (nNowBus != nBusNum || nNowDev != nDevNum) 
			{
				continue;
			}
		}
		struct libusb_config_descriptor *conf_desc;
		LIBUSB_CHECK(libusb_get_active_config_descriptor(device_list[i], &conf_desc));
		unsigned int j = 0;
		for (j = 0; j < conf_desc->bNumInterfaces; j++) 
		{
			const struct libusb_interface *interface = &conf_desc->interface[j];
			if (interface->num_altsetting >= 1) 
			{
				const struct libusb_interface_descriptor *if_desc = &interface->altsetting[0];
				if (if_desc->bInterfaceClass == LIBUSB_CLASS_MASS_STORAGE && if_desc->bInterfaceSubClass == 0x06 && if_desc->bInterfaceProtocol == 0x50) 
				{
					unsigned int k = 0;
					for (k = 0; k < if_desc->bNumEndpoints; k++) 
					{
						const struct libusb_endpoint_descriptor *ep_desc = &if_desc->endpoint[k];

						if ((ep_desc->bmAttributes & 0x03) == LIBUSB_TRANSFER_TYPE_BULK) 
						{
							if (ep_desc->bEndpointAddress & 0x80)
							{
								hUSB->endpoint_d2h = ep_desc->bEndpointAddress;
								//payload_d2h = ep_desc->wMaxPacketSize;
							}
							else 
							{
								hUSB->endpoint_h2d = ep_desc->bEndpointAddress;
								//payload_h2d = ep_desc->wMaxPacketSize;
							}
						}
					}
					if (hUSB->endpoint_d2h == 0 || hUSB->endpoint_h2d == 0) 
					{
						fprintf(stderr, "[Device] Interface is not complete.\n");
						libusb_free_config_descriptor(conf_desc);
						libusb_free_device_list(device_list, 1);
						libusb_exit(hUSB->context);
						return -2;
					}

					LIBUSB_CHECK(libusb_open(device_list[i], &(hUSB->handle)));
					
					libusb_set_auto_detach_kernel_driver(hUSB->handle, 1);
					libusb_reset_device(hUSB->handle);//不能加LIBUSB_CHECK，mips兼容存在问题
					libusb_claim_interface(hUSB->handle, 0);


					char szManufacturer[256] = "";
					char szProduct[256] = "";
					if (libusb_get_string_descriptor_ascii(hUSB->handle, desc.iManufacturer,(uint8 *)szManufacturer, 256) < 0 || 
						libusb_get_string_descriptor_ascii(hUSB->handle, desc.iProduct, (uint8 *)szProduct, 256) < 0){
						fprintf(stderr,"[Device] libusb_get_string_descriptor_ascii is not complete.\n");
						if ((desc.idVendor == 0x0680) && (desc.idProduct == 0x2006))
						{
							libusb_free_config_descriptor(conf_desc);
							libusb_free_device_list(device_list, 1);
							return DEVICE_MENGBAI2;
						}
						if ((desc.idVendor == 0x2ec9) && (desc.idProduct == 0x1001))
						{
							libusb_free_config_descriptor(conf_desc);
							libusb_free_device_list(device_list, 1);
							return DEVICE_CNTAX;
						}
						if ((desc.idVendor == 0x3a59) && (desc.idProduct == 0x4458))
						{
							libusb_free_config_descriptor(conf_desc);
							libusb_free_device_list(device_list, 1);
							return DEVICE_CNTAX;
						}
						libusb_free_config_descriptor(conf_desc);
						libusb_free_device_list(device_list, 1);
						libusb_close(hUSB->handle);
						libusb_exit(hUSB->context);
						return -3;
					}
					//printf("[Device] BusID:%d DevID:%d Manufacturer:%s Product:%s\r\n", nNowBus,   nNowDev, szManufacturer, szProduct);
					int nDeviceType = -1;
					if (strstr((const char *)szManufacturer, "NISEC"))
					{
						if((desc.idVendor == 0x1432) && (desc.idProduct == 0x07dc))
						{
							nDeviceType = DEVICE_NISEC;
						}
						else if ((desc.idVendor == 0x1432) && (desc.idProduct == 0x07e2))
						{
							nDeviceType = DEVICE_SKP_KEY;
						}
					}
					else if (strstr((const char *)szManufacturer, "Aisino"))
					{			
						if (!strcmp(szProduct, "USB Flash Disk"))
							nDeviceType = DEVICE_AISINO;
						else if (!strcmp(szProduct, "ChinaTax SWUKey"))
							nDeviceType = DEVICE_CNTAX;
						else
							nDeviceType = DEVICE_AISINO;
					}
					else if (strstr((const char *)szManufacturer, "Aisno"))
					{
						nDeviceType = DEVICE_AISINO;
					}
					else if (strstr((const char *)szManufacturer, "ChinaTax SWUKey"))
					{
						if (strcmp(szProduct, "ChinaTax SWUKey") == 0)
						{
							if ((desc.idVendor == 0x14d6) && (desc.idProduct == 0xa002))
							{
								nDeviceType = DEVICE_CNTAX;
							}
							else if ((desc.idVendor == 0x3254) && (desc.idProduct == 0xff10))
							{
								nDeviceType = DEVICE_CNTAX;
							}
							else if ((desc.idVendor == 0x1677) && (desc.idProduct == 0x6101))
							{
								nDeviceType = DEVICE_CNTAX;
							}
							else if ((desc.idVendor == 0x2012) && (desc.idProduct == 0x2010))
							{
								nDeviceType = DEVICE_CNTAX;
							}
							else if ((desc.idVendor == 0x1432) && (desc.idProduct == 0x07d0))
							{
								nDeviceType = DEVICE_CNTAX;
							}
							else
							{
								nDeviceType = DEVICE_UNKNOW;
							}
						}
						else
						{
							nDeviceType = DEVICE_UNKNOW;
						}

					} 
					else if (strstr((const char *)szManufacturer, "USB_MSC"))
					{
						if (strcmp(szProduct, "ChinaTax SWUKey") == 0)
						{
							if ((desc.idVendor == 0x287f) && (desc.idProduct == 0x00d4))
							{
								nDeviceType = DEVICE_CNTAX;
							}
						}
						else
						{
							nDeviceType = DEVICE_UNKNOW;
						}
					}
					else if (strstr((const char *)szManufacturer, "ExcelSecu"))
					{
						if ((strcmp(szProduct, "ChinaTax SWUKey") == 0) || (strcmp(szProduct, "USB Key")==0))
						{
							if ((desc.idVendor == 0x1ea8) && (desc.idProduct == 0xb00b))
							{
								nDeviceType = DEVICE_CNTAX;
							}
						}
						else
						{
							nDeviceType = DEVICE_UNKNOW;
						}
					}
					else if (strstr((const char *)szManufacturer, "ShuiWu"))
					{
						if (strcmp(szProduct, "UKEY") == 0)
						{
							if ((desc.idVendor == 0x1ea8) && (desc.idProduct == 0xb00b))
							{
								nDeviceType = DEVICE_CNTAX;
							}
						}
						else
						{
							nDeviceType = DEVICE_UNKNOW;
						}
					}
					else if (strstr((const char *)szManufacturer, "SWUKEY"))
					{
						if (strcmp(szProduct+1, "ChinaTax SWUKey") == 0)
						{
							if ((desc.idVendor == 0xd449) && (desc.idProduct == 0x0001))
							{
								nDeviceType = DEVICE_CNTAX;
							}
							
						}
						else
						{
							nDeviceType = DEVICE_UNKNOW;
						}
					}
					else if (strstr((const char *)szManufacturer, "Watchdata"))
					{
						if (strcmp(szProduct + 1, "ChinaTax SWUKey") == 0)
						{
							if ((desc.idVendor == 0x163c) && (desc.idProduct == 0x5480))
							{
								nDeviceType = DEVICE_CNTAX;
							}

						}
						else
						{
							nDeviceType = DEVICE_UNKNOW;
						}
					}
					else if (strstr((const char *)szManufacturer, "RONGANCHINA"))
					{
						if (strcmp(szProduct, "SWUKey") == 0)
						{
							if ((desc.idVendor == 0x2ec9) && (desc.idProduct == 0x1001))
							{
								nDeviceType = DEVICE_CNTAX;
							}

						}
						else
						{
							nDeviceType = DEVICE_UNKNOW;
						}
					}
					else
					{
						nDeviceType = DEVICE_UNKNOW;
					}
					if (nDeviceType == DEVICE_NISEC) 
					{  //修正百旺读取描述符问题
                        unsigned char usb_data[128];
                        memset(usb_data, 0, sizeof usb_data);
                        // USB大容量存储需要这段代码？如果是第一次插上的设备执行会报个错误(资源占用的错误，不用管)，第二次再执行就无错了
                        libusb_control_transfer(hUSB->handle, 0xa1, 0xfe, 0x0000, 0, usb_data, 1,3000);
                    }

					libusb_free_config_descriptor(conf_desc);
					libusb_free_device_list(device_list, 1);
					return nDeviceType;
				}
			}
		}
		libusb_free_config_descriptor(conf_desc);
	}
	libusb_free_device_list(device_list, 1);
	libusb_exit(hUSB->context);
	fprintf(stderr, "[Device] No target device found.\n");
	return -4;
}


void usb_device_close(HUSB husb)
{
    if (husb != NULL)
        libusb_close(husb->handle);
}

int usb_bulk_read(HUSB husb, void *data, int length)
{
    int transferred;
	if (husb == NULL)
		return -1;
	if (husb->handle == NULL)
		return -2;
    LIBUSB_CHECK(libusb_bulk_transfer(husb->handle, husb->endpoint_d2h, (uint8 *)data, length,
                                      &transferred, 0));
    return transferred;
}

int usb_bulk_write(HUSB husb, const void *data, int length)
{
    int transferred;
	if (husb == NULL)
		return -1;
	if (husb->handle == NULL)
		return -2;
    LIBUSB_CHECK(libusb_bulk_transfer(husb->handle, husb->endpoint_h2d, (uint8 *)data, length,
									&transferred, 0));
    return transferred;
}

int mass_storage_send_command(HUSB husb, const void *command_block, int command_block_length,
                              uint8 direction, int transfer_length)
{
    uint8 cbw[31];
    memset(cbw, 0, 31);
    memcpy(cbw + 15, command_block, command_block_length);
    memcpy(cbw, "\x55\x53\x42\x43", 4);
    memcpy(cbw + 4, "\x98\xf8\xd6\x81", 4);
    memcpy(cbw + 8, &transfer_length, 4);
    cbw[12] = direction;
    cbw[13] = 0;
    cbw[14] = command_block_length;
    return usb_bulk_write(husb, cbw, 31);
}

//返回值<0，设备IO出错,-100为USB SCSI协议返回的执行状态代码不为0（错误）
//返回值=0，无额外数据可读
//返回值>0，有可用数据长度;pRecvDataBuff存放可用数据缓存
int mass_storage_io(HUSB husb, uint8 *pSendCmd, int nCmdLen, int UMSHeader_nDataTransferLen,
                    uint8 UMSHeader_direction, uint8 *pTransferData, int nTransferLen,
                    uint8 *pRecvDataBuff, int nRecvDataBuffLen)
{
    int nReallyTransferLen = UMSHeader_nDataTransferLen;
    if (0 != pTransferData)
        nReallyTransferLen = nTransferLen;
    int result =
        mass_storage_send_command(husb, pSendCmd, nCmdLen, UMSHeader_direction, nReallyTransferLen);
    if (result < 0)
        return -1;
    if (0 != pTransferData) {
        result = usb_bulk_write(husb, pTransferData, nTransferLen);
        if (result < 0)
            return -2;
    }
    memset(pRecvDataBuff, 0, nRecvDataBuffLen);
    result = usb_bulk_read(husb, pRecvDataBuff, nRecvDataBuffLen);
    if (result < 0)
        return -3;
    if (0x55 == pRecvDataBuff[0] && 0x53 == pRecvDataBuff[1] && 0x42 == pRecvDataBuff[2]) {
        ///先USBflag，后数据;出现USBFlag，说明执行已经完成，理论上已经可以直接取执行结果
        if (13 != result)
            return -3;
        uint8 bOK = pRecvDataBuff[12];
        if (0 != bOK)
            return -100;  //这个返回值不能改动
        uint32 uFollowDataLen = ntohl(*(uint32 *)(pRecvDataBuff + 8));
        if (uFollowDataLen > nRecvDataBuffLen)
            return -5;
        if (0 == uFollowDataLen)
            return 0;
        memset(pRecvDataBuff, 0, nRecvDataBuffLen);
        result = usb_bulk_read(husb, pRecvDataBuff, nRecvDataBuffLen);
        if (result < 0)
            return -4;
        return result;
    } else {  //先数据已经被上面的read读掉了，剩余只剩下后USBflag(爱信诺)；百旺会出现read返回为0，执行结果没出来，需要等会再读一次
        uint8 abNew[512];
        int i = 0;
        for (i = 0; i < 30; i++) {  //返回结果超时3秒读取
            memset(abNew, 0, 512);
            int nNew = usb_bulk_read(husb, abNew, 512);
            if (nNew < 0)
                break;
            if (0 == nNew) {
                usleep(100000);  // 100ms
                continue;
            }
            if (13 != nNew)
                break;
            uint8 bOK = abNew[12];
            if (0 != bOK)
                return -100;  //这个返回值不能改动
            return result;
        }
        return -6;
    }
    return 0;
}