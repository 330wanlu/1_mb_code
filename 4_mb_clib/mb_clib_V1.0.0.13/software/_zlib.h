#ifndef _BASE64_DEFINE
	
	#define _BASE64_DEFINE
/*==============================================================inClude File==========================================================================================================*/
	#include "mb_typedef.h"		
	#include <string.h>
	#include <zlib.h>
	#include <stdlib.h>
	#include <stdio.h>
	#include "_base64.h"
	//#include "../_opt.h"
	#ifdef _zlib_c
		#ifdef DEBUG
			#define out(s,arg...)       				//log_out("all.c",s,##arg) 
		#else
			#define out(s,arg...)						//log_out("all.c",s,##arg) 
		#endif	
		#define zlib_extern				
	#else
		#define zlib_extern							extern
	#endif
/*==============================================================================================================================================================================================*/		

int asc_compress_base64(const unsigned char *src, int src_len, unsigned char *dest_data, int *dest_len);
int uncompress_asc_base64(const unsigned char *src, int src_len, unsigned char *dest, int *count);
int asc_uncompress(const unsigned char *src, int src_len, unsigned char *dest_data, int *dest_len);
int asc_compress(const unsigned char *src, int src_len, unsigned char *dest_data, int *dest_len);
#endif