/******************************************************************************
*											
* Copyright (C) 2014, Sunic-ocean
* All Rights Reserved										
*
* 文 件 名： GpsParse.C
* 描    述：GPS数据解析

* 创建日期： 2014年9月5日11:31:59
* 作    者： Bob
* 当前版本： V1.00

------------------------------------------------------
*******************************************************************************/

#define GPS_GLOBALS
#include "GpsParse.h"
#include "string.h"
#include "ctype.h"
#include "global.h"
#include "Timer.h"
#include "Uart2.h"
#include "Uart3.h"
#include "SystemCtl.h"
#include "OS_MEM.h"
#include "SoftWatchDog.h"

#define GPS_BUF_SIZE    BUF_LARGE_SIZE
//定义GPS接收缓冲区
//UINT8 au8GpsBodyBuf[MAX_BODY_SIZE];

//GPS命令类型
const UINT8 *pGpsCmd[] = {"$GNGGA", "$GNGLL","$GNRMC", "$GNVTG"};
//GPS 命令类型
enum eNmeaType
{
  GNNON = 0x00,
  GNGGA = 0x01,
  GNGLL = 0x02,
  GNRMC = 0x04,
  GNVTG = 0x08
};

//一年内月份列表
const UINT8 au8Leap[13] = {0, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
const UINT8 au8Year[13] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

#define RtcHex2Bcd(u8Hex) ((((u8Hex % 100) / 10) << 4) + ((u8Hex % 100) % 10))
#define Bcd2Hex(u8Bcd) ((((u8Bcd & 0xf0) >> 4) % 10) * 10 + ((u8Bcd & 0x0f) % 10))


/**********************************************************************************************
 函数名称：NmeaPackType
 函数功能：GPS 命令解析

 参数： 无
 返回： 0183协议命令类型
**********************************************************************************************/
SINT8 NmeaPackType(const UINT8 *pu8Buf, UINT8 u8Size)
{
  if(u8Size < 6)
    return GNNON;
  else if(0 == memcmp(pu8Buf, pGpsCmd[0], 6))
    return GNGGA;
  else if(0 == memcmp(pu8Buf, pGpsCmd[1], 6))
    return GNGLL;
  else if(0 == memcmp(pu8Buf, pGpsCmd[2], 6))
    return GNRMC;

  return GNNON;
}

/***********************************************************************************************
  函数名称：GpsCmdAssemble(UINT8 u8InData)
  函数功能：把接收的数据组合成有效的数据包

  输入：u8InData
  输出：无
***********************************************************************************************/
void GpsCmdAssemble(UINT8 u8InData)
{
  switch(sGpsCmdCb.eAssembleState)
  {
    case E_UART_CMD_RECEIVING_HEADER:
    {
      if(u8InData == '$')
      {
        sGpsCmdCb.pu8AssembleBuf[0] = u8InData;
        sGpsCmdCb.u16BufDataLen = 1;
	      sGpsCmdCb.eAssembleState = E_UART_CMD_RECEIVING_BODY;	
      }
      break;
     }	
		
    case E_UART_CMD_RECEIVING_BODY:
    {
      sGpsCmdCb.pu8AssembleBuf[sGpsCmdCb.u16BufDataLen++] = u8InData;
      if(u8InData == '\r')
      {
        sGpsCmdCb.eAssembleState = E_UART_CMD_RECEIVING_VALID;
      }
      else
      {
        if(sGpsCmdCb.u16BufDataLen == GPS_BUF_SIZE)	
        {
          sGpsCmdCb.eAssembleState = E_UART_CMD_RECEIVING_HEADER;
          sGpsCmdCb.u16BufDataLen = 0;
          break;
         }
        }
        break;
      }
		
    case E_UART_CMD_RECEIVING_VALID:
    {
      break;
    }
		
    default:
    {
      sGpsCmdCb.eAssembleState = E_UART_CMD_RECEIVING_HEADER;
      sGpsCmdCb.u16BufDataLen = 0;
    }
  }	
}

/**********************************************************************************************
 函数名称：GpsCmdInit
 函数功能：GPS 命令解析初始化

 参数： 无
 返回： 无
**********************************************************************************************/
void GpsCmdInit(void)
{
  UINT8 err1=0;
  UINT8 err2=0;
  
   sGpsCmdCb.bGpsParse = FALSE;
   sGpsCmdCb.u16BufDataLen = 0;
   sGpsCmdCb.u16RxDataLen = 0;
   sGpsCmdCb.eAssembleState = E_UART_CMD_RECEIVING_HEADER;
   sGpsCmdCb.pu8AssembleBuf = OSMemGet(pSmallMem, &err1);
   sGpsCmdCb.pu8RxDataBuf = OSMemGet(pLargeMem, &err2);
   sGpsCmdCb.sGpsDog = SwdGet();
   SwdSetLifeTime(sGpsCmdCb.sGpsDog, 200);
     
   if(g_bDebug == TRUE)
   {
    if(err1 == OS_NO_ERR)
    {
      Uart3SendString("#GPS组包内存分配成功！\r\n");
    }
    else
    {
      Uart3SendString("#GPS组包内存分配失败！\r\n");
    }
    
    if(err2 == OS_NO_ERR)
    {
      Uart3SendString("#GPS数据备份内存分配成功！\r\n");
    }
    else
    {
      Uart3SendString("#GPS数据备份内存分配失败！\r\n");
    }
   }
}


/**********************************************************************************************
 函数名称：GpsCmdRun
 函数功能：GPS 命令解析，连续采集8组有效数据，找出卫星质量好的一组数据

 参数： 无
 返回： 无
**********************************************************************************************/
void GpsCmdRun(void)
{
  UINT8 u8Cmd=0;
  static UINT16 i=0;
  
  if(Uart2RxTimeoutCheck() == TRUE)
  {
    sGpsCmdCb.u16RxDataLen = Uart2GetRxDataLength();
    memset(sGpsCmdCb.pu8RxDataBuf, 0, sizeof(GPS_BUF_SIZE));
    Uart2GetRxData(sGpsCmdCb.pu8RxDataBuf, sGpsCmdCb.u16RxDataLen);
    sGpsCmdCb.bGpsParse = TRUE;
  }
  
  if(sGpsCmdCb.bGpsParse == TRUE)
  {
    //获取有效数据包
    while((sGpsCmdCb.eAssembleState != E_UART_CMD_RECEIVING_VALID) && (sGpsCmdCb.u16RxDataLen > 0))	
    {
      GpsCmdAssemble(sGpsCmdCb.pu8RxDataBuf[i++]);
      sGpsCmdCb.u16RxDataLen--;
      if(sGpsCmdCb.u16RxDataLen == 0)
      {
        i = 0;
      }

      if(SwdEnable(sGpsCmdCb.sGpsDog) == TRUE)
      {
        SwdDisable(sGpsCmdCb.sGpsDog);
      }
    }
    
    if(sGpsCmdCb.eAssembleState == E_UART_CMD_RECEIVING_VALID)
    {
      u8Cmd = NmeaPackType(sGpsCmdCb.pu8AssembleBuf, sGpsCmdCb.u16BufDataLen);
      if(u8Cmd != GNNON)
      {
        if(g_bDebug == TRUE)
        {
          Uart3SendBuffer(sGpsCmdCb.pu8AssembleBuf, sGpsCmdCb.u16BufDataLen);
        }
        
      }
      
      sGpsCmdCb.eAssembleState = E_UART_CMD_RECEIVING_HEADER;
      sGpsCmdCb.u16BufDataLen = 0;
    }
  }
}
