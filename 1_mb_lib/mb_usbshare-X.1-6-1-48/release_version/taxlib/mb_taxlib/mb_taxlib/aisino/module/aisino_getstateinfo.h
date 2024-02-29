#ifndef STATE_INFO_H
#define STATE_INFO_H
#include "aisino_common.h"

#define STATEINFO_FPXE_MAXCOUNT 8

struct stFPXE {
    uint8 bFPLX;
    char szOffLineAmoutLimit[64];          //离线金额    OffLineAmoutLimit
    char szMonthAmountLimit[64];           //月开票限额 MonthAmountLimit
    char szInvAmountLimit[64];             //单张开票限额 InvAmountLimit
    char szReturnAmountLimit[64];          //不知道什么限额 ReturnAmountLimit
    char szOffLineAmoutLimit_Surplus[64];  //离线剩余金额

    char szBsDeadline[20];     //报送终止日期
    char szKpDeadline[20];     //开票截止时间
    char szBsLastestDate[20];  //最新报税日期
    char szBsBeginDate[20];    //报税起始日期
};

struct stStateInfo {
    ushort IsLockReached;     //是否锁定
    ushort IsRepReached;      //是否到报税期
    ushort LockedDays;        //锁定日期-17号左右
    ushort ICCardNo;          //分机号码
    char szRegCode[12];       //区域代码
    uint8 abInvTypeList[16];  //数量（1）+发票类型代码...
    //省事了，不作链表了
    struct stFPXE fpType[STATEINFO_FPXE_MAXCOUNT];
};
typedef struct stStateInfo StateInfo;

int GetStateInfo(HUSB device, StateInfo *pRetObj);
int GetInvStockBuf(HUSB device, unsigned char *szRetBuff, unsigned char *buf2);
struct stFPXE *FindStateFPXEByType(uint8 bInvType, uint8 bStaticMaxTypeCount, StateInfo *pSi);
bool ReadUnlockInvoiceFromDevice(HDEV hDev, bool bCheckOrRead);

//mengbai USED need declare
int GetStateInfoBuf(HUSB device, char *szRetBuff);
void PriceInt2DoubleStr(uint64 u64Price, char *szOut);
int NewDataModify(StateInfo *pRetObj, char *pNewCfgStr, int bStaticMaxTypeCount);
int AisinoGetOfflineAmount(HUSB device, uint8 bInvType, char *szOutInvTotalAmount,char *szOutInvAmount);
#endif
