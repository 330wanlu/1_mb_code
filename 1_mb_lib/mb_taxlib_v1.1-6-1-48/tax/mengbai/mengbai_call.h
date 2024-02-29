#ifndef MENBAI_CALL_H
#define MENBAI_CALL_H
//#include "../../linux_s/_timer.h"
//#include <libusb-1.0/libusb.h>
//#include "../../software/_utf82gbk.h"
//#include "../../software/_algorithm.h"
//#include "../../software/_code_s.h"
//#include "../../software/cJSON.h"
//#include "../../software/_base64.h"
//#include "../../core/_port_core.h"
//#include "../common/tax_info.h"
#include "usb/hid_io.h"
//#include "../../software/_zlib.h"
#include <openssl/x509v3.h>
#include "../common/importinv.h"
#include "../common/function.h"
#ifdef mengbai_call_c
	#ifdef DEBUG
		#define out(s,arg...)       				log_out("all.c",s,##arg) 
	#else
		#define out(s,arg...)						log_out("all.c",s,##arg) 
	#endif	
#endif
#ifdef mengbai_call_c
	#define DF_PC_USB_ORDER_CONNECT			0X01				//��������
	#define DF_PC_USB_ORDER_GET_TIME		0X02				//��ȡʱ��
	#define DF_PC_USB_ORDER_SET_TIME		0X03				//����ʱ��
	#define DF_PC_USB_ORDER_GET_BASE_INFO	0X04				//��ѯ��������Ϣ
	#define DF_PC_USB_ORDER_GET_SER_ID		0X05				//��ȡ��Ƭ��Ψһ��ʶ
	#define DF_PC_USB_ORDER_SET_ID			0X06				//���ÿ����
	#define DF_PC_USB_ORDER_FORMAT			0X07				//��ʽ����
	#define DF_PC_USB_ORDER_GET_CARD_IFNFO	0X11				//��ȡ����Ϣ
	#define DF_PC_USB_ORDER_GET_JKXXSJ		0X12				//�����Ϣ���ݲ�ѯ
	#define DF_PC_USB_ORDER_GET_SQSL		0X13				//��Ȩ˰�ʲ�ѯ
	#define DF_PC_USB_ORDER_GET_WSYFPJ		0X14				//�ѷַ�δʹ�÷�Ʊ��ѯ
	#define DF_PC_USB_ORDER_GET_LXPFS		0X15				//��ѯ����Ʊ����
	#define DF_PC_USB_ORDER_GET_SCLXP		0X16				//��ȡ�ϴ�����Ʊ
	#define DF_PC_USB_ORDER_GET_FPMX		0X17				//��ѯ��Ʊ��ϸ
	#define DF_PC_USB_ORDER_GET_FPMXSL		0X18				//��ѯ��Ʊ��ϸ����
	#define DF_PC_USB_ORDER_GET_L_TIME		0X19				//��ȡ˰����Сʱ��
	#define DF_PC_USB_ORDER_BGZSKL			0X21				//���֤�����
	#define DF_PC_USB_ORDER_ZSKLHCSZ		0X22				//֤�����س�ʼֵ
	#define DF_PC_USB_ORDER_CSHKK			0X23				//��ʼ������
	#define DF_PC_USB_ORDER_ZJFPLX			0X24				//���ӷ�Ʊ����
	#define DF_PC_USB_ORDER_SCFPLX			0X25				//ɾ����Ʊ����
	#define DF_PC_USB_ORDER_ZJSQSL	        0X26                //������Ȩ˰��
	#define DF_PC_USB_ORDER_SCSQSL          0X27                //ɾ����Ȩ˰��
	#define DF_PC_USB_ORDER_LGFP	        0X28                //�칺��Ʊ
	#define DF_PC_USB_ORDER_FPHT		    0X29                //��Ʊ����
	#define DF_PC_USB_ORDER_FPKJ			0X2A                //��Ʊ����
	#define DF_PC_USB_ORDER_FPZF		    0X2B                //��Ʊ����
	#define DF_PC_USB_ORDER_KBZF			0X2C                //�հ׷�
	#define DF_PC_USB_ORDER_CB				0X2D                //����
	#define DF_PC_USB_ORDER_JKXXHC			0X2E				//�����Ϣ�ش�
	#define DF_PC_USB_ORDER_LXPSCCG			0X2F				//����Ʊ�ϴ��ɹ�
	#define DF_PC_USB_ORDER_GET_FPQM		0X20				//��ȡ��Ʊǩ��
	#define DF_PC_USB_ORDER_YZZSKL			0X31				//��֤֤�����


	

	static int pack_s_buf(uint8 order, int s_num, uint8 *in_buf, uint8 *out_buf, int in_len);
	
	//static int String2Bytes(unsigned char *szSrc, unsigned char *pDst, int nDstMaxLen);
	static void month_to_month_range(unsigned char *month, unsigned char *month_range);
	//static int check_fpxx(struct Fpxx* stp_fpxx, struct _plate_infos *plate_infos);
	static int pack_get_inv_json(unsigned char *in_data, int in_len, unsigned char *out_data, unsigned char *ca_name, unsigned char *ca_number);
	static int pack_offline_report_json_data(unsigned char *in_data, unsigned char *json_data, unsigned char *hjje, char *name, char *number);

	static int mengbai_upload_firstOfflineInv(HDEV hDev, struct _offline_upload_result *offline_upload_result);
	
	

	static int check_fpxx_mengbai(struct Fpxx* stp_fpxx, struct _plate_infos *plate_infos, char *errinfo);
	static int pack_bin_data(struct Fpxx* stp_fpxx, unsigned char* pc_Bin, struct _make_invoice_result *make_invoice_result);
	static int analysis_json_pack_bin_data(HDEV hDev, struct Fpxx *stp_fpxx, unsigned char *json_data, unsigned char *bin_data, struct _plate_infos *plate_infos, struct _make_invoice_result *make_invoice_result);
	static int exchange_spmc_spsmmc(char *spsmmc_mc, int size);

#endif

	
	int LoadMengBaiInfo(HUSB hUSB, HDEV hDev);
	int mengbai_read_basic_info(HDEV hDev, struct _plate_infos *plate_infos);
	int mengbai_read_inv_type_areacode(HUSB hUSB, struct _plate_infos *plate_infos);
	int mengbai_read_monitor_info(HUSB hUSB, unsigned char *inv_type, struct _plate_infos *plate_infos);
	int mengbai_read_inv_coil_number(HUSB hUSB, unsigned char *inv_type, struct _plate_infos *plate_infos);
	int mengbai_read_plate_time(HUSB hUSB, unsigned char *plate_time);
	int mengbai_read_plate_least_time(HUSB hUSB, unsigned char *least_time);
	int mengbai_set_plate_time(HUSB hUSB, unsigned char *set_time);
	int mengbai_read_plate_support_slv(HUSB hUSB, unsigned char *inv_type, struct _plate_infos *plate_infos);
	int mengbai_read_invoice_num(HUSB hUSB, unsigned char *month, unsigned int *count, unsigned long *count_size);
	int mengbai_read_inv_details_mqtt(HDEV hDev, unsigned char *month, void *function, void *arg, char *errinfo, int old_new, unsigned short *start_stop);
	int mengbai_read_inv_to_mengbai_server(HUSB hUSB, unsigned char *month, void *function, void *arg, struct _plate_infos *plate_infos, int common_num, int ser_now_num, unsigned short *start_stop);
	int mengbai_upload_m_server(HDEV hDev, unsigned short *start_stop, struct _upload_inv_num *upload_inv_num, void *function, void *arg, struct _inv_sum_data *now_month_sum);
	int mengbai_read_offline_inv_num(HUSB hUSB, unsigned char *inv_type, struct _plate_infos *plate_infos);
	int mengbai_read_offline_inv_details(HUSB hUSB, unsigned char inv_type, int num, char *inv_data, int *inv_len, unsigned char *hjje, char *name, char *number);
	int mengbai_set_monitor_data(HUSB hUSB, unsigned char inv_type, char *errinfo);
	int mengbai_get_chao_bao_data(HDEV hDev, uint8 inv_type, char *errinfo);
	int mengbai_make_invoice_to_plate(HDEV hDev, HUSB hUSB, unsigned char inv_type, char *inv_info, struct _plate_infos *plate_infos, struct _make_invoice_result *make_invoice_result);
	int mengbai_cancel_invoice_to_plate(HDEV hDev, unsigned char inv_type, char *fpdm, char *fphm, struct _cancel_invoice_result *cancel_invoice_result);
	int mengbai_cancel_null_invoice_to_plate(HUSB hUSB, struct _cancel_invoice_result *cancel_invoice_result);
	int mengbai_set_invoice_to_plate(HUSB hUSB, unsigned char inv_type);
	int mengbai_read_invoice_sign(HUSB hUSB, unsigned char inv_type, unsigned char *inv_info, int inv_len, unsigned char *sign);
	int mengbai_read_cert_passwd(HUSB hUSB, unsigned char *c_passwd);
	int mengbai_read_current_inv_code(HUSB hUSB, unsigned char inv_type, char* invnum, char* invcode);
	int mengbai_inv_upload_server(HDEV hDev, struct _offline_upload_result *offline_upload_result);
	int mengbai_check_server_address(HDEV hDev, char *errinfo);

	int LoadMengBaiSeverKey(HUSB hUSB, HDEV hDev);
	int mengbai_get_server_key_info(HUSB hUSB, HDEV hDev, char *info);
#endif
