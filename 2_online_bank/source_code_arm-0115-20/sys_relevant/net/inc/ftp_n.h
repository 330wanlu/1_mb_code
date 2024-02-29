#ifndef _FTP_N_H
#define _FTP_N_H

/*===================================°üº¬ÎÄ¼þ============================================*/
#include "opt.h"

#include "linux_ip.h"

#define ftp_n_out(s,arg...)         //log_out("_lcd_state.c",s,##arg)                       

int ftp_get_file(char *host, char *user, char *pwd, char *sev_file, char *save_file, int port);
int socket_connect(char *host,int port);
int connect_server( char *host, int port );
int ftp_sendcmd_re( int sock, char *cmd, void *re_buf, ssize_t *len);
int ftp_sendcmd( int sock, char *cmd );
int login_server( int sock, char *user, char *pwd );
int create_datasock( int ctrl_sock );
int ftp_pasv_connect( int c_sock );
int ftp_type( int c_sock, char mode );
int ftp_cwd( int c_sock, char *path );
int ftp_cdup( int c_sock );
int ftp_mkd( int c_sock, char *path );
int ftp_list( int c_sock, char *path, void **data, unsigned long long *data_len);
int ftp_retrfile( int c_sock, char *s, char *d ,unsigned long long *stor_size, int *stop);
int ftp_storfile( int c_sock, char *s, char *d ,unsigned long long *stor_size, int *stop);
int ftp_renamefile( int c_sock, char *s, char *d );
int ftp_deletefile( int c_sock, char *s );
int ftp_deletefolder( int c_sock, char *s );
int ftp_connect( char *host, int port, char *user, char *pwd );
int ftp_quit( int c_sock);

#endif
