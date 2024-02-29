#ifndef _TY_CLASS_ERR_DEFINE
	#define _TY_CLASS_ERR_DEFINE
	#define DF_ERR_CLASS_NO_OUT_DEV		DF_ERR_CLASS_START			//不支持其他设备使用
	#define DF_ERR_CLASS_FD_ED			(DF_ERR_CLASS_NO_OUT_DEV-1)	//该设备已经打开
	#define DF_ERR_CLASS_NO_DEV			(DF_ERR_CLASS_START-1)		//对方不存在	
	#define DF_ERR_CLASS_DEV_NO_OPEN	(DF_ERR_CLASS_NO_DEV-1)	//设备未打开
	#define DF_ERR_CLASS_NO_ORDER		(DF_ERR_CLASS_DEV_NO_OPEN-1)	//系统不支持该命令
	#define DF_ERR_CLASA_NO_FILE		(DF_ERR_CLASS_NO_ORDER-1)		//文件系统不支持该命令	
	#define DF_ERR_CLASS_FILE			(DF_ERR_CLASA_NO_FILE-1)		//没有该文件


#endif

