#ifndef AISINO_GETINVOIVE_H
#define AISINO_GETINVOIVE_H
#include "aisino_common.h"

struct STSPXXv2 {
    char SPMC[92];
    char GGXH[40];
    char JLDW[22];
    char SL[21];
    char DJ[21];
    char JE[18];
    char SLV[6];
    char SE[18];
    char NUM[8];
    char FPHXZ[3];
    char HSJBZ[3];
};

//老版本STSPXX结构体
//商品明细，GBK格式化结构体,该结构体每个变量长度为固定值，不能修改，否则KP错误
struct STSPXXv3 {
    char SPMC[100];
    char GGXH[40];
    char JLDW[22];
    char SL[21];
    char DJ[21];
    char JE[18];
    char SLV[6];
    char SE[18];
    char NUM[8];
    char FPHXZ[3];
    char HSJBZ[3];
    char FLBM[30];  //此处的分类编码flbm对应我方商品spbh，下方的真实商品编码废弃
    char SPBH[30];  // SPSM 商品税目编码,官方保留字段,目前为空
    char XSYH[3];
    char YHSM[50];
    char LSLVBS[3];
};

int DecodeInvoiceBlock1(uint8 *pInv, int nInvBuffLen, char *szRet);
int DecodeInvoiceBlock2(uint8 *pInv, int nInvBuffLen, char *szRet);
int DecodeInvoiceBlock3(HDEV hDev, uint8 *pRawInv, int nRawInvBuffLen, uint8 *pBlock1,
                        uint8 *pBlock2, uint8 *pBlock3);
int GetInvMonthTotal(HUSB hUSB, char *szFormatYearMonth);
int GetInvDetail(HDEV hDev, int nInvoiceNb, char *szFormatYearMonth, uint8 **ppInBuff,
                 int *pnBuffLen, uint8 *aisino_fplx);
int GetMWJYMFromRawInvoice(HDEV hDev, char *abRawInvoice, char *abDecInvoice, char *pBlock2,
                           char *szOutMW, char *szOutJYM);
int DecodeInvBuf(HDEV hDev, uint8 *pInBuf, int nInBufLen, uint8 *pOutBuf);
int GetFirstOfflineInv(HDEV hDev, char *szOutFPDM, char *szOutFPHM, int *dzsyh, char *kpsj);
int QueryInvInfo(HDEV hDev, char *szTypecode, char *szNumer, uint32 nDZSYH, HFPXX pOutFpxx);
int WasteInvoice(HFPXX fpxx);
int AisinoInvRaw2Fpxx(uint8 *byte_2, HFPXX fpxx);
void GetQueryMonthFormat(uint8 nCentury, uint8 nYear, uint8 nMonth, uint8 bChildQuery,
                         char *szOutBuff);
uint8 SplitInvoiceDetail(uint8 *invoice_data, uint8 *myarray[]);
int AisinoGetTailInvVersion(HDEV hDev, char *szOutVersion);

#endif
