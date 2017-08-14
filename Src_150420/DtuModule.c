
/******************************************************************************
*											
* Copyright (C) 2014, Sunic-ocean
* All Rights Reserved										
*
* 文 件 名： DtuModule.C
* 描    述： DTU数据解析模块

* 创建日期： 2014年9月5日11:31:59
* 作    者： Bob
* 当前版本： V1.00

------------------------------------------------------
*******************************************************************************/

#ifdef MSP430F149
#include "msp430x14x.h"
#else
#include "msp430x54x.h"
#endif

#include "DtuModule.h"
#include "DataType.h"
#include "Uart.h"
#include "SystemCtl.h"
#include "string.h"
#include "Flash.h"
#include "Timer.h"
#include "GpsParse.h"
#include "UartCmd.h"
#include "Display.h"
#include "stdio.h"

//前面的强制类型转化缺省会造成计算错误
#define _GetWordFromAddr(pAddr)         (UINT16)(((UINT16)pAddr[0]) << 8) + pAddr[1]

#define _GetDoubleWordFromAddr(pAddr)   ((UINT32)(((UINT32)pAddr[0]) << 24) +    \
                                         (UINT32)(((UINT32)pAddr[1]) << 16) +    \
                                         (UINT32)(((UINT32)pAddr[2]) << 8) + pAddr[3])



//命令解析结构定义
_tsUartCmdCb sDtuCmdCb;

//DTU接收缓冲区
UINT8 au8DtuBodyBuf[MAX_BODY_SIZE];

//TCP / UDP
#define TCP         0
#define UDP         1

//移动 / 联通选择
#define CMNET       0
#define UNINET      1

//进入DTU配置模式
const UINT8 u8ConfigMode[] = {"enter_config_mode\0"};
const UINT8 u8ConfigStart[] = {0xff,0xff,0xff,0xff};
const UINT8 u8ConfigRxData[] = {"aaaaaaaaaaaaaaaa\0"};

const UINT8 au8BackOk[3] = {0x4f, 0x4b, 0x00};

//串口波特率设置
const UINT8 au8UartSet[] = {"AT+UART=9600,ON,8,N,1\r"};
//使用固定的协议，通讯方式，用户名和密码。
const UINT8 au8Tcp[] = {"AT+NET="}; //TCP,CMNET,card,card,"};
//上电自动连接
const UINT8 au8AutoCon[] = {"AT+AUTOCON=1\r"};
//设备ID
const UINT8 au8Id[] = {"AT+ID="};
const UINT8 au8GetId[] = {"AT+ID?\r"};
//设置重播时间(秒)
const UINT8 au8RedialTime[] = {"AT+REDIALTIME=8\r"};
//网络空闲时间（分钟）
const UINT8 au8TcpIdle[] = {"AT+TCPIDLE=30\r"};
//设置心跳包
const UINT8 au8Beat[] = {"AT+BEAT=30,KYTENCE,0\r"};
//设置发送数据包间隔
const UINT8 au8PkgTime[] = {"AT+PKGT=100\r"};
//设置DSC
const UINT8 au8Dsc[] = {"AT+DSC=1\r"};
//多中心模式
const UINT8 au8NetMode[] = {"AT+NETMODE=1\r"};
//调试模式开关
const UINT8 au8Trace[] = {"AT+TRACE=0\r"};
//发送复位命令
const UINT8 au8Reset[] = {"AT+RESET=1\r"};

//分中心1配置
const UINT8 au8EnableSub1[] = {"AT+NET1=1,"};
const UINT8 au8DisableSub1[] = {"AT+NET1=0,"};
//分中心2配置
const UINT8 au8EnableSub2[] = {"AT+NET2=1,"};
const UINT8 au8DisableSub2[] = {"AT+NET2=0,"};
//分中心3配置
const UINT8 au8EnableSub3[] = {"AT+NET3=1,"};
const UINT8 au8DisableSub3[] = {"AT+NET3=0,"};
//分中心4配置
const UINT8 au8EnableSub4[] = {"AT+NET4=1,"};
const UINT8 au8DisableSub4[] = {"AT+NET4=0,"};

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
 函 数 名  : DtuEnterConfigMode
 功能描述  : DTU module initialize, enter config mode

 输入参数  : none
 返 回 值  : 配置状态,
*****************************************************************************/
UINT8 DtuEnterConfigMode(void)
{
  UINT8 u8Buf[20];
  UINT8 u8State = 0;
  UINT8 i;
  
  MEM_SET(u8Buf, 0, 20);
  DtuPowerOff();
  Delay(10000);
  DtuPowerOn();
//  Delay(100);
  
  //将串口0接收超时定时器作为DTU配置时的超时定时器，定时为10秒
  asTimer[TIMER_UART1].u16LifeTime = TIMER_TICK * 10;
  asTimer[TIMER_UART1].u16LifeOldTime = TIMER_TICK * 10;
  asTimer[TIMER_UART1].bEnable = TRUE;
  
  while(TRUE)
  {
    FreeWatchDog();         //清狗,设置为16s
    
    switch(u8State)
    {
      case 0:
      {  
        for(i=0; i<255; i++)
        {
          UartSendBuffer(UART1, u8ConfigStart, sizeof(u8ConfigStart));
          
          if(Uart1GetRxDataLength() > 16)
          {
            u8State = 1;
            break;
          }
        }

        break;
      }
      
      case 1:
      {
        Uart1GetRxData(u8Buf, 17);         
        if(0 == MEM_CMP(u8Buf, u8ConfigRxData, 17))
        {
          MEM_SET(u8Buf, 0, 20);
          u8State = 2;
        }
        else
        {
          u8State = 0;
        }

        break;
      }
      
      case 2:
      {
        if(Uart1GetRxDataLength() > 17)
        {
          Uart1GetRxData(u8Buf, 18);
          if(0 == MEM_CMP(u8Buf, u8ConfigMode, 18))
          {
            u8State = 3;
          }
          else
          {
            u8State = 0;
          }
        }
        break;
      }
      
      case 3:
      {
        //恢复串口0接收超时定时器。
        asTimer[TIMER_UART1].u16LifeTime = TIMER_TICK / 2;
        asTimer[TIMER_UART1].u16LifeOldTime = TIMER_TICK / 2;
        asTimer[TIMER_UART1].bEnable = FALSE;
        
        return DTU_CFG_MODE;
      }
      
      default: u8State = 0;
    }
    
    if(asTimer[TIMER_UART1].u16LifeTime == 0)
    {
      //恢复串口0接收超时定时器。
      asTimer[TIMER_UART1].u16LifeTime = TIMER_TICK / 2;
      asTimer[TIMER_UART1].u16LifeOldTime = TIMER_TICK / 2;
      asTimer[TIMER_UART1].bEnable = FALSE;
        
      return DTU_CFG_ERROR;
    }
    
  }
}

/*****************************************************************************
 函 数 名  : GetDtuConfigInfo
 功能描述  : Get DTU module config, Here mainly get module ID.

 输入参数  : none
 返 回 值  : 配置状态
*****************************************************************************/
UINT8 GetDtuConfigInfo(UINT8 *pu8Buf)
{
  UINT8 u8CfgState=0xff;
  UINT8 i,j,u8Temp;
  UINT8 au8IdBuf[16]={0};
  
  //将DTU进入配置模式
  u8CfgState = DtuEnterConfigMode();
  if(DTU_CFG_MODE == u8CfgState)
  {
    //清空BUF1
    Uart1EmptyRxBuf();
     //配置TCP
    UartSendBuffer(UART1, &au8GetId[0], (sizeof(au8GetId)-1));
    while(Uart1GetRxDataLength() <22);
    for(i=0; i<10; i++)
      Uart1GetByte();
      
    for(i=0;i<16;i++)
    {
      u8Temp = Uart1GetByte();
      if(u8Temp != ';')
      {
        pu8Buf[i] = u8Temp;
      }
      else
      {
        break;
      }
    }

    //发复位命令
     Delay(1000);
    UartSendBuffer(UART1, &au8Reset[0], (sizeof(au8Reset)-1));
    Delay(1000);
    return DTU_CFG_OK;
  }
    else
  {
    //发复位命令
    UartSendBuffer(UART1, &au8Reset[0], (sizeof(au8Reset)-1));
    
    return DTU_CFG_ERROR;
  }
}

/*****************************************************************************
 函 数 名  : GetStringLen
 功能描述  : 获取字符串长度，遇0返回

 输入参数  : *S字符串指针
 返 回 值  : 字符串长度
*****************************************************************************/
UINT8 GetStringLen(UINT8 *S)
{
  UINT8 i=0;
  
  while(S[i] != 0)
  {
    i++;
  }
  
  if(i>0)
    return i;
  else
    return 0;
}

UINT8 GetDataWidth(UINT32 u32Data)
{ 
  if(u32Data < 10)
    return 1;
  else if((u32Data > 9) && (u32Data < 100))
    return 2;
  else if((u32Data > 99) && (u32Data < 1000))
    return 3;
  else if((u32Data > 999) && (u32Data < 10000))
    return 4;
  else if((u32Data > 9999) && (u32Data < 100000))
    return 5;
  else
    return 6;
}


/*****************************************************************************
 函 数 名  : ConfigDtuModule
 功能描述  : Config dtu module, 

 输入参数  : UINT8 u8State
 返 回 值  :
*****************************************************************************/
UINT8 McuConfigDtuMainServer(void)
{
  UINT8 u8StrLen, u8IpLen; //字符串长度  
  UINT8 au8ServerBuf[128];
  char au8IpBuf[64];
  UINT16 u16CfgCom;
  UINT8 u8CfgState = 0xff;
  
  MEM_SET(au8ServerBuf, 0, sizeof(au8ServerBuf));
  u8StrLen = 7; //GetStringLen(&au8Tcp[0]);
  MEM_CPY(&au8ServerBuf[0], &au8Tcp[0], u8StrLen);
  
  //TCP/UDP协议
  if(sMainServer.bBckTcpUdp == TCP)
  {
    MEM_CPY(&au8ServerBuf[u8StrLen], "TCP,", 4);
  }
  else
  {
    if(sMainServer.bBckTcpUdp == UDP)
    {
      MEM_CPY(&au8ServerBuf[u8StrLen], "UDP,", 4);
    }
  }
  
  u8StrLen += 4;
  
  //网电接入
  if(sMainServer.bCfgApn == CMNET)    //中国移动
  {
    MEM_CPY(&au8ServerBuf[u8StrLen], "CMNET,", 6);
    u8StrLen += 6;
  }
  else
  {
    if(sMainServer.bCfgApn == UNINET) //中国联通
    {
      MEM_CPY(&au8ServerBuf[u8StrLen], "UNINET,", 7);
      u8StrLen += 7;
    }
  }
  
  if((sMainServer.u8CfgUserNameLen == 1) && (sMainServer.u8CfgPasswordLen == 1))
  {
    au8ServerBuf[u8StrLen++] = ',';
    au8ServerBuf[u8StrLen++] = ',';
  }
  else
  {
    //用户名设置
    MEM_CPY(&au8ServerBuf[u8StrLen], &sMainServer.au8CfgUserName[0], sMainServer.u8CfgUserNameLen);
    u8StrLen += sMainServer.u8CfgUserNameLen;
    au8ServerBuf[u8StrLen++] = ',';
    //用户密码设置
    MEM_CPY(&au8ServerBuf[u8StrLen], &sMainServer.au8CfgPassword[0], sMainServer.u8CfgPasswordLen);
    u8StrLen += sMainServer.u8CfgPasswordLen;
    au8ServerBuf[u8StrLen++] = ',';
  }
  
  //IP地址获取
  if(sMainServer.u8CfgIpLen == 4)
  {
    u8IpLen = 0;
    MEM_SET(&au8IpBuf[0], 0, sizeof(au8IpBuf));
    sprintf(&au8IpBuf[u8IpLen],"%d", sMainServer.au8CfgIpAddr[0]);
    u8IpLen += GetDataWidth(sMainServer.au8CfgIpAddr[0]);
    au8IpBuf[u8IpLen++] = '.';
    
    sprintf(&au8IpBuf[u8IpLen],"%d", sMainServer.au8CfgIpAddr[1]);
    u8IpLen += GetDataWidth(sMainServer.au8CfgIpAddr[1]);
    au8IpBuf[u8IpLen++] = '.';
    
    sprintf(&au8IpBuf[u8IpLen],"%d", sMainServer.au8CfgIpAddr[2]);
    u8IpLen += GetDataWidth(sMainServer.au8CfgIpAddr[2]);
    au8IpBuf[u8IpLen++] = '.';
    
    sprintf(&au8IpBuf[u8IpLen],"%d", sMainServer.au8CfgIpAddr[3]);
    u8IpLen += GetDataWidth(sMainServer.au8CfgIpAddr[3]);
    au8IpBuf[u8IpLen++] = ',';
    
    MEM_CPY(&au8ServerBuf[u8StrLen], &au8IpBuf[0], u8IpLen);
    
    u8StrLen += u8IpLen;
  }
  else
  {
    MEM_CPY(&au8ServerBuf[u8StrLen], &sMainServer.au8CfgIpAddr[0], sMainServer.u8CfgIpLen);
    u8StrLen += sMainServer.u8CfgIpLen;
    au8ServerBuf[u8StrLen++] = ',';
  }

  //配置端口号
  u16CfgCom = (UINT16)(((UINT16)sMainServer.au8CfgComIndex[0]) << 8) + sMainServer.au8CfgComIndex[1];
  MEM_SET(&au8IpBuf[0], 0, sizeof(au8IpBuf));
  sprintf(&au8IpBuf[0], "%d", u16CfgCom);
  u8IpLen = GetDataWidth(u16CfgCom);
  MEM_CPY(&au8ServerBuf[u8StrLen], &au8IpBuf[0], u8IpLen);
  u8StrLen += u8IpLen;
  au8ServerBuf[u8StrLen++] = '\r';
  
  //将DTU进入配置模式
  u8CfgState = DtuEnterConfigMode();
  if(DTU_CFG_MODE == u8CfgState)
  {
    //清空BUF1
    Uart1EmptyRxBuf();
    //配置UART
    UartSendBuffer(UART1, &au8UartSet[0], (sizeof(au8UartSet)-1));
    Delay(1000);
    if(Uart1GetRxDataLength() > 2)
    {
      au8IpBuf[0] = Uart1GetByte();
      au8IpBuf[1] = Uart1GetByte();
      au8IpBuf[2] = Uart1GetByte();
      if(0 != MEM_CMP(&au8IpBuf[0], &au8BackOk[0], 3))
      {
        //发复位命令
        UartSendBuffer(UART1, &au8Reset[0], (sizeof(au8Reset)-1));
        return DTU_UART_SET_ERROR;
      }
    }
    
    //配置TCP
    UartSendBuffer(UART1, &au8ServerBuf[0], u8StrLen);
    Delay(1000);
    if(Uart1GetRxDataLength() > 2)
    {
      au8IpBuf[0] = Uart1GetByte();
      au8IpBuf[1] = Uart1GetByte();
      au8IpBuf[2] = Uart1GetByte();
      if(0 != MEM_CMP(&au8IpBuf[0], &au8BackOk[0], 3))
      {
        //发复位命令
        UartSendBuffer(UART1, &au8Reset[0], (sizeof(au8Reset)-1));
        return DTU_TCP_SET_ERROR;
      }
    }
    
    //配置自动连接项
    UartSendBuffer(UART1, &au8AutoCon[0], (sizeof(au8AutoCon)-1));
    Delay(1000);
    if(Uart1GetRxDataLength() > 2)
    {
      au8IpBuf[0] = Uart1GetByte();
      au8IpBuf[1] = Uart1GetByte();
      au8IpBuf[2] = Uart1GetByte();
      if(0 != MEM_CMP(&au8IpBuf[0], &au8BackOk[0], 3))
      {
        //发复位命令
        UartSendBuffer(UART1, &au8Reset[0], (sizeof(au8Reset)-1));
        return DTU_AUTO_SET_ERROR;
      }
    }
    
    //ID设置
    u8StrLen = 0;
    u8IpLen = sizeof(au8Id)-1;
    MEM_CPY(&au8IpBuf[u8StrLen], &au8Id[0],  u8IpLen);
    u8StrLen += u8IpLen;
    MEM_CPY(&au8IpBuf[u8StrLen], &sMainServer.au8CfgId[0], 11);
    u8StrLen += 11;
    au8IpBuf[u8StrLen++] = ',';
    au8IpBuf[u8StrLen++] = '0';
    au8IpBuf[u8StrLen++] = '\r';
    
    UartSendBuffer(UART1, &au8IpBuf[0], u8StrLen);
    Delay(1000);
    if(Uart1GetRxDataLength() > 2)
    {
      au8IpBuf[0] = Uart1GetByte();
      au8IpBuf[1] = Uart1GetByte();
      au8IpBuf[2] = Uart1GetByte();
      if(0 != MEM_CMP(&au8IpBuf[0], &au8BackOk[0], 3))
      {
        //发复位命令
        UartSendBuffer(UART1, &au8Reset[0], (sizeof(au8Reset)-1));
        return DTU_ID_SET_ERROR;
      }
    }
    
    //设置重播时间
    UartSendBuffer(UART1, &au8RedialTime[0], (sizeof(au8RedialTime)-1));
    Delay(1000);
    if(Uart1GetRxDataLength() > 2)
    {
      au8IpBuf[0] = Uart1GetByte();
      au8IpBuf[1] = Uart1GetByte();
      au8IpBuf[2] = Uart1GetByte();
      if(0 != MEM_CMP(&au8IpBuf[0], &au8BackOk[0], 3))
      {
        //发复位命令
        UartSendBuffer(UART1, &au8Reset[0], (sizeof(au8Reset)-1));
        return DTU_AGAIN_SET_ERROR;
      }
    }
    
    //设置网络空闲时间
    UartSendBuffer(UART1, &au8TcpIdle[0], (sizeof(au8TcpIdle)-1));
    Delay(1000);
    if(Uart1GetRxDataLength() > 2)
    {
      au8IpBuf[0] = Uart1GetByte();
      au8IpBuf[1] = Uart1GetByte();
      au8IpBuf[2] = Uart1GetByte();
      if(0 != MEM_CMP(&au8IpBuf[0], &au8BackOk[0], 3))
      { 
        //发复位命令
        UartSendBuffer(UART1, &au8Reset[0], (sizeof(au8Reset)-1));
        return DTU_IDLE_SET_ERROR;
      }
    }
    
    //设置心跳包
    UartSendBuffer(UART1, &au8Beat[0], (sizeof(au8Beat)-1));
    Delay(1000);
    if(Uart1GetRxDataLength() > 2)
    {
      au8IpBuf[0] = Uart1GetByte();
      au8IpBuf[1] = Uart1GetByte();
      au8IpBuf[2] = Uart1GetByte();
      if(0 != MEM_CMP(&au8IpBuf[0], &au8BackOk[0], 3))
      {
        //发复位命令
        UartSendBuffer(UART1, &au8Reset[0], (sizeof(au8Reset)-1));
        return DTU_BEAT_SET_ERROR;
      }
    }
    
    //设置数据包间隔
    UartSendBuffer(UART1, &au8PkgTime[0], (sizeof(au8PkgTime)-1));
    Delay(1000);
    if(Uart1GetRxDataLength() > 2)
    {
      au8IpBuf[0] = Uart1GetByte();
      au8IpBuf[1] = Uart1GetByte();
      au8IpBuf[2] = Uart1GetByte();
      if(0 != MEM_CMP(&au8IpBuf[0], &au8BackOk[0], 3))
      {
        //发复位命令
        UartSendBuffer(UART1, &au8Reset[0], (sizeof(au8Reset)-1));
        return DTU_GAP_SET_ERROR;
      }
    }

    //设置DCS
    UartSendBuffer(UART1, &au8Dsc[0], (sizeof(au8Dsc)-1));
    Delay(1000);
    if(Uart1GetRxDataLength() > 2)
    {
      au8IpBuf[0] = Uart1GetByte();
      au8IpBuf[1] = Uart1GetByte();
      au8IpBuf[2] = Uart1GetByte();
      if(0 != MEM_CMP(&au8IpBuf[0], &au8BackOk[0], 3))
      {
        //发复位命令
        UartSendBuffer(UART1, &au8Reset[0], (sizeof(au8Reset)-1));
        return DTU_DCS_SET_ERROR;
      }
    }
    
    //设置多中心、备份模式
    UartSendBuffer(UART1, &au8NetMode[0], (sizeof(au8NetMode)-1));
    Delay(1000);
    if(Uart1GetRxDataLength() > 2)
    {
      au8IpBuf[0] = Uart1GetByte();
      au8IpBuf[1] = Uart1GetByte();
      au8IpBuf[2] = Uart1GetByte();
      if(0 != MEM_CMP(&au8IpBuf[0], &au8BackOk[0], 3))
      {
        //发复位命令
        UartSendBuffer(UART1, &au8Reset[0], (sizeof(au8Reset)-1));
        return DTU_MUL_SET_ERROR;
      }
    }
    
    //设置调试开关
    UartSendBuffer(UART1, &au8Trace[0], (sizeof(au8Trace)-1));
    Delay(1000);
    if(Uart1GetRxDataLength() > 2)
    {
      au8IpBuf[0] = Uart1GetByte();
      au8IpBuf[1] = Uart1GetByte();
      au8IpBuf[2] = Uart1GetByte();
      if(0 != MEM_CMP(&au8IpBuf[0], &au8BackOk[0], 3))
      {
        //发复位命令
        UartSendBuffer(UART1, &au8Reset[0], (sizeof(au8Reset)-1));
        return DTU_DEB_SET_ERROR;
      }
    }
    
    //发复位命令
    UartSendBuffer(UART1, &au8Reset[0], (sizeof(au8Reset)-1));
    g_u8NeedSaved |= SAVE_DTU_MAIN;
    
    return DTU_CFG_OK;
  }
  else
  {
    //发复位命令
    UartSendBuffer(UART1, &au8Reset[0], (sizeof(au8Reset)-1));
    
    return DTU_CFG_ERROR;
  }
}

/*****************************************************************************
 函 数 名  : McuConfigDtuSub1
 功能描述  : Config dtu module, 

 输入参数  : UINT8 u8State
 返 回 值  :
*****************************************************************************/
UINT8 McuConfigDtuSub1(UINT8 u8CfgEnable)
{
  UINT8  u8IpLen;   
  char  au8IpBuf[64];
  UINT16 u16CfgCom;
  UINT8  u8CfgState = 0xff;
    
  u8IpLen = 0;
  MEM_SET(&au8IpBuf[u8IpLen], 0, sizeof(au8IpBuf));
  if(u8CfgEnable == DTU_CFG_ENABLE)
  {
    MEM_CPY(&au8IpBuf[u8IpLen], &au8EnableSub1[0], (sizeof(au8EnableSub1)-1));
    u8IpLen += sizeof(au8EnableSub1)-1;
  }
  else
  {
    MEM_CPY(&au8IpBuf[u8IpLen], &au8DisableSub1[0], (sizeof(au8DisableSub1)-1));
    u8IpLen += sizeof(au8DisableSub1)-1;
  }
    
  //IP地址获取
  if(sDtuCfg[IP_SUB_1].u8CfgIpLen == 4)
  {
    sprintf(&au8IpBuf[u8IpLen],"%d", sDtuCfg[IP_SUB_1].au8CfgIpAddr[0]);
    u8IpLen += GetDataWidth(sDtuCfg[IP_SUB_1].au8CfgIpAddr[0]);
    au8IpBuf[u8IpLen++] = '.';
    
    sprintf(&au8IpBuf[u8IpLen],"%d", sDtuCfg[IP_SUB_1].au8CfgIpAddr[1]);
    u8IpLen += GetDataWidth(sDtuCfg[IP_SUB_1].au8CfgIpAddr[1]);
    au8IpBuf[u8IpLen++] = '.';
    
    sprintf(&au8IpBuf[u8IpLen],"%d", sDtuCfg[IP_SUB_1].au8CfgIpAddr[2]);
    u8IpLen += GetDataWidth(sDtuCfg[IP_SUB_1].au8CfgIpAddr[2]);
    au8IpBuf[u8IpLen++] = '.';
    
    sprintf(&au8IpBuf[u8IpLen],"%d", sDtuCfg[IP_SUB_1].au8CfgIpAddr[3]);
    u8IpLen += GetDataWidth(sDtuCfg[IP_SUB_1].au8CfgIpAddr[3]);
    au8IpBuf[u8IpLen++] = ',';
  }
  else
  {
    MEM_CPY(&au8IpBuf[u8IpLen], &sDtuCfg[IP_SUB_1].au8CfgIpAddr[0], sDtuCfg[IP_SUB_1].u8CfgIpLen);
    u8IpLen += sDtuCfg[IP_SUB_1].u8CfgIpLen;
    au8IpBuf[u8IpLen++] = ',';
  }

  //配置端口号
  u16CfgCom = (UINT16)(((UINT16)sDtuCfg[IP_SUB_1].au8CfgComIndex[0]) << 8) + sDtuCfg[IP_SUB_1].au8CfgComIndex[1];
  sprintf(&au8IpBuf[u8IpLen], "%d", u16CfgCom);
  u8IpLen += GetDataWidth(u16CfgCom);
  au8IpBuf[u8IpLen++] = '\r';
  
  //将DTU进入配置模式
  u8CfgState = DtuEnterConfigMode();
  if(DTU_CFG_MODE == u8CfgState)
  {
    //清空BUF1
    Uart1EmptyRxBuf();
    //配置UART
    UartSendBuffer(UART1, &au8IpBuf[0], u8IpLen);
    Delay(1000);
    if(Uart1GetRxDataLength() > 2)
    {
      au8IpBuf[0] = Uart1GetByte();
      au8IpBuf[1] = Uart1GetByte();
      au8IpBuf[2] = Uart1GetByte();
      if(0 != MEM_CMP(&au8IpBuf[0], &au8BackOk[0], 3))
      {
        //发复位命令
        UartSendBuffer(UART1, &au8Reset[0], (sizeof(au8Reset)-1));
        return DTU_IP_SUB1_ERROR;
      }     
      //发复位命令
      UartSendBuffer(UART1, &au8Reset[0], (sizeof(au8Reset)-1));
      g_u8NeedSaved |= SAVE_DTU_SUB1;
        
      return DTU_CFG_OK;
    }
   
    return DTU_CFG_ERROR;
  }
  else
  {
    //发复位命令
    UartSendBuffer(UART1, &au8Reset[0], (sizeof(au8Reset)-1));
    
    return DTU_CFG_ERROR;
  }
  
}

/*****************************************************************************
 函 数 名  : McuConfigDtuSub2
 功能描述  : Config dtu module, 

 输入参数  : UINT8 u8State
 返 回 值  :
*****************************************************************************/
UINT8 McuConfigDtuSub2(UINT8 u8CfgEnable)
{
  UINT8  u8IpLen;   
  char  au8IpBuf[64];
  UINT16 u16CfgCom;
  UINT8  u8CfgState = 0xff;
    
  u8IpLen = 0;
  MEM_SET(&au8IpBuf[u8IpLen], 0, sizeof(au8IpBuf));
  if(u8CfgEnable == DTU_CFG_ENABLE)
  {
    MEM_CPY(&au8IpBuf[u8IpLen], &au8EnableSub2[0], (sizeof(au8EnableSub1)-1));
    u8IpLen += sizeof(au8EnableSub1)-1;
  }
  else
  {
    MEM_CPY(&au8IpBuf[u8IpLen], &au8DisableSub2[0], (sizeof(au8DisableSub1)-1));
    u8IpLen += sizeof(au8DisableSub1)-1;
  }
    
  //IP地址获取
  if(sDtuCfg[IP_SUB_2].u8CfgIpLen == 4)
  {
    sprintf(&au8IpBuf[u8IpLen],"%d", sDtuCfg[IP_SUB_2].au8CfgIpAddr[0]);
    u8IpLen += GetDataWidth(sDtuCfg[IP_SUB_2].au8CfgIpAddr[0]);
    au8IpBuf[u8IpLen++] = '.';
    
    sprintf(&au8IpBuf[u8IpLen],"%d", sDtuCfg[IP_SUB_2].au8CfgIpAddr[1]);
    u8IpLen += GetDataWidth(sDtuCfg[IP_SUB_2].au8CfgIpAddr[1]);
    au8IpBuf[u8IpLen++] = '.';
    
    sprintf(&au8IpBuf[u8IpLen],"%d", sDtuCfg[IP_SUB_2].au8CfgIpAddr[2]);
    u8IpLen += GetDataWidth(sDtuCfg[IP_SUB_2].au8CfgIpAddr[2]);
    au8IpBuf[u8IpLen++] = '.';
    
    sprintf(&au8IpBuf[u8IpLen],"%d", sDtuCfg[IP_SUB_2].au8CfgIpAddr[3]);
    u8IpLen += GetDataWidth(sDtuCfg[IP_SUB_2].au8CfgIpAddr[3]);
    au8IpBuf[u8IpLen++] = ',';
  }
  else
  {
    MEM_CPY(&au8IpBuf[u8IpLen], &sDtuCfg[IP_SUB_2].au8CfgIpAddr[0], sDtuCfg[IP_SUB_2].u8CfgIpLen);
    u8IpLen += sDtuCfg[IP_SUB_2].u8CfgIpLen;
    au8IpBuf[u8IpLen++] = ',';
  }

  //配置端口号
  u16CfgCom = (UINT16)(((UINT16)sDtuCfg[IP_SUB_2].au8CfgComIndex[0]) << 8) + sDtuCfg[IP_SUB_2].au8CfgComIndex[1];
  sprintf(&au8IpBuf[u8IpLen], "%d", u16CfgCom);
  u8IpLen += GetDataWidth(u16CfgCom);
  au8IpBuf[u8IpLen++] = '\r';
  
  //将DTU进入配置模式
  u8CfgState = DtuEnterConfigMode();
  if(DTU_CFG_MODE == u8CfgState)
  {
    //清空BUF1
    Uart1EmptyRxBuf();
    //配置UART
    UartSendBuffer(UART1, &au8IpBuf[0], u8IpLen);
    Delay(1000);
    if(Uart1GetRxDataLength() > 2)
    {
      au8IpBuf[0] = Uart1GetByte();
      au8IpBuf[1] = Uart1GetByte();
      au8IpBuf[2] = Uart1GetByte();
      if(0 != MEM_CMP(&au8IpBuf[0], &au8BackOk[0], 3))
      {
        //发复位命令
        UartSendBuffer(UART1, &au8Reset[0], (sizeof(au8Reset)-1));
        return DTU_UART_SET_ERROR;
      }
      
      //发复位命令
      UartSendBuffer(UART1, &au8Reset[0], (sizeof(au8Reset)-1));
      g_u8NeedSaved |= SAVE_DTU_SUB2;
      
      return DTU_CFG_OK;
    }
    
    return DTU_CFG_ERROR;
  }
  else
  {
    //发复位命令
    UartSendBuffer(UART1, &au8Reset[0], (sizeof(au8Reset)-1));
    return DTU_CFG_ERROR;
  }
}

/*****************************************************************************
 函 数 名  : McuConfigDtuSub3
 功能描述  : Config dtu module, 

 输入参数  : UINT8 u8State
 返 回 值  :
*****************************************************************************/
UINT8 McuConfigDtuSub3(UINT8 u8CfgEnable)
{
  UINT8  u8IpLen;   
  char  au8IpBuf[64];
  UINT16 u16CfgCom;
  UINT8  u8CfgState = 0xff;
    
  u8IpLen = 0;
  MEM_SET(&au8IpBuf[u8IpLen], 0, sizeof(au8IpBuf));
  if(u8CfgEnable == DTU_CFG_ENABLE)
  {
    MEM_CPY(&au8IpBuf[u8IpLen], &au8EnableSub3[0], (sizeof(au8EnableSub1)-1));
    u8IpLen += sizeof(au8EnableSub1)-1;
  }
  else
  {
    MEM_CPY(&au8IpBuf[u8IpLen], &au8DisableSub3[0], (sizeof(au8DisableSub1)-1));
    u8IpLen += sizeof(au8DisableSub1)-1;
  }
    
  //IP地址获取
  if(sDtuCfg[IP_SUB_3].u8CfgIpLen == 4)
  {
    sprintf(&au8IpBuf[u8IpLen],"%d", sDtuCfg[IP_SUB_3].au8CfgIpAddr[0]);
    u8IpLen += GetDataWidth(sDtuCfg[IP_SUB_3].au8CfgIpAddr[0]);
    au8IpBuf[u8IpLen++] = '.';
    
    sprintf(&au8IpBuf[u8IpLen],"%d", sDtuCfg[IP_SUB_3].au8CfgIpAddr[1]);
    u8IpLen += GetDataWidth(sDtuCfg[IP_SUB_3].au8CfgIpAddr[1]);
    au8IpBuf[u8IpLen++] = '.';
    
    sprintf(&au8IpBuf[u8IpLen],"%d", sDtuCfg[IP_SUB_3].au8CfgIpAddr[2]);
    u8IpLen += GetDataWidth(sDtuCfg[IP_SUB_3].au8CfgIpAddr[2]);
    au8IpBuf[u8IpLen++] = '.';
    
    sprintf(&au8IpBuf[u8IpLen],"%d", sDtuCfg[IP_SUB_3].au8CfgIpAddr[3]);
    u8IpLen += GetDataWidth(sDtuCfg[IP_SUB_3].au8CfgIpAddr[3]);
    au8IpBuf[u8IpLen++] = ',';
  }
  else
  {
    MEM_CPY(&au8IpBuf[u8IpLen], &sDtuCfg[IP_SUB_3].au8CfgIpAddr[0], sDtuCfg[IP_SUB_3].u8CfgIpLen);
    u8IpLen += sDtuCfg[IP_SUB_3].u8CfgIpLen;
    au8IpBuf[u8IpLen++] = ',';
  }

  //配置端口号
  u16CfgCom = (UINT16)(((UINT16)sDtuCfg[IP_SUB_3].au8CfgComIndex[0]) << 8) + sDtuCfg[IP_SUB_3].au8CfgComIndex[1];
  sprintf(&au8IpBuf[u8IpLen], "%d", u16CfgCom);
  u8IpLen += GetDataWidth(u16CfgCom);
  au8IpBuf[u8IpLen++] = '\r';
  
  //将DTU进入配置模式
  u8CfgState = DtuEnterConfigMode();
  if(DTU_CFG_MODE == u8CfgState)
  {
    //清空BUF1
    Uart1EmptyRxBuf();
    //配置UART
    UartSendBuffer(UART1, &au8IpBuf[0], u8IpLen);
    Delay(1000);
    if(Uart1GetRxDataLength() > 2)
    {
      au8IpBuf[0] = Uart1GetByte();
      au8IpBuf[1] = Uart1GetByte();
      au8IpBuf[2] = Uart1GetByte();
      if(0 != MEM_CMP(&au8IpBuf[0], &au8BackOk[0], 3))
      {
        //发复位命令
        UartSendBuffer(UART1, &au8Reset[0], (sizeof(au8Reset)-1));
        return DTU_UART_SET_ERROR;
      }
      
      //发复位命令
      UartSendBuffer(UART1, &au8Reset[0], (sizeof(au8Reset)-1));
      g_u8NeedSaved |= SAVE_DTU_SUB3;
      
      return DTU_CFG_OK;
    }
    
    return DTU_CFG_ERROR;
  }
  else
  {
    //发复位命令
    UartSendBuffer(UART1, &au8Reset[0], (sizeof(au8Reset)-1));
    return DTU_CFG_ERROR;
  }
}

/*****************************************************************************
 函 数 名  : McuConfigDtuSub1
 功能描述  : Config dtu module, 

 输入参数  : UINT8 u8State
 返 回 值  :
*****************************************************************************/
UINT8 McuConfigDtuSub4(UINT8 u8CfgEnable)
{
  UINT8  u8IpLen;   
  char  au8IpBuf[64];
  UINT16 u16CfgCom;
  UINT8  u8CfgState = 0xff;
    
  u8IpLen = 0;
  MEM_SET(&au8IpBuf[u8IpLen], 0, sizeof(au8IpBuf));
  if(u8CfgEnable == DTU_CFG_ENABLE)
  {
    MEM_CPY(&au8IpBuf[u8IpLen], &au8EnableSub4[0], (sizeof(au8EnableSub1)-1));
    u8IpLen += sizeof(au8EnableSub1)-1;
  }
  else
  {
    MEM_CPY(&au8IpBuf[u8IpLen], &au8DisableSub4[0], (sizeof(au8DisableSub1)-1));
    u8IpLen += sizeof(au8DisableSub1)-1;
  }
    
  //IP地址获取
  if(sDtuCfg[IP_SUB_4].u8CfgIpLen == 4)
  {
    sprintf(&au8IpBuf[u8IpLen],"%d", sDtuCfg[IP_SUB_4].au8CfgIpAddr[0]);
    u8IpLen += GetDataWidth(sDtuCfg[IP_SUB_4].au8CfgIpAddr[0]);
    au8IpBuf[u8IpLen++] = '.';
    
    sprintf(&au8IpBuf[u8IpLen],"%d", sDtuCfg[IP_SUB_4].au8CfgIpAddr[1]);
    u8IpLen += GetDataWidth(sDtuCfg[IP_SUB_4].au8CfgIpAddr[1]);
    au8IpBuf[u8IpLen++] = '.';
    
    sprintf(&au8IpBuf[u8IpLen],"%d", sDtuCfg[IP_SUB_4].au8CfgIpAddr[2]);
    u8IpLen += GetDataWidth(sDtuCfg[IP_SUB_4].au8CfgIpAddr[2]);
    au8IpBuf[u8IpLen++] = '.';
    
    sprintf(&au8IpBuf[u8IpLen],"%d", sDtuCfg[IP_SUB_4].au8CfgIpAddr[3]);
    u8IpLen += GetDataWidth(sDtuCfg[IP_SUB_4].au8CfgIpAddr[3]);
    au8IpBuf[u8IpLen++] = ',';
  }
  else
  {
    MEM_CPY(&au8IpBuf[u8IpLen], &sDtuCfg[IP_SUB_4].au8CfgIpAddr[0], sDtuCfg[IP_SUB_4].u8CfgIpLen);
    u8IpLen += sDtuCfg[IP_SUB_4].u8CfgIpLen;
    au8IpBuf[u8IpLen++] = ',';
  }

  //配置端口号
  u16CfgCom = (UINT16)(((UINT16)sDtuCfg[IP_SUB_4].au8CfgComIndex[0]) << 8) + sDtuCfg[IP_SUB_4].au8CfgComIndex[1];
  sprintf(&au8IpBuf[u8IpLen], "%d", u16CfgCom);
  u8IpLen += GetDataWidth(u16CfgCom);
  au8IpBuf[u8IpLen++] = '\r';
  
  //将DTU进入配置模式
  u8CfgState = DtuEnterConfigMode();
  if(DTU_CFG_MODE == u8CfgState)
  {
    //清空BUF1
    Uart1EmptyRxBuf();
    //配置UART
    UartSendBuffer(UART1, &au8IpBuf[0], u8IpLen);
    Delay(1000);
    if(Uart1GetRxDataLength() > 2)
    {
      au8IpBuf[0] = Uart1GetByte();
      au8IpBuf[1] = Uart1GetByte();
      au8IpBuf[2] = Uart1GetByte();
      if(0 != MEM_CMP(&au8IpBuf[0], &au8BackOk[0], 3))
      {
        //发复位命令
        UartSendBuffer(UART1, &au8Reset[0], (sizeof(au8Reset)-1));
        return DTU_UART_SET_ERROR;
      }
      
      //发复位命令
      UartSendBuffer(UART1, &au8Reset[0], (sizeof(au8Reset)-1));
      g_u8NeedSaved |= SAVE_DTU_SUB4;
      
      return DTU_CFG_OK;
    }
    
    return DTU_CFG_ERROR;
  }
  else
  {
    //发复位命令
    UartSendBuffer(UART1, &au8Reset[0], (sizeof(au8Reset)-1));
    return DTU_CFG_ERROR;
  }
}



  

/*****************************************************************************
 函 数 名  : SendTargetInfo
 功能描述  : Send infomation to center through DTU module

 输入参数  : USHORT16 usTime
 返 回 值  :
*****************************************************************************/
void SendTargetInfo(void)
{
  UINT8 j;
  UINT8 i=0;
  UINT8 au8Data[120];
  UINT8 u8CheckSum=0;
  
  MEM_SET(au8Data, 0, 120);
  
  //数据帧头
  au8Data[i++] = '$';

  //指令码 -- 4byte
  au8Data[i++] = 'X';
  au8Data[i++] = 'X';
  au8Data[i++] = 'F';
  au8Data[i++] = 'S';
  
  au8Data[i++] = ',';

  //空出UINT16数据长度，最后计算.位置6，7
  au8Data[i++] = 0;    
  i++;    
  
  au8Data[i++] = ',';
  
  //设备编号
  au8Data[i++] = (UINT8)((sDisplay.u32TmlIndex & 0xff000000) >> 24);
  au8Data[i++] = (UINT8)((sDisplay.u32TmlIndex & 0x00ff0000) >> 16);
  au8Data[i++] = (UINT8)((sDisplay.u32TmlIndex & 0x0000ff00) >> 8);
  au8Data[i++] = (UINT8)(sDisplay.u32TmlIndex & 0x000000ff);
  
  au8Data[i++] = ',';

  //发送设备类型
  au8Data[i++] = sDisplay.u8TmlType;

  au8Data[i++] = ',';

  //发送设备型号
  for(j=0; j<sDisplay.u8InfoLen; j++) 
  {
    au8Data[i++] = sDisplay.au8TmlInfo[j];
  }
  
  au8Data[i++] = ',';

  //发送的纬度信息
  for(j=0; j<9; j++)
  {
    au8Data[i++] = sGPSBuf[GPS_MAX_SIZE-1].GPS_X[j];
  }

  au8Data[i++] = ',';

  //发送速度信息
  for(j=0; j<10; j++)
  {
	au8Data[i++] = sGPSBuf[GPS_MAX_SIZE-1].GPS_Y[j];
  }

  au8Data[i++] = ',';
  
  //发送经度信息
  for(j=0; j<5; j++)
  {
	  au8Data[i++] = sGPSBuf[GPS_MAX_SIZE-1].Course[j];
  }
  
  au8Data[i++] = ',';
  
  //发送方位角
  for(j=0; j<5; j++)
  {
	  au8Data[i++] = sGPSBuf[GPS_MAX_SIZE-1].Speed[j];
  }

  //发送电量信息
  au8Data[i++] = sSendData.u8Battery;
  
  au8Data[i++] = ',';

  //校验前缀
  au8Data[i++] = '*';
  for(j=1; j<au8Data[4]+1; j++)
  {
    u8CheckSum ^= au8Data[j];
  }

  au8Data[i++] = u8CheckSum;

  //发送数据长度
  au8Data[7] = i;

  au8Data[i++] = '\r';

  Delay(50);
  UartSendBuffer(UART1, &au8Data[0], i);
  Delay(3000);
}

/******************************************************************************
 * 函数名称：DtuInfoSend
 * 描述：发送信息到中心。
 *
 * 参数：无
 * 返回：无
 *****************************************************************************/
void DtuInfoSend(void)
{
  if(g_bTmlStart == TRUE)
  {
    if(sDisplay.bDtuValid == TRUE)
    {
      if(g_bDtuFlag == TRUE)
      {
        if(g_bDtuSend == TRUE)
        {
          g_bDtuFlag = FALSE;
          g_bDtuSend = FALSE;
    
          SendTargetInfo();
    
          MEM_SET(&sSendData.u8Index, 0, sizeof(sSendData));
    
          g_bDtuSendCompleted = TRUE;
          
    #ifdef DEBUG
        UartSendString(UART2, "信息发送完成！\r\n");
    #endif
        }
      }
    }
  }
}
#endif
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                   DTU配置设备信息解析
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void DtuCmdInit(void)
{
  sUartCmdCb[DTU_TYPE].eAssembleState = E_UART_CMD_RECEIVING_HEADER;
  sUartCmdCb[DTU_TYPE].pu8AssembleBuf = (UINT8 *)au8DtuBodyBuf;
  sUartCmdCb[DTU_TYPE].u8BufDataLength = 0;

  //串口0接收超时定时器。
  asTimer[TIMER_UART1].u16LifeTime = TIMER_TICK / 2;
  asTimer[TIMER_UART1].u16LifeOldTime = TIMER_TICK / 2;
  asTimer[TIMER_UART1].bEnable = FALSE;
}


/***********************************************************************************************
  函数名称：UartCmdAssemble(UINT8 u8InData)
  函数功能：把接收的数据组合成有效的数据包

  输入：u8InData
  输出：无
***********************************************************************************************/
void Uart1CmdAssemble(UINT8 u8InData)
{
  switch(sUartCmdCb[DTU_TYPE].eAssembleState)
  {
    case E_UART_CMD_RECEIVING_HEADER:
    {
      if(u8InData == '$')
      {
        sUartCmdCb[DTU_TYPE].pu8AssembleBuf[0] = u8InData;
        sUartCmdCb[DTU_TYPE].u8BufDataLength = 1;
	sUartCmdCb[DTU_TYPE].eAssembleState = E_UART_CMD_RECEIVING_BODY;	
      }
      break;
     }	
		
    case E_UART_CMD_RECEIVING_BODY:
    {
      sUartCmdCb[DTU_TYPE].pu8AssembleBuf[sUartCmdCb[DTU_TYPE].u8BufDataLength++] = u8InData;
      if(u8InData == '\r')
      {
        sUartCmdCb[DTU_TYPE].eAssembleState = E_UART_CMD_RECEIVING_VALID;
      }
      else
      {
        if(sUartCmdCb[DTU_TYPE].u8BufDataLength == MAX_BODY_SIZE)	
        {
          sUartCmdCb[DTU_TYPE].eAssembleState = E_UART_CMD_RECEIVING_HEADER;
          sUartCmdCb[DTU_TYPE].u8BufDataLength = 0;
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
      sUartCmdCb[DTU_TYPE].eAssembleState = E_UART_CMD_RECEIVING_HEADER;
      sUartCmdCb[DTU_TYPE].u8BufDataLength = 0;
    }
  }	
}


/***************************************************************************
//函数名称：CmdSetRun
//功能描述：串口设置运行
//
//参数：无
//返回：无
***************************************************************************/
void DtuCmdRun(void)
{
  UINT8 u8Cmd;
  UINT8 u8CheckSumState=CMD_NONE;

  while((Uart1GetRxDataLength() > 0) && (sUartCmdCb[DTU_TYPE].eAssembleState != E_UART_CMD_RECEIVING_VALID))
  {
    Uart1CmdAssemble(Uart1GetByte());

    if(asTimer[TIMER_UART1].bEnable == TRUE)
    {
      asTimer[TIMER_UART1].bEnable = FALSE;
    }
  }
  
  //数据包有效，对可用的指令进行解析
    if(sUartCmdCb[DTU_TYPE].eAssembleState == E_UART_CMD_RECEIVING_VALID)
    {
      u8Cmd = ConfigPackType(au8DtuBodyBuf, sUartCmdCb[DTU_TYPE].u8BufDataLength);
      //建立索引表
      CreatCmdIndex(au8DtuBodyBuf);  
      //信息配置命令
     switch(u8Cmd)
      {
        case XXPZ:
        {
          u8CheckSumState = ConfigInfoProcess(DTU_TYPE, au8DtuBodyBuf);   //提取配置信息
          break;
        }
        
        case DMPZ:
        {
          u8CheckSumState = MainServerConfigProcess(DTU_TYPE, au8DtuBodyBuf);   //提取配置信息
          break;
        }
        
        case S1PZ:
        {
          u8CheckSumState = Sub1ConfigProcess(DTU_TYPE, au8DtuBodyBuf);   //提取配置信息
          break;
        }
        
        case S2PZ:
        {
          u8CheckSumState = Sub2ConfigProcess(DTU_TYPE, au8DtuBodyBuf);   //提取配置信息
          break;
        }
        
        case S3PZ:
        {
          u8CheckSumState = Sub3ConfigProcess(DTU_TYPE, au8DtuBodyBuf);   //提取配置信息
          break;
        }
        
        case S4PZ:
        {
          u8CheckSumState = Sub4ConfigProcess(DTU_TYPE, au8DtuBodyBuf);   //提取配置信息
          break;
        }
         
        case ZDQD:
        {
          g_bTmlStart = TRUE;
          break;
        }
        
        case ZDTZ:
        {
          
          break;
        }
        
        case ZDGJ:
        {
          u8CheckSumState = PowerDownProcess(DTU_TYPE, au8DtuBodyBuf);          
          break;
        }
        
        case TSKG:
        {
          u8CheckSumState = DebugProcess(DTU_TYPE, au8DtuBodyBuf);
          break;
        }
        
        case BBHM:
        {
          u8CheckSumState = GetVersionProcess(DTU_TYPE, au8DtuBodyBuf);
          break;
        }
        
        case DTHM:
        {
          u8CheckSumState = ReadDtuConfigProcess(DTU_TYPE, au8DtuBodyBuf);
          break;
        }
        
        default: u8CheckSumState = CMD_NONE;
      }
      
     //打印调试信息，配置状态 
     if(g_bDebug == TRUE)
     {
        switch(u8CheckSumState)
        {
          case CMD_NONE:  
          {
            UartSendString(UART2, "无效的配置命令！\r\n"); 
            break;
          }
          
          case CMD_XXPZ_OK:
          {
            UartSendString(UART2, "信息远程配置成功！\r\n");
            break;
          }
          
          case CMD_XXPZ_ERROR:
          {
            UartSendString(UART2, "信息远程配置失败！\r\n");
            break;
          }
          
          case CMD_DMPZ_OK:
          {
            UartSendString(UART2, "主服务器远程配置成功！\r\n");
            break;
          }
          
          case CMD_DMPZ_ERROR:
          {
            UartSendString(UART2, "主服务器远程配置失败！\r\n");
            break;
          }
          
          case CMD_S1PZ_OK:
          {
            UartSendString(UART2, "分中心1远程配置成功！\r\n");
            break;
          }
          
          case CMD_S1PZ_ERROR:
          {
            UartSendString(UART2, "分中心1远程配置失败！\r\n");
            break;
          }
          
          case CMD_S2PZ_OK:
          {
            UartSendString(UART2, "分中心2远程配置成功！\r\n");
            break;
          }
          
          case CMD_S2PZ_ERROR:
          {
            UartSendString(UART2, "分中心2远程配置失败！\r\n");
            break;
          }
          
          case CMD_S3PZ_OK:
          {
            UartSendString(UART2, "分中心3远程配置成功！\r\n");
            break;
          }
          
          case CMD_S3PZ_ERROR:
          {
            UartSendString(UART2, "分中心3远程配置失败！\r\n");
            break;
          }
          
          case CMD_S4PZ_OK:
          {
            UartSendString(UART2, "分中心4远程配置成功！\r\n");
            break;
          }
          
          case CMD_S4PZ_ERROR:
          {
            UartSendString(UART2, "分中心4远程配置失败！\r\n");
            break;
          }
          
          default: break;
        }
     }
      //处理完相关指令，清除buf
      MEM_SET(&au8DtuBodyBuf[0], 0, sizeof(au8DtuBodyBuf));
      sUartCmdCb[DTU_TYPE].eAssembleState = E_UART_CMD_RECEIVING_HEADER;
      sUartCmdCb[DTU_TYPE].u8BufDataLength = 0;
    }
    else
    {
      //规定时间内无数据，指令复位，重新提取有效数据
      if(asTimer[TIMER_UART1].bEnable == TRUE)                
      {
        if(asTimer[TIMER_UART1].u16LifeTime == 0)
        {
          sUartCmdCb[DTU_TYPE].eAssembleState = E_UART_CMD_RECEIVING_HEADER;
          sUartCmdCb[DTU_TYPE].u8BufDataLength = 0;

          asTimer[TIMER_UART1].bEnable = FALSE;
        }	
       }
       else
       {
          if(sUartCmdCb[DTU_TYPE].u8BufDataLength != 0)	
          {
            asTimer[TIMER_UART1].u16LifeTime = asTimer[TIMER_UART1].u16LifeOldTime;
            asTimer[TIMER_UART1].bEnable = TRUE;
          }
        }	
     }

  sDisplay.bDtuValid = DtuSvrCheck();
}

