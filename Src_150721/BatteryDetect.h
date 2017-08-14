

#ifndef __BATDETECT_H__
#define __BATDETECT_H__

#include "DataType.h"


//ADCʹ�ܿ���
#define ADC_START_DETECT()    ADC12CTL0 |= ADC12ENC + ADC12SC + ADC12ON
#define ADC_STOP_DETECT()     ADC12CTL0 &= ~(ADC12ENC + ADC12SC + ADC12ON)

/*******************************************************************************
 * �������ƣ�Adc_Init
 * �������ܣ���ʼ��ADC

 * ����: ��
 * ����: ��
********************************************************************************/
extern void Adc_Init(void);

/*******************************************************************************
 * �������ƣ�BatteryDetect
 * �������ܣ���ص����������

 * ����: ��
 * ����: ��
********************************************************************************/
extern void BatteryDetect(void);

/*******************************************************************************
 * �������ƣ�AdcDetect
 * �������ܣ���ص����������

 * ����: bType -- 1:�������еĶ�ȡ��0����������ȡ
 * ����: �����ٷֱȣ�1~10
********************************************************************************/
extern UINT8 AdcDetect(BOOL bType);


#endif