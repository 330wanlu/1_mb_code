#ifndef _PORT_CORE_DEFINE
	#define _PORT_CORE_DEFINE
	#include "../file/_file.h"
#ifndef RELEASE_SO
	#include <libusb-1.0/libusb.h>
	#include "../tax/interface_main.h"

	#include "../linux_sys/_file_oper.h"
	#include "../linux_s/_folder.h"
	#include "../linux_s/_timer.h"
	#include "../linux_s/_t_file_c.h"

	#include "../manage/_log_file.h"

	#include "../software/_des.h"
	#include "../software/_base64.h"
	#include "../software/cJSON.h"
	#include "../software/_utf82gbk.h"
	#include "../software/_zlib.h"
	#include "../software/_Date.h"
	#include "../software/_app_file.h"
	#include "../software/_code_s.h"
	#include "../software/_algorithm.h"
#endif
	
	/*===============================´íÎó´úÂë¶¨Òå==================================*/
	#define DF_ERR_PORT_CORE_NO_DEV						DF_ERR_PORT_CORE_START
	#define DF_ERR_PORT_CORE_NO_SPACE					(DF_ERR_PORT_CORE_NO_DEV-1)
	#define DF_ERR_PORT_CORE_SLAVE_SPACE				(DF_ERR_PORT_CORE_NO_SPACE-1)
	#define DF_ERR_PORT_CORE_MAIN_DEV					(DF_ERR_PORT_CORE_SLAVE_SPACE-1)
	#define DF_ERR_PORT_CORE_SLAVE_DEV					(DF_ERR_PORT_CORE_MAIN_DEV-1)
	#define DF_ERR_PORT_CORE_CM							(DF_ERR_PORT_CORE_SLAVE_DEV-1)
	#define DF_ERR_PORT_CORE_OPEN_NULL					(DF_ERR_PORT_CORE_CM-1)
	#define DF_ERR_PORT_CORE_CM_BPS						(DF_ERR_PORT_CORE_OPEN_NULL-1)
	#define DF_ERR_PORT_CORE_CM_MAIN					(DF_ERR_PORT_CORE_CM_BPS-1)
	#define DF_ERR_PORT_CORE_BSP						(DF_ERR_PORT_CORE_CM_MAIN-1)
	#define DF_ERR_PORT_CORE_FD_NULL					(DF_ERR_PORT_CORE_BSP-1)
	#define DF_ERR_PORT_CORE_FD_Z						(DF_ERR_PORT_CORE_FD_NULL-1)
	#define	DF_ERR_PORT_CORE_PRINT_LINE					(DF_ERR_PORT_CORE_FD_Z-1)
	#define DF_ERR_PORT_CORE_FEED_PAPER					(DF_ERR_PORT_CORE_PRINT_LINE-1)
	#define DF_ERR_PORT_CORE_BACK						(DF_ERR_PORT_CORE_FEED_PAPER-1)
	#define DF_ERR_PORT_CORE_BLACK						(DF_ERR_PORT_CORE_BACK-1)
	#define DF_ERR_PORT_CORE_STATE						(DF_ERR_PORT_CORE_BLACK-1)
	#define DF_ERR_PORT_CORE_PAR						(DF_ERR_PORT_CORE_STATE-1)
	#define DF_ERR_PORT_CORE_FONT						(DF_ERR_PORT_CORE_PAR-1)
	#define DF_ERR_PORT_CORE_SET_LEN					(DF_ERR_PORT_CORE_FONT-1)
	#define DF_ERR_PORT_CORE_BID						(DF_ERR_PORT_CORE_SET_LEN-1)
	#define DF_ERR_PORT_BID_SET							(DF_ERR_PORT_CORE_BID-1)
	#define DF_ERR_PORT_TEST							(DF_ERR_PORT_BID_SET-1)
	#define DF_ERR_PORT_CORE_NO_MEM						(DF_ERR_PORT_TEST-1)
	#define DF_ERR_PORT_CORE_SAM_NOPOWER				(DF_ERR_PORT_CORE_NO_MEM-1)
	#define DF_ERR_PORT_CORE_SAM_POWERED				(DF_ERR_PORT_CORE_SAM_NOPOWER-1)
    /*--------------------------------switch_dev----------------------------------------------*/
    #define DF_ERR_PORT_CORE_SWITCH_SERIA               (DF_ERR_PORT_CORE_SAM_POWERED-1)
    
    
    
    /*---------------------------------ty_usb_core--------------------------------------------*/
    #define DF_ERR_PORT_CORE_TY_USB_INIT                (DF_ERR_PORT_CORE_SWITCH_SERIA-1)
    #define DF_ERR_PORT_CORE_TY_USB_PORT_OVER           (DF_ERR_PORT_CORE_TY_USB_INIT-1)
    #define DF_ERR_PORT_CORE_TY_USB_PCB_NO              (DF_ERR_PORT_CORE_TY_USB_PORT_OVER-1)
    #define DF_ERR_PORT_CORE_TY_USB_STM32_OPEN          (DF_ERR_PORT_CORE_TY_USB_PCB_NO-1)
    #define DF_ERR_PORT_CORE_TY_USB_BUS_NUMBER          (DF_ERR_PORT_CORE_TY_USB_STM32_OPEN-1)
    #define DF_ERR_PORT_CORE_TY_USB_NO_FIND_DEV         (DF_ERR_PORT_CORE_TY_USB_BUS_NUMBER-1)
	#define DF_ERR_PORT_CORE_TY_USB_NO_PERMISSION  		(DF_ERR_PORT_CORE_TY_USB_NO_FIND_DEV-1)
    #define DF_ERR_PORT_CORE_TY_USB_STM32_ANSWER        (DF_ERR_PORT_CORE_TY_USB_NO_PERMISSION-1)
    #define DF_ERR_PORT_CORE_TY_USB_STM32_CLOSE         (DF_ERR_PORT_CORE_TY_USB_STM32_ANSWER-1)
    #define DF_ERR_PORT_CORE_TY_USB_STM32_PCB           (DF_ERR_PORT_CORE_TY_USB_STM32_CLOSE-1)
    #define DF_ERR_PORT_CORE_TY_USB_STM32_DEV           (DF_ERR_PORT_CORE_TY_USB_STM32_PCB-1)
    #define DF_ERR_PORT_CORE_TY_USB_TREE_READ           (DF_ERR_PORT_CORE_TY_USB_STM32_DEV-1)
    #define DF_ERR_PORT_CORE_TY_USB_TREE_PCB            (DF_ERR_PORT_CORE_TY_USB_TREE_READ-1)
    #define DF_ERR_PORT_CORE_TY_USB_PORT_USED           (DF_ERR_PORT_CORE_TY_USB_TREE_PCB-1)
    #define DF_ERR_PORT_CORE_TY_USB_OPER_ERR            (DF_ERR_PORT_CORE_TY_USB_PORT_USED-1)
    /*-----------------------------ty_usb-----------------------------------------------------*/
    #define DF_ERR_PORT_CORE_TY_USB_MAIN_BUSID          (DF_ERR_PORT_CORE_TY_USB_OPER_ERR-1)
    #define DF_ERR_PORT_CORE_TY_USB_MAIN_HUB            (DF_ERR_PORT_CORE_TY_USB_MAIN_BUSID-1)    
    #define DF_ERR_PORT_CORE_TY_USB_PCB_HUB             (DF_ERR_PORT_CORE_TY_USB_MAIN_HUB-1)
    #define DF_ERR_PORT_CORE_TY_USB_PORT_NO_ENUM        (DF_ERR_PORT_CORE_TY_USB_PCB_HUB-1)  
    #define DF_ERR_PORT_CORE_TY_USB_CLOSE               (DF_ERR_PORT_CORE_TY_USB_PORT_NO_ENUM-1)    
    
    /*-----------------------------ty_usb_port_manage-----------------------------------------*/
    #define DF_ERR_PORT_CORE_TY_USB_PORT_MANAGE_PORT_USEED          (DF_ERR_PORT_CORE_TY_USB_CLOSE-1)
    #define DF_ERR_PORT_CORE_TY_USB_SET_START_ZERO      (DF_ERR_PORT_CORE_TY_USB_PORT_MANAGE_PORT_USEED-1)
    #define DF_ERR_PORT_CORE_TY_USB_PORT_MANAGE_NO_CLOSE            (DF_ERR_PORT_CORE_TY_USB_SET_START_ZERO-1)
    #define DF_ERR_PORT_CORE_TY_USB_PORT_MANAGE_CHECK_USB_BUSY      (DF_ERR_PORT_CORE_TY_USB_PORT_MANAGE_NO_CLOSE-1)
    #define DF_ERR_PORT_CORE_TY_USB_PORT_MANAGE_CHECK_PAR           (DF_ERR_PORT_CORE_TY_USB_PORT_MANAGE_CHECK_USB_BUSY-1)
    #define DF_ERR_PORT_CORE_TY_USB_PORT_MANAGE_LINUX_FILE          (DF_ERR_PORT_CORE_TY_USB_PORT_MANAGE_CHECK_PAR-1)
    /*---------------------------------usb ip-------------------------------------------------*/
    #define DF_ERR_PORT_CORE_USB_IP_ROOT                (DF_ERR_PORT_CORE_TY_USB_PORT_MANAGE_LINUX_FILE-1)
    #define DF_ERR_PORT_CORE_USB_IP_KO                  (DF_ERR_PORT_CORE_USB_IP_ROOT-1)
    #define DF_ERR_PORT_CORE_USB_IP_SOCKET              (DF_ERR_PORT_CORE_USB_IP_KO-1)
    #define DF_ERR_PORT_CORE_REGIT_ERR                  (DF_ERR_PORT_CORE_USB_IP_SOCKET-1)
    /*--------------------------------socket_sev----------------------------------------------*/
    #define DF_ERR_PORT_CORE_SOCKET_SEV_BIND            (DF_ERR_PORT_CORE_REGIT_ERR-1)
    #define DF_ERR_PORT_CORE_SOCKET_LISTER              (DF_ERR_PORT_CORE_SOCKET_SEV_BIND-1)
    #define DF_ERR_PORT_CORE_USB_IP_BUSID_RE            (DF_ERR_PORT_CORE_SOCKET_LISTER-1)
    #define DF_ERR_PORT_CORE_USB_IP_WRITE               (DF_ERR_PORT_CORE_USB_IP_BUSID_RE-1)
	#define DF_ERR_PORT_CORE_FTP_DOWNLOAD_ERR			(DF_ERR_PORT_CORE_USB_IP_WRITE-1)
    #define	DF_ERR_PORT_CORE_FTP_DOWNLOAD_PROGRAM_IS_NEW	(DF_ERR_PORT_CORE_FTP_DOWNLOAD_ERR-1)
	#define DF_ERR_PORT_CORE_PORT_ISNOT_SHUIPAN			(DF_ERR_PORT_CORE_FTP_DOWNLOAD_PROGRAM_IS_NEW-1)
	#define DF_ERR_PORT_CORE_GET_INVOICE_ERR			(DF_ERR_PORT_CORE_PORT_ISNOT_SHUIPAN-1)
	#define DF_ERR_PORT_CORE_GET_FP_MONITOR_DATA		(DF_ERR_PORT_CORE_GET_INVOICE_ERR-1)
	#define DF_ERR_PORT_CORE_GET_FP_PARAMETERS			(DF_ERR_PORT_CORE_GET_FP_MONITOR_DATA-1)
	#define DF_ERR_PORT_CORE_GET_FP_OPEER_USB			(DF_ERR_PORT_CORE_GET_FP_PARAMETERS-1)
	#define DF_ERR_PORT_CORE_GET_FP_INVIOCE_NUM			(DF_ERR_PORT_CORE_GET_FP_OPEER_USB-1)
	#define DF_ERR_PORT_CORE_GET_FP_INVOICE_LEN			(DF_ERR_PORT_CORE_GET_FP_INVIOCE_NUM-1)





#endif