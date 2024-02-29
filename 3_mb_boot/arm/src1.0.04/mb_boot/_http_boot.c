#include "_http_boot.h"


int http_socket_open(int *socket_t,char *ip,int port)
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
	return sock;
}

/*********************************************************************
-   Function : http_upload_err
-   Description：通过http上传错误信息到服务器
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
int http_upload_err(int socketfd, struct http_parm *parm)
{
	unsigned char s_buf[1024] = { 0 };
	unsigned char *r_buf = NULL;
	unsigned char parmdata[500] = { 0 };
	unsigned char sort_sign[500] = { 0 };
	unsigned char md5_sha_sign[100] = { 0 };
	unsigned char hash[100] = { 0 };
	unsigned char outmd[16] = { 0 };
	unsigned char outmd_up[50] = { 0 };
	char *g_buf = NULL;
	//long file_len = 0;
	char *p = NULL;
	int  result;
	cJSON *root = NULL, *recv_root = NULL, *item = NULL;
	
	MD5_CTX ctx;
	SHA_CTX s;
	//进行url的首字母排序和body组合在一起
	sprintf((char *)sort_sign, "appid=%s&format=json&timestamp=%s&token=%s&version=1.0{\"ter_id\":\"%s\",\"err_msg\":\"%s\"}", (char *)parm->appid, (char *)parm->timestamp, (char *)parm->token, (char *)parm->ter_id, (char *)parm->errmsg);

	printf("sort_sign:%s\n", sort_sign);
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


	sprintf((char*)parmdata, "/was.tersvr/opererr?version=1.0&appid=%s&timestamp=%s&sign=%s&token=%s&format=json", parm->ter_id, parm->timestamp, md5_sha_sign, parm->token);
	memset(s_buf, 0, sizeof(s_buf));
	
	sprintf((char*)s_buf, "POST %s HTTP/1.1\r\n", parmdata);
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
	cJSON_AddStringToObject(root, "err_msg", (const char *)parm->errmsg);

	g_buf = cJSON_PrintUnformatted(root);
	sprintf((char*)s_buf + strlen((char*)s_buf), "Content-Length: %d\r\n", strlen(g_buf) + 2);
	sprintf((char*)s_buf + strlen((char*)s_buf), "User-Agent: libghttp/1.0\r\n");
	sprintf((char*)s_buf + strlen((char*)s_buf), "\r\n");
	sprintf((char*)s_buf + strlen((char*)s_buf), "%s\r\n", g_buf);

	cJSON_Delete(root);
	free(g_buf);

	if (socket_write(socketfd, (unsigned char *)s_buf, strlen((char*)s_buf)) < 0)
	{
		printf("发送失败\n");
		return  -2;
	}

	result = deal_http_data(socketfd, (char **)&r_buf);
	if (result < 0)
	{
		printf("http upload err!\n");
		cJSON_Delete(recv_root);
		return -1;
	}
	


	recv_root = cJSON_Parse((const char *)r_buf);
	if (root == NULL)
	{
		printf("err recv upload,is not a json !\n");

		free(r_buf);
		cJSON_Delete(recv_root);
		return -1;
	}
	item = cJSON_GetObjectItem(recv_root, "result");
	if (item == NULL)
	{
		printf("err recv upload:json err1\n");

		free(r_buf);
		cJSON_Delete(recv_root);
		return -1;
	}
	if (memcmp(item->valuestring, "s", 1) != 0)
	{
		printf("err upload result is f:%s\n", r_buf);

		free(r_buf);
		cJSON_Delete(recv_root);
		return -1;
	}
	cJSON_Delete(recv_root);
	free(r_buf);

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
int http_get_ter_auth(int socketfd, struct http_parm *parm)
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
	sprintf((char *)sort_sign, "appid=%s&format=json&timestamp=%s&token=%s&version=1.0{\"ter_id\":\"%s\",\"tar_md5\":\"%s\"}", (char *)parm->appid, (char *)parm->timestamp, (char *)parm->token, (char *)parm->ter_id, (char *)parm->tar_md5);

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
		printf("Do not need update!\n");

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
	sprintf((char *)auth_update_path, "%s/tmpauth", TAR_PATH);
	
	result = Write_Decompress_file(dfile, file_len, (unsigned char *)DOWNLOAD_AUTH, auth_update_path);
	if (result < 0)
	{
		printf("err write auth file :%d\n", result);
		printf("dfile :%s\n", dfile);
		if (dfile != NULL)
			free(dfile);
		cJSON_Delete(recv_root);

		
		return result;
	}
	free(dfile);
	cJSON_Delete(recv_root);
	return 0;

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
int http_get_ter_update(int socketfd, struct http_parm *parm)
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

	printf("sort_sign:%s\n", sort_sign);
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
	printf("http send by get ter update :\n %s\n", s_buf);
	if (socket_write(socketfd, (unsigned char *)s_buf, strlen((char*)s_buf)) <= 0)
	{
		printf("发送失败\n");
		return  BOOT_ERROR_HTTP_SEND;
	}
	printf("1111111111111111111111111111111\r\n");

	result = deal_http_data(socketfd, (char **)&r_buf);
	if (result < 0)
	{
		printf("http get token err!\n");
		return BOOT_ERROR_HTTP_RECV;
	}
	printf("1111111111111111111111111111112,r_buf = %s\r\n",r_buf);
	
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
		printf("Do not need update!\n");

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
	sprintf((char *)app_update_path, "%s/tmpapp", TAR_PATH);
	result = Write_Decompress_file(dfile, file_len, (unsigned char *)DOWNLOAD_FILENAME, app_update_path);
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
	char str_hash[100] = { 0 };
	unsigned char hash[20];
	result = http_get_server_time(socketfd, time,parm->ip,parm->port);//获取服务器时间
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
	//printf("s_buf = %s\r\n",s_buf);			//wang
	if (socket_write(socketfd, (unsigned char *)s_buf, strlen((char*)s_buf)) <= 0)
	{
		printf("发送失败\n");
		return -2;
	}
	result = deal_http_data(socketfd, (char **)&r_buf);
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

	result = deal_http_data(socketfd, (char **)&r_buf);
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
int http_get_server_time(int socketfd, unsigned char *time,char *ip,int port)
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

	result = deal_http_data(socketfd, (char **)&r_buf);
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
int open_rec_over_timer(int socketfd, int over_timer)
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
int open_send_over_timer(int socketfd, int over_timer)
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
int deal_http_data(int sockfd, char **outbuf)
{
	char buf[1024];
	int i;
	char c;
	char len_buf[50];
	int numchars = 1;
	int content_length = -1;
	memset(buf, 0, sizeof(buf));
	printf("deal_http_data start!\r\n");
	numchars = get_line(sockfd, buf, sizeof(buf));
	printf("deal_http_data numchars%d\r\n",numchars);
	if (numchars == 0)
	{
		return -1;
	}
	memset(buf, 0, sizeof(buf));
	numchars = get_line(sockfd, buf, sizeof(buf));
	printf("deal_http_data1 numchars%d\r\n",numchars);
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
		numchars = get_line(sockfd, buf, sizeof(buf));

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
	printf("recv over!\r\n");
	*outbuf = tmp;
	return content_length;
}




#ifdef MIPS_MTK
int GetDnsIp(char *host, char *ip)
{
	char   buf[100];
	struct addrinfo hints;
	struct addrinfo *res, *curr;
	struct sockaddr_in *sa;
	int ret;
	/* obtaining address matching host */
	if (host == NULL)
	{
		//out("要解析的域名错误\n");
		return -1;
	}
	//out("get_DNS_ip 11\n");
	if (memcmp(host, "127.0.0.1", strlen((const char*)host)) == 0)
	{
		//out("本机IP不用解析\n");
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
#else
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

#endif
