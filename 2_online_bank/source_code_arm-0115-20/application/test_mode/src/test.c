#include "test.h"

TEST_DATA test_data[ROBOT_ARM_NUM];

TEST_MODEL_CLASS test_model_class;

int test_get_push_value(int port);
int test_set_busid(int port,char *busid);
int test_device_init(int port,char *busid);
int test_device_deinit(int port);
int start_test(void);

int test_model_init(int max_port_num)
{
	int i = 0;
	memset(test_data,0x00,ROBOT_ARM_NUM*sizeof(TEST_DATA));
	test_model_class.max_port_num = max_port_num;
	test_model_class.fun_dev_init = test_device_init;
	test_model_class.fun_dev_deinit = test_device_deinit;
	test_model_class.fun_set_busid = test_set_busid;
	test_model_class.fun_get_push_value = test_get_push_value;
	test_model_class.fun_start_test = start_test;
	for(i = 0;i<ROBOT_ARM_NUM;++i)
	{
		test_data[i].port = i;
	}
	test_model_class.test_data_barry = test_data;
	return 0;
}

TEST_MODEL_CLASS *get_test_model_calss(void)
{
	return &test_model_class;
}

int test_thread(void *arg, int o_timer)
{
	TEST_DATA *test_data = (TEST_DATA *)arg;
	if(NULL == test_data)
	{
		return -1;
	}
	
	if(0 < strlen(get_robot_arm_class()->robot_arm_data_barry[test_data->port].version))
	{
		test_data->total_count++;
		if(0 <= get_robot_arm_class()->fun_arm_action(test_data->port))   //操作成功
		{
			
			//if(0 <= test_get_push_value(test_data->port))  //判断按到了,成功
			{
				test_data->sucess_count++;
			}
			printf("test_thread success port = %d,total_count = %d,sucess_count=%d\n",test_data->port,test_data->total_count,test_data->sucess_count);
			return 0;
		}
	}
	printf("test_thread failed port = %d,total_count = %d,sucess_count=%d\n",test_data->port,test_data->total_count,test_data->sucess_count);
	return -1;
}

int start_test(void)
{
	int i = 0;
	for(i = 0;i<test_model_class.max_port_num;++i)  //test_model_class.max_port_num  && i != 5  && i != 17
	{
		// if(i%2 == 1)
			_scheduled_task_open_set_size("start_test",test_thread,(void *)(&(test_data[i])),3000,NULL,500*1024);
	}
	return 0;
}

//根据端口号获取设备号
int test_get_device_no(int port)
{
	return port;
}

//获取busid
char *test_get_busid(int port)
{
	int device_no = 0;
	if(0 > port || test_model_class.max_port_num < port)
	{
		return -1;
	}
	device_no = test_get_device_no(port);
	if(0 > device_no || ROBOT_ARM_NUM < device_no)
	{
		return -1;
	}
	printf("test_get_busid bus_id = %s\n",test_data[device_no].bus_id);
	return test_data[device_no].bus_id;
}

//设置busid
int test_set_busid(int port,char *busid)
{
	int device_no = 0;
	if(0 > port || test_model_class.max_port_num < port)
	{
		return -1;
	}
	device_no = test_get_device_no(port);
	if(0 > device_no || ROBOT_ARM_NUM < device_no)
	{
		return -1;
	}
	memset(test_data[device_no].bus_id,0x00,sizeof(test_data[device_no].bus_id));
	strcpy(test_data[device_no].bus_id,busid);
	printf("test_set_busid test_data[device_no].bus_id = %s\n",test_data[device_no].bus_id);
	return 0;
}

//某个自测设备的初始化
int test_device_init(int port,char *busid)
{
	int device_no = 0;
	int nhUsbType = 0,nBusID = 0,nDevID = 0;
	//char access_path[128] = "/sys/bus/usb/device/";
	if(0 > port || test_model_class.max_port_num < port)
	{
		return -1;
	}
	device_no = test_get_device_no(port);
	if(0 > device_no || ROBOT_ARM_NUM < device_no)
	{
		return -1;
	}
	if(strlen(busid) == 0)
		return -1;
	//memset(&test_data[device_no],0x00,sizeof(ROBOT_ARM));
	nBusID = get_busid_busnum(busid);
	nDevID = get_busid_devnum(busid);
	nhUsbType = robot_arm_open_hid(&test_data[device_no].usb_session,nBusID,nDevID);
	// if (nhUsbType != DEVICE_ROBOT_ARM_LOCAL)
	// {
	// 	return -1;
	// }
	//printf("test_device_init successful = %d,nhUsbType=%d\n",port,nhUsbType);
	return 0;
}

//自测设备的反初始化
int test_device_deinit(int port)
{
	int device_no = 0;
	if(0 > port || test_model_class.max_port_num < port)
	{
		return -1;
	}
	device_no = test_get_device_no(port);
	if(0 > device_no || ROBOT_ARM_NUM < device_no)
	{
		return -1;
	}
	if (NULL != test_data[device_no].usb_session.handle)
	{
		libusb_close(test_data[device_no].usb_session.handle);
		test_data[device_no].usb_session.handle = NULL;
	}
	if(NULL != test_data[device_no].usb_session.context)
	{
		libusb_exit(test_data[device_no].usb_session.context);
		test_data[device_no].usb_session.context = NULL;
	}
	return 0;
}

//机械臂发送、接收数据
int test_send_recv(int device_no,unsigned char *indata, int in_len, unsigned char *outdata, int *out_len)
{
	int i = 0,j = 0,num = 0,result = -1,size = 0,total_len = 0,left_len = 0,index = 0;
	unsigned char r_data[ROBOT_ARM_BUFF_LEN] = {0x00};
	unsigned char s_data[50] = {0x00};
	if(0 > device_no || ROBOT_ARM_NUM < device_no || NULL == test_data[device_no].usb_session.handle)
	{
		if(NULL == test_data[device_no].usb_session.handle)
		{
			//test_device_init(device_no*2,test_data[device_no].bus_id);
			printf("test_send_recv   NULL == test_data[%d].usb_session.handle\n",device_no);
		}
		return -1;
	}
	memset(r_data, 0, ROBOT_ARM_BUFF_LEN);
	num = in_len / 64 + 1;
	for (i = 0; i < num; i++)
	{
		result = libusb_bulk_transfer(test_data[device_no].usb_session.handle, 0x01, indata + i * 64, 64, &size, 4000);
		if (result < 0)
		{
			robot_arm_out("数据发送失败,result = %d\n", result);
			return -1;
		}
	}
	result = libusb_bulk_transfer(test_data[device_no].usb_session.handle, 0x82, r_data, 64, &size, 4000);
	if (result < 0)
	{
		robot_arm_out("数据接收失败,result = %d，device_no = %d\n", result,device_no);
		return -1;
	}
	// robot_arm_out("--------test_send_recv recv data:----------\n");
	// for(i = 0;i < size;++i)
	// {
	// 	printf("%02x ",r_data[i]);
	// }
	// printf("\n");
	if ((r_data[0] != 0x4d) || (r_data[1] != 0x42))
	{
		robot_arm_out("版本号错误\n");
		return -1;
	}
	total_len = r_data[2] * 0x100 + r_data[3];
	if (total_len > *out_len)
	{
		robot_arm_out("接收缓冲去内存太小,total_len=%d,out_len=%d\n",total_len,out_len);
		return -1;
	}
	*out_len = total_len;
	memcpy(outdata, r_data, total_len);
	// printf("--------hid_usb recv data:----------\n");
	// for(i = 0;i < total_len - 6;++i)
	// {
	// 	printf("%02x ",outdata[i]);
	// }
	// printf("\n");
	return *out_len;
}

int test_get_push_value(int port)
{
	unsigned char send_data[ROBOT_ARM_BUFF_LEN] = {0x00};
	unsigned char recv_data[ROBOT_ARM_BUFF_LEN] = {0x00};
	int send_len = 0,rec_len = ROBOT_ARM_BUFF_LEN,result = 0;
	int device_no = 0;
	if(0 > port || test_model_class.max_port_num < port)
	{
		return -1;
	}
	device_no = test_get_device_no(port);
	if(0 > device_no || ROBOT_ARM_NUM < device_no)
	{
		return -1;
	}
	send_len = robot_arm_pack(ROBOT_ARM_TEST_VALUE,0,recv_data,0,send_data);	
	test_device_init(port,test_data[device_no].bus_id);
	result = test_send_recv(device_no,send_data,send_len,recv_data,&rec_len);
	test_device_deinit(port);
	if(ROBOT_ARM_TEST_VALUE !=recv_data[4])
	{
		return -1;
	}
	if(0 < result)
	{
		if(recv_data[6] == 0x01)
		{
			return 0;
		}
		else
		{
			return -1;
		}
	}
	return -1;
}





