
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

BOOL bCdmaSend=FALSE;
BOOL bBeiDouSend=FALSE;

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
 // UINT8 u8Check=0;

  //if( TRUE == g_bOnTimeSend)
  if( TRUE == bCdmaSend )
  {
    bCdmaSend = FALSE;
    
    ptr = OSMemGet(pSmallMem, &err);
    memset(ptr, 0, BUF_SMALL_SIZE);
    
    if(TRUE == g_bDebug)
    {
      Uart3SendString("CDMA信号正常，准备发送数据!\r\n");
    }
    
    //发送数据组包
    i=0;
    ptr[i++] = '$';
    
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
    ptr[i++] = '9'; //电池电量信息
    ptr[i++] = '\r';

    AssembleCdmaMsg(&sRecordCb.au8CdmaRevNbr[0],ptr, i);
//    AssembleCdmaMsg(&sRecordCb.au8CdmaBckNbr[0],ptr, i);  //备用手机号接收机
    
    OSMemPut(pSmallMem, ptr);
    
    g_bStandby = TRUE;
  }
  else
  {
    if( TRUE == bBeiDouSend )
    { 
       bBeiDouSend = FALSE;
       
       g_bStandby = TRUE;
    }
  }
}


/****************************************************************
 * 函数名称：ComModuleSwitch
 * 功    能：判断CDMA模块信号强度，若满足发射要求则使用CDMA发送；
             否则打开北斗模块
 *
 * 参    数：无
 * 返回值  ：无
****************************************************************/
void ComModuleSwitch(void)
{
  if(TRUE == g_bGpsCompleted)
  {
    g_bGpsCompleted = FALSE;
    
    if( GetCdmaCSQ() > 16 )
    {
      bCdmaSend = TRUE;
    }
    else
    {
      //打开北斗模块电源
      BeiDouPowerOn();
      bBeiDouSend = TRUE;
    }
  }
}