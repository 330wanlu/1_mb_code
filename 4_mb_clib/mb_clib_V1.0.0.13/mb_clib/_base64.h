#ifndef _BASE64_DFINE
	
	#define _BASE64_DFINE
/*==============================================================inClude File==========================================================================================================*/
	#include "mb_typedef.h"
	#ifdef _base64_c
		#define base64_extern				
	#else
		#define base64_extern							extern
	#endif
/*==============================================================================================================================================================================================*/		
	base64_extern uint16 base64_enc(uint8 *dest, const uint8 *src, int16 count);
	base64_extern int16 base64_dec(uint8* dest,uint8* src, int16 count);
	base64_extern	void decode(char *src, int src_len, char *dst);
	base64_extern	void encode(char *src, int src_len, char *dst);
	base64_extern	int calc_decode_length(const char *b64input);
#endif