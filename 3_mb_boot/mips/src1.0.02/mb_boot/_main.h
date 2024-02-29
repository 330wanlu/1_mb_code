#ifndef _MAIN_DEFINE
#define _MAIN_DEFINE
#include "../_opt.h"
#include "../software/_code_s.h"
#include "../software/_app_file.h"
#include "_comm_boot.h"
#include "_http_boot.h"
#include "_tools_boot.h"

void *main_app_thread(void *arg);
void *update_thread(void *arg);
int parse_errcode_upload(int socketfd, struct http_parm *parm, int errcode);
int run_main_app();
int dectec_app(char *errinfo);
int get_real_host(char *config_host, char *real_host);
static const char *soft_d[] =
{
	"/home/share/exe/mb_usbshare_s",
	"/home/share/exe/mb_usbshare_n",
	"/home/share/exe/mb_usbshare_u"
};

 static const struct err_n err_info[] =
{
	{ BOOT_ERROR_OPER_PARSE_CONFIG_NOT_FOUND_CONFIG, "The .tar file not include .config file!" },
	{ BOOT_ERROR_OPER_PARSE_CONFIG_ERR_OPEN_CONFIG, "Error open .config file!" },
	{ BOOT_ERROR_OPER_PARSE_CONFIG_ERR_FORMAT_CONFIG, "The .config file format not right!" },
	{ BOOT_ERROR_OPER_PARSE_CONFIG_ERR_PATH_NOT_FOUND, "The path read of .config file not found in terminal!" },
	{ BOOT_ERROR_OPER_PARSE_CONFIG_ERR_FILE_NOT_FOUND, "The .tar file not include the file you write to the .config file!" },
	{ BOOT_ERROR_OPER_SYSTEM_MALLOC_ERR, "Err, system malloc err!" },
	{ BOOT_ERROR_OPER_SYSTEM_READ_ERR, "Err,system read file err!" },
	{ BOOT_ERROR_OPER_DETECT_APP_NOT_FOUND, "Not found the main app!" },
	{ BOOT_ERROR_OPER_DETECT_FILE_CHECK_ERR, "The tar file is not right!" },
	{ BOOT_ERROR_HTTP_SEND ,"HTTP SEND ERR!"},
	{ BOOT_ERROR_HTTP_RECV ,"HTTP RECV  ERR!"},
	{ BOOT_ERROR_HTTP_PARSE_JSON ,"HTTP JSON ERR!"},
	{ BOOT_ERROR_HTTP_RECV_PACKET_TOO_LARGE ,"HTTP PACKET TOO LARGE!"},
};

#endif
