#ifndef _LOAD_DEFINE
    #define _LOAD_DEFINE
	#include "../linux_s/_task.h"
	#include "../file/_file.h"
	#include "../core/_port_core.h"
	#include "../bsp/_bsp_note.h"
	#include "../file/_file_ac.h"
	#include "../bsp/_bsp_note.h"
	//#include "../class/_ty_core.h"
	#include "../file/_file.h"
	#include "_app.h"
	void load(void);
    #ifdef _load_c
        #define out(s,arg...)              
    #endif
    /*=================================常数定义===============================================*/
    #ifdef _load_c

		static struct _linux_s		linux_s;
        //static const struct _ty_usb_arg     ty_usb_par=
        //{   .seria="/dev/seria0",
        //    .bps=115200
        //};

		static  _err	port_core_err[] =
		{ { .err = DF_ERR_PORT_CORE_NO_DEV,
		.name = "没有该设备"
		},
		{ .err = DF_ERR_PORT_CORE_NO_SPACE,
		.name = "设备已经没有空间注册"
		},
		{ .err = DF_ERR_PORT_CORE_SLAVE_SPACE,
		.name = "子设备没有空间"
		},
		{ .err = DF_ERR_PORT_CORE_MAIN_DEV,
		.name = "主设备错误"
		},
		{ .err = DF_ERR_PORT_CORE_SLAVE_DEV,
		.name = "从设备错误"
		},
		{ .err = DF_ERR_PORT_CORE_CM,
		.name = "不支持该参数"
		},
		{ .err = DF_ERR_PORT_CORE_OPEN_NULL,
		.name = "open函数不能为空"
		},
		{ .err = DF_ERR_PORT_CORE_CM_BPS,
		.name = "不支持波特率设置"
		},
		{ .err = DF_ERR_PORT_CORE_CM_MAIN,
		.name = "从设备无此功能"
		},
		{ .err = DF_ERR_PORT_CORE_BSP,
		.name = "BSP包不支持该功能"
		},
		{ .err = DF_ERR_PORT_CORE_FD_NULL,
		.name = "私有数据为空"
		},
		{ .err = DF_ERR_PORT_CORE_FD_Z,
		.name = "子类fd超出范围"
		},
		{ .err = DF_ERR_PORT_CORE_PRINT_LINE,
		.name = "不支持[DF_PRINTER_CM_LINE]命令"
		},
		{ .err = DF_ERR_PORT_CORE_FEED_PAPER,
		.name = "不支持[DF_PRINTER_CM_FEED]命令"
		},
		{ .err = DF_ERR_PORT_CORE_BACK,
		.name = "不支持[DF_PRINTER_CM_BACK]命令"
		},
		{ .err = DF_ERR_PORT_CORE_BLACK,
		.name = "不支持[DF_PRINTER_CM_BLACK]命令"
		},
		{ .err = DF_ERR_PORT_CORE_STATE,
		.name = "不支持[DF_PRINTER_CM_STATE]命令"
		},
		{ .err = DF_ERR_PORT_CORE_PAR,
		.name = "不支持[DF_PRINTER_CM_PAR]命令"
		},
		{ .err = DF_ERR_PORT_CORE_FONT,
		.name = "不支持[DF_PRINTER_CM_FONT]命令"
		},
		{ .err = DF_ERR_PORT_CORE_SET_LEN,
		.name = "不支持[DF_PRINTER_CM_PAPER_LEN]命令"
		},
		{ .err = DF_ERR_PORT_CORE_BID,
		.name = "不支持[DF_PRINTER_CM_BID]命令"
		},
		{ .err = DF_ERR_PORT_BID_SET,
		.name = "不支持[DF_PRINTER_CM_BID_SET]命令"
		},
		{ .err = DF_ERR_PORT_TEST,
		.name = "不支持[DF_PRINTER_CM_TEST]命令"
		},
		{ .err = DF_ERR_PORT_CORE_NO_MEM,
		.name = "不支持远程访问"
		},
		{ .err = DF_ERR_PORT_CORE_SAM_NOPOWER,
		.name = "psam没有上电"
		},
		{ .err = DF_ERR_PORT_CORE_SAM_POWERED,
		.name = "psam已上电"
		},
		{ .err = DF_ERR_PORT_CORE_TY_USB_INIT,
		.name = "stm32初始化中"
		},
		{ .err = DF_ERR_PORT_CORE_TY_USB_PORT_OVER,
		.name = "超出USB物理端口数量"
		},
		{ .err = DF_ERR_PORT_CORE_TY_USB_PCB_NO,
		.name = "该块PCB初始化时,出问题"
		},
		{ .err = DF_ERR_PORT_CORE_TY_USB_STM32_OPEN,
		.name = "stm32打开端口失败"
		},
		{ .err = DF_ERR_PORT_CORE_TY_USB_BUS_NUMBER,
		.name = "usb bus上设备超限"
		},
		{ .err = DF_ERR_PORT_CORE_TY_USB_NO_FIND_DEV,
		.name = "USB端口已经打开,没有发现该设备"
		},
		{ .err = DF_ERR_PORT_CORE_TY_USB_NO_PERMISSION,
		.name = "无操作权限"
		},
		{ .err = DF_ERR_PORT_CORE_TY_USB_STM32_ANSWER,
		.name = "stm32在规定时间内没有应答"
		},
		{ .err = DF_ERR_PORT_CORE_TY_USB_STM32_CLOSE,
		.name = "stm32应答关闭端口失败"
		},
		{ .err = DF_ERR_PORT_CORE_TY_USB_STM32_PCB,
		.name = "stm32未找到PCB"
		},
		{ .err = DF_ERR_PORT_CORE_TY_USB_STM32_DEV,
		.name = "stm32未找到DEV"
		},
		{ .err = DF_ERR_PORT_CORE_TY_USB_TREE_READ,
		.name = "读usb树失败"
		},
		{ .err = DF_ERR_PORT_CORE_TY_USB_TREE_PCB,
		.name = "没有找到该板子"
		},
		{ .err = DF_ERR_PORT_CORE_TY_USB_SET_START_ZERO,
		.name = "设置USB起始号不能为0"
		},
		{ .err = DF_ERR_PORT_CORE_TY_USB_PORT_MANAGE_PORT_USEED,
		.name = "该端口已经给使用了"
		},
		{ .err = DF_ERR_PORT_CORE_TY_USB_PORT_MANAGE_NO_CLOSE,
		.name = "无权关闭该端口"
		},
		{ .err = DF_ERR_PORT_CORE_USB_IP_ROOT,
		.name = "服务器非root用户登录"
		},
		{ .err = DF_ERR_PORT_CORE_USB_IP_KO,
		.name = "usbip没有找到驱动"
		},
		{ .err = DF_ERR_PORT_CORE_REGIT_ERR,
		.name = "注册通云协议栈失败"
		},
		{ .err = DF_ERR_PORT_CORE_TY_USB_PORT_USED,
		.name = "该端口在使用"
		},
		{ .err = DF_ERR_PORT_CORE_TY_USB_PORT_MANAGE_CHECK_USB_BUSY,
		.name = "port_manage检测端口中"
		},
		{ .err = DF_ERR_PORT_CORE_TY_USB_PORT_MANAGE_CHECK_PAR,
		.name = "参数检测中"
		},
		{ .err = DF_ERR_PORT_CORE_TY_USB_OPER_ERR,
		.name = "线路在运行过程中发生故障,USB HUB打开失败,系统要重启"
		},
		{ .err = DF_ERR_PORT_CORE_TY_USB_PORT_MANAGE_LINUX_FILE,
		.name = "操作系统USB文件系统发生致命错误,服务器要求重新启动"
		},
		{ .err = DF_ERR_PORT_CORE_SOCKET_SEV_BIND,
		.name = "socket绑定端口失败"
		},
		{ .err = DF_ERR_PORT_CORE_SOCKET_LISTER,
		.name = "socket监听端口失败"
		},
		{ .err = DF_ERR_PORT_CORE_SWITCH_SERIA,
		.name = "seria操作错误"
		},
		{ .err = DF_ERR_PORT_CORE_TY_USB_MAIN_BUSID,
		.name = "没有可用的主busid"
		},
		{ .err = DF_ERR_PORT_CORE_TY_USB_MAIN_HUB,
		.name = "主hub丢失,要求重新启动"
		},
		{ .err = DF_ERR_PORT_CORE_TY_USB_PCB_HUB,
		.name = "单板hub丢失"
		},
		{ .err = DF_ERR_PORT_CORE_TY_USB_PORT_NO_ENUM,
		.name = "没有找到对应的设备枚举"
		},
		{ .err = DF_ERR_PORT_CORE_TY_USB_CLOSE,
		.name = "设备已经关闭,但文件系统上有设备信息,系统重新启动"
		},
		{ .err = DF_ERR_PORT_CORE_USB_IP_BUSID_RE,
		.name = "该busid已经存在"
		},
		{ .err = DF_ERR_PORT_CORE_USB_IP_WRITE,
		.name = "写USBshare协议栈失败(bind error)"
		},
		{ .err = DF_ERR_PORT_CORE_FTP_DOWNLOAD_ERR,
		.name = "下载更新失败"
		},
		{ .err = DF_ERR_PORT_CORE_FTP_DOWNLOAD_PROGRAM_IS_NEW,
		.name = "软件已经是最新版本"
		},
		{ .err = DF_ERR_PORT_CORE_PORT_ISNOT_SHUIPAN,
		.name = "该端口非支持的税盘"
		},
		{ .err = DF_ERR_PORT_CORE_GET_INVOICE_ERR,
		.name = "获取税盘发票信息错误"
		},
		{ .err = DF_ERR_PORT_CORE_GET_FP_MONITOR_DATA,
		.name = "不支持此类发票查询"
		},
		{ .err = DF_ERR_PORT_CORE_GET_FP_PARAMETERS,
		.name = "获取税盘发票参数错误"
		},
		{ .err = DF_ERR_PORT_CORE_GET_FP_OPEER_USB,
		.name = "获取税盘信息操作USB失败"
		},
		{ .err = DF_ERR_PORT_CORE_GET_FP_INVIOCE_NUM,
		.name = "获取税盘发票数量错误"
		},
		{ .err = DF_ERR_PORT_CORE_GET_FP_INVOICE_LEN,
		.name = "获取发票数据大小错误"
		}


		};
		static const _err_buf	port_core_err_buf =
		{ port_core_err,
		sizeof(port_core_err) / sizeof(_err),
		0
		};
    #endif




#endif