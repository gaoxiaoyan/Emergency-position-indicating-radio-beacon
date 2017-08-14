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

#include "global.h"
#include "DataType.h"
#include "SystemCtl.h"
#include "Timer.h"
#include "Uart3.h"
#include "SoftTimer.h"
#include "Uart2.h"
#include "GpsParse.h"
#include "OS_MEM.h"

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

/*========================== 主函数 ===================================*/
void main(void)
{
  SINT8 s8LedDog=0;
  BOOL bFlag=0;
  
    StopWatchDog();           // 关闭看门狗
    
    CPU_ENTER_CRITICAL();

    InitClk();                //初始化时钟，DCO倍频到4Mhz
      
    OS_MemInit();             //内存初始化
    
    SystemInit();             //端口初始化
   
    TimerA_Init();            //定时器A0初始化
    
    Uart3Init();              //UART3初始化

    CPU_EXIT_CRITICAL();
    
    WaitSystemStart();        //等待开机按键
    
    Uart2Init();              //串口2初始化
    
    GpsCmdInit();
    
    s8LedDog = SwdGet();
    SwdSetLifeTime(s8LedDog, 500);
    SwdEnable(s8LedDog);
    
//    SystemIdleInit();         //统计任务初始化
      
    while(TRUE)
    {     
      FreeWatchDog();          //释放看门狗
 
      if(SwdGetLifeTime(s8LedDog) == 0)
      {
        SwdReset(s8LedDog);
        bFlag = ~bFlag;
        
        if(bFlag == 0)
        {
          SystemRunEnable();
        }
        else
        {
          SystemRunDisable();
        }
      }
      
      GpsCmdRun();
      
    }

}









