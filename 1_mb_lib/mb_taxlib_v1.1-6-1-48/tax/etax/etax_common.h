#ifndef ETAX_COMMON_H
#define ETAX_COMMON_H

#include "../common/common.h"
#include "etax_auth_server.h"
#include "etax_confirm.h"

enum enEtaxOperate {
    ETAX_HELLO = 0,
    ETAX_QUERYMM,
    ETAX_LOGIN,
    ETAX_LOGOUT,
    ETAX_SPLIT_LINE = 999,
    ETAX_SWSXTZS = 1000,  
    ETAX_FPXZ,  
    ETAX_HZFP,  
    ETAX_NSRXX, 
    ETAX_FPCX,
};
typedef enum enEtaxOperate ETaxFlag;


//全电登陆所需要的HTTP结构
struct EtaxConnections {
    struct EHTTP etax;
    struct EHTTP tpass;
    struct EHTTP dppt;
};
typedef struct EtaxConnections *HETAX;

//HTTP应用层上下文结构
struct ContextHttpAppBridge {
    char szUuid[BS_SMALL];                 //连接层UUID，而非业务层的uuid(很重要，不要乱覆盖)
    char szClientId[BS_LITTLE];
    char szBuf[BS_BIG];
    char szRandom16BytesKey[BS_TINY];       //贯穿会话始终不变，否则加解密失败
    char szToken[BS_BIG];                   //认证后token
    char szSecurityTokenKey[BS_LITTLE];     //数字账户页TokenKey
    char szHeaderLzkqow23819[BS_NORMAL];    // 20230317新增dppt校验参数
    char sPublickKey[BS_HUGE];;         // tpass层公钥
    struct evbuffer *ebJsVmpWebshellcode;   // dppt用
    uint64 llFingerPrint;
};
typedef struct ContextHttpAppBridge *CtxBridge;



typedef int (*EtaxFuction)(HETAX hEtax, HDEV hDev,void *args,char *errinfo_uft8);

typedef struct 
{
    EtaxFuction etax_login;             //一键登陆
    EtaxFuction etax_user_query;        //用户查询
    EtaxFuction etax_enterpriselist;    //企业列表
    EtaxFuction etax_relationchange;    //企业切换

    EtaxFuction etax_invoice_query;            //发票列表查询
    EtaxFuction etax_invoice_query_detail;     //发票详情查询
    EtaxFuction etax_invoice_query_down;       //发票下载
    EtaxFuction etax_rpa_invoice_issuance;     //发票开具RPA扫码
    
    EtaxFuction etax_rpa_invoice_issuance_status;     //发票开具RPA扫码【状态查询】

    EtaxFuction etax_credit_limit;             //授信额度查询

    EtaxFuction etax_blue_ink_invoice_issuance;     //蓝字发票开具

    EtaxFuction etax_hzqrxxSzzhQuery;               //可开红票的蓝票查询
    EtaxFuction etax_hzqrxxSave;                    //红字确认单开具
    EtaxFuction etax_hzqrxxQuery;                   //红字确认单列表查询
    EtaxFuction etax_hzqrxxQueryDetail;             //红字确认单详情查询
    EtaxFuction etax_hcstaquery_invoice_issuance;   //红冲类查询

    EtaxFuction etax_invoice_query_by_cookies;      //发票列表查询by cookies
    EtaxFuction etax_invoice_query_detail_by_cookies;      //发票详情查询by cookies
    EtaxFuction etax_invoice_query_down_by_cookies;      //下载发票by cookies
    EtaxFuction etax_rpa_invoice_issuance_by_cookies;      //rpa by cookies
    EtaxFuction etax_credit_limit_by_cookies;      //授信额度 by cookie
    EtaxFuction etax_hcstaquery_invoice_issuance_by_cookies;      //红冲类查询
    EtaxFuction etax_blue_ink_invoice_issuance_by_cookies; //蓝票开具 by cookie
    EtaxFuction etax_hzqrxxSzzhQuery_by_cookies; //可开红票的蓝票查询 by cookie
    EtaxFuction etax_hzqrxxSave_by_cookies; //红字确认单开具 by cookie
    EtaxFuction etax_hzqrxxQuery_by_cookies; //红字确认单列表查询 by cookie
    EtaxFuction etax_hzqrxxQueryDetail_by_cookies; //红字确认单详情查询 by cookie
    EtaxFuction etax_fjxxpeizhi_query_by_cookies;
    EtaxFuction etax_cjmbpeizhi_query_by_cookies;


    EtaxFuction etax_rpa_invoice_issuance_status_by_cookies;


    EtaxFuction etax_get_cookie;      //发票详情查询by cookies
	EtaxFuction etax_fjxxpeizhi_query;
	EtaxFuction etax_cjmbpeizhi_query;

    EtaxFuction etax_get_nsrjcxx_by_cookies;
    EtaxFuction etax_get_nsrfxxx_by_cookies;

    EtaxFuction etax_get_nsrjcxx;   

}ETAX_FUCTION;

typedef struct 
{
    char *szRegCode;
    ETAX_FUCTION* etax_fuction;

}ETAX_STRUCT;

//一键登陆
typedef struct 
{
    char *area_code;
    char *login_method;
    char *svr_container;
    char *related_type;
    char *full_name;
    char *mobile;
    char *idcard;
    char *sz_password;
    char *lpgoto;
    char *lpcode;
    char *lptoken;
    char *sys_type;
    char *soft_cert_ip;
    int soft_cert_port;

}ETAX_LOGIN_PARAMETER;

//用户查询
typedef struct 
{
    char *area_code;
    char *login_method;
    char *svr_container;

    char **outinfo;

    char *soft_cert_ip;
    int soft_cert_port;
    
}ETAX_USER_QUERY_PARAMETER;

//企业列表
typedef struct 
{
    char *area_code;
    char *login_method;
    char *svr_container;

    char *related_type;
    char *full_name;
    char *mobile;
    char *idcard;
    char *sz_password;
    char **outenterpriselist;

    char *soft_cert_ip;
    int soft_cert_port;
    
}ETAX_ENTERPRISELIST_PARAMETER;

//企业切换
typedef struct 
{
    char *area_code;
    char *login_method;
    char *svr_container;
    
    char *related_type;
    char *full_name;
    char *mobile;
    char *idcard;
    char *sz_password;
    char *uniqueIdentity;
    char *realationStatus;
    char *lpgoto;
    char *lpcode;
    char *lptoken;
    char *sys_type;

    char *soft_cert_ip;
    int soft_cert_port;
    
}ETAX_RELATIONCHANGE_PARAMETER;

//发票列表查询
typedef struct 
{
    char *fpcxsj;
    char *related_type;
    char *full_name;
    char *mobile;
    char *idcard;
    char *sz_password;
    int realationtype;
    char *uniqueIdentity;
    char *realationStatus;
    char **fpcxdata;
    
}INVOICE_QUERY;


//发票详情
typedef struct 
{
    char *fpcxsj;
    char *related_type;
    char *full_name;
    char *mobile;
    char *idcard;
    char *sz_password;
    int realationtype;
    char *uniqueIdentity;
    char *realationStatus;
    char **fpcxdata;
    
}INVOICE_QUERY_DETAIL;


//发票下载
typedef struct 
{
    char *fpcxsj;
    char *related_type;
    char *full_name;
    char *mobile;
    char *idcard;
    char *sz_password;
    int realationtype;
    char *uniqueIdentity;
    char *realationStatus;
    char **fpcxdata;
    char **fpcxname;
    
}INVOICE_QUERY_DOWN;

//开具扫码 RAP
typedef struct 
{
    char *fpkjurl;
    char *nsrsbh;
    char *related_type;
    char *mobile;
    char *idcard;
    char *sz_password;
    int realationtype;
    char *uniqueIdentity;
    char *realationStatus;
    char **fpkjdata;
}RPA_INVOICE_ISSUANCE;

//开具扫码 RAP 【状态查询】
typedef struct 
{
    char *fpkjurl;
    char *nsrsbh;
    char *related_type;
    char *mobile;
    char *idcard;
    char *sz_password;
    int realationtype;
    char *uniqueIdentity;
    char *realationStatus;
    char **fpkjdata;
    char *rzid;
    char *app_code;
}RPA_INVOICE_STATUS_ISSUANCE;

//授信额度
typedef struct 
{
    char *nsrsbh;
    char *related_type;
    char *mobile;
    char *idcard;
    char *sz_password;
    int realationtype;
    char *uniqueIdentity;
    char *realationStatus;
    char **sxeddata;
}CREDIT_LIMIT;

//蓝字发票开具
typedef struct 
{
    char *fpkjjson;
    char *related_type;
    char *mobile;
    char *idcard;
    char *sz_password;
    int realationtype;
    char *uniqueIdentity;
    char *realationStatus;
    char **fpkjout;
    char **hqnsrjcxx;
}BLUE_INK_INVOICE_ISSUANCE;


//发票列表查询
typedef struct 
{
    char *fpcxsj;
    char *related_type;
    char *mobile;
    char *idcard;
    char *sz_password;
    int realationtype;
    char *uniqueIdentity;
    char *realationStatus;
    char **fpcxdata;
    
}INVOICE_HCSTAQUERY;

typedef struct 
{
    char *fpcxsj;
    char *mode;
    char *related_type;
    char *mobile;
    char *idcard;
    char *sz_password;
    int realationtype;
    char *uniqueIdentity;
    char *realationStatus;
    char **fpcxdata;
    
}FJXXCJMB_MODE;


//开具扫码 RAP  by cookie
typedef struct 
{
    char *fpcxsj;
    char *mode;
    char *related_type;
    char *mobile;
    char *idcard;
    char *sz_password;
    int realationtype;
    char *uniqueIdentity;
    char *realationStatus;
    char *fpkjurl;
    char *nsrsbh;
    char **fpkjdata;
    char *rzid;
    char *app_code;

}RPA_INVOICE_ISSUANCE_STATUS;

//剥离后的接口

//获取cookie
typedef struct 
{
    char *related_type;
    char *mobile;
    char *idcard;
    char *sz_password;
    int realationtype;
    char *uniqueIdentity;
    char *realationStatus;
    char **etax_cookie;
    char **tpass_cookie;
    char **dppt_cookie;
    char *svr_container;
    char *soft_cert_ip;
    int soft_cert_port;
    char *area_code;
    char *login_method;
    char *temp_data;
}GET_COOKIE;

//发票列表查询by_cookies
typedef struct 
{
    char *area_code;
    char *etax_cookie;
    char *tpass_cookie;
    char *dppt_cookie;
    char *new_etax_cookie;
    char *new_tpass_cookie;
    char *new_dppt_cookie;
    char *fpcxsj;
    char **fpcxdata;

}INVOICE_QUERY_BY_COOKIES;

//发票下载 by cookie
typedef struct 
{
    char *area_code;
    char *etax_cookie;
    char *tpass_cookie;
    char *dppt_cookie;
    char *new_etax_cookie;
    char *new_tpass_cookie;
    char *new_dppt_cookie;
    char *fpcxsj;
    char **fpcxdata;
    char **fpcxname;
    
}INVOICE_QUERY_DOWN_BY_COOKIE;

//开具扫码 RAP  by cookie
typedef struct 
{
    char *area_code;
    char *etax_cookie;
    char *tpass_cookie;
    char *dppt_cookie;
    char *new_etax_cookie;
    char *new_tpass_cookie;
    char *new_dppt_cookie;
    char *fpkjurl;
    char *nsrsbh;
    char **fpkjdata;

}RPA_INVOICE_ISSUANCE_BY_COOKIE;

//开具扫码 RAP  by cookie
typedef struct 
{
    char *area_code;
    char *etax_cookie;
    char *tpass_cookie;
    char *dppt_cookie;
    char *new_etax_cookie;
    char *new_tpass_cookie;
    char *new_dppt_cookie;
    char *fpkjurl;
    char *nsrsbh;
    char **fpkjdata;
    char *rzid;
    char *app_code;

}RPA_INVOICE_ISSUANCE_STATUS_BY_COOKIE;


//授信额度 by cookie
typedef struct 
{
    char *area_code;
    char *etax_cookie;
    char *tpass_cookie;
    char *dppt_cookie;
    char *new_etax_cookie;
    char *new_tpass_cookie;
    char *new_dppt_cookie;
    char *nsrsbh;
    char **sxeddata;
}CREDIT_LIMITB_BY_COOKIE;

typedef struct 
{
    char *area_code;
    char *etax_cookie;
    char *tpass_cookie;
    char *dppt_cookie;
    char *new_etax_cookie;
    char *new_tpass_cookie;
    char *new_dppt_cookie;
    char *fpcxsj;
    char **fpcxdata;
    
}INVOICE_HCSTAQUERY_BY_COOKIE;

//蓝字发票开具 by cookie
typedef struct 
{
    char *area_code;
    char *etax_cookie;
    char *tpass_cookie;
    char *dppt_cookie;
    char *new_etax_cookie;
    char *new_tpass_cookie;
    char *new_dppt_cookie;
    char *fpkjjson;
    char **fpkjout;
    char **hqnsrjcxx;
}BLUE_INK_INVOICE_ISSUANCE_BY_COOKIE;

typedef struct 
{
    char *area_code;
    char *etax_cookie;
    char *tpass_cookie;
    char *dppt_cookie;
    char *new_etax_cookie;
    char *new_tpass_cookie;
    char *new_dppt_cookie;
    char *mode;
    char *fpcxsj;
    char **fpcxdata;
    
}FJXXCJMB_MODE_BY_COOKIE;



typedef struct 
{
    char *area_code;
    char *etax_cookie;
    char *tpass_cookie;
    char *dppt_cookie;
    char **fpcxdata;
    
}NRSXXCX_BY_COOKIE;

#endif