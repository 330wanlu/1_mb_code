#ifndef _USBIP_FORWARD_H
#define _USBIP_FORWARD_H
#pragma

#include "win_stub.h"
#include <winsock2.h>
#include "usbip_protocol.h"
#include <signal.h>
#include "mylog.h"
#include <time.h>
typedef size_t ssize_t;
int flag;
HANDLE test_fd;
int usbip_forward(HANDLE hdev_src, int sockfd, BOOL inbound);
#endif