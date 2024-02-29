#define _algorithm_e
#include "_algorithm.h"
uint16 crc_8005(uint8 *s,uint16 len,uint16 value)              //16位的CRC校验       
{	uint16 acc,i;
	acc=value;   
	while (len--)                              
	{	acc = acc ^ (*s++ << 8);                
		for (i = 0; i++ < 8;)                   
		{	if (acc & 0x8000)                 
				acc = (acc << 1) ^ 0x8005;     
			else
				acc <<= 1;                     
		}
	}
	return  acc;                   
}


uint16 crc_8005_int(uint8 *s,int len,uint16 value)              //16位的CRC校验       
{	uint16 acc;
    int i;
	acc=value;   
	while (len--)                              
	{	acc = acc ^ (*s++ << 8);                
		for (i = 0; i++ < 8;)                   
		{	if (acc & 0x8000)                 
				acc = (acc << 1) ^ 0x8005;     
			else
				acc <<= 1;                     
		}
	}
	return  acc;                   
}