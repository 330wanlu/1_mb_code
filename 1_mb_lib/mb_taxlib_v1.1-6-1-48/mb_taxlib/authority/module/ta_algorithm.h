
#ifndef TA_ALGORITHM_H
#define TA_ALGORITHM_H
#include "../ta_iface.h"

char *GetSignString(HFPXX stp_fpxx, int *pnRetBufLen);
char *GetDeviceSignHashBase64(HDEV hDev, const char *sSignStrGBKBuff, int nGBKBuffLen);
int BuildClientAuthCode(uint8 *pRepRadom1, int nRepRadom1Size, uint8 *pOutBuf, int *pnOutSize,
                        HDEV hDev);
size_t HtmlEscape(uint8 **dest, const uint8 *src, size_t size);
int GetTaClientHello(uint8 bNeedType, HDEV hDev, char *szOutBuf);
char *UriEncode(const char *);
char *UriDecode(const char *);

#endif