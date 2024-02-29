#ifndef _REG_HTTP_H
#define _REG_HTTP_H
#include "../_opt.h"
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <openssl/sha.h>
#include <openssl/md5.h>
#include <ctype.h>
#include <sys/time.h>
#ifndef RELEASE_SO
#include "../software/cJSON.h"
#include "../software/_dec_file.h"
#endif
#include "../core/_socket_client.h"
#include "../software/_mb_tools.h"

#ifdef http_reg_c
	#ifdef DEBUG
		#define out(s,arg...)       				log_out("all.c",s,##arg) 
	#else
		#define out(s,arg...)						log_out("all.c",s,##arg)
	#endif	
		#define e_htt_reg
	#else    
		#define e_htt_reg                    extern    
#endif

#define HTTP_PORT		10065
#define HTTP_ADDR		"www.njmbxx.com"
#define HTTP_CONTENT_TYPE "application/x-www-form-urlencoded"


#define HTTP_TIMEOUT	60

struct _http_comon
{
	char token[100];
	char timestamp[20];
	char appid[50];
	char appsecret[50];
	char s_ip[50];
	int s_port;
};

struct http_parm{
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
#ifdef http_reg_c
	static int open_rec_over_timer(int socketfd, int over_timer);
	static int open_send_over_timer(int socketfd, int over_timer);
	static int get_line(int sock, char *buf, int size);
	static int deal_http_data(int sockfd, char **outbuf);
#endif
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
#endif