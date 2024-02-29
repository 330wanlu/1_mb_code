#ifndef _DEBUG_H
#define _DEBUG_H
#include "common.h"
//##-------���Ժ궨��
#ifndef _NODEVICE_TEST
//#define _NODEVICE_TEST  //���ô˺궨����������̲��ԣ���֤��Ʊ���ݡ����Է�Ʊ�ָ����㷨����ȵ�
#endif

struct stTestData {  //����Ĳ��ֳ�Աδ�ر���ʼ��������ʱ��΢ע����
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