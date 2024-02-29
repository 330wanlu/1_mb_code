#define _linux_c
#include "_linux_ip_mtk.h"
int get_net(char *eth,struct _net_define  *net)
{   uint32 ip,mask,gateway,dns=0; 
	int type;
    if(net==NULL)
    {   out("输入指针不能为空\n");
        return -1;
    }
    ip =  get_ip(eth);
	if(ip < 0)
	{
		out("获取ip地址失败\n");
	}
	net->ip = ip;
	//out("IP=%08x\n",net->ip);
	mask = get_mask(eth);
	if(mask < 0)
	{
		out("获取子网掩码失败\n");
	}
	net->mask = mask;
	//out("mask=%08x\n",net->mask);
	gateway = get_gateway(eth, net->gateway,(sizeof(net->gateway)/sizeof(uint32)));
	if(gateway < 0)
	{
		out("获取网关失败\n");
	}
	//out("gateway=%08x\n",net->gateway[0]);
	//dns = get_dns_name();
	//dns = 0x
	type = get_type(eth);
	net->type = type;
	if (type != 1)//not static 
	{
		net->DNS = ip_asc2ip("114.114.114.114", 15);
	}
	else
	{

		char dns_tmp[255] = { 0 };

		dns = get_dns_file_servername(dns_tmp);
		if (dns < 0)
		{
			out("获取DNS失败\n");
		}
		//printf("%s dns_tmp\n",dns_tmp);
		net->DNS = dns;
	}
	
	return 0;  
    
}

int jude_dev_support_wifi(void)
{
	return -1;
}
int find_now_wifi_ssid(char *ssid)
{
	return -1;
}
int get_dns_file_servername(char *dns)
{
	FILE *fp = NULL;
	char buf[2048] = { 0 };
	char key[1024] = { 0 };
	char value[1024] = { 0 };
	uint32 dns_i = 0;
	int lineCnt = 0;
	int i;
	fp = fopen(DF_LINUX_DNS, "r");     // fp指针指向文件头部
	if (!fp) // 判断是否打开成功
		return -1;
	while (fgets(buf, 1000, fp)) // fgets循环读取，直到文件最后，才会返回NULL
	{
		lineCnt++; // 累计行数
	}
	fseek(fp, 0, SEEK_SET);
	for (i = 0; i < lineCnt; i++)
	{
		memset(key, 0, sizeof(key));
		memset(value, 0, sizeof(value));
		fscanf(fp, "%s %s", key, value);
		if (strcmp(key, "nameserver") == 0)
		{
			memcpy(dns, value, strlen(value));
			dns_i = ip_asc2ip(dns, strlen(dns));
			//printf("dns %s %08x\n",dns,dns_i);
			fclose(fp);
			return dns_i;
		}
		continue;
	}
	fclose(fp);
	return -2;
}
int judge_network_file_exit_create()
{
	if (access("/etc/config/network", 0) != 0)
	{
		system("touch /etc/config/network");
		sync();
		char buf[1024] = { 0 };
		strcpy(buf, "\n"\
			"config interface \'loopback\'\n"\
			"	option ifname \'lo\'\n"\
			"	option proto \'static\'\n"\
			"	option ipaddr \'127.0.0.1\'\n"\
			"	option netmask \'255.0.0.0\'\n"\
			"\n"\
			"config globals \'globals\'\n"\
			"	option ula_prefix \'auto\'\n"\
			"\n"\
			"config interface \'lan\'\n"\
			"	option ifname \'eth0.1\'\n"\
			"	option force_link '1'\n"\
			"	option macaddr \'da:7f:6d:ef:33:70\'\n"\
			"	option type \'bridge\'\n"\
			"	option proto \'static\'\n"\
			"	option ipaddr '192.168.155.1\'\n"\
			"	option netmask \'255.255.255.0\'\n"\
			"	option gateway \'192.168.155.1\'\n"\
			"	option broadcast \'192.168.155.255\'\n"\
			"	option dns \'114.114.114.114\'\n"\
			"	option ip6assign \'60\'\n"\
			"\n"\
			"config interface \'wan\'\n"\
			"	option ifname \'eth0.2\'\n"\
			"	option force_link \'1\'\n"\
			"	option macaddr \'da:7f:6d:ef:33:71\'\n"\
			"	option proto \'dhcp\'\n"
			"\n"\
			"\n"\
			"config interface \'wan6\'\n"\
			"	option ifname \'eth0.2\'\n"\
			"	option proto \'dhcpv6\'\n"\
			"\n"\
			"config switch\n"\
			"	option name \'switch0\'\n"\
			"	option reset \'1\'\n"\
			"	option enable_vlan \'1\'\n"\
			"\n"\
			"config switch_vlan\n"\
			"	option device \'switch0\'\n"\
			"	option vlan \'1\'\n"\
			"	option ports \'1 2 3 6t\'\n"\
			"\n"\
			"config switch_vlan\n"\
			"	option device \'switch0\'\n"\
			"	option vlan \'2\'\n"\
			"	option ports \'0 4 6t\'\n");
		//printf("network buf is:\n%s\n", buf);
		logout(INFO, "system", "ip", "judge_network_file_exit_create doont exit!\r\n");
		FILE* f = fopen(DF_LINUX_NETWORK_FILE, "w+");
		if (f == NULL)
			return -1;
		fputs(buf, f);
		fclose(f);
		sync();
		return -1;
	}
	return 0;
}

int set_net(char *eth,struct _net_define	*net)
{	int ip_type;
	int mode;
	int result;
	int system_ip_type;
	char cmd[100];
	out("set_net\n");
	out("ip = [%08x] mask = [%08x] gate = [%08x]\n",net->ip,net->mask,net->gateway[0]);
	ip_type=net->type;
	mode = net->net_mode;
	judge_network_file_exit_create();//--判断network文件是否存在，不存在生成一个默认的
	if (mode == 0)
	{
		if (ip_type == 2)
		{
			out("设置固定IP\n");
			system("killall udhcpc");
			result = set_net_file(eth, net);
			if (result < 0)
			{
				//set_mask(net->mask, eth);
				out("无需在文件中重新设置网络参数\n");
				return 0;
			}
			out("设置当前网络参数\n");
			out("ip = [%08x] mask = [%08x] gate = [%08x]\n", net->ip, net->mask, net->gateway[0]);

			return 0;
		}
		else if (ip_type == 1)
		{
			out("设置DHCP\n");
			memset(cmd, 0, sizeof(cmd));

			system_ip_type = get_type(eth);
			if (system_ip_type == 0)//DHCP
				return 0;

			sprintf(cmd, "udhcpc -i %s", eth);
			set_net_file(eth, net);
			out("cmd = %s\n", cmd);

			result = detect_process("udhcpc -i", "dhcpc.txt");
			if (result < 0)
			{
				printf("###################start udhcpc###################\n");
				system(cmd);
			}
			if (result > 1)
			{
				logout(INFO, "system", "ip", "#################udhcpc进程过多需要删除###############\r\n");
				system("killall udhcpc");
				system(cmd);
			}
			/*memset(cmd, 0, sizeof(cmd));
			sprintf(cmd, "/etc/init.d/network restart");
			system(cmd);
			sync();*/
			return 0;
		}
		else
			return -1;
	}
	else if (mode == 1)
	{
		result = set_net_file_wifi(eth, net);
		return 0;
	}
	else if (mode == 2)
	{
		if (access("/etc/ppp/peers", 0) != 0)
		{
			system("mkdir /etc/ppp/peers");
			sync();
			system("cp /home/share/exe/peers/* /etc/ppp/peers/. -r");
			sync();
		}
		result = set_net_4g(eth, net);
	}
	//return set_net_1(eth,net);	
	return 0;
}


//static int get_dns_name(void)
//{	
//	char buf[2048];
//	//int i;
//	char *p_file_head,*p_file_end;
//	struct _net_define  net;
//	memset(buf,0,sizeof(buf));
//	if(read_file(DF_LINUX_DNS,buf,sizeof(buf))<0)
//	{	out("读取文件:[%s]失败\n",DF_LINUX_DNS);
//		return -1;
//	}
//	if(get_file_head_end(buf,"nameserver","\n\n\n",&p_file_head,&p_file_end)<0)
//	{	out("dns文件内容错误\n");
//	   	return -2;
//	}
//	if(get_dns("nameserver ",p_file_head,p_file_end,&net)<0)
//	{	out("获取dns地址失败\n");
//		return -3;
//	}
//	return net.DNS;
//}

int get_type(char *eth)
{	
	char buf[2048];
	//char w_buf[2048];
	char type[20];
	char *p_file_head,*p_file_end;
	//int i;
	memset(type,0,sizeof(type));
	if(read_file(DF_LINUX_NETWORK_FILE,buf,sizeof(buf))<0)
	{	out("读取文件:[%s]失败\n",DF_LINUX_NETWORK_FILE);
		return -1;
	}
	if(get_file_head_end(buf,"config interface \'wan\'","config interface \'wan6\'",&p_file_head,&p_file_end)<0)
	{	out("dns文件内容错误\n");
	   	return -2;
	}
	if(get_dhcp_static("option proto \'",p_file_head,p_file_end,type)<0)
	{	out("获取dns地址失败\n");
		return -3;
	}
	
	if(memcmp(type,"static",strlen("static"))==0)
	{
		//out("IP为固定IP\n");
		return 1;
	}
	return 0;
}

int set_net_1(char *eth, struct _net_define  *net)
{
	struct _net_define my_net_define;
	char ip_asc[100];
	char mask_asc[100];
	char gatewaybuf[100];
	char tmpbuf[100];
	char tmp[200];
	int gateway, i;
	memset(&my_net_define, 0, sizeof(my_net_define));
	memset(ip_asc, 0, sizeof(ip_asc));
	memset(mask_asc, 0, sizeof(mask_asc));
	if ((net->ip != 0) && (net->mask != 0))
	{
		ip2ip_asc(net->ip, ip_asc);
		out("设置当前网络的IP=[%s]\n", ip_asc);
		ip2ip_asc(net->mask, mask_asc);
		out("设置当前网络的mask_asc=[%s]\n", mask_asc);
		sprintf(tmp, "ifconfig %s %s netmask %s",eth, ip_asc, mask_asc);
		out("%s\n\n\n", tmp);
		system(tmp);
	}
	if (net->gateway[0] != 0)
	{
		gateway = get_gateway(eth, &my_net_define.gateway[0], (sizeof(my_net_define.gateway) / sizeof(uint32)));
		if (gateway <= 0)
		{
			out("获取网关失败\n");
			memset(tmpbuf, 0, sizeof(tmpbuf));
			memset(gatewaybuf, 0, sizeof(gatewaybuf));
			ip2ip_asc(net->gateway[0], gatewaybuf);
			out("设置当前网络的网关%s\n", gatewaybuf);
			sprintf(tmpbuf, "route add default gw %s", gatewaybuf);
			out("%s\n\n\n", tmpbuf);
			system(tmpbuf);
			return 0;
		}
		out("获取到%d条网关信息\n", gateway);
		for (i = 0; i<gateway; i++)
		{
			out("gateway=%08x\n", my_net_define.gateway[i]);
			memset(tmpbuf, 0, sizeof(tmpbuf));
			memset(gatewaybuf, 0, sizeof(gatewaybuf));
			ip2ip_asc(my_net_define.gateway[i], gatewaybuf);
			sprintf(tmpbuf, "route del default gw %s", gatewaybuf);
			out("%s\n\n\n", tmpbuf);
			system(tmpbuf);
		}
		memset(tmpbuf, 0, sizeof(tmpbuf));
		memset(gatewaybuf, 0, sizeof(gatewaybuf));
		ip2ip_asc(net->gateway[0], gatewaybuf);
		out("设置当前网络的网关%s\n", gatewaybuf);
		sprintf(tmpbuf, "route add default gw %s", gatewaybuf);
		out("%s\n\n\n", tmpbuf);
		system(tmpbuf);
	}
	return 0;
}





//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//外部调用函数
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//int hostname_to_ip(char *host, char *ip)
//{		char               buf[100];  
//        struct addrinfo hints;
//        struct addrinfo *res, *curr;
//		struct sockaddr_in *sa;  
//        int ret;
//        /* obtaining address matching host */
//		if(host == NULL)
//		{
//			out("要解析的域名错误\n");
//			return -1;
//		}
//		//out("要解析的域名为%s\n",host);
//        memset(&hints, 0, sizeof(struct addrinfo));
//        hints.ai_family = AF_INET;
//        //hints.ai_socktype = SOCK_STREAM;
//        hints.ai_flags = AI_CANONNAME;
///*		if (gethostname(host, sizeof (host)) < 0) 
//		{  
//			perror("gethostname");  
//			return -1;   
//		}    */ 
//        //hints.ai_protocol = 0;  /* any protocol */
//        ret = getaddrinfo(host, NULL, &hints, &res);
//        if (ret != 0)
//        {	freeaddrinfo(res);
//			fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(ret));
//			return -1;
//        }
//		curr = res;  
//       /* traverse the returned list and output the ip addresses */
//        while (curr && curr->ai_canonname)
//        {	sa = (struct sockaddr_in *)curr->ai_addr;  
//			//printf("name: %s\nip:%s\n\n", curr->ai_canonname,inet_ntop(AF_INET, &sa->sin_addr.s_addr, buf, sizeof (buf)));  
//			sprintf(ip,"%s",inet_ntop(AF_INET, &sa->sin_addr.s_addr, buf, sizeof (buf)));
//			curr = curr->ai_next;
//        }
//		freeaddrinfo(res);
//        return strlen(ip);
//}

int get_DNS_ip(char *host, char *ip)
{
	char               buf[100];
	struct addrinfo hints;
	struct addrinfo *res, *curr;
	struct sockaddr_in *sa;
	char cmd[200] = { 0 };
	char cmd_readbuf[1024 * 6] = { 0 };
	int ret;
	/* obtaining address matching host */
	if (host == NULL)
	{
		out("要解析的域名错误\n");
		return -1;
	}
	//先判断网络是否通畅
	memset(cmd, 0, sizeof(cmd));
	sprintf(cmd, "ifconfig");
	mb_system_read(cmd, cmd_readbuf, sizeof(cmd_readbuf), "/tmp/ifconfig_test_2.txt");
	if (strstr(cmd_readbuf, "eth0.2") ==  NULL)
		return -1;
	//out("get_DNS_ip 11\n");
	if (memcmp(host, "127.0.0.1", strlen((const char*)host)) == 0)
	{
		out("本机IP不用解析\n");
		sprintf((char *)ip, "127.0.0.1");
		return strlen((const char*)ip);
	}
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_INET;

	hints.ai_flags = AI_CANONNAME;
	//out("get_DNS_ip 22\n");
	ret = getaddrinfo(host, NULL, &hints, &res);
	if (ret != 0)
	{
		
		freeaddrinfo(res);
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(ret));
		return -1;
	}
	//out("get_DNS_ip 33\n");
	curr = res;
	/* traverse the returned list and output the ip addresses */
	while (curr && curr->ai_canonname)
	{
		sa = (struct sockaddr_in *)curr->ai_addr;
		//printf("name: %s\nip:%s\n\n", curr->ai_canonname,inet_ntop(AF_INET, &sa->sin_addr.s_addr, buf, sizeof (buf)));  
		sprintf(ip, "%s", inet_ntop(AF_INET, &sa->sin_addr.s_addr, buf, sizeof (buf)));
		curr = curr->ai_next;
	}
	//out("get_DNS_ip 44\n");
	freeaddrinfo(res);
	//out("get_DNS_ip 55\n");
	return strlen(ip);
}


int connect_ip_test(char *ip ,int port,int timer_s)
{	//int result;
	struct sockaddr_in add;
	int socket_fd;
	//int fd = 0;
    //struct sockaddr_in  addr;
    fd_set fdr, fdw;
    struct timeval timeout;
    int err = 0;
	int rc;
	int flags;
    int errlen = sizeof(err);
	//out("connect_ip_test \n");
	socket_fd=socket(AF_INET,SOCK_STREAM, 0);
	memset(&add,0,sizeof(add));
	add.sin_family=AF_INET;
	inet_pton(AF_INET,ip,&add.sin_addr);
	add.sin_port=htons(port);

	/*设置套接字为非阻塞*/
     flags = fcntl(socket_fd, F_GETFL, 0);
     if (flags < 0) 
	 {
         fprintf(stderr, "Get flags error:%s\n", strerror(errno));
		 shutdown(socket_fd,SHUT_RDWR);
         close(socket_fd);
         return -1;
     }
     flags |= O_NONBLOCK;
     if (fcntl(socket_fd, F_SETFL, flags) < 0) 
	 {
         fprintf(stderr, "Set flags error:%s\n", strerror(errno));
		 shutdown(socket_fd,SHUT_RDWR);
         close(socket_fd);
         return -2;
     }

	 //out("连接服务器:%s:%d\n",ip,port);
	 rc=connect(socket_fd,(struct sockaddr *)&add,sizeof(add));
	 if(rc != 0)
	 {
		 //if (errno == EINPROGRESS) 
		 //{
              //printf("Doing connection.\n");
              /*正在处理连接*/
              FD_ZERO(&fdr);
              FD_ZERO(&fdw);
              FD_SET(socket_fd, &fdr);
              FD_SET(socket_fd, &fdw);
              timeout.tv_sec = timer_s;
              timeout.tv_usec = 0;
              rc = select(socket_fd + 1, &fdr, &fdw, NULL, &timeout);
              //printf("rc is: %d\n", rc);
              /*select调用失败*/
              if (rc < 0) 
			  {   //fprintf(stderr, "connect error:%s\n", strerror(errno));
				  shutdown(socket_fd,SHUT_RDWR);
                  close(socket_fd);
                  return -3;
              }              
              /*连接超时*/
              if (rc == 0) 
			  {   //fprintf(stderr, "Connect timeout.\n");
				  shutdown(socket_fd,SHUT_RDWR);
                  close(socket_fd);
                  return -4;
              }
              /*[1] 当连接成功建立时，描述符变成可写,rc=1*/
              if (rc == 1 && FD_ISSET(socket_fd, &fdw)) 
			  {
                  //printf("Connect success\n");
				  shutdown(socket_fd,SHUT_RDWR);
                  close(socket_fd);
                  return 0;
              }
              /*[2] 当连接建立遇到错误时，描述符变为即可读，也可写，rc=2 遇到这种情况，可调用getsockopt函数*/
              if (rc == 2) 
			  {
				  if (getsockopt(socket_fd, SOL_SOCKET, SO_ERROR, &err, (socklen_t *)&errlen) == -1)
				  {   fprintf(stderr, "getsockopt(SO_ERROR): %s", strerror(errno));
					  shutdown(socket_fd,SHUT_RDWR);
                      close(socket_fd);
                      return -5;  
                  }  
                  if (err) 
				  {   errno = err;
                      //fprintf(stderr, "connect error:%s\n", strerror(errno));
					  shutdown(socket_fd,SHUT_RDWR);
                      close(socket_fd);
                      return -6; 
                  }
              } 
         //} 
		fprintf(stderr, "IP %s connect failed, error:%s.\n", ip,strerror(errno));
		shutdown(socket_fd,SHUT_RDWR);
		close(socket_fd);
		return -7;
	 }
	shutdown(socket_fd,SHUT_RDWR);
    close(socket_fd);
	return 0;
}

int get_socket_net_state(char *eth)
{
	char cmd[128];
	char order[200];
	char tmp_file[100];
	char state[5];
	//int pidnum;
	int result=-1;
	int i = 0;
	char lineBuff[256];// 存储读取一行的数据  
	sprintf(cmd, "swconfig dev switch0 port 0 show");
	//out("cmd %s\n",ps);

	memset(state, 0, sizeof(state));
	memset(order, 0, sizeof(order));
	memset(tmp_file, 0, sizeof(tmp_file));
	sprintf(tmp_file, "/tmp/netstate.txt");
	sprintf(order, "%s > %s", cmd, tmp_file);
	//out("order=[%s]\n",order);
	system(order);

	FILE* fp = fopen(tmp_file, "r");
	if (fp == NULL)
	{
		out("文件打开失败\n");
		return -1;
	}
	fseek(fp, 0, SEEK_END);
	fseek(fp, 0, SEEK_SET);

	memset(lineBuff, 0, sizeof(lineBuff));
	while (fgets(lineBuff, 256, fp) != NULL)
	{
		if (strlen(lineBuff) < 1) // 空行  
		{
			continue;
		}
		i += 1;
		//out("读到第%d行的数据为：%s", i, lineBuff);
		//print_array(lineBuff,20);
		if (memcmp(lineBuff, "\tlink: port:0 link:", strlen("\tlink: port:0 link:")) == 0)
		{
			memcpy(state, lineBuff + strlen("\tlink: port:0 link:"), 4);
		
			//out("state = %s\n", state);
			if (memcmp(state, "up", 2) == 0)
			{
				//out("网络连接为在线\n");
				result = 1;
				break;
			}
			else if (memcmp(state, "down", 4) == 0)
			{
				//out("网络连接为离线\n");
				result = 0;
				break;
			}
			else
			{
				out("获取失败\n");
				result = -1;
				break;
			}
		}
		
	}
	fclose(fp);
	return result;
//struct ethtool_value edata;
//   int fd,err = 0;
//   struct ifreq ifr;
//memset(&ifr, 0, sizeof(ifr));
//memcpy(ifr.ifr_name,eth,strlen(eth));
//fd = socket(AF_INET, SOCK_DGRAM, 0);
//if (fd < 0) 
//{
//	out("Cannot get control socket");
//	return -1;
//}
//edata.cmd = 0x0000000a;
//ifr.ifr_data = (caddr_t)&edata;
//err = ioctl(fd, 0x8946, &ifr);
//if (err == 0) 
//{
//		out("edata.data = %02x\n",edata.data);
//		out(stdout, "Link detected: %s\n",edata.data ? "yes":"no");
//	close(fd);
//	return edata.data;		
//} 
//else if (errno != EOPNOTSUPP) 
//{
//       out("Cannot get link status");
//	close(fd);
//	return -1;
//   }
//close(fd);
//  return -1;
}

int get_mac_addr(uint8 *macaddr)
{	struct ifreq ifreq;
	int i;
	unsigned char mac[20];
	int sock;
	memset(mac,0,sizeof(mac));
	sock = socket(AF_INET,SOCK_STREAM,0);
	if(sock<0)
	{
		out("打开socket失败\n");
		return -1;
	}
	strcpy(ifreq.ifr_name,"eth0.2");
	if(ioctl(sock,SIOCGIFHWADDR,&ifreq)<0)
	{
		out("获取本机MAC失败\n");
		close(sock);
		return -2;
	}
	out("获取本机MAC成功\n");
	out("%02X:%02X:%02X:%02X:%02X:%02X\n",	(unsigned char)ifreq.ifr_hwaddr.sa_data[0],
											(unsigned char)ifreq.ifr_hwaddr.sa_data[1],
											(unsigned char)ifreq.ifr_hwaddr.sa_data[2],
											(unsigned char)ifreq.ifr_hwaddr.sa_data[3],
											(unsigned char)ifreq.ifr_hwaddr.sa_data[4],
											(unsigned char)ifreq.ifr_hwaddr.sa_data[5]);

	sprintf((char *)mac,"%02x:%02x:%02x:%02x:%02x:%02x\n",	(unsigned char)ifreq.ifr_hwaddr.sa_data[0],
													(unsigned char)ifreq.ifr_hwaddr.sa_data[1],
													(unsigned char)ifreq.ifr_hwaddr.sa_data[2],
													(unsigned char)ifreq.ifr_hwaddr.sa_data[3],
													(unsigned char)ifreq.ifr_hwaddr.sa_data[4],
													(unsigned char)ifreq.ifr_hwaddr.sa_data[5]);
	out("mac =");
	for(i=0;i<20;i++)
	{
		printf("%02x ",mac[i]);
	}
	printf("\n");
	memcpy(macaddr,mac,strlen((const char *)mac));
	close(sock);
	out("获取mac地址成功\n");
	return 0;

}

//////////////////////////////////////////////////////////////////////////////////////////////////////////



//socket获取及设置IP函数
uint32 get_ip(char *eth)
{   int fd;
    struct ifreq ifr;
    struct sockaddr_in *addr;
    char ip_asc[100];
    //out("获取[%s]IP地址\n",eth);
    memset(&ifr,0,sizeof(ifr));
    fd=socket(AF_INET,SOCK_DGRAM,0);
    if(fd<0)
    {   out("获取IP,socket打开失败\n");
        return 0;
    }
    memcpy(ifr.ifr_name,eth,strlen(eth));
    if(ioctl(fd,SIOCGIFADDR,&ifr)==-1)
    {   close(fd);
        out("获取eth=[%s]的IP失败\n",eth);
        return 0;
    }
    addr=(struct sockaddr_in *)&(ifr.ifr_addr);
    memset(ip_asc,0,sizeof(ip_asc));
    inet_ntop(AF_INET,&addr->sin_addr,ip_asc,sizeof(ip_asc));
    //out("IP:[%s]\n",ip_asc);
    close(fd);
    return ip_asc2ip(ip_asc,strlen(ip_asc));    
    
}

int set_ip(uint32 ip,char *eth)
{   int fd;
    struct ifreq ifr;
    struct sockaddr_in *addr;
    char ip_asc[100];
    out("设置ip\n");
    memset(&ifr,0,sizeof(ifr));
    fd=socket(AF_INET,SOCK_DGRAM,0);
    if(fd<0)
    {   out("获取IP,socket打开失败\n");
        return -1;
    }
    memcpy(ifr.ifr_name,eth,strlen(eth));
    addr=(struct sockaddr_in *)&(ifr.ifr_addr);
    memset(ip_asc,0,sizeof(ip_asc));    
    ip2ip_asc(ip,ip_asc);
    out("设置的ip=[%s]\n",ip_asc);
    addr->sin_family = AF_INET;
    inet_aton(ip_asc,&(addr->sin_addr));
    if(ioctl(fd, SIOCSIFADDR,&ifr) == -1)
    {   close(fd);
        out("设置ip失败\n");
        return -1;
    }
    #if 0
    ifr.ifr_flags |= IFF_UP;
    if( ioctl( fd, SIOCSIFFLAGS, &ifr) == -1)
    {   close(fd);
        out("网卡启动失败\n");
        return -1;
    }
    #endif
    close(fd);
    out("ip设置成功\n");
    return 0;
}

uint32 get_mask(char *eth)
{   int fd;
    struct ifreq ifr;
    struct sockaddr_in *addr;
    char ip_asc[100];
    //out("获取[%s]子网掩码\n",eth);
    memset(&ifr,0,sizeof(ifr));
    fd=socket(AF_INET,SOCK_DGRAM,0);
    if(fd<0)
    {   out("获取子网掩码,socket打开失败\n");
        return 0;
    }
    memcpy(ifr.ifr_name,eth,strlen(eth));
    if(ioctl(fd,SIOCGIFNETMASK,&ifr)==-1)
    {   close(fd);
        out("获取eth=[%s]的子网掩码失败\n",eth);
        return 0;
    }
    addr=(struct sockaddr_in *)&(ifr.ifr_addr);
    memset(ip_asc,0,sizeof(ip_asc));
    inet_ntop(AF_INET,&addr->sin_addr,ip_asc,sizeof(ip_asc));
    //out("子网掩码为:[%s]\n",ip_asc);
    close(fd);
    return ip_asc2ip(ip_asc,strlen(ip_asc));   
}

int get_net_mac(char *eth, char *mac)
{
	struct ifreq ifreq;
	//int i;
	//unsigned char mac[15];
	int sock;
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock<0)
	{
		out("打开socket失败\n");
		return -1;
	}
	strcpy(ifreq.ifr_name, eth);
	if (ioctl(sock, SIOCGIFHWADDR, &ifreq)<0)
	{
		out("获取本机MAC失败\n");
		close(sock);
		return -2;
	}
	out("获取本机MAC成功\n");
	out("%02X:%02X:%02X:%02X:%02X:%02X\n", (unsigned char)ifreq.ifr_hwaddr.sa_data[0],
		(unsigned char)ifreq.ifr_hwaddr.sa_data[1],
		(unsigned char)ifreq.ifr_hwaddr.sa_data[2],
		(unsigned char)ifreq.ifr_hwaddr.sa_data[3],
		(unsigned char)ifreq.ifr_hwaddr.sa_data[4],
		(unsigned char)ifreq.ifr_hwaddr.sa_data[5]);

	sprintf((char *)mac, "%02X:%02X:%02X:%02X:%02X:%02X", (unsigned char)ifreq.ifr_hwaddr.sa_data[0],
		(unsigned char)ifreq.ifr_hwaddr.sa_data[1],
		(unsigned char)ifreq.ifr_hwaddr.sa_data[2],
		(unsigned char)ifreq.ifr_hwaddr.sa_data[3],
		(unsigned char)ifreq.ifr_hwaddr.sa_data[4],
		(unsigned char)ifreq.ifr_hwaddr.sa_data[5]);
	//out("mac =");
	//for (i = 0; i<15; i++)
	//{
	//	out("%02x \n", mac[i]);
	//}
	//out("\n");
	//printf("mac = %s\n", mac);
	//memcpy(macaddr, mac, 15);
	close(sock);
	return 0;
}

int set_mask(uint32 ip,char *eth)
{   int fd;
    int req;
    struct ifreq ifr;
    struct sockaddr_in *addr;
    char ip_asc[100];
    out("设置子网掩码\n");
    memset(&ifr,0,sizeof(ifr));
    fd=socket(AF_INET,SOCK_DGRAM,0);
    if(fd<0)
    {   out("设置子网掩码失败,socket打开失败\n");
        return -1;
    }
    memcpy(ifr.ifr_name,eth,strlen(eth));
    addr=(struct sockaddr_in *)&(ifr.ifr_addr);
    memset(ip_asc,0,sizeof(ip_asc));
    ip2ip_asc(ip,ip_asc);
    out("设置的子网掩码=[%s]\n",ip_asc);
    addr->sin_family = AF_INET;
    inet_aton(ip_asc,&(addr->sin_addr));
    out("addr->sin_addr = %08x\n",addr->sin_addr);
    req =ioctl(fd, SIOCSIFNETMASK,&ifr);
    if(req<0)
    {   close(fd);
        out("设置子网掩码失败req = %d\n",req);
        return -1;
    }
    close(fd);
    out("子网掩码成功\n");
    return 0; 
}

int get_gateway(char *eth,uint32 getway[],int size_get)
{   char file[DF_TY_PATH_MAX];
    //int fd;
    char buf[1024];
    char *p;
    uint32 dest,way,flag,ip,i;
    char net_name[100];
    //out("获取路由列表\n");
    memset(file,0,sizeof(file));
    sprintf(file,"%s/%s",DF_LINUX_NET_DIR,"route");
    memset(buf,0,sizeof(buf));
    if(read_file(file,buf,sizeof(buf))<0)
    {   //out("获取路由文件失败:[%s]\n" ,file);
        return 0;
    }
    for(p=buf,i=0;;)
    {   memset(net_name,0,sizeof(net_name));
        sscanf(p,"%s\t%x\t%x\t%x",net_name,&dest,&way,&flag);   
        //out("name=[%s],dest=[0X%08X],way=[0X%08X],flag=[0X%08X]\n",net_name,dest,way,flag);
        p=strstr(p,"\n");
        if(p==NULL)
            break;
        p++;
        if(p==NULL)
            break;
        if(memcmp(net_name,eth,strlen(eth))!=0)
            continue;
        if(dest!=0)
            continue;
        if(flag!=3)
            continue;
        //out("找到有效的路由信息,way = %08x\n",way);
        ip=((way&0xff)<<24)+(((way>>8)&0xff)<<16)+(((way>>16)&0xff)<<8)+(((way>>24)&0xff)<<0);
        getway[i++]=ip;
        if(i>=size_get)
            break;
    }
    //out("找到可用路由信息:i=%d\n",i);
    return i;  
}

//int set_gateway(uint32 ip,char *eth)
//{   int fd;
//    struct ifreq ifr;
//    struct sockaddr_in *addr;
//		struct rtentry  rt;
//    char gateway_asc[100];
//    out("增加网关\n");
//    memset(&ifr,0,sizeof(ifr));
//    fd=socket(AF_INET,SOCK_DGRAM,0);
//    if(fd<0)
//    {   out("增加网关,socket打开失败\n");
//        return -1;
//    }
//	memset(&ifr,0,sizeof(ifr));
//	memset(&rt,0,sizeof(struct rtentry));
//	//memset(addr,0,sizeof(struct sockaddr_in));
//	memcpy(ifr.ifr_name,eth,strlen(eth));
//    addr=(struct sockaddr_in *)&(ifr.ifr_addr);
//	memset(gateway_asc,0,sizeof(gateway_asc));    
//    ip2ip_asc(ip,gateway_asc);
//    out("设置的网关=[%s]\n",gateway_asc);
//    addr->sin_family = AF_INET;
//	addr->sin_port = 0;
//    if(inet_aton(gateway_asc,&(addr->sin_addr))<0)
//	{
//		out("inet_aton error\n");
//	}
//	memcpy (&rt.rt_gateway, addr, sizeof(struct sockaddr_in));
//	((struct sockaddr_in *)&rt.rt_dst)->sin_family=AF_INET;
//    ((struct sockaddr_in *)&rt.rt_genmask)->sin_family=AF_INET;
//    rt.rt_flags = RTF_GATEWAY;
//    if(ioctl(fd, SIOCADDRT,&rt) < 0)
//    {   close(fd);
//        out("设置网关失败\n");
//        return -1;
//    }
//	return 0;
//           
//}






//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//内部读写配置文件函数
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
int get_a20_net_file(char *eth,struct _net_define  *net,uint8 *mac)//获取配置文件中IP等参数
{	char buf[2048];
	char *p_file_head,*p_file_end;
	memset(buf,0,sizeof(buf));
	if(read_file(DF_LINUX_NETWORK_FILE,buf,sizeof(buf))<0)
	{	out("读取文件:[%s]失败\n",DF_LINUX_NETWORK_FILE);
		return -1;
	}	
	//out("buf=[%s]\n",buf);
	if(get_file_head_end(buf,"config interface \'wan\'","config interface \'wan6\'",&p_file_head,&p_file_end)<0)
	{	out("ip文件内容错误\n",eth);
	   	return -2;
	}
	if(get_a20_fe("option proto \'",p_file_head,p_file_end,net,mac)<0)
	{	out("获取IP地址失败\n");
		return -3;
	}
	out("MAC地址获取成功\n");
	p_file_head = NULL;
	p_file_end = NULL;
	net->DNS = net->gateway[0];
	/*memset(buf,0,sizeof(buf));
	if(read_file(DF_LINUX_DNS,buf,sizeof(buf))<0)
	{	out("读取文件:[%s]失败\n",DF_LINUX_NETWORK_FILE);
		return -1;
	}
	if(get_file_head_end(buf,"nameserver","\n\n\n",&p_file_head,&p_file_end)<0)
	{	out("dns文件内容错误\n",eth);
	   	return -4;
	}
	if(get_dns("nameserver ",p_file_head,p_file_end,net)<0)
	{	out("获取IP地址失败\n");
		return -3;
	}*/
	out("获取所有网络信息成功\n");
	return 0;	
}

static int get_a20_fe(char *macname1,char *str,char *end,struct _net_define  *net,uint8 *macaddr)
{	char *ps,*end_ps;	
	char buf[30];
	//char mask[2];
	//int i;
	//int netmask;
	char macname[100];

	out("查找网络类型\n");
	ps=strstr(str,macname1);
	if((ps==NULL)||(ps>end))
	{	out("without:[%s]\n",macname1);
		return -1;
	}
	out("find %s\n",macname1);
	ps+=strlen(macname1);
	str = ps;
	end_ps=strstr(str,"\'");
	if((end_ps==NULL)||(end_ps>end))
	{	out("with out:\"\n");
		return -2;
	}
	//i=end_ps-ps;
	out("i = %d\n", end_ps - ps);
	memset(buf,0,sizeof(buf));
	memcpy(buf,ps,end_ps-ps);	
	if(memcmp(buf,"static",strlen("static"))==0)
	{
		out("网络类型为static\n");
		net->type = 2;

	}
	else if(memcmp(buf,"dhcp",strlen("dhcp"))==0)
	{
		out("网络类型为dhcp\n");
		net->type = 1;
	}
	out("获取网络类型成功,网络类型为%d\n",net->type);


	out("查找IP\n");
	memset(macname,0,sizeof(macname));
	sprintf(macname,"%s","option ipaddr \'");
	ps = end_ps;//指向\n\n之后
	ps=strstr(str,macname);
	if((ps==NULL)||(ps>end))
	{	out("without:[%s]\n",macname);
		return -1;
	}
	out("find %s\n",macname);
	ps+=strlen(macname);
	str = ps;
	end_ps=strstr(str,"\'");
	if((end_ps==NULL)||(end_ps>end))
	{	out("with out:\"\n");
		return -2;
	}
	//i=end_ps-ps;
	out("i = %d\n", end_ps - ps);
	memset(buf,0,sizeof(buf));
	memcpy(buf,ps,end_ps-ps);	
	cls_no_asc(buf,strlen(buf));
	net->ip=ip_asc2ip(buf,strlen(buf));
	out("*ip=0x%08X\n",net->ip);
	out("找到IP\n");

	out("查找netmask\n");
	memset(macname,0,sizeof(macname));
	sprintf(macname,"%s","option netmask \'");
	ps = end_ps;//指向\n\n之后
	ps=strstr(str,macname);
	if((ps==NULL)||(ps>end))
	{	out("without:[%s]\n",macname);
		return -1;
	}
	out("find %s\n",macname);
	ps+=strlen(macname);
	str = ps;
	end_ps=strstr(str,"\'");
	if((end_ps==NULL)||(end_ps>end))
	{	out("with out:\"\n");
		return -2;
	}
	//i=end_ps-ps;
	out("i = %d\n", end_ps - ps);
	memset(buf,0,sizeof(buf));
	memcpy(buf,ps,end_ps-ps);	
	cls_no_asc(buf,strlen(buf));
	net->mask=ip_asc2ip(buf,strlen(buf));
	out("*ip=0x%08X\n",net->mask);
	out("找到mask\n");



	out("查找gateway\n");
	memset(macname,0,sizeof(macname));
	sprintf(macname,"%s","option gateway \'");
	ps = end_ps;//指向\n\n之后
	ps=strstr(str,macname);
	if((ps==NULL)||(ps>end))
	{	out("without:[%s]\n",macname);
		return -1;
	}
	out("find %s\n",macname);
	ps+=strlen(macname);
	str = ps;
	end_ps=strstr(str,"\'");
	if((end_ps==NULL)||(end_ps>end))
	{	out("with out:\"\n");
		return -2;
	}
	//i=end_ps-ps;
	out("i = %d\n", end_ps - ps);
	memset(buf,0,sizeof(buf));
	memcpy(buf,ps,end_ps-ps);	
	cls_no_asc(buf,strlen(buf));
	net->gateway[0]=ip_asc2ip(buf,strlen(buf));
	out("*gateway=0x%08X\n",net->gateway[0]);
	out("找到gateway\n");
	return 0;
}

static int get_dhcp_static(char *name,char *str,char *end,char  *net_type)
{
	char *ps,*end_ps;	
	char buf[20];
	//int i;
	ps=strstr(str,name);//指向第开头
	if((ps==NULL)||(ps>end))
	{	out("3without:[%s]\n",name);
		return -1;
	}
	//out("find the third nameserver\n");
	ps+=strlen(name);//指向第三个nameserver 结尾
	str = ps;
	
	end_ps=strstr(str,"\'");
	if((end_ps==NULL)||(end_ps>end))
	{	out("with out:enter \n");
		return -2;
	}
	//i =  end_ps-ps;
	//out("i= %d \n",i);
	//out("test1\n");
	memset(buf,0,sizeof(buf));
	//out("test2\n");
	memcpy(buf,ps,end_ps-ps);	
	//cls_no_asc(buf,strlen(buf));
	//net->DNS=ip_asc2ip(buf,strlen(buf));
	//out("*dns=0x%08X\n",net->DNS);
	//out("找到dns\n");
	//out("test3\n");
	sprintf(net_type,"%s",buf);
	//out("IP TYPE %s\n",net_type);
	return 0;
}

//static int get_dns(char *name,char *str,char *end,struct _net_define  *net)
//{
//	char *ps,*end_ps;	
//	char buf[20];
//	int i;
//	ps=strstr(str,name);//指向第一个nameserver 开头
//	if((ps==NULL)||(ps>end))
//	{	out("1without:[%s]\n",name);
//		return -1;
//	}
//	out("find the first nameserver\n");
//	ps+=strlen(name);//指向第一个nameserver 结尾
//	str = ps;
//	ps=strstr(str,name);//指向第二个nameserver 开头
//	if((ps==NULL)||(ps>end))
//	{	out("2without:[%s]\n",name);
//		return -1;
//	}
//	out("find the second nameserver\n");
//	ps+=strlen(name);//指向第二个nameserver 结尾
//	str = ps;
//	ps=strstr(str,name);//指向第三个nameserver 开头
//	if((ps==NULL)||(ps>end))
//	{	out("3without:[%s]\n",name);
//		return -1;
//	}
//	out("find the third nameserver\n");
//	ps+=strlen(name);//指向第三个nameserver 结尾
//	str = ps;
//	
//	end_ps=strstr(str,"\n");
//	if((end_ps==NULL)||(end_ps>end))
//	{	out("with out:enter \n");
//		return -2;
//	}
//	i =  end_ps-ps;
//	out("i= %d \n",i);
//	if(i>sizeof(buf))
//	{	out("不符合DNS地址设置规范\n");
//		return -3;
//	}
//	memset(buf,0,sizeof(buf));
//	memcpy(buf,ps,end_ps-ps);	
//	cls_no_asc(buf,strlen(buf));
//	net->DNS=ip_asc2ip(buf,strlen(buf));
//	out("*dns=0x%08X\n",net->DNS);
//	out("找到dns\n");
//	return 0;
//}


static int get_file_head_end(const char *buf,const char *eth,const char *end,char **p_head,char **p_end)
{	char lan[100];
	memset(lan,0,sizeof(lan));
	//sprintf(lan,"#!/bin/sh");
	sprintf(lan,eth);
	*p_head=strstr(buf,lan);
	if(*p_head==NULL)
	{	out("[%s]未找到\n",lan);
		return -1;
	}
	*p_end=strstr(*p_head,end);
	if(*p_end==NULL)
	{	out("未找到数据结束\n");
		return -2;
	}
	//*p_head+=strlen(lan);
	*p_end+=2;
	return 0;
}

//static int get_fe(char *name,char *str,char *end,struct _net_define  *net)
//{	char *ps,*end_ps;	
//	char buf[30];
//	char mask[2];
//	int i;
//	int netmask;
//	ps=strstr(str,name);//指向第一个ifconfig eth0 开头
//	if((ps==NULL)||(ps>end))
//	{	out("without:[%s]\n",name);
//		return -1;
//	}
//	out("find the first ifconfig eth0\n");
//	ps+=strlen(name);//指向第一个ifconfig eth0 结尾
//	str = ps;
//	ps=strstr(str,name);//指向第二个ifconfig eth0 开头
//	if((ps==NULL)||(ps>end))
//	{	out("without:[%s]\n",name);
//		return -1;
//	}
//	out("find the second ifconfig eth0\n");
//	ps+=strlen(name);//指向第二个ifconfig eth0 结尾
//	str = ps;
//	end_ps=strstr(ps,"/");
//	if((end_ps==NULL)||(end_ps>end))
//	{	out("with out:/\n");
//		return -2;
//	}
//	i =  end_ps-ps;
//	if(i>sizeof(buf))
//	{	out("不符合ip地址设置规范\n");
//		return -3;
//	}
//	memset(buf,0,sizeof(buf));
//	memcpy(buf,ps,end_ps-ps);	
//	cls_no_asc(buf,strlen(buf));
//	net->ip=ip_asc2ip(buf,strlen(buf));
//	out("*ip=0x%08X\n",net->ip);
//	out("找到IP\n");
//	
//	ps = end_ps+1;//指向ifconfig eth0 192.168.1.253/之后
//	str = ps;
//	end_ps = strstr(str,"\n");
//	if((end_ps==NULL)||(end_ps>end))
//	{	out("没有:[换行]\n");
//		return -2;
//	}
//	i =  end_ps-ps;
//	if(i>4)
//	{	out("不符合子网掩码地址设置规范\n");
//		return -3;
//	}
//	memset(mask,0,sizeof(buf));
//	memcpy(mask,ps,end_ps-ps);
//	netmask = atoi(mask);
//	out("netmask = %d\n",netmask);
//	i = dton(netmask);
//	net->mask = strap_32(i);
//	//net->mask = dton(netmask);
//	out("net->mask=0x%08X\n",net->mask);
//	out("找到子网掩码\n");
//	
//	ps = end_ps+1;//指向ifconfig eth0 192.168.1.253/24\n之后
//	str = ps ;
//	ps = strstr(str,"route add default gw ");
//	ps+=strlen("route add default gw ");//指向route add default gw 之后
//	end_ps=strstr(ps,"\n");
//	if((end_ps==NULL)||(end_ps>end))
//	{	out("没有:[换行]\n");
//		return -2;
//	}
//	i =  end_ps-ps;
//	if(i>sizeof(buf))
//	{	out("不符合ip地址设置规范\n");
//		return -3;
//	}
//	memset(buf,0,sizeof(buf));
//	memcpy(buf,ps,end_ps-ps);	
//	cls_no_asc(buf,strlen(buf));
//	net->gateway[0]=ip_asc2ip(buf,strlen(buf));
//	out("net->gateway=0x%08X\n",net->gateway[0]);
//	out("找到网关\n");
//	return 0;
//}

//static int dton(int mask) 
//{	uint32 i;
//    int bits = sizeof(uint32) * 8; 
//    i = ~0;
//    bits -= mask;
//    i <<= bits;
//	return htonl(i);
//}

//static int ntod(uint32 mask) 
//{   
//	float f;
//    mask = -mask; 
//    f = mask; 
//    mask = *(unsigned *)&f; 
//    mask = 159 - (mask >> 23); 
//    return mask;
//}

//static int strap_32(int data)
//{
//	int result;
//	int temp[4];
//	temp[0]=data>>24&0xff;
//	temp[1]=data>>16&0xff;
//	temp[2]=data>>8&0xff;
//	temp[3]=data&0xff;
//	out("temp[0]=%d temp[1]=%d temp[2]=%d temp[3]=%d\n",temp[0],temp[1],temp[2],temp[3]);
//	result = (temp[3]<<24&0xff000000) + (temp[2]<<16&0xff0000) + (temp[1]<<8&0xff00)+(temp[0]&0xff);
//	out("result = %08x \n",result);
//	return result;
//}
static int set_DNS_value(struct _net_define  *net)
{
	char DNS[100];
	char gateway[100];
	char data[1000];
	memset(DNS,0,sizeof(DNS));
	memset(gateway,0,sizeof(gateway));
	memset(data, 0, sizeof(data));
	ip2ip_asc(net->DNS, DNS);
	out("DNS地址为%s\n", DNS);
	ip2ip_asc(net->gateway[0], gateway);
	out("网关地址为%s\n", gateway);
	sprintf(data, "nameserver %s\nnameserver %s\nnameserver %s\n\n\n", DNS ,"114.114.114.114", "223.6.6.6");

	remove(DF_LINUX_DNS);
	FILE* f = fopen(DF_LINUX_DNS, "w+");
	if (f == NULL)
		return 0;
	fputs(data, f);
	fclose(f);
	sync();
	return 0;
}
//int change_mtk4_wifi_wireless_file(char *eth, struct _net_define  *net)
//{
//	char buf[2048];
//	char *p_file_head, *p_file_end;
//	char *iface = NULL, *name = NULL;
//	memset(buf, 0, sizeof(buf));
//	if (read_file(DF_LINUXWIRELESS_FILE, buf, sizeof(buf))<0)
//	{
//		out("读取文件:[%s]失败\n", DF_LINUXWIRELESS_FILE);
//		return -1;
//	}
//	//out("buf=[%s]\n",buf);
//	iface = strstr("wifi-iface", buf);
//	if (iface == NULL)
//	{
//		return -1;
//	}
//	name = strstr(net->wifi_name, iface);
//	if (name == NULL)
//	{
//		set_net_config_file("config interface ", "wan", "	option proto ", "dhcp");
//		set_net_config_file("config interface ", "wan", "	option ifname ", "apcli0");
//		del_net_config_file_line("config interface ", "wan", "	option ipaddr ");
//		del_net_config_file_line("config interface ", "wan", "	option netmask ");
//		del_net_config_file_line("config interface ", "wan", "	option gateway ");
//		del_net_config_file_line("config interface ", "wan", "	option broadcast ");
//
//	}
//	return 0;
//}
int change_mtk4_wifi_netconfig_file(char *eth, struct _net_define  *net)//获取配置文件中IP等参数
{
	char buf[2048];
	//char *p_file_head, *p_file_end;
	char *wan = NULL,*name = NULL;
	memset(buf, 0, sizeof(buf));
	if (read_file(DF_LINUX_NETWORK_FILE, buf, sizeof(buf))<0)
	{
		out("读取文件:[%s]失败\n", DF_LINUX_NETWORK_FILE);
		return -1;
	}
	//out("buf=[%s]\n",buf);
	wan = strstr(buf, "config interface \'wan\'");
	if (wan == NULL)
	{
		return -1;
	}
	name = strstr(wan, net->wifi_name);
	if (name == NULL)
	{
		set_net_config_file("config interface ", "wan", "	option proto ", "dhcp");
		set_net_config_file("config interface ", "wan", "	option ifname ", "apcli0");
		del_net_config_file_line("config interface ", "wan", "	option ipaddr ");
		del_net_config_file_line("config interface ", "wan", "	option netmask ");
		del_net_config_file_line("config interface ", "wan", "	option gateway ");
		del_net_config_file_line("config interface ", "wan", "	option broadcast ");
		
	}
	return 0;
	/*if (get_file_head_end(buf, "config interface \'wan\'", "config interface \'wan6\'", &p_file_head, &p_file_end)<0)
	{
		out("ip文件内容错误\n", eth);
		return -2;
	}
	if (get_a20_fe("option proto \'", p_file_head, p_file_end, net, mac)<0)
	{
		out("获取IP地址失败\n");
		return -3;
	}
	out("MAC地址获取成功\n");
	p_file_head = NULL;
	p_file_end = NULL;

	out("获取所有网络信息成功\n");*/
	return 0;
}
int set_net_file_wifi(char *eth, struct _net_define  *net)//通过UCI 命令改变wireless 和network内容然后重启网络
{
	//int result;
	char cmd[1024] = { 0 };
	sprintf(cmd, "uci set wireless.@wifi-iface[0].ApCliEnable=1&&uci set wireless.@wifi-iface[0].ApCliSsid=%s&&uci set wireless.@wifi-iface[0].ApCliAuthMode=%s"
		"&&uci set wireless.@wifi-iface[0].ApCliEncrypType=AES&&uci set wireless.@wifi-iface[0].ApCliPassWord=%s",net->wifi_name,net->wifi_mode,net->wifi_pwd);
	system(cmd);
	printf("cmd :%s\n", cmd);
	sync();
	memset(cmd, 0, sizeof(cmd));
	sprintf(cmd, "uci commit");
	system(cmd);
	sync();

	set_net_config_file("config interface ", "wan", "	option ifname ", "apcli0");

	memset(cmd, 0, sizeof(cmd));
	sprintf(cmd, "/etc/init.d/network restart");
	system(cmd);
	sync();
	//result = change_mtk4_wifi_netconfig_file(eth, net);
	//
	//result = change_mtk4_wifi_wireless_file(eth, net);
	return 0;

}
int set_net_4g(char *eth, struct _net_define  *net)
{
	//int result;
	char cmd[200] = { 0 };
	char ip[30] = { 0 };
	//启动拨号程序
	sprintf(cmd, "cd /etc/ppp/peers/");
	system(cmd);
	
	sync();
	memset(cmd, 0, sizeof(cmd));
	sprintf(cmd, "pppd call quectel-ppp &");
	system(cmd);
	sync();
	sleep(7);
	//FILE *stream;
	char buf1[2048] = { 0 };
	char *p = NULL,*end = NULL;
	int count = 0;
	//获取拨号成功后的IP
RELOAD:
	memset(buf1, 0, sizeof(buf1));

	memset(cmd, 0, sizeof(cmd));
	sprintf(cmd, "ifconfig ppp0");
	mb_system_read(cmd, buf1, sizeof(buf1), "/tmp/ifconfig_ppp0.txt");

	//stream = popen("ifconfig ppp0", "r"); //将命令的输出 通过管道读取（“r”参数）到FILE* stream  
	//fread(buf1, sizeof(char), sizeof(buf1), stream); //将刚刚FILE* stream的数据流读取到buf中
	//pclose(stream);
	printf("ppp0 info:%s\n", buf1);
	p = strstr(buf1, "P-t-P:");
	//p = strstr(buf1, "inet addr:");
	if (p == NULL){
		count++;
		sleep(1);
		if (count < 5)
			goto RELOAD;
		else
			return -1;
	}
		
	end = strstr(p + 6, " ");


	memcpy(ip, p + 6, end - (p + 6));
	//设置默认网关，即路由优先级
	memset(cmd, 0, sizeof(cmd));
	sprintf(cmd, "route del default");
	system(cmd);
	sync();

	
	
	memset(cmd, 0, sizeof(cmd));
	sprintf(cmd, "route add default gw %s", ip);
	printf("route cmd :\n%s\n", cmd);
	system(cmd);
	sync();


	remove(DF_LINUX_DNS);
	FILE* f = fopen(DF_LINUX_DNS, "w+");
	if (f == NULL)
		return -1;
	char data[300] = { 0 };
	sprintf(data, "nameserver 114.114.114.114");
	fputs(data, f);
	fclose(f);
	sync();

	char ip_t[30] = { 0 };
	int result,i;
	for (i = 0; i < 3; i++)
	{
		result = get_DNS_ip("www.baidu.com", ip_t);
		if (result < 0)
		{
			printf("SEEETT ROUTE AGAIN!\n");
			memset(cmd, 0, sizeof(cmd));
			sprintf(cmd, "route add default gw %s", ip);
			printf("route cmd :\n%s\n", cmd);
			system(cmd);
			sync();
			sleep(1);
			continue;
		}
	}
	return 0;
}
int set_net_file(char *eth,struct _net_define  *net)
{	struct _net_define  o_net;
	//char buf[2048];
	
	uint8 mac[120];
	//uint8 benjimac[15];
	//char w_buf[2048];
	//char set_buf[500];
	//char *p_head,*p_end;
	int result;
	//char cmd[100] = { 0 };
	uint32 net_ip;
	uint32 net_mask;
	uint32 net_broadcast;
	char ip_asc[100];
	char mask_asc[100];
	char gatewaybuf[100];
	char broadcast[100];
	memset(&o_net,0,sizeof(o_net));
	out("修改DNS值\n");
	result = set_DNS_value(net);

	out("set_net_file \n");
	result = get_a20_net_file(eth, &o_net, mac);
	if (result == 0)
	{
		out("已找到本机网络设置\n");

		if (net->type == 1)
		{
			set_net_config_file("config interface ", "wan", "	option ifname ", "eth0.2");
			set_net_config_file("config interface ", "wan", "	option proto ", "dhcp");
			del_net_config_file_line("config interface ", "wan", "	option ipaddr ");
			del_net_config_file_line("config interface ", "wan", "	option netmask ");
			del_net_config_file_line("config interface ", "wan", "	option gateway ");
			del_net_config_file_line("config interface ", "wan", "	option broadcast ");

			/*sprintf(cmd, "uci set wireless.@wifi-iface[0].ApCliEnable=0");
			system(cmd);
			sync();
			memset(cmd, 0, sizeof(cmd));
			sprintf(cmd, "uci commit");
			system(cmd);
			sync();*/
			
			return 0;
		}
		//if(memcmp(net->ip,&o_net.ip,sizeof(uint32))==0)
		//{	out("IP已经与设置结果一样了\n");
		//	return 0;
		//}
		out("net_file gateway = %08x\n", o_net.gateway[0]);
		out("net_file ip = %08x\n", o_net.ip);
		out("net_file netmask = %08x\n", o_net.mask);
		out("net_file NDS = %08x\n", o_net.DNS);
		if ((net->ip == o_net.ip) && (net->DNS == o_net.DNS) && (net->mask == o_net.mask) && (net->gateway[0] == o_net.gateway[0]))
		{
			/*sprintf(cmd, "uci set wireless.@wifi-iface[0].ApCliEnable=0");
			system(cmd);
			sync();

			memset(cmd, 0, sizeof(cmd));
			sprintf(cmd, "uci commit");
			system(cmd);
			sync();*/
			
			out("IP已经与设置结果一样了\n");
			return -1;
		}

	}

	out("set_net_file gateway = %08x\n",net->gateway[0]);
	out("set_net_file ip = %08x\n",net->ip);
	out("set_net_file netmask = %08x\n",net->mask);
	out("set_net_file NDS = %08x\n",net->DNS);
	//get_mac_addr(benjimac);
	if(net->type==1)
	{
		set_net_config_file("config interface ", "wan", "	option ifname ", "eth0.2");
		set_net_config_file("config interface ","wan","	option proto ","dhcp");
		del_net_config_file_line("config interface ","wan","	option ipaddr ");
		del_net_config_file_line("config interface ","wan","	option netmask ");
		del_net_config_file_line("config interface ","wan","	option gateway ");
		del_net_config_file_line("config interface ","wan","	option broadcast ");

		/*sprintf(cmd, "uci set wireless.@wifi-iface[0].ApCliEnable=0");
		system(cmd);
		sync();
		memset(cmd, 0, sizeof(cmd));
		sprintf(cmd, "uci commit");
		system(cmd);
		sync();*/
		
		return 0;
	}
	memset(ip_asc,0,sizeof(ip_asc));
	memset(mask_asc,0,sizeof(mask_asc));
	memset(gatewaybuf,0,sizeof(gatewaybuf));
	memset(broadcast,0,sizeof(broadcast));
	ip2ip_asc(net->ip, ip_asc);	
	ip2ip_asc(net->mask, mask_asc);	
	ip2ip_asc(net->gateway[0], gatewaybuf);	
	out("获取网络地址\n");
	net_ip = net->ip & net->mask;
	out("网络地址为%08x\n",net_ip);
	out("获取广播地址\n");
	net_mask = ~net->mask;
	out("子网掩码取反为%08x\n",net_mask);
	net_broadcast = net_ip | net_mask;
	out("广播地址为%08x\n",net_broadcast);
	ip2ip_asc(net_broadcast, broadcast);	
	out("广播地址为%s\n",broadcast);

	out("要设置的IP为%s,子网掩码为%s,网关为%s\n",ip_asc,mask_asc,gatewaybuf);
	set_net_config_file("config interface ", "wan", "	option ifname ", "eth0.2");
	set_net_config_file("config interface ","wan","	option proto ","static");
	set_net_config_file("config interface ","wan","	option ipaddr ",ip_asc); //参数为配置项 + 配置值  
    set_net_config_file("config interface ","wan","	option netmask ",mask_asc);   
    set_net_config_file("config interface ","wan","	option gateway ",gatewaybuf);   
	set_net_config_file("config interface ","wan","	option broadcast ",broadcast);  

	/*sprintf(cmd, "uci set wireless.@wifi-iface[0].ApCliEnable=0");
	system(cmd);
	sync();
	memset(cmd, 0, sizeof(cmd));
	sprintf(cmd, "uci commit");
	system(cmd);
	sync();*/
	/*memset(cmd, 0, sizeof(cmd));
	sprintf(cmd, "/etc/init.d/network restart");
	system(cmd);
	sync();*/
	//system("/etc/init.d/network restart");
	set_net_1(eth, net);
	return 0;
}

static int del_net_config_file_line(char *config_inter,char *groupname,char *configName)
{
	int i=0;
	//int len1; 
	int configLen;	
	
	//int configBufferLen;
	char lineBuff[256];// 存储读取一行的数据  
	char lineBuff1[256];
    memset(lineBuff,0,256);    
    FILE* fp  = fopen(DF_LINUX_NETWORK_FILE,"r");  
    if(fp == NULL)  
    {
		out("文件打开失败\n");
		return -1;
	}  
    fseek(fp,0,SEEK_END);  
    configLen = ftell(fp); //配置文件长度  
    fseek(fp,0,SEEK_SET);  
    //configBufferLen = strlen(configBuffer);  
    char sumBuf[configLen+1024];  
	memset(sumBuf, 0, sizeof(sumBuf)); //原文件长度 + 修改字符长度  
    while(fgets(lineBuff,256,fp) != NULL)  
    {	if(strlen(lineBuff) < 4) // 空行  
        {  strcat(sumBuf,lineBuff);  
            continue;  
        }  
		out("处理此行：%s",lineBuff);
        char* linePos = NULL;  //找到‘号
        linePos = strstr(lineBuff,"\'"); // 定位到‘  
        if(linePos == NULL)  
        {	strcat(sumBuf,lineBuff);  
            continue;  
        }  
        int lineNum = linePos - lineBuff;  //获取到定位位置有多少字节
        char lineName[lineNum + 1];			
        memset(lineName,0,sizeof(lineName));  
        strncpy(lineName,lineBuff,lineNum);  
		out("lineName =%s\n",lineName);	
		char* lineEnd = NULL;
		lineEnd = strstr(lineBuff+lineNum+1,"\'"); // 定位到’
        if(lineEnd == NULL)  
        {	strcat(sumBuf,lineBuff);  
            continue;  
        }
		int len = lineEnd-lineBuff-lineNum-1;
		out("c_name len num = %d\n",len);
		char c_name[len+1];
		memset(c_name,0,sizeof(c_name));
		strncpy(c_name,linePos+1,len);
		out("c_name = %s\n",c_name);
        if(lineName[0] == '#')  
        {  
            for(i = 0; i < lineNum;i++)  
            lineName[i] = lineName[i+1];  
        }  
        //"=" 前匹配  
        if((strcmp(config_inter,lineName) == 0)&&(strcmp(groupname,c_name) == 0))
        {	strcat(sumBuf,lineBuff);
			out("找到当前组，在此组中查找配置成员\n");
			while(fgets(lineBuff1,256,fp) != NULL) 
			{	if(strlen(lineBuff1) < 4) // 空行  
				{  	strcat(sumBuf,lineBuff1);  
					continue;  
				}  
				out("处理此行：%s",lineBuff1);
				char* linePos1 = NULL;  //找到‘号
				linePos1 = strstr(lineBuff1,"\'"); // 定位到‘  
				if(linePos1 == NULL)  
				{	strcat(sumBuf,lineBuff1);  
					continue;  
				}  
				int lineNum1 = linePos1 - lineBuff1;  //获取到定位位置有多少字节
				out("lineNum1 = %d\n",lineNum1);
				char lineName1[lineNum1 + 1];			
				memset(lineName1,0,sizeof(lineName1));  
				strncpy(lineName1,lineBuff1,lineNum1);  
				if(lineName1[0] == '#')  
				{  	for(i = 0; i < lineNum1;i++)  
					lineName1[i] = lineName1[i+1];  
				}  
				//"=" 前匹配  
				if((strcmp(configName,lineName1) == 0))
				{	out("匹配到要修改的位置,不进行任何操作直接下一次\n");					
					break;
				}
				else
				{	strcat(sumBuf,lineBuff1);  
				}					
			}
        }  
        else  
        {  strcat(sumBuf,lineBuff);  
        }  
    }  
	out("修改好的配置文件：%s\n\n\n\n\n",sumBuf);
    fclose(fp);  
    remove(DF_LINUX_NETWORK_FILE);  
    FILE* f = fopen(DF_LINUX_NETWORK_FILE,"w+");  
	if (f == NULL)
		return 0;
    fputs(sumBuf,f);  
    fclose(f);  
	sync();
	return 0;
}


static int set_net_config_file(char *config_inter,char *groupname,char *configName,char *configBuffer)
{	int i=0;
	//int len1; 
	int configLen;	
	
	int configBufferLen;
	int config_len;
	int group_len;
	char lineBuff[256];// 存储读取一行的数据  
	char lineBuff1[256];
    memset(lineBuff,0,256);    
    FILE* fp  = fopen(DF_LINUX_NETWORK_FILE,"r");  
    if(fp == NULL)  
    {
		out("文件打开失败\n");
		return -1;
	}  
    fseek(fp,0,SEEK_END);  
    configLen = ftell(fp); //配置文件长度  
    fseek(fp,0,SEEK_SET);  


    configBufferLen = strlen(configBuffer); 
	config_len = strlen(config_inter);
	group_len = strlen(groupname);
    char sumBuf[configLen+config_len+configBufferLen+group_len+500];  
	memset(sumBuf, 0, sizeof(sumBuf)); //原文件长度 + 修改字符长度  
    while(fgets(lineBuff,256,fp) != NULL)  
    {	if(strlen(lineBuff) < 4) // 空行  
        {  strcat(sumBuf,lineBuff);  
            continue;  
        }  
		//out("处理此行：%s",lineBuff);
        char* linePos = NULL;  //找到‘号
        linePos = strstr(lineBuff,"\'"); // 定位到‘  
        if(linePos == NULL)  
        {	strcat(sumBuf,lineBuff);  
            continue;  
        }  
        int lineNum = linePos - lineBuff;  //获取到定位位置有多少字节
        char lineName[lineNum + 1];			
        memset(lineName,0,sizeof(lineName));  
        strncpy(lineName,lineBuff,lineNum);  
		//out("lineName =%s\n",lineName);	



		char* lineEnd = NULL;
		lineEnd = strstr(lineBuff+lineNum+1,"\'"); // 定位到’
        if(lineEnd == NULL)  
        {	strcat(sumBuf,lineBuff);  
            continue;  
        }
		int len = lineEnd-lineBuff-lineNum-1;
		//out("c_name len num = %d\n",len);
		char c_name[len+1];
		memset(c_name,0,sizeof(c_name));
		strncpy(c_name,linePos+1,len);
		//out("c_name = %s\n",c_name);
        if(lineName[0] == '#')  
        {  
            for(i = 0; i < lineNum;i++)  
            lineName[i] = lineName[i+1];  
        }  
        //"=" 前匹配  
        if((strcmp(config_inter,lineName) == 0)&&(strcmp(groupname,c_name) == 0))
        {	strcat(sumBuf,lineBuff);
			//out("找到当前组，在此组中查找配置成员\n");
			while(fgets(lineBuff1,256,fp) != NULL) 
			{	if(strlen(lineBuff1) < 4) // 如果在空行前没有找到对应配置，则之间添加 
				{	//out("遇到空行，在空行前添加配置\n");
					strcat(sumBuf,configName);  
					strcat(sumBuf,"\'");
					strcat(sumBuf,configBuffer); 
					strcat(sumBuf,"\'\n");
					strcat(sumBuf,lineBuff1);  
					break;  
				}  
				//out("处理此行：%s",lineBuff1);
				char* linePos1 = NULL;  //找到‘号
				linePos1 = strstr(lineBuff1,"\'"); // 定位到‘  
				if(linePos1 == NULL)  
				{	strcat(sumBuf,lineBuff1);  
					continue;  
				}  
				
				int lineNum1 = linePos1 - lineBuff1;  //获取到定位位置有多少字节
				//out("lineNum1 = %d\n",lineNum1);
				char lineName1[lineNum1 + 1];			
				memset(lineName1,0,sizeof(lineName1));  
				strncpy(lineName1,lineBuff1,lineNum1);  
				if(lineName1[0] == '#')  
				{  
					for(i = 0; i < lineNum1;i++)  
					lineName1[i] = lineName1[i+1];  
				}  
				//"=" 前匹配  
				if((strcmp(configName,lineName1) == 0))
				{	//out("匹配到要修改的位置\n");
					strcat(sumBuf,configName);  
					strcat(sumBuf,"\'");
					strcat(sumBuf,configBuffer); 
					strcat(sumBuf,"\'\n");
					break;
				}
				else
				{
					strcat(sumBuf,lineBuff1);  
				}					
			}
        }  
        else  
        {  
            strcat(sumBuf,lineBuff);  
        }  
    }  
	//out("修改好的配置文件：%s\n\n\n\n\n",sumBuf);
    fclose(fp);  
    remove(DF_LINUX_NETWORK_FILE);  
    FILE* f = fopen(DF_LINUX_NETWORK_FILE,"w+");  
	if (f == NULL)
		return 0;
    fputs(sumBuf,f);  
    fclose(f);  
	sync();
	return 0;
}
