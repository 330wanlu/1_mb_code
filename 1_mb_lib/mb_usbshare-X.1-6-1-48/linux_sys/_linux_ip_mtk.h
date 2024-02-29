#ifndef _LINUX_IP_DEFINE
    #define _LINUX_IP_DEFINE
    /*====================================include file================================================*/
    #include "../_opt.h"
    #include <dirent.h>
    #include <sys/types.h>
    #include <sys/stat.h>
    #include <sys/ioctl.h>
    #include <regex.h>
    #include <errno.h>
    #include <string.h>
    #include <stdio.h>
    #include <fcntl.h>
    #include "net/if.h"
    #include "arpa/inet.h"
    #include "linux/sockios.h"
	#include <net/route.h>
	#include <netinet/in.h>
	#include <stdlib.h>
	#include <arpa/inet.h>
	#include <netdb.h> 
	#include <unistd.h>
	#include <sys/socket.h>
#include "../software/_mb_tools.h"
#include "../manage/_process_manage.h"
#ifndef RELEASE_SO
	#include "../software/_code_s.h"
	#include "../linux_s/_t_file_c.h"
#else
	#include <mb_clib/_code_s.h>
#endif
	#include <sys/socket.h>  //add by whl
	//#include "../linux_s/_t_file_c.h"

    /*==================================立即数定义===================================================*/
    #ifdef _linux_c
        #define e_linux_ip
        #define out(s,arg...)                //log_out("_linux_ip.c",s,##arg)        
    
    #else
        #define e_linux_ip                  extern
    
    #endif
    
    #ifdef _linux_c
        
		#define DF_LINUX_NETWORK_FILE		"/etc/config/network"
		#define DF_LINUX_WIRELESS_FILE		"/etc/config/wireless"
		

    
    
    #endif
    /*=====================================网络结构体定义===========================================*/
    struct _net_define
    {   uint32 ip;
        uint32 mask;
        uint32 gateway[3]; 
		uint32 DNS;
		int type;
		int net_mode;//0:lan 1:wifi  2:4g
		int flag_dhcp;
		char wifi_name[50];
		char wifi_pwd[20];
		char wifi_mode[30];
    };
    
	struct ethtool_value 
	{
        uint32      cmd;
        uint32      data;
	};
    
    
    
    
    /*====================================函数定义===================================================*/
	e_linux_ip int set_net_4g(char *eth, struct _net_define  *net);
	e_linux_ip int set_net_file_wifi(char *eth, struct _net_define  *net);
	e_linux_ip int change_mtk4_wifi_netconfig_file(char *eth, struct _net_define  *net);
    e_linux_ip  int get_net(char *eth,struct _net_define  *net);
    e_linux_ip  int set_net(char *eth,struct _net_define  *net);
	//e_linux_ip	int get_net_file(char *eth,struct _net_define  *net);
    //e_linux_ip  int get_DNS_ip(uint8 *hostname,uint8 *hostip);
    e_linux_ip  uint32 get_ip(char *eth);
    e_linux_ip  int set_ip(uint32 ip,char *eth);
    e_linux_ip  uint32 get_mask(char *eth);
	e_linux_ip  int juge_gateway(char *eth, uint32 gateway);
    e_linux_ip  int get_gateway(char *eth,uint32 getway[],int size_get);
    e_linux_ip  int set_mask(uint32 ip,char *eth);
	//e_linux_ip  int set_gateway(uint32 ip,char *eth);
	e_linux_ip int get_type(char *eth);
	e_linux_ip int get_socket_net_state(char *eth);
	e_linux_ip int get_net_mac(char *eth, char *mac);
	e_linux_ip int connect_ip_test(char *ip ,int port,int timer_s);
	e_linux_ip int set_net_1(char *eth,struct _net_define  *net);
	e_linux_ip int get_a20_net_file(char *eth,struct _net_define  *net,uint8 *mac);
	e_linux_ip int get_mac_addr(uint8 *macaddr);
	e_linux_ip int hostname_to_ip(char *host, char *ip);
	e_linux_ip int get_DNS_ip(char *host, char *ip);
	e_linux_ip int set_net_file(char *eth, struct _net_define  *net);
	e_linux_ip int get_dns_file_servername(char *dns);
	e_linux_ip int jude_dev_support_wifi(void);
	e_linux_ip int find_now_wifi_ssid(char *ssid);
    #ifdef _linux_c
        //static int oper_ifconfig(char *buf,int buf_len);
        //static int oper_set_ifconfig(char *eth,char *ip_asc);
		//static int get_fe(char *name,char *str,char *end,struct _net_define  *net);
		static int get_a20_fe(char *macname,char *str,char *end,struct _net_define  *net,uint8 *macaddr);
		static int set_DNS_value(struct _net_define  *net);
		//static int get_dns(char *name,char *str,char *end,struct _net_define  *net);
		static int get_dhcp_static(char *name,char *str,char *end,char  *net_type);
		static int get_file_head_end(const char *buf,const char *eth,const char *end,char **p_head,char **p_end);
		//static int pack_eth(char *eth,struct _net_define  *net,char *o_buf);
		//static int pack_a20_eth(char *eth,struct _net_define  *net,char *o_buf,uint8 *mac_addr);
		//static int pack_dns(struct _net_define  *net,char *o_buf);
		//static int pack_eth_ip(uint32 ip,char *head,char *o_buf);
		//static int get_dns_name(void);
		//static	int set_net_file_dhcp(char *eth);
		//static int ntod(uint32 mask);
		//static int dton(int mask);
		//static void itoa (int n,char s[]);
		static int set_net_config_file(char *config_inter,char *groupname,char *configName,char *configBuffer);
		static int del_net_config_file_line(char *config_inter,char *groupname,char *configName);
		//static int strap_32(int data);
    #endif
    
    
    
    







#endif

