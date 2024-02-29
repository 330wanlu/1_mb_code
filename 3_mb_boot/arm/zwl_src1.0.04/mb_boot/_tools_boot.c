#include "_tools_boot.h"

extern unsigned char update_msg[1024];
extern unsigned char auth_msg[1024];
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
	//MD5加密完成
	//MD5加密结果进行转16进制字符串再转大写
	HexToStr((char *)outmd_up, (char *)outmd, 16);
	p = str2upper((char *)outmd_up);

	memcpy(outmd5, p, strlen(p));
	close(fd);
	free(readbuf);
	return 0;

}
/*********************************************************************
-   Function : boot_malloc
-   Description：malloc的再封装,malloc后清空
-   Calls：
-   Calles By:
-   Table Accessed :
-   Table Update :
-   Input  :参数一:len:动态分配的内存大小
-   Output :
-   Return :NULL：错误 其他：正常
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
int get_line(int sock, char *buf, int size)
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
				//out("\n但如果是换行符则把它吸收掉\n");
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


/*********************************************************************
-   Function : Write_Decompress_file
-   Description：写入信息到指定文件并解压到指定目录下
-   Calls：
-   Calles By:
-   Table Accessed :
-   Table Update :
-   Input  :参数一:file:待写入的数据
参数二:file_len:待写入的数据长度
参数三:filename:指定的文件名
参数四:Dest_dir:指定的路径
-   Output :
-   Return :0：正常 <0：错误
-   Other :
***********************************************************************/
int Write_Decompress_file(unsigned char *file, long file_len, unsigned char *filename, unsigned char * Dest_dir)
{
	int file_fd, result;
	char config_path[200] = { 0 };
	mb_delete_file_dir((char *)filename,0);
	mb_delete_file_dir((char *)Dest_dir, 1);

	file_fd = open((char *)filename, O_CREAT | O_RDWR);
	if (file_fd < 0)
	{
		printf("err open file\n");
		return -1;
	}
	result = write(file_fd, file, file_len);
	if (result < 0)
	{
		printf("err write update file!\n");
		close(file_fd);
		return -1;
	}
	if (access((char *)Dest_dir, 0) != 0)
	{
		mb_create_file_dir((char *)Dest_dir);
	}
	mb_chmod_file_path("777", (char *)filename);
	mb_tar_decompress((char *)filename, (char *)Dest_dir);
	close(file_fd);
	sleep(1);//等待解压完成
	//进行解压文件的校验和复制到当前系统
	sprintf((char*)config_path, "%s/mb_config.cnf", Dest_dir);
	result = parse_config_copy_file((unsigned char *)config_path, Dest_dir,UPDATE);
	if (result < 0)
		return result;
	////没有问题 先复制该tar包到/tmpapp | /tmpauth目录下用于一会检测成功后替换原有的包
	mb_copy_file_path((char *)filename, (char *)Dest_dir);
	return 0;


}


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
-   Function : HexToStr
-   Description：将16进制数转换为16进制的字符串,即"\x01\x02\x03"->"010203"
-   Calls：
-   Calles By:
-   Table Accessed :
-   Table Update :
-   Input  :参数一:pszDest:指向转换后的字符串指针
参数二:pbSrc:指向待转换的源16进制数字符串指针
参数三:nLen:待转换的源长度
-   Output :
-   Return :
-   Other :
***********************************************************************/
void HexToStr(char *pszDest, char *pbSrc, int nLen)
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
-   Function : str2upper
-   Description：将字符串转为大写
-   Calls：
-   Calles By:
-   Table Accessed :
-   Table Update :
-   Input  :参数一:str:待转换的字符串

-   Output :
-   Return :NULL :错误 其他：已转换成大写的字符串指针
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


int File_Compare_MD5(char *src_file, char *comp_file)
{
	int fds, fdc;
	int sizes = 0, sizec = 0, size_read = 0;
	struct stat statbufs;
	struct stat statbufc;
	char *buff_s = NULL, *buff_c = NULL;
	MD5_CTX ctx;
	char md5_s[16] = { 0 }, md5_c[16] = { 0 };
	if (src_file == NULL || comp_file == NULL)
		return -1;
	if (access(src_file, 0) != 0)
	{

		return BOOT_ERROR_OPER_PARSE_CONFIG_ERR_FILE_NOT_FOUND;
	}
	if (access(comp_file, 0) != 0)
	{
		printf("原目录文件不存在[%s],MD5值默认为不同 !\n", comp_file);
		return 0;//此种情况可以说是MD5值不同
	}
	stat(src_file, &statbufs);
	sizes = statbufs.st_size;
	buff_s = boot_malloc(sizes + 10);
	if (buff_s == NULL)
	{
		printf("err malloc!\n");
		return BOOT_ERROR_OPER_SYSTEM_MALLOC_ERR;
	}


	stat(comp_file, &statbufc);
	sizec = statbufc.st_size;
	buff_c = boot_malloc(sizec + 10);
	if (buff_c == NULL)
	{
		printf("err malloc!\n");
		free(buff_s);
		return BOOT_ERROR_OPER_SYSTEM_MALLOC_ERR;
	}


	fds = open(src_file, O_RDONLY);
	if (fds < 0)
	{
		printf("tar包下%s文件打开失败!\n", src_file);
		free(buff_c);
		free(buff_s);
		return BOOT_ERROR_OPER_SYSTEM_READ_ERR;
	}
	size_read = read(fds, buff_s, sizes);
	if (size_read != sizes)
	{
		printf("tar包%s文件读取大小和文件大小不一致!\n", src_file);
		free(buff_c);
		free(buff_s);
		close(fds);
		return BOOT_ERROR_OPER_SYSTEM_READ_ERR;
	}

	MD5_Init(&ctx);
	MD5_Update(&ctx, buff_s, sizes);
	MD5_Final((unsigned char *)md5_s, &ctx);

	close(fds);

	size_read = 0;
	fdc = open(comp_file, O_RDONLY);
	if (fdc < 0)
	{
		free(buff_c);
		free(buff_s);
		return 0;
	}
	size_read = read(fdc, buff_c, sizec);
	if (size_read != sizec)
	{
		//printf("read err ! not full!\n");
		free(buff_c);
		free(buff_s);
		close(fdc);
		return 0;
	}

	MD5_Init(&ctx);
	MD5_Update(&ctx, buff_c, sizec);
	MD5_Final((unsigned char *)md5_c, &ctx);

	close(fdc);

	if (memcmp(md5_s, md5_c, 16) != 0)
	{
		free(buff_c);
		free(buff_s);
		return 0;//MD5值不同
	}
	free(buff_c);
	free(buff_s);
	//printf("MD5值相同不进行替换!\n");
	return 1;//MD5值相同
	

}


int parse_config_del_backfile(char *configname, char *now_path)
{

	FILE *file;
	//int result = 0;
	char lineBuff[256] = { 0 };// 存储读取一行的数据
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
	file = fopen((char *)configname, "rb");
	if (!file)
	{
		return BOOT_ERROR_OPER_PARSE_CONFIG_ERR_OPEN_CONFIG;
	}
	fseek(file, 0, SEEK_END);
	ftell(file); //配置文件长度  
	fseek(file, 0, SEEK_SET);
	memset(lineBuff, 0, sizeof(lineBuff));
	//下面的循环是先判断config文件的格式是否正确
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
		memcpy(file_real_path, linePos + 3, strlen(linePos) - 3);//减去一个" = "


		if (file_real_path[strlen(file_real_path) - 1] == '/')//统一格式(如路径中结尾有'/'，则去掉，后面统一加上)
		{
			file_real_path[strlen(file_real_path) - 1] = '\0';
		}

		memset(tar_file_path, 0, sizeof(tar_file_path));
		sprintf(tar_file_path, "%s/%s", now_path, file_name);
		//printf("back path = %s\n", file_real_path);
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
		if (access(backfile_path, 0) == 0)//进行back备份的删除
		{
		
			memset(cmd, 0, sizeof(cmd));
			sprintf(cmd, "rm -rf %s", backfile_path);
			system(cmd);
			sync();
		}

		memset(lineBuff, 0, sizeof(lineBuff));
		continue;
	}
	return 0;
	fclose(file);


}
int parse_config_back_oldver(char *configname, char *now_path)
{

	FILE *file;
	//int result = 0;
	char lineBuff[256] = { 0 };// 存储读取一行的数据
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
		return BOOT_ERROR_OPER_PARSE_CONFIG_ERR_OPEN_CONFIG;
	}
	fseek(file, 0, SEEK_END);
	ftell(file); //配置文件长度  
	fseek(file, 0, SEEK_SET);
	memset(lineBuff, 0, sizeof(lineBuff));
	//下面的循环是先判断config文件的格式是否正确
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
		memcpy(file_real_path, linePos + 3, strlen(linePos) - 3);//减去一个" = "


		if (file_real_path[strlen(file_real_path) - 1] == '/')//统一格式(如路径中结尾有'/'，则去掉，后面统一加上)
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
		if (access(backfile_path, 0) == 0)//进行back备份的回退
		{
			memset(cmd, 0, sizeof(cmd));
			sprintf(cmd, "cp %s %s", backfile_path, real_file_path);
			system(cmd);
			sync();

			memset(cmd, 0, sizeof(cmd));
			sprintf(cmd, "rm -rf %s", backfile_path);
			system(cmd);
			sync();
		}
	
		memset(lineBuff, 0, sizeof(lineBuff));
		continue;
	}
	fclose(file);
	return 0;
	


}

int parse_config_copy_file(unsigned char *configname,unsigned char *now_path,int update_or_check)
{
	FILE *file;
	int result = 0;
	char lineBuff[256] = { 0 };// 存储读取一行的数据
	char* linePos = NULL;
	char file_name[100] = { 0 };
	char file_real_path[100] = { 0 };
	char tar_file_path[100] = { 0 };
	char real_file_path[100] = { 0 };
	//char cmd[100] = { 0 };
	char update_filename[200] = { 0 };
	
	if (access((char *)configname, 0) != 0)
	{
		return BOOT_ERROR_OPER_PARSE_CONFIG_NOT_FOUND_CONFIG;
	}
	file = fopen((char *)configname, "rb");
	if (!file)
	{
		return BOOT_ERROR_OPER_PARSE_CONFIG_ERR_OPEN_CONFIG;
	}
	fseek(file, 0, SEEK_END);
	ftell(file); //配置文件长度  
	fseek(file, 0, SEEK_SET);
	memset(lineBuff, 0, sizeof(lineBuff));
	//下面的循环是先判断config文件的格式是否正确
	while (fgets(lineBuff, 256, file) != NULL)
	{
		if ((lineBuff[0] == '#') || (lineBuff[0] == ';') || memcmp(lineBuff, "\r\n", 2) == 0 || lineBuff[0] == '\n')
			continue;
		linePos = strstr(lineBuff, "=");
		if (linePos == NULL)
		{
			printf("err format configfile :%s!\n", lineBuff);
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
		if (memcmp(file_name, "mb_boot", 7) == 0)//判断是否是本boot程序，如果是，则跳过.
			continue;
		memset(file_real_path, 0, sizeof(file_real_path));
		if (linePos[strlen(linePos) - 1] == '\n'&&linePos[strlen(linePos) - 2] == '\r')
			linePos[strlen(linePos) - 2] = '\0';
		else if (linePos[strlen(linePos) - 1] == '\n')
			linePos[strlen(linePos) - 1] = '\0';
		memcpy(file_real_path, linePos + 3, strlen(linePos) - 3);//减去一个" = "


		if (file_real_path[strlen(file_real_path) - 1] == '/')//统一格式(如路径中结尾有'/'，则去掉，后面统一加上)
		{
			file_real_path[strlen(file_real_path) - 1] = '\0';
		}

		memset(tar_file_path, 0, sizeof(tar_file_path));
		sprintf(tar_file_path, "%s/%s", now_path,file_name);
		//检测tar目录下是否有要更新的文件和是否存在真实复制替换的目录
		//printf("path1 = %s\npath2 = %s\n", file_real_path, tar_file_path);
		if (access(file_real_path, 0) != 0)
		{
			printf("err format configfile Path1 not found!\n");
			fclose(file);
			return BOOT_ERROR_OPER_PARSE_CONFIG_ERR_PATH_NOT_FOUND;
		}
		if (access(tar_file_path, 0) != 0)
		{
			printf("err format configfile Path1 not found!\n");
			fclose(file);
			return BOOT_ERROR_OPER_PARSE_CONFIG_ERR_FILE_NOT_FOUND;
		}
		memset(real_file_path, 0, sizeof(real_file_path));
		sprintf(real_file_path, "%s/%s", file_real_path, file_name);
		result = File_Compare_MD5(tar_file_path, real_file_path);
		if (result == 0)
		{
			//MD5值不同进行替换
			//原系统有该文件，先备份该文件为filename.back,用于检测失败回退使用
			if (access(real_file_path, 0) == 0)
			{
				char backpath[200] = { 0 };
			/*	memset(cmd, 0, sizeof(cmd));
				sprintf(cmd, "cp %s %s.back", real_file_path, real_file_path);
				system(cmd);
				sync();*/
				sprintf(backpath, "%s.back", real_file_path);
				mb_copy_file_path(real_file_path, backpath);
			}
			//用本次tar压缩包下的文件替换系统下文件
			/*memset(cmd, 0, sizeof(cmd));
			sprintf(cmd, "cp %s %s/.", tar_file_path, file_real_path);
			system(cmd);
			sync();*/
			mb_copy_file_path(tar_file_path, file_real_path);
			//mb_chmod_file_path("777", (char *)real_file_path);

			//printf("###################update_or_check:%d-----now_path:%s\n", update_or_check, now_path);

			if (update_or_check == UPDATE && strstr((char *)now_path,"app") !=NULL)//app.tar更新内容
			{
				//printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
				memset(update_filename, 0, sizeof(update_filename));
				sprintf(update_filename, "updatefile=%s\n", real_file_path);
				strcat((char *)update_msg, update_filename);
			}
			else if (update_or_check == UPDATE && strstr((char *)now_path, "auth") != NULL)//auth.tar更新内容
			{
				memset(update_filename, 0, sizeof(update_filename));
				sprintf(update_filename, "updatefile=%s\n", real_file_path);
				strcat((char *)auth_msg, update_filename);
			}

		}
		else if (result < 0)
		{
			printf("err Compare MD5!\n");
			fclose(file);
			return result;
		}
		memset(lineBuff, 0, sizeof(lineBuff));
		continue;
	}
	fclose(file);
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
		return BOOT_ERROR_OPER_PARSE_CONFIG_ERR_OPEN_CONFIG;
	}
	fseek(file, 0, SEEK_SET);

	memset(lineBuff, 0, sizeof(lineBuff));
	//下面的循环是先判断config文件的格式是否正确
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

int detect_process(char * process_name, char *tmp)
{
	//int   stream;
	//char buff[512];  
	char ps[128];
	char order[200];
	char tmp_file[100];
	int num;
	sprintf(ps, "ps | grep %s | grep -v grep", process_name);
	//out("cmd %s\n",ps);
	unsigned char lineBuff[256];// 存储读取一行的数据  

	memset(order, 0, sizeof(order));
	memset(tmp_file, 0, sizeof(tmp_file));
	sprintf(tmp_file, "/home/share/exe/%s", tmp);
	sprintf(order, "%s > %s", ps, tmp_file);
	//out("order=[%s]\n",order);
	system(order);

	FILE* fp = fopen(tmp_file, "r");
	if (fp == NULL)
	{
		printf("文件打开失败,文件名：%s\n", tmp_file);
		sprintf(order, "rm %s", tmp_file);
		system(order);

		return 1;
	}
	fseek(fp, 0, SEEK_SET);
	num = 0;
	while (fgets((char *)lineBuff, 256, fp) != NULL)
	{

		if (strlen((const char*)lineBuff) < 4) // 空行  
		{
			continue;
		}
		//out("%s\n", lineBuff);
		num += 1;
	}
	fclose(fp);
	if (num >= 1)
	{	//out("检测到进程(%s)存在，无需再次启动,%s\n",process_name,buff);
		sprintf(order, "rm %s", tmp_file);
		system(order);
		return num;
	}
	//out("检测到进程(%s)不存在,需再次启动,num = %d\n",process_name,num);
	//out("检测到进程未启动\n");
	sprintf(order, "rm %s", tmp_file);
	system(order);
	return -1;
}

int check_and_close_process(char * process_name)
{
	//int   stream;
	//int configLen;
	//char buff[512];  
	char ps[128];
	char order[200];
	char tmp_file[100];
	int pidnum;
	int result;
	int i = 0;
	char lineBuff[256];// 存储读取一行的数据  
	pid_t pID;
	sprintf(ps, "ps | grep %s | grep -v grep", process_name);
	//out("cmd %s\n",ps);


	memset(order, 0, sizeof(order));
	memset(tmp_file, 0, sizeof(tmp_file));
	sprintf(tmp_file, "/etc/closetmpcheck.txt");
	sprintf(order, "%s > %s", ps, tmp_file);
	//out("order=[%s]\n",order);
	system(order);

	//stream=open(tmp_file,O_RDWR);;
	//memset(buff,0,sizeof(buff));
	//read(stream,buff,sizeof(buff));
	//close(stream);
	//out("buf = %s\n\n\n",buff);
	FILE* fp = fopen(tmp_file, "r");
	if (fp == NULL)
	{
		printf("文件打开失败\n");

		memset(order, 0, sizeof(order));
		sprintf(order, "rm %s",tmp_file);
		system(order);
		return -1;
	}
	fseek(fp, 0, SEEK_END);
	//configLen = ftell(fp); //配置文件长度  
	fseek(fp, 0, SEEK_SET);

	//out("配置文件总大小为：%d字节\n",configLen);
	memset(lineBuff, 0, sizeof(lineBuff));
	while (fgets(lineBuff, 256, fp) != NULL)
	{
		if (strlen(lineBuff) < 1) // 空行  
		{
			continue;
		}
		i += 1;
		printf("读到第%d行的数据为：%s\n", i, lineBuff);
		pidnum = atoi(lineBuff);
		printf("[%s] process PID num = %d\n", process_name, pidnum);
		pID = (pid_t)pidnum;
		result = kill(pID, SIGKILL);//这里不是要杀死进程，而是验证一下进程是否真的存在，返回0表示真的存在
		printf("process: %s exist!kill pid over,result = %d\n", process_name, result);
	}
	fclose(fp);

	memset(order, 0, sizeof(order));
	sprintf(order, "rm %s", tmp_file);
	system(order);
	return -1;
}

int write_update_record(char *filename, char *writebuf)
{
	//char cmd[100] = { 0 };
	int fd = 0,len = 0;
	//int sleep_count = 0;
	//struct flock lock;
	char tmpfile[100] = { 0 };
	sprintf(tmpfile, "/tmp/update_tmp.log");
	if (filename == NULL || writebuf == NULL)
	{
		return -1;
	}
	if (strlen(writebuf) == 0)
	{
		//无所需要写入的更新文件信息内容
		return 0;
	}
	if (access(filename, 0) == 0)
	{
		mb_delete_file_dir(filename, 0);
	}
	if (access(tmpfile, 0) == 0)
	{
		mb_delete_file_dir(tmpfile,0);
		
	}
	fd = open(tmpfile, O_RDWR | O_CREAT);
	if (fd < 0)
		return -2;
	//独占方式打开文件
	/*lock.l_type = F_WRLOCK;
	fcntl(fd, F_SETLK,lock);*/
	len = write(fd, writebuf, strlen(writebuf));
	if (len != strlen(writebuf))
	{
		/*lock.l_type = F_UNLOCK;
		fcntl(fd, F_SETLK, lock);*/
		close(fd);
		return -3;
	}
	/*lock.l_type = F_UNLOCK;
	fcntl(fd, F_SETLK, lock);*/
	close(fd);
	mb_copy_file_path(tmpfile, filename);
	return 0;
}


int mb_delete_file_dir(char *file_path, int file_type)
{
	char cmd[1000] = { 0 };
	if (file_type == 0)
	{
		//文件删除
		sprintf(cmd, "rm %s", file_path);
	}
	else
	{
		//文件夹删除
		sprintf(cmd, "rm -rf %s/", file_path);
	}
	system(cmd);
	sync();
	return 0;
}

int mb_create_file_dir(char *file_path)
{
	char cmd[1000] = { 0 };
	sprintf(cmd, "mkdir %s", file_path);
	system(cmd);
	sync();
	return 0;
}

int mb_tar_decompress(char *tar, char *path)
{
	char cmd[1000] = { 0 };
	memset(cmd, 0, sizeof(cmd));
	sprintf(cmd, "tar -xf %s -C %s/", tar, path);
	system(cmd);
	sync();
	return 0;
}

int mb_copy_file_path(char *from, char *to)
{
	char cmd[1000] = { 0 };
	memset(cmd, 0, sizeof(cmd));
	sprintf(cmd, "cp %s %s", from, to);
	system(cmd);
	sync();
	return 0;
}

int mb_chmod_file_path(char *power, char *path)
{
	char cmd[1000] = { 0 };
	memset(cmd, 0, sizeof(cmd));
	sprintf(cmd, "chmod %s %s", power, path);
	system(cmd);
	sync();
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


int mb_system_read(char *cmd, char *r_buf, int r_buf_len)
{
	char order[200];
	char tmp_file[100];
	int fd;
	int result;

	memset(order, 0, sizeof(order));
	memset(tmp_file, 0, sizeof(tmp_file));
	sprintf(tmp_file, "/tmp/system_cmd.log");
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
	mb_delete_file_dir(tmp_file, 0);
	sync();
	return 0;
}

long long calc_path_size(char *path)
{
	struct statfs diskInfo;

	statfs(path, &diskInfo);
	unsigned long long blocksize = diskInfo.f_bsize;	//每个block里包含的字节数
	//unsigned long long totalsize = blocksize * diskInfo.f_blocks; 	//总的字节数，f_blocks为block的数目
	//printf("Total_size = %llu B = %llu KB = %llu MB = %llu GB\n",
		//totalsize, totalsize >> 10, totalsize >> 20, totalsize >> 30);

	unsigned long long freeDisk = diskInfo.f_bfree * blocksize;	//剩余空间的大小
	//unsigned long long availableDisk = diskInfo.f_bavail * blocksize; 	//可用空间大小
	return freeDisk >> 20;
}

