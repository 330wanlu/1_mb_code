#ifndef _HTTP_H
#define _HTTP_H

#include "opt.h"

#include "ghttp.h"

#define http_out(s,arg...) 		//log_out("main.c",s,##arg)

int netGet(char* url, char* params, int timeout, char *result, int *result_len);
int netPost(char* uri, char* params, int timeout, char *result, int *result_len);

#endif
