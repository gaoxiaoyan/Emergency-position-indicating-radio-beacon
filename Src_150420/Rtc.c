/******************************************************************************
*											
* Copyright (C) 2014, Sunic-ocean
* All Rights Reserved										
*
* 文 件 名： Rtc.c
* 描       述：

* 创建日期： 2014年11月12日16:15:25
* 作    者： Bob
* 当前版本： V1.00

------------------------------------------------------
*******************************************************************************/

#ifdef MSP430F149
#include "msp430x14x.h"
#else
#include "msp430x54x.h"
#endif

#include "global.h"
#include "DataType.h"
#include "Rtc.h"
#include "I2c_Driver.h"
#include "string.h"

#define RtcBcd2Hex(u8Bcd) ((((u8Bcd & 0xf0) >> 4) % 10) * 10 + ((u8Bcd & 0x0f) % 10))

void SetTimeToRtc(_tsTimeCb * psTime);

/*****************************************************************************
 函 数 名  : Delay
 功能描述  : 在12MHz晶振 1T下 延时时间约等于usTime ms
 输入参数  : USHORT16 usTime
 输出参数  : NONE
 返 回 值  :
*****************************************************************************/
static void Delay(UINT16 usTime)
{
    UINT16 j;
    UINT32 i;

    for (i = usTime; i > 0; i--)
    {
        for(j = 110; j > 0; j--);
    }
}

//字节高地位变换
UINT8 ByteExchange(UINT8 u8Data)
{
  UINT8 u8Temp;

  u8Temp = u8Data;
  u8Temp = ((u8Temp & 0x55) << 1) | ((u8Temp & 0xaa) >> 1);
  u8Temp = ((u8Temp & 0x33) << 2) | ((u8Temp & 0xcc) >> 2);
  u8Temp = ((u8Temp & 0x0f) << 4) | ((u8Temp & 0xf0) >> 4);

  return (u8Temp);
}

/*************************************************************************************************************
 * 函数名称： RtcRest
 * 功能描述： 复位RTC芯片
 * 输  入：
 * 输  出： 					
 ************************************************************************************************************/
void RtcRest(void)
{
  UINT8 u8Cmd=0;

  u8Cmd |= STAT1_RESET;
  RtcWrite(CMD_STAT_1, &u8Cmd, 1);
  Delay(10);
}

#if 1
/*************************************************************************************************************
 * 函数名称： RtcConverHour
 * 功能描述： 12/24进制设置
 * 输  入： bConver 1-- 24进制； 0 -- 12进制
 * 输  出： 					
 ************************************************************************************************************/
void RtcConverHour(BOOL bConver)
{
  UINT8 u8Cmd=0;

  if(bConver == TRUE)
    u8Cmd |= STAT1_CONVER;
  else
    u8Cmd &= ~STAT1_CONVER;

  RtcWrite(CMD_STAT_1, &u8Cmd, 1);
  Delay(10);
  //进行一次读操作，否则不能设置为24显示
  RtcRead(CMD_STAT_1, &u8Cmd, 1);
}
#endif

/*************************************************************************************************************
 * 函数名称： SetInt1FrequencyOut
 * 功能描述： 设置中断1频率输出
 * 输  入：
 * 输  出： 					
 ************************************************************************************************************/
void SetInt1FrequencyOut(void)
{
  UINT8 u8Cmd=0;

  u8Cmd |= STAT2_INT1ME + STAT2_INT1AE + STAT2_INT1FE;
  
  RtcWrite(CMD_STAT_2, &u8Cmd, 1);
  Delay(10);

  u8Cmd |= BIT7;
  RtcWrite(CMD_ALARM_1, &u8Cmd, 1);
  Delay(10);
/*
  u8Cmd[0] = 0;
  RtcWrite(CMD_STAT_2, &u8Cmd[0], 1);
  Delay(10);
  
  u8Cmd[0] = STAT2_INT1AE;
  RtcWrite(CMD_STAT_2, &u8Cmd[0], 1);
  Delay(10);

  u8Cmd[0]  = 0;
  u8Cmd[1] = ByteExchange(0x15) | A1HE | BIT1;
  u8Cmd[2] = ByteExchange(0x12) | A1ME;
  RtcWrite(CMD_ALARM_1, &u8Cmd[0], 3);
  Delay(10);  */
}

//从RTC中获取时间戳
void GetStampFromRtc(void)
{
  _tsTimeCb sTime;
  UINT32 u32Time = 0;

  GetTimeFromRtc(&sTime);

  u32Time += ((UINT32)RtcBcd2Hex(sTime.Hour)) * 3600;
  u32Time += ((UINT32)RtcBcd2Hex(sTime.Minute)) * 60;
  u32Time += (UINT32)RtcBcd2Hex(sTime.Second);

  _DINT();
  g_u32TimeStamp = u32Time;
  _EINT();
}

/*************************************************************************************************************
 * 函数名称： RtcInit
 * 功能描述： RTC初始化
 * 输  入： psTime -- 当前时间结构
 * 输  出： 					
 ************************************************************************************************************/
void RtcInit(void)
{
#if 0
  
  _tsTimeCb sTime;
  
  RtcRest();
  RtcConverHour(1);

  sTime.Year = 0x15;
  sTime.Month = 0x01;
  sTime.Date  = 0x08;
  sTime.Week  = 0x00;
  sTime.Hour  = 0x10;
  sTime.Minute = 0x46;
  sTime.Second = 0x00;

  SetTimeToRtc(&sTime);
  Delay(10);
#endif
//  RtcConverHour(1);

  SetInt1FrequencyOut();
//  GetStampFromRtc();
}

/*************************************************************************************************************
 * 函数名称： CaculateWeek
 * 功能描述： 根据年月日计算星期，使用泰勒算法
 * 输  入：
 * 输  出：
 ************************************************************************************************************/
UINT8 CaculateWeek(UINT8 year, UINT8 month, UINT8 date)
{
  UINT8 u8Temp;

  if(month==1)
  {
    year-=1;
    month=13;
  }
  else
  {
    if(month==2)
    {
      year-=1;
      month=14;
    }
  }

  u8Temp = year+year/4+13*((month+1)/5)+date-36;
  u8Temp %= 7;

  return(u8Temp);
}

/*************************************************************************************************************
 * 函数名称： GetTimeFromRtc
 * 功能描述： 从RTC中读出当前的日期和时间
 * 输  入：
 * 输  出： psTime -- 当前时间结构					
 ************************************************************************************************************/
void GetTimeFromRtc(_tsTimeCb * psTime)
{
  UINT8 au8Temp[8];

  //从RTC获得原始数据
  RtcRead(CMD_YEAR, &au8Temp[0], 7);

  psTime->Year  = ByteExchange(au8Temp[0]);
  psTime->Month = ByteExchange(au8Temp[1]) & 0x1f;
  psTime->Date  = ByteExchange(au8Temp[2]) & 0x3f;

  psTime->Week = CaculateWeek(psTime->Year, psTime->Month, psTime->Date);

  psTime->Hour  = ByteExchange(au8Temp[4]) & 0x3f;
  psTime->Minute = ByteExchange(au8Temp[5]) & 0x7f;
  psTime->Second = ByteExchange(au8Temp[6]) & 0x7f;
}

/*************************************************************************************************************
 * 函数名称： GetMinuteFromRtc
 * 功能描述： 从RTC中读出当前的分时间
 * 输  入：
 * 输  出： 当前的分时间				
 ************************************************************************************************************/
UINT8 GetMinuteFromRtc(void)
{
  UINT8 au8Temp[8];
  UINT8 u8Minute;

  //从RTC获得原始数据
  RtcRead(CMD_YEAR, &au8Temp[0], 7);
  u8Minute = ByteExchange(au8Temp[5]) & 0x7f;
  
  return(RtcBcd2Hex(u8Minute));
}

/*************************************************************************************************************
 * 函数名称： SetTimeToRtc
 * 功能描述： 将时间设置到RTC
 * 输  入： psTime -- 当前时间结构
 * 输  出： 					
 ************************************************************************************************************/
void SetTimeToRtc(_tsTimeCb * psTime)
{
  UINT8 au8Temp[8];

  au8Temp[0] = ByteExchange(psTime->Year & 0xff);
  au8Temp[1] = ByteExchange(psTime->Month & 0x1f);
  au8Temp[2] = ByteExchange(psTime->Date & 0x3f);

  psTime->Week = CaculateWeek(psTime->Year, psTime->Month, psTime->Date);
  au8Temp[3] = ByteExchange(psTime->Week & 0x07);

  au8Temp[4] = ByteExchange(psTime->Hour & 0x7f);
  au8Temp[5] = ByteExchange(psTime->Minute & 0x7f);
  au8Temp[6] = ByteExchange(psTime->Second & 0x7f);
  au8Temp[7] = 0;

  RtcWrite(CMD_YEAR, &au8Temp[0], sizeof(_tsTimeCb));
  Delay(10);
}

