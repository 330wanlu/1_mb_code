#define _app_file_c
#include "_app_file.h"
int app_file_oper(const char *file)
{   FILE   *stream;
    char soft_file[200]; 
    char order[200];
    memset(soft_file,0,sizeof(soft_file));
    get_soft_file(file,soft_file);
    memset(order,0,sizeof(order));
    sprintf(order,"%s -D",soft_file);
    out("order=[%s]\n",order);
    stream=popen(order,"w");
    if(stream==NULL)
    {   out("运行管道失败\n");
        return -1;
    }
    for(;;sleep(3600));
}

int app_file_juge_new(const char *file,char *ver,char *file_ver)
{   struct _app_file_note    note;
    int a_ver,s_ver;
    int result;
    memset(&note,0,sizeof(note));
    result=app_file_get(file,&note);
    if(result<0)
        return result;
	memcpy(file_ver,note.ver,sizeof(note.ver));
	s_ver = ismoney((uint8 *)note.ver + 2);
	a_ver = ismoney((uint8 *)ver + 2);
    if(s_ver==a_ver)
	{
		out("s_ver==a_ver\n");
        return 0;
	}
    else if(s_ver<a_ver)
	{	out("s_ver<a_ver\n");
        return 1;
	}
    else
	{	return 2;
	}
}

int app_file_juge_down_now_ser(const char *file,char *ver,char *ser_ver)
{   struct _app_file_note    note;
    int a_ver,s_ver;
    int result;
    memset(&note,0,sizeof(note));
    result=app_file_get(file,&note);
    if(result<0)
        return result;
    s_ver=ismoney((uint8 *)note.ver+2);
	a_ver = ismoney((uint8 *)ver + 2);
    if(s_ver==a_ver)
	{
		out("s_ver==a_ver\n");
        return 0;
	}
    else if(s_ver<a_ver)
	{	out("s_ver<a_ver\n");
        return 1;
	}
    else
	{	memcpy(ser_ver,note.ver,sizeof(note.ver));
		return 2;		
	}
}



int app_file_get(const char *file,struct _app_file_note *file_note)
{   uint8 *soft_buf;
    struct _s_info     infor;
    char soft_file[200];   
    int result;
    out("获取文件的基本信息\n");
    memset(&infor,0,sizeof(infor));
    if(app_file_get_infor(file,&infor)<0)
    {   out("获取信息头失败\n");
        return -1;
    }
    out("获取程序运行的文件名\n");
    memset(soft_file,0,sizeof(soft_file));
    get_soft_file(file,soft_file);
    soft_buf=get_soft_ware((char *)file,&infor);
    if(soft_buf==NULL)
    {   out("获取软件失败\n");
        return -1;
    }
    out("比较并创建运行软件\n");
    result=cmp_soft_new(soft_file,soft_buf,infor.s_len);
    free(soft_buf);
    if(result<0)
    {   out("创建软件失败\n");
        return -1;
    }
    if(get_soft_infor(soft_file,file_note)<0)
    {   out("获取版本信息失败\n");
        return -1;
    }
    file_note->len=infor.s_len;
    return 0;
}

int app_file_get_infor(const char *file,struct	_s_info *infor)
{   int fd;
    struct stat pro;
    uint8 buf[128];
    fd=open(file,O_RDWR); 
    if(fd<0)
    {   out("file=%s,打开错误\n",file);
        return fd;
    }
    memset(&pro,0,sizeof(pro));
    stat(file,&pro);
	out("pro.st_size  = %d\n",pro.st_size);
    if(pro.st_size<128)
    {   out("信息头长度不够,pro.st_size=%d\n",pro.st_size);
        return -2;
    }
    lseek(fd,0,SEEK_SET);
    memset(buf,0,sizeof(buf));
    read(fd,buf,sizeof(buf));
    close(fd);
    if(get_s_infor(infor,buf)<0)
    {   out("获取信息头失败\n");
        return -3;
    }
    out("获取信息头成功\n");
    return 0;
}

static int get_soft_infor(const char *soft_file,struct _app_file_note *file_note)
{   int   stream;
    char order[200];
    char buf[1024];
    //char *pbuf,*pbuf_e;
	char tmp_file[100];
    out("获取软件:[%s]的版本信息\n",soft_file);
    memset(order,0,sizeof(order));
	memset(tmp_file,0,sizeof(tmp_file));
	sprintf(tmp_file,"/tmp/tmpsoft.txt");
    sprintf(order,"%s -v > %s",soft_file,tmp_file);
    out("order=[%s]\n",order);
	system(order);
	stream=open(tmp_file,O_RDWR);;
    //stream=popen(order,"r");
    //if(stream==NULL)
    //{   out("运行管道失败\n");
    //    return -1;
    //}
    memset(buf,0,sizeof(buf));
	read(stream,buf,sizeof(buf));
	close(stream);
	//fgets(buf, sizeof(buf), stream);
	//fgets(buf+strlen(buf), sizeof(buf), stream);
	//fgets(buf+strlen(buf), sizeof(buf), stream);
	//fgets(buf+strlen(buf), sizeof(buf), stream);
	//fgets(buf+strlen(buf), sizeof(buf), stream);
	//fgets(buf+strlen(buf), sizeof(buf), stream);
    //fread(buf,sizeof(char),buf,stream); 
	out("buf = %s\n\n\n",buf);
    //pclose(stream);
    get_feild(buf,"name",file_note->name);
    get_feild(buf,"r_date",file_note->r_date);
    get_feild(buf,"ver",file_note->ver);
    get_feild(buf,"writer",file_note->writer);


	memset(order, 0, sizeof(order));

	sprintf(order, "rm %s",tmp_file);
	
	system(order);
	sync();

    return 0;
}











static int cmp_soft_new(const char *soft_file,uint8 *soft_buf,int size_buf)
{   int fd,i,i_size;
    uint8 soft[1024];
    struct stat pro;
    if(access(soft_file,0)!=0)
    {   out("没有该文件[%s]存在\n",soft_file);
        fd=open(soft_file,O_RDWR|O_CREAT);
        if(fd<0)
        {   out("创建文件:%s,失败\n",soft_file);
            return -1;
        }    
        write(fd,soft_buf,size_buf);
        close(fd);
        file_mod((char *)soft_file);
        return 0;
    }
    memset(&pro,0,sizeof(pro));
    stat(soft_file,&pro);
    if(pro.st_size!=size_buf)
    {   out("软件大小不一致,进行更新\n");
        unlink(soft_file);
        fd=open(soft_file,O_RDWR|O_CREAT);
        if(fd<0)
        {   out("创建文件:%s,失败\n",soft_file);
            return -1;
        }    
        write(fd,soft_buf,size_buf);
        close(fd);
		file_mod((char *)soft_file);
        return 0;
    }
    fd=open(soft_file,O_RDONLY);
    if(fd<0)
        return -1;
    lseek(fd,0,SEEK_SET);
    for(i=0;i<size_buf;i+=i_size)
    {   i_size=sizeof(soft);
        if((i+i_size)>size_buf)
            i_size=size_buf-i;
        read(fd,soft,i_size);
        if(memcmp(soft,soft_buf+i,i_size)!=0)
        {   out("发现软件不一致\n");
            break;
        }
    }
    close(fd);
    if(i!=size_buf)
    {   unlink(soft_file);
        fd=open(soft_file,O_RDWR|O_CREAT);
        if(fd<0)
        {   out("创建文件:%s,失败\n",soft_file);
            return -1;
        }    
        write(fd,soft_buf,size_buf);
        close(fd);
		file_mod((char *)soft_file);
        return 0;
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



static uint8 *get_soft_ware(char *file,struct _s_info     *infor)
{   uint8 *buf;
    uint16 crc_val;
    int fd;
    out("获取软件,%s\n",file);
    buf=malloc(infor->s_len);
    if(buf==NULL)
    {   out("内存分配错误\n");
        return NULL;    
    }
    fd=open(file,O_RDWR); 
    lseek(fd,infor->offset,SEEK_SET);
    read(fd,buf,infor->s_len);
    crc_val=crc_8005_int(buf,infor->s_len,0);
    if(crc_val!=infor->crc_val)
    {   free(buf);
        return NULL;
    }
    out("软件获取成功\n");
    return buf;
}





static int get_s_infor(struct	_s_info		*infor,uint8 *buf)		//获取信息头
{	int buf_len;
	uint16 crc_val,crc;
	out("%s------------------信息段检测------------------\n",DF_FORMATER_TAB);
	if((buf[0]!='T')&&(buf[1]!='Y'))
	{	out("%s%s信息段检测错误\r\n",DF_FORMATER_TAB,DF_FORMATER_ERR);
		out("%s----------------------------------------------\r\n",DF_FORMATER_TAB);
		return _INFOR_ERR_CUT;							//信息头不对
	}
	memcpy(&buf_len,buf+2,sizeof(buf_len));
	if((buf_len>128)||(buf_len<10))
	{	out("%s%s长度错误\r\n",DF_FORMATER_TAB,DF_FORMATER_ERR);
		out("%s----------------------------------------------\r\n",DF_FORMATER_TAB);
		return _INFOR_ERR_CUT;							//长度错误
	}	
	crc_val=crc_8005((uint8 *)buf,buf_len-2,0);
	memcpy(&crc,buf+buf_len-sizeof(crc),sizeof(crc));
	if(crc!=crc_val)	
	{	out("%s%s信息段CRC错误\r\n",DF_FORMATER_TAB,DF_FORMATER_ERR);
		out("%s----------------------------------------------\r\n",DF_FORMATER_TAB);
		return _INFOR_ERR_CUT;							//CRC校验错误
	}	
	buf+=(2+sizeof(buf_len));
	memcpy(infor->name, buf, strlen((const char *)buf));
	buf += (strlen((const char *)buf) + 1);
	memcpy(&infor->s_addree,buf,sizeof(infor->s_addree));
	buf+=sizeof(infor->s_addree);
	memcpy(&infor->s_len,buf,sizeof(infor->s_len));
	buf+=sizeof(infor->s_len);
	memcpy(&infor->crc_val,buf,sizeof(infor->crc_val));
    infor->offset=infor->s_addree-0X8006000;
    out("infor->name=%s\n",infor->name);
    out("infor->s_addree=%08x\n",infor->s_addree);
    out("infor->s_len=%d\n",infor->s_len);    
    out("infor->crc_val=%04x\n",infor->crc_val);
    
    
    
	out("%s%s信息段检测成功\r\n",DF_FORMATER_TAB,DF_FORMATER_SUC);
	out("%s----------------------------------------------\r\n",DF_FORMATER_TAB);
	return 0;
}