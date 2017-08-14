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

void UartCmdInit(void)
{
  CmdHandlerInit();
  
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

  while((Uart2GetRxDataLength() > 0) && (sUartCmdCb[USB_TYPE].eAssembleState != E_UART_CMD_RECEIVING_VALID))
  {
    Uart2CmdAssemble(Uart2GetByte());

    if(asTimer[TIMER_UART2].bEnable == TRUE)
    {
      asTimer[TIMER_UART2].bEnable = FALSE;
    }
  }
  
  //数据包有效，对可用的指令进行解析
    if(sUartCmdCb[USB_TYPE].eAssembleState == E_UART_CMD_RECEIVING_VALID)
    {
      u8Cmd = ConfigPackType(au8UsbBodyBuf, sUartCmdCb[USB_TYPE].u8BufDataLength);
      //建立索引表
      CreatCmdIndex(au8UsbBodyBuf);   
      //处理完相关指令，清除buf
      MEM_SET(&au8UsbBodyBuf[0], 0, sizeof(au8UsbBodyBuf));
      sUartCmdCb[USB_TYPE].eAssembleState = E_UART_CMD_RECEIVING_HEADER;
      sUartCmdCb[USB_TYPE].u8BufDataLength = 0;
    }
    else
    {
      //规定时间内无数据，指令复位，重新提取有效数据

    }
}