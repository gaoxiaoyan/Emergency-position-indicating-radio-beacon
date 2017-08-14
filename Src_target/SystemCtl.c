
/******************************************************************************************************
 *											
 * Copyright (C) 2014, Sunic-ocean
 * All Rights Reserved										
 *
 * �� �� ���� SystemCtl.c
 * ��       ����

 * �������ڣ� 2014��9��4�� 13:23:50
 * ��    �ߣ� Bob
 * ��ǰ�汾�� V1.00
 *******************************************************************************************************/
#define SYS_GLOBAL
#include "msp430x54x.h"
#include "global.h"
#include "DataType.h"
#include "Timer.h"
#include "SystemCtl.h"
#include "string.h"
#include "Uart3.h"
#include "Uart2.h"
#include "SoftTimer.h"
#include "CdmaModule.h"
#include "Rtc.h"
#include "I2c_Driver.h"
#include "Record.h"
#include "Communication.h"
#include "BatteryDetect.h"
#include "GpsParse.h"

#define CPU_CLK       4194304ul 
#define FLL_FACTOR    (CPU_CLK / 32768ul)

//����ʹ��
//#define TEST 

void LpEnterCfg(void);
void LpExitCfg(void);

/*****************************************************************************
 *��������: InitClk
 *��������: ϵͳʱ�ӳ�ʼ��

 *����: 
 *����:
*****************************************************************************/
void InitClk(void) 
{     
    P11DS  |= BIT0 + BIT1 + BIT2;
    P11SEL |= BIT0 + BIT1 + BIT2;
    P11DIR |= BIT0 + BIT1 + BIT2;
    
    // ����XT1  
    P7SEL |= BIT0 + BIT1;                        // P7.0 P7.1 ���蹦��  
    UCSCTL6 &= ~(XT1OFF);                       // XT1��  
    UCSCTL6 |= XCAP_3;                          // �ڲ�����  
    do  
    {  
        UCSCTL7 &= ~XT1LFOFFG;                  // ���XT1������  
    }while (UCSCTL7&XT1LFOFFG);                 // ���XT1������  

    __bis_SR_register(SCG0);                    // ��ֹFLL����  
    UCSCTL0 = 0x0000;                           // Set lowest possible DCOx, MODx  
    UCSCTL1 = DCORSEL_5;                        // DCO���Ƶ��Ϊ6MHz  
    UCSCTL2 = FLLD_1 + FLL_FACTOR;              // ����DCOƵ��Ϊ4MHz  

    __bic_SR_register(SCG0);                    // ʹ��FLL����  
  
    // ��Ҫ��ʱ  
    __delay_cycles(250000);  
  
    // ��������־λ  
    do  
    {  
        UCSCTL7 &= ~(XT2OFFG + XT1LFOFFG + XT1HFOFFG + DCOFFG);  
                                                // ����������������־λ  
        SFRIFG1 &= ~OFIFG;                      // �����������  
    }while (SFRIFG1&OFIFG);                     // �ȴ�������  
    
    UCSCTL4 |= SELS_7 + SELM_7;
}  

/*****************************************************************************
 *��������:  DcoSwitchXt1
 *��������: ����ʱ�Ӻ���ʱ����DCO�л���XT1

 *����: 
 *����:
*****************************************************************************/
void DcoSwitchXt1(void)
{
  UCSCTL4 = SELS_0 + SELM_0;
  __bis_SR_register(SCG0);                    // ��ֹFLL���� 
  __delay_cycles(250);  
}

/*****************************************************************************
 *��������: ReservedPinCfg
 *��������: ��ʹ�õĹܽ�����

 *����: 
 *����:
*****************************************************************************/
void ReservedPinCfg(void)
{
  P1DIR |=  (BIT0 + BIT1 + BIT2 + BIT3 + BIT4);
  P1OUT &= ~(BIT0 + BIT1 + BIT2 + BIT3 + BIT4);
  
  P2DIR |=  (BIT5 + BIT6);
  P2OUT &= ~(BIT5 + BIT6);
  
  P3DIR |=  (BIT2 + BIT3 + BIT6 + BIT7);
  P3OUT &= ~(BIT2 + BIT3 + BIT6 + BIT7);
  
  P4DIR |=  (BIT0 + BIT1 + BIT2 + BIT3 + BIT4 + BIT5 + BIT6);
  P4OUT &= ~(BIT0 + BIT1 + BIT2 + BIT3 + BIT4 + BIT5 + BIT6);
  
  P5DIR |=  (BIT2 + BIT3 + BIT4 + BIT5);
  P5OUT &= ~(BIT2 + BIT3 + BIT4 + BIT5);
  
  P6DIR |=  (BIT0 + BIT1 + BIT2 + BIT4 + BIT5);
  P6OUT &= ~(BIT0 + BIT1 + BIT2 + BIT4 + BIT5);
  
  P7DIR |=  (BIT2 + BIT3 + BIT6 + BIT7);
  P7OUT &= ~(BIT2 + BIT3 + BIT6 + BIT7);
  
  P8DIR |=  (BIT0 + BIT1 + BIT2 + BIT3 + BIT4 + BIT5 + BIT6 + BIT7);
  P8OUT &= ~(BIT0 + BIT1 + BIT2 + BIT3 + BIT4 + BIT5 + BIT6 + BIT7);
  
  P9DIR |=  (BIT0 + BIT1 + BIT2 + BIT3 + BIT6 + BIT7);
  P9OUT &= ~(BIT0 + BIT1 + BIT2 + BIT3 + BIT6 + BIT7);
  
  P10DIR |=  (BIT6 + BIT7);
  P10OUT &= ~(BIT6 + BIT7);

  P11DIR |=  (BIT0 + BIT1 + BIT2);
  P11OUT &= ~(BIT0 + BIT1 + BIT2);
  
  PJDIR  |=  (BIT0 + BIT1 + BIT2 + BIT3);
  PJOUT  &= ~(BIT0 + BIT1 + BIT2 + BIT3);
}

/*****************************************************************************
 *��������: SystemInit
 *��������: ϵͳ��ʼ��

 *����: 
 *����:
*****************************************************************************/
void SystemInit(void)
{
  
  //��������ָʾ��
  SystemRunDisable();
  SystemRunDir |= SystemRun;
  
  //��ѹָʾ��
  LowPowerLedDisable();
  LowPowerLedDir |= LowPowerLed;
 
 	//Χ����ָʾ��
 	WylLedDisable();
 	WylLedDir |= WylLed;
 	
 	//���ͻ�ָʾ��
 	SyjLedDisable();
 	SyjLedDir |= SyjLed;
  
  EepromPowerDir |= EepromPower;
  EepromPowerOn();
  
  EepromWpDirSetOut();
  EnableEeprom();
 	
 	//���ػ������ӿ�, �½��ش����ж�
 	KeyPowerIesEdge();
// 	KeyPowerIeEnable();
 	KeyPowerDir &= ~KeyPower;
 	
 	//GPS��Դ���ƽӿ�
 	GpsPowerOff();
 	GpsPowerDir |= GpsPower;
 	
 	//CDMA��Դ���ƽӿ�
 	CdmaPowerOff();
 	CdmaPowerDir |= CdmaPower;
 	
 	//BD��Դ���ƽӿ�
 	BeiDouPowerOff();
 	BeiDouPowerDir |= BeiDouPower;
  //����ģ��IC���
  BdIcCheckSetIn();
 	
 	//������Դ���ƽӿ�
 	SystemPowerOff();
 	SystemPowerDir |= SystemPower;
  
  OS_MemInit();     
  
  ReservedPinCfg();
  
  g_bCdmaSend = FALSE;
  g_bBdSend   = FALSE;
  g_bSendInfo = FALSE;
}

/*****************************************************************************
 *��������: WaitSystemStart
 *��������: ϵͳ�������

 *����: 
 *����:
*****************************************************************************/
void WaitSystemStart(void)
{  
  SINT8 s8Dog;
  UINT8 u8Bat;

  s8Dog = SwdGet();
  SwdSetLifeTime(s8Dog, 3000);

  while(TRUE)
  {
    if(!KeyPowerCheck)
    {
      if(SwdIsEnable(s8Dog) == TRUE)
      {
        if(SwdGetLifeTime(s8Dog) == 0)
        {
          SwdDisable(s8Dog);
          SwdFree(s8Dog);
                    
          SystemPowerOn();
          
          GpsPowerOn();
          CdmaPowerOn();
          
          u8Bat = AdcDetect(0);
          switch(u8Bat)
          {
            case 0:                     
            {
              FreeWatchDog();          //�ͷſ��Ź�
              GpsPowerOff();
              CdmaPowerOff();
              while(TRUE)              //��ѹ����ʱ��ֱ���ͷ�16�뿴�Ź�������ѹָʾ��˸16���ر�ϵͳ
              {                   
                g_u8TimerDelay = 0;
                while(g_u8TimerDelay<50);
                LowPowerLedDisable();
                
                g_u8TimerDelay = 0;
                while(g_u8TimerDelay<50);
                LowPowerLedEnable();
              }
              
              SystemPowerOn();
              break;
            }
            
            case 1:                   //����20%����
            {
              SystemRunDisable();
              LowPowerLedDisable();
              WylLedDisable();
              SyjLedEnable();
              break;
            }
            
            case 2:                   //����40%����
            {
              SystemRunDisable();
              LowPowerLedDisable();
              WylLedEnable();
              SyjLedEnable();
              break;
            }
            
            case 3:                   //����60%����
            {
              SystemRunDisable();
              LowPowerLedEnable();
              WylLedEnable();
              SyjLedEnable();
              break;
            }
            
            case 4:                   //����80%����
            {
              SystemRunEnable();
              LowPowerLedEnable();
              WylLedEnable();
              SyjLedEnable();
              break;
            }
            
            default:  
            {
              break;
            }
          }
         
          FreeWatchDog();          //�ͷſ��Ź�
          
          return;
        }
        else
        {
          SystemPowerOff();
        }
      }
      else
      {
        SwdEnable(s8Dog);
      }
    }
    else
    {
      SystemPowerOff();
    }
  }
}


/*****************************************************************************
 �� �� ��  : PORT1_ISR
 ��������  : P1�˿ڵ��жϷ�����

  ��  ��  : NONE
 �� �� ֵ : NONE
*****************************************************************************/
BOOL bKeyFlag = FALSE;
BOOL bKeyCompleted = FALSE;
BOOL bLedFlag=FALSE;
SINT8 s8LedDog;
SINT8 s8KeyDog;
SINT8 s8KeySetDog;
BOOL  bKeyWakeUp;
#pragma vector = PORT1_VECTOR
__interrupt void PORT1_ISR(void)
{
  static UINT8 u8KeySet = 1;
  _tsTimeCb sTime;
  
  if(KeyPowerIntCheck())
  {
    KeyPowerIntFlagClr();
    bKeyFlag = TRUE;
    
    if(bKeyWakeUp == FALSE)
    {
      if((u8KeySet > 2) && (SwdGetLifeTime(s8KeySetDog) > 0))
      {
        bKeyCompleted = TRUE;
        SwdDisable(s8KeySetDog);
        SwdReset(s8KeySetDog);    //��λ��ʱ����
        u8KeySet = 1;
      }
      
      if(SwdGetLifeTime(s8KeySetDog) == 0)
      {
        u8KeySet = 1;
      }
      u8KeySet++;
    }

  }
  
  if( RtcIntCheck() )
  {
    RtcIntFlagClr();
    
    sRecordCb.u32SysRunTime++;
    
    //�˴���ʱ�Ƚϴ󣬵��ǻ���RTC�жϻ��ƣ��ֱ����жϺ���7.81ms�ٴ�����ж�
    //��ʹ��ʱ����ۼӷ�ʽ�������һ���������ڴ���״̬�£���ֱ�Ӷ�ȡRTCʱ��
    //���ж��Ƿ񵽻���ʱ�̣�������RTC���жϣ���7.81msʱ��
    GetTimeFromRtc(&sTime);
    if(g_bStandby == TRUE)
    {
      if(((sTime.Minute + 2) % (sSendInfo.u8GapTime * 5)) == 0) //��ǰ�����ӻ���
      {
         LPM3_EXIT;
      }
    }
    else
    {
      if(((sTime.Minute + 1) % (sSendInfo.u8GapTime * 5)) == 0)       //��ǰһ���Ӵ�ͨѶģ��
      {
         g_bCommuWake = TRUE;
      }
    
      if(g_bGpsCompleted == TRUE)
      {
        if((sTime.Minute % (sSendInfo.u8GapTime * 5)) == 0)       //��ʱ����
        {
           g_bSendInfo = TRUE;
        }
      }
    }
    
    SetRtcInt1Out();
    
    I2cSclDir |= I2cScl;
    I2cSdaDir |= I2cSda;
    I2cSclOut |= I2cScl;
    I2cSdaOut |= I2cSda;
  }
  
  if( CdmaRiIntCheck() )
  {
    CdmaRiIntFlagClr();
    if(g_bStandby == TRUE)
    {
      g_bCdmaRiEvent = TRUE;      //���յ�����ʱ��RI�¼���־λ��λ
      LPM3_EXIT;
    }
  }
}

/***************************************************************************
//�������ƣ�EnterLowPower
//�������������ݷ��ͳɹ�������͹���ģ��
//
//��������
//���أ���
***************************************************************************/
void EnterLowPower(void)
{
   if( TRUE == g_bStandby )
   {      
     StopWatchDog();
     GpsPowerOff();
     
     SystemRunDisable();
     WylLedDisable();
     SyjLedDisable();
     //��RTC�ⲿ�ж�
      SetRtcInt1Out();
      //�򿪰����жϻ��ѹ���
      bKeyWakeUp = TRUE;
     
     g_u16TimerDelay=0;
     while(g_u16TimerDelay < 500);  //��ʱ5S, �ȴ���Ϣ��ȫ���͡�

      CdmaDtrSetHigh();     //����DTR����CDMA���������();
      
		  BeiDouPowerOff();
      
      //�ر�Timer
      TimerInterruptDisable();  //����رն�ʱ���жϣ�����ʱ��һֱ�ж�ռ���ⲿ�жϽ����жϡ�
      //�ر�ADC
      ADC_STOP_DETECT();
      //�ر�Uart
      
      //�͹��Ĺܽ�����
      LpEnterCfg();
      
      //��ʼ���л����ⲿ����
      DcoSwitchXt1();
      
      //�������
      //��RTC�����ж�
      LPM3;
      
      //���Ѻ��Ƚ�ʱ�ӻָ���DCO��
      InitClk();
           
      LpExitCfg();
      
      //����ADC
      ADC_START_DETECT();
      //����Timer
      TimerInterruptEnable(); 
      //����Uart
      
      g_bStandby = FALSE;
      bKeyWakeUp = FALSE;
      
      SystemRunEnable();
      Uart2EnableInterrupt();
      GpsPowerOn();
      
      SaveSystemRunTime(&sRecordCb.u32SysRunTime);
      
      if(sRecordCb.u8SysType == WYL_LED)
      {
        WylLedEnable();
      }
      else
      {
        if(sRecordCb.u8SysType == SYJ_LED)
        {
          SyjLedEnable();
        }
      }
   }
}

/***************************************************************************
//�������ƣ�SystemIdleInit
//������������ʼ��
//
//��������
//���أ���
***************************************************************************/
void SystemIdleInit(void)
{
  //��ʼ����������ָʾ�ƶ�ʱ��
  s8LedDog = SwdGet();
  SwdSetName(s8LedDog, "LedTimer");
  SwdSetLifeTime(s8LedDog, 500);
  SwdEnable(s8LedDog);  
  
  //��ʼ���ػ�������ʱ��
  s8KeyDog = SwdGet();
  SwdSetName(s8KeyDog, "KeyTimer");
  SwdSetLifeTime(s8KeyDog, 3000);
  SwdDisable(s8KeyDog);  
  
  //��ʼ���ػ�������ʱ��
  s8KeySetDog = SwdGet();
  SwdSetName(s8KeySetDog, "KeySetTimer");
  SwdSetLifeTime(s8KeySetDog, 1000);
  SwdDisable(s8KeySetDog);  
  
  if(sRecordCb.u8SysType == WYL_LED)
  {
    WylLedEnable();
    SyjLedDisable();
  }
  else
  {
    if(sRecordCb.u8SysType == SYJ_LED)
    {
      WylLedDisable();
      SyjLedEnable();
    }
  }
  
  LowPowerLedDisable();
  
  //���ж�ʹ��
  KeyPowerIeEnable();
  RtcIntIeEnable();
  CdmaRiIntEnable();
}

/***************************************************************************
//�������ƣ�SytemIdleRun
//������������������ָʾ��
//
//��������
//���أ���
***************************************************************************/
void SystemIdleRun(void)
{
  BOOL bFlag = FALSE;
  
  if(SwdGetLifeTime(s8LedDog) == 0)
  {
    SwdReset(s8LedDog);
    bLedFlag = ~bLedFlag;
        
    if(bLedFlag == 0)
      SystemRunEnable();
    else
      SystemRunDisable();
  }
  
  if(bKeyFlag == TRUE)
  {
    bKeyFlag = FALSE;
    SwdEnable(s8KeySetDog);
    SwdSetLifeTime(s8KeySetDog, 300);
  }
  //�������
  if(bKeyCompleted == TRUE)
  {
    bKeyCompleted = FALSE;
    SwdSetLifeTime(s8KeySetDog, 6000);
    SwdEnable(s8KeySetDog);  
    SystemRunDisable();       //�ر�LED
    KeyPowerIeDisable();      //�رհ����ж�
    StopWatchDog();
    while(SwdGetLifeTime(s8KeySetDog)>0)
    {
      if(!KeyPowerCheck)
      {
        g_u8TimerDelay=0;
        while(g_u8TimerDelay<2);  //��ʱ20ms��
        if(!KeyPowerCheck)
        {
          while(!KeyPowerCheck);
          SwdReset(s8KeySetDog);    //��λ��ʱ����
          if(bFlag == TRUE)
          {
            sRecordCb.u8SysType = SYJ_LED;
            g_u8NeedSaved |= SAVE_SYS_TYPE;
            
            WylLedDisable();
            SyjLedEnable();
        
            bFlag = FALSE;
          }
          else
          {
            sRecordCb.u8SysType = WYL_LED;
            g_u8NeedSaved |= SAVE_SYS_TYPE;
            
            WylLedEnable();
            SyjLedDisable();
        
            bFlag = TRUE;
          }
        }
      }
    }
    SwdSetLifeTime(s8KeySetDog, 1000);
    SwdDisable(s8KeySetDog);  
    KeyPowerIeEnable();      //�رհ����ж�
  }
  
  //���ػ�
  if( !KeyPowerCheck)
  {
    if(SwdIsEnable(s8KeyDog) == TRUE)
    {
      if(SwdGetLifeTime(s8KeyDog) == 0)
      {
        SwdReset(s8KeyDog);
        
        //�ر�LEDָʾ
        SystemRunDisable();
        WylLedDisable();
        SyjLedDisable();
        
        GpsPowerOff();
        CdmaPowerOff();
        BeiDouPowerOff();
        //�ر�ϵͳ��Դ
        SystemPowerOff();
        
        while(TRUE);  //�ȴ����Ź���λ
      }
    }
    else
    {
      SwdEnable(s8KeyDog);
    }
  }
  else
  {
    SwdDisable(s8KeyDog);
    SwdReset(s8KeyDog);
  }
}

/***************************************************************************
//�������ƣ�LpEnterCfg
//��������������͹��Ĺܽ�����
//
//��������
//���أ���
***************************************************************************/
void LpEnterCfg(void)
{
  UART0_SEL &=  ~(UART0_TXD + UART0_RXD);       // ѡ��˿���UARTͨ�Ŷ˿�
  UART0_DIR |=   (UART0_TXD + UART0_RXD);
  UART0_PIN_SET();
//  UART0_PIN_CLR();
  
  UART1_SEL &=  ~(UART1_TXD + UART1_RXD);       // ѡ��˿���UARTͨ�Ŷ˿�
  UART1_DIR |=   (UART1_TXD + UART1_RXD);
  UART1_PIN_CLR();
  
  UART2_SEL &=  ~(UART2_TXD + UART2_RXD);       // ѡ��˿���UARTͨ�Ŷ˿�
  UART2_DIR |=   (UART2_TXD + UART2_RXD);
  UART2_PIN_CLR();
  
  UART3_SEL &=  ~(UART3_TXD + UART3_RXD);       // ѡ��˿���UARTͨ�Ŷ˿�
  UART3_DIR |=   (UART3_TXD + UART3_RXD);
  UART3_PIN_CLR();
  
  I2cSclDir |= I2cScl;
  I2cSdaDir |= I2cSda;
  I2cSclOut |= I2cScl;
  I2cSdaOut |= I2cSda;
  
  CdmaIcCheckDir |=  CdmaIcCheck;
  CdmaIcCheckOut &= ~CdmaIcCheck;
  
  CdmaOnOffDir  |=  CdmaOnOff;
  CdmaOnOffOut  |=  CdmaOnOff;
  
  CdmaResetDir  |=  CdmaReset;
  CdmaResetOut  |=  CdmaReset;
  
/*  CdmaOnOffDir  |=  CdmaOnOff;
  CdmaOnOffOut  &= ~CdmaOnOff;
  
  CdmaResetDir  |=  CdmaReset;
  CdmaResetOut  &= ~CdmaReset;
    //CDMA RI�˿�
  CdmaRiDir |=  CdmaRi;
  CdmaRiOut &= ~CdmaRi;*/
}

/***************************************************************************
//�������ƣ�LpExitCfg
//�����������˳��͹��Ĺܽ�����
//
//��������
//���أ���
***************************************************************************/
void LpExitCfg(void)
{
  UART0_SEL |=  UART0_TXD + UART0_RXD;       // ѡ��˿���UARTͨ�Ŷ˿�
  UART0_DIR |=  UART0_TXD;
  UART0_DIR &= ~UART0_RXD;
  
  UART1_SEL |=  UART1_TXD + UART1_RXD;       // ѡ��˿���UARTͨ�Ŷ˿�
  UART1_DIR |=  UART1_TXD;
  UART1_DIR &= ~UART1_RXD;
  
  UART2_SEL |=  UART2_TXD + UART2_RXD;       // ѡ��˿���UARTͨ�Ŷ˿�
  UART2_DIR |=  UART2_TXD;
  UART2_DIR &= ~UART2_RXD;
  
  UART3_SEL |=  UART3_TXD + UART3_RXD;       // ѡ��˿���UARTͨ�Ŷ˿�
  UART3_DIR |=  UART3_TXD;
  UART3_DIR &= ~UART3_RXD;
}

