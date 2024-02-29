#include "ip_check.h"

static int get_mac(const char *if_name, char *mac);
static int get_ifidx(const char *if_name, int *idx);

// int get_net_delay_intranet(char *ip)
// {
//     char order[1024] = {0x00},read_data[1024] = {0x00};
// 	char *ptr = NULL,*p = NULL,*start = NULL,*end = NULL;
// 	char time_data[3][32],count = 0,tmp_time_data[32] = {0x00};
// 	float time_ms = 0.00;
// 	int time_us = 0;
// 	memset(time_data,0x00,sizeof(time_data));
// 	//sprintf(order,"ping %s -c 1",ip);
//     sprintf(order,"traceroute -q 1 -n %s",ip);
//     // sprintf(order,"traceroute %s",ip);
// 	//printf("get_net_delay order = %s\n",order);
// 	mb_system_read(order,read_data,1024,IP_CHECK_TMP_FILE_PATH);
// 	printf("get_net_delay read_data = %s\n",read_data);
//     if(NULL == (ptr = strstr(read_data,"ms")))
//     {
//         return -1;
//     }
//     while(ptr != NULL)
//     {
//        end = ptr;
//        ptr =  strstr(ptr + 1,"ms");
//     }
	
// 	{
//         ptr = end - 2;
//         while(ptr > read_data)
//         {
//             if((*ptr < 0x30 || *ptr > 0x39) && *ptr != '.')
//                 break;
//             ptr = ptr - 1;


//             // if(*ptr != 0x20 || *ptr == 0x0d)
//             // {
//             //     ptr = ptr - 1;
//             // }
//             // else
//             // {
//             //     count++;
//             //     if(count == 2)
//             //         break;
//             //     ptr = ptr - 1;
//             // }
//         }
//         memcpy(tmp_time_data,ptr + 1,end - ptr);
// 	}
// 	//printf("get_net_delay tmp_time_data = %s\n",tmp_time_data);
// 	time_ms = atof(tmp_time_data);
// 	//printf("get_net_delay time_ms = %f\n",time_ms);
// 	time_us = (int)(time_ms*1000);
// 	//printf("get_net_delay time_us = %d\n",time_us);
// 	return time_us;

// 	// char order[1024] = {0x00},read_data[1024] = {0x00};
// 	// char *ptr = NULL,*p = NULL;
// 	// char time_data[3][32],count = 0;
// 	// float time_ms = 0.00;
// 	// int time_us = 0;
// 	// memset(time_data,0x00,sizeof(time_data));
// 	// sprintf(order,"ping %s -c 1",ip);
// 	// //printf("get_net_delay order = %s\n",order);
// 	// mb_system_read(order,read_data,1024,IP_CHECK_TMP_FILE_PATH);
// 	// //printf("get_net_delay read_data = %s\n",read_data);
// 	// if(NULL == (ptr = strstr(read_data,"round-trip min/avg/max =")))
// 	// {
// 	// 	return -1;
// 	// }
// 	// else
// 	// {
// 	// 	ptr = ptr + strlen("round-trip min/avg/max =") + 1;
// 	// 	for (p = strtok(ptr, "/");p != NULL;p=strtok(NULL,"/"))
// 	// 	{
// 	// 		//printf("%s\n", p);
// 	// 		strcpy(time_data[count++],p);
// 	// 	}
// 	// }
// 	// //printf("get_net_delay time_data = %s\n",time_data[1]);
// 	// time_ms = atof(time_data[1]);
// 	// //printf("get_net_delay time_ms = %f\n",time_ms);
// 	// time_us = (int)(time_ms*1000);
// 	// //printf("get_net_delay time_us = %d\n",time_us);
// 	// return time_us;
// }

int get_net_delay_external(char *ip)
{
	char order[1024] = {0x00},read_data[1024] = {0x00};
	char *ptr = NULL,*p = NULL;
	char time_data[3][32],count = 0;
	float time_ms = 0.00;
	int time_us = 0;
	memset(time_data,0x00,sizeof(time_data));
	sprintf(order,"ping %s -c 1 -W 1",ip);
	//printf("get_net_delay order = %s\n",order);
	mb_system_read(order,read_data,1024,IP_CHECK_TMP_FILE_PATH);
	//printf("get_net_delay read_data = %s\n",read_data);
	if(NULL == (ptr = strstr(read_data,"round-trip min/avg/max =")))
	{
		return -1;
	}
	else
	{
		ptr = ptr + strlen("round-trip min/avg/max =") + 1;
		for (p = strtok(ptr, "/");p != NULL;p=strtok(NULL,"/"))
		{
			//printf("%s\n", p);
			strcpy(time_data[count++],p);
		}
	}
	//printf("get_net_delay time_data = %s\n",time_data[1]);
	time_ms = atof(time_data[1]);
	//printf("get_net_delay time_ms = %f\n",time_ms);
	time_us = (int)(time_ms*1000);
	//printf("get_net_delay time_us = %d\n",time_us);
	return time_us;
}

int get_mac(const char *if_name, char *mac)
{
    int fd, rtn;
    struct ifreq ifr;

    if (!if_name || !mac)
    {
        return -1;
    }
    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0)
    {
        perror("socket");
        return -1;
    }
    ifr.ifr_addr.sa_family = AF_INET;
    strncpy(ifr.ifr_name, (const char *)if_name, IFNAMSIZ - 1);

    if ((rtn = ioctl(fd, SIOCGIFHWADDR, &ifr)) == 0)
        memcpy(mac, (unsigned char *)ifr.ifr_hwaddr.sa_data, 6);
    close(fd);
    return rtn;
}

int get_ifidx(const char *if_name, int *idx)
{
    int fd, rtn;
    struct ifreq ifr;

    if (!if_name || !idx)
    {
        return -1;
    }
    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0)
    {
        perror("socket");
        return -1;
    }
    ifr.ifr_addr.sa_family = AF_INET;
    strncpy(ifr.ifr_name, (const char *)if_name, IFNAMSIZ - 1);

    if ((rtn = ioctl(fd, SIOCGIFINDEX, &ifr)) == 0)
        *idx = ifr.ifr_ifindex;
    close(fd);
    return rtn;
}

int get_net_delay_intranet(const char *if_name, const char *dst_ip)
{
    uint64 delay_time = 0,tmp_delay_time = 0,tmp_delay_time1 = 0;
    unsigned int ip4bit = 0,i = 0;
    {
        struct in_addr addr = {0};
        if (inet_aton(dst_ip, &addr) == 0)
        {
            perror("inet_aton");
            return -1;
        }
        ip4bit = addr.s_addr;
    }

    char mac[6] = {0};
    if (get_mac(if_name, mac) != 0)
    {
        perror("inet_aton");
        return -1;
    }

    int sock_client = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (sock_client < 0)
    {
        perror("socket");
        return -1;
    }
    int sock_server;
    if ((sock_server = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ARP))) < 0)
    {
        close(sock_client);
        perror("cannot create socket");
        return -1;
    }
    struct timeval tv_out;
    tv_out.tv_sec = 1;
    tv_out.tv_usec = 0;
    setsockopt(sock_server, SOL_SOCKET, SO_RCVTIMEO, &tv_out, sizeof(tv_out));

    unsigned char arp_probe_msg[] = {
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, /*目的mac地址*/
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /*源mac地址*/
        0x08, 0x06,                         /*帧类型*/

        /*ARP报文头部(28个字节)*/
        0x00, 0x01,                         /*硬件类型*/
        0x08, 0x00,                         /*协议类型*/
        6,                                  /*硬件地址长度*/
        4,                                  /*协议地址长度*/
        0x00, 0x01,                         /*ARP请求*/
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /*源mac地址*/
        0, 0, 0, 0,                         /*源IP*/
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, /*目的mac地址*/
        0, 0, 0, 0                          /*目的IP*/
    };

    memcpy(arp_probe_msg + 6, mac, 6);
    memcpy(arp_probe_msg + 22, mac, 6);
    memcpy(arp_probe_msg + 38, &ip4bit, 4);

    int if_idx;
    if (get_ifidx(if_name, &if_idx) != 0)
    {
        close(sock_client);
        close(sock_server);
        return -1;
    }

    // 发送5次
    struct timeval recv_start_time;
    struct timeval recv_end_time;
    gettimeofday(&recv_start_time, NULL);
    //ip_check_out("\n发送\n");
    for (i = 0; i < 5; ++i)
    {
        struct sockaddr_ll sll;
        bzero(&sll, sizeof(sll));
        sll.sll_ifindex = if_idx;

        if (sendto(sock_client, arp_probe_msg, sizeof arp_probe_msg, 0, (struct sockaddr *)&sll, sizeof(sll)) < sizeof arp_probe_msg)
        {
            close(sock_client);
            close(sock_server);
            perror("sendto");
            return -1;
        }
    }

    char buffer[42] = {0};
    int recv_count = 0;
    // 接受最多100条或3秒超时
    
    while (recv_count++ < 100 && recv(sock_server, buffer, sizeof(buffer), 0))
    {
        if ((((buffer[12]) << 8) + buffer[13]) != ETH_P_ARP)
            continue;
        struct timeval now_time;
        gettimeofday(&now_time, NULL);
        if (now_time.tv_sec - recv_start_time.tv_sec > 2)
            break;
        char arp_rsp_mac[18] = {0};
        char arp_rsp_ip[18] = {0};
        sprintf(arp_rsp_mac, "%02x:%02x:%02x:%02x:%02x:%02x", buffer[22], buffer[23], buffer[24], buffer[25], buffer[26], buffer[27]);
        sprintf(arp_rsp_ip, "%d.%d.%d.%d", buffer[28], buffer[29], buffer[30], buffer[31]);
        // printf("%s %s\n", arp_rsp_mac, arp_rsp_ip);
        if (strcmp(arp_rsp_ip, dst_ip) == 0)
        {
			// printf("\n%s %s\n", dst_ip, arp_rsp_ip);
            // printf("lbc %s\n", arp_rsp_mac);
            gettimeofday(&recv_end_time, NULL);
            tmp_delay_time = recv_start_time.tv_sec * 1000*1000;
            tmp_delay_time = tmp_delay_time + recv_start_time.tv_usec;
            tmp_delay_time1 = recv_end_time.tv_sec * 1000*1000;
            tmp_delay_time1 = tmp_delay_time1 + recv_end_time.tv_usec;
            delay_time = tmp_delay_time1 - tmp_delay_time;
            close(sock_client);
            close(sock_server);
            // printf("delay_time = %llu,tmp_delay_time1 = %llu,tmp_delay_time = %llu\n",delay_time,tmp_delay_time1,tmp_delay_time);
            //ip_check_out("\n收到\n");
            return (int)delay_time;
        }
    }
    close(sock_client);
    close(sock_server);
    return 0;  //不存在ip冲突
}

int ip_conflict_check(const char *if_name, const char *dst_ip)
{
    unsigned int ip4bit = 0,i = 0;
    {
        struct in_addr addr = {0};
        if (inet_aton(dst_ip, &addr) == 0)
        {
            perror("inet_aton");
            return -1;
        }
        ip4bit = addr.s_addr;
    }

    char mac[6] = {0};
    if (get_mac(if_name, mac) != 0)
    {
        perror("inet_aton");
        return -1;
    }

    int sock_client = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (sock_client < 0)
    {
        perror("socket");
        return -1;
    }
    int sock_server;
    if ((sock_server = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ARP))) < 0)
    {
        close(sock_client);
        perror("cannot create socket");
        return -1;
    }
    struct timeval tv_out;
    tv_out.tv_sec = 1;
    tv_out.tv_usec = 0;
    setsockopt(sock_server, SOL_SOCKET, SO_RCVTIMEO, &tv_out, sizeof(tv_out));

    unsigned char arp_probe_msg[] = {
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, /*目的mac地址*/
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /*源mac地址*/
        0x08, 0x06,                         /*帧类型*/

        /*ARP报文头部(28个字节)*/
        0x00, 0x01,                         /*硬件类型*/
        0x08, 0x00,                         /*协议类型*/
        6,                                  /*硬件地址长度*/
        4,                                  /*协议地址长度*/
        0x00, 0x01,                         /*ARP请求*/
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /*源mac地址*/
        0, 0, 0, 0,                         /*源IP*/
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, /*目的mac地址*/
        0, 0, 0, 0                          /*目的IP*/
    };

    memcpy(arp_probe_msg + 6, mac, 6);
    memcpy(arp_probe_msg + 22, mac, 6);
    memcpy(arp_probe_msg + 38, &ip4bit, 4);

    int if_idx;
    if (get_ifidx(if_name, &if_idx) != 0)
    {
        close(sock_client);
        close(sock_server);
        return -1;
    }
        

    // 发送5次
    ip_check_out("\n发送\n");
    for (i = 0; i < 5; ++i)
    {
        struct sockaddr_ll sll;
        bzero(&sll, sizeof(sll));
        sll.sll_ifindex = if_idx;

        if (sendto(sock_client, arp_probe_msg, sizeof arp_probe_msg, 0, (struct sockaddr *)&sll, sizeof(sll)) < sizeof arp_probe_msg)
        {
            perror("sendto");
            close(sock_client);
            close(sock_server);
            return -1;
        }
    }

    char buffer[42] = {0};
    int recv_count = 0;
    // 接受最多100条或3秒超时
    struct timeval recv_start_time;
    gettimeofday(&recv_start_time, NULL);
    while (recv_count++ < 100 && recv(sock_server, buffer, sizeof(buffer), 0))
    {
        if ((((buffer[12]) << 8) + buffer[13]) != ETH_P_ARP)
            continue;
        struct timeval now_time;
        gettimeofday(&now_time, NULL);
        if (now_time.tv_sec - recv_start_time.tv_sec > 2)
            break;
        char arp_rsp_mac[18] = {0};
        char arp_rsp_ip[18] = {0};
        sprintf(arp_rsp_mac, "%02x:%02x:%02x:%02x:%02x:%02x", buffer[22], buffer[23], buffer[24], buffer[25], buffer[26], buffer[27]);
        sprintf(arp_rsp_ip, "%d.%d.%d.%d", buffer[28], buffer[29], buffer[30], buffer[31]);
        // printf("%s %s\n", arp_rsp_mac, arp_rsp_ip);
        if (strcmp(arp_rsp_ip, dst_ip) == 0)
        {
			printf("%s %s\n", dst_ip, arp_rsp_ip);
            printf("lbc %s\n", arp_rsp_mac);
            close(sock_client);
            close(sock_server);
            ip_check_out("\n收到\n");
            return 1;  //存在ip冲突
        }
    }
    close(sock_client);
    close(sock_server);
    return 0;  //不存在ip冲突
}
