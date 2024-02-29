#ifndef ETAX_BRIDGE
#define ETAX_BRIDGE
#include "etax_common.h"

int CreateEtaxTalk(EHHTTP hi, HDEV hDev);
int QDCreateEtaxTalk(EHHTTP hi, HDEV hDev);
void CloseEtaxConnections(HETAX hEtax);

int CreateEtaxTpassTalk(EHHTTP hi, HDEV hDev, EHHTTP hiEtax);

int TpassLogin(EHHTTP hiTpass, EHHTTP hiEtax, char *szUserIdIn, char *szPassword,char *szOptUserName);

int BuildTPassFormatHMacSHA256Packet(bool bDatagramSm4Encrypt, int encryptCode, char *szDatagram,
                                     char *szRandomHex16Bytes, struct evbuffer *bufOut);
int BuildBridgePostJmbw(char *szOrigUrl, char *szSsoToken, char *szPost, char *szOut,
                        int nOutBufSize);
int GetRandUrlyzmPath(char *szOrigUrl, char *szCookieSsoToken, char *szRepTokenKey,
                      char *szOutString, int nOutBufSize);
int GetLos28199UrlPath(char *szOrigUrl, uint64 llTimeStampMs, uint64 llFingerPrint,
                       char *szOutString, int nOutBufSize);
void GetLos28199FingerprintNum(uint64 *pOutFingerPrint);

int EtaxQueryInvoice(HETAX hEtax, char *szUrlPathQuery, char *szQueryJsonString);

int TpassLogicIo(EHHTTP hi, char *szPostUrl, char *szBuf, int nBufSize);
bool IsEtaxConnectionOKDummy(HETAX hEtax);

int get_tpass_addr(char *src_data,char **tpass_addr,char *sys_type);

////////////////////////////////////////////////////////////////////////////
int EtaxBridgeClose(EHHTTP hi);
int EtaxGetEtaxServerURL(EHHTTP hi, HDEV hDev);
int TPassLoginAuthSm4AndPublickey(EHHTTP hi,char *szRegCode);
int TPassLoginAuthHelloAndAuth(EHHTTP hi, struct evhttp_uri *uriTpassFull);
int TPassLoginPasswordLoginDo(EHHTTP hiTpass, char *szUserIdIn, char *szPassword,char *szBuf, int nBufSize);
int TPassLoginUserInfo(EHHTTP hiTpass, EHHTTP hiEtax,char *errinfo_uft8,char *szBuf,int nBufSize);
int EtaxQueryInvoice_test(EHHTTP hi, char *szUrlPathQuery, char *szQueryJsonString);
int InitSecurityConfig(EHHTTP hi);
int CreateEtaxDpptTalk(HETAX hEtax);
int TPassSelectRelationList(EHHTTP hiTpass, char **outenterpriselist,char *szBuf, int nBufSize);
int TPassSelectRelationchange(EHHTTP hiTpass,char *uniqueIdentity,char *realationStatus,char *szBuf, int nBufSize);
int CreateEtaxDpptTalkWang(EHHTTP hi, HDEV hDev, EHHTTP hiEtax);
int CallbackDpptfpdown(EHHTTP hi, struct evhttp_request *req);
int DpptHandshake(HETAX hEtax, char *szDpptInitUrl);
void CloseEtaxConnectionsNoEtax(HETAX hEtax);
int EtaxQueryInvoicefpdown(HETAX hEtax, char *szUrlPathQuery);
int TPassLoginUserInfoChange(EHHTTP hiTpass, EHHTTP hiEtax,char *uniqueIdentity,char *realationStatus,char *errinfo_uft8,char *szBuf,int nBufSize);

int TPassLoginPasswordLoginDoShanXi(EHHTTP hiTpass, char *szUserIdIn, char *szPassword,char *szBuf, int nBufSize);
int TPassLoginAuthHelloAndAuthShanXi(EHHTTP hi, struct evhttp_uri *uriTpassFull);
int TPassLoginAuthDecodeStep3RepForStep4ShanXi(EHHTTP hi, struct evhttp_uri *uriTpassFull,char *szRandomHex16Bytes, char *szRepBuf, int nRepBufSize);
int get_final_url(char *src_data,char *code,char *final_url);
int EtaxLoadDevOnline(HETAX hEtax, HDEV hDev,EHFPXX fpxx);
int EtaxAnalyzeJsonBuf(char *szJsonString, EHFPXX fpxx, HDEV hDev,char *errinfo_uft8);
int EtaxMakeInvoiceMain(HETAX hEtax, EHFPXX fpxx, HDEV hDev,char *errinfo_uft8,char *szJsonStr);


bool FillUploadAddrModelDynamic(uint8 bServType, char *szRegCode6, char *szTaxAuthorityURL,char *szInitRequestPage, struct EtaxUploadAddrModel *pstOutModel);

int GetRandUrlyzmPath(char *szOrigUrl, char *szCookieSsoToken, char *szRepTokenKey,char *szOutString, int nOutBufSize);

//重庆
int TPassLoginPasswordLoginDoChongQing(EHHTTP hiTpass, char *szUserIdIn, char *szPassword,char *szBuf, int nBufSize);
int TPassLoginAuthHelloAndAuthChongQing(EHHTTP hi, struct evhttp_uri *uriTpassFull);
int TPassLoginAuthDecodeStep3RepForStep4ChongQing(EHHTTP hi, struct evhttp_uri *uriTpassFull,char *szRandomHex16Bytes, char *szRepBuf, int nRepBufSize);

//
int EtaxQueryInvoiceJiangSu(HETAX hEtax, char *szUrlPathQuery, char *szQueryJsonString);
int GetRandUrlyzmPathJiangSu(char *szOrigUrl, char *szCookieSsoToken, char *szRepTokenKey,char *szOutString, int nOutBufSize);
int DpptJiangSuConnect(EHHTTP hi, char *szUrlPathQuery, HDEV hDev);
int DpptJiangSuConnectBycookie(EHHTTP hi, HDEV hDev);

int DpptshanghaiConnectBycookie(EHHTTP hi, HDEV hDev);
int DpptshanghaiConnect(EHHTTP hi, char *szUrlPathQuery, HDEV hDev,char *stateCode);

int DpptHeNanConnect(EHHTTP hi, char *szUrlPathQuery, HDEV hDev);
int DpptHeNanConnectBycookie(EHHTTP hi, HDEV hDev);

int CreateEtaxTalkCookies(EHHTTP hi, HDEV hDev);
int GetEtaxTpassAndInit(EHHTTP hi, EHHTTP hiEtax, struct evhttp_uri **uriTpassFull);
int ExportEtaxCookies(HETAX hEtax, char *szEtaxOutCookies,char *szTpassOutCookies);
int EtaxReuseEtaxTpassTalk(EHHTTP hi, struct evbuffer *evTpassCookies,HDEV hDev);
int TpassLoginMainFrame(HHTTP hiTpass, HHTTP hiEtax, char *szToken, char *szCode,
                        char *szMainPageDirectUrl);
int TPassLoginPasswordLoginDoGetCookies(EHHTTP hiTpass, EHHTTP hiEtax,char *szUserIdIn, char *szPassword,char *szBuf, int nBufSize);
int TPassSelectRelationchangeGetCookies(EHHTTP hiTpass,EHHTTP hiEtax,char *szUserIdIn, char *szPassword,char *uniqueIdentity,char *realationStatus,char *szBuf, int nBufSize);

int CreateEtaxCookies(HETAX hEtax, HDEV hDev, char *szEtaxAllCookies, char *szTpassAllCookies,char *errinfo_uft8);

int TPassLoginAuthHelloAndAuth_soft_cert(EHHTTP hi, struct evhttp_uri *uriTpassFull,char *svr_container,char *soft_cert_ip,int soft_cert_port);

int TPassLoginAuthHelloAndAuth_soft_cert_shanxi(EHHTTP hi, struct evhttp_uri *uriTpassFull,char *svr_container,char *soft_cert_ip,int soft_cert_port,char *errinfo_uft8_21);
int TPassLoginAuthHelloAndAuth_soft_cert_chongqing(EHHTTP hi, struct evhttp_uri *uriTpassFull,char *svr_container,char *soft_cert_ip,int soft_cert_port);
int TPassLoginAuthHelloAndAuth_soft_cert_21(EHHTTP hi, struct evhttp_uri *uriTpassFull,char *svr_container,char *soft_cert_ip,int soft_cert_port);

int DpptShanghaiConnect(EHHTTP hi, char *szUrlPathQuery, HDEV hDev);

int TPassLoginAuthHelloAndAuth_soft_cert_21_Msg(EHHTTP hi, struct evhttp_uri *uriTpassFull,char *svr_container,char *soft_cert_ip,int soft_cert_port,char *errinfo_uft8_21);
int TPassLoginPasswordLoginDoGetCookies_shanghai(EHHTTP hiTpass, EHHTTP hiEtax,char *szUserIdIn, char *szPassword,char *szBuf, int nBufSize,char *out_token);
int TPassSelectRelationchangeGetCookies_shanghai(EHHTTP hiTpass,EHHTTP hiEtax,char *szUserIdIn, char *szPassword,char *uniqueIdentity,char *realationStatus,char *szBuf, int nBufSize,char *out_token);

int TPassLoginAuthHelloAndAuthGuizhou(EHHTTP hi, struct evhttp_uri *uriTpassFull);
int TPassLoginAuthHelloAndAuth_soft_cert_guizhou(EHHTTP hi, struct evhttp_uri *uriTpassFull,char *svr_container,char *soft_cert_ip,int soft_cert_port,char *errinfo_uft8_21);


int TPassLoginAuthHelloAndAuth_soft_cert_21_Msg_shaanxi(EHHTTP hi, struct evhttp_uri *uriTpassFull,char *svr_container,char *soft_cert_ip,int soft_cert_port,char *errinfo_uft8_21);
int TPassLoginAuthHelloAndAuth_soft_cert_21_Msg_guangxi(EHHTTP hi, struct evhttp_uri *uriTpassFull,char *svr_container,char *soft_cert_ip,int soft_cert_port,char *errinfo_uft8_21);


#endif
