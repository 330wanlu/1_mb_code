#ifndef _HTTP_H
    #define _HTTP_H
    /*=================================包含文件============================================*/

    #include "ghttp.h"
    /*=================================立即数定义==========================================*/
    #ifdef _HTTP_C
        #define e_http
        #define out(s,arg...) 							//log_out("main.c",s,##arg)                          
    #else
        #define e_http                                 extern
    #endif
    /*==================================函数定义===========================================*/
    e_http int netGet(char* url, char* params, int timeout, char *result, int *result_len);
    e_http int netPost(char* uri, char* params, int timeout, char *result, int *result_len);
    /*==================================变量定义===========================================*/
    #ifdef _HTTP_C

    #endif




#endif
