
#ifndef __UART1_H__
#define __UART1_H__

#include "global.h"
#include "DataType.h"


/***************************************************************************
�������ƣ�Uart1Init()
��    �ܣ�UART1��ʼ����
          ���ò�����Ϊ115200,8bit data��1bit stop��none verify
��    ������
����ֵ  ����
***************************************************************************/
extern void Uart1Init(void);

/*******************************************
�������ƣ�Uart1SendByte()
��    �ܣ�ͨ��UART0����һ��byte
��    ����u8Data
����ֵ  ����
********************************************/
extern void Uart1SendByte(UINT8 u8Data);


/*******************************************
�������ƣ�Uart1SendString()
��    �ܣ�ͨ��UART0�����ַ���
��    ����*pString
����ֵ  ����
********************************************/
extern void Uart1SendString(const UINT8 * pString);

/********************************************************************************
  �������ƣ�Uart1SendBuffer
  �������ܣ�����buffer��u8Length��byte

  ������*pu8Buf
        u8Lenght   --  ���͸���
  ���أ���
*********************************************************************************/
extern void Uart1SendBuffer(const UINT8 *pu8Buf, UINT8 u8Length);

/***************************************************************************
  �������ƣ�Uart1EnableInterrupt
  �������ܣ�ʹ��Uart�ж�

  ��������
  ���أ���
***************************************************************************/
extern void Uart1EnableInterrupt(void);

/***************************************************************************
  �������ƣ�Uart1DisableInterrupt
  �������ܣ��ر�Uart�ж�

  ��������
  ���أ���
***************************************************************************/
extern void Uart1DisableInterrupt(void);

/***************************************************************************
//�������ƣ�Uart1GetRxDataLength
//�����������Ӵ��ڻ�����ȡ�ý��ճ���
//
//��������
//���أ�u8DataLen
***************************************************************************/
extern UINT16 Uart1GetRxDataLength(void);

/***************************************************************************
//�������ƣ�Uart1GetByte
//�����������Ӵ��ڻ�����ȡ�� һ������
//
//��������
//���أ�u8RxData
***************************************************************************/
extern UINT8 Uart1GetByte(void);

/***************************************************************************
//�������ƣ�Uart1GetRxData
//�����������Ӵ��ڻ�����ȡ������
//
//��������
//���أ���ȡ���ݵĳ���
***************************************************************************/
extern UINT16 Uart1GetRxData(UINT8 *pu8Buffer, UINT16 u16DataLen);

/***************************************************************************
//�������ƣ�Uart1EmptyRxBuf
//������������ս��ջ�����
//
//��������
//���أ���
***************************************************************************/
extern void Uart1EmptyRxBuf(void);


#endif