#include "dev_main.h"
#include "manage_set_file.h"
#include "terminal_manage.h"




BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)                                    //DLL主函数入口，暂时默认不动
{
	switch (fdwReason)
	{
	case DLL_PROCESS_ATTACH:
	{
			break;
	}
	case DLL_PROCESS_DETACH:
	{
			break;
	}
	case DLL_THREAD_ATTACH:
	{
			break;
	}
	case DLL_THREAD_DETACH:
	{
			break;
	}
	}

	/* Return TRUE on success, FALSE on failure */
	return TRUE;
}

/*********************************************************************
-   Function : main_thread
-   Description：主线程，负责循环的进行udp广播查找局域网设备
-   Calls：
-   Calles By:
-   Table Accessed :
-   Table Update :
-   Input  :
-   Output :
-   Return :0 正常 非零 返回相应的错误码(通过文档查找相应的错误描述)
-   Other :
***********************************************************************/
DWORD WINAPI main_thread(LPVOID lpParam)
{
	// int result, i;
	// char dev_info_tmp[1024*10];
	// already_found = 0;
	// int not_found_count = 0;
	while (api_nThreadFlag)
	{
		
		// for (i = 0; i < MAX_NUM; i++)
		// {
		// 	memset(&dev_info_old[i], 0, sizeof(struct dev_info));
		// 	memset(&dev_info_new[i], 0, sizeof(struct dev_info));
		// }
		// memset(dev_info_tmp, 0, sizeof(dev_info_tmp));
		// WriteLogFile("in dev_discovery time:%d", time_loop);
		// result = dev_discovery(dev_info_tmp);
		// WriteLogFile("out dev_discovery  result:%d ", result);
		// if (result == 0)
		// {
		// 	if ((strlen(dev_info_tmp) < 40 && (strlen(dev_info_tmp) != strlen(dev_info_back))) && not_found_count < 3)//标示一台机柜都没搜到,但上次是有搜索到机柜的
		// 	{
		// 		not_found_count++;
		// 		WriteLogFile("out dev_discovery not_found_count! ");
		// 		continue;
		// 	}
		// 	not_found_count = 0;
		// 	WaitForSingleObject(main_mutex, INFINITE);
		// 	memset(dev_info_back, 0, sizeof(dev_info_back));
		// 	memcpy(dev_info_back, dev_info_tmp, strlen(dev_info_tmp));
		// 	ReleaseMutex(main_mutex);
		// 	already_found = 1;
		// }
		//WriteLogFile("&&&&&&&&&&&&&&&&&&&&&&&&&&&&&lbc dev_info heart");
		Sleep(1 * 1000);
		
	}
	api_nThreadFlag = 2;
	return 0;
}



/*********************************************************************
-   Function : init_dll_interface
-   Description：初始化本dll中一些全局、公共资源和主线程监听服务等
-   Calls：
-   Calles By:
-   Table Accessed :
-   Table Update :
-   Input  :
-   Output :
-   Return :0 正常 非零 返回相应的错误码(通过文档查找相应的错误描述)
-   Other :
***********************************************************************/
EXPORT_DLL int init_dll_interface(int time)
{
	int ret = 0;
	int i = 0;
	unsigned short version = 0x202; /* winsock 2.2 */
	WSADATA data;
	// 获取配置文件全路径(包括文件名)
	//_snprintf((char*)g_szLogFile, sizeof(g_szLogFile)-1, "C:\\WriteLog.log");

	WriteLogFile("dll_info init_dll_interface");
	ret = WSAStartup(version, &data);
	if (ret)
	{
		ret = GetLastError();
		WriteLogFile("WSAStartup err");
		return ret;
	}
	Mutex_old = CreateMutex(NULL, FALSE, NULL);
	if (Mutex_old == NULL)
	{
		WriteLogFile("create mutex old error!");
		return DLL_ERROR_SYSTEM;
	}
	Mutex_new = CreateMutex(NULL, FALSE, NULL);
	if (Mutex_new == NULL)
	{
		WriteLogFile("create mutex new error!");
		return DLL_ERROR_SYSTEM;
	}
	main_mutex = CreateMutex(NULL, FALSE, NULL);
	if (main_mutex == NULL)
	{
		WriteLogFile("create main_mutex error!");
		return DLL_ERROR_SYSTEM;
	}
	for (i = 0; i < MAX_NUM; i++)
	{
		memset(&dev_info_old[i], 0, sizeof(struct dev_info));
		memset(&dev_info_new[i], 0, sizeof(struct dev_info));
	}
	api_nThreadFlag = 1;
	init_flag = 1;
	//last_find_over_flag = NOT_WORKING;
	already_found = 0;
	time_loop = time;
	if (time_loop < 0 || time_loop > 120)
		time_loop = 10;
	if (_access("local_dev.txt", 0) == 0)
	{
		time_loop = 1;
	}
	else
	{
		time_loop = 180;
	}
	WriteLogFile("CreateThread loop");
#ifdef ONLY_DEV_OPER
	return 0;
#endif
	

	/*配置文件表初始化*/
	// if (!terminal_setting_init()){
	// 	WriteLogFile("配置文件初始化失败");
	// 	return 0;
	// }

	// /*终端信息表初始化*/
	// if (!terminal_init()){
	// 	return 0;
	// }


	// Sleep(2 * 1000);
	// main_Handle = CreateThread(NULL, 0, main_thread, NULL, 0, NULL);
	// if (main_Handle == NULL)
	// {
	// 	WriteLogFile("Create main_handle Thread err");
	// 	return DLL_ERROR_SYSTEM;
	// }

	return 0;
}




/*********************************************************************
-   Function : clean_dll_interface
-   Description：释放本dll中打开的所有资源和线程服务
-   Calls：
-   Calles By:
-   Table Accessed :
-   Table Update :
-   Input  :
-   Output :
-   Return :0 正常 非零 返回相应的错误码(通过文档查找相应的错误描述)
-   Other :
***********************************************************************/
EXPORT_DLL int clean_dll_interface()
{
//	int i;
	if (init_flag != 1)
		return DLL_ERROR_SYSTEM;

	api_nThreadFlag = 0;
	
	while (1)
	{
		if (api_nThreadFlag == 2)
			break;
		Sleep(100);
	}
	int ret = WSACleanup();
	if (ret != 0){
		ret = GetLastError();
	}
	CloseHandle(Mutex_new);
	CloseHandle(main_Handle);
	CloseHandle(Mutex_old);//关闭互斥锁
	CloseHandle(main_mutex);
	return ret;

}


/*********************************************************************
-   Function : dev_find_thread_old
-   Description：旧协议局域网udp发现线程
-   Calls：
-   Calles By:
-   Table Accessed :
-   Table Update :
-   Input  :参数一:
-
-   Output :
-   Return :
-   Other :
***********************************************************************/
DWORD WINAPI dev_find_thread_old(LPVOID lpParam)
{
	int i = 0;
	int err;
	struct ip_info*info = (struct ip_info *)lpParam;
	SOCKET connect_socket;
//	u_short local_port;
	/*if (info == NULL)
		local_port = 12312;
	else
		local_port = info->port_old;*/
	connect_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (INVALID_SOCKET == connect_socket)
	{
		/*err = WSAGetLastError();
		printf("\"socket\" error! error code is %d\n", err);*/
		return -1;
	}
	info->sockfd_old = connect_socket;
	// 用来绑定套接字   
	SOCKADDR_IN sin;
	sin.sin_family = AF_INET;
	//sin.sin_port = htons(local_port);
	sin.sin_port = 0;
	if (info == NULL)//一个网卡
		sin.sin_addr.s_addr = 0;
	else
		sin.sin_addr.S_un.S_addr = inet_addr(info->ip);

	int opt = 1;
	// sockfd为需要端口复用的套接字
	setsockopt(connect_socket, SOL_SOCKET, SO_REUSEADDR, (const void *)&opt, sizeof(opt));

	//用来从网络上的广播地址接收数据   
	SOCKADDR_IN sin_from;
	sin_from.sin_family = AF_INET;
	sin_from.sin_port = htons(10001);
	sin_from.sin_addr.s_addr = INADDR_BROADCAST;

	//设置该套接字为广播类型，   
	bool bOpt = true;
	setsockopt(connect_socket, SOL_SOCKET, SO_BROADCAST, (char*)&bOpt, sizeof(bOpt));

	// 绑定套接字   
	err = bind(connect_socket, (SOCKADDR*)&sin, sizeof(SOCKADDR));
	if (SOCKET_ERROR == err)
	{
		shutdown(info->sockfd_old, SD_BOTH);
		closesocket(info->sockfd_old);
		info->sockfd_old = -1;
		err = WSAGetLastError();
	//	printf("\"bind\" error! error code is %d\n", err);
		return -1;
	}
	//printf("the client is start.\n");
	int nAddrLen = sizeof(SOCKADDR);
	int nLoop = 0;
	unsigned char buff[200] = {0};
	char    szMsg[200] = {0};
	//组装UDP 搜索包
	memcpy(szMsg, "\x01\x10", 2);//协议头
	memcpy(szMsg+2, "\x00\x06", 2);//长度位
	memcpy(szMsg +4, "\x04\x01", 2);//命令字
	int nLen = sizeof(sin_from);
	if (SOCKET_ERROR == sendto(connect_socket, szMsg, 6, 0, (SOCKADDR*)&sin_from, nLen))
	{
		return -1;
	}
	int j, repeat_flag = 0;
	while (1)
	{
		// 接收数据   
		int nSendSize = recvfrom(connect_socket, buff, 200, 0, (SOCKADDR*)&sin_from, &nAddrLen);
		if (SOCKET_ERROR == nSendSize)
		{
			err = WSAGetLastError();
		//	printf("\"recvfrom\" error! error code is %d\n", err);
			return -1;
		}
		buff[nSendSize] = '\0';
		for (i = 0; i < MAX_NUM; i++)
		{
			if (dev_info_old[i].state == 0)
				break;
		}
		if (i == MAX_NUM)
			break;
		//检查协议版本等
		if (memcmp(buff, "\x01\x10\x00\x30\x14\x01", 6) != 0)
			continue;
		WaitForSingleObject(Mutex_old, INFINITE);
		char tmp_id[16] = { 0 };
		memcpy(tmp_id, buff + 14, 12);

		//检查是否已经收到过应答（查重）
		for (j = 0; j < MAX_NUM; j++)
		{
			if (dev_info_old[j].state == 1)
			{
			
				if (memcmp(tmp_id, dev_info_old[j].ter_id, strlen(tmp_id)) == 0)
				{
					repeat_flag = 1;
					break;
				}
					
			}
		}
		if (repeat_flag == 1)
		{
			repeat_flag = 0;
			dev_info_old[i].state = 0;
			continue;
		}
		//解析数据保存
		sprintf(dev_info_old[i].ip, "%d.%d.%d.%d", buff[6], buff[7], buff[8], buff[9]);
		dev_info_old[i].cmd_port = buff[10] * 0x100 + buff[11];
		dev_info_old[i].data_port = buff[12] * 0x100 + buff[13];
		memcpy(dev_info_old[i].ter_id, buff + 14, 12);
		dev_info_old[i].usb_port_num = buff[26] * 0x1000000 + buff[27] * 0x10000 + buff[28] * 0x100 + buff[29];
		memcpy(dev_info_old[i].ter_version, buff + 30, 8);
		memcpy(dev_info_old[i].ter_type, buff + 38, 10);
		dev_info_old[i].state = 1;
		memcpy(dev_info_old[i].protocol_ver, "0", 1);
		ReleaseMutex(Mutex_old);
		memset(buff, 0, sizeof(buff));
	}
	
	return 0;
}


DWORD WINAPI deal_new_protocol_recv(LPVOID lpParam)
{
	struct dev_info *line_info;
	int size;
	line_info = (struct dev_info*)lpParam;
	cJSON *item = NULL;
	cJSON *json = NULL;
	if (line_info == NULL)
		return -1;
	json = cJSON_Parse(line_info->tmpinfo);
	if (!json)
	{
		return -1;;
	}
	cJSON *object_data = cJSON_GetObjectItem(json, "data");
	size = cJSON_GetArraySize(object_data);
	if (size != 1)
	{

		cJSON_Delete(json);
		return -1;
	}
	cJSON *arrayItem = cJSON_GetArrayItem(object_data, 0);
	if (arrayItem == NULL)
	{

		cJSON_Delete(json);
		return -1;
	}
	item = cJSON_GetObjectItem(arrayItem, "ter_id");
	if (item == NULL)
	{
		cJSON_Delete(json);
		return -1;
	}
	else
	{
		WaitForSingleObject(Mutex_new, INFINITE);
		if (item->valuestring != NULL)
		{
			char tmp_id[16] = { 0 };
			memcpy(tmp_id, item->valuestring, strlen(item->valuestring));
			int repeat_flag, j = 0;
			for (j = 0; j < MAX_NUM; j++)
			{
				if (dev_info_new[j].state == 1)
				{

					if (memcmp(tmp_id, dev_info_new[j].ter_id, strlen(tmp_id)) == 0)
					{
						repeat_flag = 1;
						break;
					}

				}
			}
			if (repeat_flag == 1)
			{
				repeat_flag = 0;
				
				dev_info_new[j].state = 0;
			}
			memset(line_info->ter_id, 0, sizeof(line_info->ter_id));

			memcpy(line_info->ter_id, item->valuestring, strlen(item->valuestring));
		}
		else
		{
			cJSON_Delete(json);
			line_info->state = 0;
			ReleaseMutex(Mutex_new);
			return -1;
		}

	}
	item = cJSON_GetObjectItem(arrayItem, "ter_ip");
	if (item == NULL)
	{
		cJSON_Delete(json);
		line_info->state = 0;
		ReleaseMutex(Mutex_new);
		return -1;
	}
	else
	{
		if (item->valuestring != NULL)
		{
			memset(line_info->ip, 0, sizeof(line_info->ip));
			memcpy(line_info->ip, item->valuestring, strlen(item->valuestring));
		}

	}
	item = cJSON_GetObjectItem(arrayItem, "usb_port_num");
	if (item == NULL)
	{
		cJSON_Delete(json);
		line_info->state = 0;
		ReleaseMutex(Mutex_new);
		return -1;
	}
	else
	{
		if (item->valuestring != NULL)
		{
			line_info->usb_port_num = atoi(item->valuestring);

		}

	}
	item = cJSON_GetObjectItem(arrayItem, "ter_version");
	if (item == NULL)
	{
		cJSON_Delete(json);
		line_info->state = 0;
		ReleaseMutex(Mutex_new);
		return -1;
	}
	else
	{
		if (item->valuestring != NULL)
		{
			memset(line_info->ter_version, 0, sizeof(line_info->ter_version));
			memcpy(line_info->ter_version, item->valuestring, strlen(item->valuestring));
		}

	}
	item = cJSON_GetObjectItem(arrayItem, "ter_type");
	if (item == NULL)
	{
		cJSON_Delete(json);
		line_info->state = 0;
		ReleaseMutex(Mutex_new);
		return -1;
	}
	else
	{
		if (item->valuestring != NULL)
		{
			memset(line_info->ter_type, 0, sizeof(line_info->ter_type));
			memcpy(line_info->ter_type, item->valuestring, strlen(item->valuestring));
		}

	}
	cJSON_Delete(json);
	line_info->state = 1;
	memcpy(line_info->protocol_ver, "1", 1);
	ReleaseMutex(Mutex_new);

	return 0;
}
/*********************************************************************
-   Function : dev_find_thread_new
-   Description：新协议局域网udp发现线程
-   Calls：
-   Calles By:
-   Table Accessed :
-   Table Update :
-   Input  :参数一: 
-
-   Output :
-   Return :
-   Other :
***********************************************************************/



DWORD WINAPI dev_find_thread_new(LPVOID lpParam)
{
	int i = 0;
	int err;
	struct ip_info*info = (struct ip_info *)lpParam;
	cJSON *root = NULL, *dir1 = NULL,*dir2 = NULL;
	SOCKET udp_socket;
	udp_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (INVALID_SOCKET == udp_socket)
	{
		/*err = WSAGetLastError();
		printf("\"socket\" error! error code is %d\n", err);*/
		return -1;
	}
	info->sockfd_new = udp_socket;
	// 用来绑定套接字   
	SOCKADDR_IN sin;
	sin.sin_family = AF_INET;
	//sin.sin_port = 0;
	sin.sin_port = htons(10003);
	

	if (info == NULL)
		sin.sin_addr.s_addr = 0;
	else
		sin.sin_addr.S_un.S_addr = inet_addr(info->ip);
		//sin.sin_addr.s_addr = INADDR_ANY;

	

	// 用来从网络上的广播地址接收数据   
	SOCKADDR_IN sin_from;
	sin_from.sin_family = AF_INET;
	sin_from.sin_port = htons(10001);
	sin_from.sin_addr.s_addr = INADDR_BROADCAST;



	//设置该套接字为广播类型，   
	bool bOpt = true;
	setsockopt(udp_socket, SOL_SOCKET, SO_BROADCAST, (char*)&bOpt, sizeof(bOpt));

	// 绑定套接字   
	err = bind(udp_socket, (SOCKADDR*)&sin, sizeof(SOCKADDR));
	if (SOCKET_ERROR == err)
	{
		shutdown(info->sockfd_new, SD_BOTH);
		closesocket(info->sockfd_new);
		info->sockfd_new = -1;
		err = WSAGetLastError();
		return -1;
	}
	int nAddrLen = sizeof(SOCKADDR);
	int nLoop = 0;
	char buff[50000] = { 0 };
	char    szMsg[500] = { 0 };
	char *g_buf = NULL;
	int size = 0;
	


	//打包新接口协议json发送包
	root = cJSON_CreateObject();
	if (root == NULL)
		return -1;
	cJSON_AddItemToObject(root, "head", dir1 = cJSON_CreateObject());
	cJSON_AddStringToObject(dir1, "protocol", "mb_usbshare_20191024");
	cJSON_AddStringToObject(dir1, "code_type", "request");
	cJSON_AddStringToObject(dir1, "cmd", "search_ters");
	cJSON_AddStringToObject(dir1, "source_topic", " ");
	cJSON_AddStringToObject(dir1, "random", "123456789");
	cJSON_AddStringToObject(dir1, "result", " ");
	cJSON_AddItemToObject(root, "data", dir2 = cJSON_CreateArray());
	g_buf = cJSON_PrintUnformatted(root);
	memcpy(szMsg, g_buf, strlen(g_buf));
	free(g_buf);
	cJSON_Delete(root);
	int nLen = sizeof(sin_from);

	int nRecvBuf = 10000 * 1024;
	setsockopt(udp_socket, SOL_SOCKET, SO_RCVBUF, (const char*)&nRecvBuf, sizeof(int));
	if (SOCKET_ERROR == sendto(udp_socket, szMsg, strlen(szMsg), 0, (SOCKADDR*)&sin_from, nLen))
	{
		shutdown(info->sockfd_new, SD_BOTH);
		closesocket(info->sockfd_new);
		info->sockfd_new = -1;
		return -1;
	}
	int count = 0;
	//Sleep(10*1000);




	while (1)
	{
		// 接收数据   
		
		memset(buff, 0, sizeof(buff));
		int nSendSize = recvfrom(udp_socket, buff, 255, 0, (SOCKADDR*)&sin, &nAddrLen);//sin or sin_from
		if (SOCKET_ERROR == nSendSize)
		{
			WriteLogFile("socket err!\n");
			if (info->sockfd_new > 0)
			{
				shutdown(info->sockfd_new, SD_BOTH);
				closesocket(info->sockfd_new);
				info->sockfd_new = -1;
			}
			err = WSAGetLastError();
			return -1;
		}
//#ifndef TEST
//		WriteLogFile("count:%d :size:%d\n!\n", count++, nSendSize);
//		continue;
//#endif
		buff[nSendSize] = '\0';
		WaitForSingleObject(Mutex_new, INFINITE);
		for (i = 0; i < 100; i++)
		{
			if (dev_info_new[i].state == 0)
				break;
		}
		if (i == 100)
		{
			ReleaseMutex(Mutex_new);
			WriteLogFile("count:max\n");
			break;
		}
		else
		{
			memset(dev_info_new[i].tmpinfo, 0, sizeof(dev_info_new[i].tmpinfo));
			memcpy(dev_info_new[i].tmpinfo, buff, nSendSize);
			dev_info_new[i].state = 1;
			ReleaseMutex(Mutex_new);
			int j;
			for (j = 0; j < 100; j++)
			{
				if (deal_thread_new[j] == NULL)
					break;
			}
			if (j == 100)
				continue;
			deal_thread_new[j] = CreateThread(NULL, 0, deal_new_protocol_recv, &dev_info_new[i], 0, NULL);
			//WriteLogFile("count:%d size :%d \n", j, nSendSize);
			if (deal_thread_new[j] == NULL)
			{
				return -1;
			}
		}

	}

	return 0;
}



/*********************************************************************
-   Function : GetAdapterState
-   Description：获取本网卡的网络状态
-   Calls：
-   Calles By:
-   Table Accessed :
-   Table Update :
-   Input  :参数一: nIndex:当前所需要获取状态的网卡索引
-
-   Output :
-   Return :0 正常 非零 返回相应的错误码(通过文档查找相应的错误描述)
-   Other :
***********************************************************************/
int GetAdapterState(DWORD nIndex)
{
	MIB_IFROW miInfo;   // 存放获取到的 Adapter 参数
	memset(&miInfo, 0, sizeof(MIB_IFROW));
	miInfo.dwIndex = nIndex;   // dwIndex 是需要获取的 Adapter 的索引
	if (GetIfEntry(&miInfo) != NOERROR)
	{
		//printf("ErrorCode = %d\n", GetLastError());
		return false;
	}
	if (miInfo.dwOperStatus == IF_OPER_STATUS_NON_OPERATIONAL || miInfo.dwOperStatus == IF_OPER_STATUS_UNREACHABLE
		|| miInfo.dwOperStatus == IF_OPER_STATUS_DISCONNECTED || miInfo.dwOperStatus == IF_OPER_STATUS_CONNECTING)
	{
		return -1;
	}
	else if (miInfo.dwOperStatus == IF_OPER_STATUS_OPERATIONAL || miInfo.dwOperStatus == IF_OPER_STATUS_CONNECTED)
	{
		return 0;
	}
	else
	{
		return false;
	}
}




int if_a_string_is_a_valid_ipv4_address(char *str)
{
	struct in_addr addr;
	int ret;
	//int local_errno;

	errno = 0;
	ret = inet_pton(AF_INET, str, &addr);
	//local_errno = errno;
	/*if (ret > 0)
		fprintf(stderr, "\"%s\" is a valid IPv4 address\n", str);
	else if (ret < 0)
		fprintf(stderr, "EAFNOSUPPORT: %s\n", strerror(local_errno));
	else
		fprintf(stderr, "\"%s\" is not a valid IPv4 address\n", str);*/

	return ret;
}

/*********************************************************************
-   Function : find_adapter_ip
-   Description：查找本机所有网卡的ip
-   Calls：
-   Calles By:
-   Table Accessed :
-   Table Update :
-   Input  :参数一: total:传出参数，返回的当前电脑的所有网卡ip数量
-
-   Output :
-   Return :0 正常 非零 返回相应的错误码(通过文档查找相应的错误描述)
-   Other :
***********************************************************************/
int find_adapter_ip(int *total)
{
	*total = 0;
	PIP_ADAPTER_INFO pIpAdapterInfo =(IP_ADAPTER_INFO *)MALLOC(sizeof (IP_ADAPTER_INFO)*10);
	unsigned long stSize = sizeof(IP_ADAPTER_INFO)* ADAPTERNUM;
	if (pIpAdapterInfo == NULL) {
		WriteLogFile("Error allocating memory needed to call GetAdaptersinfo\n");
		return -1;
	}
	int nRel = GetAdaptersInfo(pIpAdapterInfo, &stSize);
	if (ERROR_BUFFER_OVERFLOW == nRel) {
		// 释放空间
		if (pIpAdapterInfo != NULL)
			FREE(pIpAdapterInfo);
		return -1;
	}
	PIP_ADAPTER_INFO cur = pIpAdapterInfo;
	while (cur){
		
		//printf("\tComboIndex: \t%d\n", cur->ComboIndex);
		//printf("\tAdapter Name: \t%s\n", cur->AdapterName);
		//printf("\tAdapter Desc: \t%s\n", cur->Description);

		//printf("\tAdapter Type: \t%d\n", cur->Type);
		IP_ADDR_STRING *pIpAddrString1 = &(cur->IpAddressList);
		//printf("\tAdapter  IP: \t%s\n", pIpAddrString1->IpAddress.String);
		//printf("\tAdapter Addr: \t");
		
		if (GetAdapterState(cur->Index) == 0)
		{
		}
		else
		{
			cur = cur->Next;
			continue;
		}

		switch (cur->Type) {
			case MIB_IF_TYPE_OTHER:
				break;
			case MIB_IF_TYPE_ETHERNET:
			{
					if (strstr(cur->Description, "Virtual") != NULL || strstr(cur->Description, "Loop")!= NULL)
					{
						cur = cur->Next;
						continue;
					}
				
					IP_ADDR_STRING *pIpAddrString = &(cur->IpAddressList);
					if (memcmp(pIpAddrString->IpAddress.String, "0.0.0.0", 7) == 0)
					{
						cur = cur->Next;
						continue;
					}
					if (*total >= 9)
					{
						cur = cur->Next;
						continue;
					}
					memcpy(ip_group[*total], pIpAddrString->IpAddress.String, strlen(pIpAddrString->IpAddress.String));
					WriteLogFile("Get ip is:%s\n", ip_group[*total]);
					memcpy(mask_group[*total], pIpAddrString->IpMask.String, strlen(pIpAddrString->IpMask.String));
					*total+=1;
					IP_ADDR_STRING *cur_ip = pIpAddrString->Next;
					while (cur_ip)
					{
						if (*total >= 9)
						{
							cur = cur->Next;
							break;
						}
						if (cur_ip != NULL)
						{
							memcpy(ip_group[*total], cur_ip->IpAddress.String, strlen(cur_ip->IpAddress.String));
							WriteLogFile("Get ip MIB_IF_TYPE_ETHERNET is:%s\n", ip_group[*total]);
							memcpy(mask_group[*total], cur_ip->IpMask.String, strlen(cur_ip->IpMask.String));
							*total += 1;
							cur_ip = cur_ip->Next;
						}
					}

			}
				break;
			case MIB_IF_TYPE_TOKENRING:
				printf("MIB_IF_TYPE_TOKENRING\n");
				break;
			case MIB_IF_TYPE_FDDI:
				printf("MIB_IF_TYPE_FDDI\n");
				break;
			case MIB_IF_TYPE_PPP:
				printf("MIB_IF_TYPE_PPP\n");
				break;
			case MIB_IF_TYPE_LOOPBACK:
				printf("MIB_IF_TYPE_LOOPBACK\n");
				break;
			case MIB_IF_TYPE_SLIP:
				break;
			default://无线网卡,Unknown type
			{
					if (strstr(cur->Description, "Virtual") != NULL || strstr(cur->Description, "Loop") != NULL)
					{
						cur = cur->Next;
						continue;
					}
					
					IP_ADDR_STRING *pIpAddrString = &(cur->IpAddressList);
					if (memcmp(pIpAddrString->IpAddress.String, "0.0.0.0", 7) == 0)
					{
						cur = cur->Next;
						continue;
					}
					if (*total >= 9)
					{
						cur = cur->Next;
						continue;
					}
					//printf("wireless IP:%s\n", pIpAddrString->IpAddress.String);
					if (if_a_string_is_a_valid_ipv4_address(pIpAddrString->IpAddress.String) < 0)
					{
						WriteLogFile("Get ip Unknown is error\n");
						break;
					}
					memcpy(ip_group[*total], pIpAddrString->IpAddress.String, strlen(pIpAddrString->IpAddress.String));
					WriteLogFile("Get ip Wireless network card is:%s\n", ip_group[*total]);
					memcpy(mask_group[*total], pIpAddrString->IpMask.String, strlen(pIpAddrString->IpMask.String));
					*total+=1;
					break;
					WriteLogFile("Get ip Wireless network end\n");
				
			}
			
		}
		
		cur = cur->Next;
	}
	if (pIpAdapterInfo != NULL)
		FREE(pIpAdapterInfo);

	return 0;
}



/*********************************************************************
-   Function : dev_discovery
-   Description：局域网设备发现
-   Calls：
-   Calles By:
-   Table Accessed :
-   Table Update :
-   Input  :参数一: output:传出参数，返回的当前局域网的所有机柜ip编号信息等json字符串
-			
-   Output :
-   Return :0 正常 非零 返回相应的错误码(通过文档查找相应的错误描述)
-   Other :
***********************************************************************/
//int dev_discovery(char *output)
//{
//
////这个函数改造，从client_query_cabinet_attribute_A函数中获取参数
//
//	cJSON *root = NULL;
//	int i,result;
//	int num = 0;
//	char ip1[20] = { 0 };
//	char ip2[20] = { 0 };
//	char *tmp = NULL;
//	char port_num[5] = { 0 };
//	//HANDLE handle_old[10];
//	HANDLE handle_new[10];
//
//	for (i = 0; i < 100; i++)
//	{
//		deal_thread_new[i] = NULL;
//	}
//	struct ip_info info_ip[10];
//
//	for (i = 0; i < 10; i++)
//	{
//		memset(ip_group[i], 0, sizeof(ip_group[i]));
//	}
//	//获取不同网卡的ip信息(不包括虚拟网卡等不考虑的网卡)
//	result = find_adapter_ip(&num);
//	if (result < 0)
//		return DLL_ERROR_GET_ADAPTER;
//
//	//WriteLogFile("find_adapter num is :%d",num);
//	for (i = 0; i < 10; i++)
//	{
//		memset(&info_ip[i], 0, sizeof(struct ip_info));
//	}
//	if (num >= 10)//最多支持10个IP
//		num = 10;
//	//根据返回的所有网卡IP总数创建对应数量的新老协议线程
//	for (i = 0; i < num; i++)
//	{
//		memcpy(info_ip[i].ip, ip_group[i], strlen(ip_group[i]));
//		//memcpy(info_ip[i].mask, mask_group[i], strlen(mask_group[i]));
//		/*handle_old[i] = CreateThread(NULL, 0, dev_find_thread_old, &info_ip[i], 0, NULL);
//		if (handle_old[i] == NULL)
//		{
//			WriteLogFile("Create old_handle Thread err");
//			return DLL_ERROR_SYSTEM;
//		}*/
//		handle_new[i] = CreateThread(NULL, 0, dev_find_thread_new, &info_ip[i], 0, NULL);
//		if (handle_new[i] == NULL)
//		{
//			WriteLogFile("Create new_handle Thread err");
//			return DLL_ERROR_SYSTEM;
//		}
//
//	}
//	//WriteLogFile("dev_discovery step2");
//	//等待所有线程子线程结束,并释放资源
//	for (i = 0; i < num; i++)
//	{
//		//WaitForSingleObject(handle_old[i], (time_loop-1)*1000);//5000
//		//if (info_ip[i].sockfd_old > 0)
//		//{
//		//	
//		//	
//		//	shutdown(info_ip[i].sockfd_old, SD_BOTH);
//		//	closesocket(info_ip[i].sockfd_old);
//		//	CloseHandle(handle_old[i]);
//		//}
//		if (already_found != 1)
//			WaitForSingleObject(handle_new[i], (time_loop - 1) * 1000);
//		else
//			WaitForSingleObject(handle_new[i], 179 * 1000);
//		
//		if (info_ip[i].sockfd_new > 0)
//		{
//			shutdown(info_ip[i].sockfd_new, SD_BOTH);
//			closesocket(info_ip[i].sockfd_new);
//			CloseHandle(handle_new[i]);
//		}
//		//ReleaseMutex(Mutex_new);
//		for (i = 0; i < 100; i++)
//		{
//			if (deal_thread_new[i] != NULL)
//			{
//				WaitForSingleObject(deal_thread_new[i], INFINITE);
//				CloseHandle(deal_thread_new[i]);
//				deal_thread_new[i] = NULL;
//			}
//		}
//		//ReleaseMutex(Mutex_old);
//		ReleaseMutex(Mutex_new);
//	}
//	//组回调返回包
//
//	//WriteLogFile("dev_discovery step3");
//
//	if (_access("local_dev.txt", 0) == 0)
//	{
//		WriteLogFile("have local configure file\n");
//		char buf[100] = { 0 };
//		_getcwd(buf, sizeof(buf));
//		char filedir[200] = { 0 };
//		sprintf(filedir, "%s\\local_dev.txt", buf);
//		WriteLogFile("configure file address :%s", filedir);
//
//		unsigned char *pDst = NULL;
//	
//		int nFileSize = 1024 * 1024;
//		
//		pDst = (unsigned char *)malloc(nFileSize);
//		if (!pDst)
//		{
//			WriteLogFile("malloc file mem error");
//			return -1;
//		}
//		memset(pDst, 0, nFileSize);
//
//	
//		FILE *fp = fopen(filedir, "r");
//		if (fp == NULL)
//		{
//			WriteLogFile("open file error");
//			return -1;
//		}
//		WriteLogFile("configure file read");
//		int nByteIO = fread(pDst, 1, nFileSize, fp);
//		fclose(fp);
//		WriteLogFile("configure file data :%s", pDst);
//		memcpy(output, pDst, strlen(pDst));
//		free(pDst);
//		WriteLogFile("configure file data :%s", output);
//	}
//	else
//	{
//		cJSON  *dir2;
//		int j, repeat_flag = 0;
//		root = cJSON_CreateObject();
//		cJSON_AddStringToObject(root, "cmd", "find_dev");
//		cJSON_AddItemToObject(root, "data", dir2 = cJSON_CreateArray());
//		WaitForSingleObject(Mutex_new, INFINITE);
//		for (i = 0; i < MAX_NUM; i++)
//		{
//
//			if (dev_info_new[i].state == 1)
//			{
//				cJSON *p_layer;
//				cJSON_AddItemToObject(dir2, "dira", p_layer = cJSON_CreateObject());
//
//				cJSON_AddStringToObject(p_layer, "ter_ip", dev_info_new[i].ip);
//				cJSON_AddStringToObject(p_layer, "ter_id", dev_info_new[i].ter_id);
//				memset(port_num, 0, sizeof(port_num));
//				sprintf(port_num, "%d", dev_info_new[i].usb_port_num);
//				cJSON_AddStringToObject(p_layer, "usb_port_num", port_num);
//				cJSON_AddStringToObject(p_layer, "ter_version", dev_info_new[i].ter_version);
//				cJSON_AddStringToObject(p_layer, "ter_type", dev_info_new[i].ter_type);
//				cJSON_AddStringToObject(p_layer, "protocol_ver", dev_info_new[i].protocol_ver);
//				cJSON_AddStringToObject(p_layer, "online", "1");
//			}
//		}
//
//		ReleaseMutex(Mutex_new);
//
//		tmp = cJSON_PrintUnformatted(root);
//		WaitForSingleObject(main_mutex, INFINITE);
//		memcpy(output, tmp, strlen(tmp));
//		WriteLogFile("find dev:len:%d \n%s\n", strlen(tmp), output);
//		ReleaseMutex(main_mutex);
//		cJSON_Delete(root);
//		free(tmp);
//	}
//	
//	return 0;
//}
//

int dev_discovery(char *output)
{
	//这个函数改造，从client_query_cabinet_attribute_A函数中获取参数
	cJSON *root = NULL, *dir2 = NULL, *p_layer = NULL;
	int i = 0, j = 0, ret = -1, ret1 = -1;
	char *tmp = NULL;
	char port_num[5] = { 0x00 };
	struct CabinetInfo *cabinet = NULL;
	struct file_setting	file[128] = { 0 };

	cabinet = (struct CabinetInfo*)calloc(1, sizeof(struct CabinetInfo) * 128);
	if (NULL == cabinet)
	{
		return -1;
	}
	ret = terminal_query_cabinet_attribute(cabinet, 128);
	ret1 = read_setting(file, 128);

	root = cJSON_CreateObject();
	cJSON_AddStringToObject(root, "cmd", "find_dev");
	cJSON_AddItemToObject(root, "data", dir2 = cJSON_CreateArray());
	for (i = 0; i < 128; ++i)
	{
		if (*cabinet[i].ca_SettingIP == 0)
		{
			continue;
		}
		for (j = 0; j < 128; ++j)
		{
			if (strcmp(cabinet[i].ca_SettingIP, file[j].terminal_ip) == 0 && cabinet[i].dataport == file[j].data_port)
			{
				break;
			}
		}
		if (128 == j)
		{
			continue;
		}
		cJSON_AddItemToObject(dir2, "dira", p_layer = cJSON_CreateObject());
		cJSON_AddStringToObject(p_layer, "ter_ip", file[j].remote_ip);
		cJSON_AddStringToObject(p_layer, "server_port", file[j].remote_dataport);
		cJSON_AddStringToObject(p_layer, "ter_id", cabinet[i].ca_TerminalID);
		memset(port_num, 0, sizeof(port_num));
		sprintf(port_num, "%d", cabinet[i].i_USBSumNumber);
		cJSON_AddStringToObject(p_layer, "usb_port_num", port_num);
		cJSON_AddStringToObject(p_layer, "ter_version", cabinet[i].ca_SoftVesion);
		cJSON_AddStringToObject(p_layer, "ter_type", cabinet[i].ca_TerminalModel);
		cJSON_AddStringToObject(p_layer, "protocol_ver", "v1.0.0.1");
		cJSON_AddStringToObject(p_layer, "online", "1");
	}
	tmp = cJSON_PrintUnformatted(root);
	if (NULL == tmp)
	{
		return -1;
	}
	memcpy(output, tmp, strlen(tmp));
	WriteLogFile("find dev:len:%d \n%s\n", strlen(tmp), output);
	cJSON_Delete(root);
	free(tmp);
	return 0;
}

int set_ip_ip(char *json_in, void *hook, char *output)
{
	cJSON *root = NULL;
	cJSON *item = NULL;
	int result = 0;
	result_CallBack callback = (result_CallBack)hook;
	root = cJSON_Parse(json_in);
	if (root == NULL)
	{
		WriteLogFile("json in is not a json!");
		return DLL_ERROR_PARAM;
	}
	char ip[50] = { 0 };
	char ip_type[5] = { 0 };
	char mask[50] = { 0 };
	char gateway[50] = { 0 };
	char dns[50] = { 0 };
	char server_ip[50] = { 0 };
	int server_port;
	item = cJSON_GetObjectItem(root, "server_ip");
	if (item == NULL)
	{
		WriteLogFile("get server_ip error!");
		return DLL_ERROR_PARAM;
	}
	memcpy(server_ip, item->valuestring, strlen(item->valuestring));

	item = cJSON_GetObjectItem(root, "server_port");
	if (item == NULL)
	{
		WriteLogFile("get server_port error!");
		return DLL_ERROR_PARAM;
	}
	server_port = atoi(item->valuestring);
	//server_port = 13029;

	item = cJSON_GetObjectItem(root, "ip_type");
	if (item == NULL)
	{
		WriteLogFile("get ip type error!");
		return DLL_ERROR_PARAM;
	}
	memcpy(ip_type, item->valuestring, strlen(item->valuestring));

	if (atoi(ip_type) == 0)//dhcp
	{
		strcpy(ip, "192.168.0.1");
		strcpy(mask, "192.168.0.1");
		strcpy(gateway, "192.168.0.1");
		strcpy(dns, "192.168.0.1");
	}
	else
	{

		item = cJSON_GetObjectItem(root, "ip_address");
		if (item == NULL)
		{
			WriteLogFile("get ip_address error!");
			return DLL_ERROR_PARAM;
		}
		memcpy(ip, item->valuestring, strlen(item->valuestring));

		item = cJSON_GetObjectItem(root, "netmask");
		if (item == NULL)
		{
			WriteLogFile("get netmask error!");
			return DLL_ERROR_PARAM;
		}
		memcpy(mask, item->valuestring, strlen(item->valuestring));

		item = cJSON_GetObjectItem(root, "gateway");
		if (item == NULL)
		{
			WriteLogFile("get gateway error!");
			return DLL_ERROR_PARAM;
		}
		memcpy(gateway, item->valuestring, strlen(item->valuestring));

		item = cJSON_GetObjectItem(root, "dns");
		if (item == NULL)
		{
			WriteLogFile("get dns error!");
			return DLL_ERROR_PARAM;
		}
		memcpy(dns, item->valuestring, strlen(item->valuestring));
	}
	result = set_usbshare_ip_new(atoi(ip_type), server_ip, server_port, ip, mask, gateway, dns, NULL);
	return result;
}
int set_id_date(char *json_in, void *hook, char *output)
{
	cJSON *root = NULL;
	cJSON *item = NULL;
	int result;
	result_CallBack callback = (result_CallBack)hook;
	root = cJSON_Parse(json_in);
	if (root == NULL)
	{
		WriteLogFile("json in is not a json!");
		return DLL_ERROR_PARAM;
	}
	char server_ip[50] = { 0 };
	int server_port;
	char ter_id[20] = { 0 };
	char pro_date[50] = { 0 };
	item = cJSON_GetObjectItem(root, "server_ip");
	if (item == NULL)
	{
		WriteLogFile("get server_ip error!");
		return DLL_ERROR_PARAM;
	}
	memcpy(server_ip, item->valuestring, strlen(item->valuestring));

	item = cJSON_GetObjectItem(root, "server_port");
	if (item == NULL)
	{
		WriteLogFile("get server_port error!");
		return DLL_ERROR_PARAM;
	}
	server_port = atoi(item->valuestring);

	item = cJSON_GetObjectItem(root, "ter_id");
	if (item == NULL)
	{
		WriteLogFile("get ter_id error!");
		return DLL_ERROR_PARAM;
	}
	memcpy(ter_id, item->valuestring, strlen(item->valuestring));

	item = cJSON_GetObjectItem(root, "product_date");
	if (item == NULL)
	{
		WriteLogFile("get product_date error!");
		return DLL_ERROR_PARAM;
	}
	memcpy(pro_date, item->valuestring, strlen(item->valuestring));
	result = set_usbshare_id_date(server_ip, server_port, ter_id, pro_date);
	return result;
}
int reboot_terminal(char *json_in, void *hook, char *output)
{
	cJSON *root = NULL;
	cJSON *item = NULL;
	int result = 0;
	result_CallBack callback = (result_CallBack)hook;
	root = cJSON_Parse(json_in);
	if (root == NULL)
	{
		WriteLogFile("json in is not a json!");
		return DLL_ERROR_PARAM;
	}
	char server_ip[50] = { 0 };
	int server_port;
	item = cJSON_GetObjectItem(root, "server_ip");
	if (item == NULL)
	{
		WriteLogFile("get server_ip error!");
		return DLL_ERROR_PARAM;
	}
	memcpy(server_ip, item->valuestring, strlen(item->valuestring));

	item = cJSON_GetObjectItem(root, "server_port");
	if (item == NULL)
	{
		WriteLogFile("get server_port error!");
		return DLL_ERROR_PARAM;
	}
	server_port = atoi(item->valuestring);
	result = restart_terminal(server_ip,server_port);
	return result;
}
int get_ter_all_info(char *json_in, void *hook,char *output)
{
	cJSON *root = NULL;
	cJSON *item = NULL;
	char intranet_ip[64] = {0x00},external_ip[64] = {0x00};
	result_CallBack callback = (result_CallBack)hook;

	root = cJSON_Parse(json_in);
	if (root == NULL)
	{
		WriteLogFile("json in is not a json!");
		return DLL_ERROR_PARAM;
	}

	WriteLogFile("dev_info 获取机柜信息准备!");
	//char all_ter_info[1024 * 80] = { 0 };
	char server_ip[50] = { 0 };
	int server_port;
	item = cJSON_GetObjectItem(root, "server_ip");
	if (item == NULL)
	{
		cJSON_Delete(root);
		WriteLogFile("get server_ip error!");
		return DLL_ERROR_PARAM;
	}
	memcpy(server_ip, item->valuestring, strlen(item->valuestring));

	item = cJSON_GetObjectItem(root, "server_port");
	if (item == NULL)
	{
		cJSON_Delete(root);
		WriteLogFile("get server_port error!");
		return DLL_ERROR_PARAM;
	}
	server_port = atoi(item->valuestring);

	item = cJSON_GetObjectItem(root, "intranet_ip");
	if (item != NULL)
	{
		memcpy(intranet_ip,item->valuestring,strlen(item->valuestring));
	}

	item = cJSON_GetObjectItem(root, "external_ip");
	if (item != NULL)
	{
		memcpy(external_ip,item->valuestring,strlen(item->valuestring));
	}

	//server_port = 13029;
	WriteLogFile("dev_info 获取机柜信息打包!,server_port=%d",server_port);
	memset(output, 0, sizeof(output));
	get_usbshare_status_new(server_ip, server_port,intranet_ip, external_ip,output);

	cJSON_Delete(root);
	return 0;
}


int set_iptables(char *json_in, void *hook, char *output)
{
	cJSON *root = NULL;
	cJSON *item = NULL;
	int result;
	result_CallBack callback = (result_CallBack)hook;
	root = cJSON_Parse(json_in);
	if (root == NULL)
	{
		WriteLogFile("json in is not a json!");
		return DLL_ERROR_PARAM;
	}
	char server_ip[50] = { 0 };
	int server_port,custom = 0;
	struct iptables_oper iptb_oper;
	memset(&iptb_oper, 0, sizeof(struct iptables_oper));

	item = cJSON_GetObjectItem(root, "server_ip");
	if (item == NULL)
	{
		WriteLogFile("get server_ip error!");
		return DLL_ERROR_PARAM;
	}
	memcpy(server_ip, item->valuestring, strlen(item->valuestring));

	item = cJSON_GetObjectItem(root, "server_port");
	if (item == NULL)
	{
		WriteLogFile("get server_port error!");
		return DLL_ERROR_PARAM;
	}
	server_port = atoi(item->valuestring);


	item = cJSON_GetObjectItem(root, "custom");
	if (item == NULL)
	{
		WriteLogFile("get custom error!");
		return DLL_ERROR_PARAM;
	}
	custom = atoi(item->valuestring);
	if (custom == true)
	{
	

		item = cJSON_GetObjectItem(root, "oper_order");
		if (item == NULL)
		{
			WriteLogFile("get oper_order error!");
			return DLL_ERROR_PARAM;
		}
	
		if (strlen(item->valuestring) > sizeof(iptb_oper.order))
		{
			WriteLogFile("err ,oper_order too long !");
			return DLL_ERROR_PARAM;
		}
		memcpy(iptb_oper.order, item->valuestring, strlen(item->valuestring));
	}
	else
	{
	
		item = cJSON_GetObjectItem(root, "oper_port");
		if (item == NULL)
		{
			WriteLogFile("get oper_port error!");
			return DLL_ERROR_PARAM;
		}
		iptb_oper.oper_port = atoi(item->valuestring);

		item = cJSON_GetObjectItem(root, "drop_accept");
		if (item == NULL)
		{
			WriteLogFile("get drop_accept error!");
			return DLL_ERROR_PARAM;
		}
		iptb_oper.drop_or_accpet = atoi(item->valuestring);
	}
	result = set_usbshare_iptables(server_ip, server_port, &iptb_oper,output);
	return result;
}

/*********************************************************************
-   Function : send_cmd_deal
-   Description：接收指令并进行相关处理
-   Calls：
-   Calles By:
-   Table Accessed :
-   Table Update :
-   Input  :参数一: Json_In:包含指令等信息的json数据
-			参数二: callback:响应消息的回调函数
-   Output :
-   Return :0 正常 非零 返回相应的错误码(通过文档查找相应的错误描述)
-   Other :
***********************************************************************/
EXPORT_DLL int  send_cmd_deal(char * json_in, result_CallBack callback,char *output)
{
	cJSON *root = NULL;
	cJSON*item = NULL;
//	int i;
	int result = 0;
	if (init_flag != 1)
	{
		WriteLogFile("init not ok!");
		return DLL_ERROR_SYSTEM;
	}
	if (json_in == NULL)
	{
		WriteLogFile("error parm");
		return DLL_ERROR_PARAM;
	}
	root = cJSON_Parse(json_in);
	if (root == NULL)
	{
		WriteLogFile("json in is not a json!");
		return DLL_ERROR_PARAM;
	}
	item = cJSON_GetObjectItem(root, "cmd");
	if (item == NULL)
	{
		WriteLogFile("get cmd error!");
		return DLL_ERROR_PARAM;
	}
	WriteLogFile("dev_info = %s!",item->valuestring);
	if (memcmp(item->valuestring, "find_dev", 8) == 0)
	{
		WriteLogFile("find_dev!");
		int count = 0;
		while (1){
			if (already_found == 1)//等待第一次搜索完毕
			{
				WaitForSingleObject(main_mutex, INFINITE);
				//callback(dev_info_back);

				memset(output, 0, sizeof(output));

				memcpy(output, dev_info_back, strlen(dev_info_back));
			//	printf("call back:%s\n", dev_info_back);
				WriteLogFile("discovery dev info -len:%d:%s", strlen(dev_info_back), dev_info_back);
				ReleaseMutex(main_mutex);
				break;
			}
			Sleep(100);
			if (count>=30*60)//超过60S第一次还是未搜索完毕返回超时
			{
				WriteLogFile("设备发现超时!\n");
				return DLL_ERROR_TIME_OUT;
			}
			count++;
		}
		return 0;
	}
	else//处理新接口usbshare指令集
	{
		WriteLogFile("dev_info 接收到的指令为:%s\n", item->valuestring);
		int i;
		for (i = 0; i < sizeof(tcp_order) / sizeof(tcp_order[0]); i++)
		{
			if (memcmp(item->valuestring, tcp_order[i].order, strlen(tcp_order[i].order)) == 0)
			{
				result = tcp_order[i].answer(json_in, callback, output);
				WriteLogFile("dev_info exit!!!!!\n");
				cJSON_Delete(root);
				return result;

			}
		}
		cJSON_Delete(root);
		return DLL_ERROR_CMD;
	}
	//else if (memcmp(item->valuestring, "set_ip", 6) == 0)
	//{
	//	char ip[50] = { 0 };
	//	char ip_type[5] = { 0 };
	//	char mask[50] = { 0 };
	//	char gateway[50] = { 0 };
	//	char dns[50] = { 0 };
	//	char server_ip[50] = { 0 };
	//	int server_port;
	//	item = cJSON_GetObjectItem(root, "server_ip");
	//	if (item == NULL)
	//	{
	//		WriteLogFile("get server_ip error!");
	//		return DLL_ERROR_PARAM;
	//	}
	//	memcpy(server_ip, item->valuestring, strlen(item->valuestring));

	//	item = cJSON_GetObjectItem(root, "server_port");
	//	if (item == NULL)
	//	{
	//		WriteLogFile("get server_port error!");
	//		return DLL_ERROR_PARAM;
	//	}
	//	server_port=atoi(item->valuestring);


	//	item = cJSON_GetObjectItem(root, "ip_type");
	//	if (item == NULL)
	//	{
	//		WriteLogFile("get ip type error!");
	//		return DLL_ERROR_PARAM;
	//	}
	//	memcpy(ip_type, item->valuestring, strlen(item->valuestring));

	//	if (atoi(ip_type) == 0)//dhcp
	//	{
	//		strcpy(ip, "192.168.0.1");
	//		strcpy(mask, "192.168.0.1");
	//		strcpy(gateway, "192.168.0.1");
	//		strcpy(dns, "192.168.0.1");
	//	}
	//	else
	//	{
	//	
	//		item = cJSON_GetObjectItem(root, "ip_address");
	//		if (item == NULL)
	//		{
	//			WriteLogFile("get ip_address error!");
	//			return DLL_ERROR_PARAM;
	//		}
	//		memcpy(ip, item->valuestring, strlen(item->valuestring));

	//		item = cJSON_GetObjectItem(root, "netmask");
	//		if (item == NULL)
	//		{
	//			WriteLogFile("get netmask error!");
	//			return DLL_ERROR_PARAM;
	//		}
	//		memcpy(mask, item->valuestring, strlen(item->valuestring));

	//		item = cJSON_GetObjectItem(root, "gateway");
	//		if (item == NULL)
	//		{
	//			WriteLogFile("get gateway error!");
	//			return DLL_ERROR_PARAM;
	//		}
	//		memcpy(gateway, item->valuestring, strlen(item->valuestring));

	//		item = cJSON_GetObjectItem(root, "dns");
	//		if (item == NULL)
	//		{
	//			WriteLogFile("get dns error!");
	//			return DLL_ERROR_PARAM;
	//		}
	//		memcpy(dns, item->valuestring, strlen(item->valuestring));
	//	}
	//	result = set_usbshare_ip_new(atoi(ip_type), server_ip, server_port, ip, mask, gateway, dns, NULL);
	//	return result;
	//}
	//else if (memcmp(item->valuestring, "get_ter_status", 14) == 0)
	//{
	//	char all_ter_info[1024 * 20] = { 0 };
	//	char server_ip[50] = { 0 };
	//	int server_port;
	//	item = cJSON_GetObjectItem(root, "server_ip");
	//	if (item == NULL)
	//	{
	//		WriteLogFile("get server_ip error!");
	//		return DLL_ERROR_PARAM;
	//	}
	//	memcpy(server_ip, item->valuestring, strlen(item->valuestring));

	//	item = cJSON_GetObjectItem(root, "server_port");
	//	if (item == NULL)
	//	{
	//		WriteLogFile("get server_port error!");
	//		return DLL_ERROR_PARAM;
	//	}
	//	server_port = atoi(item->valuestring);
	//	get_usbshare_status_new(server_ip, server_port, all_ter_info);

	//	callback(all_ter_info);
	//	return 0;
	//}
	//else if (memcmp(item->valuestring, "set_id_date", 11) == 0)
	//{
	//	char server_ip[50] = { 0 };
	//	int server_port;
	//	char ter_id[20] = { 0 };
	//	char pro_date[50] = { 0 };
	//	item = cJSON_GetObjectItem(root, "server_ip");
	//	if (item == NULL)
	//	{
	//		WriteLogFile("get server_ip error!");
	//		return DLL_ERROR_PARAM;
	//	}
	//	memcpy(server_ip, item->valuestring, strlen(item->valuestring));

	//	item = cJSON_GetObjectItem(root, "server_port");
	//	if (item == NULL)
	//	{
	//		WriteLogFile("get server_port error!");
	//		return DLL_ERROR_PARAM;
	//	}
	//	server_port = atoi(item->valuestring);

	//	item = cJSON_GetObjectItem(root, "ter_id");
	//	if (item == NULL)
	//	{
	//		WriteLogFile("get ter_id error!");
	//		return DLL_ERROR_PARAM;
	//	}
	//	memcpy(ter_id, item->valuestring, strlen(item->valuestring));

	//	item = cJSON_GetObjectItem(root, "product_date");
	//	if (item == NULL)
	//	{
	//		WriteLogFile("get product_date error!");
	//		return DLL_ERROR_PARAM;
	//	}
	//	memcpy(pro_date, item->valuestring, strlen(item->valuestring));
	//	result = set_usbshare_id_date(server_ip, server_port,ter_id,pro_date);
	//	return result;
	//}
	//else if (memcmp(item->valuestring, "ter_restart", 11) == 0)
	//{

	//}
	/*else
	{
		return DLL_ERROR_CMD;
	}*/
	return 0;
}