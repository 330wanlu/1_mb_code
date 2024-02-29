﻿#ifndef AISINO_COMMON_H
#define AISINO_COMMON_H

#include "../../common/common.h"
#include "../usb/aisino_usbio.h"
#include "aisino_algorithm.h"
#include "aisino_getinvoice.h"
#include "aisino_getmonitor.h"
#include "aisino_makeinvoice.h"
#include "aisino_netinvoice.h"
#include "aisino_report_clear.h"
#include "aisino_uploadinvoice.h"
#include "../../common/mbdiff.h"
//----------------------aisino公用全局变量声明定义
const static char defAisino3SplitFlag[] = "#$`^";

//内部版本号 V2.2.34.190919 V2.3.11.200630 V3.2.00.210930 V3.2.00.220107
/*V3.2.00.220328 V3.2.00.220507 V3.2.00.220624  V3.2.00.220928  V3.2.00.221028  V3.2.00.221129 V3.2.00.230109 V3.2.00.230224 V3.2.00.230227*/

const static char defAisinoInnerVersion3[] = "V3.2.00.231228";
//"V3.2.00.230328";

// 内部版本号 2.0最后版本号，不再改动
const static char defAisinoInnerVersion2[] = "V2.3.11.210102";
//税务总局版本号航信版
// V2.0.33_ZS_20190919, V2.0.35_ZS_20200228, V2.0.37_ZS_20200430, V2.0.39_LS_20200630,
// V2.0.39_LS_20200630 V2.0.44_ZS_20210930 V2.0.47_ZS_20220331 V2.0.49_ZS_20220630 V2.0.50_ZS_20220930   V2.0.51_ZS_20221031 V2.0.52_ZS_20221130 V2.0.54_ZS_20230109
// V2.0.55_ZS_20230228
const static char defTaxAuthorityVersionAisino[] = "V2.0.64_ZS_20231228";
//"V2.0.56_ZS_20230331";

//----------------------aisino公用函数声明
int AisinoGetDeivceTime(HUSB hUSB, char *szOutTime);
int AisinoFillExtraDeviceInfo(HDEV hDev);
uint8 MakePackVerityFlag(uint8 *pData, int nDataLen);
int AisinoClose(HUSB hUSB);
int AisinoOpen(HUSB hUSB);
int AisinoReadTaxID(HUSB husb, char *szTaxID);
int AisinoReadTaxIDandCustomName(HUSB hUSB, char *szTaxID, char *szComName, size_t nComNameBufLen);
uint8 GetTrainDeviceType(HUSB hUSB, HDEV hDev);
int SignAPI_OpenDevice(HUSB hUSB, bool bBigCryptIO, char *szPassWord);
int AisinoEnumCertsMem(HUSB hUsb, void *pRetBuff, bool bBigCryptIO);
int ReadTaxDeviceID(HUSB hUSB, char *szDevID);
int InvSignStrToDev128ByteRep(HUSB hUSB, bool bBigCryptIO, const char *sSignStrGBKBuff,
                              int nGBKBuffLen, char *pRecvBuff, int nRecvBuffLen);
int GetDriverVersion(HUSB hUSB, char *szOutVersion, uint32 *nNumVerion);
int GetJMBBH(HUSB hUSB, char *szJMBBH);
int CommonFPLX2AisinoFPLX(uint8 fplxdm, uint8 *aisino_fplxdm);
int AisinoFPLX2CommonFPLX(uint8 aisino_fplxdm, uint8 *fplxdm);
int AisinoDeviceEffectDate(HUSB hUSB, char *szOutTime);
int AisinoDevSelectFileCMD(HUSB hUSB, uint8 *abCmd, int nCmdLen, bool bBigCryptIO);
int AisinoCryptSimpleTransferPacket(uint8 *ab3BytesCmd, uint8 *pData, int nDataLen,
                                    uint8 *abOutBuf);
int SignAPI_ChangePassword(HUSB hUSB, bool bBigCryptIO, char *szOldPassWord, char *szNewPassWord);
int AisinoStateInfoToHDEV(HDEV hDev);
void GetHashTaxCode(char *szHashTaxCodeOut, char *sz9BitTaxCodeIn);
int Get9BitHashTaxCode(HUSB hUSB, char *sz9BitTaxCode);
int GetCompressTaxCode(HUSB hUSB, char *szCompressTaxCode);
bool IsAisino3Device(int nVersionNum);
void GetAisinoKPRJVersion(int nVersionNum, char *szOutStr);
bool GetAisino3NodeValue(char *szOldTypeStringLine, char *szOut, bool bRemoveVauleFromLine);

#endif