
/******************************************************************************************************
 *											
 * Copyright (C) 2014, Sunic-ocean
 * All Rights Reserved										
 *
 * �� �� ���� BatteryDetect.c
 * ��       ����

 * �������ڣ� 2014��9��5�� 09:45:35
 * ��    �ߣ� Bob
 * ��ǰ�汾�� V1.00
 *******************************************************************************************************/

#ifdef MSP430F149
#include "msp430x14x.h"
#else
#include "msp430x54x.h"
#endif

#include "DataType.h"
#include "global.h"
#include "BatteryDetect.h"
#include "Uart.h"
#include "Timer.h"
#include "Display.h"
#include "stdio.h"

#define ADC_MAX     8
UINT16 au16AdcBuffer[ADC_MAX];
UINT16 u16AdcResules = 0;
BOOL bComplete = FALSE;

/*******************************************************************************
 * �������ƣ�AdcInit
 * �������ܣ���ʼ��ADC

 * ����: ��
 * ����: ��
********************************************************************************/
void Adc_Init(void)
{
  P6SEL |= BIT3;                            // ��P6.1��ΪADC�ļ��ͨ��
//  P6DIR &= BIT1;

  ADC12CTL0 = ADC12MSC + ADC12SHT0_15;                 // ���ò���ʱ��
  ADC12CTL1 = ADC12SHP + ADC12CONSEQ_1;                 // ʹ�ò�����ʱ��
  ADC12IE = BIT3;                           // ʹ���ж�
  ADC12MCTL0 = ADC12INCH_3;
  
  //ADC��ⶨʱ��
  asTimer[TIMER_ADC].u16LifeTime = TIMER_TICK * 4;
  asTimer[TIMER_ADC].u16LifeOldTime = TIMER_TICK * 4;
  asTimer[TIMER_ADC].bEnable = TRUE;
}

/*******************************************************************************
 * �������ƣ�BatteryDetect
 * �������ܣ���ص����������

 * ����: ��
 * ����: ��
********************************************************************************/
void BatteryDetect(void)
{
  UINT8 i;
  UINT8 u8Bat[15] = {"��ǰ����Ϊ000%!"};
  
  //ADC��ʱʱ�䵽������һ��ADC
  if(asTimer[TIMER_ADC].u16LifeTime == 0)
  {
    asTimer[TIMER_ADC].u16LifeTime = asTimer[TIMER_ADC].u16LifeOldTime;
    ADC_START_DETECT();
  }
  
  //ADC�����ɣ��������ݴ�����������İٷֱ�
  if(bComplete == TRUE)
  {
    bComplete = FALSE;

    for(i=0; i<ADC_MAX; i++)
    {
      u16AdcResules += au16AdcBuffer[i];
    }

    u16AdcResules >>= 3;      //ȡ8������ƽ��ֵ

    //��ص�ѹΪ7.4v������1/3��ѹ ADC�ɼ�������ѹ��3.6Vʱ��������DTU��ѹҪ��
    //��ѹ��Ϊ1.2V������ʱΪ�˼��㽫��ѹ������10��������
    sDisplay.u8BatElec = ((UINT32)u16AdcResules * 33) >> 12; //ADC����

    if(sDisplay.u8BatElec > 12)
    {
        sDisplay.u8BatElec -= 12;
        if(sDisplay.u8BatElec > 9)
        {
            sDisplay.u8BatElec = 9;
        }
    }
    else
    {
      sDisplay.u8BatElec = 0;
    }
    
    sSendData.u8Battery = sDisplay.u8BatElec;
    
    //������Ϣ
    if(g_bDebug == TRUE)
    {
      sprintf(&u8Bat[10], "%2d", sDisplay.u8BatElec);
      UartSendString(UART2, &u8Bat[0]);
      UartSendString(UART2, " \r\n");
    }
  }
  
}

/*******************************************************************************
 * �������ƣ�ADC12_ISR
 * �������ܣ�ADC�жϴ���������β���ȡƽ��ֵ

 * ����: ��
 * ����: ��
********************************************************************************/
#pragma vector = ADC12_VECTOR
__interrupt void ADC_ISR(void)
{
  static UINT8 u8Index=0;

  au16AdcBuffer[u8Index++] = ADC12MEM0;
  if(u8Index == ADC_MAX)
  {
    bComplete = TRUE;
    ADC_STOP_DETECT();  //�ر�ADC
    u8Index = 0;
    u16AdcResules = 0;
  }
}







