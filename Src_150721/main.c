/*****************************************************************************************************
*											
* Copyright (C) 2015, Sunic-ocean
* All Rights Reserved										
*
* 文 件 名： Main.c
* 描    述：

=====================================================================================================
                                    版本记录
=====================================================================================================
* 创建日期： 2015年4月10日14:23:50
* 作    者： Bob
* 当前版本： V1.1
* 描    述： 1、根据新版示位标的需求，砍掉显示屏部分，将DTU的网络传输模式改为北斗和短信的双模通讯。
             2、显示部分使用4位LED分别指示系统运行，低压报警。设备类型指示（围油栏 or 收油机）。
    

*****************************************************************************************************/
#define  MAIN_GLOBAL
#include "msp430x54x.h"
#include "BatteryDetect.h"
#include "global.h"
#include "DataType.h"
#include "SystemCtl.h"
#include "Timer.h"
#include "SoftTimer.h"
#include "Uart0.h"
#include "Uart1.h"
#include "Uart2.h"
#include "Uart3.h"
#include "GpsParse.h"
#include "OS_MEM.h"
#include "Rtc.h"
#include "CdmaModule.h"
#include "Communication.h"
#include "DebugCmd.h"
#include "Record.h"
#include "BeiDouModule.h"

/*
*********************************************************************************************************
*                                              MAIN FUNCTION
*********************************************************************************************************
*/
void main(void)
{
    
    StopWatchDog();           // 关闭看门狗
    
    CPU_ENTER_CRITICAL();

    InitClk(1);                //初始化时钟，DCO倍频到4Mhz
    
    SystemInit();             //端口初始化
   
    TimerA_Init();            //定时器A0初始化
    
    Uart3Init();              //UART3初始化

    CPU_EXIT_CRITICAL();
    
    Adc_Init();               //打开AD检测
    
//    WaitSystemStart();        //等待开机按键
    
    DebugCmdInit();
    
    Uart0Init();              //串口0初始化
    CdmaInit();
    
    Uart1Init();              //串口1初始化
    BdModuleInit();          
    
    Uart2Init();              //串口2初始化
    GpsCmdInit();
    
    CommunicationInit();      //通讯数据结构初始化
    
    RtcInit();                //RTC初始化
    
    RecordInit();             //配置记录初始化
    
    SystemIdleInit();         //程序运行初始化
    
    while(TRUE)
    {     
      FreeWatchDog();          //释放看门狗
 
      GpsCmdRun();            //GPS数据获取
      
      BatteryDetect();        //AD检测任务
      
      SendInfoToCenter();     //发送数据到中心
      
      SystemIdleRun();        //系统空闲任务
      
      BdCmdRun();             //北斗命令解析
      
      EnterLowPower();        //进入低功耗，等待唤醒
      
      DebugCmdRun();          //调试及设置信息解析
          
      RecordRun();            //参数保存    
      
//      RtcRun();
    }

}









