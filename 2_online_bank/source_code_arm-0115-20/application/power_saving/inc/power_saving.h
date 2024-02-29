#ifndef _POWER_SAVING_H
#define _POWER_SAVING_H

#include "opt.h"
// #include "module.h"
#include "scheduled_task.h"
#include "ty_usb.h"
#include "robot_arm.h"


typedef int (*Funport_check)(unsigned char *state_buff);

enum POWER_SAVING_RESPONSE_CODE
{
	POWER_SAVING_NO_ERROR = 0,
	POWER_SAVING_PARAMETER_ERROR, //传参错误
	POWER_SAVING_POWER_LOWER_PERMISSION_ERROR, //低权限
	POWER_SAVING_NO_ACTION_REQUIRED, //无需动作
	POWER_SAVING_OVER_MAX_ONLINE_NUM, //超过最大上限数
	POWER_SAVING_NO_ACTION,			//不允许动作
	POWER_SAVING_NULL_FUN			//空函数功能
};

typedef enum PORT_STATE_ENUM_T
{
	CLOSE_STATE = 0,
	OPEN_STATE,
	RETART_ACTION

}PORT_STATE_ENUM;

typedef int (*Funport_ctrl)(int port_num,PORT_STATE_ENUM state,int type);

typedef enum PORT_PRIORITY_ENUM_T
{
	PORT_CONFPRM_PRIORITY = 0,
	READ_COMPANY_TAX_PRIORITY,
	ATTACH_PRIORITY,
	HIGHEST_PRIORITY
	
}PORT_PRIORITY_ENUM;

typedef struct PORT_STATE_T
{
	PORT_PRIORITY_ENUM priority;
	PORT_STATE_ENUM port_state_enum;
}PORT_STATE;

typedef struct POWER_SAVING_STRUCT_T
{
	int machine_type;
	int online_num;
	int max_online_num;
	unsigned char *state_buff;
	PORT_STATE port_status[DF_TY_USB_PORT_N_MAX];
	PORT_STATE robot_arm_status[DF_TY_USB_PORT_N_MAX];
	Funport_ctrl port_ctrl;
	Funport_check port_check;
	MODULE *module; //模块管理指针
} POWER_SAVING_STRUCT;

int power_saving_init(MODULE *module,int machine_type,int max_onlie_num);
int power_saving_contrl_port(int port_num,PORT_STATE_ENUM state,PORT_PRIORITY_ENUM priority,int type);
PORT_STATE *get_port_priority_usedstate(int port_num);
int power_saving_port_data_init(int port_num);

#endif
