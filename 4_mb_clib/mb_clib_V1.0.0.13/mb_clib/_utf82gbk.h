#ifndef _UTF82GBK_DEFINE
	#define _UTF82GBK_DEFINE
	/*===============================include file============================================*/
	#include "mb_typedef.h"
	#include <assert.h>
	#ifdef _UFT82GBK_C
		#include "utf8.h"
	#endif
	/*=================================Á¢¼´Êý===============================================*/
	#ifdef _UFT82GBK_C
		#define _utf8gbk_e					
	#else
		#define _utf8gbk_e									extern

	#endif
	
	#ifdef _UFT82GBK_C
		#define MAX_UNI_INDEX									23940
		#define De_Uni_Offset									95761
	#endif
	/*=================================º¯Êý=================================================*/
	_utf8gbk_e	uint32 utf82gbk(uint8 *strutf8,uint8 *strgbk);
	_utf8gbk_e	uint32 gbk2utf8(uint8 *GBK,uint8 *utf);
	_utf8gbk_e  uint32 unicode_to_utf8(uint8 *unicode,uint8 *utf8);
	_utf8gbk_e	uint32 unicode_to_gbk(uint8 *unicode,uint8 *gbk);	
	_utf8gbk_e	uint32 unicode2gbk(uint8 *unicode, uint8 *gbk);
	_utf8gbk_e	uint32 unicode2utf8(uint8 *unicode, uint8 *utf8);
	#ifdef _UFT82GBK_C
		static uint16 BinarySearchStr(uint16 gb,uint16 *uni);
		static uint16 BinarySearchUni(uint16 uni,uint16 *gb);
		static uint16 utf82unicode(uint8 *utf8char, uint16* unicode, uint16 *len, uint16 leaveLen);
		static uint8 u2utf8(uint32 uni,uint8 *utf);
		static  int enc_unicode_to_utf8_one(unsigned long unic, unsigned char *pOutput, int outSize) ;
	#endif
	
	
	
	
#endif