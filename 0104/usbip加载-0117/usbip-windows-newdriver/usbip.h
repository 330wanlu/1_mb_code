/*
 * Copyright (C) 2005-2007 Takahiro Hirofuchi
 */

#ifndef _USBIP_H
#define _USBIP_H

#ifdef _DEBUG
	//#define DEBUG
#endif

#define _CRT_SECURE_NO_WARNINGS
#define WINVER 0x0501
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <setupapi.h>
#include <initguid.h>
#include <winioctl.h>

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>
#include "mylog.h"
//#include <basetyps.h>
//#include <wtypes.h>

#ifdef _MSC_VER 
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Setupapi.lib")
typedef size_t ssize_t;
#endif
#include "Cjson\cJSON.h"
#include "win_stub.h"
#include "usbip_protocol.h"
#include "usbip_network.h"
#include "usbip_common.h"
#include "usbip_vbus_ui.h"
#include "usbip_forward.h"
#include "usbip_wudev.h"
#define USBIP_ERROR_CONNECT			-500
#define USBIP_ERROR_ONLINE			USBIP_ERROR_CONNECT-1
#define USBIP_ERROR_QURY_DEVICE		USBIP_ERROR_ONLINE-1
#define USBIP_ERROR_APPLY_TER_USE	USBIP_ERROR_QURY_DEVICE-1
#define USBIP_ERROR_DRIVER			USBIP_ERROR_APPLY_TER_USE-1


#define USBIP_ERR_CONNECT_SERVER -1000
#define USBIP_ERR_IMPORT_PORT		-1007
//#define USBIP_ERR_CONNECT_SERVER -1008
//以下三个为公网加载机柜返回错误代码
#define USBIP_ERR_PORT_ERR	-105
#define USBIP_ERR_NOT_AUTH	-106
#define USBIP_ERR_PORT_BUSY -107

#define USBIP_ERROR_GET_DEVICE -1066
#define USBIP_ERR_PORT_DETTACH -1099
/****************************内网报错*************************/
#define USBIP_ERR_INNER_PORT_AUTH -1066

#define USBIP_ERR_INNER_PORT_ERR -1005
#define USBIP_ERR_INNER_PORT_ERR1 -1006

#define NEW_USBSHARE_PROTOCOL				"mb_usbshare_20191024"
#define NEW_USB_SHARE_SYNC_CODE_REQUEST		"request"
#define NEW_USB_SHARE_SYNC_CODE_RESPONSE	"response"

int deal_sockets5(SOCKET sockfd, unsigned char *ip,int port,int flag);
void socket_close(SOCKET socketfd);

extern int attach_usbip(char * host, int serv_port, int dev_port, int trans_type, char *user,int *sockfd,char *oper_id,int *virtual_port);
static int apply_for_port_use(SOCKET sockfd,int port,char *trans_server,char *trans_port,char * clientid);
static int get_busid(SOCKET sockfd,int port,unsigned char *busid);
static int query_interface_8009(SOCKET sockfd, char * busid, struct usb_interface * uinf0,int trans_type,int  dev_port,char *user,char *oper_id);
#endif
