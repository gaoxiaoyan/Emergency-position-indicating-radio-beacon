/******************************************************************************
*
* Copyright (C) 2015, Sunic-ocean
* All Rights Reserved
*
* 文 件 名： DebugCmd.c
* 描       述：

* 创建日期： 2015年4月29日16:41:25
* 作    者： Bob
* 当前版本： V1.00

------------------------------------------------------
*******************************************************************************/
#include "msp430x54x.h"
#include "BatteryDetect.h"
#include "global.h"
#include "DataType.h"
#include "DebugCmd.h"
#include "SystemCtl.h"
#include "Uart2.h"
#include "string.h"
#include "SoftTimer.h"
#include "CdmaModule.h"
#include "Rtc.h"
#include "Record.h"
#include "stdio.h"
#include "stdlib.h"
#include "ctype.h"
#include "Communication.h"
#include "BeiDouModule.h"

//数据包解析协议
typedef enum
{
  //命令帧头
  E_UART_CMD_RECEIVING_HEADER1,
  E_UART_CMD_RECEIVING_HEADER2,

  //接收命令
  E_UART_CMD_RECEIVING_CMD,

  //命令帧尾
  E_UART_CMD_RECEIVING_TAIL1,
  E_UART_CMD_RECEIVING_TAIL2,

  //数据有效
  E_UART_CMD_RECEIVING_VALID,

}teDebugCmdAssembleState;

//BSL下载命令
const UINT8 au8DownloadCmd[6] = {0x80, 0x01, 0x00, 0x15, 0x64, 0xa3};

/* 数据组包结构声明 */
typedef struct
{
  UINT8 *pu8AssembleBuf;  //组包区数据指针

  UINT8 u8BufDataLen ;  //组包的数据长度

  teDebugCmdAssembleState eAssembleState;  //组包状态

  SINT8 sDebugDog;

}_tsDebugCmdCb;

_tsDebugCmdCb sDebugCmdCb;

//定义通讯命令列表
typedef void CmdHandler(void);
CmdHandler *apCmdHandlerList[64];

const UINT8 au8LastTime[10] = __TIME__"\r\n";
const UINT8 au8LastDate[14] = __DATE__",  ";

/***************************************************************************
*函数名称：PrintfCmdList
*功能描述：打印命令列表

*参数：无
*返回：无

 发送： 0xAA + 0x55 + 0x02 + 0xCC + 0x33
***************************************************************************/
void PrintfCmdList(void)
{
  Uart2SendString("###############################################################\r\n");
  Uart2SendString("# Copyright (C) 2015, Sunic-ocean.\r\n");
  Uart2SendString("# All Rights Reserved.\r\n");
  Uart2SendString("# \r\n");
  Uart2SendString("# Discription：内部调试指令集.\r\n");
  Uart2SendString("#  \r\n");
  Uart2SendString("#==============================================================\r\n");
  g_u8TimerDelay=0;
  while(g_u8TimerDelay < 15);    //低电平延时150ms
  Uart2SendString("#  0、命令格式： AA 55 cmd CC 33.  \r\n");
  Uart2SendString("#  1、cmd<00> -- 串口通讯握手命令；\r\n");
  Uart2SendString("#  2、cmd<01> -- 获取软件版本信息；\r\n");
  Uart2SendString("#  3、cmd<02> -- 打印调试指令列表；\r\n");
  Uart2SendString("#  4、cmd<03> -- 设置设备 RTC时间；\r\n");

  Uart2SendString("#  5、cmd<04> -- 获取设备 RTC时间；\r\n");
  Uart2SendString("#  6、cmd<05> -- 获取当前剩余电量；\r\n");
  Uart2SendString("#  7、cmd<06> -- 设置通讯间隔时间；\r\n");
  Uart2SendString("#  8、cmd<07> -- 获取通讯间隔时间；\r\n");
  Uart2SendString("#  9、cmd<08> -- 设置本台设备编码；\r\n");
  g_u8TimerDelay=0;
  while(g_u8TimerDelay < 15);    //低电平延时150ms
  Uart2SendString("# 10、cmd<09> -- 获取本台设备编码；\r\n");
  Uart2SendString("# 11、cmd<0a> -- 清除程序运行时间；\r\n");
  Uart2SendString("# 12、cmd<0b> -- 获取程序运行时间；\r\n");
  Uart2SendString("# 13、cmd<0c> -- 使用北斗模块通讯；\r\n");
  Uart2SendString("# 14、cmd<0d> -- 使用CDMA模块通讯；\r\n");

  Uart2SendString("# 15、cmd<10> -- GPS 调试信息关闭；\r\n");
  Uart2SendString("# 16、cmd<11> -- GPS 调试信息打开；\r\n");

  Uart2SendString("# 17、cmd<20> -- 北斗定位获取命令；\r\n");
  Uart2SendString("# 18、cmd<21> -- 北斗本机卡号获取；\r\n");
  Uart2SendString("# 19、cmd<22> -- 北斗接收机号设置；\r\n");
  g_u8TimerDelay=0;
  while(g_u8TimerDelay < 15);    //低电平延时150ms
  Uart2SendString("# 20、cmd<23> -- 北斗接收机号获取；\r\n");
  Uart2SendString("# 21、cmd<24> -- 北斗接收备用卡号设置；\r\n");
  Uart2SendString("# 22、cmd<25> -- 北斗接收备用卡号获取；\r\n");
  Uart2SendString("# 23、cmd<26> -- 北斗报文测试命令；\r\n");
  Uart2SendString("# 24、cmd<27> -- 北斗时间获取命令；\r\n");
  Uart2SendString("# 25、cmd<28> -- 北斗发送成功率获取；\r\n");
  Uart2SendString("# 26、cmd<29> -- 北斗发送成功率清零；\r\n");

  Uart2SendString("# 27、cmd<30> -- CDMA调试信息关闭；\r\n");
  Uart2SendString("# 28、cmd<31> -- CDMA调试信息打开；\r\n");
  Uart2SendString("# 29、cmd<32> -- 通过CDMA获取时间；\r\n");
  g_u8TimerDelay=0;
  while(g_u8TimerDelay < 15);    //低电平延时150ms
  Uart2SendString("# 30、cmd<33> -- 设置CDMA本机号码；\r\n");
  Uart2SendString("# 31、cmd<34> -- 获取CDMA本机号码；\r\n");
  Uart2SendString("# 32、cmd<35> -- 设置CDMA接收号码；\r\n");
  Uart2SendString("# 33、cmd<36> -- 获取CDMA接收号码；\r\n");
  Uart2SendString("# 34、cmd<37> -- 设置CDMA接收备用号码；\r\n");
  Uart2SendString("# 35、cmd<38> -- 获取CDMA接收备用号码；\r\n");
  Uart2SendString("# 36、cmd<39> -- 获取CDMA信号强度；\r\n");
  Uart2SendString("# 37、cmd<3A> -- CDMA短信发送测试；\r\n");


  Uart2SendString("###############################################################\r\n\r\n");
}

/*****************************************************************************
 *函数名称: UartHandShake
 *功能描述: 串口握手命令

 *参数:
 *返回:

发送： 0xAA + 0x55 + 0x00 + 0xCC + 0x33
返回：
*****************************************************************************/
void UartHandShake(void)
{
  Uart2SendString("###############################################################\r\n");
  Uart2SendString("# Debug HandShake OK! The Bandrate is 115200bps.\r\n");
  Uart2SendString("###############################################################\r\n\r\n");
}

/*****************************************************************************
 *函数名称: GetSystemVersionInfo
 *功能描述: 打印系统版本信息

 *参数:
 *返回:

发送： 0xAA + 0x55 + 0x01 + 0xCC + 0x33
*****************************************************************************/
void GetSystemVersionInfo(void)
{
  UINT8 au8Version[8];
  UINT8 au8Time[10];
  UINT8 au8Date[14];

  memcpy(&au8Time[0], &au8LastTime[0], 10);
  memcpy(&au8Date[0], &au8LastDate[0], 14);

//  if(g_bDebug == TRUE)
  {
    Uart2SendString("###############################################################\r\n");
    memset(&au8Version[0], 0, 8);
    au8Version[0] = 'V';
    au8Version[1] = VERSION / 100 + '0';
    au8Version[2] = '.';
    au8Version[3] = VERSION % 100 / 10 + '0';
    au8Version[4] = VERSION % 10 + '0';
    au8Version[5] = '\r';

    Uart2SendString("# The Current Version  Is: ");
    Uart2SendString(&au8Version[0]);
    Uart2SendString("# The Compilation Time Is: ");
    Uart2SendBuffer(&au8Date[0], 14);
    Uart2SendBuffer(&au8Time[0], 10);
    Uart2SendString("###############################################################\r\n\r\n");
    Uart2SendString(" \r\n");
  }
}


/*****************************************************************************
 *函数名称: SetRtcTime
 *功能描述: 设置RTC时间

 *参数:
 *返回:

发送： 0xAA + 0x55 + 0x03 + 0xCC + 0x33
*****************************************************************************/
void SetRtcTime(void)
{
  Uart2SendString("###############################################################\r\n");
  if( SetCdmaTimeToRtc() )
  {
    Uart2SendString("# Set Rtc Time Successful! \r\n");
  }
  else
  {
    Uart2SendString("# Set Rtc Time Failed! \r\n");
  }
  Uart2SendString("###############################################################\r\n\r\n");
}
/*****************************************************************************
 *函数名称: GetTimeFromRtc
 *功能描述: 获取RTC时间

 *参数:
 *返回:

发送： 0xAA + 0x55 + 0x04 + 0xCC + 0x33
*****************************************************************************/
void GetRtcTime(void)
{
  _tsTimeCb sTime;
  UINT8 au8Time[24];
  UINT8 i=0;

  GetTimeFromRtc(&sTime);
  au8Time[i++] = '2';
  au8Time[i++] = '0';
  au8Time[i++] = sTime.Year / 10 + '0';
  au8Time[i++] = sTime.Year % 10 + '0';
  au8Time[i++] = '/';

  au8Time[i++] = sTime.Month / 10 + '0';
  au8Time[i++] = sTime.Month % 10 + '0';
  au8Time[i++] = '/';

  au8Time[i++] = sTime.Date / 10 + '0';
  au8Time[i++] = sTime.Date % 10 + '0';
  au8Time[i++] = ',';

  au8Time[i++] = sTime.Week % 10 + '0';
  au8Time[i++] = ',';

  au8Time[i++] = sTime.Hour / 10 + '0';
  au8Time[i++] = sTime.Hour % 10 + '0';
  au8Time[i++] = ':';

  au8Time[i++] = sTime.Minute / 10 + '0';
  au8Time[i++] = sTime.Minute % 10 + '0';
  au8Time[i++] = ':';

  au8Time[i++] = sTime.Second / 10 + '0';
  au8Time[i++] = sTime.Second % 10 + '0';
  au8Time[i++] = '\r';
  au8Time[i++] = '\n';

  Uart2SendString("###############################################################\r\n");
  Uart2SendString("# The Current RTC Time Is: ");
  Uart2SendBuffer(&au8Time[0], i);
  Uart2SendString("###############################################################\r\n\r\n");
}

/*****************************************************************************
 *函数名称: GetBatteryRemainPower
 *功能描述: 获取当前电量信息

 *参数:
 *返回:

发送： 0xAA + 0x55 + 0x05 + 0xCC + 0x33
*****************************************************************************/
void GetBatteryRemainPower(void)
{
  Uart2SendString("###############################################################\r\n");
  AdcDetect(1);
  Uart2SendString("###############################################################\r\n\r\n");
}

/*****************************************************************************
 *函数名称: SetSendInfoGapTime
 *功能描述: 设置信息发送时间间隔

 *参数:
 *返回:

发送： 0xAA + 0x55 + 0x06 + 0xCC + 0x33
*****************************************************************************/
void SetSendInfoGapTime(void)
{
  SINT8 s8Dog=-1;
  UINT8 u8Temp=0;

  Uart2EmptyRxBuf();
  Uart2SendString("###############################################################\r\n");
  Uart2SendString("# Please Enter The Gap Time: Index <--> Minutes.\r\n");
  Uart2SendString("#  1 <-->  5 Minutes. \r\n");
  Uart2SendString("#  2 <--> 10 Minutes. \r\n");
  Uart2SendString("#  3 <--> 15 Minutes. \r\n");
  Uart2SendString("#  4 <--> 20 Minutes. \r\n");
  Uart2SendString("#  5 <--> 25 Minutes. \r\n");
  Uart2SendString("#  6 <--> 30 Minutes. \r\n");
//  Uart2SendString("#  7 <--> 35 Minutes. \r\n");
//  Uart2SendString("#  8 <--> 40 Minutes. \r\n");

  s8Dog = SwdGet();
  if(s8Dog < 0)
  {
    Uart2SendString("SetSendInfoGap TimeSoftTimer Is Out Of Scope!\r\n");
    return;
  }
  SwdSetLifeTime(s8Dog, 20000);
  SwdEnable(s8Dog);

  //关闭看门狗
  StopWatchDog();
  while(TRUE)
  {
    if(Uart2GetRxDataLength()>0)
    {
      Uart2GetRxData(&u8Temp, 1);

      if((u8Temp >= '1') && (u8Temp <= '6'))
      {
        SwdDisable(s8Dog);
        SwdFree(s8Dog);
        FreeWatchDog();          //释放看门狗
        break;
      }
      else
      {
        Uart2SendString("# The Enter Index Is Beyond! Please Resend The Cmd!\r\n");
        Uart2SendString("###############################################################\r\n\r\n");
        SwdDisable(s8Dog);
        SwdFree(s8Dog);
        FreeWatchDog();          //释放看门狗
        return;
      }
    }

    if(SwdGetLifeTime(s8Dog) == 0)
    {
      SwdDisable(s8Dog);
      SwdFree(s8Dog);
      FreeWatchDog();          //释放看门狗
      Uart2SendString("# No Valid Number, Please Resend The Cmd! \r\n");
      Uart2SendString("###############################################################\r\n\r\n");
      return;
    }
  }

  Uart2SendString("# System Gap Time Set Successfull! \r\n");
  Uart2SendString("###############################################################\r\n\r\n");

  sRecordCb.u8GapTime = u8Temp;
  g_u8NeedSaved  |=  SAVE_SYS_GAP;
}

/*****************************************************************************
 *函数名称: GetSendInfoGapTime
 *功能描述: 获取当前信息发送时间间隔

 *参数:
 *返回:

发送： 0xAA + 0x55 + 0x07 + 0xCC + 0x33
*****************************************************************************/
void GetSendInfoGapTime(void)
{
  UINT8 au8Buf[16];
  UINT8 i=0;
  UINT8 u8Temp=0;

  if((sRecordCb.u8GapTime >= '1') && (sRecordCb.u8GapTime <= '6'))
  {
    u8Temp = sRecordCb.u8GapTime - '0';
    u8Temp = u8Temp * 5;

    Uart2SendString("###############################################################\r\n");
    au8Buf[i++] = u8Temp % 100 / 10 + '0';
    au8Buf[i++] = u8Temp % 10 + '0';
    memcpy(&au8Buf[i], "Minute.", 7);
    i += 7;
    au8Buf[i++] = '\r';
    au8Buf[i++] = '\n';

    Uart2SendString("The System Gap Time Is: ");
    Uart2SendBuffer(&au8Buf[0], i);
    Uart2SendString("###############################################################\r\n\r\n");
  }
  else
  {
    Uart2SendString("###############################################################\r\n");
    Uart2SendString("The System Gap Time Get Error!\r\n");
    Uart2SendString("###############################################################\r\n\r\n");
  }

}

/*****************************************************************************
 *函数名称: SetSystemIndex
 *功能描述: 设置本台设备编码

 *参数:
 *返回:

发送： 0xAA + 0x55 + 0x08 + 0xCC + 0x33
*****************************************************************************/
void SetSystemIndex(void)
{
  SINT8 s8Dog;
  UINT8 au8Nbr[12];
  UINT16 u16CmpNbr;     //公司编号
  UINT16 u16SysNbr;     //设备编码

  Uart2EmptyRxBuf();
  Uart2SendString("###############################################################\r\n");
  Uart2SendString("# Please Enter The Index Of This System: \r\n");
  Uart2SendString("#  00001~60000,00001~60000(Compnay Nbr, System Nbr)\r\n");

  s8Dog = SwdGet();
  if(s8Dog < 0)
  {
    Uart2SendString("SetSystemIndex SoftTimer Is Out Of Scope!\r\n");
    return;
  }
  SwdSetLifeTime(s8Dog, 20000);
  SwdEnable(s8Dog);

  memset(au8Nbr, 0, 8);
  //关闭看门狗
  StopWatchDog();

  while(TRUE)
  {
    if(Uart2GetRxDataLength()>10)
    {
      Uart2GetRxData(&au8Nbr[0], 11);

      if((memcmp(&au8Nbr[0], "65535", 5)>0) && (memcmp(&au8Nbr[6], "65535", 5)>0))
      {
        Uart2SendString("# The Enter Index Is Beyond! Please Resend The Cmd!\r\n\r\n");
        SwdDisable(s8Dog);
        SwdFree(s8Dog);
        FreeWatchDog();          //释放看门狗
        return;
      }
      else
      {
        SwdDisable(s8Dog);
        SwdFree(s8Dog);
        FreeWatchDog();          //释放看门狗
        break;
      }
    }

    if(SwdGetLifeTime(s8Dog) == 0)
    {
      SwdDisable(s8Dog);
      SwdFree(s8Dog);
      FreeWatchDog();          //释放看门狗
      Uart2SendString("# No Valid Number, Please Resend The Cmd! \r\n");
      Uart2SendString("###############################################################\r\n\r\n");
      return;
    }
  }

  //提取公司编号
  u16CmpNbr  = (au8Nbr[0 ] - '0') * 10000;
  u16CmpNbr += (au8Nbr[1 ] - '0') * 1000;
  u16CmpNbr += (au8Nbr[2 ] - '0') * 100;
  u16CmpNbr += (au8Nbr[3 ] - '0') * 10;
  u16CmpNbr += (au8Nbr[4 ] - '0') ;

  //提取设备编号
  u16SysNbr  = (au8Nbr[6 ] - '0') * 10000;
  u16SysNbr += (au8Nbr[7 ] - '0') * 1000;
  u16SysNbr += (au8Nbr[8 ] - '0') * 100;
  u16SysNbr += (au8Nbr[9 ] - '0') * 10;
  u16SysNbr += (au8Nbr[10] - '0') ;

  if((u16CmpNbr != sRecordCb.u16CmpNbr) || (u16SysNbr != sRecordCb.u16SysNbr))
  {
    sRecordCb.u16CmpNbr = u16CmpNbr;
    sRecordCb.u16SysNbr = u16SysNbr;
    g_u8NeedSaved |= SAVE_SYS_NBR;
  }

  Uart2SendString("# System Index Set Successfull! \r\n");
  Uart2SendString("###############################################################\r\n\r\n");
}

/*****************************************************************************
 *函数名称: GetSystemIndex
 *功能描述: 获取本台设备编码

 *参数:
 *返回:

发送： 0xAA + 0x55 + 0x09 + 0xCC + 0x33
*****************************************************************************/
void GetSystemIndex(void)
{
  UINT8 au8Index[16];

  au8Index[0] = sRecordCb.u16CmpNbr/10000 + '0';
  au8Index[1] = sRecordCb.u16CmpNbr%10000/1000 + '0';
  au8Index[2] = sRecordCb.u16CmpNbr%1000/100 + '0';
  au8Index[3] = sRecordCb.u16CmpNbr%100/10 + '0';
  au8Index[4] = sRecordCb.u16CmpNbr%10 + '0';

  au8Index[5] = ',';

  au8Index[6] = sRecordCb.u16SysNbr/10000 + '0';
  au8Index[7] = sRecordCb.u16SysNbr%10000/1000 + '0';
  au8Index[8] = sRecordCb.u16SysNbr%1000/100 + '0';
  au8Index[9] = sRecordCb.u16SysNbr%100/10 + '0';
  au8Index[10] = sRecordCb.u16SysNbr%10 + '0';

  au8Index[11] = '\r';
  au8Index[12] = '\n';

  Uart2SendString("###############################################################\r\n");
  Uart2SendString("# The Index Of This System Is: ");
  Uart2SendBuffer(&au8Index[0], 13);
  Uart2SendString("###############################################################\r\n\r\n");
}

/*****************************************************************************
 *函数名称: SystemRunTimeClear
 *功能描述: 清除程序运行时间

 *参数:
 *返回:

发送： 0xAA + 0x55 + 0x0a + 0xCC + 0x33
*****************************************************************************/
void SystemRunTimeClear(void)
{
  sRecordCb.u32SysRunTime = 0;
  SaveSystemRunTime(&sRecordCb.u32SysRunTime);
  Uart2SendString("###############################################################\r\n");
  Uart2SendString(" System Run Time To Be Cleared\r\n");
  Uart2SendString("###############################################################\r\n\r\n");
}

/*****************************************************************************
 *函数名称: SystemRunTimeGet
 *功能描述: 获取程序运行时间

 *参数:
 *返回:

发送： 0xAA + 0x55 + 0x0b + 0xCC + 0x33
*****************************************************************************/
void SystemRunTimeGet(void)
{
  UINT8 au8Minute[12];
  UINT8 i=0;
  UINT32 RunTime;

  memset(&au8Minute[0], ' ', 12);
  ReadSystemRunTime(&RunTime);
  sprintf(&au8Minute[0], "%d", RunTime);

  while((au8Minute[i] != 0) && (i<12))
  {
    i++;
  }

  Uart2SendString("###############################################################\r\n");
  Uart2SendString("# System Has Been Running Minute : ");
  Uart2SendBuffer(&au8Minute[0], i);
  Uart2SendString("\r\n");
  Uart2SendString("###############################################################\r\n\r\n");
}

/*****************************************************************************
 *函数名称: UseBeiDouMode
 *功能描述: 使用北斗通讯模式

 *参数:
 *返回:

发送： 0xAA + 0x55 + 0x0c + 0xCC + 0x33
*****************************************************************************/
void UseBeiDouMode(void)
{
    if(g_bBdSend == TRUE)
    {
        Uart2SendString("Current Mode Is Using BeiDou Module!\r\n");
        return;
    }

    CdmaToBeiDou();
}

/*****************************************************************************
 *函数名称: UseCdmaMode
 *功能描述: 使用CDMA通讯模式

 *参数:
 *返回:

发送： 0xAA + 0x55 + 0x0d + 0xCC + 0x33
*****************************************************************************/
void UseCdmaMode(void)
{
    if(g_bCdmaSend == TRUE)
    {
        Uart2SendString("Current Mode Is Using CDMA Module!\r\n");
        return;
    }

    BeiDouToCdma();
}

/*****************************************************************************
 *函数名称: GpsDebugClose
 *功能描述: 关闭GPS调试信息

 *参数:
 *返回:

发送： 0xAA + 0x55 + 0x10 + 0xCC + 0x33
*****************************************************************************/
void CmdGpsDebugClose(void)
{
  g_bDebug &= ~GPS_DEBUG;
  Uart2SendString("###############################################################\r\n");
  Uart2SendString("# Gps Debug Close Successfull!\r\n ");
  Uart2SendString("###############################################################\r\n\r\n");
}

/*****************************************************************************
 *函数名称: GpsDebugOpen
 *功能描述: 打开GPS调试信息

 *参数:
 *返回:

发送： 0xAA + 0x55 + 0x11 + 0xCC + 0x33
*****************************************************************************/
void CmdGpsDebugOpen(void)
{
  g_bDebug |=  GPS_DEBUG;
  Uart2SendString("###############################################################\r\n");
  Uart2SendString("# Gps Debug Open Successfull!\r\n ");
  Uart2SendString("###############################################################\r\n\r\n");
}


/*****************************************************************************
 *函数名称: CmdBeiDouDebugClose
 *功能描述: 关闭BeiDou调试信息

 *参数:
 *返回:

发送： 0xAA + 0x55 + 0x20 + 0xCC + 0x33
*****************************************************************************/
void CmdBeiDouDebugClose(void)
{
  g_bDebug &= ~BD_DEBUG;
  Uart2SendString("###############################################################\r\n");
  Uart2SendString("# BeiDou Debug Close Successfull!\r\n ");
  Uart2SendString("###############################################################\r\n\r\n");
}

void CmdIntToString(UINT32 u32Temp, UINT8 *ptr, UINT8 *u8Len)
{
  UINT8 au8Buf[10];
  UINT8 i,n,j=0;

  i=0;
  do
  {
    au8Buf[i++] = u32Temp%10 + '0';
  }while((u32Temp /= 10) > 0);

  *u8Len = i;
  n=0;
  for(j=(i-1); j>0; j--)
  {
    ptr[n++] = au8Buf[j];
  }
  ptr[n] = au8Buf[j];
}
/*****************************************************************************
 *函数名称: CmdBeiDouLocNumGet
 *功能描述: 北斗本机卡号获取

 *参数: 无
 *返回: 无

发送： 0xAA + 0x55 + 0x21 + 0xCC + 0x33
*****************************************************************************/
void CmdBeiDouLocNumGet(void)
{
  UINT8 u8IcNum[3];
  UINT8 u8DisNum[12];
  UINT8 i;
  UINT32 u32Addr=0;

  GetBeiDouIcNum(&u8IcNum[0]);
  u32Addr  = ((UINT32)u8IcNum[0]) << 16;
  u32Addr += ((UINT32)u8IcNum[1]) << 8;
  u32Addr +=  (UINT32)u8IcNum[2];

  memset(&u8DisNum[0], 0, 12);
  CmdIntToString(u32Addr, &u8DisNum[0], &i);

  Uart2SendString("###############################################################\r\n");
  Uart2SendString("# BeiDou Location Number Is: ");
  Uart2SendBuffer(&u8DisNum[0], i);
  Uart2SendString("\r\n");
  Uart2SendString("###############################################################\r\n\r\n");
}

UINT32 My_atoi(UINT8 *ptr, UINT8 u8Len)
{
    UINT8 i=0;
    UINT32 u32Temp=0;

    for(i=0; i<u8Len; i++)
    {
        u32Temp = u32Temp * 10 + *(ptr+i) - '0';
    }

    return u32Temp;
}

/*****************************************************************************
 *函数名称: CmdBeiDouRecNumSet
 *功能描述: 北斗接收机卡号设置

 *参数:
 *返回:

发送： 0xAA + 0x55 + 0x22 + 0xCC + 0x33
*****************************************************************************/
void CmdBeiDouRecNumSet(void)
{
//  UINT8 u8IcNum[3];
  UINT8 u8DisNum[12];
  UINT8 i=0;
  UINT32 u32Addr=0;
  SINT8 sDog1, sDog2;

  memset(&u8DisNum[0], 0, 12);

  sDog1 = SwdGet();             /* 定时20秒，超时直接返回主循环 */
  if(sDog1 < 0)
  {
    Uart2SendString("CmdBeiDouRecNumSet SoftTimer Is Out Of Scope!\r\n\r\n");
    return;
  }
  SwdSetLifeTime(sDog1, 20000);
  SwdEnable(sDog1);

  sDog2 = SwdGet();
  if(sDog2 < 0)
  {
    Uart2SendString("CmdBeiDouRecNumSet SoftTimer Is Out Of Scope!\r\n\r\n");
    return;
  }
  SwdSetLifeTime(sDog2, 500);
//  SwdEnable(s8Dog2);

  Uart2SendString("###############################################################\r\n");
  Uart2SendString("# Please Input BeiDou Receiving Number: 0 ~ 16777215 \r\n"); //北斗卡号为3个byte表示，最高为此数

  //关闭看门狗
  StopWatchDog();

  while(TRUE)
  {
    if(Uart2GetRxDataLength()>0)
    {
      u8DisNum[i++] = Uart2GetByte();
      SwdEnable(sDog2);
    }

    if(SwdGetLifeTime(sDog2) == 0)
    {
      SwdDisable(sDog2);
      SwdFree(sDog2);

      Uart2SendBuffer(&u8DisNum[0], i);
      Uart2SendString(".\r\n");

      break;
    }

    if(SwdGetLifeTime(sDog1) == 0)
    {
      SwdDisable(sDog1);
      SwdFree(sDog1);

      SwdDisable(sDog2);
      SwdFree(sDog2);

      FreeWatchDog();          //释放看门狗
      Uart2SendString("# No Valid Number, Please Resend The Cmd! \r\n");
      Uart2SendString("###############################################################\r\n\r\n");
      return;
    }
  }

  u32Addr = (UINT32)My_atoi(&u8DisNum[0], i);
  sRecordCb.au8BdRevNbr[0] = (UINT8)(u32Addr >> 16);
  sRecordCb.au8BdRevNbr[1] = (UINT8)(u32Addr >> 8);
  sRecordCb.au8BdRevNbr[2] = (UINT8)(u32Addr);

  Uart2SendString("# BeiDou Receiving Number Set Successfull! \r\n");
  Uart2SendString("###############################################################\r\n\r\n");

  g_u8NeedSaved |= SAVE_BD_NBR;

  SwdDisable(sDog1);
  SwdFree(sDog1);
  FreeWatchDog();          //释放看门狗
}

/*****************************************************************************
 *函数名称: CmdBeiDouRecNumGet
 *功能描述: 北斗接收机卡号获取

 *参数:
 *返回:

发送： 0xAA + 0x55 + 0x23 + 0xCC + 0x33
*****************************************************************************/
void CmdBeiDouRecNumGet(void)
{
  UINT8 u8IcNum[3];
  UINT8 u8DisNum[10];
  UINT8 i;
  UINT32 u32Addr=0;

  GetBeiDouSendNum(&u8IcNum[0]);
  u32Addr  = ((UINT32)u8IcNum[0]) << 16;
  u32Addr += ((UINT32)u8IcNum[1]) << 8;
  u32Addr +=  (UINT32)u8IcNum[2];

  memset(&u8DisNum[0], 0, 10);
  CmdIntToString(u32Addr, &u8DisNum[0], &i);

  Uart2SendString("###############################################################\r\n");
  Uart2SendString("# BeiDou Receiving Number Is: ");
  Uart2SendBuffer(&u8DisNum[0], i);
  Uart2SendString("\r\n");
  Uart2SendString("###############################################################\r\n\r\n");
}

/*****************************************************************************
 *函数名称: CmdBeiDouBckRecNumSet
 *功能描述: 北斗接收机卡号设置

 *参数:
 *返回:

发送： 0xAA + 0x55 + 0x24 + 0xCC + 0x33
*****************************************************************************/
void CmdBeiDouBckRecNumSet(void)
{
//  UINT8 u8IcNum[3];
  UINT8 u8DisNum[12];
  UINT8 i=0;
  UINT32 u32Addr=0;
  SINT8 sDog1, sDog2;

  memset(&u8DisNum[0], 0, 12);

  sDog1 = SwdGet();             /* 定时20秒，超时直接返回主循环 */
  if(sDog1 < 0)
  {
    Uart2SendString("CmdBeiDouBckRecNumSet SoftTimer Is Out Of Scope!\r\n\r\n");
    return;
  }
  SwdSetLifeTime(sDog1, 20000);
  SwdEnable(sDog1);

  sDog2 = SwdGet();
  if(sDog2 < 0)
  {
    Uart2SendString("CmdBeiDouBckRecNumSet SoftTimer Is Out Of Scope!\r\n\r\n");
    return;
  }
  SwdSetLifeTime(sDog2, 500);
//  SwdEnable(s8Dog2);

  Uart2SendString("###############################################################\r\n");
  Uart2SendString("# Please Input BeiDou Back Receiving Number: 0 ~ 16777215 \r\n"); //北斗卡号为3个byte表示，最高为此数

  //关闭看门狗
  StopWatchDog();

  while(TRUE)
  {
    if(Uart2GetRxDataLength()>0)
    {
      u8DisNum[i++] = Uart2GetByte();
      SwdEnable(sDog2);
    }

    if(SwdGetLifeTime(sDog2) == 0)
    {
      SwdDisable(sDog2);
      SwdFree(sDog2);

      Uart2SendBuffer(&u8DisNum[0], i);
      Uart2SendString(".\r\n");

      break;
    }

    if(SwdGetLifeTime(sDog1) == 0)
    {
      SwdDisable(sDog1);
      SwdFree(sDog1);

      SwdDisable(sDog2);
      SwdFree(sDog2);

      FreeWatchDog();          //释放看门狗
      Uart2SendString("# No Valid Number, Please Resend The Cmd! \r\n");
      Uart2SendString("###############################################################\r\n\r\n");
      return;
    }
  }

  u32Addr = (UINT32)My_atoi(&u8DisNum[0], i);
  sRecordCb.au8BdBckNbr[0] = (UINT8)(u32Addr >> 16);
  sRecordCb.au8BdBckNbr[1] = (UINT8)(u32Addr >> 8);
  sRecordCb.au8BdBckNbr[2] = (UINT8)(u32Addr);

  Uart2SendString("# BeiDou Back Receiving Number Set Successfull! \r\n");
  Uart2SendString("###############################################################\r\n\r\n");

  g_u8NeedSaved |= SAVE_BD_NBR;

  SwdDisable(sDog1);
  SwdFree(sDog1);
  FreeWatchDog();          //释放看门狗
}

/*****************************************************************************
 *函数名称: CmdBeiDouBckRecNumGet
 *功能描述: 北斗接收机卡号获取

 *参数:
 *返回:

发送： 0xAA + 0x55 + 0x25 + 0xCC + 0x33
*****************************************************************************/
void CmdBeiDouBckRecNumGet(void)
{
  UINT8 u8IcNum[3];
  UINT8 u8DisNum[10];
  UINT8 i;
  UINT32 u32Addr=0;

  GetBeiDouBckSendNum(&u8IcNum[0]);
  u32Addr  = ((UINT32)u8IcNum[0]) << 16;
  u32Addr += ((UINT32)u8IcNum[1]) << 8;
  u32Addr +=  (UINT32)u8IcNum[2];

  memset(&u8DisNum[0], 0, 10);
  CmdIntToString(u32Addr, &u8DisNum[0], &i);

  Uart2SendString("###############################################################\r\n");
  Uart2SendString("# BeiDou Back Receiving Number Is: ");
  Uart2SendBuffer(&u8DisNum[0], i);
  Uart2SendString("\r\n");
  Uart2SendString("###############################################################\r\n\r\n");
}


/*****************************************************************************
 *函数名称: CmdBeiDouGetSendSuccess
 *功能描述: 获取发射成功率

 *参数: 无
 *返回: 无

发送： 0xAA + 0x55 + 0x26 + 0xCC + 0x33
*****************************************************************************/
void CmdBeiDouGetSendSuccess(void)
{
  UINT16 u16BdRx=0;
  UINT16 u16BdSend=0;
  UINT16 u16BdSucceed=0;
  UINT8 au8Rx[5];
  UINT8 au8Send[5];
  UINT8 au8SendSucceed[5];
  UINT8 i,j,n;

  ReadBdRxNbr(&u16BdRx);
  ReadBdSendNbr(&u16BdSend);
  ReadBdSendSucceedNbr(&u16BdSucceed);

  sprintf(&au8Rx[0], "%d", u16BdRx);
  i=0;
  while((au8Rx[i] != 0) && (i<6))
  {
    i++;
  }

  j=0;
  sprintf(&au8Send[0], "%d", u16BdSend);
  while((au8Send[j] != 0) && (j<6))
  {
    j++;
  }

  n=0;
  sprintf(&au8SendSucceed[0], "%d", u16BdSucceed);
  while((au8SendSucceed[n] != 0) && (n<6))
  {
    n++;
  }

  Uart2SendString("###############################################################\r\n");
  Uart2SendString("# BeiDou Send Massage Is: ");
  Uart2SendBuffer(&au8Send[0], j);
  Uart2SendString("\r\n");
  Uart2SendString("# BeiDou Send Succeed Massage Is: ");
  Uart2SendBuffer(&au8SendSucceed[0], n);
  Uart2SendString("\r\n");
  Uart2SendString("# BeiDou Receive Massage Is: ");
  Uart2SendBuffer(&au8Rx[0], i);
  Uart2SendString("\r\n");
  Uart2SendString("###############################################################\r\n\r\n");
}
/*****************************************************************************
 *函数名称: CmdBeiDouClrSendSuccess
 *功能描述: 清除北斗发射成功次数

 *参数: 无
 *返回: 无

发送： 0xAA + 0x55 + 0x27 + 0xCC + 0x33
*****************************************************************************/
void CmdBeiDouClrSendSuccess(void)
{
  u16BdRxNum = 0;
  u16BdSendNum = 0;
  u16BdSendSucceed = 0;

  SaveBdRxNbr(&u16BdRxNum);
  SaveBdSendNbr(&u16BdSendNum);
  SaveBdSendSucceedNbr(&u16BdSendSucceed);

  Uart2SendString("###############################################################\r\n");
  Uart2SendString(" BeiDou Send & Rx Number To Be Cleared\r\n");
  Uart2SendString("###############################################################\r\n\r\n");
}

/*****************************************************************************
 *函数名称: CmdBeiDouSendMsg
 *功能描述: 自收发报文测试

 *参数: 无
 *返回: 无

发送： 0xAA + 0x55 + 0x28 + 0xCC + 0x33
*****************************************************************************/
void CmdBeiDouSendMsg(void)
{
    UINT8 *pMsg;
    UINT8  au8Buf[24];
//    UINT8  i;
    UINT8  au8RevNbr[3];

    if(g_bBdSend != TRUE)
    {
        Uart2SendString("BeiDou Module Is Power Down, Please Turn On BD Power!\r\n\r\n");
        return;
    }

    memset(&au8Buf[0], 0, 24);
    pMsg = &au8Buf[0]; //OSMemGet(pSmallMem, &err);
    memcpy(pMsg, "01234567890123456789", 20);

    memset(&au8RevNbr[0], 0, 3);
    GetBeiDouIcNum(&au8RevNbr[0]);

    BdSendCmdTXSQ(&au8RevNbr[0], pMsg, 20);

//    OSMemPut(pSmallMem, pMsg);
}

/*****************************************************************************
 *函数名称: CmdCdmaDebugClose
 *功能描述: 关闭GPS调试信息

 *参数:
 *返回:

发送： 0xAA + 0x55 + 0x30 + 0xCC + 0x33
*****************************************************************************/
void CmdCdmaDebugClose(void)
{
  g_bDebug &= ~CDMA_DEBUG;
  Uart2SendString("###############################################################\r\n");
  Uart2SendString("# CDMA Debug Close Successfull!\r\n ");
  Uart2SendString("###############################################################\r\n\r\n");
}

/*****************************************************************************
 *函数名称: CmdCdmaDebugOpen
 *功能描述: 打开GPS调试信息

 *参数:
 *返回:

发送： 0xAA + 0x55 + 0x31 + 0xCC + 0x33
*****************************************************************************/
void CmdCdmaDebugOpen(void)
{
  g_bDebug |=  CDMA_DEBUG;
  Uart2SendString("###############################################################\r\n");
  Uart2SendString("# CDMA Debug Open Successfull!\r\n");
  Uart2SendString("###############################################################\r\n\r\n");
}

/*****************************************************************************
 *函数名称: CmdGetCdmaTime
 *功能描述: 获取CDMA时间

 *参数:
 *返回:

发送： 0xAA + 0x55 + 0x32 + 0xCC + 0x33
*****************************************************************************/
void CmdGetCdmaTime(void)
{
  UINT8 au8Time[32];
  UINT8 u8Len;

  if(g_bCdmaSend != TRUE)
  {
       Uart2SendString("CDMA Module Is Power Down, Please Turn On CDMA Power!\r\n\r\n");
       return;
   }

  Uart2SendString("###############################################################\r\n");
  Uart2SendString("# The Current CDMA Time Is: ");
  GetCdmaTime(&au8Time[0], &u8Len);
  Uart2SendBuffer(&au8Time[0], u8Len);
  Uart2SendString("\r\n");
  Uart2SendString("###############################################################\r\n\r\n");
}

/*****************************************************************************
 *函数名称: CmdSetCdmaLocationNbr
 *功能描述: 设置本机号码

 *参数:
 *返回:

发送： 0xAA + 0x55 + 0x33 + 0xCC + 0x33
*****************************************************************************/
void CmdSetCdmaLocationNbr(void)
{
  SINT8 s8Dog;
  UINT8 i=0;
  UINT8 au8Nbr[20];

  if(g_bCdmaSend != TRUE)
  {
       Uart2SendString("CDMA Module Is Power Down, Please Turn On CDMA Power!\r\n\r\n");
       return;
   }

  Uart2SendString("###############################################################\r\n");
  s8Dog = SwdGet();
  if(s8Dog < 0)
  {
    Uart2SendString("CmdSetCdmaLocationNbr SoftTimer Is Out Of Scope!\r\n\r\n");
    return;
  }
  SwdSetLifeTime(s8Dog, 25000);
  SwdEnable(s8Dog);
  //关闭看门狗
  StopWatchDog();
  Uart2SendString("# Please Enter The Location Number: ");
  while(TRUE)
  {
    if(Uart2GetRxDataLength()>10)
    {
      Uart2GetRxData(&au8Nbr[0], 11);

      for(i=0; i<11; i++)
      {
        if((au8Nbr[i] < '0') && (au8Nbr[i] > '9'))
        {
          Uart2SendString("# You Input Number Is Error, Please Resend The Cmd! \r\n");
          Uart2SendString("###############################################################\r\n\r\n");
          SwdDisable(s8Dog);
          SwdFree(s8Dog);
          FreeWatchDog();          //释放看门狗
          return;
        }
      }

      au8Nbr[11] = '\r';
      au8Nbr[12] = '\n';
      Uart2SendBuffer(&au8Nbr[0], 13);

      SwdDisable(s8Dog);
      SwdFree(s8Dog);
      FreeWatchDog();          //释放看门狗
      break;
    }


    if(SwdGetLifeTime(s8Dog) == 0)
    {
      SwdDisable(s8Dog);
      SwdFree(s8Dog);
      FreeWatchDog();          //释放看门狗
      Uart2SendString("# No Valid Number, Please Resend The Cmd! \r\n");
      Uart2SendString("###############################################################\r\n\r\n");
      return;
    }

  }

  if( SetLocationNbr(&au8Nbr[0]) )
  {
    Uart2SendString("# CDMA Location Number Set Successfull! \r\n");
  }
  else
  {
    Uart2SendString("# CDMA Location Number Set Failed! \r\n");
  }
  Uart2SendString("###############################################################\r\n\r\n");
}

/*****************************************************************************
 *函数名称: CmdGetCdmaLocationNbr
 *功能描述: 获取本机号码

 *参数:
 *返回:

发送： 0xAA + 0x55 + 0x34 + 0xCC + 0x33
*****************************************************************************/
void CmdGetCdmaLocationNbr(void)
{
  UINT8 au8Nbr[20];

  if(g_bCdmaSend != TRUE)
  {
       Uart2SendString("CDMA Module Is Power Down, Please Turn On CDMA Power!\r\n\r\n");
       return;
   }

  //关闭看门狗
  StopWatchDog();

  Uart2SendString("###############################################################\r\n");
  if( GetLocationNbr(&au8Nbr[0]) )
  {
    au8Nbr[11] = '\r';
    au8Nbr[12] = '\n';
    Uart2SendString("# CDMA Location Number Is: ");
    Uart2SendBuffer(au8Nbr, 13);
  }
  else
  {
    Uart2SendString("# CDMA Location Number Get Failed! \r\n");
  }
  Uart2SendString("###############################################################\r\n\r\n");
}

/*****************************************************************************
 *函数名称: CmdSetCdmaReceivingNbr
 *功能描述: 设置接收号码

 *参数:
 *返回:

发送： 0xAA + 0x55 + 0x35 + 0xCC + 0x33
*****************************************************************************/
void CmdSetCdmaReceivingNbr(void)
{
  //输入完成，将设置的号码保存到EEPROM
  SINT8 s8Dog;
  UINT8 i;
  UINT8 au8Nbr[20];

//  if(g_bCdmaSend != TRUE)
//  {
//       Uart2SendString("CDMA Module Is Power Down, Please Turn On CDMA Power!\r\n\r\n");
//       return;
//   }

  //关闭看门狗
  StopWatchDog();
  Uart2SendString("###############################################################\r\n");
  Uart2EmptyRxBuf();
  Uart2SendString("#Please Enter The Receiving Number: ");

  s8Dog = SwdGet();
  if(s8Dog < 0)
  {
    Uart2SendString("CmdSetCdmaReceivingNbr SoftTimer Is Out Of Scope!\r\n");
    return;
  }
  SwdSetLifeTime(s8Dog, 25000);
  SwdEnable(s8Dog);

  while(TRUE)
  {
    if(Uart2GetRxDataLength()>10)
    {
      Uart2GetRxData(&au8Nbr[0], 11);

      for(i=0; i<11; i++)
      {
        if((au8Nbr[i] < '0') && (au8Nbr[i] > '9'))
        {
          Uart2SendString("# You Input Number Is Error, Please Resend The Cmd! \r\n");
          Uart2SendString("###############################################################\r\n\r\n");
          SwdDisable(s8Dog);
          SwdFree(s8Dog);
          FreeWatchDog();          //释放看门狗
          return;
        }
      }

      au8Nbr[11] = '\r';
      au8Nbr[12] = '\n';
      Uart2SendBuffer(&au8Nbr[0], 13);
      memcpy(&sRecordCb.au8CdmaRevNbr[0], &au8Nbr[0], 11);
      Uart2SendString("# CDMA Receiving Number Set Successfull! \r\n");
      break;
    }

    if(SwdGetLifeTime(s8Dog) == 0)
    {
      SwdDisable(s8Dog);
      SwdFree(s8Dog);
      FreeWatchDog();          //释放看门狗
      Uart2SendString("# CDMA Receiving Number Set Failed! \r\n");
      Uart2SendString("###############################################################\r\n\r\n");
      return;
    }
  }
  Uart2SendString("###############################################################\r\n\r\n");
  g_u8NeedSaved |= SAVE_CDMA_NBR;
  SwdDisable(s8Dog);
  SwdFree(s8Dog);
  FreeWatchDog();          //释放看门狗
}


/*****************************************************************************
 *函数名称: CmdGetCdmaReceivingNbr
 *功能描述: 获取接收号码

 *参数:
 *返回:

发送： 0xAA + 0x55 + 0x36 + 0xCC + 0x33
*****************************************************************************/
void CmdGetCdmaReceivingNbr(void)
{
  UINT8 au8Nbr[20];

//  if(g_bCdmaSend != TRUE)
//  {
//       Uart2SendString("CDMA Module Is Power Down, Please Turn On CDMA Power!\r\n\r\n");
//       return;
//  }

  Uart2SendString("###############################################################\r\n");
  Uart2SendString("# The CDMA Receiving Number Is: ");
  ReadCdmaReceivingNbr(&au8Nbr[0]);
  au8Nbr[11] = '\r';
  au8Nbr[12] = '\n';

  Uart2SendBuffer(&au8Nbr[0], 13);
  Uart2SendString("###############################################################\r\n\r\n");
}

/*****************************************************************************
 *函数名称: CmdSetCdmaBckReceivingNbr
 *功能描述: 设置接收号码

 *参数:
 *返回:

发送： 0xAA + 0x55 + 0x37 + 0xCC + 0x33
*****************************************************************************/
void CmdSetCdmaBckReceivingNbr(void)
{
  //输入完成，将设置的号码保存到EEPROM
  SINT8 s8Dog;
  UINT8 i;
  UINT8 au8Nbr[20];

//  if(g_bCdmaSend != TRUE)
//  {
//       Uart2SendString("CDMA Module Is Power Down, Please Turn On CDMA Power!\r\n\r\n");
//       return;
//   }

  //关闭看门狗
  StopWatchDog();
  Uart2SendString("###############################################################\r\n");
  Uart2EmptyRxBuf();
  Uart2SendString("#Please Enter The Back Receiving Number: ");

  s8Dog = SwdGet();
  if(s8Dog < 0)
  {
    Uart2SendString("CmdSetCdmaBckReceivingNbr SoftTimer Is Out Of Scope!\r\n");
    return;
  }
  SwdSetLifeTime(s8Dog, 25000);
  SwdEnable(s8Dog);

  while(TRUE)
  {
    if(Uart2GetRxDataLength()>10)
    {
      Uart2GetRxData(&au8Nbr[0], 11);

      for(i=0; i<11; i++)
      {
        if((au8Nbr[i] < '0') && (au8Nbr[i] > '9'))
        {
          Uart2SendString("# You Input Number Is Error, Please Resend The Cmd! \r\n");
          Uart2SendString("###############################################################\r\n\r\n");
          SwdDisable(s8Dog);
          SwdFree(s8Dog);
          FreeWatchDog();          //释放看门狗
          return;
        }
      }

      au8Nbr[11] = '\r';
      au8Nbr[12] = '\n';
      Uart2SendBuffer(&au8Nbr[0], 13);
      memcpy(&sRecordCb.au8CdmaBckNbr[0], &au8Nbr[0], 11);
      Uart2SendString("# CDMA Back Receiving Number Set Successfull! \r\n");
      break;
    }

    if(SwdGetLifeTime(s8Dog) == 0)
    {
      SwdDisable(s8Dog);
      SwdFree(s8Dog);
      FreeWatchDog();          //释放看门狗
      Uart2SendString("# CDMA Back Receiving Number Set Failed! \r\n");
      Uart2SendString("###############################################################\r\n\r\n");
      return;
    }
  }
  Uart2SendString("###############################################################\r\n\r\n");
  g_u8NeedSaved |= SAVE_CDMA_NBR;
  SwdDisable(s8Dog);
  SwdFree(s8Dog);
  FreeWatchDog();          //释放看门狗
}


/*****************************************************************************
 *函数名称: CmdGetCdmaBckReceivingNbr
 *功能描述: 获取接收号码

 *参数:
 *返回:

发送： 0xAA + 0x55 + 0x38 + 0xCC + 0x33
*****************************************************************************/
void CmdGetCdmaBckReceivingNbr(void)
{
  UINT8 au8Nbr[20];

 // if(g_bCdmaSend != TRUE)
 // {
 //      Uart2SendString("CDMA Module Is Power Down, Please Turn On CDMA Power!\r\n\r\n");
 //      return;
 // }

  Uart2SendString("###############################################################\r\n");
  Uart2SendString("# The CDMA Receiving Number Is: ");
  ReadBckCdmaReceivingNbr(&au8Nbr[0]);
  au8Nbr[11] = '\r';
  au8Nbr[12] = '\n';

  Uart2SendBuffer(&au8Nbr[0], 13);
  Uart2SendString("###############################################################\r\n\r\n");
}


/*****************************************************************************
 *函数名称: CmdGetCdmaCSQ
 *功能描述: 获取CDMA的信号强度

 *参数:
 *返回:

发送： 0xAA + 0x55 + 0x39 + 0xCC + 0x33
*****************************************************************************/
void CmdGetCdmaCSQ(void)
{
  UINT8 u8Temp;
  UINT8 u8Num[4];

  if(g_bCdmaSend != TRUE)
  {
       Uart2SendString("CDMA Module Is Power Down, Please Turn On CDMA Power!\r\n\r\n");
       return;
   }

  u8Temp = GetCdmaCSQ();
  u8Num[0] = u8Temp / 10 + '0';
  u8Num[1] = u8Temp % 10 + '0';
  u8Num[2] = '\r';
  u8Num[3] = '\n';

  Uart2SendString("###############################################################\r\n");
  Uart2SendString("# The CDMA Current CSQ Is: ");
  Uart2SendBuffer(u8Num, 4);
  Uart2SendString("###############################################################\r\n\r\n");
}


/*****************************************************************************
 *函数名称: CmdCdmaInfoTest
 *功能描述: CDMA模块信息发送测试

 *参数:
 *返回:

发送： 0xAA + 0x55 + 0x3A + 0xCC + 0x33
*****************************************************************************/
void CmdCdmaInfoTest(void)
{
  SINT8 s8Dog;
  UINT8 i=0;
  UINT8 au8Nbr[20];

  if(g_bCdmaSend != TRUE)
  {
       Uart2SendString("CDMA Module Is Power Down, Please Turn On CDMA Power!\r\n");
       return;
   }

  s8Dog = SwdGet();
  if(s8Dog < 0)
  {
    Uart2SendString("CmdCdmaInfoTest SoftTimer Is Out Of Scope!\r\n");
    return;
  }
  SwdSetLifeTime(s8Dog, 25000);
  SwdEnable(s8Dog);
  //关闭看门狗
  StopWatchDog();
  Uart2SendString("###############################################################\r\n");
  Uart2SendString("# Please Enter Test Phone Number: ");
  while(TRUE)
  {
    if(Uart2GetRxDataLength()>10)
    {
      Uart2GetRxData(&au8Nbr[0], 11);

      for(i=0; i<11; i++)
      {
        if((au8Nbr[i] < '0') && (au8Nbr[i] > '9'))
        {
          Uart2SendString("# You Input Number Is Error, Please Resend The Cmd! \r\n");
          Uart2SendString("###############################################################\r\n\r\n");
          SwdDisable(s8Dog);
          SwdFree(s8Dog);
          FreeWatchDog();          //释放看门狗
          return;
        }
      }

      au8Nbr[11] = '\r';
      au8Nbr[12] = '\n';
      Uart2SendBuffer(&au8Nbr[0], 13);

      SwdDisable(s8Dog);
      SwdFree(s8Dog);
      FreeWatchDog();          //释放看门狗
      break;
    }

    if(SwdGetLifeTime(s8Dog) == 0)
    {
      SwdDisable(s8Dog);
      SwdFree(s8Dog);
      FreeWatchDog();          //释放看门狗
      Uart2SendString("#\r\n No Valid Phone Number, Please Resend The Cmd! \r\n");
      Uart2SendString("###############################################################\r\n\r\n");
      return;
    }

  }

  if( AssembleCdmaMsg(&au8Nbr[0], "This Is A Test Massage!", 23) )
  {
    Uart2SendString("# CDMA Massage Test Successfull! \r\n");
  }
  else
  {
    Uart2SendString("# CDMA Massage Test Failed! \r\n");
  }
  Uart2SendString("###############################################################\r\n\r\n");
}

/*****************************************************************************
 *函数名称: CdmaDeletMsg
 *功能描述: 删除CDMA模块指定短信

 *参数:
 *返回:

发送： 0xAA + 0x55 + 0x3B + 0xCC + 0x33
*****************************************************************************/
void CdmaDeletMsg(void)
{
  SINT8 s8Dog;
  UINT8 i=0;
  UINT8 au8Nbr[10];
  UINT8 u8Temp = 0;

  if(g_bCdmaSend != TRUE)
  {
       Uart2SendString("CDMA Module Is Power Down, Please Turn On CDMA Power!\r\n");
       return;
   }

  s8Dog = SwdGet();
  if(s8Dog < 0)
  {
    Uart2SendString("CdmaDeletMsg SoftTimer Is Out Of Scope!\r\n");
    return;
  }
  SwdSetLifeTime(s8Dog, 25000);
  SwdEnable(s8Dog);

  //关闭看门狗
  StopWatchDog();
  Uart2SendString("###############################################################\r\n");
  Uart2SendString("# Please Enter The Number You Want To Delete:\r\n ");
  Uart2SendString("# 1. Delete All   -- 30 \r\n ");
  Uart2SendString("# 2. Delete Index -- 00~29 \r\n ");
  while(TRUE)
  {
    if(Uart2GetRxDataLength()>1)
    {
      Uart2GetRxData(&au8Nbr[0], 2);

      for(i=0; i<2; i++)
      {
        if((au8Nbr[i] < '0') && (au8Nbr[i] > '9'))
        {
          Uart2SendString("# You Input Number Is Error, Please Resend The Cmd! \r\n");
          Uart2SendString("###############################################################\r\n\r\n");
          SwdDisable(s8Dog);
          SwdFree(s8Dog);
          FreeWatchDog();          //释放看门狗
          return;
        }
      }

      au8Nbr[2] = '\r';
      au8Nbr[3] = '\n';
      Uart2SendBuffer(&au8Nbr[0], 4);

      SwdDisable(s8Dog);
      SwdFree(s8Dog);
      FreeWatchDog();          //释放看门狗
      break;
    }

    if(SwdGetLifeTime(s8Dog) == 0)
    {
      SwdDisable(s8Dog);
      SwdFree(s8Dog);
      FreeWatchDog();          //释放看门狗
      Uart2SendString("# No Valid Phone Number, Please Resend The Cmd! \r\n");
      Uart2SendString("###############################################################\r\n\r\n");
      return;
    }

  }

  u8Temp  = (au8Nbr[0] - '0') * 10;
  u8Temp += (au8Nbr[1] - '0');

//  if(u8Temp == MSG_MAX_SIZE)
//  {
//    u8Temp = 0xff;
//  }

  if(DeletCdmaMsg(u8Temp))
  {
    Uart2SendString("# CDMA Delete Massage Successfull! \r\n");
  }
  else
  {
    Uart2SendString("# CDMA Delete Massage Failed! \r\n");
  }
  Uart2SendString("###############################################################\r\n\r\n");
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

  apCmdHandlerList[CMD_ALL_INFO_PRINT   ]  = PrintfCmdList;              //打印命令列表
  apCmdHandlerList[CMD_HAND_SHAKE       ]  = UartHandShake;              //打印命令列表
  apCmdHandlerList[CMD_GET_VERSION      ]  = GetSystemVersionInfo;       //打印命令列表
  apCmdHandlerList[CMD_SET_RTC_TIME     ]  = SetRtcTime;                 //设置系统时间
  apCmdHandlerList[CMD_GET_RTC_TIME     ]  = GetRtcTime;                 //获取系统时间
  apCmdHandlerList[CMD_SET_GAP_TIME     ]  = SetSendInfoGapTime;         //设置间隔时间
  apCmdHandlerList[CMD_GET_GAP_TIME     ]  = GetSendInfoGapTime;         //获取间隔时间
  apCmdHandlerList[CMD_SET_SYS_INDEX    ]  = SetSystemIndex;             //设置系统编号
  apCmdHandlerList[CMD_GET_SYS_INDEX    ]  = GetSystemIndex;             //获取系统编号
  apCmdHandlerList[CMD_CLR_RUN_TIME     ]  = SystemRunTimeClear;         //清除系统运行时间
  apCmdHandlerList[CMD_GET_RUN_TIME     ]  = SystemRunTimeGet;           //获取系统运行时间
  apCmdHandlerList[CMD_GET_SYS_BAT      ]  = GetBatteryRemainPower;      //获取当前电量
  apCmdHandlerList[CMD_USE_BD_MODE      ]  = UseBeiDouMode;              //使用北斗模式进行通讯
  apCmdHandlerList[CMD_USE_CDMA_MODE    ]  = UseCdmaMode;                //使用CDMA模式进行通讯

  apCmdHandlerList[CMD_GPS_INFO_CLOSE   ]  = CmdGpsDebugClose;           //关闭GPS的debug显示
  apCmdHandlerList[CMD_GPS_INFO_OPEN    ]  = CmdGpsDebugOpen;            //打开GPS的debug显示

//  apCmdHandlerList[CMD_BD_INFO_CLOSE]      = CmdBeiDouDebugClose;        //北斗通讯数据关闭
  apCmdHandlerList[CMD_BD_GET_LOC_NUM   ]  = CmdBeiDouLocNumGet;         //北斗本机卡号获取
  apCmdHandlerList[CMD_BD_SET_REC_NUM   ]  = CmdBeiDouRecNumSet;         //北斗接收机卡号设置
  apCmdHandlerList[CMD_BD_GET_REC_NUM   ]  = CmdBeiDouRecNumGet;         //北斗接收机卡号获取
  apCmdHandlerList[CMD_BD_SET_BCK_NUM   ]  = CmdBeiDouBckRecNumSet;      //北斗接收机卡号设置
  apCmdHandlerList[CMD_BD_GET_BCK_NUM   ]  = CmdBeiDouBckRecNumGet;      //北斗接收机卡号获取
  apCmdHandlerList[CMD_BD_INFO_TEST     ]  = CmdBeiDouSendMsg;           //北斗自收发测试
  apCmdHandlerList[CMD_BD_GET_SEND_SUC  ]  = CmdBeiDouGetSendSuccess;    //北斗通讯数据打开
  apCmdHandlerList[CMD_BD_CLR_SEND_SUC  ]  = CmdBeiDouClrSendSuccess;    //北斗通讯数据打开

  apCmdHandlerList[CMD_CDMA_INFO_CLOSE  ]  = CmdCdmaDebugClose;          //打开cdma调试
  apCmdHandlerList[CMD_CDMA_INFO_OPEN   ]  = CmdCdmaDebugOpen;           //关闭cdma调试
  apCmdHandlerList[CMD_CDMA_GET_TIME    ]  = CmdGetCdmaTime;             //获取CDMA时间
  apCmdHandlerList[CMD_CDMA_SET_LOC_NUM ]  = CmdSetCdmaLocationNbr;      //设置本机号码
  apCmdHandlerList[CMD_CDMA_GET_LOC_NUM ]  = CmdGetCdmaLocationNbr;      //获取本机号码
  apCmdHandlerList[CMD_CDMA_SET_SEND_NUM]  = CmdSetCdmaReceivingNbr;     //设置接收机号码
  apCmdHandlerList[CMD_CDMA_GET_SEND_NUM]  = CmdGetCdmaReceivingNbr;     //获取接收机号码
  apCmdHandlerList[CMD_CDMA_GET_CSQ     ]  = CmdGetCdmaCSQ;              //获取CDMA信号强度
  apCmdHandlerList[CMD_CDMA_INFO_TEST   ]  = CmdCdmaInfoTest;            //信息发送测试
  apCmdHandlerList[CMD_CDMA_DEL_MSG     ]  = CdmaDeletMsg;               //信息发送测试

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
      sDebugCmdCb.eAssembleState = E_UART_CMD_RECEIVING_HEADER1;
      sDebugCmdCb.u8BufDataLen = 0;
    }
  }
}

/**********************************************************************************************
 函数名称：DebugCmdInit
 函数功能：Debug cmd parse init

 参数： 无
 返回： 无
**********************************************************************************************/
void DebugCmdInit(void)
{
  //初始化命令列表
  CmdHandlerInit();

  //  sGpsCmdCb.bGpsParse = FALSE;
  sDebugCmdCb.u8BufDataLen = 0;
  sDebugCmdCb.eAssembleState = E_UART_CMD_RECEIVING_HEADER1;
  sDebugCmdCb.pu8AssembleBuf = OSMemGet(pSmallMem, &err);

  //Get One soft timer
  sDebugCmdCb.sDebugDog = SwdGet();
  SwdSetName(sDebugCmdCb.sDebugDog, "DebugTimer");
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

  while((Uart2GetRxDataLength() > 0) && (sDebugCmdCb.eAssembleState != E_UART_CMD_RECEIVING_VALID))
  {
    DebugCmdAssemble(Uart2GetByte());

    if(SwdIsEnable(sDebugCmdCb.sDebugDog) == TRUE)
    {
      SwdDisable(sDebugCmdCb.sDebugDog);
    }
  }

  //数据包有效，对可用的指令进行解析
  if(sDebugCmdCb.eAssembleState == E_UART_CMD_RECEIVING_VALID)
  {
    u8Cmd = sDebugCmdCb.pu8AssembleBuf[2];

    if(apCmdHandlerList[u8Cmd] != NULL)
    {
      apCmdHandlerList[u8Cmd]();
    }

    sDebugCmdCb.u8BufDataLen = 0;
    sDebugCmdCb.eAssembleState = E_UART_CMD_RECEIVING_HEADER1;
  }
  else
  {
    if(SwdIsEnable(sDebugCmdCb.sDebugDog) == TRUE)
    {
      if(SwdGetLifeTime(sDebugCmdCb.sDebugDog) == 0)
      {
        sDebugCmdCb.eAssembleState = E_UART_CMD_RECEIVING_HEADER1;
        sDebugCmdCb.u8BufDataLen = 0;

        SwdReset(sDebugCmdCb.sDebugDog);
        SwdDisable(sDebugCmdCb.sDebugDog);
      }
    }
    else
    {
      if(sDebugCmdCb.u8BufDataLen != 0)
      {
        SwdReset(sDebugCmdCb.sDebugDog);
        SwdEnable(sDebugCmdCb.sDebugDog);
      }
    }
  }
}
