
/******************************************************************************************************
 *											
 * Copyright (C) 2014, Sunic-ocean
 * All Rights Reserved										
 *
 * 文 件 名： SystemCtl.c
 * 描       述：

 * 创建日期： 2014年9月4日 13:23:50
 * 作    者： Bob
 * 当前版本： V1.00
 *******************************************************************************************************/
#define SYS_GLOBAL
#include "msp430x54x.h"
#include "global.h"
#include "DataType.h"
#include "Timer.h"
#include "SystemCtl.h"
#include "string.h"
#include "Uart3.h"
#include "SoftTimer.h"
#include "CdmaModule.h"
#include "Rtc.h"

#define CPU_CLK       4194304ul 
#define FLL_FACTOR    (CPU_CLK / 32768ul)


/*****************************************************************************
 *函数名称: InitClk
 *功能描述: 系统时钟初始化

 *参数: 
 *返回:
*****************************************************************************/
void InitClk(void) 
{     
    P11DS  |= BIT0 + BIT1 + BIT2;
    P11SEL |= BIT0 + BIT1 + BIT2;
    P11DIR |= BIT0 + BIT1 + BIT2;
    
    // 启动XT1  
    P7SEL |= BIT0 + BIT1;                        // P7.0 P7.1 外设功能  
    UCSCTL6 &= ~(XT1OFF);                       // XT1打开  
    UCSCTL6 |= XCAP_3;                          // 内部电容  
    do  
    {  
        UCSCTL7 &= ~XT1LFOFFG;                  // 清楚XT1错误标记  
    }while (UCSCTL7&XT1LFOFFG);                 // 检测XT1错误标记  

    __bis_SR_register(SCG0);                    // 禁止FLL功能  
    UCSCTL0 = 0x0000;                           // Set lowest possible DCOx, MODx  
    UCSCTL1 = DCORSEL_5;                        // DCO最大频率为6MHz  
    UCSCTL2 = FLLD_1 + FLL_FACTOR;              // 设置DCO频率为4MHz  

    __bic_SR_register(SCG0);                    // 使能FLL功能  
  
    // 必要延时  
    __delay_cycles(250000);  
  
    // 清楚错误标志位  
    do  
    {  
        UCSCTL7 &= ~(XT2OFFG + XT1LFOFFG + XT1HFOFFG + DCOFFG);  
                                                // 清除所有振荡器错误标志位  
        SFRIFG1 &= ~OFIFG;                      // 清除振荡器错误  
    }while (SFRIFG1&OFIFG);                     // 等待清楚完成  
    
    UCSCTL4 |= SELS_7 + SELM_7;
}  

/*****************************************************************************
 *函数名称: SystemInit
 *功能描述: 系统初始化

 *参数: 
 *返回:
*****************************************************************************/
void SystemInit(void)
{
  
  //程序运行指示灯
  SystemRunDisable();
  SystemRunDir |= SystemRun;
  
  //低压指示灯
  LowPowerLedDisable();
  LowPowerLedDir |= LowPowerLed;
 
 	//围油栏指示灯
 	WylLedDisable();
 	WylLedDir |= WylLed;
 	
 	//收油机指示灯
 	SyjLedDisable();
 	SyjLedDir |= SyjLed;
 	
 	//开关机按键接口, 下降沿触发中断
 	KeyPowerIesEdge();
// 	KeyPowerIeEnable();
 	KeyPowerDir &= ~KeyPower;
 	
 	//GPS电源控制接口
 	GpsPowerOff();
 	GpsPowerDir |= GpsPower;
 	
 	//CDMA电源控制接口
 	CdmaPowerOff();
 	CdmaPowerDir |= CdmaPower;
 	
 	//BD电源控制接口
 	BeiDouPowerOff();
 	BeiDouPowerDir |= BeiDouPower;
 	
 	//自锁电源控制接口
 	SystemPowerOff();
 	SystemPowerDir |= SystemPower;
  
  OS_MemInit();         
}

/*****************************************************************************
 *函数名称: WaitSystemStart
 *功能描述: 系统开机检查

 *参数: 
 *返回:
*****************************************************************************/
void WaitSystemStart(void)
{  
  SINT8 s8Dog;

  s8Dog = SwdGet();
  SwdSetLifeTime(s8Dog, 3000);
//  SwdEnable(s8Dog);

  while(TRUE)
  {
    if(!KeyPowerCheck)
    {
      if(SwdIsEnable(s8Dog) == TRUE)
      {
        if(SwdGetLifeTime(s8Dog) == 0)
        {
          SwdDisable(s8Dog);
          SwdFree(s8Dog);
          
          //打开按键中断
          KeyPowerIeEnable();
          
          SystemPowerOn();
          GpsPowerOn();
          CdmaPowerOn();
          
          
          //打开程序运行指示灯，说明系统已经启动。
          SystemRunEnable();
          
          if(g_bDebug == TRUE)
         {
  //         SystemVersionInfo();
  //         PrintfCmdList();
           Uart3SendString("#System Power On......  \r\n");
           Uart3SendString("#Turn On Gps Power......  \r\n");
           Uart3SendString("#Turn On Cdma Power......  \r\n");
           Uart3SendString("#Turn On BeiDou Power.....  \r\n");
           Uart3SendString("#Initialize System..... \r\n");
         }
          
          return;
        }
        else
        {
          SystemPowerOff();
        }
      }
      else
      {
        SwdEnable(s8Dog);
      }
    }
    else
    {
      SystemPowerOff();
    }
  }
}


/*****************************************************************************
 函 数 名  : PORT1_ISR
 功能描述  : P1端口的中断服务函数

  参  数  : NONE
 返 回 值 : NONE
*****************************************************************************/
#pragma vector = PORT1_VECTOR
__interrupt void PORT1_ISR(void)
{
  static BOOL bFlag = 0;
  
  if( RtcIntCheck() )
  {
    RtcIntFlagClr();
    SetRtcInt1Out();
  }
  
  if(KeyPowerIntCheck())
  {
    KeyPowerIntFlagClr();
    
    if(bFlag == FALSE)
    {
      u8ClassLed &= ~BIT1;
      u8ClassLed |=  BIT0;
      bFlag = TRUE;
    }
    else
    {
      u8ClassLed &= ~BIT0;
      u8ClassLed |=  BIT1;
      bFlag = FALSE;
    }
  }
}

/***************************************************************************
//函数名称：EnterLowPower
//功能描述：数据发送成功，进入低功耗模块
//
//参数：无
//返回：无
***************************************************************************/
void EnterLowPower(void)
{
   if( TRUE == g_bStandby )
   {
      g_bStandby = FALSE;
      
      //进入待机
      //打开RTC分钟中断
      
      //待机唤醒
   }
}

/***************************************************************************
//函数名称：SystemIdleInit
//功能描述：初始化
//
//参数：无
//返回：无
***************************************************************************/
BOOL bFlag=FALSE;
SINT8 s8LedDog;
SINT8 s8KeyDog;
void SystemIdleInit(void)
{
  //初始化程序运行指示灯定时器
  s8LedDog = SwdGet();
  SwdSetLifeTime(s8LedDog, 500);
  SwdEnable(s8LedDog);  
  
  //初始化关机按键定时器
  s8KeyDog = SwdGet();
  SwdSetLifeTime(s8KeyDog, 3000);
  SwdDisable(s8KeyDog);  
}

/***************************************************************************
//函数名称：SytemIdleRun
//功能描述：程序运行指示灯
//
//参数：无
//返回：无
***************************************************************************/
void SystemIdleRun(void)
{
  if(SwdGetLifeTime(s8LedDog) == 0)
  {
    SwdReset(s8LedDog);
    bFlag = ~bFlag;
        
    if(bFlag == 0)
      SystemRunEnable();
    else
      SystemRunDisable();
  }
  //收油机和围油栏指示灯切换
  if(u8ClassLed & SYJ_LED)
  {
    WylLedDisable();
    SyjLedEnable();
  }
  else
  {
    if(u8ClassLed & WYL_LED)
    {
      WylLedEnable();
      SyjLedDisable();
    }
  }
  
  if( !KeyPowerCheck)
  {
    if(SwdIsEnable(s8KeyDog) == TRUE)
    {
      if(SwdGetLifeTime(s8KeyDog) == 0)
      {
        SwdReset(s8KeyDog);
        
        //关闭LED指示
        SystemRunDisable();
        
        //关闭系统电源
        SystemPowerOff();
        
        while(TRUE);  //等待看门狗复位
      }
    }
    else
    {
      SwdEnable(s8KeyDog);
    }
  }
  else
  {
    SwdDisable(s8KeyDog);
    SwdReset(s8KeyDog);
  }
}


