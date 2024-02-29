#define _ty_usb_c
#include "_ty_usb.h"
int ty_usb_add(char *switch_name,int type)
{   struct _ty_usb *stream;
	int result;
	stream=malloc(sizeof(struct _ty_usb));
	if(stream==NULL)
		return DF_ERR_MEM_ERR;
	memset(stream,0,sizeof(struct _ty_usb));
    memcpy(stream->switch_name,switch_name,strlen(switch_name));
	//out("本系统操作时,直接打开所有HUB\n");
	stream->hub=DF_TY_USB_HUB_OPEN;//云账房版本常开，航信版本常关
	stream->type=type;
	ty_usb_id = stream;
	result = ty_file_add(DF_DEV_TY_USB, stream, "ty_usb", (struct _file_fuc *)&ty_usb_fuc);
	if(result<0)
		free(stream);
	return result;
}

static int ty_usb_open(struct ty_file	*file)
{   struct _ty_usb  *stream;
    int result,j;
	stream=file->pro_data;
	if((stream==NULL))	
		return DF_ERR_PORT_CORE_FD_NULL;
    //out("打开ty_usb_open模块\n");
    if(stream->state==0)
    {   //out("第一次使用需要建立,usb_dev_class\n");
        stream->lock=_lock_open("ty_usb_open.c",NULL);
        if(stream->lock<0)
        {   out("申请锁失败\n");
            return stream->lock;
        }
        //out("打开开关模块\n");
        stream->switch_fd=ty_open(stream->switch_name,0,0,NULL);
        if(stream->switch_fd<0)
        {   out("打开开关失败\n");
            _lock_close(stream->lock);
            return stream->switch_fd;
        }
       // out("获取设备的信息\n");
        if((result=ty_ctl(stream->switch_fd,DF_SWITCH_CM_GET_INFOR,&stream->data.stm32_infor))<0)
        {   out("获取设备信息失败\n");
            ty_close(stream->switch_fd);
            _lock_close(stream->lock);
            return result;
        }
        //out("获取设备的id信息\n");
        if((result=ty_ctl(stream->switch_fd,DF_SWITCH_CM_GET_ID,&stream->data.self_id))<0)
        {   out("获取设备id失败\n");
            ty_close(stream->switch_fd);
            _lock_close(stream->lock);
            return result;
        }
        //out("获取机柜的id\n");
		uint8 gg_id[6];
		memset(gg_id,0,sizeof(gg_id));
        if((result=ty_ctl(stream->switch_fd,DF_SWITCH_CM_GET_CLASS,DF_SWITCH_CM_CLASS_ID,stream->data.dev.id,1))<0)
        {   out("获取机柜id失败\n");
            ty_close(stream->switch_fd);
            _lock_close(stream->lock);
            return result;
        }
        //out("获取机柜的软件版本\n");
        if((result=ty_ctl(stream->switch_fd,DF_SWITCH_CM_GET_DEV_SOFT_EDIT,stream->data.dev.soft))<0)
        {   out("获取机柜软件版本失败\n");
            ty_close(stream->switch_fd);
            _lock_close(stream->lock);
            return result;  
        }
        struct _switch_dev_pcb   pcb_id[sizeof(stream->data.dev.pcb)/sizeof(stream->data.dev.pcb[0])];
        //out("获取单板信息\n");
        memset(pcb_id,0,sizeof(pcb_id));
        if((result=ty_ctl(stream->switch_fd,DF_SWITCH_CM_GET_PCB,stream->data.dev.id,
            pcb_id,sizeof(pcb_id)/sizeof(pcb_id[0])))<0)
        {   out("获取机柜群单板信息失败\n");
            ty_close(stream->switch_fd);
            _lock_close(stream->lock);
            return result;
        }
        stream->data.dev.n=result;
		if(stream->hub==DF_TY_USB_HUB_OPEN)
		{	out("打开hub\n");
			ty_ctl(stream->switch_fd,DF_SWITCH_CM_OPEN_HUB);
		}
		for(j=0,stream->data.dev.usb_port_n=0;j<result;j++)
        {   memcpy(stream->data.dev.pcb[j].id,pcb_id[j].id,sizeof(stream->data.dev.pcb[0].id));
            stream->data.dev.pcb[j].n=pcb_id[j].n;
            stream->data.dev.usb_port_n+=stream->data.dev.pcb[j].n;       
        }
		if (stream->data.dev.usb_port_n> DF_TY_USB_PORT_N)
		{
			logout(INFO, "SYSTEM", "USB", "端口数量获取失败,该机型端口数为%d口,获取到的端口数量为%d\r\n", DF_TY_USB_PORT_N, stream->data.dev.usb_port_n);
			return -1;
		}
        for(j=0;j<stream->data.dev.usb_port_n;j++)
        {   
			stream->data.dev.port_lock[j]=_lock_open("d",NULL);
			if (stream->data.dev.port_lock[j] < 0)
			{
				logout(INFO, "SYSTEM", "USB", "ty_usb端口互斥锁打开失败\r\n");
				return -1;
			}
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
	stream->fd[j].dev=stream; 
	stream->fd[j].state=1; 
	stream->fd[j].mem_fd=&file->fd[j];
	stream->state++;	
    out("打开ty_usb成功:j=%d\n",j);
    //if(ty_pd==0)
      //  ty_pd_add_file("ty_usb");
	return j+1;	   
}

static int ty_usb_ctl(void *data,int fd,int cm,va_list args)
{   struct _ty_usb  *stream;
    struct _ty_usb_fd   *id;
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
    for(i=0;i<sizeof(ctl_fun)/sizeof(ctl_fun[0]);i++)
    {   if(cm==ctl_fun[i].cm)
            return ctl_fun[i].ctl(id,args);
    }
    return DF_ERR_PORT_CORE_CM;    
}

static int ty_usb_close(void *data,int fd)
{   struct _ty_usb  *stream;
    struct _ty_usb_fd   *id;
    int j;
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
	{   ty_close(stream->switch_fd);
        for(j=0;j<stream->data.dev.usb_port_n;j++)
        {   _lock_close(stream->data.dev.port_lock[j]);
        }
        _lock_close(stream->data.dev.port_lock[j]);
	}
    return 0;   
}


static int _get_all_port_status(struct _ty_usb_fd *id, va_list args)
{
	struct _ty_usb  *stream;
	int result;
	uint8 *all_info;
	//char bus[4];
	//char busid[DF_TY_USB_BUSID];
	stream = id->dev;
	all_info = va_arg(args, uint8 *);
	result = ty_ctl(stream->switch_fd, DF_SWITCH_CM_GET_ALL_PORT_STATUS, all_info);
	if (result < 0)
	{
		return result;

	}
	return result;
}
static int _set_fan(struct _ty_usb_fd *id, va_list args)
{
	struct _ty_usb  *stream;
	int result;
	uint8 *all_info;
	//char bus[4];
	//char busid[DF_TY_USB_BUSID];
	stream = id->dev;
	all_info = va_arg(args, uint8 *);
	result = ty_ctl(stream->switch_fd, DF_SWITCH_CM_CONTRL_FAN, all_info);
	if (result < 0)
	{
		return result;

	}
	return result;
}
static int _ctl_dev_restore(struct _ty_usb_fd *id, va_list args)
{
	struct _ty_usb  *stream;
	int result;
	stream = id->dev;
	result = ty_ctl(stream->switch_fd, DF_SWITCH_CM_RESTORE, NULL);
	if (result<0)
	{
		return result;
	}
	return 0;
}

static int _get_all_ad_status(struct _ty_usb_fd *id, va_list args)
{
	struct _ty_usb  *stream;
	int result;
	uint8 *all_info;
	//char bus[4];
	//char busid[DF_TY_USB_BUSID];
	stream = id->dev;
	all_info = va_arg(args, uint8 *);
	result = ty_ctl(stream->switch_fd, DF_SWITCH_CM_GET_AD_STATUS, all_info);
	if (result < 0)
	{
		return result;

	}
	return result;
}

static int _set_led_double_light_yellow(struct _ty_usb_fd *id, va_list args)
{
	struct _ty_usb  *stream;
	int port, result;
	stream = id->dev;
	port = va_arg(args, int);
	result = ty_ctl(stream->switch_fd, DF_SWITCH_CM_PORT_DOUBLE_LIGHT_YELLOW, port);
	if (result<0)
	{
		return result;
	}
	return result;

}

static int _ctl_bluetooth_mode(struct _ty_usb_fd *id, va_list args)
{
	struct _ty_usb  *stream;
	int result;
	uint8 * control;
	control = va_arg(args, uint8 *);
	stream = id->dev;
	result = ty_ctl(stream->switch_fd, DF_SWITCH_CM_BLUETOOTH_MODE, control);
	if (result < 0)
	{
		return result;

	}
	return result;
}
static int _ctl_bluetooth_power(struct _ty_usb_fd *id, va_list args)
{
	struct _ty_usb  *stream;
	int result;
	uint8 * control;
	control = va_arg(args, uint8 *);
	stream = id->dev;
	result = ty_ctl(stream->switch_fd, DF_SWITCH_CM_BLUETOOTH_POWER, control);
	if (result < 0)
	{
		return result;

	}
	return result;
}

static int _ctl_dev_conn(struct _ty_usb_fd *id, va_list args)
{
	struct _ty_usb  *stream;
	stream = id->dev;
	return ty_ctl(stream->switch_fd, DF_SWITCH_CM_CONN, NULL);

}

static int _set_busid(struct _ty_usb_fd *id,va_list args)
{   char *busid;
    out("设置主线路的busid\n");
    busid=va_arg(args,char *);
    _lock_set(id->dev->lock);
    memcpy(id->dev->data.dev.busid,busid,strlen(busid));
    _lock_un(id->dev->lock);
    return 0;
}

static int _get_busid(struct _ty_usb_fd *id,va_list args)
{   char *busid;
    out("获取主线路的busid\n");
    busid=va_arg(args,char *);
    if(busid==NULL)
        return -1;
    _lock_set(id->dev->lock);
    memcpy(busid,id->dev->data.dev.busid,strlen(id->dev->data.dev.busid));
    _lock_un(id->dev->lock);
    return strlen(id->dev->data.dev.busid);    
}

static int _set_port_n(struct _ty_usb_fd *id,va_list args)
{   out("设置端口排列顺序\n");
    _lock_set(id->dev->lock);
    id->dev->port_n=va_arg(args,int);
    _lock_un(id->dev->lock);
    return id->dev->port_n;
}

static int _get_port_n(struct _ty_usb_fd *id,va_list args)
{   int port_n;
    out("获取端口排列顺序\n");
    _lock_set(id->dev->lock);
    port_n=id->dev->port_n;
    _lock_un(id->dev->lock);
    return port_n;
}

static int _set_enum_timer(struct _ty_usb_fd *id,va_list args)
{   out("设置最大枚举时间\n");
    _lock_set(id->dev->lock);
    id->dev->enum_timer=va_arg(args,int);
    out("最大枚举时间为:%d\n",id->dev->enum_timer);
    _lock_un(id->dev->lock);
    return id->dev->enum_timer;
}

static int _get_enum_timer(struct _ty_usb_fd *id,va_list args)
{   int enum_timer;
    out("获取最大枚举时间\n");
    _lock_set(id->dev->lock);
    enum_timer=id->dev->enum_timer;
    _lock_un(id->dev->lock);
    return enum_timer;
}

static int _get_busid2port(struct _ty_usb_fd *id,va_list args)
{   char *busid;
    struct _ty_usb  *stream;
    char port_busid[DF_TY_USB_BUSID];
    int n,i;
    out("通过busid获取到相应的port\n");
    busid=va_arg(args,char *);
    stream=id->dev;
    n=stream->data.dev.usb_port_n;
    for(i=0;i<n;i++)
    {   memset(port_busid,0,sizeof(port_busid));
        get_port_busid(stream,i+1,port_busid);
        if(memcmp(busid,port_busid,strlen(port_busid))==0)
            return i+1;
    }
    return -1;
}

static int _get_machine_id(struct _ty_usb_fd *id, va_list args)
{
	struct _ty_usb  *stream;
	char * ter_id;
	stream = id->dev;
	ter_id = va_arg(args, char *);

	bcd2asc(stream->data.self_id.id, (uint8 *)ter_id, sizeof(stream->data.self_id.id));
	
	return 0;
}

static int _get_register_whether(struct _ty_usb_fd *id,va_list args)
{
    struct _ty_usb  *stream;
	uint8 buf[12];
	uint8 idbuf[12];
	//int i;
	long int iddata;
	//char port_busid[DF_TY_USB_BUSID];
    memset(buf,0,sizeof(buf));
	buf[0]=0x30;
	buf[1]=0x36;
	buf[2]=0x38;
	stream=id->dev;
	bcd2asc(stream->data.self_id.id,idbuf,sizeof(stream->data.self_id.id));
	/*out("机器编号：");
	for(i=0;i<12;i++)
	{
		out("%02x ",idbuf[i]);
	}
	out("\n");*/
	if(memcmp(idbuf,buf,3)!=0)
	{
		out("未设置机器编号或机器编号错误\n");
		return 0;
	}
	
	iddata=(idbuf[6]&0x0f)*10000+(idbuf[7]&0x0f)*1000+(idbuf[8]&0x0f)*100+(idbuf[9]&0x0f)*10+(idbuf[10]&0x0f)*1;
	//out("ID号%d\n",iddata);
	if(iddata<0x64)
	{	
		return 2;
	}
    return 1;
}

static int _get_tryout_whether(struct _ty_usb_fd *id,va_list args)
{
	uint8 buf[8];
	//uint8 sbuf[4];
	uint8 getcbuf[8];
	uint8 tryouttime[8];
	uint8 today[8];
	int result;
	//int i;
	struct day_t day1,day2;
	//buf[0]=0x32;buf[1]=0x30;buf[2]=0x31;buf[3]=0x36;
	//buf[4]=0x31;buf[5]=0x32;buf[6]=0x31;buf[7]=0x33;
	//asc2bcd(buf,sbuf,8);
	//result = _t_file_c_write("/etc/gettry.conf",sbuf,4);
	//sync();
	memset(buf,0,sizeof(buf));
	if(read_file("/etc/gettry.conf",(char *)buf,sizeof(buf))<0)
	{	out("读取文件:[%s]失败\n","/etc/gettry.conf");
		return -1;
	}
	//out("读取到数据：");
	//for(i=0;i<4;i++)
	//{	
	//	out("%02x ",buf[i]);
	//}
	//out("\n");
	bcd2asc(buf,tryouttime,4);

	//out("试用时间：");
	//for(i=0;i<8;i++)
	//{	
	//	out("%02x ",tryouttime[i]);
	//}
	//out("\n");
	day1.year=tryouttime[0]*1000+tryouttime[1]*100+tryouttime[2]*10+tryouttime[3];
	day1.month=tryouttime[4]*10+tryouttime[5];
	day1.day=tryouttime[6]*10+tryouttime[7];



	clock_get(getcbuf);
	bcd2asc(getcbuf,today,4);
	//out("当前时间：");
	//for(i=0;i<8;i++)
	//{	
	//	out("%02x ",today[i]);
	//}
	//out("\n");
	day2.year=today[0]*1000+today[1]*100+today[2]*10+today[3];
	day2.month=today[4]*10+today[5];
	day2.day=today[6]*10+today[7];

	result = dmax(&day2,&day1);
	//out("_get_tryout_whether result =%d\n",result);
	if(result>0)
	{
		out("试用期限已过\n");
		return -1;
	}
	return 0;
}

static int _ctl_port_no_dev_led(struct _ty_usb_fd *id,va_list args)
{
	struct _ty_usb  *stream;
    int port,result;
	 stream=id->dev;
    port=va_arg(args,int);
	//out("进去_ctl_port_no_dev_led\n");
    result=ty_ctl(stream->switch_fd,DF_SWITCH_CM_PORT_NO_DEV,port);
    if(result<0)
    {   out("_ctl_port_no_dev_led:STM32 ctl port led no dev error\n");
        return DF_ERR_PORT_CORE_TY_USB_STM32_OPEN;
    }
    return 0;   
}

static int _ctl_port_dev_local_led(struct _ty_usb_fd *id,va_list args)
{
	struct _ty_usb  *stream;
    int port,result;
	 stream=id->dev;
    port=va_arg(args,int);
	//out("进去_ctl_port_dev_local_led\n");
    result=ty_ctl(stream->switch_fd,DF_SWITCH_CM_PORT_DEV_LOCAL,port);
    if(result<0)
    {   out("_ctl_port_dev_local_led:STM32 ctl port led local dev error\n");
        return DF_ERR_PORT_CORE_TY_USB_STM32_OPEN;
    }
    return 0;   
}

static int _ctl_port_dev_share_led(struct _ty_usb_fd *id,va_list args)
{
	struct _ty_usb  *stream;
    int port,result;
	 stream=id->dev;
    port=va_arg(args,int);
    result=ty_ctl(stream->switch_fd,DF_SWITCH_CM_PORT_DEV_SHARE,port);
    if(result<0)
    {   out("_ctl_port_dev_share_led:STM32 ctl port led share dev error\n");
        return DF_ERR_PORT_CORE_TY_USB_STM32_OPEN;
    }
    return 0;   
}


static int _set_led_red_off_gren_off(struct _ty_usb_fd *id, va_list args)
{
	struct _ty_usb  *stream;
	int port, result;
	stream = id->dev;
	port = va_arg(args, int);
	result = ty_ctl(stream->switch_fd, DF_SWITCH_CM_PORT_RED_OFF_GREN_OFF, port);
	if (result<0)
	{
		out("_set_led_red_off_gren_off:STM32 ctl port led share dev error\n");
		return DF_ERR_PORT_CORE_TY_USB_STM32_OPEN;
	}
	return 0;
}
static int _set_led_red_on_gren_off(struct _ty_usb_fd *id, va_list args)
{
	struct _ty_usb  *stream;
	int port, result;
	stream = id->dev;
	port = va_arg(args, int);
	result = ty_ctl(stream->switch_fd, DF_SWITCH_CM_PORT_RED_ON_GREN_OFF, port);
	if (result<0)
	{
		out("_set_led_red_on_gren_off:STM32 ctl port led share dev error\n");
		return DF_ERR_PORT_CORE_TY_USB_STM32_OPEN;
	}
	return 0;
}
static int _set_led_red_off_gren_on(struct _ty_usb_fd *id, va_list args)
{
	struct _ty_usb  *stream;
	int port, result;
	stream = id->dev;
	port = va_arg(args, int);
	result = ty_ctl(stream->switch_fd, DF_SWITCH_CM_PORT_RED_OFF_GREN_ON, port);
	if (result<0)
	{
		out("_set_led_red_off_gren_on:STM32 ctl port led share dev error\n");
		return DF_ERR_PORT_CORE_TY_USB_STM32_OPEN;
	}
	return 0;
}
static int _set_led_red_on_gren_twi(struct _ty_usb_fd *id, va_list args)
{
	struct _ty_usb  *stream;
	int port, result;
	stream = id->dev;
	port = va_arg(args, int);
	result = ty_ctl(stream->switch_fd, DF_SWITCH_CM_PORT_RED_ON_GREN_TWI, port);
	if (result<0)
	{
		out("_set_led_red_on_gren_twi:STM32 ctl port led share dev error\n");
		return DF_ERR_PORT_CORE_TY_USB_STM32_OPEN;
	}
	return 0;
}
static int _set_led_red_on_gren_on(struct _ty_usb_fd *id, va_list args)
{
	struct _ty_usb  *stream;
	int port, result;
	stream = id->dev;
	port = va_arg(args, int);
	result = ty_ctl(stream->switch_fd, DF_SWITCH_CM_PORT_RED_ON_GREN_ON, port);
	if (result<0)
	{
		out("_set_led_red_on_gren_on:STM32 ctl port led share dev error\n");
		return DF_ERR_PORT_CORE_TY_USB_STM32_OPEN;
	}
	return 0;
}
static int _set_led_red_twi_gren_on(struct _ty_usb_fd *id, va_list args)
{
	struct _ty_usb  *stream;
	int port, result;
	stream = id->dev;
	port = va_arg(args, int);
	result = ty_ctl(stream->switch_fd, DF_SWITCH_CM_PORT_RED_TWI_GREN_ON, port);
	if (result<0)
	{
		out("_set_led_red_twi_gren_on:STM32 ctl port led share dev error\n");
		return DF_ERR_PORT_CORE_TY_USB_STM32_OPEN;
	}
	return 0;
}

static int _set_led_red_twi_gren_twi_1s(struct _ty_usb_fd *id, va_list args)
{
	struct _ty_usb  *stream;
	int port, result;
	stream = id->dev;
	port = va_arg(args, int);
	result = ty_ctl(stream->switch_fd, DF_SWITCH_CM_PORT_RED_TWI_GREN_TWI_1S, port);
	if (result<0)
	{
		out("_set_led_red_twi_gren_on:STM32 ctl port led share dev error\n");
		return DF_ERR_PORT_CORE_TY_USB_STM32_OPEN;
	}
	return 0;
}

static int _set_led_read_off_green_twi(struct _ty_usb_fd *id, va_list args)
{
	struct _ty_usb  *stream;
	int port, result;
	stream = id->dev;
	port = va_arg(args, int);
	result = ty_ctl(stream->switch_fd, DF_SWITCH_CM_PORT_RED_OFF_GREN_TWI, port);
	if (result<0)
	{
		out("_set_led_red_on_gren_twi:STM32 ctl port led share dev error\n");
		return DF_ERR_PORT_CORE_TY_USB_STM32_OPEN;
	}
	return 0;
}
static int _set_led_read_twi_green_off(struct _ty_usb_fd *id, va_list args)
{
	struct _ty_usb  *stream;
	int port, result;
	stream = id->dev;
	port = va_arg(args, int);
	result = ty_ctl(stream->switch_fd, DF_SWITCH_CM_PORT_RED_TWI_GREN_OFF, port);
	if (result<0)
	{
		out("_set_led_red_on_gren_twi:STM32 ctl port led share dev error\n");
		return DF_ERR_PORT_CORE_TY_USB_STM32_OPEN;
	}
	return 0;
}
#ifdef MTK_OPENWRT

static int _set_led_red_twi_gren_twi(struct _ty_usb_fd *id, va_list args)
{
	struct _ty_usb  *stream;
	int port, result;
	stream = id->dev;
	port = va_arg(args, int);
	result = ty_ctl(stream->switch_fd, DF_SWITCH_CM_PORT_RED_TWI_GREN_TWI, port);
	if (result<0)
	{
		out("_set_led_red_twi_gren_on:STM32 ctl port led share dev error\n");
		return DF_ERR_PORT_CORE_TY_USB_STM32_OPEN;
	}
	return 0;
}

#endif

static int _get_public_praviate_key(struct _ty_usb_fd *id, va_list args)
{
	struct _ty_usb  *stream;
	int result;
	unsigned char *key_s;
	stream = id->dev;
	key_s = va_arg(args, unsigned char *);
	out("进去_get_public_praviate_key\n");
	result = ty_ctl(stream->switch_fd, DF_SWITCH_CM_GET_KEY_VALUE, key_s);
	if (result<0)
	{
		out("_ctl_port_dev_local_led:STM32 ctl port led local dev error\n");
		return DF_ERR_PORT_CORE_TY_USB_STM32_OPEN;
	}
	return 0;
}



static int _open_port_power(struct _ty_usb_fd *id,va_list args)
{   struct _ty_usb  *stream;
    int port,result;
    //char bus[4];
    //char busid[DF_TY_USB_BUSID];
    stream=id->dev;

    port=va_arg(args,int);

    _lock_set(stream->data.dev.port_lock[port-1]);
    out("_open_port_power:Send code to STM32\n");
    result=ty_ctl(stream->switch_fd,DF_SWITCH_CM_OPEN,stream->data.dev.id,port);
    if(result<0)
    {   out("_open_port_power:STM32 open port error\n");
        _lock_un(stream->data.dev.port_lock[port-1]);
        return DF_ERR_PORT_CORE_TY_USB_STM32_OPEN;
    }
    //stream->data.dev.port_state[port-1]=1;
    _lock_un(stream->data.dev.port_lock[port-1]);
    out("_open_port_power:端口打开成功\n");
    return 0;       
}

static int _close_port_power(struct _ty_usb_fd *id, va_list args)
{
	struct _ty_usb  *stream;
	int port, result;
	//char bus[4];
	//char busid[DF_TY_USB_BUSID];
	stream = id->dev;
	port = va_arg(args, int);
	_lock_set(stream->data.dev.port_lock[port - 1]);
	out("_close_port_power:Send code to STM32\n");
	result = ty_ctl(stream->switch_fd, DF_SWITCH_CM_CLOSE, stream->data.dev.id, port);
	if (result<0)
	{
		out("_close_port_power:STM32 close port error\n");
		_lock_un(stream->data.dev.port_lock[port - 1]);
		return DF_ERR_PORT_CORE_TY_USB_STM32_CLOSE;
	}
	//stream->data.dev.port_state[port-1]=1;
	_lock_un(stream->data.dev.port_lock[port - 1]);
	out("_close_port_power:端口关闭成功\n");
	return 0;
}

static int _get_port_exist(struct _ty_usb_fd *id,va_list args)
{
	
	struct _ty_usb  *stream;
    int port,result;
    char bus[4];
    char busid[DF_TY_USB_BUSID];
    stream=id->dev;
	
    port=va_arg(args,int);
    memset(bus,0,sizeof(bus));
    memcpy(bus,stream->data.dev.busid,3);
    memset(busid,0,sizeof(busid));
	
    get_port_busid(stream,port,busid);
	//out("检测busid %s\n",busid);
	//result = get_busid_over_timer(busid,1);
	//printf("")
    result=get_busid_exist(busid);
    if(result<0)
    {   //out("%d端口没有发现设备\n",port)
        return result;
    }
    return 0;

}

static int get_busid_exist(char *busid)
{   /*struct _usb_file_dev    usb_file;
	memset(&usb_file,0,sizeof(usb_file));*/
    /*if(_usb_file_get_infor(busid,&usb_file)<0)*/
	if(_usb_file_exist(busid)<0)
	{
        return DF_ERR_PORT_CORE_TY_USB_NO_FIND_DEV;
	}
    return 0;      
}



static int _open_port(struct _ty_usb_fd *id,va_list args)
{   struct _ty_usb  *stream;
    int port,result,enum_timer;
    char bus[4];
    char busid[DF_TY_USB_BUSID];
    stream=id->dev;
    out("打开usb端口==================\n");
    if(strlen(stream->data.dev.busid)<3)
    {   out("没有可用的busid\n");
        return DF_ERR_PORT_CORE_TY_USB_MAIN_BUSID;
    }
    port=va_arg(args,int);
    if((port==0)||(port>stream->data.dev.usb_port_n))    
    {   out("超过端口数量\n");
        return DF_ERR_PORT_CORE_TY_USB_PORT_OVER;
    }
    memset(bus,0,sizeof(bus));
    memcpy(bus,stream->data.dev.busid,3);
    _lock_set(stream->lock);
    if(_usb_file_get_bus_dev_n(bus)>DF_TY_BUS_DEV_NUMBER)
    {   out("已经超过了USB枚举的数量\n");
        _lock_un(stream->lock);
        return DF_ERR_PORT_CORE_TY_USB_BUS_NUMBER;
    }
    _lock_un(stream->lock);
    out("获取实际物理端口\n");
    _lock_set(stream->lock);
    port=get_port(id->dev,port);
    _lock_un(stream->lock);
    out("获取端口的busid\n");
    _lock_set(stream->lock);
    memset(busid,0,sizeof(busid));
    get_port_busid(stream,port,busid);
    _lock_un(stream->lock);
    out("获取最大枚举时间\n");
    _lock_set(stream->lock);
    enum_timer=stream->enum_timer;
    _lock_un(stream->lock);
    out("端口:%d,busid=%s,enum_timer=%d\n",port,busid,enum_timer);
    _lock_set(stream->data.dev.port_lock[port-1]);
    if(id->dev->data.dev.port_state[port-1]!=0)
    {   out("该端口端口已经打开\n");
        _lock_un(stream->data.dev.port_lock[port-1]);
        return port;
    }
    out("发送命令让单片机打开端口\n");
    result=ty_ctl(stream->switch_fd,DF_SWITCH_CM_OPEN,stream->data.dev.id,port);
    if(result<0)
    {   out("物理层打开端口失败\n");
        _lock_un(stream->data.dev.port_lock[port-1]);
        return DF_ERR_PORT_CORE_TY_USB_STM32_OPEN;
    }
    out("获取usb的文件系统是否打开\n");
    result=get_usb_file(busid,enum_timer,stream->type);
    if(result<0)
    {   out("端口已经打开没有发现设备\n");
        //ty_ctl(stream->switch_fd,DF_SWITCH_CM_CLOSE,stream->data.dev.id,port);
        _lock_un(stream->data.dev.port_lock[port-1]);
        return result;
    }
    stream->data.dev.port_state[port-1]=1;
    _lock_un(stream->data.dev.port_lock[port-1]);
    out("端口打开成功\n");
    return 0;       
}


static int _close_port_power_just(struct _ty_usb_fd *id,va_list args)
{
	struct _ty_usb  *stream;
    int port,result; 
    stream=id->dev;
    port=va_arg(args,int);
    if((port==0)||(port>stream->data.dev.usb_port_n))    
    {   out("超过端口数量\n");
        return DF_ERR_PORT_CORE_TY_USB_PORT_OVER;
    }
    out("获取实际物理端口\n");
    port=get_port(id->dev,port);    
    result = ty_ctl(stream->switch_fd,DF_SWITCH_CM_CLOSE,stream->data.dev.id,port);
    id->dev->data.dev.port_state[port-1]=0;
	//usleep(500000);
    return result;   
}

static int _close_port(struct _ty_usb_fd *id,va_list args)
{   struct _ty_usb  *stream;
    int port,result,enum_timer;
    char bus[4];
    char busid[DF_TY_USB_BUSID];    
    stream=id->dev;
    if(strlen(stream->data.dev.busid)<3)
    {   out("没有可用的busid\n");
        return DF_ERR_PORT_CORE_TY_USB_MAIN_BUSID;
    }
    port=va_arg(args,int);
    if((port==0)||(port>stream->data.dev.usb_port_n))    
    {   out("超过端口数量\n");
        return DF_ERR_PORT_CORE_TY_USB_PORT_OVER;
    }
    memset(bus,0,sizeof(bus));
    memcpy(bus,stream->data.dev.busid,3);
    out("获取实际物理端口\n");
    _lock_set(stream->lock);
    port=get_port(id->dev,port);
    _lock_un(stream->lock);
    out("获取端口的busid\n");
    _lock_set(stream->lock);
    memset(busid,0,sizeof(busid));
    get_port_busid(stream,port,busid);
    _lock_un(stream->lock);
    out("获取最大枚举时间\n");
    _lock_set(stream->lock);
    enum_timer=stream->enum_timer;
    _lock_un(stream->lock);
    out("端口:%d,busid=%s,enum_timer=%d\n",port,busid,enum_timer);
    _lock_set(stream->data.dev.port_lock[port-1]);
    if(id->dev->data.dev.port_state[port-1]==0)
    {   out("该端口端口已经关闭\n");
        _lock_un(stream->data.dev.port_lock[port-1]);
        return port;
    }
    ty_ctl(stream->switch_fd,DF_SWITCH_CM_CLOSE,stream->data.dev.id,port);
    id->dev->data.dev.port_state[port-1]=0;
    out("判断该设备是否在文件系统中消失\n");
    result=wait_usb_file_no(busid,enum_timer);
    if(result<0)
    {   _lock_un(stream->data.dev.port_lock[port-1]);
        return result;    
    }
	//if(stream->hub!=DF_TY_USB_HUB_OPEN)
	//{	out("判断该设备关闭时,hub是否也关闭\n");
	//    if(juge_sub_hub(stream,port)>=0)
	//    {   char busid_hub[DF_TY_USB_BUSID];
	//        memset(busid_hub,0,sizeof(busid_hub));
	//        memcpy(busid_hub,busid,strlen(busid)-2);
	//        out("判断该设备的HUB是否存在\n");
	//        result=wait_usb_file_no(busid_hub,enum_timer);
	//        if(result<0)
	//        {   _lock_un(stream->data.dev.port_lock[port-1]);
	//            return result; 
	//        }
	//    }
	//    out("判断该PCB关闭时,PCB的HUB是否关闭\n");
	//    if(juge_pcb_hub(stream,port)>=0)
	//    {   char busid_pcb[DF_TY_USB_BUSID];
	//        memset(busid_pcb,0,sizeof(busid_pcb));
	//        memcpy(busid_pcb,busid,strlen(busid)-4);
	//        out("判断该设备的PCB HUB是否存在\n");
	//        result=wait_usb_file_no(busid_pcb,enum_timer);
	//        if(result<0)
	//        {   _lock_un(stream->data.dev.port_lock[port-1]);
	//            return result; 
	//        }
	//    }
	//}	
    _lock_un(stream->data.dev.port_lock[port-1]);    
	usleep(500000);
    return result;   
}

//static int juge_sub_hub(struct _ty_usb  *stream,int port)
//{   int i;
//    out("判断是否有HUB要判断\n");
//    port--;
//    port=port/7;
//    port=port*7;
//    for(i=0;i<7;i++)
//    {   if(stream->data.dev.port_state[i+port]!=0)
//            return -1;
//    }
//    return 0;
//}
//
//static int juge_pcb_hub(struct _ty_usb  *stream,int port)
//{   int i;
//    out("判断是否有PCB要判断\n");
//    port--;
//    port=port/35;
//    port=port*35;
//    for(i=0;i<35;i++)
//    {   if(stream->data.dev.port_state[i+port]!=0)
//            return -1;
//    }
//    return 0;  
//}



int get_busid_info(int port,char *busid)
{   
	struct _ty_usb  *stream;
    stream=ty_usb_id;
    if(strlen(stream->data.dev.busid)<3)
    {   out("没有可用的busid\n");
        return DF_ERR_PORT_CORE_TY_USB_MAIN_BUSID;
    }
    if((port==0)||(port>stream->data.dev.usb_port_n))    
    {   out("超过端口数量\n");
        return DF_ERR_PORT_CORE_TY_USB_PORT_OVER;
    }
    out("获取实际物理端口\n");
    _lock_set(stream->lock);
    port=get_port(stream,port);
    _lock_un(stream->lock);
    out("获取端口的busid\n");
    _lock_set(stream->lock);
    get_port_busid(stream,port,busid);
    _lock_un(stream->lock);
    return 0;   
}



static int get_busid(struct _ty_usb_fd *id,va_list args)
{   struct _ty_usb  *stream;
    int port;
    char bus[4];
    char *busid;
    out("获取设备的busid\n");
    stream=id->dev;
    port=va_arg(args,int);
    busid=va_arg(args,char *);
    if(strlen(stream->data.dev.busid)<3)
    {   out("没有可用的busid\n");
        return DF_ERR_PORT_CORE_TY_USB_MAIN_BUSID;
    }
    if((port==0)||(port>stream->data.dev.usb_port_n))    
    {   out("超过端口数量\n");
        return DF_ERR_PORT_CORE_TY_USB_PORT_OVER;
    }
    memset(bus,0,sizeof(bus));
    memcpy(bus,stream->data.dev.busid,3);
    out("获取实际物理端口\n");
    _lock_set(stream->lock);
    port=get_port(id->dev,port);
    _lock_un(stream->lock);
    out("获取端口的busid\n");
    _lock_set(stream->lock);
    get_port_busid(stream,port,busid);
    _lock_un(stream->lock);
    return 0;   
}

static int get_port_number(struct _ty_usb_fd *id,va_list args)
{   struct _ty_usb  *stream;
    int n;
    //out("获取设备的总数\n");
    stream=id->dev;
    _lock_set(stream->lock);
    n=stream->data.dev.usb_port_n;
    _lock_un(stream->lock);
    //out("设备总数:%d\n",n);
    return n;   
}

static int juge_port(struct _ty_usb_fd *id,va_list args)
{   struct _ty_usb  *stream; 
    int port;
	//out("juge_port 开始\n");
    stream=id->dev;
    port=va_arg(args,int);
    if((port>stream->data.dev.usb_port_n)||(port==0))
        return DF_ERR_PORT_CORE_TY_USB_PORT_OVER;
	//out("juge_port 结束\n");
    return 0;       
}

static int get_port_maid(struct _ty_usb_fd *id,va_list args)
{   struct _ty_usb  *stream;
    int port;
    uint8 *maid;
    port=va_arg(args,int);
    maid=va_arg(args,uint8 *);
    stream=id->dev;
    if(id==NULL)
        return -1;
    memcpy(maid,stream->data.dev.id,sizeof(stream->data.dev.id));
    return port;    
}

static int get_port_pcb(struct _ty_usb_fd *id,va_list args)
{   struct _ty_usb  *stream;
    int port,i,n;
    uint8 *subid;
    port=va_arg(args,int);
    subid=va_arg(args,uint8 *);
    stream=id->dev;
    if(id==NULL)
        return -1;
    port--;
    for(i=0,n=0;i<stream->data.dev.n;i++,n+=stream->data.dev.pcb[i].n)
    {   if(port<n+stream->data.dev.pcb[i].n)
            break;
        continue;
    }
    port-=n;
    memcpy(subid,stream->data.dev.pcb[i].id,sizeof(stream->data.dev.pcb[0].id));
    port++;
    return port;
}

//static int pd_oper(struct _ty_usb_fd *id,va_list args)
//{   int i;
//    struct _ty_usb  *stream;
//    stream=id->dev;
//    out("运行守护线程\n");
//    _lock_set(stream->lock);
//    _lock_un(stream->lock);
//    for(i=0;i<stream->data.dev.usb_port_n;i++)
//    {   _lock_set(stream->data.dev.port_lock[i]);
//        _lock_un(stream->data.dev.port_lock[i]);
//        
//    }
//    return 1;
//}





/*======================================辅助函数====================================================*/
static int get_port_busid(struct _ty_usb  *stream,int port,char *busid)
{   int i,n,len;
    //out("获取对应端口的busid\n");
    port--;

    for(i=0,n=0;i<stream->data.dev.n;i++,n+=stream->data.dev.pcb[i].n)
    {   if(port<n+stream->data.dev.pcb[i].n)
            break;
        continue;
    }
    memcpy(busid,stream->data.dev.busid,strlen(stream->data.dev.busid));
    len=strlen(busid);
    busid[len++]='.';
	//out("stream->type = %d\n",stream->type);
	if((stream->type == 1) || (stream->type == 4)||(stream->type == 6))
	{
		busid[len++]='1'+i;
		busid[len++]='.';//金税盘机器
	}
	else if(stream->type == 3)
	{
		if(port==0)
		{	sprintf(busid,"2-1.4");
		}
		else if(port==1)
		{	sprintf(busid,"2-1.3");
		}
		else if(port==2)
		{	sprintf(busid,"2-1.2");
		}
		else if(port==3)
		{	sprintf(busid,"2-1.1");
		}
		else if(port==4)
		{	sprintf(busid,"2-1.5");
		}
		else if(port==5)
		{	sprintf(busid,"2-1.6");
		}
		else if(port==6)
		{	sprintf(busid,"2-1.7.4");
		}
		else if(port==7)
		{	sprintf(busid,"2-1.7.3");
		}
		//out("busid=[%s]\n",busid);
		return 0;
	}
	else if(stream->type == 5)
	{
		if(port == 0)
		{	sprintf(busid,"1-1.1");
		}
		else if(port == 1)
		{	sprintf(busid,"1-1.2");
		}
		return 0;
	}
	else if (stream->type == 7)
	{
		
		if (port == 0)
		{
			sprintf(busid, "1-1.1");
		}
		else if (port == 1)
		{
			sprintf(busid, "1-1.2");
		}
		else if (port == 2)
		{
			sprintf(busid, "1-1.3");
		}
		else if (port == 3)
		{
			sprintf(busid, "1-1.4");
		}
		return 0;
	}
	else if (stream->type == 8)
	{
		memcpy(busid, busid_20[port], strlen(busid_20[port]));
		return 0;
	}
	else if (stream->type == 9)
	{
		memcpy(busid, busid_120[port], strlen(busid_120[port]));
		return 0;
	}
	else if (stream->type == 10)
	{
		memcpy(busid, busid_240[port], strlen(busid_240[port]));
		return 0;
	}
	else if (stream->type == 11)			//wang busid
	{
		memcpy(busid, busid_60[port], strlen(busid_60[port]));
		return 0;
	}
	//#if MB1601_OR_MB1602
	////out("此为MB1601，多一级hub\n");
 //   busid[len++]='1'+i;
 //   busid[len++]='.';//金税盘机器
	//#endif
	if (stream->type == 4)
	{	//out("MB1603C(1910 2110)机型端口在HUB上排列方式不同\n");
		memcpy(busid, busid_100[port], strlen(busid_100[port]));
		return 0;
	}
    port-=n;

	//if(stream->type == 4)
	//{	//out("MB1603C机型端口在HUB上排列方式不同\n");
	//	memcpy(busid+len,usb_port_busid_a33[port],strlen(usb_port_busid_a33[port]));
	//	return 0;
	//}
    memcpy(busid+len,usb_port_busid[port],strlen(usb_port_busid[port]));
    //out("busid=[%s]\n",busid);
    return 0;
}

static int wait_usb_file_no(char *busid,int enum_timer)
{   int i;
	char buisdd[20];
	sprintf(buisdd,"%s:1.0",busid);
    enum_timer=enum_timer/100;    
    for(i=0;i<enum_timer;i++,usleep(100000))
    {   if(_usb_file_ls_busid(buisdd)==0)
            return 0; 
    }
    out("在单位时间内还未等到该设备消失\n",enum_timer*100);
    return DF_ERR_PORT_CORE_TY_USB_CLOSE;  
}

static int get_usb_file(char *busid,int enum_timer,int mach_type)
{   char main_hub[DF_TY_USB_BUSID];
    char pcb_hub[DF_TY_USB_BUSID];
    char port_hub[DF_TY_USB_BUSID];
	if (mach_type == 3 || mach_type == 5 || mach_type == 7 || mach_type == 8)
	{
		return get_busid_over_timer(busid,enum_timer);
	}


    memset(port_hub,0,sizeof(port_hub));
    memset(pcb_hub,0,sizeof(pcb_hub));
    memset(main_hub,0,sizeof(main_hub));
    memcpy(port_hub,busid,strlen(busid)-2);
    memcpy(pcb_hub,busid,strlen(port_hub)-2);
    memcpy(main_hub,busid,strlen(pcb_hub)-2);

	if(mach_type == 1)
	{
	//#if MB1601_OR_MB1602
		out("此为mb1601会检测主HUB\n");
		if(get_busid_over_timer(main_hub,enum_timer)<0)
		{   out("单位时间内没有找到主HUB系统要求要重新启动\n");
			return DF_ERR_PORT_CORE_TY_USB_MAIN_HUB;
		}
	}
	//#endif
    if(get_busid_over_timer(pcb_hub,enum_timer)<0)
    {   out("没有找到单板的HUB\n");
        return DF_ERR_PORT_CORE_TY_USB_PCB_HUB;
    }
    if(get_busid_over_timer(port_hub,enum_timer)<0)
    {   out("没有找到端口的HUB\n");
        return DF_ERR_PORT_CORE_TY_USB_PORT_NO_ENUM;
    }
    return get_busid_over_timer(busid,9000);
}

static int get_busid_over_timer(const char *busid,int enum_timer)
{   struct _usb_file_dev    usb_file;
    int i;
    enum_timer=enum_timer/100;    
    for(i=0;i<enum_timer;i++,usleep(100000))
    {   memset(&usb_file,0,sizeof(usb_file));
        if(_usb_file_get_infor(busid,&usb_file)<0)
            continue;
        return 0;    
    }
    out("在[%d]时间内未找到设备\n",enum_timer*100);
    return DF_ERR_PORT_CORE_TY_USB_NO_FIND_DEV;   
}

static int get_port(struct _ty_usb  *stream,int port)
{   int sub_s,result,i;
    if(stream->port_n==0)
        return port;
    sub_s=(port-1);
    result=(sub_s%35);
    i=sub_s-result;
    port=i+(35-result);
    return port;
}
