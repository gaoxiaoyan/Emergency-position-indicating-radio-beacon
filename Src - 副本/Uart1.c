/******************************************************************************
*											
* Copyright (C) 2014, Sunic-ocean
* All Rights Reserved										
*
* �� �� ���� Uart1.C
* ��    ��������1�ײ�����, ��uart�ڹ�����GPS��λʹ�á�

* �������ڣ� 2015��4��16��16:58:36
* ��    �ߣ� Bob
* ��ǰ�汾�� V1.0

------------------------------------------------------
*******************************************************************************/
#include "msp430x54x.h"

#include "global.h"
#include "Uart1.h"
#include "Uart3.h"
#include "string.h"
#include "SystemCtl.h"
#include "OS_MEM.h"

//UART1 �궨��
#define UART1_DISABLE_INTERRUPT()  (UCA1IE &= ~UCRXIE)
#define UART1_ENABLE_INTERRUPT()   (UCA1IE |=  UCRXIE)

#define Uart1TxUntilIdle()         while (!(UCTXIFG & UCA1IFG))             //�ȴ���ǰ���ַ��������

#define UART1_RX_MAX_SIZE       BUF_LARGE_SIZE
//UART1�������ݻ�����
//UINT8 au8RxBuffer2[UART1_RX_MAX_SIZE];

//���ڽ����豸���ýṹ����
typedef struct
{
  UINT16 u16RxHeader;

  UINT16 u16RxTail;

  UINT16 u16RxDataLen;

  UINT8 *pu8RxBuf;

}_tsUartCb;
_tsUartCb sUart1Cb;

/*******************************************
�������ƣ�Uart1Init()
��    �ܣ�UART1��ʼ����
          ���ò�����Ϊ9600,8bit data��1bit stop��none verify
��    ������
����ֵ  ����
********************************************/
void Uart1Init(void)
{
    UART1_SEL |=  UART1_TXD + UART1_RXD;       // ѡ��˿���UARTͨ�Ŷ˿�
    UART1_DIR |=  UART1_TXD;
    UART1_DIR &= ~UART1_RXD;
    
    UCA1CTL1  =  UCSWRST;                          //״̬����λ
    UCA1CTL1 |=  UCSSEL1;                          // UCLK = ACLK
    UCA1BR0   =  0x24;                             // ������9600
    UCA1BR1   =  0x00;                             //
    UCA1MCTL  =  UCBRS_7 + UCBRF_0;                // Modulation
    UCA1CTL1 &= ~UCSWRST;                          // ��ʼ��UART״̬��
    UCA1IE   |=  UCRXIE;                           // ʹ��USART0�Ľ����ж�

    sUart1Cb.pu8RxBuf = OSMemGet(pLargeMem, &err);//au8RxBuffer2;              //��ʼ���������ṹ��
    sUart1Cb.u16RxHeader = 0;
    sUart1Cb.u16RxTail = 0;
    sUart1Cb.u16RxDataLen = 0;
}


/*******************************************
�������ƣ�Uart1SendByte()
��    �ܣ�ͨ��UART1����һ��byte

��    ������
����ֵ  ����
********************************************/
void Uart1SendByte(UINT8 u8Data)
{
   UCA1TXBUF = u8Data;
   Uart1TxUntilIdle();
}

/************************************************************************************
  �������ƣ�Uart1SendString()
  ��    �ܣ�ͨ��UART1�����ַ���

  ��    ����*pString
  ����ֵ  ����
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
  �������ƣ�Uart1SendBuffer
  �������ܣ�����buffer��u8Length��byte

  ������*pu8Buf
        u8Lenght   --  ���͸���
  ���أ���
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
  �������ƣ�Uart1EnableInterrupt
  �������ܣ�ʹ��Uart3�ж�

  ��������
  ���أ���
*********************************************************************************/
void Uart1EnableInterrupt(void)
{
  UART1_ENABLE_INTERRUPT();
}

/*********************************************************************************
  �������ƣ�Uart1EnableInterrupt
  �������ܣ��ر�Uart1�ж�

  ��������
  ���أ���
*********************************************************************************/
void Uart1DisableInterrupt(void)
{
  UART1_DISABLE_INTERRUPT();
}


/***************************************************************************
//�������ƣ�Uart1GetRxDataLength
//�����������Ӵ��ڻ�����ȡ�ý��ճ���
//
//��������
//���أ�u16DataLen
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
//�������ƣ�Uart1GetByte
//�����������Ӵ��ڻ�����ȡ�� һ������
//
//��������
//���أ�u16RxData
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
//�������ƣ�Uart1InsertByte
//�����������򴮿ڻ�������ͷ����һ��byte
//
//������u8Data -- ��������
//
//���أ���
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
//�������ƣ�Uart3GetRxData
//�����������Ӵ��ڻ�����ȡ������
//
//��������
//���أ�UINT8
***************************************************************************/
UINT16 Uart1GetRxData(UINT8 *pu8Buffer, UINT16 u16DataLen)
{
  UINT16 u16Len;
  UINT16 u16HeaderToEndOfBuf;
	
  UART1_DISABLE_INTERRUPT();
  u16Len = sUart1Cb.u16RxDataLen;
  UART1_ENABLE_INTERRUPT();
	
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
  u16HeaderToEndOfBuf = UART1_RX_MAX_SIZE - sUart1Cb.u16RxHeader;
	
  //��֡ͷ��֡β���ȴ�����Ҫ�ĳ���
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
//�������ƣ�Uart0EmptyRxBuf
//������������ս��ջ�����
//
//��������
//���أ���
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
  �������ƣ�UART1_RXISR
  ��     �ܣ�UART1�Ľ����жϷ������������﻽��
          CPU��ʹ���˳��͹���ģʽ

  ��������
  ���أ���
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