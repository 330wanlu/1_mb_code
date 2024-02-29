#define _folder_c
#include "_folder.h"
int _folder_ls_seek(const char *path,const char *name,char r_file[][256],int size_r)
{   out("在%s下查找[%s*]文件\n",path,name);
    DIR *dp;
    struct dirent *entp;
    int i,len;
    if((dp=opendir(path))==NULL)
    {   out("该文件夹不存在或者为空\n");
        return 0;
    }
    for(len=strlen(name),i=0;;)
    {   entp=readdir(dp);   
        if(entp==NULL)
            break;
        if(memcmp(name,entp->d_name,len)!=0)
            continue;
        if(r_file!=NULL)
            memcpy(r_file[i++],entp->d_name,strlen(entp->d_name));
        else
            i++;
        if(i==size_r)
            break;
    }
    closedir(dp);
    return i;
}


int _folder_new(const char *path,const char *new_folder)
{   int result;
    char file_name[256];
    out("在%s下创建%s文件夹\n",path,new_folder);
    memset(file_name,0,sizeof(file_name));
    sprintf(file_name,"%s/%s",path,new_folder);
    out("创建文件夹名称:[%s]\n",file_name);
    result=_file_pro_type(file_name);
    if(result==DF_FILE_PRO_DIR)
    {   out("该文件夹已经存在\n");
        return 0;
    }
    if(result>=0)
    {   out("该文件存在,但属性不是文件夹\n");
        return -2;
    }
    return mkdir(file_name,S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH);
}

