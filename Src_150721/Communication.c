
/******************************************************************************
*											
* Copyright (C) 2015, Sunic-ocean
* All Rights Reserved										
*
* 文 件 名： CommunicationModule.C
* 描    述： 通讯模块，将获取的GPS北斗定位信息打包成所需的数据格式
             获取当前CDMA的信号强度，若达到发射强度，使用CDMA模块发送信息给中心
             否则，使用北斗模块发送报文，关掉CDMA模块。

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
  sSendInfo.u8Index = 0;
  sSendInfo.pGPS_Y = NULL;
  sSendInfo.pGPS_X = NULL;
  sSendInfo.u8Battery = 0; 
  sSendInfo.u8GapTime =  1;      //默认为5分钟发送间隔
  sSendInfo.u8Minute  =  sSendInfo.u8GapTime * 5;
  
  sSendInfo.sDog = SwdGet();
  SwdSetName(sSendInfo.sDog, "OpenModuleTimer");
  SwdSetLifeTime(sSendInfo.sDog, 45000);
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
	
  if(g_bSendInfo == TRUE)
  {
    if(TRUE == g_bGpsCompleted)
    {
      g_bSendInfo     = FALSE;
      g_bGpsCompleted = FALSE;
      
      ptr = OSMemGet(pSmallMem, &err);
//      memset(ptr, 0, (BUF_SMALL_SIZE-1));
  
      //发送数据组包
      i=0;
      ptr[i++] = '#';                        //信息开始符
      
      ptr[i++] = sRecordCb.u16SysNbr/10000 + '0';
      ptr[i++] = sRecordCb.u16SysNbr%10000/1000 + '0';
      ptr[i++] = sRecordCb.u16SysNbr%1000/100 + '0';
      ptr[i++] = sRecordCb.u16SysNbr%100/10 + '0';
      ptr[i++] = sRecordCb.u16SysNbr%10 + '0';
      ptr[i++] = ',';
      memcpy(&ptr[i], sSendInfo.pGPS_Y, 10);
      i += 10;
      ptr[i++] = ',';
      memcpy(&ptr[i], sSendInfo.pGPS_X, 11);
      i += 11;
      ptr[i++] = ',';
      ptr[i++] = sSendInfo.u8GapTime + '0';		// 1 ~6代表5鍉30分钟
      ptr[i++] = ',';
      ptr[i++] = sSendInfo.u8Battery + '0'; //电池电量信息
      ptr[i++] = '#';                       //信息结束符
  
      if(g_bCdmaSend == TRUE)
      {
//        AssembleCdmaMsg(&sRecordCb.au8CdmaRevNbr[0], ptr, i);
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

