
#ifndef __MAIN_H__
#define __MAIN_H__

#include "msp430x54x.h"
#include "DataType.h"
#include "OS_MEM.h"

/*-------------------------------------------------------------------------*
 *    宏定义                                                               *
 *-------------------------------------------------------------------------*/
//#define CPU_CLOCK		1000000UL

//打开调试开关使能
//#define DEBUG              1u

//工程版本信息
#define VERSION              100

#define CPU_ENTER_CRITICAL()     _DINT()     //关闭全局中断控制位,进入临界区
#define CPU_EXIT_CRITICAL()      _EINT()     //打开全局中断控制位,退出临界区

//调试信息开关
extern BOOL g_bDebug;

//AT24C02器件从地址
#define EEPROM_WRITE_ADDR    0xa0
#define EEPROM_READ_ADDR     0xa1

//RTC S35390A器件地址
#define RTC_WRITE_ADDR       0x60
#define RTC_READ_ADDR        0x61

#define OpenWatchDog()        WDTCTL  &=  ~WDTHOLD
#define SetWatchDog()         WDTCTL  =  0x5A2B   //设置为16s
#define FreeWatchDog()        WDTCTL  =  0x5A2B   //设置为16s
#define StopWatchDog()        WDTCTL  =  WDTPW + WDTHOLD   //关闭看门狗

//ADC使能控制
#define ADC_START_DETECT()    ADC12CTL0 |= ADC12ENC + ADC12SC + ADC12ON
#define ADC_STOP_DETECT()     ADC12CTL0 &= ~(ADC12ENC + ADC12SC + ADC12ON)

//程序运行指示灯
#define SystemRun             BIT4
#define SystemRunOut          P2OUT
#define SystemRunDir          P2DIR
#define SystemRunEnable()     SystemRunOut |=  SystemRun
#define SystemRunDisable()    SystemRunOut &= ~SystemRun

//低压指示灯
#define LowPowerLed           BIT3
#define LowPowerLedOut        P2OUT
#define LowPowerLedDir        P2DIR
#define LowPowerLedEnable()   LowPowerLedOut |=  LowPowerLed
#define LowPowerLedDisable()  LowPowerLedOut &= ~LowPowerLed

//围油栏指示灯
#define WylLed                BIT2
#define WylLedOut             P2OUT
#define WylLedDir             P2DIR
#define WylEnable()           WylLedOut |=  WylLed
#define WylLedDisable()       WylLedOut &= ~WylLed

//收油机指示灯
#define SyjLed                BIT1
#define SyjLedOut             P2OUT
#define SyjLedDir             P2DIR
#define SyjLedEnable()        SyjLedOut |=  SyjLed
#define SyjLedDisable()       SyjLedOut &= ~SyjLed

//串口0接口定义. 连接CDMA模块
#define UART0_SEL             P3SEL
#define UART0_DIR             P3DIR
#define UART0_TXD             BIT4
#define UART0_RXD             BIT5
//串口1接口定义. 连接BD模块
#define UART1_SEL             P5SEL
#define UART1_DIR             P5DIR
#define UART1_TXD             BIT6
#define UART1_RXD             BIT7
//串口2接口定义. 连接GPS  
#define UART2_SEL             P9SEL
#define UART2_DIR             P9DIR
#define UART2_TXD             BIT4
#define UART2_RXD             BIT5
//串口3接口定义. 连接USB
#define UART3_SEL             P10SEL
#define UART3_DIR             P10DIR
#define UART3_TXD             BIT4
#define UART3_RXD             BIT5

//开关机按键接口定义
#define KeyPower              BIT5
#define KeyPowerDir           P1DIR
#define KeyPowerIn            P1IN
#define KeyPowerIES           P1IES
#define KeyPowerIE            P1IE
#define KeyPowerIntFlag       P1IFG
#define KeyPowerCheck         (KeyPowerIn & KeyPower)
#define KeyPowerIntCheck      (KeyPowerIntFlag & KeyPower)
#define KeyPowerIeEnable()    (KeyPowerIE | KeyPower)
#define KeyPowerIeDisable()   (KeyPowerIE & (~KeyPower))
#define KeyPowerIesEdge()      (KeyPowerIES | KeyPower)

//RTC中断接口
#define RtcInt                BIT6
#define RtcIntDir             P1DIR
#define RtcIntIn              P1IN
#define RtcIntFlag            P1IFG
#define RtcIntIES             P1IES
#define RtcIntIE              P1IE
#define RtcIntCheck           (RtcIntFlag & RtcInt)
#define RtcIntIeEnable()      (RtcIntIE | RtcInt)
#define RtcIntIeDisable()     (RtcIntIE & (~RtcInt))
#define RtcIntIesEdge()       (RtcIntIES & (~RtcInt))

//CDMA_RI中断接口
#define CdmaRi                BIT7
#define CdmaRiDir             P1DIR
#define CdmaRiIn              P1IN
#define CdmaRiIntFlag         P1IFG 
#define CdmaRiIES             P1IES
#define CdmaRiIE              P1IE
#define CdmaRiIntCheck        (CdmaRiIntFlag & CdmaRi)
#define CdmaRiIntEnable()     (CdmaRiIE | CdmaRi)
#define CdmaRiIntDisable()    (CdmaRiIE & (~CdmaRi))
#define CdmaRiIesEdge()       (CdmaRiIES | CdmaRi)

//GPS电源控制
#define GpsPower              BIT0
#define GpsPowerOut           P10OUT
#define GpsPowerDir           P10DIR
#define GpsPowerOn()          GpsPowerOut |=  GpsPower
#define GpsPowerOff()         GpsPowerOut &= ~GpsPower

//CDMA电源控制
#define CdmaPower              BIT1
#define CdmaPowerOut           P10OUT
#define CdmaPowerDir           P10DIR
#define CdmaPowerOn()          CdmaPowerOut |=  CdmaPower
#define CdmaPowerOff()         CdmaPowerOut &= ~CdmaPower

//BD电源控制
#define BeiDouPower             BIT2
#define BeiDouPowerOut          P10OUT
#define BeiDouPowerDir          P10DIR
#define BeiDouPowerOn()         BeiDouPowerOut |=  BeiDouPower
#define BeiDouPowerOff()        BeiDouPowerOut &= ~BeiDouPower

//系统自锁电源
#define SystemPower           BIT3
#define SystemPowerPort       P10OUT
#define SystemPowerDir        P10DIR
#define SystemPowerOn()       SystemPowerPort |=  SystemPower
#define SystemPowerOff()      SystemPowerPort &= ~SystemPower

//I2C端口定义
#define I2cSda                BIT5
#define I2cSdaOut             P7OUT
#define I2cSdaIn              P7IN
#define I2cSdaDir             P7DIR

#define I2cScl                BIT4
#define I2cSclOut             P7OUT
#define I2cSclDir             P7DIR

#define EepromWp             BIT0
#define EepromWpPort         P10OUT
#define EepromWpDir          P10DIR
#define EnableEeprom()       EepromWpPort &= ~EepromWp //使能EEPROM
#define DisableEeprom()      EepromWpPort |=  EepromWp //失能EEPROM

volatile extern UINT8 g_u8TimerDelay;
volatile extern UINT16 g_u16TimerDelay;

typedef struct
{
  UINT8 Year;
  UINT8 Month;
  UINT8 Date;
  UINT8 Week;
  UINT8 Hour;
  UINT8 Minute;
  UINT8 Second;

}_tsTimeCb;

// 时间戳，只记录当天的秒数，用于定时发送信息间隔
extern volatile UINT32  g_u32TimeStamp;

#endif