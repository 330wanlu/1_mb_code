#ifndef _UPDATE_H
#define _UPDATE_H

#include "opt.h"
#include "mb_tools.h"
#include "process_manage.h"
#include "machine_infor.h"
#include "usb_port.h"

#include "http_reg.h"
#include "../inc/download.h"


// #include <stdio.h>    
// #include <stdlib.h>   
// #include <string.h>   
// #include <sys/types.h>
// #include <sys/stat.h> 
// #include <fcntl.h>    
// #include <pthread.h>  
// #include <stdint.h>   
// #include <unistd.h>   
// #include <stdbool.h>  

// extern unsigned char update_msg[1024];
// extern unsigned char auth_msg[1024];

unsigned char update_msg_m[1024];
unsigned char auth_msg_m[1024];

struct http_parm_m{
    unsigned char token[100];
    unsigned char ter_id[13];
    unsigned char timestamp[20];
    unsigned char app_ver[20];
    unsigned char tar_md5[50];
    unsigned char appid[20];
    unsigned char errmsg[500];
    char ip[30];
    int port;
    //int first_no_app;
    
};

struct err_n{
    int err_no;
    unsigned char err_info[200];
};


static const char *soft_d[] =
{
	"/home/share/exe/mb_usbshare_s",
	"/home/share/exe/mb_usbshare_n",
	"/home/share/exe/mb_usbshare_u"
};
#define HTTP_TOKEN_ERR	-999
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
#define BOOT_ERROR_HTTP_SPACE_FULL	-504

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

	struct _s_info_new
	{
		char name[20];//主程序名称
		char type[10];//主程序适用机型
		char app_ver[20];//主程序版本号
		char app_md5[20];//主程序MD5 值  有效16位
		long app_len;//主程序长度
		char *app_content;//主程序内容
	};


    struct _app_file_note
    {   char name[100];             //程序名称
        int len;                    //程序长度
        char r_date[20];            //发布日期
        char ver[20];               //版本号
        char writer[20];            //作者
    };

#define update_out(s,arg...)       				log_out("all.c",s,##arg) 
#define net_update_out(s,arg...)       				log_out("all.c",s,##arg) 

void update_task_hook(void *arg, int timer);
void intranet_update_task_hook(void *arg, int timer);
void *intranet_update_thread(void *arg);
int dectec_app(char *errinfo);


int app_file_get_new(const char *file, struct _app_file_note *file_note);
int app_file_detect_m(const char *file,char *outerr);
int GetDnsIp(char *hostname, char *hostip);
int http_api_test(int socketfd,char *ip,int port);
int deal_http_data_m(int sockfd, char **outbuf);
int mb_tar_decompress_m(char *tar, char *path);
int mb_create_file_dir_m(char *file_path);
int parse_config_copy_file_m(unsigned char *configname,unsigned char *now_path,int update_or_check);
int mb_copy_file_path_m(char *from, char *to);
int mb_system_read_m(char *cmd, char *r_buf, int r_buf_len);
int write_update_record_m(char *filename, char *writebuf);
int app_file_get_infor_new_m(const char *file, struct _s_info_new *infor);
static int app_file_write_real_app_m(char *file, char *writebuf,int len);
int mb_delete_file_dir_m(char *file_path, int file_type);
int get_line_m(int sock, char *buf, int size);
int File_Compare_MD5_m(char *src_file, char *comp_file);
static int get_soft_file(const char *file,char *r_file);
int get_dir_file_m(const char *dir,char *r_name);
int get_dir_m(const char *dir,char *r_dir);
static int get_s_info_new_m(struct	_s_info_new		*infor, char *buf);
int http_get_token_m(int socketfd, struct http_parm_m *parm);
int http_get_server_time_m(int socketfd, unsigned char *time,char *ip,int port);


int http_get_ter_update(int socketfd, struct http_parm_m *parm);

int Write_Decompress_file_m(unsigned char *file, long file_len, unsigned char *filename, unsigned char * Dest_dir);
int parse_config_del_backfile(char *configname, char *now_path);
int mb_chmod_file_path_m(char *power, char *path);
// void intranet_update_task_hook(void *arg);
int read_txt_data(char *temp_url);

#endif
