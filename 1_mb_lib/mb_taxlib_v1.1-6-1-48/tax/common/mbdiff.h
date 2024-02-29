#ifndef _MBDIFF_H
#define _MBDIFF_H
#include "common.h"

#include "function.h"
#define AUTH_CERT_FILE_PATH		"/etc/auth_cert.json"

#ifdef mbdiff_c
#define out(s,arg...)               log_out("all.c",s,##arg)
#else
#endif

struct _busid_auth
{
	int state;
	int port;
	char busid[32];
	int aisino_sup;
	int nisec_sup;
	int cntax_sup;
	int mengbai_sup;
	char start_time[20];//起始时间
	char end_time[20];//结束时间
	char aisino_end_time[20];
	char nisec_end_time[20];
	char cntax_end_time[20];
	char mengbai_end_time[20];
	//char remark[20];
};

struct _tax_auth
{
	char version[128];
	char serial_num[128];
	char name[128];
	char auth_type[10];//usb-share授权类型，whole part，whole模式不存在时效 part模式存在shixiao
	char start_time[20];//usb-share起始时间
	char end_time[20];//usb-share结束时间
	struct _busid_auth busid_auth[MAX_USB_PORT_NUM];

};
struct _tax_auth  tax_auth;

int init_tax_lib(uint8 *key_zlib_json, char *taxlib_version);
int reload_tax_auth(uint8 *key_zlib_json);
int jude_plate_auth(uint8 bDeviceType, char *busid, char *errinfo);
int get_cert_json_file(uint8 *key_zlib_json, char **cert_json);
int jude_busnum_devnum_handle(struct HandlePlate *h_plate);


#ifdef mbdiff_c
#ifdef RELEASE_SO
static int analysis_auth_cert_json(char *cert_json);
#endif
#endif


#define DF_INV_MONTH_COMPLETE  1


#define DF_ERR_SERVER_ADDRESS	"www.njmbxx.com"
//#define DF_ERR_SERVER_ADDRESS	"103.45.250.88"

#define DF_ERR_SERVER_PORT 20020
#define DF_ERR_SERVER_APPID "EE8B56879B7D5449933C1F8918C33FB6"
#define DF_ERR_SERVER_APPSECERT "74993F73731FAE499F0F2A98CC4A9E90"

#define DF_FPQD_N 0
#define DF_FPQD_Y 1

#define DF_FP_JSON 0
#define DF_FP_BASE 1



int aisino_usbio_err_code_find_describe(char *szOutErrorCode, char *describe);
int QueryInvInfoOutBinStr(HDEV hDev, char *fpdm, char *fphm, uint32 nDZSYH, HFPXX pOutFpxx, char **bindata, char **kpstr);
int invoice_bin_fpxx_json_base_outfpxx(struct StaticDeviceInfo *pDevInfo, char *pInvDecBuf, char **inv_json, int sfxqqd, HFPXX fpxx);
int invoice_bin_fpxx_json_base(struct StaticDeviceInfo *pDevInfo, char *pInvDecBuf, char **inv_json, int sfxqqd, uint8 *zfbz);
int invoice_bin_fpxx_json_base_old(struct StaticDeviceInfo *pDevInfo, char *pInvDecBuf, char **inv_json);
int LoadAisinoSelectDevInfo(HDEV hDev, HUSB hUSB, uint8 bAuthDev);
//int NetInvoiceGet(HDEV hDev, char *date_range);
void find_inv_type_mem(uint8 type, int *mem_i);
void check_special_char(uint8 *str);
int check_invoice_str(uint8 *str);
int GetDnsIp(char *host, char *ip);
int jude_need_upload_inv(struct _upload_inv_num *upload_inv_num, HFPXX fpxx);
int add_inv_sum_data(HFPXX fpxx, struct _inv_sum_data *inv_sum);
int compare_inv_sum_statistics(struct _inv_sum_data *inv_sum, char *statistics);
int mb_get_today_month(char *today_month);
int mb_get_next_month(char *now_month);
int mb_get_last_month_first_day(char *now_month, char *last_month_fistday);
int GetRedInvFromRedNum(HDEV hDev, char *redNum, uint8 **inv_json);
int swukey_get_basic_info(HDEV hDev, struct _plate_infos *plate_info);
int nisec_usbio_err_code_find_describe(char *szOutErrorCode, char *describe);
int LoadSkpKeyDevInfo(HDEV hDev, HUSB hUSB);
int fpxx_to_json_base_fpsjbbh_v102(HFPXX stp_fpxx, char **fpxx_json, int sfxqqd, int data_type);
int fpxx_to_json_base_fpsjbbh_v102_aison(HFPXX stp_fpxx, char **fpxx_json, int sfxqqd,int data_type);
int fpxx_to_json_rednum_data_v102(HFPXX stp_fpxx, uint8 **fpxx_json, int sfxqqd);
int fpxx_to_json_base_ykfpcx(HFPXX stp_fpxx, char **json_data);
int get_fpxx_from_hzxxb_data_aisino_nisec(HDEV hDev, char *szRep, uint8 *send_data, int send_len, char *errinfo);
int get_fpxx_from_hzxxb_data_cntax(HDEV hDev, char *szRep, uint8 *send_data, int send_len, char *errinfo);
int tcp_http_netPost(char *hostname, int port, char *request, char *content_type, char *parmdata, int time_out, char **returndata, int *returndata_len);
int report_event(char *plate_num, char *err, char *errinfo, int result);
int turn_month_to_range(char *month, char *date_range);
int turn_month_to_date_range(char *month, char *time_now, char *start_date, char *end_date);
int get_plate_invs_sync(char *plate_num, char ** invs_data);
int RedInvOnlineCheck_TZD(HFPXX fpxx,uint8 **red_inv);
void Test_TAConnect(HDEV hDev);
int HZXMLNodeMx2FPXXCntax(mxml_node_t *pXMLChildRoot, struct Spxx *spxx);
int HZXMLNode2FPXXCntax(mxml_node_t *pXMLChildRoot, HFPXX fpxx);
int LoadZZSXMLDataNodeTemplateCntax(mxml_node_t **pXMLRootOut, mxml_node_t **nodeDataNode,char *szDataNodeName, char *szInputXMLAndOutputError);
int GetFpxxFromRepByRedNumCntax(char *szRep, char *szRedNum, HFPXX fpxx);


int AisinoNetInvoiceQueryReadyEasy(HHTTP hi, HDEV hDev, char *szInputQueryDateRange, char *szOutFormatStrMultiLine);
int NisecNetInvoiceQueryReadyEasy(HHTTP hi, HDEV hDev, char *szOutFormatStrMultiLine);
int CntaxNetInvoiceQueryReadyEasy(HHTTP hi, HDEV hDev, char *szOutFormatStrMultiLine);

int AisinoNetInvoiceDownloadUnlockEasy(HHTTP hi, HDEV hDev, char *szInputFormatStrSingleLine);
int NisecNetInvoiceDownloadWriteEasy(HHTTP hi, HDEV hDev, char *szInputFormatSingleLine);
int CntaxNetInvoiceDownloadWriteEasy(HHTTP hi, HDEV hDev, char *szInputFormatSingleLine);

int send_zip_data_hook(HDEV hDev, char *month, void *function, void *arg, int need_up_count, zipFile zf, char * fp_zip_name, int zip_data_len, int is_over);
int deal_plate_statistics_to_now_month_sum(char *statistics, struct _inv_sum_data *now_month_sum);
#endif