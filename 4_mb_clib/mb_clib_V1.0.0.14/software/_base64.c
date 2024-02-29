#define _base64_c
#include "_base64.h"
uint16 base64_enc(uint8 *dest, const uint8 *src, int16 count)
{ 	const uint8 Base64_EnCoding[65] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    int16 len = 0; 
    uint8 *pt = (uint8 *)src;
    
    /* count小于0，则自行计算src的长度*/
    if( count < 0 )
    {
        while( *pt++ )
		{ count++; }
        pt = (uint8 *)src;
    }
    if( !count ) return 0;
 
    while( count > 0 )
    {
        *dest++ = Base64_EnCoding[ ( pt[0] >> 2 ) & 0x3f];
        if( count > 2 )
        {
            *dest++ = Base64_EnCoding[((pt[0] & 3) << 4) | (pt[1] >> 4)];
            *dest++ = Base64_EnCoding[((pt[1] & 0xF) << 2) | (pt[2] >> 6)];
            *dest++ = Base64_EnCoding[ pt[2] & 0x3F];
        }
        else
        {
            switch( count )
            {
            case 1:
                *dest++ = Base64_EnCoding[(pt[0] & 3) << 4 ];
                *dest++ = '=';
                *dest++ = '=';
                break;
            case 2: 
                *dest++ = Base64_EnCoding[((pt[0] & 3) << 4) | (pt[1] >> 4)]; 
                *dest++ = Base64_EnCoding[((pt[1] & 0x0F) << 2) | (pt[2] >> 6)]; 
                *dest++ = '='; 
            } 
        } 
        pt += 3; 
        count -= 3; 
        len += 4; 
    } 
    *dest = 0; 
    return len; 
} 

int16 base64_dec(uint8* dest,uint8* src, int16 count) 
{
    uint8 ucs[4];
    uint8 *pt = (unsigned char *)src;
    int16 len = 0 , nfag = 0 , i = 0;
//    /*count小于0，则自行计算src的长度*/
//    if( count < 0 )
//    {
//        while( *pt++ ){ count++; }
//        pt = (uint8 *)src;
//    }
    if( !count ) 
		return 0;
    while( count > 0 )
    {
        nfag = 0;
        for( i=0 ; i<4 ; i++ )
        {
            if (*pt >= 'A' && *pt <= 'Z') 
                ucs[i] = *pt - 'A'; 
            else if (*pt >= 'a' && *pt <= 'z') 
                ucs[i] = *pt - 'a' + 26; 
            else if (*pt >= '0' && *pt <= '9') 
                ucs[i] = *pt - '0' + 52; 
            else{
                switch (*pt)
                { 
                case '+': 
                    ucs[i] = 62;
                    break;
                case '/': 
                    ucs[i] = 63;
                    break;
                case '=': /* base64 padding */ 
                    ucs[i] = 0; 
                    break;
                case '\t':
                case '\r':
                case '\n':
                case ' ':
                    nfag++;
                    i--;
                    break;
                case '\0': /*遇到文尾,直接返回上次字符 buf 的长度*/ 
                    *dest = 0;
                    return len;
                default:  /* 遇到其它非法字符,直接返回 */
                    *dest = 0;
                    return -1;
                }
            }
            pt++;
        }
        *dest++ = (ucs[0] << 2) | (ucs[1] >> 4);
        *dest++ = (ucs[1] << 4) | (ucs[2] >> 2);
        *dest++ = (ucs[2] << 6) | (ucs[3]);
        count -= nfag + 4;
        len += 3;
    }
    *dest = 0;
    return len;
}



void encode(char *src, int src_len, char *dst)
{
        int i = 0, j = 0;
        char base64_map[65] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
 
        for (; i < src_len - src_len % 3; i += 3) {
                dst[j++] = base64_map[(src[i] >> 2) & 0x3F];
                dst[j++] = base64_map[((src[i] << 4) & 0x30) + ((src[i + 1] >> 4) & 0xF)];
                dst[j++] = base64_map[((src[i + 1] << 2) & 0x3C) + ((src[i + 2] >> 6) & 0x3)];
                dst[j++] = base64_map[src[i + 2] & 0x3F];
        }
 
        if (src_len % 3 == 1) {
                 dst[j++] = base64_map[(src[i] >> 2) & 0x3F];
                 dst[j++] = base64_map[(src[i] << 4) & 0x30];
                 dst[j++] = '=';
                 dst[j++] = '=';
        }
        else if (src_len % 3 == 2) {
                dst[j++] = base64_map[(src[i] >> 2) & 0x3F];
                dst[j++] = base64_map[((src[i] << 4) & 0x30) + ((src[i + 1] >> 4) & 0xF)];
                dst[j++] = base64_map[(src[i + 1] << 2) & 0x3C];
                dst[j++] = '=';
        }
 
        dst[j] = '\0';
}
 
void decode(char *src, int src_len, char *dst)
{
        int i = 0, j = 0;
        char base64_decode_map[256] = {
             255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
             255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
             255, 255, 255, 62, 255, 255, 255, 63, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 255, 255,
             255, 0, 255, 255, 255, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14,
             15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 255, 255, 255, 255, 255, 255, 26, 27, 28,
             29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48,
             49, 50, 51, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
             255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
             255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
             255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
             255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
             255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
             255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255};
 
        for (; i < src_len; i += 4) 
		{
			dst[j++] = (((base64_decode_map[(int)src[i]]) << 2) | ((base64_decode_map[(int)src[i + 1]]) >> 4));
			dst[j++] = (((base64_decode_map[(int)src[i + 1]]) << 4) | ((base64_decode_map[(int)src[i + 2]]) >> 2));
			dst[j++] = (((base64_decode_map[(int)src[i + 2]]) << 6) | (base64_decode_map[(int)src[i + 3]]));
        }
 
        dst[j] = '\0';
}


















const char *chlist = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
 
int encode_string(char* str, unsigned int length, char* stat) 
{
    char s[103];
    int i,j;
    unsigned temp;
    if(length <= 0) 
		return 1;
    if(length > 100) 
		return 2;
    str[length] = '\0';
    strcpy(s,str);
    while(strlen(s) % 3) strcat(s,"=");
    for(i = 0,j = 0; s[i]; i += 3,j += 4) 
	{
        temp = s[i];
        temp = (temp << 8) + s[i + 1];
        temp = (temp << 8) + s[i + 2];
        stat[j + 3] = chlist[temp & 0X3F];
        temp >>= 6;
        stat[j + 2] = chlist[temp & 0X3F];
        temp >>= 6;
        stat[j + 1] = chlist[temp & 0X3F];
        temp >>= 6;
        stat[j + 0] = chlist[temp & 0X3F];
    }
    stat[j] = '\0';
    return 0;
}
 
int Index(char ch) 
{
    int i;
    for(i = 0; chlist[i]; ++i) 
	{
        if(chlist[i] == ch)
            return i;
    }
    return -1;
}
 
void decode_string(char *s, char *t) 
{
    unsigned temp;
    int i,j,k,len = strlen(s);
    if(len % 4) 
	{
        printf("?????\n");
        return ;
    }
    for(i = 0,j = 0; i <= len; i += 4,j += 3) 
	{
        temp = 0;
        for(k = 0; k < 4; ++k)
            temp = (temp << 6) + Index(s[i + k]);
        for(k = 2; k >= 0; --k) 
		{
            t[j + k] = temp & 0XFF;
            temp >>= 8;
        }
    }
    t[j + k] = '\0';
}

int calc_decode_length(const char *b64input)
{
	int len = strlen(b64input);
	int padding = 0;

	if (b64input[len - 1] == '=' && b64input[len - 2] == '=')  // last two chars are =
		padding = 2;
	else if (b64input[len - 1] == '=')  // last char is =
		padding = 1;

	return (int)len * 0.75 - padding;
}