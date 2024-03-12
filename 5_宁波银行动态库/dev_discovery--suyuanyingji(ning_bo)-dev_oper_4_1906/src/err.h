#ifndef ERR_H_
#define ERR_H_

//�����ϵͳ����
#define DF_SYSTEM_ERROR                     -1//����ϵͳ����
#define DF_USB_LOAD_TIMEOUT                 -2//�˿ڼ��س�ʱ
#define DF_USB_UNLOAD_TIMEOUT               -3//�˿��ͷų�ʱ


//�м������
#define DF_ERR_EXCUTE_USBSHARE_ERR		   -12001//USB���߶˿�����ʧ��
#define DF_ERR_MIDWARE_SYSTEM_ERR          -12002 //�м��ϵͳ����


#define DF_ERR_USBPORT_ERROR               -12200 //�˿ںŴ���
#define DF_ERR_TERMINAL_ID                 -12201//�ն�ID���� 


#define DF_ERR_NO_WRTITE_SETTING           -14003 //д���ñ����
#define DF_ERR_READ_SETTING                -14004 //�������ļ�����
#define DF_ERR_NO_SEARCH_SCHEDULE          -14005 //û��˰����ϸ��ѯ����
#define DF_ERR_ON_SEARCHING                -14006 //���ڲ�ѯ������н��Ȳ�ѯ
#define DF_SEARCH_COMPLETE                 -14007 //˰�̷�Ʊ��ϸ��ѯ�����

#define DF_ERR_CLIENT_PROTOCOL             -16000//Э�鲻֧��



/*****************************************************�ն˴���*********************************************************/

#define DF_ERR_TERMINAL_PROTOCOL_START		-10001
#define DF_ERR_SOCKET_ERROR               (DF_ERR_TERMINAL_PROTOCOL_START-1)//socket����
#define DF_ERR_PROTOCOL_ERROR             (DF_ERR_SOCKET_ERROR-1)//Э�����
#define DF_ERR_TERMINAL_REOPONSE_ERROR    (DF_ERR_PROTOCOL_ERROR-1)//�ն��������


#define DF_ERR_PORT_MANAGE_START            -13001
#define DF_ERR_NO_RIGHT_CLOSE_PORT          (DF_ERR_PORT_MANAGE_START-1)//��Ȩ�رոö˿�
#define DF_ERR_UBSPORT_OPENED               (DF_ERR_NO_RIGHT_CLOSE_PORT-1)//�ö˿��Ѿ�������
#define DF_ERR_USBPORT_NO_DEVICE            (DF_ERR_UBSPORT_OPENED-1) //�˿����豸

#define DF_ERR_NO_ACCESS_CABINET            -14002 //���ñ�û����Ч����
#define DF_ERR_TERMINAL_OFFLINE             -14003 //�ն˲�����

//�ն�ϵͳ����
#define DF_ERR_TERMINAL_SYSTEM              -15001 //�ն�ϵͳ����
#define DF_ERR_NO_RIGHT_TERMINAL           (DF_ERR_TERMINAL_ID-1)//�޲���Ȩ��
#define DF_UPGRADE_PROGRAM_ERROR           (DF_ERR_NO_RIGHT_TERMINAL-1)//�����ն˳���ʧ��

//˰�̴���
#define DF_ERR_INVOICE                      -15002 //��Ʊ��ѯ����(�����޷�Ʊ)
#define DF_ERR_GUARD                        -15003 //��֧�ִ����͵ļ����Ϣ��ѯ
#define DF_ERR_TYPE_NO_SUPPORT              -15004 //˰�̲�֧��
#define DF_ERR_PORT_CORE_GETFP_PARAMENTS    -15005 //��ȡ˰�̷�Ʊ��������
#define DF_ERR_PORT_GETFP_OPENR_USB         -15006 //��ȡ˰����Ϣ����USBʧ��
#define DF_ERR_PORT_CORE_GET_FP_INVOICE_NUM -15007 //��ȡ˰�̷�Ʊ��������
#define DF_ERR_PORT_CORE_GETFP_INVOICE_LEN  -15008 //��ȡ��Ʊ���ݴ�С����






#endif

