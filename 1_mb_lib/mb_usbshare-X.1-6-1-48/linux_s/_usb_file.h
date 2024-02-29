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
    /*==================================立即数定义=============================================*/
    #define DF_USB_FILE_DRIVER_SIZE                             50      //驱动名称大小
    #define DF_USB_FILE_INTERFACE_SIZE                          5       //一个设备所支持的接口
    #define DF_USB_FILE_PORT_SIZE                               10      //端点的数量
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
    /*==================================结构体定义=============================================*/
    //端点描述
    struct _usb_file_port
    {   uint8 bEndpointAddress;                                                       //端点
        uint8 bmAttributes;                                                           //位图
        uint16 wMaxPacketSize;                                                         //数据包的大小
        uint8 bInterval;                                                              //时间间隙 
    };
    //接口描述符
    struct _usb_file_interface
    {   uint8 bInterfaceNumber;                                                       //接口号
        uint8 bAlternateSetting;                                                      //可选设置索引
        uint8 bNumEndpoints;                                                          //所用的端点数量
        uint8 bInterfaceClass;                                                        //接口所属类值
        uint8 bInterfaceSubClass;                                                     //子类码
        uint8 bInterfaceProtocol;                                                     //协议
          
        char driver[DF_USB_FILE_DRIVER_SIZE];                                       //驱动大小
        struct _usb_file_port   ep[DF_USB_FILE_PORT_SIZE];                          //端点
    };
    

    //设备描述符
    struct _usb_file_dev
    {   uint8 bDeviceClass;                                                             //设备类
        uint8 bDeviceSubClass;                                                          //设备子类
        uint8 bDeviceProtocol;                                                          //协议
        uint8 bMaxPacketSize0;                                                          //端点0的最大包大小
        uint8 bNumInterfaces;                                                           //所支持的接口数
        uint8 bNumConfigurations;                                                       //可能配置的描述符数目
        uint8 bConfigurationValue;                                                      //接口数
        uint8 bmAttributes;                                                             //电源属性
        uint8 MaxPower;                                                                 //总线电源消耗
        char speed[50];                                                                 //运行速度
        int busnum;                                                                     //总线号
        int dev;                                                                        //设备号
        int devnum;                                                                     //次设备号  
        uint16 idVendor;                                                                //厂商标志
        uint16 idProduct;                                                               //产品标志
        uint16 bcdDevice;                                                               //设备发行号
        char driver[DF_USB_FILE_DRIVER_SIZE];                                           //驱动大小
        struct _usb_file_interface  interface[DF_USB_FILE_INTERFACE_SIZE];              //接口描述
        struct _usb_file_port   ep;                                                     //端口描述
        
    };
    
    /*======================================函数定义==============================================*/
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
