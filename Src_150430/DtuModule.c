
/******************************************************************************
*											
* Copyright (C) 2014, Sunic-ocean
* All Rights Reserved										
*
* 文 件 名： CdmaModule.C
* 描    述： CDMA数据解析模块

* 创建日期： 2015年4月24日10:15:40
* 作    者： Bob
* 当前版本： V1.00

------------------------------------------------------
*******************************************************************************/
#include "msp430x54x.h"

#include "CdmaModule.h"
#include "DataType.h"
#include "Uart0.h"
#include "SystemCtl.h"
#include "string.h"
#include "Timer.h"
#include "SoftTimer.h"
#include "GpsParse.h"
#include "stdio.h"

//前面的强制类型转化缺省会造成计算错误
#define _GetWordFromAddr(pAddr)         (UINT16)(((UINT16)pAddr[0]) << 8) + pAddr[1]

#define _GetDoubleWordFromAddr(pAddr)   ((UINT32)(((UINT32)pAddr[0]) << 24) +    \
                                         (UINT32)(((UINT32)pAddr[1]) << 16) +    \
                                         (UINT32)(((UINT32)pAddr[2]) << 8) + pAddr[3])

//进入DTU配置模式
const UINT8 u8ConfigMode[] = {"enter_config_mode\0"};
const UINT8 u8ConfigStart[] = {0xff,0xff,0xff,0xff};
const UINT8 u8ConfigRxData[] = {"aaaaaaaaaaaaaaaa\0"};

const UINT8 au8BackOk[3] = {0x4f, 0x4b, 0x00};

//串口波特率设置
const UINT8 au8UartSet[] = {"AT+UART=9600,ON,8,N,1\r"};

#if 1
/*****************************************************************************
 函 数 名  : Delay
 功能描述  : 在12MHz晶振 1T下 延时时间约等于usTime ms
 输入参数  : USHORT16 usTime
 输出参数  : NONE
 返 回 值  :
*****************************************************************************/
static void Delay(UINT16 usTime)
{
    UINT16 j;
    UINT32 i;

    for (i = usTime; i > 0; i--)
    {
        for(j = 110; j > 0; j--);
    }
}

/*****************************************************************************
 函 数 名  : CdmaInit
 功能描述  : CDMA模块初始化

 输入参数  : none
 返 回 值  : none,
*****************************************************************************/
void CdmaInit(void)
{
  CdmaResetDir |= CdmaReset;
  CdmaOnOffDir |= CdmaOnOff;
  //cdma模块复位
  SetCdmaReset(0);
  g_u16TimerDelay=0;
  while(g_u16TimerDelay < 20);    //低电平延时20ms
  SetCdmaReset(1);
  //cdma模块开机
  SetCdmaOnOff(1);
  g_u16TimerDelay=0;
  while(g_u16TimerDelay < 10);
  SetCdmaOnOff(0);
  g_u16TimerDelay=0;
  while(g_u16TimerDelay < 2000);
  SetCdmaOnOff(1);
}


