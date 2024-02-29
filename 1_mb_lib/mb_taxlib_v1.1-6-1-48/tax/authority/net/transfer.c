/*************************************************
File name:   transfer.c
Description: 用于处理航信、百旺等税盘与税务局交互的传输层协议封装
             包含TCP连接、SSL连接、HTTP和socks5代理
Author:      Zako
Version:     1.0
Date:        2019.12
History:
20201225     加入数个代理协议
20211018     部分接口调整，参数由Init初始化
20220627     为解决SSL连接部分与libevent不兼容，大幅度精简我方代码，由libevent实现SSL连接
*************************************************/
// SSL envirnment = openssl 1.0.2k
#include "transfer.h"
#include <openssl/ossl_typ.h>
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"  // ignore 'TLSv1_1_method' deprecated

//注意这里的返回值，不能随意返回，正确返回1， 错误返回0
int TA_Pkcs11_Aisino(int type, const uint8 *m, unsigned int m_length, uint8 *sigret,
                     unsigned int *siglen, const RSA *rsa)
{
    if (type > 0) {
        // null,这里是回调函数，不能去除type变量，用空函数消除编译器警告
    }
    // m_length == 36
    if (m_length > 48 || m_length < 24)
        return 0;
    HDEV hDev = (HDEV)RSA_get_app_data(rsa);
    HUSB hUSB = hDev->hUSB;
    uint8 abDevSignIO_MD5SHA1_Model[136] = {
        0x00, 0x85, 0x00, 0x80, 0xB8, 0x50, 0x03, 0x80, 0x00, 0x01, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0x00, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11,
        0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11,
        0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11};
    // 0x11,为填充字符,将会被替换,一共填充替换36(0x24)个字节

    char szBuff[256] = "";
    //选择application，否则有的盘兼容性不好，没这段代码会签名错误
    if (AisinoCryptIO(hUSB, hDev->bCryptBigIO, (uint8 *)"\x00\x07\x00\x00\xa4\x00\x00\x02\x4f\x01",
                      10, (uint8 *)szBuff, sizeof(szBuff)) < 0)
        return 0;
    memset(szBuff, 0, sizeof(szBuff));
    memcpy(abDevSignIO_MD5SHA1_Model + sizeof(abDevSignIO_MD5SHA1_Model) - m_length, m, m_length);
    int nChildRet =
        AisinoCryptIO(hUSB, hDev->bCryptBigIO, (uint8 *)abDevSignIO_MD5SHA1_Model,
                      sizeof(abDevSignIO_MD5SHA1_Model), (uint8 *)szBuff, sizeof(szBuff));
    if (nChildRet != 133) {
        if (nChildRet == 5) {
            _WriteLog(LL_WARN, "Open cert failed, maybe cert password incorrect. We can not signed "
                               "hash\r\n");
        }
        return 0;
    }
    int nRetBuff = 0x80;
    memcpy(sigret, szBuff + 3, nRetBuff);
    memcpy(siglen, &nRetBuff, 4);
    return 1;
}

//注意这里的返回值，不能随意返回，正确返回1， 错误返回0
int TA_Pkcs11_Nisec(int type, const uint8 *m, unsigned int m_length, uint8 *sigret,
                    unsigned int *siglen, const RSA *rsa)
{
    if (type > 0) {
        // null,这里是回调函数，不能去除type变量，用空函数消除编译器警告
    }
    // m_length == 36
    if (m_length > 48 || m_length < 24) {
        _WriteLog(LL_WARN, "Issue:TA_Pkcs11_Nisec return -1");
        return 0;
    }
    HDEV hDev = (HDEV)RSA_get_app_data(rsa);
    HUSB hUSB = hDev->hUSB;
    // crypt_login已经登陆了,不需要重复
    unsigned char abDevSignIO_MD5SHA1_Model[128] = {
        0x00, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0x00, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11,
        0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11,
        0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11};
    // 0x11,为填充字符,将会被替换,一共填充替换36(0x24)个字节
    memcpy(abDevSignIO_MD5SHA1_Model + sizeof(abDevSignIO_MD5SHA1_Model) - m_length, m, m_length);
    // sigret must > 0x80
    if (Nisec_Crypt_Login(hUSB, hDev->szDeviceID) <= 0) {
        _WriteLog(LL_WARN, "Issue:TA_Pkcs11_Nisec return -2");
        return 0;
    }
    if (NisecCryptHash(hUSB, 2, abDevSignIO_MD5SHA1_Model, sizeof abDevSignIO_MD5SHA1_Model,
                       (char *)sigret, 256) < 0) {
        _WriteLog(LL_WARN, "Issue:TA_Pkcs11_Nisec return -3");
        return 0;
    }
    *siglen = 0x80;
    return 1;
}

// 回调路径xx->opensslxx->...NID_brainpoolP160t1(922)()?->
//->ssl3_finish_mac函数中BIO_write，多次回调压入ssl-handshake-buf，逐个长度为(80+fcd+4d+bf+4+58f+a2=178e=6030)=tbs_data
//->sm3(tbs_data)=hashOfTbsdata
//->sm3(zvalue(1234567812345678)[32Bytes]+hashOfTbsdata[32Bytes])=dgst
//->TA_Pkcs11_Cntax
//
// openssl1.1.1，客户端证书认证签名函数为tls_construct_cert_verify
//使用EC密钥对哈希进行签名，并返回DER编码的sig签名，其长度为siglen。成功时返回值为1，错误时返回值为0。
int TA_Pkcs11_Cntax(int type, const unsigned char *dgst, int dlen, unsigned char *sig,
                    unsigned int *siglen, const BIGNUM *kinv, const BIGNUM *r, EC_KEY *eckey)
{
    if (r || !kinv || !dgst || !dlen || !sig || !siglen) {
        // null,这里是回调函数，不能去除type变量，用空函数消除编译器警告
    }
    if (type <= 0 || !eckey || dlen != 32) {
        _WriteLog(LL_WARN, "Issue:TA_Pkcs11_Cntax return -1");
        return 0;
    }
    uint8 abSM3HashRet[64] = {0};
    uint8 abSM3Hash[32 + 1] = {0};
    abSM3Hash[0] = 0x81;  //首字节为0x81,后续20字节为sha1Hash SM3_DIGEST_LENGTH 32
    memcpy(abSM3Hash + 1, dgst, dlen);
    const EC_KEY_METHOD *ecmeth = EC_KEY_get_method(eckey);
    HDEV hDev = NULL;
    EC_KEY_METHOD_get_keygen(ecmeth, (void *)&hDev);
    if (!hDev) {
        _WriteLog(LL_WARN, "Hdev failed");
        return 0;
    }
    //不能使用CntaxGetDevice64ByteSignRepAsn1，因为这里是很纯粹的Hash签名算法，Z值计算什么的已经在之前的代码里面做过了
    if (CntaxCryptLogin(hDev->hUSB, hDev->szDeviceID) <= 0) {
        _WriteLog(LL_WARN, "Issue:TA_Pkcs11_Nisec return -2");
        return 0;
    }
    if (NisecCryptHash(hDev->hUSB, 3, abSM3Hash, sizeof(abSM3Hash), (char *)abSM3HashRet,
                       sizeof(abSM3HashRet)) < 0) {
        _WriteLog(LL_WARN, "Issue:TA_Pkcs11_Nisec return -3");
        return 0;
    }
    unsigned char abTaHandshake_Model[72] = {0};  //最长72
    *siglen =
        BuildCntaxPkcs11CallbackBuf(abSM3HashRet, abTaHandshake_Model, sizeof(abTaHandshake_Model));
    memcpy(sig, abTaHandshake_Model, *siglen);
    return 1;
}

// called when EC_KEY is destroyed
void EckeyFinish(EC_KEY *ec)
{
    const EC_KEY_METHOD *ec_meth = EC_KEY_get_method(ec);
    EC_KEY_METHOD_free((EC_KEY_METHOD *)ec_meth);
}

int RsaFinish(RSA *rsa)
{
    RSA_METHOD *rsameth = (RSA_METHOD *)RSA_get_method(rsa);
    RSA_meth_free(rsameth);
    return 1;
}

//云南cntax税局RSA算法连接测试 /openssl s_client -tls1_2 -connect 116.52.1.245:9006
//国密算法天安的s_client app有问题，可以参考PR解决 https://github.com/jntass/TASSL-1.1.1b/pull/37
//修改后的命令 ./openssl s_client -cipher ECC-SM4-SM3 -cert ../tassl_demo/cert/certs/CS.pem
//-cert_enc ../tassl_demo/cert/certs/CE.pem -CAfile capath -connect 116.52.1.245:9006
int InitSSL(HHTTP hi)
{
    HDEV hDev = hi->hDev;
    if (hDev->bTrainFPDiskType || !hDev->pX509Cert) {
        _WriteLog(LL_WARN, "Cannot get dev's cert. Maybe a train disk?");
        return -1;
    }
    //设置客户端使用的SSL版本
    SSL_METHOD *sslMethod = NULL;
    bool bYunnanCntax = false;
    //只能使用tls1.0不能使用更高的，否则部分省份兼容性不好
    if (hDev->bDeviceType == DEVICE_AISINO || hDev->bDeviceType == DEVICE_NISEC)
        sslMethod = (SSL_METHOD *)TLSv1_method();
    else if (hDev->bDeviceType == DEVICE_CNTAX) {
        if (strstr(hi->modelTaAddr.szTaxAuthorityURL, "yunnan.chinatax.gov.cn")) {
            //云南ssl连接，暂且使用RSA算法
            sslMethod = (SSL_METHOD *)TLSv1_method();
            bYunnanCntax = true;
        } else
            sslMethod = (SSL_METHOD *)CNTLS_client_method();
    }
    //创建SSL上下文环境 每个进程只需维护一个SSL_CTX结构体
    SSL_CTX *ctx = SSL_CTX_new(sslMethod);
    if (!ctx) {
        _WriteLog(LL_FATAL, "InitSSL new sslctx failed");
        return -2;
    }
    if (bYunnanCntax) {
        hi->ctx = ctx;
        return 0;
    }
    int nRet = -3;
    while (1) {
        // noticket参数不能加，否则部分省份兼容性不好(宁波)
        SSL_CTX_set_security_level(ctx, 0);
        if (!SSL_CTX_use_certificate(ctx, (X509 *)hDev->pX509Cert)) {
            _WriteLog(LL_FATAL, "InitSSL load certificate failed, error:%s",
                      ERR_error_string(ERR_get_error(), NULL));
            break;
        }
        //不是手动申请的，不需要手动调用EVP_PKEY_free释放pPubKey
        EVP_PKEY *pPubKey = X509_get_pubkey(hDev->pX509Cert);
        if (!pPubKey)
            break;
        if (hDev->bDeviceType == DEVICE_AISINO || hDev->bDeviceType == DEVICE_NISEC) {
            struct rsa_st *rsa = EVP_PKEY_get0_RSA(pPubKey);
            RSA_METHOD *rsameth = RSA_meth_dup(RSA_get_default_method());
            RSA_set_method(rsa, rsameth);
            RSA_meth_set_finish(rsameth, RsaFinish);
            if (DEVICE_AISINO == hDev->bDeviceType)
                RSA_meth_set_sign(rsameth, &TA_Pkcs11_Aisino);
            else if (DEVICE_NISEC == hDev->bDeviceType)
                RSA_meth_set_sign(rsameth, &TA_Pkcs11_Nisec);
            RSA_set_app_data(rsa, hDev);
            SSL_CTX_use_PrivateKey(ctx, pPubKey);
        } else if (hDev->bDeviceType == DEVICE_CNTAX) {
            //调试在/opt/tassl111bd/src/ssl/statem/statem_clnt.c 1037行左右的多个swtich标签里面
            SSL_CTX_set_options(ctx, SSL_OP_NO_ENCRYPT_THEN_MAC);
            //国密算法无需ticket
            SSL_CTX_set_options(ctx, SSL_OP_NO_TICKET);
            //不同的openssl，该名称可能字符串内容不一样，如果有兼容问题，可以不加，加上之后少握手时少发送好几十个字节
            SSL_CTX_set_cipher_list(ctx, "ECC-SM4-SM3");
            EC_KEY *ec = EC_KEY_dup(EVP_PKEY_get0_EC_KEY(pPubKey));
            EC_KEY_METHOD *ecmethNew = EC_KEY_METHOD_new(EC_KEY_OpenSSL());
            // EC_FLAG_TASS_CUSTOM_SIGN是江南天安自己定义的宏，在tassl底层会有判断，进入SM2自定义callback
            //具体位置在ECDSA_sign.c:ECDSA_sign
            EC_KEY_set_flags(ec, EC_KEY_get_flags(ec) | EC_FLAG_TASS_CUSTOM_SIGN);
            //找个地方"藏"hev，openssl1.11没有RSA的set0_app_data,替代方案EC_KEY_get_ex_new_index，暂且不用
            EC_KEY_METHOD_set_keygen(ecmethNew, (void *)hDev);
            //这个会触发EckeyFinish，因此需要在EC_KEY_METHOD_set_init之前调用
            EC_KEY_set_method(ec, ecmethNew);
            //设置回调free掉ecmethNew,Eckey好像是自动释放，暂且不管
            EC_KEY_METHOD_set_init(ecmethNew, NULL, EckeyFinish, NULL, NULL, NULL, NULL);
            EC_KEY_METHOD_set_sign(ecmethNew, TA_Pkcs11_Cntax, NULL, NULL);
            EVP_PKEY *privkey = NULL;
            privkey = EVP_PKEY_new();
            EVP_PKEY_assign_EC_KEY(privkey, ec);
            SSL_CTX_use_PrivateKey(ctx, privkey);
            EVP_PKEY_free(privkey);
        } else {
            _WriteLog(LL_WARN, "hDev->bDeviceType failed");
            break;
        }
        EVP_PKEY_free(pPubKey);
        //验证密钥是否与证书一致
        if (!SSL_CTX_check_private_key(ctx)) {
            _WriteLog(LL_WARN, "SSL_CTX_check_private_key failed");
            break;  // todo free, 这个一般不会触发，泄漏就不管了
        }
        nRet = 0;
        break;
    }
    if (nRet < 0)
        SSL_CTX_free(ctx);
    else
        hi->ctx = ctx;
    return nRet;
}

void FreeSSL(SSL_CTX *ctx, BIO *bio)
{
    //关闭连接
    if (bio) {
        //必须这么写，不然有警告
        if (BIO_reset(bio)) {
        }
        BIO_free_all(bio);
    }
    if (ctx)
        SSL_CTX_free(ctx);
}

// http://192.168.0.1:8080
// socks5://192.168.0.1:1080
bool SetProxy(HDEV hDev, char *szProxyStr)
{
    char szTunnelHost[32] = "";
    int nPort = 0;
    if (strlen(szProxyStr) > 32)
        return false;
    if (!strncmp(szProxyStr, "http://", 7))
        strcpy(szTunnelHost, szProxyStr + 7);
    else if (!strncmp(szProxyStr, "socks5://", 9))
        strcpy(szTunnelHost, szProxyStr + 9);
    else
        return false;
    if (!strchr(szTunnelHost, ':'))
        return false;
    ByteReverse(szTunnelHost, strlen(szTunnelHost));
    *strchr(szTunnelHost, ':') = '\0';
    ByteReverse(szTunnelHost, strlen(szTunnelHost));
    nPort = atoi(szTunnelHost);
    if (nPort >= 65535 || nPort <= 0)
        return false;
    strcpy(hDev->szProxyStr, szProxyStr);
    return true;
}

//------------------------------连接初始化及建立代码begin----------------------------------------------------------

// old:HTTPEventInit
int EventInit(HHTTP hi)
{
    int ret = ERR_PARM_INIT;
    char host[256] = "";
    int port = 0;
    do {
        //多线程支持，启用了之后会有内存泄露，编译libevent加上参数可以规避该问题，暂时不打开线程支持，需要时再启用
        //./configure --disable-debug-mode
        //参考
        // https://libevent-users.monkey.narkive.com/Qemtm7oP/libevent-memory-leak
        // https://blog.csdn.net/wwyyxx26/article/details/48315361
        if (!(hi->evBase = event_base_new()))
            break;
        if (!(hi->uri = evhttp_uri_parse(hi->modelTaAddr.szTaxAuthorityURL)))
            break;
        if (GetHostAndPortFromEvUri(hi->uri, host, &port) < 0)
            break;
#ifndef MIPS_MTK
        if (!(hi->evDns = evdns_base_new(hi->evBase, EVDNS_BASE_INITIALIZE_NAMESERVERS |
                                                         EVDNS_BASE_DISABLE_WHEN_INACTIVE)))
            break;
#else
        //考虑到三通机房的dns负载均衡，尽量使用设备自带的dns服务器
        // MIPS4口设备本地dns解析服务配置有问题，使用自定义第三方dns
        if (!(hi->evDns = evdns_base_new(hi->evBase, EVDNS_BASE_NAMESERVERS_NO_DEFAULT)))
            break;
        // 114dns
        evdns_base_nameserver_ip_add(hi->evDns, "114.114.114.114");
        evdns_base_nameserver_ip_add(hi->evDns, "114.114.115.115");
        //阿里云dns
        evdns_base_nameserver_ip_add(hi->evDns, "223.5.5.5");
        evdns_base_nameserver_ip_add(hi->evDns, "223.6.6.6");
#endif
        if (strcasecmp(evhttp_uri_get_scheme(hi->uri), "http") == 0)
            hi->evEvent = bufferevent_socket_new(hi->evBase, -1, BEV_OPT_CLOSE_ON_FREE);
        else {
            hi->ssl = SSL_new(hi->ctx);
            hi->evEvent = bufferevent_openssl_socket_new(
                hi->evBase, -1, hi->ssl, BUFFEREVENT_SSL_CONNECTING,
                BEV_OPT_CLOSE_ON_FREE | BEV_OPT_DEFER_CALLBACKS | BEV_OPT_THREADSAFE);
        }
        if (hi->evEvent == NULL)
            break;
        bufferevent_openssl_set_allow_dirty_shutdown(hi->evEvent, 1);
        //必须tcp层和libevent逻辑双重制约才可（实际TCP作用占绝大多数）
        int fd = bufferevent_getfd(hi->evEvent);
        SetSocketOption(fd, 20);
        ret = RET_SUCCESS;
    } while (false);
    return ret;
}

//++Connect Step3
int ConnectIP(HHTTP hi, char *szIP)
{
    char host[150] = "", szBuf[256] = "";
    int port, nRet = ERR_GENERIC;
    struct evhttp_request *req = NULL;
    //异步dns解析,但是不使用默认，默认的会查询IPv6，依赖系统配置，得修改系统配置才行
    do {
        GetHostAndPortFromEvUri(hi->uri, host, &port);  //初始化已检测过
        _WriteLog(LL_DEBUG, "Try connect TA, %s:%d", szIP, port);
        //此处使用异步dns查询出来的IP，而非域名进行连接
        if (!(hi->evConn = evhttp_connection_base_bufferevent_new(hi->evBase, NULL, hi->evEvent,
                                                                  szIP, port)))
            break;
        evhttp_connection_set_flags(hi->evConn, EVHTTP_CON_REUSE_CONNECTED_ADDR);
        //此处不能设置重传，否则高峰时期丢包率高会导致大量TCP层快速重传，容易被税局封IP
        evhttp_connection_set_retries(hi->evConn, 0);
        //
        //还有个evhttp_set_timeout函数，是如下函数的上层封装，再调用evhttp_set_timeout_tv，最终底层还是设置的evconn的超时
        //蒙柏接口超时时间为60s，底层老代码是15秒，新代码3秒略先嫌小，20220802改为30秒,20220813改为15
        //此处超时时间已经经过多次实际测试越大越稳定，但是又不能无限放大，不能超过蒙柏接口大小，原则上只增加不减小
        //此处超时原则上不再改动，否则会导致税务总局的红字信息表上传出现乱码异常
        //
        // 20220814
        //该时间仅为TCP发送FIN结束包的间隔时间，但是发送FIN包后设备TCP栈依旧发送Retry包，易导致税局封IP
        //需要结合TCP的setsocketoption双重制约才可
        evhttp_connection_set_timeout(hi->evConn, 20);
        // GetHttpReponseCallback为++Connect Step4
        if (!(req = evhttp_request_new(GetHttpReponseCallback, hi)))
            break;
        snprintf(szBuf, sizeof(szBuf) - 1, "%s:%d", host, port);
        evhttp_add_header(evhttp_request_get_output_headers(req), "Host", szBuf);
        strcpy(szBuf, "/AuthLogin.auth");
        if (TAADDR_CONFIRM == hi->modelTaAddr.bServType)
            strcpy(szBuf, "/");
        if (evhttp_make_request(hi->evConn, req, EVHTTP_REQ_GET, szBuf) < 0) {
            evhttp_request_free(req);
            break;
        }
        nRet = RET_SUCCESS;
    } while (false);
    return nRet;
}

//++Connect Step2
void DnsResolvCallback(int errcode, struct evutil_addrinfo *addr, void *ptr)
{
    struct evutil_addrinfo *ai = NULL;
    char buf[128];
    HHTTP hi = (HHTTP)ptr;
    do {
        if (errcode)
            break;  // dns 服务器查询失败
        for (ai = addr; ai; ai = ai->ai_next) {
            const char *s = NULL;
            memset(buf, 0, sizeof(buf));
            if (ai->ai_family == AF_INET) {
                struct sockaddr_in *sin = (struct sockaddr_in *)ai->ai_addr;
                s = evutil_inet_ntop(AF_INET, &sin->sin_addr, buf, 128);
            } else if (ai->ai_family == AF_INET6) {
                struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *)ai->ai_addr;
                s = evutil_inet_ntop(AF_INET6, &sin6->sin6_addr, buf, 128);
                continue;  //不需要IPv6
            }
            if (!s || strlen(s) < 7)
                continue;
            // dns OK
            if (ConnectIP(hi, (char *)s))
                continue;
            // TCP 3次握手ok，实际SSL通信等pendding
            break;
        }
    } while (false);
    if (addr)
        evutil_freeaddrinfo(addr);
}

void EtaxDnsResolvCallback(int errcode, struct evutil_addrinfo *addr, void *ptr)
{
    struct evutil_addrinfo *ai = NULL;
    char buf[128];
    EHHTTP hi = (EHHTTP)ptr;
    do {
        if (errcode)
            break;  // dns 服务器查询失败
        for (ai = addr; ai; ai = ai->ai_next) {
            const char *s = NULL;
            memset(buf, 0, sizeof(buf));
            if (ai->ai_family == AF_INET) {
                struct sockaddr_in *sin = (struct sockaddr_in *)ai->ai_addr;
                s = evutil_inet_ntop(AF_INET, &sin->sin_addr, buf, 128);
            } else if (ai->ai_family == AF_INET6) {
                struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *)ai->ai_addr;
                s = evutil_inet_ntop(AF_INET6, &sin6->sin6_addr, buf, 128);
                continue;  //不需要IPv6
            }
            if (!s || strlen(s) < 7)
                continue;
            // dns OK
            if (EtaxConnectIP(hi, (char *)s))
                continue;
            // TCP 3次握手ok，实际SSL通信等pendding
            break;
        }
    } while (false);
    if (addr)
        evutil_freeaddrinfo(addr);
}


//不能使用C程序或者libevent自带的Dns解析，这两个解析依赖系统配置，都会去查找resolv.conf
//默认会查询AAA IPv6记录，部分税局服务器IPv6配置有问题不响应，会导致程序dns查询延迟15秒左右
//两个解决办法1)禁用设备IPv6 2)使用自定义dns异步查询
//
//++Connect Step1
int EventConnect(HHTTP hi)
{
    char host[256] = "";
    int port, nRet = RET_SUCCESS;
    bool bIpOrDoamin = false;
    struct in_addr addr;

    GetHostAndPortFromEvUri(hi->uri, host, &port);
    bIpOrDoamin = evutil_inet_pton(AF_INET, host, &addr);
    if (bIpOrDoamin) {
        // IP无需解析，
        if (ConnectIP(hi, host))
            nRet = ERR_IO_SOCKET_FAIL;
        // TCP 3次握手ok，实际SSL通信等pendding
    } else {
        //除非指定socket类型，否则每个请求将会收到2个响应，一个TCP一个UDP，因此需要指定socket类型
        struct evutil_addrinfo hints;
        memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_TCP;
        evdns_base_set_option(hi->evDns, "attempts:", "4");   //最多尝试4个dns服务器
        evdns_base_set_option(hi->evDns, "timeout:", "0.5");  //每个dns最大0.5s超时
        evdns_base_set_option(hi->evDns, "max-timeouts:", "2");  //所有dns加起来最大查询时间2s
        //该函数返回值判断无意义
        evdns_getaddrinfo(hi->evDns, host, NULL, &hints, DnsResolvCallback, hi);
        nRet = RET_SUCCESS;  //异步OK
    }
    return nRet;
}

void PenddingBreakCheck(int fd, short event, void *param)
{
    HHTTP hi = (HHTTP)param;
    if (hi->hDev->bBreakAllIO) {
        event_base_loopbreak(hi->evBase);
        hi->hDev->bBreakAllIO = false;  //自动恢复
        _WriteLog(LL_INFO, "PenddingBreakCheck set pending break");
    }
}

void PenddingTimeoutCheck(int fd, short event, void *param)
{
    HHTTP hi = (HHTTP)param;
    _WriteLog(LL_WARN, "PenddingTimeoutCheck limit time 45s");
    event_base_loopbreak(hi->evBase);
}

void EvenPendingWithBreakSingal(HHTTP hi)
{
    // struct event event_on_stack,栈上分配总是编译报错，只能使用event_new动态分配
    struct timeval tv;
    evutil_timerclear(&tv);
    tv.tv_sec = 2;  // 2秒循环一次，IO中断检测
    if (!hi->evBreak)
        hi->evBreak = event_new(hi->evBase, -1, EV_PERSIST, PenddingBreakCheck, hi);
    evtimer_add(hi->evBreak, &tv);
    // IO总超时控制即整个函数API调用时pendding阻塞等待时间
    evutil_timerclear(&tv);
    tv.tv_sec = 45;  //上层最长60s等待，底层总IO时长不超过45s，否则视为超时
    if (!hi->evIoTimeout)
        hi->evIoTimeout = event_new(hi->evBase, -1, 0, PenddingTimeoutCheck, hi);
    evtimer_add(hi->evIoTimeout, &tv);
    // wait
    event_base_dispatch(hi->evBase);
}



int EtaxEventInit(EHHTTP hi)
{
    int ret = ERR_PARM_INIT;
    char host[256] = "";
    int port = 0;
    do {
        //多线程支持，启用了之后会有内存泄露，编译libevent加上参数可以规避该问题，暂时不打开线程支持，需要时再启用
        //./configure --disable-debug-mode
        //参考
        // https://libevent-users.monkey.narkive.com/Qemtm7oP/libevent-memory-leak
        // https://blog.csdn.net/wwyyxx26/article/details/48315361
        if (!(hi->ctxCon.evBase = event_base_new()))
            break;
        if (!(hi->ctxCon.uri = evhttp_uri_parse(hi->modelTaAddr.szTaxAuthorityURL)))
            break;
        if (GetHostAndPortFromEvUri(hi->ctxCon.uri, host, &port) < 0)
            break;
#ifndef MIPS_MTK
        if (!(hi->ctxCon.evDns =
                  evdns_base_new(hi->ctxCon.evBase, EVDNS_BASE_INITIALIZE_NAMESERVERS |
                                                        EVDNS_BASE_DISABLE_WHEN_INACTIVE)))
            break;
#else
        //考虑到三通机房的dns负载均衡，尽量使用设备自带的dns服务器
        // MIPS4口设备本地dns解析服务配置有问题，使用自定义第三方dns
        if (!(hi->evDns = evdns_base_new(hi->evBase, EVDNS_BASE_NAMESERVERS_NO_DEFAULT)))
            break;
        // 114dns
        evdns_base_nameserver_ip_add(hi->evDns, "114.114.114.114");
        evdns_base_nameserver_ip_add(hi->evDns, "114.114.115.115");
        //阿里云dns
        evdns_base_nameserver_ip_add(hi->evDns, "223.5.5.5");
        evdns_base_nameserver_ip_add(hi->evDns, "223.6.6.6");
#endif
        if (strcasecmp(evhttp_uri_get_scheme(hi->ctxCon.uri), "http") == 0)
            hi->ctxCon.evEvent =
                bufferevent_socket_new(hi->ctxCon.evBase, -1, BEV_OPT_CLOSE_ON_FREE);
        else {
            hi->ctxCon.ssl = SSL_new(hi->ctxCon.ctxSsl);
            hi->ctxCon.evEvent = bufferevent_openssl_socket_new(
                hi->ctxCon.evBase, -1, hi->ctxCon.ssl, BUFFEREVENT_SSL_CONNECTING,
                BEV_OPT_CLOSE_ON_FREE | BEV_OPT_DEFER_CALLBACKS | BEV_OPT_THREADSAFE);
        }
        if (hi->ctxCon.evEvent == NULL)
            break;
        bufferevent_openssl_set_allow_dirty_shutdown(hi->ctxCon.evEvent, 1);
        //必须tcp层和libevent逻辑双重制约才可（实际TCP作用占绝大多数）
        int fd = bufferevent_getfd(hi->ctxCon.evEvent);
        SetSocketOption(fd, 20);
        ret = RET_SUCCESS;
    } while (false);
    return ret;
}

int EtaxConnectIP(EHHTTP hi, char *szIP)
{
    char host[150] = "", szBuf[256] = "";
    int port, nRet = ERR_GENERIC,i = 0;
    struct evhttp_request *req = NULL;
    HEADER_STRUCT *header = NULL;
    //异步dns解析,但是不使用默认，默认的会查询IPv6，依赖系统配置，得修改系统配置才行
    do {
        GetHostAndPortFromEvUri(hi->ctxCon.uri, host, &port);  //初始化已检测过
        _WriteLog(LL_DEBUG, "Try connect TA, %s:%d", szIP, port);
        //此处使用异步dns查询出来的IP，而非域名进行连接
        if (!(hi->ctxCon.evConn = evhttp_connection_base_bufferevent_new(
                  hi->ctxCon.evBase, NULL, hi->ctxCon.evEvent, szIP, port)))
            break;
        evhttp_connection_set_flags(hi->ctxCon.evConn, EVHTTP_CON_REUSE_CONNECTED_ADDR);
        //此处不能设置重传，否则高峰时期丢包率高会导致大量TCP层快速重传，容易被税局封IP
        evhttp_connection_set_retries(hi->ctxCon.evConn, 0);
        //
        //还有个evhttp_set_timeout函数，是如下函数的上层封装，再调用evhttp_set_timeout_tv，最终底层还是设置的evconn的超时
        //蒙柏接口超时时间为60s，底层老代码是15秒，新代码3秒略先嫌小，20220802改为30秒,20220813改为15
        //此处超时时间已经经过多次实际测试越大越稳定，但是又不能无限放大，不能超过蒙柏接口大小，原则上只增加不减小
        //此处超时原则上不再改动，否则会导致税务总局的红字信息表上传出现乱码异常
        //
        // 20220814
        //该时间仅为TCP发送FIN结束包的间隔时间，但是发送FIN包后设备TCP栈依旧发送Retry包，易导致税局封IP
        //需要结合TCP的setsocketoption双重制约才可
        evhttp_connection_set_timeout(hi->ctxCon.evConn, 20);
        // GetHttpReponseCallback为++Connect Step4
        if (!(req = evhttp_request_new(EtaxGetHttpReponseCallback, hi)))
            break;
        snprintf(szBuf, sizeof(szBuf) - 1, "%s:%d", host, port);
        evhttp_add_header(evhttp_request_get_output_headers(req), "Host", szBuf);

        if(NULL != hi->user_args)
        {
            header = (HEADER_STRUCT *)(hi->user_args);
            while(header->head)
            {
                evhttp_add_header(evhttp_request_get_output_headers(req), header->head, header->value);
                //printf("%s:%s\n",header->head,header->value);
                header++;
            }
        }
        hi->user_args = NULL;

        if (evhttp_make_request(hi->ctxCon.evConn, req, EVHTTP_REQ_GET,
                                hi->modelTaAddr.szInitRequestPage) < 0) {
            evhttp_request_free(req);
            break;
        }
        nRet = RET_SUCCESS;
    } while (false);
    return nRet;
}

int EtaxEventConnect(EHHTTP hi)
{
    char host[256] = "";
    int port, nRet = RET_SUCCESS;
    bool bIpOrDoamin = false;
    struct in_addr addr;

    GetHostAndPortFromEvUri(hi->ctxCon.uri, host, &port);
    bIpOrDoamin = evutil_inet_pton(AF_INET, host, &addr);
    if (bIpOrDoamin) {
        // IP无需解析，
        if (EtaxConnectIP(hi, host))
            nRet = ERR_IO_SOCKET_FAIL;
        // TCP 3次握手ok，实际SSL通信等pendding
    } else {
        //除非指定socket类型，否则每个请求将会收到2个响应，一个TCP一个UDP，因此需要指定socket类型
        struct evutil_addrinfo hints;
        memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_TCP;
        evdns_base_set_option(hi->ctxCon.evDns, "attempts:", "4");   //最多尝试4个dns服务器
        evdns_base_set_option(hi->ctxCon.evDns, "timeout:", "0.5");  //每个dns最大0.5s超时
        evdns_base_set_option(hi->ctxCon.evDns,
                              "max-timeouts:", "2");  //所有dns加起来最大查询时间2s
        //该函数返回值判断无意义
        evdns_getaddrinfo(hi->ctxCon.evDns, host, NULL, &hints, EtaxDnsResolvCallback, hi);
        nRet = RET_SUCCESS;  //异步OK
    }
    return nRet;
}

void EtaxPenddingBreakCheck(int fd, short event, void *param)
{
    EHHTTP hi = (EHHTTP)param;
    if (hi->hDev->bBreakAllIO) {
        event_base_loopbreak(hi->ctxCon.evBase);
        hi->hDev->bBreakAllIO = false;  //自动恢复
        _WriteLog(LL_INFO, "Etax PenddingBreakCheck set pending break");
    }
}

void EtaxEvenPendingWithBreakSingal(EHHTTP hi)
{
    // struct event event_on_stack,栈上分配总是编译报错，只能使用event_new动态分配
    struct timeval tv;
    evutil_timerclear(&tv);
    tv.tv_sec = 2;  // 2秒循环一次，IO中断检测
    if (!hi->ctxCon.evBreak)
        hi->ctxCon.evBreak = event_new(hi->ctxCon.evBase, -1, EV_PERSIST, EtaxPenddingBreakCheck, hi);
    evtimer_add(hi->ctxCon.evBreak, &tv);
    // IO总超时控制即整个函数API调用时pendding阻塞等待时间
    evutil_timerclear(&tv);
    tv.tv_sec = 45;  //上层最长60s等待，底层总IO时长不超过45s，否则视为超时
    if (!hi->ctxCon.evIoTimeout)
        hi->ctxCon.evIoTimeout = event_new(hi->ctxCon.evBase, -1, 0, PenddingTimeoutCheck, hi);
    evtimer_add(hi->ctxCon.evIoTimeout, &tv);
    // wait
    event_base_dispatch(hi->ctxCon.evBase);
}


int EtaxInitSSL(EHHTTP hi)
{
    HDEV hDev = hi->hDev;
    if (hDev->bTrainFPDiskType || !hDev->pX509Cert) {
        _WriteLog(LL_WARN, "Cannot get dev's cert. Maybe a train disk?");
        return -1;
    }
    //设置客户端使用的SSL版本
    SSL_METHOD *sslMethod = NULL;
    bool bYunnanCntax = false;
    //只能使用tls1.0不能使用更高的，否则部分省份兼容性不好
    if (hDev->bDeviceType == DEVICE_AISINO || hDev->bDeviceType == DEVICE_NISEC)
        sslMethod = (SSL_METHOD *)TLSv1_method();
    else if (hDev->bDeviceType == DEVICE_CNTAX) {
        if (strstr(hi->modelTaAddr.szTaxAuthorityURL, "yunnan.chinatax.gov.cn")) {
            //云南ssl连接，暂且使用RSA算法
            sslMethod = (SSL_METHOD *)TLSv1_method();
            bYunnanCntax = true;
        } else
            sslMethod = (SSL_METHOD *)CNTLS_client_method();
    }
    //创建SSL上下文环境 每个进程只需维护一个SSL_CTX结构体
    SSL_CTX *ctx = SSL_CTX_new(sslMethod);
    if (!ctx) {
        _WriteLog(LL_FATAL, "InitSSL new sslctx failed");
        return -2;
    }
    if (bYunnanCntax) {
        hi->ctxCon.ctxSsl = ctx;
        return 0;
    }
    int nRet = -3;
    while (1) {
        // noticket参数不能加，否则部分省份兼容性不好(宁波)
        SSL_CTX_set_security_level(ctx, 0);
        if (!SSL_CTX_use_certificate(ctx, (X509 *)hDev->pX509Cert)) {
            _WriteLog(LL_FATAL, "InitSSL load certificate failed, error:%s",
                      ERR_error_string(ERR_get_error(), NULL));
            break;
        }
        //不是手动申请的，不需要手动调用EVP_PKEY_free释放pPubKey
        EVP_PKEY *pPubKey = X509_get_pubkey(hDev->pX509Cert);
        if (!pPubKey)
            break;
        if (hDev->bDeviceType == DEVICE_AISINO || hDev->bDeviceType == DEVICE_NISEC) {
            struct rsa_st *rsa = EVP_PKEY_get0_RSA(pPubKey);
            RSA_METHOD *rsameth = RSA_meth_dup(RSA_get_default_method());
            RSA_set_method(rsa, rsameth);
            RSA_meth_set_finish(rsameth, RsaFinish);
            if (DEVICE_AISINO == hDev->bDeviceType)
                RSA_meth_set_sign(rsameth, &TA_Pkcs11_Aisino);
            else if (DEVICE_NISEC == hDev->bDeviceType)
                RSA_meth_set_sign(rsameth, &TA_Pkcs11_Nisec);
            RSA_set_app_data(rsa, hDev);
            SSL_CTX_use_PrivateKey(ctx, pPubKey);
        } else if (hDev->bDeviceType == DEVICE_CNTAX) {
            //调试在/opt/tassl111bd/src/ssl/statem/statem_clnt.c 1037行左右的多个swtich标签里面
            SSL_CTX_set_options(ctx, SSL_OP_NO_ENCRYPT_THEN_MAC);
            //国密算法无需ticket
            SSL_CTX_set_options(ctx, SSL_OP_NO_TICKET);
            //不同的openssl，该名称可能字符串内容不一样，如果有兼容问题，可以不加，加上之后少握手时少发送好几十个字节
            SSL_CTX_set_cipher_list(ctx, "ECC-SM4-SM3");
            EC_KEY *ec = EC_KEY_dup(EVP_PKEY_get0_EC_KEY(pPubKey));
            EC_KEY_METHOD *ecmethNew = EC_KEY_METHOD_new(EC_KEY_OpenSSL());
            // EC_FLAG_TASS_CUSTOM_SIGN是江南天安自己定义的宏，在tassl底层会有判断，进入SM2自定义callback
            //具体位置在ECDSA_sign.c:ECDSA_sign
            EC_KEY_set_flags(ec, EC_KEY_get_flags(ec) | EC_FLAG_TASS_CUSTOM_SIGN);
            //找个地方"藏"hev，openssl1.11没有RSA的set0_app_data,替代方案EC_KEY_get_ex_new_index，暂且不用
            EC_KEY_METHOD_set_keygen(ecmethNew, (void *)hDev);
            //这个会触发EckeyFinish，因此需要在EC_KEY_METHOD_set_init之前调用
            EC_KEY_set_method(ec, ecmethNew);
            //设置回调free掉ecmethNew,Eckey好像是自动释放，暂且不管
            EC_KEY_METHOD_set_init(ecmethNew, NULL, EckeyFinish, NULL, NULL, NULL, NULL);
            EC_KEY_METHOD_set_sign(ecmethNew, TA_Pkcs11_Cntax, NULL, NULL);
            EVP_PKEY *privkey = NULL;
            privkey = EVP_PKEY_new();
            EVP_PKEY_assign_EC_KEY(privkey, ec);
            SSL_CTX_use_PrivateKey(ctx, privkey);
            EVP_PKEY_free(privkey);
        } else {
            _WriteLog(LL_WARN, "hDev->bDeviceType failed");
            break;
        }
        EVP_PKEY_free(pPubKey);
        //验证密钥是否与证书一致
        if (!SSL_CTX_check_private_key(ctx)) {
            _WriteLog(LL_WARN, "SSL_CTX_check_private_key failed");
            break;  // todo free, 这个一般不会触发，泄漏就不管了
        }
        nRet = 0;
        break;
    }
    if (nRet < 0)
        SSL_CTX_free(ctx);
    else
        hi->ctxCon.ctxSsl = ctx;
    return nRet;
}