
#ifndef __UART3_H__
#define __UART3_H__

#include "global.h"
#include "DataType.h"


/***************************************************************************
�������ƣ�Uart3Init()
��    �ܣ�UART0��ʼ����
          ���ò�����Ϊ9600,8bit data��1bit stop��none verify
��    ������
����ֵ  ����
***************************************************************************/
extern void Uart3Init(void);

/*******************************************
�������ƣ�Uart3SendByte()
��    �ܣ�ͨ��UART1����һ��byte
��    ����u8Data
����ֵ  ����
********************************************/
extern void Uart3SendByte(UINT8 u8Data);


/*******************************************
�������ƣ�Uart3SendString()
��    �ܣ�ͨ��UART1�����ַ���
��    ����*pString
����ֵ  ����
********************************************/
extern void Uart3SendString(UINT8 * pString);

/********************************************************************************
  �������ƣ�Uart3SendBuffer
  �������ܣ�����buffer��u8Length��byte

  ������*pu8Buf
        u8Lenght   --  ���͸���
  ���أ���
*********************************************************************************/
extern void Uart3SendBuffer(UINT8 *pu8Buf, UINT8 u8Length);

/***************************************************************************
  �������ƣ�Uart3EnableInterrupt
  �������ܣ�ʹ��Uart�ж�

  ��������
  ���أ���
***************************************************************************/
extern void Uart3EnableInterrupt(void);

/***************************************************************************
  �������ƣ�Uart3DisableInterrupt
  �������ܣ��ر�Uart�ж�

  ��������
  ���أ���
***************************************************************************/
extern void Uart3DisableInterrupt(void);

/***************************************************************************
//�������ƣ�Uart3GetRxDataLength
//�����������Ӵ��ڻ�����ȡ�ý��ճ���
//
//��������
//���أ�u8DataLen
***************************************************************************/
extern UINT16 Uart3GetRxDataLength(void);

/***************************************************************************
//�������ƣ�Uart3GetByte
//�����������Ӵ��ڻ�����ȡ�� һ������
//
//��������
//���أ�u8RxData
***************************************************************************/
extern UINT8 Uart3GetByte(void);

/***************************************************************************
//�������ƣ�Uart3GetRxData
//�����������Ӵ��ڻ�����ȡ������
//
//��������
//���أ���ȡ���ݵĳ���
***************************************************************************/
extern UINT16 Uart3GetRxData(UINT8 *pu8Buffer, UINT16 u16DataLen);

/***************************************************************************
//�������ƣ�Uart3EmptyRxBuf
//������������ս��ջ�����
//
//��������
//���أ���
***************************************************************************/
extern void Uart3EmptyRxBuf(void);

#endif