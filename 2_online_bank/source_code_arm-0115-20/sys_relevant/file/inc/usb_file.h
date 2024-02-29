#ifndef _USB_FILE_H
#define _USB_FILE_H

#include "opt.h"

#include "linux_s.h"

#define DF_USB_FILE_DRIVER_SIZE             50      //�������ƴ�С
#define DF_USB_FILE_INTERFACE_SIZE          5       //һ���豸��֧�ֵĽӿ�
#define DF_USB_FILE_PORT_SIZE               10      //�˵������

#define DF_USB_FILE_SYS_DIR                 "/sys/bus/usb/devices"

#define usbfile_out(s,arg...)               //log_out("all.c",s,##arg) 

#define e_usb_file
#define DF_USB_FILE_EP_00                   "ep_00"    

//�˵�����
struct _usb_file_port
{
    uint8 bEndpointAddress;          //�˵�
    uint8 bmAttributes;              //λͼ
    uint16 wMaxPacketSize;           //���ݰ��Ĵ�С
    uint8 bInterval;                 //ʱ���϶
};

//�ӿ�������
struct _usb_file_interface
{
    uint8 bInterfaceNumber;           //�ӿں�
    uint8 bAlternateSetting;          //��ѡ��������
    uint8 bNumEndpoints;              //���õĶ˵�����
    uint8 bInterfaceClass;            //�ӿ�������ֵ
    uint8 bInterfaceSubClass;         //������
    uint8 bInterfaceProtocol;         //Э��
    char driver[DF_USB_FILE_DRIVER_SIZE];   //������С
    struct _usb_file_port ep[DF_USB_FILE_PORT_SIZE]; //�˵�
};

//�豸������
struct _usb_file_dev
{
    uint8 bDeviceClass;                //�豸��
    uint8 bDeviceSubClass;             //�豸����
    uint8 bDeviceProtocol;             //Э��
    uint8 bMaxPacketSize0;             //�˵�0��������С
    uint8 bNumInterfaces;              //��֧�ֵĽӿ���
    uint8 bNumConfigurations;          //�������õ���������Ŀ
    uint8 bConfigurationValue;         //�ӿ���
    uint8 bmAttributes;                //��Դ����
    uint8 MaxPower;                    //���ߵ�Դ����
    char speed[DF_USB_FILE_DRIVER_SIZE];//�����ٶ�
    int busnum;                        //���ߺ�
    int dev;                           //�豸��
    int devnum;                        //���豸��
    uint16 idVendor;                   //���̱�־
    uint16 idProduct;                  //��Ʒ��־
    uint16 bcdDevice;                  //�豸���к�
    char driver[DF_USB_FILE_DRIVER_SIZE];//������С
    struct _usb_file_interface  interface[DF_USB_FILE_INTERFACE_SIZE];//�ӿ�����
    struct _usb_file_port   ep;        //�˿�����
};

int _usb_file_get_infor(const char *busid,struct _usb_file_dev    *dev);
int _usb_file_get_bus_dev_n(const char *bus);
int _usb_file_exist(const char *busid);
int get_busid_busnum(char *busid);
int get_busid_devnum(char *busid);

#endif
