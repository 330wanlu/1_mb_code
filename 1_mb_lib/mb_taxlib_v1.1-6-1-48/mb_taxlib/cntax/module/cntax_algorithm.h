#ifndef CNTAX_ALGORITHM_H
#define CNTAX_ALGORITHM_H
#include "cntax_common.h"

int CntaxMWDecAPI(uint8 bInvType, bool bIsRed, char *szDeviceID, char *szTaxAuthorityCodeEx,
                  char *kpsj_standard, char *xfsh, uint8 *pabUSBMWRawIn_0x30Bytes, char *szOutMW);
int EncryptDecryptV2String(int nEncryptOrDecrypt, uint8 *abInputBuf, int nInputSize,
                           uint8 *abOutBuf);
int CalcSM3(uint8 *pSrc, int nSrcLen, uint8 *pOutDigest);
int CntaxBuild16ByteCryptKeyAnd4ByteVCode(char *szDevIDEx, char *szCertPwd, uint8 *abPreHashBegin16,
                                          uint8 *ab16BytesCommand, uint8 *ab16BytesParament,
                                          uint8 *abOutCryptKey, uint8 *abOutUSBCmd4ByteVCode);
int CallNisecCntaxDecryptLog(uint8 bVendor, char *szEncFilePath);
int SM4Algorithm(int nEncryptOrDecrypt, uint8 *pSrc, int nSrcLen, char *abIV, uint8 *pOutBuf);
int BuildCntaxPkcs11CallbackBuf(uint8 *abSM3HashRet, uint8 *dstBuf, int nDstBufSize);
void Get32bytesOperKeyVersionBuf(char *szDevId, uint8 *abOutBuf);
int GetAlgIndexFromUsbReponse(char *szDevId, uint8 *abUsbResponse);
void GetHeadBuffer0x60BytesFinalBuf(char *szDevId, int nTableIndex, uint8 *pOrig0xc0HexString,
                                    uint8 *pOutHex0xc0StringBuf);
int GetHead0x60DevIdSeed(char *szDevID, uint8 *abOutBuf);

#endif
