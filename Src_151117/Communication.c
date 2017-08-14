
/******************************************************************************
*
* Copyright (C) 2015, Sunic-ocean
* All Rights Reserved
*
* 文 件 名： CommunicationModule.C
* 描    述： 通讯模块，将获取的GPS北斗定位信息打包成所需的数据格式
             获取当前CDMA的信号强度，若达到发射强度，使用CDMA模块发送信息给中心
             否则，使用北斗模块发送报文，关掉CDMA模块。

//-----------------------------------------------------------------------------
* 修改日期：2015年10月30日17:03:09
* 作    者：Bob
* 修改描述：1、修改发送格式，增加北纬和东经指示；将之前的‘#’结束符改为'&'

//-----------------------------------------------------------------------------
* 创建日期： 2015年4月27日14:51:26
* 作    者： Bob
* 当前版本： V1.00

------------------------------------------------------
*******************************************************************************/
#define  COMMU_GLOBAL
#include "msp430x54x.h"
#include "global.h"
#include "Communication.h"
#include "DataType.h"
#include "CdmaModule.h"
#include "SystemCtl.h"
#include "string.h"
#include "Timer.h"
#include "SoftTimer.h"
#include "GpsParse.h"
#include "OS_MEM.h"
#include "Uart3.h"
#include "Record.h"
#include "BeiDouModule.h"

SINT8 s8TestDog=0;

/*****************************************************************************
 函 数 名  : CommunicationInit
 功能描述  : 发送数据结构初始化

 输入参数  : none
 返 回 值  : none,
*****************************************************************************/
void CommunicationInit(void)
{
//  UINT8 u8Temp=0;

  sSendInfo.u8Index = 0;
  sSendInfo.pGPS_Y = NULL;
  sSendInfo.pGPS_X = NULL;
  sSendInfo.u8Battery = 0;
//  sSendInfo.u8GapTime =  1;      //默认为5分钟发送间隔

  sSendInfo.u8Minute  = (sRecordCb.u8GapTime - '0') * 5;
  sSendInfo.u8GapTime = sRecordCb.u8GapTime;

  sSendInfo.sDog = SwdGet();
  SwdSetName(sSendInfo.sDog, "OpenModuleTimer");
  SwdSetLifeTime(sSendInfo.sDog, 40000);
  SwdEnable(sSendInfo.sDog);     //启动开启通讯模块定时器

  s8TestDog = SwdGet();
  SwdSetName(s8TestDog, "TestTimer");
  SwdSetLifeTime(s8TestDog, 5000);

  u16BdSendNum = 0;  //北斗发送报文次数
  u16BdRxNum   = 0;    //北斗接收报文次数
  u8BdCentage  = 0;      //成功率
}


/*****************************************************************************
 函 数 名  : SendInfoToCenter
 功能描述  : 发送数据到中心，

 输入参数  : none
 返 回 值  : none,
*****************************************************************************/
void SendInfoToCenter(void)
{
	UINT8 *ptr;
  UINT8 i;

#if 0
  UINT8 au8Y[10]    = {0x31,0x32,0x33,0x34,0x35,0x31,0x32,0x33,0x34,0x35};
  UINT8 au8X[11]    = {0x35,0x34,0x33,0x32,0x31,0x35,0x34,0x33,0x32,0x31,0x30};

  g_bSendInfo       = 0x55;
  g_bGpsCompleted   = TRUE;
  g_bBdSend         = TRUE;
//  g_bCdmaSend       = FALSE;

  sSendInfo.pGPS_Y  = &au8Y[0];
  sSendInfo.pGPS_X  = &au8X[0];

  StopWatchDog();
#endif

  if(g_bSendInfo == TRUE)
  {
    if(TRUE == g_bGpsCompleted)
    {
      g_bSendInfo     = FALSE;
      g_bGpsCompleted = FALSE;

      //获取一块内存空间
      ptr = OSMemGet(pSmallMem, &err);

      //发送数据组包
      i=0;
      ptr[i++] = '#';                        //信息开始符

      ptr[i++] = sRecordCb.u16CmpNbr/10000 + '0';
      ptr[i++] = sRecordCb.u16CmpNbr%10000/1000 + '0';
      ptr[i++] = sRecordCb.u16CmpNbr%1000/100 + '0';
      ptr[i++] = sRecordCb.u16CmpNbr%100/10 + '0';
      ptr[i++] = sRecordCb.u16CmpNbr%10 + '0';

      ptr[i++] = ',';
      ptr[i++] = sRecordCb.u16SysNbr/10000 + '0';
      ptr[i++] = sRecordCb.u16SysNbr%10000/1000 + '0';
      ptr[i++] = sRecordCb.u16SysNbr%1000/100 + '0';
      ptr[i++] = sRecordCb.u16SysNbr%100/10 + '0';
      ptr[i++] = sRecordCb.u16SysNbr%10 + '0';

      ptr[i++] = ',';
      ptr[i++] = sRecordCb.u8SysType % 100 / 10 + '0';
      ptr[i++] = sRecordCb.u8SysType % 10 + '0';

      ptr[i++] = ',';
      ptr[i++] = 'N';
      ptr[i++] = ',';
      memcpy(&ptr[i], sSendInfo.pGPS_Y, 10);
      i += 10;
      ptr[i++] = ',';
      ptr[i++] = 'E';
      ptr[i++] = ',';
      memcpy(&ptr[i], sSendInfo.pGPS_X, 11);
      i += 11;
      ptr[i++] = ',';
      ptr[i++] = sRecordCb.u8GapTime;		// 1 ~ 6 代表5到30分钟(ASCII格式)
      ptr[i++] = ',';
      ptr[i++] = (sSendInfo.u8Battery%100)/10 + '0'; //电池电量信息
      ptr[i++] = sSendInfo.u8Battery%10 + '0'; //电池电量信息
//      ptr[i++] = '#';                       //信息结束符
      ptr[i++] = '&';                       //信息结束符, 更改为目前与

      if(g_bCdmaSend == TRUE)
      {
        AssembleCdmaMsg(&sRecordCb.au8CdmaRevNbr[0], ptr, i);
      }
      else
      {
        if(g_bBdSend == TRUE)
        {
          GetBeiDouIcNum(&sRecordCb.au8BdRevNbr[0]);
//          sRecordCb.au8BdRevNbr[0] = 0x1f;
//          sRecordCb.au8BdRevNbr[1] = 0xf9;
//          sRecordCb.au8BdRevNbr[2] = 0xe8;
          BdSendCmdTXSQ(&sRecordCb.au8BdRevNbr[0], ptr, i);

#ifdef BEIDOU_TEST
          u16BdSendNum++;
          memset(&au8BdSendInfo[0], 0, sizeof(au8BdSendInfo));
          memcpy(&au8BdSendInfo[0], ptr, i);
#endif
        }
      }

      //使用完毕，释放该内存
      OSMemPut(pSmallMem, ptr);

       SwdReset(s8TestDog);
       SwdEnable(s8TestDog);
    }
  }

  if(SwdGetLifeTime(s8TestDog) == 0)
  {
    SwdDisable(s8TestDog);
    SwdReset(s8TestDog);

    g_bStandby = TRUE;
  }

  //提前一分钟唤醒通讯模块，当CDMA通讯时接收到短信直接唤醒
//  if((g_bCommuWake == TRUE) || (g_bCdmaRiEvent == TRUE))
  if(SwdGetLifeTime(sSendInfo.sDog) == 0)
  {
//    g_bCommuWake   = FALSE;
//    g_bCdmaRiEvent = FALSE;

    SwdDisable(sSendInfo.sDog);
    SwdReset(sSendInfo.sDog);

    if(g_bCdmaSend == TRUE)
    {
      CdmaWakeUpCfg();
//      AssembleCdmaMsg("18561795199", "This Is A Test Massage!", 23);
    }
    else
    {
      if(g_bBdSend == TRUE)
      {
        BeiDouPowerOn();
      }
    }
  }
}

