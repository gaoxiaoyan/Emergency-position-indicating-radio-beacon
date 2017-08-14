
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
#include "BatteryDetect.h"
#include "GpsParse.h"
#include "BeiDouModule.h"

#define CPU_CLK       4194304ul 
#define FLL_FACTOR    (CPU_CLK / 32768ul)

//测试使用
//#define TEST 

void LpEnterCfg(void);
void LpExitCfg(void);

/*
****************************************************************************
*函数名称: InitClk
*功能描述: 系统时钟初始化
*
*参数: 系统启动写1，系统启动时默认DCO是1Mhz，需要延时周期长
*      唤醒启动写0，从32768hz唤醒，需要延时周期短
*返回:
****************************************************************************
*/
void InitClk(BOOL bFlag) 
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
    if(bFlag == TRUE)
      __delay_cycles(250000);  
    else
      __delay_cycles(6000);  
  
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
 *函数名称:  DcoSwitchXt1
 *功能描述: 把主时钟和子时钟由DCO切换到XT1

 *参数: 
 *返回:
*****************************************************************************/
void DcoSwitchXt1(void)
{
  UCSCTL4 = SELS_0 + SELM_0;
  __bis_SR_register(SCG0);                    // 禁止FLL功能 
  __delay_cycles(250);  
}

/*****************************************************************************
 *函数名称: ReservedPinCfg
 *功能描述: 不使用的管脚配置

 *参数: 
 *返回:
*****************************************************************************/
void ReservedPinCfg(void)
{
  P1DIR &= ~(BIT0 + BIT3 + BIT4);
  P1OUT &= ~(BIT0 + BIT3 + BIT4);
  
  P2DIR &= ~(BIT1 + BIT2 + BIT3 + BIT4 + BIT5 + BIT6 + BIT7);
  P2OUT &= ~(BIT1 + BIT2 + BIT3 + BIT4 + BIT5 + BIT6 + BIT7);
  
  P3DIR &= ~(BIT2 + BIT3 + BIT6 + BIT7);
  P3OUT &= ~(BIT2 + BIT3 + BIT6 + BIT7);
  
  P4DIR &= ~(BIT0 + BIT1 + BIT2 + BIT3 + BIT4 + BIT5 + BIT6 + BIT7);
  P4OUT &= ~(BIT0 + BIT1 + BIT2 + BIT3 + BIT4 + BIT5 + BIT6 + BIT7);
  
  P5DIR &= ~(BIT2 + BIT3 + BIT4 + BIT5);
  P5OUT &= ~(BIT2 + BIT3 + BIT4 + BIT5);
  
  P6DIR &= ~(BIT4 + BIT5 + BIT6);
  P6OUT &= ~(BIT4 + BIT5 + BIT6);
  
  P7DIR &= ~(BIT2 + BIT3 + BIT6 + BIT7);
  P7OUT &= ~(BIT2 + BIT3 + BIT6 + BIT7);
  
  P8DIR &= ~(BIT2 + BIT3 + BIT4 + BIT5 + BIT6 + BIT7);
  P8OUT &= ~(BIT2 + BIT3 + BIT4 + BIT5 + BIT6 + BIT7);
  
  P9DIR &= ~(BIT0 + BIT1 + BIT2 + BIT3 + BIT6 + BIT7);
  P9OUT &= ~(BIT0 + BIT1 + BIT2 + BIT3 + BIT6 + BIT7);
  
  P10DIR &= ~(BIT1 + BIT2 + BIT3 + BIT6 + BIT7);
  P10OUT &= ~(BIT1 + BIT2 + BIT3 + BIT6 + BIT7);

  P11DIR &= ~(BIT0 + BIT1 + BIT2);
  P11OUT &= ~(BIT0 + BIT1 + BIT2);
  
//  PJDIR  |=  (BIT0 + BIT1 + BIT2 + BIT3);
//  PJOUT  &= ~(BIT0 + BIT1 + BIT2 + BIT3);
}

/*****************************************************************************
 *函数名称: SystemInit
 *功能描述: 系统初始化

 *参数: 
 *返回:
*****************************************************************************/
void SystemInit(void)
{
  /* debug控制切换 */
  DebugCtlSet();
  DebugCtlDir |= DebugCtl;
  
  RstCtlSet();
  RstCtlDir   |= RstCtl;
    
  //程序运行指示灯
  SystemRunDisable();
  SystemRunDir |= SystemRun;
  
  //低压指示灯
  LowPowerLedDisable();
  LowPowerLedDir |= LowPowerLed;
  
  EepromWpDirSetOut();
  EnableEeprom();
 	
//开关机按键接口, 下降沿触发中断
#if KEY_SCAN_EN > 0
 	KeyPowerIesEdge();
 	KeyPowerIeEnable();
 	KeyPowerDir &= ~KeyPower;
#endif
 	
 	//GPS电源控制接口
 	GpsPowerOn();
 	GpsPowerDir |= GpsPower;
 	
 	//CDMA电源控制接口
 	CdmaPowerOn();
 	CdmaPowerDir |= CdmaPower;
 	
 	//BD电源控制接口
 	BeiDouPowerOn();
 	BeiDouPowerDir |= BeiDouPower;
//北斗模块IC检测
//  BdIcCheckSetIn();
 	
 	//自锁电源控制接口
#if KEY_SCAN_EN > 0
 	SystemPowerOff();
 	SystemPowerDir |= SystemPower;
#endif
  
  OS_MemInit();     
  
  ReservedPinCfg();
  
  g_bCdmaSend = FALSE;
  g_bBdSend   = FALSE;
  g_bSendInfo = FALSE;
}

/*****************************************************************************
 *函数名称: WaitSystemStart
 *功能描述: 系统开机检查

 *参数: 
 *返回:
*****************************************************************************/
#if KEY_SCAN_EN > 0
void WaitSystemStart(void)
{  
  SINT8 s8Dog;
  UINT8 u8Bat;

  s8Dog = SwdGet();
  SwdSetLifeTime(s8Dog, 3000);

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
                    
          SystemPowerOn();
          
          GpsPowerOn();
          BeiDouPowerOn();
//          CdmaPowerOn();
          
          u8Bat = AdcDetect(0);

          switch(u8Bat)
          {
            case 0:                     
            {
              FreeWatchDog();          //释放看门狗
              while(TRUE)              //电压过低时，直接释放16秒看门狗，将低压指示闪烁16秒后关闭系统
              {                   
                g_u8TimerDelay = 0;
                while(g_u8TimerDelay<50);
                LowPowerLedDisable();
                
                g_u8TimerDelay = 0;
                while(g_u8TimerDelay<50);
                LowPowerLedEnable();
              }
              
              SystemPowerOff();
              break;
            }
            
            case 1:                   //高于20%电量
            {
              SystemRunDisable();
              LowPowerLedDisable();
              WylLedDisable();
              SyjLedEnable();
              break;
            }
            
            case 2:                   //高于40%电量
            {
              SystemRunDisable();
              LowPowerLedDisable();
              WylLedEnable();
              SyjLedEnable();
              break;
            }
            
            case 3:                   //高于60%电量
            {
              SystemRunDisable();
              LowPowerLedEnable();
              WylLedEnable();
              SyjLedEnable();
              break;
            }
            
            case 4:                   //高于80%电量
            {
              SystemRunEnable();
              LowPowerLedEnable();
              WylLedEnable();
              SyjLedEnable();
              break;
            }
            
            default:  
            {
              break;
            }
          }
       
          FreeWatchDog();          //释放看门狗
          
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
#endif

/*****************************************************************************
 函 数 名  : PORT1_ISR
 功能描述  : P1端口的中断服务函数

  参  数  : NONE
 返 回 值 : NONE
*****************************************************************************/
BOOL bLedFlag=FALSE;
SINT8 s8LedDog;
_tsTimeCb sTime;

#if KEY_SCAN_EN > 0
  BOOL  bKeyFlag = FALSE;
  BOOL  bKeyCompleted = FALSE;
  SINT8 s8KeyDog;
  SINT8 s8KeySetDog;
  BOOL  bKeyWakeUp;
  UINT8 u8KeySet = 1;
#endif

#pragma vector = PORT1_VECTOR
__interrupt void PORT1_ISR(void)
{
#if KEY_SCAN_EN > 0  
  if(KeyPowerIntCheck())
  {

    KeyPowerIntFlagClr();
    bKeyFlag = TRUE;
    
    if(bKeyWakeUp == FALSE)
    {
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
#endif
  
  if( RtcIntCheck() )
  {
    RtcIntFlagClr();
    
    sRecordCb.u32SysRunTime++;
    
    //此处耗时比较大，但是基于RTC中断机制，分边沿中断后，在7.81ms再次输出中断
    GetTimeFromRtc(&sTime);
    if(g_bStandby == TRUE)
    {      
      if(((sTime.Minute + 1) % sSendInfo.u8Minute) == 0) //提前一分钟唤醒
      {
            LPM3_EXIT;
      }
    }
    else
    {  
      if(g_bGpsCompleted == TRUE)
      {
        if((sTime.Minute % sSendInfo.u8Minute) == 0)       //到时发送
        {
          g_bSendInfo = TRUE;
        }
      }
    }
    
    SetRtcInt1Out();
    
    I2cSclDir |= I2cScl;
    I2cSdaDir |= I2cSda;
    I2cSclOut |= I2cScl;
    I2cSdaOut |= I2cSda;
  }

  if(g_bCdmaSend == TRUE)
  {
    if( CdmaRiIntCheck() )
    {
      CdmaRiIntFlagClr();
      if(g_bStandby == TRUE)
      {
        g_bCdmaRiEvent = TRUE;      //接收到短信时将RI事件标志位置位
        LPM3_EXIT;
      }
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
     StopWatchDog();
     GpsPowerOff();
     SystemRunDisable();
     
     //打开RTC外部中断
      SetRtcInt1Out();
      
#if KEY_SCAN_EN > 0
      //打开按键中断唤醒功能
      bKeyWakeUp = TRUE;
#endif
      
#ifdef BEIDOU_TEST
      u8BdCentage = (UINT32)u16BdRxNum * 100 / u16BdSendNum;
      
      SaveBdRxNbr(&u16BdRxNum);
      SaveBdSendNbr(&u16BdSendNum);
      SaveBdSendSucceedNbr(&u16BdSendSucceed);
#else     
     g_u16TimerDelay=0;
     while(g_u16TimerDelay < 500);  //延时5S, 等待信息完全发送。
#endif

      if(g_bCdmaSend == TRUE)
        CdmaDtrSetHigh();     //拉高DTR，将CDMA进入待机。();
      else
        CdmaPowerOff();
      
		  BeiDouPowerOff();
      
      //关闭Timer
      TimerInterruptDisable();  //必须关闭定时器中断，否则定时器一直中断占用外部中断进入中断。
      //关闭ADC
      ADC_STOP_DETECT();
      //关闭Uart
      
      //低功耗管脚配置
      LpEnterCfg();
      
      //将始终切换到外部晶体
      DcoSwitchXt1();
      
      //进入待机
      //打开RTC分钟中断
      LPM3;
      
      //唤醒后先将时钟恢复到DCO。
      InitClk(0);
           
      LpExitCfg();
//      EepromPowerOn();
      
      //启动ADC
      ADC_START_DETECT();
      //启动Timer
      TimerInterruptEnable(); 
      //启动Uart
      
      g_bStandby = FALSE;
      
#if KEY_SCAN_EN > 0
      bKeyWakeUp = FALSE;
#endif
      
      SystemRunEnable();
      Uart2EnableInterrupt();
      GpsPowerOn();
      
      //每次唤醒保存一下运行时间。
      SaveSystemRunTime(&sRecordCb.u32SysRunTime);
      
      SwdEnable(sSendInfo.sDog);     //启动开启通讯模块定时器
      SwdReset(sSendInfo.sDog);
      
      //使能GPS接收超时定时器
      SwdReset(g_s8GpsFirstDog);
      SwdEnable(g_s8GpsFirstDog);    
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
  
#if KEY_SCAN_EN > 0
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
  
  //打开开机按键中断使能
  KeyPowerIeEnable();
#endif
  
  LowPowerLedDisable();
  
  //打开中断使能
  RtcIntIeEnable();
  
  if(g_bCdmaSend == TRUE)
    CdmaRiIntEnable();
 
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
#if KEY_SCAN_EN > 0
  BOOL bFlag = FALSE;
    
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
#endif  
  
   if(SwdGetLifeTime(s8LedDog) == 0)
  {
    SwdReset(s8LedDog);
    bLedFlag = ~bLedFlag;
        
    if(bLedFlag == 0)
      SystemRunEnable();
    else
      SystemRunDisable();
  }

}

/***************************************************************************
//函数名称：LpEnterCfg
//功能描述：进入低功耗管脚配置
//
//参数：无
//返回：无
***************************************************************************/
void LpEnterCfg(void)
{
  UART0_SEL &=  ~(UART0_TXD + UART0_RXD);       // 选择端口做UART通信端口
  UART0_DIR |=   (UART0_TXD + UART0_RXD);
  if(g_bCdmaSend == TRUE)
    UART0_PIN_SET();
  else
    UART0_PIN_CLR();
  
  UART1_SEL &=  ~(UART1_TXD + UART1_RXD);       // 选择端口做UART通信端口
  UART1_DIR |=   (UART1_TXD + UART1_RXD);
  UART1_PIN_CLR();
  
  UART2_SEL &=  ~(UART2_TXD + UART2_RXD);       // 选择端口做UART通信端口
  UART2_DIR |=   (UART2_TXD + UART2_RXD);
  UART2_PIN_CLR();
  
  UART3_SEL &=  ~(UART3_TXD + UART3_RXD);       // 选择端口做UART通信端口
  UART3_DIR |=   (UART3_TXD + UART3_RXD);
  UART3_PIN_CLR();
  
  I2cSclDir |= I2cScl;
  I2cSdaDir |= I2cSda;
  I2cSclOut |= I2cScl;
  I2cSdaOut |= I2cSda;
  
   if(g_bCdmaSend == TRUE)
   {
//      CdmaIcCheckDir |=  CdmaIcCheck;
//      CdmaIcCheckOut &= ~CdmaIcCheck;
      
      CdmaOnOffDir  |=  CdmaOnOff;
      CdmaOnOffOut  |=  CdmaOnOff;
      
      CdmaResetDir  |=  CdmaReset;
      CdmaResetOut  |=  CdmaReset;
   }
  else
  {
    CdmaOnOffDir  |=  CdmaOnOff;
    CdmaOnOffOut  &= ~CdmaOnOff;
    
    CdmaResetDir  |=  CdmaReset;
    CdmaResetOut  &= ~CdmaReset;
      //CDMA RI端口
    CdmaRiDir |=  CdmaRi;
    CdmaRiOut &= ~CdmaRi;
  }
}

/***************************************************************************
//函数名称：LpExitCfg
//功能描述：退出低功耗管脚配置
//
//参数：无
//返回：无
***************************************************************************/
void LpExitCfg(void)
{
  UART0_SEL |=  UART0_TXD + UART0_RXD;       // 选择端口做UART通信端口
  UART0_DIR |=  UART0_TXD;
  UART0_DIR &= ~UART0_RXD;
  
  UART1_SEL |=  UART1_TXD + UART1_RXD;       // 选择端口做UART通信端口
  UART1_DIR |=  UART1_TXD;
  UART1_DIR &= ~UART1_RXD;
  
  UART2_SEL |=  UART2_TXD + UART2_RXD;       // 选择端口做UART通信端口
  UART2_DIR |=  UART2_TXD;
  UART2_DIR &= ~UART2_RXD;
  
  UART3_SEL |=  UART3_TXD + UART3_RXD;       // 选择端口做UART通信端口
  UART3_DIR |=  UART3_TXD;
  UART3_DIR &= ~UART3_RXD;
}

