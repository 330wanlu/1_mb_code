#ifndef _TEST_H
#define _TEST_H

#include "opt.h"
#include "robot_arm.h"

#define ROBOT_ARM_TEST_VALUE	 0x05  //获取test键值

#define TEST_DEV_PID 	0x5537	//pid
#define TEST_DEV_VID 	0x4348	//vid

typedef struct TEST_DATA_T
{
	struct USBSession_Local usb_session;
	char bus_id[32];
	int total_count;  //总次数
	int sucess_count;  //成功的次数
	int port;
}TEST_DATA;

typedef struct TEST_MODEL_CLASS_T
{
	int max_port_num;
	TEST_DATA *test_data_barry;
	Fun_p1   fun_get_push_value;
	Fun_p3   fun_dev_init;
	Fun_p3   fun_set_busid;
	Fun_p1   fun_dev_deinit;
	Fun_void fun_start_test;

}TEST_MODEL_CLASS;

int test_model_init(int max_port_num);
TEST_MODEL_CLASS *get_test_model_calss(void);

#endif
