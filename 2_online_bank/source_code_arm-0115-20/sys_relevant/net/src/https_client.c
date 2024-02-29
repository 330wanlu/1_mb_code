#include "https_client.h"
 
char *http_find_header(struct evhttp_request* remote_rsp, const char *key)
{
    struct evkeyvalq* headers = evhttp_request_get_input_headers(remote_rsp);
    struct evkeyval *header;

    TAILQ_FOREACH(header, headers, next)
	{
        if (evutil_ascii_strcasecmp(header->key, key) == 0)
		{
			return (header->value);
		}
    }
    return (NULL);
}

void RemoteConnectionCloseCallback(struct evhttp_connection* connection, void* arg)
{
	fprintf(stderr, "remote connection closed\n");
	//CallBackparm* callback_parm = (CallBackparm*)arg;
	event_base_loopexit((struct event_base*)(arg), NULL);
}

#if 0
//解析post请求数据
void get_post_message(char *buf, struct evhttp_request *req)
{
	size_t post_size = 0;
	post_size = evbuffer_get_length(req->input_buffer);//获取数据长度
	printf("====line:%d,post len:%d\n", __LINE__, (int)post_size);
	if (post_size <= 0)
	{
		printf("====line:%d,post msg is empty!\n", __LINE__);
		return;
	}
	else
	{
		char *post_data = (char *)evbuffer_pullup(req->input_buffer, -1);
		//printf("====line:%d,postadta:%s\n", __LINE__, post_data);
		size_t copy_len = post_size > BUF_MAX ? BUF_MAX : post_size;
		//printf("====line:%d,post len:%d, copy_len:%d\n", __LINE__, (int)post_size, (int)copy_len);
		memcpy(buf, evbuffer_pullup(req->input_buffer, -1), (int)copy_len);
		buf[post_size] = '\0';
		printf("====line:%d,post msg:%s\n", __LINE__, buf);
	}
}

void RemoteReadCallback(struct evhttp_request* remote_rsp, void* arg)
{
	int bsize = remote_rsp->body_size;
	char buf[BUF_MAX] = { 0 };
    printf("Content-Type = %s\n",http_find_header(remote_rsp,"Content-Type"));
	get_post_message(buf, remote_rsp);//获取请求数据，一般是json格式的数据
	event_base_loopexit((struct event_base*)arg, NULL);
}

int ReadHeaderDoneCallback(struct evhttp_request* remote_rsp, void* arg)
{
	fprintf(stderr, "< HTTP/1.1 %d %s\n", evhttp_request_get_response_code(remote_rsp), evhttp_request_get_response_code_line(remote_rsp));
	struct evkeyvalq* headers = evhttp_request_get_input_headers(remote_rsp);
	struct evkeyval* header;
	TAILQ_FOREACH(header, headers, next)
	{
		fprintf(stderr, "< %s: %s\n", header->key, header->value);
	}
	fprintf(stderr, "< \n");
	return 0;
}
 
void ReadChunkCallback(struct evhttp_request* remote_rsp, void* arg)
{
	char buf[4096];
	struct evbuffer* evbuf = evhttp_request_get_input_buffer(remote_rsp);
	int n = 0;
	while ((n = evbuffer_remove(evbuf, buf, 4096)) > 0)
	{
		fwrite(buf, n, 1, stdout);
	}
}
 
void RemoteRequestErrorCallback(enum evhttp_request_error error, void* arg)
{
	fprintf(stderr, "request failed\n");
	event_base_loopexit((struct event_base*)arg, NULL);
}
 

#endif

int ConnectTarget(char *host,int port)
{
    struct sockaddr_in them;
    struct hostent *hostip;
    int s, i = 0, nConnectRetry = 0;// nTimeout = 0;
    bool bConnectOK = false;
	struct timeval timeout;
	if(!(hostip=gethostbyname(host)))
	{
		printf("gethostbyname(%s) error!\n", host);
		return -2;
	}
    memset((char *)&them, 0, sizeof(them));
    them.sin_family = AF_INET;
    them.sin_port = htons(port);
    them.sin_addr=*((struct in_addr *)hostip->h_addr);
    s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (s == -1)
	{
		return -4;
	}
	// int nRecvBufLen = 32 * 1024; //设置为32K
	// setsockopt( s, SOL_SOCKET, SO_RCVBUF, ( const char* )&nRecvBufLen, sizeof( int ) );

	//  int flags = fcntl(s,F_GETFL,0);
	//  fcntl(s,F_SETFL,flags | O_NONBLOCK);
	timeout.tv_sec = 5;//5秒超时
	setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(struct timeval));
    nConnectRetry = 200;
    // 5s循环一次，检测是否需要跳出,重试nConnectRetry次;连接建立完成后超时时间为nTimeout
    for (i = 0; i < nConnectRetry; i++)
    {
        if (connect(s, (struct sockaddr *)&them, sizeof(them)) != -1)
        {
            bConnectOK = true;
            break;
        }
    }
    if (bConnectOK)
    {
        return s;
    }
	else
	{
        close(s);
        return -5;
    }
}

int https_post(char* url,char* body,int body_len,char* content_type,char* cookie,FunRemoteReadCallback call_back,void* args,int* result)
{
	//printf("lbc----------------enter https_post\n");
	CallBackparm callback_parm = {
		.args = NULL,
		.base = NULL,
		.result = NULL,
	};
	int length = body_len;
	struct evhttp_uri* uri = evhttp_uri_parse(url);
	if (!uri)
	{
		fprintf(stderr, "parse url failed!\n");
		return 1;
	}
	struct event_base* base = event_base_new();
	event_reinit(base);
	if (!base)
	{
		fprintf(stderr, "create event base failed!\n");
		return 1;
	}
	// struct evdns_base* dnsbase = evdns_base_new(base, 1);
	// if (!dnsbase)
	// {
	// 	fprintf(stderr, "create dns base failed!\n");
	// }
	//assert(dnsbase);
	callback_parm.base = base;
	callback_parm.args = args;
	callback_parm.result = result;
	struct evhttp_request* request = evhttp_request_new(call_back,(void*)(&callback_parm));
//  evhttp_request_set_header_cb(request, ReadHeaderDoneCallback);
// 	evhttp_request_set_chunked_cb(request, ReadChunkCallback);
// 	evhttp_request_set_error_cb(request, RemoteRequestErrorCallback);
	const char* host = evhttp_uri_get_host(uri);
	if (!host)
	{
		fprintf(stderr, "parse host failed!\n");
		return 1;
	}
 
	int port = evhttp_uri_get_port(uri);
	if (port < 0) port = 80;
 
	const char* request_url = url;
	const char* path = evhttp_uri_get_path(uri);
	if (path == NULL || strlen(path) == 0)
	{
		request_url = "/";
	}
 
    //printf("lbc----------------------url:%s host:%s port:%d\n", url, host, port);
//add ssl
    SSL *ssl;
	SSL_CTX *ctx;
	SSL_library_init();
	SSL_load_error_strings();
	ctx = SSL_CTX_new(SSLv23_client_method());
	if(ctx == NULL)
	{
		return -1;
	}
	ssl = SSL_new(ctx);
    int sockfd = ConnectTarget((char*)host,port);
    if(sockfd < 0) 
    {
        printf("sockfd < 0\n");
        return -1;
    }
    struct  bufferevent* evEvent = bufferevent_openssl_socket_new(
                base,sockfd,ssl,BUFFEREVENT_SSL_CONNECTING,
                BEV_OPT_CLOSE_ON_FREE | BEV_OPT_DEFER_CALLBACKS | BEV_OPT_THREADSAFE);
        
    if (evEvent == NULL)
	{
		return -1;
	}
    bufferevent_openssl_set_allow_dirty_shutdown(evEvent,1);
    struct evhttp_connection* evConn = evhttp_connection_base_bufferevent_new(base,NULL,evEvent,host,port);
	if(evConn == NULL)
	{
		return -1;
	}
	evhttp_connection_set_retries(evConn, 2);
	evhttp_connection_set_timeout(evConn, 5);
//end add ssl
	// struct evhttp_connection* connection = evhttp_connection_base_new(base, dnsbase, host, port);
	// if (!connection)
	// {
	// 	fprintf(stderr, "create evhttp connection failed!\n");
	// 	return 1;
	// }
	evhttp_connection_set_closecb(evConn, RemoteConnectionCloseCallback,base);
	evhttp_add_header(evhttp_request_get_output_headers(request),"Host",host);
	evhttp_add_header(evhttp_request_get_output_headers(request),"Connection","keep-alive");
	// evhttp_add_header(evhttp_request_get_output_headers(request),"Accept","text/javascript, application/javascript, application/ecmascript, application/x-ecmascript, */*; q=0.01");
	// evhttp_add_header(evhttp_request_get_output_headers(request),"Origin","https://fpdk.jiangsu.chinatax.gov.cn:81");
	// evhttp_add_header(evhttp_request_get_output_headers(request),"X-Requested-With","XMLHttpRequest");
	// evhttp_add_header(evhttp_request_get_output_headers(request),"Sec-Fetch-Dest","empty");
	
	evhttp_add_header(evhttp_request_get_output_headers(request),"User-Agent","Mozilla/5.0 (Windows NT 10.0; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/80.0.3987.87 Safari/537.36 SE 2.X MetaSr 1.0");
	if(NULL != content_type)
		evhttp_add_header(evhttp_request_get_output_headers(request),"Content-Type",content_type);
	// evhttp_add_header(evhttp_request_get_output_headers(request),"Sec-Fetch-Site","same-origin");
	// evhttp_add_header(evhttp_request_get_output_headers(request),"Sec-Fetch-Mode","cors");
	//evhttp_add_header(evhttp_request_get_output_headers(request),"Referer","https://fpdk.jiangsu.chinatax.gov.cn:81/sigin.3447ff68.html?_1654655983653");
	// evhttp_add_header(evhttp_request_get_output_headers(request),"Accept-Encoding","gzip, deflate, br");
	// evhttp_add_header(evhttp_request_get_output_headers(request),"Accept-Language","zh-CN,zh;q=0.9");
	if(NULL != cookie)
		evhttp_add_header(evhttp_request_get_output_headers(request),"Cookie",cookie);
	struct evbuffer *buffer = NULL;
	if(NULL != body && 0 < length)
	{
	 	buffer = evhttp_request_get_output_buffer(request);
		evbuffer_add(buffer, body, length);
	}
	evhttp_make_request(evConn, request,EVHTTP_REQ_POST,request_url);
	event_base_dispatch(base);
	close(sockfd);
	event_base_free(base);
	// SSL_shutdown(ssl);
	// SSL_free(ssl);
	// SSL_CTX_free(ctx);
	return 0;
}

void RemoteReadCallback(struct evhttp_request* remote_rsp, void* arg)
{
    event_base_loopexit((struct event_base*)arg, NULL);
}

int https_get(char* url,char* body,int body_len,char* content_type,char* cookie,FunRemoteReadCallback call_back,void* args,int* result)
{
	int sockfd = -1;
	CallBackparm callback_parm = {
		.args = NULL,
		.base = NULL,
		.result = NULL,
	};
	printf("https_get url:%s\n", url);
	int length = body_len;
	struct evhttp_uri* uri = evhttp_uri_parse(url);
	if (!uri)
	{
		fprintf(stderr, "parse url failed!\n");
		return 1;
	}
 
	struct event_base* base = event_base_new();
	if (!base)
	{
		fprintf(stderr, "create event base failed!\n");
		return 1;
	}
	//event_reinit(base);
	// struct evdns_base* dnsbase = evdns_base_new(base, 1);
	// if (!dnsbase)
	// {
	// 	fprintf(stderr, "create dns base failed!\n");
	// }
	//assert(dnsbase);
	callback_parm.base = base;
	//callback_parm.args = args;
	callback_parm.result = result;
	
	const char* host = evhttp_uri_get_host(uri);
	if (!host)
	{
		fprintf(stderr, "parse host failed!\n");
		return 1;
	}
	printf("https_get url:%s\n", host);

	int port = evhttp_uri_get_port(uri);
	if (port < 0) port = 80;
 
	const char* request_url = url;
	const char* path = evhttp_uri_get_path(uri);
	// if (path == NULL || strlen(path) == 0)
	// {
	// 	request_url = "/";
	// }
 
	printf("url:%s host:%s port:%d path=%s\n", url, host, port,path);

	struct evhttp_request* request = evhttp_request_new(call_back,base);   //RemoteReadCallback
	if(NULL == request)
	{
		printf("https_get NULL == request\n");
		event_base_free(base);
		return -1;
	}
	//evhttp_request_set_chunked_cb(request, call_back);


//add ssl
    // SSL *ssl;
	// SSL_CTX *ctx;
	// SSL_library_init();
	// SSL_load_error_strings();
	// ctx = SSL_CTX_new(SSLv23_client_method());
	// if(ctx == NULL)
	// {
	// 	return -1;
	// }
	// ssl = SSL_new(ctx);
    // sockfd = ConnectTarget((char*)host,port);
    // if(sockfd < 0) 
    // {
    //     printf("sockfd < 0\n");
    //     return -1;
    // }
    // struct  bufferevent* evEvent = bufferevent_openssl_socket_new(
    //             base,sockfd,ssl,BUFFEREVENT_SSL_CONNECTING,
    //             BEV_OPT_CLOSE_ON_FREE | BEV_OPT_DEFER_CALLBACKS | BEV_OPT_THREADSAFE);
        
    // if (evEvent == NULL)
	// {
	// 	return -1;
	// }
	// printf("lbc----------------get complated 22222\n");
    // bufferevent_openssl_set_allow_dirty_shutdown(evEvent, 1);

    struct evhttp_connection* evConn = evhttp_connection_base_new(base,NULL,host, port);//evhttp_connection_base_bufferevent_new(base, NULL, evEvent, host, port);
	if(evConn == NULL)
	{
		return -1;
	}
	printf("lbc----------------get complated 22222\n");
	 //evhttp_connection_set_base(evConn, base);
	 printf("lbc----------------get complated 44444\n");
	evhttp_connection_set_retries(evConn, 2);
	evhttp_connection_set_timeout(evConn, 5);

	
	//struct evhttp_request* request = evhttp_request_new(RemoteReadCallback,base);
//  evhttp_request_set_header_cb(request, ReadHeaderDoneCallback);
 	// evhttp_request_set_chunked_cb(request, call_back);
// 	evhttp_request_set_error_cb(request, RemoteRequestErrorCallback);
//end add ssl
	// struct evhttp_connection* connection = evhttp_connection_base_new(base, dnsbase, host, port);
	// if (!connection)
	// {
	// 	fprintf(stderr, "create evhttp connection failed!\n");
	// 	return 1;
	// }

	//evhttp_connection_set_closecb(evConn, RemoteConnectionCloseCallback, base);
	
	evhttp_add_header(evhttp_request_get_output_headers(request), "Host", host);
	evhttp_add_header(evhttp_request_get_output_headers(request),"Connection","keep-alive");
	evhttp_add_header(evhttp_request_get_output_headers(request),"User-Agent","Mozilla/5.0 (Windows NT 10.0; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/80.0.3987.87 Safari/537.36 SE 2.X MetaSr 1.0");
	// if(NULL != content_type)
	// 	evhttp_add_header(evhttp_request_get_output_headers(request),"Content-Type",content_type);
	// if(NULL != cookie)
	// {
	// 	evhttp_add_header(evhttp_request_get_output_headers(request),"Cookie",cookie);
	// }
	printf("lbc----------------get complated   11111\n");
	//struct evbuffer *buffer = NULL;
	// if(NULL != body && 0 < length)
	// {
		
	//  	buffer = evhttp_request_get_output_buffer(request);
	// 	evbuffer_add(buffer, body, length);
	// }
	printf("lbc----------------get complated   3333,request_url=%s\n",request_url);
	evhttp_make_request(evConn, request, EVHTTP_REQ_GET, path);
	printf("lbc----------------get complated   4444\n");
	event_base_dispatch(base);
	printf("lbc----------------get complated\n");
	// if(0 < sockfd)
	// 	close(sockfd);
	// else
	// 	printf("lbc----------sockfd is <0\n");
	printf("lbc----------evhttp_connection_free 111\n");
	evhttp_connection_free(evConn);
	printf("lbc----------evhttp_connection_free 2222\n");
	if(NULL != base)
		event_base_free(base);
	else
		printf("lbc----------base is NULL\n");
	
	// SSL_shutdown(ssl);
	// SSL_free(ssl);
	// SSL_CTX_free(ctx);
	return 0;
}

// int main(int argc, char** argv)
// {
// 	char testdata[] = "test";
// 	int count = 0;
// 	while(1)
// 	{
// 		// https_get("https://fpdk.jiangsu.chinatax.gov.cn:81/",testdata,strlen(testdata));
// 		https_post("https://127.0.0.1:8421/login",testdata,strlen(testdata),&RemoteReadCallback);
// 		printf("testing................count = %d\n",count++);
// 		usleep(1000*1000);
// 	}
// 	return 0;
// }