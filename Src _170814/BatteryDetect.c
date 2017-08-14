
/******************************************************************************************************
 *
 * Copyright (C) 2014, Sunic-ocean
 * All Rights Reserved
 *
 * 文 件 名： BatteryDetect.c
 * 描    述：
 *
 * 修改日期： 2015年4月29日09:16:28
 * 作    者： Bob
 * 当前版本： V1.01
 * 修改内容：
 * ?////////////////////////////////////////////////////////////////////////////////////////
 * 创建日期： 2014年9月5日 09:45:35
 * 作    者： Bob
 * 当前版本： V1.00
 *******************************************************************************************************/
#include "msp430x54x.h"

#include "DataType.h"
#include "global.h"
#include "BatteryDetect.h"
#include "Uart2.h"
#include "SoftTimer.h"
#include "stdio.h"
#include "Communication.h"

//定义电压下限为6v
#define REF_VOL         (UINT8)(3.3 * 10)       //基准电压3.3V
#define VOLTAGE_MIN     6.4       //最小电压6.2V
#define VOLTAGE_MAX     8.3       //最高电压8.4V
#define DIV             ((FLOAT32)15.1/5.1)    //分压系数
#define BAT_VOL_MIN     (FLOAT32)((VOLTAGE_MIN * 10.0) / DIV)
#define BAT_VOL_MAX     (FLOAT32)((VOLTAGE_MAX * 10.0) / DIV)
#define AMP_PARA_2      (FLOAT32)(10.0 / (BAT_VOL_MAX - BAT_VOL_MIN))  //二级放大系数

#define ADC_MAX     16
UINT16 au16AdcBuffer[ADC_MAX];
UINT16 u16AdcResules = 0;
BOOL bComplete = FALSE;
SINT8 s8BatDog=0;

/*******************************************************************************
 * 函数名称：AdcInit
 * 函数功能：初始化ADC

 * 参数: 无
 * 返回: 无
********************************************************************************/
void Adc_Init(void)
{
  P6SEL |= BIT3;                            // 用P6.1作为ADC的检测通道

  AdcCtlDisable();
  AdcCtlDir |= AdcCtl;

  ADC12CTL0  = ADC12MSC + ADC12SHT0_15;                 // 设置采样时间
  ADC12CTL1  = ADC12SHP + ADC12CONSEQ_1;                 // 使用采样定时器
  ADC12IE   |= BIT0;                           // 使能中断
  ADC12MCTL0 = ADC12INCH_3;

  //ADC检测定时器
  s8BatDog = SwdGet();
  SwdSetName(s8BatDog, "BatteryTimer");
  SwdSetLifeTime(s8BatDog, 2000); //设置采样时间为2秒
  SwdEnable(s8BatDog);
}

/*******************************************************************************
 * 函数名称：BatteryDetect
 * 函数功能：电池电量检测任务

 * 参数: 无
 * 返回: 无
********************************************************************************/
void BatteryDetect(void)
{
  UINT8 i=0;
  FLOAT32 f32Bat = 0.0;

  //ADC定时时间到，启动一次ADC
  if(SwdGetLifeTime(s8BatDog) == 0)
  {
    SwdReset(s8BatDog);
    AdcCtlEnable();
    //启动一次AD检测
    ADC_START_DETECT();
  }

  //ADC检测完成，进行数据处理，算出电量的百分比
  if(bComplete == TRUE)
  {
    bComplete = FALSE;

    AdcCtlDisable();

    for(i=0; i<ADC_MAX; i++)
    {
      u16AdcResules += au16AdcBuffer[i];
    }

    u16AdcResules >>= 4;      //取8组数据平均值

    //电池电压为8.4v，经过3/13衰减，然后软件上全量程放大。然后进行四舍五入，得到剩余电量的百分比
    f32Bat = ((FLOAT32)u16AdcResules * REF_VOL) / 4096;  //ADC量化
    f32Bat = (FLOAT32)(f32Bat - BAT_VOL_MIN)*AMP_PARA_2 + 0.5;
    sSendInfo.u8Battery = (UINT8)(f32Bat*10);

//    sSendInfo.u8Battery = (UINT8)((FLOAT32)(((FLOAT32)u16AdcResules * REF_VOL) / 4096) + 0.5); //ADC量化
//    sSendInfo.u8Battery = (UINT8)((FLOAT32)(sSendInfo.u8Battery - BAT_VOL_MIN)*AMP_PARA_2+0.5);

    if(sSendInfo.u8Battery > 99)
    {
      sSendInfo.u8Battery = 99;
    }
    else
    {
      sSendInfo.u8Battery = sSendInfo.u8Battery;
    }

  }
}

/*******************************************************************************
 * 函数名称：AdcDetect
 * 函数功能：电池电量检测任务

 * 参数: bType -- 1:正常运行的读取，0：开机检查读取
 * 返回: 电量百分比，1~10
********************************************************************************/
UINT8 AdcDetect(BOOL bType)
{
  UINT8 buf[2], u8Bat;
  UINT8 i=0;
  FLOAT32 f32Bat=0.0;

  //ADC检测完成，进行数据处理，算出电量的百分比
  ADC_START_DETECT();
  bComplete = FALSE;
  while(bComplete != TRUE);
  bComplete = FALSE;

  for(i=0; i<ADC_MAX; i++)
  {
    u16AdcResules += au16AdcBuffer[i];
  }

  u16AdcResules >>= 4;      //取8组数据平均值

  //电池电压为8.4v，经过3/13衰减，然后软件上全量程放大。然后进行四舍五入，得到剩余电量的百分比
  f32Bat = ((FLOAT32)u16AdcResules * REF_VOL) / 4096;  //ADC量化
  f32Bat = (FLOAT32)(f32Bat - BAT_VOL_MIN)*AMP_PARA_2 + 0.5;
  u8Bat = (UINT8)(f32Bat*10);

  if(u8Bat > 99)
    u8Bat = 99;

  if(bType == TRUE)
  {
    Uart2SendString("# The Current Remaining Power Is: ");
    buf[0] = u8Bat/10 + 0x30;
    buf[1] = u8Bat%10 + 0x30;
    Uart2SendBuffer(&buf[0], 2);
    Uart2SendString("%! \r\n");

    if(u8Bat < 30)
    {
      Uart2SendString("# Power Less Than 30%, Please Recharge In Time！\r\n");
    }

    return 0xaa;
  }
  else
  {
    if(u8Bat > 7)
    {
      return 0x04;
    }
    else if((u8Bat > 5) && (u8Bat < 8))
    {
      return 0x03;
    }
    else if((u8Bat > 3) && (u8Bat < 6))
    {
      return 0x02;
    }
    else if((u8Bat > 1) && (u8Bat < 4))
    {
      return 0x01;
    }
    else
    {
      return 0x00;
    }
  }
}

/*******************************************************************************
 * 函数名称：ADC12_ISR
 * 函数功能：ADC中断处理函数。多次采样取平均值

 * 参数: 无
 * 返回: 无
********************************************************************************/
#pragma vector = ADC12_VECTOR
__interrupt void ADC_ISR(void)
{
  static UINT8 u8Index=0;

  au16AdcBuffer[u8Index++] = ADC12MEM0;
  if(u8Index == ADC_MAX)
  {
    bComplete = TRUE;
    ADC_STOP_DETECT();  //关闭ADC
    u8Index = 0;
    u16AdcResules = 0;
  }
}







