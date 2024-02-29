#include "_mb_tools.h"



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