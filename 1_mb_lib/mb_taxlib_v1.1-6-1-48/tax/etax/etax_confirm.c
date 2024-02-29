/*****************************************************************************
File name:   etax_confirm.c
Description:用于税局发票综合服务平台\发票确认平台连接及操作；
            2022年6月24日上海税务局全电试点，预计后续传统确认平台(该模块)将会被停用
Author:      Zako
Version:     1.0
Date:        2021.11
History:
20211114     最初代码实现
20221002     针对部分省份(上海等)逐渐弃用确认平台，我方代码逐渐收缩确认平台功能，
             将确认平台代码模块化，暂时不再纳入主干代码进行维护
20230202     和业务线沟通后，该模块还在继续使用，从税务局模块独立出来，成立etax电子税务局模块
*****************************************************************************/
#include "etax_confirm.h"
// HTTP应用层上下文结构
struct ContextHttpAppConfirm {
    char szVersion[BS_16];    //版本信息
    char szToken[BS_NORMAL];  //登陆成功后token保存
};
typedef struct ContextHttpAppConfirm *CtxConfirm;

// szOutBuf 空间必须足够大
void GetJQueryRandomCallback(char *szOutBuf, char *szQueryPath)
{
    char szRandom[24] = "";
    GetRandHexString(szRandom, 20, 10, NULL);
    sprintf(szOutBuf, "%s?callback=jQuery%s_%llu", szQueryPath, szRandom,
            GetMicroSecondUnixTimestamp());
}

int FormatReponseJson(struct evbuffer *bufHttpRep)
{
    // jQuery91142927356834238507_1637108736716({"key1":"01","key2":"...","key3":"..."})
    // check
    char *pStart = (char *)evbuffer_find(bufHttpRep, (const unsigned char *)"(", 1);
    char *pEnd = (char *)evbuffer_find(bufHttpRep, (const unsigned char *)")", 1);
    char *pContent = (char *)evbuffer_pullup(bufHttpRep, -1);
    int nRet = RET_SUCCESS;
    if (strncmp(pContent, "jQuery", 6)) {
        return ERR_BUF_CHECK;
    }
    if (!pStart || !pEnd)
        return ERR_BUF_CHECK;
    // do
    struct evbuffer *opBuf = evbuffer_new();
    do {
        evbuffer_remove_buffer(bufHttpRep, opBuf, evbuffer_get_length(bufHttpRep));
        int size = pStart + 1 - pContent;
        if (size <= 0) {
            nRet = ERR_BUF_CHECK;
            break;
        }
        evbuffer_drain(opBuf, size);  // remove head junk

        pEnd = (char *)evbuffer_find(opBuf, (const unsigned char *)")", 1);
        pContent = (char *)evbuffer_pullup(opBuf, -1);
        size = pEnd - pContent;
        evbuffer_drain(bufHttpRep, evbuffer_get_length(bufHttpRep));  // clear orig
        evbuffer_remove_buffer(opBuf, bufHttpRep, size);
        //末尾追加\0
        uint8 tail = '\0';
        evbuffer_add(bufHttpRep, &tail, 1);
    } while (false);
    evbuffer_free(opBuf);
    return nRet;
}

int LoadJsonFromConfirmReponse(struct evbuffer *bufHttpRep, char *szInOut, int nInBufSize)
{
    int nRet = ERR_BUF_CHECK, nChildRet = 0;
    cJSON *root = NULL;
    do {
        const char *pContent = (const char *)evbuffer_pullup(bufHttpRep, -1);
        if (!(root = cJSON_Parse(pContent)))  // pContent末尾'}'后面可能未\0断开,不影响解析
            break;
        if (!cJSON_GetArraySize(root))
            break;
        if (strlen(szInOut) > 0) {
            cJSON *obj = cJSON_GetObjectItem(root, szInOut);
            if (!obj)
                break;
            if ((nChildRet = snprintf(szInOut, nInBufSize, "%s", obj->valuestring)) !=
                strlen(obj->valuestring)) {
                break;
            }
        } else {
            if ((nChildRet = snprintf(szInOut, nInBufSize, "%s", pContent)) != strlen(pContent)) {
                break;
            }
        }
        nRet = RET_SUCCESS;
    } while (false);
    if (root)
        cJSON_Delete(root);
    return nRet;
}

int GetServerJson(HHTTP hi, char *szPost, char *szQueryPath)
{
    int nChildRet = 0;
    int nRet = 0;
    char szQueryCallbackPath[512] = "";
    do {
        GetJQueryRandomCallback(szQueryCallbackPath, szQueryPath);
        nChildRet = HTTPPostEvent(hi, szQueryCallbackPath, "application/x-www-form-urlencoded",
                                  (unsigned char *)szPost, strlen(szPost));
        if (nChildRet < 0) {
            nRet = nChildRet;
            break;
        }
        // jQuery22141359627387384507_1637288890807XXXXX 需要大于40字节
        if (nChildRet < 40) {
            nRet = ERR_TA_REPONSE_CHECK;
            break;
        }
        nRet = ERR_BUF_CHECK;
        if (FormatReponseJson(hi->bufHttpRep) < 0)
            break;
        nRet = RET_SUCCESS;
    } while (false);
    return nRet;
}

// szOutServerPacket 280bytes
int ConfirmPreLogin(HHTTP hi, uint16 opFlag, char *szOutServerPacket, int nBufSize)
{
    char szBuf1[1024] = "";
    char szPost[1280] = "";
    char szQueryPath[512] = "";
    int nRet = RET_SUCCESS;
    CtxConfirm ctx = (CtxConfirm)hi->ctxApp;
    do {
        switch (opFlag) {
            case ETAX_HELLO:
                if (GetTaClientHello(0, hi->hDev, szBuf1) < 0) {
                    nRet = ERR_TA_CLIENTHELLO;
                    break;
                }
                // data "type=CLIENT-HELLO&clientHello=3071020103305831...20402&alg=1&ymbb=4.0.16";
                sprintf(szPost, "type=CLIENT-HELLO&clientHello=%s&alg=1&ymbb=%s", szBuf1,
                        ctx->szVersion);
                strcpy(szQueryPath, "/NSbsqWW/login.do");
                strcpy(szOutServerPacket, "key2");
                break;
            case ETAX_QUERYMM:
                sprintf(szPost, "cert=%s&funType=01", hi->hDev->szCommonTaxID);
                strcpy(szQueryPath, "/NSbsqWW/querymm.do");
                strcpy(szOutServerPacket, "ts");
                break;
            case ETAX_LOGOUT:
                sprintf(szPost, "cert=%s&token=%s", hi->hDev->szCommonTaxID, ctx->szToken);
                strcpy(szQueryPath, "/NSbsqWW/quit.do");
                strcpy(szOutServerPacket, "key1");
                break;
            default:
                nRet = ERR_GENERIC;
                break;
        }
        if (RET_SUCCESS != nRet) {
            break;
        }
        if (GetServerJson(hi, szPost, szQueryPath) < 0) {
            break;
        }
        if (LoadJsonFromConfirmReponse(hi->bufHttpRep, szOutServerPacket, nBufSize) < 0)
            break;
        nRet = RET_SUCCESS;
    } while (false);
    return nRet;
}

// 4.0.16 4.0.17 线上版本老是更新，版本不对就不停跳转，只能动态获取了
int GetLastestVersion(HHTTP hi)
{
    // https://fpdk.jiangsu.chinatax.gov.cn:81/sigin.1e660de2.html?_1639678753198
    char szQueryPath[256] = "";
    int nRet = ERR_PROGRAME_API;
    CtxConfirm ctx = (CtxConfirm)hi->ctxApp;
    do {
        char szRadom[64] = "";
        // window.location.href = "sigin.1e660de2.html
        if (HTTPGetEvent(hi, "/") < 0)
            break;
        if (FindStringFromEvBuffer(hi->bufHttpRep, "window.location.href = \"sigin.", ".html",
                                   szRadom, sizeof(szRadom)))
            break;
        sprintf(szQueryPath, "/sigin.%s.html?_%llu", szRadom, GetMicroSecondUnixTimestamp());
        // var ymbb = "4.0.17";
        if (HTTPGetEvent(hi, szQueryPath) < 0)
            break;
        // if (FindStringFromEvBuffer(hi->bufHttpRep, "var ymbb = \"", "\"",
        // hi->ctxApp.szVersion,
        //                            sizeof(hi->ctxApp.szVersion)))
        //     break;
        // if (strlen(hi->ctxApp.szVersion) < 4 || strlen(hi->ctxApp.szVersion) > 16)
        //     break;

        // 20221004测试，var ymbb = getYmbb(),版本号已由静态值变为动态获取，由js获取
        // 暂不深入跟进，待需要时再深入研究。测试调试时可以直接填入固定值
        strcpy(ctx->szVersion, "4.0.24");
        nRet = RET_SUCCESS;
    } while (false);
    return nRet;
}

void GetConfirmDevID(uint8 bDevType, char *szInDevID, char *szOutFormatDevID)
{
    switch (bDevType) {
        case DEVICE_AISINO:
            sprintf(szOutFormatDevID, "44%s", szInDevID);
            break;
        case DEVICE_NISEC:
            sprintf(szOutFormatDevID, "33%s", szInDevID);
            break;
        case DEVICE_CNTAX:
            sprintf(szOutFormatDevID, "03-66%s", szInDevID);
            break;
        default:
            break;
    }
}

void AppendCookies(HHTTP hi, const char *szNewCookie)
{
    //追加cookies
    if (evbuffer_get_length(hi->ctxCon.bufCookies) != 0)
        evbuffer_add(hi->ctxCon.bufCookies, ";", 1);
    evbuffer_add(hi->ctxCon.bufCookies, szNewCookie, strlen(szNewCookie));
}

int GetConfirmLoginFaileReason(HHTTP hi)
{
    int nRet = ERR_TA_CONFIRM_LOGIN;
    char szBuf[512] = "";
    strcpy(szBuf, "key1");
    do {
        if (LoadJsonFromConfirmReponse(hi->bufHttpRep, szBuf, sizeof(szBuf)) < 0) {
            SetLastError(hi->hDev->hUSB, ERR_TA_REPONSE_CHECK, "Load ta response failed");
            break;
        }
        if (!strcmp(szBuf, "03")) {
            nRet = RET_SUCCESS;
            break;
        }
        int nErrCode = atoi(szBuf);
        switch (nErrCode) {
            case 1:
                SetLastError(hi->hDev->hUSB, ERR_TA_DATA_ERROR, "确认平台认证:数据库异常/会话超时");
                break;
            case 2:
                SetLastError(hi->hDev->hUSB, ERR_TA_DATA_ERROR,
                             "确认平台认证:纳税人档案信息不存在");
                break;
            case 11:
                SetLastError(
                    hi->hDev->hUSB, ERR_TA_DATA_ERROR,
                    "确认平台认证:系统检测到您当前使用的税号为您的旧税号，请核实并使用新税号");
                break;
            case 12:
                SetLastError(hi->hDev->hUSB, ERR_TA_DATA_ERROR,
                             "确认平台认证:本企业属于取消认证政策的纳税人");
                break;
            case 13:
                SetLastError(hi->hDev->hUSB, ERR_TA_DATA_ERROR,
                             "确认平台认证:特定企业不允许进行网上发票认证");
                break;
            case 4:
                SetLastError(hi->hDev->hUSB, ERR_TA_PASSWORD_ERROR, "确认平台认证:平台密码不正确");
                break;
            case 5:
                SetLastError(
                    hi->hDev->hUSB, ERR_TA_PASSWORD_ERROR,
                    "确认平台认证:平台密码错误次数超过十次，请联系税务机关解锁或明天再试！");
                break;
            case 6:
                SetLastError(hi->hDev->hUSB, ERR_TA_DATA_ERROR,
                             "确认平台认证:纳税人档案当前状态为已注销");
                break;
            case 21:
                SetLastError(hi->hDev->hUSB, ERR_TA_DATA_ERROR,
                             "确认平台认证:未启用,无权登录此系统");
                break;
            case 22:
            case 23:
                SetLastError(hi->hDev->hUSB, ERR_TA_DATA_ERROR,
                             "确认平台认证:初始化期初数据出现数据库异常");
                break;
            case 400001:
            case 400002:
                SetLastError(hi->hDev->hUSB, ERR_TA_DATA_ERROR,
                             "确认平台认证:转登记纳税人，未获取到您的转登记认定时间");
                break;
            case 400004:
                SetLastError(hi->hDev->hUSB, ERR_TA_DATA_ERROR, "确认平台认证:纳税人资格代码为空");
                break;
            case 98:
            case 99:
            case 101:
                SetLastError(hi->hDev->hUSB, ERR_TA_DATA_ERROR,
                             "确认平台认证:网络调用异常，请重新登录！");
                break;
            default:
                SetLastError(hi->hDev->hUSB, ERR_TA_DATA_ERROR,
                             "确认平台认证:登陆错误，错误代码:%s", szBuf);
                break;
        }
    } while (false);
    return nRet;
}

int GetConfirmToken(HHTTP hi)
{
    char szAuthCode[5120] = "", szFormatDevID[64] = "", szTotalSendBuf[7680] = "",
         szRandomNum[512] = "";
    int nBufSize = sizeof(szAuthCode);
    HDEV hDev = hi->hDev;
    int nRet = 0;
    CtxConfirm ctx = (CtxConfirm)hi->ctxApp;
    //获取server随机数
    if ((nRet = ConfirmPreLogin(hi, ETAX_HELLO, szRandomNum, sizeof(szRandomNum))) < 0)
        return nRet;
    //生成认证代码
    if (BuildClientAuthCode((unsigned char *)szRandomNum, strlen(szRandomNum),
                            (unsigned char *)szAuthCode, &nBufSize, hDev) < 0)
        return ERR_TA_AUTHCODE;
    //构造post请求
    GetConfirmDevID(hDev->bDeviceType, hDev->szDeviceID, szFormatDevID);
    snprintf(szTotalSendBuf, sizeof(szTotalSendBuf),
             "type=CLIENT-AUTH&clientAuthCode=%s&serverRandom=%s&alg=1&password=&cert=%s&ymbb=%s&"
             "sbh=%s&currdate=%llu",
             szAuthCode, szRandomNum, hDev->szCommonTaxID, ctx->szVersion, szFormatDevID,
             GetMicroSecondUnixTimestamp());
    if (GetServerJson(hi, szTotalSendBuf, "/NSbsqWW/login.do") < 0) {
        return ERR_TA_CONFIRM_LOGIN;
    }
    // jQuery12237472956881345907_1639965699246({"timestamp":"1639965793860","key4":"2021-12-20","key3":"%E5%8D%97%E4%BA%AC%E5%B8%82%E6%B1%9F%E5%AE%81%E5%8C%BA%E6%B7%B3%E8%93%9D%E8%BD%AF%E4%BB%B6%E7%A7%91%E6%8A%80%E6%9C%8D%E5%8A%A1%E4%B8%AD%E5%BF%83","key5":"false","key2":"7~0~0~~1~1~0~0~2~0~1~0~d35a1391-81fe-449c-8c56-c75926a1d80f@@92320115MA1TAWE49D@@00","key1":"03"})
    //返回的key1是返回结果,正常是03，异常是其他值
    if ((nRet = GetConfirmLoginFaileReason(hi)) < 0) {
        return nRet;
    }
    strcpy(szAuthCode, "key2");
    if (LoadJsonFromConfirmReponse(hi->bufHttpRep, szAuthCode, sizeof(szAuthCode)) < 0)
        return ERR_TA_CONFIRM_LOGIN;
    strcpy(ctx->szToken, szAuthCode);
    // settoken to cookies
    sprintf(szTotalSendBuf, "token=%s", ctx->szToken);
    char *pEscape = StrReplace(szTotalSendBuf, "~", "%7E");
    AppendCookies(hi, pEscape);
    free(pEscape);
    return RET_SUCCESS;
}

//注销会话，不调用关闭问题也不大，相当于直接关闭浏览器
int ConfirmLogout(HHTTP hi)
{
    int nRet = ERR_GENERIC;
    char szBuf[128] = "";
    CtxConfirm ctx = NULL;
    do {
        if (!hi->ctxApp) {
            nRet = RET_SUCCESS;
            break;
        }
        ctx = (CtxConfirm)hi->ctxApp;
        if ((nRet = ConfirmPreLogin(hi, ETAX_LOGOUT, szBuf, sizeof(szBuf))) < 0)
            break;
        if (strcmp(szBuf, "200")) {
            nRet = ERR_TA_REPONSE_FAILE;
            break;
        }
        nRet = RET_SUCCESS;
    } while (false);
    if (ctx)
        free(ctx);
    return nRet;
}

//尚未完善，需蒙柏方需求确认后根据业务内容自行完善
void SplitConfirmSupportItem(char *szBuf)
{
    //变量e为token，此处应该根据token的分割值进行菜单选项是否启用的判断
    //只有启用的菜单才能被调用，否则会报禁止调用的错误
    //打印所有功能
    int i;
    int num = 0;
    char *ptrItems[100] = {0};
    char szBufItem[256] = "";
    struct evbuffer *buf = evbuffer_new();
    //分割
    GetSplitSubString(szBuf, "<li", ptrItems, &num);
    //获取结果并释放
    for (i = 0; i < num; i++) {
        if (strstr(ptrItems[i], " style=\"display: none"))
            goto FreeItem;  //不显示，此功能不开启
        if (!strstr(ptrItems[i], "onclick=\"loadUrl")) {
            goto FreeItem;  //如果无click则非点击菜单
        }
        memset(szBufItem, 0, sizeof(szBufItem));
        evbuffer_drain(buf, evbuffer_get_length(buf));
        evbuffer_add_printf(buf, "%s", ptrItems[i]);
        if (FindStringFromEvBuffer(buf, "');\">", "</a>", szBufItem, sizeof(szBufItem)) < 0)
            goto FreeItem;
        printf("%s\n", szBufItem);
    FreeItem:
        free(ptrItems[i]);
    }
    evbuffer_free(buf);
}

int GetConfirmSupportType(HHTTP hi)
{
    char szBuf[128000] = "";
    char szQueryMain[64] = "";
    int nRet = ERR_GENERIC;
    CtxConfirm ctx = (CtxConfirm)hi->ctxApp;
    do {
        sprintf(szQueryMain, "/main.7e6b909e.html?_%s", ctx->szVersion);
        if (HTTPGetEvent(hi, szQueryMain) < 0)
            break;
        if (FindStringFromEvBuffer(
                hi->bufHttpRep, "<ul><li><a href=javascript:void(0); name=menu_sy",
                "<div id=footer><div class=footercont>", szBuf, sizeof(szBuf)) < 0)
            break;
        _WriteHexToDebugFile("h.txt", (unsigned char *)szBuf, strlen(szBuf));
        SplitConfirmSupportItem(szBuf);

        nRet = RET_SUCCESS;
    } while (false);
    return nRet;
}

int ConfirmLogin(HHTTP hi)
{
    int nRet = 0;
    // getversion,动态获取版本
    if ((nRet = GetLastestVersion(hi)) < 0)
        return nRet;
    //获取server时间辍(可选) ETAX_QUERYMM
    if ((nRet = GetConfirmToken(hi)) < 0)
        return nRet;
    //登陆成功前，判断是否有未确认的撤销发票提示(可选) add 20200715
    //获取主页 main.7e6b909e.html?_=" + ymbb;
    // char szQueryMain[64] = "";
    // sprintf(szQueryMain, "/main.7e6b909e.html?_%s", hi->ctxApp.szVersion);
    // if (HTTPGetEvent(hi, szQueryMain) < 0)
    //     return ERR_TA_REPONSE_FAILE;

    // hi->ctxApp.bLoginOK = true;
    return RET_SUCCESS;
}

//------------------------------------------如上为内部函数，如下为逻辑业务函数-----------------------------------------------
//只有return值>0，返回结果retBuf才有意义，返回值为返回缓冲长度和缓冲区指针


//有cokies会话超时时间，不可长时间开启
int CreateConfirmTalk(HHTTP hi, HDEV hDev)
{
    int nRet = ERR_TA_CONFIRM_LOGIN, nChildRet = 0;
    memset(hi, 0, sizeof(struct HTTP));
    do {
        if (GetTaServerURL(TAADDR_CONFIRM, hDev->szRegCode, &hi->modelTaAddr) != RET_SUCCESS) {
            _WriteLog(LL_WARN, "Get server url failed");
            nRet = ERR_TA_GET_SERVERADDR;
            break;
        }
        if ((nChildRet = HTTPInit(hi, hDev)) || (nChildRet = HTTPOpen(hi))) {
            _WriteLog(LL_WARN, "HTTPS Init/Open failed, nRet:%d", nChildRet);
            nRet = ERR_TA_DATA_ERROR;
            break;
        }
        if (!(hi->ctxApp = calloc(1, sizeof(struct ContextHttpAppConfirm))))
            break;
        hi->cbHttpClose = ConfirmLogout;
        if ((nChildRet = ConfirmLogin(hi)) < 0) {
            _WriteLog(LL_WARN, "ConfirmAuth failed, nRet:%d", nChildRet);
            break;
        }

        _WriteLog(LL_INFO, "Confirm platform connection OK");
        nRet = RET_SUCCESS;
    } while (false);
    return nRet;
}