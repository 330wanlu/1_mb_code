#ifndef MANAGE_SET_FILE_H_
#define MANAGE_SET_FILE_H_

#include <Windows.h>
#include <tchar.h>
//#include "typedef_data.h"
#include <string.h>

struct file_setting{
	char terminal_ip[100];
	int terminal_port;
	int data_port;
	int state;

	int used;
//#ifdef NINGBO_BANK
	char remote_ip[100];
	int remote_dataport;
	int remote_port;
//#endif
};

struct files_setting{
	struct file_setting terminal_setting[128];
	int count;
	char guid[64];
	HANDLE hmutex;
	HANDLE process_id;
};


#ifdef DF_MANAGE_SET_FILE
struct files_setting all_terminal_setting_table;
#endif

#ifdef LOGLEVEL
int i_LogLevel;
#else
extern int i_LogLevel;
#endif

int old_read_setting(struct file_setting file[], int size_file);
int old_write_setting(struct file_setting file[], int size_file);


int write_setting(struct file_setting file[], int size_file);
int read_setting(struct file_setting	file[], int size_file);
BOOL terminal_setting_init(void);
int get_client_id(char* client_id);
int read_guid(char *buf, int size);

#endif
