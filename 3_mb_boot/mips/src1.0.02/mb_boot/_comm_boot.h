#ifndef _COMM_BOOT_H
#define _COMM_BOOT_H
	#include <time.h>
	#include <sys/types.h>
	#include <sys/socket.h>

#include <netinet/in.h>
#include <netinet/tcp.h>
	#include <sys/stat.h>
	#include <arpa/inet.h>
	#include <netdb.h>
	#include <stdio.h>
	#include <errno.h>
	#include <stdlib.h>
	#include <string.h>
	#include <unistd.h>
	#include <openssl/sha.h>
	#include <openssl/md5.h>
	#include "../software/cJSON.h"
	#include "../software/_base64.h"
	#include <ctype.h>
	#include <pthread.h>  
	#include <sys/time.h>

/*===========================================================================================*/
#define LOG_P
//#define TEST
#define MIPS_MTK

#define UPDATE 2
#define CHECK 3

#define NO_UPDATE	0
#define ONLY_APP	1
#define ONLY_AUTH	2
#define BOTH_APP_AUTH	3


#define HTTP_PORT		10066
#define HTTP_ADDR		"www.njmbxx.com"//"103.45.250.88"
#define HTTP_CONTENT_TYPE "application/x-www-form-urlencoded"

#define BOOT_APP_PATH		"/home/share/exe/mb_usbshare_*"
#define BOOT_APP_PATH_RUN	"/tmp/mb_usbshare_n_d"//"/home/share/exe/mb_usbshare_n_d" 
#define DOWNLOAD_FILENAME	"/tmp/app.tar"
#define DOWNLOAD_AUTH		"/tmp/auth.tar"
#define TER_ID_INFO_PATH	"/etc/ter.info"
#define TAR_CONFIG_PATH	"/home/share/exe/tar/mb_config.cnf"
#define TAR_PATH		"/home/share/exe/tar"


#define TAR_FILE_PATH_APP	"/home/share/exe/tar/app.tar"
#define TAR_FILE_PATH_AUTH	"/home/share/exe/tar/auth.tar"

#define TAR_FILE_APP_DECTECT_PATH	"/home/share/exe/tar/app"
#define TAR_FILE_AUTH_DECTECT_PATH	"/home/share/exe/tar/auth"

#define UPDATE_FILE_RECORD_PATH_APP		"/etc/update_app_mb.log"
#define UPDATE_FILE_RECORD_PATH_AUTH	"/etc/update_auth_mb.log"

#define HTTP_TIMEOUT	60


/*******************************************ERROR CODE DEFINE*************************************/

#define BOOT_ERROR_OPER_PARSE_CONFIG_NOT_FOUND_CONFIG	-200
#define BOOT_ERROR_OPER_PARSE_CONFIG_ERR_OPEN_CONFIG	-201
#define BOOT_ERROR_OPER_PARSE_CONFIG_ERR_FORMAT_CONFIG	-202
#define BOOT_ERROR_OPER_PARSE_CONFIG_ERR_PATH_NOT_FOUND	-203
#define BOOT_ERROR_OPER_PARSE_CONFIG_ERR_FILE_NOT_FOUND	-204	//tar 包下没发现对应文件


#define BOOT_ERROR_OPER_SYSTEM_MALLOC_ERR		-300
#define BOOT_ERROR_OPER_SYSTEM_READ_ERR			-301


#define BOOT_ERROR_OPER_DETECT_APP_NOT_FOUND	-401
#define BOOT_ERROR_OPER_DETECT_FILE_CHECK_ERR	-402


#define BOOT_ERROR_HTTP_SEND	-500
#define BOOT_ERROR_HTTP_RECV	-501
#define BOOT_ERROR_HTTP_PARSE_JSON	-502
#define BOOT_ERROR_HTTP_RECV_PACKET_TOO_LARGE	-503

#define HTTP_TOKEN_ERR	-999
	struct http_parm{
		 char token[100];
		 char ter_id[13];
		 char timestamp[20];
		 char app_ver[20];
		 char tar_md5[50];
		 char appid[20];
		 char errmsg[500];
		char ip[30];
		//int first_no_app;
		
	};
	struct err_n{
		int err_no;
		char err_info[200];
	};

#endif