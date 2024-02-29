#define _t_file_c_c
#include "_t_file_c.h"
int _t_file_c_read(const char *file_name,char *buf,int buf_size)
{   int fd;
    int result;
    memset(buf,0,buf_size);
    result=_file_pro_type(file_name);
    if(result<0)
    {   out("该文件不存在\n");
        return result;
    }    
    if(result==DF_FILE_PRO_DIR)
    {   out("该文件是文件夹\n");
        return -1;
    }
    fd=open(file_name,O_RDONLY);
    if(fd<0)
    {   out("打开文件失败\n");
        return -1;
        
    }    
    result = read(fd, buf, buf_size);  
    if(result<0)
    {   out("读取文件失败\n");

    }
    close(fd);    
    return result;    
}

int _t_file_c_write_no_file(const char *file_name, char *buf, int buf_size)
{   int fd;
    int result;
    result=_file_pro_type(file_name);
    if(result<0)
    {   out("该文件不存在\n");
		//printf("This path does no exit\n");
        //return result;
    }    
    if(result==DF_FILE_PRO_DIR)
    {   out("该文件是文件夹\n");
		printf("This path was not file\n");
        return -1;
    }
	unlink(file_name);
    fd=open(file_name,O_WRONLY|O_CREAT,0644);
    if(fd<0)
	{	//printf("open %s failed!\n",file_name);
        return -1;		
	}
	lseek(fd,0,SEEK_SET);
    result = write(fd, buf, buf_size);  
	if(result <0)
	{
		printf("write %s file buf %s failed\n",file_name,buf);
	}
    close(fd);
	//sync();	
	return result;   
}

int _t_file_c_write(const char *file_name,char *buf,int buf_size)
{   int fd;
    int result;
    result=_file_pro_type(file_name);
    if(result<0)
    {   out("该文件不存在\n");
		printf("This path does no exit\n");
        return result;
    }    
    if(result==DF_FILE_PRO_DIR)
    {   out("该文件是文件夹\n");
		printf("This path was not file\n");
        return -1;
    }
	unlink(file_name);
    fd=open(file_name,O_WRONLY|O_CREAT,0644);
    if(fd<0)
	{	printf("open %s failed!\n",file_name);
        return -1;		
	}
	lseek(fd,0,SEEK_SET);
    result = write(fd, buf, buf_size);  
	if(result <0)
	{
		printf("write %s file buf %s failed\n",file_name,buf);
	}
    close(fd);
	//sync();	
	return result;   
}

int read_file(const char *file_name, char *buf, int buf_size)
{
	return _t_file_c_read(file_name, buf, buf_size);
}

int write_file(const char *file_name, char *buf, int buf_size)
{
	return _t_file_c_write_no_file(file_name, buf, buf_size);
}

void chmod_file(char *file_name, char *level)
{
	char cmd[200] = { 0 };
	sprintf(cmd, "chmod %s %s", level, file_name);
	system(cmd);
}

void copy_file(char *src_file_name, char *dest_file_name)
{
	char cmd[400] = { 0 };
	sprintf(cmd, "cp %s %s", src_file_name, dest_file_name);
	system(cmd);
}

void copy_file_cmp(char *src_file_name, char *dest_file_name)
{
	char src_file[1024*100];
	char dest_file[1024 * 100];
	char cmd[400] = { 0 };
	if (read_file(src_file_name, src_file, sizeof(src_file)) < 0)
	{
		goto End;
	}
	if (read_file(dest_file_name, dest_file, sizeof(dest_file)) < 0)
	{
		goto End;
	}
	if (strcmp(src_file, dest_file) == 0)
		return;
End:
	sprintf(cmd, "cp %s %s", src_file_name, dest_file_name);
	system(cmd);
}

void delete_file(char *file_name)
{
	char cmd[400] = { 0 };
	if (file_exists(file_name) != 0)
		return;
	sprintf(cmd, "rm %s", file_name);
	system(cmd);
}

void delete_dir(char *dir_name)
{
	char cmd[400] = { 0 };
	if (file_exists(dir_name) != 0)
		return;
	sprintf(cmd, "rm -rf %s", dir_name);
	system(cmd);
}

void copy_dir(char *src_dir_name, char *dest_dir_name)
{
	char cmd[400] = { 0 };
	sprintf(cmd, "cp -rf %s %s", src_dir_name, dest_dir_name);
	system(cmd);
}

int file_exists(char *file_name)
{
	if (access(file_name, 0) != 0)
	{
		return -1;
	}
	return 0;
}