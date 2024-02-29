#ifndef TA_VENDOR_H
#define TA_VENDOR_H
#include "../ta_iface.h"

int AisinoBuildZZSXml(int nOpType, char *szDataPayload, HDEV hDev, char *pOutBuf);
int AisinoTechnologyXML(int nOpType, HDEV hDev, char *szOutBuf, int nSizeOutBuf);
int AisinoSyncCompanyInfo(HHTTP hi, bool bNeedUpdateIO);

int NisecBuildZZSXml(int nOpType, char *szDataPayload, HDEV hDev, char *pOutBuf);
int NisecTechnologyXML(int nOpType, HDEV hDev, char *pc_xml);
int NisecSyncCompanyInfo(HHTTP hi, bool bNeedUpdateIO);

int CntaxBuildTyxxXml(int nOpType, char *szYwxxPayload, HDEV hDev, char *pOutBuf, int nBusBufLen);
int CntaxTechnologyXML(int nOpType, HDEV hDev, char *pc_xml);
int CntaxSyncCompanyInfo(HHTTP hi, bool bNeedUpdateIO);

#endif
