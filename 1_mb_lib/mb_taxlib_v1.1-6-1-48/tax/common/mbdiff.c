/*****************************************************************************
File name: mbdiff.c
Description: 蒙柏方专用函数合集
Author: adde
Version: 1.0
Date: 20210106
History:
*****************************************************************************/
#define mbdiff_c
#include "mbdiff.h"
int init_tax_lib(uint8 *key_zlib_json, char *taxlib_version)
{
	int result = 0;
	sprintf(taxlib_version, "%s", MB_TAX_LIB_VERSION);
	printf("	########38	\r\n");
	logout(INFO, "TAXLIB", "INIT", "当前加密库版本号:%s,迭代版本号：%s,日期:%s\r\n", MB_TAX_LIB_VERSION, MB_TAX_LIB_V_NUM, MB_TAX_LIB_TIME);
#ifdef RELEASE_SO	
	char *cert_json = NULL;
	char enc_cert[1024 * 1024] = { 0 };
	int enc_cert_len;
	memset(&tax_auth, 0, sizeof(struct _tax_auth));
	//判断加密授权文件是否存在
	result = access(AUTH_CERT_FILE_PATH, 0);
	if (result < 0)
	{
		out("授权文件不存在,无法使用税务相关功能\n");
		return 0;
	}
	//读取授权文件
	enc_cert_len = read_file(AUTH_CERT_FILE_PATH, enc_cert, 1024 * 1024);
	if (enc_cert_len < 10)
	{
		out("授权文件读取失败,文件异常\n");
		return 0;
	}
	//解析加密授权文件
	int cert_len;
	result = mb_dec_file_function(key_zlib_json, enc_cert, enc_cert_len, &cert_json, &cert_len);
	if (result != 0)
	{
		out("授权文件验签或解密失败\n");
		return 0;
	}
	//printf("cert json =%s\n", cert_json);
	//解析授权文件内容
	result = analysis_auth_cert_json(cert_json);
	if (result < 0)
	{
		out("授权文件验签或解密失败\n");
		free(cert_json);
		return 0;
	}
	out("税务动态初始化完成\n");
	free(cert_json);

	ProgInitDestory(1);
#else
	result = 100;
	ProgInitDestory(1);
#endif

	//sprintf(taxlib_version,"%s",MB_TAX_LIB_VERSION);
	return result;
}

int reload_tax_auth(uint8 *key_zlib_json)
{
	int result = 0;
#ifdef RELEASE_SO	
	char *cert_json = NULL;
	char *enc_cert = NULL;//[1024 * 1024] = { 0 };
	int enc_cert_len;

	enc_cert = calloc(1,1024*1024);
	if(enc_cert == NULL)
	{
		return -1;
	}

	memset(&tax_auth, 0, sizeof(struct _tax_auth));
	//判断加密授权文件是否存在
	result = access(AUTH_CERT_FILE_PATH, 0);
	if (result < 0)
	{
		if(enc_cert != NULL)
		{	
			free(enc_cert);
		}
		out("授权文件不存在,无法使用税务相关功能\n");
		return -1;
	}
	//读取授权文件
	enc_cert_len = read_file(AUTH_CERT_FILE_PATH, enc_cert, 1024 * 1024);
	if (enc_cert_len < 10)
	{
		if(enc_cert != NULL)
		{	
			free(enc_cert);
		}
		out("授权文件读取失败,文件异常\n");
		return -2;
	}
	//解析加密授权文件
	int cert_len;
	result = mb_dec_file_function(key_zlib_json, enc_cert, enc_cert_len, &cert_json, &cert_len);
	if (result != 0)
	{
		if(enc_cert != NULL)
		{	
			free(enc_cert);
		}
		out("授权文件验签或解密失败\n");
		return -3;
	}
	//printf("cert json =%s\n", cert_json);
	//解析授权文件内容
	result = analysis_auth_cert_json(cert_json);
	if (result < 0)
	{
		if(enc_cert != NULL)
		{	
			free(enc_cert);
		}
		out("授权文件验签或解密失败\n");
		free(cert_json);
		return -4;
	}
	out("税务动态初始化完成\n");
	if(enc_cert != NULL)
	{	
		free(enc_cert);
	}
	if(cert_json != NULL)
			free(cert_json);
#else
	result = 0;
#endif

	//sprintf(taxlib_version,"%s",MB_TAX_LIB_VERSION);
	return result;
}

int get_cert_json_file(uint8 *key_zlib_json, char **cert_json)
{
	int result = 0;
	char *enc_cert = NULL;//[1024 * 1024] = { 0 };
	int enc_cert_len;
	//判断加密授权文件是否存在

	enc_cert = calloc(1,1024 * 1024);
	if(NULL == enc_cert)
	{
		return -1;
	}

	result = access(AUTH_CERT_FILE_PATH, 0);
	if (result < 0)
	{
		if(NULL != enc_cert)
		{
			free(enc_cert);
		}
		out("授权文件不存在,无法使用税务相关功能\n");
		return -1;
	}
	//读取授权文件
	enc_cert_len = read_file(AUTH_CERT_FILE_PATH, enc_cert, 1024 * 1024);
	if (enc_cert_len < 10)
	{
		if(NULL != enc_cert)
		{
			free(enc_cert);
		}
		out("授权文件读取失败,文件异常\n");
		return -2;
	}
	//解析加密授权文件
	int cert_len;
	result = mb_dec_file_function(key_zlib_json, enc_cert, enc_cert_len, cert_json, &cert_len);
	if (result != 0)
	{
		if(NULL != enc_cert)
		{
			free(enc_cert);
		}
		out("授权文件验签或解密失败\n");
		return result;
	}
	if(NULL != enc_cert)
	{
		free(enc_cert);
	}

	return result;
}


#ifdef RELEASE_SO
static int analysis_auth_cert_json(char *cert_json)
{
	int size;
	int i;
	char num[10];
	cJSON *arrayItem, *item;
	int auth_count = 0;

	//out("授权文件内容:%s\n", cert_json);

	cJSON *root = cJSON_Parse(cert_json);
	if (!root)
	{
		out("Error before: [%s]\n", cJSON_GetErrorPtr());
		return -1;
	}
	item = cJSON_GetObjectItem(root, "version");
	if (get_json_value_can_not_null(item, tax_auth.version, 0, 128) < 0) {
		out("解析授权文件版本号失败\n");
		goto End;
	}

	item = cJSON_GetObjectItem(root, "serial_num");
	if (get_json_value_can_not_null(item, tax_auth.serial_num, 0, 128) < 0) {
		out("解析授权文件序列号失败\n");
		goto End;
	}

	item = cJSON_GetObjectItem(root, "name");
	if (get_json_value_can_not_null(item, tax_auth.name, 0, 128) < 0) {
		out("解析授权文件名称失败\n");
		goto End;
	}

	cJSON *object_tax = cJSON_GetObjectItem(root, "tax");
	if (object_tax == NULL)
	{
		out("Error before: [%s]\n", cJSON_GetErrorPtr());
		goto End;
	}
	size = cJSON_GetArraySize(object_tax);
	if (size < 1)
	{
		out("Error before: [%s]\n", cJSON_GetErrorPtr());
		goto End;
	}
	for (i = 0; i < size; i++)
	{
		//int num_i;
		arrayItem = cJSON_GetArrayItem(object_tax, i);
		if (arrayItem == NULL)
		{
			out("Error before: [%s]\n", cJSON_GetErrorPtr());
			goto End;
		}
		memset(num, 0, sizeof(num));
		item = cJSON_GetObjectItem(arrayItem, "port_num");
		if (get_json_value_can_not_null(item, num, 0, 10) < 0) {
			out("解析USB端口号失败,i=%d\n", i);
			goto End;
		}
		tax_auth.busid_auth[i].port = atoi(num);

		item = cJSON_GetObjectItem(arrayItem, "busid");
		if (get_json_value_can_not_null(item, tax_auth.busid_auth[i].busid, 0, 32) < 0) {
			out("解析busid失败,i=%d\n", i);
			goto End;
		}
		if (strcmp(tax_auth.version,"V1.1.01") <= 0)
		{
			memset(num, 0, sizeof(num));
			item = cJSON_GetObjectItem(arrayItem, "aisino_sup");
			if (get_json_value_can_not_null(item, num, 0, 10) < 0) {
				out("解析航信盘支持失败,i=%d\n", i);
				goto End;
			}
			tax_auth.busid_auth[i].aisino_sup = atoi(num);
			tax_auth.busid_auth[i].cntax_sup = atoi(num);
			tax_auth.busid_auth[i].mengbai_sup = atoi(num);

			memset(num, 0, sizeof(num));
			item = cJSON_GetObjectItem(arrayItem, "nesic_sup");
			if (get_json_value_can_not_null(item, num, 0, 10) < 0) {
				out("解析百旺盘支持失败,i=%d\n", i);
				goto End;
			}
			tax_auth.busid_auth[i].nisec_sup = atoi(num);

			item = cJSON_GetObjectItem(arrayItem, "start_time");
			if (get_json_value_can_not_null(item, tax_auth.busid_auth[i].start_time, 0, 32) < 0) {
				out("解析起始时间失败,i=%d\n", i);
				goto End;
			}

			item = cJSON_GetObjectItem(arrayItem, "end_time");
			if (get_json_value_can_not_null(item, tax_auth.busid_auth[i].end_time, 0, 32) < 0) {
				out("解析起始时间失败,i=%d\n", i);
				goto End;
			}

			strcpy(tax_auth.busid_auth[i].aisino_end_time, tax_auth.busid_auth[i].end_time);
			strcpy(tax_auth.busid_auth[i].nisec_end_time, tax_auth.busid_auth[i].end_time);
			strcpy(tax_auth.busid_auth[i].cntax_end_time, tax_auth.busid_auth[i].end_time);
			strcpy(tax_auth.busid_auth[i].mengbai_end_time, tax_auth.busid_auth[i].end_time);
		}
		else
		{
			char *p1,*p2;
			char auth_str[10] = {0};
			char tmp_str[100] = {0};


			//金税盘
			memset(tmp_str, 0, sizeof(tmp_str));
			item = cJSON_GetObjectItem(arrayItem, "aisino_auth");
			if (get_json_value_can_not_null(item, tmp_str, 0, sizeof(tmp_str)) < 0) {
				out("解析金税盘授权信息失败,i=%d\n", i);
				continue;
			}
			p1 = strstr(tmp_str, ",");
			if (p1 == NULL)
				continue;
			p2 += strlen(",");
			memset(auth_str, 0, sizeof(auth_str));
			memcpy(auth_str, tmp_str, p1 - tmp_str);
			tax_auth.busid_auth[i].aisino_sup = atoi(auth_str);

			p1 = strstr(p2, ",");
			if (p1 == NULL)
				continue;
			memcpy(tax_auth.busid_auth[i].aisino_end_time, p2, p1 - p2);
			p1 = NULL;
			p2 = NULL;

			//税控盘
			memset(tmp_str, 0, sizeof(tmp_str));
			item = cJSON_GetObjectItem(arrayItem, "nesic_auth");
			if (get_json_value_can_not_null(item, tmp_str, 0, sizeof(tmp_str)) < 0) {
				out("解析金税盘授权信息失败,i=%d\n", i);
				continue;
			}
			p1 = strstr(tmp_str, ",");
			if (p1 == NULL)
				continue;
			p2 += strlen(",");
			memset(auth_str, 0, sizeof(auth_str));
			memcpy(auth_str, tmp_str, p1 - tmp_str);
			tax_auth.busid_auth[i].nisec_sup = atoi(auth_str);

			p1 = strstr(p2, ",");
			if (p1 == NULL)
				continue;
			memcpy(tax_auth.busid_auth[i].nisec_end_time, p2, p1 - p2);
			p1 = NULL;
			p2 = NULL;

			//税务ukey
			memset(tmp_str, 0, sizeof(tmp_str));
			item = cJSON_GetObjectItem(arrayItem, "cntax_auth");
			if (get_json_value_can_not_null(item, tmp_str, 0, sizeof(tmp_str)) < 0) {
				out("解析税务ukey授权信息失败,i=%d\n", i);
				continue;
			}
			p1 = strstr(tmp_str, ",");
			if (p1 == NULL)
				continue;
			p2 += strlen(",");
			memset(auth_str, 0, sizeof(auth_str));
			memcpy(auth_str, tmp_str, p1 - tmp_str);
			tax_auth.busid_auth[i].mengbai_sup = atoi(auth_str);

			p1 = strstr(p2, ",");
			if (p1 == NULL)
				continue;
			memcpy(tax_auth.busid_auth[i].cntax_end_time, p2, p1 - p2);
			p1 = NULL;
			p2 = NULL;

			//蒙柏
			memset(tmp_str, 0, sizeof(tmp_str));
			item = cJSON_GetObjectItem(arrayItem, "mengbai_auth");
			if (get_json_value_can_not_null(item, tmp_str, 0, sizeof(tmp_str)) < 0) {
				out("解析模拟盘授权信息失败,i=%d\n", i);
				continue;
			}
			p1 = strstr(tmp_str, ",");
			if (p1 == NULL)
				continue;
			p2 += strlen(",");
			memset(auth_str, 0, sizeof(auth_str));
			memcpy(auth_str, tmp_str, p1 - tmp_str);
			tax_auth.busid_auth[i].mengbai_sup = atoi(auth_str);

			p1 = strstr(p2, ",");
			if (p1 == NULL)
				continue;
			memcpy(tax_auth.busid_auth[i].mengbai_end_time, p2, p1 - p2);
			p1 = NULL;
			p2 = NULL;
		}

		tax_auth.busid_auth[i].state = 1;
		if ((tax_auth.busid_auth[i].aisino_sup == 1) || (tax_auth.busid_auth[i].nisec_sup == 1) || (tax_auth.busid_auth[i].cntax_sup == 1))
		{
			auth_count += 1;
		}

	}
End:
	cJSON_Delete(root);
	return auth_count;
}
#endif

static int auth_read_int(const char *path, const char *name)
{
	char buf[200] = { 0 };
	char file_name[256] = { 0 };
	int result;
	memset(buf, 0, sizeof(buf));
	memset(file_name, 0, sizeof(file_name));
	sprintf(file_name, "%s/%s", path, name);
	if (read_file(file_name, buf, sizeof(buf))<0)
	{
		out("auth_read_int读取文件[%s]失败\n", file_name);
		return -1;
	}
	sscanf(buf, "%d\n", &result);
	return result;
}

static int auth_get_busid_busnum(char *busid)
{
	int busnum = 0;
	char path[200] = { 0 };
	sprintf(path, "%s/%s", "/sys/bus/usb/devices", busid);
	busnum = auth_read_int(path, "busnum");
	return busnum;
}

static int auth_get_busid_devnum(char *busid)
{
	int devnum = 0;
	char path[200] = { 0 };
	sprintf(path, "%s/%s", "/sys/bus/usb/devices", busid);
	devnum = auth_read_int(path, "devnum");
	return devnum;
}

int jude_plate_auth(uint8 bDeviceType, char *busid,char *errinfo)
{
	//_WriteLog(LL_INFO,"jude_plate_auth bDeviceType %d",bDeviceType);

//#ifndef DF_UK_NEED_AUTH
//	if (bDeviceType == DEVICE_CNTAX)
//	{
//		return 1;//9字头版本可能存在没有授权文件的情况，在此判断直接返回。
//	}
//#endif

#ifdef RELEASE_SO
	//_WriteLog(LL_INFO,"最大端口支持%d口",MAX_USB_PORT_NUM);
	int i;
	for (i = 0; i < MAX_USB_PORT_NUM; i++)
	{
		if (tax_auth.busid_auth[i].state == 0)
			continue;
		#ifndef MB2201
		if (strcmp(busid, tax_auth.busid_auth[i].busid) != 0)		//wang 02 02 一口授权问题修改
			continue;
		#endif
		char date_now[20] = { 0 };
		timer_read_y_m_d_h_m_s(date_now);
		if(bDeviceType == DEVICE_AISINO)
		{
			if(tax_auth.busid_auth[i].aisino_sup!= 1)
			{
				sprintf(errinfo,"该端口未授权金税盘使用");
				return -1;			
			}
			if (memcmp(date_now, tax_auth.busid_auth[i].aisino_end_time, 10) > 0)
			{
				sprintf(errinfo,"该端口金税盘授权到期");
				return -1;
			}
			return 1;
		}
		else if(bDeviceType == DEVICE_NISEC)
		{
			if(tax_auth.busid_auth[i].nisec_sup!= 1)
			{
				sprintf(errinfo,"该端口未授权税控盘使用");
				return -1;
			}
			if (memcmp(date_now, tax_auth.busid_auth[i].nisec_end_time, 10) > 0)
			{
				sprintf(errinfo,"该端口税控盘授权到期");
				return -1;
			}
			return 1;
		}
		else if (bDeviceType == DEVICE_CNTAX)
		{
			if (tax_auth.busid_auth[i].aisino_sup != 1)
			{
				sprintf(errinfo,"该端口未授权税务ukey使用");
				return -1;
			}
			if (memcmp(date_now, tax_auth.busid_auth[i].cntax_end_time, 10) > 0)
			{
				sprintf(errinfo,"该端口税务ukey授权到期");
				return -1;
			}
			return 1;			
		}
		else if ((bDeviceType == DEVICE_MENGBAI)||(bDeviceType == DEVICE_MENGBAI2)||(bDeviceType == DEVICE_SKP_KEY))
		{
			return 1;
		}

		return -1;
	}
#else
	//测试程序默认全部权限
	return 1;
#endif
	return 0;
}

int jude_busnum_devnum_handle(struct HandlePlate *h_plate)
{
	int bus_id;
	int dev_id;

	if (h_plate == NULL)
	{
		logout(INFO, "TAXLIB", "INTERFACE", "h_plate 句柄发生变化\r\n");
		return -1;
	}
	if (h_plate->hDev == NULL)
	{
		h_plate->nBusID = -1;
		h_plate->nDevID = -1;
		logout(INFO, "TAXLIB", "INTERFACE", "hDev 句柄发生变化\r\n");
		return -2;
	}
	if (h_plate->hDev->hUSB == NULL)
	{
		h_plate->nBusID = -1;
		h_plate->nDevID = -1;
		logout(INFO, "TAXLIB", "INTERFACE", "hUSB 句柄发生变化\r\n");
		return -3;
	}

	bus_id = auth_get_busid_busnum(h_plate->busid);
	dev_id = auth_get_busid_devnum(h_plate->busid);
	if ((bus_id != h_plate->nBusID) || (dev_id != h_plate->nDevID))
	{
		h_plate->nBusID = -1;
		h_plate->nDevID = -1;
		logout(INFO, "TAXLIB", "INTERFACE", "USB端口发生变化,信息不一致,原记录中busnum = %d,devnum = %d,当前busnum = %d,devnum = %d\r\n", h_plate->nBusID, h_plate->nDevID, bus_id, dev_id);
		return -4;
	}
	return 0;
}




//该函数输入参数两种方法
// 1）输入发票号码、代码 可以查找当前月份或者上月尚未抄报的发票信息，但是抄报了就不行
// 2）输全3个参数，可以查询任意存在发票,最大查询实际可能是半年？？？
// 3) 注意如果是012345678901 发票代码，必须带前面0进行查询，12345678901则错误
int QueryInvInfoOutBinStr(HDEV hDev, char *fpdm, char *fphm, uint32 nDZSYH, HFPXX pOutFpxx, char **kp_bin, char **kp_str)
{
	int nIndex = 0;
	uint8 *pInvDecBuf = NULL;
	uint8 cb[768] = { 0 };
	int nChildRet = 0;
	int ncbLen = sizeof(cb);
	int nRet = -2;
	memcpy(cb, "\xfe\xff\xcc\xbc\x53\xe1\xe3\x00\x1f\x00", 10);
	if ((nChildRet = AisinoLogicIO(hDev->hUSB, cb, 10, ncbLen, NULL, 0)) < 0)
		return -1;

	uint8 abQueryParam[14] = { 0 };
	memset(abQueryParam, 0, sizeof(abQueryParam));
	InvTypeCode2Byte5(abQueryParam, fpdm);
	*(uint32 *)(abQueryParam + 5) = atoi(fphm);
	if (0 != nDZSYH) {
		int nDZSYH_R = bswap_32(nDZSYH);
		memcpy(abQueryParam + 9, &nDZSYH_R, 4);
	}
	abQueryParam[13] = MakePackVerityFlag(abQueryParam, 13);
	int nMaxEncodeInvoiceLen = DEF_MAX_FPBUF_LEN;
	uint8 *pInvIOBuf = calloc(1, nMaxEncodeInvoiceLen);
	uint8 bContinue = 1;
	while (bContinue) {
		memcpy(cb, "\xfe\xff\xcc\xbc\x53\xe1\xe3\x01\x02\x00", 10);
		nChildRet = AisinoLogicIO(hDev->hUSB, cb, 10, ncbLen, abQueryParam, 14);
		if (nChildRet <= 0)
			break;
		//设置continue
		bContinue = cb[nChildRet + 1];
		int nDataLen = nChildRet - 1;  //最后一个是校验字符需要去掉
		memcpy(pInvIOBuf + nIndex, cb, nDataLen);
		nIndex += nDataLen;
		if ((cb[nDataLen - 4] == 0x54 && cb[nDataLen - 3] == 0x61 && cb[nDataLen - 2] == 0x69 &&
			cb[nDataLen - 1] == 0x6c) ||
			nIndex > 10240000)  //当读取到发票结尾标志或者设定发票最大值 10M
			break;
		if (nMaxEncodeInvoiceLen - nIndex < sizeof(cb)) {  // dynamic memory
			nMaxEncodeInvoiceLen = nMaxEncodeInvoiceLen + 10 * sizeof(cb);
			uint8 *pNewBuff = (uint8 *)realloc(pInvIOBuf, nMaxEncodeInvoiceLen);
			if (pNewBuff == NULL) {
				break;
			}
			pInvIOBuf = pNewBuff;
		}
	}
	if (nIndex <= 512) {
		nRet = -3;
		goto QueryInvInfo_Finish;
	}
	//_WriteHexToDebugFile("raw_enc-fp.bin", pInvIOBuf, nIndex);
#ifdef NEED_FP_DATA
	write_file("/mnt/masstore/raw_enc_fp.txt", (char *)pInvIOBuf, nIndex);
#endif
	int nOutBuffLen = nIndex * 30 + 30000;  //正常情况下，扩展10+倍
	pInvDecBuf = calloc(1, nOutBuffLen);
	if (!pInvDecBuf) {
		nRet = -4;
		goto QueryInvInfo_Finish;
	}
	int nDecLen = DecodeInvBuf(hDev, pInvIOBuf, nIndex, pInvDecBuf);
	if (nDecLen < 0) {
		nRet = -5;
		goto QueryInvInfo_Finish;
	}
	int result;
	// _WriteHexToDebugFile("raw_dec-fp.bin", pInvDecBuf, nDecLen);
#ifdef NEED_FP_DATA
	write_file("/mnt/masstore/raw_dec_fp.txt", (char *)pInvDecBuf, nDecLen);
#endif
	if ((result = AisinoInvRaw2Fpxx(pInvDecBuf, pOutFpxx)) < 0) {
		nRet = -6;
		_WriteLog(LL_DEBUG, "InvRaw2Fpxx,failed result = %d\n", result);
		goto QueryInvInfo_Finish;
	}
	//_WriteLog(LL_DEBUG, "FPQueryResult - fpdm:%s fphm:%s gfmc:%s gfdzdh:%s jshj:%s", pOutFpxx->fpdm,
	//          pOutFpxx->fphm, pOutFpxx->gfmc, pOutFpxx->gfdzdh, pOutFpxx->jshj);
	nRet = 0;
QueryInvInfo_Finish:

	if (nRet == 0)
	{
		char *bindatabase;
		char *strdatabase;

		bindatabase = calloc(1, nIndex * 2 + 1000);
		if (!bindatabase)
		{
			free(pInvIOBuf);
			free(pInvDecBuf);
			return -100;
		}

		Base64_Encode((char *)pInvIOBuf, nIndex, bindatabase);
		*kp_bin = bindatabase;

		strdatabase = calloc(1, nDecLen * 2 + 1000);
		if (!strdatabase)
		{
			free(pInvIOBuf);
			free(pInvDecBuf);
			free(bindatabase);
			return -101;
		}

		Base64_Encode((char *)pInvDecBuf, nDecLen, strdatabase);
		*kp_str = strdatabase;
	}


	free(pInvIOBuf);
	if (pInvDecBuf)
		free(pInvDecBuf);
	return nRet;
}



//发票bin数据解析到通用结构体在转成json的base64数据--对应发票数据文件V1.0.2版本V3.0
int invoice_bin_fpxx_json_base_outfpxx(struct StaticDeviceInfo *pDevInfo, char *pInvDecBuf, char **inv_json, int sfxqqd, HFPXX fpxx)//最新版本的发票数据接口
{
	int ret = -1;
	int result = 0;
	fpxx->hDev = pDevInfo;
	result = AisinoInvRaw2Fpxx((uint8 *)pInvDecBuf, fpxx);
	if (result  < 0)
	{
		printf("invoice_bin_fpxx_json_base_outfpxx AisinoInvRaw2Fpxx result %d\n", result);
		goto get_end;
	}
	fpxx_to_json_base_fpsjbbh_v102(fpxx, inv_json, sfxqqd,DF_FP_JSON);
	ret = 0;
get_end:
	return ret;
}

//发票bin数据解析到通用结构体在转成json的base64数据--对应发票数据文件V1.0.2版本V3.0
int invoice_bin_fpxx_json_base(struct StaticDeviceInfo *pDevInfo, char *pInvDecBuf, char **inv_json, int sfxqqd, uint8 *zfbz)//最新版本的发票数据接口
{
	int ret = -1;
	int result = 0;
	HFPXX fpxx = MallocFpxx();
	fpxx->hDev = pDevInfo;
	result = AisinoInvRaw2Fpxx((uint8 *)pInvDecBuf, fpxx);
	if (result  < 0)
	{
		printf("invoice_bin_fpxx_json_base AisinoInvRaw2Fpxx result %d\n", result);
		goto get_end;
	}
	//_WriteLog(LL_INFO,"发票二进制解析到结构体完成\n");
	fpxx_to_json_base_fpsjbbh_v102(fpxx, inv_json, sfxqqd,DF_FP_BASE);
	_WriteLog(LL_INFO, "发票结构体组包到json完成,发票代码：%s，发票号码：%s\n", fpxx->fpdm, fpxx->fphm);
	*zfbz = fpxx->zfbz;
	ret = 0;
get_end:
	if (fpxx)
	{
		//_WriteLog(LL_INFO,"释放发票结果体指针\n");
		FreeFpxx(fpxx);
	}
	return ret;
}
int invoice_bin_fpxx_json_base_old(struct StaticDeviceInfo *pDevInfo, char *pInvDecBuf, char **inv_json)//高灯现在使用的接口
{
	int ret = -1;
	int result = 0;
	HFPXX fpxx = MallocFpxx();
	fpxx->hDev = pDevInfo;
	result = AisinoInvRaw2Fpxx((uint8 *)pInvDecBuf, fpxx);
	if (result  < 0)
	{
		printf("invoice_bin_fpxx_json_base_old AisinoInvRaw2Fpxx result %d\n", result);
		goto get_end;
	}
	//_WriteLog(LL_INFO,"发票二进制解析到结构体完成\n");
	fpxx_to_json_base_ykfpcx(fpxx, inv_json);
	//_WriteLog(LL_INFO,"发票结构体组包到json完成\n");
	ret = 0;
get_end:
	if (fpxx)
	{
		//_WriteLog(LL_INFO,"释放发票结果体指针\n");
		FreeFpxx(fpxx);
	}
	return ret;
}
//=============================================我司自行增加代码=============================================//
//==========================================================================================================//
//==========================================================================================================//
//==========================================================================================================//
//==========================================================================================================//
//==========================================================================================================//
//==========================================================================================================//
//==========================================================================================================//
//==========================================================================================================//
//==========================================================================================================//
//==========================================================================================================//

static unsigned char *SearchBytes(unsigned char *pBegin, unsigned char *pEnd, unsigned char *s, unsigned int nsize)
{
	unsigned int count;
	unsigned char *pDst;

	while ((unsigned long)pBegin + (unsigned long)nsize <= (unsigned long)pEnd)
	{
		pDst = s;
		count = 0;
		while (count < nsize && *pBegin == *pDst)
		{
			pBegin++;
			pDst++;
			count++;
		}
		if (count == nsize)
			break;
		pBegin = pBegin - count + 1;
	}
	//printf("count = %d \n",count);
	if (count == nsize)
	{
		return (unsigned char *)((unsigned long)pBegin - (unsigned long)count);
	}
	return NULL;

}


//航信勾选税盘支持

int LoadAisinoSelectDevInfo(HDEV hDev, HUSB hUSB, uint8 bAuthDev)
{
	int nRet = -1;
	hDev->hUSB = hUSB;
	hDev->bDeviceType = DEVICE_AISINO;
#ifdef _NODEVICE_TEST
	if (LoadNoDeviceJson(hDev) < 0)
		goto LoadAisinoDevInfo_Finish;
	nRet = 0;
	goto LoadAisinoDevInfo_Finish;
#endif
	//压缩税号
	if (GetCompressTaxCode(hUSB, hDev->szCompressTaxID) < 0) {
		nRet = -2;
		goto LoadAisinoDevInfo_Finish;
	}
	//// 9位hash税号
	if (Get9BitHashTaxCode(hUSB, hDev->sz9ByteHashTaxID) < 0) {
		nRet = -3;
		goto LoadAisinoDevInfo_Finish;
	}

	_WriteLog(LL_FATAL, "压缩税号：%s，hash税号：%s\n", hDev->szCompressTaxID, hDev->sz9ByteHashTaxID);
	//分机号码
	//hDev->uICCardNo = GetICCardNo(device);
	////区域代码
	//if (GetRegionCode(device, hDev->szRegCode) < 0) {
	//	nRet = -4;
	//	goto LoadAisinoDevInfo_Finish;
	//}
	//if (AisinoGetDeivceTime(device, hDev->szDeviceTime) < 0) {
	//	nRet = -5;
	//	goto LoadAisinoDevInfo_Finish;
	//}
	//设备编号SN
	if (ReadTaxDeviceID(hUSB, hDev->szDeviceID) < 0) {
		nRet = -6;
		goto LoadAisinoDevInfo_Finish;
	}
	_WriteLog(LL_FATAL, "第一次获取到的机器编号：%s\n", hDev->szDeviceID);




	uint8 cb[51200] = "";
	char data[512] = "";
	int nRetChild;
	////_WriteLog(LL_FATAL,"第1次交互\n");
	//memset(cb, 0, sizeof(cb));
	//memset(data, 0, sizeof(data));
	//memcpy(cb, "\xf1\xf1\x00\x00\x00\x00\x02\x08\x00\xff", 10);
	//memcpy(data, "\xb5\xf0\xaa\x1d\x69\x10\x51\x61", 8);
	//int nRetChild = mass_storage_io(device, (uint8 *)cb, 10, 0, MSC_DIR_H2D, (uint8 *)data,	8, (uint8 *)cb, sizeof(cb));
	//if (nRetChild != 0)
	//	return -1;
	//memcpy(cb, "\xf1\xf1\x00\x00\x00\x00\x01\x04\x01\xff", 10);
	//nRetChild =	mass_storage_io(device, (uint8 *)cb, 10, 0, MSC_DIR_D2H, NULL, 0, (uint8 *)cb, sizeof(cb));
	//if (nRetChild < 0)
	//	return -2;
	//char *pRawID = cb + 0x12;
	//pRawID[12] = '\0';
	//printf_array(cb, 200);
	//_WriteLog(LL_FATAL,"第二次获取到的机器编号：%s\n", pRawID);

	//_WriteLog(LL_FATAL,"第2次交互\n");
	memset(cb, 0, sizeof(cb));
	memset(data, 0, sizeof(data));
	memcpy(cb, "\xf1\x00\x00\x00\x00\x00\x02\x08\x00\xff", 10);
	memcpy(data, "\x00\x05\x00\x00\xac\x06\x00\x04", 8);
	nRetChild = mass_storage_io(hUSB, (uint8 *)cb, 10, 0, MSC_DIR_H2D, (uint8 *)data, 8, (uint8 *)cb, sizeof(cb));
	if (nRetChild != 0)
		return -3;
	memset(cb, 0, sizeof(cb));
	memcpy(cb, "\xf1\x00\x00\x00\x00\x00\x01\xfc\x08\xff", 10);
	nRetChild = mass_storage_io(hUSB, (uint8 *)cb, 10, 0, MSC_DIR_D2H, NULL, 0, (uint8 *)cb, sizeof(cb));
	if (nRetChild < 0)
		return -4;
	//printf_array(cb,200);

	//_WriteLog(LL_FATAL,"第3次交互\n");
	memset(cb, 0, sizeof(cb));
	memset(data, 0, sizeof(data));
	memcpy(cb, "\xf1\x00\x00\x00\x00\x00\x02\x0a\x00\xff", 10);
	memcpy(data, "\x00\x07\x00\x00\xa4\x00\x00\x02\x4f\x01", 10);
	nRetChild = mass_storage_io(hUSB, (uint8 *)cb, 10, 0, MSC_DIR_H2D, (uint8 *)data, 10, (uint8 *)cb, sizeof(cb));
	if (nRetChild != 0)
		return -5;
	memset(cb, 0, sizeof(cb));
	memcpy(cb, "\xf1\x00\x00\x00\x00\x00\x01\xfc\x08\xff", 10);
	nRetChild = mass_storage_io(hUSB, (uint8 *)cb, 10, 0, MSC_DIR_D2H, NULL, 0, (uint8 *)cb, sizeof(cb));
	if (nRetChild < 0)
		return -6;
	//printf_array(cb, 200);


	//_WriteLog(LL_FATAL,"第4次交互\n");
	memset(cb, 0, sizeof(cb));
	memset(data, 0, sizeof(data));
	memcpy(cb, "\xf1\x00\x00\x00\x00\x00\x02\x08\x00\xff", 10);
	memcpy(data, "\x00\x05\x00\x00\xac\x02\x00\x04", 8);
	nRetChild = mass_storage_io(hUSB, (uint8 *)cb, 10, 0, MSC_DIR_H2D, (uint8 *)data, 8, (uint8 *)cb, sizeof(cb));
	if (nRetChild != 0)
		return -5;
	memset(cb, 0, sizeof(cb));
	memcpy(cb, "\xf1\x00\x00\x00\x00\x00\x01\xfc\x08\xff", 10);
	nRetChild = mass_storage_io(hUSB, (uint8 *)cb, 10, 0, MSC_DIR_D2H, NULL, 0, (uint8 *)cb, sizeof(cb));
	if (nRetChild < 0)
		return -6;
	//printf_array(cb, 200);


	//_WriteLog(LL_FATAL,"第5次交互\n");
	memset(cb, 0, sizeof(cb));
	memset(data, 0, sizeof(data));
	memcpy(cb, "\xf1\x00\x00\x00\x00\x00\x02\x08\x00\xff", 10);
	memcpy(data, "\x00\x05\x00\x00\xac\x00\x01\x01", 8);
	nRetChild = mass_storage_io(hUSB, (uint8 *)cb, 10, 0, MSC_DIR_H2D, (uint8 *)data, 8, (uint8 *)cb, sizeof(cb));
	if (nRetChild != 0)
		return -5;
	memset(cb, 0, sizeof(cb));
	memcpy(cb, "\xf1\x00\x00\x00\x00\x00\x01\xfc\x08\xff", 10);
	nRetChild = mass_storage_io(hUSB, (uint8 *)cb, 10, 0, MSC_DIR_D2H, NULL, 0, (uint8 *)cb, sizeof(cb));
	if (nRetChild < 0)
		return -6;
	//printf_array(cb, 200);



	//_WriteLog(LL_FATAL,"第6次交互\n");
	memset(cb, 0, sizeof(cb));
	memset(data, 0, sizeof(data));
	memcpy(cb, "\xf1\x00\x00\x00\x00\x00\x02\x08\x00\xff", 10);
	memcpy(data, "\x00\x05\x00\x00\x84\x00\x00\x08", 8);
	nRetChild = mass_storage_io(hUSB, (uint8 *)cb, 10, 0, MSC_DIR_H2D, (uint8 *)data, 8, (uint8 *)cb, sizeof(cb));
	if (nRetChild != 0)
		return -5;
	memset(cb, 0, sizeof(cb));
	memcpy(cb, "\xf1\x00\x00\x00\x00\x00\x01\xfc\x08\xff", 10);
	nRetChild = mass_storage_io(hUSB, (uint8 *)cb, 10, 0, MSC_DIR_D2H, NULL, 0, (uint8 *)cb, sizeof(cb));
	if (nRetChild < 0)
		return -6;
	//printf_array(cb, 200);

	//_WriteLog(LL_FATAL,"第7次交互\n");
	memset(cb, 0, sizeof(cb));
	memset(data, 0, sizeof(data));
	memcpy(cb, "\xf1\x00\x00\x00\x00\x00\x02\x0a\x00\xff", 10);
	memcpy(data, "\x00\x07\x00\x00\xa4\x00\x00\x02\x3f\x00", 10);
	nRetChild = mass_storage_io(hUSB, (uint8 *)cb, 10, 0, MSC_DIR_H2D, (uint8 *)data, 10, (uint8 *)cb, sizeof(cb));
	if (nRetChild != 0)
		return -5;
	memset(cb, 0, sizeof(cb));
	memcpy(cb, "\xf1\x00\x00\x00\x00\x00\x01\xfc\x08\xff", 10);
	nRetChild = mass_storage_io(hUSB, (uint8 *)cb, 10, 0, MSC_DIR_D2H, NULL, 0, (uint8 *)cb, sizeof(cb));
	if (nRetChild < 0)
		return -6;
	//printf_array(cb, 200);


	//_WriteLog(LL_FATAL,"第8次交互\n");
	memset(cb, 0, sizeof(cb));
	memset(data, 0, sizeof(data));
	memcpy(cb, "\xf1\x00\x00\x00\x00\x00\x02\x0a\x00\xff", 10);
	memcpy(data, "\x00\x07\x00\x00\xa4\x00\x00\x02\x4d\x01", 10);
	nRetChild = mass_storage_io(hUSB, (uint8 *)cb, 10, 0, MSC_DIR_H2D, (uint8 *)data, 10, (uint8 *)cb, sizeof(cb));
	if (nRetChild != 0)
		return -5;
	memset(cb, 0, sizeof(cb));
	memcpy(cb, "\xf1\x00\x00\x00\x00\x00\x01\xfc\x08\xff", 10);
	nRetChild = mass_storage_io(hUSB, (uint8 *)cb, 10, 0, MSC_DIR_D2H, NULL, 0, (uint8 *)cb, sizeof(cb));
	if (nRetChild < 0)
		return -6;
	//printf_array(cb, 200);


	//_WriteLog(LL_FATAL,"第9次交互\n");
	memset(cb, 0, sizeof(cb));
	memset(data, 0, sizeof(data));
	memcpy(cb, "\xf1\x00\x00\x00\x00\x00\x02\x09\x00\xff", 10);
	memcpy(data, "\x00\x06\x00\x00\xb0\x00\x00\x00\x20", 9);
	nRetChild = mass_storage_io(hUSB, (uint8 *)cb, 10, 0, MSC_DIR_H2D, (uint8 *)data, 9, (uint8 *)cb, sizeof(cb));
	if (nRetChild != 0)
		return -5;
	memset(cb, 0, sizeof(cb));
	memcpy(cb, "\xf1\x00\x00\x00\x00\x00\x01\xfc\x08\xff", 10);
	nRetChild = mass_storage_io(hUSB, (uint8 *)cb, 10, 0, MSC_DIR_D2H, NULL, 0, (uint8 *)cb, sizeof(cb));
	if (nRetChild < 0)
		return -6;
	//printf_array(cb, 200);





	//_WriteLog(LL_FATAL,"第10次交互\n");
	memset(cb, 0, sizeof(cb));
	memset(data, 0, sizeof(data));
	memcpy(cb, "\xf1\x00\x00\x00\x00\x00\x02\x0a\x00\xff", 10);
	memcpy(data, "\x00\x07\x00\x00\xa4\x00\x00\x02\x4f\x01", 10);
	nRetChild = mass_storage_io(hUSB, (uint8 *)cb, 10, 0, MSC_DIR_H2D, (uint8 *)data, 10, (uint8 *)cb, sizeof(cb));
	if (nRetChild != 0)
		return -5;
	memset(cb, 0, sizeof(cb));
	memcpy(cb, "\xf1\x00\x00\x00\x00\x00\x01\xfc\x08\xff", 10);
	nRetChild = mass_storage_io(hUSB, (uint8 *)cb, 10, 0, MSC_DIR_D2H, NULL, 0, (uint8 *)cb, sizeof(cb));
	if (nRetChild < 0)
		return -6;
	//_WriteLog(LL_FATAL,"返回数据长度为%d,数据为：", nRetChild);
	//printf_array(cb, nRetChild);


	//_WriteLog(LL_FATAL,"第11次交互\n");
	memset(cb, 0, sizeof(cb));
	memset(data, 0, sizeof(data));
	memcpy(cb, "\xf1\x00\x00\x00\x00\x00\x02\x0a\x00\xff", 10);
	memcpy(data, "\x00\x07\x00\x00\xa4\x00\x00\x02\x4e\x01", 10);
	nRetChild = mass_storage_io(hUSB, (uint8 *)cb, 10, 0, MSC_DIR_H2D, (uint8 *)data, 10, (uint8 *)cb, sizeof(cb));
	if (nRetChild != 0)
		return -5;
	memset(cb, 0, sizeof(cb));
	memcpy(cb, "\xf1\x00\x00\x00\x00\x00\x01\xfc\x08\xff", 10);
	nRetChild = mass_storage_io(hUSB, (uint8 *)cb, 10, 0, MSC_DIR_D2H, NULL, 0, (uint8 *)cb, sizeof(cb));
	if (nRetChild < 0)
		return -6;
	//_WriteLog(LL_FATAL,"返回数据长度为%d,数据为：", nRetChild);
	//printf_array(cb, nRetChild);

	//_WriteLog(LL_FATAL,"第12次交互\n");
	memset(cb, 0, sizeof(cb));
	memset(data, 0, sizeof(data));
	memcpy(cb, "\xf1\x00\x00\x00\x00\x00\x02\x09\x00\xff", 10);
	memcpy(data, "\x00\x06\x00\x00\xb0\x00\x00\x00\x01", 9);
	nRetChild = mass_storage_io(hUSB, (uint8 *)cb, 10, 0, MSC_DIR_H2D, (uint8 *)data, 9, (uint8 *)cb, sizeof(cb));
	if (nRetChild != 0)
		return -5;
	memset(cb, 0, sizeof(cb));
	memcpy(cb, "\xf1\x00\x00\x00\x00\x00\x01\xfc\x08\xff", 10);
	nRetChild = mass_storage_io(hUSB, (uint8 *)cb, 10, 0, MSC_DIR_D2H, NULL, 0, (uint8 *)cb, sizeof(cb));
	if (nRetChild < 0)
		return -6;
	//_WriteLog(LL_FATAL,"返回数据长度为%d,数据为：", nRetChild);
	//printf_array(cb, nRetChild);

	//_WriteLog(LL_FATAL,"第13次交互\n");
	memset(cb, 0, sizeof(cb));
	memset(data, 0, sizeof(data));
	memcpy(cb, "\xf1\x00\x00\x00\x00\x00\x02\x09\x00\xff", 10);
	memcpy(data, "\x00\x06\x00\x00\xb0\x00\x00\x00\x40", 9);
	nRetChild = mass_storage_io(hUSB, (uint8 *)cb, 10, 0, MSC_DIR_H2D, (uint8 *)data, 9, (uint8 *)cb, sizeof(cb));
	if (nRetChild != 0)
		return -5;
	memset(cb, 0, sizeof(cb));
	memcpy(cb, "\xf1\x00\x00\x00\x00\x00\x01\xfc\x08\xff", 10);
	nRetChild = mass_storage_io(hUSB, (uint8 *)cb, 10, 0, MSC_DIR_D2H, NULL, 0, (uint8 *)cb, sizeof(cb));
	if (nRetChild < 0)
		return -6;
	//_WriteLog(LL_FATAL,"返回数据长度为%d,数据为：", nRetChild);
	//printf_array(cb, nRetChild);


	//_WriteLog(LL_FATAL,"第14次交互\n");
	memset(cb, 0, sizeof(cb));
	memset(data, 0, sizeof(data));
	memcpy(cb, "\xf1\x00\x00\x00\x00\x00\x02\x0a\x00\xff", 10);
	memcpy(data, "\x00\x07\x00\x00\xa4\x00\x00\x02\x4e\x02", 10);
	nRetChild = mass_storage_io(hUSB, (uint8 *)cb, 10, 0, MSC_DIR_H2D, (uint8 *)data, 10, (uint8 *)cb, sizeof(cb));
	if (nRetChild != 0)
		return -5;
	memset(cb, 0, sizeof(cb));
	memcpy(cb, "\xf1\x00\x00\x00\x00\x00\x01\xfc\x08\xff", 10);
	nRetChild = mass_storage_io(hUSB, (uint8 *)cb, 10, 0, MSC_DIR_D2H, NULL, 0, (uint8 *)cb, sizeof(cb));
	if (nRetChild < 0)
		return -6;
	//_WriteLog(LL_FATAL,"返回数据长度为%d,数据为：", nRetChild);
	//printf_array(cb, nRetChild);


	//_WriteLog(LL_FATAL,"第15次交互\n");
	memset(cb, 0, sizeof(cb));
	memset(data, 0, sizeof(data));
	memcpy(cb, "\xf1\x00\x00\x00\x00\x00\x02\x09\x00\xff", 10);
	memcpy(data, "\x00\x06\x00\x00\xb0\x00\x00\x04\x80", 9);
	nRetChild = mass_storage_io(hUSB, (uint8 *)cb, 10, 0, MSC_DIR_H2D, (uint8 *)data, 9, (uint8 *)cb, sizeof(cb));
	if (nRetChild != 0)
		return -5;
	memset(cb, 0, sizeof(cb));
	memcpy(cb, "\xf1\x00\x00\x00\x00\x00\x01\xfc\x08\xff", 10);
	nRetChild = mass_storage_io(hUSB, (uint8 *)cb, 10, 0, MSC_DIR_D2H, NULL, 0, (uint8 *)cb, sizeof(cb));
	if (nRetChild < 0)
		return -6;
	//_WriteLog(LL_FATAL,"返回数据长度为%d,数据为：", nRetChild);
	//printf_array(cb, nRetChild);

	//_WriteLog(LL_FATAL,"第16次交互\n");
	memset(cb, 0, sizeof(cb));
	memset(data, 0, sizeof(data));
	memcpy(cb, "\xf1\x00\x00\x00\x00\x00\x02\x0a\x00\xff", 10);
	memcpy(data, "\x00\x07\x00\x00\xa4\x00\x00\x02\x4f\x01", 10);
	nRetChild = mass_storage_io(hUSB, (uint8 *)cb, 10, 0, MSC_DIR_H2D, (uint8 *)data, 10, (uint8 *)cb, sizeof(cb));
	if (nRetChild != 0)
		return -5;
	memset(cb, 0, sizeof(cb));
	memcpy(cb, "\xf1\x00\x00\x00\x00\x00\x01\xfc\x08\xff", 10);
	nRetChild = mass_storage_io(hUSB, (uint8 *)cb, 10, 0, MSC_DIR_D2H, NULL, 0, (uint8 *)cb, sizeof(cb));
	if (nRetChild < 0)
		return -6;
	//_WriteLog(LL_FATAL,"返回数据长度为%d,数据为：", nRetChild);
	//printf_array(cb, nRetChild);

	//_WriteLog(LL_FATAL,"第17次交互\n");
	memset(cb, 0, sizeof(cb));
	memset(data, 0, sizeof(data));
	memcpy(cb, "\xf1\x00\x00\x00\x00\x00\x02\x0a\x00\xff", 10);
	memcpy(data, "\x00\x07\x00\x00\xa4\x00\x00\x02\x50\x01", 10);
	nRetChild = mass_storage_io(hUSB, (uint8 *)cb, 10, 0, MSC_DIR_H2D, (uint8 *)data, 10, (uint8 *)cb, sizeof(cb));
	if (nRetChild != 0)
		return -5;
	memset(cb, 0, sizeof(cb));
	memcpy(cb, "\xf1\x00\x00\x00\x00\x00\x01\xfc\x08\xff", 10);
	nRetChild = mass_storage_io(hUSB, (uint8 *)cb, 10, 0, MSC_DIR_D2H, NULL, 0, (uint8 *)cb, sizeof(cb));
	if (nRetChild < 0)
		return -6;
	//_WriteLog(LL_FATAL,"返回数据长度为%d,数据为：", nRetChild);
	//printf_array(cb, nRetChild);

	//_WriteLog(LL_FATAL,"第18次交互\n");
	memset(cb, 0, sizeof(cb));
	memset(data, 0, sizeof(data));
	memcpy(cb, "\xf1\x00\x00\x00\x00\x00\x02\x09\x00\xff", 10);
	memcpy(data, "\x00\x06\x00\x00\xb0\x00\x00\x00\x92", 9);
	nRetChild = mass_storage_io(hUSB, (uint8 *)cb, 10, 0, MSC_DIR_H2D, (uint8 *)data, 9, (uint8 *)cb, sizeof(cb));
	if (nRetChild != 0)
		return -5;
	memset(cb, 0, sizeof(cb));
	memcpy(cb, "\xf1\x00\x00\x00\x00\x00\x01\xfc\x08\xff", 10);
	nRetChild = mass_storage_io(hUSB, (uint8 *)cb, 10, 0, MSC_DIR_D2H, NULL, 0, (uint8 *)cb, sizeof(cb));
	if (nRetChild < 0)
		return -6;
	//_WriteLog(LL_FATAL,"返回数据长度为%d,数据为：", nRetChild);
	//printf_array(cb, nRetChild);

	//_WriteLog(LL_FATAL,"第19次交互\n");
	memset(cb, 0, sizeof(cb));
	memset(data, 0, sizeof(data));
	memcpy(cb, "\xf1\x00\x00\x00\x00\x00\x02\x0a\x00\xff", 10);
	memcpy(data, "\x00\x07\x00\x00\xa4\x00\x00\x02\x4f\x01", 10);
	nRetChild = mass_storage_io(hUSB, (uint8 *)cb, 10, 0, MSC_DIR_H2D, (uint8 *)data, 10, (uint8 *)cb, sizeof(cb));
	if (nRetChild != 0)
		return -5;
	memset(cb, 0, sizeof(cb));
	memcpy(cb, "\xf1\x00\x00\x00\x00\x00\x01\xfc\x08\xff", 10);
	nRetChild = mass_storage_io(hUSB, (uint8 *)cb, 10, 0, MSC_DIR_D2H, NULL, 0, (uint8 *)cb, sizeof(cb));
	if (nRetChild < 0)
		return -6;
	//_WriteLog(LL_FATAL,"返回数据长度为%d,数据为：", nRetChild);
	//printf_array(cb, nRetChild);

	//_WriteLog(LL_FATAL,"第20次交互\n");
	memset(cb, 0, sizeof(cb));
	memset(data, 0, sizeof(data));
	memcpy(cb, "\xf1\x00\x00\x00\x00\x00\x02\x0a\x00\xff", 10);
	memcpy(data, "\x00\x07\x00\x00\xa4\x00\x00\x02\x50\x00", 10);
	nRetChild = mass_storage_io(hUSB, (uint8 *)cb, 10, 0, MSC_DIR_H2D, (uint8 *)data, 10, (uint8 *)cb, sizeof(cb));
	if (nRetChild != 0)
		return -5;
	memset(cb, 0, sizeof(cb));
	memcpy(cb, "\xf1\x00\x00\x00\x00\x00\x01\xfc\x08\xff", 10);
	nRetChild = mass_storage_io(hUSB, (uint8 *)cb, 10, 0, MSC_DIR_D2H, NULL, 0, (uint8 *)cb, sizeof(cb));
	if (nRetChild < 0)
		return -6;
	//_WriteLog(LL_FATAL,"返回数据长度为%d,数据为：", nRetChild);
	//printf_array(cb, nRetChild);

	//_WriteLog(LL_FATAL,"第21次交互\n");
	memset(cb, 0, sizeof(cb));
	memset(data, 0, sizeof(data));
	memcpy(cb, "\xf1\x00\x00\x00\x00\x00\x02\x09\x00\xff", 10);
	memcpy(data, "\x00\x06\x00\x00\xb0\x00\x00\x00\xb4", 9);
	nRetChild = mass_storage_io(hUSB, (uint8 *)cb, 10, 0, MSC_DIR_H2D, (uint8 *)data, 9, (uint8 *)cb, sizeof(cb));
	if (nRetChild != 0)
		return -5;
	memset(cb, 0, sizeof(cb));
	memcpy(cb, "\xf1\x00\x00\x00\x00\x00\x01\xfc\x08\xff", 10);
	nRetChild = mass_storage_io(hUSB, (uint8 *)cb, 10, 0, MSC_DIR_D2H, NULL, 0, (uint8 *)cb, sizeof(cb));
	if (nRetChild < 0)
		return -6;
	//_WriteLog(LL_FATAL,"返回数据长度为%d,数据为：", nRetChild);
	//printf_array(cb, nRetChild);


	//_WriteLog(LL_FATAL,"第22次交互\n");
	memset(cb, 0, sizeof(cb));
	memset(data, 0, sizeof(data));
	memcpy(cb, "\xf1\x00\x00\x00\x00\x00\x02\x0a\x00\xff", 10);
	memcpy(data, "\x00\x07\x00\x00\xa4\x00\x00\x02\x4f\x01", 10);
	nRetChild = mass_storage_io(hUSB, (uint8 *)cb, 10, 0, MSC_DIR_H2D, (uint8 *)data, 10, (uint8 *)cb, sizeof(cb));
	if (nRetChild != 0)
		return -5;
	memset(cb, 0, sizeof(cb));
	memcpy(cb, "\xf1\x00\x00\x00\x00\x00\x01\xfc\x08\xff", 10);
	nRetChild = mass_storage_io(hUSB, (uint8 *)cb, 10, 0, MSC_DIR_D2H, NULL, 0, (uint8 *)cb, sizeof(cb));
	if (nRetChild < 0)
		return -6;
	//_WriteLog(LL_FATAL,"返回数据长度为%d,数据为：", nRetChild);
	//printf_array(cb, nRetChild);


	//_WriteLog(LL_FATAL,"第23次交互\n");
	memset(cb, 0, sizeof(cb));
	memset(data, 0, sizeof(data));
	memcpy(cb, "\xf1\x00\x00\x00\x00\x00\x02\x0a\x00\xff", 10);
	memcpy(data, "\x00\x07\x00\x00\xa4\x00\x00\x02\x50\x04", 10);
	nRetChild = mass_storage_io(hUSB, (uint8 *)cb, 10, 0, MSC_DIR_H2D, (uint8 *)data, 10, (uint8 *)cb, sizeof(cb));
	if (nRetChild != 0)
		return -5;
	memset(cb, 0, sizeof(cb));
	memcpy(cb, "\xf1\x00\x00\x00\x00\x00\x01\xfc\x08\xff", 10);
	nRetChild = mass_storage_io(hUSB, (uint8 *)cb, 10, 0, MSC_DIR_D2H, NULL, 0, (uint8 *)cb, sizeof(cb));
	if (nRetChild < 0)
		return -6;
	//_WriteLog(LL_FATAL,"返回数据长度为%d,数据为：", nRetChild);
	//printf_array(cb, nRetChild);


	//_WriteLog(LL_FATAL,"第24次交互\n");
	memset(cb, 0, sizeof(cb));
	memset(data, 0, sizeof(data));
	memcpy(cb, "\xf1\x00\x00\x00\x00\x00\x02\x09\x00\xff", 10);
	memcpy(data, "\x00\x06\x00\x00\xb0\x00\x00\x04\xef", 9);
	nRetChild = mass_storage_io(hUSB, (uint8 *)cb, 10, 0, MSC_DIR_H2D, (uint8 *)data, 9, (uint8 *)cb, sizeof(cb));
	if (nRetChild != 0)
		return -5;
	memset(cb, 0, sizeof(cb));
	memcpy(cb, "\xf1\x00\x00\x00\x00\x00\x01\xfc\x08\xff", 10);
	nRetChild = mass_storage_io(hUSB, (uint8 *)cb, 10, 0, MSC_DIR_D2H, NULL, 0, (uint8 *)cb, sizeof(cb));
	if (nRetChild < 0)
		return -6;
	/*_WriteLog(LL_FATAL,"返回数据长度为%d,数据为：", nRetChild);
	printf_array(cb, nRetChild);*/

	//_WriteLog(LL_FATAL,"第25次交互\n");
	memset(cb, 0, sizeof(cb));
	memset(data, 0, sizeof(data));
	memcpy(cb, "\xf1\x00\x00\x00\x00\x00\x02\x0a\x00\xff", 10);
	memcpy(data, "\x00\x07\x00\x00\xa4\x00\x00\x02\x4f\x01", 10);
	nRetChild = mass_storage_io(hUSB, (uint8 *)cb, 10, 0, MSC_DIR_H2D, (uint8 *)data, 10, (uint8 *)cb, sizeof(cb));
	if (nRetChild != 0)
		return -5;
	memset(cb, 0, sizeof(cb));
	memcpy(cb, "\xf1\x00\x00\x00\x00\x00\x01\xfc\x08\xff", 10);
	nRetChild = mass_storage_io(hUSB, (uint8 *)cb, 10, 0, MSC_DIR_D2H, NULL, 0, (uint8 *)cb, sizeof(cb));
	if (nRetChild < 0)
		return -6;
	/*_WriteLog(LL_FATAL,"返回数据长度为%d,数据为：", nRetChild);
	printf_array(cb, nRetChild);*/



	//_WriteLog(LL_FATAL,"第26次交互\n");
	memset(cb, 0, sizeof(cb));
	memset(data, 0, sizeof(data));
	memcpy(cb, "\xf1\x00\x00\x00\x00\x00\x02\x0a\x00\xff", 10);
	memcpy(data, "\x00\x07\x00\x00\xa4\x00\x00\x02\x50\x08", 10);
	nRetChild = mass_storage_io(hUSB, (uint8 *)cb, 10, 0, MSC_DIR_H2D, (uint8 *)data, 10, (uint8 *)cb, sizeof(cb));
	if (nRetChild != 0)
		return -5;
	memset(cb, 0, sizeof(cb));
	memcpy(cb, "\xf1\x00\x00\x00\x00\x00\x01\xfc\x08\xff", 10);
	nRetChild = mass_storage_io(hUSB, (uint8 *)cb, 10, 0, MSC_DIR_D2H, NULL, 0, (uint8 *)cb, sizeof(cb));
	if (nRetChild < 0)
		return -6;
	//_WriteLog(LL_FATAL,"返回数据长度为%d,数据为：", nRetChild);
	//printf_array(cb, nRetChild);


	//_WriteLog(LL_FATAL,"第27次交互\n");
	memset(cb, 0, sizeof(cb));
	memset(data, 0, sizeof(data));
	memcpy(cb, "\xf1\x00\x00\x00\x00\x00\x02\x0a\x00\xff", 10);
	memcpy(data, "\x00\x07\x00\x00\xa4\x00\x00\x02\x50\x08", 10);
	nRetChild = mass_storage_io(hUSB, (uint8 *)cb, 10, 0, MSC_DIR_H2D, (uint8 *)data, 10, (uint8 *)cb, sizeof(cb));
	if (nRetChild != 0)
		return -5;
	memset(cb, 0, sizeof(cb));
	memcpy(cb, "\xf1\x00\x00\x00\x00\x00\x01\xfc\x08\xff", 10);
	nRetChild = mass_storage_io(hUSB, (uint8 *)cb, 10, 0, MSC_DIR_D2H, NULL, 0, (uint8 *)cb, sizeof(cb));
	if (nRetChild < 0)
		return -6;
	//_WriteLog(LL_FATAL,"返回数据长度为%d,数据为：", nRetChild);
	//printf_array(cb, nRetChild);


	//_WriteLog(LL_FATAL,"第28次交互\n");
	memset(cb, 0, sizeof(cb));
	memset(data, 0, sizeof(data));
	memcpy(cb, "\xf1\x00\x00\x00\x00\x00\x02\x09\x00\xff", 10);
	memcpy(data, "\x00\x06\x00\x00\xb0\x00\x00\x00\x92", 9);
	nRetChild = mass_storage_io(hUSB, (uint8 *)cb, 10, 0, MSC_DIR_H2D, (uint8 *)data, 9, (uint8 *)cb, sizeof(cb));
	if (nRetChild != 0)
		return -5;
	memset(cb, 0, sizeof(cb));
	memcpy(cb, "\xf1\x00\x00\x00\x00\x00\x01\xfc\x08\xff", 10);
	nRetChild = mass_storage_io(hUSB, (uint8 *)cb, 10, 0, MSC_DIR_D2H, NULL, 0, (uint8 *)cb, sizeof(cb));
	if (nRetChild < 0)
		return -6;
	/*_WriteLog(LL_FATAL,"返回数据长度为%d,数据为：", nRetChild);
	printf_array(cb, nRetChild);*/

	//_WriteLog(LL_FATAL,"第29次交互\n");
	memset(cb, 0, sizeof(cb));
	memset(data, 0, sizeof(data));
	memcpy(cb, "\xf1\x00\x00\x00\x00\x00\x02\x0a\x00\xff", 10);
	memcpy(data, "\x00\x07\x00\x00\xa4\x00\x00\x02\x4f\x01", 10);
	nRetChild = mass_storage_io(hUSB, (uint8 *)cb, 10, 0, MSC_DIR_H2D, (uint8 *)data, 10, (uint8 *)cb, sizeof(cb));
	if (nRetChild != 0)
		return -5;
	memset(cb, 0, sizeof(cb));
	memcpy(cb, "\xf1\x00\x00\x00\x00\x00\x01\xfc\x08\xff", 10);
	nRetChild = mass_storage_io(hUSB, (uint8 *)cb, 10, 0, MSC_DIR_D2H, NULL, 0, (uint8 *)cb, sizeof(cb));
	if (nRetChild < 0)
		return -6;
	/*_WriteLog(LL_FATAL,"返回数据长度为%d,数据为：", nRetChild);
	printf_array(cb, nRetChild);*/

	//_WriteLog(LL_FATAL,"第30次交互\n");
	memset(cb, 0, sizeof(cb));
	memset(data, 0, sizeof(data));
	memcpy(cb, "\xf1\x00\x00\x00\x00\x00\x02\x0a\x00\xff", 10);
	memcpy(data, "\x00\x07\x00\x00\xa4\x00\x00\x02\x50\x07", 10);
	nRetChild = mass_storage_io(hUSB, (uint8 *)cb, 10, 0, MSC_DIR_H2D, (uint8 *)data, 10, (uint8 *)cb, sizeof(cb));
	if (nRetChild != 0)
		return -5;
	memset(cb, 0, sizeof(cb));
	memcpy(cb, "\xf1\x00\x00\x00\x00\x00\x01\xfc\x08\xff", 10);
	nRetChild = mass_storage_io(hUSB, (uint8 *)cb, 10, 0, MSC_DIR_D2H, NULL, 0, (uint8 *)cb, sizeof(cb));
	if (nRetChild < 0)
		return -6;
	//_WriteLog(LL_FATAL,"返回数据长度为%d,数据为：", nRetChild);
	//printf_array(cb, nRetChild);

	//_WriteLog(LL_FATAL,"第31次交互\n");
	memset(cb, 0, sizeof(cb));
	memset(data, 0, sizeof(data));
	memcpy(cb, "\xf1\x00\x00\x00\x00\x00\x02\x09\x00\xff", 10);
	memcpy(data, "\x00\x06\x00\x00\xb0\x00\x00\x00\x9c", 9);
	nRetChild = mass_storage_io(hUSB, (uint8 *)cb, 10, 0, MSC_DIR_H2D, (uint8 *)data, 9, (uint8 *)cb, sizeof(cb));
	if (nRetChild != 0)
		return -5;
	memset(cb, 0, sizeof(cb));
	memcpy(cb, "\xf1\x00\x00\x00\x00\x00\x01\xfc\x08\xff", 10);
	nRetChild = mass_storage_io(hUSB, (uint8 *)cb, 10, 0, MSC_DIR_D2H, NULL, 0, (uint8 *)cb, sizeof(cb));
	if (nRetChild < 0)
		return -6;
	//_WriteLog(LL_FATAL,"返回数据长度为%d,数据为：", nRetChild);
	//printf_array(cb, nRetChild);


	//_WriteLog(LL_FATAL,"第32次交互\n");
	memset(cb, 0, sizeof(cb));
	memset(data, 0, sizeof(data));
	memcpy(cb, "\xf1\x00\x00\x00\x00\x00\x02\x0a\x00\xff", 10);
	memcpy(data, "\x00\x07\x00\x00\xa4\x00\x00\x02\x4f\x01", 10);
	nRetChild = mass_storage_io(hUSB, (uint8 *)cb, 10, 0, MSC_DIR_H2D, (uint8 *)data, 10, (uint8 *)cb, sizeof(cb));
	if (nRetChild != 0)
		return -5;
	memset(cb, 0, sizeof(cb));
	memcpy(cb, "\xf1\x00\x00\x00\x00\x00\x01\xfc\x08\xff", 10);
	nRetChild = mass_storage_io(hUSB, (uint8 *)cb, 10, 0, MSC_DIR_D2H, NULL, 0, (uint8 *)cb, sizeof(cb));
	if (nRetChild < 0)
		return -6;
	//_WriteLog(LL_FATAL,"返回数据长度为%d,数据为：", nRetChild);
	//printf_array(cb, nRetChild);


	//_WriteLog(LL_FATAL,"第33次交互\n");
	memset(cb, 0, sizeof(cb));
	memset(data, 0, sizeof(data));
	memcpy(cb, "\xf1\x00\x00\x00\x00\x00\x02\x0a\x00\xff", 10);
	memcpy(data, "\x00\x07\x00\x00\xa4\x00\x00\x02\x50\x09", 10);
	nRetChild = mass_storage_io(hUSB, (uint8 *)cb, 10, 0, MSC_DIR_H2D, (uint8 *)data, 10, (uint8 *)cb, sizeof(cb));
	if (nRetChild != 0)
		return -5;
	memset(cb, 0, sizeof(cb));
	memcpy(cb, "\xf1\x00\x00\x00\x00\x00\x01\xfc\x08\xff", 10);
	nRetChild = mass_storage_io(hUSB, (uint8 *)cb, 10, 0, MSC_DIR_D2H, NULL, 0, (uint8 *)cb, sizeof(cb));
	if (nRetChild < 0)
		return -6;
	//_WriteLog(LL_FATAL,"返回数据长度为%d,数据为：", nRetChild);
	//printf_array(cb, nRetChild);


	//_WriteLog(LL_FATAL,"第34次交互\n");
	memset(cb, 0, sizeof(cb));
	memset(data, 0, sizeof(data));
	memcpy(cb, "\xf1\x00\x00\x00\x00\x00\x02\x09\x00\xff", 10);
	memcpy(data, "\x00\x06\x00\x00\xb0\x00\x00\x06\x20", 9);
	nRetChild = mass_storage_io(hUSB, (uint8 *)cb, 10, 0, MSC_DIR_H2D, (uint8 *)data, 9, (uint8 *)cb, sizeof(cb));
	if (nRetChild != 0)
		return -5;
	memset(cb, 0, sizeof(cb));
	memcpy(cb, "\xf1\x00\x00\x00\x00\x00\x01\xfc\x08\xff", 10);
	nRetChild = mass_storage_io(hUSB, (uint8 *)cb, 10, 0, MSC_DIR_D2H, NULL, 0, (uint8 *)cb, sizeof(cb));
	if (nRetChild < 0)
		return -6;
	//_WriteLog(LL_FATAL,"返回数据长度为%d,数据为：", nRetChild);
	//printf_array(cb, nRetChild);




	unsigned char Flag1[] = { 0x06, 0x03, 0x55, 0x04, 0x03 };
	unsigned char *p = NULL;

	p = SearchBytes(cb, cb + nRetChild, Flag1, sizeof Flag1);
	if (p == NULL)
	{
		_WriteLog(LL_FATAL, "未找到第一个标志位：0x06, 0x03, 0x55, 0x04, 0x03\n");
		return -7;
	}

	p = SearchBytes(p + 1, cb + nRetChild, Flag1, sizeof Flag1);
	if (p == NULL)
	{
		_WriteLog(LL_FATAL, "未找到第二个标志位：0x06, 0x03, 0x55, 0x04, 0x03\n");
		return -8;
	}

	if (*(p + 5) == 0x1e)
	{//unicode

	}
	else if ((*(p + 5) == 0x0c))
	{//utf8
		char name[500] = { 0 };
		unsigned char bLen = (unsigned char)*(p - 1);
		int nExtraLen = sizeof Flag1 + 2;
		unsigned char *pStr = p + nExtraLen;
		memcpy(name, pStr, bLen - nExtraLen);


		EncordingConvert("utf-8", "gbk", name, strlen(name), hDev->szCompanyName, sizeof(hDev->szCompanyName));
		_WriteLog(LL_FATAL, "szCompanyName：%s\n", hDev->szCompanyName);
		return 0;
	}


	//memset(cb, 0, sizeof(cb));
	//memset(data, 0, sizeof(data));
	//memcpy(cb, "\xf1\x00\x00\x00\x00\x00\x02\x09\x00\xff", 10);
	//memcpy(data, "\x00\x06\x00\x00\xb0\x00\x00\x06\x20", 9);
	//nRetChild = mass_storage_io(device, (uint8 *)cb, 10, 0, MSC_DIR_H2D, (uint8 *)data, 9, (uint8 *)cb, sizeof(cb));
	//if (nRetChild != 0)
	//	return -3;
	//memset(cb, 0, sizeof(cb));
	//memcpy(cb, "\xf1\x00\x00\x00\x00\x00\x01\xfc\x08\xff", 10);
	//nRetChild = mass_storage_io(device, (uint8 *)cb, 10, 0, MSC_DIR_D2H, NULL, 0, (uint8 *)cb, sizeof(cb));
	//if (nRetChild < 0)
	//	return -4;
	//printf_array(cb, 200);


	////通用显示税号 税盘公司名称
	//if (AisinoReadTaxIDandCustomName(device, hDev->szCommonTaxID, hDev->szCompanyName,
	//	sizeof(hDev->szCompanyName)) < 0) {
	//	nRet = -7;
	//	goto LoadAisinoDevInfo_Finish;
	//}
	// driver ver
	//if (GetDriverVersion(device, hDev->szDriverVersion) < 0) {
	//	nRet = -8;
	//	goto LoadAisinoDevInfo_Finish;
	//}
	// jmbmh
	//GetJMBBH(device, hDev->szJMBBH);
	// trandisk check
	//hDev->bTrainFPDiskType = GetTrainDeviceType(device, hDev);

	nRet = 0;
LoadAisinoDevInfo_Finish:
	if (nRet) {
		_WriteLog(LL_FATAL, "[-] LoadAisinoSelectDevInfo failed,nRet = %d\n", nRet);
	}
	else {
		;
	}
	return nRet;
}


//=============================================我司自行增加代码=============================================//
//==========================================================================================================//
//==========================================================================================================//
//==========================================================================================================//
//==========================================================================================================//
//==========================================================================================================//
//==========================================================================================================//
//==========================================================================================================//
//==========================================================================================================//
//==========================================================================================================//
//==========================================================================================================//
///////////////////////////////////////////////////////
void find_inv_type_mem(uint8 type, int *mem_i)
{
	if (type == 4)
		*mem_i = 0;
	else if (type == 5)
		*mem_i = 1;
	else if (type == 6)
		*mem_i = 2;
	else if (type == 7)
		*mem_i = 3;
	else if (type == 9)
		*mem_i = 4;
	else if (type == 25)
		*mem_i = 5;
	else if (type == 26)
		*mem_i = 6;
	else if (type == 28)
		*mem_i = 7;
	else
		*mem_i = 8;
}

/*检查字符串中是否有特殊字符(\ 、")
* 如果有就替换*/
void check_special_char(uint8 *str)
{
	int i_len = strlen((const char *)str), i = 0;
	for (i = 0; i < i_len; i++) {
		if (str[i] == '\\')
			str[i] = '/';
		if (str[i] == '"')
			str[i] = '\'';
	}
}

/*检查开票字符串有无特殊字符*/
int check_invoice_str(uint8 *str)
{
	int i_len = strlen((const char *)str), i = 0;
	for (i = 0; i < i_len; i++) {
		//if (str[i] == '&' || str[i] == '\\')
		if (str[i] == '\\')
			return -1;
	}
	return 0;
}
#ifdef MIPS_MTK
int GetDnsIp(char *host, char *ip)
{
	char   buf[100];
	struct addrinfo hints;
	struct addrinfo *res, *curr;
	struct sockaddr_in *sa;
	int ret;
	/* obtaining address matching host */
	if (host == NULL)
	{
		//_WriteLog(LL_INFO,"要解析的域名错误\n");
		return -1;
	}
	//_WriteLog(LL_INFO,"get_DNS_ip 11\n");
	if (memcmp(host, "127.0.0.1", strlen((const char*)host)) == 0)
	{
		//_WriteLog(LL_INFO,"本机IP不用解析\n");
		sprintf((char *)ip, "127.0.0.1");
		return strlen((const char*)ip);
	}
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_INET;

	hints.ai_flags = AI_CANONNAME;
	//_WriteLog(LL_INFO,"get_DNS_ip 22\n");
	ret = getaddrinfo(host, NULL, &hints, &res);
	if (ret != 0)
	{

		freeaddrinfo(res);
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(ret));
		return -1;
	}
	//_WriteLog(LL_INFO,"get_DNS_ip 33\n");
	curr = res;
	/* traverse the returned list and output the ip addresses */
	while (curr && curr->ai_canonname)
	{
		sa = (struct sockaddr_in *)curr->ai_addr;
		//printf("name: %s\nip:%s\n\n", curr->ai_canonname,inet_ntop(AF_INET, &sa->sin_addr.s_addr, buf, sizeof (buf)));  
		sprintf(ip, "%s", inet_ntop(AF_INET, &sa->sin_addr.s_addr, buf, sizeof (buf)));
		curr = curr->ai_next;
	}
	//_WriteLog(LL_INFO,"get_DNS_ip 44\n");
	freeaddrinfo(res);
	//_WriteLog(LL_INFO,"get_DNS_ip 55\n");
	return strlen(ip);
}
#else
int GetDnsIp(char *hostname, char *hostip)
{
	struct hostent *h;
	struct in_addr in;
	struct sockaddr_in addr_in;
	struct sockaddr_in adr_inet;
	int len;
	int ip_len;
	char hostnamebuf[100] = { 0 };
	len = strlen((char *)hostname);
	memcpy(hostnamebuf, hostname, len);

	memset(&adr_inet, 0, sizeof(adr_inet));
	if (inet_aton(hostnamebuf, &adr_inet.sin_addr))//判断是否是IP
	{
		//_WriteLog(LL_INFO,"is ip\n");
		//_WriteLog(LL_INFO,"hostnamebuf222 %s\n", hostnamebuf);
		memcpy(hostip, hostnamebuf, len);
		return len;
	}
	if (len<5)
	{
		//_WriteLog(LL_INFO,"域名长度错误\n");
		return -1;
	}
	//_WriteLog(LL_INFO,"hostnamebuf %s\n", hostnamebuf);
	if (memcmp(hostnamebuf, "127.0.0.1", len) == 0)
	{
		//_WriteLog(LL_INFO,"本机IP不用解析\n");
		sprintf((char *)hostip, "127.0.0.1");
		return strlen((const char*)hostip);
	}
	h = gethostbyname((const char *)hostnamebuf);
	if (h == NULL)
	{
		//_WriteLog(LL_INFO,"解析IP地址失败\n");
		return -1;
	}
	else
	{
		memcpy(&addr_in.sin_addr.s_addr, h->h_addr, 4);
		in.s_addr = addr_in.sin_addr.s_addr;
	}
	ip_len = strlen(inet_ntoa(in));
	memcpy(hostip, inet_ntoa(in), ip_len);

	return ip_len;
}
#endif


int mb_get_today_month(char *today_month)
{
	timer_read_y_m_d(today_month);
	today_month[6] = 0x00;
	today_month[7] = 0x00;//剔除时间到年月
	return 0;
}

int mb_get_next_month(char *now_month)
{
	next_month_asc(now_month);
	now_month[6] = 0x00;
	now_month[7] = 0x00;//剔除时间到年月
	return 0;
}

int mb_get_last_month_first_day(char *now_month, char *last_month_fistday)
{
	char year_s[10] = {0};
	char month_s[10] = { 0 };
	int year = 0;
	int month = 0;
	memcpy(year_s, now_month,4);
	memcpy(month_s, now_month+4, 2);
	year = atoi(year_s);
	month = atoi(month_s);
	if (month == 1)
	{
		month = 12;
		year -= 1;
	}
	else
	{
		month -= 1;
	}
	sprintf(last_month_fistday, "%04d%02d01", year, month);
	return 0;
}
#ifdef DF_OLD_MSERVER
int jude_need_upload_inv(struct _upload_inv_num *upload_inv_num, HFPXX fpxx)
{
	int mem_i = -1;
	int need_upload = 0;

	switch (fpxx->fplx)
	{
	case FPLX_COMMON_ZYFP:
		mem_i = 0;
		break;
	case FPLX_COMMON_PTFP:
		mem_i = 1;
		break;
	case FPLX_COMMON_DZFP:
		mem_i = 2;
		break;
	case FPLX_COMMON_DZZP:
		mem_i = 3;
		break;
	default:
		//_WriteLog(LL_INFO, "jude_need_upload_inv 不支持的发票种类：%03d\n", fpxx->fplx);
		need_upload = 0;
		goto END;
	}
	if (upload_inv_num->upload_inv_type[mem_i].state != 1)
	{
		need_upload = 1;
		goto END;
	}

	if (strcmp(fpxx->kpsj_F2, upload_inv_num->upload_inv_type[mem_i].zhkpsj) < 0)
	{
		if (fpxx->zfbz != 1)
		{
			need_upload = 0;
			goto END;
		}
		if (strcmp(fpxx->zfsj_F2, upload_inv_num->upload_inv_type[mem_i].zhzfsj) < 0)
		{
			need_upload = 0;
			goto END;
		}
		else if (strcmp(fpxx->zfsj_F2, upload_inv_num->upload_inv_type[mem_i].zhzfsj) == 0)
		{
			if ((strcmp(fpxx->fpdm, upload_inv_num->upload_inv_type[mem_i].zhzfdm) == 0) && (strcmp(fpxx->fphm, upload_inv_num->upload_inv_type[mem_i].zhzfhm) == 0))
			{
				need_upload = 0;
				goto END;
			}
			//_WriteLog(LL_INFO,"该张发票作废时间等于服务器端记录的最后作废时间,但发票代码号码不一致需要重传,发票代码：%s，发票号码：%s\n", fpxx->fpdm, fpxx->fphm);
			need_upload = 1;
			goto END;
		}
		else
		{
			//_WriteLog(LL_INFO, "该张发票作废时间【%s】大于服务器端记录的最后作废时间【%s】,需要重传,发票代码：%s，发票号码：%s\n", fpxx->zfsj_F2, upload_inv_num->upload_inv_type[mem_i].zhzfhm, fpxx->fpdm, fpxx->fphm);
			need_upload = 1;
			goto END;
		}
	}
	else if (strcmp(fpxx->kpsj_F2, upload_inv_num->upload_inv_type[mem_i].zhkpsj) == 0)
	{
		//_WriteLog(LL_INFO, "开票时间【%s】等于最后开票时间【%s】\n", fpxx->kpsj_F2, upload_inv_num->upload_inv_type[mem_i].zhkpsj);
		if ((strcmp(fpxx->fpdm, upload_inv_num->upload_inv_type[mem_i].zhkpdm) == 0) && (strcmp(fpxx->fphm, upload_inv_num->upload_inv_type[mem_i].zhkphm) == 0))
		{
			//_WriteLog(LL_INFO, "发票代码号码【%s-%s】等于最后发票代码号码【%s-%s】\n", fpxx->fpdm, fpxx->fphm, upload_inv_num->upload_inv_type[mem_i].zhkpdm, upload_inv_num->upload_inv_type[mem_i].zhkphm);
			if (fpxx->zfbz != 1)
			{
				need_upload = 0;
				goto END;
			}
			if (strcmp(fpxx->zfsj_F2, upload_inv_num->upload_inv_type[mem_i].zhzfsj) < 0)
			{
				need_upload = 0;
				goto END;
			}
			else if (strcmp(fpxx->zfsj_F2, upload_inv_num->upload_inv_type[mem_i].zhzfsj) == 0)
			{
				if ((strcmp(fpxx->fpdm, upload_inv_num->upload_inv_type[mem_i].zhzfdm) == 0) && (strcmp(fpxx->fphm, upload_inv_num->upload_inv_type[mem_i].zhzfhm) == 0))
				{
					need_upload = 0;
					goto END;
				}
				//_WriteLog(LL_INFO,"该张发票与最后记录开票时间一致,且代码号码一致,作废时间也等于服务器端记录的最后作废时间,但发票代码号码不一致需要重传,发票代码：%s，发票号码：%s\n", fpxx->fpdm, fpxx->fphm);
				need_upload = 1;
				goto END;
			}
			else
			{
				//_WriteLog(LL_INFO,"该张发票与最后记录开票时间一致,且代码号码一致,作废时间大于服务器端记录的最后作废时间,需要重传,发票代码：%s，发票号码：%s\n", fpxx->fpdm, fpxx->fphm);
				need_upload = 1;
				goto END;
			}
		}
		//_WriteLog(LL_INFO,"该张发票开票时间与服务器时间一致,但发票代码号码不一致,需要重传,发票代码：%s，发票号码：%s\n", fpxx->fpdm, fpxx->fphm);
		need_upload = 1;
		goto END;
	}
	else
	{
		//_WriteLog(LL_INFO, "该张发票开时间【%s】大于最后开票最后开票时间【%s】需上传此张发票,发票代码：%s，发票号码：%s\n", fpxx->kpsj_F2, upload_inv_num->upload_inv_type[mem_i].zhkpsj, fpxx->fpdm, fpxx->fphm);
		need_upload = 1;
		goto END;
	}

	need_upload = 1;
END:
	return need_upload;
}

int add_inv_sum_data(HFPXX fpxx, struct _inv_sum_data *inv_sum)
{
	int mem_i = -1;
	int ret = 0;

	switch (fpxx->fplx)
	{
	case FPLX_COMMON_ZYFP:
		mem_i = 0;
		break;
	case FPLX_COMMON_PTFP:
		mem_i = 1;
		break;
	case FPLX_COMMON_DZFP:
		mem_i = 2;
		break;
	case FPLX_COMMON_DZZP:
		mem_i = 3;
		break;
	default:
		//_WriteLog(LL_INFO, "jude_need_upload_inv 不支持的发票种类：%03d\n", fpxx->fplx);
		ret = -1;
		goto END;
	}
	if (fpxx->zfbz == 1)
	{
		ret = 0;//废票不纳入计算
		goto END;
	}
	inv_sum->type_sum[mem_i].sjxsje += strtold(fpxx->je, NULL);
	inv_sum->type_sum[mem_i].sjxsse += strtold(fpxx->se, NULL);
	ret = 0;
END:
	return ret;
}
#else
int add_inv_sum_data(HFPXX fpxx, struct _inv_sum_data *inv_sum)
{
	int mem_i = -1;
	int ret = 0;

	switch (fpxx->fplx)
	{
	case FPLX_COMMON_ZYFP:
		mem_i = 0;
		break;
	case FPLX_COMMON_PTFP:
		mem_i = 1;
		break;
	case FPLX_COMMON_DZFP:
		mem_i = 2;
		break;
	case FPLX_COMMON_DZZP:
		mem_i = 3;
		break;
	default:
		//_WriteLog(LL_INFO, "jude_need_upload_inv 不支持的发票种类：%03d\n", fpxx->fplx);
		ret = -1;
		goto END;
	}
	long double je;
	long double se;

	je = strtold(fpxx->je, NULL);
	se = strtold(fpxx->se, NULL);

	if (fpxx->zfbz == 1)
	{

		if (je > 0)
		{
			inv_sum->type_sum[mem_i].zffpfs += 1;
			inv_sum->type_sum[mem_i].zffpljje += je;
			inv_sum->type_sum[mem_i].zffpljse += se;
		}
		else if (je < 0)
		{
			inv_sum->type_sum[mem_i].fffpfs += 1;
			inv_sum->type_sum[mem_i].fffpljje += je;
			inv_sum->type_sum[mem_i].fffpljse += se;
		}
		else
		{
			inv_sum->type_sum[mem_i].kffpfs += 1;
		}
	}
	else
	{
		if (je > 0)
		{
			inv_sum->type_sum[mem_i].zsfpfs += 1;
			inv_sum->type_sum[mem_i].zsfpljje += je;
			inv_sum->type_sum[mem_i].zsfpljse += se;
		}
		else if (je < 0)
		{
			inv_sum->type_sum[mem_i].fsfpfs += 1;
			inv_sum->type_sum[mem_i].fsfpljje += je;
			inv_sum->type_sum[mem_i].fsfpljse += se;
		}
	}
	inv_sum->type_sum[mem_i].sjxsje += je;
	inv_sum->type_sum[mem_i].sjxsse += se;
	ret = 0;
END:
	return ret;
}
#endif

int compare_inv_sum_statistics(struct _inv_sum_data *inv_sum, char *statistics)
{
	int size;
	int i;
	uint8 fplxdm;
	char fplxdm_s[10] = {0};
	char data_base[8196] = { 0 };
	char *data_json;
	char *statistics_json;
	char sjxsje[100];
	char sjxsse[100];
	char sjxsje_mx[100] = { 0 };
	char sjxsse_mx[100] = { 0 };
	int mem_i;
	int ret = 0;
	//_WriteLog(LL_INFO, "statistics :%s", statistics);
	cJSON *arrayItem, *item; //解析使用
	if (statistics == NULL)
	{
		return -1;
	}
	statistics_json = malloc(strlen(statistics) + 1024);
	if (statistics_json == NULL)
	{
		return -2;
	}
	Base64_Decode(statistics, strlen(statistics), statistics_json);
	cJSON *root = cJSON_Parse(statistics_json);
	if (!root)
	{
		free(statistics_json);
		return -3;
	}
	free(statistics_json);
	cJSON *object_data = cJSON_GetObjectItem(root, "statistics");
	if (object_data == NULL)
	{
		cJSON_Delete(root);
		return -4;
	}
	size = cJSON_GetArraySize(object_data);
	if (size == 0)
	{
		cJSON_Delete(root);
		return -5;
	}

	//out("按要求的税盘编号查询证书密码\n");
	for (i = 0; i < size; i++)
	{
		//int time_now;
		arrayItem = cJSON_GetArrayItem(object_data, i);
		if (arrayItem == NULL)
		{
			cJSON_Delete(root);
			return -6;
		}
		item = cJSON_GetObjectItem(arrayItem, "fplxdm");
		memset(fplxdm_s, 0, sizeof(fplxdm_s));
		if (get_json_value_can_not_null(item, fplxdm_s, 1, 5) < 0)
		{
			cJSON_Delete(root);
			return -7;
		}
		fplxdm = atoi(fplxdm_s);



		switch (fplxdm)
		{
		case FPLX_COMMON_ZYFP:
			mem_i = 0;
			break;
		case FPLX_COMMON_PTFP:
			mem_i = 1;
			break;
		case FPLX_COMMON_DZFP:
			mem_i = 2;
			break;
		case FPLX_COMMON_DZZP:
			mem_i = 3;
			break;
		default:
			//_WriteLog(LL_INFO, "jude_need_upload_inv 不支持的发票种类：%03d\n", fplxdm);
			continue;
		}



		item = cJSON_GetObjectItem(arrayItem, "data");
		memset(data_base, 0, sizeof(data_base));
		if (get_json_value_can_not_null(item, data_base, 1, sizeof(data_base)) < 0)
		{
			cJSON_Delete(root);
			return -8;
		}

		if (strlen(data_base) == 0)
		{
			cJSON_Delete(root);
			return -9;
		}

		data_json = malloc(strlen(data_base) + 1024);
		if (data_json == NULL)
		{
			cJSON_Delete(root);
			return -10;
		}
		Base64_Decode(data_base, strlen(data_base), data_json);


		cJSON *json = cJSON_Parse(data_json);
		if (!json)
		{
			free(data_json);
			return -11;
		}
		free(data_json);

		item = cJSON_GetObjectItem(json, "sjxsje");
		memset(sjxsje, 0, sizeof(sjxsje));
		if (get_json_value_can_not_null(item, sjxsje, 1, sizeof(sjxsje)) < 0)
		{
			cJSON_Delete(root);
			cJSON_Delete(json);
			return -12;
		}

		item = cJSON_GetObjectItem(json, "sjxsse");
		memset(sjxsse, 0, sizeof(sjxsse));
		if (get_json_value_can_not_null(item, sjxsse, 1, sizeof(sjxsse)) < 0)
		{
			cJSON_Delete(root);
			cJSON_Delete(json);
			return -13;
		}
		cJSON_Delete(json);
		//_WriteLog(LL_FATAL, "发票类型：%03d,汇总数据中实际销售金额：%s,实际销售税额：%s,明细数据中实际销售金额：%4.2f,实际销售税额：%4.2f", fplxdm,sjxsje, sjxsse, inv_sum->type_sum[mem_i].sjxsje, inv_sum->type_sum[mem_i].sjxsse);
		
		memset(sjxsje_mx, 0, sizeof(sjxsje_mx));
		memset(sjxsse_mx, 0, sizeof(sjxsse_mx));
		sprintf(sjxsje_mx, "%4.2Lf", inv_sum->type_sum[mem_i].sjxsje);
		sprintf(sjxsse_mx, "%4.2Lf", inv_sum->type_sum[mem_i].sjxsse);

		if (strcmp(sjxsje_mx, "-0.00") == 0)
		{
			memset(sjxsje_mx, 0, sizeof(sjxsje_mx));
			sprintf(sjxsje_mx,"0.00");
		}
		if (strcmp(sjxsse_mx, "-0.00") == 0)
		{
			memset(sjxsse_mx, 0, sizeof(sjxsse_mx));
			sprintf(sjxsse_mx, "0.00");
		}

		if ((strcmp(sjxsje_mx, sjxsje) == 0) && (strcmp(sjxsse_mx, sjxsse) == 0))
		{
			//_WriteLog(LL_FATAL, "发票类型：%03d,汇总数据与明细数据一致", fplxdm);
			ret = 0;
		}
		else
		{
			_WriteLog(LL_FATAL, "发票类型：%03d,汇总数据与明细数据不一致", fplxdm);
			cJSON_Delete(root);			
			return -14;
		}
	}
	
	cJSON_Delete(root);
	return ret;
}



int GetRedInvFromRedNum(HDEV hDev, char *redNum, uint8 **inv_json)
{

	if (!CheckTzdh(redNum, "s")) {
		_WriteLog(LL_FATAL, "[-] CheckTzdh1 failed");
		sprintf(hDev->hUSB->errinfo, "传入的红字信息表编号格式有误");
		return -1;
	}
	//申请时可以是16个0，校验时不能是16个0了
	if (!strcmp(redNum, "0000000000000000")) {
		_WriteLog(LL_FATAL, "[-] CheckTzdh2 failed");
		sprintf(hDev->hUSB->errinfo, "传入的红字信息表编号不能为0000000000000000");
		return -2;
	}
	if (!hDev)
		return -3;
	int nRet = -10;
	//int nChildRet = -1;
	int nBufSize = DEF_MAX_FPBUF_LEN;
	char *szBuf = calloc(1, nBufSize);
	HFPXX fpxxRepMini = MallocFpxx();
	if (!szBuf || !fpxxRepMini)
		goto FreeAndExit;

	if (DownloadTZDBH(hDev, "", "", redNum, szBuf, DEF_MAX_FPBUF_LEN) < 0)
		return -5;
	//_WriteHexToDebugFile("red-dl-rep.xml", (uint8 *)szBuf, strlen(szBuf));

#ifdef NEED_FP_DATA
	char name[200] = { 0 };
	memset(name, 0, sizeof(name));
	sprintf(name, "/mnt/masstore/get_invnum%s_xml.txt", redNum);
	write_file(name, (char *)szBuf, strlen(szBuf));

#endif
	if ((hDev->bDeviceType == DEVICE_AISINO) || (hDev->bDeviceType == DEVICE_NISEC))
	{
		if (GetFpxxFromRepByRedNum(szBuf, redNum, fpxxRepMini) < 0) {
			_WriteLog(LL_FATAL, "[-] GetFpxxFromRepByRedNum failed\r\n");
			nRet = -6;
			goto FreeAndExit;
		}
	}
	else if (hDev->bDeviceType == DEVICE_CNTAX)
	{
		if (GetFpxxFromRepByRedNumCntax(szBuf, redNum, fpxxRepMini) < 0) {
			_WriteLog(LL_FATAL, "[-] GetFpxxFromRepByRedNum failed\r\n");
			nRet = -6;
			goto FreeAndExit;
		}
	}
	fpxxRepMini->hDev = hDev;
	fpxx_to_json_rednum_data_v102(fpxxRepMini, inv_json, 1);
	nRet = 0;

FreeAndExit:
	if (fpxxRepMini)
		FreeFpxx(fpxxRepMini);
	if (szBuf)
		free(szBuf);

	return nRet;
}



int swukey_get_basic_info(HDEV hDev, struct _plate_infos *plate_info)
{

	//int nDataLen = 0;
	if (CntaxGetBasicInfo(hDev, (char *)plate_info->plate_basic_info.plate_num, (char *)plate_info->plate_basic_info.ca_number, (char *)plate_info->plate_basic_info.ca_name) < 0) {
		_WriteLog(LL_FATAL, "Nisec get bh|taxid|mc failed");
		return -1;
	}
	/*printf("----------------------------------------------------------------\n"
		"[+] Chinatax ueky found, Device SN:%s NSRID:%s NSRName:%s\n"
		"----------------------------------------------------------------\n",
		plate_info->plate_basic_info.plate_num, plate_info->plate_basic_info.ca_number, plate_info->plate_basic_info.ca_name);*/
	plate_info->plate_basic_info.result = 3;
	plate_info->plate_basic_info.plate_type = hDev->bDeviceType;
	plate_info->plate_basic_info.extension = hDev->uICCardNo;
	return 0;
}





int LoadSkpKeyDevInfo(HDEV hDev, HUSB hUSB)
{
	unsigned char cb[1024 * 10] = { 0 };
	//int nChildRet;

	memset(cb, 0, sizeof(cb));
	memcpy(cb, "\xfe\x6f\x01", 3);  // 
	NisecLogicIO(hUSB, cb, 3, sizeof(cb), NULL, 0);
	//printf("nChildRet = %d\n", nChildRet);
	//printf_array((char *)cb, nChildRet);

	sprintf(hDev->szCompanyName, "TCG-O1K税控钥匙");
	memcpy(hDev->szDeviceID, cb + 2, 12);
	sprintf(hDev->szCommonTaxID, "33-%s", hDev->szDeviceID);

	return 0;
}


int fpxx_to_json_base_fpsjbbh_v102(HFPXX stp_fpxx, char **fpxx_json, int sfxqqd,int data_type)
{
	int i;
	char tmp_s[200] = { 0 };
	//char spsmmc[200];
	//char spmc[200];
	int Spxx_num;
	cJSON *dir1;
	cJSON *json = cJSON_CreateObject();
	cJSON_AddStringToObject(json, "fpsjbbh", "V1.0.8");
	cJSON_AddStringToObject(json, "fpqqlsh", "");
	memset(tmp_s, 0, sizeof(tmp_s)); sprintf(tmp_s, "%d", stp_fpxx->hDev->bDeviceType);
	cJSON_AddStringToObject(json, "splx", tmp_s);

	cJSON_AddStringToObject(json, "spbh", stp_fpxx->hDev->szDeviceID);
	memset(tmp_s, 0, sizeof(tmp_s)); sprintf(tmp_s, "%d", stp_fpxx->kpjh);
	cJSON_AddStringToObject(json, "kpjh", tmp_s);
	memset(tmp_s, 0, sizeof(tmp_s)); sprintf(tmp_s, "%d", stp_fpxx->zfbz);
	cJSON_AddStringToObject(json, "zfbz", tmp_s);
	memset(tmp_s, 0, sizeof(tmp_s)); sprintf(tmp_s, "%d", stp_fpxx->bIsUpload);
	cJSON_AddStringToObject(json, "sbbz", tmp_s);
	cJSON_AddStringToObject(json, "fpdm", stp_fpxx->fpdm);
	cJSON_AddStringToObject(json, "fphm", stp_fpxx->fphm);
	cJSON_AddStringToObject(json, "kpsj", stp_fpxx->kpsj_F2);
	memset(tmp_s, 0, sizeof(tmp_s)); sprintf(tmp_s, "%d", stp_fpxx->isRed);
	cJSON_AddStringToObject(json, "kplx", tmp_s);
	memset(tmp_s, 0, sizeof(tmp_s)); sprintf(tmp_s, "%03d", stp_fpxx->fplx);
	cJSON_AddStringToObject(json, "fpzls", tmp_s);
	memset(tmp_s, 0, sizeof(tmp_s)); sprintf(tmp_s, "%d", stp_fpxx->zyfpLx);
	cJSON_AddStringToObject(json, "tspz", tmp_s);
	cJSON_AddStringToObject(json, "bmbbbh", stp_fpxx->bmbbbh);
	//cJSON_AddStringToObject(json, "hsbz", stp_fpxx->hsbz);
	//cJSON_AddStringToObject(json, "hzfw", stp_fpxx->hzfw);
	//cJSON_AddStringToObject(json, "dslvzc", stp_fpxx->dslp);
	//cJSON_AddStringToObject(json, "zhsl", stp_fpxx->zhsl);
	cJSON_AddStringToObject(json, "gfmc", stp_fpxx->gfmc);
	cJSON_AddStringToObject(json, "gfsh", stp_fpxx->gfsh);
	cJSON_AddStringToObject(json, "gfdzdh", stp_fpxx->gfdzdh);
	cJSON_AddStringToObject(json, "gfyhzh", stp_fpxx->gfyhzh);
	cJSON_AddStringToObject(json, "xfmc", stp_fpxx->xfmc);
	cJSON_AddStringToObject(json, "xfsh", stp_fpxx->xfsh);
	cJSON_AddStringToObject(json, "xfdzdh", stp_fpxx->xfdzdh);
	cJSON_AddStringToObject(json, "xfyhzh", stp_fpxx->xfyhzh);
	cJSON_AddStringToObject(json, "hsjbz", stp_fpxx->hsjbz);
	cJSON_AddStringToObject(json, "hjje", stp_fpxx->je);
	cJSON_AddStringToObject(json, "hjse", stp_fpxx->se);

	if (strlen(stp_fpxx->jshj) == 0)
		cJSON_AddStringToObject(json, "jshj", "0.00");
	else
		cJSON_AddStringToObject(json, "jshj", stp_fpxx->jshj);
	cJSON_AddStringToObject(json, "qdbz", stp_fpxx->qdbj);
	char bz_s[5120] = { 0 };
	sprintf(bz_s, "%s", stp_fpxx->bz);
	str_replace(bz_s, "\r\n", "<br/>");	
	str_replace(bz_s, "\n", "<br/>");			//wang 03 03 增加换行判断方式
	cJSON_AddStringToObject(json, "bz", bz_s);
	cJSON_AddStringToObject(json, "skr", stp_fpxx->skr);
	cJSON_AddStringToObject(json, "fhr", stp_fpxx->fhr);
	cJSON_AddStringToObject(json, "kpr", stp_fpxx->kpr);
	cJSON_AddStringToObject(json, "zyspsmmc", stp_fpxx->zyspsmmc);
	cJSON_AddStringToObject(json, "zyspmc", stp_fpxx->zyspmc);

	int tmp_time[20] = { 0 };
	sscanf(stp_fpxx->kpsj_F2, "%02x%02x-%02x-%02x %02x:%02x:%02x", &tmp_time[0], &tmp_time[1], &tmp_time[2], &tmp_time[3], &tmp_time[4], &tmp_time[5], &tmp_time[6]);
	sprintf(stp_fpxx->ssyf, "%02x%02x%02x", tmp_time[0], tmp_time[1], tmp_time[2]);


	cJSON_AddStringToObject(json, "ssyf", stp_fpxx->ssyf);
	cJSON_AddStringToObject(json, "tzdbh", stp_fpxx->redNum);

	memset(tmp_s, 0, sizeof(tmp_s)); sprintf(tmp_s, "%d", stp_fpxx->sslkjly);
	cJSON_AddStringToObject(json, "sslkjly", tmp_s);

	if (stp_fpxx->isRed)
	{
		cJSON_AddStringToObject(json, "yfpdm", stp_fpxx->blueFpdm);
		cJSON_AddStringToObject(json, "yfphm", stp_fpxx->blueFphm);
		cJSON_AddStringToObject(json, "yfpkjsj", stp_fpxx->blueKpsj);
		if (stp_fpxx->blueFplx != 0)
		{
			memset(tmp_s, 0, sizeof(tmp_s)); sprintf(tmp_s, "%03d", stp_fpxx->blueFplx);
			cJSON_AddStringToObject(json, "yfpzls", tmp_s);
		}
		else
		{
			cJSON_AddStringToObject(json, "yfpzls", "");
		}
		memset(tmp_s, 0, sizeof(tmp_s)); sprintf(tmp_s, "%d", stp_fpxx->hcyy);
		cJSON_AddStringToObject(json, "hcyy", tmp_s);
	}
	else
	{
		cJSON_AddStringToObject(json, "yfpdm", "");
		cJSON_AddStringToObject(json, "yfphm", "");
		cJSON_AddStringToObject(json, "yfpkjsj", "");
		cJSON_AddStringToObject(json, "yfpzls", "");
		cJSON_AddStringToObject(json, "hcyy", "");
	}

	//cJSON_AddStringToObject(json, "zfrq", stp_fpxx->zfrq);
	cJSON_AddStringToObject(json, "zfsj", stp_fpxx->zfsj_F2);
	cJSON_AddStringToObject(json, "jym", stp_fpxx->jym);
	cJSON_AddStringToObject(json, "mw", stp_fpxx->mw);
	memset(tmp_s, 0, sizeof(tmp_s)); sprintf(tmp_s, "%d", stp_fpxx->dzsyh);
	cJSON_AddStringToObject(json, "dzsyh", tmp_s);



	if (((stp_fpxx->fplx == FPLX_COMMON_DZFP) || (stp_fpxx->fplx == FPLX_COMMON_DZZP)) && (strlen(stp_fpxx->hDev->szPubServer) != 0))
	{
		//公共服务平台配置, 平台类型(2) + 平台号码...
		//char used_ofd[10] = { 0 };
		char server_type[20] = { 0 };
		char server_number[50] = { 0 };

		//memcpy(used_ofd, stp_fpxx->hDev->szPubServer,1);
		memcpy(server_type, stp_fpxx->hDev->szPubServer, 2);
		strcpy(server_number, stp_fpxx->hDev->szPubServer + 2);

		cJSON_AddStringToObject(json, "used_ofd", "1");
		cJSON_AddStringToObject(json, "server_type", server_type);
		cJSON_AddStringToObject(json, "server_number", server_number);

		//_WriteLog(LL_FATAL, "stp_fpxx->pubcode  %s\n", stp_fpxx->pubcode);
		char url[5000] = { 0 };
		if (strlen(stp_fpxx->pubcode) > 5)
		{
			char tqmxx[100] = { 0 };
			char *p;			
			char time_str[20] = { 0 };
			//char szUPServURL[500] = {0};
			p = strstr(stp_fpxx->pubcode, ";");
			if (p != NULL)
			{
				p = strstr(p + 1, ";");
				if (p != NULL)
				{
					memcpy(time_str, stp_fpxx->kpsj_standard, 8);
					memcpy(tqmxx, p + 1, strlen(stp_fpxx->pubcode) - (p - stp_fpxx->pubcode));
					if (strlen(tqmxx) > 0)
					{
						struct UploadAddrModel model;
						memset(&model,0,sizeof(struct UploadAddrModel));
						if (GetTaServerURL(TAADDR_PLATFORM, stp_fpxx->hDev->szRegCode, &model) == RET_SUCCESS)
						//if (GetUPServerURL(1, stp_fpxx->hDev->szRegCode, szUPServURL) == 0)
						{
							sprintf(url, "%s/preview.html?code=%s_%s_%s_%s", model.szTaxAuthorityURL, stp_fpxx->fpdm, stp_fpxx->fphm, time_str, tqmxx);
						}
					}
				}
			}
		}
		cJSON_AddStringToObject(json, "ofdurl", url);
	}

	//_WriteLog(LL_FATAL,"发票信息组包完成\n");
	cJSON_AddItemToObject(json, "fpmx", dir1 = cJSON_CreateArray());
	struct Spxx *stp_Spxx = NULL;
	if (strcmp(stp_fpxx->qdbj, "Y") == 0)
	{
		stp_Spxx = stp_fpxx->stp_MxxxHead;
		Spxx_num = stp_fpxx->spsl + 1;
	}
	else
	{
		stp_Spxx = stp_fpxx->stp_MxxxHead->stp_next;
		Spxx_num = stp_fpxx->spsl;
	}
	//_WriteLog(LL_FATAL,"商品数量=%d\n", Spxx_num);
	for (i = 0; i < Spxx_num; i++)
	{
		//_WriteLog(LL_FATAL,"组包第%d行商品\n",i+1);
		if (stp_Spxx == NULL)
		{
			break;
		}
		cJSON *dir2;
		char xh_tmp[10] = { 0 };
		sprintf(xh_tmp, "%d", i + 1);
		cJSON_AddItemToObject(dir1, "dira", dir2 = cJSON_CreateObject());
		if ((i == 0) && (strcmp(stp_fpxx->qdbj, "Y") == 0))
		{
			cJSON_AddStringToObject(dir2, "xh", xh_tmp);
			cJSON_AddStringToObject(dir2, "spsmmc", "");
			cJSON_AddStringToObject(dir2, "spmc", stp_Spxx->spmc);
			cJSON_AddStringToObject(dir2, "spbh", stp_Spxx->spbh);
			cJSON_AddStringToObject(dir2, "zxbh", "");

			cJSON_AddStringToObject(dir2, "fphxz", stp_Spxx->fphxz);
			cJSON_AddStringToObject(dir2, "hsjbz", stp_Spxx->hsjbz);
			cJSON_AddStringToObject(dir2, "dj", stp_Spxx->dj);
			cJSON_AddStringToObject(dir2, "jldw", stp_Spxx->jldw);
			cJSON_AddStringToObject(dir2, "ggxh", stp_Spxx->ggxh);
			cJSON_AddStringToObject(dir2, "se", stp_Spxx->se);
			cJSON_AddStringToObject(dir2, "je", stp_Spxx->je);
			cJSON_AddStringToObject(dir2, "slv", stp_Spxx->slv);
			cJSON_AddStringToObject(dir2, "sl", stp_Spxx->sl);
			cJSON_AddStringToObject(dir2, "yhzcbs", stp_Spxx->xsyh);
			cJSON_AddStringToObject(dir2, "slbs", stp_Spxx->lslvbs);
			//cJSON_AddStringToObject(dir2, "zzstsgl", "");
			cJSON_AddStringToObject(dir2, "zzstsgl", stp_Spxx->yhsm);
			cJSON_AddStringToObject(dir2, "zsfs", "0");
			cJSON_AddStringToObject(dir2, "cezskce", "");
			if (sfxqqd == 0)
				break;
		}
		else
		{
			//memset(spsmmc, 0, sizeof(spsmmc));
			//memset(spmc, 0, sizeof(spmc));
			//get_spsmmc_spmc(stp_Spxx->spmc, spsmmc, spmc);
			cJSON_AddStringToObject(dir2, "xh", xh_tmp);
			cJSON_AddStringToObject(dir2, "spsmmc", stp_Spxx->spsmmc);
			cJSON_AddStringToObject(dir2, "spmc", stp_Spxx->spmc);
			cJSON_AddStringToObject(dir2, "spbh", stp_Spxx->spbh);
			cJSON_AddStringToObject(dir2, "zxbh", "");


			cJSON_AddStringToObject(dir2, "fphxz", stp_Spxx->fphxz);
			cJSON_AddStringToObject(dir2, "hsjbz", stp_Spxx->hsjbz);
			cJSON_AddStringToObject(dir2, "dj", stp_Spxx->dj);
			cJSON_AddStringToObject(dir2, "jldw", stp_Spxx->jldw);
			cJSON_AddStringToObject(dir2, "ggxh", stp_Spxx->ggxh);
			cJSON_AddStringToObject(dir2, "se", stp_Spxx->se);
			cJSON_AddStringToObject(dir2, "je", stp_Spxx->je);
			cJSON_AddStringToObject(dir2, "slv", stp_Spxx->slv);
			cJSON_AddStringToObject(dir2, "sl", stp_Spxx->sl);
			cJSON_AddStringToObject(dir2, "yhzcbs", stp_Spxx->xsyh);
			cJSON_AddStringToObject(dir2, "slbs", stp_Spxx->lslvbs);
			//cJSON_AddStringToObject(dir2, "zzstsgl", "");
			cJSON_AddStringToObject(dir2, "zzstsgl", stp_Spxx->yhsm);
			cJSON_AddStringToObject(dir2, "zsfs", "0");
			cJSON_AddStringToObject(dir2, "cezskce", "");
		}
		stp_Spxx = stp_Spxx->stp_next;
	}
	//_WriteLog(LL_FATAL,"商品信息组包完成\n");
	char *json_buf;
	char *g_buf;
	char *base_buf;
	int g_len;
	json_buf = cJSON_Print(json);
	g_len = strlen(json_buf) * 2;

	g_buf = malloc(g_len);
	memset(g_buf, 0, g_len);
	memcpy(g_buf, json_buf, strlen(json_buf));
	free(json_buf);
	//str_replace(g_buf, "\\", "\\\\");
	str_replace(g_buf, "\\\\", "[@*br/*@]");
	str_replace(g_buf, "[@*br/*@]", "\\");

	//_WriteLog(LL_FATAL, "%s", g_buf);
#ifdef NEED_FP_DATA
	char name[200] = { 0 };
	memset(name, 0, sizeof(name));
	sprintf(name, "/mnt/masstore/get_kpjson%s_%s.txt", stp_fpxx->fpdm, stp_fpxx->fphm);
	write_file(name, (char *)g_buf, strlen(g_buf));

#endif
#ifndef DF_OLD_MSERVER
	if (data_type == 0)
	{
		*fpxx_json = g_buf;
		cJSON_Delete(json);
		return 0;
	}
#endif	
	base_buf = malloc(strlen(g_buf) * 2);
	memset(base_buf, 0, sizeof(strlen(g_buf) * 2));
	Base64_Encode(g_buf, strlen(g_buf), base_buf);
	//_WriteLog(LL_FATAL,"发票json数据，压缩前长度%d,压缩后长度%d\n", strlen(g_buf), strlen(base_buf));
	*fpxx_json = base_buf;
	free(g_buf);
	g_buf = NULL;
	cJSON_Delete(json);
	return 0;
}


int fpxx_to_json_base_fpsjbbh_v102_aison(HFPXX stp_fpxx, char **fpxx_json, int sfxqqd,int data_type)
{
	int i;
	char tmp_s[200] = { 0 };
	//char spsmmc[200];
	//char spmc[200];
	int Spxx_num;
	cJSON *dir1;
	cJSON *json = cJSON_CreateObject();
	cJSON_AddStringToObject(json, "fpsjbbh", "V1.0.8");
	cJSON_AddStringToObject(json, "fpqqlsh", "");
	memset(tmp_s, 0, sizeof(tmp_s)); sprintf(tmp_s, "%d", stp_fpxx->hDev->bDeviceType);
	cJSON_AddStringToObject(json, "splx", tmp_s);

	cJSON_AddStringToObject(json, "spbh", stp_fpxx->hDev->szDeviceID);
	memset(tmp_s, 0, sizeof(tmp_s)); sprintf(tmp_s, "%d", stp_fpxx->kpjh);
	cJSON_AddStringToObject(json, "kpjh", tmp_s);
	memset(tmp_s, 0, sizeof(tmp_s)); sprintf(tmp_s, "%d", stp_fpxx->zfbz);
	cJSON_AddStringToObject(json, "zfbz", tmp_s);
	memset(tmp_s, 0, sizeof(tmp_s)); sprintf(tmp_s, "%d", stp_fpxx->bIsUpload);
	cJSON_AddStringToObject(json, "sbbz", tmp_s);
	cJSON_AddStringToObject(json, "fpdm", stp_fpxx->fpdm);
	cJSON_AddStringToObject(json, "fphm", stp_fpxx->fphm);
	cJSON_AddStringToObject(json, "kpsj", stp_fpxx->kpsj_F2);
	memset(tmp_s, 0, sizeof(tmp_s)); sprintf(tmp_s, "%d", stp_fpxx->isRed);
	cJSON_AddStringToObject(json, "kplx", tmp_s);
	memset(tmp_s, 0, sizeof(tmp_s)); sprintf(tmp_s, "%03d", stp_fpxx->fplx);
	cJSON_AddStringToObject(json, "fpzls", tmp_s);
	memset(tmp_s, 0, sizeof(tmp_s)); sprintf(tmp_s, "%d", stp_fpxx->zyfpLx);
	cJSON_AddStringToObject(json, "tspz", tmp_s);
	cJSON_AddStringToObject(json, "bmbbbh", stp_fpxx->bmbbbh);
	//cJSON_AddStringToObject(json, "hsbz", stp_fpxx->hsbz);
	//cJSON_AddStringToObject(json, "hzfw", stp_fpxx->hzfw);
	//cJSON_AddStringToObject(json, "dslvzc", stp_fpxx->dslp);
	//cJSON_AddStringToObject(json, "zhsl", stp_fpxx->zhsl);
	cJSON_AddStringToObject(json, "gfmc", stp_fpxx->gfmc);
	cJSON_AddStringToObject(json, "gfsh", stp_fpxx->gfsh);
	cJSON_AddStringToObject(json, "gfdzdh", stp_fpxx->gfdzdh);
	cJSON_AddStringToObject(json, "gfyhzh", stp_fpxx->gfyhzh);
	cJSON_AddStringToObject(json, "xfmc", stp_fpxx->xfmc);
	cJSON_AddStringToObject(json, "xfsh", stp_fpxx->xfsh);
	cJSON_AddStringToObject(json, "xfdzdh", stp_fpxx->xfdzdh);
	cJSON_AddStringToObject(json, "xfyhzh", stp_fpxx->xfyhzh);
	cJSON_AddStringToObject(json, "hsjbz", stp_fpxx->hsjbz);
	cJSON_AddStringToObject(json, "hjje", stp_fpxx->je);
	cJSON_AddStringToObject(json, "hjse", stp_fpxx->se);

	if (strlen(stp_fpxx->jshj) == 0)
		cJSON_AddStringToObject(json, "jshj", "0.00");
	else
		cJSON_AddStringToObject(json, "jshj", stp_fpxx->jshj);
	cJSON_AddStringToObject(json, "qdbz", stp_fpxx->qdbj);
	char bz_s[5120] = { 0 };
	sprintf(bz_s, "%s", stp_fpxx->bz);
	str_replace(bz_s, "\r\n", "<br/>");	
	str_replace(bz_s, "\n", "<br/>");			//wang 03 03 增加换行判断方式
	cJSON_AddStringToObject(json, "bz", bz_s);
	cJSON_AddStringToObject(json, "skr", stp_fpxx->skr);
	cJSON_AddStringToObject(json, "fhr", stp_fpxx->fhr);
	// printf("stp_fpxx->zfbz = %d,stp_fpxx->spsl = %d\n",stp_fpxx->zfbz,stp_fpxx->spsl);
	// if(stp_fpxx->stp_MxxxHead != NULL)
	// {
	// 	printf("stp_fpxx->stp_MxxxHead != NULL\n");
	// }
	// else
	// {
	// 	printf("stp_fpxx->stp_MxxxHead == NULL\n");
	// }
	//if(true != stp_fpxx->zfbz || stp_fpxx->spsl != 0)   //只有空白费不显示  kpr
	{
		cJSON_AddStringToObject(json, "kpr", stp_fpxx->kpr);
	}
	cJSON_AddStringToObject(json, "zyspsmmc", stp_fpxx->zyspsmmc);
	cJSON_AddStringToObject(json, "zyspmc", stp_fpxx->zyspmc);

	int tmp_time[20] = { 0 };
	sscanf(stp_fpxx->kpsj_F2, "%02x%02x-%02x-%02x %02x:%02x:%02x", &tmp_time[0], &tmp_time[1], &tmp_time[2], &tmp_time[3], &tmp_time[4], &tmp_time[5], &tmp_time[6]);
	sprintf(stp_fpxx->ssyf, "%02x%02x%02x", tmp_time[0], tmp_time[1], tmp_time[2]);


	cJSON_AddStringToObject(json, "ssyf", stp_fpxx->ssyf);
	cJSON_AddStringToObject(json, "tzdbh", stp_fpxx->redNum);

	memset(tmp_s, 0, sizeof(tmp_s)); sprintf(tmp_s, "%d", stp_fpxx->sslkjly);
	cJSON_AddStringToObject(json, "sslkjly", tmp_s);

	if (stp_fpxx->isRed)
	{
		cJSON_AddStringToObject(json, "yfpdm", stp_fpxx->blueFpdm);
		cJSON_AddStringToObject(json, "yfphm", stp_fpxx->blueFphm);
		cJSON_AddStringToObject(json, "yfpkjsj", stp_fpxx->blueKpsj);
		if (stp_fpxx->blueFplx != 0)
		{
			memset(tmp_s, 0, sizeof(tmp_s)); sprintf(tmp_s, "%03d", stp_fpxx->blueFplx);
			cJSON_AddStringToObject(json, "yfpzls", tmp_s);
		}
		else
		{
			cJSON_AddStringToObject(json, "yfpzls", "");
		}
		memset(tmp_s, 0, sizeof(tmp_s)); sprintf(tmp_s, "%d", stp_fpxx->hcyy);
		cJSON_AddStringToObject(json, "hcyy", tmp_s);
	}
	else
	{
		cJSON_AddStringToObject(json, "yfpdm", "");
		cJSON_AddStringToObject(json, "yfphm", "");
		cJSON_AddStringToObject(json, "yfpkjsj", "");
		cJSON_AddStringToObject(json, "yfpzls", "");
		cJSON_AddStringToObject(json, "hcyy", "");
	}

	//cJSON_AddStringToObject(json, "zfrq", stp_fpxx->zfrq);
	cJSON_AddStringToObject(json, "zfsj", stp_fpxx->zfsj_F2);
	cJSON_AddStringToObject(json, "jym", stp_fpxx->jym);
	cJSON_AddStringToObject(json, "mw", stp_fpxx->mw);
	memset(tmp_s, 0, sizeof(tmp_s)); sprintf(tmp_s, "%d", stp_fpxx->dzsyh);
	cJSON_AddStringToObject(json, "dzsyh", tmp_s);



	if (((stp_fpxx->fplx == FPLX_COMMON_DZFP) || (stp_fpxx->fplx == FPLX_COMMON_DZZP)) && (strlen(stp_fpxx->hDev->szPubServer) != 0))
	{
		//公共服务平台配置, 平台类型(2) + 平台号码...
		//char used_ofd[10] = { 0 };
		char server_type[20] = { 0 };
		char server_number[50] = { 0 };

		//memcpy(used_ofd, stp_fpxx->hDev->szPubServer,1);
		memcpy(server_type, stp_fpxx->hDev->szPubServer, 2);
		strcpy(server_number, stp_fpxx->hDev->szPubServer + 2);

		cJSON_AddStringToObject(json, "used_ofd", "1");
		cJSON_AddStringToObject(json, "server_type", server_type);
		cJSON_AddStringToObject(json, "server_number", server_number);

		//_WriteLog(LL_FATAL, "stp_fpxx->pubcode  %s\n", stp_fpxx->pubcode);
		char url[5000] = { 0 };
		if (strlen(stp_fpxx->pubcode) > 5)
		{
			char tqmxx[100] = { 0 };
			char *p;			
			char time_str[20] = { 0 };
			//char szUPServURL[500] = {0};
			p = strstr(stp_fpxx->pubcode, ";");
			if (p != NULL)
			{
				p = strstr(p + 1, ";");
				if (p != NULL)
				{
					memcpy(time_str, stp_fpxx->kpsj_standard, 8);
					memcpy(tqmxx, p + 1, strlen(stp_fpxx->pubcode) - (p - stp_fpxx->pubcode));
					if (strlen(tqmxx) > 0)
					{
						struct UploadAddrModel model;
						memset(&model,0,sizeof(struct UploadAddrModel));
						if (GetTaServerURL(TAADDR_PLATFORM, stp_fpxx->hDev->szRegCode, &model) == RET_SUCCESS)
						//if (GetUPServerURL(1, stp_fpxx->hDev->szRegCode, szUPServURL) == 0)
						{
							sprintf(url, "%s/preview.html?code=%s_%s_%s_%s", model.szTaxAuthorityURL, stp_fpxx->fpdm, stp_fpxx->fphm, time_str, tqmxx);
						}
					}
				}
			}
		}
		cJSON_AddStringToObject(json, "ofdurl", url);
	}

	//_WriteLog(LL_FATAL,"发票信息组包完成\n");
	cJSON_AddItemToObject(json, "fpmx", dir1 = cJSON_CreateArray());
	struct Spxx *stp_Spxx = NULL;
	if (strcmp(stp_fpxx->qdbj, "Y") == 0)
	{
		stp_Spxx = stp_fpxx->stp_MxxxHead;
		Spxx_num = stp_fpxx->spsl + 1;
	}
	else
	{
		stp_Spxx = stp_fpxx->stp_MxxxHead->stp_next;
		Spxx_num = stp_fpxx->spsl;
	}
	_WriteLog(LL_FATAL,"商品数量=%d\n", Spxx_num);
	for (i = 0; i < Spxx_num; i++)
	{
		//_WriteLog(LL_FATAL,"组包第%d行商品\n",i+1);
		if (stp_Spxx == NULL)
		{
			break;
		}
		cJSON *dir2;
		char xh_tmp[10] = { 0 };
		sprintf(xh_tmp, "%d", i + 1);
		cJSON_AddItemToObject(dir1, "dira", dir2 = cJSON_CreateObject());
		if ((i == 0) && (strcmp(stp_fpxx->qdbj, "Y") == 0))
		{
			cJSON_AddStringToObject(dir2, "xh", xh_tmp);
			cJSON_AddStringToObject(dir2, "spsmmc", "");
			cJSON_AddStringToObject(dir2, "spmc", stp_Spxx->spmc);
			cJSON_AddStringToObject(dir2, "spbh", stp_Spxx->spbh);
			cJSON_AddStringToObject(dir2, "zxbh", "");

			cJSON_AddStringToObject(dir2, "fphxz", stp_Spxx->fphxz);
			cJSON_AddStringToObject(dir2, "hsjbz", stp_Spxx->hsjbz);
			cJSON_AddStringToObject(dir2, "dj", stp_Spxx->dj);
			cJSON_AddStringToObject(dir2, "jldw", stp_Spxx->jldw);
			cJSON_AddStringToObject(dir2, "ggxh", stp_Spxx->ggxh);
			cJSON_AddStringToObject(dir2, "se", stp_Spxx->se);
			cJSON_AddStringToObject(dir2, "je", stp_Spxx->je);
			cJSON_AddStringToObject(dir2, "slv", stp_Spxx->slv);
			cJSON_AddStringToObject(dir2, "sl", stp_Spxx->sl);
			cJSON_AddStringToObject(dir2, "yhzcbs", stp_Spxx->xsyh);
			cJSON_AddStringToObject(dir2, "slbs", stp_Spxx->lslvbs);
			//cJSON_AddStringToObject(dir2, "zzstsgl", "");
			cJSON_AddStringToObject(dir2, "zzstsgl", stp_Spxx->yhsm);
			cJSON_AddStringToObject(dir2, "zsfs", "0");
			cJSON_AddStringToObject(dir2, "cezskce", "");
			if (sfxqqd == 0)
				break;
		}
		else
		{
			//memset(spsmmc, 0, sizeof(spsmmc));
			//memset(spmc, 0, sizeof(spmc));
			//get_spsmmc_spmc(stp_Spxx->spmc, spsmmc, spmc);
			cJSON_AddStringToObject(dir2, "xh", xh_tmp);
			cJSON_AddStringToObject(dir2, "spsmmc", stp_Spxx->spsmmc);
			cJSON_AddStringToObject(dir2, "spmc", stp_Spxx->spmc);
			cJSON_AddStringToObject(dir2, "spbh", stp_Spxx->spbh);
			cJSON_AddStringToObject(dir2, "zxbh", "");


			cJSON_AddStringToObject(dir2, "fphxz", stp_Spxx->fphxz);
			cJSON_AddStringToObject(dir2, "hsjbz", stp_Spxx->hsjbz);
			cJSON_AddStringToObject(dir2, "dj", stp_Spxx->dj);
			cJSON_AddStringToObject(dir2, "jldw", stp_Spxx->jldw);
			cJSON_AddStringToObject(dir2, "ggxh", stp_Spxx->ggxh);
			cJSON_AddStringToObject(dir2, "se", stp_Spxx->se);
			cJSON_AddStringToObject(dir2, "je", stp_Spxx->je);
			cJSON_AddStringToObject(dir2, "slv", stp_Spxx->slv);
			cJSON_AddStringToObject(dir2, "sl", stp_Spxx->sl);
			cJSON_AddStringToObject(dir2, "yhzcbs", stp_Spxx->xsyh);
			cJSON_AddStringToObject(dir2, "slbs", stp_Spxx->lslvbs);
			//cJSON_AddStringToObject(dir2, "zzstsgl", "");
			cJSON_AddStringToObject(dir2, "zzstsgl", stp_Spxx->yhsm);
			cJSON_AddStringToObject(dir2, "zsfs", "0");
			cJSON_AddStringToObject(dir2, "cezskce", "");
		}
		stp_Spxx = stp_Spxx->stp_next;
	}
	//_WriteLog(LL_FATAL,"商品信息组包完成\n");
	char *json_buf;
	char *g_buf;
	char *base_buf;
	int g_len;
	json_buf = cJSON_Print(json);
	g_len = strlen(json_buf) * 2;

	g_buf = malloc(g_len);
	memset(g_buf, 0, g_len);
	memcpy(g_buf, json_buf, strlen(json_buf));
	free(json_buf);
	//str_replace(g_buf, "\\", "\\\\");
	str_replace(g_buf, "\\\\", "[@*br/*@]");
	str_replace(g_buf, "[@*br/*@]", "\\");

	//_WriteLog(LL_FATAL, "%s", g_buf);
#ifdef NEED_FP_DATA
	char name[200] = { 0 };
	memset(name, 0, sizeof(name));
	sprintf(name, "/mnt/masstore/get_kpjson%s_%s.txt", stp_fpxx->fpdm, stp_fpxx->fphm);
	write_file(name, (char *)g_buf, strlen(g_buf));

#endif
#ifndef DF_OLD_MSERVER
	if (data_type == 0)
	{
		*fpxx_json = g_buf;
		cJSON_Delete(json);
		return 0;
	}
#endif	
	base_buf = malloc(strlen(g_buf) * 2);
	memset(base_buf, 0, sizeof(strlen(g_buf) * 2));
	Base64_Encode(g_buf, strlen(g_buf), base_buf);
	//_WriteLog(LL_FATAL,"发票json数据，压缩前长度%d,压缩后长度%d\n", strlen(g_buf), strlen(base_buf));
	*fpxx_json = base_buf;
	free(g_buf);
	g_buf = NULL;
	cJSON_Delete(json);
	return 0;
}


int fpxx_to_json_base_ykfpcx(HFPXX stp_fpxx, char **json_data)
{
	//unsigned char fplxdm;
	char tmp[20] = { 0 };
	float jshj;
	int i;
	int Spxx_num;
	//uint8  *p_buf = NULL;;
	cJSON *root = NULL, *array_tmp = NULL;
	root = cJSON_CreateObject();
	if (root == NULL){
		return -1;
	}


	memset(tmp, 0, sizeof(tmp)); sprintf(tmp, "%d", stp_fpxx->zfbz);
	cJSON_AddStringToObject(root, "ZFBZ", tmp);//作废标志
	memset(tmp, 0, sizeof(tmp)); sprintf(tmp, "%03d", stp_fpxx->fplx);
	cJSON_AddStringToObject(root, "ZFSJ", (const char *)stp_fpxx->zfsj_F2);//作废时间
	cJSON_AddStringToObject(root, "FPLX", (const char *)tmp);//发票类型
	cJSON_AddStringToObject(root, "FPDM", (const char *)stp_fpxx->fpdm);//发票代码
	cJSON_AddStringToObject(root, "FPHM", (const char *)stp_fpxx->fphm);//发票号码
	cJSON_AddStringToObject(root, "GFMC", (const char *)stp_fpxx->gfmc);//购方名称
	cJSON_AddStringToObject(root, "GFSH", (const char *)stp_fpxx->gfsh);//购方税号
	cJSON_AddStringToObject(root, "XFMC", (const char *)stp_fpxx->xfmc);//销方名称
	cJSON_AddStringToObject(root, "XFSH", (const char *)stp_fpxx->xfsh);//销方税号
	cJSON_AddStringToObject(root, "KPSJ", (const char *)stp_fpxx->kpsj_F2);//开票时间	

	cJSON_AddStringToObject(root, "HJJE", (const char *)stp_fpxx->je);//合计金额
	cJSON_AddStringToObject(root, "HJSE", (const char *)stp_fpxx->se);//合计税额
	jshj = atof((const char *)stp_fpxx->je) + atof((const char *)stp_fpxx->se);
	memset(tmp, 0, sizeof(tmp)); sprintf(tmp, "%.2f", jshj);
	cJSON_AddStringToObject(root, "JSHJ", (const char *)tmp);//合计税额

	char bz_s[1024] = { 0 };
	sprintf(bz_s, "%s", (const char *)stp_fpxx->bz);
	str_replace(bz_s, "\r\n", "<br/>");

	cJSON_AddStringToObject(root, "BZ", bz_s);//备注



	cJSON_AddStringToObject(root, "HSJBZ", stp_fpxx->hsjbz);//含税价标志
	memset(tmp, 0, sizeof(tmp)); sprintf(tmp, "%d", (int)stp_fpxx->bIsUpload);
	cJSON_AddStringToObject(root, "SBBZ", tmp);
	check_special_char((unsigned char *)stp_fpxx->gfdzdh);
	cJSON_AddStringToObject(root, "GFDZDH", (const char *)stp_fpxx->gfdzdh);
	check_special_char((unsigned char *)stp_fpxx->gfyhzh);
	cJSON_AddStringToObject(root, "GFYHZH", (const char *)stp_fpxx->gfyhzh);
	check_special_char((unsigned char *)stp_fpxx->xfdzdh);
	cJSON_AddStringToObject(root, "XFDZDH", (const char *)stp_fpxx->xfdzdh);
	check_special_char((unsigned char *)stp_fpxx->xfyhzh);
	cJSON_AddStringToObject(root, "XFYHZH", (const char *)stp_fpxx->xfyhzh);
	cJSON_AddStringToObject(root, "JYM", (const char *)stp_fpxx->jym);
	cJSON_AddStringToObject(root, "MW", (const char *)stp_fpxx->mw);

	if (strcmp(stp_fpxx->qdbj, "Y") == 0)
	{
		cJSON_AddStringToObject(root, "QDBZ", "1");
	}
	else
	{
		cJSON_AddStringToObject(root, "QDBZ", "0");
	}

	array_tmp = cJSON_CreateArray();
	cJSON_AddItemToObject(root, "FPMX", array_tmp);
	struct Spxx *stp_Spxx = NULL;
	if (strcmp(stp_fpxx->qdbj, "Y") == 0)
	{
		stp_Spxx = stp_fpxx->stp_MxxxHead;
		Spxx_num = stp_fpxx->spsl + 1;
	}
	else
	{
		stp_Spxx = stp_fpxx->stp_MxxxHead->stp_next;
		Spxx_num = stp_fpxx->spsl;
	}
	for (i = 0; i < Spxx_num; i++)
	{
		//_WriteLog(LL_FATAL,"组包第%d行商品\n", i + 1);
		if (stp_Spxx == NULL)
		{
			break;
		}
		cJSON *dir2;

		if ((i == 0) && (strcmp(stp_fpxx->qdbj, "Y") == 0))
		{
			;//清单行不输出

		}
		else
		{
			cJSON_AddItemToObject(array_tmp, "dira", dir2 = cJSON_CreateObject());
			char spmc[500] = { 0 };
			sprintf(spmc, "%s%s", stp_Spxx->spsmmc, stp_Spxx->spmc);
			cJSON_AddStringToObject(dir2, "SPMC", spmc);
			cJSON_AddStringToObject(dir2, "DJ", stp_Spxx->dj);
			cJSON_AddStringToObject(dir2, "JLDW", stp_Spxx->jldw);
			cJSON_AddStringToObject(dir2, "GGXH", stp_Spxx->ggxh);
			cJSON_AddStringToObject(dir2, "SE", stp_Spxx->se);
			cJSON_AddStringToObject(dir2, "JE", stp_Spxx->je);
			cJSON_AddStringToObject(dir2, "SLV", stp_Spxx->slv);
			cJSON_AddStringToObject(dir2, "SL", stp_Spxx->sl);
			cJSON_AddStringToObject(dir2, "SPBH", stp_Spxx->spbh);
			cJSON_AddStringToObject(dir2, "HSJBZ", stp_Spxx->hsjbz);

		}
		stp_Spxx = stp_Spxx->stp_next;
	}
	//_WriteLog(LL_FATAL,"商品信息组包完成\n");

	//char *g_buf;
	//char *base_buf;
	//g_buf = cJSON_Print(root);
	//str_replace(g_buf, "\\\\", "\\");

	int g_len;
	char *json_buf;
	char *g_buf;
	char *base_buf;
	json_buf = cJSON_Print(root);
	g_len = strlen(json_buf) * 2;
	g_buf = malloc(g_len);
	memset(g_buf, 0, g_len);
	memcpy(g_buf, json_buf, strlen(json_buf));
	free(json_buf);
	//str_replace(g_buf, "\\", "\\\\");
	str_replace(g_buf, "\\\\", "[@*br/*@]");
	str_replace(g_buf, "[@*br/*@]", "\\");
	//_WriteLog(LL_FATAL,"g_buf：%s\n", g_buf);

	//char name[200];
	//memset(name, 0, sizeof(name));
	//sprintf(name, "NO%s_%s.%d", stp_fpxx->fpdm, stp_fpxx->fphm,2);
	//if (write_file(name, (char *)g_buf, strlen(g_buf))<0)
	//{
	//	//_WriteLog(LL_FATAL,"读取文件:[%s]失败\n", name);
	//}


	base_buf = malloc(strlen(g_buf) * 2);
	Base64_Encode(g_buf, strlen(g_buf), base_buf);
	//_WriteLog(LL_FATAL,"压缩前长度%d,压缩后长度%d\n", strlen(g_buf), strlen(base_buf));
	*json_data = base_buf;
	free(g_buf);
	cJSON_Delete(root);
	return 0;
}

// fpxx need init by caller, free by caller
int get_fpxx_from_hzxxb_data_aisino_nisec(HDEV hDev, char *szRep, uint8 *send_data, int send_len, char *errinfo)
{
	// 3201142004040539
	int tzd_num = 0;
	long index = 0;
	mxml_node_t *pXMLRoot = NULL, *node = NULL, *nodeResultChildSingle = NULL;
	int nRet = 0, i = 0, nCount = 0;
	char *pPayload = NULL, szBuf[512] = "";
	if (LoadZZSXMLDataNodeTemplate(&pXMLRoot, &node, "ALLCOUNT", szRep) < 0) {
		_WriteLog(LL_FATAL, "LoadZZSXMLDataNodeTemplate failed, Msg:%s", szRep);
		return -1;
	}
	if ((pPayload = (char *)mxmlGetOpaque(node)) == NULL) {
		nRet = -6;
		goto Finish;
	}
	nCount = atoi(pPayload);
	//_WriteLog(LL_FATAL, "nCount = %d\n", nCount);
	nodeResultChildSingle = node;
	// node1 node2 没用了可以被改为其他值
	for (i = 0; i < nCount; i++) {
		//需要两次，不清楚为什么
		//_WriteLog(LL_FATAL, "第 %d次解析\n", i);
		if ((nodeResultChildSingle = mxmlGetNextSibling(nodeResultChildSingle)) == NULL) {
			nRet = -5;
			goto Finish;
		}
		if ((nodeResultChildSingle = mxmlGetNextSibling(nodeResultChildSingle)) == NULL) {
			nRet = -6;
			goto Finish;
		}
		if ((node = mxmlFindElement(nodeResultChildSingle, pXMLRoot, "ResBillNo", NULL, NULL,
			MXML_DESCEND)) == NULL)
			continue;
		if ((pPayload = (char *)mxmlGetOpaque(node)) == NULL)
			continue;
		//if (strcmp(szRedNum, pPayload))
		//	continue;  // not match
		if (GetChildNodeValue(nodeResultChildSingle, "StatusDM", szBuf) < 0) {
			nRet = -7;
			goto Finish;
		}
		if (strcmp(szBuf, "TZD0082") == 0) {
			continue;
		}

		tzd_num += 1;
		//_WriteLog(LL_INFO, "读取到有效红字信息表张数%d", tzd_num);
		HFPXX fpxxRepMini = MallocFpxx();
		nRet = HZXMLNode2FPXX(nodeResultChildSingle, fpxxRepMini);
		if (nRet < 0)
		{
			FreeFpxx(fpxxRepMini);
			continue;
		}
		_WriteLog(LL_INFO, "读取到有效红字信息表张数%d,发票代码：%s,发票号码：%s,通知单编号：%s", tzd_num, fpxxRepMini->blueFpdm, fpxxRepMini->blueFphm, fpxxRepMini->redNum);
		fpxxRepMini->hDev = hDev;
		uint8 *encode_data;
		int inv_len;
		fpxx_to_json_rednum_data_v102(fpxxRepMini, &encode_data, 1);
		if (encode_data != NULL)
			inv_len = strlen((const char *)encode_data);
		if ((index + 8 + inv_len) > send_len)
		{
			FreeFpxx(fpxxRepMini);
			if (encode_data != NULL)
			{
				free(encode_data);
				encode_data = NULL;
			}
			sprintf(errinfo, "超过查询内存,请缩小查询范围");
			nRet = -10;
			goto Finish;
		}

		send_data[index + 0] = (((tzd_num) >> 24) & 0xff); //发票序号
		send_data[index + 1] = (((tzd_num) >> 16) & 0xff);
		send_data[index + 2] = (((tzd_num) >> 8) & 0xff);
		send_data[index + 3] = (((tzd_num) >> 0) & 0xff);
		if (encode_data != NULL)
			inv_len = strlen((const char *)encode_data);
		//_WriteLog(LL_INFO, "读取到第%d张有效红字信息表张数长度为%d", tzd_num, inv_len);
		send_data[index + 4] = ((inv_len >> 24) & 0xff); send_data[index + 5] = ((inv_len >> 16) & 0xff);//发票长度
		send_data[index + 6] = ((inv_len >> 8) & 0xff); send_data[index + 7] = ((inv_len >> 0) & 0xff);
		memcpy(send_data + 8 + index, encode_data, inv_len);
		if (encode_data != NULL)
		{
			free(encode_data);
			encode_data = NULL;
		}
		index += 8 + inv_len;

		FreeFpxx(fpxxRepMini);

		nRet = tzd_num;
		//_WriteLog(LL_INFO, "读取到有效红字信息表张数%d,发票代码：%s,发票号码：%s,处理完成！", tzd_num, fpxxRepMini->blueFpdm, fpxxRepMini->blueFphm);
		continue;
	}
	//_WriteLog(LL_INFO, "xml for 循环解析结束\n");
Finish:
	if (pXMLRoot)
		mxmlDelete(pXMLRoot);
	//_WriteLog(LL_INFO, "退出get_fpxx_from_hzxxb_data_aisino_nisec函数\n");
	return nRet;
}




int get_fpxx_from_hzxxb_data_cntax(HDEV hDev, char *szRep, uint8 *send_data, int send_len, char *errinfo)
{
	// 3201142004040539
	int tzd_num = 0;
	long index = 0;
	mxml_node_t *pXMLRoot = NULL, *node = NULL, *nodeResultChildSingle = NULL;
	int nRet = 0, i = 0, nCount = 0;
	char *pPayload = NULL, szBuf[512] = "";
	if ((nRet = LoadZZSXMLDataNodeTemplateCntax(&pXMLRoot, &node, "fhtjjls", szRep)) < 0) {
		_WriteLog(LL_FATAL, "LoadZZSXMLDataNodeTemplateCntax failed, Msg:,nRet:%d",  nRet);
		return -1;
	}
	if ((pPayload = (char *)mxmlGetOpaque(node)) == NULL) {
		nRet = -6;
		goto Finish;
	}
	nCount = atoi(pPayload);
	//_WriteLog(LL_FATAL, "nCount = %d\n", nCount);
	nodeResultChildSingle = node;
	// node1 node2 没用了可以被改为其他值
	for (i = 0; i < nCount; i++) {
		//需要两次，不清楚为什么
		//_WriteLog(LL_FATAL, "第 %d次解析\n", i);
		if ((nodeResultChildSingle = mxmlGetNextSibling(nodeResultChildSingle)) == NULL) {
			nRet = -5;
			goto Finish;
		}
		if ((nodeResultChildSingle = mxmlGetNextSibling(nodeResultChildSingle)) == NULL) {
			nRet = -6;
			goto Finish;
		}
		if ((node = mxmlFindElement(nodeResultChildSingle, pXMLRoot, "xxbbh", NULL, NULL,
			MXML_DESCEND)) == NULL)
			continue;
		if ((pPayload = (char *)mxmlGetOpaque(node)) == NULL)
			continue;
		//if (strcmp(szRedNum, pPayload))
		//	continue;  // not match
		if (GetChildNodeValue(nodeResultChildSingle, "xxbztdm", szBuf) < 0) {
			nRet = -7;
			goto Finish;
		}
		if (strcmp(szBuf, "TZD0082") == 0) {
			continue;
		}

		tzd_num += 1;
		//_WriteLog(LL_INFO, "读取到有效红字信息表张数%d", tzd_num);
		HFPXX fpxxRepMini = MallocFpxx();
		nRet = HZXMLNode2FPXXCntax(nodeResultChildSingle, fpxxRepMini);
		if (nRet < 0)
		{
			FreeFpxx(fpxxRepMini);
			continue;
		}
		_WriteLog(LL_INFO, "读取到有效红字信息表张数%d,发票代码：%s,发票号码：%s,通知单编号：%s", tzd_num, fpxxRepMini->blueFpdm, fpxxRepMini->blueFphm, fpxxRepMini->redNum);
		fpxxRepMini->hDev = hDev;
		uint8 *encode_data;
		int inv_len;
		fpxx_to_json_rednum_data_v102(fpxxRepMini, &encode_data, 1);
		if (encode_data != NULL)
			inv_len = strlen((const char *)encode_data);
		if ((index + 8 + inv_len) > send_len)
		{
			FreeFpxx(fpxxRepMini);
			if (encode_data != NULL)
			{
				free(encode_data);
				encode_data = NULL;
			}
			sprintf(errinfo, "超过查询内存,请缩小查询范围");
			nRet = -10;
			goto Finish;
		}

		send_data[index + 0] = (((tzd_num) >> 24) & 0xff); //发票序号
		send_data[index + 1] = (((tzd_num) >> 16) & 0xff);
		send_data[index + 2] = (((tzd_num) >> 8) & 0xff);
		send_data[index + 3] = (((tzd_num) >> 0) & 0xff);
		if (encode_data != NULL)
			inv_len = strlen((const char *)encode_data);
		//_WriteLog(LL_INFO, "读取到第%d张有效红字信息表张数长度为%d", tzd_num, inv_len);
		send_data[index + 4] = ((inv_len >> 24) & 0xff); send_data[index + 5] = ((inv_len >> 16) & 0xff);//发票长度
		send_data[index + 6] = ((inv_len >> 8) & 0xff); send_data[index + 7] = ((inv_len >> 0) & 0xff);
		memcpy(send_data + 8 + index, encode_data, inv_len);
		if (encode_data != NULL)
		{
			free(encode_data);
			encode_data = NULL;
		}
		index += 8 + inv_len;

		FreeFpxx(fpxxRepMini);

		nRet = tzd_num;
		//_WriteLog(LL_INFO, "读取到有效红字信息表张数%d,发票代码：%s,发票号码：%s,处理完成！", tzd_num, fpxxRepMini->blueFpdm, fpxxRepMini->blueFphm);
		continue;
	}
	//_WriteLog(LL_INFO, "xml for 循环解析结束\n");
Finish:
	if (pXMLRoot)
		mxmlDelete(pXMLRoot);
	//_WriteLog(LL_INFO, "退出get_fpxx_from_hzxxb_data_aisino_nisec函数\n");
	return nRet;
}


static int get_configure_file(char *path, char *in_data, char *out_data)
{
	//int configLen;
	int len;
	char tmp[256] = { 0 };
	char* linePos = NULL;
	char lineBuff[256] = { 0 };// 存储读取一行的数据
	
	memset(lineBuff, 0, 256);
	FILE* fp = fopen(path, "r");
	if (fp == NULL)
	{
		_WriteLog(LL_DEBUG, "文件打开失败\n");
		return -1;
	}
	fseek(fp, 0, SEEK_END);
	ftell(fp); //配置文件长度  
	fseek(fp, 0, SEEK_SET);
	while (fgets(lineBuff, 256, fp) != NULL)
	{	
		if ((lineBuff[0] == '#') || (lineBuff[0] == ';'))
			continue;
		linePos = NULL;
		linePos = strstr(lineBuff, in_data);
		if (linePos == NULL)
		{	
			memset(lineBuff, 0, sizeof(lineBuff));
			continue;
		}
		linePos += strlen(in_data);
		len = strlen(lineBuff) - strlen(in_data);
		memset(tmp, 0, sizeof(tmp));
		strncpy(tmp, linePos, len);
		char *p = strtok(tmp, "\r\n");
		if (p == NULL)
		{
			fclose(fp);
			return -1;
		}
		sprintf(out_data, "%s", p);
		fclose(fp);
		return 0;
	}
	_WriteLog(LL_DEBUG, "在配置文件中没找到\" %s \"数据\n", in_data);
	fclose(fp);
	return -1;
}






struct _http_comon
{
	char token[100];
	char timestamp[20];
	char appid[50];
	char appsecret[50];
	char s_ip[50];
	int s_port;
};

int tcp_get_line(int sock, char *buf, int size)
{
	int i = 0;
	unsigned char c = '\0';
	int n;
	//int j;

	/*把终止条件统一为 \n 换行符，标准化 buf 数组*/
	while ((i < size - 1) && (c != '\n'))
	{
		/*一次仅接收一个字节*/
		n = recv(sock, &c, 1, 0);
		/* DEBUG*/
		//printf("%02x ", c); 
		if (n > 0)
		{
			/*收到 \r 则继续接收下个字节，因为换行符可能是 \r\n */
			if (c == '\r')
			{
				/*使用 MSG_PEEK 标志使下一次读取依然可以得到这次读取的内容，可认为接收窗口不滑动*/
				n = recv(sock, &c, 1, MSG_PEEK);
				/* DEBUG printf("%02X\n", c); */
				//out("\n但如果是换行符则把它吸收掉\n");
				if ((n > 0) && (c == '\n'))
				{
					recv(sock, &c, 1, 0);
					//printf("%02x ", c); 
				}
				else
				{
					c = '\n';
					//printf("%02x ", c); 
				}
			}
			/*存到缓冲区*/
			buf[i] = c;
			i++;
		}
		else
			c = '\n';
	}
	//printf("\n")

	//out("接收到的一行数据：");
	//for(j=0;j<i;j++)
	//{
	//	printf("%02x ",(unsigned char)buf[j]);
	//}
	//printf("\n");
	//out("获取到的数据：%s\n",buf);
	/*返回 buf 数组大小*/
	return(i);
}

int tcp_deal_http_data(int sockfd, char **outbuf)
{
	char buf[1024] = { 0 };
	int i;
	//int j;
	char c;
	//char num=3;
	char len_buf[50] = { 0 };
	int numchars = 1;
	int content_length = -1;
	//out("获取第一行数据\n");
	memset(buf, 0, sizeof(buf));
	numchars = tcp_get_line(sockfd, buf, sizeof(buf));
	//out("获取到的第一行数据为：%s\n",buf);
	memset(buf, 0, sizeof(buf));
	numchars = tcp_get_line(sockfd, buf, sizeof(buf));
	//out("获取到的第二行数据为：%s\n",buf);
	while ((numchars > 0) && strcmp("\n", buf))
	{
		if (strncasecmp(buf, "Content-Length:", 15) == 0)
		{
			memset(len_buf, 0, sizeof(len_buf));
			memcpy(len_buf, &buf[15], 20);
			//out("len_buf = %s",len_buf);
			cls_no_asc(len_buf, strlen(len_buf));
			content_length = atoi(len_buf);
			//out("Content-Length = %d\n",content_length);
		}
		memset(buf, 0, sizeof(buf));
		numchars = tcp_get_line(sockfd, buf, sizeof(buf));
		//out("获取到的第%d行数据为：%s",num,buf);
		//num +=1;
		//for(j=0;j<numchars;j++)
		//{
		//	printf("%02x ",(unsigned char)buf[j]);
		//}
		//printf("\n");
	}
	/*没有找到 content_length */
	//out("退出获取头循环\n");
	if (content_length <= 0) {
		_WriteLog(LL_DEBUG, "错误请求,接收到的长度不对content_length = %d\n", content_length);
		return -1;
	}
	/*接收 POST 过来的数据*/
	//if(content_length>102400)
	//{
	//	out("超过缓存大小,无法接收\n");
	//	return -1;
	//}
	int glen = sizeof(char*)*content_length + 100;

	char *tmp = NULL;
	tmp = malloc(glen);
	if (tmp == NULL)
	{
		_WriteLog(LL_DEBUG, "分配接收内存失败\n");
		return -1;
	}

	memset(tmp, 0, glen);
	for (i = 0; i < content_length; i++)
	{
		recv(sockfd, &c, 1, 0);
		tmp[i] = c;
	}
	//out("HTTP data :%s\n",tmp);
	*outbuf = tmp;
	//memcpy(outbuf,tmp,sizeof(tmp));
	//out("TRAN return data :%s\n",*outbuf);
	return content_length;
}


static int tcp_socket_write(int sock, uint8 *buf, int buf_len)
{
	int i, result;
	for (i = 0; i<buf_len; i += result)
	{
		result = write(sock, buf + i, buf_len - i);
		if (result <= 0)
			return result;
	}
	return i;
}


static int tcp_open_rec_over_timer(int socketfd, int over_timer)
{
	struct timeval timeout;
	//out("设置接受超时时间为:%d\n",over_timer);
	memset(&timeout, 0, sizeof(timeout));
	timeout.tv_sec = over_timer;
	return setsockopt(socketfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
}

static int tcp_open_send_over_timer(int socketfd, int over_timer)
{
	struct timeval timeout;
	//out("设置发送超时时间为:%d\n",over_timer);
	memset(&timeout, 0, sizeof(timeout));
	timeout.tv_sec = over_timer;
	return setsockopt(socketfd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));

}

int tcp_http_netPost(char *hostname, int port, char *request, char *content_type, char *parmdata, int time_out, char **returndata, int *returndata_len)
{
	char name[300] = { 0 };
	int ip_len;
	int result;
	int socketfd;
	char s_buf[102400] = { 0 };
	char r_buf[102400] = { 0 };
	struct sockaddr_in add;
	memset(name, 0, sizeof(name));
	ip_len = GetDnsIp((char *)hostname, (char *)name);
	if (ip_len<0)
	{
		_WriteLog(LL_DEBUG, "解析IP失败\n");
		return -1;
	}
	//out("解析IP成功 IP为：%s\n",name);
	if ((socketfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		_WriteLog(LL_DEBUG, "创建socket失败\n");
		return -1;
	}
	memset(&add, 0, sizeof(add));
	add.sin_family = AF_INET;
	add.sin_port = htons(port);
	add.sin_addr.s_addr = htonl(INADDR_ANY);
	inet_pton(AF_INET, name, &add.sin_addr);
	//out("连接服务器:%s:%d\n",name,port);
	if ((result = connect(socketfd, (struct sockaddr *)&add, sizeof(add))) == -1)
	{
		_WriteLog(LL_DEBUG, "连接服务器%s:%d失败\n", name, port);
		close(socketfd);
		return -1;
	}
	tcp_open_rec_over_timer(socketfd, time_out);
	tcp_open_send_over_timer(socketfd, time_out);
	memset(s_buf, 0, sizeof(s_buf));
	memset(r_buf, 0, sizeof(r_buf));
	sprintf(s_buf, "POST %s HTTP/1.1\r\n", request);
	sprintf(s_buf + strlen(s_buf), "Content-Type: %s\r\n", content_type);
	sprintf(s_buf + strlen(s_buf), "Timeout: 5000\r\n");
	sprintf(s_buf + strlen(s_buf), "Host: %s\r\n", name);
	sprintf(s_buf + strlen(s_buf), "content-Length: %d\r\n", strlen(parmdata) + 2);
	sprintf(s_buf + strlen(s_buf), "User-Agent: libghttp/1.0\r\n");
	sprintf(s_buf + strlen(s_buf), "\r\n");
	sprintf(s_buf + strlen(s_buf), "%s\r\n", parmdata);


	//printf("http :%s\n",s_buf);
	if (tcp_socket_write(socketfd, (uint8 *)s_buf, strlen(s_buf))<0)
	{
		_WriteLog(LL_DEBUG, "发送失败\n");
		result = -2;
	}

	//memset(returndata,0,sizeof(returndata));
	//_WriteLog(LL_DEBUG, "进入获取数据函数\n");
	result = tcp_deal_http_data(socketfd, returndata);
	//out("退出接收处理函数\n");
	*returndata_len = result;


	//out("接收到数据：%s\n",returndata);
	shutdown(socketfd, SHUT_RDWR);
	close(socketfd);
	return result;
}

int get_appid_secert_token(struct _http_comon *parm)
{
	int result;
	char *returndata = NULL;
	int returndata_len;
	uint8 s_data[100] = { 0 };
	char request[500] = { 0 };
	cJSON *root = NULL, *item = NULL;
	//sprintf((char*)request, "/oauth_token?version=1.0&grant_type=password&username=%s&password=%s", appid, appsecert);
	sprintf((char*)request, "/was.oauth/oauth2?version=%s&grant_type=%s&appid=%s&appsecret=%s", "2.0.1.0", "client_credentials", parm->appid, parm->appsecret);

	//_WriteLog(LL_DEBUG, "resquest = %s\n", request);
	result = tcp_http_netPost(parm->s_ip, parm->s_port, request, "application/x-www-form-urlencoded", (char *)s_data, 5, &returndata, &returndata_len);
	if (result < 0)
	{
		if (returndata != NULL)
		{
			_WriteLog(LL_DEBUG, "释放http接收数据内存\n");
			free(returndata);
			returndata = NULL;
			_WriteLog(LL_DEBUG, "释放内存成功,指向空\n");
		}
		_WriteLog(LL_DEBUG, "获取token失败,http post error\n");
		return -1;
	}
	//_WriteLog(LL_DEBUG, "token获取到的是数据为%s\n", returndata);


	char res_result[10] = { 0 };
	root = cJSON_Parse((const char *)returndata);
	if (root == NULL)
	{
		printf("err recv ,is not a json !\n");
		free(returndata);
		return -2;
	}
	free(returndata);
	item = cJSON_GetObjectItem(root, "result");
	if (item == NULL)
	{
		printf("err recv token:json err2\n");
		cJSON_Delete(root);
		return -3;
	}
	memcpy(res_result, item->valuestring, strlen(item->valuestring));
	if (strcmp(res_result, "s") != 0)
	{
		char err[100] = { 0 };
		char msg[1000] = { 0 };
		char errinfo[1000] = { 0 };
		_WriteLog(LL_DEBUG, "服务器应答失败,解析错误描述\n");
		cJSON *object = cJSON_GetObjectItem(root, "response");
		item = cJSON_GetObjectItem(object, "err");
		if (get_json_value_can_not_null(item, err, 1, sizeof(err)) < 0)
		{
			_WriteLog(LL_DEBUG, "err recv token:json err3\n");
			cJSON_Delete(root);
			return -4;
		}
		item = cJSON_GetObjectItem(object, "msg");
		if (get_json_value_can_not_null(item, msg, 1, sizeof(msg)) < 0)
		{
			_WriteLog(LL_DEBUG, "err recv token:json err4\n");
			cJSON_Delete(root);
			return -5;
		}
		item = cJSON_GetObjectItem(object, "errinfo");
		if (get_json_value_can_not_null(item, errinfo, 1, sizeof(errinfo)) < 0)
		{
			_WriteLog(LL_DEBUG, "err recv token:json err5\n");
			cJSON_Delete(root);
			return -6;
		}
		_WriteLog(LL_DEBUG, " 服务器应答错误：");
		_WriteLog(LL_DEBUG, "err = %s,msg = %s,errinfo = %s\n", err, msg, errinfo);
	}
	else
	{
		cJSON *object = cJSON_GetObjectItem(root, "response");
		item = cJSON_GetObjectItem(object, "access_token");
		if (get_json_value_can_not_null(item, parm->token, 1, sizeof(parm->token)) < 0)
		{
			_WriteLog(LL_DEBUG, "err recv token:json err6\n");
			cJSON_Delete(root);
			return -7;
		}
	}

	cJSON_Delete(root);
	return 0;
}


int http_token_report_error_event(struct _http_comon *parm, char *s_data, int s_len)
{


	int result;
	char *returndata = NULL;
	int returndata_len;
	char request[500] = { 0 };
	cJSON *root = NULL, *item = NULL;
	char *sign_data;
	MD5_CTX ctx;
	//char *p = NULL;
	char md5_sha_sign[100] = { 0 };
	char outmd[16] = { 0 };
	char outmd_up[50] = { 0 };

	result = get_appid_secert_token(parm);
	if (result < 0)
	{
		_WriteLog(LL_DEBUG, "获取token失败\n");
		return -1;
	}
	//out("token 获取成功\n");
	sign_data = calloc(1, s_len + 1000);

	sprintf(sign_data, "access_token=%s&appid=%s&timestamp=%s%s", parm->token, parm->appid, parm->timestamp, s_data);

	MD5_Init(&ctx);
	MD5_Update(&ctx, sign_data, strlen((const char *)sign_data));
	MD5_Final((uint8 *)outmd, &ctx);

	//printf_array(outmd,16);

	//MD5加密完成
	//MD5加密结果进行转16进制字符串再转大写
	int i = 0;
	for (i = 0; i < 16; i++)
	{
		sprintf(outmd_up + i * 2, "%02X", (unsigned char)outmd[i]);
	}


	//out("outmd_up = %s\n", outmd_up);
	//HexToStr((char *)outmd_up, (char *)outmd, 16);
	//p = str2upper((char *)outmd_up);
	sprintf(md5_sha_sign, "%s", outmd_up);
	free(sign_data);





	sprintf((char*)request, "/was.middlesvr/app_err_logoper?appid=%s&timestamp=%s&sign=%s&access_token=%s", parm->appid, parm->timestamp, md5_sha_sign, parm->token);
	result = tcp_http_netPost(parm->s_ip, parm->s_port, request, "application/x-www-form-urlencoded", (char *)s_data, 5, &returndata, &returndata_len);
	if (result < 0)
	{
		if (returndata != NULL)
		{
			_WriteLog(LL_DEBUG, "释放http接收数据内存\n");
			free(returndata);
			returndata = NULL;
			_WriteLog(LL_DEBUG, "释放内存成功,指向空\n");
		}
		_WriteLog(LL_DEBUG, "获取http 数据失败\n");
		return -2;
	}
	//out("获取到的是数据为%s\n", returndata);
	char tmp[2048] = { 0 };
	//_gbk2utf8(returndata, tmp);

	//EncordingConvert("gbk", "utf-8", returndata, strlen(returndata), tmp, sizeof(tmp));
	EncordingConvert("utf-8", "gbk", returndata, strlen(returndata), tmp, sizeof(tmp));
	//_WriteLog(LL_DEBUG, "获取到的是数据为1%s\n", returndata);
	//_WriteLog(LL_DEBUG, "获取到的是数据为2%s\n", tmp);
	//printf("获取到的是数据为3%s\n", returndata);
	//printf("获取到的是数据为4%s\n", tmp);
	char res_result[10] = { 0 };
	root = cJSON_Parse((const char *)tmp);
	if (root == NULL)
	{
		_WriteLog(LL_DEBUG, "err recv ,is not a json !\n");
		free(returndata);
		return -3;
	}
	free(returndata);
	item = cJSON_GetObjectItem(root, "result");
	if (item == NULL)
	{
		_WriteLog(LL_DEBUG, "err recv token:json err2\n");
		cJSON_Delete(root);
		return -4;
	}
	memcpy(res_result, item->valuestring, strlen(item->valuestring));
	if (strcmp(res_result, "s") != 0)
	{
		char err[100] = { 0 };
		char msg[1000] = { 0 };
		char errinfo[1000] = { 0 };
		_WriteLog(LL_DEBUG, "服务器应答失败,解析错误描述\n");
		cJSON *object = cJSON_GetObjectItem(root, "response");
		item = cJSON_GetObjectItem(object, "err");
		if (get_json_value_can_not_null(item, err, 1, sizeof(err)) < 0)
		{
			_WriteLog(LL_DEBUG, "err recv token:json err3\n");
			cJSON_Delete(root);
			return -5;
		}
		item = cJSON_GetObjectItem(object, "msg");
		if (get_json_value_can_not_null(item, msg, 1, sizeof(msg)) < 0)
		{
			_WriteLog(LL_DEBUG, "err recv token:json err4\n");
			cJSON_Delete(root);
			return -6;
		}
		item = cJSON_GetObjectItem(object, "errinfo");
		if (get_json_value_can_not_null(item, errinfo, 1, sizeof(errinfo)) < 0)
		{
			_WriteLog(LL_DEBUG, "err recv token:json err5\n");
			cJSON_Delete(root);
			return -7;
		}
		//out(" 服务器应答错误：\n");
		cJSON_Delete(root);
		_WriteLog(LL_DEBUG, "err = %s,msg = %s,errinfo = %s\n", err, msg, errinfo);
		return -8;
		/*if (strcmp(errinfo, "未找到此机器ID生产计划") == 0)
		{
		_WriteLog(LL_DEBUG, "注册错误代码：未找到此机器ID生产计划\n");
		return -8;
		}
		else if (strcmp(errinfo, "已存在此注册机器信息") == 0)
		{
		_WriteLog(LL_DEBUG, "注册错误代码：已存在此注册机器信息\n");
		return -9;
		}
		else if (strcmp(errinfo, "注册失败") == 0)
		{
		_WriteLog(LL_DEBUG, "注册错误代码：注册失败\n");
		return -10;
		}
		else
		{
		_WriteLog(LL_DEBUG, "注册错误代码：未知错误代码\n");
		return -11;
		}*/
	}
	cJSON_Delete(root);
	return 0;
}

char last_errinfo[5000];

int report_event(char *plate_num, char *err, char *errinfo, int result)
{
	char ter_id[20] = { 0 };
	char ter_time[50] = { 0 };
	char detail_info[5000] = { 0 };
	//timer_read_asc(ter_time);
	int ret;
	timer_read_y_m_d_h_m_s(ter_time);

	get_configure_file("/etc/mqtt_tmp.conf", "mqtt_user = ", ter_id);
	//_WriteLog(LL_DEBUG, "【致命错误】 错误类型：[机柜错误],错误时间：[%s],错误标识：[%s],错误简要：[%s],错误信息：[机器编号：%s,税盘:%s,错误代码：%d,错误描述：%s]", ter_time, ter_id, err, ter_id, plate_num, result, errinfo);

	if (strlen(errinfo) > 4000)
	{
		_WriteLog(LL_DEBUG, "错误信息超长无法传出\n");
		return 0;
	}

	memset(detail_info, 0, sizeof(detail_info));
	sprintf(detail_info, "【错误信息】：机器编号：%s,税盘:%s,错误代码：%d,错误描述：%s", ter_id, plate_num, result, errinfo);

	if (strcmp(detail_info, last_errinfo) == 0)
	{
		//_WriteLog(LL_DEBUG, "与上次错误信息一致不再传出\n");
		return 0;
	}

	struct _http_comon parm;



	memset(&parm, 0, sizeof(struct _http_comon));
	sprintf(parm.s_ip, "%s", DF_ERR_SERVER_ADDRESS);
	parm.s_port = DF_ERR_SERVER_PORT;
	sprintf(parm.appid, "%s", DF_ERR_SERVER_APPID);
	sprintf(parm.appsecret, "%s", DF_ERR_SERVER_APPSECERT);
	sprintf((char*)parm.timestamp, "%ld", get_time_sec());

	cJSON *json = cJSON_CreateObject();
	cJSON_AddStringToObject(json, "runtype", "save");
	cJSON_AddStringToObject(json, "app_id", (char *)ter_id);
	cJSON_AddStringToObject(json, "app_type", "1");
	cJSON_AddStringToObject(json, "type_code", "0");
	cJSON_AddStringToObject(json, "brief", (char *)err);
	cJSON_AddStringToObject(json, "detail", (char *)detail_info);
	cJSON_AddStringToObject(json, "ctime", (char *)ter_time);
	char *g_buf;
	g_buf = cJSON_PrintUnformatted(json);
	//_WriteLog(LL_DEBUG, "注册查询组包数据：%s\n", g_buf);

	char utf_data[4096] = { 0 };
	EncordingConvert("gbk", "utf-8", g_buf, strlen(g_buf), utf_data, sizeof(utf_data));
	ret = http_token_report_error_event(&parm, utf_data, strlen(utf_data));
	free(g_buf);
	cJSON_Delete(json);
	//_WriteLog(LL_DEBUG, "异常上报完成,结果：%s\n", ret < 0? "失败" : "成功");
	if (ret == 0)
	{
		//_WriteLog(LL_DEBUG, "错误信息传出成功，记录上传错误传出信息\n");
		memset(last_errinfo, 0, sizeof(last_errinfo));
		memcpy(last_errinfo, detail_info, strlen(detail_info));
	}
	return 0;
}

int turn_month_to_range(char *month, char *date_range)
{
	int tmp_time[4] = { 0 };
	int date_start[4] = { 0 };
	int date_end[4] = { 0 };
	uint16 Year;

	memset(tmp_time, 0, sizeof(tmp_time));
	memset(date_start, 0, sizeof(date_start));
	memset(date_end, 0, sizeof(date_end));
	sscanf(month, "%02x%02x%02x", &tmp_time[0], &tmp_time[1], &tmp_time[2]);

	date_start[0] = tmp_time[0];
	date_start[1] = tmp_time[1];
	date_start[2] = tmp_time[2];
	date_start[3] = 0x01;

	date_end[0] = tmp_time[0];
	date_end[1] = tmp_time[1];
	date_end[2] = tmp_time[2];
	if ((tmp_time[2] == 0x04) || (tmp_time[2] == 0x06) || (tmp_time[2] == 0x09) || (tmp_time[2] == 0x11))
		date_end[3] = 0x30;
	else if ((tmp_time[2] == 0x01) || (tmp_time[2] == 0x03) || (tmp_time[2] == 0x05) || (tmp_time[2] == 0x07) || (tmp_time[2] == 0x08) || (tmp_time[2] == 0x10) || (tmp_time[2] == 0x12))
		date_end[3] = 0x31;
	else if (tmp_time[2] == 0x02)
	{
		char year_tmp[50] = { 0 };
		memcpy(year_tmp, month, 4);
		Year = atoi(year_tmp);
		if ((Year & 0x03) != 0)
			date_end[3] = 0x28;
		else
			date_end[3] = 0x29;
	}

	sprintf(date_range, "%02x%02x%02x%02x-%02x%02x%02x%02x", date_start[0], date_start[1], date_start[2], date_start[3], date_end[0], date_end[1], date_end[2], date_end[3]);
	//sprintf(date_range, "20190423-20190424");
	//out("date_range %s\n", date_range);
	if (17 != strlen(date_range)) {
		printf("[-] Query date range format error, example:20190101-20190131\n");
		return -2;
	}
	return 0;
}

int turn_month_to_date_range(char *month, char *time_now,char *start_date,char *end_date)
{
	int tmp_time[4] = { 0 };
	int date_start[4] = { 0 };
	int date_end[4] = { 0 };
	uint16 Year;

	memset(tmp_time, 0, sizeof(tmp_time));
	memset(date_start, 0, sizeof(date_start));
	memset(date_end, 0, sizeof(date_end));
	sscanf(month, "%02x%02x%02x", &tmp_time[0], &tmp_time[1], &tmp_time[2]);

	date_start[0] = tmp_time[0];
	date_start[1] = tmp_time[1];
	date_start[2] = tmp_time[2];
	date_start[3] = 0x01;

	date_end[0] = tmp_time[0];
	date_end[1] = tmp_time[1];
	date_end[2] = tmp_time[2];
	if ((tmp_time[2] == 0x04) || (tmp_time[2] == 0x06) || (tmp_time[2] == 0x09) || (tmp_time[2] == 0x11))
		date_end[3] = 0x30;
	else if ((tmp_time[2] == 0x01) || (tmp_time[2] == 0x03) || (tmp_time[2] == 0x05) || (tmp_time[2] == 0x07) || (tmp_time[2] == 0x08) || (tmp_time[2] == 0x10) || (tmp_time[2] == 0x12))
		date_end[3] = 0x31;
	else if (tmp_time[2] == 0x02)//如果是2月份
	{
		char year_tmp[50] = {0};
		memcpy(year_tmp, month,4);
		Year = atoi(year_tmp);
		if ((Year & 0x03) != 0)
			date_end[3] = 0x28;
		else
			date_end[3] = 0x29;
	}

	sprintf(start_date, "%02x%02x%02x%02x", date_start[0], date_start[1], date_start[2], date_start[3]);
	if (memcmp(month, time_now, 6) == 0)
	{
		memcpy(end_date, time_now, 8);
	}
	else
	{
		sprintf(end_date, "%02x%02x%02x%02x", date_end[0], date_end[1], date_end[2], date_end[3]);
	}
	//sprintf(date_range, "20190423-20190424");
	//out("date_range %s\n", date_range);
	if ((8 != strlen(end_date)) && ((8 != strlen(start_date))))
	{
		printf("[-] Query date range format error, example:2019010120190131\n");
		return -2;
	}
	return 0;
}

int http_token_get_invs_sync(struct _http_comon *parm, char *s_data, int s_len,char **invs_sync)
{


	int result;
	char *returndata = NULL;
	int returndata_len;
	char request[500] = { 0 };
	cJSON *root = NULL, *item = NULL;
	char *sign_data;
	MD5_CTX ctx;
	//char *p = NULL;
	char md5_sha_sign[100] = { 0 };
	char outmd[16] = { 0 };
	char outmd_up[50] = { 0 };

	result = get_appid_secert_token(parm);
	if (result < 0)
	{
		_WriteLog(LL_DEBUG, "获取token失败\n");
		return -1;
	}
	//out("token 获取成功\n");
	sign_data = calloc(1, s_len + 1000);

	sprintf(sign_data, "access_token=%s&appid=%s&timestamp=%s%s", parm->token, parm->appid, parm->timestamp, s_data);

	MD5_Init(&ctx);
	MD5_Update(&ctx, sign_data, strlen((const char *)sign_data));
	MD5_Final((uint8 *)outmd, &ctx);

	//printf_array(outmd,16);

	//MD5加密完成
	//MD5加密结果进行转16进制字符串再转大写
	int i = 0;
	for (i = 0; i < 16; i++)
	{
		sprintf(outmd_up + i * 2, "%02X", (unsigned char)outmd[i]);
	}


	sprintf(md5_sha_sign, "%s", outmd_up);
	free(sign_data);





	sprintf((char*)request, "/was.middlesvr/tax_month_inv_now_info?appid=%s&timestamp=%s&sign=%s&access_token=%s", parm->appid, parm->timestamp, md5_sha_sign, parm->token);
	result = tcp_http_netPost(parm->s_ip, parm->s_port, request, "application/x-www-form-urlencoded", (char *)s_data, 5, &returndata, &returndata_len);
	if (result < 0)
	{
		if (returndata != NULL)
		{
			_WriteLog(LL_DEBUG, "释放http接收数据内存\n");
			free(returndata);
			returndata = NULL;
			_WriteLog(LL_DEBUG, "释放内存成功,指向空\n");
		}
		_WriteLog(LL_DEBUG, "获取http 数据失败\n");
		return -2;
	}
	//_WriteLog(LL_DEBUG, "获取到的http数据：%s\n", returndata);

	char res_result[10] = { 0 };
	root = cJSON_Parse((const char *)returndata);
	if (root == NULL)
	{
		_WriteLog(LL_DEBUG, "err recv ,is not a json !\n");
		free(returndata);
		return -3;
	}
	free(returndata);
	item = cJSON_GetObjectItem(root, "result");
	if (item == NULL)
	{
		_WriteLog(LL_DEBUG, "err recv token:json err2\n");
		cJSON_Delete(root);
		return -4;
	}
	memcpy(res_result, item->valuestring, strlen(item->valuestring));
	if (strcmp(res_result, "s") == 0)
	{
		char compress_len_asc[10] = { 0 };
		char uncompress_len_asc[10] = { 0 };
		
		cJSON *object = cJSON_GetObjectItem(root, "response");
		item = cJSON_GetObjectItem(object, "datalen");
		if (get_json_value_can_not_null(item, uncompress_len_asc, 1, sizeof(uncompress_len_asc)) < 0)
		{
			_WriteLog(LL_DEBUG, "err recv token:json err3\n");
			cJSON_Delete(root);
			return -5;
		}
		item = cJSON_GetObjectItem(object, "baselen");
		if (get_json_value_can_not_null(item, compress_len_asc, 1, sizeof(compress_len_asc)) < 0)
		{
			_WriteLog(LL_DEBUG, "err recv token:json err4\n");
			cJSON_Delete(root);
			return -6;
		}
		char *compress_data = NULL;
		char *uncompress_data = NULL;
		long compress_len = atoll(compress_len_asc) + 5000;
		long uncompress_len = atoll(uncompress_len_asc) + 5000;
		compress_data = malloc(compress_len);
		if (compress_data == NULL)
		{
			_WriteLog(LL_DEBUG, "内存申请失败\n");
			cJSON_Delete(root);
			return -7;
		}
		item = cJSON_GetObjectItem(object, "data");
		if (get_json_value_can_not_null(item, compress_data, 1, compress_len) < 0)
		{
			_WriteLog(LL_DEBUG, "err recv token:json err5\n");
			cJSON_Delete(root);
			free(compress_data);
			return -8;
		}
		uncompress_data = malloc(uncompress_len);
		if (uncompress_data == NULL)
		{
			_WriteLog(LL_DEBUG, "内存申请失败\n");
			cJSON_Delete(root);
			free(compress_data);
			return -9;
		}
		int invs_data_len = uncompress_len;
		result = uncompress_asc_base64((const uint8 *)compress_data, strlen((const char *)compress_data), (uint8 *)uncompress_data, &invs_data_len);
		if (result < 0)
		{
			_WriteLog(LL_DEBUG, "内存申请失败\n");
			cJSON_Delete(root);
			free(compress_data);
			free(uncompress_data);
			return -10;
		}
		//_WriteLog(LL_DEBUG, "获取到的未压缩数据：%s\n", uncompress_data);
		free(compress_data);
		*invs_sync = uncompress_data;

		//out(" 服务器应答错误：\n");
		cJSON_Delete(root);
		return invs_data_len;
	}
	//_WriteLog(LL_DEBUG, "json数据解析返回异常\n");
	cJSON_Delete(root);
	return -100;

}


int get_plate_invs_sync(char *plate_num, char ** invs_data)
{
	char ter_id[20] = { 0 };
	char ter_time[50] = { 0 };
	//char detail_info[5000] = { 0 };
	//timer_read_asc(ter_time);
	int ret;
	timer_read_y_m_d_h_m_s(ter_time);

	get_configure_file("/etc/mqtt_tmp.conf", "mqtt_user = ", ter_id);
	//_WriteLog(LL_DEBUG, "【致命错误】 错误类型：[机柜错误],错误时间：[%s],错误标识：[%s],错误简要：[%s],错误信息：[机器编号：%s,税盘:%s,错误代码：%d,错误描述：%s]", ter_time, ter_id, err, ter_id, plate_num, result, errinfo);




	struct _http_comon parm;
	memset(&parm, 0, sizeof(struct _http_comon));
	sprintf(parm.s_ip, "%s", DF_ERR_SERVER_ADDRESS);//正式
	//sprintf(parm.s_ip, "%s", "103.45.249.88");//测试
	parm.s_port = DF_ERR_SERVER_PORT;
	sprintf(parm.appid, "%s", DF_ERR_SERVER_APPID);
	sprintf(parm.appsecret, "%s", DF_ERR_SERVER_APPSECERT);
	sprintf((char*)parm.timestamp, "%ld", get_time_sec());

	cJSON *json = cJSON_CreateObject();	
	cJSON_AddStringToObject(json, "compress", "0");
	cJSON_AddStringToObject(json, "plate_num", plate_num);
	char *g_buf;
	g_buf = cJSON_PrintUnformatted(json);
	//_WriteLog(LL_DEBUG, "注册查询组包数据：%s\n", g_buf);

	char utf_data[4096] = { 0 };
	EncordingConvert("gbk", "utf-8", g_buf, strlen(g_buf), utf_data, sizeof(utf_data));
	ret = http_token_get_invs_sync(&parm, utf_data, strlen(utf_data), invs_data);
	free(g_buf);
	cJSON_Delete(json);

	return ret;
}

// fpxx need init by caller, free by caller
int get_tzdbh_from_hzxxb_data_by_fpdmhm(HFPXX fpxx, char *szRep)
{
	// 3201142004040539
	//int tzd_num = 0;
	//long index = 0;
	mxml_node_t *pXMLRoot = NULL, *node = NULL, *nodeResultChildSingle = NULL;
	int nRet = 0, i = 0, nCount = 0;
	char *pPayload = NULL, szBuf[512] = "";
	char xml_bluefpdm[50] = { 0 };
	char xml_bluefphm[50] = { 0 };
	if (LoadZZSXMLDataNodeTemplate(&pXMLRoot, &node, "ALLCOUNT", szRep) < 0) {
		_WriteLog(LL_FATAL, "LoadZZSXMLDataNodeTemplate failed, Msg:%s", szRep);
		return -1;
	}
	if ((pPayload = (char *)mxmlGetOpaque(node)) == NULL) {
		nRet = -6;
		goto Finish;
	}
	nCount = atoi(pPayload);
	//_WriteLog(LL_FATAL, "nCount = %d\n", nCount);
	nodeResultChildSingle = node;
	// node1 node2 没用了可以被改为其他值
	for (i = 0; i < nCount; i++) {
		//需要两次，不清楚为什么
		//_WriteLog(LL_FATAL, "第 %d次解析\n", i);
		if ((nodeResultChildSingle = mxmlGetNextSibling(nodeResultChildSingle)) == NULL) {
			nRet = -5;
			goto Finish;
		}
		if ((nodeResultChildSingle = mxmlGetNextSibling(nodeResultChildSingle)) == NULL) {
			nRet = -6;
			goto Finish;
		}
		if ((node = mxmlFindElement(nodeResultChildSingle, pXMLRoot, "ResBillNo", NULL, NULL,
			MXML_DESCEND)) == NULL)
			continue;
		if ((pPayload = (char *)mxmlGetOpaque(node)) == NULL)
			continue;
		//if (strcmp(szRedNum, pPayload))
		//	continue;  // not match
		if (GetChildNodeValue(nodeResultChildSingle, "StatusDM", szBuf) < 0) {
			nRet = -7;
			goto Finish;
		}
		if (strcmp(szBuf, "TZD0000") != 0)
			continue;
		if (GetChildNodeValue(nodeResultChildSingle, "TypeCode", xml_bluefpdm) < 0) {
			nRet = -8;
			goto Finish;
		}
		if (GetChildNodeValue(nodeResultChildSingle, "InvNo", xml_bluefphm) < 0) {
			nRet = -9;
			goto Finish;
		}

		if ((strcmp(xml_bluefpdm, fpxx->blueFpdm) == 0) && (strcmp(xml_bluefphm, fpxx->blueFphm) == 0))
		{
			if (GetChildNodeValue(nodeResultChildSingle, "ReqBillNo", (char *)fpxx->redNum_serial) < 0) {
				nRet = -10;
				goto Finish;
			}
			if (strlen((char *)fpxx->redNum_serial)>0)
			{
				_WriteLog(LL_INFO, "获取到的信息表流水号%s\n", fpxx->redNum_serial);
			}

			if (GetChildNodeValue(nodeResultChildSingle, "ResBillNo", (char *)fpxx->redNum) < 0) {
				nRet = -11;
				goto Finish;
			}
			if (strlen((char *)fpxx->redNum)>0)
			{
				_WriteLog(LL_INFO, "获取到的信息表编号为%s\n", fpxx->redNum);
				nRet = 0;
				goto Finish;
			}

			

		}
		continue;
	}
	//_WriteLog(LL_INFO, "xml for 循环解析结束\n");
Finish:
	if (pXMLRoot)
		mxmlDelete(pXMLRoot);
	//_WriteLog(LL_INFO, "退出get_fpxx_from_hzxxb_data_aisino_nisec函数\n");
	return nRet;
}



int RedInvOnlineCheck_TZD(HFPXX fpxx,uint8 **red_inv)
{
	int result;
	char time_end[50] = { 0 };
	char time_start[50] = { 0 };
	char now_month[50] = { 0 };
	char date_range[100] = { 0 };
	memcpy(time_end, fpxx->hDev->szDeviceTime, 8);
	memcpy(now_month, fpxx->hDev->szDeviceTime, 6);
	mb_get_last_month_first_day(now_month, time_start);
	sprintf(date_range, "%s-%s", time_start, time_end);

	char *szBuf = calloc(1, DEF_MAX_FPBUF_LEN);
	result = DownloadTZDBH(fpxx->hDev, date_range, "", "", szBuf, DEF_MAX_FPBUF_LEN);
	if (result < 0)
	{
		free(szBuf);
		return result;
	}
	result = get_tzdbh_from_hzxxb_data_by_fpdmhm(fpxx, szBuf);
	if (result < 0)
	{
		free(szBuf);
		return result;
	}

	HFPXX fpxxRepMini = MallocFpxx();

	if ((fpxx->hDev->bDeviceType == DEVICE_AISINO) || (fpxx->hDev->bDeviceType == DEVICE_NISEC))
	{
		if (GetFpxxFromRepByRedNum(szBuf, fpxx->redNum, fpxxRepMini) < 0) {
			_WriteLog(LL_FATAL, "[-] GetFpxxFromRepByRedNum failed\r\n");
			FreeFpxx(fpxxRepMini);
			free(szBuf);
			return -2;
		}
	}
	else if (fpxx->hDev->bDeviceType == DEVICE_CNTAX)
	{
		if (GetFpxxFromRepByRedNumCntax(szBuf, fpxx->redNum, fpxxRepMini) < 0) {
			_WriteLog(LL_FATAL, "[-] GetFpxxFromRepByRedNum failed\r\n");
			FreeFpxx(fpxxRepMini);
			free(szBuf);
			return -2;
		}
	}

	fpxxRepMini->hDev = fpxx->hDev;
	fpxx_to_json_rednum_data_v102(fpxxRepMini, red_inv, 1);
	
	FreeFpxx(fpxxRepMini);
	free(szBuf);
	return result;
}



void Test_TAConnect(HDEV hDev)
{
	struct HTTP hi;
	//FillDevInfoCert(hDev, 1);
	LoadNisecDevInfo(hDev, hDev->hUSB, true);
	memset(&hi, 0, sizeof(struct HTTP));
	int nRet = -1;
	if ((nRet = CreateServerTalk(&hi, hDev)) < 0) {
		_WriteLog(LL_INFO, "TestTAConnect connect failed, nRet:%d", nRet);
		HTTPClose(&hi);
		return;
	}
	_WriteLog(LL_INFO, "TestTAConnect connect successful");
	HTTPClose(&hi);
}


int AisinoNetInvoiceQueryReadyEasy(HHTTP hi, HDEV hDev, char *szInputQueryDateRange, char *szOutFormatStrMultiLine)
{
	int nRet = -1, nChildRet = -1;
	if ((nChildRet = QueryNetInvoiceReady(hi, szInputQueryDateRange, szOutFormatStrMultiLine)) <
		0) {
		_WriteLog(LL_FATAL, "NetInvoice_QueryReady, IO failed");
		goto FreeAndExit;
	}
	if (!strlen(szOutFormatStrMultiLine)) {
		SetLastError(hDev->hUSB, ERR_TA_REPONSE_FAILE,"没有查询到可以领取的发票");
		_WriteLog(LL_FATAL, "NetInvoice_QueryReady, not found avail invoice to download");
		goto FreeAndExit;
	}
	//  _WriteLog(LL_DEBUG, "%s", szOutFormatStrMultiLine);
	nRet = 0;
FreeAndExit:
	return nRet;
}
int NisecNetInvoiceQueryReadyEasy(HHTTP hi, HDEV hDev, char *szOutFormatStrMultiLine)
{
	int nRet = -1, nChildRet = -1;
	if ((nChildRet = NisecQueryNetInvoiceReady(0, hi, szOutFormatStrMultiLine)) < 0) {
		_WriteLog(LL_FATAL, "NetInvoice_QueryReady, IO failed");
		goto FreeAndExit;
	}
	if (!strlen(szOutFormatStrMultiLine)) {
		SetLastError(hDev->hUSB, ERR_TA_REPONSE_FAILE,"没有查询到可以领取的发票");
		_WriteLog(LL_FATAL, "NetInvoice_QueryReady, not found avail invoice to download");
		goto FreeAndExit;
	}
	nRet = 0;
FreeAndExit:
	return nRet;
}

int CntaxNetInvoiceQueryReadyEasy(HHTTP hi, HDEV hDev, char *szOutFormatStrMultiLine)
{
	int nRet = -1, nChildRet = -1;
	if ((nChildRet = CntaxQueryNetInvoiceReady(0, hi, szOutFormatStrMultiLine)) < 0) {
		_WriteLog(LL_FATAL, "CntaxNetInvoiceQueryReady, IO failed,nChildRet = %d", nChildRet);
		goto FreeAndExit;
	}
	if (!strlen(szOutFormatStrMultiLine)) {
		SetLastError(hDev->hUSB, ERR_TA_REPONSE_FAILE, "没有查询到可以领取的发票");
		_WriteLog(LL_FATAL, "CntaxNetInvoiceQueryReady, not found avail invoice to download");
		goto FreeAndExit;
	}
	nRet = 0;
FreeAndExit:
	return nRet;
}

int AisinoNetInvoiceDownloadUnlockEasy(HHTTP hi, HDEV hDev, char *szInputFormatStrSingleLine)
{
	// #发票下载主流程
	// Aisino.Fwkp.Fplygl.Controller.WebDownload.WebDownloadController.ExecuteDownloadList(List<InvVolumeApp>,
	// string)
	//
	// #发票领用XML构造
	// Aisino.Fwkp.Fplygl.Controller.WebDownload.WebDownloadController.RequestDownloadInput(InvVolumeApp,
	// InvVolumeApp, string, int, string, bool)
	int nRet = -1, nChildRet = -1;
	while (1) {
		//清除之前可能下载了，但是尚未解锁的；否则无法进入下一步
		if ((nChildRet = ClearLockedInvoiceVolume(hi)) < 0) {
			SetLastError(hDev->hUSB, ERR_LOGIC_CLEAR_LOCKED_INVOICE, "安装历史发票不成功");
			_WriteLog(LL_FATAL, "NetInvoice_DownloadUnlock, failed 1,nChildRet = %d", nChildRet);
			break;
		}
		//下载指定的
		if ((nChildRet = DownloadSpecifyInvoice(hi, szInputFormatStrSingleLine)) < 0) {
			_WriteLog(LL_FATAL, "NetInvoice_DownloadUnlock, failed 2,nChildRet = %d", nChildRet);
			break;
		}
		//解锁刚下载的
		if ((nChildRet = ClearLockedInvoiceVolume(hi)) < 0) {
			SetLastError(hDev->hUSB, ERR_LOGIC_CLEAR_LOCKED_INVOICE, "安装发票失败");
			_WriteLog(LL_FATAL, "NetInvoice_DownloadUnlock, failed 3,nChildRet = %d", nChildRet);
			break;
		}
		nRet = 0;
		break;
	}
	return nRet;
}

int NisecNetInvoiceDownloadWriteEasy(HHTTP hi,HDEV hDev, char *szInputFormatSingleLine)
{
	char szOutMW[512] = "";
	int nRet = -1, nChildRet = -1;


	if (strlen(szInputFormatSingleLine) == 0) {
		//查询税局尙存在，但是实际盘中发票已写入并且可开票，尝试同步并删除局端信息
		if ((nChildRet = UnlockReadyInvoiceInDevice(hi) < 0)) {
			_WriteLog(LL_FATAL, "UnlockReadyInvoiceInDevice failed");
		}
		else {
			nRet = 0;
		}
		goto FreeAndExit;
	}
	if (strlen(szInputFormatSingleLine) > sizeof(szOutMW)) {
		return -1;
	}
	strcpy(szOutMW, szInputFormatSingleLine);
	if ((nChildRet = NisecQueryNetInvoiceReady(1, hi, szOutMW)) < 0) {
		_WriteLog(LL_FATAL, "NisecQueryNetInvoiceReady failed");
		goto FreeAndExit;
	}
	// open device
	if (NisecEntry(hDev->hUSB, NOP_OPEN_FULL) < 0) {
		goto FreeAndExit;
	}
	NisecSelectInvoiceType(hDev, hDev->bOperateInvType, SELECT_OPTION_TEMPLATE);
	nChildRet = WriteGpxxmwToDevice(hDev, szOutMW, szInputFormatSingleLine);
	NisecEntry(hDev->hUSB, NOP_CLOSE);
	if (nChildRet < 0) {
		_WriteLog(LL_FATAL, "WriteGpxxmwToDevice failed,last errinfo = %s", hDev->hUSB->errinfo);
		goto FreeAndExit;
	}
	_WriteLog(LL_INFO, "Invoice info has writed to device, notify TA now...");
	if ((nChildRet = NisecQueryNetInvoiceReady(2, hi, szInputFormatSingleLine)) < 0) {
		_WriteLog(LL_FATAL, "Gpxx write ok, notify TA failed");
		goto FreeAndExit;
	}
	_WriteLog(LL_INFO, "Nisec buy invoice finish");
	nRet = 0;
FreeAndExit:
	return nRet;
}

int CntaxNetInvoiceDownloadWriteEasy(HHTTP hi, HDEV hDev, char *szInputFormatSingleLine)
{
	char szOutMW[512] = "";
	int nRet = -1, nChildRet = -1;
	do {
		if (strlen(szInputFormatSingleLine) == 0) {
			//查询税局尙存在，但是实际盘中发票已写入并且可开票，尝试同步并删除局端信息
			if ((nChildRet = CntaxUnlockReadyInvoiceInDevice(hi) < 0)) {
				_WriteLog(LL_FATAL, "UnlockReadyInvoiceInDevice failed");
			}
			else {
				nRet = 0;
			}
			break;
		}
		if (strlen(szInputFormatSingleLine) > sizeof(szOutMW))
			break;
		strcpy(szOutMW, szInputFormatSingleLine);
		if ((nChildRet = CntaxQueryNetInvoiceReady(1, hi, szOutMW)) < 0) {
			_WriteLog(LL_FATAL, "QueryNetInvoiceReady failed");
			break;
		}
		// open device
		if (CntaxEntry(hDev->hUSB, NOP_OPEN_FULL) < 0)
			break;
		CntaxSelectInvoiceType(hDev, hDev->bOperateInvType, SELECT_OPTION_TEMPLATE);
		nChildRet = CntaxWriteGpxxmwToDevice(hDev, szOutMW, szInputFormatSingleLine);
		CntaxEntry(hDev->hUSB, NOP_CLOSE);
		if (nChildRet < 0) {
			_WriteLog(LL_FATAL, "WriteGpxxmwToDevice failed");
			break;
		}
		_WriteLog(LL_INFO, "Invoice info has writed to device, notify TA now...");
		if ((nChildRet = CntaxQueryNetInvoiceReady(2, hi, szInputFormatSingleLine)) < 0) {
			_WriteLog(LL_FATAL, "Gpxx write ok, notify TA failed");
			break;
		}
		_WriteLog(LL_INFO, "CntaxNetInvoiceDownloadWrite finish");
		nRet = 0;
	} while (false);

//FreeAndExit:
	return nRet;
}

int send_zip_data_hook(HDEV hDev, char *month, void *function, void *arg, int need_up_count, zipFile zf, char * fp_zip_name, int zip_data_len, int is_over)
{
	int result;
	void *hook_arg;
	int(*hook)(void *hook_arg, char *s_month, char *s_data, int inv_sum);
	hook = function;
	hook_arg = arg;

	if (need_up_count == 0)
	{
		result = hook(hook_arg, (char *)month, NULL, DF_UPLOAD_INV);
		return result;
	}

	zipClose(zf, NULL);
	unsigned char *zip_data;

	zip_data = (uint8 *)calloc(1, zip_data_len + 1024);
	int zip_file_size = read_file(fp_zip_name, (char *)zip_data, zip_data_len);
	_WriteLog(LL_INFO, "读取zip文件大小为%d字节", zip_file_size);
	delete_file(fp_zip_name);//文件一定要删除，否则会导致zf句柄重复释放问题
	char *base_buf = calloc(1, zip_file_size * 2 + 1024);
	EVP_EncodeBlock((uint8 *)base_buf, (const uint8 *)zip_data, zip_file_size);
	free(zip_data);

	result = hook(hook_arg, (char *)month, base_buf, DF_UPLOAD_INV);
	free(base_buf);
	if (result < 0)
	{
		//free(pInvoiceBuff);
		//_WriteLog(LL_INFO, "%s税盘回调同步发送数据超时\n", hDev->szCompanyName);
		return 	DF_TAX_ERR_CODE_HOOK_SERVER_TIME_OUT;
	}
	if (is_over == 1)
	{
		_WriteLog(LL_INFO, "[%s],%s月,所有发票全部读取完成,数据发送完成\n", hDev->szCompanyName, month);
	}
	else
	{
		_WriteLog(LL_INFO, "[%s],%s月,发票读取压缩超过1M,数据发送完成\n", hDev->szCompanyName, month);
	}
	return 0;
}

static int deal_fplx_statistics_to_now_month_sum(char *fplx_data_base, struct _inv_type_sum_data  *type_sum)
{
	char *fplx_data_json;
	int fplx_data_json_len;
	cJSON	*item;
	if (fplx_data_base == NULL)
	{
		return -1;
	}
	fplx_data_json_len = strlen(fplx_data_base) + 1024;
	fplx_data_json = calloc(1, fplx_data_json_len);
	if (fplx_data_json == NULL)
	{
		return -2;
	}
	Base64_Decode(fplx_data_base, strlen(fplx_data_base), fplx_data_json);
	//_WriteLog(LL_INFO, "%s", fplx_data_json);

	cJSON *root = cJSON_Parse(fplx_data_json);
	if (!root)
	{
		free(fplx_data_json);
		return -3;
	}
	free(fplx_data_json);


	char qckcfs[10] = { 0 };				//期初库存
	char lgfpfs[10] = { 0 };				//领购发票份数
	char thfpfs[10] = { 0 };				//退回发票份数
	char qmkcfs[10] = { 0 };				//期末库存份数
	char zsfpfs[20] = { 0 };				//正数发票份数
	char zffpfs[20] = { 0 };				//正废发票份数
	char fsfpfs[20] = { 0 };				//负数发票份数
	char fffpfs[20] = { 0 };				//负废发票份数
	char kffpfs[20] = { 0 };				//空废发票份数
	char zsfpljje[20] = { 0 };	//正数发票累计金额
	char zsfpljse[20] = { 0 };	//正数发票累计税额
	char zffpljje[20] = { 0 };	//正废发票累计金额
	char zffpljse[20] = { 0 };	//正废发票累计税额
	char fsfpljje[20] = { 0 };	//负数发票累计金额
	char fsfpljse[20] = { 0 };	//负数发票累计税额
	char fffpljje[20] = { 0 };	//负废发票累计金额
	char fffpljse[20] = { 0 };	//负废发票累计税额
	char sjxsje[20] = { 0 };		//实际销售金额
	char sjxsse[20] = { 0 };		//实际销售税额



	item = cJSON_GetObjectItem(root, "qckcfs");
	if (get_json_value_can_not_null(item, (char *)qckcfs, 0, sizeof(qckcfs)) < 0)
	{
		cJSON_Delete(root);
		return -4;
	}
	type_sum->qckcfs = atoi(qckcfs);


	item = cJSON_GetObjectItem(root, "lgfpfs");
	if (get_json_value_can_not_null(item, (char *)lgfpfs, 0, sizeof(lgfpfs)) < 0)
	{
		cJSON_Delete(root);
		return -4;
	}
	type_sum->lgfpfs = atoi(lgfpfs);


	item = cJSON_GetObjectItem(root, "thfpfs");
	if (get_json_value_can_not_null(item, (char *)thfpfs, 0, sizeof(thfpfs)) < 0)
	{
		cJSON_Delete(root);
		return -4;
	}
	type_sum->thfpfs = atoi(thfpfs);


	item = cJSON_GetObjectItem(root, "qmkcfs");
	if (get_json_value_can_not_null(item, (char *)qmkcfs, 0, sizeof(qmkcfs)) < 0)
	{
		cJSON_Delete(root);
		return -4;
	}
	type_sum->qmkcfs = atoi(qmkcfs);

	item = cJSON_GetObjectItem(root, "zsfpfs");
	if (get_json_value_can_not_null(item, (char *)zsfpfs, 0, sizeof(zsfpfs)) < 0)
	{
		cJSON_Delete(root);
		return -4;
	}
	type_sum->zsfpfs = atoi(zsfpfs);

	item = cJSON_GetObjectItem(root, "zffpfs");
	if (get_json_value_can_not_null(item, (char *)zffpfs, 0, sizeof(zffpfs)) < 0)
	{
		cJSON_Delete(root);
		return -4;
	}
	type_sum->zffpfs = atoi(zffpfs);

	item = cJSON_GetObjectItem(root, "fsfpfs");
	if (get_json_value_can_not_null(item, (char *)fsfpfs, 0, sizeof(fsfpfs)) < 0)
	{
		cJSON_Delete(root);
		return -4;
	}
	type_sum->fsfpfs = atoi(fsfpfs);

	item = cJSON_GetObjectItem(root, "fffpfs");
	if (get_json_value_can_not_null(item, (char *)fffpfs, 0, sizeof(fffpfs)) < 0)
	{
		cJSON_Delete(root);
		return -4;
	}
	type_sum->fffpfs = atoi(fffpfs);

	item = cJSON_GetObjectItem(root, "kffpfs");
	if (get_json_value_can_not_null(item, (char *)kffpfs, 0, sizeof(kffpfs)) < 0)
	{
		cJSON_Delete(root);
		return -4;
	}
	type_sum->kffpfs = atoi(kffpfs);

	item = cJSON_GetObjectItem(root, "zsfpljje");
	if (get_json_value_can_not_null(item, (char *)zsfpljje, 0, sizeof(zsfpljje)) < 0)
	{
		cJSON_Delete(root);
		return -4;
	}
	type_sum->zsfpljje = strtold(zsfpljje, NULL);

	item = cJSON_GetObjectItem(root, "zsfpljse");
	if (get_json_value_can_not_null(item, (char *)zsfpljse, 0, sizeof(zsfpljse)) < 0)
	{
		cJSON_Delete(root);
		return -4;
	}
	type_sum->zsfpljse = strtold(zsfpljse, NULL);

	item = cJSON_GetObjectItem(root, "zffpljje");
	if (get_json_value_can_not_null(item, (char *)zffpljje, 0, sizeof(zffpljje)) < 0)
	{
		cJSON_Delete(root);
		return -4;
	}
	type_sum->zffpljje = strtold(zffpljje, NULL);




	item = cJSON_GetObjectItem(root, "zffpljse");
	if (get_json_value_can_not_null(item, (char *)zffpljse, 0, sizeof(zffpljse)) < 0)
	{
		cJSON_Delete(root);
		return -4;
	}
	type_sum->zffpljse = strtold(zffpljse, NULL);

	item = cJSON_GetObjectItem(root, "fsfpljje");
	if (get_json_value_can_not_null(item, (char *)fsfpljje, 0, sizeof(fsfpljje)) < 0)
	{
		cJSON_Delete(root);
		return -4;
	}
	type_sum->fsfpljje = strtold(fsfpljje, NULL);

	item = cJSON_GetObjectItem(root, "fsfpljse");
	if (get_json_value_can_not_null(item, (char *)fsfpljse, 0, sizeof(fsfpljse)) < 0)
	{
		cJSON_Delete(root);
		return -4;
	}
	type_sum->fsfpljse = strtold(fsfpljse, NULL);

	item = cJSON_GetObjectItem(root, "fffpljje");
	if (get_json_value_can_not_null(item, (char *)fffpljje, 0, sizeof(fffpljje)) < 0)
	{
		cJSON_Delete(root);
		return -4;
	}
	type_sum->fffpljje = strtold(fffpljje, NULL);

	item = cJSON_GetObjectItem(root, "fffpljse");
	if (get_json_value_can_not_null(item, (char *)fffpljse, 0, sizeof(fffpljse)) < 0)
	{
		cJSON_Delete(root);
		return -4;
	}
	type_sum->fffpljse = strtold(fffpljse, NULL);

	item = cJSON_GetObjectItem(root, "sjxsje");
	if (get_json_value_can_not_null(item, (char *)sjxsje, 0, sizeof(sjxsje)) < 0)
	{
		cJSON_Delete(root);
		return -4;
	}
	type_sum->sjxsje = strtold(sjxsje, NULL);

	item = cJSON_GetObjectItem(root, "sjxsse");
	if (get_json_value_can_not_null(item, (char *)sjxsse, 0, sizeof(sjxsse)) < 0)
	{
		cJSON_Delete(root);
		return -4;
	}
	type_sum->sjxsse = strtold(sjxsse, NULL);

	cJSON_Delete(root);
	return 0;
}

int deal_plate_statistics_to_now_month_sum(char *statistics, struct _inv_sum_data *now_month_sum)
{
	char *statistics_json;
	int statistics_json_len;
	char fplx_s[10];
	char fplx_data_base[4096];
	uint8 fplx;
	int mem_i = -1;
	if (statistics == NULL)
	{
		return -1;
	}
	statistics_json_len = strlen(statistics)+1024;
	statistics_json = calloc(1, statistics_json_len);
	if (statistics_json == NULL)
	{
		return -2;
	}
	Base64_Decode(statistics, strlen(statistics), statistics_json);
	//_WriteLog(LL_INFO, "%s", statistics_json);



	cJSON *arrayItem, *item; //解析使用
	int size,i;
	cJSON *root = cJSON_Parse(statistics_json);
	if (!root)
	{
		free(statistics_json);
		return -3;
	}
	free(statistics_json);
	cJSON *object_data = cJSON_GetObjectItem(root, "statistics");
	if (object_data == NULL)
	{
		cJSON_Delete(root);
		return -4;
	}
	size = cJSON_GetArraySize(object_data);
	for (i = 0; i < size; i++)
	{
		arrayItem = cJSON_GetArrayItem(object_data, i);
		if (arrayItem == NULL)
		{
			cJSON_Delete(root);
			return -5;
		}
		item = cJSON_GetObjectItem(arrayItem, "fplxdm");
		if (item == NULL)
		{
			cJSON_Delete(root);
			return -6;
		}
		if (item->valuestring != NULL)
		{
			memset(fplx_s, 0, sizeof(fplx_s));
			memcpy(fplx_s, item->valuestring, strlen(item->valuestring));
			fplx = atoi(fplx_s);
		}
		//out("获取到的发票类型为%03d\n", fplx);

		item = cJSON_GetObjectItem(arrayItem, "data");
		if (item == NULL)
		{
			cJSON_Delete(root);
			return -7;
		}
		if (item->valuestring != NULL)
		{
			memset(fplx_data_base, 0, sizeof(fplx_data_base));
			memcpy(fplx_data_base, item->valuestring, strlen(item->valuestring));
		}
		//out("获取到的汇总数据为%s\n", fplx_data_base);


		switch (fplx)
		{
		case FPLX_COMMON_ZYFP:
			mem_i = 0;
			break;
		case FPLX_COMMON_PTFP:
			mem_i = 1;
			break;
		case FPLX_COMMON_DZFP:
			mem_i = 2;
			break;
		case FPLX_COMMON_DZZP:
			mem_i = 3;
			break;
		default:
			//_WriteLog(LL_INFO, "jude_need_upload_inv 不支持的发票种类：%03d\n", fpxx->fplx);
			return -8;
		}

		//out("获取到的内存地址为%d\n", mem_i);
		deal_fplx_statistics_to_now_month_sum(fplx_data_base, &now_month_sum->type_sum[mem_i]);

	}

	cJSON_Delete(root);

	return 0;


}