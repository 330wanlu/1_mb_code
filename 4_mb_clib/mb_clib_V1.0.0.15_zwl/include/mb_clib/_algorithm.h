#ifndef _ALGORITHM_DEFINE
	#define _ALGORITHM_DEFINE
	#include "mb_typedef.h"
	#ifdef _algorithm_e
		#define e_algorithm
	#else
		#define e_algorithm						extern 
	#endif
	
	/*===============================º¯Êý¶¨Òå===================================*/
	e_algorithm	uint16 crc_8005(uint8 *s,uint16 len,uint16 value);
    e_algorithm uint16 crc_8005_int(uint8 *s,int len,uint16 value);


#endif

