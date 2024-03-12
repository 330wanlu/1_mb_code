#include "dev_main.h"
#include "manage_set_file.h"
#include "terminal_manage.h"
#include <fcntl.h>

typedef int  (*fun_send_cmd_deal)(char * json_in, result_CallBack callback,char *output);

typedef int (*fun_init_dll_interface)(int time);

typedef int (*fun_clean_dll_interface)();

typedef int (*fun_usbip)();

int start_robot_arm(char *inbuff);


HINSTANCE usbip_hDLL = NULL;

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)                                    //DLL��������ڣ���ʱĬ�ϲ���
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

// int get_addrinfo(const struct sockaddr* addr, string& ip, uint16_t port)
// {
// 	void* numeric_addr = NULL;
// 	char addr_buff[INET6_ADDRSTRLEN];
 
// 	int len = sizeof(sockaddr_in);;
// 	char* b = (char*)addr;
 
// 	unsigned char str16;
// 	memset(&str16, 0, 1);
// 	// for (int i = 0; i < len; i++)
// 	// {
// 	// 	memcpy(&str16, b + i, 1);
// 	// 	printf("%02x ", str16);
// 	// }
// 	// printf("\n");
 
// 	if (AF_INET == addr->sa_family)
// 	{
// 		len = sizeof(sockaddr_in);
// 		numeric_addr = &((struct sockaddr_in*)addr)->sin_addr;
// 		port = ntohs(((struct sockaddr_in*)addr)->sin_port);
 
// 		b = (char*)((struct sockaddr_in*)addr);
// 	}
// 	else if (AF_INET6 == addr->sa_family)
// 	{
// 		len = sizeof(sockaddr_in6);
// 		numeric_addr = &((struct sockaddr_in6*)addr)->sin6_addr;
// 		port = ntohs(((struct sockaddr_in6*)addr)->sin6_port);
 
// 		b = (char*)((struct sockaddr_in6*)addr);
// 	}
// 	else
// 	{
// 		return -1;
// 	}
 
// 	// memset(&str16, 0, 1);
// 	// for (int i = 0; i < len; i++)
// 	// {
// 	// 	memcpy(&str16, b + i, 1);
// 	// 	printf("%02x ", str16);
// 	// }
// 	// printf("\n");
 
// 	//printf("family:%d\n", addr->sa_family);
// 	if (NULL != inet_ntop(addr->sa_family, numeric_addr, addr_buff, sizeof(addr_buff)))
// 		ip = addr_buff;
// 	else
// 		return -1;
 
// 	return 0;
// } 

int tcp_listen(const char* host, const char* service, int listen_num)
{
	int listenfd, ret;
	const char on = 1;
	struct addrinfo hints, * res, * ressave;
	memset(&hints, 0, sizeof(hints));
	hints.ai_flags = AI_PASSIVE;
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;//SOCK_DGRAM;//SOCK_STREAM;
	hints.ai_protocol = IPPROTO_IP;
 
	if (0 != (ret = getaddrinfo(host, service, &hints, &res)))
	{
		//cout << "getaddrinfo error: " << gai_strerrorA(ret) << endl;
		return -1;
	}
 
	ressave = res;
	while (NULL != res)
	{
		if (-1 == (listenfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)))
		{
			//cout << "create socket error: " << strerror(errno) << endl;
			res = res->ai_next;
			continue;
		}
 
		if (-1 == setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)))
		{
			//cout << "setsockopt error: " << strerror(errno) << endl;
			closesocket(listenfd);
			res = res->ai_next;
			continue;
		}
 
		int ipv6only = 0;
		if (setsockopt(listenfd, IPPROTO_IPV6, IPV6_V6ONLY, (char*)&ipv6only, sizeof(ipv6only)) != 0) {
			//cout << "set ipv6only failed!";
			continue;
		}
 
		if (-1 == bind(listenfd, res->ai_addr, res->ai_addrlen))
		{
			//cout << "bind error: " << strerror(errno) << endl;
			closesocket(listenfd);
			res = res->ai_next;
			continue;
		}
 
		if (-1 == listen(listenfd, listen_num))
		{
			//cout << "listen error: " << strerror(errno) << endl;
			closesocket(listenfd);
			res = res->ai_next;
			continue;
		}
 
		break;
	}
 
	freeaddrinfo(ressave);
 
	if (NULL == res)
		return -1;
 
	return listenfd;
}
 

 
 
// int main(int argc, char* argv[])
// {
// 	int ret = 0;
// 	int listenfd, connfd;
// 	struct sockaddr_storage cliaddr;
// 	socklen_t len = sizeof(cliaddr);
// 	time_t now;
// 	char buff[128];
 
// 	//������ip�Ͷ˿�
//     //char* ip = NULL;//ipΪNULLʱ�������������е�ַ
// 	char* ip = (char*)"fe80::b090:dc48:48fd:fa63";
//     char* port = (char*)"5555";
//     listenfd = tcp_listen(ip, port);
	
// 	if (listenfd < 0)
// 	{
// 		cout << "call tcp_listen error" << endl;
// 		return -1;
// 	}
 
// 	while (true)
// 	{
// 		connfd = accept(listenfd, (struct sockaddr*)&cliaddr, &len);
 
// 		int size = 0x1UL << 26;   // 32M
// 		ret = setsockopt(connfd, SOL_SOCKET, SO_SNDBUF, (char*)&size, sizeof(size));
// 		if (ret < 0) {
// 			cout << "client error!"<< endl;
// 		}
// 		string ip = "";
// 		uint16_t port = 0;
// 		get_addrinfo((struct sockaddr*)&cliaddr, ip, port);
// 		cout << "client " << ip << "|" << port << " login" << endl;
 
// 		auto clientip = get_remote_addr(connfd);
// 		cout << "clientip " << clientip << endl;
 
// 		now = time(NULL);
// 		snprintf(buff, sizeof(buff) - 1, "%.24s", ctime(&now));
// 		send(connfd, buff, strlen(buff), 0);
// 		closesocket(connfd);
// 	}
 
// 	closesocket(listenfd);
// 	return 0;
// }
 
 
 
 
 
// #ifdef WIN32
// class WSInit
// {
// public:
// 	WSInit()
// 	{
// 		WSADATA wsadata;
// 		WSAStartup(MAKEWORD(2, 2), &wsadata);
// 	}
 
// 	~WSInit() { WSACleanup(); }
// };
 
// static WSInit wsinit_;
// #endif

/*********************************************************************
-   Function : main_thread
-   Description�����̣߳�����ѭ���Ľ���udp�㲥���Ҿ������豸
-   Calls��
-   Calles By:
-   Table Accessed :
-   Table Update :
-   Input  :
-   Output :
-   Return :0 ���� ���� ������Ӧ�Ĵ�����(ͨ���ĵ�������Ӧ�Ĵ�������)
-   Other :
***********************************************************************/
DWORD WINAPI main_thread(LPVOID lpParam)
{
	int result, i;
	char dev_info_tmp[1024*10];
	already_found = 0;
	int not_found_count = 0;
	while (api_nThreadFlag)
	{
		for (i = 0; i < MAX_NUM; i++)
		{
			memset(&dev_info_old[i], 0, sizeof(struct dev_info));
			memset(&dev_info_new[i], 0, sizeof(struct dev_info));
		}
		memset(dev_info_tmp, 0, sizeof(dev_info_tmp));
		//WriteLogFile("in dev_discovery time:%d", time_loop);
		result = dev_discovery(dev_info_tmp);
		//WriteLogFile("main_thread dev_info_tmp:%s ", dev_info_tmp);
		if (result == 0)
		{
			if ((strlen(dev_info_tmp) < 40 && (strlen(dev_info_tmp) != strlen(dev_info_back))) && not_found_count < 3)//��ʾһ̨����û�ѵ�,���ϴ����������������
			{
				not_found_count++;
				WriteLogFile("out dev_discovery not_found_count! ");
				continue;
			}
			not_found_count = 0;
			WaitForSingleObject(main_mutex, INFINITE);
			memset(dev_info_back, 0, sizeof(dev_info_back));
			memcpy(dev_info_back, dev_info_tmp, strlen(dev_info_tmp));
			ReleaseMutex(main_mutex);
			already_found = 1;
		}
		Sleep(10 * 1000);
		
	}
	api_nThreadFlag = 2;
	return 0;
}

int get_all_online_dev_status(char *outbuff)
{
	int ret = -1,count = 1,i = 0,online_num = 0;
	struct CabinetInfo *cabinet = NULL;
	char tmpbuff[64] = {0x00};
	cabinet = (struct CabinetInfo*)calloc(1, sizeof(struct CabinetInfo) * 128);
	if (NULL == cabinet)
	{
		return -1;
	}
	ret = terminal_query_cabinet_attribute(cabinet, 128);
	for(i = 0;i<128;++i)
	{
		if(strlen(cabinet[i].ca_TerminalID))
		{
			online_num++;
			memcpy(&outbuff[count],cabinet[i].ca_SettingIP,strlen(cabinet[i].ca_SettingIP));
			count += 24;
			outbuff[(count)] = (cabinet[i].i_CmdPort>>24)&0xff;
			count++;
			outbuff[(count)] = (cabinet[i].i_CmdPort>>16)&0xff;
			count++;
			outbuff[(count)] = (cabinet[i].i_CmdPort>>8)&0xff;
			count++;
			outbuff[(count)] = (cabinet[i].i_CmdPort>>0)&0xff;
			count++;
			outbuff[(count)] = (cabinet[i].i_DataPort>>24)&0xff;
			count++;
			outbuff[(count)] = (cabinet[i].i_DataPort>>16)&0xff;
			count++;
			outbuff[(count)] = (cabinet[i].i_DataPort>>8)&0xff;
			count++;
			outbuff[(count)] = (cabinet[i].i_DataPort>>0)&0xff;
			count++;
			memcpy(&outbuff[count],cabinet[i].ca_TerminalID,strlen(cabinet[i].ca_TerminalID));
			count += 12;
			outbuff[(count)] = (cabinet[i].i_USBSumNumber>>24)&0xff;
			count++;
			outbuff[(count)] = (cabinet[i].i_USBSumNumber>>16)&0xff;
			count++;
			outbuff[(count)] = (cabinet[i].i_USBSumNumber>>8)&0xff;
			count++;
			outbuff[(count)] = (cabinet[i].i_USBSumNumber>>0)&0xff;
			count++;
			memcpy(&outbuff[count],cabinet[i].ca_SoftVesion,strlen(cabinet[i].ca_SoftVesion));
			count += 8;
			memcpy(&outbuff[count],cabinet[i].ca_TerminalModel,strlen(cabinet[i].ca_TerminalModel));
			count += 10;
		}
	}
	outbuff[0] = online_num;
	free(cabinet);
	return count;
}

char my_error_buff[1024] = {0x00};

void apply_reslut_callback(char *errbuf)
{
	memset(my_error_buff,0x00,sizeof(my_error_buff));
	strcpy(my_error_buff,errbuf);
} 


int apply_port(unsigned char *inbuff)
{
	char ter_id[32] = {0x00};
	char status_data[5120] = {0x00};
	char tmp[32]= {0x00};
	char *gbuf = NULL;
	int port = 0,i = 0,j = 0,ret = 0,force_close_flag = 0 ,result = 0;
	fun_send_cmd_deal send_cmd;

	fun_usbip usbip_get_init_flag;
	fun_usbip usbip_init;
	
	struct CabinetInfo *cabinet = NULL;
	//HINSTANCE hDLL;

	cJSON *root = NULL;
	struct file_setting file_setting[128] = {0};
	SOCKET s_client;

	WriteLogFile("enetr apply_port");

	root = cJSON_CreateObject();
	cJSON_AddStringToObject(root, "cmd", "open");
	
	//��ȡid��port
	memcpy(ter_id,inbuff,12);

	port = inbuff[12];
	port = (port<<8) | inbuff[13];
	port = (port<<8) | inbuff[14];
	port = (port<<8) | inbuff[15];

	result = judge_port_exit(ter_id,port);
	WriteLogFile("apply_port judge_port_exit,result = %d\n",result);
	
	if(result != 1)
	{
		WriteLogFile("apply_port,�˿ڲ����ڣ�port = %d\n",port);
		cJSON_Delete(root);
		return -1;
	}

	// cJSON_Delete(root);
	// return 0;

	cabinet = (struct CabinetInfo*)calloc(1, sizeof(struct CabinetInfo) * 128);
	if (NULL == cabinet)
	{
		cJSON_Delete(root);
		return -1;
	}
	ret = terminal_query_cabinet_attribute(cabinet, 128);

	for(i = 0;i<128;++i)
	{
		if(0 == memcmp(cabinet[i].ca_TerminalID,ter_id,strlen(ter_id)))  //�ҵ���Ӧ�Ļ���id
		{
			WriteLogFile("enetr apply_port,�ҵ���Ӧid = %s",cabinet[i].ca_TerminalID);
			break;
		}
	}
	
    if(i == 128) //δ�ҵ���Ӧ�Ļ���id
	{
		cJSON_Delete(root);
		free(cabinet);
		return -1;
	}

	// if(i == 128)
	// {
	// 	if(NULL != cabinet)
	// 		free(cabinet);
	// 	WriteLogFile("δ�ҵ���Ӧid");
	// 	return -1;
	// }


	// result = read_setting(file_setting, 128);
	// if(result < 0)
	// {
	// 	if(NULL != cabinet)
	// 		free(cabinet);
	// 	WriteLogFile("read_setting failed");
	// 	return -1;
	// }
	// for(j = 0;j < result;++j)
	// {
	// 	if(0 == memcmp(file_setting[j].terminal_ip,cabinet[i].ca_SettingIP,strlen(cabinet[i].ca_SettingIP)) && file_setting[j].data_port == cabinet[i].dataport)
	// 	{
	// 		break;
	// 	}
	// }
	// if(j == result)
	// {
	// 	WriteLogFile("dev_info can not find mattched ip");
	// 	return -1;
	// }

	// WriteLogFile("dev_oper restart_signle_port terminal_ip=%s,data_port=%d,remote_ip=%s,remote_data_port=%d", file_setting[j].terminal_ip,file_setting[j].data_port,file_setting[j].remote_ip,file_setting[j].remote_dataport);
	// //result = socket_create_connect(&s_client, server_ip, server_port);

	// if((s_client = socket_connect(file_setting[j].terminal_ip,file_setting[j].data_port,file_setting[j].remote_ip,file_setting[j].remote_dataport,3000)) == SOCKET_ERROR)
	// {
	// 	WriteLogFile("dev_info get_usbshare_status_new create or connect socket fail!");
	// 	return DLL_ERR_NEW_SHARE_SOCKET_CONNET;
	// }


	
	cJSON_AddStringToObject(root, "server_ip", cabinet[i].ca_SettingIP);
	// memset(tmp,0x00,sizeof(tmp));
	// sprintf(tmp,"%d",cabinet[i].i_DataPort);
	cJSON_AddNumberToObject(root, "server_port", cabinet[i].dataport);
	// memset(tmp,0x00,sizeof(tmp));
	// sprintf(tmp,"%d",port);
	cJSON_AddNumberToObject(root, "dev_port", port);
	cJSON_AddStringToObject(root, "ter_id", cabinet[i].ca_TerminalID);
	cJSON_AddStringToObject(root, "user", "usbjqr");
	cJSON_AddStringToObject(root, "trans_type", "0");
	cJSON_AddStringToObject(root, "oper_id", "00-E2-69-64-13-51");

	WriteLogFile("apply_port server_ip = %s,server_port=%d\n",cabinet[i].ca_SettingIP,cabinet[i].dataport);

	//WriteLogFile("cJSON_PrintUnformatted\n");

	gbuf = cJSON_PrintUnformatted(root);

	//WriteLogFile("gbuf=%s\n",gbuf);

	cJSON_Delete(root);

	//Sleep(20*1000);
	//hDLL=LoadLibrary("ModelDll\\ModelApiDll\\usbip.dll");//���ض�̬���ӿ�MyDll.dll�ļ���
	//hDLL=LoadLibrary("ModelDll\\ModelApiDll\\usbip.dll");



	WriteLogFile("GetProcAddress\n");
	// usbip_get_init_flag = (fun_usbip)GetProcAddress(hDLL,"get_init_flag");
	// if(NULL == usbip_get_init_flag)
	// {
	// 	WriteLogFile("usbip_get_init_flag == NULL\n");
	// 	return -1;
	// }
	WriteLogFile("enter GetProcAddress init_dll_interface \n");
	usbip_init = (fun_usbip)GetProcAddress(usbip_hDLL,"init_dll_interface");
	if(NULL == usbip_init)
	{
		WriteLogFile("usbip_init == NULL\n");
		return -1;
	}
	WriteLogFile("enter usbip_get_init_flag\n");
	//if(1 != usbip_get_init_flag())
	{
		WriteLogFile("enter usbip_init\n");
		usbip_init();
		WriteLogFile("exit usbip_init\n");
	}



	send_cmd=(fun_send_cmd_deal)GetProcAddress(usbip_hDLL,"send_cmd_deal");
	WriteLogFile("enter send_cmd\n");
	memset(my_error_buff,0x00,sizeof(my_error_buff));

	
	ret = send_cmd(gbuf,&apply_reslut_callback,NULL);
	WriteLogFile("exit send_cmd\n");
	ret = strlen(my_error_buff);
	WriteLogFile("____________________________apply_reslut_callback = %s,ret = %d",my_error_buff,ret);
	if(NULL != gbuf)
	{
		free(gbuf);
	}
	WriteLogFile("exit apply port 1\n");
	free(cabinet);
	WriteLogFile("exit apply port 2\n");
	//FreeLibrary(hDLL);//ж��MyDll.dll�ļ���
	WriteLogFile("exit apply port 3\n");
	return ret;
}

int relese_port(unsigned char *inbuff)
{
	char ter_id[32] = {0x00};
	char tmp[32]= {0x00};
	char *gbuf = NULL;
	int port = 0,i = 0,ret = 0,force_close_flag = 0;
	fun_send_cmd_deal send_cmd;
	// fun_usbip usbip_get_init_flag;
	// fun_usbip usbip_init;
	struct CabinetInfo *cabinet = NULL;
	//HINSTANCE hDLL;

	cJSON *root = NULL;

	// WriteLogFile("______________________________________________enetr relese_port");

	root = cJSON_CreateObject();
	cJSON_AddStringToObject(root, "cmd", "close_new");
	
	//��ȡid��port,ǿ���ͷű�־
	memcpy(ter_id,inbuff,12);
	port = inbuff[12];
	port = (port<<8) | inbuff[13];
	port = (port<<8) | inbuff[14];
	port = (port<<8) | inbuff[15];

	force_close_flag = inbuff[16];
	
	cabinet = (struct CabinetInfo*)calloc(1, sizeof(struct CabinetInfo) * 128);
	if (NULL == cabinet)
	{
		return 0;
	}
	ret = terminal_query_cabinet_attribute(cabinet, 128);

	for(i = 0;i<128;++i)
	{
		if(0 == memcmp(cabinet[i].ca_TerminalID,ter_id,strlen(ter_id)))  //�ҵ���Ӧ�Ļ���id
		{
			WriteLogFile("enetr apply_port,�ҵ���Ӧid = %s",cabinet[i].ca_TerminalID);
			break;
		}
	}
    // if(i == 128) //δ�ҵ���Ӧ�Ļ���id
	// 	return 0;

	if(i == 128)
	{
		WriteLogFile("_____1_____\r\n");
		if(NULL != cabinet)
			free(cabinet);
		WriteLogFile("δ�ҵ���Ӧid");
		return -1;
	}
	WriteLogFile("_____2_____\r\n");
	// cJSON_AddStringToObject(root, "server_ip", cabinet[i].ca_SettingIP);
	// // memset(tmp,0x00,sizeof(tmp));
	// // sprintf(tmp,"%d",cabinet[i].i_DataPort);
	// cJSON_AddNumberToObject(root, "server_port", cabinet[i].i_DataPort);
	// // memset(tmp,0x00,sizeof(tmp));
	// // sprintf(tmp,"%d",port);
	cJSON_AddNumberToObject(root, "dev_port", port);
	cJSON_AddStringToObject(root, "ter_id", cabinet[i].ca_TerminalID);
	cJSON_AddStringToObject(root, "user", "usbjqr");
	cJSON_AddStringToObject(root, "trans_type", "0");

	//WriteLogFile("cJSON_PrintUnformatted\n");

	gbuf = cJSON_PrintUnformatted(root);

	//WriteLogFile("relese_port gbuf=%s\n",gbuf);

	cJSON_Delete(root);
	WriteLogFile("_____3_____\r\n");
	//hDLL=LoadLibrary("ModelDll\\ModelApiDll\\usbip.dll");//���ض�̬���ӿ�MyDll.dll�ļ���
	//hDLL=LoadLibrary("ModelDll\\ModelApiDll\\usbip.dll");
	WriteLogFile("_____4_____\r\n");
	//WriteLogFile("GetProcAddress\n");

	// usbip_get_init_flag = (fun_usbip)GetProcAddress(hDLL,"get_init_flag");
	// usbip_init = (fun_usbip)GetProcAddress(hDLL,"init_dll_interface");
	// if(1 != usbip_get_init_flag())
	// {
	// 	usbip_init();
	// }
	WriteLogFile("_____5_____\r\n");
	memset(my_error_buff,0x00,sizeof(my_error_buff));
	send_cmd=(fun_send_cmd_deal)GetProcAddress(usbip_hDLL,"send_cmd_deal");
	//WriteLogFile("enter send_cmd\n");
	WriteLogFile("_____6_____\r\n");
	ret = send_cmd(gbuf,&apply_reslut_callback,NULL);
	WriteLogFile("_____7_____\r\n");
	WriteLogFile("apply_reslut_callback = %s",my_error_buff);
	//WriteLogFile("exit send_cmd\n");

	if(NULL != gbuf)
	{
		free(gbuf);
	}
	free(cabinet);
	//FreeLibrary(hDLL);//ж��MyDll.dll�ļ���
	return 0;
}

static int parse_json_result_nb(char *json_buf)
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


int reboot_terminal_nb(unsigned char *inbuff)
{


	char ter_id[32] = {0x00};
	char tmp[32]= {0x00};
	char *gbuf = NULL;
	int port = 0,i = 0,j = 0,ret = 0,force_close_flag = 0 ,result = 0;
	struct CabinetInfo *cabinet = NULL;
	SOCKET s_client;
	cJSON *root = NULL;
	cJSON *head = NULL;
	struct file_setting file_setting[128] = {0};

	char *g_buf;
	char send_json[4*1024] = { 0 };
	// int len;

	cJSON *item = NULL;
	char server_ip[50] = { 0x00 },tmp_part[4] = { 0x00 };
	// char tmp[32] = {0x00};
	// char *recvbody = NULL;
	// char recvhead[10] = { 0 };
	//unsigned char *recvbody = NULL;
	// long recvlen = 0;
	int re_read = 0;
	int server_port = 0,usb_port = 0;
	
	memcpy(ter_id,inbuff,12);
	port = inbuff[12];
	port = (port<<8) | inbuff[13];
	port = (port<<8) | inbuff[14];
	port = (port<<8) | inbuff[15];
	usb_port = port;
	WriteLogFile("usb_port		reboot_terminal_nb  \r\n");
	cabinet = (struct CabinetInfo*)calloc(1, sizeof(struct CabinetInfo) * 128);
	if (NULL == cabinet)
	{
		return -1;
	}
	ret = terminal_query_cabinet_attribute(cabinet, 128);

	for(i = 0;i<128;++i)
	{
		if(0 == memcmp(cabinet[i].ca_TerminalID,ter_id,strlen(ter_id)))  //�ҵ���Ӧ�Ļ���id
		{
			WriteLogFile("enetr apply_port,�ҵ���Ӧid = %s",cabinet[i].ca_TerminalID);
			break;
		}
	}
	if(i == 128)
	{
		if(NULL != cabinet)
			free(cabinet);
		WriteLogFile("δ�ҵ���Ӧid");
		return -1;
	}


	result = read_setting(file_setting, 128);
	if(result < 0)
	{
		if(NULL != cabinet)
			free(cabinet);
		WriteLogFile("read_setting failed");
		return -1;
	}
	for(j = 0;j < result;++j)
	{
		if(0 == memcmp(file_setting[j].terminal_ip,cabinet[i].ca_SettingIP,strlen(cabinet[i].ca_SettingIP)) && file_setting[j].data_port == cabinet[i].dataport)
		{
			break;
		}
	}
	if(j == result)
	{
		WriteLogFile("dev_info can not find mattched ip");
		return -1;
	}

	WriteLogFile("dev_oper start_robot_arm_nb terminal_ip=%s,data_port=%d,remote_ip=%s,remote_data_port=%d", file_setting[j].terminal_ip,file_setting[j].data_port,file_setting[j].remote_ip,file_setting[j].remote_dataport);
	//result = socket_create_connect(&s_client, server_ip, server_port);

	if((s_client = socket_connect(file_setting[j].terminal_ip,file_setting[j].data_port,file_setting[j].remote_ip,file_setting[j].remote_dataport,3000)) == SOCKET_ERROR)
	{
		WriteLogFile("dev_info get_usbshare_status_new create or connect socket fail!");
		return DLL_ERR_NEW_SHARE_SOCKET_CONNET;
	}

	WriteLogFile("_____________file_setting[j].terminal_ip = %s		 file_setting[j].terminal_port = %d\r\n",file_setting[j].terminal_ip, file_setting[j].terminal_port);

	// result = restart_terminal_nb(file_setting[j].terminal_ip,3240);

	root = cJSON_CreateObject();
	if (root == NULL)
	{
		WriteLogFile("restart_terminal cJSON_CreateObject root error!");
		shutdown(s_client, SD_BOTH);
		closesocket(s_client);
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
	// cJSON *item;
	cJSON_AddItemToObject(data, "dira", item = cJSON_CreateObject());


	// char *g_buf;
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

		WriteLogFile("restart_terminal send error!");
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

		WriteLogFile("restart_terminal recv head error!");
		shutdown(s_client, SD_BOTH);
		closesocket(s_client);
		return DLL_ERR_NEW_SHARE_SOCKET_RECV;
	}
	recvlen = my_recv_data_len(s_client);
	if (recvlen < 0)
	{

		WriteLogFile("restart_terminal recv len error!");
		shutdown(s_client, SD_BOTH);
		closesocket(s_client);
		return DLL_ERR_NEW_SHARE_SOCKET_RECV;
	}
	result = recv(s_client, recvbody, recvlen, 0);
	if (result < 0)
	{

		WriteLogFile("restart_terminal recv body error!");
		shutdown(s_client, SD_BOTH);
		closesocket(s_client);
		return DLL_ERR_NEW_SHARE_SOCKET_RECV;
	}
	result = parse_json_result_nb(recvbody);
	shutdown(s_client, SD_BOTH);
	closesocket(s_client);

	return result;





















	// cJSON *root = NULL;
	// cJSON *item = NULL;
	// int result = 0;
	// result_CallBack callback = (result_CallBack)hook;
	// root = cJSON_Parse(json_in);
	// if (root == NULL)
	// {
	// 	WriteLogFile("json in is not a json!");
	// 	return DLL_ERROR_PARAM;
	// }
	// char server_ip[50] = { 0 };
	// int server_port;
	// item = cJSON_GetObjectItem(root, "server_ip");
	// if (item == NULL)
	// {
	// 	WriteLogFile("get server_ip error!");
	// 	return DLL_ERROR_PARAM;
	// }
	// memcpy(server_ip, item->valuestring, strlen(item->valuestring));

	// item = cJSON_GetObjectItem(root, "server_port");
	// if (item == NULL)
	// {
	// 	WriteLogFile("get server_port error!");
	// 	return DLL_ERROR_PARAM;
	// }
	// server_port = atoi(item->valuestring);
	// result = restart_terminal(server_ip,server_port);
	// return result;
}

// int forced_relese_port(unsigned char *inbuff)
// {
// 	char ter_id[32] = {0x00};
// 	char tmp[32]= {0x00};
// 	char *gbuf = NULL;
// 	int port = 0,i = 0,ret = 0,force_close_flag = 0;
// 	fun_send_cmd_deal send_cmd;
// 	// fun_usbip usbip_get_init_flag;
// 	// fun_usbip usbip_init;
// 	struct CabinetInfo *cabinet = NULL;
// 	HINSTANCE hDLL;

// 	cJSON *root = NULL;

// 	WriteLogFile("______________________________________________enetr ǿ relese_port");

// 	root = cJSON_CreateObject();
// 	cJSON_AddStringToObject(root, "cmd", "forced_close_new");
	
// 	//��ȡid��port,ǿ���ͷű�־
// 	memcpy(ter_id,inbuff,12);
// 	port = inbuff[12];
// 	port = (port<<8) | inbuff[13];
// 	port = (port<<8) | inbuff[14];
// 	port = (port<<8) | inbuff[15];

// 	force_close_flag = inbuff[16];
	
// 	cabinet = (struct CabinetInfo*)calloc(1, sizeof(struct CabinetInfo) * 128);
// 	if (NULL == cabinet)
// 	{
// 		return 0;
// 	}
// 	ret = terminal_query_cabinet_attribute(cabinet, 128);

// 	for(i = 0;i<128;++i)
// 	{
// 		if(0 == memcmp(cabinet[i].ca_TerminalID,ter_id,strlen(ter_id)))  //�ҵ���Ӧ�Ļ���id
// 		{
// 			WriteLogFile("enetr apply_port,�ҵ���Ӧid = %s",cabinet[i].ca_TerminalID);
// 			break;
// 		}
// 	}
//     // if(i == 128) //δ�ҵ���Ӧ�Ļ���id
// 	// 	return 0;

// 	if(i == 128)
// 	{
// 		WriteLogFile("_____1_____\r\n");
// 		if(NULL != cabinet)
// 			free(cabinet);
// 		WriteLogFile("δ�ҵ���Ӧid");
// 		return -1;
// 	}
// 	WriteLogFile("_____2_____\r\n");
// 	// cJSON_AddStringToObject(root, "server_ip", cabinet[i].ca_SettingIP);
// 	// // memset(tmp,0x00,sizeof(tmp));
// 	// // sprintf(tmp,"%d",cabinet[i].i_DataPort);
// 	// cJSON_AddNumberToObject(root, "server_port", cabinet[i].i_DataPort);
// 	// // memset(tmp,0x00,sizeof(tmp));
// 	// // sprintf(tmp,"%d",port);
// 	cJSON_AddNumberToObject(root, "dev_port", port);
// 	cJSON_AddStringToObject(root, "ter_id", cabinet[i].ca_TerminalID);
// 	cJSON_AddStringToObject(root, "user", "usbjqr");
// 	cJSON_AddStringToObject(root, "trans_type", "0");

// 	//WriteLogFile("cJSON_PrintUnformatted\n");

// 	gbuf = cJSON_PrintUnformatted(root);

// 	//WriteLogFile("relese_port gbuf=%s\n",gbuf);

// 	cJSON_Delete(root);
// 	WriteLogFile("_____3_____\r\n");
// 	//hDLL=LoadLibrary("ModelDll\\ModelApiDll\\usbip.dll");//���ض�̬���ӿ�MyDll.dll�ļ���
// 	hDLL=LoadLibrary("ModelDll\\ModelApiDll\\usbip.dll");
// 	WriteLogFile("_____4_____\r\n");
// 	//WriteLogFile("GetProcAddress\n");

// 	// usbip_get_init_flag = (fun_usbip)GetProcAddress(hDLL,"get_init_flag");
// 	// usbip_init = (fun_usbip)GetProcAddress(hDLL,"init_dll_interface");
// 	// if(1 != usbip_get_init_flag())
// 	// {
// 	// 	usbip_init();
// 	// }
// 	WriteLogFile("_____5_____\r\n");
// 	memset(my_error_buff,0x00,sizeof(my_error_buff));
// 	send_cmd=(fun_send_cmd_deal)GetProcAddress(hDLL,"send_cmd_deal");
// 	//WriteLogFile("enter send_cmd\n");
// 	WriteLogFile("_____6_____\r\n");
// 	ret = send_cmd(gbuf,&apply_reslut_callback,NULL);
// 	WriteLogFile("_____7_____\r\n");
// 	WriteLogFile("apply_reslut_callback = %s",my_error_buff);
// 	//WriteLogFile("exit send_cmd\n");

// 	if(NULL != gbuf)
// 	{
// 		free(gbuf);
// 	}
// 	free(cabinet);
// 	FreeLibrary(hDLL);//ж��MyDll.dll�ļ���
// 	return 0;
// }

long my_recv_data_len(int fd)
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

int my_socket_read(int sock, unsigned char *buf, int buf_len)
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

int restart_signle_port(char *inbuff)
{
	char ter_id[32] = {0x00};
	char tmp[32]= {0x00};
	char *gbuf = NULL;
	int port = 0,i = 0,j = 0,ret = 0,force_close_flag = 0 ,result = 0;
	struct CabinetInfo *cabinet = NULL;
	SOCKET s_client;
	cJSON *root = NULL;
	cJSON *head = NULL;
	struct file_setting file_setting[128] = {0};

	char *g_buf;
	char send_json[4*1024] = { 0 };
	int len;

	WriteLogFile("enetr restart_signle_port");
	
	memcpy(ter_id,inbuff,12);
	port = inbuff[12];
	port = (port<<8) | inbuff[13];
	port = (port<<8) | inbuff[14];
	port = (port<<8) | inbuff[15];

	cabinet = (struct CabinetInfo*)calloc(1, sizeof(struct CabinetInfo) * 128);
	if (NULL == cabinet)
	{
		return -1;
	}
	ret = terminal_query_cabinet_attribute(cabinet, 128);

	for(i = 0;i<128;++i)
	{
		if(0 == memcmp(cabinet[i].ca_TerminalID,ter_id,strlen(ter_id)))  //�ҵ���Ӧ�Ļ���id
		{
			WriteLogFile("enetr apply_port,�ҵ���Ӧid = %s",cabinet[i].ca_TerminalID);
			break;
		}
	}
	if(i == 128)
	{
		if(NULL != cabinet)
			free(cabinet);
		WriteLogFile("δ�ҵ���Ӧid");
		return -1;
	}


	result = read_setting(file_setting, 128);
	if(result < 0)
	{
		if(NULL != cabinet)
			free(cabinet);
		WriteLogFile("read_setting failed");
		return -1;
	}
	for(j = 0;j < result;++j)
	{
		if(0 == memcmp(file_setting[j].terminal_ip,cabinet[i].ca_SettingIP,strlen(cabinet[i].ca_SettingIP)) && file_setting[j].data_port == cabinet[i].dataport)
		{
			break;
		}
	}
	if(j == result)
	{
		WriteLogFile("dev_info can not find mattched ip");
		return -1;
	}

	WriteLogFile("dev_oper restart_signle_port terminal_ip=%s,data_port=%d,remote_ip=%s,remote_data_port=%d", file_setting[j].terminal_ip,file_setting[j].data_port,file_setting[j].remote_ip,file_setting[j].remote_dataport);
	//result = socket_create_connect(&s_client, server_ip, server_port);

	if((s_client = socket_connect(file_setting[j].terminal_ip,file_setting[j].data_port,file_setting[j].remote_ip,file_setting[j].remote_dataport,3000)) == SOCKET_ERROR)
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
	cJSON_AddStringToObject(head, "cmd", "restart_port");
	cJSON_AddStringToObject(head, "source_topic", "123");
	cJSON_AddStringToObject(head, "random", "987654321");
	cJSON_AddStringToObject(head, "result", "s");
	cJSON *data;
	cJSON_AddItemToObject(root, "data", data = cJSON_CreateArray());
	cJSON *item;
	cJSON_AddItemToObject(data, "dira", item = cJSON_CreateObject());
	memset(tmp,0x00,sizeof(tmp));
	sprintf(tmp,"%d",port);
	cJSON_AddStringToObject(item, "usb_port", tmp);


	
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
	recvlen = my_recv_data_len(s_client);
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
	//WriteLogFile("dev_info ����json���ݿ�ʼ\n");
	result = my_socket_read(s_client,recvbody, recvlen);
	if (result < 0)
	{

		free(recvbody);
		WriteLogFile("dev_info set_usbshare_ip_new recv body error!");
		shutdown(s_client, SD_BOTH);
		closesocket(s_client);
		return DLL_ERR_NEW_SHARE_SOCKET_RECV;
	}
	//WriteLogFile("dev_info ����json�������:re_read:%d\n", re_read);
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
	//memcpy(out_ter_info, recvbody, strlen(recvbody));
	//WriteLogFile("dev_info ���յ��İ���Ϊ:%d!", strlen(out_ter_info));
	WriteLogFile("restart_signle_port:%s!", recvbody);
	cJSON_Delete(rt);
	free(noformat);
	free(recvbody);
	//WriteLogFile("dev_info get_usbshare_status_new exit!!!!!\n");

	return 0;
}

int judge_port_exit(char *terid,int port_data)
{
	char ter_id[32] = {0x00};
	char *ter_info = calloc(1,1024*1024);
	int ter_info_len = 0,usb_start_len = 0;
	char tmp[32]= {0x00};
	char *gbuf = NULL;
	int port = 0,i = 0,j = 0,ret = 0,force_close_flag = 0 ,result = 0;
	struct CabinetInfo *cabinet = NULL;
	SOCKET s_client;
	cJSON *root = NULL;
	cJSON *head = NULL;
	HINSTANCE hDLL;
	fun_send_cmd_deal send_cmd;
	fun_init_dll_interface fun_init;
	fun_clean_dll_interface fun_deinit;
	struct file_setting file_setting[128] = {0};

	char *g_buf;
	char send_json[4*1024] = { 0 };
	int len;
	char set_path[MAX_PATH] = { 0 }, *c_DestPath = NULL, ip_asc[100] = { 0 }, buf_prefix[30];
	struct hostent *h;

	if (!GetModuleFileName(NULL, set_path, sizeof(set_path))){
		WriteLogFile("��ȡ��ǰ·������");
		if(NULL != ter_info)
			free(ter_info);
		return FALSE;
	}
	*(strrchr(set_path,'\\')+1) = 0;
	//WriteLogFile("set_path : %s",set_path);
	//c_DestPath = strcat(set_path, "ModelDll\\ModelApiDll\\Client.set");

	strcat(set_path, "ModelDll\\ModelApiDll\\dev_info.dll");
	// strcat(set_path, "ModelApiDll\\dev_info.dll");

	
	memcpy(ter_id, terid, 12);

	cabinet = (struct CabinetInfo*)calloc(1, sizeof(struct CabinetInfo) * 128);
	if (NULL == cabinet)
	{
		if(NULL != ter_info)
			free(ter_info);
		return -1;
	}
	ret = terminal_query_cabinet_attribute(cabinet, 128);

	for(i = 0;i<128;++i)
	{
		if(0 == memcmp(cabinet[i].ca_TerminalID,ter_id,strlen(ter_id)))  //�ҵ���Ӧ�Ļ���id
		{
			WriteLogFile("enetr get_info_with_terID,�ҵ���Ӧid = %s",cabinet[i].ca_TerminalID);
			break;
		}
	}
	if(i == 128)
	{
		if(NULL != cabinet)
			free(cabinet);
		if(NULL != ter_info)
			free(ter_info);
		WriteLogFile("get_info_with_terID δ�ҵ���Ӧid");
		return -1;
	}
	result = read_setting(file_setting, 128);
	if(result < 0)
	{
		if(NULL != cabinet)
			free(cabinet);
		if(NULL != ter_info)
			free(ter_info);
		WriteLogFile("read_setting failed");
		return -1;
	}
	for(j = 0;j < result;++j)
	{
		if(0 == memcmp(file_setting[j].terminal_ip,cabinet[i].ca_SettingIP,strlen(cabinet[i].ca_SettingIP)) && file_setting[j].data_port == cabinet[i].dataport)
		{
			break;
		}
	}
	if(j == result)
	{
		if(NULL != cabinet)
			free(cabinet);
		if(NULL != ter_info)
			free(ter_info);
		WriteLogFile("dev_info can not find mattched ip");
		return -1;
	}

	root = cJSON_CreateObject();
	cJSON_AddStringToObject(root, "cmd", "get_ter_status");
	cJSON_AddStringToObject(root, "server_ip", cabinet[i].ca_SettingIP);
	memset(tmp,0x00,sizeof(tmp));
	sprintf(tmp,"%d",cabinet[i].dataport);
	cJSON_AddStringToObject(root, "server_port",tmp);

	gbuf = cJSON_PrintUnformatted(root);

	WriteLogFile("get_info_with_terID gbuf=%s\n",gbuf);

	cJSON_Delete(root);

	hDLL=LoadLibrary(set_path);//���ض�̬���ӿ�MyDll.dll�ļ���

	if(NULL == hDLL)
	{
		WriteLogFile("NULL == hDLL\n");
	}

	fun_init = (fun_init_dll_interface)GetProcAddress(hDLL,"init_dll_interface");
	//fun_deinit = (fun_clean_dll_interface)GetProcAddress(hDLL,"clean_dll_interface");
	send_cmd=(fun_send_cmd_deal)GetProcAddress(hDLL,"send_cmd_deal");
	fun_init(10);
	memset(my_error_buff,0x00,sizeof(my_error_buff));
	if(NULL == send_cmd)
	{
		WriteLogFile("NULL == send_cmd\n");
	}
	WriteLogFile("enter send_cmd\n");
	ret = send_cmd(gbuf,&apply_reslut_callback,ter_info);

	ter_info_len = strlen(ter_info);

	WriteLogFile("apply_reslut_callback = %s",my_error_buff);
	//WriteLogFile("ter_info = %s,\n len = %d\n",ter_info,strlen(ter_info));

	// fun_deinit();
	FreeLibrary(hDLL);//ж��MyDll.dll�ļ���
	WriteLogFile("enter ter_info\n");
	root = cJSON_Parse(ter_info);
	if (root == NULL)
	{
		WriteLogFile("ter_info is not a json!");
		
		if(NULL != ter_info)
			free(ter_info);
		if(NULL != cabinet)
			free(cabinet);
		if(NULL != gbuf)
	{
		free(gbuf);
	}
		return 0;
	}
	cJSON *object_data = cJSON_GetObjectItem(root, "data");
	if (object_data == NULL)
	{
		WriteLogFile("ter_info object_data == NULL!");
		cJSON_Delete(root);
		if(NULL != ter_info)
			free(ter_info);
		if(NULL != cabinet)
			free(cabinet);
		if(NULL != gbuf)
	{
		free(gbuf);
	}
		return 0;
	}
	int size = cJSON_GetArraySize(object_data);
	cJSON *arrayItem = cJSON_GetArrayItem(object_data, 0);
	if (arrayItem == NULL)
	{
		WriteLogFile("ter_info arrayItem == NULL!");
		cJSON_Delete(root);
		if(NULL != ter_info)
			free(ter_info);
		if(NULL != cabinet)
			free(cabinet);
		if(NULL != gbuf)
	{
		free(gbuf);
	}
		return 0;
	}
	cJSON *usb_data = cJSON_GetObjectItem(arrayItem, "usb_port_manage");
	if (usb_data == NULL)
	{
		WriteLogFile("ter_info usb_data == NULL!");
		cJSON_Delete(root);
		if(NULL != ter_info)
			free(ter_info);
		if(NULL != cabinet)
			free(cabinet);
		if(NULL != gbuf)
	{
		free(gbuf);
	}
		return 0;
	}
	cJSON *usb_array_item = cJSON_GetObjectItem(usb_data, "usb_port_info");
	if (usb_array_item == NULL)
	{
		WriteLogFile("ter_info usb_array_item == NULL!");
		cJSON_Delete(root);
		if(NULL != ter_info)
			free(ter_info);
		if(NULL != cabinet)
			free(cabinet);
		if(NULL != gbuf)
	{
		free(gbuf);
	}
		return 0;
	}
	size = cJSON_GetArraySize(usb_array_item);
	cJSON *usb_arry,*usb_item;

	int port_num = 0,dev_exit = 0,usb_share = 0;
	for(i = 0;i<size;i++)
	{
		usb_arry = cJSON_GetArrayItem(usb_array_item, i);
		if (usb_arry == NULL)
		{
			WriteLogFile("ter_info usb_arry == NULL!");
			cJSON_Delete(root);
			if(NULL != ter_info)
				free(ter_info);
			if(NULL != cabinet)
			free(cabinet);
			if(NULL != gbuf)
	{
		free(gbuf);
	}
			return 0;
		}
		usb_item = cJSON_GetObjectItem(usb_arry, "usb_port");
		if (usb_item == NULL)
		{
			WriteLogFile("ter_info usb_item == NULL!");
			cJSON_Delete(root);
			if(NULL != ter_info)
				free(ter_info);
			if(NULL != cabinet)
			free(cabinet);
		if(NULL != gbuf)
	{
		free(gbuf);
	}
			return 0;
		}
		port_num = atoi(usb_item->valuestring);
		
		usb_item = cJSON_GetObjectItem(usb_arry, "dev_exists");
		if (usb_item == NULL)
		{
			WriteLogFile("ter_info usb_item == NULL!");
			cJSON_Delete(root);
			if(NULL != ter_info)
				free(ter_info);
			if(NULL != cabinet)
			free(cabinet);
			if(NULL != gbuf)
	{
		free(gbuf);
	}
			return 0;
		}
		dev_exit = atoi(usb_item->valuestring);
		if (port_num == port_data)
		{
			if(dev_exit == 1)
			{
				WriteLogFile("port_num = %d port_data = %d,dev_exit = %d\n", port_num, port_data, dev_exit);
				cJSON_Delete(root);
				if(NULL != ter_info)
					free(ter_info);
				if(NULL != cabinet)
			free(cabinet);
			if(NULL != gbuf)
	{
		free(gbuf);
	}
				return 1;
			}
			else
			{
				WriteLogFile("port_num = %d port_data = %d,dev_exit = %d\n", port_num, port_data, dev_exit);
				cJSON_Delete(root);
				if(NULL != ter_info)
					free(ter_info);
				if(NULL != cabinet)
			free(cabinet);
			if(NULL != gbuf)
	{
		free(gbuf);
	}
				return 0;
			}

		}
		usb_item = cJSON_GetObjectItem(usb_arry, "usb_share");
		if (usb_item == NULL)
		{
			WriteLogFile("ter_info usb_item == NULL!");
			cJSON_Delete(root);
			if(NULL != ter_info)
				free(ter_info);
			if(NULL != cabinet)
			free(cabinet);
			if(NULL != gbuf)
	{
		free(gbuf);
	}
			return 0;
		}
		usb_share = atoi(usb_item->valuestring);
	}

	WriteLogFile("ter_info size = %d\n",size);

	// if(NULL != gbuf)
	// {
	// 	free(gbuf);
	// }

	// gbuf = cJSON_PrintUnformatted(usb_array_item);
	// cJSON_Delete(root);
	// WriteLogFile("ter_info gbuf = %s\n",gbuf);
	if(NULL != gbuf)
	{
		free(gbuf);
	}
	free(cabinet);
	
	if(NULL != ter_info)
		free(ter_info);
	return 0;


}


int get_info_with_terID(char *inbuff,char *outbuff)
{
	char ter_id[32] = {0x00};
	char *ter_info = calloc(1,1024*1024);
	int ter_info_len = 0,usb_start_len = 0;
	char tmp[32]= {0x00};
	char *gbuf = NULL;
	int port = 0,i = 0,j = 0,ret = 0,force_close_flag = 0 ,result = 0;
	struct CabinetInfo *cabinet = NULL;
	SOCKET s_client;
	cJSON *root = NULL;
	cJSON *head = NULL;
	HINSTANCE hDLL;
	fun_send_cmd_deal send_cmd;
	fun_init_dll_interface fun_init;
	fun_clean_dll_interface fun_deinit;
	struct file_setting file_setting[128] = {0};

	char *g_buf;
	char send_json[4*1024] = { 0 };
	int len;
	char set_path[MAX_PATH] = { 0 }, *c_DestPath = NULL, ip_asc[100] = { 0 }, buf_prefix[30];
	struct hostent *h;

	if (!GetModuleFileName(NULL, set_path, sizeof(set_path))){
		WriteLogFile("��ȡ��ǰ·������");
		return FALSE;
	}
	*(strrchr(set_path,'\\')+1) = 0;
	//WriteLogFile("set_path : %s",set_path);
	//c_DestPath = strcat(set_path, "ModelDll\\ModelApiDll\\Client.set");
	//strcat(set_path, "ModelDll\\ModelApiDll\\dev_info.dll");

	 strcat(set_path, "ModelDll\\ModelApiDll\\dev_info.dll");

	WriteLogFile("get_info_with_terID set_path : %s",set_path);

	WriteLogFile("enetr get_info_with_terID");
	
	memcpy(ter_id,inbuff,12);

	cabinet = (struct CabinetInfo*)calloc(1, sizeof(struct CabinetInfo) * 128);
	if (NULL == cabinet)
	{
		return -1;
	}
	ret = terminal_query_cabinet_attribute(cabinet, 128);

	for(i = 0;i<128;++i)
	{
		if(0 == memcmp(cabinet[i].ca_TerminalID,ter_id,strlen(ter_id)))  //�ҵ���Ӧ�Ļ���id
		{
			WriteLogFile("enetr get_info_with_terID,�ҵ���Ӧid = %s",cabinet[i].ca_TerminalID);
			break;
		}
	}
	if(i == 128)
	{
		if(NULL != cabinet)
			free(cabinet);
		WriteLogFile("get_info_with_terID δ�ҵ���Ӧid");
		return -1;
	}
	result = read_setting(file_setting, 128);
	if(result < 0)
	{
		if(NULL != cabinet)
			free(cabinet);
		WriteLogFile("read_setting failed");
		return -1;
	}
	for(j = 0;j < result;++j)
	{
		if(0 == memcmp(file_setting[j].terminal_ip,cabinet[i].ca_SettingIP,strlen(cabinet[i].ca_SettingIP)) && file_setting[j].data_port == cabinet[i].dataport)
		{
			break;
		}
	}
	if(j == result)
	{
		WriteLogFile("dev_info can not find mattched ip");
		return -1;
	}

	root = cJSON_CreateObject();
	cJSON_AddStringToObject(root, "cmd", "get_ter_status");
	cJSON_AddStringToObject(root, "server_ip", cabinet[i].ca_SettingIP);
	memset(tmp,0x00,sizeof(tmp));
	sprintf(tmp,"%d",cabinet[i].dataport);
	cJSON_AddStringToObject(root, "server_port",tmp);

	gbuf = cJSON_PrintUnformatted(root);

	WriteLogFile("get_info_with_terID gbuf=%s\n",gbuf);

	cJSON_Delete(root);

	hDLL=LoadLibrary(set_path);//���ض�̬���ӿ�MyDll.dll�ļ���

	if(NULL == hDLL)
	{
		WriteLogFile("NULL == hDLL\n");
	}

	fun_init = (fun_init_dll_interface)GetProcAddress(hDLL,"init_dll_interface");
	fun_deinit = (fun_clean_dll_interface)GetProcAddress(hDLL,"clean_dll_interface");
	send_cmd=(fun_send_cmd_deal)GetProcAddress(hDLL,"send_cmd_deal");
	fun_init(10);
	memset(my_error_buff,0x00,sizeof(my_error_buff));
	if(NULL == send_cmd)
	{
		WriteLogFile("NULL == send_cmd\n");
	}
	WriteLogFile("enter send_cmd\n");
	ret = send_cmd(gbuf,&apply_reslut_callback,ter_info);

	ter_info_len = strlen(ter_info);

	WriteLogFile("apply_reslut_callback = %s",my_error_buff);
	//WriteLogFile("ter_info = %s,\n len = %d\n",ter_info,strlen(ter_info));

	//fun_deinit();
	//WriteLogFile("enter ter_info\n");
	FreeLibrary(hDLL);//ж��MyDll.dll�ļ���
	root = cJSON_Parse(ter_info);
	if (root == NULL)
	{
		WriteLogFile("ter_info is not a json!");
		if(NULL != ter_info)
			free(ter_info);
		return 0;
	}
	cJSON *object_data = cJSON_GetObjectItem(root, "data");
	if (object_data == NULL)
	{
		WriteLogFile("ter_info object_data == NULL!");
		cJSON_Delete(root);
		if(NULL != ter_info)
			free(ter_info);
		return 0;
	}
	int size = cJSON_GetArraySize(object_data);
	cJSON *arrayItem = cJSON_GetArrayItem(object_data, 0);
	if (arrayItem == NULL)
	{
		WriteLogFile("ter_info arrayItem == NULL!");
		cJSON_Delete(root);
		if(NULL != ter_info)
			free(ter_info);
		return 0;
	}
	cJSON *usb_data = cJSON_GetObjectItem(arrayItem, "usb_port_manage");
	if (usb_data == NULL)
	{
		WriteLogFile("ter_info usb_data == NULL!");
		cJSON_Delete(root);
		if(NULL != ter_info)
			free(ter_info);
		return 0;
	}
	cJSON *usb_array_item = cJSON_GetObjectItem(usb_data, "usb_port_info");
	if (usb_array_item == NULL)
	{
		WriteLogFile("ter_info usb_array_item == NULL!");
		cJSON_Delete(root);
		if(NULL != ter_info)
			free(ter_info);
		return 0;
	}
	size = cJSON_GetArraySize(usb_array_item);
	cJSON *usb_arry,*usb_item;

	ter_info_len = 0;
	memcpy(&outbuff[ter_info_len],cabinet[i].ca_SettingIP,strlen(cabinet[i].ca_SettingIP));
	ter_info_len += 24;
	outbuff[ter_info_len++] = cabinet[i].i_CmdPort>>24;
	outbuff[ter_info_len++] = cabinet[i].i_CmdPort>>16;
	outbuff[ter_info_len++] = cabinet[i].i_CmdPort>>8;
	outbuff[ter_info_len++] = cabinet[i].i_CmdPort>>0;
	outbuff[ter_info_len++] = cabinet[i].i_DataPort>>24;
	outbuff[ter_info_len++] = cabinet[i].i_DataPort>>16;
	outbuff[ter_info_len++] = cabinet[i].i_DataPort>>8;
	outbuff[ter_info_len++] = cabinet[i].i_DataPort>>0;
	outbuff[ter_info_len++] = 1;
	memcpy(&outbuff[ter_info_len],cabinet[i].ca_TerminalID,strlen(cabinet[i].ca_TerminalID));
	ter_info_len += 12;
	outbuff[ter_info_len++] = cabinet[i].i_USBSumNumber>>24;
	outbuff[ter_info_len++] = cabinet[i].i_USBSumNumber>>16;
	outbuff[ter_info_len++] = cabinet[i].i_USBSumNumber>>8;
	outbuff[ter_info_len++] = cabinet[i].i_USBSumNumber>>0;
	memcpy(&outbuff[ter_info_len],cabinet[i].ca_SoftVesion,strlen(cabinet[i].ca_SoftVesion));
	ter_info_len += 8;
	memcpy(&outbuff[ter_info_len],cabinet[i].ca_TerminalModel,strlen(cabinet[i].ca_TerminalModel));
	ter_info_len += 10;
	memset(&outbuff[ter_info_len],0x00,256);
	usb_start_len = ter_info_len;
	ter_info_len += 256;
	memcpy(&outbuff[ter_info_len],cabinet[i].ca_SoftName,strlen(cabinet[i].ca_SoftName));
	ter_info_len += 50;
	outbuff[ter_info_len++] = 1;
	memcpy(&outbuff[ter_info_len],cabinet[i].ca_ReleaseData,strlen(cabinet[i].ca_ReleaseData));
	ter_info_len += 10;
	memcpy(&outbuff[ter_info_len],cabinet[i].ca_SoftVersion,strlen(cabinet[i].ca_SoftVersion));
	ter_info_len += 100;
	memcpy(&outbuff[ter_info_len],cabinet[i].ca_Author,strlen(cabinet[i].ca_Author));
	ter_info_len += 50;
	memcpy(&outbuff[ter_info_len],cabinet[i].ca_SoftDescription,strlen(cabinet[i].ca_SoftDescription));
	ter_info_len += 50;
	memcpy(&outbuff[ter_info_len],cabinet[i].ca_CodeType,strlen(cabinet[i].ca_CodeType));
	ter_info_len += 20;

	int port_num = 0,dev_exit = 0,usb_share = 0;
	for(i = 0;i<size;i++)
	{
		usb_arry = cJSON_GetArrayItem(usb_array_item, i);
		if (usb_arry == NULL)
		{
			WriteLogFile("ter_info usb_arry == NULL!");
			cJSON_Delete(root);
			if(NULL != ter_info)
				free(ter_info);
			return 0;
		}
		usb_item = cJSON_GetObjectItem(usb_arry, "usb_port");
		if (usb_item == NULL)
		{
			WriteLogFile("ter_info usb_item == NULL!");
			cJSON_Delete(root);
			if(NULL != ter_info)
				free(ter_info);
			return 0;
		}
		port_num = atoi(usb_item->valuestring);
		usb_item = cJSON_GetObjectItem(usb_arry, "dev_exists");
		if (usb_item == NULL)
		{
			WriteLogFile("ter_info usb_item == NULL!");
			cJSON_Delete(root);
			if(NULL != ter_info)
				free(ter_info);
			return 0;
		}
		dev_exit = atoi(usb_item->valuestring);
		usb_item = cJSON_GetObjectItem(usb_arry, "usb_share");
		if (usb_item == NULL)
		{
			WriteLogFile("ter_info usb_item == NULL!");
			cJSON_Delete(root);
			if(NULL != ter_info)
				free(ter_info);
			return 0;
		}
		usb_share = atoi(usb_item->valuestring);
		outbuff[usb_start_len + port_num -1] = dev_exit + usb_share;
	}

	WriteLogFile("ter_info size = %d\n",size);

	// if(NULL != gbuf)
	// {
	// 	free(gbuf);
	// }

	// gbuf = cJSON_PrintUnformatted(usb_array_item);
	// cJSON_Delete(root);
	// WriteLogFile("ter_info gbuf = %s\n",gbuf);
	if(NULL != gbuf)
	{
		free(gbuf);
	}
	free(cabinet);
	
	if(NULL != ter_info)
		free(ter_info);
	return ter_info_len;


}

int protocol_analysis(unsigned char *inbuff,int len,char *outbuff)
{
	int ret = 0;
	WriteLogFile("inbuff[4]=%d\n",inbuff[4]);
	switch(inbuff[4])
	{
		case 0x89:  //��ѯ�豸״̬
			ret = get_all_online_dev_status(&outbuff[5]);
			if(ret)
			{
				outbuff[0] = ret>>24;
				outbuff[1] = ret>>16;
				outbuff[2] = ret>>8;
				outbuff[3] = ret>>0;
				outbuff[4] = 0xa9;
				ret += 5;
			}
		break;

		case 0x80:  //����˿ڼ���
			ret = apply_port(&inbuff[5]);
			WriteLogFile("apply_port ret= %d\n",ret);
			if(0 == ret)
			{
				outbuff[0] = 0x00;
				outbuff[1] = 0x00;
				outbuff[2] = 0x00;
				outbuff[3] = 0x01;
				outbuff[4] = 0xa0;
				ret += 5;
			}
			else
			{
				outbuff[0] = 0x00;
				outbuff[1] = 0x00;
				outbuff[2] = 0x00;
				outbuff[3] = 0x01;
				outbuff[4] = 0xa1;
				ret = 5;
			}
				
		break;

		case 0x81:	//�ͷŶ˿�
			ret = relese_port(&inbuff[5]);
			//WriteLogFile("relese_port ret= %d\n",ret);
			if(!ret)
			{
				outbuff[0] = 0x00;
				outbuff[1] = 0x00;
				outbuff[2] = 0x00;
				outbuff[3] = 0x01;
				outbuff[4] = 0xa1;
				ret += 5;
			}
			else
				ret = 0;
		break;

		case 0x83:	//��������ָ���ն�
			ret = reboot_terminal_nb(&inbuff[5]);
			//WriteLogFile("relese_port ret= %d\n",ret);
			if(!ret)
			{
				outbuff[0] = 0x00;
				outbuff[1] = 0x00;
				outbuff[2] = 0x00;
				outbuff[3] = 0x01;
				outbuff[4] = 0xa3;
				ret += 5;
			}
			else
				ret = 0;
		break;

		case 0x8a: //���������˿�
			ret = restart_signle_port(&inbuff[5]);
			//WriteLogFile("relese_port ret= %d\n",ret);
			if(!ret)
			{
				outbuff[0] = 0x00;
				outbuff[1] = 0x00;
				outbuff[2] = 0x00;
				outbuff[3] = 0x01;
				outbuff[4] = 0xaa;
				ret += 5;
			}
			else
				ret = 0;
		break;

		case 0x8b: //������е��
			ret = start_robot_arm_nb(&inbuff[5]);
			WriteLogFile("start_robot_arm_nb________1 ret= %d\n",ret);
			if(!ret)
			{
				outbuff[0] = 0x00;
				outbuff[1] = 0x00;
				outbuff[2] = 0x00;
				outbuff[3] = 0x01;
				outbuff[4] = 0xaa;
				ret += 5;
			}
			else
				ret = 0;
		break;

		case 0x96: //����id��ȡ�ն���ϸ��Ϣ
		ret = get_info_with_terID(&inbuff[5],&outbuff[5]);
		if(ret > 0)
		{
			outbuff[0] = ret>>24;
			outbuff[1] = ret>>16;
			outbuff[2] = ret>>8;
			outbuff[3] = ret>>0;
			outbuff[4] = 0xb6;
			ret += 5;
		}
		else
			ret = 0;
		break;

		default:
		break;
	}
	return ret;
}

#define TRANS_PORT 8110
#define MAX_CLIENT_NUM 127
#define REC_BUFF_LEN 4096

int client_id[MAX_CLIENT_NUM] = {-1};
int server_fd;

DWORD WINAPI thrans_thread(LPVOID lpParam)
{
	int i = 0,num = 0,count = 0,max_fd = 0,client_sock_fd = -1,ret = 0,rec_len = 0;
    struct timeval mytime;
	struct sockaddr_in client_address;
	socklen_t address_len; 
	char rec_buff[REC_BUFF_LEN] = {0x00};
	char *sebd_buff = calloc(1,1024*1024);
	fd_set stFdSet;
    mytime.tv_sec = 2;
    mytime.tv_usec = 0;
	// WriteLogFile("enter thrans_thread\n");
	// WriteLogFile("thrans_task_init server_fd = %d,api_nThreadFlag =%d\n",server_fd,api_nThreadFlag);
	while (api_nThreadFlag)
	{
		//WriteLogFile("enter thrans_thread1111\n");
		FD_ZERO(&stFdSet);
		FD_SET(server_fd, &stFdSet);
		max_fd = max_fd>server_fd?max_fd:server_fd;
		//WriteLogFile("thrans_task_init max_fd11111 = %d\n",max_fd);
		for(i = 0;i<MAX_CLIENT_NUM;i++)
		{
			if(-1 != client_id[i])
			{
				//WriteLogFile("client_id[%d]=%d\n",i,client_id[i]);
				FD_SET(client_id[i], &stFdSet);
				max_fd = max_fd>client_id[i]?max_fd:client_id[i];
			}
		}
		//WriteLogFile("thrans_task_init max_fd22222 = %d\n",max_fd);
		ret = select(max_fd + 1, &stFdSet, NULL, NULL, &mytime);

		//WriteLogFile("thrans_thread = %d\n",ret);
		if(ret>0)
		{
			
			if(FD_ISSET(server_fd, &stFdSet))
			{
				//WriteLogFile("server_fd=%d\n",server_fd);
				address_len = sizeof(struct sockaddr_in);
				client_sock_fd = accept(server_fd,(struct sockaddr *)&client_address, &address_len);
				//WriteLogFile("client_sock_fd=%d\n",client_sock_fd);
				if(client_sock_fd > 0)
				{
					for(i = 0;i<MAX_CLIENT_NUM;i++)
					{
						if(-1 == client_id[i])
						{
							WriteLogFile("new connection\n");
							client_id[i] = client_sock_fd;
							break;
						}
					}
					if(i==MAX_CLIENT_NUM)
					{
						WriteLogFile("too many clients!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
					}
				}
			}
			for(i = 0;i<MAX_CLIENT_NUM;i++) 
			{
				//printf("sever_struct.multiple_client[%d].client_fd = %d\n",i,sever_struct.multiple_client[i].client_fd);
				if(client_id[i]<0)
				{
					continue;
				}
				if(FD_ISSET(client_id[i], &stFdSet)) 
				{
					memset(rec_buff,0x00,REC_BUFF_LEN);
					rec_len = recv(client_id[i],rec_buff,REC_BUFF_LEN,0);
					if(rec_len > 0) //���յ�����
					{
						WriteLogFile("recdata rec_len=%d\n",rec_len);
						for(count = 0;count<rec_len;++count)
						{
							WriteLogFile("recdata rec_buff[%d]=%02x\n",count,(unsigned char)rec_buff[count]);
						}
						
						//Э�����
						memset(sebd_buff,0x00,sizeof(sebd_buff));
						ret = protocol_analysis((unsigned char*)rec_buff,rec_len,sebd_buff);
						if(ret)
						{
							send(client_id[i],sebd_buff,ret,0);
						}
					}
					else	//�����ݣ��ر�socket
					{
						WriteLogFile("discollected\n");
						socket_close(client_id[i]);
            			client_id[i] = -1;
					}
				}
			}
		}
		Sleep(100);
	}
	for(i = 0;i<MAX_CLIENT_NUM;i++) 
	{
		//printf("sever_struct.multiple_client[%d].client_fd = %d\n",i,sever_struct.multiple_client[i].client_fd);
		if(client_id[i]<0)
		{
			continue;
		}
		socket_close(client_id[i]);
		client_id[i] = -1;
	}
	WriteLogFile("exit thrans_thread\n");
	if(sebd_buff != NULL)
		free(sebd_buff);
	socket_close(server_fd);
	return 0;
}

BOOL thrans_task_init(void)
{
	HANDLE handle;
	struct sockaddr_in sin;
	int iOn = 1,iMode = 1;
	memset(client_id,-1,sizeof(int)*MAX_CLIENT_NUM);

	//server_fd = tcp_listen(NULL, port, MAX_CLIENT_NUM);

	sin.sin_addr.S_un.S_addr = htonl(INADDR_ANY);//inet_addr(SERVER_IP);
	//sin.sin_addr.s_addr = htonl(INADDR_ANY);  SERVER_IP
	sin.sin_family = AF_INET;
	sin.sin_port = htons(TRANS_PORT);
	server_fd = socket(AF_INET, SOCK_STREAM, 0);
	WriteLogFile("thrans_task_init server_fd = %d\n",server_fd);
	if (server_fd == SOCKET_ERROR){
		WriteLogFile("socket����ʧ��,GetLastError = %d\n" , GetLastError());
		return SOCKET_ERROR;
	}
	if(setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &iOn, sizeof(iOn)) < 0)
	{
		WriteLogFile("setsockopt error\n");
	}
	iMode = 1;
	ioctlsocket(server_fd, FIONBIO, (u_long FAR*)&iMode); //����Ϊ������ģʽ
	if(bind(server_fd,(struct sockaddr*)(&sin),sizeof(sin)) < 0)
	{
		WriteLogFile("bind error\n");
	}
	if (listen(server_fd, MAX_CLIENT_NUM) < 0)
	{
    	WriteLogFile("listen error\n");
  	}


	// setsockopt(clifd, SOL_SOCKET, SO_RCVBUF, (const char*)&nRecvBuf, sizeof(int));
	// setsockopt(clifd, SOL_SOCKET, SO_SNDBUF, (const char*)&nSendBuf, sizeof(int));
	// setsockopt(clifd, SOL_SOCKET, SO_DONTLINGER, (const char*)&bDontLinger, sizeof(BOOL));
	// setsockopt(clifd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout.tv_sec, sizeof(timeout));
	// setsockopt(clifd, SOL_SOCKET, SO_SNDTIMEO, (const char*)&timeout.tv_sec, sizeof(timeout));

	//if (setsockopt(clifd, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse0, sizeof(reuse0)) == -1) return SOCKET_ERROR;

	handle = CreateThread(NULL, 0, thrans_thread, NULL, 0, NULL);
	if (handle == NULL)
	{
		WriteLogFile("Create thrans_thread Thread err");
		return FALSE;
	}
	return TRUE;
}


/*********************************************************************
-   Function : init_dll_interface
-   Description����ʼ����dll��һЩȫ�֡�������Դ�����̼߳��������
-   Calls��
-   Calles By:
-   Table Accessed :
-   Table Update :
-   Input  :
-   Output :
-   Return :0 ���� ���� ������Ӧ�Ĵ�����(ͨ���ĵ�������Ӧ�Ĵ�������)
-   Other :
***********************************************************************/
int start_flag = 0;
EXPORT_DLL int init_dll_interface(int time)
{
	int ret = 0;
	int i = 0;
	unsigned short version = 0x202; /* winsock 2.2 */
	WSADATA data;
	// ��ȡ�����ļ�ȫ·��(�����ļ���)
	//_snprintf((char*)g_szLogFile, sizeof(g_szLogFile)-1, "C:\\WriteLog.log");
	WriteLogFile("dll_oper init_dll_interface");
	if(start_flag == 0)
	{
		start_flag = 1;
		usbip_hDLL=LoadLibrary("ModelDll\\ModelApiDll\\usbip.dll");
		if(usbip_hDLL == NULL)
		{
			WriteLogFile("LoadLibrary err");
		}
		WriteLogFile("dll_oper init_dll_interface");
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
		

		/*�����ļ����ʼ��*/
		if (!terminal_setting_init()){
			WriteLogFile("�����ļ���ʼ��ʧ��");
			return 0;
		}
		/*�ն���Ϣ���ʼ��*/
		if (!terminal_init()){
			return 0;
		}
		Sleep(2 * 1000);
		main_Handle = CreateThread(NULL, 0, main_thread, NULL, 0, NULL);
		if (main_Handle == NULL)
		{
			WriteLogFile("Create main_handle Thread err");
			return DLL_ERROR_SYSTEM;
		}
		WriteLogFile("create thrans_thread\n");
		if(!thrans_task_init())
		{
			WriteLogFile("����ת������ʧ��");
			return 0;
		}
	}
	else
	{
		WriteLogFile("dll_oper already start!!!!!!!");
	}
	return 0;
}




/*********************************************************************
-   Function : clean_dll_interface
-   Description���ͷű�dll�д򿪵�������Դ���̷߳���
-   Calls��
-   Calles By:
-   Table Accessed :
-   Table Update :
-   Input  :
-   Output :
-   Return :0 ���� ���� ������Ӧ�Ĵ�����(ͨ���ĵ�������Ӧ�Ĵ�������)
-   Other :
***********************************************************************/
EXPORT_DLL int clean_dll_interface()
{
	int i;
	WriteLogFile("clean_dll_interface\n");
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
	CloseHandle(Mutex_old);//�رջ�����
	CloseHandle(main_mutex);
	FreeLibrary(usbip_hDLL);//ж��MyDll.dll�ļ���
	
	start_flag = 0;
	return ret;
}


/*********************************************************************
-   Function : dev_find_thread_old
-   Description����Э�������udp�����߳�
-   Calls��
-   Calles By:
-   Table Accessed :
-   Table Update :
-   Input  :����һ:
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
	// �������׽���   
	SOCKADDR_IN sin;
	sin.sin_family = AF_INET;
	//sin.sin_port = htons(local_port);
	sin.sin_port = 0;
	if (info == NULL)//һ������
		sin.sin_addr.s_addr = 0;
	else
		sin.sin_addr.S_un.S_addr = inet_addr(info->ip);

	int opt = 1;
	// sockfdΪ��Ҫ�˿ڸ��õ��׽���
	setsockopt(connect_socket, SOL_SOCKET, SO_REUSEADDR, (const void *)&opt, sizeof(opt));

	//�����������ϵĹ㲥��ַ��������   
	SOCKADDR_IN sin_from;
	sin_from.sin_family = AF_INET;
	sin_from.sin_port = htons(10001);
	sin_from.sin_addr.s_addr = INADDR_BROADCAST;

	//���ø��׽���Ϊ�㲥���ͣ�   
	bool bOpt = true;
	setsockopt(connect_socket, SOL_SOCKET, SO_BROADCAST, (char*)&bOpt, sizeof(bOpt));

	// ���׽���   
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
	//��װUDP ������
	memcpy(szMsg, "\x01\x10", 2);//Э��ͷ
	memcpy(szMsg+2, "\x00\x06", 2);//����λ
	memcpy(szMsg +4, "\x04\x01", 2);//������
	int nLen = sizeof(sin_from);
	if (SOCKET_ERROR == sendto(connect_socket, szMsg, 6, 0, (SOCKADDR*)&sin_from, nLen))
	{
		return -1;
	}
	int j, repeat_flag = 0;
	while (1)
	{
		// ��������   
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
		//���Э��汾��
		if (memcmp(buff, "\x01\x10\x00\x30\x14\x01", 6) != 0)
			continue;
		WaitForSingleObject(Mutex_old, INFINITE);
		char tmp_id[16] = { 0 };
		memcpy(tmp_id, buff + 14, 12);

		//����Ƿ��Ѿ��յ���Ӧ�𣨲��أ�
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
		//�������ݱ���
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
-   Description����Э�������udp�����߳�
-   Calls��
-   Calles By:
-   Table Accessed :
-   Table Update :
-   Input  :����һ: 
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
	// �������׽���   
	SOCKADDR_IN sin;
	sin.sin_family = AF_INET;
	//sin.sin_port = 0;
	sin.sin_port = htons(10003);
	

	if (info == NULL)
		sin.sin_addr.s_addr = 0;
	else
		sin.sin_addr.S_un.S_addr = inet_addr(info->ip);
		//sin.sin_addr.s_addr = INADDR_ANY;

	

	// �����������ϵĹ㲥��ַ��������   
	SOCKADDR_IN sin_from;
	sin_from.sin_family = AF_INET;
	sin_from.sin_port = htons(10001);
	sin_from.sin_addr.s_addr = INADDR_BROADCAST;



	//���ø��׽���Ϊ�㲥���ͣ�   
	bool bOpt = true;
	setsockopt(udp_socket, SOL_SOCKET, SO_BROADCAST, (char*)&bOpt, sizeof(bOpt));

	// ���׽���   
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
	


	//����½ӿ�Э��json���Ͱ�
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
		// ��������   
		
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
-   Description����ȡ������������״̬
-   Calls��
-   Calles By:
-   Table Accessed :
-   Table Update :
-   Input  :����һ: nIndex:��ǰ����Ҫ��ȡ״̬����������
-
-   Output :
-   Return :0 ���� ���� ������Ӧ�Ĵ�����(ͨ���ĵ�������Ӧ�Ĵ�������)
-   Other :
***********************************************************************/
int GetAdapterState(DWORD nIndex)
{
	MIB_IFROW miInfo;   // ��Ż�ȡ���� Adapter ����
	memset(&miInfo, 0, sizeof(MIB_IFROW));
	miInfo.dwIndex = nIndex;   // dwIndex ����Ҫ��ȡ�� Adapter ������
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

bool GetMacByGetAdaptersInfo(char *out)
{
	//WriteLogFile("enter GetMacByGetAdaptersInfo\n");
	bool ret = false;
	char acMAC[32];
	char mac_array[10][128] = {0x00};
	char ip_array[10][128] = {0x00};
	int count = 0,i = 0;
	char *tmp = NULL;
	cJSON *p_layer,*root,*dir2;
	int ulOutBufLen = sizeof(IP_ADAPTER_INFO);
	PIP_ADAPTER_INFO pAdapterInfo = (IP_ADAPTER_INFO*)malloc(sizeof(IP_ADAPTER_INFO));
	if(pAdapterInfo == NULL)
	{
		WriteLogFile("pAdapterInfo == NULL\n");
		return false;
	}
		
	// Make an initial call to GetAdaptersInfo to get the necessary size into the ulOutBufLen variable
	if(GetAdaptersInfo(pAdapterInfo, &ulOutBufLen) == ERROR_BUFFER_OVERFLOW) 
	{
		free(pAdapterInfo);
		pAdapterInfo = (IP_ADAPTER_INFO *)malloc(ulOutBufLen);
		if (pAdapterInfo == NULL) 
		{
			WriteLogFile("pAdapterInfo == NULL  111\n");
			return false;
		}
	}
	if(GetAdaptersInfo(pAdapterInfo, &ulOutBufLen) == NO_ERROR)
	{
		for(PIP_ADAPTER_INFO pAdapter = pAdapterInfo; pAdapter != NULL; pAdapter = pAdapter->Next)
		{
			// ȷ������̫��
			// if(pAdapter->Type != MIB_IF_TYPE_ETHERNET || )
			// 	continue;
			// ȷ��MAC��ַ�ĳ���Ϊ 00-00-00-00-00-00
			if(pAdapter->AddressLength != 6)
				continue;
			// WriteLogFile("pAdapter->Type:%d\n", pAdapter->Type);
			memset(acMAC,0x00,sizeof(acMAC));
			sprintf(acMAC, "%02X-%02X-%02X-%02X-%02X-%02X",
				 (pAdapter->Address[0]),
				 (pAdapter->Address[1]),
				 (pAdapter->Address[2]),
				 (pAdapter->Address[3]),
				 (pAdapter->Address[4]),
				 (pAdapter->Address[5]));
			//WriteLogFile("acMAC:%s\n", acMAC);

			memset(&mac_array[count][0],0x00,128);
			memcpy(&mac_array[count][0],acMAC,strlen(acMAC));

			// memcpy(&total_mac[strlen(total_mac)],acMAC,strlen(acMAC));
			// memcpy(&total_mac[strlen(total_mac)],";",1);

			IP_ADDR_STRING *pIpAddrString =&(pAdapter->IpAddressList);
			IP_ADDR_STRING *getwayString =&(pAdapter->GatewayList);

			// WriteLogFile("pIpAddrString:%s,getwayString=%s\n", pIpAddrString->IpAddress.String,getwayString->IpAddress.String);
			memset(&ip_array[count][0],0x00,128);
			if(strlen(pIpAddrString->IpAddress.String) > 0 && strlen(getwayString->IpAddress.String) > 0 && (pAdapter->Type == 6 || pAdapter->Type == 71))
			{
				if(0 != memcmp("0.0.0.0",pIpAddrString->IpAddress.String,strlen("0.0.0.0")) && 0 != memcmp("0.0.0.0",getwayString->IpAddress.String,strlen("0.0.0.0")))
				{
					memcpy(&ip_array[count][0],pIpAddrString->IpAddress.String,strlen(pIpAddrString->IpAddress.String));
				}
			}
			count++;

			//ret = true;
			//break;
		}
	}

	root = cJSON_CreateObject();
	cJSON_AddStringToObject(root, "cmd", "get_mac");
	cJSON_AddItemToObject(root, "data", dir2 = cJSON_CreateArray());
	for(i = 0;i<count;++i)
	{
		if(strlen(ip_array[i]) > 0)
		{
			cJSON_AddItemToObject(dir2, "dira", p_layer = cJSON_CreateObject());
			cJSON_AddStringToObject(p_layer, "mac", mac_array[i]);
			cJSON_AddStringToObject(p_layer, "ip", ip_array[i]);
		}
	}
	
	tmp = cJSON_PrintUnformatted(root);
		
	memcpy(out, tmp, strlen(tmp));
	//WriteLogFile("tmp:%s\n", tmp);
	cJSON_Delete(root);
	free(tmp);
	free(pAdapterInfo);
	ret = true;
	return ret;
}


/*********************************************************************
-   Function : find_adapter_ip
-   Description�����ұ�������������ip
-   Calls��
-   Calles By:
-   Table Accessed :
-   Table Update :
-   Input  :����һ: total:�������������صĵ�ǰ���Ե���������ip����
-
-   Output :
-   Return :0 ���� ���� ������Ӧ�Ĵ�����(ͨ���ĵ�������Ӧ�Ĵ�������)
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
		// �ͷſռ�
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
			default://��������,Unknown type
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
-   Description���������豸����
-   Calls��
-   Calles By:
-   Table Accessed :
-   Table Update :
-   Input  :����һ: output:�������������صĵ�ǰ�����������л���ip�����Ϣ��json�ַ���
-			
-   Output :
-   Return :0 ���� ���� ������Ӧ�Ĵ�����(ͨ���ĵ�������Ӧ�Ĵ�������)
-   Other :
***********************************************************************/
//int dev_discovery(char *output)
//{
//
////����������죬��client_query_cabinet_attribute_A�����л�ȡ����
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
//	//��ȡ��ͬ������ip��Ϣ(���������������Ȳ����ǵ�����)
//	result = find_adapter_ip(&num);
//	if (result < 0)
//		return DLL_ERROR_GET_ADAPTER;
//
//	//WriteLogFile("find_adapter num is :%d",num);
//	for (i = 0; i < 10; i++)
//	{
//		memset(&info_ip[i], 0, sizeof(struct ip_info));
//	}
//	if (num >= 10)//���֧��10��IP
//		num = 10;
//	//���ݷ��ص���������IP����������Ӧ����������Э���߳�
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
//	//�ȴ������߳����߳̽���,���ͷ���Դ
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
//	//��ص����ذ�
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
	//����������죬��client_query_cabinet_attribute_A�����л�ȡ����
	cJSON *root = NULL, *dir2 = NULL, *p_layer = NULL;
	int i = 0, j = 0, ret = -1, ret1 = -1;
	char *tmp = NULL;
	char port_num[16] = { 0x00 };
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
		//WriteLogFile("#######lbc find j= %d", j);
		//WriteLogFile("#######lbc find ter_ip:%s,server = %d\n", file[j].remote_ip,file[j].remote_dataport);
		if (128 == j)
		{
			continue;
		}
		if(strlen(cabinet[i].ca_TerminalID))
		{
			cJSON_AddItemToObject(dir2, "dira", p_layer = cJSON_CreateObject());
			cJSON_AddStringToObject(p_layer, "ter_ip", file[j].terminal_ip);
			memset(port_num, 0, sizeof(port_num));
			sprintf(port_num, "%d", file[j].data_port);
			cJSON_AddStringToObject(p_layer, "server_port", port_num);
			cJSON_AddStringToObject(p_layer, "ter_id", cabinet[i].ca_TerminalID);
			memset(port_num, 0, sizeof(port_num));
			sprintf(port_num, "%d", cabinet[i].i_USBSumNumber);
			cJSON_AddStringToObject(p_layer, "usb_port_num", port_num);
			cJSON_AddStringToObject(p_layer, "ter_version", cabinet[i].ca_SoftVesion);
			cJSON_AddStringToObject(p_layer, "ter_type", cabinet[i].ca_TerminalModel);
			cJSON_AddStringToObject(p_layer, "protocol_ver", "v1.0.0.1");
			cJSON_AddStringToObject(p_layer, "online", "1");
		}
	}
	tmp = cJSON_PrintUnformatted(root);
	if (NULL == tmp)
	{
		cJSON_Delete(root);
		free(cabinet);
		return -1;
	}
	memcpy(output, tmp, strlen(tmp));
	//WriteLogFile("find dev:len:%d \n%s\n", strlen(tmp), output);
	cJSON_Delete(root);
	free(cabinet);
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

typedef struct
{
	struct server_ip_struct struct_server_ip;
	char ter_data_list[50*1024];
	int get_end_flag;
	int get_ters_all_info_num;

}GET_TERS_STRUCT_T;

DWORD WINAPI get_ters_all_info_thread(LPVOID lpParam)
{
	GET_TERS_STRUCT_T *get_ters_struct = (GET_TERS_STRUCT_T *)lpParam;
	int num = get_ters_struct->get_ters_all_info_num;
	WriteLogFile("get_ters_all_info_thread num = %d!",num);
	memset(get_ters_struct->ter_data_list,0x00,50*1024);
	if(num < 100)
	{
		get_usbshare_status_new(get_ters_struct->struct_server_ip.ip, get_ters_struct->struct_server_ip.port, get_ters_struct->ter_data_list);
		get_ters_struct->get_end_flag = 1;
	}
}

int get_ters_all_info(char *json_in, void *hook,char *output)
{
	WriteLogFile("enter get_ters_all_info_thread num !");
	HANDLE get_ters_all_info_Thread_handle[100];

	GET_TERS_STRUCT_T *get_ters_struct = NULL;

	char *g_buf = NULL;
	char server_ip_data[50] = { 0 };
	int server_port = 0,size = 0,i = 0,count = 0,time_count = 0;
	cJSON *root = NULL;
	cJSON *item = NULL;
	cJSON *object_data = NULL;
	cJSON *arrayItem = NULL;
	//���ʹ��
	cJSON *json = NULL,*dir1 = NULL,*dir2 = NULL,*p_layer = NULL;

	result_CallBack callback = (result_CallBack)hook;
	root = cJSON_Parse(json_in);
	if (root == NULL)
	{
		WriteLogFile("json in is not a json!");
		return DLL_ERROR_PARAM;
	}
	object_data = cJSON_GetObjectItem(root, "ter_data");
	if (object_data == NULL)
	{
		WriteLogFile("json in no object_data!");
		cJSON_Delete(root);
		return DLL_ERROR_PARAM;
	}
	size = cJSON_GetArraySize(object_data);
	WriteLogFile("11111111111111 get_ters_all_info,size = %d!",size);
	if(size > 0)
	{
		get_ters_struct = (GET_TERS_STRUCT_T *)malloc(sizeof(GET_TERS_STRUCT_T)*(size+1));
		if(get_ters_struct == NULL)
		{
			WriteLogFile("get_ters_struct == NULL!");
			cJSON_Delete(root);
			return DLL_ERROR_PARAM;
		}
	}
	for(i = 0;i<size;i++)
	{
		arrayItem = cJSON_GetArrayItem(object_data, i);
		if (arrayItem == NULL)
		{
			WriteLogFile("json in no arrayItem!");
			cJSON_Delete(root);
			free(get_ters_struct);
			return DLL_ERROR_PARAM;
		}
		item = cJSON_GetObjectItem(arrayItem, "server_ip");
		if (item == NULL)
		{
			WriteLogFile("get server_ip error!");
			cJSON_Delete(root);
			free(get_ters_struct);
			return DLL_ERROR_PARAM;
		}
		
		memset(server_ip_data,0x00,sizeof(server_ip_data));
		memcpy(server_ip_data, item->valuestring, strlen(item->valuestring));
		item = cJSON_GetObjectItem(arrayItem, "server_port");
		if (item == NULL)
		{
			WriteLogFile("get server_port error!");
			cJSON_Delete(root);
			free(get_ters_struct);
			return DLL_ERROR_PARAM;
		}
		server_port = atoi(item->valuestring);
		memset((get_ters_struct + i)->struct_server_ip.ip,0x00,sizeof((get_ters_struct + i)->struct_server_ip.ip));
		memcpy((get_ters_struct + i)->struct_server_ip.ip,server_ip_data,strlen(server_ip_data));
		(get_ters_struct + i)->struct_server_ip.port = server_port;
		(get_ters_struct + i)->get_end_flag = 0;
		(get_ters_struct + i)->get_ters_all_info_num = i;

		get_ters_all_info_Thread_handle[i] = CreateThread(NULL, 0, get_ters_all_info_thread, (get_ters_struct + i), 0, NULL);
	}
	cJSON_Delete(root);
	while(1) //���ȴ�4��
	{
		time_count++;
		count = 0;
		// if(time_count>40)
		// 	break;
		for(i = 0;i<size;i++)
		{
			if((get_ters_struct + i)->get_end_flag == 1)
				count++;
		}
		if(count == size)
		{
			break;
		}
		Sleep(100);
	}
	json = cJSON_CreateObject();
	cJSON_AddItemToObject(json, "head", dir1 = cJSON_CreateObject());
	cJSON_AddStringToObject(dir1, "protocol", "mb_usbshare_20191024");
	cJSON_AddStringToObject(dir1, "code_type", "response");
	cJSON_AddStringToObject(dir1, "cmd", "ters_status");
	cJSON_AddStringToObject(dir1, "source_topic", "123");
	cJSON_AddStringToObject(dir1, "random", "123456789");
	cJSON_AddStringToObject(dir1, "result", "s");
	cJSON_AddItemToObject(json, "data", dir2 = cJSON_CreateArray());
	for(i = 0;i<size;i++)
	{
		if((get_ters_struct + i)->get_end_flag == 1)
		{
			//(get_ters_struct + i)->get_end_flag = 0;
			cJSON_AddItemToObject(dir2, "dira", p_layer = cJSON_CreateObject());
			cJSON_AddStringToObject(p_layer, "ter_data", (get_ters_struct + i)->ter_data_list);
		}
		CloseHandle(get_ters_all_info_Thread_handle[i]);
	}
	g_buf = cJSON_PrintUnformatted(json);
    WriteLogFile("1111111111111111,len = %d, g_pub_buf = %s\n", strlen(g_buf),g_buf);
	memcpy(output, g_buf, strlen(g_buf));
	free(g_buf);
	cJSON_Delete(json);

	while(1)
	{
		count = 0;
		for(i = 0;i<size;i++)
		{
			if((get_ters_struct + i)->get_end_flag == 1)
			{
				count++;
			}
		}
		if(count == size)
		{
			free(get_ters_struct);
			break;
		}
	}
	WriteLogFile("exit get_ters_all_info\n");
	
	return 0;
}

// int get_ter_all_info(char *json_in, void *hook,char *output)
// {
// 	cJSON *root = NULL;
// 	cJSON *item = NULL;
// 	result_CallBack callback = (result_CallBack)hook;

// 	root = cJSON_Parse(json_in);
// 	if (root == NULL)
// 	{
// 		WriteLogFile("json in is not a json!");
// 		return DLL_ERROR_PARAM;
// 	}

// 	WriteLogFile("dev_info ��ȡ������Ϣ׼��!");
// 	//char all_ter_info[1024 * 80] = { 0 };
// 	char server_ip[50] = { 0 };
// 	int server_port;
// 	item = cJSON_GetObjectItem(root, "server_ip");
// 	if (item == NULL)
// 	{
// 		WriteLogFile("get server_ip error!");
// 		return DLL_ERROR_PARAM;
// 	}
// 	memcpy(server_ip, item->valuestring, strlen(item->valuestring));

// 	item = cJSON_GetObjectItem(root, "server_port");
// 	if (item == NULL)
// 	{
// 		WriteLogFile("get server_port error!");
// 		return DLL_ERROR_PARAM;
// 	}
// 	server_port = atoi(item->valuestring);
// 	//server_port = 13029;
// 	WriteLogFile("dev_info ��ȡ������Ϣ���!,server_port=%d",server_port);
// 	memset(output, 0, sizeof(output));
// 	get_usbshare_status_new(server_ip, server_port, output);

	
// 	return 0;
// }


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

	WriteLogFile("dev_info ��ȡ������Ϣ׼��!");
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

	//�õ�������ʱʱ��
	int _delay_time	= get_want_time(server_ip, server_port, intranet_ip ,external_ip);
	WriteLogFile("		_delay_time = %d ms\n",_delay_time);


	//server_port = 13029;
	WriteLogFile("dev_info ��ȡ������Ϣ���!,server_port=%d",server_port);
	memset(output, 0, sizeof(output));
	//get_usbshare_status_new(server_ip, server_port,intranet_ip, external_ip,output);
	get_usbshare_status_new_time(server_ip, server_port,intranet_ip, external_ip,output,_delay_time);

	WriteLogFile("output:%s	",output);


	//����������п��ޣ�����������֤��Ҫ���ֶ�
	char *g_buf;
	cJSON *rt_tmp = NULL;
	rt_tmp = cJSON_Parse(output);
	if (rt_tmp == NULL)
	{
		WriteLogFile("json in is not a json!____________");
		return DLL_ERROR_PARAM;
	}
	WriteLogFile("dev_info ��ȡ������Ϣ׼��_______!");

	cJSON *object_data = cJSON_GetObjectItem(rt_tmp, "data");
	if (object_data == NULL)
	{
		WriteLogFile("����Dataʧ��\r\n");
		cJSON_Delete(rt_tmp);
		WriteLogFile("get server_ip error!");
		return DLL_ERROR_PARAM;

	}     

	cJSON *obj = cJSON_GetArrayItem(object_data, 0);		// ��ȡ���������obj
	// item = cJSON_GetObjectItem(obj, "product_code");//intranet_delay
	// WriteLogFile("product_code:%s\r\n",item->valuestring);  //eth_manage
	object_data = cJSON_GetObjectItem(obj, "eth_manage");
	if (object_data == NULL)
	{
		cJSON_Delete(rt_tmp);
		WriteLogFile("get eth_manage error!");
		return DLL_ERROR_PARAM;
	}

	g_buf = cJSON_Print(object_data);
	WriteLogFile("g_buf=%s\r\n",g_buf);

	item = cJSON_GetObjectItem(object_data, "intranet_delay");
	if (item == NULL)
	{
		cJSON_Delete(rt_tmp);
		WriteLogFile("get intranet_delay error!");
		return DLL_ERROR_PARAM;
	}
	WriteLogFile("intranet_delay:%s\r\n",item->valuestring);  //eth_manage

	cJSON_Delete(root);
	cJSON_Delete(rt_tmp);
	return 0;
}

int get_mac(char *json_in, void *hook,char *output)
{
	 if(GetMacByGetAdaptersInfo(output) == true)
	 	return 0;
	return 1;
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

/*********************************************************************************/
int get_net_delay(char *json_in, void *hook, char *output)
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

	WriteLogFile("dev_info ��ȡ������Ϣ׼��!");
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
	WriteLogFile("___1___server_ip:%s\r\n",item->valuestring);
	memcpy(server_ip, item->valuestring, strlen(item->valuestring));

	item = cJSON_GetObjectItem(root, "server_port");
	if (item == NULL)
	{
		cJSON_Delete(root);
		WriteLogFile("get server_port error!");
		return DLL_ERROR_PARAM;
	}
	WriteLogFile("___2____server_port:%s\r\n",item->valuestring);
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

	// WriteLogFile("dev_info 		get_net_delay	intranet_time");
	// measure_latency(server_ip,server_port);
	WriteLogFile("dev_info 	get  intranet_time");
	get_usbshare_net_delay_time_test(server_ip, server_port,intranet_ip, external_ip);

}


// int upgrade_net_intranet(char *json_in, void *hook, char *output)
// {
// 	cJSON *root = NULL;
// 	cJSON *item = NULL;
// 	char intranet_ip[64] = {0x00},external_ip[64] = {0x00};
// 	result_CallBack callback = (result_CallBack)hook;

// 	char ter_id[64] = {0x00};
// 	int notice_type = -1;
// 	char url_buff[128] = {0x00};

// 	root = cJSON_Parse(json_in);
// 	if (root == NULL)
// 	{
// 		WriteLogFile("json in is not a json!");
// 		return DLL_ERROR_PARAM;
// 	}

// 	WriteLogFile("dev_info ��ȡ������Ϣ׼��!");
// 	//char all_ter_info[1024 * 80] = { 0 };
// 	char server_ip[50] = { 0 };
// 	int server_port;
// 	item = cJSON_GetObjectItem(root, "server_ip");
// 	if (item == NULL)
// 	{
// 		cJSON_Delete(root);
// 		WriteLogFile("get server_ip error!");
// 		return DLL_ERROR_PARAM;
// 	}
// 	WriteLogFile("___1___server_ip:%s\r\n",item->valuestring);
// 	memcpy(server_ip, item->valuestring, strlen(item->valuestring));

// 	item = cJSON_GetObjectItem(root, "server_port");
// 	if (item == NULL)
// 	{
// 		cJSON_Delete(root);
// 		WriteLogFile("get server_port error!");
// 		return DLL_ERROR_PARAM;
// 	}
// 	WriteLogFile("___2____server_port:%s\r\n",item->valuestring);
// 	server_port = atoi(item->valuestring);

// 	item = cJSON_GetObjectItem(root, "intranet_ip");
// 	if (item != NULL)
// 	{
// 		memcpy(intranet_ip,item->valuestring,strlen(item->valuestring));
// 	}

// 	item = cJSON_GetObjectItem(root, "external_ip");
// 	if (item != NULL)
// 	{
// 		memcpy(external_ip,item->valuestring,strlen(item->valuestring));
// 	}
	
// /*						######################################						*/

// 	item = cJSON_GetObjectItem(root, "ter_id");
// 	if (item == NULL)
// 	{
// 		cJSON_Delete(root);
// 		WriteLogFile("get ter_id error!");
// 		return DLL_ERROR_PARAM;
// 	}

// 	memcpy(ter_id,item->valuestring,strlen(item->valuestring));
// 	WriteLogFile("___3____ter_id:%s\r\n",ter_id);

// 	item = cJSON_GetObjectItem(root, "notice_type");
// 	if (item == NULL)
// 	{
// 		notice_type = -999;
// 		cJSON_Delete(root);
// 		WriteLogFile("get notice_type error!  notice_type =-999 ");
// 		// return DLL_ERROR_PARAM;
// 	}
// 	else
// 	{
// 		notice_type = atoi(item->valuestring);
// 		WriteLogFile("___444____notice_type:%d\r\n",notice_type);
// 	}

// 	item = cJSON_GetObjectItem(root, "url_buff");
// 	if (item == NULL)
// 	{
// 		cJSON_Delete(root);
// 		WriteLogFile("get url_buff error!");
// 		return DLL_ERROR_PARAM;
// 	}

// 	memcpy(url_buff,item->valuestring,strlen(item->valuestring));
// 	WriteLogFile("___5____url_buff:%s\r\n",url_buff);


// /*						######################################						*/

// 	get_upgrade_net_intranet(server_ip, server_port,intranet_ip, external_ip,output,ter_id,notice_type,url_buff);
// 	// get_usbshare_net_delay_time_test(server_ip, server_port,intranet_ip, external_ip);

// }


int upgrade_net_intranet(char *json_in, void *hook, char *output)
{
	cJSON *root = NULL;
	cJSON *head;
	cJSON *item = NULL;
	char *g_buf;
	char intranet_ip[64] = {0x00},external_ip[64] = {0x00};
	result_CallBack callback = (result_CallBack)hook;

	char ter_id[64] = {0x00};
	int notice_type = -1;
	char url_buff[128] = {0x00};

	root = cJSON_Parse(json_in);
	if (root == NULL)
	{
		WriteLogFile("json in is not a json!");
		return DLL_ERROR_PARAM;
	}

		g_buf = cJSON_Print(root);
		WriteLogFile("2��������g_buf=%s\r\n",g_buf);

	WriteLogFile("dev_info ��ȡ������Ϣ׼��!");
	//char all_ter_info[1024 * 80] = { 0 };
	char server_ip[50] = { 0 };
	int server_port = -1;

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

	item = cJSON_GetObjectItem(root, "ter_data");
	if (item == NULL)
	{
		WriteLogFile("����Dataʧ��\r\n");
		cJSON_Delete(root);
		WriteLogFile("get ter_data error!");
		return DLL_ERROR_PARAM;

	}     

	root = cJSON_CreateObject();

	cJSON_AddItemToObject(root, "head", head = cJSON_CreateObject());
	cJSON_AddStringToObject(head, "protocol", "mb_usbshare_20191024");
	cJSON_AddStringToObject(head, "code_type", "request");
	cJSON_AddStringToObject(head, "cmd", "intranet_net_upgrade");
	cJSON_AddStringToObject(head, "source_topic", "123");
	cJSON_AddStringToObject(head, "random", "9876543210");
	cJSON_AddStringToObject(head, "result", "s");
	cJSON *data;
	cJSON_AddItemToObject(root, "data", data = cJSON_CreateArray());
	cJSON *temp_item;
	//cJSON_AddItemToObject(data, "dira", temp_item = cJSON_CreateObject());
	// cJSON_AddStringToObject(item, "ter_id", ter_id);


	char temp_out_data[1024] = { 0 };
	memset(temp_out_data,0,sizeof(temp_out_data));
	// char *g_buf;
	char g_buf_temp[1024];
	memset(g_buf_temp,0,sizeof(g_buf_temp));
	cJSON *rt_tmp = NULL;
	cJSON *likeObject1 = NULL;
	char temp_ter_id[20] = { 0 };
	cJSON *object_data = NULL;
	
	if (item != NULL) {
		int size = cJSON_GetArraySize(item);	// ��ȡ�������С
		WriteLogFile("-----------------------------size =%d\r\n",size);
		for (int i = 0; i < size; i++) {
			cJSON *obj = cJSON_GetArrayItem(item, i);		// ��ȡ���������obj
			cJSON *val = NULL;
			
			if (obj != NULL && obj->type == cJSON_Object) {	// �ж������ڵ�Ԫ���ǲ���obj����

				val = cJSON_GetObjectItem(obj, "server_ip");		// ���obj���ֵ
				if (val != NULL && val->type == cJSON_String) {
					WriteLogFile("___i=%d__1_server_ip:%s\r\n",i+1,val->valuestring);
					memset(server_ip,0,sizeof(server_ip));
					memcpy(server_ip, val->valuestring, strlen(val->valuestring));
				}

				val = cJSON_GetObjectItem(obj, "server_port");
				if (val != NULL && val->type == cJSON_String) {
					WriteLogFile("___i=%d__2_server_port:%s\r\n",i+1,val->valuestring);
					server_port = atoi(val->valuestring);
				}

				val = cJSON_GetObjectItem(obj, "ter_id");
				if (val != NULL && val->type == cJSON_String) {
					memset(ter_id,0,sizeof(ter_id));
					memcpy(ter_id,val->valuestring,strlen(val->valuestring));
					// memcpy(all_ter_id[i],ter_id,strlen(ter_id));
					WriteLogFile("___i=%d__3_ter_id:%s\r\n",i+1,ter_id);
					// WriteLogFile("___i=%d__3_all_ter_id:%s\r\n",i+1,all_ter_id[i]);
				}

				val = cJSON_GetObjectItem(obj, "notice_type");
				if (val != NULL && val->type == cJSON_String) {
					notice_type = atoi(val->valuestring);
					WriteLogFile("___i=%d__4_notice_type:%d\r\n",i+1,notice_type);
				}
				else
				{
					notice_type = -999;
					cJSON_Delete(root);
					WriteLogFile("get notice_type error!  notice_type =-999 ");
				}

				val = cJSON_GetObjectItem(obj, "url_buff");
				if (val != NULL && val->type == cJSON_String) {
					memset(url_buff,0,sizeof(url_buff));
					memcpy(url_buff,val->valuestring,strlen(val->valuestring));
					WriteLogFile("___i=%d__5_url_buff:%s\r\n",i+1,url_buff);
				}


			}

			get_upgrade_net_intranet(server_ip, server_port,intranet_ip, external_ip,output,ter_id,notice_type,url_buff);

			// likeObject1 = cJSON_CreateObject();


			// memset(temp_ter_id,0,sizeof(temp_ter_id));
			// sprintf(temp_ter_id,"ter_id_%d",i);


			// cJSON_AddItemToObject(likeObject1, temp_ter_id, cJSON_CreateString(ter_id));
			// cJSON_AddStringToObject(temp_item, likeObject1);

			cJSON_AddItemToArray(data, cJSON_CreateString(ter_id));
			//cJSON_AddStringToObject(temp_item, "ter_id", ter_id);
		}

		g_buf = cJSON_Print(root);
		// WriteLogFile("2��������g_buf=%s\r\n",g_buf);
		memset(temp_out_data,0,sizeof(temp_out_data));
		memcpy(temp_out_data, g_buf, strlen(g_buf));
		memcpy(output,temp_out_data,sizeof(temp_out_data));
		WriteLogFile("---------all end-----------<<output>>:%s", output);
	}
}


int restart_single_port(char *json_in, void *hook, char *output)
{
	cJSON *root = NULL;
	cJSON *item = NULL;
	SOCKET s_client;
	int result;
	char server_ip[50] = { 0x00 };
	char tmp[32] = {0x00},*recvbody = NULL;
	char recvhead[10] = { 0 };
	//unsigned char *recvbody = NULL;
	long recvlen = 0;
	int re_read = 0;
	int server_port = 0,usb_port = 0;
	char *g_buf;
	char send_json[4*1024] = { 0 };
	int len;
	result_CallBack callback = (result_CallBack)hook;
	root = cJSON_Parse(json_in);
	if (root == NULL)
	{
		WriteLogFile("json in is not a json!");
		return DLL_ERROR_PARAM;
	}
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
	
	item = cJSON_GetObjectItem(root, "oper_port");
	if (item == NULL)
	{
		cJSON_Delete(root);
		WriteLogFile("get oper_port error!");
		return DLL_ERROR_PARAM;
	}
	usb_port = atoi(item->valuestring);
	cJSON_Delete(root);

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
	cJSON *head;
	cJSON_AddItemToObject(root, "head", head = cJSON_CreateObject());
	cJSON_AddStringToObject(head, "protocol", "mb_usbshare_20191024");
	cJSON_AddStringToObject(head, "code_type", "request");
	cJSON_AddStringToObject(head, "cmd", "restart_port");
	cJSON_AddStringToObject(head, "source_topic", "123");
	cJSON_AddStringToObject(head, "random", "987654321");
	cJSON_AddStringToObject(head, "result", "s");
	cJSON *data;
	cJSON_AddItemToObject(root, "data", data = cJSON_CreateArray());
	//cJSON *item;
	cJSON_AddItemToObject(data, "dira", item = cJSON_CreateObject());
	memset(tmp,0x00,sizeof(tmp));
	sprintf(tmp, "%d", usb_port);
	cJSON_AddStringToObject(item, "usb_port", tmp);


	
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
	//char recvhead[10] = { 0 };
	//unsigned char *recvbody = NULL;
	//long recvlen = 0;
	//int re_read = 0;
re_read:
	result = recv(s_client, recvhead, 4, 0);
	if (result < 0)
	{

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
	recvlen = my_recv_data_len(s_client);
	if (recvlen < 0)
	{

		WriteLogFile("dev_info set_usbshare_ip_new recv len error!");
		shutdown(s_client, SD_BOTH);
		closesocket(s_client);
		return DLL_ERR_NEW_SHARE_SOCKET_RECV;
	}
	WriteLogFile("recvbody malloc start\n");
	recvbody = (unsigned char*)malloc(recvlen + 100);
	if (recvbody == NULL)
	{
		WriteLogFile("dev_info err malloc recvbody\n");
		shutdown(s_client, SD_BOTH);
		closesocket(s_client);
		return DLL_ERR_NEW_SHARE_SOCKET_RECV;
	}
	memset(recvbody, 0, recvlen + 100);
	WriteLogFile("start_robot_arm ����json���ݿ�ʼ\n");
	result = my_socket_read(s_client,recvbody, recvlen);
	if (result < 0)
	{

		free(recvbody);
		WriteLogFile("dev_info set_usbshare_ip_new recv body error!");
		shutdown(s_client, SD_BOTH);
		closesocket(s_client);
		return DLL_ERR_NEW_SHARE_SOCKET_RECV;
	}
	WriteLogFile("start_robot_arm:%s!", recvbody);
	//WriteLogFile("dev_info ����json�������:re_read:%d\n", re_read);
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
	//memcpy(out_ter_info, recvbody, strlen(recvbody));
	//WriteLogFile("dev_info ���յ��İ���Ϊ:%d!", strlen(out_ter_info));
	WriteLogFile("start_robot_arm:%s!", recvbody);
	memcpy(output,recvbody,strlen(recvbody));
	cJSON_Delete(rt);
	free(noformat);
	free(recvbody);
	//WriteLogFile("dev_info get_usbshare_status_new exit!!!!!\n");

	return 0;
}

int start_robot_arm(char *json_in, void *hook, char *output)
{
	cJSON *root = NULL;
	cJSON *item = NULL;
	SOCKET s_client;
	int result;
	char server_ip[50] = { 0x00 };
	char tmp[32] = {0x00},*recvbody = NULL;
	char recvhead[10] = { 0 };
	//unsigned char *recvbody = NULL;
	long recvlen = 0;
	int re_read = 0;
	int server_port = 0,usb_port = 0;
	char *g_buf;
	char send_json[4*1024] = { 0 };
	int len;
	result_CallBack callback = (result_CallBack)hook;
	root = cJSON_Parse(json_in);
	if (root == NULL)
	{
		WriteLogFile("json in is not a json!");
		return DLL_ERROR_PARAM;
	}
	
	g_buf = cJSON_Print(root);
	WriteLogFile("______________g_buf=%s\r\n",g_buf);

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
	
	item = cJSON_GetObjectItem(root, "oper_port");
	if (item == NULL)
	{
		cJSON_Delete(root);
		WriteLogFile("get oper_port error!");
		return DLL_ERROR_PARAM;
	}
	usb_port = atoi(item->valuestring);
	cJSON_Delete(root);

	if((s_client = socket_connect(server_ip,server_port,server_ip,server_port,3000)) == SOCKET_ERROR)
	{
		WriteLogFile("dev_oper start_robot_arm create or connect socket fail!");
		return DLL_ERR_NEW_SHARE_SOCKET_CONNET;
	}
	WriteLogFile("______________1111111111111111111\r\n");
	root = cJSON_CreateObject();
	if (root == NULL)
	{
		shutdown(s_client, SD_BOTH);
		closesocket(s_client);
		return -1;
	}
	cJSON *head;
	cJSON_AddItemToObject(root, "head", head = cJSON_CreateObject());
	cJSON_AddStringToObject(head, "protocol", "mb_usbshare_20191024");
	cJSON_AddStringToObject(head, "code_type", "request");
	cJSON_AddStringToObject(head, "cmd", "start_robot_arm");
	cJSON_AddStringToObject(head, "source_topic", "123");
	cJSON_AddStringToObject(head, "random", "987654321");
	cJSON_AddStringToObject(head, "result", "s");
	cJSON *data;
	cJSON_AddItemToObject(root, "data", data = cJSON_CreateArray());
	//cJSON *item;
	cJSON_AddItemToObject(data, "dira", item = cJSON_CreateObject());
	memset(tmp,0x00,sizeof(tmp));
	sprintf(tmp, "%d", usb_port);
	cJSON_AddStringToObject(item, "usb_port", tmp);


	
	g_buf = cJSON_Print(root);
	WriteLogFile("______________g_buf=%s\r\n",g_buf);
	
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
	//char recvhead[10] = { 0 };
	//unsigned char *recvbody = NULL;
	//long recvlen = 0;
	//int re_read = 0;
re_read:
	result = recv(s_client, recvhead, 4, 0);
	if (result < 0)
	{

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
	recvlen = my_recv_data_len(s_client);
	if (recvlen < 0)
	{

		WriteLogFile("dev_info set_usbshare_ip_new recv len error!");
		shutdown(s_client, SD_BOTH);
		closesocket(s_client);
		return DLL_ERR_NEW_SHARE_SOCKET_RECV;
	}
	WriteLogFile("recvbody malloc start\n");
	recvbody = (unsigned char*)malloc(recvlen + 100);
	if (recvbody == NULL)
	{
		WriteLogFile("dev_info err malloc recvbody\n");
		shutdown(s_client, SD_BOTH);
		closesocket(s_client);
		return DLL_ERR_NEW_SHARE_SOCKET_RECV;
	}
	memset(recvbody, 0, recvlen + 100);
	WriteLogFile("start_robot_arm ����json���ݿ�ʼ\n");
	result = my_socket_read(s_client,recvbody, recvlen);
	if (result < 0)
	{

		free(recvbody);
		WriteLogFile("dev_info set_usbshare_ip_new recv body error!");
		shutdown(s_client, SD_BOTH);
		closesocket(s_client);
		return DLL_ERR_NEW_SHARE_SOCKET_RECV;
	}
	WriteLogFile("start_robot_arm:%s!", recvbody);
	//WriteLogFile("dev_info ����json�������:re_read:%d\n", re_read);
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
	//memcpy(out_ter_info, recvbody, strlen(recvbody));
	//WriteLogFile("dev_info ���յ��İ���Ϊ:%d!", strlen(out_ter_info));
	WriteLogFile("start_robot_arm:%s!", recvbody);
	memcpy(output,recvbody,strlen(recvbody));
	cJSON_Delete(rt);
	free(noformat);
	free(recvbody);
	//WriteLogFile("dev_info get_usbshare_status_new exit!!!!!\n");

	return 0;
}



// int start_robot_arm(char *json_in, void *hook, char *output)
// {
// 	cJSON *root = NULL;
// 	cJSON *item = NULL;
// 	SOCKET s_client;
// 	int result;
// 	char server_ip[50] = { 0x00 },tmp_part[4] = { 0x00 };
// 	char tmp[32] = {0x00},*recvbody = NULL;
// 	char recvhead[10] = { 0 };
// 	//unsigned char *recvbody = NULL;
// 	long recvlen = 0;
// 	int re_read = 0;
// 	int server_port = 0,usb_port = 0;
// 	char *g_buf;
// 	char send_json[4*1024] = { 0 };
// 	int len;
// 	result_CallBack callback = (result_CallBack)hook;
// 	root = cJSON_Parse(json_in);
// 	if (root == NULL)
// 	{
// 		WriteLogFile("json in is not a json!");
// 		return DLL_ERROR_PARAM;
// 	}
// 	item = cJSON_GetObjectItem(root, "server_ip");
// 	if (item == NULL)
// 	{
// 		cJSON_Delete(root);
// 		WriteLogFile("get server_ip error!");
// 		return DLL_ERROR_PARAM;
// 	}
// 	memcpy(server_ip, item->valuestring, strlen(item->valuestring));

// 	item = cJSON_GetObjectItem(root, "server_port");
// 	if (item == NULL)
// 	{
// 		cJSON_Delete(root);
// 		WriteLogFile("get server_port error!");
// 		return DLL_ERROR_PARAM;
// 	}
// 	server_port = atoi(item->valuestring);
	
// 	item = cJSON_GetObjectItem(root, "oper_port");
// 	if (item == NULL)
// 	{
// 		cJSON_Delete(root);
// 		WriteLogFile("get oper_port error!");
// 		return DLL_ERROR_PARAM;
// 	}
// 	memcpy(tmp_part, item->valuestring, strlen(item->valuestring));
// 	usb_port = atoi(item->valuestring);
// 	WriteLogFile("_____-----____tmp_part = %s",tmp_part);
// 	// cJSON_Delete(root);


// 	if((s_client = socket_connect(server_ip,server_port,server_ip,server_port,3000)) == SOCKET_ERROR)
// 	{
// 		WriteLogFile("dev_oper start_robot_arm create or connect socket fail!");
// 		return DLL_ERR_NEW_SHARE_SOCKET_CONNET;
// 	}



// 	char tmp_output[4096];
// 	memset(tmp_output, 0, sizeof(tmp_output));
// 	get_usbshare_status_new(server_ip, server_port, tmp_output);

// 	WriteLogFile("tmp_output:%s	",tmp_output);


// 	//����������֤��Ҫ���ֶ�
// 	// char *g_buf;
// 	char usb_vid_tmp[8];
// 	char usb_pid_tmp[8];
// 	cJSON *rt_tmp = NULL;
// 	rt_tmp = cJSON_Parse(tmp_output);
// 	if (rt_tmp == NULL)
// 	{
// 		WriteLogFile("json in is not a json!____________");
// 		return DLL_ERROR_PARAM;
// 	}
// 	WriteLogFile("dev_info ��ȡ������Ϣ׼��_______!");
	
// 	// g_buf = cJSON_Print(rt_tmp);
// 	// WriteLogFile("g_buf=%s\r\n",g_buf);

// 	cJSON *object_data = cJSON_GetObjectItem(rt_tmp, "data");
// 	if (object_data == NULL)
// 	{
// 		WriteLogFile("����Dataʧ��\r\n");
// 		cJSON_Delete(rt_tmp);
// 		WriteLogFile("get server_ip error!");
// 		return DLL_ERROR_PARAM;

// 	}     

// 	cJSON *obj = cJSON_GetArrayItem(object_data, 0);		// ��ȡ���������obj
// 	// item = cJSON_GetObjectItem(obj, "product_code");//intranet_delay
// 	// WriteLogFile("product_code:%s\r\n",item->valuestring);  //eth_manage
// 	object_data = cJSON_GetObjectItem(obj, "usb_port_manage");
// 	if (object_data == NULL)
// 	{
// 		cJSON_Delete(rt_tmp);
// 		WriteLogFile("get eth_manage error!");
// 		return DLL_ERROR_PARAM;
// 	}

// 	g_buf = cJSON_Print(object_data);
// 	WriteLogFile("g_buf=%s\r\n",g_buf);


// 	/*************** ��ʽһ ��ȡvid �� pid***************/
// 	char *v_str = NULL;
// 	WriteLogFile("=====================================================================================111111111\r\n");
// 	item = cJSON_GetObjectItem(object_data, "usb_port_info");
// 	if (item != NULL) {
// 		int size = cJSON_GetArraySize(item);	// ��ȡ�������С
// 		WriteLogFile("=====================================================================================333333 size = %d\r\n",size);
// 		if (size == 0)
// 		{
// 					WriteLogFile(" not usb_port_info data \r\n");
// 					// cJSON_Delete(root);
// 					// cJSON_Delete(rt_tmp);
// 					// return DLL_ERROR_PARAM;
// 					sprintf(usb_vid_tmp,"%s","9999");
// 					sprintf(usb_pid_tmp,"%s","9999");
// 		}
// 		else
// 		{
// 			for (int i = 0; i < size; i++) {
// 				cJSON *obj = cJSON_GetArrayItem(item, i);		// ��ȡ���������obj
// 				cJSON *val = NULL;

// 				WriteLogFile(" Array_number:%d\n", i);
// 				WriteLogFile("=====================================================================================55555555\r\n");
// 				if (obj != NULL && obj->type == cJSON_Object) {	// �ж������ڵ�Ԫ���ǲ���obj����

// 					val = cJSON_GetObjectItem(obj, "usb_port");		// ���obj���ֵ

// 					if (val != NULL && val->type == cJSON_String) {
// 						v_str = val->valuestring;
// 						WriteLogFile("usb_port = %s\n", v_str);
// 					}

// 					if (0 == strcmp(v_str,tmp_part))
// 					{

// 						val = cJSON_GetObjectItem(obj, "usb_vid");		// ���obj���ֵ

// 						if (val != NULL && val->type == cJSON_String) {
// 							// v_str = val->valuestring;
// 							// WriteLogFile("usb_vid = %s\n", v_str);

// 							memset(usb_vid_tmp,0,sizeof(usb_vid_tmp));
// 							memcpy(usb_vid_tmp,val->valuestring,4);
// 							WriteLogFile("usb_vid_tmp:%s\r\n",usb_vid_tmp);

// 						}

// 						val = cJSON_GetObjectItem(obj, "usb_pid");
// 						if (val != NULL && val->type == cJSON_String) {
// 							// v_str = val->valuestring;
// 							// WriteLogFile("usb_pid = %s\n", v_str);

// 							memset(usb_pid_tmp,0,sizeof(usb_pid_tmp));
// 							memcpy(usb_pid_tmp,val->valuestring,4);
// 							WriteLogFile("usb_pid_tmp:%s\r\n",usb_pid_tmp);

// 						}
// 					}
// 					else
// 					{
// 						WriteLogFile(" not usb_port 1\r\n");
// 						// cJSON_Delete(root);
// 						// cJSON_Delete(rt_tmp);
// 						// return DLL_ERROR_PARAM;
// 						sprintf(usb_vid_tmp,"%s","9999");
// 						sprintf(usb_pid_tmp,"%s","9999");
// 					}
					
// 				}

// 			}


// 		}
		
// 	}
// 	else
// 	{
// 		WriteLogFile("=====================================================================================4444444\r\n");
// 		WriteLogFile(" not usb_port 2\r\n");
// 		memset(usb_vid_tmp,0,sizeof(usb_vid_tmp));
// 		memset(usb_pid_tmp,0,sizeof(usb_pid_tmp));
// 		sprintf(usb_vid_tmp,"%s","0000");
// 		sprintf(usb_pid_tmp,"%s","0000");
// 	}

// 	// item = cJSON_GetObjectItem(object_data, "usb_port_info");
// 	// if (object_data == NULL)
// 	// {
// 	// 	cJSON_Delete(rt_tmp);
// 	// 	WriteLogFile("get usb_port_info error!");
// 	// 	return DLL_ERROR_PARAM;
// 	// }

// 	// obj = cJSON_GetArrayItem(item, 0);		// ��ȡ���������obj

// 	// item = cJSON_GetObjectItem(obj, "usb_vid");
// 	// if (item == NULL)
// 	// {
// 	// 	cJSON_Delete(rt_tmp);
// 	// 	WriteLogFile("get usb_vid error!");
// 	// 	return DLL_ERROR_PARAM;
// 	// }

// 	// memset(usb_vid_tmp,0,sizeof(usb_vid_tmp));
// 	// memcpy(usb_vid_tmp,item->valuestring,4);
// 	// WriteLogFile("usb_vid_tmp:%s\r\n",usb_vid_tmp);  //eth_manage


// 	// item = cJSON_GetObjectItem(obj, "usb_pid");
// 	// if (item == NULL)
// 	// {
// 	// 	cJSON_Delete(rt_tmp);
// 	// 	WriteLogFile("get usb_pid error!");
// 	// 	return DLL_ERROR_PARAM;
// 	// }
// 	// memset(usb_pid_tmp,0,sizeof(usb_pid_tmp));
// 	// memcpy(usb_pid_tmp,item->valuestring,4);
// 	// WriteLogFile("usb_pid_tmp:%s\r\n",usb_pid_tmp);  //eth_manage


// 	cJSON_Delete(root);
// 	cJSON_Delete(rt_tmp);
// 	// return 0;

// 	root = cJSON_CreateObject();
// 	if (root == NULL)
// 	{
// 		shutdown(s_client, SD_BOTH);
// 		closesocket(s_client);
// 		return -1;
// 	}
// 	cJSON *head;
// 	cJSON_AddItemToObject(root, "head", head = cJSON_CreateObject());
// 	cJSON_AddStringToObject(head, "protocol", "mb_usbshare_20191024");
// 	cJSON_AddStringToObject(head, "code_type", "request");
// 	cJSON_AddStringToObject(head, "cmd", "start_robot_arm");
// 	cJSON_AddStringToObject(head, "source_topic", "123");
// 	cJSON_AddStringToObject(head, "random", "9876543210");
// 	cJSON_AddStringToObject(head, "result", "s");
// 	cJSON *data;
// 	cJSON_AddItemToObject(root, "data", data = cJSON_CreateArray());
// 	//cJSON *item;
// 	cJSON_AddItemToObject(data, "dira", item = cJSON_CreateObject());
// 	memset(tmp,0x00,sizeof(tmp));
// 	sprintf(tmp, "%d", usb_port);
// 	cJSON_AddStringToObject(item, "usb_port", tmp);



// 	char time_buffer[20]; 
// 	memset(time_buffer,0,sizeof(time_buffer));
// 	sprintf(time_buffer,"%s",usb_vid_tmp);
// 	WriteLogFile("____1____usb_vid =%s",time_buffer);
// 	cJSON_AddStringToObject(item, "usb_vid", time_buffer);

// 	memset(time_buffer,0,sizeof(time_buffer));
// 	sprintf(time_buffer,"%s",usb_pid_tmp);
// 	WriteLogFile("____2____usb_pid =%s",time_buffer);
// 	cJSON_AddStringToObject(item, "usb_pid", time_buffer);

	

// 	g_buf = cJSON_Print(root);
// 	WriteLogFile("__send___g_buf=%s\r\n",g_buf);

// 	memcpy(send_json, "\x01\x06\x80\x07", 4);
// 	send_json[4] = strlen(g_buf) / 0x1000000; send_json[5] = strlen(g_buf) / 0x10000;
// 	send_json[6] = strlen(g_buf) / 256; send_json[7] = strlen(g_buf) % 256;
// 	memcpy(send_json + 8, g_buf, strlen(g_buf));
// 	len = strlen(g_buf) + 8;
// 	free(g_buf);
// 	cJSON_Delete(root);

// 	result = send(s_client, send_json, len, 0);
// 	if (result < 0)
// 	{
// 		WriteLogFile("dev_info set_usbshare_ip_new send error!");
// 		shutdown(s_client, SD_BOTH);
// 		closesocket(s_client);
// 		return DLL_ERR_NEW_SHARE_SOCKET_SEND;
// 	}
// 	//char recvhead[10] = { 0 };
// 	//unsigned char *recvbody = NULL;
// 	//long recvlen = 0;
// 	//int re_read = 0;
// re_read:
// 	result = recv(s_client, recvhead, 4, 0);
// 	if (result < 0)
// 	{

// 		Sleep(1000);
// 		re_read++;
// 		if (re_read == 30)
// 		{
		
// 			shutdown(s_client, SD_BOTH);
// 			closesocket(s_client);
// 			return DLL_ERR_NEW_SHARE_SOCKET_RECV;
// 		}
// 		goto re_read;
		
	
		
// 	}
// 	recvlen = my_recv_data_len(s_client);
// 	if (recvlen < 0)
// 	{

// 		WriteLogFile("dev_info set_usbshare_ip_new recv len error!");
// 		shutdown(s_client, SD_BOTH);
// 		closesocket(s_client);
// 		return DLL_ERR_NEW_SHARE_SOCKET_RECV;
// 	}
// 	WriteLogFile("recvbody malloc start\n");
// 	recvbody = (unsigned char*)malloc(recvlen + 100);
// 	if (recvbody == NULL)
// 	{
// 		WriteLogFile("dev_info err malloc recvbody\n");
// 		shutdown(s_client, SD_BOTH);
// 		closesocket(s_client);
// 		return DLL_ERR_NEW_SHARE_SOCKET_RECV;
// 	}
// 	memset(recvbody, 0, recvlen + 100);
// 	WriteLogFile("start_robot_arm ����json���ݿ�ʼ\n");
// 	result = my_socket_read(s_client,recvbody, recvlen);
// 	if (result < 0)
// 	{

// 		free(recvbody);
// 		WriteLogFile("dev_info set_usbshare_ip_new recv body error!");
// 		shutdown(s_client, SD_BOTH);
// 		closesocket(s_client);
// 		return DLL_ERR_NEW_SHARE_SOCKET_RECV;
// 	}
// 	WriteLogFile("start_robot_arm:%s!", recvbody);
// 	//WriteLogFile("dev_info ����json�������:re_read:%d\n", re_read);
// 	shutdown(s_client, SD_BOTH);
// 	closesocket(s_client);
// 	cJSON *rt = NULL;
// 	rt = cJSON_Parse(recvbody);
// 	if (rt == NULL)
// 	{
// 		WriteLogFile("dev_info set_usbshare_ip_new cJSON_Parse error!");
// 		free(recvbody);
// 		shutdown(s_client, SD_BOTH);
// 		closesocket(s_client);
// 		return DLL_ERR_NEW_SHARE_SOCKET_RECV;
// 	}
// 	char *noformat = NULL;
// 	noformat = cJSON_PrintUnformatted(rt);
// 	if (noformat == NULL)
// 	{
// 		WriteLogFile("dev_info set_usbshare_ip_new cJSON_PrintUnformatted error!");
// 		cJSON_Delete(rt);
// 		free(recvbody);
// 		shutdown(s_client, SD_BOTH);
// 		closesocket(s_client);
// 		return DLL_ERR_NEW_SHARE_SOCKET_RECV;
// 	}
// 	//memcpy(out_ter_info, recvbody, strlen(recvbody));
// 	//WriteLogFile("dev_info ���յ��İ���Ϊ:%d!", strlen(out_ter_info));
// 	WriteLogFile("start_robot_arm:%s!", recvbody);
// 	memcpy(output,recvbody,strlen(recvbody));
// 	cJSON_Delete(rt);
// 	free(noformat);
// 	free(recvbody);
// 	//WriteLogFile("dev_info get_usbshare_status_new exit!!!!!\n");

// 	return 0;
// }



int start_robot_arm_nb(char *inbuff)
{

	char ter_id[32] = {0x00};
	char tmp[32]= {0x00};
	char *gbuf = NULL;
	int port = 0,i = 0,j = 0,ret = 0,force_close_flag = 0 ,result = 0;
	struct CabinetInfo *cabinet = NULL;
	SOCKET s_client;
	cJSON *root = NULL;
	cJSON *head = NULL;
	struct file_setting file_setting[128] = {0};

	char *g_buf;
	char send_json[4*1024] = { 0 };
	int len;

	cJSON *item = NULL;
	char server_ip[50] = { 0x00 },tmp_part[4] = { 0x00 };
	// char tmp[32] = {0x00};
	char *recvbody = NULL;
	char recvhead[10] = { 0 };
	//unsigned char *recvbody = NULL;
	long recvlen = 0;
	int re_read = 0;
	int server_port = 0,usb_port = 0;

	// WriteLogFile("enetr start_robot_arm_nb		______inbuff = %s 	\r\n",inbuff);
	
	memcpy(ter_id,inbuff,12);
	port = inbuff[12];
	port = (port<<8) | inbuff[13];
	port = (port<<8) | inbuff[14];
	port = (port<<8) | inbuff[15];
	usb_port = port;
	WriteLogFile("usb_port		______usb_port = %d 	\r\n",usb_port);
	cabinet = (struct CabinetInfo*)calloc(1, sizeof(struct CabinetInfo) * 128);
	if (NULL == cabinet)
	{
		return -1;
	}
	ret = terminal_query_cabinet_attribute(cabinet, 128);

	for(i = 0;i<128;++i)
	{
		if(0 == memcmp(cabinet[i].ca_TerminalID,ter_id,strlen(ter_id)))  //�ҵ���Ӧ�Ļ���id
		{
			WriteLogFile("enetr apply_port,�ҵ���Ӧid = %s",cabinet[i].ca_TerminalID);
			break;
		}
	}
	if(i == 128)
	{
		if(NULL != cabinet)
			free(cabinet);
		WriteLogFile("δ�ҵ���Ӧid");
		return -1;
	}


	result = read_setting(file_setting, 128);
	if(result < 0)
	{
		if(NULL != cabinet)
			free(cabinet);
		WriteLogFile("read_setting failed");
		return -1;
	}
	for(j = 0;j < result;++j)
	{
		if(0 == memcmp(file_setting[j].terminal_ip,cabinet[i].ca_SettingIP,strlen(cabinet[i].ca_SettingIP)) && file_setting[j].data_port == cabinet[i].dataport)
		{
			break;
		}
	}
	if(j == result)
	{
		WriteLogFile("dev_info can not find mattched ip");
		return -1;
	}

	WriteLogFile("dev_oper start_robot_arm_nb terminal_ip=%s,data_port=%d,remote_ip=%s,remote_data_port=%d", file_setting[j].terminal_ip,file_setting[j].data_port,file_setting[j].remote_ip,file_setting[j].remote_dataport);
	//result = socket_create_connect(&s_client, server_ip, server_port);

	if((s_client = socket_connect(file_setting[j].terminal_ip,file_setting[j].data_port,file_setting[j].remote_ip,file_setting[j].remote_dataport,3000)) == SOCKET_ERROR)
	{
		WriteLogFile("dev_info get_usbshare_status_new create or connect socket fail!");
		return DLL_ERR_NEW_SHARE_SOCKET_CONNET;
	}


	// if((s_client = socket_connect(server_ip,server_port,server_ip,server_port,3000)) == SOCKET_ERROR)
	// {
	// 	WriteLogFile("dev_oper start_robot_arm create or connect socket fail!");
	// 	return DLL_ERR_NEW_SHARE_SOCKET_CONNET;
	// }




	WriteLogFile("_____________file_setting[j].terminal_ip = %s		 file_setting[j].terminal_port = %d\r\n",file_setting[j].terminal_ip, file_setting[j].terminal_port);


	char tmp_output[4096];
	memset(tmp_output, 0, sizeof(tmp_output));
	// get_usbshare_status_new(file_setting[j].terminal_ip, file_setting[j].terminal_port, tmp_output); //3240
	get_usbshare_status_new(file_setting[j].terminal_ip, 3240, tmp_output); //3240

	WriteLogFile("tmp_output:%s	",tmp_output);


	//����������֤��Ҫ���ֶ�
	// char *g_buf;
	char usb_vid_tmp[8] = "9999";
	char usb_pid_tmp[8] = "9999";
	WriteLogFile("usb_vid_tmp:%s	usb_pid_tmp:%s",usb_vid_tmp,usb_pid_tmp);
	cJSON *rt_tmp = NULL;
	rt_tmp = cJSON_Parse(tmp_output);
	if (rt_tmp == NULL)
	{
		WriteLogFile("json in is not a json!____________");
		return DLL_ERROR_PARAM;
	}
	WriteLogFile("dev_info ��ȡ������Ϣ׼��_______!");
	
	// g_buf = cJSON_Print(rt_tmp);
	// WriteLogFile("g_buf=%s\r\n",g_buf);

	cJSON *object_data = cJSON_GetObjectItem(rt_tmp, "data");
	if (object_data == NULL)
	{
		WriteLogFile("����Dataʧ��\r\n");
		cJSON_Delete(rt_tmp);
		WriteLogFile("get server_ip error!");
		return DLL_ERROR_PARAM;

	}     

	cJSON *obj = cJSON_GetArrayItem(object_data, 0);		// ��ȡ���������obj
	// item = cJSON_GetObjectItem(obj, "product_code");//intranet_delay
	// WriteLogFile("product_code:%s\r\n",item->valuestring);  //eth_manage
	object_data = cJSON_GetObjectItem(obj, "usb_port_manage");
	if (object_data == NULL)
	{
		cJSON_Delete(rt_tmp);
		WriteLogFile("get eth_manage error!");
		return DLL_ERROR_PARAM;
	}

	g_buf = cJSON_Print(object_data);
	WriteLogFile("g_buf=%s\r\n",g_buf);


	/*************** ��ʽһ ��ȡvid �� pid***************/
	char *v_str = NULL;
	WriteLogFile("=====================================================================================111111111\r\n");
	item = cJSON_GetObjectItem(object_data, "usb_port_info");
	if (item != NULL) {
		int size = cJSON_GetArraySize(item);	// ��ȡ�������С
		WriteLogFile("=====================================================================================333333 size = %d\r\n",size);
		if (size == 0)
		{
					WriteLogFile(" not usb_port_info data \r\n");
					// cJSON_Delete(root);
					// cJSON_Delete(rt_tmp);
					// return DLL_ERROR_PARAM;
					sprintf(usb_vid_tmp,"%s","9999");
					sprintf(usb_pid_tmp,"%s","9999");
		}
		else
		{
			for (int i = 0; i < size; i++) {
				cJSON *obj = cJSON_GetArrayItem(item, i);		// ��ȡ���������obj
				cJSON *val = NULL;

				WriteLogFile(" Array_number:%d\n", i);
				WriteLogFile("=====================================================================================55555555\r\n");
				if (obj != NULL && obj->type == cJSON_Object) {	// �ж������ڵ�Ԫ���ǲ���obj����

					val = cJSON_GetObjectItem(obj, "usb_port");		// ���obj���ֵ

					if (val != NULL && val->type == cJSON_String) {
						v_str = val->valuestring;
						WriteLogFile("usb_port = %s\n", v_str);
					}

					if (0 == strcmp(v_str,tmp_part))
					{

						val = cJSON_GetObjectItem(obj, "usb_vid");		// ���obj���ֵ

						if (val != NULL && val->type == cJSON_String) {
							// v_str = val->valuestring;
							// WriteLogFile("usb_vid = %s\n", v_str);

							memset(usb_vid_tmp,0,sizeof(usb_vid_tmp));
							memcpy(usb_vid_tmp,val->valuestring,4);
							WriteLogFile("usb_vid_tmp:%s\r\n",usb_vid_tmp);

						}

						val = cJSON_GetObjectItem(obj, "usb_pid");
						if (val != NULL && val->type == cJSON_String) {
							// v_str = val->valuestring;
							// WriteLogFile("usb_pid = %s\n", v_str);

							memset(usb_pid_tmp,0,sizeof(usb_pid_tmp));
							memcpy(usb_pid_tmp,val->valuestring,4);
							WriteLogFile("usb_pid_tmp:%s\r\n",usb_pid_tmp);

						}
					}
					else
					{
						WriteLogFile(" not usb_port 1\r\n");
						// cJSON_Delete(root);
						// cJSON_Delete(rt_tmp);
						// return DLL_ERROR_PARAM;
						sprintf(usb_vid_tmp,"%s","9999");
						sprintf(usb_pid_tmp,"%s","9999");
					}
					
				}

			}


		}
		
	}
	else
	{
		WriteLogFile("=====================================================================================4444444\r\n");
		WriteLogFile(" not usb_port 2\r\n");
		memset(usb_vid_tmp,0,sizeof(usb_vid_tmp));
		memset(usb_pid_tmp,0,sizeof(usb_pid_tmp));
		sprintf(usb_vid_tmp,"%s","0000");
		sprintf(usb_pid_tmp,"%s","0000");
	}
	// item = cJSON_GetObjectItem(object_data, "usb_port_info");
	// if (object_data == NULL)
	// {
	// 	cJSON_Delete(rt_tmp);
	// 	WriteLogFile("get usb_port_info error!");
	// 	return DLL_ERROR_PARAM;
	// }

	// obj = cJSON_GetArrayItem(item, 0);		// ��ȡ���������obj

	// item = cJSON_GetObjectItem(obj, "usb_vid");
	// if (item == NULL)
	// {
	// 	cJSON_Delete(rt_tmp);
	// 	WriteLogFile("get usb_vid error!");
	// 	return DLL_ERROR_PARAM;
	// }

	// memset(usb_vid_tmp,0,sizeof(usb_vid_tmp));
	// memcpy(usb_vid_tmp,item->valuestring,4);
	// WriteLogFile("usb_vid_tmp:%s\r\n",usb_vid_tmp);  //eth_manage


	// item = cJSON_GetObjectItem(obj, "usb_pid");
	// if (item == NULL)
	// {
	// 	cJSON_Delete(rt_tmp);
	// 	WriteLogFile("get usb_pid error!");
	// 	return DLL_ERROR_PARAM;
	// }
	// memset(usb_pid_tmp,0,sizeof(usb_pid_tmp));
	// memcpy(usb_pid_tmp,item->valuestring,4);
	// WriteLogFile("usb_pid_tmp:%s\r\n",usb_pid_tmp);  //eth_manage


	cJSON_Delete(root);
	cJSON_Delete(rt_tmp);
	// return 0;

	root = cJSON_CreateObject();
	if (root == NULL)
	{
		shutdown(s_client, SD_BOTH);
		closesocket(s_client);
		return -1;
	}
	// cJSON *head;
	cJSON_AddItemToObject(root, "head", head = cJSON_CreateObject());
	cJSON_AddStringToObject(head, "protocol", "mb_usbshare_20191024");
	cJSON_AddStringToObject(head, "code_type", "request");
	cJSON_AddStringToObject(head, "cmd", "start_robot_arm");
	cJSON_AddStringToObject(head, "source_topic", "123");
	cJSON_AddStringToObject(head, "random", "9876543210");
	cJSON_AddStringToObject(head, "result", "s");
	cJSON *data;
	cJSON_AddItemToObject(root, "data", data = cJSON_CreateArray());
	//cJSON *item;
	cJSON_AddItemToObject(data, "dira", item = cJSON_CreateObject());
	memset(tmp,0x00,sizeof(tmp));
	sprintf(tmp, "%d", port);
	cJSON_AddStringToObject(item, "usb_port", tmp);



	char time_buffer[20]; 
	memset(time_buffer,0,sizeof(time_buffer));
	sprintf(time_buffer,"%s",usb_vid_tmp);
	WriteLogFile("____1____usb_vid =%s",time_buffer);
	cJSON_AddStringToObject(item, "usb_vid", time_buffer);

	memset(time_buffer,0,sizeof(time_buffer));
	sprintf(time_buffer,"%s",usb_pid_tmp);
	WriteLogFile("____2____usb_pid =%s",time_buffer);
	cJSON_AddStringToObject(item, "usb_pid", time_buffer);

	

	g_buf = cJSON_Print(root);
	WriteLogFile("__send___g_buf=%s\r\n",g_buf);

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
	//char recvhead[10] = { 0 };
	//unsigned char *recvbody = NULL;
	//long recvlen = 0;
	//int re_read = 0;
re_read:
	result = recv(s_client, recvhead, 4, 0);
	if (result < 0)
	{

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
	recvlen = my_recv_data_len(s_client);
	if (recvlen < 0)
	{

		WriteLogFile("dev_info set_usbshare_ip_new recv len error!");
		shutdown(s_client, SD_BOTH);
		closesocket(s_client);
		return DLL_ERR_NEW_SHARE_SOCKET_RECV;
	}
	WriteLogFile("recvbody malloc start\n");
	recvbody = (unsigned char*)malloc(recvlen + 100);
	if (recvbody == NULL)
	{
		WriteLogFile("dev_info err malloc recvbody\n");
		shutdown(s_client, SD_BOTH);
		closesocket(s_client);
		return DLL_ERR_NEW_SHARE_SOCKET_RECV;
	}
	memset(recvbody, 0, recvlen + 100);
	WriteLogFile("start_robot_arm ����json���ݿ�ʼ\n");
	result = my_socket_read(s_client,recvbody, recvlen);
	if (result < 0)
	{

		free(recvbody);
		WriteLogFile("dev_info set_usbshare_ip_new recv body error!");
		shutdown(s_client, SD_BOTH);
		closesocket(s_client);
		return DLL_ERR_NEW_SHARE_SOCKET_RECV;
	}
	WriteLogFile("start_robot_arm:%s!", recvbody);
	//WriteLogFile("dev_info ����json�������:re_read:%d\n", re_read);
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
	//memcpy(out_ter_info, recvbody, strlen(recvbody));
	//WriteLogFile("dev_info ���յ��İ���Ϊ:%d!", strlen(out_ter_info));
	WriteLogFile("start_robot_arm:%s!", recvbody);
	// memcpy(output,recvbody,strlen(recvbody));
	cJSON_Delete(rt);
	free(noformat);
	free(recvbody);
	//WriteLogFile("dev_info get_usbshare_status_new exit!!!!!\n");

	return 0;
}

/*********************************************************************
-   Function : send_cmd_deal
-   Description������ָ�������ش���
-   Calls��
-   Calles By:
-   Table Accessed :
-   Table Update :
-   Input  :����һ: Json_In:����ָ�����Ϣ��json����
-			������: callback:��Ӧ��Ϣ�Ļص�����
-   Output :
-   Return :0 ���� ���� ������Ӧ�Ĵ�����(ͨ���ĵ�������Ӧ�Ĵ�������)
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
			if (already_found == 1)//�ȴ���һ���������
			{
				WaitForSingleObject(main_mutex, INFINITE);
				//callback(dev_info_back);

				memset(output, 0, sizeof(output));

				memcpy(output, dev_info_back, strlen(dev_info_back));
			//	printf("call back:%s\n", dev_info_back);
				WriteLogFile("discovery dev info -len:%d:%s", strlen(dev_info_back), dev_info_back);
				ReleaseMutex(main_mutex);
				cJSON_Delete(root);
				break;
			}
			Sleep(100);
			if (count>=30*60)//����60S��һ�λ���δ������Ϸ��س�ʱ
			{
				WriteLogFile("�豸���ֳ�ʱ!\n");
				cJSON_Delete(root);
				return DLL_ERROR_TIME_OUT;
			}
			count++;
		}
		return 0;
	}
	else//�����½ӿ�usbshareָ�
	{
		WriteLogFile("dev_info ���յ���ָ��Ϊ:%s\n", item->valuestring);
		int i;
		for (i = 0; i < sizeof(tcp_order) / sizeof(tcp_order[0]); i++)
		{
			if (memcmp(item->valuestring, tcp_order[i].order, strlen(tcp_order[i].order)) == 0)
			{
				result = tcp_order[i].answer(json_in, callback, output);
				WriteLogFile("dev_oper exit!!!!!\n");
				return result;

			}
		}
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