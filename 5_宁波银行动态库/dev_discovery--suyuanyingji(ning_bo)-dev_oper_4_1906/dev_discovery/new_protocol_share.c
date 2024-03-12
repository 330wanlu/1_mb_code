#include "new_protocol_share.h"



void socket_close1(SOCKET socketfd)
{
	shutdown(socketfd, SD_BOTH);
	closesocket(socketfd);
}
/*********************************************************************
-   Function : socket_create_connect
-   Description：创建一个socket并链接指定ip端口，属性为无延时长链
-   Calls：
-   Calles By:
-   Table Accessed :
-   Table Update :
-   Input  :参数一:
-
-   Output :
-   Return :0 正常 非零 返回相应的错误码(通过文档查找相应的错误描述)
-   Other :
***********************************************************************/

int socket_create_connect_old(SOCKET *socket_fd, char *server_ip, int port)
{
	SOCKET sclient = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);


	if (sclient == INVALID_SOCKET)
	{

		WriteLogFile("socket_create_connect invalid socket!");
		return DLL_ERR_NEW_SHARE_SOCKET_CONNET;
	}
	struct sockaddr_in serAddr;
	serAddr.sin_family = AF_INET;
	serAddr.sin_port = htons(port);
	serAddr.sin_addr.S_un.S_addr = inet_addr(server_ip);
	if (connect(sclient, (struct sockaddr *)&serAddr, sizeof(serAddr)) == SOCKET_ERROR)
	{
		closesocket(sclient);
		return -1;
	}
	const int val = 1;
	const int keepAlive = 1;
	setsockopt(sclient, IPPROTO_TCP, TCP_NODELAY, (char*)&val, sizeof(val));
	setsockopt(sclient, SOL_SOCKET, SO_KEEPALIVE, (void*)&keepAlive, sizeof(keepAlive));

	int timeout = 15000; //15s
	int ret = setsockopt(sclient, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));


	ret = setsockopt(sclient, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

	*socket_fd = sclient;
	return 0;
}

int socket_create_connect(SOCKET *socket_fd, char *server_ip, int port)
{
	int ret = 0;
	struct sockaddr_in sin;
	int iMode = 1, nRecvBuf = 0, nSendBuf = 0, reuse0 = 1;
	struct timeval tm;
	BOOL bDontLinger = FALSE;
	struct timeval timeout;

	tm.tv_sec = 2;
	tm.tv_usec = 0;
	timeout.tv_sec = 2000;
	sin.sin_addr.S_un.S_addr = inet_addr(server_ip);
	sin.sin_family = AF_INET;
	sin.sin_port = htons(port);
	SOCKET clifd = socket(AF_INET, SOCK_STREAM, 0);


	setsockopt(clifd, SOL_SOCKET, SO_RCVBUF, (const char*)&nRecvBuf, sizeof(int));
	setsockopt(clifd, SOL_SOCKET, SO_SNDBUF, (const char*)&nSendBuf, sizeof(int));
	setsockopt(clifd, SOL_SOCKET, SO_DONTLINGER, (const char*)&bDontLinger, sizeof(BOOL));
	setsockopt(clifd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout.tv_sec, sizeof(timeout));
	setsockopt(clifd, SOL_SOCKET, SO_SNDTIMEO, (const char*)&timeout.tv_sec, sizeof(timeout));

	//if (setsockopt(clifd, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse0, sizeof(reuse0)) == -1) return SOCKET_ERROR;

	iMode = 1;
	ioctlsocket(clifd, FIONBIO, (u_long FAR*)&iMode); //设置为非阻塞模式

	if (connect(clifd, (struct sockaddr*)&sin, sizeof(sin)) != SOCKET_ERROR){
		
		deal_sockets5(clifd, server_ip,port,1);
		*socket_fd = clifd;
		WriteLogFile("socket_create_connect deal_sockets5  clifd = %d",clifd);
		return 0;
	}
	else{
		fd_set set;
		FD_ZERO(&set);
		FD_SET(clifd, &set);

		if (select(-1, NULL, &set, NULL, &tm) <= 0)
		{
			ret = GetLastError();
			ret = -1; // 有错误(select错误或者超时)
			socket_close1(clifd);
			return -1;
		}
		else
		{
			int error = -1;
			int optLen = sizeof(int);
			getsockopt(clifd, SOL_SOCKET, SO_ERROR, (char*)&error, &optLen);

			if (0 != error){
				ret = -1; // 有错误  
				socket_close1(clifd);
				return -1;
			}
			else{
				ret = 0;  // 无错误  
			}
		}
	}
	iMode = 0;
	ioctlsocket(clifd, FIONBIO, (u_long FAR*)&iMode); //设置为阻塞模式

	*socket_fd = clifd;
	return ret;
}

/*********************************************************************
-   Function : recv_data_len
-   Description：接收4个字节长度位并转换为long型返回
-   Calls：
-   Calles By:
-   Table Accessed :
-   Table Update :
-   Input  :参数一:
-
-   Output :
-   Return :0 正常 非零 返回相应的错误码(通过文档查找相应的错误描述)
-   Other :
***********************************************************************/
long recv_data_len(int fd)
{
	int result;
	long len;
	unsigned char len_code[4];
	memset(len_code, 0, sizeof(len_code));
	result = recv(fd, len_code, sizeof(len_code),0);
	if (result<0)
	{
		WriteLogFile("recv len err!\n");
		return result;
	}
	WriteLogFile("recv len %02x %02x %02x %02x !\n", len_code[0], len_code[1], len_code[2], len_code[3]);
	len = (len_code[0] * 0x1000000) + (len_code[1] * 0x10000) + (len_code[2] * 0x100) + len_code[3];
	return len;
}


/*********************************************************************
-   Function : parse_json_result
-   Description：解析返回的json的head中的result是否为S
-   Calls：
-   Calles By:
-   Table Accessed :
-   Table Update :
-   Input  :参数一:
-
-   Output :
-   Return :0 正常 非零 返回相应的错误码(通过文档查找相应的错误描述)
-   Other :
***********************************************************************/
static int parse_json_result(char *json_buf)
{
	cJSON *root = NULL;
	cJSON *head;
	cJSON *item;
//	int result;
	root = cJSON_Parse(json_buf);
	if (root == NULL)
		return -1;
	head = cJSON_GetObjectItem(root, "head");
	if (head == NULL)
		return DLL_ERR_NEW_SHARE_CMD_OPER;
	item = cJSON_GetObjectItem(head, "result");
	if (item == NULL)
		return DLL_ERR_NEW_SHARE_CMD_OPER;
	if (memcmp(item->valuestring, "s", 1) == 0)
		return 0;
	else
		return DLL_ERR_NEW_SHARE_CMD_OPER;

}


/*********************************************************************
-   Function : restart_terminal
-   Description：重启终端机柜
-   Calls：
-   Calles By:
-   Table Accessed :
-   Table Update :
-   Input  :参数一:
-
-   Output :
-   Return :0 正常 非零 返回相应的错误码(通过文档查找相应的错误描述)
-   Other :
***********************************************************************/
int restart_terminal(char *server_ip, int server_port)
{
	char send_json[1024] = { 0 };
	int result,i = 0;
	cJSON *root = NULL;
	cJSON *head = NULL;
	/*SOCKET sclient = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);


	if (sclient == INVALID_SOCKET)
	{

		WriteLogFile("restart_terminal invalid socket!");
		return DLL_ERR_NEW_SHARE_SOCKET_CONNET;
	}
	struct sockaddr_in serAddr;
	serAddr.sin_family = AF_INET;
	serAddr.sin_port = htons(server_port);
	serAddr.sin_addr.S_un.S_addr = inet_addr(server_ip);
	if (connect(sclient, (struct sockaddr *)&serAddr, sizeof(serAddr)) == SOCKET_ERROR)
	{

		WriteLogFile("restart_terminal connect error!");
		closesocket(sclient);
		return 0;
	}*/
	SOCKET sclient;

	struct file_setting file_setting[128] = {0};
	WriteLogFile("dev_oper restart_terminal server_ip=%s,server_port=%d", server_ip,server_port);

	result = read_setting(file_setting, 128);
	if(result < 0)
	{
		WriteLogFile("read_setting failed");
	}
	for(i = 0;i < result;++i)
	{
		if(0 == memcmp(file_setting[i].terminal_ip,server_ip,strlen(server_ip)) && file_setting[i].data_port == server_port)
		{
			break;
		}
	}
	if(i == result)
	{
		WriteLogFile("dev_oper can not find mattched ip");
		return -1;
	}

	WriteLogFile("dev_oper get_usbshare_status_new terminal_ip=%s,data_port=%d,remote_ip=%s,remote_data_port=%d", file_setting[i].terminal_ip,file_setting[i].data_port,file_setting[i].remote_ip,file_setting[i].remote_dataport);
	//result = socket_create_connect(&s_client, server_ip, server_port);

	if((sclient = socket_connect(file_setting[i].terminal_ip,file_setting[i].data_port,file_setting[i].remote_ip,file_setting[i].remote_dataport,3000)) == SOCKET_ERROR)
	{
		WriteLogFile("dev_oper get_usbshare_status_new create or connect socket fail!");
		return DLL_ERR_NEW_SHARE_SOCKET_CONNET;
	}

	// result = socket_create_connect(&sclient, server_ip, server_port);
	// if (result < 0)
	// {
	// 	WriteLogFile("restart_terminal create or connect socket fail!");
	// 	return DLL_ERR_NEW_SHARE_SOCKET_CONNET;
	// }

	root = cJSON_CreateObject();
	if (root == NULL)
	{
		WriteLogFile("restart_terminal cJSON_CreateObject root error!");
		shutdown(sclient, SD_BOTH);
		closesocket(sclient);
		return -1;
	}


	cJSON_AddItemToObject(root, "head", head = cJSON_CreateObject());
	cJSON_AddStringToObject(head, "protocol", "mb_usbshare_20191024");
	cJSON_AddStringToObject(head, "code_type", "request");
	cJSON_AddStringToObject(head, "cmd", "ter_restart");
	cJSON_AddStringToObject(head, "source_topic", "");
	cJSON_AddStringToObject(head, "random", "987654321");
	cJSON_AddStringToObject(head, "result", "");
	cJSON *data;
	cJSON_AddItemToObject(root, "data", data = cJSON_CreateArray());
	cJSON *item;
	cJSON_AddItemToObject(data, "dira", item = cJSON_CreateObject());


	char *g_buf;
	int len;
	g_buf = cJSON_Print(root);

	memcpy(send_json, "\x01\x06\x80\x07", 4);
	send_json[4] = strlen(g_buf) / 0x1000000; send_json[5] = strlen(g_buf) / 0x10000;
	send_json[6] = strlen(g_buf) / 256; send_json[7] = strlen(g_buf) % 256;
	memcpy(send_json + 8, g_buf, strlen(g_buf));
	len = strlen(g_buf) + 8;
	free(g_buf);
	cJSON_Delete(root);

	result = send(sclient, send_json, len, 0);
	if (result < 0)
	{

		WriteLogFile("restart_terminal send error!");
		shutdown(sclient, SD_BOTH);
		closesocket(sclient);
		return DLL_ERR_NEW_SHARE_SOCKET_SEND;
	}
	char recvhead[10] = { 0 };
	char recvbody[1024] = { 0 };
	long recvlen = 0;
	result = recv(sclient, recvhead, 4, 0);
	if (result < 0)
	{

		WriteLogFile("restart_terminal recv head error!");
		shutdown(sclient, SD_BOTH);
		closesocket(sclient);
		return DLL_ERR_NEW_SHARE_SOCKET_RECV;
	}
	recvlen = recv_data_len(sclient);
	if (recvlen < 0)
	{

		WriteLogFile("restart_terminal recv len error!");
		shutdown(sclient, SD_BOTH);
		closesocket(sclient);
		return DLL_ERR_NEW_SHARE_SOCKET_RECV;
	}
	result = recv(sclient, recvbody, recvlen, 0);
	if (result < 0)
	{

		WriteLogFile("restart_terminal recv body error!");
		shutdown(sclient, SD_BOTH);
		closesocket(sclient);
		return DLL_ERR_NEW_SHARE_SOCKET_RECV;
	}
	result = parse_json_result(recvbody);
	shutdown(sclient, SD_BOTH);
	closesocket(sclient);
	return result;
}


// int restart_terminal_nb(char *server_ip, int server_port)
// {
// 	char send_json[1024] = { 0 };
// 	int result,i = 0;
// 	cJSON *root = NULL;
// 	cJSON *head = NULL;
// 	/*SOCKET sclient = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);


// 	if (sclient == INVALID_SOCKET)
// 	{

// 		WriteLogFile("restart_terminal invalid socket!");
// 		return DLL_ERR_NEW_SHARE_SOCKET_CONNET;
// 	}
// 	struct sockaddr_in serAddr;
// 	serAddr.sin_family = AF_INET;
// 	serAddr.sin_port = htons(server_port);
// 	serAddr.sin_addr.S_un.S_addr = inet_addr(server_ip);
// 	if (connect(sclient, (struct sockaddr *)&serAddr, sizeof(serAddr)) == SOCKET_ERROR)
// 	{

// 		WriteLogFile("restart_terminal connect error!");
// 		closesocket(sclient);
// 		return 0;
// 	}*/
	
	
	
// 	// SOCKET sclient;

// 	// struct file_setting file_setting[128] = {0};
// 	// WriteLogFile("dev_oper restart_terminal server_ip=%s,server_port=%d", server_ip,server_port);

// 	// result = read_setting(file_setting, 128);
// 	// if(result < 0)
// 	// {
// 	// 	WriteLogFile("read_setting failed");
// 	// }
// 	// for(i = 0;i < result;++i)
// 	// {
// 	// 	if(0 == memcmp(file_setting[i].terminal_ip,server_ip,strlen(server_ip)) && file_setting[i].data_port == server_port)
// 	// 	{
// 	// 		break;
// 	// 	}
// 	// }
// 	// if(i == result)
// 	// {
// 	// 	WriteLogFile("dev_oper can not find mattched ip");
// 	// 	return -1;
// 	// }

// 	// WriteLogFile("dev_oper get_usbshare_status_new terminal_ip=%s,data_port=%d,remote_ip=%s,remote_data_port=%d", file_setting[i].terminal_ip,file_setting[i].data_port,file_setting[i].remote_ip,file_setting[i].remote_dataport);
// 	// //result = socket_create_connect(&s_client, server_ip, server_port);

// 	// if((sclient = socket_connect(file_setting[i].terminal_ip,file_setting[i].data_port,file_setting[i].remote_ip,file_setting[i].remote_dataport,3000)) == SOCKET_ERROR)
// 	// {
// 	// 	WriteLogFile("dev_oper get_usbshare_status_new create or connect socket fail!");
// 	// 	return DLL_ERR_NEW_SHARE_SOCKET_CONNET;
// 	// }


// 	// result = socket_create_connect(&sclient, server_ip, server_port);
// 	// if (result < 0)
// 	// {
// 	// 	WriteLogFile("restart_terminal create or connect socket fail!");
// 	// 	return DLL_ERR_NEW_SHARE_SOCKET_CONNET;
// 	// }

// 	root = cJSON_CreateObject();
// 	if (root == NULL)
// 	{
// 		WriteLogFile("restart_terminal cJSON_CreateObject root error!");
// 		shutdown(sclient, SD_BOTH);
// 		closesocket(sclient);
// 		return -1;
// 	}


// 	cJSON_AddItemToObject(root, "head", head = cJSON_CreateObject());
// 	cJSON_AddStringToObject(head, "protocol", "mb_usbshare_20191024");
// 	cJSON_AddStringToObject(head, "code_type", "request");
// 	cJSON_AddStringToObject(head, "cmd", "ter_restart");
// 	cJSON_AddStringToObject(head, "source_topic", "");
// 	cJSON_AddStringToObject(head, "random", "987654321");
// 	cJSON_AddStringToObject(head, "result", "");
// 	cJSON *data;
// 	cJSON_AddItemToObject(root, "data", data = cJSON_CreateArray());
// 	cJSON *item;
// 	cJSON_AddItemToObject(data, "dira", item = cJSON_CreateObject());


// 	char *g_buf;
// 	int len;
// 	g_buf = cJSON_Print(root);

// 	memcpy(send_json, "\x01\x06\x80\x07", 4);
// 	send_json[4] = strlen(g_buf) / 0x1000000; send_json[5] = strlen(g_buf) / 0x10000;
// 	send_json[6] = strlen(g_buf) / 256; send_json[7] = strlen(g_buf) % 256;
// 	memcpy(send_json + 8, g_buf, strlen(g_buf));
// 	len = strlen(g_buf) + 8;
// 	free(g_buf);
// 	cJSON_Delete(root);

// 	result = send(sclient, send_json, len, 0);
// 	if (result < 0)
// 	{

// 		WriteLogFile("restart_terminal send error!");
// 		shutdown(sclient, SD_BOTH);
// 		closesocket(sclient);
// 		return DLL_ERR_NEW_SHARE_SOCKET_SEND;
// 	}
// 	char recvhead[10] = { 0 };
// 	char recvbody[1024] = { 0 };
// 	long recvlen = 0;
// 	result = recv(sclient, recvhead, 4, 0);
// 	if (result < 0)
// 	{

// 		WriteLogFile("restart_terminal recv head error!");
// 		shutdown(sclient, SD_BOTH);
// 		closesocket(sclient);
// 		return DLL_ERR_NEW_SHARE_SOCKET_RECV;
// 	}
// 	recvlen = recv_data_len(sclient);
// 	if (recvlen < 0)
// 	{

// 		WriteLogFile("restart_terminal recv len error!");
// 		shutdown(sclient, SD_BOTH);
// 		closesocket(sclient);
// 		return DLL_ERR_NEW_SHARE_SOCKET_RECV;
// 	}
// 	result = recv(sclient, recvbody, recvlen, 0);
// 	if (result < 0)
// 	{

// 		WriteLogFile("restart_terminal recv body error!");
// 		shutdown(sclient, SD_BOTH);
// 		closesocket(sclient);
// 		return DLL_ERR_NEW_SHARE_SOCKET_RECV;
// 	}
// 	result = parse_json_result(recvbody);
// 	shutdown(sclient, SD_BOTH);
// 	closesocket(sclient);
// 	return result;
// }

/*********************************************************************
-   Function : set_usbshare_ip
-   Description：机柜IP设置
-   Calls：
-   Calles By:
-   Table Accessed :
-   Table Update :
-   Input  :参数一:
-
-   Output :
-   Return :0 正常 非零 返回相应的错误码(通过文档查找相应的错误描述)
-   Other :
***********************************************************************/
int set_usbshare_ip_new(int ip_type, char *server_ip, int server_port, char *ip_in, char *mask_in, char *gateway_in, char *dns_in, char *extension)
{
	char send_json[1024] = { 0 };
	int result;
	cJSON *root = NULL;
	cJSON *head = NULL;
	//SOCKET sclient = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	SOCKET sclient;
	result = socket_create_connect(&sclient, server_ip, server_port);
	if (result < 0)
	{
		WriteLogFile("set_usbshare_ip_new create or connect socket fail!");
		return DLL_ERR_NEW_SHARE_SOCKET_CONNET;
	}
	if (sclient == INVALID_SOCKET)
	{
		
		WriteLogFile("set_usbshare_ip_new invalid socket!");
		return DLL_ERR_NEW_SHARE_SOCKET_CONNET;
	}
	/*struct sockaddr_in serAddr;
	serAddr.sin_family = AF_INET;
	serAddr.sin_port = htons(server_port);
	serAddr.sin_addr.S_un.S_addr = inet_addr(server_ip);
	if (connect(sclient, (struct sockaddr *)&serAddr, sizeof(serAddr)) == SOCKET_ERROR)
	{
		
		WriteLogFile("set_usbshare_ip_new connect error!");
		closesocket(sclient);
		return 0;
	}*/
	root = cJSON_CreateObject();
	if (root == NULL)
	{
		WriteLogFile("set_usbshare_ip_new cJSON_CreateObject root error!");
		shutdown(sclient, SD_BOTH);
		closesocket(sclient);
		return -1;
	}

	
	cJSON_AddItemToObject(root, "head", head = cJSON_CreateObject());
	cJSON_AddStringToObject(head, "protocol", "mb_usbshare_20191024");
	cJSON_AddStringToObject(head, "code_type", "request");
	cJSON_AddStringToObject(head, "cmd", "set_ip");
	cJSON_AddStringToObject(head, "source_topic", "");
	cJSON_AddStringToObject(head, "random", "987654321");
	cJSON_AddStringToObject(head, "result", "");
	cJSON *data;
	cJSON_AddItemToObject(root, "data", data = cJSON_CreateArray());
	cJSON *item;
	cJSON_AddItemToObject(data, "dira", item = cJSON_CreateObject());

	char type[5] = { 0 };
	sprintf(type, "%d", ip_type);
	cJSON_AddStringToObject(item, "ip_type", type);
	cJSON_AddStringToObject(item, "ip_address", ip_in);
	cJSON_AddStringToObject(item, "netmask", mask_in);
	cJSON_AddStringToObject(item, "gateway", gateway_in);
	cJSON_AddStringToObject(item, "dns", dns_in);


	char *g_buf;
	int len;
	g_buf = cJSON_Print(root);

	memcpy(send_json, "\x01\x06\x80\x07", 4);
	send_json[4] = strlen(g_buf) / 0x1000000; send_json[5] = strlen(g_buf) / 0x10000;
	send_json[6] = strlen(g_buf) / 256; send_json[7] = strlen(g_buf) % 256;
	memcpy(send_json + 8, g_buf, strlen(g_buf));
	len = strlen(g_buf) + 8;
	free(g_buf);
	cJSON_Delete(root);
	WriteLogFile("获取机柜信息发送报文中!");
	result = send(sclient, send_json, len, 0);
	if (result < 0)
	{

		WriteLogFile("set_usbshare_ip_new send error!");
		shutdown(sclient, SD_BOTH);
		closesocket(sclient);
		return DLL_ERR_NEW_SHARE_SOCKET_SEND;
	}
	Sleep(4*1000);
	//WriteLogFile("获取机柜信息发送报文完成!");
	char recvhead[10] = { 0 };
	char recvbody[1024] = { 0 };
	long recvlen = 0;
	result = recv(sclient, recvhead, 4, 0);
	if (result < 0)
	{

		WriteLogFile("set_usbshare_ip_new recv head error!");
		shutdown(sclient, SD_BOTH);
		closesocket(sclient);
		return DLL_ERR_NEW_SHARE_SOCKET_RECV;
	}
	WriteLogFile("获取机柜信息头部接收完成!");
	recvlen = recv_data_len(sclient);
	if (recvlen < 0)
	{

		WriteLogFile("set_usbshare_ip_new recv len error!");
		shutdown(sclient, SD_BOTH);
		closesocket(sclient);
		return DLL_ERR_NEW_SHARE_SOCKET_RECV;
	}
	WriteLogFile("获取机柜信息解析json长度完成!");
	result = recv(sclient, recvbody, recvlen, 0);
	if (result < 0)
	{

		WriteLogFile("set_usbshare_ip_new recv body error!");
		shutdown(sclient, SD_BOTH);
		closesocket(sclient);
		return DLL_ERR_NEW_SHARE_SOCKET_RECV;
	}
	WriteLogFile("获取机柜信息接收json完成!");
	result = parse_json_result(recvbody);
	shutdown(sclient, SD_BOTH);
	closesocket(sclient);
	return result;
}


/*********************************************************************
-   Function : set_usbshare_id_date
-   Description：机柜编号及时间设置
-   Calls：
-   Calles By:
-   Table Accessed :
-   Table Update :
-   Input  :参数一:
-
-   Output :
-   Return :0 正常 非零 返回相应的错误码(通过文档查找相应的错误描述)
-   Other :
***********************************************************************/
int set_usbshare_id_date(char *server_ip, int server_port, char *ter_id, char *product_date)
{
	SOCKET s_client;
	int result;
	cJSON *root = NULL;
	cJSON *head;
	result = socket_create_connect(&s_client, server_ip, server_port);
	if (result < 0)
	{
		WriteLogFile("set_usbshare_id_date create or connect socket fail!");
		return DLL_ERR_NEW_SHARE_SOCKET_CONNET;
	}

	root = cJSON_CreateObject();
	if (root == NULL)
	{
		shutdown(s_client, SD_BOTH);
		closesocket(s_client);
		return -1;
	}



	cJSON_AddItemToObject(root, "head", head = cJSON_CreateObject());
	cJSON_AddStringToObject(head, "protocol", "mb_usbshare_20191024");
	cJSON_AddStringToObject(head, "code_type", "request");
	cJSON_AddStringToObject(head, "cmd", "set_ip");
	cJSON_AddStringToObject(head, "source_topic", "");
	cJSON_AddStringToObject(head, "random", "987654321");
	cJSON_AddStringToObject(head, "result", "");
	cJSON *data;
	cJSON_AddItemToObject(root, "data", data = cJSON_CreateArray());
	cJSON *item;
	cJSON_AddItemToObject(data, "dira", item = cJSON_CreateObject());


	cJSON_AddStringToObject(item, "ter_id", ter_id);
	cJSON_AddStringToObject(item, "product_date", product_date);
	


	char *g_buf;
	char send_json[1024] = { 0 };
	int len;
	g_buf = cJSON_Print(root);

	memcpy(send_json, "\x01\x06\x80\x07", 4);
	send_json[4] = strlen(g_buf) / 0x1000000; send_json[5] = strlen(g_buf) / 0x10000;
	send_json[6] = strlen(g_buf) / 256; send_json[7] = strlen(g_buf) % 256;
	memcpy(send_json + 8, g_buf, strlen(g_buf));
	len = strlen(g_buf) + 8;
	free(g_buf);
	cJSON_Delete(root);

	result = send(s_client, send_json, len, 0);
	if (result < 0)
	{

		WriteLogFile("set_usbshare_ip_new send error!");
		shutdown(s_client, SD_BOTH);
		closesocket(s_client);
		return DLL_ERR_NEW_SHARE_SOCKET_SEND;
	}
	char recvhead[10] = { 0 };
	char recvbody[1024] = { 0 };
	long recvlen = 0;
	result = recv(s_client, recvhead, 4, 0);
	if (result < 0)
	{

		WriteLogFile("set_usbshare_ip_new recv head error!");
		shutdown(s_client, SD_BOTH);
		closesocket(s_client);
		return DLL_ERR_NEW_SHARE_SOCKET_RECV;
	}
	recvlen = recv_data_len(s_client);
	if (recvlen < 0)
	{

		WriteLogFile("set_usbshare_ip_new recv len error!");
		shutdown(s_client, SD_BOTH);
		closesocket(s_client);
		return DLL_ERR_NEW_SHARE_SOCKET_RECV;
	}
	result = recv(s_client, recvbody, recvlen, 0);
	if (result < 0)
	{

		WriteLogFile("set_usbshare_ip_new recv body error!");
		shutdown(s_client, SD_BOTH);
		closesocket(s_client);
		return DLL_ERR_NEW_SHARE_SOCKET_RECV;
	}
	result = parse_json_result(recvbody);



	shutdown(s_client, SD_BOTH);
	closesocket(s_client);
	return result;
}


static int ty_socket_read(int sock, unsigned char *buf, int buf_len)
{
	int i, result;
	for (i = 0; i<buf_len; i += result)
	{
		//WriteLogFile("in this \n");
		result = recv(sock, buf + i, buf_len - i, 0);
		//result = read(sock, buf + i, buf_len - i);
		if (result <= 0)
		{
			//out("read data time out or err \n");
			return result;
		}
	}
	return i;
}
/*********************************************************************
-   Function : get_usbshare_status_new
-   Description：查询机柜状态信息
-   Calls：
-   Calles By:
-   Table Accessed :
-   Table Update :
-   Input  :参数一:
-
-   Output :
-   Return :0 正常 非零 返回相应的错误码(通过文档查找相应的错误描述)
-   Other :
***********************************************************************/
int get_usbshare_status_new(char *server_ip, int server_port, char *out_ter_info)
{
	SOCKET s_client;
	int result;
	cJSON *root = NULL;
	cJSON *head;

	WriteLogFile("dev_info get_usbshare_status_new server_ip=%s,server_port=%d", server_ip,server_port);
	result = socket_create_connect(&s_client, server_ip, server_port);
	if (result < 0)
	{
		WriteLogFile("dev_info set_usbshare_id_date create or connect socket fail!");
		return DLL_ERR_NEW_SHARE_SOCKET_CONNET;
	}

	root = cJSON_CreateObject();
	if (root == NULL)
	{
		shutdown(s_client, SD_BOTH);
		closesocket(s_client);
		return -1;
	}



	cJSON_AddItemToObject(root, "head", head = cJSON_CreateObject());
	cJSON_AddStringToObject(head, "protocol", "mb_usbshare_20191024");
	cJSON_AddStringToObject(head, "code_type", "request");
	cJSON_AddStringToObject(head, "cmd", "ter_status");
	cJSON_AddStringToObject(head, "source_topic", "123");
	cJSON_AddStringToObject(head, "random", "987654321");
	cJSON_AddStringToObject(head, "result", "s");
	cJSON *data;
	cJSON_AddItemToObject(root, "data", data = cJSON_CreateArray());
	cJSON *item;
	cJSON_AddItemToObject(data, "dira", item = cJSON_CreateObject());




	char *g_buf;
	char send_json[1024] = { 0 };
	int len;
	g_buf = cJSON_Print(root);

	memcpy(send_json, "\x01\x06\x80\x07", 4);
	send_json[4] = strlen(g_buf) / 0x1000000; send_json[5] = strlen(g_buf) / 0x10000;
	send_json[6] = strlen(g_buf) / 256; send_json[7] = strlen(g_buf) % 256;
	memcpy(send_json + 8, g_buf, strlen(g_buf));
	len = strlen(g_buf) + 8;
	free(g_buf);
	cJSON_Delete(root);

	result = send(s_client, send_json, len, 0);
	if (result < 0)
	{

		WriteLogFile("dev_info set_usbshare_ip_new send error!");
		shutdown(s_client, SD_BOTH);
		closesocket(s_client);
		return DLL_ERR_NEW_SHARE_SOCKET_SEND;
	}
	char recvhead[10] = { 0 };
	unsigned char *recvbody = NULL;
	long recvlen = 0;
	int re_read = 0;
re_read:
	result = recv(s_client, recvhead, 4, 0);
	if (result < 0)
	{

		WriteLogFile("dev_info set_usbshare_ip_new recv head error!:result :%d ip:%s port:%d last_err:%d\n", result, server_ip, server_port, GetLastError());
		Sleep(1000);
		re_read++;
		if (re_read == 3)//30
		{
		
			shutdown(s_client, SD_BOTH);
			closesocket(s_client);
			return DLL_ERR_NEW_SHARE_SOCKET_RECV;
		}
		goto re_read;
		
	
		
	}
	recvlen = recv_data_len(s_client);
	if (recvlen < 0)
	{

		WriteLogFile("dev_info set_usbshare_ip_new recv len error!");
		shutdown(s_client, SD_BOTH);
		closesocket(s_client);
		return DLL_ERR_NEW_SHARE_SOCKET_RECV;
	}
	//WriteLogFile("recvbody malloc start\n");
	recvbody = (unsigned char*)malloc(recvlen + 100);
	if (recvbody == NULL)
	{
		WriteLogFile("dev_info err malloc recvbody\n");
		shutdown(s_client, SD_BOTH);
		closesocket(s_client);
		return DLL_ERR_NEW_SHARE_SOCKET_RECV;
	}
	memset(recvbody, 0, recvlen + 100);
	WriteLogFile("dev_info 接收json数据开始\n");
	result = ty_socket_read(s_client,recvbody, recvlen);
	if (result < 0)
	{

		free(recvbody);
		WriteLogFile("dev_info set_usbshare_ip_new recv body error!");
		shutdown(s_client, SD_BOTH);
		closesocket(s_client);
		return DLL_ERR_NEW_SHARE_SOCKET_RECV;
	}
	WriteLogFile("dev_info 接收json数据完成:re_read:%d\n", re_read);
	shutdown(s_client, SD_BOTH);
	closesocket(s_client);
	cJSON *rt = NULL;
	rt = cJSON_Parse(recvbody);
	if (rt == NULL)
	{
		WriteLogFile("dev_info set_usbshare_ip_new cJSON_Parse error!");
		free(recvbody);
		shutdown(s_client, SD_BOTH);
		closesocket(s_client);
		return DLL_ERR_NEW_SHARE_SOCKET_RECV;
	}
	char *noformat = NULL;
	noformat = cJSON_PrintUnformatted(rt);
	if (noformat == NULL)
	{
		WriteLogFile("dev_info set_usbshare_ip_new cJSON_PrintUnformatted error!");
		cJSON_Delete(rt);
		free(recvbody);
		shutdown(s_client, SD_BOTH);
		closesocket(s_client);
		return DLL_ERR_NEW_SHARE_SOCKET_RECV;
	}
	memcpy(out_ter_info, recvbody, strlen(recvbody));
	WriteLogFile("dev_info 接收到的包长为:%d!", strlen(out_ter_info));
	WriteLogFile("dev_info 接收到的完整包为:%s!", out_ter_info);
	cJSON_Delete(rt);
	free(noformat);
	free(recvbody);
	WriteLogFile("dev_info get_usbshare_status_new exit!!!!!\n");
	return 0;
}





int get_usbshare_status_new_time(char *server_ip, int server_port,char *intranet_ip ,char *external_ip, char *out_ter_info,int _delay_time)
{
	SOCKET s_client;
	int result,i = 0;
	cJSON *root = NULL;
	cJSON *head;
	struct file_setting file_setting[128] = {0};

	WriteLogFile("dev_info get_usbshare_status_new server_ip=%s,server_port=%d", server_ip,server_port);

	// result = read_setting(file_setting, 128);
	// if(result < 0)
	// {
	// 	WriteLogFile("read_setting failed");
	// }
	// for(i = 0;i < result;++i)
	// {
	// 	if(0 == memcmp(file_setting[i].terminal_ip,server_ip,strlen(server_ip)) && file_setting[i].data_port == server_port)
	// 	{
	// 		break;
	// 	}
	// }
	// if(i == result)
	// {
	// 	WriteLogFile("dev_info can not find mattched ip");
	// 	return -1;
	// }

	WriteLogFile("dev_info get_usbshare_status_new terminal_ip=%s,data_port=%d,remote_ip=%s,remote_data_port=%d", file_setting[i].terminal_ip,file_setting[i].data_port,file_setting[i].remote_ip,file_setting[i].remote_dataport);
	//result = socket_create_connect(&s_client, server_ip, server_port);

	if((s_client = socket_connect(server_ip,server_port,server_ip,server_port,3000)) == SOCKET_ERROR)
	{
		WriteLogFile("dev_info get_usbshare_status_new create or connect socket fail!");
		return DLL_ERR_NEW_SHARE_SOCKET_CONNET;
	}

	// if ((s_client = socket_connect(stp_cabinetinfo->ca_SettingIP, stp_cabinetinfo->port, remote_ip, remote_port,3000)) == SOCKET_ERROR){}
	// if (result < 0)
	// {
	// 	WriteLogFile("dev_info set_usbshare_id_date create or connect socket fail!");
	// 	return DLL_ERR_NEW_SHARE_SOCKET_CONNET;
	// }

	root = cJSON_CreateObject();
	if (root == NULL)
	{
		shutdown(s_client, SD_BOTH);
		closesocket(s_client);
		return -1;
	}



	cJSON_AddItemToObject(root, "head", head = cJSON_CreateObject());
	cJSON_AddStringToObject(head, "protocol", "mb_usbshare_20191024");
	cJSON_AddStringToObject(head, "code_type", "request");
	cJSON_AddStringToObject(head, "cmd", "ter_status");
	cJSON_AddStringToObject(head, "source_topic", "123");
	cJSON_AddStringToObject(head, "random", "9876543210");
	cJSON_AddStringToObject(head, "result", "s");
	cJSON *data;
	cJSON_AddItemToObject(root, "data", data = cJSON_CreateArray());
	cJSON *item;
	cJSON_AddItemToObject(data, "dira", item = cJSON_CreateObject());

	// char time_buffer[20]; 
	// memset(time_buffer,0,sizeof(time_buffer));
	// sprintf(time_buffer,"%d",_delay_time);
	// WriteLogFile("_delay_time =%s",time_buffer);
	// cJSON_AddStringToObject(item, "_delay_time", time_buffer);
	if(strlen(intranet_ip) > 0)
	{
		cJSON_AddStringToObject(item, "intranet_ip", intranet_ip);
	}

	if(strlen(external_ip) > 0)
	{
		cJSON_AddStringToObject(item, "external_ip", external_ip);
	}

	char *g_buf;
	char send_json[4*1024] = { 0 };
	int len;
	g_buf = cJSON_Print(root);

	memcpy(send_json, "\x01\x06\x80\x07", 4);
	send_json[4] = strlen(g_buf) / 0x1000000; send_json[5] = strlen(g_buf) / 0x10000;
	send_json[6] = strlen(g_buf) / 256; send_json[7] = strlen(g_buf) % 256;
	memcpy(send_json + 8, g_buf, strlen(g_buf));
	len = strlen(g_buf) + 8;
	free(g_buf);
	cJSON_Delete(root);

	result = send(s_client, send_json, len, 0);
	if (result < 0)
	{

		WriteLogFile("dev_info set_usbshare_ip_new send error!");
		shutdown(s_client, SD_BOTH);
		closesocket(s_client);
		return DLL_ERR_NEW_SHARE_SOCKET_SEND;
	}
	char recvhead[10] = { 0 };
	unsigned char *recvbody = NULL;
	long recvlen = 0;
	int re_read = 0;
re_read:
	result = recv(s_client, recvhead, 4, 0);
	if (result < 0)
	{

		WriteLogFile("dev_info set_usbshare_ip_new recv head error!:result :%d ip:%s port:%d last_err:%d\n", result, server_ip, server_port, GetLastError());
		Sleep(1000);
		re_read++;
		if (re_read == 30)
		{
		
			shutdown(s_client, SD_BOTH);
			closesocket(s_client);
			return DLL_ERR_NEW_SHARE_SOCKET_RECV;
		}
		goto re_read;
		
	
		
	}
	recvlen = recv_data_len(s_client);
	if (recvlen < 0)
	{

		WriteLogFile("dev_info set_usbshare_ip_new recv len error!");
		shutdown(s_client, SD_BOTH);
		closesocket(s_client);
		return DLL_ERR_NEW_SHARE_SOCKET_RECV;
	}
	//WriteLogFile("recvbody malloc start\n");
	recvbody = (unsigned char*)malloc(recvlen + 100);
	if (recvbody == NULL)
	{
		WriteLogFile("dev_info err malloc recvbody\n");
		shutdown(s_client, SD_BOTH);
		closesocket(s_client);
		return DLL_ERR_NEW_SHARE_SOCKET_RECV;
	}
	memset(recvbody, 0, recvlen + 100);
	WriteLogFile("dev_info 接收json数据开始\n");
	result = ty_socket_read(s_client,recvbody, recvlen);
	if (result < 0)
	{

		free(recvbody);
		WriteLogFile("dev_info set_usbshare_ip_new recv body error!");
		shutdown(s_client, SD_BOTH);
		closesocket(s_client);
		return DLL_ERR_NEW_SHARE_SOCKET_RECV;
	}
	WriteLogFile("dev_info 接收json数据完成:re_read:%d\n", re_read);
	shutdown(s_client, SD_BOTH);
	closesocket(s_client);
	cJSON *rt = NULL;
	rt = cJSON_Parse(recvbody);
	if (rt == NULL)
	{
		WriteLogFile("dev_info set_usbshare_ip_new cJSON_Parse error!");
		free(recvbody);
		shutdown(s_client, SD_BOTH);
		closesocket(s_client);
		return DLL_ERR_NEW_SHARE_SOCKET_RECV;
	}

	cJSON *object_data = cJSON_GetObjectItem(rt, "data");
	if (object_data == NULL)
	{
		WriteLogFile("请求Data失败\r\n");
		cJSON_Delete(rt);
		return DLL_ERR_NEW_SHARE_SOCKET_RECV;

	}    

	// g_buf = cJSON_Print(object_data);
	// WriteLogFile("___02_21___g_buf=%s\r\n",g_buf);


	char time_buffer[20]; 
	memset(time_buffer,0,sizeof(time_buffer));
	sprintf(time_buffer,"%d ms",_delay_time);
	cJSON *obj = cJSON_GetArrayItem(object_data, 0);		// 获取的数组里的obj
	// item = cJSON_GetObjectItem(obj, "intranet_delay");

	object_data = cJSON_GetObjectItem(obj, "eth_manage");
	if (object_data == NULL)
	{
		cJSON_Delete(rt);
		WriteLogFile("get eth_manage error!");
		return DLL_ERR_NEW_SHARE_SOCKET_RECV;
	}

	cJSON_ReplaceItemInObject(object_data, "intranet_delay", cJSON_CreateString(time_buffer));

	item = cJSON_GetObjectItem(object_data, "intranet_delay");
	if (item == NULL)
	{
		cJSON_Delete(rt);
		WriteLogFile("get intranet_delay error!");
		return DLL_ERR_NEW_SHARE_SOCKET_RECV;
	}
	WriteLogFile("___01_31__2_intranet_delay=%s\r\n",item->valuestring);

	g_buf = cJSON_Print(obj);
	WriteLogFile("___01_31___g_buf=%s\r\n",g_buf);



	char *noformat = NULL;
	noformat = cJSON_PrintUnformatted(rt);
	if (noformat == NULL)
	{
		WriteLogFile("dev_info set_usbshare_ip_new cJSON_PrintUnformatted error!");
		cJSON_Delete(rt);
		free(recvbody);
		shutdown(s_client, SD_BOTH);
		closesocket(s_client);
		return DLL_ERR_NEW_SHARE_SOCKET_RECV;
	}
	// memcpy(out_ter_info, recvbody, strlen(recvbody));
	memcpy(out_ter_info, noformat, strlen(noformat));
	WriteLogFile("dev_info 接收到的包长为:%d!", strlen(out_ter_info));
	WriteLogFile("dev_info 接收到的完整包为:%s!", out_ter_info);
	cJSON_Delete(rt);
	free(noformat);
	free(recvbody);
	WriteLogFile("dev_info get_usbshare_status_new exit!!!!!\n");
	return 0;
}









int set_usbshare_iptables(char *server_ip, int server_port, struct iptables_oper *iptb_oper, char *output)
{
	SOCKET s_client;
	int result;
	cJSON *root = NULL;
	cJSON *head;
	char order[500] = { 0 };
	result = socket_create_connect(&s_client, server_ip, server_port);
	if (result < 0)
	{
		WriteLogFile("set_usbshare_iptables create or connect socket fail!");
		return DLL_ERR_NEW_SHARE_SOCKET_CONNET;
	}

	root = cJSON_CreateObject();
	if (root == NULL)
	{
		shutdown(s_client, SD_BOTH);
		closesocket(s_client);
		return -1;
	}



	cJSON_AddItemToObject(root, "head", head = cJSON_CreateObject());
	cJSON_AddStringToObject(head, "protocol", "mb_usbshare_20191024");
	cJSON_AddStringToObject(head, "code_type", "request");
	cJSON_AddStringToObject(head, "cmd", "setup_iptables");
	cJSON_AddStringToObject(head, "source_topic", "123");
	cJSON_AddStringToObject(head, "random", "987654321");
	cJSON_AddStringToObject(head, "result", "s");
	cJSON *data;
	cJSON_AddItemToObject(root, "data", data = cJSON_CreateArray());
	cJSON *item;
	cJSON_AddItemToObject(data, "dira", item = cJSON_CreateObject());

	
	if (strlen(iptb_oper->order) != 0)
	{
		memcpy(order, iptb_oper->order, strlen(iptb_oper->order));
	}
	else
	{
		if (iptb_oper->drop_or_accpet == true)//打开端口权限
		{
			sprintf(order, "iptables -I INPUT -p tcp --dport %d -j ACCEPT", iptb_oper->oper_port);
		}
		else
			sprintf(order, "iptables -I INPUT -p tcp --dport %d -j DROP", iptb_oper->oper_port);
	}
	cJSON_AddStringToObject(item, "order", order);


	char *g_buf;
	char send_json[1024*2] = { 0 };
	int len;
	g_buf = cJSON_Print(root);

	WriteLogFile("set_usbshare_iptables g_buf:%s\n", g_buf);
	memcpy(send_json, "\x01\x06\x80\x07", 4);
	send_json[4] = strlen(g_buf) / 0x1000000; send_json[5] = strlen(g_buf) / 0x10000;
	send_json[6] = strlen(g_buf) / 256; send_json[7] = strlen(g_buf) % 256;
	memcpy(send_json + 8, g_buf, strlen(g_buf));
	len = strlen(g_buf) + 8;
	free(g_buf);
	cJSON_Delete(root);

	result = send(s_client, send_json, len, 0);
	if (result < 0)
	{

		WriteLogFile("set_usbshare_iptables send error!");
		shutdown(s_client, SD_BOTH);
		closesocket(s_client);
		return DLL_ERR_NEW_SHARE_SOCKET_SEND;
	}
	char recvhead[10] = { 0 };
	unsigned char *recvbody = NULL;
	long recvlen = 0;
	result = recv(s_client, recvhead, 4, 0);
	if (result < 0)
	{

		WriteLogFile("set_usbshare_iptables recv head error!");
		shutdown(s_client, SD_BOTH);
		closesocket(s_client);
		return DLL_ERR_NEW_SHARE_SOCKET_RECV;
	}
	recvlen = recv_data_len(s_client);
	if (recvlen < 0)
	{

		WriteLogFile("set_usbshare_iptables recv len error!");
		shutdown(s_client, SD_BOTH);
		closesocket(s_client);
		return DLL_ERR_NEW_SHARE_SOCKET_RECV;
	}
	WriteLogFile("recvbody malloc start\n");
	recvbody = (unsigned char*)malloc(recvlen + 100);
	if (recvbody == NULL)
	{
		WriteLogFile("err malloc recvbody\n");
		shutdown(s_client, SD_BOTH);
		closesocket(s_client);
		return DLL_ERR_NEW_SHARE_SOCKET_RECV;
	}
	memset(recvbody, 0, recvlen + 100);
	WriteLogFile("接收json数据开始\n");
	result = ty_socket_read(s_client, recvbody, recvlen);
	if (result < 0)
	{

		free(recvbody);
		WriteLogFile("set_usbshare_iptables recv body error!");
		shutdown(s_client, SD_BOTH);
		closesocket(s_client);
		return DLL_ERR_NEW_SHARE_SOCKET_RECV;
	}
	WriteLogFile("接收json数据完成\n");

	cJSON *rt = NULL;
	rt = cJSON_Parse(recvbody);
	if (rt == NULL)
	{
		WriteLogFile("set_usbshare_iptables cJSON_Parse error!");
		free(recvbody);
		shutdown(s_client, SD_BOTH);
		closesocket(s_client);
		return DLL_ERR_NEW_SHARE_SOCKET_RECV;
	}
	char *noformat = NULL;
	noformat = cJSON_PrintUnformatted(rt);
	if (noformat == NULL)
	{
		WriteLogFile("set_usbshare_iptables cJSON_PrintUnformatted error!");
		cJSON_Delete(rt);
		free(recvbody);
		shutdown(s_client, SD_BOTH);
		closesocket(s_client);
		return DLL_ERR_NEW_SHARE_SOCKET_RECV;
	}
	memcpy(output, noformat, strlen(noformat));
	WriteLogFile("接收到的完整包为:%s!", recvbody);
	cJSON_Delete(rt);
	free(noformat);
	free(recvbody);
	shutdown(s_client, SD_BOTH);
	closesocket(s_client);
	return 0;
}


int get_usbshare_net_delay_time_test(char *server_ip, int server_port,char *intranet_ip ,char *external_ip)
{
	//初始化
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("WSAStartup failed\n");
        return 1;
    }
    struct addrinfo *result = NULL, *ptr = NULL, hints;
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    int iResult = getaddrinfo(server_ip, "3240", &hints, &result);
    if (iResult != 0) {
        WriteLogFile("getaddrinfo failed: %d\n", iResult);
        WSACleanup();
        return 1;
    }


	SOCKET s_client;
	int result_2,i = 0;
	cJSON *root = NULL;
	cJSON *head;
	struct file_setting file_setting[128] = {0};

	WriteLogFile("dev_info get_usbshare_status_new server_ip=%s,server_port=%d", server_ip,server_port);
	WriteLogFile("dev_info get_usbshare_status_new terminal_ip=%s,data_port=%d,remote_ip=%s,remote_data_port=%d", file_setting[i].terminal_ip,file_setting[i].data_port,file_setting[i].remote_ip,file_setting[i].remote_dataport);

	if((s_client = socket_connect(server_ip,server_port,server_ip,server_port,3000)) == SOCKET_ERROR)
	{
		WriteLogFile("dev_info get_usbshare_status_new create or connect socket fail!");
		return DLL_ERR_NEW_SHARE_SOCKET_CONNET;
	}

	root = cJSON_CreateObject();
	if (root == NULL)
	{
		shutdown(s_client, SD_BOTH);
		closesocket(s_client);
		return -1;
	}

	cJSON_AddItemToObject(root, "head", head = cJSON_CreateObject());
	cJSON_AddStringToObject(head, "protocol", "mb_usbshare_20191024");
	cJSON_AddStringToObject(head, "code_type", "request");
	cJSON_AddStringToObject(head, "cmd", "check_net_delay");
	cJSON_AddStringToObject(head, "source_topic", "123");
	cJSON_AddStringToObject(head, "random", "987654321");
	cJSON_AddStringToObject(head, "result", "s");
	cJSON *data;
	cJSON_AddItemToObject(root, "data", data = cJSON_CreateArray());
	cJSON *item;
	cJSON_AddItemToObject(data, "dira", item = cJSON_CreateObject());

	char *g_buf;
	char send_json[4*1024] = { 0 };
	int len;
	g_buf = cJSON_Print(root);

	memcpy(send_json, "\x01\x06\x80\x07", 4);
	send_json[4] = strlen(g_buf) / 0x1000000; send_json[5] = strlen(g_buf) / 0x10000;
	send_json[6] = strlen(g_buf) / 256; send_json[7] = strlen(g_buf) % 256;
	memcpy(send_json + 8, g_buf, strlen(g_buf));
	len = strlen(g_buf) + 8;
	free(g_buf);
	cJSON_Delete(root);

	DWORD start = GetTickCount(); // 获取起始时间
	
	result_2 = send(s_client, send_json, len, 0);

	if (result_2 < 0)
	{
		WriteLogFile("dev_info check_net_delay send error!");
		shutdown(s_client, SD_BOTH);
		closesocket(s_client);
		return DLL_ERR_NEW_SHARE_SOCKET_SEND;
	}
	char recvhead[10] = { 0 };
	unsigned char *recvbody = NULL;
	long recvlen = 0;
	int re_read = 0;
re_read:
	result_2 = recv(s_client, recvhead, 4, 0);
	if (result_2 < 0)
	{

		WriteLogFile("dev_info check_net_delay recv head error!:result_2 :%d ip:%s port:%d last_err:%d\n", result_2, server_ip, server_port, GetLastError());
		Sleep(1000);
		re_read++;
		if (re_read == 30)
		{
		
			shutdown(s_client, SD_BOTH);
			closesocket(s_client);
			return DLL_ERR_NEW_SHARE_SOCKET_RECV;
		}
		goto re_read;
	}

	recvlen = recv_data_len(s_client);
	if (recvlen < 0)
	{

		WriteLogFile("dev_info check_net_delay recv len error!");
		shutdown(s_client, SD_BOTH);
		closesocket(s_client);
		return DLL_ERR_NEW_SHARE_SOCKET_RECV;
	}
	//WriteLogFile("recvbody malloc start\n");
	recvbody = (unsigned char*)malloc(recvlen + 100);
	if (recvbody == NULL)
	{
		WriteLogFile("dev_info err malloc recvbody\n");
		shutdown(s_client, SD_BOTH);
		closesocket(s_client);
		return DLL_ERR_NEW_SHARE_SOCKET_RECV;
	}
	memset(recvbody, 0, recvlen + 100);
	WriteLogFile("dev_info 接收json数据开始\n");
	result_2 = ty_socket_read(s_client,recvbody, recvlen);
	if (result_2 < 0)
	{

		free(recvbody);
		WriteLogFile("dev_info check_net_delay recv body error!");
		shutdown(s_client, SD_BOTH);
		closesocket(s_client);
		return DLL_ERR_NEW_SHARE_SOCKET_RECV;
	}
	WriteLogFile("dev_info 接收json数据完成:re_read:%d\n", re_read);

	DWORD end = GetTickCount(); // 获取结束时间
	int delay_tmie = end - start;
	WriteLogFile("经过的时间: %d 毫秒\n", delay_tmie);
	
	shutdown(s_client, SD_BOTH);
	closesocket(s_client);
	cJSON *rt = NULL;
	rt = cJSON_Parse(recvbody);
	if (rt == NULL)
	{
		WriteLogFile("dev_info check_net_delay cJSON_Parse error!");
		free(recvbody);
		shutdown(s_client, SD_BOTH);
		closesocket(s_client);
		return DLL_ERR_NEW_SHARE_SOCKET_RECV;
	}
	char *noformat = NULL;
	noformat = cJSON_PrintUnformatted(rt);
	if (noformat == NULL)
	{
		WriteLogFile("dev_info check_net_delay cJSON_PrintUnformatted error!");
		cJSON_Delete(rt);
		free(recvbody);
		shutdown(s_client, SD_BOTH);
		closesocket(s_client);
		return DLL_ERR_NEW_SHARE_SOCKET_RECV;
	}
	char out_ter_info[2048];
	memset(out_ter_info,0,sizeof(out_ter_info));
	memcpy(out_ter_info, recvbody, strlen(recvbody));
	WriteLogFile("dev_info 接收到的包长为:%d!", strlen(out_ter_info));
	WriteLogFile("dev_info 接收到的完整包为:%s!", out_ter_info);
	cJSON_Delete(rt);
	WriteLogFile("时间完成！！！\n");
	free(noformat);
	free(recvbody);
	WriteLogFile("dev_info get_usbshare_status_new exit!!!!!\n");
	
	return delay_tmie;


}

int get_want_time(char *server_ip, int server_port,char *intranet_ip ,char *external_ip)
{
	int time;
	WriteLogFile("get_want_time______________\n");
	time = get_usbshare_net_delay_time_test(server_ip, server_port,intranet_ip, external_ip);
	return time;
}



int get_upgrade_net_intranet(char *server_ip, int server_port,char *intranet_ip ,char *external_ip, char *out_ter_info, char *ter_id, int notice_type,char *url_buff)
{
	SOCKET s_client;
	int result,i = 0;
	cJSON *root = NULL;
	cJSON *head;
	struct file_setting file_setting[128] = {0};

	WriteLogFile("dev_info get_usbshare_status_new server_ip=%s,server_port=%d", server_ip,server_port);
	//WriteLogFile("dev_info get_usbshare_status_new terminal_ip=%s,data_port=%d,remote_ip=%s,remote_data_port=%d", file_setting[i].terminal_ip,file_setting[i].data_port,file_setting[i].remote_ip,file_setting[i].remote_dataport);
	//result = socket_create_connect(&s_client, server_ip, server_port);

	if((s_client = socket_connect(server_ip,server_port,server_ip,server_port,3000)) == SOCKET_ERROR)
	{
		WriteLogFile("dev_info get_usbshare_status_new create or connect socket fail!");
		return DLL_ERR_NEW_SHARE_SOCKET_CONNET;
	}

	// if ((s_client = socket_connect(stp_cabinetinfo->ca_SettingIP, stp_cabinetinfo->port, remote_ip, remote_port,3000)) == SOCKET_ERROR){}
	// if (result < 0)
	// {
	// 	WriteLogFile("dev_info set_usbshare_id_date create or connect socket fail!");
	// 	return DLL_ERR_NEW_SHARE_SOCKET_CONNET;
	// }

	root = cJSON_CreateObject();
	if (root == NULL)
	{
		shutdown(s_client, SD_BOTH);
		closesocket(s_client);
		return -1;
	}



	cJSON_AddItemToObject(root, "head", head = cJSON_CreateObject());
	cJSON_AddStringToObject(head, "protocol", "mb_usbshare_20191024");
	cJSON_AddStringToObject(head, "code_type", "request");
	cJSON_AddStringToObject(head, "cmd", "intranet_net_upgrade");
	cJSON_AddStringToObject(head, "source_topic", "123");
	cJSON_AddStringToObject(head, "random", "9876543210");
	cJSON_AddStringToObject(head, "result", "s");
	cJSON *data;
	cJSON_AddItemToObject(root, "data", data = cJSON_CreateArray());
	cJSON *item;
	cJSON_AddItemToObject(data, "dira", item = cJSON_CreateObject());

	if(strlen(intranet_ip) > 0)
	{
		cJSON_AddStringToObject(item, "intranet_ip", intranet_ip);
	}

	if(strlen(external_ip) > 0)
	{
		cJSON_AddStringToObject(item, "external_ip", external_ip);
	}



	// char time_buffer[20]; 
	// memset(time_buffer,0,sizeof(time_buffer));
	// sprintf(time_buffer,"%d",_delay_time);
	// WriteLogFile("_delay_time =%s",time_buffer);
	cJSON_AddStringToObject(item, "ter_id", ter_id);
	if(0 == notice_type)
	{
		cJSON_AddStringToObject(item, "notice_type", "auth");
		WriteLogFile("notice_type = 0   【auth】");
	}
	else if(1 == notice_type)
	{
		cJSON_AddStringToObject(item, "notice_type", "update");
		WriteLogFile("notice_type = 1   【update】");
	}
	else
		cJSON_AddStringToObject(item, "notice_type", "no_notice");

	cJSON_AddStringToObject(item, "url_buff", url_buff);


	char *g_buf;
	char send_json[4*1024] = { 0 };
	int len;
	g_buf = cJSON_Print(root);
	WriteLogFile("g_buf = %s",g_buf);

	memcpy(send_json, "\x01\x06\x80\x07", 4);
	send_json[4] = strlen(g_buf) / 0x1000000; send_json[5] = strlen(g_buf) / 0x10000;
	send_json[6] = strlen(g_buf) / 256; send_json[7] = strlen(g_buf) % 256;
	memcpy(send_json + 8, g_buf, strlen(g_buf));
	len = strlen(g_buf) + 8;
	free(g_buf);
	cJSON_Delete(root);

	result = send(s_client, send_json, len, 0);
	if (result < 0)
	{

		WriteLogFile("dev_info intranet_net_upgrade send error!");
		shutdown(s_client, SD_BOTH);
		closesocket(s_client);
		return DLL_ERR_NEW_SHARE_SOCKET_SEND;
	}
	char recvhead[10] = { 0 };
	unsigned char *recvbody = NULL;
	long recvlen = 0;
	int re_read = 0;
re_read:
	result = recv(s_client, recvhead, 4, 0);
	if (result < 0)
	{

		WriteLogFile("dev_info intranet_net_upgrade recv head error!:result :%d ip:%s port:%d last_err:%d\n", result, server_ip, server_port, GetLastError());
		Sleep(1000);
		re_read++;
		if (re_read == 30)
		{
		
			shutdown(s_client, SD_BOTH);
			closesocket(s_client);
			return DLL_ERR_NEW_SHARE_SOCKET_RECV;
		}
		goto re_read;
		
	
		
	}
	recvlen = recv_data_len(s_client);
	if (recvlen < 0)
	{

		WriteLogFile("dev_info intranet_net_upgrade recv len error!");
		shutdown(s_client, SD_BOTH);
		closesocket(s_client);
		return DLL_ERR_NEW_SHARE_SOCKET_RECV;
	}
	//WriteLogFile("recvbody malloc start\n");
	recvbody = (unsigned char*)malloc(recvlen + 100);
	if (recvbody == NULL)
	{
		WriteLogFile("dev_info err malloc recvbody\n");
		shutdown(s_client, SD_BOTH);
		closesocket(s_client);
		return DLL_ERR_NEW_SHARE_SOCKET_RECV;
	}
	memset(recvbody, 0, recvlen + 100);
	WriteLogFile("dev_info 接收json数据开始\n");
	result = ty_socket_read(s_client,recvbody, recvlen);
	if (result < 0)
	{

		free(recvbody);
		WriteLogFile("dev_info intranet_net_upgrade recv body error!");
		shutdown(s_client, SD_BOTH);
		closesocket(s_client);
		return DLL_ERR_NEW_SHARE_SOCKET_RECV;
	}
	WriteLogFile("dev_info 接收json数据完成:re_read:%d\n", re_read);
	shutdown(s_client, SD_BOTH);
	closesocket(s_client);
	cJSON *rt = NULL;
	rt = cJSON_Parse(recvbody);
	if (rt == NULL)
	{
		WriteLogFile("dev_info intranet_net_upgrade cJSON_Parse error!");
		free(recvbody);
		shutdown(s_client, SD_BOTH);
		closesocket(s_client);
		return DLL_ERR_NEW_SHARE_SOCKET_RECV;
	}
	char *noformat = NULL;
	noformat = cJSON_PrintUnformatted(rt);
	if (noformat == NULL)
	{
		WriteLogFile("dev_info intranet_net_upgrade cJSON_PrintUnformatted error!");
		cJSON_Delete(rt);
		free(recvbody);
		shutdown(s_client, SD_BOTH);
		closesocket(s_client);
		return DLL_ERR_NEW_SHARE_SOCKET_RECV;
	}
	memcpy(out_ter_info, recvbody, strlen(recvbody));
	WriteLogFile("dev_info 接收到的包长为:%d!", strlen(out_ter_info));
	WriteLogFile("dev_info 接收到的完整包为:%s!", out_ter_info);
	cJSON_Delete(rt);
	free(noformat);
	free(recvbody);
	WriteLogFile("dev_info get_usbshare_status_new exit!!!!!\n");
	return 0;
}



// int get_upgrade_net_intranet(char *server_ip, int server_port,char *intranet_ip ,char *external_ip, char *out_ter_info, char *ter_id, int notice_type,char *url_buff,int size)
// {
// 	SOCKET s_client;
// 	int result,i = 0;
// 	cJSON *root = NULL;
// 	cJSON *head;
// 	struct file_setting file_setting[128] = {0};

// 	char recvhead[10] = { 0 };
// 	unsigned char *recvbody = NULL;
// 	long recvlen = 0;
// 	int re_read = 0;
	
// 	// unsigned char *temp_out_data = NULL;
// 	char temp_out_data[1024] = { 0 };
// 	memset(temp_out_data,0,sizeof(temp_out_data));

// 	for (size_t k = 0; k < size; k++)
// 	{
		
// 		WriteLogFile("dev_info get_usbshare_status_new server_ip=%s,server_port=%d", server_ip,server_port);
// 		//WriteLogFile("dev_info get_usbshare_status_new terminal_ip=%s,data_port=%d,remote_ip=%s,remote_data_port=%d", file_setting[i].terminal_ip,file_setting[i].data_port,file_setting[i].remote_ip,file_setting[i].remote_dataport);
// 		//result = socket_create_connect(&s_client, server_ip, server_port);

// 		if((s_client = socket_connect(server_ip,server_port,server_ip,server_port,3000)) == SOCKET_ERROR)
// 		{
// 			WriteLogFile("dev_info get_usbshare_status_new create or connect socket fail!");
// 			return DLL_ERR_NEW_SHARE_SOCKET_CONNET;
// 		}

// 		// if ((s_client = socket_connect(stp_cabinetinfo->ca_SettingIP, stp_cabinetinfo->port, remote_ip, remote_port,3000)) == SOCKET_ERROR){}
// 		// if (result < 0)
// 		// {
// 		// 	WriteLogFile("dev_info set_usbshare_id_date create or connect socket fail!");
// 		// 	return DLL_ERR_NEW_SHARE_SOCKET_CONNET;
// 		// }

// 		root = cJSON_CreateObject();
// 		if (root == NULL)
// 		{
// 			shutdown(s_client, SD_BOTH);
// 			closesocket(s_client);
// 			return -1;
// 		}

// 		cJSON_AddItemToObject(root, "head", head = cJSON_CreateObject());
// 		cJSON_AddStringToObject(head, "protocol", "mb_usbshare_20191024");
// 		cJSON_AddStringToObject(head, "code_type", "request");
// 		cJSON_AddStringToObject(head, "cmd", "intranet_net_upgrade");
// 		cJSON_AddStringToObject(head, "source_topic", "123");
// 		cJSON_AddStringToObject(head, "random", "9876543210");
// 		cJSON_AddStringToObject(head, "result", "s");
// 		cJSON *data;
// 		cJSON_AddItemToObject(root, "data", data = cJSON_CreateArray());
// 		cJSON *item;
// 		cJSON_AddItemToObject(data, "dira", item = cJSON_CreateObject());

// 		if(strlen(intranet_ip) > 0)
// 		{
// 			cJSON_AddStringToObject(item, "intranet_ip", intranet_ip);
// 		}

// 		if(strlen(external_ip) > 0)
// 		{
// 			cJSON_AddStringToObject(item, "external_ip", external_ip);
// 		}

// 		// char time_buffer[20]; 
// 		// memset(time_buffer,0,sizeof(time_buffer));
// 		// sprintf(time_buffer,"%d",_delay_time);
// 		// WriteLogFile("_delay_time =%s",time_buffer);
// 		cJSON_AddStringToObject(item, "ter_id", ter_id);
// 		if(0 == notice_type)
// 		{
// 			cJSON_AddStringToObject(item, "notice_type", "auth");
// 			WriteLogFile("notice_type = 0   【auth】");
// 		}
// 		else if(1 == notice_type)
// 		{
// 			cJSON_AddStringToObject(item, "notice_type", "update");
// 			WriteLogFile("notice_type = 1   【update】");
// 		}
// 		else
// 			cJSON_AddStringToObject(item, "notice_type", "no_notice");

// 		cJSON_AddStringToObject(item, "url_buff", url_buff);


// 		char *g_buf;
// 		char send_json[4*1024] = { 0 };
// 		int len;
// 		g_buf = cJSON_Print(root);
// 		WriteLogFile("g_buf = %s",g_buf);

// 		memcpy(send_json, "\x01\x06\x80\x07", 4);
// 		send_json[4] = strlen(g_buf) / 0x1000000; send_json[5] = strlen(g_buf) / 0x10000;
// 		send_json[6] = strlen(g_buf) / 256; send_json[7] = strlen(g_buf) % 256;
// 		memcpy(send_json + 8, g_buf, strlen(g_buf));
// 		len = strlen(g_buf) + 8;
// 		free(g_buf);
// 		cJSON_Delete(root);

// 		result = send(s_client, send_json, len, 0);
// 		if (result < 0)
// 		{

// 			WriteLogFile("dev_info intranet_net_upgrade send error!");
// 			shutdown(s_client, SD_BOTH);
// 			closesocket(s_client);
// 			return DLL_ERR_NEW_SHARE_SOCKET_SEND;
// 		}

// 	re_read:
// 		memset(recvhead,0,sizeof(recvhead));
// 		result = recv(s_client, recvhead, 4, 0);
// 		if (result < 0)
// 		{

// 			WriteLogFile("dev_info intranet_net_upgrade recv head error!:result :%d ip:%s port:%d last_err:%d\n", result, server_ip, server_port, GetLastError());
// 			Sleep(1000);
// 			re_read++;
// 			if (re_read == 30)
// 			{
			
// 				shutdown(s_client, SD_BOTH);
// 				closesocket(s_client);
// 				return DLL_ERR_NEW_SHARE_SOCKET_RECV;
// 			}
// 			goto re_read;
			
		
			
// 		}
// 		recvlen = recv_data_len(s_client);
// 		if (recvlen < 0)
// 		{
// 			WriteLogFile("dev_info intranet_net_upgrade recv len error!");
// 			shutdown(s_client, SD_BOTH);
// 			closesocket(s_client);
// 			return DLL_ERR_NEW_SHARE_SOCKET_RECV;
// 		}
// 		//WriteLogFile("recvbody malloc start\n");
// 		recvbody = (unsigned char*)malloc(recvlen + 100);
// 		if (recvbody == NULL)
// 		{
// 			WriteLogFile("dev_info err malloc recvbody\n");
// 			shutdown(s_client, SD_BOTH);
// 			closesocket(s_client);
// 			return DLL_ERR_NEW_SHARE_SOCKET_RECV;
// 		}
// 		memset(recvbody, 0, recvlen + 100);
// 		WriteLogFile("dev_info 接收json数据开始\n");



// 		result = ty_socket_read(s_client,recvbody, recvlen);
// 		if (result < 0)
// 		{

// 			free(recvbody);
// 			WriteLogFile("dev_info intranet_net_upgrade recv body error!");
// 			shutdown(s_client, SD_BOTH);
// 			closesocket(s_client);
// 			return DLL_ERR_NEW_SHARE_SOCKET_RECV;
// 		}
// 		WriteLogFile("dev_info 接收json数据完成:re_read:%d\n", re_read);
// 		shutdown(s_client, SD_BOTH);
// 		closesocket(s_client);
// 		cJSON *rt = NULL;
// 		rt = cJSON_Parse(recvbody);
// 		if (rt == NULL)
// 		{
// 			WriteLogFile("dev_info intranet_net_upgrade cJSON_Parse error!");
// 			free(recvbody);
// 			shutdown(s_client, SD_BOTH);
// 			closesocket(s_client);
// 			return DLL_ERR_NEW_SHARE_SOCKET_RECV;
// 		}
// 		char *noformat = NULL;
// 		noformat = cJSON_PrintUnformatted(rt);
// 		if (noformat == NULL)
// 		{
// 			WriteLogFile("dev_info intranet_net_upgrade cJSON_PrintUnformatted error!");
// 			cJSON_Delete(rt);
// 			free(recvbody);
// 			shutdown(s_client, SD_BOTH);
// 			closesocket(s_client);
// 			return DLL_ERR_NEW_SHARE_SOCKET_RECV;
// 		}


// 		memcpy(out_ter_info, recvbody, strlen(recvbody));
// 		WriteLogFile("dev_info 接收到的包长为:%d!", strlen(out_ter_info));
// 		WriteLogFile("dev_info 接收到的完整包为:%s!", out_ter_info);


// 		strcat(temp_out_data, noformat);
// 		WriteLogFile("k= [%d] <<temp_out_data>>:%s",k, temp_out_data);

// 	}
// 	WriteLogFile("---------all end-----------<<temp_out_data>>:%s", temp_out_data);

// 	cJSON_Delete(rt);
// 	free(noformat);
// 	free(recvbody);
// 	WriteLogFile("dev_info get_usbshare_status_new exit!!!!!\n");
// 	return 0;





















// // 	WriteLogFile("dev_info get_usbshare_status_new server_ip=%s,server_port=%d", server_ip,server_port);
// // 	//WriteLogFile("dev_info get_usbshare_status_new terminal_ip=%s,data_port=%d,remote_ip=%s,remote_data_port=%d", file_setting[i].terminal_ip,file_setting[i].data_port,file_setting[i].remote_ip,file_setting[i].remote_dataport);
// // 	//result = socket_create_connect(&s_client, server_ip, server_port);

// // 	if((s_client = socket_connect(server_ip,server_port,server_ip,server_port,3000)) == SOCKET_ERROR)
// // 	{
// // 		WriteLogFile("dev_info get_usbshare_status_new create or connect socket fail!");
// // 		return DLL_ERR_NEW_SHARE_SOCKET_CONNET;
// // 	}

// // 	// if ((s_client = socket_connect(stp_cabinetinfo->ca_SettingIP, stp_cabinetinfo->port, remote_ip, remote_port,3000)) == SOCKET_ERROR){}
// // 	// if (result < 0)
// // 	// {
// // 	// 	WriteLogFile("dev_info set_usbshare_id_date create or connect socket fail!");
// // 	// 	return DLL_ERR_NEW_SHARE_SOCKET_CONNET;
// // 	// }

// // 	root = cJSON_CreateObject();
// // 	if (root == NULL)
// // 	{
// // 		shutdown(s_client, SD_BOTH);
// // 		closesocket(s_client);
// // 		return -1;
// // 	}



// // 	cJSON_AddItemToObject(root, "head", head = cJSON_CreateObject());
// // 	cJSON_AddStringToObject(head, "protocol", "mb_usbshare_20191024");
// // 	cJSON_AddStringToObject(head, "code_type", "request");
// // 	cJSON_AddStringToObject(head, "cmd", "intranet_net_upgrade");
// // 	cJSON_AddStringToObject(head, "source_topic", "123");
// // 	cJSON_AddStringToObject(head, "random", "9876543210");
// // 	cJSON_AddStringToObject(head, "result", "s");
// // 	cJSON *data;
// // 	cJSON_AddItemToObject(root, "data", data = cJSON_CreateArray());
// // 	cJSON *item;
// // 	cJSON_AddItemToObject(data, "dira", item = cJSON_CreateObject());

// // 	if(strlen(intranet_ip) > 0)
// // 	{
// // 		cJSON_AddStringToObject(item, "intranet_ip", intranet_ip);
// // 	}

// // 	if(strlen(external_ip) > 0)
// // 	{
// // 		cJSON_AddStringToObject(item, "external_ip", external_ip);
// // 	}



// // 	// char time_buffer[20]; 
// // 	// memset(time_buffer,0,sizeof(time_buffer));
// // 	// sprintf(time_buffer,"%d",_delay_time);
// // 	// WriteLogFile("_delay_time =%s",time_buffer);
// // 	cJSON_AddStringToObject(item, "ter_id", ter_id);
// // 	if(0 == notice_type)
// // 	{
// // 		cJSON_AddStringToObject(item, "notice_type", "auth");
// // 		WriteLogFile("notice_type = 0   【auth】");
// // 	}
// // 	else if(1 == notice_type)
// // 	{
// // 		cJSON_AddStringToObject(item, "notice_type", "update");
// // 		WriteLogFile("notice_type = 1   【update】");
// // 	}
// // 	else
// // 		cJSON_AddStringToObject(item, "notice_type", "no_notice");

// // 	cJSON_AddStringToObject(item, "url_buff", url_buff);


// // 	char *g_buf;
// // 	char send_json[4*1024] = { 0 };
// // 	int len;
// // 	g_buf = cJSON_Print(root);
// // 	WriteLogFile("g_buf = %s",g_buf);

// // 	memcpy(send_json, "\x01\x06\x80\x07", 4);
// // 	send_json[4] = strlen(g_buf) / 0x1000000; send_json[5] = strlen(g_buf) / 0x10000;
// // 	send_json[6] = strlen(g_buf) / 256; send_json[7] = strlen(g_buf) % 256;
// // 	memcpy(send_json + 8, g_buf, strlen(g_buf));
// // 	len = strlen(g_buf) + 8;
// // 	free(g_buf);
// // 	cJSON_Delete(root);

// // 	result = send(s_client, send_json, len, 0);
// // 	if (result < 0)
// // 	{

// // 		WriteLogFile("dev_info intranet_net_upgrade send error!");
// // 		shutdown(s_client, SD_BOTH);
// // 		closesocket(s_client);
// // 		return DLL_ERR_NEW_SHARE_SOCKET_SEND;
// // 	}






// // 	char recvhead[10] = { 0 };
// // 	unsigned char *recvbody = NULL;
// // 	long recvlen = 0;
// // 	int re_read = 0;
// // re_read:
// // 	result = recv(s_client, recvhead, 4, 0);
// // 	if (result < 0)
// // 	{

// // 		WriteLogFile("dev_info intranet_net_upgrade recv head error!:result :%d ip:%s port:%d last_err:%d\n", result, server_ip, server_port, GetLastError());
// // 		Sleep(1000);
// // 		re_read++;
// // 		if (re_read == 30)
// // 		{
		
// // 			shutdown(s_client, SD_BOTH);
// // 			closesocket(s_client);
// // 			return DLL_ERR_NEW_SHARE_SOCKET_RECV;
// // 		}
// // 		goto re_read;
		
	
		
// // 	}
// // 	recvlen = recv_data_len(s_client);
// // 	if (recvlen < 0)
// // 	{

// // 		WriteLogFile("dev_info intranet_net_upgrade recv len error!");
// // 		shutdown(s_client, SD_BOTH);
// // 		closesocket(s_client);
// // 		return DLL_ERR_NEW_SHARE_SOCKET_RECV;
// // 	}
// // 	//WriteLogFile("recvbody malloc start\n");
// // 	recvbody = (unsigned char*)malloc(recvlen + 100);
// // 	if (recvbody == NULL)
// // 	{
// // 		WriteLogFile("dev_info err malloc recvbody\n");
// // 		shutdown(s_client, SD_BOTH);
// // 		closesocket(s_client);
// // 		return DLL_ERR_NEW_SHARE_SOCKET_RECV;
// // 	}
// // 	memset(recvbody, 0, recvlen + 100);
// // 	WriteLogFile("dev_info 接收json数据开始\n");











// // 	result = ty_socket_read(s_client,recvbody, recvlen);
// // 	if (result < 0)
// // 	{

// // 		free(recvbody);
// // 		WriteLogFile("dev_info intranet_net_upgrade recv body error!");
// // 		shutdown(s_client, SD_BOTH);
// // 		closesocket(s_client);
// // 		return DLL_ERR_NEW_SHARE_SOCKET_RECV;
// // 	}
// // 	WriteLogFile("dev_info 接收json数据完成:re_read:%d\n", re_read);
// // 	shutdown(s_client, SD_BOTH);
// // 	closesocket(s_client);
// // 	cJSON *rt = NULL;
// // 	rt = cJSON_Parse(recvbody);
// // 	if (rt == NULL)
// // 	{
// // 		WriteLogFile("dev_info intranet_net_upgrade cJSON_Parse error!");
// // 		free(recvbody);
// // 		shutdown(s_client, SD_BOTH);
// // 		closesocket(s_client);
// // 		return DLL_ERR_NEW_SHARE_SOCKET_RECV;
// // 	}
// // 	char *noformat = NULL;
// // 	noformat = cJSON_PrintUnformatted(rt);
// // 	if (noformat == NULL)
// // 	{
// // 		WriteLogFile("dev_info intranet_net_upgrade cJSON_PrintUnformatted error!");
// // 		cJSON_Delete(rt);
// // 		free(recvbody);
// // 		shutdown(s_client, SD_BOTH);
// // 		closesocket(s_client);
// // 		return DLL_ERR_NEW_SHARE_SOCKET_RECV;
// // 	}



// // 	memcpy(out_ter_info, recvbody, strlen(recvbody));
// // 	WriteLogFile("dev_info 接收到的包长为:%d!", strlen(out_ter_info));
// // 	WriteLogFile("dev_info 接收到的完整包为:%s!", out_ter_info);


// // 	cJSON_Delete(rt);
// // 	free(noformat);
// // 	free(recvbody);
// // 	WriteLogFile("dev_info get_usbshare_status_new exit!!!!!\n");
// // 	return 0;
// }