#define _switch_dev_c
#include "_switch_dev.h"
int switch_dev_add(int hard_type,char *ter_type)
{   struct _switch_dev *stream;
	int result;
	stream=malloc(sizeof(struct _switch_dev));
	if(stream==NULL)
		return DF_ERR_MEM_ERR;
	memset(stream,0,sizeof(struct _switch_dev));
    //out("增加\"/dev/switch\"文件至文件系统\n");
	result = ty_file_add(DF_DEV_TY_SWITCH_DEV, stream, "switch", (struct _file_fuc *)&switch_fuc);
	if(result<0)
		free(stream);
	stream->hardtype = hard_type;
	memcpy(stream->ter_type, ter_type, strlen(ter_type));
	return result;
}

static int switch_dev_open(struct ty_file	*file)
{   struct _switch_dev	*stream;
	int result,j;
	stream=file->pro_data;
	if((stream==NULL))	
		return DF_ERR_PORT_CORE_FD_NULL;
    //out("打开switch_dev_open模块\n");
    if(stream->state==0)
    {   //out("首次使用switch_dev_open\n");
        stream->seria=ty_open(DF_SERIA_DIR,0,0,NULL);
        if(stream->seria<0)
        {   out("打开串口[%s]错误\n",DF_SERIA_DIR);
            return DF_ERR_PORT_CORE_SWITCH_SERIA;
        }
        //out("设置串口波特率[%d]\n",DF_SERIA_BPS);
        ty_ctl(stream->seria,DF_SERIA_CM_BPS,DF_SERIA_BPS);
        stream->lock=_lock_open("switch_dev.c",NULL);
        if(stream->lock<0)
        {   out("申请本页锁失败\n",stream->lock);
            return stream->lock;
        }

        out("发送上电命令给设备\n");
        result=dev_power_up(stream);
        if(result<0)
        {   
			logout(INFO, "system", "init", "与单片机设备联机失败\n");
			sleep(2);
			result = dev_power_up(stream);
			if (result < 0)
			{
				logout(INFO, "system", "init", "与单片机设备第二次联机失败\n");
				ty_close(stream->seria);
				_lock_close(stream->lock);
				return result;
			}
        }
		sleep(1);//联机之后延时1秒
		dev_get_hard_version(stream);

		//out("与单片机通讯成功,指示正在找网络\n");
		oper_stm32_dev(stream,DF_PC_ORDER_NO_NET,NULL,NULL,0,NULL);


		char baseudid[50] = { 0 };
		//out("初始化风扇状态\n");
		uint8 s_buf[2000] = { 0 };
		int8 r_order = 0;
		memset(s_buf, 0, sizeof(s_buf));
		result = oper_stm32_dev(stream, DF_PC_ORDER_CTL_FAN, s_buf, NULL, 1, &r_order);
		if (result< 0 || r_order < 0)
		{
			out("操作风扇失败,默认认为机柜不支持风扇控制\n");
			stream->dev_support.support_fan = 0;
		}
		else
		{
			stream->dev_support.support_fan = 1;
		}

		memset(s_buf, 0, sizeof(s_buf));
		result = oper_stm32_dev(stream, DF_PC_ORDER_BLUETOOTH_POWER, s_buf, NULL, 1, &r_order);
		if (result< 0 || r_order < 0)
		{
			out("操作蓝牙失败,默认认为机柜不支持蓝牙控制\n");
			stream->dev_support.support_ble = 0;
		}
		else
		{
			stream->dev_support.support_ble = 1;
		}

	


		
		memset(s_buf, 0, sizeof(s_buf));
		result = oper_stm32_dev(stream, DF_PC_ORDER_CTL_GET_ALL_PORT_STATUS, NULL, s_buf, 0, &r_order);
		if (result< 0 || r_order < 0)
		{
			out("操作端口节电功能失败,默认认为机柜不支持端口节电功能\n");
			stream->dev_support.support_power = 0;
		}
		else
		{
			stream->dev_support.support_power = 1;
		}



		
		memset(s_buf, 0, sizeof(s_buf));
		result = oper_stm32_dev(stream, DF_PC_ORDER_GET_SERIAL_NUM, NULL, s_buf, 0, &r_order);
		if (result< 0 || r_order < 0)
			return result;
		encode((char *)s_buf, 12, (char *)baseudid);
		out("获取后:[%s]\n", baseudid);

		strcpy(stream->cpu_id, baseudid);



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
    //out("打开switch成功:j=%d\n",j);
	return j+1;	   
}

static int switch_dev_ctl(void *data,int fd,int cm,va_list args)
{   struct _switch_dev	*stream;
    struct _switch_dev_fd   *id;
    int i;
    stream=data;
	//out("进入switch_dev_ctl函数\n");
	if((stream==NULL))	
		return DF_ERR_PORT_CORE_FD_NULL;
	if((fd==0)||(fd>sizeof(stream->fd)/sizeof(stream->fd[0])))
		return DF_ERR_PORT_CORE_FD_Z;
	fd--;
	id=&stream->fd[fd];
	if(id->state==0)
		return DF_ERR_PORT_CORE_SLAVE_DEV;
    //out("执行[switch_dev_ctl]\n");
    for(i=0;i<sizeof(ctl_fun)/sizeof(ctl_fun[0]);i++)
    {   if(cm==ctl_fun[i].cm)
            return ctl_fun[i].ctl(id,args);
    }
    //out("没有找到对应的命令\n");
    return DF_ERR_PORT_CORE_CM; 
}

static int switch_dev_close(void *data,int fd)
{   struct _switch_dev	*stream;
    struct _switch_dev_fd   *id;
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
    if(stream->state==0)			//全部关闭了
	{   out("对运行设备下电\n");
        dev_power_down(stream);    
        ty_close(stream->seria);
        _lock_close(stream->lock);
	}
    return 0;  
}
static int dev_conn(struct _switch_dev_fd   *id, va_list args)
{
	//out("给设备开机\n");
	return oper_stm32_dev(id->dev, DF_PC_ORDER_CONNECT, NULL, NULL, 0, NULL);
}
#ifndef MTK_OPENWRT
static int dev_port_get_all_port_status(struct _switch_dev_fd   *id, va_list args)
{
	
	uint8 *all_info;
	int result;
	int8 r_order = 0;
	all_info = va_arg(args, uint8 *);

	//wang 
	#if ((!defined DF_2212_1) && (!defined DF_2212_8) && (!defined DF_2216_1) && (!defined DF_2216_8))
	if (id->dev->dev_support.support_power != 1)
	{
		return -1;
	}
	#endif

	result = oper_stm32_dev(id->dev, DF_PC_ORDER_CTL_GET_ALL_PORT_STATUS, NULL, all_info, 0, &r_order);
//	out("获取下电端口状态完成---result:%d\n", result);
	if (r_order<0)
		return result;
	if (result < 0)
		return result;
	return result;
}

static int dev_get_ad_status(struct _switch_dev_fd   *id, va_list args)
{
	int result;
	uint8 *ad_status;
	int8 r_order = 0;
	ad_status = va_arg(args, uint8*);


	//wang 
	#if ((!defined DF_2212_1) && (!defined DF_2212_8) && (!defined DF_2216_1) && (!defined DF_2216_8))
	if (id->dev->dev_support.support_power != 1)
	{
		return -1;
	}
	#endif

	//out("获取ad\n");
	result = oper_stm32_dev(id->dev, DF_PC_ORDER_GET_AD_STATUS, NULL, ad_status, 0, &r_order);
	if (r_order<0)
		return result;
	if (result<0)
		return result;
	return result;
}

static int dev_control_fan(struct _switch_dev_fd   *id, va_list args)
{
	int result;
	uint8 *indata;
	uint8 buf[14];
	indata = va_arg(args, uint8 *);
	if (id->dev->dev_support.support_fan != 1)
	{
		return -1;
	}
	buf[0] = indata[0];
	result = oper_stm32_dev(id->dev, DF_PC_ORDER_CTL_FAN, buf, NULL, 1, NULL);
	if (result<0)
		return result;
	return 0;
}


static int dev_bluetooth_mode_change(struct _switch_dev_fd   *id, va_list args)
{
	int result;
	uint8 *change;
	uint8 buf[14] = { 0 };
	change = va_arg(args, uint8 *);

	if (id->dev->dev_support.support_ble != 1)
	{
		return -1;
	}
	buf[0] = change[0];
	result = oper_stm32_dev(id->dev, DF_PC_ORDER_CTL_BLUETOOTH, buf, NULL, 1, NULL);
	if (result<0)
		return result;
	return 0;
}

static int dev_bluetooth_power(struct _switch_dev_fd   *id, va_list args)
{
	int result;
	uint8 *change;
	uint8 buf[14] = { 0 };
	change = va_arg(args, uint8 *);

	if (id->dev->dev_support.support_ble != 1)
	{
		return -1;
	}

	buf[0] = change[0];
	result = oper_stm32_dev(id->dev, DF_PC_ORDER_BLUETOOTH_POWER, buf, NULL, 1, NULL);
	if (result<0)
		return result;
	return 0;
}

static int dev_restart_mac_ic(struct _switch_dev_fd   *id, va_list args)
{
	int result;
	result = mac_ic_restart(id->dev);
	return result;
}
static int dev_restore(struct _switch_dev_fd   *id, va_list args)
{
	oper_stm32_dev(id->dev, DF_PC_ORDER_RESTORE, NULL, NULL, 0, NULL);
	return 0;
}
static int dev_oper_double_light_yellow(struct _switch_dev_fd   *id, va_list args)
{
	int port;
	int result;
	uint8 buf[20];
	port = va_arg(args, int);
	//out("指示端口%d没有设备\n",port);
	buf[0] = (port / 256); buf[1] = (port & 0xff);
	buf[2] = 0x06;//点亮1 2号灯
	buf[3] = 0x00;//熄灭时间100ms单位
	buf[4] = 0x00;//点亮时间100ms单位
	result = oper_stm32_dev(id->dev, DF_PC_ORDER_SET_PORT_DOUBLE_LIGHT, buf, NULL, 5, NULL);
	return result;

}
#endif
static int dev_get_hard_version(struct _switch_dev	*stream)
{
	uint8 r_buf[200];
	int result;
	int8 r_oder;
	struct _stm32_infor infor;
	memset(r_buf, 0, sizeof(r_buf));
	//out("获取唯一序列号\n");
	result = oper_stm32_dev(stream, DF_PC_ORDER_GET_EDIT_INFOR, NULL, r_buf, 0, &r_oder);
	if (result<0)
		return result;
	if (r_oder<0)
		return result;


	out("获取后:[%s]\n", r_buf);
	memset(&infor, 0, sizeof(struct _stm32_infor));
	memcpy(&infor.r_date, r_buf, 10);
	memcpy(&infor.soft, r_buf + 10, 8);
	memcpy(&infor.hard, r_buf + 18, 8);

	
	out("发布日期：%s\n", infor.r_date);
	out("发布软件版本：%s\n", infor.soft);
	out("发布硬件版本：%s\n", infor.hard);

	if (strcmp((const char *)infor.soft, "PM9.01") >= 0)
	{
		out("单片机版本支持LCD屏显\n");
		stream->dev_support.support_lcd = 1;
	}
	else if(
		(strcmp(stream->ter_type, "MB_1905") == 0) ||
		(strcmp(stream->ter_type, "MB_1906") == 0) ||
		(strcmp(stream->ter_type, "MB_1910") == 0) ||
		(strcmp(stream->ter_type, "MB_1908") == 0) ||
		(strcmp(stream->ter_type, "MB_2106") == 0) ||
		(strcmp(stream->ter_type, "MB_2108") == 0) || 
		(strcmp(stream->ter_type, "MB_2110") == 0) ||
		(strcmp(stream->ter_type, "MB_2202") == 0))
	{
		stream->dev_support.support_lcd = 1;
	}

	return 0;
}

/*------------------------------------命令操作-----------------------------------------------*/



static int dev_open_port(struct _switch_dev_fd   *id,va_list args)
{   uint8 *dev_id;
    uint8 buf[14];
    int port;     
    int8 order;
    int result;
    //out("打开端口命令\n");
    dev_id=va_arg(args,uint8 *);
    port=va_arg(args,int );    
    memset(buf,0,sizeof(buf));
    bcd2asc(dev_id,buf,6);
    buf[12]=(port/256);buf[13]=(port&0xff);
    //out("从id=[%s]的[%d]打开\n",buf,port);
	//out("通知单片机打开%d端口\n", port);
	//long long start = get_time_msec();
    result=oper_stm32_dev(id->dev,DF_PC_ORDER_USB_MA_OPEN,buf,NULL,sizeof(buf),&order); 
	//out("打开端口命令完成\n");
    if(result<0)
        return result;
    if(order<0)
        return DF_ERR_PORT_CORE_TY_USB_STM32_OPEN;
	//long long end = get_time_msec();
	//long long diff = (long long)end - (long long)start;
	//out("单片机应答打开%d端口成功,耗时：%lldms\n", port, diff);
    return result;   
}

static int dev_close_port(struct _switch_dev_fd   *id,va_list args)         
{   uint8 *dev_id;
    uint8 buf[14];
    int port;     
    int8 order;
    int result;
   // out("关闭端口命令\n");
    dev_id=va_arg(args,uint8 *);
    port=va_arg(args,int );    
    memset(buf,0,sizeof(buf));
    bcd2asc(dev_id,buf,6);
    buf[12]=(port/256);buf[13]=(port&0xff);
   // out("从id=[%s]的[%d]关闭\n",buf,port);
	//out("通知单片机关闭%d端口\n",port);
	//long long start = get_time_msec();
    result=oper_stm32_dev(id->dev,DF_PC_ORDER_USB_MA_CLOSE,buf,NULL,sizeof(buf),&order);  
	//out("关闭端口命令完成\n");
	if (result < 0)
	{
		out("单片机应答关闭%d端口失败\n", port);
		return result;
	}
	if (order < 0)
	{
		out("单片机应答关闭%d端口命令字错误,命令字：%02x\n", port,order);
		return DF_ERR_PORT_CORE_TY_USB_STM32_CLOSE;
	}
	//long long end = get_time_msec();
	//long long diff = (long long)end - (long long)start;
	//out("单片机应答关闭%d端口成功,耗时：%lldms\n", port, diff);
    return result; 
}

static int dev_get_only_serial_number(struct _switch_dev_fd   *id,va_list args)
{
	struct _switch_dev_only  *udid;
    uint8 r_buf[12];
    int result;
    int8 r_oder;
    
    memset(r_buf,0,sizeof(r_buf));
	//out("获取唯一序列号\n");
    result=oper_stm32_dev(id->dev,DF_PC_ORDER_GET_SERIAL_NUM,NULL,r_buf,0,&r_oder);
    if(result<0)
        return result;
    if(r_oder<0)
        return DF_ERR_PORT_CORE_TY_USB_STM32_DEV;
    out("获取后:[%s]\n",r_buf);
    udid=va_arg(args,struct _switch_dev_only  *); 
    if(udid!=NULL)
    {   memcpy(udid->serial_num,r_buf,12);
		//memcpy(udid->serial_num,"123AZaz+/axr",12);
		out("获取唯一序列号成功\n");
    }    
    return 0; 

}

static int dev_set_id(struct _switch_dev_fd   *id,va_list args)
{   uint8 buf[100];
    struct _switch_dev_id  *mac_id;
    out("设置设备机器编号\n");
    mac_id=va_arg(args,struct _switch_dev_id  *); 
    memset(buf,0,sizeof(buf));
    bcd2asc(mac_id->id,buf,sizeof(mac_id->id));
    time_asc(mac_id->date,buf+12,2);
    out("设置的信息:[%s]\n",buf);
	return oper_stm32_dev(id->dev, DF_PC_ORDER_SET_DEV_ID, buf, NULL, strlen((const char *)buf), NULL);
}

static int dev_get_id(struct _switch_dev_fd   *id,va_list args)
{   struct _switch_dev_id  *mac_id;
    uint8 r_buf[100];
    int result;
    int8 r_oder;
    //out("获取设备机器编号\n");
    memset(r_buf,0,sizeof(r_buf));
    result=oper_stm32_dev(id->dev,DF_PC_ORDER_READ_DEV_ID,NULL,r_buf,0,&r_oder);
    if(result<0)
	{	out("oper_stm32_dev result = %d\n",result);
        return result;
	}
    if(r_oder<0)
	{	out("oper_stm32_dev r_oder = %d\n",r_oder);
        return DF_ERR_PORT_CORE_TY_USB_STM32_DEV;
	}
    //out("获取后:[%s]\n",r_buf);
    mac_id=va_arg(args,struct _switch_dev_id  *); 
    if(mac_id!=NULL)
    {   asc2bcd(r_buf,mac_id->id,12);
        asc2bcd(r_buf+12,mac_id->date,4);
        asc2bcd(r_buf+17,mac_id->date+2,2);
        asc2bcd(r_buf+20,mac_id->date+3,2);
    }   
	bcd2asc(mac_id->id, (uint8 *)id->dev->ter_id, 6);
    return result;   
}

static int dev_get_cpuid(struct _switch_dev_fd   *id, va_list args)
{
	struct _switch_dev	*stream;
	char *cpu_id;
	stream = id->dev;

	cpu_id = va_arg(args, char *);

	strcpy(cpu_id,stream->cpu_id);
	
	return 0;
}

static int dev_get_support(struct _switch_dev_fd   *id, va_list args)
{
	struct _switch_dev	*stream;
	stream = id->dev;
	struct _dev_support * dev_sup;
	dev_sup = va_arg(args, struct _dev_support *);

	memcpy(dev_sup, &stream->dev_support, sizeof(struct _dev_support));

	return 0;
}


static int dev_set_pcb_id(struct _switch_dev_fd   *id,va_list args)
{   uint8 buf[100];
    struct _switch_dev_pcb_id   *pcb_id;
    out("设置单板id\n");
    pcb_id=va_arg(args,struct _switch_dev_pcb_id  *);
    memset(buf,0,sizeof(buf));
    bcd2asc(pcb_id->id,buf,sizeof(pcb_id->id));
    time_asc(pcb_id->date,buf+12,2);
    sprintf((char *)buf+strlen((const char *)buf),"%d",pcb_id->n);
    out("设置信息[%s]\n",buf);
    return oper_stm32_dev(id->dev,DF_PC_ORDER_SET_SUB_ID,buf,NULL,strlen((const char *)buf),NULL);   
}

static int dev_get_pcb_id(struct _switch_dev_fd   *id,va_list args)
{   struct _switch_dev_pcb_id  *pcb_id;
    uint8 r_buf[100];
    int result;
    int8 r_oder;
    out("获取单板id\n");
    memset(r_buf,0,sizeof(r_buf));
    result=oper_stm32_dev(id->dev,DF_PC_ORDER_READ_SUB_ID,NULL,r_buf,0,&r_oder);
    if(result<0)
        return result;
    if(r_oder<0)
        return DF_ERR_PORT_CORE_TY_USB_STM32_DEV;
    out("获取后:[%s]\n",r_buf);
    pcb_id=va_arg(args,struct _switch_dev_pcb_id  *);
    if(pcb_id!=NULL)
    {   asc2bcd(r_buf,pcb_id->id,12);
        asc2bcd(r_buf+12,pcb_id->date,4);
        asc2bcd(r_buf+17,pcb_id->date+2,2);
        asc2bcd(r_buf+20,pcb_id->date+3,2);
        sscanf((char *)r_buf+22, "%d", &pcb_id->n);
    }
    return result; 
}

static int dev_get_class(struct _switch_dev_fd   *id,va_list args) 
{   int result,i,n,cm;
    int8 order;
    uint8 buf[1024];
    struct _switch_dev_class    dev_class[10];
    //out("获取机柜群的属性\n");
    memset(buf,0,sizeof(buf));
    result=oper_stm32_dev(id->dev,DF_PC_ORDER_GROUNP_PRO,NULL,buf,0,&order);
    if(result<0)
        return result;
    if(order<0)
        return DF_ERR_PORT_CORE_TY_USB_STM32_DEV;
    memset(dev_class,0,sizeof(dev_class));
    n=result/16;
    for(i=0;((i<n)&&(i<sizeof(dev_class)/sizeof(dev_class[0])));i++)
    {   dev_class[i].n=buf[i*16]*256+buf[i*16+1];
        asc2bcd(buf+i*16+2,dev_class[i].id,12);
    }
    cm=va_arg(args,int);
    if(cm==DF_SWITCH_CM_CLASS_ID)
    {   uint8 *id;
        int id_n;
        out("只获取ID\n");
        id=va_arg(args,uint8 *);
        id_n=va_arg(args,int);
        if(id==NULL)
        {   if(i>id_n)
                i=id_n;
            return i;
        }
        for(n=0;(n<i)&&(n<id_n);n++)
            memcpy(id+n*sizeof(dev_class[0].id),dev_class[n].id,sizeof(dev_class[n].id));
        return n;
    }
    else if(cm==DF_SWITCH_CM_CLASS_ALL)
    {   struct _switch_dev_class    *app_class;
        int c_n;
        //out("获取所有的class\n");
        app_class=va_arg(args,struct _switch_dev_class    *);
        c_n=va_arg(args,int);
        if(app_class==NULL)
        {   if(i>c_n)
                i=c_n;
            return i;
        }
        for(n=0;(n<i)&&(n<c_n);n++)
            memcpy(&app_class[n],&dev_class[n],sizeof(dev_class[0]));
        return n;
    }
    return -1;
}


static int dev_get_pcb(struct _switch_dev_fd   *id,va_list args)    
{   int result,i,n,pcb_n;
    uint8 buf[1024];
    uint8 id_asc[12];
    uint8 *dev_id;
    struct _switch_dev_pcb  *pcb;
    int8 order;
    out("获取指定机柜的单板信息\n");
    dev_id=va_arg(args,uint8 *);
    memset(buf,0,sizeof(buf));
    bcd2asc(dev_id,id_asc,6);
    result=oper_stm32_dev(id->dev,DF_PC_ORDER_SUB_PRO,id_asc,buf,sizeof(id_asc),&order);
    if(result<0)
        return result;  
    if(order<0)
        return DF_ERR_PORT_CORE_TY_USB_STM32_PCB;
    pcb=va_arg(args,struct _switch_dev_pcb    *);
    pcb_n=va_arg(args,int);
    n=result/16;
    if(pcb==NULL)
    {   return n;
    }
    out("获取指定机柜信息成功:n=%d\n",n);
    for(i=0;((i<n)&&(i<pcb_n));i++)
    {   asc2bcd(buf+i*16+2,pcb[i].id,12);
        pcb[i].n=buf[i*16+14]*256+buf[i*16+15];
        out("pcb[i].n=%d\n",pcb[i].n);
    }
    out("成功返回\n");
    return i;
}


static int dev_port_state(struct _switch_dev_fd   *id,va_list args)  
{   int result,i,n,p_n;
    uint8 buf[1024];
    uint8 id_asc[12];
    int8 order;
    uint8 *dev_id;
    int *port;
    out("按指定机器编号获取端口状态\n");
    dev_id=va_arg(args,uint8 *);
    bcd2asc(dev_id,id_asc,6);
    memset(buf,0,sizeof(buf));
    result=oper_stm32_dev(id->dev,DF_PC_ORDER_USB_GET_OPEN_MA,id_asc,buf,sizeof(id_asc),&order);
    if(result<0)
        return result;
    if(order<0)
        return 0;
    port=va_arg(args,int *);
    p_n=va_arg(args,int);
    n=buf[12]*256+buf[13];
    if(port==NULL)
        return n;
    for(i=0;(i<n)&&(i<p_n);i++)
    {   port[i]=buf[14+i*2]+buf[15+i*2];
    }
    return i;
}

static int dev_reset_ma(struct _switch_dev_fd   *id,va_list args)
{   out("重新启动\n");
    //log_file_close();
	out("使用reoot重启命令\n");
#ifndef PRODUCE_PROGRAM
	_mqtt_client_pub("mb_equ_ter_will", id->dev->ter_id,DF_MQ_QOS0);//生产环境未发货无法连接MQTT
#endif
	system("sync");
	#ifdef MTK_OPENWRT
	sleep(1);
    oper_stm32_dev(id->dev,DF_PC_ORDER_RESET,NULL,NULL,0,NULL);
	#else
	//system("reboot");
	oper_stm32_dev(id->dev, DF_PC_ORDER_RESET, NULL, NULL, 0, NULL);
	#endif

	sleep(8);

	system("reboot");
    //oper_stm32_dev(id->dev,DF_PC_ORDER_RESET,NULL,NULL,NULL,NULL);
    exit(0);
    return 0;
}

static int dev_write_flash(struct _switch_dev_fd   *id,va_list args)
{   uint8 *buf;
    int buf_len;
    out("写flash\n");
    buf=va_arg(args,uint8 *);
    buf_len=va_arg(args,int);
    return oper_stm32_dev(id->dev,DF_PC_ORDER_WRITE_FLASH,buf,NULL,buf_len,NULL);
}

static int dev_read_flash(struct _switch_dev_fd   *id,va_list args)
{   uint8 *buf;
    int buf_len;
    uint8 s_buf[2];
    out("读flash命令\n");
    buf=va_arg(args,uint8 *);
    buf_len=va_arg(args,int);
    if(buf==NULL)
        return -1;
    s_buf[0]=((buf_len>>8)&0xff);s_buf[1]=((buf_len)&0xff);
    return oper_stm32_dev(id->dev,DF_PC_ORDER_READ_FLASH,s_buf,buf,sizeof(s_buf),NULL);
}

static int dev_get_infor(struct _switch_dev_fd   *id,va_list args)
{   struct _stm32_infor *infor;
    uint8 r_buf[100];
    int result;
    int8 r_oder;
    //out("获取单片机信息\n");
    infor=va_arg(args,struct _stm32_infor *);
    if(infor==NULL)
        return -1;
    memset(r_buf,0,sizeof(r_buf));
    result=oper_stm32_dev(id->dev,DF_PC_ORDER_GET_EDIT_INFOR,NULL,r_buf,0,&r_oder);
    if(result<0)
        return result;
    if(r_oder<0)
        return DF_ERR_PORT_CORE_TY_USB_STM32_DEV;
    memcpy(infor->r_date,r_buf,10);
    memcpy(infor->soft,r_buf+10,8);
    memcpy(infor->hard,r_buf+18,8);
    return 0;
}

static int dev_get_sev_soft_edit(struct _switch_dev_fd   *id,va_list args)
{   char *soft;
    out("获取机柜USB控制板软件版本\n");
    soft=va_arg(args,char *);
    if(soft==NULL)
        return -1;
	return oper_stm32_dev(id->dev, DF_PC_ORDER_USB_GET_SOFT, NULL, (uint8 *)soft, 0, NULL);
}

static int dev_get_heart_timer(struct _switch_dev_fd   *id,va_list args)
{   uint8 buf[1024];
    int result;
    int8 order;
    memset(buf,0,sizeof(buf));
    //out("获取心跳时间\n");
    result=oper_stm32_dev(id->dev,DF_PC_ORDER_HREAT_TIMER,NULL,buf,0,&order);
    //out("获取心跳时间成功,result=%d\n",result);
    if(result<0)
        return result;
    if(order<0)
        return DF_ERR_PORT_CORE_TY_USB_STM32_DEV;
    result=buf[0];
    //out("心跳时间为:%d秒\n",result);
    return result;
}

static int dev_oper_heart(struct _switch_dev_fd   *id,va_list args)
{   uint8 clock[8];
    //out("心跳\n");
    clock_get(clock);
	//logout(INFO, "system", "心跳","心跳时间%02x%02x-%02x-%02x %02x:%02x:%02x\n", clock[0], clock[1], clock[2], clock[3], clock[4], clock[5], clock[6], clock[7]);
    return oper_stm32_dev(id->dev,DF_PC_ORDER_HREAT,clock,NULL,sizeof(clock),NULL);   
}

static int dev_oper_add_recoder(struct _switch_dev_fd   *id,va_list args)
{   struct _switch_event    *event;
    out("增加事件\n");
    event=va_arg(args,struct _switch_event *);
	return oper_stm32_dev(id->dev, DF_PC_ORDER_WRITE_RECODER, (uint8 *)event, NULL, sizeof(struct _switch_event), NULL);
}

static int dev_open_read_recoder(struct _switch_dev_fd   *id,va_list args)
{   struct _switch_event_all    *all;
    //out("读事件\n");
    all=va_arg(args,struct _switch_event_all *);
    if(all==NULL)
        return -1;
    return oper_stm32_dev(id->dev,DF_PC_ORDER_READ_RECODER,NULL,(uint8 *)all,0,NULL);
}

static int dev_oper_get_net_ok(struct _switch_dev_fd   *id,va_list args)
{	//out("获取网络通知指示\n");
	return oper_stm32_dev(id->dev,DF_PC_ORDER_GET_NET_OK,NULL,NULL,0,NULL);
}

static int dev_oper_no_net(struct _switch_dev_fd   *id,va_list args)
{	//out("网络在运行过程中丢失\n");
	return oper_stm32_dev(id->dev,DF_PC_ORDER_NO_NET,NULL,NULL,0,NULL);
}


static int dev_oper_port_dev_local(struct _switch_dev_fd *id,va_list args)
{	int port;
	int result;
	uint8 buf[20];	
	port=va_arg(args,int );  
	//out("指示端口%d设备在本地\n",port);
	buf[0]=(port/256);buf[1]=(port&0xff);
	buf[2]=0x01;
	buf[3]=0x00;
	result = oper_stm32_dev(id->dev,DF_PC_ORDER_CTL_PORT_LED,buf,NULL,4,NULL);
	if (result < 0)
	{
		logout(INFO, "SYSTEM", "单片机", "操作单片机，%d端口灯亮红灯失败\r\n", port);
	}
	return result;
}

static int dev_oper_port_dev_share(struct _switch_dev_fd *id,va_list args)
{	int port;
	int result;
	uint8 buf[20];	
	port=va_arg(args,int ); 
	//out("指示端口%d设备被共享\n",port);
	buf[0]=(port/256);buf[1]=(port&0xff);
	buf[2]=0x00;
	buf[3]=0x01;
	result = oper_stm32_dev(id->dev,DF_PC_ORDER_CTL_PORT_LED,buf,NULL,4,NULL);
	if (result < 0)
	{
		logout(INFO, "SYSTEM", "单片机", "操作单片机，%d端口灯亮绿灯失败\r\n", port);
	}
	return result;
}

static int dev_oper_port_no_dev(struct _switch_dev_fd *id,va_list args)
{	int port;
	int result;
	uint8 buf[20];	
	port=va_arg(args,int );  
	//out("指示端口%d没有设备\n",port);
	buf[0]=(port/256);buf[1]=(port&0xff);
	buf[2]=0x00;
	buf[3]=0x00;
	result = oper_stm32_dev(id->dev,DF_PC_ORDER_CTL_PORT_LED,buf,NULL,4,NULL);
	if (result < 0)
	{
		logout(INFO, "SYSTEM", "单片机", "操作单片机，%d端口灯熄灭失败\r\n", port);
	}
	return result;
}

static int dev_oper_red_off_gren_off(struct _switch_dev_fd *id, va_list args)
{
	int port;
	int result;
	uint8 buf[20];
	port = va_arg(args, int);

	struct _switch_dev *stream;
	stream = id->dev;
	if (stream->dev_support.support_lcd != 1)
	{
		buf[0] = (port / 256); buf[1] = (port & 0xff);
		buf[2] = 0x00;
		buf[3] = 0x00;
		result = oper_stm32_dev(id->dev, DF_PC_ORDER_CTL_PORT_LED, buf, NULL, 4, NULL);
		return result;
	}
	//out("指示端口%d没有设备\n",port);
	buf[0] = (port / 256); buf[1] = (port & 0xff);
	buf[2] = 0x00;//1号灯状态 0灭 1亮 2闪
	buf[3] = 0x00;//2号灯状态 0灭 1亮 2闪
	buf[4] = 0x00;//1号灯熄灭时长 x*100ms
	buf[5] = 0x00;//1号灯点亮时长 x*100ms
	buf[6] = 0x00;//2号灯熄灭时长 x*100ms
	buf[7] = 0x00;//2号灯点亮时长 x*100ms
	result = oper_stm32_dev(id->dev, DF_PC_ORDER_CTL_PORT_LED_2, buf, NULL, 8, NULL);
	return result;
}
static int dev_oper_red_on_gren_off(struct _switch_dev_fd *id, va_list args)
{
	int port;
	int result;
	uint8 buf[20];
	port = va_arg(args, int);

	struct _switch_dev *stream;
	stream = id->dev;
	if (stream->dev_support.support_lcd != 1)
	{
		buf[0] = (port / 256); buf[1] = (port & 0xff);
		buf[2] = 0x01;
		buf[3] = 0x00;
		result = oper_stm32_dev(id->dev, DF_PC_ORDER_CTL_PORT_LED, buf, NULL, 4, NULL);
		return result;
	}
	out("指示端口%d红灯亮绿灯灭\n",port);

	buf[0] = (port / 256); buf[1] = (port & 0xff);
	buf[2] = 0x01;//1号灯状态 0灭 1亮 2闪
	buf[3] = 0x00;//2号灯状态 0灭 1亮 2闪
	buf[4] = 0x00;//1号灯熄灭时长 x*100ms
	buf[5] = 0x00;//1号灯点亮时长 x*100ms
	buf[6] = 0x00;//2号灯熄灭时长 x*100ms
	buf[7] = 0x00;//2号灯点亮时长 x*100ms
	result = oper_stm32_dev(id->dev, DF_PC_ORDER_CTL_PORT_LED_2, buf, NULL, 8, NULL);
	return result;
}
static int dev_oper_red_off_gren_on(struct _switch_dev_fd *id, va_list args)
{
	int port;
	int result;
	uint8 buf[20];
	port = va_arg(args, int);

	struct _switch_dev *stream;
	stream = id->dev;
	if (stream->dev_support.support_lcd != 1)
	{
		buf[0] = (port / 256); buf[1] = (port & 0xff);
		buf[2] = 0x00;
		buf[3] = 0x01;
		result = oper_stm32_dev(id->dev, DF_PC_ORDER_CTL_PORT_LED, buf, NULL, 4, NULL);
		return result;
	}
	//out("指示端口%d没有设备\n",port);

	buf[0] = (port / 256); buf[1] = (port & 0xff);
	buf[2] = 0x00;//1号灯状态 0灭 1亮 2闪
	buf[3] = 0x01;//2号灯状态 0灭 1亮 2闪
	buf[4] = 0x00;//1号灯熄灭时长 x*100ms
	buf[5] = 0x00;//1号灯点亮时长 x*100ms
	buf[6] = 0x00;//2号灯熄灭时长 x*100ms
	buf[7] = 0x00;//2号灯点亮时长 x*100ms
	result = oper_stm32_dev(id->dev, DF_PC_ORDER_CTL_PORT_LED_2, buf, NULL, 8, NULL);
	return result;
}
static int dev_oper_red_on_gren_twi(struct _switch_dev_fd *id, va_list args)
{
	int port;
	int result;
	uint8 buf[20];
	port = va_arg(args, int);

	struct _switch_dev *stream;
	stream = id->dev;
	//out("指示端口%d交替闪烁,红灯常1.9秒,绿灯亮0.1秒\n",port);
	if (stream->dev_support.support_lcd != 1)
	{
		buf[0] = (port / 256); buf[1] = (port & 0xff);
		buf[2] = 0x01;
		buf[3] = 0x00;
		result = oper_stm32_dev(id->dev, DF_PC_ORDER_CTL_PORT_LED, buf, NULL, 4, NULL);
		return result;
	}

	buf[0] = (port / 256); buf[1] = (port & 0xff);
	buf[2] = 0x03;//1号灯状态 0灭 1亮 2闪
	buf[3] = 0x03;//2号灯状态 0灭 1亮 2闪
	buf[4] = 0x00;//1号灯熄灭时长 x*100ms
	buf[5] = 0x01;//1号灯点亮时长 x*100ms
	buf[6] = 0x00;//2号灯熄灭时长 x*100ms
	buf[7] = 0x1D;//2号灯点亮时长 x*100ms
	result = oper_stm32_dev(id->dev, DF_PC_ORDER_CTL_PORT_LED_2, buf, NULL, 8, NULL);
	return result;
}
static int dev_oper_red_on_gren_on(struct _switch_dev_fd *id, va_list args)
{
	int port;
	int result;
	uint8 buf[20];
	port = va_arg(args, int);
	struct _switch_dev *stream;
	stream = id->dev;
	//out("指示端口%d交替闪烁,红灯常0.3秒,绿灯亮0.3秒\n", port);
	if (stream->dev_support.support_lcd != 1)
	{
		buf[0] = (port / 256); buf[1] = (port & 0xff);
		buf[2] = 0x00;
		buf[3] = 0x01;
		result = oper_stm32_dev(id->dev, DF_PC_ORDER_CTL_PORT_LED, buf, NULL, 4, NULL);
		return result;
	}
	buf[0] = (port / 256); buf[1] = (port & 0xff);
	buf[2] = 0x03;//1号灯状态 0灭 1亮 2闪
	buf[3] = 0x03;//2号灯状态 0灭 1亮 2闪
	buf[4] = 0x00;//1号灯熄灭时长 x*100ms
	buf[5] = 0x01;//1号灯点亮时长 x*100ms
	buf[6] = 0x00;//2号灯熄灭时长 x*100ms
	buf[7] = 0x01;//2号灯点亮时长 x*100ms
	result = oper_stm32_dev(id->dev, DF_PC_ORDER_CTL_PORT_LED_2, buf, NULL, 8, NULL);
	return result;
}
static int dev_oper_red_twi_gren_on(struct _switch_dev_fd *id, va_list args)
{
	int port;
	int result;
	uint8 buf[20];
	port = va_arg(args, int);
	struct _switch_dev *stream;
	stream = id->dev;
	if (stream->dev_support.support_lcd != 1)
	{
		return 0;
	}
	//out("指示端口%d没有设备\n",port);
	buf[0] = (port / 256); buf[1] = (port & 0xff);
	buf[2] = 0x00;//1号灯状态 0灭 1亮 2闪
	buf[3] = 0x02;//2号灯状态 0灭 1亮 2闪
	buf[4] = 0x00;//1号灯熄灭时长 x*100ms
	buf[5] = 0x00;//1号灯点亮时长 x*100ms
	buf[6] = 0x01;//2号灯熄灭时长 x*100ms
	buf[7] = 0x01;//2号灯点亮时长 x*100ms
	result = oper_stm32_dev(id->dev, DF_PC_ORDER_CTL_PORT_LED_2, buf, NULL, 8, NULL);
	return result;
}

static int dev_oper_red_twi_gren_twi_1s(struct _switch_dev_fd *id, va_list args)//税控口令异常显示
{
	int port;
	int result;
	uint8 buf[20];
	port = va_arg(args, int);
	struct _switch_dev *stream;
	stream = id->dev;
	if (stream->dev_support.support_lcd != 1)
	{
		return 0;
	}
	out("指示端口%d灯交替闪烁1s\n",port);
	buf[0] = (port / 256); buf[1] = (port & 0xff);
	buf[2] = 0x03;//1号灯状态 0灭 1亮 2闪
	buf[3] = 0x03;//2号灯状态 0灭 1亮 2闪
	buf[4] = 0x00;//1号灯熄灭时长 x*100ms
	buf[5] = 0x01;//1号灯点亮时长 x*100ms
	buf[6] = 0x00;//2号灯熄灭时长 x*100ms
	buf[7] = 0x01;//2号灯点亮时长 x*100ms
	result = oper_stm32_dev(id->dev, DF_PC_ORDER_CTL_PORT_LED_2, buf, NULL, 8, NULL);
	return result;
}
#ifndef MTK_OPENWRT
static int dev_oper_lcd_screen_print(struct _switch_dev_fd *id, va_list args)
{
	int result;
	struct _switch_dev *stream;
	uint8 *buf;
	int buf_len;
	buf = va_arg(args, uint8 *);
	buf_len = va_arg(args, int);
	stream = id->dev;	
	if (stream->dev_support.support_lcd != 1)
	{
		out("版本不支持LCD不显示,id->dev->support_lcd = %d,stream->ter_type = %s\n", id->dev->dev_support.support_lcd, stream->ter_type);
		return 0;
	}
	//out("驱动显示屏显示内容\n");
	//print_array(buf,buf_len);
	result = oper_stm32_dev(id->dev, DF_PC_ORDER_CTL_LCD_SCREEN, buf, NULL, buf_len, NULL);
	//out("单片机应答结果%d\n",result);
	return result;
}
#endif

static int dev_oper_red_off_gren_twi(struct _switch_dev_fd *id, va_list args)
{
	int port;
	int result;
	uint8 buf[20];
	port = va_arg(args, int);
	//struct _switch_dev *stream;
	//stream = id->dev;
	//printf("################dev_oper_red_off_gren_twi!!!\n");

	buf[0] = (port / 256); buf[1] = (port & 0xff);
	buf[2] = 0x00;//1号灯状态 0灭 1亮 2闪
	buf[3] = 0x02;//2号灯状态 0灭 1亮 2闪
	buf[4] = 0x00;//1号灯熄灭时长 x*100ms
	buf[5] = 0x00;//1号灯点亮时长 x*100ms
	buf[6] = 0x01;//2号灯熄灭时长 x*100ms
	buf[7] = 0x01;//2号灯点亮时长 x*100ms
	result = oper_stm32_dev(id->dev, DF_PC_ORDER_CTL_PORT_LED_2, buf, NULL, 8, NULL);
	return result;
}

static int dev_oper_red_twi_gren_off(struct _switch_dev_fd *id, va_list args)
{
	int port;
	int result;
	uint8 buf[20];
	port = va_arg(args, int);
	//struct _switch_dev *stream;
	//stream = id->dev;

	buf[0] = (port / 256); buf[1] = (port & 0xff);
	buf[2] = 0x02;//1号灯状态 0灭 1亮 2闪
	buf[3] = 0x00;//2号灯状态 0灭 1亮 2闪
	buf[4] = 0x01;//1号灯熄灭时长 x*100ms
	buf[5] = 0x01;//1号灯点亮时长 x*100ms
	buf[6] = 0x00;//2号灯熄灭时长 x*100ms
	buf[7] = 0x00;//2号灯点亮时长 x*100ms
	result = oper_stm32_dev(id->dev, DF_PC_ORDER_CTL_PORT_LED_2, buf, NULL, 8, NULL);
	return result;
}

#ifdef MTK_OPENWRT
static int dev_oper_red_twi_gren_twi(struct _switch_dev_fd *id, va_list args)
{
	int port;
	int result;
	uint8 buf[20];
	port = va_arg(args, int); 
	//struct _switch_dev *stream;
	//stream = id->dev;

	//out("指示端口%d没有设备\n",port);
	buf[0] = (port / 256); buf[1] = (port & 0xff);
	buf[2] = 0x03;//1号灯状态 0灭 1亮 2闪
	buf[3] = 0x03;//2号灯状态 0灭 1亮 2闪
	buf[4] = 0x01;//1号灯熄灭时长 x*100ms
	buf[5] = 0x01;//1号灯点亮时长 x*100ms
	buf[6] = 0x01;//2号灯熄灭时长 x*100ms
	buf[7] = 0x01;//2号灯点亮时长 x*100ms
	result = oper_stm32_dev(id->dev, DF_PC_ORDER_CTL_PORT_LED_2, buf, NULL, 8, NULL);
	return result;
}
#endif

static int dev_oper_get_key_value(struct _switch_dev_fd *id, va_list args)
{
	int result;
	struct _switch_dev *stream;
	unsigned char  *buf;
	//int buf_len;
	buf = va_arg(args, unsigned char *);
	//buf_len = va_arg(args, int);
	stream = id->dev;
	unsigned char compare_buf[1024] = { 0 };
	unsigned char s_buf[4] = { 0 };
	s_buf[0] = 0x00; s_buf[1] = 0x00; s_buf[2] = 0x02; s_buf[3] = 0x00;
	result = oper_stm32_dev(stream, DF_PC_ORDER_CTL_READ_KEY, s_buf, (unsigned char *)buf, 4, NULL);
	if (memcmp(compare_buf, buf, 512) == 0)
	{
		logout(INFO, "SWITCH_DEV", "获取单片机密钥", "获取单片机密钥失败第一次失败\r\n");
		s_buf[0] = 0x00; s_buf[1] = 0x00; s_buf[2] = 0x02; s_buf[3] = 0x00;
		result = oper_stm32_dev(stream, DF_PC_ORDER_CTL_READ_KEY, s_buf, (unsigned char *)buf, 4, NULL);

		if (memcmp(compare_buf, buf, 512) == 0)
		{

			logout(INFO, "SWITCH_DEV", "获取单片机密钥", "获取单片机密钥失败第二次失败\r\n");
		}
	}

	s_buf[0] = 0x02; s_buf[1] = 0x00; s_buf[2] = 0x02; s_buf[3] = 0x00;
	result = oper_stm32_dev(stream, DF_PC_ORDER_CTL_READ_KEY, s_buf, (unsigned char *)buf + 512, 4, NULL);
	if (memcmp(compare_buf, buf + 512, 512) == 0)
	{

		logout(INFO, "SWITCH_DEV", "获取单片机密钥", "获取单片机密钥失败第三次失败\r\n");
		s_buf[0] = 0x02; s_buf[1] = 0x00; s_buf[2] = 0x02; s_buf[3] = 0x00;
		result = oper_stm32_dev(stream, DF_PC_ORDER_CTL_READ_KEY, s_buf, (unsigned char *)buf + 512, 4, NULL);
		if (memcmp(compare_buf, buf + 512, 512) == 0)
		{

			logout(INFO, "SWITCH_DEV", "获取单片机密钥", "获取单片机密钥失败第四次失败\r\n");
		}
	}

	s_buf[0] = 0x04; s_buf[1] = 0x00; s_buf[2] = 0x02; s_buf[3] = 0x00;
	result = oper_stm32_dev(stream, DF_PC_ORDER_CTL_READ_KEY, s_buf, (unsigned char *)buf + 512 + 512, 4, NULL);
	if (memcmp(compare_buf, buf + 512 + 512, 512) == 0)
	{
		logout(INFO, "SWITCH_DEV", "获取单片机密钥", "获取单片机密钥失败第五次失败\r\n");
		s_buf[0] = 0x04; s_buf[1] = 0x00; s_buf[2] = 0x02; s_buf[3] = 0x00;
		result = oper_stm32_dev(stream, DF_PC_ORDER_CTL_READ_KEY, s_buf, (unsigned char *)buf + 512 + 512, 4, NULL);
		if (memcmp(compare_buf, buf + 512 + 512, 512) == 0)
		{
			logout(INFO, "SWITCH_DEV", "获取单片机密钥", "获取单片机密钥失败第六次失败\r\n");
		}
	}




	s_buf[0] = 0x06; s_buf[1] = 0x00; s_buf[2] = 0x02; s_buf[3] = 0x00;
	result = oper_stm32_dev(stream, DF_PC_ORDER_CTL_READ_KEY, s_buf, (unsigned char *)buf + 512 + 512 + 512, 4, NULL);
	if (memcmp(compare_buf, buf + 512 + 512 + 512, 512) == 0)
	{
		logout(INFO, "SWITCH_DEV", "获取单片机密钥", "获取单片机密钥失败第七次失败\r\n");
		s_buf[0] = 0x06; s_buf[1] = 0x00; s_buf[2] = 0x02; s_buf[3] = 0x00;
		result = oper_stm32_dev(stream, DF_PC_ORDER_CTL_READ_KEY, s_buf, (unsigned char *)buf + 512 + 512 + 512, 4, NULL);


		if (memcmp(compare_buf, buf + 512 + 512 + 512, 512) == 0)
		{
			logout(INFO, "SWITCH_DEV", "获取单片机密钥", "获取单片机密钥失败第八次失败\r\n");
		}
	}




	s_buf[0] = 0x08; s_buf[1] = 0x00; s_buf[2] = 0x02; s_buf[3] = 0x00;
	result = oper_stm32_dev(stream, DF_PC_ORDER_CTL_READ_KEY, s_buf, (unsigned char *)buf + 512 + 512 + 512 + 512, 4, NULL);
	if (memcmp(compare_buf, buf, 512) == 0)
	{
		logout(INFO, "SWITCH_DEV", "获取单片机密钥", "获取单片机密钥失败第九次失败\r\n");
		s_buf[0] = 0x08; s_buf[1] = 0x00; s_buf[2] = 0x02; s_buf[3] = 0x00;
		result = oper_stm32_dev(stream, DF_PC_ORDER_CTL_READ_KEY, s_buf, (unsigned char *)buf + 512 + 512 + 512 + 512, 4, NULL);
	}




	s_buf[0] = 0x0a; s_buf[1] = 0x00; s_buf[2] = 0x02; s_buf[3] = 0x00;
	result = oper_stm32_dev(stream, DF_PC_ORDER_CTL_READ_KEY, s_buf, (unsigned char *)buf + 512 + 512 + 512 + 512 + 512, 4, NULL);
	if (memcmp(compare_buf, buf + 1024, 1024) == 0)
	{
		logout(INFO, "SWITCH_DEV", "获取单片机密钥", "获取单片机密钥失败第十次失败\r\n");
		s_buf[0] = 0x0a; s_buf[1] = 0x00; s_buf[2] = 0x02; s_buf[3] = 0x00;
		result = oper_stm32_dev(stream, DF_PC_ORDER_CTL_READ_KEY, s_buf, (unsigned char *)buf + 512 + 512 + 512 + 512 + 512, 4, NULL);
	}




	s_buf[0] = 0x0c; s_buf[1] = 0x00; s_buf[2] = 0x02; s_buf[3] = 0x00;
	result = oper_stm32_dev(stream, DF_PC_ORDER_CTL_READ_KEY, s_buf, (unsigned char *)buf + 512 + 512 + 512 + 512 + 512 + 512, 4, NULL);
	if (memcmp(compare_buf, buf + 1024 + 1024, 1024) == 0)
	{
		logout(INFO, "SWITCH_DEV", "获取单片机密钥", "获取单片机密钥失败第十一次失败\r\n");
		s_buf[0] = 0x0c; s_buf[1] = 0x00; s_buf[2] = 0x02; s_buf[3] = 0x00;
		result = oper_stm32_dev(stream, DF_PC_ORDER_CTL_READ_KEY, s_buf, (unsigned char *)buf + 512 + 512 + 512 + 512 + 512 + 512, 4, NULL);
	}



	s_buf[0] = 0x0e; s_buf[1] = 0x00; s_buf[2] = 0x02; s_buf[3] = 0x00;
	result = oper_stm32_dev(stream, DF_PC_ORDER_CTL_READ_KEY, s_buf, (unsigned char *)buf + 512 + 512 + 512 + 512 + 512 + 512 + 512, 4, NULL);
	if (memcmp(compare_buf, buf + 1024 + 1024 + 1024, 1024) == 0)
	{
		logout(INFO, "SWITCH_DEV", "获取单片机密钥", "获取单片机密钥失败第十二次失败\r\n");
		s_buf[0] = 0x0e; s_buf[1] = 0x00; s_buf[2] = 0x02; s_buf[3] = 0x00;
		result = oper_stm32_dev(stream, DF_PC_ORDER_CTL_READ_KEY, s_buf, (unsigned char *)buf + 512 + 512 + 512 + 512 + 512 + 512 + 512, 4, NULL);
	}
	//printf_array(buf,4096);

	return result;

}

static int dev_power_close(struct _switch_dev_fd   *id,va_list args)                            
{   out("关机完成,等待复位\n");
	oper_stm32_dev(id->dev,DF_PC_ORDER_POWER_OFF,NULL,NULL,0,NULL);
    
    for(;;);
	return 0;
}

static int dev_open_hub(struct _switch_dev_fd   *id,va_list args)
{   out("打开所有的hub\n");
    return oper_stm32_dev(id->dev,DF_PC_ORDER_OPEN_HUB,NULL,NULL,0,NULL);
}



static int dev_power_up(struct _switch_dev	*stream)               //开机
{   //out("给设备开机\n");
	return oper_stm32_dev(stream,DF_PC_ORDER_CONNECT,NULL,NULL,0,NULL);
}
#ifndef MTK_OPENWRT
static int mac_ic_restart(struct _switch_dev	*stream)
{	uint8 buf[10];
	out("下电\n");
	memset(buf,0,sizeof(buf));
	buf[0]=0x00;
	oper_stm32_dev(stream,DF_PC_ORDER_CTL_MAC_POWER,buf,NULL,1,NULL);
	usleep(100000);
	out("上电\n");
	buf[0]=0x01;
	oper_stm32_dev(stream,DF_PC_ORDER_CTL_MAC_POWER,buf,NULL,1,NULL);
	//logout(INFO, "system", "init", "复位mac芯片成功，延时10秒\n");
	//sleep(1);//要兼容dhcp此处需延时5秒
	//sleep(2);
	return 0;
}
#endif

static int dev_power_down(struct _switch_dev	*stream)         //释放控制权
{	//out("释放控制权\n");
	return oper_stm32_dev(stream,DF_PC_ORDER_DISCONNECT,NULL,NULL,0,NULL);
}

#ifdef MTK_OPENWRT
static int dev_status_set(struct _switch_dev_fd *id, va_list args)
{
	out("修改灯状态\n");
	int result;
	struct _switch_dev *stream;
	uint8 *buf;
	int buf_len;
	buf = va_arg(args, uint8 *);
	buf_len = va_arg(args, int);
	stream = id->dev;
	//out("驱动显示屏显示内容--\n");
	//print_array(buf,buf_len);
	if (strcmp(stream->ter_type, "MB1906") != 0)
		return 0;
	result = oper_stm32_dev(id->dev, DF_PC_ORDER_CTL_NEW_STATUS, buf, NULL, buf_len, NULL);
	//out("单片机应答结果%d\n",result);
	return result;
}

#endif




/*--------------------------------------发送和接受命令--------------------------------------------*/
static int oper_stm32_dev(struct _switch_dev	*stream,uint8 order,uint8 *in_buf,uint8 *out_buf,
                        int in_len,int8 *r_order)
{
	uint8 buf[65536] = { 0 };
    int len,i,j,datalen;
    int result;
    uint16 crc,crc_val;
    int timer;
   // out("发送命令给STM32,并且等待返回,该部分单任务运行,命令字为:%02x\n",order);
    len=pack_s_buf(order,in_buf,buf,in_len);
    _lock_set(stream->lock);
    ty_ctl(stream->seria,DF_SERIA_CM_CLS);
    result=ty_write(stream->seria,buf,len);
    if(result<0)
    {   _lock_un(stream->lock);
        return result;
    }    
    for(len=0,timer=0;timer<DF_TY_STM32_OVER_TIMER;timer++,usleep(1000))
    {   result=ty_ctl(stream->seria,DF_SERIA_CM_GET);
        if(result<0)
        {  _lock_un(stream->lock);
            return result;
        }    
        if(result==0)
            continue;
        len+=ty_read(stream->seria,buf+len,sizeof(buf)-len);
        if(len<DF_PC_ORDER_HEAD_LEN)
            continue;
        for(i=0;i<(len-1);i++)
		{	if((buf[i]=='T')&&(buf[i+1]=='Y'))
				break;
		}
		for(j=0;j<(len-i);j++)
		{	buf[j]=buf[i+j];	
		}
		len=len-i;
		if(len<DF_PC_ORDER_HEAD_LEN)
			continue;
		datalen=buf[2]*256+buf[3];
		if(datalen>len)
			continue;
		if (datalen - 2 < 0)
		{
			logout(INFO, "SYSTEM", "单片机", "crc_8005 测试 datalen 长度错误\r\n");
			_lock_un(stream->lock);
			return -1;
		}			
		crc=crc_8005(buf,datalen-2,0);
		crc_val=buf[datalen-2]*256+buf[datalen-1];
		if(crc!=crc_val)
		{	len=0;
			continue;
		}
        if(r_order!=NULL)
        {   *r_order=buf[4];
        }
        if(out_buf==NULL)
		{	_lock_un(stream->lock);
            return 0;
		}	
		memcpy(out_buf,buf+5,datalen-DF_PC_ORDER_HEAD_LEN);
        _lock_un(stream->lock);
        return datalen-DF_PC_ORDER_HEAD_LEN;	
    } 
    _lock_un(stream->lock);
    return DF_ERR_PORT_CORE_TY_USB_STM32_ANSWER;
}


static int pack_s_buf(uint8 order,uint8 *in_buf,uint8 *out_buf,int in_len)
{   uint16 len;
    uint16 crc;
    len=in_len+DF_PC_ORDER_HEAD_LEN;
	out_buf[0]='T';out_buf[1]='Y';
	out_buf[2]=((len>>8)&0xff);out_buf[3]=(len&0xff);
	out_buf[4]=order;
	memcpy(out_buf+5,in_buf,in_len);
	crc=crc_8005(out_buf,len-2,0);
	out_buf[len-2]=((crc>>8)&0xff);
	out_buf[len-1]=(crc&0xff);
    return (int )len;
}


///////////////////////////仅发送命令////////////////////////////////////////////////////////
//static int send_oper_stm32_dev(struct _switch_dev	*stream,uint8 order,uint8 *in_buf,uint8 *out_buf,int in_len,int8 *r_order)
//{   uint8 buf[2048];
//    int len,i,j,datalen;
//    int result;
//    uint16 crc,crc_val;
//    int timer;
//    //out("发送命令给STM32,不等待返回\n");
//    len=pack_s_buf(order,in_buf,buf,in_len);
//    //_lock_set(stream->lock);
//    ty_ctl(stream->seria,DF_SERIA_CM_CLS);
//    result=ty_write(stream->seria,buf,len);
//    if(result<0)
//    {   //_lock_un(stream->lock);
//		out("发送串口数据失败\n");
//        return result;
//    }    
//    return result;
//}