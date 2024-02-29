
#ifndef TRANSFER_H
#define TRANSFER_H
#include "../ta_iface.h"

int InitSSL(HHTTP hi);
void FreeSSL(SSL_CTX *ctx, BIO *bio);
bool SetProxy(HDEV hDev, char *szProxyStr);
int EventConnect(HHTTP hi);
int EventInit(HHTTP hi);
void EvenPendingWithBreakSingal(HHTTP hi);


int EtaxEventInit(EHHTTP hi);
int EtaxConnectIP(EHHTTP hi, char *szIP);
int EtaxEventConnect(EHHTTP hi);
void EtaxEvenPendingWithBreakSingal(EHHTTP hi);
void EtaxDnsResolvCallback(int errcode, struct evutil_addrinfo *addr, void *ptr);
int EtaxInitSSL(EHHTTP hi);
#endif