#define _zlib_c
#include "_zlib.h"

int asc_compress(const unsigned char *src, int src_len, unsigned char *dest_data, int *dest_len)
{	int result;
	//int i;
	unsigned char *dest;
	unsigned long count = 2 * (src_len + 12);////destLen表示 dest 缓冲区的大小，destLen > (sourceLen + 12)*100.1%
	unsigned long tmp_len;
	//out("压缩该数据：\n");
	tmp_len = compressBound(src_len);	

	if((dest = (unsigned char*)malloc(sizeof(unsigned char) * tmp_len)) == NULL)  
    {  
        out("No enough memory!\n");  
        return -1;  
    } 
	result = compress(dest,&count,src,src_len);
	if(result != Z_OK)  
    {  
        printf("Compress failed ERRCODE =%d!\n",result);  
		free(dest); 
        return -1;  
    }  
	if(count<1)
	{
		out("解压后的数据长度错误\n");
		free(dest);
		return -1;
	}
	//out("zlib压缩后数据长度：%d\n",count);
	//out("压缩后数据：");
	/*for(i=0;i<count;i++)
	{
		printf("%02x ",dest[i]);
	}
	printf("\n");*/
	memcpy(dest_data,dest,count);
	*dest_len = count;
	free(dest);
	return 0;
}


int asc_uncompress(const unsigned char *src, int src_len, unsigned char *dest_data, int *dest_len)
{	int result;
	//int i;
	unsigned char *dest;
	unsigned long count = 9048000;
	//out("解压该数据：\n");	
	if ((dest = (unsigned char*)malloc(sizeof(unsigned char)* 9048000)) == NULL)
    {  
        out("No enough memory!\n");  
        return -1;  
    } 
	result = uncompress(dest,&count,src,src_len);
	if(result != Z_OK)  
    {  
        printf("Uncompress failed ERRCODE =%d!\n",result);  
		free(dest); 
		if (result == Z_MEM_ERROR)
		{
			printf("Z_MEM_ERROR\n");
		}
		else if (result == Z_BUF_ERROR)
		{
			printf("Z_BUF_ERROR\n");
		}
		else if (result == Z_DATA_ERROR)
		{
			printf("Z_DATA_ERROR\n");
		}
        return -1;  
    }  
	if(count<1)
	{
		out("解压后的数据长度错误\n");
		free(dest);
		return -1;
	}
	/*out("zlib解压后数据长度：%d\n",count);
	out("解压后数据：");
	int i = 0;
	for(i=0;i<count;i++)
	{
		printf("%02x ",dest[i]);
	}
	printf("\n");
	printf("%s\n",dest);
	*/
	//sprintf(dest_data,"%s",dest);
	memcpy(dest_data, dest, count);
	*dest_len = count;
	free(dest);
	return 0;
}


int asc_compress_base64(const unsigned char *src, int src_len, unsigned char *dest_data, int *dest_len)
{
	int result;
	//int i;
	unsigned char *dest;
	unsigned long count = 2 * (src_len + 12);////destLen表示 dest 缓冲区的大小，destLen > (sourceLen + 12)*100.1%
	unsigned long tmp_len;
	//out("压缩该数据：\n");
	tmp_len = compressBound(src_len);

	if ((dest = (unsigned char*)malloc(sizeof(unsigned char)* tmp_len)) == NULL)
	{
		out("No enough memory!\n");
		return -1;
	}
	result = compress(dest, &count, src, src_len);
	if (result != Z_OK)
	{
		printf("Compress failed ERRCODE =%d!\n", result);
		free(dest);
		return -1;
	}
	if (count<1)
	{
		out("解压后的数据长度错误\n");
		free(dest);
		return -1;
	}
	//out("zlib压缩后数据长度：%d\n",count);
	//out("压缩后数据：");
	/*for(i=0;i<count;i++)
	{
	printf("%02x ",dest[i]);
	}
	printf("\n");*/
	encode((char *)dest, count, (char *)dest_data);

	//memcpy(dest_data, dest, count);
	*dest_len = strlen((char *)dest_data);
	free(dest);
	return 0;
}

int uncompress_asc_base64(const unsigned char *src, int src_len, unsigned char *dest, int *count)
{
	int result;
	//char *tmp;
	int base_tmp_len = 10 * 1024 * 1024;
	unsigned char *base_tmp = calloc(1, base_tmp_len);
	if (base_tmp == NULL)
	{
		return -1;
	}
	uint32 len;
	int i,j;

	if (src_len > base_tmp_len)
	{
		printf("src data > 4k\n");
		free(base_tmp);
		return -2;
	}

	memset(base_tmp, 0, base_tmp_len);
	for(i=0,j=0;i<=src_len;i=i+4,j=j+3)
	{
		decode((char *)src+i,4,(char *)base_tmp+j);
	}
	len=(src_len/4)*3;


	result = asc_uncompress(base_tmp, len, dest, count);
	
	free(base_tmp);
	return result;
}

