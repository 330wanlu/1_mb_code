#define hid_io_c
#include "hid_io.h"


//static libusb_context *usb = NULL;
//static uint8 endpoint_d2h = 0, endpoint_h2d = 0;
//static uint16 payload_d2h = 0, payload_h2d = 0;


//static int inpoint = 0x82, outpoint = 0x01;
#define LIBUSB_CHECK(action)\
do {\
	int ret = (action);\
if (ret != LIBUSB_SUCCESS) {\
	return -1;\
}\
	break;\
} while (0)


int usb_device_open_hid(HUSB hUSB, int nBusNum, int nDevNum)
{
	if ((nBusNum<1) || (nBusNum>127))
	{
		printf("nBusNum range error\n");
		return -1;
	}
	if ((nDevNum<1) || (nDevNum>127))
	{
		printf("nDevNum range error\n");
		return -1;
	}

	//if (nBusNum) {
	//    fprintf(stdout, "[Device] Assign busid devid, %d:%d\n", nBusNum, nDevNum);
	//} else {
	//    fprintf(stdout, "[Device] Auto detect mass storage device\n");
	//}
	//uint16 payload_d2h = 0;
	//uint16 payload_h2d = 0;
	libusb_device **device_list = NULL;
	int device_num = libusb_get_device_list(hUSB->context, &device_list);
	if (device_num <= 0) {
		libusb_free_device_list(device_list, 1);
		fprintf(stderr, "[Device] No device found on the bus.\n");
		return -1;
	}
	int i = 0;
	for (i = 0; i < device_num; i++) {
		struct libusb_device_descriptor desc;
		LIBUSB_CHECK(libusb_get_device_descriptor(device_list[i], &desc));
		int nNowBus = libusb_get_bus_number(device_list[i]);
		int nNowDev = libusb_get_device_address(device_list[i]);
		if (nBusNum != 0 || nDevNum != 0) {
			if (nNowBus != nBusNum || nNowDev != nDevNum) {
				continue;
			}
		}
		struct libusb_config_descriptor *conf_desc;
		LIBUSB_CHECK(libusb_get_active_config_descriptor(device_list[i], &conf_desc));
		unsigned int j = 0;
		for (j = 0; j < conf_desc->bNumInterfaces; j++) {
			const struct libusb_interface *interface = &conf_desc->interface[j];
			if (interface->num_altsetting >= 1) {
				const struct libusb_interface_descriptor *if_desc = &interface->altsetting[0];
				/*if (if_desc->bInterfaceClass == LIBUSB_CLASS_MASS_STORAGE &&
				if_desc->bInterfaceSubClass == 0x06 && if_desc->bInterfaceProtocol == 0x50) {*/
				if (1){
					unsigned int k = 0;
					for (k = 0; k < if_desc->bNumEndpoints; k++) {
						const struct libusb_endpoint_descriptor *ep_desc = &if_desc->endpoint[k];

						if ((ep_desc->bmAttributes & 0x03) == LIBUSB_TRANSFER_TYPE_BULK) {
							if (ep_desc->bEndpointAddress & 0x80) {
								hUSB->endpoint_d2h = ep_desc->bEndpointAddress;
								//payload_d2h = ep_desc->wMaxPacketSize;
							}
							else {
								hUSB->endpoint_h2d = ep_desc->bEndpointAddress;
								//payload_h2d = ep_desc->wMaxPacketSize;
							}
						}
					}
					//printf("payload_d2h = %d,payload_h2d = %d\n", payload_d2h, payload_h2d);
					/*if (hUSB->endpoint_d2h == 0 || hUSB->endpoint_h2d == 0) {
					fprintf(stderr, "[Device] Interface is not complete.\n");
					libusb_free_config_descriptor(conf_desc);
					libusb_free_device_list(device_list, 1);
					return -2;
					}*/

					LIBUSB_CHECK(libusb_open(device_list[i], &(hUSB->handle)));
					LIBUSB_CHECK(libusb_reset_device(hUSB->handle));
					libusb_set_auto_detach_kernel_driver(hUSB->handle, 1);
					LIBUSB_CHECK(libusb_claim_interface(hUSB->handle, 0));

					libusb_set_auto_detach_kernel_driver(hUSB->handle, 1);
					if (libusb_reset_device(hUSB->handle) < 0) {
						fprintf(stderr, "[Device] libusb_reset_device reset failed\n");
						libusb_free_config_descriptor(conf_desc);
						libusb_free_device_list(device_list, 1);
						libusb_close(hUSB->handle);
						return -3;
					}
					uint8 szManufacturer[256] = "";
					if (libusb_get_string_descriptor_ascii(hUSB->handle, desc.iManufacturer,
						szManufacturer, 256) < 0)
					{
						fprintf(stderr, "[Device] libusb_get_string_descriptor_ascii failed\n");
						libusb_free_config_descriptor(conf_desc);
						libusb_free_device_list(device_list, 1);
						libusb_close(hUSB->handle);
						return -1;
					}

					int nDeviceType = -1;
					if ((desc.idVendor == 0x0680) && (desc.idProduct == 0x1901))
					{
						nDeviceType = DEVICE_MENGBAI;
					}
					else if((desc.idVendor == 0x0680) && (desc.idProduct == 0x2008))
					{
						nDeviceType = DEVICE_MENGBAI2;
					}
					else
						nDeviceType = DEVICE_UNKNOW;

					libusb_free_config_descriptor(conf_desc);
					libusb_free_device_list(device_list, 1);
					return nDeviceType;
				}
			}
		}
		libusb_free_config_descriptor(conf_desc);
	}
	libusb_free_device_list(device_list, 1);
	fprintf(stderr, "[Device] No target device found.\n");
	return -4;
}


















int usb_data_write_read(HUSB husb, unsigned char *indata, int in_len, unsigned char *outdata, int *out_len, char *errinfo)
{
	int err_num;
	int result;
	int i;
	int num;
	int size;
	int total_len;
	int left_len;
	int index = 0;
	unsigned char s_data[50];
	unsigned char *r_data;
	uint16 crc_v;
	uint16 crc;
	
	r_data = (uint8 *)malloc(1000000);
	memset(r_data, 0, sizeof(1000000));
	num = in_len / 64 + 1;

	//out("发送数据,num= %d,in_len=%d\n",num,in_len);
	//out("发送数据:");
	//printf_array((char *)indata, in_len);
	//char s_time[100] = { 0 };
	//char e_time[100] = { 0 };
	//long start_time;
	//long end_time;
	//start_time = get_time_sec();
	//timer_read_asc(s_time);
	for (i = 0; i < num; i++)
	{
		result = libusb_interrupt_transfer(husb->handle, 0x01, indata + i * 64, 64, &size, 4000);
		if (result < 0)
		{
			out("数据发送失败,result = %d\n",result);
			sprintf(errinfo, "模拟盘底层数据写入超时失败");
			free(r_data);
			return DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
		}
		
	}
	//out("准备进行数据接收\n");	
	result = libusb_interrupt_transfer(husb->handle, 0x82, r_data, 64, &size, 2000);
	if (result < 0)
	{
		out("数据接收失败,result = %d\n",result);
		sprintf(errinfo, "模拟盘底层数据应答超时失败");
		free(r_data);
		return DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
	}
	if ((r_data[0] != 0x4d) || (r_data[1] != 0x42))
	{
		out("版本号错误\n");
		sprintf(errinfo, "模拟盘底层数据应答版本号错误");
		free(r_data);
		return DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
	}
	total_len = r_data[2] * 0x100 + r_data[3];
	left_len = total_len - 22;
	//out("total_len = %d\n", total_len);
	//out("left_len = %d\n", left_len);
	if (total_len > *out_len)
	{
		out("接收缓冲去内存太小\n");
		sprintf(errinfo, "模拟盘底层数据接收缓冲区太小");
		free(r_data);
		return DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
	}
	if (left_len > 0)
	{
		num = (left_len / 22) + 1;
		//out("num = %d\n", num);
		for (i = 0; i < num; i++)
		{
			memset(s_data, 0, sizeof(s_data));
			libusb_interrupt_transfer(husb->handle, 0x82, s_data, 22, &size, 1000);
			memcpy(r_data + (22 * (i + 1)), s_data, size);
			index += size;
		}
	}
	else
	{
		num = 0;
	}
	crc = crc_8005(r_data, total_len - 2, 0);
	crc_v = r_data[total_len - 2] * 256 + r_data[total_len - 1];
	if (crc != crc_v)
	{
		out("crc校验错误\n");
		sprintf(errinfo,"模拟盘底层应答CRC数据校验错误");
		free(r_data);
		return DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
	}
	//out("应答数据：");
	//printf_array((char *)r_data, total_len);

	if (r_data[4] != 0)
	{
		out("USB操作应答为错误代码,%02x\n",r_data[4]);

		err_num = base_err_to_errnum(r_data[4],errinfo);
		free(r_data);
		r_data = NULL;
		return err_num;
	}

	*out_len = total_len;
	memcpy(outdata, r_data+6, total_len-8);
	free(r_data);
	r_data = NULL;
	return *out_len;
}

int base_err_to_errnum(unsigned char err,char *errinfo)
{
	int errnum;
	switch (err)
	{
	case 0xFF:
		out("未联机\n");
		sprintf(errinfo, "未联机");
		errnum = DF_TAX_ERR_CODE_UNSUPPORT_INV_TYPE;
		break;
	case 0xFE:
		out("时间格式错误\n");
		sprintf(errinfo, "时间格式错误");
		errnum = DF_TAX_ERR_CODE_UNSUPPORT_INV_TYPE;
		break;
	case 0xFD:
		out("卡编号不符合规范\n");
		sprintf(errinfo, "卡编号不符合规范");
		errnum = DF_TAX_ERR_CODE_UNSUPPORT_INV_TYPE;
		break;
	case 0xFC:
		out("未注册\n");
		sprintf(errinfo, "未注册");
		errnum = DF_TAX_ERR_CODE_UNSUPPORT_INV_TYPE;
		break;
	case 0xFB:
		out("未开卡\n");
		sprintf(errinfo, "未开卡");
		errnum = DF_TAX_ERR_CODE_UNSUPPORT_INV_TYPE;
		break;
	case 0xFA:
		out("此卡已格式化，请重新上电\n");
		sprintf(errinfo, "此卡已格式化，请重新上电");
		errnum = DF_TAX_ERR_CODE_UNSUPPORT_INV_TYPE;
		break;
	case 0xF9:
		out("开卡错误,该盘已经开卡\n");
		sprintf(errinfo, "开卡错误,该盘已经开卡");
		errnum = DF_TAX_ERR_CODE_UNSUPPORT_INV_TYPE;
		break;
	case 0xF8:
		out("无此发票类型\n");
		sprintf(errinfo, "无此发票类型");
		errnum = DF_TAX_ERR_CODE_UNSUPPORT_INV_TYPE;
		break;
	case 0xF7:
		out("无可用发票\n");
		sprintf(errinfo, "无可用发票");
		errnum = DF_TAX_ERR_CODE_HAVE_NO_INVOICE;
		break;
	case 0xF6:
		out("口令错误\n");
		sprintf(errinfo, "口令错误");
		errnum = DF_TAX_ERR_CODE_PLATE_CERT_PASSWD_ERR;
		break;
	case 0xF5:
		out("数字证书口令为8位\n");
		sprintf(errinfo, "数字证书口令为8位");
		errnum = DF_TAX_ERR_CODE_PLATE_CERT_PASSWD_ERR;
		break;
	case 0xF4:
		out("该发票类型代码已存在\n");
		sprintf(errinfo, "该发票类型代码已存在");
		errnum = DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
		break;
	case 0xF3:
		out("该授权税率已存在\n");
		sprintf(errinfo, "该授权税率已存在");
		errnum = DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
		break;
	case 0xF2:
		out("该发票段存在重复\n");
		sprintf(errinfo, "该发票段存在重复");
		errnum = DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
		break;
	case 0xF1:
		out("发票库存已满\n");
		sprintf(errinfo, "发票库存已满");
		errnum = DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
		break;
	case 0xF0:
		out("无此发票段\n");
		sprintf(errinfo, "无此发票段");
		errnum = DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
		break;
	case 0xEF:
		out("购货单位是必填项\n");
		sprintf(errinfo, "购货单位是必填项");
		errnum = DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
		break;
	case 0xEE:
		out("超过开票截至日期\n");
		sprintf(errinfo, "超过开票截至日期");
		errnum = DF_TAX_ERR_CODE_EXCEED_KP_END_DATE;
		break;
	case 0xED:
		out("超过单张发票限额\n");
		sprintf(errinfo, "超过单张发票限额");
		errnum = DF_TAX_ERR_CODE_EXCEED_SINGLE_LIMIT;
		break;
	case 0xEC:
		out("超过离线时长\n");
		sprintf(errinfo, "超过离线时长");
		errnum = DF_TAX_ERR_CODE_EXCEED_OFFLINE_LIMIT_TIME;
		break;
	case 0xEB:
		out("超过离线累计金额\n");
		sprintf(errinfo, "超过离线累计金额");
		errnum = DF_TAX_ERR_CODE_EXCEED_OFFLINE_LIMIT;
		break;
	case 0xEA:
		out("无此授权税率\n");
		sprintf(errinfo, "无此授权税率");
		errnum = DF_TAX_ERR_CODE_TAX_RATE_UNSUPPORTED;
		break;
	case 0xE9:
		out("数据库无此发票\n");
		sprintf(errinfo, "数据库无此发票");
		errnum = DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
		break;
	case 0xE8:
		out("未到抄报时间\n");
		sprintf(errinfo, "未到抄报时间");
		errnum = DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
		break;
	case 0xE7:
		out("无需监控信息回传\n");
		sprintf(errinfo, "无需监控信息回传");
		errnum = DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
		break;
	case 0xE6:
		out("记录超出离线票范围\n");
		sprintf(errinfo, "记录超出离线票范围");
		errnum = DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
		break;
	case 0xE5:
		out("此票已作废\n");
		sprintf(errinfo, "此票已作废");
		errnum = DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
		break;
	case 0xE4:
		out("不能作废隔月票\n");
		sprintf(errinfo, "不能作废隔月票");
		errnum = DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
		break;
	case 0xE3:
		out("系统时钟错误\n");
		sprintf(errinfo, "系统时钟错误");
		errnum = DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
		break;
	case 0xE2:
		out("未抄报，不能进行监控信息回传\n");
		sprintf(errinfo, "未抄报，不能进行监控信息回传");
		errnum = DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
		break;
	case 0xE1:
		out("监控信息回传，开票截至日期应往后顺延\n");
		sprintf(errinfo, "监控信息回传，开票截至日期应往后顺延");
		errnum = DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
		break;
	case 0xE0:
		out("有作废发票未上传，先上传作废发票\n");
		sprintf(errinfo, "有作废发票未上传，先上传作废发票");
		errnum = DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
		break;	
	case 0xDF:
		out("有离线票未上传，请先上传离线票\n");
		sprintf(errinfo, "有离线票未上传，请先上传离线票");
		errnum = DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
		break;
	case 0xDE:
		out("占未支持该命令\n");
		sprintf(errinfo, "占未支持该命令");
		errnum = DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
		break;
	case 0xDD:
		out("有红票未上传\n");
		sprintf(errinfo, "有红票未上传");
		errnum = DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
		break;
	case 0xDC:
		out("空白废不支持作废\n");
		sprintf(errinfo, "空白废不支持作废");
		errnum = DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
		break;
	default :
		out("错误代码未定义\n");
		sprintf(errinfo, "模拟盘错误代码未定义");
		errnum = DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
		break;
	}
	return errnum;
}

int usb_data_write(HUSB husb, unsigned char *indata, int in_len)
{
	int result;
	int i;
	int num;
	int size;
	unsigned char *r_data;

	r_data = (uint8 *)malloc(1000000);
	memset(r_data, 0, sizeof(1000000));
	num = in_len / 64 + 1;

	//out("发送数据,num= %d,in_len=%d\n", num, in_len);
	//out("发送数据:");
	//printf_array((char *)indata, in_len);
	for (i = 0; i < num; i++)
	{
		result = libusb_interrupt_transfer(husb->handle, 0x01, indata + i * 64, 64, &size, 4000);
		if (result < 0)
		{
			out("数据发送失败,result = %d\n", result);
			free(r_data);
			return -1;
		}

	}
	return 0;
}

int usb_data_read(HUSB husb, unsigned char *outdata, int *out_len)
{
	int result;
	int i;
	int num;
	int size;
	int total_len;
	int left_len;
	int index = 0;
	unsigned char s_data[50];
	unsigned char *r_data;
	uint16 crc_v;
	uint16 crc;

	r_data = (uint8 *)malloc(1000000);

	//out("准备进行数据接收\n");
	result = libusb_interrupt_transfer(husb->handle, 0x82, r_data, 64, &size, 1000);
	if (result < 0)
	{
		out("数据接收失败,result = %d\n", result);
		free(r_data);
		return -2;
	}
	if ((r_data[0] != 0x4d) || (r_data[1] != 0x42))
	{
		out("版本号错误\n");
		free(r_data);
		return -3;
	}
	total_len = r_data[2] * 0x100 + r_data[3];
	left_len = total_len - 22;
	//out("total_len = %d\n", total_len);
	//out("left_len = %d\n", left_len);
	if (total_len > *out_len)
	{
		out("接收缓冲去内存太小\n");
		free(r_data);
		return -4;
	}
	num = (left_len / 22) + 1;

	for (i = 0; i<num; i++)
	{
		memset(s_data, 0, sizeof(s_data));
		libusb_interrupt_transfer(husb->handle, 0x82, s_data, 22, &size, 1000);
		memcpy(r_data + (22 * (i + 1)), s_data, size);
		index += size;
	}
	crc = crc_8005(r_data, total_len - 2, 0);
	crc_v = r_data[total_len - 2] * 256 + r_data[total_len - 1];
	if (crc != crc_v)
	{
		out("crc校验错误\n");
		free(r_data);
		return -5;
	}
	//out("应答数据：");
	//printf_array((char *)r_data, total_len);

	*out_len = total_len;
	memcpy(outdata, r_data + 6, total_len - 8);
	free(r_data);
	r_data = NULL;
	return *out_len;
}


void usb_hid_close(HUSB husb)
{
	if (husb != NULL)
		libusb_close(husb->handle);
}