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

/*======================================�ṹ�嶨��============================================*/
struct _lcd_print_info
{
	char version[LCD_STATE_BUFF_SIZE];
	char local_ip[LCD_STATE_BUFF_SIZE];
	int usbshare_en;	//0����usb-share����		1����usb - share����
	int usbshare_state;	//0����ֹ״̬				1������ʹ��
	int tax_en;			//0������ֵ˰��Ʊ����		1������ֵ˰��Ʊ����
	int tax_state;		//0����ֹ״̬				1������ʹ��
	int eth_en;			//0������������				1������������
	int eth_state;		//0��û�в��ҵ�����			1�����������ѻ�ȡ��ip��ַ					2������������������				8�������������ڻ�ȡip��ַ
	int wifi_en;		//0�����ȵ㹦��				1�����ȵ㹦��
	int wifi_state;		//0���ȵ����������			1���ȵ��Ѿ�����								2���ȵ��Ѿ�������
	int four_g_en;		//0:��4G����				1����4G����
	int four_g_state;	//0:���ڲ�������			1:���Ŵ���									2�����������ѻ�ȡ��ip��ַ		3��������������������δ����		8�������������ڻ�ȡip��ַ
	int sd_en;			//0����SD������				1����SD������
	int sd_state;		//0����SD������				1����SD�����룬��SD��״̬������				2����SD�����룬����
	int mqtt_en;		//0����						1����
	int mqtt_state;		//0��δ��¼					1���Ѿ���¼
	int udp_en;			//0����						1����
	int udp_state;		//0��δ��¼					1���Ѿ���¼
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
