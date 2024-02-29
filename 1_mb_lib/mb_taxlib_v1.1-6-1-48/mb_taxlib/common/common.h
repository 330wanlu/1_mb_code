/*****************************************************************************
File name:   common.h
Description: ȫ��ͷ�ļ�����
Author:      Zako
Version:     1.0
Date:        2019.12
History:
20191201     ��ʼ��
*****************************************************************************/
#ifndef COMMON_COMMON_H
#define COMMON_COMMON_H

//---------------------------ϵͳ����------------------------------------
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
//---------------------------����������------------------------------------
// usb�����⡾libusb������
#include <libusb-1.0/libusb.h>
// ͨ�ż��㷨���ܿ�openssl����
#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/md5.h>
#include <openssl/sm4.h>
#include <openssl/ssl.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>
// http���¼��⡾libevent������
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
//����⡾mpfr�����������ڽ��Ƕ��ʽӲ������������������
#include <mpfr.h>

//---------------------------��������------------------------------------

//---------------------------���ص���������---------------------------------
//--------------------------����ģ��ͷ�ļ�-----------------------------------
//##�����Ⱥ�˳���ܵ�����������Ҫ����ͷ�ļ�����
// clang-format off
#include "idaheader.h"
#include "define.h"
#include "debug.h"
//##�����Ⱥ� begin
#include "importinv.h"
#include "fpxx_json.h"
#include "usb_device.h"
//##�����Ⱥ� finish
//ͨ��ʵ�ֵ�����ģ���ǲ�̫���ʵģ���Ϊ����C++���̣��޷����������м̳У�����Ϊ�˼��ٴ�����������ôʵ�ְ�
#include "function.h"  //�漰aisino��nisec�������̵����ӣ���˱�������������
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
