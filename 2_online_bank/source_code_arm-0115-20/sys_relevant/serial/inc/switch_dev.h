#ifndef _SWITCH_DEV_H
#define _SWITCH_DEV_H

#include "opt.h"

#include "seria.h"
//#include "mqtt_client.h"
#include "linux_ip.h"

#define DF_SWITCH_CM_OPEN               100                         //�򿪶˿�
#define DF_SWITCH_CM_CLOSE              (DF_SWITCH_CM_OPEN+1)       //�رն˿�
#define DF_SWITCH_CM_SET_ID             (DF_SWITCH_CM_CLOSE+1)      //����ID
#define DF_SWITCH_CM_GET_ID             (DF_SWITCH_CM_SET_ID+1)     //��ȡID
#define DF_SWITCH_CM_GET_CPUID          (DF_SWITCH_CM_GET_ID+1)     //��ȡCPUID
#define DF_SWITCH_CM_SET_PCB_ID         (DF_SWITCH_CM_GET_CPUID+1)     //���õ���ID
#define DF_SWITCH_CM_GET_PCB_ID         (DF_SWITCH_CM_SET_PCB_ID+1) //��ȡ����ID
#define DF_SWITCH_CM_GET_CLASS          (DF_SWITCH_CM_GET_PCB_ID+1) //��ȡ����
#define DF_SWITCH_CM_GET_PCB            (DF_SWITCH_CM_GET_CLASS+1)  //��ȡ����
#define DF_SWITCH_CM_PORT_STATE         (DF_SWITCH_CM_GET_PCB+1)    //�˿�״̬      
#define DF_SWITCH_CM_RESET_MA           (DF_SWITCH_CM_PORT_STATE+1) //��������
#define DF_SWITCH_CM_POWER_CLOSE        (DF_SWITCH_CM_RESET_MA+1)   //�رյ�Դ    
#define DF_SWITCH_CM_WRITE_FLASH        (DF_SWITCH_CM_POWER_CLOSE+1) //дflash
#define DF_SWITCH_CM_READ_FLASH         (DF_SWITCH_CM_WRITE_FLASH+1) //��flash  
#define DF_SWITCH_CM_GET_INFOR          (DF_SWITCH_CM_READ_FLASH+1)  //��ȡ�豸��Ϣ  
#define DF_SWITCH_CM_GET_DEV_SOFT_EDIT  (DF_SWITCH_CM_GET_INFOR+1)      //��ȡ����汾   
#define DF_SWITCH_CM_GET_HEART_TIMER    (DF_SWITCH_CM_GET_DEV_SOFT_EDIT+1)  //��ѯ����ʱ��  
#define DF_SWITCH_CM_HEART              (DF_SWITCH_CM_GET_HEART_TIMER+1)    //����   
#define DF_SWITCH_CM_ADD_RECODER        (DF_SWITCH_CM_HEART+1)          //���Ӽ�¼
#define DF_SWITCH_CM_READ_RECODER       (DF_SWITCH_CM_ADD_RECODER+1)    //����¼
#define DF_SWITCH_CM_OPEN_HUB			(DF_SWITCH_CM_READ_RECODER+1)	//��HUB����Ƭ��һֱ��HUB����
#define DF_SWITCH_CM_GET_NET_OK			(DF_SWITCH_CM_OPEN_HUB+1)		//��ȡ������,ָ֪ͨʾ
#define DF_SWITCH_CM_NO_NET				(DF_SWITCH_CM_GET_NET_OK+1)		//���綪ʧ,֪ͨ
#define DF_SWITCH_CM_GET_SERIAL_NUM     (DF_SWITCH_CM_NO_NET+1)			//��ȡ��Ƭ��Ψһ��ʶ�� 
#define DF_SWITCH_CM_PORT_NO_DEV		(DF_SWITCH_CM_GET_SERIAL_NUM+1)			//ָʾ�˿�û���豸
#define DF_SWITCH_CM_PORT_DEV_LOCAL     (DF_SWITCH_CM_PORT_NO_DEV+1)			//ָʾ�˿��豸�ڱ���
#define DF_SWITCH_CM_PORT_DEV_SHARE     (DF_SWITCH_CM_PORT_DEV_LOCAL+1)			//ָʾ�˿��豸��Զ�̹���
#define DF_SWITCH_CM_CONN				DF_SWITCH_CM_PORT_DEV_SHARE+1
#define DF_SWITCH_CM_PORT_RED_OFF_GREN_OFF	(DF_SWITCH_CM_CONN+1)
#define DF_SWITCH_CM_PORT_RED_ON_GREN_OFF	(DF_SWITCH_CM_PORT_RED_OFF_GREN_OFF+1)
#define DF_SWITCH_CM_PORT_RED_OFF_GREN_ON	(DF_SWITCH_CM_PORT_RED_ON_GREN_OFF+1)
#define DF_SWITCH_CM_PORT_RED_ON_GREN_TWI	(DF_SWITCH_CM_PORT_RED_OFF_GREN_ON+1)
#define DF_SWITCH_CM_PORT_RED_ON_GREN_ON	(DF_SWITCH_CM_PORT_RED_ON_GREN_TWI+1)
#define DF_SWITCH_CM_PORT_RED_TWI_GREN_ON	(DF_SWITCH_CM_PORT_RED_ON_GREN_ON+1)
#define DF_SWITCH_CM_PORT_RED_TWI_GREN_TWI_1S		(DF_SWITCH_CM_PORT_RED_TWI_GREN_ON+1)
#define DF_SWITCH_CM_LCD_SCREEN_PRINT		(DF_SWITCH_CM_PORT_RED_TWI_GREN_TWI_1S+1)
#define DF_SWITCH_CM_GET_KEY_VALUE			(DF_SWITCH_CM_LCD_SCREEN_PRINT+1)
#define DF_SWITCH_CM_PORT_RED_TWI_GREN_OFF	(DF_SWITCH_CM_GET_KEY_VALUE+1)
#define DF_SWITCH_CM_PORT_RED_OFF_GREN_TWI	(DF_SWITCH_CM_PORT_RED_TWI_GREN_OFF+1)
#define DF_SWITCH_CM_MAC_RESTART_POWER		(DF_SWITCH_CM_PORT_RED_OFF_GREN_TWI+1)
#define DF_SWITCH_CM_GET_ALL_PORT_STATUS	(DF_SWITCH_CM_MAC_RESTART_POWER+1)
#define DF_SWITCH_CM_CONTRL_FAN			(DF_SWITCH_CM_GET_ALL_PORT_STATUS+1)
#define DF_SWITCH_CM_PORT_DOUBLE_LIGHT_YELLOW			(DF_SWITCH_CM_CONTRL_FAN+1)
#define DF_SWITCH_CM_GET_AD_STATUS			(DF_SWITCH_CM_PORT_DOUBLE_LIGHT_YELLOW+1)
#define DF_SWITCH_CM_RESTORE				(DF_SWITCH_CM_GET_AD_STATUS+1)
#define DF_SWITCH_CM_BLUETOOTH_MODE			(DF_SWITCH_CM_RESTORE+1)
#define DF_SWITCH_CM_BLUETOOTH_POWER				(DF_SWITCH_CM_BLUETOOTH_MODE+1)
#define DF_SWITCH_CM_GET_SUPPORT			DF_SWITCH_CM_BLUETOOTH_POWER+1

#define DF_SWITCH_CM_CLASS_ID           100                         //ֻ��ȡID
#define DF_SWITCH_CM_CLASS_ALL          (DF_SWITCH_CM_CLASS_ID+1)   //��ȡ���е�class
        
#define DF_PC_ORDER_HEAD_LEN		 7                  //STM32����ͷ����
/*+++++++++++++++++++++++++++++++�����ֶ���++++++++++++++++++++++++++++++++++++++*/
#define DF_PC_ORDER_CONNECT			0X01				//��������
#define DF_PC_ORDER_DISCONNECT		0X02				//�ͷſ���Ȩ����
#define DF_PC_ORDER_SET_DEV_ID		0X07				//�����豸ID
#define DF_PC_ORDER_READ_DEV_ID		0X08				//��ȡ�豸ID����������
#define DF_PC_ORDER_SET_SUB_ID		0X11				//���õ���ID
#define DF_PC_ORDER_READ_SUB_ID		0X12				//��ȡ����ID
#define DF_PC_ORDER_GROUNP_PRO		0X13				//��ȡ����Ⱥ����
#define DF_PC_ORDER_SUB_PRO			0X14				//��ȡ��������
#define DF_PC_ORDER_USB_OPEN		0X15				//��ָ���˿�
#define DF_PC_ORDER_USB_CLOSE		0X16				//�ر�ָ���˿�
#define DF_PC_ORDER_USB_OPEN_LIST	0X17				//��ȡ�Ѵ�USB�б�	
#define DF_PC_ORDER_USB_GET_SOFT	0X18				//��ȡ����汾
#define DF_PC_ORDER_USB_WRITE_SOFT	0X19				//д���
#define DF_PC_ORDER_USB_GET_OPEN_MA	0X20				//���������������ʾ�Ѿ��򿪵�USB�˿�
#define DF_PC_ORDER_USB_GET_MA_OPEN	0X21				//ָ��������Ż�ȡ�Ѿ��򿪵�USB�˿�
#define DF_PC_ORDER_USB_MA_OPEN		0X22				//ָ��������Ŵ�USB�˿�
#define DF_PC_ORDER_USB_MA_CLOSE	0X23				//ָ��������Źر�USB�˿�
#define DF_PC_ORDER_POWER_OFF		0X24				//�رջ���
#define DF_PC_ORDER_USB_HUB_N		0X25				//USB HUB����
#define DF_PC_ORDER_OPEN_HUB        0X26                //������HUB
#define DF_PC_ORDER_RESET           0X27                //��������
#define DF_PC_ORDER_HREAT           0X28                //��������
#define DF_PC_ORDER_HREAT_TIMER     0X29                //��ѯ����ʱ��
#define DF_PC_ORDER_WRITE_FLASH     0X2a                //дflash
#define DF_PC_ORDER_READ_FLASH      0X2B                //��flash
#define DF_PC_ORDER_GET_EDIT_INFOR  0X2c                //��ȡ�汾��Ϣ
#define DF_PC_ORDER_WRITE_RECODER   0X2D                //���Ӽ�¼�ļ�
#define DF_PC_ORDER_READ_RECODER    0X2E                //����¼�ļ�  
#define DF_PC_ORDER_GET_NET_OK		0X2F				//��ȡ������֪ͨ��λ��
#define DF_PC_ORDER_NO_NET			0X30				//���綪ʧ֪ͨ��λ��
#define DF_PC_ORDER_GET_SERIAL_NUM	0X31				//��ȡ��Ƭ��Ψһ���к�
#define DF_PC_ORDER_CTL_PORT_LED	0X32				//���ƶ˿ڵ�״ָ̬ʾ
#define DF_PC_ORDER_CTL_MAC_POWER	0X33				//���ƿ���MAC��Դ

#define DF_PC_ORDER_CTL_PORT_LED_2	0X40				//���ƶ˿ڵ�״ָ̬ʾ(��������)

#define DF_PC_ORDER_CTL_LCD_SCREEN	0X41				//����LCD��ʾ����ʾ

#define DF_PC_ORDER_CTL_WRITE_KEY	0X45				//д����Կ�ļ�
#define DF_PC_ORDER_CTL_READ_KEY	0X46				//��ȡ��Կ�ļ�
#define DF_PC_ORDER_CTL_GET_ALL_PORT_STATUS	0X47		//��ȡ���µ�˿��豸״̬
#define DF_PC_ORDER_CTL_FAN					0X49		//���Ʒ��ȴ򿪹ر�
#define DF_PC_ORDER_CTL_BLUETOOTH	0x51				//��������ģʽ AT  ͸��
#define	DF_PC_ORDER_BLUETOOTH_POWER	0X52				//����������Դ
#define DF_PC_ORDER_RESTORE						0X53	//�س���
#define DF_PC_ORDER_SET_PORT_DOUBLE_LIGHT			0X54
#define DF_PC_ORDER_GET_AD_STATUS				0x55

#define	DF_WIFI_LED_SAME_USBPORT	10001
   
#define switch_dev_out(s,arg...)       		//log_out("all.c",s,##arg) 
		
#define DF_SERIA_DIR                    "/dev/seria0"   
#define DF_SERIA_BPS                    115200    
#define DF_TY_STM32_OVER_TIMER          2000        //STM32ͨѶ��ʱʱ��   

struct _switch_event
{
	int event;
	int reason;
	uint8 clock[8];
};

struct _switch_event_all
{
	int n;
	struct _switch_event event[30];
};
    
struct _switch_dev_pcb_id
{
	uint8 id[6];
	uint8 date[4];
	int n;          //���忪�ؿ�����
};

struct _switch_dev_id
{
	uint8 id[6];
	uint8 date[4];
};

struct _switch_dev_only
{
	uint8 serial_num[12];
};

struct _switch_dev_class        //����Ⱥ����
{
	uint8 id[6];
	int n;
};

struct _switch_dev_pcb      //��������
{
	uint8 id[6];
	int n;
};

struct _stm32_infor
{
	char r_date[11];        //��������
	char soft[9];           //����汾
	char hard[9];           //Ӳ���汾
};

struct _dev_support
{
	int support_fan;//����
	int support_ble;//�����ͻƵ�
	int support_power;//�ڵ�
	int support_lcd;//��ʾ����LED�°�֧��
};

struct _switch_dev_fd
{
	int state;
	struct ty_file_mem *mem;
	void *mem_dev;
	struct ty_file_fd *mem_fd;
	struct _switch_dev *dev;
};

struct _switch_dev
{
	struct _switch_dev_fd fd[DF_FILE_OPEN_NUMBER];
	int state;
	int lock;
	int seria;
	int hardtype;
	struct _dev_support dev_support;
	char ter_type[50];
	char ter_id[20];
	char cpu_id[50];
};

struct _switch_ctl_fu
{
	int cm;
	int (*ctl)(struct _switch_dev_fd *id,va_list args);
};

int switch_dev_add(int hard_type, char *ter_type);
    
#endif
