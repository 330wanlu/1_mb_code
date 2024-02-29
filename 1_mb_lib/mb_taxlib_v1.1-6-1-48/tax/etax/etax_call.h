#ifndef ETAX_CALL_H
#define ETAX_CALL_H

#include "etax_common.h"
#include "../common/function.h"

int my_etax_confirm_query(HDEV hDev, ETaxFlag flag);
void my_etax_auth_server(HDEV hDev);
int my_etax_query_invoice(HDEV hDev, ETaxFlag flag);

/********************************全电接口***********************************/
//一键登陆
int etax_login_Interface(char *busid,char *related_type,char *full_name,char *mobile,char *idcard,char *sz_password,char *errinfo,char *lpgoto,char *lpcode,char *lptoken,char *sys_type,char *svr_container,char *soft_cert_ip,int soft_cert_port,char *login_method,char *area_code);
//用户列表查询
//int etax_user_query_Interface(char *busid,char *errinfo,char **outinfo);
int etax_user_query_Interface(char *busid,char *errinfo,char **outinfo,char *svr_container,char *soft_cert_ip,int soft_cert_port,char *login_method,char *area_code);

//企业列表查询
//int etax_relationlist_Interface(char *busid,char *related_type,char *full_name,char *mobile,char *idcard,char *sz_password,char *errinfo,char **outenterpriselist);
int etax_relationlist_Interface(char *busid,char *related_type,char *full_name,char *mobile,char *idcard,char *sz_password,char *errinfo,char **outenterpriselist,char *svr_container,char *soft_cert_ip,int soft_cert_port,char *login_method,char *area_code);

//企业切换
//int etax_relationchange_Interface(char *busid,char *related_type,char *full_name,char *mobile,char *idcard,char *sz_password,char *errinfo,char *uniqueIdentity,char *realationStatus,char *lpgoto,char *lpcode,char *lptoken,char *sys_type);
int etax_relationchange_Interface(char *busid,char *related_type,char *full_name,char *mobile,char *idcard,char *sz_password,char *errinfo,char *uniqueIdentity,char *realationStatus,char *lpgoto,char *lpcode,char *lptoken,char *sys_type,char *svr_container,char *soft_cert_ip,int soft_cert_port,char *login_method,char *area_code);

//发票列表查询
int etax_invoice_query_Interface(char *busid,char *fpcxsj,char *related_type,char *full_name,char *mobile,char *idcard,char *sz_password,int realationtype,char *uniqueIdentity,char *realationStatus,char *errinfo,char **fpcxdata);
//发票详情查询
int etax_invoice_query_detail_Interface(char *busid,char *fpcxsj,char *related_type,char *full_name,char *mobile,char *idcard,char *sz_password,int realationtype,char *uniqueIdentity,char *realationStatus,char *errinfo,char **fpcxdata);
//发票下载查询
int etax_invoice_query_down_Interface(char *busid,char *fpcxsj,char *related_type,char *full_name,char *mobile,char *idcard,char *sz_password,int realationtype,char *uniqueIdentity,char *realationStatus,char *errinfo,char **fpcxdata,char **fpcxname);
//发票开具RPA扫码接口
int etax_rpa_invoice_issuance_Interface(char *busid,char *fpkjurl,char *nsrsbh,char *related_type,char *mobile,char *idcard,char *sz_password,int realationtype,char *uniqueIdentity,char *realationStatus,char *errinfo,char **fpkjdata);
int etax_rpa_invoice_issuance_status_Interface(char *busid,char *rzid,char *app_code, char *fpkjurl,char *nsrsbh,char *related_type,char *mobile,char *idcard,char *sz_password,int realationtype,char *uniqueIdentity,char *realationStatus,char *errinfo,char **fpkjdata);

int etax_invoice_rpa_invoice_issuance_status_by_cookie_Interface(char *area_code,char *rzid,char *app_code,char *etax_cookie,char *tpass_cookie,char *dppt_cookie,char *new_etax_cookie,char *new_tpass_cookie,char *new_dppt_cookie,char *errinfo,char *fpkjurl,char *nsrsbh,char **fpkjdata);


//授信额度
int etax_credit_limit_Interface(char *busid,char *nsrsbh,char *related_type,char *mobile,char *idcard,char *sz_password,int realationtype,char *uniqueIdentity,char *realationStatus,char *errinfo,char **sxeddata);
//蓝字发票开具
int etax_blue_ink_invoice_issuance_Interface(char *busid,char *fpkjjson,char *related_type,char *mobile,char *idcard,char *sz_password,int realationtype,char *uniqueIdentity,char *realationStatus,char *errinfo,char **fpkjdata,char **hqnsrjcxx);
//可开红票的蓝票查询
int etax_hzqrxxSzzhQuery_Interface(char *busid,char *fpcxsj,char *related_type,char *full_name,char *mobile,char *idcard,char *sz_password,int realationtype,char *uniqueIdentity,char *realationStatus,char *errinfo,char **fpcxdata);
//红字确认单开具
int etax_hzqrxxSave_Interface(char *busid,char *fpcxsj,char *related_type,char *full_name,char *mobile,char *idcard,char *sz_password,int realationtype,char *uniqueIdentity,char *realationStatus,char *errinfo,char **fpcxdata);
//红字确认单查询
int etax_hzqrxxQuery_Interface(char *busid,char *fpcxsj,char *related_type,char *full_name,char *mobile,char *idcard,char *sz_password,int realationtype,char *uniqueIdentity,char *realationStatus,char *errinfo,char **fpcxdata);
//红字确认单详情查询
int etax_hzqrxxQueryDetail_Interface(char *busid,char *fpcxsj,char *related_type,char *full_name,char *mobile,char *idcard,char *sz_password,int realationtype,char *uniqueIdentity,char *realationStatus,char *errinfo,char **fpcxdata);
//发票红字类查询
int etax_hcstaquery_invoice_issuance_Interface(char *busid,char *fpcxsj,char *related_type,char *mobile,char *idcard,char *sz_password,int realationtype,char *uniqueIdentity,char *realationStatus,char *errinfo,char **fpcxdata);
int etax_cjmbpeizhi_Interface(char *busid,char *fpcxsj,char *mode,char *related_type,char *mobile,char *idcard,char *sz_password,int realationtype,char *uniqueIdentity,char *realationStatus,char *errinfo,char **fpcxdata);
int etax_fjxxpeizhi_Interface(char *busid,char *fpcxsj,char *mode,char *related_type,char *mobile,char *idcard,char *sz_password,int realationtype,char *uniqueIdentity,char *realationStatus,char *errinfo,char **fpcxdata);
int etax_get_cookie_Interface(char *busid,char *related_type,char *mobile,char *idcard,char *sz_password,int realationtype,char *uniqueIdentity,char *realationStatus,char *errinfo,char **etax_cookie,char **tpass_cookie,char **dppt_cookie,char *svr_container,char *soft_cert_ip,int soft_cert_port,char *login_method,char *area_code,char *temp_data);
int etax_invoice_query_by_cookie_Interface(char *area_code,char *etax_cookie,char *tpass_cookie,char *dppt_cookie,char *new_etax_cookie,char *new_tpass_cookie,char *new_dppt_cookie,char *fpcxsj,char **fpcxdata,char *errinfo);
int etax_invoice_query_detail_by_cookie_Interface(char *area_code,char *etax_cookie,char *tpass_cookie,char *dppt_cookie,char *new_etax_cookie,char *new_tpass_cookie,char *new_dppt_cookie,char *fpcxsj,char **fpcxdata,char *errinfo);
int etax_invoice_query_down_by_cookie_Interface(char *area_code,char *etax_cookie,char *tpass_cookie,char *dppt_cookie,char *new_etax_cookie,char *new_tpass_cookie,char *new_dppt_cookie,char *fpcxsj,char *errinfo,char **fpcxdata,char **fpcxname);
int etax_invoice_rpa_invoice_issuance_by_cookie_Interface(char *area_code,char *etax_cookie,char *tpass_cookie,char *dppt_cookie,char *new_etax_cookie,char *new_tpass_cookie,char *new_dppt_cookie,char *errinfo,char *fpkjurl,char *nsrsbh,char **fpkjdata);


int etax_credit_limit_by_cookie_Interface(char *area_code,char *etax_cookie,char *tpass_cookie,char *dppt_cookie,char *new_etax_cookie,char *new_tpass_cookie,char *new_dppt_cookie,char *errinfo,char *nsrsbh,char **sxeddata);
int etax_hcstaquery_invoice_issuance_by_cookie_Interface(char *area_code,char *etax_cookie,char *tpass_cookie,char *dppt_cookie,char *new_etax_cookie,char *new_tpass_cookie,char *new_dppt_cookie,char *errinfo,char *fpcxsj,char **fpcxdata);
int etax_blue_ink_invoice_issuance_by_cookie_Interface(char *area_code,char *etax_cookie,char *tpass_cookie,char *dppt_cookie,char *new_etax_cookie,char *new_tpass_cookie,char *new_dppt_cookie,char *errinfo,char *fpkjjson,char **fpkjout,char **hqnsrjcxx);
int etax_hzqrxxSzzhQuery_by_cookies_Interface(char *area_code,char *etax_cookie,char *tpass_cookie,char *dppt_cookie,char *new_etax_cookie,char *new_tpass_cookie,char *new_dppt_cookie,char *fpcxsj,char **fpcxdata,char *errinfo);
int etax_hzqrxxSave_by_cookies_Interface(char *area_code,char *etax_cookie,char *tpass_cookie,char *dppt_cookie,char *new_etax_cookie,char *new_tpass_cookie,char *new_dppt_cookie,char *fpcxsj,char **fpcxdata,char *errinfo);
int etax_hzqrxxQuery_by_cookies_Interface(char *area_code,char *etax_cookie,char *tpass_cookie,char *dppt_cookie,char *new_etax_cookie,char *new_tpass_cookie,char *new_dppt_cookie,char *fpcxsj,char **fpcxdata,char *errinfo);
int etax_hzqrxxQueryDetail_by_cookies_Interface(char *area_code,char *etax_cookie,char *tpass_cookie,char *dppt_cookie,char *new_etax_cookie,char *new_tpass_cookie,char *new_dppt_cookie,char *fpcxsj,char **fpcxdata,char *errinfo);
int etax_fjxxpeizhi_by_cookie_Interface(char *area_code,char *etax_cookie,char *tpass_cookie,char *dppt_cookie,char *new_etax_cookie,char *new_tpass_cookie,char *new_dppt_cookie,char *errinfo,char *fpcxsj,char *mode,char **fpcxdata);
int etax_cjmbpeizhi_by_cookie_Interface(char *area_code,char *etax_cookie,char *tpass_cookie,char *dppt_cookie,char *new_etax_cookie,char *new_tpass_cookie,char *new_dppt_cookie,char *errinfo,char *fpcxsj,char *mode,char **fpcxdata);

int etax_get_nsrjcxx_by_cookie_Interface(char *area_code,char *etax_cookie,char *tpass_cookie,char *dppt_cookie,char *errinfo,char **fpcxdata);
int etax_get_nsrfxxx_by_cookie_Interface(char *area_code,char *etax_cookie,char *tpass_cookie,char *dppt_cookie,char *errinfo,char **fpcxdata);
#endif