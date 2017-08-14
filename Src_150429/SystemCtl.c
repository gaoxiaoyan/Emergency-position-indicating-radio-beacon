
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
 *函数名称: GetSystemVersionInfo
 *功能描述: 打印系统版本信息

 *参数: 
 *返回:
*****************************************************************************/
void GetSystemVersionInfo(void)
{
  UINT8 au8Version[8];

//  if(g_bDebug == TRUE)
  {
    Uart3SendString("\r\n#Debug HandShake OK! The Bandrate is 115200bps.\r\n");
    Uart3SendString("/***********************************************************/\r\n");
    Uart3SendString("// Copyright (C) 2015, Sunic-ocean.\r\n");
    Uart3SendString("// All Rights Reserved.\r\n");
    Uart3SendString(" \r\n");
    Uart3SendString("// Project Name：应急物资示位标.\r\n");
      
    memset(&au8Version[0], 0, 8);
    au8Version[0] = 'V';
    au8Version[1] = VERSION / 100 + '0';
    au8Version[2] = '.';
    au8Version[3] = VERSION % 100 / 10 + '0';
    au8Version[4] = VERSION % 10 + '0';
    au8Version[5] = '\r';
  
    Uart3SendString("// Version：");
    Uart3SendString(&au8Version[0]);
    Uart3SendString("/***********************************************************/\r\n");
    Uart3SendString(" \r\n");
  }
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
  if( RtcIntCheck() )
  {
    RtcIntFlagClr();
    SetRtcInt1Out();
  }
  
  if(KeyPowerIntCheck())
  {
    KeyPowerIntFlagClr();
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

/***************************************************************************
//函数名称：PrintfCmdList
//功能描述：打印命令列表
//
//参数：无
//返回：无
***************************************************************************/
void PrintfCmdList(void)
{
  Uart3SendString("###############################################################\r\n");
  Uart3SendString("#            应急物资示位标内部调试指令       \r\n");
  Uart3SendString("#==============================================================\r\n");
  Uart3SendString("#  0、命令格式：AA + 55 + cmd + CC + 33.  \r\n");
  Uart3SendString("#  1、cmd<00> -- 串口通讯握手命令；\r\n");
  Uart3SendString("#  2、cmd<01> -- 获取系统软件版本信息；\r\n");
  Uart3SendString("#  3、cmd<10> -- GPS调试信息关闭；\r\n");
  Uart3SendString("#  4、cmd<11> -- GPS调试信息打开；\r\n");
  Uart3SendString("#  5、cmd<20> -- BeiDou调试信息关闭；\r\n");
  Uart3SendString("#  6、cmd<21> -- BeiDou调试信息打开；\r\n");
  Uart3SendString("#  7、cmd<22> -- BeiDou发送报文测试；\r\n");
  Uart3SendString("#  8、cmd<30> -- CDMA调试信息关闭；\r\n");
  Uart3SendString("#  9、cmd<31> -- CDMA调试信息打开；\r\n");
  Uart3SendString("# 10、cmd<32> -- 获取CDMA时间；\r\n");
  Uart3SendString("# 11、cmd<33> -- 将CDMA时间校准RTC；\r\n");
  Uart3SendString("# 12、cmd<34> -- 获取本机号码；\r\n");
  Uart3SendString("# 13、cmd<35> -- 设置接收号码；\r\n");
  Uart3SendString("# 14、cmd<35> -- 设置接收号码；\r\n");
  Uart3SendString("# 15、cmd<40> -- 设置通讯间隔时间；\r\n");
  Uart3SendString("# 16、cmd<50> -- 获取当前电池剩余电量；\r\n");
  Uart3SendString("###############################################################\r\n");
}

