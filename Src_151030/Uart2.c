/******************************************************************************
*
* Copyright (C) 2015, Sunic-ocean
* All Rights Reserved
*
* �� �� ���� Uart2.C
* ��    ��������2�ײ�����, ��uart�ڹ�Debug����ģ�顣

* �������ڣ� 2015��5��6��09:23:55
* ��    �ߣ� Bob
* ��ǰ�汾�� V1.0

------------------------------------------------------
*******************************************************************************/
#include "msp430x54x.h"
#include "global.h"
#include "Uart2.h"
#include "Uart3.h"
#include "string.h"
#include "SystemCtl.h"
#include "OS_MEM.h"

//UART2 �궨��
#define UART2_DISABLE_INTERRUPT()  (UCA2IE &= ~UCRXIE)
#define UART2_ENABLE_INTERRUPT()   (UCA2IE |=  UCRXIE)

#define Uart2TxUntilIdle()         while (!(UCTXIFG & UCA2IFG))             //�ȴ���ǰ���ַ��������

#define UART2_RX_MAX_SIZE       BUF_LARGE_SIZE
//UART1�������ݻ�����
//UINT8 au8RxBuffer1[UART1_RX_MAX_SIZE];

//���ڽ����豸���ýṹ����
typedef struct
{
  UINT16 u16RxHeader;

  UINT16 u16RxTail;

  UINT16 u16RxDataLen;

  UINT8 *pu8RxBuf;

}_tsUartCb;
_tsUartCb sUart2Cb;

/*******************************************
�������ƣ�Uart2Init()
��    �ܣ�UART2��ʼ����
          ���ò�����Ϊ115200,8bit data��1bit stop��none verify
��    ������
����ֵ  ����
********************************************/
void Uart2Init(void)
{
    UART2_SEL |=  UART2_TXD + UART2_RXD;       		// ѡ��˿���UARTͨ�Ŷ˿�
    UART2_DIR |=  UART2_TXD;
    UART2_DIR &= ~UART2_RXD;

    UCA2CTL1  =  UCSWRST;                          //״̬����λ
    UCA2CTL1 |=  UCSSEL1;                          // UCLK = ACLK
    UCA2BR0   =  0x24;                             // ������115200
    UCA2BR1   =  0x00;                             //
    UCA2MCTL  =  UCBRS_7 + UCBRF_0;                // Modulation
    UCA2CTL1 &= ~UCSWRST;                          // ��ʼ��UART״̬��
    UCA2IE   |=  UCRXIE;                           // ʹ��USART0�Ľ����ж�

    sUart2Cb.pu8RxBuf = OSMemGet(pSmallMem, &err);//au8RxBuffer0;              //��ʼ���������ṹ��
    sUart2Cb.u16RxHeader = 0;
    sUart2Cb.u16RxTail = 0;
    sUart2Cb.u16RxDataLen = 0;
}


/*******************************************
�������ƣ�Uart2SendByte()
��    �ܣ�ͨ��UART2����һ��byte

��    ������
����ֵ  ����
********************************************/
void Uart2SendByte(UINT8 u8Data)
{
   UCA2TXBUF = u8Data;
   Uart2TxUntilIdle();
}

/************************************************************************************
  �������ƣ�Uart2SendString()
  ��    �ܣ�ͨ��UART2�����ַ���

  ��    ����*pString
  ����ֵ  ����
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
  �������ƣ�Uart2SendBuffer
  �������ܣ�����buffer��u8Length��byte

  ������*pu8Buf
        u8Lenght   --  ���͸���
  ���أ���
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
  �������ƣ�Uart2EnableInterrupt
  �������ܣ�ʹ��Uart2�ж�

  ��������
  ���أ���
*********************************************************************************/
void Uart2EnableInterrupt(void)
{
  UART2_ENABLE_INTERRUPT();
}

/*********************************************************************************
  �������ƣ�Uart2EnableInterrupt
  �������ܣ��ر�Uart2�ж�

  ��������
  ���أ���
*********************************************************************************/
void Uart2DisableInterrupt(void)
{
  UART2_DISABLE_INTERRUPT();
}


/***************************************************************************
//�������ƣ�Uart2GetRxDataLength
//�����������Ӵ��ڻ�����ȡ�ý��ճ���
//
//��������
//���أ�u16DataLen
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
//�������ƣ�Uart2GetByte
//�����������Ӵ��ڻ�����ȡ�� һ������
//
//��������
//���أ�u16RxData
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
//�������ƣ�Uart2GetRxData
//�����������Ӵ��ڻ�����ȡ������
//
//��������
//���أ�UINT8
***************************************************************************/
UINT16 Uart2GetRxData(UINT8 *pu8Buffer, UINT16 u16DataLen)
{
  UINT16 u16Len;
  UINT16 u16HeaderToEndOfBuf;

  UART2_DISABLE_INTERRUPT();
  u16Len = sUart2Cb.u16RxDataLen;
  UART2_ENABLE_INTERRUPT();

  //���յ������ݺ�ȡ������Ϊ0 ��ֱ�ӷ���0
  if((u16Len == 0) || (u16DataLen == 0))
  {
    return 0;
  }

  //���յ������ݳ���С����Ҫ���ȣ���ʵ�ʳ���Ϊ׼
  if(u16DataLen > u16Len)
  {
    u16DataLen = u16Len;
  }

  //ȡ��buf֡ͷ��֡β�ĳ���
  u16HeaderToEndOfBuf = UART2_RX_MAX_SIZE - sUart2Cb.u16RxHeader;

  //��֡ͷ��֡β���ȴ�����Ҫ�ĳ���
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
//�������ƣ�Uart2EmptyRxBuf
//������������ս��ջ�����
//
//��������
//���أ���
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
  �������ƣ�UART2_RXISR
  ��     �ܣ�UART2�Ľ����жϷ������������﻽��
          CPU��ʹ���˳��͹���ģʽ

  ��������
  ���أ���
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