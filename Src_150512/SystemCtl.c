
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
#include "Uart2.h"
#include "SoftTimer.h"
#include "CdmaModule.h"
#include "Rtc.h"
#include "I2c_Driver.h"
#include "Record.h"
#include "Communication.h"

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
  
  EepromWpDirSetOut();
  EnableEeprom();
 	
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
  //北斗模块IC检测
  BdIcCheckSetIn();
 	
 	//自锁电源控制接口
 	SystemPowerOff();
 	SystemPowerDir |= SystemPower;
  
  g_bCdmaSend = FALSE;
  g_bBdSend   = FALSE;
  
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
          
          BeiDouPowerOn();
          SystemPowerOn();
          GpsPowerOn();
          CdmaPowerOn();
          
          //打开程序运行指示灯，说明系统已经启动。
          SystemRunEnable();
          
          if(g_bDebug == TRUE)
         {
  //         SystemVersionInfo();
  //         PrintfCmdList();
           Uart3SendString("# System Power On......  \r\n");
           Uart3SendString("# Turn On Gps Power......  \r\n");
           Uart3SendString("# Turn On Cdma Power......  \r\n");
           Uart3SendString("# Turn On BeiDou Power.....  \r\n");
           Uart3SendString("# Initialize System..... \r\n");
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
BOOL bKeyFlag = FALSE;
BOOL bKeyCompleted = FALSE;
BOOL bLedFlag=FALSE;
SINT8 s8LedDog;
SINT8 s8KeyDog;
SINT8 s8KeySetDog;
#pragma vector = PORT1_VECTOR
__interrupt void PORT1_ISR(void)
{
  static UINT8 u8KeySet = 1;
  _tsTimeCb sTime;
  
  if( RtcIntCheck() )
  {
    RtcIntFlagClr();
    SetRtcInt1Out();
    //此处耗时比较大，需要改进
    if(g_bStandby == TRUE)
    {
      GetTimeFromRtc(&sTime);
      if(((sTime.Minute + 2) % (sSendInfo.u8GapTime * 5)) == 0) //提前两分钟唤醒
      {
         LPM1_EXIT;
      }
    }
  }
  
  if(KeyPowerIntCheck())
  {
    KeyPowerIntFlagClr();
    bKeyFlag = TRUE;
    
    if((u8KeySet > 2) && (SwdGetLifeTime(s8KeySetDog) > 0))
    {
      bKeyCompleted = TRUE;
      SwdDisable(s8KeySetDog);
      SwdReset(s8KeySetDog);    //复位定时器。
      u8KeySet = 1;
    }

    if(SwdGetLifeTime(s8KeySetDog) == 0)
    {
      u8KeySet = 1;
    }
    u8KeySet++;
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
     StopWatchDog();
     
     g_u16TimerDelay=0;
     while(g_u16TimerDelay < 500);  //延时5S, 等待信息完全发送。
  
      GpsPowerOff();
      CdmaPowerOff();
		  BeiDouPowerOff();
      SystemRunDisable();
      
      WylLedDisable();
      SyjLedDisable();
      
      //进入待机
      //打开RTC分钟中断
      LPM1;
      
      g_bStandby = FALSE;
      
      SystemRunEnable();
      Uart2EnableInterrupt();
      GpsPowerOn();
      CdmaPowerOn();
		  BeiDouPowerOn();
      
      CdmaInit();
      
      if(sRecordCb.u8SysType == WYL_LED)
      {
        WylLedEnable();
      }
      else
      {
        if(sRecordCb.u8SysType == SYJ_LED)
        {
          SyjLedEnable();
        }
      }
      
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
void SystemIdleInit(void)
{
  //初始化程序运行指示灯定时器
  s8LedDog = SwdGet();
  SwdSetName(s8LedDog, "LedTimer");
  SwdSetLifeTime(s8LedDog, 500);
  SwdEnable(s8LedDog);  
  
  //初始化关机按键定时器
  s8KeyDog = SwdGet();
  SwdSetName(s8KeyDog, "KeyTimer");
  SwdSetLifeTime(s8KeyDog, 3000);
  SwdDisable(s8KeyDog);  
  
  //初始化关机按键定时器
  s8KeySetDog = SwdGet();
  SwdSetName(s8KeySetDog, "KeySetTimer");
  SwdSetLifeTime(s8KeySetDog, 1000);
  SwdDisable(s8KeySetDog);  
  
  if(sRecordCb.u8SysType == WYL_LED)
  {
    WylLedEnable();
    SyjLedDisable();
  }
  else
  {
    if(sRecordCb.u8SysType == SYJ_LED)
    {
      WylLedDisable();
      SyjLedEnable();
    }
  }
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
  BOOL bFlag = FALSE;
  
  if(SwdGetLifeTime(s8LedDog) == 0)
  {
    SwdReset(s8LedDog);
    bLedFlag = ~bLedFlag;
        
    if(bLedFlag == 0)
      SystemRunEnable();
    else
      SystemRunDisable();
  }
  
  if(bKeyFlag == TRUE)
  {
    bKeyFlag = FALSE;
    SwdEnable(s8KeySetDog);
    SwdSetLifeTime(s8KeySetDog, 300);
  }
  //检测设置
  if(bKeyCompleted == TRUE)
  {
    bKeyCompleted = FALSE;
    SwdSetLifeTime(s8KeySetDog, 6000);
    SwdEnable(s8KeySetDog);  
    SystemRunDisable();       //关闭LED
    KeyPowerIeDisable();      //关闭按键中断
    StopWatchDog();
    while(SwdGetLifeTime(s8KeySetDog)>0)
    {
      if(!KeyPowerCheck)
      {
        g_u8TimerDelay=0;
        while(g_u8TimerDelay<2);  //延时20ms；
        if(!KeyPowerCheck)
        {
          while(!KeyPowerCheck);
          SwdReset(s8KeySetDog);    //复位定时器。
          if(bFlag == TRUE)
          {
            sRecordCb.u8SysType = SYJ_LED;
            g_u8NeedSaved |= SAVE_SYS_TYPE;
            
            WylLedDisable();
            SyjLedEnable();
        
            bFlag = FALSE;
          }
          else
          {
            sRecordCb.u8SysType = WYL_LED;
            g_u8NeedSaved |= SAVE_SYS_TYPE;
            
            WylLedEnable();
            SyjLedDisable();
        
            bFlag = TRUE;
          }
        }
      }
    }
    SwdSetLifeTime(s8KeySetDog, 1000);
    SwdDisable(s8KeySetDog);  
    KeyPowerIeEnable();      //关闭按键中断
  }
  
  //检测关机
  if( !KeyPowerCheck)
  {
    if(SwdIsEnable(s8KeyDog) == TRUE)
    {
      if(SwdGetLifeTime(s8KeyDog) == 0)
      {
        SwdReset(s8KeyDog);
        
        //关闭LED指示
        SystemRunDisable();
        WylLedDisable();
        SyjLedDisable();
        
        GpsPowerOff();
        CdmaPowerOff();
        BeiDouPowerOff();
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


