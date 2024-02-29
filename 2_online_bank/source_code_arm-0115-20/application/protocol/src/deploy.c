#include "../inc/deploy.h"

struct _deploy  *fd_stream = NULL;

static int deploy_open(struct ty_file	*file);  
static int deploy_ctl(void *data,int fd,int cm,va_list args); 
static int deploy_close(void *data,int fd);    
static int set_busid(struct _deploy_fd  *id,va_list args);
static int set_ip_ip(struct _deploy_fd  *id,va_list args);
static int set_enum_timer(struct _deploy_fd  *id,va_list args);
static int set_port_n(struct _deploy_fd  *id,va_list args);
static int set_start_port(struct _deploy_fd  *id,va_list args);
static int set_vpn_par(struct _deploy_fd  *id,va_list args);
static int get_wifi_par(struct _deploy_fd  *id, va_list args);
static int get_net_par(struct _deploy_fd  *id,va_list args);		
static int get_busid(struct _deploy_fd  *id,va_list args);
static int get_port_n(struct _deploy_fd  *id,va_list args);
static int get_enum_timer(struct _deploy_fd  *id,va_list args);
static int get_ip_ip(struct _deploy_fd  *id,va_list args);
static int get_start_port(struct _deploy_fd  *id,va_list args);
static int get_net_soft(struct _deploy_fd  *id,va_list args);
static int get_vpn_par(struct _deploy_fd  *id,va_list args);
static int get_close_update(struct _deploy_fd  *id,va_list args);
static int set_auth(struct _deploy_fd  *id, va_list args);
static void reset_systerm(void *arg);
static void set_ip_q(void *arg);
static int update_ip_ip(struct _deploy_fd *id,va_list args);
static int write_power_mode_flash(struct _deploy_fd  *id, va_list args);
static int read_power_mode_flash(struct _deploy_fd  *id, va_list args);

//默认为A33配置,判断传入硬件类型后修改
static struct _deploy_file soft_cut =
{
	.edit=DF_DEPLOY_PAR_EDIT_2,         //版本号
	.port_n=0,                          //默认排列顺序
	.enum_timer=5,                      //默认枚举时间
	.busid={'1','-','1'},       		//U盘升级地址             
	.start_port=1,						//起始端口号为1
	.ip=0,                              //默认不设置ip
	.mask=0,                            //子网掩码
	.getway=0,                          //网关
	.DNS=0,								//默认为0
	.ip_type=0,							//默认IP类型为固定IP
	.net_soft="host=[156d721f79.iok.la],user=[njmb],pwd=[123456],timer=[0]",
	.vpn_enable=0,
	.vpn_mode=0,
	.vpn_ftp_port=21,
	.vpn_ftp_user="vpnuser",
	.vpn_ftp_passwd="vpnuser123",
	.close_auto_update=0
};

static const struct _deloy_ctl_fu ctl_fun[]=
{
	{   DF_DEPLOY_CM_SET_BUSID,         set_busid       },
	{   DF_DEPLOY_CM_SET_ENUM_TIMER,    set_enum_timer  },
	{   DF_DEPLOY_CM_SET_PORT_N,        set_port_n      },
	{   DF_DEPLOY_CM_SET_IP,            set_ip_ip       },
	{   DF_DEPLOY_CM_SET_START_PORT,    set_start_port  },
	{	DF_DEPLOY_CM_SET_VPN_PAR,		set_vpn_par		},
	{   DF_DEPLOY_CM_GET_BUSID,         get_busid       },
	{   DF_DEPLOY_CM_GET_PORT_N,        get_port_n      },
	{   DF_DEPLOY_CM_GET_ENUM_TIMER,    get_enum_timer  },
	{   DF_DEPLOY_CM_GET_IP,            get_ip_ip       },
	{   DF_DEPLOY_CM_GET_START_PORT,    get_start_port  },
	{   DF_DEPLOY_CM_GET_NET_SOFT,      get_net_soft    },
	{	DF_DEPLOY_CM_GET_WIFI_PAR,		get_wifi_par	},
	{   DF_DEPLOY_CM_GET_NET_PAR,       get_net_par     },
	{	DF_DEPLOY_CM_SET_IP_UPDATE,		update_ip_ip	},
	{	DF_DEPLOY_CM_GET_VPN_PAR,		get_vpn_par		},
	{	DF_DEPLOY_CM_GET_CLOSE_UPDATE,	get_close_update},
	{	DF_DEPLOY_CM_SET_AUTH,			set_auth },
	{	DF_DEPLOY_CM_SET_TER_POW_MODE,	write_power_mode_flash },
	{	DF_DEPLOY_CM_GET_TER_POW_MODE,	read_power_mode_flash },
};

static const struct _file_fuc	deploy_fuc=
{
	.open=deploy_open,
	.read=NULL,
	.write=NULL,
	.ctl=deploy_ctl,
	.close=deploy_close,
	.del=NULL
};

int deploy_add(_so_note    *note,const char *switch_name,const char *usb_name,int hard_type)
{
	struct _deploy *stream;
	int result;
	if ((hard_type == 1) || (hard_type == 2) || (hard_type == 3))
	{
		deploy_out("A33和MTK使用默认配置");
	}
	else
	{
		#define HARD_TYPE_A20
		memset(soft_cut.busid,0,sizeof(soft_cut.busid));
		sprintf(soft_cut.busid,"2-1");
	}
	
	stream=malloc(sizeof(struct _deploy));
	if(stream==NULL)
		return DF_ERR_MEM_ERR;
	memset(stream,0,sizeof(struct _deploy));
	stream->note = note;
	stream->hardtype = hard_type;
    //deploy_out("加载:[deploy_add]模块\n");
    memcpy(stream->switch_name,switch_name,strlen(switch_name));
    memcpy(stream->ty_usb_name,usb_name,strlen(usb_name));
	//memcpy(stream->net_time_name,net_time,strlen(net_time));
	result = ty_file_add(DF_DEV_DEPLOY, stream, "deploy", (struct _file_fuc *)&deploy_fuc);
	fd_stream = stream;
	if(result<0)
		free(stream);
    //deploy_out("加载:[deploy_add]成功\n");
	return result;    
}

static int _mac_restart(struct _deploy *stream)
{
	int result,try_count = 0;
	int i;

	system("ifconfig wlan0 up");

	deploy_out("检测网络是否已经就绪\n");
	result = get_socket_net_state("eth0");
	if (result < 0)
	{
		//eth0驱动未加载
		deploy_out("网络检测失败,即将复位MAC\n");
		goto restart;
	}
	else if (result == 0)
	{
		//eth0网线离线或ifconfig未启用
		deploy_out("网络状态为离线,即将复位MAC\n");
		goto restart;
	}
	else if (result == 1)
	{
		deploy_out("网络正常,无需复位MAC\n");
		return 0;
	}

	deploy_out("延时5秒\n");
	sleep(5);

restart:
	//deploy_out("延时5秒\n");

	for (i = 0; i < 1000; i++)
	{
		if (access("/sys/bus/usb/devices/3-0:1.0", 0) == 0)
		{
			break;
		}
		usleep(10000);
		continue;
	}
	if (i == 1000)
	{
		logout(INFO, "SYSTEM", "INIT", "检测到OTG总线未正常加载即将重启机柜\r\n");
		return -1;
	}
	//sleep(1);
	result = ty_ctl(stream->switch_fd, DF_SWITCH_CM_MAC_RESTART_POWER);
	result = ty_ctl(stream->switch_fd, DF_SWITCH_CM_MAC_RESTART_POWER);
	if (result < 0)
	{
		logout(INFO, "SYSTEM", "INIT", "操作MAC芯片复位失败\r\n");
		return result;
	}
	for (i = 0; i < 10000; i++)
	{
		result = get_socket_net_state("eth0");
		if (result >= 0)
		{
			deploy_out("eth0网络正常\n");
			break;
		}
		usleep(10000);
		continue;
	}
	if (i == 1000)
	{
		logout(INFO, "SYSTEM", "INIT", "网卡复位后文件系统查询eth0网卡设备失败\r\n");
		try_count++;
		if(try_count < 3)
		{
			goto restart;
		}
		return -1;
	}

	deploy_out("MAC复位完成\n");
	return 0;
}

static int deploy_open(struct ty_file	*file)
{   struct _deploy  *stream;
    int result,j;
	//int fd;
	//char name[DF_FILE_NAME_MAX_LEN];
	//uint8 gbuf[1024];
	stream=file->pro_data;
	if((stream==NULL))	
		return DF_ERR_PORT_CORE_FD_NULL;
    //deploy_out("打开deploy_open模块\n");
    if(stream->state==0)
    {   //deploy_out("第一次打开配置文件\n");
	
        stream->switch_fd=ty_open(stream->switch_name,0,0,NULL);
        if(stream->switch_fd<0)
        {   
			deploy_out("打开开关失败\n");
            return stream->switch_fd;
        }
        stream->ty_usb_fd=ty_open(stream->ty_usb_name,0,0,NULL);
        if(stream->ty_usb_fd<0)
        {   
			deploy_out("打开USB[%s]模块失败\n",stream->ty_usb_name);
            ty_close(stream->switch_fd);
            return stream->ty_usb_fd;
        }
		result = ty_ctl(stream->switch_fd,DF_SWITCH_CM_READ_FLASH,&stream->file,sizeof(stream->file));
		if(result >= 0)
		{
			//logout(INFO, "SYSTEM", "INIT","获取单片机中FLASH参数成功：协议版本:%d,IP类型:%d,IP:%08x,MASK:%08x,GATE:%08x,DNS:%08x,UPDATEADDR:%s\r\n",stream->file.edit,stream->file.ip_type,stream->file.ip,stream->file.mask,stream->file.getway,stream->file.DNS,stream->file.net_soft);
		}
		else
		{	
			logout(INFO, "SYSTEM", "INIT", "启动第一次获取单片机FLASH失败,即将再次获取\r\n");
			sleep(1);
			result = ty_ctl(stream->switch_fd,DF_SWITCH_CM_READ_FLASH,&stream->file,sizeof(stream->file));
			
			if(result >= 0)
			{	
				logout(INFO, "SYSTEM", "INIT", "获取单片机中FLASH参数成功：协议版本:%d,IP类型:%d,IP:%08x,MASK:%08x,GATE:%08x,DNS:%08x,UPDATEADDR:%s\r\n", stream->file.edit, stream->file.ip_type, stream->file.ip, stream->file.mask, stream->file.getway, stream->file.DNS, stream->file.net_soft);
			}
			else
			{
				logout(INFO, "SYSTEM", "INIT", "启动第二次获取单片机FLASH失败,即将再次获取\r\n");
				sleep(2);
				result = ty_ctl(stream->switch_fd,DF_SWITCH_CM_READ_FLASH,&stream->file,sizeof(stream->file));
				if(result >= 0)
				{
					logout(INFO, "SYSTEM", "INIT", "获取单片机中FLASH参数成功：协议版本:%d,IP类型:%d,IP:%08x,MASK:%08x,GATE:%08x,DNS:%08x,UPDATEADDR:%s\r\n", stream->file.edit, stream->file.ip_type, stream->file.ip, stream->file.mask, stream->file.getway, stream->file.DNS, stream->file.net_soft);
				}
				else
				{
					logout(INFO, "SYSTEM", "INIT", "启动第三次获取单片机FLASH失败,即将重启程序\r\n");
					return -1;
				}
			}
		}
		if (stream->file.edit == 0xffffffff)
		{
			logout(INFO, "SYSTEM", "INIT", "读取单片机flash数据为0xff,为恢复出厂设置后的数据\r\n");
			sleep(1);
			struct _deploy_file     file;
			memset(&file, 0, sizeof(struct _deploy_file));
			ty_ctl(stream->switch_fd, DF_SWITCH_CM_READ_FLASH, &file, sizeof(file));

			sleep(2);
			memset(&file, 0, sizeof(struct _deploy_file));
			ty_ctl(stream->switch_fd, DF_SWITCH_CM_READ_FLASH, &file, sizeof(file));
		}
		else if (stream->file.edit == 0x00000000)
		{
			logout(INFO, "SYSTEM", "INIT", "读取单片机flash数据为0x00,可能为通讯异常\r\n");
			sleep(1);
			struct _deploy_file     file;
			memset(&file, 0, sizeof(struct _deploy_file));
			ty_ctl(stream->switch_fd, DF_SWITCH_CM_READ_FLASH, &file, sizeof(file));

			sleep(2);
			memset(&file, 0, sizeof(struct _deploy_file));
			ty_ctl(stream->switch_fd, DF_SWITCH_CM_READ_FLASH, &file, sizeof(file));
		}
		else
		{
			logout(INFO, "SYSTEM", "INIT", "读取单片机flash数据中版本号为：%02x\r\n", stream->file.edit);
		}


        //deploy_out("获取参数成功\n");
		int tmp_close_auto_update = stream->file.close_auto_update;//最后一位现在作为是否强制非节电标志
        if(stream->file.edit!=DF_DEPLOY_PAR_EDIT_2)
        {   
			deploy_out("参数版本不一致\n");			
            if(stream->file.edit==DF_DEPLOY_PAR_EDIT_ODE)
            {   
				deploy_out("1.0版本的参数\n");
				stream->file.edit = DF_DEPLOY_PAR_EDIT_2;
				stream->file.getway=0;
                stream->file.mask=0;
            }
			else if(stream->file.edit == DF_DEPLOY_PAR_EDIT)
			{
				deploy_out("2.0版本的参数\n");
				stream->file.edit = DF_DEPLOY_PAR_EDIT_2;
				stream->file.vpn_enable = 0;
				stream->file.vpn_mode = 0;
				memset(stream->file.vpn_addr,0,sizeof(stream->file.vpn_addr));
				stream->file.vpn_port = 0;
				memset(stream->file.vpn_user,0,sizeof(stream->file.vpn_user));
				memset(stream->file.vpn_passwd,0,sizeof(stream->file.vpn_passwd));
				stream->file.vpn_ftp_port = 21;
				memset(stream->file.vpn_ftp_user,0,sizeof(stream->file.vpn_ftp_user));
				sprintf(stream->file.vpn_ftp_user,"vpnuser");
				memset(stream->file.vpn_ftp_passwd,0,sizeof(stream->file.vpn_ftp_passwd));
				sprintf(stream->file.vpn_ftp_passwd,"vpnuser123");
				stream->file.close_auto_update = 0;
			}
            else
            {   
				memcpy(&stream->file,&soft_cut,sizeof(stream->file));
            }      
        }
		stream->file.close_auto_update = tmp_close_auto_update;//最后一位现在作为是否强制非节电标志
        //deploy_out("设置ty_usb的BUSID[%s]\n",stream->file.busid);
        ty_ctl(stream->ty_usb_fd,DF_TY_USB_CM_SET_BUSID,stream->file.busid);
        //deploy_out("设置端口的排列顺序:%d\n",stream->file.port_n);
        ty_ctl(stream->ty_usb_fd,DF_TY_USB_CM_SET_PORT_N,stream->file.port_n);
       // deploy_out("设置最大枚举时间:%d秒\n",stream->file.enum_timer);
        ty_ctl(stream->ty_usb_fd,DF_TY_USB_CM_SET_ENUM_TIMER,stream->file.enum_timer*1000);

		int usb_port_numb = ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_PORT_NUMBER);
		// int port;
		// for (port = 1; port <= usb_port_numb; port++)
		// {
		// 	//deploy_out("[ty_usb_m.c]按端口号打开端口 port = %d\n",i);
		// 	ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_PORT_CLOSE_PORT_POWER, port);
		// 	usleep(10000);

		// }

		//logout(INFO, "SYSTEM", "INIT","所有端口电源关闭成功\r\n");
		//sleep(2);

		if ((stream->hardtype == 1) || (stream->hardtype == 3))
		{
			_mac_restart(stream);
		}
		
       // deploy_out("设置ip地址\n");
		struct _net_define  net_par;
		memset(&net_par,0,sizeof(struct _net_define));
        net_par.ip=stream->file.ip;
        net_par.mask=stream->file.mask;		
		net_par.gateway[0]=stream->file.getway;
		net_par.DNS=stream->file.DNS;
		net_par.type=stream->file.ip_type;


		//deploy_out("读到单片机FLASH中IP为：%08x \n",net_par.ip);
		//deploy_out("读到单片机FLASH中mask为：%08x \n",net_par.mask);
		//deploy_out("读到单片机FLASH中gateway为：%08x \n",net_par.gateway[0]);
		//deploy_out("读到单片机FLASH中DNS为：%08x \n",net_par.DNS);
		//deploy_out("读到单片机FLASH中IP类型为 %d\n",net_par.type);
		if(net_par.type==0)
		{
			//deploy_out("IP此为第一次开机\n");

			logout(INFO, "SYSTEM", "INIT", "单片机IP参数未设置,或IP已恢复出厂设置\r\n");

			if (file_exists("/etc/wifi_config") != 0)
			{
				delete_dir("/etc/wifi_config");
			}

// 			if((net_par.ip==0)&&(net_par.mask==0)&&(net_par.gateway[0]==0)&&(net_par.DNS==0))
// 			{	
			//deploy_out("net_par.type==0 单片机IP参数未设置，设置IP为主板默认值192.168.0.250\n");
			stream->file.ip=0xC0A800FA;
			stream->file.mask=0xFFFFFF00;		
			stream->file.getway=0xC0A80001;
			stream->file.DNS=0x72727272;
			stream->file.ip_type=1;
			write_log("回出厂，删除mac_address文件\n");
			delete_file(DF_TER_MAC_ADDR_FILE);
			sync();
			ty_ctl(stream->switch_fd,DF_SWITCH_CM_WRITE_FLASH,&stream->file,sizeof(stream->file));
			_delay_task_add("set_ip",set_ip_q,stream,100); 

		}
		else if (net_par.type == 1)
		{
			deploy_out("需要设置成DHCP\n");
			set_net(DF_NET_DEV, &net_par);
		}
		else if(net_par.type==2)
		{
			//deploy_out("需要设置固定IP\n");
			if((net_par.ip==0)&&(net_par.mask==0)&&(net_par.gateway[0]==0)&&(net_par.DNS==0))
			{
				//deploy_out("net_par.type==2 单片机IP参数未设置，设置IP为主板默认值192.168.0.250\n");
				stream->file.ip=0xC0A800FA;
				stream->file.mask=0xFFFFFF00;		
				stream->file.getway=0xC0A80001;
				stream->file.DNS=0xC0A80001;
				stream->file.ip_type=2;
				ty_ctl(stream->switch_fd,DF_SWITCH_CM_WRITE_FLASH,&stream->file,sizeof(stream->file));
				_delay_task_add("set_ip",set_ip_q,stream,100); 				
			}
			else 
			{	deploy_out("单片机FLASH中有数据，正常使用\n");
				set_net(DF_NET_DEV,&net_par);
			}
		}
        stream->lock=_lock_open("deloy.c",NULL);
		//ftp改成按键升级后不需要开机检查更新

		//stream->ftp_task=_scheduled_task_open("_deploy.c",deploy_task,stream,DF_DEPLOY_TASK_TIMER,NULL);
		
    }
    for(j=0;j<sizeof(stream->fd)/sizeof(stream->fd[0]);j++)
	{	if(stream->fd[j].state==0)
			break;
	}
	if(j==sizeof(stream->fd)/sizeof(stream->fd[0]))
	{	
		return DF_ERR_PORT_CORE_SLAVE_SPACE;
	}
	memset(&stream->fd[j],0,sizeof(stream->fd[0]));
	stream->fd[j].dev=stream; 
	stream->fd[j].state=1; 
	stream->fd[j].mem_fd=&file->fd[j];
	stream->state++;	
    deploy_out("打开deploy_open成功:j=%d\n",j);
	return j+1;    
}

static int deploy_ctl(void *data,int fd,int cm,va_list args)
{   
	struct _deploy  *stream;
    struct _deploy_fd  *id;
    int result;
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
    {   
		if(cm==ctl_fun[i].cm)
        {   
			_lock_set(stream->lock);
            result=ctl_fun[i].ctl(id,args);
            _lock_un(stream->lock);
            return result;
        }    
    }
    return DF_ERR_PORT_CORE_CM;    
}

static int deploy_close(void *data,int fd)
{   
	struct _deploy  *stream;
    struct _deploy_fd  *id;
    //int j;
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
    {   _scheduled_task_close(stream->ftp_task);
        ty_close(stream->switch_fd);
        ty_close(stream->ty_usb_fd);
        ty_close(stream->ftp_fd);
        _lock_close(stream->lock);
    }
    return 0;
}

/*==========================================ctl执行函数==============================================*/
static int get_busid(struct _deploy_fd  *id,va_list args)
{   
	char *busid;
    busid=va_arg(args,char *);
    if(busid==NULL)
        return -1;
    memcpy(busid,id->dev->file.busid,strlen(id->dev->file.busid));
    //deploy_out("deploy:get_busid=[%s]\n",busid);
    return strlen(id->dev->file.busid);
}

static int get_port_n(struct _deploy_fd  *id,va_list args)
{   //deploy_out("deploy:get_port_n=[%d]\n",id->dev->file.port_n);
    return id->dev->file.port_n;    
}

static int get_enum_timer(struct _deploy_fd  *id,va_list args)
{   //deploy_out("deploy:get_enum_timer=[%d]\n",id->dev->file.enum_timer);
    return id->dev->file.enum_timer;    
}

static int get_ip_ip(struct _deploy_fd  *id,va_list args)
{   //deploy_out("deploy:get_ip_ip=[%d]\n",id->dev->file.ip);
	struct _net_define   net_par;
    //uint32 *ip;
	get_net(DF_NET_DEV,&net_par);
	return net_par.ip;    
}

static int get_start_port(struct _deploy_fd  *id,va_list args)
{   //deploy_out("deploy:get_start_port=[%d]\n",id->dev->file.start_port);
    return id->dev->file.start_port;    
}

static int get_vpn_par(struct _deploy_fd  *id,va_list args)
{	
	struct _vpn_deploy *vpn_deploy;

	vpn_deploy = va_arg(args,struct _vpn_deploy *);

	vpn_deploy->enable = id->dev->file.vpn_enable;
	vpn_deploy->mode = id->dev->file.vpn_mode;
	vpn_deploy->port = id->dev->file.vpn_port;
	vpn_deploy->ftp_port = id->dev->file.vpn_ftp_port;
	memcpy(vpn_deploy->addr,id->dev->file.vpn_addr,sizeof(vpn_deploy->addr));
	memcpy(vpn_deploy->user,id->dev->file.vpn_user,sizeof(vpn_deploy->user));
	memcpy(vpn_deploy->passwd,id->dev->file.vpn_passwd,sizeof(vpn_deploy->passwd));
	memcpy(vpn_deploy->ftp_user,id->dev->file.vpn_ftp_user,sizeof(vpn_deploy->ftp_user));
	memcpy(vpn_deploy->ftp_passwd,id->dev->file.vpn_ftp_passwd,sizeof(vpn_deploy->ftp_passwd));
	return 0;
}

static int get_close_update(struct _deploy_fd  *id,va_list args)
{   //deploy_out("deploy:get_start_port=[%d]\n",id->dev->file.start_port);
    return id->dev->file.close_auto_update;    
}

static int set_auth(struct _deploy_fd  *id, va_list args)
{
	struct _deploy  *stream;
	stream = id->dev;
	int port_n = 0;
	port_n = va_arg(args, int);
	stream->file.port_n = port_n;
	if (ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_SET_PORT_N, stream->file.port_n)<0)
		return -1;
	ty_ctl(stream->switch_fd, DF_SWITCH_CM_WRITE_FLASH, &stream->file, sizeof(stream->file));
	return 0;
}

static int get_net_soft(struct _deploy_fd  *id,va_list args)
{   
	char *net_soft;
    net_soft=va_arg(args,char *);
    if(net_soft==NULL)
        return -1;
    memcpy(net_soft,id->dev->file.net_soft,strlen(id->dev->file.net_soft));
    deploy_out("deploy:get_net_soft=[%s]\n",net_soft);
    return strlen(id->dev->file.net_soft);
}

static int set_busid(struct _deploy_fd  *id,va_list args)
{   char *busid;
    char name[DF_FILE_NAME_MAX_LEN];
    int len,fd;
    struct _deploy  *stream;
    busid=va_arg(args,char *);
    stream=id->dev;
    len=strlen(busid);
    deploy_out("deploy:set_busid=[%s]\n",busid);
    if(len==strlen(id->dev->file.busid))
    {   if(memcmp(busid,id->dev->file.busid,len)==0)
        {   deploy_out("deploy:更原来的一样\n");
            return 0;
        }    
    }
    memset(stream->file.busid,0,sizeof(stream->file.busid));
    memcpy(stream->file.busid,busid,strlen(busid));
    if(ty_ctl(stream->ty_usb_fd,DF_TY_USB_CM_SET_BUSID,stream->file.busid)<0)
        return -1;
    deploy_out("deploy:set_busid=[%s] to switch_fd\n",busid);
    ty_ctl(stream->switch_fd,DF_SWITCH_CM_WRITE_FLASH,&stream->file,sizeof(stream->file));
    memset(name,0,sizeof(name));
    sprintf(name,"/dev/%s",DF_EVENT_FILE_NAME);
    fd=ty_open(name,0,0,NULL);       
    deploy_out("写事件\n");
    ty_ctl(fd,DF_EVENT_NAME_RESET,DF_EVENT_REASON_SET_BUSID);
    ty_close(fd);
    deploy_out("deploy:set_busid=[%s] to suc\n",busid);
    _delay_task_add("reset",reset_systerm,stream,10); 
    return 0;
}

static int update_ip_ip(struct _deploy_fd *id,va_list args)
{
	struct _deploy  *stream;	
	struct _net_define  net_par;
	stream=id->dev;
	memset(&net_par,0,sizeof(struct _net_define));
	net_par.ip=stream->file.ip;
	net_par.mask=stream->file.mask;		
	net_par.gateway[0]=stream->file.getway;
	net_par.DNS=stream->file.DNS;
	net_par.type=stream->file.ip_type;
	deploy_out("读到单片机FLASH中IP为：%08x \n",net_par.ip);
	//deploy_out("读到单片机FLASH中mask为：%08x \n",net_par.mask);
	//deploy_out("读到单片机FLASH中gateway为：%08x \n",net_par.gateway[0]);
	//deploy_out("读到单片机FLASH中DNS为：%08x \n",net_par.DNS);
	//deploy_out("读到单片机FLASH中IP类型为 %d\n",net_par.type);
	if(net_par.type==2)
	{	if((net_par.ip==0)&&(net_par.mask==0)&&(net_par.gateway[0]==0)&&(net_par.DNS==0))
		{
			deploy_out("网络参数为配置，不进行设置\n");
			return -1;
		}
		set_net(DF_NET_DEV,&net_par);
	}
	else if(net_par.type == 1)
		set_net(DF_NET_DEV, &net_par);
	//deploy_out("IP配置完成\n");
	return 0;
}

static int set_ip_ip(struct _deploy_fd  *id,va_list args)
{   
	struct _deploy  *stream;
    uint32 ip,mask,getway,DNS;
	int ip_type;
    stream=id->dev;
    ip=va_arg(args,uint32);
    mask=va_arg(args,uint32);
    getway=va_arg(args,uint32);
	DNS=va_arg(args,uint32);
	ip_type=va_arg(args,uint32);
	#if 1
	
	if(ip_type==1)
	{	
		deploy_out("要设置的IP为固定IP\n");
		
		if((stream->file.ip==ip)&&(stream->file.mask==mask)&&(stream->file.getway==getway)&&(stream->file.DNS==DNS)&&(stream->file.ip_type==2))
		{   
			deploy_out("与原来设置一样\n");
			//return 0;
		}
		else
		{
			stream->file.ip=ip;
			stream->file.mask=mask;
			stream->file.getway=getway;
			stream->file.DNS=DNS;	
			stream->file.ip_type=2;	//2为固定IP		
			deploy_out("写FLASH\n");
			ty_ctl(stream->switch_fd,DF_SWITCH_CM_WRITE_FLASH,&stream->file,sizeof(stream->file));
			deploy_out("写FLASH成功\n");
		}
	}
	else if(ip_type==0)
	{	
		deploy_out("要设置的IP为DHCP\n");
		stream->file.ip_type=1; //1为DHCP
		deploy_out("写FLASH\n");
		
		ty_ctl(stream->switch_fd,DF_SWITCH_CM_WRITE_FLASH,&stream->file,sizeof(stream->file));
		
	}
    
	#endif
	//logout(INFO, "SYSTEM", "INIT","Set network\r\n",ip_type,ip,mask,getway,DNS);
    _delay_task_add("set_ip",set_ip_q,stream,100); 
    return 0;  
}

//int set_ip_by_overall(uint32 ip,uint32 mask,uint32 getway,uint32 DNS,int ip_type)
//{   
//	struct _deploy  *stream;
//	
//    stream = fd_stream;
//
//	#if 1
//
//	if(ip_type==1)
//	{	
//		deploy_out("要设置的IP为固定IP\n");
//		if((stream->file.ip==ip)&&(stream->file.mask==mask)&&(stream->file.getway==getway)&&(stream->file.DNS==DNS)&&(stream->file.ip_type==2))
//		{   
//			deploy_out("与原来设置一样\n");
//			//return 0;
//		}
//		else
//		{
//			stream->file.ip=ip;
//			stream->file.mask=mask;
//			stream->file.getway=getway;
//			stream->file.DNS=DNS;	
//			stream->file.ip_type=2;	//2为固定IP		
//			deploy_out("写FLASH\n");
//			ty_ctl(stream->switch_fd,DF_SWITCH_CM_WRITE_FLASH,&stream->file,sizeof(stream->file));
//		}
//	}
//	else if(ip_type==0)
//	{	
//		deploy_out("要设置的IP为DHCP\n");
//		stream->file.ip_type=1; //1为DHCP
//		deploy_out("写FLASH\n");
//		ty_ctl(stream->switch_fd,DF_SWITCH_CM_WRITE_FLASH,&stream->file,sizeof(stream->file));
//	}
//    
//	#endif
//    _delay_task_add("set_ip",set_ip_q,stream,100); 
//    return 0;  
//}

static int set_enum_timer(struct _deploy_fd  *id,va_list args)
{   
	struct _deploy  *stream;
    int enum_timer;
    stream=id->dev;
    enum_timer=va_arg(args,int);
    deploy_out("deploy:set_enum_timer=[%d]\n",enum_timer);
    if(stream->file.enum_timer==enum_timer)
    {   
		deploy_out("set_enum_timer:跟原来的一样\n");
        return 0;
    }
    stream->file.enum_timer=enum_timer;
    deploy_out("deploy:set_enum_timer=[%d] to switch_fd\n",enum_timer);
    ty_ctl(stream->switch_fd,DF_SWITCH_CM_WRITE_FLASH,&stream->file,sizeof(stream->file));
    return ty_ctl(stream->ty_usb_fd,DF_TY_USB_CM_SET_ENUM_TIMER,enum_timer*1000);     
}

static int set_port_n(struct _deploy_fd  *id,va_list args)
{   
	struct _deploy  *stream;
    int port_n;
    stream=id->dev;
    port_n=va_arg(args,int);
    deploy_out("deploy:set_port_n=[%d]\n",port_n);
    if(stream->file.port_n==port_n)
    {   
		deploy_out("set_port_n:跟原来的一样\n");
        return 0;
    }
    stream->file.port_n=port_n;
    ty_ctl(stream->switch_fd,DF_SWITCH_CM_WRITE_FLASH,&stream->file,sizeof(stream->file));
    deploy_out("deploy:set_port_n=[%d] to switch_fd\n",port_n);
    return ty_ctl(stream->ty_usb_fd,DF_TY_USB_CM_SET_PORT_N,port_n);
}

static int set_start_port(struct _deploy_fd  *id,va_list args)
{   
	struct _deploy  *stream;
    int start_port;
    start_port=va_arg(args,int);
    stream=id->dev;
    deploy_out("deploy:set_start_port=[%d]\n",start_port);
    if(stream->file.start_port==start_port)
    {   
		deploy_out("set_start_port:跟原来的一样\n");
        return 0;
    }       
    stream->file.start_port=start_port;
    deploy_out("deploy:start_port=[%d] to switch_fd\n",start_port);
	//ty_ctl(stream->switch_fd,DF_SWITCH_CM_WRITE_FLASH,&stream->file,sizeof(stream->file));
    return ty_ctl(stream->switch_fd,DF_SWITCH_CM_WRITE_FLASH,&stream->file,sizeof(stream->file));  
}

static int write_power_mode_flash(struct _deploy_fd  *id, va_list args)
{
	struct _deploy  *stream;
	int mode;
	mode = va_arg(args, int);
	stream = id->dev;

	if (stream->file.close_auto_update == mode)
	{
		deploy_out("write_power_mode_flash:跟原来的一样\n");
		return 0;
	}
	stream->file.close_auto_update = mode;
	printf(" write mode :%d\n", stream->file.close_auto_update);
	ty_ctl(stream->switch_fd, DF_SWITCH_CM_WRITE_FLASH, &stream->file, sizeof(stream->file));
	return 0;
}
static int read_power_mode_flash(struct _deploy_fd  *id, va_list args)
{
	struct _deploy  *stream;
	stream = id->dev;

	printf(" read mode :%d\n", stream->file.close_auto_update);
	return stream->file.close_auto_update;
}

static int set_vpn_par(struct _deploy_fd  *id,va_list args)
{   
	struct _deploy  *stream;
    struct _vpn_deploy *vpn_deploy;
	deploy_out("进入deploy VPN配置函数\n");
	vpn_deploy = va_arg(args,struct _vpn_deploy *);
	deploy_out("判断要设置的VPN参数与内存中是否一致\n");
    stream=id->dev;
	if(stream->file.vpn_enable == vpn_deploy->enable&&stream->file.vpn_mode == vpn_deploy->mode&&stream->file.vpn_port == vpn_deploy->port&&stream->file.vpn_ftp_port == vpn_deploy->ftp_port)
    {   
		if(memcmp(stream->file.vpn_addr,vpn_deploy->addr,sizeof(stream->file.vpn_addr))==0)
		{	
			if(memcmp(stream->file.vpn_user,vpn_deploy->user,sizeof(stream->file.vpn_user))==0)
			{	
				if(memcmp(stream->file.vpn_passwd,vpn_deploy->passwd,sizeof(stream->file.vpn_passwd))==0)
				{	
					if(memcmp(stream->file.vpn_ftp_user,vpn_deploy->ftp_user,sizeof(stream->file.vpn_ftp_user))==0)
					{	
						if(memcmp(stream->file.vpn_ftp_passwd,vpn_deploy->ftp_passwd,sizeof(stream->file.vpn_ftp_passwd))==0)
						{	
							deploy_out("要设置的VPN配置与原来的一样\n");
							return 0;
						}	
					}					
				}
			}
		}
    }       
	stream->file.vpn_enable=vpn_deploy->enable;
	stream->file.vpn_mode=vpn_deploy->mode;
	stream->file.vpn_port=vpn_deploy->port;
	stream->file.vpn_ftp_port=vpn_deploy->ftp_port;
	memcpy(stream->file.vpn_addr,vpn_deploy->addr,sizeof(stream->file.vpn_addr));
	memcpy(stream->file.vpn_user,vpn_deploy->user,sizeof(stream->file.vpn_user));
	memcpy(stream->file.vpn_passwd,vpn_deploy->passwd,sizeof(stream->file.vpn_passwd));
	memcpy(stream->file.vpn_ftp_user,vpn_deploy->ftp_user,sizeof(stream->file.vpn_ftp_user));
	memcpy(stream->file.vpn_ftp_passwd,vpn_deploy->ftp_passwd,sizeof(stream->file.vpn_ftp_passwd));
    return ty_ctl(stream->switch_fd,DF_SWITCH_CM_WRITE_FLASH,&stream->file,sizeof(stream->file));  
}
static int get_wifi_par(struct _deploy_fd  *id, va_list args)
{
	struct _deploy  *stream;
	struct _net_define   net_par;
	uint32 *ip, *mask, *getway, *dns;
	int *type;
	stream = id->dev;
	ip = va_arg(args, uint32 *);
	mask = va_arg(args, uint32 *);
	getway = va_arg(args, uint32 *);
	dns = va_arg(args, uint32 *);
	type = va_arg(args, int *);
	get_net("wlan0", &net_par);
	//deploy_out("获取网络参数\n");
	if (ip != NULL)
	{
		*ip = net_par.ip;

	}
	if (mask != NULL)
	{
		*mask = net_par.mask;

	}
	if (getway != NULL)
	{
		*getway = net_par.gateway[0];

	}
	if (dns != NULL)
	{
		//*dns=stream->file.DNS;
		*dns = net_par.DNS;

	}
	if (type != NULL)
	{
		*type = stream->file.ip_type - 1;

	}
	char tmp[100] = {0};
	memset(tmp, 0, sizeof(tmp));
	ip2ip_asc(*ip, tmp);
	//deploy_out("获取到的wifi IP :%s\n", tmp);

	memset(tmp, 0, sizeof(tmp));
	ip2ip_asc(*mask, tmp);
	//deploy_out("获取到的wifi mask :%s\n", tmp);

	memset(tmp, 0, sizeof(tmp));
	ip2ip_asc(*getway, tmp);
	//deploy_out("获取到的wifi gateway :%s\n", tmp);

	memset(tmp, 0, sizeof(tmp));
	ip2ip_asc(*dns, tmp);
	//deploy_out("获取到的wifi DNS :%s\n", tmp);
	return 0;
}
static int get_net_par(struct _deploy_fd  *id,va_list args)
{   
	struct _deploy  *stream;  
	struct _net_define   net_par;
    uint32 *ip,*mask,*getway,*dns;
	int *type;
    stream=id->dev;
    ip=va_arg(args,uint32 *);
    mask=va_arg(args,uint32 *);
    getway=va_arg(args,uint32 *);
	dns=va_arg(args,uint32 *);
	type = va_arg(args,int *);
	get_net(DF_NET_DEV,&net_par);
    //deploy_out("获取网络参数\n");
    if(ip!=NULL)
    {   
		*ip=net_par.ip;

    }
    if(mask!=NULL)
    {   
		*mask=net_par.mask;

    }
    if(getway!=NULL)
    {   
		*getway=net_par.gateway[0];

    }
	if(dns!=NULL)
    {   
		//*dns=stream->file.DNS;
		*dns = net_par.DNS;

    }
	if(type!=NULL)
    {   
		*type=stream->file.ip_type-1;

    }
    return 0;    
}




//static void deploy_task(void *arg,int timer)
//{   struct _deploy  *stream;
//    stream=arg;
//    if(ty_read(stream->ftp_fd,NULL,0)==1)
//    {   int fd;
//        char name[DF_FILE_NAME_MAX_LEN];
//        deploy_out("ftp有新的软件,将在3秒后重新启动\n");
//		sleep(3);
//        memset(name,0,sizeof(name));
//        sprintf(name,"/dev/%s",DF_EVENT_FILE_NAME);
//        fd=ty_open(name,0,0,NULL);
//        if(fd<0)
//            return ;        
//        deploy_out("写事件\n");
//        ty_ctl(fd,DF_EVENT_NAME_RESET,DF_EVENT_REASON_SOFT);
//        ty_close(fd);
//        memset(name,0,sizeof(name));
//        sprintf(name,"/dev/%s",DF_TY_PD_NAME);
//        fd=ty_open(name,0,0,NULL);
//        ty_ctl(fd,DF_TY_PD_CM_RESET);
//    }
//	else if(ty_read(stream->sd_fd,NULL,0)==1)
//    {   int fd;
//        char name[DF_FILE_NAME_MAX_LEN];
//        deploy_out("sd有新的软件,将在3秒后重新启动\n");
//		sleep(3);
//        memset(name,0,sizeof(name));
//        sprintf(name,"/dev/%s",DF_EVENT_FILE_NAME);
//        fd=ty_open(name,0,0,NULL);
//        if(fd<0)
//            return ;        
//        deploy_out("写事件\n");
//        ty_ctl(fd,DF_EVENT_NAME_RESET,DF_EVENT_REASON_SOFT);
//        ty_close(fd);
//        memset(name,0,sizeof(name));
//        sprintf(name,"/dev/%s",DF_TY_PD_NAME);
//        fd=ty_open(name,0,0,NULL);
//        ty_ctl(fd,DF_TY_PD_CM_RESET);
//    }
//}
static void reset_systerm(void *arg)
{
	// //struct _deploy  *stream;
    // char name[DF_FILE_NAME_MAX_LEN];
    // int fd;
    // //stream=arg;
    // printf("reset_systerm\n");
    // memset(name,0,sizeof(name));
    // sprintf(name,"/dev/%s",DF_TY_PD_NAME);
    // fd=ty_open(name,0,0,NULL);
    // ty_ctl(fd,DF_TY_PD_CM_RESET);

	struct _deploy  *stream;
    char name[DF_FILE_NAME_MAX_LEN];
    int fd;
    stream=arg;
    printf("reset_systerm\n");

	ty_ctl(stream->switch_fd,DF_SWITCH_CM_RESET_MA);

    // memset(name,0,sizeof(name));
    // sprintf(name,"/dev/%s",DF_TY_PD_NAME);
    // fd=ty_open(name,0,0,NULL);
    // ty_ctl(fd,DF_TY_PD_CM_RESET);
}

static void set_ip_q(void *arg)
{   
	struct _deploy  *stream;
    struct _net_define   net_par;
    stream=arg;
    memset(&net_par,0,sizeof(net_par));
    net_par.gateway[0]=stream->file.getway;
    net_par.ip=stream->file.ip;
    net_par.mask=stream->file.mask;
	net_par.type=stream->file.ip_type;
	net_par.DNS=stream->file.DNS;
	deploy_out("设置机柜网络参数\n");
    set_net(DF_NET_DEV,&net_par);

    return ;
}
