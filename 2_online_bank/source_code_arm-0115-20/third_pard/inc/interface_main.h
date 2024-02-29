#ifndef interface_main_h
#define interface_main_h
#include "aisino/aisino_call.h"
#include "nisec/nisec_call.h"
#include "mengbai/mengbai_call.h"
#include "cntax/cntax_call.h"
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
//==通用接口==//
//初始化动态库
int function_common_init_tax_lib(uint8 *key_zlib_json, char *taxlib_version);
//重载授权文件
int function_common_reload_tax_auth(uint8 *key_zlib_json);
//判断已识别税盘的授权信息
int function_common_jude_plate_auth(uint8 bDeviceType, char *busid,char *errinfo);
//初始化动态库内部日志
int function_common_init_tax_lib_log_hook(void *function, void *arg);
//获取票种信息对应结构体内存号
int function_common_find_inv_type_mem(uint8 type, int *mem_i);
//获取授权文件信息
int function_common_get_cert_json_file(uint8 *key_zlib_json, char **cert_json);
//严重错误信息上报运维后台
int function_common_report_event(char *plate_num, char *err, char *errinfo, int result);
//上层预解析发票数据
int function_common_analyze_json_buff(uint8 bDeviceType, char *json_data, HFPXX stp_fpxx, char *errinfo);


//通用接口，获取USB句柄及DEV句柄
int function_common_get_basic_simple(struct HandlePlate *h_plate, struct _plate_infos *plate_info);

//通用接口，读取基础信息、税务信息及发票卷信息，支持金税盘、税控盘、模拟盘、税务ukey、航信勾选key、百旺勾选key
int function_common_get_basic_tax_info(struct HandlePlate *h_plate, struct _plate_infos *plate_info);

//关闭USB设备句柄
int function_common_close_usb_device(struct HandlePlate *h_plate);

//==不需要授权且不需要口令验证通过亦可调用的接口==//
//获取税盘当前时间
int function_aisino_get_tax_time(struct HandlePlate *h_plate, char *outtime);
int function_nisec_get_tax_time(struct HandlePlate *h_plate, char *outtime);
int function_cntax_get_tax_time(struct HandlePlate *h_plate, char* outtime);
int function_mengbai_get_plate_time(struct HandlePlate *h_plate, char *outtime);
//mqtt接口获取发票明细
int function_aisino_get_inv_details_mqtt(struct HandlePlate *h_plate, unsigned char *month, void *function, void *arg, char *errinfo, int old_new, unsigned short *start_stop, int sfxqqd);
int function_nisec_get_inv_details_mqtt(struct HandlePlate *h_plate, unsigned char *month, void *function, void *arg, char *errinfo, int old_new, unsigned short *start_stop, int sfxqqd);
int function_cntax_get_inv_details_mqtt(struct HandlePlate *h_plate, unsigned char *month, void *function, void *arg, char *errinfo, int old_new, unsigned short *start_stop, int sfxqqd);
int function_mengbai_get_inv_details_mqtt(struct HandlePlate *h_plate, unsigned char *month, void *function, void *arg, char *errinfo, int old_new, unsigned short *start_stop, int sfxqqd);
//发票上传M服务
int function_aisino_upload_m_server(struct HandlePlate *h_plate, unsigned short *start_stop, struct _upload_inv_num *upload_inv_num, void *function, void *arg, struct _inv_sum_data *now_month_sum);
int function_nisec_upload_m_server(struct HandlePlate *h_plate, unsigned short *start_stop, struct _upload_inv_num *upload_inv_num, void *function, void *arg, struct _inv_sum_data *now_month_sum);
int function_cntax_upload_m_server(struct HandlePlate *h_plate, unsigned short *start_stop, struct _upload_inv_num *upload_inv_num, void *function, void *arg, struct _inv_sum_data *now_month_sum);
int function_mengbai_upload_m_server(struct HandlePlate *h_plate, unsigned short *start_stop, struct _upload_inv_num *upload_inv_num, void *function, void *arg, struct _inv_sum_data *now_month_sum);
//获取月发票总数
int function_aisino_get_month_invoice_num(struct HandlePlate *h_plate, unsigned char *month, unsigned int  *count, unsigned long *size);
int function_nisec_get_month_invoice_num(struct HandlePlate *h_plate, unsigned char *month, unsigned int  *count, unsigned long *size);
int function_cntax_get_month_invoice_num(struct HandlePlate *h_plate, unsigned char *month, unsigned int  *count, unsigned long *size);
int function_mengbai_get_month_invoice_num(struct HandlePlate *h_plate, unsigned char *month, unsigned int  *count, unsigned long *size);
//按发票代码号码获取发票信息
int function_aisino_fpdm_fphm_get_invs(struct HandlePlate *h_plate, char  *fpdm, char *fphm, int dzsyh, char **inv_json);
int function_nisec_fpdm_fphm_get_invs(struct HandlePlate *h_plate, char  *fpdm, char *fphm, int dzsyh, char **inv_json);
int function_cntax_fpdm_fphm_get_invs(struct HandlePlate *h_plate, char  *fpdm, char *fphm, int dzsyh, char **inv_json);

//按发票代码号码获取发票二进制文件信息
int function_aisino_fpdm_fphm_get_invs_kpstr(struct HandlePlate *h_plate, char  *fpdm, char *fphm, int dzsyh, char **inv_json, char **kp_bin, char **kp_str);
int function_nisec_fpdm_fphm_get_invs_kpstr(struct HandlePlate *h_plate, char  *fpdm, char *fphm, int dzsyh, char **inv_json, char **kp_bin, char **kp_str);
int function_cntax_fpdm_fphm_get_invs_kpstr(struct HandlePlate *h_plate, char  *fpdm, char *fphm, int dzsyh, char **inv_json, char **kp_bin, char **kp_str);

//按月获取税盘汇总信息接口
int function_aisino_query_invoice_month_all_data(struct HandlePlate *h_plate, char *month, char **data_json, char *errinfo);
int function_nisec_query_invoice_month_all_data(struct HandlePlate *h_plate, char *month, char **data_json, char *errinfo);
int function_cntax_query_invoice_month_all_data(struct HandlePlate *h_plate, char *month, char **data_json, char *errinfo);

//读历史票卷信息
int function_nisec_read_inv_coil_number_history(struct HandlePlate *h_plate, char **data_json, char *errinfo);
int function_cntax_read_inv_coil_number_history(struct HandlePlate *h_plate, char **data_json, char *errinfo);

//==需要授权未验证口令或口令错误可调用的接口==//
//程序主动验证证书口令
int function_aisino_get_auth_passwd(struct HandlePlate *h_plate, struct _plate_infos *plate_info);
int function_nisec_get_auth_passwd(struct HandlePlate *h_plate, struct _plate_infos *plate_info);
int function_cntax_get_auth_passwd(struct HandlePlate *h_plate, struct _plate_infos *plate_info);
int function_mengbai_get_auth_passwd(struct HandlePlate *h_plate, struct _plate_infos *plate_info);
//接口调用验证默认证书口令是否正确
int function_aisino_verify_cert_passwd(struct HandlePlate *h_plate,char *passwd);
int function_nisec_verify_cert_passwd(struct HandlePlate *h_plate,char *passwd);
int function_cntax_verify_cert_passwd(struct HandlePlate *h_plate, char *passwd);
int function_mengbai_verify_cert_passwd(struct HandlePlate *h_plate, char *passwd);
//修改证书口令
int function_aisino_change_certpassword(struct HandlePlate *h_plate, char *old_passwd, char *new_passwd, char *errinfo);
int function_nisec_change_certpassword(struct HandlePlate *h_plate, char *old_passwd, char *new_passwd, char *errinfo);
int function_cntax_change_certpassword(struct HandlePlate *h_plate, char *old_passwd, char *new_passwd, char *errinfo);
int function_mengbai_change_certpassword(struct HandlePlate *h_plate, char *old_passwd, char *new_passwd, char *errinfo);
//获取当前发票代码号码
int function_aisino_get_current_invcode(struct HandlePlate *h_plate, unsigned char type, char* invnum, char* invcode);
int function_nisec_get_current_invcode(struct HandlePlate *h_plate,  unsigned char type, char* invnum, char* invcode);
int function_cntax_get_current_invcode(struct HandlePlate *h_plate, unsigned char type, char* invnum, char* invcode);
int function_mengbai_get_current_invcode(struct HandlePlate *h_plate,  unsigned char type, char* invnum, char* invcode);



//======需要授权且验证口令正确后方可操作的接口======//
//抄税汇总???
int function_aisino_copy_report_data(struct HandlePlate *h_plate, uint8 inv_type, char *errinfo);
int function_nisec_copy_report_data(struct HandlePlate *h_plate, uint8 inv_type, char *errinfo);
int function_cntax_copy_report_data(struct HandlePlate *h_plate, uint8 inv_type, char *errinfo);
int function_mengbai_copy_report_data(struct HandlePlate *h_plate, uint8 inv_type, char *errinfo);
//汇总清卡???
int function_aisino_report_summary_write_back(struct HandlePlate *h_plate, uint8 inv_type, char *errinfo);
int function_nisec_report_summary_write_back(struct HandlePlate *h_plate, uint8 inv_type, char *errinfo);
int function_cntax_report_summary_write_back(struct HandlePlate *h_plate, uint8 inv_type, char *errinfo);
int function_mengbai_report_summary_write_back(struct HandlePlate *h_plate, uint8 inv_type, char *errinfo);
//离线发票上传???
int function_aisino_inv_upload_server(struct HandlePlate *h_plate, struct _offline_upload_result *offline_upload_result);
int function_nisec_inv_upload_server(struct HandlePlate *h_plate, struct _offline_upload_result *offline_upload_result);
int function_cntax_inv_upload_server(struct HandlePlate *h_plate, struct _offline_upload_result *offline_upload_result);
int function_mengbai_inv_upload_server(struct HandlePlate *h_plate, struct _offline_upload_result *offline_upload_result);
//按发票代码号码上传离线发票???
int function_aisino_fpdm_fphm_update_invs(struct HandlePlate *h_plate, char  *fpdm, char *fphm, int dzsyh, char **inv_json);
int function_nisec_fpdm_fphm_update_invs(struct HandlePlate *h_plate, char  *fpdm, char *fphm, int dzsyh, char **inv_json);
int function_cntax_fpdm_fphm_update_invs(struct HandlePlate *h_plate, char  *fpdm, char *fphm, int dzsyh, char **inv_json);
//测试税盘与服务器连接
int function_aisino_test_server_connect(struct HandlePlate *h_plate, char **splxxx, char *errinfo);
int function_nisec_test_server_connect(struct HandlePlate *h_plate, char **splxxx, char *errinfo);
int function_cntax_test_server_connect(struct HandlePlate *h_plate, char **splxxx, char *errinfo);
int function_mengbai_test_server_connect(struct HandlePlate *h_plate, char **splxxx, char *errinfo);

//正数、负数发票开具
int function_aisino_make_invoice(struct HandlePlate *h_plate, unsigned char inv_type, char* inv_data, struct _plate_infos *plate_infos, struct _make_invoice_result *make_invoice_result);
int function_nisec_make_invoice(struct HandlePlate *h_plate, unsigned char inv_type, char* inv_data, struct _plate_infos *plate_infos, struct _make_invoice_result *make_invoice_result);
int function_cntax_make_invoice(struct HandlePlate *h_plate, unsigned char inv_type, char* inv_data, struct _plate_infos *plate_infos, struct _make_invoice_result *make_invoice_result);
int function_mengbai_make_invoice(struct HandlePlate *h_plate, unsigned char inv_type, char *inv_info, struct _plate_infos *plate_infos, struct _make_invoice_result *make_invoice_result);
//已开发票作废
int function_aisino_waste_invoice(struct HandlePlate *h_plate, unsigned char inv_type, char  *fpdm, char *fphm, char *zfr, struct _cancel_invoice_result *cancel_invoice_result);
int function_nisec_waste_invoice(struct HandlePlate *h_plate, unsigned char inv_type, char  *fpdm, char *fphm, char *zfr, struct _cancel_invoice_result *cancel_invoice_result);
int function_cntax_waste_invoice(struct HandlePlate *h_plate, unsigned char inv_type, char  *fpdm, char *fphm, char *zfr, struct _cancel_invoice_result *cancel_invoice_result);
int function_mengbai_waste_invoice(struct HandlePlate *h_plate, unsigned char inv_type, char *fpdm, char *fphm, char *zfr, struct _cancel_invoice_result *cancel_invoice_result);
//空白作废
int function_aisino_waste_null_invoice(struct HandlePlate *h_plate, struct _plate_infos *plate_infos, struct _cancel_invoice_result *cancel_invoice_result);
int function_nisec_waste_null_invoice(struct HandlePlate *h_plate, struct _plate_infos *plate_infos, struct _cancel_invoice_result *cancel_invoice_result);
int function_cntax_waste_null_invoice(struct HandlePlate *h_plate, struct _plate_infos *plate_infos, struct _cancel_invoice_result *cancel_invoice_result);
int function_mengbai_waste_null_invoice(struct HandlePlate *h_plate, struct _plate_infos *plate_infos, struct _cancel_invoice_result *cancel_invoice_result);

//检查蓝字发票代码号码是否可以开具红票???
int function_aisino_check_lzfpdm_lzfphm_allow(struct HandlePlate *h_plate, unsigned char inv_type, char *lzfpdm, char *lzfphm, uint32 dzsyh, char *errinfo);


//红字信息表申请
int function_aisino_upload_hzxxb(struct HandlePlate *h_plate, unsigned char inv_type, char* inv_data, struct _plate_infos *plate_infos, struct _askfor_tzdbh_result *askfor_tzdbh_result);
int function_nisec_upload_hzxxb(struct HandlePlate *h_plate, unsigned char inv_type, char* inv_data, struct _plate_infos *plate_infos, struct _askfor_tzdbh_result *askfor_tzdbh_result);
int function_cntax_upload_hzxxb(struct HandlePlate *h_plate, unsigned char inv_type, char* inv_data, struct _plate_infos *plate_infos, struct _askfor_tzdbh_result *askfor_tzdbh_result);

//通过红字信息表编号查询申请信息
int function_aisino_download_hzxxb_from_rednum(struct HandlePlate *h_plate, char *redNum, uint8 **inv_json, char *errinfo);
int function_nisec_download_hzxxb_from_rednum(struct HandlePlate *h_plate, char *redNum, uint8 **inv_json, char *errinfo);
int function_cntax_download_hzxxb_from_rednum(struct HandlePlate *h_plate, char *redNum, uint8 **inv_json, char *errinfo);

//通过时间段查询红字信息表申请信息
int function_aisino_download_hzxxb_from_date_range(struct HandlePlate *h_plate, char *date_range, uint8 **tzdbh_data, char *errinfo);
int function_nisec_download_hzxxb_from_date_range(struct HandlePlate *h_plate, char *date_range, uint8 **tzdbh_data, char *errinfo);
int function_cntax_download_hzxxb_from_date_range(struct HandlePlate *h_plate, char *date_range, uint8 **tzdbh_data, char *errinfo);

//撤销红字信息表申请
int function_aisino_cancel_hzxxb(struct HandlePlate *h_plate, char *redNum, char *errinfo);
int function_nisec_cancel_hzxxb(struct HandlePlate *h_plate, char *redNum, char *errinfo);
int function_cntax_cancel_hzxxb(struct HandlePlate *h_plate, char *redNum, char *errinfo);

////在线发票一键领用//接口停用
//int function_aisino_get_download_net_invoice_coil(struct HandlePlate *h_plate, char *date_range, char *errinfo);


//在线发票领用查询
int function_aisino_query_net_invoice_coil(struct HandlePlate *h_plate, char *date_range, char *inv_data, char *errinfo);
int function_nisec_query_net_invoice_coil(struct HandlePlate *h_plate, char *date_range, char *inv_data, char *errinfo);
int function_cntax_query_net_invoice_coil(struct HandlePlate *h_plate, char *date_range, char *inv_data, char *errinfo);
//在线发票领用
int function_aisino_net_invoice_coil_download_unlock(struct HandlePlate *h_plate, char *date_range, uint8 fllxdm, char *fpdm, char *fpqshm, int fpzs, char *errinfo);
int function_nisec_net_invoice_coil_download_unlock(struct HandlePlate *h_plate, char *date_range, uint8 fllxdm, char *fpdm, char *fpqshm, int fpzs, char *errinfo);
int function_cntax_net_invoice_coil_download_unlock(struct HandlePlate *h_plate, char *date_range, uint8 fllxdm, char *fpdm, char *fpqshm, int fpzs, char *errinfo);
//申请接入公共服务平台
int function_aisino_connect_pubservice(struct HandlePlate *h_plate, uint8 inv_type, char *errinfo);
int function_nisec_connect_pubservice(struct HandlePlate *h_plate, uint8 inv_type, char *errinfo);
//撤销接入公共服务平台
int function_aisino_disconnect_pubservice(struct HandlePlate *h_plate, uint8 inv_type, char *errinfo);
int function_nisec_disconnect_pubservice(struct HandlePlate *h_plate, uint8 inv_type, char *errinfo);

//服务器连接握手--hello
int function_aisino_client_hello(struct HandlePlate *h_plate, char *client_hello, char *errinfo);
int function_nisec_client_hello(struct HandlePlate *h_plate, char *client_hello, char *errinfo);
int function_cntax_client_hello(struct HandlePlate *h_plate, char *client_hello, char *errinfo);

//服务器连接认证--auth
int function_aisino_client_auth(struct HandlePlate *h_plate, char *server_hello, char *client_hello, char *errinfo);
int function_nisec_client_auth(struct HandlePlate *h_plate, char *server_hello, char *client_hello, char *errinfo);
int function_cntax_client_auth(struct HandlePlate *h_plate, char *server_hello, char *client_hello, char *errinfo);

//获取税盘证书数据
int function_aisino_get_cert(struct HandlePlate *h_plate, char *cert_data, char *errinfo);
int function_nisec_get_cert(struct HandlePlate *h_plate, char *cert_data, char *errinfo);
int function_cntax_get_cert(struct HandlePlate *h_plate, char *cert_data, char *errinfo);

//获取蒙柏服务商key加密信息
int function_mengbai_server_key_info(struct HandlePlate *h_plate, char *info);


#endif