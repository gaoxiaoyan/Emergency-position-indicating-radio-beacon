
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
#define VERSION                   100

//条件编译，剪裁功能
#define KEY_SCAN_EN               0        /* 使能开关机按键和动作识别功能 */
#define BDIC_DET_EN               0        /* 使能北斗卡检测功能           */
#define DTIC_DET_EN               0        /* 使能DTU卡检测功能            */

#define CPU_ENTER_CRITICAL()      _DINT()     //关闭全局中断控制位,进入临界区
#define CPU_EXIT_CRITICAL()       _EINT()     //打开全局中断控制位,退出临界区

//调试信息开关
#define GPS_DEBUG                 BIT0
#define CDMA_DEBUG                BIT1
#define BD_DEBUG                  BIT2
#define BAT_DEBUG                 BIT3

//目前支持的设备指示灯
#define SYJ_LED                   BIT0
#define WYL_LED                   BIT1

//AT24C02器件从地址
#define EEPROM_WRITE_ADDR         0xa0
#define EEPROM_READ_ADDR          0xa1

//RTC S35390A器件地址
#define RTC_WRITE_ADDR            0x60
#define RTC_READ_ADDR             0x61

#define OpenWatchDog()            WDTCTL  &=  ~WDTHOLD
#define SetWatchDog()             WDTCTL   =  0x5A2B   //设置为16s
#define FreeWatchDog()            WDTCTL   =  0x5A2B   //设置为16s
#define StopWatchDog()            WDTCTL   =  WDTPW + WDTHOLD   //关闭看门狗

//程序运行指示灯
#define SystemRun                 BIT1
#define SystemRunOut              P6OUT
#define SystemRunDir              P6DIR
#define SystemRunEnable()         SystemRunOut |=  SystemRun
#define SystemRunDisable()        SystemRunOut &= ~SystemRun

//低压指示灯
#define LowPowerLed               BIT0
#define LowPowerLedOut            P6OUT
#define LowPowerLedDir            P6DIR
#define LowPowerLedEnable()       LowPowerLedOut |=  LowPowerLed
#define LowPowerLedDisable()      LowPowerLedOut &= ~LowPowerLed

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
#define GpsPower                  BIT0
#define GpsPowerOut               P8OUT
#define GpsPowerDir               P8DIR
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

//Debug控制引脚切换，1 -- debug模式，0 -- 下载模式
#define DebugCtl                  BIT5
#define DebugCtlOut               P1OUT
#define DebugCtlDir               P1DIR
#define DebugCtlSet()             DebugCtlOut |=  DebugCtl
#define DebugCtlClr()             DebugCtlOut &= ~DebugCtl

//RST控制引脚切换，1 -- debug模式，0 -- 下载模式
#define RstCtl                    BIT7
#define RstCtlOut                 P9OUT
#define RstCtlDir                 P9DIR
#define RstCtlSet()               RstCtlOut |=  RstCtl
#define RstCtlClr()               RstCtlOut &= ~RstCtl

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