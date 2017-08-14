/******************************************************************************
*
* Copyright (C) 2015, Sunic-ocean
* All Rights Reserved
*
* 文 件 名： Uart2.C
* 描    述：串口2底层驱动, 本uart口供Debug调试模块。

* 创建日期： 2015年5月6日09:23:55
* 作    者： Bob
* 当前版本： V1.0

------------------------------------------------------
*******************************************************************************/
#include "msp430x54x.h"
#include "global.h"
#include "Uart2.h"
#include "Uart3.h"
#include "string.h"
#include "SystemCtl.h"
#include "OS_MEM.h"

//UART2 宏定义
#define UART2_DISABLE_INTERRUPT()  (UCA2IE &= ~UCRXIE)
#define UART2_ENABLE_INTERRUPT()   (UCA2IE |=  UCRXIE)

#define Uart2TxUntilIdle()         while (!(UCTXIFG & UCA2IFG))             //等待以前的字符发送完毕

#define UART2_RX_MAX_SIZE       BUF_LARGE_SIZE
//UART1接收数据缓冲器
//UINT8 au8RxBuffer1[UART1_RX_MAX_SIZE];

//串口接收设备设置结构声明
typedef struct
{
  UINT16 u16RxHeader;

  UINT16 u16RxTail;

  UINT16 u16RxDataLen;

  UINT8 *pu8RxBuf;

}_tsUartCb;
_tsUartCb sUart2Cb;

/*******************************************
函数名称：Uart2Init()
功    能：UART2初始化，
          配置波特率为115200,8bit data，1bit stop，none verify
参    数：无
返回值  ：无
********************************************/
void Uart2Init(void)
{
    UART2_SEL |=  UART2_TXD + UART2_RXD;       		// 选择端口做UART通信端口
    UART2_DIR |=  UART2_TXD;
    UART2_DIR &= ~UART2_RXD;

    UCA2CTL1  =  UCSWRST;                          //状态机复位
    UCA2CTL1 |=  UCSSEL1;                          // UCLK = ACLK
    UCA2BR0   =  0x24;                             // 波特率115200
    UCA2BR1   =  0x00;                             //
    UCA2MCTL  =  UCBRS_7 + UCBRF_0;                // Modulation
    UCA2CTL1 &= ~UCSWRST;                          // 初始化UART状态机
    UCA2IE   |=  UCRXIE;                           // 使能USART0的接收中断

    sUart2Cb.pu8RxBuf = OSMemGet(pSmallMem, &err);//au8RxBuffer0;              //初始化缓冲区结构体
    sUart2Cb.u16RxHeader = 0;
    sUart2Cb.u16RxTail = 0;
    sUart2Cb.u16RxDataLen = 0;
}


/*******************************************
函数名称：Uart2SendByte()
功    能：通过UART2发送一个byte

参    数：无
返回值  ：无
********************************************/
void Uart2SendByte(UINT8 u8Data)
{
   UCA2TXBUF = u8Data;
   Uart2TxUntilIdle();
}

/************************************************************************************
  函数名称：Uart2SendString()
  功    能：通过UART2发送字符串

  参    数：*pString
  返回值  ：无
*********************************************************************************/
void Uart2SendString(const UINT8 * pString)
{
  while(*pString != 0)
  {
    UCA2TXBUF = *pString;
    pString++;

    Uart2TxUntilIdle();
  }
}

/********************************************************************************
  函数名称：Uart2SendBuffer
  函数功能：发送buffer中u8Length个byte

  参数：*pu8Buf
        u8Lenght   --  发送个数
  返回：无
*********************************************************************************/
void Uart2SendBuffer(const UINT8 *pu8Buf, UINT8 u8Length)
{
  while(u8Length > 0)
  {
    UCA2TXBUF = *pu8Buf;
    pu8Buf++;
    u8Length--;

    Uart2TxUntilIdle();
  }
}

/*********************************************************************************
  函数名称：Uart2EnableInterrupt
  函数功能：使能Uart2中断

  参数：无
  返回：无
*********************************************************************************/
void Uart2EnableInterrupt(void)
{
  UART2_ENABLE_INTERRUPT();
}

/*********************************************************************************
  函数名称：Uart2EnableInterrupt
  函数功能：关闭Uart2中断

  参数：无
  返回：无
*********************************************************************************/
void Uart2DisableInterrupt(void)
{
  UART2_DISABLE_INTERRUPT();
}


/***************************************************************************
//函数名称：Uart2GetRxDataLength
//功能描述：从串口缓冲区取得接收长度
//
//参数：无
//返回：u16DataLen
***************************************************************************/
UINT16 Uart2GetRxDataLength(void)
{
  UINT16 u16DataLen;

  UART2_DISABLE_INTERRUPT();
  u16DataLen = sUart2Cb.u16RxDataLen;
  UART2_ENABLE_INTERRUPT();

  return u16DataLen;
}

/***************************************************************************
//函数名称：Uart2GetByte
//功能描述：从串口缓冲区取得 一个数据
//
//参数：无
//返回：u16RxData
***************************************************************************/
UINT8 Uart2GetByte(void)
{
  UINT8 u8RxData;

  UART2_DISABLE_INTERRUPT();
  u8RxData = sUart2Cb.pu8RxBuf[sUart2Cb.u16RxHeader];
  sUart2Cb.u16RxHeader++;
  if(sUart2Cb.u16RxHeader == UART2_RX_MAX_SIZE)
  {
    sUart2Cb.u16RxHeader = 0;
  }
  sUart2Cb.u16RxDataLen--;
  UART2_ENABLE_INTERRUPT();

  return u8RxData;
}

/***************************************************************************
//函数名称：Uart2GetRxData
//功能描述：从串口缓冲区取得数据
//
//参数：无
//返回：UINT8
***************************************************************************/
UINT16 Uart2GetRxData(UINT8 *pu8Buffer, UINT16 u16DataLen)
{
  UINT16 u16Len;
  UINT16 u16HeaderToEndOfBuf;

  UART2_DISABLE_INTERRUPT();
  u16Len = sUart2Cb.u16RxDataLen;
  UART2_ENABLE_INTERRUPT();

  //接收到的数据和取得数据为0 ，直接返回0
  if((u16Len == 0) || (u16DataLen == 0))
  {
    return 0;
  }

  //接收到的数据长度小于所要长度，以实际长度为准
  if(u16DataLen > u16Len)
  {
    u16DataLen = u16Len;
  }

  //取得buf帧头到帧尾的长度
  u16HeaderToEndOfBuf = UART2_RX_MAX_SIZE - sUart2Cb.u16RxHeader;

  //若帧头到帧尾长度大于所要的长度
  if(u16HeaderToEndOfBuf > u16DataLen)
  {
    memcpy(pu8Buffer, sUart2Cb.pu8RxBuf+sUart2Cb.u16RxHeader, u16DataLen);
    UART2_DISABLE_INTERRUPT();
    sUart2Cb.u16RxDataLen -= u16DataLen;
    sUart2Cb.u16RxHeader += u16DataLen;
    UART2_ENABLE_INTERRUPT();
  }
  else
  {
    memcpy(pu8Buffer, sUart2Cb.pu8RxBuf+sUart2Cb.u16RxHeader, u16HeaderToEndOfBuf);
    memcpy(pu8Buffer+u16HeaderToEndOfBuf, sUart2Cb.pu8RxBuf, u16DataLen - u16HeaderToEndOfBuf);

    UART2_DISABLE_INTERRUPT();
    sUart2Cb.u16RxDataLen -= u16DataLen;
    sUart2Cb.u16RxHeader = u16DataLen - u16HeaderToEndOfBuf;
    UART2_ENABLE_INTERRUPT();
   }

  return u16DataLen;
}

/***************************************************************************
//函数名称：Uart2EmptyRxBuf
//功能描述：清空接收缓冲区
//
//参数：无
//返回：无
***************************************************************************/
void Uart2EmptyRxBuf(void)
{
  UART2_DISABLE_INTERRUPT();

  sUart2Cb.u16RxDataLen = 0;
  sUart2Cb.u16RxHeader  = 0;
  sUart2Cb.u16RxTail    = 0;

  UART2_ENABLE_INTERRUPT();
}

/****************************************************************************
  函数名称：UART2_RXISR
  功     能：UART2的接收中断服务函数，在这里唤醒
          CPU，使它退出低功耗模式

  参数：无
  返回：无
****************************************************************************/
#pragma vector = USCI_A2_VECTOR
__interrupt void UART2_RXISR(void)
{
  UINT8 u8Temp;

  u8Temp = UCA2RXBUF;
//  UCA0TXBUF = u8Temp;

  if(sUart2Cb.u16RxDataLen < UART2_RX_MAX_SIZE)
  {
    sUart2Cb.pu8RxBuf[sUart2Cb.u16RxTail++] = u8Temp;

    if(sUart2Cb.u16RxTail == UART2_RX_MAX_SIZE)
    {
      sUart2Cb.u16RxTail = 0;
    }

    sUart2Cb.u16RxDataLen++;
  }
}