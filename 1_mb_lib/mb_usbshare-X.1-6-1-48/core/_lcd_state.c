#define _lcd_state_c
#include "_lcd_state.h"
int lcd_state_add(void)
{   struct _lcd_state *stream;
	int result;
	stream=malloc(sizeof(struct _lcd_state));
	if(stream==NULL)
		return DF_ERR_MEM_ERR;
	memset(stream,0,sizeof(struct _lcd_state));
   // out("增加\"/dev/lcd_state\"文件至文件系统\n");
	result = ty_file_add(DF_DEV_LCD_STATE, stream, "lcd_state", (struct _file_fuc *)&lcd_state_fuc);
	if(result<0)
		free(stream);
	
	return result;
}


static int lcd_state_open(struct ty_file	*file)
{
	struct _lcd_state *stream;
	int j;
	uint32 ip = 0;
	uint8 s_buf[200] = { 0 };
	stream=file->pro_data;
	if((stream==NULL))	
		return DF_ERR_PORT_CORE_FD_NULL;
    //out("打开lcd_state模块\n");
    if(stream->state==0)
    {   //out("首次使用lcd_state\n");
		memset(&lcd_print_info, 0, sizeof(struct _lcd_print_info));
        stream->switch_dev=ty_open("/dev/switch",0,0,NULL);
        if(stream->switch_dev<0)
        {   out("打开switch设备失败\n");
            return stream->switch_dev;
        }
		stream->lock = _lock_open("s", NULL);

		_lock_set(stream->lock);
		memcpy(&lcd_print_info, &default_info, sizeof(struct _lcd_print_info));
		ip = ip_asc2ip(lcd_print_info.local_ip, strlen(lcd_print_info.local_ip));
		memcpy(s_buf, lcd_print_info.version, 8);

		s_buf[8] = ((ip >> 24) & 0xff); s_buf[9] = ((ip >> 16) & 0xff);
		s_buf[10] = ((ip >> 8) & 0xff); s_buf[11] = ((ip >> 0) & 0xff);
		s_buf[12] = lcd_print_info.usbshare_en;
		s_buf[13] = lcd_print_info.usbshare_state;
		s_buf[14] = lcd_print_info.tax_en;
		s_buf[15] = lcd_print_info.tax_state;
		s_buf[16] = lcd_print_info.eth_en;
		s_buf[17] = lcd_print_info.eth_state;
		s_buf[18] = lcd_print_info.wifi_en;
		s_buf[19] = lcd_print_info.wifi_state;
		s_buf[20] = lcd_print_info.four_g_en;
		s_buf[21] = lcd_print_info.four_g_state;
		s_buf[22] = lcd_print_info.sd_en;
		s_buf[23] = lcd_print_info.sd_state;
		s_buf[24] = lcd_print_info.mqtt_en;
		s_buf[25] = lcd_print_info.mqtt_state;
		s_buf[26] = lcd_print_info.udp_en;
		s_buf[27] = lcd_print_info.udp_state;
		_lock_un(stream->lock);
		//print_array(s_buf,28);
		//out("设置LCD显示数据，即将设置LCD显示\n");
#ifdef MTK_OPENWRT
		ty_ctl(stream->switch_dev, DF_SWITCH_CM_STATUS_SET, s_buf, 28);
#else
		ty_ctl(stream->switch_dev, DF_SWITCH_CM_LCD_SCREEN_PRINT, s_buf, 28);
#endif

		// out("lcd_state模块初始化成功\n");
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
    out("打开get_net_state成功:j=%d\n",j);
	return j+1;	 
}

static int lcd_state_ctl(void *data, int fd, int cm, va_list args)
{
	struct _lcd_state        *stream;
	struct _lcd_state_fd     *id;
	int i;
	stream = data;
	if ((stream == NULL))
		return DF_ERR_PORT_CORE_FD_NULL;
	if ((fd == 0) || (fd>sizeof(stream->fd) / sizeof(stream->fd[0])))
		return DF_ERR_PORT_CORE_FD_Z;
	fd--;
	id = &stream->fd[fd];
	if (id->state == 0)
		return DF_ERR_PORT_CORE_SLAVE_DEV;
	for (i = 0; i<sizeof(ctl_fun) / sizeof(ctl_fun[0]); i++)
	{
		if (cm == ctl_fun[i].cm)
			return ctl_fun[i].ctl(id, args);
	}
	return DF_ERR_PORT_CORE_CM;
}
static int lcd_state_close(void *data, int fd)
{
	return 0;
}


static int get_lcd_update_state(struct _lcd_state_fd *id, va_list args)
{
	struct _lcd_state  *stream;
	struct _lcd_print_info *_info;
	stream = id->dev;
	_info = va_arg(args, struct _lcd_print_info *);
	_lock_set(stream->lock);
	memcpy(_info, &lcd_print_info, sizeof(struct _lcd_print_info));
	_lock_un(stream->lock);
	out("获取当前LCD状态数据成功\n");
	return 0;
}

static int set_lcd_update_state(struct _lcd_state_fd *id, va_list args)
{
	struct _lcd_state  *stream;
	//struct _lcd_print_info *_info;
	uint32 ip = 0;
	char s_buf[100] = {0};
	stream = id->dev;
	//_info = va_arg(args, struct _lcd_print_info *);
	_lock_set(stream->lock);
	//memcpy(&lcd_print_info, _info, sizeof(struct _lcd_print_info));
	ip = ip_asc2ip(lcd_print_info.local_ip, strlen(lcd_print_info.local_ip));
	memcpy(s_buf, lcd_print_info.version,8);

	s_buf[8] = ((ip >> 24) & 0xff); s_buf[9] = ((ip >> 16) & 0xff);
	s_buf[10] = ((ip >> 8) & 0xff); s_buf[11] = ((ip >> 0) & 0xff);
	s_buf[12] = lcd_print_info.usbshare_en;
	s_buf[13] = lcd_print_info.usbshare_state;
	s_buf[14] = lcd_print_info.tax_en;
	s_buf[15] = lcd_print_info.tax_state;
	s_buf[16] = lcd_print_info.eth_en;
	s_buf[17] = lcd_print_info.eth_state;
	s_buf[18] = lcd_print_info.wifi_en;
	s_buf[19] = lcd_print_info.wifi_state;
	s_buf[20] = lcd_print_info.four_g_en;
	s_buf[21] = lcd_print_info.four_g_state;
	s_buf[22] = lcd_print_info.sd_en;
	s_buf[23] = lcd_print_info.sd_state;
	s_buf[24] = lcd_print_info.mqtt_en;
	s_buf[25] = lcd_print_info.mqtt_state;
	s_buf[26] = 0;//udp 功能
	s_buf[27] = 0;//udp 状态
	_lock_un(stream->lock);
	out("设置LCD显示数据，即将设置LCD显示\n");
	//print_array(s_buf, 28);
#ifdef MTK_OPENWRT
	ty_ctl(stream->switch_dev, DF_SWITCH_CM_STATUS_SET, s_buf, 28);
#else
	ty_ctl(stream->switch_dev, DF_SWITCH_CM_LCD_SCREEN_PRINT,s_buf,28);
#endif
	return 0;
}