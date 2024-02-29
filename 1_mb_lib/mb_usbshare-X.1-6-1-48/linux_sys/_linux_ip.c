#define _linux_c
#include "_linux_ip.h"
int get_net(char *eth,struct _net_define  *net)
{
	uint32 ip, mask, gateway, dns;
	//int type;
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
	char dns_tmp[255] = {0};
	
	dns = get_dns_file_servername(dns_tmp);
	if (dns < 0)
	{
		out("获取DNS失败\n");
	}
	//printf("%s dns_tmp\n",dns_tmp);
	net->DNS = dns;
	//type = get_type(eth);
	//net->type = type;
	return 0;  
    
}

int jude_dev_support_wifi(void)
{
	char buf[4096] = {0};
	system("ifconfig wlan0 up");
	mb_system_read("ifconfig", buf, sizeof(buf), "/tmp/ifconfig_wlan.txt");
	if (strstr(buf, "wlan0") == NULL)
	{
		return -1;
	}
	return 1;
}

int find_now_wifi_ssid(char *ssid)
{
	char now_wifi_buf[2048] = { 0 };
	mb_system_read("iwconfig wlan0", now_wifi_buf, sizeof(now_wifi_buf), "/tmp/ifconfig_wlan0.txt");
	char *p_s, *p_e;
	p_s = strstr(now_wifi_buf, "ESSID:\"");
	if (p_s == NULL)
	{
		out("当前未连接至wifi\n");
		return -1;
	}
	p_s += strlen("ESSID:\"");
	p_e = strstr(p_s, "\"  ");
	if (p_e == NULL)
	{
		out("当前连接的wifi SSID规则有误\n");
		return - 2;
	}
	memcpy(ssid, p_s, p_e - p_s);
	//out("当前连接的wifi SSID:%s\n", ssid);

	return 0;		
}

//static int get_configure_file(char *path, char *in_data, char *out_data)
//{
//	//int configLen;
//	int len;
//	char tmp[256] = { 0 };
//	char* linePos = NULL;
//	char lineBuff[256] = { 0 };// 存储读取一行的数据
//
//	memset(lineBuff, 0, 256);
//	FILE* fp = fopen(path, "r");
//	if (fp == NULL)
//	{
//		out("文件打开失败\n");
//		return -1;
//	}
//	fseek(fp, 0, SEEK_END);
//	ftell(fp); //配置文件长度  
//	fseek(fp, 0, SEEK_SET);
//	while (fgets(lineBuff, 256, fp) != NULL)
//	{
//		if ((lineBuff[0] == '#') || (lineBuff[0] == ';'))
//			continue;
//		linePos = NULL;
//		linePos = strstr(lineBuff, in_data);
//		if (linePos == NULL)
//		{
//			memset(lineBuff, 0, sizeof(lineBuff));
//			continue;
//		}
//		linePos += strlen(in_data);
//		len = strlen(lineBuff) - strlen(in_data);
//		memset(tmp, 0, sizeof(tmp));
//		strncpy(tmp, linePos, len);
//		char *p = strtok(tmp, "\r\n");
//		if (p == NULL)
//		{
//			fclose(fp);
//			return -1;
//		}
//		sprintf(out_data, "%s", p);
//		fclose(fp);
//		return 0;
//	}
//	out("在配置文件中没找到\" %s \"数据\n", in_data);
//	fclose(fp);
//	return -1;
//}

int set_net(char* eth_wlan, struct _net_define	*net)
{
	int ip_type;
	int result;
	char buf[1024];
	char set_buf[1024] = { 0 };
	char *start = NULL, *dw = NULL;
	char hd[100] = { 0 };
	
	out("set_net\n");
	

	if (strcmp(eth_wlan,"wlan0") == 0)
	{
		out("//////wifi随机配置模式\n");
		char reslov_path[200];
		char wifi_config_path[200];
		int wifi_find=0;
		char wifi_ssid[50];
		out("支持无线网络且需设置无线网络\n");
		char wifi_list[2048] = { 0 };
		mb_system_read("iwlist wlan0 scan | grep ESSID", wifi_list, sizeof(wifi_list), "/tmp/iwlist_wlan0_sacn.txt");
		char *p_index, *p_start, *p_end;
		p_index = strstr(wifi_list,"ESSID:\"");
		if (p_index == NULL)
		{
			out("wifi搜索可用列表失败\n");
			return -1;
		}
		for (;;)
		{
			p_start = strstr(p_index, "ESSID:\"");
			if (p_start == NULL)
				break;
			p_start += strlen("ESSID:\"");
			p_end = strstr(p_start, "\"");
			if (p_end == NULL)
				break;
			p_index = p_end;
			memset(wifi_ssid, 0, sizeof(wifi_ssid));
			memcpy(wifi_ssid, p_start, p_end-p_start);

			memset(wifi_config_path, 0, sizeof(wifi_config_path));
			sprintf(wifi_config_path, "/etc/wifi_config/wifi_%s.sh", wifi_ssid);

			if (file_exists(wifi_config_path) == 0)
			{
				out("找到可用wifi ssid：%s\n", wifi_ssid);
				wifi_find = 1;
				break;
			}
			continue;
		}
		if (wifi_find != 1)
		{
			out("未找到可用WIFI列表\n");
			return -1;
		}
		memset(reslov_path, 0, sizeof(reslov_path));
		sprintf(reslov_path, "/etc/wifi_config/resolv.conf_%s", wifi_ssid);

		if (file_exists(reslov_path) == 0)
		{
			out("找到可用wifi DNS 配置文件\n");
			copy_file_cmp(reslov_path, DF_LINUX_DNS);
		}
		system("ifconfig wlan0 up");
		system(wifi_config_path);
		return 0;

	}
	else if ((strcmp(eth_wlan, "wlan0") != 0) && (strcmp(eth_wlan, "eth0")!=0))
	{
		out("//////wifi 指定ssid配置模式\n");
		char reslov_path[200] = {0};
		char wifi_config_path[200] = {0};
		sprintf(wifi_config_path, "/etc/wifi_config/wifi_%s.sh", eth_wlan);
		if (file_exists(wifi_config_path) != 0)
		{
			out("找到可用wifi ssid：%s\n", eth_wlan);
			return -1;
		}
		memset(reslov_path, 0, sizeof(reslov_path));
		sprintf(reslov_path, "/etc/wifi_config/resolv.conf_%s", eth_wlan);

		if (file_exists(reslov_path) == 0)
		{
			out("找到可用wifi DNS 配置文件\n");
			copy_file_cmp(reslov_path, DF_LINUX_DNS);
		}
		system("ifconfig wlan0 up");
		system(wifi_config_path);
		return 0;
	}
	else
	{
		out("//////eth0 配置模式\n");
		out("ip = [%08x] mask = [%08x] gate = [%08x]\n", net->ip, net->mask, net->gateway[0]);
		ip_type = net->type;
		memset(buf, 0, sizeof(buf));
		if (read_file(DF_LINUX_NETWORK_FILE, buf, sizeof(buf)) < 0)
		{
			out("读取文件:[%s]失败\n", DF_LINUX_NETWORK_FILE);
			if (file_exists(DF_LINUX_NETWORK_FILE) != 0)
			{
				copy_file(DF_LINUX_NETWORK_FILE_BACK, DF_LINUX_NETWORK_FILE);
				if (read_file(DF_LINUX_NETWORK_FILE, buf, sizeof(buf)) < 0)
				{
					return -1;
				}
			}
			else
			{
				return -1;
			}
		}
		out("buf=[%s]\n", buf);
		dw = strstr(buf, "down");
		if (dw == NULL || dw > buf + strlen(buf))
		{
			start = strstr(buf, "ifconfig eth0");
			out("start=[%s]\n", start);
			memcpy(hd, buf, strlen(buf) - strlen(start));
			out("hd=[%s]\n", hd);
			sprintf(set_buf, "%sifconfig %s down\n%s", hd, "eth0", start);//add by whl
			out("whlll setbuf=[%s]\n", set_buf);
			result = write_file(DF_LINUX_NETWORK_FILE, set_buf, strlen(set_buf));
			chmod_file(DF_LINUX_NETWORK_FILE, "777");
			copy_file(DF_LINUX_NETWORK_FILE, DF_LINUX_NETWORK_FILE_BACK);
			sync();
		}
		//write_correct_mac2file();
		//chmod_file(DF_LINUX_NETWORK_FILE, "777");



		if (ip_type == 2)
		{
			out("//////eth0 配置模式之固定IP\n");
			system("killall udhcpc");
			out("设置固定IP\n");
			chmod_file(DF_LINUX_NETWORK_FILE, "777");
			system("/etc/init.d/auto_config_network");
			result = set_net_file("eth0", net);//48:D3:22:14:96:6D 
			if (result < 0)
			{
				//set_mask(net->mask, eth);
				out("无需在文件中重新设置网络参数\n");
				//return 0;
			}
			//out("设置固定IP操作\n");
			out("设置当前网络参数\n");
			out("ip = [%08x] mask = [%08x] gate = [%08x]\n", net->ip, net->mask, net->gateway[0]);
			chmod_file(DF_LINUX_NETWORK_FILE, "777");
			system("/etc/init.d/auto_config_network");
			//write_log("/etc/init.d/auto_config_network\n");
			//return 0;
			result = set_net_1("eth0", net);
			copy_file_cmp(DF_LINUX_NETWORK_FILE, DF_LINUX_NETWORK_FILE_BACK);
			return result;
		}
		else if (ip_type == 1)
		{
			out("//////eth0 配置模式之DHCP IP\n");
			int mac_use = 0;
			char if_data[1024] = { 0 };
			char mac_address[20] = { 0 };
			char cmd[1024] = { 0 };
			//判断MAC地址是否存在
			if (access(DF_TER_MAC_ADDR_FILE, 0) == 0)
			{
				result = read_file(DF_TER_MAC_ADDR_FILE, mac_address, sizeof(mac_address));
				if ((result < 0) || (strlen(mac_address) != 17) || \
					(mac_address[0] != 0x34) || (mac_address[1] != 0x38) || \
					(mac_address[2] != 0x3A) || (mac_address[5] != 0x3A) || \
					(mac_address[8] != 0x3A) || (mac_address[11] != 0x3A) ||
					(mac_address[14] != 0x3A))
				{
					logout(INFO, "system", "ip", "原MAC地址不可用\r\n");
					mac_use = 0;
				}
				else
				{
					//out("原MAC地址可用\n");
					mac_use = 1;
				}
			}

			if (mac_use == 0)
			{
				out("//////eth0 配置模式之DHCP IP ,配置地址文件不存在或mac地址异常\n");
				logout(INFO, "system", "ip", "初次使用,mac地址配置文件不存在或是新机器\r\n");

				char ter_id[13] = { 0 };
				read_file(DF_TER_ID_INI, ter_id, sizeof(ter_id));

				//logout(INFO, "system", "ip", "ter_id :%s\r\n", ter_id);
				if (memcmp(ter_id, "068", 3) != 0)//第一次，没有机器编号的时候
				{
					mb_system_read("ifconfig eth0 up", if_data, sizeof(if_data), "/tmp/ifconfig_eth0_UP.txt");
					logout(INFO, "system", "ip", "设置eth0开启结果:\n %s \r\n", if_data);

					sprintf(cmd, "ifconfig eth0 hw ether \"48`uuidgen |awk -F- \'{print $5}\'|sed \'s/../&:/g\'|sed \'s/\\(.\\)$//\' |cut -b3-17`\"");
					logout(INFO, "system", "ip", "%s\r\n", cmd);
					//system(cmd);
					mb_system_read(cmd, if_data, sizeof(if_data), "/tmp/ifconfig_hw_ether.txt");
					logout(INFO, "system", "ip", "设置网络mac地址结果:\n %s \r\n", if_data);

					mb_system_read("ifconfig eth0", if_data, sizeof(if_data), "/tmp/ifconfig_eth0_test2.txt");
					logout(INFO, "system", "ip", "网络查看执行结果:\n %s \r\n", if_data);
					get_net_mac("eth0", mac_address);
					write_file(DF_TER_MAC_ADDR_FILE, mac_address, strlen(mac_address));
					write_mac2network(mac_address);
				}
				else//根据机器编号生成一个MAC
				{
					memcpy(mac_address, "48:A", 4);//
				//1->E  3->B 0->D
					int i,j;
					char tmp;
					for (i = 3, j = 4; i <= 11; i++)
					{
						if (ter_id[i] == '1')
						{
							tmp = 'E';
						}
						else if (ter_id[i] == '3')
						{
							tmp = 'B';
						}
						else if (ter_id[i] == '0')
						{
							tmp = 'D';
						}
						else
						{
							tmp = ter_id[i];
						}
						if (j == 5 || j == 8 || j == 11 || j == 14)
						{
							mac_address[j] = ':';
							j++;
						}
						mac_address[j] = tmp;
						j++;
					}
					sprintf(cmd, "ifconfig eth0 hw ether \"%s\"",mac_address);
					system(cmd);
					write_file(DF_TER_MAC_ADDR_FILE, mac_address, strlen(mac_address));
					write_mac2network(mac_address);
				}
				sync();
			}
			else
			{	
				out("//////eth0 配置模式之DHCP IP ,配置地址文件正常\n");
				char ter_id[13] = { 0 };

				read_file(DF_TER_ID_INI, ter_id, sizeof(ter_id));

				//没有机器编号的时候,但之前根据机器编号生成过--(工厂mac重复问题)
				if (memcmp(ter_id, "068", 3) != 0 && (memcmp(mac_address,"48:A", 4) == 0 || memcmp(mac_address, "48:a", 4) == 0))
				{
					out("//////eth0 配置模式之DHCP IP ,配置地址文件正常，且当前无机器编号\n");
					write_log("当前无机器编号，之前注册根据机器编号生成过mac，MAC重新随机生成!\n");

					mb_system_read("ifconfig eth0 up", if_data, sizeof(if_data), "/tmp/ifconfig_eth0_up_3.txt");
					write_log("设置eth0开启结果:\n %s \n", if_data);

					sprintf(cmd, "ifconfig eth0 hw ether \"48`uuidgen |awk -F- \'{print $5}\'|sed \'s/../&:/g\'|sed \'s/\\(.\\)$//\' |cut -b3-17`\"");
					write_log("%s\n", cmd);
				//system(cmd);
					mb_system_read(cmd, if_data, sizeof(if_data), "/tmp/ifconfig_eth0_hw_ether2.txt");
					write_log("设置网络mac地址结果:\n %s \n", if_data);

					mb_system_read("ifconfig eth0", if_data, sizeof(if_data), "/tmp/ifconfig_eth0test4.txt");
					write_log("网络查看执行结果:\n %s \n", if_data);
					get_net_mac("eth0", mac_address);
					write_file(DF_TER_MAC_ADDR_FILE, mac_address, strlen(mac_address));
					write_mac2network(mac_address);
				}
				else
				{
					//logout(INFO, "system", "ip", "使用历史mac地址配置\r\n");
					//mb_system_read("ifconfig eth0 down", if_data, sizeof(if_data));
					//logout(INFO, "system", "ip", "设置eth0关闭结果:\n %s \r\n", if_data);

					mb_system_read("ifconfig eth0 up", if_data, sizeof(if_data), "/tmp/ifconfig_eth0_up_test3.txt");
					//logout(INFO, "system", "ip", "设置eth0开启结果:\n %s \r\n", if_data);


					sprintf(cmd, "ifconfig eth0 hw ether \"%s\"", mac_address);
					system(cmd);
					mb_system_read(cmd, if_data, sizeof(if_data), "/tmp/ifconfig_eth0_hw_ethertest5.txt");
					//logout(INFO, "system", "ip", "设置网络mac地址结果:\n %s \r\n", if_data);

					mb_system_read("ifconfig eth0", if_data, sizeof(if_data), "/tmp/ifconfig_eth0_test7.txt");
					//write_log("网络查看执行结果:\n %s \n", if_data);
					write_mac2network(mac_address);
				}
			}
			system("ifconfig lo 127.0.0.1");
			result = detect_process("udhcpc", "dhcpc.txt");
			if (result < 0)
			{
				system("udhcpc -i eth0 &");
				int i;
				for (i = 0; i < 20; i++)
				{
					uint32 ip;
					ip = get_ip("eth0");
					//char ipasc[100] = {0};
					//ip2ip_asc(ip, ipasc);
					//printf("ip hex:%08x\n", ip);
					//printf("ip asc:%s\n", ipasc);
					if (ip != 0)
						break;
					sleep(1);
				}


			}
			copy_file_cmp(DF_LINUX_NETWORK_FILE, DF_LINUX_NETWORK_FILE_BACK);
			return 0;
		}
		
	}
	logout(INFO, "system", "ip", "要设置IP类型错误\r\n");
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
	int i = 0;
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

 int write_mac2network(char *w_mac)
 {
	 char *mace = NULL, *macs = NULL;
	 char buf[1024];
	 char hd[100] = { 0 };
	 char set_buf[1024] = { 0 };
	 memset(buf, 0, sizeof(buf));
	 if (read_file(DF_LINUX_NETWORK_FILE, buf, sizeof(buf))<0)
	 {
		 out("读取文件:[%s]失败\n", DF_LINUX_NETWORK_FILE);
		 return -1;
	 }
	 out("write_correct_mac2file buf=[%s]\n", buf);
	 macs = strstr(buf, "MAC_ADDR=\"");
	 macs += strlen("MAC_ADDR=\"");
	 mace = strstr(macs, "\"");
	 mace += 1;
	 memcpy(hd, buf, macs - buf - strlen("MAC_ADDR=\""));
	 sprintf(set_buf, "%sMAC_ADDR=\"%s\"%s", hd, w_mac, mace);
	 out("mac setbuf=[%s]\n", set_buf);
	 write_file(DF_LINUX_NETWORK_FILE, set_buf, strlen(set_buf));
	 system("chmod 777 /etc/init.d/auto_config_network");
	 sync();


	 return 0;
 }

//static int write_correct_mac2file()
//{
//	char *mace = NULL, *macs = NULL;
//	char buf[1024];
//	char hd[100] = { 0 };
//	char set_buf[1024] = { 0 };
//	char c_mac[16] = { 0 };
//	memset(buf, 0, sizeof(buf));
//	if (read_file(DF_LINUX_NETWORK_FILE, buf, sizeof(buf))<0)
//	{
//		out("读取文件:[%s]失败\n", DF_LINUX_NETWORK_FILE);
//		return -1;
//	}
//	out("write_correct_mac2file buf=[%s]\n", buf);
//	macs = strstr(buf, "MAC_ADDR=\"");
//	macs += strlen("MAC_ADDR=\"");
//	mace = strstr(macs, "\"");
//	if (mace - macs >= 15)
//	{
//		memcpy(c_mac, macs, 15);
//		mace += 1;
//		memcpy(hd, buf, macs - buf - strlen("MAC_ADDR=\""));
//		sprintf(set_buf, "%sMAC_ADDR=\"%s\"%s", hd, c_mac, mace);
//		out("mac setbuf=[%s]\n", set_buf);
//		write_file(DF_LINUX_NETWORK_FILE, set_buf, strlen(set_buf));
//		system("chmod 777 /etc/init.d/auto_config_network");
//		sync();
//	}
//	else
//	{
//		char cmd[100] = { 0 };
//		char if_data[1000] = { 0 };
//		sprintf(cmd, "ifconfig eth0 hw ether \"48`uuidgen |awk -F- \'{print $5}\'|sed \'s/../&:/g\'|sed \'s/\\(.\\)$//\' |cut -b3-17`\"");
//		
//		//system(cmd);
//		mb_system_read(cmd, if_data, sizeof(if_data));
//
//		mb_system_read("ifconfig eth0", if_data, sizeof(if_data));
//
//
//		get_net_mac("eth0", c_mac);
//		mace += 1;
//		memcpy(hd, buf, macs - buf - strlen("MAC_ADDR=\""));
//		sprintf(set_buf, "%sMAC_ADDR=\"%s\"%s", hd, c_mac, mace);
//		out("mac setbuf=[%s]\n", set_buf);
//		write_file(DF_LINUX_NETWORK_FILE, set_buf, strlen(set_buf));
//		system("chmod 777 /etc/init.d/auto_config_network");
//		sync();
//
//	}
//	
//	return 0;
//}




int set_net_1(char *eth,struct _net_define  *net)
{
	struct _net_define my_net_define;
	char ip_asc[100];
	char mask_asc[100];
	char gatewaybuf[100];
	char tmpbuf[100];
	char tmp[200];
	int gateway,i;
	memset(&my_net_define,0,sizeof(my_net_define));
	memset(ip_asc,0,sizeof(ip_asc));
	memset(mask_asc,0,sizeof(mask_asc));
	if((net->ip!=0)&&(net->mask!=0))
	{
		ip2ip_asc(net->ip,ip_asc);
		out("设置当前网络的IP=[%s]\n",ip_asc);
		ip2ip_asc(net->mask,mask_asc);
		out("设置当前网络的mask_asc=[%s]\n",mask_asc);
		sprintf(tmp,"ifconfig eth0 %s netmask %s",ip_asc,mask_asc);
		out("%s\n\n\n",tmp);
		system(tmp);
	}
	if(net->gateway[0]!=0)
	{
		gateway = get_gateway(eth, &my_net_define.gateway[0],(sizeof(my_net_define.gateway)/sizeof(uint32)));
		if(gateway <=0)
		{
			out("获取网关失败\n");
			memset(tmpbuf,0,sizeof(tmpbuf));
			ip2ip_asc(net->gateway[0], gatewaybuf);			
			out("设置当前网络的网关%s\n", gatewaybuf);
			sprintf(tmpbuf, "route add default gw %s", gatewaybuf);
			out("%s\n\n\n",tmpbuf);
			system(tmpbuf);
			return 0;
		}
		out("获取到%d条网关信息\n",gateway);
		for(i=0;i<gateway;i++)
		{
			out("gateway=%08x\n",my_net_define.gateway[i]);
			memset(tmpbuf,0,sizeof(tmpbuf));
			memset(gatewaybuf,0,sizeof(gatewaybuf));
			ip2ip_asc(my_net_define.gateway[i], gatewaybuf);	
			sprintf(tmpbuf, "route del default gw %s", gatewaybuf);
			out("%s\n\n\n",tmpbuf);
			system(tmpbuf);
		}
		memset(tmpbuf,0,sizeof(tmpbuf));
		memset(gatewaybuf,0,sizeof(gatewaybuf));
		ip2ip_asc(net->gateway[0], gatewaybuf);			
		out("设置当前网络的网关%s\n", gatewaybuf);
		sprintf(tmpbuf, "route add default gw %s", gatewaybuf);
		out("%s\n\n\n",tmpbuf);
		system(tmpbuf);
	}
   	return 0;
}


int get_DNS_ip(char *hostname,char *hostip)
{
	struct hostent *h;  
    struct in_addr in;  
    struct sockaddr_in addr_in; 
	struct sockaddr_in adr_inet; 
	int len;
	int ip_len;
	char hostnamebuf[100] = { 0 };
	len = strlen((char *)hostname);
	memcpy(hostnamebuf, hostname, len);
	
	memset(&adr_inet, 0, sizeof(adr_inet));
	if (inet_aton(hostnamebuf, &adr_inet.sin_addr))//判断是否是IP
	{
		out("is ip\n");
		out("hostnamebuf222 %s\n", hostnamebuf);
		memcpy(hostip, hostnamebuf,len);
		return len;
	}
	if(len<5)
	{
		out("域名长度错误\n");
		return -1;
	}
	//out("hostnamebuf %s\n", hostnamebuf);
	if (memcmp(hostnamebuf, "127.0.0.1", len) == 0)
	{
		out("本机IP不用解析\n");
		sprintf((char *)hostip, "127.0.0.1");
		return strlen((const char*)hostip);
	}
	//signal(SIGALRM, alarm_func);
	//if (sigsetjmp(jmpbuf, 1) != 0)
	//{
	//	alarm(0); /* 取消闹钟 */
	//	signal(SIGALRM, SIG_IGN);
	//	out("解析IP地址失败sigsetjmp\n");
	//	return -1;
	//}
	//alarm(2); /* 设置超时时间 */

	h = gethostbyname((const char *)hostnamebuf);
	//signal(SIGALRM, SIG_IGN);
	if(h==NULL)
	{
		out("解析IP地址失败\n");
		return -1;
	}	
	else
	{	
		memcpy(&addr_in.sin_addr.s_addr,h->h_addr,4);
		in.s_addr=addr_in.sin_addr.s_addr;
		//out("host name	:%s\n",h->h_name);
		//out("ip length	:%d\n",h->h_length);
		//out("type	:%d\n",h->h_addrtype);
		//out("ip	:%s\n",inet_ntoa(in));
	}
	ip_len = strlen(inet_ntoa(in));
	memcpy(hostip,inet_ntoa(in),ip_len);
	
	return ip_len;
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


//int set_mask(uint32 ip,char *eth)
//{   int fd;
//    int req;
//    struct ifreq ifr;
//    struct sockaddr_in *addr;
//    char ip_asc[100];
//    out("设置子网掩码\n");
//    memset(&ifr,0,sizeof(ifr));
//    fd=socket(AF_INET,SOCK_DGRAM,0);
//    if(fd<0)
//    {   out("设置子网掩码失败,socket打开失败\n");
//        return -1;
//    }
//    memcpy(ifr.ifr_name,eth,strlen(eth));
//    addr=(struct sockaddr_in *)&(ifr.ifr_addr);
//    memset(ip_asc,0,sizeof(ip_asc));
//    ip2ip_asc(ip,ip_asc);
//    out("设置的子网掩码=[%s]\n",ip_asc);
//    addr->sin_family = AF_INET;
//    inet_aton(ip_asc,&(addr->sin_addr));
//    out("addr->sin_addr = %08x\n",addr->sin_addr);
//    req =ioctl(fd, SIOCSIFNETMASK,&ifr);
//    if(req<0)
//    {   close(fd);
//        out("设置子网掩码失败req = %d\n",req);
//        return -1;
//    }
//    close(fd);
//    out("子网掩码成功\n");
//    return 0; 
//}


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
	if (read_file(file, buf, sizeof(buf))<0)
    {   //out("获取路由文件失败:[%s]\n",file);
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
        //out("找到有效的路由信息\n");
        ip=((way&0xff)<<24)+(((way>>8)&0xff)<<16)+(((way>>16)&0xff)<<8)+(((way>>24)&0xff)<<0);
        getway[i++]=ip;
        if(i>=size_get)
            break;
    }
    //out("找到可用路由信息:i=%d\n",i);
    return i;  
}





//int juge_gateway(char *eth,uint32 gateway)
//{   char file[DF_TY_PATH_MAX];
//    //int fd;
//    char buf[1024];
//    char *p;
//    uint32 dest,way,flag,ip;
//    char net_name[100];
//    out("获取路由列表\n");
//    memset(file,0,sizeof(file));
//    sprintf(file,"%s/%s",DF_LINUX_NET_DIR,"route");
//    memset(buf,0,sizeof(buf));
//    if(read_file(file,buf,sizeof(buf))<0)
//    {   out("获取路由文件失败:[%s]\n",file);
//        return -1;
//    }
//    for(p=buf;;)
//    {   memset(net_name,0,sizeof(net_name));
//        sscanf(p,"%s\t%lx\t%lx\t%lx",net_name,&dest,&way,&flag);   
//        out("name=[%s],dest=[0X%08X],way=[0X%08X],flag=[0X%08X]\n",net_name,dest,way,flag);
//        p=strstr(p,"\n");
//        if(p==NULL)
//            break;
//        p++;
//        if(p==NULL)
//            break;
//        if(memcmp(net_name,eth,strlen(eth))!=0)
//            continue;
//        if(dest!=0)
//            continue;
//        if(flag!=3)
//            continue;
//        out("找到有效的路由信息\n");
//        ip=((way&0xff)<<24)+(((way>>8)&0xff)<<16)+(((way>>16)&0xff)<<8)+(((way>>24)&0xff)<<0);
//        out("gateway=0x%08x,ip=0x%08x\n",gateway,ip);
//        if(ip==gateway)
//        {   out("找到相同的路由信息\n");
//            return 0;
//        }     
//    }
//    out("没有找到路由信息\n");
//    return -2;
//}


//int set_ip(uint32 ip,char *eth)
//{   int fd;
//    struct ifreq ifr;
//    struct sockaddr_in *addr;
//    char ip_asc[100];
//    out("设置ip\n");
//    memset(&ifr,0,sizeof(ifr));
//    fd=socket(AF_INET,SOCK_DGRAM,0);
//    if(fd<0)
//    {   out("获取IP,socket打开失败\n");
//        return -1;
//    }
//    memcpy(ifr.ifr_name,eth,strlen(eth));
//    addr=(struct sockaddr_in *)&(ifr.ifr_addr);
//    memset(ip_asc,0,sizeof(ip_asc));    
//    ip2ip_asc(ip,ip_asc);
//    out("设置的ip=[%s]\n",ip_asc);
//    addr->sin_family = AF_INET;
//    inet_aton(ip_asc,&(addr->sin_addr));
//    if(ioctl(fd, SIOCSIFADDR,&ifr) == -1)
//    {   close(fd);
//        out("设置ip失败\n");
//        return -1;
//    }
//    #if 0
//    ifr.ifr_flags |= IFF_UP;
//    if( ioctl( fd, SIOCSIFFLAGS, &ifr) == -1)
//    {   close(fd);
//        out("网卡启动失败\n");
//        return -1;
//    }
//    #endif
//    close(fd);
//    out("ip设置成功\n");
//    return 0;
//}

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
        //out("获取eth=[%s]的IP失败\n",eth);
        return 0;
    }
    addr=(struct sockaddr_in *)&(ifr.ifr_addr);
    memset(ip_asc,0,sizeof(ip_asc));
    inet_ntop(AF_INET,&addr->sin_addr,ip_asc,sizeof(ip_asc));
    //out("IP:[%s]\n",ip_asc);
    close(fd);
    return ip_asc2ip(ip_asc,strlen(ip_asc));    
    
}

int get_socket_net_state(char *eth)
{
	struct ethtool_value edata;
    int fd,err = 0;
    struct ifreq ifr;
	memset(&ifr, 0, sizeof(ifr));
	memcpy(ifr.ifr_name,eth,strlen(eth));
	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd < 0) 
	{
		perror("Cannot get control socket");
		return -1;
	}
	edata.cmd = 0x0000000a;
	ifr.ifr_data = (caddr_t)&edata;
	err = ioctl(fd, 0x8946, &ifr);
	if (err == 0) 
	{
// 		out("edata.data = %02x\n",edata.data);
// 		out(stdout, "Link detected: %s\n",edata.data ? "yes":"no");
		close(fd);
		return edata.data;		
	} 
	else if (errno != EOPNOTSUPP) 
	{
        out("Cannot get link status\n");
		close(fd);
		return -1;
    }
	close(fd);
   return -1;
}

static int pack_dns(struct _net_define  *net,char *o_buf)
{
	pack_eth_ip(net->DNS, "nameserver", o_buf);	
	sprintf(o_buf+strlen(o_buf),"\n");
	sprintf(o_buf+strlen(o_buf),"nameserver 114.114.114.114\n");	

	sprintf(o_buf + strlen(o_buf), "nameserver 223.6.6.6\n");
	//pack_eth_ip(net->gateway[0], "nameserver", o_buf);
	sprintf(o_buf+strlen(o_buf),"\n\n\n");
	return strlen(o_buf); 
}

//static int pack_eth(char *eth,struct _net_define  *net,char *o_buf)
//{	int netmask;
//	char buf[2];
//	sprintf(o_buf,"#!/bin/sh\n\n");
//	sprintf(o_buf+strlen(o_buf),"MAC_ADDR=\"`uuidgen |awk -F- \'{print $5}\'|sed \'s/../&:/g\'|sed \'s/\(.\)$//\' |cut -b3-17`\"\n\n");
//	sprintf(o_buf+strlen(o_buf),"ifconfig %s hw ether \"48$MAC_ADDR\"\n",eth);
//	sprintf(o_buf+strlen(o_buf),"ifconfig lo 127.0.0.1\n");	
//	pack_eth_ip(net->ip,"ifconfig eth0",o_buf);
//	sprintf(o_buf+strlen(o_buf),"/");
//	netmask = ntod(net->mask);
//	out("pack_eth netmask = %d\n",netmask);
//	sprintf(o_buf+strlen(o_buf),"%d\n",netmask);
//	pack_eth_ip(net->gateway[0] ,"route add default gw",o_buf);
//	sprintf(o_buf+strlen(o_buf),"\n\n\n");
//	return strlen(o_buf); 	
//}

//void itoa (int n,char s[])
// {
//	int i,j,sign;
//	if((sign=n)<0)//记录符号
//	n=-n;//使n成为正数
//	i=0;
//	do
//	{
//       s[i++]=n%10+'0';//取下一个数字
//	}
//	while ((n/=10)>0);//删除该数字
//	if(sign<0)
//	s[i++]='-';
//	s[i]='\0';
//	for(j=i;j>=0;j--)//生成的数字是逆序的，所以要逆序输出
//       printf("%c",s[j]);
// } 

static int pack_eth_ip(uint32 ip,char *head,char *o_buf)
{ 	char ip_asc[100];
	memset(ip_asc,0,sizeof(ip_asc));
	ip2ip_asc(ip,ip_asc);
	sprintf(o_buf+strlen(o_buf),"%s %s",head,ip_asc);	
	return strlen(o_buf);
}


int get_a20_net_file(char *eth,struct _net_define  *net,uint8 *mac)
{	char buf[2048];
	char *p_file_head,*p_file_end;
	memset(buf,0,sizeof(buf));
	if (read_file(DF_LINUX_NETWORK_FILE, buf, sizeof(buf))<0)
	{	out("读取文件:[%s]失败\n",DF_LINUX_NETWORK_FILE);
		return -1;
	}	
	out("buf=[%s]\n",buf);
	if(get_file_head_end(buf,"#!/bin/sh",&p_file_head,&p_file_end)<0)
	{	out("ip文件内容错误\n",eth);
	   	return -2;
	}
	if(get_a20_fe("MAC_ADDR=\"",p_file_head,p_file_end,net,mac)<0)
	{	out("获取IP地址失败\n");
		return -3;
	}
	out("MAC地址获取成功\n");
	p_file_head = NULL;
	p_file_end = NULL;
	memset(buf,0,sizeof(buf));
	if (read_file(DF_LINUX_DNS, buf, sizeof(buf))<0)
	{	out("读取文件:[%s]失败\n",DF_LINUX_NETWORK_FILE);
		return -1;
	}
	if(get_file_head_end(buf,"nameserver",&p_file_head,&p_file_end)<0)
	{	out("dns文件内容错误\n",eth);
	   	return -4;
	}
	if(get_dns("nameserver ",p_file_head,p_file_end,net)<0)
	{	out("获取IP地址失败\n");
		return -3;
	}
	out("获取所有网络信息成功\\n\n\n\n");
	return 0;	
}

static int get_a20_fe(char *macname,char *str,char *end,struct _net_define  *net,uint8 *macaddr)
{	char *ps,*end_ps;	
	char buf[30];
	char mask[2];
	int i;
	int netmask;
	ps=strstr(str,macname);
	if((ps==NULL)||(ps>end))
	{	out("without:[%s]\n",macname);
		return -1;
	}
	out("find %s\n",macname);
	ps+=strlen(macname);
	str = ps;
	end_ps=strstr(str,"\"");
	if((end_ps==NULL)||(end_ps>end))
	{	out("with out:\"\n");
		return -2;
	}
	i=end_ps-ps;
	out("i = %d\n",i);
	if(i>100)
	{
		out("不符合mac地址规范\n");
		return -3;
	}
	memset(macaddr,0,sizeof(macaddr));
	memcpy(macaddr,ps,i);
	out("mac=%s\n",macaddr);
	out("找到MAC地址");
	ps = end_ps+2;//指向\n\n之后

	ps=strstr(str,"ifconfig eth0 ");//指向第一个ifconfig eth0 开头
	if((ps==NULL)||(ps>end))
	{	out("without:[%s]\n","ifconfig eth0 ");
		return -1;
	}
	out("find the first ifconfig eth0\n");
	ps+=strlen("ifconfig eth0 ");//指向第一个ifconfig eth0 结尾
	str = ps;
	ps=strstr(str,"ifconfig eth0 ");//指向第二个ifconfig eth0 开头
	if((ps==NULL)||(ps>end))
	{	out("without:[%s]\n","ifconfig eth0 ");
		return -1;
	}
	out("find the second ifconfig eth0\n");
	ps+=strlen("ifconfig eth0 ");//指向第二个ifconfig eth0 结尾
	str = ps;

	//add by whl
	ps = strstr(str, "ifconfig eth0 ");//指向第三个ifconfig eth0 开头
	if ((ps == NULL) || (ps>end))
	{
		out("without3:[%s]\n", "ifconfig eth0 ");
		return -1;
	}
	out("find the third ifconfig eth0\n");
	ps += strlen("ifconfig eth0 ");//指向第三个ifconfig eth0 结尾
	str = ps;
	//end


	end_ps=strstr(ps,"/");
	if((end_ps==NULL)||(end_ps>end))
	{	out("with out:/\n");
		return -2;
	}
	i =  end_ps-ps;
	if(i>sizeof(buf))
	{	out("不符合ip地址设置规范\n");
		return -3;
	}
	memset(buf,0,sizeof(buf));
	memcpy(buf,ps,end_ps-ps);	
	cls_no_asc(buf,strlen(buf));
	net->ip=ip_asc2ip(buf,strlen(buf));
	out("*ip=0x%08X\n",net->ip);
	out("找到IP\n");
	
	ps = end_ps+1;//指向ifconfig eth0 192.168.1.253/之后
	str = ps;
	end_ps = strstr(str,"\n");
	if((end_ps==NULL)||(end_ps>end))
	{	out("没有:[换行]\n");
		return -2;
	}
	i =  end_ps-ps;
	if(i>4)
	{	out("不符合子网掩码地址设置规范\n");
		return -3;
	}
	memset(mask,0,sizeof(buf));
	memcpy(mask,ps,end_ps-ps);
	netmask = atoi(mask);
	out("netmask = %d\n",netmask);
	i = dton(netmask);
	net->mask = strap_32(i);
	//net->mask = dton(netmask);
	out("net->mask=0x%08X\n",net->mask);
	out("找到子网掩码\n");
	
	ps = end_ps+1;//指向ifconfig eth0 192.168.1.253/24\n之后
	str = ps ;
	ps = strstr(str,"route add default gw ");
	ps+=strlen("route add default gw ");//指向route add default gw 之后
	end_ps=strstr(ps,"\n");
	if((end_ps==NULL)||(end_ps>end))
	{	out("没有:[换行]\n");
		return -2;
	}
	i =  end_ps-ps;
	if(i>sizeof(buf))
	{	out("不符合ip地址设置规范\n");
		return -3;
	}
	memset(buf,0,sizeof(buf));
	memcpy(buf,ps,end_ps-ps);	
	cls_no_asc(buf,strlen(buf));
	net->gateway[0]=ip_asc2ip(buf,strlen(buf));
	out("net->gateway=0x%08X\n",net->gateway[0]);
	out("找到网关\n");
	return 0;
}

int get_mac_addr(uint8 *macaddr)
{	struct ifreq ifreq;
	int i;
	unsigned char mac[15];
	int sock;
	sock = socket(AF_INET,SOCK_STREAM,0);
	if(sock<0)
	{
		out("打开socket失败\n");
		return -1;
	}
	strcpy(ifreq.ifr_name,"eth0");
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

	sprintf((char *)mac,":%02X:%02X:%02X:%02X:%02X\n",	(unsigned char)ifreq.ifr_hwaddr.sa_data[1],
												(unsigned char)ifreq.ifr_hwaddr.sa_data[2],
												(unsigned char)ifreq.ifr_hwaddr.sa_data[3],
												(unsigned char)ifreq.ifr_hwaddr.sa_data[4],
												(unsigned char)ifreq.ifr_hwaddr.sa_data[5]);
	out("mac =");
	for(i=0;i<15;i++)
	{
		out("%02x \n",mac[i]);
	}
	out("\n");
	//printf("mac = %s\n", macaddr);
	memcpy(macaddr,mac,15);
	close(sock);
	return 0;

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

static int pack_a20_eth(char *eth,struct _net_define  *net,char *o_buf,uint8 *mac_addr)
{	int netmask;
	//char buf[2];
	sprintf(o_buf,"#!/bin/sh\n\n");
	sprintf(o_buf+strlen(o_buf),"MAC_ADDR=\"%s\"\n\n",mac_addr);
	sprintf(o_buf + strlen(o_buf), "ifconfig %s down\n", eth);//add by whl
	sprintf(o_buf+strlen(o_buf),"ifconfig %s hw ether \"48$MAC_ADDR\"\n",eth);
	sprintf(o_buf+strlen(o_buf),"ifconfig lo 127.0.0.1\n");	
	pack_eth_ip(net->ip,"ifconfig eth0",o_buf);
	sprintf(o_buf+strlen(o_buf),"/");
	netmask = ntod(net->mask);
	out("pack_eth netmask = %d\n",netmask);
	sprintf(o_buf+strlen(o_buf),"%d\n",netmask);
	pack_eth_ip(net->gateway[0] ,"route add default gw",o_buf);
	sprintf(o_buf+strlen(o_buf),"\n\n\n");
	return strlen(o_buf); 	
}

int set_net_file(char *eth,struct _net_define  *net)
{	struct _net_define  o_net;
	//char buf[2048];
	
	uint8 mac[120] = { 0 };
	uint8 benjimac[15];
	//char w_buf[2048];
	char set_buf[500];
	//char *p_head,*p_end;
	int result;
	
	memset(&o_net,0,sizeof(o_net));
	out("set_net_file \n");
	result = get_a20_net_file(eth, &o_net, mac);
	if (result == 0)
	{
		out("已找到本机网络设置\n");
		//if(memcmp(net->ip,&o_net.ip,sizeof(uint32))==0)
		//{	out("IP已经与设置结果一样了\n");
		//	return 0;
		//}
		out("原来IP=%08x,要设置的IP=%08x\n",o_net.ip,net->ip);
		out("原来DNS=%08x,要设置的DNS=%08x\n",o_net.DNS,net->DNS);
		out("原来netmask=%08x,要设置的netmask=%08x\n",o_net.mask,net->mask);
		out("原来gateway=%08x,要设置的gateway=%08x\n",o_net.gateway[0],net->gateway[0]);
		if ((net->ip == o_net.ip) && (net->DNS == o_net.DNS) && (net->mask == o_net.mask) &&(net->gateway[0] == o_net.gateway[0]))
		{
			out("IP已经与设置结果一样了\n");
			return -1;
		}

	}
	//memset(buf,0,sizeof(buf));
	//if(read_file(DF_LINUX_NETWORK_FILE,buf,sizeof(buf))<0)
	//{	out("读取文件:[%s]失败\n",DF_LINUX_NETWORK_FILE);
	//	return -1;
	//}
	out("set_net_file gateway = %08x\n",net->gateway[0]);
	out("set_net_file ip = %08x\n",net->ip);
	out("set_net_file netmask = %08x\n",net->mask);
	out("set_net_file NDS = %08x\n",net->DNS);
	out("set_net_file MAC ADDRESS = %s \n",mac);
	get_mac_addr(benjimac);
	if(net->type==1)//如果是DHCP,保持文件中内容不变
	{
		if(memcmp(mac,benjimac,10)==0)
		{
			out("要设置的MAC地址与本机一致\n");
			return 0;
		}
		net->ip=o_net.ip;
		net->mask=o_net.mask;
		net->gateway[0]=o_net.gateway[0];
		net->DNS=o_net.DNS;

		memset(set_buf,0,sizeof(set_buf));
		pack_a20_eth(eth,net,set_buf,benjimac);
		out("写入文件[%s]:%s\n",DF_LINUX_NETWORK_FILE,set_buf);
		result = write_file(DF_LINUX_NETWORK_FILE, set_buf, strlen(set_buf));
		system("chmod 777 /etc/init.d/auto_config_network");
		sync();
		return result;
	}
	memset(set_buf,0,sizeof(set_buf));
	pack_a20_eth(eth,net,set_buf,benjimac);
	out("写入文件[%s]:%s\n",DF_LINUX_NETWORK_FILE,set_buf);
	result = write_file(DF_LINUX_NETWORK_FILE, set_buf, strlen(set_buf));
	system("chmod 777 /etc/init.d/auto_config_network");

	memset(set_buf,0,sizeof(set_buf));
	pack_dns(net,set_buf);
	out("写入文件[%s]:%s\n",DF_LINUX_DNS,set_buf);
	result = write_file(DF_LINUX_DNS, set_buf, strlen(set_buf));
	sync();
	return result;
}
 
int get_net_file(char *eth,struct _net_define  *net)
{	char buf[2048];
	char *p_file_head,*p_file_end;
	memset(buf,0,sizeof(buf));
	if (read_file(DF_LINUX_NETWORK_FILE, buf, sizeof(buf))<0)
	{	out("读取文件:[%s]失败\n",DF_LINUX_NETWORK_FILE);
		return -1;
	}	
	out("buf=[%s]\n",buf);
	if(get_file_head_end(buf,"#!/bin/sh",&p_file_head,&p_file_end)<0)
	{	out("ip文件内容错误\n",eth);
	   	return -2;
	}
	if(get_fe("ifconfig eth0 ",p_file_head,p_file_end,net)<0)
	{	out("获取IP地址失败\n");
		return -3;
	}
	
	p_file_head = NULL;
	p_file_end = NULL;
	memset(buf,0,sizeof(buf));
	if (read_file(DF_LINUX_DNS, buf, sizeof(buf))<0)
	{	out("读取文件:[%s]失败\n",DF_LINUX_NETWORK_FILE);
		return -1;
	}
	if(get_file_head_end(buf,"nameserver",&p_file_head,&p_file_end)<0)
	{	out("dns文件内容错误\n",eth);
	   	return -4;
	}
	if(get_dns("nameserver ",p_file_head,p_file_end,net)<0)
	{	out("获取IP地址失败\n");
		return -3;
	}
	return 0;	
}

static int get_dns(char *name,char *str,char *end,struct _net_define  *net)
{
	char *ps,*end_ps;	
	char buf[20];
	int i;
	ps=strstr(str,name);//指向第一个nameserver 开头
	if((ps==NULL)||(ps>end))
	{	out("1without:[%s]\n",name);
		return -1;
	}
	//out("find the first nameserver\n");
	ps+=strlen(name);//指向第一个nameserver 结尾
	str = ps;
	ps=strstr(str,name);//指向第二个nameserver 开头
	if((ps==NULL)||(ps>end))
	{	out("2without:[%s]\n",name);
		return -1;
	}
	//out("find the second nameserver\n");
	ps+=strlen(name);//指向第二个nameserver 结尾
	str = ps;
	ps=strstr(str,name);//指向第三个nameserver 开头
	if((ps==NULL)||(ps>end))
	{	out("3without:[%s]\n",name);
		return -1;
	}
	//out("find the third nameserver\n");
	ps+=strlen(name);//指向第三个nameserver 结尾
	str = ps;
	
	end_ps=strstr(str,"\n");
	if((end_ps==NULL)||(end_ps>end))
	{	out("with out:enter \n");
		return -2;
	}
	i =  end_ps-ps;
	//out("i= %d \n",i);
	if(i>sizeof(buf))
	{	out("不符合DNS地址设置规范\n");
		return -3;
	}
	memset(buf,0,sizeof(buf));
	memcpy(buf,ps,end_ps-ps);	
	cls_no_asc(buf,strlen(buf));
	net->DNS=ip_asc2ip(buf,strlen(buf));
	//out("*dns=0x%08X\n",net->DNS);
	//out("找到dns\n");
	return 0;
}

static int get_file_head_end(const char *buf,const char *eth,char **p_head,char **p_end)
{	char lan[100];
	memset(lan,0,sizeof(lan));
	//sprintf(lan,"#!/bin/sh");
	sprintf(lan,eth);
	*p_head=strstr(buf,lan);
	if(*p_head==NULL)
	{	out("[%s]未找到\n",lan);
		return -1;
	}
	*p_end=strstr(*p_head,"\n\n\n");
	if(*p_end==NULL)
	{	out("未找到数据结束\n");
		return -2;
	}
	//*p_head+=strlen(lan);
	*p_end+=2;
	return 0;
}

static int get_fe(char *name,char *str,char *end,struct _net_define  *net)
{	char *ps,*end_ps;	
	char buf[30];
	char mask[2];
	int i;
	int netmask;
	ps=strstr(str,name);//指向第一个ifconfig eth0 开头
	if((ps==NULL)||(ps>end))
	{	out("without:[%s]\n",name);
		return -1;
	}
	out("find the first ifconfig eth0\n");
	ps+=strlen(name);//指向第一个ifconfig eth0 结尾
	str = ps;
	ps=strstr(str,name);//指向第二个ifconfig eth0 开头
	if((ps==NULL)||(ps>end))
	{	out("without:[%s]\n",name);
		return -1;
	}
	out("find the second ifconfig eth0\n");
	ps+=strlen(name);//指向第二个ifconfig eth0 结尾
	str = ps;
	end_ps=strstr(ps,"/");
	if((end_ps==NULL)||(end_ps>end))
	{	out("with out:/\n");
		return -2;
	}
	i =  end_ps-ps;
	if(i>sizeof(buf))
	{	out("不符合ip地址设置规范\n");
		return -3;
	}
	memset(buf,0,sizeof(buf));
	memcpy(buf,ps,end_ps-ps);	
	cls_no_asc(buf,strlen(buf));
	net->ip=ip_asc2ip(buf,strlen(buf));
	out("*ip=0x%08X\n",net->ip);
	out("找到IP\n");
	
	ps = end_ps+1;//指向ifconfig eth0 192.168.1.253/之后
	str = ps;
	end_ps = strstr(str,"\n");
	if((end_ps==NULL)||(end_ps>end))
	{	out("没有:[换行]\n");
		return -2;
	}
	i =  end_ps-ps;
	if(i>4)
	{	out("不符合子网掩码地址设置规范\n");
		return -3;
	}
	memset(mask,0,sizeof(buf));
	memcpy(mask,ps,end_ps-ps);
	netmask = atoi(mask);
	out("netmask = %d\n",netmask);
	i = dton(netmask);
	net->mask = strap_32(i);
	//net->mask = dton(netmask);
	out("net->mask=0x%08X\n",net->mask);
	out("找到子网掩码\n");
	
	ps = end_ps+1;//指向ifconfig eth0 192.168.1.253/24\n之后
	str = ps ;
	ps = strstr(str,"route add default gw ");
	ps+=strlen("route add default gw ");//指向route add default gw 之后
	end_ps=strstr(ps,"\n");
	if((end_ps==NULL)||(end_ps>end))
	{	out("没有:[换行]\n");
		return -2;
	}
	i =  end_ps-ps;
	if(i>sizeof(buf))
	{	out("不符合ip地址设置规范\n");
		return -3;
	}
	memset(buf,0,sizeof(buf));
	memcpy(buf,ps,end_ps-ps);	
	cls_no_asc(buf,strlen(buf));
	net->gateway[0]=ip_asc2ip(buf,strlen(buf));
	out("net->gateway=0x%08X\n",net->gateway[0]);
	out("找到网关\n");
	return 0;
}

static int dton(int mask) 
{	uint32 i;
    int bits = sizeof(uint32) * 8; 
    i = ~0;
    bits -= mask;
    i <<= bits;
	return htonl(i);
}

static int ntod(uint32 mask) 
{   
	float f;
    mask = -mask; 
    f = mask; 
    mask = *(unsigned *)&f; 
    mask = 159 - (mask >> 23); 
    return mask;
}

static int strap_32(int data)
{
	int result;
	int temp[4];
	temp[0]=data>>24&0xff;
	temp[1]=data>>16&0xff;
	temp[2]=data>>8&0xff;
	temp[3]=data&0xff;
	out("temp[0]=%d temp[1]=%d temp[2]=%d temp[3]=%d\n",temp[0],temp[1],temp[2],temp[3]);
	result = (temp[3]<<24&0xff000000) + (temp[2]<<16&0xff0000) + (temp[1]<<8&0xff00)+(temp[0]&0xff);
	out("result = %08x \n",result);
	return result;
}

int connect_ip_test(char *ip, int port, int timer_s)
{
	struct sockaddr_in add;
	int socket_fd;
	//int fd = 0;
	fd_set fdr, fdw;
	struct timeval timeout;
	int err = 0;
	int rc;
	int flags;
	int errlen = sizeof(err);
	//out("connect_ip_test \n");
	socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	memset(&add, 0, sizeof(add));
	add.sin_family = AF_INET;
	inet_pton(AF_INET, ip, &add.sin_addr);
	add.sin_port = htons(port);

	/*设置套接字为非阻塞*/
	flags = fcntl(socket_fd, F_GETFL, 0);
	if (flags < 0)
	{
		fprintf(stderr, "Get flags error:%s\n", strerror(errno));
		shutdown(socket_fd, SHUT_RDWR);
		close(socket_fd);
		return -1;
	}
	flags |= O_NONBLOCK;
	if (fcntl(socket_fd, F_SETFL, flags) < 0)
	{
		fprintf(stderr, "Set flags error:%s\n", strerror(errno));
		shutdown(socket_fd, SHUT_RDWR);
		close(socket_fd);
		return -2;
	}

	//out("连接服务器:%s:%d\n",ip,port);
	rc = connect(socket_fd, (struct sockaddr *)&add, sizeof(add));
	if (rc != 0)
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
			shutdown(socket_fd, SHUT_RDWR);
			close(socket_fd);
			return -3;
		}
		/*连接超时*/
		if (rc == 0)
		{   //fprintf(stderr, "Connect timeout.\n");
			shutdown(socket_fd, SHUT_RDWR);
			close(socket_fd);
			return -4;
		}
		/*[1] 当连接成功建立时，描述符变成可写,rc=1*/
		if (rc == 1 && FD_ISSET(socket_fd, &fdw))
		{
			//printf("Connect success\n");
			shutdown(socket_fd, SHUT_RDWR);
			close(socket_fd);
			return 0;
		}
		/*[2] 当连接建立遇到错误时，描述符变为即可读，也可写，rc=2 遇到这种情况，可调用getsockopt函数*/
		if (rc == 2)
		{
			if (getsockopt(socket_fd, SOL_SOCKET, SO_ERROR, &err, (socklen_t *)&errlen) == -1)
			{
				fprintf(stderr, "getsockopt(SO_ERROR): %s", strerror(errno));
				shutdown(socket_fd, SHUT_RDWR);
				close(socket_fd);
				return -5;
			}
			if (err)
			{
				errno = err;
				//fprintf(stderr, "connect error:%s\n", strerror(errno));
				shutdown(socket_fd, SHUT_RDWR);
				close(socket_fd);
				return -6;
			}
		}
		//} 
		fprintf(stderr, "IP %s connect failed, error:%s.\n", ip, strerror(errno));
		shutdown(socket_fd, SHUT_RDWR);
		close(socket_fd);
		return -7;
	}
	shutdown(socket_fd, SHUT_RDWR);
	close(socket_fd);
	return 0;
}