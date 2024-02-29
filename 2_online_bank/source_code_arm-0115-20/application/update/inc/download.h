#ifndef _DOWNLOAD_H
#define _DOWNLOAD_H


/************************************************************
Copyright (C), 2016, Leon, All Rights Reserved.
FileName: download.c
coding: UTF-8
Description: 实现简单的http下载功能
Author: Leon
Version: 1.0
Date: 2016-12-2 10:49:32
Function:
 
History:
<author>    <time>  <version>   <description>
 Leon
 
************************************************************/
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>
#include <netdb.h>
#include <errno.h>
 
#define HOST_NAME_LEN   256
#define URI_MAX_LEN     2048
#define RECV_BUF        8192
#define RCV_SND_TIMEOUT (10*1000)   //收发数据超时时间(ms)
 
typedef struct {
    int sock;                       //与服务器通信的socket
    FILE *in;                       //sock描述符转为文件指针，方便读写
    char host_name[HOST_NAME_LEN];  //主机名
    int port;                       //主机端口号
    char uri[URI_MAX_LEN];          //资源路径
    char buffer[RECV_BUF];          //读写缓冲
    int status_code;                //http状态码
    int chunked_flag;               //chunked传输的标志位
    int len;                        //Content-length里的长度
    char location[URI_MAX_LEN];     //重定向地址
    char *save_path;                //保存内容的路径指针
    FILE *save_file;                //保存内容的文件指针
    int recv_data_len;              //收到数据的总长度
    time_t start_recv_time;         //开始接受数据的时间
    time_t end_recv_time;           //结束接受数据的时间
} http_t_m;
 
/* 打印宏 */
#define MSG_DEBUG   0x01
#define MSG_INFO    0x02
#define MSG_ERROR   0x04
 
static int print_level = /*MSG_DEBUG |*/ MSG_INFO | MSG_ERROR;
 
#define lprintf(level, format, argv...) do{     \
    if(level & print_level)     \
        printf("[%s][%s(%d)]:"format, #level, __FUNCTION__, __LINE__, ##argv);  \
}while(0)
 
#define MIN(x, y) ((x) > (y) ? (y) : (x))
 
#define HTTP_OK         200
#define HTTP_REDIRECT   302
#define HTTP_NOT_FOUND  404





char *strncasestr_m(char *str, char *sub);
int parser_URL_m(char *url, http_t_m *info);
unsigned long dns_m(char* host_name);
int set_socket_option_m(int sock);
int connect_server_m(http_t_m *info);
int send_request_m(http_t_m *info);
int parse_http_header_m(http_t_m *info);
int save_data_m(http_t_m *info, const char *buf, int len);
int read_data_m(http_t_m *info, int len);
int recv_chunked_response_m(http_t_m *info);
float calc_download_speed_m(http_t_m *info);
int recv_response_m(http_t_m *info);
void clean_up_m(http_t_m *info);
int http_download_m(char *url, char *save_path);


int download_main_m(int argc, char *argv[]);

#endif