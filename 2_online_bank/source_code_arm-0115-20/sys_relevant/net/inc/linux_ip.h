#ifndef _LINUX_IP_H
#define _LINUX_IP_H

#include "opt.h"

#include "process_manage.h"
#include "mb_tools.h"

#define linux_ip_out(s,arg...)             log_out("all.c",s,##arg)        
    
#define DF_LINUX_NET_DIR            "/proc/net"
#define DF_LINUX_NETWORK_FILE		"/etc/init.d/auto_config_network"
#define DF_LINUX_NETWORK_FILE_BACK	"/etc/init.d/auto_config_network_back"
#define DF_LINUX_DNS				"/etc/resolv.conf"
    
struct _net_define
{   uint32 ip;
	uint32 mask;
	uint32 gateway[3]; 
	uint32 DNS;
	int type;
};

struct ethtool_value 
{
	uint32      cmd;
	uint32      data;
};

int get_net(char *eth,struct _net_define  *net);
int set_net(char *eth,struct _net_define  *net);
int get_net_file(char *eth,struct _net_define  *net);
int get_DNS_ip(char *hostname, char *hostip);
uint32 get_ip(char *eth);
int jude_dev_support_wifi(void);
uint32 get_mask(char *eth);
int get_gateway(char *eth,uint32 getway[],int size_get);
int get_socket_net_state(char *eth);
int set_net_1(char *eth,struct _net_define  *net);
int get_a20_net_file(char *eth,struct _net_define  *net,uint8 *mac);
int get_mac_addr(uint8 *macaddr);
int get_net_mac(char *eth, char *mac);
int connect_ip_test(char *ip, int port, int timer_s);
int write_mac2network(char *w_mac);
int find_now_wifi_ssid(char *ssid);
int get_dns_file_servername(char *dns);

#endif
