
#ifndef __MAIN_H__
#define __MAIN_H__

#include "msp430x54x.h"
#include "DataType.h"
#include "OS_MEM.h"

/*-------------------------------------------------------------------------*
 *    �궨��                                                               *
 *-------------------------------------------------------------------------*/
//#define CPU_CLOCK		1000000UL

//�򿪵��Կ���ʹ��
//#define DEBUG              1u

//���̰汾��Ϣ
#define VERSION              100

#define CPU_ENTER_CRITICAL()     _DINT()     //�ر�ȫ���жϿ���λ,�����ٽ���
#define CPU_EXIT_CRITICAL()      _EINT()     //��ȫ���жϿ���λ,�˳��ٽ���

//������Ϣ����
extern BOOL g_bDebug;

//AT24C02�����ӵ�ַ
#define EEPROM_WRITE_ADDR    0xa0
#define EEPROM_READ_ADDR     0xa1

//RTC S35390A������ַ
#define RTC_WRITE_ADDR       0x60
#define RTC_READ_ADDR        0x61

#define OpenWatchDog()        WDTCTL  &=  ~WDTHOLD
#define SetWatchDog()         WDTCTL  =  0x5A2B   //����Ϊ16s
#define FreeWatchDog()        WDTCTL  =  0x5A2B   //����Ϊ16s
#define StopWatchDog()        WDTCTL  =  WDTPW + WDTHOLD   //�رտ��Ź�

//ADCʹ�ܿ���
#define ADC_START_DETECT()    ADC12CTL0 |= ADC12ENC + ADC12SC + ADC12ON
#define ADC_STOP_DETECT()     ADC12CTL0 &= ~(ADC12ENC + ADC12SC + ADC12ON)

//��������ָʾ��
#define SystemRun             BIT4
#define SystemRunOut          P2OUT
#define SystemRunDir          P2DIR
#define SystemRunEnable()     SystemRunOut |=  SystemRun
#define SystemRunDisable()    SystemRunOut &= ~SystemRun

//��ѹָʾ��
#define LowPowerLed           BIT3
#define LowPowerLedOut        P2OUT
#define LowPowerLedDir        P2DIR
#define LowPowerLedEnable()   LowPowerLedOut |=  LowPowerLed
#define LowPowerLedDisable()  LowPowerLedOut &= ~LowPowerLed

//Χ����ָʾ��
#define WylLed                BIT2
#define WylLedOut             P2OUT
#define WylLedDir             P2DIR
#define WylEnable()           WylLedOut |=  WylLed
#define WylLedDisable()       WylLedOut &= ~WylLed

//���ͻ�ָʾ��
#define SyjLed                BIT1
#define SyjLedOut             P2OUT
#define SyjLedDir             P2DIR
#define SyjLedEnable()        SyjLedOut |=  SyjLed
#define SyjLedDisable()       SyjLedOut &= ~SyjLed

//����0�ӿڶ���. ����CDMAģ��
#define UART0_SEL             P3SEL
#define UART0_DIR             P3DIR
#define UART0_TXD             BIT4
#define UART0_RXD             BIT5
//����1�ӿڶ���. ����BDģ��
#define UART1_SEL             P5SEL
#define UART1_DIR             P5DIR
#define UART1_TXD             BIT6
#define UART1_RXD             BIT7
//����2�ӿڶ���. ����GPS  
#define UART2_SEL             P9SEL
#define UART2_DIR             P9DIR
#define UART2_TXD             BIT4
#define UART2_RXD             BIT5
//����3�ӿڶ���. ����USB
#define UART3_SEL             P10SEL
#define UART3_DIR             P10DIR
#define UART3_TXD             BIT4
#define UART3_RXD             BIT5

//���ػ������ӿڶ���
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

//RTC�жϽӿ�
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

//CDMA_RI�жϽӿ�
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

//GPS��Դ����
#define GpsPower              BIT0
#define GpsPowerOut           P10OUT
#define GpsPowerDir           P10DIR
#define GpsPowerOn()          GpsPowerOut |=  GpsPower
#define GpsPowerOff()         GpsPowerOut &= ~GpsPower

//CDMA��Դ����
#define CdmaPower              BIT1
#define CdmaPowerOut           P10OUT
#define CdmaPowerDir           P10DIR
#define CdmaPowerOn()          CdmaPowerOut |=  CdmaPower
#define CdmaPowerOff()         CdmaPowerOut &= ~CdmaPower

//BD��Դ����
#define BeiDouPower             BIT2
#define BeiDouPowerOut          P10OUT
#define BeiDouPowerDir          P10DIR
#define BeiDouPowerOn()         BeiDouPowerOut |=  BeiDouPower
#define BeiDouPowerOff()        BeiDouPowerOut &= ~BeiDouPower

//ϵͳ������Դ
#define SystemPower           BIT3
#define SystemPowerPort       P10OUT
#define SystemPowerDir        P10DIR
#define SystemPowerOn()       SystemPowerPort |=  SystemPower
#define SystemPowerOff()      SystemPowerPort &= ~SystemPower

//I2C�˿ڶ���
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
#define EnableEeprom()       EepromWpPort &= ~EepromWp //ʹ��EEPROM
#define DisableEeprom()      EepromWpPort |=  EepromWp //ʧ��EEPROM

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

// ʱ�����ֻ��¼��������������ڶ�ʱ������Ϣ���
extern volatile UINT32  g_u32TimeStamp;

#endif