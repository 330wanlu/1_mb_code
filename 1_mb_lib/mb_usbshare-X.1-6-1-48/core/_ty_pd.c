#define _ty_pd_c
#include "_ty_pd.h"
int ty_pd_add(char *switch_name)
{   struct _ty_pd *stream;
	int result;
	stream=malloc(sizeof(struct _ty_pd));
	if(stream==NULL)
		return DF_ERR_MEM_ERR;
	memset(stream,0,sizeof(struct _ty_pd));   
    memcpy(stream->swith_name,switch_name,strlen(switch_name));
    //out("ty_pd_add 开关名称[%s]\n\n\n\n\n\n\n\n\n",stream->swith_name);
	result = ty_file_add(DF_DEV_TY_USB, stream, DF_TY_PD_NAME, (struct _file_fuc *)&ty_pd_fuc);
	if(result<0)
		free(stream);
	ty_pd_id = stream;
    //out("守护线程加载成功\n");
	return result;    
}
//
//int ty_pd_add_file(char *name)
//{   char pd_name[DF_FILE_NAME_MAX_LEN];
//    int fd,result;
//    memset(pd_name,0,sizeof(pd_name));
//    sprintf(pd_name,"/dev/%s",DF_TY_PD_NAME);
//    fd=ty_open(pd_name,0,0,NULL);
//    if(fd<0)
//        return fd;
//    result=ty_ctl(fd,DF_TY_PD_CM_ADD_NAME,name);
//    ty_close(fd);
//    return result;
//}
//
//int ty_pd_del_file(char *name)
//{   char pd_name[DF_FILE_NAME_MAX_LEN];
//    int fd,result;
//    memset(pd_name,0,sizeof(pd_name));
//    sprintf(pd_name,"/dev/%s",DF_TY_PD_NAME);
//    fd=ty_open(pd_name,0,0,NULL);
//    if(fd<0)
//        return fd;
//    result=ty_ctl(fd,DF_TY_PD_CM_DEL_NAME,name);
//    ty_close(fd);
//    return result;
//}
//


static int ty_pd_open(struct ty_file	*file)
{   struct _ty_pd  *stream;
    int j;
	stream=file->pro_data;
	if((stream==NULL))	
		return DF_ERR_PORT_CORE_FD_NULL;
    //out("打开ty_pd模块\n");
    if(stream->state==0)
    {   stream->switch_fd=ty_open(stream->swith_name,0,0,NULL);
        if(stream->switch_fd<0)
        {   out("看门狗模块打开错误\n");
            return stream->switch_fd;
        }
        stream->check_timer=ty_ctl(stream->switch_fd,DF_SWITCH_CM_GET_HEART_TIMER);
        if(stream->check_timer<0)
        {   stream->check_timer=40;   
        }
        //out("获取到心跳时间为:%d\n",stream->check_timer);
		stream->check_timer=20;//将原先40秒心跳改为20秒
        stream->check_timer=stream->check_timer*1000;
        stream->task=_task_open("pd",task_pd,stream,stream->check_timer,NULL); 
        stream->lock=_lock_open("pd",NULL);
    }
    for(j=0;j<sizeof(stream->fd)/sizeof(stream->fd[0]);j++)
	{	if(stream->fd[j].state==0)
			break;
	}
	if(j==sizeof(stream->fd)/sizeof(stream->fd[0]))
	{	return DF_ERR_PORT_CORE_SLAVE_SPACE;
	}
	memset(&stream->fd[j],0,sizeof(stream->fd[0]));
	stream->fd[j].dev=stream; 
	stream->fd[j].state=1; 
	stream->fd[j].mem_fd=&file->fd[j];
	stream->state++;	
    out("打开ty_pd_open成功:j=%d\n",j);
	return j+1;	 
}

static int ty_pd_ctl(void *data,int fd,int cm,va_list args)
{   struct _ty_pd  *stream;
    struct _ty_pd_fd   *id;
    int i;
    stream=data;
	if((stream==NULL))	
		return DF_ERR_PORT_CORE_FD_NULL;
	if((fd==0)||(fd>sizeof(stream->fd)/sizeof(stream->fd[0])))
		return DF_ERR_PORT_CORE_FD_Z;
	fd--;
	id=&stream->fd[fd];
	if(id->state==0)
		return DF_ERR_PORT_CORE_SLAVE_DEV;
    if(cm==DF_TY_PD_CM_RESET)
    {   out("发起重新启动命令\n");
        ty_ctl(stream->switch_fd,DF_SWITCH_CM_RESET_MA);   
        exit(0);
    }
    else if(cm==DF_TY_PC_CM_POWER_OFF)
    {   out("关闭电源\n");
        ty_ctl(stream->switch_fd,DF_SWITCH_CM_POWER_CLOSE);
        exit(0);
    }
    else if(cm==DF_TY_PD_CM_ADD_NAME)
    {   out("增加守护文件\n");
        char *name;
        name=va_arg(args,char *);
        _lock_set(stream->lock);
        for(i=0;i<sizeof(stream->file)/sizeof(stream->file[0]);i++)
        {   if(stream->file[i].state==0)
                break;
        }
        memcpy(stream->file[i].name,name,strlen(name));
        stream->file[i].state=1;
        _lock_un(stream->lock);
        return 0;
    }
    return DF_ERR_PORT_CORE_CM; 
}
static int ty_pd_read(void *data, int fd, uint8 *buf, int buf_len)
{
	struct _ty_pd	*stream;
	
    stream=data;
	//out("获取系统状态%d\n",stream->sys_state);
	return stream->sys_state; 
}
static int ty_pd_close(void *data,int fd)
{   struct _ty_pd  *stream;
    struct _ty_pd_fd   *id;
    //int j,i;
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
    {   out("要求全部关闭\n");
        _task_close(stream->task);
        memset(stream->file,0,sizeof(stream->file));
        ty_close(stream->switch_fd);
        _lock_close(stream->lock);
    }
    return 0;
}

//static int check_tcp_port(void)
//{
//	char tmp_file[10240] = {0};
//	char order[200];
//	sprintf(order, "netstat -l > /tmp/netstat.txt");
//	system(order);
//	read_file("/tmp/netstat.txt", tmp_file, sizeof(tmp_file));
//	system("rm /tmp/netstat.txt");
//	if ((StrFind(tmp_file, ":10001") == 0) && (StrFind(tmp_file, ":3240") == 0))
//	{
//		return 0;
//	}
//	return -1;
//}



int ty_pd_update_task_status(int task_num,unsigned long up_time)
{
	struct _ty_pd  *stream;
	stream = ty_pd_id;
	switch (task_num)
	{
	case DF_TASK_PORT_CHECK:
		stream->port_check_task_new = up_time;
		break;
	default:
		break;			//printf("不支持的发票类型\n");
	}
	return 0;
}
#ifndef MTK_OPENWRT	
static int check_port_status(struct _ty_pd  *par)
{
	//struct _usb_port_infor usb_t;
	int result;
	uint8 s_buf[1024] = { 0 };
	int i, usb_sum;
	memset(s_buf, 0, sizeof(s_buf));
	if (par->ty_usb_fd == 0)
	{
		par->ty_usb_fd = ty_open("/dev/ty_usb", 0, 0, NULL);
	}
	if (par->ty_usb_m_fd == 0)
	{
		par->ty_usb_m_fd = ty_open("/dev/ty_usb_m", 0, 0, NULL);
	}
	usb_sum = ty_ctl(par->ty_usb_fd, DF_TY_USB_CM_PORT_NUMBER);
	s_buf[0] = usb_sum;
	for (i = 1; i <= usb_sum; i++)
	{
		//out("查询终端%d号端口信息\n",i);
		//result = ty_ctl(par->ty_usb_m_fd, DF_TY_USB_M_CM_GET_USB_PORT_INFORMATION, i, &usb_t);

		struct _usb_port_infor port_info;
		memset(&port_info, 0, sizeof(struct _usb_port_infor));
		result = fun_get_plate_usb_info(i, &port_info);

		if (result<0)
		{   //out("获取USB端口信息失败result=%d\n",result);
			continue;
		}
		if (port_info.with_dev == 0)
		{
			continue;
		}
		if (port_info.port_status == 0)
		{
			continue;
		}
		else
		{//税盘使用中分情况
			if (port_info.port_used == 1 && port_info.system_used == 0)
				return -1;
			if ((strcmp(port_info.used_info, DF_PORT_USED_USB_SHARE) == 0) ||
				//(strcmp(usb_t.used_info, DF_PORT_USED_DQMCSHXX) == 0) ||
				(strcmp(port_info.used_info, DF_PORT_USED_FPKJ) == 0) ||
				(strcmp(port_info.used_info, DF_PORT_USED_FPZF) == 0))
				return -1;
			else
				continue;
		}
	}
	return 0;
}
#endif
static void task_pd(void *arg,int timer)
{   struct _ty_pd  *stream;
	
    stream=arg;
	
	ty_ctl(stream->switch_fd, DF_SWITCH_CM_HEART);


#ifndef MTK_OPENWRT	
	char mem_per[50];
	if (check_mem(mem_per) < 0)
	{
		stream->mem_err_counts += 1;
		if(stream->mem_err_counts >= 10)
		{
			logout(INFO, "SYSTEM", "守护线程", "检查到内存出现异常,当前已使用%s,需重启机柜\r\n", mem_per);
#ifdef RELEASE_SO
			so_common_report_event("001", "1", "检查到内存出现异常,需重启机柜", 0);
#else
			function_common_report_event("001", "1", "检查到内存出现异常,需重启机柜", 0);
#endif
			
			if (check_port_status(stream) == 0)//没有端口在被占用
			{
				sync();
				ty_ctl(stream->switch_fd, DF_SWITCH_CM_RESET_MA);
				sleep(10);
				return;
			}
		}

	}
#endif
	if ((stream->port_check_task_last == 0) && (stream->port_check_task_new == 0))
	{
		return ;
	}

	if (stream->port_check_task_last == stream->port_check_task_new)
	{
		logout(INFO, "SYSTEM", "守护线程", "检查到USB端口检测线程出现异常,再次尝试检查依然失败,需重启机柜\r\n");
		sync();
		ty_ctl(stream->switch_fd, DF_SWITCH_CM_RESET_MA);
		sleep(10);		
	}

	stream->port_check_task_last = stream->port_check_task_new;

   
}
