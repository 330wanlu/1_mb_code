#ifndef _MAIN_DEFINF
    #define _MAIN_DEFINF
    /*=================================�����ļ�==============================================*/
    #include "_load.h"
    #include "../software/_app_file.h"
    #include <getopt.h>
	#include <termios.h> 
	#include <stdio.h>
	#include <sys/wait.h>
	#include "_app.h"
	#include "../core/_ty_usb_m.h"
	#include <iconv.h>
    /*================================����������============================================*/
    #ifdef _main_c
        #define out(s,arg...)               log_out("main.c",s,##arg)
    #endif
    /*================================�ṹ�嶨��============================================*/
#define NEW_BOOT 
    
    
    /*=================================��������=============================================*/
    #ifdef _main_c
        static int get_dev_ver(void);
        static int oper_app(void);
        static int oper_debug(void);
		static int get_help(void);
		static int test_invoice(void);
		static int test_rsa(void);
    #endif
    /*================================��������==============================================*/
    #ifdef _main_c
        static const struct option longopts[] = 
        {   {   .name="version",
                .has_arg=no_argument,
                .flag=NULL,
                .val='v'
            },
            {   .name="help",
                .has_arg=no_argument,
                .flag=NULL,
                .val='h'
            },
            {   .name="Dperation",
                .has_arg=no_argument,
                .flag=NULL,
                .val='D'
            },
            {   .name="debug",
                .has_arg=no_argument,
                .flag=NULL,
                .val='d'
            },
            {   .name=NULL,
                .has_arg=no_argument,
                .flag=NULL,
                .val=0
            }
        };
    #endif   
    
#ifdef _main_c
		int get_ter_id_ver(char *filename, char *ter_id, char *ver);
#endif
    


#endif