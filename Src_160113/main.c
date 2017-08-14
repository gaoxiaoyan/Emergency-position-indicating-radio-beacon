/*
*****************************************************************************************************
*
* Copyright (C) 2015, Sunic-ocean
* All Rights Reserved
*
* 文 件 名： Main.c
* 描    述：

=====================================================================================================
                                    版本记录
=====================================================================================================
* 创建日期： 2015年7月21日09:57:22
* 作    者： Bob
* 当前版本： V1.01
* 描    述： 1、硬件架构上做了修改，使用4节并联锂电池供电，单片机，gps和CDMA使用LDO降压；而北斗使用升压
                方案升到额定电压。
             2、因为后期在出厂时在围油栏和收油机上绑定程序固化，则不需要进行设置，去掉指示区分
             3、将之前的开关机按键改为自锁开关。防止系统代码跑飞复位后的开机问题。
             4、根据目前的PCB设计更改了部分控制管脚。
             5、北斗和GPS通讯接口对调，目前北斗使用UART2，GPS使用UART1.

-----------------------------------------------------------------------------------------------------
* 创建日期： 2015年4月10日14:23:50
* 作    者： Bob
* 当前版本： V1.00
* 描    述： 1、根据新版示位标的需求，砍掉显示屏部分，将DTU的网络传输模式改为北斗和短信的双模通讯。
             2、显示部分使用4位LED分别指示系统运行，低压报警。设备类型指示（围油栏 or 收油机）。


*****************************************************************************************************
*/

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

//    SystemInit();             //端口初始化

    //程序运行指示灯
    SystemRunEnable();
    SystemRunDir |= SystemRun;

    InitClk(1);                //初始化时钟，DCO倍频到4Mhz

    SystemInit();             //端口初始化

    TimerA_Init();            //定时器A0初始化

    Uart2Init();              //UART2初始化

    CPU_EXIT_CRITICAL();

    Adc_Init();               //打开AD检测

#if KEY_SCAN_EN > 0
    WaitSystemStart();        //等待开机按键
#endif

    DebugCmdInit();

    Uart0Init();              //串口0初始化
//    CdmaInit();

    Uart3Init();              //串口3初始化
//    BdModuleInit();

    Uart1Init();              //串口1初始化
    GpsCmdInit();

    RtcInit();                //RTC初始化

    RecordInit();             //配置记录初始化

    CommunicationInit();      //通讯数据结构初始化

    SystemIdleInit();         //程序运行初始化

//    AdcDetect(1);

    while(TRUE)
    {
      FreeWatchDog();          //释放看门狗

      GpsCmdRun();            //GPS数据获取任务

      BatteryDetect();        //AD检测任务

      SendInfoToCenter();     //通讯任务

      SystemIdleRun();        //显示指示灯任务

//      BdCmdRun();             //北斗命令解析任务

//      CdmaCmdRun();           //CDMA模块命令解析任务

      EnterLowPower();        //进入低功耗，等待唤醒

      DebugCmdRun();          //调试及设置信息解析

      RecordRun();            //参数保存

//      RtcRun();
    }

}









