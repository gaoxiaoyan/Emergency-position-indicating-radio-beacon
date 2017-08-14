/******************************************************************************
*											
* Copyright (C) 2014, Sunic-ocean
* All Rights Reserved										
*
* 文 件 名： Uart1.C
* 描    述：串口1底层驱动, 本uart口供北斗GPS定位使用。

* 创建日期： 2015年4月16日16:58:36
* 作    者： Bob
* 当前版本： V1.0

------------------------------------------------------
*******************************************************************************/
#include "msp430x54x.h"

#include "global.h"
#include "Uart1.h"
#include "Uart3.h"
#include "string.h"
#include "SystemCtl.h"
#include "OS_MEM.h"

//UART1 宏定义
#define UART1_DISABLE_INTERRUPT()  (UCA1IE &= ~UCRXIE)
#define UART1_ENABLE_INTERRUPT()   (UCA1IE |=  UCRXIE)

#define Uart1TxUntilIdle()         while (!(UCTXIFG & UCA1IFG))             //等待以前的字符发送完毕

#define UART1_RX_MAX_SIZE       BUF_LARGE_SIZE
//UART1接收数据缓冲器
//UINT8 au8RxBuffer2[UART1_RX_MAX_SIZE];

//串口接收设备设置结构声明
typedef struct
{
  UINT16 u16RxHeader;

  UINT16 u16RxTail;

  UINT16 u16RxDataLen;

  UINT8 *pu8RxBuf;

}_tsUartCb;
_tsUartCb sUart1Cb;

/*******************************************
函数名称：Uart1Init()
功    能：UART1初始化，
          配置波特率为9600,8bit data，1bit stop，none verify
参    数：无
返回值  ：无
********************************************/
void Uart1Init(void)
{
    UART1_SEL |=  UART1_TXD + UART1_RXD;       // 选择端口做UART通信端口
    UART1_DIR |=  UART1_TXD;
    UART1_DIR &= ~UART1_RXD;
    
    UCA1CTL1  =  UCSWRST;                          //状态机复位
    UCA1CTL1 |=  UCSSEL1;                          // UCLK = ACLK
    UCA1BR0   =  0x24;                             // 波特率9600
    UCA1BR1   =  0x00;                             //
    UCA1MCTL  =  UCBRS_7 + UCBRF_0;                // Modulation
    UCA1CTL1 &= ~UCSWRST;                          // 初始化UART状态机
    UCA1IE   |=  UCRXIE;                           // 使能USART0的接收中断

    sUart1Cb.pu8RxBuf = OSMemGet(pLargeMem, &err);//au8RxBuffer2;              //初始化缓冲区结构体
    sUart1Cb.u16RxHeader = 0;
    sUart1Cb.u16RxTail = 0;
    sUart1Cb.u16RxDataLen = 0;
}


/*******************************************
函数名称：Uart1SendByte()
功    能：通过UART1发送一个byte

参    数：无
返回值  ：无
********************************************/
void Uart1SendByte(UINT8 u8Data)
{
   UCA1TXBUF = u8Data;
   Uart1TxUntilIdle();
}

/************************************************************************************
  函数名称：Uart1SendString()
  功    能：通过UART1发送字符串

  参    数：*pString
  返回值  ：无
*********************************************************************************/
void Uart1SendString(const UINT8 * pString)
{
  while(*pString != 0)
  {
    UCA1TXBUF = *pString;
    pString++;

    Uart1TxUntilIdle();
  }
}

/********************************************************************************
  函数名称：Uart1SendBuffer
  函数功能：发送buffer中u8Length个byte

  参数：*pu8Buf
        u8Lenght   --  发送个数
  返回：无
*********************************************************************************/
void Uart1SendBuffer(const UINT8 *pu8Buf, UINT8 u8Length)
{
  while(u8Length > 0)
  {
    UCA1TXBUF = *pu8Buf;
    pu8Buf++;
    u8Length--;

    Uart1TxUntilIdle();
  }
}

/*********************************************************************************
  函数名称：Uart1EnableInterrupt
  函数功能：使能Uart3中断

  参数：无
  返回：无
*********************************************************************************/
void Uart1EnableInterrupt(void)
{
  UART1_ENABLE_INTERRUPT();
}

/*********************************************************************************
  函数名称：Uart1EnableInterrupt
  函数功能：关闭Uart1中断

  参数：无
  返回：无
*********************************************************************************/
void Uart1DisableInterrupt(void)
{
  UART1_DISABLE_INTERRUPT();
}


/***************************************************************************
//函数名称：Uart1GetRxDataLength
//功能描述：从串口缓冲区取得接收长度
//
//参数：无
//返回：u16DataLen
***************************************************************************/
UINT16 Uart1GetRxDataLength(void)
{
  UINT16 u16DataLen;

  UART1_DISABLE_INTERRUPT();
  u16DataLen = sUart1Cb.u16RxDataLen;
  UART1_ENABLE_INTERRUPT();

  return u16DataLen;
}

/***************************************************************************
//函数名称：Uart1GetByte
//功能描述：从串口缓冲区取得 一个数据
//
//参数：无
//返回：u16RxData
***************************************************************************/
UINT8 Uart1GetByte(void)
{
  UINT8 u8RxData;
	
  UART1_DISABLE_INTERRUPT();
  u8RxData = sUart1Cb.pu8RxBuf[sUart1Cb.u16RxHeader];
  sUart1Cb.u16RxHeader++;
  if(sUart1Cb.u16RxHeader == UART1_RX_MAX_SIZE)
  {
    sUart1Cb.u16RxHeader = 0;		
  }
  sUart1Cb.u16RxDataLen--;
  UART1_ENABLE_INTERRUPT();

  return u8RxData;
}

/***************************************************************************
//函数名称：Uart1InsertByte
//功能描述：向串口缓冲区表头插入一个byte
//
//参数：u8Data -- 插入数据
//
//返回：无
***************************************************************************/
void Uart1InsertByte(UINT8 u8Data)
{
    UART1_DISABLE_INTERRUPT();

    if(sUart1Cb.u16RxHeader > 0)
    {
        sUart1Cb.u16RxHeader--;
    }
    else
    {
        sUart1Cb.u16RxHeader = UART1_RX_MAX_SIZE - 1;
    }

    sUart1Cb.pu8RxBuf[sUart1Cb.u16RxHeader] = u8Data;
    sUart1Cb.u16RxDataLen++;

    UART1_ENABLE_INTERRUPT();
}

/***************************************************************************
//函数名称：Uart3GetRxData
//功能描述：从串口缓冲区取得数据
//
//参数：无
//返回：UINT8
***************************************************************************/
UINT16 Uart1GetRxData(UINT8 *pu8Buffer, UINT16 u16DataLen)
{
  UINT16 u16Len;
  UINT16 u16HeaderToEndOfBuf;
	
  UART1_DISABLE_INTERRUPT();
  u16Len = sUart1Cb.u16RxDataLen;
  UART1_ENABLE_INTERRUPT();
	
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
  u16HeaderToEndOfBuf = UART1_RX_MAX_SIZE - sUart1Cb.u16RxHeader;
	
  //若帧头到帧尾长度大于所要的长度
  if(u16HeaderToEndOfBuf > u16DataLen)
  {
    memcpy(pu8Buffer, sUart1Cb.pu8RxBuf+sUart1Cb.u16RxHeader, u16DataLen);
    UART1_DISABLE_INTERRUPT();
    sUart1Cb.u16RxDataLen -= u16DataLen;
    sUart1Cb.u16RxHeader += u16DataLen;
    UART1_ENABLE_INTERRUPT();	
  }
  else
  {
    memcpy(pu8Buffer, sUart1Cb.pu8RxBuf+sUart1Cb.u16RxHeader, u16HeaderToEndOfBuf);	
    memcpy(pu8Buffer+u16HeaderToEndOfBuf, sUart1Cb.pu8RxBuf, u16DataLen - u16HeaderToEndOfBuf);

    UART1_DISABLE_INTERRUPT();
    sUart1Cb.u16RxDataLen -= u16DataLen;
    sUart1Cb.u16RxHeader = u16DataLen - u16HeaderToEndOfBuf;
    UART1_ENABLE_INTERRUPT();
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
void Uart1EmptyRxBuf(void)
{
  UART1_DISABLE_INTERRUPT();
  sUart1Cb.u16RxDataLen = 0;
  sUart1Cb.u16RxHeader  = 0;
  sUart1Cb.u16RxTail    = 0;
  UART1_ENABLE_INTERRUPT();
}

/****************************************************************************
  函数名称：UART1_RXISR
  功     能：UART1的接收中断服务函数，在这里唤醒
          CPU，使它退出低功耗模式

  参数：无
  返回：无
****************************************************************************/
#pragma vector = USCI_A1_VECTOR
__interrupt void UART1_RXISR(void)
{
  UINT8 u8Temp;

  u8Temp = UCA1RXBUF;
   UCA2TXBUF = u8Temp;

  if(sUart1Cb.u16RxDataLen < UART1_RX_MAX_SIZE)
  {
    sUart1Cb.pu8RxBuf[sUart1Cb.u16RxTail++] = u8Temp;
  
    if(sUart1Cb.u16RxTail == UART1_RX_MAX_SIZE)
    {
      sUart1Cb.u16RxTail = 0;
    }
      
    sUart1Cb.u16RxDataLen++;
  }
}