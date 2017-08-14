/******************************************************************************
*
* Copyright (C) 2014, Sunic-ocean
* All Rights Reserved
*
* �� �� ���� Uart3.C
* ��    ��������3�ײ�����, ��uart�ڹ�����ͨѶʹ�á�

* �������ڣ� 2015��4��13��13:44:28
* ��    �ߣ� Bob
* ��ǰ�汾�� V1.0

------------------------------------------------------
*******************************************************************************/
#include "msp430x54x.h"

#include "Uart3.h"
#include "Uart0.h"
#include "string.h"
#include "SystemCtl.h"

//UART3 �궨��
#define UART3_DISABLE_INTERRUPT()  (UCA3IE &= ~UCRXIE)
#define UART3_ENABLE_INTERRUPT()   (UCA3IE |=  UCRXIE)

#define Uart3TxUntilIdle()         while (!(UCTXIFG & UCA3IFG))             //�ȴ���ǰ���ַ��������

#define UART3_RX_MAX_SIZE       BUF_SMALL_SIZE
//UART3�������ݻ�����
//UINT8 au8RxBuffer3[UART3_RX_MAX_SIZE];

//���ڽ����豸���ýṹ����
typedef struct
{
  UINT16 u16RxHeader;

  UINT16 u16RxTail;

  UINT16 u16RxDataLen;

  UINT8 *pu8RxBuf;

}_tsUartCb;
_tsUartCb sUart3Cb;

/*******************************************
�������ƣ�Uart3Init()
��    �ܣ�UART3��ʼ����
          ���ò�����Ϊ115200,8bit data��1bit stop��none verify
��    ������
����ֵ  ����
********************************************/
void Uart3Init(void)
{
    UART3_SEL |=  UART3_TXD + UART3_RXD;       // ѡ��˿���UARTͨ�Ŷ˿�
    UART3_DIR |=  UART3_TXD;
    UART3_DIR &= ~UART3_RXD;

    UCA3CTL1  =  UCSWRST;                          //״̬����λ
    UCA3CTL1 |=  UCSSEL1;                          // UCLK = ACLK
    UCA3BR0   =  0x24;                             // ������115200
    UCA3BR1   =  0x00;                             //
    UCA3MCTL  =  UCBRS_3 + UCBRF_0;                // Modulation
    UCA3CTL1 &= ~UCSWRST;                          // ��ʼ��UART״̬��
    UCA3IE   |=  UCRXIE;                           // ʹ��USART0�Ľ����ж�

    sUart3Cb.pu8RxBuf = OSMemGet(pMediumMem, &err);//au8RxBuffer3;              //��ʼ���������ṹ��
    sUart3Cb.u16RxHeader = 0;
    sUart3Cb.u16RxTail = 0;
    sUart3Cb.u16RxDataLen = 0;
}


/*******************************************
�������ƣ�Uart3SendByte()
��    �ܣ�ͨ��UART3����һ��byte

��    ������
����ֵ  ����
********************************************/
void Uart3SendByte(UINT8 u8Data)
{
   UCA3TXBUF = u8Data;
   Uart3TxUntilIdle();
}

/************************************************************************************
  �������ƣ�Uart3SendString()
  ��    �ܣ�ͨ��UART3�����ַ���

  ��    ����*pString
  ����ֵ  ����
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
  �������ƣ�Uart3SendBuffer
  �������ܣ�����buffer��u8Length��byte

  ������*pu8Buf
        u8Lenght   --  ���͸���
  ���أ���
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
  �������ƣ�Uart3EnableInterrupt
  �������ܣ�ʹ��Uart3�ж�

  ��������
  ���أ���
*********************************************************************************/
void Uart3EnableInterrupt(void)
{
  UART3_ENABLE_INTERRUPT();
}

/*********************************************************************************
  �������ƣ�Uart3EnableInterrupt
  �������ܣ��ر�Uart3�ж�

  ��������
  ���أ���
*********************************************************************************/
void Uart3DisableInterrupt(void)
{
  UART3_DISABLE_INTERRUPT();
}


/***************************************************************************
//�������ƣ�Uart3GetRxDataLength
//�����������Ӵ��ڻ�����ȡ�ý��ճ���
//
//��������
//���أ�u16DataLen
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
//�������ƣ�Uart3GetByte
//�����������Ӵ��ڻ�����ȡ�� һ������
//
//��������
//���أ�u16RxData
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
//�������ƣ�Uart3GetRxData
//�����������Ӵ��ڻ�����ȡ������
//
//��������
//���أ�UINT8
***************************************************************************/
UINT16 Uart3GetRxData(UINT8 *pu8Buffer, UINT16 u16DataLen)
{
  UINT16 u16Len;
  UINT16 u16HeaderToEndOfBuf;

  UART3_DISABLE_INTERRUPT();
  u16Len = sUart3Cb.u16RxDataLen;
  UART3_ENABLE_INTERRUPT();

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
  u16HeaderToEndOfBuf = UART3_RX_MAX_SIZE - sUart3Cb.u16RxHeader;

  //��֡ͷ��֡β���ȴ�����Ҫ�ĳ���
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
//�������ƣ�Uart0EmptyRxBuf
//������������ս��ջ�����
//
//��������
//���أ���
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
  �������ƣ�UART3_RXISR
  ��     �ܣ�UART3�Ľ����жϷ������������﻽��
          CPU��ʹ���˳��͹���ģʽ

  ��������
  ���أ���
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


