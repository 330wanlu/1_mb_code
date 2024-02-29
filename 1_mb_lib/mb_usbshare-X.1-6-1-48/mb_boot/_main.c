#define _main_c
#include "_main.h"
int main(int argc, char *argv[])
{   struct _app_file_note   file_note;
    int i;
    int ver,n;
    int s_ver;
    for(i=0,ver=-1,n=-1;i<sizeof(soft_d)/sizeof(const char *);i++)
    {   printf("file_note.name=%s\n",soft_d[i]);
        memset(&file_note,0,sizeof(file_note));
        if(app_file_get(soft_d[i],&file_note)<0)
        {   printf("get err\n");
            continue;
        }    
        
        printf("file_note.len=%d\n",file_note.len);
        printf("file_note.r_date=%s\n",file_note.r_date);
        printf("file_note.ver=%s\n",file_note.ver);
        printf("file_note.writer=%s\n",file_note.writer);
        s_ver=ismoney(file_note.ver+2);
        if(s_ver>ver)
        {   ver=s_ver;
            n=i;
        }
    }
    if(n<0)
    {   printf("no oper app\n");
        return -1;
    }
    app_file_oper(soft_d[n]);
    
    
    
}





