#define _date_c						//编译函数体   
#include "_Date.h"

#if 0
#define De_Date_Mod							(60*60*24)
#define De_Hour_Mod							(60*60)
#define De_Minute_Mod						(60)
void Pack_Up_DateFormat(uint32 Sec,uint8 *Buf)
{	st_Date_Format	CN_st_Date_Format;
	uint16 DataLen;
	Get_Sec2DateFormat(Sec,&CN_st_Date_Format);
	DataLen=0;
	if(CN_st_Date_Format.Date!=0)
	{	Algorism2Asc(CN_st_Date_Format.Date,Buf);
		Copy_Asc(Buf,"天");	
	 	DataLen=Strsize(Buf);
	}
	if(CN_st_Date_Format.Hour!=0)
	{	Algorism2Asc(CN_st_Date_Format.Hour,Buf+DataLen);
	   	Copy_Asc(Buf,"时");
		DataLen=Strsize(Buf);	
	}
	if(CN_st_Date_Format.Minute!=0)
	{	Algorism2Asc(CN_st_Date_Format.Minute,Buf+DataLen);
	   	Copy_Asc(Buf,"分");
		DataLen=Strsize(Buf);
	}
	Algorism2Asc(CN_st_Date_Format.Second,Buf+DataLen);
   	Copy_Asc(Buf,"秒");
}

	
void Get_Sec2DateFormat(uint32 Sec,st_Date_Format	*ptst_Date_Format)
{	uint32 Hex;
	ClsArray(ptst_Date_Format,sizeof(st_Date_Format));
	Hex=(Sec/De_Date_Mod);
	if(Hex!=0)
		ptst_Date_Format->Date=Hex;
	Sec=(Sec%De_Date_Mod);
	Hex=(Sec/De_Hour_Mod);
	if(Hex!=0)
		ptst_Date_Format->Hour=Hex;	
	Sec=(Sec%De_Hour_Mod);
	Hex=(Sec/De_Minute_Mod);
	if(Hex!=0)
		ptst_Date_Format->Minute=Hex;
	Sec=(Sec%De_Minute_Mod);
	ptst_Date_Format->Second=Sec;					
	return ;					
}
uint16 Timer_Explain(uint8 *AscBuf,uint8 *HexBuf)
{	MemSpace(AscBuf);		
   	Asc2Bcd(AscBuf,HexBuf,4);
	Asc2Bcd(AscBuf+5,HexBuf+2,2);
	Asc2Bcd(AscBuf+8,HexBuf+3,2);
	Asc2Bcd(AscBuf+10,HexBuf+4,2);
	Asc2Bcd(AscBuf+13,HexBuf+5,2);
	Asc2Bcd(AscBuf+16,HexBuf+6,2);
	if(Juge_Clock(HexBuf,De_Type_Date_Second)==0)
		return De_Suc_Date;
	return 1;
}

uint16 Timer_Explain_Date(uint8 *AscBuf,uint8 *HexBuf)
{	MemSpace(AscBuf);		
   	Asc2Bcd(AscBuf,HexBuf,4);
	Asc2Bcd(AscBuf+5,HexBuf+2,2);
	Asc2Bcd(AscBuf+8,HexBuf+3,2);
	if(Juge_Clock(HexBuf,De_Type_Date_Date)==0)
		return De_Suc_Date;
	return 1;
}

uint16 Get_ClockNumber(uint8 *Start,uint8 *AoutDate)
{	uint8 Buf[4];
	uint16 Cuit;
	if((JugeDate(Start)!=De_Suc_Date)||(JugeDate(AoutDate)!=De_Suc_Date)||(CompareVa(AoutDate,Start,4,De_AscNumber_Va_Big)==De_AscNumber_Va_Flase))
		return 0;
	for(Cuit=1,MoveByte(Buf,Start,4);CompareBuf(Buf,AoutDate,4)!=0;NextDate(Buf),Cuit++);
	return Cuit;
}

							
uint32 Timer2Number(uint8 *Timer)
{	uint8 Buf[3];
	uint16 Year;
	if((Year=Juge_Clock(Timer,De_Type_Date_Second))!=De_Suc_Date)
		return Year;
	Bcd2Hex(Timer+4,Buf,sizeof(Buf));
	return Date2Number(Timer)+Moth2Number(Timer)+Buf[0]*3600+Buf[1]*60+Buf[2];
}

uint32 Date2Number(uint8 *Timer)
{	uint32 Num;
	uint16 Year,j;
	uint8 Buf[2];
	Bcd2Hex(Timer,Buf,sizeof(Buf));
	Year=Buf[0]*100+Buf[1];
	if(Year<2000)
		return De_Erro_Date_JugeDate;
	Year=Year-2000;
	for(j=0,Num=0;j<Year;j++)
	{	if(j&0x03)
			Num+=365;
		else
			Num+=366;
	}
	return Num*86400;		
}
uint32 Moth2Number(uint8 *Timer)
{	uint16 i;
	uint8 DateTimer[4];
	MoveByte(DateTimer,Timer,sizeof(DateTimer));
	DateTimer[2]=0x01;DateTimer[3]=0x01;
	for(i=0;JugeBuf(DateTimer,Timer,sizeof(DateTimer))!=0;i++,Next_Clock(DateTimer,De_Type_Date_Date));
	return i*86400;
}

uint16 GetTimer(uint32 NumAll,uint8 *Date)
{	uint32 Year,Timer;
	uint8 Buf[3];
	Year=NumAll/86400;
	Timer=NumAll%86400;
	Buf[0]=Timer/3600;
	Timer=Timer%3600;
	Buf[1]=Timer/60;
	Timer=Timer%60;
	Buf[2]=Timer;
	GetDate(Year,Date);
	Hex2Bcd(Buf,Date+4,sizeof(Buf));
   	Date[7]=GetWeek(Date);
	return De_Suc_Date;
}




uint16 GetDate(uint32 NumAll,uint8 *Date)	   //从2000.1.1日开始
{	uint8 Buf[4];
	uint16 Year,Num,Dat;
	uint32 i;
	for(i=0,Year=2000,Num=0;i<NumAll;i+=Num,Year++)
	{	Num=365;
		if((Year&0x03)==0)	
			Num++;
		if((i+Num)>NumAll)
			break;
	}
	Dat=NumAll-i;
	for(i=0;i<12;i++)
	{	if((i==3)||(i==5)||(i==8)||(i==10))
			Num=30;
		else if(i==1)
		{	if(Year&0x03)
				Num=28;
			else
				Num=29;
		}
		else
			Num=31;
		if(Dat<Num)
			break;
		Dat-=Num;					
	}
	Buf[0]=Year/100;Buf[1]=Year%100;
	Buf[2]=i+1;Buf[3]=Dat+1;	
	Hex2Bcd(Buf,Date,sizeof(Buf));
	return De_Suc_Date;
}
#endif

uint16 next_hour(uint8 *Buf)
{	uint8 BufHex;
	Bcd2Hex(Buf,&BufHex,sizeof(BufHex));	
	BufHex++;
	if(BufHex>23)
		return 1;
	Hex2Bcd(&BufHex,Buf,sizeof(BufHex));
	return 0;
}

uint16 next_min(uint8 *Buf)
{	uint8 BufHex;
	Bcd2Hex(Buf,&BufHex,sizeof(BufHex));	
	BufHex++;
	if(BufHex>59)
		return 1;
	Hex2Bcd(&BufHex,Buf,sizeof(BufHex));
	return 0;
}

uint16 back_hour(uint8 *Buf)
{	uint8 BufHex;
	Bcd2Hex(Buf,&BufHex,sizeof(BufHex)); 
	if(BufHex==0)
		return 1;
	BufHex--;
	Hex2Bcd(&BufHex,Buf,sizeof(BufHex));
	return 0;
}



uint16 next_year(uint8 *Buf)
{	uint8 BufHex;
	Bcd2Hex(Buf+1,&BufHex,sizeof(BufHex));	
	BufHex++;
	if(BufHex>99)
		return 1;
	Hex2Bcd(&BufHex,Buf+1,sizeof(BufHex));
	return 0;
}

uint16 next_moth(uint8 *Buf)
{	uint8 BufHex;
	Bcd2Hex(Buf,&BufHex,sizeof(BufHex)); 
	BufHex++;
	if(BufHex>12)
		return 1;
	Hex2Bcd(&BufHex,Buf,sizeof(BufHex));
	return 0;
}

uint16 back_year(uint8 *Buf)
{	uint8 BufHex;
	Bcd2Hex(Buf+1,&BufHex,sizeof(BufHex));
	if(BufHex==0)
		return 1;
	BufHex--;
	Hex2Bcd(&BufHex,Buf+1,sizeof(BufHex));		
	return 0;
}

uint16 back_moth(uint8 *Buf)
{	uint8 BufHex;
	Bcd2Hex(Buf,&BufHex,sizeof(BufHex)); 
	BufHex--;
	if(BufHex==0)
		return 1;
	Hex2Bcd(&BufHex,Buf,sizeof(BufHex));
	return 0;
}

uint16 back_date(uint8 *Buf)
{
	uint8 BufHex[4];
	uint16 Year;
	Bcd2Hex(Buf, BufHex, sizeof(BufHex));
	BufHex[3]--;
	if (BufHex[3] == 0x00)
	{
		BufHex[2]--;
		if (BufHex[2] == 0)
		{
			if (BufHex[1] == 0)
			{
				BufHex[0]--;
				BufHex[1] = 0x99;
			}
			else
				BufHex[1]--;
			BufHex[2] = 12;
			BufHex[3] = 31;
		}
		else
		{
			if ((BufHex[2] == 4) || (BufHex[2] == 6) || (BufHex[2] == 9) || (BufHex[2] == 11))
				BufHex[3] = 30;
			else if ((BufHex[2] == 1) || (BufHex[2] == 3) || (BufHex[2] == 5) || (BufHex[2] == 7)
				|| (BufHex[2] == 8) || (BufHex[2] == 12) || (BufHex[2] == 10))
				BufHex[3] = 31;
			else if (BufHex[2] == 2)
			{
				Year = BufHex[0] * 100 + BufHex[1];
				if ((Year & 0x03) != 0)
					BufHex[3] = 28;
				else
					BufHex[3] = 29;
			}
		}
	}
	Hex2Bcd(BufHex, Buf, sizeof(BufHex));
	return 0;
}

uint16 next_date(uint8 *Buf)
{
	uint8 BufHex[4];
	uint16 Year;
	Bcd2Hex(Buf, BufHex, sizeof(BufHex));
	BufHex[3] += 1;//day+1;
	if ((BufHex[3] == 31) && ((BufHex[2] == 4) || (BufHex[2] == 6) || (BufHex[2] == 9) || (BufHex[2] == 11)))
	{
		BufHex[2] += 1;
		BufHex[3] = 1;
	}                                                                 //上次是月底
	else if ((BufHex[3] == 32) && ((BufHex[2] == 1) || (BufHex[2] == 3) || (BufHex[2] == 5) || (BufHex[2] == 7)
		|| (BufHex[2] == 8) || (BufHex[2] == 12) || (BufHex[2] == 10)))
	{
		BufHex[2] += 1;
		if (BufHex[2] == 13)                                        //超过12月
		{
			BufHex[2] = 1;                                     //month
			BufHex[1] += 1;                                    //year			
		}
		BufHex[3] = 1;
	}
	else if ((BufHex[3] == 29) && (BufHex[2] == 2))
	{
		Year = BufHex[0] * 100 + BufHex[1];
		if ((Year & 0x03) != 0)                                        /*判断是不是闰年,不是闰年*/
		{
			BufHex[2] = 3;
			BufHex[3] = 1;
		}
	}
	else if ((BufHex[3] == 30) && (BufHex[2] == 2))
	{
		BufHex[2] = 3;
		BufHex[3] = 1;
	}
	Hex2Bcd(BufHex, Buf, sizeof(BufHex));
	return 0;
}


int next_month_asc(char *buf)
{
	char date_tmp[100] = { 0 };
	int year;
	int month;
	int date;

	sscanf(buf, "%04d%02d%02d", &year, &month, &date);
	month++;
	if (month > 12)
	{
		month = 1;
		year += 1;
	}
	sprintf(date_tmp, "%04d%02d%02d", year, month, date);
	memcpy(buf, date_tmp, 8);
	return 0;
}

int next_month_last_date_asc(char *buf)
{
	char date_tmp[100] = { 0 };
	int year;
	int month;
	int date;

	sscanf(buf, "%04d%02d%02d", &year, &month, &date);
	month++;
	if (month > 12)
	{
		month = 1;
		year += 1;
	}
	if ((month == 4) || (month == 6) || (month == 9) || (month == 11))
		date = 30;
	else if ((month == 1) || (month == 3) || (month == 5) || (month == 7) || (month == 8) || (month == 12) || (month == 10))
		date = 31;
	else if (month == 2)
	{
		if ((year & 03) != 0)
			date = 28;
		else
			date = 29;
	}
	sprintf(date_tmp, "%04d%02d%02d", year, month, date);
	memcpy(buf, date_tmp, 8);
	return 0;
}

int next_month_frist_date_asc(char *buf)
{
	char date_tmp[100] = { 0 };
	int year;
	int month;
	int date;

	sscanf(buf, "%04d%02d%02d", &year, &month, &date);
	month++;
	if (month > 12)
	{
		month = 1;
		year += 1;
	}
	date = 1;
	sprintf(date_tmp, "%04d%02d%02d", year, month, date);
	memcpy(buf, date_tmp, 8);
	return 0;
}


uint32 clock_GB18240(uint8 *Buf)
{	uint8 HexBuf[7];
	uint32 Hex;
	Bcd2Hex(Buf,HexBuf,sizeof(HexBuf));
	Hex=(uint32 )HexBuf[1]*1048576+(uint32 )HexBuf[2]*65536+(uint32 )HexBuf[3]*2048+(uint32 )HexBuf[4]*64+(uint32 )HexBuf[5];
	return Hex;
}






//uint16 juge_clock(uint8 *Buf,uint8 Type)    	//时间判断函数                          
//{     uint16 Erro;
//      if(JugeHexBcd(Buf,3+Type)!=0)
//	  		return De_Erro_Date_JugeDate;
//	  if((Erro=jugedate(Buf))!=De_Suc_Date)
//            return Erro;
//      if(Type==De_Type_Date_Date)
//            return De_Suc_Date;
//      if((Erro=jugehour(Buf[4]))!=De_Suc_Date)
//            return Erro;
//      if(Type==De_Type_Date_Hour)
//            return De_Suc_Date;
//      if((Erro=jugeminute(Buf[5]))!=De_Suc_Date)
//            return Erro;
//      if(Type==De_Type_Date_Minute)
//            return De_Suc_Date;    
//      if((Erro=jugesecond(Buf[6]))!=De_Suc_Date)
//            return Erro;
//      return De_Suc_Date;           
//}

//uint16 next_clock(uint8 *Buf,uint8 Type) //下一个时间
//{     uint16 Erro;
//      if((Erro=juge_clock(Buf,Type))!=De_Suc_Date)
//            return Erro;
//      if(Type==De_Type_Date_Date)
//            nextdate(Buf);
//      else if(Type==De_Type_Date_Hour)
//            nexthour(Buf);      
//      else if(Type==De_Type_Date_Minute)
//            nextminute(Buf);   
//      else
//            nextsecond(Buf);           
//      return De_Suc_Date;
//}

//uint16 back_clock(uint8 *Buf,uint8 Type) //上一个时间
//{     uint16 Erro;
//      if((Erro=juge_clock(Buf,Type))!=De_Suc_Date)
//            return Erro;
//      if(Type==De_Type_Date_Date)
//            backdate(Buf);
//      else if(Type==De_Type_Date_Hour)
//            backhour(Buf);      
//      else if(Type==De_Type_Date_Minute)
//            backminute(Buf);   
//      else
//            backsecond(Buf);           
//      return De_Suc_Date;
//}

//uint16 get_week(uint8 *NonceDate)
//{	uint32 DateNumber;
//	if(JugeDate(NonceDate)!=De_Suc_Date)
//		return 0;
//	DateNumber=((Date2Number(NonceDate)+Moth2Number(NonceDate))/86400);
//	DateNumber=DateNumber%7;
//	DateNumber=DateNumber+6;
//	DateNumber=(DateNumber%7);
//	return DateNumber;
//}





/*=======================================以下为静态函数==================================================*/
//static uint16 jugedate(uint8 *Buf)
//{     uint8 BufHex[4];
//      uint16 Year;
//      Bcd2Hex(Buf,BufHex,sizeof(BufHex));
//      Year=BufHex[0]*100+BufHex[1];
//      if(Year&0x03)
//            Year=1;
//      else
//            Year=0;        
//      if((BufHex[3]>31)||(BufHex[0]!=20)||(BufHex[2]>12)||(BufHex[2]==0)||(BufHex[3]==0))    //不符合习惯
//              return De_Erro_Date_JugeDate;        
//      if(((BufHex[2]==4)||(BufHex[2]==6)||(BufHex[2]==9)||(BufHex[2]==11))&&(BufHex[3]>30))
//              return De_Erro_Date_JugeDate;
//      if(((BufHex[2]==1)||(BufHex[2]==3)||(BufHex[2]==5)||(BufHex[2]==7)||(BufHex[2]==8)||
//      		(BufHex[2]==12)||(BufHex[2]==10))&&(BufHex[3]>31))
//              return De_Erro_Date_JugeDate;
//      if((BufHex[2]==2)&&(BufHex[3]>28)&&(Year==1))
//              return De_Erro_Date_JugeDate; 
//      if((BufHex[2]==2)&&(BufHex[3]>29)&&(Year==0))
//              return De_Erro_Date_JugeDate;     
//      return De_Suc_Date; 
//}


//static uint16 jugehour(uint8 Byte)
//{     
//      if(Byte>0x23)
//            return De_Erro_Date_JugeHour;
//      return De_Suc_Date;      
//}
//
//static uint16 jugeminute(uint8 Byte)
//{     
//      if(Byte>0x59)
//            return De_Erro_Date_JugeMinute;
//      return De_Suc_Date;
//}

//static uint16 jugesecond(uint8 Byte)
//{     
//      if(Byte>0x59)
//            return De_Erro_Date_JugeSecond;
//      return De_Suc_Date;
//}






//static void nexthour(uint8 *Buf)
//{     uint8 ByteV;
//      Bcd2Hex(Buf+4,&ByteV,sizeof(ByteV));
//      ByteV++;
//      if(ByteV>23)
//      {     NextDate(Buf);     
//            Buf[4]=0x00;
//      }
//      else
//      {     Hex2Bcd(&ByteV,Buf+4,sizeof(ByteV));
//      }
//}

//static void nextminute(uint8 *Buf)
//{     uint8 ByteV;
//      Bcd2Hex(Buf+5,&ByteV,sizeof(ByteV));
//      ByteV++;    
//      if(ByteV>59)
//      {     NextHour(Buf);
//            Buf[5]=00;
//      }
//      else
//      {     Hex2Bcd(&ByteV,Buf+5,sizeof(ByteV));
//      }
//}

//static void nextsecond(uint8 *Buf)
//{     uint8 ByteV;
//      Bcd2Hex(Buf+6,&ByteV,sizeof(ByteV));
//      ByteV++;
//      if(ByteV>59)
//      {     NextMinute(Buf);
//            Buf[6]=00;
//      }
//      else
//      {     Hex2Bcd(&ByteV,Buf+6,sizeof(ByteV));
//      }
//}


//static void backdate(uint8 *Buf)
//{     uint8 BufHex[4];
//      uint16 Year;
//      Bcd2Hex(Buf,BufHex,sizeof(BufHex));
//      BufHex[3]--;
//      if(BufHex[3]==0x00)
//      {     BufHex[2]--;  
//            if(BufHex[2]==0)
//            {     if(BufHex[1]==0)
//                  {     BufHex[0]--;    
//                        BufHex[1]=0x99;
//                  }
//                  else
//                        BufHex[1]--;                 
//                  BufHex[2]=12;
//                  BufHex[3]=31;
//            }
//            else
//            {     if((BufHex[2]==4)||(BufHex[2]==6)||(BufHex[2]==9)||(BufHex[2]==11))       
//                        BufHex[3]=30;
//                  else if((BufHex[2]==1)||(BufHex[2]==3)||(BufHex[2]==5)||(BufHex[2]==7)
//                  ||(BufHex[2]==8)||(BufHex[2]==12)||(BufHex[2]==10))	                              
//                        BufHex[3]=31;
//                  else if(BufHex[2]==2)
//                  {     Year=BufHex[0]*100+BufHex[1];
//                        if((Year&0x03)!=0)
//                              BufHex[3]=28;
//                        else 
//                              BufHex[3]=29;              
//                  }      
//            }         
//      }
//      Hex2Bcd(BufHex,Buf,sizeof(BufHex));
//}

//static void backhour(uint8 *Buf)
//{     uint8 ByteV;
//      Bcd2Hex(Buf+4,&ByteV,sizeof(ByteV));
//      if(ByteV==0)
//      {     BackDate(Buf);
//            Buf[4]=0x23;
//      }
//      else
//      {     ByteV--;
//            Hex2Bcd(&ByteV,Buf+4,sizeof(ByteV));
//      }      
//}

//static void backminute(uint8 *Buf)
//{     uint8 ByteV;
//      Bcd2Hex(Buf+5,&ByteV,sizeof(ByteV));
//      if(ByteV==0)
//      {     BackHour(Buf);
//            Buf[5]=0x59;
//      }
//      else
//      {     ByteV--;
//            Hex2Bcd(&ByteV,Buf+5,sizeof(ByteV));
//      }  
//}

//static void backsecond(uint8 *Buf)
//{     uint8 ByteV;
//      Bcd2Hex(Buf+6,&ByteV,sizeof(ByteV));
//      if(ByteV==0)
//      {     BackMinute(Buf);
//            Buf[6]=0x59;
//      }
//      else
//      {     ByteV--;
//            Hex2Bcd(&ByteV,Buf+6,sizeof(ByteV));
//      } 
//} 

static void Bcd2Hex(uint8 *BcdBuf,uint8 *HexBuf,uint8 SizeofBcdBuf)
{     uint8 i,Byte;
      for(i=0;i<SizeofBcdBuf;i++)
      {     Byte=(BcdBuf[i]>>4)*10+(BcdBuf[i]&0x0f);
            HexBuf[i]=Byte;
      }       
}

static uint8 Hex2Bcd(uint8 *HexBuf,uint8 *BcdBuf,uint8 SizeofHexBuf)
{     uint8 i;
      for(i=0;i<SizeofHexBuf;i++)
      {     if(HexBuf[i]>99)
                  return De_Erro_Date_Hex2BCD;
            BcdBuf[i]=(HexBuf[i]%10)+((HexBuf[i]/10)<<4);  
      }
      return De_Suc_Date;
}
//static uint8 JugeBuf(uint8 *Buf1,uint8 *Buf2,uint16 SizeofBuf)
//{	uint16 i;
//	for(i=0;i<SizeofBuf;i++)
//	{	if(Buf1[i]!=Buf2[i])
//			return 1;
//	}
//	return De_Suc_Date;
//}





