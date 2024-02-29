#ifndef MB_TOOLS_H
#define MB_TOOLS_H

#include "opt.h"

void *boot_malloc(long len);
char* str2upper(char *str);
void HexToStr(unsigned char *pszDest, unsigned char *pbSrc, int nLen);
int read_file_md5(char *filepath, unsigned char *outmd_str);
int mb_system_read(char *cmd, char *r_buf, int r_buf_len,char *tmp_file);
int mb_system_read_sleep(char *cmd, char *r_buf, int r_buf_len, int time);
int mb_delete_file_dir_n(char *file_path, int file_type);
int mb_system_read_tmp(char *cmd, char *r_buf, int r_buf_len, char *tmp_path);

int get_ter_id_ver(char *filename, char *ter_id,char *ver,char *hostname,int *port);
int get_local_tar_mad5(char *filepath,char *outmd5);
int parse_config_back_oldver(char *configname, char *now_path);
int sleep_hour(int hour);
long long calc_path_size(char *path);

#endif
