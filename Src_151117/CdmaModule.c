
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
#include "BeiDouModule.h"

//前面的强制类型转化缺省会造成计算错误
#define _GetWordFromAddr(pAddr)         (UINT16)(((UINT16)pAddr[0]) << 8) + pAddr[1]

#define _GetDoubleWordFromAddr(pAddr)   ((UINT32)(((UINT32)pAddr[0]) << 24) +    \
                                         (UINT32)(((UINT32)pAddr[1]) << 16) +    \
                                         (UINT32)(((UINT32)pAddr[2]) << 8) + pAddr[3])

//命令字母相加，取低8位作为指令代码
#define CMD_ZCANS         0x7F
#define CMD_ZCCNT         0x82
#define CMD_CMTI          0x2D
#define CMD_ZCEND         0x74
#define CMD_CSQ           0xE7
#define CMD_SMS           0x68
#define CMD_CMGR          0x29
#define CMD_NONE          0x00

#define CMD_ASSERT(a)    ((a == CMD_CMTI ) || \
                          (a == CMD_CMGR ) || \
                          (a == CMD_CSQ  ))


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

  UINT8 u8MsgEnvent;      //收到短信后，模块会向主机发送一系列指令，
                          //4条指令分别对应低4位

  teCdmaAssembleState eAssembleState;  //组包状态

  SINT8 sCdmaDog;         //cdma接收命令超时定时器

  SINT8 sCsqDog;          //CSQ信号强度获取时间定时器

}_tsCdmaCmdCb;

_tsCdmaCmdCb sCdmaCmdCb;

//当有未读短信时，在相应的位置标识为1，否则为0
UINT8 au8MsgUnRead[MSG_MAX_SIZE];
BOOL  bMsgReady = FALSE;

//定义当前接收到的短信编号
SINT8 s8CurrentIndex = -1;

//进入DTU配置模式
const UINT8 u8ConfigMode[]         = {"enter_config_mode\0"};
const UINT8 u8ConfigStart[]        = {0xff,0xff,0xff,0xff};
const UINT8 u8ConfigRxData[]       = {"aaaaaaaaaaaaaaaa\0"};

const UINT8 au8BackOk[3]           = {0x4f, 0x4b, 0x00};

//查询模块型号
const UINT8 au8GetModuleIndex[]    = {"AT+CGMM\r"};
//串口波特率设置
const UINT8 au8UartSet[]           = {"AT+UART=9600,ON,8,N,1\r"};
//设置32khz深度睡眠模式
const UINT8 au8EnableZDSLEEP[]     = {"AT+ZDSLEEP=1\r"};
const UINT8 au8DisableZDSLEEP[]    = {"AT+ZDSLEEP=0\r"};
//DTR引脚休眠控制
const UINT8 au8EnableZDSLEEPDTR[]  = {"AT+ZDSLEEPDTR=1\r"};
const UINT8 au8DisableZDSLEEPDTR[] = {"AT+ZDSLEEPDTR=0\r"};
//获取CDMA时间
const UINT8 au8GetCdmaTime[]       = {"AT+ZDSLEEPDTR=2\r"};
//复位模块
const UINT8 au8DisableZRST[]       = {"AT+ZRST=0\r"};
//允许复位,1分钟后复位
//const UINT8 au8EnableZRST[] = {"AT+ZRST=1,"00:01"\r"};

//信号强度查询，
//返回：+CSQ: 28,99 OK，16以下可能呼叫失败。
const UINT8 au8CSQ[]               = {"AT+CSQ\r"};

//时钟查询
//返回：+CCLK:"2004/02/09，17：34：23.694"当前网络时间。
const UINT8 au8CCLK[]              = {"AT+CCLK?\r"};

//设置短消息模式
const UINT8 au8CMGF[]              = {"AT+CMGF=1\r"};
//读取短消息
const UINT8 au8CMGR[]              = {"AT+CMGR="};

//设置本机号码
//const UINT8 au8SetCNUM[] = {"AT+CNUM="};
//const UINT8 au8GetCNUM[] = {"AT+CNUM?"};

//模块指示
const UINT8 au8ZIND[] = {"AT+ZIND="};

/////////////////////////////////////////////////////////////
// 短信服务
//选择短信服务
const UINT8 au8MsgType[]            = {"AT+CSMS?\r"};
//设置短信为英文， ascii 格式
const UINT8 au8ZMSGL_ASCII[]        = {"AT+ZMSGL=1,2\r"};
//设置短信为汉语 ， unicode格式
const UINT8 au8ZMSCL_UNICODE[]      = {"AT+ZMSGL=6,4\r"};

//短信删除
const UINT8 au8CMGD[] = {"AT+CMGD="};
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
//  Uart0EmptyRxBuf();
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
//  UINT8 *pRxBuf;
  UINT8 i=0;
//  SINT8 s8Dog;
//  UINT8 u8Back;

  pMsg = OSMemGet(pSmallMem, &err);
//  pRxBuf = OSMemGet(pSmallMem, &err);

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

  CdmaSendMsg(pMsg, i);
  g_u8TimerDelay=0;
  while(g_u8TimerDelay < 10);    //延时20ms

  OSMemPut(pSmallMem, pMsg);

  return TRUE;
 /*
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
//      Uart0EmptyRxBuf();
      //发送模块型号命令
      CdmaSendMsg(pMsg, i);
     }
  }*/
}

void CdmaToBeiDou(void)
{
  g_bCdmaSend = FALSE;
  g_bBdSend		= TRUE;
  CdmaPowerOff();

  BeiDouPowerOn();
  BdSendCmdICJC();
}

void BeiDouToCdma(void)
{
  g_bCdmaSend = TRUE;
  g_bBdSend	  = FALSE;

  CdmaPowerOn();
  StopWatchDog();           // 关闭看门狗
  CdmaInit();
  FreeWatchDog();          //释放看门狗

  BeiDouPowerOff();
}


/*****************************************************************************
 函 数 名  : DeletCdmaMsg()
 功能描述  : 短信删除功能

 输入参数  : u8Index -- 所要删除的短信编号, 最多存储30条短信
                        0xff -- 删除全部的短消息

 返 回 值  : TRUE, FALSE
*****************************************************************************/
BOOL DeletCdmaMsg(SINT8 s8Index)
{
    UINT8   au8Buf[24];
    UINT8   i= 0;
    UINT8 * pMsg;
//    SINT8   s8Dog;
//    UINT8   u8Back = 0;
//   UINT8   au8RxBuf[16];

    if((s8Index > MSG_MAX_SIZE) || (s8Index < 0))
    {
        return FALSE;
    }

    memset(&au8Buf[0], 0, 24);
    pMsg = &au8Buf[0];

    memcpy(&pMsg[i], &au8CMGD[0], 8);
    i += 8;

    if(s8Index == MSG_MAX_SIZE)
    {
        pMsg[i++] = 0x31;
        pMsg[i++] = ',';
        pMsg[i++] = 0x34;
    }
    else
    {
        if(s8Index < 9)
        {
            pMsg[i++] = s8Index + '0';
        }
        else
        {
            pMsg[i++] = s8Index / 10 + '0';
            pMsg[i++] = s8Index % 10 + '0';
        }
    }
    pMsg[i++] = '\r';

    //发送模块删除命令
    CdmaSendMsg(pMsg, i);
    
    return TRUE;

    //当同时接收大量信息时，等待时间过长，则直接忽略掉反馈信息。
/*
    StopWatchDog();
    s8Dog = SwdGet();
    SwdSetLifeTime(s8Dog, 5000);
    SwdEnable(s8Dog);
    memset(&au8RxBuf[0], 0, 16);

    while(TRUE)
    {
        if(Uart0GetRxDataLength() > 2)
        {
          Uart0GetRxData(&au8RxBuf[0], 2);

          if(0 == memcmp(&au8RxBuf[0], "OK", 2))
          {
            return TRUE;
          }
        }

        if(SwdGetLifeTime(s8Dog) == 0)
        {
          SwdReset(s8Dog);

          //发送5包数据还没有接收到命令，直接返回
          ++u8Back;
          if(u8Back > 2)
          {
            SwdFree(s8Dog);

            return FALSE;
          }

           //发送模块删除命令
           CdmaSendMsg(pMsg, i);
        }
    }*/
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
  while(g_u16TimerDelay < 100);  //延时2s
  SetCdmaOnOff(1);
  g_u16TimerDelay=0;
  while(g_u16TimerDelay < 100);  //延时5s，时间太短会检测不到手机卡

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
    /*
    if(GetCdmaCSQ() > 20)					//信号强度可以发送信息
    {
      g_bCdmaSend = TRUE;
      g_bBdSend   = FALSE;

      BeiDouPowerOff();

      CdmaSendMsg(au8ZMSGL_ASCII, sizeof(au8ZMSGL_ASCII));

      CdmaDtrSetLow();             //拉低DTR，将CDMA退出待机。
      CdmaDtrSetOut();
      CdmaLowPowerEnter();

      CdmaIndicatorEnable();
    }
    else
    {
      CdmaToBeiDou();
    }   */

    g_bCdmaSend = TRUE;
    g_bBdSend   = FALSE;

    CdmaSendMsg(au8ZMSGL_ASCII, sizeof(au8ZMSGL_ASCII));
    CdmaDtrSetLow();             //拉低DTR，将CDMA退出待机。
    CdmaDtrSetOut();
    CdmaLowPowerEnter();
 //   CdmaIndicatorEnable();

    CdmaSendMsg(&au8CMGF[0], sizeof(au8CMGF));

    //cdma模块接收命令解析初始化。
    CdmaCmdInit();

    //清除接收缓冲区内容
    Uart0EmptyRxBuf();
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
//  Uart0EmptyRxBuf();
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
  sCdmaCmdCb.u8MsgEnvent    = 0;
  sCdmaCmdCb.eAssembleState = E_CDMA_RECEIVING_HEADER;
  sCdmaCmdCb.pu8AssembleBuf = OSMemGet(pSmallMem, &err);

  //Get One soft timer
  sCdmaCmdCb.sCdmaDog       = SwdGet();
  SwdSetName(sCdmaCmdCb.sCdmaDog, "CdmaTimer");
  SwdSetLifeTime(sCdmaCmdCb.sCdmaDog, 500);

  sCdmaCmdCb.sCsqDog       = SwdGet();
  SwdSetName(sCdmaCmdCb.sCsqDog, "CSQTimer");
  SwdSetLifeTime(sCdmaCmdCb.sCsqDog, 40000);     //每45秒获取一次信号强度
  SwdEnable(sCdmaCmdCb.sCsqDog);

  memset(&au8MsgUnRead[0], 0, MSG_MAX_SIZE);
  bMsgReady = FALSE;
}

/*****************************************************************************
 函 数 名  : CdmaCmdAssemble
 功能描述  : CDMA接收命令解析

 输入参数  : 无
 返 回 值  : 无 , 无
*****************************************************************************/
void CdmaCmdAssemble(UINT8 u8InData)
{
    switch(sCdmaCmdCb.eAssembleState)
    {
        case E_CDMA_RECEIVING_HEADER:
        {
            if(u8InData == '+')
            {
               sCdmaCmdCb.pu8AssembleBuf[0] = u8InData;
               sCdmaCmdCb.u8Cmd             = 0;
               sCdmaCmdCb.u16BufDataLen     = 1;
               sCdmaCmdCb.eAssembleState    = E_CDMA_RECEIVING_CMD;
            }
/*            else
            {
                if((u8InData == 'S')  && (sCdmaCmdCb.u8MsgEnvent & 0x0C == 0x0C))
                {
                    sCdmaCmdCb.pu8AssembleBuf[0] = u8InData;
                    sCdmaCmdCb.u8Cmd             = u8InData;
                    sCdmaCmdCb.u16BufDataLen     = 1;
                    sCdmaCmdCb.eAssembleState    = E_CDMA_RECEIVING_SMS;
                }
            }*/

            break;
        }

        case E_CDMA_RECEIVING_CMD:
        {
            if((u8InData >= 'A') && (u8InData <= 'Z'))
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

                if((u8InData == 'K') && (sCdmaCmdCb.pu8AssembleBuf[sCdmaCmdCb.u16BufDataLen-2] == 'O'))
                {
                    sCdmaCmdCb.eAssembleState = E_CDMA_RECEIVING_VALID;
                }
                else
                {
                  if((sCdmaCmdCb.u8Cmd == CMD_ZCEND) && (u8InData == 0x0d))
                  {
                      sCdmaCmdCb.eAssembleState = E_CDMA_RECEIVING_VALID;
                  }
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
/*
        case E_CDMA_RECEIVING_SMS:
        {
            sCdmaCmdCb.pu8AssembleBuf[sCdmaCmdCb.u16BufDataLen++] = u8InData;

            if((u8InData >= 'A') && (u8InData <= 'Z'))
            {
                sCdmaCmdCb.u8Cmd += u8InData;
            }

            if(u8InData == 0x0d)
            {
                sCdmaCmdCb.eAssembleState = E_CDMA_RECEIVING_VALID;
            }

            break;
        }*/

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
 函 数 名  : RegisteUnReadMsg
 功能描述  : 标识未读短信到注册缓冲区

 输入参数  : 无
 返 回 值  : BOOL
*****************************************************************************/
BOOL RegisteUnReadMsg(void)
{
    UINT8 *pRxBuf;
    UINT8  i = 0;
    UINT8  u8Temp = 0;

    //获取可用的内存单元
    pRxBuf = OSMemGet(pSmallMem, &err);
    memcpy(pRxBuf, sCdmaCmdCb.pu8AssembleBuf, sCdmaCmdCb.u16BufDataLen);

    while((pRxBuf[i] != '"') && (i < sCdmaCmdCb.u16BufDataLen))
    {
        i++;
    }

    if((pRxBuf[i+1] == 'M') && (pRxBuf[i+2] == 'T'))
    {
        if((pRxBuf[i+6] >= '0') && (pRxBuf[i+6] <= '9'))
        {
            if((pRxBuf[i+5] >= '0') && (pRxBuf[i+5] <= '9'))
            {
                u8Temp  = (pRxBuf[i+5] - '0') * 10;
                u8Temp += (pRxBuf[i+6] - '0');
            }
            else
            {
                u8Temp = 0xff;
            }
        }
        else
        {
            if((pRxBuf[i+5] >= '0') && (pRxBuf[i+5] <= '9'))
            {
                u8Temp  = (pRxBuf[i+5] - '0');
            }
            else
            {
                u8Temp = 0xff;
            }
        }
    }
    else
    {
        u8Temp = 0xff;
    }

    //释放内存
    OSMemPut(pSmallMem, pRxBuf);

    if(u8Temp < MSG_MAX_SIZE)
    {
        au8MsgUnRead[u8Temp] = 1;
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

/******************************************************************************
 * 函数名称：GetUnReadMsg
 * 描述：    得到一条未读的短消息编号
 *
 * 参数：无
 * 返回：未读短信编号
 *****************************************************************************/
SINT8 GetUnReadMsg(void)
{
  SINT8 i;

  for(i=0; i<MSG_MAX_SIZE; i++)
  {
    if(au8MsgUnRead[i] == 1)
    {
      au8MsgUnRead[i] = 0;
      return i;
    }
  }

  return -1;
}

/*****************************************************************************
 函 数 名  : GetCdmaCsqValue
 功能描述  : 从组包缓冲区中获取CDMA信号强度值

 输入参数  : none
 返 回 值  : 信号强度,0~31
*****************************************************************************/
SINT8 GetCdmaCsqValue(void)
{
    SINT8 s8Value=-1;   //CSQ值
    UINT8 au8Buf[24];
    UINT8 i=0;

    //获取缓冲区中的命令
    memcpy(&au8Buf[0], sCdmaCmdCb.pu8AssembleBuf, sCdmaCmdCb.u16BufDataLen);

    while((au8Buf[i] != ':') && (i < sCdmaCmdCb.u16BufDataLen))
    {
        i++;
    }

    if((au8Buf[i+1] == ' ') && (au8Buf[i+2] >= '0') && (au8Buf[i+2] <= '9'))
    {
        i += 2;
        s8Value  = (au8Buf[i] - '0') * 10;
        s8Value += (au8Buf[i+1] - '0');

        if(s8Value > 31)
        {
            return -1;
        }
        else
        {
            return s8Value;
        }
    }
    else
    {
        return -1;
    }

}

/*****************************************************************************
 函 数 名  : SendReadMsgCmd
 功能描述  : 向CDMA模块发送读取短信命令

 输入参数  : 无
 返 回 值  : BOOL
*****************************************************************************/
SINT8 SendReadMsgCmd(void)
{
    UINT8 u8Buf[24];
    UINT8 i=0;
    SINT8 s8MsgIndex = -1;

    s8MsgIndex = GetUnReadMsg();
    if(s8MsgIndex < 0)
    {
        return -1;
    }

    s8CurrentIndex = s8MsgIndex;
    memset(&u8Buf[0], 0, 24);
    memcpy(&u8Buf[0], &au8CMGR[0], sizeof(au8CMGR));
    i = sizeof(au8CMGD)-1;

    if(s8MsgIndex > 9)
    {
        u8Buf[i]  = s8MsgIndex / 10;
        u8Buf[i] += '0';
        i++;
        u8Buf[i]  = s8MsgIndex % 10;
        u8Buf[i] += '0';
        i++;
        u8Buf[i++] = '\r';

        CdmaSendMsg(&u8Buf[0], i);
    }
    else
    {
        u8Buf[i]  = s8MsgIndex + '0';
        i++;
        u8Buf[i++] = '\r';

        CdmaSendMsg(&u8Buf[0], i);
    }

    return s8MsgIndex;
}

/*****************************************************************************
 函 数 名  : CdmaCmdHandler
 功能描述  : CDMA接收命令解析

 输入参数  : 无
 返 回 值  : 无 , 无
*****************************************************************************/
void CdmaCmdHandler(UINT8 u8Cmd)
{
  SINT8 Csq = -1;

    switch(u8Cmd)
    {
        case CMD_CSQ:
        {
            if(g_bDebug & CDMA_DEBUG)    //打印调试信息
            {
                Uart2SendString(" CDMA CSQ Parse Successful!\r\n");
            }

            Csq = GetCdmaCsqValue();
            //信号强度小于20，则切换到北斗模式
            //再次检测则需在下次系统唤醒后。
            if(Csq < 20)
            {
                CdmaToBeiDou();
            }

            break;
        }

        case CMD_ZCANS:
        {
          // 关闭信号强度获取定时器，防止在接收信息时发送信号获取导致获取失败
            SwdDisable(sCdmaCmdCb.sCsqDog);
            sCdmaCmdCb.u8MsgEnvent |= BIT0;
            if(g_bDebug & CDMA_DEBUG)    //打印调试信息
            {
                Uart2SendString(" CDMA ZCANS Parse Successful!\r\n");
            }
            break;
        }

        case CMD_ZCCNT:
        {
            SwdDisable(sCdmaCmdCb.sCsqDog);
            sCdmaCmdCb.u8MsgEnvent |= BIT1;
            if(g_bDebug & CDMA_DEBUG)    //打印调试信息
            {
                Uart2SendString(" CDMA ZCCNT Parse Successful!\r\n");
            }
            break;
        }

        case CMD_CMTI:
        {
            SwdDisable(sCdmaCmdCb.sCsqDog);
            RegisteUnReadMsg();
            if(g_bDebug & CDMA_DEBUG)    //打印调试信息
            {
                Uart2SendString(" CDMA CMTI Parse Successful!\r\n");
            }
            break;
        }

        case CMD_ZCEND:
        {
            SwdDisable(sCdmaCmdCb.sCsqDog);
            sCdmaCmdCb.u8MsgEnvent |= BIT3;
            if(g_bDebug & CDMA_DEBUG)    //打印调试信息
            {
                Uart2SendString(" CDMA ZCEND Parse Successful!\r\n");
            }
            break;
        }

        case CMD_SMS:
        {
            SwdDisable(sCdmaCmdCb.sCsqDog);
            sCdmaCmdCb.u8MsgEnvent = 0;
            if(g_bDebug & CDMA_DEBUG)    //打印调试信息
            {
                Uart2SendString(" CDMA SMS READY!\r\n");
            }
            break;
        }

      case CMD_CMGR:
      {
          SwdEnable(sCdmaCmdCb.sCsqDog);

          //解析一条信息删除一条。
          DeletCdmaMsg(s8CurrentIndex);
          s8CurrentIndex = -1;
          if(g_bDebug & CDMA_DEBUG)    //打印调试信息
          {
            Uart2SendString(" CDMA CMGR Parse Successful!\r\n");
          }
          break;
      }

        default: break;
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

    //获得有效的命令包，则进行命令解析
    if(sCdmaCmdCb.eAssembleState == E_CDMA_RECEIVING_VALID)
    {
        if(CMD_ASSERT(sCdmaCmdCb.u8Cmd))
        {
            CdmaCmdHandler(sCdmaCmdCb.u8Cmd);

            if(g_bDebug & CDMA_DEBUG)    //打印调试信息
            {
               Uart2SendBuffer(sCdmaCmdCb.pu8AssembleBuf, sCdmaCmdCb.u16BufDataLen);
            }
         }

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

    if(SwdGetLifeTime(sCdmaCmdCb.sCsqDog) == 0)
    {
        SwdReset(sCdmaCmdCb.sCsqDog);

        CdmaSendMsg(au8CSQ, sizeof(au8CSQ));
    }

    SendReadMsgCmd();
  }
}

