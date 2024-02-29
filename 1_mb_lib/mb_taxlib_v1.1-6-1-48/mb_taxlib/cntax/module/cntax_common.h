#ifndef CNTAX_COMMMON_H
#define CNTAX_COMMMON_H
#include "../../common/common.h"
#include "../../nisec/module/nisec_common.h"  // cntax ukey depend nisec's function
#include "cntax_algorithm.h"
#include "cntax_getinvoice.h"
#include "cntax_getmonitor.h"
#include "cntax_makeinvoice.h"
#include "cntax_netinvoice.h"
#include "cntax_report_clear.h"
#include "cntax_uploadinvoice.h"

//˰���ְܾ汾��ukey�� V1.0.13_ZS_20210930 V1.0.16_ZS_20220331 V1.0.17_ZS_20220429  V1.0.18_ZS_20220630  V1.0.19_ZS_20220930  V1.0.20_ZS_20221031 V1.0.23_ZS_20230109
//V1.0.24_ZS_20230228
const static char defTaxAuthorityVersionCntax[] = "V1.0.33_ZS_20231228";
//"V1.0.25_ZS_20230331";


int CntaxGetBasicInfo(HDEV hDev, char *szOutSN, char *szOutTaxID, char *szOutCusName);
int LoadCntaxDevInfo(HDEV hDev, HUSB hUSB, bool bAuthDev);
int CntaxEntry(HUSB hUSB, uint8 bOPType);
int CntaxEntryRetInfo(HUSB hUSB, uint8 bOPType, uint8 *pDeviceOpenInfo);
int CntaxSelectInvoiceTypeMini(HUSB hUSB, uint8 bInvoiceType);
int CntaxSelectInvoiceType(HDEV hDev, uint8 bInvoiceType, enum SELECT_INVOICE_OPTION option);
int CntaxEnumCertsMem(HUSB hUSB, void *pRetBuff);
int Cntax_Crypt_Password(HUSB hUSB, char *szDeviceID, char *szCertPassword,
                         char *szNewCertPassword);
int CntaxChangeCertPassword(HUSB hUSB, char *szDeviceID, char *szOldPassWord, char *szNewPassWord);
int CntaxChangeDevicePassword(HUSB hUSB, char *szOldPassWord, char *szNewPassWord);
int CntaxCryptLogin(HUSB hUSB, char *szDeviceID);
int CntaxGetDeviceTime(HUSB hUSB, char *szOutTime);
int CntaxGetDeviceFsyz(HUSB hUSB, char *szOutFsyz);
void Transfer10ByteToFPDMAndFPHM(uint8 *pInBuf, char *szInOutFPDM, char *szInOutFPHM);
void TransferFPDMAndFPHMTo10Byte(char *szInFPDM, char *szInFPHM, uint8 *pOutBuf);
int CnTaxGetDeviceEnv(HDEV hDev, HUSB hUSB, uint8 *abDeviceOpenInfo);
char *CntaxGetUtf8YwxxStringFromXmlRoot(mxml_node_t *root);
int GetCntaxInvoiceTypeProperty(uint8 bInvType, HUSB hUSB, HDITI hDiti);
uint8 CntaxGetInvTemplateValue(uint8 *abInvTemplate, uint8 bType, uint8 bIndex);

#endif