#include "../inc/power_saving.h"

POWER_SAVING_STRUCT power_saving;

void power_check_thread(void *arg, int timer);
int port_control(int port_num,PORT_STATE_ENUM state,int type);

/*********************************************************************
-   Function : power_saving_init
-   Description：节电模块初始化，注册端口操作函数、检测usb设备插拔函数，启动节电策略线程
-   Input  :
-   Output :
-   Return :0 正常 非零 返回相应的错误码
-   Other :
***********************************************************************/
int power_saving_init(MODULE *module,int machine_type,int max_onlie_num)
{
	if(NULL == module)
	{
		return POWER_SAVING_PARAMETER_ERROR;
	}
	memset(&power_saving,0x00,sizeof(POWER_SAVING_STRUCT));
	power_saving.port_ctrl = port_control;
	power_saving.module = module;
	power_saving.max_online_num = max_onlie_num;
	power_saving.machine_type = machine_type;
	//power_saving.port_check = port_check;
	//_scheduled_task_open("power_saving", power_check_thread, NULL, 100, NULL);		
	return POWER_SAVING_NO_ERROR;
}

/*********************************************************************
-   Function : port_control
-   Description：节电模块端口操作
-   Input  :
-   Output :
-   Return :0 正常 非零 返回相应的错误码
-   Other :
***********************************************************************/
int port_control(int port_num,PORT_STATE_ENUM state,int type)
{
	int result = POWER_SAVING_NO_ERROR;
	
	if(type == 1)  //机械臂方式
	{
		if(1 == power_saving.machine_type)
		{
			if(0 == get_robot_arm_class()->fun_judge_robot_arm(port_num))
			{
				logout(INFO, "DEV", "PORT_POWER", "此端口是机械臂，暂不允许执行端口电源操作\n");
				return POWER_SAVING_NO_ACTION;
			}
			result = (get_robot_arm_class())->fun_set_power(port_num/2,(PORT_POWER_STATE)state);
			if (result < 0)
			{
				logout(INFO, "DEV", "PORT_POWER", "1 port_control:机械臂执行 %d 失败端口%d,result=%d\n", (int)state,port_num,result);
				return result;
			}
		}
		else
		{
			result = (get_robot_arm_class())->fun_set_power(port_num,(PORT_POWER_STATE)state);
			if (result < 0)
			{
				logout(INFO, "DEV", "PORT_POWER", "2 port_control:机械臂执行 %d 失败端口%d,result=%d\n", (int)state,port_num,result);
				return result;
			}
		}
	}
	else
	{
		if(CLOSE_STATE == state)
		{
			result = ty_ctl(power_saving.module->ty_usb_fd, DF_TY_USB_CM_CLOSE_PORT_POWER,port_num + 1);//执行下电操作
			if (result < 0)
			{
				logout(INFO, "DEV", "PORT_POWER", "port_control:单片机执行下电失败%d\n", port_num + 1);
				return result;
			}
		}
		else
		{
			result = ty_ctl(power_saving.module->ty_usb_fd, DF_TY_USB_CM_OPEN_PORT_POWER,port_num + 1);//执行上电操作
			if (result < 0)
			{
				logout(INFO, "DEV", "PORT_POWER", "port_control:单片机执行上电失败%d\n", port_num + 1);
				return result;
			}
		}
	}
	return POWER_SAVING_NO_ERROR;
}


/*********************************************************************
-   Function : power_saving_policy
-   Description：节电策略判定实现
-   Input  :port_num:从1开始
-   Output :
-   Return :0 可以操作 !0 不可以操作
-   Other :
***********************************************************************/
int power_saving_policy_judge(int port_num,PORT_STATE_ENUM state,PORT_PRIORITY_ENUM priority)
{
	if(PORT_START_NUM > port_num + 1 || DF_TY_USB_PORT_N_MAX < port_num)
		return POWER_SAVING_PARAMETER_ERROR;
	if(OPEN_STATE == state && priority < power_saving.port_status[port_num].priority) //请求者权限等级低于占用者等级，打开操作没有操作权限
	{
		return POWER_SAVING_POWER_LOWER_PERMISSION_ERROR;
	}
	if(CLOSE_STATE != power_saving.port_status[port_num].port_state_enum)//状态不属于关闭状态，说明已被占用
	{
		if(priority < power_saving.port_status[port_num].priority) //请求者权限等级低于占用者等级，没有操作权限
		{
			return POWER_SAVING_POWER_LOWER_PERMISSION_ERROR;
		}
	}
	printf("power_saving_policy_judge port_state_enum = %d,state = %d\n",power_saving.port_status[port_num].port_state_enum,state);
	if(state == power_saving.port_status[port_num].port_state_enum)//相同状态，不需要动作
	{
		//printf("1111  power_saving.port_status[%d].priority = %d\n",port_num,power_saving.port_status[port_num].priority);
		power_saving.port_status[port_num].priority = power_saving.port_status[port_num].priority > priority ? power_saving.port_status[port_num].priority : priority;
		//printf("2222  power_saving.port_status[%d].priority = %d\n",port_num,power_saving.port_status[port_num].priority);
		return POWER_SAVING_NO_ACTION_REQUIRED; //不需要动作
	}
	if(CLOSE_STATE == state)//关闭动作
	{
		return POWER_SAVING_NO_ERROR;
	}
	else if(OPEN_STATE == state)//开启动作
	{
		if(power_saving.online_num < MAX_ONLINE_NUM)
			return POWER_SAVING_NO_ERROR;
		else
		{
			printf("power_saving.online_num = %d\n",power_saving.online_num);
			return POWER_SAVING_OVER_MAX_ONLINE_NUM;
		}
			

	}
	return POWER_SAVING_NO_ERROR;
}

int power_saving_port_data_init(int port_num)
{
	power_saving.port_status[port_num].priority = PORT_CONFPRM_PRIORITY;
	power_saving.port_status[port_num].port_state_enum = CLOSE_STATE;
	power_saving.online_num = (power_saving.online_num > 0)?(power_saving.online_num-1):0;
	return 0;
}

/*********************************************************************
-   Function : power_saving_contrl_port
-   Description：端口操作
-   Input  :
-   Output :
-   Return :0 操作成功 非零 返回相应的错误码
-   Other :
***********************************************************************/
int power_saving_contrl_port(int port_num,PORT_STATE_ENUM state,PORT_PRIORITY_ENUM priority,int type)
{
	int ret = POWER_SAVING_NO_ERROR;
	if(NULL == power_saving.port_ctrl)
	{
		return POWER_SAVING_NULL_FUN;
	}
	ret = power_saving_policy_judge(port_num,state,priority);
	printf("power_saving_policy_judge ret = %d\n",ret);
	if(POWER_SAVING_NO_ERROR == ret)
	{
		ret = power_saving.port_ctrl(port_num,state,type);
		printf("power_saving.port_ctrl ret = %d\n",ret);
		if(POWER_SAVING_NO_ERROR == ret)
		{
			if(state == CLOSE_STATE)
			{
				power_saving.online_num = (power_saving.online_num > 0)?(power_saving.online_num-1):0;
				power_saving.port_status[port_num].priority = PORT_CONFPRM_PRIORITY; //关闭时将权限置为最低
			}
			else
			{
				power_saving.online_num++;
				printf("11111111111 power_saving.online_num = %d\n",power_saving.online_num);
				if(HIGHEST_PRIORITY == priority)	//强制关闭时，优先级改为最低
					power_saving.port_status[port_num].priority = PORT_CONFPRM_PRIORITY;
				else
					power_saving.port_status[port_num].priority = priority;
			}
			power_saving.port_status[port_num].port_state_enum = state;
			return POWER_SAVING_NO_ERROR;
		}
		else if(POWER_SAVING_NO_ACTION == ret)   //机械别不允许动作
		{
			if(state == CLOSE_STATE)
			{
				// power_saving.online_num = (power_saving.online_num > 0)?(power_saving.online_num-1):0;
				power_saving.port_status[port_num].priority = PORT_CONFPRM_PRIORITY; //关闭时将权限置为最低
			}
			else
			{
				// power_saving.online_num++;
				// printf("11111111111 power_saving.online_num = %d\n",power_saving.online_num);
				if(HIGHEST_PRIORITY == priority)	//强制关闭时，优先级改为最低
					power_saving.port_status[port_num].priority = PORT_CONFPRM_PRIORITY;
				else
					power_saving.port_status[port_num].priority = priority;
			}
			power_saving.port_status[port_num].port_state_enum = state;
			return POWER_SAVING_NO_ERROR;
		}
		return ret;
	}
	else if(POWER_SAVING_NO_ACTION_REQUIRED == ret) //无需动作，认为是操作成功
	{
		if(state == CLOSE_STATE)   //关闭时将权限置为最低
		{
			power_saving.port_status[port_num].priority = PORT_CONFPRM_PRIORITY;
		}
		return POWER_SAVING_NO_ERROR;
	}
	
	return ret;
}

/*********************************************************************
-   Function : power_check_thread
-   Description：节电策略轮询线程,主要负责端口检测
-   Input  :
-   Output :
-   Return :
-   Other :
***********************************************************************/
void power_check_thread(void *arg, int timer)
{
	//power_saving.port_check();
}

/*********************************************************************
-   Function : get_port_priority_usedstate
-   Description：获取某一端口的打开状态和使用者等级
-   Input  :
-   Output :
-   Return :
-   Other :
***********************************************************************/
PORT_STATE *get_port_priority_usedstate(int port_num)
{
	return &(power_saving.port_status[port_num]);
}
