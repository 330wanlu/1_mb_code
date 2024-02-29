#include "../inc/mb_tools.h"
#include "../../update/inc/update.h"


/*********************************************************************
-   Function : boot_malloc
-   Description��malloc���ٷ�װ,malloc�����
-   Calls��
-   Calles By:
-   Table Accessed :
-   Table Update :
-   Input  :����һ:len:��̬������ڴ��С
-   Output :
-   Return :NULL������ ����������
-   Other :
***********************************************************************/
void *boot_malloc(long len)
{
	void *pbuf;
	pbuf = malloc(len);
	if (pbuf == NULL)
	{
		return NULL;
	}
	memset(pbuf, 0, len);
	return pbuf;
}
/*********************************************************************
-   Function : str2upper
-   Description�����ַ���תΪ��д
-   Calls��
-   Calles By:
-   Table Accessed :
-   Table Update :
-   Input  :����һ:str:��ת�����ַ���

-   Output :
-   Return :NULL :���� ��������ת���ɴ�д���ַ���ָ��
-   Other :
***********************************************************************/
char* str2upper(char *str)
{
	if (str == NULL)
		return NULL;
	char *p = str;
	while (*str){
		*str = toupper(*str);
		str++;
	}
	return p;
}

/*********************************************************************
-   Function : HexToStr
-   Description����16������ת��Ϊ16���Ƶ��ַ���,��"\x01\x02\x03"->"010203"
-   Calls��
-   Calles By:
-   Table Accessed :
-   Table Update :
-   Input  :����һ:pszDest:ָ��ת������ַ���ָ��
������:pbSrc:ָ���ת����Դ16�������ַ���ָ��
������:nLen:��ת����Դ����
-   Output :
-   Return :
-   Other :
***********************************************************************/
void HexToStr(unsigned char *pszDest, unsigned char *pbSrc, int nLen)
{
	char	ddl, ddh;
	int i;
	for (i = 0; i < nLen; i++)
	{
		ddh = 48 + pbSrc[i] / 16;
		ddl = 48 + pbSrc[i] % 16;
		if (ddh > 57) ddh = ddh + 7;
		if (ddl > 57) ddl = ddl + 7;
		pszDest[i * 2] = ddh;
		pszDest[i * 2 + 1] = ddl;
	}

	pszDest[nLen * 2] = '\0';
}

/*********************************************************************
-   Function : read_file_md5
-   Description����ȡָ���ļ���MD5ֵ����ת�����ַ���
-   Calls��
-   Calles By:
-   Table Accessed :
-   Table Update :
-   Input  :����һ:filepath:��Ҫ��ȡ���ļ�����·��
������:outmd_str:�ó����ļ�MD5ֵ�ַ���
-   Output :
-   Return :0 normal -1 error
-   Other :
***********************************************************************/
int read_file_md5(char *filepath, unsigned char *outmd_str)
{
	unsigned char md5[17] = { 0 };
	FILE *fd = fopen(filepath, "r");
	MD5_CTX c;
	if (fd == NULL)
	{
		return -1;
	}
	int len;
	unsigned char *pData = (unsigned char*)malloc(1024 * 1024 * 2);
	if (!pData)
	{

		return -1;
	}
	MD5_Init(&c);
	while (0 != (len = fread(pData, 1, 1024 * 1024 * 2, fd)))
	{
		MD5_Update(&c, pData, len);
	}
	MD5_Final(md5, &c);


	HexToStr((unsigned char *)outmd_str, (unsigned char *)md5, 16);
	fclose(fd);
	free(pData);
	return 0;
}

int mb_system_read(char *cmd, char *r_buf, int r_buf_len,char *tmp_file)
{
	char order[200];
	int fd;
	int result;

	memset(order, 0, sizeof(order));
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
	mb_delete_file_dir_n(tmp_file, 0);
	sync();
	return 0;
}

int mb_system_read_sleep(char *cmd, char *r_buf, int r_buf_len,int time)
{
	char order[200];
	char tmp_file[100];
	int fd;
	int result;

	memset(order, 0, sizeof(order));
	memset(tmp_file, 0, sizeof(tmp_file));
	sprintf(tmp_file, "/tmp/system_cmd_sleep.log");
	sprintf(order, "%s > %s 2>&1", cmd, tmp_file);
	printf("order:%s\n", order);
	system(order);

	sleep(time);

	fd = open(tmp_file, O_RDONLY);
	if (fd<0)
	{
		return -1;
	}
	result = read(fd, r_buf, r_buf_len);
	if (result<0)
	{
		printf("read file error");
	}
	close(fd);
	//mb_delete_file_dir_n(tmp_file, 0);
	sync();
	return 0;
}

int mb_system_read_tmp(char *cmd, char *r_buf, int r_buf_len,char *tmp_path)
{
	char order[200];
	char tmp_file[100];
	int fd;
	int result;

	memset(order, 0, sizeof(order));
	memset(tmp_file, 0, sizeof(tmp_file));
	sprintf(order, "%s  > %s 2>&1", cmd, tmp_path);
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
	mb_delete_file_dir_n(tmp_file, 0);
	sync();
	return 0;
}

int mb_delete_file_dir_n(char *file_path, int file_type)
{
	char cmd[1000] = { 0 };
	if (file_type == 0)
	{
		//�ļ�ɾ��
		sprintf(cmd, "rm %s", file_path);
	}
	else
	{
		//�ļ���ɾ��
		sprintf(cmd, "rm -rf %s/", file_path);
	}
	system(cmd);
	sync();
	return 0;
}


int get_ter_id_ver(char *filename, char *ter_id,char *ver,char *hostname,int *port)
{
	FILE *file;
	char lineBuff[256] = { 0 };
	int find_flag = 0;
	char port_str[10] = { 0 };
	char *linePos = NULL;
	if (filename == NULL)
		return -1;
	file = fopen(filename, "rb");
	if (!file)
	{
		return -201;
	}
	fseek(file, 0, SEEK_SET);

	memset(lineBuff, 0, sizeof(lineBuff));
	//�����ѭ�������ж�config�ļ��ĸ�ʽ�Ƿ���ȷ
	while (fgets(lineBuff, 256, file) != NULL)
	{
		if ((lineBuff[0] == '#') || (lineBuff[0] == ';') || memcmp(lineBuff, "\r\n", 2) == 0 || lineBuff[0] == '\n')
			continue;
		linePos = strstr(lineBuff, "terid=");
		if (linePos != NULL)
		{
			find_flag += 1;
			if (linePos[strlen(linePos) - 1] == '\n'&&linePos[strlen(linePos) - 2] == '\r')
				linePos[strlen(linePos) - 2] = '\0';
			else if (linePos[strlen(linePos) - 1] == '\n')
				linePos[strlen(linePos) - 1] = '\0';
			memcpy(ter_id, linePos + 6, strlen(linePos) - 6);

			memset(lineBuff, 0, sizeof(lineBuff));
			linePos = NULL;
			continue;
		}
		linePos = strstr(lineBuff, "server=");
		if (linePos != NULL)
		{
			if (linePos[strlen(linePos) - 1] == '\n'&&linePos[strlen(linePos) - 2] == '\r')
				linePos[strlen(linePos) - 2] = '\0';
			else if (linePos[strlen(linePos) - 1] == '\n')
				linePos[strlen(linePos) - 1] = '\0';
			memcpy(hostname, linePos + 7, strlen(linePos) - 7);
			//printf("host !:%s\n", hostname);
			find_flag += 1;
			memset(lineBuff, 0, sizeof(lineBuff));
			linePos = NULL;
			continue;
		}
		linePos = strstr(lineBuff, "ver=");
		if (linePos != NULL)
		{
			if (linePos[strlen(linePos) - 1] == '\n'&&linePos[strlen(linePos) - 2] == '\r')
				linePos[strlen(linePos) - 2] = '\0';
			else if (linePos[strlen(linePos) - 1] == '\n')
				linePos[strlen(linePos) - 1] = '\0';
			memcpy(ver, linePos + 4, strlen(linePos) - 4);
			find_flag += 1;
			memset(lineBuff, 0, sizeof(lineBuff));
			linePos = NULL;
			continue;
		}

		linePos = strstr(lineBuff, "port=");
		if (linePos != NULL)
		{
			if (linePos[strlen(linePos) - 1] == '\n'&&linePos[strlen(linePos) - 2] == '\r')
				linePos[strlen(linePos) - 2] = '\0';
			else if (linePos[strlen(linePos) - 1] == '\n')
				linePos[strlen(linePos) - 1] = '\0';
			memcpy(port_str, linePos + 5, strlen(linePos) - 5);
			*port = atoi(port_str);
			find_flag += 1;
			memset(lineBuff, 0, sizeof(lineBuff));
			linePos = NULL;
			continue;
		}
		
		
		
		//break;
	}
	fclose(file);
	if (find_flag == 4)
		return 0;
	if (find_flag == 3)
	{
		*port = 10066;
		return 0;
	}
		

	return -1;
}

int get_local_tar_mad5(char *filepath,char *outmd5)
{
	int fd;
	struct stat statbufs;
	MD5_CTX ctx;
	unsigned char outmd[16] = { 0 };
	unsigned char outmd_up[50] = { 0 };
	char *p = NULL;
	long sizes;
	char *readbuf = NULL;
	if (access(filepath, 0) != 0)
		return -1;
	stat(filepath, &statbufs);
	sizes = statbufs.st_size;
	printf("size:%ld\n", sizes);
	readbuf = boot_malloc(sizes + 10);
	if (readbuf == NULL)
		return -1;
	memset(readbuf, 0, sizes + 10);
	fd = open(filepath, O_RDONLY);
	if (fd < 0)
	{
		free(readbuf);
		return -1;
	}
	read(fd, readbuf, sizes);

	MD5_Init(&ctx);
	MD5_Update(&ctx, readbuf, sizes);
	MD5_Final(outmd, &ctx);
	//MD5�������
	//MD5���ܽ������ת16�����ַ�����ת��д
	HexToStr((char *)outmd_up, (char *)outmd, 16);
	p = str2upper((char *)outmd_up);
	net_update_out("^^^^^^^^^^^^^^^^^^^				mad5			^^^^^^^^^^^^^^^^^^^^\n");
	memcpy(outmd5, p, strlen(p));
	close(fd);
	free(readbuf);
	return 0;

}

int parse_config_back_oldver(char *configname, char *now_path)
{

	FILE *file;
	//int result = 0;
	char lineBuff[256] = { 0 };// �洢��ȡһ�е�����
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
	if (access((char *)now_path, 0) != 0)
	{
		return BOOT_ERROR_OPER_PARSE_CONFIG_ERR_PATH_NOT_FOUND;
	}
	file = fopen((char *)configname, "rb");
	if (!file)
	{
		return -201;
	}
	fseek(file, 0, SEEK_END);
	ftell(file); //�����ļ�����  
	fseek(file, 0, SEEK_SET);
	memset(lineBuff, 0, sizeof(lineBuff));
	//�����ѭ�������ж�config�ļ��ĸ�ʽ�Ƿ���ȷ
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
		memcpy(file_real_path, linePos + 3, strlen(linePos) - 3);//��ȥһ��" = "


		if (file_real_path[strlen(file_real_path) - 1] == '/')//ͳһ��ʽ(��·���н�β��'/'����ȥ��������ͳһ����)
		{
			file_real_path[strlen(file_real_path) - 1] = '\0';
		}

		memset(tar_file_path, 0, sizeof(tar_file_path));
		sprintf(tar_file_path, "%s/%s", now_path, file_name);
	//	printf("back path = %s\n", file_real_path);
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
		printf("backfile_path=%s\r\n",backfile_path);
		if (access(backfile_path, 0) == 0)//����back���ݵĻ���
		{
			memset(cmd, 0, sizeof(cmd));
			sprintf(cmd, "cp %s %s", backfile_path, real_file_path);
			printf("cmd1=%s\r\n",cmd);
			system(cmd);
			sync();

			memset(cmd, 0, sizeof(cmd));
			sprintf(cmd, "rm -rf %s", backfile_path);
			printf("cmd=%s\r\n",cmd);
			system(cmd);
			sync();
		}
	
		memset(lineBuff, 0, sizeof(lineBuff));
		continue;
	}
	fclose(file);
	return 0;
	


}

int sleep_hour(int hour)
{
	int min = hour * 60;
	int i;
	for (i = 0; i < min; i++)
		sleep(60);
	return 0;
}

long long calc_path_size(char *path)
{
	struct statfs diskInfo;

	statfs(path, &diskInfo);
	unsigned long long blocksize = diskInfo.f_bsize;	//ÿ��block��������ֽ���
	//unsigned long long totalsize = blocksize * diskInfo.f_blocks; 	//�ܵ��ֽ�����f_blocksΪblock����Ŀ
	//printf("Total_size = %llu B = %llu KB = %llu MB = %llu GB\n",
		//totalsize, totalsize >> 10, totalsize >> 20, totalsize >> 30);

	unsigned long long freeDisk = diskInfo.f_bfree * blocksize;	//ʣ��ռ�Ĵ�С
	//unsigned long long availableDisk = diskInfo.f_bavail * blocksize; 	//���ÿռ��С
	return freeDisk >> 20;
}
