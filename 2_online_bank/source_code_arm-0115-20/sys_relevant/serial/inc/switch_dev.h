#ifndef _SWITCH_DEV_H
#define _SWITCH_DEV_H

#include "opt.h"

#include "seria.h"
//#include "mqtt_client.h"
#include "linux_ip.h"

#define DF_SWITCH_CM_OPEN               100                         //打开端口
#define DF_SWITCH_CM_CLOSE              (DF_SWITCH_CM_OPEN+1)       //关闭端口
#define DF_SWITCH_CM_SET_ID             (DF_SWITCH_CM_CLOSE+1)      //设置ID
#define DF_SWITCH_CM_GET_ID             (DF_SWITCH_CM_SET_ID+1)     //获取ID
#define DF_SWITCH_CM_GET_CPUID          (DF_SWITCH_CM_GET_ID+1)     //获取CPUID
#define DF_SWITCH_CM_SET_PCB_ID         (DF_SWITCH_CM_GET_CPUID+1)     //设置单板ID
#define DF_SWITCH_CM_GET_PCB_ID         (DF_SWITCH_CM_SET_PCB_ID+1) //获取单板ID
#define DF_SWITCH_CM_GET_CLASS          (DF_SWITCH_CM_GET_PCB_ID+1) //获取机柜
#define DF_SWITCH_CM_GET_PCB            (DF_SWITCH_CM_GET_CLASS+1)  //获取单板
#define DF_SWITCH_CM_PORT_STATE         (DF_SWITCH_CM_GET_PCB+1)    //端口状态      
#define DF_SWITCH_CM_RESET_MA           (DF_SWITCH_CM_PORT_STATE+1) //重新启动
#define DF_SWITCH_CM_POWER_CLOSE        (DF_SWITCH_CM_RESET_MA+1)   //关闭电源    
#define DF_SWITCH_CM_WRITE_FLASH        (DF_SWITCH_CM_POWER_CLOSE+1) //写flash
#define DF_SWITCH_CM_READ_FLASH         (DF_SWITCH_CM_WRITE_FLASH+1) //读flash  
#define DF_SWITCH_CM_GET_INFOR          (DF_SWITCH_CM_READ_FLASH+1)  //获取设备信息  
#define DF_SWITCH_CM_GET_DEV_SOFT_EDIT  (DF_SWITCH_CM_GET_INFOR+1)      //获取软件版本   
#define DF_SWITCH_CM_GET_HEART_TIMER    (DF_SWITCH_CM_GET_DEV_SOFT_EDIT+1)  //查询心跳时间  
#define DF_SWITCH_CM_HEART              (DF_SWITCH_CM_GET_HEART_TIMER+1)    //心跳   
#define DF_SWITCH_CM_ADD_RECODER        (DF_SWITCH_CM_HEART+1)          //增加记录
#define DF_SWITCH_CM_READ_RECODER       (DF_SWITCH_CM_ADD_RECODER+1)    //读记录
#define DF_SWITCH_CM_OPEN_HUB			(DF_SWITCH_CM_READ_RECODER+1)	//开HUB，单片机一直将HUB开着
#define DF_SWITCH_CM_GET_NET_OK			(DF_SWITCH_CM_OPEN_HUB+1)		//获取到网络,通知指示
#define DF_SWITCH_CM_NO_NET				(DF_SWITCH_CM_GET_NET_OK+1)		//网络丢失,通知
#define DF_SWITCH_CM_GET_SERIAL_NUM     (DF_SWITCH_CM_NO_NET+1)			//获取单片机唯一标识符 
#define DF_SWITCH_CM_PORT_NO_DEV		(DF_SWITCH_CM_GET_SERIAL_NUM+1)			//指示端口没有设备
#define DF_SWITCH_CM_PORT_DEV_LOCAL     (DF_SWITCH_CM_PORT_NO_DEV+1)			//指示端口设备在本地
#define DF_SWITCH_CM_PORT_DEV_SHARE     (DF_SWITCH_CM_PORT_DEV_LOCAL+1)			//指示端口设备被远程共享
#define DF_SWITCH_CM_CONN				DF_SWITCH_CM_PORT_DEV_SHARE+1
#define DF_SWITCH_CM_PORT_RED_OFF_GREN_OFF	(DF_SWITCH_CM_CONN+1)
#define DF_SWITCH_CM_PORT_RED_ON_GREN_OFF	(DF_SWITCH_CM_PORT_RED_OFF_GREN_OFF+1)
#define DF_SWITCH_CM_PORT_RED_OFF_GREN_ON	(DF_SWITCH_CM_PORT_RED_ON_GREN_OFF+1)
#define DF_SWITCH_CM_PORT_RED_ON_GREN_TWI	(DF_SWITCH_CM_PORT_RED_OFF_GREN_ON+1)
#define DF_SWITCH_CM_PORT_RED_ON_GREN_ON	(DF_SWITCH_CM_PORT_RED_ON_GREN_TWI+1)
#define DF_SWITCH_CM_PORT_RED_TWI_GREN_ON	(DF_SWITCH_CM_PORT_RED_ON_GREN_ON+1)
#define DF_SWITCH_CM_PORT_RED_TWI_GREN_TWI_1S		(DF_SWITCH_CM_PORT_RED_TWI_GREN_ON+1)
#define DF_SWITCH_CM_LCD_SCREEN_PRINT		(DF_SWITCH_CM_PORT_RED_TWI_GREN_TWI_1S+1)
#define DF_SWITCH_CM_GET_KEY_VALUE			(DF_SWITCH_CM_LCD_SCREEN_PRINT+1)
#define DF_SWITCH_CM_PORT_RED_TWI_GREN_OFF	(DF_SWITCH_CM_GET_KEY_VALUE+1)
#define DF_SWITCH_CM_PORT_RED_OFF_GREN_TWI	(DF_SWITCH_CM_PORT_RED_TWI_GREN_OFF+1)
#define DF_SWITCH_CM_MAC_RESTART_POWER		(DF_SWITCH_CM_PORT_RED_OFF_GREN_TWI+1)
#define DF_SWITCH_CM_GET_ALL_PORT_STATUS	(DF_SWITCH_CM_MAC_RESTART_POWER+1)
#define DF_SWITCH_CM_CONTRL_FAN			(DF_SWITCH_CM_GET_ALL_PORT_STATUS+1)
#define DF_SWITCH_CM_PORT_DOUBLE_LIGHT_YELLOW			(DF_SWITCH_CM_CONTRL_FAN+1)
#define DF_SWITCH_CM_GET_AD_STATUS			(DF_SWITCH_CM_PORT_DOUBLE_LIGHT_YELLOW+1)
#define DF_SWITCH_CM_RESTORE				(DF_SWITCH_CM_GET_AD_STATUS+1)
#define DF_SWITCH_CM_BLUETOOTH_MODE			(DF_SWITCH_CM_RESTORE+1)
#define DF_SWITCH_CM_BLUETOOTH_POWER				(DF_SWITCH_CM_BLUETOOTH_MODE+1)
#define DF_SWITCH_CM_GET_SUPPORT			DF_SWITCH_CM_BLUETOOTH_POWER+1

#define DF_SWITCH_CM_CLASS_ID           100                         //只获取ID
#define DF_SWITCH_CM_CLASS_ALL          (DF_SWITCH_CM_CLASS_ID+1)   //获取所有的class
        
#define DF_PC_ORDER_HEAD_LEN		 7                  //STM32命令头长读
/*+++++++++++++++++++++++++++++++命令字定义++++++++++++++++++++++++++++++++++++++*/
#define DF_PC_ORDER_CONNECT			0X01				//联机命令
#define DF_PC_ORDER_DISCONNECT		0X02				//释放控制权命令
#define DF_PC_ORDER_SET_DEV_ID		0X07				//设置设备ID
#define DF_PC_ORDER_READ_DEV_ID		0X08				//获取设备ID和生产日期
#define DF_PC_ORDER_SET_SUB_ID		0X11				//设置单板ID
#define DF_PC_ORDER_READ_SUB_ID		0X12				//获取单板ID
#define DF_PC_ORDER_GROUNP_PRO		0X13				//获取机柜群属性
#define DF_PC_ORDER_SUB_PRO			0X14				//获取单板属性
#define DF_PC_ORDER_USB_OPEN		0X15				//打开指定端口
#define DF_PC_ORDER_USB_CLOSE		0X16				//关闭指定端口
#define DF_PC_ORDER_USB_OPEN_LIST	0X17				//获取已打开USB列表	
#define DF_PC_ORDER_USB_GET_SOFT	0X18				//获取软件版本
#define DF_PC_ORDER_USB_WRITE_SOFT	0X19				//写软件
#define DF_PC_ORDER_USB_GET_OPEN_MA	0X20				//按机器编号排序显示已经打开的USB端口
#define DF_PC_ORDER_USB_GET_MA_OPEN	0X21				//指定机器编号获取已经打开的USB端口
#define DF_PC_ORDER_USB_MA_OPEN		0X22				//指定机器编号打开USB端口
#define DF_PC_ORDER_USB_MA_CLOSE	0X23				//指定机器编号关闭USB端口
#define DF_PC_ORDER_POWER_OFF		0X24				//关闭机器
#define DF_PC_ORDER_USB_HUB_N		0X25				//USB HUB数量
#define DF_PC_ORDER_OPEN_HUB        0X26                //打开所有HUB
#define DF_PC_ORDER_RESET           0X27                //重新启动
#define DF_PC_ORDER_HREAT           0X28                //心跳命令
#define DF_PC_ORDER_HREAT_TIMER     0X29                //查询心跳时间
#define DF_PC_ORDER_WRITE_FLASH     0X2a                //写flash
#define DF_PC_ORDER_READ_FLASH      0X2B                //读flash
#define DF_PC_ORDER_GET_EDIT_INFOR  0X2c                //获取版本信息
#define DF_PC_ORDER_WRITE_RECODER   0X2D                //增加记录文件
#define DF_PC_ORDER_READ_RECODER    0X2E                //读记录文件  
#define DF_PC_ORDER_GET_NET_OK		0X2F				//获取到网络通知下位机
#define DF_PC_ORDER_NO_NET			0X30				//网络丢失通知下位机
#define DF_PC_ORDER_GET_SERIAL_NUM	0X31				//获取单片机唯一序列号
#define DF_PC_ORDER_CTL_PORT_LED	0X32				//控制端口灯状态指示
#define DF_PC_ORDER_CTL_MAC_POWER	0X33				//控制控制MAC电源

#define DF_PC_ORDER_CTL_PORT_LED_2	0X40				//控制端口灯状态指示(亮、灭、闪)

#define DF_PC_ORDER_CTL_LCD_SCREEN	0X41				//驱动LCD显示屏显示

#define DF_PC_ORDER_CTL_WRITE_KEY	0X45				//写入密钥文件
#define DF_PC_ORDER_CTL_READ_KEY	0X46				//读取密钥文件
#define DF_PC_ORDER_CTL_GET_ALL_PORT_STATUS	0X47		//获取已下电端口设备状态
#define DF_PC_ORDER_CTL_FAN					0X49		//控制风扇打开关闭
#define DF_PC_ORDER_CTL_BLUETOOTH	0x51				//控制蓝牙模式 AT  透传
#define	DF_PC_ORDER_BLUETOOTH_POWER	0X52				//控制蓝牙电源
#define DF_PC_ORDER_RESTORE						0X53	//回出厂
#define DF_PC_ORDER_SET_PORT_DOUBLE_LIGHT			0X54
#define DF_PC_ORDER_GET_AD_STATUS				0x55

#define	DF_WIFI_LED_SAME_USBPORT	10001
   
#define switch_dev_out(s,arg...)       		//log_out("all.c",s,##arg) 
		
#define DF_SERIA_DIR                    "/dev/seria0"   
#define DF_SERIA_BPS                    115200    
#define DF_TY_STM32_OVER_TIMER          2000        //STM32通讯超时时间   

struct _switch_event
{
	int event;
	int reason;
	uint8 clock[8];
};

struct _switch_event_all
{
	int n;
	struct _switch_event event[30];
};
    
struct _switch_dev_pcb_id
{
	uint8 id[6];
	uint8 date[4];
	int n;          //单板开关口数量
};

struct _switch_dev_id
{
	uint8 id[6];
	uint8 date[4];
};

struct _switch_dev_only
{
	uint8 serial_num[12];
};

struct _switch_dev_class        //机柜群属性
{
	uint8 id[6];
	int n;
};

struct _switch_dev_pcb      //单板属性
{
	uint8 id[6];
	int n;
};

struct _stm32_infor
{
	char r_date[11];        //发布日期
	char soft[9];           //软件版本
	char hard[9];           //硬件版本
};

struct _dev_support
{
	int support_fan;//风扇
	int support_ble;//蓝牙和黄灯
	int support_power;//节电
	int support_lcd;//显示屏和LED新版支持
};

struct _switch_dev_fd
{
	int state;
	struct ty_file_mem *mem;
	void *mem_dev;
	struct ty_file_fd *mem_fd;
	struct _switch_dev *dev;
};

struct _switch_dev
{
	struct _switch_dev_fd fd[DF_FILE_OPEN_NUMBER];
	int state;
	int lock;
	int seria;
	int hardtype;
	struct _dev_support dev_support;
	char ter_type[50];
	char ter_id[20];
	char cpu_id[50];
};

struct _switch_ctl_fu
{
	int cm;
	int (*ctl)(struct _switch_dev_fd *id,va_list args);
};

int switch_dev_add(int hard_type, char *ter_type);
    
#endif
