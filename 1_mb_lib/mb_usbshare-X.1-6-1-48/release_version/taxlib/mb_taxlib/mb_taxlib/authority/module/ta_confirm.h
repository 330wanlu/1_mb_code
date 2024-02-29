#ifndef TA_CONFIRM_H
#define TA_CONFIRM_H
#include "../ta_iface.h"

enum ConfirmOpeartePublic {
    //如下为业务逻辑调用编号
    OPCONFIRM_SWSXTZS = 1000,  //系统辅助功能-税务事项通知书查询
    OPCONFIRM_FPXZ,            //发票管理-发票下载
    OPCONFIRM_HZFP,            //登陆提醒-红字发票查询
};
typedef enum ConfirmOpeartePublic ConfirmFlag;

int ConfirmLogout(HHTTP hi);
int ConfirmAPI(ConfirmFlag opFlag, HHTTP hi, char **retBuf, const char *msg, ...);
int CreateConfirmTalk(HHTTP hi, HDEV hDev);

int CreateChinataxLocalAuthServer(HDEV hDev);

int GetCookiesSegment(struct evbuffer *evBufCookies, char *szQueryKey, char *szOutValue,int nOutBufSize);

#endif
