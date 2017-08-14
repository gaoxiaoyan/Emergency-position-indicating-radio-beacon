/******************************************************************************
*
* Copyright (C) 2014, Sunic-ocean
* All Rights Reserved
*
* 文 件 名： Uart3.C
* 描    述：串口3底层驱动, 本uart口供北斗通讯使用。

* 创建日期： 2015年4月13日13:44:28
* 作    者： Bob
* 当前版本： V1.0

------------------------------------------------------
*******************************************************************************/
#include "msp430x54x.h"

#include "Uart3.h"
#include "Uart0.h"
#include "string.h"
#include "SystemCtl.h"

//UART3 宏定义
#define UART3_DISABLE_INTERRUPT()  (UCA3IE &= ~UCRXIE)
#define UART3_ENABLE_INTERRUPT()   (UCA3IE |=  UCRXIE)

#define Uart3TxUntilIdle()         while (!(UCTXIFG & UCA3IFG))             //等待以前的字符发送完毕

#define UART3_RX_MAX_SIZE       BUF_SMALL_SIZE
//UART3接收数据缓冲器
//UINT8 au8RxBuffer3[UART3_RX_MAX_SIZE];

//串口接收设备设置结构声明
typedef struct
{
  UINT16 u16RxHeader;

  UINT16 u16RxTail;

  UINT16 u16RxDataLen;

  UINT8 *pu8RxBuf;

}_tsUartCb;
_tsUartCb sUart3Cb;

/*******************************************
函数名称：Uart3Init()
功    能：UART3初始化，
          配置波特率为115200,8bit data，1bit stop，none verify
参    数：无
返回值  ：无
********************************************/
void Uart3Init(void)
{
    UART3_SEL |=  UART3_TXD + UART3_RXD;       // 选择端口做UART通信端口
    UART3_DIR |=  UART3_TXD;
    UART3_DIR &= ~UART3_RXD;

    UCA3CTL1  =  UCSWRST;                          //状态机复位
    UCA3CTL1 |=  UCSSEL1;                          // UCLK = ACLK
    UCA3BR0   =  0x24;                             // 波特率115200
    UCA3BR1   =  0x00;                             //
    UCA3MCTL  =  UCBRS_3 + UCBRF_0;                // Modulation
    UCA3CTL1 &= ~UCSWRST;                          // 初始化UART状态机
    UCA3IE   |=  UCRXIE;                           // 使能USART0的接收中断

    sUart3Cb.pu8RxBuf = OSMemGet(pMediumMem, &err);//au8RxBuffer3;              //初始化缓冲区结构体
    sUart3Cb.u16RxHeader = 0;
    sUart3Cb.u16RxTail = 0;
    sUart3Cb.u16RxDataLen = 0;
}


/*******************************************
函数名称：Uart3SendByte()
功    能：通过UART3发送一个byte

参    数：无
返回值  ：无
********************************************/
void Uart3SendByte(UINT8 u8Data)
{
   UCA3TXBUF = u8Data;
   Uart3TxUntilIdle();
}

/************************************************************************************
  函数名称：Uart3SendString()
  功    能：通过UART3发送字符串

  参    数：*pString
  返回值  ：无
*********************************************************************************/
void Uart3SendString(UINT8 * pString)
{
  UINT8 *ptr;

  ptr = pString;
  while(*ptr != 0)
  {
    UCA3TXBUF = *ptr;
    ptr++;

    Uart3TxUntilIdle();
  }
}

/********************************************************************************
  函数名称：Uart3SendBuffer
  函数功能：发送buffer中u8Length个byte

  参数：*pu8Buf
        u8Lenght   --  发送个数
  返回：无
*********************************************************************************/
void Uart3SendBuffer(UINT8 *pu8Buf, UINT8 u8Length)
{
//  UINT8 *ptr;

//  ptr = pu8Buf;
  while(u8Length > 0)
  {
    UCA3TXBUF = *pu8Buf;
    pu8Buf++;
    u8Length--;

    Uart3TxUntilIdle();
  }
}

/*********************************************************************************
  函数名称：Uart3EnableInterrupt
  函数功能：使能Uart3中断

  参数：无
  返回：无
*********************************************************************************/
void Uart3EnableInterrupt(void)
{
  UART3_ENABLE_INTERRUPT();
}

/*********************************************************************************
  函数名称：Uart3EnableInterrupt
  函数功能：关闭Uart3中断

  参数：无
  返回：无
*********************************************************************************/
void Uart3DisableInterrupt(void)
{
  UART3_DISABLE_INTERRUPT();
}


/***************************************************************************
//函数名称：Uart3GetRxDataLength
//功能描述：从串口缓冲区取得接收长度
//
//参数：无
//返回：u16DataLen
***************************************************************************/
UINT16 Uart3GetRxDataLength(void)
{
  UINT16 u16DataLen;

  UART3_DISABLE_INTERRUPT();
  u16DataLen = sUart3Cb.u16RxDataLen;
  UART3_ENABLE_INTERRUPT();

  return u16DataLen;
}

/***************************************************************************
//函数名称：Uart3GetByte
//功能描述：从串口缓冲区取得 一个数据
//
//参数：无
//返回：u16RxData
***************************************************************************/
UINT8 Uart3GetByte(void)
{
  UINT8 u8RxData;

  UART3_DISABLE_INTERRUPT();
  u8RxData = sUart3Cb.pu8RxBuf[sUart3Cb.u16RxHeader];
  sUart3Cb.u16RxHeader++;
  if(sUart3Cb.u16RxHeader == UART3_RX_MAX_SIZE)
  {
    sUart3Cb.u16RxHeader = 0;
  }
  sUart3Cb.u16RxDataLen--;
  UART3_ENABLE_INTERRUPT();

  return u8RxData;
}

/***************************************************************************
//函数名称：Uart3GetRxData
//功能描述：从串口缓冲区取得数据
//
//参数：无
//返回：UINT8
***************************************************************************/
UINT16 Uart3GetRxData(UINT8 *pu8Buffer, UINT16 u16DataLen)
{
  UINT16 u16Len;
  UINT16 u16HeaderToEndOfBuf;

  UART3_DISABLE_INTERRUPT();
  u16Len = sUart3Cb.u16RxDataLen;
  UART3_ENABLE_INTERRUPT();

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
  u16HeaderToEndOfBuf = UART3_RX_MAX_SIZE - sUart3Cb.u16RxHeader;

  //若帧头到帧尾长度大于所要的长度
  if(u16HeaderToEndOfBuf > u16DataLen)
  {
    memcpy(pu8Buffer, sUart3Cb.pu8RxBuf+sUart3Cb.u16RxHeader, u16DataLen);
    UART3_DISABLE_INTERRUPT();
    sUart3Cb.u16RxDataLen -= u16DataLen;
    sUart3Cb.u16RxHeader += u16DataLen;
    UART3_ENABLE_INTERRUPT();
  }
  else
  {
    memcpy(pu8Buffer, sUart3Cb.pu8RxBuf+sUart3Cb.u16RxHeader, u16HeaderToEndOfBuf);
    memcpy(pu8Buffer+u16HeaderToEndOfBuf, sUart3Cb.pu8RxBuf, u16DataLen - u16HeaderToEndOfBuf);

    UART3_DISABLE_INTERRUPT();
    sUart3Cb.u16RxDataLen -= u16DataLen;
    sUart3Cb.u16RxHeader = u16DataLen - u16HeaderToEndOfBuf;
    UART3_ENABLE_INTERRUPT();
   }

  return u16DataLen;
}

/***************************************************************************
//函数名称：Uart0EmptyRxBuf
//功能描述：清空接收缓冲区
//
//参数：无
//返回：无
***************************************************************************/
void Uart3EmptyRxBuf(void)
{
  UART3_DISABLE_INTERRUPT();

  sUart3Cb.u16RxDataLen = 0;
  sUart3Cb.u16RxHeader  = 0;
  sUart3Cb.u16RxTail    = 0;

  UART3_ENABLE_INTERRUPT();
}

/****************************************************************************
  函数名称：UART3_RXISR
  功     能：UART3的接收中断服务函数，在这里唤醒
          CPU，使它退出低功耗模式

  参数：无
  返回：无
****************************************************************************/
#pragma vector = USCI_A3_VECTOR
__interrupt void UART3_RXISR(void)
{
  UINT8 u8Temp;

  u8Temp = UCA3RXBUF;

  if(sUart3Cb.u16RxDataLen < UART3_RX_MAX_SIZE)
  {
    sUart3Cb.pu8RxBuf[sUart3Cb.u16RxTail++] = u8Temp;

    if(sUart3Cb.u16RxTail == UART3_RX_MAX_SIZE)
    {
      sUart3Cb.u16RxTail = 0;
    }

    sUart3Cb.u16RxDataLen++;
  }
}


