
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
#define  CDMA_EXT
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

//命令字母相加，取低8位作为指令代码
#define CMD_ZCANS         0x7F
#define CMD_ZCCNT         0x82
#define CMD_CMTZ          0x3E
#define CMD_ZCEND         0x74
#define CMD_CSQ           0xE7


//CDMA接收状态
typedef enum
{
    //命令接收头
    E_CDMA_RECEIVING_HEADER,
    //接收到的命令
    E_CDMA_RECEIVING_CMD,
    //数据主体
    E_CDMA_RECEIVING_BODY,
    //SMS READY命令
    E_CDMA_RECEIVING_SMS,
    //有效标志状态
    E_CDMA_RECEIVING_VALID

}teCdmaAssembleState;

/* 数据组包结构声明 */
typedef struct
{
  UINT8 *pu8AssembleBuf;  //组包区数据指针

  UINT8 u8Cmd;            //cdma接收命令

  UINT16 u16BufDataLen ;  //组包的数据长度

  UINT8 u8SmsLen;         //短信SMS REDAY接收长度

  teCdmaAssembleState eAssembleState;  //组包状态

  SINT8 sCdmaDog;         //cdma接收命令超时定时器

}_tsCdmaCmdCb;

_tsCdmaCmdCb sCdmaCmdCb;

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
//const UINT8 au8SetCNUM[] = {"AT+CNUM="};
//const UINT8 au8GetCNUM[] = {"AT+CNUM?"};

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

//函数声明
void CdmaCmdInit(void);


/*****************************************************************************
 函 数 名  : CdmaSendMsg
 功能描述  : CDMA模块发送短信

 输入参数  : none
 返 回 值  : none,
*****************************************************************************/
BOOL CdmaSendMsg(const UINT8 * ptr, UINT8 u8Len)
{
  Uart0EmptyRxBuf();
  Uart0SendBuffer(ptr, u8Len);
  return 1;
}

/*****************************************************************************
 函 数 名  : AssembleCdmaMsg
 功能描述  : AssembleCdmaMsg

 输入参数  : *pNum -- 发送号码的指针
             *pBuf -- 发送信息指针

 返 回 值  : TRUE, FALSE
*****************************************************************************/
const UINT8 au8CMGS[8] = {"AT+CMGS="};
BOOL AssembleCdmaMsg(UINT8 *pNum, UINT8 *pBuf, UINT8 u8Len)
{
  UINT8 *pMsg;
  UINT8 *pRxBuf;
  UINT8 i=0;
  SINT8 s8Dog;
  UINT8 u8Back;

  pMsg = OSMemGet(pSmallMem, &err);
  pRxBuf = OSMemGet(pSmallMem, &err);

  memcpy(&pMsg[i], &au8CMGS[0], 8);
  i += 8;
  pMsg[i++] = '"';
  memcpy(&pMsg[i], pNum, 11);
  i += 11;
  pMsg[i++] = '"';
  pMsg[i++] = '\r';

  memcpy(&pMsg[i], pBuf, u8Len);
  i += u8Len;

  pMsg[i++] = 0x1A;   //结束符

  StopWatchDog();
  s8Dog = SwdGet();
  SwdSetLifeTime(s8Dog, 2000);
  SwdEnable(s8Dog);
  CdmaSendMsg(pMsg, i);

  while(TRUE)
  {
    if(Uart0GetRxDataLength() > 21)
    {
      Uart0GetRxData(&pRxBuf[0], 22);
      i=0;
      while((pRxBuf[i] != 'O') && (i<22))
      {
        i += 1;
      }
      OSMemPut(pSmallMem, pMsg);
      OSMemPut(pSmallMem, pRxBuf);
      SwdFree(s8Dog);

      return TRUE;
    }

    if(SwdGetLifeTime(s8Dog) == 0)
    {
      SwdReset(s8Dog);

      //发送10包数据还没有接收到命令，直接返回
      ++u8Back;
      if(u8Back > 10)
      {
        OSMemPut(pSmallMem, pMsg);
        OSMemPut(pSmallMem, pRxBuf);
        SwdFree(s8Dog);

        return FALSE;
      }

      //清空串口缓冲区
      Uart0EmptyRxBuf();
      //发送模块型号命令
      CdmaSendMsg(pMsg, i);
     }
  }
}

void CdmaToBeiDou(void)
{
  g_bCdmaSend = FALSE;
  g_bBdSend		= TRUE;
  CdmaPowerOff();
//  BeiDouPowerOn();
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

  //IC卡检测
//  CdmaIcCheckSetIn();
  //CDMA RI中断，下降沿触发中断
 	CdmaRiIesEdge();
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
  g_u16TimerDelay=0;
  while(g_u16TimerDelay < 500);  //延时1s，时间太短会检测不到手机卡

  //检测一下是否插入IC卡
#if DTIC_DET_EN > 0
  if(!CdmaIcDet())
  {
    if(g_bDebug & CDMA_DEBUG)
    {
      Uart3SendString("#CDMA IC Card Not Found!\r\n ");
    }

    CdmaToBeiDou();
//    return;
  }
  else
#endif
  {
    if(GetCdmaCSQ() > 16)					//信号强度可以发送信息
    {
      g_bCdmaSend = TRUE;
      g_bBdSend   = FALSE;

      BeiDouPowerOff();
    }
    else
    {
      CdmaToBeiDou();
    }

    CdmaDtrSetLow();             //拉低DTR，将CDMA退出待机。
    CdmaDtrSetOut();
    CdmaLowPowerEnter();

    CdmaSendMsg(au8ZMSGL_ASCII, sizeof(au8ZMSGL_ASCII));

    //cdma模块接收命令解析初始化。
    CdmaCmdInit();
  }
}

/*****************************************************************************
 函 数 名  : SetLocationNbr
 功能描述  : 设置本机号码

 输入参数  : PTR
 返 回 值  : TRUE , FALSE
*****************************************************************************/
BOOL SetLocationNbr(UINT8 *ptr)
{
  UINT8 au8SetCNUM[24];
  UINT8 i,u8Back;
  SINT8 sDog;

  i = 0;
  memcpy(&au8SetCNUM[i], "AT+CNUM=", 8);
  i += 8;

  memcpy(&au8SetCNUM[i], ptr, 11);
  i += 11;

  au8SetCNUM[i++] = '\r';

  //清空串口缓冲区
  Uart0EmptyRxBuf();
  //发送模块型号命令
  CdmaSendMsg(au8SetCNUM, i);

  sDog = SwdGet();
  SwdSetLifeTime(sDog, 2000);
  SwdEnable(sDog);
  StopWatchDog();

  while(TRUE)
  {
    if(Uart0GetRxDataLength() > 1)
    {
      if(Uart0GetByte() == 'O')
      {
        if(Uart0GetByte() == 'K')
        {
          SwdFree(sDog);
          return TRUE;
        }
      }
    }

    if(SwdGetLifeTime(sDog) == 0)
    {
      SwdReset(sDog);

      //发送5包数据还没有接收到命令，直接返回
      ++u8Back;
      if(u8Back > 5)
      {
        SwdFree(sDog);
        return FALSE;
      }

      //清空串口缓冲区
      Uart0EmptyRxBuf();
      //发送模块型号命令
      CdmaSendMsg(au8SetCNUM, i);
     }
  }

}

/*****************************************************************************
 函 数 名  : GetLocationNbr
 功能描述  : 设置本机号码

 输入参数  : NONE
 返 回 值  : TRUE , FALSE
*****************************************************************************/
BOOL GetLocationNbr(UINT8 *ptr)
{
  UINT8 au8GetCNUM[16];
  UINT8 au8Buf[48];
  UINT8 i,u8Back;
  SINT8 sDog;

  memcpy(&au8GetCNUM[0], "AT+CNUM?\r", 10);

  //清空串口缓冲区
  Uart0EmptyRxBuf();
  //发送模块型号命令
  CdmaSendMsg(au8GetCNUM, 10);

  sDog = SwdGet();
  SwdSetLifeTime(sDog, 2000);
  SwdEnable(sDog);
  StopWatchDog();

  while(TRUE)
  {
    if(Uart0GetRxDataLength() > 34)
    {
      Uart0GetRxData(au8Buf, 34);

      i = 0;
      while((au8Buf[i] != ':') && (i<34))
      {
        i++;
      }

      i += 1;
      memcpy(ptr, &au8Buf[i], 11);

      SwdFree(sDog);
      return TRUE;
    }

    if(SwdGetLifeTime(sDog) == 0)
    {
      SwdReset(sDog);

      //发送5包数据还没有接收到命令，直接返回
      ++u8Back;
      if(u8Back > 5)
      {
        SwdFree(sDog);
        return FALSE;
      }

      //清空串口缓冲区
      Uart0EmptyRxBuf();
      //发送模块型号命令
      CdmaSendMsg(au8GetCNUM, i);
     }
  }
}

/*****************************************************************************
 函 数 名  : GetCdmaModule
 功能描述  : 获取CDMA模块型号

 输入参数  : none
 返 回 值  : none,
*****************************************************************************/
void GetCdmaModule(void)
{
  UINT8 i,u8Back;
  SINT8 sDog;
  UINT8 *pRxBuf;

  pRxBuf = OSMemGet(pSmallMem, &err);
//  memset(pRxBuf, 0, BUF_SMALL_SIZE);

  sDog = SwdGet();
  SwdSetLifeTime(sDog, 2000);
  SwdEnable(sDog);
  StopWatchDog();

  //清空串口缓冲区
  Uart0EmptyRxBuf();
  //发送模块型号命令
  CdmaSendMsg(au8GetModuleIndex, sizeof(au8GetModuleIndex));

  while(TRUE)
  {
    if(Uart0GetRxDataLength() > 19)
    {
      Uart0GetRxData(pRxBuf, 20);

      i = 0;
      while((pRxBuf[i] != '\r') && (i<20))
      {
        i++;
      }

      if(memcmp(&pRxBuf[i-6], "MC8332", 6) == 0)
      {
        Uart3SendString("#Detect ZTE CDMA Module. The Product Model Is MC8332!\r\n");
      }
      else
      {
        Uart3SendString("#CDMA Module To Find The Failure!\r\n");
      }

      break;
    }

    if(SwdGetLifeTime(sDog) == 0)
    {
      SwdReset(sDog);
      i = 0;

      //发送5包数据还没有接收到命令，直接返回
      ++u8Back;
      if(u8Back > 5)
        break;

      //清空串口缓冲区
      Uart0EmptyRxBuf();
      //发送模块型号命令
      CdmaSendMsg(au8GetModuleIndex, sizeof(au8GetModuleIndex));
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
  UINT8 i,u8Back;
  SINT8 sDog;
  UINT8 *pRxBuf;
  UINT8 u8Temp;

  pRxBuf = OSMemGet(pSmallMem, &err);
//  memset(pRxBuf, 0, BUF_SMALL_SIZE);

  sDog = SwdGet();
  SwdSetLifeTime(sDog, 2000);
  SwdEnable(sDog);
  //清空串口缓冲区
  Uart0EmptyRxBuf();
  //发送模块型号命令
  CdmaSendMsg(au8CSQ, sizeof(au8CSQ));
  StopWatchDog();

  while(TRUE)
  {
    if(Uart0GetRxDataLength() > 24)
    {
      Uart0GetRxData(pRxBuf, 24);

      i = 0;
      while((pRxBuf[i] != ':') && (i<24))
      {
        i++;
      }

      if((pRxBuf[i+3] >= 0x30) && (pRxBuf[i+2] >= 0x30))
      {
        u8Temp = (pRxBuf[i+2]-0x30)*10 + (pRxBuf[i+3]-0x30);
        break;
      }
     }

    if(SwdGetLifeTime(sDog) == 0)
    {
      SwdReset(sDog);

      //发送10包数据还没有接收到命令，直接返回
      ++u8Back;
      if(u8Back > 5)
      {
        CdmaToBeiDou();
        u8Temp = 0xFF;
        break;
      }

      //清空串口缓冲区
      Uart0EmptyRxBuf();
      //发送模块型号命令
      CdmaSendMsg(au8CSQ, sizeof(au8CSQ));
    }
  }

  //释放内存
  OSMemPut(pSmallMem, pRxBuf);
  //释放软件定时器
  SwdFree(sDog);

  return u8Temp;
}

/*****************************************************************************
 函 数 名  : GetCdmaTime
 功能描述  : 获取CDMA时间, 设置到RTC

 输入参数  : *pTime，*pu8Len
 返 回 值  : none
*****************************************************************************/
void GetCdmaTime(UINT8 *pTime, UINT8 *pu8Len)
{
  UINT8 i,j,u8Back;
  SINT8 sDog;
  UINT8 *pRxBuf;

  pRxBuf = OSMemGet(pSmallMem, &err);
//  memset(pRxBuf, 0, BUF_SMALL_SIZE);

  sDog = SwdGet();
  SwdSetLifeTime(sDog, 2000);
  SwdEnable(sDog);
  //清空串口缓冲区
  Uart0EmptyRxBuf();
  //发送模块型号命令
  CdmaSendMsg(au8CCLK, sizeof(au8CCLK));

  while(TRUE)
  {
    if(Uart0GetRxDataLength() > 48)
    {
      Uart0GetRxData(pRxBuf, 48);

      i = 0;
      while((pRxBuf[i] != '"') && (i<48))
      {
        i++;
      }

      j = ++i;
      while((pRxBuf[j] != '"') && (j<48))
      {
        j++;
      }

      break;
    }

    if(SwdGetLifeTime(sDog) == 0)
    {
      SwdReset(sDog);

      //发送10包数据还没有接收到命令，直接返回
      ++u8Back;
      if(u8Back > 5)
        break;

      //清空串口缓冲区
      Uart0EmptyRxBuf();
      //发送模块型号命令
      CdmaSendMsg(au8CCLK, sizeof(au8CCLK));
    }
  }
  //时间拷贝，忽略ms数据
  *pu8Len = j-i;
  memcpy(pTime, &pRxBuf[i], j-i);
  //释放内存
  OSMemPut(pSmallMem, pRxBuf);
  //释放软件定时器
  SwdFree(sDog);
}


/*****************************************************************************
 函 数 名  : SetCdmaTimeToRtc
 功能描述  : 获取CDMA时间, 设置到RTC

 输入参数  : none
 返 回 值  : TRUE -- 校准成功； FALSE -- 校准失败
*****************************************************************************/
BOOL SetCdmaTimeToRtc(void)
{
  UINT8 au8Time[32];
  UINT8 u8Len;
  UINT8 i;
  _tsTimeCb sTime;

  GetCdmaTime(&au8Time[0], &u8Len);

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

  RtcConverHour(1);
//  SetRtcInt1Out();
  SetTimeToRtc(&sTime);

  //打开RTC外部中断
  SetRtcInt1Out();

  return TRUE;
}

/*****************************************************************************
 函 数 名  : CdmaLowPowerEnter
 功能描述  : CDMA模块进入低功耗模块

 输入参数  : PTR
 返 回 值  : TRUE , FALSE
*****************************************************************************/
void CdmaLowPowerEnter(void)
{
  UINT8 au8RxBuf[24];
  UINT8 i,u8Back=0;
  SINT8 sDog;

  memset(au8RxBuf, 0, 24);

  //设置CDMA 32k深度睡眠
  CdmaSendMsg(&au8EnableZDSLEEP[0], sizeof(au8EnableZDSLEEP));
  g_u8TimerDelay=0;
  while(g_u8TimerDelay < 10);  //延时100ms，等待回复命令

  sDog = SwdGet();
  SwdSetLifeTime(sDog, 2000);
  SwdEnable(sDog);

  while(TRUE)
  {
    if(Uart0GetRxDataLength() > 17)
    {
      Uart0GetRxData(au8RxBuf, 18);

      i = 0;
      while((au8RxBuf[i] != 'O') && (i<18))
      {
        i++;
      }

      break;
    }

    if(SwdGetLifeTime(sDog) == 0)
    {
      SwdReset(sDog);

      //发送10包数据还没有接收到命令，直接返回
      ++u8Back;
      if(u8Back > 5)
      {
        CdmaToBeiDou();
        //释放软件定时器
        SwdFree(sDog);
        return;
      }

      //发送模块型号命令
      CdmaSendMsg(&au8EnableZDSLEEP[0], sizeof(au8EnableZDSLEEP));
    }
  }

  memset(au8RxBuf, 0, 24);

  //允许DTR管脚控制进入待机
  CdmaSendMsg(&au8EnableZDSLEEPDTR[0], sizeof(au8EnableZDSLEEPDTR));
  while(g_u8TimerDelay < 10);  //延时100ms，等待回复命令

   while(TRUE)
  {
    if(Uart0GetRxDataLength() > 19)
    {
      Uart0GetRxData(au8RxBuf, 20);

      i = 0;
      while((au8RxBuf[i] != 'O') && (i<20))
      {
        i++;
      }

      break;
    }

    if(SwdGetLifeTime(sDog) == 0)
    {
      SwdReset(sDog);

      //发送10包数据还没有接收到命令，直接返回
      ++u8Back;
      if(u8Back > 5)
      {
        CdmaToBeiDou();
        //释放软件定时器
        SwdFree(sDog);
        return;
      }

      //发送模块型号命令
      CdmaSendMsg(&au8EnableZDSLEEPDTR[0], sizeof(au8EnableZDSLEEPDTR));
    }
  }
  SwdFree(sDog);
}

/*****************************************************************************
 函 数 名  : CdmaLowPowerExit
 功能描述  : CDMA模块退出低功耗模块

 输入参数  : PTR
 返 回 值  : TRUE , FALSE
*****************************************************************************/
void CdmaLowPowerExit(void)
{
  UINT8 au8RxBuf[24];
  UINT8 i,u8Back=0;
  SINT8 sDog;

  memset(au8RxBuf, 0, 24);

  //禁止CDMA 32k深度睡眠
  CdmaSendMsg(&au8DisableZDSLEEP[0], sizeof(au8DisableZDSLEEP));
  g_u8TimerDelay=0;
  while(g_u8TimerDelay < 10);  //延时100ms，等待回复命令

  sDog = SwdGet();
  SwdSetLifeTime(sDog, 2000);
  SwdEnable(sDog);

  while(TRUE)
  {
    if(Uart0GetRxDataLength() > 17)
    {
      Uart0GetRxData(au8RxBuf, 18);

      i = 0;
      while((au8RxBuf[i] != 'O') && (i<18))
      {
        i++;
      }

      break;
    }

    if(SwdGetLifeTime(sDog) == 0)
    {
      SwdReset(sDog);

      //发送10包数据还没有接收到命令，直接返回
      ++u8Back;
      if(u8Back > 5)
      {
        CdmaToBeiDou();
        //释放软件定时器
        SwdFree(sDog);
        return;
      }

      //发送模块型号命令
      CdmaSendMsg(&au8DisableZDSLEEP[0], sizeof(au8DisableZDSLEEP));
    }
  }

  memset(au8RxBuf, 0, 24);

  //禁止DTR管脚控制进入待机
  CdmaSendMsg(&au8DisableZDSLEEPDTR[0], sizeof(au8DisableZDSLEEPDTR));
  while(g_u8TimerDelay < 10);  //延时100ms，等待回复命令

   while(TRUE)
  {
    if(Uart0GetRxDataLength() > 19)
    {
      Uart0GetRxData(au8RxBuf, 20);

      i = 0;
      while((au8RxBuf[i] != 'O') && (i<20))
      {
        i++;
      }

      break;
    }

    if(SwdGetLifeTime(sDog) == 0)
    {
      SwdReset(sDog);

      //发送10包数据还没有接收到命令，直接返回
      ++u8Back;
      if(u8Back > 5)
      {
        CdmaToBeiDou();
        //释放软件定时器
        SwdFree(sDog);
        return;
      }

      //发送模块型号命令
      CdmaSendMsg(&au8DisableZDSLEEPDTR[0], sizeof(au8DisableZDSLEEPDTR));
    }
  }

  SwdFree(sDog);
}

/*****************************************************************************
 函 数 名  : CdmaWakeUpCfg
 功能描述  : CDMA模块退出睡眠模式，并读取CDMA信号，信号强度低于16时自动切换到
             北斗通讯模块上

 输入参数  : PTR
 返 回 值  : TRUE , FALSE
*****************************************************************************/
void CdmaWakeUpCfg(void)
{
  CdmaDtrSetLow();     //拉低DTR，将CDMA唤醒。();

  g_u16TimerDelay=0;
  while(g_u16TimerDelay < 200);  //延时一下，等待模块稳定

  if(GetCdmaCSQ() > 16)					//信号强度可以发送信息
  {
    g_bCdmaSend = TRUE;
    g_bBdSend   = FALSE;

     BeiDouPowerOff();
  }
  else
  {
    CdmaToBeiDou();
  }
}

/*****************************************************************************
 函 数 名  : CdmaCmdInit
 功能描述  : CDMA模块命令接收初始化

 输入参数  : none
 返 回 值  : none,
*****************************************************************************/
void CdmaCmdInit(void)
{
  sCdmaCmdCb.u16BufDataLen  = 0;
  sCdmaCmdCb.eAssembleState = E_CDMA_RECEIVING_HEADER;
  sCdmaCmdCb.pu8AssembleBuf = OSMemGet(pSmallMem, &err);

  //Get One soft timer
  sCdmaCmdCb.sCdmaDog       = SwdGet();
  SwdSetName(sCdmaCmdCb.sCdmaDog, "CdmaTimer");
  SwdSetLifeTime(sCdmaCmdCb.sCdmaDog, 2000);
}

/*****************************************************************************
 函 数 名  : CdmaCmdAssemble
 功能描述  : CDMA接收命令解析

 输入参数  : 无
 返 回 值  : 无 , 无
*****************************************************************************/
void CdmaCmdAssemble(UINT8 u8InData)
{
    switch(u8InData)
    {
        case E_CDMA_RECEIVING_HEADER:
        {
            if(u8InData == '+')
            {
               sCdmaCmdCb.pu8AssembleBuf[0] = u8InData;
               sCdmaCmdCb.u8Cmd             = 0x00;
               sCdmaCmdCb.u16BufDataLen     = 1;
               sCdmaCmdCb.eAssembleState    = E_CDMA_RECEIVING_CMD;
            }

            break;
        }

        case E_CDMA_RECEIVING_CMD:
        {
            if((u8InData >= 'A') || (u8InData <= 'Z'))
            {
                sCdmaCmdCb.u8Cmd += u8InData;
                sCdmaCmdCb.pu8AssembleBuf[sCdmaCmdCb.u16BufDataLen++] = u8InData;
            }
            else
            {
                if(u8InData == ':')
                {
                    sCdmaCmdCb.pu8AssembleBuf[sCdmaCmdCb.u16BufDataLen++] = u8InData;
                    sCdmaCmdCb.eAssembleState = E_CDMA_RECEIVING_BODY;
                }
                else
                {
                    sCdmaCmdCb.eAssembleState = E_CDMA_RECEIVING_HEADER;
                    sCdmaCmdCb.u16BufDataLen  = 0;
                }
            }

            break;
        }

        case E_CDMA_RECEIVING_BODY:
        {
            if(u8InData != '+')
            {
                //将数据放入组包缓冲区
                sCdmaCmdCb.pu8AssembleBuf[sCdmaCmdCb.u16BufDataLen++] = u8InData;

                if(u8InData == 'S')
                {
                    sCdmaCmdCb.eAssembleState = E_CDMA_RECEIVING_SMS;
                    sCdmaCmdCb.u8SmsLen       = 0;
                    break;
                }
            }
            else
            {
                if(u8InData == '+')
                {
                    Uart0InsertByte('+');
                    sCdmaCmdCb.eAssembleState = E_CDMA_RECEIVING_VALID;
                }
            }
            break;
        }

        case E_CDMA_RECEIVING_SMS:
        {
            sCdmaCmdCb.pu8AssembleBuf[sCdmaCmdCb.u16BufDataLen++] = u8InData;
            sCdmaCmdCb.u8SmsLen++;

            if(sCdmaCmdCb.u8SmsLen == 9)
            {
                if(0 == memcmp(&sCdmaCmdCb.pu8AssembleBuf[sCdmaCmdCb.u16BufDataLen-9], "SMS REDAY", 9))
                {
                    sCdmaCmdCb.eAssembleState = E_CDMA_RECEIVING_VALID;
                }
                else
                {
                    sCdmaCmdCb.eAssembleState = E_CDMA_RECEIVING_BODY;
                }
            }

            break;
        }

        case E_CDMA_RECEIVING_VALID:
        {
            break;
        }

        default:
        {
            sCdmaCmdCb.eAssembleState = E_CDMA_RECEIVING_HEADER;
            sCdmaCmdCb.u16BufDataLen  = 0;
            break;
        }
    }
}

/*****************************************************************************
 函 数 名  : CdmaCmdRun
 功能描述  : CDMA数据解析

 输入参数  : 无
 返 回 值  : 无 , 无
*****************************************************************************/
void CdmaCmdRun(void)
{
  UINT8 u8Cmd = 0;
  
  if(g_bCdmaSend == TRUE)
  {
    //从串口缓冲区内获取数据，对其进行解析
    //主要针对短信接收命令过程和CSQ信号强度的接收。
    while((sCdmaCmdCb.eAssembleState != E_CDMA_RECEIVING_VALID) && (Uart0GetRxDataLength() > 0))
    {
        CdmaCmdAssemble(Uart0GetByte());

        if(SwdIsEnable(sCdmaCmdCb.sCdmaDog) == TRUE)
        {
            SwdDisable(sCdmaCmdCb.sCdmaDog);
        }
    }

    if(sCdmaCmdCb.eAssembleState == E_CDMA_RECEIVING_VALID)
    {
        u8Cmd = sCdmaCmdCb.u8Cmd;
        // 1、信号强度
        // 2、短信接收
        // 3、短信读取和解析命令，接收到启动命令后使能启动标志位。
        
        memset(sCdmaCmdCb.pu8AssembleBuf, 0, sCdmaCmdCb.u16BufDataLen);
        sCdmaCmdCb.eAssembleState = E_CDMA_RECEIVING_HEADER;
        sCdmaCmdCb.u16BufDataLen = 0;
    }
    else
    {
       if(SwdIsEnable(sCdmaCmdCb.sCdmaDog) == TRUE)
       {
           if(SwdGetLifeTime(sCdmaCmdCb.sCdmaDog) == 0)
           {
                sCdmaCmdCb.eAssembleState = E_CDMA_RECEIVING_HEADER;
                sCdmaCmdCb.u16BufDataLen  = 0;

                SwdReset(sCdmaCmdCb.sCdmaDog);
                SwdDisable(sCdmaCmdCb.sCdmaDog);
           }
       }
       else
       {
           if(sCdmaCmdCb.u16BufDataLen != 0)
           {
                SwdReset(sCdmaCmdCb.sCdmaDog);
                SwdEnable(sCdmaCmdCb.sCdmaDog);
           }
       }
    }

  }
}

