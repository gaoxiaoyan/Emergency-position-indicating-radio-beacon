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
#include "Uart3.h"
#include "string.h"
#include "SoftTimer.h"
#include "CdmaModule.h"
#include "Rtc.h"
#include "Record.h"

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
  E_UART_CMD_RECEIVING_VALID
		
}teDebugCmdAssembleState;

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
//函数名称：PrintfCmdList
//功能描述：打印命令列表
//
//参数：无
//返回：无

 发送： 0xAA + 0x55 + 0x02 + 0xCC + 0x33
***************************************************************************/
void PrintfCmdList(void)
{
  Uart3SendString("###############################################################\r\n");
  Uart3SendString("# Copyright (C) 2015, Sunic-ocean.\r\n");
  Uart3SendString("# All Rights Reserved.\r\n");
  Uart3SendString("# \r\n");
  Uart3SendString("# Project Name：应急物资示位标.\r\n");
  Uart3SendString("#  \r\n");
  Uart3SendString("#==============================================================\r\n");
  Uart3SendString("#  0、命令格式： AA 55 cmd CC 33.  \r\n");
  Uart3SendString("#  1、cmd<00> -- 串口通讯握手命令；\r\n");
  Uart3SendString("#  2、cmd<01> -- 获取软件版本信息；\r\n");
  Uart3SendString("#  3、cmd<02> -- 打印调试指令列表；\r\n");
  Uart3SendString("#  4、cmd<03> -- 设置设备 RTC时间；\r\n");
  Uart3SendString("#  5、cmd<04> -- 获取设备 RTC时间；\r\n");
  Uart3SendString("#  6、cmd<05> -- 获取当前剩余电量；\r\n");
  Uart3SendString("#  7、cmd<06> -- 设置通讯间隔时间；\r\n");
  Uart3SendString("#  7、cmd<07> -- 获取通讯间隔时间；\r\n");
  Uart3SendString("#  8、cmd<08> -- 设置本台设备编码；\r\n");
  Uart3SendString("#  9、cmd<09> -- 获取本台设备编码；\r\n");
  Uart3SendString("# 10、cmd<10> -- GPS 调试信息关闭；\r\n");
  Uart3SendString("# 11、cmd<11> -- GPS 调试信息打开；\r\n");
  Uart3SendString("# 12、cmd<20> -- 北斗定位获取命令；\r\n");
  Uart3SendString("# 13、cmd<21> -- 北斗卡号获取命令；\r\n");
  Uart3SendString("# 14、cmd<22> -- 北斗报文测试命令；\r\n");
  Uart3SendString("# 15、cmd<23> -- 北斗时间获取命令；\r\n");
  Uart3SendString("# 16、cmd<30> -- CDMA调试信息关闭；\r\n");
  Uart3SendString("# 17、cmd<31> -- CDMA调试信息打开；\r\n");
  Uart3SendString("# 18、cmd<32> -- 通过CDMA获取时间；\r\n");
  Uart3SendString("# 19、cmd<33> -- 设置CDMA本机号码；\r\n");
  Uart3SendString("# 20、cmd<34> -- 获取CDMA本机号码；\r\n");
  Uart3SendString("# 21、cmd<35> -- 设置CDMA接收号码；\r\n");
  Uart3SendString("# 22、cmd<36> -- 获取CDMA接收号码；\r\n");
  Uart3SendString("# 23、cmd<37> -- 获取CDMA信号强度；\r\n");
  Uart3SendString("# 24、cmd<38> -- CDMA短信发送测试；\r\n");
  Uart3SendString("###############################################################\r\n");
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
  Uart3SendString("###############################################################\r\n");
  Uart3SendString("# Debug HandShake OK! The Bandrate is 115200bps.\r\n");
  Uart3SendString("###############################################################\r\n");
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
    Uart3SendString("###############################################################\r\n");      
    memset(&au8Version[0], 0, 8);
    au8Version[0] = 'V';
    au8Version[1] = VERSION / 100 + '0';
    au8Version[2] = '.';
    au8Version[3] = VERSION % 100 / 10 + '0';
    au8Version[4] = VERSION % 10 + '0';
    au8Version[5] = '\r';
  
    Uart3SendString("# The Current Version  Is: ");
    Uart3SendString(&au8Version[0]);
    Uart3SendString("# The Compilation Time Is: ");
    Uart3SendBuffer(&au8Date[0], 14);
    Uart3SendBuffer(&au8Time[0], 10);
    Uart3SendString("###############################################################\r\n");
    Uart3SendString(" \r\n");
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
  Uart3SendString("###############################################################\r\n");
  if( SetCdmaTimeToRtc() )
  {
    Uart3SendString("# Set Rtc Time Successful! \r\n");
  }
  else
  {
    Uart3SendString("# Set Rtc Time Failed! \r\n");
  }
  Uart3SendString("###############################################################\r\n");
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
  
  Uart3SendString("###############################################################\r\n");
  Uart3SendString("#The Current RTC Time Is: ");
  Uart3SendBuffer(&au8Time[0], i);
  Uart3SendString("###############################################################\r\n");
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
  Uart3SendString("###############################################################\r\n");
  AdcDetect();
  Uart3SendString("###############################################################\r\n");
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
  Uart3SendString("###############################################################\r\n");
  Uart3SendString("###############################################################\r\n");
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
  Uart3SendString("###############################################################\r\n");
  Uart3SendString("###############################################################\r\n");
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
  UINT8 au8Nbr[8];
  UINT16 u16Temp;
  
  Uart3EmptyRxBuf();
  Uart3SendString("###############################################################\r\n");
  Uart3SendString("# Please Enter The Index Of This System: 00001~60000\r\n"); 
  
  s8Dog = SwdGet();
  SwdSetLifeTime(s8Dog, 20000);
  SwdEnable(s8Dog);
  
  memset(au8Nbr, 0, 8);
  //关闭看门狗
  StopWatchDog();
  
  while(TRUE)
  {
    if(Uart3GetRxDataLength()>4)
    {
      Uart3GetRxData(&au8Nbr[0], 5);
      
      if(memcmp(&au8Nbr[0], "60000", 5)>0)
      {
        Uart3SendString("# The Enter Index Is Beyond! Please Resend The Cmd!\r\n"); 
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
      Uart3SendString("# No Valid Number, Please Resend The Cmd! \r\n");
      return;
    }
  }
  
  u16Temp  = (au8Nbr[0] - '0') * 10000;
  u16Temp += (au8Nbr[1] - '0') * 1000;
  u16Temp += (au8Nbr[2] - '0') * 100;
  u16Temp += (au8Nbr[3] - '0') * 10;
  u16Temp += (au8Nbr[4] - '0') ;
  
  if(u16Temp != sRecordCb.u16SysNbr)
  {
    sRecordCb.u16SysNbr = u16Temp;
    g_u8NeedSaved |= SAVE_SYS_NBR;
  }
  
  Uart3SendString("# System Index Set Successfull! \r\n");
  Uart3SendString("###############################################################\r\n");
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
  UINT8 au8Index[8];
  
  au8Index[0] = sRecordCb.u16SysNbr/10000 + '0';
  au8Index[1] = sRecordCb.u16SysNbr%10000/1000 + '0';
  au8Index[2] = sRecordCb.u16SysNbr%1000/100 + '0';
  au8Index[3] = sRecordCb.u16SysNbr%100/10 + '0';
  au8Index[4] = sRecordCb.u16SysNbr%10 + '0';
  au8Index[5] = '\r';
  au8Index[6] = '\n';
  
  Uart3SendString("###############################################################\r\n");
  Uart3SendString("# The Index Of This System Is: "); 
  Uart3SendBuffer(&au8Index[0], 7); 
  Uart3SendString("###############################################################\r\n");
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
}

/*****************************************************************************
 *函数名称: CmdBeiDouDebugOpen
 *功能描述: 打开BeiDou调试信息

 *参数: 
 *返回:

发送： 0xAA + 0x55 + 0x21 + 0xCC + 0x33
*****************************************************************************/
void CmdBeiDouDebugOpen(void)
{
  g_bDebug |=  BD_DEBUG;
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
  
  Uart3SendString("###############################################################\r\n");
  Uart3SendString("# The Current CDMA Time Is: ");
  GetCdmaTime(&au8Time[0], &u8Len);
  Uart3SendBuffer(&au8Time[0], u8Len);
  Uart3SendString("\r\n");
  Uart3SendString("###############################################################\r\n");
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
  
  Uart3SendString("###############################################################\r\n");
  s8Dog = SwdGet();
  SwdSetLifeTime(s8Dog, 25000);
  SwdEnable(s8Dog);
  //关闭看门狗
  StopWatchDog();
  Uart3SendString("# Please Enter The Location Number: ");
  while(TRUE)
  {
    if(Uart3GetRxDataLength()>10)
    {
      Uart3GetRxData(&au8Nbr[0], 11);
      
      for(i=0; i<11; i++)
      {
        if((au8Nbr[i] < '0') && (au8Nbr[i] > '9'))
        {
          Uart3SendString("# You Input Number Is Error, Please Resend The Cmd! \r\n");
          SwdDisable(s8Dog);
          SwdFree(s8Dog);
          FreeWatchDog();          //释放看门狗
          return;
        }
      }
      
      au8Nbr[11] = '\r'; 
      au8Nbr[12] = '\n'; 
      Uart3SendBuffer(&au8Nbr[0], 13);
      
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
      Uart3SendString("# No Valid Number, Please Resend The Cmd! \r\n");
      return;
    }
    
  }
  
  if( SetLocationNbr(&au8Nbr[0]) )
  {
    Uart3SendString("# CDMA Location Number Set Successfull! \r\n");
  }
  else
  {
    Uart3SendString("# CDMA Location Number Set Failed! \r\n");
  }
  Uart3SendString("###############################################################\r\n");
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
  
  Uart3SendString("###############################################################\r\n");
  if( GetLocationNbr(&au8Nbr[0]) )
  {
    au8Nbr[11] = '\r';
    au8Nbr[12] = '\n';
    Uart3SendString("# CDMA Location Number Is: ");
    Uart3SendBuffer(au8Nbr, 13);
  }
  else
  {
    Uart3SendString("# CDMA Location Number Get Failed! \r\n");
  }
  Uart3SendString("###############################################################\r\n");
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

  //关闭看门狗
  StopWatchDog();
  Uart3SendString("###############################################################\r\n");
  Uart3EmptyRxBuf();
  Uart3SendString("#Please Enter The Receiving Number: ");
  
  s8Dog = SwdGet();
  SwdSetLifeTime(s8Dog, 25000);
  SwdEnable(s8Dog);
  
  while(TRUE)
  {
    if(Uart3GetRxDataLength()>10)
    {
      Uart3GetRxData(&au8Nbr[0], 11);
        
      for(i=0; i<11; i++)
      {
        if((au8Nbr[i] < '0') && (au8Nbr[i] > '9'))
        {
          Uart3SendString("# You Input Number Is Error, Please Resend The Cmd! \r\n");
          Uart3SendString("###############################################################\r\n");
          SwdDisable(s8Dog);
          SwdFree(s8Dog);
          FreeWatchDog();          //释放看门狗
          return;
        }
      }
        
      au8Nbr[11] = '\r'; 
      au8Nbr[12] = '\n'; 
      Uart3SendBuffer(&au8Nbr[0], 13);
      memcpy(&sRecordCb.au8CdmaRevNbr[0], &au8Nbr[0], 11);
      Uart3SendString("# CDMA Receiving Number Set Successfull! \r\n");
      break;
    }
            
    if(SwdGetLifeTime(s8Dog) == 0)
    {
      SwdDisable(s8Dog);
      SwdFree(s8Dog);
      FreeWatchDog();          //释放看门狗
      Uart3SendString("# CDMA Receiving Number Set Failed! \r\n");
      Uart3SendString("###############################################################\r\n");
      return;
    }
  }
  Uart3SendString("###############################################################\r\n");
  g_u8NeedSaved |= SAVE_REV_CDMA_NBR;
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
 
  Uart3SendString("###############################################################\r\n");
  Uart3SendString("# The CDMA Receiving Number Is: ");
  ReadCdmaReceivingNbr(&au8Nbr[0]);
  au8Nbr[11] = '\r';
  au8Nbr[12] = '\n';
  
  Uart3SendBuffer(&au8Nbr[0], 13);
  Uart3SendString("###############################################################\r\n");
}

/*****************************************************************************
 *函数名称: CmdGetCdmaCSQ
 *功能描述: 获取CDMA的信号强度

 *参数: 
 *返回:

发送： 0xAA + 0x55 + 0x38 + 0xCC + 0x33
*****************************************************************************/
void CmdGetCdmaCSQ(void)
{
  UINT8 u8Temp;
  UINT8 u8Num[4];
  
  u8Temp = GetCdmaCSQ();
  u8Num[0] = u8Temp / 10 + '0';
  u8Num[1] = u8Temp % 10 + '0';
  u8Num[2] = '\r';
  u8Num[3] = '\n';
   
  Uart3SendString("###############################################################\r\n");
  Uart3SendString("# The CDMA Current CSQ Is: ");
  Uart3SendBuffer(u8Num, 4);
  Uart3SendString("###############################################################\r\n");
}


/*****************************************************************************
 *函数名称: CmdCdmaInfoTest
 *功能描述: CDMA模块信息发送测试

 *参数: 
 *返回:

发送： 0xAA + 0x55 + 0x39 + 0xCC + 0x33
*****************************************************************************/
void CmdCdmaInfoTest(void)
{
  SINT8 s8Dog;
  UINT8 i=0;
  UINT8 au8Nbr[20];
  
  s8Dog = SwdGet();
  SwdSetLifeTime(s8Dog, 25000);
  SwdEnable(s8Dog);
  //关闭看门狗
  StopWatchDog();
  Uart3SendString("###############################################################\r\n");
  Uart3SendString("# Please Enter Test Phone Number: ");
  while(TRUE)
  {
    if(Uart3GetRxDataLength()>10)
    {
      Uart3GetRxData(&au8Nbr[0], 11);
      
      for(i=0; i<11; i++)
      {
        if((au8Nbr[i] < '0') && (au8Nbr[i] > '9'))
        {
          Uart3SendString("# You Input Number Is Error, Please Resend The Cmd! \r\n");
          Uart3SendString("###############################################################\r\n");
          SwdDisable(s8Dog);
          SwdFree(s8Dog);
          FreeWatchDog();          //释放看门狗
          return;
        }
      }
      
      au8Nbr[11] = '\r'; 
      au8Nbr[12] = '\n'; 
      Uart3SendBuffer(&au8Nbr[0], 13);
      
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
      Uart3SendString("#\r\n No Valid Phone Number, Please Resend The Cmd! \r\n");
      Uart3SendString("###############################################################\r\n");
      return;
    }
    
  }
  
  if( AssembleCdmaMsg(&au8Nbr[0], "This Is A Test Massage!", 23) )
  {
    Uart3SendString("# CDMA Massage Test Successfull! \r\n");
  }
  else
  {
    Uart3SendString("# CDMA Massage Test Failed! \r\n");
  }
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
  
  apCmdHandlerList[CMD_ALL_INFO_PRINT]     = PrintfCmdList;              //打印命令列表
  apCmdHandlerList[CMD_HAND_SHAKE]         = UartHandShake;              //打印命令列表
  apCmdHandlerList[CMD_GET_VERSION]        = GetSystemVersionInfo;       //打印命令列表
  apCmdHandlerList[CMD_SET_RTC_TIME]       = SetRtcTime;                 //设置系统时间
  apCmdHandlerList[CMD_GET_RTC_TIME]       = GetRtcTime;                 //获取系统时间
  apCmdHandlerList[CMD_SET_SYS_INDEX]      = SetSystemIndex;             //设置系统编号
  apCmdHandlerList[CMD_GET_SYS_INDEX]      = GetSystemIndex;             //获取系统编号
  apCmdHandlerList[CMD_GPS_INFO_CLOSE]     = CmdGpsDebugClose;           //关闭GPS的debug显示
  apCmdHandlerList[CMD_GPS_INFO_OPEN]      = CmdGpsDebugOpen;            //打开GPS的debug显示
//  apCmdHandlerList[CMD_BD_INFO_CLOSE]      = CmdBeiDouDebugClose;        //北斗通讯数据关闭
//  apCmdHandlerList[CMD_BD_INFO_OPEN]       = CmdBeiDouDebugOpen;         //北斗通讯数据打开
  apCmdHandlerList[CMD_CDMA_GET_TIME]      = CmdGetCdmaTime;             //获取CDMA时间
  apCmdHandlerList[CMD_CDMA_SET_LOC_NUM]   = CmdSetCdmaLocationNbr;      //设置本机号码
  apCmdHandlerList[CMD_CDMA_GET_LOC_NUM]   = CmdGetCdmaLocationNbr;      //获取本机号码
  apCmdHandlerList[CMD_CDMA_SET_SEND_NUM]  = CmdSetCdmaReceivingNbr;     //设置接收机号码
  apCmdHandlerList[CMD_CDMA_GET_SEND_NUM]  = CmdGetCdmaReceivingNbr;     //获取接收机号码
  apCmdHandlerList[CMD_CDMA_GET_CSQ]       = CmdGetCdmaCSQ;              //获取CDMA信号强度
  apCmdHandlerList[CMD_CDMA_INFO_TEST ]    = CmdCdmaInfoTest;            //信息发送测试
  apCmdHandlerList[CMD_GET_SYS_BAT]        = GetBatteryRemainPower;      //获取当前电量
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

  while((Uart3GetRxDataLength() > 0) && (sDebugCmdCb.eAssembleState != E_UART_CMD_RECEIVING_VALID))
  {
    DebugCmdAssemble(Uart3GetByte());

    if(SwdEnable(sDebugCmdCb.sDebugDog) == TRUE)
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
    if(SwdEnable(sDebugCmdCb.sDebugDog) == TRUE)
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