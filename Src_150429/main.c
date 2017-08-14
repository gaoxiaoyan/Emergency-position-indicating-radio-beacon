/******************************************************************************
*											
* Copyright (C) 2015, Sunic-ocean
* All Rights Reserved										
*
* 文 件 名： Main.c
* 描    述：

===============================================================================
                                    版本记录
================================================================================
* 创建日期： 2015年4月10日14:23:50
* 作    者： Bob
* 当前版本： V1.1
* 描    述： 1、根据新版示位标的需求，砍掉显示屏部分，将DTU的网络传输模式改为北斗和短信的双模通讯。
             2、显示部分使用4位LED分别指示系统运行，低压报警。设备类型指示（围油栏 or 收油机）。
    

*******************************************************************************/

#include "msp430x54x.h"
#include "BatteryDetect.h"
#include "global.h"
#include "DataType.h"
#include "SystemCtl.h"
#include "Timer.h"
#include "Uart3.h"
#include "SoftTimer.h"
#include "Uart0.h"
#include "Uart2.h"
#include "GpsParse.h"
#include "OS_MEM.h"
#include "Rtc.h"
#include "CdmaModule.h"
#include "Communication.h"

/*
*********************************************************************************************************
*                                              GLOBALS PARAM
*********************************************************************************************************
*/

//调试信息开关
BOOL g_bDebug=TRUE;

volatile UINT8  g_u8TimerDelay;
volatile UINT16 g_u16TimerDelay;
volatile UINT32 g_u32TimeStamp;

_tsSendInfo sSendInfo;

/*========================== 主函数 ===================================*/
void main(void)
{
    StopWatchDog();           // 关闭看门狗
    
    CPU_ENTER_CRITICAL();

    InitClk();                //初始化时钟，DCO倍频到4Mhz
    
    SystemInit();             //端口初始化
   
    TimerA_Init();            //定时器A0初始化
    
    Uart3Init();              //UART3初始化

    CPU_EXIT_CRITICAL();
    
    WaitSystemStart();        //等待开机按键
    
    Uart0Init();              //串口0初始化
    CdmaInit();
    
    Uart2Init();              //串口2初始化
    GpsCmdInit();
    
    CommunicationInit();      //通讯数据结构初始化
    
    Adc_Init();               //打开AD检测
    
    RtcInit();                //RTC初始化
    
    SystemIdleInit();         //程序运行初始化
      
    while(TRUE)
    {     
      FreeWatchDog();          //释放看门狗
 
      GpsCmdRun();            //GPS数据获取
      
      ComModuleSwitch();      //通讯链路切换
      
      BatteryDetect();        //AD检测任务
      
      SendInfoToCenter();     //发送数据到中心
      
      SystemIdleRun();        //系统空闲任务
      
      EnterLowPower();        //进入低功耗，等待唤醒
    }

}









