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
#define  RTC_GLOBAL
#include "msp430x54x.h"
#include "global.h"
#include "DataType.h"
#include "Rtc.h"
#include "I2c_Driver.h"
#include "string.h"
#include "SoftTimer.h"

#define RtcBcd2Hex(u8Bcd) ((((u8Bcd & 0xf0) >> 4) % 10) * 10 + ((u8Bcd & 0x0f) % 10))
#define RtcHex2Bcd(u8Hex) ((((u8Hex % 100) / 10) << 4) + ((u8Hex % 100) % 10))

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
  Delay(10);
}
#endif

/*************************************************************************************************************
 * 函数名称： SetRtcInt1Out
 * 功能描述： 设置中断1频率输出, 分中断，边沿触发
 * 输  入：
 * 输  出：
 ************************************************************************************************************/
void SetRtcInt1Out(void)
{
  UINT8 u8Cmd=0;

  RtcWrite(CMD_STAT_2, &u8Cmd, 1);
  Delay(5);

  u8Cmd |= STAT2_INT1ME;

  RtcWrite(CMD_STAT_2, &u8Cmd, 1);
  Delay(5);

/*  u8Cmd |= BIT7;
  RtcWrite(CMD_ALARM_1, &u8Cmd, 1);
  Delay(10);

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

/*************************************************************************************************************
 * 函数名称： RtcInit
 * 功能描述： RTC初始化
 * 输  入： psTime -- 当前时间结构
 * 输  出：
 ************************************************************************************************************/
void RtcInit(void)
{
  //RTC中断，下降沿触发中断
  RtcIntIesEdge();
  RtcIntDir &= ~RtcInt;

//  RtcConverHour(1);

  SetRtcInt1Out();

  GetTimeFromRtc(&sCurTime);
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

  psTime->Year  = RtcBcd2Hex( ByteExchange(au8Temp[0]));
  psTime->Month = RtcBcd2Hex( ByteExchange(au8Temp[1]) & 0x1f);
  psTime->Date  = RtcBcd2Hex( ByteExchange(au8Temp[2]) & 0x3f);

  psTime->Week  = CaculateWeek(psTime->Year, psTime->Month, psTime->Date);

  psTime->Hour   = RtcBcd2Hex( ByteExchange(au8Temp[4]) & 0x3f);
//  RtcIntIeDisable();
  psTime->Minute = RtcBcd2Hex( ByteExchange(au8Temp[5]) & 0x7f);
//  RtcIntIeEnable();
  psTime->Second = RtcBcd2Hex( ByteExchange(au8Temp[6]) & 0x7f);
}

/*************************************************************************************************************
 * 函数名称： GetHMFromRtc
 * 功能描述： 从RTC中读出当前的小时和分钟
 * 输  入：
 * 输  出： psTime -- 当前时间结构
 ************************************************************************************************************/
void GetHMFromRtc(_tsTimeCb * psTime)
{
  UINT8 au8Temp[3];

  //从RTC获得原始数据
  RtcRead(CMD_HOUR, &au8Temp[0], 2);
  psTime->Hour   = RtcBcd2Hex( ByteExchange(au8Temp[0]) & 0x3f);
  psTime->Minute = RtcBcd2Hex( ByteExchange(au8Temp[1]) & 0x7f);
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

  au8Temp[0] = ByteExchange( RtcHex2Bcd(psTime->Year)  & 0xff);
  au8Temp[1] = ByteExchange( RtcHex2Bcd(psTime->Month) & 0x1f);
  au8Temp[2] = ByteExchange( RtcHex2Bcd(psTime->Date)  & 0x3f);

  psTime->Week = CaculateWeek(psTime->Year, psTime->Month, psTime->Date);
  au8Temp[3] = ByteExchange(psTime->Week & 0x07);

  au8Temp[4] = ByteExchange( RtcHex2Bcd(psTime->Hour)   & 0x7f);
  au8Temp[5] = ByteExchange( RtcHex2Bcd(psTime->Minute) & 0x7f);
  au8Temp[6] = ByteExchange( RtcHex2Bcd(psTime->Second) & 0x7f);
  au8Temp[7] = 0;

  RtcWrite(CMD_YEAR, &au8Temp[0], sizeof(_tsTimeCb));
  Delay(10);
}

