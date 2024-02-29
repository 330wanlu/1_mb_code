#ifndef _TOOLS_BOOT_H
#define _TOOLS_BOOT_H
#include<fcntl.h>
#include "_comm_boot.h"
#include "../software/_code_s.h"
#include "signal.h"
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/statfs.h>
#include <fcntl.h>
	void *boot_malloc(long len);
	int Write_Decompress_file(unsigned char *file, long file_len, unsigned char *filename, unsigned char * Dest_dir);
	int Boot_Strptime(char *szStandTimeIn, struct tm *tt0);
	void HexToStr(char *pszDest, char *pbSrc, int nLen);
	char* str2upper(char *str);
	int File_Compare_MD5(char *src_file, char *comp_file);
	int parse_config_copy_file(unsigned char *configname, unsigned char *now_path, int update_or_check);
	int get_ter_id_ver(char *filename, char *ter_id, char *ver, char *hostname,int *port);
	int write_update_record(char *filename, char *writebuf);
	int get_line(int sock, char *buf, int size);
	int get_local_tar_mad5(char *filepath, char *outmd5);
	int parse_config_back_oldver(char *configname, char *now_path);
	int parse_config_del_backfile(char *configname, char *now_path);


	int detect_process(char * process_name, char *tmp);
	int check_and_close_process(char * process_name);
	int mb_delete_file_dir(char *file_path, int file_type);
	int mb_create_file_dir(char *file_path);
	int mb_tar_decompress(char *tar, char *path);
	int mb_copy_file_path(char *from, char *to);
	int mb_chmod_file_path(char *power, char *path);
	int sleep_hour(int hour);
	int mb_system_read(char *cmd, char *r_buf, int r_buf_len);
	long long calc_path_size(char *path);
#endif