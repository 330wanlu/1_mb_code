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

	//out("��������,num= %d,in_len=%d\n",num,in_len);
	//out("��������:");
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
			out("���ݷ���ʧ��,result = %d\n",result);
			sprintf(errinfo, "ģ���̵ײ�����д�볬ʱʧ��");
			free(r_data);
			return DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
		}
		
	}
	//out("׼���������ݽ���\n");	
	result = libusb_interrupt_transfer(husb->handle, 0x82, r_data, 64, &size, 2000);
	if (result < 0)
	{
		out("���ݽ���ʧ��,result = %d\n",result);
		sprintf(errinfo, "ģ���̵ײ�����Ӧ��ʱʧ��");
		free(r_data);
		return DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
	}
	if ((r_data[0] != 0x4d) || (r_data[1] != 0x42))
	{
		out("�汾�Ŵ���\n");
		sprintf(errinfo, "ģ���̵ײ�����Ӧ��汾�Ŵ���");
		free(r_data);
		return DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
	}
	total_len = r_data[2] * 0x100 + r_data[3];
	left_len = total_len - 22;
	//out("total_len = %d\n", total_len);
	//out("left_len = %d\n", left_len);
	if (total_len > *out_len)
	{
		out("���ջ���ȥ�ڴ�̫С\n");
		sprintf(errinfo, "ģ���̵ײ����ݽ��ջ�����̫С");
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
		out("crcУ�����\n");
		sprintf(errinfo,"ģ���̵ײ�Ӧ��CRC����У�����");
		free(r_data);
		return DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
	}
	//out("Ӧ�����ݣ�");
	//printf_array((char *)r_data, total_len);

	if (r_data[4] != 0)
	{
		out("USB����Ӧ��Ϊ�������,%02x\n",r_data[4]);

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
		out("δ����\n");
		sprintf(errinfo, "δ����");
		errnum = DF_TAX_ERR_CODE_UNSUPPORT_INV_TYPE;
		break;
	case 0xFE:
		out("ʱ���ʽ����\n");
		sprintf(errinfo, "ʱ���ʽ����");
		errnum = DF_TAX_ERR_CODE_UNSUPPORT_INV_TYPE;
		break;
	case 0xFD:
		out("����Ų����Ϲ淶\n");
		sprintf(errinfo, "����Ų����Ϲ淶");
		errnum = DF_TAX_ERR_CODE_UNSUPPORT_INV_TYPE;
		break;
	case 0xFC:
		out("δע��\n");
		sprintf(errinfo, "δע��");
		errnum = DF_TAX_ERR_CODE_UNSUPPORT_INV_TYPE;
		break;
	case 0xFB:
		out("δ����\n");
		sprintf(errinfo, "δ����");
		errnum = DF_TAX_ERR_CODE_UNSUPPORT_INV_TYPE;
		break;
	case 0xFA:
		out("�˿��Ѹ�ʽ�����������ϵ�\n");
		sprintf(errinfo, "�˿��Ѹ�ʽ�����������ϵ�");
		errnum = DF_TAX_ERR_CODE_UNSUPPORT_INV_TYPE;
		break;
	case 0xF9:
		out("��������,�����Ѿ�����\n");
		sprintf(errinfo, "��������,�����Ѿ�����");
		errnum = DF_TAX_ERR_CODE_UNSUPPORT_INV_TYPE;
		break;
	case 0xF8:
		out("�޴˷�Ʊ����\n");
		sprintf(errinfo, "�޴˷�Ʊ����");
		errnum = DF_TAX_ERR_CODE_UNSUPPORT_INV_TYPE;
		break;
	case 0xF7:
		out("�޿��÷�Ʊ\n");
		sprintf(errinfo, "�޿��÷�Ʊ");
		errnum = DF_TAX_ERR_CODE_HAVE_NO_INVOICE;
		break;
	case 0xF6:
		out("�������\n");
		sprintf(errinfo, "�������");
		errnum = DF_TAX_ERR_CODE_PLATE_CERT_PASSWD_ERR;
		break;
	case 0xF5:
		out("����֤�����Ϊ8λ\n");
		sprintf(errinfo, "����֤�����Ϊ8λ");
		errnum = DF_TAX_ERR_CODE_PLATE_CERT_PASSWD_ERR;
		break;
	case 0xF4:
		out("�÷�Ʊ���ʹ����Ѵ���\n");
		sprintf(errinfo, "�÷�Ʊ���ʹ����Ѵ���");
		errnum = DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
		break;
	case 0xF3:
		out("����Ȩ˰���Ѵ���\n");
		sprintf(errinfo, "����Ȩ˰���Ѵ���");
		errnum = DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
		break;
	case 0xF2:
		out("�÷�Ʊ�δ����ظ�\n");
		sprintf(errinfo, "�÷�Ʊ�δ����ظ�");
		errnum = DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
		break;
	case 0xF1:
		out("��Ʊ�������\n");
		sprintf(errinfo, "��Ʊ�������");
		errnum = DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
		break;
	case 0xF0:
		out("�޴˷�Ʊ��\n");
		sprintf(errinfo, "�޴˷�Ʊ��");
		errnum = DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
		break;
	case 0xEF:
		out("������λ�Ǳ�����\n");
		sprintf(errinfo, "������λ�Ǳ�����");
		errnum = DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
		break;
	case 0xEE:
		out("������Ʊ��������\n");
		sprintf(errinfo, "������Ʊ��������");
		errnum = DF_TAX_ERR_CODE_EXCEED_KP_END_DATE;
		break;
	case 0xED:
		out("�������ŷ�Ʊ�޶�\n");
		sprintf(errinfo, "�������ŷ�Ʊ�޶�");
		errnum = DF_TAX_ERR_CODE_EXCEED_SINGLE_LIMIT;
		break;
	case 0xEC:
		out("��������ʱ��\n");
		sprintf(errinfo, "��������ʱ��");
		errnum = DF_TAX_ERR_CODE_EXCEED_OFFLINE_LIMIT_TIME;
		break;
	case 0xEB:
		out("���������ۼƽ��\n");
		sprintf(errinfo, "���������ۼƽ��");
		errnum = DF_TAX_ERR_CODE_EXCEED_OFFLINE_LIMIT;
		break;
	case 0xEA:
		out("�޴���Ȩ˰��\n");
		sprintf(errinfo, "�޴���Ȩ˰��");
		errnum = DF_TAX_ERR_CODE_TAX_RATE_UNSUPPORTED;
		break;
	case 0xE9:
		out("���ݿ��޴˷�Ʊ\n");
		sprintf(errinfo, "���ݿ��޴˷�Ʊ");
		errnum = DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
		break;
	case 0xE8:
		out("δ������ʱ��\n");
		sprintf(errinfo, "δ������ʱ��");
		errnum = DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
		break;
	case 0xE7:
		out("��������Ϣ�ش�\n");
		sprintf(errinfo, "��������Ϣ�ش�");
		errnum = DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
		break;
	case 0xE6:
		out("��¼��������Ʊ��Χ\n");
		sprintf(errinfo, "��¼��������Ʊ��Χ");
		errnum = DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
		break;
	case 0xE5:
		out("��Ʊ������\n");
		sprintf(errinfo, "��Ʊ������");
		errnum = DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
		break;
	case 0xE4:
		out("�������ϸ���Ʊ\n");
		sprintf(errinfo, "�������ϸ���Ʊ");
		errnum = DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
		break;
	case 0xE3:
		out("ϵͳʱ�Ӵ���\n");
		sprintf(errinfo, "ϵͳʱ�Ӵ���");
		errnum = DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
		break;
	case 0xE2:
		out("δ���������ܽ��м����Ϣ�ش�\n");
		sprintf(errinfo, "δ���������ܽ��м����Ϣ�ش�");
		errnum = DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
		break;
	case 0xE1:
		out("�����Ϣ�ش�����Ʊ��������Ӧ����˳��\n");
		sprintf(errinfo, "�����Ϣ�ش�����Ʊ��������Ӧ����˳��");
		errnum = DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
		break;
	case 0xE0:
		out("�����Ϸ�Ʊδ�ϴ������ϴ����Ϸ�Ʊ\n");
		sprintf(errinfo, "�����Ϸ�Ʊδ�ϴ������ϴ����Ϸ�Ʊ");
		errnum = DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
		break;	
	case 0xDF:
		out("������Ʊδ�ϴ��������ϴ�����Ʊ\n");
		sprintf(errinfo, "������Ʊδ�ϴ��������ϴ�����Ʊ");
		errnum = DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
		break;
	case 0xDE:
		out("ռδ֧�ָ�����\n");
		sprintf(errinfo, "ռδ֧�ָ�����");
		errnum = DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
		break;
	case 0xDD:
		out("�к�Ʊδ�ϴ�\n");
		sprintf(errinfo, "�к�Ʊδ�ϴ�");
		errnum = DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
		break;
	case 0xDC:
		out("�հ׷ϲ�֧������\n");
		sprintf(errinfo, "�հ׷ϲ�֧������");
		errnum = DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
		break;
	default :
		out("�������δ����\n");
		sprintf(errinfo, "ģ���̴������δ����");
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

	//out("��������,num= %d,in_len=%d\n", num, in_len);
	//out("��������:");
	//printf_array((char *)indata, in_len);
	for (i = 0; i < num; i++)
	{
		result = libusb_interrupt_transfer(husb->handle, 0x01, indata + i * 64, 64, &size, 4000);
		if (result < 0)
		{
			out("���ݷ���ʧ��,result = %d\n", result);
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

	//out("׼���������ݽ���\n");
	result = libusb_interrupt_transfer(husb->handle, 0x82, r_data, 64, &size, 1000);
	if (result < 0)
	{
		out("���ݽ���ʧ��,result = %d\n", result);
		free(r_data);
		return -2;
	}
	if ((r_data[0] != 0x4d) || (r_data[1] != 0x42))
	{
		out("�汾�Ŵ���\n");
		free(r_data);
		return -3;
	}
	total_len = r_data[2] * 0x100 + r_data[3];
	left_len = total_len - 22;
	//out("total_len = %d\n", total_len);
	//out("left_len = %d\n", left_len);
	if (total_len > *out_len)
	{
		out("���ջ���ȥ�ڴ�̫С\n");
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
		out("crcУ�����\n");
		free(r_data);
		return -5;
	}
	//out("Ӧ�����ݣ�");
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