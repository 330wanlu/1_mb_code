#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include<sys/types.h>
#include<sys/stat.h>
#include <stdlib.h>
#include <openssl/sha.h>
#include <openssl/md5.h>
#include <ctype.h>


void *boot_malloc(long len);
char* str2upper(char *str);
void HexToStr(unsigned char *pszDest, unsigned char *pbSrc, int nLen);
int read_file_md5(char *filepath, unsigned char *outmd_str);
int mb_system_read(char *cmd, char *r_buf, int r_buf_len,char *tmp_file);
int mb_system_read_sleep(char *cmd, char *r_buf, int r_buf_len, int time);
int mb_delete_file_dir_n(char *file_path, int file_type);
int mb_system_read_tmp(char *cmd, char *r_buf, int r_buf_len, char *tmp_path);