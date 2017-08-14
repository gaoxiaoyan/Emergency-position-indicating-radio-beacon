
#ifndef __MAIN_H__
#define __MAIN_H__

#include "msp430x54x.h"
#include "DataType.h"
#include "OS_MEM.h"

/*-------------------------------------------------------------------------*
 *    宏定义                                                               *
 *-------------------------------------------------------------------------*/
//#define CPU_CLOCK		1000000UL

#ifdef  MAIN_GLOBAL
#define MAIN_EXT
#else
#define MAIN_EXT extern
#endif

//打开调试开关使能
//#define DEBUG                   1u

//工程版本信息
#define VERSION                   100      /* 系统版本号                    */

//条件编译，剪裁功能
#define KEY_SCAN_EN               0        /* 使能开关机按键和动作识别功能  */
#define BDIC_DET_EN               0        /* 使能北斗卡检测功能            */
#define DTIC_DET_EN               0        /* 使能DTU卡检测功能             */
#define DEBUG_EN                  1        /* 使能调试信息功能              */

#define CPU_ENTER_CRITICAL()      _DINT()  /* 关闭全局中断控制位,进入临界区 */
#define CPU_EXIT_CRITICAL()       _EINT()  /* 打开全局中断控制位,退出临界区 */

//调试信息开关
#define GPS_DEBUG                 BIT0     /* GPS调试信息使能               */
#define CDMA_DEBUG                BIT1     /* CDMA调试信息使能              */
#define BD_DEBUG                  BIT2     /* BeiDou调试信息使能            */
#define BAT_DEBUG                 BIT3     /* 电量采集调试信息使能          */

//目前支持的设备指示灯
#define SYS_LED                   BIT0      /* 系统运行指示灯               */
#define CDMA_LED                  BIT1      /* CDMA信号指示灯               */
#define LP_LED                    BIT2      /* 剩余电量指示灯               */

//AT24C02器件从地址
#define EEPROM_WRITE_ADDR         0xa0      /* EEPROM写器件地址             */
#define EEPROM_READ_ADDR          0xa1      /* EEPROM读器件地址             */

//RTC S35390A器件地址
#define RTC_WRITE_ADDR            0x60      /* RTC写器件地址                */
#define RTC_READ_ADDR             0x61      /* RTC读器件地址                */

#define OpenWatchDog()            WDTCTL  &=  ~WDTHOLD  /* 启动看门狗       */
#define SetWatchDog()             WDTCTL   =  0x5A2B    /* 设置为16s        */
#define FreeWatchDog()            WDTCTL   =  0x5A2B    /* 设置为16s        */
#define StopWatchDog()            WDTCTL   =  WDTPW + WDTHOLD /* 关闭看门狗 */

//程序运行指示灯
#define SystemRun                 BIT7
#define SystemRunOut              P8OUT
#define SystemRunDir              P8DIR
#define SystemRunEnable()         SystemRunOut |=  SystemRun
#define SystemRunDisable()        SystemRunOut &= ~SystemRun

//低压指示灯, 三色驱动
#define LowPowerRed               BIT3
#define LowPowerRedOut            P8OUT
#define LowPowerRedDir            P8DIR
#define LowPowerRedEnable()       LowPowerRedOut |=  LowPowerRed
#define LowPowerRedDisable()      LowPowerRedOut &= ~LowPowerRed

#define LowPowerBlue              BIT2
#define LowPowerBlueOut           P8OUT
#define LowPowerBlueDir           P8DIR
#define LowPowerBlueEnable()      LowPowerBlueOut |=  LowPowerBlue
#define LowPowerBlueDisable()     LowPowerBlueOut &= ~LowPowerBlue

#define LowPowerGreen             BIT5
#define LowPowerGreenOut          P8OUT
#define LowPowerGreenDir          P8DIR
#define LowPowerGreenEnable()     LowPowerGreenOut |=  LowPowerGreen
#define LowPowerGreenDisable()    LowPowerGreenOut &= ~LowPowerGreen

//CDMA信号指示灯
#define CdmaIndicator             BIT6
#define CdmaIndicatorOut          P8OUT
#define CdmaIndicatorDir          P8DIR
#define CdmaIndicatorEnable()     CdmaIndicatorOut |=  CdmaIndicator
#define CdmaIndicatorDisable()    CdmaIndicatorOut &= ~CdmaIndicator


//串口0接口定义. 连接CDMA模块
#define UART0_SEL                 P3SEL
#define UART0_DIR                 P3DIR
#define UART0_PORT                P3OUT
#define UART0_TXD                 BIT4
#define UART0_RXD                 BIT5
#define UART0_PIN_CLR()           (UART0_PORT &= ~(UART0_TXD + UART0_RXD) )
#define UART0_PIN_SET()           (UART0_PORT |=  (UART0_TXD + UART0_RXD) )

//串口1接口定义. 连接BD模块
#define UART1_SEL                 P5SEL
#define UART1_DIR                 P5DIR
#define UART1_PORT                P5OUT
#define UART1_TXD                 BIT6
#define UART1_RXD                 BIT7
#define UART1_PIN_CLR()           (UART1_PORT &= ~(UART1_TXD + UART1_RXD) )

//串口2接口定义. 连接GPS
#define UART2_SEL                 P9SEL
#define UART2_DIR                 P9DIR
#define UART2_PORT                P9OUT
#define UART2_TXD                 BIT4
#define UART2_RXD                 BIT5
#define UART2_PIN_CLR()           (UART2_PORT &= ~(UART2_TXD + UART2_RXD) )

//串口3接口定义. 连接USB
#define UART3_SEL                 P10SEL
#define UART3_DIR                 P10DIR
#define UART3_PORT                P10OUT
#define UART3_TXD                 BIT4
#define UART3_RXD                 BIT5
#define UART3_PIN_CLR()           (UART3_PORT &= ~(UART3_TXD + UART3_RXD) )

//开关机按键接口定义
#if KEY_SCAN_EN > 0
#define KeyPower                  BIT5
#define KeyPowerDir               P1DIR
#define KeyPowerIn                P1IN
#define KeyPowerIES               P1IES
#define KeyPowerIE                P1IE
#define KeyPowerIntFlag           P1IFG
#define KeyPowerCheck             (KeyPowerIn & KeyPower )
#define KeyPowerIntCheck()        ((KeyPowerIntFlag & KeyPower) == KeyPower)
#define KeyPowerIntFlagClr()      (KeyPowerIntFlag &= ~KeyPower)
#define KeyPowerIeEnable()        (KeyPowerIE |= KeyPower)
#define KeyPowerIeDisable()       (KeyPowerIE &= ~KeyPower)
#define KeyPowerIesEdge()         (KeyPowerIES |= KeyPower)
#endif

//GPS电源控制
#define GpsPower                  BIT6
#define GpsPowerOut               P9OUT
#define GpsPowerDir               P9DIR
#define GpsPowerOn()              GpsPowerOut |=  GpsPower
#define GpsPowerOff()             GpsPowerOut &= ~GpsPower

//BD电源控制
#define BeiDouPower               BIT0
#define BeiDouPowerOut            P10OUT
#define BeiDouPowerDir            P10DIR
#define BeiDouPowerOn()           BeiDouPowerOut |=  BeiDouPower
#define BeiDouPowerOff()          BeiDouPowerOut &= ~BeiDouPower

#if BDIC_DET_EN > 0
#define BdIcCheck                 BIT7
#define BdIcCheckDir              P4DIR
#define BdIcCheckIn               P4IN
#define BdIcCheckSetIn()          (BdIcCheckDir &= ~BdIcCheck)
#define BdIcDet()                 ((BdIcCheckIn & BdIcCheck) == BdIcCheck)
#endif

/*
//系统自锁电源
#define SystemPower               BIT3
#define SystemPowerPort           P10OUT
#define SystemPowerDir            P10DIR
#define SystemPowerOn()           SystemPowerPort |=  SystemPower
#define SystemPowerOff()          SystemPowerPort &= ~SystemPower
*/

#define EepromWp                  BIT7
#define EepromWpPort              P6OUT
#define EepromWpDir               P6DIR
#define EepromWpDirSetOut()       (EepromWpDir |= EepromWp)
#define EnableEeprom()            EepromWpPort &= ~EepromWp //使能EEPROM
#define DisableEeprom()           EepromWpPort |=  EepromWp //失能EEPROM

//收油机和围油栏标识
MAIN_EXT          UINT8   u8ClassLed;

//定时器延时变量
MAIN_EXT volatile UINT8   g_u8TimerDelay;
MAIN_EXT volatile UINT16  g_u16TimerDelay;

// 时间戳，只记录当天的秒数，用于定时发送信息间隔
MAIN_EXT volatile UINT32  g_u32TimeStamp;

//保存标志
MAIN_EXT          BOOL    g_u8NeedSaved;

#endif

