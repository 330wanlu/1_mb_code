#ifndef AISINO_ALGORITHM_H
#define AISINO_ALGORITHM_H
#include "aisino_common.h"

int InvEncryptBlock2(BYTE *pthis, long *pDecodeBuff, _DWORD dwDecodeBuffLen, long *szRet);
int DecryptBlock1(long a1, long szRet, long pInv, int nDecInvoiceBlock2Len);
int GetInvNumLoop(uint8 *cb, int nTypeCount, uint8 *szRetBuff);
signed int sub_41D9EE0(signed int a1, int a2);
uint8 sub_41D5430(uint8 a1);
uint8 sub_476AE90(unsigned __int8 a1);
int DecodeMWandJYM(uint8 bCondition, bool bIsMulitTaxRate, bool bNCP_SG, uint8 *pInDeviceMWJYMBuff,
                   char *szCompressTaxID, char *szGFSH, char *szXFSH, char *szOutMW,
                   char *szOutJYM);
signed int BuildInv0x40(const void *a2, uint8 *a3);
signed __int64 JE2InvRaw(long a1, double a2, int a3);
int Makehashcode_GenProKey(uint8 *bufOut, uint8 *bufIn);
int Build8BitMakeInvVCode(long a1, long a2, _BYTE *a3, signed int a4);
uint8 DeviceTimeRandomSeed(long a1, long a2, int a3);
DWORD InvDecryptBlock2(uint8 *pthis, long *pEncodeBuff, _DWORD dwEncodeBuffLen, long *szRet);
void method_61(uint8 *byte_3, char *szInvTypeCode);
void method_64(uint8 *pByteIn, char *szInvTypeCode);
int InvTypeCode2Byte5(uint8 *szOut, char *szIn);
int TaxID212Bytes(char *szTaxID, uint8 *abOut);
int Get25ByteXor(bool bIsMulitTaxRate, bool bNCP_SG, uint8 *ab25, char *szCompressID6,
                 char *GFTaxID, char *XFTaxID);
signed int sub_10034160(long this);
uint8 SetPreInvoiceBuff0(float je);
// Start----------------------------------------发票第一段解密算法----------------------------------------
int sub_39E2630(uint8 a2, uint8 a3, unsigned __int8 a4);
int sub_39E2830(long a1, long a2);
int sub_39D5EE0(unsigned __int8 a1);
int sub_39D5E90(long a1, long a2, int a3);
signed int sub_39DB320(uint8 *pthis, long a2, long a3);
int Block1PreDecrypt(long a1, long a3, long a4);
long UnMaskBlock1Number(long a1, _BYTE *a2, uint8 a3);
// Finish----------------------------------------发票第一段解密算法----------------------------------------
// Start----------------------------------------发票第二段解密算法----------------------------------------
int My_Func_5B441F0(_WORD *this, int a2, unsigned __int16 a3);
int Block2_WriteblockToDstbuff(long this, long a2, int a3);
__int16 Block2DecodeBuff(long this, uint8 a2);
__int16 My_Func_5B44620(long this, long a2, unsigned int a3);
uint8 My_Func_5B44110(long this, _WORD *a2);
uint16 My_Func_5B44260(long a1);
signed int Block2_ZeroVar(BYTE *this);
__int16 My_Func_5B43FD0(long this);
// Finish----------------------------------------发票第二段解密算法----------------------------------------
signed int sub_10037260(long this);
int sub_10037370(_BYTE *this, int a2, int a3);
__int16 sub_100372E0(long this, int a2);
//---------------------------开票加密USBIO第二块 结束-----------------------
//---------------------------密文、校验码解密函数 开始-----------------------
int sub_10038990(uint8 *a1, int a2);
int sub_10038750(unsigned __int8 a1);
int sub_10038B00(uint8 *pOut, uint8 *sTaxID, int nTaxLen);
signed __int16 JYMMW_Head12Byte(uint8 *pOut, uint8 *sTaxID, int nTaxLen);
uint8 sub_10006230(unsigned __int8 a1);
int sub_10006290(unsigned __int8 a1);

//---------------------------密文、校验码解密函数 结束-----------------------
//---------------------------开票数据包结束处8位验证码函数 开始-----------------------
int sub_10033160(uint8 *this);
signed long sub_10033040(uint8 *this);
signed long sub_100330D0(uint8 *this);
uint8 sub_10032BF0(uint8 *this, long a2);
int sub_10032DA0(uint8 *this, _BYTE *a2, uint8 *a3);
int sub_10032F20(uint8 *this, unsigned int a2, long a3);
unsigned int sub_10032E60(uint8 *this, _BYTE *a2, long a3);
int sub_10032CE0(void *this, long a2, long a3);
//---------------------------开票数据包结束处8位验证码函数 结束-----------------------
uint8 sub_10039420(char *a1, unsigned __int8 a2);
unsigned int sub_100341E0(_DWORD *this, int a2, _DWORD a3, _DWORD a4, _DWORD a5, signed int a6);
uint8 *sub_10034010(uint8 *this);
uint16 MakeJESLSE_VCodeByte(uint8 bAisinoFPLX, uint8 zyfpLx, double fJE, double fSL, double fSE,
                            uint16 wPreStr0x100x11);
void GetInvTypeCode(int nInvNum, uint8 *pByte5InvTypeInBuff, char *szInvTypeCode);
int CalcDZSYH_method13(uint8 *szServerRep16Byte, int nByte0Len, int nDZSYH, uint8 *pOut);
int BuildPubServiceRandom(char *szFPDM, char *szFPHM, char *szKPTimeStandard, char *szOut8ByteStr);
int AisinoDesEcbEncrypt(uint8 *pInputBuf);
uint8 sub_8B1F970(uint8 a1);
int RestoreAisinoMonthStatisticApiBuf(uint8 *pInBuf, uint8 *pOutMem, int nOutMemSize);
void BuildRedTzdbh6Bytes(uint8 *pOut6BytesBuf, char *sz16BytesTzdbh);
int GetFpdmAndSpecialFlagFromBlock2(char *a1, char *a2);

#endif