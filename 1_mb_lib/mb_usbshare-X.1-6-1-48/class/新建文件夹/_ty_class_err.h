#ifndef _TY_CLASS_ERR_DEFINE
	#define _TY_CLASS_ERR_DEFINE
	#define DF_ERR_CLASS_NO_OUT_DEV		DF_ERR_CLASS_START			//��֧�������豸ʹ��
	#define DF_ERR_CLASS_FD_ED			(DF_ERR_CLASS_NO_OUT_DEV-1)	//���豸�Ѿ���
	#define DF_ERR_CLASS_NO_DEV			(DF_ERR_CLASS_START-1)		//�Է�������	
	#define DF_ERR_CLASS_DEV_NO_OPEN	(DF_ERR_CLASS_NO_DEV-1)	//�豸δ��
	#define DF_ERR_CLASS_NO_ORDER		(DF_ERR_CLASS_DEV_NO_OPEN-1)	//ϵͳ��֧�ָ�����
	#define DF_ERR_CLASA_NO_FILE		(DF_ERR_CLASS_NO_ORDER-1)		//�ļ�ϵͳ��֧�ָ�����	
	#define DF_ERR_CLASS_FILE			(DF_ERR_CLASA_NO_FILE-1)		//û�и��ļ�


#endif

