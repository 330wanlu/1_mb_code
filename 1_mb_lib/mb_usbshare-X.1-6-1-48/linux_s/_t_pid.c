#define _t_pid_c
#include "_t_pid.h"
int _t_pid_n(void)
{   pid_t pid;
    out("获取本程序进程号\n");    
    pid = getpid();
    return pid;
}

int _t_hpid_get_file_name(char *r_file_name)
{   char path[DF_TY_PATH_MAX];
    char link[DF_TY_PATH_MAX];
    int len;
    out("获取程序运行的全称\n");
    memset(path,0,sizeof(path));
    memset(link,0,sizeof(link));
    sprintf(link,"%s/%d/exe",DF_T_PID_DIR,getpid());
    out("读取文件[%s]\n",link);
    if(readlink(link, path, sizeof(path))<0)
        return -1;
    if(r_file_name==NULL)
        return 0;
    len=strlen(path);
    memcpy(r_file_name,path,len);
    return len;
}

int _t_pid_path(char *r_path)
{   int result;
    char path[DF_TY_PATH_MAX];
    memset(path,0,sizeof(path));
    result=_t_hpid_get_file_name(path);
    if(result<0)
    {   out("获取程序运行全称失败\n");
        return result;
    }
    if(r_path==NULL)
        return 0;
    get_dir(path,r_path);
    r_path[strlen(r_path)-1]=0;
    return strlen(r_path);
}

int _t_pid_name(char *name)
{   int result;
    char path[DF_TY_PATH_MAX];
    memset(path,0,sizeof(path));
    result=_t_hpid_get_file_name(path);
    if(result<0)
    {   out("获取程序运行全称失败\n");
        return result;
    }
    if(name==NULL)
        return 0;
    get_dir_file(path,name);
    return strlen(name);
}





int test_t_pid(int cm)
{   if(cm==DF_T_PID_CM_N)
    {   printf("pid=%d\n",_t_pid_n());   
        return 0;
    }
    else if(cm==DF_T_PID_CM_PATH)
    {   char path[DF_TY_PATH_MAX];
        memset(path,0,sizeof(path));
        if(_t_pid_path(path)<0)
        {   printf("test [_t_pid_path] err\n");
            return -1;
        }
        printf("[_t_pid_path]:%s\n",path);
        return 1;    
    }
    return -1;
}


