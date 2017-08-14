
#ifndef __MAIN_H__
#define __MAIN_H__

#include "msp430x54x.h"
#include "DataType.h"
#include "OS_MEM.h"

/*-------------------------------------------------------------------------*
 *    �궨��                                                               *
 *-------------------------------------------------------------------------*/
//#define CPU_CLOCK		1000000UL

#ifdef  MAIN_GLOBAL
#define MAIN_EXT
#else
#define MAIN_EXT extern
#endif

//�򿪵��Կ���ʹ��
//#define DEBUG                   1u

//���̰汾��Ϣ
#define VERSION                   100

//�������룬���ù���
#define KEY_SCAN_EN               0        /* ʹ�ܿ��ػ������Ͷ���ʶ���� */
#define BDIC_DET_EN               0        /* ʹ�ܱ�������⹦��           */
#define DTIC_DET_EN               0        /* ʹ��DTU����⹦��            */

#define CPU_ENTER_CRITICAL()      _DINT()     //�ر�ȫ���жϿ���λ,�����ٽ���
#define CPU_EXIT_CRITICAL()       _EINT()     //��ȫ���жϿ���λ,�˳��ٽ���

//������Ϣ����
#define GPS_DEBUG                 BIT0
#define CDMA_DEBUG                BIT1
#define BD_DEBUG                  BIT2
#define BAT_DEBUG                 BIT3

//Ŀǰ֧�ֵ��豸ָʾ��
#define SYJ_LED                   BIT0
#define WYL_LED                   BIT1

//AT24C02�����ӵ�ַ
#define EEPROM_WRITE_ADDR         0xa0
#define EEPROM_READ_ADDR          0xa1

//RTC S35390A������ַ
#define RTC_WRITE_ADDR            0x60
#define RTC_READ_ADDR             0x61

#define OpenWatchDog()            WDTCTL  &=  ~WDTHOLD
#define SetWatchDog()             WDTCTL   =  0x5A2B   //����Ϊ16s
#define FreeWatchDog()            WDTCTL   =  0x5A2B   //����Ϊ16s
#define StopWatchDog()            WDTCTL   =  WDTPW + WDTHOLD   //�رտ��Ź�

//��������ָʾ��
#define SystemRun                 BIT1
#define SystemRunOut              P6OUT
#define SystemRunDir              P6DIR
#define SystemRunEnable()         SystemRunOut |=  SystemRun
#define SystemRunDisable()        SystemRunOut &= ~SystemRun

//��ѹָʾ��
#define LowPowerLed               BIT0
#define LowPowerLedOut            P6OUT
#define LowPowerLedDir            P6DIR
#define LowPowerLedEnable()       LowPowerLedOut |=  LowPowerLed
#define LowPowerLedDisable()      LowPowerLedOut &= ~LowPowerLed

//����0�ӿڶ���. ����CDMAģ��
#define UART0_SEL                 P3SEL
#define UART0_DIR                 P3DIR
#define UART0_PORT                P3OUT
#define UART0_TXD                 BIT4
#define UART0_RXD                 BIT5
#define UART0_PIN_CLR()           (UART0_PORT &= ~(UART0_TXD + UART0_RXD) )   
#define UART0_PIN_SET()           (UART0_PORT |=  (UART0_TXD + UART0_RXD) )      

//����1�ӿڶ���. ����BDģ��
#define UART1_SEL                 P5SEL
#define UART1_DIR                 P5DIR
#define UART1_PORT                P5OUT
#define UART1_TXD                 BIT6
#define UART1_RXD                 BIT7
#define UART1_PIN_CLR()           (UART1_PORT &= ~(UART1_TXD + UART1_RXD) ) 

//����2�ӿڶ���. ����GPS  
#define UART2_SEL                 P9SEL
#define UART2_DIR                 P9DIR
#define UART2_PORT                P9OUT
#define UART2_TXD                 BIT4
#define UART2_RXD                 BIT5
#define UART2_PIN_CLR()           (UART2_PORT &= ~(UART2_TXD + UART2_RXD) ) 

//����3�ӿڶ���. ����USB
#define UART3_SEL                 P10SEL
#define UART3_DIR                 P10DIR
#define UART3_PORT                P10OUT
#define UART3_TXD                 BIT4
#define UART3_RXD                 BIT5
#define UART3_PIN_CLR()           (UART3_PORT &= ~(UART3_TXD + UART3_RXD) ) 

//���ػ������ӿڶ���
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

//GPS��Դ����
#define GpsPower                  BIT0
#define GpsPowerOut               P8OUT
#define GpsPowerDir               P8DIR
#define GpsPowerOn()              GpsPowerOut |=  GpsPower
#define GpsPowerOff()             GpsPowerOut &= ~GpsPower

//BD��Դ����
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
//ϵͳ������Դ
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
#define EnableEeprom()            EepromWpPort &= ~EepromWp //ʹ��EEPROM
#define DisableEeprom()           EepromWpPort |=  EepromWp //ʧ��EEPROM

//Debug���������л���1 -- debugģʽ��0 -- ����ģʽ
#define DebugCtl                  BIT5
#define DebugCtlOut               P1OUT
#define DebugCtlDir               P1DIR
#define DebugCtlSet()             DebugCtlOut |=  DebugCtl
#define DebugCtlClr()             DebugCtlOut &= ~DebugCtl

//RST���������л���1 -- debugģʽ��0 -- ����ģʽ
#define RstCtl                    BIT7
#define RstCtlOut                 P9OUT
#define RstCtlDir                 P9DIR
#define RstCtlSet()               RstCtlOut |=  RstCtl
#define RstCtlClr()               RstCtlOut &= ~RstCtl

//���ͻ���Χ������ʶ
MAIN_EXT          UINT8   u8ClassLed;

//��ʱ����ʱ����
MAIN_EXT volatile UINT8   g_u8TimerDelay;
MAIN_EXT volatile UINT16  g_u16TimerDelay;

// ʱ�����ֻ��¼��������������ڶ�ʱ������Ϣ���
MAIN_EXT volatile UINT32  g_u32TimeStamp;

//�����־
MAIN_EXT          BOOL    g_u8NeedSaved;

#endif