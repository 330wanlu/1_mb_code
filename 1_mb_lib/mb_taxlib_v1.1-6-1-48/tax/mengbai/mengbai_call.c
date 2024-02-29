#define mengbai_call_c
#include "mengbai_call.h"


static int String2Bytes(unsigned char *szSrc, unsigned char *pDst, int nDstMaxLen)
{
	if (szSrc == NULL)
	{
		return 0;
	}
	int iLen = strlen((char *)szSrc);
	if (iLen <= 0 || iLen % 2 != 0 || pDst == NULL || nDstMaxLen < iLen / 2)
	{
		return 0;
	}
	iLen /= 2;
	int i = 0;
	for (i = 0; i < iLen; i++)
	{
		int iVal = 0;
		unsigned char *pSrcTemp = szSrc + i * 2;
		sscanf((char *)pSrcTemp, "%02x", &iVal);
		pDst[i] = (unsigned char)iVal;
	}
	return iLen;
}

static void month_to_month_range(unsigned char *month, unsigned char *month_range)
{
	int tmp_time[4];
	int date_start[4];
	int date_end[4];
	unsigned char date_range[32];
	char date_start_s[12];
	char date_end_s[12];

	uint16 Year;

	out("查询月份szFormatMonth = %s\n", month);
	//out("转换月份成时间段\n");
	memset(date_range, 0, sizeof(date_range));
	memset(tmp_time, 0, sizeof(tmp_time));
	memset(date_start, 0, sizeof(date_start));
	memset(date_end, 0, sizeof(date_end));
	sscanf((char *)month, "%02x%02x%02x", &tmp_time[0], &tmp_time[1], &tmp_time[2]);

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
		Year = tmp_time[0] * 100 + tmp_time[1];
		if ((Year & 0x03) != 0)
			date_end[3] = 0x28;
		else
			date_end[3] = 0x29;
	}
	sprintf(date_start_s, "%02x%02x%02x%02x", date_start[0], date_start[1], date_start[2], date_start[3]);
	//out("发票查询起始日期：%s\n", date_start_s);
	sprintf(date_end_s, "%02x%02x%02x%02x", date_end[0], date_end[1], date_end[2], date_end[3]);
	//out("发票查询结束日期：%s\n", date_end_s);
	sprintf((char *)date_range, "%02x%02x-%02x-%02x%02x%02x-%02x-%02x", date_start[0], date_start[1], date_start[2], date_start[3], date_end[0], date_end[1], date_end[2], date_end[3]);
	memcpy(month_range, date_range, strlen((const char *)date_range));
	return;
}

static int pack_s_buf(uint8 order, int s_num,uint8 *in_buf, uint8 *out_buf, int in_len)
{
	uint16 len;
	uint16 crc;
	len = in_len + 8;
	out_buf[0] = 'M'; out_buf[1] = 'B';
	//out("len = %d\n",len);
	out_buf[2] = ((len >> 8) & 0xff); out_buf[3] = (len & 0xff);
	out_buf[4] = order;
	out_buf[5] = s_num;
	memcpy(out_buf + 6, in_buf, in_len);
	crc = crc_8005(out_buf, len - 2, 0);
	out_buf[len - 2] = ((crc >> 8) & 0xff);
	out_buf[len - 1] = (crc & 0xff);
	return (int)len;
}

static int pack_offline_report_json_data(unsigned char *in_data, unsigned char *json_data, unsigned char *jshj, char *name, char *number)
{
	int sp_count;
	struct Fpxx fpxx;
	struct Spxx spxx;
	char sp_data[4096];
	memset(&fpxx,0,sizeof(struct Fpxx));
	char fplx_tmp[20] = { 0 };
	memcpy(fplx_tmp, in_data + 0, 3);
	fpxx.fplx = atoi(fplx_tmp);

	//memcpy(fpxx.fplx, in_data + 0, 3);
	memcpy(fpxx.kpsj_F2, in_data + 3, 19);
	memcpy(fpxx.fpdm, in_data + 22, 20);
	memcpy(fpxx.fphm, in_data + 42, 8);
	memcpy(fpxx.kplx, in_data + 50, 1);
	//memcpy(fpxx.zfbz_s, in_data + 51, 1);
	fpxx.zfbz = in_data[51];
	memcpy(fpxx.zfsj, in_data + 52, 19);
	memcpy(fpxx.gfsh, in_data + 71, 20);
	memcpy(fpxx.gfmc, in_data + 91, 80);
	memcpy(fpxx.je, in_data + 171, 20);
	memcpy(fpxx.se, in_data + 191, 20);
	memcpy(fpxx.jshj, in_data + 211, 20);
	memcpy(fpxx.bz, in_data + 231, 20);
	memcpy(fpxx.skr, in_data + 251, 10);
	memcpy(fpxx.kpr, in_data + 261, 10);
	memcpy(fpxx.blueFpdm, in_data + 271, 20);
	memcpy(fpxx.blueFphm, in_data + 291, 8);

	memcpy(jshj, fpxx.jshj, 20);
	//out("fpxx.blueFphm = %s\n", fpxx.blueFphm);
	char spsl_s[10] = {0};
	memcpy(spsl_s, in_data + 299, 2);
	fpxx.spsl = atoi(spsl_s);
	cJSON *fpmx_array, *data_array, *head_layer,*data_layer;
	cJSON *json = cJSON_CreateObject();
	cJSON_AddItemToObject(json, "head", head_layer = cJSON_CreateObject());
	cJSON_AddStringToObject(head_layer, "protocol", "MB_API20190627");
	cJSON_AddStringToObject(head_layer, "mode", "request");
	cJSON_AddStringToObject(head_layer, "cmd", "/WAS.UploadBillingData/offlineTicketUpload");
	cJSON_AddStringToObject(head_layer, "random", "");
	cJSON_AddStringToObject(head_layer, "result", "");
	cJSON_AddItemToObject(json, "data", data_array = cJSON_CreateArray());	
	cJSON_AddItemToObject(data_array, "dira", data_layer = cJSON_CreateObject());
	cJSON_AddStringToObject(data_layer, "FPLXDM", fplx_tmp);
	cJSON_AddStringToObject(data_layer, "KPSJ", fpxx.kpsj_F2);
	cJSON_AddStringToObject(data_layer, "FPDM", fpxx.fpdm);
	cJSON_AddStringToObject(data_layer, "FPHM", fpxx.fphm);
	cJSON_AddStringToObject(data_layer, "KJLX", fpxx.kplx);
	char zfbz_s[10] = {0};
	sprintf(zfbz_s, "%d", fpxx.zfbz);
	cJSON_AddStringToObject(data_layer, "SFZF", zfbz_s);
	cJSON_AddStringToObject(data_layer, "ZFSJ", fpxx.zfsj);
	cJSON_AddStringToObject(data_layer, "XFDWMC", name);
	cJSON_AddStringToObject(data_layer, "XFDWSBH", number);
	cJSON_AddStringToObject(data_layer, "XFDWDZDH", "");
	cJSON_AddStringToObject(data_layer, "XFDWYHZH", "");
	cJSON_AddStringToObject(data_layer, "GHDWMC", fpxx.gfmc);
	cJSON_AddStringToObject(data_layer, "GHDWSBH", fpxx.gfsh);
	cJSON_AddStringToObject(data_layer, "GHDWDZDH", "");
	cJSON_AddStringToObject(data_layer, "GHDWYHZH", "");
	cJSON_AddStringToObject(data_layer, "HJJE", fpxx.je);
	cJSON_AddStringToObject(data_layer, "HJSE", fpxx.se);
	cJSON_AddStringToObject(data_layer, "JSHJ", fpxx.jshj);
	cJSON_AddStringToObject(data_layer, "BZ", fpxx.bz);
	cJSON_AddStringToObject(data_layer, "SKR", fpxx.skr);
	cJSON_AddStringToObject(data_layer, "KPR", fpxx.kpr);
	cJSON_AddStringToObject(data_layer, "YFPDM", fpxx.blueFpdm);
	cJSON_AddStringToObject(data_layer, "YFPHM", fpxx.blueFphm);
	char spsl_tmp[10] = {0};
	sprintf(spsl_tmp, "%d", fpxx.spsl);
	cJSON_AddStringToObject(data_layer, "KPXMSL", spsl_tmp);
	cJSON_AddStringToObject(data_layer, "JYM", "");
	cJSON_AddStringToObject(data_layer, "MW", "");
	cJSON_AddStringToObject(data_layer, "QDBZ", "0");
	cJSON_AddStringToObject(data_layer, "SBBZ", "0");
	cJSON_AddItemToObject(data_layer, "FPMX", fpmx_array = cJSON_CreateArray());
	for (sp_count = 0; sp_count < fpxx.spsl; sp_count++)
	{
		memset(sp_data, 0, sizeof(sp_data));
		memcpy(sp_data, in_data + 301 + sp_count * 232, 232);
		memset(&spxx, 0, sizeof(struct Spxx));
		memcpy(spxx.spmc, sp_data + 0, 40);
		memcpy(spxx.dj, sp_data + 40, 20);
		memcpy(spxx.sl, sp_data + 60, 20);
		memcpy(spxx.je, sp_data + 80, 20);
		memcpy(spxx.jldw, sp_data + 100, 22);
		memcpy(spxx.ggxh, sp_data + 122, 40);
		memcpy(spxx.se, sp_data + 162, 20);
		char spslv[20] = {0};
		memcpy(spslv, sp_data + 182, 10);
		//printf_array(spslv,10);
		float i_slv = atof(spslv);
		float f_slv = i_slv / 100;
		sprintf(spxx.slv, "%4.2f", f_slv);
		memcpy(spxx.spbh, sp_data + 192, 40);
		cJSON *fpmx_layer;
		cJSON_AddItemToObject(fpmx_array, "dira", fpmx_layer = cJSON_CreateObject());
		cJSON_AddStringToObject(fpmx_layer, "SPMC", spxx.spmc);
		cJSON_AddStringToObject(fpmx_layer, "DJ", spxx.dj);
		cJSON_AddStringToObject(fpmx_layer, "JLDW", spxx.jldw);
		cJSON_AddStringToObject(fpmx_layer, "GGXH", spxx.ggxh);
		cJSON_AddStringToObject(fpmx_layer, "SE", spxx.se);
		cJSON_AddStringToObject(fpmx_layer, "JE", spxx.je);
		cJSON_AddStringToObject(fpmx_layer, "SLV", spxx.slv);
		cJSON_AddStringToObject(fpmx_layer, "SL", spxx.sl);
		cJSON_AddStringToObject(fpmx_layer, "SPBH", spxx.spbh);
	}
	char *g_buf;
	g_buf = cJSON_Print(json);
	
	//out("离线票数据 = %s\n", g_buf);
	memcpy(json_data,g_buf,strlen(g_buf));
	free(g_buf);
	cJSON_Delete(json);
	return 0;
}

static int pack_chao_bao_json_data(unsigned char *in_data, unsigned char *json_data,unsigned char *nsrsbh)
{
	int index;
	struct _copy_report copy_report;
	unsigned int zcfpkjje; //正常发票开具金额
	unsigned int zcfpse;   //正常发票税额
	unsigned int zcfpjshj;	//正常发票价税合计
	unsigned int hpkpje;	//红票开具金额
	unsigned int hpse;		//红票税额
	unsigned int hpjshj;	//红票价税合计
	char fplxdm[3];
	float tmp_money;
	char tmp[20];
	memset(&copy_report, 0, sizeof(struct _copy_report));


	memset(fplxdm, 0, sizeof(fplxdm));
	memcpy(fplxdm, in_data + 0, 3);
	copy_report.fplxdm = atoi(fplxdm);
	memcpy(copy_report.cbsj, in_data + 3, 19);
	memcpy(copy_report.hzksrq, in_data + 22, 10);
	memcpy(copy_report.hzjzrq, in_data + 32, 10);

	index = 42;
	copy_report.zcfpkjfs = in_data[index + 0] * 0x1000000 + in_data[index + 1] * 0x10000 + in_data[index + 2] * 0x100 + in_data[index + 3];

	index = 46;
	copy_report.hpkjfs = in_data[index + 0] * 0x1000000 + in_data[index + 1] * 0x10000 + in_data[index + 2] * 0x100 + in_data[index + 3];

	index = 50;
	copy_report.zcfpzffs = in_data[index + 0] * 0x1000000 + in_data[index + 1] * 0x10000 + in_data[index + 2] * 0x100 + in_data[index + 3];

	index = 54;
	copy_report.hpzffs = in_data[index + 0] * 0x1000000 + in_data[index + 1] * 0x10000 + in_data[index + 2] * 0x100 + in_data[index + 3];

	index = 58;
	copy_report.kbfs = in_data[index + 0] * 0x1000000 + in_data[index + 1] * 0x10000 + in_data[index + 2] * 0x100 + in_data[index + 3];

	index = 62;
	zcfpkjje = in_data[index + 0] * 0x1000000 + in_data[index + 1] * 0x10000 + in_data[index + 2] * 0x100 + in_data[index + 3];
	tmp_money = (float)zcfpkjje / 100;
	sprintf((char *)copy_report.zcfpkjje, "%.2f", tmp_money);

	index = 66;
	zcfpse = in_data[index + 0] * 0x1000000 + in_data[index + 1] * 0x10000 + in_data[index + 2] * 0x100 + in_data[index + 3];
	tmp_money = (float)zcfpse / 100;
	sprintf((char *)copy_report.zcfpse, "%.2f", tmp_money);

	index = 70;
	zcfpjshj = in_data[index + 0] * 0x1000000 + in_data[index + 1] * 0x10000 + in_data[index + 2] * 0x100 + in_data[index + 3];
	tmp_money = (float)zcfpjshj / 100;
	sprintf((char *)copy_report.zcfpjshj, "%.2f", tmp_money);

	index = 74;
	hpkpje = in_data[index + 0] * 0x1000000 + in_data[index + 1] * 0x10000 + in_data[index + 2] * 0x100 + in_data[index + 3];
	tmp_money = (float)hpkpje / 100;
	sprintf((char *)copy_report.hpkjje, "%.2f", tmp_money);

	index = 78;
	hpse = in_data[index + 0] * 0x1000000 + in_data[index + 1] * 0x10000 + in_data[index + 2] * 0x100 + in_data[index + 3];
	tmp_money = (float)hpse / 100;
	sprintf((char *)copy_report.hpse, "%.2f", tmp_money);

	index = 82;
	hpjshj = in_data[index + 0] * 0x1000000 + in_data[index + 1] * 0x10000 + in_data[index + 2] * 0x100 + in_data[index + 3];
	tmp_money = (float)hpjshj / 100;
	sprintf((char *)copy_report.hpjshj, "%.2f", tmp_money);

	cJSON *data_array, *head_layer, *data_layer;
	cJSON *json = cJSON_CreateObject();
	cJSON_AddItemToObject(json, "head", head_layer = cJSON_CreateObject());
	cJSON_AddStringToObject(head_layer, "protocol", "MB_API20190627");
	cJSON_AddStringToObject(head_layer, "mode", "request");
	cJSON_AddStringToObject(head_layer, "cmd", "/WAS.UploadBillingData/declareDutiableGoods");
	cJSON_AddStringToObject(head_layer, "random", "");
	cJSON_AddStringToObject(head_layer, "result", "");
	cJSON_AddItemToObject(json, "data", data_array = cJSON_CreateArray());
	cJSON_AddItemToObject(data_array, "dira", data_layer = cJSON_CreateObject());

	cJSON_AddStringToObject(data_layer, "NSRSBH", (char *)nsrsbh);
	cJSON_AddStringToObject(data_layer, "FPLXDM", fplxdm);
	cJSON_AddStringToObject(data_layer, "CBSJ", (char *)copy_report.cbsj);
	cJSON_AddStringToObject(data_layer, "HZKSRQ", (char *)copy_report.hzksrq);
	cJSON_AddStringToObject(data_layer, "HZJZRQ", (char *)copy_report.hzjzrq);

	memset(tmp, 0, sizeof(tmp));	sprintf(tmp, "%d", copy_report.zcfpkjfs);
	cJSON_AddStringToObject(data_layer, "ZCFPKJFS",tmp);

	memset(tmp, 0, sizeof(tmp));	sprintf(tmp, "%d", copy_report.hpkjfs);
	cJSON_AddStringToObject(data_layer, "HPKJFS", tmp);

	memset(tmp, 0, sizeof(tmp));	sprintf(tmp, "%d", copy_report.zcfpzffs);
	cJSON_AddStringToObject(data_layer, "ZCFPZFFS", tmp);

	memset(tmp, 0, sizeof(tmp));	sprintf(tmp, "%d", copy_report.hpzffs);
	cJSON_AddStringToObject(data_layer, "HPZFFS", tmp);

	memset(tmp, 0, sizeof(tmp));	sprintf(tmp, "%d", copy_report.kbfs);
	cJSON_AddStringToObject(data_layer, "KBFS", tmp);

	cJSON_AddStringToObject(data_layer, "ZCFPKJJE", (char *)copy_report.zcfpkjje);
	cJSON_AddStringToObject(data_layer, "ZCFPSE", (char *)copy_report.zcfpse);
	cJSON_AddStringToObject(data_layer, "ZCFPJSHJ", (char *)copy_report.zcfpjshj);
	cJSON_AddStringToObject(data_layer, "HPKJJE", (char *)copy_report.hpkjje);
	cJSON_AddStringToObject(data_layer, "HPSE", (char *)copy_report.hpse);
	cJSON_AddStringToObject(data_layer, "HPJSHJ", (char *)copy_report.hpjshj);
	char *g_buf;
	g_buf = cJSON_Print(json);
	//out("g_buf = %s\n", g_buf);
	memcpy(json_data, g_buf, strlen(g_buf));
	cJSON_Delete(json);
	free(g_buf);
	return 0;
}

static int pack_get_inv_json(unsigned char *in_data, int in_len, unsigned char *out_data,unsigned char *ca_name,unsigned char *ca_number)
{
	int sp_count;
	struct Fpxx fpxx;
	struct Spxx spxx;
	char sp_data[4096];
	memset(&fpxx, 0, sizeof(struct Fpxx));

	//out("发票数据：\n");
	//print_array(in_data, in_len);
	//memset(tmp, 0, sizeof(tmp));
	//memcpy(tmp, in_data + 0, 4);
	//*total_num = tmp[0] * 0x1000000 + tmp[1] * 0x10000 + tmp[2] * 0x100 + tmp[3] * 0x1;


	//memset(tmp, 0, sizeof(tmp));
	//memcpy(tmp, in_data + 4, 4);
	//*now_num = tmp[0] * 0x1000000 + tmp[1] * 0x10000 + tmp[2] * 0x100 + tmp[3] * 0x1;
	//out("发票总数为：%d,当前序号为：%d\n", *total_num, *now_num);
	char fplx_tmp[20] = { 0 };
	memcpy(fplx_tmp, in_data + 8, 3);
	fpxx.fplx = atoi(fplx_tmp);
	memcpy(fpxx.kpsj_F2, in_data + 11, 19);
	memcpy(fpxx.fpdm, in_data + 30, 20);
	memcpy(fpxx.fphm, in_data + 50, 8);
	memcpy(fpxx.kplx, in_data + 58, 1);
	//memcpy(fpxx.zfbz_s, in_data + 59, 1);
	fpxx.zfbz = in_data[59];
	memcpy(fpxx.zfsj, in_data + 60, 19);
	memcpy(fpxx.gfsh, in_data + 79, 20);
	memcpy(fpxx.gfmc, in_data + 99, 80);
	memcpy(fpxx.je, in_data + 179, 20);
	memcpy(fpxx.se, in_data + 199, 20);
	memcpy(fpxx.jshj, in_data + 219, 20);
	memcpy(fpxx.bz, in_data + 239, 20);
	memcpy(fpxx.skr, in_data + 259, 10);
	memcpy(fpxx.kpr, in_data + 269, 10);
	memcpy(fpxx.blueFpdm, in_data + 279, 20);
	memcpy(fpxx.blueFphm, in_data + 299, 8);
	//memcpy(fpxx.spsl, in_data + 307, 2);

	char spsl_s[10] = { 0 };
	memcpy(spsl_s, in_data + 307, 2);
	fpxx.spsl = atoi(spsl_s);


	//fpxx.spsl = in_data[307] * 0x100 + in_data[308] * 0x1;
	//spsl = fpxx.spsl;
	cJSON *arrayItem1;
	cJSON *json = cJSON_CreateObject();
	char zfbz_s[10] = {0};
	sprintf(zfbz_s, "%d", fpxx.zfbz);
	cJSON_AddStringToObject(json, "ZFBZ", zfbz_s);
	cJSON_AddStringToObject(json, "SBBZ", "0");
	cJSON_AddStringToObject(json, "FPLX", fplx_tmp);
	cJSON_AddStringToObject(json, "FPDM", fpxx.fpdm);
	cJSON_AddStringToObject(json, "FPHM", fpxx.fphm);
	cJSON_AddStringToObject(json, "GFMC", fpxx.gfmc);
	cJSON_AddStringToObject(json, "GFSH", fpxx.gfsh);
	cJSON_AddStringToObject(json, "XFMC", (char *)ca_name);
	cJSON_AddStringToObject(json, "XFSH", (char *)ca_number);
	cJSON_AddStringToObject(json, "KPSJ", fpxx.kpsj_F2);
	cJSON_AddStringToObject(json, "HJJE", fpxx.je);
	cJSON_AddStringToObject(json, "HJSE", fpxx.se);
	cJSON_AddStringToObject(json, "JSHJ", fpxx.jshj);
	cJSON_AddStringToObject(json, "GFDZDH", "");
	cJSON_AddStringToObject(json, "GFYHZH", "");
	cJSON_AddStringToObject(json, "XFDZDH", "");
	cJSON_AddStringToObject(json, "XFYHZH", "");
	cJSON_AddStringToObject(json, "JYM", "");
	cJSON_AddStringToObject(json, "MW", "");
	cJSON_AddStringToObject(json, "QDBZ", "0");
	cJSON_AddItemToObject(json, "FPMX", arrayItem1 = cJSON_CreateArray());
	for (sp_count = 0; sp_count < fpxx.spsl; sp_count++)
	{
		memset(sp_data, 0, sizeof(sp_data));
		memcpy(sp_data, in_data + 309 + sp_count * 232, 232);
		memset(&spxx, 0, sizeof(struct Spxx));
		memcpy(spxx.spmc, sp_data + 0, 40);
		memcpy(spxx.dj, sp_data + 40, 20);
		memcpy(spxx.sl, sp_data + 60, 20);
		memcpy(spxx.je, sp_data + 80, 20);
		memcpy(spxx.jldw, sp_data + 100, 22);
		memcpy(spxx.ggxh, sp_data + 122, 40);
		memcpy(spxx.se, sp_data + 162, 20);
		char slv_s[10] = { 0 };
		float slv_f;
		memcpy(slv_s, sp_data + 182, 10);
		slv_f = atof(slv_s) / 100;
		sprintf(spxx.slv, "%4.2f", slv_f);
		memcpy(spxx.spbh, sp_data + 192, 40);
		cJSON *p_layer1;
		cJSON_AddItemToObject(arrayItem1, "dira", p_layer1 = cJSON_CreateObject());
		cJSON_AddStringToObject(p_layer1, "SPMC", spxx.spmc);
		cJSON_AddStringToObject(p_layer1, "DJ", spxx.dj);
		cJSON_AddStringToObject(p_layer1, "JLDW", spxx.jldw);
		cJSON_AddStringToObject(p_layer1, "GGXH", spxx.ggxh);
		cJSON_AddStringToObject(p_layer1, "SE", spxx.se);
		cJSON_AddStringToObject(p_layer1, "JE", spxx.je);
		cJSON_AddStringToObject(p_layer1, "SLV", spxx.slv);
		cJSON_AddStringToObject(p_layer1, "SL", spxx.sl);
		cJSON_AddStringToObject(p_layer1, "SPBH", spxx.spbh);
	}
	char *json_buf;
	char *g_buf;
	int g_len;

	json_buf = cJSON_Print(json);
	g_buf = malloc(strlen(json_buf) * 2);
	memset(g_buf, 0, strlen(json_buf) * 2);
	memcpy(g_buf, json_buf, strlen(json_buf));
	free(json_buf);


	str_replace(g_buf, "\\\\", "[@*br/*@]");
	str_replace(g_buf, "[@*br/*@]", "\\");


	//out("g_buf = %s\n", g_buf);
	g_len = strlen(g_buf);

	encode(g_buf, g_len, (char *)out_data);
	free(g_buf);
	cJSON_Delete(json);
	return strlen((const char*)out_data);
}


static int pack_get_inv_json_new(unsigned char *in_data, int in_len, unsigned char *out_data,uint8 *zfbz, unsigned char *plate_num, unsigned char *ca_name, unsigned char *ca_number)
{
	int sp_count;
	HFPXX fpxx = MallocFpxx();
	//struct Spxx spxx;
	char sp_data[4096];
	//memset(&fpxx, 0, sizeof(struct Fpxx));

	//out("发票数据：\n");
	//print_array(in_data, in_len);
	//memset(tmp, 0, sizeof(tmp));
	//memcpy(tmp, in_data + 0, 4);
	//*total_num = tmp[0] * 0x1000000 + tmp[1] * 0x10000 + tmp[2] * 0x100 + tmp[3] * 0x1;


	//memset(tmp, 0, sizeof(tmp));
	//memcpy(tmp, in_data + 4, 4);
	//*now_num = tmp[0] * 0x1000000 + tmp[1] * 0x10000 + tmp[2] * 0x100 + tmp[3] * 0x1;
	//out("发票总数为：%d,当前序号为：%d\n", *total_num, *now_num);
	char fplx_tmp[20] = { 0 };
	memcpy(fplx_tmp, in_data + 8, 3);
	fpxx->fplx = atoi(fplx_tmp);
	memcpy(fpxx->kpsj_F2, in_data + 11, 19);
	memcpy(fpxx->fpdm, in_data + 30, 20);
	memcpy(fpxx->fphm, in_data + 50, 8);
	memcpy(fpxx->kplx, in_data + 58, 1);
	//memcpy(fpxx.zfbz_s, in_data + 59, 1);
	if (in_data[59] == 0x30)
		fpxx->zfbz = 0;
	else if(in_data[59] == 0x31)
		fpxx->zfbz = 1;
	else
		fpxx->zfbz = 0;
	*zfbz = fpxx->zfbz;
	if (fpxx->zfbz == 1)
		memcpy(fpxx->zfsj_F2, in_data + 60, 19);
	memcpy(fpxx->gfsh, in_data + 79, 20);
	memcpy(fpxx->gfmc, in_data + 99, 80);
	memcpy(fpxx->je, in_data + 179, 20);
	memcpy(fpxx->se, in_data + 199, 20);
	memcpy(fpxx->jshj, in_data + 219, 20);
	memcpy(fpxx->bz, in_data + 239, 20);
	memcpy(fpxx->skr, in_data + 259, 10);
	memcpy(fpxx->kpr, in_data + 269, 10);
	memcpy(fpxx->blueFpdm, in_data + 279, 20);
	char yfphm[20] = { 0 };
	memcpy(yfphm, in_data + 299, 8);
	if (atoi(yfphm) != 0)
	{
		sprintf(fpxx->blueFpdm, "%s", yfphm);
	}
	//memcpy(fpxx.spsl, in_data + 307, 2);
	sprintf(fpxx->bmbbbh,"33.0");
	sprintf(fpxx->hsjbz, "0");
	sprintf(fpxx->qdbj, "N");
	sprintf(fpxx->xfmc, "%s", ca_name);
	sprintf(fpxx->xfsh, "%s", ca_number);
	sprintf(fpxx->xfdzdh, "仅供测试使用");
	sprintf(fpxx->xfyhzh, "仅供测试使用");
	sprintf(fpxx->gfdzdh, "仅供测试使用");
	sprintf(fpxx->gfyhzh, "仅供测试使用");
	sprintf(fpxx->jym, "01234567890123456789");
	sprintf(fpxx->mw, "<*/*0000000000111111111122222222223333333333444444444455555555556666666666777777777788888888889999999999*/*>");
	

	char spsl_s[10] = { 0 };
	memcpy(spsl_s, in_data + 307, 2);
	fpxx->spsl = atoi(spsl_s);

	for (sp_count = 0; sp_count < fpxx->spsl; sp_count++)
	{
		struct Spxx *Head = (struct Spxx *)calloc(1, sizeof(struct Spxx));

		memset(sp_data, 0, sizeof(sp_data));
		memcpy(sp_data, in_data + 309 + sp_count * 232, 232);
		char spsmmc_spmc[200] = { 0 };
		memcpy(spsmmc_spmc, sp_data + 0, 40);
		exchange_spmc_spsmmc(spsmmc_spmc, sizeof(spsmmc_spmc));
		GetSpsmmcSpmc(spsmmc_spmc, Head->spsmmc, Head->spmc);
		if (sp_count == 0)
		{
			memcpy(fpxx->zyspsmmc, Head->spsmmc, strlen(Head->spsmmc));
			memcpy(fpxx->zyspmc, Head->spmc, strlen(Head->spmc));
		}

		memcpy(Head->dj, sp_data + 40, 20);
		memcpy(Head->sl, sp_data + 60, 20);
		memcpy(Head->je, sp_data + 80, 20);
		memcpy(Head->jldw, sp_data + 100, 22);
		memcpy(Head->ggxh, sp_data + 122, 40);
		memcpy(Head->se, sp_data + 162, 20);
		char slv_s[10] = { 0 };
		float slv_f;
		memcpy(slv_s, sp_data + 182, 10);
		slv_f = atof(slv_s) / 100;
		sprintf(Head->slv, "%4.2f", slv_f);
		memcpy(Head->spbh, sp_data + 192, 40);

		sprintf(Head->xsyh, "0");
		sprintf(Head->fphxz, "0");
		sprintf(Head->hsjbz, "0");

		InsertMxxx(fpxx, Head);

	}

	char *g_buf;
	struct StaticDeviceInfo pDevInfo;
	memset(&pDevInfo, 0, sizeof(struct StaticDeviceInfo));
	pDevInfo.bDeviceType = 3;
	memcpy(pDevInfo.szDeviceID, plate_num, strlen((char *)plate_num));
	fpxx->hDev = &pDevInfo;
	fpxx_to_json_base_fpsjbbh_v102(fpxx, &g_buf, DF_FPQD_Y, DF_FP_BASE);

	memcpy(out_data, g_buf, strlen(g_buf));
	FreeFpxx(fpxx);
	free(g_buf);
	return strlen((const char*)out_data);
}

static int exchange_spmc_spsmmc(char *spsmmc_mc,int size)
{
	char tmp[200] = {0};
	char spsmmc[200] = {0};
	char spmc[200] = {0};
	char *h_flag;
	memcpy(tmp, spsmmc_mc, strlen(spsmmc_mc));
	if (tmp[0]!= '*') 
	{
		//out("为错误商品税目名称顺序，需颠倒\n");
		h_flag = strstr(tmp, "*");
		if (h_flag != NULL)
		{
			memcpy(spmc, tmp, h_flag - tmp);
			memcpy(spsmmc, h_flag, strlen(tmp) - strlen(spmc));
			memset(spsmmc_mc, 0, size);
			sprintf(spsmmc_mc, "%s%s", spsmmc, spmc);
		}
	}
	return 0;
}


int LoadMengBaiInfo(HUSB hUSB, HDEV hDev)
{
	int in_len, out_len;
	unsigned char s_data[4096];
	unsigned char r_data[4096];
	int result;
	char errinfo[2048] = {0};
	//联机命令
	//////////////////////////////////////////////////////////////////////////
	//out("联机命令\n");
	char time_now[20];
	memset(time_now, 0, sizeof(time_now));

	timer_read_y_m_d_h_m_s(time_now);
	//out("设置当前时间%s\n", time_now);

	strcpy(hDev->szDeviceTime, time_now);

	//////////print_array((char *)usb_data, size);
	//////////////////////////////////////////////////////////////////////
	memset(s_data, 0, sizeof s_data);
	in_len = pack_s_buf(DF_PC_USB_ORDER_CONNECT, 0, (unsigned char *)time_now, s_data, strlen(time_now));
	out_len = sizeof(r_data);

	result = usb_data_write_read(hUSB, s_data, in_len, r_data, &out_len, errinfo);
	if (result < 0)
	{
		out("联机失败\n");
		return result;
	}

	//获取卡信息
	////////////////////////////////////////////////////////////////////////
	//out("获取卡信息\n");
	memset(s_data, 0, sizeof(s_data));
	memset(r_data, 0, sizeof(r_data));
	in_len = pack_s_buf(DF_PC_USB_ORDER_GET_CARD_IFNFO, 0, NULL, s_data, 0);
	out_len = sizeof(r_data);
	result = usb_data_write_read(hUSB, s_data, in_len, r_data, &out_len, errinfo);
	if (result < 0)
	{
		out("获取卡信息失败\n");
		return -1;
	}
	//out("获取卡信息成功\n");

	hDev->uICCardNo = r_data[122] * 0x1000000 + r_data[123] * 0x10000 + r_data[124] * 0x100 + r_data[125] * 0x1;
	memcpy(hDev->szDeviceID, r_data, 12);
	memcpy(hDev->szCompanyName, r_data + 32, 80);
	//out("获取到的纳税人名称：%s\n", device->szCompanyName);
	memcpy(hDev->szCommonTaxID, r_data + 12, 20);


	return 0;
}

//查询税盘基本信息
int mengbai_read_basic_info(HDEV hDev, struct _plate_infos *plate_infos)
{
	unsigned char usb_data[4096];
	int nDataLen = 0;
	//int result;
	//int in_len, out_len;
	unsigned char s_data[4096];
	//unsigned char r_data[4096];
	//char errinfo[500];
	//memset(usb_data, 0, sizeof(usb_data));
	//if (libusb_control_transfer(device, 0x80, 0x06, 0x0100, 0, usb_data, 0x12, 3000) < 0)
	//{
	//	return DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
	//}
	//print_array((char *)usb_data, 0x12);

	//memset(usb_data, 0, sizeof(usb_data));
	//if (libusb_control_transfer(device, 0x80, 6, 0x0200, 0, usb_data, 9, 3000) < 0){
	//	return DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
	//}
	//nDataLen = usb_data[2];
	//memset(usb_data, 0, sizeof(usb_data));
	//if (libusb_control_transfer(device, 0x80, 6, 0x0200, 0, usb_data, nDataLen, 3000) < 0){
	//	return DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
	//}
	//print_array((char *)usb_data, nDataLen);

	//sleep(5);
	

	//end_time = get_time_sec();
	//timer_read_asc(e_time);
	//out("连接成功，连接开始时间：%s,连接结束时间：%s,共耗时%d秒\n", s_time, e_time, end_time-start_time);
	//out("联机成功\n");

	//更改证书秘钥
	////////////////////////////////////////////////////////////////////////
	//out("更改证书秘钥\n");
	char szBin0[500];
	int num;
	int i;
	int size;
	memset(szBin0, 0, sizeof(szBin0)); sprintf(szBin0, "4D4200182100323232323232323231313131313131319FB8");
	memset(usb_data, 0, sizeof usb_data); nDataLen = String2Bytes((unsigned char *)szBin0, usb_data, sizeof usb_data);
	//print_array((char *)usb_data, nDataLen);

	//out("nDataLen = %d\n", nDataLen);
	num = (nDataLen / 64) + 1;
	//out("num = %d\n", num);
	for (i = 0; i<num; i++)
	{
		memset(s_data, 0, sizeof(s_data));
		memcpy(s_data, usb_data + i * 64, 64);
		libusb_interrupt_transfer(hDev->hUSB->handle, 0x01, s_data, 64, &size, 100);
	}


	memset(usb_data, 0, sizeof usb_data);
	libusb_interrupt_transfer(hDev->hUSB->handle, 0x82, usb_data, 64, &size, 100);
	//print_array((char *)usb_data, size);


	////领购发票
	////////////////////////////////////////////////////////////////////////////
	//out("领购发票\n");
	//memset(szBin0, 0, sizeof(szBin0)); sprintf(szBin0, "4D4200432800323232323232323230303430313233343536373839313000000000000000000000038500000002323031392D30362D3038D5C5EACD00B2E2CAD400E822");
	//memset(usb_data, 0, sizeof usb_data); nDataLen = String2Bytes((unsigned char *)szBin0, usb_data, sizeof usb_data);
	//print_array((char *)usb_data, nDataLen);
	//
	//out("nDataLen = %d\n", nDataLen);
	//num = (nDataLen / 64) + 1;
	//out("num = %d\n",num);
	//for (i = 0; i<num; i++)
	//{
	//	memset(s_data,0,sizeof(s_data));
	//	memcpy(s_data, usb_data + i * 64,64);
	//	libusb_interrupt_transfer(device, outpoint, s_data, 64, &size, 100);
	//}
	//

	//memset(usb_data, 0, sizeof usb_data);
	//libusb_interrupt_transfer(device, inpoint, usb_data, 64, &size, 100);
	//print_array((char *)usb_data, size);

	
	//获取卡信息
	////////////////////////////////////////////////////////////////////////
	//out("获取卡信息\n");
	//memset(s_data, 0, sizeof(s_data));
	//memset(r_data, 0, sizeof(r_data));
	//in_len = pack_s_buf(DF_PC_USB_ORDER_GET_CARD_IFNFO, 0, NULL, s_data, 0);
	//out_len = sizeof(r_data);
	//result = usb_data_write_read(device, s_data, in_len, r_data, &out_len, errinfo);
	//if (result < 0)
	//{
	//	out("获取卡信息失败\n");
	//	return -1;
	//}
	//out("获取卡信息成功\n");
	plate_infos->plate_basic_info.plate_type = 3;
	plate_infos->plate_basic_info.result = 3;
	plate_infos->plate_basic_info.extension = hDev->uICCardNo;
	memcpy(plate_infos->plate_basic_info.plate_num, hDev->szDeviceID, 12);
	memcpy(plate_infos->plate_basic_info.ca_name, hDev->szCompanyName, 80);
	out("获取到的纳税人名称：%s\n", plate_infos->plate_basic_info.ca_name);
	memcpy(plate_infos->plate_basic_info.ca_number, hDev->szCommonTaxID, 20);
	return 0;
}



//查询支持的发票类型和区域代码
int mengbai_read_inv_type_areacode(HUSB hUSB, struct _plate_infos *plate_infos)
{
	unsigned char type_s[32];
	unsigned char type[3];
	int i;
	int type_i;
	int type_num=0;
	int result;
	int in_len, out_len;
	unsigned char s_data[4096];
	unsigned char r_data[4096];
	char errinfo[500] = { 0 };
	memset(s_data, 0, sizeof(s_data));
	memset(r_data, 0, sizeof(r_data));
	//获取卡信息
	////////////////////////////////////////////////////////////////////////
	//out("获取发票类型和区域代码\n");
	in_len = pack_s_buf(DF_PC_USB_ORDER_GET_CARD_IFNFO, 0, NULL, s_data, 0);
	out_len = sizeof(r_data);
	result = usb_data_write_read(hUSB, s_data, in_len, r_data, &out_len, errinfo);
	if (result < 0)
	{
		out("获取卡信息失败\n");
		return result;
	}

	memset(type_s, 0, sizeof(type_s));
	memcpy(type_s, r_data + 217, 30);
	for (i = 0; i < 10; i++)
	{
		memset(type, 0, sizeof(type));
		memcpy(type,type_s+i*3,3);
		type_i = atoi((const char*)type);
		if (type_i> 0)
		{
			plate_infos->plate_tax_info.inv_type[i] = type_i;
			type_num += 1;
		}
	}
	//out("支持的发票种类,type_num = %d\n",type_num);
	plate_infos->plate_tax_info.inv_type_num = type_num;
	//print_array((char *)plate_infos->plate_tax_info.inv_type, 10);
	memcpy(plate_infos->plate_tax_info.area_code, r_data + 112, 10);
	memcpy(plate_infos->plate_tax_info.tax_office_code, r_data + 126, 11);
	memcpy(plate_infos->plate_tax_info.tax_office_name, r_data + 137, 80);
	memcpy(plate_infos->plate_tax_info.startup_date, r_data + 247, 10);
	cls_character((char *)plate_infos->plate_tax_info.startup_date, strlen((const char*)plate_infos->plate_tax_info.startup_date), 0X2D);
	return type_num;
}



//查询监控信息数据
int mengbai_read_monitor_info(HUSB hUSB, unsigned char *inv_type, struct _plate_infos *plate_infos)
{
	unsigned char type[4];
	unsigned char type_num;
	int type_tmp[4];
	unsigned char money[4];
	int money_i;
	unsigned char money_s[15];
	unsigned char time[4];
	int time_i;
	int type_count;
	unsigned char time_tmp[20];
	int type_mem;
	int result;
	int in_len, out_len;
	unsigned char s_data[4096];
	unsigned char r_data[4096];
	char errinfo[500] = {0};
	for (type_count = 0; type_count < DF_MAX_INV_TYPE; type_count++)
	{
		if (inv_type[type_count] == 0)
		{
			//out("发票类型不正确\n");
			continue;
		}
		type_num = inv_type[type_count];
		//out("查询发票类型为%3d的监控信息数据\n", type_num);
		memset(type,0,sizeof(type));
		sprintf((char *)type, "%03d", type_num);

		//out("获取监控信息\n");
		memset(s_data, 0, sizeof(s_data));
		memset(r_data, 0, sizeof(r_data));
		in_len = pack_s_buf(DF_PC_USB_ORDER_GET_JKXXSJ, 0, type, s_data, strlen((const char*)type));
		out_len = sizeof(r_data);
		result = usb_data_write_read(hUSB, s_data, in_len, r_data, &out_len, errinfo);
		if (result < 0)
		{
			out("获取卡信息失败\n");
			return result;
		}


		memset(type_tmp, 0, sizeof(type_tmp));
		memcpy(type_tmp, r_data, 3);
		if (memcmp(type, type_tmp, 3) != 0)
		{
			out("获取到的发票类型与要查询的发票类型不一致\n");
			continue;
		}
		find_inv_type_mem(type_num, &type_mem);
		plate_infos->invoice_type_infos[type_mem].state = 1;
		plate_infos->invoice_type_infos[type_mem].fplxdm = type_num;

		plate_infos->invoice_type_infos[type_mem].monitor_info.state = 1;
		//out("结构内存号%d,发票种类%d,结构体状态%d\n", type_count, type_num, 1);

		memset(time_tmp, 0, sizeof(time_tmp));
		memcpy(time_tmp, r_data + 13, 10);
		cls_character((char *)time_tmp, strlen((const char*)time_tmp), 0X2D);
		memcpy(plate_infos->invoice_type_infos[type_mem].monitor_info.kpjzsj, time_tmp, strlen((const char*)time_tmp));
		//out("结构内存号%d,发票种类%d,开票截止日期%s\n", type_count, type_num, plate_infos->invoice_type_infos[type_mem].monitor_info.kpjzsj);

		char today_month[10] = { 0 };
		char date_range[100] = { 0 };
		char month_first_day[20] = { 0 };
		char month_last_day[20] = { 0 };
		mb_get_today_month(today_month);
		turn_month_to_range((char *)today_month, date_range);
		memcpy(month_first_day, date_range, 8);
		memcpy(month_last_day, date_range + 9, 8);

		memset(time_tmp, 0, sizeof(time_tmp));
		memcpy(time_tmp, r_data + 23, 10);
		cls_character((char *)time_tmp, strlen((const char*)time_tmp), 0X2D);
		memcpy(plate_infos->invoice_type_infos[type_mem].monitor_info.bsqsrq, time_tmp, strlen((const char*)time_tmp));
		if (strcmp((char *)plate_infos->invoice_type_infos[type_mem].monitor_info.bsqsrq,"00000000") == 0)
		{
			//out("刚发行的模拟盘报税起始日期为空，默认改为当月第一天\n");
			strcpy((char *)plate_infos->invoice_type_infos[type_mem].monitor_info.bsqsrq, month_first_day);
		}
		//out("结构内存号%d,发票种类%d,报税起始日期%s\n", type_count, type_num, plate_infos->invoice_type_infos[type_mem].monitor_info.bsqsrq);

		memset(time_tmp, 0, sizeof(time_tmp));
		memcpy(time_tmp, r_data + 33, 10);
		cls_character((char *)time_tmp, strlen((const char*)time_tmp), 0X2D);
		memcpy(plate_infos->invoice_type_infos[type_mem].monitor_info.bszzrq, time_tmp, strlen((const char*)time_tmp));
		if (strcmp((char *)plate_infos->invoice_type_infos[type_mem].monitor_info.bszzrq, "00000000") == 0)
		{
			//out("刚发行的模拟盘报税终止日期为空，默认改为当月最后一天\n");
			strcpy((char *)plate_infos->invoice_type_infos[type_mem].monitor_info.bszzrq, month_last_day);
		}
		//out("结构内存号%d,发票种类%d,报税终止日期%s\n", type_count, type_num, plate_infos->invoice_type_infos[type_mem].monitor_info.bszzrq);

		memset(time_tmp, 0, sizeof(time_tmp));
		memcpy(time_tmp, r_data + 47, 10);
		cls_character((char *)time_tmp, strlen((const char*)time_tmp), 0X2D);
		memcpy(plate_infos->invoice_type_infos[type_mem].monitor_info.zxbsrq, time_tmp, strlen((const char*)time_tmp));
		if (strcmp((char *)plate_infos->invoice_type_infos[type_mem].monitor_info.zxbsrq, "00000000") == 0)
		{
			//out("刚发行的模拟盘最新报税日期为空，默认改为当月第一天\n");
			strcpy((char *)plate_infos->invoice_type_infos[type_mem].monitor_info.zxbsrq, month_first_day);
		}
		//out("结构内存号%d,发票种类%d,最新报税日期%s\n", type_count, type_num, plate_infos->invoice_type_infos[type_mem].monitor_info.zxbsrq);

		memset(money, 0, sizeof(money));
		memcpy(money, r_data + 43, 4);
		money_i = money[0] * 0x1000000 + money[1] * 0x10000 + money[2] * 0x100 + money[3] * 0x1;
		memset(money_s,0,sizeof(money_s));
		sprintf((char *)money_s, "%d", money_i);
		memcpy((char *)plate_infos->invoice_type_infos[type_mem].monitor_info.dzkpxe, money_s, strlen((const char*)money_s) - 2);
		sprintf((char *)plate_infos->invoice_type_infos[type_mem].monitor_info.dzkpxe + strlen((const char*)money_s) - 2, ".");
		memcpy((char *)plate_infos->invoice_type_infos[type_mem].monitor_info.dzkpxe + strlen((const char*)money_s) - 1, money_s + strlen((const char*)money_s) - 2, 2);
		//out("结构内存号%d,发票种类%d,单张发票开票限额%s\n", type_count, type_num, plate_infos->invoice_type_infos[type_mem].monitor_info.dzkpxe);

		memset(money, 0, sizeof(money));
		memcpy(money, r_data + 62, 4);
		money_i = money[0] * 0x1000000 + money[1] * 0x10000 + money[2] * 0x100 + money[3] * 0x1;
		memset(money_s, 0, sizeof(money_s));
		sprintf((char *)money_s, "%d", money_i);
		memcpy((char *)plate_infos->invoice_type_infos[type_mem].monitor_info.lxzsljje, money_s, strlen((const char*)money_s) - 2);
		sprintf((char *)plate_infos->invoice_type_infos[type_mem].monitor_info.lxzsljje + strlen((const char*)money_s) - 2, ".");
		memcpy((char *)plate_infos->invoice_type_infos[type_mem].monitor_info.lxzsljje + strlen((const char*)money_s) - 1, money_s + strlen((const char*)money_s) - 2, 2);
		//out("结构内存号%d,发票种类%d,离线累积开票限额%s\n", type_count, type_num, plate_infos->invoice_type_infos[type_mem].monitor_info.lxzsljje);

		memcpy((char *)plate_infos->invoice_type_infos[type_mem].monitor_info.lxzssyje, plate_infos->invoice_type_infos[type_mem].monitor_info.lxzsljje, 20);

		memset(time, 0, sizeof(time));
		memcpy(time, r_data + 58, 4);
		time_i = time[0] * 0x1000000 + time[1] * 0x10000 + time[2] * 0x100 + time[3] * 0x1;

		plate_infos->invoice_type_infos[type_mem].monitor_info.lxkpsc = time_i;
		//plate_infos->plate_tax_info.lxsc = time_i;
		//out("结构内存号%d,发票种类%d,离线时长%d\n", type_count, type_num, plate_infos->invoice_type_infos[type_mem].monitor_info.lxsc);
	}
	return 0;
}

//查询发票卷信息
int mengbai_read_inv_coil_number(HUSB hUSB, unsigned char *inv_type, struct _plate_infos *plate_infos)
{
	unsigned char type[4];
	unsigned char type_num;
	int type_tmp[4];
	unsigned char data[4];
	int data_i;
	int type_count;
	int coil_count;

	unsigned char time_tmp[20];
	int type_mem;
	unsigned char coil_data[4096];
	int result;
	int in_len, out_len;
	unsigned char s_data[4096];
	unsigned char r_data[4096];
	char errinfo[500] = {0};
	for (type_count = 0; type_count < DF_MAX_INV_TYPE; type_count++)
	{
		if (inv_type[type_count] == 0)
		{
			//out("发票类型不正确\n");
			continue;
		}
		type_num = inv_type[type_count];
		//out("查询发票类型为%d的发票卷数据\n", type_num);
		memset(type, 0, sizeof(type));
		sprintf((char *)type, "%03d", type_num);

		//out("获取发票卷信息\n");
		memset(s_data, 0, sizeof(s_data));
		memset(r_data, 0, sizeof(r_data));
		in_len = pack_s_buf(DF_PC_USB_ORDER_GET_WSYFPJ, 0, type, s_data, strlen((const char*)type));
		out_len = sizeof(r_data);
		result = usb_data_write_read(hUSB, s_data, in_len, r_data, &out_len, errinfo);
		if (result < 0)
		{
			out("获取卡信息失败,发票类型代码为：%d\n",type_num);
			continue;
		}


		memset(type_tmp, 0, sizeof(type_tmp));
		memcpy(type_tmp, r_data, 3);
		if (memcmp(type, type_tmp, 3) != 0)
		{
			out("获取到的发票类型与要查询的发票类型不一致\n");
			continue;
		}
		find_inv_type_mem(type_num, &type_mem);
		plate_infos->invoice_type_infos[type_mem].state = 1;
		plate_infos->invoice_type_infos[type_mem].fplxdm = type_num;

		plate_infos->invoice_type_infos[type_mem].invoice_coils_info.state = 1;
		//out("结构内存号%d,发票种类%d,结构体状态%d\n", type_count, type_num, 1);

		memcpy(plate_infos->invoice_type_infos[type_mem].invoice_coils_info.dqfpdm, r_data + 3, 20);
		//out("结构内存号%d,发票种类%d,当前发票代码%s\n", type_count, type_num, plate_infos->invoice_type_infos[type_mem].invoice_coils_info.dqfpdm);

		memset(data, 0, sizeof(data));
		memcpy(data, r_data + 23, 4);
		data_i = data[0] * 0x1000000 + data[1] * 0x10000 + data[2] * 0x100 + data[3] * 0x1;
		plate_infos->invoice_type_infos[type_mem].invoice_coils_info.dqfphm = data_i;
		//out("结构内存号%d,发票种类%d,当前发票号码%d\n", type_count, type_num, plate_infos->invoice_type_infos[type_mem].invoice_coils_info.dqfphm);

		memset(data, 0, sizeof(data));
		memcpy(data, r_data + 27, 4);
		data_i = data[0] * 0x1000000 + data[1] * 0x10000 + data[2] * 0x100 + data[3] * 0x1;
		plate_infos->invoice_type_infos[type_mem].invoice_coils_info.zsyfpfs = data_i;
		//out("结构内存号%d,发票种类%d,总剩余发票份数%d\n", type_count, type_num, plate_infos->invoice_type_infos[type_mem].invoice_coils_info.zsyfpfs);

		memset(data, 0, sizeof(data));
		memcpy(data, r_data + 31, 1);
		data_i = data[0];
		plate_infos->invoice_type_infos[type_mem].invoice_coils_info.wsyfpjsl = data_i;
		//out("结构内存号%d,发票种类%d,未使用发票卷数量%d\n", type_count, type_num, plate_infos->invoice_type_infos[type_mem].invoice_coils_info.wsyfpjsl);

		if (plate_infos->invoice_type_infos[type_mem].invoice_coils_info.wsyfpjsl > DF_MAX_INV_COILS)
		{
			out("超出程序规定的发票卷数量最大值,强制将发票卷数量改为10卷\n");
			plate_infos->invoice_type_infos[type_mem].invoice_coils_info.wsyfpjsl = DF_MAX_INV_COILS;
		}

		
		for (coil_count = 0; coil_count < plate_infos->invoice_type_infos[type_mem].invoice_coils_info.wsyfpjsl; coil_count++)
		{
			memset(coil_data, 0, sizeof(coil_data));
			memcpy(coil_data, r_data + 32 + coil_count * 52, 52);

			plate_infos->invoice_type_infos[type_mem].invoice_coils_info.invoice_coil[coil_count].state = 1;
			//out("\t结构内存号%d,结构体状态%d\n", coil_count, 1);
			
			memcpy(plate_infos->invoice_type_infos[type_mem].invoice_coils_info.invoice_coil[coil_count].fplbdm, coil_data, 20);
			//out("\t结构内存号%d,发票类别代码%s\n", coil_count, plate_infos->invoice_type_infos[type_mem].invoice_coils_info.invoice_coil[coil_count].fplbdm);

			memset(data, 0, sizeof(data));
			memcpy(data, coil_data+20, 4);
			data_i = data[0] * 0x1000000 + data[1] * 0x10000 + data[2] * 0x100 + data[3] * 0x1;
			plate_infos->invoice_type_infos[type_mem].invoice_coils_info.invoice_coil[coil_count].fpqshm = data_i;
			//out("\t结构内存号%d,发票起始号码%d\n", coil_count, plate_infos->invoice_type_infos[type_mem].invoice_coils_info.invoice_coil[coil_count].fpqshm);

			memset(data, 0, sizeof(data));
			memcpy(data, coil_data+24, 4);
			data_i = data[0] * 0x1000000 + data[1] * 0x10000 + data[2] * 0x100 + data[3] * 0x1;
			plate_infos->invoice_type_infos[type_mem].invoice_coils_info.invoice_coil[coil_count].fpzzhm = data_i;
			//out("\t结构内存号%d,发票终止号码%d\n", coil_count, plate_infos->invoice_type_infos[type_mem].invoice_coils_info.invoice_coil[coil_count].fpzzhm);

			memset(data, 0, sizeof(data));
			memcpy(data, coil_data+28, 4);
			data_i = data[0] * 0x1000000 + data[1] * 0x10000 + data[2] * 0x100 + data[3] * 0x1;
			plate_infos->invoice_type_infos[type_mem].invoice_coils_info.invoice_coil[coil_count].fpsyfs = data_i;
			//out("\t结构内存号%d,剩余发票份数%d\n", coil_count, plate_infos->invoice_type_infos[type_mem].invoice_coils_info.invoice_coil[coil_count].fpsyfs);

			memset(time_tmp, 0, sizeof(time_tmp));
			memcpy(time_tmp, coil_data+32, 10);
			cls_character((char*)time_tmp, strlen((const char*)time_tmp), 0X2D);
			memcpy(plate_infos->invoice_type_infos[type_mem].invoice_coils_info.invoice_coil[coil_count].fpgmsj, time_tmp, strlen((const char*)time_tmp));
			//out("\t结构内存号%d,发票购买日期%s\n", coil_count, plate_infos->invoice_type_infos[type_mem].invoice_coils_info.invoice_coil[coil_count].fpgmsj);

			plate_infos->invoice_type_infos[type_mem].invoice_coils_info.invoice_coil[coil_count].fpgmsl = plate_infos->invoice_type_infos[type_mem].invoice_coils_info.invoice_coil[coil_count].fpzzhm - plate_infos->invoice_type_infos[type_mem].invoice_coils_info.invoice_coil[coil_count].fpqshm + 1;
			//out("\t结构内存号%d,发票购买份数%d\n", coil_count, plate_infos->invoice_type_infos[type_mem].invoice_coils_info.invoice_coil[coil_count].fpgmsl);
		}
	}
	return 0;
}

//获取税盘当前时间
int mengbai_read_plate_time(HUSB hUSB, unsigned char *plate_time)
{
	int result;
	int in_len, out_len;
	unsigned char s_data[4096];
	unsigned char r_data[4096];
	char errinfo[100] = {0};
	//获取税盘时间
	////////////////////////////////////////////////////////////////////////
	//out("获取税盘时间\n");
	memset(s_data, 0, sizeof(s_data));
	memset(r_data, 0, sizeof(r_data));
	in_len = pack_s_buf(DF_PC_USB_ORDER_GET_TIME, 0, NULL, s_data, 0);
	out_len = sizeof(r_data);
	result = usb_data_write_read(hUSB, s_data, in_len, r_data, &out_len, errinfo);
	if (result < 0)
	{
		out("获取卡信息失败\n");
		return result;
	}

	memcpy(plate_time, r_data, 19);
	//out("获取到的税盘时间为：%s\n",plate_time);
	return 0;
}

//获取税盘系统最小时间--仅测试使用
int mengbai_read_plate_least_time(HUSB hUSB, unsigned char *least_time)
{
	int result;
	int in_len, out_len;
	unsigned char s_data[4096];
	unsigned char r_data[4096];
	char errinfo[100] = { 0 };
	//获取税盘时间
	////////////////////////////////////////////////////////////////////////
	out("获取税盘最小时间\n");
	memset(s_data, 0, sizeof(s_data));
	memset(r_data, 0, sizeof(r_data));
	in_len = pack_s_buf(DF_PC_USB_ORDER_GET_L_TIME, 0, NULL, s_data, 0);
	out_len = sizeof(r_data);
	result = usb_data_write_read(hUSB, s_data, in_len, r_data, &out_len, errinfo);
	if (result < 0)
	{
		out("获取税盘最小时间失败\n");
		return result;
	}
	memcpy(least_time, r_data, 19);
	out("获取到的税盘最小时间为：%s\n", least_time);
	return 0;
}

//设置税盘系统时间--仅测试使用
int mengbai_set_plate_time(HUSB hUSB, unsigned char *set_time)
{
	int result;
	int in_len,out_len;
	unsigned char r_data[4096];
	unsigned char s_data[4096];	
	char errinfo[100] = { 0 };
	//设置税盘时间
	////////////////////////////////////////////////////////////////////////
	out("设置税盘时间\n");
	memset(s_data, 0, sizeof(s_data));
	memset(r_data, 0, sizeof(r_data));
	in_len = pack_s_buf(DF_PC_USB_ORDER_SET_TIME, 0, set_time, s_data, strlen((const char *)set_time));
	out_len = sizeof(r_data);
	result = usb_data_write_read(hUSB, s_data, in_len, r_data, &out_len, errinfo);
	if (result < 0)
	{
		out("获取税盘最小时间失败\n");
		return result;
	}
	return 0;
}


//获取授权税率
int mengbai_read_plate_support_slv(HUSB hUSB, unsigned char *inv_type, struct _plate_infos *plate_infos)
{
	unsigned char type[4];
	unsigned char type_num;
	int type_tmp[4];
	char data_tmp[4];
	int type_count;
	int slv_count;
	int type_mem;
	int result;
	int in_len, out_len;
	unsigned char s_data[4096];
	unsigned char r_data[4096];
	char errinfo[100] = { 0 };
	for (type_count = 0; type_count < DF_MAX_INV_TYPE; type_count++)
	{
		if (inv_type[type_count] == 0)
		{
			//out("发票类型不正确\n");
			continue;
		}
		type_num = inv_type[type_count];
		//out("查询发票类型为%d的发票卷数据\n", type_num);
		memset(type, 0, sizeof(type));
		sprintf((char *)type, "%03d", type_num);

		//out("获取授权税率信息\n");
		memset(s_data, 0, sizeof(s_data));
		memset(r_data, 0, sizeof(r_data));
		in_len = pack_s_buf(DF_PC_USB_ORDER_GET_SQSL, 0, type, s_data, strlen((const char*)type));
		out_len = sizeof(r_data);
		result = usb_data_write_read(hUSB, s_data, in_len, r_data, &out_len, errinfo);
		if (result < 0)
		{
			out("获取税盘最小时间失败\n");
			return result;
		}

		memset(type_tmp, 0, sizeof(type_tmp));
		memcpy(type_tmp, r_data, 3);
		if (memcmp(type, type_tmp, 3) != 0)
		{
			out("获取到的发票类型与要查询的发票类型不一致\n");
			continue;
		}
		find_inv_type_mem(type_num, &type_mem);
		plate_infos->invoice_type_infos[type_mem].state = 1;
		plate_infos->invoice_type_infos[type_mem].fplxdm = type_num;


		plate_infos->invoice_type_infos[type_mem].plate_support_slv.state = 1;
		//out("结构内存号%d,发票种类%d,结构体状态%d\n", type_count, type_num, 1);
		
		//print_array(r_data, out_len);

		plate_infos->invoice_type_infos[type_mem].plate_support_slv.slv_sl = r_data[3];
		if (plate_infos->invoice_type_infos[type_mem].plate_support_slv.slv_sl > 6)
			plate_infos->invoice_type_infos[type_mem].plate_support_slv.slv_sl = 6;
		//memset(plate_infos->invoice_type_infos[type_mem].plate_support_slv.slv, 0xff, sizeof(plate_infos->invoice_type_infos[type_mem].plate_support_slv.slv));
		//out("发票种类%03d支持的授权税率数量为%d\n", type_num, plate_infos->invoice_type_infos[type_mem].plate_support_slv.slv_sl);
		for (slv_count = 0; slv_count < plate_infos->invoice_type_infos[type_mem].plate_support_slv.slv_sl; slv_count++)
		{
			memcpy(data_tmp, r_data + 4 + slv_count * 4, 4);
			plate_infos->invoice_type_infos[type_mem].plate_support_slv.slv[slv_count] = data_tmp[0] * 0x1000000 + data_tmp[1] * 0x10000 + data_tmp[2] * 0x100 + data_tmp[3] * 0x1;	
			//out("发票种类%03d支持的授权税率%d号的税率为%d\n", type_num, slv_count + 1, plate_infos->invoice_type_infos[type_mem].plate_support_slv.slv[slv_count]);
		}
		
	}
	return 0;
}

//查询某时间断发票数量
int mengbai_read_invoice_num(HUSB hUSB, unsigned char *month, unsigned int *count, unsigned long *count_size)
{
	unsigned char date_range[32];
	int invoice_num;
	int result;
	int in_len, out_len;
	unsigned char s_data[4096];
	unsigned char r_data[4096];
	char errinfo[100] = { 0 };
	memset(s_data, 0, sizeof(s_data));
	memset(r_data, 0, sizeof(r_data));

	out("查询月份szFormatMonth = %s\n", month);
	out("转换月份成时间段\n");
	memset(date_range, 0, sizeof(date_range));
	month_to_month_range(month, date_range);
	//获取发票明细数量
	////////////////////////////////////////////////////////////////////////
	out("获取发票明细数量\n");
	in_len = pack_s_buf(DF_PC_USB_ORDER_GET_FPMXSL, 0, date_range, s_data, strlen((const char *)date_range));
	out_len = sizeof(r_data);
	result = usb_data_write_read(hUSB, s_data, in_len, r_data, &out_len, errinfo);
	if (result < 0)
	{
		out("获取卡信息失败\n");
		return result;
	}
	invoice_num = r_data[0] * 0x1000000 + r_data[1] * 0x10000 + r_data[2] * 0x100 + r_data[3] * 0x1;
	*count = invoice_num;
	*count_size = 30000 * invoice_num;
	return invoice_num;
}


//查询某时间断发票明细MQTT传出
int mengbai_read_inv_details_mqtt(HDEV hDev, unsigned char *month, void *function, void *arg, char *errinfo, int old_new, unsigned short *start_stop)
{

	unsigned char date_range[32];

	int result;
	int in_len, r_len;
	unsigned char s_data[4096];
	unsigned char r_data[4096];
	//unsigned char inv_info[4096];
	unsigned char out_data[8000];
	int total_num;
	int now_num;
	unsigned char *send_data;
	void *hook_arg;                                             //应用参数
	int(*hook)(unsigned char *s_data, int total_num, int now_num, void *hook_arg, char *plate_num);			//应用回调函数

	int max_len = 1 * 1024 * 1024;//最大数据超过1M即回调发送
	int over_time = 5;//发票数据获取时间超过5秒即回调发送
	long begin_time;
	long now_time;
	long index =0;
	long inv_len;
	int now_count =0;

	send_data = (uint8 *)malloc(max_len * 10); //发票数据缓冲区10M
	memset(send_data, 0, max_len * 10);

	hook = function;
	hook_arg = arg;

	//out("查询月份szFormatMonth = %s\n", month);
	//out("转换月份成时间段\n");
	memset(date_range, 0, sizeof(date_range));
	month_to_month_range(month, date_range);

	memset(s_data, 0, sizeof(s_data));
	memset(r_data, 0, sizeof(r_data));
	in_len = pack_s_buf(DF_PC_USB_ORDER_GET_FPMX, 0, date_range, s_data, strlen((const char *)date_range));

	usb_data_write(hDev->hUSB, s_data, in_len);
	begin_time = get_time_sec();
	*start_stop = 1;
	for (;;)
	{
		if (*start_stop != 1)
		{
			out("停止正在查询%s月发票详情动作\n", (char *)month);
			break;
		}
		memset(r_data, 0, sizeof(r_data));
		r_len = sizeof(r_data);
		result = usb_data_read(hDev->hUSB, r_data, &r_len);
		if (result < 0)
		{
			out("发票数据读取失败\n");			
			break;
		}
		total_num = r_data[0] * 0x1000000 + r_data[1] * 0x10000 + r_data[2] * 0x100 + r_data[3];
		//out("当月发票总数为：%d\n", total_num);
		now_num = r_data[4] * 0x1000000 + r_data[5] * 0x10000 + r_data[6] * 0x100 + r_data[7];
		//out("当前发票号为：%d\n", now_num);
		//memcpy(inv_info, r_data + 8, out_len - 8);
		//print_array((char *)inv_info, out_len - 8);
		//out("将发票数据推送至客户端\n");
		//out("调用回调函数\n");
		memset(out_data, 0, sizeof(out_data));
		if (old_new == 0)
		{
			result = pack_get_inv_json(r_data, r_len, out_data, (uint8 *)hDev->szCompanyName, (uint8 *)hDev->szCommonTaxID);
		}
		else
		{
			uint8 zfbz;
			result = pack_get_inv_json_new(r_data, r_len, out_data, &zfbz, (uint8 *)hDev->szDeviceID, (uint8 *)hDev->szCompanyName, (uint8 *)hDev->szCommonTaxID);
		}
		if (result < 0)
		{
			out("发票数据解析失败\n");
			break;
		}
		//out("发布打包成功,base64data= %s\n", out_data);
		memcpy(send_data + index, r_data+4, 4);
		inv_len = strlen((const char *)out_data);
		send_data[index + 4] = ((inv_len >> 24) & 0xff); send_data[index + 5] = ((inv_len >> 16) & 0xff);
		send_data[index + 6] = ((inv_len >> 8) & 0xff); send_data[index + 7] = ((inv_len >> 0) & 0xff);
		memcpy(send_data + 8 + index, out_data, inv_len);
		index += 8 + inv_len;
		now_count += 1;
		if (total_num == now_num)
		{
			hook(send_data, total_num, now_count, hook_arg, hDev->szDeviceID);
			//out("所有发票数据读取完成\n");
			break;
		}
		if (index > max_len)
		{
			//out("发票数据量大于1M\n");
			hook(send_data, total_num, now_count, hook_arg, hDev->szDeviceID);
			memset(send_data, 0, max_len * 10);
			begin_time = get_time_sec();
			now_count = 0;
			index = 0;
		}
		now_time = get_time_sec();
		if (now_time - begin_time > over_time)
		{
			//out("获取时间超过5秒\n");
			hook(send_data, total_num, now_count, hook_arg, hDev->szDeviceID);
			memset(send_data, 0, max_len * 10);
			begin_time = get_time_sec();
			now_count = 0;
			index = 0;
		}		
	}
	*start_stop = 0;
	if (send_data)
		free(send_data);
	return 0;
}


int mengbai_read_inv_to_mengbai_server(HUSB hUSB, unsigned char *month, void *function, void *arg, struct _plate_infos *plate_infos, int common_num, int ser_now_num, unsigned short *start_stop)
{

	unsigned char date_range[32];

	int result;
	int in_len, r_len;
	unsigned char s_data[4096];
	unsigned char r_data[4096];
	//unsigned char inv_info[4096];
	unsigned char out_data[8000];
	int total_num;
	int now_num=0;
	unsigned char *send_data;

	int max_len = 1 * 1024 * 1024;//最大数据超过1M即回调发送
	int over_time = 5;//发票数据获取时间超过5秒即回调发送
	long begin_time;
	long now_time;
	long index = 0;
	long inv_len;
	int now_count = 0;
	void *hook_arg;                                             //应用参数
	int(*hook)(unsigned char *s_data, int total_num, int now_num, void *hook_arg, char *plate_num,uint8 *month);			//应用回调函数
	send_data = (uint8 *)malloc(max_len * 10); //发票数据缓冲区10M
	memset(send_data, 0, max_len * 10);

	hook = function;
	hook_arg = arg;

	//out("查询月份szFormatMonth = %s\n", month);
	//out("转换月份成时间段\n");
	memset(date_range, 0, sizeof(date_range));
	month_to_month_range(month, date_range);

	memset(s_data, 0, sizeof(s_data));
	memset(r_data, 0, sizeof(r_data));
	in_len = pack_s_buf(DF_PC_USB_ORDER_GET_FPMX, 0, date_range, s_data, strlen((const char *)date_range));

	usb_data_write(hUSB, s_data, in_len);
	begin_time = get_time_sec();
	*start_stop = 1;
	for (;;)
	{
		//if (*start_stop != 1)
		//{
		//	out("停止正在查询%s月发票详情动作\n", (char *)month);
		//	break;
		//}
		memset(r_data, 0, sizeof(r_data));
		r_len = sizeof(r_data);
		result = usb_data_read(hUSB, r_data, &r_len);
		if (result < 0)
		{
			out("发票数据读取失败\n");
			break;
		}
		total_num = r_data[0] * 0x1000000 + r_data[1] * 0x10000 + r_data[2] * 0x100 + r_data[3];
		//if (total_num == common_num)
		//{
		//	return 0;
		//}
		//out("%s月发票总数为%d张\n", month, total_num);
		if (total_num < 0)
		{
			out("发票数据读取失败,总数有误\n", month);
			break;
		}
		else if (total_num == 0)
		{
			out("%s月发票总数为0\n", month);
			break;
		}




		//out("当月发票总数为：%d\n", total_num);
		now_num = r_data[4] * 0x1000000 + r_data[5] * 0x10000 + r_data[6] * 0x100 + r_data[7];

		if (now_num <= 0)
		{
			out("当前发票号为：%d,有误\n", now_num);
			break;
		}
		//out("当前发票号为：%d\n", now_num);
		//memcpy(inv_info, r_data + 8, out_len - 8);
		//print_array((char *)inv_info, out_len - 8);
		//out("将发票数据推送至客户端\n");
		//out("调用回调函数\n");
		memset(out_data, 0, sizeof(out_data));
		uint8 zfbz;
		result = pack_get_inv_json_new(r_data, r_len, out_data, &zfbz,plate_infos->plate_basic_info.plate_num, plate_infos->plate_basic_info.ca_name, plate_infos->plate_basic_info.ca_number);
		if (result < 0)
		{
			out("发票数据解析失败\n");
			break;
		}

		if (now_num-1 < ser_now_num)
		{	

			if (zfbz == 1)
			{
				;// out("当前发票%d为作废发票需重新上传\n", now_num);
			}
			else
			{

				//out("已上传过无需重复上传第%d张,总数%d,当前%d\n", now_num, total_num, now_num);
				if (total_num == now_num)
				{
					break;
				}
				continue;
			}
		}

		//out("发布打包成功,base64data= %s\n", out_data);
		memcpy(send_data + index, r_data + 4, 4);
		inv_len = strlen((const char *)out_data);
		send_data[index + 4] = ((inv_len >> 24) & 0xff); send_data[index + 5] = ((inv_len >> 16) & 0xff);
		send_data[index + 6] = ((inv_len >> 8) & 0xff); send_data[index + 7] = ((inv_len >> 0) & 0xff);
		memcpy(send_data + 8 + index, out_data, inv_len);
		index += 8 + inv_len;
		now_count += 1;
		if (total_num == now_num)
		{
			hook(send_data, 0, now_count, hook_arg, (char *)plate_infos->plate_basic_info.plate_num, month);
			//out("所有发票数据读取完成\n");
			break;
		}
		if (index > max_len)
		{
			//out("发票数据量大于1M\n");
			hook(send_data, 0, now_count, hook_arg, (char *)plate_infos->plate_basic_info.plate_num, month);
			memset(send_data, 0, max_len * 10);
			begin_time = get_time_sec();
			now_count = 0;
			index = 0;
		}
		now_time = get_time_sec();
		if (now_time - begin_time > over_time)
		{
			//out("获取时间超过5秒\n");
			hook(send_data, 0, now_count, hook_arg, (char *)plate_infos->plate_basic_info.plate_num, month);
			memset(send_data, 0, max_len * 10);
			begin_time = get_time_sec();
			now_count = 0;
			index = 0;
		}
	}

	//if (*start_stop == 1)
	//{
		//out("当前发票正常读取完成,非中断结束,通知M服务当前月读取完成\n");
		hook(NULL, 1, now_num, hook_arg, (char *)plate_infos->plate_basic_info.plate_num, month);

	//}

	*start_stop = 0;
	if (send_data)
		free(send_data);
	return 0;
}


static int pack_get_inv_json_fpxx(unsigned char *in_data, int in_len, unsigned char *out_data, uint8 *zfbz, unsigned char *plate_num, unsigned char *ca_name, unsigned char *ca_number, HFPXX fpxx)
{
	int sp_count;
	//HFPXX fpxx = MallocFpxx();
	//struct Spxx spxx;
	char sp_data[4096];
	//memset(&fpxx, 0, sizeof(struct Fpxx));

	//out("发票数据：\n");
	//print_array(in_data, in_len);
	//memset(tmp, 0, sizeof(tmp));
	//memcpy(tmp, in_data + 0, 4);
	//*total_num = tmp[0] * 0x1000000 + tmp[1] * 0x10000 + tmp[2] * 0x100 + tmp[3] * 0x1;


	//memset(tmp, 0, sizeof(tmp));
	//memcpy(tmp, in_data + 4, 4);
	//*now_num = tmp[0] * 0x1000000 + tmp[1] * 0x10000 + tmp[2] * 0x100 + tmp[3] * 0x1;
	//out("发票总数为：%d,当前序号为：%d\n", *total_num, *now_num);
	char fplx_tmp[20] = { 0 };
	memcpy(fplx_tmp, in_data + 8, 3);
	fpxx->fplx = atoi(fplx_tmp);
	memcpy(fpxx->kpsj_F2, in_data + 11, 19);
	memcpy(fpxx->fpdm, in_data + 30, 20);
	memcpy(fpxx->fphm, in_data + 50, 8);
	memcpy(fpxx->kplx, in_data + 58, 1);
	//memcpy(fpxx.zfbz_s, in_data + 59, 1);
	if (in_data[59] == 0x30)
		fpxx->zfbz = 0;
	else if (in_data[59] == 0x31)
		fpxx->zfbz = 1;
	else
		fpxx->zfbz = 0;
	*zfbz = fpxx->zfbz;
	if (fpxx->zfbz == 1)
		memcpy(fpxx->zfsj_F2, in_data + 60, 19);
	memcpy(fpxx->gfsh, in_data + 79, 20);
	memcpy(fpxx->gfmc, in_data + 99, 80);
	memcpy(fpxx->je, in_data + 179, 20);
	memcpy(fpxx->se, in_data + 199, 20);
	memcpy(fpxx->jshj, in_data + 219, 20);
	memcpy(fpxx->bz, in_data + 239, 20);
	memcpy(fpxx->skr, in_data + 259, 10);
	memcpy(fpxx->kpr, in_data + 269, 10);
	memcpy(fpxx->blueFpdm, in_data + 279, 20);
	char yfphm[20] = { 0 };
	memcpy(yfphm, in_data + 299, 8);
	if (atoi(yfphm) != 0)
	{
		sprintf(fpxx->blueFpdm, "%s", yfphm);
	}
	//memcpy(fpxx.spsl, in_data + 307, 2);
	sprintf(fpxx->bmbbbh, "33.0");
	sprintf(fpxx->hsjbz, "0");
	sprintf(fpxx->qdbj, "N");
	sprintf(fpxx->xfmc, "%s", ca_name);
	sprintf(fpxx->xfsh, "%s", ca_number);
	sprintf(fpxx->xfdzdh, "仅供测试使用");
	sprintf(fpxx->xfyhzh, "仅供测试使用");
	sprintf(fpxx->gfdzdh, "仅供测试使用");
	sprintf(fpxx->gfyhzh, "仅供测试使用");
	sprintf(fpxx->jym, "01234567890123456789");
	sprintf(fpxx->mw, "<*/*0000000000111111111122222222223333333333444444444455555555556666666666777777777788888888889999999999*/*>");
	

	char spsl_s[10] = { 0 };
	memcpy(spsl_s, in_data + 307, 2);
	fpxx->spsl = atoi(spsl_s);

	for (sp_count = 0; sp_count < fpxx->spsl; sp_count++)
	{
		struct Spxx *Head = (struct Spxx *)calloc(1, sizeof(struct Spxx));

		memset(sp_data, 0, sizeof(sp_data));
		memcpy(sp_data, in_data + 309 + sp_count * 232, 232);
		char spsmmc_spmc[200] = { 0 };
		memcpy(spsmmc_spmc, sp_data + 0, 40);
		exchange_spmc_spsmmc(spsmmc_spmc, sizeof(spsmmc_spmc));
		GetSpsmmcSpmc(spsmmc_spmc, Head->spsmmc, Head->spmc);
		if (sp_count == 0)
		{
			memcpy(fpxx->zyspsmmc, Head->spsmmc, strlen(Head->spsmmc));
			memcpy(fpxx->zyspmc, Head->spmc, strlen(Head->spmc));
		}

		memcpy(Head->dj, sp_data + 40, 20);
		memcpy(Head->sl, sp_data + 60, 20);
		memcpy(Head->je, sp_data + 80, 20);
		memcpy(Head->jldw, sp_data + 100, 22);
		memcpy(Head->ggxh, sp_data + 122, 40);
		memcpy(Head->se, sp_data + 162, 20);
		char slv_s[10] = { 0 };
		float slv_f;
		memcpy(slv_s, sp_data + 182, 10);
		slv_f = atof(slv_s) / 100;
		sprintf(Head->slv, "%4.2f", slv_f);
		memcpy(Head->spbh, sp_data + 192, 40);

		sprintf(Head->xsyh, "0");
		sprintf(Head->fphxz, "0");
		sprintf(Head->hsjbz, "0");

		InsertMxxx(fpxx, Head);

	}

	char *g_buf;
	struct StaticDeviceInfo pDevInfo;
	memset(&pDevInfo, 0, sizeof(struct StaticDeviceInfo));
	pDevInfo.bDeviceType = 3;
	memcpy(pDevInfo.szDeviceID, plate_num, strlen((char *)plate_num));
	fpxx->hDev = &pDevInfo;
	fpxx_to_json_base_fpsjbbh_v102(fpxx, &g_buf, DF_FPQD_Y, DF_FP_JSON);

	memcpy(out_data, g_buf, strlen(g_buf));
	//FreeFpxx(fpxx);
	free(g_buf);
	return strlen((const char*)out_data);
}

#ifdef DF_OLD_MSERVER
static int mengbai_get_invoice_month_upload_hook(HDEV hDev, uint8 *month, void *function, void *arg, struct _upload_inv_num *upload_inv_num, unsigned short *start_stop, struct _inv_sum_data *now_month_sum)
{

	unsigned char date_range[32];

	int result;
	int in_len, r_len;
	unsigned char s_data[4096];
	unsigned char r_data[4096];
	//unsigned char inv_info[4096];
	unsigned char out_data[8000];
	int total_num;
	int now_num = 0;
	unsigned char *send_data;
	unsigned char *lxp_data;
	int up_num=0;
	int max_len = 1 * 1024 * 1024;//最大数据超过1M即回调发送
	int over_time = 5;//发票数据获取时间超过5秒即回调发送
	long begin_time;
	long now_time;
	long index = 0;
	long inv_len;
	int now_count = 0;
	char today_month[10] = { 0 };
	char *year_month_data = NULL;
	int use_http_data = 0;
	void *hook_arg;                                             //应用参数
	int(*hook)(unsigned char *s_data, int total_num, int now_num, void *hook_arg, char *plate_num, uint8 *month, char *statistics, int complete_err);			//应用回调函数
	struct _inv_sum_data inv_sum;
	memset(&inv_sum, 0, sizeof(struct _inv_sum_data));
	send_data = (uint8 *)malloc(max_len * 10); //发票数据缓冲区10M
	if (send_data == NULL)
		return -1;
	memset(send_data, 0, max_len * 10);
	lxp_data = (uint8 *)malloc(max_len);
	memset(lxp_data, 0, max_len);
	mb_get_today_month(today_month);
	hook = function;
	hook_arg = arg;

	if ((strcmp(upload_inv_num->year_month, today_month) == 0) && upload_inv_num->now_num > 0 && strlen(upload_inv_num->year_month_data) == 0)
	{

		_WriteLog(LL_INFO, "%s税盘当月%s发票数据有%d张,可能存在不一致问题,需查询服务器再对齐\n", hDev->szCompanyName, month, upload_inv_num->now_num);
		result = get_plate_invs_sync(hDev->szDeviceID, &year_month_data);
		if (result < 0)
		{
			use_http_data = 0;
			_WriteLog(LL_INFO, "%s税盘当月%s发票数据有%d张,可能存在不一致问题,与服务器获取对齐失败\n", hDev->szCompanyName, month, upload_inv_num->now_num);
		}
		else
			use_http_data = 1;

	}

	//out("查询月份szFormatMonth = %s\n", month);
	//out("转换月份成时间段\n");
	memset(date_range, 0, sizeof(date_range));
	month_to_month_range(month, date_range);

	memset(s_data, 0, sizeof(s_data));
	memset(r_data, 0, sizeof(r_data));
	in_len = pack_s_buf(DF_PC_USB_ORDER_GET_FPMX, 0, date_range, s_data, strlen((const char *)date_range));

	usb_data_write(hDev->hUSB, s_data, in_len);
	begin_time = get_time_sec();
	for (;;)
	{

		memset(r_data, 0, sizeof(r_data));
		r_len = sizeof(r_data);
		result = usb_data_read(hDev->hUSB, r_data, &r_len);
		if (result < 0)
		{
			out("发票数据读取失败\n");
			break;
		}
		total_num = r_data[0] * 0x1000000 + r_data[1] * 0x10000 + r_data[2] * 0x100 + r_data[3];

		//out("%s月发票总数为%d张\n", month, total_num);
		if (total_num <= 0)
		{
			out("%s月发票数据读取失败,总数有误或为0\n", month);
			break;
		}
		
		
		now_num = r_data[4] * 0x1000000 + r_data[5] * 0x10000 + r_data[6] * 0x100 + r_data[7];

		if (now_num <= 0)
		{
			out("当前发票号为：%d,有误\n", now_num);
			break;
		}
		//out("当前发票号为：%d\n", now_num);
		//memcpy(inv_info, r_data + 8, out_len - 8);
		//print_array((char *)inv_info, out_len - 8);
		//out("将发票数据推送至客户端\n");
		//out("调用回调函数\n");
		memset(out_data, 0, sizeof(out_data));
		uint8 zfbz;
		HFPXX fpxx = MallocFpxx();
		result = pack_get_inv_json_fpxx(r_data, r_len, out_data, &zfbz, (uint8 *)hDev->szDeviceID, (uint8 *)hDev->szCompanyName, (uint8 *)hDev->szCommonTaxID, fpxx);
		if (result < 0)
		{
			out("发票数据解析失败\n");
			FreeFpxx(fpxx);
			break;
		}
		add_inv_sum_data(fpxx, &inv_sum);
		fpxx->bIsUpload = 1;
		//_WriteLog(LL_INFO, "处理第 %d 张发票数据发票代码为%s 发票号码为 %s,作废标志 %d,开票时间 %s\n", now_num, fpxx->fpdm, fpxx->fphm, fpxx->zfbz, fpxx->kpsj_F2);
		//if (fpxx->bIsUpload == 0)
		//{
		//	if (strlen((char *)lxp_data) < max_len - 100)
		//	{
		//		sprintf((char *)lxp_data + strlen((char *)lxp_data), "%s,%s;", fpxx->fpdm, fpxx->fphm);
		//	}
		//}
		int need_up = jude_need_upload_inv(upload_inv_num, fpxx);
		if (use_http_data == 1)
		{
			if (year_month_data != NULL)
			{
				if ((need_up == 0) && (strlen(year_month_data) != 0))
				{
					//_WriteLog(LL_INFO, "处理第 %d 张发票数据,判断服务器端是否已经入库\n", inv_count + 1);
					char fpdm_fphm_up_off_str[50] = { 0 };
					memset(fpdm_fphm_up_off_str, 0, sizeof(fpdm_fphm_up_off_str));
					sprintf(fpdm_fphm_up_off_str, "%s,%s,%d,%d;", fpxx->fpdm, fpxx->fphm, fpxx->zfbz, fpxx->bIsUpload);

					if (str_replace(year_month_data, fpdm_fphm_up_off_str, ";") != 1)
					{
						_WriteLog(LL_INFO, "%s税盘处理第 %d 张发票数据,未找到服务器入库信息,可能存在错误需重新上传,%s\n", hDev->szCompanyName, now_num, fpdm_fphm_up_off_str);
						need_up = 1;
					}
				}
			}
		}
		else
		{
			if ((need_up == 0) && (strlen(upload_inv_num->year_month_data) != 0))
			{
				//_WriteLog(LL_INFO, "处理第 %d 张发票数据,判断服务器端是否已经入库\n", now_num);
				char fpdm_fphm_up_off_str[50] = { 0 };
				memset(fpdm_fphm_up_off_str, 0, sizeof(fpdm_fphm_up_off_str));
				fpxx->bIsUpload = 1;
				sprintf(fpdm_fphm_up_off_str, "%s,%s,%d,%d;", fpxx->fpdm, fpxx->fphm, fpxx->zfbz, fpxx->bIsUpload);

				if (str_replace(upload_inv_num->year_month_data, fpdm_fphm_up_off_str, ";") != 1)
				{
					_WriteLog(LL_INFO, "处理第 %d 张发票数据,未找到服务器入库信息,可能存在错误需重新上传,%s\n", now_num, fpdm_fphm_up_off_str);
					need_up = 1;
				}
			}
		}
		if (need_up != 1)
		{
			FreeFpxx(fpxx);
			continue;
		}
		//_WriteLog(LL_INFO, "处理第 %d 张发票数据判定该张发票需上传服务器，发票代码为%s 发票号码为 %s\n", now_num, fpxx->fpdm, fpxx->fphm);
		FreeFpxx(fpxx);

		up_num += 1;
		//out("发布打包成功,base64data= %s\n", out_data);
		memcpy(send_data + index, r_data + 4, 4);
		inv_len = strlen((const char *)out_data);
		send_data[index + 4] = ((inv_len >> 24) & 0xff); send_data[index + 5] = ((inv_len >> 16) & 0xff);
		send_data[index + 6] = ((inv_len >> 8) & 0xff); send_data[index + 7] = ((inv_len >> 0) & 0xff);
		memcpy(send_data + 8 + index, out_data, inv_len);
		index += 8 + inv_len;
		now_count += 1;
		if (total_num == now_num)
		{
			hook(send_data, 0, now_count, hook_arg, (char *)hDev->szDeviceID, month, NULL,0);
			up_num = 0;
			//out("所有发票数据读取完成\n");
			break;
		}
		if (index > max_len)
		{
			//out("发票数据量大于1M\n");
			hook(send_data, 0, now_count, hook_arg, (char *)hDev->szDeviceID, month, NULL,0);
			up_num = 0;
			memset(send_data, 0, max_len * 10);
			begin_time = get_time_sec();
			now_count = 0;
			index = 0;
		}
		now_time = get_time_sec();
		if (now_time - begin_time > over_time)
		{
			//out("获取时间超过5秒\n");
			hook(send_data, 0, now_count, hook_arg, (char *)hDev->szDeviceID, month, NULL,0);
			up_num = 0;
			memset(send_data, 0, max_len * 10);
			begin_time = get_time_sec();
			now_count = 0;
			index = 0;
		}
	}
	if (up_num != 0)
	{
		hook(send_data, 0, now_count, hook_arg, (char *)hDev->szDeviceID, month, NULL,0);
		up_num = 0;
	}

	out("当前发票正常读取完成,非中断结束,通知M服务当前月读取完成\n");

	char time_now[50] = {0};
	memcpy(time_now, hDev->szDeviceTime,4);
	memcpy(time_now+strlen(time_now), hDev->szDeviceTime+5, 2);
	if (memcmp(month, hDev->szDeviceTime, 6) == 0)
	{
		memcpy(now_month_sum, &inv_sum, sizeof(struct _inv_sum_data));
	}

	hook(lxp_data, 1, now_num, hook_arg, (char *)hDev->szDeviceID, month, NULL,0);
	up_num = 0;
	if (year_month_data != NULL)
	{
		free(year_month_data);
	}
	free(send_data);
	free(lxp_data);
	return 0;
}

int mengbai_upload_m_server(HDEV hDev, unsigned short *start_stop, struct _upload_inv_num *upload_inv_num, void *function, void *arg, struct _inv_sum_data *now_month_sum)
{
	int result;
	char today_month[10] = { 0 };
	char year_month[100] = { 0 };
	if (strcmp(upload_inv_num->plate_num, hDev->szDeviceID) != 0)
	{
		return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
	}
	if ((upload_inv_num->state == 0) && (upload_inv_num->need_fpdmhm_flag == 1))
	{
		//_WriteLog(LL_INFO,"需要按发票代码号码上传发票");
		/////后续上传操作
		//nisec_get_invoice_single_upload_hook(device, function, arg, upload_inv_num);
		return 0;

	}
	if (upload_inv_num->state != 1)
	{
		//_WriteLog(LL_INFO,"无需上传发票");
		return 0;
	}
	//_WriteLog(LL_INFO,"需要按时间段上传发票\n");
	mb_get_today_month(today_month);
	*start_stop = 1;
	strcpy(year_month, upload_inv_num->year_month);
	if (strlen(year_month) == 0 || (strcmp(year_month, "201801") < 0))
	{
		memset(year_month, 0, sizeof(year_month));
		sprintf(year_month, "201801");
		if (upload_inv_num->just_single_month == 1)
		{
			return 0;
		}
	}

	for (; strcmp(year_month, today_month) <= 0; mb_get_next_month(year_month))
	{
		//_WriteLog(LL_INFO, "读取%s月发票并上传", year_month);
		if (*start_stop != 1)
		{
			_WriteLog(LL_INFO, "停止正在查询%s月发票详情动作\n", (char *)year_month);
			break;
		}
		result = mengbai_get_invoice_month_upload_hook(hDev, (uint8 *)year_month, function, arg, upload_inv_num, start_stop, now_month_sum);
		if ((result < 0) && (*start_stop != 1))
		{
			_WriteLog(LL_INFO, "非正常结束,由外部中断,结束按月查询上传循环\n");
			break;
		}
		if (upload_inv_num->just_single_month == 1)
		{
			return 0;
		}

	}
	if (*start_stop != 1)
	{
		_WriteLog(LL_INFO, "非正常结束,由外部中断结束\n");
		return -1;
	}
	return 0;

}
#else
static int mengbai_get_invoice_month_upload_hook(HDEV hDev, char *month, void *function, void *arg, struct _upload_inv_num *upload_inv_num, unsigned short *start_stop, struct _inv_sum_data *now_month_sum)
{

	unsigned char date_range[32];

	int result;
	int in_len, r_len;
	unsigned char s_data[4096];
	unsigned char r_data[4096];
	struct stat statbufs;
	int zip_data_len = 0;
	unsigned char inv_data[8000];
	int total_num;
	int now_num = 0;
	int need_up;
	int need_up_count = 0;
	char today_month[10] = { 0 };
	char fp_zip_name[100] = { 0 };
	char fp_txt_name[100] = { 0 };
	char fphm_crc[50] = { 0 };
	struct _inv_sum_data inv_sum;
	zipFile zf = NULL;
	memset(&inv_sum, 0, sizeof(struct _inv_sum_data));
	
	mb_get_today_month(today_month);
	sprintf(fp_zip_name, "/tmp/%s.zip", hDev->szDeviceID);
	if (file_exists(fp_zip_name) == 0)
	{
		_WriteLog(LL_INFO, "%s税盘处理发票数据前存在压缩文件先删除\n", hDev->szCompanyName);
		delete_file(fp_zip_name);
	}
	zf = zipOpen64(fp_zip_name, 0);
	if (zf == NULL)
	{
		_WriteLog(LL_INFO, "zipOpen64 compress file:%s fail!\n", fp_zip_name);
		return 	DF_TAX_ERR_CODE_SYSTEM_MEM_INSUFFICIENT;
	}

	//out("查询月份szFormatMonth = %s\n", month);
	//out("转换月份成时间段\n");
	memset(date_range, 0, sizeof(date_range));
	month_to_month_range((uint8 *)month, date_range);

	memset(s_data, 0, sizeof(s_data));
	memset(r_data, 0, sizeof(r_data));
	in_len = pack_s_buf(DF_PC_USB_ORDER_GET_FPMX, 0, date_range, s_data, strlen((const char *)date_range));

	usb_data_write(hDev->hUSB, s_data, in_len);
	for (;;)
	{

		memset(r_data, 0, sizeof(r_data));
		r_len = sizeof(r_data);
		result = usb_data_read(hDev->hUSB, r_data, &r_len);
		if (result < 0)
		{
			out("发票数据读取失败\n");
			break;
		}
		total_num = r_data[0] * 0x1000000 + r_data[1] * 0x10000 + r_data[2] * 0x100 + r_data[3];

		//out("%s月发票总数为%d张\n", month, total_num);
		if (total_num <= 0)
		{
			out("%s月发票数据读取失败,总数有误或为0\n", month);
			break;
		}
		
		
		now_num = r_data[4] * 0x1000000 + r_data[5] * 0x10000 + r_data[6] * 0x100 + r_data[7];

		if (now_num <= 0)
		{
			out("当前发票号为：%d,有误\n", now_num);
			break;
		}
		//out("当前发票号为：%d\n", now_num);
		//memcpy(inv_info, r_data + 8, out_len - 8);
		//print_array((char *)inv_info, out_len - 8);
		//out("将发票数据推送至客户端\n");
		//out("调用回调函数\n");
		memset(inv_data, 0, sizeof(inv_data));
		uint8 zfbz;
		HFPXX fpxx = MallocFpxx();
		result = pack_get_inv_json_fpxx(r_data, r_len, inv_data, &zfbz, (uint8 *)hDev->szDeviceID, (uint8 *)hDev->szCompanyName, (uint8 *)hDev->szCommonTaxID, fpxx);
		if (result < 0)
		{
			out("发票数据解析失败\n");
			FreeFpxx(fpxx);
			break;
		}
		add_inv_sum_data(fpxx, &inv_sum);
		fpxx->bIsUpload = 1;
		
		uint16 crc = crc_8005((uint8 *)inv_data, strlen((char *)inv_data), 0);
		char invoice_crc[10] = { 0 };
		sprintf(invoice_crc, "%04x", crc);

		need_up = 1;

		if (upload_inv_num->dmhmzfsbstr != NULL)
		{
			if (strlen(upload_inv_num->dmhmzfsbstr) != 0)
			{
				//_WriteLog(LL_INFO, "处理第 %d 张发票数据,判断服务器端是否已经入库\n", inv_count + 1);
				
				memset(fphm_crc, 0, sizeof(fphm_crc));
				sprintf(fphm_crc, "%s%s;", fpxx->fphm, invoice_crc);

				if (delete_str(upload_inv_num->dmhmzfsbstr, fphm_crc) == 1)
				{
					need_up = 0;
					//_WriteLog(LL_INFO, "查找%s数据成功,发票存在且无变动,不需重传发票\n", fphm_crc);
				}
			}
		}
		if (need_up != 1)
		{
			FreeFpxx(fpxx);
			continue;
		}
		_WriteLog(LL_INFO, "查找%s_%s数据成功crc= %s,需重传发票\n", fpxx->fpdm, fpxx->fphm, fphm_crc);
		need_up_count += 1;


		memset(fp_txt_name, 0, sizeof(fp_txt_name));
		sprintf(fp_txt_name, "%s_%s_%s.txt", fpxx->fpdm, fpxx->fphm, invoice_crc);

		zip_fileinfo zi;
		memset(&zi, 0, sizeof(zip_fileinfo));
		zipOpenNewFileInZip3_64(zf, fp_txt_name, &zi, NULL, 0, NULL, 0, NULL, Z_DEFLATED, 9, 0, -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY, NULL, 0, 0);
		zipWriteInFileInZip(zf, inv_data, strlen((char *)inv_data));
		zipCloseFileInZip(zf);
		stat(fp_zip_name, &statbufs);
		zip_data_len = statbufs.st_size;
		_WriteLog(LL_INFO, "压缩第%d张发票后,压缩包大小为%d字节", now_num, zip_data_len);


		FreeFpxx(fpxx);

		if (zip_data_len > 1024 * 1024)
		{
			//此情况不管是否失败，zip包及指针肯定都已经清理
			result = send_zip_data_hook(hDev, month, function, arg, need_up_count, zf, fp_zip_name, zip_data_len, 0);
			if (result < 0)
			{
				_WriteLog(LL_INFO, "%s税盘回调同步发送数据超时\n", hDev->szCompanyName);
				return 	DF_TAX_ERR_CODE_HOOK_SERVER_TIME_OUT;
			}
			zf = zipOpen64(fp_zip_name, 0);
			if (zf == NULL)
			{
				_WriteLog(LL_INFO, "zipOpen64 compress file:%s fail!\n", fp_zip_name);
				return 	DF_TAX_ERR_CODE_SYSTEM_MEM_INSUFFICIENT;
			}
			need_up_count = 0;//已上传完成，需上传发票数量清零
			continue;
		}
	}
	result = send_zip_data_hook(hDev, month, function, arg, need_up_count, zf, fp_zip_name, zip_data_len, 1);
	if (result < 0)
	{
		if (file_exists(fp_zip_name) == 0)
		{
			zipClose(zf, NULL);
			delete_file(fp_zip_name);
		}
		_WriteLog(LL_INFO, "%s税盘回调同步发送数据超时\n", hDev->szCompanyName);
		return DF_TAX_ERR_CODE_HOOK_SERVER_TIME_OUT;
	}

	if (file_exists(fp_zip_name) == 0)
	{
		zipClose(zf, NULL);
		delete_file(fp_zip_name);
	}

	out("当前发票正常读取完成,非中断结束,通知M服务当前月读取完成\n");
	if (memcmp(month, hDev->szDeviceTime, 6) == 0)
	{
		memcpy(now_month_sum, &inv_sum, sizeof(struct _inv_sum_data));
	}

	return 0;
}

int mengbai_upload_m_server(HDEV hDev, unsigned short *start_stop, struct _upload_inv_num *upload_inv_num, void *function, void *arg, struct _inv_sum_data *now_month_sum)
{
	int result = 0;
	char today_month[10] = { 0 };
	char invoice_month[100] = { 0 };
	ClearLastError(hDev->hUSB);

	if (upload_inv_num->state == 0)
	{
		_WriteLog(LL_INFO, "[%s]无需上传发票", upload_inv_num->invoice_month);
		return 0;
	}

	//_WriteLog(LL_INFO,"需要按时间段上传发票\n");
	mb_get_today_month(today_month);
	*start_stop = 1;
	strcpy(invoice_month, upload_inv_num->invoice_month);
	if (strlen(invoice_month) == 0 || (strcmp(invoice_month, "201801") < 0))
	{
		memset(invoice_month, 0, sizeof(invoice_month));
		sprintf(invoice_month, "201801");
		if (upload_inv_num->just_single_month == 1)
		{
			return 0;
		}
	}
	for (; strcmp(invoice_month, today_month) <= 0; mb_get_next_month(invoice_month))
	{
		_WriteLog(LL_INFO, "\n读取%s月发票并上传", invoice_month);
		if (*start_stop != 1)
		{
			_WriteLog(LL_INFO, "停止正在查询%s月发票详情动作\n", (char *)invoice_month);
			return -1;
		}
		//_WriteLog(LL_INFO, "正在处理的汇总月份为%s，正在处理的发票月份为%s\n", (char *)summary_month, invoice_month);

		result = mengbai_get_invoice_month_upload_hook(hDev, invoice_month, function, arg, upload_inv_num, start_stop, now_month_sum);
		if (result < 0)
		{
			_WriteLog(LL_INFO, "读取发票数据并回调上传过程中出现异常,停止正在查询%s月发票详情动作\n", (char *)invoice_month);
			return result;
		}

		if (upload_inv_num->just_single_month == 1)
		{
			return 0;
		}
	}
	return 0;
}
#endif

//获取离线票份数
int mengbai_read_offline_inv_num(HUSB hUSB, unsigned char *inv_type, struct _plate_infos *plate_infos)
{
	unsigned char type[4];
	unsigned char type_num;
	int type_tmp[4];
	int type_count;
	int type_mem;
	int in_len;
	int out_len;
	int result;
	int count=0;
	unsigned char r_data[4096];
	unsigned char s_data[4096];
	char errinfo[100] = { 0 };
	//long start_time;
	//long end_time;
	//start_time =get_time_sec();
	for (type_count = 0; type_count < DF_MAX_INV_TYPE; type_count++)
	{
		//long start_time1;
		//long end_time1;
		//start_time1 = get_time_sec();
		if (inv_type[type_count] == 0)
		{
			//out("发票类型不正确\n");
			continue;
		}
		type_num = inv_type[type_count];
		//out("查询发票类型为%d的离线票份数\n", type_num);
		memset(type, 0, sizeof(type));
		sprintf((char *)type, "%03d", type_num);
		//out("获取离线票份数\n");
		memset(s_data, 0, sizeof(s_data));
		memset(r_data, 0, sizeof(r_data));
		in_len = pack_s_buf(DF_PC_USB_ORDER_GET_LXPFS, 0, type, s_data, strlen((const char*)type));
		out_len = sizeof(r_data);
		//char s_time[100] = { 0 };
		//char e_time[100] = { 0 };
		//timer_read_asc(s_time);
		//printf_array(s_data, in_len);
		result = usb_data_write_read(hUSB, s_data, in_len, r_data, &out_len, errinfo);
		if (result < 0)
		{
			out("查询发票类型为%d的离线票份数失败\n", type_num);
			continue;
		}
		//timer_read_asc(e_time);
		//out("模拟盘读取开始时间%s,结束时间%s\n", s_time, e_time);
		memset(type_tmp, 0, sizeof(type_tmp));
		memcpy(type_tmp, r_data , 3);
		if (memcmp(type, type_tmp, 3) != 0)
		{
			out("获取到的发票类型与要查询的发票类型不一致\n");
			continue;
		}
		find_inv_type_mem(type_num, &type_mem);
		plate_infos->invoice_type_infos[type_mem].state = 1;
		plate_infos->invoice_type_infos[type_mem].fplxdm = type_num;
		plate_infos->invoice_type_infos[type_mem].invoice_offline_info.state = 1;
		plate_infos->invoice_type_infos[type_mem].invoice_offline_info.offline_num = r_data[3] * 0x1000000 + r_data[4] * 0x10000 + r_data[5] * 0x100 + r_data[6] * 0x1;
		//end_time1 = get_time_sec();
		//out("获取发票种类%d的离线票数据成功,离线票张数为%d张\n", type_num,plate_infos->invoice_type_infos[type_mem].invoice_offline_info.offline_num);
		//out("结构内存号%d,发票种类%d,结构体状态%d,离线票数量%d\n", type_count, type_num, 1, plate_infos->invoice_type_infos[type_mem].invoice_offline_info.offline_num);
		count += plate_infos->invoice_type_infos[type_mem].invoice_offline_info.offline_num;
	}
	//end_time = get_time_sec();
	//out("获取离线票结束共耗时%d秒\n", end_time - start_time);
	return count;
}

//获取离线票明细
int mengbai_read_offline_inv_details(HUSB hUSB, unsigned char inv_type, int num, char *inv_data, int *inv_len, unsigned char *jshj, char *name, char *number)
{
	unsigned char type[4];
	int type_tmp[4];
	int in_len;
	int out_len;
	int result;
	unsigned char r_data[4096];
	unsigned char s_data[4096];
	unsigned char pack[20];
	char errinfo[100] = { 0 };

	memset(type, 0, sizeof(type));
	memset(pack, 0, sizeof(pack));
	sprintf((char *)type, "%03d", inv_type);
	memcpy(pack, type,3);
	pack[3] = ((num >> 24) & 0xff); pack[4] = ((num >> 16) & 0xff);
	pack[5] = ((num >> 8) & 0xff); pack[6] = ((num >> 0) & 0xff);


	//out("获取离线票详情\n");
	memset(s_data, 0, sizeof(s_data));
	memset(r_data, 0, sizeof(r_data));
	in_len = pack_s_buf(DF_PC_USB_ORDER_GET_SCLXP, 0, pack, s_data, 7);
	out_len = sizeof(r_data);
	//print_array((char *)s_data,in_len);
	result = usb_data_write_read(hUSB, s_data, in_len, r_data, &out_len, errinfo);
	if (result < 0)
	{
		out("查询发票类型为%d的离线票明细失败\n", inv_type);
		return result;
	}
	memset(type_tmp, 0, sizeof(type_tmp));
	memcpy(type_tmp, r_data, 3);
	//if (memcmp(type, type_tmp, 3) != 0)
	//{
	//	out("获取到的发票类型与要查询的发票类型不一致,取到的类型%s，查询的类型%s\n", type_tmp, type);
	//	return DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
	//}

	//out("离线票获取成功,组成上报所需json数据包\n");

	pack_offline_report_json_data(r_data, (unsigned char*)inv_data, jshj,name,number);
	str_replace(inv_data, "\\\\", "\\");
	*inv_len = strlen(inv_data);
	return out_len;
}
//开票开具
int mengbai_make_invoice_to_plate(HDEV hDev, HUSB hUSB, unsigned char inv_type, char *inv_info, struct _plate_infos *plate_infos, struct _make_invoice_result *make_invoice_result)
{
	unsigned char type[4];
	int type_tmp[4];
	int in_len;
	int out_len;
	int result;
	unsigned char inv_data[4096];
	unsigned char r_data[4096];
	unsigned char s_data[4096];
	int s_len = 0;
	memset(type, 0, sizeof(type));
	sprintf((char *)type, "%03d", inv_type);
	//out("发票开具\n");
	
	memcpy(inv_data, "11111111", 8);
	memcpy(inv_data+8, type, 3);
	
	
	memset(s_data, 0, sizeof(s_data));
	memset(r_data, 0, sizeof(r_data));
	//out("将JSON数据解析成结构体，再拼接成开票数据\n");
	int i_FplxIndex;
	find_inv_type_mem((unsigned char)inv_type, &i_FplxIndex);
	//out("发票类型代码%03d\n", inv_type);

	struct Fpxx *fpxx = NULL;
	fpxx = malloc(sizeof(struct Fpxx));
	if (fpxx == NULL)
	{
		printf("stp_fpxx malloc Err\n");
		sprintf((char *)make_invoice_result->errinfo, "系统内存申请失败,机柜可能存在故障,请重启机柜");
		return DF_TAX_ERR_CODE_SYSTEM_MEM_INSUFFICIENT;
	}
	memset(fpxx, 0, sizeof(struct Fpxx));

	fpxx->stp_MxxxHead = malloc(sizeof(struct Spxx));
	if (fpxx->stp_MxxxHead == NULL)
	{
		printf("stp_fpxx->stp_MxxxHead malloc Err\n");
		sprintf((char *)make_invoice_result->errinfo, "系统内存申请失败,机柜可能存在故障,请重启机柜");
		free(fpxx);
		return DF_TAX_ERR_CODE_SYSTEM_MEM_INSUFFICIENT;
	}
	memset(fpxx->stp_MxxxHead, 0, sizeof(struct Spxx));

	//out("联机命令\n");
	char time_now[20];
	memset(time_now, 0, sizeof(time_now));

	timer_read_y_m_d_h_m_s(time_now);
	//out("设置当前时间%s\n", time_now);

	strcpy(hDev->szDeviceTime, time_now);

	fpxx->hDev = hDev;
	strcpy(fpxx->tax_nature, (char *)plate_infos->plate_tax_info.tax_nature);
	result = analysis_json_pack_bin_data(hDev, fpxx, (uint8 *)inv_info, s_data, plate_infos, make_invoice_result);
	if (result < 0)
	{
		out("json数据解析失败，无法开票\n");
		FreeFpxx(fpxx);
		return result;
	}

	if (make_invoice_result->test_only == 1)
	{
		logout(INFO, "TAXLIB", "发票开具", "测试接口不执行最终开具动作\r\n");
		sprintf((char *)make_invoice_result->errinfo, "开票数据及税盘环境校验均成功,测试接口不执行最终开具");
		result = DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
		FreeFpxx(fpxx);
		return result;
	}

	_WriteLog(LL_INFO, "发票数据校验通过开始底层开具\n");
	memcpy(inv_data + 11, s_data, result);
	s_len = 11 + result;
	memset(s_data, 0, sizeof(s_data));
	in_len = pack_s_buf(DF_PC_USB_ORDER_FPKJ, 0, inv_data, s_data, s_len);
	out_len = sizeof(r_data);
	//printf_array((char *)s_data, in_len);
	result = usb_data_write_read(hUSB, s_data, in_len, r_data, &out_len, (char *)make_invoice_result->errinfo);
	if (result < 0)
	{
		out("发票类型为%d发票开具失败\n", inv_type);
		FreeFpxx(fpxx);
		return result;
	}
	
	//out("模拟盘发票底层开具成功\n");
	memset(type_tmp, 0, sizeof(type_tmp));
	memcpy(type_tmp, r_data, 3);
	if (memcmp(type, type_tmp, 3) != 0)
	{
		out("获取到的发票类型与要查询的发票类型不一致\n");
		out("开具的发票不正确\n");
		FreeFpxx(fpxx);
		return DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
	}
	make_invoice_result->fplxdm = inv_type;

	memcpy(make_invoice_result->fpdm, r_data + 3,20);
	memcpy(make_invoice_result->fphm, r_data + 3 + 20, 8);
	memcpy(make_invoice_result->kpsj, r_data + 3 + 20 + 8, 19);

	sprintf((char *)make_invoice_result->jym, "01234567890123456789");
	sprintf((char *)make_invoice_result->mwq, "<*/*0000000000111111111122222222223333333333444444444455555555556666666666777777777788888888889999999999*/*>");

	//memcpy(make_invoice_result->jym,  r_data + 3 + 20 + 8 + 19, 20);
	//memcpy(make_invoice_result->mwq,  r_data + 3 + 20 + 8 + 19 + 20, 100);
	_WriteLog(LL_INFO, "底层开具成功\n");

	struct StaticDeviceInfo pDevInfo;
	pDevInfo.bDeviceType = 3;
	sprintf(pDevInfo.szDeviceID, "%s",plate_infos->plate_basic_info.plate_num);
	fpxx->hDev = &pDevInfo;
	memcpy(fpxx->fpdm, r_data + 3, 20);
	memcpy(fpxx->fphm, r_data + 3 + 20, 8);
	memcpy(fpxx->kpsj_F2, r_data + 3 + 20 + 8, 19);
	sprintf(fpxx->bmbbbh, "33.0");
	sprintf(fpxx->hsjbz, "0");
	sprintf(fpxx->qdbj, "N");
	sprintf(fpxx->xfmc, "%s", plate_infos->plate_basic_info.ca_name);
	sprintf(fpxx->xfsh, "%s", plate_infos->plate_basic_info.ca_number);
	sprintf(fpxx->jym, "01234567890123456789");
	sprintf(fpxx->mw, "<*/*0000000000111111111122222222223333333333444444444455555555556666666666777777777788888888889999999999*/*>");

	//if (fpxx->fplx == FPLX_COMMON_DZFP)
	//{		
	//	char url[5000] = { 0 };
	//	int year, month, date, hour, min, sec;
	//	char time_str[20] = { 0 };
	//	sscanf(fpxx->kpsj_F2, "%04d-%02d-%02d %02d:%02d:%02d", &year, &month, &date, &hour, &min, &sec);
	//	sprintf(time_str, "%04d%02d%02d", year, month, date);
	//	sprintf(url, "http://103.45.249.88:20020/InvoiceFile/%s/%s_%s_%s.ofd", h_Dev->szDeviceID, fpxx->fpdm, fpxx->fphm, time_str);		
	//	out("OFD:%s\n", url);
	//	strcpy((char *)make_invoice_result->ofdurl, url);
	//}

	fpxx_to_json_base_fpsjbbh_v102(fpxx, &make_invoice_result->scfpsj, DF_FPQD_Y, DF_FP_JSON);
	FreeFpxx(fpxx);
	return out_len;
}

//发票作废
int mengbai_cancel_invoice_to_plate(HDEV hDev, unsigned char inv_type, char *fpdm, char *fphm, struct _cancel_invoice_result *cancel_invoice_result)
{
	unsigned char type[4];
	int type_tmp[4];
	int in_len;
	int out_len;
	int result;
	int fphm_i;
	char fphm_s[4];
	unsigned char inv_data[4096];
	unsigned char r_data[4096];
	unsigned char s_data[4096];
	//char errinfo[100] = { 0 };

	memset(type, 0, sizeof(type));
	sprintf((char *)type, "%03d", inv_type);
	out("发票作废\n");
	memcpy(inv_data, "11111111", 8);
	memcpy(inv_data + 8, type, 3);
	memcpy(inv_data + 11, fpdm, 20);
	fphm_i = atoi((const char *)fphm);
	out("发票号码：%d\n", fphm_i);
	fphm_s[0] = ((fphm_i >> 24) & 0xff);			fphm_s[1] = ((fphm_i >> 16) & 0xff);
	fphm_s[2] = ((fphm_i >> 8) & 0xff);				fphm_s[3] = ((fphm_i >> 0) & 0xff);


	memcpy(inv_data + 31, fphm_s, 4);	

	memset(s_data, 0, sizeof(s_data));
	memset(r_data, 0, sizeof(r_data));
	in_len = pack_s_buf(DF_PC_USB_ORDER_FPZF, 0, inv_data, s_data, 45);
	//	print_array(s_data,45);
	out_len = sizeof(r_data);
	result = usb_data_write_read(hDev->hUSB, s_data, in_len, r_data, &out_len, (char *)cancel_invoice_result->errinfo);
	if (result < 0)
	{
		out("发票类型为%d发票作废失败\n", inv_type);
		return result;
	}

	out("发票作废成功\n");
	memset(type_tmp, 0, sizeof(type_tmp));
	memcpy(type_tmp, r_data, 3);
	if (memcmp(type, type_tmp, 3) != 0)
	{
		out("获取到的发票类型与要查询的发票类型不一致\n");
		out("开具的发票不正确\n");
		return DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
	}
	cancel_invoice_result->fplxdm = inv_type;

	memcpy(cancel_invoice_result->fpdm, fpdm, strlen((char *)fpdm));
	memcpy(cancel_invoice_result->fphm, fphm, strlen((char *)fphm));
	memcpy(cancel_invoice_result->zfsj, r_data + 3, 19);
	memcpy(cancel_invoice_result->mwq, r_data + 3 + 19, 100);
	return out_len;

}

//空白发票作废
int mengbai_cancel_null_invoice_to_plate(HUSB hUSB, struct _cancel_invoice_result *cancel_invoice_result)
{
	unsigned char type[4];
	int type_tmp[4];
	int in_len;
	int out_len;
	int result;
	unsigned char inv_data[4096] = {0};
	unsigned char r_data[4096] = { 0 };
	unsigned char s_data[4096] = { 0 };
	char errinfo[100] = { 0 };
	int i;

	if ((cancel_invoice_result->fplxdm != FPLX_COMMON_PTFP) && (cancel_invoice_result->fplxdm != FPLX_COMMON_ZYFP))
	{
		sprintf((char *)cancel_invoice_result->errinfo, "发票作废仅支持普通发票和专用发票");
		return DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
	}
	if ((strlen((char *)cancel_invoice_result->zfr) == 0) || (strlen((char *)cancel_invoice_result->zfr) > 16))
	{
		sprintf((char *)cancel_invoice_result->errinfo, "作废人长度有误");
		return DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
	}
	result = IsGBK((char *)cancel_invoice_result->zfr);
	if (result != 1)
	{
		sprintf((char *)cancel_invoice_result->errinfo, "作废人编码格式有误");
		return DF_TAX_ERR_CODE_USE_ILL_CHAR_OR_UTF8;//编码格式错误
	}
	if (cancel_invoice_result->zfzs < 1)
	{
		sprintf((char *)cancel_invoice_result->errinfo, "作废总数有误");
		return DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
	}



	for (i = 0; i < cancel_invoice_result->zfzs; i++)
	{

		memset(type, 0, sizeof(type));
		sprintf((char *)type, "%03d", cancel_invoice_result->fplxdm);
		out("空白发票作废\n");
		memcpy(inv_data, "11111111", 8);
		memcpy(inv_data + 8, type, 3);

		memset(s_data, 0, sizeof(s_data));
		memset(r_data, 0, sizeof(r_data));
		in_len = pack_s_buf(DF_PC_USB_ORDER_KBZF, 0, inv_data, s_data, 11);
		out_len = sizeof(r_data);
		result = usb_data_write_read(hUSB, s_data, in_len, r_data, &out_len, errinfo);
		if (result < 0)
		{
			out("查询发票类型为%d的空白作废失败\n", cancel_invoice_result->fplxdm);
			return result;
		}

		out("空白发票作废成功\n");
		memset(type_tmp, 0, sizeof(type_tmp));
		memcpy(type_tmp, r_data, 3);
		if (memcmp(type, type_tmp, 3) != 0)
		{
			out("获取到的发票类型与要查询的发票类型不一致\n");
			out("开具的发票不正确\n");
			return DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
		}
	}

	return out_len;

}


//解析json头信息
static int analysis_json_head(char *inbuf, char *s_cmd)
{
	int ret;
	char protocol[50] = { 0 };
	char code_type[50] = { 0 };
	char cmd[50] = { 0 };
	char result[20] = { 0 };
	cJSON *item;
	cJSON *root = cJSON_Parse(inbuf);
	if (!root)
	{
		out("Error before: [%s]\n", cJSON_GetErrorPtr());
		return -1;
	}
	cJSON *object = cJSON_GetObjectItem(root, "head");
	if (object == NULL)
	{
		out("Error before: [%s]\n", cJSON_GetErrorPtr());
		cJSON_Delete(root);
		return -1;
	}
	//out("获取协议版本\n");
	item = cJSON_GetObjectItem(object, "protocol");
	if (item == NULL)
	{
		out("Error before: [%s]\n", cJSON_GetErrorPtr());
		cJSON_Delete(root);
		return -1;
	}
	else
	{
		//out("cJSON_GetObjectItem: type=%d, string is %s, valuestring=%s\n", item->type, item->string, item->valuestring);
		memcpy(protocol, item->valuestring, strlen(item->valuestring));
	}
	ret = strcmp(protocol, "MB_API20190627");
	if (ret != 0)
	{
		//out("协议版本不一致,protocol = %s\n", protocol);
		cJSON_Delete(root);
		return -1;
	}
	///////////////////////////////////////////////////////////////////
	item = cJSON_GetObjectItem(object, "code_type");
	if (item == NULL)
	{
		out("Error before: [%s]\n", cJSON_GetErrorPtr());
		cJSON_Delete(root);
		return -1;
	}
	else
	{
		//out("cJSON_GetObjectItem: type=%d, string is %s, valuestring=%s\n", item->type, item->string, item->valuestring);
		memcpy(code_type, item->valuestring, strlen(item->valuestring));
	}
	if (strcmp(code_type, "response") != 0)
	{
		out("消息类型错误,parm->code_type = %s\n", code_type);
		cJSON_Delete(root);
		return -1;
	}

	item = cJSON_GetObjectItem(object, "cmd");
	if (item == NULL)
	{
		out("Error before: [%s]\n", cJSON_GetErrorPtr());
		cJSON_Delete(root);
		return -1;
	}
	else
	{
		//out("cJSON_GetObjectItem: type=%d, string is %s, valuestring=%s\n", item->type, item->string, item->valuestring);
		memcpy(cmd, item->valuestring, strlen(item->valuestring));
	}
	//out("获取到命令字为%s\n", cmd);
	if (strcmp(cmd, s_cmd) != 0)
	{
		out("命令字错误,cmd = %s\n", cmd);
		cJSON_Delete(root);
		return -1;
	}

	//out(获取操作结果\n);
	item = cJSON_GetObjectItem(object, "result");
	if (item == NULL)
	{
		out("Error before: [%s]\n", cJSON_GetErrorPtr());
		cJSON_Delete(root);
		return -1;
	}
	else
	{
		//out("cJSON_GetObjectItem: type=%d, string is %s, valuestring=%s\n", item->type, item->string, item->valuestring);
		memcpy(result, item->valuestring, strlen(item->valuestring));
		//out("获取到操作结果为%s\n", result);
	}
	ret = strcmp(result, "s");
	if (ret != 0)
	{
		//out("获取结果代码不为成功,result = %s\n", result);
		cJSON_Delete(root);
		return -1;
	}
	cJSON_Delete(root);
	return 0;
}
//抄报
int mengbai_get_chao_bao_data(HDEV hDev, unsigned char inv_type, char *errinfo)
{
	unsigned char type[4];
	int in_len;
	int out_len;
	int result;
	unsigned char inv_data[4096] = { 0 };
	unsigned char r_data[4096] = { 0 };
	unsigned char s_data[4096] = {0};
	uint8 cb_data[4096] = { 0 };
	memset(type, 0, sizeof(type));
	sprintf((char *)type, "%03d", inv_type);
	//out("抄报数据获取\n");
	memcpy(inv_data, "11111111", 8);
	memcpy(inv_data + 8, type, 3);

	memset(s_data, 0, sizeof(s_data));
	memset(r_data, 0, sizeof(r_data));
	in_len = pack_s_buf(DF_PC_USB_ORDER_CB, 0, inv_data, s_data, 11);
	out_len = sizeof(r_data);
	result = usb_data_write_read(hDev->hUSB, s_data, in_len, r_data, &out_len, errinfo);
	if (result < 0)
	{
		out("查询发票类型为%d的抄报失败\n", inv_type);
		return result;
	}
	//out("获取到的抄报数据为：\n");
	//print_array((char *)r_data, out_len);
	//out("抄报数据组包\n");
	pack_chao_bao_json_data(r_data, cb_data, (uint8 *)hDev->szCommonTaxID);

	char *returndata;
	int returndata_len;


	result = tcp_http_netPost("www.njmbxx.com", 10080, "/WAS.UploadBillingData/declareDutiableGoods", "application/x-www-form-urlencoded", (char *)cb_data, 5, &returndata, &returndata_len);
	if (result < 0)
	{
		if (returndata != NULL)
		{
			free(returndata);
			out("释放http接收数据内存\n");
		}
		out("抄报数据上传失败\n");
		return -1;
	}

	result = analysis_json_head(returndata, "/WAS.UploadBillingData/declareDutiableGoods");
	free(returndata);
	if (result < 0)
	{
		out("解析抄报应答出错,抄报未成功\n");
		return -2;
	}
	out("发票类型：%03d,抄税及上传汇总成功\n", inv_type);



	//out("组包好的抄报数据为：%s\n", cb_data);
	return 0;
}
//监控信息回传
int mengbai_set_monitor_data(HUSB hUSB, unsigned char inv_type, char *errinfo)
{
	unsigned char type[4];
	int in_len;
	int out_len;
	int result;
	int lxsc;
	unsigned char inv_data[4096] = {0};
	unsigned char r_data[4096] = { 0 };
	unsigned char s_data[4096] = { 0 };
	unsigned char tmp_time[20] = { 0 };
	char tmp_t[20] = {0};
	char time_date[20] = {0};
	float tmp_float;
	long tmp_int;
	char tmp_buf[4] = { 0 };

	struct _plate_infos plate_infos;
	struct _monitor_info monitor_info;
	int mem_i;
	memset(&plate_infos, 0, sizeof(struct _plate_infos));
	memset(&monitor_info, 0, sizeof(struct _monitor_info));

	mengbai_read_inv_type_areacode(hUSB, &plate_infos);
	mengbai_read_monitor_info(hUSB, plate_infos.plate_tax_info.inv_type, &plate_infos);
	find_inv_type_mem(inv_type, &mem_i);
	memcpy(&monitor_info, &plate_infos.invoice_type_infos[mem_i].monitor_info, sizeof(struct _monitor_info));


	memset(tmp_time, 0, sizeof(tmp_time));
	memset(tmp_t, 0, sizeof(tmp_t));
	memcpy(tmp_time, monitor_info.kpjzsj, 8);

	//out("开票截止日期改为下月,修改前%s\n", tmp_time);
	next_month_asc((char *)tmp_time);
	//out("开票截止日期更改后：%s\n", tmp_time);
	memset(monitor_info.kpjzsj, 0, sizeof(monitor_info.kpjzsj));
	memcpy(monitor_info.kpjzsj, tmp_time, 8);


	memcpy(tmp_time, monitor_info.bszzrq, 8);
	//out("报送终止日期改为下月最后一天, 修改前%s\n", tmp_time);
	next_month_last_date_asc((char *)tmp_time);
	//out("报送终止日期更改后：%s\n", tmp_time);
	memset(monitor_info.bszzrq, 0, sizeof(monitor_info.bszzrq));
	memcpy(monitor_info.bszzrq, tmp_time, 8);


	memcpy(tmp_time, monitor_info.bsqsrq, 8);
	//out("报送起始日期改为下月第一天, 修改前%s\n", tmp_time);
	next_month_frist_date_asc((char *)tmp_time);
	//out("报送起始日期更改后：%s\n", tmp_time);
	memset(monitor_info.bsqsrq, 0, sizeof(monitor_info.bsqsrq));
	memcpy(monitor_info.bsqsrq, tmp_time, 8);

	memset(time_date, 0, sizeof(time_date));
	timer_read_y_m_d(time_date);
	memset(monitor_info.bsqsrq, 0, sizeof(monitor_info.bsqsrq));
	memcpy(monitor_info.bsqsrq, time_date, 8);






	memset(type, 0, sizeof(type));
	sprintf((char *)type, "%03d", inv_type);
	//out("监控信息回传\n");
	memcpy(inv_data, "11111111", 8);
	memcpy(inv_data + 8, type, 3);

	memset(tmp_time, 0, sizeof(tmp_time));
	memcpy(tmp_time, monitor_info.kpjzsj, 4);
	sprintf((char *)tmp_time + strlen((const char *)tmp_time), "-");
	memcpy(tmp_time + strlen((const char *)tmp_time), monitor_info.kpjzsj + 4, 2);
	sprintf((char *)tmp_time + strlen((const char *)tmp_time), "-");
	memcpy(tmp_time + strlen((const char *)tmp_time), monitor_info.kpjzsj + 6, 2);
	memcpy(inv_data + 11, tmp_time, 10);

	tmp_float = atof((const char *)monitor_info.dzkpxe) * 100;
	tmp_int = tmp_float;
	memset(tmp_buf, 0, sizeof(tmp_buf));
	tmp_buf[0] = ((tmp_int >> 24) & 0xff); tmp_buf[1] = ((tmp_int >> 16) & 0xff);
	tmp_buf[2] = ((tmp_int >> 8) & 0xff); tmp_buf[3] = ((tmp_int >> 0) & 0xff);
	memcpy(inv_data + 21, tmp_buf, 4);
	

	memset(tmp_buf, 0, sizeof(tmp_buf));
	memcpy(tmp_buf, monitor_info.bszzrq+6, 2);
	inv_data[25] = atoi(tmp_buf);

	lxsc = 72;
	inv_data[26] = ((lxsc >> 24) & 0xff); inv_data[27] = ((lxsc >> 16) & 0xff);
	inv_data[28] = ((lxsc >> 8) & 0xff); inv_data[29] = ((lxsc >> 0) & 0xff);

	tmp_float = atof((const char *)monitor_info.lxzsljje) * 100;
	tmp_int = tmp_float;
	memset(tmp_buf, 0, sizeof(tmp_buf));
	tmp_buf[0] = ((tmp_int >> 24) & 0xff); tmp_buf[1] = ((tmp_int >> 16) & 0xff);
	tmp_buf[2] = ((tmp_int >> 8) & 0xff); tmp_buf[3] = ((tmp_int >> 0) & 0xff);
	memcpy(inv_data + 30, tmp_buf, 4);


	memset(s_data, 0, sizeof(s_data));
	memset(r_data, 0, sizeof(r_data));
	in_len = pack_s_buf(DF_PC_USB_ORDER_JKXXHC, 0, inv_data, s_data, 34);
	out_len = sizeof(r_data);
	//printf_array(s_data, in_len);
	result = usb_data_write_read(hUSB, s_data, in_len, r_data, &out_len, errinfo);
	if (result < 0)
	{
		out("查询发票类型为%d的监控信息回传失败\n", inv_type);
		return result;
	}
	//out("获取到监控信息回写应答数据为：\n");
	//print_array((char *)r_data, out_len);
	return 0;
}

//离线票上传成功
int mengbai_set_invoice_to_plate(HUSB hUSB, unsigned char inv_type)
{
	unsigned char type[4];
	int in_len;
	int out_len;
	int result;
	unsigned char inv_data[4096];
	unsigned char r_data[4096];
	unsigned char s_data[4096];
	char errinfo[100] = { 0 };
	memset(type, 0, sizeof(type));
	sprintf((char *)type, "%03d", inv_type);
	//out("离线发票上传\n");
	memcpy(inv_data, "11111111", 8);
	memcpy(inv_data + 8, type, 3);

	memset(s_data, 0, sizeof(s_data));
	memset(r_data, 0, sizeof(r_data));
	in_len = pack_s_buf(DF_PC_USB_ORDER_LXPSCCG, 0, inv_data, s_data, 11);
	out_len = sizeof(r_data);
	result = usb_data_write_read(hUSB, s_data, in_len, r_data, &out_len, errinfo);
	if (result < 0)
	{
		out("查询发票类型为%d的离线发票上传失败\n", inv_type);
		return result;
	}

	//out("离线发票上传成功\n");
	
	return out_len;

}

//获取发票签名
int mengbai_read_invoice_sign(HUSB hUSB, unsigned char inv_type, unsigned char *inv_info, int inv_len, unsigned char *sign)
{
	unsigned char type[4];
	int in_len;
	int out_len;
	int result;
	unsigned char inv_data[4096];
	unsigned char r_data[4096];
	unsigned char s_data[4096];
	char errinfo[100] = { 0 };
	memset(type, 0, sizeof(type));
	sprintf((char *)type, "%03d", inv_type);
	out("获取发票签名\n");
	memcpy(inv_data, "11111111", 8);
	memcpy(inv_data + 8, type, 3);
	memcpy(inv_data + 11, inv_data, inv_len);
	inv_len = inv_len + 11;
	memset(s_data, 0, sizeof(s_data));
	memset(r_data, 0, sizeof(r_data));
	in_len = pack_s_buf(DF_PC_USB_ORDER_GET_FPQM, 0, inv_data, s_data, inv_len);
	out_len = sizeof(r_data);
	result = usb_data_write_read(hUSB, s_data, in_len, r_data, &out_len, errinfo);
	if (result < 0)
	{
		out("获取发票签名\n");
		return result;
	}
	out("获取发票签名成功\n");
	memcpy(sign, r_data, out_len);
	return out_len;
}

//验证证书口令
int mengbai_read_cert_passwd(HUSB hUSB, unsigned char *c_passwd)
{
	int in_len;
	int out_len;
	int result;
	unsigned char r_data[4096];
	unsigned char s_data[4096];
	char errinfo[100] = { 0 };
	//out("验证证书口令\n");
	memset(s_data, 0, sizeof(s_data));
	memset(r_data, 0, sizeof(r_data));
	in_len = pack_s_buf(DF_PC_USB_ORDER_YZZSKL, 0, c_passwd, s_data, 8);
	out_len = sizeof(r_data);
	result = usb_data_write_read(hUSB, s_data, in_len, r_data, &out_len, errinfo);
	if (result < 0)
	{
		out("验证证书口令失败\n");
		return result;
	}
	//out("验证证书口令成功\n");
	return out_len;
}

/*组包发票字符串*/
static int pack_bin_data(struct Fpxx* stp_fpxx, unsigned char* pc_Bin, struct _make_invoice_result *make_invoice_result)
{
	float sqsl;
	char sqslv[10], spmc[200] = {0};
	if (!stp_fpxx || !pc_Bin)
		return DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
	struct Spxx * stp_spxx = NULL;
	int i_index = 0;
	memcpy(pc_Bin, stp_fpxx->kplx, 1); i_index += 1;
	memcpy(pc_Bin + i_index, stp_fpxx->gfsh, 20); i_index += 20;
	memcpy(pc_Bin + i_index, stp_fpxx->gfmc, 80); i_index += 80;
	memcpy(pc_Bin + i_index, stp_fpxx->je, 20); i_index += 20;
	memcpy(pc_Bin + i_index, stp_fpxx->se, 20); i_index += 20;
	memcpy(pc_Bin + i_index, stp_fpxx->jshj, 20); i_index += 20;
	memcpy(pc_Bin + i_index, stp_fpxx->bz, 20); i_index += 20;
	memcpy(pc_Bin + i_index, stp_fpxx->skr, 10); i_index += 10;
	memcpy(pc_Bin + i_index, stp_fpxx->kpr, 10); i_index += 10;
	memcpy(pc_Bin + i_index, stp_fpxx->blueFpdm, 20); i_index += 20;
	memcpy(pc_Bin + i_index, stp_fpxx->blueFphm, 8); i_index += 8;

	memcpy(make_invoice_result->hjje, stp_fpxx->je,20);
	memcpy(make_invoice_result->hjse, stp_fpxx->se, 20);
	memcpy(make_invoice_result->jshj, stp_fpxx->jshj, 20);
	memcpy(make_invoice_result->fpqqlsh, stp_fpxx->fpqqlsh, sizeof(stp_fpxx->fpqqlsh));
	UTF8ToGBKBase64(stp_fpxx->bz, strlen(stp_fpxx->bz), (char *)make_invoice_result->bz);
	//out("开票结果备注信息内容：%s\n", make_invoice_result->bz);
	char spsl_tmp[10] = {0};
	sprintf(spsl_tmp, "%d", stp_fpxx->spsl);
	memcpy(pc_Bin + i_index, spsl_tmp, 2); i_index += 2;
	//第一个商品明细
	stp_spxx = stp_fpxx->stp_MxxxHead->stp_next;

	while (stp_spxx){
		memset(spmc, 0, sizeof(spmc)); sprintf(spmc, "%s%s", stp_spxx->spsmmc, stp_spxx->spmc);
		memcpy(pc_Bin + i_index, spmc, 40); i_index += 40;
		memcpy(pc_Bin + i_index, stp_spxx->dj, 20); i_index += 20;
		memcpy(pc_Bin + i_index, stp_spxx->sl, 20); i_index += 20;
		memcpy(pc_Bin + i_index, stp_spxx->je, 20); i_index += 20;
		memcpy(pc_Bin + i_index, stp_spxx->jldw, 22); i_index += 22;
		memcpy(pc_Bin + i_index, stp_spxx->ggxh, 40); i_index += 40;
		memcpy(pc_Bin + i_index, stp_spxx->se, 20); i_index += 20;

		switch (atoi(stp_spxx->fphxz)) {
		case FPHXZ_AISINO_SPXX:
			sprintf(stp_spxx->fphxz, "0");
			break;
		case FPHXZ_AISINO_ZKXX:
			sprintf(stp_spxx->fphxz, "1");
			break;
		case FPHXZ_AISINO_SPXX_ZK:
			sprintf(stp_spxx->fphxz, "2");
			break;
		case FPHXZ_AISINO_XJXHQD:
			sprintf(stp_spxx->fphxz, "3");
			break;
		default:
			sprintf(stp_spxx->fphxz, "0");
			break;
		}


		//out("商品税率为%s\n", stp_spxx->slv);
		sqsl = atof(stp_spxx->slv)*100;
		memset(sqslv, 0, sizeof(sqslv));
		sprintf(sqslv, "%d", (int )sqsl);
		//out("slv为%s\n",sqslv);
		memcpy(pc_Bin + i_index, sqslv, 10); i_index += 10;
		memcpy(pc_Bin + i_index, stp_spxx->spbh, 40); i_index += 40;

		stp_spxx = stp_spxx->stp_next;
	}
	return i_index;
}


static int analysis_json_pack_bin_data(HDEV hDev, struct Fpxx *stp_fpxx, unsigned char *json_data, unsigned char *bin_data, struct _plate_infos *plate_infos, struct _make_invoice_result *make_invoice_result)
{
	int result = 0;
	
	//out("json_data = %s\n", json_data);
	//out("json_data = %s", json_data);
	//str_replace((char *)json_data, "\\", "[@*br/*@]");
	//result = check_invoice_str((unsigned char *)json_data);
	//if (result < 0)
	//{
	//	sprintf((char *)make_invoice_result->errinfo, "发票JSON数据中存在特殊字符无法解析,例如&和\\");
	//	out("check_invoice_str error result = %d",result);
	//	return DF_TAX_ERR_CODE_USE_ILL_CHAR_OR_UTF8;
	//}
	//out("json_data = %s",json_data);
	//result = judge_code((const char*)json_data, strlen((const char *)json_data));
	//result = IsGBK((const char*)json_data);
	//if (result != 1)
	//{
	//	sprintf((char *)make_invoice_result->errinfo, "发票JSON数据汉字编码格式错误,非GBK编码");
	//	out("judge_code error result = %d", result);
	//	return DF_TAX_ERR_CODE_USE_ILL_CHAR_OR_UTF8;//编码格式错误
	//}
	
	stp_fpxx->isHzxxb = false;
	if ((result = AnalyzeJsonBuff(1,(char *)json_data, stp_fpxx, (char *)make_invoice_result->errinfo)) < 0)
	{
		//FreeFpxx(stp_fpxx);
		out("JSON解析失败,result = %d\n", result);
		return result;
	}
	//out("JSON数据解析成功\n");
	//强制税盘开票人税号和开票人名称更新开票信息
	strcpy(stp_fpxx->xfsh, hDev->szCommonTaxID);
	strcpy(stp_fpxx->xfmc, hDev->szCompanyName);

	if ((strlen(stp_fpxx->fpdm) > 0) || (strlen(stp_fpxx->fphm) > 0))
	{
		unsigned char type[4];	
		unsigned char data[4];		
		int result;
		int in_len, out_len;
		unsigned char s_data[4096];
		unsigned char r_data[4096];
		char errinfo[500] = { 0 };	
		char dqfpdm[20] = {0};
		int find_fpdm_fphm = 0;
		unsigned int dqfphm;
			//out("查询发票类型为%d的发票卷数据\n", type_num);
		memset(type, 0, sizeof(type));
		sprintf((char *)type, "%03d", stp_fpxx->fplx);
		//out("获取发票卷信息\n");
		memset(s_data, 0, sizeof(s_data));
		memset(r_data, 0, sizeof(r_data));
		in_len = pack_s_buf(DF_PC_USB_ORDER_GET_WSYFPJ, 0, type, s_data, strlen((const char*)type));
		out_len = sizeof(r_data);
		result = usb_data_write_read(hDev->hUSB, s_data, in_len, r_data, &out_len, errinfo);
		if (result < 0)
		{
			sprintf((char *)make_invoice_result->errinfo, "获取当前发票代码号码失败");
			return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
		}
		memcpy(dqfpdm, r_data + 3, 20);
		//out("结构内存号%d,发票种类%d,当前发票代码%s\n", type_count, type_num, plate_infos->invoice_type_infos[type_mem].invoice_coils_info.dqfpdm);
		memset(data, 0, sizeof(data));
		memcpy(data, r_data + 23, 4);
		dqfphm = data[0] * 0x1000000 + data[1] * 0x10000 + data[2] * 0x100 + data[3] * 0x1;
		if ((strcmp(stp_fpxx->fpdm, (char *)dqfpdm) == 0))
		{
			unsigned int dqfphm_tmp = atoi(stp_fpxx->fphm);
			if (dqfphm == dqfphm_tmp)
			{
				find_fpdm_fphm = 1;
			}
		}
		if (find_fpdm_fphm != 1)
		{
			sprintf((char *)make_invoice_result->errinfo, "传入的当前发票代码号码有误,模拟盘不支持选择发票卷开票");
			return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
		}
	}

	//检查发票参数
	if ((result = check_fpxx_mengbai(stp_fpxx, plate_infos, (char *)make_invoice_result->errinfo)) != 0){
		
		//FreeFpxx(stp_fpxx);
		out("发票参数检查失败,result = %d\n", result);
		return result;
	}
	
	if ((result = pack_bin_data(stp_fpxx, bin_data,make_invoice_result)) < 0)
	{
		//FreeFpxx(stp_fpxx);
		out("发票数据组包失败,result = %d\n",result);
		return result;
	}
	//out("发票数据组包成功,result = %d\n",result);
	//printf_array((char *)bin_data, result);
	
	

	//FreeFpxx(stp_fpxx);
	return result;
}

static int check_fpxx_mengbai(struct Fpxx* stp_fpxx, struct _plate_infos *plate_infos,char *errinfo)
{
	int i_spsmmclen = 0;

	//char* pc_Tmp = NULL;
	int inv_type;
	if (!stp_fpxx || !plate_infos)
		return DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
	struct Spxx *stp_spxx = stp_fpxx->stp_MxxxHead->stp_next;
	struct Spxx *stp_spxx_tmp = stp_fpxx->stp_MxxxHead->stp_next;
	int i_FplxIndex = 0;
	//发票类型

	inv_type = stp_fpxx->fplx;

	find_inv_type_mem((unsigned char)inv_type, &i_FplxIndex);
	if (i_FplxIndex < 0 || i_FplxIndex> 7)
	{
		sprintf(errinfo, "发票类型错误");
		return DF_TAX_ERR_CODE_UNSUPPORT_INV_TYPE;
	}
	//销方税号
	if ((strlen((const char *)plate_infos->plate_basic_info.ca_number) == 0) || (strcmp((const char *)stp_fpxx->xfsh, (const char *)plate_infos->plate_basic_info.ca_number) != 0) || (strlen(stp_fpxx->xfsh) < 15) || (strlen(stp_fpxx->xfsh) > 20)){
		sprintf(errinfo, "销方税号信息与税盘不匹配");
		logout(INFO, "TAXLIB", "发票开具", "系统税号%s,发票数据名称%s\r\n", plate_infos->plate_basic_info.ca_number, stp_fpxx->xfsh);
		return DF_TAX_ERR_CODE_INV_DATA_DISACCORD;
	}
	//销方名称
	if ((strlen((const char *)plate_infos->plate_basic_info.ca_name) == 0) || (strcmp((const char *)stp_fpxx->xfmc, (const char *)plate_infos->plate_basic_info.ca_name)) != 0){
		sprintf(errinfo, "销方名称信息与税盘不匹配");
		logout(INFO, "TAXLIB", "发票开具", "系统名称%s,发票数据名称%s\r\n", plate_infos->plate_basic_info.ca_name, stp_fpxx->xfmc);
		return DF_TAX_ERR_CODE_INV_DATA_DISACCORD;
	}
	//盘号
	if ((strlen((const char *)plate_infos->plate_basic_info.plate_num) == 0) || (strcmp((const char *)stp_fpxx->jqbh, (const char *)plate_infos->plate_basic_info.plate_num) != 0)){
		sprintf(errinfo, "销方盘号信息与税盘不匹配,系统盘号%s,发票数据盘号%s", plate_infos->plate_basic_info.plate_num, stp_fpxx->jqbh);
		logout(INFO, "TAXLIB", "发票开具", "系统盘号%s,发票数据盘号%s\r\n", plate_infos->plate_basic_info.plate_num, stp_fpxx->jqbh);
		return DF_TAX_ERR_CODE_INV_DATA_DISACCORD;
	}
	/*主要商品税目名称*/
	if (strcmp(stp_fpxx->zyspsmmc, stp_spxx->spsmmc) != 0){
		out("check_fpxx_aisino test4\n");
		sprintf(errinfo, "主要商品税目名称需要第一行商品税目名称一致");
		return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
	}
	/*主要商品税目名称*/
	if (strcmp(stp_fpxx->zyspmc, stp_spxx->spmc) != 0){
		out("check_fpxx_aisino test5\n");
		out("主要商品名称：%s\n", stp_fpxx->zyspmc);
		out("第一行商品名称：%s\n", stp_spxx->spmc);
		sprintf(errinfo, "主要商品名称需要第一行商品名称一致");
		return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
	}

	/*商品税目名称*/
	while (stp_spxx_tmp){
		if ((strcmp(stp_spxx_tmp->spmc, "(详见对应正数发票清单)") != 0) && (strcmp(stp_spxx_tmp->spmc, "详见对应正数发票及清单") != 0))
		{
			i_spsmmclen = strlen(stp_spxx_tmp->spsmmc);
			if (i_spsmmclen < 3)
			{
				out("test i_spsmmclen < 3\n");
				sprintf(errinfo, "商品税目名称规则错误");
				return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
			}
			if (stp_spxx_tmp->spsmmc[0] != '*')
			{
				if ((strcmp(stp_spxx_tmp->spsmmc, "原价合计") == 0) || (strcmp(stp_spxx_tmp->spsmmc, "折扣额合计") == 0))
				{
					stp_spxx_tmp = stp_spxx_tmp->stp_next;
					continue;
				}

				out("test stp_spxx_tmp->spsmmc[0] != *,税目名：%s\n", stp_spxx_tmp->spsmmc);
				sprintf(errinfo, "商品税目名称规则错误");
				return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
			}

			if (stp_spxx_tmp->spsmmc[strlen(stp_spxx_tmp->spsmmc) - 1] != '*')
			{
				out("test stp_spxx_tmp->spsmmc[strlen(stp_spxx_tmp->spsmmc) - 1] != *\n");
				sprintf(errinfo, "商品税目名称规则错误");
				return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
			}
		}
		stp_spxx_tmp = stp_spxx_tmp->stp_next;
	}

	//税号检查
	if (!strlen(stp_fpxx->xfsh) || !strlen(stp_fpxx->gfmc) || !strlen(stp_fpxx->xfmc)) {
		out("[-] Need mc and sh");
		sprintf(errinfo, "销购方信息填写有误");
		return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
	}
	// 非专用发票gf税号可为空
	if (stp_fpxx->fplx_aisino == FPLX_AISINO_ZYFP) {
		if (!strlen(stp_fpxx->gfsh) || !strlen(stp_fpxx->gfdzdh) || !strlen(stp_fpxx->gfyhzh)) {
			out("[-] Zyfp gf info error");
			sprintf(errinfo, "增值税专用发票需填写购方税号、购方地址电话、购方银行账号");
			return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
		}
	}
	else {
		if (!strlen(stp_fpxx->gfsh))
			strcpy(stp_fpxx->gfsh, "000000000000000");
	}
	if (!strlen(stp_fpxx->gfsh) && (strlen(stp_fpxx->gfsh) != 15) && (strlen(stp_fpxx->gfsh) != 17) &&
		(strlen(stp_fpxx->gfsh) != 18) && (strlen(stp_fpxx->gfsh) != 20)) {
		out("[-] TaxID's gfsh length must be [15,20]");
		sprintf(errinfo, "购方税号长度有误非15、17、18、20位");
		return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
	}
	if ((strlen(stp_fpxx->xfsh) != 15) && (strlen(stp_fpxx->xfsh) != 17) && (strlen(stp_fpxx->xfsh) != 18) &&
		(strlen(stp_fpxx->xfsh) != 20)) {
		out("[-] TaxID's xfsh length must be [15,20]");
		sprintf(errinfo, "销方税号长度有误非15、17、18、20位");
		return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
	}
	////购方税号(购方税号!=销方税号)
	//if (strcmp((const char *)stp_fpxx->gfsh, (const char *)plate_infos->plate_basic_info.ca_number) == 0){
	//	out("gfsh = %s 等于 xfsh=%s", stp_fpxx->gfsh, plate_infos->plate_basic_info.ca_number);
	//	sprintf(errinfo, "购方税号不得等于销方税号");
	//	return DF_TAX_ERR_CODE_INV_DATA_DISACCORD;
	//}
	if (stp_fpxx->spsl > 6)
	{
		out("蒙柏盘商品数量不能超过6行\n");
		sprintf(errinfo, "商品行数不得大于6行");
		return DF_TAX_ERR_CODE_INV_DATA_TOO_LARGE;
	}
	return 0;
}


// 查询当前发票卷信息
int mengbai_read_current_inv_code(HUSB hUSB, unsigned char inv_type, char* invnum, char* invcode)
{
	unsigned char type[4];
	int type_tmp[4];
	unsigned char data[4];
	int data_i;
	int result;
	int in_len, out_len;
	unsigned char s_data[4096];
	unsigned char r_data[4096];
	char errinfo[500] = { 0 };

	sprintf((char *)type, "%03d", inv_type);

	//out("获取发票卷信息\n");
	memset(s_data, 0, sizeof(s_data));
	memset(r_data, 0, sizeof(r_data));
	in_len = pack_s_buf(DF_PC_USB_ORDER_GET_WSYFPJ, 0, type, s_data, strlen((const char*)type));
	out_len = sizeof(r_data);
	result = usb_data_write_read(hUSB, s_data, in_len, r_data, &out_len, errinfo);
	if (result < 0)
	{
		out("获取卡信息失败,发票类型代码为：%d\n", inv_type);
		return -1;
	}


	memset(type_tmp, 0, sizeof(type_tmp));
	memcpy(type_tmp, r_data, 3);
	if (memcmp(type, type_tmp, 3) != 0)
	{
		out("获取到的发票类型与要查询的发票类型不一致\n");
		return -1;
	}


	memcpy(invcode, r_data + 3, 20);
	//out("结构内存号%d,发票种类%d,当前发票代码%s\n", type_count, type_num, plate_infos->invoice_type_infos[type_mem].invoice_coils_info.dqfpdm);

	memset(data, 0, sizeof(data));
	memcpy(data, r_data + 23, 4);
	data_i = data[0] * 0x1000000 + data[1] * 0x10000 + data[2] * 0x100 + data[3] * 0x1;
	sprintf(invnum, "%08d", data_i);
	return 0;
}


static int mengbai_upload_firstOfflineInv(HDEV hDev, struct _offline_upload_result *offline_upload_result)
{
	unsigned char type[4];
	unsigned char type_num;
	//int type_tmp[4];
	int type_count;
	//int type_mem;
	int in_len;
	int out_len;
	int result;
	//int count = 0;
	int offline_count = 0;
	int offline_num = 0;
	unsigned char r_data[4096];
	unsigned char s_data[4096];
	char errinfo[100] = { 0 };

	uint8 abInvoiceTypes[] = { 4, 7, 26 };

	for (type_count = 0; type_count < 3; type_count++)
	{

		type_num = abInvoiceTypes[type_count];
		out("查询发票类型为%d的离线票份数\n", type_num);
		memset(type, 0, sizeof(type));
		sprintf((char *)type, "%03d", type_num);
		//out("获取离线票份数\n");
		memset(s_data, 0, sizeof(s_data));
		memset(r_data, 0, sizeof(r_data));
		in_len = pack_s_buf(DF_PC_USB_ORDER_GET_LXPFS, 0, type, s_data, strlen((const char*)type));
		out_len = sizeof(r_data);
		//char s_time[100] = { 0 };
		//char e_time[100] = { 0 };
		//timer_read_asc(s_time);
		//printf_array(s_data, in_len);
		result = usb_data_write_read(hDev->hUSB, s_data, in_len, r_data, &out_len, errinfo);
		if (result < 0)
		{
			out("查询发票类型为%d的离线票份数失败\n", type_num);
			continue;
		}
		//timer_read_asc(e_time);
		offline_num = r_data[3] * 0x1000000 + r_data[4] * 0x10000 + r_data[5] * 0x100 + r_data[6] * 0x1;
		
		out("查询发票类型为%d的离线票份数为%d\n", type_num,offline_num);
		if (offline_num == 0)
			continue;
		for (offline_count = 0; offline_count<offline_num; offline_count++)
		{
			int num = 1;
			int in_len;
			int out_len;
			int result;
			unsigned char inv_data[4096];
			unsigned char pack[20];
			unsigned char jshj[20] = { 0 };
			char errinfo[100] = { 0 };

			out("查询发票类型为%s的第一张离线票数据\n", type);
			memset(pack, 0, sizeof(pack));
			memcpy(pack, type, 3);
			pack[3] = ((num >> 24) & 0xff); pack[4] = ((num >> 16) & 0xff);
			pack[5] = ((num >> 8) & 0xff); pack[6] = ((num >> 0) & 0xff);


			//out("获取离线票详情\n");
			memset(s_data, 0, sizeof(s_data));
			memset(r_data, 0, sizeof(r_data));
			in_len = pack_s_buf(DF_PC_USB_ORDER_GET_SCLXP, 0, pack, s_data, 7);
			out_len = sizeof(r_data);
			//print_array((char *)s_data,in_len);
			result = usb_data_write_read(hDev->hUSB, s_data, in_len, r_data, &out_len, errinfo);
			if (result < 0)
			{
				out("查询发票类型为%s的离线票明细失败\n", type);
				continue;
			}


			out("%s离线票获取成功,组成上报所需json数据包\n", type);
			memset(inv_data, 0, sizeof(inv_data));
			pack_offline_report_json_data(r_data, (unsigned char*)inv_data, jshj, hDev->szCompanyName, hDev->szCommonTaxID);
			str_replace((char*)inv_data, "\\\\", "\\");
			
			mengbai_set_invoice_to_plate(hDev->hUSB, type_num);
			continue;

		}

	}
	return 100;	
}


int mengbai_inv_upload_server(HDEV hUSB, struct _offline_upload_result *offline_upload_result)
{
	int result;
	//载入设备信息
	if (hUSB->bTrainFPDiskType == 0)
	{
		result = mengbai_upload_firstOfflineInv(hUSB, offline_upload_result);
	}
	else
	{
		_WriteLog(LL_INFO, "[+]测试盘不需要发票上传\n");
		sprintf((char *)offline_upload_result->errinfo, "测试盘不需要发票上传");
		result = 99;
	}
	//_WriteLog(LL_INFO, "发票上传完成,result = %d\n", result);
	return result;
}


int mengbai_check_server_address(HDEV hDev, char *errinfo)
{
	char szIP[100] = {0};
	if (Domain2IP("www.njmbxx.com", szIP) <= 0)
	{
		sprintf(errinfo,"域名“www.njmbxx.com”解析失败");
		return -1;
	}
	return  0;
}



int LoadMengBaiSeverKey(HUSB hUSB, HDEV hDev)
{
	int in_len, out_len;
	unsigned char s_data[4096];
	unsigned char r_data[4096];
	int result;
	char errinfo[2048] = { 0 };
	//联机命令
	//////////////////////////////////////////////////////////////////////////
	//out("联机命令\n");
	char time_now[20];
	memset(time_now, 0, sizeof(time_now));

	timer_read_y_m_d_h_m_s(time_now);
	//out("设置当前时间%s\n", time_now);

	strcpy(hDev->szDeviceTime, time_now);

	//////////print_array((char *)usb_data, size);
	//////////////////////////////////////////////////////////////////////
	memset(s_data, 0, sizeof s_data);
	in_len = pack_s_buf(DF_PC_USB_ORDER_CONNECT, 0, (unsigned char *)time_now, s_data, strlen(time_now));
	out_len = sizeof(r_data);

	result = usb_data_write_read(hUSB, s_data, in_len, r_data, &out_len, errinfo);
	if (result < 0)
	{
		out("联机失败\n");
		return result;
	}

	//获取卡信息
	////////////////////////////////////////////////////////////////////////
	//out("获取卡信息\n");
	memset(s_data, 0, sizeof(s_data));
	memset(r_data, 0, sizeof(r_data));
	in_len = pack_s_buf(DF_PC_USB_ORDER_GET_CARD_IFNFO, 0, NULL, s_data, 0);
	out_len = sizeof(r_data);
	result = usb_data_write_read(hUSB, s_data, in_len, r_data, &out_len, errinfo);
	if (result < 0)
	{
		out("获取卡信息失败\n");
		return -1;
	}
	//out("获取卡信息成功\n");

	hDev->uICCardNo = 0;
	memcpy(hDev->szDeviceID, r_data, 12);
	memcpy(hDev->szCommonTaxID, r_data + 12 + 1024 + 1024, 12);
	memcpy(hDev->szCompanyName, r_data + 12 + 1024 + 1024 + 12, 80);
	out("获取到的卡编号：%s,识别号：%s,名称：%s\n", hDev->szDeviceID, hDev->szCommonTaxID, hDev->szCompanyName);
	return 0;
}

int mengbai_get_server_key_info(HUSB hUSB, HDEV hDev, char *info)
{
	
	int in_len, out_len;
	unsigned char s_data[4096] = { 0 };
	unsigned char r_data[4096] = { 0 };
	int result;
	char errinfo[2048] = { 0 };
	char key[4096] = { 0 };

	char pb[2048] = {0};
	char pv[2048] = {0};

	char pb_stand[2048] = {0};
	char pv_stand[2048] = {0};
	//获取卡信息
	////////////////////////////////////////////////////////////////////////
	//out("获取卡信息\n");
	memset(s_data, 0, sizeof(s_data));
	memset(r_data, 0, sizeof(r_data));
	in_len = pack_s_buf(DF_PC_USB_ORDER_GET_CARD_IFNFO, 0, NULL, s_data, 0);
	out_len = sizeof(r_data);
	result = usb_data_write_read(hUSB, s_data, in_len, r_data, &out_len, errinfo);
	if (result < 0)
	{
		out("获取卡信息失败\n");
		return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
	}

	memcpy(key, r_data + 12, 2048);
	//out("设备密钥：%s\n", key);


	cJSON *item;
	cJSON *root = cJSON_Parse(key);
	if (!root)
	{
		out("Error before: [%s]\n", cJSON_GetErrorPtr());
		return -1;
	}
	item = cJSON_GetObjectItem(root, "pb");
	if (get_json_value_can_not_null(item, pb, 0, 2048) < 0) {
		out("解析pb失败\n");
		cJSON_Delete(root);
		return -2;
	}

	item = cJSON_GetObjectItem(root, "pv");
	if (get_json_value_can_not_null(item, pv, 0, 2048) < 0) {
		out("解析pv失败\n");
		cJSON_Delete(root);
		return -3;
	}

	if (strlen(pb))
	{
		int count;
		int last = 0;
		char tmp[65] = { 0 };
		char real_key[2048] = { 0 };
		int j;

		count = strlen(pb) / 64;
		last = strlen(pb) % 64;
		for (j = 0; j < count; j++)
		{
			memset(tmp, 0, 64);
			memcpy(tmp, pb + (64 * j), 64);
			sprintf(real_key + strlen(real_key), "%s\n", tmp);
		}
		memset(tmp, 0, 64);
		memcpy(tmp, pb + (64 * count), last);
		sprintf(real_key + strlen(real_key), "%s", tmp);

		sprintf(pb_stand, "-----BEGIN PUBLIC KEY-----\n%s\n-----END PUBLIC KEY-----", real_key);

	}
	if (strlen(pv))
	{
		int count;
		int last = 0;
		char tmp[65] = { 0 };
		char real_key[2048] = { 0 };
		int j;

		count = strlen(pv) / 64;
		last = strlen(pv) % 64;
		for (j = 0; j < count; j++)
		{
			memset(tmp, 0, 64);
			memcpy(tmp, pv + (64 * j), 64);
			sprintf(real_key + strlen(real_key), "%s\n", tmp);
		}
		memset(tmp, 0, 64);
		memcpy(tmp, pv + (64 * count), last);
		sprintf(real_key + strlen(real_key), "%s", tmp);


		sprintf(pv_stand, "-----BEGIN RSA PRIVATE KEY-----\n%s\n-----END RSA PRIVATE KEY-----", real_key);
	}
	//_WriteLog(LL_INFO,"设备私钥：\n%s\n", pv_stand);
	//_WriteLog(LL_INFO,"服务器公钥：\n%s\n", pb_stand);

	char time_now[20];
	memset(time_now, 0, sizeof(time_now));
	timer_read_y_m_d_h_m_s(time_now);


	int tmp_time[7];
	
	sscanf((char *)time_now, "%02x%02x-%02x-%02x %02x:%02x:%02x", &tmp_time[0], &tmp_time[1], &tmp_time[2], &tmp_time[3], &tmp_time[4], &tmp_time[5], &tmp_time[6]);


	char time_data[20] = { 0 };
	sprintf(time_data, "%02x%02x%02x%02x%02x%02x", tmp_time[4], tmp_time[1], tmp_time[5], tmp_time[2], tmp_time[6], tmp_time[3]);


	//out("加密日期：%s\n", time_data);

	//原始数据为时间12位字符串,公钥加密,加密数据进行base64编码放在返回结果前部,再将加密的的base64数据进行MD5,再将MD5值进行私钥签名,签名后的数据进行base64编码并放在返回结果的后部

	unsigned char pb_en_data[2048] = { 0 };
	unsigned int pb_len;
	pb_len = public_encrypt((uint8 *)time_data, strlen(time_data), (uint8 *)pb_stand, (uint8 *)pb_en_data);

	char base_pb_data[2048] = { 0 };
	EVP_EncodeBlock((uint8 *)base_pb_data, (const uint8 *)pb_en_data, pb_len);


	unsigned char pv_sign_data[2048] = { 0 };
	unsigned int pv_len;

	MD5_CTX ctx;
	unsigned char outmd[20] = { 0 };
	MD5_Init(&ctx);
	MD5_Update(&ctx, base_pb_data, strlen(base_pb_data));
	MD5_Final((uint8 *)outmd, &ctx);

	private_sign((uint8 *)outmd, 16, (uint8 *)pv_sign_data, &pv_len, (uint8 *)pv_stand);


	unsigned char base_pv_data[2048] = { 0 };
	EVP_EncodeBlock((uint8 *)base_pv_data, (const uint8 *)pv_sign_data, pv_len);

	
	//printf("\n\n%s\n", base_pv_data);
	sprintf(info, "%s$&&$%s", base_pb_data, base_pv_data);
	//printf("\n\n%s\n", info);
	cJSON_Delete(root);
	return 0;
}