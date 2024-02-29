#ifndef AISINO_NETINVOICE_H
#define AISINO_NETINVOICE_H
#include "aisino_common.h"

struct InvoiceVol {
    bool bDownloadOrUnlock;
    char szFPZL[8];
    char szTypeCode[24];
    char szFPHMHead[24];
    char szFPCount[24];
    char szB64Buf[128];      // fpsq-b64 or unlockmw-b64
    uint8 bFPZLOperateType;  //发票种类操作类型 专普票为1，电票为4
};

// ZZS step1;
int NetInvoice_QueryReady(HDEV hDev, char *szInputQueryDateRange, char *szOutFormatStrMultiLine);
// ZZS step2; DZ step1
int NetInvoice_DownloadUnlock(HDEV hDev, char *szInputFormatStrSingleLine);
int UpdateCompanyInfo(HUSB hUSB, char *szNewCompanyInfoB64);
int RedInvOnlineCheck_PTDZ(HDEV hDev, uint8 bAisinoFPLX, char *szLZFPDM, char *szLZFPHM,
                           uint32 nDZSYH, char *szOutMsg, int nOutMsgBuffSize);
int RedInvOnlineCheck_ZY(HFPXX fpxx, uint8 **inv_json);

// mengbai USED need declare
int ReadInvGetAuthorization(HUSB hUSB, char *szOutInfo);
int NetInvoice_DownloadOrUnlockAPI(HHTTP hi, struct InvoiceVol *pInvv);
int NInvGetUnlockInvoice(HHTTP hi, struct InvoiceVol *pInvv);
int HZXMLNode2FPXX(mxml_node_t *pXMLChildRoot, HFPXX fpxx);
int GetFpxxFromRepByRedNum(char *szRep, char *szRedNum, HFPXX fpxx);
int QueryNetInvoiceReady(HHTTP hi, char *szQueryDateRange, char *szOutFormatStr);
int ClearLockedInvoiceVolume(HHTTP hi);
int DownloadSpecifyInvoice(HHTTP hi, char *szInFormatStr);
#endif
