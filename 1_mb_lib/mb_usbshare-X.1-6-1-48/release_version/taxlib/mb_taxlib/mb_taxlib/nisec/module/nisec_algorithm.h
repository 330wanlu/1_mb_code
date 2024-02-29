#ifndef NISEC_ALGORITHM_H
#define NISEC_ALGORITHM_H
#include "nisec_common.h"

void Num2FloatS(uint64 nNum, char *szRet);
int NisecBaseConv(const char *szInputStr, int nInputLen, uint8 bDstBase, char *szRetBuff,
                  uint8 bOrigBase, uint8 bRetStrLen);
int CalcNisecTaxID(uint8 *szUSBData, uint8 *szOutBuff);
uint16 NisecBuildVCode(uint8 *a1, int a2);
uint8 BuildTransferVCode(uint8 *a1, int a2);
uint16 BuildPreMW_String2Word(char *a1, int a2);
sint16 BuildTwoByteMWVcode(const void *a1, signed int a2);
int Build6ByteMWPreTail(uint8 bIsRed, char *kpsj_standard, char *szDeviceID,
                        char *szTaxAuthorityCodeEx, char *xfsh, uint8 *abOutBuf);
int NisecMWDec(uint8 a1, uint8 a2, uint8 bFlag0, uint8 *a4, uint8 *a5, uint32 a6, char *a7);
int JYMBinChange(uint8 *a1, int a1Len, uint8 *a3);
int NisecMWDecAPI(uint8 bFPLX_Common, bool bIsRed, char *szDeviceID, char *szTaxAuthorityCodeEx,
                  char *kpsj_standard, char *xfsh, uint8 *pabUSBMWRawIn_50Bytes, char *szOutMW);
int64 NisecDecodeNum(const char *pInputBuf, int nInbufLenMax);
int NisecFormatStr2Num(int nMethod, const char *pInputBuf, int nInbufLenMax, char *szOutBuf);
int DecodeTail(uint8 bInvoiceStatusFlag, uint8 *abInvTemplate, char *szDst, int nInputStrLen,
               int nBlockBinLen, int nSignStrLen, int *nNewLen);
int NisecJYMDecAPI(uint8 *pabUSBMWRawIn, int JymChangeSize, char *szOutJYM);
int Build16ByteCryptKey_DeviceOpen(char *prefixDevID, char *szDevIDInput, uint8 *abPreHashEnd16,
                                   uint8 *abCryptKey);
int SymEncrypt4(long a2, long a3, int a4, long pDst);
int SymEncrypt5(long a2, long a3, int a4, long pDst, long a6);
int Build16ByteCryptKeyAnd4ByteVCode(char *szDevIDEx, char *szCertPwd, uint8 *abPreHashBegin8,
                                     uint8 *ab16BytesCommand, uint8 *ab16BytesParament,
                                     uint8 *abOutCryptKey, uint8 *abOutUSBCmd4ByteVCode);
unsigned int sub_365670(long a1, unsigned int a2, _DWORD *a3);
void SpecialGfshRestore(char *szInput, int nInputSize);
void SpecialGfshChange(char *szInput);
int NisecCntaxLogBufXor(uint8 bVendorCode, uint8 bRawBufIndex1, uint8 *pInBuf, int nInBufSize,
                        uint8 *pOutBuf);
void NisecKjbbhEncrypt(char *szZjbbh, char *szFdm, char *szFphm, char *szDevId, char *szKjrq,
                       uint8 *abOutBuf);
void Get32bytesOperKeyVersionBuf_InputSeed(uint8 *a1);
int NisecEncryptMakeInvoiceHeadBuffer0x60Bytes(HDEV hDev, char *pBuf);
uint16 NisecBuildVCodeXorNum(uint16 vInputNum, uint8 *a1, int a2);
int Nisec3DesCbcEncrypt8Bytes(uint8 *toEncBuf8Bytes, uint8 *key, uint8 *outBuf8Bytes);
int NisecDriverVersionNumAbove1106(char *szDriverVersion);
int NisecBuildNewAuthKey(uint8 bVersion, char *szDeviceID, uint8 *szOutBuf);

#endif
