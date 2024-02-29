#ifndef _HTTP_BOOT_H
#define _HTTP_BOOT_H
#include "_comm_boot.h"
#include "_tools_boot.h"
#ifdef OPENWRT
#include <sys/socket.h>
#include <netdb.h>
#endif
int GetDnsIp(char *host, char *ip);
int http_socket_open(int *socket_t, char *ip);
int http_upload_err(int socketfd, struct http_parm *parm);
int http_get_ter_auth(int socketfd, struct http_parm *parm);
int http_get_ter_update(int socketfd, struct http_parm *parm);
int http_get_token(int socketfd, struct http_parm *parm);
int http_get_server_time(int socketfd, char *time, char *ip);
int open_rec_over_timer(int socketfd, int over_timer);
int open_send_over_timer(int socketfd, int over_timer);
int socket_write(int sock, unsigned char *buf, int buf_len);
int deal_http_data(int sockfd, char **outbuf);
int get_DNS_ip(char *hostname, char *hostip);
int http_api_test(int socketfd, char *ip);


#endif