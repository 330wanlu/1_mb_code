#ifndef NISEC_CALL_H
#define NISEC_CALL_H
#include "module/nisec_common.h"

int my_nisec_read_invoice(HDEV hDev, char *szQueryDateRange);
int my_nisec_read_id(HDEV hDev);
int my_nisec_read_buyinvinfo_alldisplay(HDEV hDev);
int my_nisec_read_taxinfo_alldisplay(HUSB hUSB);
int my_nisec_make_invoice(HDEV hDev);
int my_nisec_read_upload_invoice(HDEV hDev);
int my_nisec_upload_invoice_condition(HDEV hDev, uint8 bInvType, char *szFPDM, char *szFPHM);
int my_nisec_change_certpassword(HDEV hDev, char *szOldPassword, char *szNewPassword);
int my_nisec_report_clear(HDEV hDev, uint8 bReportOrClear);
int my_nisec_get_pubserviceurl(HDEV hDev, uint8 bInvType, char *szFPDM, char *szFPHM);
int my_nisec_redinv_operate(HDEV hDev);
int my_nisec_switch_pubservice(HDEV hDev, bool bEnableOrDisable);



//封装函数用于上层业务，切勿删除
int nisec_get_basic_info(HDEV hDev, struct _plate_infos *plate_info);
int nisec_get_state_info(HDEV hDev, struct _plate_infos *plate_info);
int nisec_read_inv_coil_number(HDEV hDev, struct _plate_infos *plate_info);
int nisec_get_state_info_and_coil_info(HDEV hDev, HUSB hUSB, struct _plate_infos *plate_info);
int nisec_read_tax_time(HDEV hDev, char *outtime);
int nisec_get_inv_details_mqtt(HDEV hDev, unsigned char *month, void *function, void *arg, char *errinfo, int old_new, unsigned short *start_stop, int sfxqqd);
int nisec_upload_m_server(HDEV hDev, unsigned short *start_stop, struct _upload_inv_num *upload_inv_num, void *function, void *arg, struct _inv_sum_data *now_month_sum);
int nisec_upload_summary_m_server(HDEV hDev, unsigned short *start_stop, struct _upload_inv_num *upload_inv_num, void *function, void *arg, struct _inv_sum_data *now_month_sum);
int nisec_make_invoice(HDEV hDev, char* inv_data, struct _plate_infos *plate_infos, struct _make_invoice_result *make_invoice_result);
int nesic_my_read_invoice_num(HDEV hDev, char *month, unsigned int *count, unsigned long *size);
int nisec_read_current_inv_code(HDEV hDev, uint8_t bNeedType, char *szInvCurrentNum, char *szInvTypeCode);
int nisec_waste_invoice(HDEV hDev, uint8 bInvType, char *szFPDM, char *szFPHM, struct _cancel_invoice_result *cancel_invoice_result);
int nisec_cancel_null_invoice_to_plate(HDEV hDev, struct _cancel_invoice_result *cancel_invoice_result);
int nisec_fpdm_fphm_get_invs(HDEV hDev, char  *fpdm, char *fphm, char **inv_json);
int nisec_is_exsist_offline_inv(HDEV hDev, char *fpdm, char *fphm, char *kpsj, int *dzsyh);
int nisec_inv_upload_server(HDEV hDev, struct _offline_upload_result *offline_upload_result);
int nisec_fpdm_fphm_update_invs(HDEV hDev, char  *fpdm, char *fphm, char **inv_json);
int nisec_check_server_address(HDEV hDev, char **splxxx, char *errinfo);
int nisec_verify_cert_passwd(HDEV hDev, int *left_num);
int nisec_change_certpassword(HDEV hDev, char *szOldPassword, char *szNewPassword, char *errinfo);
int nisec_copy_report_data(HDEV hDev, uint8 inv_type, char *errinfo);
int nisec_report_clear(HDEV hDev, uint8 inv_type, char *errinfo);
int nisec_upload_check_redinvform(HDEV hDev, uint8 inv_type, char* inv_data, struct _plate_infos *plate_infos, struct _askfor_tzdbh_result *askfor_tzdbh_result);
int nisec_download_hzxxb_from_rednum(HDEV hDev, char *redNum, uint8 **inv_json, char *errinfo);
int nisec_download_hzxxb_from_date_range(HDEV hDev, char *date_range, uint8 **tzdbh_data, char *errinfo);
int nisec_redinv_tzdbh_cancel(HDEV hDev, char *redNum, char *errinfo);
int nisec_connect_pubservice(HDEV hDev, uint8 inv_type, char *errinfo);
int nisec_disconnect_pubservice(HDEV hDev, uint8 inv_type, char *errinfo);
int nisec_query_net_invoice_coil(HDEV hDev, char *date_range, char *inv_data, char *errinfo);
int nisec_query_net_invoice_coil_download_unlock(HDEV hDev, char *date_range, uint8 fllxdm, char *fpdm, char *fpqshm, int fpzs, char *errinfo);
int nisec_read_inv_coil_number_history(HDEV hDev, char **data_json, char *errinfo);
int nisec_query_invoice_month_all_data(HDEV hDev, char *month, char **data_json, char *errinfo);
int nisec_client_hello(HDEV hDev, char *client_hello, char *errinfo);
int nisec_client_auth(HDEV hDev, char *server_hello, char *client_auth, char *errinfo);
int nisec_get_cert(HDEV hDev, char *cert_data, char *errinfo);
int nisec_update_summary_data(HDEV hDev, struct _plate_infos *plate_infos);
#endif
