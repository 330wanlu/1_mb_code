/*****************************************************************************
File name:   etax_bridge.c
Description: 用于模拟上海税务局本地客户端登陆访问功能，并将网页转换为API接口
Author:      Zako
Version:     1.0
Date:        2023.01
History:
20230202     最初代码实现
*****************************************************************************/
#include "etax_confirm.h"

#define MAX(x, y)                                                                                  \
    ({                                                                                             \
        typeof(x) _x = (x);                                                                        \
        typeof(y) _y = (y);                                                                        \
        _x > _y ? _x : _y;                                                                         \
    })

#define MIN(x, y)                                                                                  \
    ({                                                                                             \
        typeof(x) _x = (x);                                                                        \
        typeof(y) _y = (y);                                                                        \
        _x < _y ? _x : _y;                                                                         \
    })


void CloseEtaxConnections(HETAX hEtax)
{
    EtaxHTTPClose(&hEtax->dppt);
    EtaxHTTPClose(&hEtax->tpass);
    EtaxHTTPClose(&hEtax->etax);
}
void CloseEtaxConnectionsNoEtax(HETAX hEtax)
{
    EtaxHTTPClose(&hEtax->dppt);
    EtaxHTTPClose(&hEtax->tpass);
}

void CloseEtaxConnectionsNoTpass(HETAX hEtax)
{
    EtaxHTTPClose(&hEtax->dppt);
    EtaxHTTPClose(&hEtax->etax);
}

uint8 GetFirstIntFromSsoToken(char *szSsoToken)
{
    int i = 0, nRet = 0;
    uint8 b = 0;
    char szBuf[BS_8];
    for (i = 0; i < strlen(szSsoToken); i++) {
        b = szSsoToken[i];
        if (isdigit(b)) {
            strncpy(szBuf, szSsoToken + i, 1);
            sscanf(szBuf, "%d", &nRet);
            break;
        }
    }
    return nRet;
}

void ReverseStringByNum(char *szStr, int nNum)
{
    int nIndex = 0, nStrLen = strlen(szStr), nDoNum = nNum;
    if (nNum >= nStrLen)
        return;
    do {
        ByteReverse(szStr + nIndex, nDoNum);

        nIndex += nDoNum;
        nDoNum = MIN(nNum, nStrLen - nIndex);
    } while (nDoNum);
}

//获取tpass地址
int get_tpass_addr(char *src_data,char **tpass_addr,char *sys_type)
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

int GetRandUrlyzmPath(char *szOrigUrl, char *szCookieSsoToken, char *szRepTokenKey,
                      char *szOutString, int nOutBufSize)
{
    // 在assets_res/js/chunk-vendors.63bcfeb2.js文件中，搜索如下相关字符串为urljym生成函数
    // var y = ""[u(4144)](l, "+")["concat"](p), x = i[u(4350)](me, y);
    // return s + ""["concat"](s[u(2387)]("?") ? "&" : "?",
    // i[u(5107)])[u(4144)](encodeURIComponent(x))
    //
    // 54098ed1b9967923ac59+/szzhzz/swszzhCtr/v1/getYwqxbz?ruuid=1679280540989&Lydz=/digital-tax-account
    int nRet = ERR_GENERIC;
    char szBuf[BS_NORMAL] = "", szBuf2[BS_NORMAL] = "", szBuf3[BS_NORMAL] = "", *pNewBuf = NULL;
    char szRuuid[BS_TINY] = "";  //不可修改
    uint8 bRange = 0;
    // _WriteLog(LL_DEBUG, "CookieSsoToken:%s RepTokenKey:%s", szCookieSsoToken, szRepTokenKey);
    do {
        if (strstr(szOrigUrl, "&urlyzm="))
            break;
        if (!(pNewBuf = calloc(1, strlen(szOrigUrl) + 128)))
            break;
        sprintf(szRuuid, "%llu", GetMicroSecondUnixTimestamp());
        //获取cookies中dzfp-token，并逻辑分段翻转
        bRange = GetFirstIntFromSsoToken(szCookieSsoToken) + 3;  //值+3
        strcpy(szBuf2, szCookieSsoToken);
        strcpy(szBuf, szRuuid);
        ByteReverse(szBuf, strlen(szBuf));
        ByteReverse(szBuf2, strlen(szBuf2));
        strcat(szBuf, szBuf2);
        ReverseStringByNum(szBuf, bRange);
        if (0 == bRange % 2) {
            //(0, 5),(15, 20) (5, 10),(20, 25)
            memset(szBuf2, 0, sizeof(szBuf2));
            strncpy(szBuf2, szBuf + 0, 5);
            strncpy(szBuf2 + 5, szBuf + 15, 5);
            strncpy(szBuf2 + 10, szBuf + 5, 5);
            strncpy(szBuf2 + 15, szBuf + 20, 5);
        } else {
            //逻辑分段翻转szRepTokenKey=p,szCookieSsoToken翻转后为f
            strcpy(szBuf3, szRepTokenKey);
            ReverseStringByNum(szBuf3, bRange);
            ByteReverse(szBuf3, strlen(szBuf3));
            // f(0, 5), p(5, 10)), f(15, 20)), p(20, 25)
            memset(szBuf2, 0, sizeof(szBuf2));
            strncpy(szBuf2, szBuf + 0, 5);
            strncpy(szBuf2 + 5, szBuf3 + 5, 5);
            strncpy(szBuf2 + 10, szBuf + 15, 5);
            strncpy(szBuf2 + 15, szBuf3 + 20, 5);
        }
        //计算生成字符串md5
        sprintf(pNewBuf, "%s+%s?ruuid=%s", szBuf2, szOrigUrl, szRuuid);
        CalcMD5(pNewBuf, strlen(pNewBuf), szBuf2);
        memset(szBuf, 0, sizeof(szBuf));
        Byte2Str(szBuf, (uint8 *)szBuf2, 0x10);
        sprintf(pNewBuf, "%s?ruuid=%s&urlyzm=%s", szOrigUrl, szRuuid, szBuf);
        if (strlen(pNewBuf) > nOutBufSize - 1)
            break;
        strcpy(szOutString, pNewBuf);
        nRet = RET_SUCCESS;
    } while (false);
    if (pNewBuf)
        free(pNewBuf);
    return nRet;
}

//注意:szPostAndOut既当输入又当输出，因此分配和预估时，大概分配2-3倍左右
int BuildBridgePostJmbw(char *szOrigUrl, char *szSsoToken, char *szPost, char *szOut,
                        int nOutBufSize)
{
    char szRandom32ByteString[BS_LITTLE], szCookieSsoToken[BS_LITTLE],
        *pNewBuf = NULL, szBuf[BS_NORMAL], szBuf2[BS_NORMAL];
    uint8 bRange;
    int nRet = ERR_GENERIC, nChildRet = 0;
    do {
        //--生成buf
        if (!(pNewBuf =
                  calloc(1, MAX(strlen(szOrigUrl), strlen(szPost)) * 2 + 512)))  // base64 1.33
            break;
        if (nOutBufSize < strlen(szPost) + 64)  // 32bytestring+aes16 padding
            break;
        //--生成必要参数
        GetRandString(szRandom32ByteString, 32);
        bRange = GetFirstIntFromSsoToken(szSsoToken) + 3;  //值+3
        //--计算MD5必要参数
        strcpy(szCookieSsoToken, szSsoToken);
        ByteReverse(szCookieSsoToken, strlen(szCookieSsoToken));
        ReverseStringByNum(szCookieSsoToken, bRange);
        szCookieSsoToken[20] = '\0';
        //--计算IV
        sprintf(pNewBuf, "%s+%s?", szCookieSsoToken, szOrigUrl);  //这里官方多出来一个'?',有点2b
        CalcMD5(pNewBuf, strlen(pNewBuf), szBuf);
        memset(szBuf2, 0, sizeof(szBuf2));
        Byte2Str(szBuf2, (uint8 *)szBuf, 0x10);
        //
        sprintf(pNewBuf, "%s%s", szRandom32ByteString, szPost);
        if ((nChildRet = AesAlgorithm(true, EVP_aes_256_ecb(), 1, (uint8 *)pNewBuf, strlen(pNewBuf),
                                      (uint8 *)szBuf2, (uint8 *)szOut)) <= 0)
            break;
        nChildRet = Base64_Encode(szOut, nChildRet, pNewBuf);
        if (nChildRet + 32 >= nOutBufSize)
            break;  // buf overflow
        snprintf(szOut, nOutBufSize, "{\"Jmbw\":\"%s\"}", pNewBuf);
        nRet = RET_SUCCESS;
    } while (false);
    if (pNewBuf)
        free(pNewBuf);
    return nRet;
}

//暂不考虑cookies中dzfp-ssotoken变量，实际测试后，时间戳(搜索 'new
// Date()')和指纹错误都不会影响最终服务器认证，只是提供一个随机数而已
void GetLos28199FingerprintNum(uint64 *pOutFingerPrint)
{
    //程序中需要的格式为1679673735635-2256260239-，第一个为unix时间辍，第二个为客户端指纹
    //客户端指纹:中断方式为dppt/asset_res/js/chunk_[random].js 文件底部fingerprint_getfp函数返回值
    //
    // 官方生成方式为:fingerprint_getfp由如下字符串变换，最终取10位整数
    // [UserAgent]###939x1904x24###-480###true###true###PDF Viewer::Portable
    // Document Format::application/pdf~pdf,text/pdf~pdf;[...];WebKit built-in PDF::Portable
    // Document Format::application/pdf~pdf,text/pdf~pdf###[Cookies中的dzfp-ssotoken]
    char szOut[BS_TINY] = "";
    GetRandHexString(szOut, 10, 10, NULL);
    *pOutFingerPrint = (uint64)atoll(szOut);
}

// llTimeStampMs需要和GetLzkqow23819Header函数的数值一致，szOrigUrlAndOutString输出长度需要增加40个字节左右，调用时需要预留空间
int GetLos28199UrlPath(char *szOrigUrl, uint64 llTimeStampMs, uint64 llFingerPrint,
                       char *szOutString, int nOutBufSize)
{
    // chunk.[random].js搜索'new
    // Date()'附近几个两三行的小函数基本都是sha1、md5、sm3等加密函数，下断点即可看到加密前明文
    //
    // 1679673735635-2256260239-/szzhzz/cssSecurity/v1/getPublicKey?t=1679673432303
    // md5=69a9eb86122cc282bc7bcb7cdda63d13
    // sm3=7a09f13d19cbede09b71d37be122abaa337821821b274b32ae59b64025b03817
    // md5=2de0987feb4346b4284cabd81553e272
    // los28199=[md5Sum]
    //
    // 1679673735635-2256260239-
    // 该字符串如何调试测试,首次getpublickey,set处下断点,N次之后看到1679*数值型变量，下一个即是2256260239
    int nRet = ERR_GENERIC, nNewBufSize = strlen(szOrigUrl) + 128;
    char szLosFlag[] = "&los28199=", szBuf[BS_NORMAL] = "", szBuf2[BS_NORMAL] = "", *pNewBuf = NULL;
    do {
        if (strstr(szOrigUrl, szLosFlag))
            break;
        if (!(pNewBuf = calloc(1, nNewBufSize)))
            break;
        sprintf(pNewBuf, "%llu-%llu-%s", llTimeStampMs, llFingerPrint, szOrigUrl);
        CalcMD5(pNewBuf, strlen(pNewBuf), szBuf);
        Byte2Str(szBuf2, (uint8 *)szBuf, 0x10);  // MD5'size

        CalcSM3((uint8 *)szBuf2, strlen(szBuf2), (uint8 *)szBuf);
        memset(szBuf2, 0, sizeof(szBuf2));
        Byte2Str(szBuf2, (uint8 *)szBuf, 0x20);  // SM3_DIGEST_LENGTH

        CalcMD5(szBuf2, strlen(szBuf2), szBuf);
        memset(szBuf2, 0, sizeof(szBuf2));
        Byte2Str(szBuf2, (uint8 *)szBuf, 0x10);  // MD5'size
        sprintf(pNewBuf, "%s%s%s", szOrigUrl, szLosFlag, szBuf2);
        if (strlen(pNewBuf) > nOutBufSize - 1)
            break;
        strcpy(szOutString, pNewBuf);
        nRet = RET_SUCCESS;
    } while (false);
    if (pNewBuf)
        free(pNewBuf);
    return nRet;
}

int CallbackDppt(EHHTTP hi, struct evhttp_request *req)
{
    char szBuf[BS_BIG];
    CtxBridge ctx = (CtxBridge)hi->ctxApp;
    do {
        //此处cookies中SSO_SECURITY_CHECK_TOKEN为大写，头中为小写
        struct evkeyvalq *headerOutput = evhttp_request_get_output_headers(req);
        if (FindStringFromEvBuffer(hi->ctxCon.bufCookies, "SSO_SECURITY_CHECK_TOKEN=", ";", szBuf,
                                   sizeof(szBuf)))
            break;
        evhttp_add_header(headerOutput, "sso_security_check_token", szBuf);
        if (strlen(ctx->szHeaderLzkqow23819))
            evhttp_add_header(headerOutput, "lzkqow23819", ctx->szHeaderLzkqow23819);
    } while (false);
    hi->cbHttpNewReq = NULL;
    return RET_SUCCESS;
}

int CallbackDpptfpdown(EHHTTP hi, struct evhttp_request *req)
{
    char szBuf[BS_BIG];
    CtxBridge ctx = (CtxBridge)hi->ctxApp;
    do {
        //此处cookies中SSO_SECURITY_CHECK_TOKEN为大写，头中为小写
        struct evkeyvalq *headerOutput = evhttp_request_get_output_headers(req);
        if (FindStringFromEvBuffer(hi->ctxCon.bufCookies, "SSO_SECURITY_CHECK_TOKEN=", ";", szBuf,
                                   sizeof(szBuf)))
            break;
        //evhttp_add_header(headerOutput, "sso_security_check_token", szBuf);
        if (strlen(ctx->szHeaderLzkqow23819))
            evhttp_add_header(headerOutput, "lzkqow23819", ctx->szHeaderLzkqow23819);
    } while (false);
    hi->cbHttpNewReq = NULL;
    return RET_SUCCESS;
}



int BuildTPassFormatHMacSHA256Packet(bool bDatagramSm4Encrypt, int encryptCode, char *szDatagram,
                                     char *szRandomHex16Bytes, struct evbuffer *bufOut)
{
    // zipCode=0 accessToken="" 先不加参数，用默认值
    int zipCode = 0;
    char accessToken[] = "";
    //{"zipCode":"0","encryptCode":"0","datagram":"{}","timestamp":"20230203104010","access_token":"","signtype":"HMacSHA256","signature":"..."}
    // signature=HMAC256(zipCode+encryptCode+c+timestamp+signtype+Random16bytes),
    char szSignType[] = "HMacSHA256", szBuf[BS_NORMAL], szBuf2[BS_NORMAL],
         szTimestamp[BS_TINY] = "";
    char *pOutJson = NULL, *szFinalDatagram = NULL, *pBufMidDatagram = NULL,
         *pBufMidDatagram2 = NULL;
    cJSON *jsonRoot = NULL;
    // clear
    evbuffer_drain(bufOut, -1);
    int nRet = ERR_GENERIC, nChildRet = 0;
    do {
        //必要2个
        GetOSTime(szTimestamp);
        if (!strlen(szRandomHex16Bytes))
            break;
        //判断是否加密sm4传输
        if (bDatagramSm4Encrypt) {
            if (!(pBufMidDatagram = calloc(1, strlen(szDatagram) * 2 + 256)) ||
                !(pBufMidDatagram2 = calloc(1, strlen(szDatagram) * 2 + 256)))
                break;
            if (!(nChildRet = EtaxSM4Algorithm(true, 1, (uint8 *)szDatagram, strlen(szDatagram),
                                           (uint8 *)szRandomHex16Bytes, (uint8 *)pBufMidDatagram2)))
                break;
            //加密完大于等于
            if (nChildRet < strlen(szDatagram))
                break;
            Byte2Str(pBufMidDatagram, (uint8 *)pBufMidDatagram2, nChildRet);
            szFinalDatagram = pBufMidDatagram;
        } else
            szFinalDatagram = szDatagram;
        if (!(jsonRoot = cJSON_CreateObject()))
            break;
        // zipCode
        sprintf(szBuf, "%d", zipCode);
        evbuffer_add(bufOut, szBuf, strlen(szBuf));
        cJSON_AddStringToObject(jsonRoot, "zipCode", szBuf);
        // encryptCode
        sprintf(szBuf, "%d", encryptCode);
        evbuffer_add(bufOut, szBuf, strlen(szBuf));
        cJSON_AddStringToObject(jsonRoot, "encryptCode", szBuf);
        // datagram
        evbuffer_add(bufOut, szFinalDatagram, strlen(szFinalDatagram));
        cJSON_AddStringToObject(jsonRoot, "datagram", szFinalDatagram);
        // timstamp
        evbuffer_add(bufOut, szTimestamp, strlen(szTimestamp));
        cJSON_AddStringToObject(jsonRoot, "timestamp", szTimestamp);
        // access_token
        cJSON_AddStringToObject(jsonRoot, "access_token", "");
        // signtype
        evbuffer_add_printf(bufOut, "%s", szSignType);
        cJSON_AddStringToObject(jsonRoot, "signtype", szSignType);
        // signature
        nChildRet = sizeof(szBuf);
        memset(szBuf, 0, nChildRet);
        if (!HMAC(EVP_sha256(), szRandomHex16Bytes, 16, evbuffer_pullup(bufOut, -1),
                  evbuffer_get_length(bufOut), (uint8 *)szBuf, (uint *)&nChildRet) ||
            nChildRet != 32)
            break;
        memset(szBuf2, 0, sizeof(szBuf2));
        Byte2Str(szBuf2, (uint8 *)szBuf, 32);
        cJSON_AddStringToObject(jsonRoot, "signature", szBuf2);
        evbuffer_drain(bufOut, -1);
        if (!(pOutJson = cJSON_Print(jsonRoot)))
            break;
        evbuffer_add_printf(bufOut, "%s", pOutJson);
        nRet = RET_SUCCESS;
    } while (false);
    if (pOutJson)
        free(pOutJson);
    if (jsonRoot)
        cJSON_Delete(jsonRoot);
    if (pBufMidDatagram)
        free(pBufMidDatagram);
    if (pBufMidDatagram2)
        free(pBufMidDatagram2);
    return nRet;
}

// szWantKey为datagram下子键值，空值则返回整个datagram
int GetDataGramValueFromTPassReponse(char *szJsonReponse, const char *szWantKey, char *szOutValue,
                                     int nOutBufSize)
{
    int nRet = ERR_TA_GET_JSONXML_VALUE, nChildRet = 0;
    cJSON *root = NULL, *objDataGram = NULL, *rootDataGram = NULL, *objTarget = NULL;
    do {
        if (!(root = cJSON_Parse(szJsonReponse)))
            break;
        if (!cJSON_GetArraySize(root))
            break;
        if (!(objDataGram = cJSON_GetObjectItem(root, "datagram")))
            break;
        if (!strlen(objDataGram->valuestring))
            break;
        if (!strlen(szWantKey)) {  //空值则返回整个datagram
            strcpy(szOutValue, objDataGram->valuestring);
            nRet = RET_SUCCESS;
            break;
        }
        if (!(rootDataGram = cJSON_Parse(objDataGram->valuestring)))
            break;
        if (!cJSON_GetArraySize(rootDataGram))
            break;
        if (!(objTarget = cJSON_GetObjectItem(rootDataGram, szWantKey)))
            break;
        if (strlen(objTarget->valuestring) > nOutBufSize - 1)
            break;
        strcpy(szOutValue, objTarget->valuestring);
        nRet = RET_SUCCESS;
    } while (false);
    if (root)
        cJSON_Delete(root);
    if (rootDataGram)
        cJSON_Delete(rootDataGram);
    return nRet;
}

// 1)szWantKey为datagram下子键值，空值则返回整个datagram;2)如果明确响应不含有sm4加密，szRandomHex16Bytes可为空,为空时只检测szJsonReponse返回值
int EtaxGetDataGramValueFromTPassReponse(char *szJsonReponse, const char *szRandomHex16Bytes,
                                     const char *szWantKey, char *szOutValue, int nOutBufSize)
{
    bool bNeedSm4Decrypt = false;
    int nRet = ERR_TA_GET_JSONXML_VALUE, nChildRet = 0;
    cJSON *root = NULL, *objItem = NULL, *objDataGram = NULL, *rootDataGram = NULL,
          *objTarget = NULL;
    char szBuf[BS_NORMAL], *szDataGram = NULL, *pDecBuf = NULL, *pDecBuf2 = NULL;
    do {
        //返回值判断
        if (!strstr(szJsonReponse, "\"code\":1000")) {
            if (!(root = cJSON_Parse(szJsonReponse)))
                break;
            if (!(objItem = cJSON_GetObjectItem(root, "msg")))
                break;
            if (strlen(objItem->valuestring) >= nOutBufSize)
                break;
            strcpy(szOutValue, objItem->valuestring);
            break;
        }
        if (!strlen(szRandomHex16Bytes)) {
            nRet = RET_SUCCESS;
            break;
        }
        if (strstr(szJsonReponse, "\"encryptCode\":\"2\""))
            bNeedSm4Decrypt = true;
        if (!(root = cJSON_Parse(szJsonReponse)))
            break;
        if (!cJSON_GetArraySize(root))
            break;
        if (!(objDataGram = cJSON_GetObjectItem(root, "datagram")))
            break;
        if (!strlen(objDataGram->valuestring)) {
            szOutValue[0] = '\0';
            nRet = RET_SUCCESS;
            break;
        }
        szDataGram = objDataGram->valuestring;
        if (bNeedSm4Decrypt) {
            if (!(pDecBuf = calloc(1, strlen(szDataGram) + 512)) ||
                !(pDecBuf2 = calloc(1, strlen(szDataGram) + 512)))
                break;
            memset(szBuf, 0, sizeof(szBuf));
            strncpy(szBuf, szRandomHex16Bytes, 8);
            //  strcat(szBuf, "feB!Xm2y");
            strcat(szBuf, "HTUurf^2");  // 20230729 官方更改，随机因子种子为此
            nChildRet = Str2Byte((uint8 *)pDecBuf, szDataGram, strlen(szDataGram));
            if (!(nChildRet = EtaxSM4Algorithm(false, 1, (uint8 *)pDecBuf, nChildRet, (uint8 *)szBuf,
                                           (uint8 *)pDecBuf2)))
                break;
            pDecBuf2[nChildRet] = '\0';
            szDataGram = pDecBuf2;
        }
        if (!strlen(szWantKey)) {  //空值则返回整个datagram
            strcpy(szOutValue, szDataGram);
            nRet = RET_SUCCESS;
            break;
        }
        if (!(rootDataGram = cJSON_Parse(szDataGram)))
            break;
        if (!cJSON_GetArraySize(rootDataGram))
            break;
        if (!(objTarget = cJSON_GetObjectItem(rootDataGram, szWantKey)))
            break;
        if (strlen(objTarget->valuestring) > nOutBufSize - 1)
            break;
        strcpy(szOutValue, objTarget->valuestring);
        nRet = RET_SUCCESS;
    } while (false);
    if (root)
        cJSON_Delete(root);
    if (rootDataGram)
        cJSON_Delete(rootDataGram);
    if (pDecBuf)
        free(pDecBuf);
    if (pDecBuf2)
        free(pDecBuf2);
    return nRet;
}

// szOptSm4Iv不传入则不解密
int DecodeTpassReponseJson(EHHTTP hi, char *szRepBuf, char *szOptSm4Iv)
{
    int nRet = ERR_TA_REPONSE_CHECK, nChildRet = 0;
    cJSON *root = NULL, *objItem = NULL;
    do {
        //返回值判断
        if (!strstr(szRepBuf, "\"code\":1000")) {
            if (!(root = cJSON_Parse(szRepBuf)))
                break;
            if (!(objItem = cJSON_GetObjectItem(root, "msg")))
                break;
            SetLastError(hi->hDev->hUSB, nRet, objItem->valuestring);
            break;
        }
        if (!strlen(szOptSm4Iv)) {
            nRet = RET_SUCCESS;
            break;
        }
        //解密响应
        if (GetDataGramValueFromTPassReponse(szRepBuf, "", szRepBuf, (int)sizeof(szRepBuf)))
            break;
        nChildRet = Str2Byte((uint8 *)szRepBuf, szRepBuf, strlen(szRepBuf));
        if (!(nChildRet = EtaxSM4Algorithm(false, 1, (uint8 *)szRepBuf, nChildRet, (uint8 *)szOptSm4Iv,
                                       (uint8 *)szRepBuf)))
            break;
        szRepBuf[nChildRet] = '\0';
        nRet = RET_SUCCESS;
    } while (false);
    if (root)
        cJSON_Delete(root);
    return nRet;
}

int CallbackTPassSetHttpHeaderUuid(EHHTTP hi, struct evhttp_request *req)
{
    CtxBridge ctx = (CtxBridge)hi->ctxApp;
    struct evkeyvalq *headerOutput = evhttp_request_get_output_headers(req);
    evhttp_add_header(headerOutput, "X-SM4-INFO", "0");           // static
    evhttp_add_header(headerOutput, "X-TEMP-INFO", ctx->szUuid);  //存放uuid信息
    if (strlen(ctx->szToken))  //如果有已认证信息则需要加上
        evhttp_add_header(headerOutput, "Authorization", ctx->szToken);
    //此处加 "Accept-Encoding", "gzip, deflate, br"，gzip支持则返回响应会被压缩
    // C语言需要第三方压缩库支持，因此暂不加入压缩头请求
    hi->cbHttpNewReq = NULL;  //只有在认证时需要，后续请求不再需要，因此不再回调
    return RET_SUCCESS;
}

//返回值在输入值buf上，因此输入buf需要稍微大点（有buf大小校验，不会越界，但是不够会报错）
// int TpassLogicIo(EHHTTP hi, char *szPostUrl, char *szBuf, int nBufSize)
// {
//     int nRet = ERR_GENERIC;
//     struct evbuffer *buf = NULL;
//     CtxBridge ctx = (CtxBridge)hi->ctxApp;
//     do {
//         if (!(buf = evbuffer_new()))
//             break;
//         if (BuildTPassFormatHMacSHA256Packet(true, 2, szBuf, ctx->szRandom16BytesKey, buf))
//             break;
//         //需要在回调函数中使用hi->ctxApp.szTmpBuf中保存的uuid
//         hi->cbHttpNewReq = CallbackTPassSetHttpHeaderUuid;
//         memset(szBuf, 0, nBufSize);
//         if (EtaxHTTPPost(hi, szPostUrl, "application/json", (char *)evbuffer_pullup(buf, -1),
//                      evbuffer_get_length(buf), szBuf, nBufSize) < 0)
//             break;
//         if (DecodeTpassReponseJson(hi, szBuf, ctx->szRandom16BytesKey))
//         {
//             printf("          DecodeTpassReponseJson                DecodeTpassReponseJson      \r\n");
//             break;
//         }
            
//         nRet = RET_SUCCESS;
//     } while (false);
//     if (buf)
//         evbuffer_free(buf);
//     return nRet;
// }

int TpassLogicIo(EHHTTP hi, char *szPostUrl, char *szBuf, int nBufSize)
{
    char szBuf_tmp[BS_BLOCK * 2000] = {0};
    int nRet = ERR_GENERIC;
    struct evbuffer *buf = NULL;
    CtxBridge ctx = (CtxBridge)hi->ctxApp;
    do {
        if (!(buf = evbuffer_new()))
            break;
        if (BuildTPassFormatHMacSHA256Packet(true, 2, szBuf, ctx->szRandom16BytesKey, buf))
            break;
        //需要在回调函数中使用hi->ctxApp.szTmpBuf中保存的uuid
        hi->cbHttpNewReq = CallbackTPassSetHttpHeaderUuid;
        memset(szBuf, 0, nBufSize);
        memset(szBuf_tmp, 0, sizeof(szBuf_tmp));
        if (EtaxHTTPPost(hi, szPostUrl, "application/json", (char *)evbuffer_pullup(buf, -1),
                     evbuffer_get_length(buf), szBuf_tmp, sizeof(szBuf_tmp)) < 0)
            break;
        if (DecodeTpassReponseJson(hi, szBuf_tmp, ctx->szRandom16BytesKey))
        {
            printf("          DecodeTpassReponseJson                DecodeTpassReponseJson      \r\n");
            break;
        }
        printf("     ________________ sizeof(szBuf_tmp) = %d         \r\n",strlen(szBuf_tmp));
        printf("     ________________ szBuf_tmp = %s         \r\n",szBuf_tmp);
        if (strlen(szBuf_tmp) < nBufSize)
        {
           //memcpy(szBuf,szBuf_tmp,sizeof(szBuf_tmp));
           strcpy(szBuf,szBuf_tmp);
        }
        else
        {
            printf("           szBuf_tmp     ccopy      error !!!                  \r\n");
            break;
        }
        
        nRet = RET_SUCCESS;
    } while (false);
    if (buf)
        evbuffer_free(buf);
    return nRet;
}

//步骤1获取公钥和步骤2发送国密四，2个步骤是任何操作（查询二维码、获取地区、SSO登陆等等）的建立连接的必要操作
int TPassLoginAuthStep1and2(EHHTTP hi)
{
    CtxBridge ctx = (CtxBridge)hi->ctxApp;
    int nRet = ERR_GENERIC, nChildRet = 0;
    struct evbuffer *buf = NULL;
    char szRepBuf[2048] = "", szBuf[BS_BLOCK] = "", szUuid[BS_SMALL] = "", szPubKey[BS_BIG] = "";
    do {
        if (!(buf = evbuffer_new()))
            break;
        //GetRandHexString(ctx->szRandom16BytesKey, 16, 16, NULL);  //获取后贯穿始终
        GetRandString(ctx->szRandom16BytesKey, 8);
        strcat(ctx->szRandom16BytesKey, "feB!Xm2y");
        // ===============================step1--getPublicKey======================================
        if (BuildTPassFormatHMacSHA256Packet(false, 0, "{}", ctx->szRandom16BytesKey, buf))
            break;
        if (EtaxHTTPPost(hi, "/sys-api/v1.0/auth/oauth2/getPublicKey", "application/json",
                     (char *)evbuffer_pullup(buf, -1), evbuffer_get_length(buf), szRepBuf,
                     sizeof(szRepBuf)) < 0)
            break;
        if (DecodeTpassReponseJson(hi, szRepBuf, "")) {
            _WriteLog(LL_FATAL, "Try to TPassLoginAuthStep1and2 decode rep1 failed");
            break;
        }
        if (GetDataGramValueFromTPassReponse(szRepBuf, "uuid", szUuid, sizeof(szUuid)))  // 32字节
            break;
        strcpy(ctx->szUuid, szUuid);
        // =============================step2--sendSm4=============================================
        if (GetDataGramValueFromTPassReponse(szRepBuf, "publickey", szPubKey, sizeof(szPubKey)))
            break;  // szPubKey130字节
        // etax前端加密vue加密，为sm2 C1C2C3加密格式
        //参考https://blog.csdn.net/qq_34169240/article/details/120312569
        if ((nChildRet = SM2EncryptAlgorithm((uint8 *)ctx->szRandom16BytesKey, 16, szPubKey,
                                             (uint8 *)szRepBuf, sizeof(szRepBuf))) <= 0)
            break;
        if (112 != (nChildRet = SM2EncryptDataRemoveAsn1((uint8 *)szRepBuf, nChildRet)))
            break;
        Byte2Str(szBuf, (uint8 *)szRepBuf, nChildRet);
        sprintf(szRepBuf, "{\"uuid\":\"%s\",\"secret\":\"%s\"}", szUuid, szBuf);
        if (BuildTPassFormatHMacSHA256Packet(false, 0, szRepBuf, ctx->szRandom16BytesKey, buf))
            break;
        if (EtaxHTTPPost(hi, "/sys-api/v1.0/auth/white/sendSm4", "application/json",
                     (char *)evbuffer_pullup(buf, -1), evbuffer_get_length(buf), szRepBuf,
                     sizeof(szRepBuf)) < 0)
            break;
        if (DecodeTpassReponseJson(hi, szRepBuf, "")) {
            _WriteLog(LL_FATAL, "Try to TPassLoginAuthStep1and2 decode rep2 failed");
            break;
        }
        //保存登陆成功的uuid信息，给后续业务逻辑使用
        nRet = RET_SUCCESS;
    } while (false);
    if (buf)
        evbuffer_free(buf);
    return nRet;
}

//给etax的单点登陆认证
int TPassLoginAuthSso(EHHTTP hi, struct evhttp_uri *uriTpassFull)
{
    int nRet = ERR_GENERIC, nChildRet = 0;
    struct evbuffer *buf = NULL;
    CtxBridge ctx = (CtxBridge)hi->ctxApp;
    //可变重复用buf，szRepBuf最大接收7k数据,szBuf最大接收3k+数据
    char szRepBuf[9216] = "", szBuf[5120] = "";
    do {
        if (!(buf = evbuffer_new()))
            break;
        // =============================step3--serverHello=============================================
        memset(szBuf, 0, sizeof(szBuf));
        if (GetTaClientHello(1, hi->hDev, szBuf) < 0) {
            nRet = -1;
            break;
        }
        sprintf(szRepBuf, "{\"server_client_hello\":\"%s\"}", szBuf);
        if (BuildTPassFormatHMacSHA256Packet(true, 2, szRepBuf, ctx->szRandom16BytesKey, buf))
            break;
        //需要在回调函数中使用hi->ctxApp.szTmpBuf中保存的uuid
        hi->cbHttpNewReq = CallbackTPassSetHttpHeaderUuid;
        if (EtaxHTTPPost(hi, "/sys-api/v1.0/auth/cert/serverHello", "application/json",
                     (char *)evbuffer_pullup(buf, -1), evbuffer_get_length(buf), szRepBuf,
                     sizeof(szRepBuf)) < 0) {
            nRet = -2;
            break;
        }
        if (DecodeTpassReponseJson(hi, szRepBuf, "")) {
            nRet = -3;
            break;
        }
        // =============================step4--serverAuth=============================================
        if (TPassLoginAuthDecodeStep3RepForStep4(hi, uriTpassFull, ctx->szRandom16BytesKey,
                                                 szRepBuf, sizeof(szRepBuf))) {
            nRet = -4;
            break;
        }
        if (TpassLogicIo(hi, "/sys-api/v1.0/auth/cert/serverAuth", szRepBuf, sizeof(szRepBuf))) {
            nRet = -5;
            break;
        }
        evbuffer_drain(hi->bufHttpRep, -1);
        evbuffer_add_printf(hi->bufHttpRep, "%s", szRepBuf);
        //_WriteHexToDebugFile("user.json", (uint8 *)szRepBuf, strlen(szRepBuf));
        nRet = RET_SUCCESS;
    } while (false);
    if (buf)
        evbuffer_free(buf);
    return nRet;
}

//注销会话，不调用关闭问题也不大，相当于直接关闭浏览器
int EtaxBridgeClose(EHHTTP hi)
{
    int nRet = RET_SUCCESS;
    struct ContextHttpAppBridge *ctxApp = NULL;
    do {
        if (!hi->ctxApp)
            break;
        ctxApp = (struct ContextHttpAppBridge *)hi->ctxApp;
    } while (false);
    if (ctxApp)
        free(ctxApp);
    return nRet;
}

bool FillUploadAddrModelDynamic(uint8 bServType, char *szRegCode6, char *szTaxAuthorityURL,
                                char *szInitRequestPage, struct EtaxUploadAddrModel *pstOutModel)
{
    bool ret = false;
    char host[100] = "";
    int port = 0;
    struct evhttp_uri *uriTa = NULL;
    do {
        if (!(uriTa = evhttp_uri_parse(szTaxAuthorityURL)))
            break;
        pstOutModel->bServType = bServType;
        strcpy(pstOutModel->szRegName, evhttp_uri_get_host(uriTa));
        strcpy(pstOutModel->szRegCode, szRegCode6);
        // ta
        if (GetHostAndPortFromEvUri(uriTa, host, &port))
            break;
        snprintf(pstOutModel->szTaxAuthorityURL, sizeof(pstOutModel->szTaxAuthorityURL),
                 "%s://%s:%d", evhttp_uri_get_scheme(uriTa), host, port);
        strcpy(pstOutModel->szInitRequestPage, szInitRequestPage);
        _WriteLog(LL_DEBUG, "Get dynamic ta server's url:[%s]", pstOutModel->szTaxAuthorityURL);
        ret = true;
    } while (false);
    if (uriTa)
        evhttp_uri_free(uriTa);
    return ret;
}

int InitSecurityConfig(EHHTTP hi)
{
    int nChildRet = ERR_GENERIC, nRet = ERR_GENERIC, nServRepBufSize = 24 * BS_BLOCK;
    char szUrlBuf[BS_HUGE] = "", *szServRep = NULL;
    CtxBridge ctx = (CtxBridge)hi->ctxApp;
    cJSON *root = NULL, *objItem = NULL;
    uint64 llTimeStampMs = GetMicroSecondUnixTimestamp();
    do {
        if (strlen(ctx->szSecurityTokenKey)) {
            nRet = RET_SUCCESS;  // has init
            break;
        }
        if (!(szServRep = calloc(1, nServRepBufSize))) {
            nRet = -1;
            break;
        }
        //如果不重置似乎一直错
        GetLos28199FingerprintNum(&ctx->llFingerPrint);
        //--公钥只获取一次，后期二次开发，可以将前几步操作独立出去，第一次是个错的算法请求，目的是获取到webpayload
        sprintf(szUrlBuf, "/sqlz/cssSecurity/v1/getPublicKey?t=%llu",
                GetMicroSecondUnixTimestamp());
        if (GetLos28199UrlPath(szUrlBuf, llTimeStampMs, ctx->llFingerPrint, szUrlBuf,
                               sizeof(szUrlBuf))) {
            nRet = -2;
            break;
        }
        if (GetCookiesSegment(hi->ctxCon.bufCookies, "dzfp-ssotoken", szServRep, nServRepBufSize)){
            break;
        }
        if (GetLzkqow23819Header(szUrlBuf, szServRep, "", "", llTimeStampMs, ctx->llFingerPrint,
                                 ctx->szHeaderLzkqow23819)) {
            nRet = -3;
            break;
        }
        hi->cbHttpNewReq = CallbackDppt;
        if ((nChildRet = EtaxHTTPPost(hi, szUrlBuf, "", "", 0, szServRep, nServRepBufSize)) <
            0)  //返回值365字节左右
        {
            nRet = -4;
            break;
        }
        // json结果代码0,如果格式出错必然是419返回值
        if (!(root = cJSON_Parse(szServRep))) {
            nRet = -5;
            break;
        }
        if (!(objItem = cJSON_GetObjectItem(root, "code")))
            break;
        if (objItem->valueint)
            break;
        if (!(objItem = cJSON_GetObjectItem(root, "data"))) {
            nRet = -6;
            break;
        }
        //备份buf，后期连接需要shellcode计算参数
        evbuffer_add_printf(ctx->ebJsVmpWebshellcode, "%s", objItem->valuestring);
        if (GetCookiesSegment(hi->ctxCon.bufCookies, "dzfp-ssotoken", szServRep, nServRepBufSize)) {
            nRet = -7;
            break;
        }
        if (GetLzkqow23819Header(szUrlBuf, szServRep, objItem->valuestring, "", llTimeStampMs,
                                 ctx->llFingerPrint, ctx->szHeaderLzkqow23819)) {
            nRet = -8;
            break;
        }
        hi->cbHttpNewReq = CallbackDppt;
        if ((nChildRet = EtaxHTTPPost(hi, szUrlBuf, "", "", 0, szServRep, nServRepBufSize)) <
            0)  //返回值365字节左右
        {
            nRet = -9;
            break;
        }
        cJSON_Delete(root);  // close old
        //{"ruuid":167755..,"tokenKey":"0575c620f9034..","sign":"c51c...","securityrate":900,"publicKey":"MIG..."}
        if (!(root = cJSON_Parse(szServRep)))
            break;
        if (!(objItem = cJSON_GetObjectItem(root, "tokenKey"))) {
            nRet = ERR_TA_TOKEN_CHECK;
            break;
        }
        strcpy(ctx->szSecurityTokenKey, objItem->valuestring);
        nRet = RET_SUCCESS;
    } while (false);
    if ((nRet && objItem && objItem->valuestring)) {
        //第二次GetLzkqow23819Header会容易出错，对一些错误和不能解析的需要记录数据回溯测试
        _WriteHexToDebugFile("etax_dppt-lastfailed-rep.txt", (uint8 *)objItem->valuestring,
                             strlen(objItem->valuestring));
    }
    if (root)
        cJSON_Delete(root);
    if (szServRep)
        free(szServRep);
    return nRet;
}

int DpptHandshake(HETAX hEtax, char *szDpptInitUrl)
{
    EHHTTP hi = &hEtax->dppt;
    char szBuf[BS_BLOCK] = "", szOauth2HandleServletUrl[BS_HUGE] = "", szCode[BS_SMALL] = "",
         *pDecodeUrl = NULL, szFlag[] = "\"code\":\"", *pValue = NULL;
    int nRet = ERR_TA_ETAX_LOGIN, nChildRet = 0, nBufSize = sizeof(szBuf);
    struct evhttp_uri *uriDpptInit = NULL;
    CtxBridge ctxTpass = (CtxBridge)hEtax->tpass.ctxApp;
    do {
        // 1)访问Dppt szzhzz/spHandler?cdlj=...，获取初始化跳转连接
        if (!(uriDpptInit = evhttp_uri_parse(szDpptInitUrl)))
            break;
        if (nBufSize < snprintf(szBuf, nBufSize, "%s?%s", evhttp_uri_get_path(uriDpptInit),
                                evhttp_uri_get_query(uriDpptInit)))
            break;
        if ((nChildRet = EtaxHTTPGetEvent(hi, szBuf)) < 0 || hi->ctxCon.nHttpRepCode != 302) {
            _WriteLog(LL_FATAL, "Dpp tHandshake1 failed");
            break;
        }
        // 2)访问tpass，获取dppt跳转连接
        // https://tpass.sh.../api/v1.0/auth/oauth2/login?response_type=code&client_id=d3f...de&redirect_uri=https%3A%2F%2Fdppt.shanghai.chinatax.gov.cn%3A8443%2Ffpjf%2FOauth2HandleServlet%3FcdPath%3DZG...Dgx&state=1bc54f90920c4ee3b0c4c73e03f92668&ruuid=1690615463862
        pDecodeUrl = evhttp_uridecode(evhttp_uri_get_query(hi->ctxCon.uriLocation), 0, NULL);
        if (nBufSize < snprintf(szBuf, nBufSize, "%s?%s",
                                evhttp_uri_get_path(hi->ctxCon.uriLocation), pDecodeUrl))
            break;
        if ((nChildRet = EtaxHTTPGetEvent(&hEtax->tpass, szBuf)) < 0 ||
            hEtax->tpass.ctxCon.nHttpRepCode != 302) {
            _WriteLog(LL_FATAL, "Dppt Handshake2 failed");
            break;
        }
        if (FindStringFromEvBuffer(hEtax->tpass.ctxCon.bufLocation, "client_id=", "&",
                                   ctxTpass->szClientId, sizeof(ctxTpass->szClientId)))
            break;
        if (FindStringFromEvBuffer(hEtax->tpass.ctxCon.bufLocation, "redirect_uri=", "&ruuid=",
                                   szOauth2HandleServletUrl, sizeof(szOauth2HandleServletUrl)))
            break;

        sprintf(szBuf, "{\"client_id\":\"%s\",\"redirect_uri\":\"%s\"}", ctxTpass->szClientId,
                szOauth2HandleServletUrl);
        if (TpassLogicIo(&hEtax->tpass, "/sys-api/v1.0/auth/user/verifyLogin", szBuf,
                         sizeof(szBuf)))
            break;
        if (!(pValue = strstr(szBuf, szFlag)))
            break;
        pValue += strlen(szFlag);
        *strchr(pValue, '"') = '\0';
        strcpy(szCode, pValue);
        // 3)此前服务器返回的state和code合并，简单认证
        // if (!(pValue = strstr(szOauth2HandleServletUrl, "/fpjf/")))
        //     break;

        struct evhttp_uri *uriDpptRepRedirect = NULL;
        if (!(uriDpptRepRedirect = evhttp_uri_parse(szOauth2HandleServletUrl)))
            break;        
        snprintf(szBuf, sizeof(szBuf), "%s?%s&code=%s", evhttp_uri_get_path(uriDpptRepRedirect),evhttp_uri_get_query(uriDpptRepRedirect),szCode);
        if ((nChildRet = EtaxHTTPGetEvent(hi, szBuf)) < 0 || hi->ctxCon.nHttpRepCode != 302) {
            _WriteLog(LL_FATAL, "Dppt Handshake3 failed");
            break;
        }
        // 4)访问上一步获取到的dppt连接，获取dppt最终认证完毕页面200
        if (nBufSize < snprintf(szBuf, nBufSize, "%s?%s",
                                evhttp_uri_get_path(hi->ctxCon.uriLocation),
                                evhttp_uri_get_query(hi->ctxCon.uriLocation)))
            break;
        if ((nChildRet = EtaxHTTPGetEvent(hi, szBuf)) < 0 || hi->ctxCon.nHttpRepCode != 200) {
            _WriteLog(LL_FATAL, "Dppt Handshake4 failed");
            break;
        }
        nRet = RET_SUCCESS;
    } while (false);
    if (uriDpptInit)
        evhttp_uri_free(uriDpptInit);
    if (pDecodeUrl)
        free(pDecodeUrl);
    return nRet;
}

int CreateEtaxDpptTalk(HETAX hEtax)
{
    //官方登陆etax前有个dppt的退出
    // GET https://dppt.shanghai.chinatax.gov.cn:8443/kpfw/SSOLogout?random=0.8670495387648007
    // Cookie: x_host_key=188e712c807-82ff18c39f132568d7bc14dde24ca5b26d6736f4
    int nRet = ERR_TA_ETAX_LOGIN, nChildRet = 0;
    char szBuf[BS_HUGE] = "";
    HDEV hDev = hEtax->etax.hDev;
    EHHTTP hi = (EHHTTP)&hEtax->dppt;

    struct timeval start;
    gettimeofday(&start, NULL);
    uint64 timeStamp = 1000 * start.tv_sec + start.tv_usec / 1000;

    do {
        //通过主界面函数列表，获取其他操作调用功能列表地址
        // sprintf(szBuf, "/wszx-web/api/desktop/allFunctionsEX/get?_=%llu",
        //         timeStamp);
        // if ((nChildRet = EtaxHTTPGetEvent(&hEtax->etax, szBuf)) <= 0)
        // {
        //     printf("szBuf error=%s\r\n",szBuf);
        //     break;
        // }
        // //printf("szBuf=%s\r\n",szBuf);
        // //_WriteHexToDebugFile("func-menu.json", evbuffer_pullup(hi->bufHttpRep, -1), nChildRet);
        // //找到形如下列地址
        // // https://dppt.shanghai.chinatax.gov.cn:8443/szzhzz/spHandler?cdlj=digital-tax-account&id=80000995&code=80000995
        // if (FindStringFromEvBuffer(hEtax->etax.bufHttpRep, "\"code\":\"80000995\",\"url\":\"",
        //                            "\",", szBuf, sizeof(szBuf)))
        //     break;
        strcpy(szBuf,"https://dppt.shanghai.chinatax.gov.cn:8443/szzhzz/spHandler?cdlj=digital-tax-account&id=80000995&code=80000995");
        if (!FillUploadAddrModelDynamic(TAADDR_ETAX_DPPT, hDev->szRegCode, szBuf, "/",
                                        &hi->modelTaAddr)) {
            _WriteLog(LL_WARN, "Get server url failed");
            nRet = -1;
            break;
        }
        if ((nChildRet = EtaxHTTPInit(hi, hDev)) || (nChildRet = EtaxHTTPOpen(hi))) {
            _WriteLog(LL_WARN, "HTTPS Init/Open failed, nRet:%d", nChildRet);
            nRet = -2;
            break;
        }
        if (!(hi->ctxApp = calloc(1, sizeof(struct ContextHttpAppBridge))))
            break;
        hi->cbHttpClose = EtaxBridgeClose;
        if (!(((struct ContextHttpAppBridge *)hi->ctxApp)->ebJsVmpWebshellcode =
                  (void *)evbuffer_new()))
            break;
        if (DpptHandshake(hEtax, szBuf))
            break;
        nRet = RET_SUCCESS;
    } while (false);
    return nRet;
}


//从token服务器获取登陆凭证去登陆Etax;登陆完成后返回的用户列表在hiTPass->bufHttpRep中
int CreateEtaxTpassTalk(EHHTTP hi, HDEV hDev, EHHTTP hiEtax)
{
    int nRet = ERR_TA_ETAX_LOGIN, nChildRet = 0;
    char *pValue = NULL;
    cJSON *jsonTpassConfig = NULL;
    cJSON *jsonValue = NULL;
    struct evhttp_uri *uriTpassFull = NULL;
    char *szTPassConfigJson = (char *)evbuffer_pullup(hiEtax->bufHttpRep, -1);
    do {
        if (hiEtax->ctxCon.nHttpRepCode != 200)
            break;

        if(strcmp(hiEtax->modelTaAddr.szRegCode,"31") == 0){
            //获取跳转tpass地址
            if (!(jsonTpassConfig = cJSON_Parse(szTPassConfigJson)))
                break;
            if (!(pValue = cJSON_GetObjectItem(jsonTpassConfig, "value")->valuestring))
                break;
        }
        if(strcmp(hiEtax->modelTaAddr.szRegCode,"51") == 0){
            //获取跳转tpass地址
            if (!(jsonTpassConfig = cJSON_Parse(szTPassConfigJson)))
                break;
            int size = cJSON_GetArraySize(jsonTpassConfig);
            if (!(jsonValue = cJSON_GetObjectItem(jsonTpassConfig, "value"))){
                break;
            }      
            if (!(pValue = cJSON_GetObjectItem(jsonValue, "loginUrl")->valuestring)){
                break;
            }
        }        
        printf("pValue=%s\r\n",pValue);
        if (!(uriTpassFull = evhttp_uri_parse(pValue)))
            break;

        if (!FillUploadAddrModelDynamic(TAADDR_ETAX_TPASS, hDev->szRegCode, pValue, "/",
                                        &hi->modelTaAddr)) {
            _WriteLog(LL_WARN, "Get server url failed");
            nRet = -1;
            break;
        }
        if ((nChildRet = EtaxHTTPInit(hi, hDev)) || (nChildRet = EtaxHTTPOpen(hi))) {
            _WriteLog(LL_WARN, "HTTPS Init/Open failed, nRet:%d", nChildRet);
            nRet = -2;
            break;
        }
        if (!(hi->ctxApp = calloc(1, sizeof(struct ContextHttpAppBridge))))
            break;
        hi->cbHttpClose = EtaxBridgeClose;
        //登陆步骤1、2
        if (TPassLoginAuthStep1and2(hi)) {
            _WriteLog(LL_FATAL, "EtaxTpass TPassLoginAuthStep1and2 failed");
            nRet = -3;
            break;
        }
        //登陆步骤3、4
        if ((nChildRet = TPassLoginAuthSso(hi, uriTpassFull))) {
            _WriteLog(LL_FATAL, "EtaxTpass TPassLoginAuthSso failed,Ret:%d", nChildRet);
            nRet = -4;
            break;
        }
        _WriteLog(LL_INFO, "TPass connection OK");
        nRet = RET_SUCCESS;
    } while (false);
    if (jsonTpassConfig)
        cJSON_Delete(jsonTpassConfig);
    if (uriTpassFull)
        evhttp_uri_free(uriTpassFull);
    return nRet;
}

//有cokies会话超时时间，不可长时间开启
int CreateEtaxTalk(EHHTTP hi, HDEV hDev)
{
    int nRet = ERR_TA_ETAX_LOGIN, nChildRet = 0;
    memset(hi, 0, sizeof(struct HTTP));
    do {
        if (GetTaServerURL(TAADDR_ETAX, hDev->szRegCode, &hi->modelTaAddr) != RET_SUCCESS) {
            _WriteLog(LL_WARN, "Get server url failed");
            nRet = ERR_TA_GET_SERVERADDR;
            break;
        }
        if ((nChildRet = EtaxHTTPInit(hi, hDev)) || (nChildRet = EtaxHTTPOpen(hi))) {
            _WriteLog(LL_WARN, "HTTPS Init/Open failed, nRet:%d", nChildRet);
            break;
        }
        if (!(hi->ctxApp = calloc(1, sizeof(struct ContextHttpAppBridge))))
            break;
        hi->cbHttpClose = EtaxBridgeClose;
        //获取tpass地址
        if (EtaxHTTPGetEvent(hi, "/wszx-web/api/sh/login/mode") < 0)
            break;
        _WriteLog(LL_INFO, "Etax connection OK");
        nRet = RET_SUCCESS;
    } while (false);
    return nRet;
}

int CreateEtaxTalkCookies(EHHTTP hi, HDEV hDev)
{
    int nRet = ERR_TA_ETAX_LOGIN, nChildRet = 0;
    memset(hi, 0, sizeof(struct HTTP));
    do {
        if (GetEtaxServerURL(TAADDR_ETAX, hDev->szRegCode, &hi->modelTaAddr) != RET_SUCCESS) {
            _WriteLog(LL_WARN, "Get server url failed");
            nRet = ERR_TA_GET_SERVERADDR;
            break;
        }
        if ((nChildRet = EtaxHTTPInit(hi, hDev)) || (nChildRet = EtaxHTTPOpen(hi))) {
            _WriteLog(LL_WARN, "HTTPS Init/Open failed, nRet:%d", nChildRet);
            break;
        }
        if (!(hi->ctxApp = calloc(1, sizeof(struct ContextHttpAppBridge))))
            break;
        hi->cbHttpClose = EtaxBridgeClose;
        _WriteLog(LL_INFO, "Etax connection OK");
        nRet = RET_SUCCESS;
    } while (false);
    return nRet;
}

//有cokies会话超时时间，不可长时间开启
int QDCreateEtaxTalk(EHHTTP hi, HDEV hDev)
{
    int nRet = ERR_TA_ETAX_LOGIN, nChildRet = 0;
    char HTTPGetEventUrl[200];
    memset(HTTPGetEventUrl,0,sizeof(HTTPGetEventUrl));
    memset(hi, 0, sizeof(struct HTTP));
    do {
        if (GetTaServerURL(TAADDR_ETAX, hDev->szRegCode, &hi->modelTaAddr) != RET_SUCCESS) {
            _WriteLog(LL_WARN, "Get server url failed");
            nRet = ERR_TA_GET_SERVERADDR;
            break;
        }
        if ((nChildRet = EtaxHTTPInit(hi, hDev)) || (nChildRet = EtaxHTTPOpen(hi))) {
            _WriteLog(LL_WARN, "HTTPS Init/Open failed, nRet:%d", nChildRet);
            break;
        }
        if (!(hi->ctxApp = calloc(1, sizeof(struct ContextHttpAppBridge))))
            break;
        hi->cbHttpClose = EtaxBridgeClose;
        //获取tpass地址
        //不同省的tpass地址不同
        if(strcmp(hi->modelTaAddr.szRegCode,"31") == 0){
            if (EtaxHTTPGetEvent(hi, "/wszx-web/api/sh/login/mode") < 0)
                break;
        }
        else if(strcmp(hi->modelTaAddr.szRegCode,"51") == 0){
            sprintf(HTTPGetEventUrl,"/bszm-web/api/spcdesktop/get/kexin/login?_=%llu",GetMicroSecondUnixTimestamp());
            if (EtaxHTTPGetEvent(hi, HTTPGetEventUrl) < 0)
                break;
        }
        else {              //不支持的省份
            nRet = -1;
            break;
        }
        _WriteLog(LL_INFO, "Etax connection OK");
        nRet = RET_SUCCESS;
    } while (false);
    return nRet;
}

int TpassPasswordLoginDo(EHHTTP hiTpass, char *szUserIdIn, char *szPassword, char *szOptUserName,
                         char *szBuf, int nBufSize)
{
    char *pValue = NULL, szUserId[BS_LITTLE] = "";
    int nRet = ERR_GENERIC, nCount = 0, i = 0, nChildRet = 0;
    cJSON *root = NULL, *objItem = NULL, *objItemChild = NULL, *objItemChild2 = NULL;
    CtxBridge ctx = (CtxBridge)hiTpass->ctxApp;

    do {
        if (!(root = cJSON_Parse((const char *)evbuffer_pullup(hiTpass->bufHttpRep, -1)))) {
            _WriteLog(LL_FATAL, "cJSON_Parse failed");
            break;
        }
        if (22 != strlen(szUserIdIn)) {
            if (!strlen(szOptUserName))
                break;
            if (!(objItem = cJSON_GetObjectItem(root, "userList"))) {
                _WriteLog(LL_FATAL, "No userlist found, incorrect json reponse");
                break;
            }
            nCount = cJSON_GetArraySize(objItem);
            for (i = 0; i < nCount; i++) {
                objItemChild = cJSON_GetArrayItem(objItem, i);
                if (!(objItemChild2 = cJSON_GetObjectItem(objItemChild, "user_name")))
                    continue;
                if (strcmp(objItemChild2->valuestring, szOptUserName))
                    continue;
                if (!(objItemChild2 = cJSON_GetObjectItem(objItemChild, "user_id")))
                    continue;
                strcpy(szUserId, objItemChild2->valuestring);
                break;
            }
        } else{
            strcpy(szUserId, szUserIdIn);
        }
        //--拼接buf
        if (!(objItem = cJSON_GetObjectItem(root, "uuid"))) {
            _WriteLog(LL_FATAL, "No uuid");
            break;
        }
        pValue = objItem->valuestring;
        snprintf(szBuf, nBufSize,
                 "{\"uuid\":\"%s\",\"client_id\":\"%s\",\"user_id\":\"%s\",\"password\":\"%s\","
                 "\"redirect_uri\":\"%s\"}",
                 pValue, ctx->szClientId, szUserId, szPassword, ctx->szBuf);
        if (TpassLogicIo(hiTpass, "/sys-api/v1.0/auth/enterprise/passwordLogin", szBuf, nBufSize)) {
            _WriteLog(LL_FATAL, "PasswordLogin failed");
            break;
        }
        nRet = RET_SUCCESS;
    } while (false);
    if (root)
        cJSON_Delete(root);
    return nRet;
}

int TpassLogin(EHHTTP hiTpass, EHHTTP hiEtax, char *szUserIdIn, char *szPassword, char *szOptUserName)
{
    int nRet = ERR_GENERIC, nChildRet = 0;
    char szBuf[5120] = "";
    CtxBridge ctx = (CtxBridge)hiTpass->ctxApp;
    cJSON *root = NULL, *objItem = NULL;
    do {
        _WriteLog(LL_DEBUG, "Try to tpasslogin");
        //--校验参数
        if (strlen(szPassword) > 64) {
            nRet = SetLastError(hiTpass->hDev->hUSB, ERR_PARM_CHECK, "Password's length too big");
            break;
        }
        //--密码方式登陆
        if (TpassPasswordLoginDo(hiTpass, szUserIdIn, szPassword, szOptUserName, szBuf,
                                 sizeof(szBuf)))
            break;
        if (!(root = cJSON_Parse(szBuf)))
            break;
        if (!(objItem = cJSON_GetObjectItem(root, "code")))
            break;     
        //--获取用户权限信息
        //{"access_token":"...","refresh_token":"...","expires_in":3600,"code":"262358ADA58B457DB623E5114092CF9B","token_type":"bearer","scope":"","bz":"1","uid":"","fid":"","reg_number":""}
        if (!(root = cJSON_Parse(szBuf)))
            break;
        if (!(objItem = cJSON_GetObjectItem(root, "access_token")))
            break;
        strcpy(ctx->szToken, objItem->valuestring);
        sprintf(szBuf, "{\"access_token\":\"%s\"}", objItem->valuestring);
        if (TpassLogicIo(hiTpass, "/sys-api/v1.0/auth/oauth2/userinfo", szBuf, sizeof(szBuf)))
            break;
        evbuffer_drain(hiTpass->bufHttpRep, -1);
        evbuffer_add_printf(hiTpass->bufHttpRep, "%s", szBuf);
        //--最终刷新etax cookies完成登陆过程
        if (!(objItem = cJSON_GetObjectItem(root, "code")))
            break;
        struct evhttp_uri *uriRedirectUrl = evhttp_uri_parse(ctx->szBuf);
        if (!uriRedirectUrl)
            break;
        snprintf(szBuf, sizeof(szBuf), "%s?%s&code=%s&state=pro",
                 evhttp_uri_get_path(uriRedirectUrl), evhttp_uri_get_query(uriRedirectUrl),
                 objItem->valuestring);
        nChildRet = EtaxHTTPGetEvent(hiEtax, szBuf);
        evhttp_uri_free(uriRedirectUrl);
        if (nChildRet < 0) {
            _WriteLog(LL_FATAL, "TPass rediect to etax failed");
            break;
        }
        //给出最终的JSESSIONID,才是登陆OK
        if (!strstr((char *)evbuffer_pullup(hiEtax->ctxCon.bufCookies, -1), "JSESSIONID="))
            break;
        if (!strstr(evhttp_uri_get_path(hiEtax->ctxCon.uriLocation), "/desktop/home"))
            break; 
        _WriteLog(LL_DEBUG, "Etax login successful. Cookies:%s",
                  evbuffer_pullup(hiEtax->ctxCon.bufCookies, -1));
        nRet = RET_SUCCESS;
    } while (false);
    if (root)
        cJSON_Delete(root);
    return nRet;
}

//####urlyzm只和url有关系，和post内容和HTTP头无关；返回值在hi->bufHttpRep
int EtaxQueryInvoice(HETAX hEtax, char *szUrlPathQuery, char *szQueryJsonString)
{
    int nChildRet = ERR_GENERIC, nRet = ERR_GENERIC, nBufSize = 0;
    char szSsoToken[BS_BIG] = "", szBuf[BS_HUGE], *pBuf = NULL, *pRep = NULL;
    EHHTTP hi = &hEtax->dppt;
    CtxBridge ctx = (CtxBridge)hi->ctxApp;

    struct timeval start;
    gettimeofday(&start, NULL);
    uint64 llTimeStampMs = 1000 * (uint64)start.tv_sec + start.tv_usec / 1000;
    do {
        nBufSize = strlen(szQueryJsonString) * 6 + 1024;         //单张发票大小最多(可以修改大小) 1024 * 6 + 512
        if (!(pBuf = calloc(1, nBufSize)))
            break;
        if (GetCookiesSegment(hi->ctxCon.bufCookies, "dzfp-ssotoken", szSsoToken,
                              sizeof(szSsoToken)))
            break;
        //--根据配置IO
    
        if (GetRandUrlyzmPath(szUrlPathQuery, szSsoToken, ctx->szSecurityTokenKey, szBuf,
                            sizeof(szBuf)))
            break;

        if (GetLos28199UrlPath(szBuf, llTimeStampMs, ctx->llFingerPrint, szBuf, sizeof(szBuf)))
            break;
        if (BuildBridgePostJmbw(szUrlPathQuery, szSsoToken, szQueryJsonString, pBuf, nBufSize))
            break;
        if (GetLzkqow23819Header(szBuf, szSsoToken,
                                 (char *)evbuffer_pullup(ctx->ebJsVmpWebshellcode, -1), pBuf,
                                 llTimeStampMs, ctx->llFingerPrint, ctx->szHeaderLzkqow23819))
            break;
        hi->cbHttpNewReq = CallbackDppt;
        if ((nChildRet = EtaxHTTPPostEvent(hi, szBuf, "application/json", (uint8 *)pBuf, strlen(pBuf))) < 0) {
            break;
        }
        pRep = (char *)evbuffer_pullup(hi->bufHttpRep, -1);
        if (strstr(pRep, "\"Error\":{\"Code")) {
            if (FindStringFromEvBuffer(hi->bufHttpRep, "\"Message\":\"", "\"", szBuf,
                                       sizeof(szBuf)))
                break;
            break;
        }
        if (strstr(pRep, "{\"Response\":") != pRep) {
            break;
        }
        nRet = nChildRet;
    } while (false);
    if (pBuf)
        free(pBuf);
    _WriteLog(nRet > 0 ? LL_INFO : LL_FATAL, "EtaxQueryInvoice %s",
              nRet > 0 ? "Successful" : "Failed");
    return nRet;
}


int EtaxQueryInvoicefpdown(HETAX hEtax, char *szUrlPathQuery)
{
    int nChildRet = ERR_GENERIC, nRet = ERR_GENERIC, nBufSize = 0;
    char szSsoToken[BS_BIG] = "", szBuf[BS_HUGE], *pRep = NULL;
    EHHTTP hi = &hEtax->dppt;
    CtxBridge ctx = (CtxBridge)hi->ctxApp;
    cJSON *root = NULL, *objItem = NULL;
    uint64 llTimeStampMs = GetMicroSecondUnixTimestamp();
    do {
        if (GetCookiesSegment(hi->ctxCon.bufCookies, "dzfp-ssotoken", szSsoToken,
                              sizeof(szSsoToken)))
            break;
        strcpy(szBuf,szUrlPathQuery);
        //printf("szUrlPathQuery=%s\r\nszBuf=%s\r\n",szUrlPathQuery,szBuf);
        if (GetLos28199UrlPath(szBuf, llTimeStampMs, ctx->llFingerPrint, szBuf, sizeof(szBuf)))
            break;
        if (GetLzkqow23819Header(szBuf, szSsoToken,(char *)evbuffer_pullup(ctx->ebJsVmpWebshellcode, -1), "",llTimeStampMs, ctx->llFingerPrint, ctx->szHeaderLzkqow23819))
            break;
        hi->cbHttpNewReq = CallbackDpptfpdown;
        if ((nChildRet = EtaxHTTPGetEventdown(hi, szBuf)) <= 0) {   
            //printf("nRet error = %d\r\n",nRet);      
            break;
        }
        nRet = nChildRet;
        //printf("nRet = %d\r\n",nRet);
    } while (false);
    _WriteLog(nRet > 0 ? LL_INFO : LL_FATAL, "EtaxQueryInvoice %s",
              nRet > 0 ? "Successful" : "Failed");
    return nRet;
}

//用于返回电子税局地址
int EtaxGetEtaxServerURL(EHHTTP hi, HDEV hDev)
{
    int nRet = ERR_GENERIC, nChildRet = 0;
    char HTTPGetEventUrl[200];
    memset(HTTPGetEventUrl,0,sizeof(HTTPGetEventUrl));
    memset(hi, 0, sizeof(struct EHTTP));
    if (GetEtaxServerURL(TAADDR_ETAX, hDev->szRegCode, &hi->modelTaAddr) != RET_SUCCESS) {
        _WriteLog(LL_WARN, "Get server url failed");
        nRet = ERR_GENERIC;
    }
    nRet = RET_SUCCESS;
    return nRet;
}

int TPassLoginAuthSm4AndPublickey_tmp(EHHTTP hi,char *szRegCode,char *tem_key)
{
    CtxBridge ctx = (CtxBridge)hi->ctxApp;
    int nRet = ERR_GENERIC, nChildRet = 0;
    struct evbuffer *buf = NULL;
    char szRepBuf[2048] = "", szBuf[BS_BLOCK] = "", szUuid[BS_SMALL] = "", szPubKey[BS_BIG] = "";
    do {
        if (!(buf = evbuffer_new()))
            break;
        //GetRandHexString(ctx->szRandom16BytesKey, 16, 16, NULL);                        //获取后贯穿始终
        GetRandString(ctx->szRandom16BytesKey, 8);
        // strcat(ctx->szRandom16BytesKey, "feB!Xm2y");
        strcat(ctx->szRandom16BytesKey, "HTUurf^2");  //HTUurf^2


        printf("ctx->szRandom16BytesKey = %s\r\n",ctx->szRandom16BytesKey);
        
        strcpy(tem_key,ctx->szRandom16BytesKey);
        printf("         tem_key = [%s]       \r\n",tem_key);

        // ===============================step1--getPublicKey======================================
        if (BuildTPassFormatHMacSHA256Packet(false, 0, "{}", ctx->szRandom16BytesKey, buf))
            break;
        if (EtaxHTTPPost(hi, "/sys-api/v1.0/auth/oauth2/getPublicKey", "application/json",
                     (char *)evbuffer_pullup(buf, -1), evbuffer_get_length(buf), szRepBuf,
                     sizeof(szRepBuf)) < 0)
            break;
        if (DecodeTpassReponseJson(hi, szRepBuf, "")) {
            _WriteLog(LL_FATAL, "Try to TPassLoginAuthStep1and2 decode rep1 failed");
            break;
        }
        if (GetDataGramValueFromTPassReponse(szRepBuf, "uuid", szUuid, sizeof(szUuid)))  // 32字节
            break;
        strcpy(ctx->szUuid, szUuid);

        // =============================step2--sendSm4=============================================
        if (GetDataGramValueFromTPassReponse(szRepBuf, "publickey", szPubKey, sizeof(szPubKey)))
            break;  
        strcpy(ctx->sPublickKey,szPubKey);
        if ((nChildRet = SM2EncryptAlgorithm((uint8 *)ctx->szRandom16BytesKey, 16, szPubKey,
                                             (uint8 *)szRepBuf, sizeof(szRepBuf))) <= 0)
            break;
        if (112 != (nChildRet = SM2EncryptDataRemoveAsn1((uint8 *)szRepBuf, nChildRet)))
            break;
        Byte2Str(szBuf, (uint8 *)szRepBuf, nChildRet);
        sprintf(szRepBuf, "{\"uuid\":\"%s\",\"secret\":\"%s\"}", szUuid, szBuf);
        if (BuildTPassFormatHMacSHA256Packet(false, 0, szRepBuf, ctx->szRandom16BytesKey, buf))
            break;
        if (EtaxHTTPPost(hi, "/sys-api/v1.0/auth/white/sendSm4", "application/json",
                     (char *)evbuffer_pullup(buf, -1), evbuffer_get_length(buf), szRepBuf,
                     sizeof(szRepBuf)) < 0)
            break;
        if (DecodeTpassReponseJson(hi, szRepBuf, "")) {
            _WriteLog(LL_FATAL, "Try to TPassLoginAuthStep1and2 decode rep2 failed");
            break;
        }
        nRet = RET_SUCCESS;
    } while (false);
    if (buf)
        evbuffer_free(buf);
    return nRet;
}

//步骤1获取公钥和步骤2发送国密四，2个步骤是任何操作（查询二维码、获取地区、SSO登陆等等）的建立连接的必要操作
int TPassLoginAuthSm4AndPublickey(EHHTTP hi,char *szRegCode)
{
    CtxBridge ctx = (CtxBridge)hi->ctxApp;
    int nRet = ERR_GENERIC, nChildRet = 0;
    struct evbuffer *buf = NULL;
    char szRepBuf[2048] = "", szBuf[BS_BLOCK] = "", szUuid[BS_SMALL] = "", szPubKey[BS_BIG] = "";
    do {
        if (!(buf = evbuffer_new()))
            break;
        //GetRandHexString(ctx->szRandom16BytesKey, 16, 16, NULL);                        //获取后贯穿始终
        GetRandString(ctx->szRandom16BytesKey, 8);
        // strcat(ctx->szRandom16BytesKey, "feB!Xm2y");
        //strcat(ctx->szRandom16BytesKey, "HTUurf^2");//^bfrgM2R
        strcat(ctx->szRandom16BytesKey, "^bfrgM2R");//^bfrgM2R  //11-27
        printf("ctx->szRandom16BytesKey = %s\r\n",ctx->szRandom16BytesKey);
        // ===============================step1--getPublicKey======================================
        if (BuildTPassFormatHMacSHA256Packet(false, 0, "{}", ctx->szRandom16BytesKey, buf))
            break;
        if (EtaxHTTPPost(hi, "/sys-api/v1.0/auth/oauth2/getPublicKey", "application/json",
                     (char *)evbuffer_pullup(buf, -1), evbuffer_get_length(buf), szRepBuf,
                     sizeof(szRepBuf)) < 0)
            break;
        if (DecodeTpassReponseJson(hi, szRepBuf, "")) {
            _WriteLog(LL_FATAL, "Try to TPassLoginAuthStep1and2 decode rep1 failed");
            break;
        }
        if (GetDataGramValueFromTPassReponse(szRepBuf, "uuid", szUuid, sizeof(szUuid)))  // 32字节
            break;
        strcpy(ctx->szUuid, szUuid);
        // =============================step2--sendSm4=============================================
        if (GetDataGramValueFromTPassReponse(szRepBuf, "publickey", szPubKey, sizeof(szPubKey)))
            break;  
        strcpy(ctx->sPublickKey,szPubKey);
        if ((nChildRet = SM2EncryptAlgorithm((uint8 *)ctx->szRandom16BytesKey, 16, szPubKey,
                                             (uint8 *)szRepBuf, sizeof(szRepBuf))) <= 0)
            break;
        if (112 != (nChildRet = SM2EncryptDataRemoveAsn1((uint8 *)szRepBuf, nChildRet)))
            break;
        Byte2Str(szBuf, (uint8 *)szRepBuf, nChildRet);
        sprintf(szRepBuf, "{\"uuid\":\"%s\",\"secret\":\"%s\"}", szUuid, szBuf);
        if (BuildTPassFormatHMacSHA256Packet(false, 0, szRepBuf, ctx->szRandom16BytesKey, buf))
            break;
        if (EtaxHTTPPost(hi, "/sys-api/v1.0/auth/white/sendSm4", "application/json",
                     (char *)evbuffer_pullup(buf, -1), evbuffer_get_length(buf), szRepBuf,
                     sizeof(szRepBuf)) < 0)
            break;
        if (DecodeTpassReponseJson(hi, szRepBuf, "")) {
            _WriteLog(LL_FATAL, "Try to TPassLoginAuthStep1and2 decode rep2 failed");
            break;
        }
        nRet = RET_SUCCESS;
    } while (false);
    if (buf)
        evbuffer_free(buf);
    return nRet;
}

//给etax的单点登陆认证
int TPassLoginAuthHelloAndAuth(EHHTTP hi, struct evhttp_uri *uriTpassFull)
{
    int nRet = ERR_GENERIC, nChildRet = 0;
    struct evbuffer *buf = NULL;
    CtxBridge ctx = (CtxBridge)hi->ctxApp;
    //可变重复用buf，szRepBuf最大接收7k数据,szBuf最大接收3k+数据
    char szRepBuf[10*9216] = "", szBuf[5120] = "";
    do {
        if (!(buf = evbuffer_new()))
            break;
        // =============================step3--serverHello=============================================
        memset(szBuf, 0, sizeof(szBuf));
        if (GetTaClientHello(1, hi->hDev, szBuf) < 0) {
            nRet = -1;
            break;
        }
        sprintf(szRepBuf, "{\"server_client_hello\":\"%s\"}", szBuf);
        if (BuildTPassFormatHMacSHA256Packet(true, 2, szRepBuf, ctx->szRandom16BytesKey, buf))
            break;
        //需要在回调函数中使用hi->ctxApp.szTmpBuf中保存的uuid
        hi->cbHttpNewReq = CallbackTPassSetHttpHeaderUuid;
        if (EtaxHTTPPost(hi, "/sys-api/v1.0/auth/cert/serverHello", "application/json",
                     (char *)evbuffer_pullup(buf, -1), evbuffer_get_length(buf), szRepBuf,
                     sizeof(szRepBuf)) < 0) {
            nRet = -2;
            break;
        }
        if (DecodeTpassReponseJson(hi, szRepBuf, "")) {
            nRet = -3;
            break;
        }
        // =============================step4--serverAuth=============================================
        if (TPassLoginAuthDecodeStep3RepForStep4(hi, uriTpassFull, ctx->szRandom16BytesKey,
                                                 szRepBuf, sizeof(szRepBuf))) {
            nRet = -4;
            break;
        }
        //printf("lbc-debug TPassLoginAuthSso szRepBuf=%s\n",szRepBuf);
        if (TpassLogicIo(hi, "/sys-api/v1.0/auth/cert/serverAuth", szRepBuf, sizeof(szRepBuf))) {
            nRet = -5;
            break;
        }
        evbuffer_drain(hi->bufHttpRep, -1);
        evbuffer_add_printf(hi->bufHttpRep, "%s", szRepBuf);
        //_WriteHexToDebugFile("user.json", (uint8 *)szRepBuf, strlen(szRepBuf));
        nRet = RET_SUCCESS;
    } while (false);
    if (buf)
        evbuffer_free(buf);
    return nRet;
}

//登陆主框架，就是用户选择各种子功能的页面
int TpassLoginMainFrame(EHHTTP hiTpass, EHHTTP hiEtax, char *szToken, char *szCode,
                        char *szMainPageDirectUrl)
{
    int nRet = ERR_GENERIC, nChildRet = 0;
    char szBuf[5120] = "";
    CtxBridge ctx = (CtxBridge)hiTpass->ctxApp;
    do {
        strcpy(ctx->szToken, szToken);
        sprintf(szBuf, "{\"access_token\":\"%s\"}", szToken);
        if (TpassLogicIo(hiTpass, "/sys-api/v1.0/auth/oauth2/userinfo", szBuf, sizeof(szBuf)))
            break;
        evbuffer_drain(hiTpass->bufHttpRep, -1);
        evbuffer_add_printf(hiTpass->bufHttpRep, "%s", szBuf);
        //--最终刷新etax cookies完成登陆过程
        struct evhttp_uri *uriRedirectUrl = evhttp_uri_parse(szMainPageDirectUrl);
        if (!uriRedirectUrl)
            break;

        snprintf(szBuf, sizeof(szBuf), "%s?%s&code=%s&state=pro",
                 evhttp_uri_get_path(uriRedirectUrl), evhttp_uri_get_query(uriRedirectUrl), szCode);
        evhttp_uri_free(uriRedirectUrl);
        if (EtaxHTTPGetEvent(hiEtax, szBuf) < 0) {
            _WriteLog(LL_FATAL, "TPass rediect to etax failed");
            break;
        }
        //给出最终的JSESSIONID,才是登陆OK
        if (!strstr((char *)evbuffer_pullup(hiEtax->ctxCon.bufCookies, -1), "JSESSIONID="))
            break;
        // if (!strstr(evhttp_uri_get_path(hiEtax->ctxCon.uriLocation), "/desktop/home")) zwl 1012
        //     break;
        nRet = RET_SUCCESS;
    } while (false);
    return nRet;
}

int TPassLoginPasswordLoginDo(EHHTTP hiTpass, char *szUserIdIn, char *szPassword,char *szBuf, int nBufSize)
{
    char *pValue = NULL, szUserId[BS_LITTLE] = "";
    int nRet = ERR_GENERIC, nCount = 0, i = 0, nChildRet = 0;
    cJSON *root = NULL, *objItem = NULL, *objItemChild = NULL, *objItemChild2 = NULL;
    CtxBridge ctx = (CtxBridge)hiTpass->ctxApp;

    do {
        if (!(root = cJSON_Parse((const char *)evbuffer_pullup(hiTpass->bufHttpRep, -1)))) {
            _WriteLog(LL_FATAL, "cJSON_Parse failed");
            break;
        }
        strcpy(szUserId, szUserIdIn);
        if (!(objItem = cJSON_GetObjectItem(root, "uuid"))) {
            _WriteLog(LL_FATAL, "No uuid");
            break;
        }
        pValue = objItem->valuestring;
        snprintf(szBuf, nBufSize,
                 "{\"uuid\":\"%s\",\"client_id\":\"%s\",\"user_id\":\"%s\",\"password\":\"%s\","
                 "\"redirect_uri\":\"%s\"}",
                 pValue, ctx->szClientId, szUserId, szPassword, ctx->szBuf);
        if (TpassLogicIo(hiTpass, "/sys-api/v1.0/auth/enterprise/passwordLogin", szBuf, nBufSize)) {
            _WriteLog(LL_FATAL, "PasswordLogin failed");
            break;
        }
        nRet = RET_SUCCESS;
    } while (false);
    if (root)
        cJSON_Delete(root);
    return nRet;
}

int TPassLoginPasswordLoginDoGetCookies_shanghai(EHHTTP hiTpass, EHHTTP hiEtax,char *szUserIdIn, char *szPassword,char *szBuf, int nBufSize,char *out_token)
{
    char *pValue = NULL, szUserId[BS_LITTLE] = "";
    int nRet = ERR_GENERIC, nCount = 0, i = 0, nChildRet = 0;
    cJSON *root = NULL, *objItem = NULL, *objItemChild = NULL, *objItemChild2 = NULL, *objItemToken = NULL, *objItemCode = NULL;
    CtxBridge ctx = (CtxBridge)hiTpass->ctxApp;

    do {
        if (!(root = cJSON_Parse((const char *)evbuffer_pullup(hiTpass->bufHttpRep, -1)))) {
            _WriteLog(LL_FATAL, "cJSON_Parse failed");
            break;
        }
        strcpy(szUserId, szUserIdIn);
        if (!(objItem = cJSON_GetObjectItem(root, "uuid"))) {
            _WriteLog(LL_FATAL, "No uuid");
            break;
        }
        pValue = objItem->valuestring;
        snprintf(szBuf, nBufSize,
                 "{\"uuid\":\"%s\",\"client_id\":\"%s\",\"user_id\":\"%s\",\"password\":\"%s\","
                 "\"redirect_uri\":\"%s\"}",
                 pValue, ctx->szClientId, szUserId, szPassword, ctx->szBuf);
        if (TpassLogicIo(hiTpass, "/sys-api/v1.0/auth/enterprise/passwordLogin", szBuf, nBufSize)) {
            _WriteLog(LL_FATAL, "PasswordLogin failed");
            break;
        }

        if (root)
            cJSON_Delete(root);

        if (!(root = cJSON_Parse(szBuf)))
            break;
        if (!(objItemToken = cJSON_GetObjectItem(root, "access_token")))
            break;
        //--最终刷新etax cookies完成登陆过程
        if (!(objItemCode = cJSON_GetObjectItem(root, "code")))
            break;
        if (TpassLoginMainFrame(hiTpass, hiEtax, objItemToken->valuestring,
                                objItemCode->valuestring, ctx->szBuf))
            break;

        strcpy(out_token,objItemToken->valuestring);
        //printf("    out_token    =  %s    \r\n",out_token);

        nRet = RET_SUCCESS;
    } while (false);
    if (root)
        cJSON_Delete(root);
    return nRet;
}

int TPassLoginPasswordLoginDoGetCookies(EHHTTP hiTpass, EHHTTP hiEtax,char *szUserIdIn, char *szPassword,char *szBuf, int nBufSize)
{
    char *pValue = NULL, szUserId[BS_LITTLE] = "";
    int nRet = ERR_GENERIC, nCount = 0, i = 0, nChildRet = 0;
    cJSON *root = NULL, *objItem = NULL, *objItemChild = NULL, *objItemChild2 = NULL, *objItemToken = NULL, *objItemCode = NULL;
    CtxBridge ctx = (CtxBridge)hiTpass->ctxApp;

    do {
        if (!(root = cJSON_Parse((const char *)evbuffer_pullup(hiTpass->bufHttpRep, -1)))) {
            _WriteLog(LL_FATAL, "cJSON_Parse failed");
            break;
        }
        strcpy(szUserId, szUserIdIn);
        if (!(objItem = cJSON_GetObjectItem(root, "uuid"))) {
            _WriteLog(LL_FATAL, "No uuid");
            break;
        }
        pValue = objItem->valuestring;
        snprintf(szBuf, nBufSize,
                 "{\"uuid\":\"%s\",\"client_id\":\"%s\",\"user_id\":\"%s\",\"password\":\"%s\","
                 "\"redirect_uri\":\"%s\"}",
                 pValue, ctx->szClientId, szUserId, szPassword, ctx->szBuf);
        if (TpassLogicIo(hiTpass, "/sys-api/v1.0/auth/enterprise/passwordLogin", szBuf, nBufSize)) {
            _WriteLog(LL_FATAL, "PasswordLogin failed");
            break;
        }

        if (root)
            cJSON_Delete(root);

        if (!(root = cJSON_Parse(szBuf)))
            break;
        if (!(objItemToken = cJSON_GetObjectItem(root, "access_token")))
            break;
        //--最终刷新etax cookies完成登陆过程
        if (!(objItemCode = cJSON_GetObjectItem(root, "code")))
            break;
        if (TpassLoginMainFrame(hiTpass, hiEtax, objItemToken->valuestring,
                                objItemCode->valuestring, ctx->szBuf))
            break;


        nRet = RET_SUCCESS;
    } while (false);
    if (root)
        cJSON_Delete(root);
    return nRet;
}

int TPassSelectRelationList(EHHTTP hiTpass, char **outenterpriselist,char *szBuf, int nBufSize)
{
    char *pValue = NULL, szUserId[BS_LITTLE] = "";
    int nRet = ERR_GENERIC, nCount = 0, i = 0, nChildRet = 0;
    cJSON *root = NULL, *objItem = NULL, *objItemChild = NULL, *objItemChild2 = NULL;
    CtxBridge ctx = (CtxBridge)hiTpass->ctxApp;

    do {
        if (!(root = cJSON_Parse(szBuf)))
            break;
        if (!(objItem = cJSON_GetObjectItem(root, "access_token")))
            break;
        strcpy(ctx->szToken, objItem->valuestring);
        sprintf(szBuf, "{\"access_token\":\"%s\"}", objItem->valuestring);
        if (TpassLogicIo(hiTpass, "/sys-api/v1.0/auth/oauth2/userinfo", szBuf,nBufSize)) {
            break;
        }

        if (!(root = cJSON_Parse(szBuf)))
            break;
        if (!(objItem = cJSON_GetObjectItem(root, "reg_number")))
            break;
        if (!(objItemChild = cJSON_GetObjectItem(root, "enterprise_id")))
            break;
        if (!(objItemChild2 = cJSON_GetObjectItem(root, "user_id")))
            break;

        sprintf(szBuf,"{\"reg_number\":\"%s\",\"fid\":\"%s\",\"uid\":\"%s\",\"uniqueIdentity\":\"\",\"optype\":\"1\",\"queryScene\":\"112\",\"relatedStatus\":\"\",\"maskingFlag\":\"0\",\"enterpriseName\":\"\",\"pageNo\":1,\"pageSize\":1000}",objItem->valuestring,objItemChild->valuestring,objItemChild2->valuestring);
        
        if (TpassLogicIo(hiTpass, "/sys-api/v1.0/idm/internal/relation/selectRelationList", szBuf, nBufSize)){
            break;
        }

        nChildRet = evbuffer_get_length(hiTpass->bufHttpRep);        
        // printf("selectRelationList nChildRet =%d\r\n  ____________szBuf = %d\r\n",nChildRet,sizeof(szBuf));        
        
        printf("################################______strlen(szBuf) = %d   #################______nBufSize = %d\r\n",strlen(szBuf),nBufSize);
        // if (strlen(szBuf) >= nBufSize) {
        //     nRet = ERR_BUF_OVERFLOW;
        //     _WriteLog(LL_FATAL, "Buffer overflow, small buf,nChildRet=%d,repBufferLen=%d",nChildRet,repBufferLen);
        //     break;
        // }

        *outenterpriselist = NULL;
        *outenterpriselist = (char *)malloc(nChildRet);
        if(*outenterpriselist == NULL){    
            break;
        }
        
        EncordingConvert( "utf-8","gbk",szBuf,nChildRet, *outenterpriselist,nChildRet);
        // printf("selectRelationList nChildRet =%d\r\n  ____________szBuf = %d\r\n ___szBuf = %s\r\n___## strlen(szBuf) = %d\r\n",nChildRet,sizeof(szBuf),szBuf,strlen(szBuf));
        nRet = RET_SUCCESS;
    } while (false);
    if (root)
        cJSON_Delete(root);
    return nRet;
}

int TPassSelectRelationchange(EHHTTP hiTpass,char *uniqueIdentity,char *realationStatus,char *szBuf, int nBufSize)
{
    char *pValue = NULL, szUserId[BS_LITTLE] = "";
    int nRet = ERR_GENERIC, nCount = 0, i = 0, nChildRet = 0;
    cJSON *root = NULL, *objItem = NULL, *objItemChild = NULL, *objItemChild2 = NULL;
    CtxBridge ctx = (CtxBridge)hiTpass->ctxApp;

    do {
        if (!(root = cJSON_Parse(szBuf)))
            break;
        if (!(objItem = cJSON_GetObjectItem(root, "access_token")))
            break;
        strcpy(ctx->szToken, objItem->valuestring);

        sprintf(szBuf,"{\"creditCode\":\"%s\",\"relatedType\":\"%s\",\"agencyCreditCode\":\"\"}",uniqueIdentity,realationStatus);
        //printf("szBuf=%s\r\n",szBuf);
        if (TpassLogicIo(hiTpass, "/sys-api/v1.0/auth/oauth2/changeUser", szBuf, nBufSize)) 
            break;               

        nChildRet = evbuffer_get_length(hiTpass->bufHttpRep);        
        // printf("changeUser nChildRet =%d\r\n%s\r\n",nChildRet,szBuf);        

        nRet = RET_SUCCESS;
    } while (false);
    if (root)
        cJSON_Delete(root);
    return nRet;    
}

int TPassSelectRelationchangeGetCookies_shanghai(EHHTTP hiTpass,EHHTTP hiEtax,char *szUserIdIn, char *szPassword,char *uniqueIdentity,char *realationStatus,char *szBuf, int nBufSize,char *out_token)
{
    char *pValue = NULL, szUserId[BS_LITTLE] = "";
    int nRet = ERR_GENERIC, nCount = 0, i = 0, nChildRet = 0;
    cJSON *root = NULL, *objItem = NULL, *objItemChild = NULL, *objItemChild2 = NULL, *objItemToken = NULL, *objItemCode = NULL;
    CtxBridge ctx = (CtxBridge)hiTpass->ctxApp;

    do {
        if (!(root = cJSON_Parse((const char *)evbuffer_pullup(hiTpass->bufHttpRep, -1)))) {
            _WriteLog(LL_FATAL, "cJSON_Parse failed");
            break;
        }
        strcpy(szUserId, szUserIdIn);
        if (!(objItem = cJSON_GetObjectItem(root, "uuid"))) {
            _WriteLog(LL_FATAL, "No uuid");
            break;
        }
        pValue = objItem->valuestring;
        snprintf(szBuf, nBufSize,
                 "{\"uuid\":\"%s\",\"client_id\":\"%s\",\"user_id\":\"%s\",\"password\":\"%s\","
                 "\"redirect_uri\":\"%s\"}",
                 pValue, ctx->szClientId, szUserId, szPassword, ctx->szBuf);
        if (TpassLogicIo(hiTpass, "/sys-api/v1.0/auth/enterprise/passwordLogin", szBuf, nBufSize)) {
            _WriteLog(LL_FATAL, "PasswordLogin failed");
            break;
        }

        if (!(root = cJSON_Parse(szBuf)))
            break;
        if (!(objItem = cJSON_GetObjectItem(root, "access_token")))
            break;
        strcpy(ctx->szToken, objItem->valuestring);
        sprintf(szBuf,"{\"creditCode\":\"%s\",\"relatedType\":\"%s\",\"agencyCreditCode\":\"\"}",uniqueIdentity,realationStatus);
        // printf("    zwl            szBuf=%s\r\n",szBuf);
        if (TpassLogicIo(hiTpass, "/sys-api/v1.0/auth/oauth2/changeUser", szBuf, nBufSize)) 
            break;               
        nChildRet = evbuffer_get_length(hiTpass->bufHttpRep);        
        printf("changeUser nChildRet =%d\r\n%s\r\n",nChildRet,szBuf);        

        if (root)
            cJSON_Delete(root);
        if (!(root = cJSON_Parse(szBuf)))
            break;
        if (!(objItemToken = cJSON_GetObjectItem(root, "access_token")))
            break;
        //--最终刷新etax cookies完成登陆过程
        if (!(objItemCode = cJSON_GetObjectItem(root, "code")))
            break;
        if (TpassLoginMainFrame(hiTpass, hiEtax, objItemToken->valuestring,
                               objItemCode->valuestring, ctx->szBuf))
           break;
        strcpy(out_token,objItemToken->valuestring);
        //printf("    out_token    =  %s    \r\n",out_token);

        nRet = RET_SUCCESS;
    } while (false);
    if (root)
        cJSON_Delete(root);
    return nRet;    
}

int TPassSelectRelationchangeGetCookies(EHHTTP hiTpass,EHHTTP hiEtax,char *szUserIdIn, char *szPassword,char *uniqueIdentity,char *realationStatus,char *szBuf, int nBufSize)
{
    char *pValue = NULL, szUserId[BS_LITTLE] = "";
    int nRet = ERR_GENERIC, nCount = 0, i = 0, nChildRet = 0;
    cJSON *root = NULL, *objItem = NULL, *objItemChild = NULL, *objItemChild2 = NULL, *objItemToken = NULL, *objItemCode = NULL;
    CtxBridge ctx = (CtxBridge)hiTpass->ctxApp;

    do {

        if (!(root = cJSON_Parse((const char *)evbuffer_pullup(hiTpass->bufHttpRep, -1)))) {
            _WriteLog(LL_FATAL, "cJSON_Parse failed");
            break;
        }
        strcpy(szUserId, szUserIdIn);
        if (!(objItem = cJSON_GetObjectItem(root, "uuid"))) {
            _WriteLog(LL_FATAL, "No uuid");
            break;
        }
        pValue = objItem->valuestring;
        snprintf(szBuf, nBufSize,
                 "{\"uuid\":\"%s\",\"client_id\":\"%s\",\"user_id\":\"%s\",\"password\":\"%s\","
                 "\"redirect_uri\":\"%s\"}",
                 pValue, ctx->szClientId, szUserId, szPassword, ctx->szBuf);
        if (TpassLogicIo(hiTpass, "/sys-api/v1.0/auth/enterprise/passwordLogin", szBuf, nBufSize)) {
            _WriteLog(LL_FATAL, "PasswordLogin failed");
            break;
        }


        if (!(root = cJSON_Parse(szBuf)))
            break;
        if (!(objItem = cJSON_GetObjectItem(root, "access_token")))
            break;
        strcpy(ctx->szToken, objItem->valuestring);

        sprintf(szBuf,"{\"creditCode\":\"%s\",\"relatedType\":\"%s\",\"agencyCreditCode\":\"\"}",uniqueIdentity,realationStatus);
        // printf("szBuf=%s\r\n",szBuf);
        if (TpassLogicIo(hiTpass, "/sys-api/v1.0/auth/oauth2/changeUser", szBuf, nBufSize)) 
            break;               

        nChildRet = evbuffer_get_length(hiTpass->bufHttpRep);        
        // printf("changeUser nChildRet =%d\r\n%s\r\n",nChildRet,szBuf);        

        if (root)
            cJSON_Delete(root);

        if (!(root = cJSON_Parse(szBuf)))
            break;
        if (!(objItemToken = cJSON_GetObjectItem(root, "access_token")))
            break;
        //--最终刷新etax cookies完成登陆过程
        if (!(objItemCode = cJSON_GetObjectItem(root, "code")))
            break;
        if (TpassLoginMainFrame(hiTpass, hiEtax, objItemToken->valuestring,
                               objItemCode->valuestring, ctx->szBuf))
           break;

        nRet = RET_SUCCESS;
    } while (false);
    if (root)
        cJSON_Delete(root);
    return nRet;    
}



int TPassLoginUserInfo(EHHTTP hiTpass, EHHTTP hiEtax,char *errinfo_uft8,char *szBuf,int nBufSize)
{
    char *pValue = NULL, szUserId[BS_LITTLE] = "";
    int nRet = ERR_GENERIC, nCount = 0, i = 0, nChildRet = 0;
    cJSON *root = NULL, *objItem = NULL, *objItemChild = NULL, *objItemChild2 = NULL;
    CtxBridge ctx = (CtxBridge)hiTpass->ctxApp;
    do {
        if (!(root = cJSON_Parse(szBuf)))
            break;
        if (!(objItem = cJSON_GetObjectItem(root, "code")))
            break;  
        if (!(root = cJSON_Parse(szBuf)))
            break;
        if (!(objItem = cJSON_GetObjectItem(root, "access_token")))
            break;
        
        strcpy(ctx->szToken, objItem->valuestring);
        sprintf(szBuf, "{\"access_token\":\"%s\"}", objItem->valuestring);

        //请求企业用户信息失败
        if (TpassLogicIo(hiTpass, "/sys-api/v1.0/auth/oauth2/userinfo", szBuf, nBufSize)){
            sprintf((char *)errinfo_uft8, "406获取企业用户信息失败");
            break;
        }
        evbuffer_drain(hiTpass->bufHttpRep, -1);
        evbuffer_add_printf(hiTpass->bufHttpRep, "%s", szBuf);

        //最终刷新etax cookies完成登陆过程
        if (!(objItem = cJSON_GetObjectItem(root, "code"))){
            sprintf((char *)errinfo_uft8, "406获取企业用户信息成功,但返回数据解析失败");
            break;
        }

        struct evhttp_uri *uriRedirectUrl = evhttp_uri_parse(ctx->szBuf);
        if (!uriRedirectUrl){
            sprintf((char *)errinfo_uft8, "406获取企业用户信息成功,但返回数据解析失败");
            break;
        }

        snprintf(szBuf,nBufSize, "%s?%s&code=%s&state=pro",evhttp_uri_get_path(uriRedirectUrl), evhttp_uri_get_query(uriRedirectUrl),objItem->valuestring);

        nChildRet = EtaxHTTPGetEvent(hiEtax, szBuf);
        evhttp_uri_free(uriRedirectUrl);
        if (nChildRet < 0) {
            sprintf((char *)errinfo_uft8, "406刷新跳转税局地址失败");
            break;
        }

        //给出最终的JSESSIONID,才是登陆OK
        if (!strstr((char *)evbuffer_pullup(hiEtax->ctxCon.bufCookies, -1), "JSESSIONID=")){
            sprintf((char *)errinfo_uft8, "406刷新跳转税局地址失败,%s",(char *)evbuffer_pullup(hiEtax->ctxCon.bufCookies, -1));
            break;
        }
            
        if (!strstr(evhttp_uri_get_path(hiEtax->ctxCon.uriLocation), "/desktop/home")){
            sprintf((char *)errinfo_uft8, "406刷新跳转税局地址失败,%s",evhttp_uri_get_path(hiEtax->ctxCon.uriLocation));
            break;
        }
        nRet = RET_SUCCESS;
    } while (false);
    if (root)
        cJSON_Delete(root);
    return nRet;
}

int TPassLoginUserInfoChange(EHHTTP hiTpass, EHHTTP hiEtax,char *uniqueIdentity,char *realationStatus,char *errinfo_uft8,char *szBuf,int nBufSize)
{
    char *pValue = NULL, szUserId[BS_LITTLE] = "";
    int nRet = ERR_GENERIC, nCount = 0, i = 0, nChildRet = 0;
    cJSON *root = NULL, *objItem = NULL, *objItemChild = NULL, *objItemChild2 = NULL;
    CtxBridge ctx = (CtxBridge)hiTpass->ctxApp;
    do {
        if (!(root = cJSON_Parse(szBuf)))
            break;
        if (!(objItem = cJSON_GetObjectItem(root, "code")))
            break;  
        if (!(root = cJSON_Parse(szBuf)))
            break;
        if (!(objItem = cJSON_GetObjectItem(root, "access_token")))
            break;
        
        strcpy(ctx->szToken, objItem->valuestring);

        /*sprintf(szBuf, "{\"access_token\":\"%s\"}", objItem->valuestring);
        printf("/sys-api/v1.0/auth/oauth2/userinfo url =%s\r\n",szBuf);
        //请求企业用户信息失败
        if (TpassLogicIo(hiTpass, "/sys-api/v1.0/auth/oauth2/userinfo", szBuf, nBufSize)){
            sprintf((char *)errinfo_uft8, "406获取企业用户信息失败");
            break;
        }
        evbuffer_drain(hiTpass->bufHttpRep, -1);
        evbuffer_add_printf(hiTpass->bufHttpRep, "%s", szBuf);*/

        sprintf(szBuf,"{\"creditCode\":\"%s\",\"relatedType\":\"%s\",\"agencyCreditCode\":\"\"}",uniqueIdentity,realationStatus);
        if (TpassLogicIo(hiTpass, "/sys-api/v1.0/auth/oauth2/changeUser", szBuf,nBufSize)){
            printf("TpassLogicIo szBuf=%s\r\n",szBuf);
            if (!(root = cJSON_Parse(szBuf)))
                break;
            if (!(objItem = cJSON_GetObjectItem(root, "msg")))
                break;
            sprintf((char *)errinfo_uft8, "406企业切换失败,%s",objItem->valuestring);
            break; 
        }        
        if (!(root = cJSON_Parse(szBuf)))
            break;
        if (!(objItem = cJSON_GetObjectItem(root, "access_token")))
            break;
        strcpy(ctx->szToken, objItem->valuestring);
        sprintf(szBuf, "{\"access_token\":\"%s\"}", objItem->valuestring);


        //最终刷新etax cookies完成登陆过程
        if (!(objItem = cJSON_GetObjectItem(root, "code"))){
            sprintf((char *)errinfo_uft8, "406获取企业用户信息成功,但返回数据解析失败");
            break;
        }

        struct evhttp_uri *uriRedirectUrl = evhttp_uri_parse(ctx->szBuf);
        if (!uriRedirectUrl){
            sprintf((char *)errinfo_uft8, "406获取企业用户信息成功,但返回数据解析失败");
            break;
        }

        snprintf(szBuf,nBufSize, "%s?%s&code=%s&state=pro",evhttp_uri_get_path(uriRedirectUrl), evhttp_uri_get_query(uriRedirectUrl),objItem->valuestring);

        nChildRet = EtaxHTTPGetEvent(hiEtax, szBuf);
        evhttp_uri_free(uriRedirectUrl);
        if (nChildRet < 0) {
            sprintf((char *)errinfo_uft8, "406刷新跳转税局地址失败");
            break;
        }
        //给出最终的JSESSIONID,才是登陆OK
        if (!strstr((char *)evbuffer_pullup(hiEtax->ctxCon.bufCookies, -1), "JSESSIONID=")){
            sprintf((char *)errinfo_uft8, "406刷新跳转税局地址失败,%s",(char *)evbuffer_pullup(hiEtax->ctxCon.bufCookies, -1));
            break;
        }     
        if (!strstr(evhttp_uri_get_path(hiEtax->ctxCon.uriLocation), "/desktop/home")){
            sprintf((char *)errinfo_uft8, "406刷新跳转税局地址失败,%s",evhttp_uri_get_path(hiEtax->ctxCon.uriLocation));
            break;
        }
        nRet = RET_SUCCESS;
    } while (false);
    if (root)
        cJSON_Delete(root);
    return nRet;
}

int TPassLoginPasswordLoginDoShanXi(EHHTTP hiTpass, char *szUserIdIn, char *szPassword,char *szBuf, int nBufSize)
{
    char *pValue = NULL, szUserId[BS_LITTLE] = "";
    char pDecodeEtaxUrl[BS_NORMAL] = {0};
    int nRet = ERR_GENERIC, nCount = 0, i = 0, nChildRet = 0;
    cJSON *root = NULL, *objItem = NULL, *objItemChild = NULL, *objItemChild2 = NULL;
    CtxBridge ctx = (CtxBridge)hiTpass->ctxApp;

    do {
        if (!(root = cJSON_Parse((const char *)evbuffer_pullup(hiTpass->bufHttpRep, -1)))) {
            _WriteLog(LL_FATAL, "cJSON_Parse failed");
            break;
        }
        strcpy(szUserId, szUserIdIn);
        if (!(objItem = cJSON_GetObjectItem(root, "uuid"))) {
            _WriteLog(LL_FATAL, "No uuid");
            break;
        }
        pValue = objItem->valuestring;

        strcpy(pDecodeEtaxUrl,"https://etax.shanxi.chinatax.gov.cn/tpass/sso/loginSubmit");
        snprintf(szBuf, nBufSize,
                 "{\"uuid\":\"%s\",\"client_id\":\"%s\",\"user_id\":\"%s\",\"password\":\"%s\","
                 "\"redirect_uri\":\"%s\"}",
                 pValue, ctx->szClientId, szUserId, szPassword, pDecodeEtaxUrl);
        if (TpassLogicIo(hiTpass, "/sys-api/v1.0/auth/enterprise/passwordLogin", szBuf, nBufSize)) {
            _WriteLog(LL_FATAL, "PasswordLogin failed");
            break;
        }
        nRet = RET_SUCCESS;
    } while (false);
    if (root)
        cJSON_Delete(root);
    return nRet;
}

//给etax的单点登陆认证
int TPassLoginAuthHelloAndAuthShanXi(EHHTTP hi, struct evhttp_uri *uriTpassFull)
{
    int nRet = ERR_GENERIC, nChildRet = 0;
    struct evbuffer *buf = NULL;
    CtxBridge ctx = (CtxBridge)hi->ctxApp;
    //可变重复用buf，szRepBuf最大接收7k数据,szBuf最大接收3k+数据
    char szRepBuf[9216] = "", szBuf[5120] = "";
    do {
        if (!(buf = evbuffer_new()))
            break;
        // =============================step3--serverHello=============================================
        memset(szBuf, 0, sizeof(szBuf));
        if (GetTaClientHello(1, hi->hDev, szBuf) < 0) {
            nRet = -1;
            break;
        }
        sprintf(szRepBuf, "{\"server_client_hello\":\"%s\"}", szBuf);
        if (BuildTPassFormatHMacSHA256Packet(true, 2, szRepBuf, ctx->szRandom16BytesKey, buf))
            break;
        //需要在回调函数中使用hi->ctxApp.szTmpBuf中保存的uuid
        hi->cbHttpNewReq = CallbackTPassSetHttpHeaderUuid;
        if (EtaxHTTPPost(hi, "/sys-api/v1.0/auth/cert/serverHello", "application/json",
                     (char *)evbuffer_pullup(buf, -1), evbuffer_get_length(buf), szRepBuf,
                     sizeof(szRepBuf)) < 0) {
            nRet = -2;
            break;
        }
        if (DecodeTpassReponseJson(hi, szRepBuf, "")) {
            nRet = -3;
            break;
        }
        // =============================step4--serverAuth=============================================
        if (TPassLoginAuthDecodeStep3RepForStep4ShanXi(hi, uriTpassFull, ctx->szRandom16BytesKey,
                                                 szRepBuf, sizeof(szRepBuf))) {
            nRet = -4;
            break;
        }
        //printf("lbc-debug TPassLoginAuthSso szRepBuf=%s\n",szRepBuf);
        if (TpassLogicIo(hi, "/sys-api/v1.0/auth/cert/serverAuth", szRepBuf, sizeof(szRepBuf))) {
            nRet = -5;
            break;
        }
        evbuffer_drain(hi->bufHttpRep, -1);
        evbuffer_add_printf(hi->bufHttpRep, "%s", szRepBuf);
        //_WriteHexToDebugFile("user.json", (uint8 *)szRepBuf, strlen(szRepBuf));
        nRet = RET_SUCCESS;
    } while (false);
    if (buf)
        evbuffer_free(buf);
    return nRet;
}


int TPassLoginAuthDecodeStep3RepForStep4ShanXi(EHHTTP hi, struct evhttp_uri *uriTpassFull,
                                         char *szRandomHex16Bytes, char *szRepBuf, int nRepBufSize)
{
    int nRet = ERR_GENERIC, nChildRet = 0;
    cJSON *root = NULL;
    CtxBridge ctx = (CtxBridge)hi->ctxApp;
    char szBuf[5120] = "", szUuid[BS_LITTLE] = "", *pValue = NULL, *pDecodeEtaxUrl = NULL,
         szClientIdFlag[] = "&client_id=";
    do {
        if (GetDataGramValueFromTPassReponse(szRepBuf, "", szRepBuf, nRepBufSize)){
            printf("----1----\n");
            break;
        }

        nChildRet = Str2Byte((uint8 *)szRepBuf, szRepBuf, strlen(szRepBuf));
        if (!(nChildRet = EtaxSM4Algorithm(false, 1, (uint8 *)szRepBuf, nChildRet,
                                       (uint8 *)szRandomHex16Bytes, (uint8 *)szRepBuf))){
            printf("----2----\n");
            break;
        }
        //解出 新的uuid，serverHelloResult
        if (!(root = cJSON_Parse(szRepBuf))){
            printf("----3----\n");
            break;
        }
        if (!(pValue = cJSON_GetObjectItem(root, "uuid")->valuestring)){
            printf("----4----\n");
            break;
        }
        strcpy(szUuid, pValue);  //新的业务uuid
        if (!(pValue = cJSON_GetObjectItem(root, "serverHelloResult")->valuestring)){
            printf("----5----\n");
            break;
        }
        strcpy(szRepBuf, pValue);
        //生成认证代码
        nChildRet = nRepBufSize;
        memset(szBuf, 0, sizeof(szBuf));
        if (BuildClientAuthCode((uint8 *)szRepBuf, strlen(szRepBuf), (uint8 *)szBuf, &nChildRet,
                                hi->hDev)){
            printf("----6----\n");
            break;
        }/*
        if (!(pValue = (char *)evhttp_uri_get_query(uriTpassFull))){
            printf("---pValue=[]---",pValue);
            printf("----7----\n");
            break;
        }
        pValue = strchr(pValue, '=') + 1;*/
        /*if (!(pDecodeEtaxUrl = evhttp_decode_uri(pValue)))
            break;
        // pDecodeEtaxUrl=https://etax.shanghai.chinatax.gov.cn/login-web/api/third/sso/login/redirect?qd=KEXIN&channelId=web&goto=30010666&client_id=d598efbeddc7558c98fc32197114a36b&response_type=code&state=pro
        if (!(pValue = strstr(pDecodeEtaxUrl, szClientIdFlag)))
            break;
        *pValue = '\0';  //原字节截断
        pValue += strlen(szClientIdFlag);
        if (strlen(pValue) < 32)
            break;
        strcpy(ctx->szBuf, pDecodeEtaxUrl);*/


        //char szClientId[100],redirect_uri[100];
        strcpy(ctx->szClientId,"d74f6Xacf6434c36acbepe4e6fpbcf8f");
        //strcpy(ctx->szClientId,ctx->szClientId");
        char pDecodeEtaxUrl_z[100];
        strcpy(pDecodeEtaxUrl_z,"https://etax.shanxi.chinatax.gov.cn/tpass/sso/loginSubmit");

        //构建json
        sprintf(szRepBuf,
                "{\"client_id\":\"%s\",\"redirect_uri\":\"%s\",\"uuid\":\"%s\",\"str_client_auth\":"
                "\"%s\",\"post_type\":\"0\",\"user_type\":\"1\",\"login_type\":\"0\"}",
                ctx->szClientId, pDecodeEtaxUrl_z, szUuid, szBuf);
        //printf("TPassLoginAuthDecodeStep3RepForStep4 szRepBuf =%s\r\n",szRepBuf);
        nRet = RET_SUCCESS;
    } while (false);
    if (root)
        cJSON_Delete(root);
    if (pDecodeEtaxUrl)
        free(pDecodeEtaxUrl);
    return nRet;
}

//给etax的单点登陆认证
int TPassLoginAuthHelloAndAuthGuizhou(EHHTTP hi, struct evhttp_uri *uriTpassFull)
{
    int nRet = ERR_GENERIC, nChildRet = 0;
    struct evbuffer *buf = NULL;
    CtxBridge ctx = (CtxBridge)hi->ctxApp;
    //可变重复用buf，szRepBuf最大接收7k数据,szBuf最大接收3k+数据
    char szRepBuf[9216] = "", szBuf[5120] = "";
    do {
        if (!(buf = evbuffer_new()))
            break;
        // =============================step3--serverHello=============================================
        memset(szBuf, 0, sizeof(szBuf));
        if (GetTaClientHello(1, hi->hDev, szBuf) < 0) {
            nRet = -1;
            break;
        }
        sprintf(szRepBuf, "{\"server_client_hello\":\"%s\"}", szBuf);
        if (BuildTPassFormatHMacSHA256Packet(true, 2, szRepBuf, ctx->szRandom16BytesKey, buf))
            break;
        //需要在回调函数中使用hi->ctxApp.szTmpBuf中保存的uuid
        hi->cbHttpNewReq = CallbackTPassSetHttpHeaderUuid;
        if (EtaxHTTPPost(hi, "/sys-api/v1.0/auth/cert/serverHello", "application/json",
                     (char *)evbuffer_pullup(buf, -1), evbuffer_get_length(buf), szRepBuf,
                     sizeof(szRepBuf)) < 0) {
            nRet = -2;
            break;
        }
        if (DecodeTpassReponseJson(hi, szRepBuf, "")) {
            nRet = -3;
            break;
        }
        // =============================step4--serverAuth=============================================
        if (TPassLoginAuthDecodeStep3RepForStep4Guizhou(hi, uriTpassFull, ctx->szRandom16BytesKey,
                                                 szRepBuf, sizeof(szRepBuf))) {
            nRet = -4;
            break;
        }
        //printf("lbc-debug TPassLoginAuthSso szRepBuf=%s\n",szRepBuf);
        if (TpassLogicIo(hi, "/sys-api/v1.0/auth/cert/serverAuth", szRepBuf, sizeof(szRepBuf))) {
            nRet = -5;
            break;
        }
        evbuffer_drain(hi->bufHttpRep, -1);
        evbuffer_add_printf(hi->bufHttpRep, "%s", szRepBuf);
        //_WriteHexToDebugFile("user.json", (uint8 *)szRepBuf, strlen(szRepBuf));
        nRet = RET_SUCCESS;
    } while (false);
    if (buf)
        evbuffer_free(buf);
    return nRet;
}

int TPassLoginAuthDecodeStep3RepForStep4Guizhou(EHHTTP hi, struct evhttp_uri *uriTpassFull,
                                         char *szRandomHex16Bytes, char *szRepBuf, int nRepBufSize)
{
    int nRet = ERR_GENERIC, nChildRet = 0;
    cJSON *root = NULL;
    CtxBridge ctx = (CtxBridge)hi->ctxApp;
    char szBuf[5120] = "", szUuid[BS_LITTLE] = "", *pValue = NULL, *pDecodeEtaxUrl = NULL,
         szClientIdFlag[] = "&client_id=";
    do {
        if (GetDataGramValueFromTPassReponse(szRepBuf, "", szRepBuf, nRepBufSize)){
            printf("----1----\n");
            break;
        }

        nChildRet = Str2Byte((uint8 *)szRepBuf, szRepBuf, strlen(szRepBuf));
        if (!(nChildRet = EtaxSM4Algorithm(false, 1, (uint8 *)szRepBuf, nChildRet,
                                       (uint8 *)szRandomHex16Bytes, (uint8 *)szRepBuf))){
            printf("----2----\n");
            break;
        }
        //解出 新的uuid，serverHelloResult
        if (!(root = cJSON_Parse(szRepBuf))){
            printf("----3----\n");
            break;
        }
        if (!(pValue = cJSON_GetObjectItem(root, "uuid")->valuestring)){
            printf("----4----\n");
            break;
        }
        strcpy(szUuid, pValue);  //新的业务uuid
        if (!(pValue = cJSON_GetObjectItem(root, "serverHelloResult")->valuestring)){
            printf("----5----\n");
            break;
        }
        strcpy(szRepBuf, pValue);
        //生成认证代码
        nChildRet = nRepBufSize;
        memset(szBuf, 0, sizeof(szBuf));
        if (BuildClientAuthCode((uint8 *)szRepBuf, strlen(szRepBuf), (uint8 *)szBuf, &nChildRet,
                                hi->hDev)){
            printf("----6----\n");
            break;
        }/*
        if (!(pValue = (char *)evhttp_uri_get_query(uriTpassFull))){
            printf("---pValue=[]---",pValue);
            printf("----7----\n");
            break;
        }
        pValue = strchr(pValue, '=') + 1;*/
        /*if (!(pDecodeEtaxUrl = evhttp_decode_uri(pValue)))
            break;
        // pDecodeEtaxUrl=https://etax.shanghai.chinatax.gov.cn/login-web/api/third/sso/login/redirect?qd=KEXIN&channelId=web&goto=30010666&client_id=d598efbeddc7558c98fc32197114a36b&response_type=code&state=pro
        if (!(pValue = strstr(pDecodeEtaxUrl, szClientIdFlag)))
            break;
        *pValue = '\0';  //原字节截断
        pValue += strlen(szClientIdFlag);
        if (strlen(pValue) < 32)
            break;
        strcpy(ctx->szBuf, pDecodeEtaxUrl);*/

        
        //char szClientId[100],redirect_uri[100];
        strcpy(ctx->szClientId,"f3dHd4H42d994f3H8b56a3577bHa9fb4");
        //strcpy(ctx->szClientId,ctx->szClientId");
        char pDecodeEtaxUrl_z[100];
        strcpy(pDecodeEtaxUrl_z,"https://etax.guizhou.chinatax.gov.cn/kxsfrz-cjpt-web/tpass/tpassLogin.do");

        //构建json
        sprintf(szRepBuf,
                "{\"client_id\":\"%s\",\"redirect_uri\":\"%s\",\"uuid\":\"%s\",\"str_client_auth\":"
                "\"%s\",\"post_type\":\"0\",\"user_type\":\"1\",\"login_type\":\"0\"}",
                ctx->szClientId, pDecodeEtaxUrl_z, szUuid, szBuf);
        //printf("TPassLoginAuthDecodeStep3RepForStep4 szRepBuf =%s\r\n",szRepBuf);
        nRet = RET_SUCCESS;
    } while (false);
    if (root)
        cJSON_Delete(root);
    if (pDecodeEtaxUrl)
        free(pDecodeEtaxUrl);
    return nRet;
}

int TPassLoginAuthDecodeStep3RepForStep4(EHHTTP hi, struct evhttp_uri *uriTpassFull,
                                         char *szRandomHex16Bytes, char *szRepBuf, int nRepBufSize)
{
    int nRet = ERR_GENERIC, nChildRet = 0;
    cJSON *root = NULL;
    CtxBridge ctx = (CtxBridge)hi->ctxApp;
    char szBuf[4*5120] = "", szUuid[BS_LITTLE] = "", *pValue = NULL,*tmp_pValue = NULL, *pDecodeEtaxUrl = NULL,
         szClientIdFlag[] = "&client_id=",tmp_szBuf[9*1024] = {0x00};
    do {
        if (GetDataGramValueFromTPassReponse(szRepBuf, "", szRepBuf, nRepBufSize))
            break;
        nChildRet = Str2Byte((uint8 *)szRepBuf, szRepBuf, strlen(szRepBuf));
        
        if (!(nChildRet = EtaxSM4Algorithm(false, 1, (uint8 *)szRepBuf, nChildRet,
                                       (uint8 *)szRandomHex16Bytes, (uint8 *)szRepBuf)))
            break;
        //解出 新的uuid，serverHelloResult
        if (!(root = cJSON_Parse(szRepBuf)))
            break;
        if (!(pValue = cJSON_GetObjectItem(root, "uuid")->valuestring))
            break;
        strcpy(szUuid, pValue);  //新的业务uuid
        if (!(pValue = cJSON_GetObjectItem(root, "serverHelloResult")->valuestring))
            break;
        memset(szRepBuf,0x00,nRepBufSize);
        strcpy(szRepBuf, pValue);
        
        //生成认证代码
        nChildRet = nRepBufSize;
        memset(szBuf, 0, sizeof(szBuf));
        if (BuildClientAuthCode((uint8 *)szRepBuf, strlen(szRepBuf), (uint8 *)szBuf, &nChildRet,
                                hi->hDev))
            break;
        if(NULL == uriTpassFull)
        {
            break;
        }
        if (!(pValue = (char *)evhttp_uri_get_query(uriTpassFull)) && hi->user_args == NULL)
            break;

        
        if(pValue == NULL && hi->user_args != NULL)
        {
            pValue = hi->user_args;
        }
        if (!(tmp_pValue = strstr(pValue, szClientIdFlag)))
            break;
        //*pValue = '\0';  //原字节截断
        tmp_pValue += strlen(szClientIdFlag);
        if (strlen(tmp_pValue) < 32)
            break;
        stpncpy(ctx->szClientId, tmp_pValue, 32);
        //pValue = strchr(pValue, '=') + 1;  redirect_uri
        tmp_pValue = strstr(pValue,"redirect_uri=");
        if(tmp_pValue == NULL)
            break;
        if (!(pDecodeEtaxUrl = evhttp_decode_uri(tmp_pValue + strlen("redirect_uri="))))
            break;
        
        strcpy(ctx->szBuf, pDecodeEtaxUrl);
        //构建json
        memset(szRepBuf,0x00,nRepBufSize);
        sprintf(szRepBuf,
                "{\"client_id\":\"%s\",\"redirect_uri\":\"%s\",\"uuid\":\"%s\",\"str_client_auth\":"
                "\"%s\",\"post_type\":\"0\",\"user_type\":\"1\",\"login_type\":\"0\"}",
                ctx->szClientId, pDecodeEtaxUrl, szUuid, szBuf);
        nRet = RET_SUCCESS;
    } while (false);
    if (root)
        cJSON_Delete(root);
    if (pDecodeEtaxUrl)
        free(pDecodeEtaxUrl);
    return nRet;
}

int get_final_url(char *src_data,char *code,char *final_url)
{
    int i = 0,count = 0,num1 = 0,num2 = 0;
    char *p_start = NULL,*p_end = NULL;
    char *tmp_url = NULL;
    p_start = strstr(src_data,"&redirect_uri=");
    if(NULL == p_start)
    {
        return -1;
    }
    p_end = strstr(p_start + strlen("&redirect_uri=") + 1,"&");
    if(NULL == p_end)
    {
        return -1;
    }
    tmp_url = (char *)calloc(1,p_end - p_start - strlen("&redirect_uri=") + 1);
    if(NULL == tmp_url)
    {
        return -1;
    }
    memcpy(tmp_url,p_start + strlen("&redirect_uri="),p_end - p_start - strlen("&redirect_uri="));
    for(i = 0;i<strlen(tmp_url);)
    {
        if((i < strlen(tmp_url) - 6 ) && tmp_url[i] == '%' )   //&& (tmp_url[i+1] != '3' || tmp_url[i+2] != 'D')
        {
            if(tmp_url[i+1] >= '0' && tmp_url[i+1] <= '9')
            {
                num1 = tmp_url[i+1] - '0';
            }
            else if(tmp_url[i+1] >= 'A' && tmp_url[i+1] <= 'F')
            {
                num1 = 10 + tmp_url[i+1] - 'A';
            }
            if(tmp_url[i+2] >= '0' && tmp_url[i+2] <= '9')
            {
                num2 = tmp_url[i+2] - '0';
            }
            else if(tmp_url[i+2] >= 'A' && tmp_url[i+2] <= 'F')
            {
                num2 = 10 + tmp_url[i+2] - 'A';
            }
            final_url[count++] = num1*16 + num2;
            i += 3;
        }
        else
        {
            final_url[count++] = tmp_url[i++];
        }
    }
    strcat(final_url,"&code=");
    strcat(final_url,code);
    if(tmp_url)
        free(tmp_url);
    return 0;
}

//调用后改动ctx的szSecurityTokenKey和llFingerPrint
//####urlyzm只和url有关系，和post内容和HTTP头无关；返回值在hi->bufHttpRep;szPostJsonString为空字符串时GET方法
char *DpptGetLosHeaderAndBody(EHHTTP hi, char *szUrlPathQuery, char *szPostJsonString,
                              char *szOutLos28199Path, int nOutPahtSize)
{
    char szSsoToken[BS_BIG] = "", *pBuf = NULL;
    CtxBridge ctx = (CtxBridge)hi->ctxApp;
    int nBufSize = 0, nRet = ERR_GENERIC;

    struct timeval start;
    gettimeofday(&start, NULL);
    uint64 llTimeStampMs = 1000 * (uint64)start.tv_sec + start.tv_usec / 1000;

    do {
        nBufSize = strlen(szPostJsonString) * 3 + 1024;
        if (!(pBuf = calloc(1, nBufSize)))
            break;
        if (GetCookiesSegment(hi->ctxCon.bufCookies, "dzfp-ssotoken", szSsoToken,
                              sizeof(szSsoToken)))
            break;
        printf("hi->modelTaAddr.szRegCode=%s\r\n",hi->modelTaAddr.szRegCode);


        if (GetRandUrlyzmPath(szUrlPathQuery, szSsoToken, ctx->szSecurityTokenKey,
                            szOutLos28199Path, nOutPahtSize))
        break;    


        if (GetLos28199UrlPath(szOutLos28199Path, llTimeStampMs, ctx->llFingerPrint,
                               szOutLos28199Path, nOutPahtSize))
            break;
        // post时才计算,如果GET时进行无效计算会导致KeepAlive接口调用出错，暂没空测试原因
        if (strlen(szPostJsonString) > 0 &&
            BuildBridgePostJmbw(szUrlPathQuery, szSsoToken, szPostJsonString, pBuf, nBufSize))
            break;
        if (GetLzkqow23819Header(szOutLos28199Path, szSsoToken,
                                 (char *)evbuffer_pullup(ctx->ebJsVmpWebshellcode, -1), pBuf,
                                 llTimeStampMs, ctx->llFingerPrint, ctx->szHeaderLzkqow23819))
            break;
        nRet = RET_SUCCESS;
    } while (false);
    if (nRet && pBuf) {
        free(pBuf);  //错误且申请
        pBuf = NULL;
    }
    return pBuf;
}

// 鼠标操作模拟函数，szOut缓存要足够大
int GetHumanMouseBehavior(int nWantCount, char *szOut)
{
    //[[1620,281,"1687080274339"],[1288,1049,"1687080530439"],[1355,1163,"1687080549039"]]
    int i, x, y;
    uint64 lTimeStamp;
    struct timeval start;
    const int nScreenMaxX = 2560, nScreenMaxY = 1600, nEdge = 20;
    char szBuf[BS_NORMAL];
    strcpy(szOut, "[");
    // nEdge为起始边缘系数，控制起始坐标位于屏幕左上角区域,不能小于5
    x = GetRandNum(nScreenMaxX / nEdge, nScreenMaxX / 5);
    y = GetRandNum(nScreenMaxY / nEdge, nScreenMaxY / 5);
    for (i = 0; i < nWantCount; i++) {
        //每次移动30-120像素
        x = MIN(x + GetRandNum(30, 120), nScreenMaxX);
        y = MIN(y + GetRandNum(20, 80), nScreenMaxY);
        //往前推算时间防止造成未来时间,最后一次时间又不能距离当前时间久远
        gettimeofday(&start, NULL);
        lTimeStamp = 1000 * (uint64)start.tv_sec + start.tv_usec / 1000;
        lTimeStamp -= (nWantCount - i - 1) * 500;
        lTimeStamp -= GetRandNum(100, 400);
        sprintf(szBuf, "[%d,%d,\"%llu\"]%s", x, y, lTimeStamp, (i + 1 == nWantCount ? "" : ","));
        strcat(szOut, szBuf);
    }
    strcat(szOut, "]");
    return RET_SUCCESS;
}

int DpptHttpIo(EHHTTP hi, char *szReqPath, bool bPostGet, bool bWithHmbEvent, char *szJmbwBuf)
{
    char *pSendBuf = NULL, *pBuf1 = NULL, *pBuf2 = NULL, *pRep = NULL, szHmbBufHead[BS_BLOCK],
         szHmbBufTail[BS_BLOCK], szPubKey[BS_BIG] = "04";
    int nBufSize = strlen(szJmbwBuf) * 3 + 2048, nRet = ERR_GENERIC, nChildRet = 0, nSendSize = 0;
    //行为缓冲拼凑特征逻辑
    do {
        if (!bWithHmbEvent) {
            pSendBuf = szJmbwBuf;
            nSendSize = strlen(szJmbwBuf);
            break;
        }
        // 04开头SM2withSM3 Hash国标key标准
        if (GetCookiesSegment(hi->ctxCon.bufCookies, "DTSwUOYx7MiWN", szPubKey + 2,
                              sizeof(szPubKey) - 2) ||
            strlen(szPubKey) != 130)
            break;
        if (!(pBuf1 = calloc(1, nBufSize)) || !(pBuf2 = calloc(1, nBufSize)))
            break;
        //官方3次和20次
        memset(pBuf1, 0, nBufSize);
        GetHumanMouseBehavior(3, pBuf1);
        Base64_Encode(pBuf1, strlen(pBuf1), szHmbBufHead);
        memset(pBuf1, 0, nBufSize);
        GetHumanMouseBehavior(20, pBuf1);
        Base64_Encode(pBuf1, strlen(pBuf1), szHmbBufTail);
        // databuf
        memset(pBuf1, 0, nBufSize);
        Base64_Encode(szJmbwBuf, strlen(szJmbwBuf), pBuf1);
        // format
        snprintf(pBuf2, nBufSize, "%s,%s,%s", szHmbBufHead, pBuf1, szHmbBufTail);
        if ((nChildRet = SM2EncryptAlgorithm((uint8 *)pBuf2, strlen(pBuf2), szPubKey,
                                             (uint8 *)pBuf1, nBufSize)) <= 0)
            break;
        nChildRet = SM2EncryptDataRemoveAsn1((uint8 *)pBuf1, nChildRet);
        memset(pBuf2, 0, nBufSize);
        strcpy(pBuf2, "$04d");
        Byte2Str(pBuf2 + 4, pBuf1, nChildRet);
        pSendBuf = pBuf2;
        nSendSize = strlen(pBuf2);
    } while (false);
    //发送逻辑
    do {
        if (bPostGet && !nSendSize) {
            nRet = ERR_GENERIC;
            break;
        }
        hi->cbHttpNewReq = CallbackDppt;
        if (bPostGet){
            nChildRet = EtaxHTTPPostEvent(hi, szReqPath, "application/json", (uint8 *)pSendBuf, nSendSize);
        }
        else
            nChildRet = EtaxHTTPGetEvent(hi, szReqPath);
        if (nChildRet < 0)
            break;
        pRep = (char *)evbuffer_pullup(hi->bufHttpRep, -1);
        if (strstr(pRep, "\"Error\":{\"Code")) {
            if (FindStringFromEvBuffer(hi->bufHttpRep, "\"Error\":", ",\"Data\":", szHmbBufHead,
                                       sizeof(szHmbBufHead)))
                break;
            SetLastError(hi->hDev->hUSB, ERR_TA_REPONSE_CHECK, szHmbBufHead);
            break;
        }
        if (strstr(pRep, "{\"Response\":") != pRep) {
            SetLastError(hi->hDev->hUSB, ERR_TA_REPONSE_CHECK, "Unknow reponse,%s", pRep);
            break;
        }
        nRet = nChildRet;
    } while (false);
    // clear
    if (pBuf1)
        free(pBuf1);
    if (pBuf2)
        free(pBuf2);
    return nRet;
}

//####urlyzm只和url有关系，和post内容和HTTP头无关；返回值在hi->bufHttpRep;szPostJsonString为空字符串时GET方法
int DpptEncryptIo(HETAX hEtax, char *szUrlPathQuery, char *szPostJsonString)
{
    int nChildRet = ERR_GENERIC, nRet = ERR_TA_REPONSE_CHECK, nBufSize = 0;
    char szBuf[BS_HUGE], *pBuf = NULL, *pRep = NULL;
    EHHTTP hi = &hEtax->dppt;
    do {
        if (!(pBuf = DpptGetLosHeaderAndBody(hi, szUrlPathQuery, szPostJsonString, szBuf,
                                             sizeof(szBuf))))
            break;
        if ((nChildRet = DpptHttpIo(hi, szBuf, strlen(szPostJsonString) > 0, false, pBuf)) < 0)
            break;
        nRet = nChildRet;
    } while (false);
    if (pBuf)
        free(pBuf);
    return nRet;
}

int EtaxLoadDataItemFromResponse(struct evbuffer *bufHttpRep, cJSON **root, cJSON **jItem)
{
    int nRet = ERR_GENERIC;
    do {
        if (!(*root = cJSON_Parse((const char *)evbuffer_pullup(bufHttpRep, -1))) ||
            !(*jItem = cJSON_GetObjectItem(*root, "Response")) ||
            !(*jItem = cJSON_GetObjectItem(*jItem, "Data")))
            break;
        nRet = RET_SUCCESS;
    } while (false);
    return nRet;
}


//税务机关代码查询
int EtaxApiGetSwjgdm(HETAX hEtax, char *szOutZgswskfjDm)
{
    //返回json种还有个sjswjgdm，暂且不管
    int nRet = ERR_GENERIC, nChildRet = 0;
    cJSON *root = NULL, *jItem = NULL, *jTarget = NULL;
    do {
        if ((nChildRet =
                 DpptEncryptIo(hEtax, "/sqlz/lskjysyslfpqxsq/v1/getNsrTokenAndSwjgdm", "{}")) < 0)
            break;
        if (EtaxLoadDataItemFromResponse(hEtax->dppt.bufHttpRep, &root, &jItem))
            break;
        //{"Response":{"RequestId":"715e4cd79676fb15","Data":{"zgswskfjDm":"13102305100","baseUrlNsrd":"https://.../wszx-web/api/sh/zsk/redirectToZnzx","dptptoken":"...","sjswjgdm":"13100000000","state":"success"}}}
        if (!(jTarget = cJSON_GetObjectItem(jItem, "zgswskfjDm")))
            break;
        strcpy(szOutZgswskfjDm, jTarget->valuestring);
        nRet = RET_SUCCESS;
    } while (false);
    if (root)
        cJSON_Delete(root);
    return nRet;
}

//纳税人注册信息查询，目前测试了只查自己；
int EtaxApiNsrjcxx(HETAX hEtax, HDEV hDev,EHFPXX fpxx)
{
    //返回json种还有个sjswjgdm，暂且不管
    int nRet = ERR_GENERIC, nChildRet = 0;
    cJSON *root = NULL, *jItem = NULL, *jTarget = NULL,*objItemChild = NULL,*objItemChild2 = NULL;
    char szReserve[BS_NORMAL] = "";
    do {
        if ((nChildRet = DpptEncryptIo(hEtax, "/kpfw/hqnsrjcxx/v1/hqnsrjcxx", "{\"Nsrsbh\":\"\"}")) < 0)
            break;
        //{"Response":{"RequestId":""Error":null,"Data":{"Nsrsbh":"91310113MA1GP39U05","Djxh":"10213102000000461568","Qdbz":"Y","Rylb":"09","Ssdabh":"91310113MA1GP39U05","Shxydm":null,"ProvinceId":"3100","Nsrmc":"上海洛骆文化创意有限公司","Nsrlx":"1","Scjydz":"上海市普陀区...","Zcdz":"上海市崇明区长兴镇...","Scjydlxdh":"18501681220","Zcdyzbm":"200940","Zcdlxdh":null,"Fddbrxm":null,"Djzclxdm":"173","Hydm":"8890","Ssjswjgdm":"13100000000","Dsjswjgdm":"13102000000","Qxjswjgdm":"13102300000","Swjgdm":"13102300000","Swjgmc":"","Jyfw":"一般项目：组织文化艺术交流活动...","Cwfzrxm":"陈子潇","Khhmc":null,"Yhzh":null,"Bsrxm":"胡庆","Ssglydm":"13102301214","Ssglyxm":"","JdxzDm":"310230123","Djrq":"2020-11-13
        // 00:00:00","Xdpsdqybz":"","Fxnsrlx":"","Xydj":"","Ckqyfldm":"","Zfjglxdm":"0","Nsryjjb":"","Dqsj":"20230523","Province":"上海","Fxnsrbz":"","Nsrztdm":"03","Yxqq":"","Yxqz":"","Sfkkjzzfp":"Y","Dqskssq":"202305","Sbzt":"","Kpr":"xxx","Qyhyxz":[],"Xfsnsrlx":[],"Qysxed":null,"Tdys":[{"Tdyslxdm":"03","Tdyslxmc":"建筑服务","Xybz":null,"Yxbz":null},{"Tdyslxdm":"04","Tdyslxmc":"货物运输服务","Xybz":null,"Yxbz":null},{"Tdyslxdm":"05","Tdyslxmc":"不动产销售","Xybz":null,"Yxbz":null},{"Tdyslxdm":"13","Tdyslxmc":"拖拉机和联合收割机","Xybz":null,"Yxbz":null},{"Tdyslxdm":"06","Tdyslxmc":"不动产经营租赁服务","Xybz":null,"Yxbz":null},
        //{"Tdyslxdm":"09","Tdyslxmc":"旅客运输服务","Xybz":null,"Yxbz":null}],"Fpzldms":[{"Fpzldm":"01","Fpzlmc":"增值税专用发票"},{"Fpzldm":"02","Fpzlmc":"普通发票"}],"ZgswskfjDm":null,"Xfdzdhyhzh":null,"Jzjtbz":"N","YhxxList":[{"YhzhxzDm":"1110","YhyywdDm":"103290043119","Yhyywdmc":"中国农业银行股份有限公司上海程桥支行","Yhzh":"03312800040033545"}],"Smkpid":"?p=Zjk2...&s=DPPT_SMKP&v=01&province=上海&smkpid=...&provinceid=3100&kpdwmc=上海洛骆文化创意有限公司&kpdwnsrsbh=91310113MA1GP39U05&gndm=SMKP","ProviceId":"3100"}}}
        //
        // Rylb=人员类别;Shxydm=社会信用代码;Scjydz=生产经营地址;Zcdz=注册地址;Scjydlxdh=生产经营地联系电话;Zcdyzbm=注册地邮政编码;Zcdlxdh=注册地联系电话;Fddbrxm=法定代表人姓名;Djzclxdm=登记注册类型代码;Hydm=行业代码;Cwfzrxm=财务负责人姓名
        // Khhmc=开户行名称;Yhzh=银行账号;Bsrxm=报税人姓名;Ssglydm=税收管理员代码;Ssglyxm=税收管理员姓名;JdxzDm=街道乡镇代码;Fxnsrlx=风险纳税人类型;Xydj=信用等级;Ckqyfldm=出口企业分类代码;Zfjglxdm=政府机关类型代码;Nsryjjb=纳税人预警级别;Fxnsrbz=风险纳税人标志
        // Nsrztdm=纳税人状态代码;Sfkkjzzfp=是否可开具纸质发票;Dqskssq=当前税款所属期;Sbzt=上报状态;Qyhyxz=企业行业性质;Xfsnsrlx=消费税纳税人类型;Qysxed=企业授信额度;ZgswskfjDm=主管税务所（科，分局）代码代码;Xfdzdhyhzh=销方地址电话银行账号;Jzjtbz=即征即退标志;
        if (EtaxLoadDataItemFromResponse(hEtax->dppt.bufHttpRep, &root, &jItem))
            break;
        // szCommonTaxID
        if (!(jTarget = cJSON_GetObjectItem(jItem, "Nsrsbh")))
            break;
        strcpy(hDev->szCommonTaxID, jTarget->valuestring);
        // szDjxh 登记序号 example:10213102000000461568
        if (!(jTarget = cJSON_GetObjectItem(jItem, "Djxh")))
            break;
        strcpy(szReserve, jTarget->valuestring);
        // szCompanyName
        if (!(jTarget = cJSON_GetObjectItem(jItem, "Nsrmc")))
            break;
        strcpy(hDev->szCompanyName, jTarget->valuestring);
        //纳税人性质,和传统的有区别；
        // 1：一般纳税人；2：小规模纳税人；3：转登记小规模纳税人；4：辅导期一般纳税人；5：自然人
        if (!(jTarget = cJSON_GetObjectItem(jItem, "Nsrlx")))
            break;
        if (!strcmp(jTarget->valuestring, "1"))
            hDev->bNatureOfTaxpayer = 2;
        else if (!strcmp(jTarget->valuestring, "2"))
            hDev->bNatureOfTaxpayer = 1;
        else if (!strcmp(jTarget->valuestring, "3"))
            hDev->bNatureOfTaxpayer = 3;
        else if (!strcmp(jTarget->valuestring, "4"))
            hDev->bNatureOfTaxpayer = 9;
        else if (!strcmp(jTarget->valuestring, "5"))
            hDev->bNatureOfTaxpayer = 100;
        //销方地址和注册地址，Scjydlxdh,暂跳过
        // 拼接税务层级 "Ssjswjgdm": "13100000000", "Dsjswjgdm": "13102000000", "Qxjswjgdm":
        // "13102300000", "Swjgdm": "13102300000",
        if (!(jTarget = cJSON_GetObjectItem(jItem, "Ssjswjgdm")))
            break;
        strcpy(hDev->szTaxAuthorityCodeBelong, jTarget->valuestring);
        strcat(hDev->szTaxAuthorityCodeBelong, "|");
        if (!(jTarget = cJSON_GetObjectItem(jItem, "Dsjswjgdm")))
            break;
        strcat(hDev->szTaxAuthorityCodeBelong, jTarget->valuestring);
        strcat(hDev->szTaxAuthorityCodeBelong, "|");
        if (!(jTarget = cJSON_GetObjectItem(jItem, "Qxjswjgdm")))
            break;
        strcat(hDev->szTaxAuthorityCodeBelong, jTarget->valuestring);
        strcat(hDev->szTaxAuthorityCodeBelong, "|");
        if (!(jTarget = cJSON_GetObjectItem(jItem, "Swjgdm")))
            break;
        strcat(hDev->szTaxAuthorityCodeBelong, jTarget->valuestring);
        //销售方信息在这里获取
        //销售方地址
        if (!(jTarget = cJSON_GetObjectItem(jItem, "Scjydz")))
            break;
        if(jTarget->valuestring != NULL)
            strcpy(fpxx->xfdz,jTarget->valuestring);
        //销售方联系电话
        if (!(jTarget = cJSON_GetObjectItem(jItem, "Zcdlxdh")))
            break;
        if(jTarget->valuestring != NULL)
            strcpy(fpxx->xfdh,jTarget->valuestring);
        //销售方名称
        if (!(jTarget = cJSON_GetObjectItem(jItem, "Nsrmc")))
            break;
        if(jTarget->valuestring != NULL)
            strcpy(fpxx->xfmc,jTarget->valuestring);
        //销售方纳税人识别号
        if (!(jTarget = cJSON_GetObjectItem(jItem, "Ssdabh")))
            break;
        if(jTarget->valuestring != NULL)
            strcpy(fpxx->xfsh,jTarget->valuestring);
        //销售方银行账号
        if (!(jTarget = cJSON_GetObjectItem(jItem, "YhxxList")))
            break;
        int i,nCount;
        nCount = cJSON_GetArraySize(jTarget);
        char *p_buf;
        p_buf = cJSON_Print(jTarget);
        for (i = 0; i < nCount; i++) 
        {
            objItemChild = cJSON_GetArrayItem(jTarget, i);
            if (!(objItemChild2 = cJSON_GetObjectItem(objItemChild, "Yhzh")))
                break;
            if(objItemChild2->valuestring != NULL)
                strcpy(fpxx->xfyhzh,objItemChild2->valuestring);
            if (!(objItemChild2 = cJSON_GetObjectItem(objItemChild, "Yhyywdmc")))
                break;
            if(objItemChild2->valuestring != NULL)
                strcpy(fpxx->xfkhh,objItemChild2->valuestring);
        }
        nRet = RET_SUCCESS;
    } while (false);
    if (root)
        cJSON_Delete(root);
    return nRet;
}

int EtaxLoadDevOnline(HETAX hEtax, HDEV hDev,EHFPXX fpxx)
{
    int nRet = ERR_PROGRAME_LOADDEV;
    do {
        if (EtaxApiGetSwjgdm(hEtax, hDev->szTaxAuthorityCode))
            break;
        if (EtaxApiNsrjcxx(hEtax, hDev,fpxx))
            break;
        nRet = RET_SUCCESS;
    } while (false);
    return nRet;
}

// strin和strout不能为同一地址，否则存在潜在内存错误
void RemoveHeadTailCtrlChar(char *strIn, char *strOut)
{
    int i, j;
    int nInSize = strlen(strIn);
    if (!nInSize)
        return;
    i = 0;
    j = nInSize - 1;

    int k = 0;
    for (k = 0; k < nInSize; k++) {
        if ((strIn[k] == 0x20) || (strIn[k] >= 0x09 && strIn[k] <= 0x0d))
            continue;
        break;
    }
    if (k == nInSize) {
        _WriteLog(LL_INFO, "传入的数据全部为不可见字符或空格,数据：%s,%02x", strIn, strIn[0]);
        return;
    }
    while ((strIn[i] == 0x20) || (strIn[i] >= 0x09 && strIn[i] <= 0x0d)) ++i;
    while ((strIn[j] == 0x20) || (strIn[j] >= 0x09 && strIn[j] <= 0x0d)) --j;
    strncpy(strOut, strIn + i, j - i + 1);
    strOut[j - i + 1] = '\0';
}

//格式化导入json数据;除备注外其他皆从此导入（备注单独调用json
// API导入），因此不用考虑备注存在控制字符的特殊情况
//判断规则:字符串前后控制字符自动移除，中间控制字符报错
int GetAvailImportDataKp(cJSON *jsonInputParentNode, char *szGetNodeKey, char *szOut, int nOutMaxLen,
                       bool bCannotNull)
{
    char *pNew = NULL;
    memset(szOut, 0, nOutMaxLen);
    int nChildRet = 0;
    // jsonNode 有可能为空,因此不判断返回
    cJSON *jsonNode = cJSON_GetObjectItem(jsonInputParentNode, szGetNodeKey);
    //比最大值少1位，保护缓冲区
    if (bCannotNull)
        nChildRet = get_json_value_can_not_null(jsonNode, szOut, 1, nOutMaxLen - 1);
    else
        nChildRet = get_json_value_can_null(jsonNode, szOut, 0, nOutMaxLen - 1);
    if (nChildRet < 0)
        return -1;
    //要求<br/>转换为\r\n,并且字符串前后不能有\r\n \\n;\t等其他特殊字符则先不处理
    if (!(pNew = StrReplace(szOut, "<br/>", "\r\n")))
        return -2;
    RemoveHeadTailCtrlChar(pNew, szOut);
    free(pNew);
    //字符串中间不允许存在回车换行
    if (strchr(szOut, '\r') || strchr(szOut, '\n'))
        return -3;
    return RET_SUCCESS;
}

int JudgeTaxIDKp(HUSB hUsb, char *szTaxID, bool bCanNotEmpty)
{
    int nTaxIDLength = strlen(szTaxID);
    int i;
    //排除掉空
    if (!bCanNotEmpty && !nTaxIDLength)
        return RET_SUCCESS;
    if (nTaxIDLength != 15 && nTaxIDLength != 17 && nTaxIDLength != 18 && nTaxIDLength != 20)
        return SetLastError(hUsb, DF_TAX_ERR_CODE_DATA_PARSING_FAILED,
                            "税号只允许15、17、18、20位");
    for (i = 0; i < nTaxIDLength; i++) {
        if ((('0' <= szTaxID[i]) && (szTaxID[i] <= '9')) ||
            (('A' <= szTaxID[i]) && (szTaxID[i] <= 'Z'))) {
            //去除I、O、Z、S、V校验
            continue;
        }
        return SetLastError(hUsb, DF_TAX_ERR_CODE_DATA_PARSING_FAILED,
                            "税号中检测到存在除大写字母及数字外的其他字符");
    }
    //金税盘不允许开具此特殊税号,可能与发票蓝冲有关
    if (!strcmp(szTaxID, "000000123456789"))
        return SetLastError(hUsb, DF_TAX_ERR_CODE_DATA_PARSING_FAILED, "不允许000000123456789税号");
    // 15位全0税号不允许开具
    if (!strcmp(szTaxID, "000000000000000"))
        return SetLastError(hUsb, DF_TAX_ERR_CODE_DATA_PARSING_FAILED, "15位全0税号不允许开具");
    return RET_SUCCESS;
}

//基本通用模块导入检测，所有开票都会从这过一遍,经典发票（专普电、二手车)、全电
//非通用字段放置于InnerImportClassical等具体发票分类导入模块中
int InnerImportCommon(cJSON *root, EHFPXX fpxx, HDEV hDev)
{
    HUSB hUsb = hDev->hUSB;
    int nRet = ERR_GENERIC, nTmpVar = 0;
    char szBuf[BS_NORMAL] = "";
    do {
        printf("2111111111111111111111111111111111111111\r\n"); 
        //发票请求流水号
        if (GetAvailImportDataKp(root, "fpqqlsh", fpxx->fpqqlsh, sizeof(fpxx->fpqqlsh), false)) {
            nRet = SetLastError(hUsb, DF_TAX_ERR_CODE_DATA_PARSING_FAILED,
                                "发票请求流水号(fpqqlsh)解析出错或缺失");
            break;
        }
        //开票类型(正数、负数)
        if (GetAvailImportDataKp(root, "kplx", szBuf, BS_FLAG, true) < 0) {
            nRet = SetLastError(hUsb, DF_TAX_ERR_CODE_DATA_PARSING_FAILED,
                                "开票类型(kplx)解析出错或缺失");
            break;
        }
        if (strcasecmp(szBuf, "0")) {
            nRet = SetLastError(hUsb, DF_TAX_ERR_CODE_DATA_PARSING_FAILED,
                                "开票类型错误，非蓝字发票");
            break;
        }
        fpxx->isRed = atoi(szBuf);
        //购方名称
        if (GetAvailImportDataKp(root, "gfmc", fpxx->gfmc, sizeof(fpxx->gfmc), true)) {
            nRet = SetLastError(hUsb, DF_TAX_ERR_CODE_DATA_PARSING_FAILED,
                                "购方名称(gfmc)解析出错或缺失");
            break;
        }
        printf("2111111111111111111111111111111111111112\r\n"); 
        //购方税号
        if (fpxx->fplx == FPLX_COMMON_ZYFP || fpxx->fplx == FPLX_COMMON_DZZP ||
            fpxx->fplx == FPLX_COMMON_ESC)
            nTmpVar = 1;  //不可为空
        else if (fpxx->fplx == FPLX_COMMON_PTFP || fpxx->fplx == FPLX_COMMON_DZFP)
            nTmpVar = 0;  //普票及电票购方税号、地址电话、银行账号可为空（暂不支持税号空）
        if (GetAvailImportDataKp(root, "gfsh", fpxx->gfsh, sizeof(fpxx->gfsh), nTmpVar)) {
            nRet = SetLastError(hUsb, DF_TAX_ERR_CODE_DATA_PARSING_FAILED,
                                "购方税号(gfsh)解析出错或缺失");
            break;
        }
        if ((nRet = JudgeTaxIDKp(hUsb, fpxx->gfsh, nTmpVar)))
            break;
        //购方银行账号
        if (GetAvailImportDataKp(root, "gfyhzh", fpxx->gfyhzh, sizeof(fpxx->gfyhzh), false)) {
            nRet = SetLastError(hUsb, DF_TAX_ERR_CODE_DATA_PARSING_FAILED,
                                "购方银行账号(gfyhzh)解析出错或缺失");
            break;
        }
        printf("2111111111111111111111111111111111111113\r\n"); 
        /*//销方名称
        if (GetAvailImportDataKp(root, "xfmc", fpxx->xfmc, sizeof(fpxx->xfmc), true)) {
            nRet = SetLastError(hUsb, DF_TAX_ERR_CODE_DATA_PARSING_FAILED,
                                "销方名称(xfmc)解析出错或缺失");
            break;
        }
        //销方税号
        if (GetAvailImportDataKp(root, "xfsh", fpxx->xfsh, sizeof(fpxx->xfsh), true)) {
            nRet = SetLastError(hUsb, DF_TAX_ERR_CODE_DATA_PARSING_FAILED,
                                "销方税号(xfsh)解析出错或缺失");
            break;
        }
        //销方银行账号
        if (GetAvailImportDataKp(root, "xfyhzh", fpxx->xfyhzh, sizeof(fpxx->xfyhzh), false)) {
            nRet = SetLastError(hUsb, DF_TAX_ERR_CODE_DATA_PARSING_FAILED,
                                "销方银行账号(xfyhzh)解析出错或缺失");
            break;
        }*/
        //综合税率导入接口,不能修改只能保持zhsl,内部值改为slv
        if (GetAvailImportDataKp(root, "zhsl", fpxx->slv, sizeof(fpxx->slv), false)) {
            nRet = SetLastError(hUsb, DF_TAX_ERR_CODE_DATA_PARSING_FAILED,
                                "综合税率(zhsl)解析出错或缺失");
            break;
        }
        //含税价标志 经典:0/1/2 全电:1/2
        if (GetAvailImportDataKp(root, "hsjbz", fpxx->hsjbz, sizeof(fpxx->hsjbz), true)) {
            nRet = SetLastError(hUsb, DF_TAX_ERR_CODE_DATA_PARSING_FAILED,
                                "含税价标志(hsjbz)解析出错或缺失");
            break;
        }
        nTmpVar = atoi(fpxx->hsjbz);
        if ((nTmpVar != 0) && (nTmpVar != 1)) {
            nRet = SetLastError(hUsb, DF_TAX_ERR_CODE_DATA_PARSING_FAILED,
                                "目前仅支持商品全部含税或不含税开票");
            break;
        }
        printf("2111111111111111111111111111111111111114\r\n"); 
        //合计金额
        if (GetAvailImportDataKp(root, "hjje", fpxx->je, sizeof(fpxx->je), true)) {
            nRet = SetLastError(hUsb, DF_TAX_ERR_CODE_DATA_PARSING_FAILED,
                                "合计金额(hjje)解析出错或缺失");
            break;
        }
        //合计税额
        if (GetAvailImportDataKp(root, "hjse", fpxx->se, sizeof(fpxx->se), true)) {
            nRet = SetLastError(hUsb, DF_TAX_ERR_CODE_DATA_PARSING_FAILED,
                                "合计税额(hjse)解析出错或缺失");
            break;
        }
        //价税合计
        if (GetAvailImportDataKp(root, "jshj", fpxx->jshj, sizeof(fpxx->jshj), true)) {
            nRet = SetLastError(hUsb, DF_TAX_ERR_CODE_DATA_PARSING_FAILED,
                                "价税合计(jshj)解析出错或缺失");
            break;
        }
        printf("2111111111111111111111111111111111111115\r\n"); 
        //--校验如上几个涉及金额的字段
        if ((nRet = CheckFpxx(fpxx)) < 0)
            break;
        printf("2111111111111111111111111111111111111116\r\n"); 
        //收款人(二手车无此字段，暂放)
        if (GetAvailImportDataKp(root, "skr", fpxx->skr, sizeof(fpxx->skr), false)) {
            nRet = SetLastError(hUsb, DF_TAX_ERR_CODE_DATA_PARSING_FAILED,
                                "收款人(skr)解析出错或缺失,%s");
            break;
        }
        //复核人(二手车无此字段，暂放)
        if (GetAvailImportDataKp(root, "fhr", fpxx->fhr, sizeof(fpxx->fhr), false)) {
            nRet = SetLastError(hUsb, DF_TAX_ERR_CODE_DATA_PARSING_FAILED,
                                "复核人(fhr)解析出错或缺失");
            break;
        }
        //开票人
        if (GetAvailImportDataKp(root, "kpr", fpxx->kpr, sizeof(fpxx->kpr), false)) {
            nRet = SetLastError(hUsb, DF_TAX_ERR_CODE_DATA_PARSING_FAILED,
                                "开票人(kpr)解析出错或缺失");
            break;
        }
        printf("2111111111111111111111111111111111111117\r\n"); 
        nRet = RET_SUCCESS;
    } while (false);
    return nRet;
}


bool CheckEtaxFplxSupportByDeviceType(int fplx)
{
    bool ret = false;
    do {
        if (fplx != FPLX_ETAX_ZYFP && fplx != FPLX_ETAX_PTFP)
            break;
        ret = true;
    } while (false);
    return ret;
}

void FillHsAndBhsPrice(bool bPriceWithTax, struct ESpxx *spxx)
{
    if (bPriceWithTax) {  //含税
        strcpy(spxx->hsdj, spxx->dj);
        strcpy(spxx->hsje, spxx->je);
        CalcTaxratePrice(true, spxx->dj, spxx->slv, 2, spxx->bhsdj);
        CalcTaxratePrice(true, spxx->je, spxx->slv, 2, spxx->bhsje);
    } else {  //不含税
        strcpy(spxx->bhsdj, spxx->dj);
        strcpy(spxx->bhsje, spxx->je);
        CalcTaxratePrice(false, spxx->dj, spxx->slv, 2, spxx->hsdj);
        CalcTaxratePrice(false, spxx->je, spxx->slv, 2, spxx->hsje);
    }
}


//之前离线经典开票计算太复杂，现在在线开票由税局自己校验，可以减少计算；为减少历史包袱，更正为最简单的导入后续逐步完善
int EtaxInnerImportSpxxCommon(cJSON *root, EHFPXX fpxx, HDEV hDev)
{
    cJSON *jsonArraySpxx = NULL, *jsonItem = NULL;
    int nCount = 0, nAvailSpxxIndex = 0, nRet = RET_SUCCESS;
    struct ESpxx *spxx = NULL;
    HUSB hUsb = hDev->hUSB;
    bool bPriceWithTax = !strcmp(fpxx->hsjbz, "1");  // 0/1,0=不含税，1=含税
    //商品明细
    if (!(jsonArraySpxx = cJSON_GetObjectItem(root, "fpmx")))
        return SetLastError(hUsb, DF_TAX_ERR_CODE_DATA_PARSING_FAILED, "导入发票明细信息错误");
    fpxx->spsl = cJSON_GetArraySize(jsonArraySpxx);
    if (fpxx->spsl > 2000)
        return SetLastError(hUsb, DF_TAX_ERR_CODE_INV_DATA_TOO_LARGE, "商品条数不得超过2000行");
    if (((fpxx->fplx == FPLX_COMMON_DZFP) || (fpxx->fplx == FPLX_COMMON_DZZP)) &&
        (fpxx->spsl > 100))
        return SetLastError(hUsb, DF_TAX_ERR_CODE_INV_DATA_TOO_LARGE,
                            "电子发票商品条数不得超过100行");
    //商品枚举主循环
    for (nCount = 0; nCount < fpxx->spsl; nCount++) {
        nAvailSpxxIndex++;
        spxx = calloc(1, sizeof(struct ESpxx));
        if (!spxx) {
            nRet = SetLastError(hUsb, DF_TAX_ERR_CODE_SYSTEM_MEM_INSUFFICIENT,
                                "商品信息结构体系统内存申请失败");
            break;
        }
        if (!(jsonItem = cJSON_GetArrayItem(jsonArraySpxx, nCount))) {
            nRet =
                SetLastError(hUsb, DF_TAX_ERR_CODE_DATA_PARSING_FAILED, "商品信息JSON节点解析出错");
            break;
        }
        //商品序号
        if (!GetAvailImportDataKp(jsonItem, "xh", spxx->xh, sizeof(spxx->xh), false)) {
            if (!strcmp(spxx->xh, "0")) {
                //清单头，跳过[外包方导入开票会使用，xh0为详解销货清单行，此处直接跳过]
                free(spxx);
                nAvailSpxxIndex--;
                continue;
            }
            if (atoi(spxx->xh) != nAvailSpxxIndex) {
                nRet = SetLastError(hUsb, DF_TAX_ERR_CODE_DATA_PARSING_FAILED,
                                    "商品信息中商品序号与json中顺序不一致,商品序号 %d",
                                    nAvailSpxxIndex);
                break;
            }
        }
        printf("33333333333333333333333333333333333339\r\n");
        //商品名称
        if (GetAvailImportDataKp(jsonItem, "spmc", spxx->spmc, sizeof(spxx->spmc), true)) {
            nRet =
                SetLastError(hUsb, DF_TAX_ERR_CODE_DATA_PARSING_FAILED,
                             "商品信息中商品名称(spmc)解析出错或缺失,商品序号 %d", nAvailSpxxIndex);
            break;
        }
        //商品税目名称
        if (GetAvailImportDataKp(jsonItem, "spsmmc", spxx->spsmmc, sizeof(spxx->spsmmc), true)) {
            nRet = SetLastError(hUsb, DF_TAX_ERR_CODE_DATA_PARSING_FAILED,
                                "商品信息中商品税目名称(spsmmc)解析出错或缺失, 商品序号 %d",
                                nAvailSpxxIndex);
            break;
        }
        //商品编号
        if (GetAvailImportDataKp(jsonItem, "spbh", spxx->spbh, sizeof(spxx->spbh), true)) {
            nRet = SetLastError(hUsb, DF_TAX_ERR_CODE_DATA_PARSING_FAILED,
                                "商品信息中商品分类编码(spbh)解析出错或缺失,商品序号 %d",
                                nAvailSpxxIndex);
            break;
        }
        //发票行性质
        if (GetAvailImportDataKp(jsonItem, "fphxz", spxx->fphxz, sizeof(spxx->fphxz), true) < 0) {
            nRet = SetLastError(hUsb, DF_TAX_ERR_CODE_DATA_PARSING_FAILED,
                                "商品信息中商品行性质(fphxz)解析出错或缺失,商品序号 %d",
                                nAvailSpxxIndex);
            break;
        }
        printf("33333333333333333333333333333333333337\r\n");
        //计量单位
        if (GetAvailImportDataKp(jsonItem, "jldw", spxx->jldw, sizeof(spxx->jldw), false)) {
            nRet =
                SetLastError(hUsb, DF_TAX_ERR_CODE_DATA_PARSING_FAILED,
                             "商品信息中计量单位(jldw)解析出错或缺失,商品序号 %d", nAvailSpxxIndex);
            break;
        }
        //规格型号
        if (GetAvailImportDataKp(jsonItem, "ggxh", spxx->ggxh, sizeof(spxx->ggxh), false)) {
            nRet =
                SetLastError(hUsb, DF_TAX_ERR_CODE_DATA_PARSING_FAILED,
                             "商品信息中规格型号(ggxh)解析出错或缺失,商品序号 %d", nAvailSpxxIndex);
            break;
        }
        //数量
        if (GetAvailImportDataKp(jsonItem, "sl", spxx->sl, sizeof(spxx->sl), false)) {
            nRet = SetLastError(hUsb, DF_TAX_ERR_CODE_DATA_PARSING_FAILED,
                                "商品信息中数量(sl)解析出错或缺失,商品序号 %d", nAvailSpxxIndex);
            break;
        }
        //单价
        if (GetAvailImportDataKp(jsonItem, "dj", spxx->dj, sizeof(spxx->dj), false)) {
            nRet = SetLastError(hUsb, DF_TAX_ERR_CODE_DATA_PARSING_FAILED,
                                "商品信息中单价(dj)解析出错或缺失,商品序号 %d", nAvailSpxxIndex);
            break;
        }
        //税额
        if (GetAvailImportDataKp(jsonItem, "se", spxx->se, sizeof(spxx->se), true)) {
            nRet = SetLastError(hUsb, DF_TAX_ERR_CODE_DATA_PARSING_FAILED,
                                "商品信息中税额(se)解析出错或缺失,商品序号 %d", nAvailSpxxIndex);
            break;
        }
        //金额
        if (GetAvailImportDataKp(jsonItem, "je", spxx->je, sizeof(spxx->je), true)) {
            nRet = SetLastError(hUsb, DF_TAX_ERR_CODE_DATA_PARSING_FAILED,
                                "商品信息中金额(je)解析出错或缺失,商品序号 %d", nAvailSpxxIndex);
            break;
        }
        //税率
        if (GetAvailImportDataKp(jsonItem, "slv", spxx->slv, sizeof(spxx->slv), true)) {
            nRet = SetLastError(hUsb, DF_TAX_ERR_CODE_DATA_PARSING_FAILED,
                                "商品信息中税率(slv)解析出错或缺失,商品序号 %d", nAvailSpxxIndex);
            break;
        }


        char szBuf[BS_NORMAL] = "";
        if (GetAvailImportDataKp(root, "sslkjly", szBuf, sizeof(szBuf), false)) {
            nRet = SetLastError(hUsb, DF_TAX_ERR_CODE_DATA_PARSING_FAILED,
                                "3%税率开具理由(sslkjly)解析出错或缺失");
            break;
        }
        fpxx->sslkjly = atoi(szBuf);


        if(strcmp(spxx->slv,"0.03")==0){                //3%税率
            if((hDev->bNatureOfTaxpayer != 1) ||(fpxx->sslkjly != 3)){        //小规模纳税人且中台传入3%税率开具理由
                nRet = SetLastError(hUsb, DF_TAX_ERR_CODE_DATA_PARSING_FAILED,
                                    "检测到当前开票税率为3%但该企业非小规模纳税人或传入3%税率开具理由(sslkjly)错误");
                break;               
            }
        }
        printf("33333333333333333333333333333333333336\r\n");
        //根据单价、税额、金额、税率 填充含税价和不含税价格
        FillHsAndBhsPrice(bPriceWithTax, spxx);
        //优惠政策标识
        if (GetAvailImportDataKp(jsonItem, "yhzcbs", spxx->xsyh, sizeof(spxx->xsyh), true)) {
            nRet = SetLastError(hUsb, DF_TAX_ERR_CODE_DATA_PARSING_FAILED,
                                "商品信息中优惠政策标识(yhzcbs)解析出错或缺失,商品序号 %d",
                                nAvailSpxxIndex);
            break;
        }

        //零税率标示=零税率标识
        if (GetAvailImportDataKp(jsonItem, "lslbs", spxx->lslvbs, sizeof(spxx->lslvbs), false)) {
            nRet = SetLastError(hUsb, DF_TAX_ERR_CODE_DATA_PARSING_FAILED,
                                "商品信息中零税率标示=零税率标识(lslbs)解析出错或缺失,商品序号 %d",
                                nAvailSpxxIndex);
            break;
        }
        printf("33333333333333333333333333333333333337\r\n");
        //增值税特殊管理
        if (GetAvailImportDataKp(jsonItem, "zzstsgl", spxx->yhsm, sizeof(spxx->yhsm), false)) {
            // 01:简易征收|02:稀土产品|03:免税|04:不征税|05:先征后退|06:100%先征后退|07:50%先征后退|08:按3%简易征收|09:按5%简易征收|10:按5%简易征收减按1.5%计征|11:即征即退30%
            // 12:即征即退50%|13:即征即退70%|14:即征即退100%|15:超税负3%即征即退|16:超税负8%即征即退|17:超税负12%即征即退|18:超税负6%即征即退
            nRet = SetLastError(hUsb, DF_TAX_ERR_CODE_DATA_PARSING_FAILED,
                                "商品信息中增值税特殊管理(zzstsgl)解析出错或缺失,商品序号 %d",
                                nAvailSpxxIndex);
            break;
        }
        //含税价标志
        if (GetAvailImportDataKp(jsonItem, "hsjbz", spxx->hsjbz, sizeof(spxx->hsjbz), false))
            memcpy(spxx->hsjbz, fpxx->hsjbz, strlen(fpxx->hsjbz));
        //插入结构体链表
        if (EInsertMxxx(fpxx, spxx) != 0) {
            nRet = SetLastError(hUsb, DF_TAX_ERR_CODE_DATA_PARSING_FAILED,
                                "系统错误，商品数据无法插入链表,商品序号 %d", nAvailSpxxIndex);
            break;
        }
        printf("33333333333333333333333333333333333338\r\n");
    }
    if (nRet != RET_SUCCESS && spxx)
       free(spxx);  //如果中间插入中断，则释放上一次
    struct ESpxx *spxx1 = NULL;
    spxx1 = fpxx->stp_MxxxHead->stp_next;
    return nRet;
}

int EtaxInnerImporRemark(cJSON *root, EHFPXX fpxx, HDEV hDev)
{
    char szUserBz[BS_BIG] = {0};
    HUSB hUsb = hDev->hUSB;
    int nRet = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
    do {
        if (get_json_value_can_not_null(cJSON_GetObjectItem(root, "bz"), szUserBz, 0,
                                        sizeof(szUserBz) - 1) < 0) {
            nRet =
                SetLastError(hUsb, DF_TAX_ERR_CODE_DATA_PARSING_FAILED, "备注(bz)解析出错或缺失");
            break;
        }
        strcpy(fpxx->bz, szUserBz);
        nRet = RET_SUCCESS;
    } while (false);
    return nRet;
}

int EtaxInnerImportFplxMain(cJSON *root, EHFPXX fpxx, HDEV hDev)
{
    HUSB hUsb = hDev->hUSB;
    int nRet = ERR_GENERIC, nChildRet = 0;
    char szBuf[BS_NORMAL] = "";
    do {
        //购方地址
        printf("33333333333333333333333333333333333331\r\n");
        if (GetAvailImportDataKp(root, "gfdz", fpxx->gfdz, sizeof(fpxx->gfdz), false)) {
            nRet = SetLastError(hUsb, DF_TAX_ERR_CODE_DATA_PARSING_FAILED,
                                "购方地址(gfdz)解析出错或缺失");
            break;
        }
        //购方电话
        if (GetAvailImportDataKp(root, "gfdh", fpxx->gfdh, sizeof(fpxx->gfdh), false)) {
            nRet = SetLastError(hUsb, DF_TAX_ERR_CODE_DATA_PARSING_FAILED,
                                "购方电话(gfdh)解析出错或缺失");
            break;
        }
        //购方开户行
        if (GetAvailImportDataKp(root, "gfkhh", fpxx->gfkhh, sizeof(fpxx->gfkhh), false)) {
            nRet = SetLastError(hUsb, DF_TAX_ERR_CODE_DATA_PARSING_FAILED,
                                "购方开户行(gfkhh)解析出错或缺失");
            break;
        }
        //购方自然人标识
        if (GetAvailImportDataKp(root, "gfzrr", szBuf, sizeof(szBuf), false)) {
            nRet = SetLastError(hUsb, DF_TAX_ERR_CODE_DATA_PARSING_FAILED,
                                "购方自然人标识(gfzrr)解析出错或缺失");
            break;
        }
        fpxx->gfzrr = !strcmp(szBuf, "1") || !strcmp(szBuf, "Y");  //同时兼容1和Y
        printf("33333333333333333333333333333333333332\r\n");
        //销方地址
        /*if (GetAvailImportDataKp(root, "xfdz", fpxx->xfdz, sizeof(fpxx->xfdz), false)) {
            nRet = SetLastError(hUsb, DF_TAX_ERR_CODE_DATA_PARSING_FAILED,
                                "销方地址(xfdz)解析出错或缺失");
            break;
        }
        //销方电话
        if (GetAvailImportDataKp(root, "xfdh", fpxx->xfdh, sizeof(fpxx->xfdh), false)) {
            nRet = SetLastError(hUsb, DF_TAX_ERR_CODE_DATA_PARSING_FAILED,
                                "销方电话(xfdh)解析出错或缺失");
            break;
        }
        //销方开户行
        if (GetAvailImportDataKp(root, "xfkhh", fpxx->xfkhh, sizeof(fpxx->xfkhh), false)) {
            nRet = SetLastError(hUsb, DF_TAX_ERR_CODE_DATA_PARSING_FAILED,
                                "销方开户行(xfkhh)解析出错或缺失");
            break;
        }*/
        //购方经办人
        if (GetAvailImportDataKp(root, "gfjbr", fpxx->gfjbr, sizeof(fpxx->gfjbr), false)) {
            nRet = SetLastError(hUsb, DF_TAX_ERR_CODE_DATA_PARSING_FAILED,
                                "购方经办人(gfjbr)解析出错或缺失");
            break;
        }
        //经办人国家代码
        if (GetAvailImportDataKp(root, "jbrgjdm", szBuf, sizeof(szBuf), false)) {
            nRet = SetLastError(hUsb, DF_TAX_ERR_CODE_DATA_PARSING_FAILED,
                                "经办人国家代码(jbrgjdm)解析出错或缺失");
            break;
        }
        printf("33333333333333333333333333333333333333\r\n");
        fpxx->jbrgjdm = atoi(szBuf);
        if (fpxx->jbrgjdm != 0 && fpxx->jbrgjdm != 156) {
            // 004:阿富汗|008:阿尔巴尼亚共和国|010:南极洲|012:阿尔及利亚民主人民共和国|016:美属萨摩亚|020:安道尔公国|024:安哥拉共和国|028:安提瓜和巴布达|031:阿塞拜疆共和国|032:阿根廷共和国|036:澳大利亚联邦|040:奥地利共和国|044:巴哈马联邦|048:巴林国|050:孟加拉人民共和国|051:亚美尼亚共和国|052:巴巴多斯|056:比利时王国|060:百慕大|064:不丹王国|068:玻利维亚共和国|070:波斯尼亚和黑塞哥维那|072:博茨瓦纳共和国|074:布维岛|076:巴西联邦共和国|084:伯利兹|086:英属印度洋领地|090:所罗门群岛|092:英属维尔京群岛|096:文莱达鲁萨兰国|100:保加利亚共和国|104:缅甸联邦|108:布隆迪共和国|
            // 112:白俄罗斯共和国|116:柬埔寨王国|120:喀麦隆共和国|124:加拿大|132:佛得角共和国|136:开曼群岛|140:中非共和国|144:斯里兰卡民主社会主义共和国|148:乍得共和国|152:智利共和国|156:中华人民共和国|158:中国台湾|162:圣诞岛|166:科科斯（基林）群岛|170:哥伦比亚共和国|174:科摩罗伊斯兰联邦共和国|175:马约特|178:刚果共和国|180:刚果民主共和国|184:库克群岛|188:哥斯达黎加共和国|191:克罗地亚共和国|192:古巴共和国|196:塞浦路斯共和国|203:捷克共和国|204:贝宁共和国|208:丹麦王国|212:多米尼克国|214:多米尼加共和国|218:厄瓜多尔共和国|222:萨尔瓦多共和国|226:赤道几内亚共和国|
            // 231:埃塞俄比亚联邦民主共和国|232:厄立特里亚国|233:爱沙尼亚共和国|234:法罗群岛|238:福克兰群岛（马尔维纳斯）|239:南乔治亚岛和南桑德韦奇岛|242:斐济群岛共和国|246:芬兰共和国|250:法兰西共和国|254:法属圭亚那|258:法属波利尼西亚|260:法属南部领地|262:吉布提共和国|266:加蓬共和国|268:格鲁吉亚|270:冈比亚共和国|275:巴勒斯坦国|276:德意志联邦共和国|288:加纳共和国|292:直布罗陀|296:基里巴斯共和国|300:希腊共和国|304:格陵兰|308:格林纳达|312:瓜德罗普|316:关岛|320:危地马拉共和国|324:几内亚共和国|328:圭亚那合作共和国|332:海地共和国|334:赫德岛和麦克唐纳岛|336:梵蒂冈城国|
            // 340:洪都拉斯共和国|344:中国香港特别行政区|348:匈牙利共和国|352:冰岛共和国|356:印度共和国|360:印度尼西亚共和国|364:伊朗伊斯兰共和国|368:伊拉克共和国|372:爱尔兰|376:以色列国|380:意大利共和国|384:科特迪瓦共和国|388:牙买加|392:日本国|398:哈萨克斯坦共和国|400:约旦哈希姆王国|404:肯尼亚共和国|408:朝鲜民主主义人民共和国|410:大韩民国|414:科威特国|417:吉尔吉斯共和国|418:老挝人民民主共和国|422:黎巴嫩共和国|426:莱索托王国|428:拉脱维亚共和国|430:利比里亚共和国|434:大阿拉伯利比亚人民社会主义民众国|438:列支敦士登公国|440:立陶宛共和国|442:卢森堡大公国|446:中国澳门特别行政区|
            // 450:马达加斯加共和国|454:马拉维共和国|458:马来西亚|462:马尔代夫共和国|466:马里共和国|470:马耳他共和国|474:马提尼克|478:毛里塔尼亚伊斯兰共和国|480:毛里求斯共和国|484:墨西哥合众国|492:摩纳哥公国|496:蒙古国|498:摩尔多瓦共和国|499:黑山|500:蒙特塞拉特|504:摩洛哥王国|508:莫桑比克共和国|512:阿曼苏丹国|516:纳米比亚共和国|520:瑙鲁共和国|524:尼泊尔王国|528:荷兰王国|530:荷属安的列斯|533:阿鲁巴|540:新喀里多尼亚|548:瓦努阿图共和国|554:新西兰|558:尼加拉瓜共和国|562:尼日尔共和国|566:尼日利亚联邦共和国|570:纽埃|574:诺福克岛|578:挪威王国|580:北马里亚纳自由联邦|
            // 581:美国本土外小岛屿|583:密克罗尼西亚联邦|584:马绍尔群岛共和国|585:帕劳共和国|586:巴基斯坦伊斯兰共和国|591:巴拿马共和国|598:巴布亚新几内亚独立国|600:巴拉圭共和国|604:秘鲁共和国|608:菲律宾共和国|612:皮特凯恩|616:波兰共和国|620:葡萄牙共和国|624:几内亚比绍共和国|626:东帝汶|630:波多黎各|634:卡塔尔国|638:留尼汪|642:罗马尼亚|643:俄罗斯联邦|646:卢旺达共和国|654:圣赫勒拿|659:圣基茨和尼维斯联邦|660:安圭拉|662:圣卢西亚|666:圣皮埃尔和密克隆|670:圣文森特和格林纳丁斯|674:圣马力诺共和国|678:圣多美和普林西比民主共和国|682:沙特阿拉伯王国|686:塞内加尔共和国|
            // 688:塞尔维亚|690:塞舌尔共和国|694:塞拉利昂共和国|702:新加坡共和国|703:斯洛伐克共和国|704:越南社会主义共和国|705:斯洛文尼亚共和国|706:索马里共和国|710:南非共和国|716:津巴布韦共和国|724:西班牙王国|728:南苏丹|732:西撒哈拉|736:苏丹共和国|740:苏里南共和国|744:斯瓦尔巴岛和扬马延岛|748:斯威士兰王国|752:瑞典王国|756:瑞士联邦|760:阿拉伯叙利亚共和国|762:塔吉克斯坦共和国|764:泰王国|768:多哥共和国|772:托克劳|776:汤加王国|780:特立尼达和多巴哥共和国|784:阿拉伯联合酋长国|788:突尼斯共和国|792:土耳其共和国|795:土库曼斯坦|796:特克斯和凯科斯群岛|798:图瓦卢|
            // 800:乌干达共和国|804:乌克兰|807:前南斯拉夫马其顿共和国|818:阿拉伯埃及共和国|826:大不列颠及北爱尔兰联合王国|831:根西岛|832:泽西岛|833:马恩岛|834:坦桑尼亚联合共和国|840:美利坚合众国|850:美属维尔京群岛|854:布基纳法索|858:乌拉圭东岸共和国|860:乌兹别克斯坦共和国|862:委内瑞拉共和国|876:瓦利斯和富图纳|882:萨摩亚独立国|887:也门共和国|891:南斯拉夫联盟共和国|894:赞比亚共和国|A00:科索沃
            nRet = SetLastError(hUsb, DF_TAX_ERR_CODE_DATA_PARSING_FAILED,
                                "经办人国家代码(jbrgjdm),只支持:'空'\'中华人民共和国(156)'");
            break;
        }
        //经办人身份证件号码代码
        if (GetAvailImportDataKp(root, "jbrsfzjzldm", szBuf, sizeof(szBuf), false)) {
            nRet = SetLastError(hUsb, DF_TAX_ERR_CODE_DATA_PARSING_FAILED,
                                "经办人身份证件号码代码(jbrsfzjzldm)解析出错或缺失");
            break;
        }
        fpxx->jbrsfzjzldm = atoi(szBuf);
        nChildRet = (fpxx->jbrsfzjzldm > 100 && fpxx->jbrsfzjzldm < 104) ||
                    (fpxx->jbrsfzjzldm > 198 && fpxx->jbrsfzjzldm < 242) ||
                    fpxx->jbrsfzjzldm == 291 || fpxx->jbrsfzjzldm == 299;
        if (fpxx->jbrsfzjzldm != 0 && !nChildRet) {
            // 101:组织机构代码证|102:营业执照|103:税务登记证|199:其他单位证件|201:居民身份证|202:军官证|203:武警警官证|204:士兵证|205:军队离退休干部证|206:残疾人证|207:残疾军人证（1-8级）|208:外国护照|210:港澳居民来往内地通行证|212:中华人民共和国往来港澳通行证|213:台湾居民来往大陆通行证|214:大陆居民往来台湾通行证|215:外国人居留证|216:外交官证|217:使（领事）馆证|218:海员证|219:香港永久性居民身份证|220:台湾身份证|221:澳门特别行政区永久性居民身份证|222:外国人身份证件|224:就业失业登记证|225:退休证|226:离休证
            // 227:中国护照|228:城镇退役士兵自谋职业证|229:随军家属身份证明|230:中国人民解放军军官转业证书|231:中国人民解放军义务兵退出现役证|232:中国人民解放军士官退出现役证|233:外国人永久居留身份证（外国人永久居留证）|234:就业创业证|235:香港特别行政区护照|236:澳门特别行政区护照|237:中华人民共和国港澳居民居住证|238:中华人民共和国台湾居民居住证|239:《中华人民共和国外国人工作许可证》（A类）|240:《中华人民共和国外国人工作许可证》（B类）|241:《中华人民共和国外国人工作许可证》（C类）|291:出生医学证明|299:其他个人证件
            nRet = SetLastError(
                hUsb, DF_TAX_ERR_CODE_DATA_PARSING_FAILED,
                "经办人身份证件号码代码(jbrsfzjzldm),只支持:'空'或者合法数字代码(参照文档)");
            break;
        }
        //经办人身份证件号码
        if (GetAvailImportDataKp(root, "jbrsfzjhm", fpxx->jbrsfzjhm, sizeof(fpxx->jbrsfzjhm),
                               false)) {
            nRet = SetLastError(hUsb, DF_TAX_ERR_CODE_DATA_PARSING_FAILED,
                                "经办人身份证件号码(jbrsfzjhm)解析出错或缺失");
            break;
        }
        printf("33333333333333333333333333333333333334\r\n");
        //经办人自然人纳税人识别号
        if (GetAvailImportDataKp(root, "jbrzrrnsrsbh", fpxx->jbrzrrnsrsbh, sizeof(fpxx->jbrzrrnsrsbh),
                               false)) {
            nRet = SetLastError(hUsb, DF_TAX_ERR_CODE_DATA_PARSING_FAILED,
                                "经办人自然人纳税人识别号(jbrzrrnsrsbh)解析出错或缺失");
            break;
        }
        //cezslxDm 差额征收类型代码,01:差额征税-全额开票;02:差额征税-差额开票
        if (GetAvailImportDataKp(root, "cezslxDm", fpxx->cezslxDm, sizeof(fpxx->cezslxDm),
                               false)) {
            nRet = SetLastError(hUsb, DF_TAX_ERR_CODE_DATA_PARSING_FAILED,
                                "差额征收类型代码(cezslxDm)解析出错或缺失");
            break;
        }
        printf("33333333333333333333333333333333333335\r\n");
        //清单标志
        if (GetAvailImportDataKp(root, "qdbz", szBuf, BS_FLAG, true)) {
            nRet = SetLastError(hUsb, DF_TAX_ERR_CODE_DATA_PARSING_FAILED,
                                "清单标志(qdbz)解析出错或缺失");
            break;
        }
        fpxx->qdbz = !strcmp(szBuf, "1") || !strcmp(szBuf, "Y");  //同时兼容1和Y
        //导入商品信息
        if ((nRet = EtaxInnerImportSpxxCommon(root, fpxx,hDev)) < 0)
            break;
        printf("33333333333333333333333333333333333336\r\n");
        //备注-很多小种类发票都是在备注中进行标识，因此备注依赖很多参数，需要在最后导入
        if ((nRet = EtaxInnerImporRemark(root, fpxx,hDev)) < 0)
            break;
        printf("33333333333333333333333333333333333337\r\n");
        nRet = RET_SUCCESS;
    } while (false);
    return nRet;
}

//开票时电子税务局会对开票json作出大量校验，因此我们本地只需要作出简单校验即可，无需和经典发票开具一样本地作出海量检测
int EtaxAnalyzeJsonBuf(char *szJsonString, EHFPXX fpxx, HDEV hDev,char *errinfo_uft8)
{
    cJSON *root = NULL;
    char szBuf[BS_NORMAL] = "";
    int nRet = DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
    do {
        //检测参数
        if (!szJsonString || !fpxx) {
            nRet = ERR_PARM_CHECK;
            sprintf((char *)errinfo_uft8, "306发票数据参数不可为空");  
            break;
        }
        //初始化结构
        if (!(root = cJSON_Parse((const char *)szJsonString))) {
            sprintf((char *)errinfo_uft8, "306发票JSON数据格式存在问题"); 
            break;
        }
        //发票类型最先导入作分支
        if (GetAvailImportDataKp(root, "fpzls", szBuf, sizeof(szBuf), true) < 0) {
            sprintf((char *)errinfo_uft8, "306发票种类(fpzls)解析出错或缺失"); 
            break;
        }
        fpxx->fplx_etax = atoi(szBuf);  // fplx_etax和fplx不具备一一对应关系，因此不做转换
        if (!CheckEtaxFplxSupportByDeviceType(fpxx->fplx_etax)) {
            sprintf((char *)errinfo_uft8, "306目前支持全电发票开具类型:专票(1)、普票(2)"); 
            break;
        }
        if ((nRet = InnerImportCommon(root, fpxx,hDev))){
            strcpy(errinfo_uft8,"306");
            strcat(errinfo_uft8,hDev->hUSB->errinfo);     //拷贝错误信息
            break;
        }
        if ((nRet = EtaxInnerImportFplxMain(root, fpxx,hDev))){
            strcpy(errinfo_uft8,"306");
            strcat(errinfo_uft8,hDev->hUSB->errinfo);     //拷贝错误信息
            break;
        }   
        nRet = RET_SUCCESS;
    } while (false);
    if (root)
        cJSON_Delete(root);
    return nRet;
}

void EtaxUpdateDevInfoToFPXX(HDEV hDev, EHFPXX fpxx)
{
    strcpy(fpxx->xfsh, hDev->szCommonTaxID);
    strcpy(fpxx->xfmc, hDev->szCompanyName);
}

void GetSpsmmcWithoutStar(char *szSmmcMc, char *szOut)
{
    if (!strchr(szSmmcMc, '*') || szSmmcMc[0] != '*' || szSmmcMc[strlen(szSmmcMc) - 1] != '*')
        return;
    strncpy(szOut, szSmmcMc + 1, strlen(szSmmcMc) - 2);
    szOut[strlen(szSmmcMc) - 2] = '\0';
}

int EtaxInsertSpxx(cJSON *root, EHFPXX fpxx)
{
    char szBuf[BS_BIG];
    long double fPrice = 0;     //合计税额
    int nSpxxNum = 0, i = 0, nTmpNum = 0;
    cJSON *node2 = NULL, *node3 = NULL, *node4 = NULL, *node5 = NULL;
    struct ESpxx *spxx = NULL;
    cJSON_AddItemToObject(root, "mxzbList", node2 = cJSON_CreateArray());
    if (fpxx->qdbz) {
        spxx = fpxx->stp_MxxxHead;
        nSpxxNum = fpxx->spsl + 1;
        printf("11111nSpxxNum=%d\r\n",nSpxxNum);
    } else {
        spxx = fpxx->stp_MxxxHead->stp_next;
        nSpxxNum = fpxx->spsl;
        printf("11112nSpxxNum=%d\r\n",nSpxxNum);
    }
    for (i = 0; i < nSpxxNum; i++) {
        cJSON_AddItemToObject(node2, "", node3 = cJSON_CreateObject());
        //序号
        cJSON_AddNumberToObject(node3, "xh", atoi(spxx->xh));
        //发票行性质代码
        cJSON_AddStringToObject(node3, "fphxzDm", spxx->fphxz);
        //折扣序号1
        cJSON_AddNumberToObject(node3, "zkxh1", 0);
        //项目名称
        cJSON_AddStringToObject(node3, "xmmc", spxx->spmc);
        //项目名称全称
        cJSON_AddStringToObject(node3, "fullxmmc", "");
        //货物或应税劳务、服务名称
        sprintf(szBuf, "%s%s", spxx->spsmmc, spxx->spmc);
        cJSON_AddStringToObject(node3, "hwhyslwfwmc", szBuf);
        //商品和服务税收分类合并编码
        cJSON_AddStringToObject(node3, "sphfwssflhbbm", spxx->spbh);
        //商品服务简称
        GetSpsmmcWithoutStar(spxx->spsmmc, szBuf);
        cJSON_AddStringToObject(node3, "spfwjc", szBuf);
        //规格型号
        cJSON_AddStringToObject(node3, "ggxh", spxx->ggxh);
        //单位
        cJSON_AddStringToObject(node3, "dw", spxx->jldw);
        //商品数量
        cJSON_AddStringToObject(node3, "spsl", spxx->sl);
        //单价
        cJSON_AddStringToObject(node3, "dj", spxx->dj);
        //金额[原版数值]
        cJSON_AddStringToObject(node3, "je", spxx->je);
        //含税金额[原版数值]
        cJSON_AddStringToObject(node3, "hsje", spxx->hsje);
        //税率[原版数值]
        if(atof(spxx->slv) * 100 != 0)
            cJSON_AddStringToObject(node3, "slv", spxx->slv);
        else 
            cJSON_AddStringToObject(node3, "slv", "0");
        //税额[原版数值]
        cJSON_AddStringToObject(node3, "se", spxx->se);
        fPrice = fPrice + strtold(spxx->se, NULL);
        printf("fPrice = %.24Lf,spxx->se = %.24Lf\r\n",fPrice,strtold(spxx->se, NULL));
        //扣除额
        cJSON_AddStringToObject(node3, "kce", "");
        //即征即退比例
        cJSON_AddStringToObject(node3, "jzjtbl", "");
        //即征即退超税负率
        cJSON_AddStringToObject(node3, "jzjtcsfl", "");
        //零税率标志
        cJSON_AddStringToObject(node3, "lslbz", "");

        //免税
        if((atof(spxx->slv) * 100 == 0) && (atof(spxx->lslvbs) == 1))
        {
            //享受优惠政策标志
            cJSON_AddStringToObject(node3, "xsyhzcbz", "Y");
            //享受优惠政策类型代码
            cJSON_AddStringToObject(node3, "ssyhzclxDm", "03");
            //增值税特殊管理
            cJSON_AddStringToObject(node3, "zzstsgl", "03");
            //特定征收方式代码
            cJSON_AddStringToObject(node3, "tdzsfsDm", "04");
        }
        else 
        {
            //享受优惠政策标志
            cJSON_AddStringToObject(node3, "xsyhzcbz", "N");
            //享受优惠政策类型代码
            cJSON_AddStringToObject(node3, "ssyhzclxDm", "");
            //增值税特殊管理
            cJSON_AddStringToObject(node3, "zzstsgl", "");
            //特定征收方式代码
            cJSON_AddStringToObject(node3, "tdzsfsDm", "");
            //征收品目代码
            cJSON_AddStringToObject(node3, "zspmDm", "");            
        }



        //特定要素类型代码
        cJSON_AddStringToObject(node3, "tdyslxDm", "");

        //增值税政策依据代码
        cJSON_AddStringToObject(node3, "zzszcyjDm", "");
        //红字发票对应蓝字发票明细序号
        cJSON_AddStringToObject(node3, "hzfpdylzfpmxxh", "");
        //含税单价[原版数值]
        cJSON_AddStringToObject(node3, "hsdj", spxx->hsdj);
        //不含税单价
        cJSON_AddStringToObject(node3, "bhsdj", spxx->bhsdj);
        //不含税金额[原版数值]
        cJSON_AddStringToObject(node3, "bhsje", spxx->bhsje);
        //税率属性
        cJSON_AddItemToObject(node3, "slvOptions", node4 = cJSON_CreateArray());
        cJSON_AddItemToObject(node4, "", node5 = cJSON_CreateObject());
        nTmpNum = strlen(spxx->slv);
        nTmpNum = (nTmpNum > 4 ? nTmpNum : 4);
        sprintf(szBuf, "%.*f%%", nTmpNum - 4, atof(spxx->slv) * 100);
        cJSON_AddStringToObject(node5, "label", szBuf);
        if(atof(spxx->slv) * 100 != 0)
            cJSON_AddStringToObject(node5, "value", spxx->slv);
        else 
            cJSON_AddStringToObject(node5, "value","0");
        //自动税率
        cJSON_AddStringToObject(node3, "autoSlv", "");
        //报错描述标志?
        cJSON_AddStringToObject(node3, "bcmsbz", "");
        //即征即退类型代码
        cJSON_AddStringToObject(node3, "jzjtlxDm", "");
        //税率报错标志?
        if(atof(spxx->slv) * 100 != 0)
            cJSON_AddNullToObject(node3, "slvbcbz");
        else 
            cJSON_AddStringToObject(node3, "slvbcbz", "01");
        //?
        cJSON_AddStringToObject(node3, "fylb", "");
        //?
        cJSON_AddStringToObject(node3, "zfbl", "");
        //?
        cJSON_AddStringToObject(node3, "slqdbz", "Y");
        spxx = spxx->stp_next;
    }
    long double fhjse= 0;     //合计税额
    fhjse = strtold(fpxx->se, NULL);
    printf("fhjse = %.24Lf\r\n",fhjse);
    if(fPrice != fhjse)                //商品税额与合计税额比较
    {
        printf("fPrice = %.24Lf,fhjse = %.24Lf\r\n",fabs(fPrice),fabs(fhjse));
        if(fabs(fPrice)-fabs(fhjse) >= 0.06){           //差值大于6分钱
            printf("商品税额与合计税额比较差值大于6分钱\r\n");
            return -1;
        }
        if(fabs(fhjse)-fabs(fPrice) >= 0.06){           //差值大于6分钱
            printf("商品税额与合计税额比较差值大于6分钱\r\n");
            return -1;
        }
    }
    return RET_SUCCESS;
}

//必须以新发票开具格式json为准，发票复制json时会代入旧的无效数据(这些旧数据我们也无法获取到)
char *GetOfficialKpJsonFromFpxx(HETAX hEtax, EHFPXX fpxx, char *szKjlp, HDEV hDev, cJSON *root1,char *errinfo_uft8)
{
    // 1)!!!注意，很重要 https://dppt-url/assets_res/js/index.77ab0810.js:formatted.大约22297行左右
    //搜索''['concat'](_ 字样可以找到开票报文未加密
    //或者搜索'jmbw'有3个，在第二个处下断点即可找到开票报文未加密数据
    // 2)函数内部部分涉及金额相关，因为json在cJSON_Print时，会将浮点数例如50.01\0.06转换字符串时，转换为50.010000\0.060000这种高精度格式
    //税务局会判定该数值错误,只能将cJSON_AddNumberToObject改为cJSON_AddStringToObject，利用字符串格式插入，虽然数据格式上与税局原版略有区别，但是税局可识别
    // 3)最终json字符串，null和""无差异，官方都能识别
    int nChildRet = 0, i = 0;
    int count = 0;
    cJSON *root = NULL, *node2 = NULL, *node3 = NULL;
    char *pRetJsonBuf = NULL, szBuf[BS_BIG];
    do {
        if (!(root = cJSON_CreateObject()))
            break;
        //二维码ID
        cJSON_AddStringToObject(root, "ewmId", "");
        //发票票种代码
        sprintf(szBuf, "%02d", fpxx->fplx_etax);
        cJSON_AddStringToObject(root, "fppzDm", szBuf);
        //特定要素类型代码
        // 01:成品油发票|02:稀土发票|03:建筑服务发票|04:货物运输服务发票|05:不动产销售服务发票|06:不动产经营租赁服务|07:代收车船税|08:通行费|09:旅客运输服务发票|
        // 10:医疗服务（住院）发票|11:医疗服务（门诊）发票|12:自产农产品销售发票|13:拖拉机和联合收割机发票|14:机动车|15:二手车|16:农产品收购发票|17:光伏收购发票|
        // 18:卷烟发票|19:出口发票|20:农产品|21:铁路电子客票|22:航空运输电子客票行程单|31:二手车*|32:电子烟|51:正常开具|52:反向开具
        cJSON_AddStringToObject(root, "tdyslxDm", "");
        //纸票发票票种代码
        cJSON_AddStringToObject(root, "zpFppzDm", "");
        //是否为纸质发票
        cJSON_AddStringToObject(root, "sfwzzfp", "N");
        //差额征收类型代码,01:差额征税-全额开票;02:差额征税-差额开票
        cJSON_AddStringToObject(root, "cezslxDm", fpxx->cezslxDm);
        //减按征收类型代码
        cJSON_AddStringToObject(root, "jazslxDm", "");
        //销售方地址
        cJSON_AddStringToObject(root, "xsfdz", fpxx->xfdz);
        //销售方开户行
        cJSON_AddStringToObject(root, "xsfkhh", fpxx->xfkhh);
        //销售方联系电话
        cJSON_AddStringToObject(root, "xsflxdh", fpxx->xfdh);
        //销售方名称
        cJSON_AddStringToObject(root, "xsfmc", fpxx->xfmc);
        //销售方纳税人识别号
        cJSON_AddStringToObject(root, "xsfnsrsbh", fpxx->xfsh);
        //销售方社会信用代码
        cJSON_AddStringToObject(root, "xsfshxydm", "");
        //销售方银行账号
        cJSON_AddStringToObject(root, "xsfyhzh", fpxx->xfyhzh);
        // CPU序列号
        cJSON_AddNullToObject(root, "cpuid");
        //成品油单价过低
        cJSON_AddStringToObject(root, "cpydjToolow", "");
        //复核人
        cJSON_AddStringToObject(root, "fhr", fpxx->fhr);
        //发票代码
        cJSON_AddStringToObject(root, "fpdm", "");
        //纸质发号码
        cJSON_AddStringToObject(root, "zzfphm", "");
        //发票号码
        cJSON_AddStringToObject(root, "fphm", "");
        //发票开具方式代码
        cJSON_AddStringToObject(root, "fpkjfsDm", "");
        //购买方登记序号(非必须，复制开具时有，新开具不存在),
        //官方未做购方在线检测，我们暂且也不启用,接口预留EtaxApiQueryDjxxByTaxidName
        cJSON_AddStringToObject(root, "gmfdjxh", "");
        //购买方地址
        cJSON_AddStringToObject(root, "gmfdz", fpxx->gfdz);
        //购买方经办人
        cJSON_AddStringToObject(root, "gmfjbr", fpxx->gfjbr);
        //购买方开户行
        cJSON_AddStringToObject(root, "gmfkhh", fpxx->gfkhh);
        //购买方联系电话
        cJSON_AddStringToObject(root, "gmflxdh", fpxx->gfdh);
        //购买方名称
        cJSON_AddStringToObject(root, "gmfmc", fpxx->gfmc);
        //购买方纳税人识别号
        cJSON_AddStringToObject(root, "gmfnsrsbh", fpxx->gfsh);
        //购买方社会信用代码
        cJSON_AddNullToObject(root, "gmfshxydm");
        //购买方银行账号
        cJSON_AddStringToObject(root, "gmfyhzh", fpxx->gfyhzh);
        //合计金额[原版数值]
        cJSON_AddStringToObject(root, "hjje", fpxx->je);
        //合计税额[原版数值]
        cJSON_AddStringToObject(root, "hjse", fpxx->se);
        //合同编号
        cJSON_AddStringToObject(root, "htbh", "");
        // IP地址,开具时非必须，开具后税局自行加上，发票查询时会查询到
        cJSON_AddStringToObject(root, "ip", "");
        //经办人联系电话,票面无暂不支持
        cJSON_AddStringToObject(root, "jbrlxdh", "");
        //经办人身份证件号码
        cJSON_AddStringToObject(root, "jbrsfzjhm", fpxx->jbrsfzjhm);
        //经办人身份证件种类代码
        sprintf(szBuf, "%d", fpxx->jbrsfzjzldm);
        fpxx->jbrsfzjzldm == 0 ? cJSON_AddStringToObject(root, "jbrsfzjzlDm", "")
                               : cJSON_AddStringToObject(root, "jbrsfzjzlDm", szBuf);
        //金额合计
        cJSON_AddNullToObject(root, "jehj");
        //结算方式代码
        cJSON_AddStringToObject(root, "jsfsDm", "");
        //价税合计金额
        cJSON_AddStringToObject(root, "jshj", fpxx->jshj);
        //扣除额
        cJSON_AddNumberToObject(root, "kce", 0);
        //开票方纳税人识别号,二手车时可存在第三方，目前正常发票为销方税号
        cJSON_AddStringToObject(root, "kpfnsrsbh", fpxx->xfsh);
        //开票人
        cJSON_AddStringToObject(root, "kpr", fpxx->kpr);
        //开票日期
        cJSON_AddStringToObject(root, "kprq", "");
        //开票人实人认证电子信息
        cJSON_AddNullToObject(root, "kprsrrzdzxx");
        // MAC地址
        cJSON_AddStringToObject(root, "macdz", "");
        //纳税人地区码
        cJSON_AddNullToObject(root, "nsrdqm");
        //纳税义务发生时间
        cJSON_AddNullToObject(root, "nsywfssj");
        //平台编码
        cJSON_AddStringToObject(root, "ptbm", "");
        //是否蓝字发票标志
        cJSON_AddStringToObject(root, "sflzfpbz", fpxx->isRed ? "N" : "Y");
        //收购发票类型代码
        cJSON_AddStringToObject(root, "sgfplxDm", "");
        //手机开票电子信息
        cJSON_AddStringToObject(root, "sjkpdzxx", "");
        //收款人
        cJSON_AddStringToObject(root, "skr", fpxx->skr);
        //收款银行
        cJSON_AddStringToObject(root, "skyh", "");
        //收款银行名称
        cJSON_AddStringToObject(root, "skyhmc", "");
        //收款账号
        cJSON_AddStringToObject(root, "skzh", "");
        //商品数量
        cJSON_AddNumberToObject(root, "spsl", fpxx->spsl);
        //实人认证id
        cJSON_AddNullToObject(root, "srrzId");
        //销售方登记序号(非必须，复制开具时有，新开具不存在)
        cJSON_AddStringToObject(root, "xsfDjxh", "");
        //备注
        cJSON_AddStringToObject(root, "bz", fpxx->bz);
        //当前模板
        cJSON_AddStringToObject(root, "curTemplate", "");
        //含税标志,和经典开具有点区别，默认不含税==1，==2时为含税
        sprintf(szBuf, "%d", atoi(fpxx->hsjbz) + 1);
        cJSON_AddStringToObject(root, "hsbz", szBuf);
        //开具理由
        if(fpxx->sslkjly == 3)
            cJSON_AddStringToObject(root, "kjly", "02");
        else 
            cJSON_AddStringToObject(root, "kjly", "");           
        //购买方自然人标识
        cJSON_AddStringToObject(root, "gmfZrrbs", fpxx->gfzrr ? "Y" : "N");
        //销售方自然人标识，暂不加入
        cJSON_AddStringToObject(root, "xsfZrrbs", "N");
        //二手车销售统一发票纸质发票代码
        cJSON_AddStringToObject(root, "escxstyfpZzfpdm", "");
        //二手车销售统一发票纸质发票号码
        cJSON_AddStringToObject(root, "escxstyfpZzfphm", "");
        //二手车销售统一发票号码
        cJSON_AddStringToObject(root, "escxstyfpFphm", "");
        //红字x票标志?
        cJSON_AddStringToObject(root, "hzxpbz", "N");
        //++插入商品信息节点
        if(EtaxInsertSpxx(root, fpxx) != 0)     //税额有问题
        {
            sprintf((char *)errinfo_uft8, "306商品税额与合计税额比较差值大于6分钱");  
            break;
        }
        //差额凭证明细列表
        cJSON_AddNullToObject(root, "cepzmxList");
        //建筑服务特定要素
        cJSON_AddNullToObject(root, "jzfwTdys");
        //建筑服务发票明细列表
        cJSON_AddNullToObject(root, "jzfwfpmxList");
        //不动产特定要素
        cJSON_AddItemToObject(root, "bdcTdys", node2 = cJSON_CreateObject());
        //租赁期起止
        cJSON_AddStringToObject(node2, "zlqqz", "");
        //不动产特定要素明细列表
        cJSON_AddNullToObject(root, "bdcMxTdysList");
        //货物运输服务电子发票明细表
        cJSON_AddItemToObject(root, "hwysfwdzfpmxbList", cJSON_CreateArray());
        //旅客运输服务特定要素列表
        cJSON_AddItemToObject(root, "lkysfwTdysList", cJSON_CreateArray());
        //电子车船税特定要素
        cJSON_AddItemToObject(root, "dzccsTdys", node2 = cJSON_CreateObject());
        //收款x税期?
        cJSON_AddStringToObject(node2, "skssq", "");
        //通行费列表
        cJSON_AddNullToObject(root, "txfList");
        //拖拉机和联合收割机
        cJSON_AddNullToObject(root, "tljhLhsgj");
        //机动车销售特定要素列表
        cJSON_AddNullToObject(root, "jdcxsTdysList");
        //二手车销售特定要素
        cJSON_AddItemToObject(root, "escxsTdys", node2 = cJSON_CreateObject());
        //二手车销售特定要素列表
        cJSON_AddNullToObject(root, "escxsTdysList");
        // yl服务
        cJSON_AddItemToObject(root, "ylfw", node2 = cJSON_CreateObject());
        // zy时间
        cJSON_AddStringToObject(node2, "zysj", "");
        // sl清单标志
        cJSON_AddStringToObject(root, "slqdbz", "");
        // sfzs购买方银行账号
        cJSON_AddStringToObject(root, "sfzsgmfyhzh", "");
        // sfzs销售方银行账号
        cJSON_AddStringToObject(root, "sfzsxsfyhzh", "");
        //纳税人识别号
        cJSON_AddNullToObject(root, "nsrsbh");
        //查询张数
        cJSON_AddNullToObject(root, "cxzs");
        //发票号码起
        cJSON_AddNullToObject(root, "fphmq");
        //发票号码止
        cJSON_AddNullToObject(root, "fphmz");
        //页码
        cJSON_AddNullToObject(root, "pageNumber");
        //页大小
        cJSON_AddNullToObject(root, "pageSize");
        //二维码id
        cJSON_AddNullToObject(root, "ewmid");
        //发票票种类
        cJSON_AddStringToObject(root, "fppzl", "01");
        //企业代码
        cJSON_AddNullToObject(root, "qydm");
        //应税行为发生地
        cJSON_AddStringToObject(root, "ysxwfsd", "");
        //主板序列号
        cJSON_AddNullToObject(root, "zbxlh");
        //价税合计中文 API Num2ChnUpr
        cJSON_AddStringToObject(root, "jshjchn", "");
        //销售方主管税务所（科、分局）代码
        cJSON_AddStringToObject(root, "xsfZgswskfjDm", "");
        //售方主管税务局代码
        cJSON_AddStringToObject(root, "xsfZgswjDm", "");
        //销售方地市级税务机关代码
        cJSON_AddStringToObject(root, "xsfDsjswjgDm", "");
        //销售方省市级税务机关代码
        cJSON_AddStringToObject(root, "xsfSsjswjgDm", "");
        //录入日期
        cJSON_AddNullToObject(root, "lrrq");
        //二维码
        cJSON_AddStringToObject(root, "ewm", "");
        //开具红字发票对应的蓝字发票号码
        cJSON_AddNullToObject(root, "kjhzfpdydlzfphm");
        //开具红字发票对应纸质发票代码
        cJSON_AddNullToObject(root, "kjhzfpdyzzfpdm");
        //开具红字发票对应纸质发票号码
        cJSON_AddNullToObject(root, "kjhzfpdyzzfphm");
        //作废标志1
        cJSON_AddNullToObject(root, "zfbz1");
        //出口退税类型代码
        cJSON_AddStringToObject(root, "cktslxDm", "");
        //附加要素明细
        cJSON_AddItemToObject(root, "fjysList", node2 = cJSON_CreateArray());

        cJSON *jsonItem = NULL,*row=NULL,*jsonNode=NULL;

        count = cJSON_GetArraySize(root1);
        printf("count = %d\r\n",count);
        for(i=0;i<count;i++)
        {
            cJSON_AddItemToArray(node2,row = cJSON_CreateObject());

            jsonItem = cJSON_GetArrayItem(root1, i);

            jsonNode = cJSON_GetObjectItem(jsonItem, "fjysmc");
            cJSON_AddStringToObject(row, "fjysmc", jsonNode->valuestring);


            jsonNode = cJSON_GetObjectItem(jsonItem, "sjlx1");
            cJSON_AddStringToObject(row, "sjlx1", jsonNode->valuestring);

            printf("jsonNode->valuestring=%s\r\n",jsonNode->valuestring);

            if(strcmp(jsonNode->valuestring,"number")!=0){
                jsonNode = cJSON_GetObjectItem(jsonItem, "fjysz");
                cJSON_AddStringToObject(row, "fjysz", jsonNode->valuestring);               
            }
            else{
                jsonNode = cJSON_GetObjectItem(jsonItem, "fjysz");
                cJSON_AddNumberToObject(row, "fjysz", jsonNode->valueint);
            }
        }
        
        //发票标签
        cJSON_AddItemToObject(root, "fpbq", node2 = cJSON_CreateObject());
        //出口退税特定要素明细
        cJSON_AddNullToObject(root, "cktsTdysList");
        //发票代开特定要素明细
        cJSON_AddNullToObject(root, "fpdkTdysList");
        // yl服务电子发票明细VO列表
        cJSON_AddNullToObject(root, "ylfwdzfpmxVOList");
        // mg政策忽略标志
        cJSON_AddStringToObject(root, "mgzcIgnoreBz", "");
        //官方信息确认
        cJSON_AddStringToObject(root, "gfxxConfirm", "");
        //?
        cJSON_AddStringToObject(root, "sxedDefptxgz", "");
        //经办人国家代码
        sprintf(szBuf, "%d", fpxx->jbrgjdm);
        fpxx->jbrgjdm == 0 ? cJSON_AddStringToObject(root, "jbrgjDm", "")
                           : cJSON_AddStringToObject(root, "jbrgjDm", szBuf);
        //经办人-自然人纳税人识别号
        cJSON_AddStringToObject(root, "jbrZrrNsrsbh", fpxx->jbrzrrnsrsbh);
        //地址
        cJSON_AddStringToObject(root, "dz", fpxx->gfdz);
        //银行营业网点名称
        cJSON_AddStringToObject(root, "yhyywdmc", fpxx->gfkhh);
        //联系电话
        cJSON_AddStringToObject(root, "lxdh", fpxx->gfdh);
        //银行账号
        cJSON_AddStringToObject(root, "yhzh", fpxx->gfyhzh);
        //自然人标识,是否卖方自然人也会Y，暂无环境测试
        cJSON_AddStringToObject(root, "zrrbs", fpxx->gfzrr ? "Y" : "N");
        // email
        cJSON_AddStringToObject(root, "email", "");
        //开具流水号
        cJSON_AddStringToObject(root, "kjlp", szKjlp);
        // qtzj号码
        cJSON_AddStringToObject(root, "qtzjhm", "");
        if (!(pRetJsonBuf = cJSON_Print(root)))
            break;
        //_WriteHexToDebugFile("etax_makeinvoice.my.json", pRetJsonBuf, strlen(pRetJsonBuf));
        printf("pRetJsonBuf=%s\r\n",pRetJsonBuf);
    } while (false);
    if (root)
        cJSON_Delete(root);
    return pRetJsonBuf;
}

//【临时接口】后期如果除了开票接口外其他接口仍有使用，考虑和DpptEncryptIo合并
int DpptEncryptIoHmb(HETAX hEtax, char *szUrlPathQuery, char *szPostJsonString)
{
    int nChildRet = ERR_GENERIC, nRet = ERR_TA_REPONSE_CHECK, nBufSize = 0;
    char szBuf[BS_HUGE], *pBuf = NULL, *pRep = NULL;
    EHHTTP hi = &hEtax->dppt;
    do {
        if (!(pBuf = DpptGetLosHeaderAndBody(hi, szUrlPathQuery, szPostJsonString, szBuf,
                                             sizeof(szBuf))))
            break;
        //printf("DpptEncryptIoHmb szBuf=%s\r\n,pBuf=%s\r\n",szBuf,pBuf);
        if ((nChildRet = DpptHttpIo(hi, szBuf, strlen(szPostJsonString) > 0, true, pBuf)) < 0)
            break;
        nRet = nChildRet;
    } while (false);
    if (pBuf)
        free(pBuf);
    return nRet;
}

int EtaxMakeInvoiceMain(HETAX hEtax, EHFPXX fpxx, HDEV hDev,char *errinfo_uft8,char *szJsonStr)
{
    int nRet = ERR_GENERIC, nChildRet = 0;
    char *pKpClearJson = NULL, *pRepJson = NULL;
    cJSON *root = NULL, *jItem = NULL;
    cJSON *root1 = NULL, *jItem1 = NULL;
    do {
        EtaxUpdateDevInfoToFPXX(hDev, fpxx);
        //获取开票流水kjlp
        if ((nChildRet = DpptEncryptIo(hEtax, "/kpfw/fjxx/v1/kjlp/get", "")) < 0){
            sprintf((char *)errinfo_uft8, "406开具流水号获取失败,%s",(const char *)evbuffer_pullup(hEtax->dppt.bufHttpRep, -1));    
            break;
        }    
        if (!(root = cJSON_Parse((const char *)evbuffer_pullup(hEtax->dppt.bufHttpRep, -1))) ||
            !(jItem = cJSON_GetObjectItem(root, "Response")) ||
            !(jItem = cJSON_GetObjectItem(jItem, "Data")))
            break;

        if (!(root1 = cJSON_Parse((const char *)szJsonStr))) {
            break;
        }
        if (!(jItem1 = cJSON_GetObjectItem(root1, "fjysList")))
            break;

        if (!(pKpClearJson = GetOfficialKpJsonFromFpxx(hEtax, fpxx, jItem->valuestring,hDev,jItem1,errinfo_uft8))){
            if(!strstr(errinfo_uft8,"306")){
                sprintf((char *)errinfo_uft8, "306发票开具组包失败,请检查发票数据");   
            }   
            break;
        }
        //printf("pKpClearJson=%s\r\n",pKpClearJson);
        if ((nChildRet = DpptEncryptIoHmb(hEtax, "/kpfw/lzfpkj/v1/tyfpkj", pKpClearJson)) < 0) {
            sprintf((char *)errinfo_uft8, "406发票开具失败,%s",(const char *)evbuffer_pullup(hEtax->dppt.bufHttpRep, -1));   
            break;
        }
        pRepJson = evbuffer_pullup(hEtax->dppt.bufHttpRep, -1);
        if (!strstr(pRepJson, "\"Error\":null,\"Data\":")) {
            sprintf((char *)errinfo_uft8, "406发票开具失败,%s",pRepJson); 
            break;
        }
        nRet = RET_SUCCESS;
    } while (false);
    if (pKpClearJson)
        free(pKpClearJson);
    if (root)
        cJSON_Delete(root);
    return nRet;
}

int TPassLoginPasswordLoginDoChongqing(EHHTTP hiTpass, char *szUserIdIn, char *szPassword,char *szBuf, int nBufSize)
{
    char *pValue = NULL, szUserId[BS_LITTLE] = "";
    char pDecodeEtaxUrl[BS_NORMAL] = {0};
    int nRet = ERR_GENERIC, nCount = 0, i = 0, nChildRet = 0;
    cJSON *root = NULL, *objItem = NULL, *objItemChild = NULL, *objItemChild2 = NULL;
    CtxBridge ctx = (CtxBridge)hiTpass->ctxApp;

    do {
        if (!(root = cJSON_Parse((const char *)evbuffer_pullup(hiTpass->bufHttpRep, -1)))) {
            _WriteLog(LL_FATAL, "cJSON_Parse failed");
            break;
        }
        strcpy(szUserId, szUserIdIn);
        if (!(objItem = cJSON_GetObjectItem(root, "uuid"))) {
            _WriteLog(LL_FATAL, "No uuid");
            break;
        }
        pValue = objItem->valuestring;

        strcpy(pDecodeEtaxUrl,"https://etax.chongqing.chinatax.gov.cn/sword?ctrl=KxLoginCtrl_initNsrxx");
        snprintf(szBuf, nBufSize,
                 "{\"uuid\":\"%s\",\"client_id\":\"%s\",\"user_id\":\"%s\",\"password\":\"%s\","
                 "\"redirect_uri\":\"%s\"}",
                 pValue, ctx->szClientId, szUserId, szPassword, pDecodeEtaxUrl);
        if (TpassLogicIo(hiTpass, "/sys-api/v1.0/auth/enterprise/passwordLogin", szBuf, nBufSize)) {
            _WriteLog(LL_FATAL, "PasswordLogin failed");
            break;
        }
        nRet = RET_SUCCESS;
    } while (false);
    if (root)
        cJSON_Delete(root);
    return nRet;
}

//给etax的单点登陆认证
int TPassLoginAuthHelloAndAuthChongQing(EHHTTP hi, struct evhttp_uri *uriTpassFull)
{
    int nRet = ERR_GENERIC, nChildRet = 0;
    struct evbuffer *buf = NULL;
    CtxBridge ctx = (CtxBridge)hi->ctxApp;
    //可变重复用buf，szRepBuf最大接收7k数据,szBuf最大接收3k+数据
    char szRepBuf[9216] = "", szBuf[5120] = "";
    do {
        if (!(buf = evbuffer_new()))
            break;
        // =============================step3--serverHello=============================================
        memset(szBuf, 0, sizeof(szBuf));
        if (GetTaClientHello(1, hi->hDev, szBuf) < 0) {
            nRet = -1;
            break;
        }
        sprintf(szRepBuf, "{\"server_client_hello\":\"%s\"}", szBuf);
        if (BuildTPassFormatHMacSHA256Packet(true, 2, szRepBuf, ctx->szRandom16BytesKey, buf))
            break;
        //需要在回调函数中使用hi->ctxApp.szTmpBuf中保存的uuid
        hi->cbHttpNewReq = CallbackTPassSetHttpHeaderUuid;
        if (EtaxHTTPPost(hi, "/sys-api/v1.0/auth/cert/serverHello", "application/json",
                     (char *)evbuffer_pullup(buf, -1), evbuffer_get_length(buf), szRepBuf,
                     sizeof(szRepBuf)) < 0) {
            nRet = -2;
            break;
        }
        if (DecodeTpassReponseJson(hi, szRepBuf, "")) {
            nRet = -3;
            break;
        }
        // =============================step4--serverAuth=============================================
        if (TPassLoginAuthDecodeStep3RepForStep4ChongQing(hi, uriTpassFull, ctx->szRandom16BytesKey,
                                                 szRepBuf, sizeof(szRepBuf))) {
            nRet = -4;
            break;
        }
        //printf("lbc-debug TPassLoginAuthSso szRepBuf=%s\n",szRepBuf);
        if (TpassLogicIo(hi, "/sys-api/v1.0/auth/cert/serverAuth", szRepBuf, sizeof(szRepBuf))) {
            nRet = -5;
            break;
        }
        evbuffer_drain(hi->bufHttpRep, -1);
        evbuffer_add_printf(hi->bufHttpRep, "%s", szRepBuf);
        //_WriteHexToDebugFile("user.json", (uint8 *)szRepBuf, strlen(szRepBuf));
        nRet = RET_SUCCESS;
    } while (false);
    if (buf)
        evbuffer_free(buf);
    return nRet;
}

int TPassLoginAuthDecodeStep3RepForStep4ChongQing(EHHTTP hi, struct evhttp_uri *uriTpassFull,
                                         char *szRandomHex16Bytes, char *szRepBuf, int nRepBufSize)
{
    int nRet = ERR_GENERIC, nChildRet = 0;
    cJSON *root = NULL;
    CtxBridge ctx = (CtxBridge)hi->ctxApp;
    char szBuf[5120] = "", szUuid[BS_LITTLE] = "", *pValue = NULL, *pDecodeEtaxUrl = NULL,
         szClientIdFlag[] = "&client_id=";
    do {
        if (GetDataGramValueFromTPassReponse(szRepBuf, "", szRepBuf, nRepBufSize)){
            printf("----1----\n");
            break;
        }

        nChildRet = Str2Byte((uint8 *)szRepBuf, szRepBuf, strlen(szRepBuf));
        if (!(nChildRet = EtaxSM4Algorithm(false, 1, (uint8 *)szRepBuf, nChildRet,
                                       (uint8 *)szRandomHex16Bytes, (uint8 *)szRepBuf))){
            printf("----2----\n");
            break;
        }
        //解出 新的uuid，serverHelloResult
        if (!(root = cJSON_Parse(szRepBuf))){
            printf("----3----\n");
            break;
        }
        if (!(pValue = cJSON_GetObjectItem(root, "uuid")->valuestring)){
            printf("----4----\n");
            break;
        }
        strcpy(szUuid, pValue);  //新的业务uuid
        if (!(pValue = cJSON_GetObjectItem(root, "serverHelloResult")->valuestring)){
            printf("----5----\n");
            break;
        }
        strcpy(szRepBuf, pValue);
        //生成认证代码
        nChildRet = nRepBufSize;
        memset(szBuf, 0, sizeof(szBuf));
        if (BuildClientAuthCode((uint8 *)szRepBuf, strlen(szRepBuf), (uint8 *)szBuf, &nChildRet,
                                hi->hDev)){
            printf("----6----\n");
            break;
        }
        if (!(pValue = (char *)evhttp_uri_get_query(uriTpassFull))){
            printf("---pValue=[]---",pValue);
            printf("----7----\n");
            break;
        }

        //char szClientId[100],redirect_uri[100];
        strcpy(ctx->szClientId,"CfcG3KCCGeG44e7faC374Cb5bK3GC3d4");
        //strcpy(ctx->szClientId,ctx->szClientId");
        char pDecodeEtaxUrl_z[100];
        strcpy(pDecodeEtaxUrl_z,"https://etax.chongqing.chinatax.gov.cn/sword?ctrl=KxLoginCtrl_initNsrxx");

        //构建json
        sprintf(szRepBuf,
                "{\"client_id\":\"%s\",\"redirect_uri\":\"%s\",\"uuid\":\"%s\",\"str_client_auth\":"
                "\"%s\",\"post_type\":\"0\",\"user_type\":\"1\",\"login_type\":\"0\"}",
                ctx->szClientId, pDecodeEtaxUrl_z, szUuid, szBuf);
        //printf("TPassLoginAuthDecodeStep3RepForStep4 szRepBuf =%s\r\n",szRepBuf);
        nRet = RET_SUCCESS;
    } while (false);
    if (root)
        cJSON_Delete(root);
    if (pDecodeEtaxUrl)
        free(pDecodeEtaxUrl);
    return nRet;
}

int GetRandUrlyzmPathJiangSu(char *szOrigUrl, char *szCookieSsoToken, char *szRepTokenKey,
                      char *szOutString, int nOutBufSize)
{
    // 在assets_res/js/chunk-vendors.63bcfeb2.js文件中，搜索如下相关字符串为urljym生成函数
    // var y = ""[u(4144)](l, "+")["concat"](p), x = i[u(4350)](me, y);
    // return s + ""["concat"](s[u(2387)]("?") ? "&" : "?",
    // i[u(5107)])[u(4144)](encodeURIComponent(x))
    //
    // 54098ed1b9967923ac59+/szzhzz/swszzhCtr/v1/getYwqxbz?ruuid=1679280540989&Lydz=/digital-tax-account
    int nRet = ERR_GENERIC;
    char szBuf[BS_NORMAL] = "", szBuf2[BS_NORMAL] = "", szBuf3[BS_NORMAL] = "", *pNewBuf = NULL;
    char szRuuid[BS_TINY] = "";  //不可修改
    uint8 bRange = 0;
    // _WriteLog(LL_DEBUG, "CookieSsoToken:%s RepTokenKey:%s", szCookieSsoToken, szRepTokenKey);
    do {
        if (strstr(szOrigUrl, "&urlyzm="))
            break;
        if (!(pNewBuf = calloc(1, strlen(szOrigUrl) + 128)))
            break;
        struct timeval start;
        gettimeofday(&start, NULL);
        uint64 timeStamp = 1000 * (uint64)start.tv_sec + start.tv_usec / 1000;
        sprintf(szRuuid, "%llu", timeStamp);
        //获取cookies中dzfp-token，并逻辑分段翻转
        bRange = GetFirstIntFromSsoToken(szCookieSsoToken) + 3;  //值+3
        strcpy(szBuf2, szCookieSsoToken);
        strcpy(szBuf, szRuuid);
        ByteReverse(szBuf, strlen(szBuf));
        ByteReverse(szBuf2, strlen(szBuf2));
        strcat(szBuf, szBuf2);
        ReverseStringByNum(szBuf, bRange);
        if (0 == bRange % 2) {
            //(0, 5),(15, 20) (5, 10),(20, 25)
            memset(szBuf2, 0, sizeof(szBuf2));
            strncpy(szBuf2, szBuf + 0, 5);
            strncpy(szBuf2 + 5, szBuf + 15, 5);
            strncpy(szBuf2 + 10, szBuf + 5, 5);
            strncpy(szBuf2 + 15, szBuf + 20, 5);
        } else {
            //逻辑分段翻转szRepTokenKey=p,szCookieSsoToken翻转后为f
            strcpy(szBuf3, szRepTokenKey);
            ReverseStringByNum(szBuf3, bRange);
            ByteReverse(szBuf3, strlen(szBuf3));
            // f(0, 5), p(5, 10)), f(15, 20)), p(20, 25)
            memset(szBuf2, 0, sizeof(szBuf2));
            strncpy(szBuf2, szBuf + 0, 5);
            strncpy(szBuf2 + 5, szBuf3 + 5, 5);
            strncpy(szBuf2 + 10, szBuf + 15, 5);
            strncpy(szBuf2 + 15, szBuf3 + 20, 5);
        }
        //计算生成字符串md5
        sprintf(pNewBuf, "%s+%s?t=%s", szBuf2, szOrigUrl, szRuuid);
        CalcMD5(pNewBuf, strlen(pNewBuf), szBuf2);
        memset(szBuf, 0, sizeof(szBuf));
        Byte2Str(szBuf, (uint8 *)szBuf2, 0x10);
        sprintf(pNewBuf, "%s?t=%s", szOrigUrl, szRuuid);
        if (strlen(pNewBuf) > nOutBufSize - 1)
            break;
        strcpy(szOutString, pNewBuf);
        nRet = RET_SUCCESS;
    } while (false);
    if (pNewBuf)
        free(pNewBuf);
    return nRet;
}

int DpptJiangSuConnect(EHHTTP hi, char *szUrlPathQuery, HDEV hDev)
{
    int nChildRet = ERR_GENERIC;
    int nRet = ERR_GENERIC;
    do{
        if (!FillUploadAddrModelDynamic(TAADDR_ETAX_DPPT, hDev->szRegCode, "https://dppt.jiangsu.chinatax.gov.cn:8443/szzhzz/spHandler?cdlj=digital-tax-account", "/",
                                        &hi->modelTaAddr)) {
            printf("Get server url failed");
            nRet = -1;
            break;
        }
        if ((nChildRet = EtaxHTTPInit(hi, hDev)) || (nChildRet = EtaxHTTPOpen(hi))) {
            printf("HTTPS Init/Open failed, nRet:%d", nChildRet);
            nRet = -2;
            break;
        }
        if (!(hi->ctxApp = calloc(1, sizeof(struct ContextHttpAppBridge))))
            break;
        if (!(((struct ContextHttpAppBridge *)hi->ctxApp)->ebJsVmpWebshellcode =
                    (void *)evbuffer_new()))
            break;
        hi->cbHttpClose = EtaxBridgeClose; 
        //printf("szUrlPathQuery=%s\r\n",szUrlPathQuery);
        if(nChildRet = EtaxHTTPGetEvent(hi, szUrlPathQuery) < 0)
            break;
        //printf("bufCookies = %s\r\n",evbuffer_pullup(hi->ctxCon.bufCookies, -1));
        nRet = RET_SUCCESS;
    } while (false);
    return nRet;
}


int DpptShanghaiConnect(EHHTTP hi, char *szUrlPathQuery, HDEV hDev)
{
    int nChildRet = ERR_GENERIC;
    int nRet = ERR_GENERIC;
    do{
        if (!FillUploadAddrModelDynamic(TAADDR_ETAX_DPPT, hDev->szRegCode, "https://dppt.shanghai.chinatax.gov.cn:8443/szzhzz/spHandler?cdlj=digital-tax-account&id=80000995&code=80000995", "/",
                                        &hi->modelTaAddr)) {
            printf("Get server url failed");
            nRet = -1;
            break;
        }
        if ((nChildRet = EtaxHTTPInit(hi, hDev)) || (nChildRet = EtaxHTTPOpen(hi))) {
            printf("HTTPS Init/Open failed, nRet:%d", nChildRet);
            nRet = -2;
            break;
        }
        if (!(hi->ctxApp = calloc(1, sizeof(struct ContextHttpAppBridge))))
            break;
        if (!(((struct ContextHttpAppBridge *)hi->ctxApp)->ebJsVmpWebshellcode =
                    (void *)evbuffer_new()))
            break;
        hi->cbHttpClose = EtaxBridgeClose; 
        //printf("szUrlPathQuery=%s\r\n",szUrlPathQuery);
        if(nChildRet = EtaxHTTPGetEvent(hi, szUrlPathQuery) < 0)
            break;
        //printf("bufCookies = %s\r\n",evbuffer_pullup(hi->ctxCon.bufCookies, -1));
        nRet = RET_SUCCESS;
    } while (false);
    return nRet;
}

int DpptHeNanConnect(EHHTTP hi, char *szUrlPathQuery, HDEV hDev)
{
    int nChildRet = ERR_GENERIC;
    int nRet = ERR_GENERIC;
    do{
        if (!FillUploadAddrModelDynamic(TAADDR_ETAX_DPPT, hDev->szRegCode, "https://dppt.henan.chinatax.gov.cn:8443/szzhzz/spHandler?cdlj=digital-tax-account", "/",
                                        &hi->modelTaAddr)) {
            printf("Get server url failed");
            nRet = -1;
            break;
        }
        if ((nChildRet = EtaxHTTPInit(hi, hDev)) || (nChildRet = EtaxHTTPOpen(hi))) {
            printf("HTTPS Init/Open failed, nRet:%d", nChildRet);
            nRet = -2;
            break;
        }
        if (!(hi->ctxApp = calloc(1, sizeof(struct ContextHttpAppBridge))))
            break;
        if (!(((struct ContextHttpAppBridge *)hi->ctxApp)->ebJsVmpWebshellcode =
                    (void *)evbuffer_new()))
            break;
        hi->cbHttpClose = EtaxBridgeClose; 
        //printf("szUrlPathQuery=%s\r\n",szUrlPathQuery);
        if(nChildRet = EtaxHTTPGetEvent(hi, szUrlPathQuery) < 0)
            break;
        //printf("bufCookies = %s\r\n",evbuffer_pullup(hi->ctxCon.bufCookies, -1));
        nRet = RET_SUCCESS;
    } while (false);
    return nRet;
}

int DpptHeNanConnectBycookie(EHHTTP hi, HDEV hDev)
{
    int nChildRet = ERR_GENERIC;
    int nRet = ERR_GENERIC;
    do{
        if (!FillUploadAddrModelDynamic(TAADDR_ETAX_DPPT, hDev->szRegCode, "https://dppt.henan.chinatax.gov.cn:8443/szzhzz/spHandler?cdlj=digital-tax-account", "/",
                                        &hi->modelTaAddr)) {
            printf("Get server url failed");
            nRet = -1;
            break;
        }
        if ((nChildRet = EtaxHTTPInit(hi, hDev)) || (nChildRet = EtaxHTTPOpen(hi))) {
            printf("HTTPS Init/Open failed, nRet:%d", nChildRet);
            nRet = -2;
            break;
        }
        if (!(hi->ctxApp = calloc(1, sizeof(struct ContextHttpAppBridge))))
            break;
        if (!(((struct ContextHttpAppBridge *)hi->ctxApp)->ebJsVmpWebshellcode =
                    (void *)evbuffer_new()))
            break;
        hi->cbHttpClose = EtaxBridgeClose; 
        nRet = RET_SUCCESS;
    } while (false);
    return nRet;
}


int DpptJiangSuConnectBycookie(EHHTTP hi, HDEV hDev)
{
    int nChildRet = ERR_GENERIC;
    int nRet = ERR_GENERIC;
    do{
        if (!FillUploadAddrModelDynamic(TAADDR_ETAX_DPPT, hDev->szRegCode, "https://dppt.jiangsu.chinatax.gov.cn:8443/szzhzz/spHandler?cdlj=digital-tax-account", "/",
                                        &hi->modelTaAddr)) {
            printf("Get server url failed");
            nRet = -1;
            break;
        }
        if ((nChildRet = EtaxHTTPInit(hi, hDev)) || (nChildRet = EtaxHTTPOpen(hi))) {
            printf("HTTPS Init/Open failed, nRet:%d", nChildRet);
            nRet = -2;
            break;
        }
        if (!(hi->ctxApp = calloc(1, sizeof(struct ContextHttpAppBridge))))
            break;
        if (!(((struct ContextHttpAppBridge *)hi->ctxApp)->ebJsVmpWebshellcode =
                    (void *)evbuffer_new()))
            break;
        hi->cbHttpClose = EtaxBridgeClose; 
        nRet = RET_SUCCESS;
    } while (false);
    return nRet;
}


int DpptshanghaiConnect(EHHTTP hi, char *szUrlPathQuery, HDEV hDev,char *stateCode)
{
    int nChildRet = ERR_GENERIC;
    int nRet = ERR_GENERIC;
    do{
        if (!FillUploadAddrModelDynamic(TAADDR_ETAX_DPPT, hDev->szRegCode, "https://dppt.shanghai.chinatax.gov.cn:8443/szzhzz/spHandler?cdlj=digital-tax-account", "/",
                                        &hi->modelTaAddr)) {
            printf("Get server url failed");
            nRet = -1;
            break;
        }
        if ((nChildRet = EtaxHTTPInit(hi, hDev)) || (nChildRet = EtaxHTTPOpen(hi))) {
            printf("HTTPS Init/Open failed, nRet:%d", nChildRet);
            nRet = -2;
            break;
        }
        if (!(hi->ctxApp = calloc(1, sizeof(struct ContextHttpAppBridge))))
            break;
        if (!(((struct ContextHttpAppBridge *)hi->ctxApp)->ebJsVmpWebshellcode =
                    (void *)evbuffer_new()))
            break;
        hi->cbHttpClose = EtaxBridgeClose; 
        strcat(szUrlPathQuery,"&state=");
        strcat(szUrlPathQuery,stateCode);
        if(nChildRet = EtaxHTTPGetEvent(hi, szUrlPathQuery) < 0)
            break;
        nRet = RET_SUCCESS;
    } while (false);
    return nRet;
}

int DpptshanghaiConnectBycookie(EHHTTP hi, HDEV hDev)
{
    int nChildRet = ERR_GENERIC;
    int nRet = ERR_GENERIC;
    do{
        if (!FillUploadAddrModelDynamic(TAADDR_ETAX_DPPT, hDev->szRegCode, "https://dppt.shanghai.chinatax.gov.cn:8443/szzhzz/spHandler?cdlj=digital-tax-account", "/",
                                        &hi->modelTaAddr)) {
            printf("Get server url failed");
            nRet = -1;
            break;
        }
        if ((nChildRet = EtaxHTTPInit(hi, hDev)) || (nChildRet = EtaxHTTPOpen(hi))) {
            printf("HTTPS Init/Open failed, nRet:%d", nChildRet);
            nRet = -2;
            break;
        }
        if (!(hi->ctxApp = calloc(1, sizeof(struct ContextHttpAppBridge))))
            break;
        if (!(((struct ContextHttpAppBridge *)hi->ctxApp)->ebJsVmpWebshellcode =
                    (void *)evbuffer_new()))
            break;
        hi->cbHttpClose = EtaxBridgeClose; 
        nRet = RET_SUCCESS;
    } while (false);
    return nRet;
}

//仅仅测试连接是否有效
bool IsEtaxConnectionOKDummy(HETAX hEtax)
{
    bool ret = true;
    /*char szBuf[BS_HUGE];
    int nChildRet = ERR_GENERIC;
    //随便找个接口校验cookies有效性
    sprintf(szBuf, "/wszx-web/api/sh/config/getClientId?_=%llu", GetMicroSecondUnixTimestamp());
    if ((nChildRet = EtaxHTTPGetEvent(&hEtax->etax, szBuf)) <= 0) {
        printf("IsEtaxConnectionOKDummy failed \n");
        ret = false;
    }
    else
        printf("IsEtaxConnectionOKDummy successfully \n");*/
    return ret;
}

// szOutClientId一般32字节以上，szOutDecodeUrl必须256字节以上
int GetRedirectUrlAndClientIdFromTPassConfigString(struct evhttp_uri *uriTpassFull,
                                                   char *szOutClientId, char *szOutDecodeUrl)
{
    // inurl:https://tpass.shanghai.chinatax.gov.cn:8443/api/v1.0/auth/oauth2/login?redirect_uri=https%3A%2F%2Fetax.shanghai.chinatax.gov.cn%2Flogin-web%2Fapi%2Fthird%2Fsso%2Flogin%2Fredirect%3Fqd%3DKEXIN%26channelId%3Dweb%26goto%3D30010666&client_id=d598efbeddc7558c98fc32197114a36b&response_type=code&state=pro
    int nRet = ERR_GENERIC;
    char *pValue = NULL, *pDecodeEtaxUrl = NULL, szClientIdFlag[] = "&client_id=";
    do {
        if (!(pValue = (char *)evhttp_uri_get_query(uriTpassFull)))
            break;
        pValue = strchr(pValue, '=') + 1;
        if (!(pDecodeEtaxUrl = evhttp_decode_uri(pValue)))
            break;
        if (!(pValue = strstr(pDecodeEtaxUrl, szClientIdFlag)))
            break;
        *pValue = '\0';  //原字节截断
        pValue += strlen(szClientIdFlag);
        if (strlen(pValue) < 32)
            break;
        stpncpy(szOutClientId, pValue, 32);
        strcpy(szOutDecodeUrl, pDecodeEtaxUrl);
        nRet = RET_SUCCESS;
    } while (false);
    if (pDecodeEtaxUrl)
        free(pDecodeEtaxUrl);
    return nRet;
}

int ReuseEtaxTpassTalk(EHHTTP hi, EHHTTP hiEtax, struct evbuffer *evTpassCookies)
{
    int nRet = ERR_TA_ETAX_LOGIN, nChildRet = 0;
    char szBuf[BS_BLOCK];
    struct evhttp_uri *uriTpassFull = NULL;
    HDEV hDev = hiEtax->hDev;
    CtxBridge ctx = NULL;
    do {
        if (GetEtaxTpassAndInit(hi, hiEtax, &uriTpassFull))
            break;

        ctx = (CtxBridge)hi->ctxApp;
        evbuffer_add_printf(hi->ctxCon.bufCookies, "; %s", evbuffer_pullup(evTpassCookies, -1));

        EtaxRemoveRepeatOldCookies(hi->ctxCon.bufCookies);

        // key
        if (GetCookiesSegment(evTpassCookies, "key", szBuf, sizeof(szBuf)) || 16 != strlen(szBuf))
            break;

        strcpy(ctx->szRandom16BytesKey, szBuf);
        // token

        if (GetCookiesSegment(evTpassCookies, "token", szBuf, sizeof(szBuf)) ||
            strlen(szBuf) < 160 || !strchr(szBuf, '.'))
            break;

        strcpy(ctx->szToken, szBuf);
        // clientId

        if (GetRedirectUrlAndClientIdFromTPassConfigString(uriTpassFull, ctx->szClientId,
                                                           ctx->szBuf))
            break;

        nRet = RET_SUCCESS;
    } while (false);
    if (uriTpassFull)
        evhttp_uri_free(uriTpassFull);
    return nRet;
}


int ExportEtaxCookies(HETAX hEtax, char *szEtaxOutCookies,char *szTpassOutCookies)
{
    int nRet = ERR_GENERIC;
    int Len = 0;
    char OuttokenAndkey[1024]={0};
    do {
        if (!hEtax || !hEtax->etax.hDev || !hEtax->tpass.hDev) {
            _WriteLog(LL_FATAL, "Ptr null");
            break;
        }
        if (!hEtax->tpass.ctxApp || !hEtax->etax.ctxApp || !hEtax->etax.bufHttpRep ||
            !hEtax->tpass.bufHttpRep) {
            _WriteLog(LL_FATAL, "Ptr null2");
            break;
        }
        CtxBridge ctx = (CtxBridge)hEtax->tpass.ctxApp;
        //sprintf(szEtaxOutCookies, "%s",evbuffer_pullup(hEtax->etax.ctxCon.bufCookies, -1));
        //sprintf(szTpassOutCookies, "%s; token=%s; key=%s",evbuffer_pullup(hEtax->tpass.ctxCon.bufCookies, -1), ctx->szToken,ctx->szRandom16BytesKey);
        Len = evbuffer_get_length(hEtax->dppt.ctxCon.bufCookies);
        printf("ExportEtaxCookies Len = %d\r\n",Len);
        evbuffer_copyout_from(hEtax->etax.ctxCon.bufCookies,NULL,szEtaxOutCookies,Len);


        Len = evbuffer_get_length(hEtax->tpass.ctxCon.bufCookies);
        printf("ExportEtaxCookies Len = %d\r\n",Len);
        evbuffer_copyout_from(hEtax->tpass.ctxCon.bufCookies,NULL,szTpassOutCookies,Len);
        sprintf(OuttokenAndkey, "; token=%s; key=%s",ctx->szToken,ctx->szRandom16BytesKey);
        strcat(szTpassOutCookies,OuttokenAndkey);

        _WriteLog(LL_DEBUG, "EtaxCookies:%s TpassCookies:%s", szEtaxOutCookies, szTpassOutCookies);
        nRet = RET_SUCCESS;
    } while (false);
    return nRet;
}


int GetEtaxTpassAndInit(EHHTTP hi, EHHTTP hiEtax, struct evhttp_uri **uriTpassFull)
{
    // p.send(f)为html异步之前最后一个函数调用，发送到chrome的js异步队列中进行处理
    int nRet = ERR_TA_ETAX_LOGIN, nChildRet = 0;
    char *pValue = NULL;
    cJSON *jsonTpassConfig = NULL;
    HDEV hDev = hiEtax->hDev;  // hiEtax已初始化 hi=hiTpass尚未
    do {
        //获取tpass地址
        if (EtaxHTTPGetEvent(hiEtax, "/wszx-web/api/sh/login/mode") < 0)
            break;
        if (hiEtax->ctxCon.nHttpRepCode != 200)
            break;
        //获取跳转tpass地址
        if (!(jsonTpassConfig = cJSON_Parse((char *)evbuffer_pullup(hiEtax->bufHttpRep, -1))))
            break;
        if (!(pValue = cJSON_GetObjectItem(jsonTpassConfig, "value")->valuestring))
            break;
        if (!(*uriTpassFull = evhttp_uri_parse(pValue)))
            break;
        if (!FillUploadAddrModelDynamic(TAADDR_ETAX_TPASS, hDev->szRegCode, pValue, "/",
                                        &hi->modelTaAddr)) {
            _WriteLog(LL_WARN, "Get server url failed");
            nRet = -1;
            break;
        }
        if ((nChildRet = EtaxHTTPInit(hi, hDev)) || (nChildRet = EtaxHTTPOpen(hi))) {
            _WriteLog(LL_WARN, "HTTPS Init/Open failed, nRet:%d", nChildRet);
            nRet = -2;
            break;
        }
        if (!(hi->ctxApp = calloc(1, sizeof(struct ContextHttpAppBridge))))
            break;
        hi->cbHttpClose = EtaxBridgeClose;
        nRet = RET_SUCCESS;
    } while (false);
    if (jsonTpassConfig)
        cJSON_Delete(jsonTpassConfig);
    return nRet;
}


int CreateEtaxCookies(HETAX hEtax, HDEV hDev, char *szEtaxAllCookies, char *szTpassAllCookies,char *errinfo_uft8)
{
    int nRet = ERR_GENERIC, nChildRet = 0;
    char szBuf[BS_HUGE];
    EHHTTP hi = (EHHTTP)&hEtax->dppt;
    struct evbuffer *evEtaxCookies = NULL, *evTpassCookies = NULL;
    do {
        //参数检测
        if (!szEtaxAllCookies || strlen(szEtaxAllCookies) < 32) {
            sprintf((char *)errinfo_uft8, "406传入参数缺少EtaxCookies或者EtaxCookies长度小于32字节");  
            break;
        }

        if (!szTpassAllCookies || strlen(szTpassAllCookies) < 32) {
            sprintf((char *)errinfo_uft8, "406传入参数缺少TpassCookies或者TpassCookies长度小于32字节");  
            break;
        }

        if (!strstr(szTpassAllCookies, "key=")) {
            sprintf((char *)errinfo_uft8, "406传入TpassCookies参数中缺少必要数据 'key', 这是一个16位的字符船随机数");  
            break;
        }

        if (strchr(szEtaxAllCookies, '\n')) {
            sprintf((char *)errinfo_uft8, "406传入EtaxCookies参数中缺少必要数据 '\\n'");  
            break;
        }

        if (!(evEtaxCookies = evbuffer_new()) || !(evTpassCookies = evbuffer_new())){
            sprintf((char *)errinfo_uft8, "306机柜动态申请内存失败,请重启机柜");  
            break;
        }
            
        evbuffer_add_printf(evEtaxCookies, "%s", szEtaxAllCookies);
        if (GetCookiesSegment(evEtaxCookies, "JSESSIONID", szBuf, sizeof(szBuf)) ||
            GetCookiesSegment(evEtaxCookies, "SSO_LOGIN_TGC", szBuf, sizeof(szBuf)) ||
            GetCookiesSegment(evEtaxCookies, "DZSWJ_TGC", szBuf, sizeof(szBuf))) {
            sprintf((char *)errinfo_uft8, "406EtaxCookies 格式错误,Example: JSESSIONID=\"xxx\"; SSO_LOGIN_TGC=\"yyy\";DZSWJ_TGC=\"zzz\"");   
            break;
        }

        if ((nChildRet = CreateEtaxTalkCookies(&hEtax->etax, hDev)) < 0) {
            sprintf((char *)errinfo_uft8, "406Etax请求地址失败");   
            break;
        }

        evbuffer_add_printf(hEtax->etax.ctxCon.bufCookies, "; %s",evbuffer_pullup(evEtaxCookies, -1));

        EtaxRemoveRepeatOldCookies(hEtax->etax.ctxCon.bufCookies);

        evbuffer_add_printf(evTpassCookies, "%s", szTpassAllCookies);

        if ((nChildRet = ReuseEtaxTpassTalk(&hEtax->tpass, &hEtax->etax, evTpassCookies)) < 0) {
            sprintf((char *)errinfo_uft8, "406Tpass地址获取失败,请重新登陆刷新Cookies");        
            break;
        }

        if ((nChildRet = CreateEtaxDpptTalk(hEtax))) {
            sprintf((char *)errinfo_uft8, "406该Cookies已失效,请重新请求该接口");        
            break;
        }

        if ((nChildRet = InitSecurityConfig(hi))) {
            if(nChildRet != -8){
                sprintf((char *)errinfo_uft8, "406该Cookies已失效,请重新请求该接口");   
                break;                
            }
            else 
            {
                sprintf((char *)errinfo_uft8, "406初始化dppt服务器失败,错误状态为:%d", nChildRet);    
                break;
            }
        }

        nRet = RET_SUCCESS;
    } while (false);
    if (evEtaxCookies)
        evbuffer_free(evEtaxCookies);
    if (evTpassCookies)
        evbuffer_free(evTpassCookies);
    return nRet;
}

int get_DNS_ip(char *hostname,char *hostip)
{
	struct hostent *h;  
    struct in_addr in;  
    struct sockaddr_in addr_in; 
	struct sockaddr_in adr_inet; 
	int len;
	int ip_len;
	char hostnamebuf[100] = { 0 };
	len = strlen((char *)hostname);
	memcpy(hostnamebuf, hostname, len);

	memset(&adr_inet, 0, sizeof(adr_inet));
	if (inet_aton(hostnamebuf, &adr_inet.sin_addr))//判断是否是IP
	{
		memcpy(hostip, hostnamebuf,len);
		return len;
	}
	if(len<5)
	{
		printf("域名长度错误\n");
		return -1;
	}
	//printf("hostnamebuf %s\n", hostnamebuf);
	if (memcmp(hostnamebuf, "127.0.0.1", len) == 0)
	{
		printf("本机IP不用解析\n");
		sprintf((char *)hostip, "127.0.0.1");
		return strlen((const char*)hostip);
	}
	//signal(SIGALRM, alarm_func);
	//if (sigsetjmp(jmpbuf, 1) != 0)
	//{
	//	alarm(0); /* 取消闹钟 */
	//	signal(SIGALRM, SIG_IGN);
	//	printf("解析IP地址失败sigsetjmp\n");
	//	return -1;
	//}
	//alarm(2); /* 设置超时时间 */

	h = gethostbyname((const char *)hostnamebuf);
	//signal(SIGALRM, SIG_IGN);
	if(h==NULL)
	{
		//printf("解析IP地址失败\n");
		return -1;
	}	
	else
	{	
		memcpy(&addr_in.sin_addr.s_addr,h->h_addr,4);
		in.s_addr=addr_in.sin_addr.s_addr;
		//printf("host name	:%s\n",h->h_name);
		//printf("ip length	:%d\n",h->h_length);
		//printf("type	:%d\n",h->h_addrtype);
		//printf("ip	:%s\n",inet_ntoa(in));
	}
	ip_len = strlen(inet_ntoa(in));
	memcpy(hostip,inet_ntoa(in),ip_len);
	
	return ip_len;
}

/*********************************************************************
-   Function : get_line
-   Description：获取一行信息
-   Calls：
-   Calles By:
-   Table Accessed :
-   Table Update :
-   Input  :参数一:
-   Output :
-   Return :：错误 其他：正常
-   Other :
***********************************************************************/
static int get_line(int sock, char *buf, int size)
{
	int i = 0;
	unsigned char c = '\0';
	int n;
	//int j;

	/*把终止条件统一为 \n 换行符，标准化 buf 数组*/
	while ((i < size - 1) && (c != '\n'))
	{
		/*一次仅接收一个字节*/
		n = recv(sock, &c, 1, 0);
		/* DEBUG*/
		//printf("%02x ", c); 
		if (n > 0)
		{
			/*收到 \r 则继续接收下个字节，因为换行符可能是 \r\n */
			if (c == '\r')
			{
				/*使用 MSG_PEEK 标志使下一次读取依然可以得到这次读取的内容，可认为接收窗口不滑动*/
				n = recv(sock, &c, 1, MSG_PEEK);
				/* DEBUG printf("%02X\n", c); */
				//printf("\n但如果是换行符则把它吸收掉\n");
				if ((n > 0) && (c == '\n'))
				{
					recv(sock, &c, 1, 0);
					//printf("%02x ", c); 
				}
				else
				{
					c = '\n';
					//printf("%02x ", c); 
				}
			}
			/*存到缓冲区*/
			buf[i] = c;
			i++;
		}
		else
			c = '\n';
	}
	//printf("\n")
	return(i);
}

static int deal_http_data(int sockfd,char **outbuf)
{
	char buf[1024];
	int i;
	//int j;
	char c;
	//char num=3;
	char len_buf[50];
	int numchars = 1;  
    int content_length = -1;
	//printf("获取第一行数据\n");
	memset(buf,0,sizeof(buf));
	numchars = get_line(sockfd, buf, sizeof(buf));
	//printf("获取到的第一行数据为：%s\n",buf);
	memset(buf,0,sizeof(buf));
	numchars = get_line(sockfd, buf, sizeof(buf)); 
	//printf("获取到的第二行数据为：%s\n",buf);
    while ((numchars > 0) && strcmp("\n", buf))  
    {  
        if (strncasecmp(buf, "Content-Length:",15) == 0)  
		{	memset(len_buf,0,sizeof(len_buf));
			memcpy(len_buf,&buf[15],20);
			//printf("len_buf = %s",len_buf);
			cls_no_asc(len_buf,strlen(len_buf)); 
            content_length = atoi(len_buf);  
			//printf("Content-Length = %d\n",content_length);
		}
		memset(buf,0,sizeof(buf));
        numchars = get_line(sockfd, buf, sizeof(buf));  
		//printf("获取到的第%d行数据为：%s",num,buf);
		//num +=1;
		//for(j=0;j<numchars;j++)
		//{
		//	printf("%02x ",(unsigned char)buf[j]);
		//}
		//printf("\n");
    }  
    /*没有找到 content_length */  
	//printf("退出获取头循环\n");
    if (content_length <0) {  
        printf("错误请求,接收到的长度不对content_length = %d\n",content_length); 
        return -1;  
    }  
	 /*接收 POST 过来的数据*/  
	//if(content_length>102400)
	//{
	//	printf("超过缓存大小,无法接收\n");
	//	return -1;
	//}
	int glen=sizeof(char*)*content_length+100;
	
	char *tmp=NULL;
	tmp = malloc(glen);	
	if(tmp ==NULL)
	{	printf("分配接收内存失败\n");
		return -1;
	}
	
	memset(tmp,0,glen);
    for (i = 0; i < content_length; i++) 
	{  
        recv(sockfd, &c, 1, 0);
		tmp[i]=c;  
    }  

	*outbuf = tmp;
	//memcpy(outbuf,tmp,sizeof(tmp));
	return content_length;
}

static int open_rec_over_timer(int socketfd,int over_timer)
{   struct timeval timeout;
    //printf("设置接受超时时间为:%d\n",over_timer);
    memset(&timeout,0,sizeof(timeout));
    timeout.tv_sec=over_timer;
    return setsockopt(socketfd,SOL_SOCKET,SO_RCVTIMEO,&timeout,sizeof(timeout));
}

static int open_send_over_timer(int socketfd,int over_timer)
{   struct timeval timeout;
    //printf("设置发送超时时间为:%d\n",over_timer);
    memset(&timeout,0,sizeof(timeout));
    timeout.tv_sec=over_timer;
    return setsockopt(socketfd,SOL_SOCKET,SO_SNDTIMEO,&timeout,sizeof(timeout));
    
}

static int ty_socket_write(int sock,uint8 *buf,int buf_len)
{   int i,result;
    for(i=0;i<buf_len;i+=result)
    {   result=write(sock,buf+i,buf_len-i);
        if(result<=0)
            return result;
    }
    return i;   
}


int socket_http_netPost_post(char *hostname,int port,char *request,char *content_type,char *parmdata,int time_out,char **returndata,int *returndata_len)
{	char name[300];
	int ip_len; 
	int result;
	int socketfd;
	char s_buf[102400];
	char r_buf[102400];
	struct sockaddr_in add;
	memset(name,0,sizeof(name));
	ip_len = get_DNS_ip((char *)hostname, (char *)name);
	if(ip_len<0)
	{	printf("解析IP失败\n");
		return -1;
	}
	//printf("解析IP成功 IP为：%s\n",name);
	if((socketfd=socket(AF_INET,SOCK_STREAM,0)) == -1)
	{
		printf("创建socket失败\n");
		return -1;
	}
	memset(&add,0,sizeof(add));
	add.sin_family=AF_INET;
	add.sin_port=htons(port);
	add.sin_addr.s_addr = htonl(INADDR_ANY);
	inet_pton(AF_INET,name,&add.sin_addr);
	//printf("连接服务器:%s:%d\n",name,port);
	if((result=connect(socketfd,(struct sockaddr *)&add,sizeof(add)))==-1)
	{	printf("连接服务器%s:%d失败\n",name,port);
		close(socketfd);
		return -1;
	}	
	open_rec_over_timer(socketfd,time_out);
	open_send_over_timer(socketfd,time_out);
	memset(s_buf,0,sizeof(s_buf));
	memset(r_buf,0,sizeof(r_buf));
	sprintf(s_buf, "POST %s HTTP/1.1\r\n",request);
	//sprintf(s_buf, "POST %s HTTP/1.1\r\n","/cert/openDevice");
    sprintf(s_buf+strlen(s_buf),"Content-Type: %s\r\n",content_type); 
	//sprintf(s_buf+strlen(s_buf),"Content-Type: %s\r\n","application/json"); 
	sprintf(s_buf+strlen(s_buf),"Timeout: 5000\r\n");
	sprintf(s_buf+strlen(s_buf),"Host: %s:18001\r\n",name);
	sprintf(s_buf+strlen(s_buf),"content-Length: %d\r\n",strlen(parmdata)+2);
	sprintf(s_buf+strlen(s_buf),"User-Agent: libghttp/1.0\r\n");
    sprintf(s_buf+strlen(s_buf), "\r\n");  
    sprintf(s_buf+strlen(s_buf), "%s\r\n",parmdata);   

	//printf("-----------   s_buf = [%s]\r\n",s_buf);
	
	if (ty_socket_write(socketfd, (uint8 *)s_buf, strlen(s_buf))<0)
	{
		printf("发送失败\n");
		result =  -2;
	}

	//memset(returndata,0,sizeof(returndata));
	//printf("进入获取数据函数\n");
	result = deal_http_data(socketfd,returndata);
	//printf("退出接收处理函数\n");
	*returndata_len = result;

	shutdown(socketfd,SHUT_RDWR);
	close(socketfd); 
	return result;
}

int socket_http_netPost_get(char *hostname,int port,char *request,char *content_type,char *parmdata,int time_out,char **returndata,int *returndata_len)
{	char name[300];
	int ip_len; 
	int result;
	int socketfd;
	char s_buf[102400];
	char r_buf[102400];
	struct sockaddr_in add;
	memset(name,0,sizeof(name));
	ip_len = get_DNS_ip((char *)hostname, (char *)name);
	if(ip_len<0)
	{	printf("解析IP失败\n");
		return -1;
	}
	//printf("解析IP成功 IP为：%s\n",name);
	if((socketfd=socket(AF_INET,SOCK_STREAM,0)) == -1)
	{
		printf("创建socket失败\n");
		return -1;
	}
	memset(&add,0,sizeof(add));
	add.sin_family=AF_INET;
	add.sin_port=htons(port);
	add.sin_addr.s_addr = htonl(INADDR_ANY);
	inet_pton(AF_INET,name,&add.sin_addr);
	//printf("连接服务器:%s:%d\n",name,port);
	if((result=connect(socketfd,(struct sockaddr *)&add,sizeof(add)))==-1)
	{	printf("连接服务器%s:%d失败\n",name,port);
		close(socketfd);
		return -1;
	}	
	open_rec_over_timer(socketfd,time_out);
	open_send_over_timer(socketfd,time_out);
	memset(s_buf,0,sizeof(s_buf));
	memset(r_buf,0,sizeof(r_buf));
	sprintf(s_buf, "GET %s HTTP/1.1\r\n",request);
    sprintf(s_buf+strlen(s_buf),"Content-Type: %s\r\n",content_type);  
	sprintf(s_buf+strlen(s_buf),"Timeout: 5000\r\n");
	sprintf(s_buf+strlen(s_buf),"Host: %s:18001\r\n",name);
	sprintf(s_buf+strlen(s_buf),"content-Length: %d\r\n",strlen(parmdata)+2);
	sprintf(s_buf+strlen(s_buf),"User-Agent: libghttp/1.0\r\n");
    sprintf(s_buf+strlen(s_buf), "\r\n");  
    sprintf(s_buf+strlen(s_buf), "%s\r\n",parmdata);   

	//printf("-----------   s_buf = [%s]\r\n",s_buf);
	
	if (ty_socket_write(socketfd, (uint8 *)s_buf, strlen(s_buf))<0)
	{
		printf("发送失败\n");
		result =  -2;
	}

	//memset(returndata,0,sizeof(returndata));
	//printf("进入获取数据函数\n");
	result = deal_http_data(socketfd,returndata);
	//printf("退出接收处理函数\n");
	*returndata_len = result;


	//printf("接收到数据：%s\n",returndata);
	shutdown(socketfd,SHUT_RDWR);
	close(socketfd); 
	return result;
}

int TPassLoginAuthDecodeStep3RepForStep4_soft_cert_21(EHHTTP hi, struct evhttp_uri *uriTpassFull,char *szRandomHex16Bytes, char *szRepBuf, int nRepBufSize,char *unquid,char *soft_cert_ip,int soft_cert_port,char *tem_open,char *tem_hello)
{
    int nRet = ERR_GENERIC, nChildRet = 0;
    cJSON *root = NULL;
    CtxBridge ctx = (CtxBridge)hi->ctxApp;
    char szBuf[5120] = "", szUuid[BS_LITTLE] = "", *pValue = NULL,*tmp_pValue = NULL, *pDecodeEtaxUrl = NULL;
    char tem_uuid[100];
    int result;
    char szRepBuf_z[9*1024];
    char tmp[100] = {0};
    char content_type[100] = {0};
    char *returndata = NULL;
    int returndata_len;
    do {
        if (GetDataGramValueFromTPassReponse(szRepBuf, "", szRepBuf, nRepBufSize))
            break;
        nChildRet = Str2Byte((uint8 *)szRepBuf, szRepBuf, strlen(szRepBuf));
        if (!(nChildRet = EtaxSM4Algorithm(false, 1, (uint8 *)szRepBuf, nChildRet,
                                       (uint8 *)szRandomHex16Bytes, (uint8 *)szRepBuf)))
            break;
        //解出 新的uuid，serverHelloResult
        
        if (!(root = cJSON_Parse(szRepBuf)))
        {
            break;
        }
        if (!(pValue = cJSON_GetObjectItem(root, "uuid")->valuestring))
            break;
        strcpy(szUuid, pValue);  //新的业务uuid
        strcpy(tem_uuid,szUuid);

        if (!(pValue = cJSON_GetObjectItem(root, "serverHelloResult")->valuestring))
            break;
        strcpy(szRepBuf, pValue);
        //生成认证代码
        nChildRet = nRepBufSize;
        memset(szBuf, 0, sizeof(szBuf));

        /////////////////////////////////////////////////////  ClientAuth  /////////////////////////////////////////////////////
        printf("#########################################      ClientAuth         ############################################    \r\n");

        memset(szRepBuf_z,0,sizeof(szRepBuf_z));
        sprintf(tmp, "/api/v1/ClientAuth");  //POST 请求的地址
        sprintf(content_type, "application/json");
        sprintf(szRepBuf_z, "{\"reqid\":\"%s\",\"ServerHello\":\"%s\"}",tem_open,szRepBuf);
        printf("     【3】 AUTH  param     szRepBuf_z =  【%s】       \r\n",szRepBuf_z);
        result = socket_http_netPost_post(soft_cert_ip, soft_cert_port, tmp, content_type, szRepBuf_z, 5, &returndata, &returndata_len);
        if (result < 0 || returndata_len <= 0)
        {
            if (returndata != NULL)
            {
                free(returndata);
                returndata = NULL;
            }
            nRet = -1;
        }
        printf("#######################################  ClientAuth returndata =  %s \r\n",returndata);

        char *tem_p = NULL;
        char *tem_p_2 = NULL;
        char *pValue = NULL;
        char tem_data[2048] = {0};
        //获取Code
        memset(tem_data,0,sizeof(tem_data));

        cJSON *root3 = NULL;                     
        if (!(root3 = cJSON_Parse((const char *)returndata))) {          //步骤1 json初始化
            break;
        }
        if (GetAvailImportDataKp(root3, "Code", tem_data, sizeof(tem_data), true) < 0) {       //步骤2  导入code
            break;
        }

        if (0 == strcmp(tem_data,"200"))
        {
            printf("请求成功！\r\n");
        }
        else
        {
            printf("请求失败：%s\r\n",tem_data);
            if(returndata)
                free(returndata);
		    returndata = NULL;
            if(root3)
                cJSON_Delete(root3);
            break;
        }

        cJSON *object_data3 = cJSON_GetObjectItem(root3, "Data");
        if (object_data3 == NULL)
        {
            printf("请求Data失败\r\n");
            if(returndata)
                free(returndata);
		    returndata = NULL;
            if(root3)
                cJSON_Delete(root3);
            break;
        }     
        printf("ClientAuth Data:%s\r\n",object_data3->valuestring);

        //cJSON *item2 = NULL;  
        // item = cJSON_GetObjectItem(object_data3, "code");
        // printf("area_code:%s\r\n",item2->valuestring);
        
        //item = cJSON_GetObjectItem(object_data3, "reqid");
        //printf("Data:%s\r\n",object_data3->valuestring);        
        
        char tem_auth[2048] = {0};
        strcpy(tem_auth,object_data3->valuestring);
        printf("   AUTH  END  END      tem_auth = [ %s ]               \r\n",tem_auth);

        printf("##########################################      deal with    【client_id  and  redirect_uri】 START   ########################################## \r\n");
        if(NULL == uriTpassFull)
        {
            break;
        }
        if (!(pValue = (char *)evhttp_uri_get_query(uriTpassFull)) && hi->user_args == NULL)
            break;
        if(pValue == NULL && hi->user_args != NULL)
        {
            pValue = hi->user_args;
        }
        
        //获取client_id
        if (!(tmp_pValue = strstr(pValue, "&client_id=")))
        {
            break;
        }
        tmp_pValue += strlen("&client_id=");
        if (strlen(tmp_pValue) < 32)
            break;
        stpncpy(ctx->szClientId, tmp_pValue, 32);
        //获取redirect_uri
        tmp_pValue = strstr(pValue,"redirect_uri=");
        if(tmp_pValue == NULL)
            break;
        if (!(pDecodeEtaxUrl = evhttp_decode_uri(tmp_pValue + strlen("redirect_uri="))))
            break;
        strcpy(ctx->szBuf, pDecodeEtaxUrl);
        
        printf("    ctx->szClientId = [ %s ]                pDecodeEtaxUrl = [ %s ]\r\n",ctx->szClientId,pDecodeEtaxUrl);
        printf("##########################################      deal with    【client_id  and  redirect_uri】  END  ########################################## \r\n");

        // strcpy(ctx->szClientId,"d74f6Xacf6434c36acbepe4e6fpbcf8f");
        // //strcpy(ctx->szClientId,ctx->szClientId");
        // char pDecodeEtaxUrl_z[100];
        // strcpy(pDecodeEtaxUrl_z,"https://etax.shanxi.chinatax.gov.cn/tpass/sso/loginSubmit");
        
        //构建json
        sprintf(szRepBuf,
                "{\"client_id\":\"%s\",\"redirect_uri\":\"%s\",\"uuid\":\"%s\",\"str_client_auth\":"
                "\"%s\",\"post_type\":\"0\",\"user_type\":\"1\",\"login_type\":\"0\"}",
                ctx->szClientId, pDecodeEtaxUrl, tem_uuid, tem_auth);

        /////////////////////////////////////////////////////  closeDevice  /////////////////////////////////////////////////////
        memset(szRepBuf_z,0,sizeof(szRepBuf_z));
        memset(tmp,0,sizeof(tmp));
        sprintf(tmp,"/api/v1/CloseCert");  //POST 请求的地址

        sprintf(szRepBuf_z, "{\"reqid\":\"%s\"}",tem_open); 
        printf("     【4】 Close  param     szRepBuf_z  = [%s]      \r\n",szRepBuf_z);
        result = socket_http_netPost_post(soft_cert_ip, soft_cert_port, tmp, "application/json", szRepBuf_z, 5, &returndata, &returndata_len);
        if (result < 0 || returndata_len <= 0)
        {
            if (returndata != NULL)
            {
                free(returndata);
                returndata = NULL;
            }
            nRet = -1;
        }
        printf("###################################  CloseCert returndata =  %s \r\n",returndata);

        memset(tem_data,0,sizeof(tem_data));
        tem_p = strstr(returndata,"Code\":\"");
        if(tem_p == NULL)
            break;
        tem_p += 7;
        stpncpy(tem_data, tem_p, 3);
        if (0 == strcmp(tem_data,"200"))
        {
            printf("           【请求成功！】         \r\n");
        }
        else
        {
            printf("请求失败：%s\r\n",tem_data);
            if(returndata)
                free(returndata);
		    returndata = NULL;
            break;
        }    
        
        free(returndata);
        returndata = NULL;
        
        nRet = RET_SUCCESS;
    } while (false);
    if (root)
        cJSON_Delete(root);
    if (pDecodeEtaxUrl)
        free(pDecodeEtaxUrl);
    return nRet;
}

int TPassLoginAuthDecodeStep3RepForStep4_soft_cert_21_shaanxi(EHHTTP hi, struct evhttp_uri *uriTpassFull,char *szRandomHex16Bytes, char *szRepBuf, int nRepBufSize,char *unquid,char *soft_cert_ip,int soft_cert_port,char *tem_open,char *tem_hello)
{
    int nRet = ERR_GENERIC, nChildRet = 0;
    cJSON *root = NULL;
    CtxBridge ctx = (CtxBridge)hi->ctxApp;
    char szBuf[5120] = "", szUuid[BS_LITTLE] = "", *pValue = NULL,*tmp_pValue = NULL, *pDecodeEtaxUrl = NULL;
    char tem_uuid[100];
    int result;
    char szRepBuf_z[9*1024];
    char tmp[100] = {0};
    char content_type[100] = {0};
    char *returndata = NULL;
    int returndata_len;
    do {
        if (GetDataGramValueFromTPassReponse(szRepBuf, "", szRepBuf, nRepBufSize))
            break;
        nChildRet = Str2Byte((uint8 *)szRepBuf, szRepBuf, strlen(szRepBuf));
        if (!(nChildRet = EtaxSM4Algorithm(false, 1, (uint8 *)szRepBuf, nChildRet,
                                       (uint8 *)szRandomHex16Bytes, (uint8 *)szRepBuf)))
            break;
        //解出 新的uuid，serverHelloResult
        
        if (!(root = cJSON_Parse(szRepBuf)))
        {
            break;
        }
        if (!(pValue = cJSON_GetObjectItem(root, "uuid")->valuestring))
            break;
        strcpy(szUuid, pValue);  //新的业务uuid
        strcpy(tem_uuid,szUuid);

        if (!(pValue = cJSON_GetObjectItem(root, "serverHelloResult")->valuestring))
            break;
        strcpy(szRepBuf, pValue);
        //生成认证代码
        nChildRet = nRepBufSize;
        memset(szBuf, 0, sizeof(szBuf));

        /////////////////////////////////////////////////////  ClientAuth  /////////////////////////////////////////////////////
        printf("#########################################      ClientAuth         ############################################    \r\n");

        memset(szRepBuf_z,0,sizeof(szRepBuf_z));
        sprintf(tmp, "/api/v1/ClientAuth");  //POST 请求的地址
        sprintf(content_type, "application/json");
        sprintf(szRepBuf_z, "{\"reqid\":\"%s\",\"ServerHello\":\"%s\"}",tem_open,szRepBuf);
        printf("     【3】 AUTH  param     szRepBuf_z =  【%s】       \r\n",szRepBuf_z);
        result = socket_http_netPost_post(soft_cert_ip, soft_cert_port, tmp, content_type, szRepBuf_z, 5, &returndata, &returndata_len);
        if (result < 0 || returndata_len <= 0)
        {
            if (returndata != NULL)
            {
                free(returndata);
                returndata = NULL;
            }
            nRet = -1;
        }
        printf("#######################################  ClientAuth returndata =  %s \r\n",returndata);

        char *tem_p = NULL;
        char *tem_p_2 = NULL;
        char *pValue = NULL;
        char tem_data[2048] = {0};
        //获取Code
        memset(tem_data,0,sizeof(tem_data));

        cJSON *root3 = NULL;                     
        if (!(root3 = cJSON_Parse((const char *)returndata))) {          //步骤1 json初始化
            break;
        }
        if (GetAvailImportDataKp(root3, "Code", tem_data, sizeof(tem_data), true) < 0) {       //步骤2  导入code
            break;
        }

        if (0 == strcmp(tem_data,"200"))
        {
            printf("请求成功！\r\n");
        }
        else
        {
            printf("请求失败：%s\r\n",tem_data);
            if(returndata)
                free(returndata);
		    returndata = NULL;
            if(root3)
                cJSON_Delete(root3);
            break;
        }

        cJSON *object_data3 = cJSON_GetObjectItem(root3, "Data");
        if (object_data3 == NULL)
        {
            printf("请求Data失败\r\n");
            if(returndata)
                free(returndata);
		    returndata = NULL;
            if(root3)
                cJSON_Delete(root3);
            break;
        }     
        printf("ClientAuth Data:%s\r\n",object_data3->valuestring);

        //cJSON *item2 = NULL;  
        // item = cJSON_GetObjectItem(object_data3, "code");
        // printf("area_code:%s\r\n",item2->valuestring);
        
        //item = cJSON_GetObjectItem(object_data3, "reqid");
        //printf("Data:%s\r\n",object_data3->valuestring);        
        
        char tem_auth[2048] = {0};
        strcpy(tem_auth,object_data3->valuestring);
        printf("   AUTH  END  END      tem_auth = [ %s ]               \r\n",tem_auth);

        printf("##########################################      deal with    【client_id  and  redirect_uri】 START   ########################################## \r\n");
        // if(NULL == uriTpassFull)
        // {
        //     break;
        // }
        // if (!(pValue = (char *)evhttp_uri_get_query(uriTpassFull)) && hi->user_args == NULL)
        //     break;
        // if(pValue == NULL && hi->user_args != NULL)
        // {
        //     pValue = hi->user_args;
        // }
        
        // //获取client_id
        // if (!(tmp_pValue = strstr(pValue, "&client_id=")))
        // {
        //     break;
        // }
        // tmp_pValue += strlen("&client_id=");
        // if (strlen(tmp_pValue) < 32)
        //     break;
        // stpncpy(ctx->szClientId, tmp_pValue, 32);
        // //获取redirect_uri
        // tmp_pValue = strstr(pValue,"redirect_uri=");
        // if(tmp_pValue == NULL)
        //     break;
        // if (!(pDecodeEtaxUrl = evhttp_decode_uri(tmp_pValue + strlen("redirect_uri="))))
        //     break;
        // strcpy(ctx->szBuf, pDecodeEtaxUrl);
        
        // printf("    ctx->szClientId = [ %s ]                pDecodeEtaxUrl = [ %s ]\r\n",ctx->szClientId,pDecodeEtaxUrl);
        printf("##########################################      deal with    【client_id  and  redirect_uri】  END  ########################################## \r\n");

        strcpy(ctx->szClientId,"k8n367k7fn884nedb9bfnnff8729a4nf");
        //strcpy(ctx->szClientId,ctx->szClientId");
        char pDecodeEtaxUrl_z[100];
        strcpy(pDecodeEtaxUrl_z,"https://etax.shaanxi.chinatax.gov.cn/kxsfrz-cjpt-web/tpass/tpassLogin.do");
        
        //构建json
        sprintf(szRepBuf,
                "{\"client_id\":\"%s\",\"redirect_uri\":\"%s\",\"uuid\":\"%s\",\"str_client_auth\":"
                "\"%s\",\"post_type\":\"0\",\"user_type\":\"1\",\"login_type\":\"0\"}",
                ctx->szClientId, pDecodeEtaxUrl_z, tem_uuid, tem_auth);

        /////////////////////////////////////////////////////  closeDevice  /////////////////////////////////////////////////////
        memset(szRepBuf_z,0,sizeof(szRepBuf_z));
        memset(tmp,0,sizeof(tmp));
        sprintf(tmp,"/api/v1/CloseCert");  //POST 请求的地址

        sprintf(szRepBuf_z, "{\"reqid\":\"%s\"}",tem_open); 
        printf("     【4】 Close  param     szRepBuf_z  = [%s]      \r\n",szRepBuf_z);
        result = socket_http_netPost_post(soft_cert_ip, soft_cert_port, tmp, "application/json", szRepBuf_z, 5, &returndata, &returndata_len);
        if (result < 0 || returndata_len <= 0)
        {
            if (returndata != NULL)
            {
                free(returndata);
                returndata = NULL;
            }
            nRet = -1;
        }
        printf("###################################  CloseCert returndata =  %s \r\n",returndata);

        memset(tem_data,0,sizeof(tem_data));
        tem_p = strstr(returndata,"Code\":\"");
        if(tem_p == NULL)
            break;
        tem_p += 7;
        stpncpy(tem_data, tem_p, 3);
        if (0 == strcmp(tem_data,"200"))
        {
            printf("           【请求成功！】         \r\n");
        }
        else
        {
            printf("请求失败：%s\r\n",tem_data);
            if(returndata)
                free(returndata);
		    returndata = NULL;
            break;
        }    
        
        free(returndata);
        returndata = NULL;
        
        nRet = RET_SUCCESS;
    } while (false);
    if (root)
        cJSON_Delete(root);
    if (pDecodeEtaxUrl)
        free(pDecodeEtaxUrl);
    return nRet;
}


int TPassLoginAuthDecodeStep3RepForStep4_soft_cert_21_guangxi(EHHTTP hi, struct evhttp_uri *uriTpassFull,char *szRandomHex16Bytes, char *szRepBuf, int nRepBufSize,char *unquid,char *soft_cert_ip,int soft_cert_port,char *tem_open,char *tem_hello)
{
    int nRet = ERR_GENERIC, nChildRet = 0;
    cJSON *root = NULL;
    CtxBridge ctx = (CtxBridge)hi->ctxApp;
    char szBuf[5120] = "", szUuid[BS_LITTLE] = "", *pValue = NULL,*tmp_pValue = NULL, *pDecodeEtaxUrl = NULL;
    char tem_uuid[100];
    int result;
    char szRepBuf_z[9*1024];
    char tmp[100] = {0};
    char content_type[100] = {0};
    char *returndata = NULL;
    int returndata_len;
    do {
        if (GetDataGramValueFromTPassReponse(szRepBuf, "", szRepBuf, nRepBufSize))
            break;
        nChildRet = Str2Byte((uint8 *)szRepBuf, szRepBuf, strlen(szRepBuf));
        if (!(nChildRet = EtaxSM4Algorithm(false, 1, (uint8 *)szRepBuf, nChildRet,
                                       (uint8 *)szRandomHex16Bytes, (uint8 *)szRepBuf)))
            break;
        //解出 新的uuid，serverHelloResult
        
        if (!(root = cJSON_Parse(szRepBuf)))
        {
            break;
        }
        if (!(pValue = cJSON_GetObjectItem(root, "uuid")->valuestring))
            break;
        strcpy(szUuid, pValue);  //新的业务uuid
        strcpy(tem_uuid,szUuid);

        if (!(pValue = cJSON_GetObjectItem(root, "serverHelloResult")->valuestring))
            break;
        strcpy(szRepBuf, pValue);
        //生成认证代码
        nChildRet = nRepBufSize;
        memset(szBuf, 0, sizeof(szBuf));

        /////////////////////////////////////////////////////  ClientAuth  /////////////////////////////////////////////////////
        printf("#########################################      ClientAuth         ############################################    \r\n");

        memset(szRepBuf_z,0,sizeof(szRepBuf_z));
        sprintf(tmp, "/api/v1/ClientAuth");  //POST 请求的地址
        sprintf(content_type, "application/json");
        sprintf(szRepBuf_z, "{\"reqid\":\"%s\",\"ServerHello\":\"%s\"}",tem_open,szRepBuf);
        printf("     【3】 AUTH  param     szRepBuf_z =  【%s】       \r\n",szRepBuf_z);
        result = socket_http_netPost_post(soft_cert_ip, soft_cert_port, tmp, content_type, szRepBuf_z, 5, &returndata, &returndata_len);
        if (result < 0 || returndata_len <= 0)
        {
            if (returndata != NULL)
            {
                free(returndata);
                returndata = NULL;
            }
            nRet = -1;
        }
        printf("#######################################  ClientAuth returndata =  %s \r\n",returndata);

        char *tem_p = NULL;
        char *tem_p_2 = NULL;
        char *pValue = NULL;
        char tem_data[2048] = {0};
        //获取Code
        memset(tem_data,0,sizeof(tem_data));

        cJSON *root3 = NULL;                     
        if (!(root3 = cJSON_Parse((const char *)returndata))) {          //步骤1 json初始化
            break;
        }
        if (GetAvailImportDataKp(root3, "Code", tem_data, sizeof(tem_data), true) < 0) {       //步骤2  导入code
            break;
        }

        if (0 == strcmp(tem_data,"200"))
        {
            printf("请求成功！\r\n");
        }
        else
        {
            printf("请求失败：%s\r\n",tem_data);
            if(returndata)
                free(returndata);
		    returndata = NULL;
            if(root3)
                cJSON_Delete(root3);
            break;
        }

        cJSON *object_data3 = cJSON_GetObjectItem(root3, "Data");
        if (object_data3 == NULL)
        {
            printf("请求Data失败\r\n");
            if(returndata)
                free(returndata);
		    returndata = NULL;
            if(root3)
                cJSON_Delete(root3);
            break;
        }     
        printf("ClientAuth Data:%s\r\n",object_data3->valuestring);

        //cJSON *item2 = NULL;  
        // item = cJSON_GetObjectItem(object_data3, "code");
        // printf("area_code:%s\r\n",item2->valuestring);
        
        //item = cJSON_GetObjectItem(object_data3, "reqid");
        //printf("Data:%s\r\n",object_data3->valuestring);        
        
        char tem_auth[2048] = {0};
        strcpy(tem_auth,object_data3->valuestring);
        printf("   AUTH  END  END      tem_auth = [ %s ]               \r\n",tem_auth);

        printf("##########################################      deal with    【client_id  and  redirect_uri】 START   ########################################## \r\n");
        // if(NULL == uriTpassFull)
        // {
        //     break;
        // }
        // if (!(pValue = (char *)evhttp_uri_get_query(uriTpassFull)) && hi->user_args == NULL)
        //     break;
        // if(pValue == NULL && hi->user_args != NULL)
        // {
        //     pValue = hi->user_args;
        // }
        
        // //获取client_id
        // if (!(tmp_pValue = strstr(pValue, "&client_id=")))
        // {
        //     break;
        // }
        // tmp_pValue += strlen("&client_id=");
        // if (strlen(tmp_pValue) < 32)
        //     break;
        // stpncpy(ctx->szClientId, tmp_pValue, 32);
        // //获取redirect_uri
        // tmp_pValue = strstr(pValue,"redirect_uri=");
        // if(tmp_pValue == NULL)
        //     break;
        // if (!(pDecodeEtaxUrl = evhttp_decode_uri(tmp_pValue + strlen("redirect_uri="))))
        //     break;
        // strcpy(ctx->szBuf, pDecodeEtaxUrl);
        
        // printf("    ctx->szClientId = [ %s ]                pDecodeEtaxUrl = [ %s ]\r\n",ctx->szClientId,pDecodeEtaxUrl);
        printf("##########################################      deal with    【client_id  and  redirect_uri】  END  ########################################## \r\n");

        // strcpy(ctx->szClientId,"k8n367k7fn884nedb9bfnnff8729a4nf");
        // //strcpy(ctx->szClientId,ctx->szClientId");
        // char pDecodeEtaxUrl_z[100];
        // strcpy(pDecodeEtaxUrl_z,"https://etax.shaanxi.chinatax.gov.cn/kxsfrz-cjpt-web/tpass/tpassLogin.do");
        
        strcpy(ctx->szClientId,"r98rerfrr235rBr3bb3552f2arfrarB8");
        //strcpy(ctx->szClientId,ctx->szClientId");
        char pDecodeEtaxUrl_z[100];
        strcpy(pDecodeEtaxUrl_z,"https://etax.guangxi.chinatax.gov.cn:9723/web/dzswj/ythclient/mh.html");    


        //构建json
        sprintf(szRepBuf,
                "{\"client_id\":\"%s\",\"redirect_uri\":\"%s\",\"uuid\":\"%s\",\"str_client_auth\":"
                "\"%s\",\"post_type\":\"0\",\"user_type\":\"1\",\"login_type\":\"0\"}",
                ctx->szClientId, pDecodeEtaxUrl_z, tem_uuid, tem_auth);

        /////////////////////////////////////////////////////  closeDevice  /////////////////////////////////////////////////////
        memset(szRepBuf_z,0,sizeof(szRepBuf_z));
        memset(tmp,0,sizeof(tmp));
        sprintf(tmp,"/api/v1/CloseCert");  //POST 请求的地址

        sprintf(szRepBuf_z, "{\"reqid\":\"%s\"}",tem_open); 
        printf("     【4】 Close  param     szRepBuf_z  = [%s]      \r\n",szRepBuf_z);
        result = socket_http_netPost_post(soft_cert_ip, soft_cert_port, tmp, "application/json", szRepBuf_z, 5, &returndata, &returndata_len);
        if (result < 0 || returndata_len <= 0)
        {
            if (returndata != NULL)
            {
                free(returndata);
                returndata = NULL;
            }
            nRet = -1;
        }
        printf("###################################  CloseCert returndata =  %s \r\n",returndata);

        memset(tem_data,0,sizeof(tem_data));
        tem_p = strstr(returndata,"Code\":\"");
        if(tem_p == NULL)
            break;
        tem_p += 7;
        stpncpy(tem_data, tem_p, 3);
        if (0 == strcmp(tem_data,"200"))
        {
            printf("           【请求成功！】         \r\n");
        }
        else
        {
            printf("请求失败：%s\r\n",tem_data);
            if(returndata)
                free(returndata);
		    returndata = NULL;
            break;
        }    
        
        free(returndata);
        returndata = NULL;
        
        nRet = RET_SUCCESS;
    } while (false);
    if (root)
        cJSON_Delete(root);
    if (pDecodeEtaxUrl)
        free(pDecodeEtaxUrl);
    return nRet;
}

int TPassLoginAuthDecodeStep3RepForStep4_soft_cert_chongqing(EHHTTP hi, struct evhttp_uri *uriTpassFull,char *szRandomHex16Bytes, char *szRepBuf, int nRepBufSize,char *unquid,char *soft_cert_ip,int soft_cert_port,char *tem_open,char *tem_hello)
{
    int nRet = ERR_GENERIC, nChildRet = 0;
    cJSON *root = NULL;
    CtxBridge ctx = (CtxBridge)hi->ctxApp;
    char szBuf[5120] = "", szUuid[BS_LITTLE] = "", *pValue = NULL,*tmp_pValue = NULL, *pDecodeEtaxUrl = NULL;
    char tem_uuid[100];
    int result;
    char szRepBuf_z[9*1024];
    char tmp[100] = {0};
    char content_type[100] = {0};
    char *returndata = NULL;
    int returndata_len;
    do {
        if (GetDataGramValueFromTPassReponse(szRepBuf, "", szRepBuf, nRepBufSize))
            break;
        nChildRet = Str2Byte((uint8 *)szRepBuf, szRepBuf, strlen(szRepBuf));
        if (!(nChildRet = EtaxSM4Algorithm(false, 1, (uint8 *)szRepBuf, nChildRet,
                                       (uint8 *)szRandomHex16Bytes, (uint8 *)szRepBuf)))
            break;
        //解出 新的uuid，serverHelloResult
        
        if (!(root = cJSON_Parse(szRepBuf)))
        {
            break;
        }
        if (!(pValue = cJSON_GetObjectItem(root, "uuid")->valuestring))
            break;
        strcpy(szUuid, pValue);  //新的业务uuid
        strcpy(tem_uuid,szUuid);

        if (!(pValue = cJSON_GetObjectItem(root, "serverHelloResult")->valuestring))
            break;
        strcpy(szRepBuf, pValue);
        //生成认证代码
        nChildRet = nRepBufSize;
        memset(szBuf, 0, sizeof(szBuf));

        printf("#########################################      ClientAuth         ############################################    \r\n");

        memset(szRepBuf_z,0,sizeof(szRepBuf_z));
        sprintf(tmp, "/api/v1/ClientAuth");  //POST 请求的地址
        sprintf(content_type, "application/json");
        sprintf(szRepBuf_z, "{\"reqid\":\"%s\",\"ServerHello\":\"%s\"}",tem_open,szRepBuf); 
        
        printf("        szRepBuf_z =  【%s】       \r\n",szRepBuf_z);

        result = socket_http_netPost_post(soft_cert_ip, soft_cert_port, tmp, content_type, szRepBuf_z, 5, &returndata, &returndata_len);
        if (result < 0 || returndata_len <= 0)
        {
            if (returndata != NULL)
            {
                free(returndata);
                returndata = NULL;
            }
            nRet = -1;
        }
        //printf("  ClientAuth returndata =  %s \r\n",returndata);

        char *tem_p = NULL;
        char *tem_p_2 = NULL;
        char *pValue = NULL;
        char tem_data[2048] = {0};

        memset(tem_data,0,sizeof(tem_data));

        cJSON *root3 = NULL;                     
        if (!(root3 = cJSON_Parse((const char *)returndata))) {          //步骤1 json初始化
            break;
        }
        if (GetAvailImportDataKp(root3, "Code", tem_data, sizeof(tem_data), true) < 0) {       //步骤2  导入code
            break;
        }

        if (0 == strcmp(tem_data,"200"))
        {
            printf("请求成功！\r\n");
        }
        else
        {
            printf("请求失败：%s\r\n",tem_data);
            if(returndata)
                free(returndata);
		    returndata = NULL;
            if(root3)
                cJSON_Delete(root3);
            break;
        }

        cJSON *object_data3 = cJSON_GetObjectItem(root3, "Data");
        if (object_data3 == NULL)
        {
            printf("请求Data失败\r\n");
            if(returndata)
                free(returndata);
		    returndata = NULL;
            if(root3)
                cJSON_Delete(root3);
            break;
        }     
        printf("ClientAuth Data:%s\r\n",object_data3->valuestring);      
        
        char tem_auth[2048] = {0};
        strcpy(tem_auth,object_data3->valuestring);
        printf("   AUTH  END  END      tem_auth = [ %s ]               \r\n",tem_auth);


        strcpy(ctx->szClientId,"CfcG3KCCGeG44e7faC374Cb5bK3GC3d4");
        char pDecodeEtaxUrl_z[100];
        strcpy(pDecodeEtaxUrl_z,"https://etax.chongqing.chinatax.gov.cn/sword?ctrl=KxLoginCtrl_initNsrxx");
        
        //构建json
        sprintf(szRepBuf,
                "{\"client_id\":\"%s\",\"redirect_uri\":\"%s\",\"uuid\":\"%s\",\"str_client_auth\":"
                "\"%s\",\"post_type\":\"0\",\"user_type\":\"1\",\"login_type\":\"0\"}",
                ctx->szClientId, pDecodeEtaxUrl_z, tem_uuid, tem_auth);

        /////////////////////////////////////////////////////  closeDevice  /////////////////////////////////////////////////////

        memset(szRepBuf_z,0,sizeof(szRepBuf_z));
        memset(tmp,0,sizeof(tmp));
        sprintf(tmp,"/api/v1/CloseCert");  //POST 请求的地址
        sprintf(szRepBuf_z, "{\"reqid\":\"%s\"}",tem_open); 
        printf("         szRepBuf_z  = [%s]      \r\n",szRepBuf_z);
        result = socket_http_netPost_post(soft_cert_ip, soft_cert_port, tmp, "application/json", szRepBuf_z, 5, &returndata, &returndata_len);
        if (result < 0 || returndata_len <= 0)
        {
            if (returndata != NULL)
            {
                free(returndata);
                returndata = NULL;
            }
            nRet = -1;
        }

        memset(tem_data,0,sizeof(tem_data));
        tem_p = strstr(returndata,"Code\":\"");
        if(tem_p == NULL)
            break;
        tem_p += 7;
        stpncpy(tem_data, tem_p, 3);
        if (0 == strcmp(tem_data,"200"))
        {
            printf("           【请求成功！】         \r\n");
        }
        else
        {
            printf("请求失败：%s\r\n",tem_data);
            if(returndata)
                free(returndata);
		    returndata = NULL;
            break;
        }    
        free(returndata);
        returndata = NULL;

        nRet = RET_SUCCESS;
    } while (false);
    if (root)
        cJSON_Delete(root);
    if (pDecodeEtaxUrl)
        free(pDecodeEtaxUrl);
    return nRet;
}

int TPassLoginAuthDecodeStep3RepForStep4_soft_cert_shanxi(EHHTTP hi, struct evhttp_uri *uriTpassFull,char *szRandomHex16Bytes, char *szRepBuf, int nRepBufSize,char *unquid,char *soft_cert_ip,int soft_cert_port,char *tem_open,char *tem_hello)
{
    int nRet = ERR_GENERIC, nChildRet = 0;
    cJSON *root = NULL;
    CtxBridge ctx = (CtxBridge)hi->ctxApp;
    char szBuf[5120] = "", szUuid[BS_LITTLE] = "", *pValue = NULL,*tmp_pValue = NULL, *pDecodeEtaxUrl = NULL;
    char tem_uuid[100];
    int result;
    char szRepBuf_z[9*1024];
    char tmp[100] = {0};
    char content_type[100] = {0};
    char *returndata = NULL;
    int returndata_len;
    do {
        if (GetDataGramValueFromTPassReponse(szRepBuf, "", szRepBuf, nRepBufSize))
            break;
        nChildRet = Str2Byte((uint8 *)szRepBuf, szRepBuf, strlen(szRepBuf));
        if (!(nChildRet = EtaxSM4Algorithm(false, 1, (uint8 *)szRepBuf, nChildRet,
                                       (uint8 *)szRandomHex16Bytes, (uint8 *)szRepBuf)))
            break;
        //解出 新的uuid，serverHelloResult
        
        if (!(root = cJSON_Parse(szRepBuf)))
        {
            break;
        }
        if (!(pValue = cJSON_GetObjectItem(root, "uuid")->valuestring))
            break;
        strcpy(szUuid, pValue);  //新的业务uuid
        strcpy(tem_uuid,szUuid);

        if (!(pValue = cJSON_GetObjectItem(root, "serverHelloResult")->valuestring))
            break;
        strcpy(szRepBuf, pValue);
        //生成认证代码
        nChildRet = nRepBufSize;
        memset(szBuf, 0, sizeof(szBuf));


        /////////////////////////////////////////////////////  ClientAuth  /////////////////////////////////////////////////////

        printf("#########################################      ClientAuth         ############################################    \r\n");

        memset(szRepBuf_z,0,sizeof(szRepBuf_z));
        sprintf(tmp, "/api/v1/ClientAuth");  //POST 请求的地址
        sprintf(content_type, "application/json");
        sprintf(szRepBuf_z, "{\"reqid\":\"%s\",\"ServerHello\":\"%s\"}",tem_open,szRepBuf); 
        printf("        szRepBuf_z =  【%s】       \r\n",szRepBuf_z);
        result = socket_http_netPost_post(soft_cert_ip, soft_cert_port, tmp, content_type, szRepBuf_z, 5, &returndata, &returndata_len);
        printf("    zwl returndata =  %s        \r\n",returndata);
        if (result < 0 || returndata_len <= 0)
        {
            if (returndata != NULL)
            {
                free(returndata);
                returndata = NULL;
            }
            nRet = -1;
        }

        char *tem_p = NULL;
        char *tem_p_2 = NULL;
        char *pValue = NULL;
        char tem_data[2048] = {0};

        memset(tem_data,0,sizeof(tem_data));

        cJSON *root3 = NULL;                     
        if (!(root3 = cJSON_Parse((const char *)returndata))) {          //步骤1 json初始化
            break;
        }
        if (GetAvailImportDataKp(root3, "Code", tem_data, sizeof(tem_data), true) < 0) {       //步骤2  导入code
            break;
        }

        if (0 == strcmp(tem_data,"200"))
        {
            printf("请求成功！\r\n");
        }
        else
        {
            printf("请求失败：%s\r\n",tem_data);
            if(returndata)
                free(returndata);
		    returndata = NULL;
            if(root3)
                cJSON_Delete(root3);
            break;
        }

        cJSON *object_data3 = cJSON_GetObjectItem(root3, "Data");
        if (object_data3 == NULL)
        {
            printf("请求Data失败\r\n");
            if(returndata)
                free(returndata);
		    returndata = NULL;
            if(root3)
                cJSON_Delete(root3);
            break;
        }     
        printf("ClientAuth Data:%s\r\n",object_data3->valuestring);

        //cJSON *item2 = NULL;  
        // item = cJSON_GetObjectItem(object_data3, "code");
        // printf("area_code:%s\r\n",item2->valuestring);
        
        //item = cJSON_GetObjectItem(object_data3, "reqid");
        //printf("Data:%s\r\n",object_data3->valuestring);        
        
        char tem_auth[2048] = {0};
        strcpy(tem_auth,object_data3->valuestring);
        printf("   AUTH  END  END      tem_auth = [ %s ]               \r\n",tem_auth);


        strcpy(ctx->szClientId,"d74f6Xacf6434c36acbepe4e6fpbcf8f");
        char pDecodeEtaxUrl_z[100];
        strcpy(pDecodeEtaxUrl_z,"https://etax.shanxi.chinatax.gov.cn/tpass/sso/loginSubmit");
        
        //构建json
        sprintf(szRepBuf,
                "{\"client_id\":\"%s\",\"redirect_uri\":\"%s\",\"uuid\":\"%s\",\"str_client_auth\":"
                "\"%s\",\"post_type\":\"0\",\"user_type\":\"1\",\"login_type\":\"0\"}",
                ctx->szClientId, pDecodeEtaxUrl_z, tem_uuid, tem_auth);

        /////////////////////////////////////////////////////  closeDevice  /////////////////////////////////////////////////////

        memset(szRepBuf_z,0,sizeof(szRepBuf_z));
        memset(tmp,0,sizeof(tmp));
        sprintf(tmp,"/api/v1/CloseCert");  //POST 请求的地址

        sprintf(szRepBuf_z, "{\"reqid\":\"%s\"}",tem_open); 
        printf("         szRepBuf_z  = [%s]      \r\n",szRepBuf_z);
        result = socket_http_netPost_post(soft_cert_ip, soft_cert_port, tmp, "application/json", szRepBuf_z, 5, &returndata, &returndata_len);
        if (result < 0 || returndata_len <= 0)
        {
            if (returndata != NULL)
            {
                free(returndata);
                returndata = NULL;
            }
            nRet = -1;
        }

        memset(tem_data,0,sizeof(tem_data));
        tem_p = strstr(returndata,"Code\":\"");
        if(tem_p == NULL)
            break;
        tem_p += 7;
        stpncpy(tem_data, tem_p, 3);

        if (0 == strcmp(tem_data,"200"))
        {
            printf("           【请求成功！】         \r\n");
        }
        else
        {
            printf("请求失败：%s\r\n",tem_data);
            if(returndata)
                free(returndata);
		    returndata = NULL;
            break;
        }    

        free(returndata);
        returndata = NULL;
        
        nRet = RET_SUCCESS;
    } while (false);
    if (root)
        cJSON_Delete(root);
    if (pDecodeEtaxUrl)
        free(pDecodeEtaxUrl);
    return nRet;
}

int TPassLoginAuthDecodeStep3RepForStep4_soft_cert_guizhou(EHHTTP hi, struct evhttp_uri *uriTpassFull,char *szRandomHex16Bytes, char *szRepBuf, int nRepBufSize,char *unquid,char *soft_cert_ip,int soft_cert_port,char *tem_open,char *tem_hello)
{
    int nRet = ERR_GENERIC, nChildRet = 0;
    cJSON *root = NULL;
    CtxBridge ctx = (CtxBridge)hi->ctxApp;
    char szBuf[5120] = "", szUuid[BS_LITTLE] = "", *pValue = NULL,*tmp_pValue = NULL, *pDecodeEtaxUrl = NULL;
    char tem_uuid[100];
    int result;
    char szRepBuf_z[9*1024];
    char tmp[100] = {0};
    char content_type[100] = {0};
    char *returndata = NULL;
    int returndata_len;
    do {
        if (GetDataGramValueFromTPassReponse(szRepBuf, "", szRepBuf, nRepBufSize))
            break;
        nChildRet = Str2Byte((uint8 *)szRepBuf, szRepBuf, strlen(szRepBuf));
        if (!(nChildRet = EtaxSM4Algorithm(false, 1, (uint8 *)szRepBuf, nChildRet,
                                       (uint8 *)szRandomHex16Bytes, (uint8 *)szRepBuf)))
            break;
        //解出 新的uuid，serverHelloResult
        
        if (!(root = cJSON_Parse(szRepBuf)))
        {
            break;
        }
        if (!(pValue = cJSON_GetObjectItem(root, "uuid")->valuestring))
            break;
        strcpy(szUuid, pValue);  //新的业务uuid
        strcpy(tem_uuid,szUuid);

        if (!(pValue = cJSON_GetObjectItem(root, "serverHelloResult")->valuestring))
            break;
        strcpy(szRepBuf, pValue);
        //生成认证代码
        nChildRet = nRepBufSize;
        memset(szBuf, 0, sizeof(szBuf));


        /////////////////////////////////////////////////////  ClientAuth  /////////////////////////////////////////////////////

        printf("#########################################      ClientAuth         ############################################    \r\n");

        memset(szRepBuf_z,0,sizeof(szRepBuf_z));
        sprintf(tmp, "/api/v1/ClientAuth");  //POST 请求的地址
        sprintf(content_type, "application/json");
        sprintf(szRepBuf_z, "{\"reqid\":\"%s\",\"ServerHello\":\"%s\"}",tem_open,szRepBuf); 
        printf("        szRepBuf_z =  【%s】       \r\n",szRepBuf_z);
        result = socket_http_netPost_post(soft_cert_ip, soft_cert_port, tmp, content_type, szRepBuf_z, 5, &returndata, &returndata_len);
        printf("    zwl returndata =  %s        \r\n",returndata);
        if (result < 0 || returndata_len <= 0)
        {
            if (returndata != NULL)
            {
                free(returndata);
                returndata = NULL;
            }
            nRet = -1;
        }

        char *tem_p = NULL;
        char *tem_p_2 = NULL;
        char *pValue = NULL;
        char tem_data[2048] = {0};

        memset(tem_data,0,sizeof(tem_data));

        cJSON *root3 = NULL;                     
        if (!(root3 = cJSON_Parse((const char *)returndata))) {          //步骤1 json初始化
            break;
        }
        if (GetAvailImportDataKp(root3, "Code", tem_data, sizeof(tem_data), true) < 0) {       //步骤2  导入code
            break;
        }

        if (0 == strcmp(tem_data,"200"))
        {
            printf("请求成功！\r\n");
        }
        else
        {
            printf("请求失败：%s\r\n",tem_data);
            if(returndata)
                free(returndata);
		    returndata = NULL;
            if(root3)
                cJSON_Delete(root3);
            break;
        }

        cJSON *object_data3 = cJSON_GetObjectItem(root3, "Data");
        if (object_data3 == NULL)
        {
            printf("请求Data失败\r\n");
            if(returndata)
                free(returndata);
		    returndata = NULL;
            if(root3)
                cJSON_Delete(root3);
            break;
        }     
        printf("ClientAuth Data:%s\r\n",object_data3->valuestring);

        //cJSON *item2 = NULL;  
        // item = cJSON_GetObjectItem(object_data3, "code");
        // printf("area_code:%s\r\n",item2->valuestring);
        
        //item = cJSON_GetObjectItem(object_data3, "reqid");
        //printf("Data:%s\r\n",object_data3->valuestring);        
        
        char tem_auth[2048] = {0};
        strcpy(tem_auth,object_data3->valuestring);
        printf("   AUTH  END  END      tem_auth = [ %s ]               \r\n",tem_auth);


        strcpy(ctx->szClientId,"f3dHd4H42d994f3H8b56a3577bHa9fb4");
        char pDecodeEtaxUrl_z[100];
        strcpy(pDecodeEtaxUrl_z,"https://etax.guizhou.chinatax.gov.cn/kxsfrz-cjpt-web/tpass/tpassLogin.do");
        
        //构建json
        sprintf(szRepBuf,
                "{\"client_id\":\"%s\",\"redirect_uri\":\"%s\",\"uuid\":\"%s\",\"str_client_auth\":"
                "\"%s\",\"post_type\":\"0\",\"user_type\":\"1\",\"login_type\":\"0\"}",
                ctx->szClientId, pDecodeEtaxUrl_z, tem_uuid, tem_auth);

        /////////////////////////////////////////////////////  closeDevice  /////////////////////////////////////////////////////

        memset(szRepBuf_z,0,sizeof(szRepBuf_z));
        memset(tmp,0,sizeof(tmp));
        sprintf(tmp,"/api/v1/CloseCert");  //POST 请求的地址

        sprintf(szRepBuf_z, "{\"reqid\":\"%s\"}",tem_open); 
        printf("         szRepBuf_z  = [%s]      \r\n",szRepBuf_z);
        result = socket_http_netPost_post(soft_cert_ip, soft_cert_port, tmp, "application/json", szRepBuf_z, 5, &returndata, &returndata_len);
        if (result < 0 || returndata_len <= 0)
        {
            if (returndata != NULL)
            {
                free(returndata);
                returndata = NULL;
            }
            nRet = -1;
        }

        memset(tem_data,0,sizeof(tem_data));
        tem_p = strstr(returndata,"Code\":\"");
        if(tem_p == NULL)
            break;
        tem_p += 7;
        stpncpy(tem_data, tem_p, 3);

        if (0 == strcmp(tem_data,"200"))
        {
            printf("           【请求成功！】         \r\n");
        }
        else
        {
            printf("请求失败：%s\r\n",tem_data);
            if(returndata)
                free(returndata);
		    returndata = NULL;
            break;
        }    

        free(returndata);
        returndata = NULL;
        
        nRet = RET_SUCCESS;
    } while (false);
    if (root)
        cJSON_Delete(root);
    if (pDecodeEtaxUrl)
        free(pDecodeEtaxUrl);
    return nRet;
}

int TPassLoginAuthDecodeStep3RepForStep4_soft_cert(EHHTTP hi, struct evhttp_uri *uriTpassFull,char *szRandomHex16Bytes, char *szRepBuf, int nRepBufSize,char *unquid,char *soft_cert_ip,int soft_cert_port)
{
    int nRet = ERR_GENERIC, nChildRet = 0;
    cJSON *root = NULL;
    CtxBridge ctx = (CtxBridge)hi->ctxApp;
    char szBuf[5120] = "", szUuid[BS_LITTLE] = "", *pValue = NULL,*tmp_pValue = NULL, *pDecodeEtaxUrl = NULL;
    char tem_uuid[100];
    int result;
    char szRepBuf_z[9*1024];
    char tmp[100] = {0};
    char content_type[100] = {0};
    char *returndata = NULL;
    int returndata_len;
    do {
        if (GetDataGramValueFromTPassReponse(szRepBuf, "", szRepBuf, nRepBufSize))
            break;
        nChildRet = Str2Byte((uint8 *)szRepBuf, szRepBuf, strlen(szRepBuf));
        if (!(nChildRet = EtaxSM4Algorithm(false, 1, (uint8 *)szRepBuf, nChildRet,
                                       (uint8 *)szRandomHex16Bytes, (uint8 *)szRepBuf)))
            break;
        //解出 新的uuid，serverHelloResult
        
        if (!(root = cJSON_Parse(szRepBuf)))
        {
            break;
        }
        if (!(pValue = cJSON_GetObjectItem(root, "uuid")->valuestring))
            break;
        strcpy(szUuid, pValue);  //新的业务uuid
        strcpy(tem_uuid,szUuid);

        if (!(pValue = cJSON_GetObjectItem(root, "serverHelloResult")->valuestring))
            break;
        strcpy(szRepBuf, pValue);
        //生成认证代码
        nChildRet = nRepBufSize;
        memset(szBuf, 0, sizeof(szBuf));

        memset(szRepBuf_z,0,sizeof(szRepBuf_z));
        sprintf(tmp, "/cert/clientAuth?dwFlags=1&unqid=%s",unquid);  //POST 请求的地址 
        sprintf(content_type, "application/json");
        sprintf(szRepBuf_z, "{\"svrContainer\":\"%s\"}",szRepBuf); //传的参数
        result = socket_http_netPost_post(soft_cert_ip, soft_cert_port, tmp, content_type, szRepBuf_z, 5, &returndata, &returndata_len);
        /*if (result < 0)
        {
            if (returndata != NULL)
            {
                free(returndata);
                returndata = NULL;
            }
            return -1;
        }*/
        if (result < 0 || returndata_len <= 0)
        {
            if (returndata != NULL)
            {
                free(returndata);
                returndata = NULL;
            }
            return -1;
        }

        if(NULL == uriTpassFull)
        {
            break;
        }
        if (!(pValue = (char *)evhttp_uri_get_query(uriTpassFull)) && hi->user_args == NULL)
            break;
        if(pValue == NULL && hi->user_args != NULL)
        {
            pValue = hi->user_args;
        }

        if (!(tmp_pValue = strstr(pValue, "&client_id=")))
        {
            break;
        }
        tmp_pValue += strlen("&client_id=");
        if (strlen(tmp_pValue) < 32)
            break;
        stpncpy(ctx->szClientId, tmp_pValue, 32);
        tmp_pValue = strstr(pValue,"redirect_uri=");
        if(tmp_pValue == NULL)
            break;
        if (!(pDecodeEtaxUrl = evhttp_decode_uri(tmp_pValue + strlen("redirect_uri="))))
            break;
        strcpy(ctx->szBuf, pDecodeEtaxUrl);
        
        /*strcpy(ctx->szClientId,"s44fftt3bc634tcab4teasbaasba7ft4");
        //strcpy(ctx->szClientId,ctx->szClientId");
        char pDecodeEtaxUrl_z[100];
        strcpy(pDecodeEtaxUrl_z,"https://etax.jiangsu.chinatax.gov.cn/sso/kxLogin/authorize");*/
        
        //构建json

        sprintf(szRepBuf,
                "{\"client_id\":\"%s\",\"redirect_uri\":\"%s\",\"uuid\":\"%s\",\"str_client_auth\":"
                "\"%s\",\"post_type\":\"0\",\"user_type\":\"1\",\"login_type\":\"0\"}",
                ctx->szClientId, pDecodeEtaxUrl, tem_uuid, returndata);

        memset(szRepBuf_z,0,sizeof(szRepBuf_z));
        memset(tmp,0,sizeof(tmp));
        sprintf(tmp, "/cert/closeDevice?unqid=%s",unquid);  //GET 请求的地址 
        //printf("###############################################222######################################\r\n");
        result = socket_http_netPost_get(soft_cert_ip, soft_cert_port, tmp, content_type, szRepBuf_z, 5, &returndata, &returndata_len);
        /*if (result < 0)
        {
            if (returndata != NULL)
            {
                printf("释放http接收数据内存\n");
                free(returndata);
                returndata = NULL;
                printf("释放内存成功,指向空\n");
            }
        }*/
        if (result < 0 || returndata_len <= 0)
        {
            if (returndata != NULL)
            {
                free(returndata);
                returndata = NULL;
            }
            return -1;
        }
        free(returndata);
        returndata = NULL;
        
        nRet = RET_SUCCESS;
    } while (false);
    if (root)
        cJSON_Delete(root);
    if (pDecodeEtaxUrl)
        free(pDecodeEtaxUrl);
    return nRet;
}

//软证书
int TPassLoginAuthHelloAndAuth_soft_cert(EHHTTP hi, struct evhttp_uri *uriTpassFull,char *svr_container,char *soft_cert_ip,int soft_cert_port)
{
    int nRet = ERR_GENERIC, nChildRet = 0,result = -1,returndata_len = 0;
	char szRepBuf_z[1024];
	char tmp[100] = {0};
	char content_type[100] = {0};
	char *returndata = NULL,*unquid = NULL;
    struct evbuffer *buf = NULL;
    CtxBridge ctx = (CtxBridge)hi->ctxApp;
    //可变重复用buf，szRepBuf最大接收7k数据,szBuf最大接收3k+数据
    char szRepBuf[9216] = "", szBuf[5120] = "";
    do
    {
        printf("    请求的ip: %s \r\n",soft_cert_ip);
        printf("        请求的port: %d \r\n",soft_cert_port);


        if (!(buf = evbuffer_new()))
            break;
        sprintf(tmp, "/cert/openDevice");  //POST 请求的地址 
        sprintf(content_type, "application/json");
        sprintf(szRepBuf_z, "{\"svrContainer\":\"%s\"}",svr_container); 
        result = socket_http_netPost_post(soft_cert_ip, soft_cert_port, tmp, content_type, szRepBuf_z, 5, &returndata, &returndata_len);
        if (result < 0 || returndata_len <= 0)
        {
            if (returndata != NULL)
            {
                free(returndata);
                returndata = NULL;
            }
            return -1;
        }
        unquid = calloc(sizeof(char),returndata_len);
        if(NULL == unquid)
        {
            if (returndata != NULL)
            {
                free(returndata);
                returndata = NULL;
            }
            return -1;
        }
        strcpy(unquid,returndata);
        //printf("#####################################################################################\r\n");
        memset(szRepBuf_z,0,sizeof(szRepBuf_z));
        sprintf(tmp, "/cert/clientHello?dwFlags=1&unqid=%s",unquid);  //POST 请求的地址 
        sprintf(content_type, "application/json");

        result = socket_http_netPost_get(soft_cert_ip, soft_cert_port, tmp, content_type, szRepBuf_z, 5, &returndata, &returndata_len);
        /*if (result < 0)
        {
            if (returndata != NULL)
            {
                printf("释放http接收数据内存\n");
                free(returndata);
                returndata = NULL;
                printf("释放内存成功,指向空\n");
            }
            //return -1;
        }*/
        if (result < 0 || returndata_len <= 0)
        {
            if (returndata != NULL)
            {
                free(returndata);
                returndata = NULL;
            }
            return -1;
        }
        sprintf(szRepBuf, "{\"server_client_hello\":\"%s\"}", returndata);
        free(returndata);
		returndata = NULL;
        if (BuildTPassFormatHMacSHA256Packet(true, 2, szRepBuf, ctx->szRandom16BytesKey, buf))
            break;
        //需要在回调函数中使用hi->ctxApp.szTmpBuf中保存的uuid
        hi->cbHttpNewReq = CallbackTPassSetHttpHeaderUuid;
        if (EtaxHTTPPost(hi, "/sys-api/v1.0/auth/cert/serverHello", "application/json",
                     (char *)evbuffer_pullup(buf, -1), evbuffer_get_length(buf), szRepBuf,
                     sizeof(szRepBuf)) < 0) {
            nRet = -2;
            break;
        }
        if (DecodeTpassReponseJson(hi, szRepBuf, "")) {
            nRet = -3;
            break;
        }
        if (TPassLoginAuthDecodeStep3RepForStep4_soft_cert(hi, uriTpassFull, ctx->szRandom16BytesKey,szRepBuf, sizeof(szRepBuf),unquid,soft_cert_ip, soft_cert_port))
        {
            nRet = -4;
            break;
        }
        if (TpassLogicIo(hi, "/sys-api/v1.0/auth/cert/serverAuth", szRepBuf, sizeof(szRepBuf))) {
            nRet = -5;
            break;
        }
        evbuffer_drain(hi->bufHttpRep, -1);
        evbuffer_add_printf(hi->bufHttpRep, "%s", szRepBuf);
        nRet = RET_SUCCESS;
    } while (false);
    if (buf)
        evbuffer_free(buf);
    return nRet;
}

//软证书【重庆】
int TPassLoginAuthHelloAndAuth_soft_cert_chongqing(EHHTTP hi, struct evhttp_uri *uriTpassFull,char *svr_container,char *soft_cert_ip,int soft_cert_port)
{
    int nRet = ERR_GENERIC, nChildRet = 0,result = -1,returndata_len = 0;
	char szRepBuf_z[1024];
	char tmp[100] = {0};
	char content_type[100] = {0};
	char *returndata = NULL;
    struct evbuffer *buf = NULL;
    CtxBridge ctx = (CtxBridge)hi->ctxApp;

    cJSON *root1 = NULL, *objItem = NULL, *objItemChild = NULL;
    cJSON *objItemChild2 = NULL,*objItemToken  = NULL;


    //可变重复用buf，szRepBuf最大接收7k数据,szBuf最大接收3k+数据
    char szRepBuf[9216] = "", szBuf[5120] = "";
    do
    {
        printf("请求的ip: %s \r\n",soft_cert_ip);
        printf("请求的port: %d \r\n",soft_cert_port);

        /////////////////////////////////////////////////////  OpenCert  /////////////////////////////////////////////////////

        if (!(buf = evbuffer_new()))
            break;
        sprintf(tmp, "/api/v1/OpenCert");  
        sprintf(content_type, "application/json");
        sprintf(szRepBuf_z, "{\"cpy_num\":\"%s\"}",svr_container);
        printf("************************szRepBuf_z = [%s]\r\n",szRepBuf_z);
        result = socket_http_netPost_post(soft_cert_ip, soft_cert_port, tmp, content_type, szRepBuf_z, 5, &returndata, &returndata_len);
        printf("returndata_len=%d\r\n,returndata=%s\r\n",returndata_len,returndata);
        if (result < 0 || returndata_len <= 0)
        {
            if (returndata != NULL)
            {
                free(returndata);
                returndata = NULL;
            }
            nRet = -1;
        }
        
        char *tem_p = NULL;
        char *pValue = NULL;
        char tem_data[256] = {0};

        memset(tem_data,0,sizeof(tem_data));
        cJSON *root = NULL;                     
        if (!(root = cJSON_Parse((const char *)returndata))) {          //步骤1 json初始化
            break;
        }
       
        if (GetAvailImportDataKp(root, "Code", tem_data, sizeof(tem_data), true) < 0) {       //步骤2  导入code
            break;
        }

        if (0 == strcmp(tem_data,"200"))
        {
            printf("请求成功！\r\n");
        }
        else
        {
            printf("请求失败：%s\r\n",tem_data);
            if(returndata)
                free(returndata);
		    returndata = NULL;
            if(root)
                cJSON_Delete(root);
            break;
        }
        
        cJSON *object_data = cJSON_GetObjectItem(root, "Data");
        if (object_data == NULL)
        {
            printf("请求Data失败\r\n");
            if(returndata)
                free(returndata);
		    returndata = NULL;
            if(root)
                cJSON_Delete(root);
            break;
        }       

        cJSON *item = NULL; 
        // item = cJSON_GetObjectItem(object_data, "code");
        // printf("area_code:%s\r\n",item->valuestring);
        item = cJSON_GetObjectItem(object_data, "reqid");
        printf("reqid:%s\r\n",item->valuestring);        

        char tem_open[64] = {0};
        strcpy(tem_open,item->valuestring);

        sprintf(tmp, "/api/v1/ClientHello"); 
        sprintf(content_type, "application/json");

        sprintf(szRepBuf_z, "{\"reqid\":\"%s\"}",tem_open);

        printf("    szRepBuf_z = [ %s ]    \r\n",szRepBuf_z);
        result = socket_http_netPost_post(soft_cert_ip, soft_cert_port, tmp, content_type, szRepBuf_z, 5, &returndata, &returndata_len);
        if (result < 0)
        {
            if (returndata != NULL)
            {
                printf("释放http接收数据内存\n");
                free(returndata);
                returndata = NULL;
                printf("释放内存成功,指向空\n");
            }
            nRet = -1;
        }
        sprintf(szRepBuf, "{\"server_client_hello\":\"%s\"}", returndata);


        memset(tem_data,0,sizeof(tem_data));
        
        cJSON *root2 = NULL;                     
        if (!(root2 = cJSON_Parse((const char *)returndata))) {          //步骤1 json初始化
            break;
        }
        if (GetAvailImportDataKp(root2, "Code", tem_data, sizeof(tem_data), true) < 0) {       //步骤2  导入code
            break;
        }

        if (0 == strcmp(tem_data,"200"))
        {
            printf("请求成功！\r\n");
        }
        else
        {
            printf("请求失败：%s\r\n",tem_data);
            if(returndata)
                free(returndata);
		    returndata = NULL;
            if(root2)
                cJSON_Delete(root2);
            break;
        }

        cJSON *object_data2 = cJSON_GetObjectItem(root2, "Data");
        if (object_data2 == NULL)
        {
            printf("请求Data失败\r\n");
            if(returndata)
                free(returndata);
		    returndata = NULL;
            if(root2)
                cJSON_Delete(root2);
            break;
        }     
        printf("Data:%s\r\n",object_data2->valuestring); 

        char tem_hello[1024] = {0};
        strcpy(tem_hello,object_data2->valuestring);
        sprintf(szRepBuf, "{\"server_client_hello\":\"%s\"}", tem_hello);

        free(returndata);
		returndata = NULL;


        if (BuildTPassFormatHMacSHA256Packet(true, 2, szRepBuf, ctx->szRandom16BytesKey, buf))
            break;
        //需要在回调函数中使用hi->ctxApp.szTmpBuf中保存的uuid
        hi->cbHttpNewReq = CallbackTPassSetHttpHeaderUuid;
        if (EtaxHTTPPost(hi, "/sys-api/v1.0/auth/cert/serverHello", "application/json",
                     (char *)evbuffer_pullup(buf, -1), evbuffer_get_length(buf), szRepBuf,
                     sizeof(szRepBuf)) < 0) {
            nRet = -2;
            break;
        }
        if (DecodeTpassReponseJson(hi, szRepBuf, "")) {
            nRet = -3;
            break;
        }
        if (TPassLoginAuthDecodeStep3RepForStep4_soft_cert_chongqing(hi, uriTpassFull, ctx->szRandom16BytesKey,szRepBuf, sizeof(szRepBuf),"",soft_cert_ip, soft_cert_port,tem_open,tem_hello))
        {
            nRet = -4;
            break;
        }
        //printf("4444444444zRepBuf = [%s]\r\n",szRepBuf);
        if (TpassLogicIo(hi, "/sys-api/v1.0/auth/cert/serverAuth", szRepBuf, sizeof(szRepBuf))) {
            printf("                TpassLogicIo            TpassLogicIo            TpassLogicIo            \r\n");
            nRet = -5;
            break;
        }
        evbuffer_drain(hi->bufHttpRep, -1);
        evbuffer_add_printf(hi->bufHttpRep, "%s", szRepBuf);
        nRet = RET_SUCCESS;
    } while (false);
    if (buf)
        evbuffer_free(buf);
    return nRet;
}

//软证书【山西】
int TPassLoginAuthHelloAndAuth_soft_cert_shanxi(EHHTTP hi, struct evhttp_uri *uriTpassFull,char *svr_container,char *soft_cert_ip,int soft_cert_port,char *errinfo_uft8_21)
{
    int nRet = ERR_GENERIC, nChildRet = 0,result = -1,returndata_len = 0;
	char szRepBuf_z[1024];
	char tmp[100] = {0};
	char content_type[100] = {0};
	char *returndata = NULL;
    struct evbuffer *buf = NULL;
    CtxBridge ctx = (CtxBridge)hi->ctxApp;

    cJSON *root1 = NULL, *objItem = NULL, *objItemChild = NULL;
    cJSON *objItemChild2 = NULL,*objItemToken  = NULL;


    //可变重复用buf，szRepBuf最大接收7k数据,szBuf最大接收3k+数据
    printf("                           1                     \r\n");
    char szRepBuf[9216] = "", szBuf[5120] = "";
    do
    {
        printf("请求的ip: %s \r\n",soft_cert_ip);
        printf("请求的port: %d \r\n",soft_cert_port);

        /////////////////////////////////////////////////////  OpenCert  /////////////////////////////////////////////////////

        if (!(buf = evbuffer_new()))
            break;
        sprintf(tmp, "/api/v1/OpenCert"); 
        sprintf(content_type, "application/json");
        sprintf(szRepBuf_z, "{\"cpy_num\":\"%s\"}",svr_container); 
        printf("szRepBuf_z = [%s]\r\n",szRepBuf_z);
        result = socket_http_netPost_post(soft_cert_ip, soft_cert_port, tmp, content_type, szRepBuf_z, 5, &returndata, &returndata_len);
        //result = socket_http_netPost_post("192.168.0.247", 18002, tmp, content_type, szRepBuf_z, 5, &returndata, &returndata_len);  
        printf("returndata_len=%d\r\n,returndata=%s\r\n",returndata_len,returndata);             
        if (result < 0 || returndata_len <= 0)
        {
            if (returndata != NULL)
            {
                free(returndata);
                returndata = NULL;
            }
            nRet = -1;
        }

        char *tem_p = NULL;
        char *pValue = NULL;
        char tem_data[256] = {0};

        memset(tem_data,0,sizeof(tem_data));
        
        
        cJSON *root = NULL;                
        if (!(root = cJSON_Parse((const char *)returndata))) {          //步骤1 json初始化
            break;
        }
       
        if (GetAvailImportDataKp(root, "Code", tem_data, sizeof(tem_data), true) < 0) {       //步骤2  导入code
            break;
        }

        if (0 == strcmp(tem_data,"200"))
        {
            printf("请求成功！\r\n");
        }
        else
        {
            // printf("请求失败：%s\r\n",tem_data);
            // if(returndata)
            //     free(returndata);
		    // returndata = NULL;
            // if(root)
            //     cJSON_Delete(root);
            // break;
            cJSON *item_msg = NULL;
            char *v_str = NULL;
            item_msg = cJSON_GetObjectItem(root, "Msg");
            if (item_msg != NULL) {
                if (item_msg->type == cJSON_String) {	
                    v_str = item_msg->valuestring;	
                    printf("Msg = %s\n", v_str);
                }
            }
            sprintf(errinfo_uft8_21, "102%s",v_str);
            printf("请求失败：%s\r\n",errinfo_uft8_21);
            if(returndata)
                free(returndata);
		    returndata = NULL;
            if(root)
                cJSON_Delete(root);
            break;
        }

        cJSON *object_data = cJSON_GetObjectItem(root, "Data");
        if (object_data == NULL)
        {
            printf("请求Data失败\r\n");
            if(returndata)
                free(returndata);
		    returndata = NULL;
            if(root)
                cJSON_Delete(root);
            break;
        }      

        cJSON *item = NULL; 
        // item = cJSON_GetObjectItem(object_data, "code");
        // printf("area_code:%s\r\n",item->valuestring);
        item = cJSON_GetObjectItem(object_data, "reqid");
        printf("reqid:%s\r\n",item->valuestring);        

        char tem_open[64] = {0};
        strcpy(tem_open,item->valuestring);
        
      /////////////////////////////////////////////////////  ClientHello  /////////////////////////////////////////////////////
        
        sprintf(tmp, "/api/v1/ClientHello");  //POST 请求的地址 
        sprintf(content_type, "application/json");

        sprintf(szRepBuf_z, "{\"reqid\":\"%s\"}",tem_open);
        printf("    szRepBuf_z = [ %s ]    \r\n",szRepBuf_z);
        result = socket_http_netPost_post(soft_cert_ip, soft_cert_port, tmp, content_type, szRepBuf_z, 5, &returndata, &returndata_len);
        if (result < 0 || returndata_len <= 0)
        {
            if (returndata != NULL)
            {
                free(returndata);
                returndata = NULL;
            }
            nRet = -1;
        }
        sprintf(szRepBuf, "{\"server_client_hello\":\"%s\"}", returndata);

        memset(tem_data,0,sizeof(tem_data));

        cJSON *root2 = NULL;                     
        if (!(root2 = cJSON_Parse((const char *)returndata))) {          //步骤1 json初始化
            break;
        }
        if (GetAvailImportDataKp(root2, "Code", tem_data, sizeof(tem_data), true) < 0) {       //步骤2  导入code
            break;
        }

        if (0 == strcmp(tem_data,"200"))
        {
            printf("请求成功！\r\n");
        }
        else
        {
            printf("请求失败：%s\r\n",tem_data);
            if(returndata)
                free(returndata);
		    returndata = NULL;
            if(root2)
                cJSON_Delete(root2);
            break;
        }

        cJSON *object_data2 = cJSON_GetObjectItem(root2, "Data");
        if (object_data2 == NULL)
        {
            printf("请求Data失败\r\n");
            if(returndata)
                free(returndata);
		    returndata = NULL;
            if(root2)
                cJSON_Delete(root2);
            break;
        }     
        printf("Data:%s\r\n",object_data2->valuestring);

        //cJSON *item2 = NULL;  
        // item = cJSON_GetObjectItem(object_data2, "code");
        // printf("area_code:%s\r\n",item2->valuestring);
        
        //item = cJSON_GetObjectItem(object_data2, "reqid");
        //printf("Data:%s\r\n",object_data2->valuestring);        

        char tem_hello[1024] = {0};
        strcpy(tem_hello,object_data2->valuestring);    
        sprintf(szRepBuf, "{\"server_client_hello\":\"%s\"}", tem_hello);
        free(returndata);
		returndata = NULL;


        if (BuildTPassFormatHMacSHA256Packet(true, 2, szRepBuf, ctx->szRandom16BytesKey, buf))
            break;
        //需要在回调函数中使用hi->ctxApp.szTmpBuf中保存的uuid
        hi->cbHttpNewReq = CallbackTPassSetHttpHeaderUuid;
        if (EtaxHTTPPost(hi, "/sys-api/v1.0/auth/cert/serverHello", "application/json",
                     (char *)evbuffer_pullup(buf, -1), evbuffer_get_length(buf), szRepBuf,
                     sizeof(szRepBuf)) < 0) {
            nRet = -2;
            break;
        }
        if (DecodeTpassReponseJson(hi, szRepBuf, "")) {
            nRet = -3;
            break;
        }
        if (TPassLoginAuthDecodeStep3RepForStep4_soft_cert_shanxi(hi, uriTpassFull, ctx->szRandom16BytesKey,szRepBuf, sizeof(szRepBuf),"",soft_cert_ip, soft_cert_port,tem_open,tem_hello))
        {
            nRet = -4;
            break;
        }
        //printf("444444444 szRepBuf = [%s]\r\n",szRepBuf);
        if (TpassLogicIo(hi, "/sys-api/v1.0/auth/cert/serverAuth", szRepBuf, sizeof(szRepBuf))) {
            printf("                TpassLogicIo            TpassLogicIo            TpassLogicIo            \r\n");
            nRet = -5;
            break;
        }
        evbuffer_drain(hi->bufHttpRep, -1);
        evbuffer_add_printf(hi->bufHttpRep, "%s", szRepBuf);
        nRet = RET_SUCCESS;
    } while (false);
    if (buf)
        evbuffer_free(buf);
    return nRet;
}

//软证书【贵州】
int TPassLoginAuthHelloAndAuth_soft_cert_guizhou(EHHTTP hi, struct evhttp_uri *uriTpassFull,char *svr_container,char *soft_cert_ip,int soft_cert_port,char *errinfo_uft8_21)
{
    int nRet = ERR_GENERIC, nChildRet = 0,result = -1,returndata_len = 0;
	char szRepBuf_z[1024];
	char tmp[100] = {0};
	char content_type[100] = {0};
	char *returndata = NULL;
    struct evbuffer *buf = NULL;
    CtxBridge ctx = (CtxBridge)hi->ctxApp;

    cJSON *root1 = NULL, *objItem = NULL, *objItemChild = NULL;
    cJSON *objItemChild2 = NULL,*objItemToken  = NULL;


    //可变重复用buf，szRepBuf最大接收7k数据,szBuf最大接收3k+数据
    printf("                           1                     \r\n");
    char szRepBuf[9216] = "", szBuf[5120] = "";
    do
    {
        printf("请求的ip: %s \r\n",soft_cert_ip);
        printf("请求的port: %d \r\n",soft_cert_port);

        /////////////////////////////////////////////////////  OpenCert  /////////////////////////////////////////////////////

        if (!(buf = evbuffer_new()))
            break;
        sprintf(tmp, "/api/v1/OpenCert"); 
        sprintf(content_type, "application/json");
        sprintf(szRepBuf_z, "{\"cpy_num\":\"%s\"}",svr_container); 
        printf("szRepBuf_z = [%s]\r\n",szRepBuf_z);
        result = socket_http_netPost_post(soft_cert_ip, soft_cert_port, tmp, content_type, szRepBuf_z, 5, &returndata, &returndata_len);
        //result = socket_http_netPost_post("192.168.0.247", 18002, tmp, content_type, szRepBuf_z, 5, &returndata, &returndata_len);  
        printf("returndata_len=%d\r\n,returndata=%s\r\n",returndata_len,returndata);             
        if (result < 0 || returndata_len <= 0)
        {
            if (returndata != NULL)
            {
                free(returndata);
                returndata = NULL;
            }
            nRet = -1;
        }

        char *tem_p = NULL;
        char *pValue = NULL;
        char tem_data[256] = {0};

        memset(tem_data,0,sizeof(tem_data));
        
        
        cJSON *root = NULL;                
        if (!(root = cJSON_Parse((const char *)returndata))) {          //步骤1 json初始化
            break;
        }
       
        if (GetAvailImportDataKp(root, "Code", tem_data, sizeof(tem_data), true) < 0) {       //步骤2  导入code
            break;
        }

        if (0 == strcmp(tem_data,"200"))
        {
            printf("请求成功！\r\n");
        }
        else
        {
            // printf("请求失败：%s\r\n",tem_data);
            // if(returndata)
            //     free(returndata);
		    // returndata = NULL;
            // if(root)
            //     cJSON_Delete(root);
            // break;

            cJSON *item_msg = NULL;
            char *v_str = NULL;
            item_msg = cJSON_GetObjectItem(root, "Msg");
            if (item_msg != NULL) {
                if (item_msg->type == cJSON_String) {	
                    v_str = item_msg->valuestring;	
                    printf("Msg = %s\n", v_str);
                }
            }
            sprintf(errinfo_uft8_21, "102%s",v_str);
            printf("请求失败：%s\r\n",errinfo_uft8_21);
            if(returndata)
                free(returndata);
		    returndata = NULL;
            if(root)
                cJSON_Delete(root);
            break;
        }

        cJSON *object_data = cJSON_GetObjectItem(root, "Data");
        if (object_data == NULL)
        {
            printf("请求Data失败\r\n");
            if(returndata)
                free(returndata);
		    returndata = NULL;
            if(root)
                cJSON_Delete(root);
            break;
        }      

        cJSON *item = NULL; 
        // item = cJSON_GetObjectItem(object_data, "code");
        // printf("area_code:%s\r\n",item->valuestring);
        item = cJSON_GetObjectItem(object_data, "reqid");
        printf("reqid:%s\r\n",item->valuestring);        

        char tem_open[64] = {0};
        strcpy(tem_open,item->valuestring);
        
      /////////////////////////////////////////////////////  ClientHello  /////////////////////////////////////////////////////
        
        sprintf(tmp, "/api/v1/ClientHello");  //POST 请求的地址 
        sprintf(content_type, "application/json");

        sprintf(szRepBuf_z, "{\"reqid\":\"%s\"}",tem_open);
        printf("    szRepBuf_z = [ %s ]    \r\n",szRepBuf_z);
        result = socket_http_netPost_post(soft_cert_ip, soft_cert_port, tmp, content_type, szRepBuf_z, 5, &returndata, &returndata_len);
        if (result < 0 || returndata_len <= 0)
        {
            if (returndata != NULL)
            {
                free(returndata);
                returndata = NULL;
            }
            nRet = -1;
        }
        sprintf(szRepBuf, "{\"server_client_hello\":\"%s\"}", returndata);

        memset(tem_data,0,sizeof(tem_data));

        cJSON *root2 = NULL;                     
        if (!(root2 = cJSON_Parse((const char *)returndata))) {          //步骤1 json初始化
            break;
        }
        if (GetAvailImportDataKp(root2, "Code", tem_data, sizeof(tem_data), true) < 0) {       //步骤2  导入code
            break;
        }

        if (0 == strcmp(tem_data,"200"))
        {
            printf("请求成功！\r\n");
        }
        else
        {
            // printf("请求失败：%s\r\n",tem_data);
            // if(returndata)
            //     free(returndata);
		    // returndata = NULL;
            // if(root2)
            //     cJSON_Delete(root2);
            // break;
            
            printf("请求失败：%s\r\n",tem_data);

            cJSON *item_msg = NULL;
            char *v_str = NULL;
            item_msg = cJSON_GetObjectItem(root, "Msg");
            if (item_msg != NULL) {
                if (item_msg->type == cJSON_String) {	
                    v_str = item_msg->valuestring;	
                    printf("Msg = %s\n", v_str);
                }
            }
            sprintf(errinfo_uft8_21, "102%s",v_str);
            printf("请求失败：%s\r\n",errinfo_uft8_21);

            if(returndata)
                free(returndata);
		    returndata = NULL;
            if(root2)
                cJSON_Delete(root2);
            break;
        }

        cJSON *object_data2 = cJSON_GetObjectItem(root2, "Data");
        if (object_data2 == NULL)
        {
            printf("请求Data失败\r\n");
            if(returndata)
                free(returndata);
		    returndata = NULL;
            if(root2)
                cJSON_Delete(root2);
            break;
        }     
        printf("Data:%s\r\n",object_data2->valuestring);

        //cJSON *item2 = NULL;  
        // item = cJSON_GetObjectItem(object_data2, "code");
        // printf("area_code:%s\r\n",item2->valuestring);
        
        //item = cJSON_GetObjectItem(object_data2, "reqid");
        //printf("Data:%s\r\n",object_data2->valuestring);        

        char tem_hello[1024] = {0};
        strcpy(tem_hello,object_data2->valuestring);    
        sprintf(szRepBuf, "{\"server_client_hello\":\"%s\"}", tem_hello);
        free(returndata);
		returndata = NULL;


        if (BuildTPassFormatHMacSHA256Packet(true, 2, szRepBuf, ctx->szRandom16BytesKey, buf))
            break;
        //需要在回调函数中使用hi->ctxApp.szTmpBuf中保存的uuid
        hi->cbHttpNewReq = CallbackTPassSetHttpHeaderUuid;
        if (EtaxHTTPPost(hi, "/sys-api/v1.0/auth/cert/serverHello", "application/json",
                     (char *)evbuffer_pullup(buf, -1), evbuffer_get_length(buf), szRepBuf,
                     sizeof(szRepBuf)) < 0) {
            nRet = -2;
            break;
        }
        if (DecodeTpassReponseJson(hi, szRepBuf, "")) {
            nRet = -3;
            break;
        }
        if (TPassLoginAuthDecodeStep3RepForStep4_soft_cert_guizhou(hi, uriTpassFull, ctx->szRandom16BytesKey,szRepBuf, sizeof(szRepBuf),"",soft_cert_ip, soft_cert_port,tem_open,tem_hello))
        {
            nRet = -4;
            break;
        }
        //printf("444444444 szRepBuf = [%s]\r\n",szRepBuf);
        if (TpassLogicIo(hi, "/sys-api/v1.0/auth/cert/serverAuth", szRepBuf, sizeof(szRepBuf))) {
            printf("                TpassLogicIo            TpassLogicIo            TpassLogicIo            \r\n");
            nRet = -5;
            break;
        }
        evbuffer_drain(hi->bufHttpRep, -1);
        evbuffer_add_printf(hi->bufHttpRep, "%s", szRepBuf);
        nRet = RET_SUCCESS;
    } while (false);
    if (buf)
        evbuffer_free(buf);
    return nRet;
}

//软证书【江苏】
int TPassLoginAuthHelloAndAuth_soft_cert_21(EHHTTP hi, struct evhttp_uri *uriTpassFull,char *svr_container,char *soft_cert_ip,int soft_cert_port)
{
    int nRet = ERR_GENERIC, nChildRet = 0,result = -1,returndata_len = 0;
	char szRepBuf_z[1024];
	char tmp[100] = {0};
	char content_type[100] = {0};
	char *returndata = NULL;
    struct evbuffer *buf = NULL;
    CtxBridge ctx = (CtxBridge)hi->ctxApp;

    cJSON *root1 = NULL, *objItem = NULL, *objItemChild = NULL;
    cJSON *objItemChild2 = NULL,*objItemToken  = NULL;


    //可变重复用buf，szRepBuf最大接收7k数据,szBuf最大接收3k+数据
    char szRepBuf[9216] = "", szBuf[5120] = "";
    do
    {
        printf("请求的ip: %s \r\n",soft_cert_ip);
        printf("请求的port: %d \r\n",soft_cert_port);

        /////////////////////////////////////////////////////  OpenCert  /////////////////////////////////////////////////////

        if (!(buf = evbuffer_new()))
            break;
        sprintf(tmp, "/api/v1/OpenCert");
        sprintf(content_type, "application/json");
        sprintf(szRepBuf_z, "{\"cpy_num\":\"%s\"}",svr_container); 
        printf("     【1】 OPEN  param     szRepBuf_z = [%s]\r\n",szRepBuf_z);
        result = socket_http_netPost_post(soft_cert_ip, soft_cert_port, tmp, content_type, szRepBuf_z, 5, &returndata, &returndata_len);
        //result = socket_http_netPost_post("192.168.0.247", 18002, tmp, content_type, szRepBuf_z, 5, &returndata, &returndata_len); 
        printf("returndata_len=%d\r\n,returndata=%s\r\n",returndata_len,returndata);       
        if (result < 0 || returndata_len <= 0)
        {
            if (returndata != NULL)
            {
                free(returndata);
                returndata = NULL;
            }
            nRet = -1;
        }
        printf("####################################    OPEN returndata = [%s]\r\n",returndata);

        printf("####################################    deal with  returndata   #################################################\r\n");
        char *tem_p = NULL;
        char *pValue = NULL;
        char tem_data[64] = {0};
        //获取Code
        memset(tem_data,0,sizeof(tem_data));
        
        cJSON *root = NULL;                     
        if (!(root = cJSON_Parse((const char *)returndata))) {          //步骤1 json初始化
            break;
        }
        if (GetAvailImportDataKp(root, "Code", tem_data, sizeof(tem_data), true) < 0) {       //步骤2  导入code
            break;
        }

        if (0 == strcmp(tem_data,"200"))
        {
            printf("请求成功！\r\n");
        }
        else
        {
            printf("请求失败：%s\r\n",tem_data);
            if(returndata)
                free(returndata);
		    returndata = NULL;
            if(root)
                cJSON_Delete(root);
            break;
        }

        cJSON *object_data = cJSON_GetObjectItem(root, "Data");
        if (object_data == NULL)
        {
            printf("请求Data失败\r\n");
            if(returndata)
                free(returndata);
		    returndata = NULL;
            if(root)
                cJSON_Delete(root);
            break;
        }     

        cJSON *item = NULL;  
        // item = cJSON_GetObjectItem(object_data, "code");
        // printf("area_code:%s\r\n",item->valuestring);
        
        item = cJSON_GetObjectItem(object_data, "reqid");
        printf("reqid:%s\r\n",item->valuestring);        

        char tem_open[64] = {0};
        strcpy(tem_open,item->valuestring);
        printf("    OPEN     END  END   tem_open = [ %s ]               \r\n",tem_open);


      /////////////////////////////////////////////////////  ClientHello  /////////////////////////////////////////////////////

        sprintf(tmp, "/api/v1/ClientHello"); 
        sprintf(content_type, "application/json");
        sprintf(szRepBuf_z, "{\"reqid\":\"%s\"}",tem_open);
        printf("     【2】 HELLO  param     szRepBuf_z = [%s]\r\n",szRepBuf_z);
        result = socket_http_netPost_post(soft_cert_ip, soft_cert_port, tmp, content_type, szRepBuf_z, 5, &returndata, &returndata_len);
        if (result < 0 || returndata_len <= 0)
        {
            if (returndata != NULL)
            {
                free(returndata);
                returndata = NULL;
            }
            nRet = -1;
        }
        printf("##########################################  HELLO returndata = [%s]\r\n",returndata);
        sprintf(szRepBuf, "{\"server_client_hello\":\"%s\"}", returndata);

        //获取Code
        memset(tem_data,0,sizeof(tem_data));

        cJSON *root2 = NULL;                     
        if (!(root2 = cJSON_Parse((const char *)returndata))) {          //步骤1 json初始化
            break;
        }
        if (GetAvailImportDataKp(root2, "Code", tem_data, sizeof(tem_data), true) < 0) {       //步骤2  导入code
            break;
        }

        if (0 == strcmp(tem_data,"200"))
        {
            printf("请求成功！\r\n");
        }
        else
        {
            printf("请求失败：%s\r\n",tem_data);
            if(returndata)
                free(returndata);
		    returndata = NULL;
            if(root2)
                cJSON_Delete(root2);
            break;
        }

        cJSON *object_data2 = cJSON_GetObjectItem(root2, "Data");
        if (object_data2 == NULL)
        {
            printf("请求Data失败\r\n");
            if(returndata)
                free(returndata);
		    returndata = NULL;
            if(root2)
                cJSON_Delete(root2);
            break;
        }     
        printf("hello  Data:%s\r\n",object_data2->valuestring);

        //cJSON *item2 = NULL;  
        // item = cJSON_GetObjectItem(object_data2, "code");
        // printf("area_code:%s\r\n",item2->valuestring);
        
        //item = cJSON_GetObjectItem(object_data2, "reqid");
        //printf("Data:%s\r\n",object_data2->valuestring);        

        char tem_hello[1024] = {0};
        strcpy(tem_hello,object_data2->valuestring);
        sprintf(szRepBuf, "{\"server_client_hello\":\"%s\"}", tem_hello);

        free(returndata);
		returndata = NULL;

        if (BuildTPassFormatHMacSHA256Packet(true, 2, szRepBuf, ctx->szRandom16BytesKey, buf))
            break;
        //需要在回调函数中使用hi->ctxApp.szTmpBuf中保存的uuid
        hi->cbHttpNewReq = CallbackTPassSetHttpHeaderUuid;
        if (EtaxHTTPPost(hi, "/sys-api/v1.0/auth/cert/serverHello", "application/json",
                     (char *)evbuffer_pullup(buf, -1), evbuffer_get_length(buf), szRepBuf,
                     sizeof(szRepBuf)) < 0) {
            nRet = -2;
            break;
        }
        if (DecodeTpassReponseJson(hi, szRepBuf, "")) {
            nRet = -3;
            break;
        }
        if (TPassLoginAuthDecodeStep3RepForStep4_soft_cert_21(hi, uriTpassFull, ctx->szRandom16BytesKey,szRepBuf, sizeof(szRepBuf),"",soft_cert_ip, soft_cert_port,tem_open,tem_hello))
        {
            nRet = -4;
            break;
        }
        //printf("444444444 szRepBuf = [%s]\r\n",szRepBuf);
        if (TpassLogicIo(hi, "/sys-api/v1.0/auth/cert/serverAuth", szRepBuf, sizeof(szRepBuf))) {
            printf("                TpassLogicIo            TpassLogicIo            TpassLogicIo            \r\n");
            nRet = -5;
            break;
        }
        evbuffer_drain(hi->bufHttpRep, -1);
        evbuffer_add_printf(hi->bufHttpRep, "%s", szRepBuf);
        nRet = RET_SUCCESS;
    } while (false);
    if (buf)
        evbuffer_free(buf);
    return nRet;
}

int TPassLoginAuthHelloAndAuth_soft_cert_21_Msg(EHHTTP hi, struct evhttp_uri *uriTpassFull,char *svr_container,char *soft_cert_ip,int soft_cert_port,char *errinfo_uft8_21)
{
    int nRet = ERR_GENERIC, nChildRet = 0,result = -1,returndata_len = 0;
	char szRepBuf_z[1024];
	char tmp[100] = {0};
	char content_type[100] = {0};
	char *returndata = NULL;
    struct evbuffer *buf = NULL;
    CtxBridge ctx = (CtxBridge)hi->ctxApp;

    cJSON *root1 = NULL, *objItem = NULL, *objItemChild = NULL;
    cJSON *objItemChild2 = NULL,*objItemToken  = NULL;


    //可变重复用buf，szRepBuf最大接收7k数据,szBuf最大接收3k+数据
    char szRepBuf[9216] = "", szBuf[5120] = "";
    do
    {
        printf("请求的ip: %s \r\n",soft_cert_ip);
        printf("请求的port: %d \r\n",soft_cert_port);

        /////////////////////////////////////////////////////  OpenCert  /////////////////////////////////////////////////////

        if (!(buf = evbuffer_new()))
            break;
        sprintf(tmp, "/api/v1/OpenCert");
        sprintf(content_type, "application/json");
        sprintf(szRepBuf_z, "{\"cpy_num\":\"%s\"}",svr_container); 
        printf("     【1】 OPEN  param     szRepBuf_z = [%s]\r\n",szRepBuf_z);
        result = socket_http_netPost_post(soft_cert_ip, soft_cert_port, tmp, content_type, szRepBuf_z, 5, &returndata, &returndata_len);
        //result = socket_http_netPost_post("192.168.0.247", 18002, tmp, content_type, szRepBuf_z, 5, &returndata, &returndata_len); 
        printf("returndata_len=%d\r\n,returndata=%s\r\n",returndata_len,returndata);       
        if (result < 0 || returndata_len <= 0)
        {
            if (returndata != NULL)
            {
                free(returndata);
                returndata = NULL;
            }
            nRet = -1;
        }
        printf("####################################    OPEN returndata = [%s]\r\n",returndata);

        printf("####################################    deal with  returndata   #################################################\r\n");
        char *tem_p = NULL;
        char *pValue = NULL;
        char tem_data[64] = {0};
        //获取Code
        memset(tem_data,0,sizeof(tem_data));
        
        cJSON *root = NULL;                     
        if (!(root = cJSON_Parse((const char *)returndata))) {          //步骤1 json初始化
            break;
        }
        if (GetAvailImportDataKp(root, "Code", tem_data, sizeof(tem_data), true) < 0) {       //步骤2  导入code
            break;
        }
        
        if (0 == strcmp(tem_data,"200"))
        {
            printf("请求成功！\r\n");
        }
        else
        {
            cJSON *item_msg = NULL;
            char *v_str = NULL;
            item_msg = cJSON_GetObjectItem(root, "Msg");
            if (item_msg != NULL) {
                if (item_msg->type == cJSON_String) {	
                    v_str = item_msg->valuestring;	
                    printf("Msg = %s\n", v_str);
                }
            }
            sprintf(errinfo_uft8_21, "102%s",v_str);
            printf("请求失败：%s\r\n",errinfo_uft8_21);
            if(returndata)
                free(returndata);
		    returndata = NULL;
            if(root)
                cJSON_Delete(root);
            break;
        }

        cJSON *object_data = cJSON_GetObjectItem(root, "Data");
        if (object_data == NULL)
        {
            printf("请求Data失败\r\n");
            if(returndata)
                free(returndata);
		    returndata = NULL;
            if(root)
                cJSON_Delete(root);
            break;
        }     

        cJSON *item = NULL;  
        // item = cJSON_GetObjectItem(object_data, "code");
        // printf("area_code:%s\r\n",item->valuestring);
        
        item = cJSON_GetObjectItem(object_data, "reqid");
        printf("reqid:%s\r\n",item->valuestring);        

        char tem_open[64] = {0};
        strcpy(tem_open,item->valuestring);
        printf("    OPEN     END  END   tem_open = [ %s ]               \r\n",tem_open);


      /////////////////////////////////////////////////////  ClientHello  /////////////////////////////////////////////////////

        sprintf(tmp, "/api/v1/ClientHello"); 
        sprintf(content_type, "application/json");
        sprintf(szRepBuf_z, "{\"reqid\":\"%s\"}",tem_open);
        printf("     【2】 HELLO  param     szRepBuf_z = [%s]\r\n",szRepBuf_z);
        result = socket_http_netPost_post(soft_cert_ip, soft_cert_port, tmp, content_type, szRepBuf_z, 5, &returndata, &returndata_len);
        if (result < 0 || returndata_len <= 0)
        {
            if (returndata != NULL)
            {
                free(returndata);
                returndata = NULL;
            }
            nRet = -1;
        }
        printf("##########################################  HELLO returndata = [%s]\r\n",returndata);
        sprintf(szRepBuf, "{\"server_client_hello\":\"%s\"}", returndata);

        //获取Code
        memset(tem_data,0,sizeof(tem_data));

        cJSON *root2 = NULL;                     
        if (!(root2 = cJSON_Parse((const char *)returndata))) {          //步骤1 json初始化
            break;
        }
        if (GetAvailImportDataKp(root2, "Code", tem_data, sizeof(tem_data), true) < 0) {       //步骤2  导入code
            break;
        }

        if (0 == strcmp(tem_data,"200"))
        {
            printf("请求成功！\r\n");
        }
        else
        {
            printf("请求失败：%s\r\n",tem_data);

            cJSON *item_msg = NULL;
            char *v_str = NULL;
            item_msg = cJSON_GetObjectItem(root, "Msg");
            if (item_msg != NULL) {
                if (item_msg->type == cJSON_String) {	
                    v_str = item_msg->valuestring;	
                    printf("Msg = %s\n", v_str);
                }
            }
            sprintf(errinfo_uft8_21, "102%s",v_str);
            printf("请求失败：%s\r\n",errinfo_uft8_21);

            if(returndata)
                free(returndata);
		    returndata = NULL;
            if(root2)
                cJSON_Delete(root2);
            break;
        }

        cJSON *object_data2 = cJSON_GetObjectItem(root2, "Data");
        if (object_data2 == NULL)
        {
            printf("请求Data失败\r\n");
            if(returndata)
                free(returndata);
		    returndata = NULL;
            if(root2)
                cJSON_Delete(root2);
            break;
        }     
        printf("hello  Data:%s\r\n",object_data2->valuestring);

        //cJSON *item2 = NULL;  
        // item = cJSON_GetObjectItem(object_data2, "code");
        // printf("area_code:%s\r\n",item2->valuestring);
        
        //item = cJSON_GetObjectItem(object_data2, "reqid");
        //printf("Data:%s\r\n",object_data2->valuestring);        

        char tem_hello[1024] = {0};
        strcpy(tem_hello,object_data2->valuestring);
        sprintf(szRepBuf, "{\"server_client_hello\":\"%s\"}", tem_hello);

        free(returndata);
		returndata = NULL;

        if (BuildTPassFormatHMacSHA256Packet(true, 2, szRepBuf, ctx->szRandom16BytesKey, buf))
            break;
        //需要在回调函数中使用hi->ctxApp.szTmpBuf中保存的uuid
        hi->cbHttpNewReq = CallbackTPassSetHttpHeaderUuid;
        if (EtaxHTTPPost(hi, "/sys-api/v1.0/auth/cert/serverHello", "application/json",
                     (char *)evbuffer_pullup(buf, -1), evbuffer_get_length(buf), szRepBuf,
                     sizeof(szRepBuf)) < 0) {
            nRet = -2;
            break;
        }
        if (DecodeTpassReponseJson(hi, szRepBuf, "")) {
            printf("111111111111111111111111111111111111111111111111111\r\n");
            nRet = -3;
            break;
        }
        if (TPassLoginAuthDecodeStep3RepForStep4_soft_cert_21(hi, uriTpassFull, ctx->szRandom16BytesKey,szRepBuf, sizeof(szRepBuf),"",soft_cert_ip, soft_cert_port,tem_open,tem_hello))
        {
            printf("1111111111111111111111111111111111111111111111111112\r\n");
            nRet = -4;
            break;
        }
        printf("444444444 szRepBuf = [%s]\r\n",szRepBuf);
        if (TpassLogicIo(hi, "/sys-api/v1.0/auth/cert/serverAuth", szRepBuf, sizeof(szRepBuf))) {
            printf("                TpassLogicIo            TpassLogicIo            TpassLogicIo            \r\n");
            nRet = -5;
            break;
        }
        printf("44444444455555555555 szRepBuf = [%s]\r\n",szRepBuf);
        evbuffer_drain(hi->bufHttpRep, -1);
        evbuffer_add_printf(hi->bufHttpRep, "%s", szRepBuf);
        nRet = RET_SUCCESS;
    } while (false);
    if (buf)
        evbuffer_free(buf);
    return nRet;
}


int TPassLoginAuthHelloAndAuth_soft_cert_21_Msg_shaanxi(EHHTTP hi, struct evhttp_uri *uriTpassFull,char *svr_container,char *soft_cert_ip,int soft_cert_port,char *errinfo_uft8_21)
{
    int nRet = ERR_GENERIC, nChildRet = 0,result = -1,returndata_len = 0;
	char szRepBuf_z[1024];
	char tmp[100] = {0};
	char content_type[100] = {0};
	char *returndata = NULL;
    struct evbuffer *buf = NULL;
    CtxBridge ctx = (CtxBridge)hi->ctxApp;

    cJSON *root1 = NULL, *objItem = NULL, *objItemChild = NULL;
    cJSON *objItemChild2 = NULL,*objItemToken  = NULL;


    //可变重复用buf，szRepBuf最大接收7k数据,szBuf最大接收3k+数据
    char szRepBuf[9216] = "", szBuf[5120] = "";
    do
    {
        printf("请求的ip: %s \r\n",soft_cert_ip);
        printf("请求的port: %d \r\n",soft_cert_port);

        /////////////////////////////////////////////////////  OpenCert  /////////////////////////////////////////////////////

        if (!(buf = evbuffer_new()))
            break;
        sprintf(tmp, "/api/v1/OpenCert");
        sprintf(content_type, "application/json");
        sprintf(szRepBuf_z, "{\"cpy_num\":\"%s\"}",svr_container); 
        printf("     【1】 OPEN  param     szRepBuf_z = [%s]\r\n",szRepBuf_z);
        result = socket_http_netPost_post(soft_cert_ip, soft_cert_port, tmp, content_type, szRepBuf_z, 5, &returndata, &returndata_len);
        //result = socket_http_netPost_post("192.168.0.247", 18002, tmp, content_type, szRepBuf_z, 5, &returndata, &returndata_len); 
        printf("returndata_len=%d\r\n,returndata=%s\r\n",returndata_len,returndata);       
        if (result < 0 || returndata_len <= 0)
        {
            if (returndata != NULL)
            {
                free(returndata);
                returndata = NULL;
            }
            nRet = -1;
        }
        printf("####################################    OPEN returndata = [%s]\r\n",returndata);

        printf("####################################    deal with  returndata   #################################################\r\n");
        char *tem_p = NULL;
        char *pValue = NULL;
        char tem_data[64] = {0};
        //获取Code
        memset(tem_data,0,sizeof(tem_data));
        
        cJSON *root = NULL;                     
        if (!(root = cJSON_Parse((const char *)returndata))) {          //步骤1 json初始化
            break;
        }
        if (GetAvailImportDataKp(root, "Code", tem_data, sizeof(tem_data), true) < 0) {       //步骤2  导入code
            break;
        }
        
        if (0 == strcmp(tem_data,"200"))
        {
            printf("请求成功！\r\n");
        }
        else
        {
            cJSON *item_msg = NULL;
            char *v_str = NULL;
            item_msg = cJSON_GetObjectItem(root, "Msg");
            if (item_msg != NULL) {
                if (item_msg->type == cJSON_String) {	
                    v_str = item_msg->valuestring;	
                    printf("Msg = %s\n", v_str);
                }
            }
            sprintf(errinfo_uft8_21, "102%s",v_str);
            printf("请求失败：%s\r\n",errinfo_uft8_21);
            if(returndata)
                free(returndata);
		    returndata = NULL;
            if(root)
                cJSON_Delete(root);
            break;
        }

        cJSON *object_data = cJSON_GetObjectItem(root, "Data");
        if (object_data == NULL)
        {
            printf("请求Data失败\r\n");
            if(returndata)
                free(returndata);
		    returndata = NULL;
            if(root)
                cJSON_Delete(root);
            break;
        }     

        cJSON *item = NULL;  
        // item = cJSON_GetObjectItem(object_data, "code");
        // printf("area_code:%s\r\n",item->valuestring);
        
        item = cJSON_GetObjectItem(object_data, "reqid");
        printf("reqid:%s\r\n",item->valuestring);        

        char tem_open[64] = {0};
        strcpy(tem_open,item->valuestring);
        printf("    OPEN     END  END   tem_open = [ %s ]               \r\n",tem_open);


      /////////////////////////////////////////////////////  ClientHello  /////////////////////////////////////////////////////

        sprintf(tmp, "/api/v1/ClientHello"); 
        sprintf(content_type, "application/json");
        sprintf(szRepBuf_z, "{\"reqid\":\"%s\"}",tem_open);
        printf("     【2】 HELLO  param     szRepBuf_z = [%s]\r\n",szRepBuf_z);
        result = socket_http_netPost_post(soft_cert_ip, soft_cert_port, tmp, content_type, szRepBuf_z, 5, &returndata, &returndata_len);
        if (result < 0 || returndata_len <= 0)
        {
            if (returndata != NULL)
            {
                free(returndata);
                returndata = NULL;
            }
            nRet = -1;
        }
        printf("##########################################  HELLO returndata = [%s]\r\n",returndata);
        sprintf(szRepBuf, "{\"server_client_hello\":\"%s\"}", returndata);

        //获取Code
        memset(tem_data,0,sizeof(tem_data));

        cJSON *root2 = NULL;                     
        if (!(root2 = cJSON_Parse((const char *)returndata))) {          //步骤1 json初始化
            break;
        }
        if (GetAvailImportDataKp(root2, "Code", tem_data, sizeof(tem_data), true) < 0) {       //步骤2  导入code
            break;
        }

        if (0 == strcmp(tem_data,"200"))
        {
            printf("请求成功！\r\n");
        }
        else
        {
            printf("请求失败：%s\r\n",tem_data);

            cJSON *item_msg = NULL;
            char *v_str = NULL;
            item_msg = cJSON_GetObjectItem(root, "Msg");
            if (item_msg != NULL) {
                if (item_msg->type == cJSON_String) {	
                    v_str = item_msg->valuestring;	
                    printf("Msg = %s\n", v_str);
                }
            }
            sprintf(errinfo_uft8_21, "102%s",v_str);
            printf("请求失败：%s\r\n",errinfo_uft8_21);

            if(returndata)
                free(returndata);
		    returndata = NULL;
            if(root2)
                cJSON_Delete(root2);
            break;
        }

        cJSON *object_data2 = cJSON_GetObjectItem(root2, "Data");
        if (object_data2 == NULL)
        {
            printf("请求Data失败\r\n");
            if(returndata)
                free(returndata);
		    returndata = NULL;
            if(root2)
                cJSON_Delete(root2);
            break;
        }     
        printf("hello  Data:%s\r\n",object_data2->valuestring);

        //cJSON *item2 = NULL;  
        // item = cJSON_GetObjectItem(object_data2, "code");
        // printf("area_code:%s\r\n",item2->valuestring);
        
        //item = cJSON_GetObjectItem(object_data2, "reqid");
        //printf("Data:%s\r\n",object_data2->valuestring);        

        char tem_hello[1024] = {0};
        strcpy(tem_hello,object_data2->valuestring);
        sprintf(szRepBuf, "{\"server_client_hello\":\"%s\"}", tem_hello);

        free(returndata);
		returndata = NULL;

        if (BuildTPassFormatHMacSHA256Packet(true, 2, szRepBuf, ctx->szRandom16BytesKey, buf))
            break;
        //需要在回调函数中使用hi->ctxApp.szTmpBuf中保存的uuid
        hi->cbHttpNewReq = CallbackTPassSetHttpHeaderUuid;
        if (EtaxHTTPPost(hi, "/sys-api/v1.0/auth/cert/serverHello", "application/json",
                     (char *)evbuffer_pullup(buf, -1), evbuffer_get_length(buf), szRepBuf,
                     sizeof(szRepBuf)) < 0) {
            nRet = -2;
            break;
        }
        if (DecodeTpassReponseJson(hi, szRepBuf, "")) {
            printf("111111111111111111111111111111111111111111111111111\r\n");
            nRet = -3;
            break;
        }
        if (TPassLoginAuthDecodeStep3RepForStep4_soft_cert_21_shaanxi(hi, uriTpassFull, ctx->szRandom16BytesKey,szRepBuf, sizeof(szRepBuf),"",soft_cert_ip, soft_cert_port,tem_open,tem_hello))
        {
            printf("1111111111111111111111111111111111111111111111111112\r\n");
            nRet = -4;
            break;
        }
        printf("444444444 szRepBuf = [%s]\r\n",szRepBuf);
        if (TpassLogicIo(hi, "/sys-api/v1.0/auth/cert/serverAuth", szRepBuf, sizeof(szRepBuf))) {
            printf("                TpassLogicIo            TpassLogicIo            TpassLogicIo            \r\n");
            nRet = -5;
            break;
        }
        printf("44444444455555555555 szRepBuf = [%s]\r\n",szRepBuf);
        evbuffer_drain(hi->bufHttpRep, -1);
        evbuffer_add_printf(hi->bufHttpRep, "%s", szRepBuf);
        nRet = RET_SUCCESS;
    } while (false);
    if (buf)
        evbuffer_free(buf);
    return nRet;
}


int TPassLoginAuthHelloAndAuth_soft_cert_21_Msg_guangxi(EHHTTP hi, struct evhttp_uri *uriTpassFull,char *svr_container,char *soft_cert_ip,int soft_cert_port,char *errinfo_uft8_21)
{
    int nRet = ERR_GENERIC, nChildRet = 0,result = -1,returndata_len = 0;
	char szRepBuf_z[1024];
	char tmp[100] = {0};
	char content_type[100] = {0};
	char *returndata = NULL;
    struct evbuffer *buf = NULL;
    CtxBridge ctx = (CtxBridge)hi->ctxApp;

    cJSON *root1 = NULL, *objItem = NULL, *objItemChild = NULL;
    cJSON *objItemChild2 = NULL,*objItemToken  = NULL;


    //可变重复用buf，szRepBuf最大接收7k数据,szBuf最大接收3k+数据
    char szRepBuf[9216] = "", szBuf[5120] = "";
    do
    {
        printf("请求的ip: %s \r\n",soft_cert_ip);
        printf("请求的port: %d \r\n",soft_cert_port);

        /////////////////////////////////////////////////////  OpenCert  /////////////////////////////////////////////////////

        if (!(buf = evbuffer_new()))
            break;
        sprintf(tmp, "/api/v1/OpenCert");
        sprintf(content_type, "application/json");
        sprintf(szRepBuf_z, "{\"cpy_num\":\"%s\"}",svr_container); 
        printf("     【1】 OPEN  param     szRepBuf_z = [%s]\r\n",szRepBuf_z);
        result = socket_http_netPost_post(soft_cert_ip, soft_cert_port, tmp, content_type, szRepBuf_z, 5, &returndata, &returndata_len);
        //result = socket_http_netPost_post("192.168.0.247", 18002, tmp, content_type, szRepBuf_z, 5, &returndata, &returndata_len); 
        printf("returndata_len=%d\r\n,returndata=%s\r\n",returndata_len,returndata);       
        if (result < 0 || returndata_len <= 0)
        {
            if (returndata != NULL)
            {
                free(returndata);
                returndata = NULL;
            }
            nRet = -1;
        }
        printf("####################################    OPEN returndata = [%s]\r\n",returndata);

        printf("####################################    deal with  returndata   #################################################\r\n");
        char *tem_p = NULL;
        char *pValue = NULL;
        char tem_data[64] = {0};
        //获取Code
        memset(tem_data,0,sizeof(tem_data));
        
        cJSON *root = NULL;                     
        if (!(root = cJSON_Parse((const char *)returndata))) {          //步骤1 json初始化
            break;
        }
        if (GetAvailImportDataKp(root, "Code", tem_data, sizeof(tem_data), true) < 0) {       //步骤2  导入code
            break;
        }
        
        if (0 == strcmp(tem_data,"200"))
        {
            printf("请求成功！\r\n");
        }
        else
        {
            cJSON *item_msg = NULL;
            char *v_str = NULL;
            item_msg = cJSON_GetObjectItem(root, "Msg");
            if (item_msg != NULL) {
                if (item_msg->type == cJSON_String) {	
                    v_str = item_msg->valuestring;	
                    printf("Msg = %s\n", v_str);
                }
            }
            sprintf(errinfo_uft8_21, "102%s",v_str);
            printf("请求失败：%s\r\n",errinfo_uft8_21);
            if(returndata)
                free(returndata);
		    returndata = NULL;
            if(root)
                cJSON_Delete(root);
            break;
        }

        cJSON *object_data = cJSON_GetObjectItem(root, "Data");
        if (object_data == NULL)
        {
            printf("请求Data失败\r\n");
            if(returndata)
                free(returndata);
		    returndata = NULL;
            if(root)
                cJSON_Delete(root);
            break;
        }     

        cJSON *item = NULL;  
        // item = cJSON_GetObjectItem(object_data, "code");
        // printf("area_code:%s\r\n",item->valuestring);
        
        item = cJSON_GetObjectItem(object_data, "reqid");
        printf("reqid:%s\r\n",item->valuestring);        

        char tem_open[64] = {0};
        strcpy(tem_open,item->valuestring);
        printf("    OPEN     END  END   tem_open = [ %s ]               \r\n",tem_open);


      /////////////////////////////////////////////////////  ClientHello  /////////////////////////////////////////////////////

        sprintf(tmp, "/api/v1/ClientHello"); 
        sprintf(content_type, "application/json");
        sprintf(szRepBuf_z, "{\"reqid\":\"%s\"}",tem_open);
        printf("     【2】 HELLO  param     szRepBuf_z = [%s]\r\n",szRepBuf_z);
        result = socket_http_netPost_post(soft_cert_ip, soft_cert_port, tmp, content_type, szRepBuf_z, 5, &returndata, &returndata_len);
        if (result < 0 || returndata_len <= 0)
        {
            if (returndata != NULL)
            {
                free(returndata);
                returndata = NULL;
            }
            nRet = -1;
        }
        printf("##########################################  HELLO returndata = [%s]\r\n",returndata);
        sprintf(szRepBuf, "{\"server_client_hello\":\"%s\"}", returndata);

        //获取Code
        memset(tem_data,0,sizeof(tem_data));

        cJSON *root2 = NULL;                     
        if (!(root2 = cJSON_Parse((const char *)returndata))) {          //步骤1 json初始化
            break;
        }
        if (GetAvailImportDataKp(root2, "Code", tem_data, sizeof(tem_data), true) < 0) {       //步骤2  导入code
            break;
        }

        if (0 == strcmp(tem_data,"200"))
        {
            printf("请求成功！\r\n");
        }
        else
        {
            printf("请求失败：%s\r\n",tem_data);

            cJSON *item_msg = NULL;
            char *v_str = NULL;
            item_msg = cJSON_GetObjectItem(root, "Msg");
            if (item_msg != NULL) {
                if (item_msg->type == cJSON_String) {	
                    v_str = item_msg->valuestring;	
                    printf("Msg = %s\n", v_str);
                }
            }
            sprintf(errinfo_uft8_21, "102%s",v_str);
            printf("请求失败：%s\r\n",errinfo_uft8_21);

            if(returndata)
                free(returndata);
		    returndata = NULL;
            if(root2)
                cJSON_Delete(root2);
            break;
        }

        cJSON *object_data2 = cJSON_GetObjectItem(root2, "Data");
        if (object_data2 == NULL)
        {
            printf("请求Data失败\r\n");
            if(returndata)
                free(returndata);
		    returndata = NULL;
            if(root2)
                cJSON_Delete(root2);
            break;
        }     
        printf("hello  Data:%s\r\n",object_data2->valuestring);

        //cJSON *item2 = NULL;  
        // item = cJSON_GetObjectItem(object_data2, "code");
        // printf("area_code:%s\r\n",item2->valuestring);
        
        //item = cJSON_GetObjectItem(object_data2, "reqid");
        //printf("Data:%s\r\n",object_data2->valuestring);        

        char tem_hello[1024] = {0};
        strcpy(tem_hello,object_data2->valuestring);
        sprintf(szRepBuf, "{\"server_client_hello\":\"%s\"}", tem_hello);

        free(returndata);
		returndata = NULL;

        if (BuildTPassFormatHMacSHA256Packet(true, 2, szRepBuf, ctx->szRandom16BytesKey, buf))
            break;
        //需要在回调函数中使用hi->ctxApp.szTmpBuf中保存的uuid
        hi->cbHttpNewReq = CallbackTPassSetHttpHeaderUuid;
        if (EtaxHTTPPost(hi, "/sys-api/v1.0/auth/cert/serverHello", "application/json",
                     (char *)evbuffer_pullup(buf, -1), evbuffer_get_length(buf), szRepBuf,
                     sizeof(szRepBuf)) < 0) {
            nRet = -2;
            break;
        }
        if (DecodeTpassReponseJson(hi, szRepBuf, "")) {
            printf("111111111111111111111111111111111111111111111111111\r\n");
            nRet = -3;
            break;
        }
        if (TPassLoginAuthDecodeStep3RepForStep4_soft_cert_21_guangxi(hi, uriTpassFull, ctx->szRandom16BytesKey,szRepBuf, sizeof(szRepBuf),"",soft_cert_ip, soft_cert_port,tem_open,tem_hello))
        {
            printf("1111111111111111111111111111111111111111111111111112\r\n");
            nRet = -4;
            break;
        }
        printf("444444444 szRepBuf = [%s]\r\n",szRepBuf);
        if (TpassLogicIo(hi, "/sys-api/v1.0/auth/cert/serverAuth", szRepBuf, sizeof(szRepBuf))) {
            printf("                TpassLogicIo            TpassLogicIo            TpassLogicIo            \r\n");
            nRet = -5;
            break;
        }
        printf("44444444455555555555 szRepBuf = [%s]\r\n",szRepBuf);
        evbuffer_drain(hi->bufHttpRep, -1);
        evbuffer_add_printf(hi->bufHttpRep, "%s", szRepBuf);
        nRet = RET_SUCCESS;
    } while (false);
    if (buf)
        evbuffer_free(buf);
    return nRet;
}
