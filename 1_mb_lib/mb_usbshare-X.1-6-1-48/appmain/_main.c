﻿#define _main_c
#include "_main.h"
#include <stdio.h>  
#include <stdlib.h>  
#include <string.h>  
#include <ctype.h>  
#include <sys/un.h>  
#include <sys/ioctl.h>  
#include <sys/socket.h>  
#include <linux/types.h>  
#include <linux/netlink.h>  
#include <errno.h>  
#include <unistd.h>  
#include <arpa/inet.h>  
#include <netinet/in.h>  

#define UEVENT_BUFFER_SIZE 2048  
int main(int argc, char *argv[])
{
	int c;
	int index = 0;
	for (;;)
	{
		c = getopt_long(argc, argv, "VvHhDdIi", longopts, &index);
		if (c == 'v')
		{
			return get_dev_ver();
		}
		else if (c == 'V')
		{
			return get_dev_ver();
		}
		else if (c == 'h')
		{
			return get_help();
		}
		else if (c == 'H')
		{
			return get_help();
		}
		else if (c == 'D')
		{
			return oper_app();
		}
		else if (c == 'd')
		{
			return oper_debug();
		}
		else if (c == 'I')
		{
			return test_invoice();
		}
		else if (c == 'i')
		{
			printf("test ssl rsa\n");
			return test_rsa();
		}
		break;
	}
	return 0;
}


static int test_rsa(void)
{


	return 0;
}
#ifdef NEW_BOOT
int get_ter_id_ver(char *filename, char *ter_id, char *ver)
{
	FILE *file;
	char lineBuff[256] = { 0 };
	int find_flag = 0;
	char *linePos = NULL;
	if (filename == NULL)
		return -1;
	file = fopen(filename, "rb");
	if (!file)
	{
		return -1;
	}
	fseek(file, 0, SEEK_SET);

	memset(lineBuff, 0, sizeof(lineBuff));
	//下面的循环是先判断config文件的格式是否正确
	while (fgets(lineBuff, 256, file) != NULL)
	{
		if ((lineBuff[0] == '#') || (lineBuff[0] == ';') || memcmp(lineBuff, "\r\n", 2) == 0 || lineBuff[0] == '\n')
			continue;
		linePos = strstr(lineBuff, "terid=");
		if (linePos == NULL)
		{
			linePos = strstr(lineBuff, "ver=");
			if (linePos != NULL)
			{
				if (linePos[strlen(linePos) - 1] == '\n'&&linePos[strlen(linePos) - 2] == '\r')
					linePos[strlen(linePos) - 2] = '\0';
				else if (linePos[strlen(linePos) - 1] == '\n')
					linePos[strlen(linePos) - 1] = '\0';
				memcpy(ver, linePos + 4, strlen(linePos) - 4);
				find_flag += 1;
				continue;
			}
			memset(lineBuff, 0, sizeof(lineBuff));
			continue;
		}
		find_flag += 1;
		if (linePos[strlen(linePos) - 1] == '\n'&&linePos[strlen(linePos) - 2] == '\r')
			linePos[strlen(linePos) - 2] = '\0';
		else if (linePos[strlen(linePos) - 1] == '\n')
			linePos[strlen(linePos) - 1] = '\0';
		memcpy(ter_id, linePos + 6, strlen(linePos) - 6);
		break;
	}
	if (find_flag == 2)
		return 0;
	return -1;
}
#endif


static int get_dev_ver(void)
{
	_so_note    *note;
	int result;
	char ter_id[13] = { 0 };
	//char ver[20] = { 0 };
	note = get_application_version();
	if (note == NULL)
	{
		printf("get app err\n");
		return -1;
	}
	printf("name=%s\n", note->name);
	printf("r_date=%s\n", note->r_date);
	printf("ver=%s\n", note->ver);
	printf("writer=%s\n", note->writer);
	printf("note=%s\n", note->note);
	printf("code=%s\n", note->code);
	printf("DETECT SUCCESS!\n");
	return 0;
#ifdef NEW_BOOT
	printf("get machine id :%s!\n", ter_id);
	//进行是否匹配机型程序检测
	//if ((ter_id[3] == 0x30) && (ter_id[4] == 0x31 || ter_id[4] == 0x32 || ter_id[4] == 0x33))
	//{
	//	sprintf(mach_type, "MB_16");
	//	memcpy(mach_type + 5, ter_id + 3, 2);
	//	if (ter_id[5] == 0x31 || ter_id[5] == 0x30)
	//	{
	//		mach_type[7] = 'A';
	//	}
	//}
	//else if ((ter_id[3] == 0x30) && (ter_id[4] == 0x34))
	//{
	//	sprintf(mach_type, "MB_1702");
	//	//memcpy(mach_type+5,ter_id+3,2);
	//}
	//else if ((ter_id[3] == 0x30) && (ter_id[4] == 0x35))
	//{
	//	sprintf(mach_type, "MB_1703");
	//	//memcpy(mach_type+5,ter_id+3,2);
	//}
	//else if ((ter_id[3] == 0x30) && (ter_id[4] == 0x37))
	//{
	//	sprintf(mach_type, "MB_1805");
	//}
	//else if ((ter_id[3] == 0x30) && (ter_id[4] == 0x38))
	//{
	//	sprintf(mach_type, "MB_1806");
	//}
	//else if ((ter_id[3] == 0x31) && (ter_id[4] == 0x31))
	//{
	//	sprintf(mach_type, "MB_1902");
	//}
	//else if ((ter_id[3] == 0x31) && (ter_id[4] == 0x32))
	//{
	//	sprintf(mach_type, "MB_1906");
	//}
	//else if ((ter_id[3] == 0x31) && (ter_id[4] == 0x33))
	//{
	//	sprintf(mach_type, "MB_1905");
	//}
	//else if (ter_id[5] != 0x30)
	//{
	//	memset(mach_type + 7, ter_id[5] + 0x10, 1);
	//}
	//printf("type 1:%s\n type2:%s\n", mach_type, note->type);
	//if (memcmp(mach_type, note->type, strlen(note->type)) != 0)//不是匹配机型的程序
	//{
	//	printf("err type not match!\n");
	//	return -2;
	//}
	//进行Mysql环境检测
	result = access("/mnt/masstore/mysql", 0);
	if (result < 0)
	{
		printf("err detect /mnt/masstore/mysql!\n");
		return -3;
	}
	result = access("/etc/init.d/mysqld2.0", 0);
	if (result < 0)
	{
		printf("err detect /etc/init.d/mysqld2.0!\n");
		return -3;
	}
	result = access("/usr/sbin/ntpdate", 0);
	if (result < 0)
	{
		printf("err detect /usr/sbin/ntpdate!\n");
		return -3;
	}
	result = access("/etc/autotime.sh", 0);
	if (result < 0)
	{
		printf("err detect /etc/autotime.sh!\n");
		return -3;
	}
	result = access("/home/share/exe/mb_ca_reader", 0);
	if (result < 0)
	{
		printf("err detect /home/share/exe/mb_ca_reader!\n");
		return -3;
	}
	result = access("/home/share/exe/ca_server", 0);
	if (result < 0)
	{
		printf("ERR detect /home/share/exe/ca_server!\n");
		return -3;
	}


#endif
	printf("DETECT SUCCESS!\n");
	return 0;
}

static int oper_app(void)
{
	load();
	app_main();
	return -1;
}



static int oper_debug(void)
{
	return 0;
}


static int get_help(void)
{
	return 0;
}

static int test_invoice(void)
{
	return 0;
}

