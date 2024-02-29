#include "../inc/file_oper.h"

int file_mod(char *file)
{
    FILE *stream = NULL;
    char buf[256];
    memset(buf,0,sizeof(buf));
    sprintf(buf,"chmod 777 %s",file);
    stream = popen(buf,"r");
    pclose(stream);
    return 0;
}

int file_oper_copy(char *scr,char *dest)
{
    char buf[256];
    memset(buf,0,sizeof(buf));
    sprintf(buf,"cp %s %s",scr,dest);
	system(buf);
    return 0;
}

int file_seek_creat_dir(char *dir,char *name)
{
    FILE   *stream = NULL;
    char buf[2048];
    int result;
    memset(buf,0,sizeof(buf));
    sprintf(buf,"ls %s/%s*",dir,name);
    stream = popen(buf,"r");
    memset(buf,0,sizeof(buf));
    fread(buf,sizeof(char),sizeof(buf),stream);
    pclose(stream);
    if(strstr(buf,name)!=NULL)
    {
        file_mod(buf);
        file_oper_out("该目录已经存在\n");
        return 0;
    }
    memset(buf,0,sizeof(buf));
    sprintf(buf,"%s/%s",dir,name);
    file_oper_out("创建:%s\n",buf);
    result = mkdir(buf,S_IRUSR|S_IREAD|S_IWRITE);
    file_mod(buf);
    return result;
}
