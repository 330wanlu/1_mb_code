/*****************************************************************************
File name:   etax_call.c
Description: 用于福建调用etax接口
Author:      Wang
Version:     1.0
Date:        2023.02
History:
*****************************************************************************/
#include "shandong_call.h"
#include "../common/console.h"

int etax_shandong_login(HETAX hEtax, HDEV hDev,void *args,char *errinfo_uft8);
int etax_shandong_user_query(HETAX hEtax, HDEV hDev,void *args,char *errinfo_uft8);
int etax_shandong_enterpriselist(HETAX hEtax, HDEV hDev,void *args,char *errinfo_uft8);
int etax_shandong_relationchange(HETAX hEtax, HDEV hDev,void *args,char *errinfo_uft8);

ETAX_FUCTION etax_shandong_fuction = 
{
    .etax_login = etax_shandong_login,
    .etax_user_query = etax_shandong_user_query,
    .etax_enterpriselist = etax_shandong_enterpriselist,
    .etax_relationchange = etax_shandong_relationchange
};

//自定义header
HEADER_STRUCT header_array_shandong[] = {
    {"Cookie", "oauth2_referer=szzhzz.shandong.chinatax.gov.cn; x_host_key=188715edd92-73398ffbd187ed688673e9dcd9a219287ac81e5c"},
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


//获取tpass地址
int get_tpass_addr_shandong(char *src_data,char **tpass_addr,char *sys_type)
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

//一键登陆
int etax_shandong_login(HETAX hEtax, HDEV hDev,void *args,char *errinfo_uft8)
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

    hEtax->etax.user_args = NULL;
    hEtax->tpass.user_args= NULL;

    if(NULL == login_parameter)
        return nRet;
    hEtax->etax.modelTaAddr.bServType = TAADDR_ETAX;
    if(!atoi(login_parameter->sys_type))  //默认税
    {
        hEtax->etax.user_args = NULL;
    }
    else                 //票
    {
        hEtax->etax.user_args = (void *)(&header_array_shandong);
        memset(hEtax->etax.modelTaAddr.szTaxAuthorityURL,0x00,sizeof(hEtax->etax.modelTaAddr.szTaxAuthorityURL));
        strcpy(hEtax->etax.modelTaAddr.szTaxAuthorityURL,"https://dppt.shandong.chinatax.gov.cn:8443");
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
        
        /*//获取tpass地址
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
        printf("szTPassConfigJson=%s\r\n",szTPassConfigJson);*/
        
        if(!atoi(login_parameter->sys_type)) 
        {
            pValue = calloc(1,BS_BLOCK*2);  //b62deMdW365e4dfba3W63bab7dW77be4
            //https://tpass.sichuan.chinatax.gov.cn:8443/api/v1.0/auth/oauth2/login?redirect_uri=https://etax.sichuan.chinatax.gov.cn/login-web/api/third/sso/login/redirect?qd=KEXIN&channelId=web&goto=30010666&client_id=7ede9cef518555089c0e890ae17e5ce1&response_type=code&state=test
            strcpy(pValue,"https://tpass.shandong.chinatax.gov.cn:8443/api/v1.0/auth/oauth2/login?redirect_uri=https://etax.shandong.chinatax.gov.cn/KxsfrzAction.do?activity=login&client_id=s4ce78569sbs47w3b8bw38D56a3D5cc2&response_type=code&state=test");
            //strcpy(pValue,"");
            printf("lbc-debug pValue = %s\n",pValue);

            if (!(uriTpassFull = evhttp_uri_parse(pValue))){
                sprintf((char *)errinfo_uft8, "406获取跳转地址错误,地址转换失败"); 
                break;
            }

            if (!FillUploadAddrModelDynamic(TAADDR_ETAX_TPASS, hDev->szRegCode, pValue, "/",&hEtax->tpass.modelTaAddr)) {
                sprintf((char *)errinfo_uft8, "406获取跳转地址错误,地址解析失败");
                break;
            } 
        }
        
        else{
            szTPassConfigJson = (char *)hEtax->etax.user_args;
            //获取跳转tpass地址
            if(RET_SUCCESS != get_tpass_addr_shandong(szTPassConfigJson,&pValue,login_parameter->sys_type))
            {
                sprintf((char *)errinfo_uft8, "406获取跳转地址错误,地址解析失败");
                break;
            }

            printf("pValue=%s\r\n",pValue);
        
            if (!(uriTpassFull = evhttp_uri_parse(pValue))){
                sprintf((char *)errinfo_uft8, "406获取跳转地址错误,地址解析失败");
                break;
            }

            if (!FillUploadAddrModelDynamic(TAADDR_ETAX_TPASS, hDev->szRegCode, pValue, "/",&hEtax->tpass.modelTaAddr)) {
                sprintf((char *)errinfo_uft8, "406请求跳转地址失败,HTTP Code:%d",hEtax->etax.ctxCon.nHttpRepCode);
                break;  
            } 
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

        printf("             login_method           login_method            =   [%s]                       \r\n",login_parameter->login_method);
        if (memcmp(login_parameter->login_method,"21",2) == 0)// "121.40.230.253",18001    login_parameter->soft_cert_ip,login_parameter->soft_cert_port
        {
            if ((nChildRet = TPassLoginAuthHelloAndAuth_soft_cert_21_Msg(&hEtax->tpass, uriTpassFull,login_parameter->svr_container,login_parameter->soft_cert_ip,login_parameter->soft_cert_port,errinfo_uft8_21)))
            {
                sprintf((char *)errinfo_uft8, "%s",errinfo_uft8_21);
                printf("lbc-debug nChildRet=%d        nRet = %d     \n",nChildRet,nRet);
                break;
            }
        }       
        else if(memcmp(login_parameter->login_method,"02",2) == 0)  //数字证书
        // {
        //     //获取税盘证书与税局地址进行签名验证
        //     if ((nChildRet = TPassLoginAuthHelloAndAuth(&hEtax->tpass, uriTpassFull))) {
        //         sprintf((char *)errinfo_uft8, "签名验签服务器响应请求错误,可能为税局服务器原因，请插盘尝试");
        //         printf("lbc-debug nChildRet=%d\n",nChildRet);
        //         break;
        //     }
        // }
        {
            int i=0;
            // //获取税盘证书与税局地址进行签名验证
            // if ((nChildRet = TPassLoginAuthHelloAndAuth(&hEtax->tpass, uriTpassFull))) {
            //     sprintf((char *)errinfo_uft8, "签名验签服务器响应请求错误,可能为税局服务器原因，请插盘尝试");
            //     printf("lbc-debug nChildRet=%d\n",nChildRet);
            //     break;
            // }
        NETX_CA:
            if ((nChildRet = TPassLoginAuthHelloAndAuth(&hEtax->tpass, uriTpassFull))) {
                // sprintf((char *)errinfo_uft8, "签名验签服务器响应请求错误,可能为税局服务器原因，请插盘尝试");
                // printf("lbc-debug nChildRet=%d\n",nChildRet);
                i++;
                printf("_______________i = %d\r\n",i);
                if (i>5)
                {
                    sprintf((char *)errinfo_uft8, "签名验签服务器响应请求错误,可能为税局服务器原因，请插盘尝试");
                    printf("lbc-debug nChildRet=%d\n",nChildRet);
                    break;
                }
                
                goto NETX_CA;
                //break;
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
        printf("TpassPasswordLoginDo objItem->valuestring=%s\r\n",(const char *)evbuffer_pullup(hEtax->tpass.bufHttpRep, -1));
        nCount = cJSON_GetArraySize(objItem);
        printf("*********nCount=[%d]********\n",nCount);
        for (i = 0; i < nCount; i++) {
            objItemChild = cJSON_GetArrayItem(objItem, i);

            if(strlen(login_parameter->idcard)){
                if (!(objItemChild2 = cJSON_GetObjectItem(objItemChild, "id_card"))){
                    printf("----");
                    continue;}
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
        /*for (i = 0; i < nCount; i++) {  // 测试
            objItemChild = cJSON_GetArrayItem(objItem, i);
            if(strlen(login_parameter->idcard)){
                if (!(objItemChild2 = cJSON_GetObjectItem(objItemChild, "id_card")))
                    continue;
                if (strcmp(objItemChild2->valuestring, login_parameter->idcard)){
                    printf("sssssssssssss1sssssssssss  objItemChild2->valuestring = %s  login_parameter->idcard = %s\r\n",objItemChild2->valuestring,login_parameter->idcard);
                    continue;}
            }

            if(strlen(login_parameter->mobile)){
                if (!(objItemChild2 = cJSON_GetObjectItem(objItemChild, "mobile")))
                    continue;
                if (strcmp(objItemChild2->valuestring, login_parameter->mobile)){
                    printf("ssssssssssss2ssssssssss   objItemChild2->valuestring = %s  login_parameter->mobile = %s\r\n",objItemChild2->valuestring,login_parameter->mobile);
                    continue;}
            }

            if (!(objItemChild2 = cJSON_GetObjectItem(objItemChild, "relatedType")))
                continue;
            if (strcmp(objItemChild2->valuestring, login_parameter->related_type)){
                printf("sssssssssssss3sssssssss  objItemChild2->valuestring = %s  login_parameter->related_type = %s\r\n",objItemChild2->valuestring, login_parameter->related_type);
            //if (strcmp(objItemChild2->valuestring, "03"))
                continue;}
                
            if (!(objItemChild2 = cJSON_GetObjectItem(objItemChild, "user_id"))){
                //printf("ssssssssssssssssssssss  login_parameter->user_id = %s\r\n",login_parameter->user_id);
                continue;}
            strcpy(szUserId, objItemChild2->valuestring);
            
            printf("sssssss  ssssss  szUserId=%s,sz_password=%s\r\n",szUserId,login_parameter->sz_password);
            break;
        } */


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

        printf("TPassLoginPasswordLoginDo Code = %s\r\n",objItem->valuestring);
        if(!atoi(login_parameter->sys_type))  //默认税
        {
            //https://etax.sichuan.chinatax.gov.cn/login-web/api/third/sso/login/redirect?qd=KEXIN&channelId=web&goto=30010666&code=%s&state=test
            //sprintf(login_parameter->lpcode,"https://etax.shandong.chinatax.gov.cn/kxsfrz-cjpt-web/tpass/tpassLogin.do?client_id=18169169c19c11edb15240a6b75aad54&code=%s&service=http://etax.shandong.chinatax.gov.cn/xxmh/html/index_login.html&state=test",objItem->valuestring);
            sprintf(login_parameter->lpcode,"https://etax.shandong.chinatax.gov.cn/KxsfrzAction.do?activity=login&client_id=s4ce78569sbs47w3b8bw38D56a3D5cc2&code=%s&state=test",objItem->valuestring);

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
int etax_shandong_user_query(HETAX hEtax, HDEV hDev,void *args,char *errinfo_uft8)
{
    struct evhttp_uri *uriTpassFull = NULL;
    ETAX_USER_QUERY_PARAMETER *user_query_parameter = (ETAX_USER_QUERY_PARAMETER *)args;

    int nRet = ERR_GENERIC;
    int nChildRet = ERR_GENERIC;

    char *nEtaxBufData = NULL;
    char *pValue = NULL;
    char *szTPassConfigJson = NULL;

    cJSON *jsonTpassConfig = NULL;
    cJSON *root = NULL, *objItem = NULL;

    hEtax->etax.user_args = NULL;
    if(NULL == user_query_parameter)
    {
        return nRet;
    }

    hEtax->etax.user_args = (void *)(&header_array_shandong);
    memset(hEtax->etax.modelTaAddr.szTaxAuthorityURL,0x00,sizeof(hEtax->etax.modelTaAddr.szTaxAuthorityURL));
    strcpy(hEtax->etax.modelTaAddr.szTaxAuthorityURL,"https://dppt.shandong.chinatax.gov.cn:8443");
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
        //步骤3 从token服务器获取登陆凭证去登陆Etax
        if (hEtax->etax.ctxCon.nHttpRepCode != 200 && hEtax->etax.ctxCon.nHttpRepCode != 302){
            sprintf((char *)errinfo_uft8, "406请求跳转地址失败,HTTP Code:%d",hEtax->etax.ctxCon.nHttpRepCode);  
            break;
        }
        //char *szTPassConfigJson = (char *)evbuffer_pullup(hEtax->etax.bufHttpRep, -1);
       
        if(hEtax->etax.ctxCon.nHttpRepCode == 200)
        {
            szTPassConfigJson = (char *)evbuffer_pullup(hEtax->etax.bufHttpRep, -1);
        }
        else if(hEtax->etax.ctxCon.nHttpRepCode == 302)
        {
            szTPassConfigJson = (char *)hEtax->etax.user_args;
        }
        //获取跳转tpass地址
        //获取跳转tpass地址
        if(RET_SUCCESS != get_tpass_addr_shandong(szTPassConfigJson,&pValue,"1"))
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
        if (memcmp(user_query_parameter->login_method,"21",2) == 0)//user_query_parameter->soft_cert_ip,user_query_parameter->soft_cert_port   121.40.230.253:18001
        {
            if ((nChildRet = TPassLoginAuthHelloAndAuth_soft_cert_21(&hEtax->tpass, uriTpassFull,user_query_parameter->svr_container,user_query_parameter->soft_cert_ip,user_query_parameter->soft_cert_port)))
            {
                sprintf((char *)errinfo_uft8, "102税盘已离线或是登陆参数有误");
                printf("lbc-debug nChildRet=%d        nRet = %d     \n",nChildRet,nRet);
                break;
            }
        }       
        else if(memcmp(user_query_parameter->login_method,"02",2) == 0)  //数字证书
        // {
        //     //获取税盘证书与税局地址进行签名验证
        //     if ((nChildRet = TPassLoginAuthHelloAndAuth(&hEtax->tpass, uriTpassFull))) {
        //         sprintf((char *)errinfo_uft8, "签名验签服务器响应请求错误,可能为税局服务器原因，请插盘尝试");
        //         printf("lbc-debug nChildRet=%d\n",nChildRet);
        //         break;
        //     }
        // }
        {
            int i=0;
            // //获取税盘证书与税局地址进行签名验证
            // if ((nChildRet = TPassLoginAuthHelloAndAuth(&hEtax->tpass, uriTpassFull))) {
            //     sprintf((char *)errinfo_uft8, "签名验签服务器响应请求错误,可能为税局服务器原因，请插盘尝试");
            //     printf("lbc-debug nChildRet=%d\n",nChildRet);
            //     break;
            // }
        NETX_CA:
            if ((nChildRet = TPassLoginAuthHelloAndAuth(&hEtax->tpass, uriTpassFull))) {
                // sprintf((char *)errinfo_uft8, "签名验签服务器响应请求错误,可能为税局服务器原因，请插盘尝试");
                // printf("lbc-debug nChildRet=%d\n",nChildRet);
                i++;
                printf("_______________i = %d\r\n",i);
                if (i>5)
                {
                    sprintf((char *)errinfo_uft8, "签名验签服务器响应请求错误,可能为税局服务器原因，请插盘尝试");
                    printf("lbc-debug nChildRet=%d\n",nChildRet);
                    break;
                }
                
                goto NETX_CA;
                //break;
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
            printf("lbc-debug nChildRet = %d,objItem->valuestring=%s\n nEtaxBufData = %s\n",nChildRet,objItem->valuestring,nEtaxBufData);
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
int etax_shandong_enterpriselist(HETAX hEtax, HDEV hDev,void *args,char *errinfo_uft8)
{
    struct evhttp_uri *uriTpassFull = NULL;
    ETAX_ENTERPRISELIST_PARAMETER *enterpriselist_parameter = (ETAX_ENTERPRISELIST_PARAMETER *)args;

    int nRet = ERR_GENERIC;
    int nChildRet = ERR_GENERIC;
    int nCount,i;

    char *pValue = NULL;
    char *szTPassConfigJson = NULL;
    char szUserId[BS_LITTLE] = {0};
    char szBuf[BS_BLOCK * 1000] = {0};

    cJSON *jsonTpassConfig = NULL;
    cJSON *root = NULL, *objItem = NULL, *objItemChild = NULL;
    cJSON *objItemChild2 = NULL,*objItemToken  = NULL;

    hEtax->etax.user_args = NULL;

    if(NULL == enterpriselist_parameter)
    {
        return nRet;
    }

    hEtax->etax.user_args = (void *)(&header_array_shandong);
    memset(hEtax->etax.modelTaAddr.szTaxAuthorityURL,0x00,sizeof(hEtax->etax.modelTaAddr.szTaxAuthorityURL));
    strcpy(hEtax->etax.modelTaAddr.szTaxAuthorityURL,"https://dppt.shandong.chinatax.gov.cn:8443");
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
        
        //获取tpass地
        //步骤3 从token服务器获取登陆凭证去登陆Etax,登陆完成后返回的用户列表在hiTPass->bufHttpRep中
        if (hEtax->etax.ctxCon.nHttpRepCode != 200 && hEtax->etax.ctxCon.nHttpRepCode != 302){
            sprintf((char *)errinfo_uft8, "406请求跳转地址失败,HTTP Code:%d",hEtax->etax.ctxCon.nHttpRepCode);
            break;
        }
        //char *szTPassConfigJson = (char *)evbuffer_pullup(hEtax->etax.bufHttpRep, -1);
       
        if(hEtax->etax.ctxCon.nHttpRepCode == 200)
        {
            szTPassConfigJson = (char *)evbuffer_pullup(hEtax->etax.bufHttpRep, -1);
        }
        else if(hEtax->etax.ctxCon.nHttpRepCode == 302)
        {
            szTPassConfigJson = (char *)hEtax->etax.user_args;
        }

        //获取跳转tpass地址
        if(RET_SUCCESS != get_tpass_addr_shandong(szTPassConfigJson,&pValue,"1"))
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
        if (memcmp(enterpriselist_parameter->login_method,"21",2) == 0)//"121.40.230.253",18001  enterpriselist_parameter->soft_cert_ip,enterpriselist_parameter->soft_cert_port
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
            int i=0;
            // //获取税盘证书与税局地址进行签名验证
            // if ((nChildRet = TPassLoginAuthHelloAndAuth(&hEtax->tpass, uriTpassFull))) {
            //     sprintf((char *)errinfo_uft8, "签名验签服务器响应请求错误,可能为税局服务器原因，请插盘尝试");
            //     printf("lbc-debug nChildRet=%d\n",nChildRet);
            //     break;
            // }
        NETX_CA:
            if ((nChildRet = TPassLoginAuthHelloAndAuth(&hEtax->tpass, uriTpassFull))) {
                // sprintf((char *)errinfo_uft8, "签名验签服务器响应请求错误,可能为税局服务器原因，请插盘尝试");
                // printf("lbc-debug nChildRet=%d\n",nChildRet);
                i++;
                printf("_______________i = %d\r\n",i);
                if (i>5)
                {
                    sprintf((char *)errinfo_uft8, "签名验签服务器响应请求错误,可能为税局服务器原因，请插盘尝试");
                    printf("lbc-debug nChildRet=%d\n",nChildRet);
                    break;
                }
                
                goto NETX_CA;
                //break;
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
int etax_shandong_relationchange(HETAX hEtax, HDEV hDev,void *args,char *errinfo_uft8)
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
    char errinfo_uft8_21[128] = {0};

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
        hEtax->etax.user_args = (void *)(&header_array_shandong);
        memset(hEtax->etax.modelTaAddr.szTaxAuthorityURL,0x00,sizeof(hEtax->etax.modelTaAddr.szTaxAuthorityURL));
        strcpy(hEtax->etax.modelTaAddr.szTaxAuthorityURL,"https://dppt.shandong.chinatax.gov.cn:8443");
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
        /*//获取tpass地址
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
        }*/
        /*//获取跳转tpass地址
        //if(RET_SUCCESS != get_tpass_addr_shandong(szTPassConfigJson,&pValue,relationchange_parametetr->sys_type))
        if(RET_SUCCESS != get_tpass_addr_shandong(szTPassConfigJson,&pValue,"1"))
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
        } */

        if(!atoi(relationchange_parametetr->sys_type)) 
        {
            pValue = calloc(1,BS_BLOCK*2);  //b62deMdW365e4dfba3W63bab7dW77be4
            strcpy(pValue,"https://tpass.shandong.chinatax.gov.cn:8443/api/v1.0/auth/oauth2/login?redirect_uri=https://etax.shandong.chinatax.gov.cn/KxsfrzAction.do?activity=login&client_id=s4ce78569sbs47w3b8bw38D56a3D5cc2&response_type=code&state=test");
            printf("lbc-debug pValue = %s\n",pValue);

            if (!(uriTpassFull = evhttp_uri_parse(pValue))){
                sprintf((char *)errinfo_uft8, "406获取跳转地址错误,地址转换失败"); 
                break;
            }

            if (!FillUploadAddrModelDynamic(TAADDR_ETAX_TPASS, hDev->szRegCode, pValue, "/",&hEtax->tpass.modelTaAddr)) {
                sprintf((char *)errinfo_uft8, "406获取跳转地址错误,地址解析失败");
                break;
            } 
        }        
        else{
            szTPassConfigJson = (char *)hEtax->etax.user_args;
            //获取跳转tpass地址
            if(RET_SUCCESS != get_tpass_addr_shandong(szTPassConfigJson,&pValue,relationchange_parametetr->sys_type))
            {
                sprintf((char *)errinfo_uft8, "406获取跳转地址错误,地址解析失败");
                break;
            }

            printf("pValue=%s\r\n",pValue);
        
            if (!(uriTpassFull = evhttp_uri_parse(pValue))){
                sprintf((char *)errinfo_uft8, "406获取跳转地址错误,地址解析失败");
                break;
            }

            if (!FillUploadAddrModelDynamic(TAADDR_ETAX_TPASS, hDev->szRegCode, pValue, "/",&hEtax->tpass.modelTaAddr)) {
                sprintf((char *)errinfo_uft8, "406请求跳转地址失败,HTTP Code:%d",hEtax->etax.ctxCon.nHttpRepCode);
                break;  
            } 
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
        if (memcmp(relationchange_parametetr->login_method,"21",2) == 0)//"121.40.230.253",18001  relationchange_parametetr->soft_cert_ip,relationchange_parametetr->soft_cert_port
        {
            if ((nChildRet = TPassLoginAuthHelloAndAuth_soft_cert_21_Msg(&hEtax->tpass, uriTpassFull,relationchange_parametetr->svr_container,relationchange_parametetr->soft_cert_ip,relationchange_parametetr->soft_cert_port,errinfo_uft8_21)))
            {
                sprintf((char *)errinfo_uft8, "%s",errinfo_uft8_21);
                printf("lbc-debug nChildRet=%d        nRet = %d     \n",nChildRet,nRet);
                break;
            }
        }       
        else if(memcmp(relationchange_parametetr->login_method,"02",2) == 0)  //数字证书
        // {
        //     //获取税盘证书与税局地址进行签名验证
        //     if ((nChildRet = TPassLoginAuthHelloAndAuth(&hEtax->tpass, uriTpassFull))) {
        //         sprintf((char *)errinfo_uft8, "签名验签服务器响应请求错误,可能为税局服务器原因，请插盘尝试");
        //         printf("lbc-debug nChildRet=%d\n",nChildRet);
        //         break;
        //     }
        // }
        {
            int i=0;
            // //获取税盘证书与税局地址进行签名验证
            // if ((nChildRet = TPassLoginAuthHelloAndAuth(&hEtax->tpass, uriTpassFull))) {
            //     sprintf((char *)errinfo_uft8, "签名验签服务器响应请求错误,可能为税局服务器原因，请插盘尝试");
            //     printf("lbc-debug nChildRet=%d\n",nChildRet);
            //     break;
            // }
        NETX_CA:
            if ((nChildRet = TPassLoginAuthHelloAndAuth(&hEtax->tpass, uriTpassFull))) {
                // sprintf((char *)errinfo_uft8, "签名验签服务器响应请求错误,可能为税局服务器原因，请插盘尝试");
                // printf("lbc-debug nChildRet=%d\n",nChildRet);
                i++;
                printf("_______________i = %d\r\n",i);
                if (i>5)
                {
                    sprintf((char *)errinfo_uft8, "签名验签服务器响应请求错误,可能为税局服务器原因，请插盘尝试");
                    printf("lbc-debug nChildRet=%d\n",nChildRet);
                    break;
                }
                
                goto NETX_CA;
                //break;
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
        printf("TpassPasswordLoginDo objItem->valuestring=%s\r\n",(const char *)evbuffer_pullup(hEtax->tpass.bufHttpRep, -1));
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

        printf("TPassLoginPasswordLoginDo Code = %s\r\n",objItem->valuestring);
        if(!atoi(relationchange_parametetr->sys_type))  //默认税
        {
            //strcpy(relationchange_parametetr->lpcode, objItem->valuestring); 
            sprintf(relationchange_parametetr->lpcode,"https://etax.shandong.chinatax.gov.cn/KxsfrzAction.do?activity=login&client_id=s4ce78569sbs47w3b8bw38D56a3D5cc2&code=%s&state=test",objItem->valuestring);
        }
        else                //票
        {
            if(0 > get_final_url(pValue,objItem->valuestring,final_url))
                break;
            strcpy(relationchange_parametetr->lpcode,final_url); 
        }


        // strcpy(lpcode, objItem->valuestring); 

        if (!(objItemToken = cJSON_GetObjectItem(root, "access_token"))){
            sprintf((char *)errinfo_uft8, "406企业切换失败,返回数据中无token参数");
            break;
        }

        strcpy(relationchange_parametetr->lptoken, objItemToken->valuestring); 
        printf("TPassSelectRelationchange Token = %s\r\n",objItemToken->valuestring);       
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


