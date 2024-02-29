/*****************************************************************************
File name:   common.h
Description: 全局头文件引用
Author:      Zako
Version:     1.0
Date:        2019.12
History:
20191201     初始化
*****************************************************************************/
#ifndef COMMON_COMMON_H
#define COMMON_COMMON_H

//---------------------------系统依赖------------------------------------
#include <arpa/inet.h>
#include <byteswap.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <iconv.h>
#include <math.h>
#include <memory.h>
#include <netdb.h>
#include <netinet/tcp.h>
#include <poll.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/queue.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
//---------------------------第三方依赖------------------------------------
// usb操作库【libusb】依赖
#include <libusb-1.0/libusb.h>
// 通信及算法加密库openssl依赖
#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/md5.h>
#include <openssl/sm4.h>
#include <openssl/ssl.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>
// http及事件库【libevent】依赖
#include <event2/buffer.h>
#include <event2/buffer_compat.h>
#include <event2/bufferevent.h>
#include <event2/bufferevent_ssl.h>
#include <event2/dns.h>
#include <event2/event.h>
#include <event2/http.h>
#include <event2/keyvalq_struct.h>
#include <event2/listener.h>
#include <event2/thread.h>
#include <event2/util.h>
//软浮点库【mpfr】依赖，用于解决嵌入式硬件浮点运算能力不足
#include <mpfr.h>

//---------------------------条件编译------------------------------------

//---------------------------本地第三方依赖---------------------------------
//--------------------------本地模块头文件-----------------------------------
//##调用先后顺序不能调整，否则需要调整头文件引用
// clang-format off
#include "idaheader.h"
#include "define.h"
#include "debug.h"
//##不分先后 begin
#include "importinv.h"
#include "fpxx_json.h"
#include "usb_device.h"
//##不分先后 finish
//通用实现调用子模块是不太合适的，因为不是C++工程，无法用类来进行继承，但是为了减少代码冗余先这么实现吧
#include "function.h"  //涉及aisino和nisec两个工程的连接，因此必须放在最后引用
#include "character.h"
#include "../authority/ta_iface.h"
#include "../aisino/module/aisino_common.h"
#include "../nisec/module/nisec_common.h"
#include "../cntax/module/cntax_common.h"

// clang-format on

typedef struct
{
    char *head;
    char *value;

}HEADER_STRUCT;

#endif  //_COMMON_HEADER_H
