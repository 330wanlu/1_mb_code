#ifndef NISEC_COMMMON_H
#define NISEC_COMMMON_H
#include "../../common/common.h"
#include "../usb/nisec_usbio.h"
#include "nisec_algorithm.h"
#include "nisec_getinvoice.h"
#include "nisec_getmonitor.h"
#include "nisec_makeinvoice.h"
#include "nisec_netinvoice.h"
#include "nisec_report_clear.h"
#include "nisec_uploadinvoice.h"

//˰���ְܾ汾�Ű�����
// V2.0.48_ZS_20220429 V2.0.49_ZS_20220630 V2.0.50_ZS_20220930   V2.0.51_ZS_20221031 V2.0.52_ZS_20221130 V2.0.54_ZS_20230109
//V2.0.55_ZS_20230228
const static char defTaxAuthorityVersionNisec[] = "V2.0.64_ZS_20231228";
//"V2.0.56_ZS_20230331";
//�������ںͺ���һ��������һ���ڲ��İ汾�ţ��ƺ�ֻ���ڲ�����ʹ�ã�������ǲ�����ע

enum enumNisecOPType { NOP_CLOSE, NOP_OPEN, NOP_OPEN_FULL };
int NisecOpen(HUSB hUSB, char *szDevicePw);
int NisecClose(HUSB hUSB);
int NisecSelectInvoiceType(HDEV hDev, uint8 bInvoiceType, enum SELECT_INVOICE_OPTION option);
int NisecSelectInvoiceTypeMini(HUSB hUSB, uint8 bInvoiceType);
int NisecGetBasicInfo(HDEV hDev, char *szOutSN, char *szOutTaxID, char *szOutCusName);
int LoadNisecDevInfo(HDEV hDev, HUSB hUSB, uint8 bAuthDev);
int NisecEntry(HUSB hUSB, uint8 bOPType);
int CommonGetCurrentInvCode(HDEV hDev, char *szInOutFPDM, char *szInOutFPHM,
                            char *szOptOutInvEndNum);
uint8 CheckDeviceSupportInvType(uint8 *abSupportInvtypeBuf, uint8 bInvType);
void FillAllInvType(HDEV hDev, uint8 *cb);
int NisecGetDeviceTime(HUSB hUSB, char *szOutTime);
int NisecGetDeviceFsyz(HUSB hUSB, char *szOutFsyz);
int NisecEnumCertsMem(HUSB hUSB, void *pRetBuff);
void Transfer9ByteToFPDMAndFPHM(uint8 *pInBuf, char *szOutFPDM, char *szOutFPHM);
void TransferFPDMAndFPHMTo9Byte(char *szInFPDM, char *szInFPHM, uint8 *pOutBuf);
int EncryptOpenDevice(HUSB hUSB, uint8 bDeviceType, char *szDeviceID);
int Nisec_Crypt_Login(HUSB hUSB, char *szDeviceID);
int NisecChangeCertPassword(HUSB hUSB, char *szDeviceID, char *szOldPassWord, char *szNewPassWord);
int FormatTemplate(bool bUtf8Buf, char *pDetailBuff, int nIndex, char *pOutBuf, int nOutBufSize);
int GetDiskEnvFromManageData(HDEV hDev);
uint8 GetDeviceBBH(char *szDeviceVersion);
uint8 NisecGetInvTemplateValue(uint8 *abInvTemplate, uint8 bType, uint8 bIndex);
int NisecBaseStrExpand(uint8 *abInvTemplate, char **pszStr, int *pnStrBufLen, int nIndex);
void RemovePrefixTwoZero(char *szInput);
void NisecFPZTAnalyze(int nFPZT, HFPXX fpxx);
void DetectSpecialFplxFromBZ(HFPXX fpxx);
void Build8BytesRandomTQM(char *szOutBuf);
int GetDeviceCert(HDEV hDev);
int CheckInvoiceExistAtDevice(HDEV hDev, char (*dst)[768]);
void GetNatureOfTaxpayerStr(uint8 bNatureOfTaxpayer, char *szOutString);
int GetTaxrateStatic(HDEV hDev, uint8 bInvType, bool hsOrbhs, char *szOut);
int GetTaxrate(char *szInTaxrateConfig, bool hsOrbhs, char *szOut);
int GetNisecInvoiceTypeProperty(uint8 bInvType, HDEV hDev, HDITI hDiti);
bool GetDeviceQmcs(HDEV hDev, char *szOut);

#endif
