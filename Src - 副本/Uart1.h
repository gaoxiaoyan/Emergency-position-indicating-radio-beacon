
#ifndef __UART1_H__
#define __UART1_H__

#include "global.h"
#include "DataType.h"


/***************************************************************************
函数名称：Uart1Init()
功    能：UART1初始化，
          配置波特率为115200,8bit data，1bit stop，none verify
参    数：无
返回值  ：无
***************************************************************************/
extern void Uart1Init(void);

/*******************************************
函数名称：Uart1SendByte()
功    能：通过UART0发送一个byte
参    数：u8Data
返回值  ：无
********************************************/
extern void Uart1SendByte(UINT8 u8Data);


/*******************************************
函数名称：Uart1SendString()
功    能：通过UART0发送字符串
参    数：*pString
返回值  ：无
********************************************/
extern void Uart1SendString(const UINT8 * pString);

/********************************************************************************
  函数名称：Uart1SendBuffer
  函数功能：发送buffer中u8Length个byte

  参数：*pu8Buf
        u8Lenght   --  发送个数
  返回：无
*********************************************************************************/
extern void Uart1SendBuffer(const UINT8 *pu8Buf, UINT8 u8Length);

/***************************************************************************
  函数名称：Uart1EnableInterrupt
  函数功能：使能Uart中断

  参数：无
  返回：无
***************************************************************************/
extern void Uart1EnableInterrupt(void);

/***************************************************************************
  函数名称：Uart1DisableInterrupt
  函数功能：关闭Uart中断

  参数：无
  返回：无
***************************************************************************/
extern void Uart1DisableInterrupt(void);

/***************************************************************************
//函数名称：Uart1GetRxDataLength
//功能描述：从串口缓冲区取得接收长度
//
//参数：无
//返回：u8DataLen
***************************************************************************/
extern UINT16 Uart1GetRxDataLength(void);

/***************************************************************************
//函数名称：Uart1GetByte
//功能描述：从串口缓冲区取得 一个数据
//
//参数：无
//返回：u8RxData
***************************************************************************/
extern UINT8 Uart1GetByte(void);

/***************************************************************************
//函数名称：Uart1InsertByte
//功能描述：向串口缓冲区表头插入一个byte
//
//参数：u8Data -- 插入数据
//
//返回：无
***************************************************************************/
extern void Uart1InsertByte(UINT8 u8Data);

/***************************************************************************
//函数名称：Uart1GetRxData
//功能描述：从串口缓冲区取得数据
//
//参数：无
//返回：获取数据的长度
***************************************************************************/
extern UINT16 Uart1GetRxData(UINT8 *pu8Buffer, UINT16 u16DataLen);

/***************************************************************************
//函数名称：Uart1EmptyRxBuf
//功能描述：清空接收缓冲区
//
//参数：无
//返回：无
***************************************************************************/
extern void Uart1EmptyRxBuf(void);


#endif