/*****************************************************************************
File name:   etax_call.c
Description: 用于江苏省调用etax接口
Author:      Wang
Version:     1.0
Date:        2023.02
History:
*****************************************************************************/
#include "jiangsu_call.h"
#include "../common/console.h"

int etax_jiangsu_login(HETAX hEtax, HDEV hDev,void *args,char *errinfo_uft8);
int etax_jiangsu_user_query(HETAX hEtax, HDEV hDev,void *args,char *errinfo_uft8);
int etax_jiangsu_enterpriselist(HETAX hEtax, HDEV hDev,void *args,char *errinfo_uft8);
int etax_jiangsu_relationchange(HETAX hEtax, HDEV hDev,void *args,char *errinfo_uft8);

int etax_jiangsu_invoice_query(HETAX hEtax, HDEV hDev,void *args,char *errinfo_uft8);
int etax_jiangsu_invoice_query_detail(HETAX hEtax, HDEV hDev,void *args,char *errinfo_uft8);
int etax_jiangsu_invoice_query_down(HETAX hEtax, HDEV hDev,void *args,char *errinfo_uft8);
int etax_jiangsu_rpa_invoice_issuance(HETAX hEtax, HDEV hDev,void *args,char *errinfo_uft8);

int etax_jiangsu_rpa_invoice_issuance_status(HETAX hEtax, HDEV hDev,void *args,char *errinfo_uft8);

int etax_jiangsu_credit_limit(HETAX hEtax, HDEV hDev,void *args,char *errinfo_uft8);
int etax_jiangsu_hcstaquery_invoice_issuance(HETAX hEtax, HDEV hDev,void *args,char *errinfo_uft8);

int etax_jiangsu_blue_ink_invoice_issuance(HETAX hEtax, HDEV hDev,void *args,char *errinfo_uft8);
int etax_jiangsu_hzqrxxSzzhQuery(HETAX hEtax, HDEV hDev,void *args,char *errinfo_uft8);
int etax_jiangsu_hzqrxxQuery(HETAX hEtax, HDEV hDev,void *args,char *errinfo_uft8);
int etax_jiangsu_hzqrxxQueryDetail(HETAX hEtax, HDEV hDev,void *args,char *errinfo_uft8);
int etax_jiangsu_hzqrxxSave(HETAX hEtax, HDEV hDev,void *args,char *errinfo_uft8);


int etax_jiangsu_invoice_query_bycookies(HETAX hEtax, HDEV hDev,void *args,char *errinfo_uft8);
int etax_jiangsu_invoice_query_detail_bycookies(HETAX hEtax, HDEV hDev,void *args,char *errinfo_uft8);
int etax_jiangsu_invoice_query_down_by_cookie(HETAX hEtax, HDEV hDev,void *args,char *errinfo_uft8);
int etax_jiangsu_rpa_invoice_issuance_by_cookie(HETAX hEtax, HDEV hDev,void *args,char *errinfo_uft8);
int etax_jiangsu_rpa_invoice_issuance_status_by_cookie(HETAX hEtax, HDEV hDev,void *args,char *errinfo_uft8);


int etax_jiangsu_credit_limit_by_cookie(HETAX hEtax, HDEV hDev,void *args,char *errinfo_uft8);

int etax_jiangsu_hzqrxxSzzhQuery_bycookies(HETAX hEtax, HDEV hDev,void *args,char *errinfo_uft8);
int etax_jiangsu_hzqrxxSave_bycookies(HETAX hEtax, HDEV hDev,void *args,char *errinfo_uft8);
int etax_jiangsu_hzqrxxQuery_bycookies(HETAX hEtax, HDEV hDev,void *args,char *errinfo_uft8);
int etax_jiangsu_hzqrxxQueryDetail_bycookies(HETAX hEtax, HDEV hDev,void *args,char *errinfo_uft8);

int etax_jiangsu_hcstaquery_invoice_issuance_by_cookie(HETAX hEtax, HDEV hDev,void *args,char *errinfo_uft8);

int etax_jiangsu_get_cookie(HETAX hEtax, HDEV hDev,void *args,char *errinfo_uft8);

int etax_jiangsu_blue_ink_invoice_issuance_bycookies(HETAX hEtax, HDEV hDev,void *args,char *errinfo_uft8);


int etax_jiangsu_fjxxpeizhi_query_bycookies(HETAX hEtax, HDEV hDev,void *args,char *errinfo_uft8);
int etax_jiangsu_cjmbpeizhi_query_bycookies(HETAX hEtax, HDEV hDev,void *args,char *errinfo_uft8);

int etax_jiangsu_fjxxpeizhi_query(HETAX hEtax, HDEV hDev,void *args,char *errinfo_uft8);
int etax_jiangsu_cjmbpeizhi_query(HETAX hEtax, HDEV hDev,void *args,char *errinfo_uft8);

int etax_jiangsu_get_nsrjcxx_by_cookies(HETAX hEtax, HDEV hDev,void *args,char *errinfo_uft8);
int etax_jiangsu_get_nsrfxxx_by_cookies(HETAX hEtax, HDEV hDev,void *args,char *errinfo_uft8);

ETAX_FUCTION etax_jiangsu_fuction = 
{
    .etax_login = etax_jiangsu_login,
    .etax_user_query = etax_jiangsu_user_query,
    .etax_enterpriselist = etax_jiangsu_enterpriselist,
    .etax_relationchange = etax_jiangsu_relationchange,
    //传统方式
    .etax_invoice_query = etax_jiangsu_invoice_query,
    .etax_invoice_query_detail = etax_jiangsu_invoice_query_detail,
    .etax_invoice_query_down = etax_jiangsu_invoice_query_down,
    .etax_rpa_invoice_issuance = etax_jiangsu_rpa_invoice_issuance,
    .etax_credit_limit = etax_jiangsu_credit_limit,
    .etax_hcstaquery_invoice_issuance = etax_jiangsu_hcstaquery_invoice_issuance,
	.etax_blue_ink_invoice_issuance = etax_jiangsu_blue_ink_invoice_issuance,
    .etax_hzqrxxSzzhQuery = etax_jiangsu_hzqrxxSzzhQuery,
    .etax_hzqrxxQuery = etax_jiangsu_hzqrxxQuery,
    .etax_hzqrxxQueryDetail = etax_jiangsu_hzqrxxQueryDetail,
    .etax_hzqrxxSave = etax_jiangsu_hzqrxxSave,
    .etax_fjxxpeizhi_query = etax_jiangsu_fjxxpeizhi_query,
    .etax_cjmbpeizhi_query = etax_jiangsu_cjmbpeizhi_query,
	.etax_rpa_invoice_issuance_status = etax_jiangsu_rpa_invoice_issuance_status,

    //通过cookie的方式
    .etax_invoice_query_by_cookies = etax_jiangsu_invoice_query_bycookies,
    .etax_invoice_query_detail_by_cookies = etax_jiangsu_invoice_query_detail_bycookies,
    .etax_invoice_query_down_by_cookies = etax_jiangsu_invoice_query_down_by_cookie,
    .etax_rpa_invoice_issuance_by_cookies = etax_jiangsu_rpa_invoice_issuance_by_cookie,
    .etax_credit_limit_by_cookies = etax_jiangsu_credit_limit_by_cookie,
    .etax_hcstaquery_invoice_issuance_by_cookies = etax_jiangsu_hcstaquery_invoice_issuance_by_cookie,
    .etax_blue_ink_invoice_issuance_by_cookies = etax_jiangsu_blue_ink_invoice_issuance_bycookies,
    .etax_hzqrxxSzzhQuery_by_cookies = etax_jiangsu_hzqrxxSzzhQuery_bycookies,
    .etax_hzqrxxSave_by_cookies = etax_jiangsu_hzqrxxSave_bycookies,
    .etax_hzqrxxQuery_by_cookies = etax_jiangsu_hzqrxxQuery_bycookies,
    .etax_hzqrxxQueryDetail_by_cookies = etax_jiangsu_hzqrxxQueryDetail_bycookies,
    .etax_fjxxpeizhi_query_by_cookies = etax_jiangsu_fjxxpeizhi_query_bycookies,
    .etax_cjmbpeizhi_query_by_cookies = etax_jiangsu_cjmbpeizhi_query_bycookies,
    .etax_get_nsrjcxx_by_cookies = etax_jiangsu_get_nsrjcxx_by_cookies,
    .etax_get_nsrfxxx_by_cookies = etax_jiangsu_get_nsrfxxx_by_cookies,

    .etax_rpa_invoice_issuance_status_by_cookies = etax_jiangsu_rpa_invoice_issuance_status_by_cookie,

    //获取cookie
    .etax_get_cookie = etax_jiangsu_get_cookie
};

//自定义header
HEADER_STRUCT header_array_jiangsu[] = {
    {"Cookie", "oauth2_referer=szzhzz.jiangsu.chinatax.gov.cn; x_host_key=188715edd92-73398ffbd187ed688673e9dcd9a219287ac81e5c"},
    {"Sec-Fetch-Dest", "document"},
    {"Sec-Fetch-Mode", "navigate"},
    {"Sec-Fetch-Site", "none"},
    {"Sec-Fetch-User", "?1"},
    {"Upgrade-Insecure-Requests", "1"},
    {"User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/113.0.0.0 Safari/537.36"},
    {"sec-ch-ua", "\"Google Chrome\";v=\"113\", \"Chromium\";v=\"113\", \"Not-A.Brand\";v=\"24\""},
    {"sec-ch-ua-mobile", "?0"},
    {"sec-ch-ua-platform", "\"Windows\""},
    NULL,NULL
};

//一键登陆
int etax_jiangsu_login(HETAX hEtax, HDEV hDev,void *args,char *errinfo_uft8)
{
    struct evhttp_uri *uriTpassFull = NULL;
    ETAX_LOGIN_PARAMETER *login_parameter = (ETAX_LOGIN_PARAMETER *)args;

    int nRet = ERR_GENERIC;
    int nChildRet = ERR_GENERIC;
    int nCount,i;

    char *nEtaxBufData = NULL;
    char *pValue = NULL;
    char *szTPassConfigJson = NULL;
    char szUserId[BS_LITTLE] = {0};
    char szBuf[BS_BLOCK * 6] = {0};
    char HTTPGetEventUrl[BS_BIG];
    char final_url[BS_HUGE] = {0x00};
    char errinfo_uft8_21[128] = {0};

    cJSON *jsonTpassConfig = NULL;
    cJSON *jsonValue = NULL;
    cJSON *root = NULL, *objItem = NULL, *objItemChild = NULL;
    cJSON *objItemChild2 = NULL,*objItemToken  = NULL;

    if(NULL == login_parameter)
        return nRet;

    if(!atoi(login_parameter->sys_type))  //默认税
    {
        hEtax->etax.user_args = NULL;
    }
    else                 //票
    {
        hEtax->etax.user_args = (void *)(&header_array_jiangsu);
        memset(hEtax->etax.modelTaAddr.szTaxAuthorityURL,0x00,sizeof(hEtax->etax.modelTaAddr.szTaxAuthorityURL));
        strcpy(hEtax->etax.modelTaAddr.szTaxAuthorityURL,"https://dppt.jiangsu.chinatax.gov.cn:8443");
        memset(hEtax->etax.modelTaAddr.szInitRequestPage,0x00,sizeof(hEtax->etax.modelTaAddr.szInitRequestPage));
        strcpy(hEtax->etax.modelTaAddr.szInitRequestPage,"/szzhzz/spHandler?cdlj=digital-tax-account");
    }

    do{
        //步骤1 使用etax地址获取tpass地址
        if (nChildRet = EtaxHTTPInit(&hEtax->etax, hDev)) {
            sprintf((char *)errinfo_uft8, "306Steps 1:HTTP 初始化失败, nRet:%d", nChildRet);
            break;
        }
        if (nChildRet = EtaxHTTPOpen(&hEtax->etax)) {
            sprintf((char *)errinfo_uft8, "306Steps 1:HTTP 连接失败, nRet:%d", nChildRet);
            break;
        }
        if (!(hEtax->etax.ctxApp = calloc(1, sizeof(struct ContextHttpAppBridge)))){
            sprintf((char *)errinfo_uft8, "306内存申请失败,空间不足");
            break;
        }
        hEtax->etax.cbHttpClose = EtaxBridgeClose;
        //获取tpass地址
        //步骤3 从token服务器获取登陆凭证去登陆Etax,登陆完成后返回的用户列表在hiTPass->bufHttpRep中
        if (hEtax->etax.ctxCon.nHttpRepCode != 200 && hEtax->etax.ctxCon.nHttpRepCode != 302){
            sprintf((char *)errinfo_uft8, "406请求跳转地址失败,HTTP Code:%d",hEtax->etax.ctxCon.nHttpRepCode);  
            break;
        }
        if(hEtax->etax.ctxCon.nHttpRepCode == 200)
        {
            szTPassConfigJson = (char *)evbuffer_pullup(hEtax->etax.bufHttpRep, -1);
        }
        else if(hEtax->etax.ctxCon.nHttpRepCode == 302)
        {
            szTPassConfigJson = (char *)hEtax->etax.user_args;
        }
        //printf("szTPassConfigJson=%s\r\n",szTPassConfigJson);
        //获取跳转tpass地址
        if(RET_SUCCESS != get_tpass_addr(szTPassConfigJson,&pValue,login_parameter->sys_type))
        {
             sprintf((char *)errinfo_uft8, "406获取跳转地址错误,地址解析失败");
             break;
        }

        //printf("pValue=%s\r\n",pValue);
        
        if (!(uriTpassFull = evhttp_uri_parse(pValue))){
            sprintf((char *)errinfo_uft8, "406获取跳转地址错误,地址解析失败");
            break;
        }

        if (!FillUploadAddrModelDynamic(TAADDR_ETAX_TPASS, hDev->szRegCode, pValue, "/",&hEtax->tpass.modelTaAddr)) {
            sprintf((char *)errinfo_uft8, "406请求跳转地址失败,HTTP Code:%d",hEtax->etax.ctxCon.nHttpRepCode);
            break;  
        } 

        if (nChildRet = EtaxHTTPInit(&hEtax->tpass, hDev)) {
            sprintf((char *)errinfo_uft8, "306Steps 2:HTTP 初始化失败, nRet:%d", nChildRet);
            break;
        }

        if (nChildRet = EtaxHTTPOpen(&hEtax->tpass)) {
            sprintf((char *)errinfo_uft8, "306Steps 2:HTTP 连接失败, nRet:%d", nChildRet);
            break;
        }
        if (!(hEtax->tpass.ctxApp = calloc(1, sizeof(struct ContextHttpAppBridge)))){
            sprintf((char *)errinfo_uft8, "306Steps 3:HTTPS CtxApp calloc Error");
            break;
        }
        hEtax->tpass.cbHttpClose = EtaxBridgeClose;
        
        //生成SM4密钥、获取公钥、发送SM4密钥
        if ((nChildRet = TPassLoginAuthSm4AndPublickey(&hEtax->tpass,hDev->szRegCode))) {
            sprintf((char *)errinfo_uft8, "406请求公钥或者发送SM4密钥失败:%s",(const char *)evbuffer_pullup(hEtax->tpass.bufHttpRep, -1));
            break;
        }

        //获取税盘证书与税局地址进行签名验证
        printf("             login_method           login_method            =   [%s]                       \r\n",login_parameter->login_method);
        if(memcmp(login_parameter->login_method,"01",2) == 0)  //软证书  "192.168.0.247",18002  老：172.30.92.124:18001 login_parameter->soft_cert_ip,login_parameter->soft_cert_port
        {
            TPassLoginAuthHelloAndAuth_soft_cert(&hEtax->tpass, uriTpassFull,login_parameter->svr_container,login_parameter->soft_cert_ip,login_parameter->soft_cert_port);
        }
        else if (memcmp(login_parameter->login_method,"21",2) == 0)
        {
            if ((nChildRet = TPassLoginAuthHelloAndAuth_soft_cert_21_Msg(&hEtax->tpass, uriTpassFull,login_parameter->svr_container,login_parameter->soft_cert_ip,login_parameter->soft_cert_port,errinfo_uft8_21)))
            {
                sprintf((char *)errinfo_uft8, "%s",errinfo_uft8_21);
                printf("lbc-debug nChildRet=%d        nRet = %d     \n",nChildRet,nRet);
                break;
            }
        }       
        else if(memcmp(login_parameter->login_method,"02",2) == 0)  //数字证书
        {
            if ((nChildRet = TPassLoginAuthHelloAndAuth(&hEtax->tpass, uriTpassFull))) {
                sprintf((char *)errinfo_uft8, "406签名验签服务器响应请求错误,可能为税局服务器原因，请插盘尝试");
                printf("lbc-debug nChildRet=%d\n",nChildRet);
                break;
            }
        }
        
        //步骤4 根据签名验证请求后返回的用户信息以及中台传入的用户信息进行比较，筛选出需要登陆的身份
        if (!(root = cJSON_Parse((const char *)evbuffer_pullup(hEtax->tpass.bufHttpRep, -1)))) {
            sprintf((char *)errinfo_uft8, "406登陆凭证中返回用户列表为空");
            break;
        }
        if (!(objItem = cJSON_GetObjectItem(root, "userList"))) {
            sprintf((char *)errinfo_uft8, "406登陆凭证中返回用户列表解析失败");
            break;
        }
        nCount = cJSON_GetArraySize(objItem);
        for (i = 0; i < nCount; i++) {
            objItemChild = cJSON_GetArrayItem(objItem, i);

            if(strlen(login_parameter->idcard)){
                if (!(objItemChild2 = cJSON_GetObjectItem(objItemChild, "id_card")))
                    continue;
                if (strcmp(objItemChild2->valuestring, login_parameter->idcard))
                    continue;
            }

            if(strlen(login_parameter->mobile)){
                if (!(objItemChild2 = cJSON_GetObjectItem(objItemChild, "mobile")))
                    continue;
                if (strcmp(objItemChild2->valuestring, login_parameter->mobile))
                    continue;
            }

            if (!(objItemChild2 = cJSON_GetObjectItem(objItemChild, "relatedType")))
                continue;
            if (strcmp(objItemChild2->valuestring, login_parameter->related_type))
                continue;
                
            if (!(objItemChild2 = cJSON_GetObjectItem(objItemChild, "user_id")))
                continue;
            strcpy(szUserId, objItemChild2->valuestring);
            printf("szUserId=%s,sz_password=%s\r\n",szUserId,login_parameter->sz_password);
            break;
        }

        if(!strlen(szUserId)){
            sprintf((char *)errinfo_uft8, "306用户列表中未检测到该用户信息,请核实登陆信息是否有误");
            break;
        }

        //步骤5 根据筛选出来的身份进行登陆
        if (nChildRet = TPassLoginPasswordLoginDo(&hEtax->tpass, szUserId, login_parameter->sz_password, szBuf,sizeof(szBuf))){
            if (!(root = cJSON_Parse(szBuf)))
                break;
            if (!(objItem = cJSON_GetObjectItem(root, "msg")))
                break;
            sprintf((char *)errinfo_uft8, "406登陆失败:%s",objItem->valuestring);
            break;
        }

        //步骤6 获取一键登陆需要的code参数和RPA需要的token参数
        if (root)
            cJSON_Delete(root);
        root = NULL;

        printf("TPassLoginPasswordLoginDo data is %s\n", szBuf);
        if (!(root = cJSON_Parse(szBuf))){
            sprintf((char *)errinfo_uft8, "406登陆失败,返回数据解析失败");
            break;
        }
        if (!(objItem = cJSON_GetObjectItem(root, "code"))){
            sprintf((char *)errinfo_uft8, "406登陆失败,返回数据中无code参数");
            break;
        }

        printf(" = %s\r\n",objItem->valuestring);
        if(!atoi(login_parameter->sys_type))  //默认税
        {
            strcpy(login_parameter->lpcode, objItem->valuestring); 
        }
        else                //票
        {
            if(0 > get_final_url(pValue,objItem->valuestring,final_url))
                break;
            strcpy(login_parameter->lpcode,final_url); 
        }

        if (!(objItemToken = cJSON_GetObjectItem(root, "access_token"))){
            sprintf((char *)errinfo_uft8, "406登陆失败,返回数据中无token参数");
            break;
        }

        strcpy(login_parameter->lptoken, objItemToken->valuestring); 
        printf("TPassLoginPasswordLoginDo Token = %s\r\n",objItemToken->valuestring);

        //结束
        nRet = RET_SUCCESS;
    }while (false);

    //释放内存
    if(nEtaxBufData)
        free(nEtaxBufData);
    nEtaxBufData = NULL;
    //释放内存
    if (root)
        cJSON_Delete(root);
    root = NULL;
    //释放内存
    if (jsonTpassConfig)
        cJSON_Delete(jsonTpassConfig);
    jsonTpassConfig  = NULL;
    if (uriTpassFull)
        evhttp_uri_free(uriTpassFull);
    if(pValue)
        free(pValue);
    if(hEtax->etax.user_args)
    {
        free(hEtax->etax.user_args);
        hEtax->etax.user_args = NULL;
    }
    return nRet;
}

//用户查询
int etax_jiangsu_user_query(HETAX hEtax, HDEV hDev,void *args,char *errinfo_uft8)
{
    struct evhttp_uri *uriTpassFull = NULL;
    ETAX_USER_QUERY_PARAMETER *user_query_parameter = (ETAX_USER_QUERY_PARAMETER *)args;

    int nRet = ERR_GENERIC;
    int nChildRet = ERR_GENERIC;
    int nCount,i;

    char *nEtaxBufData = NULL;
    char *pValue = NULL;
    char szUserId[BS_LITTLE] = {0};
    char szBuf[BS_BLOCK * 6] = {0};
    char HTTPGetEventUrl[200];

    cJSON *jsonTpassConfig = NULL;
    cJSON *jsonValue = NULL;
    cJSON *root = NULL, *objItem = NULL, *objItemChild = NULL;
    cJSON *objItemChild2 = NULL,*objItemToken  = NULL;

    hEtax->etax.user_args = NULL;
    if(NULL == user_query_parameter)
    {
        return nRet;
    }

    do{
        //步骤1 使用etax地址获取tpass地址
        if (nChildRet = EtaxHTTPInit(&hEtax->etax, hDev)) {
            sprintf((char *)errinfo_uft8, "306Steps 1:HTTP 初始化失败, nRet:%d", nChildRet);
            break;
        }
        if (nChildRet = EtaxHTTPOpen(&hEtax->etax)) {
            sprintf((char *)errinfo_uft8, "306Steps 1:HTTP 连接失败, nRet:%d", nChildRet);
            break;
        }
        if (!(hEtax->etax.ctxApp = calloc(1, sizeof(struct ContextHttpAppBridge)))){
            sprintf((char *)errinfo_uft8, "306内存申请失败,空间不足");
            break;
        }
        hEtax->etax.cbHttpClose = EtaxBridgeClose;
        //获取tpass地址
        //步骤3 从token服务器获取登陆凭证去登陆Etax
        if (hEtax->etax.ctxCon.nHttpRepCode != 200){
            sprintf((char *)errinfo_uft8, "406请求跳转地址失败,HTTP Code:%d",hEtax->etax.ctxCon.nHttpRepCode);  
            break;
        }
        char *szTPassConfigJson = (char *)evbuffer_pullup(hEtax->etax.bufHttpRep, -1);
       
        //获取跳转tpass地址
        //获取跳转tpass地址
        if(RET_SUCCESS != get_tpass_addr(szTPassConfigJson,&pValue,"0"))
        {
             sprintf((char *)errinfo_uft8, "406获取跳转地址错误,地址解析失败");
             break;
        }

        if (!(uriTpassFull = evhttp_uri_parse(pValue))){
            sprintf((char *)errinfo_uft8, "406获取跳转地址错误,地址解析失败");
            break;
        }

        if (!FillUploadAddrModelDynamic(TAADDR_ETAX_TPASS, hDev->szRegCode, pValue, "/",&hEtax->tpass.modelTaAddr)) {
            sprintf((char *)errinfo_uft8, "406获取跳转地址错误,地址解析失败");
            break;
        } 

        if (nChildRet = EtaxHTTPInit(&hEtax->tpass, hDev)) {
            sprintf((char *)errinfo_uft8, "306Steps 2:HTTP 初始化失败, nRet:%d", nChildRet);
            break;
        }

        if (nChildRet = EtaxHTTPOpen(&hEtax->tpass)) {
            sprintf((char *)errinfo_uft8, "306Steps 2:HTTP 连接失败, nRet:%d", nChildRet);
            break;
        }
        if (!(hEtax->tpass.ctxApp = calloc(1, sizeof(struct ContextHttpAppBridge)))){
            sprintf((char *)errinfo_uft8, "306内存申请失败,空间不足");
            break;
        }
        hEtax->tpass.cbHttpClose = EtaxBridgeClose;
        
        //生成SM4密钥、获取公钥、发送SM4密钥
        if (TPassLoginAuthSm4AndPublickey(&hEtax->tpass,hDev->szRegCode)) {
            sprintf((char *)errinfo_uft8, "406请求公钥或者发送SM4密钥失败:%s",(const char *)evbuffer_pullup(hEtax->tpass.bufHttpRep, -1));
            break;
        }

        //获取税盘证书与税局地址进行签名验证
        printf("             login_method           login_method            =   [%s]                       \r\n",user_query_parameter->login_method);
        if(memcmp(user_query_parameter->login_method,"01",2) == 0)  //软证书  "192.168.0.247",18002   login_parameter->soft_cert_ip,login_parameter->soft_cert_port
        {
            TPassLoginAuthHelloAndAuth_soft_cert(&hEtax->tpass, uriTpassFull,user_query_parameter->svr_container,user_query_parameter->soft_cert_ip,user_query_parameter->soft_cert_port);
        }
        else if (memcmp(user_query_parameter->login_method,"21",2) == 0)
        {
            if ((nChildRet = TPassLoginAuthHelloAndAuth_soft_cert_21(&hEtax->tpass, uriTpassFull,user_query_parameter->svr_container,user_query_parameter->soft_cert_ip,user_query_parameter->soft_cert_port)))
            {
                sprintf((char *)errinfo_uft8, "102税盘已离线或是登陆参数有误");
                printf("lbc-debug nChildRet=%d        nRet = %d     \n",nChildRet,nRet);
                break;
            }
        }    
        else if(memcmp(user_query_parameter->login_method,"02",2) == 0)  //数字证书
        {
            //获取税盘证书与税局地址进行签名验证
            if ((nChildRet = TPassLoginAuthHelloAndAuth(&hEtax->tpass, uriTpassFull))) {
                sprintf((char *)errinfo_uft8, "406签名验签服务器响应请求错误,可能为税局服务器原因，请插盘尝试");
                printf("lbc-debug nChildRet=%d\n",nChildRet);
                break;
            }
        }

        //释放内存
        if (uriTpassFull)
            evhttp_uri_free(uriTpassFull);
        uriTpassFull = NULL;

        //步骤4 提取签名验证成功后返回的用户信息
        nChildRet = evbuffer_get_length(hEtax->tpass.bufHttpRep);

        nEtaxBufData = NULL;
        nEtaxBufData = (char *)malloc(nChildRet);
        if(nEtaxBufData == NULL){
            sprintf((char *)errinfo_uft8, "306内存申请失败,空间不足");        
            break;
        }
        evbuffer_copyout_from(hEtax->tpass.bufHttpRep,NULL, nEtaxBufData,nChildRet);

        if (!(root = cJSON_Parse((const char *)evbuffer_pullup(hEtax->tpass.bufHttpRep, -1)))) {
            sprintf((char *)errinfo_uft8, "406登陆凭证中返回用户列表为空");
            break;
        }
        if (!(objItem = cJSON_GetObjectItem(root, "userList"))) {
            sprintf((char *)errinfo_uft8, "406登陆凭证中返回用户列表解析失败");
            break;
        }
        else 
        {
            nChildRet = evbuffer_get_length(hEtax->tpass.bufHttpRep);   
            //printf("lbc-debug nChildRet = %d,objItem->valuestring=%s\n nEtaxBufData = %s\n",nChildRet,objItem->valuestring,nEtaxBufData);
            *(user_query_parameter->outinfo) = NULL;
            *(user_query_parameter->outinfo) = (char *)malloc(nChildRet);
            if(*(user_query_parameter->outinfo) == NULL){    
                break;
            }
            EncordingConvert( "utf-8","gbk",nEtaxBufData, nChildRet, *(user_query_parameter->outinfo),nChildRet);
        }
        //结束
        nRet = RET_SUCCESS;
    }while (false);

    //释放内存
    if (root)
        cJSON_Delete(root);
    root = NULL;

    if(nEtaxBufData)
        free(nEtaxBufData);
    nEtaxBufData = NULL;     

    //释放内存
    if (uriTpassFull)
        evhttp_uri_free(uriTpassFull);
    uriTpassFull = NULL;    

    if (jsonTpassConfig)
        cJSON_Delete(jsonTpassConfig);
    jsonTpassConfig = NULL;

    if(pValue)
        free(pValue);

    if(hEtax->etax.user_args)
    {
        free(hEtax->etax.user_args);
        hEtax->etax.user_args = NULL;
    }

    return nRet;   
}

//企业列表获取
int etax_jiangsu_enterpriselist(HETAX hEtax, HDEV hDev,void *args,char *errinfo_uft8)
{
    struct evhttp_uri *uriTpassFull = NULL;
    ETAX_ENTERPRISELIST_PARAMETER *enterpriselist_parameter = (ETAX_ENTERPRISELIST_PARAMETER *)args;

    int nRet = ERR_GENERIC;
    int nChildRet = ERR_GENERIC;
    int nCount,i;

    char *pValue = NULL;
    char szUserId[BS_LITTLE] = {0};
    char szBuf[BS_BLOCK * 600] = {0};

    cJSON *jsonTpassConfig = NULL;
    cJSON *root = NULL, *objItem = NULL, *objItemChild = NULL;
    cJSON *objItemChild2 = NULL,*objItemToken  = NULL;

    hEtax->etax.user_args = NULL;

    if(NULL == enterpriselist_parameter)
    {
        return nRet;
    }

    do{
        //步骤1 使用etax地址获取tpass地址
        if (nChildRet = EtaxHTTPInit(&hEtax->etax, hDev)) {
            sprintf((char *)errinfo_uft8, "306Steps 1:HTTP 初始化失败, nRet:%d", nChildRet);
            break;
        }
        if (nChildRet = EtaxHTTPOpen(&hEtax->etax)) {
            sprintf((char *)errinfo_uft8, "306Steps 1:HTTP 连接失败, nRet:%d", nChildRet);
            break;
        }
        if (!(hEtax->etax.ctxApp = calloc(1, sizeof(struct ContextHttpAppBridge)))){
            sprintf((char *)errinfo_uft8, "306内存申请失败,空间不足");
            break;
        }
        hEtax->etax.cbHttpClose = EtaxBridgeClose;
        //获取tpass地
        //步骤3 从token服务器获取登陆凭证去登陆Etax,登陆完成后返回的用户列表在hiTPass->bufHttpRep中
        if (hEtax->etax.ctxCon.nHttpRepCode != 200){
            sprintf((char *)errinfo_uft8, "406请求跳转地址失败,HTTP Code:%d",hEtax->etax.ctxCon.nHttpRepCode);
            break;
        }
        char *szTPassConfigJson = (char *)evbuffer_pullup(hEtax->etax.bufHttpRep, -1);
       
        //获取跳转tpass地址
        if(RET_SUCCESS != get_tpass_addr(szTPassConfigJson,&pValue,"0"))
        {
             sprintf((char *)errinfo_uft8, "406获取跳转地址错误,地址解析失败");
             break;
        }
        
        if (!(uriTpassFull = evhttp_uri_parse(pValue))){
            sprintf((char *)errinfo_uft8, "406获取跳转地址错误,地址转换失败"); 
            break;
        }

        if (!FillUploadAddrModelDynamic(TAADDR_ETAX_TPASS, hDev->szRegCode, pValue, "/",&hEtax->tpass.modelTaAddr)) {
            sprintf((char *)errinfo_uft8, "406获取跳转地址错误,地址解析失败");
            break;
        } 

        if (nChildRet = EtaxHTTPInit(&hEtax->tpass, hDev)) {
            sprintf((char *)errinfo_uft8, "306Steps 2:HTTP 初始化失败, nRet:%d", nChildRet);
            break;
        }

        if (nChildRet = EtaxHTTPOpen(&hEtax->tpass)) {
            sprintf((char *)errinfo_uft8, "306Steps 2:HTTP 连接失败, nRet:%d", nChildRet);
            break;
        }
        if (!(hEtax->tpass.ctxApp = calloc(1, sizeof(struct ContextHttpAppBridge)))){
            sprintf((char *)errinfo_uft8, "306内存申请失败,空间不足");
            break;
        }
        hEtax->tpass.cbHttpClose = EtaxBridgeClose;
        
        //生成SM4密钥、获取公钥、发送SM4密钥
        if ((nChildRet = TPassLoginAuthSm4AndPublickey(&hEtax->tpass,hDev->szRegCode))) {
            sprintf((char *)errinfo_uft8, "406请求公钥或者发送SM4密钥失败:%s",(const char *)evbuffer_pullup(hEtax->tpass.bufHttpRep, -1));
            break;
        }

        //获取税盘证书与税局地址进行签名验证
        printf("             login_method           login_method            =   [%s]                       \r\n",enterpriselist_parameter->login_method);
        if(memcmp(enterpriselist_parameter->login_method,"01",2) == 0)  //软证书  "192.168.0.247",18002  老："172.30.92.124",18001 login_parameter->soft_cert_ip,login_parameter->soft_cert_port
        {
            TPassLoginAuthHelloAndAuth_soft_cert(&hEtax->tpass, uriTpassFull,enterpriselist_parameter->svr_container,enterpriselist_parameter->soft_cert_ip,enterpriselist_parameter->soft_cert_port);
        }
        else if (memcmp(enterpriselist_parameter->login_method,"21",2) == 0)
        {
            if ((nChildRet = TPassLoginAuthHelloAndAuth_soft_cert_21(&hEtax->tpass, uriTpassFull,enterpriselist_parameter->svr_container,enterpriselist_parameter->soft_cert_ip,enterpriselist_parameter->soft_cert_port)))
            {
                sprintf((char *)errinfo_uft8, "102税盘已离线或是登陆参数有误");
                printf("lbc-debug nChildRet=%d        nRet = %d     \n",nChildRet,nRet);
                break;
            }
        }
        else if(memcmp(enterpriselist_parameter->login_method,"02",2) == 0)  //数字证书
        {
            //获取税盘证书与税局地址进行签名验证
            if ((nChildRet = TPassLoginAuthHelloAndAuth(&hEtax->tpass, uriTpassFull))) {
                sprintf((char *)errinfo_uft8, "406签名验签服务器响应请求错误,可能为税局服务器原因，请插盘尝试");
                printf("lbc-debug nChildRet=%d\n",nChildRet);
                break;
            }
        }

        //步骤4 根据签名验证请求后返回的用户信息以及中台传入的用户信息进行比较，筛选出需要登陆的身份
        if (!(root = cJSON_Parse((const char *)evbuffer_pullup(hEtax->tpass.bufHttpRep, -1)))) {
            sprintf((char *)errinfo_uft8, "406登陆凭证中返回用户列表为空");
            break;
        }
        if (!(objItem = cJSON_GetObjectItem(root, "userList"))) {
            sprintf((char *)errinfo_uft8, "406登陆凭证中返回用户列表解析失败");
            break;
        }
        nCount = cJSON_GetArraySize(objItem);
        for (i = 0; i < nCount; i++) {
            objItemChild = cJSON_GetArrayItem(objItem, i);

            if(strlen(enterpriselist_parameter->idcard)){
                if (!(objItemChild2 = cJSON_GetObjectItem(objItemChild, "id_card")))
                    continue;
                if (strcmp(objItemChild2->valuestring, enterpriselist_parameter->idcard))
                    continue;
            }

            if(strlen(enterpriselist_parameter->mobile)){
                if (!(objItemChild2 = cJSON_GetObjectItem(objItemChild, "mobile")))
                    continue;
                if (strcmp(objItemChild2->valuestring, enterpriselist_parameter->mobile))
                    continue;
            }

                
            if (!(objItemChild2 = cJSON_GetObjectItem(objItemChild, "user_id")))
                continue;
            strcpy(szUserId, objItemChild2->valuestring);
            printf("szUserId=%s,sz_password=%s\r\n",szUserId,enterpriselist_parameter->sz_password);
            break;
        }
        if(!strlen(szUserId)){
            sprintf((char *)errinfo_uft8, "306用户列表中未检测到该用户信息,请核实登陆信息是否有误");
            break;
        }

        //步骤5 根据筛选出来的身份进行登陆
        if (nChildRet = TPassLoginPasswordLoginDo(&hEtax->tpass, szUserId, enterpriselist_parameter->sz_password, szBuf,sizeof(szBuf))){
            if (!(root = cJSON_Parse(szBuf)))
                break;
            if (!(objItem = cJSON_GetObjectItem(root, "msg")))
                break;
            sprintf((char *)errinfo_uft8, "406登陆失败:%s",objItem->valuestring);
            break;
        }


        //步骤6 获取企业列表信息
        if (nChildRet = TPassSelectRelationList(&hEtax->tpass,enterpriselist_parameter->outenterpriselist, szBuf,sizeof(szBuf))){
            sprintf((char *)errinfo_uft8, "406获取企业列表信息失败,HTTP Code:%d",hEtax->tpass.ctxCon.nHttpRepCode);
            break;
        }        
        //结束
        nRet = RET_SUCCESS;
    }while (false);

    //释放内存
    if (root)
        cJSON_Delete(root);
    root = NULL;

    //释放内存
    if (jsonTpassConfig)
        cJSON_Delete(jsonTpassConfig);
    jsonTpassConfig  = NULL;

    if (uriTpassFull)
        evhttp_uri_free(uriTpassFull);

    if(pValue)
        free(pValue);

    if(hEtax->etax.user_args)
    {
        free(hEtax->etax.user_args);
        hEtax->etax.user_args = NULL;
    }

    return nRet;
}

//企业切换
int etax_jiangsu_relationchange(HETAX hEtax, HDEV hDev,void *args,char *errinfo_uft8)
{
    struct evhttp_uri *uriTpassFull = NULL;
    ETAX_RELATIONCHANGE_PARAMETER *relationchange_parametetr = (ETAX_RELATIONCHANGE_PARAMETER *)args;

    int nRet = ERR_GENERIC;
    int nChildRet = ERR_GENERIC;
    int nCount,i;

    char *pValue = NULL;
    char *nEtaxBufData = NULL;
    char szUserId[BS_LITTLE] = {0};
    char szBuf[BS_BLOCK * 6] = {0};
    char final_url[512] = {0x00};
    char *szTPassConfigJson = NULL;

    cJSON *jsonTpassConfig = NULL;
    cJSON *root = NULL, *objItem = NULL, *objItemChild = NULL;
    cJSON *objItemChild2 = NULL,*objItemToken  = NULL;

    if(NULL == relationchange_parametetr)
    {
        return nRet;
    }

    if(!atoi(relationchange_parametetr->sys_type))  //默认税
    {
        hEtax->etax.user_args = NULL;
    }
    else                                            //票
    {
        hEtax->etax.user_args = (void *)(&header_array_jiangsu);
        memset(hEtax->etax.modelTaAddr.szTaxAuthorityURL,0x00,sizeof(hEtax->etax.modelTaAddr.szTaxAuthorityURL));
        strcpy(hEtax->etax.modelTaAddr.szTaxAuthorityURL,"https://dppt.jiangsu.chinatax.gov.cn:8443");
        memset(hEtax->etax.modelTaAddr.szInitRequestPage,0x00,sizeof(hEtax->etax.modelTaAddr.szInitRequestPage));
        strcpy(hEtax->etax.modelTaAddr.szInitRequestPage,"/szzhzz/spHandler?cdlj=digital-tax-account");
    }

    do{
        //步骤1 使用etax地址获取tpass地址
        if (nChildRet = EtaxHTTPInit(&hEtax->etax, hDev)) {
            sprintf((char *)errinfo_uft8, "306Steps 1:HTTP 初始化失败, nRet:%d", nChildRet);
            break;
        }
        if (nChildRet = EtaxHTTPOpen(&hEtax->etax)) {
            sprintf((char *)errinfo_uft8, "306Steps 1:HTTP 连接失败, nRet:%d", nChildRet);
            break;
        }
        if (!(hEtax->etax.ctxApp = calloc(1, sizeof(struct ContextHttpAppBridge)))){
            sprintf((char *)errinfo_uft8, "306内存申请失败,空间不足");
            break;
        }
        hEtax->etax.cbHttpClose = EtaxBridgeClose;
        //获取tpass地址
        //步骤3 从token服务器获取登陆凭证去登陆Etax,登陆完成后返回的用户列表在hiTPass->bufHttpRep中
        if (hEtax->etax.ctxCon.nHttpRepCode != 200 && hEtax->etax.ctxCon.nHttpRepCode != 302){
            sprintf((char *)errinfo_uft8, "406请求跳转地址失败,HTTP Code:%d",hEtax->etax.ctxCon.nHttpRepCode);  
            break;
        }
        if(hEtax->etax.ctxCon.nHttpRepCode == 200)
        {
            szTPassConfigJson = (char *)evbuffer_pullup(hEtax->etax.bufHttpRep, -1);
        }
        else if(hEtax->etax.ctxCon.nHttpRepCode == 302)
        {
            szTPassConfigJson = (char *)hEtax->etax.user_args;
        }
        //获取跳转tpass地址
        if(RET_SUCCESS != get_tpass_addr(szTPassConfigJson,&pValue,relationchange_parametetr->sys_type))
        {
             sprintf((char *)errinfo_uft8, "406获取跳转地址错误,地址解析失败");
             break;
        }

        printf("pValue=%s\r\n",pValue);

        if (!(uriTpassFull = evhttp_uri_parse(pValue))){
            sprintf((char *)errinfo_uft8, "406获取跳转地址错误,地址转换失败"); 
            break;
        }

        if (!FillUploadAddrModelDynamic(TAADDR_ETAX_TPASS, hDev->szRegCode, pValue, "/",&hEtax->tpass.modelTaAddr)) {
            sprintf((char *)errinfo_uft8, "406获取跳转地址错误,地址解析失败");
            break;
        } 

        if (nChildRet = EtaxHTTPInit(&hEtax->tpass, hDev)) {
            sprintf((char *)errinfo_uft8, "306Steps 2:HTTP 初始化失败, nRet:%d", nChildRet);
            break;
        }

        if (nChildRet = EtaxHTTPOpen(&hEtax->tpass)) {
            sprintf((char *)errinfo_uft8, "306Steps 2:HTTP 连接失败, nRet:%d", nChildRet);
            break;
        }
        if (!(hEtax->tpass.ctxApp = calloc(1, sizeof(struct ContextHttpAppBridge)))){
            sprintf((char *)errinfo_uft8, "306内存申请失败,空间不足");
            break;
        }
        hEtax->tpass.cbHttpClose = EtaxBridgeClose;
        
        //生成SM4密钥、获取公钥、发送SM4密钥
        if ((nChildRet = TPassLoginAuthSm4AndPublickey(&hEtax->tpass,hDev->szRegCode))) {
            sprintf((char *)errinfo_uft8, "406请求公钥或者发送SM4密钥失败:%s",(const char *)evbuffer_pullup(hEtax->tpass.bufHttpRep, -1));
            break;
        }

        //获取税盘证书与税局地址进行签名验证
        printf("             login_method           login_method            =   [%s]                       \r\n",relationchange_parametetr->login_method);
        if(memcmp(relationchange_parametetr->login_method,"01",2) == 0)  //软证书  "192.168.0.247",18002  老：172.30.92.124:18001 login_parameter->soft_cert_ip,login_parameter->soft_cert_port
        {
            TPassLoginAuthHelloAndAuth_soft_cert(&hEtax->tpass, uriTpassFull,relationchange_parametetr->svr_container,relationchange_parametetr->soft_cert_ip,relationchange_parametetr->soft_cert_port);
        }
        else if (memcmp(relationchange_parametetr->login_method,"21",2) == 0)
        {
            if ((nChildRet = TPassLoginAuthHelloAndAuth_soft_cert_21(&hEtax->tpass, uriTpassFull,relationchange_parametetr->svr_container,relationchange_parametetr->soft_cert_ip,relationchange_parametetr->soft_cert_port)))
            {
                sprintf((char *)errinfo_uft8, "102税盘已离线或是登陆参数有误");
                printf("lbc-debug nChildRet=%d        nRet = %d     \n",nChildRet,nRet);
                break;
            }
        }  
        else if(memcmp(relationchange_parametetr->login_method,"02",2) == 0)  //数字证书
        {
            //获取税盘证书与税局地址进行签名验证
            if ((nChildRet = TPassLoginAuthHelloAndAuth(&hEtax->tpass, uriTpassFull))) {
                sprintf((char *)errinfo_uft8, "406签名验签服务器响应请求错误,可能为税局服务器原因，请插盘尝试");
                printf("lbc-debug nChildRet=%d\n",nChildRet);
                break;
            }
        }

        //步骤4 根据签名验证请求后返回的用户信息以及中台传入的用户信息进行比较，筛选出需要登陆的身份
        if (!(root = cJSON_Parse((const char *)evbuffer_pullup(hEtax->tpass.bufHttpRep, -1)))) {
            sprintf((char *)errinfo_uft8, "406登陆凭证中返回用户列表为空");
            break;
        }
        if (!(objItem = cJSON_GetObjectItem(root, "userList"))) {
            sprintf((char *)errinfo_uft8, "406登陆凭证中返回用户列表解析失败");
            break;
        }
        nCount = cJSON_GetArraySize(objItem);
        for (i = 0; i < nCount; i++) {
            objItemChild = cJSON_GetArrayItem(objItem, i);

            if(strlen(relationchange_parametetr->idcard)){
                if (!(objItemChild2 = cJSON_GetObjectItem(objItemChild, "id_card")))
                    continue;
                if (strcmp(objItemChild2->valuestring, relationchange_parametetr->idcard))
                    continue;
            }

            if(strlen(relationchange_parametetr->mobile)){
                if (!(objItemChild2 = cJSON_GetObjectItem(objItemChild, "mobile")))
                    continue;
                if (strcmp(objItemChild2->valuestring, relationchange_parametetr->mobile))
                    continue;
            }

            if (!(objItemChild2 = cJSON_GetObjectItem(objItemChild, "user_id")))
                continue;
            strcpy(szUserId, objItemChild2->valuestring);
            printf("szUserId=%s,sz_password=%s\r\n",szUserId,relationchange_parametetr->sz_password);
            break;
        }
        if(!strlen(szUserId)){
            sprintf((char *)errinfo_uft8, "306用户列表中未检测到该用户信息,请核实登陆信息是否有误");
            break;
        }

        //步骤5 根据筛选出来的身份进行登陆
        if (nChildRet = TPassLoginPasswordLoginDo(&hEtax->tpass, szUserId, relationchange_parametetr->sz_password, szBuf,sizeof(szBuf))){
            if (!(root = cJSON_Parse(szBuf)))
                break;
            if (!(objItem = cJSON_GetObjectItem(root, "msg")))
                break;
            sprintf((char *)errinfo_uft8, "406登陆失败:%s",objItem->valuestring);
            break;
        }

        if (root)
            cJSON_Delete(root);
        root = NULL;

        //步骤6 企业切换
        if (nChildRet = TPassSelectRelationchange(&hEtax->tpass,relationchange_parametetr->uniqueIdentity,relationchange_parametetr->realationStatus, szBuf,sizeof(szBuf))){
            if (!(root = cJSON_Parse(szBuf)))
                break;
            if (!(objItem = cJSON_GetObjectItem(root, "msg")))
                break;
            sprintf((char *)errinfo_uft8, "406企业切换失败,%s",objItem->valuestring);
            break;
        } 

        //步骤7 获取一键登陆需要的code参数和RPA需要的token参数
        if (root)
            cJSON_Delete(root);
        root = NULL;

        printf("TPassSelectRelationchange data is %s\n", szBuf);
        if (!(root = cJSON_Parse(szBuf))){
            sprintf((char *)errinfo_uft8, "406企业切换失败,返回数据解析失败");
            break;
        }
        if (!(objItem = cJSON_GetObjectItem(root, "code"))){
            sprintf((char *)errinfo_uft8, "406企业切换失败,返回数据中无登陆参数");
            break;
        }

        printf(" = %s\r\n",objItem->valuestring);
        if(!atoi(relationchange_parametetr->sys_type))  //默认税
        {
            strcpy(relationchange_parametetr->lpcode, objItem->valuestring); 
        }
        else                //票
        {
            if(0 > get_final_url(pValue,objItem->valuestring,final_url))
                break;
            strcpy(relationchange_parametetr->lpcode,final_url); 
        }

        if (!(objItemToken = cJSON_GetObjectItem(root, "access_token"))){
            sprintf((char *)errinfo_uft8, "406企业切换失败,返回数据中无token参数");
            break;
        }

        strcpy(relationchange_parametetr->lptoken, objItemToken->valuestring); 
        printf(" = %s\r\n",objItemToken->valuestring);       
        //结束
        nRet = RET_SUCCESS;
    }while (false);

    //释放内存
    if(nEtaxBufData)
        free(nEtaxBufData);
    nEtaxBufData = NULL;

    //释放内存
    if (root)
        cJSON_Delete(root);
    root = NULL;

    //释放内存
    if (jsonTpassConfig)
        cJSON_Delete(jsonTpassConfig);
    jsonTpassConfig  = NULL;

    if (uriTpassFull)
        evhttp_uri_free(uriTpassFull);

    if(pValue)
        free(pValue);

    if(hEtax->etax.user_args)
    {
        free(hEtax->etax.user_args);
        hEtax->etax.user_args = NULL;
    }

    return nRet;    
}

int etax_jiangsu_invoice_query(HETAX hEtax, HDEV hDev,void *args,char *errinfo_uft8)
{
    struct evhttp_uri *uriTpassFull = NULL;

    INVOICE_QUERY *invoice_query_parametetr = (INVOICE_QUERY *)args;

    int nRet = ERR_GENERIC;
    int nChildRet = ERR_GENERIC;
    int nCount,i;

    char *pValue = NULL;
    char *nEtaxBufData = NULL;
    char szUserId[BS_LITTLE] = {0};
    char szBuf[BS_BLOCK * 6] = {0};
    char final_url[512] = {0x00};
    char *szTPassConfigJson = NULL;

    cJSON *jsonTpassConfig = NULL;
    cJSON *root = NULL, *objItem = NULL, *objItemChild = NULL;
    cJSON *objItemChild2 = NULL,*objItemToken  = NULL;

    if(NULL == invoice_query_parametetr)
    {
        return nRet;
    }

    hEtax->etax.user_args = (void *)(&header_array_jiangsu);
    memset(hEtax->etax.modelTaAddr.szTaxAuthorityURL,0x00,sizeof(hEtax->etax.modelTaAddr.szTaxAuthorityURL));
    strcpy(hEtax->etax.modelTaAddr.szTaxAuthorityURL,"https://dppt.jiangsu.chinatax.gov.cn:8443");
    memset(hEtax->etax.modelTaAddr.szInitRequestPage,0x00,sizeof(hEtax->etax.modelTaAddr.szInitRequestPage));
    strcpy(hEtax->etax.modelTaAddr.szInitRequestPage,"/szzhzz/spHandler?cdlj=digital-tax-account");    

    do{
        //步骤1 使用etax地址获取tpass地址
        if (nChildRet = EtaxHTTPInit(&hEtax->etax, hDev)) {
            sprintf((char *)errinfo_uft8, "306Steps 1:HTTP 初始化失败, nRet:%d", nChildRet);
            break;
        }
        if (nChildRet = EtaxHTTPOpen(&hEtax->etax)) {
            sprintf((char *)errinfo_uft8, "306Steps 1:HTTP 连接失败, nRet:%d", nChildRet);
            break;
        }
        if (!(hEtax->etax.ctxApp = calloc(1, sizeof(struct ContextHttpAppBridge)))){
            sprintf((char *)errinfo_uft8, "306内存申请失败,空间不足");
            break;
        }
        hEtax->etax.cbHttpClose = EtaxBridgeClose;
        //获取tpass地址
        //步骤3 从token服务器获取登陆凭证去登陆Etax,登陆完成后返回的用户列表在hiTPass->bufHttpRep中
        if (hEtax->etax.ctxCon.nHttpRepCode != 200 && hEtax->etax.ctxCon.nHttpRepCode != 302){
            sprintf((char *)errinfo_uft8, "406请求跳转地址失败,HTTP Code:%d",hEtax->etax.ctxCon.nHttpRepCode);  
            break;
        }
        if(hEtax->etax.ctxCon.nHttpRepCode == 200)
        {
            szTPassConfigJson = (char *)evbuffer_pullup(hEtax->etax.bufHttpRep, -1);
        }
        else if(hEtax->etax.ctxCon.nHttpRepCode == 302)
        {
            szTPassConfigJson = (char *)hEtax->etax.user_args;
        }
        printf("szTPassConfigJson=%s\r\n",szTPassConfigJson);
        //获取跳转tpass地址
        if(RET_SUCCESS != get_tpass_addr(szTPassConfigJson,&pValue,"1"))
        {
             sprintf((char *)errinfo_uft8, "406获取跳转地址错误,地址解析失败");
             break;
        }

        if (!(uriTpassFull = evhttp_uri_parse(pValue))){
            sprintf((char *)errinfo_uft8, "406获取跳转地址错误,地址转换失败"); 
            break;
        }

        if (!FillUploadAddrModelDynamic(TAADDR_ETAX_TPASS, hDev->szRegCode, pValue, "/",&hEtax->tpass.modelTaAddr)) {
            sprintf((char *)errinfo_uft8, "406获取跳转地址错误,地址解析失败");
            break;
        } 

        if (nChildRet = EtaxHTTPInit(&hEtax->tpass, hDev)) {
            sprintf((char *)errinfo_uft8, "306Steps 2:HTTP 初始化失败, nRet:%d", nChildRet);
            break;
        }

        if (nChildRet = EtaxHTTPOpen(&hEtax->tpass)) {
            sprintf((char *)errinfo_uft8, "306Steps 2:HTTP 连接失败, nRet:%d", nChildRet);
            break;
        }
        if (!(hEtax->tpass.ctxApp = calloc(1, sizeof(struct ContextHttpAppBridge)))){
            sprintf((char *)errinfo_uft8, "306内存申请失败,空间不足");
            break;
        }
        hEtax->tpass.cbHttpClose = EtaxBridgeClose;
        
        //生成SM4密钥、获取公钥、发送SM4密钥
        if ((nChildRet = TPassLoginAuthSm4AndPublickey(&hEtax->tpass,hDev->szRegCode))) {
            sprintf((char *)errinfo_uft8, "406请求公钥或者发送SM4密钥失败:%s",(const char *)evbuffer_pullup(hEtax->tpass.bufHttpRep, -1));
            break;
        }

        //获取税盘证书与税局地址进行签名验证
        if ((nChildRet = TPassLoginAuthHelloAndAuth(&hEtax->tpass, uriTpassFull))) {
            sprintf((char *)errinfo_uft8, "406签名验签服务器响应请求错误:%s",(const char *)evbuffer_pullup(hEtax->tpass.bufHttpRep, -1));
            break;
        }

        //步骤4 根据签名验证请求后返回的用户信息以及中台传入的用户信息进行比较，筛选出需要登陆的身份
        if (!(root = cJSON_Parse((const char *)evbuffer_pullup(hEtax->tpass.bufHttpRep, -1)))) {
            sprintf((char *)errinfo_uft8, "406登陆凭证中返回用户列表为空");
            break;
        }
        if (!(objItem = cJSON_GetObjectItem(root, "userList"))) {
            sprintf((char *)errinfo_uft8, "406登陆凭证中返回用户列表解析失败");
            break;
        }
        nCount = cJSON_GetArraySize(objItem);
        for (i = 0; i < nCount; i++) {
            objItemChild = cJSON_GetArrayItem(objItem, i);

            if(strlen(invoice_query_parametetr->idcard)){
                if (!(objItemChild2 = cJSON_GetObjectItem(objItemChild, "id_card")))
                    continue;
                if (strcmp(objItemChild2->valuestring, invoice_query_parametetr->idcard))
                    continue;
            }

            if(strlen(invoice_query_parametetr->mobile)){
                if (!(objItemChild2 = cJSON_GetObjectItem(objItemChild, "mobile")))
                    continue;
                if (strcmp(objItemChild2->valuestring, invoice_query_parametetr->mobile))
                    continue;
            }

            if(invoice_query_parametetr->realationtype == 0){         //是否需要切换企业
                if (!(objItemChild2 = cJSON_GetObjectItem(objItemChild, "relatedType")))
                    continue;
                if (strcmp(objItemChild2->valuestring, invoice_query_parametetr->related_type))
                    continue;
            }

            if (!(objItemChild2 = cJSON_GetObjectItem(objItemChild, "user_id")))
                continue;
            strcpy(szUserId, objItemChild2->valuestring);
            printf("szUserId=%s,sz_password=%s\r\n",szUserId,invoice_query_parametetr->sz_password);
            break;
        }
        if(!strlen(szUserId)){
            sprintf((char *)errinfo_uft8, "306用户列表中未检测到该用户信息,请核实登陆信息是否有误");
            break;
        }

        if (nChildRet = TPassLoginPasswordLoginDo(&hEtax->tpass, szUserId, invoice_query_parametetr->sz_password, szBuf,sizeof(szBuf))){
            if (!(root = cJSON_Parse(szBuf)))
                break;
            if (!(objItem = cJSON_GetObjectItem(root, "msg")))
                break;
            sprintf((char *)errinfo_uft8, "406登陆失败:%s",objItem->valuestring);
            break;
        }

        //步骤5 根据筛选出来的身份进行登陆
        if(invoice_query_parametetr->realationtype == 1){         //是否需要切换企业后查询
            if (nChildRet = TPassSelectRelationchange(&hEtax->tpass,invoice_query_parametetr->uniqueIdentity,invoice_query_parametetr->realationStatus, szBuf,sizeof(szBuf))){
                if (!(root = cJSON_Parse(szBuf)))
                    break;
                if (!(objItem = cJSON_GetObjectItem(root, "msg")))
                    break;
                sprintf((char *)errinfo_uft8, "406企业切换失败,%s",objItem->valuestring);
                break;
            } 
        }


        //步骤7 获取一键登陆需要的code参数和RPA需要的token参数
        printf("TPassSelectRelationchange data is %s\n", szBuf);
        if (!(root = cJSON_Parse(szBuf))){
            sprintf((char *)errinfo_uft8, "406企业切换失败,返回数据解析失败");
            break;
        }
        if (!(objItem = cJSON_GetObjectItem(root, "code"))){
            sprintf((char *)errinfo_uft8, "406企业切换失败,返回数据中无登陆参数");
            break;
        }

        printf(" = %s\r\n",objItem->valuestring);

        if(0 > get_final_url(pValue,objItem->valuestring,final_url))
            break;

        DpptJiangSuConnect(&hEtax->dppt,final_url,hDev);

        if (!(nChildRet = InitSecurityConfig(&hEtax->dppt))) {
        }
        if(nChildRet){
            sprintf((char *)errinfo_uft8, "406初始化dppt服务器失败,错误状态为:%d", nChildRet);    
            break;
        }

        //步骤7 发票查询(列表)
        if ((nChildRet = EtaxQueryInvoice(hEtax, "/szzhzz/qlfpcx/v1/queryFpjcxx",invoice_query_parametetr->fpcxsj)) < 0){
            sprintf((char *)errinfo_uft8, "406查询接口失败:%s\r\n",(const char *)evbuffer_pullup(hEtax->dppt.bufHttpRep, -1));
            break;                      
        }  
        nChildRet = evbuffer_get_length(hEtax->dppt.bufHttpRep);

        nEtaxBufData = (char *)malloc(nChildRet);
        if(nEtaxBufData == NULL)
            break;

        *(invoice_query_parametetr->fpcxdata) = NULL;
        *(invoice_query_parametetr->fpcxdata) = (char *)malloc(nChildRet);
        if(*(invoice_query_parametetr->fpcxdata) == NULL){    
            break;
        }
        evbuffer_copyout_from(hEtax->dppt.bufHttpRep,NULL, nEtaxBufData,nChildRet);
        EncordingConvert( "utf-8","gbk",nEtaxBufData, nChildRet, *(invoice_query_parametetr->fpcxdata), nChildRet);


        //结束
        nRet = RET_SUCCESS;
    }while (false);

    //释放内存
    if(nEtaxBufData)
        free(nEtaxBufData);
    nEtaxBufData = NULL;

    //释放内存
    if (root)
        cJSON_Delete(root);
    root = NULL;

    //释放内存
    if (jsonTpassConfig)
        cJSON_Delete(jsonTpassConfig);
    jsonTpassConfig  = NULL;

    if (uriTpassFull)
        evhttp_uri_free(uriTpassFull);

    if(pValue)
        free(pValue);

    if(hEtax->etax.user_args)
    {
        free(hEtax->etax.user_args);
        hEtax->etax.user_args = NULL;
    }

    return nRet; 
}

int etax_jiangsu_invoice_query_detail(HETAX hEtax, HDEV hDev,void *args,char *errinfo_uft8)
{
    struct evhttp_uri *uriTpassFull = NULL;

    INVOICE_QUERY_DETAIL *invoice_query_detail_parametetr = (INVOICE_QUERY_DETAIL *)args;

    int nRet = ERR_GENERIC;
    int nChildRet = ERR_GENERIC;
    int nCount,i;

    char *pValue = NULL;
    char *nEtaxBufData = NULL;
    char szUserId[BS_LITTLE] = {0};
    char szBuf[BS_BLOCK * 6] = {0};
    char final_url[512] = {0x00};
    char *szTPassConfigJson = NULL;

    cJSON *jsonTpassConfig = NULL;
    cJSON *root = NULL, *objItem = NULL, *objItemChild = NULL;
    cJSON *objItemChild2 = NULL,*objItemToken  = NULL;

    if(NULL == invoice_query_detail_parametetr)
    {
        return nRet;
    }

    hEtax->etax.user_args = (void *)(&header_array_jiangsu);
    memset(hEtax->etax.modelTaAddr.szTaxAuthorityURL,0x00,sizeof(hEtax->etax.modelTaAddr.szTaxAuthorityURL));
    strcpy(hEtax->etax.modelTaAddr.szTaxAuthorityURL,"https://dppt.jiangsu.chinatax.gov.cn:8443");
    memset(hEtax->etax.modelTaAddr.szInitRequestPage,0x00,sizeof(hEtax->etax.modelTaAddr.szInitRequestPage));
    strcpy(hEtax->etax.modelTaAddr.szInitRequestPage,"/szzhzz/spHandler?cdlj=digital-tax-account");

    do{
        //步骤1 使用etax地址获取tpass地址
        if (nChildRet = EtaxHTTPInit(&hEtax->etax, hDev)) {
            sprintf((char *)errinfo_uft8, "306Steps 1:HTTP 初始化失败, nRet:%d", nChildRet);
            break;
        }
        if (nChildRet = EtaxHTTPOpen(&hEtax->etax)) {
            sprintf((char *)errinfo_uft8, "306Steps 1:HTTP 连接失败, nRet:%d", nChildRet);
            break;
        }
        if (!(hEtax->etax.ctxApp = calloc(1, sizeof(struct ContextHttpAppBridge)))){
            sprintf((char *)errinfo_uft8, "306内存申请失败,空间不足");
            break;
        }
        hEtax->etax.cbHttpClose = EtaxBridgeClose;
        //获取tpass地址
        //步骤3 从token服务器获取登陆凭证去登陆Etax,登陆完成后返回的用户列表在hiTPass->bufHttpRep中
        if (hEtax->etax.ctxCon.nHttpRepCode != 200 && hEtax->etax.ctxCon.nHttpRepCode != 302){
            sprintf((char *)errinfo_uft8, "406请求跳转地址失败,HTTP Code:%d",hEtax->etax.ctxCon.nHttpRepCode);  
            break;
        }
        if(hEtax->etax.ctxCon.nHttpRepCode == 200)
        {
            szTPassConfigJson = (char *)evbuffer_pullup(hEtax->etax.bufHttpRep, -1);
        }
        else if(hEtax->etax.ctxCon.nHttpRepCode == 302)
        {
            szTPassConfigJson = (char *)hEtax->etax.user_args;
        }
        //获取跳转tpass地址
        if(RET_SUCCESS != get_tpass_addr(szTPassConfigJson,&pValue,"1"))
        {
             sprintf((char *)errinfo_uft8, "406获取跳转地址错误,地址解析失败");
             break;
        }

        if (!(uriTpassFull = evhttp_uri_parse(pValue))){
            sprintf((char *)errinfo_uft8, "406获取跳转地址错误,地址转换失败"); 
            break;
        }

        if (!FillUploadAddrModelDynamic(TAADDR_ETAX_TPASS, hDev->szRegCode, pValue, "/",&hEtax->tpass.modelTaAddr)) {
            sprintf((char *)errinfo_uft8, "406获取跳转地址错误,地址解析失败");
            break;
        } 

        if (nChildRet = EtaxHTTPInit(&hEtax->tpass, hDev)) {
            sprintf((char *)errinfo_uft8, "306Steps 2:HTTP 初始化失败, nRet:%d", nChildRet);
            break;
        }

        if (nChildRet = EtaxHTTPOpen(&hEtax->tpass)) {
            sprintf((char *)errinfo_uft8, "306Steps 2:HTTP 连接失败, nRet:%d", nChildRet);
            break;
        }
        if (!(hEtax->tpass.ctxApp = calloc(1, sizeof(struct ContextHttpAppBridge)))){
            sprintf((char *)errinfo_uft8, "306内存申请失败,空间不足");
            break;
        }
        hEtax->tpass.cbHttpClose = EtaxBridgeClose;
        
        //生成SM4密钥、获取公钥、发送SM4密钥
        if ((nChildRet = TPassLoginAuthSm4AndPublickey(&hEtax->tpass,hDev->szRegCode))) {
            sprintf((char *)errinfo_uft8, "406请求公钥或者发送SM4密钥失败:%s",(const char *)evbuffer_pullup(hEtax->tpass.bufHttpRep, -1));
            break;
        }

        //获取税盘证书与税局地址进行签名验证
        if ((nChildRet = TPassLoginAuthHelloAndAuth(&hEtax->tpass, uriTpassFull))) {
            sprintf((char *)errinfo_uft8, "406签名验签服务器响应请求错误:%s",(const char *)evbuffer_pullup(hEtax->tpass.bufHttpRep, -1));
            break;
        }

        //步骤4 根据签名验证请求后返回的用户信息以及中台传入的用户信息进行比较，筛选出需要登陆的身份
        if (!(root = cJSON_Parse((const char *)evbuffer_pullup(hEtax->tpass.bufHttpRep, -1)))) {
            sprintf((char *)errinfo_uft8, "406登陆凭证中返回用户列表为空");
            break;
        }
        if (!(objItem = cJSON_GetObjectItem(root, "userList"))) {
            sprintf((char *)errinfo_uft8, "406登陆凭证中返回用户列表解析失败");
            break;
        }
        nCount = cJSON_GetArraySize(objItem);
        for (i = 0; i < nCount; i++) {
            objItemChild = cJSON_GetArrayItem(objItem, i);

            if(strlen(invoice_query_detail_parametetr->idcard)){
                if (!(objItemChild2 = cJSON_GetObjectItem(objItemChild, "id_card")))
                    continue;
                if (strcmp(objItemChild2->valuestring, invoice_query_detail_parametetr->idcard))
                    continue;
            }

            if(strlen(invoice_query_detail_parametetr->mobile)){
                if (!(objItemChild2 = cJSON_GetObjectItem(objItemChild, "mobile")))
                    continue;
                if (strcmp(objItemChild2->valuestring, invoice_query_detail_parametetr->mobile))
                    continue;
            }

            if(invoice_query_detail_parametetr->realationtype == 0){         //是否需要切换企业
                if (!(objItemChild2 = cJSON_GetObjectItem(objItemChild, "relatedType")))
                    continue;
                if (strcmp(objItemChild2->valuestring, invoice_query_detail_parametetr->related_type))
                    continue;
            }

            if (!(objItemChild2 = cJSON_GetObjectItem(objItemChild, "user_id")))
                continue;
            strcpy(szUserId, objItemChild2->valuestring);
            printf("szUserId=%s,sz_password=%s\r\n",szUserId,invoice_query_detail_parametetr->sz_password);
            break;
        }
        if(!strlen(szUserId)){
            sprintf((char *)errinfo_uft8, "306用户列表中未检测到该用户信息,请核实登陆信息是否有误");
            break;
        }

        if (nChildRet = TPassLoginPasswordLoginDo(&hEtax->tpass, szUserId, invoice_query_detail_parametetr->sz_password, szBuf,sizeof(szBuf))){
            if (!(root = cJSON_Parse(szBuf)))
                break;
            if (!(objItem = cJSON_GetObjectItem(root, "msg")))
                break;
            sprintf((char *)errinfo_uft8, "406登陆失败:%s",objItem->valuestring);
            break;
        }

        //步骤5 根据筛选出来的身份进行登陆
        if(invoice_query_detail_parametetr->realationtype == 1){         //是否需要切换企业后查询
            if (nChildRet = TPassSelectRelationchange(&hEtax->tpass,invoice_query_detail_parametetr->uniqueIdentity,invoice_query_detail_parametetr->realationStatus, szBuf,sizeof(szBuf))){
                if (!(root = cJSON_Parse(szBuf)))
                    break;
                if (!(objItem = cJSON_GetObjectItem(root, "msg")))
                    break;
                sprintf((char *)errinfo_uft8, "406企业切换失败,%s",objItem->valuestring);
                break;
            } 
        }


        //步骤7 获取一键登陆需要的code参数和RPA需要的token参数
        printf("TPassSelectRelationchange data is %s\n", szBuf);
        if (!(root = cJSON_Parse(szBuf))){
            sprintf((char *)errinfo_uft8, "406企业切换失败,返回数据解析失败");
            break;
        }
        if (!(objItem = cJSON_GetObjectItem(root, "code"))){
            sprintf((char *)errinfo_uft8, "406企业切换失败,返回数据中无登陆参数");
            break;
        }

        printf(" = %s\r\n",objItem->valuestring);

        if(0 > get_final_url(pValue,objItem->valuestring,final_url))
            break;

        printf("final_url=%s\r\n",final_url);

        DpptJiangSuConnect(&hEtax->dppt,final_url,hDev);

        if (!(nChildRet = InitSecurityConfig(&hEtax->dppt))) {

        }
        if(nChildRet){
            sprintf((char *)errinfo_uft8, "406初始化dppt服务器失败,错误状态为:%d", nChildRet);    
            break;
        }

       //步骤7 发票查询(详情)
        if ((nChildRet = EtaxQueryInvoice(hEtax, "/szzhzz/fppmcx/v1/queryFppmxx",invoice_query_detail_parametetr->fpcxsj)) < 0){
            sprintf((char *)errinfo_uft8, "406发票查询接口失败:%s",evbuffer_pullup(hEtax->dppt.bufHttpRep, -1));
            break;                      
        }  
        nChildRet = evbuffer_get_length(hEtax->dppt.bufHttpRep);
        printf("nChildRet = %d\r\n,%s\r\n",nChildRet,evbuffer_pullup(hEtax->dppt.bufHttpRep, -1));

        nEtaxBufData = (char *)malloc(nChildRet);
        if(nEtaxBufData == NULL)
            break;

        *(invoice_query_detail_parametetr->fpcxdata) = NULL;
        *(invoice_query_detail_parametetr->fpcxdata) = (char *)malloc(nChildRet);
        if(*(invoice_query_detail_parametetr->fpcxdata) == NULL){    
            break;
        }
        evbuffer_copyout_from(hEtax->dppt.bufHttpRep,NULL, nEtaxBufData,nChildRet);
        EncordingConvert( "utf-8","gbk",nEtaxBufData, nChildRet, *(invoice_query_detail_parametetr->fpcxdata), nChildRet);

        //结束
        nRet = RET_SUCCESS;
    }while (false);

    //释放内存
    if(nEtaxBufData)
        free(nEtaxBufData);
    nEtaxBufData = NULL;

    //释放内存
    if (root)
        cJSON_Delete(root);
    root = NULL;

    //释放内存
    if (jsonTpassConfig)
        cJSON_Delete(jsonTpassConfig);
    jsonTpassConfig  = NULL;

    if (uriTpassFull)
        evhttp_uri_free(uriTpassFull);

    if(pValue)
        free(pValue);

    if(hEtax->etax.user_args)
    {
        free(hEtax->etax.user_args);
        hEtax->etax.user_args = NULL;
    }

    return nRet; 
}

int etax_jiangsu_invoice_query_down(HETAX hEtax, HDEV hDev,void *args,char *errinfo_uft8)
{
    struct evhttp_uri *uriTpassFull = NULL;

    INVOICE_QUERY_DOWN *invoice_query_down_parametetr = (INVOICE_QUERY_DOWN *)args;

    int nRet = ERR_GENERIC;
    int nChildRet = ERR_GENERIC;
    int nCount,i;

    char *pValue = NULL;
    char *nEtaxBufData = NULL;
    char szUserId[BS_LITTLE] = {0};
    char szBuf[BS_BLOCK * 6] = {0};
    char final_url[512] = {0x00};
    char *szTPassConfigJson = NULL;

    cJSON *jsonTpassConfig = NULL;
    cJSON *root = NULL, *objItem = NULL, *objItemChild = NULL;
    cJSON *objItemChild2 = NULL,*objItemToken  = NULL;

    if(NULL == invoice_query_down_parametetr)
    {
        return nRet;
    }


    hEtax->etax.user_args = (void *)(&header_array_jiangsu);
    memset(hEtax->etax.modelTaAddr.szTaxAuthorityURL,0x00,sizeof(hEtax->etax.modelTaAddr.szTaxAuthorityURL));
    strcpy(hEtax->etax.modelTaAddr.szTaxAuthorityURL,"https://dppt.jiangsu.chinatax.gov.cn:8443");
    memset(hEtax->etax.modelTaAddr.szInitRequestPage,0x00,sizeof(hEtax->etax.modelTaAddr.szInitRequestPage));
    strcpy(hEtax->etax.modelTaAddr.szInitRequestPage,"/szzhzz/spHandler?cdlj=digital-tax-account");

    do{
        //步骤1 使用etax地址获取tpass地址
        if (nChildRet = EtaxHTTPInit(&hEtax->etax, hDev)) {
            sprintf((char *)errinfo_uft8, "306Steps 1:HTTP 初始化失败, nRet:%d", nChildRet);
            break;
        }
        if (nChildRet = EtaxHTTPOpen(&hEtax->etax)) {
            sprintf((char *)errinfo_uft8, "306Steps 1:HTTP 连接失败, nRet:%d", nChildRet);
            break;
        }
        if (!(hEtax->etax.ctxApp = calloc(1, sizeof(struct ContextHttpAppBridge)))){
            sprintf((char *)errinfo_uft8, "306内存申请失败,空间不足");
            break;
        }
        hEtax->etax.cbHttpClose = EtaxBridgeClose;
        //获取tpass地址
        //步骤3 从token服务器获取登陆凭证去登陆Etax,登陆完成后返回的用户列表在hiTPass->bufHttpRep中
        if (hEtax->etax.ctxCon.nHttpRepCode != 200 && hEtax->etax.ctxCon.nHttpRepCode != 302){
            sprintf((char *)errinfo_uft8, "406请求跳转地址失败,HTTP Code:%d",hEtax->etax.ctxCon.nHttpRepCode);  
            break;
        }
        if(hEtax->etax.ctxCon.nHttpRepCode == 200)
        {
            szTPassConfigJson = (char *)evbuffer_pullup(hEtax->etax.bufHttpRep, -1);
        }
        else if(hEtax->etax.ctxCon.nHttpRepCode == 302)
        {
            szTPassConfigJson = (char *)hEtax->etax.user_args;
        }
        //获取跳转tpass地址
        if(RET_SUCCESS != get_tpass_addr(szTPassConfigJson,&pValue,"1"))
        {
             sprintf((char *)errinfo_uft8, "406获取跳转地址错误,地址解析失败");
             break;
        }

        if (!(uriTpassFull = evhttp_uri_parse(pValue))){
            sprintf((char *)errinfo_uft8, "406获取跳转地址错误,地址转换失败"); 
            break;
        }

        if (!FillUploadAddrModelDynamic(TAADDR_ETAX_TPASS, hDev->szRegCode, pValue, "/",&hEtax->tpass.modelTaAddr)) {
            sprintf((char *)errinfo_uft8, "406获取跳转地址错误,地址解析失败");
            break;
        } 

        if (nChildRet = EtaxHTTPInit(&hEtax->tpass, hDev)) {
            sprintf((char *)errinfo_uft8, "306Steps 2:HTTP 初始化失败, nRet:%d", nChildRet);
            break;
        }

        if (nChildRet = EtaxHTTPOpen(&hEtax->tpass)) {
            sprintf((char *)errinfo_uft8, "306Steps 2:HTTP 连接失败, nRet:%d", nChildRet);
            break;
        }
        if (!(hEtax->tpass.ctxApp = calloc(1, sizeof(struct ContextHttpAppBridge)))){
            sprintf((char *)errinfo_uft8, "306内存申请失败,空间不足");
            break;
        }
        hEtax->tpass.cbHttpClose = EtaxBridgeClose;
        
        //生成SM4密钥、获取公钥、发送SM4密钥
        if ((nChildRet = TPassLoginAuthSm4AndPublickey(&hEtax->tpass,hDev->szRegCode))) {
            sprintf((char *)errinfo_uft8, "406请求公钥或者发送SM4密钥失败:%s",(const char *)evbuffer_pullup(hEtax->tpass.bufHttpRep, -1));
            break;
        }

        //获取税盘证书与税局地址进行签名验证
        if ((nChildRet = TPassLoginAuthHelloAndAuth(&hEtax->tpass, uriTpassFull))) {
            sprintf((char *)errinfo_uft8, "406签名验签服务器响应请求错误:%s",(const char *)evbuffer_pullup(hEtax->tpass.bufHttpRep, -1));
            break;
        }

        //步骤4 根据签名验证请求后返回的用户信息以及中台传入的用户信息进行比较，筛选出需要登陆的身份
        if (!(root = cJSON_Parse((const char *)evbuffer_pullup(hEtax->tpass.bufHttpRep, -1)))) {
            sprintf((char *)errinfo_uft8, "406登陆凭证中返回用户列表为空");
            break;
        }
        if (!(objItem = cJSON_GetObjectItem(root, "userList"))) {
            sprintf((char *)errinfo_uft8, "406登陆凭证中返回用户列表解析失败");
            break;
        }
        nCount = cJSON_GetArraySize(objItem);
        for (i = 0; i < nCount; i++) {
            objItemChild = cJSON_GetArrayItem(objItem, i);

            if(strlen(invoice_query_down_parametetr->idcard)){
                if (!(objItemChild2 = cJSON_GetObjectItem(objItemChild, "id_card")))
                    continue;
                if (strcmp(objItemChild2->valuestring, invoice_query_down_parametetr->idcard))
                    continue;
            }

            if(strlen(invoice_query_down_parametetr->mobile)){
                if (!(objItemChild2 = cJSON_GetObjectItem(objItemChild, "mobile")))
                    continue;
                if (strcmp(objItemChild2->valuestring, invoice_query_down_parametetr->mobile))
                    continue;
            }

            if(invoice_query_down_parametetr->realationtype == 0){  
                if (!(objItemChild2 = cJSON_GetObjectItem(objItemChild, "relatedType")))
                    continue;
                if (strcmp(objItemChild2->valuestring, invoice_query_down_parametetr->related_type))
                    continue;
            }

            if (!(objItemChild2 = cJSON_GetObjectItem(objItemChild, "user_id")))
                continue;
            strcpy(szUserId, objItemChild2->valuestring);
            printf("szUserId=%s,sz_password=%s\r\n",szUserId,invoice_query_down_parametetr->sz_password);
            break;
        }
        if(!strlen(szUserId)){
            sprintf((char *)errinfo_uft8, "306用户列表中未检测到该用户信息,请核实登陆信息是否有误");
            break;
        }

        if (nChildRet = TPassLoginPasswordLoginDo(&hEtax->tpass, szUserId, invoice_query_down_parametetr->sz_password, szBuf,sizeof(szBuf))){
            if (!(root = cJSON_Parse(szBuf)))
                break;
            if (!(objItem = cJSON_GetObjectItem(root, "msg")))
                break;
            sprintf((char *)errinfo_uft8, "406登陆失败:%s",objItem->valuestring);
            break;
        }

        //步骤5 根据筛选出来的身份进行登陆
        if(invoice_query_down_parametetr->realationtype == 1){         //是否需要切换企业后查询
            if (nChildRet = TPassSelectRelationchange(&hEtax->tpass,invoice_query_down_parametetr->uniqueIdentity,invoice_query_down_parametetr->realationStatus, szBuf,sizeof(szBuf))){
                if (!(root = cJSON_Parse(szBuf)))
                    break;
                if (!(objItem = cJSON_GetObjectItem(root, "msg")))
                    break;
                sprintf((char *)errinfo_uft8, "406企业切换失败,%s",objItem->valuestring);
                break;
            } 
        }


        //步骤7 获取一键登陆需要的code参数和RPA需要的token参数
        printf("TPassSelectRelationchange data is %s\n", szBuf);
        if (!(root = cJSON_Parse(szBuf))){
            sprintf((char *)errinfo_uft8, "406企业切换失败,返回数据解析失败");
            break;
        }
        if (!(objItem = cJSON_GetObjectItem(root, "code"))){
            sprintf((char *)errinfo_uft8, "406企业切换失败,返回数据中无登陆参数");
            break;
        }

        printf(" = %s\r\n",objItem->valuestring);

        if(0 > get_final_url(pValue,objItem->valuestring,final_url))
            break;

        printf("final_url=%s\r\n",final_url);

        DpptJiangSuConnect(&hEtax->dppt,final_url,hDev);

        if (!(nChildRet = InitSecurityConfig(&hEtax->dppt))) {

        }
        if(nChildRet){
            sprintf((char *)errinfo_uft8, "406初始化dppt服务器失败,错误状态为:%d", nChildRet);    
            break;
        }

       //步骤7 发票查询(下载)
        char fpcxsjtime[30];
        struct timeval start;
        gettimeofday(&start, NULL);
        uint64 timeStamp = 1000 * (uint64)start.tv_sec + start.tv_usec / 1000;
        sprintf(fpcxsjtime,"&timeStampId=%llu",timeStamp);
        strcat(invoice_query_down_parametetr->fpcxsj,fpcxsjtime);

        if ((nChildRet = EtaxQueryInvoicefpdown(hEtax,invoice_query_down_parametetr->fpcxsj)) < 0){
            sprintf((char *)errinfo_uft8, "406接口下载失败:%s\r\n",(const char *)evbuffer_pullup(hEtax->dppt.bufHttpRep, -1));
            break;                      
        }  
        nChildRet = evbuffer_get_length(hEtax->dppt.bufHttpRep);
        
        printf("nChildRet = %d\r\n,%s\r\n",nChildRet,evbuffer_pullup(hEtax->dppt.bufHttpRep, -1));

        *(invoice_query_down_parametetr->fpcxdata) = NULL;
        *(invoice_query_down_parametetr->fpcxdata) = (char *)malloc(nChildRet + (1024 * 600));
        if(*(invoice_query_down_parametetr->fpcxdata) == NULL) 
            break;

        int json_zlib_len = nChildRet + (1024 * 600);

        Base64_Encode(evbuffer_pullup(hEtax->dppt.bufHttpRep, -1),nChildRet,*(invoice_query_down_parametetr->fpcxdata));
        
        //asc_compress_base64(evbuffer_pullup(hEtax->dppt.bufHttpRep, -1), nChildRet, *(invoice_query_down_parametetr->fpcxdata), &json_zlib_len);

        *(invoice_query_down_parametetr->fpcxname) = NULL;
        *(invoice_query_down_parametetr->fpcxname) = (char *)malloc(128);
        if(*(invoice_query_down_parametetr->fpcxname) == NULL) 
            break;        


        //dzfp_$发票号码_$开票时间.pdf
        memset(*(invoice_query_down_parametetr->fpcxname),0,128);
        strcpy(*(invoice_query_down_parametetr->fpcxname),"dzfp_");

        char *ptr_start = NULL;
        if(NULL == (ptr_start = strstr(invoice_query_down_parametetr->fpcxsj,"Fphm=")))
        {
            _WriteLog(LL_WARN, "Get server Fphm failed");
            break;
        }
        memcpy(*(invoice_query_down_parametetr->fpcxname)+5,ptr_start+strlen("Fphm="),20);
        strcat(*(invoice_query_down_parametetr->fpcxname),"_");

        if(NULL == (ptr_start = strstr(invoice_query_down_parametetr->fpcxsj,"Kprq=")))
        {
            _WriteLog(LL_WARN, "Get server Kprq failed");
            break;
        }
        memcpy(*(invoice_query_down_parametetr->fpcxname)+26,ptr_start+strlen("Kprq="),14);
        strcat(*(invoice_query_down_parametetr->fpcxname),".");

        if(NULL == (ptr_start = strstr(invoice_query_down_parametetr->fpcxsj,"PDF")))
        {
            if(NULL == (ptr_start = strstr(invoice_query_down_parametetr->fpcxsj,"XML")))
            {
                if(NULL == (ptr_start = strstr(invoice_query_down_parametetr->fpcxsj,"OFD")))
                {
                    _WriteLog(LL_WARN, "Get server XZLX failed");
                    break;                    
                }
                else 
                {
                    strcat(*(invoice_query_down_parametetr->fpcxname),"ofd");
                }
            }
            else 
            {
                strcat(*(invoice_query_down_parametetr->fpcxname),"zip");
            }
        }
        else 
        {
            strcat(*(invoice_query_down_parametetr->fpcxname),"pdf");
        }
        printf("fpcxname3=%s\r\n",*(invoice_query_down_parametetr->fpcxname));
        //结束
        nRet = RET_SUCCESS;
    }while (false);

    //释放内存
    if(nEtaxBufData)
        free(nEtaxBufData);
    nEtaxBufData = NULL;

    //释放内存
    if (root)
        cJSON_Delete(root);
    root = NULL;

    //释放内存
    if (jsonTpassConfig)
        cJSON_Delete(jsonTpassConfig);
    jsonTpassConfig  = NULL;

    if (uriTpassFull)
        evhttp_uri_free(uriTpassFull);

    if(pValue)
        free(pValue);

    if(hEtax->etax.user_args)
    {
        free(hEtax->etax.user_args);
        hEtax->etax.user_args = NULL;
    }

    return nRet; 
}

int etax_jiangsu_rpa_invoice_issuance(HETAX hEtax, HDEV hDev,void *args,char *errinfo_uft8)
{
    struct evhttp_uri *uriTpassFull = NULL;

    RPA_INVOICE_ISSUANCE *rpa_invoice_issuance_parametetr = (RPA_INVOICE_ISSUANCE *)args;

    int nRet = ERR_GENERIC;
    int nChildRet = ERR_GENERIC;
    int nCount,i;

    char *pValue = NULL;
    char *nEtaxBufData = NULL;
    char szUserId[BS_LITTLE] = {0};
    char szBuf[BS_BLOCK * 6] = {0};
    char final_url[512] = {0x00};
    char nsrsbhbuf[BS_NORMAL]={0};
    char *szTPassConfigJson = NULL;

    cJSON *jsonTpassConfig = NULL;
    cJSON *root = NULL, *objItem = NULL, *objItemChild = NULL;
    cJSON *objItemChild2 = NULL,*objItemToken  = NULL;

    if(NULL == rpa_invoice_issuance_parametetr)
    {
        return nRet;
    }

    hEtax->etax.user_args = (void *)(&header_array_jiangsu);
    memset(hEtax->etax.modelTaAddr.szTaxAuthorityURL,0x00,sizeof(hEtax->etax.modelTaAddr.szTaxAuthorityURL));
    strcpy(hEtax->etax.modelTaAddr.szTaxAuthorityURL,"https://dppt.jiangsu.chinatax.gov.cn:8443");
    memset(hEtax->etax.modelTaAddr.szInitRequestPage,0x00,sizeof(hEtax->etax.modelTaAddr.szInitRequestPage));
    strcpy(hEtax->etax.modelTaAddr.szInitRequestPage,"/szzhzz/spHandler?cdlj=digital-tax-account");

    do{
        //步骤1 使用etax地址获取tpass地址
        if (nChildRet = EtaxHTTPInit(&hEtax->etax, hDev)) {
            sprintf((char *)errinfo_uft8, "306Steps 1:HTTP 初始化失败, nRet:%d", nChildRet);
            break;
        }
        if (nChildRet = EtaxHTTPOpen(&hEtax->etax)) {
            sprintf((char *)errinfo_uft8, "306Steps 1:HTTP 连接失败, nRet:%d", nChildRet);
            break;
        }
        if (!(hEtax->etax.ctxApp = calloc(1, sizeof(struct ContextHttpAppBridge)))){
            sprintf((char *)errinfo_uft8, "306内存申请失败,空间不足");
            break;
        }
        hEtax->etax.cbHttpClose = EtaxBridgeClose;
        //获取tpass地址
        //步骤3 从token服务器获取登陆凭证去登陆Etax,登陆完成后返回的用户列表在hiTPass->bufHttpRep中
        if (hEtax->etax.ctxCon.nHttpRepCode != 200 && hEtax->etax.ctxCon.nHttpRepCode != 302){
            sprintf((char *)errinfo_uft8, "406请求跳转地址失败,HTTP Code:%d",hEtax->etax.ctxCon.nHttpRepCode);  
            break;
        }
        if(hEtax->etax.ctxCon.nHttpRepCode == 200)
        {
            szTPassConfigJson = (char *)evbuffer_pullup(hEtax->etax.bufHttpRep, -1);
        }
        else if(hEtax->etax.ctxCon.nHttpRepCode == 302)
        {
            szTPassConfigJson = (char *)hEtax->etax.user_args;
        }
        //获取跳转tpass地址
        if(RET_SUCCESS != get_tpass_addr(szTPassConfigJson,&pValue,"1"))
        {
             sprintf((char *)errinfo_uft8, "406获取跳转地址错误,地址解析失败");
             break;
        }

        if (!(uriTpassFull = evhttp_uri_parse(pValue))){
            sprintf((char *)errinfo_uft8, "406获取跳转地址错误,地址转换失败"); 
            break;
        }

        if (!FillUploadAddrModelDynamic(TAADDR_ETAX_TPASS, hDev->szRegCode, pValue, "/",&hEtax->tpass.modelTaAddr)) {
            sprintf((char *)errinfo_uft8, "406获取跳转地址错误,地址解析失败");
            break;
        } 

        if (nChildRet = EtaxHTTPInit(&hEtax->tpass, hDev)) {
            sprintf((char *)errinfo_uft8, "306Steps 2:HTTP 初始化失败, nRet:%d", nChildRet);
            break;
        }

        if (nChildRet = EtaxHTTPOpen(&hEtax->tpass)) {
            sprintf((char *)errinfo_uft8, "306Steps 2:HTTP 连接失败, nRet:%d", nChildRet);
            break;
        }
        if (!(hEtax->tpass.ctxApp = calloc(1, sizeof(struct ContextHttpAppBridge)))){
            sprintf((char *)errinfo_uft8, "306内存申请失败,空间不足");
            break;
        }
        hEtax->tpass.cbHttpClose = EtaxBridgeClose;
        
        //生成SM4密钥、获取公钥、发送SM4密钥
        if ((nChildRet = TPassLoginAuthSm4AndPublickey(&hEtax->tpass,hDev->szRegCode))) {
            sprintf((char *)errinfo_uft8, "406请求公钥或者发送SM4密钥失败:%s",(const char *)evbuffer_pullup(hEtax->tpass.bufHttpRep, -1));
            break;
        }

        //获取税盘证书与税局地址进行签名验证
        if ((nChildRet = TPassLoginAuthHelloAndAuth(&hEtax->tpass, uriTpassFull))) {
            sprintf((char *)errinfo_uft8, "406签名验签服务器响应请求错误:%s",(const char *)evbuffer_pullup(hEtax->tpass.bufHttpRep, -1));
            break;
        }

        //步骤4 根据签名验证请求后返回的用户信息以及中台传入的用户信息进行比较，筛选出需要登陆的身份
        if (!(root = cJSON_Parse((const char *)evbuffer_pullup(hEtax->tpass.bufHttpRep, -1)))) {
            sprintf((char *)errinfo_uft8, "406登陆凭证中返回用户列表为空");
            break;
        }
        if (!(objItem = cJSON_GetObjectItem(root, "userList"))) {
            sprintf((char *)errinfo_uft8, "406登陆凭证中返回用户列表解析失败");
            break;
        }
        nCount = cJSON_GetArraySize(objItem);
        for (i = 0; i < nCount; i++) {
            objItemChild = cJSON_GetArrayItem(objItem, i);

            if(strlen(rpa_invoice_issuance_parametetr->idcard)){
                if (!(objItemChild2 = cJSON_GetObjectItem(objItemChild, "id_card")))
                    continue;
                if (strcmp(objItemChild2->valuestring, rpa_invoice_issuance_parametetr->idcard))
                    continue;
            }

            if(strlen(rpa_invoice_issuance_parametetr->mobile)){
                if (!(objItemChild2 = cJSON_GetObjectItem(objItemChild, "mobile")))
                    continue;
                if (strcmp(objItemChild2->valuestring, rpa_invoice_issuance_parametetr->mobile))
                    continue;
            }

            if(rpa_invoice_issuance_parametetr->realationtype == 0){         //是否需要切换企业
                if (!(objItemChild2 = cJSON_GetObjectItem(objItemChild, "relatedType")))
                    continue;
                if (strcmp(objItemChild2->valuestring, rpa_invoice_issuance_parametetr->related_type))
                    continue;
            }

            if (!(objItemChild2 = cJSON_GetObjectItem(objItemChild, "user_id")))
                continue;
            strcpy(szUserId, objItemChild2->valuestring);
            printf("szUserId=%s,sz_password=%s\r\n",szUserId,rpa_invoice_issuance_parametetr->sz_password);
            break;
        }
        if(!strlen(szUserId)){
            sprintf((char *)errinfo_uft8, "306用户列表中未检测到该用户信息,请核实登陆信息是否有误");
            break;
        }

        if (nChildRet = TPassLoginPasswordLoginDo(&hEtax->tpass, szUserId, rpa_invoice_issuance_parametetr->sz_password, szBuf,sizeof(szBuf))){
            if (!(root = cJSON_Parse(szBuf)))
                break;
            if (!(objItem = cJSON_GetObjectItem(root, "msg")))
                break;
            sprintf((char *)errinfo_uft8, "406登陆失败:%s",objItem->valuestring);
            break;
        }

        //步骤5 根据筛选出来的身份进行登陆
        if(rpa_invoice_issuance_parametetr->realationtype == 1){         //是否需要切换企业后查询
            if (nChildRet = TPassSelectRelationchange(&hEtax->tpass,rpa_invoice_issuance_parametetr->uniqueIdentity,rpa_invoice_issuance_parametetr->realationStatus, szBuf,sizeof(szBuf))){
                if (!(root = cJSON_Parse(szBuf)))
                    break;
                if (!(objItem = cJSON_GetObjectItem(root, "msg")))
                    break;
                sprintf((char *)errinfo_uft8, "406企业切换失败,%s",objItem->valuestring);
                break;
            } 
        }


        //步骤7 获取一键登陆需要的code参数和RPA需要的token参数
        printf("TPassSelectRelationchange data is %s\n", szBuf);
        if (!(root = cJSON_Parse(szBuf))){
            sprintf((char *)errinfo_uft8, "406企业切换失败,返回数据解析失败");
            break;
        }
        if (!(objItem = cJSON_GetObjectItem(root, "code"))){
            sprintf((char *)errinfo_uft8, "406企业切换失败,返回数据中无登陆参数");
            break;
        }

        printf(" = %s\r\n",objItem->valuestring);

        if(0 > get_final_url(pValue,objItem->valuestring,final_url))
            break;

        printf("final_url=%s\r\n",final_url);

        DpptJiangSuConnect(&hEtax->dppt,final_url,hDev);

        if (!(nChildRet = InitSecurityConfig(&hEtax->dppt))) {

        }
        if(nChildRet){
            sprintf((char *)errinfo_uft8, "406初始化dppt服务器失败,错误状态为:%d", nChildRet);    
            break;
        }

        //步骤7 发票查询(列表)
        sprintf(nsrsbhbuf,"{\"nsrsbh\":\"%s\"}",rpa_invoice_issuance_parametetr->nsrsbh);
        printf("url = %s\r\n",rpa_invoice_issuance_parametetr->fpkjurl);
        printf("nsrsbh = %s\r\n",nsrsbhbuf);
        if ((nChildRet = EtaxQueryInvoice(hEtax, rpa_invoice_issuance_parametetr->fpkjurl,nsrsbhbuf)) < 0){
            sprintf((char *)errinfo_uft8, "406查询接口失败:%s\r\n",(const char *)evbuffer_pullup(hEtax->dppt.bufHttpRep, -1));
            break;                      
        }  
        nChildRet = evbuffer_get_length(hEtax->dppt.bufHttpRep);

        printf("############  nRepSize = %d\r\n,%s\r\n",nChildRet,evbuffer_pullup(hEtax->dppt.bufHttpRep, -1));


        //nEtaxBufData = (char *)malloc(nChildRet);
        //if(nEtaxBufData == NULL)
        //    break;

        *(rpa_invoice_issuance_parametetr->fpkjdata) = NULL;
        *(rpa_invoice_issuance_parametetr->fpkjdata) = (char *)malloc(nChildRet);
        if(*(rpa_invoice_issuance_parametetr->fpkjdata) == NULL){    
            break;
        }
        evbuffer_copyout_from(hEtax->dppt.bufHttpRep,NULL, *(rpa_invoice_issuance_parametetr->fpkjdata),nChildRet);
        //EncordingConvert( "utf-8","gbk",nEtaxBufData, nChildRet, *(rpa_invoice_issuance_parametetr->fpcxdata), nChildRet);

        //结束
        nRet = RET_SUCCESS;
    }while (false);

    //释放内存
    if(nEtaxBufData)
        free(nEtaxBufData);
    nEtaxBufData = NULL;

    //释放内存
    if (root)
        cJSON_Delete(root);
    root = NULL;

    //释放内存
    if (jsonTpassConfig)
        cJSON_Delete(jsonTpassConfig);
    jsonTpassConfig  = NULL;

    if (uriTpassFull)
        evhttp_uri_free(uriTpassFull);

    if(pValue)
        free(pValue);

    if(hEtax->etax.user_args)
    {
        free(hEtax->etax.user_args);
        hEtax->etax.user_args = NULL;
    }

    return nRet; 
}

int etax_jiangsu_credit_limit(HETAX hEtax, HDEV hDev,void *args,char *errinfo_uft8)
{
    struct evhttp_uri *uriTpassFull = NULL;

    CREDIT_LIMIT *credit_limit_parametetr = (CREDIT_LIMIT *)args;

    int nRet = ERR_GENERIC;
    int nChildRet = ERR_GENERIC;
    int nCount,i;

    char *pValue = NULL;
    char *nEtaxBufData = NULL;
    char szUserId[BS_LITTLE] = {0};
    char szBuf[BS_BLOCK * 6] = {0};
    char final_url[512] = {0x00};
    char nsrsbhbuf[BS_NORMAL]={0};
    char *szTPassConfigJson = NULL;

    cJSON *jsonTpassConfig = NULL;
    cJSON *root = NULL, *objItem = NULL, *objItemChild = NULL;
    cJSON *objItemChild2 = NULL,*objItemToken  = NULL;

    if(NULL == credit_limit_parametetr)
    {
        return nRet;
    }

    hEtax->etax.user_args = (void *)(&header_array_jiangsu);
    memset(hEtax->etax.modelTaAddr.szTaxAuthorityURL,0x00,sizeof(hEtax->etax.modelTaAddr.szTaxAuthorityURL));
    strcpy(hEtax->etax.modelTaAddr.szTaxAuthorityURL,"https://dppt.jiangsu.chinatax.gov.cn:8443");
    memset(hEtax->etax.modelTaAddr.szInitRequestPage,0x00,sizeof(hEtax->etax.modelTaAddr.szInitRequestPage));
    strcpy(hEtax->etax.modelTaAddr.szInitRequestPage,"/szzhzz/spHandler?cdlj=digital-tax-account");

    do{
        //步骤1 使用etax地址获取tpass地址
        if (nChildRet = EtaxHTTPInit(&hEtax->etax, hDev)) {
            sprintf((char *)errinfo_uft8, "306Steps 1:HTTP 初始化失败, nRet:%d", nChildRet);
            break;
        }
        if (nChildRet = EtaxHTTPOpen(&hEtax->etax)) {
            sprintf((char *)errinfo_uft8, "306Steps 1:HTTP 连接失败, nRet:%d", nChildRet);
            break;
        }
        if (!(hEtax->etax.ctxApp = calloc(1, sizeof(struct ContextHttpAppBridge)))){
            sprintf((char *)errinfo_uft8, "306内存申请失败,空间不足");
            break;
        }
        hEtax->etax.cbHttpClose = EtaxBridgeClose;
        //获取tpass地址
        //步骤3 从token服务器获取登陆凭证去登陆Etax,登陆完成后返回的用户列表在hiTPass->bufHttpRep中
        if (hEtax->etax.ctxCon.nHttpRepCode != 200 && hEtax->etax.ctxCon.nHttpRepCode != 302){
            sprintf((char *)errinfo_uft8, "406请求跳转地址失败,HTTP Code:%d",hEtax->etax.ctxCon.nHttpRepCode);  
            break;
        }
        if(hEtax->etax.ctxCon.nHttpRepCode == 200)
        {
            szTPassConfigJson = (char *)evbuffer_pullup(hEtax->etax.bufHttpRep, -1);
        }
        else if(hEtax->etax.ctxCon.nHttpRepCode == 302)
        {
            szTPassConfigJson = (char *)hEtax->etax.user_args;
        }
        //获取跳转tpass地址
        if(RET_SUCCESS != get_tpass_addr(szTPassConfigJson,&pValue,"1"))
        {
             sprintf((char *)errinfo_uft8, "406获取跳转地址错误,地址解析失败");
             break;
        }

        if (!(uriTpassFull = evhttp_uri_parse(pValue))){
            sprintf((char *)errinfo_uft8, "406获取跳转地址错误,地址转换失败"); 
            break;
        }

        if (!FillUploadAddrModelDynamic(TAADDR_ETAX_TPASS, hDev->szRegCode, pValue, "/",&hEtax->tpass.modelTaAddr)) {
            sprintf((char *)errinfo_uft8, "406获取跳转地址错误,地址解析失败");
            break;
        } 

        if (nChildRet = EtaxHTTPInit(&hEtax->tpass, hDev)) {
            sprintf((char *)errinfo_uft8, "306Steps 2:HTTP 初始化失败, nRet:%d", nChildRet);
            break;
        }

        if (nChildRet = EtaxHTTPOpen(&hEtax->tpass)) {
            sprintf((char *)errinfo_uft8, "306Steps 2:HTTP 连接失败, nRet:%d", nChildRet);
            break;
        }
        if (!(hEtax->tpass.ctxApp = calloc(1, sizeof(struct ContextHttpAppBridge)))){
            sprintf((char *)errinfo_uft8, "306内存申请失败,空间不足");
            break;
        }
        hEtax->tpass.cbHttpClose = EtaxBridgeClose;
        
        //生成SM4密钥、获取公钥、发送SM4密钥
        if ((nChildRet = TPassLoginAuthSm4AndPublickey(&hEtax->tpass,hDev->szRegCode))) {
            sprintf((char *)errinfo_uft8, "406请求公钥或者发送SM4密钥失败:%s",(const char *)evbuffer_pullup(hEtax->tpass.bufHttpRep, -1));
            break;
        }

        //获取税盘证书与税局地址进行签名验证
        if ((nChildRet = TPassLoginAuthHelloAndAuth(&hEtax->tpass, uriTpassFull))) {
            sprintf((char *)errinfo_uft8, "406签名验签服务器响应请求错误:%s",(const char *)evbuffer_pullup(hEtax->tpass.bufHttpRep, -1));
            break;
        }

        //步骤4 根据签名验证请求后返回的用户信息以及中台传入的用户信息进行比较，筛选出需要登陆的身份
        if (!(root = cJSON_Parse((const char *)evbuffer_pullup(hEtax->tpass.bufHttpRep, -1)))) {
            sprintf((char *)errinfo_uft8, "406登陆凭证中返回用户列表为空");
            break;
        }
        if (!(objItem = cJSON_GetObjectItem(root, "userList"))) {
            sprintf((char *)errinfo_uft8, "406登陆凭证中返回用户列表解析失败");
            break;
        }
        nCount = cJSON_GetArraySize(objItem);
        for (i = 0; i < nCount; i++) {
            objItemChild = cJSON_GetArrayItem(objItem, i);

            if(strlen(credit_limit_parametetr->idcard)){
                if (!(objItemChild2 = cJSON_GetObjectItem(objItemChild, "id_card")))
                    continue;
                if (strcmp(objItemChild2->valuestring, credit_limit_parametetr->idcard))
                    continue;
            }

            if(strlen(credit_limit_parametetr->mobile)){
                if (!(objItemChild2 = cJSON_GetObjectItem(objItemChild, "mobile")))
                    continue;
                if (strcmp(objItemChild2->valuestring, credit_limit_parametetr->mobile))
                    continue;
            }

            if(credit_limit_parametetr->realationtype == 0){         //是否需要切换企业
                if (!(objItemChild2 = cJSON_GetObjectItem(objItemChild, "relatedType")))
                    continue;
                if (strcmp(objItemChild2->valuestring, credit_limit_parametetr->related_type))
                    continue;
            }

            if (!(objItemChild2 = cJSON_GetObjectItem(objItemChild, "user_id")))
                continue;
            strcpy(szUserId, objItemChild2->valuestring);
            printf("szUserId=%s,sz_password=%s\r\n",szUserId,credit_limit_parametetr->sz_password);
            break;
        }
        if(!strlen(szUserId)){
            sprintf((char *)errinfo_uft8, "306用户列表中未检测到该用户信息,请核实登陆信息是否有误");
            break;
        }

        if (nChildRet = TPassLoginPasswordLoginDo(&hEtax->tpass, szUserId, credit_limit_parametetr->sz_password, szBuf,sizeof(szBuf))){
            if (!(root = cJSON_Parse(szBuf)))
                break;
            if (!(objItem = cJSON_GetObjectItem(root, "msg")))
                break;
            sprintf((char *)errinfo_uft8, "406登陆失败:%s",objItem->valuestring);
            break;
        }

        //步骤5 根据筛选出来的身份进行登陆
        if(credit_limit_parametetr->realationtype == 1){         //是否需要切换企业后查询
            if (nChildRet = TPassSelectRelationchange(&hEtax->tpass,credit_limit_parametetr->uniqueIdentity,credit_limit_parametetr->realationStatus, szBuf,sizeof(szBuf))){
                if (!(root = cJSON_Parse(szBuf)))
                    break;
                if (!(objItem = cJSON_GetObjectItem(root, "msg")))
                    break;
                sprintf((char *)errinfo_uft8, "406企业切换失败,%s",objItem->valuestring);
                break;
            } 
        }


        //步骤7 获取一键登陆需要的code参数和RPA需要的token参数
        printf("TPassSelectRelationchange data is %s\n", szBuf);
        if (!(root = cJSON_Parse(szBuf))){
            sprintf((char *)errinfo_uft8, "406企业切换失败,返回数据解析失败");
            break;
        }
        if (!(objItem = cJSON_GetObjectItem(root, "code"))){
            sprintf((char *)errinfo_uft8, "406企业切换失败,返回数据中无登陆参数");
            break;
        }

        printf(" = %s\r\n",objItem->valuestring);

        if(0 > get_final_url(pValue,objItem->valuestring,final_url))
            break;

        printf("final_url=%s\r\n",final_url);

        DpptJiangSuConnect(&hEtax->dppt,final_url,hDev);

        if (!(nChildRet = InitSecurityConfig(&hEtax->dppt))) {

        }
        if(nChildRet){
            sprintf((char *)errinfo_uft8, "406初始化dppt服务器失败,错误状态为:%d", nChildRet);    
            break;
        }

        //步骤7 授信额度查询
        sprintf(nsrsbhbuf,"{\"kpysfid\":\"%s\",\"nsrsbh\":\"%s\"}",credit_limit_parametetr->nsrsbh,credit_limit_parametetr->nsrsbh);

        if ((nChildRet = EtaxQueryInvoice(hEtax,"/kpfw/sjtj/v1/info",nsrsbhbuf)) < 0){
            sprintf((char *)errinfo_uft8, "406查询接口失败:%s\r\n",(const char *)evbuffer_pullup(hEtax->dppt.bufHttpRep, -1));
            break;                      
        }  
        nChildRet = evbuffer_get_length(hEtax->dppt.bufHttpRep);

        *(credit_limit_parametetr->sxeddata) = NULL;
        *(credit_limit_parametetr->sxeddata) = (char *)malloc(nChildRet);
        if(*(credit_limit_parametetr->sxeddata) == NULL){    
            break;
        }
        evbuffer_copyout_from(hEtax->dppt.bufHttpRep,NULL, *(credit_limit_parametetr->sxeddata),nChildRet);

        //结束
        nRet = RET_SUCCESS;
    }while (false);

    //释放内存
    if(nEtaxBufData)
        free(nEtaxBufData);
    nEtaxBufData = NULL;

    //释放内存
    if (root)
        cJSON_Delete(root);
    root = NULL;

    //释放内存
    if (jsonTpassConfig)
        cJSON_Delete(jsonTpassConfig);
    jsonTpassConfig  = NULL;

    if (uriTpassFull)
        evhttp_uri_free(uriTpassFull);

    if(pValue)
        free(pValue);

    if(hEtax->etax.user_args)
    {
        free(hEtax->etax.user_args);
        hEtax->etax.user_args = NULL;
    }

    return nRet; 
}

int etax_jiangsu_hcstaquery_invoice_issuance(HETAX hEtax, HDEV hDev,void *args,char *errinfo_uft8)
{
    struct evhttp_uri *uriTpassFull = NULL;

    INVOICE_HCSTAQUERY *hcstaquery_parametetr = (INVOICE_HCSTAQUERY *)args;

    int nRet = ERR_GENERIC;
    int nChildRet = ERR_GENERIC;
    int nCount,i;

    char *pValue = NULL;
    char *nEtaxBufData = NULL;
    char szUserId[BS_LITTLE] = {0};
    char szBuf[BS_BLOCK * 6] = {0};
    char final_url[512] = {0x00};
    char *szTPassConfigJson = NULL;

    cJSON *jsonTpassConfig = NULL;
    cJSON *root = NULL, *objItem = NULL, *objItemChild = NULL;
    cJSON *objItemChild2 = NULL,*objItemToken  = NULL;

    if(NULL == hcstaquery_parametetr)
    {
        return nRet;
    }

    hEtax->etax.user_args = (void *)(&header_array_jiangsu);
    memset(hEtax->etax.modelTaAddr.szTaxAuthorityURL,0x00,sizeof(hEtax->etax.modelTaAddr.szTaxAuthorityURL));
    strcpy(hEtax->etax.modelTaAddr.szTaxAuthorityURL,"https://dppt.jiangsu.chinatax.gov.cn:8443");
    memset(hEtax->etax.modelTaAddr.szInitRequestPage,0x00,sizeof(hEtax->etax.modelTaAddr.szInitRequestPage));
    strcpy(hEtax->etax.modelTaAddr.szInitRequestPage,"/szzhzz/spHandler?cdlj=digital-tax-account");

    do{
        //步骤1 使用etax地址获取tpass地址
        if (nChildRet = EtaxHTTPInit(&hEtax->etax, hDev)) {
            sprintf((char *)errinfo_uft8, "306Steps 1:HTTP 初始化失败, nRet:%d", nChildRet);
            break;
        }
        if (nChildRet = EtaxHTTPOpen(&hEtax->etax)) {
            sprintf((char *)errinfo_uft8, "306Steps 1:HTTP 连接失败, nRet:%d", nChildRet);
            break;
        }
        if (!(hEtax->etax.ctxApp = calloc(1, sizeof(struct ContextHttpAppBridge)))){
            sprintf((char *)errinfo_uft8, "306内存申请失败,空间不足");
            break;
        }
        hEtax->etax.cbHttpClose = EtaxBridgeClose;
        //获取tpass地址
        //步骤3 从token服务器获取登陆凭证去登陆Etax,登陆完成后返回的用户列表在hiTPass->bufHttpRep中
        if (hEtax->etax.ctxCon.nHttpRepCode != 200 && hEtax->etax.ctxCon.nHttpRepCode != 302){
            sprintf((char *)errinfo_uft8, "406请求跳转地址失败,HTTP Code:%d",hEtax->etax.ctxCon.nHttpRepCode);  
            break;
        }
        if(hEtax->etax.ctxCon.nHttpRepCode == 200)
        {
            szTPassConfigJson = (char *)evbuffer_pullup(hEtax->etax.bufHttpRep, -1);
        }
        else if(hEtax->etax.ctxCon.nHttpRepCode == 302)
        {
            szTPassConfigJson = (char *)hEtax->etax.user_args;
        }
        //获取跳转tpass地址
        if(RET_SUCCESS != get_tpass_addr(szTPassConfigJson,&pValue,"1"))
        {
             sprintf((char *)errinfo_uft8, "406获取跳转地址错误,地址解析失败");
             break;
        }

        if (!(uriTpassFull = evhttp_uri_parse(pValue))){
            sprintf((char *)errinfo_uft8, "406获取跳转地址错误,地址转换失败"); 
            break;
        }

        if (!FillUploadAddrModelDynamic(TAADDR_ETAX_TPASS, hDev->szRegCode, pValue, "/",&hEtax->tpass.modelTaAddr)) {
            sprintf((char *)errinfo_uft8, "406获取跳转地址错误,地址解析失败");
            break;
        } 

        if (nChildRet = EtaxHTTPInit(&hEtax->tpass, hDev)) {
            sprintf((char *)errinfo_uft8, "306Steps 2:HTTP 初始化失败, nRet:%d", nChildRet);
            break;
        }

        if (nChildRet = EtaxHTTPOpen(&hEtax->tpass)) {
            sprintf((char *)errinfo_uft8, "306Steps 2:HTTP 连接失败, nRet:%d", nChildRet);
            break;
        }
        if (!(hEtax->tpass.ctxApp = calloc(1, sizeof(struct ContextHttpAppBridge)))){
            sprintf((char *)errinfo_uft8, "306内存申请失败,空间不足");
            break;
        }
        hEtax->tpass.cbHttpClose = EtaxBridgeClose;
        
        //生成SM4密钥、获取公钥、发送SM4密钥
        if ((nChildRet = TPassLoginAuthSm4AndPublickey(&hEtax->tpass,hDev->szRegCode))) {
            sprintf((char *)errinfo_uft8, "406请求公钥或者发送SM4密钥失败:%s",(const char *)evbuffer_pullup(hEtax->tpass.bufHttpRep, -1));
            break;
        }

        //获取税盘证书与税局地址进行签名验证
        if ((nChildRet = TPassLoginAuthHelloAndAuth(&hEtax->tpass, uriTpassFull))) {
            sprintf((char *)errinfo_uft8, "406签名验签服务器响应请求错误:%s",(const char *)evbuffer_pullup(hEtax->tpass.bufHttpRep, -1));
            break;
        }

        //步骤4 根据签名验证请求后返回的用户信息以及中台传入的用户信息进行比较，筛选出需要登陆的身份
        if (!(root = cJSON_Parse((const char *)evbuffer_pullup(hEtax->tpass.bufHttpRep, -1)))) {
            sprintf((char *)errinfo_uft8, "406登陆凭证中返回用户列表为空");
            break;
        }
        if (!(objItem = cJSON_GetObjectItem(root, "userList"))) {
            sprintf((char *)errinfo_uft8, "406登陆凭证中返回用户列表解析失败");
            break;
        }
        nCount = cJSON_GetArraySize(objItem);
        for (i = 0; i < nCount; i++) {
            objItemChild = cJSON_GetArrayItem(objItem, i);

            if(strlen(hcstaquery_parametetr->idcard)){
                if (!(objItemChild2 = cJSON_GetObjectItem(objItemChild, "id_card")))
                    continue;
                if (strcmp(objItemChild2->valuestring, hcstaquery_parametetr->idcard))
                    continue;
            }

            if(strlen(hcstaquery_parametetr->mobile)){
                if (!(objItemChild2 = cJSON_GetObjectItem(objItemChild, "mobile")))
                    continue;
                if (strcmp(objItemChild2->valuestring, hcstaquery_parametetr->mobile))
                    continue;
            }

            if(hcstaquery_parametetr->realationtype == 0){         //是否需要切换企业
                if (!(objItemChild2 = cJSON_GetObjectItem(objItemChild, "relatedType")))
                    continue;
                if (strcmp(objItemChild2->valuestring, hcstaquery_parametetr->related_type))
                    continue;
            }

            if (!(objItemChild2 = cJSON_GetObjectItem(objItemChild, "user_id")))
                continue;
            strcpy(szUserId, objItemChild2->valuestring);
            printf("szUserId=%s,sz_password=%s\r\n",szUserId,hcstaquery_parametetr->sz_password);
            break;
        }
        if(!strlen(szUserId)){
            sprintf((char *)errinfo_uft8, "306用户列表中未检测到该用户信息,请核实登陆信息是否有误");
            break;
        }

        if (nChildRet = TPassLoginPasswordLoginDo(&hEtax->tpass, szUserId, hcstaquery_parametetr->sz_password, szBuf,sizeof(szBuf))){
            if (!(root = cJSON_Parse(szBuf)))
                break;
            if (!(objItem = cJSON_GetObjectItem(root, "msg")))
                break;
            sprintf((char *)errinfo_uft8, "406登陆失败:%s",objItem->valuestring);
            break;
        }

        //步骤5 根据筛选出来的身份进行登陆
        if(hcstaquery_parametetr->realationtype == 1){         //是否需要切换企业后查询
            if (nChildRet = TPassSelectRelationchange(&hEtax->tpass,hcstaquery_parametetr->uniqueIdentity,hcstaquery_parametetr->realationStatus, szBuf,sizeof(szBuf))){
                if (!(root = cJSON_Parse(szBuf)))
                    break;
                if (!(objItem = cJSON_GetObjectItem(root, "msg")))
                    break;
                sprintf((char *)errinfo_uft8, "406企业切换失败,%s",objItem->valuestring);
                break;
            } 
        }


        //步骤7 获取一键登陆需要的code参数和RPA需要的token参数
        printf("TPassSelectRelationchange data is %s\n", szBuf);
        if (!(root = cJSON_Parse(szBuf))){
            sprintf((char *)errinfo_uft8, "406企业切换失败,返回数据解析失败");
            break;
        }
        if (!(objItem = cJSON_GetObjectItem(root, "code"))){
            sprintf((char *)errinfo_uft8, "406企业切换失败,返回数据中无登陆参数");
            break;
        }

        printf(" = %s\r\n",objItem->valuestring);

        if(0 > get_final_url(pValue,objItem->valuestring,final_url))
            break;

        printf("final_url=%s\r\n",final_url);

        DpptJiangSuConnect(&hEtax->dppt,final_url,hDev);

        if (!(nChildRet = InitSecurityConfig(&hEtax->dppt))) {

        }
        if(nChildRet){
            sprintf((char *)errinfo_uft8, "406初始化dppt服务器失败,错误状态为:%d", nChildRet);    
            break;
        }

        //步骤7 发票查询
        if ((nChildRet = EtaxQueryInvoice(hEtax, "/szzhzz/fpcx/v1/queryFphcztxx",hcstaquery_parametetr->fpcxsj)) < 0){
            sprintf((char *)errinfo_uft8, "406发票查询接口失败:%s",evbuffer_pullup(hEtax->dppt.bufHttpRep, -1));
            break;                      
        }  
        nChildRet = evbuffer_get_length(hEtax->dppt.bufHttpRep);
        printf("nChildRet = %d\r\n,%s\r\n",nChildRet,evbuffer_pullup(hEtax->dppt.bufHttpRep, -1));

        nEtaxBufData = (char *)malloc(nChildRet);
        if(nEtaxBufData == NULL)
            break;

        *(hcstaquery_parametetr->fpcxdata) = NULL;
        *(hcstaquery_parametetr->fpcxdata) = (char *)malloc(nChildRet);
        if(*(hcstaquery_parametetr->fpcxdata) == NULL){    
            break;
        }
        evbuffer_copyout_from(hEtax->dppt.bufHttpRep,NULL, nEtaxBufData,nChildRet);
        EncordingConvert( "utf-8","gbk",nEtaxBufData, nChildRet, *(hcstaquery_parametetr->fpcxdata), nChildRet);

        //结束
        nRet = RET_SUCCESS;
    }while (false);

    //释放内存
    if(nEtaxBufData)
        free(nEtaxBufData);
    nEtaxBufData = NULL;

    //释放内存
    if (root)
        cJSON_Delete(root);
    root = NULL;

    //释放内存
    if (jsonTpassConfig)
        cJSON_Delete(jsonTpassConfig);
    jsonTpassConfig  = NULL;

    if (uriTpassFull)
        evhttp_uri_free(uriTpassFull);

    if(pValue)
        free(pValue);

    if(hEtax->etax.user_args)
    {
        free(hEtax->etax.user_args);
        hEtax->etax.user_args = NULL;
    }

    return nRet; 
}

//蓝字发票开具
int etax_jiangsu_blue_ink_invoice_issuance(HETAX hEtax, HDEV hDev,void *args,char *errinfo_uft8)
{
    struct evhttp_uri *uriTpassFull = NULL;

    BLUE_INK_INVOICE_ISSUANCE *blue_ink_invoice_issuance_parametetr = (BLUE_INK_INVOICE_ISSUANCE *)args;

    int nRet = ERR_GENERIC;
    int nChildRet = ERR_GENERIC;
    int nCount,i;

    char *pValue = NULL;
    char *nsrdata = NULL;
    char *nEtaxBufData = NULL;
    char szUserId[BS_LITTLE] = {0};
    char szBuf[BS_BLOCK * 6] = {0};
    char final_url[512] = {0x00};
    char *szTPassConfigJson = NULL;

    cJSON *jsonTpassConfig = NULL;
    cJSON *root = NULL, *objItem = NULL, *objItemChild = NULL;
    cJSON *objItemChild2 = NULL,*objItemToken  = NULL;

    EHFPXX fpxx = NULL;

    if(NULL == blue_ink_invoice_issuance_parametetr)
    {
        return nRet;
    }

    hEtax->etax.user_args = (void *)(&header_array_jiangsu);
    memset(hEtax->etax.modelTaAddr.szTaxAuthorityURL,0x00,sizeof(hEtax->etax.modelTaAddr.szTaxAuthorityURL));
    strcpy(hEtax->etax.modelTaAddr.szTaxAuthorityURL,"https://dppt.jiangsu.chinatax.gov.cn:8443");
    memset(hEtax->etax.modelTaAddr.szInitRequestPage,0x00,sizeof(hEtax->etax.modelTaAddr.szInitRequestPage));
    strcpy(hEtax->etax.modelTaAddr.szInitRequestPage,"/szzhzz/spHandler?cdlj=digital-tax-account");

    do{

        if (!(fpxx = EMallocFpxxLinkDev(hDev)))
            break;

        //步骤1 使用etax地址获取tpass地址
        if (nChildRet = EtaxHTTPInit(&hEtax->etax, hDev)) {
            sprintf((char *)errinfo_uft8, "306Steps 1:HTTP 初始化失败, nRet:%d", nChildRet);
            break;
        }
        if (nChildRet = EtaxHTTPOpen(&hEtax->etax)) {
            sprintf((char *)errinfo_uft8, "306Steps 1:HTTP 连接失败, nRet:%d", nChildRet);
            break;
        }
        if (!(hEtax->etax.ctxApp = calloc(1, sizeof(struct ContextHttpAppBridge)))){
            sprintf((char *)errinfo_uft8, "306内存申请失败,空间不足");
            break;
        }
        hEtax->etax.cbHttpClose = EtaxBridgeClose;
        //获取tpass地址
        //步骤3 从token服务器获取登陆凭证去登陆Etax,登陆完成后返回的用户列表在hiTPass->bufHttpRep中
        if (hEtax->etax.ctxCon.nHttpRepCode != 200 && hEtax->etax.ctxCon.nHttpRepCode != 302){
            sprintf((char *)errinfo_uft8, "406请求跳转地址失败,HTTP Code:%d",hEtax->etax.ctxCon.nHttpRepCode);  
            break;
        }
        if(hEtax->etax.ctxCon.nHttpRepCode == 200)
        {
            szTPassConfigJson = (char *)evbuffer_pullup(hEtax->etax.bufHttpRep, -1);
        }
        else if(hEtax->etax.ctxCon.nHttpRepCode == 302)
        {
            szTPassConfigJson = (char *)hEtax->etax.user_args;
        }
        //获取跳转tpass地址
        if(RET_SUCCESS != get_tpass_addr(szTPassConfigJson,&pValue,"1"))
        {
             sprintf((char *)errinfo_uft8, "406获取跳转地址错误,地址解析失败");
             break;
        }

        if (!(uriTpassFull = evhttp_uri_parse(pValue))){
            sprintf((char *)errinfo_uft8, "406获取跳转地址错误,地址转换失败"); 
            break;
        }

        if (!FillUploadAddrModelDynamic(TAADDR_ETAX_TPASS, hDev->szRegCode, pValue, "/",&hEtax->tpass.modelTaAddr)) {
            sprintf((char *)errinfo_uft8, "406获取跳转地址错误,地址解析失败");
            break;
        } 

        if (nChildRet = EtaxHTTPInit(&hEtax->tpass, hDev)) {
            sprintf((char *)errinfo_uft8, "306Steps 2:HTTP 初始化失败, nRet:%d", nChildRet);
            break;
        }

        if (nChildRet = EtaxHTTPOpen(&hEtax->tpass)) {
            sprintf((char *)errinfo_uft8, "306Steps 2:HTTP 连接失败, nRet:%d", nChildRet);
            break;
        }
        if (!(hEtax->tpass.ctxApp = calloc(1, sizeof(struct ContextHttpAppBridge)))){
            sprintf((char *)errinfo_uft8, "306内存申请失败,空间不足");
            break;
        }
        hEtax->tpass.cbHttpClose = EtaxBridgeClose;
        
        //生成SM4密钥、获取公钥、发送SM4密钥
        if ((nChildRet = TPassLoginAuthSm4AndPublickey(&hEtax->tpass,hDev->szRegCode))) {
            sprintf((char *)errinfo_uft8, "406请求公钥或者发送SM4密钥失败:%s",(const char *)evbuffer_pullup(hEtax->tpass.bufHttpRep, -1));
            break;
        }

        //获取税盘证书与税局地址进行签名验证
        if ((nChildRet = TPassLoginAuthHelloAndAuth(&hEtax->tpass, uriTpassFull))) {
            sprintf((char *)errinfo_uft8, "406签名验签服务器响应请求错误:%s",(const char *)evbuffer_pullup(hEtax->tpass.bufHttpRep, -1));
            break;
        }

        //步骤4 根据签名验证请求后返回的用户信息以及中台传入的用户信息进行比较，筛选出需要登陆的身份
        if (!(root = cJSON_Parse((const char *)evbuffer_pullup(hEtax->tpass.bufHttpRep, -1)))) {
            sprintf((char *)errinfo_uft8, "406登陆凭证中返回用户列表为空");
            break;
        }
        if (!(objItem = cJSON_GetObjectItem(root, "userList"))) {
            sprintf((char *)errinfo_uft8, "406登陆凭证中返回用户列表解析失败");
            break;
        }
        nCount = cJSON_GetArraySize(objItem);
        for (i = 0; i < nCount; i++) {
            objItemChild = cJSON_GetArrayItem(objItem, i);

            if(strlen(blue_ink_invoice_issuance_parametetr->idcard)){
                if (!(objItemChild2 = cJSON_GetObjectItem(objItemChild, "id_card")))
                    continue;
                if (strcmp(objItemChild2->valuestring, blue_ink_invoice_issuance_parametetr->idcard))
                    continue;
            }

            if(strlen(blue_ink_invoice_issuance_parametetr->mobile)){
                if (!(objItemChild2 = cJSON_GetObjectItem(objItemChild, "mobile")))
                    continue;
                if (strcmp(objItemChild2->valuestring, blue_ink_invoice_issuance_parametetr->mobile))
                    continue;
            }

            if(blue_ink_invoice_issuance_parametetr->realationtype == 0){         //是否需要切换企业
                if (!(objItemChild2 = cJSON_GetObjectItem(objItemChild, "relatedType")))
                    continue;
                if (strcmp(objItemChild2->valuestring, blue_ink_invoice_issuance_parametetr->related_type))
                    continue;
            }

            if (!(objItemChild2 = cJSON_GetObjectItem(objItemChild, "user_id")))
                continue;
            strcpy(szUserId, objItemChild2->valuestring);
            printf("szUserId=%s,sz_password=%s\r\n",szUserId,blue_ink_invoice_issuance_parametetr->sz_password);
            break;
        }
        if(!strlen(szUserId)){
            sprintf((char *)errinfo_uft8, "306用户列表中未检测到该用户信息,请核实登陆信息是否有误");
            break;
        }

        if (nChildRet = TPassLoginPasswordLoginDo(&hEtax->tpass, szUserId, blue_ink_invoice_issuance_parametetr->sz_password, szBuf,sizeof(szBuf))){
            if (!(root = cJSON_Parse(szBuf)))
                break;
            if (!(objItem = cJSON_GetObjectItem(root, "msg")))
                break;
            sprintf((char *)errinfo_uft8, "406登陆失败:%s",objItem->valuestring);
            break;
        }

        //步骤5 根据筛选出来的身份进行登陆
        if(blue_ink_invoice_issuance_parametetr->realationtype == 1){         //是否需要切换企业后查询
            if (nChildRet = TPassSelectRelationchange(&hEtax->tpass,blue_ink_invoice_issuance_parametetr->uniqueIdentity,blue_ink_invoice_issuance_parametetr->realationStatus, szBuf,sizeof(szBuf))){
                if (!(root = cJSON_Parse(szBuf)))
                    break;
                if (!(objItem = cJSON_GetObjectItem(root, "msg")))
                    break;
                sprintf((char *)errinfo_uft8, "406企业切换失败,%s",objItem->valuestring);
                break;
            } 
        }


        //步骤7 获取一键登陆需要的code参数和RPA需要的token参数
        printf("TPassSelectRelationchange data is %s\n", szBuf);
        if (!(root = cJSON_Parse(szBuf))){
            sprintf((char *)errinfo_uft8, "406企业切换失败,返回数据解析失败");
            break;
        }
        if (!(objItem = cJSON_GetObjectItem(root, "code"))){
            sprintf((char *)errinfo_uft8, "406企业切换失败,返回数据中无登陆参数");
            break;
        }

        printf(" = %s\r\n",objItem->valuestring);

        if(0 > get_final_url(pValue,objItem->valuestring,final_url))
            break;

        printf("final_url=%s\r\n",final_url);

        DpptJiangSuConnect(&hEtax->dppt,final_url,hDev);

        if (!(nChildRet = InitSecurityConfig(&hEtax->dppt))) {

        }
        if(nChildRet){
            sprintf((char *)errinfo_uft8, "406初始化dppt服务器失败,错误状态为:%d", nChildRet);    
            break;
        }

        if (EtaxLoadDevOnline(hEtax, hDev,fpxx)){
            sprintf((char *)errinfo_uft8, "406税务机关代码查询或纳税人注册信息查询失败");    
            break; 
        }

        //输出纳税人信息
        cJSON *json = cJSON_CreateObject();
        cJSON_AddStringToObject(json, "Nsrsbh", hDev->szCommonTaxID);
        cJSON_AddStringToObject(json, "Scjydz",fpxx->xfdz);
        cJSON_AddStringToObject(json, "Zcdlxdh",fpxx->xfdh);
        cJSON_AddStringToObject(json, "Nsrmc", fpxx->xfmc);
        cJSON_AddStringToObject(json, "Ssdabh", fpxx->xfsh);
        cJSON_AddStringToObject(json, "Yhzh", fpxx->xfyhzh);
        cJSON_AddStringToObject(json, "Yhyywdmc",fpxx->xfkhh);

        if (!(nsrdata = cJSON_Print(json)))
            break;

        printf("DpptEncryptIo strlen(nsrdata) =%d \r\n data = %s\r\n",strlen(nsrdata),nsrdata);    

        *(blue_ink_invoice_issuance_parametetr->hqnsrjcxx) = NULL;
        *(blue_ink_invoice_issuance_parametetr->hqnsrjcxx) = (char *)malloc(strlen(nsrdata));
        if(*(blue_ink_invoice_issuance_parametetr->hqnsrjcxx) == NULL){    
            break;
        }

        EncordingConvert( "utf-8","gbk",nsrdata, strlen(nsrdata), *(blue_ink_invoice_issuance_parametetr->hqnsrjcxx), strlen(nsrdata));

        printf("blue_ink_invoice_issuance_parametetr->fpkjjson=%s\r\n",blue_ink_invoice_issuance_parametetr->fpkjjson);
        if (EtaxAnalyzeJsonBuf(blue_ink_invoice_issuance_parametetr->fpkjjson, fpxx,hDev,errinfo_uft8))
            break;
        //调用开票接口开票，可重复循环开具
        if (EtaxMakeInvoiceMain(hEtax,fpxx,hDev,errinfo_uft8,blue_ink_invoice_issuance_parametetr->fpkjjson) < 0) {
            break;
        }

        nChildRet = evbuffer_get_length(hEtax->dppt.bufHttpRep);

        *(blue_ink_invoice_issuance_parametetr->fpkjout) = NULL;
        *(blue_ink_invoice_issuance_parametetr->fpkjout) = (char *)malloc(nChildRet);
        if(*(blue_ink_invoice_issuance_parametetr->fpkjout) == NULL){    
            break;
        }
        evbuffer_copyout_from(hEtax->dppt.bufHttpRep,NULL, *(blue_ink_invoice_issuance_parametetr->fpkjout),nChildRet);

        //结束
        nRet = RET_SUCCESS;
    }while (false);

    //释放内存
    if(nEtaxBufData)
        free(nEtaxBufData);
    nEtaxBufData = NULL;

    //释放内存
    if (root)
        cJSON_Delete(root);
    root = NULL;

    //释放内存
    if (jsonTpassConfig)
        cJSON_Delete(jsonTpassConfig);
    jsonTpassConfig  = NULL;

    if (uriTpassFull)
        evhttp_uri_free(uriTpassFull);

    if(pValue)
        free(pValue);

    if(hEtax->etax.user_args)
    {
        free(hEtax->etax.user_args);
        hEtax->etax.user_args = NULL;
    }

    return nRet; 
}

//可开红票的蓝票查询
int etax_jiangsu_hzqrxxSzzhQuery(HETAX hEtax, HDEV hDev,void *args,char *errinfo_uft8)
{
    struct evhttp_uri *uriTpassFull = NULL;

    INVOICE_QUERY *invoice_query_parametetr = (INVOICE_QUERY *)args;

    int nRet = ERR_GENERIC;
    int nChildRet = ERR_GENERIC;
    int nCount,i;

    char *pValue = NULL;
    char *nEtaxBufData = NULL;
    char szUserId[BS_LITTLE] = {0};
    char szBuf[BS_BLOCK * 6] = {0};
    char final_url[512] = {0x00};
    char *szTPassConfigJson = NULL;

    cJSON *jsonTpassConfig = NULL;
    cJSON *root = NULL, *objItem = NULL, *objItemChild = NULL;
    cJSON *objItemChild2 = NULL,*objItemToken  = NULL;

    if(NULL == invoice_query_parametetr)
    {
        return nRet;
    }

    hEtax->etax.user_args = (void *)(&header_array_jiangsu);
    memset(hEtax->etax.modelTaAddr.szTaxAuthorityURL,0x00,sizeof(hEtax->etax.modelTaAddr.szTaxAuthorityURL));
    strcpy(hEtax->etax.modelTaAddr.szTaxAuthorityURL,"https://dppt.jiangsu.chinatax.gov.cn:8443");
    memset(hEtax->etax.modelTaAddr.szInitRequestPage,0x00,sizeof(hEtax->etax.modelTaAddr.szInitRequestPage));
    strcpy(hEtax->etax.modelTaAddr.szInitRequestPage,"/szzhzz/spHandler?cdlj=digital-tax-account");    

    do{
        //步骤1 使用etax地址获取tpass地址
        if (nChildRet = EtaxHTTPInit(&hEtax->etax, hDev)) {
            sprintf((char *)errinfo_uft8, "306Steps 1:HTTP 初始化失败, nRet:%d", nChildRet);
            break;
        }
        if (nChildRet = EtaxHTTPOpen(&hEtax->etax)) {
            sprintf((char *)errinfo_uft8, "306Steps 1:HTTP 连接失败, nRet:%d", nChildRet);
            break;
        }
        if (!(hEtax->etax.ctxApp = calloc(1, sizeof(struct ContextHttpAppBridge)))){
            sprintf((char *)errinfo_uft8, "306内存申请失败,空间不足");
            break;
        }
        hEtax->etax.cbHttpClose = EtaxBridgeClose;
        //获取tpass地址
        //步骤3 从token服务器获取登陆凭证去登陆Etax,登陆完成后返回的用户列表在hiTPass->bufHttpRep中
        if (hEtax->etax.ctxCon.nHttpRepCode != 200 && hEtax->etax.ctxCon.nHttpRepCode != 302){
            sprintf((char *)errinfo_uft8, "406请求跳转地址失败,HTTP Code:%d",hEtax->etax.ctxCon.nHttpRepCode);  
            break;
        }
        if(hEtax->etax.ctxCon.nHttpRepCode == 200)
        {
            szTPassConfigJson = (char *)evbuffer_pullup(hEtax->etax.bufHttpRep, -1);
        }
        else if(hEtax->etax.ctxCon.nHttpRepCode == 302)
        {
            szTPassConfigJson = (char *)hEtax->etax.user_args;
        }
        //获取跳转tpass地址
        if(RET_SUCCESS != get_tpass_addr(szTPassConfigJson,&pValue,"1"))
        {
             sprintf((char *)errinfo_uft8, "406获取跳转地址错误,地址解析失败");
             break;
        }

        if (!(uriTpassFull = evhttp_uri_parse(pValue))){
            sprintf((char *)errinfo_uft8, "406获取跳转地址错误,地址转换失败"); 
            break;
        }

        if (!FillUploadAddrModelDynamic(TAADDR_ETAX_TPASS, hDev->szRegCode, pValue, "/",&hEtax->tpass.modelTaAddr)) {
            sprintf((char *)errinfo_uft8, "406获取跳转地址错误,地址解析失败");
            break;
        } 

        if (nChildRet = EtaxHTTPInit(&hEtax->tpass, hDev)) {
            sprintf((char *)errinfo_uft8, "306Steps 2:HTTP 初始化失败, nRet:%d", nChildRet);
            break;
        }

        if (nChildRet = EtaxHTTPOpen(&hEtax->tpass)) {
            sprintf((char *)errinfo_uft8, "306Steps 2:HTTP 连接失败, nRet:%d", nChildRet);
            break;
        }
        if (!(hEtax->tpass.ctxApp = calloc(1, sizeof(struct ContextHttpAppBridge)))){
            sprintf((char *)errinfo_uft8, "306内存申请失败,空间不足");
            break;
        }
        hEtax->tpass.cbHttpClose = EtaxBridgeClose;
        
        //生成SM4密钥、获取公钥、发送SM4密钥
        if ((nChildRet = TPassLoginAuthSm4AndPublickey(&hEtax->tpass,hDev->szRegCode))) {
            sprintf((char *)errinfo_uft8, "406请求公钥或者发送SM4密钥失败:%s",(const char *)evbuffer_pullup(hEtax->tpass.bufHttpRep, -1));
            break;
        }

        //获取税盘证书与税局地址进行签名验证
        if ((nChildRet = TPassLoginAuthHelloAndAuth(&hEtax->tpass, uriTpassFull))) {
            sprintf((char *)errinfo_uft8, "406签名验签服务器响应请求错误:%s",(const char *)evbuffer_pullup(hEtax->tpass.bufHttpRep, -1));
            break;
        }

        //步骤4 根据签名验证请求后返回的用户信息以及中台传入的用户信息进行比较，筛选出需要登陆的身份
        if (!(root = cJSON_Parse((const char *)evbuffer_pullup(hEtax->tpass.bufHttpRep, -1)))) {
            sprintf((char *)errinfo_uft8, "406登陆凭证中返回用户列表为空");
            break;
        }
        if (!(objItem = cJSON_GetObjectItem(root, "userList"))) {
            sprintf((char *)errinfo_uft8, "406登陆凭证中返回用户列表解析失败");
            break;
        }
        nCount = cJSON_GetArraySize(objItem);
        for (i = 0; i < nCount; i++) {
            objItemChild = cJSON_GetArrayItem(objItem, i);

            if(strlen(invoice_query_parametetr->idcard)){
                if (!(objItemChild2 = cJSON_GetObjectItem(objItemChild, "id_card")))
                    continue;
                if (strcmp(objItemChild2->valuestring, invoice_query_parametetr->idcard))
                    continue;
            }

            if(strlen(invoice_query_parametetr->mobile)){
                if (!(objItemChild2 = cJSON_GetObjectItem(objItemChild, "mobile")))
                    continue;
                if (strcmp(objItemChild2->valuestring, invoice_query_parametetr->mobile))
                    continue;
            }

            if(invoice_query_parametetr->realationtype == 0){         //是否需要切换企业
                if (!(objItemChild2 = cJSON_GetObjectItem(objItemChild, "relatedType")))
                    continue;
                if (strcmp(objItemChild2->valuestring, invoice_query_parametetr->related_type))
                    continue;
            }

            if (!(objItemChild2 = cJSON_GetObjectItem(objItemChild, "user_id")))
                continue;
            strcpy(szUserId, objItemChild2->valuestring);
            printf("szUserId=%s,sz_password=%s\r\n",szUserId,invoice_query_parametetr->sz_password);
            break;
        }
        if(!strlen(szUserId)){
            sprintf((char *)errinfo_uft8, "306用户列表中未检测到该用户信息,请核实登陆信息是否有误");
            break;
        }

        if (nChildRet = TPassLoginPasswordLoginDo(&hEtax->tpass, szUserId, invoice_query_parametetr->sz_password, szBuf,sizeof(szBuf))){
            if (!(root = cJSON_Parse(szBuf)))
                break;
            if (!(objItem = cJSON_GetObjectItem(root, "msg")))
                break;
            sprintf((char *)errinfo_uft8, "406登陆失败:%s",objItem->valuestring);
            break;
        }

        //步骤5 根据筛选出来的身份进行登陆
        if(invoice_query_parametetr->realationtype == 1){         //是否需要切换企业后查询
            if (nChildRet = TPassSelectRelationchange(&hEtax->tpass,invoice_query_parametetr->uniqueIdentity,invoice_query_parametetr->realationStatus, szBuf,sizeof(szBuf))){
                if (!(root = cJSON_Parse(szBuf)))
                    break;
                if (!(objItem = cJSON_GetObjectItem(root, "msg")))
                    break;
                sprintf((char *)errinfo_uft8, "406企业切换失败,%s",objItem->valuestring);
                break;
            } 
        }


        //步骤7 获取一键登陆需要的code参数和RPA需要的token参数
        printf("TPassSelectRelationchange data is %s\n", szBuf);
        if (!(root = cJSON_Parse(szBuf))){
            sprintf((char *)errinfo_uft8, "406企业切换失败,返回数据解析失败");
            break;
        }
        if (!(objItem = cJSON_GetObjectItem(root, "code"))){
            sprintf((char *)errinfo_uft8, "406企业切换失败,返回数据中无登陆参数");
            break;
        }

        printf(" = %s\r\n",objItem->valuestring);

        if(0 > get_final_url(pValue,objItem->valuestring,final_url))
            break;

        DpptJiangSuConnect(&hEtax->dppt,final_url,hDev);

        if (!(nChildRet = InitSecurityConfig(&hEtax->dppt))) {
        }
        if(nChildRet){
            sprintf((char *)errinfo_uft8, "406初始化dppt服务器失败,错误状态为:%d", nChildRet);    
            break;
        }

        //步骤7 发票查询(列表)
        if ((nChildRet = EtaxQueryInvoice(hEtax, "/kpfw/hzqrxxSzzhQuery/v1/queryLzfpList",invoice_query_parametetr->fpcxsj)) < 0){
            sprintf((char *)errinfo_uft8, "406查询接口失败:%s\r\n",(const char *)evbuffer_pullup(hEtax->dppt.bufHttpRep, -1));
            break;                      
        }  

        nChildRet = evbuffer_get_length(hEtax->dppt.bufHttpRep);

        nEtaxBufData = (char *)malloc(nChildRet);
        if(nEtaxBufData == NULL)
            break;

        *(invoice_query_parametetr->fpcxdata) = NULL;
        *(invoice_query_parametetr->fpcxdata) = (char *)malloc(nChildRet);
        if(*(invoice_query_parametetr->fpcxdata) == NULL){    
            break;
        }
        evbuffer_copyout_from(hEtax->dppt.bufHttpRep,NULL, nEtaxBufData,nChildRet);
        EncordingConvert( "utf-8","gbk",nEtaxBufData, nChildRet, *(invoice_query_parametetr->fpcxdata), nChildRet);


        //结束
        nRet = RET_SUCCESS;
    }while (false);

    //释放内存
    if(nEtaxBufData)
        free(nEtaxBufData);
    nEtaxBufData = NULL;

    //释放内存
    if (root)
        cJSON_Delete(root);
    root = NULL;

    //释放内存
    if (jsonTpassConfig)
        cJSON_Delete(jsonTpassConfig);
    jsonTpassConfig  = NULL;

    if (uriTpassFull)
        evhttp_uri_free(uriTpassFull);

    if(pValue)
        free(pValue);

    if(hEtax->etax.user_args)
    {
        free(hEtax->etax.user_args);
        hEtax->etax.user_args = NULL;
    }

    return nRet;     
}

//红字确认单列表查询
int etax_jiangsu_hzqrxxQuery(HETAX hEtax, HDEV hDev,void *args,char *errinfo_uft8)
{
    struct evhttp_uri *uriTpassFull = NULL;

    INVOICE_QUERY *invoice_query_parametetr = (INVOICE_QUERY *)args;

    int nRet = ERR_GENERIC;
    int nChildRet = ERR_GENERIC;
    int nCount,i;

    char *pValue = NULL;
    char *nEtaxBufData = NULL;
    char szUserId[BS_LITTLE] = {0};
    char szBuf[BS_BLOCK * 6] = {0};
    char final_url[512] = {0x00};
    char *szTPassConfigJson = NULL;

    cJSON *jsonTpassConfig = NULL;
    cJSON *root = NULL, *objItem = NULL, *objItemChild = NULL;
    cJSON *objItemChild2 = NULL,*objItemToken  = NULL;

    if(NULL == invoice_query_parametetr)
    {
        return nRet;
    }

    hEtax->etax.user_args = (void *)(&header_array_jiangsu);
    memset(hEtax->etax.modelTaAddr.szTaxAuthorityURL,0x00,sizeof(hEtax->etax.modelTaAddr.szTaxAuthorityURL));
    strcpy(hEtax->etax.modelTaAddr.szTaxAuthorityURL,"https://dppt.jiangsu.chinatax.gov.cn:8443");
    memset(hEtax->etax.modelTaAddr.szInitRequestPage,0x00,sizeof(hEtax->etax.modelTaAddr.szInitRequestPage));
    strcpy(hEtax->etax.modelTaAddr.szInitRequestPage,"/szzhzz/spHandler?cdlj=digital-tax-account");    

    do{
        //步骤1 使用etax地址获取tpass地址
        if (nChildRet = EtaxHTTPInit(&hEtax->etax, hDev)) {
            sprintf((char *)errinfo_uft8, "306Steps 1:HTTP 初始化失败, nRet:%d", nChildRet);
            break;
        }
        if (nChildRet = EtaxHTTPOpen(&hEtax->etax)) {
            sprintf((char *)errinfo_uft8, "306Steps 1:HTTP 连接失败, nRet:%d", nChildRet);
            break;
        }
        if (!(hEtax->etax.ctxApp = calloc(1, sizeof(struct ContextHttpAppBridge)))){
            sprintf((char *)errinfo_uft8, "306内存申请失败,空间不足");
            break;
        }
        hEtax->etax.cbHttpClose = EtaxBridgeClose;
        //获取tpass地址
        //步骤3 从token服务器获取登陆凭证去登陆Etax,登陆完成后返回的用户列表在hiTPass->bufHttpRep中
        if (hEtax->etax.ctxCon.nHttpRepCode != 200 && hEtax->etax.ctxCon.nHttpRepCode != 302){
            sprintf((char *)errinfo_uft8, "406请求跳转地址失败,HTTP Code:%d",hEtax->etax.ctxCon.nHttpRepCode);  
            break;
        }
        if(hEtax->etax.ctxCon.nHttpRepCode == 200)
        {
            szTPassConfigJson = (char *)evbuffer_pullup(hEtax->etax.bufHttpRep, -1);
        }
        else if(hEtax->etax.ctxCon.nHttpRepCode == 302)
        {
            szTPassConfigJson = (char *)hEtax->etax.user_args;
        }
        //获取跳转tpass地址
        if(RET_SUCCESS != get_tpass_addr(szTPassConfigJson,&pValue,"1"))
        {
             sprintf((char *)errinfo_uft8, "406获取跳转地址错误,地址解析失败");
             break;
        }

        if (!(uriTpassFull = evhttp_uri_parse(pValue))){
            sprintf((char *)errinfo_uft8, "406获取跳转地址错误,地址转换失败"); 
            break;
        }

        if (!FillUploadAddrModelDynamic(TAADDR_ETAX_TPASS, hDev->szRegCode, pValue, "/",&hEtax->tpass.modelTaAddr)) {
            sprintf((char *)errinfo_uft8, "406获取跳转地址错误,地址解析失败");
            break;
        } 

        if (nChildRet = EtaxHTTPInit(&hEtax->tpass, hDev)) {
            sprintf((char *)errinfo_uft8, "306Steps 2:HTTP 初始化失败, nRet:%d", nChildRet);
            break;
        }

        if (nChildRet = EtaxHTTPOpen(&hEtax->tpass)) {
            sprintf((char *)errinfo_uft8, "306Steps 2:HTTP 连接失败, nRet:%d", nChildRet);
            break;
        }
        if (!(hEtax->tpass.ctxApp = calloc(1, sizeof(struct ContextHttpAppBridge)))){
            sprintf((char *)errinfo_uft8, "306内存申请失败,空间不足");
            break;
        }
        hEtax->tpass.cbHttpClose = EtaxBridgeClose;
        
        //生成SM4密钥、获取公钥、发送SM4密钥
        if ((nChildRet = TPassLoginAuthSm4AndPublickey(&hEtax->tpass,hDev->szRegCode))) {
            sprintf((char *)errinfo_uft8, "406请求公钥或者发送SM4密钥失败:%s",(const char *)evbuffer_pullup(hEtax->tpass.bufHttpRep, -1));
            break;
        }

        //获取税盘证书与税局地址进行签名验证
        if ((nChildRet = TPassLoginAuthHelloAndAuth(&hEtax->tpass, uriTpassFull))) {
            sprintf((char *)errinfo_uft8, "406签名验签服务器响应请求错误:%s",(const char *)evbuffer_pullup(hEtax->tpass.bufHttpRep, -1));
            break;
        }

        //步骤4 根据签名验证请求后返回的用户信息以及中台传入的用户信息进行比较，筛选出需要登陆的身份
        if (!(root = cJSON_Parse((const char *)evbuffer_pullup(hEtax->tpass.bufHttpRep, -1)))) {
            sprintf((char *)errinfo_uft8, "406登陆凭证中返回用户列表为空");
            break;
        }
        if (!(objItem = cJSON_GetObjectItem(root, "userList"))) {
            sprintf((char *)errinfo_uft8, "406登陆凭证中返回用户列表解析失败");
            break;
        }
        nCount = cJSON_GetArraySize(objItem);
        for (i = 0; i < nCount; i++) {
            objItemChild = cJSON_GetArrayItem(objItem, i);

            if(strlen(invoice_query_parametetr->idcard)){
                if (!(objItemChild2 = cJSON_GetObjectItem(objItemChild, "id_card")))
                    continue;
                if (strcmp(objItemChild2->valuestring, invoice_query_parametetr->idcard))
                    continue;
            }

            if(strlen(invoice_query_parametetr->mobile)){
                if (!(objItemChild2 = cJSON_GetObjectItem(objItemChild, "mobile")))
                    continue;
                if (strcmp(objItemChild2->valuestring, invoice_query_parametetr->mobile))
                    continue;
            }

            if(invoice_query_parametetr->realationtype == 0){         //是否需要切换企业
                if (!(objItemChild2 = cJSON_GetObjectItem(objItemChild, "relatedType")))
                    continue;
                if (strcmp(objItemChild2->valuestring, invoice_query_parametetr->related_type))
                    continue;
            }

            if (!(objItemChild2 = cJSON_GetObjectItem(objItemChild, "user_id")))
                continue;
            strcpy(szUserId, objItemChild2->valuestring);
            printf("szUserId=%s,sz_password=%s\r\n",szUserId,invoice_query_parametetr->sz_password);
            break;
        }
        if(!strlen(szUserId)){
            sprintf((char *)errinfo_uft8, "306用户列表中未检测到该用户信息,请核实登陆信息是否有误");
            break;
        }

        if (nChildRet = TPassLoginPasswordLoginDo(&hEtax->tpass, szUserId, invoice_query_parametetr->sz_password, szBuf,sizeof(szBuf))){
            if (!(root = cJSON_Parse(szBuf)))
                break;
            if (!(objItem = cJSON_GetObjectItem(root, "msg")))
                break;
            sprintf((char *)errinfo_uft8, "406登陆失败:%s",objItem->valuestring);
            break;
        }

        //步骤5 根据筛选出来的身份进行登陆
        if(invoice_query_parametetr->realationtype == 1){         //是否需要切换企业后查询
            if (nChildRet = TPassSelectRelationchange(&hEtax->tpass,invoice_query_parametetr->uniqueIdentity,invoice_query_parametetr->realationStatus, szBuf,sizeof(szBuf))){
                if (!(root = cJSON_Parse(szBuf)))
                    break;
                if (!(objItem = cJSON_GetObjectItem(root, "msg")))
                    break;
                sprintf((char *)errinfo_uft8, "406企业切换失败,%s",objItem->valuestring);
                break;
            } 
        }


        //步骤7 获取一键登陆需要的code参数和RPA需要的token参数
        printf("TPassSelectRelationchange data is %s\n", szBuf);
        if (!(root = cJSON_Parse(szBuf))){
            sprintf((char *)errinfo_uft8, "406企业切换失败,返回数据解析失败");
            break;
        }
        if (!(objItem = cJSON_GetObjectItem(root, "code"))){
            sprintf((char *)errinfo_uft8, "406企业切换失败,返回数据中无登陆参数");
            break;
        }

        printf(" = %s\r\n",objItem->valuestring);

        if(0 > get_final_url(pValue,objItem->valuestring,final_url))
            break;

        DpptJiangSuConnect(&hEtax->dppt,final_url,hDev);

        if (!(nChildRet = InitSecurityConfig(&hEtax->dppt))) {
        }
        if(nChildRet){
            sprintf((char *)errinfo_uft8, "406初始化dppt服务器失败,错误状态为:%d", nChildRet);    
            break;
        }

        //步骤7 发票查询(列表)
        if ((nChildRet = EtaxQueryInvoice(hEtax, "/kpfw/hzqrxxQuery/v1/queryHzqrxxForHzqrd",invoice_query_parametetr->fpcxsj)) < 0){
            sprintf((char *)errinfo_uft8, "406查询接口失败:%s\r\n",(const char *)evbuffer_pullup(hEtax->dppt.bufHttpRep, -1));
            break;                      
        }  

        nChildRet = evbuffer_get_length(hEtax->dppt.bufHttpRep);

        nEtaxBufData = (char *)malloc(nChildRet);
        if(nEtaxBufData == NULL)
            break;

        *(invoice_query_parametetr->fpcxdata) = NULL;
        *(invoice_query_parametetr->fpcxdata) = (char *)malloc(nChildRet);
        if(*(invoice_query_parametetr->fpcxdata) == NULL){    
            break;
        }
        evbuffer_copyout_from(hEtax->dppt.bufHttpRep,NULL, nEtaxBufData,nChildRet);
        EncordingConvert( "utf-8","gbk",nEtaxBufData, nChildRet, *(invoice_query_parametetr->fpcxdata), nChildRet);

        //结束
        nRet = RET_SUCCESS;
    }while (false);

    //释放内存
    if(nEtaxBufData)
        free(nEtaxBufData);
    nEtaxBufData = NULL;

    //释放内存
    if (root)
        cJSON_Delete(root);
    root = NULL;

    //释放内存
    if (jsonTpassConfig)
        cJSON_Delete(jsonTpassConfig);
    jsonTpassConfig  = NULL;

    if (uriTpassFull)
        evhttp_uri_free(uriTpassFull);

    if(pValue)
        free(pValue);

    if(hEtax->etax.user_args)
    {
        free(hEtax->etax.user_args);
        hEtax->etax.user_args = NULL;
    }

    return nRet;     
}

//红字确认单详情查询
int etax_jiangsu_hzqrxxQueryDetail(HETAX hEtax, HDEV hDev,void *args,char *errinfo_uft8)
{
    struct evhttp_uri *uriTpassFull = NULL;

    INVOICE_QUERY *invoice_query_parametetr = (INVOICE_QUERY *)args;

    int nRet = ERR_GENERIC;
    int nChildRet = ERR_GENERIC;
    int nCount,i;

    char *pValue = NULL;
    char *nEtaxBufData = NULL;
    char szUserId[BS_LITTLE] = {0};
    char szBuf[BS_BLOCK * 6] = {0};
    char final_url[512] = {0x00};
    char *szTPassConfigJson = NULL;

    cJSON *jsonTpassConfig = NULL;
    cJSON *root = NULL, *objItem = NULL, *objItemChild = NULL;
    cJSON *objItemChild2 = NULL,*objItemToken  = NULL;

    if(NULL == invoice_query_parametetr)
    {
        return nRet;
    }

    hEtax->etax.user_args = (void *)(&header_array_jiangsu);
    memset(hEtax->etax.modelTaAddr.szTaxAuthorityURL,0x00,sizeof(hEtax->etax.modelTaAddr.szTaxAuthorityURL));
    strcpy(hEtax->etax.modelTaAddr.szTaxAuthorityURL,"https://dppt.jiangsu.chinatax.gov.cn:8443");
    memset(hEtax->etax.modelTaAddr.szInitRequestPage,0x00,sizeof(hEtax->etax.modelTaAddr.szInitRequestPage));
    strcpy(hEtax->etax.modelTaAddr.szInitRequestPage,"/szzhzz/spHandler?cdlj=digital-tax-account");    

    do{
        //步骤1 使用etax地址获取tpass地址
        if (nChildRet = EtaxHTTPInit(&hEtax->etax, hDev)) {
            sprintf((char *)errinfo_uft8, "306Steps 1:HTTP 初始化失败, nRet:%d", nChildRet);
            break;
        }
        if (nChildRet = EtaxHTTPOpen(&hEtax->etax)) {
            sprintf((char *)errinfo_uft8, "306Steps 1:HTTP 连接失败, nRet:%d", nChildRet);
            break;
        }
        if (!(hEtax->etax.ctxApp = calloc(1, sizeof(struct ContextHttpAppBridge)))){
            sprintf((char *)errinfo_uft8, "306内存申请失败,空间不足");
            break;
        }
        hEtax->etax.cbHttpClose = EtaxBridgeClose;
        //获取tpass地址
        //步骤3 从token服务器获取登陆凭证去登陆Etax,登陆完成后返回的用户列表在hiTPass->bufHttpRep中
        if (hEtax->etax.ctxCon.nHttpRepCode != 200 && hEtax->etax.ctxCon.nHttpRepCode != 302){
            sprintf((char *)errinfo_uft8, "406请求跳转地址失败,HTTP Code:%d",hEtax->etax.ctxCon.nHttpRepCode);  
            break;
        }
        if(hEtax->etax.ctxCon.nHttpRepCode == 200)
        {
            szTPassConfigJson = (char *)evbuffer_pullup(hEtax->etax.bufHttpRep, -1);
        }
        else if(hEtax->etax.ctxCon.nHttpRepCode == 302)
        {
            szTPassConfigJson = (char *)hEtax->etax.user_args;
        }
        //获取跳转tpass地址
        if(RET_SUCCESS != get_tpass_addr(szTPassConfigJson,&pValue,"1"))
        {
             sprintf((char *)errinfo_uft8, "406获取跳转地址错误,地址解析失败");
             break;
        }

        if (!(uriTpassFull = evhttp_uri_parse(pValue))){
            sprintf((char *)errinfo_uft8, "406获取跳转地址错误,地址转换失败"); 
            break;
        }

        if (!FillUploadAddrModelDynamic(TAADDR_ETAX_TPASS, hDev->szRegCode, pValue, "/",&hEtax->tpass.modelTaAddr)) {
            sprintf((char *)errinfo_uft8, "406获取跳转地址错误,地址解析失败");
            break;
        } 

        if (nChildRet = EtaxHTTPInit(&hEtax->tpass, hDev)) {
            sprintf((char *)errinfo_uft8, "306Steps 2:HTTP 初始化失败, nRet:%d", nChildRet);
            break;
        }

        if (nChildRet = EtaxHTTPOpen(&hEtax->tpass)) {
            sprintf((char *)errinfo_uft8, "306Steps 2:HTTP 连接失败, nRet:%d", nChildRet);
            break;
        }
        if (!(hEtax->tpass.ctxApp = calloc(1, sizeof(struct ContextHttpAppBridge)))){
            sprintf((char *)errinfo_uft8, "306内存申请失败,空间不足");
            break;
        }
        hEtax->tpass.cbHttpClose = EtaxBridgeClose;
        
        //生成SM4密钥、获取公钥、发送SM4密钥
        if ((nChildRet = TPassLoginAuthSm4AndPublickey(&hEtax->tpass,hDev->szRegCode))) {
            sprintf((char *)errinfo_uft8, "406请求公钥或者发送SM4密钥失败:%s",(const char *)evbuffer_pullup(hEtax->tpass.bufHttpRep, -1));
            break;
        }

        //获取税盘证书与税局地址进行签名验证
        if ((nChildRet = TPassLoginAuthHelloAndAuth(&hEtax->tpass, uriTpassFull))) {
            sprintf((char *)errinfo_uft8, "406签名验签服务器响应请求错误:%s",(const char *)evbuffer_pullup(hEtax->tpass.bufHttpRep, -1));
            break;
        }

        //步骤4 根据签名验证请求后返回的用户信息以及中台传入的用户信息进行比较，筛选出需要登陆的身份
        if (!(root = cJSON_Parse((const char *)evbuffer_pullup(hEtax->tpass.bufHttpRep, -1)))) {
            sprintf((char *)errinfo_uft8, "406登陆凭证中返回用户列表为空");
            break;
        }
        if (!(objItem = cJSON_GetObjectItem(root, "userList"))) {
            sprintf((char *)errinfo_uft8, "406登陆凭证中返回用户列表解析失败");
            break;
        }
        nCount = cJSON_GetArraySize(objItem);
        for (i = 0; i < nCount; i++) {
            objItemChild = cJSON_GetArrayItem(objItem, i);

            if(strlen(invoice_query_parametetr->idcard)){
                if (!(objItemChild2 = cJSON_GetObjectItem(objItemChild, "id_card")))
                    continue;
                if (strcmp(objItemChild2->valuestring, invoice_query_parametetr->idcard))
                    continue;
            }

            if(strlen(invoice_query_parametetr->mobile)){
                if (!(objItemChild2 = cJSON_GetObjectItem(objItemChild, "mobile")))
                    continue;
                if (strcmp(objItemChild2->valuestring, invoice_query_parametetr->mobile))
                    continue;
            }

            if(invoice_query_parametetr->realationtype == 0){         //是否需要切换企业
                if (!(objItemChild2 = cJSON_GetObjectItem(objItemChild, "relatedType")))
                    continue;
                if (strcmp(objItemChild2->valuestring, invoice_query_parametetr->related_type))
                    continue;
            }

            if (!(objItemChild2 = cJSON_GetObjectItem(objItemChild, "user_id")))
                continue;
            strcpy(szUserId, objItemChild2->valuestring);
            printf("szUserId=%s,sz_password=%s\r\n",szUserId,invoice_query_parametetr->sz_password);
            break;
        }
        if(!strlen(szUserId)){
            sprintf((char *)errinfo_uft8, "306用户列表中未检测到该用户信息,请核实登陆信息是否有误");
            break;
        }

        if (nChildRet = TPassLoginPasswordLoginDo(&hEtax->tpass, szUserId, invoice_query_parametetr->sz_password, szBuf,sizeof(szBuf))){
            if (!(root = cJSON_Parse(szBuf)))
                break;
            if (!(objItem = cJSON_GetObjectItem(root, "msg")))
                break;
            sprintf((char *)errinfo_uft8, "406登陆失败:%s",objItem->valuestring);
            break;
        }

        //步骤5 根据筛选出来的身份进行登陆
        if(invoice_query_parametetr->realationtype == 1){         //是否需要切换企业后查询
            if (nChildRet = TPassSelectRelationchange(&hEtax->tpass,invoice_query_parametetr->uniqueIdentity,invoice_query_parametetr->realationStatus, szBuf,sizeof(szBuf))){
                if (!(root = cJSON_Parse(szBuf)))
                    break;
                if (!(objItem = cJSON_GetObjectItem(root, "msg")))
                    break;
                sprintf((char *)errinfo_uft8, "406企业切换失败,%s",objItem->valuestring);
                break;
            } 
        }


        //步骤7 获取一键登陆需要的code参数和RPA需要的token参数
        printf("TPassSelectRelationchange data is %s\n", szBuf);
        if (!(root = cJSON_Parse(szBuf))){
            sprintf((char *)errinfo_uft8, "406企业切换失败,返回数据解析失败");
            break;
        }
        if (!(objItem = cJSON_GetObjectItem(root, "code"))){
            sprintf((char *)errinfo_uft8, "406企业切换失败,返回数据中无登陆参数");
            break;
        }

        printf(" = %s\r\n",objItem->valuestring);

        if(0 > get_final_url(pValue,objItem->valuestring,final_url))
            break;

        DpptJiangSuConnect(&hEtax->dppt,final_url,hDev);

        if (!(nChildRet = InitSecurityConfig(&hEtax->dppt))) {
        }
        if(nChildRet){
            sprintf((char *)errinfo_uft8, "406初始化dppt服务器失败,错误状态为:%d", nChildRet);    
            break;
        }

        //步骤7 发票查询(列表)
        if ((nChildRet = EtaxQueryInvoice(hEtax, "/kpfw/hzqrxxQuery/v1/queryHzqrxxDetail",invoice_query_parametetr->fpcxsj)) < 0){
            sprintf((char *)errinfo_uft8, "406查询接口失败:%s\r\n",(const char *)evbuffer_pullup(hEtax->dppt.bufHttpRep, -1));
            break;                      
        }  

        nChildRet = evbuffer_get_length(hEtax->dppt.bufHttpRep);

        nEtaxBufData = (char *)malloc(nChildRet);
        if(nEtaxBufData == NULL)
            break;

        *(invoice_query_parametetr->fpcxdata) = NULL;
        *(invoice_query_parametetr->fpcxdata) = (char *)malloc(nChildRet);
        if(*(invoice_query_parametetr->fpcxdata) == NULL){    
            break;
        }
        evbuffer_copyout_from(hEtax->dppt.bufHttpRep,NULL, nEtaxBufData,nChildRet);
        EncordingConvert( "utf-8","gbk",nEtaxBufData, nChildRet, *(invoice_query_parametetr->fpcxdata), nChildRet);


        //结束
        nRet = RET_SUCCESS;
    }while (false);

    //释放内存
    if(nEtaxBufData)
        free(nEtaxBufData);
    nEtaxBufData = NULL;

    //释放内存
    if (root)
        cJSON_Delete(root);
    root = NULL;

    //释放内存
    if (jsonTpassConfig)
        cJSON_Delete(jsonTpassConfig);
    jsonTpassConfig  = NULL;

    if (uriTpassFull)
        evhttp_uri_free(uriTpassFull);

    if(pValue)
        free(pValue);

    if(hEtax->etax.user_args)
    {
        free(hEtax->etax.user_args);
        hEtax->etax.user_args = NULL;
    }

    return nRet; 
}

//红字确认单开具
int etax_jiangsu_hzqrxxSave(HETAX hEtax, HDEV hDev,void *args,char *errinfo_uft8)
{
    struct evhttp_uri *uriTpassFull = NULL;

    INVOICE_QUERY *invoice_query_parametetr = (INVOICE_QUERY *)args;

    int nRet = ERR_GENERIC;
    int nChildRet = ERR_GENERIC;
    int nCount,i;

    char *pValue = NULL;
    char *nEtaxBufData = NULL;
    char szUserId[BS_LITTLE] = {0};
    char szBuf[BS_BLOCK * 6] = {0};
    char final_url[512] = {0x00};
    char *szTPassConfigJson = NULL;

    cJSON *jsonTpassConfig = NULL;
    cJSON *root = NULL, *objItem = NULL, *objItemChild = NULL;
    cJSON *objItemChild2 = NULL,*objItemToken  = NULL;

    if(NULL == invoice_query_parametetr)
    {
        return nRet;
    }

    hEtax->etax.user_args = (void *)(&header_array_jiangsu);
    memset(hEtax->etax.modelTaAddr.szTaxAuthorityURL,0x00,sizeof(hEtax->etax.modelTaAddr.szTaxAuthorityURL));
    strcpy(hEtax->etax.modelTaAddr.szTaxAuthorityURL,"https://dppt.jiangsu.chinatax.gov.cn:8443");
    memset(hEtax->etax.modelTaAddr.szInitRequestPage,0x00,sizeof(hEtax->etax.modelTaAddr.szInitRequestPage));
    strcpy(hEtax->etax.modelTaAddr.szInitRequestPage,"/szzhzz/spHandler?cdlj=digital-tax-account");    

    do{
        //步骤1 使用etax地址获取tpass地址
        if (nChildRet = EtaxHTTPInit(&hEtax->etax, hDev)) {
            sprintf((char *)errinfo_uft8, "306Steps 1:HTTP 初始化失败, nRet:%d", nChildRet);
            break;
        }
        if (nChildRet = EtaxHTTPOpen(&hEtax->etax)) {
            sprintf((char *)errinfo_uft8, "306Steps 1:HTTP 连接失败, nRet:%d", nChildRet);
            break;
        }
        if (!(hEtax->etax.ctxApp = calloc(1, sizeof(struct ContextHttpAppBridge)))){
            sprintf((char *)errinfo_uft8, "306内存申请失败,空间不足");
            break;
        }
        hEtax->etax.cbHttpClose = EtaxBridgeClose;
        //获取tpass地址
        //步骤3 从token服务器获取登陆凭证去登陆Etax,登陆完成后返回的用户列表在hiTPass->bufHttpRep中
        if (hEtax->etax.ctxCon.nHttpRepCode != 200 && hEtax->etax.ctxCon.nHttpRepCode != 302){
            sprintf((char *)errinfo_uft8, "406请求跳转地址失败,HTTP Code:%d",hEtax->etax.ctxCon.nHttpRepCode);  
            break;
        }
        if(hEtax->etax.ctxCon.nHttpRepCode == 200)
        {
            szTPassConfigJson = (char *)evbuffer_pullup(hEtax->etax.bufHttpRep, -1);
        }
        else if(hEtax->etax.ctxCon.nHttpRepCode == 302)
        {
            szTPassConfigJson = (char *)hEtax->etax.user_args;
        }
        //获取跳转tpass地址
        if(RET_SUCCESS != get_tpass_addr(szTPassConfigJson,&pValue,"1"))
        {
             sprintf((char *)errinfo_uft8, "406获取跳转地址错误,地址解析失败");
             break;
        }

        if (!(uriTpassFull = evhttp_uri_parse(pValue))){
            sprintf((char *)errinfo_uft8, "406获取跳转地址错误,地址转换失败"); 
            break;
        }

        if (!FillUploadAddrModelDynamic(TAADDR_ETAX_TPASS, hDev->szRegCode, pValue, "/",&hEtax->tpass.modelTaAddr)) {
            sprintf((char *)errinfo_uft8, "406获取跳转地址错误,地址解析失败");
            break;
        } 

        if (nChildRet = EtaxHTTPInit(&hEtax->tpass, hDev)) {
            sprintf((char *)errinfo_uft8, "306Steps 2:HTTP 初始化失败, nRet:%d", nChildRet);
            break;
        }

        if (nChildRet = EtaxHTTPOpen(&hEtax->tpass)) {
            sprintf((char *)errinfo_uft8, "306Steps 2:HTTP 连接失败, nRet:%d", nChildRet);
            break;
        }
        if (!(hEtax->tpass.ctxApp = calloc(1, sizeof(struct ContextHttpAppBridge)))){
            sprintf((char *)errinfo_uft8, "306内存申请失败,空间不足");
            break;
        }
        hEtax->tpass.cbHttpClose = EtaxBridgeClose;
        
        //生成SM4密钥、获取公钥、发送SM4密钥
        if ((nChildRet = TPassLoginAuthSm4AndPublickey(&hEtax->tpass,hDev->szRegCode))) {
            sprintf((char *)errinfo_uft8, "406请求公钥或者发送SM4密钥失败:%s",(const char *)evbuffer_pullup(hEtax->tpass.bufHttpRep, -1));
            break;
        }

        //获取税盘证书与税局地址进行签名验证
        if ((nChildRet = TPassLoginAuthHelloAndAuth(&hEtax->tpass, uriTpassFull))) {
            sprintf((char *)errinfo_uft8, "406签名验签服务器响应请求错误:%s",(const char *)evbuffer_pullup(hEtax->tpass.bufHttpRep, -1));
            break;
        }

        //步骤4 根据签名验证请求后返回的用户信息以及中台传入的用户信息进行比较，筛选出需要登陆的身份
        if (!(root = cJSON_Parse((const char *)evbuffer_pullup(hEtax->tpass.bufHttpRep, -1)))) {
            sprintf((char *)errinfo_uft8, "406登陆凭证中返回用户列表为空");
            break;
        }
        if (!(objItem = cJSON_GetObjectItem(root, "userList"))) {
            sprintf((char *)errinfo_uft8, "406登陆凭证中返回用户列表解析失败");
            break;
        }
        nCount = cJSON_GetArraySize(objItem);
        for (i = 0; i < nCount; i++) {
            objItemChild = cJSON_GetArrayItem(objItem, i);

            if(strlen(invoice_query_parametetr->idcard)){
                if (!(objItemChild2 = cJSON_GetObjectItem(objItemChild, "id_card")))
                    continue;
                if (strcmp(objItemChild2->valuestring, invoice_query_parametetr->idcard))
                    continue;
            }

            if(strlen(invoice_query_parametetr->mobile)){
                if (!(objItemChild2 = cJSON_GetObjectItem(objItemChild, "mobile")))
                    continue;
                if (strcmp(objItemChild2->valuestring, invoice_query_parametetr->mobile))
                    continue;
            }

            if(invoice_query_parametetr->realationtype == 0){         //是否需要切换企业
                if (!(objItemChild2 = cJSON_GetObjectItem(objItemChild, "relatedType")))
                    continue;
                if (strcmp(objItemChild2->valuestring, invoice_query_parametetr->related_type))
                    continue;
            }

            if (!(objItemChild2 = cJSON_GetObjectItem(objItemChild, "user_id")))
                continue;
            strcpy(szUserId, objItemChild2->valuestring);
            printf("szUserId=%s,sz_password=%s\r\n",szUserId,invoice_query_parametetr->sz_password);
            break;
        }
        if(!strlen(szUserId)){
            sprintf((char *)errinfo_uft8, "306用户列表中未检测到该用户信息,请核实登陆信息是否有误");
            break;
        }

        if (nChildRet = TPassLoginPasswordLoginDo(&hEtax->tpass, szUserId, invoice_query_parametetr->sz_password, szBuf,sizeof(szBuf))){
            if (!(root = cJSON_Parse(szBuf)))
                break;
            if (!(objItem = cJSON_GetObjectItem(root, "msg")))
                break;
            sprintf((char *)errinfo_uft8, "406登陆失败:%s",objItem->valuestring);
            break;
        }

        //步骤5 根据筛选出来的身份进行登陆
        if(invoice_query_parametetr->realationtype == 1){         //是否需要切换企业后查询
            if (nChildRet = TPassSelectRelationchange(&hEtax->tpass,invoice_query_parametetr->uniqueIdentity,invoice_query_parametetr->realationStatus, szBuf,sizeof(szBuf))){
                if (!(root = cJSON_Parse(szBuf)))
                    break;
                if (!(objItem = cJSON_GetObjectItem(root, "msg")))
                    break;
                sprintf((char *)errinfo_uft8, "406企业切换失败,%s",objItem->valuestring);
                break;
            } 
        }


        //步骤7 获取一键登陆需要的code参数和RPA需要的token参数
        printf("TPassSelectRelationchange data is %s\n", szBuf);
        if (!(root = cJSON_Parse(szBuf))){
            sprintf((char *)errinfo_uft8, "406企业切换失败,返回数据解析失败");
            break;
        }
        if (!(objItem = cJSON_GetObjectItem(root, "code"))){
            sprintf((char *)errinfo_uft8, "406企业切换失败,返回数据中无登陆参数");
            break;
        }

        printf(" = %s\r\n",objItem->valuestring);

        if(0 > get_final_url(pValue,objItem->valuestring,final_url))
            break;

        DpptJiangSuConnect(&hEtax->dppt,final_url,hDev);

        if (!(nChildRet = InitSecurityConfig(&hEtax->dppt))) {
        }
        if(nChildRet){
            sprintf((char *)errinfo_uft8, "406初始化dppt服务器失败,错误状态为:%d", nChildRet);    
            break;
        }

        //步骤7 发票查询(列表)
        if ((nChildRet = EtaxQueryInvoice(hEtax, "/kpfw/hzqrxxSave/v1/saveHzqrxxForHztzd",invoice_query_parametetr->fpcxsj)) < 0){
            sprintf((char *)errinfo_uft8, "406查询接口失败:%s\r\n",(const char *)evbuffer_pullup(hEtax->dppt.bufHttpRep, -1));
            break;                      
        }  

        nChildRet = evbuffer_get_length(hEtax->dppt.bufHttpRep);

        nEtaxBufData = (char *)malloc(nChildRet);
        if(nEtaxBufData == NULL)
            break;

        *(invoice_query_parametetr->fpcxdata) = NULL;
        *(invoice_query_parametetr->fpcxdata) = (char *)malloc(nChildRet);
        if(*(invoice_query_parametetr->fpcxdata) == NULL){    
            break;
        }
        evbuffer_copyout_from(hEtax->dppt.bufHttpRep,NULL, nEtaxBufData,nChildRet);
        EncordingConvert( "utf-8","gbk",nEtaxBufData, nChildRet, *(invoice_query_parametetr->fpcxdata), nChildRet);

        //结束
        nRet = RET_SUCCESS;
    }while (false);

    //释放内存
    if(nEtaxBufData)
        free(nEtaxBufData);
    nEtaxBufData = NULL;

    //释放内存
    if (root)
        cJSON_Delete(root);
    root = NULL;

    //释放内存
    if (jsonTpassConfig)
        cJSON_Delete(jsonTpassConfig);
    jsonTpassConfig  = NULL;

    if (uriTpassFull)
        evhttp_uri_free(uriTpassFull);

    if(pValue)
        free(pValue);

    if(hEtax->etax.user_args)
    {
        free(hEtax->etax.user_args);
        hEtax->etax.user_args = NULL;
    }

    return nRet;    
}

int etax_jiangsu_fjxxpeizhi_query(HETAX hEtax, HDEV hDev,void *args,char *errinfo_uft8)
{
    struct evhttp_uri *uriTpassFull = NULL;

    FJXXCJMB_MODE *fjxxpeizhi_parametetr = (FJXXCJMB_MODE *)args;

    int nRet = ERR_GENERIC;
    int nChildRet = ERR_GENERIC;
    int nCount,i;

    char *pValue = NULL;
    char *nEtaxBufData = NULL;
    char szUserId[BS_LITTLE] = {0};
    char szBuf[BS_BLOCK * 6] = {0};
    char final_url[512] = {0x00};
    char *szTPassConfigJson = NULL;

    cJSON *jsonTpassConfig = NULL;
    cJSON *root = NULL, *objItem = NULL, *objItemChild = NULL;
    cJSON *objItemChild2 = NULL,*objItemToken  = NULL;

    if(NULL == fjxxpeizhi_parametetr)
    {
        return nRet;
    }

    hEtax->etax.user_args = (void *)(&header_array_jiangsu);
    memset(hEtax->etax.modelTaAddr.szTaxAuthorityURL,0x00,sizeof(hEtax->etax.modelTaAddr.szTaxAuthorityURL));
    strcpy(hEtax->etax.modelTaAddr.szTaxAuthorityURL,"https://dppt.jiangsu.chinatax.gov.cn:8443");
    memset(hEtax->etax.modelTaAddr.szInitRequestPage,0x00,sizeof(hEtax->etax.modelTaAddr.szInitRequestPage));
    strcpy(hEtax->etax.modelTaAddr.szInitRequestPage,"/szzhzz/spHandler?cdlj=digital-tax-account");    

    do{
        //步骤1 使用etax地址获取tpass地址
        if (nChildRet = EtaxHTTPInit(&hEtax->etax, hDev)) {
            sprintf((char *)errinfo_uft8, "306Steps 1:HTTP 初始化失败, nRet:%d", nChildRet);
            break;
        }
        if (nChildRet = EtaxHTTPOpen(&hEtax->etax)) {
            sprintf((char *)errinfo_uft8, "306Steps 1:HTTP 连接失败, nRet:%d", nChildRet);
            break;
        }
        if (!(hEtax->etax.ctxApp = calloc(1, sizeof(struct ContextHttpAppBridge)))){
            sprintf((char *)errinfo_uft8, "306内存申请失败,空间不足");
            break;
        }
        hEtax->etax.cbHttpClose = EtaxBridgeClose;
        //获取tpass地址
        //步骤3 从token服务器获取登陆凭证去登陆Etax,登陆完成后返回的用户列表在hiTPass->bufHttpRep中
        if (hEtax->etax.ctxCon.nHttpRepCode != 200 && hEtax->etax.ctxCon.nHttpRepCode != 302){
            sprintf((char *)errinfo_uft8, "406请求跳转地址失败,HTTP Code:%d",hEtax->etax.ctxCon.nHttpRepCode);  
            break;
        }
        if(hEtax->etax.ctxCon.nHttpRepCode == 200)
        {
            szTPassConfigJson = (char *)evbuffer_pullup(hEtax->etax.bufHttpRep, -1);
        }
        else if(hEtax->etax.ctxCon.nHttpRepCode == 302)
        {
            szTPassConfigJson = (char *)hEtax->etax.user_args;
        }
        //获取跳转tpass地址
        if(RET_SUCCESS != get_tpass_addr(szTPassConfigJson,&pValue,"1"))
        {
             sprintf((char *)errinfo_uft8, "406获取跳转地址错误,地址解析失败");
             break;
        }

        if (!(uriTpassFull = evhttp_uri_parse(pValue))){
            sprintf((char *)errinfo_uft8, "406获取跳转地址错误,地址转换失败"); 
            break;
        }

        if (!FillUploadAddrModelDynamic(TAADDR_ETAX_TPASS, hDev->szRegCode, pValue, "/",&hEtax->tpass.modelTaAddr)) {
            sprintf((char *)errinfo_uft8, "406获取跳转地址错误,地址解析失败");
            break;
        } 

        if (nChildRet = EtaxHTTPInit(&hEtax->tpass, hDev)) {
            sprintf((char *)errinfo_uft8, "306Steps 2:HTTP 初始化失败, nRet:%d", nChildRet);
            break;
        }

        if (nChildRet = EtaxHTTPOpen(&hEtax->tpass)) {
            sprintf((char *)errinfo_uft8, "306Steps 2:HTTP 连接失败, nRet:%d", nChildRet);
            break;
        }
        if (!(hEtax->tpass.ctxApp = calloc(1, sizeof(struct ContextHttpAppBridge)))){
            sprintf((char *)errinfo_uft8, "306内存申请失败,空间不足");
            break;
        }
        hEtax->tpass.cbHttpClose = EtaxBridgeClose;
        
        //生成SM4密钥、获取公钥、发送SM4密钥
        if ((nChildRet = TPassLoginAuthSm4AndPublickey(&hEtax->tpass,hDev->szRegCode))) {
            sprintf((char *)errinfo_uft8, "406请求公钥或者发送SM4密钥失败:%s",(const char *)evbuffer_pullup(hEtax->tpass.bufHttpRep, -1));
            break;
        }

        //获取税盘证书与税局地址进行签名验证
        if ((nChildRet = TPassLoginAuthHelloAndAuth(&hEtax->tpass, uriTpassFull))) {
            sprintf((char *)errinfo_uft8, "406签名验签服务器响应请求错误:%s",(const char *)evbuffer_pullup(hEtax->tpass.bufHttpRep, -1));
            break;
        }

        //步骤4 根据签名验证请求后返回的用户信息以及中台传入的用户信息进行比较，筛选出需要登陆的身份
        if (!(root = cJSON_Parse((const char *)evbuffer_pullup(hEtax->tpass.bufHttpRep, -1)))) {
            sprintf((char *)errinfo_uft8, "406登陆凭证中返回用户列表为空");
            break;
        }
        if (!(objItem = cJSON_GetObjectItem(root, "userList"))) {
            sprintf((char *)errinfo_uft8, "406登陆凭证中返回用户列表解析失败");
            break;
        }
        nCount = cJSON_GetArraySize(objItem);
        for (i = 0; i < nCount; i++) {
            objItemChild = cJSON_GetArrayItem(objItem, i);

            if(strlen(fjxxpeizhi_parametetr->idcard)){
                if (!(objItemChild2 = cJSON_GetObjectItem(objItemChild, "id_card")))
                    continue;
                if (strcmp(objItemChild2->valuestring, fjxxpeizhi_parametetr->idcard))
                    continue;
            }

            if(strlen(fjxxpeizhi_parametetr->mobile)){
                if (!(objItemChild2 = cJSON_GetObjectItem(objItemChild, "mobile")))
                    continue;
                if (strcmp(objItemChild2->valuestring, fjxxpeizhi_parametetr->mobile))
                    continue;
            }

            if(fjxxpeizhi_parametetr->realationtype == 0){         //是否需要切换企业
                if (!(objItemChild2 = cJSON_GetObjectItem(objItemChild, "relatedType")))
                    continue;
                if (strcmp(objItemChild2->valuestring, fjxxpeizhi_parametetr->related_type))
                    continue;
            }

            if (!(objItemChild2 = cJSON_GetObjectItem(objItemChild, "user_id")))
                continue;
            strcpy(szUserId, objItemChild2->valuestring);
            printf("szUserId=%s,sz_password=%s\r\n",szUserId,fjxxpeizhi_parametetr->sz_password);
            break;
        }
        if(!strlen(szUserId)){
            sprintf((char *)errinfo_uft8, "306用户列表中未检测到该用户信息,请核实登陆信息是否有误");
            break;
        }

        if (nChildRet = TPassLoginPasswordLoginDo(&hEtax->tpass, szUserId, fjxxpeizhi_parametetr->sz_password, szBuf,sizeof(szBuf))){
            if (!(root = cJSON_Parse(szBuf)))
                break;
            if (!(objItem = cJSON_GetObjectItem(root, "msg")))
                break;
            sprintf((char *)errinfo_uft8, "406登陆失败:%s",objItem->valuestring);
            break;
        }

        //步骤5 根据筛选出来的身份进行登陆
        if(fjxxpeizhi_parametetr->realationtype == 1){         //是否需要切换企业后查询
            if (nChildRet = TPassSelectRelationchange(&hEtax->tpass,fjxxpeizhi_parametetr->uniqueIdentity,fjxxpeizhi_parametetr->realationStatus, szBuf,sizeof(szBuf))){
                if (!(root = cJSON_Parse(szBuf)))
                    break;
                if (!(objItem = cJSON_GetObjectItem(root, "msg")))
                    break;
                sprintf((char *)errinfo_uft8, "406企业切换失败,%s",objItem->valuestring);
                break;
            } 
        }


        //步骤7 获取一键登陆需要的code参数和RPA需要的token参数
        printf("TPassSelectRelationchange data is %s\n", szBuf);
        if (!(root = cJSON_Parse(szBuf))){
            sprintf((char *)errinfo_uft8, "406企业切换失败,返回数据解析失败");
            break;
        }
        if (!(objItem = cJSON_GetObjectItem(root, "code"))){
            sprintf((char *)errinfo_uft8, "406企业切换失败,返回数据中无登陆参数");
            break;
        }

        printf(" = %s\r\n",objItem->valuestring);

        if(0 > get_final_url(pValue,objItem->valuestring,final_url))
            break;

        DpptJiangSuConnect(&hEtax->dppt,final_url,hDev);

        if (!(nChildRet = InitSecurityConfig(&hEtax->dppt))) {
        }
        if(nChildRet){
            sprintf((char *)errinfo_uft8, "406初始化dppt服务器失败,错误状态为:%d", nChildRet);    
            break;
        }

        if(atoi(fjxxpeizhi_parametetr->mode) == 0){
            if ((nChildRet = EtaxQueryInvoice(hEtax, "/jcxxwh/jcxxFpfjysxx/v1/create",fjxxpeizhi_parametetr->fpcxsj)) < 0){
                sprintf((char *)errinfo_uft8, "406附加信息创建失败:%s",evbuffer_pullup(hEtax->dppt.bufHttpRep, -1));
                break;                     
            }  
        }

        if(atoi(fjxxpeizhi_parametetr->mode) == 1){
            if ((nChildRet = EtaxQueryInvoice(hEtax, "/jcxxwh/jcxxFpfjysxx/v1/modify",fjxxpeizhi_parametetr->fpcxsj)) < 0){
                sprintf((char *)errinfo_uft8, "406附加信息修改失败:%s",evbuffer_pullup(hEtax->dppt.bufHttpRep, -1));
                break;                     
            }                     
        }

        if(atoi(fjxxpeizhi_parametetr->mode) == 2){
            if ((nChildRet = EtaxQueryInvoice(hEtax, "/jcxxwh/jcxxFpfjysxx/v1/delete",fjxxpeizhi_parametetr->fpcxsj)) < 0){
                sprintf((char *)errinfo_uft8, "406附加信息删除失败:%s",evbuffer_pullup(hEtax->dppt.bufHttpRep, -1));
                break;                     
            }                      
        }

        if(atoi(fjxxpeizhi_parametetr->mode) == 3){
            if ((nChildRet = EtaxQueryInvoice(hEtax, "/jcxxwh/jcxxFpfjysxx/v1/queryPageList",fjxxpeizhi_parametetr->fpcxsj)) < 0){
                sprintf((char *)errinfo_uft8, "406附加信息列表查询失败:%s",evbuffer_pullup(hEtax->dppt.bufHttpRep, -1));
                break;                     
            }                      
        }
        else 
        {
            if ((nChildRet = EtaxQueryInvoice(hEtax, "/jcxxwh/jcxxFpfjysxx/v1/queryPageList","{\"fjysxmmc\":\"\",\"order\":\"desc\",\"sortBy\":\"Lrrq\",\"pageNumber\":1,\"pageSize\":100}")) < 0){
                sprintf((char *)errinfo_uft8, "406附加信息和场景模版接口请求失败:%s",evbuffer_pullup(hEtax->dppt.bufHttpRep, -1));
                break;                      
            } 
        }

        nChildRet = evbuffer_get_length(hEtax->dppt.bufHttpRep);
        printf("nChildRet = %d\r\n,%s\r\n",nChildRet,evbuffer_pullup(hEtax->dppt.bufHttpRep, -1));

        int ChildRetLen = nChildRet + 1024;

        *(fjxxpeizhi_parametetr->fpcxdata) = NULL;
        *(fjxxpeizhi_parametetr->fpcxdata) = (char *)malloc(ChildRetLen);
        if(*(fjxxpeizhi_parametetr->fpcxdata) == NULL){    
            break;
        }
		asc_compress_base64(evbuffer_pullup(hEtax->dppt.bufHttpRep, -1), nChildRet, *(fjxxpeizhi_parametetr->fpcxdata), &ChildRetLen);
        //结束
        nRet = RET_SUCCESS;
    }while (false);

    //释放内存
    if(nEtaxBufData)
        free(nEtaxBufData);
    nEtaxBufData = NULL;

    //释放内存
    if (root)
        cJSON_Delete(root);
    root = NULL;

    //释放内存
    if (jsonTpassConfig)
        cJSON_Delete(jsonTpassConfig);
    jsonTpassConfig  = NULL;

    if (uriTpassFull)
        evhttp_uri_free(uriTpassFull);

    if(pValue)
        free(pValue);

    if(hEtax->etax.user_args)
    {
        free(hEtax->etax.user_args);
        hEtax->etax.user_args = NULL;
    }

    return nRet;    
}

int etax_jiangsu_cjmbpeizhi_query(HETAX hEtax, HDEV hDev,void *args,char *errinfo_uft8)
{
    struct evhttp_uri *uriTpassFull = NULL;

    FJXXCJMB_MODE *cjmbpeizhi_parametetr = (FJXXCJMB_MODE *)args;

    int nRet = ERR_GENERIC;
    int nChildRet = ERR_GENERIC;
    int nCount,i;

    char *pValue = NULL;
    char *nEtaxBufData = NULL;
    char szUserId[BS_LITTLE] = {0};
    char szBuf[BS_BLOCK * 6] = {0};
    char final_url[512] = {0x00};
    char *szTPassConfigJson = NULL;

    cJSON *jsonTpassConfig = NULL;
    cJSON *root = NULL, *objItem = NULL, *objItemChild = NULL;
    cJSON *objItemChild2 = NULL,*objItemToken  = NULL;

    if(NULL == cjmbpeizhi_parametetr)
    {
        return nRet;
    }

    hEtax->etax.user_args = (void *)(&header_array_jiangsu);
    memset(hEtax->etax.modelTaAddr.szTaxAuthorityURL,0x00,sizeof(hEtax->etax.modelTaAddr.szTaxAuthorityURL));
    strcpy(hEtax->etax.modelTaAddr.szTaxAuthorityURL,"https://dppt.jiangsu.chinatax.gov.cn:8443");
    memset(hEtax->etax.modelTaAddr.szInitRequestPage,0x00,sizeof(hEtax->etax.modelTaAddr.szInitRequestPage));
    strcpy(hEtax->etax.modelTaAddr.szInitRequestPage,"/szzhzz/spHandler?cdlj=digital-tax-account");    

    do{
        //步骤1 使用etax地址获取tpass地址
        if (nChildRet = EtaxHTTPInit(&hEtax->etax, hDev)) {
            sprintf((char *)errinfo_uft8, "306Steps 1:HTTP 初始化失败, nRet:%d", nChildRet);
            break;
        }
        if (nChildRet = EtaxHTTPOpen(&hEtax->etax)) {
            sprintf((char *)errinfo_uft8, "306Steps 1:HTTP 连接失败, nRet:%d", nChildRet);
            break;
        }
        if (!(hEtax->etax.ctxApp = calloc(1, sizeof(struct ContextHttpAppBridge)))){
            sprintf((char *)errinfo_uft8, "306内存申请失败,空间不足");
            break;
        }
        hEtax->etax.cbHttpClose = EtaxBridgeClose;
        //获取tpass地址
        //步骤3 从token服务器获取登陆凭证去登陆Etax,登陆完成后返回的用户列表在hiTPass->bufHttpRep中
        if (hEtax->etax.ctxCon.nHttpRepCode != 200 && hEtax->etax.ctxCon.nHttpRepCode != 302){
            sprintf((char *)errinfo_uft8, "406请求跳转地址失败,HTTP Code:%d",hEtax->etax.ctxCon.nHttpRepCode);  
            break;
        }
        if(hEtax->etax.ctxCon.nHttpRepCode == 200)
        {
            szTPassConfigJson = (char *)evbuffer_pullup(hEtax->etax.bufHttpRep, -1);
        }
        else if(hEtax->etax.ctxCon.nHttpRepCode == 302)
        {
            szTPassConfigJson = (char *)hEtax->etax.user_args;
        }
        //获取跳转tpass地址
        if(RET_SUCCESS != get_tpass_addr(szTPassConfigJson,&pValue,"1"))
        {
             sprintf((char *)errinfo_uft8, "406获取跳转地址错误,地址解析失败");
             break;
        }

        if (!(uriTpassFull = evhttp_uri_parse(pValue))){
            sprintf((char *)errinfo_uft8, "406获取跳转地址错误,地址转换失败"); 
            break;
        }

        if (!FillUploadAddrModelDynamic(TAADDR_ETAX_TPASS, hDev->szRegCode, pValue, "/",&hEtax->tpass.modelTaAddr)) {
            sprintf((char *)errinfo_uft8, "406获取跳转地址错误,地址解析失败");
            break;
        } 

        if (nChildRet = EtaxHTTPInit(&hEtax->tpass, hDev)) {
            sprintf((char *)errinfo_uft8, "306Steps 2:HTTP 初始化失败, nRet:%d", nChildRet);
            break;
        }

        if (nChildRet = EtaxHTTPOpen(&hEtax->tpass)) {
            sprintf((char *)errinfo_uft8, "306Steps 2:HTTP 连接失败, nRet:%d", nChildRet);
            break;
        }
        if (!(hEtax->tpass.ctxApp = calloc(1, sizeof(struct ContextHttpAppBridge)))){
            sprintf((char *)errinfo_uft8, "306内存申请失败,空间不足");
            break;
        }
        hEtax->tpass.cbHttpClose = EtaxBridgeClose;
        
        //生成SM4密钥、获取公钥、发送SM4密钥
        if ((nChildRet = TPassLoginAuthSm4AndPublickey(&hEtax->tpass,hDev->szRegCode))) {
            sprintf((char *)errinfo_uft8, "406请求公钥或者发送SM4密钥失败:%s",(const char *)evbuffer_pullup(hEtax->tpass.bufHttpRep, -1));
            break;
        }

        //获取税盘证书与税局地址进行签名验证
        if ((nChildRet = TPassLoginAuthHelloAndAuth(&hEtax->tpass, uriTpassFull))) {
            sprintf((char *)errinfo_uft8, "406签名验签服务器响应请求错误:%s",(const char *)evbuffer_pullup(hEtax->tpass.bufHttpRep, -1));
            break;
        }

        //步骤4 根据签名验证请求后返回的用户信息以及中台传入的用户信息进行比较，筛选出需要登陆的身份
        if (!(root = cJSON_Parse((const char *)evbuffer_pullup(hEtax->tpass.bufHttpRep, -1)))) {
            sprintf((char *)errinfo_uft8, "406登陆凭证中返回用户列表为空");
            break;
        }
        if (!(objItem = cJSON_GetObjectItem(root, "userList"))) {
            sprintf((char *)errinfo_uft8, "406登陆凭证中返回用户列表解析失败");
            break;
        }
        nCount = cJSON_GetArraySize(objItem);
        for (i = 0; i < nCount; i++) {
            objItemChild = cJSON_GetArrayItem(objItem, i);

            if(strlen(cjmbpeizhi_parametetr->idcard)){
                if (!(objItemChild2 = cJSON_GetObjectItem(objItemChild, "id_card")))
                    continue;
                if (strcmp(objItemChild2->valuestring, cjmbpeizhi_parametetr->idcard))
                    continue;
            }

            if(strlen(cjmbpeizhi_parametetr->mobile)){
                if (!(objItemChild2 = cJSON_GetObjectItem(objItemChild, "mobile")))
                    continue;
                if (strcmp(objItemChild2->valuestring, cjmbpeizhi_parametetr->mobile))
                    continue;
            }

            if(cjmbpeizhi_parametetr->realationtype == 0){         //是否需要切换企业
                if (!(objItemChild2 = cJSON_GetObjectItem(objItemChild, "relatedType")))
                    continue;
                if (strcmp(objItemChild2->valuestring, cjmbpeizhi_parametetr->related_type))
                    continue;
            }

            if (!(objItemChild2 = cJSON_GetObjectItem(objItemChild, "user_id")))
                continue;
            strcpy(szUserId, objItemChild2->valuestring);
            printf("szUserId=%s,sz_password=%s\r\n",szUserId,cjmbpeizhi_parametetr->sz_password);
            break;
        }
        if(!strlen(szUserId)){
            sprintf((char *)errinfo_uft8, "306用户列表中未检测到该用户信息,请核实登陆信息是否有误");
            break;
        }

        if (nChildRet = TPassLoginPasswordLoginDo(&hEtax->tpass, szUserId, cjmbpeizhi_parametetr->sz_password, szBuf,sizeof(szBuf))){
            if (!(root = cJSON_Parse(szBuf)))
                break;
            if (!(objItem = cJSON_GetObjectItem(root, "msg")))
                break;
            sprintf((char *)errinfo_uft8, "406登陆失败:%s",objItem->valuestring);
            break;
        }

        //步骤5 根据筛选出来的身份进行登陆
        if(cjmbpeizhi_parametetr->realationtype == 1){         //是否需要切换企业后查询
            if (nChildRet = TPassSelectRelationchange(&hEtax->tpass,cjmbpeizhi_parametetr->uniqueIdentity,cjmbpeizhi_parametetr->realationStatus, szBuf,sizeof(szBuf))){
                if (!(root = cJSON_Parse(szBuf)))
                    break;
                if (!(objItem = cJSON_GetObjectItem(root, "msg")))
                    break;
                sprintf((char *)errinfo_uft8, "406企业切换失败,%s",objItem->valuestring);
                break;
            } 
        }


        //步骤7 获取一键登陆需要的code参数和RPA需要的token参数
        printf("TPassSelectRelationchange data is %s\n", szBuf);
        if (!(root = cJSON_Parse(szBuf))){
            sprintf((char *)errinfo_uft8, "406企业切换失败,返回数据解析失败");
            break;
        }
        if (!(objItem = cJSON_GetObjectItem(root, "code"))){
            sprintf((char *)errinfo_uft8, "406企业切换失败,返回数据中无登陆参数");
            break;
        }

        printf(" = %s\r\n",objItem->valuestring);

        if(0 > get_final_url(pValue,objItem->valuestring,final_url))
            break;

        DpptJiangSuConnect(&hEtax->dppt,final_url,hDev);

        if (!(nChildRet = InitSecurityConfig(&hEtax->dppt))) {
        }
        if(nChildRet){
            sprintf((char *)errinfo_uft8, "406初始化dppt服务器失败,错误状态为:%d", nChildRet);    
            break;
        }

        if(atoi(cjmbpeizhi_parametetr->mode) == 0){
            if ((nChildRet = EtaxQueryInvoice(hEtax, "/jcxxwh/fpfjyscjmbxx/v1/create",cjmbpeizhi_parametetr->fpcxsj)) < 0){
                sprintf((char *)errinfo_uft8, "406附加信息创建失败:%s",evbuffer_pullup(hEtax->dppt.bufHttpRep, -1));
                break;                     
            }  
        }

        if(atoi(cjmbpeizhi_parametetr->mode) == 1){
            if ((nChildRet = EtaxQueryInvoice(hEtax, "/jcxxwh/fpfjyscjmbxx/v1/modify",cjmbpeizhi_parametetr->fpcxsj)) < 0){
                sprintf((char *)errinfo_uft8, "406附加信息修改失败:%s",evbuffer_pullup(hEtax->dppt.bufHttpRep, -1));
                break;                     
            }                     
        }

        if(atoi(cjmbpeizhi_parametetr->mode) == 2){
            if ((nChildRet = EtaxQueryInvoice(hEtax, "/jcxxwh/fpfjyscjmbxx/v1/delete",cjmbpeizhi_parametetr->fpcxsj)) < 0){
                sprintf((char *)errinfo_uft8, "406附加信息删除失败:%s",evbuffer_pullup(hEtax->dppt.bufHttpRep, -1));
                break;                     
            }                      
        }

        if(atoi(cjmbpeizhi_parametetr->mode) == 3){
            if ((nChildRet = EtaxQueryInvoice(hEtax, "/jcxxwh/fpfjyscjmbxx/v1/queryPageList",cjmbpeizhi_parametetr->fpcxsj)) < 0){
                sprintf((char *)errinfo_uft8, "406场景模版列表列表查询失败:%s",evbuffer_pullup(hEtax->dppt.bufHttpRep, -1));
                break;                     
            }                      
        }
        else 
        {
            if ((nChildRet = EtaxQueryInvoice(hEtax, "/jcxxwh/fpfjyscjmbxx/v1/queryPageList","{\"fjysxmmc\":\"\",\"order\":\"desc\",\"sortBy\":\"Lrrq\",\"pageNumber\":1,\"pageSize\":100}")) < 0){
                sprintf((char *)errinfo_uft8, "406场景模版列表列表查询失败:%s",evbuffer_pullup(hEtax->dppt.bufHttpRep, -1));
                break;                      
            }  
        }

        nChildRet = evbuffer_get_length(hEtax->dppt.bufHttpRep);
        printf("nChildRet = %d\r\n,%s\r\n",nChildRet,evbuffer_pullup(hEtax->dppt.bufHttpRep, -1));

        int ChildRetLen = nChildRet + 1024;

        *(cjmbpeizhi_parametetr->fpcxdata) = NULL;
        *(cjmbpeizhi_parametetr->fpcxdata) = (char *)malloc(ChildRetLen);
        if(*(cjmbpeizhi_parametetr->fpcxdata) == NULL){    
            break;
        }
		asc_compress_base64(evbuffer_pullup(hEtax->dppt.bufHttpRep, -1), nChildRet, *(cjmbpeizhi_parametetr->fpcxdata), &ChildRetLen);
        //结束
        nRet = RET_SUCCESS;
    }while (false);

    //释放内存
    if(nEtaxBufData)
        free(nEtaxBufData);
    nEtaxBufData = NULL;

    //释放内存
    if (root)
        cJSON_Delete(root);
    root = NULL;

    //释放内存
    if (jsonTpassConfig)
        cJSON_Delete(jsonTpassConfig);
    jsonTpassConfig  = NULL;

    if (uriTpassFull)
        evhttp_uri_free(uriTpassFull);

    if(pValue)
        free(pValue);

    if(hEtax->etax.user_args)
    {
        free(hEtax->etax.user_args);
        hEtax->etax.user_args = NULL;
    }

    return nRet;    
}

int etax_jiangsu_rpa_invoice_issuance_status(HETAX hEtax, HDEV hDev,void *args,char *errinfo_uft8)
{
    struct evhttp_uri *uriTpassFull = NULL;

    RPA_INVOICE_STATUS_ISSUANCE *rpa_invoice_issuance_status_parametetr = (RPA_INVOICE_STATUS_ISSUANCE *)args;

    int nRet = ERR_GENERIC;
    int nChildRet = ERR_GENERIC;
    int nCount,i;

    char *pValue = NULL;
    char *nEtaxBufData = NULL;
    char szUserId[BS_LITTLE] = {0};
    char szBuf[BS_BLOCK * 6] = {0};
    char final_url[512] = {0x00};
    char nsrsbhbuf[BS_NORMAL]={0};
    char *szTPassConfigJson = NULL;

    char rzid[BS_NORMAL]={0};

    cJSON *jsonTpassConfig = NULL;
    cJSON *root = NULL, *objItem = NULL, *objItemChild = NULL;
    cJSON *objItemChild2 = NULL,*objItemToken  = NULL;

    cJSON *root6 = NULL, *objItem6 = NULL, *objItemChild6 = NULL;

    if(NULL == rpa_invoice_issuance_status_parametetr)
    {
        return nRet;
    }
    printf("    enter  etax_jiangsu_rpa_invoice_issuance_status             etax_jiangsu_rpa_invoice_issuance_status\r\n");
    hEtax->etax.user_args = (void *)(&header_array_jiangsu);
    memset(hEtax->etax.modelTaAddr.szTaxAuthorityURL,0x00,sizeof(hEtax->etax.modelTaAddr.szTaxAuthorityURL));
    strcpy(hEtax->etax.modelTaAddr.szTaxAuthorityURL,"https://dppt.jiangsu.chinatax.gov.cn:8443");
    memset(hEtax->etax.modelTaAddr.szInitRequestPage,0x00,sizeof(hEtax->etax.modelTaAddr.szInitRequestPage));
    strcpy(hEtax->etax.modelTaAddr.szInitRequestPage,"/szzhzz/spHandler?cdlj=digital-tax-account");

    do{
        //步骤1 使用etax地址获取tpass地址
        if (nChildRet = EtaxHTTPInit(&hEtax->etax, hDev)) {
            sprintf((char *)errinfo_uft8, "306Steps 1:HTTP 初始化失败, nRet:%d", nChildRet);
            break;
        }
        if (nChildRet = EtaxHTTPOpen(&hEtax->etax)) {
            sprintf((char *)errinfo_uft8, "306Steps 1:HTTP 连接失败, nRet:%d", nChildRet);
            break;
        }
        if (!(hEtax->etax.ctxApp = calloc(1, sizeof(struct ContextHttpAppBridge)))){
            sprintf((char *)errinfo_uft8, "306内存申请失败,空间不足");
            break;
        }
        hEtax->etax.cbHttpClose = EtaxBridgeClose;
        //获取tpass地址
        //步骤3 从token服务器获取登陆凭证去登陆Etax,登陆完成后返回的用户列表在hiTPass->bufHttpRep中
        if (hEtax->etax.ctxCon.nHttpRepCode != 200 && hEtax->etax.ctxCon.nHttpRepCode != 302){
            sprintf((char *)errinfo_uft8, "406请求跳转地址失败,HTTP Code:%d",hEtax->etax.ctxCon.nHttpRepCode);  
            break;
        }
        if(hEtax->etax.ctxCon.nHttpRepCode == 200)
        {
            szTPassConfigJson = (char *)evbuffer_pullup(hEtax->etax.bufHttpRep, -1);
        }
        else if(hEtax->etax.ctxCon.nHttpRepCode == 302)
        {
            szTPassConfigJson = (char *)hEtax->etax.user_args;
        }
        //获取跳转tpass地址
        if(RET_SUCCESS != get_tpass_addr(szTPassConfigJson,&pValue,"1"))
        {
             sprintf((char *)errinfo_uft8, "406获取跳转地址错误,地址解析失败");
             break;
        }

        if (!(uriTpassFull = evhttp_uri_parse(pValue))){
            sprintf((char *)errinfo_uft8, "406获取跳转地址错误,地址转换失败"); 
            break;
        }

        if (!FillUploadAddrModelDynamic(TAADDR_ETAX_TPASS, hDev->szRegCode, pValue, "/",&hEtax->tpass.modelTaAddr)) {
            sprintf((char *)errinfo_uft8, "406获取跳转地址错误,地址解析失败");
            break;
        } 

        if (nChildRet = EtaxHTTPInit(&hEtax->tpass, hDev)) {
            sprintf((char *)errinfo_uft8, "306Steps 2:HTTP 初始化失败, nRet:%d", nChildRet);
            break;
        }

        if (nChildRet = EtaxHTTPOpen(&hEtax->tpass)) {
            sprintf((char *)errinfo_uft8, "306Steps 2:HTTP 连接失败, nRet:%d", nChildRet);
            break;
        }
        if (!(hEtax->tpass.ctxApp = calloc(1, sizeof(struct ContextHttpAppBridge)))){
            sprintf((char *)errinfo_uft8, "306内存申请失败,空间不足");
            break;
        }
        hEtax->tpass.cbHttpClose = EtaxBridgeClose;
        
        //生成SM4密钥、获取公钥、发送SM4密钥
        if ((nChildRet = TPassLoginAuthSm4AndPublickey(&hEtax->tpass,hDev->szRegCode))) {
            sprintf((char *)errinfo_uft8, "406请求公钥或者发送SM4密钥失败:%s",(const char *)evbuffer_pullup(hEtax->tpass.bufHttpRep, -1));
            break;
        }

        //获取税盘证书与税局地址进行签名验证
        if ((nChildRet = TPassLoginAuthHelloAndAuth(&hEtax->tpass, uriTpassFull))) {
            sprintf((char *)errinfo_uft8, "406签名验签服务器响应请求错误:%s",(const char *)evbuffer_pullup(hEtax->tpass.bufHttpRep, -1));
            break;
        }

        //步骤4 根据签名验证请求后返回的用户信息以及中台传入的用户信息进行比较，筛选出需要登陆的身份
        if (!(root = cJSON_Parse((const char *)evbuffer_pullup(hEtax->tpass.bufHttpRep, -1)))) {
            sprintf((char *)errinfo_uft8, "406登陆凭证中返回用户列表为空");
            break;
        }
        if (!(objItem = cJSON_GetObjectItem(root, "userList"))) {
            sprintf((char *)errinfo_uft8, "406登陆凭证中返回用户列表解析失败");
            break;
        }
        nCount = cJSON_GetArraySize(objItem);
        for (i = 0; i < nCount; i++) {
            objItemChild = cJSON_GetArrayItem(objItem, i);

            if(strlen(rpa_invoice_issuance_status_parametetr->idcard)){
                if (!(objItemChild2 = cJSON_GetObjectItem(objItemChild, "id_card")))
                    continue;
                if (strcmp(objItemChild2->valuestring, rpa_invoice_issuance_status_parametetr->idcard))
                    continue;
            }

            if(strlen(rpa_invoice_issuance_status_parametetr->mobile)){
                if (!(objItemChild2 = cJSON_GetObjectItem(objItemChild, "mobile")))
                    continue;
                if (strcmp(objItemChild2->valuestring, rpa_invoice_issuance_status_parametetr->mobile))
                    continue;
            }

            if(rpa_invoice_issuance_status_parametetr->realationtype == 0){         //是否需要切换企业
                if (!(objItemChild2 = cJSON_GetObjectItem(objItemChild, "relatedType")))
                    continue;
                if (strcmp(objItemChild2->valuestring, rpa_invoice_issuance_status_parametetr->related_type))
                    continue;
            }

            if (!(objItemChild2 = cJSON_GetObjectItem(objItemChild, "user_id")))
                continue;
            strcpy(szUserId, objItemChild2->valuestring);
            printf("szUserId=%s,sz_password=%s\r\n",szUserId,rpa_invoice_issuance_status_parametetr->sz_password);
            break;
        }
        if(!strlen(szUserId)){
            sprintf((char *)errinfo_uft8, "306用户列表中未检测到该用户信息,请核实登陆信息是否有误");
            break;
        }

        if (nChildRet = TPassLoginPasswordLoginDo(&hEtax->tpass, szUserId, rpa_invoice_issuance_status_parametetr->sz_password, szBuf,sizeof(szBuf))){
            if (!(root = cJSON_Parse(szBuf)))
                break;
            if (!(objItem = cJSON_GetObjectItem(root, "msg")))
                break;
            sprintf((char *)errinfo_uft8, "406登陆失败:%s",objItem->valuestring);
            break;
        }

        //步骤5 根据筛选出来的身份进行登陆
        if(rpa_invoice_issuance_status_parametetr->realationtype == 1){         //是否需要切换企业后查询
            if (nChildRet = TPassSelectRelationchange(&hEtax->tpass,rpa_invoice_issuance_status_parametetr->uniqueIdentity,rpa_invoice_issuance_status_parametetr->realationStatus, szBuf,sizeof(szBuf))){
                if (!(root = cJSON_Parse(szBuf)))
                    break;
                if (!(objItem = cJSON_GetObjectItem(root, "msg")))
                    break;
                sprintf((char *)errinfo_uft8, "406企业切换失败,%s",objItem->valuestring);
                break;
            } 
        }

        //步骤7 获取一键登陆需要的code参数和RPA需要的token参数
        printf("TPassSelectRelationchange data is %s\n", szBuf);
        if (!(root = cJSON_Parse(szBuf))){
            sprintf((char *)errinfo_uft8, "406企业切换失败,返回数据解析失败");
            break;
        }
        if (!(objItem = cJSON_GetObjectItem(root, "code"))){
            sprintf((char *)errinfo_uft8, "406企业切换失败,返回数据中无登陆参数");
            break;
        }

        printf(" = %s\r\n",objItem->valuestring);

        if(0 > get_final_url(pValue,objItem->valuestring,final_url))
            break;

        printf("final_url=%s\r\n",final_url);

        DpptJiangSuConnect(&hEtax->dppt,final_url,hDev);

        if (!(nChildRet = InitSecurityConfig(&hEtax->dppt))) {

        }
        if(nChildRet){
            sprintf((char *)errinfo_uft8, "406初始化dppt服务器失败,错误状态为:%d", nChildRet);    
            break;
        }

        //printf("rpa_invoice_issuance_status_parametetr->rzid  = %s\r\n",rpa_invoice_issuance_status_parametetr->rzid);
        //步骤7 RPA扫码【状态查询】
        //if( 0 == rpa_invoice_issuance_status_parametetr->type) //电子税局 Send => {"rzid":"670932e6eb9742e2b24ab38c62e40f44","kjlp":""} POST /kpfw/slrz/v1/qrslrz 
        if( 0 == strcmp(rpa_invoice_issuance_status_parametetr->app_code,"0"))
        {
            sprintf(rzid,"{\"rzid\":\"%s\",\"kjlp\":\"\"}",rpa_invoice_issuance_status_parametetr->rzid);
            printf("url = %s\r\n",rpa_invoice_issuance_status_parametetr->fpkjurl);  // "/kpfw/slrz/v1/qrslrz"
            printf("rzid = %s\r\n",rzid);
            if ((nChildRet = EtaxQueryInvoice(hEtax, rpa_invoice_issuance_status_parametetr->fpkjurl,rzid)) < 0){
                sprintf((char *)errinfo_uft8, "406查询接口失败:%s\r\n",(const char *)evbuffer_pullup(hEtax->dppt.bufHttpRep, -1));
            break;                      
            } 
        }
        else if( 1 == strcmp(rpa_invoice_issuance_status_parametetr->app_code,"0"))
        //个人所得税  Send => {"rzid":"43f5a3b8355e453eb9f914c10979e3ef_ZH.AUTH.QRCODE.SRRZ","kjlp":""} POST /kpfw/slrz/v1/qrjgForFaceCheck
        {
            //sprintf(rzid,"{\"rzid\":\"%s_ZH.AUTH.QRCODE.SRRZ\",\"kjlp\":\"\"}",rpa_invoice_issuance_status_parametetr->rzid);
            sprintf(rzid,"{\"rzid\":\"%s\",\"kjlp\":\"\"}",rpa_invoice_issuance_status_parametetr->rzid);
            printf("url = %s\r\n",rpa_invoice_issuance_status_parametetr->fpkjurl); //   "/kpfw/slrz/v1/qrjgForFaceCheck"
            printf("rzid = %s\r\n",rzid);
            if ((nChildRet = EtaxQueryInvoice(hEtax, rpa_invoice_issuance_status_parametetr->fpkjurl ,rzid)) < 0){
                sprintf((char *)errinfo_uft8, "406查询接口失败:%s\r\n",(const char *)evbuffer_pullup(hEtax->dppt.bufHttpRep, -1));
                break;            
            }
        }
        nChildRet = evbuffer_get_length(hEtax->dppt.bufHttpRep);

        //printf("############  nRepSize = %d\r\n,%s\r\n",nChildRet,evbuffer_pullup(hEtax->dppt.bufHttpRep, -1));


        //nEtaxBufData = (char *)malloc(nChildRet);
        //if(nEtaxBufData == NULL)
        //    break;

        *(rpa_invoice_issuance_status_parametetr->fpkjdata) = NULL;
        *(rpa_invoice_issuance_status_parametetr->fpkjdata) = (char *)malloc(nChildRet);
        if(*(rpa_invoice_issuance_status_parametetr->fpkjdata) == NULL){    
            break;
        }
        evbuffer_copyout_from(hEtax->dppt.bufHttpRep,NULL, *(rpa_invoice_issuance_status_parametetr->fpkjdata),nChildRet);
        //EncordingConvert( "utf-8","gbk",nEtaxBufData, nChildRet, *(rpa_invoice_issuance_status_parametetr->fpcxdata), nChildRet);

        //结束
        nRet = RET_SUCCESS;
    }while (false);

    //释放内存
    if(nEtaxBufData)
        free(nEtaxBufData);
    nEtaxBufData = NULL;

    //释放内存
    if (root)
        cJSON_Delete(root);
    root = NULL;

    //释放内存
    if (jsonTpassConfig)
        cJSON_Delete(jsonTpassConfig);
    jsonTpassConfig  = NULL;

    if (uriTpassFull)
        evhttp_uri_free(uriTpassFull);

    if(pValue)
        free(pValue);

    if(hEtax->etax.user_args)
    {
        free(hEtax->etax.user_args);
        hEtax->etax.user_args = NULL;
    }

    return nRet; 
}

//获取 cookies
int etax_jiangsu_get_cookie(HETAX hEtax, HDEV hDev,void *args,char *errinfo_uft8)
{
    struct evhttp_uri *uriTpassFull = NULL;

    GET_COOKIE *get_cookie_parametetr = (GET_COOKIE *)args;

    int nRet = ERR_GENERIC;
    int nChildRet = ERR_GENERIC;
    int nCount,i;

    char *pValue = NULL;
    char *nEtaxBufData = NULL;
    char szUserId[BS_LITTLE] = {0};
    char szBuf[BS_BLOCK * 6] = {0};
    char final_url[512] = {0x00};
    char *szTPassConfigJson = NULL;

    cJSON *jsonTpassConfig = NULL;
    cJSON *root = NULL, *objItem = NULL, *objItemChild = NULL;
    cJSON *objItemChild2 = NULL,*objItemToken  = NULL;

    if(NULL == get_cookie_parametetr)
    {
        return nRet;
    }

    hEtax->etax.user_args = (void *)(&header_array_jiangsu);
    memset(hEtax->etax.modelTaAddr.szTaxAuthorityURL,0x00,sizeof(hEtax->etax.modelTaAddr.szTaxAuthorityURL));
    strcpy(hEtax->etax.modelTaAddr.szTaxAuthorityURL,"https://dppt.jiangsu.chinatax.gov.cn:8443");
    memset(hEtax->etax.modelTaAddr.szInitRequestPage,0x00,sizeof(hEtax->etax.modelTaAddr.szInitRequestPage));
    strcpy(hEtax->etax.modelTaAddr.szInitRequestPage,"/szzhzz/spHandler?cdlj=digital-tax-account");    

    do{
        //步骤1 使用etax地址获取tpass地址
        if (nChildRet = EtaxHTTPInit(&hEtax->etax, hDev)) {
            sprintf((char *)errinfo_uft8, "306Steps 1:HTTP 初始化失败, nRet:%d", nChildRet);
            break;
        }
        if (nChildRet = EtaxHTTPOpen(&hEtax->etax)) {
            sprintf((char *)errinfo_uft8, "306Steps 1:HTTP 连接失败, nRet:%d", nChildRet);
            break;
        }
        if (!(hEtax->etax.ctxApp = calloc(1, sizeof(struct ContextHttpAppBridge)))){
            sprintf((char *)errinfo_uft8, "306内存申请失败,空间不足");
            break;
        }
        hEtax->etax.cbHttpClose = EtaxBridgeClose;
        //获取tpass地址
        //步骤3 从token服务器获取登陆凭证去登陆Etax,登陆完成后返回的用户列表在hiTPass->bufHttpRep中
        if (hEtax->etax.ctxCon.nHttpRepCode != 200 && hEtax->etax.ctxCon.nHttpRepCode != 302){
            sprintf((char *)errinfo_uft8, "406请求跳转地址失败,HTTP Code:%d",hEtax->etax.ctxCon.nHttpRepCode);  
            break;
        }
        if(hEtax->etax.ctxCon.nHttpRepCode == 200)
        {
            szTPassConfigJson = (char *)evbuffer_pullup(hEtax->etax.bufHttpRep, -1);
        }
        else if(hEtax->etax.ctxCon.nHttpRepCode == 302)
        {
            szTPassConfigJson = (char *)hEtax->etax.user_args;
        }
        printf("szTPassConfigJson=%s\r\n",szTPassConfigJson);
        //获取跳转tpass地址
        if(RET_SUCCESS != get_tpass_addr(szTPassConfigJson,&pValue,"1"))
        {
             sprintf((char *)errinfo_uft8, "406获取跳转地址错误,地址解析失败");
             break;
        }

        if (!(uriTpassFull = evhttp_uri_parse(pValue))){
            sprintf((char *)errinfo_uft8, "406获取跳转地址错误,地址转换失败"); 
            break;
        }

        if (!FillUploadAddrModelDynamic(TAADDR_ETAX_TPASS, hDev->szRegCode, pValue, "/",&hEtax->tpass.modelTaAddr)) {
            sprintf((char *)errinfo_uft8, "406获取跳转地址错误,地址解析失败");
            break;
        } 

        if (nChildRet = EtaxHTTPInit(&hEtax->tpass, hDev)) {
            sprintf((char *)errinfo_uft8, "306Steps 2:HTTP 初始化失败, nRet:%d", nChildRet);
            break;
        }

        if (nChildRet = EtaxHTTPOpen(&hEtax->tpass)) {
            sprintf((char *)errinfo_uft8, "306Steps 2:HTTP 连接失败, nRet:%d", nChildRet);
            break;
        }
        if (!(hEtax->tpass.ctxApp = calloc(1, sizeof(struct ContextHttpAppBridge)))){
            sprintf((char *)errinfo_uft8, "306内存申请失败,空间不足");
            break;
        }
        hEtax->tpass.cbHttpClose = EtaxBridgeClose;
        
        //生成SM4密钥、获取公钥、发送SM4密钥
        if ((nChildRet = TPassLoginAuthSm4AndPublickey(&hEtax->tpass,hDev->szRegCode))) {
            sprintf((char *)errinfo_uft8, "406请求公钥或者发送SM4密钥失败:%s",(const char *)evbuffer_pullup(hEtax->tpass.bufHttpRep, -1));
            break;
        }

        printf("      get_cookie       login_method           login_method            =   [%s]                       \r\n",get_cookie_parametetr->login_method);
        if(memcmp(get_cookie_parametetr->login_method,"01",2) == 0)  //软证书
        {
            TPassLoginAuthHelloAndAuth_soft_cert(&hEtax->tpass, uriTpassFull,get_cookie_parametetr->svr_container,get_cookie_parametetr->soft_cert_ip,get_cookie_parametetr->soft_cert_port);
        }
        else if (memcmp(get_cookie_parametetr->login_method,"21",2) == 0)
        {
            if ((nChildRet = TPassLoginAuthHelloAndAuth_soft_cert_21(&hEtax->tpass, uriTpassFull,get_cookie_parametetr->svr_container,get_cookie_parametetr->soft_cert_ip,get_cookie_parametetr->soft_cert_port)))
            {
                sprintf((char *)errinfo_uft8, "102税盘已离线或是登陆参数有误");
                printf("lbc-debug nChildRet=%d        nRet = %d     \n",nChildRet,nRet);
                break;
            }
        }  
        else if(memcmp(get_cookie_parametetr->login_method,"02",2) == 0)  //数字证书
        {
            //获取税盘证书与税局地址进行签名验证
            if ((nChildRet = TPassLoginAuthHelloAndAuth(&hEtax->tpass, uriTpassFull))) {
                sprintf((char *)errinfo_uft8, "406签名验签服务器响应请求错误,可能为税局服务器原因，请插盘尝试");
                printf("lbc-debug nChildRet=%d\n",nChildRet);
                break;
            }
        }

        //步骤4 根据签名验证请求后返回的用户信息以及中台传入的用户信息进行比较，筛选出需要登陆的身份
        if (!(root = cJSON_Parse((const char *)evbuffer_pullup(hEtax->tpass.bufHttpRep, -1)))) {
            sprintf((char *)errinfo_uft8, "406登陆凭证中返回用户列表为空");
            break;
        }
        if (!(objItem = cJSON_GetObjectItem(root, "userList"))) {
            sprintf((char *)errinfo_uft8, "406登陆凭证中返回用户列表解析失败");
            break;
        }
        nCount = cJSON_GetArraySize(objItem);
        for (i = 0; i < nCount; i++) {
            objItemChild = cJSON_GetArrayItem(objItem, i);

            if(strlen(get_cookie_parametetr->idcard)){
                if (!(objItemChild2 = cJSON_GetObjectItem(objItemChild, "id_card")))
                    continue;
                if (strcmp(objItemChild2->valuestring, get_cookie_parametetr->idcard))
                    continue;
            }

            if(strlen(get_cookie_parametetr->mobile)){
                if (!(objItemChild2 = cJSON_GetObjectItem(objItemChild, "mobile")))
                    continue;
                if (strcmp(objItemChild2->valuestring, get_cookie_parametetr->mobile))
                    continue;
            }

            if(get_cookie_parametetr->realationtype == 0){         //是否需要切换企业
                if (!(objItemChild2 = cJSON_GetObjectItem(objItemChild, "relatedType")))
                    continue;
                if (strcmp(objItemChild2->valuestring, get_cookie_parametetr->related_type))
                    continue;
            }

            if (!(objItemChild2 = cJSON_GetObjectItem(objItemChild, "user_id")))
                continue;
            strcpy(szUserId, objItemChild2->valuestring);
            printf("szUserId=%s,sz_password=%s\r\n",szUserId,get_cookie_parametetr->sz_password);
            break;
        }
        if(!strlen(szUserId)){
            sprintf((char *)errinfo_uft8, "306用户列表中未检测到该用户信息,请核实登陆信息是否有误");
            break;
        }
        if (nChildRet = TPassLoginPasswordLoginDo(&hEtax->tpass, szUserId, get_cookie_parametetr->sz_password, szBuf,sizeof(szBuf))){
            if (!(root = cJSON_Parse(szBuf)))
                break;
            if (!(objItem = cJSON_GetObjectItem(root, "msg")))
                break;
            sprintf((char *)errinfo_uft8, "406登陆失败:%s",objItem->valuestring);
            break;
        }

        //步骤5 根据筛选出来的身份进行登陆
        if(get_cookie_parametetr->realationtype == 1){         //是否需要切换企业后查询
            if (nChildRet = TPassSelectRelationchange(&hEtax->tpass,get_cookie_parametetr->uniqueIdentity,get_cookie_parametetr->realationStatus, szBuf,sizeof(szBuf))){
                if (!(root = cJSON_Parse(szBuf)))
                    break;
                if (!(objItem = cJSON_GetObjectItem(root, "msg")))
                    break;
                sprintf((char *)errinfo_uft8, "406企业切换失败,%s",objItem->valuestring);
                break;
            } 
        }

        //步骤7 获取一键登陆需要的code参数和RPA需要的token参数
        printf("TPassSelectRelationchange data is %s\n", szBuf);
        if (!(root = cJSON_Parse(szBuf))){
            sprintf((char *)errinfo_uft8, "406企业切换失败,返回数据解析失败");
            break;
        }
        if (!(objItem = cJSON_GetObjectItem(root, "code"))){
            sprintf((char *)errinfo_uft8, "406企业切换失败,返回数据中无登陆参数");
            break;
        }

        printf(" = %s\r\n",objItem->valuestring);

        if(0 > get_final_url(pValue,objItem->valuestring,final_url))
            break;

        printf("final_url=%s\r\n",final_url);

        DpptJiangSuConnect(&hEtax->dppt,final_url,hDev);

        if ((nChildRet = InitSecurityConfig(&hEtax->dppt))) {
            sprintf((char *)errinfo_uft8, "406初始化dppt服务器失败,错误状态为:%d", nChildRet);    
            break;
        }

        *(get_cookie_parametetr->dppt_cookie) = NULL;
        *(get_cookie_parametetr->dppt_cookie) = (char *)malloc(BS_BLOCK * 2);
        if(*(get_cookie_parametetr->dppt_cookie) == NULL) 
            break;   

        memset(*(get_cookie_parametetr->dppt_cookie),0,BS_BLOCK * 2);

        nChildRet = evbuffer_get_length(hEtax->dppt.ctxCon.bufCookies);
        evbuffer_copyout_from(hEtax->dppt.ctxCon.bufCookies,NULL,*(get_cookie_parametetr->dppt_cookie),nChildRet);

        // printf("dppt_cookie:%s\r\n", *(get_cookie_parametetr->dppt_cookie));

        //传出token
        if (!(objItemToken = cJSON_GetObjectItem(root, "access_token"))){
            sprintf((char *)errinfo_uft8, "406登陆失败,返回数据中无token参数");
            break;
        }
        strcpy(get_cookie_parametetr->temp_data, objItemToken->valuestring); 
        printf("   【Token】    TPassLoginPasswordLoginDo Token = %s\r\n",get_cookie_parametetr->temp_data);

        //结束
        nRet = RET_SUCCESS;
    }while (false);

    //释放内存
    if (root)
        cJSON_Delete(root);
    root = NULL;

    //释放内存
    if (jsonTpassConfig)
        cJSON_Delete(jsonTpassConfig);
    jsonTpassConfig  = NULL;

    return nRet; 
}

//发票列表 bycookies
int etax_jiangsu_invoice_query_bycookies(HETAX hEtax, HDEV hDev,void *args,char *errinfo_uft8)
{
    struct evhttp_uri *uriTpassFull = NULL;
    INVOICE_QUERY_BY_COOKIES *parametetr = (INVOICE_QUERY_BY_COOKIES *)args;
    struct evbuffer *evDpptCookies = NULL;
    int nRet = ERR_GENERIC,nChildRet = ERR_GENERIC;
    char *nEtaxBufData = NULL;
    if(NULL == parametetr)
    {
        return nRet;
    }
    do
    {
        //参数检测
        if (!parametetr->dppt_cookie || strlen(parametetr->dppt_cookie) < 16)
        {
            break;
        }
        if (strchr(parametetr->dppt_cookie, '\n'))
        {
            break;
        }
        if (!(evDpptCookies = evbuffer_new()))
        {
            break;
        }
        evbuffer_add_printf(evDpptCookies, "%s", parametetr->dppt_cookie);
        DpptJiangSuConnectBycookie(&hEtax->dppt,hDev);
        evbuffer_add_printf(hEtax->dppt.ctxCon.bufCookies, "; %s",evbuffer_pullup(evDpptCookies, -1));
        EtaxRemoveRepeatOldCookies(hEtax->dppt.ctxCon.bufCookies);

        if ((nChildRet = InitSecurityConfig(&hEtax->dppt))) {
            if(nChildRet == INT32_MIN){
                sprintf((char *)errinfo_uft8, "406该Cookies已失效,请重新请求该接口");   
                break;                
            }
            else 
            {
                sprintf((char *)errinfo_uft8, "406初始化dppt服务器失败,错误状态为:%d", nChildRet);    
                break;
            }
        }

        //步骤7 发票查询(列表)
        if ((nChildRet = EtaxQueryInvoice(hEtax, "/szzhzz/qlfpcx/v1/queryFpjcxx",parametetr->fpcxsj)) < 0)
        {
            sprintf((char *)errinfo_uft8, "406查询接口失败:%s\r\n",(const char *)evbuffer_pullup(hEtax->dppt.bufHttpRep, -1));
            break;                      
        } 
        nChildRet = evbuffer_get_length(hEtax->dppt.bufHttpRep);
        nEtaxBufData = (char *)malloc(nChildRet);
        if(nEtaxBufData == NULL)
        {
            break;
        }
        *(parametetr->fpcxdata) = NULL;
        *(parametetr->fpcxdata) = (char *)malloc(nChildRet);
        if(*(parametetr->fpcxdata) == NULL)
        {
            break;
        }
        evbuffer_copyout_from(hEtax->dppt.bufHttpRep,NULL, nEtaxBufData,nChildRet);
        EncordingConvert( "utf-8","gbk",nEtaxBufData, nChildRet, *(parametetr->fpcxdata), nChildRet);
        //结束
        nRet = RET_SUCCESS;
    }while (false);
    //释放内存
    if(NULL != nEtaxBufData)
        free(nEtaxBufData);
    nEtaxBufData = NULL;
    return nRet;
}

//发票详情 bycookies
int etax_jiangsu_invoice_query_detail_bycookies(HETAX hEtax, HDEV hDev,void *args,char *errinfo_uft8)
{
    struct evhttp_uri *uriTpassFull = NULL;
    INVOICE_QUERY_BY_COOKIES *parametetr = (INVOICE_QUERY_BY_COOKIES *)args;
    struct evbuffer *evDpptCookies = NULL;
    int nRet = ERR_GENERIC,nChildRet = ERR_GENERIC;
    char *nEtaxBufData = NULL;
    if(NULL == parametetr)
    {
        return nRet;
    }
    do
    {
        //参数检测
        if (!parametetr->dppt_cookie || strlen(parametetr->dppt_cookie) < 16)
        {
            break;
        }
        if (strchr(parametetr->dppt_cookie, '\n'))
        {
            break;
        }
        if (!(evDpptCookies = evbuffer_new()))
        {
            break;
        }
        evbuffer_add_printf(evDpptCookies, "%s", parametetr->dppt_cookie);
        DpptJiangSuConnectBycookie(&hEtax->dppt,hDev);
        evbuffer_add_printf(hEtax->dppt.ctxCon.bufCookies, "; %s",evbuffer_pullup(evDpptCookies, -1));
        EtaxRemoveRepeatOldCookies(hEtax->dppt.ctxCon.bufCookies);

        if ((nChildRet = InitSecurityConfig(&hEtax->dppt))) {
            if(nChildRet == INT32_MIN){
                sprintf((char *)errinfo_uft8, "406该Cookies已失效,请重新请求该接口");   
                break;                
            }
            else 
            {
                sprintf((char *)errinfo_uft8, "406初始化dppt服务器失败,错误状态为:%d", nChildRet);    
                break;
            }
        }

        if ((nChildRet = EtaxQueryInvoice(hEtax, "/szzhzz/fppmcx/v1/queryFppmxx",parametetr->fpcxsj)) < 0)
        {
            sprintf((char *)errinfo_uft8, "406查询接口失败:%s\r\n",(const char *)evbuffer_pullup(hEtax->dppt.bufHttpRep, -1));
            break;                      
        } 
        nChildRet = evbuffer_get_length(hEtax->dppt.bufHttpRep);
        nEtaxBufData = (char *)malloc(nChildRet);
        if(nEtaxBufData == NULL)
        {
            break;
        }
        *(parametetr->fpcxdata) = NULL;
        *(parametetr->fpcxdata) = (char *)malloc(nChildRet);
        if(*(parametetr->fpcxdata) == NULL)
        {
            break;
        }
        evbuffer_copyout_from(hEtax->dppt.bufHttpRep,NULL, nEtaxBufData,nChildRet);
        EncordingConvert( "utf-8","gbk",nEtaxBufData, nChildRet, *(parametetr->fpcxdata), nChildRet);
        //结束
        nRet = RET_SUCCESS;
    }while (false);
    //释放内存
    if(NULL != nEtaxBufData)
        free(nEtaxBufData);
    nEtaxBufData = NULL;
    return nRet;
}

//发票下载 bycookies
int etax_jiangsu_invoice_query_down_by_cookie(HETAX hEtax, HDEV hDev,void *args,char *errinfo_uft8)
{
    struct evhttp_uri *uriTpassFull = NULL;
    INVOICE_QUERY_DOWN_BY_COOKIE *parametetr = (INVOICE_QUERY_BY_COOKIES *)args;
    struct evbuffer *evDpptCookies = NULL;
    int nRet = ERR_GENERIC,nChildRet = ERR_GENERIC;

    if(NULL == parametetr)
    {
        return nRet;
    }
    do
    {
        //参数检测
        if (!parametetr->dppt_cookie || strlen(parametetr->dppt_cookie) < 16)
        {
            break;
        }
        if (strchr(parametetr->dppt_cookie, '\n'))
        {
            break;
        }
        if (!(evDpptCookies = evbuffer_new()))
        {
            break;
        }
        evbuffer_add_printf(evDpptCookies, "%s", parametetr->dppt_cookie);
        DpptJiangSuConnectBycookie(&hEtax->dppt,hDev);
        evbuffer_add_printf(hEtax->dppt.ctxCon.bufCookies, "; %s",evbuffer_pullup(evDpptCookies, -1));
        EtaxRemoveRepeatOldCookies(hEtax->dppt.ctxCon.bufCookies);

        if ((nChildRet = InitSecurityConfig(&hEtax->dppt))) {
            if(nChildRet == INT32_MIN){
                sprintf((char *)errinfo_uft8, "406该Cookies已失效,请重新请求该接口");   
                break;                
            }
            else 
            {
                sprintf((char *)errinfo_uft8, "406初始化dppt服务器失败,错误状态为:%d", nChildRet);    
                break;
            }
        }

       //步骤7 发票查询(下载)
        char fpcxsjtime[30];
        struct timeval start;
        gettimeofday(&start, NULL);
        uint64 timeStamp = 1000 * (uint64)start.tv_sec + start.tv_usec / 1000;
        sprintf(fpcxsjtime,"&timeStampId=%llu",timeStamp);
        strcat(parametetr->fpcxsj,fpcxsjtime);

        if ((nChildRet = EtaxQueryInvoicefpdown(hEtax,parametetr->fpcxsj)) < 0){
            sprintf((char *)errinfo_uft8, "406接口下载失败:%s\r\n",(const char *)evbuffer_pullup(hEtax->dppt.bufHttpRep, -1));
            break;                      
        }  
        nChildRet = evbuffer_get_length(hEtax->dppt.bufHttpRep);
        
        //printf("nChildRet = %d\r\n,%s\r\n",nChildRet,evbuffer_pullup(hEtax->dppt.bufHttpRep, -1));

        *(parametetr->fpcxdata) = NULL;
        *(parametetr->fpcxdata) = (char *)malloc(nChildRet + (1024 * 600));
        if(*(parametetr->fpcxdata) == NULL) 
            break;

        int json_zlib_len = nChildRet + (1024 * 600);

        Base64_Encode(evbuffer_pullup(hEtax->dppt.bufHttpRep, -1),nChildRet,*(parametetr->fpcxdata));
        

        *(parametetr->fpcxname) = NULL;
        *(parametetr->fpcxname) = (char *)malloc(128);
        if(*(parametetr->fpcxname) == NULL) 
            break;        


        //dzfp_$发票号码_$开票时间.pdf
        memset(*(parametetr->fpcxname),0,128);
        strcpy(*(parametetr->fpcxname),"dzfp_");

        char *ptr_start = NULL;
        if(NULL == (ptr_start = strstr(parametetr->fpcxsj,"Fphm=")))
        {
            _WriteLog(LL_WARN, "Get server Fphm failed");
            break;
        }
        memcpy(*(parametetr->fpcxname)+5,ptr_start+strlen("Fphm="),20);
        strcat(*(parametetr->fpcxname),"_");

        if(NULL == (ptr_start = strstr(parametetr->fpcxsj,"Kprq=")))
        {
            _WriteLog(LL_WARN, "Get server Kprq failed");
            break;
        }
        memcpy(*(parametetr->fpcxname)+26,ptr_start+strlen("Kprq="),14);
        strcat(*(parametetr->fpcxname),".");

        if(NULL == (ptr_start = strstr(parametetr->fpcxsj,"PDF")))
        {
            if(NULL == (ptr_start = strstr(parametetr->fpcxsj,"XML")))
            {
                if(NULL == (ptr_start = strstr(parametetr->fpcxsj,"OFD")))
                {
                    _WriteLog(LL_WARN, "Get server XZLX failed");
                    break;                    
                }
                else 
                {
                    strcat(*(parametetr->fpcxname),"ofd");
                }
            }
            else 
            {
                strcat(*(parametetr->fpcxname),"zip");
            }
        }
        else 
        {
            strcat(*(parametetr->fpcxname),"pdf");
        }
        printf("fpcxname3=%s\r\n",*(parametetr->fpcxname));
        //结束
        nRet = RET_SUCCESS;
    }while (false);

    return nRet; 
}

//rpa扫码 bycookies
int etax_jiangsu_rpa_invoice_issuance_by_cookie(HETAX hEtax, HDEV hDev,void *args,char *errinfo_uft8)
{
    RPA_INVOICE_ISSUANCE_BY_COOKIE *parametetr = (INVOICE_QUERY_BY_COOKIES *)args;
    struct evbuffer *evDpptCookies = NULL;
    int nRet = ERR_GENERIC,nChildRet = ERR_GENERIC;

    if(NULL == parametetr)
    {
        return nRet;
    }
    do
    {
        //参数检测
        if (!parametetr->dppt_cookie || strlen(parametetr->dppt_cookie) < 16)
        {
            break;
        }
        if (strchr(parametetr->dppt_cookie, '\n'))
        {
            break;
        }
        if (!(evDpptCookies = evbuffer_new()))
        {
            break;
        }
        evbuffer_add_printf(evDpptCookies, "%s", parametetr->dppt_cookie);
        DpptJiangSuConnectBycookie(&hEtax->dppt,hDev);
        evbuffer_add_printf(hEtax->dppt.ctxCon.bufCookies, "; %s",evbuffer_pullup(evDpptCookies, -1));
        EtaxRemoveRepeatOldCookies(hEtax->dppt.ctxCon.bufCookies);

        if ((nChildRet = InitSecurityConfig(&hEtax->dppt))) {
            if(nChildRet == INT32_MIN){
                sprintf((char *)errinfo_uft8, "406该Cookies已失效,请重新请求该接口");   
                break;                
            }
            else 
            {
                sprintf((char *)errinfo_uft8, "406初始化dppt服务器失败,错误状态为:%d", nChildRet);    
                break;
            }
        }
        char nsrsbhbuf[BS_NORMAL]={0};
       //步骤7 发票查询(列表)
        sprintf(nsrsbhbuf,"{\"nsrsbh\":\"%s\"}",parametetr->nsrsbh);
        if ((nChildRet = EtaxQueryInvoice(hEtax, parametetr->fpkjurl,nsrsbhbuf)) < 0){
            sprintf((char *)errinfo_uft8, "406查询接口失败:%s\r\n",(const char *)evbuffer_pullup(hEtax->dppt.bufHttpRep, -1));
            break;                      
        }
        nChildRet = evbuffer_get_length(hEtax->dppt.bufHttpRep);

        *(parametetr->fpkjdata) = NULL;
        *(parametetr->fpkjdata) = (char *)malloc(nChildRet);
        if(*(parametetr->fpkjdata) == NULL){    
            break;
        }
        evbuffer_copyout_from(hEtax->dppt.bufHttpRep,NULL, *(parametetr->fpkjdata),nChildRet);
        //结束
        nRet = RET_SUCCESS;
    }while (false);

    return nRet; 
}

int etax_jiangsu_rpa_invoice_issuance_status_by_cookie(HETAX hEtax, HDEV hDev,void *args,char *errinfo_uft8)
{
    RPA_INVOICE_ISSUANCE_STATUS_BY_COOKIE *parametetr = (INVOICE_QUERY_BY_COOKIES *)args;
    struct evbuffer *evDpptCookies = NULL;
    int nRet = ERR_GENERIC,nChildRet = ERR_GENERIC;
    char rzid[BS_BLOCK]={0};
    hEtax->etax.user_args = NULL;
    if(NULL == parametetr)
    {
        return nRet;
    }
    do
    {
        //参数检测
        if (!parametetr->dppt_cookie || strlen(parametetr->dppt_cookie) < 16)
        {
            break;
        }
        if (strchr(parametetr->dppt_cookie, '\n'))
        {
            break;
        }
        if (!(evDpptCookies = evbuffer_new()))
        {
            break;
        }
        evbuffer_add_printf(evDpptCookies, "%s", parametetr->dppt_cookie);
        DpptJiangSuConnectBycookie(&hEtax->dppt,hDev);
        evbuffer_add_printf(hEtax->dppt.ctxCon.bufCookies, "; %s",evbuffer_pullup(evDpptCookies, -1));
        EtaxRemoveRepeatOldCookies(hEtax->dppt.ctxCon.bufCookies);

        if ((nChildRet = InitSecurityConfig(&hEtax->dppt))) {
            if(nChildRet == INT32_MIN){
                sprintf((char *)errinfo_uft8, "406该Cookies已失效,请重新请求该接口");   
                break;                
            }
            else 
            {
                sprintf((char *)errinfo_uft8, "406初始化dppt服务器失败,错误状态为:%d", nChildRet);    
                break;
            }
        }

        if( 0 == strcmp(parametetr->app_code,"0"))
        {
            sprintf(rzid,"{\"rzid\":\"%s\",\"kjlp\":\"\"}",parametetr->rzid);
            // printf("url = %s\r\n",parametetr->fpkjurl); 
            // printf("rzid = %s\r\n",rzid);
            if ((nChildRet = EtaxQueryInvoice(hEtax, parametetr->fpkjurl,rzid)) < 0){
                sprintf((char *)errinfo_uft8, "406查询接口失败:%s\r\n",(const char *)evbuffer_pullup(hEtax->dppt.bufHttpRep, -1));
                break;                      
            } 
        }
        else if( 1 == strcmp(parametetr->app_code,"0"))
        {
            sprintf(rzid,"{\"rzid\":\"%s\",\"kjlp\":\"\"}",parametetr->rzid);
            // printf("url = %s\r\n",parametetr->fpkjurl); 
            // printf("rzid = %s\r\n",rzid);
            if ((nChildRet = EtaxQueryInvoice(hEtax, parametetr->fpkjurl ,rzid)) < 0){
                sprintf((char *)errinfo_uft8, "406查询接口失败:%s\r\n",(const char *)evbuffer_pullup(hEtax->dppt.bufHttpRep, -1));
                break;            
            }
        }
        nChildRet = evbuffer_get_length(hEtax->dppt.bufHttpRep);

        *(parametetr->fpkjdata) = NULL;
        *(parametetr->fpkjdata) = (char *)malloc(nChildRet);
        if(*(parametetr->fpkjdata) == NULL){    
            break;
        }
        evbuffer_copyout_from(hEtax->dppt.bufHttpRep,NULL, *(parametetr->fpkjdata),nChildRet);
        //结束
        nRet = RET_SUCCESS;
    }while (false);

    return nRet; 
}

//授信额度 bycookies
int etax_jiangsu_credit_limit_by_cookie(HETAX hEtax, HDEV hDev,void *args,char *errinfo_uft8)
{
    CREDIT_LIMITB_BY_COOKIE *parametetr = (CREDIT_LIMITB_BY_COOKIE *)args;
    struct evbuffer *evDpptCookies = NULL;
    int nRet = ERR_GENERIC,nChildRet = ERR_GENERIC;
    char nsrsbhbuf[BS_NORMAL]={0};
    hEtax->etax.user_args = NULL;
    if(NULL == parametetr)
    {
        return nRet;
    }
    do
    {
        //参数检测
        if (!parametetr->dppt_cookie || strlen(parametetr->dppt_cookie) < 16)
        {
            break;
        }
        if (strchr(parametetr->dppt_cookie, '\n'))
        {
            break;
        }
        if (!(evDpptCookies = evbuffer_new()))
        {
            break;
        }
        evbuffer_add_printf(evDpptCookies, "%s", parametetr->dppt_cookie);
        DpptJiangSuConnectBycookie(&hEtax->dppt,hDev);
        evbuffer_add_printf(hEtax->dppt.ctxCon.bufCookies, "; %s",evbuffer_pullup(evDpptCookies, -1));
        EtaxRemoveRepeatOldCookies(hEtax->dppt.ctxCon.bufCookies);

        if ((nChildRet = InitSecurityConfig(&hEtax->dppt))) {
            if(nChildRet == INT32_MIN){
                sprintf((char *)errinfo_uft8, "406该Cookies已失效,请重新请求该接口");   
                break;                
            }
            else 
            {
                sprintf((char *)errinfo_uft8, "406初始化dppt服务器失败,错误状态为:%d", nChildRet);    
                break;
            }
        }

        //步骤7 授信额度查询
        sprintf(nsrsbhbuf,"{\"kpysfid\":\"%s\",\"nsrsbh\":\"%s\"}",parametetr->nsrsbh,parametetr->nsrsbh);

        if ((nChildRet = EtaxQueryInvoice(hEtax,"/kpfw/sjtj/v1/info",nsrsbhbuf)) < 0){
            sprintf((char *)errinfo_uft8, "406查询接口失败:%s\r\n",(const char *)evbuffer_pullup(hEtax->dppt.bufHttpRep, -1));
            break;                      
        }  
        nChildRet = evbuffer_get_length(hEtax->dppt.bufHttpRep);

        *(parametetr->sxeddata) = NULL;
        *(parametetr->sxeddata) = (char *)malloc(nChildRet);
        if(*(parametetr->sxeddata) == NULL){    
            break;
        }
        evbuffer_copyout_from(hEtax->dppt.bufHttpRep,NULL, *(parametetr->sxeddata),nChildRet);
        //结束
        nRet = RET_SUCCESS;
    }while (false);

    return nRet; 
}

//红字类查询 bycookies
int etax_jiangsu_hcstaquery_invoice_issuance_by_cookie(HETAX hEtax, HDEV hDev,void *args,char *errinfo_uft8)
{
    INVOICE_HCSTAQUERY_BY_COOKIE *parametetr = (INVOICE_HCSTAQUERY_BY_COOKIE *)args;
    struct evbuffer *evDpptCookies = NULL;
    int nRet = ERR_GENERIC,nChildRet = ERR_GENERIC;
    char *nEtaxBufData = NULL;
    if(NULL == parametetr)
    {
        return nRet;
    }
    do
    {
        //参数检测
        if (!parametetr->dppt_cookie || strlen(parametetr->dppt_cookie) < 16)
        {
            break;
        }
        if (strchr(parametetr->dppt_cookie, '\n'))
        {
            break;
        }
        if (!(evDpptCookies = evbuffer_new()))
        {
            break;
        }
        evbuffer_add_printf(evDpptCookies, "%s", parametetr->dppt_cookie);
        DpptJiangSuConnectBycookie(&hEtax->dppt,hDev);
        evbuffer_add_printf(hEtax->dppt.ctxCon.bufCookies, "; %s",evbuffer_pullup(evDpptCookies, -1));
        EtaxRemoveRepeatOldCookies(hEtax->dppt.ctxCon.bufCookies);

        if ((nChildRet = InitSecurityConfig(&hEtax->dppt))) {
            if(nChildRet == INT32_MIN){
                sprintf((char *)errinfo_uft8, "406该Cookies已失效,请重新请求该接口");   
                break;                
            }
            else 
            {
                sprintf((char *)errinfo_uft8, "406初始化dppt服务器失败,错误状态为:%d", nChildRet);    
                break;
            }
        }

        if ((nChildRet = EtaxQueryInvoice(hEtax, "/szzhzz/fpcx/v1/queryFphcztxx",parametetr->fpcxsj)) < 0){
            sprintf((char *)errinfo_uft8, "406发票查询接口失败:%s",evbuffer_pullup(hEtax->dppt.bufHttpRep, -1));
            break;                      
        }  
        nChildRet = evbuffer_get_length(hEtax->dppt.bufHttpRep);

        nEtaxBufData = (char *)malloc(nChildRet);
        if(nEtaxBufData == NULL)
            break;

        *(parametetr->fpcxdata) = NULL;
        *(parametetr->fpcxdata) = (char *)malloc(nChildRet);
        if(*(parametetr->fpcxdata) == NULL){    
            break;
        }
        evbuffer_copyout_from(hEtax->dppt.bufHttpRep,NULL, nEtaxBufData,nChildRet);
        EncordingConvert( "utf-8","gbk",nEtaxBufData, nChildRet, *(parametetr->fpcxdata), nChildRet);

        //结束
        nRet = RET_SUCCESS;
    }while (false);


    //释放内存
    if(nEtaxBufData)
        free(nEtaxBufData);
    nEtaxBufData = NULL;

    return nRet; 
}

//蓝字发票开具 bycookie
int etax_jiangsu_blue_ink_invoice_issuance_bycookies(HETAX hEtax, HDEV hDev,void *args,char *errinfo_uft8)
{
    BLUE_INK_INVOICE_ISSUANCE_BY_COOKIE *parametetr = (BLUE_INK_INVOICE_ISSUANCE_BY_COOKIE *)args;
    struct evbuffer *evDpptCookies = NULL;
    int nRet = ERR_GENERIC,nChildRet = ERR_GENERIC;
    char *nEtaxBufData = NULL;
    char *nsrdata = NULL;

    EHFPXX fpxx = NULL;

    hEtax->etax.user_args = NULL;
    if(NULL == parametetr)
    {
        return nRet;
    }
    do
    {
        if (!(fpxx = EMallocFpxxLinkDev(hDev)))
            break;
        //参数检测
        if (!parametetr->dppt_cookie || strlen(parametetr->dppt_cookie) < 16)
        {
            break;
        }
        if (strchr(parametetr->dppt_cookie, '\n'))
        {
            break;
        }
        if (!(evDpptCookies = evbuffer_new()))
        {
            break;
        }
        evbuffer_add_printf(evDpptCookies, "%s", parametetr->dppt_cookie);
        DpptJiangSuConnectBycookie(&hEtax->dppt,hDev);
        evbuffer_add_printf(hEtax->dppt.ctxCon.bufCookies, "; %s",evbuffer_pullup(evDpptCookies, -1));
        EtaxRemoveRepeatOldCookies(hEtax->dppt.ctxCon.bufCookies);

        if ((nChildRet = InitSecurityConfig(&hEtax->dppt))) {
            if(nChildRet == INT32_MIN){
                sprintf((char *)errinfo_uft8, "406该Cookies已失效,请重新请求该接口");   
                break;                
            }
            else 
            {
                sprintf((char *)errinfo_uft8, "406初始化dppt服务器失败,错误状态为:%d", nChildRet);    
                break;
            }
        }

        if (EtaxLoadDevOnline(hEtax, hDev,fpxx)){
            sprintf((char *)errinfo_uft8, "406税务机关代码查询或纳税人注册信息查询失败");    
            break; 
        }
        //输出纳税人信息
        cJSON *json = cJSON_CreateObject();
        cJSON_AddStringToObject(json, "Nsrsbh", hDev->szCommonTaxID);
        cJSON_AddStringToObject(json, "Scjydz",fpxx->xfdz);
        cJSON_AddStringToObject(json, "Zcdlxdh",fpxx->xfdh);
        cJSON_AddStringToObject(json, "Nsrmc", fpxx->xfmc);
        cJSON_AddStringToObject(json, "Ssdabh", fpxx->xfsh);
        cJSON_AddStringToObject(json, "Yhzh", fpxx->xfyhzh);
        cJSON_AddStringToObject(json, "Yhyywdmc",fpxx->xfkhh);

        if (!(nsrdata = cJSON_Print(json)))
            break;

        printf("DpptEncryptIo strlen(nsrdata) =%d \r\n data = %s\r\n",strlen(nsrdata),nsrdata);    

        *(parametetr->hqnsrjcxx) = NULL;
        *(parametetr->hqnsrjcxx) = (char *)malloc(strlen(nsrdata));
        if(*(parametetr->hqnsrjcxx) == NULL){    
            break;
        }

        EncordingConvert( "utf-8","gbk",nsrdata, strlen(nsrdata), *(parametetr->hqnsrjcxx), strlen(nsrdata));

        //分析json
        printf("parametetr->fpkjjson=%s\r\n",parametetr->fpkjjson);
        if (EtaxAnalyzeJsonBuf(parametetr->fpkjjson, fpxx,hDev,errinfo_uft8))
            break;       
        //调用开票接口开票，可重复循环开具
        if (EtaxMakeInvoiceMain(hEtax,fpxx,hDev,errinfo_uft8,parametetr->fpkjjson) < 0) {
            break;
        }
        
        nChildRet = evbuffer_get_length(hEtax->dppt.bufHttpRep);

        *(parametetr->fpkjout) = NULL;
        *(parametetr->fpkjout) = (char *)malloc(nChildRet);
        if(*(parametetr->fpkjout) == NULL){    
            break;
        }
        evbuffer_copyout_from(hEtax->dppt.bufHttpRep,NULL, *(parametetr->fpkjout),nChildRet);
        //结束
        nRet = RET_SUCCESS;
    }while (false);

    return nRet;
}

//可开红票的蓝票查询 bycookie
int etax_jiangsu_hzqrxxSzzhQuery_bycookies(HETAX hEtax, HDEV hDev,void *args,char *errinfo_uft8)
{
    INVOICE_QUERY_BY_COOKIES *parametetr = (INVOICE_QUERY_BY_COOKIES *)args;
    struct evbuffer *evDpptCookies = NULL;
    int nRet = ERR_GENERIC,nChildRet = ERR_GENERIC;
    char *nEtaxBufData = NULL;
    if(NULL == parametetr)
    {
        return nRet;
    }
    do
    {
        //参数检测
        if (!parametetr->dppt_cookie || strlen(parametetr->dppt_cookie) < 16)
        {
            break;
        }
        if (strchr(parametetr->dppt_cookie, '\n'))
        {
            break;
        }
        if (!(evDpptCookies = evbuffer_new()))
        {
            break;
        }

        evbuffer_add_printf(evDpptCookies, "%s", parametetr->dppt_cookie);
        DpptJiangSuConnectBycookie(&hEtax->dppt,hDev);
        evbuffer_add_printf(hEtax->dppt.ctxCon.bufCookies, "; %s",evbuffer_pullup(evDpptCookies, -1));
        EtaxRemoveRepeatOldCookies(hEtax->dppt.ctxCon.bufCookies);

        if ((nChildRet = InitSecurityConfig(&hEtax->dppt))) {
            if(nChildRet == INT32_MIN){
                sprintf((char *)errinfo_uft8, "406该Cookies已失效,请重新请求该接口");   
                break;                
            }
            else 
            {
                sprintf((char *)errinfo_uft8, "406初始化dppt服务器失败,错误状态为:%d", nChildRet);    
                break;
            }
        }

        //步骤7 发票查询(列表)
        if ((nChildRet = EtaxQueryInvoice(hEtax, "/kpfw/hzqrxxSzzhQuery/v1/queryLzfpList",parametetr->fpcxsj)) < 0){
            sprintf((char *)errinfo_uft8, "406查询接口失败:%s\r\n",(const char *)evbuffer_pullup(hEtax->dppt.bufHttpRep, -1));
            break;                      
        }  
        nChildRet = evbuffer_get_length(hEtax->dppt.bufHttpRep);

        nEtaxBufData = (char *)malloc(nChildRet);
        if(nEtaxBufData == NULL)
            break;

        *(parametetr->fpcxdata) = NULL;
        *(parametetr->fpcxdata) = (char *)malloc(nChildRet);
        if(*(parametetr->fpcxdata) == NULL){    
            break;
        }
        evbuffer_copyout_from(hEtax->dppt.bufHttpRep,NULL, nEtaxBufData,nChildRet);
        EncordingConvert( "utf-8","gbk",nEtaxBufData, nChildRet, *(parametetr->fpcxdata), nChildRet);
        //结束
        nRet = RET_SUCCESS;
    }while (false);
    //释放内存
    if(NULL != nEtaxBufData)
        free(nEtaxBufData);
    nEtaxBufData = NULL;
    return nRet;
}

//红字确认单开具 bycookie
int etax_jiangsu_hzqrxxSave_bycookies(HETAX hEtax, HDEV hDev,void *args,char *errinfo_uft8)
{
    INVOICE_QUERY_BY_COOKIES *parametetr = (INVOICE_QUERY_BY_COOKIES *)args;
    struct evbuffer *evDpptCookies = NULL;
    int nRet = ERR_GENERIC,nChildRet = ERR_GENERIC;
    char *nEtaxBufData = NULL;
    hEtax->etax.user_args = NULL;
    if(NULL == parametetr)
    {
        return nRet;
    }
    do
    {
       //参数检测
        if (!parametetr->dppt_cookie || strlen(parametetr->dppt_cookie) < 16)
        {
            break;
        }
        if (strchr(parametetr->dppt_cookie, '\n'))
        {
            break;
        }
        if (!(evDpptCookies = evbuffer_new()))
        {
            break;
        }
        evbuffer_add_printf(evDpptCookies, "%s", parametetr->dppt_cookie);
        DpptJiangSuConnectBycookie(&hEtax->dppt,hDev);
        evbuffer_add_printf(hEtax->dppt.ctxCon.bufCookies, "; %s",evbuffer_pullup(evDpptCookies, -1));
        EtaxRemoveRepeatOldCookies(hEtax->dppt.ctxCon.bufCookies);

        if ((nChildRet = InitSecurityConfig(&hEtax->dppt))) {
            if(nChildRet == INT32_MIN){
                sprintf((char *)errinfo_uft8, "406该Cookies已失效,请重新请求该接口");   
                break;                
            }
            else 
            {
                sprintf((char *)errinfo_uft8, "406初始化dppt服务器失败,错误状态为:%d", nChildRet);    
                break;
            }
        }

        //步骤7 发票查询(列表)
        if ((nChildRet = EtaxQueryInvoice(hEtax, "/kpfw/hzqrxxSave/v1/saveHzqrxxForHztzd",parametetr->fpcxsj)) < 0){
            sprintf((char *)errinfo_uft8, "406查询接口失败:%s\r\n",(const char *)evbuffer_pullup(hEtax->dppt.bufHttpRep, -1));
            break;                      
        }  
        nChildRet = evbuffer_get_length(hEtax->dppt.bufHttpRep);

        nEtaxBufData = (char *)malloc(nChildRet);
        if(nEtaxBufData == NULL)
            break;

        *(parametetr->fpcxdata) = NULL;
        *(parametetr->fpcxdata) = (char *)malloc(nChildRet);
        if(*(parametetr->fpcxdata) == NULL){    
            break;
        }
        evbuffer_copyout_from(hEtax->dppt.bufHttpRep,NULL, nEtaxBufData,nChildRet);
        EncordingConvert( "utf-8","gbk",nEtaxBufData, nChildRet, *(parametetr->fpcxdata), nChildRet);
        //结束
        nRet = RET_SUCCESS;
    }while (false);
    //释放内存
    if(NULL != nEtaxBufData)
        free(nEtaxBufData);
    nEtaxBufData = NULL;
    return nRet;
}

//红字确认单列表查询 bycookie
int etax_jiangsu_hzqrxxQuery_bycookies(HETAX hEtax, HDEV hDev,void *args,char *errinfo_uft8)
{
    INVOICE_QUERY_BY_COOKIES *parametetr = (INVOICE_QUERY_BY_COOKIES *)args;
    struct evbuffer *evDpptCookies = NULL;
    int nRet = ERR_GENERIC,nChildRet = ERR_GENERIC;
    char *nEtaxBufData = NULL;
    if(NULL == parametetr)
    {
        return nRet;
    }
    do
    {
        //参数检测
        if (!parametetr->dppt_cookie || strlen(parametetr->dppt_cookie) < 16)
        {
            break;
        }
        if (strchr(parametetr->dppt_cookie, '\n'))
        {
            break;
        }
        if (!(evDpptCookies = evbuffer_new()))
        {
            break;
        }
        evbuffer_add_printf(evDpptCookies, "%s", parametetr->dppt_cookie);
        DpptJiangSuConnectBycookie(&hEtax->dppt,hDev);
        evbuffer_add_printf(hEtax->dppt.ctxCon.bufCookies, "; %s",evbuffer_pullup(evDpptCookies, -1));
        EtaxRemoveRepeatOldCookies(hEtax->dppt.ctxCon.bufCookies);

        if ((nChildRet = InitSecurityConfig(&hEtax->dppt))) {
            if(nChildRet == INT32_MIN){
                sprintf((char *)errinfo_uft8, "406该Cookies已失效,请重新请求该接口");   
                break;                
            }
            else 
            {
                sprintf((char *)errinfo_uft8, "406初始化dppt服务器失败,错误状态为:%d", nChildRet);    
                break;
            }
        }

       //步骤7 发票查询(列表)
        if ((nChildRet = EtaxQueryInvoice(hEtax, "/kpfw/hzqrxxQuery/v1/queryHzqrxxForHzqrd",parametetr->fpcxsj)) < 0){
            sprintf((char *)errinfo_uft8, "406查询接口失败:%s\r\n",(const char *)evbuffer_pullup(hEtax->dppt.bufHttpRep, -1));
            break;                      
        }  
        nChildRet = evbuffer_get_length(hEtax->dppt.bufHttpRep);

        nEtaxBufData = (char *)malloc(nChildRet);
        if(nEtaxBufData == NULL)
            break;

        *(parametetr->fpcxdata) = NULL;
        *(parametetr->fpcxdata) = (char *)malloc(nChildRet);
        if(*(parametetr->fpcxdata) == NULL){    
            break;
        }
        evbuffer_copyout_from(hEtax->dppt.bufHttpRep,NULL, nEtaxBufData,nChildRet);
        EncordingConvert( "utf-8","gbk",nEtaxBufData, nChildRet, *(parametetr->fpcxdata), nChildRet);
        //结束
        nRet = RET_SUCCESS;
    }while (false);
    //释放内存
    if(NULL != nEtaxBufData)
        free(nEtaxBufData);
    nEtaxBufData = NULL;
    return nRet;
}

//红字确认单详情查询 bycookie
int etax_jiangsu_hzqrxxQueryDetail_bycookies(HETAX hEtax, HDEV hDev,void *args,char *errinfo_uft8)
{
    INVOICE_QUERY_BY_COOKIES *parametetr = (INVOICE_QUERY_BY_COOKIES *)args;
    struct evbuffer *evDpptCookies = NULL;
    int nRet = ERR_GENERIC,nChildRet = ERR_GENERIC;
    char *nEtaxBufData = NULL;
    if(NULL == parametetr)
    {
        return nRet;
    }
    do
    {
        //参数检测
        if (!parametetr->dppt_cookie || strlen(parametetr->dppt_cookie) < 16)
        {
            break;
        }
        if (strchr(parametetr->dppt_cookie, '\n'))
        {
            break;
        }
        if (!(evDpptCookies = evbuffer_new()))
        {
            break;
        }
        evbuffer_add_printf(evDpptCookies, "%s", parametetr->dppt_cookie);
        DpptJiangSuConnectBycookie(&hEtax->dppt,hDev);
        evbuffer_add_printf(hEtax->dppt.ctxCon.bufCookies, "; %s",evbuffer_pullup(evDpptCookies, -1));
        EtaxRemoveRepeatOldCookies(hEtax->dppt.ctxCon.bufCookies);

        if ((nChildRet = InitSecurityConfig(&hEtax->dppt))) {
            if(nChildRet == INT32_MIN){
                sprintf((char *)errinfo_uft8, "406该Cookies已失效,请重新请求该接口");   
                break;                
            }
            else 
            {
                sprintf((char *)errinfo_uft8, "406初始化dppt服务器失败,错误状态为:%d", nChildRet);    
                break;
            }
        }

        if ((nChildRet = EtaxQueryInvoice(hEtax, "/kpfw/hzqrxxQuery/v1/queryHzqrxxDetail",parametetr->fpcxsj)) < 0){
            sprintf((char *)errinfo_uft8, "406查询接口失败:%s\r\n",(const char *)evbuffer_pullup(hEtax->dppt.bufHttpRep, -1));
            break;                      
        }  
        nChildRet = evbuffer_get_length(hEtax->dppt.bufHttpRep);

        nEtaxBufData = (char *)malloc(nChildRet);
        if(nEtaxBufData == NULL)
            break;

        *(parametetr->fpcxdata) = NULL;
        *(parametetr->fpcxdata) = (char *)malloc(nChildRet);
        if(*(parametetr->fpcxdata) == NULL){    
            break;
        }
        evbuffer_copyout_from(hEtax->dppt.bufHttpRep,NULL, nEtaxBufData,nChildRet);
        EncordingConvert( "utf-8","gbk",nEtaxBufData, nChildRet, *(parametetr->fpcxdata), nChildRet);
        //结束
        nRet = RET_SUCCESS;
    }while (false);
    //释放内存
    if(NULL != nEtaxBufData)
        free(nEtaxBufData);
    nEtaxBufData = NULL;
    return nRet;
}

int etax_jiangsu_fjxxpeizhi_query_bycookies(HETAX hEtax, HDEV hDev,void *args,char *errinfo_uft8)
{
    struct evhttp_uri *uriTpassFull = NULL;
    FJXXCJMB_MODE_BY_COOKIE *parametetr = (FJXXCJMB_MODE_BY_COOKIE *)args;
    struct evbuffer *evDpptCookies = NULL;
    int nRet = ERR_GENERIC,nChildRet = ERR_GENERIC;
    if(NULL == parametetr)
    {
        return nRet;
    }
    do
    {
        //参数检测
        if (!parametetr->dppt_cookie || strlen(parametetr->dppt_cookie) < 16)
        {
            break;
        }
        if (strchr(parametetr->dppt_cookie, '\n'))
        {
            break;
        }
        if (!(evDpptCookies = evbuffer_new()))
        {
            break;
        }
        evbuffer_add_printf(evDpptCookies, "%s", parametetr->dppt_cookie);
        DpptJiangSuConnectBycookie(&hEtax->dppt,hDev);
        evbuffer_add_printf(hEtax->dppt.ctxCon.bufCookies, "; %s",evbuffer_pullup(evDpptCookies, -1));
        EtaxRemoveRepeatOldCookies(hEtax->dppt.ctxCon.bufCookies);

        if ((nChildRet = InitSecurityConfig(&hEtax->dppt))) {
            if(nChildRet == INT32_MIN){
                sprintf((char *)errinfo_uft8, "406该Cookies已失效,请重新请求该接口");   
                break;                
            }
            else 
            {
                sprintf((char *)errinfo_uft8, "406初始化dppt服务器失败,错误状态为:%d", nChildRet);    
                break;
            }
        }
        if(atoi(parametetr->mode) == 0){
            if ((nChildRet = EtaxQueryInvoice(hEtax, "/jcxxwh/jcxxFpfjysxx/v1/create",parametetr->fpcxsj)) < 0){
                sprintf((char *)errinfo_uft8, "406附加信息创建失败:%s",evbuffer_pullup(hEtax->dppt.bufHttpRep, -1));
                break;                     
            }  
        }

        if(atoi(parametetr->mode) == 1){
            if ((nChildRet = EtaxQueryInvoice(hEtax, "/jcxxwh/jcxxFpfjysxx/v1/modify",parametetr->fpcxsj)) < 0){
                sprintf((char *)errinfo_uft8, "406附加信息修改失败:%s",evbuffer_pullup(hEtax->dppt.bufHttpRep, -1));
                break;                     
            }                     
        }

        if(atoi(parametetr->mode) == 2){
            if ((nChildRet = EtaxQueryInvoice(hEtax, "/jcxxwh/jcxxFpfjysxx/v1/delete",parametetr->fpcxsj)) < 0){
                sprintf((char *)errinfo_uft8, "406附加信息删除失败:%s",evbuffer_pullup(hEtax->dppt.bufHttpRep, -1));
                break;                     
            }                      
        }

        if(atoi(parametetr->mode) == 3){
            if ((nChildRet = EtaxQueryInvoice(hEtax, "/jcxxwh/jcxxFpfjysxx/v1/queryPageList",parametetr->fpcxsj)) < 0){
                sprintf((char *)errinfo_uft8, "406附加信息列表查询失败:%s",evbuffer_pullup(hEtax->dppt.bufHttpRep, -1));
                break;                     
            }                      
        }
        else 
        {
            if ((nChildRet = EtaxQueryInvoice(hEtax, "/jcxxwh/jcxxFpfjysxx/v1/queryPageList","{\"fjysxmmc\":\"\",\"order\":\"desc\",\"sortBy\":\"Lrrq\",\"pageNumber\":1,\"pageSize\":100}")) < 0){
                sprintf((char *)errinfo_uft8, "406附加信息和场景模版接口请求失败:%s",evbuffer_pullup(hEtax->dppt.bufHttpRep, -1));
                break;                      
            } 
        }

        nChildRet = evbuffer_get_length(hEtax->dppt.bufHttpRep);

        int ChildRetLen = nChildRet + 1024;

        *(parametetr->fpcxdata) = NULL;
        *(parametetr->fpcxdata) = (char *)malloc(ChildRetLen);
        if(*(parametetr->fpcxdata) == NULL){    
            break;
        }
		asc_compress_base64(evbuffer_pullup(hEtax->dppt.bufHttpRep, -1), nChildRet, *(parametetr->fpcxdata), &ChildRetLen);
        //结束
        nRet = RET_SUCCESS;
    }while (false);

    return nRet;
}

int etax_jiangsu_cjmbpeizhi_query_bycookies(HETAX hEtax, HDEV hDev,void *args,char *errinfo_uft8)
{
    FJXXCJMB_MODE_BY_COOKIE *parametetr = (FJXXCJMB_MODE_BY_COOKIE *)args;
    struct evbuffer *evDpptCookies = NULL;
    int nRet = ERR_GENERIC,nChildRet = ERR_GENERIC;
    if(NULL == parametetr)
    {
        return nRet;
    }
    do
    {
        //参数检测
        if (!parametetr->dppt_cookie || strlen(parametetr->dppt_cookie) < 16)
        {
            break;
        }
        if (strchr(parametetr->dppt_cookie, '\n'))
        {
            break;
        }
        if (!(evDpptCookies = evbuffer_new()))
        {
            break;
        }
        evbuffer_add_printf(evDpptCookies, "%s", parametetr->dppt_cookie);
        DpptJiangSuConnectBycookie(&hEtax->dppt,hDev);
        evbuffer_add_printf(hEtax->dppt.ctxCon.bufCookies, "; %s",evbuffer_pullup(evDpptCookies, -1));
        EtaxRemoveRepeatOldCookies(hEtax->dppt.ctxCon.bufCookies);

        if ((nChildRet = InitSecurityConfig(&hEtax->dppt))) {
            if(nChildRet == INT32_MIN){
                sprintf((char *)errinfo_uft8, "406该Cookies已失效,请重新请求该接口");   
                break;                
            }
            else 
            {
                sprintf((char *)errinfo_uft8, "406初始化dppt服务器失败,错误状态为:%d", nChildRet);    
                break;
            }
        }

        if(atoi(parametetr->mode) == 0){
            if ((nChildRet = EtaxQueryInvoice(hEtax, "/jcxxwh/fpfjyscjmbxx/v1/create",parametetr->fpcxsj)) < 0){
                sprintf((char *)errinfo_uft8, "406附加信息创建失败:%s",evbuffer_pullup(hEtax->dppt.bufHttpRep, -1));
                break;                     
            }  
        }

        if(atoi(parametetr->mode) == 1){
            if ((nChildRet = EtaxQueryInvoice(hEtax, "/jcxxwh/fpfjyscjmbxx/v1/modify",parametetr->fpcxsj)) < 0){
                sprintf((char *)errinfo_uft8, "406附加信息修改失败:%s",evbuffer_pullup(hEtax->dppt.bufHttpRep, -1));
                break;                     
            }                     
        }

        if(atoi(parametetr->mode) == 2){
            if ((nChildRet = EtaxQueryInvoice(hEtax, "/jcxxwh/fpfjyscjmbxx/v1/delete",parametetr->fpcxsj)) < 0){
                sprintf((char *)errinfo_uft8, "406附加信息删除失败:%s",evbuffer_pullup(hEtax->dppt.bufHttpRep, -1));
                break;                     
            }                      
        }

        if(atoi(parametetr->mode) == 3){
            if ((nChildRet = EtaxQueryInvoice(hEtax, "/jcxxwh/fpfjyscjmbxx/v1/queryPageList",parametetr->fpcxsj)) < 0){
                sprintf((char *)errinfo_uft8, "406场景模版列表列表查询失败:%s",evbuffer_pullup(hEtax->dppt.bufHttpRep, -1));
                break;                     
            }                      
        }
        else 
        {
            if ((nChildRet = EtaxQueryInvoice(hEtax, "/jcxxwh/fpfjyscjmbxx/v1/queryPageList","{\"fjysxmmc\":\"\",\"order\":\"desc\",\"sortBy\":\"Lrrq\",\"pageNumber\":1,\"pageSize\":100}")) < 0){
                sprintf((char *)errinfo_uft8, "406场景模版列表列表查询失败:%s",evbuffer_pullup(hEtax->dppt.bufHttpRep, -1));
                break;                      
            }  
        }

        nChildRet = evbuffer_get_length(hEtax->dppt.bufHttpRep);

        int ChildRetLen = nChildRet + 1024;

        *(parametetr->fpcxdata) = NULL;
        *(parametetr->fpcxdata) = (char *)malloc(ChildRetLen);
        if(*(parametetr->fpcxdata) == NULL){    
            break;
        }
		asc_compress_base64(evbuffer_pullup(hEtax->dppt.bufHttpRep, -1), nChildRet, *(parametetr->fpcxdata), &ChildRetLen);
        //结束
        nRet = RET_SUCCESS;
    }while (false);
    return nRet;
}

int etax_jiangsu_get_nsrjcxx_by_cookies(HETAX hEtax, HDEV hDev,void *args,char *errinfo_uft8)
{
    NRSXXCX_BY_COOKIE *parametetr = (NRSXXCX_BY_COOKIE *)args;
    struct evbuffer *evDpptCookies = NULL;
    int nRet = ERR_GENERIC,nChildRet = ERR_GENERIC;
    if(NULL == parametetr)
    {
        return nRet;
    }
    do
    {
        //参数检测
        if (!parametetr->dppt_cookie || strlen(parametetr->dppt_cookie) < 16)
        {
            break;
        }
        if (strchr(parametetr->dppt_cookie, '\n'))
        {
            break;
        }
        if (!(evDpptCookies = evbuffer_new()))
        {
            break;
        }

        evbuffer_add_printf(evDpptCookies, "%s", parametetr->dppt_cookie);
        DpptJiangSuConnectBycookie(&hEtax->dppt,hDev);
        evbuffer_add_printf(hEtax->dppt.ctxCon.bufCookies, "; %s",evbuffer_pullup(evDpptCookies, -1));
        EtaxRemoveRepeatOldCookies(hEtax->dppt.ctxCon.bufCookies);

        if ((nChildRet = InitSecurityConfig(&hEtax->dppt))) {
            if(nChildRet == INT32_MIN){
                sprintf((char *)errinfo_uft8, "406该Cookies已失效,请重新请求该接口");   
                break;                
            }
            else 
            {
                sprintf((char *)errinfo_uft8, "406初始化dppt服务器失败,错误状态为:%d", nChildRet);    
                break;
            }
        }

        if ((nChildRet = EtaxQueryInvoice(hEtax, "/kpfw/hqnsrjcxx/v1/hqnsrjcxx","{\"Nsrsbh\":\"\"}")) < 0){
            sprintf((char *)errinfo_uft8, "406纳税人基本信息查询失败:%s",evbuffer_pullup(hEtax->dppt.bufHttpRep, -1));
            break;                      
        }  

        nChildRet = evbuffer_get_length(hEtax->dppt.bufHttpRep);

        *(parametetr->fpcxdata) = NULL;
        *(parametetr->fpcxdata) = (char *)malloc(nChildRet);
        if(*(parametetr->fpcxdata) == NULL){    
            break;
        }

        EncordingConvert( "utf-8","gbk",evbuffer_pullup(hEtax->dppt.bufHttpRep, -1), nChildRet, *(parametetr->fpcxdata), nChildRet);
        //结束
        nRet = RET_SUCCESS;
    }while (false);

    return nRet;
}

int etax_jiangsu_get_nsrfxxx_by_cookies(HETAX hEtax, HDEV hDev,void *args,char *errinfo_uft8)
{
    NRSXXCX_BY_COOKIE *parametetr = (NRSXXCX_BY_COOKIE *)args;
    struct evbuffer *evDpptCookies = NULL;
    int nRet = ERR_GENERIC,nChildRet = ERR_GENERIC;
    hEtax->etax.user_args = NULL;
    if(NULL == parametetr)
    {
        return nRet;
    }
    do
    {
        //参数检测
        if (!parametetr->dppt_cookie || strlen(parametetr->dppt_cookie) < 16)
        {
            break;
        }
        if (strchr(parametetr->dppt_cookie, '\n'))
        {
            break;
        }
        if (!(evDpptCookies = evbuffer_new()))
        {
            break;
        }
        evbuffer_add_printf(evDpptCookies, "%s", parametetr->dppt_cookie);
        DpptJiangSuConnectBycookie(&hEtax->dppt,hDev);
        evbuffer_add_printf(hEtax->dppt.ctxCon.bufCookies, "; %s",evbuffer_pullup(evDpptCookies, -1));
        EtaxRemoveRepeatOldCookies(hEtax->dppt.ctxCon.bufCookies);

        if ((nChildRet = InitSecurityConfig(&hEtax->dppt))) {
            if(nChildRet == INT32_MIN){
                sprintf((char *)errinfo_uft8, "406该Cookies已失效,请重新请求该接口");   
                break;                
            }
            else 
            {
                sprintf((char *)errinfo_uft8, "406初始化dppt服务器失败,错误状态为:%d", nChildRet);    
                break;
            }
        }

        if ((nChildRet = EtaxQueryInvoice(hEtax, "/kpfw/fjxx/v1/sxlb/search","{\"nsrsbh\":\"\",\"dyfs\":\"0\"}")) < 0){
            sprintf((char *)errinfo_uft8, "406纳税人风险信息查询失败:%s",evbuffer_pullup(hEtax->dppt.bufHttpRep, -1));
            break;                      
        }  

        nChildRet = evbuffer_get_length(hEtax->dppt.bufHttpRep);


        *(parametetr->fpcxdata) = NULL;
        *(parametetr->fpcxdata) = (char *)malloc(nChildRet);
        if(*(parametetr->fpcxdata) == NULL){    
            break;
        }

        EncordingConvert( "utf-8","gbk",evbuffer_pullup(hEtax->dppt.bufHttpRep, -1), nChildRet, *(parametetr->fpcxdata), nChildRet);
        //结束
        nRet = RET_SUCCESS;
    }while (false);

    return nRet;
}