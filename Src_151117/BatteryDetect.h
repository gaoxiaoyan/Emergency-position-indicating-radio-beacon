

#ifndef __BATDETECT_H__
#define __BATDETECT_H__

#include "DataType.h"


//ADC使能控制
#define ADC_START_DETECT()    ADC12CTL0 |= ADC12ENC + ADC12SC + ADC12ON
#define ADC_STOP_DETECT()     ADC12CTL0 &= ~(ADC12ENC + ADC12SC + ADC12ON)

//ADC采集开关控制
#define AdcCtl                    BIT2
#define AdcCtlOut                 P6OUT
#define AdcCtlDir                 P6DIR
#define AdcCtlEnable()            AdcCtlOut |=  AdcCtl
#define AdcCtlDisable()           AdcCtlOut &= ~AdcCtl

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

/*******************************************************************************
 * 函数名称：AdcDetect
 * 函数功能：电池电量检测任务

 * 参数: bType -- 1:正常运行的读取，0：开机检查读取
 * 返回: 电量百分比，1~10
********************************************************************************/
extern UINT8 AdcDetect(BOOL bType);


#endif