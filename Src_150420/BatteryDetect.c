
/******************************************************************************************************
 *											
 * Copyright (C) 2014, Sunic-ocean
 * All Rights Reserved										
 *
 * 文 件 名： BatteryDetect.c
 * 描       述：

 * 创建日期： 2014年9月5日 09:45:35
 * 作    者： Bob
 * 当前版本： V1.00
 *******************************************************************************************************/

#ifdef MSP430F149
#include "msp430x14x.h"
#else
#include "msp430x54x.h"
#endif

#include "DataType.h"
#include "global.h"
#include "BatteryDetect.h"
#include "Uart.h"
#include "Timer.h"
#include "Display.h"
#include "stdio.h"

#define ADC_MAX     8
UINT16 au16AdcBuffer[ADC_MAX];
UINT16 u16AdcResules = 0;
BOOL bComplete = FALSE;

/*******************************************************************************
 * 函数名称：AdcInit
 * 函数功能：初始化ADC

 * 参数: 无
 * 返回: 无
********************************************************************************/
void Adc_Init(void)
{
  P6SEL |= BIT3;                            // 用P6.1作为ADC的检测通道
//  P6DIR &= BIT1;

  ADC12CTL0 = ADC12MSC + ADC12SHT0_15;                 // 设置采样时间
  ADC12CTL1 = ADC12SHP + ADC12CONSEQ_1;                 // 使用采样定时器
  ADC12IE = BIT3;                           // 使能中断
  ADC12MCTL0 = ADC12INCH_3;
  
  //ADC检测定时器
  asTimer[TIMER_ADC].u16LifeTime = TIMER_TICK * 4;
  asTimer[TIMER_ADC].u16LifeOldTime = TIMER_TICK * 4;
  asTimer[TIMER_ADC].bEnable = TRUE;
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
  UINT8 u8Bat[15] = {"当前电量为000%!"};
  
  //ADC定时时间到，启动一次ADC
  if(asTimer[TIMER_ADC].u16LifeTime == 0)
  {
    asTimer[TIMER_ADC].u16LifeTime = asTimer[TIMER_ADC].u16LifeOldTime;
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

    u16AdcResules >>= 3;      //取8组数据平均值

    //电池电压为7.4v，经过1/3分压 ADC采集，到电压到3.6V时不能满足DTU电压要求
    //分压后为1.2V。量化时为了计算将电压都扩大10倍来计算
    sDisplay.u8BatElec = ((UINT32)u16AdcResules * 33) >> 12; //ADC量化

    if(sDisplay.u8BatElec > 12)
    {
        sDisplay.u8BatElec -= 12;
        if(sDisplay.u8BatElec > 9)
        {
            sDisplay.u8BatElec = 9;
        }
    }
    else
    {
      sDisplay.u8BatElec = 0;
    }
    
    sSendData.u8Battery = sDisplay.u8BatElec;
    
    //调试信息
    if(g_bDebug == TRUE)
    {
      sprintf(&u8Bat[10], "%2d", sDisplay.u8BatElec);
      UartSendString(UART2, &u8Bat[0]);
      UartSendString(UART2, " \r\n");
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







