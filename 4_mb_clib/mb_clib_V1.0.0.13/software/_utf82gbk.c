#define _UFT82GBK_C
#include "_utf82gbk.h"
uint32 utf82gbk(uint8 *strutf8,uint8 *strgbk)
{	uint16 unicode = 0, gbk,lenTmp;
	uint32 strUtf8Len; 
	uint32 indexUtf8 = 0, indexGbk = 0;
	strUtf8Len=strlen((char *)strutf8);
	while (strUtf8Len > indexUtf8)
	{	unicode = 0;
		lenTmp = 0;
		if (utf82unicode(strutf8 + indexUtf8, &unicode, &lenTmp, strUtf8Len - indexUtf8) != 0)
		{	indexUtf8++;
			continue;
		}
		if(lenTmp==1)
		{	strgbk[indexGbk++]=unicode;
			indexUtf8 += lenTmp;
			continue;
		}
		BinarySearchUni(unicode,&gbk);
		strgbk[indexGbk++] = gbk >> 8;
		strgbk[indexGbk++] = gbk & 0xFF;
		indexUtf8 += lenTmp;
	}
	return indexGbk;	
}

uint32 gbk2utf8(uint8 *GBK,uint8 *utf)
{	uint32 i,DataLen,j;
	uint16 gb,uni;
	uint8 Len;
	DataLen=strlen((char *)GBK);
	for(i=0,j=0;i<DataLen;i++,j+=Len)
	{	if(GBK[i]<0x80)
		{	utf[j]=GBK[i];
			Len=1;
			continue;
		}
		gb=GBK[i]*256+GBK[i+1];
		BinarySearchStr(gb,&uni);
		Len=u2utf8((uint32 )uni,utf+j);
		i++;
	}
	return j;					
}

uint32 unicode2utf8(uint8 *unicode,uint8 *utf8)
{	int c_len;
	int i;
	int u_len;
	unsigned long u_tmp; 
	unsigned char utf[5];
	u_len = strlen((const char *)unicode);
	c_len = u_len/2;
	if((u_len < 2)||u_len%2!=0)
	{
		printf("unicode len error unlen = %d\n",u_len);
		return -1;
	}
	for(i=0;i<c_len;i++)
	{
		u_tmp = ((unicode[2*i+1]<<8)&0x0000ff00)+((unicode[2*i])&0x000000ff);
		//printf("u_tmp = %08x\n",u_tmp);
		memset(utf,0,sizeof(utf));
		enc_unicode_to_utf8_one(u_tmp,utf,6);
		memcpy(utf8+3*i,utf,3);
	}	
	return  3*c_len;
}

uint32 unicode2gbk(uint8 *unicode,uint8 *gbk)
{	int c_len;
	int i;
	unsigned long u_tmp; 
	unsigned char utf[5];
	int u_len;
	u_len = strlen((const char *)unicode);
	c_len = u_len/2;
	unsigned char utf8[3*c_len+100];
	memset(utf8,0,sizeof(utf8));
	if((u_len < 2)||u_len%2!=0)
	{
		printf("unicode len error unlen = %d\n",u_len);
		return -1;
	}
	for(i=0;i<c_len;i++)
	{
		u_tmp = ((unicode[2*i+1]<<8)&0x0000ff00)+((unicode[2*i])&0x000000ff);
		//printf("u_tmp = %08x\n",u_tmp);
		memset(utf,0,sizeof(utf));
		enc_unicode_to_utf8_one(u_tmp,utf,6);
		memcpy(utf8+3*i,utf,3);
	};
	printf("utf8 %s\n",utf8);
	utf82gbk(utf8,gbk);
	return u_len;
}
/*=====================================静态文件=============================================*/
	static uint16 BinarySearchStr(uint16 gb,uint16 *uni)
	{ 	uint8 Buf[4];
		uint16 Temp;
		int middle;
		int left = 0;
		int right = MAX_UNI_INDEX - 1;
		while(left <= right)
		{	middle = (left+right)/2;
			middle = middle*4;
			memcpy(Buf,utf8+middle,4);
			Temp = Buf[0]*256+Buf[1];
			if (gb == Temp) 
			{	*uni = Buf[2]*256+Buf[3]; 
				return 1;
			}
			if (gb > Temp) 
				left = middle/4 + 1;
			else 
				right = middle/4 - 1;
		}
		return 0;
	}
	
	static uint16 BinarySearchUni(uint16 uni,uint16 *gb)
	{ 	uint8 Buf[4];
		uint16 Temp;
		uint32 left = 0;
		uint32 middle;
		uint32 right = MAX_UNI_INDEX - 1;
		while(left <= right)
		{	middle = (left+right)/2;
			middle = middle*4;
			memcpy(Buf,utf8+middle+De_Uni_Offset,4);
			Temp = Buf[2]*256+Buf[3];
			if (uni == Temp)
			{	*gb = Buf[0]*256+Buf[1]; 
				return 1;
			}
			if (uni > Temp) 
				left = middle/4 + 1;
			else 
				right = middle/4 - 1;
		}
		return 0;
	}
	
	static uint16 utf82unicode(uint8 *utf8char, uint16* unicode, uint16 *len, uint16 leaveLen)
	{
		uint8 cTmp = *utf8char;
		if ((cTmp & 0xF8)  == 0xF0) /*4位*/
		{
			if ( leaveLen < 4 || 
				  (((*(utf8char + 1)) & 0xC0) != 0x80) || 
				  (((*(utf8char + 2)) & 0xC0) != 0x80) ||
				  (((*(utf8char + 3)) & 0xC0) != 0x80) )
			{
				return 2;/*剩余位数不够 */
			}
			*len = 4;
			*unicode |= (cTmp & 0x07);
			*unicode <<= 6;
			
			cTmp = *(utf8char + 1);
			*unicode |= (cTmp & 0x3F);
			*unicode <<= 6;
			
			cTmp = *(utf8char + 2);
			*unicode |= (cTmp & 0x3F);
			*unicode <<= 6;
			
			cTmp = *(utf8char + 3);
			*unicode |= (cTmp & 0x3F);
		}
		else if ( (cTmp & 0xF0) == 0xE0) /* 3位 */
		{
			if ( leaveLen < 3 || 
				  (((*(utf8char + 1)) & 0xC0) != 0x80) || 
				  (((*(utf8char + 2)) & 0xC0) != 0x80) )
			{
				return 2;/*剩余位数不够 */
			}
			*len = 3;
			*unicode |= (cTmp & 0x0F);
			*unicode <<= 6;
			
			cTmp = *(utf8char + 1);
			*unicode |= (cTmp & 0x3F);
			*unicode <<= 6;
			
			cTmp = *(utf8char + 2);
			*unicode |= (cTmp & 0x3F);
		}
		else if ( (cTmp & 0xE0) == 0xC0) /* 2位 */
		{
			if ( leaveLen < 2 || 
				  (((*(utf8char + 1)) & 0xC0) != 0x80) )
			{
				return 2;/*剩余位数不够 */
			}
			*len = 2;
			*unicode |= (cTmp & 0x1F);
			*unicode <<= 6;
			
			cTmp = *(utf8char + 1);
			*unicode |= (cTmp & 0x3F);
	
		}
		else if ( (cTmp & 0x80) == 0x00) /* 1位 */
		{
			*len = 1;
			*unicode = cTmp;
		}
		else
		{
			return 1; /*第一位不合法*/
		}
		
		return 0;
	}
	
	
	static uint8 u2utf8(uint32 uni,uint8 *utf) 
	{ 	uint32 utf8; 
		if(uni < 0x80) 
		{ 	utf[0]=uni;
			return 1; 
		} 
		if(uni < 0x800) 
		{ 
			utf8 = (0xc0 | (uni >> 6)) << 8 
				| (0x80 | (uni & 0x3f)); 
	 		utf[0]=((utf8>>8)&0xff);
			utf[1]=(utf8&0xff);
			return 2; 
		} 
		if(uni < 0x10000) 
		{ 
			utf8 = (0xe0 | (uni>>12)) << 16 
				| (0x80 | (uni >> 6 & 0x3f)) << 8 
				| (0x80 | (uni & 0x3f)); 
			utf[0]=((utf8>>16)&0xff);
			utf[1]=((utf8>>8)&0xff);
			utf[2]=((utf8)&0xff);
			return 3; 
		} 
		return 4;
	}

	
// #c---  
/***************************************************************************** 
 * 将一个字符的Unicode(UCS-2和UCS-4)编码转换成UTF-8编码. 
 * 
 * 参数: 
 *    unic     字符的Unicode编码值 
 *    pOutput  指向输出的用于存储UTF8编码值的缓冲区的指针 
 *    outsize  pOutput缓冲的大小 
 * 
 * 返回值: 
 *    返回转换后的字符的UTF8编码所占的字节数, 如果出错则返回 0 . 
 * 
 * 注意: 
 *     1. UTF8没有字节序问题, 但是Unicode有字节序要求; 
 *        字节序分为大端(Big Endian)和小端(Little Endian)两种; 
 *        在Intel处理器中采用小端法表示, 在此采用小端法表示. (低地址存低位) 
 *     2. 请保证 pOutput 缓冲区有最少有 6 字节的空间大小! 
 ****************************************************************************/  
int enc_unicode_to_utf8_one(unsigned long unic, unsigned char *pOutput, int outSize)  
{  
	//printf("unicode = %08x \n",unic);
    assert(pOutput != NULL);  
    assert(outSize >= 6);  
  
    if ( unic <= 0x0000007F )  
    {	//printf("test 1\n");
        // * U-00000000 - U-0000007F:  0xxxxxxx  
        *pOutput     = (unic & 0x7F);  
        return 1;  
    }  
    else if ( unic >= 0x00000080 && unic <= 0x000007FF )  
    {	//printf("test 2\n");
        // * U-00000080 - U-000007FF:  110xxxxx 10xxxxxx  
        *(pOutput+1) = (unic & 0x3F) | 0x80;  
        *pOutput     = ((unic >> 6) & 0x1F) | 0xC0;  
        return 2;  
    }  
    else if ( unic >= 0x00000800 && unic <= 0x0000FFFF )  
    {  // printf("test 3\n");
        // * U-00000800 - U-0000FFFF:  1110xxxx 10xxxxxx 10xxxxxx  
        *(pOutput+2) = (unic & 0x3F) | 0x80;  
        *(pOutput+1) = ((unic >>  6) & 0x3F) | 0x80;  
        *pOutput     = ((unic >> 12) & 0x0F) | 0xE0;  
        return 3;  
    }  
    else if ( unic >= 0x00010000 && unic <= 0x001FFFFF )  
    {   //printf("test 4\n");
        // * U-00010000 - U-001FFFFF:  11110xxx 10xxxxxx 10xxxxxx 10xxxxxx  
        *(pOutput+3) = (unic & 0x3F) | 0x80;  
        *(pOutput+2) = ((unic >>  6) & 0x3F) | 0x80;  
        *(pOutput+1) = ((unic >> 12) & 0x3F) | 0x80;  
        *pOutput     = ((unic >> 18) & 0x07) | 0xF0;  
        return 4;  
    }  
    else if ( unic >= 0x00200000 && unic <= 0x03FFFFFF )  
    {	//printf("test 5\n");
        // * U-00200000 - U-03FFFFFF:  111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx  
        *(pOutput+4) = (unic & 0x3F) | 0x80;  
        *(pOutput+3) = ((unic >>  6) & 0x3F) | 0x80;  
        *(pOutput+2) = ((unic >> 12) & 0x3F) | 0x80;  
        *(pOutput+1) = ((unic >> 18) & 0x3F) | 0x80;  
        *pOutput     = ((unic >> 24) & 0x03) | 0xF8;  
        return 5;  
    }  
    else if ( unic >= 0x04000000 && unic <= 0x7FFFFFFF )  
    {	//printf("test 6\n");
        // * U-04000000 - U-7FFFFFFF:  1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx  
        *(pOutput+5) = (unic & 0x3F) | 0x80;  
        *(pOutput+4) = ((unic >>  6) & 0x3F) | 0x80;  
        *(pOutput+3) = ((unic >> 12) & 0x3F) | 0x80;  
        *(pOutput+2) = ((unic >> 18) & 0x3F) | 0x80;  
        *(pOutput+1) = ((unic >> 24) & 0x3F) | 0x80;  
        *pOutput     = ((unic >> 30) & 0x01) | 0xFC;  
        return 6;  
    }  
  //printf("test 7\n");
    return 0;  
}  