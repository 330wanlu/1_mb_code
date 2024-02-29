#ifndef _ROBOT_ARM_H
#define _ROBOT_ARM_H

#include "opt.h"
#include "tax_interface.h"

#define robot_arm_out(s,arg...)       				log_out("all.c",s,##arg) 

#define ROBOT_ARM_SET_PORT_POWER_STATE				0X01	//���ö˿ڵ�Դ
#define ROBOT_ARM_ACTION							0X02	//��е�۶���
#define ROBOT_ARM_GET_PORT_STATE					0X03	//��ȡ�˿ڵ��豸�������
#define ROBOT_ARM_GET_PORT_AD						0X04	//��ȡ�˿ڵ�adֵ
#define ROBOT_ARM_GET_TERID                         0X05    //��ȡ�������
#define ROBOT_ARM_SET_TERID                         0X06    //���û������
#define ROBOT_ARM_JUST_ACTION                       0X07    //��е��ֻ����������������豸

//�������
#define GET_VERSION                                 0x21    //��ȡ����汾��
#define SET_FILE_INFO                               0x22    //�����ļ���Ϣ
#define DOWNLOAD                                    0x23    //����������
#define GET_MASK                                    0x24    //��ȡ��������֤��

#define ROBOT_ARM_NUM 								(DF_TY_USB_PORT_N_MAX)
#define ROBOT_ARM_BUFF_LEN 							1024

#define ROBOT_ARM_UPDATE_FILE_PATH   				"/etc/robot_arm.bin"
#define ROBOT_ARM_UPDATE_FLAG_FILE_PATH   			"/etc/robot_arm_update_falg_file"
#define ROBOT_ARM_UPDATE_VERSION_FILE_PATH   		"/etc/robot_arm_update_versionfile"
#define PACK_SIZE                                   256         //ÿ���Ĵ�С

#define UPDATE_FAILED_MAX_NUM 						5			//���ʧ�ܴ���

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
    uint16 file_size;//�ļ���С
    uint8 file_sum;  //�ļ���У��
    uint8 total_pack_num;  //�ܵİ�����ÿ����С256�ֽ�
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
    FILE_INFO file_info;     //�����ļ���Ϣ
	char old_version[32];	 //�ɰ汾��
	char new_version[32];	 //�°汾��
	char *file_data;		 //�����ļ�����
	UPDATE_STATE update_state; //����״̬
	int successed_pack_num;	 //�ɹ��İ���
	int update_percent;		 //��������
	int port;				 //�����Ķ˿ں�
	int update_failed_count; //����ʱʧ�ܵĴ���
	int update_fd;			 //��������fd
	char *version_ptr;
	int *report_flag;		//�ϱ���־
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
	int lock;	//�˿�usb������
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
	MODULE *module; //ģ�����ָ��
	int 	 update_lock;	//����������
	UPDATE_STATE 	updating;		//��������

}ROBOT_ARM_CALSS;

int robot_arm_model_init(MODULE *module,int max_port_num);
ROBOT_ARM_CALSS *get_robot_arm_class(void);


int robot_arm_open_hid(HUSB_LOCAL hUSB, int nBusNum, int nDevNum);
libusb_device *robot_arm_find_device_ByID(libusb_device **devs, int nBusNum, int nDevNum);
int robot_arm_pack(uint8 order, uint8 end_flag,uint8 *in_buf, int in_len, uint8 *out_buf);

#endif
