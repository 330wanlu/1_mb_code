#define _CODE_S_C
#include "_code_s.h"
/*======================================码制转换=============================================*/
int juge_usb_dev(char *name)
{   int i,len,k;
    len=strlen(name);
    for(i=0,k=0;i<len;i++)
    {   if((name[i]>='0')&&(name[i]<='9'))
        {   k++;
            continue;
        }
        if((name[i]=='-')||(name[i]=='.'))
            continue;
        return -1;
    }
    if(k==0)
        return -1;
    return 0;
}




uint32 ismoney(uint8 *p)                    	
{	uint8 i;
	uint32 integer,point=0;
  	int len;
	len = strlen((char *)p);
	if(len>10)
		return 0;
	for(i=0;i<len;i++)
	{	if(p[i]=='.')
	 	{	p[i]=0;
	       	break;
	 	}
	}
	integer = asc2algorism((char *)p, i);
	if(i!=len)
	{	point=asc2algorism((char *)p+i+1,len-i-1);
	  	if((len-i-1)==1)
	    	point=point*10;
		p[i]='.';
	}
	if(point>99)
		return 0;
	return integer*100+point; 		
}


uint32 ismoney2(uint8 *p)                    	
{	uint8 i;
	uint32 integer,point=0;
  	int len;
	len=strlen((const char *)p);
	if(len>10)
		return 0;
	for(i=0;i<len;i++)
	{	if(p[i]=='.')
	 	{	p[i]=0;
	       	break;
	 	}
	}
	integer=asc2algorism((char *)p,i); 
	for(;i<len;i++)
	{
		if(p[i]=='.')
	 	{	p[i]=0;
	       	break;
	 	}
	}
	point=asc2algorism((char *)p+i+1,len-i-1);
	//point=point*10;
	/*if(i!=len)
	{	point=asc2algorism(p+i+1,len-i-1);
	  	if((len-i-1)==1)
	    	point=point*10;
		p[i]='.';
	}*/
	if(point>99)
		return 0;
	return integer*100+point; 		
}


int get_dir(const char *dir,char *r_dir)
{   char *pbuf,*pbuf_e;
    int n;
    for(pbuf=(char *)dir,n=0,pbuf_e=NULL;pbuf!=NULL;n++)
    {   pbuf=strstr(pbuf,"/");
        if(pbuf==NULL)
            break;
        pbuf_e=pbuf;
        pbuf++;
    }
    if(pbuf_e==NULL)
    {   memcpy(r_dir,"/",1);
        return 0;
    }
    memcpy(r_dir,dir,(pbuf_e-dir));
    memcpy(r_dir+strlen(r_dir),"/",1);
    return 0;   
}




int get_dir_file(const char *dir,char *r_name)
{   char *pbuf,*pbuf_e;
    int n;
    for(pbuf=(char *)dir,n=0,pbuf_e=NULL;pbuf!=NULL;n++)
    {   pbuf=strstr(pbuf,"/");
        if(pbuf==NULL)
            break;
        pbuf_e=pbuf;
        pbuf++;
    }
    if(pbuf_e==NULL)
    {   out("字符中不存在[/]\n");
        memcpy(r_name,dir,strlen(r_name));
        return 0;
    }
    pbuf_e++;
    memcpy(r_name,pbuf_e,strlen(pbuf_e));
    return 0;
}


int get_feild(const char *scr,const char *f,char *r_f)
{   char *pbuf,*e_buf;
    pbuf=strstr(scr,f);
    if(pbuf==NULL)
        return -1;
    e_buf=strstr(pbuf,"\n");
    if(e_buf==NULL)
        return -2;
    pbuf+=(strlen(f)+1);
    memcpy(r_f,pbuf,e_buf-pbuf);
    return 0;
}


int get_feild_d(const char *scr,const char *f,char *r_f)
{   char *pbuf,*e_buf;
    pbuf=strstr(scr,f);
    if(pbuf==NULL)
        return -1;
    e_buf=strstr(pbuf,"]");
    if(e_buf==NULL)
        return -2;
    pbuf+=(strlen(f)+2);
    memcpy(r_f,pbuf,e_buf-pbuf);
    return 0;
}

void trans_asc_xml(char *buf,uint8 type)
{	char *pbuf,*pbuf_end;
	int32 len;
	for(pbuf=buf;;)
	{	pbuf=strchr(pbuf,'<');
		if(pbuf==NULL)
			return ;
		pbuf_end=strchr(pbuf,'>');
		if(pbuf_end==NULL)
			return ;
		pbuf++;
		len=(uint32 )pbuf_end-(uint32 )pbuf;
		if(type==TRANS_ASC_XML_T_SMALL)
			trans_small(pbuf,len);
		else if(type==TRANS_ASC_XML_T_BIG)
			trans_big(pbuf,len);
		continue;
	}
}

void asc2hexbuf(char *asc,uint8 *hex_buf,uint32 asc_len)
{	uint32 i,j;
	for(i=0,j=0;i<asc_len;i+=2,j++)
	{	hex_buf[j]=((asc2hex(asc[i])<<4)&0xf0);
		hex_buf[j]+=(asc2hex(asc[i+1])&0x0f);	
	}	
}

uint8 asc2hex(uint8 asc)
{ 	if((asc>='0')&&(asc<='9'))
		return 	asc-'0';
	else if((asc>='A')&&(asc<='F'))
		return asc-'A'+0x0a;
	else if((asc>='a')&&(asc<='f'))
		return asc-'a'+0x0a;
	return 0;
}





uint32 asc2algorism(char *asc,uint8 asc_len)
{	uint32 a_value,hex;
	uint8 i,j,k;
	for(i=1,a_value=0,hex=1;i<=asc_len;i++)
	{	k=asc[asc_len-i]-'0';
		for(j=0;j<k;j++)
			a_value+=hex;
		hex=(hex<<3)+(hex<<1);
	}
	return a_value;
}

uint8 algorism2asc(uint32 algorism,uint8 *asc)
{	uint8 i,j,flag,h;
	uint32 hex=1000000000;
	if(algorism==0)
	{	asc[0]='0';
		return 1;
	}
	for(i=0,flag=0,h=0;i<10;i++)
	{	j=algorism/hex;
		if(!((j==0)&&(flag==0))) 
		{	asc[h++]=j+'0';
			flag=1;
			if(algorism>=hex)
         		algorism=algorism-hex*j;
		} 
	   	hex=hex/10;
	}
	return h;
}


void asc2bcd(uint8 *asc,uint8 *bcd,uint8 size_asc)	 //ASC
{	uint8 i;
	for(i=0;i<size_asc;i+=2)
		bcd[i/2]=((asc[i]-'0')<<4)+((asc[i+1]-'0')&0x0f);
}

void bcd2asc(uint8 *bcd,uint8 *asc,uint8 size_bcd)
{	uint8 i;
	for(i=0;i<size_bcd;i++)
	{	asc[i*2]=((bcd[i]>>4)&0x0f)+'0';
		asc[i*2+1]=(bcd[i]&0x0f)+'0';
	}
}




uint32 ipaddress_asc(uint32 ip,uint16 port,char *asc)
{	uint32 len;
	len=ip2ip_asc(ip,asc);
	asc[len++]=':';
	len+=algorism2asc((uint32 )port,(uint8 *)asc+len);
	return len;
}

void ipaddress_asc_hex(const char *asc,uint32 size_asc,uint32 *ip,uint16 *port)
{	char ip_asc[100];
	char port_asc[20];
	uint8 fix;
	uint32 i,ip_n,port_n;
	memset(ip_asc,0,sizeof(ip_asc));
	memset(port_asc,0,sizeof(port_asc));
	for(i=0,fix=0,ip_n=0,port_n=0;i<size_asc;i++)
	{	if(asc[i]==':')
		{	fix=1;
			continue;
		}
		if(fix==0)
		{	ip_asc[ip_n++]=asc[i];
			if(ip_n==sizeof(ip_asc))
			{	*ip=0;
				*port=0;
				return ;
			}
		}
		else
		{	port_asc[port_n++]=asc[i];
			if(port_n==sizeof(port_asc))
			{	*ip=0;
				*port=0;
				return ;
			}
		}
	}
	
	*ip=ip_asc2ip(ip_asc,ip_n);
	*port=(uint16 )asc2algorism(port_asc,port_n);
	return ;
}

uint32 ip_asc2ip(const char *ip_asc,uint32 size_ip_asc)
{	uint32 ip,hex;
	char ip_f[10];
	uint32 i,k,f;
	out("ip=%s\n",ip_asc);
	for(i=0,f=0,ip=0;i<size_ip_asc;i++)
	{	memset(ip_f,0,sizeof(ip_f));
		for(k=0;i<size_ip_asc;i++)
		{	if(ip_asc[i]=='.')
				break;
			ip_f[k++]=ip_asc[i];
			if(k==sizeof(ip_f))
				return 0;
		}
		out("ip_f:%s\n",ip_f);
		hex=asc2algorism(ip_f,k);
		hex=(hex<<((3-f)*8));
		ip+=hex;
		f++;
		if(f==4)
			break;
	}
	out("ip=%8x\n",ip);
	return ip;
}

uint32 ip2ip_asc(uint32 ip,char *ip_asc)
{	uint32 hex;
	uint32 i,j;
	for(i=0,j=0;i<sizeof(ip);i++)
	{	hex=((ip>>((3-i)*8))&0xff);
		j+=algorism2asc(hex,(uint8 *)ip_asc+j);
		if(i!=(sizeof(ip)-1))
			ip_asc[j++]='.';
	}
	return j;
}



/*====================================字符串合法性判断==============================*/
int32 j_asc1bcd(char *asc,uint16 asc_len)
{	uint16 i;
	for(i=0;i<asc_len;i++)
		{	if((asc[i]>'9')||(asc[i]<'0'))
				return -1;
		}
	return 0;
}

int32 j_asc1hex(char *asc,int32 asc_len)
{	int32 i;
	if(asc_len%2)
		return -1;
	for(i=0;i<asc_len;i++)
	{	if((asc[i]>='0')&&(asc[i]<='9'))
			continue;
		if((asc[i]>='a')&&(asc[i]<='f'))
			continue;
		if((asc[i]>='A')&&(asc[i]<='F'))
			continue;
		return -1;
	}
	return 0;	
}

int32 j_ip_port_asc(char *asc,int32 asc_len)
{	int32 i;
	uint8 ip_fix,port_fix;
	for(i=0,ip_fix=0,port_fix=0;i<asc_len;i++)
	{	if(!((asc[i]=='.')||(asc[i]==':')||((asc[i]>='0')&&(asc[i]<='9'))))
			return -1;
		if(asc[i]=='.')
			ip_fix++;
		if(asc[i]==':')
			port_fix++;
	}
	if(ip_fix!=3)
		return -1;
	if(port_fix!=1)
		return -1;
	return 0;
}




int time_asc(uint8 *timer_bcd,uint8 *timer_asc,uint8 time_number)
{   bcd2asc(timer_bcd,timer_asc,2);
    if(time_number==0)           //只能到年
        return 0;
    timer_asc[4]='-';
    bcd2asc(timer_bcd+2,timer_asc+5,1);
    if(time_number==1)           //到月
        return 0;
    timer_asc[7]='-';
    bcd2asc(timer_bcd+3,timer_asc+8,1);
    if(time_number==2)           //到日
        return 0;
    timer_asc[10]=' ';
    bcd2asc(timer_bcd+4,timer_asc+11,1);
    if(time_number==3)           //到小时
        return 0;
    timer_asc[13]=':';
    bcd2asc(timer_bcd+5,timer_asc+14,1);
    if(time_number==4)           //到分
        return 0;
    timer_asc[16]=':';
    bcd2asc(timer_bcd+6,timer_asc+17,1);
    return 0;                   //到秒
}


/*======================================查找扩展函数=================================*/
char *strstr_n(const char *str,const char *search,uint8 n)
{	uint8 i;
	char *pbuf;
	for(i=0,pbuf=(char *)str;i<n;i++,pbuf++)
		{	pbuf=strstr((const char *)pbuf,search);
			if(pbuf==NULL)
				return NULL;
		}
	return --pbuf;
}





static void trans_small(char *buf,int32 len)
{	int32 i;
	for(i=0;i<len;i++)
	{	if((buf[i]>='A')&&(buf[i]<='Z'))
		{	buf[i]+=('a'-'A');
		}
	}	
}

static void trans_big(char *buf,int32 len)
{	int32 i;
	for(i=0;i<len;i++)
	{	if((buf[i]>='a')&&(buf[i]<='z'))
		{	buf[i]-=('a'-'A');
		}
	}	
}


/*======================================================================================*/
void cls_space(char *buf,uint32 len)
{	uint32 i,j;
	for(i=0,j=0;i<len;i++)
	{	if(buf[i]!=' ')
		{	buf[j++]=buf[i];
		}
	}
	memset(buf+j,0,len-j);
}

void cls_no_asc(char *buf,uint32 len)
{	uint32 i,j;
	for(i=0,j=0;i<len;i++)
	{	if(buf[i]<' ')
			continue;
		buf[j++]=buf[i];
	}
	memset(buf+j,0,len-j);

}

void cls_character(char *buf, uint32 len,char del_char)
{
	uint32 i, j;
	for (i = 0, j = 0; i<len; i++)
	{
		if (buf[i] == del_char)
			continue;
		buf[j++] = buf[i];
	}
	memset(buf + j, 0, len - j);
}
int delete_str(char *strRes, char *from)
{
	int flag = 0;
	int len;
	char *Ptr = NULL;
	char *middle_flag;

	middle_flag = strstr(strRes, from);
	if (middle_flag == NULL)
	{
		return flag;
	}

	len = strlen(middle_flag);
	Ptr = (char *)malloc(len * sizeof(char));
	if (NULL == Ptr)
	{
		return flag;
	}
	strcpy(Ptr, middle_flag + (strlen(from)));
	if (middle_flag != NULL)
	{
		/* code */
		*middle_flag = '\0';
		//strcat(strRes, to);
		strcat(strRes, Ptr);
		free(Ptr);
		flag = 1;
	}
	return flag;
}

int str_replace(char *strRes, char *from, char *to)
{
	int flag = 0;
	int len;
	char *Ptr = NULL;
	char *middle_flag;

	for (;;)
	{
		middle_flag = strstr(strRes, from);
		if (middle_flag == NULL)
		{
			return flag;
		}

		len = strlen(middle_flag);
		Ptr = (char *)malloc(len * sizeof(char));
		if (NULL == Ptr)
		{
			return flag;
		}
		strcpy(Ptr, middle_flag + (strlen(from)));
		if (middle_flag != NULL)
		{
			/* code */
			*middle_flag = '\0';
			strcat(strRes, to);
			strcat(strRes, Ptr);
			free(Ptr);
			flag = 1;
		}
	}
	return flag;
}

int str_replace_once(char *strRes, char *from, char *to)
{
	int flag = 0;
	int len;
	char *Ptr = NULL;
	char *middle_flag;

		middle_flag = strstr(strRes, from);
		if (middle_flag == NULL)
		{
			return flag;
		}

		len = strlen(middle_flag);
		Ptr = (char *)malloc(len * sizeof(char));
		if (NULL == Ptr)
		{
			return flag;
		}
		strcpy(Ptr, middle_flag + (strlen(from)));
		if (middle_flag != NULL)
		{
			/* code */
			*middle_flag = '\0';
			strcat(strRes, to);
			strcat(strRes, Ptr);
			free(Ptr);
			flag = 1;
		}
	return flag;
}

int StrFind(char *strRes, char *find)
{	char *middle_flag;
	middle_flag = strstr(strRes, find);
	if (middle_flag == NULL)
	{
		return -1;
	}

	return 0;
}