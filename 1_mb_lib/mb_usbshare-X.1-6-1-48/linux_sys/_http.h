#ifndef _HTTP_H
    #define _HTTP_H
    /*=================================�����ļ�============================================*/

    #include "ghttp.h"
    /*=================================����������==========================================*/
    #ifdef _HTTP_C
        #define e_http
        #define out(s,arg...) 							//log_out("main.c",s,##arg)                          
    #else
        #define e_http                                 extern
    #endif
    /*==================================��������===========================================*/
    e_http int netGet(char* url, char* params, int timeout, char *result, int *result_len);
    e_http int netPost(char* uri, char* params, int timeout, char *result, int *result_len);
    /*==================================��������===========================================*/
    #ifdef _HTTP_C

    #endif




#endif
