#ifndef _TY_USB_H
#define _TY_USB_H

#include "opt.h"

#include "switch_dev.h"
#include "usb_file.h"
#include "tax_interface.h"
	
#define DF_TY_USB_CM_GET_BUSID              100                         //获取主busid
#define DF_TY_USB_CM_SET_BUSID              (DF_TY_USB_CM_GET_BUSID+1)  //设置主busid
#define DF_TY_USB_CM_SET_PORT_N             (DF_TY_USB_CM_SET_BUSID+1)  //设置排列顺序
#define DF_TY_USB_CM_GET_PORT_N             (DF_TY_USB_CM_SET_PORT_N+1) //获取排列顺序
#define DF_TY_USB_CM_SET_ENUM_TIMER         (DF_TY_USB_CM_GET_PORT_N+1) //最大枚举时间
#define DF_TY_USB_CM_GET_ENUM_TIMER         (DF_TY_USB_CM_SET_ENUM_TIMER+1) //获取最大枚举时间
#define DF_TY_USB_CM_OPEN_PORT              (DF_TY_USB_CM_GET_ENUM_TIMER+1) //打开端口
#define DF_TY_USB_CM_CLOSE_PORT             (DF_TY_USB_CM_OPEN_PORT+1)      //关闭端口
#define DF_TY_USB_CM_PORT_BUSID             (DF_TY_USB_CM_CLOSE_PORT+1)      //获取端口的busid
#define DF_TY_USB_CM_PORT_NUMBER            (DF_TY_USB_CM_PORT_BUSID+1)     //获取端口总数量
#define DF_TY_USB_CM_JUGE_PORT              (DF_TY_USB_CM_PORT_NUMBER+1)    //判断端口的合法性
#define DF_TY_USB_CM_GET_PORT_MA_ID         (DF_TY_USB_CM_JUGE_PORT+1)      //获取端口所属机柜编号
#define DF_TY_USB_CM_GET_PORT_PCB_ID        (DF_TY_USB_CM_GET_PORT_MA_ID+1) //获取端口所在的PCB
#define DF_TY_USB_CM_GET_BUSID_PORT         (DF_TY_USB_CM_GET_PORT_PCB_ID+1)    //通过busid获取port
#define DF_TY_USB_CM_GET_REGISTER_WHETHER	(DF_TY_USB_CM_GET_BUSID_PORT+1)
#define DF_TY_USB_CM_OPEN_PORT_POWER		(DF_TY_USB_CM_GET_REGISTER_WHETHER+1)//仅仅打开端口电源
#define DF_TY_USB_CM_GET_PORT_FILE_EXIST	(DF_TY_USB_CM_OPEN_PORT_POWER+1)//检查usb文件是否存在
#define DF_TY_USB_CM_GET_TRYOUT_WHETHER		(DF_TY_USB_CM_GET_PORT_FILE_EXIST+1)//获取试用期限
#define DF_TY_USB_CM_PORT_SEV_LED_NO		(DF_TY_USB_CM_GET_TRYOUT_WHETHER+1)
#define DF_TY_USB_CM_PORT_SEV_LED_LOCAL		(DF_TY_USB_CM_PORT_SEV_LED_NO+1)
#define DF_TY_USB_CM_PORT_SEV_LED_SHARE		(DF_TY_USB_CM_PORT_SEV_LED_LOCAL+1)
#define DF_TY_USB_CM_PORT_CLOSE_PORT_POWER	(DF_TY_USB_CM_PORT_SEV_LED_SHARE+1)
#define DF_TY_USB_CM_GET_MACHINE_ID			(DF_TY_USB_CM_PORT_CLOSE_PORT_POWER+1)
#define DF_TY_USB_CM_CLOSE_PORT_POWER		(DF_TY_USB_CM_GET_MACHINE_ID+1)
#define DF_TY_USB_CM_PORT_RED_OFF_GREN_OFF	(DF_TY_USB_CM_CLOSE_PORT_POWER+1)
#define DF_TY_USB_CM_PORT_RED_ON_GREN_OFF	(DF_TY_USB_CM_PORT_RED_OFF_GREN_OFF+1)
#define DF_TY_USB_CM_PORT_RED_OFF_GREN_ON	(DF_TY_USB_CM_PORT_RED_ON_GREN_OFF+1)
#define DF_TY_USB_CM_PORT_RED_ON_GREN_TWI	(DF_TY_USB_CM_PORT_RED_OFF_GREN_ON+1)
#define DF_TY_USB_CM_PORT_RED_ON_GREN_ON	(DF_TY_USB_CM_PORT_RED_ON_GREN_TWI+1)
#define DF_TY_USB_CM_PORT_RED_TWI_GREN_ON	(DF_TY_USB_CM_PORT_RED_ON_GREN_ON+1)
#define DF_TY_USB_CM_PORT_RED_TWI_GREN_TWI_1S	(DF_TY_USB_CM_PORT_RED_TWI_GREN_ON+1)
#define DF_TU_USB_CM_PORT_GET_KEY_VALUE		(DF_TY_USB_CM_PORT_RED_TWI_GREN_TWI_1S+1)
#define DF_TY_USB_CM_PORT_RED_TWI_GREN_TWI	(DF_TU_USB_CM_PORT_GET_KEY_VALUE+1)
#define DF_TY_USB_CM_PORT_RED_OFF_GREN_TWI	(DF_TY_USB_CM_PORT_RED_TWI_GREN_TWI+1)
#define DF_TY_USB_CM_PORT_RED_TWI_GREN_OFF	(DF_TY_USB_CM_PORT_RED_OFF_GREN_TWI+1)

#define DF_TY_USB_CM_PORT_GET_ALL_PORT_STATUS	(DF_TY_USB_CM_PORT_RED_TWI_GREN_OFF+1)
#define DF_TY_USB_CM_PORT_SET_FAN	(DF_TY_USB_CM_PORT_GET_ALL_PORT_STATUS+1)

#define DF_TY_USB_CM_GET_AD_STATUS	(DF_TY_USB_CM_PORT_SET_FAN+1)
#define DF_TY_USB_CM_RESTORE	(DF_TY_USB_CM_GET_AD_STATUS+1)
#define DF_TY_USB_CM_PORT_DOUBLE_LIGHT_YELLOW	(DF_TY_USB_CM_RESTORE+1)
#define DF_TY_USB_CM_CTL_BLUETOOTH_MODE			(DF_TY_USB_CM_PORT_DOUBLE_LIGHT_YELLOW+1)
#define DF_TY_USB_CM_BLUETOOTH_POWER			(DF_TY_USB_CM_CTL_BLUETOOTH_MODE+1)
#define DF_TY_USB_CM_CONN		DF_TY_USB_CM_BLUETOOTH_POWER+1

#define DF_TY_BUS_DEV_NUMBER        127                 //BUS承载的总数量
#define DF_TY_USB_HUB_OPEN			1					//上电后,HUB一直开着
#define DF_TY_USB_HUB_CLOSE			0					//上电后,只有打开端口HUB才开

#define ty_usb_out(s,arg...)       				//log_out("all.c",s,##arg)    

struct _ty_usb_fd
{
	int state;
	struct ty_file_mem	*mem;
	void *mem_dev;
	struct ty_file_fd	*mem_fd;
	struct _ty_usb  *dev;
};

struct _ty_usb_pcb                                  //单板PCB
{
	uint8 id[6];                                    //单板id
	uint16 n;                                       //USB数量
	int state;                                      //该PCB状态
	uint8 hub_state[8];                             //HUB的状态
};

struct _ty_usb_class
{
	uint8 id[6];                                    //机柜的id
	struct _ty_usb_pcb      pcb[8];                 //单板属性描述
	uint16 n;                                       //单板数量
	int usb_port_n;                                 //USB端口数量
	uint8 soft[8];                                  //软件版本
	uint8 hard[8];                                  //硬件版本
	int port_state[DF_TY_USB_PORT_N_MAX];               //端口状态
	int port_lock[DF_TY_USB_PORT_N_MAX];                //端口锁
	char busid[DF_TY_USB_BUSID];                    //主芯片busid
};

struct _ty_usb_dev
{
	struct _ty_usb_class    dev;                    //设备 
	struct _switch_dev_id   self_id;                //自己本身的ID
	struct _stm32_infor     stm32_infor;            //STM32 INFOR
};

struct _ty_usb
{
	struct _ty_usb_dev     data;                    //USB数据
	int lock;                                       //数据锁
	int type;										//机器类型
	int state;                                      //状态
	struct _ty_usb_fd   fd[DF_FILE_OPEN_NUMBER];    //各个句柄状态
	int enum_timer;                                 //枚举最大时间
	int port_n;                                     //端口排列顺序
	char switch_name[DF_FILE_NAME_MAX_LEN];         //开关名
	int switch_fd;                                  //开关fd
	int hub;										//HUB是否一直打开着
};

struct _ty_usb_ctl_fu
{
	int cm;
	int (*ctl)(struct _ty_usb_fd   *id,va_list args);
};

int ty_usb_add(char *switch_name,int type);
int vid_pid_jude_plate(uint16 vid, uint16 pid);

int my_get_port_exit(int port,_so_note *app_note);
int my_get_busid(int port,char *busid,_so_note *app_note);

int my_get_busid_only(int port,char *busid,_so_note *app_note);

int my_judge_exit_dev_robotarm(int port,_so_note *app_note,int dev_or_roborarm);

int my_judge_exit_dev(int port,_so_note *app_note);

#endif
