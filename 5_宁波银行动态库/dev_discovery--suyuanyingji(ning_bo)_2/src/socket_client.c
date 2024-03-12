#define DF_SOCKET_CLIENT_C
#include "socket_client.h"
#include "client_portocol.h"
#include <Windows.h>
//#include "log.h"
#include "Macro.h"

//#define WriteLogFile(format,...)		log_out(format , ##__VA_ARGS__ )


#define CLIENT_SOCKET_TIMEOUT   10000


#ifdef NINGBO_BANK

/**
*@desc:sockets5代理C语言实现
*@parm_in:sockfd:connect连接服务器后的socket套接字	ip:socket5报文头中要绑定的目的IP; port:sockets5报文头中要绑定的端口 flag:测试用
*@parm_out:NULL
*@return:
*/
int deal_sockets5(SOCKET sockfd, unsigned char *ip,int port,int flag)
{
	unsigned char init_send[4] = { 0 };
	unsigned char init_recv[3] = { 0 };
	unsigned char bind_pack[100] = { 0 };
	unsigned char bind_recv[140] = { 0 };
	memcpy(init_send, "\x05\x01\x00", 3);
	//char ip[] = {"192.168.0.65"};
	//int port = 10001;
	//发送05 01 00的初始化包
	//return 0;
	if (socket_send(sockfd, init_send, 3) != 3)
	{
		WriteLogFile("deal_sockets5 send init packet err\n");
		goto ERR;
	}
	//接收应答包
	if (socket_recv(sockfd, init_recv, 2) != 2)
	{
		WriteLogFile("deal_sockets5 recv init packet err\n");
		goto ERR;
	}
	//判断应答结果
	if (memcmp(init_recv, "\x05\x00", 2) != 0)
	{
		WriteLogFile("deal_sockets5 recv init packet not success\n");
		goto ERR;
	}
	memcpy(bind_pack, "\x05\x01\x00\x01", 4);
	int ip1, ip2, ip3, ip4;
	sscanf(ip, "%d.%d.%d.%d", &ip1, &ip2, &ip3, &ip4);
	bind_pack[4] = ip1;
	bind_pack[5] = ip2;
	bind_pack[6] = ip3;
	bind_pack[7] = ip4;
	bind_pack[8] = port / 256;
	bind_pack[9] = port % 256;
	/*if (flag == 1)
		memcpy(bind_pack + 4, "\xB7\x88\xC6\x09\x27\x11", 6);
	else
		memcpy(bind_pack + 4, "\xB7\x88\xC6\x09\x0C\xA8", 6);*/
	
	//unsigned char log_str[200] = { 0 };

	//hex2str(bind_pack, 10, log_str);
	//WriteLogFile("deal_sockets5 send bind packet :\n%s\n", log_str);
	//发送绑定了IP PORT的链接包
	if (socket_send(sockfd, bind_pack, 10) != 10)
	{
		WriteLogFile("deal_sockets5 send bind packet err\n");
		goto ERR;
	}

	//接收应答并判断是否成功且判断返回ip为什么类型进一步的读取
	socket_recv(sockfd, bind_recv, 4);

	if (memcmp(bind_recv, "\x05\x00", 2) != 0)
	{
		WriteLogFile("deal_sockets5 recv bind packet sucess but the result is not right,ip=%s,port=%d,bind_recv[0]=%02x,bind_recv[1]=%02x,bind_recv[2]=%02x,bind_recv[3]=%02x\n",ip,port,bind_recv[0],bind_recv[1],bind_recv[2],bind_recv[3]);
		goto ERR;
	}
	int addr_type = bind_recv[3];
	
	switch (addr_type)
	{
	case 1://IPV4
		WriteLogFile("sockets5 server response addr type is ipv4");
		socket_recv(sockfd, bind_recv+4, 6);
		break;
	case 3://domain 
		WriteLogFile("sockets5 server response addr type is domain");
		socket_recv(sockfd, bind_recv + 4, 1);
		int len = bind_recv[4];
		socket_recv(sockfd, bind_recv + 5, len+2);
		break;
	case 4://IPV6
		WriteLogFile("sockets5 server response addr type is ipv6");
		socket_recv(sockfd, bind_recv + 4, 18);
		break;
	default:
		break;
	}
	//下面是通过select方式一个字节一个字节接收完所有的应答包，为非常规手段
	/*struct timeval tmOut;
	tmOut.tv_sec = 0;
	tmOut.tv_usec = 0;
	fd_set fds;
	FD_ZERO(&fds);
	FD_SET(sockfd, &fds);
	int nRet;
	char tmp[2];
	memset(tmp, 0, sizeof(tmp));
	while (1)
	{
		WriteLogFile("still have data!");
		nRet = select(FD_SETSIZE, &fds, NULL, NULL, &tmOut);
		if (nRet == 0)
			break;
		recv(sockfd, tmp, 1, 0);
	}*/
	return 0;
ERR:
	socket_close(sockfd);
	return DF_ERR_SOCKET_ERROR;
}
int read_sockets5_remote_ip_port(char *local_ip, int local_port, int local_dataport, char *remote_ip, int remote_port, int remote_port_type)
{
	char set_path[MAX_PATH] = { 0 }, *c_DestPath = NULL, ip_asc[100] = { 0 }, buf_prefix[100];
	//struct hostent *h;
	int port, dataport;
	if (!GetModuleFileName(NULL, set_path, sizeof(set_path))){
		WriteLogFile("获取当前路径错误");
		return -1;
	}
	*(strrchr(set_path, '\\') + 1) = 0;
	c_DestPath = strcat(set_path, "sockets5_ip_pair.set");
	if (c_DestPath == NULL){
		return -1;
	}
	/*判断文件是否存在 如果不存在创建配置文件并初始化, 否则读取配置文件*/
	if (access(set_path, 0)){
		FILE *fp = fopen(set_path, "wb+");
		if (fp == NULL){
			return -1;
		}
	}
	else{
			memset(ip_asc, 0, sizeof(ip_asc));
			memset(buf_prefix, 0, sizeof(buf_prefix));
			sprintf(buf_prefix, "Nginx_ip_%s_port1_%d_port2_%d", local_ip, local_port, local_dataport);
			GetPrivateProfileString(TEXT(buf_prefix), TEXT("IP"), TEXT(""), ip_asc, 20, TEXT(set_path));//设备IP地址
			port = GetPrivateProfileInt(TEXT(buf_prefix), TEXT("Port"), 0, TEXT(set_path));
			dataport = GetPrivateProfileInt(TEXT(buf_prefix), TEXT("DataPort"), 0, TEXT(set_path));
			if (remote_port_type == 1)
			{
				memcpy(remote_ip, ip_asc, strlen(ip_asc));
				remote_port = port;
			}
			else
			{
				memcpy(remote_ip, ip_asc, strlen(ip_asc));
				remote_port = dataport;
			}
			WriteLogFile("get the ip is:%s \nport is:%d  \n dataport is :%d\n", ip_asc, port, dataport);
		
	}
	return 0;

}
#endif
/*********************************************************************
-   Function : socket_connect
-   Description：socket连接
-   Calls：
-   Calles By:
-   Table Accessed :
-   Table Update :
-   Input  :参数一: IP地址
			参数二：端口号
			参数三：超时时间
-   Output :
-   Return :
-   Other :
***********************************************************************/
SOCKET socket_connect(char *ip, int port,char *remote_ip,int remote_port, int timeout_ms)
{
// 	int ret = 0;
// 	struct sockaddr_in sin;
// 	int iMode = 1, nRecvBuf = 0, nSendBuf = 0, reuse0 =1;
// 	struct timeval tm;
// 	BOOL bDontLinger = FALSE;
// 	struct timeval timeout;
// 	SOCKET clifd;
// 	WriteLogFile("socket connect 链接ip :%s  端口:%d", remote_ip, remote_port);
// 	tm.tv_sec = 2;
// 	tm.tv_usec = 0;
// 	timeout.tv_sec = 10000;
// 	sin.sin_addr.S_un.S_addr = inet_addr(remote_ip);
// 	sin.sin_family = AF_INET;
// 	sin.sin_port = htons(remote_port);
// 	clifd = socket(AF_INET, SOCK_STREAM, 0);
// 	if (clifd == SOCKET_ERROR){
// 		WriteLogFile("socket创建失败,GetLastError = %d\n" , GetLastError());
// 		return SOCKET_ERROR;
// 	}
// 	setsockopt(clifd, SOL_SOCKET, SO_RCVBUF, (const char*)&nRecvBuf, sizeof(int));
// 	setsockopt(clifd, SOL_SOCKET, SO_SNDBUF, (const char*)&nSendBuf, sizeof(int));
// 	setsockopt(clifd, SOL_SOCKET, SO_DONTLINGER, (const char*)&bDontLinger, sizeof(BOOL));
// 	setsockopt(clifd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout.tv_sec, sizeof(timeout));
// 	setsockopt(clifd, SOL_SOCKET, SO_SNDTIMEO, (const char*)&timeout.tv_sec, sizeof(timeout));

// 	//if (setsockopt(clifd, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse0, sizeof(reuse0)) == -1) return SOCKET_ERROR;

// 	iMode = 1;
// 	ioctlsocket(clifd, FIONBIO, (u_long FAR*)&iMode); //设置为非阻塞模式

// 	if (memcmp(ip, remote_ip, strlen(remote_ip)) == 0) //ip和remoteip一致时，直接使用remoteip连接
// 	{
// 		if (connect(clifd, (struct sockaddr*)&sin, sizeof(sin)) != SOCKET_ERROR)
// 		{
// 			WriteLogFile("connect success!!!!!!!!!!!!!\n");
// 			return clifd;
// 		}
// 		else
// 		{
// 			WriteLogFile("connect failed!!!!!!!!!!!!!\n");
// 			return -1;
// 		}
// 	}
// 	else
// 	{
// 		if (connect(clifd, (struct sockaddr*)&sin, sizeof(sin)) != SOCKET_ERROR){
// 		WriteLogFile("11111connect success!!!!!!!!!!!!!\n");
// 		//return clifd;
// 		}
// 		else{
// 			fd_set set;
// 			FD_ZERO(&set);
// 			FD_SET(clifd, &set);

// 			if (select(-1, NULL, &set, NULL, &tm) <= 0)
// 			{
// 				ret = GetLastError();
// 				ret = -1; // 有错误(select错误或者超时)
// 				socket_close(clifd);
// 				WriteLogFile("ip :%s  端口:%d connect err!\n", remote_ip, remote_port);
// 				return -1;
// 			}
// 			else
// 			{
// 				int error = -1;
// 				int optLen = sizeof(int);
// 				getsockopt(clifd, SOL_SOCKET, SO_ERROR, (char*)&error, &optLen);

// 				if (0 != error){
// 					ret = -1; // 有错误  
// 					socket_close(clifd);

// 					WriteLogFile("ip :%s  端口:%d connect err!\n", remote_ip, remote_port);
// 					return -1;
// 				}
// 				else{
// 					ret = 1;  // 无错误  
// 				}
// 			}
// 		}
// 		iMode = 0;
// 		ioctlsocket(clifd, FIONBIO, (u_long FAR*)&iMode); //设置为阻塞模式
		
// #ifdef NINGBO_BANK
// 		WriteLogFile("start deal_sockets5 port cmd port,ip :%s  端口:%d",ip, port);
// 		ret = deal_sockets5(clifd, ip, port, 1);
// 		if (ret != 0)
// 		{
// 			WriteLogFile("finish deal_sockets5 port cmd port err");
// 			return -1;
// 		}
// 		WriteLogFile("finish deal_sockets5 port cmd port success,ip :%s  端口:%d",ip, port);
// #endif
// 	}

	

// 	return clifd;



	int ret = 0;
	struct sockaddr_in sin;
	int iMode = 1, nRecvBuf = 0, nSendBuf = 0, reuse0 =1;
	struct timeval tm;
	BOOL bDontLinger = FALSE;
	struct timeval timeout;
	SOCKET clifd;
	//WriteLogFile("socket connect 链接ip :%s  端口:%d", remote_ip, remote_port);
	tm.tv_sec = 2;
	tm.tv_usec = 0;
	timeout.tv_sec = 10000;
	sin.sin_addr.S_un.S_addr = inet_addr(remote_ip);
	sin.sin_family = AF_INET;
	sin.sin_port = htons(remote_port);
	clifd = socket(AF_INET, SOCK_STREAM, 0);
	if (clifd == SOCKET_ERROR){
		WriteLogFile("socket创建失败,GetLastError = %d\n" , GetLastError());
		return SOCKET_ERROR;
	}
	setsockopt(clifd, SOL_SOCKET, SO_RCVBUF, (const char*)&nRecvBuf, sizeof(int));
	setsockopt(clifd, SOL_SOCKET, SO_SNDBUF, (const char*)&nSendBuf, sizeof(int));
	setsockopt(clifd, SOL_SOCKET, SO_DONTLINGER, (const char*)&bDontLinger, sizeof(BOOL));
	setsockopt(clifd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout.tv_sec, sizeof(timeout));
	setsockopt(clifd, SOL_SOCKET, SO_SNDTIMEO, (const char*)&timeout.tv_sec, sizeof(timeout));

	//if (setsockopt(clifd, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse0, sizeof(reuse0)) == -1) return SOCKET_ERROR;

	iMode = 1;
	ioctlsocket(clifd, FIONBIO, (u_long FAR*)&iMode); //设置为非阻塞模式

	if (connect(clifd, (struct sockaddr*)&sin, sizeof(sin)) != SOCKET_ERROR){
		WriteLogFile("connect success!!!!!!!!!!!!!\n");
		return clifd;
	}
	else{
		fd_set set;
		FD_ZERO(&set);
		FD_SET(clifd, &set);

		if (select(-1, NULL, &set, NULL, &tm) <= 0)
		{
			ret = GetLastError();
			ret = -1; // 有错误(select错误或者超时)
			// socket_close(clifd);
			shutdown(clifd, SD_BOTH);
			closesocket(clifd);
			WriteLogFile("ip :%s  端口:%d connect err!\n", remote_ip, remote_port);
			return -1;
		}
		else
		{
			int error = -1;
			int optLen = sizeof(int);
			getsockopt(clifd, SOL_SOCKET, SO_ERROR, (char*)&error, &optLen);

			if (0 != error){
				ret = -1; // 有错误  
				// socket_close(clifd);
				shutdown(clifd, SD_BOTH);
				closesocket(clifd);
				WriteLogFile("ip :%s  端口:%d connect err!\n", remote_ip, remote_port);
				return -1;
			}
			else{
				ret = 1;  // 无错误  
			}
		}
	}
	iMode = 0;
	ioctlsocket(clifd, FIONBIO, (u_long FAR*)&iMode); //设置为阻塞模式

#ifdef NINGBO_BANK
	if (memcmp(ip, remote_ip, strlen(remote_ip)) == 0)
	{
		return clifd;
	}
	//WriteLogFile("start deal_sockets5 port cmd port,ip :%s  端口:%d",ip, port);
	ret = deal_sockets5(clifd, ip, port, 1);
	if (ret != 0)
	{
		WriteLogFile("finish deal_sockets5 port cmd port err");
		return -1;
	}
	//WriteLogFile("finish deal_sockets5 port cmd port success,ip :%s  端口:%d",ip, port);
#endif
	return clifd;
}


int socket_send(SOCKET socketfd , uint8 *buf,int len)
{

	int i_AllSendLen = len , i_SendedLen = 0;
	int i = 0;
	while (i_SendedLen < i_AllSendLen){
		if ((i = send(socketfd, buf + i_SendedLen, len - i_SendedLen, 0)) == SOCKET_ERROR){
			return -1;
		}
		i_SendedLen += i;
		Sleep(10);
	}

	
	return len;
}



int socket_recv(SOCKET sockfd, uint8 *buf,int len)
{
	int recv_len = 0, totoal = 0;

	if (!len)
		return -1;

	do{
		recv_len = recv(sockfd, buf, len, 0);

		//<=-1 错误码判断
		if (recv_len <= SOCKET_ERROR){
			/*if (errno == WSAEINTR || errno == WSAEWOULDBLOCK || errno == EAGAIN)*/
			return -1;
		}
			
		if (recv_len == 0)
			return 0;
		buf = buf + recv_len;
		len -= recv_len;
		totoal += recv_len;
		Sleep(10);
	} while (len > 0);

	return totoal;
}



void socket_close(SOCKET socketfd)
{
	shutdown(socketfd, SD_BOTH);
	closesocket(socketfd);
}