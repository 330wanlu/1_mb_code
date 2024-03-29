#ifndef interface_main_h
#define interface_main_h
#include "aisino/aisino_call.h"
#include "nisec/nisec_call.h"
#include "mengbai/mengbai_call.h"
#include "cntax/cntax_call.h"
#include "etax/etax_call.h"
#include "common/define.h"
#include "common/mbdiff.h"
#include "common/function.h"

#ifdef interface_main
#ifdef DEBUG
#define out(s,arg...)       				log_out("all.c",s,##arg) 
#else
#define out(s,arg...)						log_out("all.c",s,##arg)
#endif

#endif

int DetectDeviceInit(HDEV hDev, HUSB hUSB, uint8 bAuthDev, int nBusID, int nDevID,char *busid);
int ConsoleClose(int nDeviceType, HUSB hUsb);
//====//
//
int function_common_init_tax_lib(uint8 *key_zlib_json, char *taxlib_version);
//
int function_common_reload_tax_auth(uint8 *key_zlib_json);
//
int function_common_jude_plate_auth(uint8 bDeviceType, char *busid,char *errinfo);
//
int function_common_init_tax_lib_log_hook(void *function, void *arg);
//
int function_common_find_inv_type_mem(uint8 type, int *mem_i);
//
int function_common_get_cert_json_file(uint8 *key_zlib_json, char **cert_json);
//
int function_common_report_event(char *plate_num, char *err, char *errinfo, int result);
//
int function_common_analyze_json_buff(uint8 bDeviceType, char *json_data, HFPXX stp_fpxx, char *errinfo);


//USBDEV
int function_common_get_basic_simple(struct HandlePlate *h_plate, struct _plate_infos *plate_info);

//ukeykeykey
int function_common_get_basic_tax_info(struct HandlePlate *h_plate, struct _plate_infos *plate_info);

//USB
int function_common_close_usb_device(struct HandlePlate *h_plate);

//====//
//
int function_aisino_get_tax_time(struct HandlePlate *h_plate, char *outtime);
int function_nisec_get_tax_time(struct HandlePlate *h_plate, char *outtime);
int function_cntax_get_tax_time(struct HandlePlate *h_plate, char* outtime);
int function_mengbai_get_plate_time(struct HandlePlate *h_plate, char *outtime);
//mqtt
int function_aisino_get_inv_details_mqtt(struct HandlePlate *h_plate, unsigned char *month, void *function, void *arg, char *errinfo, int old_new, unsigned short *start_stop, int sfxqqd);
int function_nisec_get_inv_details_mqtt(struct HandlePlate *h_plate, unsigned char *month, void *function, void *arg, char *errinfo, int old_new, unsigned short *start_stop, int sfxqqd);
int function_cntax_get_inv_details_mqtt(struct HandlePlate *h_plate, unsigned char *month, void *function, void *arg, char *errinfo, int old_new, unsigned short *start_stop, int sfxqqd);
int function_mengbai_get_inv_details_mqtt(struct HandlePlate *h_plate, unsigned char *month, void *function, void *arg, char *errinfo, int old_new, unsigned short *start_stop, int sfxqqd);
//M
int function_aisino_upload_m_server(struct HandlePlate *h_plate, unsigned short *start_stop, struct _upload_inv_num *upload_inv_num, void *function, void *arg, struct _inv_sum_data *now_month_sum);
int function_nisec_upload_m_server(struct HandlePlate *h_plate, unsigned short *start_stop, struct _upload_inv_num *upload_inv_num, void *function, void *arg, struct _inv_sum_data *now_month_sum);
int function_cntax_upload_m_server(struct HandlePlate *h_plate, unsigned short *start_stop, struct _upload_inv_num *upload_inv_num, void *function, void *arg, struct _inv_sum_data *now_month_sum);
int function_mengbai_upload_m_server(struct HandlePlate *h_plate, unsigned short *start_stop, struct _upload_inv_num *upload_inv_num, void *function, void *arg, struct _inv_sum_data *now_month_sum);
//
int function_aisino_get_month_invoice_num(struct HandlePlate *h_plate, unsigned char *month, unsigned int  *count, unsigned long *size);
int function_nisec_get_month_invoice_num(struct HandlePlate *h_plate, unsigned char *month, unsigned int  *count, unsigned long *size);
int function_cntax_get_month_invoice_num(struct HandlePlate *h_plate, unsigned char *month, unsigned int  *count, unsigned long *size);
int function_mengbai_get_month_invoice_num(struct HandlePlate *h_plate, unsigned char *month, unsigned int  *count, unsigned long *size);
//
int function_aisino_fpdm_fphm_get_invs(struct HandlePlate *h_plate, char  *fpdm, char *fphm, int dzsyh, char **inv_json);
int function_nisec_fpdm_fphm_get_invs(struct HandlePlate *h_plate, char  *fpdm, char *fphm, int dzsyh, char **inv_json);
int function_cntax_fpdm_fphm_get_invs(struct HandlePlate *h_plate, char  *fpdm, char *fphm, int dzsyh, char **inv_json);

//
int function_aisino_fpdm_fphm_get_invs_kpstr(struct HandlePlate *h_plate, char  *fpdm, char *fphm, int dzsyh, char **inv_json, char **kp_bin, char **kp_str);
int function_nisec_fpdm_fphm_get_invs_kpstr(struct HandlePlate *h_plate, char  *fpdm, char *fphm, int dzsyh, char **inv_json, char **kp_bin, char **kp_str);
int function_cntax_fpdm_fphm_get_invs_kpstr(struct HandlePlate *h_plate, char  *fpdm, char *fphm, int dzsyh, char **inv_json, char **kp_bin, char **kp_str);

//
int function_aisino_query_invoice_month_all_data(struct HandlePlate *h_plate, char *month, char **data_json, char *errinfo);
int function_nisec_query_invoice_month_all_data(struct HandlePlate *h_plate, char *month, char **data_json, char *errinfo);
int function_cntax_query_invoice_month_all_data(struct HandlePlate *h_plate, char *month, char **data_json, char *errinfo);

//
int function_nisec_read_inv_coil_number_history(struct HandlePlate *h_plate, char **data_json, char *errinfo);
int function_cntax_read_inv_coil_number_history(struct HandlePlate *h_plate, char **data_json, char *errinfo);

//====//
//
int function_aisino_get_auth_passwd(struct HandlePlate *h_plate, struct _plate_infos *plate_info);
int function_nisec_get_auth_passwd(struct HandlePlate *h_plate, struct _plate_infos *plate_info);
int function_cntax_get_auth_passwd(struct HandlePlate *h_plate, struct _plate_infos *plate_info);
int function_mengbai_get_auth_passwd(struct HandlePlate *h_plate, struct _plate_infos *plate_info);
//
int function_aisino_verify_cert_passwd(struct HandlePlate *h_plate,char *passwd);
int function_nisec_verify_cert_passwd(struct HandlePlate *h_plate,char *passwd);
int function_cntax_verify_cert_passwd(struct HandlePlate *h_plate, char *passwd);
int function_mengbai_verify_cert_passwd(struct HandlePlate *h_plate, char *passwd);
//
int function_aisino_change_certpassword(struct HandlePlate *h_plate, char *old_passwd, char *new_passwd, char *errinfo);
int function_nisec_change_certpassword(struct HandlePlate *h_plate, char *old_passwd, char *new_passwd, char *errinfo);
int function_cntax_change_certpassword(struct HandlePlate *h_plate, char *old_passwd, char *new_passwd, char *errinfo);
int function_mengbai_change_certpassword(struct HandlePlate *h_plate, char *old_passwd, char *new_passwd, char *errinfo);
//
int function_aisino_get_current_invcode(struct HandlePlate *h_plate, unsigned char type, char* invnum, char* invcode);
int function_nisec_get_current_invcode(struct HandlePlate *h_plate,  unsigned char type, char* invnum, char* invcode);
int function_cntax_get_current_invcode(struct HandlePlate *h_plate, unsigned char type, char* invnum, char* invcode);
int function_mengbai_get_current_invcode(struct HandlePlate *h_plate,  unsigned char type, char* invnum, char* invcode);



//============//
//
int function_aisino_copy_report_data(struct HandlePlate *h_plate, uint8 inv_type, char *errinfo);
int function_nisec_copy_report_data(struct HandlePlate *h_plate, uint8 inv_type, char *errinfo);
int function_cntax_copy_report_data(struct HandlePlate *h_plate, uint8 inv_type, char *errinfo);
int function_mengbai_copy_report_data(struct HandlePlate *h_plate, uint8 inv_type, char *errinfo);
//
int function_aisino_report_summary_write_back(struct HandlePlate *h_plate, uint8 inv_type, char *errinfo);
int function_nisec_report_summary_write_back(struct HandlePlate *h_plate, uint8 inv_type, char *errinfo);
int function_cntax_report_summary_write_back(struct HandlePlate *h_plate, uint8 inv_type, char *errinfo);
int function_mengbai_report_summary_write_back(struct HandlePlate *h_plate, uint8 inv_type, char *errinfo);
//
int function_aisino_inv_upload_server(struct HandlePlate *h_plate, struct _offline_upload_result *offline_upload_result);
int function_nisec_inv_upload_server(struct HandlePlate *h_plate, struct _offline_upload_result *offline_upload_result);
int function_cntax_inv_upload_server(struct HandlePlate *h_plate, struct _offline_upload_result *offline_upload_result);
int function_mengbai_inv_upload_server(struct HandlePlate *h_plate, struct _offline_upload_result *offline_upload_result);
//
int function_aisino_fpdm_fphm_update_invs(struct HandlePlate *h_plate, char  *fpdm, char *fphm, int dzsyh, char **inv_json);
int function_nisec_fpdm_fphm_update_invs(struct HandlePlate *h_plate, char  *fpdm, char *fphm, int dzsyh, char **inv_json);
int function_cntax_fpdm_fphm_update_invs(struct HandlePlate *h_plate, char  *fpdm, char *fphm, int dzsyh, char **inv_json);
//
int function_aisino_test_server_connect(struct HandlePlate *h_plate, char **splxxx, char *errinfo);
int function_nisec_test_server_connect(struct HandlePlate *h_plate, char **splxxx, char *errinfo);
int function_cntax_test_server_connect(struct HandlePlate *h_plate, char **splxxx, char *errinfo);
int function_mengbai_test_server_connect(struct HandlePlate *h_plate, char **splxxx, char *errinfo);

//
int function_aisino_make_invoice(struct HandlePlate *h_plate, unsigned char inv_type, char* inv_data, struct _plate_infos *plate_infos, struct _make_invoice_result *make_invoice_result);
int function_nisec_make_invoice(struct HandlePlate *h_plate, unsigned char inv_type, char* inv_data, struct _plate_infos *plate_infos, struct _make_invoice_result *make_invoice_result);
int function_cntax_make_invoice(struct HandlePlate *h_plate, unsigned char inv_type, char* inv_data, struct _plate_infos *plate_infos, struct _make_invoice_result *make_invoice_result);
int function_mengbai_make_invoice(struct HandlePlate *h_plate, unsigned char inv_type, char *inv_info, struct _plate_infos *plate_infos, struct _make_invoice_result *make_invoice_result);
//
int function_aisino_waste_invoice(struct HandlePlate *h_plate, unsigned char inv_type, char  *fpdm, char *fphm, char *zfr, struct _cancel_invoice_result *cancel_invoice_result);
int function_nisec_waste_invoice(struct HandlePlate *h_plate, unsigned char inv_type, char  *fpdm, char *fphm, char *zfr, struct _cancel_invoice_result *cancel_invoice_result);
int function_cntax_waste_invoice(struct HandlePlate *h_plate, unsigned char inv_type, char  *fpdm, char *fphm, char *zfr, struct _cancel_invoice_result *cancel_invoice_result);
int function_mengbai_waste_invoice(struct HandlePlate *h_plate, unsigned char inv_type, char *fpdm, char *fphm, char *zfr, struct _cancel_invoice_result *cancel_invoice_result);
//
int function_aisino_waste_null_invoice(struct HandlePlate *h_plate, struct _plate_infos *plate_infos, struct _cancel_invoice_result *cancel_invoice_result);
int function_nisec_waste_null_invoice(struct HandlePlate *h_plate, struct _plate_infos *plate_infos, struct _cancel_invoice_result *cancel_invoice_result);
int function_cntax_waste_null_invoice(struct HandlePlate *h_plate, struct _plate_infos *plate_infos, struct _cancel_invoice_result *cancel_invoice_result);
int function_mengbai_waste_null_invoice(struct HandlePlate *h_plate, struct _plate_infos *plate_infos, struct _cancel_invoice_result *cancel_invoice_result);

//
int function_aisino_check_lzfpdm_lzfphm_allow(struct HandlePlate *h_plate, unsigned char inv_type, char *lzfpdm, char *lzfphm, uint32 dzsyh, char *errinfo);


//
int function_aisino_upload_hzxxb(struct HandlePlate *h_plate, unsigned char inv_type, char* inv_data, struct _plate_infos *plate_infos, struct _askfor_tzdbh_result *askfor_tzdbh_result);
int function_nisec_upload_hzxxb(struct HandlePlate *h_plate, unsigned char inv_type, char* inv_data, struct _plate_infos *plate_infos, struct _askfor_tzdbh_result *askfor_tzdbh_result);
int function_cntax_upload_hzxxb(struct HandlePlate *h_plate, unsigned char inv_type, char* inv_data, struct _plate_infos *plate_infos, struct _askfor_tzdbh_result *askfor_tzdbh_result);

//
int function_aisino_download_hzxxb_from_rednum(struct HandlePlate *h_plate, char *redNum, uint8 **inv_json, char *errinfo);
int function_nisec_download_hzxxb_from_rednum(struct HandlePlate *h_plate, char *redNum, uint8 **inv_json, char *errinfo);
int function_cntax_download_hzxxb_from_rednum(struct HandlePlate *h_plate, char *redNum, uint8 **inv_json, char *errinfo);

//
int function_aisino_download_hzxxb_from_date_range(struct HandlePlate *h_plate, char *date_range, uint8 **tzdbh_data, char *errinfo);
int function_aisino_download_hzxxb_from_date_range_new(struct HandlePlate *h_plate, char *date_range,char *gfsh, uint8 **tzdbh_data, char *errinfo);
int function_nisec_download_hzxxb_from_date_range(struct HandlePlate *h_plate, char *date_range, uint8 **tzdbh_data, char *errinfo);
int function_cntax_download_hzxxb_from_date_range(struct HandlePlate *h_plate, char *date_range, uint8 **tzdbh_data, char *errinfo);

//
int function_aisino_cancel_hzxxb(struct HandlePlate *h_plate, char *redNum, char *errinfo);
int function_nisec_cancel_hzxxb(struct HandlePlate *h_plate, char *redNum, char *errinfo);
int function_cntax_cancel_hzxxb(struct HandlePlate *h_plate, char *redNum, char *errinfo);

//////
//int function_aisino_get_download_net_invoice_coil(struct HandlePlate *h_plate, char *date_range, char *errinfo);


//
int function_aisino_query_net_invoice_coil(struct HandlePlate *h_plate, char *date_range, char *inv_data, char *errinfo);
int function_nisec_query_net_invoice_coil(struct HandlePlate *h_plate, char *date_range, char *inv_data, char *errinfo);
int function_cntax_query_net_invoice_coil(struct HandlePlate *h_plate, char *date_range, char *inv_data, char *errinfo);
//
int function_aisino_net_invoice_coil_download_unlock(struct HandlePlate *h_plate, char *date_range, uint8 fllxdm, char *fpdm, char *fpqshm, int fpzs, char *errinfo);
int function_nisec_net_invoice_coil_download_unlock(struct HandlePlate *h_plate, char *date_range, uint8 fllxdm, char *fpdm, char *fpqshm, int fpzs, char *errinfo);
int function_cntax_net_invoice_coil_download_unlock(struct HandlePlate *h_plate, char *date_range, uint8 fllxdm, char *fpdm, char *fpqshm, int fpzs, char *errinfo);
//
int function_aisino_connect_pubservice(struct HandlePlate *h_plate, uint8 inv_type, char *errinfo);
int function_nisec_connect_pubservice(struct HandlePlate *h_plate, uint8 inv_type, char *errinfo);
//
int function_aisino_disconnect_pubservice(struct HandlePlate *h_plate, uint8 inv_type, char *errinfo);
int function_nisec_disconnect_pubservice(struct HandlePlate *h_plate, uint8 inv_type, char *errinfo);

//--hello
int function_aisino_client_hello(struct HandlePlate *h_plate, char *client_hello, char *errinfo);
int function_nisec_client_hello(struct HandlePlate *h_plate, char *client_hello, char *errinfo);
int function_cntax_client_hello(struct HandlePlate *h_plate, char *client_hello, char *errinfo);

//--auth
int function_aisino_client_auth(struct HandlePlate *h_plate, char *server_hello, char *client_hello, char *errinfo);
int function_nisec_client_auth(struct HandlePlate *h_plate, char *server_hello, char *client_hello, char *errinfo);
int function_cntax_client_auth(struct HandlePlate *h_plate, char *server_hello, char *client_hello, char *errinfo);

//
int function_aisino_get_cert(struct HandlePlate *h_plate, char *cert_data, char *errinfo);
int function_nisec_get_cert(struct HandlePlate *h_plate, char *cert_data, char *errinfo);
int function_cntax_get_cert(struct HandlePlate *h_plate, char *cert_data, char *errinfo);

//key
int function_mengbai_server_key_info(struct HandlePlate *h_plate, char *info);


int ConsoleClose(int nDeviceType, HUSB hUsb);
//
int function_common_etax_login_Interface(char *busid,char *related_type,char *full_name,char *mobile,char *idcard,char *sz_password,char *errinfo,char *lpgoto,char *lpcode,char *lptoken,char *sys_type,char *svr_container,char *soft_cert_ip,int soft_cert_port,char *login_method,char *area_code);

//
//int function_common_etax_user_query_Interface(char *busid,char *errinfo,char **outinfo);
int function_common_etax_user_query_Interface(char *busid,char *errinfo,char **outinfo,char *svr_container,char *soft_cert_ip,int soft_cert_port,char *login_method,char *area_code);


//
//int function_common_etax_relationlist_Interface(char *busid,char *related_type,char *full_name,char *mobile,char *idcard,char *sz_password,char *errinfo,char **outenterpriselist);
int function_common_etax_relationlist_Interface(char *busid,char *related_type,char *full_name,char *mobile,char *idcard,char *sz_password,char *errinfo,char **outenterpriselist,char *svr_container,char *soft_cert_ip,int soft_cert_port,char *login_method,char *area_code);

//
//int function_common_etax_relationchange_Interface(char *busid,char *related_type,char *full_name,char *mobile,char *idcard,char *sz_password,char *errinfo,char *uniqueIdentity,char *realationStatus,char *lpgoto,char *lpcode,char *lptoken,char *sys_type);
int function_common_etax_relationchange_Interface(char *busid,char *related_type,char *full_name,char *mobile,char *idcard,char *sz_password,char *errinfo,char *uniqueIdentity,char *realationStatus,char *lpgoto,char *lpcode,char *lptoken,char *sys_type,char *svr_container,char *soft_cert_ip,int soft_cert_port,char *login_method,char *area_code);

//
int function_common_etax_invoice_query_Interface(char *busid,char *fpcxsj,char *related_type,char *full_name,char *mobile,char *idcard,char *sz_password,int realationtype,char *uniqueIdentity,char *realationStatus,char *errinfo,char **fpcxdata,char *area_code,char *etax_cookie,char *tpass_cookie,char *dppt_cookie,char *new_etax_cookie,char *new_tpass_cookie,char *new_dppt_cookie,int use_cookie_flag);

//
int function_common_etax_invoice_query_detail_Interface(char *busid,char *fpcxsj,char *related_type,char *full_name,char *mobile,char *idcard,char *sz_password,int realationtype,char *uniqueIdentity,char *realationStatus,char *errinfo,char **fpcxdata,char *area_code,char *etax_cookie,char *tpass_cookie,char *dppt_cookie,char *new_etax_cookie,char *new_tpass_cookie,char *new_dppt_cookie,int use_cookie_flag);

//
int function_common_etax_invoice_query_down_Interface(char *busid,char *fpcxsj,char *related_type,char *full_name,char *mobile,char *idcard,char *sz_password,int realationtype,char *uniqueIdentity,char *realationStatus,char *errinfo,char **fpcxdata,char **fpcxname,char *area_code,char *etax_cookie,char *tpass_cookie,char *dppt_cookie,char *new_etax_cookie,char *new_tpass_cookie,char *new_dppt_cookie,int use_cookie_flag);

//RPA
int function_common_etax_rpa_invoice_issuance_Interface(char *busid,char *fpkjurl,char *nsrsbh,char *related_type,char *mobile,char *idcard,char *sz_password,int realationtype,char *uniqueIdentity,char *realationStatus,char *errinfo,char **fpkjdata,char *area_code,char *etax_cookie,char *tpass_cookie,char *dppt_cookie,char *new_etax_cookie,char *new_tpass_cookie,char *new_dppt_cookie,int use_cookie_flag);
int function_common_etax_rpa_invoice_issuance_status_Interface(char *busid,char *rzid, char *app_code,char *fpkjurl,char *nsrsbh,char *related_type,char *mobile,char *idcard,char *sz_password,int realationtype,char *uniqueIdentity,char *realationStatus,char *errinfo,char **fpkjdata,char *area_code,char *etax_cookie,char *tpass_cookie,char *dppt_cookie,char *new_etax_cookie,char *new_tpass_cookie,char *new_dppt_cookie,int use_cookie_flag);

//
int function_common_credit_limit_Interface(char *busid,char *nsrsbh,char *related_type,char *mobile,char *idcard,char *sz_password,int realationtype,char *uniqueIdentity,char *realationStatus,char *errinfo,char **sxeddata,char *area_code,char *etax_cookie,char *tpass_cookie,char *dppt_cookie,char *new_etax_cookie,char *new_tpass_cookie,char *new_dppt_cookie,int use_cookie_flag);

//
int function_common_etax_blue_ink_invoice_issuance_Interface(char *busid,char *fpkjjson,char *related_type,char *mobile,char *idcard,char *sz_password,int realationtype,char *uniqueIdentity,char *realationStatus,char *errinfo,char **fpkjout,char **hqnsrjcxx,char *area_code,char *etax_cookie,char *tpass_cookie,char *dppt_cookie,char *new_etax_cookie,char *new_tpass_cookie,char *new_dppt_cookie,int use_cookie_flag);

int function_common_hcstaquery_invoice_issuance_Interface(char *busid,char *fpcxsj,char *related_type,char *mobile,char *idcard,char *sz_password,int realationtype,char *uniqueIdentity,char *realationStatus,char *errinfo,char **fpcxdata,char *area_code,char *etax_cookie,char *tpass_cookie,char *dppt_cookie,char *new_etax_cookie,char *new_tpass_cookie,char *new_dppt_cookie,int use_cookie_flag);

int function_common_etax_hzqrxxSzzhQuery_Interface(char *busid,char *fpcxsj,char *related_type,char *full_name,char *mobile,char *idcard,char *sz_password,int realationtype,char *uniqueIdentity,char *realationStatus,char *errinfo,char **fpcxdata,char *area_code,char *etax_cookie,char *tpass_cookie,char *dppt_cookie,char *new_etax_cookie,char *new_tpass_cookie,char *new_dppt_cookie,int use_cookie_flag);

int function_common_etax_hzqrxxSave_Interface(char *busid,char *fpcxsj,char *related_type,char *full_name,char *mobile,char *idcard,char *sz_password,int realationtype,char *uniqueIdentity,char *realationStatus,char *errinfo,char **fpcxdata,char *area_code,char *etax_cookie,char *tpass_cookie,char *dppt_cookie,char *new_etax_cookie,char *new_tpass_cookie,char *new_dppt_cookie,int use_cookie_flag);

int function_common_etax_hzqrxxQuery_Interface(char *busid,char *fpcxsj,char *related_type,char *full_name,char *mobile,char *idcard,char *sz_password,int realationtype,char *uniqueIdentity,char *realationStatus,char *errinfo,char **fpcxdata,char *area_code,char *etax_cookie,char *tpass_cookie,char *dppt_cookie,char *new_etax_cookie,char *new_tpass_cookie,char *new_dppt_cookie,int use_cookie_flag);

int function_common_etax_hzqrxxQueryDetail_Interface(char *busid,char *fpcxsj,char *related_type,char *full_name,char *mobile,char *idcard,char *sz_password,int realationtype,char *uniqueIdentity,char *realationStatus,char *errinfo,char **fpcxdata,char *area_code,char *etax_cookie,char *tpass_cookie,char *dppt_cookie,char *new_etax_cookie,char *new_tpass_cookie,char *new_dppt_cookie,int use_cookie_flag);

int function_common_etax_invoice_query_by_cookie_Interface(char *area_code,char *etax_cookie,char *tpass_cookie,char *dppt_cookie,char *new_etax_cookie,char *new_tpass_cookie,char *new_dppt_cookie,char *fpcxsj,char **fpcxdata,char *errinfo);

int function_common_etax_invoice_query_detail_by_cookie_Interface(char *area_code,char *etax_cookie,char *tpass_cookie,char *dppt_cookie,char *new_etax_cookie,char *new_tpass_cookie,char *new_dppt_cookie,char *fpcxsj,char **fpcxdata,char *errinfo);

int function_common_etax_get_cookie_Interface(char *busid,char *related_type,char *mobile,char *idcard,char *sz_password,int realationtype,char *uniqueIdentity,char *realationStatus,char *errinfo,char **etax_cookie,char **tpass_cookie,char **dppt_cookie,char *svr_container,char *soft_cert_ip,int soft_cert_port,char *login_method,char *area_code,char *temp_data);

int function_common_fjxxpeizhi_Interface(char *busid,char *fpcxsj,char *mode,char *related_type,char *mobile,char *idcard,char *sz_password,int realationtype,char *uniqueIdentity,char *realationStatus,char *errinfo,char **fpcxdata,char *area_code,char *etax_cookie,char *tpass_cookie,char *dppt_cookie,char *new_etax_cookie,char *new_tpass_cookie,char *new_dppt_cookie,int use_cookie_flag);

int function_common_cjmbpeizhi_Interface(char *busid,char *fpcxsj,char *mode,char *related_type,char *mobile,char *idcard,char *sz_password,int realationtype,char *uniqueIdentity,char *realationStatus,char *errinfo,char **fpcxdata,char *area_code,char *etax_cookie,char *tpass_cookie,char *dppt_cookie,char *new_etax_cookie,char *new_tpass_cookie,char *new_dppt_cookie,int use_cookie_flag);

int function_common_etax_get_nsrjcxx_Interface(char *busid,char *related_type,char *mobile,char *idcard,char *sz_password,int realationtype,char *uniqueIdentity,char *realationStatus,char *errinfo,char **fpcxdata,char *area_code,char *etax_cookie,char *tpass_cookie,char *dppt_cookie,int use_cookie_flag);

int function_common_etax_get_nsrfxxx_Interface(char *busid,char *related_type,char *mobile,char *idcard,char *sz_password,int realationtype,char *uniqueIdentity,char *realationStatus,char *errinfo,char **fpcxdata,char *area_code,char *etax_cookie,char *tpass_cookie,char *dppt_cookie,int use_cookie_flag);


#endif