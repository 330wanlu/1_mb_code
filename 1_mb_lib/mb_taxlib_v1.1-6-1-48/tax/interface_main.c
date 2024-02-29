#define interface_main
#include "interface_main.h"

int ConsoleClose(int nDeviceType, HUSB hUsb)
{
    if (DEVICE_AISINO == nDeviceType)
        AisinoClose(hUsb);
    return RET_SUCCESS;
}

int DetectDeviceInit(HDEV hDev, HUSB hUSB, uint8 bAuthDev, int nBusID, int nDevID,char *busid)
{
	int nRet = -1;
	if (!hDev || !hUSB)
	{
		libusb_exit(hUSB->context);
		_WriteLog(LL_INFO, "[-] DetectDeviceInit !hDev || !hUSB\n");
		return -1;
	}	
	//_WriteLog(LL_INFO, "[+] Open usb device\n");
	
	int nhUsbType = usb_device_open(hUSB, nBusID, nDevID);
	if (nhUsbType <= 0) {
		_WriteLog(LL_INFO, "[-] Usb device open failed,nhUsbType = %d\n", nhUsbType);
		//report_event(NULL, "USB��ʧ��", "libusbͨ��busid��devid���豸ʧ��", nhUsbType);
		return -2;
	}

	if (DEVICE_AISINO == nhUsbType) {  // aisino init

		//nRet = LoadAisinoDevInfo(hDev, hUSB, false);
		hDev->bDeviceType = nhUsbType;
		CntaxEntry(hUSB, NOP_CLOSE);  //�ر������´�
		nRet = LoadAisinoDevInfo(hDev, hUSB, bAuthDev);
		if (nRet < 0) {
			//report_event(NULL, "��˰�̼��ػ�����Ϣʧ��", "��˰�̼��ػ�����Ϣʧ��,���ܴ��ڼ�������", nRet);
			_WriteLog(LL_WARN, "[+] LoadCntaxDevInfo Init device to open failed, nRet:%d\n", nRet);
			if (hUSB->handle)
				usb_device_close(hUSB);
			libusb_exit(hUSB->context);
			return ERR_DEVICE_PASSWORD_ERROR;
		}
	}
	else if (DEVICE_NISEC == nhUsbType)  {  // nisec init
		//_WriteLog(LL_INFO, "[+] Nisec device found\n");
		//out("Nisec device found\n");
		//������ʵ���ùرյģ�����Ϊ�˱�֤ÿ�ζ����µ�״̬�����ǹر������´򿪱ȽϺ�,�п���֮ǰû�����ٹؾͲ��ɹ�����˲��������ж�
		//_WriteLog(LL_WARN, "���豸ǰ�ȹؿ�\n");
		//NisecEntry(hUSB, NOP_CLOSE);
		//_WriteLog(LL_WARN, "���豸ǰ�ȹؿ�,�ؿ����\n");
		nRet = LoadNisecDevInfo(hDev, hUSB,bAuthDev);
		if (nRet < 0) {
			//report_event(NULL, "˰���̼��ػ�����Ϣʧ��", "˰���̼��ػ�����Ϣʧ��,���ܴ��ڼ�������", nRet);
			_WriteLog(LL_WARN, "[+] LoadNisecDevInfo Init device to open failed, nRet:%d\n", nRet);
			if (hUSB->handle)
				usb_device_close(hUSB);
			libusb_exit(hUSB->context);
			return -5;
		}
		//_WriteLog(LL_DEBUG, "[+] LoadNisecDevInfo hDev %s\n", hDev->szTaxAuthorityCodeEx);
	}
	else if (nhUsbType == DEVICE_CNTAX)
	{
		//_WriteLog(LL_INFO, "[+] Cntax-UKey device found");
		hDev->bDeviceType = nhUsbType;
		CntaxEntry(hUSB, NOP_CLOSE);  //�ر������´�
		nRet = LoadCntaxDevInfo(hDev, hUSB, bAuthDev);
		if (nRet < 0) {
			//report_event(NULL, "˰��UKEY���ػ�����Ϣʧ��", "˰��UKEY���ػ�����Ϣʧ��,���ܴ��ڼ�������", nRet);
			_WriteLog(LL_WARN, "[+] LoadCntaxDevInfo Init device to open failed, nRet:%d\n", nRet);
			if (hUSB->handle)
				usb_device_close(hUSB);
			libusb_exit(hUSB->context);
			return ERR_DEVICE_PASSWORD_ERROR;
		}
		//_WriteLog(LL_DEBUG, "[+] Load cntax device successful!\n");
	}
	else if (DEVICE_SKP_KEY == nhUsbType)  {  // nisec init
		_WriteLog(LL_INFO, "[+] �����豸 device found\n");
		//out("Nisec device found\n");
		//������ʵ���ùرյģ�����Ϊ�˱�֤ÿ�ζ����µ�״̬�����ǹر������´򿪱ȽϺ�,�п���֮ǰû�����ٹؾͲ��ɹ�����˲��������ж�
		LoadSkpKeyDevInfo(hDev, hUSB);
	}
	return nhUsbType;
}

static int usb_device_open_serssion(HUSB *h_USB, HDEV *h_Dev, uint8 bAuthDev, int nBusID, int nDevID,char *busid,char *lasterror)
{
	if ((nBusID<1) || (nBusID>127))
	{
		printf("nBusNum range error,nBusID = %d\n", nBusID);
		*h_USB = NULL;
		*h_Dev = NULL;
		return -1;
	}
	if ((nDevID<1) || (nDevID>127))
	{
		printf("nDevNum range error,nDevID = %d\n", nDevID);
		*h_USB = NULL;
		*h_Dev = NULL;
		return -2;
	}
	int nDeviceType = 0;
	HUSB hUSB = MallocUSBSession();
	HDEV hDev = MallocDevInfo();
	//out("usb_device_open_serssion enter\n");
	nDeviceType = DetectDeviceInit(hDev, hUSB, bAuthDev, nBusID, nDevID,busid);
	//out("DetectDeviceInit success\n");
	if (nDeviceType < 0) {
		_WriteLog(LL_INFO, "[-] No support device found\n");
		strcpy(lasterror, hUSB->errinfo);
		_WriteLog(LL_INFO, "usb_device_open_serssion DetectDeviceInit nDeviceType = %d,lasterror = %s\n", nDeviceType, lasterror);
		if (hUSB != NULL)
			free(hUSB);
		if (hDev != NULL)
			free(hDev);
		*h_USB = NULL;
		*h_Dev = NULL;
		return nDeviceType;
	}
	*h_USB = hUSB;
	*h_Dev = hDev;
	hDev->hUSB = hUSB;
	return nDeviceType;
}

static int usb_hid_close_serssion(HUSB hUSB, HDEV hDev)
{
	if (!hUSB)
		return 0;
	if (hUSB->handle)
		usb_hid_close(hUSB);
	hUSB->handle = NULL;
	libusb_exit(hUSB->context);
	hUSB->context = NULL;
	free(hUSB);
	hUSB = NULL;
	FreeDevInfo(hDev);
	return 0;
}

static int usb_hid_open_serssion(HUSB *h_USB, HDEV *h_Dev, int nBusID, int nDevID, char *busid,char *lasterror)
{
	if ((nBusID<1) || (nDevID>127))
	{
		//printf("nBusNum range error\n");
		*h_USB = NULL;
		*h_Dev = NULL;
		return -1;
	}
	if ((nBusID<1) || (nDevID>127))
	{
		//printf("nDevNum range error\n");
		*h_USB = NULL;
		*h_Dev = NULL;
		return -2;
	}
	HUSB hUSB = MallocUSBSession();
	HDEV hDev = MallocDevInfo();
	int nhUsbType = usb_device_open_hid(hUSB, nBusID, nDevID);


	if (nhUsbType == DEVICE_MENGBAI)
	{
		if (LoadMengBaiInfo(hUSB, hDev) < 0)
		{
			out("�豸�򿪳ɹ�,����ʧ��\n");
			usb_hid_close_serssion(hUSB, hDev);
			*h_USB = NULL;
			*h_Dev = NULL;
			return -3;
		}
	}
	else if (nhUsbType == DEVICE_MENGBAI2)
	{
		if (LoadMengBaiSeverKey(hUSB, hDev) < 0)
		{
			out("�豸�򿪳ɹ�,����ʧ��\n");
			usb_hid_close_serssion(hUSB, hDev);
			*h_USB = NULL;
			*h_Dev = NULL;
			return -4;
		}
	}
	else
	{
		out("�豸��ʧ��,result = %d\n", nhUsbType);
		//usb_hid_close_serssion(hUSB, hDev);
		libusb_exit(hUSB->context);
		free(hUSB);
		FreeDevInfo(hDev);
		*h_USB = NULL;
		*h_Dev = NULL;
		return -5;
	}

	*h_USB = hUSB;
	*h_Dev = hDev;
	hDev->hUSB = hUSB;
	return 0;
}



static int usb_device_close_serssion(HUSB hUSB, HDEV hDev, int nDeviceType)
{
	FreeUSBSession(hUSB);
	FreeDevInfo(hDev);
	return 0;
}

int function_common_init_tax_lib(uint8 *key_zlib_json, char *taxlib_version)
{
	return init_tax_lib(key_zlib_json, taxlib_version);
}

int function_common_reload_tax_auth(uint8 *key_zlib_json)
{
	return reload_tax_auth(key_zlib_json);
}


int function_common_jude_plate_auth(uint8 bDeviceType, char *busid,char *errinfo)
{
	return jude_plate_auth(bDeviceType, busid,errinfo);
}

int function_common_find_inv_type_mem(uint8 type, int *mem_i)
{
	find_inv_type_mem(type, mem_i);
	return 0;
}

int function_common_get_cert_json_file(uint8 *key_zlib_json, char **cert_json)
{
	return get_cert_json_file(key_zlib_json, cert_json);
}

int function_common_report_event(char *plate_num, char *err, char *errinfo, int result)
{
	return report_event(plate_num, err, errinfo, result);
}

int function_common_analyze_json_buff(uint8 bDeviceType, char *json_data, HFPXX stp_fpxx, char *errinfo)
{
	stp_fpxx->isHzxxb = false;
	return AnalyzeJsonBuff(bDeviceType, json_data, stp_fpxx, errinfo);
}

int function_common_get_basic_simple(struct HandlePlate *h_plate, struct _plate_infos *plate_info)
{
	int result;
	int nDeviceType;
	int usb_type = h_plate->usb_type;
	int nBusID = h_plate->nBusID;
	int nDevID = h_plate->nDevID;
	char last_error[1024] = { 0 };
	//int left_num = -1;
	//writelog(LL_INFO,"taxlib","���ػ�����Ϣ","��ȡ˰�̻�����Ϣ�������Ϣ");
	//_WriteLog(0,"������־���\n");
	if (usb_type == DEVICE_MENGBAI)
	{
		result = usb_hid_open_serssion(&h_plate->hUSB, &h_plate->hDev, nBusID, nDevID, h_plate->busid, last_error);
		if (result < 0)
		{
			return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
		}
		h_plate->need_free = 1;
		return 0;
	}
	else if (usb_type == DEVICE_MENGBAI2)
	{
		result = usb_hid_open_serssion(&h_plate->hUSB, &h_plate->hDev, nBusID, nDevID, h_plate->busid, last_error);
		if (result < 0)
		{
			return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
		}
		h_plate->need_free = 1;
		return 0;
	}
	else if (usb_type == DEVICE_NISEC)
	{
		nDeviceType = usb_device_open_serssion(&h_plate->hUSB, &h_plate->hDev, true, nBusID, nDevID, h_plate->busid, last_error);
		if (nDeviceType < 0)
		{
			return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
		}
		h_plate->need_free = 1;
		return 0;
	}
	else if (usb_type == DEVICE_AISINO)
	{
		nDeviceType = usb_device_open_serssion(&h_plate->hUSB, &h_plate->hDev, false, nBusID, nDevID, h_plate->busid, last_error);
		if (nDeviceType < 0)
		{
			return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
		}
		if (nDeviceType == DEVICE_AISINO)
		{
			//AisinoClose(h_plate->hUSB);
			if (AisinoOpen(h_plate->hUSB) < 0) {
				_WriteLog(LL_WARN, "[+] function_common_get_basic_simple Init device to open failed\n");
				usb_device_close_serssion(h_plate->hUSB, h_plate->hDev, nDeviceType);
				return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
			}
		}
		else if (nDeviceType == DEVICE_CNTAX)
		{
			h_plate->usb_type = nDeviceType;
			_WriteLog(LL_WARN, "����Ϊ��˰���ͺ�˰��ukey\n");
		}
		h_plate->need_free = 1;
		return 0;
	}
	else if (usb_type == DEVICE_CNTAX)
	{

		nDeviceType = usb_device_open_serssion(&h_plate->hUSB, &h_plate->hDev, false, nBusID, nDevID, h_plate->busid, last_error);
		if (nDeviceType < 0)
		{
			return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
		}
		h_plate->need_free = 1;
		return 0;
	}
	else if (usb_type == DEVICE_SKP_KEY)
	{

		nDeviceType = usb_device_open_serssion(&h_plate->hUSB, &h_plate->hDev, false, nBusID, nDevID, h_plate->busid, last_error);
		if (nDeviceType < 0)
		{
			return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
		}
		h_plate->need_free = 1;
		return 0;
	}
	else
	{
		return -1;
	}
	return 0;
}


int function_common_get_basic_tax_info(struct HandlePlate *h_plate, struct _plate_infos *plate_info)
{
	int result;
	int nDeviceType;
	int usb_type = h_plate->usb_type;
	int nBusID = h_plate->nBusID;
	int nDevID = h_plate->nDevID;
	char last_error[1024] = { 0 };
	//int left_num = -1;
	//writelog(LL_INFO,"taxlib","���ػ�����Ϣ","��ȡ˰�̻�����Ϣ�������Ϣ");
	//_WriteLog(0,"������־���\n");
	if (usb_type == DEVICE_MENGBAI)
	{
		plate_info->plate_basic_info.dev_passwd_right = 1;
		result = usb_hid_open_serssion(&h_plate->hUSB, &h_plate->hDev, nBusID, nDevID, h_plate->busid, last_error);
		if (result < 0)
		{
			out("�豸��ʧ��,result = %d\n", result);
			return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
		}
		//out("�����豸�ɹ�\n");
		//out("[+] Open device successful!\n");
		result = mengbai_read_basic_info(h_plate->hDev, plate_info);
		if (result< 0)
		{
			printf("[-] Read ID failed\n");
			usb_hid_close_serssion(h_plate->hUSB, h_plate->hDev);
			return result;
		}
		plate_info->plate_basic_info.cert_passwd_right = 1;
		strcpy((char *)plate_info->plate_tax_info.plate_time, h_plate->hDev->szDeviceTime);
		//out("��ȡ������Ϣ�ɹ�\n");
		//out("[+] Open device successful!\n");
		result = mengbai_read_inv_type_areacode(h_plate->hUSB, plate_info);
		if (result < 0)
		{
			printf("[-] Read area code failed\n");
			usb_hid_close_serssion(h_plate->hUSB, h_plate->hDev);
			return result;
		}
		//out("��ȡ�������ɹ�\n");
		result = mengbai_read_monitor_info(h_plate->hUSB, plate_info->plate_tax_info.inv_type, plate_info);
		if (result < 0)
		{
			printf("[-] Read state info failed\n");
			usb_hid_close_serssion(h_plate->hUSB, h_plate->hDev);
			return result;
		}
		//out("��ȡ�����Ϣ�ɹ�\n");
		result = mengbai_read_inv_coil_number(h_plate->hUSB, plate_info->plate_tax_info.inv_type, plate_info);
		if (result < 0)
		{
			printf("[-] Read coil number failed\n");
			usb_hid_close_serssion(h_plate->hUSB, h_plate->hDev);
			return result;
		}
		//out("��ȡ��Ʊ�����Ϣ�ɹ�\n");
		result = mengbai_read_offline_inv_num(h_plate->hUSB, plate_info->plate_tax_info.inv_type, plate_info);
		if (result < 0)
		{
			printf("[-] Read offline_inv_num failed\n");
			usb_hid_close_serssion(h_plate->hUSB, h_plate->hDev);
			return result;
		}
		plate_info->plate_basic_info.use_downgrade_version = 0;
		strcpy(plate_info->plate_basic_info.minimum_version, "V0.0.00.000000");
		//out("��ȡ���߷�Ʊ�����ɹ�\n");
		//out("��ģ���̡���ȡ������Ʊ����Ϊ%d\n",result);
		plate_info->plate_tax_info.off_inv_num = result;
		plate_info->plate_basic_info.plate_test = 1;
		h_plate->need_free = 1;
		//out("������ȡ�����Ϣ\n");
		return 0;
	}
	else if (usb_type == DEVICE_MENGBAI2)
	{
		plate_info->plate_basic_info.dev_passwd_right = 1;
		result = usb_hid_open_serssion(&h_plate->hUSB, &h_plate->hDev, nBusID, nDevID, h_plate->busid, last_error);
		if (result < 0)
		{
			out("�豸��ʧ��,result = %d\n", result);
			return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
		}
		memcpy(plate_info->plate_basic_info.ca_number, h_plate->hDev->szCommonTaxID, strlen(h_plate->hDev->szCommonTaxID));
		memcpy(plate_info->plate_basic_info.ca_name, h_plate->hDev->szCompanyName, strlen(h_plate->hDev->szCompanyName));
		memcpy(plate_info->plate_basic_info.plate_num, h_plate->hDev->szDeviceID, strlen(h_plate->hDev->szDeviceID));
		plate_info->plate_basic_info.result = 1;
		plate_info->plate_basic_info.plate_type = DEVICE_MENGBAI2;
		plate_info->plate_basic_info.extension = h_plate->hDev->uICCardNo;
		plate_info->plate_basic_info.cert_passwd_right = 1;
		h_plate->need_free = 1;
		//out("������ȡ�����Ϣ\n");
		return 0;
	}
	else if (usb_type == DEVICE_NISEC)
	{
		plate_info->plate_basic_info.dev_passwd_right = 1;
		nDeviceType = usb_device_open_serssion(&h_plate->hUSB, &h_plate->hDev, true, nBusID, nDevID, h_plate->busid,last_error);
		//_WriteLog(LL_WARN, "USB�豸�򿪳ɹ�\n");
		//out("usb_device_open_serssion success ���ߺţ�%d���豸�ţ�%d\n", nBusID, nDevID);
		if (nDeviceType < 0)
		{
			_WriteLog(LL_WARN, "˰�����豸��ʼ����ʧ��,���������Ϊ%s\n", last_error);
			if (strstr(last_error, "09d1c") != NULL)
			{
				char devpass_leftcount[10] = { 0 };
				char errinfo[1024] = { 0 };

				char *tmp = strstr(last_error, "09d1c");
				tmp += strlen("09d1c");

				memcpy(devpass_leftcount, tmp, 1);
				sprintf(errinfo, "��Ĭ���豸����88888888,ʣ�ೢ�Դ���%s", devpass_leftcount);
				logout(INFO, "TAXLIB", "˰�̶�ȡ", "�ϱ�˰�̣�˰����,������Ϣ��%s\r\n", errinfo);
				//report_event(NULL, "˰�����豸��������", errinfo, nDeviceType);
				plate_info->plate_basic_info.plate_type = DEVICE_NISEC;
				plate_info->plate_basic_info.dev_passwd_right = 0;
				plate_info->plate_basic_info.dev_left_num = atoi(devpass_leftcount);
				plate_info->plate_basic_info.dev_err_reason = ERR_DEVICE_PASSWORD_ERROR;
			}
			else if (strstr(last_error, "09d10b") != NULL)
			{
				char errinfo[1024] = { 0 };
				sprintf(errinfo, "�豸����pin����(09d10b),ʣ�ೢ�Դ���0");
				logout(INFO, "TAXLIB", "˰�̶�ȡ", "�ϱ�˰�̣�˰����,������Ϣ��%s\r\n", errinfo);
				//report_event(NULL, "˰�����豸��������", errinfo, nDeviceType);
				plate_info->plate_basic_info.plate_type = DEVICE_NISEC;
				plate_info->plate_basic_info.dev_passwd_right = 0;
				plate_info->plate_basic_info.dev_left_num = 0;
				plate_info->plate_basic_info.dev_err_reason = ERR_DEVICE_PASSWORD_LOCK;
			}
			return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
		}

		if (strlen(h_plate->hDev->szPubServer) != 0)
		{
			sprintf((char *)plate_info->plate_basic_info.used_ofd, "1");
		}
		else
		{
			sprintf((char *)plate_info->plate_basic_info.used_ofd, "0");
		}

		memcpy(plate_info->plate_basic_info.server_type, h_plate->hDev->szPubServer, 2);
		strcpy((char *)plate_info->plate_basic_info.server_number, h_plate->hDev->szPubServer + 2);

		result = nisec_get_basic_info(h_plate->hDev, plate_info);
		if (result < 0)
		{
			printf("[-]  nisec_get_basic_info failed result = %d\n", result);
		}
		//_WriteLog(LL_WARN, "������Ϣ��ȡ�ɹ�\n");
		
		result = nisec_get_state_info(h_plate->hDev, plate_info);
		if (result < 0)
		{
			printf("[-] nisec_get_state_info failed result = %d\n", result);
		}
		//_WriteLog(LL_WARN, "�����Ϣ��ȡ���\n");
		result = nisec_read_inv_coil_number(h_plate->hDev, plate_info);
		if (result < 0)
		{
			printf("[-] nisec_read_inv_coil_number failed result = %d\n", result);
		}
		//_WriteLog(LL_WARN, "��Ʊ����Ϣ��ȡ���\n");
#ifndef DF_OLD_MSERVER
		nisec_update_summary_data(h_plate->hDev, plate_info);
#endif
	/*	result = nisec_get_state_info_and_coil_info(h_plate->hDev,h_plate->hUSB, plate_info);
		_WriteLog(LL_WARN, "��Ʊ����Ϣ��ȡ���\n");*/
		TranslateStandTime(2, h_plate->hDev->szDeviceTime, (char *)plate_info->plate_tax_info.plate_time);
		//_WriteLog(LL_WARN, "��ȡ����ʱ�䣺%s��load��ȡ��ʱ�䣺%s\n", plate_info->plate_tax_info.plate_time, h_plate->hDev->szDeviceTime);
		strcpy((char *)plate_info->plate_tax_info.driver_ver,h_plate->hDev->szDriverVersion);
		int off_num;
		off_num = nisec_is_exsist_offline_inv(h_plate->hDev, (char *)plate_info->plate_tax_info.off_inv_fpdm, (char *)plate_info->plate_tax_info.off_inv_fphm, (char *)plate_info->plate_tax_info.off_inv_kpsj, &plate_info->plate_tax_info.off_inv_dzsyh);
		if (off_num <= 0)
		{
			//out("�����Ϣ��ȡ���в��������߷�Ʊ\n");
			plate_info->plate_tax_info.off_inv_num = 0;
		}
		else
		{
			plate_info->plate_tax_info.off_inv_num = 1;
		}
		//_WriteLog(LL_WARN, "��ȡ���߷�Ʊ�������\n");
		memcpy(plate_info->plate_tax_info.startup_date, h_plate->hDev->szDeviceEffectDate, 8);
		memcpy(plate_info->plate_tax_info.tax_office_name, h_plate->hDev->szTaxAuthorityName, strlen(h_plate->hDev->szTaxAuthorityName));
		memcpy(plate_info->plate_tax_info.tax_office_code, h_plate->hDev->szTaxAuthorityCode, 11);//��׼Ϊ11λ��˰��������ӵڶ�λ��ʼ

		struct UploadAddrModel model;
		memset(&model,0,sizeof(struct UploadAddrModel));
		GetTaServerURL(TAADDR_UPLOAD_RSA, h_plate->hDev->szRegCode, &model);
		strcpy((char *)plate_info->plate_tax_info.tax_server_url, model.szTaxAuthorityURL);

		memset(&model, 0, sizeof(struct UploadAddrModel));
		GetTaServerURL(TAADDR_CONFIRM, h_plate->hDev->szRegCode, &model);
		strcpy((char *)plate_info->plate_tax_info.integrated_server_url, model.szTaxAuthorityURL);

		memcpy(plate_info->plate_tax_info.area_code, h_plate->hDev->szRegCode, strlen(h_plate->hDev->szRegCode));
		switch (h_plate->hDev->bNatureOfTaxpayer) {
		case 0:
			sprintf((char *)plate_info->plate_tax_info.tax_nature, "����˰�����");
			break;
		case 1:
			sprintf((char *)plate_info->plate_tax_info.tax_nature, "С��ģ��˰��");
			break;
		case 2:
			sprintf((char *)plate_info->plate_tax_info.tax_nature, "һ����˰��");
			break;
		case 3:
			sprintf((char *)plate_info->plate_tax_info.tax_nature, "ת�Ǽ���˰��");
			break;
		default:
			//sprintf((char *)plate_infos->plate_tax_info.tax_nature, "");
			break;
		}
		plate_info->plate_basic_info.use_downgrade_version = !CheckDeviceVersionMatch(h_plate->hDev, plate_info->plate_basic_info.minimum_version);
		if (h_plate->hDev->bTrainFPDiskType != 0)
			plate_info->plate_basic_info.plate_test = 1;
		h_plate->need_free = 1;
		//out("������ȡ�����Ϣ\n");
		return 0;
	}
	else if (usb_type == DEVICE_AISINO)
	{
		plate_info->plate_basic_info.dev_passwd_right = 1;
		nDeviceType = usb_device_open_serssion(&h_plate->hUSB, &h_plate->hDev, true, nBusID, nDevID, h_plate->busid, last_error);
		if (nDeviceType < 0)
		{
			
			_WriteLog(LL_WARN, "��˰���豸��ʼ����ʧ��,���������Ϊ%s\n", last_error);
			if (strstr(last_error, "09d1c") != NULL)
			{
				char devpass_leftcount[10] = { 0 };
				char errinfo[1024] = { 0 };

				char *tmp = strstr(last_error, "09d1c");
				tmp += strlen("09d1c");

				memcpy(devpass_leftcount, tmp, 1);
				sprintf(errinfo, "��Ĭ���豸����12345678,ʣ�ೢ�Դ���%s", devpass_leftcount);
				logout(INFO, "TAXLIB", "˰�̶�ȡ", "�ϱ�˰�̣�˰��ukey,������Ϣ��%s\r\n", errinfo);
				//report_event(NULL, "˰��UKey�豸��������", errinfo, nDeviceType);
				plate_info->plate_basic_info.plate_type = DEVICE_CNTAX;
				plate_info->plate_basic_info.dev_passwd_right = 0;
				plate_info->plate_basic_info.dev_left_num = atoi(devpass_leftcount);
				plate_info->plate_basic_info.dev_err_reason = ERR_DEVICE_PASSWORD_ERROR;
			}
			else if (strstr(last_error, "09d10b") != NULL)
			{
				char errinfo[1024] = { 0 };
				sprintf(errinfo, "�豸����pin����(09d10b),ʣ�ೢ�Դ���0");
				logout(INFO, "TAXLIB", "˰�̶�ȡ", "�ϱ�˰�̣�˰��ukey,������Ϣ��%s\r\n", errinfo);
				//report_event(NULL, "˰��UKey�豸��������", errinfo, nDeviceType);
				plate_info->plate_basic_info.plate_type = DEVICE_CNTAX;
				plate_info->plate_basic_info.dev_passwd_right = 0;
				plate_info->plate_basic_info.dev_left_num = 0;
				plate_info->plate_basic_info.dev_err_reason = ERR_DEVICE_PASSWORD_LOCK;
			}
			return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
		
		}
		if (nDeviceType == DEVICE_AISINO)
		{
			////������ʵ���ùرյģ�����Ϊ�˱�֤ÿ�ζ����µ�״̬�����ǹر������´򿪱ȽϺ�
			AisinoClose(h_plate->hUSB);
			if (AisinoOpen(h_plate->hUSB) < 0) {
				_WriteLog(LL_WARN, "[+] AisinoOpen Init device to open failed\n");
				usb_device_close_serssion(h_plate->hUSB, h_plate->hDev, nDeviceType);
				h_plate->hUSB = NULL;
				return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
			}

			//out("USB load complete\n");
			memcpy(plate_info->plate_basic_info.ca_number, h_plate->hDev->szCommonTaxID, strlen(h_plate->hDev->szCommonTaxID));
			memcpy(plate_info->plate_basic_info.ca_name, h_plate->hDev->szCompanyName, strlen(h_plate->hDev->szCompanyName));
			memcpy(plate_info->plate_basic_info.plate_num, h_plate->hDev->szDeviceID, strlen(h_plate->hDev->szDeviceID));
			plate_info->plate_basic_info.result = 3;
			plate_info->plate_basic_info.plate_type = DEVICE_AISINO;
			plate_info->plate_basic_info.extension = h_plate->hDev->uICCardNo;

			if (strlen(h_plate->hDev->szPubServer) != 0)
			{
				sprintf((char *)plate_info->plate_basic_info.used_ofd, "1");
			}
			else
			{
				sprintf((char *)plate_info->plate_basic_info.used_ofd, "0");
			}

			memcpy(plate_info->plate_basic_info.server_type, h_plate->hDev->szPubServer, 2);
			strcpy((char *)plate_info->plate_basic_info.server_number, h_plate->hDev->szPubServer + 2);

			
			bool exit_unlock_inv;
			//_WriteLog(LL_INFO, "��ȡ�Ƿ����δ������Ʊ\n");
			exit_unlock_inv = ReadUnlockInvoiceFromDevice(h_plate->hDev, true);
			if (exit_unlock_inv)
			{
				_WriteLog(LL_INFO, "˰�̴���δ������Ʊ,��Ҫ���з�Ʊ����\n");
				//report_event(h_plate->hDev->szDeviceID, "δ������Ʊ����", "˰�̴���δ������Ʊ,��Ҫ���з�Ʊ����", 0);
				ReadUnlockInvoiceFromDevice(h_plate->hDev, false);
			}

			result = aisino_read_monitor_info(h_plate->hUSB, h_plate->hDev, plate_info);
			if (result < 0)
			{
				printf("[-] aisino_read_monitor_info failed result = %d\n", result);
			}

			result = aisino_read_inv_coil_number(h_plate->hUSB, plate_info);
			if (result < 0)
			{
				printf("[-] aisino_read_inv_coil_number failed result = %d\n", result);
			}
#ifndef DF_OLD_MSERVER
			aisino_update_summary_data(h_plate->hDev, plate_info);
#endif
			TranslateStandTime(2, h_plate->hDev->szDeviceTime, (char *)plate_info->plate_tax_info.plate_time);
			//out("��ȡ����ʱ�䣺%s��load��ȡ��ʱ�䣺%s\n", plate_infos->plate_tax_info.plate_time, h_Dev->szDeviceTime);

			//out("��ȡ˰��ʱ����Ϣ���\n");
			int off_num;
			off_num = aisino_is_exsist_offline_inv(h_plate->hDev, (char *)plate_info->plate_tax_info.off_inv_fpdm, (char *)plate_info->plate_tax_info.off_inv_fphm, (char *)plate_info->plate_tax_info.off_inv_kpsj, &plate_info->plate_tax_info.off_inv_dzsyh);
			if ((off_num < 0) || (off_num == 100))
			{
				//out("�����Ϣ��ȡ���в��������߷�Ʊ\n");
				plate_info->plate_tax_info.off_inv_num = 0;
			}
			else
			{
				plate_info->plate_tax_info.off_inv_num = 1;
			}
			//out("��ȡ���߷�Ʊ�������\n");
			memcpy(plate_info->plate_tax_info.startup_date, h_plate->hDev->szDeviceEffectDate, 8);
			memcpy(plate_info->plate_tax_info.tax_office_name, h_plate->hDev->szTaxAuthorityName, strlen(h_plate->hDev->szTaxAuthorityName));
			memcpy(plate_info->plate_tax_info.tax_office_code, h_plate->hDev->szTaxAuthorityCode, 11);//��˰�����⴦������\r\n
			memcpy(plate_info->plate_tax_info.area_code, h_plate->hDev->szRegCode, strlen(h_plate->hDev->szRegCode));
			//_WriteLog(LL_INFO, "�������%s", h_plate->hDev->szRegCode);
			if (memcmp(plate_info->plate_tax_info.area_code, "91", 2) == 0)
			{
				_WriteLog(LL_INFO,"����������������������Ҫ�޸�\n");
				memcpy(plate_info->plate_tax_info.area_code,"50",2);
			}
			//GetUPServerURL(0, h_plate->hDev->szRegCode, (char *)plate_info->plate_tax_info.tax_server_url);
			struct UploadAddrModel model;
			memset(&model,0,sizeof(struct UploadAddrModel));
			GetTaServerURL(TAADDR_UPLOAD_RSA, h_plate->hDev->szRegCode, &model);
			strcpy((char *)plate_info->plate_tax_info.tax_server_url, model.szTaxAuthorityURL);

			memset(&model, 0, sizeof(struct UploadAddrModel));
			GetTaServerURL(TAADDR_CONFIRM, h_plate->hDev->szRegCode, &model);
			strcpy((char *)plate_info->plate_tax_info.integrated_server_url, model.szTaxAuthorityURL);
			switch (h_plate->hDev->bNatureOfTaxpayer) {
			case 0:
				sprintf((char *)plate_info->plate_tax_info.tax_nature, "����˰�����");
				break;
			case 1:
				sprintf((char *)plate_info->plate_tax_info.tax_nature, "С��ģ��˰��");
				break;
			case 2:
				sprintf((char *)plate_info->plate_tax_info.tax_nature, "һ����˰��");
				break;
			case 3:
				sprintf((char *)plate_info->plate_tax_info.tax_nature, "ת�Ǽ���˰��");
				break;
			default:
				//sprintf((char *)plate_infos->plate_tax_info.tax_nature, "");
				break;
			}

			plate_info->plate_basic_info.use_downgrade_version = !CheckDeviceVersionMatch(h_plate->hDev, plate_info->plate_basic_info.minimum_version);

			if ((result = CheckDriverVerSupport(h_plate->hDev, h_plate->hDev->szDriverVersion)) < 0) {
				_WriteLog(LL_FATAL, "CheckDriverVerSupport failed, driverVer:%s", h_plate->hDev->szDriverVersion);

				if(result == ERR_DEVICE_FIRMWARE_LOWVERSION)
					sprintf((char *)plate_info->plate_tax_info.driver_ver_err, "�ײ������汾̫��");
				else if (result == ERR_DEVICE_FIRMWARE_HIGHVERSION)
					sprintf((char *)plate_info->plate_tax_info.driver_ver_err, "�ײ������汾����");
			}
			strcpy((char *)plate_info->plate_tax_info.driver_ver, h_plate->hDev->szDriverVersion);

			//out("������ȡ�����Ϣ\n");
			//plate_info->plate_tax_info.lxsc = h_Dev->nMaxOfflineHour;
			if (h_plate->hDev->bTrainFPDiskType != 0)
				plate_info->plate_basic_info.plate_test = 1;
			

		}
		else if (nDeviceType == DEVICE_CNTAX)
		{
			out("��ȡaisino˰��UKEY�����Ϣ");
			result = swukey_get_basic_info(h_plate->hDev, plate_info);
			if (result < 0)
			{
				printf("[-] swukey_get_basic_info failed result = %d\n", result);
			}
			if (strlen(h_plate->hDev->szPubServer) != 0)
			{
				sprintf((char *)plate_info->plate_basic_info.used_ofd, "1");
			}
			else
			{
				sprintf((char *)plate_info->plate_basic_info.used_ofd, "0");
			}
			memcpy(plate_info->plate_basic_info.server_type, h_plate->hDev->szPubServer, 2);
			strcpy((char *)plate_info->plate_basic_info.server_number, h_plate->hDev->szPubServer + 2);
			swukey_get_basic_info(h_plate->hDev, plate_info);
			cntax_get_state_info(h_plate->hDev, plate_info);
			cntax_read_inv_coil_number(h_plate->hDev, plate_info);
#ifndef DF_OLD_MSERVER
			cntax_update_summary_data(h_plate->hDev, plate_info);
#endif
			//cntax_get_state_info_and_coil_info(h_plate->hDev, h_plate->hUSB, plate_info);
			memcpy(plate_info->plate_tax_info.startup_date, h_plate->hDev->szDeviceEffectDate, 8);
			memcpy(plate_info->plate_tax_info.tax_office_name, h_plate->hDev->szTaxAuthorityName, strlen(h_plate->hDev->szTaxAuthorityName));
			TranslateStandTime(2, h_plate->hDev->szDeviceTime, (char *)plate_info->plate_tax_info.plate_time);
			strcpy((char *)plate_info->plate_tax_info.driver_ver, h_plate->hDev->szDriverVersion);
			int off_num;
			off_num = cntax_is_exsist_offline_inv(h_plate->hDev, (char *)plate_info->plate_tax_info.off_inv_fpdm, (char *)plate_info->plate_tax_info.off_inv_fphm, (char *)plate_info->plate_tax_info.off_inv_kpsj, &plate_info->plate_tax_info.off_inv_dzsyh);
			if (off_num <= 0)
			{
				//out("�����Ϣ��ȡ���в��������߷�Ʊ\n");
				plate_info->plate_tax_info.off_inv_num = 0;
			}
			else
			{
				plate_info->plate_tax_info.off_inv_num = 1;
			}

			memcpy(plate_info->plate_tax_info.tax_office_code, h_plate->hDev->szTaxAuthorityCode, 11);
			memcpy(plate_info->plate_tax_info.area_code, h_plate->hDev->szRegCode, strlen(h_plate->hDev->szRegCode));
			//GetUPServerURL(2, h_plate->hDev->szRegCode, (char *)plate_info->plate_tax_info.tax_server_url);
			struct UploadAddrModel model;
			memset(&model,0,sizeof(struct UploadAddrModel));
			GetTaServerURL(TAADDR_UPLOAD_GM, h_plate->hDev->szRegCode, &model);
			strcpy((char *)plate_info->plate_tax_info.tax_server_url, model.szTaxAuthorityURL);

			memset(&model, 0, sizeof(struct UploadAddrModel));
			GetTaServerURL(TAADDR_CONFIRM, h_plate->hDev->szRegCode, &model);
			strcpy((char *)plate_info->plate_tax_info.integrated_server_url, model.szTaxAuthorityURL);
			switch (h_plate->hDev->bNatureOfTaxpayer) {
			case 0:
				sprintf((char *)plate_info->plate_tax_info.tax_nature, "����˰�����");
				break;
			case 1:
				sprintf((char *)plate_info->plate_tax_info.tax_nature, "С��ģ��˰��");
				break;
			case 2:
				sprintf((char *)plate_info->plate_tax_info.tax_nature, "һ����˰��");
				break;
			case 3:
				sprintf((char *)plate_info->plate_tax_info.tax_nature, "ת�Ǽ���˰��");
				break;
			default:
				//sprintf((char *)plate_infos->plate_tax_info.tax_nature, "");
				break;
			}
			plate_info->plate_basic_info.use_downgrade_version = !CheckDeviceVersionMatch(h_plate->hDev, plate_info->plate_basic_info.minimum_version);
			if (h_plate->hDev->bTrainFPDiskType != 0)
				plate_info->plate_basic_info.plate_test = 1;

		}
		else if (nDeviceType == DEVICE_JSP_KEY)
		{
			//memcpy(plate_info->plate_basic_info.ca_number, h_Dev->szCompressTaxID, strlen(h_Dev->szCompressTaxID));//��֧��˰�Ŷ�ȡ,�ײ���˰����Ϣ
			memcpy(plate_info->plate_basic_info.ca_name, h_plate->hDev->szCompanyName, strlen(h_plate->hDev->szCompanyName));
			memcpy(plate_info->plate_basic_info.plate_num, h_plate->hDev->szDeviceID, strlen(h_plate->hDev->szDeviceID));
			plate_info->plate_basic_info.result = 1;
			plate_info->plate_basic_info.plate_type = DEVICE_JSP_KEY;
			plate_info->plate_basic_info.extension = h_plate->hDev->uICCardNo;
		}
		//_WriteLog(LL_INFO, "��ȡaisino�����Ϣ����,DEV [%08X]  USB [%08X]\n", h_plate->hDev, h_plate->hUSB);
		h_plate->need_free = 1;
		return 0;

	}
	else if (usb_type == DEVICE_CNTAX)
	{
		
		plate_info->plate_basic_info.dev_passwd_right = 1;
		nDeviceType = usb_device_open_serssion(&h_plate->hUSB, &h_plate->hDev, true, nBusID, nDevID, h_plate->busid, last_error);
		if (nDeviceType < 0)
		{

			_WriteLog(LL_WARN, "˰��UKEY�豸��ʼ����ʧ��,���������Ϊ%s\n", last_error);
			if (strstr(last_error, "09d1c") != NULL)
			{
				char devpass_leftcount[10] = { 0 };
				char errinfo[1024] = { 0 };

				char *tmp = strstr(last_error, "09d1c");
				tmp += strlen("09d1c");

				memcpy(devpass_leftcount, tmp, 1);
				sprintf(errinfo, "��Ĭ���豸����12345678,ʣ�ೢ�Դ���%s", devpass_leftcount);
				logout(INFO, "TAXLIB", "˰�̶�ȡ", "�ϱ�˰�̣�˰��ukey,������Ϣ��%s\r\n", errinfo);
				//report_event(NULL, "˰��UKey�豸��������", errinfo, nDeviceType);
				plate_info->plate_basic_info.plate_type = DEVICE_CNTAX;
				plate_info->plate_basic_info.dev_passwd_right = 0;
				plate_info->plate_basic_info.dev_left_num = atoi(devpass_leftcount);
				plate_info->plate_basic_info.dev_err_reason = ERR_DEVICE_PASSWORD_ERROR;
			}
			else if (strstr(last_error, "09d10b") != NULL)
			{
				char errinfo[1024] = { 0 };
				sprintf(errinfo, "�豸����pin����(09d10b),ʣ�ೢ�Դ���0");
				logout(INFO, "TAXLIB", "˰�̶�ȡ", "�ϱ�˰�̣�˰��ukey,������Ϣ��%s\r\n", errinfo);
				//report_event(NULL, "˰��UKey�豸��������", errinfo, nDeviceType);
				plate_info->plate_basic_info.plate_type = DEVICE_CNTAX;
				plate_info->plate_basic_info.dev_passwd_right = 0;
				plate_info->plate_basic_info.dev_left_num = 0;
				plate_info->plate_basic_info.dev_err_reason = ERR_DEVICE_PASSWORD_LOCK;
			}
			return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;

		}
		if (strlen(h_plate->hDev->szPubServer) != 0)
		{
			sprintf((char *)plate_info->plate_basic_info.used_ofd, "1");
		}
		else
		{
			sprintf((char *)plate_info->plate_basic_info.used_ofd, "0");
		}
		memcpy(plate_info->plate_basic_info.server_type, h_plate->hDev->szPubServer, 2);
		strcpy((char *)plate_info->plate_basic_info.server_number, h_plate->hDev->szPubServer + 2);
		swukey_get_basic_info(h_plate->hDev, plate_info);
		cntax_get_state_info(h_plate->hDev, plate_info);
		cntax_read_inv_coil_number(h_plate->hDev, plate_info);
#ifndef DF_OLD_MSERVER
		cntax_update_summary_data(h_plate->hDev, plate_info);
#endif
		//cntax_get_state_info_and_coil_info(h_plate->hDev, h_plate->hUSB, plate_info);
		memcpy(plate_info->plate_tax_info.startup_date, h_plate->hDev->szDeviceEffectDate, 8);
		memcpy(plate_info->plate_tax_info.tax_office_name, h_plate->hDev->szTaxAuthorityName, strlen(h_plate->hDev->szTaxAuthorityName));
		TranslateStandTime(2, h_plate->hDev->szDeviceTime, (char *)plate_info->plate_tax_info.plate_time);
		strcpy((char *)plate_info->plate_tax_info.driver_ver, h_plate->hDev->szDriverVersion);
		int off_num;
		off_num = cntax_is_exsist_offline_inv(h_plate->hDev, (char *)plate_info->plate_tax_info.off_inv_fpdm, (char *)plate_info->plate_tax_info.off_inv_fphm, (char *)plate_info->plate_tax_info.off_inv_kpsj, &plate_info->plate_tax_info.off_inv_dzsyh);
		if (off_num <= 0)
		{
			//out("�����Ϣ��ȡ���в��������߷�Ʊ\n");
			plate_info->plate_tax_info.off_inv_num = 0;
		}
		else
		{
			plate_info->plate_tax_info.off_inv_num = 1;
		}
		memcpy(plate_info->plate_tax_info.tax_office_code, h_plate->hDev->szTaxAuthorityCode, 11);
		memcpy(plate_info->plate_tax_info.area_code, h_plate->hDev->szRegCode, strlen(h_plate->hDev->szRegCode));
		//GetUPServerURL(2, h_plate->hDev->szRegCode, (char *)plate_info->plate_tax_info.tax_server_url);
		struct UploadAddrModel model;
		GetTaServerURL(TAADDR_UPLOAD_GM, h_plate->hDev->szRegCode, &model);
		strcpy((char *)plate_info->plate_tax_info.tax_server_url, model.szTaxAuthorityURL);

		memset(&model, 0, sizeof(struct UploadAddrModel));
		GetTaServerURL(TAADDR_CONFIRM, h_plate->hDev->szRegCode, &model);
		strcpy((char *)plate_info->plate_tax_info.integrated_server_url, model.szTaxAuthorityURL);
		switch (h_plate->hDev->bNatureOfTaxpayer) {
		case 0:
			sprintf((char *)plate_info->plate_tax_info.tax_nature, "����˰�����");
			break;
		case 1:
			sprintf((char *)plate_info->plate_tax_info.tax_nature, "С��ģ��˰��");
			break;
		case 2:
			sprintf((char *)plate_info->plate_tax_info.tax_nature, "һ����˰��");
			break;
		case 3:
			sprintf((char *)plate_info->plate_tax_info.tax_nature, "ת�Ǽ���˰��");
			break;
		default:
			//sprintf((char *)plate_infos->plate_tax_info.tax_nature, "");
			break;
		}
		plate_info->plate_basic_info.use_downgrade_version = !CheckDeviceVersionMatch(h_plate->hDev, plate_info->plate_basic_info.minimum_version);
		h_plate->need_free = 1;

		if (h_plate->hDev->bTrainFPDiskType != 0)
			plate_info->plate_basic_info.plate_test = 1;
		//out("swukey_get_basic_info ��ȡ���\n");
		//out("swukey_get_basic_info ��ȡ��ɹر�USB������\n");
		return 0;
	}
	else if (usb_type == DEVICE_SKP_KEY)
	{

		plate_info->plate_basic_info.dev_passwd_right = 1;
		nDeviceType = usb_device_open_serssion(&h_plate->hUSB, &h_plate->hDev, false, nBusID, nDevID, h_plate->busid, last_error);
		out("usb_device_open_serssion success ���ߺţ�%d���豸�ţ�%d\n", nBusID, nDevID);
		if (nDeviceType < 0)
		{
			return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
		}
		else if (nDeviceType == DEVICE_SKP_KEY)
		{
			memcpy(plate_info->plate_basic_info.ca_number, h_plate->hDev->szCommonTaxID, strlen(h_plate->hDev->szCommonTaxID));//��֧��˰�Ŷ�ȡ,�ײ���˰����Ϣ
			memcpy(plate_info->plate_basic_info.ca_name, h_plate->hDev->szCompanyName, strlen(h_plate->hDev->szCompanyName));
			memcpy(plate_info->plate_basic_info.plate_num, h_plate->hDev->szDeviceID, strlen(h_plate->hDev->szDeviceID));
			plate_info->plate_basic_info.result = 3;
			plate_info->plate_basic_info.plate_type = DEVICE_SKP_KEY;
			plate_info->plate_basic_info.extension = 0;
		}
		out("���ƣ�%s\n", plate_info->plate_basic_info.ca_name);
		h_plate->need_free = 1;
		return 0;
	}
	else
	{
		logout(INFO, "TAXLIB", "˰�̶�ȡ", "δ֪���豸���ͣ��޷�֧�ֶ�ȡ\n");
		return -1;
	}
	return 0;
}

int function_common_close_usb_device(struct HandlePlate *h_plate)
{

	int usb_type = h_plate->usb_type;

	if (usb_type == DEVICE_MENGBAI)
	{
		usb_hid_close_serssion(h_plate->hUSB, h_plate->hDev);
	}
	else if ((usb_type == DEVICE_NISEC) || (usb_type == DEVICE_AISINO) || (usb_type == DEVICE_CNTAX) || (usb_type == DEVICE_SKP_KEY))
	{
		usb_device_close_serssion(h_plate->hUSB, h_plate->hDev, 0);
	}
	else
	{
		return -1;
	}
	h_plate->hUSB = NULL;
	h_plate->hDev = NULL;
	h_plate->nBusID = 0;
	h_plate->nDevID = 0;
	h_plate->usb_type = 0;
	h_plate->need_check_passwd = 0;
	h_plate->need_free = 0;
	memset(h_plate->busid, 0, sizeof(h_plate->busid));
	return 0;
}

//==����Ҫ��Ȩ�Ҳ���Ҫ������֤ͨ����ɵ��õĽӿ�==//
//��ȡ˰�̵�ǰʱ��
int function_aisino_get_tax_time(struct HandlePlate *h_plate, char* outtime)
{
	int result = -1;
	result = jude_busnum_devnum_handle(h_plate);
	if (result < 0)
	{
		return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
	}
	result = aisino_read_tax_time(h_plate->hDev, outtime);
	if (result < 0)
	{
		printf("[-] function_aisino_get_tax_time failed result = %d\n", result);
	}
	return result;
}
int function_nisec_get_tax_time(struct HandlePlate *h_plate, char* outtime)
{
	int result = -1;
	result = jude_busnum_devnum_handle(h_plate);
	if (result < 0)
	{
		return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
	}
	result = nisec_read_tax_time(h_plate->hDev, outtime);
	if (result < 0)
	{
		printf("[-] function_nisec_get_tax_time failed result = %d\n", result);
	}
	return result;
}
int function_cntax_get_tax_time(struct HandlePlate *h_plate, char* outtime)
{
	int result = -1;
	result = jude_busnum_devnum_handle(h_plate);
	if (result < 0)
	{
		return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
	}
	result = cntax_read_tax_time(h_plate->hDev, outtime);
	if (result < 0)
	{
		printf("[-] function_cntax_get_tax_time failed result = %d\n", result);
	}
	return result;
}
int function_mengbai_get_plate_time(struct HandlePlate *h_plate, char  *outtime)
{
	int result;
	result = jude_busnum_devnum_handle(h_plate);
	if (result < 0)
	{
		return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
	}
	result = mengbai_read_plate_time(h_plate->hUSB, (uint8 *)outtime);
	if (result < 0)
	{
		printf("[-] function_mengbai_get_plate_time failed result = %d\n", result);
	}
	return result;
}


//mqtt�ӿڻ�ȡ��Ʊ��ϸ
int function_aisino_get_inv_details_mqtt(struct HandlePlate *h_plate, unsigned char *month, void *function, void *arg, char *errinfo, int old_new, unsigned short *start_stop, int sfxqqd)
{
	int result = -1;	
	result = jude_busnum_devnum_handle(h_plate);
	if (result < 0)
	{
		return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
	}
	result = aisino_get_inv_details_mqtt(h_plate->hDev, month, function, arg, errinfo, old_new, start_stop, sfxqqd);
	if (result < 0)
	{
		printf("[-] function_aisino_get_inv_details_mqtt failed result = %d\n", result);
	}
	return result;
}
int function_nisec_get_inv_details_mqtt(struct HandlePlate *h_plate, unsigned char *month, void *function, void *arg, char *errinfo, int old_new, unsigned short *start_stop, int sfxqqd)
{
	int result = -1;
	result = jude_busnum_devnum_handle(h_plate);
	if (result < 0)
	{
		return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
	}
	result = nisec_get_inv_details_mqtt(h_plate->hDev, month, function, arg, errinfo, old_new, start_stop, sfxqqd);
	if (result < 0)
	{
		printf("[-] function_nisec_get_inv_details_mqtt failed result = %d\n", result);
	}
	return result;
}
int function_cntax_get_inv_details_mqtt(struct HandlePlate *h_plate, unsigned char *month, void *function, void *arg, char *errinfo, int old_new, unsigned short *start_stop, int sfxqqd)
{
	int result = -1;
	result = jude_busnum_devnum_handle(h_plate);
	if (result < 0)
	{
		return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
	}
	result = cntax_get_inv_details_mqtt(h_plate->hDev, month, function, arg, errinfo, old_new, start_stop, sfxqqd);
	if (result < 0)
	{
		printf("[-] function_cntax_get_inv_details_mqtt failed result = %d\n", result);
	}
	return result;
}
int function_mengbai_get_inv_details_mqtt(struct HandlePlate *h_plate, unsigned char *month, void *function, void *arg, char *errinfo, int old_new, unsigned short *start_stop, int sfxqqd)
{
	int result;
	result = jude_busnum_devnum_handle(h_plate);
	if (result < 0)
	{
		return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
	}
	result = mengbai_read_inv_details_mqtt(h_plate->hDev, month, function, arg, errinfo, old_new, start_stop);
	if (result < 0)
	{
		printf("[-] Read inv_details_mqtt failed\n");
		return result;
	}
	return result;
}

//��Ʊ�ϴ�M����
int function_aisino_upload_m_server(struct HandlePlate *h_plate, unsigned short *start_stop, struct _upload_inv_num *upload_inv_num, void *function, void *arg, struct _inv_sum_data *now_month_sum)
{
	int result = -1;
	result = jude_busnum_devnum_handle(h_plate);
	if (result < 0)
	{
		return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
	}
	result = aisino_upload_m_server(h_plate->hDev, start_stop, upload_inv_num, function, arg, now_month_sum);
	if (result < 0)
	{
		printf("[-] function_aisino_upload_m_server failed result = %d\n", result);
		return result;
	}
#ifndef DF_OLD_MSERVER
	result = aisino_upload_summary_m_server(h_plate->hDev, start_stop, upload_inv_num, function, arg, now_month_sum);
	if (result < 0)
	{
		printf("[-] aisino_upload_summary_m_server failed result = %d\n", result);
		return result;
	}
#endif
	return result;
}
int function_nisec_upload_m_server(struct HandlePlate *h_plate, unsigned short *start_stop, struct _upload_inv_num *upload_inv_num, void *function, void *arg, struct _inv_sum_data *now_month_sum)
{
	int result = -1;
	result = jude_busnum_devnum_handle(h_plate);
	if (result < 0)
	{
		return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
	}
	result = nisec_upload_m_server(h_plate->hDev, start_stop, upload_inv_num, function, arg, now_month_sum);
	if (result < 0)
	{
		printf("[-] function_nisec_upload_m_server failed result = %d\n", result);
		return result;
	}
#ifndef DF_OLD_MSERVER
	result = nisec_upload_summary_m_server(h_plate->hDev, start_stop, upload_inv_num, function, arg, now_month_sum);
	if (result < 0)
	{
		printf("[-] nisec_upload_summary_m_server failed result = %d\n", result);
		return result;
	}
#endif
	return result;
}
int function_cntax_upload_m_server(struct HandlePlate *h_plate, unsigned short *start_stop, struct _upload_inv_num *upload_inv_num, void *function, void *arg, struct _inv_sum_data *now_month_sum)
{
	int result = -1;
	result = jude_busnum_devnum_handle(h_plate);
	if (result < 0)
	{
		return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
	}
	result = cntax_upload_m_server(h_plate->hDev, start_stop, upload_inv_num, function, arg, now_month_sum);
	if (result < 0)
	{
		printf("[-] function_cntax_upload_m_server failed result = %d\n", result);
		return result;
	}
#ifndef DF_OLD_MSERVER
	result = cntax_upload_summary_m_server(h_plate->hDev, start_stop, upload_inv_num, function, arg, now_month_sum);
	if (result < 0)
	{
		printf("[-] cntax_upload_summary_m_server failed result = %d\n", result);
		return result;
	}
#endif
	return result;
}
int function_mengbai_upload_m_server(struct HandlePlate *h_plate, unsigned short *start_stop, struct _upload_inv_num *upload_inv_num, void *function, void *arg, struct _inv_sum_data *now_month_sum)
{
	int result = -1;
	result = jude_busnum_devnum_handle(h_plate);
	if (result < 0)
	{
		return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
	}
	result = mengbai_upload_m_server(h_plate->hDev, start_stop, upload_inv_num, function, arg,now_month_sum);
	if (result < 0)
	{
		printf("[-] function_mengbai_upload_m_server failed result = %d\n", result);
	}

	return result;
}
//��ȡ�·�Ʊ����
int function_aisino_get_month_invoice_num(struct HandlePlate *h_plate, unsigned char *month, unsigned int  *count, unsigned long *size)
{
	int result = -1;
	result = jude_busnum_devnum_handle(h_plate);
	if (result < 0)
	{
		return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
	}
	result = aisino_my_read_invoice_month_num(h_plate->hUSB, (char *)month, count, size);
	if (result < 0)
	{
		printf("[-] function_aisino_get_month_invoice_num failed result = %d\n", result);
	}
	return result;
}
int function_nisec_get_month_invoice_num(struct HandlePlate *h_plate, unsigned char *month, unsigned int  *count, unsigned long *size)
{
	int result = -1; 
	result = jude_busnum_devnum_handle(h_plate);
	if (result < 0)
	{
		return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
	}
	result = nesic_my_read_invoice_num(h_plate->hDev, (char *)month, count, size);
	if (result < 0)
	{
		printf("[-] function_nisec_get_month_invoice_num failed result = %d\n", result);
	}
	return result;
}
int function_cntax_get_month_invoice_num(struct HandlePlate *h_plate, unsigned char *month, unsigned int  *count, unsigned long *size)
{
	int result = -1;
	result = jude_busnum_devnum_handle(h_plate);
	if (result < 0)
	{
		return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
	}
	result = cntax_my_read_invoice_num(h_plate->hDev, (char *)month, count, size);
	if (result < 0)
	{
		printf("[-] function_nisec_get_month_invoice_num failed result = %d\n", result);
	}
	return result;
}
int function_mengbai_get_month_invoice_num(struct HandlePlate *h_plate, unsigned char *month, unsigned int  *count, unsigned long *size)
{
	int result = -1;
	result = jude_busnum_devnum_handle(h_plate);
	if (result < 0)
	{
		return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
	}
	result = mengbai_read_invoice_num(h_plate->hUSB, month, count, size);
	if (result < 0)
	{
		printf("[-] Read get_month_invoice_num failed\n");
	}
	return result;
}
//����Ʊ��������ȡ��Ʊ��Ϣ
int function_aisino_fpdm_fphm_get_invs(struct HandlePlate *h_plate, char  *fpdm, char *fphm, int dzsyh, char **inv_json)
{
	int result = -1;
	result = jude_busnum_devnum_handle(h_plate);
	if (result < 0)
	{
		return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
	}
	result = aisino_fpdm_fphm_get_invs(h_plate->hDev, fpdm, fphm, dzsyh, inv_json);
	if (result < 0)
	{
		printf("[-] function_aisino_fpdm_fphm_get_invs failed result = %d\n", result);
	}
	return result;
}
int function_nisec_fpdm_fphm_get_invs(struct HandlePlate *h_plate, char  *fpdm, char *fphm, int dzsyh, char **inv_json)
{
	int result = -1;
	result = jude_busnum_devnum_handle(h_plate);
	if (result < 0)
	{
		return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
	}
	result = nisec_fpdm_fphm_get_invs(h_plate->hDev, fpdm, fphm, inv_json);
	if (result < 0)
	{
		printf("[-] function_nisec_fpdm_fphm_get_invs failed result = %d\n", result);
	}
	return result;
}
int function_cntax_fpdm_fphm_get_invs(struct HandlePlate *h_plate, char  *fpdm, char *fphm, int dzsyh, char **inv_json)
{
	int result = -1;
	result = jude_busnum_devnum_handle(h_plate);
	if (result < 0)
	{
		return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
	}
	result = cntax_fpdm_fphm_get_invs(h_plate->hDev, fpdm, fphm, inv_json);
	if (result < 0)
	{
		printf("[-] function_cntax_fpdm_fphm_get_invs failed result = %d\n", result);
	}
	return result;
}
//����Ʊ��������ȡ��Ʊ�������ļ���Ϣ
int function_aisino_fpdm_fphm_get_invs_kpstr(struct HandlePlate *h_plate, char  *fpdm, char *fphm, int dzsyh, char **inv_json, char **kp_bin, char **kp_str)
{
	int result = -1;
	result = jude_busnum_devnum_handle(h_plate);
	if (result < 0)
	{
		return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
	}
	result = aisino_fpdm_fphm_get_invs_kpstr(h_plate->hDev, fpdm, fphm, dzsyh, inv_json, kp_bin, kp_str);
	if (result < 0)
	{
		printf("[-] function_aisino_fpdm_fphm_get_invs_kpstr failed result = %d\n", result);
	}
	return result;
}
int function_nisec_fpdm_fphm_get_invs_kpstr(struct HandlePlate *h_plate, char  *fpdm, char *fphm, int dzsyh, char **inv_json, char **kp_bin, char **kp_str)
{
	int result = -1;
	result = jude_busnum_devnum_handle(h_plate);
	if (result < 0)
	{
		return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
	}
	result = nisec_fpdm_fphm_get_invs(h_plate->hDev, fpdm, fphm, inv_json);
	if (result < 0)
	{
		printf("[-] function_nisec_fpdm_fphm_get_invs_kpstr failed result = %d\n", result);
	}
	return result;
}
int function_cntax_fpdm_fphm_get_invs_kpstr(struct HandlePlate *h_plate, char  *fpdm, char *fphm, int dzsyh, char **inv_json, char **kp_bin, char **kp_str)
{
	int result = -1;
	result = jude_busnum_devnum_handle(h_plate);
	if (result < 0)
	{
		return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
	}
	result = cntax_fpdm_fphm_get_invs(h_plate->hDev, fpdm, fphm, inv_json);
	if (result < 0)
	{
		printf("[-] function_cntax_fpdm_fphm_get_invs_kpstr failed result = %d\n", result);
	}
	return result;
}

//==��Ҫ��Ȩδ��֤�����������ɵ��õĽӿ�==//
//����������֤֤�����
int function_aisino_get_auth_passwd(struct HandlePlate *h_plate, struct _plate_infos *plate_info)
{
	int result = -1;
	int left_num = -1;

	//�ж϶˿��Ƿ���Ȩ
	result = jude_busnum_devnum_handle(h_plate);
	if (result < 0)
	{
		plate_info->plate_basic_info.cert_passwd_right = 0;
		plate_info->plate_basic_info.cert_err_reason = DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
		return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
	}

	//��֤֤������Ƿ�Ϊ12345678
	//char passwd[100] = { 0 };
	result = aisino_verify_cert_passwd(h_plate->hDev, &left_num);
	if (result > 0)
	{
		plate_info->plate_basic_info.cert_passwd_right = 1;
		//out("˰����Ĭ��֤�����12345678��֤�ɹ�\n");
	}
	else
	{
		plate_info->plate_basic_info.cert_passwd_right = 0;
		plate_info->plate_basic_info.cert_left_num = left_num;
		plate_info->plate_basic_info.cert_err_reason = result;
		out("��˰��Ĭ��֤�����12345678��֤ʧ��\n");

	}
	return result;
}
int function_nisec_get_auth_passwd(struct HandlePlate *h_plate, struct _plate_infos *plate_info)
{
	int result = -1;
	int left_num = -1;

	//�ж϶˿��Ƿ���Ȩ
	result = jude_busnum_devnum_handle(h_plate);
	if (result < 0)
	{
		plate_info->plate_basic_info.cert_passwd_right = 0;
		plate_info->plate_basic_info.cert_err_reason = DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
		return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
	}
			
	//��֤֤������Ƿ�Ϊ12345678
	//char passwd[100] = { 0 };
	result = nisec_verify_cert_passwd(h_plate->hDev, &left_num);
	if (result > 0)
	{
		plate_info->plate_basic_info.cert_passwd_right = 1;
		//out("˰����Ĭ��֤�����12345678��֤�ɹ�\n");
	}
	else
	{
		plate_info->plate_basic_info.cert_passwd_right = 0;
		plate_info->plate_basic_info.cert_left_num = left_num;
		plate_info->plate_basic_info.cert_err_reason = result;
		out("˰����Ĭ��֤�����12345678��֤ʧ��\n");

	}
	return result;
}
int function_cntax_get_auth_passwd(struct HandlePlate *h_plate, struct _plate_infos *plate_info)
{
	int result = -1;
	int left_num = -1;

	//�ж϶˿��Ƿ���Ȩ
	result = jude_busnum_devnum_handle(h_plate);
	if (result < 0)
	{
		plate_info->plate_basic_info.cert_passwd_right = 0;
		plate_info->plate_basic_info.cert_err_reason = DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
		return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
	}

	//��֤֤������Ƿ�Ϊ12345678
	//char passwd[100] = { 0 };
	result = cntax_verify_cert_passwd(h_plate->hDev, &left_num);
	if (result > 0)
	{
		plate_info->plate_basic_info.cert_passwd_right = 1;
		//out("˰����Ĭ��֤�����12345678��֤�ɹ�\n");
	}
	else
	{
		plate_info->plate_basic_info.cert_passwd_right = 0;
		plate_info->plate_basic_info.cert_left_num = left_num;
		plate_info->plate_basic_info.cert_err_reason = result;
		out("˰��ukeyĬ��֤�����12345678��֤ʧ��\n");

	}
	return result;
}
int function_mengbai_get_auth_passwd(struct HandlePlate *h_plate, struct _plate_infos *plate_info)
{
	int result = -1;
	result = jude_busnum_devnum_handle(h_plate);
	if (result < 0)
	{
		plate_info->plate_basic_info.cert_passwd_right = 0;
		plate_info->plate_basic_info.cert_err_reason = DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
		return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
	}
	result = mengbai_read_cert_passwd(h_plate->hUSB, (unsigned char *)"11111111");
	if (result < 0)
	{
		plate_info->plate_basic_info.cert_err_reason = result;
		plate_info->plate_basic_info.cert_left_num = 10;
		plate_info->plate_basic_info.cert_passwd_right = 1;
	}
	else
	{
		plate_info->plate_basic_info.cert_passwd_right = 1;
	}
	return 0;
}
//�ӿڵ�����֤Ĭ��֤������Ƿ���ȷ
int function_aisino_verify_cert_passwd(struct HandlePlate *h_plate, char *passwd)
{
	int result = -1;
	int left_num = -1;
	result = jude_busnum_devnum_handle(h_plate);
	if (result < 0)
	{
		return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
	}
	//out("���ߺţ�%d���豸�ţ�%d\n", nBusID, nDevID);
	result = aisino_verify_cert_passwd(h_plate->hDev, &left_num);
	if (result > 0)
	{
		strcpy(passwd, "12345678");
		
	}
	else
	{
		printf("[-] function_aisino_verify_cert_passwd failed result = %d\n", result);
	}
	return result;
}
int function_nisec_verify_cert_passwd(struct HandlePlate *h_plate, char *passwd)
{
	int result = -1;
	int left_num = -1;
	result = jude_busnum_devnum_handle(h_plate);
	if (result < 0)
	{
		return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
	}
	//out("���ߺţ�%d���豸�ţ�%d\n", nBusID, nDevID);
	result = nisec_verify_cert_passwd(h_plate->hDev, &left_num);
	if (result <= 0)
	{
		printf("[-] function_nisec_verify_cert_passwd failed result = %d\n", result);
	}
	else
	{
		strcpy(passwd,"12345678");
	}
	return result;
}
int function_cntax_verify_cert_passwd(struct HandlePlate *h_plate, char *passwd)
{
	strcpy(passwd, "12345678");	
	return 0;
}
int function_mengbai_verify_cert_passwd(struct HandlePlate *h_plate, char *passwd)
{
	int result;
	result = jude_busnum_devnum_handle(h_plate);
	if (result < 0)
	{
		return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
	}
	result = mengbai_read_cert_passwd(h_plate->hUSB,(uint8 *) "11111111");
	if (result < 0)
	{
		printf("[-] Read ID failed\n");
	}
	else
	{
		strcpy(passwd, "12345678");
	}
	return result;
}
//�޸�֤�����
int function_aisino_change_certpassword(struct HandlePlate *h_plate, char *old_passwd, char *new_passwd, char *errinfo)
{
	int result = -1;
	result = jude_busnum_devnum_handle(h_plate);
	if (result < 0)
	{
		return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
	}
	result = aisino_change_certpassword(h_plate->hUSB, old_passwd, new_passwd, errinfo);
	if (result < 0)
	{
		printf("[-] function_aisino_change_certpassword failed result = %d\n", result);
	}
	return result;
}
int function_nisec_change_certpassword(struct HandlePlate *h_plate, char *old_passwd, char *new_passwd, char *errinfo)
{
	int result = -1;
	result = jude_busnum_devnum_handle(h_plate);
	if (result < 0)
	{
		return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
	}
	result = nisec_change_certpassword(h_plate->hDev, old_passwd, new_passwd, errinfo);
	if (result < 0)
	{
		printf("[-] function_aisino_change_certpassword failed result = %d\n", result);
	}
	return result;
}
int function_cntax_change_certpassword(struct HandlePlate *h_plate, char *old_passwd, char *new_passwd, char *errinfo)
{
	int result = -1;
	result = jude_busnum_devnum_handle(h_plate);
	if (result < 0)
	{
		return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
	}
	result = cntax_change_certpassword(h_plate->hDev, old_passwd, new_passwd, errinfo);
	if (result < 0)
	{
		printf("[-] function_aisino_change_certpassword failed result = %d\n", result);
	}
	return result;
}
int function_mengbai_change_certpassword(struct HandlePlate *h_plate, char *old_passwd, char *new_passwd, char *errinfo)
{
	int result;
	result = jude_busnum_devnum_handle(h_plate);
	if (result < 0)
	{
		return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
	}
	result = mengbai_read_cert_passwd(h_plate->hUSB, (uint8 *)old_passwd);
	if (result < 0)
	{
		printf("[-] Read ID failed\n");
	}
	return result;
}
//��ȡ��ǰ��Ʊ�������
int function_aisino_get_current_invcode(struct HandlePlate *h_plate, unsigned char type, char* invnum, char* invcode)
{
	int result = -1;
	result = jude_busnum_devnum_handle(h_plate);
	if (result < 0)
	{
		return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
	}
	//out("���ߺţ�%d���豸�ţ�%d\n", nBusID, nDevID);
	
	result = aisino_read_current_inv_code(h_plate->hUSB, type, invnum, invcode);
	if (result < 0)
	{
		printf("[-] function_aisino_get_current_invcode failed result = %d\n", result);
	}
	return result;
}
int function_nisec_get_current_invcode(struct HandlePlate *h_plate, unsigned char type, char* invnum, char* invcode)
{
	int result = -1;
	result = jude_busnum_devnum_handle(h_plate);
	if (result < 0)
	{
		return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
	}
	result = nisec_read_current_inv_code(h_plate->hDev, type, invnum, invcode);
	if (result < 0)
	{
		printf("[-] function_nisec_make_invoice failed result = %d\n", result);
	}
	return result;
}
int function_cntax_get_current_invcode(struct HandlePlate *h_plate, unsigned char type, char* invnum, char* invcode)
{
	int result = -1;
	result = jude_busnum_devnum_handle(h_plate);
	if (result < 0)
	{
		return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
	}
	//out("���ߺţ�%d���豸�ţ�%d\n", nBusID, nDevID);
	result = cntax_read_current_inv_code(h_plate->hDev, type, invnum, invcode);
	if (result < 0)
	{
		printf("[-] function_cntax_get_current_invcode failed result = %d\n", result);
	}
	return result;
}
int function_mengbai_get_current_invcode(struct HandlePlate *h_plate, unsigned char type, char* invnum, char* invcode)
{
	int result;
	result = jude_busnum_devnum_handle(h_plate);
	if (result < 0)
	{
		return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
	}
	result = mengbai_read_current_inv_code(h_plate->hUSB, type, invnum, invcode);
	if (result < 0)
	{
		printf("[-] Read ID failed\n");
	}
	return result;
}


//======��Ҫ��Ȩ����֤������ȷ�󷽿ɲ����Ľӿ�======//
//��˰����
int function_aisino_copy_report_data(struct HandlePlate *h_plate, uint8 inv_type, char *errinfo)
{
	int result = -1;
	result = jude_busnum_devnum_handle(h_plate);
	if (result < 0)
	{
		return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
	}
	result = aisino_copy_report_data(h_plate->hDev, inv_type, errinfo);
	if (result < 0)
	{
		printf("[-] function_aisino_copy_report_data failed result = %d\n", result);
	}
	return result;
}
int function_nisec_copy_report_data(struct HandlePlate *h_plate, uint8 inv_type, char *errinfo)
{
	int result = -1;
	result = jude_busnum_devnum_handle(h_plate);
	if (result < 0)
	{
		return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
	}
	result = nisec_copy_report_data(h_plate->hDev, inv_type, errinfo);
	if (result < 0)
	{
		printf("[-] function_nisec_copy_report_data failed result = %d\n", result);
	}
	return result;
}
int function_cntax_copy_report_data(struct HandlePlate *h_plate, uint8 inv_type, char *errinfo)
{
	int result = -1;
	result = jude_busnum_devnum_handle(h_plate);
	if (result < 0)
	{
		return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
	}
	result = cntax_copy_report_data(h_plate->hDev, inv_type, errinfo);
	if (result < 0)
	{
		printf("[-] function_cntax_copy_report_data failed result = %d\n", result);
	}
	return result;
}
int function_mengbai_copy_report_data(struct HandlePlate *h_plate, uint8 inv_type, char *errinfo)
{
	int result;
	result = jude_busnum_devnum_handle(h_plate);
	if (result < 0)
	{
		return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
	}
	result = mengbai_get_chao_bao_data(h_plate->hDev, inv_type, errinfo);
	if (result < 0)
	{
		printf("[-] Read chao bao qingka data failed\n");
	}
	//out("[+] Main finish\n");
	return result;
}
//�����忨
int function_aisino_report_summary_write_back(struct HandlePlate *h_plate, uint8 inv_type, char *errinfo)
{
	int result = -1;
	result = jude_busnum_devnum_handle(h_plate);
	if (result < 0)
	{
		return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
	}
	//out("���ߺţ�%d���豸�ţ�%d\n", nBusID, nDevID);
	
	result = aisino_report_clear(h_plate->hDev, inv_type, errinfo);
	if (result < 0)
	{
		;//printf("[-] function_aisino_report_summary failed result = %d\n", result);
	}
	return result;
}
int function_nisec_report_summary_write_back(struct HandlePlate *h_plate, uint8 inv_type, char *errinfo)
{
	int result = -1;
	result = jude_busnum_devnum_handle(h_plate);
	if (result < 0)
	{
		return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
	}
	//out("���ߺţ�%d���豸�ţ�%d\n", nBusID, nDevID);
	result = nisec_report_clear(h_plate->hDev, inv_type, errinfo);
	if (result < 0)
	{
		;//printf("[-] function_aisino_report_summary failed result = %d\n", result);
	}
	return result;
}
int function_cntax_report_summary_write_back(struct HandlePlate *h_plate, uint8 inv_type, char *errinfo)
{
	int result = -1;
	result = jude_busnum_devnum_handle(h_plate);
	if (result < 0)
	{
		return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
	}
	//out("���ߺţ�%d���豸�ţ�%d\n", nBusID, nDevID);
	result = cntax_report_clear(h_plate->hDev, inv_type, errinfo);
	if (result < 0)
	{
		;//printf("[-] function_aisino_report_summary failed result = %d\n", result);
	}
	return result;
}
int function_mengbai_report_summary_write_back(struct HandlePlate *h_plate, uint8 inv_type, char *errinfo)
{
	int result;
	result = jude_busnum_devnum_handle(h_plate);
	if (result < 0)
	{
		return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
	}
	result = mengbai_set_monitor_data(h_plate->hUSB, inv_type, errinfo);
	if (result < 0)
	{
		printf("[-] Read set monitor data failed\n");
	}
	//out("[+] Main finish\n");
	return result;
}
//���߷�Ʊ�ϴ�
int function_aisino_inv_upload_server(struct HandlePlate *h_plate, struct _offline_upload_result *offline_upload_result)
{
	int result = -1;
	result = jude_busnum_devnum_handle(h_plate);
	if (result < 0)
	{
		return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
	}
	result = aisino_inv_upload_server(h_plate->hDev, offline_upload_result);
	if (result < 0)
	{
		printf("[-] function_aisino_inv_upload_server failed result = %d\n", result);
	}
	return result;
}
int function_nisec_inv_upload_server(struct HandlePlate *h_plate, struct _offline_upload_result *offline_upload_result)
{
	int result = -1;
	result = jude_busnum_devnum_handle(h_plate);
	if (result < 0)
	{
		return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
	}
	result = nisec_inv_upload_server(h_plate->hDev, offline_upload_result);
	if (result < 0)
	{
		if (offline_upload_result->need_retryup == 1)
		{
			_WriteLog(LL_INFO, "�����ϴ�һ��\n");
			int need_retryup = offline_upload_result->need_retryup;
			memset(offline_upload_result, 0, sizeof(struct _offline_upload_result));
			offline_upload_result->need_retryup = need_retryup;
			result = nisec_inv_upload_server(h_plate->hDev, offline_upload_result);
			if (result < 0)
			{
				printf("[-] nisec_inv_upload_server failed result = %d\n", result);
			}
		}
	}
	return result;
}
int function_cntax_inv_upload_server(struct HandlePlate *h_plate, struct _offline_upload_result *offline_upload_result)
{
	int result = -1;
	result = jude_busnum_devnum_handle(h_plate);
	if (result < 0)
	{
		return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
	}
	result = cntax_inv_upload_server(h_plate->hDev, offline_upload_result);
	if (result < 0)
	{
		if (offline_upload_result->need_retryup == 1)
		{
			_WriteLog(LL_INFO, "�����ϴ�һ��\n");
			int need_retryup = offline_upload_result->need_retryup;
			memset(offline_upload_result, 0, sizeof(struct _offline_upload_result));
			offline_upload_result->need_retryup = need_retryup;
			result = cntax_inv_upload_server(h_plate->hDev, offline_upload_result);
			if (result < 0)
			{
				printf("[-] cntax_inv_upload_server failed result = %d\n", result);
			}
		}
	}
	return result;
}
int function_mengbai_inv_upload_server(struct HandlePlate *h_plate, struct _offline_upload_result *offline_upload_result)
{
	int result;
	result = jude_busnum_devnum_handle(h_plate);
	if (result < 0)
	{
		return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
	}
	result = mengbai_inv_upload_server(h_plate->hDev, offline_upload_result);
	if (result < 0)
	{
		printf("[-] function_mengbai_inv_upload_server failed\n");
	}
	return result;
}
//����Ʊ��������ϴ����߷�Ʊ
int function_aisino_fpdm_fphm_update_invs(struct HandlePlate *h_plate, char  *fpdm, char *fphm, int dzsyh, char **inv_json)
{
	int result = -1;
	result = jude_busnum_devnum_handle(h_plate);
	if (result < 0)
	{
		return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
	}
	result = aisino_fpdm_fphm_update_invs(h_plate->hDev, fpdm, fphm, dzsyh, inv_json);
	if (result < 0)
	{
		printf("[-] function_aisino_fpdm_fphm_update_invs failed result = %d\n", result);
	}
	return result;
}
int function_nisec_fpdm_fphm_update_invs(struct HandlePlate *h_plate, char  *fpdm, char *fphm, int dzsyh, char **inv_json)
{
	int result = -1;
	result = jude_busnum_devnum_handle(h_plate);
	if (result < 0)
	{
		return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
	}
	result = nisec_fpdm_fphm_update_invs(h_plate->hDev, fpdm, fphm, inv_json);
	if (result < 0)
	{
		printf("[-] function_nisec_fpdm_fphm_update_invs failed result = %d\n", result);
	}
	return result;
}
int function_cntax_fpdm_fphm_update_invs(struct HandlePlate *h_plate, char  *fpdm, char *fphm, int dzsyh, char **inv_json)
{
	int result = -1;
	result = jude_busnum_devnum_handle(h_plate);
	if (result < 0)
	{
		return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
	}
	result = cntax_fpdm_fphm_update_invs(h_plate->hDev, fpdm, fphm, inv_json);
	if (result < 0)
	{
		printf("[-] function_cntax_fpdm_fphm_update_invs failed result = %d\n", result);
	}
	return result;
}
//����˰�������������
int function_aisino_test_server_connect(struct HandlePlate *h_plate, char **splxxx, char *errinfo)
{
	int result = -1;
	result = jude_busnum_devnum_handle(h_plate);
	if (result < 0)
	{
		return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
	}
	result = aisino_check_server_address(h_plate->hDev, errinfo);
	if (result < 0)
	{
		printf("function_aisino_test_server_connect failed result = %d\n", result);
	}
	return result;
}
int function_nisec_test_server_connect(struct HandlePlate *h_plate, char **splxxx, char *errinfo)
{
	int result = -1;
	result = jude_busnum_devnum_handle(h_plate);
	if (result < 0)
	{
		return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
	}
	result = nisec_check_server_address(h_plate->hDev, splxxx, errinfo);
	if (result < 0)
	{
		printf("[-] function_nisec_test_server_connect failed result = %d\n", result);
	}
	return result;
}
int function_cntax_test_server_connect(struct HandlePlate *h_plate, char **splxxx, char *errinfo)
{
	int result = -1;
	result = jude_busnum_devnum_handle(h_plate);
	if (result < 0)
	{
		return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
	}
	result = cntax_check_server_address(h_plate->hDev, splxxx, errinfo);
	if (result < 0)
	{
		printf("[-] cntax_check_server_address failed result = %d\n", result);
	}
	return result;
}
int function_mengbai_test_server_connect(struct HandlePlate *h_plate, char **splxxx, char *errinfo)
{
	int result = -1;
	result = jude_busnum_devnum_handle(h_plate);
	if (result < 0)
	{
		return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
	}
	result = mengbai_check_server_address(h_plate->hDev, errinfo);
	if (result < 0)
	{
		printf("[-] function_mengbai_upload_m_server failed result = %d\n", result);
	}
	return result;
}
//������������Ʊ����
int function_aisino_make_invoice(struct HandlePlate *h_plate, unsigned char inv_type, char* inv_data, struct _plate_infos *plate_infos, struct _make_invoice_result *make_invoice_result)
{
	int result = -1;
	result = jude_busnum_devnum_handle(h_plate);
	if (result < 0)
	{
		sprintf((char *)make_invoice_result->errinfo, "USB����ʧ��,Bus��Ϣ����,������");
		return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
	}
	result = aisino_make_invoice(h_plate->hDev, inv_data, plate_infos, make_invoice_result);
	if (result < 0)
	{
		printf("[-] function_aisino_make_invoice failed result = %d\n", result);
	}
	else
	{		
		//out("��ʼ��ȡ�����Ϣ\n");
		aisino_read_monitor_info(h_plate->hUSB, h_plate->hDev, plate_infos);
		aisino_read_inv_coil_number(h_plate->hUSB, plate_infos);
		//out("������ȡ�����Ϣ\n");
	}
	if (make_invoice_result->need_restart == 1)
	{
		out("˰����Ҫ����,��bus ��Ϣ��Ϊ�쳣״̬\n");
		h_plate->nBusID = -1;
		h_plate->nDevID = -1;
	}
	return result;
}
int function_nisec_make_invoice(struct HandlePlate *h_plate, unsigned char inv_type, char* inv_data, struct _plate_infos *plate_infos, struct _make_invoice_result *make_invoice_result)
{
	int result = -1;
	result = jude_busnum_devnum_handle(h_plate);
	if (result < 0)
	{
		sprintf((char *)make_invoice_result->errinfo, "USB����ʧ��,Bus��Ϣ����,������");
		return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
	}
	_WriteLog(LL_INFO,"function_nisec_make_invoice test 1");
	result = nisec_make_invoice(h_plate->hDev, inv_data, plate_infos, make_invoice_result);
	if (result < 0)
	{
		printf("[-] function_nisec_make_invoice failed result = %d\n", result);
	}
	else
	{
		nisec_get_state_info(h_plate->hDev, plate_infos);
		nisec_read_inv_coil_number(h_plate->hDev, plate_infos);
	}
	if (make_invoice_result->need_restart == 1)
	{
		out("˰����Ҫ����,��bus ��Ϣ��Ϊ�쳣״̬\n");
		h_plate->nBusID = -1;
		h_plate->nDevID = -1;
	}
	return result;
}
int function_cntax_make_invoice(struct HandlePlate *h_plate, unsigned char inv_type, char* inv_data, struct _plate_infos *plate_infos, struct _make_invoice_result *make_invoice_result)
{
	int result = -1;
	result = jude_busnum_devnum_handle(h_plate);
	if (result < 0)
	{
		sprintf((char *)make_invoice_result->errinfo, "USB����ʧ��,Bus��Ϣ����,������");
		return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
	}
	result = cntax_make_invoice(h_plate->hDev, inv_data, plate_infos, make_invoice_result);
	if (result < 0)
	{
		printf("[-] function_cntax_make_invoice failed result = %d\n", result);
	}
	else
	{
		cntax_get_state_info(h_plate->hDev, plate_infos);
		cntax_read_inv_coil_number(h_plate->hDev, plate_infos);
	}
	if (make_invoice_result->need_restart == 1)
	{
		out("˰����Ҫ����,��bus ��Ϣ��Ϊ�쳣״̬\n");
		h_plate->nBusID = -1;
		h_plate->nDevID = -1;
	}
	return result;
}
int function_mengbai_make_invoice(struct HandlePlate *h_plate, unsigned char inv_type, char *inv_info, struct _plate_infos *plate_infos, struct _make_invoice_result *make_invoice_result)
{
	int result;
	result = jude_busnum_devnum_handle(h_plate);
	if (result < 0)
	{
		sprintf((char *)make_invoice_result->errinfo, "USB����ʧ��,Bus��Ϣ����,������");
		return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
	}
	result = mengbai_make_invoice_to_plate(h_plate->hDev, h_plate->hUSB, inv_type, inv_info, plate_infos, make_invoice_result);
	if (result < 0)
	{
		printf("[-] Read make invoice failed\n");
	}
	else
	{
		plate_infos->plate_basic_info.cert_passwd_right = 1;
		strcpy((char *)plate_infos->plate_tax_info.plate_time, h_plate->hDev->szDeviceTime);
		result = mengbai_read_inv_type_areacode(h_plate->hUSB, plate_infos);
		//out("��ȡ�������ɹ�\n");
		result = mengbai_read_monitor_info(h_plate->hUSB, plate_infos->plate_tax_info.inv_type, plate_infos);
		//out("��ȡ�����Ϣ�ɹ�\n");
		result = mengbai_read_inv_coil_number(h_plate->hUSB, plate_infos->plate_tax_info.inv_type, plate_infos);
		//out("��ȡ��Ʊ�����Ϣ�ɹ�\n");
		result = mengbai_read_offline_inv_num(h_plate->hUSB, plate_infos->plate_tax_info.inv_type, plate_infos);
		plate_infos->plate_tax_info.off_inv_num = result;
		plate_infos->plate_basic_info.plate_test = 1;
		result = 0;
	}
	if (make_invoice_result->need_restart == 1)
	{
		out("˰����Ҫ����,��bus ��Ϣ��Ϊ�쳣״̬\n");
		h_plate->nBusID = -1;
		h_plate->nDevID = -1;
	}
	return result;
}
//�ѿ���Ʊ����
int function_aisino_waste_invoice(struct HandlePlate *h_plate, unsigned char inv_type, char  *fpdm, char *fphm, char *zfr, struct _cancel_invoice_result *cancel_invoice_result)
{
	int result = -1;
	result = jude_busnum_devnum_handle(h_plate);
	if (result < 0)
	{
		return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
	}
	result = aisino_waste_invoice(h_plate->hDev, inv_type, fpdm, fphm, zfr, cancel_invoice_result);
	if (result < 0)
	{
		printf("[-] function_aisino_waste_invoice failed result = %d\n", result);
	}
	if (cancel_invoice_result->need_restart == 1)
	{
		out("˰����Ҫ����,��bus ��Ϣ��Ϊ�쳣״̬\n");
		h_plate->nBusID = -1;
		h_plate->nDevID = -1;
	}
	return result;
}
int function_nisec_waste_invoice(struct HandlePlate *h_plate, unsigned char inv_type, char  *fpdm, char *fphm, char *zfr, struct _cancel_invoice_result *cancel_invoice_result)
{
	int result = -1;
	result = jude_busnum_devnum_handle(h_plate);
	if (result < 0)
	{
		return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
	}
	result = nisec_waste_invoice(h_plate->hDev, inv_type, fpdm, fphm, cancel_invoice_result);
	if (result < 0)
	{
		printf("[-] function_nisec_waste_invoice failed result = %d\n", result);
	}
	if (cancel_invoice_result->need_restart == 1)
	{
		out("˰����Ҫ����,��bus ��Ϣ��Ϊ�쳣״̬\n");
		h_plate->nBusID = -1;
		h_plate->nDevID = -1;
	}
	return result;
}
int function_cntax_waste_invoice(struct HandlePlate *h_plate, unsigned char inv_type, char  *fpdm, char *fphm, char *zfr, struct _cancel_invoice_result *cancel_invoice_result)
{
	int result = -1;
	result = jude_busnum_devnum_handle(h_plate);
	if (result < 0)
	{
		return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
	}
	result = cntax_waste_invoice(h_plate->hDev, inv_type, fpdm, fphm, cancel_invoice_result);
	if (result < 0)
	{
		printf("[-] function_nisec_waste_invoice failed result = %d\n", result);
	}
	if (cancel_invoice_result->need_restart == 1)
	{
		out("˰����Ҫ����,��bus ��Ϣ��Ϊ�쳣״̬\n");
		h_plate->nBusID = -1;
		h_plate->nDevID = -1;
	}
	return result;
}
int function_mengbai_waste_invoice(struct HandlePlate *h_plate, unsigned char inv_type, char *fpdm, char *fphm, char *zfr, struct _cancel_invoice_result *cancel_invoice_result)
{
	int result;
	result = jude_busnum_devnum_handle(h_plate);
	if (result < 0)
	{
		return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
	}
	result = mengbai_cancel_invoice_to_plate(h_plate->hDev, inv_type, fpdm, fphm, cancel_invoice_result);
	if (result < 0)
	{
		printf("[-] Read cancel invoice failed\n");
	}
	if (cancel_invoice_result->need_restart == 1)
	{
		out("˰����Ҫ����,��bus ��Ϣ��Ϊ�쳣״̬\n");
		h_plate->nBusID = -1;
		h_plate->nDevID = -1;
	}
	return result;
}
//δ����Ʊ����
int function_aisino_waste_null_invoice(struct HandlePlate *h_plate, struct _plate_infos *plate_infos, struct _cancel_invoice_result *cancel_invoice_result)
{
	int result;
	result = jude_busnum_devnum_handle(h_plate);
	if (result < 0)
	{
		return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
	}
	result = aisino_cancel_null_invoice_to_plate(h_plate->hDev, cancel_invoice_result);
	if (result < 0)
	{
		printf("[-] Read cancel null invoice failed\n");
	}
	else
	{
		//out("��ʼ��ȡ�����Ϣ\n");
		aisino_read_monitor_info(h_plate->hUSB, h_plate->hDev, plate_infos);
		aisino_read_inv_coil_number(h_plate->hUSB, plate_infos);
		//out("������ȡ�����Ϣ\n");
	}
	if (cancel_invoice_result->need_restart == 1)
	{
		out("˰����Ҫ����,��bus ��Ϣ��Ϊ�쳣״̬\n");
		h_plate->nBusID = -1;
		h_plate->nDevID = -1;
	}
	return result;
}
int function_nisec_waste_null_invoice(struct HandlePlate *h_plate, struct _plate_infos *plate_infos, struct _cancel_invoice_result *cancel_invoice_result)
{
	int result;
	result = jude_busnum_devnum_handle(h_plate);
	if (result < 0)
	{
		return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
	}
	result = nisec_cancel_null_invoice_to_plate(h_plate->hDev, cancel_invoice_result);
	if (result < 0)
	{
		printf("[-] Read cancel null invoice failed\n");
	}
	else
	{
		nisec_get_state_info(h_plate->hDev, plate_infos);
		nisec_read_inv_coil_number(h_plate->hDev, plate_infos);
	}
	if (cancel_invoice_result->need_restart == 1)
	{
		out("˰����Ҫ����,��bus ��Ϣ��Ϊ�쳣״̬\n");
		h_plate->nBusID = -1;
		h_plate->nDevID = -1;
	}
	return result;
}
int function_cntax_waste_null_invoice(struct HandlePlate *h_plate, struct _plate_infos *plate_infos, struct _cancel_invoice_result *cancel_invoice_result)
{
	int result;
	result = jude_busnum_devnum_handle(h_plate);
	if (result < 0)
	{
		return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
	}
	result = cntax_cancel_null_invoice_to_plate(h_plate->hDev, cancel_invoice_result);
	if (result < 0)
	{
		printf("[-] Read cancel null invoice failed\n");
	}
	else
	{
		cntax_get_state_info(h_plate->hDev, plate_infos);
		cntax_read_inv_coil_number(h_plate->hDev, plate_infos);
	}
	if (cancel_invoice_result->need_restart == 1)
	{
		out("˰����Ҫ����,��bus ��Ϣ��Ϊ�쳣״̬\n");
		h_plate->nBusID = -1;
		h_plate->nDevID = -1;
	}
	return result;
}
int function_mengbai_waste_null_invoice(struct HandlePlate *h_plate, struct _plate_infos *plate_infos, struct _cancel_invoice_result *cancel_invoice_result)
{
	int result;
	result = jude_busnum_devnum_handle(h_plate);
	if (result < 0)
	{
		return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
	}
	result = mengbai_cancel_null_invoice_to_plate(h_plate->hUSB, cancel_invoice_result);
	if (result < 0)
	{
		printf("[-] Read cancel null invoice failed\n");
	}
	else
	{
		plate_infos->plate_basic_info.cert_passwd_right = 1;
		strcpy((char *)plate_infos->plate_tax_info.plate_time, h_plate->hDev->szDeviceTime);
		result = mengbai_read_inv_type_areacode(h_plate->hUSB, plate_infos);
		//out("��ȡ�������ɹ�\n");
		result = mengbai_read_monitor_info(h_plate->hUSB, plate_infos->plate_tax_info.inv_type, plate_infos);
		//out("��ȡ�����Ϣ�ɹ�\n");
		result = mengbai_read_inv_coil_number(h_plate->hUSB, plate_infos->plate_tax_info.inv_type, plate_infos);
		//out("��ȡ��Ʊ�����Ϣ�ɹ�\n");
		result = mengbai_read_offline_inv_num(h_plate->hUSB, plate_infos->plate_tax_info.inv_type, plate_infos);
		plate_infos->plate_tax_info.off_inv_num = result;
		plate_infos->plate_basic_info.plate_test = 1;
		result = 0;
	}
	if (cancel_invoice_result->need_restart == 1)
	{
		out("˰����Ҫ����,��bus ��Ϣ��Ϊ�쳣״̬\n");
		h_plate->nBusID = -1;
		h_plate->nDevID = -1;
	}
	return result;
}

//������ַ�Ʊ��������Ƿ���Կ��ߺ�Ʊ
int function_aisino_check_lzfpdm_lzfphm_allow(struct HandlePlate *h_plate, unsigned char inv_type, char *lzfpdm, char *lzfphm, uint32 dzsyh, char *errinfo)
{
	int result = -1;
	result = jude_busnum_devnum_handle(h_plate);
	if (result < 0)
	{
		return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
	}
	result = aisino_redinv_check(h_plate->hDev, inv_type, lzfpdm, lzfphm, dzsyh, errinfo);
	if (result < 0)
	{
		printf("[-] function_aisino_check_lzfpdm_lzfphm_allow failed result = %d\n", result);
	}
	return result;
}


//������Ϣ������
int function_aisino_upload_hzxxb(struct HandlePlate *h_plate, uint8 inv_type, char* inv_data, struct _plate_infos *plate_infos, struct _askfor_tzdbh_result *askfor_tzdbh_result)
{
	int result = -1;
	result = jude_busnum_devnum_handle(h_plate);
	if (result < 0)
	{
		return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
	}
	result = aisino_upload_check_redinvform(h_plate->hDev, inv_type, inv_data, plate_infos, askfor_tzdbh_result);
	if (result < 0)
	{
		printf("[-] function_aisino_upload_hzxxb failed result = %d\n", result);
	}
	return result;
}
int function_nisec_upload_hzxxb(struct HandlePlate *h_plate, uint8 inv_type, char* inv_data, struct _plate_infos *plate_infos, struct _askfor_tzdbh_result *askfor_tzdbh_result)
{
	int result = -1;
	result = jude_busnum_devnum_handle(h_plate);
	if (result < 0)
	{
		return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
	}
	result = nisec_upload_check_redinvform(h_plate->hDev, inv_type,inv_data, plate_infos, askfor_tzdbh_result);
	if (result < 0)
	{
		printf("[-] nisec_upload_check_redinvform failed result = %d\n", result);
	}
	return result;
}
int function_cntax_upload_hzxxb(struct HandlePlate *h_plate, uint8 inv_type, char* inv_data, struct _plate_infos *plate_infos, struct _askfor_tzdbh_result *askfor_tzdbh_result)
{
	int result = -1;
	result = jude_busnum_devnum_handle(h_plate);
	if (result < 0)
	{
		return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
	}
	result = cntax_upload_check_redinvform(h_plate->hDev, inv_type,inv_data, plate_infos, askfor_tzdbh_result);
	if (result < 0)
	{
		printf("[-] cntax_upload_check_redinvform failed result = %d\n", result);
	}
	return result;
}
//ͨ��������Ϣ���Ų�ѯ������Ϣ
int function_aisino_download_hzxxb_from_rednum(struct HandlePlate *h_plate, char *redNum, uint8 **inv_json, char *errinfo)
{
	int result = -1;
	result = jude_busnum_devnum_handle(h_plate);
	if (result < 0)
	{
		return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
	}
	result = aisino_download_hzxxb_from_rednum(h_plate->hDev, redNum, inv_json, errinfo);
	if (result < 0)
	{
		printf("[-] function_aisino_download_hzxxb_from_rednum failed result = %d\n", result);
	}
	return result;
}
int function_nisec_download_hzxxb_from_rednum(struct HandlePlate *h_plate, char *redNum, uint8 **inv_json, char *errinfo)
{
	int result = -1;
	result = jude_busnum_devnum_handle(h_plate);
	if (result < 0)
	{
		return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
	}
	result = nisec_download_hzxxb_from_rednum(h_plate->hDev, redNum, inv_json, errinfo);
	if (result < 0)
	{
		printf("[-] function_nisec_download_hzxxb_from_rednum failed result = %d\n", result);
	}
	return result;
}
int function_cntax_download_hzxxb_from_rednum(struct HandlePlate *h_plate, char *redNum, uint8 **inv_json, char *errinfo)
{
	int result = -1;
	result = jude_busnum_devnum_handle(h_plate);
	if (result < 0)
	{
		return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
	}
	result = cntax_download_hzxxb_from_rednum(h_plate->hDev, redNum, inv_json, errinfo);
	if (result < 0)
	{
		printf("[-] function_nisec_download_hzxxb_from_rednum failed result = %d\n", result);
	}
	return result;
}
//ͨ��ʱ��β�ѯ������Ϣ��������Ϣ
int function_aisino_download_hzxxb_from_date_range(struct HandlePlate *h_plate, char *date_range, uint8 **tzdbh_data, char *errinfo)
{
	int result = -1;
	result = jude_busnum_devnum_handle(h_plate);
	if (result < 0)
	{
		return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
	}
	//out("���ߺţ�%d���豸�ţ�%d\n", nBusID, nDevID);
	result = aisino_download_hzxxb_from_date_range(h_plate->hDev, date_range, tzdbh_data, errinfo);
	if (result < 0)
	{
		printf("function_aisino_download_hzxxb_from_date_range failed result = %d\n", result);
	}
	return result;
}
int function_aisino_download_hzxxb_from_date_range_new(struct HandlePlate *h_plate, char *date_range,char *gfsh, uint8 **tzdbh_data, char *errinfo)
{
	int result = -1;
	result = jude_busnum_devnum_handle(h_plate);
	if (result < 0)
	{
		return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
	}
	//out("���ߺţ�%d���豸�ţ�%d\n", nBusID, nDevID);
	result = aisino_download_hzxxb_from_date_range_new(h_plate->hDev, date_range, gfsh,tzdbh_data, errinfo);
	if (result < 0)
	{
		printf("function_aisino_download_hzxxb_from_date_range_new failed result = %d\n", result);
	}
	return result;
}
int function_nisec_download_hzxxb_from_date_range(struct HandlePlate *h_plate, char *date_range, uint8 **tzdbh_data, char *errinfo)
{
	int result = -1; 
	result = jude_busnum_devnum_handle(h_plate);
	if (result < 0)
	{
		return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
	}
	//out("���ߺţ�%d���豸�ţ�%d\n", nBusID, nDevID);
	result = nisec_download_hzxxb_from_date_range(h_plate->hDev, date_range, tzdbh_data, errinfo);
	if (result < 0)
	{
		printf("[-] nisec_download_hzxxb_from_date_range failed result = %d\n", result);
	}
	return result;
}
int function_cntax_download_hzxxb_from_date_range(struct HandlePlate *h_plate, char *date_range, uint8 **tzdbh_data, char *errinfo)
{
	int result = -1;
	result = jude_busnum_devnum_handle(h_plate);
	if (result < 0)
	{
		return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
	}
	//out("���ߺţ�%d���豸�ţ�%d\n", nBusID, nDevID);
	result = cntax_download_hzxxb_from_date_range(h_plate->hDev, date_range, tzdbh_data, errinfo);
	if (result < 0)
	{
		printf("[-] cntax_download_hzxxb_from_date_range failed result = %d\n", result);
	}
	return result;
}
//����������Ϣ������
int function_aisino_cancel_hzxxb(struct HandlePlate *h_plate, char *redNum, char *errinfo)
{
	int result = -1;
	result = jude_busnum_devnum_handle(h_plate);
	if (result < 0)
	{
		return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
	}
	result = aisino_redinv_tzdbh_cancel(h_plate->hDev, redNum, errinfo);
	if (result < 0)
	{
		printf("[-] function_aisino_cancel_hzxxb failed result = %d\n", result);
	}
	return result;
}
int function_nisec_cancel_hzxxb(struct HandlePlate *h_plate, char *redNum, char *errinfo)
{
	int result = -1;
	result = jude_busnum_devnum_handle(h_plate);
	if (result < 0)
	{
		return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
	}
	result = nisec_redinv_tzdbh_cancel(h_plate->hDev, redNum, errinfo);
	if (result < 0)
	{
		printf("[-] nisec_redinv_tzdbh_cancel failed result = %d\n", result);
	}
	return result;
}
int function_cntax_cancel_hzxxb(struct HandlePlate *h_plate, char *redNum, char *errinfo)
{
	int result = -1;
	result = jude_busnum_devnum_handle(h_plate);
	if (result < 0)
	{
		return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
	}
	result = cntax_redinv_tzdbh_cancel(h_plate->hDev, redNum, errinfo);
	if (result < 0)
	{
		printf("[-] nisec_redinv_tzdbh_cancel failed result = %d\n", result);
	}
	return result;
}


//���߷�Ʊ���ò�ѯ
int function_aisino_query_net_invoice_coil(struct HandlePlate *h_plate, char *date_range, char *inv_data, char *errinfo)
{
	int result = -1;
	result = jude_busnum_devnum_handle(h_plate);
	if (result < 0)
	{
		return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
	}
	result = aisino_query_net_invoice_coil(h_plate->hDev, date_range, inv_data, errinfo);
	if (result < 0)
	{
		printf("function_aisino_query_net_invoice_coil failed result = %d\n", result);
	}
	return result;
}
int function_nisec_query_net_invoice_coil(struct HandlePlate *h_plate, char *date_range, char *inv_data, char *errinfo)
{
	int result = -1;
	result = jude_busnum_devnum_handle(h_plate);
	if (result < 0)
	{
		return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
	}
	result = nisec_query_net_invoice_coil(h_plate->hDev, date_range, inv_data, errinfo);
	if (result < 0)
	{
		printf("function_nisec_query_net_invoice_coil failed result = %d\n", result);
	}
	return result;
}
int function_cntax_query_net_invoice_coil(struct HandlePlate *h_plate, char *date_range, char *inv_data, char *errinfo)
{
	int result = -1;
	result = jude_busnum_devnum_handle(h_plate);
	if (result < 0)
	{
		return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
	}
	result = cntax_query_net_invoice_coil(h_plate->hDev, date_range, inv_data, errinfo);
	if (result < 0)
	{
		printf("function_cntax_query_net_invoice_coil failed result = %d\n", result);
	}
	return result;
}

//���߷�Ʊ����
int function_aisino_net_invoice_coil_download_unlock(struct HandlePlate *h_plate, char *date_range, uint8 fllxdm, char *fpdm, char *fpqshm, int fpzs, char *errinfo)
{
	int result = -1;
	result = jude_busnum_devnum_handle(h_plate);
	if (result < 0)
	{
		return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
	}
	result = aisino_query_net_invoice_coil_download_unlock(h_plate->hDev, date_range, fllxdm, fpdm, fpqshm, fpzs, errinfo);
	if (result < 0)
	{
		printf("function_aisino_query_net_invoice_coil_download_unlock failed result = %d\n", result);
	}
	return result;
}

int function_nisec_net_invoice_coil_download_unlock(struct HandlePlate *h_plate, char *date_range, uint8 fllxdm, char *fpdm, char *fpqshm, int fpzs, char *errinfo)
{
	int result = -1;
	result = jude_busnum_devnum_handle(h_plate);
	if (result < 0)
	{
		return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
	}
	result = nisec_query_net_invoice_coil_download_unlock(h_plate->hDev, date_range, fllxdm, fpdm, fpqshm, fpzs, errinfo);
	if (result < 0)
	{
		printf("function_nisec_query_net_invoice_coil_download_unlock failed result = %d\n", result);
	}
	return result;
}
int function_cntax_net_invoice_coil_download_unlock(struct HandlePlate *h_plate, char *date_range, uint8 fllxdm, char *fpdm, char *fpqshm, int fpzs, char *errinfo)
{
	int result = -1;
	result = jude_busnum_devnum_handle(h_plate);
	if (result < 0)
	{
		return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
	}
	result = cntax_query_net_invoice_coil_download_unlock(h_plate->hDev, date_range, fllxdm, fpdm, fpqshm, fpzs, errinfo);
	if (result < 0)
	{
		printf("function_nisec_query_net_invoice_coil_download_unlock failed result = %d\n", result);
	}
	return result;
}

//������빫������ƽ̨
int function_aisino_connect_pubservice(struct HandlePlate *h_plate, uint8 inv_type, char *errinfo)
{
	int result = -1;
	result = jude_busnum_devnum_handle(h_plate);
	if (result < 0)
	{
		return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
	}
	result = aisino_connect_pubservice(h_plate->hDev, inv_type, errinfo);
	if (result < 0)
	{
		printf("function_aisino_query_net_invoice_coil_download_unlock failed result = %d\n", result);
	}
	return result;
}
int function_nisec_connect_pubservice(struct HandlePlate *h_plate, uint8 inv_type, char *errinfo)
{
	int result = -1;
	result = jude_busnum_devnum_handle(h_plate);
	if (result < 0)
	{
		return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
	}
	result = nisec_connect_pubservice(h_plate->hDev, inv_type, errinfo);
	if (result < 0)
	{
		printf("function_aisino_query_net_invoice_coil_download_unlock failed result = %d\n", result);
	}
	return result;
}
//�������빫������ƽ̨
int function_aisino_disconnect_pubservice(struct HandlePlate *h_plate, uint8 inv_type, char *errinfo)
{
	int result = -1;
	result = jude_busnum_devnum_handle(h_plate);
	if (result < 0)
	{
		return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
	}
	result = aisino_disconnect_pubservice(h_plate->hDev, inv_type,errinfo);
	if (result < 0)
	{
		printf("function_aisino_query_net_invoice_coil_download_unlock failed result = %d\n", result);
	}
	return result;
}
int function_nisec_disconnect_pubservice(struct HandlePlate *h_plate, uint8 inv_type, char *errinfo)
{
	int result = -1;
	result = jude_busnum_devnum_handle(h_plate);
	if (result < 0)
	{
		return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
	}
	result = nisec_disconnect_pubservice(h_plate->hDev, inv_type, errinfo);
	if (result < 0)
	{
		printf("function_aisino_query_net_invoice_coil_download_unlock failed result = %d\n", result);
	}
	return result;
}
//��������������--hello
int function_aisino_client_hello(struct HandlePlate *h_plate, char *client_hello, char *errinfo)
{
	int result = -1;
	result = jude_busnum_devnum_handle(h_plate);
	if (result < 0)
	{
		return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
	}
	result = aisino_client_hello(h_plate->hDev, client_hello,errinfo);
	if (result < 0)
	{
		printf("function_aisino_client_hello failed result = %d\n", result);
	}
	return result;
}

int function_nisec_client_hello(struct HandlePlate *h_plate, char *client_hello, char *errinfo)
{
	int result = -1;
	result = jude_busnum_devnum_handle(h_plate);
	if (result < 0)
	{
		return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
	}
	result = nisec_client_hello(h_plate->hDev, client_hello,errinfo);
	if (result < 0)
	{
		printf("function_nisec_client_hello failed result = %d\n", result);
	}
	return result;
}

int function_cntax_client_hello(struct HandlePlate *h_plate, char *client_hello, char *errinfo)
{
	int result = -1;
	result = jude_busnum_devnum_handle(h_plate);
	if (result < 0)
	{
		return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
	}
	result = cntax_client_hello(h_plate->hDev, client_hello,errinfo);
	if (result < 0)
	{
		printf("function_cntax_client_hello failed result = %d\n", result);
	}
	return result;
}

//������������֤--auth
int function_aisino_client_auth(struct HandlePlate *h_plate, char *server_hello, char *client_auth, char *errinfo)
{
	int result = -1;
	result = jude_busnum_devnum_handle(h_plate);
	if (result < 0)
	{
		return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
	}
	result = aisino_client_auth(h_plate->hDev, server_hello, client_auth, errinfo);
	if (result < 0)
	{
		printf("function_aisino_client_hello failed result = %d\n", result);
	}
	return result;
}

int function_nisec_client_auth(struct HandlePlate *h_plate, char *server_hello, char *client_auth, char *errinfo)
{
	int result = -1;
	result = jude_busnum_devnum_handle(h_plate);
	if (result < 0)
	{
		return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
	}
	result = nisec_client_auth(h_plate->hDev, server_hello, client_auth,errinfo);
	if (result < 0)
	{
		printf("function_aisino_client_hello failed result = %d\n", result);
	}
	return result;
}

int function_cntax_client_auth(struct HandlePlate *h_plate, char *server_hello, char *client_auth, char *errinfo)
{
	int result = -1;
	result = jude_busnum_devnum_handle(h_plate);
	if (result < 0)
	{
		return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
	}
	result = cntax_client_auth(h_plate->hDev, server_hello, client_auth,errinfo);
	if (result < 0)
	{
		printf("function_aisino_client_hello failed result = %d\n", result);
	}
	return result;
}

//��ȡ˰��֤������
int function_aisino_get_cert(struct HandlePlate *h_plate, char *cert_data, char *errinfo)
{
	int result = -1;
	result = jude_busnum_devnum_handle(h_plate);
	if (result < 0)
	{
		return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
	}
	result = aisino_get_cert(h_plate->hDev, cert_data, errinfo);
	if (result < 0)
	{
		printf("function_aisino_client_hello failed result = %d\n", result);
	}
	return result;
}

int function_nisec_get_cert(struct HandlePlate *h_plate, char *cert_data, char *errinfo)
{
	int result = -1;
	result = jude_busnum_devnum_handle(h_plate);
	if (result < 0)
	{
		return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
	}
	result = nisec_get_cert(h_plate->hDev, cert_data, errinfo);
	if (result < 0)
	{
		printf("function_aisino_client_hello failed result = %d\n", result);
	}
	return result;
}

int function_cntax_get_cert(struct HandlePlate *h_plate, char *cert_data, char *errinfo)
{
	int result = -1;
	result = jude_busnum_devnum_handle(h_plate);
	if (result < 0)
	{
		return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
	}
	result = cntax_get_cert(h_plate->hDev, cert_data, errinfo);
	if (result < 0)
	{
		printf("function_aisino_client_hello failed result = %d\n", result);
	}
	return result;
}

//���»�ȡ˰�̻�����Ϣ�ӿ�
int function_aisino_query_invoice_month_all_data(struct HandlePlate *h_plate, char *month, char **data_json, char *errinfo)
{
	int result = -1;
	result = jude_busnum_devnum_handle(h_plate);
	if (result < 0)
	{
		return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
	}
	result = aisino_query_invoice_month_all_data(h_plate->hDev, month, data_json, errinfo);
	if (result < 0)
	{
		printf("function_aisino_query_net_invoice_coil_download_unlock failed result = %d\n", result);
	}
	return result;
}
int function_nisec_query_invoice_month_all_data(struct HandlePlate *h_plate, char *month, char **data_json, char *errinfo)
{
	int result = -1;
	result = jude_busnum_devnum_handle(h_plate);
	if (result < 0)
	{
		return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
	}
	result = nisec_query_invoice_month_all_data(h_plate->hDev, month, data_json, errinfo);
	if (result < 0)
	{
		printf("function_aisino_query_net_invoice_coil_download_unlock failed result = %d\n", result);
	}
	return result;
}
int function_cntax_query_invoice_month_all_data(struct HandlePlate *h_plate, char *month, char **data_json, char *errinfo)
{
	int result = -1;
	result = jude_busnum_devnum_handle(h_plate);
	if (result < 0)
	{
		return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
	}
	result = cntax_query_invoice_month_all_data(h_plate->hDev, month, data_json, errinfo);
	if (result < 0)
	{
		printf("function_aisino_query_net_invoice_coil_download_unlock failed result = %d\n", result);
	}
	return result;
}

//����ʷƱ����Ϣ
int function_nisec_read_inv_coil_number_history(struct HandlePlate *h_plate,char **data_json, char *errinfo)
{
	int result = -1;
	result = jude_busnum_devnum_handle(h_plate);
	if (result < 0)
	{
		return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
	}
	result = nisec_read_inv_coil_number_history(h_plate->hDev, data_json, errinfo);
	if (result < 0)
	{
		printf("function_nisec_read_inv_coil_number_history failed result = %d\n", result);
	}
	return result;
}

int function_cntax_read_inv_coil_number_history(struct HandlePlate *h_plate, char **data_json, char *errinfo)
{
	int result = -1;
	result = jude_busnum_devnum_handle(h_plate);
	if (result < 0)
	{
		return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
	}
	result = cntax_read_inv_coil_number_history(h_plate->hDev, data_json, errinfo);
	if (result < 0)
	{
		printf("function_cntax_read_inv_coil_number_history failed result = %d\n", result);
	}
	return result;
}

//��ȡ�ɰط�����key������Ϣ
int function_mengbai_server_key_info(struct HandlePlate *h_plate, char *info)
{
	int result;
	result = jude_busnum_devnum_handle(h_plate);
	if (result < 0)
	{
		return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
	}
	result = mengbai_get_server_key_info(h_plate->hUSB, h_plate->hDev, info);
	if (result < 0)
	{
		printf("[-] function_mengbai_server_key_info failed\n");
	}
	return result;
}

//һ����¼
int function_common_etax_login_Interface(char *busid,char *related_type,char *full_name,char *mobile,char *idcard,char *sz_password,char *errinfo,char *lpgoto,char *lpcode,char *lptoken,char *sys_type,char *svr_container,char *soft_cert_ip,int soft_cert_port,char *login_method,char *area_code)
{
	int i = 0 , nRet = -1;
	for(i=0;i<5;i++){
		nRet = etax_login_Interface(busid,related_type,full_name,mobile,idcard,sz_password,errinfo,lpgoto,lpcode,lptoken,sys_type,svr_container,"172.30.92.126",soft_cert_port,login_method,area_code);
		if(nRet != RET_SUCCESS){
			if(strstr(errinfo,"����Կ���߷���SM4��Կʧ��"))
			{
				continue;
			}
			else return nRet;
		}
		else return nRet;
	}

	return nRet;
}

//�û���ѯ
int function_common_etax_user_query_Interface(char *busid,char *errinfo,char **outinfo,char *svr_container,char *soft_cert_ip,int soft_cert_port,char *login_method,char *area_code)
{
	int i = 0 , nRet = -1;
	for(i=0;i<5;i++){	
		nRet =  etax_user_query_Interface(busid,errinfo,outinfo,svr_container,"172.30.92.126",soft_cert_port,login_method,area_code);
		if(nRet != RET_SUCCESS){
			if(strstr(errinfo,"����Կ���߷���SM4��Կʧ��"))
			{
				continue;
			}
			else return nRet;
		}
		else return nRet;		
	}
	return nRet;
}

//��ҵ�б��ѯ
int function_common_etax_relationlist_Interface(char *busid,char *related_type,char *full_name,char *mobile,char *idcard,char *sz_password,char *errinfo,char **outenterpriselist,char *svr_container,char *soft_cert_ip,int soft_cert_port,char *login_method,char *area_code)
{
	int i = 0 , nRet = -1;
	for(i=0;i<5;i++){
		nRet = etax_relationlist_Interface(busid,related_type,full_name,mobile,idcard,sz_password,errinfo,outenterpriselist,svr_container,"172.30.92.126",soft_cert_port,login_method,area_code);
		if(nRet != RET_SUCCESS){
			if(strstr(errinfo,"����Կ���߷���SM4��Կʧ��"))
			{
				continue;
			}
			else return nRet;
		}
		else return nRet;
	}
	return nRet;
}

//��ҵ�л�
int function_common_etax_relationchange_Interface(char *busid,char *related_type,char *full_name,char *mobile,char *idcard,char *sz_password,char *errinfo,char *uniqueIdentity,char *realationStatus,char *lpgoto,char *lpcode,char *lptoken,char *sys_type,char *svr_container,char *soft_cert_ip,int soft_cert_port,char *login_method,char *area_code)
{
	int i = 0 , nRet = -1;
	for(i=0;i<5;i++){
		nRet = etax_relationchange_Interface(busid,related_type,full_name,mobile,idcard,sz_password,errinfo,uniqueIdentity,realationStatus,lpgoto,lpcode,lptoken,sys_type,svr_container,"172.30.92.126",soft_cert_port,login_method,area_code);
		if(nRet != RET_SUCCESS){
			if(strstr(errinfo,"����Կ���߷���SM4��Կʧ��"))
			{
				continue;
			}
			else return nRet;
		}
		else return nRet;
	}
	return nRet;
}

//��Ʊ��ѯ�б�
int function_common_etax_invoice_query_Interface(char *busid,char *fpcxsj,char *related_type,char *full_name,char *mobile,char *idcard,char *sz_password,int realationtype,char *uniqueIdentity,char *realationStatus,char *errinfo,char **fpcxdata,char *area_code,char *etax_cookie,char *tpass_cookie,char *dppt_cookie,char *new_etax_cookie,char *new_tpass_cookie,char *new_dppt_cookie,int use_cookie_flag)
{
	int i = 0 , nRet = -1;
	if(use_cookie_flag){
		for(i=0;i<5;i++){
			nRet = etax_invoice_query_by_cookie_Interface(area_code,etax_cookie,tpass_cookie,dppt_cookie,new_etax_cookie,new_tpass_cookie,new_dppt_cookie,fpcxsj,fpcxdata,errinfo);
			if(nRet != RET_SUCCESS){
				if(strstr(errinfo,"502 Bad Gateway") || strstr(errinfo,"��ʼ��dppt������ʧ��") || strstr(errinfo,"����Կ���߷���SM4��Կʧ��"))
				{
					continue;
				}
				else return nRet;
			}
			else return nRet;
		}
	}
	else return etax_invoice_query_Interface(busid,fpcxsj,related_type,full_name,mobile,idcard,sz_password,realationtype,uniqueIdentity,realationStatus,errinfo,fpcxdata);
}

//��Ʊ��ѯ����
int function_common_etax_invoice_query_detail_Interface(char *busid,char *fpcxsj,char *related_type,char *full_name,char *mobile,char *idcard,char *sz_password,int realationtype,char *uniqueIdentity,char *realationStatus,char *errinfo,char **fpcxdata,char *area_code,char *etax_cookie,char *tpass_cookie,char *dppt_cookie,char *new_etax_cookie,char *new_tpass_cookie,char *new_dppt_cookie,int use_cookie_flag)
{
	int i = 0 , nRet = -1;
	if(use_cookie_flag){
		for(i=0;i<5;i++){
			nRet = etax_invoice_query_detail_by_cookie_Interface(area_code,etax_cookie,tpass_cookie,dppt_cookie,new_etax_cookie,new_tpass_cookie,new_dppt_cookie,fpcxsj,fpcxdata,errinfo);
			if(nRet != RET_SUCCESS){
				if(strstr(errinfo,"502 Bad Gateway") || strstr(errinfo,"��ʼ��dppt������ʧ��") || strstr(errinfo,"����Կ���߷���SM4��Կʧ��"))
				{
					continue;
				}
				else return nRet;
			}
			else return nRet;
		}
	}
	else return etax_invoice_query_detail_Interface(busid,fpcxsj,related_type,full_name,mobile,idcard,sz_password,realationtype,uniqueIdentity,realationStatus,errinfo,fpcxdata);
}

//��Ʊ��ѯ����
int function_common_etax_invoice_query_down_Interface(char *busid,char *fpcxsj,char *related_type,char *full_name,char *mobile,char *idcard,char *sz_password,int realationtype,char *uniqueIdentity,char *realationStatus,char *errinfo,char **fpcxdata,char **fpcxname,char *area_code,char *etax_cookie,char *tpass_cookie,char *dppt_cookie,char *new_etax_cookie,char *new_tpass_cookie,char *new_dppt_cookie,int use_cookie_flag)
{
	int i = 0 , nRet = -1;
	if(use_cookie_flag){
		for(i=0;i<5;i++){
			nRet = etax_invoice_query_down_by_cookie_Interface(area_code,etax_cookie,tpass_cookie,dppt_cookie,new_etax_cookie,new_tpass_cookie,new_dppt_cookie,fpcxsj,errinfo,fpcxdata,fpcxname);
			if(nRet != RET_SUCCESS){
				if(strstr(errinfo,"502 Bad Gateway") || strstr(errinfo,"��ʼ��dppt������ʧ��") || strstr(errinfo,"����Կ���߷���SM4��Կʧ��"))
				{
					continue;
				}
				else return nRet;
			}
			else return nRet;
		}
	}
	else return etax_invoice_query_down_Interface(busid,fpcxsj,related_type,full_name,mobile,idcard,sz_password,realationtype,uniqueIdentity,realationStatus,errinfo,fpcxdata,fpcxname);
}

//��Ʊ����RPAɨ��
int function_common_etax_rpa_invoice_issuance_Interface(char *busid,char *fpkjurl,char *nsrsbh,char *related_type,char *mobile,char *idcard,char *sz_password,int realationtype,char *uniqueIdentity,char *realationStatus,char *errinfo,char **fpkjdata,char *area_code,char *etax_cookie,char *tpass_cookie,char *dppt_cookie,char *new_etax_cookie,char *new_tpass_cookie,char *new_dppt_cookie,int use_cookie_flag)
{
	int i = 0 , nRet = -1;
	if(use_cookie_flag){
		for(i=0;i<5;i++){
			nRet = etax_invoice_rpa_invoice_issuance_by_cookie_Interface(area_code,etax_cookie,tpass_cookie,dppt_cookie,new_etax_cookie,new_tpass_cookie,new_dppt_cookie,errinfo,fpkjurl,nsrsbh,fpkjdata);
			if(nRet != RET_SUCCESS){
				if(strstr(errinfo,"502 Bad Gateway") || strstr(errinfo,"��ʼ��dppt������ʧ��") || strstr(errinfo,"����Կ���߷���SM4��Կʧ��"))
				{
					continue;
				}
				else return nRet;
			}
			else return nRet;
		}
	}
	else return etax_rpa_invoice_issuance_Interface(busid,fpkjurl,nsrsbh,related_type,mobile,idcard,sz_password,realationtype,uniqueIdentity,realationStatus,errinfo,fpkjdata);
}


int function_common_etax_rpa_invoice_issuance_status_Interface(char *busid,char *rzid, char *app_code,char *fpkjurl,char *nsrsbh,char *related_type,char *mobile,char *idcard,char *sz_password,int realationtype,char *uniqueIdentity,char *realationStatus,char *errinfo,char **fpkjdata,char *area_code,char *etax_cookie,char *tpass_cookie,char *dppt_cookie,char *new_etax_cookie,char *new_tpass_cookie,char *new_dppt_cookie,int use_cookie_flag)
{
	int i = 0 , nRet = -1;
	if(use_cookie_flag){
		for(i=0;i<5;i++){
			nRet = etax_invoice_rpa_invoice_issuance_status_by_cookie_Interface(area_code,rzid,app_code,etax_cookie,tpass_cookie,dppt_cookie,new_etax_cookie,new_tpass_cookie,new_dppt_cookie,errinfo,fpkjurl,nsrsbh,fpkjdata);
			if(nRet != RET_SUCCESS){
				if(strstr(errinfo,"502 Bad Gateway") || strstr(errinfo,"��ʼ��dppt������ʧ��") || strstr(errinfo,"����Կ���߷���SM4��Կʧ��"))
				{
					continue;
				}
				else return nRet;
			}
			else return nRet;
		}
	}
	else return etax_invoice_rpa_invoice_issuance_status_by_cookie_Interface(area_code,rzid,app_code,etax_cookie,tpass_cookie,dppt_cookie,new_etax_cookie,new_tpass_cookie,new_dppt_cookie,errinfo,fpkjurl,nsrsbh,fpkjdata);
}

//���Ŷ�Ȳ�ѯ
int function_common_credit_limit_Interface(char *busid,char *nsrsbh,char *related_type,char *mobile,char *idcard,char *sz_password,int realationtype,char *uniqueIdentity,char *realationStatus,char *errinfo,char **sxeddata,char *area_code,char *etax_cookie,char *tpass_cookie,char *dppt_cookie,char *new_etax_cookie,char *new_tpass_cookie,char *new_dppt_cookie,int use_cookie_flag)
{
	int i = 0 , nRet = -1;
	if(use_cookie_flag){
		for(i=0;i<5;i++){
			nRet = etax_credit_limit_by_cookie_Interface(area_code,etax_cookie,tpass_cookie,dppt_cookie,new_etax_cookie,new_tpass_cookie,new_dppt_cookie,errinfo,nsrsbh,sxeddata);
			if(nRet != RET_SUCCESS){
				if(strstr(errinfo,"502 Bad Gateway") || strstr(errinfo,"��ʼ��dppt������ʧ��") || strstr(errinfo,"����Կ���߷���SM4��Կʧ��"))
				{
					continue;
				}
				else return nRet;
			}
			else return nRet;
		}
	}
	else return etax_credit_limit_Interface(busid,nsrsbh,related_type,mobile,idcard,sz_password,realationtype,uniqueIdentity,realationStatus,errinfo,sxeddata);
}

//���ַ�Ʊ����
int function_common_etax_blue_ink_invoice_issuance_Interface(char *busid,char *fpkjjson,char *related_type,char *mobile,char *idcard,char *sz_password,int realationtype,char *uniqueIdentity,char *realationStatus,char *errinfo,char **fpkjout,char **hqnsrjcxx,char *area_code,char *etax_cookie,char *tpass_cookie,char *dppt_cookie,char *new_etax_cookie,char *new_tpass_cookie,char *new_dppt_cookie,int use_cookie_flag)
{
	int i = 0 , nRet = -1;
	if(use_cookie_flag){
		for(i=0;i<5;i++){
			nRet = etax_blue_ink_invoice_issuance_by_cookie_Interface(area_code,etax_cookie,tpass_cookie,dppt_cookie,new_etax_cookie,new_tpass_cookie,new_dppt_cookie,errinfo,fpkjjson,fpkjout,hqnsrjcxx);
			if(nRet != RET_SUCCESS){
				if(strstr(errinfo,"502 Bad Gateway") || strstr(errinfo,"��ʼ��dppt������ʧ��") || strstr(errinfo,"����Կ���߷���SM4��Կʧ��"))
				{
					continue;
				}
				else return nRet;
			}
			else return nRet;
		}
	}
	else return etax_blue_ink_invoice_issuance_Interface(busid,fpkjjson,related_type,mobile,idcard,sz_password,realationtype,uniqueIdentity,realationStatus,errinfo,fpkjout,hqnsrjcxx);
}

//�ɿ���Ʊ����Ʊ��ѯ
int function_common_etax_hzqrxxSzzhQuery_Interface(char *busid,char *fpcxsj,char *related_type,char *full_name,char *mobile,char *idcard,char *sz_password,int realationtype,char *uniqueIdentity,char *realationStatus,char *errinfo,char **fpcxdata,char *area_code,char *etax_cookie,char *tpass_cookie,char *dppt_cookie,char *new_etax_cookie,char *new_tpass_cookie,char *new_dppt_cookie,int use_cookie_flag)
{
	int i = 0 , nRet = -1;
	if(use_cookie_flag){
		for(i=0;i<5;i++){
			nRet = etax_hzqrxxSzzhQuery_by_cookies_Interface(area_code,etax_cookie,tpass_cookie,dppt_cookie,new_etax_cookie,new_tpass_cookie,new_dppt_cookie,fpcxsj,fpcxdata,errinfo);
			if(nRet != RET_SUCCESS){
				if(strstr(errinfo,"502 Bad Gateway") || strstr(errinfo,"��ʼ��dppt������ʧ��") || strstr(errinfo,"����Կ���߷���SM4��Կʧ��"))
				{
					continue;
				}
				else return nRet;
			}
			else return nRet;
		}
	}
	else return etax_hzqrxxSzzhQuery_Interface(busid,fpcxsj,related_type,full_name,mobile,idcard,sz_password,realationtype,uniqueIdentity,realationStatus,errinfo,fpcxdata);
}

//����ȷ�ϵ�����
int function_common_etax_hzqrxxSave_Interface(char *busid,char *fpcxsj,char *related_type,char *full_name,char *mobile,char *idcard,char *sz_password,int realationtype,char *uniqueIdentity,char *realationStatus,char *errinfo,char **fpcxdata,char *area_code,char *etax_cookie,char *tpass_cookie,char *dppt_cookie,char *new_etax_cookie,char *new_tpass_cookie,char *new_dppt_cookie,int use_cookie_flag)
{
	int i = 0 , nRet = -1;
	if(use_cookie_flag){
		for(i=0;i<5;i++){
			nRet = etax_hzqrxxSave_by_cookies_Interface(area_code,etax_cookie,tpass_cookie,dppt_cookie,new_etax_cookie,new_tpass_cookie,new_dppt_cookie,fpcxsj,fpcxdata,errinfo);
			if(nRet != RET_SUCCESS){
				if(strstr(errinfo,"502 Bad Gateway") || strstr(errinfo,"��ʼ��dppt������ʧ��") || strstr(errinfo,"����Կ���߷���SM4��Կʧ��"))
				{
					continue;
				}
				else return nRet;
			}
			else return nRet;
		}
	}
	else return etax_hzqrxxSave_Interface(busid,fpcxsj,related_type,full_name,mobile,idcard,sz_password,realationtype,uniqueIdentity,realationStatus,errinfo,fpcxdata);
}

//����ȷ�ϵ��б��ѯ
int function_common_etax_hzqrxxQuery_Interface(char *busid,char *fpcxsj,char *related_type,char *full_name,char *mobile,char *idcard,char *sz_password,int realationtype,char *uniqueIdentity,char *realationStatus,char *errinfo,char **fpcxdata,char *area_code,char *etax_cookie,char *tpass_cookie,char *dppt_cookie,char *new_etax_cookie,char *new_tpass_cookie,char *new_dppt_cookie,int use_cookie_flag)
{
	int i = 0 , nRet = -1;
	if(use_cookie_flag){
		for(i=0;i<5;i++){
			nRet = etax_hzqrxxQuery_by_cookies_Interface(area_code,etax_cookie,tpass_cookie,dppt_cookie,new_etax_cookie,new_tpass_cookie,new_dppt_cookie,fpcxsj,fpcxdata,errinfo);
			if(nRet != RET_SUCCESS){
				if(strstr(errinfo,"502 Bad Gateway") || strstr(errinfo,"��ʼ��dppt������ʧ��") || strstr(errinfo,"����Կ���߷���SM4��Կʧ��"))
				{
					continue;
				}
				else return nRet;
			}
			else return nRet;
		}
	}
	else return etax_hzqrxxQuery_Interface(busid,fpcxsj,related_type,full_name,mobile,idcard,sz_password,realationtype,uniqueIdentity,realationStatus,errinfo,fpcxdata);
}

//����ȷ�ϵ������ѯ
int function_common_etax_hzqrxxQueryDetail_Interface(char *busid,char *fpcxsj,char *related_type,char *full_name,char *mobile,char *idcard,char *sz_password,int realationtype,char *uniqueIdentity,char *realationStatus,char *errinfo,char **fpcxdata,char *area_code,char *etax_cookie,char *tpass_cookie,char *dppt_cookie,char *new_etax_cookie,char *new_tpass_cookie,char *new_dppt_cookie,int use_cookie_flag)
{
	int i = 0 , nRet = -1;
	if(use_cookie_flag){
		for(i=0;i<5;i++){
			nRet = etax_hzqrxxQueryDetail_by_cookies_Interface(area_code,etax_cookie,tpass_cookie,dppt_cookie,new_etax_cookie,new_tpass_cookie,new_dppt_cookie,fpcxsj,fpcxdata,errinfo);
			if(nRet != RET_SUCCESS){
				if(strstr(errinfo,"502 Bad Gateway") || strstr(errinfo,"��ʼ��dppt������ʧ��") || strstr(errinfo,"����Կ���߷���SM4��Կʧ��"))
				{
					continue;
				}
				else return nRet;
			}
			else return nRet;
		}
	}
	else return etax_hzqrxxQueryDetail_Interface(busid,fpcxsj,related_type,full_name,mobile,idcard,sz_password,realationtype,uniqueIdentity,realationStatus,errinfo,fpcxdata);
}

//������ѯ�ӿ�
int function_common_hcstaquery_invoice_issuance_Interface(char *busid,char *fpcxsj,char *related_type,char *mobile,char *idcard,char *sz_password,int realationtype,char *uniqueIdentity,char *realationStatus,char *errinfo,char **fpcxdata,char *area_code,char *etax_cookie,char *tpass_cookie,char *dppt_cookie,char *new_etax_cookie,char *new_tpass_cookie,char *new_dppt_cookie,int use_cookie_flag)
{
	int i = 0 , nRet = -1;
	if(use_cookie_flag){
		for(i=0;i<5;i++){
			nRet = etax_hcstaquery_invoice_issuance_by_cookie_Interface(area_code,etax_cookie,tpass_cookie,dppt_cookie,new_etax_cookie,new_tpass_cookie,new_dppt_cookie,errinfo,fpcxsj,fpcxdata);
			if(nRet != RET_SUCCESS){
				if(strstr(errinfo,"502 Bad Gateway") || strstr(errinfo,"��ʼ��dppt������ʧ��") || strstr(errinfo,"����Կ���߷���SM4��Կʧ��"))
				{
					continue;
				}
				else return nRet;
			}
			else return nRet;
		}
	}
	else return etax_hcstaquery_invoice_issuance_Interface(busid,fpcxsj,related_type,mobile,idcard,sz_password,realationtype,uniqueIdentity,realationStatus,errinfo,fpcxdata);
}

int function_common_fjxxpeizhi_Interface(char *busid,char *fpcxsj,char *mode,char *related_type,char *mobile,char *idcard,char *sz_password,int realationtype,char *uniqueIdentity,char *realationStatus,char *errinfo,char **fpcxdata,char *area_code,char *etax_cookie,char *tpass_cookie,char *dppt_cookie,char *new_etax_cookie,char *new_tpass_cookie,char *new_dppt_cookie,int use_cookie_flag)
{
	int i = 0 , nRet = -1;
	if(use_cookie_flag){
		for(i=0;i<5;i++){
			nRet = etax_fjxxpeizhi_by_cookie_Interface(area_code,etax_cookie,tpass_cookie,dppt_cookie,new_etax_cookie,new_tpass_cookie,new_dppt_cookie,errinfo,fpcxsj,mode,fpcxdata);
			if(nRet != RET_SUCCESS){
				if(strstr(errinfo,"502 Bad Gateway") || strstr(errinfo,"��ʼ��dppt������ʧ��") || strstr(errinfo,"����Կ���߷���SM4��Կʧ��"))
				{
					continue;
				}
				else return nRet;
			}
			else return nRet;
		}
	}
	else return etax_fjxxpeizhi_Interface(busid,fpcxsj,mode,related_type,mobile,idcard,sz_password,realationtype,uniqueIdentity,realationStatus,errinfo,fpcxdata);
}

int function_common_cjmbpeizhi_Interface(char *busid,char *fpcxsj,char *mode,char *related_type,char *mobile,char *idcard,char *sz_password,int realationtype,char *uniqueIdentity,char *realationStatus,char *errinfo,char **fpcxdata,char *area_code,char *etax_cookie,char *tpass_cookie,char *dppt_cookie,char *new_etax_cookie,char *new_tpass_cookie,char *new_dppt_cookie,int use_cookie_flag)
{
	int i = 0 , nRet = -1;
	if(use_cookie_flag){
		for(i=0;i<5;i++){
			nRet = etax_cjmbpeizhi_by_cookie_Interface(area_code,etax_cookie,tpass_cookie,dppt_cookie,new_etax_cookie,new_tpass_cookie,new_dppt_cookie,errinfo,fpcxsj,mode,fpcxdata);
			if(nRet != RET_SUCCESS){
				if(strstr(errinfo,"502 Bad Gateway") || strstr(errinfo,"��ʼ��dppt������ʧ��") || strstr(errinfo,"����Կ���߷���SM4��Կʧ��"))
				{
					continue;
				}
				else return nRet;
			}
			else return nRet;
		}
	}
	else return etax_cjmbpeizhi_Interface(busid,fpcxsj,mode,related_type,mobile,idcard,sz_password,realationtype,uniqueIdentity,realationStatus,errinfo,fpcxdata);
}

int function_common_etax_get_cookie_Interface(char *busid,char *related_type,char *mobile,char *idcard,char *sz_password,int realationtype,char *uniqueIdentity,char *realationStatus,char *errinfo,char **etax_cookie,char **tpass_cookie,char **dppt_cookie,char *svr_container,char *soft_cert_ip,int soft_cert_port,char *login_method,char *area_code,char *temp_data)
{
	int i = 0;
	int nRet = -1;
	for(i=0;i<5;i++)
	{
		nRet = etax_get_cookie_Interface(busid,related_type,mobile,idcard,sz_password,realationtype,uniqueIdentity,realationStatus,errinfo,etax_cookie,tpass_cookie,dppt_cookie,svr_container,"172.30.92.126",soft_cert_port,login_method,area_code,temp_data);
		if(nRet != RET_SUCCESS){
			if(strstr(errinfo,"502 Bad Gateway") || strstr(errinfo,"��ʼ��dppt������ʧ��") || strstr(errinfo,"����Կ���߷���SM4��Կʧ��"))
			{
				continue;
			}
			else return nRet;
		}
		else return nRet;
	} 
}

int function_common_etax_get_nsrjcxx_Interface(char *busid,char *related_type,char *mobile,char *idcard,char *sz_password,int realationtype,char *uniqueIdentity,char *realationStatus,char *errinfo,char **fpcxdata,char *area_code,char *etax_cookie,char *tpass_cookie,char *dppt_cookie,int use_cookie_flag)
{
	int i = 0 , nRet = -1;
	if(use_cookie_flag){
		for(i=0;i<5;i++){
			nRet = etax_get_nsrjcxx_by_cookie_Interface(area_code,etax_cookie,tpass_cookie,dppt_cookie,errinfo,fpcxdata);
			if(nRet != RET_SUCCESS){
				if(strstr(errinfo,"502 Bad Gateway") || strstr(errinfo,"��ʼ��dppt������ʧ��") || strstr(errinfo,"����Կ���߷���SM4��Կʧ��"))
				{
					continue;
				}
				else return nRet;
			}
			else return nRet;
		}
	}
}

int function_common_etax_get_nsrfxxx_Interface(char *busid,char *related_type,char *mobile,char *idcard,char *sz_password,int realationtype,char *uniqueIdentity,char *realationStatus,char *errinfo,char **fpcxdata,char *area_code,char *etax_cookie,char *tpass_cookie,char *dppt_cookie,int use_cookie_flag)
{
	int i = 0 , nRet = -1;
	if(use_cookie_flag){
		for(i=0;i<5;i++){
			nRet = etax_get_nsrfxxx_by_cookie_Interface(area_code,etax_cookie,tpass_cookie,dppt_cookie,errinfo,fpcxdata);
			if(nRet != RET_SUCCESS){
				if(strstr(errinfo,"502 Bad Gateway") || strstr(errinfo,"��ʼ��dppt������ʧ��") || strstr(errinfo,"����Կ���߷���SM4��Կʧ��"))
				{
					continue;
				}
				else return nRet;
			}
			else return nRet;
		}
	}
}

