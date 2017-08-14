/******************************************************************************
*											
* Copyright (C) 2014, Sunic-ocean
* All Rights Reserved										
*
* 文 件 名： Uart0.C
* 描    述：串口0底层驱动, 本uart口供CDMA模块。

* 创建日期： 2015年4月16日16:58:36
* 作    者： Bob
* 当前版本： V1.0

------------------------------------------------------
*******************************************************************************/
#include "msp430x54x.h"

#include "global.h"
#include "Uart0.h"
#include "Uart3.h"
#include "string.h"
#include "SystemCtl.h"
#include "OS_MEM.h"

//UART0 宏定义
#define UART0_DISABLE_INTERRUPT()  (UCA0IE &= ~UCRXIE)
#define UART0_ENABLE_INTERRUPT()   (UCA0IE |=  UCRXIE)

#define Uart0TxUntilIdle()         while (!(UCTXIFG & UCA0IFG))             //等待以前的字符发送完毕

#define UART0_RX_MAX_SIZE       BUF_LARGE_SIZE
//UART2接收数据缓冲器
//UINT8 au8RxBuffer2[UART2_RX_MAX_SIZE];

//串口接收设备设置结构声明
typedef struct
{
  UINT16 u16RxHeader;

  UINT16 u16RxTail;

  UINT16 u16RxDataLen;

  UINT8 *pu8RxBuf;

}_tsUartCb;
_tsUartCb sUart0Cb;

/*******************************************
函数名称：Uart0Init()
功    能：UART0初始化，
          配置波特率为9600,8bit data，1bit stop，none verify
参    数：无
返回值  ：无
********************************************/
void Uart0Init(void)
{
    UART0_SEL |=  UART0_TXD + UART0_RXD;       // 选择端口做UART通信端口
    UART0_DIR |=  UART0_TXD;
    UART0_DIR &= ~UART0_RXD;
    
    UCA0CTL1  =  UCSWRST;                          //状态机复位
    UCA0CTL1 |=  UCSSEL1;                          // UCLK = ACLK
    UCA0BR0   =  0x24;                             // 波特率115200
    UCA0BR1   =  0x00;                             //
    UCA0MCTL  =  UCBRS_7 + UCBRF_0;                // Modulation
    UCA0CTL1 &= ~UCSWRST;                          // 初始化UART状态机
    UCA0IE   |=  UCRXIE;                           // 使能USART0的接收中断

    sUart0Cb.pu8RxBuf = OSMemGet(pLargeMem, &err);//au8RxBuffer0;              //初始化缓冲区结构体
    sUart0Cb.u16RxHeader = 0;
    sUart0Cb.u16RxTail = 0;
    sUart0Cb.u16RxDataLen = 0;
}


/*******************************************
函数名称：Uart0SendByte()
功    能：通过UART3发送一个byte

参    数：无
返回值  ：无
********************************************/
void Uart0SendByte(UINT8 u8Data)
{
   UCA0TXBUF = u8Data;
   Uart0TxUntilIdle();
}

/************************************************************************************
  函数名称：Uart0SendString()
  功    能：通过UART0发送字符串

  参    数：*pString
  返回值  ：无
*********************************************************************************/
void Uart0SendString(const UINT8 * pString)
{
  while(*pString != 0)
  {
    UCA0TXBUF = *pString;
    pString++;

    Uart0TxUntilIdle();
  }
}

/********************************************************************************
  函数名称：Uart0SendBuffer
  函数功能：发送buffer中u8Length个byte

  参数：*pu8Buf
        u8Lenght   --  发送个数
  返回：无
*********************************************************************************/
void Uart0SendBuffer(const UINT8 *pu8Buf, UINT8 u8Length)
{
  while(u8Length > 0)
  {
    UCA0TXBUF = *pu8Buf;
    pu8Buf++;
    u8Length--;

    Uart0TxUntilIdle();
  }
}

/*********************************************************************************
  函数名称：Uart0EnableInterrupt
  函数功能：使能Uart3中断

  参数：无
  返回：无
*********************************************************************************/
void Uart0EnableInterrupt(void)
{
  UART0_ENABLE_INTERRUPT();
}

/*********************************************************************************
  函数名称：Uart0EnableInterrupt
  函数功能：关闭Uart0中断

  参数：无
  返回：无
*********************************************************************************/
void Uart0DisableInterrupt(void)
{
  UART0_DISABLE_INTERRUPT();
}


/***************************************************************************
//函数名称：Uart0GetRxDataLength
//功能描述：从串口缓冲区取得接收长度
//
//参数：无
//返回：u16DataLen
***************************************************************************/
UINT16 Uart0GetRxDataLength(void)
{
  UINT16 u16DataLen;

  UART0_DISABLE_INTERRUPT();
  u16DataLen = sUart0Cb.u16RxDataLen;
  UART0_ENABLE_INTERRUPT();

  return u16DataLen;
}

/***************************************************************************
//函数名称：Uart0GetByte
//功能描述：从串口缓冲区取得 一个数据
//
//参数：无
//返回：u16RxData
***************************************************************************/
UINT8 Uart0GetByte(void)
{
  UINT8 u8RxData;
	
  UART0_DISABLE_INTERRUPT();
  u8RxData = sUart0Cb.pu8RxBuf[sUart0Cb.u16RxHeader];
  sUart0Cb.u16RxHeader++;
  if(sUart0Cb.u16RxHeader == UART0_RX_MAX_SIZE)
  {
    sUart0Cb.u16RxHeader = 0;		
  }
  sUart0Cb.u16RxDataLen--;
  UART0_ENABLE_INTERRUPT();

  return u8RxData;
}

/***************************************************************************
//函数名称：Uart0GetRxData
//功能描述：从串口缓冲区取得数据
//
//参数：无
//返回：UINT8
***************************************************************************/
UINT16 Uart0GetRxData(UINT8 *pu8Buffer, UINT16 u16DataLen)
{
  UINT16 u16Len;
  UINT16 u16HeaderToEndOfBuf;
	
  UART0_DISABLE_INTERRUPT();
  u16Len = sUart0Cb.u16RxDataLen;
  UART0_ENABLE_INTERRUPT();
	
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
  u16HeaderToEndOfBuf = UART0_RX_MAX_SIZE - sUart0Cb.u16RxHeader;
	
  //若帧头到帧尾长度大于所要的长度
  if(u16HeaderToEndOfBuf > u16DataLen)
  {
    memcpy(pu8Buffer, sUart0Cb.pu8RxBuf+sUart0Cb.u16RxHeader, u16DataLen);
    UART0_DISABLE_INTERRUPT();
    sUart0Cb.u16RxDataLen -= u16DataLen;
    sUart0Cb.u16RxHeader += u16DataLen;
    UART0_ENABLE_INTERRUPT();	
  }
  else
  {
    memcpy(pu8Buffer, sUart0Cb.pu8RxBuf+sUart0Cb.u16RxHeader, u16HeaderToEndOfBuf);	
    memcpy(pu8Buffer+u16HeaderToEndOfBuf, sUart0Cb.pu8RxBuf, u16DataLen - u16HeaderToEndOfBuf);

    UART0_DISABLE_INTERRUPT();
    sUart0Cb.u16RxDataLen -= u16DataLen;
    sUart0Cb.u16RxHeader = u16DataLen - u16HeaderToEndOfBuf;
    UART0_ENABLE_INTERRUPT();
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
void Uart0EmptyRxBuf(void)
{
  UART0_DISABLE_INTERRUPT();
  sUart0Cb.u16RxDataLen = 0;
  sUart0Cb.u16RxHeader = sUart0Cb.u16RxTail;
  UART0_ENABLE_INTERRUPT();
}

/****************************************************************************
  函数名称：UART0_RXISR
  功     能：UART0的接收中断服务函数，在这里唤醒
          CPU，使它退出低功耗模式

  参数：无
  返回：无
****************************************************************************/
#pragma vector = USCI_A0_VECTOR
__interrupt void UART0_RXISR(void)
{
  UINT8 u8Temp;

  u8Temp = UCA0RXBUF;

  if(sUart0Cb.u16RxDataLen < UART0_RX_MAX_SIZE)
  {
    sUart0Cb.pu8RxBuf[sUart0Cb.u16RxTail++] = u8Temp;
  
    if(sUart0Cb.u16RxTail == UART0_RX_MAX_SIZE)
    {
      sUart0Cb.u16RxTail = 0;
    }
      
    sUart0Cb.u16RxDataLen++;
  }
}