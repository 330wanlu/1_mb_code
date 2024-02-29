#define _app_file_c
#include "_app_file.h"
#include "../mb_boot/_comm_boot.h"
#include "../mb_boot/_tools_boot.h"
int app_file_oper(const char *file)
{  
    char soft_file[200]; 
    char order[200];
    memset(soft_file,0,sizeof(soft_file));
	
    get_soft_file(file,soft_file);
	mb_chmod_file_path("777", BOOT_APP_PATH);
    memset(order,0,sizeof(order));
    sprintf(order,"%s -D",soft_file);
	system(order);
	return 0;
}
int app_file_detect(const char *file,char *outerr)
{
	char soft_file[200];
	char order[200];
	char *p_e = NULL, *p_f = NULL;
	char read_stream[1024] = { 0 };
	

	if (outerr == NULL || file == NULL)
		return -1;
	if (access(file, 0) != 0)
		return -1;
	memset(soft_file, 0, sizeof(soft_file));
	get_soft_file(file, soft_file);
	mb_chmod_file_path("777", BOOT_APP_PATH);
	memset(order, 0, sizeof(order));
	sprintf(order, "%s -V", soft_file);
	printf("order=[%s]\n", order);
	
	
	mb_system_read(order, read_stream, sizeof(read_stream));


	if ((p_e = strstr(read_stream, "err")) != NULL)//���ʧ�ܣ��ó�������Ϣ����
	{
		p_f = strstr(p_e, "\n");
		memcpy(outerr, p_e, p_f - p_e);
		printf("the err info is :\n%s\n", outerr);
		return -1;
	}
	else if ((p_e = strstr(read_stream, "DETECT SUCCESS")) == NULL)//û���յ��������������ɵı�ʶ����ʶ����δ����
	{
		memcpy(outerr, "app start err!", 14);
		return -1;
	}
	return 0;
}

static int get_soft_file(const char *file,char *r_file)
{   //char r_name[100];
    get_dir(file,r_file);
    get_dir_file(file,r_file+strlen(r_file));
    memcpy(r_file+strlen(r_file),"_d",2);
    return 0;
}

static int get_s_info_new(struct	_s_info_new		*infor, char *buf)		//��ȡ��Ϣͷ
{
	
	char app_len[4] = { 0 };
	memcpy(infor->name, buf,20);
	memcpy(infor->type, buf + 20,20);
	memcpy(infor->app_ver, buf + 40, 20);
	memcpy(infor->app_md5, buf + 60, 16 );//20
	memcpy(app_len, buf + 80, 4);//20
	infor->app_len = (app_len[0] * 0x1000000) + (app_len[1] * 0x10000) + (app_len[2] * 0x100) + app_len[3];
	//printf("len is :%ld\n", infor->app_len);
	return 0;
}

static int app_file_write_real_app(char *file, char *writebuf,int len)
{
	char real_name[30] = { 0 };
	//char cmd[300] = { 0 };
	int fd;
	sprintf(real_name, "%s_d", file);
	if (access(real_name, 0) == 0)
	{
	/*	sprintf(cmd, "rm -rf %s", real_name);
		system(cmd);
		sync();*/
		mb_delete_file_dir(real_name, 0);
	}
	fd = open(real_name, O_CREAT | O_RDWR);
	if (fd < 0)
		return -1;
	write(fd, writebuf, len);
	close(fd);

	//memset(cmd, 0, sizeof(cmd));
	//sprintf(cmd, "chmod 777  %s", real_name);
	//system(cmd);
	//sync();
	mb_chmod_file_path("777", real_name);
	return 0;

}

int app_file_get_new(const char *file, struct _app_file_note *file_note)
{
	struct _s_info_new infor;
	int result;
	memset(&infor, 0, sizeof(struct _s_info_new));
	//printf("step1\n");
	if (access(file, 0) != 0)
		return -1;

	//printf("step2\n");
	if ((result = app_file_get_infor_new((char *)file, &infor) )< 0)
	{
		printf("step3 err:%d\n",result);
		return -1;
	}
	//printf("step3\n");
	result = app_file_write_real_app((char *)file, infor.app_content, infor.app_len);
	if (result < 0)
	{
		free(infor.app_content);
		return -2;
	}
	//printf("step4\n");
	free(infor.app_content);
	return 0;

}
int app_file_get_infor_new(const char *file, struct _s_info_new *infor)
{
	int fd;
	struct stat pro;
	long result;
	char buf[100];
	char outmd[16] = { 0 };
	MD5_CTX ctx;
	fd = open(file, O_RDWR);
	if (fd<0)
	{
		return fd;
	}
	memset(&pro, 0, sizeof(pro));
	stat(file, &pro);
	out("pro.st_size  = %d\n", pro.st_size);
	if (pro.st_size<100)
	{
		close(fd);
		return -2;
	}
	lseek(fd, 0, SEEK_SET);
	memset(buf, 0, sizeof(buf));
	result = read(fd, buf, sizeof(buf));
	if (result != sizeof(buf))
	{
		close(fd);
		return -3;
	}
		
	//close(fd);
	if (get_s_info_new(infor, buf)<0)
	{
		close(fd);
		return -4;
	}
	infor->app_content = malloc(infor->app_len + 10);
	if (infor->app_content == NULL)
	{
		close(fd);
		return -5;
	}
	result = read(fd, infor->app_content, infor->app_len);
	if (result != infor->app_len)
	{
		close(fd);
		return -6;
	}

	MD5_Init(&ctx);
	MD5_Update(&ctx, infor->app_content, infor->app_len);
	MD5_Final((unsigned char*)outmd, &ctx);

	if (memcmp(outmd, infor->app_md5, 16) == 0)
	{
		close(fd);
		return 0;
	}
	free(infor->app_content);
	close(fd);
	return -6;
}