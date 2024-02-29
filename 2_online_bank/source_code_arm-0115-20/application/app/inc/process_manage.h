#ifndef _PROCESS_MANAGE_DEFINE
#define _PROCESS_MANAGE_DEFINE

#include "opt.h"

#include "timer.h"

#define DF_PROCESS_MANAGE_UDHCPC				"udhcpc"
#define DF_PROCESS_AUTO_NTPDATE					"/etc/autotime.sh"
#define DF_PROCESS_MANAGE_OPENVPN				"/home/share/openvpn/openvpn"
#define DF_PROCESS_MANAGE_CA_SERVER				"/home/share/exe/ca_server"

#define DF_PROCESS_MANAGE_CA_PRO_N				"/home/share/exe/mb_ca_reader_n_d"
#define DF_PROCESS_MANAGE_CA_PRO_S				"/home/share/exe/mb_ca_reader_s_d"
#define DF_PROCESS_MANAGE_CA_PRO_U				"/home/share/exe/mb_ca_reader_u_d"
#define DF_PROCESS_MANAGE_CA_PRO_NEW			"/home/share/exe/mb_ca_reader_n_d"

// #define DF_PROCESS_MANAGE_CA_PRO_N				"/home/share/exe/MB2409113CX_N_d"
// #define DF_PROCESS_MANAGE_CA_PRO_S				"/home/share/exe/MB2409113CX_S_d"
// #define DF_PROCESS_MANAGE_CA_PRO_U				"/home/share/exe/MB2409113CX_U_d"
// #define DF_PROCESS_MANAGE_CA_PRO_NEW			"/home/share/exe/mb_ca_reader_n_d"

#define process_manage_out(s,arg...)       				//log_out("all.c",s,##arg) 

int main_start_init();
int detect_process(char * process_name,char *tmp);
int check_and_close_process(char * process_name);
int detect_process_path(char * process_name, char *path);

#endif
