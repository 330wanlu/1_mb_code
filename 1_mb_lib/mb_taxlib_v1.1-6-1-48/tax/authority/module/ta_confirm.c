/*****************************************************************************
File name:   ta_confirm.c
Description: 用于税局发票综合服务平台\发票确认平台连接及操作；
Author:      Zako
Version:     1.0
Date:        2021.11
History:
20211114     最初代码实现
20221002     针对部分省份(上海等)逐渐弃用确认平台，我方代码逐渐收缩确认平台功能，
             将确认平台代码模块化，暂时不再纳入主干代码进行维护
*****************************************************************************/
#include "ta_confirm.h"

//-----------------------------------------发票综合平台直接代码连接逻辑-----------------------------------------
enum ConfirmOpeartePrivate {
    // 1-999内部API，禁止调用
    OPCONFIRM_HELLO = 0,
    OPCONFIRM_QUERYMM,
    OPCONFIRM_LOGIN,
    OPCONFIRM_LOGOUT,
    OPCONFIRM_SPLIT_LINE = 999
};


//获取cookies子节点变量
int GetCookiesSegment(struct evbuffer *evBufCookies, char *szQueryKey, char *szOutValue,
                      int nOutBufSize)
{
    // if (FindStringFromEvBuffer(hi->ctxCon.bufCookies, "dzfp-ssotoken=", ";", szSsoToken,
    //                            sizeof(szSsoToken)))
    //     break;
    if (!evbuffer_get_length(evBufCookies))
        return ERR_GENERIC;
    int nRet = ERR_GENERIC;
    char *p = NULL;
    struct evkeyvalq cookiesPairs;
    TAILQ_INIT(&cookiesPairs);
    SplitInfo spOut;
    memset(&spOut, 0, sizeof(SplitInfo));
    GetSplitString((char *)evbuffer_pullup(evBufCookies, -1), "; ", &spOut);
    SplitInfo *pOut = &spOut;
    int i;
    for (i = 0; i < spOut.num; i++) {
        pOut = pOut->next;
        // do
        if (!strchr(pOut->info, '='))
            continue;
        char *pChild = strdup(pOut->info);
        if (!pChild)
            continue;
        p = strchr(pChild, '=');
        *p = '\0';
        if (strcmp(pChild, szQueryKey)) {
            free(pChild);
            continue;
        }
        if (strlen(p + 1) + 1 > nOutBufSize) {
            free(pChild);
            break;
        }
        strcpy(szOutValue, p + 1);
        free(pChild);
        nRet = RET_SUCCESS;
        break;
    }
    SplitFree(&spOut);
    return nRet;
}


uint64 GetMicroSecondUnixTimestamp()
{
    struct timeval start;
    gettimeofday(&start, NULL);
    uint64 timeStamp = 1000 * start.tv_sec + start.tv_usec / 1000;
    return timeStamp;
}

// szOutBuf 空间必须足够大
void GetJQueryRandomCallback(char *szOutBuf, char *szQueryPath)
{
    char szRandom[24] = "";
    GetRandom(szRandom, 20, 10, NULL);
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

int LoadJsonFromReponse(struct evbuffer *bufHttpRep, char *szInOut, int nInBufSize)
{
    int nRet = ERR_BUF_CHECK;
    int nChildRet = 0;
    do {
        const char *pContent = (const char *)evbuffer_pullup(bufHttpRep, -1);
        cJSON *root = cJSON_Parse(pContent);  // pContent末尾'}'后面可能未\0断开,不影响解析
        if (!root)
            break;
        if (!cJSON_GetArraySize(root)) {
            break;
        }
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
    return nRet;
}

int GetServerJson(HHTTP hi, char *szPost, char *szQueryPath)
{
    int nChildRet = 0;
    int nRet = 0;
    char szQueryCallbackPath[512] = "";
    do {
        GetJQueryRandomCallback(szQueryCallbackPath, szQueryPath);
        nChildRet = HTTPPostEvent(hi, szQueryCallbackPath, (unsigned char *)szPost, strlen(szPost));
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
    do {
        switch (opFlag) {
            case OPCONFIRM_HELLO:
                if (GetTaClientHello(0, hi->hDev, szBuf1) < 0) {
                    nRet = ERR_TA_CLIENTHELLO;
                    break;
                }
                // data "type=CLIENT-HELLO&clientHello=3071020103305831...20402&alg=1&ymbb=4.0.16";
                sprintf(szPost, "type=CLIENT-HELLO&clientHello=%s&alg=1&ymbb=%s", szBuf1,
                        hi->ctxCfmReq.szVersion);
                strcpy(szQueryPath, "/NSbsqWW/login.do");
                strcpy(szOutServerPacket, "key2");
                break;
            case OPCONFIRM_QUERYMM:
                sprintf(szPost, "cert=%s&funType=01", hi->hDev->szCommonTaxID);
                strcpy(szQueryPath, "/NSbsqWW/querymm.do");
                strcpy(szOutServerPacket, "ts");
                break;
            case OPCONFIRM_LOGOUT:
                sprintf(szPost, "cert=%s&token=%s", hi->hDev->szCommonTaxID, hi->ctxCfmReq.szToken);
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
        if (LoadJsonFromReponse(hi->bufHttpRep, szOutServerPacket, nBufSize) < 0)
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
        // hi->ctxCfmReq.szVersion,
        //                            sizeof(hi->ctxCfmReq.szVersion)))
        //     break;
        // if (strlen(hi->ctxCfmReq.szVersion) < 4 || strlen(hi->ctxCfmReq.szVersion) > 16)
        //     break;

        // 20221004测试，var ymbb = getYmbb(),版本号已由静态值变为动态获取，由js获取
        // 暂不深入跟进，待需要时再深入研究。测试调试时可以直接填入固定值
        strcpy(hi->ctxCfmReq.szVersion, "4.0.24");
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
    if (evbuffer_get_length(hi->bufCookies) != 0)
        evbuffer_add(hi->bufCookies, ";", 1);
    evbuffer_add(hi->bufCookies, szNewCookie, strlen(szNewCookie));
}

int GetConfirmLoginFaileReason(HHTTP hi)
{
    int nRet = ERR_TA_CONFIRM_LOGIN;
    char szBuf[512] = "";
    strcpy(szBuf, "key1");
    do {
        if (LoadJsonFromReponse(hi->bufHttpRep, szBuf, sizeof(szBuf)) < 0) {
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
    //获取server随机数
    if ((nRet = ConfirmPreLogin(hi, OPCONFIRM_HELLO, szRandomNum, sizeof(szRandomNum))) < 0)
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
             szAuthCode, szRandomNum, hDev->szCommonTaxID, hi->ctxCfmReq.szVersion, szFormatDevID,
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
    if (LoadJsonFromReponse(hi->bufHttpRep, szAuthCode, sizeof(szAuthCode)) < 0)
        return ERR_TA_CONFIRM_LOGIN;
    strcpy(hi->ctxCfmReq.szToken, szAuthCode);
    // settoken to cookies
    sprintf(szTotalSendBuf, "token=%s", hi->ctxCfmReq.szToken);
    char *pEscape = StrReplace(szTotalSendBuf, "~", "%7E");
    AppendCookies(hi, pEscape);
    free(pEscape);
    return RET_SUCCESS;
}

//注销会话，不调用关闭问题也不大，相当于直接关闭浏览器
int ConfirmLogout(HHTTP hi)
{
    int nRet = 0;
    char szBuf[128] = "";
    if ((nRet = ConfirmPreLogin(hi, OPCONFIRM_LOGOUT, szBuf, sizeof(szBuf))) < 0)
        return nRet;
    if (strcmp(szBuf, "200"))
        return ERR_TA_REPONSE_FAILE;
    return RET_SUCCESS;
}

//尚未完善，需蒙柏方需求确认后根据业务内容自行完善
void SplitConfirmSupportItem(char *szBuf)
{
    //变量e为token，此处应该根据token的分割值进行菜单选项是否启用的判断
    //只有启用的菜单才能被调用，否则会报禁止调用的错误
    /*
    var n = e.split('~');
    if ("2" == n[1] && $("[name='menu_dbtsgx']").hide(),
        "0" != n[1] && "1" != n[1] ||
            ($("[name='menu_tsgx']").hide(), $("[name='menu_dbtsgx']").hide()),
        "1" == n[5] && "1" == n[8]
            ? $("[name='group_dk']").hide()
            : "1" == n[5] && "2" == n[8] && "3" != n[1] &&
                  ($("[name='menu_sy']").hide(), $("[name='menu_dkgx']").hide(),
                   $("[name='menu_tsgx']").hide(), $("[name='menu_dbtsgx']").hide(),
                   $("[name='menu_pzcx']").hide(), $("[name='menu_swsxtzs']").hide(),
                   $("[name='menu_daxx']").show()),
        "1" == n[2]) {
        $("[name='jxqy_menu']").show(), $("[name='scqy_menu']").hide(),
            $("[name='qcslr_menu']").show(), $("[name='hggjxz_jxqy_menu']").show(),
            $("[name='fpgjxz_menu']").show(), $("[name='mxcx_menu']").show(),
            $("[name='cpyhsbz_menu']").show();
        var a = !1;
        "1" == n[5] && "2" == n[8] && (a = !0),
            a || ("1" == n[6] && ($("[name='fpgjxz_menu']").hide(), $("[name='ypdb_menu']").show(),
                                  $("[name='ypdbcx_menu']").show()),
                  "1" == n[7] && $("[name='yclykz_menu']").show());
    } else
        "2" == n[2] && ($("[name='jxqy_menu']").hide(), $("[name='scqy_menu']").show(),
                        $("[name='xfsgzt_menu']").show(), $("[name='hggjxz_scqy_menu']").show(),
                        $("[name='cpyfprz_menu']").show(), $("[name='cpyrztj_menu']").show(),
                        $("[name='cpyhsbz_menu']").show());
    "1" == n[11] ? $("[name='sxedtzsq']").show() : $("[name='sxedtzsq']").hide(),
        "N" == JKDZJKG ? $("#jkdzjcd").hide() : $("#jkdzjcd").show(),
        "Y" == YCPZSHOW ? $("#jxszctj").show() : $("#jxszctj").hide();
*/
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
    do {
        sprintf(szQueryMain, "/main.7e6b909e.html?_%s", hi->ctxCfmReq.szVersion);
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
    //获取server时间辍(可选) OPCONFIRM_QUERYMM
    if ((nRet = GetConfirmToken(hi)) < 0)
        return nRet;
    //登陆成功前，判断是否有未确认的撤销发票提示(可选) add 20200715
    //获取主页 main.7e6b909e.html?_=" + ymbb;
    // char szQueryMain[64] = "";
    // sprintf(szQueryMain, "/main.7e6b909e.html?_%s", hi->ctxCfmReq.szVersion);
    // if (HTTPGetEvent(hi, szQueryMain) < 0)
    //     return ERR_TA_REPONSE_FAILE;
    hi->ctxCfmReq.bLoginOK = true;
    return 0;
}

//------------------------------------------如上为内部函数，如下为逻辑业务函数-----------------------------------------------
//只有return值>0，返回结果retBuf才有意义，返回值为返回缓冲长度和缓冲区指针
int ConfirmAPI(ConfirmFlag opFlag, HHTTP hi, char **retBuf, const char *msg, ...)
{
    char szPost[68000] = "", szQueryPath[512] = "", szQueryParam[64000] = "";
    va_list val = {0};
    va_start(val, msg);
    vsnprintf(szQueryParam, sizeof(szQueryParam) - 1, msg, val);
    va_end(val);
    int nRet = ERR_PARM_CHECK;

    if ((int)opFlag <= OPCONFIRM_SPLIT_LINE) {
        //该方法接口不支持内部API调用
        _WriteLog(LL_FATAL, "Deny inner call flag");
        return ERR_PROGRAME_UNSUPPORT;
    }
    // https://tool.chinaz.com/tools/urlencode.aspx
    switch (opFlag) {
        case OPCONFIRM_SWSXTZS:
            // id=queryswsxtzs&cert=...&token=...&tzshm=&lrrqq=2021-12-01&lrrqz=2021-12-30&aoData=...&ymbb=4.0.17
            sprintf(szPost,
                    "id=queryswsxtzs&cert=%s&token=%s%s&aoData=[{\"name\":\"sEcho\","
                    "\"value\":1},{\"name\":\"iColumns\",\"value\":4},{\"name\":\"sColumns\","
                    "\"value\":\",,,\"},{\"name\":\"iDisplayStart\",\"value\":0},{\"name\":"
                    "\"iDisplayLength\",\"value\":50},{\"name\":\"mDataProp_0\",\"value\":0},{"
                    "\"name\":\"mDataProp_1\",\"value\":1},{\"name\":\"mDataProp_2\",\"value\":"
                    "2},{"
                    "\"name\":\"mDataProp_3\",\"value\":3}]&ymbb=%s",
                    hi->hDev->szCommonTaxID, hi->ctxCfmReq.szToken, szQueryParam,
                    hi->ctxCfmReq.szVersion);
            strcpy(szQueryPath, "/NSbsqWW/swsxtzs.do");
            break;
        case OPCONFIRM_FPXZ:
            // id=cxqq&cert=...&token=...&fplx=0&sjlx=2&sqrqq=2021-11-01&sqrqz=2021-12-30&clzt=-1&aoData=...&ymbb=4.0.17
            sprintf(szPost,
                    "id=cxqq&cert=%s&token=%s%s&aoData=[{\"name\":\"sEcho\",\"value\":1},{"
                    "\"name\":\"iColumns\",\"value\":10},{\"name\":\"sColumns\",\"value\":"
                    "\",,,,,,,"
                    ",,\"},{\"name\":\"iDisplayStart\",\"value\":0},{\"name\":"
                    "\"iDisplayLength\","
                    "\"value\":50},{\"name\":\"mDataProp_0\",\"value\":0},{\"name\":"
                    "\"mDataProp_"
                    "1\",\"value\":1},{\"name\":\"mDataProp_2\",\"value\":2},{\"name\":"
                    "\"mDataProp_"
                    "3\",\"value\":3},{\"name\":\"mDataProp_4\",\"value\":4},{\"name\":"
                    "\"mDataProp_"
                    "5\",\"value\":5},{\"name\":\"mDataProp_6\",\"value\":6},{\"name\":"
                    "\"mDataProp_"
                    "7\",\"value\":7},{\"name\":\"mDataProp_8\",\"value\":8},{\"name\":"
                    "\"mDataProp_"
                    "9\",\"value\":9}]&ymbb=%s",
                    hi->hDev->szCommonTaxID, hi->ctxCfmReq.szToken, szQueryParam,
                    hi->ctxCfmReq.szVersion);
            strcpy(szQueryPath, "/NFpxzWW/fpxz.do");
            break;
        case OPCONFIRM_HZFP:
            sprintf(szPost,
                    "id=hzfpcx&cert=%s&token=%s%s&aoData=[{\"name\":\"sEcho\",\"value\":1},{"
                    "\"name\":"
                    "\"iColumns\",\"value\":11},{\"name\":\"sColumns\",\"value\":\",,,,,,,,,,"
                    "\"},{"
                    "\"name\":\"iDisplayStart\",\"value\":0},{\"name\":\"iDisplayLength\","
                    "\"value\":50},{\"name\":\"mDataProp_0\",\"value\":0},{\"name\":"
                    "\"mDataProp_"
                    "1\",\"value\":1},{\"name\":\"mDataProp_2\",\"value\":2},{\"name\":"
                    "\"mDataProp_"
                    "3\",\"value\":3},{\"name\":\"mDataProp_4\",\"value\":4},{\"name\":"
                    "\"mDataProp_"
                    "5\",\"value\":5},{\"name\":\"mDataProp_6\",\"value\":6},{\"name\":"
                    "\"mDataProp_"
                    "7\",\"value\":7},{\"name\":\"mDataProp_8\",\"value\":8},{\"name\":"
                    "\"mDataProp_"
                    "9\",\"value\":9},{\"name\":\"mDataProp_10\",\"value\":10}]&ymbb=%s",
                    hi->hDev->szCommonTaxID, hi->ctxCfmReq.szToken, szQueryParam,
                    hi->ctxCfmReq.szVersion);
            strcpy(szQueryPath, "/NSbsqWW/hzfp.do");
            break;
        default:
            return ERR_PROGRAME_UNSUPPORT;
            break;
    }
    char *pUriEncodeBuf = NULL, *p1 = NULL, *p2 = NULL;
    do {
        if (!(pUriEncodeBuf = UriEncode(szPost)))
            break;
        p1 = StrReplace(pUriEncodeBuf, "%3D", "=");
        p2 = StrReplace(p1, "%26", "&");
        if (GetServerJson(hi, p2, szQueryPath) < 0)
            break;
        // OK,设置返回值
        nRet = evbuffer_get_length(hi->bufHttpRep);
        *retBuf = (char *)evbuffer_pullup(hi->bufHttpRep, -1);
    } while (false);
    if (pUriEncodeBuf)
        free(pUriEncodeBuf);
    if (p1) {
        free(p1);
        free(p2);
    }
//临时代码，不确定是否作用永久代码
#ifdef _CONSOLE
    char szKey1Ret[512] = "", szResult[512] = "";
    strcpy(szKey1Ret, "key1");
    if (LoadJsonFromReponse(hi->bufHttpRep, szKey1Ret, sizeof(szKey1Ret)) == RET_SUCCESS &&
        strcmp(szKey1Ret, "200")) {
        //错误时打印
        strcpy(szResult, "key2");
        LoadJsonFromReponse(hi->bufHttpRep, szResult, sizeof(szResult));
        _WriteLog(LL_FATAL, "ConfirmAPI failed, result:[%s:%s]", szKey1Ret, szResult);
    } else {
        _WriteLog(LL_DEBUG, "ConfirmAPI successful result:%s",
                  (const char *)evbuffer_pullup(hi->bufHttpRep, -1));
    }
#endif
    return nRet;
}

//有cokies会话超时时间，不可长时间开启
int CreateConfirmTalk(HHTTP hi, HDEV hDev)
{
    int nRet = -4;
    int nRepBufSize = 512000;
    memset(hi, 0, sizeof(struct HTTP));
    if (GetTaServerURL(TAADDR_CONFIRM, hDev->szRegCode, &hi->modelTaAddr) != RET_SUCCESS) {
        _WriteLog(LL_WARN, "GetUPServerURL failed");
        return -1;
    }
#ifdef FORCE_OTHER_PROVINCES  //外省连接检测
    // ForceOtherProvinces(TAADDR_CONFIRM, hDev->szRegCode, &hi->modelTaAddr);
#endif
    _WriteLog(LL_INFO, "Get taxauthority webURL OK, value:【%s %s】", hi->modelTaAddr.szRegName,
              hi->modelTaAddr.szTaxAuthorityURL);
    if (strncmp(hi->modelTaAddr.szTaxAuthorityURL, "https://", 8)) {
        _WriteLog(LL_WARN, "Upload by http don't support");
        return -2;
    }
    char *response = calloc(1, nRepBufSize);
    if (!response)
        return -3;
    int nChildRet = 0;
    while (1) {
        if ((nChildRet = HTTPInit(hi, hDev)) < 0) {
            _WriteLog(LL_WARN, "HTTPS Init failed, nRet:%d", nChildRet);
            nRet = -100;
            break;
        }
        if ((nChildRet = HTTPOpen(hi)) < 0) {
            _WriteLog(LL_WARN, "HTTPOpen failed, nRet:%d", nChildRet);
            nRet = -200;
            break;
        }
        if ((nChildRet = ConfirmLogin(hi)) < 0) {
            _WriteLog(LL_WARN, "ConfirmAuth failed, nRet:%d", nChildRet);
            nRet = -200;
            break;
        }
        _WriteLog(LL_INFO, "Confirm platform connection OK");
        nRet = 0;
        break;
    }
    free(response);
    return nRet;
}

//-----------------------------------------上海新电子税务局系统----转发连接28000端口认证逻辑-----------------------------------------
void BuildClientHelloEvb(HDEV hDev, char *szPostPayload, struct evbuffer *evbRep)
{
    bool bOK = false;
    char szHelloString[1024] = "";
    do {
        if (GetTaClientHello(1, hDev, szHelloString))
            break;
        if (strstr(szPostPayload, "strContainer=%2F%2FSM2%2FSM2CONTAINER0002") &&
            hDev->bDeviceType == DEVICE_CNTAX) {
            // cntax
            _WriteLog(LL_DEBUG, "Cntax device incoming auth");
        } else if (strstr(szPostPayload, "strContainer=CTAS0002++++++++") &&
                   (hDev->bDeviceType == DEVICE_AISINO || hDev->bDeviceType == DEVICE_NISEC)) {
            // aisino|nisec
            _WriteLog(LL_DEBUG, "aisino|nisec device incoming auth");
        } else
            break;
        evbuffer_add_printf(evbRep,
                            "<script>parent.postMessage('{\"code\":0,\"msg\":\"生成客户端认证请求--"
                            "双项认证方式成功\",\"clientHello\":\"%s\"}','*');</script>",
                            szHelloString);
        bOK = true;
    } while (false);
    if (!bOK)
        evbuffer_add_printf(evbRep, "<script>parent.postMessage('{\"code\":167,\"msg\":"
                                    "\"打开设备失败-未插USBKEY(0xA7)\"}','*');</script>");
}

void BuildClientAuthEvb(HDEV hDev, char *szPostPayload, struct evbuffer *evbRep)
{
    bool bOK = false;
    uint8 szBuf[4096] = "";
    do {
        int nLen = sizeof(szBuf);
        if (BuildClientAuthCode((uint8 *)szPostPayload, strlen(szPostPayload), szBuf, &nLen, hDev))
            break;
        // aisino|nisec
        evbuffer_add_printf(evbRep,
                            "<script>parent.postMessage('{\"code\":0,\"msg\":"
                            "\"生成客户端认证凭据成功\",\"clientAuth\":\"%s\"}','*');</script>",
                            szBuf);
        bOK = true;
    } while (false);
    if (!bOK)
        evbuffer_add_printf(evbRep, "<script>parent.postMessage('{\"code\":60,\"msg\":"
                                    "\"生成客户端认证请求失败，验证口令失败--函数参数错误(0x3C)("
                                    "password)\"}','*');</script>");
}

void AuthServerDo(uint8 bQueryType, struct evhttp_request *req, void *arg)
{
    struct evbuffer *buf = evhttp_request_get_input_buffer(req);
    evbuffer_add(buf, "", 1);  // NUL-terminate the buffer
    char *payload = (char *)evbuffer_pullup(buf, -1);
    HDEV hDev = (HDEV)arg;
    struct evbuffer *evb = evbuffer_new();
    if (!evb)
        return;
    switch (bQueryType) {
        case 0:
            evbuffer_add_printf(evb, "<script>parent.postMessage('{\"code\":0,\"msg\":"
                                     "\"获取版本号成功\",\"version\":\"1.4\"}','*');</script>");
            break;
        case 1:
            BuildClientHelloEvb(arg, payload, evb);
            break;
        case 2: {
            char szSrvHello[4096] = "";  //至少3k
            if (FindStringFromEvBuffer(buf, "serverHello=", "&", szSrvHello, sizeof(szSrvHello)))
                break;
            BuildClientAuthEvb(arg, szSrvHello, evb);
            break;
        }
        case 3:
            evbuffer_add_printf(
                evb,
                "<script>parent.postMessage('{\"code\":0,\"msg\":\"获取证书信息成功\","
                "\"certInfo\":\"%s\"}','*');</script>",
                hDev->szCommonTaxID);
            _WriteLog(LL_INFO, "Send authServer final reponse, all local auth finish");
            break;
        default:
            break;
    }
    //将封装好的evbuffer 发送给客户端
    evhttp_send_reply(req, HTTP_OK, "OK", evb);
    evbuffer_free(evb);
}

void UrlAccessCallback(struct evhttp_request *req, void *arg)
{
    const char *uri = evhttp_request_get_uri(req);
    struct evhttp_uri *urlParse = NULL;
    bool bNeeddo = false;
    //判断此URI是否合法
    if (!(urlParse = evhttp_uri_parse(uri))) {
        evhttp_send_error(req, HTTP_BADREQUEST, 0);
        return;
    }
    evhttp_add_header(evhttp_request_get_output_headers(req), "Server", "CHINATAX_CRYPT/1.4");
    evhttp_add_header(evhttp_request_get_output_headers(req), "Content-Type",
                      "text/html;charset=UTF-8");
    evhttp_add_header(evhttp_request_get_output_headers(req), "Connection", "keep-alive");
    do {
        //这里只处理Post请求, Get请求，就直接return 200 OK
        if (evhttp_request_get_command(req) != EVHTTP_REQ_POST)
            break;
        if (!strcmp(uri, "/api/getVersion")) {
            AuthServerDo(0, req, arg);
        } else if (!strcmp(uri, "/api/clientHello")) {
            AuthServerDo(1, req, arg);
        } else if (!strcmp(uri, "/api/clientAuth")) {
            AuthServerDo(2, req, arg);
        } else if (!strcmp(uri, "/api/readCertInfo")) {
            AuthServerDo(3, req, arg);
        } else
            break;
        bNeeddo = true;
    } while (false);
    if (!bNeeddo)
        evhttp_send_reply(req, 200, "OK", NULL);
    if (urlParse)
        evhttp_uri_free(urlParse);
    // clear repbuf
    struct evbuffer *buf = evhttp_request_get_input_buffer(req);
    evbuffer_drain(buf, evbuffer_get_length(buf));
}

struct bufferevent *SslServerBevCallback(struct event_base *base, void *arg)
{
    struct bufferevent *r;
    SSL_CTX *ctx = (SSL_CTX *)arg;
    r = bufferevent_openssl_socket_new(base, -1, SSL_new(ctx), BUFFEREVENT_SSL_ACCEPTING,
                                       BEV_OPT_CLOSE_ON_FREE);
    return r;
}

int LoadSslServerCert(SSL_CTX *ctx)
{
    int nRet = ERR_GENERIC;
    BIO *certBio = NULL;
    X509 *certX509 = NULL;
    do {
        unsigned char server_pem[] =
            "-----BEGIN CERTIFICATE-----\n"
            "MIIEzTCCA7WgAwIBAgIGAgAAABANMA0GCSqGSIb3DQEBCwUAMBwxCzAJBgNVBAYT\n"
            "AkNOMQ0wCwYDVQQDDARUTEhMMB4XDTE5MDMwNjAwMDAwMFoXDTM5MDMwNjAwMDAw\n"
            "MFowLjELMAkGA1UEBhMCQ04xCzAJBgNVBAoMAllGMRIwEAYDVQQDDAkxMjcuMC4w\n"
            "LjEwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQC8eU2ghUVltVdD5kH3\n"
            "RoO0XT8iDe6aJo2Qmw5T+RxoShzvvdNTpYSIVwjMAjdNN1Vc07DqmMNIfOUUXHQd\n"
            "z8L2HdOkRGsSSkqlG5pQQCL8fJ0QeTGm2xbOn0PpaO0linxFfoTmrFl8Ifr+ez3u\n"
            "ZPnNCu3S2mye2XxFvwCL+hzG/5QFWFtxSxDO5K9cro4seQEeX7PfmEohoruPoj2F\n"
            "j26J103Pq5nP2Q7Lo7mIkEicytBjZLnEJT1nO00wzBsQGe1k8NieHp6u8PAOeqEb\n"
            "UQmbY7LDeDqT8FcrMusLVIOnd6SObz4S6DcOLS5pxS5jX8C6qgBpcsK8qo/2+ew3\n"
            "bDUVAgMBAAGjggIBMIIB/TARBglghkgBhvhCAQEEBAMCBkAwDgYDVR0PAQH/BAQD\n"
            "AgP4MAwGA1UdEwEB/wQCMAAwEwYDVR0lBAwwCgYIKwYBBQUHAwEwHwYDVR0jBBgw\n"
            "FoAUCMBBQk4jKF+vSyC+5JU0NogNSG4wHQYDVR0OBBYEFAwKrabjbFIj7G/TQV+N\n"
            "PzAdFoy6MCEGCSsGAQQBgjcUAgQUHhIAVwBlAGIAUwBlAHIAdgBlAHIwRAYJKoZI\n"
            "hvcNAQkPBDcwNTAOBggqhkiG9w0DAgICAIAwDgYIKoZIhvcNAwQCAgCAMAcGBSsO\n"
            "AwIHMAoGCCqGSIb3DQMHMA8GA1UdEQQIMAaHBH8AAAEwewYDVR0fBHQwcjBwoG6g\n"
            "bIZqbGRhcDovL1JTQTIwNDguc2VydmVyLmNvbToyMzg5L2NuPWNybDAwMDIsb3U9\n"
            "Y3JsMDAsb3U9Y3JsLGM9Y24/Y2VydGlmaWNhdGVSZXZvY2F0aW9uTGlzdCwqP2Jh\n"
            "c2U/Y249Y3JsMDAwMjAaBgorBgEEAalDZAUGBAwMClcwMDAwMTAwNTkwGgYKKwYB\n"
            "BAGpQ2QFCQQMDApXMDAwMDEwMDU5MBIGCisGAQQBqUNkAgEEBAwCMjkwEgYKKwYB\n"
            "BAGpQ2QCBAQEDAJDQTAeBghghkgBhvhDCQQSDBAwMDAwMDAwMDAwMDAwMjAxMA0G\n"
            "CSqGSIb3DQEBCwUAA4IBAQB850cOUVq/z0S6Q+JU9YY2xVvO07lugr9OZcj6zoOz\n"
            "TBBAOq9lZP36TLU1yqGPZVi0W0iG3WC8M/gxoB+La+5x0EpO5iTClK2vDxrY/J9v\n"
            "6ciqasg2NX6VayhZt6elWG8VFa3j4laI8Xgj5Ve5y7BjAe0e+nk43iFBQ5C/V+Xq\n"
            "6/6464kCX503br1PJr9DyRghdM3lA2s/KKKKNtB2hXaELmsJXL6UKU89czLJpUzV\n"
            "d6Jm8Y9sO9un1Oj5iyS1irC0aUHxOTlqXW3T/I0yqqPsM4g+52cva6RilZZxwyPr\n"
            "e1AKQIGN6kYh4DjRY5dWz6520fC/o/OqbsKTbF8jQNH+\n"
            "-----END CERTIFICATE-----";
        unsigned char server_key[] =
            "-----BEGIN RSA PRIVATE KEY-----\n"
            "MIIEpAIBAAKCAQEAvHlNoIVFZbVXQ+ZB90aDtF0/Ig3umiaNkJsOU/kcaEoc773T\n"
            "U6WEiFcIzAI3TTdVXNOw6pjDSHzlFFx0Hc/C9h3TpERrEkpKpRuaUEAi/HydEHkx\n"
            "ptsWzp9D6WjtJYp8RX6E5qxZfCH6/ns97mT5zQrt0tpsntl8Rb8Ai/ocxv+UBVhb\n"
            "cUsQzuSvXK6OLHkBHl+z35hKIaK7j6I9hY9uiddNz6uZz9kOy6O5iJBInMrQY2S5\n"
            "xCU9ZztNMMwbEBntZPDYnh6ervDwDnqhG1EJm2Oyw3g6k/BXKzLrC1SDp3ekjm8+\n"
            "Eug3Di0uacUuY1/AuqoAaXLCvKqP9vnsN2w1FQIDAQABAoIBAQChnFJYBm8PW4PM\n"
            "kTCXXDuq+RodlVy3ib9x88NBdiZJtuNBH8CJxS/zyM2eQXgyGfR0eIEHaJ5rFc1I\n"
            "vE5FEdXXPJQyEwjM0i9XjGex0I0ve5sieQl+TfcsjZyqFHLk8e7FmEtVkQvaSTjz\n"
            "5bGhLayViX2MhAcH3u2U0McjQABVEXFMUp/PQsZVuKr+ObIOTY3zeb4jmZjElcNl\n"
            "wukMOgwXXV1iXNoOdE55exyEi5+OFUguqA0uXh2fDrwwZyk4HQVeJxJxLCoH1cu5\n"
            "CZR6o4tMMmHLHW15olf6+sxwGR1xEd0bkfEKfZr/qskofQJeMmetp5eI319ZyLAX\n"
            "WaZ/cpmBAoGBAOUsV9hrdj74VPgDBPaxcdyn2NhdbYd1sn8Ii2TnW7p7upk8r32Z\n"
            "o8PsKuEKXB9t5/Sl32FsDFblSB9Qf3ZC2a/GFv6eJ8Mrwx1s0LAJGWVXbAYHMF1s\n"
            "TyfUrRgd9dib0Jiys18tEzntOma6Mz5Lpr6Q2um+VQN4EjCiR81W6bFtAoGBANKJ\n"
            "UVyoMfsuc0lndA3sVWw3XvZX4AfVCRwFnKMV+pgmodmC4nhTqYtQ13s3aS+uPEgD\n"
            "7evpnvDoMQvOGUAg/R9gOF20+BmmM8006Mzq95nX/q5eXk+FG+b9qPC090BrRCVc\n"
            "zd8F+01/97bjdM5ivSeN8Rp+L5XcyN7iXQiyJfFJAoGAIUeQNihc0oYjaKSfmudn\n"
            "voBNWBYxKqOn2RS3kV6bjngbP9d1MIsYIgJws59ag2HldjyENNSI8jF3iLya1eSd\n"
            "Ug41FqKG6au4ONStdXLM0k+gqztuBZMN553VyRPIuLzfzNzeeWrFSPsvKO3PNTOt\n"
            "Mi087Qart8Y+9fsK7EG7y/0CgYAvei3x4jAbP7dZ102RkjkVRUtR7DEmMwFx40YW\n"
            "dO9k/llDTsO5dMxoEVv2emFrEjSf3Nr7d1yTYRkmW1zQXGAYgj9TymqkhhlqbkMN\n"
            "avNZKx9QvRExh4fqSEuftH+JdGcSseHvKE8qg/Adux9tJ1EWqWi0m74pYned45gt\n"
            "TNdHmQKBgQCIuWyf2pX7BUh2l6m8DUs3fcw0ZiavsAzDFaAFueku+Xxzef8s+em/\n"
            "nvwcDcJ+cRpRiRd4ZRRS16HKok3yeqy3WEmq22Yn/Ln/IqHa+CHKV62RmKd8trEk\n"
            "12PTx53BcZ0W1UT8zWnFGyjNeFun57AzVp+nAb6Vdz3cQ9PvFC/oqA==\n"
            "-----END RSA PRIVATE KEY-----";
        certBio = BIO_new_mem_buf((void *)server_pem, strlen((char *)server_pem));
        certX509 = PEM_read_bio_X509(certBio, NULL, NULL, NULL);
        if (1 != SSL_CTX_use_certificate(ctx, certX509))
            break;
        BIO_free(certBio);

        certBio = BIO_new_mem_buf((void *)server_key, strlen((char *)server_key));
        if (1 != SSL_CTX_use_PrivateKey(ctx, PEM_read_bio_PrivateKey(certBio, NULL, NULL, NULL)))
            break;
        if (1 != SSL_CTX_check_private_key(ctx))
            break;
        nRet = RET_SUCCESS;
    } while (false);
    if (nRet)
        _WriteLog(LL_FATAL, "Load certificate failed, error:%s",
                  ERR_error_string(ERR_get_error(), NULL));
    if (certX509)
        X509_free(certX509);
    if (certBio)
        BIO_free(certBio);
    return nRet;
}

//该方法为阻塞调用;如果需要非阻塞调用，需要将base替换为主流程或者主线程的base，在主线程中阻塞
int CreateChinataxLocalAuthServer(HDEV hDev)
{
    // windows上taskkill掉chinatax.exe,启用netsh转发至设备监听
    //注意需要打开windows iphelp服务，允许windows防火墙和第三方自定义防火墙规则允许外连设备28000端口
    // clang-format off
    //
    // netsh interface portproxy add v4tov4 listenport=28000 listenaddress=0.0.0.0 connectport=28000 connectaddress=192.168.88.160
    // netstat -ano | findstr :28000
    // netsh interface portproxy show all
    // netsh interface portproxy delete v4tov4 listenport=28000 listenaddress=0.0.0.0
    //
    // clang-format on
    struct event_base *base = NULL;
    struct evhttp *http = NULL;
    SSL_CTX *ctx = NULL;
    do {
        if (!(base = event_base_new()))
            break;
        if (!(http = evhttp_new(base)))
            break;
        ctx = SSL_CTX_new(SSLv23_server_method());

        if (LoadSslServerCert(ctx))
            break;
        evhttp_set_bevcb(http, SslServerBevCallback, ctx);
        //默认回调
        evhttp_set_gencb(http, UrlAccessCallback, hDev);
        //专属uri路径回调
        //  evhttp_set_cb(http, "/api/getVersion", UrlAccessCallback, NULL);
        evhttp_set_timeout(http, 8);
        struct evhttp_bound_socket *handle = evhttp_bind_socket_with_handle(http, "0.0.0.0", 28000);
        if (!handle)
            break;
        event_base_dispatch(base);
    } while (false);
    if (http)
        evhttp_free(http);
    if (base)
        event_base_free(base);
    if (ctx)
        SSL_CTX_free(ctx);
    return RET_SUCCESS;
}