#include "robot_arm.h"

ROBOT_ARM robot_arm_data[DF_TY_USB_PORT_N_MAX];
ROBOT_ARM_CALSS robot_arm_class;

#define ROBOT_ARM_DEFINE 0

int robot_arm_device_init(int port,char *busid);
int robot_arm_device_deinit(int port);
int robot_arm_set_usb_power(int port,PORT_POWER_STATE power_state);
int robot_arm_action(int port);
int robot_arm_action_only(int port);
int robot_arm_get_port_state(int port,int *ad_value);
int robot_arm_get_port_ad(int port);
int robot_arm_set_busid(int port,char *busid);
char *robot_arm_get_busid(int port);
int robot_arm_get_version(int port,char *version);
int init_port_state_buff(int port,char *buff);
int judge_robot_arm(int port);
int robot_arm_get_port_update_state(int port);

int start_robot_arm_update(int port,char *version,int *report_flag);
int robot_arm_update_thread(void *arg, int timer);

int robot_arm_open_hid(HUSB_LOCAL hUSB, int nBusNum, int nDevNum);

int robot_arm_set_terid(int port,char *terid);
int robot_arm_get_terid(int port,char *terid);

//机械臂模块初始化
int robot_arm_model_init(MODULE *module,int max_port_num) //模块管理指针
{
	int i = 0;
	char name[32] = {0x00};
	memset(robot_arm_data,0x00,robot_arm_class.max_port_num*sizeof(ROBOT_ARM));
	memset(&robot_arm_class,0x00,sizeof(ROBOT_ARM_CALSS));
	robot_arm_class.max_port_num = max_port_num;
	robot_arm_class.robot_arm_data_barry = robot_arm_data;
	robot_arm_class.fun_dev_init = robot_arm_device_init;
	robot_arm_class.fun_set_busid = robot_arm_set_busid;
	robot_arm_class.fun_get_busid = robot_arm_get_busid;
	robot_arm_class.fun_dev_deinit = robot_arm_device_deinit;
	robot_arm_class.fun_arm_action = robot_arm_action;
	robot_arm_class.fun_arm_action_only = robot_arm_action_only;
	robot_arm_class.fun_set_power = robot_arm_set_usb_power;
	robot_arm_class.fun_get_port_state = robot_arm_get_port_state;
	robot_arm_class.fun_get_port_ad = robot_arm_get_port_ad;
	robot_arm_class.fun_get_version = robot_arm_get_version;
	robot_arm_class.fun_start_update = start_robot_arm_update;
	robot_arm_class.fun_port_state_buff_init = init_port_state_buff;
	robot_arm_class.fun_judge_robot_arm = judge_robot_arm;
	robot_arm_class.fun_arm_get_port_update_state = robot_arm_get_port_update_state;
	robot_arm_class.fun_get_terid = robot_arm_get_terid;
	robot_arm_class.fun_set_terid = robot_arm_set_terid;
	robot_arm_class.module = module;
	robot_arm_class.update_lock = _lock_open("robot_update",NULL);
	for(i = 0;i<robot_arm_class.max_port_num;++i)
	{
		memset(name,0x00,sizeof(name));
		sprintf(name,"robot_arm_%d",i);
		robot_arm_data[i].lock = _lock_open(name,NULL);
	}
	_scheduled_task_open_set_size("r_a_update_thread",robot_arm_update_thread,NULL,100,NULL,TASK_SIZE_500K);
	return 0;
}

ROBOT_ARM_CALSS *get_robot_arm_class(void)
{
	return &robot_arm_class;
}

int init_port_state_buff(int port,char *buff)
{
	// char port_status_buff[MAX_PORT_NUM] = {1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0};
	// memcpy(buff,port_status_buff,MAX_PORT_NUM);
	return 0;
}

int judge_robot_arm(int port)
{
	//printf("judge_robot_arm port = %d\n",port);
	if(port%2 == 0)
		return 0;
	return -1;
}

//某个机械臂的初始化
int robot_arm_device_init(int port,char *busid)
{
	int device_no = 0;
	int nhUsbType = 0,nBusID = 0,nDevID = 0;
	//char access_path[128] = "/sys/bus/usb/device/";
	if(0 > port || robot_arm_class.max_port_num < port)
	{
		return -1;
	}
	// device_no = get_device_no(port);
	device_no = port;
	if(0 > device_no || robot_arm_class.max_port_num < device_no)
	{
		return -1;
	}
	//strcat(access_path,busid);
	//printf("robot_arm_device_init busid = %s\n",access_path);
	// if(access(access_path,0) != 0)
	// {
	// 	printf("access(access_path,0) != 0  -----%d\n",port);
	// 	return -1;
	// }
	//  if(0 > ty_ctl(robot_arm_class.module->ty_usb_fd, DF_TY_USB_CM_GET_PORT_FILE_EXIST, port + 1)) //无设备
	//  {
	// 	return -1;
	//  }
	if(strlen(busid) == 0)
		return -1;
	//memset(&robot_arm_data[device_no],0x00,sizeof(ROBOT_ARM));
	nBusID = get_busid_busnum(busid);
	printf("robot_arm_device_init nBusID = %d\n",nBusID);
	nDevID = get_busid_devnum(busid);
	printf("robot_arm_device_init nDevID = %d\n",nDevID);
	nhUsbType = robot_arm_open_hid(&robot_arm_data[device_no].usb_session,nBusID,nDevID);
	if (nhUsbType != DEVICE_ROBOT_ARM_LOCAL)
	{
		return -1;
	}
	//printf("robot_arm_device_init successful = %d\n",port);
	return 0;
}

//某个机械臂的反初始化
int robot_arm_device_deinit(int port)
{
	int device_no = 0;
	//printf("enter robot_arm_device_deinit = %d\n",port);
	if(0 > port || robot_arm_class.max_port_num < port)
	{
		return -1;
	}
	device_no = get_device_no(port);
	if(0 > device_no || robot_arm_class.max_port_num < device_no)
	{
		return -1;
	}
	if (NULL != robot_arm_data[device_no].usb_session.handle)
	{
		libusb_close(robot_arm_data[device_no].usb_session.handle);
		//printf("robot_arm_device_deinit111111\n");
		robot_arm_data[device_no].usb_session.handle = NULL;
	}
	if(NULL != robot_arm_data[device_no].usb_session.context)
	{
		libusb_exit(robot_arm_data[device_no].usb_session.context);
		//printf("robot_arm_device_deinit222222\n");
		robot_arm_data[device_no].usb_session.context = NULL;
	}
	memset(robot_arm_data[device_no].version,0x00,sizeof(robot_arm_data[device_no].version));
	//printf("exit robot_arm_device_deinit = %d\n",port);
	//memset(robot_arm_data,0x00,sizeof(ROBOT_ARM));
	return 0;
}

//获取busid
char *robot_arm_get_busid(int port)
{
	int device_no = 0;
	if(0 > port || robot_arm_class.max_port_num < port)
	{
		return -1;
	}
	device_no = get_device_no(port);
	if(0 > device_no || robot_arm_class.max_port_num < device_no)
	{
		return -1;
	}
	printf("robot_arm_get_busid bus_id = %s\n",robot_arm_data[device_no].bus_id);
	return robot_arm_data[device_no].bus_id;
}

//設置busid
int robot_arm_set_busid(int port,char *busid)
{
	int device_no = 0;
	if(0 > port || robot_arm_class.max_port_num < port)
	{
		return -1;
	}
	device_no = get_device_no(port);
	if(0 > device_no || robot_arm_class.max_port_num < device_no)
	{
		return -1;
	}
	memset(robot_arm_data[device_no].bus_id,0x00,sizeof(robot_arm_data[device_no].bus_id));
	strcpy(robot_arm_data[device_no].bus_id,busid);
	printf("robot_arm_set_busid robot_arm_data[device_no].bus_id = %s\n",robot_arm_data[device_no].bus_id);
	return 0;
}

//根据端口号获取设备号
int get_device_no(int port)
{
	//printf("get_device_no port=%d,device_no=%d\n",port,port/2);
	return port;
}

//机械臂打包函数
//end_flag 用于分包时判断是否是最后一包
int robot_arm_pack(uint8 order, uint8 end_flag,uint8 *in_buf, int in_len, uint8 *out_buf)
{
	uint16 len = 0;
	len = in_len + 6;
	out_buf[0] = 'M';
	out_buf[1] = 'B';
	out_buf[2] = ((len >> 8) & 0xff);
	out_buf[3] = (len & 0xff);
	out_buf[4] = order;
	out_buf[5] = end_flag;
	memcpy(out_buf + 6, in_buf, in_len);
	return (int)len;
}

libusb_device *robot_arm_find_device_ByID(libusb_device **devs, int nBusNum, int nDevNum)
{
	libusb_device *dev = NULL;
	libusb_device *devRet = NULL;
	int i = 0;
	while ((dev = devs[i++]) != NULL)
	{
		struct libusb_device_descriptor desc;
		int r = libusb_get_device_descriptor(dev, &desc);			//获取USB设备的设备描述符	//dev libusb_device的指针，是要读取的设备 desc 设备描述符的指针，用来带回设备描述符的结构
		if (r < 0)
			continue;
		if (libusb_get_bus_number(dev) == nBusNum && libusb_get_device_address(dev) == nDevNum)		//判断总线号和设备号
		{
			devRet = dev;
			break;
		}
	}
	return devRet;
}

int robot_arm_open_hid(HUSB_LOCAL hUSB, int nBusNum, int nDevNum)
{
	libusb_device **devs = NULL;
	int wToalLength = 0,bInterfaceNumber = 0,i = 0;
	unsigned char usb_data[2048];
	if ((nBusNum<1) || (nBusNum>127))
	{
		printf("nBusNum range error\n");
		return -1;
	}
	if ((nDevNum<1) || (nDevNum>127))
	{
		printf("nDevNum range error\n");
		return -1;
	}
	libusb_init(&(hUSB->context));
	int device_num = libusb_get_device_list(hUSB->context, &devs);
	if (device_num <= 0) {
		libusb_free_device_list(devs, 1);
		fprintf(stderr, "[Device] No device found on the bus.\n");
		return -1;
	}
	libusb_device *dev = robot_arm_find_device_ByID(devs, nBusNum, nDevNum);
	if(NULL == dev)
	{
		printf("robot_arm_find_device_ByID error\n");
		return -2;
	}
	ROBOT_ARM_LIBUSB_CHECK(libusb_open(dev, &(hUSB->handle)));
	if (libusb_kernel_driver_active(hUSB->handle, 0))			//确定指定接口的内核驱动程序是否已经激活。如果一个内核驱动程序是激活的，libusb_claim_interface调用的会失败
	{
		if (libusb_detach_kernel_driver(hUSB->handle, 0) < 0)	//卸载指定接口的内核驱动程序。如果一个内核驱动程序是激活的，必须先调用这个函数，再调用libusb_claim_interface
		{
			return -4;
		}
	}
	if (libusb_kernel_driver_active(hUSB->handle, 1))			//对双接口设备的兼容
	{
		if (libusb_detach_kernel_driver(hUSB->handle, 1) < 0)
		{
			return -4;
		}
	}
	ROBOT_ARM_LIBUSB_CHECK(libusb_reset_device(hUSB->handle));
	libusb_set_auto_detach_kernel_driver(hUSB->handle, 1);
	ROBOT_ARM_LIBUSB_CHECK(libusb_claim_interface(hUSB->handle, 0));
	libusb_set_auto_detach_kernel_driver(hUSB->handle, 1);
	if (libusb_control_transfer(hUSB->handle, 0x80, 0x06, 0x0100, 0, usb_data, 0x12, 3000) < 0)
	{
		//robot_arm_out("获取设备描述符失败\n");
	}
	//robot_arm_out("打印设备描述符\n");
	//printf_array((char *)usb_data, 0x12);
	//robot_arm_out("获取设备描述符成功\n");
	//robot_arm_out("获取配置描述符\n");
	memset(usb_data, 0, sizeof(usb_data));
	if (libusb_control_transfer(hUSB->handle, 0x80, 0x06, 0x0200, 0, usb_data, 0x09, 3000) < 0)
	{
		//robot_arm_out("第一次获取配置描述符失败\n");
	}
	//robot_arm_out("打印获取配置描述符\n");

	//printf_array((char *)usb_data, 0x09);
	//robot_arm_out("获取配置描述符成功\n");
	wToalLength = usb_data[2];
	//robot_arm_out("获取到配置描述符总长度为：%d\n", wToalLength);
	memset(usb_data, 0, sizeof(usb_data));
	if (libusb_control_transfer(hUSB->handle, 0x80, 0x06, 0x0200, 0, usb_data, wToalLength, 3000) < 0)
	{
		//robot_arm_out("第二次获取配置描述符失败\n");
	}
	//robot_arm_out("打印获取到的描述符\n");

	//printf_array((char *)usb_data, wToalLength);
	int count = 0;
	for (i = 0; i < wToalLength; i++)
	{
		if ((usb_data[i] == 0x09) && ((usb_data[i + 1] == 0x04)))
		{
			bInterfaceNumber = usb_data[i + 2];
			//robot_arm_out("获取到的端点号 bInterfaceNumber 为：%02x,bNumEndpoints 为：%02x\n", bInterfaceNumber, stream->line[port_num - 1].bNumEndpoints);
			if (libusb_claim_interface(hUSB->handle, bInterfaceNumber) < 0)			//为指定的设备申请接口
			{
				return -7;
			}
			count++;
		}
	}




	libusb_free_device_list(devs, 1);
	return DEVICE_ROBOT_ARM_LOCAL;

	//if (nBusNum) {
	//    fprintf(stdout, "[Device] Assign busid devid, %d:%d\n", nBusNum, nDevNum);
	//} else {
	//    fprintf(stdout, "[Device] Auto detect mass storage device\n");
	//}
	// uint16 payload_d2h = 0;
	// uint16 payload_h2d = 0;
	// libusb_device **device_list = NULL;
	// libusb_init(&hUSB->context);
	// int device_num = libusb_get_device_list(hUSB->context, &device_list);
	// if (device_num <= 0) {
	// 	libusb_free_device_list(device_list, 1);
	// 	fprintf(stderr, "[Device] No device found on the bus.\n");
	// 	return -1;
	// }
	// int i = 0;
	// for (i = 0; i < device_num; i++) {
	// 	struct libusb_device_descriptor desc;
	// 	ROBOT_ARM_LIBUSB_CHECK(libusb_get_device_descriptor(device_list[i], &desc));
	// 	int nNowBus = libusb_get_bus_number(device_list[i]);
	// 	int nNowDev = libusb_get_device_address(device_list[i]);
	// 	if (nBusNum != 0 || nDevNum != 0) {
	// 		if (nNowBus != nBusNum || nNowDev != nDevNum) {
	// 			continue;
	// 		}
	// 	}
	// 	struct libusb_config_descriptor *conf_desc;
	// 	ROBOT_ARM_LIBUSB_CHECK(libusb_get_active_config_descriptor(device_list[i], &conf_desc));
	// 	unsigned int j = 0;
	// 	for (j = 0; j < conf_desc->bNumInterfaces; j++) {
	// 		const struct libusb_interface *interface = &conf_desc->interface[j];
	// 		if (interface->num_altsetting >= 1) {
	// 			const struct libusb_interface_descriptor *if_desc = &interface->altsetting[0];
	// 			/*if (if_desc->bInterfaceClass == LIBUSB_CLASS_MASS_STORAGE &&
	// 			if_desc->bInterfaceSubClass == 0x06 && if_desc->bInterfaceProtocol == 0x50) {*/
	// 			if (1){
	// 				unsigned int k = 0;
	// 				for (k = 0; k < if_desc->bNumEndpoints; k++) {
	// 					const struct libusb_endpoint_descriptor *ep_desc = &if_desc->endpoint[k];

	// 					if ((ep_desc->bmAttributes & 0x03) == LIBUSB_TRANSFER_TYPE_BULK) {
	// 						if (ep_desc->bEndpointAddress & 0x80) {
	// 							hUSB->endpoint_d2h = ep_desc->bEndpointAddress;
	// 							payload_d2h = ep_desc->wMaxPacketSize;
	// 						}
	// 						else {
	// 							hUSB->endpoint_h2d = ep_desc->bEndpointAddress;
	// 							payload_h2d = ep_desc->wMaxPacketSize;
	// 						}
	// 					}
	// 				}
	// 				//printf("payload_d2h = %d,payload_h2d = %d\n", payload_d2h, payload_h2d);
	// 				/*if (hUSB->endpoint_d2h == 0 || hUSB->endpoint_h2d == 0) {
	// 				fprintf(stderr, "[Device] Interface is not complete.\n");
	// 				libusb_free_config_descriptor(conf_desc);
	// 				libusb_free_device_list(device_list, 1);
	// 				return -2;
	// 				}*/

	// 				ROBOT_ARM_LIBUSB_CHECK(libusb_open(device_list[i], &(hUSB->handle)));
	// 				ROBOT_ARM_LIBUSB_CHECK(libusb_reset_device(hUSB->handle));
	// 				//libusb_set_auto_detach_kernel_driver(hUSB->handle, 1);
	// 				ROBOT_ARM_LIBUSB_CHECK(libusb_claim_interface(hUSB->handle, 0));

	// 				//printf("usb_device_open_hid_local hUSB->handle = %x\n",hUSB->handle);

	// 				// libusb_set_auto_detach_kernel_driver(hUSB->handle, 1);
	// 				// if (libusb_reset_device(hUSB->handle) < 0) {
	// 				// 	fprintf(stderr, "[Device] libusb_reset_device reset failed\n");
	// 				// 	libusb_free_config_descriptor(conf_desc);
	// 				// 	libusb_free_device_list(device_list, 1);
	// 				// 	libusb_close(hUSB->handle);
	// 				// 	return -3;
	// 				// }


	// 				// uint8 szManufacturer[256] = "";
	// 				// if (libusb_get_string_descriptor_ascii(hUSB->handle, desc.iManufacturer,szManufacturer, 256) < 0)
	// 				// {
	// 				// 	fprintf(stderr, "[Device] libusb_get_string_descriptor_ascii failed\n");
	// 				// 	libusb_free_config_descriptor(conf_desc);
	// 				// 	libusb_free_device_list(device_list, 1);
	// 				// 	libusb_close(hUSB->handle);
	// 				// 	return -1;
	// 				// }

	// 				int nDeviceType = -1;
	// 				if ((desc.idVendor == 0x0680) && (desc.idProduct == 0x1901))
	// 				{
	// 					nDeviceType = DEVICE_MENGBAI_LOCAL;
	// 				}
	// 				else if ((desc.idVendor == 0x0680) && (desc.idProduct == 0x2008))
	// 				{
	// 					nDeviceType = DEVICE_MENGBAI2_LOCAL;
	// 				}
	// 				else if ((desc.idVendor == ROBOT_ARM_VID) && (desc.idProduct == ROBOT_ARM_PID))
	// 				{
	// 					nDeviceType = DEVICE_ROBOT_ARM_LOCAL;
	// 				}
	// 				else
	// 					nDeviceType = DEVICE_UNKNOW_LOCAL;

	// 				libusb_free_config_descriptor(conf_desc);
	// 				libusb_free_device_list(device_list, 1);
	// 				//printf("exit usb_device_open_hid_local\n");
	// 				return nDeviceType;
	// 			}
	// 		}
	// 	}
	// 	libusb_free_config_descriptor(conf_desc);
	// }
	// libusb_free_device_list(device_list, 1);
	// fprintf(stderr, "[Device] No target device found.\n");
	// return -4;
}


//机械臂发送、接收数据
int robot_arm_send_recv(int device_no,unsigned char *indata, int in_len, unsigned char *outdata, int *out_len)
{
	int i = 0,j = 0,num = 0,result = -1,size = 0,total_len = 0,left_len = 0,index = 0;
	unsigned char r_data[ROBOT_ARM_BUFF_LEN] = {0x00};
	unsigned char s_data[50] = {0x00};
	char usbpath[DF_TY_PATH_MAX] = {0x00};
	if(0 > device_no || robot_arm_class.max_port_num < device_no || NULL == robot_arm_data[device_no].usb_session.handle)
	{
		if(NULL == robot_arm_data[device_no].usb_session.handle)
		{
			robot_arm_device_init(device_no,robot_arm_data[device_no].bus_id);	
			printf("robot_arm_send_recv   NULL == robot_arm_data[%d].usb_session.handle\n",device_no);
			sprintf(usbpath,"/sys/bus/usb/devices/%s",robot_arm_data[device_no].bus_id);
			result = access(usbpath,0);
			printf("!!!!!!!!!!!!! robot_arm_send_recv device_no = %d,usbpath = %s,result = %d\n",device_no,usbpath,result);
			if(access(usbpath,0) >= 0)  //设备还存在
			{
				if(robot_arm_device_deinit(device_no) < 0)   //反初始化
				{
					robot_arm_out("111111数据发送失败,result = %d\n", result);
					return -2;
				}
				if(robot_arm_device_init(device_no,robot_arm_data[device_no].bus_id) < 0)
				{
					robot_arm_out("222222数据发送失败,result = %d\n", result);
					return -2;
				}
				result = libusb_interrupt_transfer(robot_arm_data[device_no].usb_session.handle, 0x01, indata + i * 64, 64, &size, 4000);
				if (result < 0)
				{
					robot_arm_out("3333333数据发送失败,result = %d\n", result);
					return -2;
				}
			}
		}
		return -2;
	}
	memset(r_data, 0, ROBOT_ARM_BUFF_LEN);
	num = in_len / 64 + 1;
    // robot_arm_out("--------hid_usb send data:----------\n");
	// for(i = 0;i < in_len;++i)
	// {
	// 	printf("%02x ",indata[i]);
	// }
	// printf("\n");
	for (i = 0; i < num; i++)
	{
		result = libusb_interrupt_transfer(robot_arm_data[device_no].usb_session.handle, 0x01, indata + i * 64, 64, &size, 4000);
		if (result < 0)
		{
			if(result == -4)   //no device
			{
				sprintf(usbpath,"/sys/bus/usb/devices/%s",robot_arm_data[device_no].bus_id);
				
				result = access(usbpath,0);
				printf("!!!!!!!!!!!!! robot_arm_send_recv device_no = %d,usbpath = %s,result = %d\n",device_no,usbpath,result);
				if(access(usbpath,0) >= 0)  //设备还存在
				{
					if(robot_arm_device_deinit(device_no) < 0)   //反初始化
					{
						robot_arm_out("111111数据发送失败,result = %d\n", result);
						return -2;
					}
					if(robot_arm_device_init(device_no,robot_arm_data[device_no].bus_id) < 0)
					{
						robot_arm_out("222222数据发送失败,result = %d\n", result);
						return -2;
					}
					result = libusb_interrupt_transfer(robot_arm_data[device_no].usb_session.handle, 0x01, indata + i * 64, 64, &size, 4000);
					if (result < 0)
					{
						robot_arm_out("3333333数据发送失败,result = %d\n", result);
						return -2;
					}
				}
			}
			else
			{
				robot_arm_out("数据发送失败,result = %d\n", result);
				return -2;
			}
		}
	}
	result = libusb_interrupt_transfer(robot_arm_data[device_no].usb_session.handle, 0x82, r_data, 64, &size, 10000);
	if (result < 0)
	{
		robot_arm_out("数据接收失败,result = %d，device_no = %d\n", result,device_no);
		return -1;
	}
	// robot_arm_out("--------hid_usb recv data:----------\n");
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
	// left_len = total_len - 64;
	// robot_arm_out("total_len = %d,left_len=%d\n",total_len,left_len);
	if (total_len > *out_len)
	{
		robot_arm_out("接收缓冲去内存太小,total_len=%d,out_len=%d\n",total_len,out_len);
		return -1;
	}
	// if (left_len > 0)
	// {
	// 	num = (left_len / 22) + 1;
	// 	for (i = 0; i < num; i++)
	// 	{
	// 		memset(s_data, 0, sizeof(s_data));
	// 		libusb_interrupt_transfer(robot_arm_data[device_no].usb_session.handle, 0x82, s_data, 22, &size, 1000);
	// 		printf("--------hid_usb recv data %d,size = %d:----------\n",i,size);
	// 		for(j = 0;j < size;++j)
	// 		{
	// 			printf("%02x ",s_data[j]);
	// 		}
	// 		printf("\n");
	// 		memcpy(r_data + (22 * (i + 1)), s_data, size);
	// 		index += size;
	// 	}
	// }
	// else
	// {
	// 	num = 0;
	// }
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

//获取此端口是否处于升级中
int robot_arm_get_port_update_state(int port)
{
	int device_no = 0;
	device_no = get_device_no(port);
	printf("robot_arm_get_port_update_state device_no=%d,update_state = %d\n",device_no,robot_arm_data[device_no].update_data.update_state);
	if(WAIT_UPDATE < robot_arm_data[device_no].update_data.update_state && UPDATE_FAILED > robot_arm_data[device_no].update_data.update_state)  //处于升级中
	{
		printf("robot_arm_get_port_update_state -1\n");
		return -1;
	}
	return 0;
}

//----------------------------------------业务相关接口------------------------------------------------//
//机械臂设置usb端口电源
//入参：端口号，需要转换成对应的device_nomber
int robot_arm_set_usb_power(int port,PORT_POWER_STATE power_state)
{
	unsigned char send_data[ROBOT_ARM_BUFF_LEN] = {0x00};
	unsigned char recv_data[ROBOT_ARM_BUFF_LEN] = {0x00};
	int send_len = 0,rec_len = ROBOT_ARM_BUFF_LEN,device_no = 0,result = 0,i = 0;
	if(0 > port || robot_arm_class.max_port_num < port)
	{
		printf("robot_arm_set_usb_power port = %d,max_port_num = %d\n",port,robot_arm_class.max_port_num);
		return -1;
	}
	device_no = get_device_no(port);
	if(0 > device_no || robot_arm_class.max_port_num < device_no)
	{
		printf("robot_arm_set_usb_power device_no = %d\n",device_no);
		return -1;
	}
	if(WAIT_UPDATE < robot_arm_data[device_no].update_data.update_state && UPDATE_FAILED > robot_arm_data[device_no].update_data.update_state)
	{
		return -2;
	}
	// if(power_state == POWER_CLOES_STATE && port == 21)
	// {
	// 	printf("lbc-debug 21port\n");
	// 	return 0;
	// }
	_lock_set(robot_arm_data[device_no].lock);
	recv_data[0] = (unsigned char)power_state;
	send_len = robot_arm_pack(ROBOT_ARM_SET_PORT_POWER_STATE,0,recv_data,1,send_data);
#if ROBOT_ARM_DEFINE
	if(0 > robot_arm_device_init(port,robot_arm_data[device_no].bus_id))
	{
		_lock_un(robot_arm_data[device_no].lock);
		return -1;
	}
#endif
	// robot_arm_out("--------robot_arm_set_usb_power send data:----------\n");
	// for(i = 0;i < send_len;++i)
	// {
	// 	printf("%02x ",send_data[i]);
	// }
	// printf("\n");
	result = robot_arm_send_recv(device_no,send_data,send_len,recv_data,&rec_len);
	if(ROBOT_ARM_SET_PORT_POWER_STATE !=recv_data[4])
	{
		_lock_un(robot_arm_data[device_no].lock);
		printf("robot_arm_set_usb_power recv_data[4] = %d\n",recv_data[4]);
		return -1;
	}
	// robot_arm_out("--------robot_arm_set_usb_power recv data:----------\n");
	// for(i = 0;i < rec_len;++i)
	// {
	// 	printf("%02x ",recv_data[i]);
	// }
	// printf("\n");
#if ROBOT_ARM_DEFINE
    robot_arm_device_deinit(port);
#endif
	if(0 < result)
	{
		if(recv_data[6] == 's')
		{
			_lock_un(robot_arm_data[device_no].lock);
			return 0;
		}
	}
	_lock_un(robot_arm_data[device_no].lock);
	return -1;
}


int robot_arm_set_terid(int port,char *terid)
{
	unsigned char send_data[ROBOT_ARM_BUFF_LEN] = {0x00};
	unsigned char recv_data[ROBOT_ARM_BUFF_LEN] = {0x00};
	int send_len = 0,rec_len = ROBOT_ARM_BUFF_LEN,device_no = 0,result = 0,i = 0;
	if(0 > port || robot_arm_class.max_port_num < port)
	{
		return -1;
	}
	device_no = get_device_no(port);
	if(0 > device_no || robot_arm_class.max_port_num < device_no)
	{
		return -1;
	}
	if(WAIT_UPDATE < robot_arm_data[device_no].update_data.update_state && UPDATE_FAILED > robot_arm_data[device_no].update_data.update_state)
	{
		return -2;
	}
	// if(power_state == POWER_CLOES_STATE && port == 21)
	// {
	// 	printf("lbc-debug 21port\n");
	// 	return 0;
	// }
	_lock_set(robot_arm_data[device_no].lock);
	memcpy(recv_data,terid,12);
	send_len = robot_arm_pack(ROBOT_ARM_SET_TERID,0,recv_data,12,send_data);
#if ROBOT_ARM_DEFINE
	if(0 > robot_arm_device_init(port,robot_arm_data[device_no].bus_id))
	{
		_lock_un(robot_arm_data[device_no].lock);
		return -1;
	}
#endif
	robot_arm_out("--------robot_arm_set_usb_power send data:----------\n");
	for(i = 0;i < send_len;++i)
	{
		printf("%02x ",send_data[i]);
	}
	printf("\n");
	result = robot_arm_send_recv(device_no,send_data,send_len,recv_data,&rec_len);
	// if(ROBOT_ARM_SET_PORT_POWER_STATE !=recv_data[4])
	// {
	// 	_lock_un(robot_arm_data[device_no].lock);
	// 	return -1;
	// }
	robot_arm_out("--------robot_arm_set_terid recv data:----------\n");
	for(i = 0;i < rec_len;++i)
	{
		printf("%02x ",recv_data[i]);
	}
	printf("\n");
#if ROBOT_ARM_DEFINE
    robot_arm_device_deinit(port);
#endif
	// if(0 < result)
	// {
	// 	if(recv_data[6] == 's')
	// 	{
	// 		_lock_un(robot_arm_data[device_no].lock);
	// 		return 0;
	// 	}
	// }
	_lock_un(robot_arm_data[device_no].lock);
	return 0;
}


int robot_arm_get_terid(int port,char *terid)
{
	unsigned char send_data[ROBOT_ARM_BUFF_LEN] = {0x00};
	unsigned char recv_data[ROBOT_ARM_BUFF_LEN] = {0x00};
	int send_len = 0,rec_len = ROBOT_ARM_BUFF_LEN,device_no = 0,result = 0,i = 0;
	if(0 > port || robot_arm_class.max_port_num < port)
	{
		return -1;
	}
	device_no = get_device_no(port);
	if(0 > device_no || robot_arm_class.max_port_num < device_no)
	{
		return -1;
	}
	if(WAIT_UPDATE < robot_arm_data[device_no].update_data.update_state && UPDATE_FAILED > robot_arm_data[device_no].update_data.update_state)
	{
		return -2;
	}
	// if(power_state == POWER_CLOES_STATE && port == 21)
	// {
	// 	printf("lbc-debug 21port\n");
	// 	return 0;
	// }
	_lock_set(robot_arm_data[device_no].lock);
	
	send_len = robot_arm_pack(ROBOT_ARM_GET_TERID,0,recv_data,0,send_data);
#if ROBOT_ARM_DEFINE
	if(0 > robot_arm_device_init(port,robot_arm_data[device_no].bus_id))
	{
		_lock_un(robot_arm_data[device_no].lock);
		return -1;
	}
#endif
	robot_arm_out("--------robot_arm_get_terid send data:----------\n");
	for(i = 0;i < send_len;++i)
	{
		printf("%02x ",send_data[i]);
	}
	printf("\n");
	result = robot_arm_send_recv(device_no,send_data,send_len,recv_data,&rec_len);
	// if(ROBOT_ARM_SET_PORT_POWER_STATE !=recv_data[4])
	// {
	// 	_lock_un(robot_arm_data[device_no].lock);
	// 	return -1;
	// }
	robot_arm_out("--------robot_arm_get_terid recv data:----------\n");
	for(i = 0;i < rec_len;++i)
	{
		printf("%02x ",recv_data[i]);
	}
	printf("\n");
#if ROBOT_ARM_DEFINE
    robot_arm_device_deinit(port);
#endif
	// if(0 < result)
	// {
	// 	if(recv_data[6] == 's')
	// 	{
	// 		_lock_un(robot_arm_data[device_no].lock);
	// 		return 0;
	// 	}
	// }
	_lock_un(robot_arm_data[device_no].lock);
	return 0;
}

//对应机械臂动作一次
//入参：对应的device_nomber
int robot_arm_action(int port)
{
	unsigned char send_data[ROBOT_ARM_BUFF_LEN] = {0x00};
	unsigned char recv_data[ROBOT_ARM_BUFF_LEN] = {0x00};
	int send_len = 0,rec_len = ROBOT_ARM_BUFF_LEN,result = 0;
	int device_no = 0;
	if(0 > port || robot_arm_class.max_port_num < port)
	{
		return -1;
	}
	device_no = get_device_no(port);
	if(0 > port || robot_arm_class.max_port_num < port)
	{
		return -1;
	}
	device_no = get_device_no(port);
	if(0 > device_no || robot_arm_class.max_port_num < device_no)
	{
		return -1;
	}
	if(WAIT_UPDATE < robot_arm_data[device_no].update_data.update_state && UPDATE_FAILED > robot_arm_data[device_no].update_data.update_state)
	{
		return -2;
	}
	_lock_set(robot_arm_data[device_no].lock);
	send_len = robot_arm_pack(ROBOT_ARM_ACTION,0,recv_data,0,send_data);
#if ROBOT_ARM_DEFINE
	if(0 > robot_arm_device_init(port,robot_arm_data[device_no].bus_id))
	{
		_lock_un(robot_arm_data[device_no].lock);
		return -1;
	}
#endif		
	result = robot_arm_send_recv(device_no,send_data,send_len,recv_data,&rec_len);
	if(ROBOT_ARM_ACTION !=recv_data[4])
	{
		_lock_un(robot_arm_data[device_no].lock);
		return -1;
	}
#if ROBOT_ARM_DEFINE
	 robot_arm_device_deinit(port);
#endif
	if(0 < result)
	{
		if(recv_data[6] == 's')
		{
			_lock_un(robot_arm_data[device_no].lock);
			return 0;
		}
		else
		{
			_lock_un(robot_arm_data[device_no].lock);
			return -1;
		}
	}
	_lock_un(robot_arm_data[device_no].lock);
	return result;
}

//对应机械臂动作一次,只动作，不检测有无设备
//入参：对应的device_nomber
int robot_arm_action_only(int port)
{
	unsigned char send_data[ROBOT_ARM_BUFF_LEN] = {0x00};
	unsigned char recv_data[ROBOT_ARM_BUFF_LEN] = {0x00};
	int send_len = 0,rec_len = ROBOT_ARM_BUFF_LEN,result = 0;
	int device_no = 0;
	if(0 > port || robot_arm_class.max_port_num < port)
	{
		return -1;
	}
	device_no = get_device_no(port);
	if(0 > port || robot_arm_class.max_port_num < port)
	{
		return -1;
	}
	device_no = get_device_no(port);
	if(0 > device_no || robot_arm_class.max_port_num < device_no)
	{
		return -1;
	}
	if(WAIT_UPDATE < robot_arm_data[device_no].update_data.update_state && UPDATE_FAILED > robot_arm_data[device_no].update_data.update_state)
	{
		return -2;
	}
	_lock_set(robot_arm_data[device_no].lock);
	send_len = robot_arm_pack(ROBOT_ARM_JUST_ACTION,0,recv_data,0,send_data);
#if ROBOT_ARM_DEFINE
	if(0 > robot_arm_device_init(port,robot_arm_data[device_no].bus_id))
	{
		_lock_un(robot_arm_data[device_no].lock);
		return -1;
	}
#endif		
	result = robot_arm_send_recv(device_no,send_data,send_len,recv_data,&rec_len);
	if(ROBOT_ARM_JUST_ACTION !=recv_data[4])
	{
		_lock_un(robot_arm_data[device_no].lock);
		return -1;
	}
#if ROBOT_ARM_DEFINE
	 robot_arm_device_deinit(port);
#endif
	if(0 < result)
	{
		if(recv_data[6] == 's')
		{
			_lock_un(robot_arm_data[device_no].lock);
			return 0;
		}
		else
		{
			_lock_un(robot_arm_data[device_no].lock);
			return -1;
		}
	}
	_lock_un(robot_arm_data[device_no].lock);
	return result;
}

//获取端口存在情况,有设备返回1，无设备返回0
int robot_arm_get_port_state(int port,int *ad_value)
{
	//printf("enter robot_arm_get_port_state = %d\n",port);
	unsigned char send_data[ROBOT_ARM_BUFF_LEN] = {0x00};
	unsigned char recv_data[ROBOT_ARM_BUFF_LEN] = {0x00};
	int send_len = 0,rec_len = ROBOT_ARM_BUFF_LEN,device_no = 0,result = 0;
	if(0 > port || robot_arm_class.max_port_num < port)
	{
		return -1;
	}
	device_no = get_device_no(port);
	if(0 > device_no || robot_arm_class.max_port_num < device_no)
	{
		return -1;
	}
	if(WAIT_UPDATE < robot_arm_data[device_no].update_data.update_state && UPDATE_FAILED > robot_arm_data[device_no].update_data.update_state)
	{
		return -2;
	}
	_lock_set(robot_arm_data[device_no].lock);
	send_len = robot_arm_pack(ROBOT_ARM_GET_PORT_STATE,0,recv_data,0,send_data);
	
#if ROBOT_ARM_DEFINE
	if(0 > robot_arm_device_init(port,robot_arm_data[device_no].bus_id))
	{
		_lock_un(robot_arm_data[device_no].lock);
		return -1;
	}
#endif

	result = robot_arm_send_recv(device_no,send_data,send_len,recv_data,&rec_len);
	if(ROBOT_ARM_GET_PORT_STATE !=recv_data[4])
	{
		_lock_un(robot_arm_data[device_no].lock);
		return -1;
	}
#if ROBOT_ARM_DEFINE
    robot_arm_device_deinit(port);
#endif
	if(0 < result)
	{
		*ad_value = recv_data[7]<<8 | recv_data[8];
		if(recv_data[6] == 's')
		{
			_lock_un(robot_arm_data[device_no].lock);
			return 1;
		}
		else
		{
			_lock_un(robot_arm_data[device_no].lock);
			return 0;
		}
	}
	_lock_un(robot_arm_data[device_no].lock);
	return -1;
}

//获取端口ad值
int robot_arm_get_port_ad(int port)
{
	unsigned char send_data[ROBOT_ARM_BUFF_LEN] = {0x00};
	unsigned char recv_data[ROBOT_ARM_BUFF_LEN] = {0x00};
	int send_len = 0,rec_len = ROBOT_ARM_BUFF_LEN,device_no = 0,result = 0,ad_value = 0;
	if(0 > port || robot_arm_class.max_port_num < port)
	{
		return -1;
	}
	device_no = get_device_no(port);
	if(0 > device_no || robot_arm_class.max_port_num < device_no)
	{
		return -1;
	}
	// if(strlen(robot_arm_data[device_no].bus_id) == 0)
	// {
	// 	return -1;
	// }
	if(WAIT_UPDATE < robot_arm_data[device_no].update_data.update_state && UPDATE_FAILED > robot_arm_data[device_no].update_data.update_state)
	{
		return -2;
	}
	_lock_set(robot_arm_data[device_no].lock);
	send_len = robot_arm_pack(ROBOT_ARM_GET_PORT_AD,0,recv_data,0,send_data);

#if ROBOT_ARM_DEFINE
	if(0 > robot_arm_device_init(port,robot_arm_data[device_no].bus_id))
	{
		_lock_un(robot_arm_data[device_no].lock);
		return -1;
	}
#endif

	result = robot_arm_send_recv(device_no,send_data,send_len,recv_data,&rec_len);
	if(ROBOT_ARM_GET_PORT_AD !=recv_data[4])
	{
		_lock_un(robot_arm_data[device_no].lock);
		return -1;
	}
#if ROBOT_ARM_DEFINE
	robot_arm_device_deinit(port);
#endif
	if(0 < result)
	{
		ad_value = recv_data[6]<<8 | recv_data[7];
		_lock_un(robot_arm_data[device_no].lock);
		return ad_value;
	}
	_lock_un(robot_arm_data[device_no].lock);
	return -1;
}


//-----------------------------------------------------升级相关------------------------------------------------//
//获取版本号
int robot_arm_get_version(int port,char *version)
{
	unsigned char send_data[ROBOT_ARM_BUFF_LEN] = {0x00};
	unsigned char recv_data[ROBOT_ARM_BUFF_LEN] = {0x00};
	int send_len = 0,rec_len = ROBOT_ARM_BUFF_LEN,device_no = 0,result = 0,ad_value = 0;
	if(0 > port || robot_arm_class.max_port_num < port)
	{
		printf("robot_arm_class.max_port_num = %d\n",robot_arm_class.max_port_num);
		return -1;
	}
	//device_no = get_device_no(port);
	device_no = port;
	if(0 > device_no || robot_arm_class.max_port_num < device_no)
	{
		return -1;
	}
	_lock_set(robot_arm_data[device_no].lock);
	send_len = robot_arm_pack(GET_VERSION,0,recv_data,0,send_data);
#if ROBOT_ARM_DEFINE
	if(0 > robot_arm_device_init(port,robot_arm_data[device_no].bus_id))
	{
		_lock_un(robot_arm_data[device_no].lock);
		return -1;
	}
#endif
	result = robot_arm_send_recv(device_no,send_data,send_len,recv_data,&rec_len);
	if(GET_VERSION !=recv_data[4])
	{
		_lock_un(robot_arm_data[device_no].lock);
		return -1;
	}
#if ROBOT_ARM_DEFINE
      robot_arm_device_deinit(port);
#endif
	if(0 < result)
	{
		memcpy(version,&recv_data[6],rec_len - 6);
		memcpy(robot_arm_data[device_no].version,&recv_data[6],rec_len - 6);
		//printf("-----------------------------version = %s\n",version);
		_lock_un(robot_arm_data[device_no].lock);
		return result;
	}
	_lock_un(robot_arm_data[device_no].lock);
	return -1;
}

//设置文件信息
int robot_arm_set_file_info(UPDATE_DATA *update_data)
{
	int i = 0,ret = 0;
	uint8 sum = 0;
	unsigned char send_data[ROBOT_ARM_BUFF_LEN] = {0x00};
	unsigned char recv_data[ROBOT_ARM_BUFF_LEN] = {0x00};
	int send_len = 0,rec_len = ROBOT_ARM_BUFF_LEN,device_no = 0,result = 0,ad_value = 0;
	if(0 > update_data->port || robot_arm_class.max_port_num < update_data->port)
	{
		return -1;
	}
	device_no = get_device_no(update_data->port);
	if(0 > device_no || robot_arm_class.max_port_num < device_no)
	{
		return -1;
	}
	update_data->file_info.file_size = _file_exsit_size(ROBOT_ARM_UPDATE_FILE_PATH);
	if(0 >= update_data->file_info.file_size)
	{
		return -1;
	}
	if(NULL != update_data->file_data)
	{
		free(update_data->file_data);
		update_data->file_data = NULL;
	}
	update_data->file_data = calloc(1,update_data->file_info.file_size + PACK_SIZE); //多出一包的剩余量
	if(NULL == update_data->file_data)
	{
		update_data->file_info.file_size = 0;
		return -1;
	}
	update_data->file_info.total_pack_num = (update_data->file_info.file_size + PACK_SIZE - 1)/PACK_SIZE; //算出总包数
	ret = read_file(ROBOT_ARM_UPDATE_FILE_PATH,update_data->file_data,update_data->file_info.file_size + PACK_SIZE);
	//printf_array(update_data->file_data,ret);
	if(ret != update_data->file_info.file_size)
	{
		update_data->file_info.file_size = 0;
		if(NULL != update_data->file_data)
		{
			free(update_data->file_data);
			update_data->file_data = NULL;
		}
		update_data->file_info.total_pack_num = 0;
		return -1;
	}
	for(i = 0;i<update_data->file_info.file_size;++i)
	{
		sum += update_data->file_data[i];
	}
	update_data->file_info.file_sum = sum;
	recv_data[0] = update_data->file_info.file_size >> 8;
	recv_data[1] = update_data->file_info.file_size & 0xff;
	recv_data[2] = update_data->file_info.file_sum;
	recv_data[3] = update_data->file_info.total_pack_num;
	printf("file_size = %d,file_sum = %d,total_pack_num = %d\n",update_data->file_info.file_size,update_data->file_info.file_sum,update_data->file_info.total_pack_num);
	send_len = robot_arm_pack(SET_FILE_INFO,0,recv_data,4,send_data);
	memset(recv_data,0x00,sizeof(recv_data));
	_lock_set(robot_arm_data[device_no].lock);
#if ROBOT_ARM_DEFINE
	if(0 > robot_arm_device_init(update_data->port,robot_arm_data[device_no].bus_id))
	{
		if(NULL != update_data->file_data)
		{
			free(update_data->file_data);
			update_data->file_data = NULL;
		}
		_lock_un(robot_arm_data[device_no].lock);
		return -1;
	}
#endif
	result = robot_arm_send_recv(device_no,send_data,send_len,recv_data,&rec_len);
	if(SET_FILE_INFO !=recv_data[4])
	{
		if(NULL != update_data->file_data)
		{
			free(update_data->file_data);
			update_data->file_data = NULL;
		}
		_lock_un(robot_arm_data[device_no].lock);
		return -1;
	}
#if ROBOT_ARM_DEFINE
	robot_arm_device_deinit(update_data->port);
#endif
	if(0 < result)
	{
		if(recv_data[6] == 's')  //设置成功
		{
			printf("robot_arm_set_file_info success!\n");
		}
		else
		{
			printf("robot_arm_set_file_info fialed!\n");
		}
		ret = recv_data[7]<<8 | recv_data[8];
		printf("file_size1 = %d,file_sum1 = %d,total_pack_sum1 = %d\n",ret,recv_data[9],recv_data[10]);
	}
	_lock_un(robot_arm_data[device_no].lock);
	//update_data->update_state = UPDATING;
	return result;
}

//下载文件，返回0说明所有升级包下载完成，返回1说明单个升级包下载完成
int robot_arm_down_load(UPDATE_DATA *update_data)
{
	int i = 0,j = 0,ret = 0,count = 0;
	unsigned char send_data[ROBOT_ARM_BUFF_LEN] = {0x00};
	unsigned char recv_data[ROBOT_ARM_BUFF_LEN] = {0x00};
	int send_len = 0,rec_len = ROBOT_ARM_BUFF_LEN,device_no = 0,result = 0,ad_value = 0;
	if(0 > update_data->port || robot_arm_class.max_port_num < update_data->port)
	{
		return -1;
	}
	device_no = get_device_no(update_data->port);
	if(0 > device_no || robot_arm_class.max_port_num < device_no)
	{
		return -1;
	}
	if(NULL == update_data->file_data)
	{
		return -1;
	}
	//每次发送升级包之前先查询升级掩码，发送第一包需要发送的升级包
	send_len = robot_arm_pack(GET_MASK,0,recv_data,0,send_data);
	_lock_set(robot_arm_data[device_no].lock);
#if ROBOT_ARM_DEFINE
	if(0 > robot_arm_device_init(update_data->port,robot_arm_data[device_no].bus_id))
	{
		_lock_un(robot_arm_data[device_no].lock);
		return -1;
	}
#endif
	result = robot_arm_send_recv(device_no,send_data,send_len,recv_data,&rec_len);
	if(GET_MASK !=recv_data[4])
	{
		_lock_un(robot_arm_data[device_no].lock);
		return -1;
	}
		
#if ROBOT_ARM_DEFINE
	//robot_arm_device_deinit(update_data->port);
#endif

	// if(0 < result)
	// {
	// 	//if(rec_len != update_data->file_info.total_pack_num)
	// 	{
	// 		return -1;
	// 	}
	// }
	if(result < 0)
	{
		_lock_un(robot_arm_data[device_no].lock);
		return -1;
	}
	memset(send_data,0x00,sizeof(send_data));
	printf("recv_data,rec_len=%d\n",rec_len - 6);
	for(i = 0;i<rec_len - 6;i++)
	{
		printf("%02x ",recv_data[i + 6]);
		for(j = 7;j>=0;j--)
		{
			if(0x01 == ((recv_data[i + 6]>>j)&0x01))
			{
				send_data[count++] = 0xff;
			}
			else
			{
				send_data[count++] = 0x00;
			}
		}
	}
	printf("\n");

	// printf("get mask,count=%d\n",count);
	// for(i = 0;i<count;i++)
	// {
	// 	printf("%02x ",send_data[i]);
	// }
	// printf("\n");

	
	for(i = 0;i<update_data->file_info.total_pack_num;i++)
	{
		if(0x00 == send_data[i])
			break;
	}
	printf("i =%d,total_pack_num = %d\n",i,update_data->file_info.total_pack_num);
	if(i == update_data->file_info.total_pack_num && i) //说明所有升级包都正确接收,升级完成
	{
		_lock_un(robot_arm_data[device_no].lock);
		return 0;
	}
	memset(recv_data,0x00,sizeof(recv_data));
	recv_data[0] = i;
	memcpy(&recv_data[1],update_data->file_data + i*PACK_SIZE,PACK_SIZE);
	//打包升级数据
	send_len = robot_arm_pack(DOWNLOAD,0,recv_data,PACK_SIZE + 1,send_data);
	memset(recv_data,0x00,sizeof(recv_data));
#if ROBOT_ARM_DEFINE
	if(0 > robot_arm_device_init(update_data->port,robot_arm_data[device_no].bus_id))
	{
		_lock_un(robot_arm_data[device_no].lock);
		return -1;
	}
#endif
	result = robot_arm_send_recv(device_no,send_data,send_len,recv_data,&rec_len);
	if(DOWNLOAD !=recv_data[4])
	{
		_lock_un(robot_arm_data[device_no].lock);
		return -1;
	}
#if ROBOT_ARM_DEFINE
	robot_arm_device_deinit(update_data->port);
#endif
	if(0 < result)
	{
		if(0 < rec_len)
		{
			if('s' == recv_data[6]) //此包下载成功
			{
				update_data->successed_pack_num++;
				update_data->update_percent = update_data->successed_pack_num*100/update_data->file_info.total_pack_num;   //升级百分比，进度
				_lock_un(robot_arm_data[device_no].lock);
				// if(update_data->successed_pack_num == update_data->file_info.total_pack_num)
				// {
				// 	update_data->update_state = GET_NEW_VERSION;
				// }
				return 1;
			}

		}
	}
	else
	{
		_lock_un(robot_arm_data[device_no].lock);
		return -1;
	}
	_lock_un(robot_arm_data[device_no].lock);
	return -11;
}

//机械臂升级处理  UPDATE_FAILED_MAX_NUM
int robot_arm_update_handle(void *arg, int timer)
{
	UPDATE_DATA *update = (UPDATE_DATA *)arg;
	int result = -1;
	if(NULL == update)
	{
		return -1;
	}
	_lock_set(robot_arm_class.update_lock);  //同时只能有一个机械臂进行升级操作
	if(update->update_failed_count > UPDATE_FAILED_MAX_NUM)
	{
		if(update->file_data != NULL)
		{
			free(update->file_data);
			update->file_data = NULL;
		}
		update->update_state = UPDATE_FAILED;
		_scheduled_task_close_self(update->update_fd); //升级失败，关闭此任务
		robot_arm_device_deinit(update->port);
		robot_arm_device_init(update->port,robot_arm_data[get_device_no(update->port)].bus_id);
		robot_arm_out("~~~~~~~~~~~~~~~~robot_arm_update_handle failed\n");
		robot_arm_class.updating = WAIT_UPDATE;
		_lock_un(robot_arm_class.update_lock);
		return -1;
	}
	switch(update->update_state)
	{
		case GET_OLD_VERSION:
		robot_arm_out("enter robot_arm_get_version old,port=%d\n",update->port);
		result = robot_arm_get_version(update->port,update->old_version);
		robot_arm_out("exit robot_arm_get_version old result = %d\n",result);
		if(0 > result)  //执行失败
		{
			update->update_failed_count++;
		}
		else
		{
			update->update_state = SET_FILE_INFO_CMD;
			update->update_failed_count = 0;
		}
		break;

		case SET_FILE_INFO_CMD:
		robot_arm_out("enter robot_arm_set_file_info,port=%d\n",update->port);
		result = robot_arm_set_file_info(update);
		robot_arm_out("exit robot_arm_set_file_info,result = %d\n",result);
		if(0 > result)  //执行失败
		{
			update->update_failed_count++;
		}
		else
		{
			update->update_state = DOWN_LOAD_FILE;
			update->update_failed_count = 0;
		}
		break;

		case DOWN_LOAD_FILE:
		robot_arm_out("enter robot_arm_down_load,port=%d\n",update->port);
		result = robot_arm_down_load(update);
		robot_arm_out("exit robot_arm_down_load,result = %d\n",result);
		if(0 > result)  //执行失败
		{
			update->update_failed_count++;
		}
		else if(0 == result) //所有升级包下载完成
		{
			update->update_state = GET_NEW_VERSION;
			update->update_failed_count = 0;
		}
		else if(1 == result) //单个数据包执行成功
		{
			//robot_arm_out("单个数据包下载执行成功\n");
		}
		break;

		case GET_NEW_VERSION:
		sleep(1);//等待单片机重启
		robot_arm_out("enter robot_arm_get_version new,port=%d\n",update->port);
		robot_arm_device_deinit(update->port);
		if(0 > robot_arm_device_init(update->port,robot_arm_data[get_device_no(update->port)].bus_id))
		{
			update->update_failed_count++;
		}
		else
		{
			result = robot_arm_get_version(update->port,update->new_version);
			robot_arm_out("exit robot_arm_get_version new result = %d\n",result);
			if(0 > result)  //执行失败
			{
				update->update_failed_count++;
			}
			else
			{
				if(0 != memcmp(update->new_version,update->old_version,strlen(update->new_version)>strlen(update->old_version)?strlen(update->new_version):strlen(update->old_version))) //版本号不同，升级成功
				{
					if(NULL != update->version_ptr)
					{
						memset(update->version_ptr,0x00,strlen(update->version_ptr));
						memcpy(update->version_ptr,update->new_version,strlen(update->new_version));
						*(update->report_flag) = 1;  //进行一次上报
					}
					update->update_state = UPDATE_SUCCESSED;
					update->update_failed_count = 0;
					robot_arm_class.updating = WAIT_UPDATE;
				}
				else
				{
					update->update_failed_count++;
				}
			}
		}
		break;

		case UPDATE_SUCCESSED:

		if(update->file_data != NULL)
		{
			free(update->file_data);
			update->file_data = NULL;
		}
		update->update_state = UPDATE_SUCCESSED;
		_scheduled_task_close_self(update->update_fd); //升级完成，关闭此任务
		robot_arm_out("~~~~~~~~~~~~~~~~robot_arm_update_handle success\n");
		//_lock_un(robot_arm_class.update_lock);
		break;

		default:
		break;
	}
	_lock_un(robot_arm_class.update_lock);
	return 0;
}

int robot_arm_update_thread(void *arg, int timer)
{
	int i = 0;
	char name[DF_MAX_SCHEDULED_TASK_NAME_LEN] = {0x00};
	for(i = 0;i<robot_arm_class.max_port_num;++i)
	{
		if(GET_OLD_VERSION == robot_arm_data[i].update_data.update_state)
		{
			if(robot_arm_class.updating == UPDATING)
				return 0;
			// printf("start update port = %d\n",i);
			sprintf(name,"update_robot_arm_%d",i);
			robot_arm_class.updating = UPDATING;
			robot_arm_data[i].update_data.update_fd = _scheduled_task_open_set_size(name,robot_arm_update_handle,(void *)(&robot_arm_data[i].update_data),100,NULL,TASK_SIZE_500K);
			return 0;
		}
	}
}

//开始某个机械臂的升级  _scheduled_task_close_self
int start_robot_arm_update(int port,char *version,int *report_flag)
{
	int device_no = 0;
	if(0 > port || robot_arm_class.max_port_num < port)
	{
		return -1;
	}
	device_no = get_device_no(port);
	if(0 > device_no || robot_arm_class.max_port_num < device_no)
	{
		return -1;
	}
	// if(0 != judge_robot_arm(port))
	// 	return -1;
	if(WAIT_UPDATE < robot_arm_data[device_no].update_data.update_state && UPDATE_FAILED > robot_arm_data[device_no].update_data.update_state)  //已处于升级中，直接返回
	{
		return 0;
	}
	// printf("start_robot_arm_update port = %d\n",port);
	robot_arm_data[device_no].update_data.update_state = GET_OLD_VERSION;
	robot_arm_data[device_no].update_data.port = port;
	robot_arm_data[device_no].update_data.update_failed_count = 0;
	robot_arm_data[device_no].update_data.version_ptr = version;
	robot_arm_data[device_no].update_data.report_flag = report_flag;
	// sprintf(name,"update_robot_arm_%d",port);
	// robot_arm_data[device_no].update_data.update_fd = _scheduled_task_open_set_size(name,robot_arm_update_handle,(void *)(&robot_arm_data[device_no].update_data),100,NULL,TASK_SIZE_500K);
	// if(0 > robot_arm_data[device_no].update_data.update_fd)
	// {
	// 	return -1;
	// }
	return 0;
}




