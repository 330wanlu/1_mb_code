#include "../inc/http.h"


int netGet(char* url, char* params, int timeout, char *result, int *result_len) 
{	char timeout_str[10];
	char *tmp;  
    ghttp_request *request = NULL;  
    request = ghttp_request_new();      
    if(params!=NULL&&strlen(params)>0)  
    {         
        //strcpy(tmp,url);  
        //if(strchr(tmp,'?') == NULL)//url不存在
        //{     strcat(tmp,"?");            
        //}  
        //strcat(tmp,params) ;    
		printf("%s\n", url);
        //ghttp_set_uri(request,tmp);        
    }
	else
	{	ghttp_set_uri(request,url);  
    }   
    ghttp_set_type(request,ghttp_type_get); //get方法
    ghttp_set_header(request,http_hdr_Connection,"close");        
    sprintf(timeout_str,"%d",timeout);  
    ghttp_set_header(request,http_hdr_Timeout,timeout_str);       
    ghttp_prepare(request);  
    ghttp_process(request);  
	tmp = ghttp_get_body(request);  
	//printf("netget data tmp %s\n",tmp);
	
	//sprintf(*result,"%s",tmp);
    *result_len = ghttp_get_body_len(request);  
	memcpy(result,tmp,*result_len);
    ghttp_request_destroy(request);  
    return 0;  
}  

int netPost(char* uri, char* params, int timeout, char *result, int *result_len) 
{  
    //char szVal[1024]; 
	char timeout_str[10]; 	
	char *tmp;  
    ghttp_request *request = NULL;  
    ghttp_status status;  
    int len;  
    //printf("netPost:%s\n", params); //test  
    request = ghttp_request_new();  
    if (ghttp_set_uri(request, uri) == -1)
	{	//http_out("ghttp_set_uri == -1\n");	
        return -1;  
	}
    if (ghttp_set_type(request, ghttp_type_post) == -1) //post  
	{	//http_out("ghttp_set_type == -1\n");	
        return -1;  
	}
    ghttp_set_header(request, http_hdr_Content_Type,"application/x-www-form-urlencoded");           
    sprintf(timeout_str,"%d",timeout);  
    ghttp_set_header(request, http_hdr_Timeout, timeout_str);  
    len = strlen(params);  
    ghttp_set_body(request, params,len); //  
    ghttp_prepare(request);  
    status = ghttp_process(request);  
    if(status == ghttp_error)  
	{	//http_out("status == ghttp_error\n");
        return -1;  
	}
    tmp = ghttp_get_body(request); //test  
    *result_len=ghttp_get_body_len(request);  
	memcpy(result,tmp,*result_len);
    ghttp_clean(request);  
    return 0;  
}
