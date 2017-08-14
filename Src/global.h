
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
//������������
//#define BD_TEST                   1u

//���̰汾��Ϣ
#define VERSION                   100      /* ϵͳ�汾��                    */

//�������룬���ù���
#define KEY_SCAN_EN               0        /* ʹ�ܿ��ػ������Ͷ���ʶ����  */
#define BDIC_DET_EN               0        /* ʹ�ܱ�������⹦��            */
#define DTIC_DET_EN               0        /* ʹ��DTU����⹦��             		*/
#define CDMA_WAKE_EN	          0        /* CDMAģ��˯��ģ��ʹ�ܲ��� */
#define DEBUG_EN                  1        /* ʹ�ܵ�����Ϣ����              */

#define CPU_ENTER_CRITICAL()      _DINT()  /* �ر�ȫ���жϿ���λ,�����ٽ��� */
#define CPU_EXIT_CRITICAL()       _EINT()  /* ��ȫ���жϿ���λ,�˳��ٽ��� */

//������Ϣ����
#define GPS_DEBUG                 BIT0     /* GPS������Ϣʹ��               */
#define CDMA_DEBUG                BIT1     /* CDMA������Ϣʹ��              */
#define BD_DEBUG                  BIT2     /* BeiDou������Ϣʹ��            */
#define BAT_DEBUG                 BIT3     /* �����ɼ�������Ϣʹ��          */

//Ŀǰ֧�ֵ��豸ָʾ��
#define SYS_LED                   BIT0      /* ϵͳ����ָʾ��               */
#define CDMA_LED                  BIT1      /* CDMA�ź�ָʾ��               */
#define LP_LED                    BIT2      /* ʣ�����ָʾ��               */

//AT24C02�����ӵ�ַ
#define EEPROM_WRITE_ADDR         0xa0      /* EEPROMд������ַ             */
#define EEPROM_READ_ADDR          0xa1      /* EEPROM��������ַ             */

//RTC S35390A������ַ
#define RTC_WRITE_ADDR            0x60      /* RTCд������ַ                */
#define RTC_READ_ADDR             0x61      /* RTC��������ַ                */

#define OpenWatchDog()            WDTCTL  &=  ~WDTHOLD  /* �������Ź�       */
#define SetWatchDog()             WDTCTL   =  0x5A2B    /* ����Ϊ16s        */
#define FreeWatchDog()            WDTCTL   =  0x5A2B    /* ����Ϊ16s        */
#define StopWatchDog()            WDTCTL   =  WDTPW + WDTHOLD /* �رտ��Ź� */

//��������ָʾ��
#define SystemRun                 BIT0
#define SystemRunOut              P3OUT
#define SystemRunDir              P3DIR
#define SystemRunEnable()         SystemRunOut |=  SystemRun
#define SystemRunDisable()        SystemRunOut &= ~SystemRun

//��ѹָʾ��, ��ɫ����
#define LowPowerRed               BIT5
#define LowPowerRedOut            P2OUT
#define LowPowerRedDir            P2DIR
#define LowPowerRedEnable()       LowPowerRedOut |=  LowPowerRed
#define LowPowerRedDisable()      LowPowerRedOut &= ~LowPowerRed

#define LowPowerBlue              BIT2
#define LowPowerBlueOut           P2OUT
#define LowPowerBlueDir           P2DIR
#define LowPowerBlueEnable()      LowPowerBlueOut |=  LowPowerBlue
#define LowPowerBlueDisable()     LowPowerBlueOut &= ~LowPowerBlue

#define LowPowerGreen             BIT3
#define LowPowerGreenOut          P2OUT
#define LowPowerGreenDir          P2DIR
#define LowPowerGreenEnable()     LowPowerGreenOut |=  LowPowerGreen
#define LowPowerGreenDisable()    LowPowerGreenOut &= ~LowPowerGreen

//CDMA�ź�ָʾ��
#define CdmaIndicator             BIT7
#define CdmaIndicatorOut          P2OUT
#define CdmaIndicatorDir          P2DIR
#define CdmaIndicatorEnable()     CdmaIndicatorOut |=  CdmaIndicator
#define CdmaIndicatorDisable()    CdmaIndicatorOut &= ~CdmaIndicator


//����0�ӿڶ���. ����GPSģ��
#define UART0_SEL                 P3SEL
#define UART0_DIR                 P3DIR
#define UART0_PORT                P3OUT
#define UART0_TXD                 BIT4
#define UART0_RXD                 BIT5
#define UART0_PIN_CLR()           (UART0_PORT &= ~(UART0_TXD + UART0_RXD) )
#define UART0_PIN_SET()           (UART0_PORT |=  (UART0_TXD + UART0_RXD) )

//����1�ӿڶ���. ����CDMAģ��
#define UART1_SEL                 P5SEL
#define UART1_DIR                 P5DIR
#define UART1_PORT                P5OUT
#define UART1_TXD                 BIT6
#define UART1_RXD                 BIT7
#define UART1_PIN_CLR()           (UART1_PORT &= ~(UART1_TXD + UART1_RXD) )

//����2�ӿڶ���. ����USB
#define UART2_SEL                 P9SEL
#define UART2_DIR                 P9DIR
#define UART2_PORT                P9OUT
#define UART2_TXD                 BIT4
#define UART2_RXD                 BIT5
#define UART2_PIN_CLR()           (UART2_PORT &= ~(UART2_TXD + UART2_RXD) )

//����3�ӿڶ���. ����BD
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
#define GpsPower                  BIT6
#define GpsPowerOut               P9OUT
#define GpsPowerDir               P9DIR
#define GpsPowerOn()              GpsPowerOut |=  GpsPower
#define GpsPowerOff()             GpsPowerOut &= ~GpsPower

//BD��Դ����
#define BeiDouPower               BIT4
#define BeiDouPowerOut            P2OUT
#define BeiDouPowerDir            P2DIR
#define BeiDouPowerOn()           BeiDouPowerOut |=  BeiDouPower
#define BeiDouPowerOff()          BeiDouPowerOut &= ~BeiDouPower

//LED��˸���ƹܽţ�����PIC --> GP1
//�͵�ƽ100ms������˸���͵�ƽ200msֹͣ��˸
#define PicFlashLedPin               BIT5
#define PicFlashLedPinOut            P6OUT
#define PicFlashLedPinDir            P6DIR
#define PicFlashLedSetOut()          PicFlashLedPinDir |=  PicFlashLedPin
#define PicFlashLedOn()              PicFlashLedPinOut |=  PicFlashLedPin
#define PicFlashLedOff()             PicFlashLedPinOut &= ~PicFlashLedPin


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
