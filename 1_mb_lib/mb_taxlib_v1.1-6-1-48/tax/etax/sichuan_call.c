/*****************************************************************************
File name:   sichuan_call.c
Description: 用于四川省调用etax接口
Author:      Wang
Version:     1.0
Date:        2023.02
History:
*****************************************************************************/
#include "sichuan_call.h"
#include "../common/console.h"

int etax_sichuan_login(HETAX hEtax, HDEV hDev,void *args,char *errinfo_uft8);
int etax_sichuan_user_query(HETAX hEtax, HDEV hDev,void *args,char *errinfo_uft8);
int etax_sichuan_enterpriselist(HETAX hEtax, HDEV hDev,void *args,char *errinfo_uft8);
int etax_sichuan_relationchange(HETAX hEtax, HDEV hDev,void *args,char *errinfo_uft8);

ETAX_FUCTION etax_sichuan_fuction = 
{
    .etax_login = etax_sichuan_login,
    .etax_user_query = etax_sichuan_user_query,
    .etax_enterpriselist = etax_sichuan_enterpriselist,
    .etax_relationchange = etax_sichuan_relationchange
};

//自定义header
HEADER_STRUCT header_array_sichuan[] = {
    {"Cookie", "oauth2_referer=szzhzz.sichuan.chinatax.gov.cn; x_host_key=188715edd92-73398ffbd187ed688673e9dcd9a219287ac81e5c"},
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

int get_tpass_addr_sichuan(char *src_data,char **tpass_addr,char *sys_type)
{
    char *ptr_start = NULL,*ptr_end = NULL;
    int len = 0,nRet = RET_SUCCESS;
    if(src_data == NULL)
    {
        return -1;
    }
    if(0 == atoi(sys_type))   //默认的税
    {
        if(NULL == (ptr_start = strstr(src_data,"var kxUrl = \"")))
        {
            _WriteLog(LL_WARN, "Get server url failed 1");
            nRet = -5;
        }
        if(NULL == (ptr_end = strstr(ptr_start + strlen("var kxUrl = \"") + 1,"\"")))
        {
            _WriteLog(LL_WARN, "Get server url failed 2");
            nRet = -6;
        }
        len = ptr_end - ptr_start - strlen("var kxUrl = \"");

        *tpass_addr = calloc(1,len + 1);
        memcpy(*tpass_addr,ptr_start + strlen("var kxUrl = \""),len);
        printf("lbc-debug tpass_addr111 = %s\n",*tpass_addr);
        return nRet;
    }
    else
    {
        *tpass_addr = calloc(1,strlen(src_data) + 1);
        strcpy(*tpass_addr,src_data);
        printf("lbc-debug tpass_addr222 = %s\n",*tpass_addr);
        return nRet;
    }
    return nRet;
}

//一键登录
int etax_sichuan_login(HETAX hEtax, HDEV hDev,void *args,char *errinfo_uft8)
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
    char tpassurl[BS_HUGE] = {0};
    char final_url[BS_HUGE] = {0x00};
    char EtaxHTTPGetEventUrl[200]; //zwl

    cJSON *jsonTpassConfig = NULL;
    cJSON *root = NULL, *objItem = NULL, *objItemChild = NULL;
    cJSON *objItemChild2 = NULL,*objItemToken  = NULL;

    hEtax->etax.user_args = NULL;
    hEtax->tpass.user_args= NULL;

    if(NULL == login_parameter)
        return nRet;

    if(!atoi(login_parameter->sys_type))  //默认税
    {
        hEtax->etax.user_args = NULL;
    }
    else                                  //票
    {
        hEtax->etax.user_args = (void *)(&header_array_sichuan);
        memset(hEtax->etax.modelTaAddr.szTaxAuthorityURL,0x00,sizeof(hEtax->etax.modelTaAddr.szTaxAuthorityURL));
        strcpy(hEtax->etax.modelTaAddr.szTaxAuthorityURL,"https://dppt.sichuan.chinatax.gov.cn:8443");
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
 
        if(!atoi(login_parameter->sys_type)) 
        {
            //获取tpass地址
            /*if (EtaxHTTPGetEvent(&hEtax->etax, "/wszx-web/api/sh/login/mode") < 0){
                sprintf((char *)errinfo_uft8, "406请求跳转地址失败,HTTP Code:%d",hEtax->etax.ctxCon.nHttpRepCode);
                break;   
            }*/
            /*memset(EtaxHTTPGetEventUrl,0,sizeof(EtaxHTTPGetEventUrl)); //zwl
            sprintf(EtaxHTTPGetEventUrl,"/bszm-web/api/spcdesktop/get/kexin/login?_=%llu",GetMicroSecondUnixTimestamp()); //zwl
            
            if (EtaxHTTPGetEvent(&hEtax->etax, EtaxHTTPGetEventUrl) < 0){
                sprintf((char *)errinfo_uft8, "406请求跳转地址失败,HTTP Code:%d",hEtax->etax.ctxCon.nHttpRepCode);
                break;   
            }

            szTPassConfigJson = (char *)evbuffer_pullup(hEtax->etax.bufHttpRep, -1);*/
            pValue = calloc(1,BS_BLOCK*2);  
            strcpy(pValue,"https://tpass.sichuan.chinatax.gov.cn:8443/api/v1.0/auth/oauth2/login?redirect_uri=https://etax.sichuan.chinatax.gov.cn/login-web/api/third/sso/login/redirect?qd=KEXIN&channelId=web&goto=30010666&client_id=7ede9cef518555089c0e890ae17e5ce1&response_type=code&state=test");
           
        //    char strtemp[10]={0};
        //    sprintf(strtemp,"30010666");
        //    strcpy(login_parameter->lpgoto,strtemp);
        //    printf("etax_sichuan_login lpgoto = %s\r\n",login_parameter->lpgoto);        

            if (!(uriTpassFull = evhttp_uri_parse(pValue))){
                sprintf((char *)errinfo_uft8, "406获取跳转地址错误,地址转换失败"); 
                break;
            }

            if (!FillUploadAddrModelDynamic(TAADDR_ETAX_TPASS, hDev->szRegCode, pValue, "/",&hEtax->tpass.modelTaAddr)) {
                sprintf((char *)errinfo_uft8, "406获取跳转地址错误,地址解析失败");
                break;
            } 

            printf("szTPassConfigJson=%s\r\n",szTPassConfigJson);
            printf("tpassurl=%s\r\n",pValue);
        }
        else
        {
            szTPassConfigJson = (char *)hEtax->etax.user_args;
            //获取跳转tpass地址
            if(RET_SUCCESS != get_tpass_addr_sichuan(szTPassConfigJson,&pValue,login_parameter->sys_type))
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

            printf("szTPassConfigJson=%s\r\n",szTPassConfigJson);
            printf("tpassurl=%s\r\n",pValue);
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

        /*if(!atoi(login_parameter->sys_type)){
            if ((nChildRet = TPassLoginAuthHelloAndAuthsichuan(&hEtax->tpass, uriTpassFull))) {
                sprintf((char *)errinfo_uft8, "406签名验签服务器响应请求错误:%s",(const char *)evbuffer_pullup(hEtax->tpass.bufHttpRep, -1));
                break;
            }
        }
        else 
        {
            if ((nChildRet = TPassLoginAuthHelloAndAuth(&hEtax->tpass, uriTpassFull))) {
                sprintf((char *)errinfo_uft8, "406签名验签服务器响应请求错误:%s",(const char *)evbuffer_pullup(hEtax->tpass.bufHttpRep, -1));
                break;
            }
        }*/            
        if(hEtax->tpass.user_args) //zwl 0612
        {
            printf("lbc-debug");
            free(hEtax->tpass.user_args);
            hEtax->tpass.user_args = NULL;
            printf("lbc-debug1111");
        }
        hEtax->tpass.user_args = (void *)((char *)calloc(1,strlen(pValue) + 1));
        if(NULL == hEtax->tpass.user_args)
            break;
        strcpy((char *)hEtax->tpass.user_args,pValue); //zwl 0612

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
        /*if(!atoi(login_parameter->sys_type)){
            if (nChildRet = TPassLoginPasswordLoginDosichuan(&hEtax->tpass, szUserId, login_parameter->sz_password, szBuf,sizeof(szBuf))){
                if (!(root = cJSON_Parse(szBuf)))
                    break;
                if (!(objItem = cJSON_GetObjectItem(root, "msg")))
                    break;
                sprintf((char *)errinfo_uft8, "406登陆失败:%s",objItem->valuestring);
                break;
            }
        }
        else 
        {
            if (nChildRet = TPassLoginPasswordLoginDo(&hEtax->tpass, szUserId, login_parameter->sz_password, szBuf,sizeof(szBuf))){
                if (!(root = cJSON_Parse(szBuf)))
                    break;
                if (!(objItem = cJSON_GetObjectItem(root, "msg")))
                    break;
                sprintf((char *)errinfo_uft8, "406登陆失败:%s",objItem->valuestring);
                break;
            }            
        }*/


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
        
        strcpy(login_parameter->lpgoto,"30010666");
        
        if(!atoi(login_parameter->sys_type))  //默认税
        {   
            //https://etax.sichuan.chinatax.gov.cn/login-web/api/third/sso/login/redirect?qd=KEXIN&channelId=web&goto=30010666&code=4354EFA97EBC4FB0952E0FA9F528733F&state=test
            sprintf(login_parameter->lpcode,"https://etax.sichuan.chinatax.gov.cn/login-web/api/third/sso/login/redirect?qd=KEXIN&channelId=web&goto=30010666&code=%s&state=test",objItem->valuestring);
            //sprintf(login_parameter->lpcode,"%s",objItem->valuestring);
        }
        else                //票
        {
            if(0 > get_final_url(pValue,objItem->valuestring,final_url))
                break;
            strcpy(login_parameter->lpcode,final_url);
            /*char *p = strstr(final_url,"code=");
            p=p+6;
            strncpy(login_parameter->lpcode,p,32);
            printf("-------login_parameter->lpcode=[%s]--------\n",login_parameter->lpcode);*/
            //strcpy(login_parameter->lpcode,objItem->valuestring);
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
    pValue = NULL;

    if(hEtax->etax.user_args)
    {
        free(hEtax->etax.user_args);
        hEtax->etax.user_args = NULL;
    }

    if(hEtax->tpass.user_args)
    {
        free(hEtax->tpass.user_args);
        hEtax->tpass.user_args = NULL;
    }

    return nRet;
}

//用户信息获取
int etax_sichuan_user_query(HETAX hEtax, HDEV hDev,void *args,char *errinfo_uft8)
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
    char tpassurl[BS_HUGE] = {0};

    cJSON *jsonTpassConfig = NULL;
    cJSON *root = NULL, *objItem = NULL, *objItemChild = NULL;
    cJSON *objItemChild2 = NULL,*objItemToken  = NULL;

    hEtax->etax.user_args = NULL;
    hEtax->tpass.user_args= NULL;

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
        // if (EtaxHTTPGetEvent(&hEtax->etax, "/wszx-web/api/sh/login/mode") < 0){
        //     sprintf((char *)errinfo_uft8, "406请求跳转地址失败,HTTP Code:%d",hEtax->etax.ctxCon.nHttpRepCode);
        //     break;    
        // } 
        pValue = calloc(1,2048);
        //strcpy(pValue,"https://tpass.sichuan.chinatax.gov.cn:8443/#/login?redirect_uri=https://etax.sichuan.chinatax.gov.cn/tpass/sso/loginSubmit&client_id=d74f6Xacf6434c36acbepe4e6fpbcf8f&response_type=code&state=test");
        strcpy(pValue,"https://tpass.sichuan.chinatax.gov.cn:8443/api/v1.0/auth/oauth2/login?redirect_uri=https://etax.sichuan.chinatax.gov.cn/login-web/api/third/sso/login/redirect?qd=KEXIN&channelId=web&goto=30010666&client_id=7ede9cef518555089c0e890ae17e5ce1&response_type=code&state=test");
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
        if (TPassLoginAuthSm4AndPublickey(&hEtax->tpass,hDev->szRegCode)) {
            sprintf((char *)errinfo_uft8, "406请求公钥或者发送SM4密钥失败:%s",(const char *)evbuffer_pullup(hEtax->tpass.bufHttpRep, -1));
            break;
        }

        //获取税盘证书与税局地址进行签名验证
        // if (nChildRet = TPassLoginAuthHelloAndAuth(&hEtax->tpass, uriTpassFull)) {
        //     sprintf((char *)errinfo_uft8, "406签名验签服务器响应请求错误:%s",(const char *)evbuffer_pullup(hEtax->tpass.bufHttpRep, -1));
        //     break;
        // }
        if(hEtax->tpass.user_args)
        {
            printf("lbc-debug");
            free(hEtax->tpass.user_args);
            hEtax->tpass.user_args = NULL;
            printf("lbc-debug1111");
        }
        hEtax->tpass.user_args = (void *)((char *)calloc(1,strlen(pValue) + 1));
        if(NULL == hEtax->tpass.user_args)
            break;
        strcpy((char *)hEtax->tpass.user_args,pValue);
        //获取税盘证书与税局地址进行签名验证
        if (nChildRet = TPassLoginAuthHelloAndAuth(&hEtax->tpass, uriTpassFull)) {
            sprintf((char *)errinfo_uft8, "406签名验签服务器响应请求错误:%s",(const char *)evbuffer_pullup(hEtax->tpass.bufHttpRep, -1));
            break;
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

    if(hEtax->tpass.user_args)
    {
        free(hEtax->tpass.user_args);
        hEtax->tpass.user_args = NULL;
    }
    return nRet;   
}

//企业列表获取
int etax_sichuan_enterpriselist(HETAX hEtax, HDEV hDev,void *args,char *errinfo_uft8)
{
    struct evhttp_uri *uriTpassFull = NULL;

    ETAX_ENTERPRISELIST_PARAMETER *enterpriselist_parameter = (ETAX_ENTERPRISELIST_PARAMETER *)args;

    int nRet = ERR_GENERIC;
    int nChildRet = ERR_GENERIC;
    int nCount,i;

    char *pValue = NULL;
    char szUserId[BS_LITTLE] = {0};
    char szBuf[BS_BLOCK * 600] = {0};           //企业列表申请600K长度
    char tpassurl[BS_HUGE] = {0};

    cJSON *jsonTpassConfig = NULL;
    cJSON *root = NULL, *objItem = NULL, *objItemChild = NULL;
    cJSON *objItemChild2 = NULL,*objItemToken  = NULL;

    hEtax->etax.user_args = NULL;
    hEtax->tpass.user_args= NULL;

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
        // if (EtaxHTTPGetEvent(&hEtax->etax, "/wszx-web/api/sh/login/mode") < 0){
        //     sprintf((char *)errinfo_uft8, "406请求跳转地址失败,HTTP Code:%d",hEtax->etax.ctxCon.nHttpRepCode);
        //     break; 
        // } 
        pValue = calloc(1,BS_BLOCK*2);
        //strcpy(tpassurl,"https://tpass.sichuan.chinatax.gov.cn:8443/#/login?redirect_uri=https://etax.sichuan.chinatax.gov.cn/tpass/sso/loginSubmit&client_id=d74f6Xacf6434c36acbepe4e6fpbcf8f&response_type=code&state=test");
        strcpy(pValue,"https://tpass.sichuan.chinatax.gov.cn:8443/api/v1.0/auth/oauth2/login?redirect_uri=https://etax.sichuan.chinatax.gov.cn/login-web/api/third/sso/login/redirect?qd=KEXIN&channelId=web&goto=30010666&client_id=7ede9cef518555089c0e890ae17e5ce1&response_type=code&state=test");
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

        if(hEtax->tpass.user_args)
        {
            printf("lbc-debug");
            free(hEtax->tpass.user_args);
            hEtax->tpass.user_args = NULL;
            printf("lbc-debug1111");
        }
        hEtax->tpass.user_args = (void *)((char *)calloc(1,strlen(pValue) + 1));
        if(NULL == hEtax->tpass.user_args)
            break;
        strcpy((char *)hEtax->tpass.user_args,pValue);

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
            sprintf((char *)errinfo_uft8, "获取企业列表信息失败,HTTP Code:%d",hEtax->tpass.ctxCon.nHttpRepCode);
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

    if(hEtax->tpass.user_args)
    {
        free(hEtax->tpass.user_args);
        hEtax->tpass.user_args = NULL;
    }
    return nRet;
}

//企业切换
int etax_sichuan_relationchange(HETAX hEtax, HDEV hDev,void *args,char *errinfo_uft8)
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
    char tpassurl[BS_HUGE] = {0};
    char final_url[BS_HUGE] = {0x00};

    char *szTPassConfigJson = NULL;
    cJSON *jsonTpassConfig = NULL;
    cJSON *root = NULL, *objItem = NULL, *objItemChild = NULL;
    cJSON *objItemChild2 = NULL,*objItemToken  = NULL;

    hEtax->etax.user_args = NULL;
    hEtax->tpass.user_args= NULL;
    
    if(NULL == relationchange_parametetr)
    {
        return nRet;
    }

    if(!atoi(relationchange_parametetr->sys_type))  //默认税
    {
        hEtax->etax.user_args = NULL;
    }
    else                                  //票
    {
        hEtax->etax.user_args = (void *)(&header_array_sichuan);
        memset(hEtax->etax.modelTaAddr.szTaxAuthorityURL,0x00,sizeof(hEtax->etax.modelTaAddr.szTaxAuthorityURL));
        strcpy(hEtax->etax.modelTaAddr.szTaxAuthorityURL,"https://dppt.sichuan.chinatax.gov.cn:8443");
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

        if(!atoi(relationchange_parametetr->sys_type)) 
        {
            //获取tpass地址
            // if (EtaxHTTPGetEvent(&hEtax->etax, "/wszx-web/api/sh/login/mode") < 0){
            //     sprintf((char *)errinfo_uft8, "406请求跳转地址失败,HTTP Code:%d",hEtax->etax.ctxCon.nHttpRepCode);
            //     break;   
            // }

            // szTPassConfigJson = (char *)evbuffer_pullup(hEtax->etax.bufHttpRep, -1);
            pValue = calloc(1,BS_BLOCK*2);  
            //strcpy(tpassurl,"https://tpass.sichuan.chinatax.gov.cn:8443/#/login?redirect_uri=https://etax.sichuan.chinatax.gov.cn/tpass/sso/loginSubmit&client_id=d74f6Xacf6434c36acbepe4e6fpbcf8f&response_type=code&state=test");
            strcpy(pValue,"https://tpass.sichuan.chinatax.gov.cn:8443/api/v1.0/auth/oauth2/login?redirect_uri=https://etax.sichuan.chinatax.gov.cn/login-web/api/third/sso/login/redirect?qd=KEXIN&channelId=web&goto=30010666&client_id=7ede9cef518555089c0e890ae17e5ce1&response_type=code&state=test");     
            if (!(uriTpassFull = evhttp_uri_parse(pValue))){
                sprintf((char *)errinfo_uft8, "406获取跳转地址错误,地址转换失败"); 
                break;
            }

            if (!FillUploadAddrModelDynamic(TAADDR_ETAX_TPASS, hDev->szRegCode, pValue, "/",&hEtax->tpass.modelTaAddr)) {
                sprintf((char *)errinfo_uft8, "406获取跳转地址错误,地址解析失败");
                break;
            } 

            printf("szTPassConfigJson=%s\r\n",szTPassConfigJson);
            printf("tpassurl=%s\r\n",tpassurl);
        }
        else
        {
            szTPassConfigJson = (char *)hEtax->etax.user_args;
            //获取跳转tpass地址
            if(RET_SUCCESS != get_tpass_addr_sichuan(szTPassConfigJson,&pValue,relationchange_parametetr->sys_type))
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

            printf("szTPassConfigJson=%s\r\n",szTPassConfigJson);
            printf("tpassurl=%s\r\n",pValue);
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
        
        if(hEtax->tpass.user_args)
        {
            printf("lbc-debug");
            free(hEtax->tpass.user_args);
            hEtax->tpass.user_args = NULL;
            printf("lbc-debug1111");
        }
        hEtax->tpass.user_args = (void *)((char *)calloc(1,strlen(pValue) + 1));
        if(NULL == hEtax->tpass.user_args)
            break;
        strcpy((char *)hEtax->tpass.user_args,pValue);
        //获取税盘证书与税局地址进行签名验证
        if ((nChildRet = TPassLoginAuthHelloAndAuth(&hEtax->tpass, uriTpassFull))) {
            sprintf((char *)errinfo_uft8, "406签名验签服务器响应请求错误:%s",(const char *)evbuffer_pullup(hEtax->tpass.bufHttpRep, -1));
            break;
        }
        //获取税盘证书与税局地址进行签名验证
        // if(!atoi(relationchange_parametetr->sys_type)){
        //     if ((nChildRet = TPassLoginAuthHelloAndAuth(&hEtax->tpass, pValue))) {
        //         sprintf((char *)errinfo_uft8, "406签名验签服务器响应请求错误:%s",(const char *)evbuffer_pullup(hEtax->tpass.bufHttpRep, -1));
        //         break;
        //     }
        // }
        // else 
        // {
        //     if ((nChildRet = TPassLoginAuthHelloAndAuth(&hEtax->tpass, uriTpassFull))) {
        //         sprintf((char *)errinfo_uft8, "406签名验签服务器响应请求错误:%s",(const char *)evbuffer_pullup(hEtax->tpass.bufHttpRep, -1));
        //         break;
        //     }
        // }    

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
        /*if(!atoi(relationchange_parametetr->sys_type)){
            if (nChildRet = TPassLoginPasswordLoginDosichuan(&hEtax->tpass, szUserId, relationchange_parametetr->sz_password, szBuf,sizeof(szBuf))){
                if (!(root = cJSON_Parse(szBuf)))
                    break;
                if (!(objItem = cJSON_GetObjectItem(root, "msg")))
                    break;
                sprintf((char *)errinfo_uft8, "406登陆失败:%s",objItem->valuestring);
                break;
            }
        }
        else 
        {
            if (nChildRet = TPassLoginPasswordLoginDo(&hEtax->tpass, szUserId, relationchange_parametetr->sz_password, szBuf,sizeof(szBuf))){
                if (!(root = cJSON_Parse(szBuf)))
                    break;
                if (!(objItem = cJSON_GetObjectItem(root, "msg")))
                    break;
                sprintf((char *)errinfo_uft8, "406登陆失败:%s",objItem->valuestring);
                break;
            }            
        }*/

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
        
        strcpy(relationchange_parametetr->lpgoto,"30010666");

        if(!atoi(relationchange_parametetr->sys_type))  //默认税
        {
            sprintf(relationchange_parametetr->lpcode,"https://etax.sichuan.chinatax.gov.cn/login-web/api/third/sso/login/redirect?qd=KEXIN&channelId=web&goto=30010666&code=%s&state=test",objItem->valuestring);
            //sprintf(relationchange_parametetr->lpcode,"https://etax.sichuan.chinatax.gov.cn/tpass/sso/loginSubmit?code=%s&state=test",objItem->valuestring);
        }
        else                //票
        {
            if(0 > get_final_url(pValue,objItem->valuestring,final_url))
                break;
            strcpy(relationchange_parametetr->lpcode,final_url); 
        }


        if (!(objItemToken = cJSON_GetObjectItem(root, "access_token"))){
            sprintf((char *)errinfo_uft8, "406登陆失败,返回数据中无token参数");
            break;
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
    pValue = NULL;   

    if(hEtax->etax.user_args)
    {
        free(hEtax->etax.user_args);
        hEtax->etax.user_args = NULL;
    }

    if(hEtax->tpass.user_args)
    {
        free(hEtax->tpass.user_args);
        hEtax->tpass.user_args = NULL;
    }

    return nRet;    
}