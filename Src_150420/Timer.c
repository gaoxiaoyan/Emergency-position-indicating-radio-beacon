
/******************************************************************************
*											
* Copyright (C) 2014, Sunic-ocean
* All Rights Reserved										
*
* 文 件 名： Timer.C
* 描    述：定时器底层驱动

* 创建日期： 2015年4月10日16:25:16
* 作    者： Bob
* 当前版本： V1.0

------------------------------------------------------
*******************************************************************************/
#include "msp430x54x.h"

#include "DataType.h"
#include "global.h"
#include "Timer.h"
#include "SoftWatchDog.h"

#define TIMER_TICK          100    //设置定时器A0的中断频率为100hz

#define INIT_VAL0           (32768/TIMER_TICK - 1)   //设置正常工作定时器初值
#define INIT_VAL1           (32768 - 1)              //待机模式下定时器设置为1s初值

//定时器中断使能
#define ITMERA_INTERRUPT_DISABLE()      TA0CCTL0 &= ~CCIE //TA0CTL = MC_0
#define TIMERA_INTERRUPT_ENABLE()       TA0CCTL0 |=  CCIE //TA0CTL = TASSEL_1 + MC_1


/******************************************************************************
 * 函数名称：TimerA_Init
 * 描述：TIMER A init
 *
 * 参数：无
 * 返回：无
 *****************************************************************************/
void TimerA_Init(void)
{
  SwdInit();
  
  TA0CCR0  = INIT_VAL0;                         //中断误差补偿
  TA0CCTL0 = CCIE;                              //使能中断
  TA0CTL   = TASSEL_1 + MC_1;                   // ACLK, upmode
}

/******************************************************************************
 * 函数名称：TimerInterruptEnable
 * 描述：使能TIMER A0中断
 *
 * 参数：无
 * 返回：无
 *****************************************************************************/
void TimerInterruptEnable(void)
{
  TIMERA_INTERRUPT_ENABLE();
}

/******************************************************************************
 * 函数名称：TimerInterruptDisable
 * 描述：关闭TIMER A0中断
 *
 * 参数：无
 * 返回：无
 *****************************************************************************/
void TimerInterruptDisable(void)
{
  ITMERA_INTERRUPT_DISABLE();
}

/******************************************************************************
 * 函数名称：TimerA0_ISR
 * 描述：TIMER A0 中断处理函数
 *
 * 参数：无
 * 返回：无
 *****************************************************************************/
#pragma vector = TIMER0_A0_VECTOR
__interrupt void TimerA_ISR(void)
{
  SwdRun();
  
  g_u8TimerDelay++;
  g_u16TimerDelay++;
}

