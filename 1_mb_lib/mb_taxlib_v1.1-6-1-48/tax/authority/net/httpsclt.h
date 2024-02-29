
#ifndef HTTPSCLT_H
#define HTTPSCLT_H
#include "../ta_iface.h"

//初始化函数
int HTTPInit(HHTTP hi, HDEV hDev);
int HTTPOpen(HHTTP hi);
void HTTPClose(HHTTP hi);
//传统接口，提供固定长度缓冲区并读取到缓冲
int HTTPPost(HHTTP hi, char *szQueryPath, char *pData, int nDataLen, char *response,
             int repBufferLen);
//不定长读取缓冲接口，读取的数据在libevent的buferevent中
int HTTPPostEvent(HHTTP hi, char *szQueryPath, uint8 *data, int nDataSize);
int HTTPGetEvent(HHTTP hi, char *szQueryPath);
//内部导出使用
void GetHttpReponseCallback(struct evhttp_request *req, void *ctx);


//全电接口
int EtaxHTTPInit(EHHTTP hi, HDEV hDev);
int EtaxHTTPOpen(EHHTTP hi);
void EtaxHTTPClose(EHHTTP hi);
int EtaxHTTPPostEvent(EHHTTP hi, char *szQueryPath, char *szContentType, uint8 *data, int nDataSize);
int EtaxHTTPGetEvent(EHHTTP hi, char *szQueryPath);
int EtaxHTTPGetEventdown(EHHTTP hi, char *szQueryPath);
void EtaxGetHttpReponseCallback(struct evhttp_request *req, void *ctx);
int EtaxHTTPPost(EHHTTP hi, char *szQueryPath, char *szContentType, char *pData, int nDataLen,char *response, int repBufferLen);
void EtaxRemoveRepeatOldCookies(struct evbuffer *evBufCookies);
#endif