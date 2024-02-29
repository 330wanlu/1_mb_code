#define _ftp_n_c
#include "_ftp_n.h"
int ftp_get_file(char *host,char *user,char *pwd,char *sev_file,char *save_file,int port)
{   int cm_sock;
    int result;
	int ret; 
    unsigned long long stor_size;
    stor_size=0;
    out("登录ftp服务器:%s\n",host);
    cm_sock=ftp_connect(host,port,user,pwd);
    if(cm_sock<0)
    {   out("登录服务器[%s]失败\n",host);   
        return -1;
    }
    result=ftp_retrfile(cm_sock,sev_file,save_file,&stor_size,NULL);
    if(result>=0)
    {   out("下载成功\n");
		ret = 0;
    }
	else
	{
		out("下载失败\n");
		ret = -1;
	}
    ftp_quit(cm_sock);
    //close(cm_sock);
    return ret;    
}


int login_server( int sock, char *user, char *pwd )
{
    char    buf[128];
    int     result;
    
    sprintf( buf, "USER %s\r\n", user );
    result = ftp_sendcmd( sock, buf );
    if ( result == 230 ) return 0;
    else if ( result == 331 ) {
        sprintf( buf, "PASS %s\r\n", pwd );
        if ( ftp_sendcmd( sock, buf ) != 230 ) return -1;
        return 0;
    }
    else
        return -1;
}


int socket_connect(char *host,int port)
{
    struct sockaddr_in address;
    int s, opvalue;
	int result;
    socklen_t slen;
	char ip[20] ;
    memset(ip,0,sizeof(ip));
    opvalue = 8;
    slen = sizeof(opvalue);
    memset(&address, 0, sizeof(address));

	result = get_DNS_ip(host, ip);
	if (result < 0)
	{
		return -1;
	}
    
    if ((s = socket(AF_INET, SOCK_STREAM, 0)) < 0 ||
        setsockopt(s, IPPROTO_IP, IP_TOS, &opvalue, slen) < 0)
        return -1;
    struct timeval timeo = {15, 0};
    setsockopt(s, SOL_SOCKET, SO_SNDTIMEO, &timeo, sizeof(timeo));
    setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &timeo, sizeof(timeo));
    
    address.sin_family = AF_INET;
    address.sin_port = htons((unsigned short)port);
	
	/*result = get_DNS_ip(host, ip);
	if (result < 0)
	{	return -1;
	}*/
	inet_pton(AF_INET, ip, &address.sin_addr);
	if (connect(s, (struct sockaddr*) &address, sizeof(address)) == -1)
	{
		close(s);
		return -1;
	}
    
    return s;
}


int connect_server( char *host, int port )
{    
    int       ctrl_sock;
    char      buf[512];
    int       result;
    ssize_t   len;
    
    ctrl_sock = socket_connect(host, port);
    if (ctrl_sock == -1) 
	{
        return -1;
    }
    
    len = recv( ctrl_sock, buf, 512, 0 );
    buf[len] = 0;
    sscanf( buf, "%d", &result );
    if ( result != 220 ) {
        close( ctrl_sock );
        return -1;
    }
    
    return ctrl_sock;
}


int ftp_sendcmd_re( int sock, char *cmd, void *re_buf, ssize_t *len)
{
    char        buf[512];
    ssize_t     r_len;
    
    if ( send( sock, cmd, strlen(cmd), 0 ) == -1 )
        return -1;
    
    r_len = recv( sock, buf, 512, 0 );
    if ( r_len < 1 ) return -1;
    buf[r_len] = 0;
    
    if (len != NULL) *len = r_len;
    if (re_buf != NULL) sprintf(re_buf, "%s", buf);
    
    return 0;
}


int ftp_sendcmd( int sock, char *cmd )
{
    char     buf[512];
    int      result;
    ssize_t  len;
    
    result = ftp_sendcmd_re(sock, cmd, buf, &len);
    if (result == 0)
    {
        sscanf( buf, "%d", &result );
    }
    
    return result;
}




int create_datasock( int ctrl_sock )
{
    int     lsn_sock;
    int     port;
    int     len;
    struct sockaddr_in sin;
    char    cmd[128];
    
    lsn_sock = socket( PF_INET, SOCK_STREAM, IPPROTO_TCP );
    if ( lsn_sock == -1 ) return -1;
    memset( (char *)&sin, 0, sizeof(sin) );
    sin.sin_family = AF_INET;
    if( bind(lsn_sock, (struct sockaddr *)&sin, sizeof(sin)) == -1 ) {
        close( lsn_sock );
        return -1;
    }
    
    if( listen(lsn_sock, 2) == -1 ) {
        close( lsn_sock );
        return -1;
    }
    
    len = sizeof( struct sockaddr );
    if ( getsockname( lsn_sock, (struct sockaddr *)&sin, (socklen_t *)&len ) == -1 )
    {
        close( lsn_sock );
        return -1;
    }
    port = sin.sin_port;
    
    if( getsockname( ctrl_sock, (struct sockaddr *)&sin, (socklen_t *)&len ) == -1 )
    {
        close( lsn_sock );
        return -1;
    }
    
    sprintf( cmd, "PORT %d,%d,%d,%d,%d,%d\r\n",
            sin.sin_addr.s_addr&0x000000FF,
            (sin.sin_addr.s_addr&0x0000FF00)>>8,
            (sin.sin_addr.s_addr&0x00FF0000)>>16,
            (sin.sin_addr.s_addr&0xFF000000)>>24,
            port>>8, port&0xff );
    
    if ( ftp_sendcmd( ctrl_sock, cmd ) != 200 ) {
        close( lsn_sock );
        return -1;
    }
    return lsn_sock;
}


int ftp_pasv_connect( int c_sock )
{
    int     r_sock;
    int     send_re;
    ssize_t len;
    int     addr[6];
    char    buf[512];
    char    re_buf[512];
    
   
    bzero(buf, sizeof(buf));
    sprintf( buf, "PASV\r\n");
    send_re = ftp_sendcmd_re( c_sock, buf, re_buf, &len);
    if (send_re == 0) {
        sscanf(re_buf, "%*[^(](%d,%d,%d,%d,%d,%d)",&addr[0],&addr[1],&addr[2],&addr[3],&addr[4],&addr[5]);
    }
    
    
    bzero(buf, sizeof(buf));
    sprintf( buf, "%d.%d.%d.%d",addr[0],addr[1],addr[2],addr[3]);
    r_sock = socket_connect(buf,addr[4]*256+addr[5]);
    
    return r_sock;
}


int ftp_type( int c_sock, char mode )
{
    char    buf[128];
    sprintf( buf, "TYPE %c\r\n", mode );
    if ( ftp_sendcmd( c_sock, buf ) != 200 )
        return -1;
    else
        return 0;
}


int ftp_cwd( int c_sock, char *path )
{
    char    buf[128];
    int     re;
    sprintf( buf, "CWD %s\r\n", path );
    re = ftp_sendcmd( c_sock, buf );
    if ( re != 250 )
        return -1;
    else
        return 0;
}


int ftp_cdup( int c_sock )
{
    int     re;
    re = ftp_sendcmd( c_sock, "CDUP\r\n" );
    if ( re != 250 )
        return re;
    else
        return 0;
}


int ftp_mkd( int c_sock, char *path )
{
    char    buf[512];
    int     re;
    sprintf( buf, "MKD %s\r\n", path );
    re = ftp_sendcmd( c_sock, buf );
    if ( re != 257 )
        return re;
    else
        return 0;
}


int ftp_list( int c_sock, char *path, void **data, unsigned long long *data_len)
{
    int     d_sock;
    char    buf[512];
    int     send_re;
    int     result;
    ssize_t     len,buf_len,total_len;
    
   
    d_sock = ftp_pasv_connect(c_sock);
    if (d_sock == -1) {
        return -1;
    }
    
    
    bzero(buf, sizeof(buf));
    sprintf( buf, "LIST %s\r\n", path);
    send_re = ftp_sendcmd( c_sock, buf );
    if (send_re >= 300 || send_re == 0)
        return send_re;
    
    len=total_len = 0;
    buf_len = 512;
    void *re_buf = malloc(buf_len);
    while ( (len = recv( d_sock, buf, 512, 0 )) > 0 )
    {
        if (total_len+len > buf_len)
        {
            buf_len *= 2;
            void *re_buf_n = malloc(buf_len);
            memcpy(re_buf_n, re_buf, total_len);
            free(re_buf);
            re_buf = re_buf_n;
        }
        memcpy(re_buf+total_len, buf, len);
        total_len += len;
    }
    close( d_sock );
    
    
    bzero(buf, sizeof(buf));
    len = recv( c_sock, buf, 512, 0 );
    buf[len] = 0;
    sscanf( buf, "%d", &result );
    if ( result != 226 )
    {
        free(re_buf);
        return result;
    }
    
    *data = re_buf;
    *data_len = total_len;
    
    return 0;
}


int ftp_retrfile( int c_sock, char *s, char *d ,unsigned long long *stor_size, int *stop)
{
    int     d_sock;
    ssize_t     len,write_len;
    char    buf[512];
    int     handle;
    int     result;
    out("打开本地文件:%s\n",d);
    handle = open( d,  O_WRONLY|O_CREAT|O_TRUNC, S_IREAD|S_IWRITE );
    if ( handle == -1 ) 
    {   out("打开本地文件失败\n");
        return -1;
    }    
    
    out("设置传输模式\n");
    ftp_type(c_sock, 'I');
    
    out("连接数据通道\n");
    d_sock = ftp_pasv_connect(c_sock);
    if (d_sock == -1)
    {   out("数据通道连接失败");
        close(handle);
        return -1;
    }
    
    bzero(buf, sizeof(buf));
    sprintf( buf, "RETR %s\r\n", s );
    out("发送命令:%s\n",buf);
    result = ftp_sendcmd( c_sock, buf );
    out("命令返回:result=%d\n",result);
    if (result >= 300 || result == 0)
    {   out("RETR失败");
        close(handle);
        return result;
    }
    bzero(buf, sizeof(buf));
    while ( (len = recv( d_sock, buf, 512, 0 )) > 0 ) {
        write_len = write( handle, buf, len );
        if (write_len != len || (stop != NULL && *stop))
        {
            close( d_sock );
            close( handle );
            return -1;
        }
        
        if (stor_size != NULL)
        {
            *stor_size += write_len;
        }
        out("下载数据:%d\n",*stor_size);
    }
    close( d_sock );
    close( handle );
    out("下载完成\n");
    bzero(buf, sizeof(buf));
    len = recv( c_sock, buf, 512, 0 );
    buf[len] = 0;
    sscanf( buf, "%d", &result );
    if ( result >= 300 ) {
        return result;
    }
    return 0;
}


int ftp_storfile( int c_sock, char *s, char *d ,unsigned long long *stor_size, int *stop)
{
    int     d_sock;
    ssize_t     len,send_len;
    char    buf[512];
    int     handle;
    int send_re;
    int result;
    
    //??????
    handle = open( s,  O_RDONLY);
    if ( handle == -1 ) return -1;
    
    //??????
    ftp_type(c_sock, 'I');
    
    //???PASV??
    d_sock = ftp_pasv_connect(c_sock);
    if (d_sock == -1)
    {
        close(handle);
        return -1;
    }
    
    //??STOR??
    bzero(buf, sizeof(buf));
    sprintf( buf, "STOR %s\r\n", d );
    send_re = ftp_sendcmd( c_sock, buf );
    if (send_re >= 300 || send_re == 0)
    {
        close(handle);
        return send_re;
    }
    
    //???PASV?????
    bzero(buf, sizeof(buf));
    while ( (len = read( handle, buf, 512)) > 0)
    {
        send_len = send(d_sock, buf, len, 0);
        if (send_len != len ||
            (stop != NULL && *stop))
        {
            close( d_sock );
            close( handle );
            return -1;
        }
        
        if (stor_size != NULL)
        {
            *stor_size += send_len;
        }
        out("下载大小:%d\n",*stor_size);
    }
    close( d_sock );
    close( handle );
    
    //?????????
    bzero(buf, sizeof(buf));
    len = recv( c_sock, buf, 512, 0 );
    buf[len] = 0;
    sscanf( buf, "%d", &result );
    if ( result >= 300 ) {
        return result;
    }
    return 0;
}

//?????&????
int ftp_renamefile( int c_sock, char *s, char *d )
{
    char    buf[512];
    int     re;
    
    sprintf( buf, "RNFR %s\r\n", s );
    re = ftp_sendcmd( c_sock, buf );
    if ( re != 350 ) return re;
    sprintf( buf, "RNTO %s\r\n", d );
    re = ftp_sendcmd( c_sock, buf );
    if ( re != 250 ) return re;
    return 0;
}

//????
int ftp_deletefile( int c_sock, char *s )
{
    char    buf[512];
    int     re;
    
    sprintf( buf, "DELE %s\r\n", s );
    re = ftp_sendcmd( c_sock, buf );
    if ( re != 250 ) return re;
    return 0;
}

//????
int ftp_deletefolder( int c_sock, char *s )
{
    char    buf[512];
    int     re;
    
    sprintf( buf, "RMD %s\r\n", s );
    re = ftp_sendcmd( c_sock, buf );
    if ( re != 250 ) return re;
    return 0;
}

//登录服务器
int ftp_connect( char *host, int port, char *user, char *pwd )
{
    int     c_sock;
    c_sock = connect_server( host, port );
    if ( c_sock == -1 ) return -1;
    if ( login_server( c_sock, user, pwd ) == -1 ) {
        close( c_sock );
        return -1;
    }
    return c_sock;
}

//退出登录
int ftp_quit( int c_sock)
{
    int re = 0;
    re = ftp_sendcmd( c_sock, "QUIT\r\n" );
    close( c_sock );
    return re;
}