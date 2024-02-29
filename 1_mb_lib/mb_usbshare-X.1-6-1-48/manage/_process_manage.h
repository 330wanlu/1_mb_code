#ifndef _PROCESS_MANAGE_DEFINE
	#define _PROCESS_MANAGE_DEFINE
	#include "signal.h"
	#include <stdio.h>
	#include "../_opt.h"
	#include <string.h>
	#include <sys/types.h>
	#include <sys/stat.h>
	#include <fcntl.h>
#ifndef RELEASE_SO
	#include "_log.h"
	#include "../linux_s/_timer.h"
#endif

#define DF_PROCESS_MANAGE_UDHCPC				"udhcpc"
#define DF_PROCESS_AUTO_NTPDATE					"/etc/autotime.sh"
#define DF_PROCESS_MANAGE_OPENVPN				"/home/share/openvpn/openvpn"
#define DF_PROCESS_MANAGE_CA_SERVER				"/home/share/exe/ca_server"
#ifdef RELEASE_SO
	#define DF_PROCESS_MANAGE_CA_PRO_N				"/home/share/exe/mb_ca_reader_n_d"
	#define DF_PROCESS_MANAGE_CA_PRO_S				"/home/share/exe/mb_ca_reader_s_d"
	#define DF_PROCESS_MANAGE_CA_PRO_U				"/home/share/exe/mb_ca_reader_u_d"
	#define DF_PROCESS_MANAGE_CA_PRO_NEW			"/home/share/exe/mb_ca_reader_n_d"
#else
	#define DF_PROCESS_MANAGE_CA_PRO_N				"/home/share/exe/MB2409113CX_N_d"
	#define DF_PROCESS_MANAGE_CA_PRO_S				"/home/share/exe/MB2409113CX_S_d"
	#define DF_PROCESS_MANAGE_CA_PRO_U				"/home/share/exe/MB2409113CX_U_d"
	#define DF_PROCESS_MANAGE_CA_PRO_NEW			"/home/share/exe/mb_ca_reader_n_d"
#endif

	#ifdef _process_manage_c
		 #ifdef DEBUG
			#define out(s,arg...)       				//log_out("all.c",s,##arg) 
		#else
			#define out(s,arg...)						//log_out("all.c",s,##arg)
		#endif		 
        #define e_process_manage
    #else
        #define e_process_manage                    extern
    #endif
	e_process_manage	int main_start_init();
	e_process_manage	int detect_process(char * process_name,char *tmp);
	e_process_manage	int check_and_close_process(char * process_name);
	e_process_manage	int detect_process_path(char * process_name, char *path);

#endif

