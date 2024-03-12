#ifndef SOCKET_CLIENT_H_
#define SOCKET_CLIENT_H_

#include <Winsock2.h>
#include "typedef_data.h"
//#include "system.h"
#define NINGBO_BANK
SOCKET socket_connect(char *ip, int port,char*remote_ip,int remote_port, int timeout_ms);
int socket_send(SOCKET socketfd, uint8 *buf, int len);
int socket_recv(SOCKET socketfd, uint8 *buf, int len);
void socket_close(SOCKET socketfd);
int socket_recv_0218(SOCKET sockfd, uint8 *buf, int len, struct InvoiceSchedulePerCmd* schedule_data);
int deal_sockets5(SOCKET sockfd, unsigned char *ip, int port,int flag);
#endif
