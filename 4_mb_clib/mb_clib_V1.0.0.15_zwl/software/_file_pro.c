#define _file_pro_c
#include "_file_pro.h"
int _file_pro_ls(const char *file_name)
{   DIR *dp;
    struct dirent *entp;
    char path[256];
    char name[256];
    int len;
    if((file_name==NULL)||(strlen(file_name)==0)||(strlen(file_name)>=256))
    {   out("�ļ������Ϸ�\n");
        return -1;    
    }
    out("���Ҹ��ļ������ļ����Ƿ����\n");
    memset(path,0,sizeof(path));
    memset(name,0,sizeof(name));
    if(get_dir(file_name,path)<0)
    {   out("��ȡĿ¼ʧ��\n");
        return -2;
    }
    if(get_dir_file(file_name,name)<0)
    {   out("��ȡ�ļ�ʧ��\n");
        return -3;
    }
    if((dp=opendir(path))==NULL)
    {   out("���ļ��в����ڻ���Ϊ��\n");
        return -4;
    }
    for(len=strlen(name);;)
    {   entp=readdir(dp);   
        if(entp==NULL)
            break;
        if(len!=strlen(entp->d_name))
            continue;
        if(memcmp(name,entp->d_name,len)!=0)
            continue;
        out("���ļ�����Ѱ�ҵ����ļ�\n");
        closedir(dp);
        return 0;
    }
    closedir(dp);
    out("���ļ�����û�и��ļ�\n");
    return -5;    
}


int _file_pro_type(const char *file_name)
{   struct stat struct_stat;
    out("��ȡ���ļ�ʱ�ļ��л����ļ�\n");
    memset(&struct_stat,0,sizeof(struct stat));
    if(stat(file_name,&struct_stat)!=0)
    {   out("���ļ�������\n");
        return -1;
    }
    if(S_ISDIR(struct_stat.st_mode))
    {   out("���ļ�Ϊ�ļ���\n");
        return DF_FILE_PRO_DIR;
    }
    else if(S_ISREG(struct_stat.st_mode))
    {   out("���ļ�Ϊ��ͨ�ļ�\n");
        return DF_FILE_PRO_FILE;
    }
    else if(S_ISCHR(struct_stat.st_mode))
    {   out("���ļ�Ϊ�ַ����ļ�\n");
        return DF_FILE_PRO_FILE_CHAR;
    }
    else if(S_ISBLK(struct_stat.st_mode))
    {   out("���ļ�Ϊ���豸�ļ�\n");
        return DF_FILE_PRO_FILE_BLK;
    }
    else if(S_ISFIFO(struct_stat.st_mode))
    {   out("���ļ�Ϊ�ܵ��ļ�\n");
        return DF_FILE_PRO_FILE_FIFO;
    }
    else if(S_ISLNK(struct_stat.st_mode))
    {   out("���ļ�Ϊ������(��������)�ļ�\n");
        return DF_FILE_PRO_FILE_LNK;
    }
    else if(S_ISSOCK(struct_stat.st_mode))
    {   out("���ļ�Ϊ�׽����ļ�\n");
        return DF_FILE_PRO_FILE_SOCK;
    }
    return 0;
}

int _file_pro_size(const char *file_name)
{   struct stat struct_stat;      
    out("��ȡ���ļ���С\n");
    memset(&struct_stat,0,sizeof(struct stat));
    if(stat(file_name,&struct_stat)!=0)
    {   out("���ļ�������\n");
        return -1;
    }
    return struct_stat.st_size;
}

int _file_exsit_size(char *file_name)
{
	struct stat struct_stat;     
	int result;
	result = access(file_name,0);
	if(result < 0)
	{	out("���ļ�������\n");
		return -1;
	}    
    memset(&struct_stat,0,sizeof(struct stat));
    if(stat(file_name,&struct_stat)!=0)
    {   out("��ȡ�ļ���Сʧ��\n");
        return -2;
    }
    return struct_stat.st_size;
}

