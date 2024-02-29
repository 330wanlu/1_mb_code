/*****************************************************************************
File name:   etax_auth_server
Description: 用于浏览器访问上海税务局时本地28000端口认证，替代chinatax.exe
Author:      Zako
Version:     1.0
Date:        2023.01
History:
20230202     最初代码实现
*****************************************************************************/
#include "etax_confirm.h"

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