#define _ma_id_c
#include "_ma_id.h"
int read_ma_id(struct st_ma_id  *id)
{   memset(id,0,sizeof(struct st_ma_id));
    memcpy(id->hard,"YA1.00",6);
    memcpy(id->kernel,"linux 2.6.32-38-generic",strlen("linux 2.6.32-38-generic"));
    id->id[0]=0x01;id->id[1]=0x50;id->id[2]=0x20;
    id->id[3]=0x00;id->id[4]=0x00;id->id[5]=0x01;
    return 0; 
}

int read_kernel(char *ker)
{   FILE   *stream;
    char buf[2048];
    char *pbuf;
    stream = popen("uname -r","r");
    memset(buf,0,sizeof(buf));
    fread(buf,sizeof(char),sizeof(buf),stream); 
    pclose(stream);
    pbuf=strstr(buf,"\n");
    if(pbuf==0)
        return 0;
    memcpy(ker,buf,pbuf-buf);
    return 0;
}

static int get_config_file_head_end(const char *buf,const char *head,const char *end,char **p_head,char **p_end)
{	
	//char lan[100];
	//memset(lan,0,sizeof(lan));
	////sprintf(lan,"#!/bin/sh");
	//sprintf(lan,head);
	*p_head=strstr(buf,head);
	if(*p_head==NULL)
	{	/*printf("[%s]未找到\n",head);*/
		return -1;
	}
	*p_end=strstr(*p_head,end);
	if(*p_end==NULL)
	{	/*printf("未找到数据结束\n");*/
		return -2;
	}
	//*p_head+=strlen(lan);
	*p_end+=4;
	return 0;
}

static int get_config_data(char *str_s,char *end_s,char *key,char *value)
{	char *ps,*end_ps;	
	char *str,*end;
	char buf[30];
	//char mask[2];
	int i;
	//int netmask;
	str = str_s;
	end = end_s;
	ps=strstr(str,key);
	if((ps==NULL)||(ps>end))
	{	return -1;
	}
	ps+=strlen(key);
	str = ps;
	end_ps=strstr(ps,"\n");
	if((end_ps==NULL)||(end_ps>end))
	{	return -2;
	}
	i =  end_ps-ps;
	if(i>sizeof(buf))
	{	return -3;
	}
	memset(buf,0,sizeof(buf));
	memcpy(buf,ps,end_ps-ps);	
	cls_no_asc(buf,strlen(buf));
	//printf("buf %s\n",buf);
	memcpy(value,buf,strlen(buf));

	return 0;
}

static int get_file_info(char *ver)
{
	char buf[2048];
	//int i;
	char *p_file_head,*p_file_end;
	memset(buf,0,sizeof(buf));
	if(read_file("/etc/kerversion.cnf",buf,sizeof(buf))<0)
	{	
		return -1;
	}
	if(get_config_file_head_end(buf,"[usb-share kernel private version]","end.",&p_file_head,&p_file_end)<0)
	{	return -2;
	}
	//////////////////////////////////////////////////////////////////////////
	if(get_config_data(p_file_head,p_file_end,"private_ver=",ver)<0)
	{	return -3;
	}
	return 0;

}

static int get_private_version(char *ver,int hard_type)
{
	int result;
	//printf("先判断协议版本文件是否存在\n");
	//out("判断是否为A33版本");
	if ((hard_type == 1) || (hard_type == 3))
	{	//printf("a33 version\n");
		result = get_file_info(ver);
		if(result < 0)
		{	result = access("/sys/bus/usb/drivers/usbhid",0);
			if(result == 0)
			{	printf("/sys/bus/usb/drivers/usbhid exsit\n");
				printf("usbhid driver is exsit ,without update\n");
				return -1;
			}
			write_file("/etc/kerversion.cnf","[usb-share kernel private version]\nprivate_ver=v1.1\nend.\n",sizeof("[usb-share kernel private version]\nprivate_ver = v1.1\nend.\n"));	
			sync();
			result = get_file_info(ver);
			return result;
		}
		return result;
	}
	return -1;
}

int get_kernel_vision(char *ker,int *kver1,int hard_type)
{
    FILE *fp;
	char str[30] = {0};
	char kver[30] = { 0 };
	char ver[30] = { 0 };
	int result ;
    system ( "uname -r > /tmp/123" );
    fp = fopen ( "/tmp/123", "rb" );
    fgets ( str, 30, fp );
    fclose(fp);
    //printf ( "%s\n", str );
	//sprintf(kver,"%s",str);
	memset(ver, 0, sizeof(ver));
	memcpy(kver,str,strlen(str)-1);
	memcpy(ker, str, strlen(str) - 1);
	result = get_private_version(ver,hard_type);
	if(result >=0)
	{	//printf("ver %s\n\n\n\n\n\n\n\n\n",ver);
		//sprintf(ker,"%s-%s",kver,ver);
		//printf("kernel version %s\n",ker);
		*kver1=ver[3]-0x30;
		return 0;
	}
	//memcpy(ker,kver,30);
	printf("kernel version %s\n",ker);
    return 0;
}


