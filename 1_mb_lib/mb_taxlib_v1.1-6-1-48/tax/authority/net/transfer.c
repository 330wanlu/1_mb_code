/*************************************************
File name:   transfer.c
Description: ���ڴ����š�������˰����˰��ֽ����Ĵ����Э���װ
             ����TCP���ӡ�SSL���ӡ�HTTP��socks5����
Author:      Zako
Version:     1.0
Date:        2019.12
History:
20201225     ������������Э��
20211018     ���ֽӿڵ�����������Init��ʼ��
20220627     Ϊ���SSL���Ӳ�����libevent�����ݣ�����Ⱦ����ҷ����룬��libeventʵ��SSL����
*************************************************/
// SSL envirnment = openssl 1.0.2k
#include "transfer.h"
#include <openssl/ossl_typ.h>
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"  // ignore 'TLSv1_1_method' deprecated

//ע������ķ���ֵ���������ⷵ�أ���ȷ����1�� ���󷵻�0
int TA_Pkcs11_Aisino(int type, const uint8 *m, unsigned int m_length, uint8 *sigret,
                     unsigned int *siglen, const RSA *rsa)
{
    if (type > 0) {
        // null,�����ǻص�����������ȥ��type�������ÿպ�����������������
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
    // 0x11,Ϊ����ַ�,���ᱻ�滻,һ������滻36(0x24)���ֽ�

    char szBuff[256] = "";
    //ѡ��application�������е��̼����Բ��ã�û��δ����ǩ������
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

//ע������ķ���ֵ���������ⷵ�أ���ȷ����1�� ���󷵻�0
int TA_Pkcs11_Nisec(int type, const uint8 *m, unsigned int m_length, uint8 *sigret,
                    unsigned int *siglen, const RSA *rsa)
{
    if (type > 0) {
        // null,�����ǻص�����������ȥ��type�������ÿպ�����������������
    }
    // m_length == 36
    if (m_length > 48 || m_length < 24) {
        _WriteLog(LL_WARN, "Issue:TA_Pkcs11_Nisec return -1");
        return 0;
    }
    HDEV hDev = (HDEV)RSA_get_app_data(rsa);
    HUSB hUSB = hDev->hUSB;
    // crypt_login�Ѿ���½��,����Ҫ�ظ�
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
    // 0x11,Ϊ����ַ�,���ᱻ�滻,һ������滻36(0x24)���ֽ�
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

// �ص�·��xx->opensslxx->...NID_brainpoolP160t1(922)()?->
//->ssl3_finish_mac������BIO_write����λص�ѹ��ssl-handshake-buf���������Ϊ(80+fcd+4d+bf+4+58f+a2=178e=6030)=tbs_data
//->sm3(tbs_data)=hashOfTbsdata
//->sm3(zvalue(1234567812345678)[32Bytes]+hashOfTbsdata[32Bytes])=dgst
//->TA_Pkcs11_Cntax
//
// openssl1.1.1���ͻ���֤����֤ǩ������Ϊtls_construct_cert_verify
//ʹ��EC��Կ�Թ�ϣ����ǩ����������DER�����sigǩ�����䳤��Ϊsiglen���ɹ�ʱ����ֵΪ1������ʱ����ֵΪ0��
int TA_Pkcs11_Cntax(int type, const unsigned char *dgst, int dlen, unsigned char *sig,
                    unsigned int *siglen, const BIGNUM *kinv, const BIGNUM *r, EC_KEY *eckey)
{
    if (r || !kinv || !dgst || !dlen || !sig || !siglen) {
        // null,�����ǻص�����������ȥ��type�������ÿպ�����������������
    }
    if (type <= 0 || !eckey || dlen != 32) {
        _WriteLog(LL_WARN, "Issue:TA_Pkcs11_Cntax return -1");
        return 0;
    }
    uint8 abSM3HashRet[64] = {0};
    uint8 abSM3Hash[32 + 1] = {0};
    abSM3Hash[0] = 0x81;  //���ֽ�Ϊ0x81,����20�ֽ�Ϊsha1Hash SM3_DIGEST_LENGTH 32
    memcpy(abSM3Hash + 1, dgst, dlen);
    const EC_KEY_METHOD *ecmeth = EC_KEY_get_method(eckey);
    HDEV hDev = NULL;
    EC_KEY_METHOD_get_keygen(ecmeth, (void *)&hDev);
    if (!hDev) {
        _WriteLog(LL_WARN, "Hdev failed");
        return 0;
    }
    //����ʹ��CntaxGetDevice64ByteSignRepAsn1����Ϊ�����Ǻܴ����Hashǩ���㷨��Zֵ����ʲô���Ѿ���֮ǰ�Ĵ�������������
    if (CntaxCryptLogin(hDev->hUSB, hDev->szDeviceID) <= 0) {
        _WriteLog(LL_WARN, "Issue:TA_Pkcs11_Nisec return -2");
        return 0;
    }
    if (NisecCryptHash(hDev->hUSB, 3, abSM3Hash, sizeof(abSM3Hash), (char *)abSM3HashRet,
                       sizeof(abSM3HashRet)) < 0) {
        _WriteLog(LL_WARN, "Issue:TA_Pkcs11_Nisec return -3");
        return 0;
    }
    unsigned char abTaHandshake_Model[72] = {0};  //�72
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

//����cntax˰��RSA�㷨���Ӳ��� /openssl s_client -tls1_2 -connect 116.52.1.245:9006
//�����㷨�찲��s_client app�����⣬���Բο�PR��� https://github.com/jntass/TASSL-1.1.1b/pull/37
//�޸ĺ������ ./openssl s_client -cipher ECC-SM4-SM3 -cert ../tassl_demo/cert/certs/CS.pem
//-cert_enc ../tassl_demo/cert/certs/CE.pem -CAfile capath -connect 116.52.1.245:9006
int InitSSL(HHTTP hi)
{
    HDEV hDev = hi->hDev;
    if (hDev->bTrainFPDiskType || !hDev->pX509Cert) {
        _WriteLog(LL_WARN, "Cannot get dev's cert. Maybe a train disk?");
        return -1;
    }
    //���ÿͻ���ʹ�õ�SSL�汾
    SSL_METHOD *sslMethod = NULL;
    bool bYunnanCntax = false;
    //ֻ��ʹ��tls1.0����ʹ�ø��ߵģ����򲿷�ʡ�ݼ����Բ���
    if (hDev->bDeviceType == DEVICE_AISINO || hDev->bDeviceType == DEVICE_NISEC)
        sslMethod = (SSL_METHOD *)TLSv1_method();
    else if (hDev->bDeviceType == DEVICE_CNTAX) {
        if (strstr(hi->modelTaAddr.szTaxAuthorityURL, "yunnan.chinatax.gov.cn")) {
            //����ssl���ӣ�����ʹ��RSA�㷨
            sslMethod = (SSL_METHOD *)TLSv1_method();
            bYunnanCntax = true;
        } else
            sslMethod = (SSL_METHOD *)CNTLS_client_method();
    }
    //����SSL�����Ļ��� ÿ������ֻ��ά��һ��SSL_CTX�ṹ��
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
        // noticket�������ܼӣ����򲿷�ʡ�ݼ����Բ���(����)
        SSL_CTX_set_security_level(ctx, 0);
        if (!SSL_CTX_use_certificate(ctx, (X509 *)hDev->pX509Cert)) {
            _WriteLog(LL_FATAL, "InitSSL load certificate failed, error:%s",
                      ERR_error_string(ERR_get_error(), NULL));
            break;
        }
        //�����ֶ�����ģ�����Ҫ�ֶ�����EVP_PKEY_free�ͷ�pPubKey
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
            //������/opt/tassl111bd/src/ssl/statem/statem_clnt.c 1037�����ҵĶ��swtich��ǩ����
            SSL_CTX_set_options(ctx, SSL_OP_NO_ENCRYPT_THEN_MAC);
            //�����㷨����ticket
            SSL_CTX_set_options(ctx, SSL_OP_NO_TICKET);
            //��ͬ��openssl�������ƿ����ַ������ݲ�һ��������м������⣬���Բ��ӣ�����֮��������ʱ�ٷ��ͺü�ʮ���ֽ�
            SSL_CTX_set_cipher_list(ctx, "ECC-SM4-SM3");
            EC_KEY *ec = EC_KEY_dup(EVP_PKEY_get0_EC_KEY(pPubKey));
            EC_KEY_METHOD *ecmethNew = EC_KEY_METHOD_new(EC_KEY_OpenSSL());
            // EC_FLAG_TASS_CUSTOM_SIGN�ǽ����찲�Լ�����ĺ꣬��tassl�ײ�����жϣ�����SM2�Զ���callback
            //����λ����ECDSA_sign.c:ECDSA_sign
            EC_KEY_set_flags(ec, EC_KEY_get_flags(ec) | EC_FLAG_TASS_CUSTOM_SIGN);
            //�Ҹ��ط�"��"hev��openssl1.11û��RSA��set0_app_data,�������EC_KEY_get_ex_new_index�����Ҳ���
            EC_KEY_METHOD_set_keygen(ecmethNew, (void *)hDev);
            //����ᴥ��EckeyFinish�������Ҫ��EC_KEY_METHOD_set_init֮ǰ����
            EC_KEY_set_method(ec, ecmethNew);
            //���ûص�free��ecmethNew,Eckey�������Զ��ͷţ����Ҳ���
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
        //��֤��Կ�Ƿ���֤��һ��
        if (!SSL_CTX_check_private_key(ctx)) {
            _WriteLog(LL_WARN, "SSL_CTX_check_private_key failed");
            break;  // todo free, ���һ�㲻�ᴥ����й©�Ͳ�����
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
    //�ر�����
    if (bio) {
        //������ôд����Ȼ�о���
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

//------------------------------���ӳ�ʼ������������begin----------------------------------------------------------

// old:HTTPEventInit
int EventInit(HHTTP hi)
{
    int ret = ERR_PARM_INIT;
    char host[256] = "";
    int port = 0;
    do {
        //���߳�֧�֣�������֮������ڴ�й¶������libevent���ϲ������Թ�ܸ����⣬��ʱ�����߳�֧�֣���Ҫʱ������
        //./configure --disable-debug-mode
        //�ο�
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
        //���ǵ���ͨ������dns���ؾ��⣬����ʹ���豸�Դ���dns������
        // MIPS4���豸����dns�����������������⣬ʹ���Զ��������dns
        if (!(hi->evDns = evdns_base_new(hi->evBase, EVDNS_BASE_NAMESERVERS_NO_DEFAULT)))
            break;
        // 114dns
        evdns_base_nameserver_ip_add(hi->evDns, "114.114.114.114");
        evdns_base_nameserver_ip_add(hi->evDns, "114.114.115.115");
        //������dns
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
        //����tcp���libevent�߼�˫����Լ�ſɣ�ʵ��TCP����ռ���������
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
    //�첽dns����,���ǲ�ʹ��Ĭ�ϣ�Ĭ�ϵĻ��ѯIPv6������ϵͳ���ã����޸�ϵͳ���ò���
    do {
        GetHostAndPortFromEvUri(hi->uri, host, &port);  //��ʼ���Ѽ���
        _WriteLog(LL_DEBUG, "Try connect TA, %s:%d", szIP, port);
        //�˴�ʹ���첽dns��ѯ������IP������������������
        if (!(hi->evConn = evhttp_connection_base_bufferevent_new(hi->evBase, NULL, hi->evEvent,
                                                                  szIP, port)))
            break;
        evhttp_connection_set_flags(hi->evConn, EVHTTP_CON_REUSE_CONNECTED_ADDR);
        //�˴����������ش�������߷�ʱ�ڶ����ʸ߻ᵼ�´���TCP������ش������ױ�˰�ַ�IP
        evhttp_connection_set_retries(hi->evConn, 0);
        //
        //���и�evhttp_set_timeout�����������º������ϲ��װ���ٵ���evhttp_set_timeout_tv�����յײ㻹�����õ�evconn�ĳ�ʱ
        //�ɰؽӿڳ�ʱʱ��Ϊ60s���ײ��ϴ�����15�룬�´���3��������С��20220802��Ϊ30��,20220813��Ϊ15
        //�˴���ʱʱ���Ѿ��������ʵ�ʲ���Խ��Խ�ȶ��������ֲ������޷Ŵ󣬲��ܳ����ɰؽӿڴ�С��ԭ����ֻ���Ӳ���С
        //�˴���ʱԭ���ϲ��ٸĶ�������ᵼ��˰���ֵܾĺ�����Ϣ���ϴ����������쳣
        //
        // 20220814
        //��ʱ���ΪTCP����FIN�������ļ��ʱ�䣬���Ƿ���FIN�����豸TCPջ���ɷ���Retry�����׵���˰�ַ�IP
        //��Ҫ���TCP��setsocketoption˫����Լ�ſ�
        evhttp_connection_set_timeout(hi->evConn, 20);
        // GetHttpReponseCallbackΪ++Connect Step4
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
            break;  // dns ��������ѯʧ��
        for (ai = addr; ai; ai = ai->ai_next) {
            const char *s = NULL;
            memset(buf, 0, sizeof(buf));
            if (ai->ai_family == AF_INET) {
                struct sockaddr_in *sin = (struct sockaddr_in *)ai->ai_addr;
                s = evutil_inet_ntop(AF_INET, &sin->sin_addr, buf, 128);
            } else if (ai->ai_family == AF_INET6) {
                struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *)ai->ai_addr;
                s = evutil_inet_ntop(AF_INET6, &sin6->sin6_addr, buf, 128);
                continue;  //����ҪIPv6
            }
            if (!s || strlen(s) < 7)
                continue;
            // dns OK
            if (ConnectIP(hi, (char *)s))
                continue;
            // TCP 3������ok��ʵ��SSLͨ�ŵ�pendding
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
            break;  // dns ��������ѯʧ��
        for (ai = addr; ai; ai = ai->ai_next) {
            const char *s = NULL;
            memset(buf, 0, sizeof(buf));
            if (ai->ai_family == AF_INET) {
                struct sockaddr_in *sin = (struct sockaddr_in *)ai->ai_addr;
                s = evutil_inet_ntop(AF_INET, &sin->sin_addr, buf, 128);
            } else if (ai->ai_family == AF_INET6) {
                struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *)ai->ai_addr;
                s = evutil_inet_ntop(AF_INET6, &sin6->sin6_addr, buf, 128);
                continue;  //����ҪIPv6
            }
            if (!s || strlen(s) < 7)
                continue;
            // dns OK
            if (EtaxConnectIP(hi, (char *)s))
                continue;
            // TCP 3������ok��ʵ��SSLͨ�ŵ�pendding
            break;
        }
    } while (false);
    if (addr)
        evutil_freeaddrinfo(addr);
}


//����ʹ��C�������libevent�Դ���Dns��������������������ϵͳ���ã�����ȥ����resolv.conf
//Ĭ�ϻ��ѯAAA IPv6��¼������˰�ַ�����IPv6���������ⲻ��Ӧ���ᵼ�³���dns��ѯ�ӳ�15������
//��������취1)�����豸IPv6 2)ʹ���Զ���dns�첽��ѯ
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
        // IP���������
        if (ConnectIP(hi, host))
            nRet = ERR_IO_SOCKET_FAIL;
        // TCP 3������ok��ʵ��SSLͨ�ŵ�pendding
    } else {
        //����ָ��socket���ͣ�����ÿ�����󽫻��յ�2����Ӧ��һ��TCPһ��UDP�������Ҫָ��socket����
        struct evutil_addrinfo hints;
        memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_TCP;
        evdns_base_set_option(hi->evDns, "attempts:", "4");   //��ೢ��4��dns������
        evdns_base_set_option(hi->evDns, "timeout:", "0.5");  //ÿ��dns���0.5s��ʱ
        evdns_base_set_option(hi->evDns, "max-timeouts:", "2");  //����dns����������ѯʱ��2s
        //�ú�������ֵ�ж�������
        evdns_getaddrinfo(hi->evDns, host, NULL, &hints, DnsResolvCallback, hi);
        nRet = RET_SUCCESS;  //�첽OK
    }
    return nRet;
}

void PenddingBreakCheck(int fd, short event, void *param)
{
    HHTTP hi = (HHTTP)param;
    if (hi->hDev->bBreakAllIO) {
        event_base_loopbreak(hi->evBase);
        hi->hDev->bBreakAllIO = false;  //�Զ��ָ�
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
    // struct event event_on_stack,ջ�Ϸ������Ǳ��뱨��ֻ��ʹ��event_new��̬����
    struct timeval tv;
    evutil_timerclear(&tv);
    tv.tv_sec = 2;  // 2��ѭ��һ�Σ�IO�жϼ��
    if (!hi->evBreak)
        hi->evBreak = event_new(hi->evBase, -1, EV_PERSIST, PenddingBreakCheck, hi);
    evtimer_add(hi->evBreak, &tv);
    // IO�ܳ�ʱ���Ƽ���������API����ʱpendding�����ȴ�ʱ��
    evutil_timerclear(&tv);
    tv.tv_sec = 45;  //�ϲ��60s�ȴ����ײ���IOʱ��������45s��������Ϊ��ʱ
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
        //���߳�֧�֣�������֮������ڴ�й¶������libevent���ϲ������Թ�ܸ����⣬��ʱ�����߳�֧�֣���Ҫʱ������
        //./configure --disable-debug-mode
        //�ο�
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
        //���ǵ���ͨ������dns���ؾ��⣬����ʹ���豸�Դ���dns������
        // MIPS4���豸����dns�����������������⣬ʹ���Զ��������dns
        if (!(hi->evDns = evdns_base_new(hi->evBase, EVDNS_BASE_NAMESERVERS_NO_DEFAULT)))
            break;
        // 114dns
        evdns_base_nameserver_ip_add(hi->evDns, "114.114.114.114");
        evdns_base_nameserver_ip_add(hi->evDns, "114.114.115.115");
        //������dns
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
        //����tcp���libevent�߼�˫����Լ�ſɣ�ʵ��TCP����ռ���������
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
    //�첽dns����,���ǲ�ʹ��Ĭ�ϣ�Ĭ�ϵĻ��ѯIPv6������ϵͳ���ã����޸�ϵͳ���ò���
    do {
        GetHostAndPortFromEvUri(hi->ctxCon.uri, host, &port);  //��ʼ���Ѽ���
        _WriteLog(LL_DEBUG, "Try connect TA, %s:%d", szIP, port);
        //�˴�ʹ���첽dns��ѯ������IP������������������
        if (!(hi->ctxCon.evConn = evhttp_connection_base_bufferevent_new(
                  hi->ctxCon.evBase, NULL, hi->ctxCon.evEvent, szIP, port)))
            break;
        evhttp_connection_set_flags(hi->ctxCon.evConn, EVHTTP_CON_REUSE_CONNECTED_ADDR);
        //�˴����������ش�������߷�ʱ�ڶ����ʸ߻ᵼ�´���TCP������ش������ױ�˰�ַ�IP
        evhttp_connection_set_retries(hi->ctxCon.evConn, 0);
        //
        //���и�evhttp_set_timeout�����������º������ϲ��װ���ٵ���evhttp_set_timeout_tv�����յײ㻹�����õ�evconn�ĳ�ʱ
        //�ɰؽӿڳ�ʱʱ��Ϊ60s���ײ��ϴ�����15�룬�´���3��������С��20220802��Ϊ30��,20220813��Ϊ15
        //�˴���ʱʱ���Ѿ��������ʵ�ʲ���Խ��Խ�ȶ��������ֲ������޷Ŵ󣬲��ܳ����ɰؽӿڴ�С��ԭ����ֻ���Ӳ���С
        //�˴���ʱԭ���ϲ��ٸĶ�������ᵼ��˰���ֵܾĺ�����Ϣ���ϴ����������쳣
        //
        // 20220814
        //��ʱ���ΪTCP����FIN�������ļ��ʱ�䣬���Ƿ���FIN�����豸TCPջ���ɷ���Retry�����׵���˰�ַ�IP
        //��Ҫ���TCP��setsocketoption˫����Լ�ſ�
        evhttp_connection_set_timeout(hi->ctxCon.evConn, 20);
        // GetHttpReponseCallbackΪ++Connect Step4
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
        // IP���������
        if (EtaxConnectIP(hi, host))
            nRet = ERR_IO_SOCKET_FAIL;
        // TCP 3������ok��ʵ��SSLͨ�ŵ�pendding
    } else {
        //����ָ��socket���ͣ�����ÿ�����󽫻��յ�2����Ӧ��һ��TCPһ��UDP�������Ҫָ��socket����
        struct evutil_addrinfo hints;
        memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_TCP;
        evdns_base_set_option(hi->ctxCon.evDns, "attempts:", "4");   //��ೢ��4��dns������
        evdns_base_set_option(hi->ctxCon.evDns, "timeout:", "0.5");  //ÿ��dns���0.5s��ʱ
        evdns_base_set_option(hi->ctxCon.evDns,
                              "max-timeouts:", "2");  //����dns����������ѯʱ��2s
        //�ú�������ֵ�ж�������
        evdns_getaddrinfo(hi->ctxCon.evDns, host, NULL, &hints, EtaxDnsResolvCallback, hi);
        nRet = RET_SUCCESS;  //�첽OK
    }
    return nRet;
}

void EtaxPenddingBreakCheck(int fd, short event, void *param)
{
    EHHTTP hi = (EHHTTP)param;
    if (hi->hDev->bBreakAllIO) {
        event_base_loopbreak(hi->ctxCon.evBase);
        hi->hDev->bBreakAllIO = false;  //�Զ��ָ�
        _WriteLog(LL_INFO, "Etax PenddingBreakCheck set pending break");
    }
}

void EtaxEvenPendingWithBreakSingal(EHHTTP hi)
{
    // struct event event_on_stack,ջ�Ϸ������Ǳ��뱨��ֻ��ʹ��event_new��̬����
    struct timeval tv;
    evutil_timerclear(&tv);
    tv.tv_sec = 2;  // 2��ѭ��һ�Σ�IO�жϼ��
    if (!hi->ctxCon.evBreak)
        hi->ctxCon.evBreak = event_new(hi->ctxCon.evBase, -1, EV_PERSIST, EtaxPenddingBreakCheck, hi);
    evtimer_add(hi->ctxCon.evBreak, &tv);
    // IO�ܳ�ʱ���Ƽ���������API����ʱpendding�����ȴ�ʱ��
    evutil_timerclear(&tv);
    tv.tv_sec = 45;  //�ϲ��60s�ȴ����ײ���IOʱ��������45s��������Ϊ��ʱ
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
    //���ÿͻ���ʹ�õ�SSL�汾
    SSL_METHOD *sslMethod = NULL;
    bool bYunnanCntax = false;
    //ֻ��ʹ��tls1.0����ʹ�ø��ߵģ����򲿷�ʡ�ݼ����Բ���
    if (hDev->bDeviceType == DEVICE_AISINO || hDev->bDeviceType == DEVICE_NISEC)
        sslMethod = (SSL_METHOD *)TLSv1_method();
    else if (hDev->bDeviceType == DEVICE_CNTAX) {
        if (strstr(hi->modelTaAddr.szTaxAuthorityURL, "yunnan.chinatax.gov.cn")) {
            //����ssl���ӣ�����ʹ��RSA�㷨
            sslMethod = (SSL_METHOD *)TLSv1_method();
            bYunnanCntax = true;
        } else
            sslMethod = (SSL_METHOD *)CNTLS_client_method();
    }
    //����SSL�����Ļ��� ÿ������ֻ��ά��һ��SSL_CTX�ṹ��
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
        // noticket�������ܼӣ����򲿷�ʡ�ݼ����Բ���(����)
        SSL_CTX_set_security_level(ctx, 0);
        if (!SSL_CTX_use_certificate(ctx, (X509 *)hDev->pX509Cert)) {
            _WriteLog(LL_FATAL, "InitSSL load certificate failed, error:%s",
                      ERR_error_string(ERR_get_error(), NULL));
            break;
        }
        //�����ֶ�����ģ�����Ҫ�ֶ�����EVP_PKEY_free�ͷ�pPubKey
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
            //������/opt/tassl111bd/src/ssl/statem/statem_clnt.c 1037�����ҵĶ��swtich��ǩ����
            SSL_CTX_set_options(ctx, SSL_OP_NO_ENCRYPT_THEN_MAC);
            //�����㷨����ticket
            SSL_CTX_set_options(ctx, SSL_OP_NO_TICKET);
            //��ͬ��openssl�������ƿ����ַ������ݲ�һ��������м������⣬���Բ��ӣ�����֮��������ʱ�ٷ��ͺü�ʮ���ֽ�
            SSL_CTX_set_cipher_list(ctx, "ECC-SM4-SM3");
            EC_KEY *ec = EC_KEY_dup(EVP_PKEY_get0_EC_KEY(pPubKey));
            EC_KEY_METHOD *ecmethNew = EC_KEY_METHOD_new(EC_KEY_OpenSSL());
            // EC_FLAG_TASS_CUSTOM_SIGN�ǽ����찲�Լ�����ĺ꣬��tassl�ײ�����жϣ�����SM2�Զ���callback
            //����λ����ECDSA_sign.c:ECDSA_sign
            EC_KEY_set_flags(ec, EC_KEY_get_flags(ec) | EC_FLAG_TASS_CUSTOM_SIGN);
            //�Ҹ��ط�"��"hev��openssl1.11û��RSA��set0_app_data,�������EC_KEY_get_ex_new_index�����Ҳ���
            EC_KEY_METHOD_set_keygen(ecmethNew, (void *)hDev);
            //����ᴥ��EckeyFinish�������Ҫ��EC_KEY_METHOD_set_init֮ǰ����
            EC_KEY_set_method(ec, ecmethNew);
            //���ûص�free��ecmethNew,Eckey�������Զ��ͷţ����Ҳ���
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
        //��֤��Կ�Ƿ���֤��һ��
        if (!SSL_CTX_check_private_key(ctx)) {
            _WriteLog(LL_WARN, "SSL_CTX_check_private_key failed");
            break;  // todo free, ���һ�㲻�ᴥ����й©�Ͳ�����
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