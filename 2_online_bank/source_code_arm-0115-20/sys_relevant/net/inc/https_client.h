#ifndef _HTTPS_CLIENT_H
#define _HTTPS_CLIENT_H

#include "event2/http.h"
#include "event2/http_struct.h"
#include "event2/event.h"
#include "event2/buffer.h"
#include "event2/dns.h"
#include "event2/thread.h"
#include "event2/bufferevent_ssl.h"
#include "event2/keyvalq_struct.h"
#include <event2/event-config.h>
#include <event2/util.h>
#include <event2/bufferevent.h>

#include <openssl/err.h>
#include <openssl/ssl.h>
#include <openssl/rand.h>
#include <openssl/crypto.h>
 
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <sys/queue.h>
#include <event.h>

#include <stdbool.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fcntl.h>
#include <sys/time.h>

#define  BUF_MAX 1024*512

typedef struct TagCallBackparm
{
	struct event_base* base;
	void* args;
	int* result;
}CallBackparm;


typedef void (*FunRemoteReadCallback)(struct evhttp_request* remote_rsp, void* arg);

int https_post(char* url,char* body,int body_len,char* content_type,char* cookie,FunRemoteReadCallback call_back,void* args,int* result);
int https_get(char* url,char* body,int body_len,char* content_type,char* cookie,FunRemoteReadCallback call_back,void* args,int* result);
char *http_find_header(struct evhttp_request* remote_rsp, const char *key);

#endif //_HTTPS_CLIENT_H
