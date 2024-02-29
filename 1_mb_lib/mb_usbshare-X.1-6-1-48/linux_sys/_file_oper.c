#define _file_oper_c
#include "_file_oper.h"
int file_mod(char *file)
{   FILE   *stream;   
    char buf[300];
    memset(buf,0,sizeof(buf));
    sprintf(buf,"chmod 777 %s",file);
    stream=popen(buf,"r");
    pclose(stream);
    return 0;  
}






int file_oper_copy(char *scr,char *dest)
{   //FILE   *stream;   
    char buf[300];
    memset(buf,0,sizeof(buf));
    sprintf(buf,"cp %s %s",scr,dest);
	system(buf);
    //stream=popen(buf,"r");
    //pclose(stream);
    return 0;       
}

int file_seek_creat_dir(char *dir,char *name)
{   FILE   *stream;
    char buf[2048];
    int result;
    memset(buf,0,sizeof(buf));
    sprintf(buf,"ls %s/%s*",dir,name);
    stream = popen(buf,"r");
    memset(buf,0,sizeof(buf));
    fread(buf,sizeof(char),sizeof(buf),stream); 
    pclose(stream); 
    if(strstr(buf,name)!=NULL)
    {   file_mod(buf);
        out("该目录已经存在\n");
        return 0;    
    }
    memset(buf,0,sizeof(buf));
    sprintf(buf,"%s/%s",dir,name);
    out("创建:%s\n",buf);
    result=mkdir(buf,S_IRUSR|S_IREAD|S_IWRITE);  
    file_mod(buf);
    return result;
}
