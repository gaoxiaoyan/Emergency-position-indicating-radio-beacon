
/******************************************************************************
*											
* Copyright (C) 2014, Sunic-ocean
* All Rights Reserved										
*
* �� �� ���� Timer.C
* ��    ������ʱ���ײ�����

* �������ڣ� 2015��4��10��16:25:16
* ��    �ߣ� Bob
* ��ǰ�汾�� V1.0

------------------------------------------------------
*******************************************************************************/
#include "msp430x54x.h"

#include "DataType.h"
#include "global.h"
#include "Timer.h"
#include "SoftWatchDog.h"

#define TIMER_TICK          100    //���ö�ʱ��A0���ж�Ƶ��Ϊ100hz

#define INIT_VAL0           (32768/TIMER_TICK - 1)   //��������������ʱ����ֵ
#define INIT_VAL1           (32768 - 1)              //����ģʽ�¶�ʱ������Ϊ1s��ֵ

//��ʱ���ж�ʹ��
#define ITMERA_INTERRUPT_DISABLE()      TA0CCTL0 &= ~CCIE //TA0CTL = MC_0
#define TIMERA_INTERRUPT_ENABLE()       TA0CCTL0 |=  CCIE //TA0CTL = TASSEL_1 + MC_1


/******************************************************************************
 * �������ƣ�TimerA_Init
 * ������TIMER A init
 *
 * ��������
 * ���أ���
 *****************************************************************************/
void TimerA_Init(void)
{
  SwdInit();
  
  TA0CCR0  = INIT_VAL0;                         //�ж�����
  TA0CCTL0 = CCIE;                              //ʹ���ж�
  TA0CTL   = TASSEL_1 + MC_1;                   // ACLK, upmode
}

/******************************************************************************
 * �������ƣ�TimerInterruptEnable
 * ������ʹ��TIMER A0�ж�
 *
 * ��������
 * ���أ���
 *****************************************************************************/
void TimerInterruptEnable(void)
{
  TIMERA_INTERRUPT_ENABLE();
}

/******************************************************************************
 * �������ƣ�TimerInterruptDisable
 * �������ر�TIMER A0�ж�
 *
 * ��������
 * ���أ���
 *****************************************************************************/
void TimerInterruptDisable(void)
{
  ITMERA_INTERRUPT_DISABLE();
}

/******************************************************************************
 * �������ƣ�TimerA0_ISR
 * ������TIMER A0 �жϴ�����
 *
 * ��������
 * ���أ���
 *****************************************************************************/
#pragma vector = TIMER0_A0_VECTOR
__interrupt void TimerA_ISR(void)
{
  SwdRun();
  
  g_u8TimerDelay++;
  g_u16TimerDelay++;
}

