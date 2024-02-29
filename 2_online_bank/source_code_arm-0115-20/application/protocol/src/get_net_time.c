#include "../inc/get_net_time.h"

static int _get_net_time_open(struct ty_file	*file);
static int _get_net_time_ctl(void *data,int fd,int cm,va_list args);
static int _get_net_time_close(void *data,int fd);
static void _get_net_time_task(void *arg,int o_timer);

static const struct _file_fuc	get_net_time_fuc=
{
	.open=_get_net_time_open,
	.read=NULL,
	.write=NULL,
	.ctl=_get_net_time_ctl,
	.close=_get_net_time_close,
	.del=NULL
};

int _get_net_time_add(void)
{	struct _get_net_time  *stream;
	int result;
	//get_net_time_out("添加_get_net_time文件\n");
	stream=malloc(sizeof(struct _get_net_time));
	if(stream==NULL)
		return DF_ERR_MEM_ERR;
	memset(stream,0,sizeof(struct _get_net_time));
	result = ty_file_add(DF_DEV_GET_NET_TIME, stream, "get_net_time", (struct _file_fuc *)&get_net_time_fuc);
	if(result<0)
		free(stream);
	//get_net_time_out("添加socket client文件成功\n");
	return result;
}

static int _get_net_time_open(struct ty_file	*file)
{   struct _get_net_time  *stream;
	//int result;
    int j;
    //get_net_time_out("打开文件:get_net_time\n");
    stream=file->pro_data;
	if(stream==NULL)
		return DF_ERR_PORT_CORE_FD_NULL;        
	if(stream->state==0)
    {  // get_net_time_out("申请一个数据锁\n");
        stream->lock=_lock_open("_get_net_time.c",NULL);
        if(stream->lock<0)
        {   get_net_time_out("申请锁失败\n");
            return stream->lock;
        }
		stream->task=_scheduled_task_open("_get_net_time",_get_net_time_task,stream,DF_GET_NET_TIME_TASK_TIMER,NULL);
		if(stream->task<0)
		{
			get_net_time_out("开启查询网络状态线程失败\n");
			return stream->task;
		}
    }  
    for(j=0;j<sizeof(stream->fd)/sizeof(stream->fd[0]);j++)
	{	if(stream->fd[j].state==0)
			break;
	}
	if(j==sizeof(stream->fd)/sizeof(stream->fd[0]))
	{	return DF_ERR_PORT_CORE_SLAVE_SPACE;
	}
	memset(&stream->fd[j],0,sizeof(stream->fd[0]));
	stream->fd[j].dev = (struct _register  *)stream;
	stream->fd[j].state=1; 
	stream->fd[j].mem_fd=&file->fd[j];
	stream->state++;	
    return j+1;
}


static int _get_net_time_ctl(void *data,int fd,int cm,va_list args)
{  
	struct _get_net_time *stream;
	struct _get_net_time_fd *id;
	//int i;
	//uint8 *outbuf;
	//uint8 returndata[1024];
	//int returndata_len;
	//int result;
	//returndata=(uint8*)malloc(sizeof(uint8*)*8096);  
    //memset(returndata, 0, sizeof(returndata));  
    stream=data;
   // get_net_time_out("_get_net_time_ctl\n");
	if((stream==NULL))	
		return DF_ERR_PORT_CORE_FD_NULL;
	if((fd==0)||(fd>sizeof(stream->fd)/sizeof(stream->fd[0])))
		return DF_ERR_PORT_CORE_FD_Z;
	fd--;
	id=&stream->fd[fd];
	if(id->state==0)
		return DF_ERR_PORT_CORE_SLAVE_DEV;
    if(cm==DF_TY_GET_NET_TIME)
    {   
		//_lock_set(stream->lock);
		//get_net_time_out("cm==DF_TY_GET_NET_TIME\n");
		////outbuf=va_arg(args,uint8 *);

		//result = netGet("http://www.njmbxx.com:25867/TimeGet/",NULL,5000,(char *)returndata,&returndata_len);
		//if(returndata == NULL)
		//{
		//	get_net_time_out("netGet error\n");
		//	_lock_un(stream->lock); 
		//	return -1;
		//}
		////get_net_time_out("获取到的数据长度：%d\n",returndata_len);
		////get_net_time_out("获取到的数据：%s\n",returndata);		
		//result = _get_time_set_sys_time(returndata);
		//if(result < 0)
		//{
		//	get_net_time_out("设置网络对时失败\n");
		//	_lock_un(stream->lock); 
		//	return -2;
		//}
		//_lock_un(stream->lock); 
		return 0;
           
    }	
    return DF_ERR_PORT_CORE_CM; 
}



static int _get_net_time_close(void *data,int fd)
{   struct _get_net_time 	*stream;
    struct _get_net_time_fd 	*id;
	stream=data;
	if((stream==NULL))	
		return DF_ERR_PORT_CORE_FD_NULL;
	if((fd==0)||(fd>sizeof(stream->fd)/sizeof(stream->fd[0])))
		return DF_ERR_PORT_CORE_FD_Z;
	fd--;
	id=&stream->fd[fd];
	if(id->state==0)
		return DF_ERR_PORT_CORE_SLAVE_DEV;
	id->state=0;
	stream->state--;
    memset(id,0,sizeof(stream->fd[0]));
    if(stream->state==0)
    {   
		get_net_time_out("关闭本模块\n");
        _lock_close(stream->lock);
        get_net_time_out("关闭成功\n");
    }    
    return 0;
}

//static int get_file_head_end(const char *buf,const char *eth,char **p_head,char **p_end)
//{	char lan[100];
//	memset(lan,0,sizeof(lan));
//	//sprintf(lan,"#!/bin/sh");
//	sprintf(lan,eth);
//	*p_head=strstr(buf,lan);
//	if(*p_head==NULL)
//	{	get_net_time_out("[%s]未找到\n",lan);
//		return -1;
//	}
//	*p_end=strstr(*p_head,"</Data>");
//	if(*p_end==NULL)
//	{	get_net_time_out("未找到数据结束\n");
//		return -2;
//	}
//	//*p_head+=strlen(lan);
//	*p_end+=2;
//	return 0;
//}

//static int _get_time_set_sys_time(uint8 *timebuf)
//{	char *p_file_head,*p_file_end;
//	//int result;
//	uint8 settimebuf[19];
//	//struct _get_time get_time;
//	memset(settimebuf,0,sizeof(settimebuf));
//	if (get_file_head_end((const char *)timebuf, "<Data>", &p_file_head, &p_file_end)<0)
//	{	get_net_time_out("ip文件内容错误\n");
//	   	return -2;
//	}
//	_get_http_buf_time(p_file_head,p_file_end,settimebuf);
//	/*memcpy(settimebuf,get_time.year,4);
//	sprintf(settimebuf+4,"-");
//	memcpy(settimebuf+5,get_time.month,2);
//	sprintf(settimebuf+7,"-");
//	memcpy(settimebuf+8,get_time.date,2);
//	sprintf(settimebuf+10," ");
//	memcpy(settimebuf+11,get_time.hour,2);
//	sprintf(settimebuf+13,":");
//	memcpy(settimebuf+14,get_time.minute,2);
//	sprintf(settimebuf+16,":");
//	memcpy(settimebuf+17,get_time.second,2);*/
//	//get_net_time_out("要设置的时间为：%s\n",settimebuf);
//	if(settimebuf[0]!=0x32||settimebuf[1]!=0x30)
//	{
//		printf("要设置的时间数据错误\n");
//		return -1;
//	}
//
//	//get_net_time_out("将时间改为时间戳\n");
//	long time_sec = get_second_from_date_time_turn((char *)settimebuf);
//	time_sec = time_sec - 8 * 3600;
//	char settime[20] = { 0 };
//	get_date_time_from_second_turn(time_sec, settime);
//
//
//	char cmd[200] = {0};
//	sprintf(cmd, "date -s \"%s\" -u", settime);
//	system(cmd);
//	//system("hwclock -w");
//	//result = SetSystemTime((uint8 *)settime);
//	//if(result < 0)
//	//{
//	//	get_net_time_out("设置时间是失败\n");
//	//	return result;
//	//}
//	//system("TZ=UTC-08:00");
//	//system("export TZ");
//	//system("hwclock - s");
//	//get_net_time_out("网络对时完成\n");
//	//get_net_time_out("当前UTC时间为：%s\n\n", settime);
//	return 0;
//
//}

//static int _get_time_set_sys_time_backup(uint8 *timebuf)
//{
//	int result = 0;
//	char ca_Time[100] = { 0 }, ca_Flag[1000] = { 0xE5, 0x8C, 0x97, 0xE4, 0xBA, 0xAC, 0xE7, 0x8E, 0xB0, 0xE5, 0x9C, 0xA8, 0xE6, 0x97, 0xB6, 0xE9, 0x97, 0xB4, 0xEF, 0xBC, 0x9A };
//	char *pc_Time = strstr((const char*)timebuf, ca_Flag);
//	if (pc_Time == NULL)
//		return -2;
//	pc_Time += strlen(ca_Flag);
//	memcpy(ca_Time, pc_Time, 19);
//	if (strlen(ca_Time) != 19)
//		return -1;
//
//	get_net_time_out("将时间改为时间戳\n");
//	long time_sec = get_second_from_date_time_turn(ca_Time);
//	time_sec = time_sec - 8 * 3600;
//	char settime[20] = { 0 };
//	get_date_time_from_second_turn(time_sec, settime);
//
//	result = SetSystemTime((uint8 *)settime);
//	if (result < 0)
//	{
//		get_net_time_out("设置时间是失败\n");
//		return result;
//	}
//
//	char cmd[200] = { 0 };
//	sprintf(cmd, "date -s \"%s\"", settime);
//	system(cmd);
//	system("hwclock -w");
//	get_net_time_out("网络对时完成\n");
//	get_net_time_out("当前UTC时间为：%s\n\n", settime);
//	return 0;
//}



//static int _get_http_buf_time(char *str,char *end,uint8 *t_time)
//{	char *ps,*end_ps;	
//	//char buf[30];
//	//char mask[2];
//	int i;
//	/////////////////////年///////////////////////////
//	ps=strstr(str,"<Time>");
//	if((ps==NULL)||(ps>end))
//	{	get_net_time_out("without:[%s]\n","<Time>");
//		return -1;
//	}
//	//get_net_time_out("find %s\n","nyear=");
//	ps+=strlen("<Time>");
//	str = ps;
//	end_ps=strstr(str,"</Time>");
//	if((end_ps==NULL)||(end_ps>end))
//	{	get_net_time_out("with get_net_time_out: </Time>\n");
//		return -2;
//	}
//	i=end_ps-ps;
//	//get_net_time_out("i = %d\n",i);
//	if(i<19)
//	{
//		get_net_time_out("不符合时间长度规范\n");
//		return -3;
//	}
//	memcpy(t_time,ps,i);
//	//get_net_time_out("second=%s\n",t_time->second);
//	//get_net_time_out("找到秒\n");
//	return 0;
//}

static void _get_net_time_task(void *arg,int o_timer)
{
	//get_net_time_out("进入网络对时检查线程\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");
	sleep(15);
#ifdef MTK_OPENWRT
	if (access("/sbin/ntpdate", 0) == 0)
#else	
	if (access("/usr/sbin/ntpdate", 0) == 0)
#endif
	{
		get_net_time_out("get_net_time_task  ntpdate -u ntp2.aliyun.com\n\n\n\n\n");
		system("ntpdate -u ntp2.aliyun.com");
		get_net_time_out("get_net_time_task  \n\n\n\n\n");
		goto END;
	}
END:
	sleep(3600);
	return;
}