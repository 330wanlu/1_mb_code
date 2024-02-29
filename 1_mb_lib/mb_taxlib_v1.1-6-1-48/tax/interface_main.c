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
		//report_event(NULL, "USB打开失败", "libusb通过busid和devid打开设备失败", nhUsbType);
		return -2;
	}

	if (DEVICE_AISINO == nhUsbType) {  // aisino init

		//nRet = LoadAisinoDevInfo(hDev, hUSB, false);
		hDev->bDeviceType = nhUsbType;
		CntaxEntry(hUSB, NOP_CLOSE);  //关闭再重新打开
		nRet = LoadAisinoDevInfo(hDev, hUSB, bAuthDev);
		if (nRet < 0) {
			//report_event(NULL, "金税盘加载基础信息失败", "金税盘加载基础信息失败,可能存在兼容问题", nRet);
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
		//这里其实不用关闭的，但是为了保证每次都是新的状态，还是关闭再重新打开比较好,有可能之前没开，再关就不成功，因此不做返回判断
		//_WriteLog(LL_WARN, "打开设备前先关卡\n");
		//NisecEntry(hUSB, NOP_CLOSE);
		//_WriteLog(LL_WARN, "打开设备前先关卡,关卡完成\n");
		nRet = LoadNisecDevInfo(hDev, hUSB,bAuthDev);
		if (nRet < 0) {
			//report_event(NULL, "税控盘加载基础信息失败", "税控盘加载基础信息失败,可能存在兼容问题", nRet);
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
		CntaxEntry(hUSB, NOP_CLOSE);  //关闭再重新打开
		nRet = LoadCntaxDevInfo(hDev, hUSB, bAuthDev);
		if (nRet < 0) {
			//report_event(NULL, "税务UKEY加载基础信息失败", "税务UKEY加载基础信息失败,可能存在兼容问题", nRet);
			_WriteLog(LL_WARN, "[+] LoadCntaxDevInfo Init device to open failed, nRet:%d\n", nRet);
			if (hUSB->handle)
				usb_device_close(hUSB);
			libusb_exit(hUSB->context);
			return ERR_DEVICE_PASSWORD_ERROR;
		}
		//_WriteLog(LL_DEBUG, "[+] Load cntax device successful!\n");
	}
	else if (DEVICE_SKP_KEY == nhUsbType)  {  // nisec init
		_WriteLog(LL_INFO, "[+] 测试设备 device found\n");
		//out("Nisec device found\n");
		//这里其实不用关闭的，但是为了保证每次都是新的状态，还是关闭再重新打开比较好,有可能之前没开，再关就不成功，因此不做返回判断
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
			out("设备打开成功,联机失败\n");
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
			out("设备打开成功,联机失败\n");
			usb_hid_close_serssion(hUSB, hDev);
			*h_USB = NULL;
			*h_Dev = NULL;
			return -4;
		}
	}
	else
	{
		out("设备打开失败,result = %d\n", nhUsbType);
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
	//writelog(LL_INFO,"taxlib","加载基础信息","读取税盘基础信息及监控信息");
	//_WriteLog(0,"测试日志输出\n");
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
			_WriteLog(LL_WARN, "此盘为金税盘型号税务ukey\n");
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
	//writelog(LL_INFO,"taxlib","加载基础信息","读取税盘基础信息及监控信息");
	//_WriteLog(0,"测试日志输出\n");
	if (usb_type == DEVICE_MENGBAI)
	{
		plate_info->plate_basic_info.dev_passwd_right = 1;
		result = usb_hid_open_serssion(&h_plate->hUSB, &h_plate->hDev, nBusID, nDevID, h_plate->busid, last_error);
		if (result < 0)
		{
			out("设备打开失败,result = %d\n", result);
			return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
		}
		//out("加载设备成功\n");
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
		//out("获取基本信息成功\n");
		//out("[+] Open device successful!\n");
		result = mengbai_read_inv_type_areacode(h_plate->hUSB, plate_info);
		if (result < 0)
		{
			printf("[-] Read area code failed\n");
			usb_hid_close_serssion(h_plate->hUSB, h_plate->hDev);
			return result;
		}
		//out("获取区域代码成功\n");
		result = mengbai_read_monitor_info(h_plate->hUSB, plate_info->plate_tax_info.inv_type, plate_info);
		if (result < 0)
		{
			printf("[-] Read state info failed\n");
			usb_hid_close_serssion(h_plate->hUSB, h_plate->hDev);
			return result;
		}
		//out("获取监控信息成功\n");
		result = mengbai_read_inv_coil_number(h_plate->hUSB, plate_info->plate_tax_info.inv_type, plate_info);
		if (result < 0)
		{
			printf("[-] Read coil number failed\n");
			usb_hid_close_serssion(h_plate->hUSB, h_plate->hDev);
			return result;
		}
		//out("获取发票库存信息成功\n");
		result = mengbai_read_offline_inv_num(h_plate->hUSB, plate_info->plate_tax_info.inv_type, plate_info);
		if (result < 0)
		{
			printf("[-] Read offline_inv_num failed\n");
			usb_hid_close_serssion(h_plate->hUSB, h_plate->hDev);
			return result;
		}
		plate_info->plate_basic_info.use_downgrade_version = 0;
		strcpy(plate_info->plate_basic_info.minimum_version, "V0.0.00.000000");
		//out("获取离线发票数量成功\n");
		//out("【模拟盘】读取到离线票数量为%d\n",result);
		plate_info->plate_tax_info.off_inv_num = result;
		plate_info->plate_basic_info.plate_test = 1;
		h_plate->need_free = 1;
		//out("结束读取监控信息\n");
		return 0;
	}
	else if (usb_type == DEVICE_MENGBAI2)
	{
		plate_info->plate_basic_info.dev_passwd_right = 1;
		result = usb_hid_open_serssion(&h_plate->hUSB, &h_plate->hDev, nBusID, nDevID, h_plate->busid, last_error);
		if (result < 0)
		{
			out("设备打开失败,result = %d\n", result);
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
		//out("结束读取监控信息\n");
		return 0;
	}
	else if (usb_type == DEVICE_NISEC)
	{
		plate_info->plate_basic_info.dev_passwd_right = 1;
		nDeviceType = usb_device_open_serssion(&h_plate->hUSB, &h_plate->hDev, true, nBusID, nDevID, h_plate->busid,last_error);
		//_WriteLog(LL_WARN, "USB设备打开成功\n");
		//out("usb_device_open_serssion success 总线号：%d，设备号：%d\n", nBusID, nDevID);
		if (nDeviceType < 0)
		{
			_WriteLog(LL_WARN, "税控盘设备初始化打开失败,最后错误代码为%s\n", last_error);
			if (strstr(last_error, "09d1c") != NULL)
			{
				char devpass_leftcount[10] = { 0 };
				char errinfo[1024] = { 0 };

				char *tmp = strstr(last_error, "09d1c");
				tmp += strlen("09d1c");

				memcpy(devpass_leftcount, tmp, 1);
				sprintf(errinfo, "非默认设备密码88888888,剩余尝试次数%s", devpass_leftcount);
				logout(INFO, "TAXLIB", "税盘读取", "上报税盘：税控盘,错误信息：%s\r\n", errinfo);
				//report_event(NULL, "税控盘设备密码有误", errinfo, nDeviceType);
				plate_info->plate_basic_info.plate_type = DEVICE_NISEC;
				plate_info->plate_basic_info.dev_passwd_right = 0;
				plate_info->plate_basic_info.dev_left_num = atoi(devpass_leftcount);
				plate_info->plate_basic_info.dev_err_reason = ERR_DEVICE_PASSWORD_ERROR;
			}
			else if (strstr(last_error, "09d10b") != NULL)
			{
				char errinfo[1024] = { 0 };
				sprintf(errinfo, "设备密码pin锁定(09d10b),剩余尝试次数0");
				logout(INFO, "TAXLIB", "税盘读取", "上报税盘：税控盘,错误信息：%s\r\n", errinfo);
				//report_event(NULL, "税控盘设备密码有误", errinfo, nDeviceType);
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
		//_WriteLog(LL_WARN, "基础信息读取成功\n");
		
		result = nisec_get_state_info(h_plate->hDev, plate_info);
		if (result < 0)
		{
			printf("[-] nisec_get_state_info failed result = %d\n", result);
		}
		//_WriteLog(LL_WARN, "监控信息读取完成\n");
		result = nisec_read_inv_coil_number(h_plate->hDev, plate_info);
		if (result < 0)
		{
			printf("[-] nisec_read_inv_coil_number failed result = %d\n", result);
		}
		//_WriteLog(LL_WARN, "发票卷信息读取完成\n");
#ifndef DF_OLD_MSERVER
		nisec_update_summary_data(h_plate->hDev, plate_info);
#endif
	/*	result = nisec_get_state_info_and_coil_info(h_plate->hDev,h_plate->hUSB, plate_info);
		_WriteLog(LL_WARN, "发票卷信息读取完成\n");*/
		TranslateStandTime(2, h_plate->hDev->szDeviceTime, (char *)plate_info->plate_tax_info.plate_time);
		//_WriteLog(LL_WARN, "获取到的时间：%s，load获取的时间：%s\n", plate_info->plate_tax_info.plate_time, h_plate->hDev->szDeviceTime);
		strcpy((char *)plate_info->plate_tax_info.driver_ver,h_plate->hDev->szDriverVersion);
		int off_num;
		off_num = nisec_is_exsist_offline_inv(h_plate->hDev, (char *)plate_info->plate_tax_info.off_inv_fpdm, (char *)plate_info->plate_tax_info.off_inv_fphm, (char *)plate_info->plate_tax_info.off_inv_kpsj, &plate_info->plate_tax_info.off_inv_dzsyh);
		if (off_num <= 0)
		{
			//out("监控信息获取盘中不存在离线发票\n");
			plate_info->plate_tax_info.off_inv_num = 0;
		}
		else
		{
			plate_info->plate_tax_info.off_inv_num = 1;
		}
		//_WriteLog(LL_WARN, "读取离线发票存在完成\n");
		memcpy(plate_info->plate_tax_info.startup_date, h_plate->hDev->szDeviceEffectDate, 8);
		memcpy(plate_info->plate_tax_info.tax_office_name, h_plate->hDev->szTaxAuthorityName, strlen(h_plate->hDev->szTaxAuthorityName));
		memcpy(plate_info->plate_tax_info.tax_office_code, h_plate->hDev->szTaxAuthorityCode, 11);//标准为11位，税控盘特殊从第二位开始

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
			sprintf((char *)plate_info->plate_tax_info.tax_nature, "代开税务机关");
			break;
		case 1:
			sprintf((char *)plate_info->plate_tax_info.tax_nature, "小规模纳税人");
			break;
		case 2:
			sprintf((char *)plate_info->plate_tax_info.tax_nature, "一般纳税人");
			break;
		case 3:
			sprintf((char *)plate_info->plate_tax_info.tax_nature, "转登记纳税人");
			break;
		default:
			//sprintf((char *)plate_infos->plate_tax_info.tax_nature, "");
			break;
		}
		plate_info->plate_basic_info.use_downgrade_version = !CheckDeviceVersionMatch(h_plate->hDev, plate_info->plate_basic_info.minimum_version);
		if (h_plate->hDev->bTrainFPDiskType != 0)
			plate_info->plate_basic_info.plate_test = 1;
		h_plate->need_free = 1;
		//out("结束获取监控信息\n");
		return 0;
	}
	else if (usb_type == DEVICE_AISINO)
	{
		plate_info->plate_basic_info.dev_passwd_right = 1;
		nDeviceType = usb_device_open_serssion(&h_plate->hUSB, &h_plate->hDev, true, nBusID, nDevID, h_plate->busid, last_error);
		if (nDeviceType < 0)
		{
			
			_WriteLog(LL_WARN, "金税盘设备初始化打开失败,最后错误代码为%s\n", last_error);
			if (strstr(last_error, "09d1c") != NULL)
			{
				char devpass_leftcount[10] = { 0 };
				char errinfo[1024] = { 0 };

				char *tmp = strstr(last_error, "09d1c");
				tmp += strlen("09d1c");

				memcpy(devpass_leftcount, tmp, 1);
				sprintf(errinfo, "非默认设备密码12345678,剩余尝试次数%s", devpass_leftcount);
				logout(INFO, "TAXLIB", "税盘读取", "上报税盘：税务ukey,错误信息：%s\r\n", errinfo);
				//report_event(NULL, "税务UKey设备密码有误", errinfo, nDeviceType);
				plate_info->plate_basic_info.plate_type = DEVICE_CNTAX;
				plate_info->plate_basic_info.dev_passwd_right = 0;
				plate_info->plate_basic_info.dev_left_num = atoi(devpass_leftcount);
				plate_info->plate_basic_info.dev_err_reason = ERR_DEVICE_PASSWORD_ERROR;
			}
			else if (strstr(last_error, "09d10b") != NULL)
			{
				char errinfo[1024] = { 0 };
				sprintf(errinfo, "设备密码pin锁定(09d10b),剩余尝试次数0");
				logout(INFO, "TAXLIB", "税盘读取", "上报税盘：税务ukey,错误信息：%s\r\n", errinfo);
				//report_event(NULL, "税务UKey设备密码有误", errinfo, nDeviceType);
				plate_info->plate_basic_info.plate_type = DEVICE_CNTAX;
				plate_info->plate_basic_info.dev_passwd_right = 0;
				plate_info->plate_basic_info.dev_left_num = 0;
				plate_info->plate_basic_info.dev_err_reason = ERR_DEVICE_PASSWORD_LOCK;
			}
			return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
		
		}
		if (nDeviceType == DEVICE_AISINO)
		{
			////这里其实不用关闭的，但是为了保证每次都是新的状态，还是关闭再重新打开比较好
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
			//_WriteLog(LL_INFO, "获取是否存在未解锁发票\n");
			exit_unlock_inv = ReadUnlockInvoiceFromDevice(h_plate->hDev, true);
			if (exit_unlock_inv)
			{
				_WriteLog(LL_INFO, "税盘存在未解锁发票,需要进行发票读入\n");
				//report_event(h_plate->hDev->szDeviceID, "未解锁发票读入", "税盘存在未解锁发票,需要进行发票读入", 0);
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
			//out("获取到的时间：%s，load获取的时间：%s\n", plate_infos->plate_tax_info.plate_time, h_Dev->szDeviceTime);

			//out("读取税盘时间信息完成\n");
			int off_num;
			off_num = aisino_is_exsist_offline_inv(h_plate->hDev, (char *)plate_info->plate_tax_info.off_inv_fpdm, (char *)plate_info->plate_tax_info.off_inv_fphm, (char *)plate_info->plate_tax_info.off_inv_kpsj, &plate_info->plate_tax_info.off_inv_dzsyh);
			if ((off_num < 0) || (off_num == 100))
			{
				//out("监控信息获取盘中不存在离线发票\n");
				plate_info->plate_tax_info.off_inv_num = 0;
			}
			else
			{
				plate_info->plate_tax_info.off_inv_num = 1;
			}
			//out("读取离线发票存在完成\n");
			memcpy(plate_info->plate_tax_info.startup_date, h_plate->hDev->szDeviceEffectDate, 8);
			memcpy(plate_info->plate_tax_info.tax_office_name, h_plate->hDev->szTaxAuthorityName, strlen(h_plate->hDev->szTaxAuthorityName));
			memcpy(plate_info->plate_tax_info.tax_office_code, h_plate->hDev->szTaxAuthorityCode, 11);//金税盘特殊处理后面多\r\n
			memcpy(plate_info->plate_tax_info.area_code, h_plate->hDev->szRegCode, strlen(h_plate->hDev->szRegCode));
			//_WriteLog(LL_INFO, "区域代码%s", h_plate->hDev->szRegCode);
			if (memcmp(plate_info->plate_tax_info.area_code, "91", 2) == 0)
			{
				_WriteLog(LL_INFO,"重庆地区区域代码有问题需要修改\n");
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
				sprintf((char *)plate_info->plate_tax_info.tax_nature, "代开税务机关");
				break;
			case 1:
				sprintf((char *)plate_info->plate_tax_info.tax_nature, "小规模纳税人");
				break;
			case 2:
				sprintf((char *)plate_info->plate_tax_info.tax_nature, "一般纳税人");
				break;
			case 3:
				sprintf((char *)plate_info->plate_tax_info.tax_nature, "转登记纳税人");
				break;
			default:
				//sprintf((char *)plate_infos->plate_tax_info.tax_nature, "");
				break;
			}

			plate_info->plate_basic_info.use_downgrade_version = !CheckDeviceVersionMatch(h_plate->hDev, plate_info->plate_basic_info.minimum_version);

			if ((result = CheckDriverVerSupport(h_plate->hDev, h_plate->hDev->szDriverVersion)) < 0) {
				_WriteLog(LL_FATAL, "CheckDriverVerSupport failed, driverVer:%s", h_plate->hDev->szDriverVersion);

				if(result == ERR_DEVICE_FIRMWARE_LOWVERSION)
					sprintf((char *)plate_info->plate_tax_info.driver_ver_err, "底层驱动版本太低");
				else if (result == ERR_DEVICE_FIRMWARE_HIGHVERSION)
					sprintf((char *)plate_info->plate_tax_info.driver_ver_err, "底层驱动版本过高");
			}
			strcpy((char *)plate_info->plate_tax_info.driver_ver, h_plate->hDev->szDriverVersion);

			//out("结束读取监控信息\n");
			//plate_info->plate_tax_info.lxsc = h_Dev->nMaxOfflineHour;
			if (h_plate->hDev->bTrainFPDiskType != 0)
				plate_info->plate_basic_info.plate_test = 1;
			

		}
		else if (nDeviceType == DEVICE_CNTAX)
		{
			out("获取aisino税务UKEY相关信息");
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
				//out("监控信息获取盘中不存在离线发票\n");
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
				sprintf((char *)plate_info->plate_tax_info.tax_nature, "代开税务机关");
				break;
			case 1:
				sprintf((char *)plate_info->plate_tax_info.tax_nature, "小规模纳税人");
				break;
			case 2:
				sprintf((char *)plate_info->plate_tax_info.tax_nature, "一般纳税人");
				break;
			case 3:
				sprintf((char *)plate_info->plate_tax_info.tax_nature, "转登记纳税人");
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
			//memcpy(plate_info->plate_basic_info.ca_number, h_Dev->szCompressTaxID, strlen(h_Dev->szCompressTaxID));//不支持税号读取,底层无税号信息
			memcpy(plate_info->plate_basic_info.ca_name, h_plate->hDev->szCompanyName, strlen(h_plate->hDev->szCompanyName));
			memcpy(plate_info->plate_basic_info.plate_num, h_plate->hDev->szDeviceID, strlen(h_plate->hDev->szDeviceID));
			plate_info->plate_basic_info.result = 1;
			plate_info->plate_basic_info.plate_type = DEVICE_JSP_KEY;
			plate_info->plate_basic_info.extension = h_plate->hDev->uICCardNo;
		}
		//_WriteLog(LL_INFO, "获取aisino相关信息结束,DEV [%08X]  USB [%08X]\n", h_plate->hDev, h_plate->hUSB);
		h_plate->need_free = 1;
		return 0;

	}
	else if (usb_type == DEVICE_CNTAX)
	{
		
		plate_info->plate_basic_info.dev_passwd_right = 1;
		nDeviceType = usb_device_open_serssion(&h_plate->hUSB, &h_plate->hDev, true, nBusID, nDevID, h_plate->busid, last_error);
		if (nDeviceType < 0)
		{

			_WriteLog(LL_WARN, "税务UKEY设备初始化打开失败,最后错误代码为%s\n", last_error);
			if (strstr(last_error, "09d1c") != NULL)
			{
				char devpass_leftcount[10] = { 0 };
				char errinfo[1024] = { 0 };

				char *tmp = strstr(last_error, "09d1c");
				tmp += strlen("09d1c");

				memcpy(devpass_leftcount, tmp, 1);
				sprintf(errinfo, "非默认设备密码12345678,剩余尝试次数%s", devpass_leftcount);
				logout(INFO, "TAXLIB", "税盘读取", "上报税盘：税务ukey,错误信息：%s\r\n", errinfo);
				//report_event(NULL, "税务UKey设备密码有误", errinfo, nDeviceType);
				plate_info->plate_basic_info.plate_type = DEVICE_CNTAX;
				plate_info->plate_basic_info.dev_passwd_right = 0;
				plate_info->plate_basic_info.dev_left_num = atoi(devpass_leftcount);
				plate_info->plate_basic_info.dev_err_reason = ERR_DEVICE_PASSWORD_ERROR;
			}
			else if (strstr(last_error, "09d10b") != NULL)
			{
				char errinfo[1024] = { 0 };
				sprintf(errinfo, "设备密码pin锁定(09d10b),剩余尝试次数0");
				logout(INFO, "TAXLIB", "税盘读取", "上报税盘：税务ukey,错误信息：%s\r\n", errinfo);
				//report_event(NULL, "税务UKey设备密码有误", errinfo, nDeviceType);
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
			//out("监控信息获取盘中不存在离线发票\n");
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
			sprintf((char *)plate_info->plate_tax_info.tax_nature, "代开税务机关");
			break;
		case 1:
			sprintf((char *)plate_info->plate_tax_info.tax_nature, "小规模纳税人");
			break;
		case 2:
			sprintf((char *)plate_info->plate_tax_info.tax_nature, "一般纳税人");
			break;
		case 3:
			sprintf((char *)plate_info->plate_tax_info.tax_nature, "转登记纳税人");
			break;
		default:
			//sprintf((char *)plate_infos->plate_tax_info.tax_nature, "");
			break;
		}
		plate_info->plate_basic_info.use_downgrade_version = !CheckDeviceVersionMatch(h_plate->hDev, plate_info->plate_basic_info.minimum_version);
		h_plate->need_free = 1;

		if (h_plate->hDev->bTrainFPDiskType != 0)
			plate_info->plate_basic_info.plate_test = 1;
		//out("swukey_get_basic_info 读取完成\n");
		//out("swukey_get_basic_info 读取完成关闭USB句柄完成\n");
		return 0;
	}
	else if (usb_type == DEVICE_SKP_KEY)
	{

		plate_info->plate_basic_info.dev_passwd_right = 1;
		nDeviceType = usb_device_open_serssion(&h_plate->hUSB, &h_plate->hDev, false, nBusID, nDevID, h_plate->busid, last_error);
		out("usb_device_open_serssion success 总线号：%d，设备号：%d\n", nBusID, nDevID);
		if (nDeviceType < 0)
		{
			return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
		}
		else if (nDeviceType == DEVICE_SKP_KEY)
		{
			memcpy(plate_info->plate_basic_info.ca_number, h_plate->hDev->szCommonTaxID, strlen(h_plate->hDev->szCommonTaxID));//不支持税号读取,底层无税号信息
			memcpy(plate_info->plate_basic_info.ca_name, h_plate->hDev->szCompanyName, strlen(h_plate->hDev->szCompanyName));
			memcpy(plate_info->plate_basic_info.plate_num, h_plate->hDev->szDeviceID, strlen(h_plate->hDev->szDeviceID));
			plate_info->plate_basic_info.result = 3;
			plate_info->plate_basic_info.plate_type = DEVICE_SKP_KEY;
			plate_info->plate_basic_info.extension = 0;
		}
		out("名称：%s\n", plate_info->plate_basic_info.ca_name);
		h_plate->need_free = 1;
		return 0;
	}
	else
	{
		logout(INFO, "TAXLIB", "税盘读取", "未知的设备类型，无法支持读取\n");
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

//==不需要授权且不需要口令验证通过亦可调用的接口==//
//获取税盘当前时间
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


//mqtt接口获取发票明细
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

//发票上传M服务
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
//获取月发票总数
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
//按发票代码号码获取发票信息
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
//按发票代码号码获取发票二进制文件信息
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

//==需要授权未验证口令或口令错误可调用的接口==//
//程序主动验证证书口令
int function_aisino_get_auth_passwd(struct HandlePlate *h_plate, struct _plate_infos *plate_info)
{
	int result = -1;
	int left_num = -1;

	//判断端口是否授权
	result = jude_busnum_devnum_handle(h_plate);
	if (result < 0)
	{
		plate_info->plate_basic_info.cert_passwd_right = 0;
		plate_info->plate_basic_info.cert_err_reason = DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
		return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
	}

	//验证证书口令是否为12345678
	//char passwd[100] = { 0 };
	result = aisino_verify_cert_passwd(h_plate->hDev, &left_num);
	if (result > 0)
	{
		plate_info->plate_basic_info.cert_passwd_right = 1;
		//out("税控盘默认证书口令12345678认证成功\n");
	}
	else
	{
		plate_info->plate_basic_info.cert_passwd_right = 0;
		plate_info->plate_basic_info.cert_left_num = left_num;
		plate_info->plate_basic_info.cert_err_reason = result;
		out("金税盘默认证书口令12345678认证失败\n");

	}
	return result;
}
int function_nisec_get_auth_passwd(struct HandlePlate *h_plate, struct _plate_infos *plate_info)
{
	int result = -1;
	int left_num = -1;

	//判断端口是否授权
	result = jude_busnum_devnum_handle(h_plate);
	if (result < 0)
	{
		plate_info->plate_basic_info.cert_passwd_right = 0;
		plate_info->plate_basic_info.cert_err_reason = DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
		return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
	}
			
	//验证证书口令是否为12345678
	//char passwd[100] = { 0 };
	result = nisec_verify_cert_passwd(h_plate->hDev, &left_num);
	if (result > 0)
	{
		plate_info->plate_basic_info.cert_passwd_right = 1;
		//out("税控盘默认证书口令12345678认证成功\n");
	}
	else
	{
		plate_info->plate_basic_info.cert_passwd_right = 0;
		plate_info->plate_basic_info.cert_left_num = left_num;
		plate_info->plate_basic_info.cert_err_reason = result;
		out("税控盘默认证书口令12345678认证失败\n");

	}
	return result;
}
int function_cntax_get_auth_passwd(struct HandlePlate *h_plate, struct _plate_infos *plate_info)
{
	int result = -1;
	int left_num = -1;

	//判断端口是否授权
	result = jude_busnum_devnum_handle(h_plate);
	if (result < 0)
	{
		plate_info->plate_basic_info.cert_passwd_right = 0;
		plate_info->plate_basic_info.cert_err_reason = DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
		return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
	}

	//验证证书口令是否为12345678
	//char passwd[100] = { 0 };
	result = cntax_verify_cert_passwd(h_plate->hDev, &left_num);
	if (result > 0)
	{
		plate_info->plate_basic_info.cert_passwd_right = 1;
		//out("税控盘默认证书口令12345678认证成功\n");
	}
	else
	{
		plate_info->plate_basic_info.cert_passwd_right = 0;
		plate_info->plate_basic_info.cert_left_num = left_num;
		plate_info->plate_basic_info.cert_err_reason = result;
		out("税务ukey默认证书口令12345678认证失败\n");

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
//接口调用验证默认证书口令是否正确
int function_aisino_verify_cert_passwd(struct HandlePlate *h_plate, char *passwd)
{
	int result = -1;
	int left_num = -1;
	result = jude_busnum_devnum_handle(h_plate);
	if (result < 0)
	{
		return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
	}
	//out("总线号：%d，设备号：%d\n", nBusID, nDevID);
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
	//out("总线号：%d，设备号：%d\n", nBusID, nDevID);
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
//修改证书口令
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
//获取当前发票代码号码
int function_aisino_get_current_invcode(struct HandlePlate *h_plate, unsigned char type, char* invnum, char* invcode)
{
	int result = -1;
	result = jude_busnum_devnum_handle(h_plate);
	if (result < 0)
	{
		return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
	}
	//out("总线号：%d，设备号：%d\n", nBusID, nDevID);
	
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
	//out("总线号：%d，设备号：%d\n", nBusID, nDevID);
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


//======需要授权且验证口令正确后方可操作的接口======//
//抄税汇总
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
//汇总清卡
int function_aisino_report_summary_write_back(struct HandlePlate *h_plate, uint8 inv_type, char *errinfo)
{
	int result = -1;
	result = jude_busnum_devnum_handle(h_plate);
	if (result < 0)
	{
		return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
	}
	//out("总线号：%d，设备号：%d\n", nBusID, nDevID);
	
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
	//out("总线号：%d，设备号：%d\n", nBusID, nDevID);
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
	//out("总线号：%d，设备号：%d\n", nBusID, nDevID);
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
//离线发票上传
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
			_WriteLog(LL_INFO, "重试上传一次\n");
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
			_WriteLog(LL_INFO, "重试上传一次\n");
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
//按发票代码号码上传离线发票
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
//测试税盘与服务器连接
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
//正数、负数发票开具
int function_aisino_make_invoice(struct HandlePlate *h_plate, unsigned char inv_type, char* inv_data, struct _plate_infos *plate_infos, struct _make_invoice_result *make_invoice_result)
{
	int result = -1;
	result = jude_busnum_devnum_handle(h_plate);
	if (result < 0)
	{
		sprintf((char *)make_invoice_result->errinfo, "USB操作失败,Bus信息有误,请重试");
		return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
	}
	result = aisino_make_invoice(h_plate->hDev, inv_data, plate_infos, make_invoice_result);
	if (result < 0)
	{
		printf("[-] function_aisino_make_invoice failed result = %d\n", result);
	}
	else
	{		
		//out("开始读取监控信息\n");
		aisino_read_monitor_info(h_plate->hUSB, h_plate->hDev, plate_infos);
		aisino_read_inv_coil_number(h_plate->hUSB, plate_infos);
		//out("结束读取监控信息\n");
	}
	if (make_invoice_result->need_restart == 1)
	{
		out("税盘需要重启,将bus 信息至为异常状态\n");
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
		sprintf((char *)make_invoice_result->errinfo, "USB操作失败,Bus信息有误,请重试");
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
		out("税盘需要重启,将bus 信息至为异常状态\n");
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
		sprintf((char *)make_invoice_result->errinfo, "USB操作失败,Bus信息有误,请重试");
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
		out("税盘需要重启,将bus 信息至为异常状态\n");
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
		sprintf((char *)make_invoice_result->errinfo, "USB操作失败,Bus信息有误,请重试");
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
		//out("获取区域代码成功\n");
		result = mengbai_read_monitor_info(h_plate->hUSB, plate_infos->plate_tax_info.inv_type, plate_infos);
		//out("获取监控信息成功\n");
		result = mengbai_read_inv_coil_number(h_plate->hUSB, plate_infos->plate_tax_info.inv_type, plate_infos);
		//out("获取发票库存信息成功\n");
		result = mengbai_read_offline_inv_num(h_plate->hUSB, plate_infos->plate_tax_info.inv_type, plate_infos);
		plate_infos->plate_tax_info.off_inv_num = result;
		plate_infos->plate_basic_info.plate_test = 1;
		result = 0;
	}
	if (make_invoice_result->need_restart == 1)
	{
		out("税盘需要重启,将bus 信息至为异常状态\n");
		h_plate->nBusID = -1;
		h_plate->nDevID = -1;
	}
	return result;
}
//已开发票作废
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
		out("税盘需要重启,将bus 信息至为异常状态\n");
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
		out("税盘需要重启,将bus 信息至为异常状态\n");
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
		out("税盘需要重启,将bus 信息至为异常状态\n");
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
		out("税盘需要重启,将bus 信息至为异常状态\n");
		h_plate->nBusID = -1;
		h_plate->nDevID = -1;
	}
	return result;
}
//未开发票作废
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
		//out("开始读取监控信息\n");
		aisino_read_monitor_info(h_plate->hUSB, h_plate->hDev, plate_infos);
		aisino_read_inv_coil_number(h_plate->hUSB, plate_infos);
		//out("结束读取监控信息\n");
	}
	if (cancel_invoice_result->need_restart == 1)
	{
		out("税盘需要重启,将bus 信息至为异常状态\n");
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
		out("税盘需要重启,将bus 信息至为异常状态\n");
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
		out("税盘需要重启,将bus 信息至为异常状态\n");
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
		//out("获取区域代码成功\n");
		result = mengbai_read_monitor_info(h_plate->hUSB, plate_infos->plate_tax_info.inv_type, plate_infos);
		//out("获取监控信息成功\n");
		result = mengbai_read_inv_coil_number(h_plate->hUSB, plate_infos->plate_tax_info.inv_type, plate_infos);
		//out("获取发票库存信息成功\n");
		result = mengbai_read_offline_inv_num(h_plate->hUSB, plate_infos->plate_tax_info.inv_type, plate_infos);
		plate_infos->plate_tax_info.off_inv_num = result;
		plate_infos->plate_basic_info.plate_test = 1;
		result = 0;
	}
	if (cancel_invoice_result->need_restart == 1)
	{
		out("税盘需要重启,将bus 信息至为异常状态\n");
		h_plate->nBusID = -1;
		h_plate->nDevID = -1;
	}
	return result;
}

//检查蓝字发票代码号码是否可以开具红票
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


//红字信息表申请
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
//通过红字信息表编号查询申请信息
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
//通过时间段查询红字信息表申请信息
int function_aisino_download_hzxxb_from_date_range(struct HandlePlate *h_plate, char *date_range, uint8 **tzdbh_data, char *errinfo)
{
	int result = -1;
	result = jude_busnum_devnum_handle(h_plate);
	if (result < 0)
	{
		return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
	}
	//out("总线号：%d，设备号：%d\n", nBusID, nDevID);
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
	//out("总线号：%d，设备号：%d\n", nBusID, nDevID);
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
	//out("总线号：%d，设备号：%d\n", nBusID, nDevID);
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
	//out("总线号：%d，设备号：%d\n", nBusID, nDevID);
	result = cntax_download_hzxxb_from_date_range(h_plate->hDev, date_range, tzdbh_data, errinfo);
	if (result < 0)
	{
		printf("[-] cntax_download_hzxxb_from_date_range failed result = %d\n", result);
	}
	return result;
}
//撤销红字信息表申请
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


//在线发票领用查询
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

//在线发票领用
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

//申请接入公共服务平台
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
//撤销接入公共服务平台
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
//服务器连接握手--hello
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

//服务器连接认证--auth
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

//获取税盘证书数据
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

//按月获取税盘汇总信息接口
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

//读历史票卷信息
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

//获取蒙柏服务商key加密信息
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

//一键登录
int function_common_etax_login_Interface(char *busid,char *related_type,char *full_name,char *mobile,char *idcard,char *sz_password,char *errinfo,char *lpgoto,char *lpcode,char *lptoken,char *sys_type,char *svr_container,char *soft_cert_ip,int soft_cert_port,char *login_method,char *area_code)
{
	int i = 0 , nRet = -1;
	for(i=0;i<5;i++){
		nRet = etax_login_Interface(busid,related_type,full_name,mobile,idcard,sz_password,errinfo,lpgoto,lpcode,lptoken,sys_type,svr_container,"172.30.92.126",soft_cert_port,login_method,area_code);
		if(nRet != RET_SUCCESS){
			if(strstr(errinfo,"请求公钥或者发送SM4密钥失败"))
			{
				continue;
			}
			else return nRet;
		}
		else return nRet;
	}

	return nRet;
}

//用户查询
int function_common_etax_user_query_Interface(char *busid,char *errinfo,char **outinfo,char *svr_container,char *soft_cert_ip,int soft_cert_port,char *login_method,char *area_code)
{
	int i = 0 , nRet = -1;
	for(i=0;i<5;i++){	
		nRet =  etax_user_query_Interface(busid,errinfo,outinfo,svr_container,"172.30.92.126",soft_cert_port,login_method,area_code);
		if(nRet != RET_SUCCESS){
			if(strstr(errinfo,"请求公钥或者发送SM4密钥失败"))
			{
				continue;
			}
			else return nRet;
		}
		else return nRet;		
	}
	return nRet;
}

//企业列表查询
int function_common_etax_relationlist_Interface(char *busid,char *related_type,char *full_name,char *mobile,char *idcard,char *sz_password,char *errinfo,char **outenterpriselist,char *svr_container,char *soft_cert_ip,int soft_cert_port,char *login_method,char *area_code)
{
	int i = 0 , nRet = -1;
	for(i=0;i<5;i++){
		nRet = etax_relationlist_Interface(busid,related_type,full_name,mobile,idcard,sz_password,errinfo,outenterpriselist,svr_container,"172.30.92.126",soft_cert_port,login_method,area_code);
		if(nRet != RET_SUCCESS){
			if(strstr(errinfo,"请求公钥或者发送SM4密钥失败"))
			{
				continue;
			}
			else return nRet;
		}
		else return nRet;
	}
	return nRet;
}

//企业切换
int function_common_etax_relationchange_Interface(char *busid,char *related_type,char *full_name,char *mobile,char *idcard,char *sz_password,char *errinfo,char *uniqueIdentity,char *realationStatus,char *lpgoto,char *lpcode,char *lptoken,char *sys_type,char *svr_container,char *soft_cert_ip,int soft_cert_port,char *login_method,char *area_code)
{
	int i = 0 , nRet = -1;
	for(i=0;i<5;i++){
		nRet = etax_relationchange_Interface(busid,related_type,full_name,mobile,idcard,sz_password,errinfo,uniqueIdentity,realationStatus,lpgoto,lpcode,lptoken,sys_type,svr_container,"172.30.92.126",soft_cert_port,login_method,area_code);
		if(nRet != RET_SUCCESS){
			if(strstr(errinfo,"请求公钥或者发送SM4密钥失败"))
			{
				continue;
			}
			else return nRet;
		}
		else return nRet;
	}
	return nRet;
}

//发票查询列表
int function_common_etax_invoice_query_Interface(char *busid,char *fpcxsj,char *related_type,char *full_name,char *mobile,char *idcard,char *sz_password,int realationtype,char *uniqueIdentity,char *realationStatus,char *errinfo,char **fpcxdata,char *area_code,char *etax_cookie,char *tpass_cookie,char *dppt_cookie,char *new_etax_cookie,char *new_tpass_cookie,char *new_dppt_cookie,int use_cookie_flag)
{
	int i = 0 , nRet = -1;
	if(use_cookie_flag){
		for(i=0;i<5;i++){
			nRet = etax_invoice_query_by_cookie_Interface(area_code,etax_cookie,tpass_cookie,dppt_cookie,new_etax_cookie,new_tpass_cookie,new_dppt_cookie,fpcxsj,fpcxdata,errinfo);
			if(nRet != RET_SUCCESS){
				if(strstr(errinfo,"502 Bad Gateway") || strstr(errinfo,"初始化dppt服务器失败") || strstr(errinfo,"请求公钥或者发送SM4密钥失败"))
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

//发票查询详情
int function_common_etax_invoice_query_detail_Interface(char *busid,char *fpcxsj,char *related_type,char *full_name,char *mobile,char *idcard,char *sz_password,int realationtype,char *uniqueIdentity,char *realationStatus,char *errinfo,char **fpcxdata,char *area_code,char *etax_cookie,char *tpass_cookie,char *dppt_cookie,char *new_etax_cookie,char *new_tpass_cookie,char *new_dppt_cookie,int use_cookie_flag)
{
	int i = 0 , nRet = -1;
	if(use_cookie_flag){
		for(i=0;i<5;i++){
			nRet = etax_invoice_query_detail_by_cookie_Interface(area_code,etax_cookie,tpass_cookie,dppt_cookie,new_etax_cookie,new_tpass_cookie,new_dppt_cookie,fpcxsj,fpcxdata,errinfo);
			if(nRet != RET_SUCCESS){
				if(strstr(errinfo,"502 Bad Gateway") || strstr(errinfo,"初始化dppt服务器失败") || strstr(errinfo,"请求公钥或者发送SM4密钥失败"))
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

//发票查询下载
int function_common_etax_invoice_query_down_Interface(char *busid,char *fpcxsj,char *related_type,char *full_name,char *mobile,char *idcard,char *sz_password,int realationtype,char *uniqueIdentity,char *realationStatus,char *errinfo,char **fpcxdata,char **fpcxname,char *area_code,char *etax_cookie,char *tpass_cookie,char *dppt_cookie,char *new_etax_cookie,char *new_tpass_cookie,char *new_dppt_cookie,int use_cookie_flag)
{
	int i = 0 , nRet = -1;
	if(use_cookie_flag){
		for(i=0;i<5;i++){
			nRet = etax_invoice_query_down_by_cookie_Interface(area_code,etax_cookie,tpass_cookie,dppt_cookie,new_etax_cookie,new_tpass_cookie,new_dppt_cookie,fpcxsj,errinfo,fpcxdata,fpcxname);
			if(nRet != RET_SUCCESS){
				if(strstr(errinfo,"502 Bad Gateway") || strstr(errinfo,"初始化dppt服务器失败") || strstr(errinfo,"请求公钥或者发送SM4密钥失败"))
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

//发票开具RPA扫码
int function_common_etax_rpa_invoice_issuance_Interface(char *busid,char *fpkjurl,char *nsrsbh,char *related_type,char *mobile,char *idcard,char *sz_password,int realationtype,char *uniqueIdentity,char *realationStatus,char *errinfo,char **fpkjdata,char *area_code,char *etax_cookie,char *tpass_cookie,char *dppt_cookie,char *new_etax_cookie,char *new_tpass_cookie,char *new_dppt_cookie,int use_cookie_flag)
{
	int i = 0 , nRet = -1;
	if(use_cookie_flag){
		for(i=0;i<5;i++){
			nRet = etax_invoice_rpa_invoice_issuance_by_cookie_Interface(area_code,etax_cookie,tpass_cookie,dppt_cookie,new_etax_cookie,new_tpass_cookie,new_dppt_cookie,errinfo,fpkjurl,nsrsbh,fpkjdata);
			if(nRet != RET_SUCCESS){
				if(strstr(errinfo,"502 Bad Gateway") || strstr(errinfo,"初始化dppt服务器失败") || strstr(errinfo,"请求公钥或者发送SM4密钥失败"))
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
				if(strstr(errinfo,"502 Bad Gateway") || strstr(errinfo,"初始化dppt服务器失败") || strstr(errinfo,"请求公钥或者发送SM4密钥失败"))
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

//授信额度查询
int function_common_credit_limit_Interface(char *busid,char *nsrsbh,char *related_type,char *mobile,char *idcard,char *sz_password,int realationtype,char *uniqueIdentity,char *realationStatus,char *errinfo,char **sxeddata,char *area_code,char *etax_cookie,char *tpass_cookie,char *dppt_cookie,char *new_etax_cookie,char *new_tpass_cookie,char *new_dppt_cookie,int use_cookie_flag)
{
	int i = 0 , nRet = -1;
	if(use_cookie_flag){
		for(i=0;i<5;i++){
			nRet = etax_credit_limit_by_cookie_Interface(area_code,etax_cookie,tpass_cookie,dppt_cookie,new_etax_cookie,new_tpass_cookie,new_dppt_cookie,errinfo,nsrsbh,sxeddata);
			if(nRet != RET_SUCCESS){
				if(strstr(errinfo,"502 Bad Gateway") || strstr(errinfo,"初始化dppt服务器失败") || strstr(errinfo,"请求公钥或者发送SM4密钥失败"))
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

//蓝字发票开具
int function_common_etax_blue_ink_invoice_issuance_Interface(char *busid,char *fpkjjson,char *related_type,char *mobile,char *idcard,char *sz_password,int realationtype,char *uniqueIdentity,char *realationStatus,char *errinfo,char **fpkjout,char **hqnsrjcxx,char *area_code,char *etax_cookie,char *tpass_cookie,char *dppt_cookie,char *new_etax_cookie,char *new_tpass_cookie,char *new_dppt_cookie,int use_cookie_flag)
{
	int i = 0 , nRet = -1;
	if(use_cookie_flag){
		for(i=0;i<5;i++){
			nRet = etax_blue_ink_invoice_issuance_by_cookie_Interface(area_code,etax_cookie,tpass_cookie,dppt_cookie,new_etax_cookie,new_tpass_cookie,new_dppt_cookie,errinfo,fpkjjson,fpkjout,hqnsrjcxx);
			if(nRet != RET_SUCCESS){
				if(strstr(errinfo,"502 Bad Gateway") || strstr(errinfo,"初始化dppt服务器失败") || strstr(errinfo,"请求公钥或者发送SM4密钥失败"))
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

//可开红票的蓝票查询
int function_common_etax_hzqrxxSzzhQuery_Interface(char *busid,char *fpcxsj,char *related_type,char *full_name,char *mobile,char *idcard,char *sz_password,int realationtype,char *uniqueIdentity,char *realationStatus,char *errinfo,char **fpcxdata,char *area_code,char *etax_cookie,char *tpass_cookie,char *dppt_cookie,char *new_etax_cookie,char *new_tpass_cookie,char *new_dppt_cookie,int use_cookie_flag)
{
	int i = 0 , nRet = -1;
	if(use_cookie_flag){
		for(i=0;i<5;i++){
			nRet = etax_hzqrxxSzzhQuery_by_cookies_Interface(area_code,etax_cookie,tpass_cookie,dppt_cookie,new_etax_cookie,new_tpass_cookie,new_dppt_cookie,fpcxsj,fpcxdata,errinfo);
			if(nRet != RET_SUCCESS){
				if(strstr(errinfo,"502 Bad Gateway") || strstr(errinfo,"初始化dppt服务器失败") || strstr(errinfo,"请求公钥或者发送SM4密钥失败"))
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

//红字确认单开具
int function_common_etax_hzqrxxSave_Interface(char *busid,char *fpcxsj,char *related_type,char *full_name,char *mobile,char *idcard,char *sz_password,int realationtype,char *uniqueIdentity,char *realationStatus,char *errinfo,char **fpcxdata,char *area_code,char *etax_cookie,char *tpass_cookie,char *dppt_cookie,char *new_etax_cookie,char *new_tpass_cookie,char *new_dppt_cookie,int use_cookie_flag)
{
	int i = 0 , nRet = -1;
	if(use_cookie_flag){
		for(i=0;i<5;i++){
			nRet = etax_hzqrxxSave_by_cookies_Interface(area_code,etax_cookie,tpass_cookie,dppt_cookie,new_etax_cookie,new_tpass_cookie,new_dppt_cookie,fpcxsj,fpcxdata,errinfo);
			if(nRet != RET_SUCCESS){
				if(strstr(errinfo,"502 Bad Gateway") || strstr(errinfo,"初始化dppt服务器失败") || strstr(errinfo,"请求公钥或者发送SM4密钥失败"))
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

//红字确认单列表查询
int function_common_etax_hzqrxxQuery_Interface(char *busid,char *fpcxsj,char *related_type,char *full_name,char *mobile,char *idcard,char *sz_password,int realationtype,char *uniqueIdentity,char *realationStatus,char *errinfo,char **fpcxdata,char *area_code,char *etax_cookie,char *tpass_cookie,char *dppt_cookie,char *new_etax_cookie,char *new_tpass_cookie,char *new_dppt_cookie,int use_cookie_flag)
{
	int i = 0 , nRet = -1;
	if(use_cookie_flag){
		for(i=0;i<5;i++){
			nRet = etax_hzqrxxQuery_by_cookies_Interface(area_code,etax_cookie,tpass_cookie,dppt_cookie,new_etax_cookie,new_tpass_cookie,new_dppt_cookie,fpcxsj,fpcxdata,errinfo);
			if(nRet != RET_SUCCESS){
				if(strstr(errinfo,"502 Bad Gateway") || strstr(errinfo,"初始化dppt服务器失败") || strstr(errinfo,"请求公钥或者发送SM4密钥失败"))
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

//红字确认单详情查询
int function_common_etax_hzqrxxQueryDetail_Interface(char *busid,char *fpcxsj,char *related_type,char *full_name,char *mobile,char *idcard,char *sz_password,int realationtype,char *uniqueIdentity,char *realationStatus,char *errinfo,char **fpcxdata,char *area_code,char *etax_cookie,char *tpass_cookie,char *dppt_cookie,char *new_etax_cookie,char *new_tpass_cookie,char *new_dppt_cookie,int use_cookie_flag)
{
	int i = 0 , nRet = -1;
	if(use_cookie_flag){
		for(i=0;i<5;i++){
			nRet = etax_hzqrxxQueryDetail_by_cookies_Interface(area_code,etax_cookie,tpass_cookie,dppt_cookie,new_etax_cookie,new_tpass_cookie,new_dppt_cookie,fpcxsj,fpcxdata,errinfo);
			if(nRet != RET_SUCCESS){
				if(strstr(errinfo,"502 Bad Gateway") || strstr(errinfo,"初始化dppt服务器失败") || strstr(errinfo,"请求公钥或者发送SM4密钥失败"))
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

//红冲类查询接口
int function_common_hcstaquery_invoice_issuance_Interface(char *busid,char *fpcxsj,char *related_type,char *mobile,char *idcard,char *sz_password,int realationtype,char *uniqueIdentity,char *realationStatus,char *errinfo,char **fpcxdata,char *area_code,char *etax_cookie,char *tpass_cookie,char *dppt_cookie,char *new_etax_cookie,char *new_tpass_cookie,char *new_dppt_cookie,int use_cookie_flag)
{
	int i = 0 , nRet = -1;
	if(use_cookie_flag){
		for(i=0;i<5;i++){
			nRet = etax_hcstaquery_invoice_issuance_by_cookie_Interface(area_code,etax_cookie,tpass_cookie,dppt_cookie,new_etax_cookie,new_tpass_cookie,new_dppt_cookie,errinfo,fpcxsj,fpcxdata);
			if(nRet != RET_SUCCESS){
				if(strstr(errinfo,"502 Bad Gateway") || strstr(errinfo,"初始化dppt服务器失败") || strstr(errinfo,"请求公钥或者发送SM4密钥失败"))
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
				if(strstr(errinfo,"502 Bad Gateway") || strstr(errinfo,"初始化dppt服务器失败") || strstr(errinfo,"请求公钥或者发送SM4密钥失败"))
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
				if(strstr(errinfo,"502 Bad Gateway") || strstr(errinfo,"初始化dppt服务器失败") || strstr(errinfo,"请求公钥或者发送SM4密钥失败"))
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
			if(strstr(errinfo,"502 Bad Gateway") || strstr(errinfo,"初始化dppt服务器失败") || strstr(errinfo,"请求公钥或者发送SM4密钥失败"))
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
				if(strstr(errinfo,"502 Bad Gateway") || strstr(errinfo,"初始化dppt服务器失败") || strstr(errinfo,"请求公钥或者发送SM4密钥失败"))
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
				if(strstr(errinfo,"502 Bad Gateway") || strstr(errinfo,"初始化dppt服务器失败") || strstr(errinfo,"请求公钥或者发送SM4密钥失败"))
				{
					continue;
				}
				else return nRet;
			}
			else return nRet;
		}
	}
}

