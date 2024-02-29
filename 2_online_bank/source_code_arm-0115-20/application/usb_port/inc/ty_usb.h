#ifndef _TY_USB_H
#define _TY_USB_H

#include "opt.h"

#include "switch_dev.h"
#include "usb_file.h"
#include "tax_interface.h"
	
#define DF_TY_USB_CM_GET_BUSID              100                         //��ȡ��busid
#define DF_TY_USB_CM_SET_BUSID              (DF_TY_USB_CM_GET_BUSID+1)  //������busid
#define DF_TY_USB_CM_SET_PORT_N             (DF_TY_USB_CM_SET_BUSID+1)  //��������˳��
#define DF_TY_USB_CM_GET_PORT_N             (DF_TY_USB_CM_SET_PORT_N+1) //��ȡ����˳��
#define DF_TY_USB_CM_SET_ENUM_TIMER         (DF_TY_USB_CM_GET_PORT_N+1) //���ö��ʱ��
#define DF_TY_USB_CM_GET_ENUM_TIMER         (DF_TY_USB_CM_SET_ENUM_TIMER+1) //��ȡ���ö��ʱ��
#define DF_TY_USB_CM_OPEN_PORT              (DF_TY_USB_CM_GET_ENUM_TIMER+1) //�򿪶˿�
#define DF_TY_USB_CM_CLOSE_PORT             (DF_TY_USB_CM_OPEN_PORT+1)      //�رն˿�
#define DF_TY_USB_CM_PORT_BUSID             (DF_TY_USB_CM_CLOSE_PORT+1)      //��ȡ�˿ڵ�busid
#define DF_TY_USB_CM_PORT_NUMBER            (DF_TY_USB_CM_PORT_BUSID+1)     //��ȡ�˿�������
#define DF_TY_USB_CM_JUGE_PORT              (DF_TY_USB_CM_PORT_NUMBER+1)    //�ж϶˿ڵĺϷ���
#define DF_TY_USB_CM_GET_PORT_MA_ID         (DF_TY_USB_CM_JUGE_PORT+1)      //��ȡ�˿�����������
#define DF_TY_USB_CM_GET_PORT_PCB_ID        (DF_TY_USB_CM_GET_PORT_MA_ID+1) //��ȡ�˿����ڵ�PCB
#define DF_TY_USB_CM_GET_BUSID_PORT         (DF_TY_USB_CM_GET_PORT_PCB_ID+1)    //ͨ��busid��ȡport
#define DF_TY_USB_CM_GET_REGISTER_WHETHER	(DF_TY_USB_CM_GET_BUSID_PORT+1)
#define DF_TY_USB_CM_OPEN_PORT_POWER		(DF_TY_USB_CM_GET_REGISTER_WHETHER+1)//�����򿪶˿ڵ�Դ
#define DF_TY_USB_CM_GET_PORT_FILE_EXIST	(DF_TY_USB_CM_OPEN_PORT_POWER+1)//���usb�ļ��Ƿ����
#define DF_TY_USB_CM_GET_TRYOUT_WHETHER		(DF_TY_USB_CM_GET_PORT_FILE_EXIST+1)//��ȡ��������
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

#define DF_TY_BUS_DEV_NUMBER        127                 //BUS���ص�������
#define DF_TY_USB_HUB_OPEN			1					//�ϵ��,HUBһֱ����
#define DF_TY_USB_HUB_CLOSE			0					//�ϵ��,ֻ�д򿪶˿�HUB�ſ�

#define ty_usb_out(s,arg...)       				//log_out("all.c",s,##arg)    

struct _ty_usb_fd
{
	int state;
	struct ty_file_mem	*mem;
	void *mem_dev;
	struct ty_file_fd	*mem_fd;
	struct _ty_usb  *dev;
};

struct _ty_usb_pcb                                  //����PCB
{
	uint8 id[6];                                    //����id
	uint16 n;                                       //USB����
	int state;                                      //��PCB״̬
	uint8 hub_state[8];                             //HUB��״̬
};

struct _ty_usb_class
{
	uint8 id[6];                                    //�����id
	struct _ty_usb_pcb      pcb[8];                 //������������
	uint16 n;                                       //��������
	int usb_port_n;                                 //USB�˿�����
	uint8 soft[8];                                  //����汾
	uint8 hard[8];                                  //Ӳ���汾
	int port_state[DF_TY_USB_PORT_N_MAX];               //�˿�״̬
	int port_lock[DF_TY_USB_PORT_N_MAX];                //�˿���
	char busid[DF_TY_USB_BUSID];                    //��оƬbusid
};

struct _ty_usb_dev
{
	struct _ty_usb_class    dev;                    //�豸 
	struct _switch_dev_id   self_id;                //�Լ������ID
	struct _stm32_infor     stm32_infor;            //STM32 INFOR
};

struct _ty_usb
{
	struct _ty_usb_dev     data;                    //USB����
	int lock;                                       //������
	int type;										//��������
	int state;                                      //״̬
	struct _ty_usb_fd   fd[DF_FILE_OPEN_NUMBER];    //�������״̬
	int enum_timer;                                 //ö�����ʱ��
	int port_n;                                     //�˿�����˳��
	char switch_name[DF_FILE_NAME_MAX_LEN];         //������
	int switch_fd;                                  //����fd
	int hub;										//HUB�Ƿ�һֱ����
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
