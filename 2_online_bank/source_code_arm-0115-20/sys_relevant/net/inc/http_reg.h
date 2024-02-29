#ifndef _REG_HTTP_H
#define _REG_HTTP_H

#include "opt.h"

#include "socket_client.h"
#include "mb_tools.h"

#include "../../update/inc/update.h"

#define http_reg_out(s,arg...)       				log_out("all.c",s,##arg) 

#define HTTP_PORT		10065
#define HTTP_ADDR		"www.njmbxx.com"
#define HTTP_CONTENT_TYPE "application/x-www-form-urlencoded"
#define HTTP_TIMEOUT	60
#define TER_ID_INFO_PATH	"/etc/ter.info"

#pragma pack(4)
struct _http_comon
{
	int s_port;
	char token[128];
	char timestamp[32];
	char appid[64];
	char appsecret[64];
	char s_ip[64];
};

struct http_parm
{
	unsigned char token[100];
	unsigned char ter_id[15];
	unsigned char timestamp[20];
	unsigned char appid[20];
	char app_ver[20];
	char cpu_id[20];
	unsigned char key_s[4096];
	char ip[30];
	unsigned char errmsg[500];
};

int Boot_Strptime(char *szStandTimeIn, struct tm *tt0);
int http_socket_open(int *socket_t, struct http_parm * parm);
int http_get_token(int socketfd, struct http_parm *parm);
int http_get_server_time(int socketfd, unsigned char *time, char *ip);
int socket_write(int sock, unsigned char *buf, int buf_len);
int http_send_sign_enc(int socketfd, struct http_parm *parm);
int http_get_appid_secert_token(struct _http_comon *parm);
int http_token_register_dev_to_server(struct _http_comon *parm, char *s_data, int s_len, char *err_msg);
int http_token_get_mqtt_server_info(struct _http_comon *parm, char *s_data, int s_len, char *r_data, int *r_len);
int http_token_get_mqtt_connect_md5(struct _http_comon *parm, char *s_data, int s_len,char **r_data, int *r_len);
int http_token_get_mqtt_connect_info(struct _http_comon *parm, char *s_data, int s_len, char **r_data, int *r_len);

int http_get_ter_auth(int socketfd, struct http_parm_m *parm);
int http_socket_open_m(int *socket_t,char *ip,int port);

#endif
