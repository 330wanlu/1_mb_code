#ifndef _USB_FILE_DEFINE
    #define _USB_FILE_DEFINE
    #include "_linux_s.h"
    //#include "_file_pro.h"
#ifndef RELEASE_SO
    #include "_folder.h"
	#include "../linux_s/_t_file_c.h"
#endif
	//#include "../software/_code_s.h"
	//#include <io.h>
    /*==================================����������=============================================*/
    #define DF_USB_FILE_DRIVER_SIZE                             50      //�������ƴ�С
    #define DF_USB_FILE_INTERFACE_SIZE                          5       //һ���豸��֧�ֵĽӿ�
    #define DF_USB_FILE_PORT_SIZE                               10      //�˵������
    #define DF_USB_FILE_SYS_DIR                             "/sys/bus/usb/devices"
    #ifdef _usb_file_c
		 #ifdef DEBUG
			#define out(s,arg...)       				//log_out("all.c",s,##arg) 
		#else
			#define out(s,arg...)
		#endif
        #define e_usb_file
        #define DF_USB_FILE_EP_00                               "ep_00"    
    #else
        #define e_usb_file
    #endif
    /*==================================�ṹ�嶨��=============================================*/
    //�˵�����
    struct _usb_file_port
    {   uint8 bEndpointAddress;                                                       //�˵�
        uint8 bmAttributes;                                                           //λͼ
        uint16 wMaxPacketSize;                                                         //���ݰ��Ĵ�С
        uint8 bInterval;                                                              //ʱ���϶ 
    };
    //�ӿ�������
    struct _usb_file_interface
    {   uint8 bInterfaceNumber;                                                       //�ӿں�
        uint8 bAlternateSetting;                                                      //��ѡ��������
        uint8 bNumEndpoints;                                                          //���õĶ˵�����
        uint8 bInterfaceClass;                                                        //�ӿ�������ֵ
        uint8 bInterfaceSubClass;                                                     //������
        uint8 bInterfaceProtocol;                                                     //Э��
          
        char driver[DF_USB_FILE_DRIVER_SIZE];                                       //������С
        struct _usb_file_port   ep[DF_USB_FILE_PORT_SIZE];                          //�˵�
    };
    

    //�豸������
    struct _usb_file_dev
    {   uint8 bDeviceClass;                                                             //�豸��
        uint8 bDeviceSubClass;                                                          //�豸����
        uint8 bDeviceProtocol;                                                          //Э��
        uint8 bMaxPacketSize0;                                                          //�˵�0��������С
        uint8 bNumInterfaces;                                                           //��֧�ֵĽӿ���
        uint8 bNumConfigurations;                                                       //�������õ���������Ŀ
        uint8 bConfigurationValue;                                                      //�ӿ���
        uint8 bmAttributes;                                                             //��Դ����
        uint8 MaxPower;                                                                 //���ߵ�Դ����
        char speed[50];                                                                 //�����ٶ�
        int busnum;                                                                     //���ߺ�
        int dev;                                                                        //�豸��
        int devnum;                                                                     //���豸��  
        uint16 idVendor;                                                                //���̱�־
        uint16 idProduct;                                                               //��Ʒ��־
        uint16 bcdDevice;                                                               //�豸���к�
        char driver[DF_USB_FILE_DRIVER_SIZE];                                           //������С
        struct _usb_file_interface  interface[DF_USB_FILE_INTERFACE_SIZE];              //�ӿ�����
        struct _usb_file_port   ep;                                                     //�˿�����
        
    };
    
    /*======================================��������==============================================*/
    e_usb_file  int _usb_file_ls_busid(const char *busid);
    e_usb_file  int _usb_file_get_infor(const char *busid,struct _usb_file_dev    *dev);
    e_usb_file  int _usb_file_get_bus_dev_n(const char *bus);
	e_usb_file	int _usb_file_exist(const char *busid);
	e_usb_file  int get_busid_busnum(char *busid);
	e_usb_file  int get_busid_devnum(char *busid);
    #ifdef _usb_file_c
        static int read_interface(const char *path,struct _usb_file_interface   *interface);
        static int read_ep(const char *path,struct _usb_file_port   *ep);
        static int read_dev(const char *path,struct _usb_file_dev    *dev);
        static int read_int(const char *path,const char *name);
        static int read_string(const char *path,const char *name,char *r_string);
        static int read_link(const char *path,const char *name,char *link_name);
        static int read_uint8(const char *path,const char *name,uint8 *r_val);
        static int read_uint16(const char *path,const char *name,uint16 *r_val);
    
    #endif



#endif
