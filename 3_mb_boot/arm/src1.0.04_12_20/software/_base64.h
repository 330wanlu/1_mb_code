#ifndef _BASE64_DFINE

#define _BASE64_DFINE
/*==============================================================inClude File==========================================================================================================*/
#include <string.h>
//#include "../file/_file.h"
#ifdef _base64_c
#define base64_extern				
#else
#define base64_extern							extern
#endif
/*==============================================================================================================================================================================================*/
base64_extern unsigned short base64_enc(unsigned char *dest, const unsigned char *src, signed short count);
base64_extern unsigned short base64_dec(unsigned char* dest, unsigned char* src, signed short count);
base64_extern	void decode(char *src, int src_len, char *dst);
base64_extern	void encode(char *src, int src_len, char *dst);

#endif