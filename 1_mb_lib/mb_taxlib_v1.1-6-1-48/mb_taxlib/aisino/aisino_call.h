#ifndef AISINO_CALL_H
#define AISINO_CALL_H

#include "../common/function.h"
#include "module/aisino_common.h"
#include "module/aisino_netinvoice.h"

int my_aisino_read_taxID(HUSB hUSB);
int my_aisino_read_ID(HUSB hUSB);
int my_aisino_read_invoice(HDEV hDev, char *szFormatMonth);
int my_aisino_read_taxInfo(HUSB hUSB);
int my_aisino_read_InvNumber(HUSB hUSB);
int my_aisino_make_invoice(HDEV hDev);
int my_aisino_upload_OfflineInv(HDEV hDev, char *szFPDMIn, char *szFPHMIn, int nDZSYH);
int my_aisino_waste_inv(HDEV hDev, char *szFPDM, char *szFPHM);
int my_aisino_report_clear(HDEV hDev, uint8 bReportOrClear);
int my_aisino_upload_check_redinvform(HDEV hDev);
int my_aisino_redinv_check(HDEV hDev, uint8 bAisinoFPLX, char *szLZFPDM, char *szLZFPHM,
                           uint32 nDZSYH);
int my_aisino_change_certpassword(HUSB hUsb, char *szOldPassword, char *szNewPassword);
int my_aisino_get_pubserviceurl(HDEV hDev, char *szFPDM, char *szFPHM, int nDZSYH);
int my_aisino_switch_pubservice(HDEV hDev, bool bEnableOrDisable);
int my_aisino_month_statistic(HDEV hDev, char *szFormatYearMonth);

#ifdef aisino_call_c
static int invoices_coil_data_to_struct(unsigned char fplxdm, unsigned char aisino_fplxdm, unsigned char *data, unsigned char *data2, struct _plate_infos *plate_infos);
static int aisino_upload_firstOfflineInv(HDEV hDev, struct _offline_upload_result *offline_upload_result);
static int check_fpxx_aisino(struct Fpxx* stp_fpxx, struct _plate_infos *plate_infos, char *errinfo);
#endif 
//int aisino_read_basic_info(HUSB hUSB, struct _plate_infos *plate_infos);
int aisino_read_monitor_info(HUSB hUSB, HDEV hDev, struct _plate_infos *plate_infos);
int aisino_read_inv_coil_number(HUSB hUSB, struct _plate_infos *plate_infos);
int aisino_read_tax_time(HDEV hDev, char* outtime);
int aisino_get_inv_details_mqtt(HDEV hDev, unsigned char *month, void *function, void *arg, char *errinfo, int old_new, unsigned short *start_stop, int sfxqqd);
int aisino_upload_m_server(HDEV hDev, unsigned short *start_stop, struct _upload_inv_num *upload_inv_num, void *function, void *arg, struct _inv_sum_data *now_month_sum);
int aisino_upload_summary_m_server(HDEV hDev, unsigned short *start_stop, struct _upload_inv_num *upload_inv_num, void *function, void *arg, struct _inv_sum_data *now_month_sum);
int aisino_is_exsist_offline_inv(HDEV hDev, char *fpdm, char *fphm, char *kpsj, int *dzsyh);
int aisino_fpdm_fphm_get_invs(HDEV hDev, char  *fpdm, char *fphm, int dzsyh, char **inv_json);
int aisino_fpdm_fphm_get_invs_kpstr(HDEV hDev, char  *fpdm, char *fphm, int dzsyh, char **inv_json, char **kp_bin, char **kp_str);
int aisino_my_read_invoice_month_num(HUSB hUSB, char *szFormatMonth, unsigned int *Count, unsigned long *Size);
int aisino_read_current_inv_code(HUSB hUSB, uint8_t bNeedType, char *szInvCurrentNum, char *szInvTypeCode);
int aisino_inv_upload_server(HDEV hDev, struct _offline_upload_result *offline_upload_result);
int aisino_verify_cert_passwd(HDEV hDev, int *left_num);
int aisino_make_invoice(HDEV hDev, char* inv_data, struct _plate_infos *plate_infos, struct _make_invoice_result *make_invoice_result);
int aisino_waste_invoice(HDEV hDev, unsigned char inv_type, char  *fpdm, char *fphm, char *zfr, struct _cancel_invoice_result *cancel_invoice_result);
int aisino_cancel_null_invoice_to_plate(HDEV hDev, struct _cancel_invoice_result *cancel_invoice_result);
int aisino_read_tax_rate(HDEV hDev, struct _plate_infos *plate_infos);
int aisino_copy_report_data(HDEV hDev, uint8 inv_type, char *errinfo);
int aisino_report_clear(HDEV hDev, uint8 inv_type, char *errinfo);
int aisino_fpdm_fphm_update_invs(HDEV hDev, char  *fpdm, char *fphm, int dzsyh, char **inv_json);
int aisino_upload_check_redinvform(HDEV hDev, uint8 inv_type, char* inv_data, struct _plate_infos *plate_infos, struct _askfor_tzdbh_result *askfor_tzdbh_result);
int aisino_redinv_check(HDEV hDev, uint8 bAisinoFPLX, char *szLZFPDM, char *szLZFPHM, uint32 nDZSYH,char *errinfo);
int aisino_download_hzxxb_from_rednum(HDEV hDev, char *redNum, uint8 **inv_json, char *errinfo);
//int aisino_get_download_net_invoice_coil(HDEV hDev, char *date_range, char *errinfo);接口停用
int aisino_change_certpassword(HUSB hUSB, char *szOldPassword, char *szNewPassword, char *errinfo);//注意!!!这是单独接口,只能打开USB handle后单独调用.不能融入主流程，因为修改完之后需要重新连接设备
int aisino_redinv_tzdbh_cancel(HDEV hDev, char *szTZDBH, char *errinfo);
int aisino_download_hzxxb_from_date_range(HDEV hDev, char *date_range, uint8 **tzdbh_data, char *errinfo);
int aisino_download_hzxxb_from_date_range_new(HDEV hDev, char *date_range,char *gfsh, uint8 **tzdbh_data, char *errinfo);
int aisino_query_net_invoice_coil(HDEV hDev, char *date_range, char *inv_data, char *errinfo);
int aisino_query_net_invoice_coil_download_unlock(HDEV hDev, char *date_range, uint8 fllxdm, char *fpdm, char *fpqshm, int fpzs, char *errinfo);
int aisino_check_server_address(HDEV hDev, char *errinfo);
int aisino_connect_pubservice(HDEV hDev, uint8 inv_type, char *errinfo);
int aisino_disconnect_pubservice(HDEV hDev, uint8 inv_type, char *errinfo);
int aisino_query_invoice_month_all_data(HDEV hDev, char *month, char **data_json,char *errinfo);
int aisino_client_hello(HDEV hDev, char *client_hello, char *errinfo);
int aisino_client_auth(HDEV hDev, char *server_hello, char *client_auth, char *errinfo);
int aisino_get_cert(HDEV hDev, char *cert_data, char *errinfo);
int aisino_update_summary_data(HDEV hDev, struct _plate_infos *plate_infos);
#endif