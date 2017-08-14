
/******************************************************************************************************
 *
 * Copyright (C) 2014, Sunic-ocean
 * All Rights Reserved
 *
 * �� �� ���� BatteryDetect.c
 * ��       ����
 *
 * �޸����ڣ� 2015��4��29��09:16:28
 * ��    �ߣ� Bob
 * ��ǰ�汾�� V1.01
 * �޸����ݣ�
 * ?////////////////////////////////////////////////////////////////////////////////////////
 * �������ڣ� 2014��9��5�� 09:45:35
 * ��    �ߣ� Bob
 * ��ǰ�汾�� V1.00
 *******************************************************************************************************/
#include "msp430x54x.h"

#include "DataType.h"
#include "global.h"
#include "BatteryDetect.h"
#include "Uart3.h"
#include "SoftTimer.h"
#include "stdio.h"
#include "Communication.h"

//�����ѹ����Ϊ6v
#define REF_VOL         (UINT8)(3.3 * 10.0)       //��׼��ѹ3.3V
#define VOLTAGE_MIN     6.2       //��С��ѹ6.5V
#define VOLTAGE_MAX     8.4       //��ߵ�ѹ8.4V
#define DIV             (FLOAT32)(13/3)    //��ѹϵ��
#define BAT_VOL_MIN     (FLOAT32)((VOLTAGE_MIN * 10.0) / DIV)
#define BAT_VOL_MAX     (FLOAT32)((VOLTAGE_MAX * 10.0) / DIV)
#define AMP_PARA_2      (FLOAT32)(10.0 / (BAT_VOL_MAX - BAT_VOL_MIN))  //�����Ŵ�ϵ��

#define ADC_MAX     16
UINT16 au16AdcBuffer[ADC_MAX];
UINT16 u16AdcResules = 0;
BOOL bComplete = FALSE;
SINT8 s8BatDog=0;

/*******************************************************************************
 * �������ƣ�AdcInit
 * �������ܣ���ʼ��ADC

 * ����: ��
 * ����: ��
********************************************************************************/
void Adc_Init(void)
{
  P6SEL |= BIT3;                            // ��P6.1��ΪADC�ļ��ͨ��

  AdcCtlDisable();
  AdcCtlDir |= AdcCtl;

  ADC12CTL0  = ADC12MSC + ADC12SHT0_15;                 // ���ò���ʱ��
  ADC12CTL1  = ADC12SHP + ADC12CONSEQ_1;                 // ʹ�ò�����ʱ��
  ADC12IE   |= BIT0;                           // ʹ���ж�
  ADC12MCTL0 = ADC12INCH_3;

  //ADC��ⶨʱ��
  s8BatDog = SwdGet();
  SwdSetName(s8BatDog, "BatteryTimer");
  SwdSetLifeTime(s8BatDog, 2000); //���ò���ʱ��Ϊ2��
  SwdEnable(s8BatDog);
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

  //ADC��ʱʱ�䵽������һ��ADC
  if(SwdGetLifeTime(s8BatDog) == 0)
  {
    SwdReset(s8BatDog);
    AdcCtlEnable();
    //����һ��AD���
    ADC_START_DETECT();
  }

  //ADC�����ɣ��������ݴ�����������İٷֱ�
  if(bComplete == TRUE)
  {
    bComplete = FALSE;

    AdcCtlDisable();

    for(i=0; i<ADC_MAX; i++)
    {
      u16AdcResules += au16AdcBuffer[i];
    }

    u16AdcResules >>= 4;      //ȡ8������ƽ��ֵ

    //��ص�ѹΪ8.4v������3/13˥����Ȼ�������ȫ���̷Ŵ�Ȼ������������룬�õ�ʣ������İٷֱ�
    sSendInfo.u8Battery = (UINT8)((FLOAT32)(((FLOAT32)u16AdcResules * REF_VOL) / 4096) + 0.5); //ADC����
    sSendInfo.u8Battery = (UINT8)((FLOAT32)(sSendInfo.u8Battery - BAT_VOL_MIN)*AMP_PARA_2+0.5);

    if(sSendInfo.u8Battery > 9)
    {
      sSendInfo.u8Battery = 9;
    }
    else
    {
      sSendInfo.u8Battery = sSendInfo.u8Battery;
    }

    if(sSendInfo.u8Battery > 6)
    {
        LowPowerRedDisable();
        LowPowerBlueDisable();
        LowPowerGreenEnable();
    }
    else
    {
        if(sSendInfo.u8Battery > 3)
        {
            LowPowerRedDisable();
            LowPowerBlueEnable();
            LowPowerGreenDisable();
        }
        else
        {
            LowPowerRedEnable();
            LowPowerBlueDisable();
            LowPowerGreenDisable();
        }
    }
  }
}

/*******************************************************************************
 * �������ƣ�AdcDetect
 * �������ܣ���ص����������

 * ����: bType -- 1:�������еĶ�ȡ��0����������ȡ
 * ����: �����ٷֱȣ�1~10
********************************************************************************/
UINT8 AdcDetect(BOOL bType)
{
  UINT8 buf[2], u8Bat;
  UINT8 i;

  //ADC�����ɣ��������ݴ�����������İٷֱ�
  ADC_START_DETECT();
  while(bComplete != TRUE);
  bComplete = FALSE;

  for(i=0; i<ADC_MAX; i++)
  {
    u16AdcResules += au16AdcBuffer[i];
  }

  u16AdcResules >>= 4;      //ȡ8������ƽ��ֵ

  //��ص�ѹΪ8.4v������3/13˥����Ȼ�������ȫ���̷Ŵ�Ȼ������������룬�õ�ʣ������İٷֱ�
  u8Bat = (UINT8)((UINT32)((FLOAT32)u16AdcResules * REF_VOL + 0.5) >> 12); //ADC����
  u8Bat = (UINT8)((FLOAT32)(u8Bat - BAT_VOL_MIN)*AMP_PARA_2+0.5);

  if(u8Bat > 9)
    u8Bat = 10;
  else
    u8Bat += 1;

  if(bType == TRUE)
  {
    Uart3SendString("# The Current Remaining Power Is: ");
    buf[0] = u8Bat/10 + 0x30;
    buf[1] = u8Bat%10 + 0x30;
    Uart3SendBuffer(&buf[0], 2);
    Uart3SendString("0%! \r\n");

    if(u8Bat < 3)
    {
      Uart3SendString("# Power Less Than 30%, Please Recharge In Time��\r\n");
    }

    return 0xaa;
  }
  else
  {
    if(u8Bat > 7)
    {
      return 0x04;
    }
    else if((u8Bat > 5) && (u8Bat < 8))
    {
      return 0x03;
    }
    else if((u8Bat > 3) && (u8Bat < 6))
    {
      return 0x02;
    }
    else if((u8Bat > 1) && (u8Bat < 4))
    {
      return 0x01;
    }
    else
    {
      return 0x00;
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







