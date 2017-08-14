/******************************************************************************
*											
* Copyright (C) 2014, Sunic-ocean
* All Rights Reserved										
*
* 文 件 名： UartCmd.h
* 描       述：

* 创建日期： 2014年12月5日11:31:59
* 作    者： Bob
* 当前版本： V1.00

------------------------------------------------------
*******************************************************************************/
#include "msp430x54x.h"

#include "global.h"
#include "DataType.h"
#include "UartCmd.h"
#include "SystemCtl.h"

//接收命令数据结构定义
typedef struct
{
  UINT8 u8Head_1;
  UINT8 u8Head_2;
  UINT8 u8Cmd;
  UINT8 u8Tail_1;
  UINT8 u8Tail_2;
}_tsUartCmdCb sUartCmd;

//定义通讯命令列表
typedef void CmdHandler(void);
CmdHandler *apCmdHandlerList[32];

/***************************************************************************
//函数名称：PrintfCmdList
//功能描述：打印命令列表
//
//参数：无
//返回：无
***************************************************************************/
void PrintfCmdList(void)
{
  Uart3SendString("###############################################################\r\n");
  Uart3SendString("#            应急物资示位标内部调试指令       \r\n");
  Uart3SendString("#==============================================================\r\n");
  Uart3SendString("#  0、命令格式：AA 55 cmd CC 33.  \r\n");
  Uart3SendString("#  1、cmd<10> -- GPS调试信息关闭；\r\n");
  Uart3SendString("#  2、cmd<11> -- GPS调试信息打开；\r\n");
  Uart3SendString("#  3、cmd<20> -- BeiDou调试信息关闭；\r\n");
  Uart3SendString("#  4、cmd<21> -- BeiDou调试信息打开；\r\n");
  Uart3SendString("#  5、cmd<22> -- BeiDou发送报文测试；\r\n");
  Uart3SendString("#  6、cmd<30> -- CDMA调试信息关闭；\r\n");
  Uart3SendString("#  7、cmd<31> -- CDMA调试信息打开；\r\n");
  Uart3SendString("#  8、cmd<32> -- 获取CDMA时间；\r\n");
  Uart3SendString("#  9、cmd<33> -- 将CDMA时间校准RTC；\r\n");
  Uart3SendString("# 10、cmd<34> -- 获取本机号码；\r\n");
  Uart3SendString("# 11、cmd<35> -- 设置接收号码；\r\n");
  Uart3SendString("# 12、cmd<40> -- 设置通讯间隔时间；\r\n");
  Uart3SendString("# 13、cmd<50> -- 获取当前电池状态；\r\n");
  Uart3SendString("###############################################################\r\n");
}


/*****************************************************************************
 *函数名称: GetSystemVersionInfo
 *功能描述: 打印系统版本信息

 *参数: 
 *返回:
*****************************************************************************/
void GetSystemVersionInfo(void)
{
  UINT8 au8Version[8];

//  if(g_bDebug == TRUE)
  {
    Uart3SendString("\r\n#Debug HandShake OK! The Bandrate is 115200bps.\r\n");
    Uart3SendString("/***********************************************************/\r\n");
    Uart3SendString("// Copyright (C) 2015, Sunic-ocean.\r\n");
    Uart3SendString("// All Rights Reserved.\r\n");
    Uart3SendString(" \r\n");
    Uart3SendString("// Project Name：应急物资示位标.\r\n");
      
    memset(&au8Version[0], 0, 8);
    au8Version[0] = 'V';
    au8Version[1] = VERSION / 100 + '0';
    au8Version[2] = '.';
    au8Version[3] = VERSION % 100 / 10 + '0';
    au8Version[4] = VERSION % 10 + '0';
    au8Version[5] = '\r';
  
    Uart3SendString("// Version：");
    Uart3SendString(&au8Version[0]);
    Uart3SendString("/***********************************************************/\r\n");
    Uart3SendString(" \r\n");
  }
}

/***************************************************************************
//函数名称：CmdHandlerInit
//功能描述：命令列表清零
//
//参数：无
//返回：无
***************************************************************************/
void CmdHandlerInit(void)
{
  //函数指针清零
  memset(apCmdHandlerList, 0, sizeof(apCmdHandlerList));
  
  apCmdHandlerList[PRINTF_ALL_CMD]  = PrintfCmdList;       //打印命令列表
//  apCmdHandlerList[GPS_DEBUG_CLOSE] = GpsDebugClose;    //关闭GPS的debug显示
//  apCmdHandlerList[GPS_DEBUG_OPEN]  = GpsDebugOpen;     //打开GPS的debug显示
//  apCmdHandlerList[BD_DEBUG_CLOSE]  = BeiDouDebugClose; //北斗通讯数据关闭
//  apCmdHandlerList[BD_DEBUG_OPEN]   = BeiDouDebugOpen;  //北斗通讯数据打开
  apCmdHandlerList[CDMA_TIME_GET]    = GetCdmaTime;       //获取CDMA时间
  apCmdHandlerList[CDMA_TIME_TO_RTC] = SetCdmaTimeToRtc;  //把CDMA时间设置到rtc中
  apCmdHandlerList[CDMA_LOCAL_NUM]   = SetCdmaLocalPhoneNum;  //设置本机号码
  apCmdHandlerList[CDMA_RECEIVE_NUM] = SetCdmaReceivePhoneNum;  //设置接收机号码
}

/***********************************************************************************************
  函数名称：GpsCmdAssemble(UINT8 u8InData)
  函数功能：把接收的数据组合成有效的数据包

  输入：u8InData
  输出：无
***********************************************************************************************/
void DebugCmdAssemble(UINT8 u8InData)
{
  switch(sDebugCmdCb.eAssembleState)
  {
    case E_UART_CMD_RECEIVING_HEADER1:
    {
      if(u8InData == 0xAA)
      {
        sDebugCmdCb.pu8AssembleBuf[0] = u8InData;
        sDebugCmdCb.u8BufDataLen = 1;
	      sDebugCmdCb.eAssembleState = E_UART_CMD_RECEIVING_HEADER2;	
      }
      break;
    }	
     
    case E_UART_CMD_RECEIVING_HEADER2:
    {
      if(u8InData == 0x55)
      {
        sDebugCmdCb.pu8AssembleBuf[sDebugCmdCb.u8BufDataLen++] = u8InData;
	      sDebugCmdCb.eAssembleState = E_UART_CMD_RECEIVING_CMD;	
      }
      break;
    }	
		
    case E_UART_CMD_RECEIVING_CMD:
    {
      sDebugCmdCb.pu8AssembleBuf[sDebugCmdCb.u8BufDataLen++] = u8InData;
      sDebugCmdCb.eAssembleState = E_UART_CMD_RECEIVING_TAIL1;
    
      break;
    }
     
    case E_UART_CMD_RECEIVING_TAIL1:
    {
      if(u8InData == 0xCC)
      {
        sDebugCmdCb.pu8AssembleBuf[sDebugCmdCb.u8BufDataLen++] = u8InData;
        sDebugCmdCb.eAssembleState = E_UART_CMD_RECEIVING_TAIL2;
      }
    
      break;
    }
     
    case E_UART_CMD_RECEIVING_TAIL2:
    {
      if(u8InData == 0x33)
      {
        sDebugCmdCb.pu8AssembleBuf[sDebugCmdCb.u8BufDataLen++] = u8InData;
        sDebugCmdCb.eAssembleState = E_UART_CMD_RECEIVING_VALID;
      }
    
      break;
    }
		
    case E_UART_CMD_RECEIVING_VALID:
    { 
      break;
    }
		
    default:
    {
      sDebugCmdCb.eAssembleState = E_UART_CMD_RECEIVING_HEADER;
      sDebugCmdCb.u8BufDataLen = 0;
    }
  }	
}

/**********************************************************************************************
 函数名称：GpsCmdInit
 函数功能：GPS cmd parse init

 参数： 无
 返回： 无
**********************************************************************************************/
void GpsCmdInit(void)
{
  //初始化命令列表
  CmdHandlerInit();
  
  //  sGpsCmdCb.bGpsParse = FALSE;
  sDebugCmdCb.u16BufDataLen = 0;
  sDebugCmdCb.eAssembleState = E_UART_CMD_RECEIVING_HEADER1;
  sDebugCmdCb.pu8AssembleBuf = OSMemGet(pSmallMem, &err);

  //Get One soft timer
  sDebugCmdCb.sGpsDog = SwdGet();   
  SwdSetLifeTime(sDebugCmdCb.sDebugDog, 200);
}

/***************************************************************************
//函数名称：UartCmdRun
//功能描述：串口接收命令解析
//
//参数：无
//返回：无
***************************************************************************/
void DebugCmdRun(void)
{
  UINT8 u8Cmd;

  while((Uart3GetRxDataLength() > 0) && (sDebugCmdCb.eAssembleState != E_UART_CMD_RECEIVING_VALID))
  {
    DebugCmdAssemble(Uart3GetByte());

    if(SwdEnable(sDebugCmdCb.sGpsDog) == TRUE)
    {
      SwdDisable(sDebugCmdCb.sGpsDog);
    }
  }
  
  //数据包有效，对可用的指令进行解析
  if(sGpsCmdCb.eAssembleState == E_UART_CMD_RECEIVING_VALID)
  {
  
  }
}