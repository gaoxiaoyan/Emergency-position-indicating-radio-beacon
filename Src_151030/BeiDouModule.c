
/******************************************************************************************************
 *
 * Copyright (C) 2015, Sunic-ocean
 * All Rights Reserved
 *
 * 文 件 名： BeiDouModule.c
 * 描    述：北斗模块基本操作。初始化，IC检测，信息发送,命令解析等。
 *
 * 创建日期： 2015年5月5日16:10:43
 * 作    者： Bob
 * 当前版本： V1.00
 *******************************************************************************************************/
#define  BD_GLOBAL
#include "msp430x54x.h"
#include "BatteryDetect.h"
#include "global.h"
#include "DataType.h"
#include "SystemCtl.h"
#include "Timer.h"
#include "Uart2.h"
#include "Uart3.h"
#include "SoftTimer.h"
#include "OS_MEM.h"
#include "Rtc.h"
#include "CdmaModule.h"
#include "Communication.h"
#include "DebugCmd.h"
#include "Record.h"
#include "BeiDouModule.h"
#include "string.h"
#include "ctype.h"

//数据包解析协议
typedef enum
{
  //命令帧头
  E_UART_CMD_RECEIVING_HEADER,

  //命令
  E_UART_CMD_RECEIVING_CMD,

  //接收数据总长
  E_UART_CMD_RECEIVING_LENGHT,

  //接收数据主体
  E_UART_CMD_RECEIVING_BODY,

  //数据有效
  E_UART_CMD_RECEIVING_VALID

}teDebugCmdAssembleState;

/* 数据组包结构声明 */
typedef struct
{
  UINT8 *pu8AssembleBuf;  //组包区数据指针

  UINT8 u8CmdLen;         //命令长度

  UINT16 u16BufDataLen;     //已经组包的数据长度

	UINT8 u8Len;						//接收数据长度为2byte，记录正在接收的高地位

  UINT16 u16RevDataLen ;   //每包数据的长度

  teDebugCmdAssembleState eAssembleState;  //组包状态

  SINT8 sBdDog;

}_tsBdCmdCb;

_tsBdCmdCb sBdCmdCb;


//IC检测命令
UINT8 au8Icjc[12] = {0x24, 0x49, 0x43, 0x4a, 0x43, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x2b};
UINT8 au8LocationAddr[3] = {0};

/****************************************************************************
函数名称：BdSendCmdDWSQ()
功    能：北斗模块发送定位申请

参    数：无
返回值  ：无
*****************************************************************************/
void BdSendCmdDWSQ(void)
{

}

/****************************************************************************
函数名称：BdSendCmdSJSC()
功    能：北斗模块发送时间输出命令

参    数：无
返回值  ：无
*****************************************************************************/
void BdSendCmdSJSC(void)
{

}

/*****************************************************************************
 函 数 名  : Delay
 功能描述  : 在12MHz晶振 1T下 延时时间约等于usTime ms
 输入参数  : USHORT16 usTime
 输出参数  : NONE
 返 回 值  :
*****************************************************************************/
/*
static void Delay(UINT16 usTime)
{
    UINT16 j;
    UINT32 i;

    for (i = usTime; i > 0; i--)
    {
        for(j = 110; j > 0; j--);
    }
}*/

/****************************************************************************
函数名称：BdSendCmdTXSQ()
功    能：北斗模块通讯申请

参    数：无
返回值  ：无
*****************************************************************************/
void BdSendCmdTXSQ(UINT8 *pSendAddr, UINT8 *pMsg, UINT8 u8MsgLen)
{
    UINT8 *pInfo;   //发送报文内存指针
	UINT8 i, j, u8Check;

    pInfo = OSMemGet(pSmallMem, &err);

	//通讯申请命令组包
	i = 0;
	memcpy(&pInfo[i], "$TXSQ", 5);					//输入发送命令
	i += 5;

	pInfo[i++] = (UINT8)(((UINT16)u8MsgLen + 18) >> 8);								//电文长度加上18个固定长度为通讯数据长度
	pInfo[i++] = (UINT8)( (UINT16)u8MsgLen + 18);

	memcpy(&pInfo[i], au8LocationAddr, 3);														//获取本机卡号
	i += 3;

	pInfo[i++]  = 0x46;								                                    //ascii 为0x46；汉字编码为0x44

	memcpy(&pInfo[i], pSendAddr, 3);																		//发送方地址
	i += 3;

	pInfo[i++] = (UINT8)((UINT16)u8MsgLen * 8 >> 8);								  //电文信息长度，bit数
	pInfo[i++] = (UINT8)((UINT16)u8MsgLen * 8);

	pInfo[i++] = 0;																										//应答位

	memcpy(&pInfo[i], pMsg, u8MsgLen);
	i += u8MsgLen;

    u8Check = 0;
	for(j=0; j<i; j++)
	{
		u8Check ^= pInfo[j];
	}

    pInfo[i] = u8Check;

    Uart3EmptyRxBuf();
//  sBdCmdCb.u16BufDataLen = 0;

	Uart3SendBuffer(&pInfo[0], i+1);

    OSMemPut(pSmallMem, pInfo);

  //延时2S，确保反馈信息
 //   g_u16TimerDelay=0;
//    while(g_u16TimerDelay<200);
}


/****************************************************************************
函数名称：BdSendCmdICJC()
功    能：北斗IC检测，存入

参    数：无
返回值  ：无
*****************************************************************************/
void BdSendCmdICJC(void)
{
  SINT8 s8Dog;
  UINT8 *pRxBuf;
  UINT8 i;

//检测一下是否插入IC卡
#if BDIC_DET_EN >0
  if(!BdIcDet())
  {
    Uart32SendString("#BeiDou IC Card Not Found!\r\n ");
  }
#endif

  pRxBuf = OSMemGet(pSmallMem, &err);

  g_u8TimerDelay=0;
  while(g_u8TimerDelay<100);

  Uart3EmptyRxBuf();
  Uart3SendBuffer(au8Icjc, 12);

  s8Dog = SwdGet();
  SwdSetLifeTime(s8Dog, 1000);
  SwdEnable(s8Dog);

  while(TRUE)
  {
    if(Uart3GetRxDataLength()>21)
    {
      Uart3GetRxData(pRxBuf, 22);
      memcpy(&au8LocationAddr[0], &pRxBuf[7],3);

      break;
    }

    //1秒超时没有收到IC信息则再次发送一次，发送5次后若没有数据直接返回
    if(SwdGetLifeTime(s8Dog) == 0)
    {
      i++;
      if(i>5)
      {
        break;
      }
      else
      {
        SwdReset(s8Dog);
        Uart3EmptyRxBuf();
        Uart3SendBuffer(au8Icjc, 12);
      }
    }
  }

  SwdDisable(s8Dog);
  SwdFree(s8Dog);
  OSMemPut(pSmallMem, pRxBuf);
}

/***********************************************************************************************
  函数名称：GetBeiDouIcNum()
  函数功能：获取本机的北斗卡号

  输入：无
  输出：无
***********************************************************************************************/
void GetBeiDouIcNum(UINT8 *ptr)
{
  memcpy(ptr, &au8LocationAddr[0], 3);
}

/***********************************************************************************************
  函数名称：GetBeiDouIcNum()
  函数功能：获取北斗接收机卡号

  输入：无
  输出：无
***********************************************************************************************/
void GetBeiDouSendNum(UINT8 *ptr)
{
  ptr[0] = sRecordCb.au8BdRevNbr[0];
  ptr[1] = sRecordCb.au8BdRevNbr[1];
  ptr[2] = sRecordCb.au8BdRevNbr[2];
}

/***********************************************************************************************
  函数名称：GetBeiDouBckSendNum()
  函数功能：获取北斗备用接收机卡号

  输入：无
  输出：无
***********************************************************************************************/
void GetBeiDouBckSendNum(UINT8 *ptr)
{
  ptr[0] = sRecordCb.au8BdBckNbr[0];
  ptr[1] = sRecordCb.au8BdBckNbr[1];
  ptr[2] = sRecordCb.au8BdBckNbr[2];
}

/***********************************************************************************************
  函数名称：CmdParseTXXX()
  函数功能：通讯信息命令解析

  输入：无
  输出：无
***********************************************************************************************/
#define OFFSET(a, b)   ((UINT8)(&((a *)0)->b))
void CmdParseTXXX(void)
{
  UINT8 *ptr;
  UINT8 u8Temp, u8Field;
  UINT16 u16Len;

  ptr = sBdCmdCb.pu8AssembleBuf;
  u8Field = OFFSET(_tsCmdTxxxCb, au8MsgLen);

  u8Temp = ptr[u8Field];
  u16Len = ((UINT16)u8Temp) << 8;
  u8Temp = ptr[u8Field+1];
  u16Len += u8Temp;
  u8Temp  = (UINT8)(u16Len / 8);

  Uart2SendString("#BeiDou Receiv One Massage: ");
  Uart2SendBuffer(&ptr[u8Field+2], u8Temp);
  Uart2SendString(".\r\n");

#ifdef BEIDOU_TEST
  memset(&au8BdRxInfo[0], 0, sizeof(au8BdRxInfo));
  u8Field = OFFSET(_tsCmdTxxxCb, pMsg);
  memcpy(&au8BdRxInfo[0], &ptr[u8Field], u8Temp);

  if(memcmp(&au8BdRxInfo[0], &au8BdSendInfo[0], sizeof(au8BdRxInfo)) == 0)
  {
     u16BdRxNum++;
  }
#endif
}

/***********************************************************************************************
  函数名称：CmdParseDWXX()
  函数功能：定位信息命令解析

  输入：无
  输出：无
***********************************************************************************************/
void CmdParseDWXX(void)
{
  memcpy(&sCmdDwxxCb, &sBdCmdCb.pu8AssembleBuf[0], sizeof(sCmdDwxxCb));
  Uart2SendBuffer(&sCmdDwxxCb.au8BdCmd[0], sizeof(sCmdDwxxCb));
}

/***********************************************************************************************
  函数名称：CmdParseFKXX()
  函数功能：定位信息命令解析

  输入：无
  输出：无
***********************************************************************************************/
void CmdParseFKXX(void )
{
   memcpy(&sCmdFkxxCb, &sBdCmdCb.pu8AssembleBuf[0], sizeof(sCmdFkxxCb));

   switch(sCmdFkxxCb.u8BackInfo)
   {
    case 0:
     {
       u16BdSendSucceed++;
       Uart2SendString("#BeiDou Send Massage Successful! \r\n");
       break;
     }

     case 1:
     {
       Uart2SendString("#BeiDou Send Massage Failed! \r\n");
       break;
     }

     case 2:
     {
       Uart2SendString("#BeiDou Signal Is Unlocked! \r\n");
       break;
     }

     case 3:
     {
       Uart2SendString("#BeiDou Battery Is Low! \r\n");
       break;
     }

     case 4:
     {
       Uart2SendString("#BeiDou Send Massage Time Don`t Come! \r\n");
       break;
     }

     case 5:
     {
       Uart2SendString("#BeiDou Encryption And Decryption Is Error! \r\n");
       break;
     }

     case 6:
     {
       Uart2SendString("#BeiDou Crc Check Is Error! \r\n");
       break;
     }

     default: break;
   }
}

/***********************************************************************************************
  函数名称：BdCmdAssemble(UINT8 u8InData)
  函数功能：把接收的数据组合成有效的数据包

  输入：u8InData
  输出：无
***********************************************************************************************/
void BdCmdParse(UINT8 *pCmd)
{
		if(memcmp(pCmd, "$TXXX", 5) == 0)
		{
			CmdParseTXXX();
		}
		else if(memcmp(pCmd, "$DWXX", 5) == 0)
		{
//			CmdParseDWXX();
		}
        else if(memcmp(pCmd, "$FKXX", 5) == 0)
		{
			CmdParseFKXX();
		}
		else if(memcmp(pCmd, "$ICXX", 5) == 0)
		{
//			CmdParseICXX();
		}
		else if(memcmp(pCmd, "$SJXX", 5) == 0)
		{
//			CmdParseSJXX();
		}

}


/***********************************************************************************************
  函数名称：BdCmdAssemble(UINT8 u8InData)
  函数功能：把接收的数据组合成有效的数据包

  输入：u8InData
  输出：无
***********************************************************************************************/
void BdCmdAssemble(UINT8 u8InData)
{
  switch(sBdCmdCb.eAssembleState)
  {
    case E_UART_CMD_RECEIVING_HEADER:
    {
      if(u8InData == '$')
      {
        sBdCmdCb.pu8AssembleBuf[0] = u8InData;
		sBdCmdCb.u16BufDataLen     = 1;
	    sBdCmdCb.eAssembleState    = E_UART_CMD_RECEIVING_CMD;
      }
      break;
    }

    case E_UART_CMD_RECEIVING_CMD:
    {
      if(sBdCmdCb.u8CmdLen < 4)
      {
        sBdCmdCb.pu8AssembleBuf[sBdCmdCb.u16BufDataLen++] = u8InData;
		sBdCmdCb.u8CmdLen++;

		if(sBdCmdCb.u8CmdLen == 4)
		{
			sBdCmdCb.u8CmdLen       = 0;
			sBdCmdCb.u8Len          = 0;
			sBdCmdCb.eAssembleState = E_UART_CMD_RECEIVING_LENGHT;
		}
      }
      break;
    }

    case E_UART_CMD_RECEIVING_LENGHT:
    {
      if(sBdCmdCb.u8Len < 2)
      {
      	sBdCmdCb.pu8AssembleBuf[sBdCmdCb.u16BufDataLen++] = u8InData;
      	sBdCmdCb.u8Len++;
		if(sBdCmdCb.u8Len == 1)
	    {
            sBdCmdCb.u16RevDataLen = (UINT16)u8InData << 8;
        }
	    else
		{
            if(sBdCmdCb.u8Len == 2)
            {
		        sBdCmdCb.u16RevDataLen += u8InData;
			    sBdCmdCb.eAssembleState = E_UART_CMD_RECEIVING_BODY;
		    }
	    }
      }
      break;
    }

    case E_UART_CMD_RECEIVING_BODY:
    {
      if(sBdCmdCb.u16BufDataLen < sBdCmdCb.u16RevDataLen)
      {
	        sBdCmdCb.pu8AssembleBuf[sBdCmdCb.u16BufDataLen++] = u8InData;

			if(sBdCmdCb.u16BufDataLen == sBdCmdCb.u16RevDataLen)
		    {
				sBdCmdCb.eAssembleState = E_UART_CMD_RECEIVING_VALID;
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
      sBdCmdCb.eAssembleState = E_UART_CMD_RECEIVING_HEADER;
      sBdCmdCb.u16BufDataLen = 0;
    }
  }
}

/**********************************************************************************************
 函数名称：DebugCmdInit
 函数功能：Debug cmd parse init

 参数： 无
 返回： 无
**********************************************************************************************/
void BdModuleInit(void)
{
   //IC检测
  BdSendCmdICJC();

  sBdCmdCb.u8CmdLen       = 0;
  sBdCmdCb.u16BufDataLen  = 0;
  sBdCmdCb.u16RevDataLen  = 0;
  sBdCmdCb.eAssembleState = E_UART_CMD_RECEIVING_HEADER;
  sBdCmdCb.pu8AssembleBuf = OSMemGet(pMediumMem, &err);

  //Get One soft timer
  sBdCmdCb.sBdDog = SwdGet();
  SwdSetName(sBdCmdCb.sBdDog, "BeiDouTimer");
  SwdSetLifeTime(sBdCmdCb.sBdDog, 500);
}

#ifdef BEIDOU_TEST
void GetBufBdInfo(void)
{
  UINT8 *ptr;
  UINT8 u8Len;
  UINT8 i,j;

  ptr = OSMemGet(pSmallMem, &err);
  u8Len = Uart3GetRxDataLength();

  if(u8Len > 30)
  {
    Uart3GetRxData(ptr, u8Len);

    i = 0;
    while((ptr[i] != '#') && (i<u8Len))
    {
      i++;
    }

    j = i+1;
    while((ptr[j] != '#') && (j<u8Len))
    {
      j++;
    }

    Uart2SendString("#BeiDou Receiv One Massage: ");
    Uart2SendBuffer(&ptr[i], (j-i+1));
    Uart2SendString(".\r\n");

    memset(&au8BdRxInfo[0], 0, 34);
    memcpy(&au8BdRxInfo[0], &ptr[i], (j-i+1));

    if(memcmp(&au8BdRxInfo[0], &au8BdSendInfo[0], 34) == 0)
       u16BdRxNum++;
  }

  OSMemPut(pSmallMem, ptr);
}
#endif

/***************************************************************************
//函数名称：BdCmdRun
//功能描述：北斗命令解析，包括反馈信息，通信返回。
//
//参数：无
//返回：无
***************************************************************************/
void BdCmdRun(void)
{
  UINT8 au8Cmd[5];

  //北斗模块发送使能时，进行命令解析
  if(g_bBdSend == TRUE)
  {
    while((Uart3GetRxDataLength() > 0) && (sBdCmdCb.eAssembleState != E_UART_CMD_RECEIVING_VALID))
    {
      BdCmdAssemble(Uart3GetByte());

      if(SwdIsEnable(sBdCmdCb.sBdDog) == TRUE)
      {
        SwdDisable(sBdCmdCb.sBdDog);
      }
    }

    //数据包有效，对可用的指令进行解析
    if(sBdCmdCb.eAssembleState == E_UART_CMD_RECEIVING_VALID)
    {
      memcpy(&au8Cmd[0], &sBdCmdCb.pu8AssembleBuf[0], 5);

      BdCmdParse(&au8Cmd[0]);

      GetBufBdInfo();

      sBdCmdCb.u16BufDataLen = 0;
      sBdCmdCb.u16RevDataLen = 0;
      sBdCmdCb.eAssembleState = E_UART_CMD_RECEIVING_HEADER;
    }
    else
    {
      if(SwdIsEnable(sBdCmdCb.sBdDog) == TRUE)
      {
        if(SwdGetLifeTime(sBdCmdCb.sBdDog) == 0)
        {
          sBdCmdCb.eAssembleState = E_UART_CMD_RECEIVING_HEADER;
          sBdCmdCb.u16BufDataLen = 0;
          sBdCmdCb.u16RevDataLen = 0;

          SwdReset(sBdCmdCb.sBdDog);
          SwdDisable(sBdCmdCb.sBdDog);
        }
      }
      else
      {
        if(sBdCmdCb.u16BufDataLen != 0)
        {
          SwdReset(sBdCmdCb.sBdDog);
          SwdEnable(sBdCmdCb.sBdDog);
        }
      }
    }
  }
}





