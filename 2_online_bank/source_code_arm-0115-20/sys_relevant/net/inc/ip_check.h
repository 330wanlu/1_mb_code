#ifndef _IP_CHECK_H
#define _IP_CHECK_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <linux/if_packet.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <net/ethernet.h>
#include <netinet/ether.h>
#include <net/if.h>
#include <netinet/ip.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/time.h>

#include "mb_tools.h"

#define IP_CHECK_TMP_FILE_PATH		"/etc/ip_check.txt"

#define ip_check_out(s,arg...)       		log_out("all.c",s,##arg) 

int get_net_delay_intranet(const char *if_name, const char *dst_ip);
int get_net_delay_external(char *ip);
// int get_net_delay_intranet(char *ip);
int ip_conflict_check(const char *if_name, const char *dst_ip);

#endif
