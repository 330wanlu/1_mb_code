#include "../inc/http_reg.h"

static int open_rec_over_timer(int socketfd, int over_timer);
static int open_send_over_timer(int socketfd, int over_timer);
static int get_line(int sock, char *buf, int size);
static int deal_http_data(int sockfd, char **outbuf);

/*********************************************************************
-   Function : Boot_Strptime
-   Description：按格式化解析存储时间  格式为YYYY-MM-DD HH:MM:SS
-   Calls：
-   Calles By:
-   Table Accessed :
-   Table Update :
-   Input  :参数一:szStandTimeIn:待转换的时间字符串
参数二:tt0:保存转换后的时间日期结构体
-   Output :
-   Return :0
-   Other :
***********************************************************************/
int Boot_Strptime(char *szStandTimeIn, struct tm *tt0)
{
	unsigned int nY = 0, nM = 0, nD = 0, nH = 0, nMn = 0, nS = 0;
	//2019-12-23 11:58:02
	sscanf((char *)szStandTimeIn, "%04d-%02d-%02d %02d:%02d:%02d", &nY, &nM, &nD, &nH, &nMn, &nS);

	memset(tt0, 0, sizeof(struct tm));  //!!!特别注意，tt0最好赋初值，否则某些量容易出错
	tt0->tm_sec = nS;
	tt0->tm_min = nMn;
	tt0->tm_hour = nH;
	tt0->tm_mday = nD;
	tt0->tm_mon = nM - 1;
	tt0->tm_year = nY - 1900;
	return 0;
}

/*********************************************************************
-   Function : get_line
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
static int get_line(int sock, char *buf, int size)
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
				//http_reg_out("\n但如果是换行符则把它吸收掉\n");
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

int http_socket_open(int *socket_t, struct http_parm * parm)
{
	int sock;
	struct sockaddr_in add;
	int result;
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0)
	{
		printf("socket create fail!\n");
		return -1;
	}
	memset(&add, 0, sizeof(add));
	add.sin_family = AF_INET;
	add.sin_port = htons(HTTP_PORT);
	add.sin_addr.s_addr = htonl(INADDR_ANY);
	inet_pton(AF_INET, parm->ip, &add.sin_addr);


	if ((result = connect(sock, (struct sockaddr *)&add, sizeof(add))) == -1)
	{
		printf("connect server err %s:%d\n", parm->ip, HTTP_PORT);
		shutdown(sock, SHUT_RDWR);
		close(sock);
		return -2;
	}

	open_rec_over_timer(sock, HTTP_TIMEOUT);
	open_send_over_timer(sock, HTTP_TIMEOUT);
	*socket_t = sock;
	return sock;
}


int http_send_sign_enc(int socketfd, struct http_parm *parm)
{
	unsigned char s_buf[1024*6] = { 0 };
	char *outdata = NULL;
	char r_outdata[1024 * 5] = { 0 };
	int outlen;
	unsigned char *r_buf = NULL;
	unsigned char parmdata[8000] = { 0 };
	unsigned char sort_sign[5000] = { 0 };
	unsigned char md5_sha_sign[100] = { 0 };
	unsigned char hash[100] = { 0 };
	unsigned char outmd[16] = { 0 };
	unsigned char outmd_up[50] = { 0 };
	int size,i;
	char *g_buf = NULL;
	char *p = NULL;
	int  result = 0;
	cJSON *root = NULL, *recv_root = NULL, *item = NULL, *item_svrpub = NULL, *item_terpiv = NULL, *s_json = NULL, *arrayItem = NULL, *svrpub_array = NULL, *terpiv_array = NULL, *dir3_s = NULL,*dir3_t = NULL;
	//	unsigned char testmd5[20] = { 0 };
	MD5_CTX ctx;
	SHA_CTX s;
	//进行url的首字母排序和body组合在一起
	//printf("mb_enc_singn_chect_function start \n");
	result = mb_enc_singn_chect_function(parm->key_s, &outdata, &outlen);
	if (result < 0)
		return -1;
	//printf("end :recv outdate :%s\n\n\n\n\n", outdata);
	root = cJSON_Parse(outdata);
	if (root == NULL)
	{
		free(outdata);
		printf("err1\n");
		return -2;
	}
	s_json = cJSON_CreateObject();

	cJSON_AddStringToObject(s_json, "ter_id", (char *)parm->ter_id);
	cJSON_AddStringToObject(s_json, "cpu_id", (char *)parm->cpu_id);

	cJSON_AddItemToObject(s_json, "svrpub", svrpub_array = cJSON_CreateArray());
	cJSON_AddItemToObject(s_json, "terpiv", terpiv_array = cJSON_CreateArray());
		
	item_svrpub = cJSON_GetObjectItem(root, "svrpub");
	if (item_svrpub == NULL)
	{
		cJSON_Delete(root);
		free(outdata);
		printf("err2\n");
		return -3;
	}
		

	size = cJSON_GetArraySize(item_svrpub);
	if (size == 0)
	{
		
		cJSON_Delete(root);
		free(outdata);
		printf("err3\n");
		return -3;
	}
	sprintf(r_outdata, "\"svrpub\":[{");
	for (i = 0; i < size; i++)
	{
		arrayItem = cJSON_GetArrayItem(item_svrpub, i);
		if (arrayItem == NULL)
		{
			cJSON_Delete(root);
			free(outdata);
			return -3;
		}
		
		item = cJSON_GetObjectItem(arrayItem, "key_num_s");
		if (item == NULL)
		{
			cJSON_Delete(root);
			free(outdata);
			return -3;
		}
		cJSON_AddItemToObject(svrpub_array, "dira", dir3_s = cJSON_CreateObject());
		cJSON_AddStringToObject(dir3_s, "key_num_s", item->valuestring);
		sprintf(r_outdata + strlen(r_outdata), "\"key_num_s\":\"%s\",",item->valuestring);
		item = cJSON_GetObjectItem(arrayItem, "checkdata");
		if (item == NULL)
		{
			cJSON_Delete(root);
			free(outdata);
			return -3;
		}

		cJSON_AddStringToObject(dir3_s, "checkdata", item->valuestring);
		sprintf(r_outdata + strlen(r_outdata), "\"checkdata\":\"%s\",", item->valuestring);

		item = cJSON_GetObjectItem(arrayItem, "encdata");
		if (item == NULL)
		{
			cJSON_Delete(root);
			free(outdata);
			return -3;
		}
		cJSON_AddStringToObject(dir3_s, "encdata", item->valuestring);
		sprintf(r_outdata + strlen(r_outdata), "\"encdata\":\"%s\"", item->valuestring);
		if (i < size -1)
			sprintf(r_outdata + strlen(r_outdata), "},{");
		else
			sprintf(r_outdata + strlen(r_outdata), "}],");
	}

	sprintf(r_outdata + strlen(r_outdata), "\"terpiv\":[{");
	item_terpiv = cJSON_GetObjectItem(root, "terpiv");
	if (item_svrpub == NULL)
	{
		cJSON_Delete(root);
		free(outdata);
		return -3;
	}
	size = cJSON_GetArraySize(item_terpiv);
	if (size == 0)
	{

		cJSON_Delete(root);
		free(outdata);
		return -3;
	}
	for (i = 0; i < size; i++)
	{
		arrayItem = cJSON_GetArrayItem(item_terpiv, i);
		if (arrayItem == NULL)
		{
			cJSON_Delete(root);
			free(outdata);
			return -3;
		}
		item = cJSON_GetObjectItem(arrayItem, "key_num_t");
		if (item == NULL)
		{
			cJSON_Delete(root);
			free(outdata);
			return -3;
		}
		cJSON_AddItemToObject(terpiv_array, "dirb", dir3_t = cJSON_CreateObject());
		cJSON_AddStringToObject(dir3_t, "key_num_t", item->valuestring);
		sprintf(r_outdata + strlen(r_outdata), "\"key_num_t\":\"%s\",", item->valuestring);
		item = cJSON_GetObjectItem(arrayItem, "checkdata");
		if (item == NULL)
		{
			cJSON_Delete(root);
			free(outdata);
			return -3;
		}
		cJSON_AddStringToObject(dir3_t, "checkdata", item->valuestring);
		sprintf(r_outdata + strlen(r_outdata), "\"checkdata\":\"%s\",", item->valuestring);

		item = cJSON_GetObjectItem(arrayItem, "tersign");
		if (item == NULL)
		{
			cJSON_Delete(root);
			free(outdata);
			return -3;
		}
		cJSON_AddStringToObject(dir3_t, "tersign", item->valuestring);
		sprintf(r_outdata + strlen(r_outdata), "\"tersign\":\"%s\"", item->valuestring);
		if (i < size - 1)
			sprintf(r_outdata + strlen(r_outdata), "},{");
		else
			sprintf(r_outdata + strlen(r_outdata), "}]}");
	}
	//printf("r_outdata###########################################:%s\n\n\n\n\n", r_outdata);
	

	

	g_buf = cJSON_Print(s_json);
	if (g_buf == NULL)
	{
		free(outdata);
		return -5;
	}
	//printf("g_buf###########################################:%s\n\n\n\n\n", g_buf);
	cJSON_Delete(root);

	sprintf((char *)sort_sign, "appid=%s&format=json&timestamp=%s&token=%s&version=1.0{\"ter_id\":\"%s\",\"cpu_id\":\"%s\",%s", (char *)parm->appid, (char *)parm->timestamp, (char *)parm->token,parm->ter_id,parm->cpu_id,r_outdata);

	//printf("sort buf is %s\n\n\n\n\n\n", sort_sign);
	//"{\"ter_id\": \"%s\",\"cpu_id\": \"%s\",\"svrpub\": [{\"key_num_s\": \"%s\",\"checkdata\": \"%s\",\"encdata\": \"%s\"}],\"terpiv\": [{\"key_num_t\": \"%s\",\"checkdata\": \"%s\",\"tersign\": \"%s\"}]}"
	//printf("sort_sign:%s\n", sort_sign);
	MD5_Init(&ctx);
	MD5_Update(&ctx, sort_sign, strlen((const char *)sort_sign));
	MD5_Final(outmd, &ctx);
	//MD5加密完成
	//MD5加密结果进行转16进制字符串再转大写
	HexToStr((unsigned char *)outmd_up, (unsigned char *)outmd, 16);
	p = str2upper((char *)outmd_up);

	//SHA1加密
	SHA1_Init(&s);
	SHA1_Update(&s, p, strlen(p));
	SHA1_Final(hash, &s);
	HexToStr(md5_sha_sign,hash, 20);
	//sha1加密完成（转成16进制字符串)


	sprintf((char*)parmdata, "/was.mbienc/regenccheck?version=1.0&appid=%s&timestamp=%s&sign=%s&token=%s&format=json", (char *)parm->appid, (char *)parm->timestamp, md5_sha_sign, (char *)parm->token);
	
	
	memset(s_buf, 0, sizeof(s_buf));
	//	memset(r_buf, 0, sizeof(r_buf));
	sprintf((char*)s_buf, "POST %s HTTP/1.1\r\n", (char *)parmdata);
	sprintf((char*)s_buf + strlen((char*)s_buf), "Content-Type: %s\r\n", HTTP_CONTENT_TYPE);
	sprintf((char*)s_buf + strlen((char*)s_buf), "Timeout: 5000\r\n");
	sprintf((char*)s_buf + strlen((char*)s_buf), "Host: %s:%d\r\n", parm->ip, HTTP_PORT);

	sprintf((char*)s_buf + strlen((char*)s_buf), "Content-Length: %d\r\n", strlen(g_buf) + 2);
	sprintf((char*)s_buf + strlen((char*)s_buf), "User-Agent: libghttp/1.0\r\n");
	sprintf((char*)s_buf + strlen((char*)s_buf), "\r\n");
	sprintf((char*)s_buf + strlen((char*)s_buf), "%s\r\n", g_buf);

	cJSON_Delete(s_json);
	free(g_buf);

	//printf("send http buf is %s\n\n\n\n\n\n", s_buf);
	if (socket_write(socketfd, (unsigned char *)s_buf, strlen((char*)s_buf)) < 0)
	{
		printf("http_send_sign_enc send failed\n");
		free(outdata);
		return  -1;
	}
	
	result = deal_http_data(socketfd, (char **)&r_buf);
	if (result < 0)
	{
		printf("http recv err!\n");
		free(outdata);
		return -1;
	}
	//printf("recv http buf is %s\n\n\n\n\n\n", r_buf);
	recv_root = cJSON_Parse((const char *)r_buf);
	if (root == NULL)
	{
		free(r_buf);
		free(outdata);
		return -1;
	}
	item = cJSON_GetObjectItem(recv_root, "result");
	if (item == NULL)
	{

		free(r_buf);
		cJSON_Delete(recv_root);
		free(outdata);
		return -1;
	}
	if (memcmp(item->valuestring, "s", 1) != 0)
	{

		free(r_buf);
		cJSON_Delete(recv_root);
		free(outdata);
		return -1;
	}
	free(outdata);
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
int http_get_token(int socketfd, struct http_parm *parm)
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
	unsigned char str_hash[100] = { 0 };
	unsigned char hash[20];
	result = http_get_server_time(socketfd, time,parm->ip);//获取服务器时间
	if (result < 0)
	{
		printf("http_get_server_time error!\n");
		return -1;
	}
	//生成时间戳
	Boot_Strptime((char *)time, &stm);
	tTime1 = mktime(&stm);
#ifdef MTK_OPENWRT
	sprintf((char *)shatime, "%ld", tTime1+28800);//加上8小时时差
#else
	//sprintf((char *)shatime, "%ld", tTime1 + 28800);
	sprintf((char *)shatime, "%ld", tTime1);
#endif
	//进行SHA1的摘要并抓16进制字符串
	SHA1_Init(&s);
	SHA1_Update(&s, shatime, strlen((char*)shatime));
	SHA1_Final(hash, &s);
	HexToStr((unsigned char*)str_hash, hash, 20);
	//int i;
	/*printf("passwd hex\n");
	for (i = 0; i < 20; i++)
	{
		printf("%02X", hash[i]);
	}
	printf("\n\n\n");*/

	sprintf((char*)parmdata, "/oauth_token?version=1.0&grant_type=password&username=%s&password=%s", parm->ter_id, str_hash);
	printf("passwd :%s\n", str_hash);
	memset(s_buf, 0, sizeof(s_buf));
	//memset(r_buf, 0, sizeof(r_buf));
	sprintf((char*)s_buf, "POST %s HTTP/1.1\r\n", (char *)parmdata);
	sprintf((char*)s_buf + strlen((char*)s_buf), "Content-Type: %s\r\n", HTTP_CONTENT_TYPE);
	sprintf((char*)s_buf + strlen((char*)s_buf), "Timeout: 5000\r\n");
	sprintf((char*)s_buf + strlen((char*)s_buf), "Host: %s:%d\r\n", parm->ip, HTTP_PORT);
	sprintf((char*)s_buf + strlen((char*)s_buf), "Content-Length: 0\r\n");
	sprintf((char*)s_buf + strlen((char*)s_buf), "User-Agent: libghttp/1.0\r\n");
	sprintf((char*)s_buf + strlen((char*)s_buf), "\r\n");
	if (socket_write(socketfd, (unsigned char *)s_buf, strlen((char*)s_buf)) < 0)
	{
		printf("http_get_token send failed\n");
		return -2;
	}
	result = deal_http_data(socketfd, (char **)&r_buf);
	if (result < 0)
	{
		printf("http get token err!\n");
		return -1;
	}
	printf("\nrecv token whole pack is :\n%s\n", r_buf);
	root = cJSON_Parse((const char *)r_buf);
	if (root == NULL)
	{
		printf("err recv ,is not a json !\n");

		free(r_buf);
		return -1;
	}



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
int http_get_server_time(int socketfd, unsigned char *time,char *ip)
{
	int result;
	unsigned char s_buf[1024] = { 0 };
	unsigned char *r_buf = NULL;
	cJSON *root = NULL;
	cJSON *item = NULL;

	memset(s_buf, 0, sizeof(s_buf));
	//memset(r_buf, 0, sizeof(r_buf));
	sprintf((char*)s_buf, "POST %s HTTP/1.1\r\n", "/svr_time");
	sprintf((char*)s_buf + strlen((char*)s_buf), "Content-Type: %s\r\n", HTTP_CONTENT_TYPE);
	sprintf((char*)s_buf + strlen((char*)s_buf), "Timeout: 5000\r\n");
	sprintf((char*)s_buf + strlen((char*)s_buf), "Host: %s:%d\r\n", ip, HTTP_PORT);
	sprintf((char *)s_buf + strlen((char*)s_buf), "Content-Length: 0\r\n");
	sprintf((char*)s_buf + strlen((char*)s_buf), "User-Agent: libghttp/1.0\r\n");
	sprintf((char*)s_buf + strlen((char*)s_buf), "\r\n");
	//sprintf(s_buf + strlen(s_buf), "%s\r\n", parmdata);


	if (socket_write(socketfd, (unsigned char *)s_buf, strlen((char*)s_buf))<0)
	{
		printf("http_get_server_time send err\n");
		result = -1;
	}

	result = deal_http_data(socketfd, (char **)&r_buf);
	if (result < 0)
	{
		printf("http_get_server_time recv http error!\n");
		return -2;
	}
	printf("recv data is :%s\n", r_buf);
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
	printf("get server time is :%s\n", time);
	free(r_buf);
	cJSON_Delete(root);
	return 0;
}
/*********************************************************************
-   Function : open_rec_over_timer
-   Description：设置TCP 接收超时时间
-   Calls：
-   Calles By:
-   Table Accessed :
-   Table Update :
-   Input  :参数一:socketfd:TCP套接字
参数二:over_timer:超时时间（单位:S）
-   Output :
-   Return :0 正常 非零 TCP errno
-   Other :
***********************************************************************/
static int open_rec_over_timer(int socketfd, int over_timer)
{
	struct timeval timeout;
	memset(&timeout, 0, sizeof(timeout));
	timeout.tv_sec = over_timer;
	return setsockopt(socketfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
}

/*********************************************************************
-   Function : open_send_over_timer
-   Description：设置TCP 发送超时时间
-   Calls：
-   Calles By:
-   Table Accessed :
-   Table Update :
-   Input  :参数一:socketfd:TCP套接字
参数二:over_timer:超时时间（单位:S）
-   Output :
-   Return :0 正常 非零 TCP errno
-   Other :
***********************************************************************/
static int open_send_over_timer(int socketfd, int over_timer)
{
	struct timeval timeout;
	memset(&timeout, 0, sizeof(timeout));
	timeout.tv_sec = over_timer;
	return setsockopt(socketfd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));

}

/*********************************************************************
-   Function : socket_write
-   Description：TCP数据发送
-   Calls：
-   Calles By:
-   Table Accessed :
-   Table Update :
-   Input  :参数一:sock:TCP套接字
参数二:buf:发送内容
参数三:buf_len:发送内容长度
-   Output :
-   Return :0 正常 非零 write errno
-   Other :
***********************************************************************/
int socket_write(int sock, unsigned char *buf, int buf_len)
{
	int i, result;
	for (i = 0; i<buf_len; i += result)
	{
		result = write(sock, buf + i, buf_len - i);
		if (result <= 0)
			return result;
	}
	return i;
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
static int deal_http_data(int sockfd, char **outbuf)
{
	char buf[1024];
	int i;
	//int j;
	char c;
	//char num=3;
	char len_buf[50];
	int numchars = 1;
	int content_length = -1;
	//http_reg_out("获取第一行数据\n");
	memset(buf, 0, sizeof(buf));
	numchars = get_line(sockfd, buf, sizeof(buf));
	//http_reg_out("获取到的第一行数据为：%s\n",buf);
	memset(buf, 0, sizeof(buf));
	numchars = get_line(sockfd, buf, sizeof(buf));
	//http_reg_out("获取到的第二行数据为：%s\n",buf);
	while ((numchars > 0) && strcmp("\n", buf))
	{
		if (strncasecmp(buf, "Content-Length:", 15) == 0)
		{
			memset(len_buf, 0, sizeof(len_buf));
			memcpy(len_buf, &buf[15], 20);
			//printf("len_buf = %s",len_buf);
			cls_no_asc(len_buf, strlen((char*)len_buf));
			content_length = atoi(len_buf);
			//printf("Content-Length = %d\n",content_length);
		}
		memset(buf, 0, sizeof(buf));
		numchars = get_line(sockfd, buf, sizeof(buf));
		//http_reg_out("获取到的第%d行数据为：%s",num,buf);
		//num +=1;
		//for(j=0;j<numchars;j++)
		//{
		//	printf("%02x ",(unsigned char)buf[j]);
		//}
		//printf("\n");
	}
	/*没有找到 content_length */
	//http_reg_out("退出获取头循环\n");
	if (content_length <= 0) {
		printf("error recv,length err !content_length = %d\n", content_length);
		return -1;
	}
	/*接收 POST 过来的数据*/
	//if(content_length>102400)
	//{
	//	http_reg_out("超过缓存大小,无法接收\n");
	//	return -1;
	//}
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
	//http_reg_out("HTTP data :%s\n",tmp);
	*outbuf = tmp;
	//memcpy(outbuf, tmp, content_length);
	//memcpy(outbuf,tmp,sizeof(tmp));
	//http_reg_out("TRAN return data :%s\n",*outbuf);
	return content_length;
}


int http_get_appid_secert_token(struct _http_comon *parm)
{
	int result;
	char *returndata=NULL;
	int returndata_len;
	uint8 s_data[100] = {0};
	char request[500] = { 0 };
	cJSON *root = NULL, *item = NULL;

	http_reg_out("enter http_get_appid_secert_token,parm->s_ip=%s,parm->s_port=%d\n",parm->s_ip,parm->s_port);

	//sprintf((char*)request, "/oauth_token?version=1.0&grant_type=password&username=%s&password=%s", appid, appsecert);
	sprintf((char*)request, "/was.oauth/oauth2?version=%s&grant_type=%s&appid=%s&appsecret=%s", "2.0.1.0", "client_credentials", parm->appid, parm->appsecret);
	result = socket_http_netPost(parm->s_ip, parm->s_port, request, "application/x-www-form-urlencoded", (char *)s_data, 5, &returndata, &returndata_len);
    http_reg_out("exit http_get_appid_secert_token\n");
	if (result < 0)
	{
		if (returndata != NULL)
		{
			http_reg_out("释放http接收数据内存\n");
			free(returndata);
			returndata = NULL;
			http_reg_out("释放内存成功,指向空\n");
		}
		http_reg_out("获取token失败,http post error,result = %d\n",result);
		return -1;
	}
	http_reg_out("http_get_appid_secert_token  parm->s_ip = %s,parm->s_port=%d ,request = %s\n", parm->s_ip,parm->s_port,request);
	http_reg_out("token获取到的是数据为%s\n", returndata);

	
	char res_result[10] = { 0 };
	root = cJSON_Parse((const char *)returndata);
	if (root == NULL)
	{
		printf("err recv ,is not a json !\n");
		free(returndata);
		return -2;
	}
	free(returndata);
	item = cJSON_GetObjectItem(root, "result");
	if (item == NULL)
	{
		printf("err recv token:json err2\n");
		cJSON_Delete(root);
		return -3;
	}
	memcpy(res_result, item->valuestring, strlen(item->valuestring));
	printf("http_get_appid_secert_token = %s\r\n",res_result);
	if (strcmp(res_result, "s") != 0)
	{
		char err[100] = { 0 };
		char msg[1000] = { 0 };
		char errinfo[1000] = { 0 };
		http_reg_out("服务器应答失败,解析错误描述\n");
		cJSON *object = cJSON_GetObjectItem(root, "response");
		item = cJSON_GetObjectItem(object, "err");
		if (get_json_value_can_not_null(item, err, 1, sizeof(err)) < 0)
		{
			printf("err recv token:json err3\n");
			cJSON_Delete(root);
			return -4;
		}
		item = cJSON_GetObjectItem(object, "msg");
		if (get_json_value_can_not_null(item, msg, 1, sizeof(msg)) < 0)
		{
			printf("err recv token:json err4\n");
			cJSON_Delete(root);
			return -5;
		}
		item = cJSON_GetObjectItem(object, "errinfo");
		if (get_json_value_can_not_null(item, errinfo, 1, sizeof(errinfo)) < 0)
		{
			printf("err recv token:json err5\n");
			cJSON_Delete(root);
			return -6;
		}
		http_reg_out(" 服务器应答错误：\n");
		printf("err = %s,msg = %s,errinfo = %s\n", err, msg, errinfo);
	}
	else
	{
		cJSON *object = cJSON_GetObjectItem(root, "response");
		item = cJSON_GetObjectItem(object, "access_token");
		if (get_json_value_can_not_null(item, parm->token, 1, sizeof(parm->token)) < 0)
		{
			printf("err recv token:json err6\n");
			cJSON_Delete(root);
			return -7;
		}
	}

	cJSON_Delete(root);
	return 0;
}

int http_token_register_dev_to_server(struct _http_comon *parm,char *s_data,int s_len,char *err_msg)
{
	http_reg_out("lbc   enter   http_token_register_dev_to_server parm->s_ip = %s, parm->s_port=%d\n",parm->s_ip, parm->s_port);
	int result;
	char *returndata = NULL;
	int returndata_len;
	char request[500] = { 0 };
	cJSON *root = NULL, *item = NULL;
	char *sign_data;
	MD5_CTX ctx;
	char *p = NULL;
	char md5_sha_sign[100] = { 0 };
	unsigned char outmd[16] = { 0 };
	unsigned char outmd_up[50] = { 0 };

	result = http_get_appid_secert_token(parm);
	if (result < 0)
	{
		http_reg_out("获取token失败\n");
		return -1;
	}

	sign_data = calloc(1,s_len+1000);

	sprintf(sign_data, "access_token=%s&appid=%s&timestamp=%s%s", parm->token, parm->appid, parm->timestamp, s_data);

	MD5_Init(&ctx);
	MD5_Update(&ctx, sign_data, strlen((const char *)sign_data));
	MD5_Final((uint8 *)outmd, &ctx);
	//MD5加密完成
	//MD5加密结果进行转16进制字符串再转大写
	HexToStr(outmd_up,outmd, 16);
	p = str2upper((char *)outmd_up);
	sprintf(md5_sha_sign, "%s", p);
	free(sign_data);

	sprintf((char*)request, "/was.tersvr/ter_regist?appid=%s&timestamp=%s&sign=%s&access_token=%s", parm->appid, parm->timestamp, md5_sha_sign, parm->token);
	result = socket_http_netPost(parm->s_ip, parm->s_port, request, "application/x-www-form-urlencoded", (char *)s_data, 5, &returndata, &returndata_len);
	if (result < 0)
	{
		if (returndata != NULL)
		{
			http_reg_out("释放http接收数据内存\n");
			free(returndata);
			returndata = NULL;
			http_reg_out("释放内存成功,指向空\n");
		}
		http_reg_out("获取token失败\n");
		return -1;
	}
	http_reg_out("token获取到的是数据为%s\n", returndata);
	char res_result[10] = {0};
	root = cJSON_Parse((const char *)returndata);
	if (root == NULL)
	{
		printf("err recv ,is not a json !\n");
		free(returndata);
		return -1;
	}
	free(returndata);
	item = cJSON_GetObjectItem(root, "result");
	if (item == NULL)
	{
		printf("http_token_register_dev_to_server:json err2\n");
		cJSON_Delete(root);
		return -1;
	}
	printf("http_token_register_dev_to_server = %s\r\n",res_result);
	memcpy(res_result, item->valuestring, strlen(item->valuestring));
	if (strcmp(res_result, "s") != 0)
	{
		char err[100] = {0};
		char msg[1000] = { 0 };
		char errinfo[1000] = {0};
		http_reg_out("服务器应答失败,解析错误描述\n");
		cJSON *object = cJSON_GetObjectItem(root, "response");
		item = cJSON_GetObjectItem(object, "err");
		if (get_json_value_can_not_null(item, err, 1, sizeof(err)) < 0)
		{
			printf("http_token_register_dev_to_server:json err3\n");
			cJSON_Delete(root);
			return -1;
		}
		item = cJSON_GetObjectItem(object, "msg");
		if (get_json_value_can_not_null(item, msg, 1, sizeof(msg)) < 0)
		{
			printf("http_token_register_dev_to_server:json err4\n");
			cJSON_Delete(root);
			return -1;
		}
		item = cJSON_GetObjectItem(object, "errinfo");
		if (get_json_value_can_not_null(item, errinfo, 1, sizeof(errinfo)) < 0)
		{
			printf("http_token_register_dev_to_server:json err5\n");
			cJSON_Delete(root);
			return -1;
		}
		http_reg_out(" 服务器应答错误：");
		printf("err = %s,msg = %s,errinfo = %s\n",err,msg,errinfo);
		if (err_msg != NULL)
		{
			memcpy(err_msg, errinfo, strlen(errinfo));
		}
		cJSON_Delete(root);
		return -1;
	}
	cJSON_Delete(root);
	return 0;
}

int http_token_get_mqtt_server_info(struct _http_comon *parm, char *s_data, int s_len,char *r_data,int *r_len)
{
	int result;
	char *returndata = NULL;
	int returndata_len;
	char request[500] = { 0 };
	cJSON *root = NULL, *item = NULL;
	char *sign_data;
	MD5_CTX ctx;
	char *p = NULL;
	char md5_sha_sign[100] = { 0 };
	unsigned char outmd[16] = { 0 };
	unsigned char outmd_up[50] = { 0 };

	result = http_get_appid_secert_token(parm);
	if (result < 0)
	{
		http_reg_out("获取token失败,result = %d\n",result);
		return -1;
	}

	sign_data = calloc(1, s_len + 1000);

	sprintf(sign_data, "access_token=%s&appid=%s&timestamp=%s%s", parm->token, parm->appid, parm->timestamp, s_data);

	MD5_Init(&ctx);
	MD5_Update(&ctx, sign_data, strlen((const char *)sign_data));
	MD5_Final((uint8 *)outmd, &ctx);
	//MD5加密完成
	//MD5加密结果进行转16进制字符串再转大写
	HexToStr(outmd_up,outmd, 16);
	p = str2upper((char *)outmd_up);
	sprintf(md5_sha_sign, "%s", p);
	free(sign_data);

	sprintf((char*)request, "/was.tersvr/mqttsvr_by_terid?appid=%s&timestamp=%s&sign=%s&access_token=%s", parm->appid, parm->timestamp, md5_sha_sign, parm->token);
	result = socket_http_netPost(parm->s_ip, parm->s_port, request, "application/x-www-form-urlencoded", (char *)s_data, 5, &returndata, &returndata_len);
	if (result < 0)
	{
		if (returndata != NULL)
		{
			http_reg_out("释放http接收数据内存\n");
			free(returndata);
			returndata = NULL;
			http_reg_out("释放内存成功,指向空\n");
		}
		http_reg_out("获取token失败\n");
		return -1;
	}
	//http_reg_out("获取到的是数据为%s\n", returndata);
	char res_result[10] = { 0 };
	root = cJSON_Parse((const char *)returndata);
	if (root == NULL)
	{
		printf("err recv ,is not a json !\n");
		free(returndata);
		return -1;
	}
	item = cJSON_GetObjectItem(root, "result");
	if (item == NULL)
	{
		printf("http_token_get_mqtt_server_info:json err2\n");
		cJSON_Delete(root);
		free(returndata);
		return -1;
	}
	memcpy(res_result, item->valuestring, strlen(item->valuestring));
	printf("http_token_get_mqtt_server_info = %s\r\n",res_result);
	if (strcmp(res_result, "s") != 0)
	{
		char err[100] = { 0 };
		char msg[1000] = { 0 };
		char errinfo[1000] = { 0 };
		http_reg_out("服务器应答失败,解析错误描述\n");
		cJSON *object = cJSON_GetObjectItem(root, "response");
		item = cJSON_GetObjectItem(object, "err");
		if (get_json_value_can_not_null(item, err, 1, sizeof(err)) < 0)
		{
			printf("http_token_get_mqtt_server_info:json err3\n");
			cJSON_Delete(root);
			free(returndata);
			return -1;
		}
		item = cJSON_GetObjectItem(object, "msg");
		if (get_json_value_can_not_null(item, msg, 1, sizeof(msg)) < 0)
		{
			printf("http_token_get_mqtt_server_info:json err4\n");
			cJSON_Delete(root);
			free(returndata);
			return -1;
		}
		item = cJSON_GetObjectItem(object, "errinfo");
		if (get_json_value_can_not_null(item, errinfo, 1, sizeof(errinfo)) < 0)
		{
			printf("http_token_get_mqtt_server_info:json err5\n");
			cJSON_Delete(root);
			free(returndata);
			return -1;
		}
		http_reg_out(" 服务器应答错误：");
		printf("err = %s,msg = %s,errinfo = %s\n", err, msg, errinfo);
		cJSON_Delete(root);
		free(returndata);
		return -1;
	}
	else
	{
		if (strlen(returndata) > *r_len)
		{
			http_reg_out("超过接收内存无法保存数据\n");
			cJSON_Delete(root);
			free(returndata);
			return -1;
		}
		memcpy(r_data, returndata, strlen(returndata));
		*r_len = strlen(returndata);
	}

	cJSON_Delete(root);
	free(returndata);
	return 0;
}

int http_token_get_mqtt_connect_md5(struct _http_comon *parm, char *s_data, int s_len,char **r_data,int *r_len)
{
	int result;
	char *returndata = NULL;
	int returndata_len;
	char request[500] = { 0 };
	cJSON *root = NULL, *item = NULL;
	char *sign_data;
	MD5_CTX ctx;
	//char *p = NULL;
	char md5_sha_sign[100] = { 0 };
	char outmd[16] = { 0 };
	char outmd_up[50] = { 0 };

	result = http_get_appid_secert_token(parm);
	if (result < 0)
	{
		http_reg_out("获取token失败\n");
		return -1;
	}
	http_reg_out("token 获取成功\n");
	sign_data = calloc(1, s_len + 1000);

	sprintf(sign_data, "access_token=%s&appid=%s&timestamp=%s%s", parm->token, parm->appid, parm->timestamp, s_data);

	MD5_Init(&ctx);
	MD5_Update(&ctx, sign_data, strlen((const char *)sign_data));
	MD5_Final((uint8 *)outmd, &ctx);

	//printf_array(outmd,16);

	//MD5加密完成
	//MD5加密结果进行转16进制字符串再转大写
	int i = 0;
	for (i = 0; i < 16; i++)
	{
		sprintf(outmd_up + i * 2, "%02X", (unsigned char)outmd[i]);
	}


	//http_reg_out("outmd_up = %s\n", outmd_up);
	//HexToStr((char *)outmd_up, (char *)outmd, 16);
	//p = str2upper((char *)outmd_up);
	sprintf(md5_sha_sign, "%s", outmd_up);
	free(sign_data);



	sprintf((char*)request, "/was.dispatch/link_key?appid=%s&timestamp=%s&sign=%s&access_token=%s", parm->appid, parm->timestamp, md5_sha_sign, parm->token);
	printf("request:\n%s\n", request);

	printf("data:\n%s\n", s_data);
	result = socket_http_netPost(parm->s_ip, parm->s_port, request, "application/x-www-form-urlencoded", (char *)s_data, 5, &returndata, &returndata_len);
	if (result < 0)
	{
		if (returndata != NULL)
		{
			http_reg_out("释放http接收数据内存\n");
			free(returndata);
			returndata = NULL;
			http_reg_out("释放内存成功,指向空\n");
		}
		http_reg_out("获取http 数据失败\n");
		return -2;
	}
	//http_reg_out("http_token_get_mqtt_connect_md5 获取到的是数据为%s\n", returndata);

	int tmp_len = strlen(returndata) + 1024;
	char *tmp;
	tmp = calloc(1, tmp_len);
	//EncordingConvert("gbk", "utf-8", returndata, strlen(returndata), tmp, sizeof(tmp));
	utf82gbk((uint8 *)returndata, (uint8 *)tmp);
	//EncordingConvert("utf-8", "gbk", returndata, strlen(returndata), tmp, tmp_len);
	//http_reg_out("http_token_get_mqtt_connect_md5 获取到的是数据为%s\n", tmp);
	char res_result[10] = { 0 };
	root = cJSON_Parse((const char *)tmp);
	if (root == NULL)
	{
		http_reg_out("err recv ,is not a json !\n");
		free(returndata);
		free(tmp);
		return -3;
	}
	free(tmp);
	free(returndata);
	item = cJSON_GetObjectItem(root, "result");
	if (item == NULL)
	{
		printf("%s\n", returndata);
		http_reg_out("http_token_get_mqtt_connect_md5 :json err2\n");
		cJSON_Delete(root);
		return -4;
	}
	memcpy(res_result, item->valuestring, strlen(item->valuestring));
	printf("http_token_get_mqtt_connect_md5 = %s\r\n",res_result);
	if (strcmp(res_result, "s") != 0)
	{
		char err[100] = { 0 };
		char msg[1000] = { 0 };
		char errinfo[1000] = { 0 };
		http_reg_out("服务器应答失败,解析错误描述\n");
		cJSON *object = cJSON_GetObjectItem(root, "response");
		item = cJSON_GetObjectItem(object, "err");
		if (get_json_value_can_not_null(item, err, 1, sizeof(err)) < 0)
		{
			http_reg_out("http_token_get_mqtt_connect_md5:json err3\n");
			cJSON_Delete(root);
			return -5;
		}
		item = cJSON_GetObjectItem(object, "msg");
		if (get_json_value_can_not_null(item, msg, 1, sizeof(msg)) < 0)
		{
			http_reg_out("http_token_get_mqtt_connect_md5:json err4\n");
			cJSON_Delete(root);
			return -6;
		}
		item = cJSON_GetObjectItem(object, "errinfo");
		if (get_json_value_can_not_null(item, errinfo, 1, sizeof(errinfo)) < 0)
		{
			http_reg_out("http_token_get_mqtt_connect_md5:json err5\n");
			cJSON_Delete(root);
			return -7;
		}
		//http_reg_out(" 服务器应答错误：\n");
		cJSON_Delete(root);
		http_reg_out("err = %s,msg = %s,errinfo = %s\n", err, msg, errinfo);
		return -8;
	}
	else
	{
		cJSON *object = cJSON_GetObjectItem(root, "response");
		item = cJSON_GetObjectItem(object, "encstr");

		int basetmp_len = 1024*1024;
		char *basetmp;
		basetmp = calloc(1, basetmp_len);

		if (get_json_value_can_not_null(item, basetmp, 1, basetmp_len) < 0)
		{
			free(basetmp);
			printf("http_token_get_mqtt_connect_md5:json err6\n");
			cJSON_Delete(root);
			return -9;
		}

		*r_data = calloc(1, strlen(basetmp)+1024);

		decode(basetmp, strlen((const char *)basetmp), *r_data);
		*r_len = strlen(*r_data);
	}
	cJSON_Delete(root);
	return 0;
}

int http_token_get_mqtt_connect_info(struct _http_comon *parm, char *s_data, int s_len,char **r_data, int *r_len)
{


	int result;
	char *returndata = NULL;
	int returndata_len;
	char request[500] = { 0 };
	cJSON *root = NULL, *item = NULL;
	char *sign_data;
	MD5_CTX ctx;
	//char *p = NULL;
	char md5_sha_sign[100] = { 0 };
	char outmd[16] = { 0 };
	char outmd_up[50] = { 0 };

	result = http_get_appid_secert_token(parm);
	if (result < 0)
	{
		http_reg_out("获取token失败\n");
		return -1;
	}
	//http_reg_out("token 获取成功\n");
	sign_data = calloc(1, s_len + 1000);

	sprintf(sign_data, "access_token=%s&appid=%s&timestamp=%s%s", parm->token, parm->appid, parm->timestamp, s_data);

	MD5_Init(&ctx);
	MD5_Update(&ctx, sign_data, strlen((const char *)sign_data));
	MD5_Final((uint8 *)outmd, &ctx);

	//printf_array(outmd,16);

	//MD5加密完成
	//MD5加密结果进行转16进制字符串再转大写
	int i = 0;
	for (i = 0; i < 16; i++)
	{
		sprintf(outmd_up + i * 2, "%02X", (unsigned char)outmd[i]);
	}


	//http_reg_out("outmd_up = %s\n", outmd_up);
	//HexToStr((char *)outmd_up, (char *)outmd, 16);
	//p = str2upper((char *)outmd_up);
	sprintf(md5_sha_sign, "%s", outmd_up);
	free(sign_data);



	

	sprintf((char*)request, "/was.dispatch/link_info?appid=%s&timestamp=%s&sign=%s&access_token=%s", parm->appid, parm->timestamp, md5_sha_sign, parm->token);
	result = socket_http_netPost(parm->s_ip, parm->s_port, request, "application/x-www-form-urlencoded", (char *)s_data, 5, &returndata, &returndata_len);
	if (result < 0)
	{
		if (returndata != NULL)
		{
			http_reg_out("释放http接收数据内存\n");
			free(returndata);
			returndata = NULL;
			http_reg_out("释放内存成功,指向空\n");
		}
		http_reg_out("获取http 数据失败\n");
		return -2;
	}
	http_reg_out("parm->s_ip = %s,parm->s_port=%d ,request = %s\n", parm->s_ip,parm->s_port,request);
	http_reg_out("获取到的是数据为%s\n", returndata);

	int tmp_len = strlen(returndata)+1024;
	char *tmp;

	tmp = calloc(1, tmp_len);
	//char tmp[204800] = { 0 };
	//_gbk2utf8(returndata, tmp);

	utf82gbk((uint8 *)returndata, (uint8 *)tmp);

	//EncordingConvert("gbk", "utf-8", returndata, strlen(returndata), tmp, sizeof(tmp));
	//EncordingConvert("utf-8", "gbk", returndata, strlen(returndata), tmp, tmp_len);
	//http_reg_out("获取到的是数据为%s\n", tmp);
	char res_result[10] = { 0 };
	root = cJSON_Parse((const char *)tmp);
	if (root == NULL)
	{
		http_reg_out("err recv ,is not a json !\n");
		free(returndata);
		free(tmp);
		return -3;
	}
	free(returndata);
	free(tmp);
	item = cJSON_GetObjectItem(root, "result");
	if (item == NULL)
	{
		http_reg_out("http_token_get_mqtt_connect_info:json err2\n");
		cJSON_Delete(root);
		return -4;
	}
	memcpy(res_result, item->valuestring, strlen(item->valuestring));
	printf("http_token_get_mqtt_connect_info = %s\r\n",res_result);
	if (strcmp(res_result, "s") != 0)
	{
		char err[100] = { 0 };
		char msg[1000] = { 0 };
		char errinfo[1000] = { 0 };
		http_reg_out("服务器应答失败,解析错误描述\n");
		cJSON *object = cJSON_GetObjectItem(root, "response");
		item = cJSON_GetObjectItem(object, "err");
		if (get_json_value_can_not_null(item, err, 1, sizeof(err)) < 0)
		{
			http_reg_out("http_token_get_mqtt_connect_info:json err3\n");
			cJSON_Delete(root);
			return -5;
		}
		item = cJSON_GetObjectItem(object, "msg");
		if (get_json_value_can_not_null(item, msg, 1, sizeof(msg)) < 0)
		{
			http_reg_out("http_token_get_mqtt_connect_info:json err4\n");
			cJSON_Delete(root);
			return -6;
		}
		item = cJSON_GetObjectItem(object, "errinfo");
		if (get_json_value_can_not_null(item, errinfo, 1, sizeof(errinfo)) < 0)
		{
			http_reg_out("http_token_get_mqtt_connect_info:json err5\n");
			cJSON_Delete(root);
			return -7;
		}
		//http_reg_out(" 服务器应答错误：\n");
		cJSON_Delete(root);
		http_reg_out("err = %s,msg = %s,errinfo = %s\n", err, msg, errinfo);
		return -8;
	}
	else
	{
		cJSON *object = cJSON_GetObjectItem(root, "response");
		item = cJSON_GetObjectItem(object, "encstr");

		int basetmp_len = 1024 * 1024;
		char *basetmp;
		basetmp = calloc(1, basetmp_len);

		if (get_json_value_can_not_null(item, basetmp, 1, basetmp_len) < 0)
		{
			free(basetmp);
			printf("http_token_get_mqtt_connect_info:json err6\n");
			cJSON_Delete(root);
			return -9;
		}

		*r_data = calloc(1, strlen(basetmp) + 1024);

		decode(basetmp, strlen((const char *)basetmp), *r_data);
		*r_len = strlen(*r_data);
	}
	cJSON_Delete(root);
	return 0;
}


/*********************************************************************
-   Function : http_get_ter_auth
-   Description：通过http获取服务器授权文件
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
int http_get_ter_auth(int socketfd, struct http_parm_m *parm)
{
	unsigned char s_buf[1024] = { 0 };
	unsigned char *r_buf = NULL;
	unsigned char parmdata[500] = { 0 };
	unsigned char sort_sign[500] = { 0 };
	unsigned char md5_sha_sign[100] = { 0 };
	unsigned char hash[100] = { 0 };
	unsigned char outmd[16] = { 0 };
	unsigned char outmd_up[50] = { 0 };
	unsigned char *dfile = NULL;
	char *g_buf = NULL;
	long file_len = 0;
	char *p = NULL;
	int  result;
	unsigned char auth_update_path[200] = { 0 };
	cJSON *root = NULL, *recv_root = NULL, *item = NULL, *res = NULL;
	MD5_CTX ctx;
	SHA_CTX s;
	//进行url的首字母排序和body组合在一起
	//http://192.168.0.164:8080/intranet/bank/downexport/download_terzip?ter_type=MB_2306&ter_version=4.0.0.3-8
	sprintf((char *)sort_sign, "appid=%s&format=json&timestamp=%s&token=%s&version=1.0{\"ter_id\":\"%s\",\"tar_md5\":\"%s\"}", (char *)parm->appid, (char *)parm->timestamp, (char *)parm->token, (char *)parm->ter_id, (char *)parm->tar_md5);
	net_update_out("^^^^^^^^^^^^^^^^^^^5555^^^^^^^^^^^^111111111^^^^^^^\n");
	//printf("sort_sign:%s\n", sort_sign);
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

	net_update_out("^^^^^^^^^^^^^^^^^^^5555^^^^^^^^^^^^222222222^^^^^^^\n");
	sprintf((char*)parmdata, "/was.tersvr/authorize?version=1.0&appid=%s&timestamp=%s&sign=%s&token=%s&format=json",(char*)parm->appid,(char *)parm->timestamp, (char *)md5_sha_sign, (char *)parm->token);
	memset(s_buf, 0, sizeof(s_buf));
	sprintf((char*)s_buf, "POST %s HTTP/1.1\r\n", (char *)parmdata);
	sprintf((char*)s_buf + strlen((char*)s_buf), "Content-Type: %s\r\n", HTTP_CONTENT_TYPE);
	sprintf((char*)s_buf + strlen((char*)s_buf), "Timeout: 5000\r\n");
	sprintf((char*)s_buf + strlen((char*)s_buf), "Host: %s:%d\r\n", parm->ip, parm->port);

	root = cJSON_CreateObject();
	if (root == NULL)
	{
		printf("http get ter auth json create err!\n");
		return -1;
	}
	cJSON_AddStringToObject(root, "ter_id", (const char *)parm->ter_id);
	cJSON_AddStringToObject(root, "tar_md5", (const char *)parm->tar_md5);

	g_buf = cJSON_PrintUnformatted(root);
	sprintf((char*)s_buf + strlen((char*)s_buf), "Content-Length: %d\r\n", strlen(g_buf) + 2);
	sprintf((char*)s_buf + strlen((char*)s_buf), "User-Agent: libghttp/1.0\r\n");
	sprintf((char*)s_buf + strlen((char*)s_buf), "\r\n");
	sprintf((char*)s_buf + strlen((char*)s_buf), "%s\r\n", g_buf);

	cJSON_Delete(root);
	free(g_buf);
	//printf("http send by get ter auth :\n %s\n", s_buf);
	if (socket_write(socketfd, (unsigned char *)s_buf, strlen((char*)s_buf)) < 0)
	{
		printf("发送失败\n");
		return  BOOT_ERROR_HTTP_SEND;
	}

	result = deal_http_data(socketfd, (char **)&r_buf);
	if (result < 0)
	{
		printf("http get token err!\n");
		return BOOT_ERROR_HTTP_RECV;
	}


	recv_root = cJSON_Parse((char *)r_buf);
	if (recv_root == NULL)
	{
		printf("err recv get ter auth,is not a json !\n");

		free(r_buf);
		return BOOT_ERROR_HTTP_PARSE_JSON;
	}
	free(r_buf);
	item = cJSON_GetObjectItem(recv_root, "result");
	if (item == NULL)
	{
		printf("err recv get ter auth:json err1\n");
		//判断是否token过期，如果过期进行重新取token操作
		item = cJSON_GetObjectItem(recv_root, "response");
		if (item == NULL)
		{
			printf("err recv get ter auth:json err2\n");

			cJSON_Delete(recv_root);
			return BOOT_ERROR_HTTP_PARSE_JSON;
		}
		printf("err recv get ter auth:json result is f get response succ\n");
		res = cJSON_GetObjectItem(item, "err");
		if (res == NULL)
		{
			printf("err recv get ter auth:upgrade not found\n");

			cJSON_Delete(recv_root);
			return BOOT_ERROR_HTTP_PARSE_JSON;
		}
		printf("err recv get ter auth:json result is f get err succ\n");
		if (memcmp(res->valuestring, "101", 3) == 0)
		{
			cJSON_Delete(recv_root);
			return HTTP_TOKEN_ERR;//重发
		}
		cJSON_Delete(recv_root);
		return BOOT_ERROR_HTTP_PARSE_JSON;
	}
	if (memcmp(item->valuestring, "s", 1) != 0)
	{
		printf("err recv get ter auth:json result is f:%s\n", r_buf);

		cJSON_Delete(recv_root);
		return -1;
	}
	item = cJSON_GetObjectItem(recv_root, "response");
	if (item == NULL)
	{
		printf("err recv get ter auth:json err2\n");

		cJSON_Delete(recv_root);
		return BOOT_ERROR_HTTP_PARSE_JSON;
	}

	res = cJSON_GetObjectItem(item, "authorize");
	if (res == NULL)
	{
		cJSON_Delete(recv_root);
		return BOOT_ERROR_HTTP_PARSE_JSON;
	}
	if (memcmp(res->valuestring, "0", 1) == 0)
	{
		printf("Do not need update AUTH!\n");

		cJSON_Delete(recv_root);
		return 2;
	}


	res = cJSON_GetObjectItem(item, "authorizefilelen");
	if (res == NULL)
	{
		printf("err recv get ter auth:authorizefilelen not found\n");

		cJSON_Delete(recv_root);
		return BOOT_ERROR_HTTP_PARSE_JSON;
	}
	file_len = atol(res->valuestring);
	if (file_len > 1024 * 1024 * 10)
	{
		printf("auth err file too large!\n");

		cJSON_Delete(recv_root);
		return BOOT_ERROR_HTTP_RECV_PACKET_TOO_LARGE;
	}
	// if ((calc_path_size("/") * 1024 * 1024) <= file_len)
	// {
	// 	printf("system space is full!\n");

	// 	cJSON_Delete(recv_root);
	// 	return BOOT_ERROR_HTTP_RECV_PACKET_TOO_LARGE;
	// }

	res = cJSON_GetObjectItem(item, "authorizefile");
	if (res == NULL)
	{
		printf("err recv get ter auth:authorizefile not found\n");

		cJSON_Delete(recv_root);
		return BOOT_ERROR_HTTP_PARSE_JSON;
	}
	dfile = boot_malloc(file_len+10);
	if (dfile == NULL)
	{
		cJSON_Delete(recv_root);
		return BOOT_ERROR_OPER_SYSTEM_MALLOC_ERR;
	}
		
	
	//进行base64解码
	decode(res->valuestring, strlen(res->valuestring), (char *)dfile);
	sprintf((char *)auth_update_path, "%s/tmpauth", "/home/share/exe/tar");
	
	result = Write_Decompress_file_m(dfile, file_len, (unsigned char *)"/tmp/auth.tar", auth_update_path);
	if (result < 0)
	{
		printf("err write auth file :%d\n", result);
		printf("dfile :%s\n", dfile);
		if (dfile != NULL)
			free(dfile);
		cJSON_Delete(recv_root);

		
		return result;
	}
	net_update_out("^^^^^^^^^^^^^^^^^^^5555^^^^^^^^^^^^33333333^^^^^^^\n");
	free(dfile);
	cJSON_Delete(recv_root);
	return 0;

}


int http_socket_open_m(int *socket_t,char *ip,int port)
{
	int sock;
	struct sockaddr_in add;
	int result;
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0)
	{
		printf("socket create fail!\n");
		return -1;
	}
	memset(&add, 0, sizeof(add));
	add.sin_family = AF_INET;
	add.sin_port = htons(port);
	add.sin_addr.s_addr = htonl(INADDR_ANY);
	inet_pton(AF_INET, ip, &add.sin_addr);


	if ((result = connect(sock, (struct sockaddr *)&add, sizeof(add))) == -1)
	{
		printf("connect server err %s:%d\n", ip, port);
		shutdown(sock, SHUT_RDWR);
		close(sock);
		return -2;
	}

	const int keepAlive = 1;
	setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, (void*)&keepAlive, sizeof(keepAlive));
	open_rec_over_timer(sock, HTTP_TIMEOUT);
	open_send_over_timer(sock, HTTP_TIMEOUT);
	*socket_t = sock;
	net_update_out("^^^^^^^^^^^^^^^^^^^1^^^^^^^^^^^^^^^^^^^^\n");
	return sock;
}
