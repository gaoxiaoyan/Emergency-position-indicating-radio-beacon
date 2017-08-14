
/******************************************************************************************************
 *											
 * Copyright (C) 2014, Sunic-ocean
 * All Rights Reserved										
 *
 * 文 件 名： BatteryDetect.c
 * 描       述：
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
#include "Uart3.h"
#include "SoftTimer.h"
#include "stdio.h"
#include "Communication.h"

//定义电压下限为6v
#define REF_VOL         3.3       //基准电压3.3V
#define VOLTAGE_MIN     6.5       //最小电压6.5V
#define VOLTAGE_MAX     8.4       //最高电压8.4V
#define DIV             (FLOAT32)(13.0/3.0)    //分压系数
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

  ADC12CTL0 = ADC12MSC + ADC12SHT0_15;                 // 设置采样时间
  ADC12CTL1 = ADC12SHP + ADC12CONSEQ_1;                 // 使用采样定时器
  ADC12IE |= BIT0;                           // 使能中断
  ADC12MCTL0 = ADC12INCH_3;
  
  //ADC检测定时器
  s8BatDog = SwdGet();
  SwdSetLifeTime(s8BatDog, 4000); //设置采样时间为2秒
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
  UINT8 i;
  
  //ADC定时时间到，启动一次ADC
  if(SwdGetLifeTime(s8BatDog) == 0)
  {
    SwdReset(s8BatDog);
    //启动一次AD检测
    ADC_START_DETECT();
  }
  
  //ADC检测完成，进行数据处理，算出电量的百分比
  if(bComplete == TRUE)
  {
    bComplete = FALSE;

    for(i=0; i<ADC_MAX; i++)
    {
      u16AdcResules += au16AdcBuffer[i];
    }

    u16AdcResules >>= 4;      //取8组数据平均值

    //电池电压为8.4v，经过3/13衰减，然后软件上全量程放大。然后进行四舍五入，得到剩余电量的百分比
    sSendInfo.u8Battery = ((UINT32)u16AdcResules * 33) >> 12; //ADC量化
    sSendInfo.u8Battery = (UINT8)((FLOAT32)(sSendInfo.u8Battery - BAT_VOL_MIN)*AMP_PARA_2+0.5);

    if(sSendInfo.u8Battery > 9)
    {
      sSendInfo.u8Battery = 9;
    }
    else
    {
      sSendInfo.u8Battery = sSendInfo.u8Battery;
    }
    
    //调试信息
    if(g_bDebug & BAT_DEBUG)
    {
      if(sSendInfo.u8Battery > 2)
      {
        Uart3SendString("#The Current Remaining Power Is: ");
        i = sSendInfo.u8Battery + 0x30;
        Uart3SendByte(i);
        Uart3SendString("0%! \r\n");
      }
      else
      {
        Uart3SendString("#Power Less Than 30%, Please Recharge In Time！\r\n");
      }
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



                     



