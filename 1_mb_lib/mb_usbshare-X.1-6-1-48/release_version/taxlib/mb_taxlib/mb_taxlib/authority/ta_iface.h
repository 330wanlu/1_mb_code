
#ifndef TA_IFACE_H
#define TA_IFACE_H
#include "../common/common.h"
//包含问题,必须将子模块头文件放这里
#include "module/ta_algorithm.h"
#include "module/ta_confirm.h"
#include "module/ta_vendor.h"
#include "module/ta_vendor_common.h"
#include "module/ta_xml_func.h"
#include "module/ta_xml_iface.h"
#include "net/httpsclt.h"
#include "net/transfer.h"
#include "ta_xml.h"

int CreateServerTalk(HHTTP hi, HDEV hDev);
int ServerIO(HHTTP hi, char *szQueryPath, uint8 *pContent, char *pRepBuf, int nRepBufLen);
int GetTaServerURL(uint8 bServType, char *szRegCode6, struct UploadAddrModel *pstOutModel);
int GetEtaxServerURL(uint8 bServType, char *szRegCode6, struct EtaxUploadAddrModel *pstOutModel);
int PreMessageDigest(uint8 bDeviceType, char *szTechnologyXML, char *szBusinessContentXML,
                     char *szOutB64Digest);
int SyncCompanyInfo(HHTTP hi, bool bNeedUpdateIO);
int SyncWithTA(HDEV hDev);

#endif