#ifndef ERR_H_
#define ERR_H_

//计算机系统错误
#define DF_SYSTEM_ERROR                     -1//操作系统错误
#define DF_USB_LOAD_TIMEOUT                 -2//端口加载超时
#define DF_USB_UNLOAD_TIMEOUT               -3//端口释放超时


//中间件错误
#define DF_ERR_EXCUTE_USBSHARE_ERR		   -12001//USB总线端口申请失败
#define DF_ERR_MIDWARE_SYSTEM_ERR          -12002 //中间件系统错误


#define DF_ERR_USBPORT_ERROR               -12200 //端口号错误
#define DF_ERR_TERMINAL_ID                 -12201//终端ID错误 


#define DF_ERR_NO_WRTITE_SETTING           -14003 //写配置表错误
#define DF_ERR_READ_SETTING                -14004 //读配置文件错误
#define DF_ERR_NO_SEARCH_SCHEDULE          -14005 //没有税盘明细查询任务
#define DF_ERR_ON_SEARCHING                -14006 //正在查询，请进行进度查询
#define DF_SEARCH_COMPLETE                 -14007 //税盘发票明细查询已完成

#define DF_ERR_CLIENT_PROTOCOL             -16000//协议不支持



/*****************************************************终端错误*********************************************************/

#define DF_ERR_TERMINAL_PROTOCOL_START		-10001
#define DF_ERR_SOCKET_ERROR               (DF_ERR_TERMINAL_PROTOCOL_START-1)//socket错误
#define DF_ERR_PROTOCOL_ERROR             (DF_ERR_SOCKET_ERROR-1)//协议错误
#define DF_ERR_TERMINAL_REOPONSE_ERROR    (DF_ERR_PROTOCOL_ERROR-1)//终端命令错误


#define DF_ERR_PORT_MANAGE_START            -13001
#define DF_ERR_NO_RIGHT_CLOSE_PORT          (DF_ERR_PORT_MANAGE_START-1)//无权关闭该端口
#define DF_ERR_UBSPORT_OPENED               (DF_ERR_NO_RIGHT_CLOSE_PORT-1)//该端口已经本机打开
#define DF_ERR_USBPORT_NO_DEVICE            (DF_ERR_UBSPORT_OPENED-1) //端口无设备

#define DF_ERR_NO_ACCESS_CABINET            -14002 //配置表没有有效机器
#define DF_ERR_TERMINAL_OFFLINE             -14003 //终端不在线

//终端系统错误
#define DF_ERR_TERMINAL_SYSTEM              -15001 //终端系统错误
#define DF_ERR_NO_RIGHT_TERMINAL           (DF_ERR_TERMINAL_ID-1)//无操作权限
#define DF_UPGRADE_PROGRAM_ERROR           (DF_ERR_NO_RIGHT_TERMINAL-1)//升级终端程序失败

//税盘错误
#define DF_ERR_INVOICE                      -15002 //发票查询错误(本月无发票)
#define DF_ERR_GUARD                        -15003 //不支持此类型的监控信息查询
#define DF_ERR_TYPE_NO_SUPPORT              -15004 //税盘不支持
#define DF_ERR_PORT_CORE_GETFP_PARAMENTS    -15005 //获取税盘发票参数错误
#define DF_ERR_PORT_GETFP_OPENR_USB         -15006 //获取税盘信息操作USB失败
#define DF_ERR_PORT_CORE_GET_FP_INVOICE_NUM -15007 //获取税盘发票数量错误
#define DF_ERR_PORT_CORE_GETFP_INVOICE_LEN  -15008 //获取发票数据大小错误






#endif

