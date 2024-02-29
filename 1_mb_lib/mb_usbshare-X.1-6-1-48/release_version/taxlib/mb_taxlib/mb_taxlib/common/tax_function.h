#ifndef TAX_FUNCTION_H
#define TAX_FUNCTION_H
#include "tax_common.h"

void GetNicMAC(char *szTaxID, char *szOut, uint8 bSplitFormat);
void GetMotherBoardId(char *szTaxID, char *szOut);
int GetSelfAbsolutePath(char *buf, int nBufflen);
int TranslateStandTime(int nType, char *szStandTimeIn, char *szOutTime);
void UpdateDevInfoToFPXX(HDEV hDev, HFPXX fpxx);
void PriceRound(char *szPrice, int nRoundSize, char *szOutPrice);
int PriceRemoveTax(char *szDJ, char *szSLV, int nRoundSize, char *szOutPriceWithTax);
int GetCertSNByteOrStr(X509 *pCert, char *retSN, uint8 bRetBinOrStr);
int TimeOutMonthDeadline(char *szBaseTime, char *szTimeDev);
int MinusTime(char *szTime1, char *szTime2);
int CalcDecodeLength(const char *b64input);
void GetFotmatJEStr(char *je, char *szOut);
int GetFotmatJE100(char *je);
int GetSpsmmcSpmc(char *szSmmcMc, char *spsmmc, char *spmc);
int CheckAndAppendCertBuf(uint8 *pRetBuff, int nRetBuffIndex, uint8 *pCertBuff, int nCertLen);
void GetBJSJ(char *szDeviceTime, char *szOut, uint8 bStyle);
int TAXML_Formater(mxml_node_t *root, char *szOut);
void UpdateFpxxAllTime(char *szStandTimeInput, HFPXX fpxx);
int EnumAllUSBDevice_CallBack(int nDeviceType, HUSB hUSB);
bool IsTheSameMonth(char *szTime1, char *szTime2);
bool SetProxy(HDEV hDev, char *szProxyStr);
void DevIOCtrl(HDEV hDev, bool bStatus);
uint8 StaticCheckTrainDeviceByDeviceID(char *szDeviceID);
int PasswordNotice(bool bDeviceOrCert, char *szLastErrorDescription);
int SetLastError(HUSB hUSB, enum ErrorCode bErrorCode, char *msg, ...);
int GetLastErrorType(enum ErrorCode bErrorCode);
void ClearLastError(HUSB hUSB);
int GetHostAndPortFromEvUri(struct evhttp_uri *uri, char *szHostOut, int *nPortOut);
int LoadTaxDevice(int bUsbType, HDEV hDev, HUSB hUSB, uint8 bAuthDev);
HDITI GetInvoiceTypeEnvirnment(HDEV hDev, uint8 bCommmonInvType);
mxml_node_t *NewByteStringToXmlNode(mxml_node_t *xmlNode, char *key, uint8 *flag);
int LoadAllInvoiceTypeProperty(HDEV hDev);
bool IsPrivateIPv4(char *szIP);
int GetSignatureCert(X509 *pX509Cert);
void Free2dArray(uint8 *array[], int nArrayNum);
int Domain2IP(char *host, char *ip);
void SetSocketOption(int s, int nTimeoutSec);
bool CheckFphmRangeIncorrect(char *szFPHMHead, char *szFPHMTail, int nWantCount);
int CalcAllocBuffLen(HFPXX hFpxx);
bool CheckDeviceVersionMatch(HDEV hDev, char *szOutDeviceDeclareVersion);
void GetOSTime(char *szOutTime);
int JudgeDecimalPoint(char *pc_jg, int min_num, int max_num);
int FreeFpxx(HFPXX fpxx);
HFPXX MallocFpxx();
HFPXX MallocFpxxLinkDev(HDEV hDev);
int InsertMxxx(HFPXX fpxx, struct Spxx *newSpxx);
bool CheckTzdh(char *szRedNum, char *szInvFlag);
enum enSPLX IdentifyProductBySpbm(struct Spxx *SpxxIn);
bool CheckInvoiceCanbeWaste(HFPXX fpxx);
int TaxMassStorageIo(HUSB hUSB, uint8 *pSendCmd, int nCmdLen, int UMSHeader_nDataTransferLen,
                     uint8 UMSHeader_direction, uint8 *pTransferData, int nTransferLen,
                     uint8 *pRecvDataBuff, int nRecvDataBuffLen);
uint64 GetMicroSecondUnixTimestamp();

#endif