#ifndef _COMMON_FUNC_H
#define _COMMON_FUNC_H
#include "common.h"
#include <mpfr.h>

//int writelog(uint8 bLoglevl, char *appcode, char *event, const char *format, ...);
void _WriteLog(uint8 bLoglevl, char *msg, ...);
void FreeDevInfo(HDEV hDev);
HDEV MallocDevInfo();
void GetNicMAC(char *szTaxID, char *szOut, uint8 bSplitFormat);
void GetMotherBoardId(char *szTaxID, char *szOut);
HUSB MallocUSBSession();
void FreeUSBSession(HUSB husb);
int GetSelfAbsolutePath(char *buf, int nBufflen);
int TranslateStandTime(int nType, char *szStandTimeIn, char *szOutTime);
void UpdateDevInfoToFPXX(HDEV hDev, HFPXX fpxx);
int ProgInitDestory(uint8 bInitOrDestory);
void PriceRound(char *szPrice, int nRoundSize, char *szOutPrice);
int PriceRemoveTax(char *szDJ, char *szSLV, int nRoundSize, char *szOutPriceWithTax);
void GetRandom(char *szOut, int nInNeedLen, int nMod, char *szSeedString);
int GetCertSNByteOrStr(X509 *pCert, char *retSN, uint8 bRetBinOrStr);
int TimeOutMonthDeadline(char *szBaseTime, char *szTimeDev);
void CalcMD5(void *pSrc, int nSrcLen, void *pDst16);
int CalcSHA1(void *pSrc, int nSrcLen, void *pOutDigest);
uint8 *CalcHmacSha256(const void *key, int keylen, const uint8 *data, int datalen, uint8 *result,
                      uint32 *resultlen);
int MinusTime(char *szTime1, char *szTime2);
int CalcDecodeLength(const char *b64input);
void GetFotmatJEStr(char *je, char *szOut);
int GetFotmatJE100(char *je);
int GetSpsmmcSpmc(char *szSmmcMc, char *spsmmc, char *spmc);
X509 *GetX509Cert(HUSB hUSB, uint8 bDeviceType, bool bBigCryptIO);
int CheckAndAppendCertBuf(uint8 *pRetBuff, int nRetBuffIndex, uint8 *pCertBuff, int nCertLen);
void GetBJSJ(char *szDeviceTime, char *szOut, uint8 bStyle);
int TAXML_Formater(mxml_node_t *root, char *szOut);
void UpdateFpxxAllTime(char *szStandTimeInput, HFPXX fpxx);
//int EnumAllUSBDevice_CallBack(int nDeviceType, HUSB hUSB);
bool IsTheSameMonth(char *szTime1, char *szTime2);
bool SetProxy(HDEV hDev, char *szProxyStr);
void DevIOCtrl(HDEV hDev, bool bStatus);
uint8 StaticCheckTrainDeviceByDeviceID(char *szDeviceID);
int PasswordNotice(bool bDeviceOrCert, char *szLastErrorDescription);
//int DetectDeviceInit(HDEV hDev, HUSB hUSB, uint8 bAuthDev, int nBusID, int nDevID);
int SetLastError(HUSB hUSB, enum ErrorCode bErrorCode, char *msg, ...);
int GetLastErrorType(enum ErrorCode bErrorCode);
void ClearLastError(HUSB hUSB);
int GetHostAndPortFromEvUri(struct evhttp_uri *uri, char *szHostOut, int *nPortOut);
uint32 GetU32IdFromString(char *src);
//nt LoadDevice(uint8 bUsbType, HDEV hDev, HUSB hUSB, uint8 bAuthDev);
HDITI GetInvoiceTypeEnvirnment(HDEV hDev, uint8 bCommmonInvType);
mxml_node_t *NewByteStringToXmlNode(mxml_node_t *xmlNode, char *key, uint8 *flag);
int LoadAllInvoiceTypeProperty(HDEV hDev);
bool IsPrivateIPv4(char *szIP);
int GetSignatureCert(X509 *pX509Cert);
void Free2dArray(uint8 *array[], int nArrayNum);
void Zyfplx2String(bool bStr2Byte, char *szStr, uint8 *pbZyfplxByte);
int Domain2IP(char *host, char *ip);
void SetSocketOption(int s, int nTimeoutSec);
bool CheckFphmRangeIncorrect(char *szFPHMHead, char *szFPHMTail, int nWantCount);
int CalcAllocBuffLen(HFPXX hFpxx);
bool CheckDeviceVersionMatch(HDEV hDev, char *szOutDeviceDeclareVersion);
void GetOSTime(char *szOutTime);
int EtaxSM4Algorithm(bool bEncOrDec, int padding, uint8 *pSrc, int nSrcLen, uint8 *abIV, uint8 *pOutBuf);
int AesAlgorithm(bool bEncOrDec, const EVP_CIPHER *cipher, int padding, uint8 *todoData,
                 int nTodoDataSize, uint8 *inKey, uint8 *abOut);
int GzipDecompress(struct evbuffer *evbuf);
void GetRandHexString(char *szOut, int nInNeedLen, int nMod, char *szSeedString);
void GetRandString(char *szOut, int nInNeedLen);
int CalcTaxratePrice(bool bPriceWithTax, char *szDJ, char *szSLV, int nRoundSize, char *szOutPrice);
int GetRandNum(int nMin, int nMax);
#endif