#ifndef _ROBOT_ARM_H
#define _ROBOT_ARM_H

#include "opt.h"
#include "tax_interface.h"

#define robot_arm_out(s,arg...)       				log_out("all.c",s,##arg) 

#define ROBOT_ARM_SET_PORT_POWER_STATE				0X01	//设置端口电源
#define ROBOT_ARM_ACTION							0X02	//机械臂动作
#define ROBOT_ARM_GET_PORT_STATE					0X03	//获取端口的设备存在情况
#define ROBOT_ARM_GET_PORT_AD						0X04	//获取端口的ad值
#define ROBOT_ARM_GET_TERID                         0X05    //获取机器编号
#define ROBOT_ARM_SET_TERID                         0X06    //设置机器编号
#define ROBOT_ARM_JUST_ACTION                       0X07    //机械臂只动作，不检测有无设备

//升级相关
#define GET_VERSION                                 0x21    //获取软件版本号
#define SET_FILE_INFO                               0x22    //设置文件信息
#define DOWNLOAD                                    0x23    //下载升级包
#define GET_MASK                                    0x24    //获取升级的验证码

#define ROBOT_ARM_NUM 								(DF_TY_USB_PORT_N_MAX)
#define ROBOT_ARM_BUFF_LEN 							1024

#define ROBOT_ARM_UPDATE_FILE_PATH   				"/etc/robot_arm.bin"
#define ROBOT_ARM_UPDATE_FLAG_FILE_PATH   			"/etc/robot_arm_update_falg_file"
#define ROBOT_ARM_UPDATE_VERSION_FILE_PATH   		"/etc/robot_arm_update_versionfile"
#define PACK_SIZE                                   256         //每包的大小

#define UPDATE_FAILED_MAX_NUM 						5			//最大失败次数

#define ROBOT_ARM_LIBUSB_CHECK(action)                                                                   \
do {                                                                                           \
    int ret = (action);                                                                        \
    if (ret != LIBUSB_SUCCESS) {                                                               \
		printf("ROBOT_ARM_LIBUSB_CHECK failed %d\n",ret);											   \
        return -1;                                                                             \
    }                                                                                          \
    break;                                                                                     \
} while (0)

typedef enum PORT_POWER_STATE_T
{
  POWER_CLOES_STATE = 0,
  POWER_OPEN_STATE
}PORT_POWER_STATE;

#pragma pack(4)
typedef struct FILE_INFO_T
{
    uint16 file_size;//文件大小
    uint8 file_sum;  //文件和校验
    uint8 total_pack_num;  //总的包数，每包大小256字节
}FILE_INFO;

typedef enum UPDATE_STATE_T
{
  WAIT_UPDATE = 0,
  GET_OLD_VERSION,
  SET_FILE_INFO_CMD,
  DOWN_LOAD_FILE,
  GET_NEW_VERSION,
  UPDATE_FAILED,
  UPDATE_SUCCESSED,
  UPDATING
}UPDATE_STATE;

#pragma pack(4)
typedef struct UPDATE_DATA_T
{
    FILE_INFO file_info;     //升级文件信息
	char old_version[32];	 //旧版本号
	char new_version[32];	 //新版本号
	char *file_data;		 //升级文件数据
	UPDATE_STATE update_state; //升级状态
	int successed_pack_num;	 //成功的包数
	int update_percent;		 //升级进度
	int port;				 //升级的端口号
	int update_failed_count; //升级时失败的次数
	int update_fd;			 //升级任务fd
	char *version_ptr;
	int *report_flag;		//上报标志
}UPDATE_DATA;

typedef int (*Fun_void)(void);
typedef int (*Fun_p1)(int );
typedef int (*Fun_p2)(int ,PORT_POWER_STATE );
typedef int (*Fun_p3)(int ,char *);
typedef int (*Fun_p5)(int ,char *,int *);
typedef char *(*Fun_p4)(int);
typedef int (*Fun_p6)(int ,int *);

typedef struct ROBOT_ARM_T
{
	struct USBSession_Local usb_session;
	char bus_id[32];
	char version[32];
	int lock;	//端口usb操作锁
	PORT_POWER_STATE power_state;
	UPDATE_DATA update_data;
}ROBOT_ARM;

typedef struct ROBOT_ARM_CLASS_T
{
	int max_port_num;
	ROBOT_ARM *robot_arm_data_barry;
	Fun_p1   fun_arm_action;
	Fun_p1   fun_arm_action_only;
	Fun_p6   fun_get_port_state;
	Fun_p1   fun_get_port_ad;
	Fun_p2   fun_set_power;
	Fun_p3   fun_dev_init;
	Fun_p3   fun_set_busid;
	Fun_p4   fun_get_busid;
	Fun_p1   fun_dev_deinit;
	Fun_p3   fun_get_version;
	Fun_p3   fun_port_state_buff_init;
	Fun_p5   fun_start_update;
	Fun_p1   fun_judge_robot_arm;
	Fun_p1   fun_arm_get_port_update_state;
	Fun_p3   fun_get_terid;
	Fun_p3   fun_set_terid;
	MODULE *module; //模块管理指针
	int 	 update_lock;	//升级操作锁
	UPDATE_STATE 	updating;		//正在升级

}ROBOT_ARM_CALSS;

int robot_arm_model_init(MODULE *module,int max_port_num);
ROBOT_ARM_CALSS *get_robot_arm_class(void);


int robot_arm_open_hid(HUSB_LOCAL hUSB, int nBusNum, int nDevNum);
libusb_device *robot_arm_find_device_ByID(libusb_device **devs, int nBusNum, int nDevNum);
int robot_arm_pack(uint8 order, uint8 end_flag,uint8 *in_buf, int in_len, uint8 *out_buf);

#endif
