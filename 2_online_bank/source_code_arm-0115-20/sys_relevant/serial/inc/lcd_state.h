#ifndef _LCD_STATE_H
#define _LCD_STATE_H

#include "opt.h"

#include "switch_dev.h"

#define DF_LCD_STATE_GET_STATE	100
#define DF_LCD_STATE_UPDATE_STATE	(DF_LCD_STATE_GET_STATE+1)

#define LCD_STATE_BUFF_SIZE_32 32
#define LCD_STATE_BUFF_SIZE_255 255

#define LCD_STATE_BUFF_SIZE 32

#define lcd_state_out(s,arg...)		//log_out("_lcd_state.c",s,##arg)

/*======================================结构体定义============================================*/
struct _lcd_print_info
{
	char version[LCD_STATE_BUFF_SIZE];
	char local_ip[LCD_STATE_BUFF_SIZE];
	int usbshare_en;	//0：无usb-share功能		1：有usb - share功能
	int usbshare_state;	//0：静止状态				1：正在使用
	int tax_en;			//0：无增值税开票功能		1：有增值税开票功能
	int tax_state;		//0：静止状态				1：正在使用
	int eth_en;			//0：无有线网络				1：有有线网络
	int eth_state;		//0：没有查找到网线			1：有线网络已获取到ip地址					2：有线网络能上外网				8：有线网络正在获取ip地址
	int wifi_en;		//0：无热点功能				1：有热点功能
	int wifi_state;		//0：热点查找驱动中			1：热点已经开启								2：热点已经有连接
	int four_g_en;		//0:无4G功能				1：有4G功能
	int four_g_state;	//0:正在查找驱动			1:拨号错误									2：无线网络已获取到ip地址		3：无线网络能上外网，未传输		8：无线网络正在获取ip地址
	int sd_en;			//0：无SD卡功能				1：有SD卡功能
	int sd_state;		//0：无SD卡插入				1：有SD卡插入，但SD卡状态有问题				2：有SD卡插入，正常
	int mqtt_en;		//0：无						1：有
	int mqtt_state;		//0：未登录					1：已经登录
	int udp_en;			//0：无						1：有
	int udp_state;		//0：未登录					1：已经登录
};

struct _lcd_state_fd
{
	int state;
	struct ty_file_mem *mem;
	void *mem_dev;
	struct ty_file_fd *mem_fd;
	struct _lcd_state *dev;
};
        
struct _lcd_state
{
	struct _lcd_state_fd fd[DF_FILE_OPEN_NUMBER];  
	int state;
	int lock;
	int task;
	int switch_dev;
	int net_state;
	int deploy;
};

struct _lcd_state_ctl_fu
{
	int cm;
	int (*ctl)(struct _lcd_state_fd *id,va_list args);
};

int lcd_state_add(void);
struct _lcd_print_info *get_lcd_printf_info(void);

#endif
