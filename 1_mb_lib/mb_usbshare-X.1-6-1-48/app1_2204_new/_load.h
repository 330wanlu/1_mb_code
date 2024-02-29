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
    /*=================================��������===============================================*/
    #ifdef _load_c

		static struct _linux_s		linux_s;
        //static const struct _ty_usb_arg     ty_usb_par=
        //{   .seria="/dev/seria0",
        //    .bps=115200
        //};

		static  _err	port_core_err[] =
		{ { .err = DF_ERR_PORT_CORE_NO_DEV,
		.name = "û�и��豸"
		},
		{ .err = DF_ERR_PORT_CORE_NO_SPACE,
		.name = "�豸�Ѿ�û�пռ�ע��"
		},
		{ .err = DF_ERR_PORT_CORE_SLAVE_SPACE,
		.name = "���豸û�пռ�"
		},
		{ .err = DF_ERR_PORT_CORE_MAIN_DEV,
		.name = "���豸����"
		},
		{ .err = DF_ERR_PORT_CORE_SLAVE_DEV,
		.name = "���豸����"
		},
		{ .err = DF_ERR_PORT_CORE_CM,
		.name = "��֧�ָò���"
		},
		{ .err = DF_ERR_PORT_CORE_OPEN_NULL,
		.name = "open��������Ϊ��"
		},
		{ .err = DF_ERR_PORT_CORE_CM_BPS,
		.name = "��֧�ֲ���������"
		},
		{ .err = DF_ERR_PORT_CORE_CM_MAIN,
		.name = "���豸�޴˹���"
		},
		{ .err = DF_ERR_PORT_CORE_BSP,
		.name = "BSP����֧�ָù���"
		},
		{ .err = DF_ERR_PORT_CORE_FD_NULL,
		.name = "˽������Ϊ��"
		},
		{ .err = DF_ERR_PORT_CORE_FD_Z,
		.name = "����fd������Χ"
		},
		{ .err = DF_ERR_PORT_CORE_PRINT_LINE,
		.name = "��֧��[DF_PRINTER_CM_LINE]����"
		},
		{ .err = DF_ERR_PORT_CORE_FEED_PAPER,
		.name = "��֧��[DF_PRINTER_CM_FEED]����"
		},
		{ .err = DF_ERR_PORT_CORE_BACK,
		.name = "��֧��[DF_PRINTER_CM_BACK]����"
		},
		{ .err = DF_ERR_PORT_CORE_BLACK,
		.name = "��֧��[DF_PRINTER_CM_BLACK]����"
		},
		{ .err = DF_ERR_PORT_CORE_STATE,
		.name = "��֧��[DF_PRINTER_CM_STATE]����"
		},
		{ .err = DF_ERR_PORT_CORE_PAR,
		.name = "��֧��[DF_PRINTER_CM_PAR]����"
		},
		{ .err = DF_ERR_PORT_CORE_FONT,
		.name = "��֧��[DF_PRINTER_CM_FONT]����"
		},
		{ .err = DF_ERR_PORT_CORE_SET_LEN,
		.name = "��֧��[DF_PRINTER_CM_PAPER_LEN]����"
		},
		{ .err = DF_ERR_PORT_CORE_BID,
		.name = "��֧��[DF_PRINTER_CM_BID]����"
		},
		{ .err = DF_ERR_PORT_BID_SET,
		.name = "��֧��[DF_PRINTER_CM_BID_SET]����"
		},
		{ .err = DF_ERR_PORT_TEST,
		.name = "��֧��[DF_PRINTER_CM_TEST]����"
		},
		{ .err = DF_ERR_PORT_CORE_NO_MEM,
		.name = "��֧��Զ�̷���"
		},
		{ .err = DF_ERR_PORT_CORE_SAM_NOPOWER,
		.name = "psamû���ϵ�"
		},
		{ .err = DF_ERR_PORT_CORE_SAM_POWERED,
		.name = "psam���ϵ�"
		},
		{ .err = DF_ERR_PORT_CORE_TY_USB_INIT,
		.name = "stm32��ʼ����"
		},
		{ .err = DF_ERR_PORT_CORE_TY_USB_PORT_OVER,
		.name = "����USB����˿�����"
		},
		{ .err = DF_ERR_PORT_CORE_TY_USB_PCB_NO,
		.name = "�ÿ�PCB��ʼ��ʱ,������"
		},
		{ .err = DF_ERR_PORT_CORE_TY_USB_STM32_OPEN,
		.name = "stm32�򿪶˿�ʧ��"
		},
		{ .err = DF_ERR_PORT_CORE_TY_USB_BUS_NUMBER,
		.name = "usb bus���豸����"
		},
		{ .err = DF_ERR_PORT_CORE_TY_USB_NO_FIND_DEV,
		.name = "USB�˿��Ѿ���,û�з��ָ��豸"
		},
		{ .err = DF_ERR_PORT_CORE_TY_USB_NO_PERMISSION,
		.name = "�޲���Ȩ��"
		},
		{ .err = DF_ERR_PORT_CORE_TY_USB_STM32_ANSWER,
		.name = "stm32�ڹ涨ʱ����û��Ӧ��"
		},
		{ .err = DF_ERR_PORT_CORE_TY_USB_STM32_CLOSE,
		.name = "stm32Ӧ��رն˿�ʧ��"
		},
		{ .err = DF_ERR_PORT_CORE_TY_USB_STM32_PCB,
		.name = "stm32δ�ҵ�PCB"
		},
		{ .err = DF_ERR_PORT_CORE_TY_USB_STM32_DEV,
		.name = "stm32δ�ҵ�DEV"
		},
		{ .err = DF_ERR_PORT_CORE_TY_USB_TREE_READ,
		.name = "��usb��ʧ��"
		},
		{ .err = DF_ERR_PORT_CORE_TY_USB_TREE_PCB,
		.name = "û���ҵ��ð���"
		},
		{ .err = DF_ERR_PORT_CORE_TY_USB_SET_START_ZERO,
		.name = "����USB��ʼ�Ų���Ϊ0"
		},
		{ .err = DF_ERR_PORT_CORE_TY_USB_PORT_MANAGE_PORT_USEED,
		.name = "�ö˿��Ѿ���ʹ����"
		},
		{ .err = DF_ERR_PORT_CORE_TY_USB_PORT_MANAGE_NO_CLOSE,
		.name = "��Ȩ�رոö˿�"
		},
		{ .err = DF_ERR_PORT_CORE_USB_IP_ROOT,
		.name = "��������root�û���¼"
		},
		{ .err = DF_ERR_PORT_CORE_USB_IP_KO,
		.name = "usbipû���ҵ�����"
		},
		{ .err = DF_ERR_PORT_CORE_REGIT_ERR,
		.name = "ע��ͨ��Э��ջʧ��"
		},
		{ .err = DF_ERR_PORT_CORE_TY_USB_PORT_USED,
		.name = "�ö˿���ʹ��"
		},
		{ .err = DF_ERR_PORT_CORE_TY_USB_PORT_MANAGE_CHECK_USB_BUSY,
		.name = "port_manage���˿���"
		},
		{ .err = DF_ERR_PORT_CORE_TY_USB_PORT_MANAGE_CHECK_PAR,
		.name = "���������"
		},
		{ .err = DF_ERR_PORT_CORE_TY_USB_OPER_ERR,
		.name = "��·�����й����з�������,USB HUB��ʧ��,ϵͳҪ����"
		},
		{ .err = DF_ERR_PORT_CORE_TY_USB_PORT_MANAGE_LINUX_FILE,
		.name = "����ϵͳUSB�ļ�ϵͳ������������,������Ҫ����������"
		},
		{ .err = DF_ERR_PORT_CORE_SOCKET_SEV_BIND,
		.name = "socket�󶨶˿�ʧ��"
		},
		{ .err = DF_ERR_PORT_CORE_SOCKET_LISTER,
		.name = "socket�����˿�ʧ��"
		},
		{ .err = DF_ERR_PORT_CORE_SWITCH_SERIA,
		.name = "seria��������"
		},
		{ .err = DF_ERR_PORT_CORE_TY_USB_MAIN_BUSID,
		.name = "û�п��õ���busid"
		},
		{ .err = DF_ERR_PORT_CORE_TY_USB_MAIN_HUB,
		.name = "��hub��ʧ,Ҫ����������"
		},
		{ .err = DF_ERR_PORT_CORE_TY_USB_PCB_HUB,
		.name = "����hub��ʧ"
		},
		{ .err = DF_ERR_PORT_CORE_TY_USB_PORT_NO_ENUM,
		.name = "û���ҵ���Ӧ���豸ö��"
		},
		{ .err = DF_ERR_PORT_CORE_TY_USB_CLOSE,
		.name = "�豸�Ѿ��ر�,���ļ�ϵͳ�����豸��Ϣ,ϵͳ��������"
		},
		{ .err = DF_ERR_PORT_CORE_USB_IP_BUSID_RE,
		.name = "��busid�Ѿ�����"
		},
		{ .err = DF_ERR_PORT_CORE_USB_IP_WRITE,
		.name = "дUSBshareЭ��ջʧ��(bind error)"
		},
		{ .err = DF_ERR_PORT_CORE_FTP_DOWNLOAD_ERR,
		.name = "���ظ���ʧ��"
		},
		{ .err = DF_ERR_PORT_CORE_FTP_DOWNLOAD_PROGRAM_IS_NEW,
		.name = "����Ѿ������°汾"
		},
		{ .err = DF_ERR_PORT_CORE_PORT_ISNOT_SHUIPAN,
		.name = "�ö˿ڷ�֧�ֵ�˰��"
		},
		{ .err = DF_ERR_PORT_CORE_GET_INVOICE_ERR,
		.name = "��ȡ˰�̷�Ʊ��Ϣ����"
		},
		{ .err = DF_ERR_PORT_CORE_GET_FP_MONITOR_DATA,
		.name = "��֧�ִ��෢Ʊ��ѯ"
		},
		{ .err = DF_ERR_PORT_CORE_GET_FP_PARAMETERS,
		.name = "��ȡ˰�̷�Ʊ��������"
		},
		{ .err = DF_ERR_PORT_CORE_GET_FP_OPEER_USB,
		.name = "��ȡ˰����Ϣ����USBʧ��"
		},
		{ .err = DF_ERR_PORT_CORE_GET_FP_INVIOCE_NUM,
		.name = "��ȡ˰�̷�Ʊ��������"
		},
		{ .err = DF_ERR_PORT_CORE_GET_FP_INVOICE_LEN,
		.name = "��ȡ��Ʊ���ݴ�С����"
		}


		};
		static const _err_buf	port_core_err_buf =
		{ port_core_err,
		sizeof(port_core_err) / sizeof(_err),
		0
		};
    #endif




#endif