#ifndef _REGISTER_H
#define _REGISTER_H

#include "opt.h"

#include "file.h"
#include "http.h"
#include "linux_ip.h"
#include "mqtt_client.h"
#include "http_reg.h"

#define DF_TY_REGISTER_CM_WRITE                   	1               			//写注册
#define DF_TY_REGISTER_CM_WHETHER 					DF_TY_REGISTER_CM_WRITE+1	//获取注册与否
#define DF_TY_MQTT_REGISTER_CM_WHETHER 				DF_TY_REGISTER_CM_WHETHER+1	//获取是否在MQTT系统中注册
#define DF_TY_MQTT_USRER_NAME_PASSWD 				DF_TY_MQTT_REGISTER_CM_WHETHER+1	//获取是否在MQTT系统中注册

#define DF_M_SERVER_ADDRESS	"www.njmbxx.com"
#define DF_M_SERVER_PORT 20020
#define DF_M_SERVER_APPID "EE8B56879B7D5449933C1F8918C33FB6"
#define DF_M_SERVER_APPSECERT "74993F73731FAE499F0F2A98CC4A9E90"

#define register_out(s,arg...)       				log_out("all.c",s,##arg) 

struct _register_fd
{
	int state;
	struct ty_file_mem	*mem;
	void *mem_dev;
	struct ty_file_fd	*mem_fd;
	struct _register  *dev;
};

struct _register
{
	int state;                                                      //本模块状态
	int lock;                                                       //数据锁
	int task_fd;                                                    //主任务号
	struct _register_fd fd[DF_FILE_OPEN_NUMBER];
	//uint8 hard_ware[20];//硬件版本最大8字节
	//uint8 soft_ware[20];//软件版本最大8字节
	//uint8 ser_num[30];//
	//uint8 id[20];//
	//uint8 onlyid[20];//
	//uint8 type[20];//
	char reg_err_info[1024];
	_so_note    *note;
};

struct _register_ctl_fu
{
	int cm;
	int(*ctl)(struct _register_fd   *id, va_list args);
};

int _register_add(const _so_note    *note);
int data_3des_base(uint8 *in_data, uint8 *out_data);
int get_reg_err_info(char *err_info);

#endif
