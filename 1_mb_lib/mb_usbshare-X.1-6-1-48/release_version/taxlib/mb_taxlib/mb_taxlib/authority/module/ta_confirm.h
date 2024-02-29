#ifndef TA_CONFIRM_H
#define TA_CONFIRM_H
#include "../ta_iface.h"

enum ConfirmOpeartePublic {
    //����Ϊҵ���߼����ñ��
    OPCONFIRM_SWSXTZS = 1000,  //ϵͳ��������-˰������֪ͨ���ѯ
    OPCONFIRM_FPXZ,            //��Ʊ����-��Ʊ����
    OPCONFIRM_HZFP,            //��½����-���ַ�Ʊ��ѯ
};
typedef enum ConfirmOpeartePublic ConfirmFlag;

int ConfirmLogout(HHTTP hi);
int ConfirmAPI(ConfirmFlag opFlag, HHTTP hi, char **retBuf, const char *msg, ...);
int CreateConfirmTalk(HHTTP hi, HDEV hDev);

int CreateChinataxLocalAuthServer(HDEV hDev);

int GetCookiesSegment(struct evbuffer *evBufCookies, char *szQueryKey, char *szOutValue,int nOutBufSize);

#endif
