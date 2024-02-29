#ifndef STATE_INFO_H
#define STATE_INFO_H
#include "aisino_common.h"

#define STATEINFO_FPXE_MAXCOUNT 8

struct stFPXE {
    uint8 bFPLX;
    char szOffLineAmoutLimit[64];          //���߽��    OffLineAmoutLimit
    char szMonthAmountLimit[64];           //�¿�Ʊ�޶� MonthAmountLimit
    char szInvAmountLimit[64];             //���ſ�Ʊ�޶� InvAmountLimit
    char szReturnAmountLimit[64];          //��֪��ʲô�޶� ReturnAmountLimit
    char szOffLineAmoutLimit_Surplus[64];  //����ʣ����

    char szBsDeadline[20];     //������ֹ����
    char szKpDeadline[20];     //��Ʊ��ֹʱ��
    char szBsLastestDate[20];  //���±�˰����
    char szBsBeginDate[20];    //��˰��ʼ����
};

struct stStateInfo {
    ushort IsLockReached;     //�Ƿ�����
    ushort IsRepReached;      //�Ƿ񵽱�˰��
    ushort LockedDays;        //��������-17������
    ushort ICCardNo;          //�ֻ�����
    char szRegCode[12];       //�������
    uint8 abInvTypeList[16];  //������1��+��Ʊ���ʹ���...
    //ʡ���ˣ�����������
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
