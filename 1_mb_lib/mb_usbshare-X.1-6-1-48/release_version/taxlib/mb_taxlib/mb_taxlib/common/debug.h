#ifndef _DEBUG_H
#define _DEBUG_H
#include "common.h"
//##-------调试宏定义
#ifndef _NODEVICE_TEST
//#define _NODEVICE_TEST  //启用此宏定义可用于无盘测试，验证开票数据、测试发票恢复、算法逆向等等
#endif

struct stTestData {  //这里的部分成员未必被初始化，测试时稍微注意下
    HDEV hDev;
    char szFPDM[32];  // AN
    char szFPHM[32];  // AN
};
typedef struct stTestData *PTData;

int _PrintfHexBuf(uint8 *pBuff, int nBuffLen);
int _WriteHexToDebugFile(char *szName, uint8 *pBuff, int nBuffLen);
int _ReadHexFromDebugFile(char *szName, uint8 *pBuff, int nBuffLen);
int ReadBinFile(char *szPath, void **pBuff, int nBuffLen);
int ReadStrFile(char *szPath, void **pBuff, int nBuffLen);
int WriteBinFile(char *szPathName, uint8 *pBuff, int nBuffLen);
int SaveNoDeviceJson(HDEV hDev, uint8 bCommonFplx);
int LoadNoDeviceJson(HDEV hDev);
void SaveFpxxToDisk(HFPXX fpxx);
void SaveFpxxByDMHM(HDEV hDev, char *szFPDM, char *szFPHM);

#endif  //_DEBUG_H