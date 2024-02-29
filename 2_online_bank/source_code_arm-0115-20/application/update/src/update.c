#include "../inc/update.h"

void get_boot_config_md5(unsigned char *outbuf)
{
	FILE* fp = NULL;
	fp = fopen("/etc/boot_update.info", "r");
	if (fp == NULL)
		return;
	fseek(fp, 0, SEEK_SET);
	fgets((char *)outbuf, 256, fp);
	fclose(fp);
	return;
}

void jugde_boot_update(MODULE *par)
{
	int result;
	unsigned char jugde_Md5[50] = { 0 };
	unsigned char boott_Md5[50] = { 0 };
	unsigned char mb_bootMd5[50] = { 0 };
	get_boot_config_md5(jugde_Md5);
	//获取最新的boot MD5值
	result = read_file_md5("/home/share/exe/boot_t", boott_Md5);
	if (result < 0)
	{
		printf("boot_t open or md5 failed!\n ");
		par->boot_update = 1;
		return;
	}
	printf("boot_md5:%s\n", boott_Md5);
	if (memcmp(boott_Md5, jugde_Md5, strlen((char *)boott_Md5)) != 0)//下载下来的boot程序不正确或已经替换完毕
	{
		printf("Boot incorrect or Already update!\n ");
		par->boot_update = 1;
		return;
	}
	read_file_md5("/home/share/exe/mb_boot", mb_bootMd5);

	if (memcmp(mb_bootMd5, jugde_Md5, strlen((char *)mb_bootMd5)) == 0)
	{
		printf("Do not need to update boot!\n ");
		par->boot_update = 1;
		return;
	}
	else
	{
		//get_boot_size(&boot_size);
		//if (boot_size < 129584)//判断boot_size大小是否正常
		{
			//异常  进行更新
			if (access("/home/share/exe/boot_t", 0) == 0)
			{
				result = detect_process("/home/share/exe/mb_boot", "tmpm.txt");
				if (result > 0)
				{
					write_log("/home/share/exe/mb_boot进程存在,需结束\n");
					check_and_close_process("/home/share/exe/mb_boot");
				}

				system("mv /home/share/exe/boot_t /home/share/exe/mb_boot");
				system("chmod 777 /home/share/exe/mb_boot");
				sync();

				logout(INFO, "UPDATE","BOOT_UPDATE", "mb_boot升级替换完成\r\n");
			}
		}
	}
}

void update_task_hook(void *arg, int timer)
{
	FILE *fp_app,*fp_auth,*fp_need;
	int result = 0;
	char cmd[200] = { 0 };
	MODULE *par = (MODULE *)arg;

	//update_out("-----------------enter update_task_hook-------------------------\n");

	if(NULL == par)
	{
		return;
	}

	if (par->boot_update == 0)
	{
		jugde_boot_update(par);
	}

	if (access("/etc/update_app_mb.log", 0) == 0)
	{
		fp_app = fopen("/etc/update_app_mb.log", "rb");
		if (fp_app != NULL)
		{
			fclose(fp_app);
			goto update;
		}
	}
	if (access("/etc/update_auth_mb.log", 0) == 0)
	{
		fp_auth = fopen("/etc/update_auth_mb.log", "rb");
		if (fp_auth != NULL)
		{
			fclose(fp_auth);
			goto update;
		}
	}
	if (access("/etc/need_update_auth.log", 0) == 0)
	{
		fp_need = fopen("/etc/need_update_auth.log", "rb");
		if (fp_need != NULL)
		{
			fclose(fp_need);
			goto update;
		}
	}
	//update_out("-----------------exit update_task_hook-------------------------\n");
	return;
update :
	update_out("需要触发机柜程序重启或机柜重启\n");
	result = *((int *)((get_usb_port_class())->fun_check_port_status(NULL)));
	//result = check_port_status(par);
	if (result < 0)////有端口正在被使用
	{
		update_out("存在端口占用暂不触发重启\n");
		return;
	}

	_scheduled_task_close(par->m_task_fd);
	update_out("关闭mqtt消息队列处理线程成功\n");


	if (access("/etc/update_auth_mb.log", 0) == 0)
	{
		memset(cmd, 0, sizeof(cmd));
		sprintf(cmd, "rm -rf /etc/update_auth_mb.log");
		system(cmd);	
		logout(INFO, "SYSTEM", "UPDATE", "检测到授权更新,即将重启程序\r\n");
		sync();
	}

	if (access("/etc/update_app_mb.log", 0) == 0)
	{
		memset(cmd, 0, sizeof(cmd));
		sprintf(cmd, "rm -rf /etc/update_app_mb.log");
		system(cmd);		
		logout(INFO, "SYSTEM", "UPDATE", "检测到程序更新,即将更新程序\r\n");

		sync();
	}

	if (access("/etc/need_update_auth.log", 0) == 0)
	{
		memset(cmd, 0, sizeof(cmd));
		sprintf(cmd, "rm -rf /etc/need_update_auth.log");
		system(cmd);		
		logout(INFO, "SYSTEM", "UPDATE", "检测到需要程序更新,5秒后即将重启机柜\r\n");

		sync();
		sleep(5);
		ty_ctl(par->machine_fd, DF_MACHINE_INFOR_CM_RESET_POWER, DF_EVENT_REASON_NET);
		sleep(10);
	}

	


	/*memset(cmd, 0, sizeof(cmd));
	sprintf(cmd, "reboot");
	system(cmd);*/
	logout(INFO, "SYSTEM", "UPDATE", "检测到需要程序更新,退出程序\r\n");

	exit(0);
	return ;


}


void intranet_update_task_hook(void *arg, int timer)
// void intranet_update_task_hook(void *arg)
{
	FILE *fp_app,*fp_auth,*fp_need;
	int result = 0;
	char cmd[200] = { 0 };
	MODULE *par = (MODULE *)arg;

	if(NULL == par)
	{
		return;
	}

	if (par->boot_update == 0)
	{
		jugde_boot_update(par);
	}

	if (access("/etc/update_app_mb_m.log", 0) == 0)
	{
		update_out("----------------		all  	intranet_update		succeed !!!	-------------------------\n");
		fp_app = fopen("/etc/update_app_mb_m.log", "rb");
		if (fp_app != NULL)
		{
			fclose(fp_app);
			goto update;
		}
	}
	// if (access("/etc/update_auth_mb.log", 0) == 0)
	// {
	// 	fp_auth = fopen("/etc/update_auth_mb.log", "rb");
	// 	if (fp_auth != NULL)
	// 	{
	// 		fclose(fp_auth);
	// 		goto update;
	// 	}
	// }
	// if (access("/etc/need_update_auth.log", 0) == 0)
	// {
	// 	update_out("-----------------			【enter		 update_task_hook】		-------------------------\n");
	// 	fp_need = fopen("/etc/need_update_auth.log", "rb");
	// 	if (fp_need != NULL)
	// 	{
	// 		fclose(fp_need);
	// 		goto update;
	// 	}
	// }
	// update_out("-----------------exit update_task_hook-------------------------\n");
	return;
update :
	update_out("需要触发机柜程序重启或机柜重启\n");
	result = *((int *)((get_usb_port_class())->fun_check_port_status(NULL)));
	//result = check_port_status(par);
	if (result < 0)////有端口正在被使用
	{
		update_out("存在端口占用暂不触发重启\n");
		return;
	}

	_scheduled_task_close(par->m_task_fd);
	update_out("关闭mqtt消息队列处理线程成功\n");


	// if (access("/etc/update_auth_mb.log", 0) == 0)
	// {
	// 	memset(cmd, 0, sizeof(cmd));
	// 	sprintf(cmd, "rm -rf /etc/update_auth_mb.log");
	// 	system(cmd);	
	// 	logout(INFO, "SYSTEM", "UPDATE", "检测到授权更新,即将重启程序\r\n");
	// 	sync();
	// }

	if (access("/etc/update_app_mb_m.log", 0) == 0)
	{
		memset(cmd, 0, sizeof(cmd));
		sprintf(cmd, "rm -rf /etc/update_app_mb_m.log");
		system(cmd);		
		logout(INFO, "SYSTEM", "UPDATE", "检测到程序更新,即将更新程序\r\n");

		sync();
		sleep(5);
		ty_ctl(par->machine_fd, DF_MACHINE_INFOR_CM_RESET_POWER, DF_EVENT_REASON_NET);
		sleep(10);
	}

	// if (access("/etc/need_update_auth.log", 0) == 0)
	// {
	// 	memset(cmd, 0, sizeof(cmd));
	// 	sprintf(cmd, "rm -rf /etc/need_update_auth.log");
	// 	system(cmd);		
	// 	logout(INFO, "SYSTEM", "UPDATE", "检测到需要程序更新,5秒后即将重启机柜\r\n");

	// 	sync();
	// 	sleep(5);
	// 	ty_ctl(par->machine_fd, DF_MACHINE_INFOR_CM_RESET_POWER, DF_EVENT_REASON_NET);
	// 	sleep(10);
	// }

	
	/*memset(cmd, 0, sizeof(cmd));
	sprintf(cmd, "reboot");
	system(cmd);*/
	logout(INFO, "SYSTEM", "UPDATE", "检测到需要程序更新,退出程序\r\n");

	exit(0);
	return ;


}


int app_file_get_new(const char *file, struct _app_file_note *file_note)
{
	struct _s_info_new infor;
	int result;
	memset(&infor, 0, sizeof(struct _s_info_new));
	//printf("step1\n");
	if (access(file, 0) != 0)
		return -1;

	//printf("step2\n");
	if ((result = app_file_get_infor_new_m((char *)file, &infor) )< 0)
	{
		printf("step3 err:%d\n",result);
		return -1;
	}
	//printf("step3\n");
	result = app_file_write_real_app_m((char *)file, infor.app_content, infor.app_len);
	if (result < 0)
	{
		free(infor.app_content);
		return -2;
	}
	//printf("step4\n");
	free(infor.app_content);
	return 0;

}


int mb_system_read_m(char *cmd, char *r_buf, int r_buf_len)
{
	char order[200];
	char tmp_file[100];
	int fd;
	int result;

	memset(order, 0, sizeof(order));
	memset(tmp_file, 0, sizeof(tmp_file));
	sprintf(tmp_file, "/tmp/system_cmd.log");
	sprintf(order, "%s  > %s 2>&1", cmd, tmp_file);
	system(order);

	fd = open(tmp_file, O_RDONLY);
	if (fd<0)
	{
		return -1;
	}
	result = read(fd, r_buf, r_buf_len);
	if (result<0)
	{
		;
	}
	close(fd);
	mb_delete_file_dir_m(tmp_file, 0);
	sync();
	return 0;
}

int get_dir_m(const char *dir,char *r_dir)
{   char *pbuf,*pbuf_e;
    int n;
    for(pbuf=(char *)dir,n=0,pbuf_e=NULL;pbuf!=NULL;n++)
    {   pbuf=strstr(pbuf,"/");
        if(pbuf==NULL)
            break;
        pbuf_e=pbuf;
        pbuf++;
    }
    if(pbuf_e==NULL)
    {   memcpy(r_dir,"/",1);
        return 0;
    }
    memcpy(r_dir,dir,(pbuf_e-dir));
    memcpy(r_dir+strlen(r_dir),"/",1);
    return 0;   
}

int get_dir_file_m(const char *dir,char *r_name)
{   char *pbuf,*pbuf_e;
    int n;
    for(pbuf=(char *)dir,n=0,pbuf_e=NULL;pbuf!=NULL;n++)
    {   pbuf=strstr(pbuf,"/");
        if(pbuf==NULL)
            break;
        pbuf_e=pbuf;
        pbuf++;
    }
    if(pbuf_e==NULL)
    {   printf("字符中不存在[/]\n");
        memcpy(r_name,dir,strlen(r_name));
        return 0;
    }
    pbuf_e++;
    memcpy(r_name,pbuf_e,strlen(pbuf_e));
    return 0;
}

static int get_soft_file(const char *file,char *r_file)
{   //char r_name[100];
    get_dir_m(file,r_file);
    get_dir_file_m(file,r_file+strlen(r_file));
    memcpy(r_file+strlen(r_file),"_d",2);
    return 0;
}

int app_file_detect_m(const char *file,char *outerr)
{
	char soft_file[200];
	char order[200];
	char *p_e = NULL, *p_f = NULL;
	char read_stream[1024] = { 0 };
	

	if (outerr == NULL || file == NULL)
		return -1;
	if (access(file, 0) != 0)
		return -1;
	memset(soft_file, 0, sizeof(soft_file));
	get_soft_file(file, soft_file);
	mb_chmod_file_path_m("777", "/home/share/exe/mb_usbshare_*");
	memset(order, 0, sizeof(order));
	sprintf(order, "%s -V", soft_file);
	printf("order=[%s]\n", order);
	
	
	mb_system_read_m(order, read_stream, sizeof(read_stream));


	if ((p_e = strstr(read_stream, "err")) != NULL)//检测失败，拿出错误信息返回
	{
		p_f = strstr(p_e, "\n");
		memcpy(outerr, p_e, p_f - p_e);
		printf("the err info is :\n%s\n", outerr);
		return -1;
	}
	else if ((p_e = strstr(read_stream, "DETECT SUCCESS")) == NULL)//没有收到程序正常检测完成的标识，标识程序未启动
	{
		memcpy(outerr, "app start err!", 14);
		return -1;
	}
	return 0;
}

int dectec_app(char *errinfo)
{
	int i;
	//int s_ver = 0;
	int result;
	int flag_found_app = 0;
	struct _app_file_note   file_note;
	mb_chmod_file_path_m("777", "/home/share/exe/mb_usbshare_*");
	for (i = 0; i < sizeof(soft_d) / sizeof(const char *); i++)
	{
		result = app_file_get_new(soft_d[i], &file_note);
		if (result < 0)
			continue;
		else
		{
			flag_found_app = 1;
			break;
		}

	}

	if (flag_found_app == 1)
	{
		mb_chmod_file_path_m("777", "/home/share/exe/mb_usbshare_*");
		result = app_file_detect_m(soft_d[i], errinfo);
		if (result < 0)
			return result;
		return 0;
	}
	return -1;
}

int GetDnsIp(char *hostname, char *hostip)
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
		//out("is ip\n");
		//out("hostnamebuf222 %s\n", hostnamebuf);
		memcpy(hostip, hostnamebuf, len);
		return len;
	}
	if (len<5)
	{
		//out("域名长度错误\n");
		return -1;
	}
	//out("hostnamebuf %s\n", hostnamebuf);
	if (memcmp(hostnamebuf, "127.0.0.1", len) == 0)
	{
		//out("本机IP不用解析\n");
		sprintf((char *)hostip, "127.0.0.1");
		return strlen((const char*)hostip);
	}
	h = gethostbyname((const char *)hostnamebuf);
	if (h == NULL)
	{
		//out("解析IP地址失败\n");
		return -1;
	}
	else
	{
		memcpy(&addr_in.sin_addr.s_addr, h->h_addr, 4);
		in.s_addr = addr_in.sin_addr.s_addr;
	}
	ip_len = strlen(inet_ntoa(in));
	memcpy(hostip, inet_ntoa(in), ip_len);

	return ip_len;
}



/*********************************************************************
-   Function : get_line_m
-   Description：获取一行信息
-   Calls：
-   Calles By:
-   Table Accessed :
-   Table Update :
-   Input  :参数一:
-   Output :
-   Return :：错误 其他：正常
-   Other :
***********************************************************************/
int get_line_m(int sock, char *buf, int size)
{
	int i = 0;
	unsigned char c = '\0';
	int n;
	//int j;

	/*把终止条件统一为 \n 换行符，标准化 buf 数组*/
	while ((i < size - 1) && (c != '\n'))
	{
		/*一次仅接收一个字节*/
		n = recv(sock, &c, 1, 0);
		/* DEBUG*/
		//printf("%02x ", c); 
		if (n > 0)
		{
			/*收到 \r 则继续接收下个字节，因为换行符可能是 \r\n */
			if (c == '\r')
			{
				/*使用 MSG_PEEK 标志使下一次读取依然可以得到这次读取的内容，可认为接收窗口不滑动*/
				n = recv(sock, &c, 1, MSG_PEEK);
				/* DEBUG printf("%02X\n", c); */
				//out("\n但如果是换行符则把它吸收掉\n");
				if ((n > 0) && (c == '\n'))
				{
					recv(sock, &c, 1, 0);
					//printf("%02x ", c); 
				}
				else
				{
					c = '\n';
					//printf("%02x ", c); 
				}
			}
			/*存到缓冲区*/
			buf[i] = c;
			i++;
		}
		else
			c = '\n';
	}
	//printf("\n")
	return(i);
}


/*********************************************************************
-   Function : deal_http_data
-   Description：接收完整HTTP数据
-   Calls：
-   Calles By:
-   Table Accessed :
-   Table Update :
-   Input  :参数一:sockfd:TCP套接字
参数二:outbuf:接收缓冲HTTP body实体内容
-   Output :
-   Return :0 正常 非零 错误
-   Other :
***********************************************************************/
int deal_http_data_m(int sockfd, char **outbuf)
{
	char buf[1024];
	int i;
	char c;
	char len_buf[50];
	int numchars = 1;
	int content_length = -1;
	memset(buf, 0, sizeof(buf));
	printf("deal_http_data_m start!\r\n");
	numchars = get_line_m(sockfd, buf, sizeof(buf));
	printf("deal_http_data_m numchars%d\r\n",numchars);
	if (numchars == 0)
	{
		return -1;
	}
	memset(buf, 0, sizeof(buf));
	numchars = get_line_m(sockfd, buf, sizeof(buf));
	printf("deal_http_data_1 numchars%d\r\n",numchars);
	if (numchars == 0)
	{
		return -1;
	}
	while ((numchars > 0) && strcmp("\n", buf))
	{
		if (strncasecmp(buf, "Content-Length:", 15) == 0)
		{
			memset(len_buf, 0, sizeof(len_buf));
			memcpy(len_buf, &buf[15], 20);
			cls_no_asc(len_buf, strlen((char*)len_buf));
			content_length = atoi(len_buf);
		}
		memset(buf, 0, sizeof(buf));
		numchars = get_line_m(sockfd, buf, sizeof(buf));

	}
	printf("deal_http_data2 content_length%d\r\n",content_length);
	/*没有找到 content_length */
	if (content_length <= 0) {
		printf("error recv,length err !content_length = %d\n", content_length);
		return -1;
	}
	/*接收 POST 过来的数据*/
	long glen = sizeof(char*)*content_length + 100;

	char *tmp = NULL;
	tmp = boot_malloc(glen);
	if (tmp == NULL)
	{
		printf("err maclloc!\n");
		return -1;
	}
	memset(tmp, 0, glen);
	for (i = 0; i < content_length; i++)
	{
		recv(sockfd, &c, 1, 0);
		tmp[i] = c;
	}
	printf("recv over!	1\r\n");
	*outbuf = tmp;
	printf("recv over!	2\r\n");
	return content_length;
}

int http_api_test(int socketfd,char *ip,int port)
{
	int result;
	unsigned char s_buf[1024] = { 0 };
	unsigned char *r_buf = NULL;

	memset(s_buf, 0, sizeof(s_buf));
	sprintf((char*)s_buf, "POST %s HTTP/1.1\r\n", "/api_test");
	sprintf((char*)s_buf + strlen((char*)s_buf), "Content-Type: %s\r\n", HTTP_CONTENT_TYPE);
	sprintf((char*)s_buf + strlen((char*)s_buf), "Timeout: 5000\r\n");
	sprintf((char*)s_buf + strlen((char*)s_buf), "Host: %s:%d\r\n", ip, port);
	sprintf((char *)s_buf + strlen((char*)s_buf), "Content-Length: 0\r\n");
	sprintf((char*)s_buf + strlen((char*)s_buf), "User-Agent: libghttp/1.0\r\n");
	sprintf((char*)s_buf + strlen((char*)s_buf), "\r\n");


	if (socket_write(socketfd, (unsigned char *)s_buf, strlen((char*)s_buf))<0)
	{
		printf("http_api_test send err\n");
		result = -1;
	}
	net_update_out("	test  	1	\n");
	result = deal_http_data_m(socketfd, (char **)&r_buf);
	net_update_out("	test  	2	result = %d\n",result);
	if (result < 0)
	{
		printf("http_api_test recv http error!\n");
		return -2;
	}
	
	if (strstr((char *)r_buf, "MBI_TERUPGRADEAPI") != NULL)
	{
		free(r_buf);
		return 0;
	}
	free(r_buf);
	return -1;
}



int get_real_host(char *config_host, char *real_host,char *outip,int port)
{
	char ip[100] = { 0 };
	int result = 0;
	int sock;
	if (GetDnsIp(config_host, ip) >= 0)
	{
		
		//return 0;
		result = http_socket_open_m(&sock, ip, port);
		if (result <0)
		{
			printf("socket create failed1\n");//socket 创建失败直接返回
		}
		result = http_api_test(sock, ip,port);
		if (result < 0)
		{
			shutdown(sock, SHUT_RDWR);
			close(sock);
		}
		else//主域名可用直接返回
		{
			shutdown(sock, SHUT_RDWR);
			close(sock);
			memcpy(real_host, config_host, strlen(config_host));
			memcpy(outip, ip, strlen(ip));
			return 0;
		}
		
	}
	memset(ip, 0, sizeof(ip));
	if (GetDnsIp(HTTP_ADDR, ip) >= 0)
	{
		
		result = http_socket_open_m(&sock, ip,port);
		if (result <0)
		{
			printf("socket create failed2\n");//socket 创建失败直接返回
		}
		result = http_api_test(sock, ip,port);
		if (result < 0)
		{
			shutdown(sock, SHUT_RDWR);
			close(sock);
		}
		else//代码写死的备份域名可用
		{
			shutdown(sock, SHUT_RDWR);
			close(sock);
			//memset(real_host, 0, sizeof(real_host));
			memcpy(real_host, HTTP_ADDR, strlen(HTTP_ADDR));
			memcpy(outip, ip, strlen(ip));
			return 0;
		}
	}
	return -1;
}

/*********************************************************************
-   Function : http_get_ter_update
-   Description：通过http获取服务器升级文件
-   Calls：
-   Calles By:
-   Table Accessed :
-   Table Update :
-   Input  :参数一:socketfd:TCP套接字
参数二:parm:http_parm结构体，传入参数，内部存储所需要的各个值
-   Output :
-   Return :0 正常 <0失败
-   Other :
***********************************************************************/
int http_get_ter_update(int socketfd, struct http_parm_m *parm)
{
	unsigned char s_buf[1024] = { 0 };
	unsigned char *r_buf = NULL;
	unsigned char parmdata[500] = { 0 };
	unsigned char sort_sign[500] = { 0 };
	unsigned char md5_sha_sign[100] = { 0 };
	unsigned char hash[100] = { 0 };
	unsigned char outmd[16] = { 0 };
	unsigned char outmd_up[50] = { 0 };
	unsigned char app_update_path[200] = { 0 };
	unsigned char *dfile = NULL;
	char *g_buf = NULL;
	long file_len = 0;
	char *p = NULL;
	int  result;
	cJSON *root = NULL, *recv_root = NULL, *item = NULL, *res = NULL;
	MD5_CTX ctx;
	SHA_CTX s;
	//进行url的首字母排序和body组合在一起
	sprintf((char *)sort_sign, "appid=%s&format=json&timestamp=%s&token=%s&version=1.0{\"ter_id\":\"%s\",\"main_pro_ver\":\"%s\",\"tar_md5\":\"%s\"}", (char *)parm->appid, (char *)parm->timestamp, (char *)parm->token, (char *)parm->ter_id, (char *)parm->app_ver, (char *)parm->tar_md5);

	printf("111__________________________sort_sign: %s\n\n\n\n", sort_sign);
	MD5_Init(&ctx);
	MD5_Update(&ctx, sort_sign, strlen((const char *)sort_sign));
	MD5_Final(outmd, &ctx);
	//MD5加密完成
	//MD5加密结果进行转16进制字符串再转大写
	HexToStr((char *)outmd_up, (char *)outmd, 16);
	p = str2upper((char *)outmd_up);

	//SHA1加密
	SHA1_Init(&s);
	SHA1_Update(&s, p, strlen(p));
	SHA1_Final(hash, &s);
	HexToStr((char *)md5_sha_sign, (char *)hash, 20);
	//sha1加密完成（转成16进制字符串)


	sprintf((char*)parmdata, "/was.tersvr/upgrade?version=1.0&appid=%s&timestamp=%s&sign=%s&token=%s&format=json", (char *)parm->appid,(char *)parm->timestamp, (char *)md5_sha_sign, (char *)parm->token);
	printf("222____________________parmdata:  %s\n\n\n\n", parmdata);
	memset(s_buf, 0, sizeof(s_buf));

	sprintf((char*)s_buf, "POST %s HTTP/1.1\r\n", (char *)parmdata);
	sprintf((char*)s_buf + strlen((char*)s_buf), "Content-Type: %s\r\n", HTTP_CONTENT_TYPE);
	sprintf((char*)s_buf + strlen((char*)s_buf), "Timeout: 5000\r\n");
	sprintf((char*)s_buf + strlen((char*)s_buf), "Host: %s:%d\r\n", parm->ip, parm->port);

	root = cJSON_CreateObject();
	if (root == NULL)
	{
		printf("http get ter update json create err!\n");
		return -1;
	}
	cJSON_AddStringToObject(root, "ter_id", (const char *)parm->ter_id);
	cJSON_AddStringToObject(root, "main_pro_ver", (const char *)parm->app_ver);
	cJSON_AddStringToObject(root, "tar_md5", (const char *)parm->tar_md5);

	g_buf = cJSON_PrintUnformatted(root);
	sprintf((char*)s_buf + strlen((char*)s_buf), "Content-Length: %d\r\n", strlen(g_buf) + 2);
	sprintf((char*)s_buf + strlen((char*)s_buf), "User-Agent: libghttp/1.0\r\n");
	sprintf((char*)s_buf + strlen((char*)s_buf), "\r\n");
	sprintf((char*)s_buf + strlen((char*)s_buf), "%s\r\n", g_buf);

	cJSON_Delete(root);
	free(g_buf);
	printf("333________________________http send by get ter update :\n\n\n %s\n\n\n\n", s_buf);
	if (socket_write(socketfd, (unsigned char *)s_buf, strlen((char*)s_buf)) <= 0)
	{
		printf("发送失败\n");
		return  BOOT_ERROR_HTTP_SEND;
	}
	printf("1111111111111111111111111111111\r\n");

	result = deal_http_data_m(socketfd, (char **)&r_buf);
	if (result < 0)
	{
		printf("http get token err!\n");
		return BOOT_ERROR_HTTP_RECV;
	}
	printf("1111111111111111111111111111112,		444_____________r_buf = %s\r\n\n\n",r_buf);
	


	recv_root = cJSON_Parse((const char *)r_buf);
	if (recv_root == NULL)
	{
		printf("err recv get ter update,is not a json !\n");
		free(r_buf);
		return BOOT_ERROR_HTTP_PARSE_JSON;
	}
	printf("1111111111111111111111111111113\r\n");
	free(r_buf);
	item = cJSON_GetObjectItem(recv_root, "result");
	if (item == NULL)
	{
		printf("err recv get ter update:json err1\n");
		cJSON_Delete(recv_root);
		return BOOT_ERROR_HTTP_PARSE_JSON;
	}
	if (memcmp(item->valuestring, "s", 1) != 0)
	{
		printf("err recv get ter update:json result is f:%s\n", r_buf);
		//判断是否token过期，如果过期进行重新取token操作
		item = cJSON_GetObjectItem(recv_root, "response");
		if (item == NULL)
		{
			printf("err recv get ter update:json err2\n");

			cJSON_Delete(recv_root);
			return BOOT_ERROR_HTTP_PARSE_JSON;
		}
		printf("err recv get ter update:json result is f get response succ\n");
		res = cJSON_GetObjectItem(item, "err");
		if (res == NULL)
		{
			printf("err recv get ter update:upgrade not found\n");

			cJSON_Delete(recv_root);
			return BOOT_ERROR_HTTP_PARSE_JSON;
		}
		printf("err recv get ter update:json result is f get err succ\n");
		if (memcmp(res->valuestring, "101", 3) == 0)
		{
			cJSON_Delete(recv_root);
			return HTTP_TOKEN_ERR;//重发
		}
		cJSON_Delete(recv_root);
		printf("free recv_root\n");
		return BOOT_ERROR_HTTP_PARSE_JSON;
	}
	printf("1111111111111111111111111111114\r\n");
	item = cJSON_GetObjectItem(recv_root, "response");
	if (item == NULL)
	{
		printf("err recv get ter update:json err2\n");

		cJSON_Delete(recv_root);
		return BOOT_ERROR_HTTP_PARSE_JSON;
	}
	printf("1111111111111111111111111111115\r\n");
	res = cJSON_GetObjectItem(item, "upgrade");
	if (res == NULL)
	{
		printf("err recv get ter update:upgrade not found\n");

		cJSON_Delete(recv_root);
		return BOOT_ERROR_HTTP_PARSE_JSON;
	}
	if (memcmp(res->valuestring, "0", 1) == 0)
	{
		printf("Do not need update APP!\n");

		cJSON_Delete(recv_root);
		return 1;
	}






	printf("1111111111111111111111111111116\r\n");
	res = cJSON_GetObjectItem(item, "upgradefilelen");
	if (res == NULL)
	{
		printf("err recv get ter update:upgradefilelen not found\n");

		cJSON_Delete(recv_root);
		return BOOT_ERROR_HTTP_PARSE_JSON;
	}
	file_len = atol(res->valuestring);
	printf("get file len:%ld,path_size = %d\n", file_len,calc_path_size("/"));
	if (file_len > 1024 * 1024 * 50)
	{
		printf("err file too large:%ld!\n", file_len);

		cJSON_Delete(recv_root);
		return BOOT_ERROR_HTTP_RECV_PACKET_TOO_LARGE;
	}
	//判断下当前剩余空间
	// if ((calc_path_size("/") * 1024 * 1024) <= file_len)
	// {
	// 	printf("system space is full!\n");

	// 	cJSON_Delete(recv_root);
	// 	return BOOT_ERROR_HTTP_RECV_PACKET_TOO_LARGE;
	// }
	res = cJSON_GetObjectItem(item, "upgradefile");
	if (res == NULL)
	{
		printf("err recv get ter update:upgradefile not found\n");

		cJSON_Delete(recv_root);
		return BOOT_ERROR_HTTP_PARSE_JSON;
	}
	dfile = boot_malloc(file_len+10);
	if (dfile == NULL)
	{
		return BOOT_ERROR_HTTP_SPACE_FULL;
	}
	//进行base64解码
	decode(res->valuestring, strlen(res->valuestring), (char *)dfile);
	printf("decode is ok!\n");
	sprintf((char *)app_update_path, "%s/tmpapp", "/home/share/exe/tar"); //"/home/share/exe/tar/tmpapp"
	result = Write_Decompress_file_m(dfile, file_len, (unsigned char *)"/tmp/app.tar", app_update_path);
	if (result < 0)
	{
		printf("err write update file :%d\n", result);

		if (dfile != NULL)
			free(dfile);
		cJSON_Delete(recv_root);
		return result;
	}
	free(dfile);
	cJSON_Delete(recv_root);

	return 0;

}


int mb_create_file_dir_m(char *file_path)
{
	char cmd[1000] = { 0 };
	sprintf(cmd, "mkdir %s", file_path);
	system(cmd);
	sync();
	return 0;
}

int mb_tar_decompress_m(char *tar, char *path)
{
	char cmd[1000] = { 0 };
	memset(cmd, 0, sizeof(cmd));
	sprintf(cmd, "tar -xf %s -C %s/", tar, path);
	system(cmd);
	sync();
	return 0;
}


int File_Compare_MD5_m(char *src_file, char *comp_file)
{
	int fds, fdc;
	int sizes = 0, sizec = 0, size_read = 0;
	struct stat statbufs;
	struct stat statbufc;
	char *buff_s = NULL, *buff_c = NULL;
	MD5_CTX ctx;
	char md5_s[16] = { 0 }, md5_c[16] = { 0 };
	if (src_file == NULL || comp_file == NULL)
		return -1;
	if (access(src_file, 0) != 0)
	{

		return BOOT_ERROR_OPER_PARSE_CONFIG_ERR_FILE_NOT_FOUND;
	}
	if (access(comp_file, 0) != 0)
	{
		printf("原目录文件不存在[%s],MD5值默认为不同 !\n", comp_file);
		return 0;//此种情况可以说是MD5值不同
	}
	stat(src_file, &statbufs);
	sizes = statbufs.st_size;
	buff_s = boot_malloc(sizes + 10);
	if (buff_s == NULL)
	{
		printf("err malloc!\n");
		return BOOT_ERROR_OPER_SYSTEM_MALLOC_ERR;
	}


	stat(comp_file, &statbufc);
	sizec = statbufc.st_size;
	buff_c = boot_malloc(sizec + 10);
	if (buff_c == NULL)
	{
		printf("err malloc!\n");
		free(buff_s);
		return BOOT_ERROR_OPER_SYSTEM_MALLOC_ERR;
	}


	fds = open(src_file, O_RDONLY);
	if (fds < 0)
	{
		printf("tar包下%s文件打开失败!\n", src_file);
		free(buff_c);
		free(buff_s);
		return BOOT_ERROR_OPER_SYSTEM_READ_ERR;
	}
	size_read = read(fds, buff_s, sizes);
	if (size_read != sizes)
	{
		printf("tar包%s文件读取大小和文件大小不一致!\n", src_file);
		free(buff_c);
		free(buff_s);
		close(fds);
		return BOOT_ERROR_OPER_SYSTEM_READ_ERR;
	}

	MD5_Init(&ctx);
	MD5_Update(&ctx, buff_s, sizes);
	MD5_Final((unsigned char *)md5_s, &ctx);

	close(fds);

	size_read = 0;
	fdc = open(comp_file, O_RDONLY);
	if (fdc < 0)
	{
		free(buff_c);
		free(buff_s);
		return 0;
	}
	size_read = read(fdc, buff_c, sizec);
	if (size_read != sizec)
	{
		//printf("read err ! not full!\n");
		free(buff_c);
		free(buff_s);
		close(fdc);
		return 0;
	}

	MD5_Init(&ctx);
	MD5_Update(&ctx, buff_c, sizec);
	MD5_Final((unsigned char *)md5_c, &ctx);

	close(fdc);

	if (memcmp(md5_s, md5_c, 16) != 0)
	{
		free(buff_c);
		free(buff_s);
		return 0;//MD5值不同
	}
	free(buff_c);
	free(buff_s);
	//printf("MD5值相同不进行替换!\n");
	return 1;//MD5值相同
	

}


int parse_config_copy_file_m(unsigned char *configname,unsigned char *now_path,int update_or_check)
{
	FILE *file;
	int result = 0;
	char lineBuff[256] = { 0 };// 存储读取一行的数据
	char* linePos = NULL;
	char file_name[100] = { 0 };
	char file_real_path[100] = { 0 };
	char tar_file_path[100] = { 0 };
	char real_file_path[100] = { 0 };
	//char cmd[100] = { 0 };
	char update_filename[200] = { 0 };
	
	if (access((char *)configname, 0) != 0)
	{
		return BOOT_ERROR_OPER_PARSE_CONFIG_NOT_FOUND_CONFIG;
	}
	file = fopen((char *)configname, "rb");
	if (!file)
	{
		return BOOT_ERROR_OPER_PARSE_CONFIG_ERR_OPEN_CONFIG;
	}
	fseek(file, 0, SEEK_END);
	ftell(file); //配置文件长度  
	fseek(file, 0, SEEK_SET);
	memset(lineBuff, 0, sizeof(lineBuff));
	//下面的循环是先判断config文件的格式是否正确
	while (fgets(lineBuff, 256, file) != NULL)
	{
		if ((lineBuff[0] == '#') || (lineBuff[0] == ';') || memcmp(lineBuff, "\r\n", 2) == 0 || lineBuff[0] == '\n')
			continue;
		linePos = strstr(lineBuff, "=");
		if (linePos == NULL)
		{
			printf("err format configfile :%s!\n", lineBuff);
			memset(lineBuff, 0, sizeof(lineBuff));
			fclose(file);
			return BOOT_ERROR_OPER_PARSE_CONFIG_ERR_FORMAT_CONFIG;
		}
	}
	fseek(file, 0, SEEK_SET);
	memset(lineBuff, 0, sizeof(lineBuff));
	while (fgets(lineBuff, 256, file) != NULL)
	{
		if ((lineBuff[0] == '#') || (lineBuff[0] == ';') || memcmp(lineBuff, "\r\n", 2) == 0 || lineBuff[0] == '\n')
			continue;
		linePos = strstr(lineBuff, " = ");
		if (linePos == NULL)
		{
			printf("err format configfile!\n");
			memset(lineBuff, 0, sizeof(lineBuff));
			fclose(file);
			return BOOT_ERROR_OPER_PARSE_CONFIG_ERR_FORMAT_CONFIG;
		}
		str_replace(lineBuff, "\r\n", "\n");

		memset(file_name, 0, sizeof(file_name));
		memcpy(file_name, lineBuff, strlen(lineBuff) - strlen(linePos));
		if (memcmp(file_name, "mb_boot", 7) == 0)//判断是否是本boot程序，如果是，则跳过.
			continue;
		memset(file_real_path, 0, sizeof(file_real_path));
		if (linePos[strlen(linePos) - 1] == '\n'&&linePos[strlen(linePos) - 2] == '\r')
			linePos[strlen(linePos) - 2] = '\0';
		else if (linePos[strlen(linePos) - 1] == '\n')
			linePos[strlen(linePos) - 1] = '\0';
		memcpy(file_real_path, linePos + 3, strlen(linePos) - 3);//减去一个" = "


		if (file_real_path[strlen(file_real_path) - 1] == '/')//统一格式(如路径中结尾有'/'，则去掉，后面统一加上)
		{
			file_real_path[strlen(file_real_path) - 1] = '\0';
		}

		memset(tar_file_path, 0, sizeof(tar_file_path));
		sprintf(tar_file_path, "%s/%s", now_path,file_name);
		//检测tar目录下是否有要更新的文件和是否存在真实复制替换的目录
		//printf("path1 = %s\npath2 = %s\n", file_real_path, tar_file_path);
		if (access(file_real_path, 0) != 0)
		{
			printf("err format configfile Path1 not found!\n");
			fclose(file);
			return BOOT_ERROR_OPER_PARSE_CONFIG_ERR_PATH_NOT_FOUND;
		}
		if (access(tar_file_path, 0) != 0)
		{
			printf("err format configfile Path1 not found!\n");
			fclose(file);
			return BOOT_ERROR_OPER_PARSE_CONFIG_ERR_FILE_NOT_FOUND;
		}
		memset(real_file_path, 0, sizeof(real_file_path));
		sprintf(real_file_path, "%s/%s", file_real_path, file_name);
		result = File_Compare_MD5_m(tar_file_path, real_file_path);
		printf("222222222222222222222222222222222\r\n");
		if (result == 0)
		{
			//MD5值不同进行替换
			//原系统有该文件，先备份该文件为filename.back,用于检测失败回退使用
			if (access(real_file_path, 0) == 0)
			{
				char backpath[200] = { 0 };
			/*	memset(cmd, 0, sizeof(cmd));
				sprintf(cmd, "cp %s %s.back", real_file_path, real_file_path);
				system(cmd);
				sync();*/
				printf("222222222222222222222222222222223\r\n");
				sprintf(backpath, "%s.back", real_file_path);
				mb_copy_file_path_m(real_file_path, backpath);
				printf("222222222222222222222222222222224\r\n");
			}
			//用本次tar压缩包下的文件替换系统下文件
			/*memset(cmd, 0, sizeof(cmd));
			sprintf(cmd, "cp %s %s/.", tar_file_path, file_real_path);
			system(cmd);
			sync();*/
			printf("222222222222222222222222222222225\r\n");
			mb_copy_file_path_m(tar_file_path, file_real_path);
			printf("222222222222222222222222222222226\r\n");
			//mb_chmod_file_path("777", (char *)real_file_path);

			printf("###################update_or_check:%d-----now_path:%s\n", update_or_check, now_path);

			if (update_or_check == 2 && strstr((char *)now_path,"app") !=NULL)//app.tar更新内容 //"/home/share/exe/tar/tmpapp"
			{
				printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
				memset(update_filename, 0, sizeof(update_filename));
				sprintf(update_filename, "updatefile=%s\n", real_file_path);
				strcat((char *)update_msg_m, update_filename);
			}
			else if (update_or_check == 2 && strstr((char *)now_path, "auth") != NULL)//auth.tar更新内容
			{
				printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!###########\n");
				memset(update_filename, 0, sizeof(update_filename));
				sprintf(update_filename, "updatefile=%s\n", real_file_path);
				strcat((char *)auth_msg_m, update_filename);
			}

		}
		else if (result < 0)
		{
			printf("err Compare MD5!\n");
			fclose(file);
			return result;
		}
		memset(lineBuff, 0, sizeof(lineBuff));
		continue;
	}
	fclose(file);
	return 0;
	
}


int mb_copy_file_path_m(char *from, char *to)
{
	char cmd[1000] = { 0 };
	memset(cmd, 0, sizeof(cmd));
	sprintf(cmd, "cp %s %s", from, to);
	system(cmd);
	sync();
	return 0;
}


/*********************************************************************
-   Function : Write_Decompress_file_m
-   Description：写入信息到指定文件并解压到指定目录下
-   Calls：
-   Calles By:
-   Table Accessed :
-   Table Update :
-   Input  :参数一:file:待写入的数据
参数二:file_len:待写入的数据长度
参数三:filename:指定的文件名
参数四:Dest_dir:指定的路径
-   Output :
-   Return :0：正常 <0：错误
-   Other :
***********************************************************************/
int Write_Decompress_file_m(unsigned char *file, long file_len, unsigned char *filename, unsigned char * Dest_dir)
{
	int file_fd, result;
	char config_path[200] = { 0 };
	mb_delete_file_dir_m((char *)filename,0);
	mb_delete_file_dir_m((char *)Dest_dir, 1);

	file_fd = open((char *)filename, O_CREAT | O_RDWR);
	if (file_fd < 0)
	{
		printf("err open file\n");
		return -1;
	}
	result = write(file_fd, file, file_len);
	if (result < 0)
	{
		printf("err write update file!\n");
		close(file_fd);
		return -1;
	}





	if (access((char *)Dest_dir, 0) != 0)
	{
		mb_create_file_dir_m((char *)Dest_dir);
	}

	mb_chmod_file_path_m("777", (char *)filename);
	mb_tar_decompress_m((char *)filename, (char *)Dest_dir); //    "/tmp/app.tar" 的压缩包 ，文件解压到这里"/home/share/exe/tar/tmpapp" 
	close(file_fd);
	sleep(1);//等待解压完成
	//进行解压文件的校验和复制到当前系统
	sprintf((char*)config_path, "%s/mb_config.cnf", Dest_dir); 
	printf("Write_Decompress_file start\r\n");			//wang
	result = parse_config_copy_file_m((unsigned char *)config_path, Dest_dir,2);//"/home/share/exe/tar/tmpapp"
	if (result < 0)
		return result;
	////没有问题 先复制该tar包到/tmpapp | /tmpauth目录下用于一会检测成功后替换原有的包
	printf("Write_Decompress_file start1\r\n");			//wang
	mb_copy_file_path_m((char *)filename, (char *)Dest_dir);
	printf("Write_Decompress_file over\r\n");			//wang
	return 0;


}


int parse_config_del_backfile(char *configname, char *now_path)
{

	FILE *file;
	//int result = 0;
	char lineBuff[256] = { 0 };// 存储读取一行的数据
	char* linePos = NULL;
	char file_name[100] = { 0 };
	char file_real_path[100] = { 0 };
	char tar_file_path[100] = { 0 };
	char real_file_path[100] = { 0 };
	char backfile_path[100] = { 0 };
	char cmd[100] = { 0 };
	if (access((char *)configname, 0) != 0)
	{
		return BOOT_ERROR_OPER_PARSE_CONFIG_NOT_FOUND_CONFIG;
	}
	file = fopen((char *)configname, "rb");
	if (!file)
	{
		return BOOT_ERROR_OPER_PARSE_CONFIG_ERR_OPEN_CONFIG;
	}
	fseek(file, 0, SEEK_END);
	ftell(file); //配置文件长度  
	fseek(file, 0, SEEK_SET);
	memset(lineBuff, 0, sizeof(lineBuff));
	//下面的循环是先判断config文件的格式是否正确
	while (fgets(lineBuff, 256, file) != NULL)
	{
		if ((lineBuff[0] == '#') || (lineBuff[0] == ';') || memcmp(lineBuff, "\r\n", 2) == 0 || lineBuff[0] == '\n')
			continue;
		linePos = strstr(lineBuff, "=");
		if (linePos == NULL)
		{
			printf("err format configfile!\n");
			memset(lineBuff, 0, sizeof(lineBuff));
			fclose(file);
			return BOOT_ERROR_OPER_PARSE_CONFIG_ERR_FORMAT_CONFIG;
		}
	}
	fseek(file, 0, SEEK_SET);
	memset(lineBuff, 0, sizeof(lineBuff));
	while (fgets(lineBuff, 256, file) != NULL)
	{
		if ((lineBuff[0] == '#') || (lineBuff[0] == ';') || memcmp(lineBuff, "\r\n", 2) == 0 || lineBuff[0] == '\n')
			continue;
		linePos = strstr(lineBuff, " = ");
		if (linePos == NULL)
		{
			printf("err format configfile!\n");
			memset(lineBuff, 0, sizeof(lineBuff));
			fclose(file);
			return BOOT_ERROR_OPER_PARSE_CONFIG_ERR_FORMAT_CONFIG;
		}
		str_replace(lineBuff, "\r\n", "\n");

		memset(file_name, 0, sizeof(file_name));
		memcpy(file_name, lineBuff, strlen(lineBuff) - strlen(linePos));

		memset(file_real_path, 0, sizeof(file_real_path));
		if (linePos[strlen(linePos) - 1] == '\n'&&linePos[strlen(linePos) - 2] == '\r')
			linePos[strlen(linePos) - 2] = '\0';
		else if (linePos[strlen(linePos) - 1] == '\n')
			linePos[strlen(linePos) - 1] = '\0';
		memcpy(file_real_path, linePos + 3, strlen(linePos) - 3);//减去一个" = "


		if (file_real_path[strlen(file_real_path) - 1] == '/')//统一格式(如路径中结尾有'/'，则去掉，后面统一加上)
		{
			file_real_path[strlen(file_real_path) - 1] = '\0';
		}

		memset(tar_file_path, 0, sizeof(tar_file_path));
		sprintf(tar_file_path, "%s/%s", now_path, file_name);
		//printf("back path = %s\n", file_real_path);
		if (access(file_real_path, 0) != 0)
		{
			printf("err format configfile Path1 not found!\n");
			fclose(file);
			return BOOT_ERROR_OPER_PARSE_CONFIG_ERR_PATH_NOT_FOUND;
		}
		//return -4;
		memset(real_file_path, 0, sizeof(real_file_path));
		sprintf(real_file_path, "%s/%s", file_real_path, file_name);

		memset(backfile_path, 0, sizeof(backfile_path));
		sprintf(backfile_path, "%s.back", real_file_path);
		if (access(backfile_path, 0) == 0)//进行back备份的删除
		{
		
			memset(cmd, 0, sizeof(cmd));
			sprintf(cmd, "rm -rf %s", backfile_path);
			system(cmd);
			sync();
		}

		memset(lineBuff, 0, sizeof(lineBuff));
		continue;
	}
	return 0;
	fclose(file);


}

int mb_chmod_file_path_m(char *power, char *path)
{
	char cmd[1000] = { 0 };
	memset(cmd, 0, sizeof(cmd));
	sprintf(cmd, "chmod %s %s", power, path);
	system(cmd);
	sync();
	return 0;
}



int write_update_record_m(char *filename, char *writebuf)
{
	//char cmd[100] = { 0 };
	int fd = 0,len = 0;
	//int sleep_count = 0;
	//struct flock lock;
	char tmpfile[100] = { 0 };
	sprintf(tmpfile, "/tmp/update_tmp.log");
	if (filename == NULL || writebuf == NULL)
	{
		return -1;
	}
	if (strlen(writebuf) == 0)
	{
		//无所需要写入的更新文件信息内容
		return 0;
	}
	if (access(filename, 0) == 0)
	{
		mb_delete_file_dir_m(filename, 0);
	}
	if (access(tmpfile, 0) == 0)
	{
		mb_delete_file_dir_m(tmpfile,0);
		
	}
	fd = open(tmpfile, O_RDWR | O_CREAT);
	if (fd < 0)
		return -2;
	//独占方式打开文件
	/*lock.l_type = F_WRLOCK;
	fcntl(fd, F_SETLK,lock);*/
	len = write(fd, writebuf, strlen(writebuf));
	if (len != strlen(writebuf))
	{
		/*lock.l_type = F_UNLOCK;
		fcntl(fd, F_SETLK, lock);*/
		close(fd);
		return -3;
	}
	/*lock.l_type = F_UNLOCK;
	fcntl(fd, F_SETLK, lock);*/
	close(fd);
	mb_copy_file_path_m(tmpfile, filename);
	return 0;
}

static int get_s_info_new_m(struct	_s_info_new		*infor, char *buf)		//获取信息头
{
	
	char app_len[4] = { 0 };
	memcpy(infor->name, buf,20);
	memcpy(infor->type, buf + 20,20);
	memcpy(infor->app_ver, buf + 40, 20);
	memcpy(infor->app_md5, buf + 60, 16 );//20
	memcpy(app_len, buf + 80, 4);//20
	infor->app_len = (app_len[0] * 0x1000000) + (app_len[1] * 0x10000) + (app_len[2] * 0x100) + app_len[3];
	//printf("len is :%ld\n", infor->app_len);
	return 0;
}

int app_file_get_infor_new_m(const char *file, struct _s_info_new *infor)
{
	int fd;
	struct stat pro;
	long result;
	char buf[100];
	char outmd[16] = { 0 };
	MD5_CTX ctx;
	fd = open(file, O_RDWR);
	if (fd<0)
	{
		return fd;
	}
	memset(&pro, 0, sizeof(pro));
	stat(file, &pro);
	printf("pro.st_size  = %d\n", pro.st_size);
	if (pro.st_size<100)
	{
		close(fd);
		return -2;
	}
	lseek(fd, 0, SEEK_SET);
	memset(buf, 0, sizeof(buf));
	result = read(fd, buf, sizeof(buf));
	if (result != sizeof(buf))
	{
		close(fd);
		return -3;
	}
		
	//close(fd);
	if (get_s_info_new_m(infor, buf)<0)
	{
		close(fd);
		return -4;
	}
	infor->app_content = malloc(infor->app_len + 10);
	if (infor->app_content == NULL)
	{
		close(fd);
		return -5;
	}
	result = read(fd, infor->app_content, infor->app_len);
	if (result != infor->app_len)
	{
		close(fd);
		return -6;
	}

	MD5_Init(&ctx);
	MD5_Update(&ctx, infor->app_content, infor->app_len);
	MD5_Final((unsigned char*)outmd, &ctx);

	if (memcmp(outmd, infor->app_md5, 16) == 0)
	{
		close(fd);
		return 0;
	}
	free(infor->app_content);
	close(fd);
	return -6;
}


static int app_file_write_real_app_m(char *file, char *writebuf,int len)
{
	char real_name[30] = { 0 };
	//char cmd[300] = { 0 };
	int fd;
	sprintf(real_name, "%s_d", file);
	if (access(real_name, 0) == 0)
	{
	/*	sprintf(cmd, "rm -rf %s", real_name);
		system(cmd);
		sync();*/
		mb_delete_file_dir_m(real_name, 0);
	}
	fd = open(real_name, O_CREAT | O_RDWR);
	if (fd < 0)
		return -1;
	write(fd, writebuf, len);
	close(fd);

	//memset(cmd, 0, sizeof(cmd));
	//sprintf(cmd, "chmod 777  %s", real_name);
	//system(cmd);
	//sync();
	mb_chmod_file_path_m("777", real_name);
	return 0;

}

int mb_delete_file_dir_m(char *file_path, int file_type)
{
	char cmd[1000] = { 0 };
	if (file_type == 0)
	{
		//文件删除
		sprintf(cmd, "rm %s", file_path);
	}
	else
	{
		//文件夹删除
		sprintf(cmd, "rm -rf %s/", file_path);
	}
	system(cmd);
	sync();
	return 0;
}



/*********************************************************************
-   Function : http_get_token
-   Description：通过http获取服务器token用于之后的请求操作
-   Calls：
-   Calles By:
-   Table Accessed :
-   Table Update :
-   Input  :参数一:socketfd:TCP套接字
参数二:parm:http_parm结构体，内部存储获取到的服务器token值
-   Output :
-   Return :0 正常 <0失败
-   Other :
***********************************************************************/
int http_get_token_m(int socketfd, struct http_parm_m *parm)
{
	unsigned char s_buf[1024] = { 0 };
	unsigned char *r_buf = NULL;
	unsigned char parmdata[500] = { 0 };
	unsigned char time[100] = { 0 };
	struct tm stm;
	time_t tTime1;
	unsigned char shatime[100] = { 0 };
	int result;
	SHA_CTX s;
	cJSON *root = NULL, *item = NULL;
	char str_hash[100] = { 0 };
	unsigned char hash[20];
	// net_update_out("^^^^^^^^^^^^^^^^^^^22^^^^^^^^^^^11111^^^^^^^^^\n");
	result = http_get_server_time_m(socketfd, time,parm->ip,parm->port);//获取服务器时间
	if (result < 0)
	{
		printf("http_get_server_time error!\n");
		return -1;
	}
	//生成时间戳
	Boot_Strptime((char *)time, &stm);
	tTime1 = mktime(&stm);
	sprintf((char *)shatime, "%ld", tTime1);
	//进行SHA1的摘要并抓16进制字符串
	SHA1_Init(&s);
	SHA1_Update(&s, shatime, strlen((char*)shatime));
	SHA1_Final(hash, &s);
	HexToStr(str_hash, (char *)hash, 20);
	net_update_out("^^^^^^^^^^^^^^^^^^^22^^^^^^^^^^^222222^^^^^^^^\n");
	sprintf((char*)parmdata, "/oauth_token?version=1.0&grant_type=password&username=%s&password=%s", parm->ter_id, str_hash);
	printf("parmdata :%s\n", parmdata);
	memset(s_buf, 0, sizeof(s_buf));
	sprintf((char*)s_buf, "POST %s HTTP/1.1\r\n", (char *)parmdata);
	sprintf((char*)s_buf + strlen((char*)s_buf), "Content-Type: %s\r\n", HTTP_CONTENT_TYPE);
	sprintf((char*)s_buf + strlen((char*)s_buf), "Timeout: 5000\r\n");
	sprintf((char*)s_buf + strlen((char*)s_buf), "Host: %s:%d\r\n", parm->ip, parm->port);
	sprintf((char*)s_buf + strlen((char*)s_buf), "Content-Length: 0\r\n");
	sprintf((char*)s_buf + strlen((char*)s_buf), "User-Agent: libghttp/1.0\r\n");
	sprintf((char*)s_buf + strlen((char*)s_buf), "\r\n");
	printf("s_buf = %s\r\n",s_buf);			//wang
	net_update_out("^^^^^^^^^^^^^^^^^^^22^^^^^^^^^^^333333^^^^^^^^^\n");
	if (socket_write(socketfd, (unsigned char *)s_buf, strlen((char*)s_buf)) <= 0)
	{
		printf("发送失败\n");
		return -2;
	}
	result = deal_http_data_m(socketfd, (char **)&r_buf);
	if (result < 0)
	{
		printf("http get token err!\n");
		return -1;
	}
	root = cJSON_Parse((const char *)r_buf);
	if (root == NULL)
	{
		printf("err recv ,is not a json !\n");

		free(r_buf);
		return -1;
	}
	printf("get the recv token buf:%s\n", r_buf);

	net_update_out("^^^^^^^^^^^^^^^^^^^22^^^^^^^^^^^44444^^^^^^^^^\n");

	item = cJSON_GetObjectItem(root, "access_token");
	if (item == NULL)
	{
		printf("err recv token:json err2\n");

		free(r_buf);
		cJSON_Delete(root);
		return -1;
	}

	memset(parm->token, 0, sizeof(parm->token));
	memcpy((char *)parm->token, item->valuestring, strlen(item->valuestring));

	// net_update_out("^^^^^^^^^^^^^^^^^^^22^^^^^^^^^^^^^^^^^^^^\n");
	free(r_buf);
	cJSON_Delete(root);
	return 0;
}



/*********************************************************************
-   Function : http_get_server_time
-   Description：通过http获取服务器时间
-   Calls：
-   Calles By:
-   Table Accessed :
-   Table Update :
-   Input  :参数一:socketfd:TCP套接字
参数二:time:存储获取到的服务器时间
-   Output :
-   Return :0 正常 <0失败
-   Other :
***********************************************************************/
int http_get_server_time_m(int socketfd, unsigned char *time,char *ip,int port)
{
	int result;
	unsigned char s_buf[1024] = { 0 };
	unsigned char *r_buf = NULL;
	cJSON *root = NULL;
	cJSON *item = NULL;
	
	memset(s_buf, 0, sizeof(s_buf));
	sprintf((char*)s_buf, "POST %s HTTP/1.1\r\n", "/svr_time");
	sprintf((char*)s_buf + strlen((char*)s_buf), "Content-Type: %s\r\n", HTTP_CONTENT_TYPE);
	sprintf((char*)s_buf + strlen((char*)s_buf), "Timeout: 5000\r\n");
	sprintf((char*)s_buf + strlen((char*)s_buf), "Host: %s:%d\r\n", ip, port);
	sprintf((char *)s_buf + strlen((char*)s_buf), "Content-Length: 0\r\n");
	sprintf((char*)s_buf + strlen((char*)s_buf), "User-Agent: libghttp/1.0\r\n");
	sprintf((char*)s_buf + strlen((char*)s_buf), "\r\n");


	if (socket_write(socketfd, (unsigned char *)s_buf, strlen((char*)s_buf)) <=0 )
	{
		printf("http_get_server_time send err\n");
		result = -1;
		return result;
	}

	result = deal_http_data_m(socketfd, (char **)&r_buf);
	if (result < 0)
	{
		printf("http_get_server_time recv http error!\n");
		return -2;
	}
	root = cJSON_Parse((const char*)r_buf);
	if (root == NULL)
	{
		printf("err recv ,is not a json !\n");
		free(r_buf);
		return -1;
	}
	item = cJSON_GetObjectItem(root, "result");
	if (item == NULL)
	{
		printf("err recv time:json err1\n");
		free(r_buf);
		cJSON_Delete(root);
		return -1;
	}
	if (memcmp(item->valuestring, "s", 1) != 0)
	{
		printf("err recv :json result is f\n");
		free(r_buf);
		cJSON_Delete(root);
		return -1;
	}
	item = cJSON_GetObjectItem(root, "response");
	if (item == NULL)
	{
		printf("err recv time:json err2\n");
		free(r_buf);
		cJSON_Delete(root);
		return -1;
	}
	memcpy(time, item->valuestring, strlen(item->valuestring));
	free(r_buf);
	cJSON_Delete(root);
	return 0;
}

//																	"/tmp/app.tar" 		"/home/share/exe/tar/tmpapp" 
//int zwl_test_download(unsigned char *file, long file_len, unsigned char *filename, unsigned char * Dest_dir)
int zwl_test_download(unsigned char *filename, unsigned char * Dest_dir)
{
	int file_fd, result;
	char config_path[200] = { 0 };
	// mb_delete_file_dir_m((char *)filename,0);
	mb_delete_file_dir_m((char *)Dest_dir, 1);
	mb_delete_file_dir_m("/tmp/tmpapp/", 1);

	// file_fd = open((char *)filename, O_CREAT | O_RDWR);
	// if (file_fd < 0)
	// {
	// 	printf("err open file\n");
	// 	return -1;
	// }
	// result = write(file_fd, file, file_len);
	// if (result < 0)
	// {
	// 	printf("err write update file!\n");
	// 	close(file_fd);
	// 	return -1;
	// }

	if (access("/tmp/tmpapp", 0) != 0)
	{
		printf("create file 		/tmp/tmpapp 	111\r\n");
		mb_create_file_dir_m("/tmp/tmpapp");
	}

	system("unzip /tmp/app.zip -d /tmp/tmpapp"); //可自动生成 /tmp/tmpapp/ 目录；
	system("tar -cvf /tmp/app.tar /tmp/tmpapp");
	sleep(1);
	mb_chmod_file_path_m("777", "/tmp/app.tar");

	if (access((char *)Dest_dir, 0) != 0)
	{
		printf("create file 		/home/share/exe/tar/tmpapp 	222\r\n");
		mb_create_file_dir_m((char *)Dest_dir);
	}

	system("tar -vxf /tmp/app.tar  -C  /home/share/exe/tar");
	//mb_tar_decompress_m((char *)filename, (char *)Dest_dir); //    "/tmp/app.tar" 的压缩包 ，文件解压到这里"/home/share/exe/tar/tmpapp/boot_t" 

	if (access("/home/share/exe/tar/tmp/tmpapp/mb_config.cnf", 0) != 0)
	{
		printf("create file  /home/share/exe/tar/tmp/tmpapp 	error !!!\r\n");
		return -1;
	}
	else
	{
		printf("create file  /home/share/exe/tar/tmp/tmpapp 	successd !!!\r\n");
		system("cp /home/share/exe/tar/tmp/tmpapp/*  /home/share/exe/tar/tmpapp/");
		if (access("/home/share/exe/tar/tmpapp/mb_config.cnf", 0) == 0)
		{
			printf("create file  /home/share/exe/tar/tmpapp 	successd  end !!!\r\n");
		}
	}


	// close(file_fd);
	sleep(1);//等待解压完成
	//进行解压文件的校验和复制到当前系统
	sprintf((char*)config_path, "%s/mb_config.cnf", Dest_dir); 
	printf("____________Write_Decompress_file start\r\n");		
	result = parse_config_copy_file_m((unsigned char *)config_path, Dest_dir,2);//"/home/share/exe/tar/tmpapp"
	if (result < 0)
		return result;
	////没有问题 先复制该tar包到/tmpapp | /tmpauth目录下用于一会检测成功后替换原有的包
	printf("Write_Decompress_file start1\r\n");		
	mb_copy_file_path_m((char *)filename, (char *)Dest_dir);
	printf("Write_Decompress_file over\r\n");	
	return 0;
}



void *intranet_update_thread(void *arg)
{
	int  result;
	char errinfo[300] = { 0 };
	char temp_url[300] = { 0 };
	int flag_app = 0;//本次更新标识位 0 表示APP 和auth都无更新，1表示只有APP，2表示只有AUTH，3表示都有
	// sleep(30);
	//net_update_out("#############################################################################################\r\n");
	
	//获取下载url
	result = read_txt_data(temp_url);
	if (result != 0)
	{
		//net_update_out("not url data txt , No upgrade required \r\n");
		return;
	}
	else
	{
		net_update_out("temp_url= %s \r\n",temp_url);  
		system("rm /etc/need_update_url_app.txt");
		// break;  

	}

	//下载包
	int down_result = http_download_m(temp_url, "/tmp/app.zip");
	sleep(1);//等待下载完成
	// int down_result = http_download_m("www.baidu.com/", "/root/test.txt");
	if (down_result < 0)
	{
		net_update_out("_________not down data_________\r\n");
		return NULL;
	}
	
	if (access("/tmp/app.zip", 0) != 0)
	{
		net_update_out("______down load  error [OR]  not down data_________\r\n");
		return NULL;
	}
	else
	{
		net_update_out("______down load  successd	!!! _________\r\n");
	}

	net_update_out("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\r\n");

	//配置
	//result = peizhi_test(down_result);
	result = zwl_test_download("/tmp/app.tar" , "/home/share/exe/tar/tmpapp");
	if(result < 0)
	{
		net_update_out("______pei zhi  error_________result = %d\r\n",result);
		return NULL;
	}
	net_update_out("rev data ___________result= [%d]\r\n",result);

	//检测包
	result = dectec_app(errinfo);
	net_update_out("rev data ___________result= [%d]\r\n",result);

	if (result == 1)
	{
		//本次没有需要更新的app内容
		flag_app = 0;
	}
	else 
		flag_app = 1;

	net_update_out("-----------------------------APP-------------------------------	flag_app = %d	\n",flag_app);

	net_update_out("88888888888888888888888888888888888888888888888\n");
	if (result < 0)//检测失败，进行本次更新回退
	{
		net_update_out("err app_file_detect_m!\n");
		parse_config_back_oldver("/home/share/exe/tar/tmpapp/mb_config.cnf", "/home/share/exe/tar/tmpapp");
		parse_config_back_oldver("/home/share/exe/tar/tmpauth/mb_config.cnf", "/home/share/exe/tar/tmpauth");

		// parse_errcode_upload(sock, parm, -402);
		// shutdown(sock, SHUT_RDWR);
		// close(sock);
		goto WAIT_NEXT;
	}
	else//删除.back文件
	{
		parse_config_del_backfile("/home/share/exe/tar/tmpapp/mb_config.cnf", "/home/share/exe/tar/tmpapp");
		parse_config_del_backfile("/home/share/exe/tar/tmpauth/mb_config.cnf", "/home/share/exe/tar/tmpauth");
	}
	net_update_out("int end!!!\n");

	// while(1);

	// memset(cmd, 0, sizeof(cmd));
	//没有问题 从tmpapp复制该tar包到exe/tar目录下用于开机校验用
	net_update_out("mb_copy_file_path start\r\n");			//wang
	if (flag_app == 1 || flag_app == 3)
	{
		net_update_out("******************************************************111\r\n");
		if (access("/home/share/exe/tar/tmpapp/app.tar", 0) == 0)
		{
			mb_copy_file_path_m("/home/share/exe/tar/tmpapp/app.tar", "/home/share/exe/tar");
			net_update_out("******************************************************222\r\n");
		}
	}
	net_update_out("mb_copy_file_path over\r\n");			//wang
	//没有问题 从tmpauth目录复制该tar包到exe/tar目录下用于开机校验用
	if (flag_app == 2 || flag_app == 3)
	{
		if (access("/home/share/exe/tar/tmpauth/auth.tar", 0) == 0)
			mb_copy_file_path_m("/home/share/exe/tar/tmpauth/auth.tar", "/home/share/exe/tar");
	}


	net_update_out("mb_copy_file_path1 over   end  end  end\r\n");		


	
	//此处进行有更新且已经正常安装的重启主程序处理:写一个更新文件给主程序响应	
	if (strlen((char *)update_msg_m) != 0)
	{
		net_update_out("update_msg_m:%s\n", update_msg_m);
		result = write_update_record_m("/etc/update_app_mb_m.log", (char*)update_msg_m);
		net_update_out("******************************************************333   result= %d\r\n",result);
		if (result < 0)
		{
			net_update_out("err write_update_record_m:%d\n", result);
		}
	}
	if (strlen((char *)auth_msg_m) != 0)
	{
		net_update_out("******************************************************444   \r\n");
		result = write_update_record_m("/etc/update_auth_mb.log", (char*)auth_msg_m);
		if (result < 0)
		{
			net_update_out("err write_update_record_m:%d\n", result);
		}
	}
WAIT_NEXT:
	net_update_out("******************************************************555   \r\n");
	sleep_hour(1);//此处等待1小时
	// sleep(60);

	return NULL;
}

int read_txt_data(char *temp_url)
{
	if (access("/etc/need_update_url_app.txt", 0) == 0)
	{
		FILE *file = fopen("/etc/need_update_url_app.txt", "r");                 
		if (file == NULL) {                                   
			perror("Error opening file");                     
			return 1;                                         
		}                                                     
		fseek(file, 0, SEEK_END);                             
		long file_size = ftell(file);                         
		rewind(file);                                         
		// char *temp_url = (char *)malloc(file_size);           
		fread(temp_url, 1, file_size, file);                  
		fclose(file);                                         
															
		printf("______________________________________\r\n"); 
																				
		return 0;     
	}
	else
		return 1;
                                        
}          


