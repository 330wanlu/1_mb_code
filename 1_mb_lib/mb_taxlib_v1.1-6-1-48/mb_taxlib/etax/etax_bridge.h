#ifndef ETAX_BRIDGE
#define ETAX_BRIDGE
#include "etax_common.h"

//主干调用API
int CreateEtaxTalk(HHTTP hi, HDEV hDev);
int CreateEtaxTpassTalk(HHTTP hi, HDEV hDev, HHTTP hiEtax);
int TpassLogin(HHTTP hiTpass, HHTTP hiEtax, char *szUserIdIn, char *szPassword,
               char *szOptUserName);
//接口API
int EtaxGetBasicInformation(HHTTP hi, ETaxFlag flag);

#endif
