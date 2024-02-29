#ifndef DEFINE_TAX_AUTH
#define DEFINE_TAX_AUTH

#include "function.h"
#define AUTH_CERT_FILE_PATH		"/etc/auth_cert.json"

#ifdef auth_c
#define out(s,arg...)               log_out("all.c",s,##arg)
#else
#endif

struct _busid_auth
{
	int state;
	int port;
	char busid[32];
	int aisino_sup;
	int nisec_sup;
	char start_time[20];//��ʼʱ��
	char end_time[20];//����ʱ��
	char remark[20];
};

struct _tax_auth
{
	char version[128];
	char serial_num[128];
	char name[128];
	char auth_type[10];//usb-share��Ȩ���ͣ�whole part��wholeģʽ������ʱЧ partģʽ����shixiao
	char start_time[20];//usb-share��ʼʱ��
	char end_time[20];//usb-share����ʱ��
	struct _busid_auth busid_auth[100];

};
struct _tax_auth  tax_auth;

int init_tax_lib(uint8 *key_zlib_json, char *taxlib_version);
int get_cert_json_file(uint8 *key_zlib_json, char **cert_json);
int auth_busnum_devnum_tax_fun(int nBusID, int nDevID, char *busid, int *aisino_sup, int *nisec_sup);


#ifdef auth_c
#ifdef NEED_AUTH
static int analysis_auth_cert_json(char *cert_json);
#endif
#endif

#endif