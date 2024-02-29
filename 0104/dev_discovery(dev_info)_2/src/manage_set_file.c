#define DF_MANAGE_SET_FILE
#define LOGLEVEL

#include "terminal_manage.h"
#include "manage_set_file.h"
#include "mylog.h"
//#include "system.h"
//#include "log.h"

#pragma warning(disable:4996)

//#define WriteLogFile(format,...)		log_WriteLogFile(format , ##__VA_ARGS__ )

char * inet_ntop2(int family, const void *addrptr, char *strptr, size_t len)
{
	const u_char *p = (const u_char *)addrptr;
	char temp[16];
	if (family == AF_INET) {

		sprintf(temp, "%d.%d.%d.%d", p[0], p[1], p[2], p[3]);
		if (strlen(temp) >= len){

			return (NULL);
		}
		strcpy(strptr, temp);
		return (strptr);
	}

	return (NULL);
}

/*******************************老版本************************************/
int old_read_setting(struct file_setting	file[], int size_file)
{
	int i;
	WaitForSingleObject(all_terminal_setting_table.hmutex, INFINITE);
	for (i = 0; i < all_terminal_setting_table.count && i < size_file; i++)
		memcpy(&file[i], &all_terminal_setting_table.terminal_setting[i], sizeof(file[0]));
	ReleaseMutex(all_terminal_setting_table.hmutex);
	return i;
}

int old_write_setting(struct file_setting file[], int size_file)
{
	int count = size_file, i = 0 , j = 0;
	char ip_prefix[30], cmd_port_prefix[20], data_port_prefix[20], data_port_asc[10], cmd_port_asc[10], count_asc[10] = {0};
	char buf_prefix[100] = {0};
	char set_path[120] = {0};

	if (!GetModuleFileName(NULL, set_path, sizeof(set_path))){
		WriteLogFile("获取当前路径错误");
		return FALSE;
	}
	*(strrchr(set_path, '\\') + 1) = 0;
	memcpy(set_path + strlen(set_path), "Client.set", 13);
	_itoa(count, count_asc, 10);
	WritePrivateProfileString(TEXT("Server"), TEXT("Count"), TEXT(count_asc), TEXT(set_path));

	//修改setting配置文件和内存配置表
	while (i < 128){//清除配置表
		sprintf(ip_prefix, "%s%d", "IP", i + 1);
		sprintf(cmd_port_prefix, "%s%d", "Port", i + 1);
		WritePrivateProfileString(TEXT("Server"), TEXT(ip_prefix), NULL, TEXT(set_path));
		WritePrivateProfileString(TEXT("Server"), TEXT(cmd_port_prefix), NULL, TEXT(set_path));
		memset(all_terminal_setting_table.terminal_setting + i, 0, sizeof(struct file_setting));
		i++;
	}
	
	for (i = 0; i < count; i++){
		memset(ip_prefix, 0, sizeof(ip_prefix));
		memset(cmd_port_prefix, 0, sizeof(cmd_port_prefix));
		memset(data_port_prefix, 0, sizeof(data_port_prefix));
		memset(data_port_asc, 0, sizeof(data_port_asc));
		memset(cmd_port_asc, 0, sizeof(cmd_port_asc));

		sprintf(ip_prefix, "%s%d", "IP", i + 1);
		sprintf(cmd_port_prefix, "%s%d", "Port", i + 1);
		_itoa(file[i].terminal_port, cmd_port_asc, 10);
		_itoa(file[i].data_port, data_port_asc, 10);

		WritePrivateProfileString(TEXT("Server"), TEXT(ip_prefix), TEXT(file[i].terminal_ip), TEXT(set_path));
		WritePrivateProfileString(TEXT("Server"), TEXT(cmd_port_prefix), TEXT(cmd_port_asc), TEXT(set_path));
		memset(all_terminal_setting_table.terminal_setting[i].terminal_ip, 0, sizeof(all_terminal_setting_table.terminal_setting[i].terminal_ip));

		memcpy(all_terminal_setting_table.terminal_setting[i].terminal_ip, file[i].terminal_ip, sizeof(file[i].terminal_ip));
		all_terminal_setting_table.terminal_setting[i].terminal_port = file[i].terminal_port;
		all_terminal_setting_table.terminal_setting[i].data_port = file[i].data_port;
	}
	all_terminal_setting_table.count = count;

	//更新终端信息内存表
	old_updata_terminal_setting(file, size_file);
	return i;
}


/*******************************新版本**************************/
int get_client_id(char* client_id)
{
	int ret = 0;
	memcpy(client_id, all_terminal_setting_table.guid, 38);
	return 0;
}


/*初始化配置*/
 BOOL terminal_setting_init(void)
{
	char set_path[MAX_PATH] = { 0 }, *c_DestPath = NULL, ip_asc[100] = { 0 }, buf_prefix[30];
	struct hostent *h;

	if (!GetModuleFileName(NULL, set_path, sizeof(set_path))){
		WriteLogFile("获取当前路径错误");
		return FALSE;
	}
	*(strrchr(set_path,'\\')+1) = 0;
	WriteLogFile("set_path : %s",set_path);
	c_DestPath = strcat(set_path, "ModelDll\\ModelApiDll\\Client.set");
	WriteLogFile("c_DestPath : %s",c_DestPath);
	if (c_DestPath == NULL){
		WriteLogFile("未找到 Client.set = %s",c_DestPath);
		return FALSE;
	}
	/*判断文件是否存在 如果不存在创建配置文件并初始化, 否则读取配置文件*/
	if (access(c_DestPath , 0)){
		WriteLogFile("%s不存在",c_DestPath);
		FILE *fp = fopen(c_DestPath , "wb+");
		if (fp == NULL){
			WriteLogFile("Client.set 打开失败");
			return FALSE;
		}
		else{
			fwrite("[Log] \nLevel=0\n", 1, strlen("[Log] \nLevel=0\n"), fp);
			fwrite("[Server] \nCount=0" , 1 , strlen("[Server] \n\t Count=0")-1 , fp);
			fflush(fp);
			fclose(fp);
		}
	}
	else{
		WriteLogFile("Client.set 存在");
		memset(buf_prefix, 0, sizeof(buf_prefix));
		
		/*读取输出日志等级*/
		i_LogLevel = GetPrivateProfileInt(TEXT("Log"), TEXT("Level"), 0, TEXT(c_DestPath));

		/*读取配置文件*/
		all_terminal_setting_table.hmutex = CreateMutex(NULL, FALSE, NULL);
		if (all_terminal_setting_table.hmutex == NULL){
			WriteLogFile("all_terminal_setting_table.hmutex 互斥量创建失败");
			return -1;
		}
		all_terminal_setting_table.count = GetPrivateProfileInt(TEXT("Server"), TEXT("Count"), 0, TEXT(c_DestPath));

		for (int i = 1; i <= all_terminal_setting_table.count; i++){
			memset(ip_asc, 0, sizeof(ip_asc));
			memset(buf_prefix, 0, sizeof(buf_prefix));
			sprintf(buf_prefix, "%s%d", "IP", i);
			GetPrivateProfileString(TEXT("Server"), TEXT(buf_prefix), TEXT(""), ip_asc, 20, TEXT(c_DestPath));//设备IP地址
			h = gethostbyname(ip_asc);
			if (h == NULL){
				WriteLogFile("域名解析错误\n");
				continue;
			}
			else{
				inet_ntop2(h->h_addrtype, *(h->h_addr_list), ip_asc, sizeof(ip_asc));
				sprintf(all_terminal_setting_table.terminal_setting[i - 1].terminal_ip, "%s", ip_asc);
			}
			memset(buf_prefix, 0, sizeof(buf_prefix));
			sprintf(buf_prefix, "%s%d", "Port", i);
			all_terminal_setting_table.terminal_setting[i - 1].terminal_port = GetPrivateProfileInt(TEXT("SERVER"), TEXT(buf_prefix), 0, TEXT(c_DestPath));

			memset(buf_prefix, 0, sizeof(buf_prefix));
			sprintf(buf_prefix, "%s%d", "DataPort", i);
			all_terminal_setting_table.terminal_setting[i - 1].data_port = GetPrivateProfileInt(TEXT("SERVER"), TEXT(buf_prefix), 3240, TEXT(c_DestPath));

#ifdef NINGBO_BANK
			memset(ip_asc, 0, sizeof(ip_asc));
			memset(buf_prefix, 0, sizeof(buf_prefix));
			sprintf(buf_prefix, "%s%d", "RemoteIP", i);
			GetPrivateProfileString(TEXT("Server"), TEXT(buf_prefix), TEXT(""), ip_asc, 20, TEXT(c_DestPath));//设备ngnix 转发IP地址
			h = gethostbyname(ip_asc);
			if (h == NULL){
				WriteLogFile("域名解析错误\n");
				continue;
			}
			else{
				inet_ntop2(h->h_addrtype, *(h->h_addr_list), ip_asc, sizeof(ip_asc));
				sprintf(all_terminal_setting_table.terminal_setting[i - 1].remote_ip, "%s", ip_asc);
			}
			memset(buf_prefix, 0, sizeof(buf_prefix));
			sprintf(buf_prefix, "%s%d", "RemotePort", i);
			all_terminal_setting_table.terminal_setting[i - 1].remote_port = GetPrivateProfileInt(TEXT("SERVER"), TEXT(buf_prefix), 0, TEXT(c_DestPath));

			memset(buf_prefix, 0, sizeof(buf_prefix));
			sprintf(buf_prefix, "%s%d", "RemoteDataPort", i);
			all_terminal_setting_table.terminal_setting[i - 1].remote_dataport = GetPrivateProfileInt(TEXT("SERVER"), TEXT(buf_prefix), 3240, TEXT(c_DestPath));
#endif
			WriteLogFile("read ip :%s --port:%d--dataport:%d --remoteip:%s--remote_port:%d--remote_dataport:%d\n", all_terminal_setting_table.terminal_setting[i - 1].terminal_ip, \
				all_terminal_setting_table.terminal_setting[i - 1].terminal_port, all_terminal_setting_table.terminal_setting[i - 1].data_port,\
				all_terminal_setting_table.terminal_setting[i - 1].remote_ip, all_terminal_setting_table.terminal_setting[i - 1].remote_port, all_terminal_setting_table.terminal_setting[i - 1].remote_dataport);
		}
	}
	return TRUE;
}
 /*从配置表中读取 ip port data_port 返回配置表里的数量*/
 int read_setting(struct file_setting	file[],int size_file)
 {
	 int i;
	 WaitForSingleObject(all_terminal_setting_table.hmutex, INFINITE);
	 for (i = 0; i < 128 ; i++)
		 memcpy(&file[i],&all_terminal_setting_table.terminal_setting[i],sizeof(file[0]));
	 ReleaseMutex(all_terminal_setting_table.hmutex);
	 return all_terminal_setting_table.count;
}

 int read_guid(char *buf , int size)
 {
	 memcpy(buf, all_terminal_setting_table.guid , 38);
	 return 0;
 }

 int write_setting(struct file_setting file[], int size_file)
 {
	 int count = size_file, i = 0;
	 char ip_prefix[30] , cmd_port_prefix[20],data_port_prefix[20] , data_port_asc[20] , cmd_port_asc[20],count_asc[10];
	 char buf_prefix[100];
	 char set_path[120];

	 memset(set_path, 0, sizeof(set_path));
	 if (!GetModuleFileName(NULL, set_path, sizeof(set_path))){
		 WriteLogFile("获取当前路径错误");
		 return FALSE;
	 }
	 WriteLogFile("获取到当前路径为:%s\n", set_path);
	 *(strrchr(set_path, '\\') + 1) = 0;
	 memcpy(set_path + strlen(set_path), "Client.set", 13);
	 memset(buf_prefix, 0, sizeof(buf_prefix));
	 memset(count_asc , 0 ,sizeof(count_asc));
	 _itoa(count , count_asc , 10);
	 WritePrivateProfileString(TEXT("Server"), TEXT("Count"), TEXT(count_asc), TEXT(set_path));
	 while (i < 128){//清除配置表
		 sprintf(ip_prefix, "%s%d", "IP", i + 1);
		 sprintf(cmd_port_prefix, "%s%d", "Port", i + 1);
		 sprintf(data_port_prefix, "%s%d", "DataPort", i + 1);
		 WritePrivateProfileString(TEXT("Server"), TEXT(ip_prefix), NULL, TEXT(set_path));
		 WritePrivateProfileString(TEXT("Server"), TEXT(cmd_port_prefix), NULL, TEXT(set_path));
		 WritePrivateProfileString(TEXT("Server"), TEXT(data_port_prefix), NULL, TEXT(set_path));
		 i++;
	 }

	 for (i = 0; i < count; i++){

		 memset(ip_prefix , 0 , sizeof(ip_prefix));
		 memset(cmd_port_prefix, 0, sizeof(cmd_port_prefix));
		 memset(data_port_prefix, 0, sizeof(data_port_prefix));
		 memset(data_port_asc, 0, sizeof(data_port_asc));
		 memset(cmd_port_asc, 0, sizeof(cmd_port_asc));

		 sprintf(ip_prefix, "%s%d", "IP", i + 1);
		 sprintf(cmd_port_prefix, "%s%d", "Port", i + 1);
		 sprintf(data_port_prefix, "%s%d", "DataPort", i + 1);
		 _itoa(file[i].terminal_port, cmd_port_asc , 10);
		 _itoa(file[i].data_port, data_port_asc, 10);

		 WritePrivateProfileString(TEXT("Server"), TEXT(ip_prefix), TEXT(file[i].terminal_ip), TEXT(set_path));
		 WritePrivateProfileString(TEXT("Server"), TEXT(cmd_port_prefix), TEXT(cmd_port_asc), TEXT(set_path));
		 WritePrivateProfileString(TEXT("Server"), TEXT(data_port_prefix), TEXT(data_port_asc), TEXT(set_path));
	 }

	 for (i = 0; i < 128; i++){
		 memset(&all_terminal_setting_table.terminal_setting[i], 0, sizeof(struct file_setting));
	 }
	 all_terminal_setting_table.count = count;
	 for (i = 0; i < count; i++){
		 /*
		 for (int j = 0; j < count; j++){
			 if ((strcmp(all_terminal_setting_table.terminal_setting[j].terminal_ip, file[i].terminal_ip) == 0) && (all_terminal_setting_table.terminal_setting[j].data_port == file[i].data_port) && (all_terminal_setting_table.terminal_setting[j].terminal_port == file[i].terminal_port))
				 break;
		 }*/
		 memcpy(&all_terminal_setting_table.terminal_setting[i], &file[i], sizeof(struct file_setting));
	 }
	 terminal_updata_terminal_setting(file ,size_file);
	 return i;
 }

