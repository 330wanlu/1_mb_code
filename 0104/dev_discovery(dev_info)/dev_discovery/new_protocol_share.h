#ifndef _NEW_PROTOCOL_SHARE_H
#define _NEW_PROTOCOL_SHARE_H
#include <Winsock2.h>

#include <string.h>
#include "../src/Cjson/cJSON.h"
#include "..\src\mylog.h"
#include "..\src\manage_set_file.h"
#pragma comment(lib, "ws2_32.lib")




#define true 1
#define false 0
#define DLL_ERR_NEW_SHARE_SOCKET_CONNET 220
#define DLL_ERR_NEW_SHARE_SOCKET_SEND	DLL_ERR_NEW_SHARE_SOCKET_CONNET+1
#define DLL_ERR_NEW_SHARE_SOCKET_RECV	DLL_ERR_NEW_SHARE_SOCKET_SEND+1
#define DLL_ERR_NEW_SHARE_CMD_OPER	DLL_ERR_NEW_SHARE_SOCKET_RECV+1

struct iptables_oper
{
	int oper_port;
	int drop_or_accpet;
	char order[500];

};
static int parse_json_result(char *json_buf);
static long recv_data_len(int fd);


extern int set_usbshare_ip_new(int ip_type, char *server_ip, int server_port, char *ip_in, char *mask_in, char *gateway_in, char *dns_in, char *extension);
extern int get_usbshare_status_new(char *server_ip, int server_port,char *intranet_ip ,char *external_ip,char *out_ter_info);
extern int set_usbshare_id_date(char *server_ip, int server_port, char *ter_id, char *product_date);
extern int restart_terminal(char *server_ip, int server_port);
extern int set_usbshare_iptables(char *server_ip, int server_port,struct iptables_oper *iptb_oper,char *output);
#endif