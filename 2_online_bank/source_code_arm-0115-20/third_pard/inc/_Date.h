#ifndef _DATE_DEFINE
    #define _DATE_DEFINE
/*=========================================Include File===========================================*/
	#include "mb_typedef.h"
/*=========================================Define Data============================================*/
    #ifdef _date_c
        #define e_date
    #else
        #define e_date                      extern
    #endif
    
    /*++++++++++++++++++++++++++++++++++++++访问类型++++++++++++++++++++++++++++++++++++++++*/
		   	#define De_Type_Date_Date                       1                             //类型为日期
			#define De_Type_Date_Hour                       De_Type_Date_Date+1           //类型为精确到小时
			#define De_Type_Date_Minute                     De_Type_Date_Hour+1           //类型精确到分
			#define De_Type_Date_Second                     De_Type_Date_Minute+1         //类型精确到秒
/*===========================================错误代码=============================================*/
	#define De_Suc                                          0
    #define De_Suc_Date										De_Suc
	
	#define De_Erro_Date_Hex2BCD                    		1                  		  //十六进制转BCD错误
	#define De_Erro_Date_JugeDate                   		De_Erro_Date_Hex2BCD+1       		  //日期错误
	#define De_Erro_Date_JugeHour                   		De_Erro_Date_JugeDate+1               //小时错误
	#define De_Erro_Date_JugeMinute                 		De_Erro_Date_JugeHour+1               //分错误 
	#define De_Erro_Date_JugeSecond                 		De_Erro_Date_JugeMinute+1             //秒错误 



	
/*==========================================全局函数===============================================*/
	e_date uint16 next_year(uint8 *Buf);
	e_date uint16 next_moth(uint8 *Buf);
	e_date uint16 back_year(uint8 *Buf);
	e_date uint16 back_moth(uint8 *Buf);	
	e_date uint16 next_hour(uint8 *Buf);
	e_date uint16 next_min(uint8 *Buf);
	e_date uint16 back_hour(uint8 *Buf);
	e_date uint16 juge_clock(uint8 *Buf,uint8 Type);
	e_date uint16 next_clock(uint8 *Buf,uint8 Type);
	e_date uint16 back_clock(uint8 *Buf,uint8 Type);
	e_date uint16 getweek(uint8 *NonceDate);
	e_date uint16 back_date(uint8 *Buf);
	e_date uint16 next_date(uint8 *Buf);
	e_date int next_month_asc(char *buf);
	e_date int next_month_last_date_asc(char *buf);
	e_date int next_month_frist_date_asc(char *buf);
	#ifdef	_date_c
		//static uint16 jugedate(uint8 *Buf);
		//static uint16 jugehour(uint8 Byte);
		//static uint16 jugeminute(uint8 Byte);
		//static uint16 Jugesecond(uint8 Byte);
		//static uint16 getdate(uint32 NumAll,uint8 *Date);
		//static void nextdate(uint8 *Buf);
		//static void nexthour(uint8 *Buf);
		//static void nextminute(uint8 *Buf);
		//static void nextsecond(uint8 *Buf);
		
		//static void backdate(uint8 *Buf);
		//static void backhour(uint8 *Buf);
		//static void backminute(uint8 *Buf);
		//static void backsecond(uint8 *Buf);
		
		static void Bcd2Hex(uint8 *BcdBuf,uint8 *HexBuf,uint8 SizeofBcdBuf );
		static uint8 Hex2Bcd(uint8 *HexBuf,uint8 *BcdBuf,uint8 SizeofHexBuf);
		//static uint8 JugeBuf(uint8 *Buf1,uint8 *Buf2,uint16 SizeofBuf);
   #endif
/*==================================================================================================================================================*/

 

#endif
