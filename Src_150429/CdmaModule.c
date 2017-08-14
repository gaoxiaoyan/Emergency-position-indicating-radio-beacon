
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
#include "global.h"
#include "CdmaModule.h"
#include "DataType.h"
#include "Uart0.h"
#include "Uart3.h"
#include "SystemCtl.h"
#include "string.h"
#include "Timer.h"
#include "SoftTimer.h"
#include "GpsParse.h"
#include "stdio.h"
#include "OS_MEM.h"
#include "Rtc.h"

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

//查询模块型号
const UINT8 au8GetModuleIndex[] = {"AT+CGMM\r"};
//串口波特率设置
const UINT8 au8UartSet[] = {"AT+UART=9600,ON,8,N,1\r"};
//设置32khz深度睡眠模式
const UINT8 au8EnableZDSLEEP[] = {"AT+ZDSLEEP=1\r"};
const UINT8 au8DisableZDSLEEP[] = {"AT+ZDSLEEP=0\r"};
//DTR引脚休眠控制
const UINT8 au8EnableZDSLEEPDTR[] = {"AT+ZDSLEEPDTR=1\r"};
const UINT8 au8DisableZDSLEEPDTR[] = {"AT+ZDSLEEPDTR=0\r"};
//获取CDMA时间
const UINT8 au8GetCdmaTime[] = {"AT+ZDSLEEPDTR=2\r"};
//复位模块
const UINT8 au8DisableZRST[] = {"AT+ZRST=0\r"};
//允许复位,1分钟后复位
//const UINT8 au8EnableZRST[] = {"AT+ZRST=1,"00:01"\r"};

//信号强度查询，返回：+CSQ: 28,99 OK，16以下可能呼叫失败。
const UINT8 au8CSQ[] = {"AT+CSQ\r"};
//时钟查询 返回：+CCLK:"2004/02/09，17：34：23.694"当前网络时间。
const UINT8 au8CCLK[] = {"AT+CCLK?\r"};

//设置本机号码
const UINT8 au8SetCNUM[] = {"AT+CNUM="};
const UINT8 au8GetCNUM[] = {"AT+CNUM?"};

//模块指示
const UINT8 au8ZIND[] = {"AT+ZIND="};

/////////////////////////////////////////////////////////////
// 短信服务
//选择短信服务
const UINT8 au8MsgType[] = {"AT+CSMS?\r"};
//设置短信为英文， ascii 格式
const UINT8 au8ZMSGL_ASCII[] = {"AT+ZMSGL=1,2\r"};
//设置短信为汉语 ， unicode格式
const UINT8 au8ZMSCL_UNICODE[] = {"AT+ZMSGL=6,4\r"};
//发送短息
typedef struct _tsCMGS
{
  UINT8 *pCmd; //存放AT命令 AT+CMGS=
  
  UINT8 *pPhoneNum;  //存放电话号码，加前后的“”
  
  UINT8 * au8Info;    //存放所发的信息指针

}tsCMGS, *pCMGS;


/*****************************************************************************
 函 数 名  : CdmaSendMsg
 功能描述  : CDMA模块发送短信

 输入参数  : none
 返 回 值  : none,
*****************************************************************************/
BOOL CdmaSendMsg(const UINT8 * ptr, UINT8 u8Len)
{
  Uart0SendBuffer(ptr, u8Len);
  return 1;
}

/*****************************************************************************
 函 数 名  : AssembleCdmaMsg
 功能描述  : AssembleCdmaMsg

 输入参数  : 发送信息指针
 返 回 值  : none,
*****************************************************************************/
void AssembleCdmaMsg(UINT8 *pBuf, UINT8 u8Len)
{
  UINT8 *pMsg;
  UINT8 i=0;
  
  pMsg = OSMemGet(pSmallMem, &err);
//  memset(pMsg, 0, BUF_SMALL_SIZE);
  
  memcpy(&pMsg[i], "AT+CMGS=", 8);
  i += 8; 
  pMsg[i++] = '"';
  memcpy(&pMsg[i], "18561795199", 11);
  i += 11;
  pMsg[i++] = '"';
  pMsg[i++] = '\r';
  
  memcpy(&pMsg[i], pBuf, u8Len);
  i += u8Len;
  
  pMsg[i++] = 0x1A;   //结束符
  
  CdmaSendMsg(pMsg, i);

  //释放内存
  OSMemPut(pSmallMem, pMsg);
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
  
 	//CDMA RI中断，下降沿触发中断
 	CdmaRiIesEdge();
 	CdmaRiIntEnable();
 	CdmaRiDir &= ~CdmaRi;

  //cdma模块复位
  SetCdmaReset(0);
  g_u16TimerDelay=0;
  while(g_u16TimerDelay < 2);    //低电平延时20ms
  SetCdmaReset(1);
  //cdma模块开机
  SetCdmaOnOff(1);
  g_u16TimerDelay=0;
  while(g_u16TimerDelay < 1);    //延时10ms
  SetCdmaOnOff(0);
  g_u16TimerDelay=0;
  while(g_u16TimerDelay < 200);  //延时2s
  SetCdmaOnOff(1);
  
  CdmaSendMsg(au8ZMSGL_ASCII, sizeof(au8ZMSGL_ASCII));
  
}

/*****************************************************************************
 函 数 名  : GetCdmaModule
 功能描述  : CDMA模块初始化

 输入参数  : none
 返 回 值  : none,
*****************************************************************************/
void GetCdmaModule(void)
{
  UINT8 i;
  SINT8 sDog;
  UINT8 *pRxBuf;
 
  pRxBuf = OSMemGet(pSmallMem, &err);
  memset(pRxBuf, 0, BUF_SMALL_SIZE);

  sDog = SwdGet();
  SwdSetLifeTime(sDog, 2000);
  SwdEnable(sDog);
  
  //清空串口缓冲区
  Uart0EmptyRxBuf();
  //发送模块型号命令
  Uart0SendBuffer(au8GetModuleIndex, sizeof(au8GetModuleIndex));
  
  while(TRUE)
  {
    if(Uart0GetRxDataLength() > 0)
    {
      pRxBuf[i++] = Uart0GetByte();
      if((pRxBuf[i-1] == 'O') && (pRxBuf[i] == 'K'))
      {
        break;
      }
    }
    
    if(SwdGetLifeTime(sDog) == 0)
    {
      SwdReset(sDog);
      i = 0;
      
      //清空串口缓冲区
      Uart0EmptyRxBuf();
      //发送模块型号命令
      Uart0SendBuffer(au8GetModuleIndex, sizeof(au8GetModuleIndex));
    } 
  }
  
  if(g_bDebug == TRUE)
  {
    if(strstr(pRxBuf, "MC8332") != NULL)
    {
      Uart3SendString("#Detect ZTE CDMA Module. The Product Model Is MC8332!\r\n");
    }
    else
    {
      Uart3SendString("#CDMA Module To Find The Failure!\r\n");
    }
  }
  //释放内存
  OSMemPut(pSmallMem, pRxBuf);
  //释放软件定时器
  SwdFree(sDog);
}

/*****************************************************************************
 函 数 名  : GetCdmaCSQ
 功能描述  : 获取CDMA信号强度

 输入参数  : none
 返 回 值  : 信号强度,0~31
*****************************************************************************/
UINT8 GetCdmaCSQ(void)
{
  UINT8 i;
  SINT8 sDog;
  UINT8 *pRxBuf;
  UINT8 u8Temp;
  
  pRxBuf = OSMemGet(pSmallMem, &err);
  memset(pRxBuf, 0, BUF_SMALL_SIZE);

  sDog = SwdGet();
  SwdSetLifeTime(sDog, 2000);
  SwdEnable(sDog);
  //清空串口缓冲区
  Uart0EmptyRxBuf();
  //发送模块型号命令
  Uart0SendBuffer(au8CSQ, sizeof(au8CSQ));
  
  while(TRUE)
  {
    if(Uart0GetRxDataLength() > 0)
    {
      pRxBuf[i++] = Uart0GetByte();
      if((pRxBuf[i-1] == 'O') && (pRxBuf[i] == 'K'))
      {
        i = 0;
        while((pRxBuf[i] != '"') && (i<BUF_SMALL_SIZE))
        {
          i++;
        }
  
        if((pRxBuf[i-2] >= 0x30) && (pRxBuf[i-1] >= 0x30))
        {
          u8Temp = (pRxBuf[i-2]-0x30)*10 + (pRxBuf[i-1]-0x30);
          break;
        }
      }
    }
    
    if(SwdGetLifeTime(sDog) == 0)
    {
      SwdReset(sDog);
      i = 0;
      
      //清空串口缓冲区
      Uart0EmptyRxBuf();
      //发送模块型号命令
      Uart0SendBuffer(au8CSQ, sizeof(au8CSQ));
    } 
  }
    
  //释放内存
  OSMemPut(pSmallMem, pRxBuf);
  //释放软件定时器
  SwdFree(sDog);
  
  return u8Temp;
}


/*****************************************************************************
 函 数 名  : SetCdmaTimeToRtc
 功能描述  : 获取CDMA时间, 设置到RTC

 输入参数  : none
 返 回 值  : TRUE -- 校准成功； FALSE -- 校准失败
*****************************************************************************/
BOOL SetCdmaTimeToRtc(void)
{
  UINT8 i,j;
  SINT8 sDog;
  UINT8 *pRxBuf;
  UINT8 au8Time[22];
  _tsTimeCb sTime;
  
  pRxBuf = OSMemGet(pSmallMem, &err);
  memset(pRxBuf, 0, BUF_SMALL_SIZE);

  sDog = SwdGet();
  SwdSetLifeTime(sDog, 2000);
  SwdEnable(sDog);
  //清空串口缓冲区
  Uart0EmptyRxBuf();
  //发送模块型号命令
  Uart0SendBuffer(au8CCLK, sizeof(au8CCLK));
  
  while(TRUE)
  {
    if(Uart0GetRxDataLength() > 48)
    {
      Uart0GetRxData(pRxBuf, 48);

      i = 0;
      while((pRxBuf[i] != '"') && (i<BUF_SMALL_SIZE))
      {
        i++;
      }
  
      j = ++i;
      while((pRxBuf[j] != '"') && (j<BUF_SMALL_SIZE))
      {
        j++;
      }
        
      break;
    }
    
    if(SwdGetLifeTime(sDog) == 0)
    {
      SwdReset(sDog);
      i = 0;
      
      //清空串口缓冲区
      Uart0EmptyRxBuf();
      //发送模块型号命令
      Uart0SendBuffer(au8CCLK, sizeof(au8CCLK));
    } 
  }
  //时间拷贝，忽略ms数据
  memcpy(au8Time, &pRxBuf[i], j-i);
  //释放内存
  OSMemPut(pSmallMem, pRxBuf);
  //释放软件定时器
  SwdFree(sDog);
  
  i = 2;   //年份偏移
  if((au8Time[i] >= 0x30) && (au8Time[i] <= 0x39))
    sTime.Year = (au8Time[i] - 0x30)*10 + (au8Time[i+1] - 0x30);
  else
    return FALSE;
  
  i += 3;   //月份偏移
  if((au8Time[i] >= 0x30) && (au8Time[i] <= 0x39))
    sTime.Month = (au8Time[i] - 0x30)*10 + (au8Time[i+1] - 0x30);
  else
    return FALSE;
  
  i += 3;   //日期偏移
  if((au8Time[i] >= 0x30) && (au8Time[i] <= 0x39))
    sTime.Date = (au8Time[i] - 0x30)*10 + (au8Time[i+1] - 0x30);
  else
    return FALSE;
  
  i += 3;   //时偏移
  if((au8Time[i] >= 0x30) && (au8Time[i] <= 0x39))
    sTime.Hour = (au8Time[i] - 0x30)*10 + (au8Time[i+1] - 0x30);
  else
    return FALSE;
  
  i += 3;   //分偏移
  if((au8Time[i] >= 0x30) && (au8Time[i] <= 0x39))
    sTime.Minute = (au8Time[i] - 0x30)*10 + (au8Time[i+1] - 0x30);
  else
    return FALSE;
  
  i += 3;   //秒偏移
  if((au8Time[i] >= 0x30) && (au8Time[i] <= 0x39))
    sTime.Second = (au8Time[i] - 0x30)*10 + (au8Time[i+1] - 0x30);
  else
    return FALSE;
  
//  SetRtcInt1Out();
  SetTimeToRtc(&sTime);
  
  return TRUE;
}