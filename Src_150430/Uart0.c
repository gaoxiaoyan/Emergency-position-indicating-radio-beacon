/******************************************************************************
*											
* Copyright (C) 2014, Sunic-ocean
* All Rights Reserved										
*
* �� �� ���� Uart0.C
* ��    ��������0�ײ�����, ��uart�ڹ�CDMAģ�顣

* �������ڣ� 2015��4��16��16:58:36
* ��    �ߣ� Bob
* ��ǰ�汾�� V1.0

------------------------------------------------------
*******************************************************************************/
#include "msp430x54x.h"

#include "global.h"
#include "Uart0.h"
#include "Uart3.h"
#include "string.h"
#include "SystemCtl.h"
#include "OS_MEM.h"

//UART0 �궨��
#define UART0_DISABLE_INTERRUPT()  (UCA0IE &= ~UCRXIE)
#define UART0_ENABLE_INTERRUPT()   (UCA0IE |=  UCRXIE)

#define Uart0TxUntilIdle()         while (!(UCTXIFG & UCA0IFG))             //�ȴ���ǰ���ַ��������

#define UART0_RX_MAX_SIZE       BUF_LARGE_SIZE
//UART2�������ݻ�����
//UINT8 au8RxBuffer2[UART2_RX_MAX_SIZE];

//���ڽ����豸���ýṹ����
typedef struct
{
  UINT16 u16RxHeader;

  UINT16 u16RxTail;

  UINT16 u16RxDataLen;

  UINT8 *pu8RxBuf;

}_tsUartCb;
_tsUartCb sUart0Cb;

/*******************************************
�������ƣ�Uart0Init()
��    �ܣ�UART0��ʼ����
          ���ò�����Ϊ9600,8bit data��1bit stop��none verify
��    ������
����ֵ  ����
********************************************/
void Uart0Init(void)
{
    UART0_SEL |=  UART0_TXD + UART0_RXD;       // ѡ��˿���UARTͨ�Ŷ˿�
    UART0_DIR |=  UART0_TXD;
    UART0_DIR &= ~UART0_RXD;
    
    UCA0CTL1  =  UCSWRST;                          //״̬����λ
    UCA0CTL1 |=  UCSSEL1;                          // UCLK = ACLK
    UCA0BR0   =  0x24;                             // ������115200
    UCA0BR1   =  0x00;                             //
    UCA0MCTL  =  UCBRS_7 + UCBRF_0;                // Modulation
    UCA0CTL1 &= ~UCSWRST;                          // ��ʼ��UART״̬��
    UCA0IE   |=  UCRXIE;                           // ʹ��USART0�Ľ����ж�

    sUart0Cb.pu8RxBuf = OSMemGet(pLargeMem, &err);//au8RxBuffer0;              //��ʼ���������ṹ��
    sUart0Cb.u16RxHeader = 0;
    sUart0Cb.u16RxTail = 0;
    sUart0Cb.u16RxDataLen = 0;
}


/*******************************************
�������ƣ�Uart0SendByte()
��    �ܣ�ͨ��UART3����һ��byte

��    ������
����ֵ  ����
********************************************/
void Uart0SendByte(UINT8 u8Data)
{
   UCA0TXBUF = u8Data;
   Uart0TxUntilIdle();
}

/************************************************************************************
  �������ƣ�Uart0SendString()
  ��    �ܣ�ͨ��UART0�����ַ���

  ��    ����*pString
  ����ֵ  ����
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
  �������ƣ�Uart0SendBuffer
  �������ܣ�����buffer��u8Length��byte

  ������*pu8Buf
        u8Lenght   --  ���͸���
  ���أ���
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
  �������ƣ�Uart0EnableInterrupt
  �������ܣ�ʹ��Uart3�ж�

  ��������
  ���أ���
*********************************************************************************/
void Uart0EnableInterrupt(void)
{
  UART0_ENABLE_INTERRUPT();
}

/*********************************************************************************
  �������ƣ�Uart0EnableInterrupt
  �������ܣ��ر�Uart0�ж�

  ��������
  ���أ���
*********************************************************************************/
void Uart0DisableInterrupt(void)
{
  UART0_DISABLE_INTERRUPT();
}


/***************************************************************************
//�������ƣ�Uart0GetRxDataLength
//�����������Ӵ��ڻ�����ȡ�ý��ճ���
//
//��������
//���أ�u16DataLen
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
//�������ƣ�Uart0GetByte
//�����������Ӵ��ڻ�����ȡ�� һ������
//
//��������
//���أ�u16RxData
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
//�������ƣ�Uart0GetRxData
//�����������Ӵ��ڻ�����ȡ������
//
//��������
//���أ�UINT8
***************************************************************************/
UINT16 Uart0GetRxData(UINT8 *pu8Buffer, UINT16 u16DataLen)
{
  UINT16 u16Len;
  UINT16 u16HeaderToEndOfBuf;
	
  UART0_DISABLE_INTERRUPT();
  u16Len = sUart0Cb.u16RxDataLen;
  UART0_ENABLE_INTERRUPT();
	
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
  u16HeaderToEndOfBuf = UART0_RX_MAX_SIZE - sUart0Cb.u16RxHeader;
	
  //��֡ͷ��֡β���ȴ�����Ҫ�ĳ���
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
//�������ƣ�Uart0EmptyRxBuf
//������������ս��ջ�����
//
//��������
//���أ���
***************************************************************************/
void Uart0EmptyRxBuf(void)
{
  UART0_DISABLE_INTERRUPT();
  sUart0Cb.u16RxDataLen = 0;
  sUart0Cb.u16RxHeader = sUart0Cb.u16RxTail;
  UART0_ENABLE_INTERRUPT();
}

/****************************************************************************
  �������ƣ�UART0_RXISR
  ��     �ܣ�UART0�Ľ����жϷ������������﻽��
          CPU��ʹ���˳��͹���ģʽ

  ��������
  ���أ���
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