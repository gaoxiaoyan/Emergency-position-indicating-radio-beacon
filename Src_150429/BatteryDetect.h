

#ifndef __BATDETECT_H__
#define __BATDETECT_H__


//ADC使能控制
#define ADC_START_DETECT()    ADC12CTL0 |= ADC12ENC + ADC12SC + ADC12ON
#define ADC_STOP_DETECT()     ADC12CTL0 &= ~(ADC12ENC + ADC12SC + ADC12ON)

/*******************************************************************************
 * 函数名称：Adc_Init
 * 函数功能：初始化ADC

 * 参数: 无
 * 返回: 无
********************************************************************************/
extern void Adc_Init(void);

/*******************************************************************************
 * 函数名称：BatteryDetect
 * 函数功能：电池电量检测任务

 * 参数: 无
 * 返回: 无
********************************************************************************/
extern void BatteryDetect(void);


#endif