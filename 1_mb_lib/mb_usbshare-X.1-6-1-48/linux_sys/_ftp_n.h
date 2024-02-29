#ifndef _FTP_N_DEFINE
    #define _FTP_N_DEFINE
    /*===================================包含文件============================================*/
    #include "../_opt.h"
    #include <stdio.h>
    #include <sys/types.h>
    #include <sys/stat.h>
    #include <fcntl.h>
    #include <sys/socket.h>
    #include <netdb.h>
    #include <stdio.h>
    #include <ctype.h>
    #include <stdlib.h>
    #include <unistd.h>
    #include <string.h>
    #include <sys/ioctl.h>
	#ifdef MTK_OPENWRT
	#include "_linux_ip_mtk.h"
	#else
	#include "_linux_ip.h"
	#endif
    /*===================================立即数定义=========================================*/
    #ifdef _ftp_n_c
        #define e_ftp_n
        #define out(s,arg...)                               
    #else
        #define e_ftp_n                         extern
    #endif
    /*==================================函数定义============================================*/
	e_ftp_n int ftp_get_file(char *host, char *user, char *pwd, char *sev_file, char *save_file, int port);
    e_ftp_n int socket_connect(char *host,int port);
    e_ftp_n int connect_server( char *host, int port );
    e_ftp_n int ftp_sendcmd_re( int sock, char *cmd, void *re_buf, ssize_t *len);
    e_ftp_n int ftp_sendcmd( int sock, char *cmd );
    e_ftp_n int login_server( int sock, char *user, char *pwd );
    e_ftp_n int create_datasock( int ctrl_sock );
    e_ftp_n int ftp_pasv_connect( int c_sock );
    e_ftp_n int ftp_type( int c_sock, char mode );
    e_ftp_n int ftp_cwd( int c_sock, char *path );
    e_ftp_n int ftp_cdup( int c_sock );
    e_ftp_n int ftp_mkd( int c_sock, char *path );
    e_ftp_n int ftp_list( int c_sock, char *path, void **data, unsigned long long *data_len);
    e_ftp_n int ftp_retrfile( int c_sock, char *s, char *d ,unsigned long long *stor_size, int *stop);
    e_ftp_n int ftp_storfile( int c_sock, char *s, char *d ,unsigned long long *stor_size, int *stop);
    e_ftp_n int ftp_renamefile( int c_sock, char *s, char *d );
    e_ftp_n int ftp_deletefile( int c_sock, char *s );
    e_ftp_n int ftp_deletefolder( int c_sock, char *s );
    e_ftp_n int ftp_connect( char *host, int port, char *user, char *pwd );
    e_ftp_n int ftp_quit( int c_sock);



#endif

