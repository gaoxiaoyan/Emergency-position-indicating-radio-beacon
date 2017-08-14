
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
#include "BeiDouModule.h"

#define CPU_CLK       4194304ul 
#define FLL_FACTOR    (CPU_CLK / 32768ul)

//����ʹ��
//#define TEST 

void LpEnterCfg(void);
void LpExitCfg(void);

/*
****************************************************************************
*��������: InitClk
*��������: ϵͳʱ�ӳ�ʼ��
*
*����: ϵͳ����д1��ϵͳ����ʱĬ��DCO��1Mhz����Ҫ��ʱ���ڳ�
*      ��������д0����32768hz���ѣ���Ҫ��ʱ���ڶ�
*����:
****************************************************************************
*/
void InitClk(BOOL bFlag) 
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
    if(bFlag == TRUE)
      __delay_cycles(250000);  
    else
      __delay_cycles(6000);  
  
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
  P1DIR &= ~(BIT0 + BIT3 + BIT4);
  P1OUT &= ~(BIT0 + BIT3 + BIT4);
  
  P2DIR &= ~(BIT1 + BIT2 + BIT3 + BIT4 + BIT5 + BIT6 + BIT7);
  P2OUT &= ~(BIT1 + BIT2 + BIT3 + BIT4 + BIT5 + BIT6 + BIT7);
  
  P3DIR &= ~(BIT2 + BIT3 + BIT6 + BIT7);
  P3OUT &= ~(BIT2 + BIT3 + BIT6 + BIT7);
  
  P4DIR &= ~(BIT0 + BIT1 + BIT2 + BIT3 + BIT4 + BIT5 + BIT6 + BIT7);
  P4OUT &= ~(BIT0 + BIT1 + BIT2 + BIT3 + BIT4 + BIT5 + BIT6 + BIT7);
  
  P5DIR &= ~(BIT2 + BIT3 + BIT4 + BIT5);
  P5OUT &= ~(BIT2 + BIT3 + BIT4 + BIT5);
  
  P6DIR &= ~(BIT4 + BIT5 + BIT6);
  P6OUT &= ~(BIT4 + BIT5 + BIT6);
  
  P7DIR &= ~(BIT2 + BIT3 + BIT6 + BIT7);
  P7OUT &= ~(BIT2 + BIT3 + BIT6 + BIT7);
  
  P8DIR &= ~(BIT2 + BIT3 + BIT4 + BIT5 + BIT6 + BIT7);
  P8OUT &= ~(BIT2 + BIT3 + BIT4 + BIT5 + BIT6 + BIT7);
  
  P9DIR &= ~(BIT0 + BIT1 + BIT2 + BIT3 + BIT6 + BIT7);
  P9OUT &= ~(BIT0 + BIT1 + BIT2 + BIT3 + BIT6 + BIT7);
  
  P10DIR &= ~(BIT1 + BIT2 + BIT3 + BIT6 + BIT7);
  P10OUT &= ~(BIT1 + BIT2 + BIT3 + BIT6 + BIT7);

  P11DIR &= ~(BIT0 + BIT1 + BIT2);
  P11OUT &= ~(BIT0 + BIT1 + BIT2);
  
//  PJDIR  |=  (BIT0 + BIT1 + BIT2 + BIT3);
//  PJOUT  &= ~(BIT0 + BIT1 + BIT2 + BIT3);
}

/*****************************************************************************
 *��������: SystemInit
 *��������: ϵͳ��ʼ��

 *����: 
 *����:
*****************************************************************************/
void SystemInit(void)
{
  /* debug�����л� */
  DebugCtlSet();
  DebugCtlDir |= DebugCtl;
  
  RstCtlSet();
  RstCtlDir   |= RstCtl;
    
  //��������ָʾ��
  SystemRunDisable();
  SystemRunDir |= SystemRun;
  
  //��ѹָʾ��
  LowPowerLedDisable();
  LowPowerLedDir |= LowPowerLed;
  
  EepromWpDirSetOut();
  EnableEeprom();
 	
//���ػ������ӿ�, �½��ش����ж�
#if KEY_SCAN_EN > 0
 	KeyPowerIesEdge();
 	KeyPowerIeEnable();
 	KeyPowerDir &= ~KeyPower;
#endif
 	
 	//GPS��Դ���ƽӿ�
 	GpsPowerOn();
 	GpsPowerDir |= GpsPower;
 	
 	//CDMA��Դ���ƽӿ�
 	CdmaPowerOn();
 	CdmaPowerDir |= CdmaPower;
 	
 	//BD��Դ���ƽӿ�
 	BeiDouPowerOn();
 	BeiDouPowerDir |= BeiDouPower;
//����ģ��IC���
//  BdIcCheckSetIn();
 	
 	//������Դ���ƽӿ�
#if KEY_SCAN_EN > 0
 	SystemPowerOff();
 	SystemPowerDir |= SystemPower;
#endif
  
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
#if KEY_SCAN_EN > 0
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
          BeiDouPowerOn();
//          CdmaPowerOn();
          
          u8Bat = AdcDetect(0);

          switch(u8Bat)
          {
            case 0:                     
            {
              FreeWatchDog();          //�ͷſ��Ź�
              while(TRUE)              //��ѹ����ʱ��ֱ���ͷ�16�뿴�Ź�������ѹָʾ��˸16���ر�ϵͳ
              {                   
                g_u8TimerDelay = 0;
                while(g_u8TimerDelay<50);
                LowPowerLedDisable();
                
                g_u8TimerDelay = 0;
                while(g_u8TimerDelay<50);
                LowPowerLedEnable();
              }
              
              SystemPowerOff();
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
#endif

/*****************************************************************************
 �� �� ��  : PORT1_ISR
 ��������  : P1�˿ڵ��жϷ�����

  ��  ��  : NONE
 �� �� ֵ : NONE
*****************************************************************************/
BOOL bLedFlag=FALSE;
SINT8 s8LedDog;
_tsTimeCb sTime;

#if KEY_SCAN_EN > 0
  BOOL  bKeyFlag = FALSE;
  BOOL  bKeyCompleted = FALSE;
  SINT8 s8KeyDog;
  SINT8 s8KeySetDog;
  BOOL  bKeyWakeUp;
  UINT8 u8KeySet = 1;
#endif

#pragma vector = PORT1_VECTOR
__interrupt void PORT1_ISR(void)
{
#if KEY_SCAN_EN > 0  
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
#endif
  
  if( RtcIntCheck() )
  {
    RtcIntFlagClr();
    
    sRecordCb.u32SysRunTime++;
    
    //�˴���ʱ�Ƚϴ󣬵��ǻ���RTC�жϻ��ƣ��ֱ����жϺ���7.81ms�ٴ�����ж�
    GetTimeFromRtc(&sTime);
    if(g_bStandby == TRUE)
    {      
      if(((sTime.Minute + 1) % sSendInfo.u8Minute) == 0) //��ǰһ���ӻ���
      {
            LPM3_EXIT;
      }
    }
    else
    {  
      if(g_bGpsCompleted == TRUE)
      {
        if((sTime.Minute % sSendInfo.u8Minute) == 0)       //��ʱ����
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

  if(g_bCdmaSend == TRUE)
  {
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
     
     //��RTC�ⲿ�ж�
      SetRtcInt1Out();
      
#if KEY_SCAN_EN > 0
      //�򿪰����жϻ��ѹ���
      bKeyWakeUp = TRUE;
#endif
      
#ifdef BEIDOU_TEST
      u8BdCentage = (UINT32)u16BdRxNum * 100 / u16BdSendNum;
      
      SaveBdRxNbr(&u16BdRxNum);
      SaveBdSendNbr(&u16BdSendNum);
      SaveBdSendSucceedNbr(&u16BdSendSucceed);
#else     
     g_u16TimerDelay=0;
     while(g_u16TimerDelay < 500);  //��ʱ5S, �ȴ���Ϣ��ȫ���͡�
#endif

      if(g_bCdmaSend == TRUE)
        CdmaDtrSetHigh();     //����DTR����CDMA���������();
      else
        CdmaPowerOff();
      
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
      InitClk(0);
           
      LpExitCfg();
//      EepromPowerOn();
      
      //����ADC
      ADC_START_DETECT();
      //����Timer
      TimerInterruptEnable(); 
      //����Uart
      
      g_bStandby = FALSE;
      
#if KEY_SCAN_EN > 0
      bKeyWakeUp = FALSE;
#endif
      
      SystemRunEnable();
      Uart2EnableInterrupt();
      GpsPowerOn();
      
      //ÿ�λ��ѱ���һ������ʱ�䡣
      SaveSystemRunTime(&sRecordCb.u32SysRunTime);
      
      SwdEnable(sSendInfo.sDog);     //��������ͨѶģ�鶨ʱ��
      SwdReset(sSendInfo.sDog);
      
      //ʹ��GPS���ճ�ʱ��ʱ��
      SwdReset(g_s8GpsFirstDog);
      SwdEnable(g_s8GpsFirstDog);    
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
  
#if KEY_SCAN_EN > 0
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
  
  //�򿪿��������ж�ʹ��
  KeyPowerIeEnable();
#endif
  
  LowPowerLedDisable();
  
  //���ж�ʹ��
  RtcIntIeEnable();
  
  if(g_bCdmaSend == TRUE)
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
#if KEY_SCAN_EN > 0
  BOOL bFlag = FALSE;
    
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
#endif  
  
   if(SwdGetLifeTime(s8LedDog) == 0)
  {
    SwdReset(s8LedDog);
    bLedFlag = ~bLedFlag;
        
    if(bLedFlag == 0)
      SystemRunEnable();
    else
      SystemRunDisable();
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
  if(g_bCdmaSend == TRUE)
    UART0_PIN_SET();
  else
    UART0_PIN_CLR();
  
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
  
   if(g_bCdmaSend == TRUE)
   {
//      CdmaIcCheckDir |=  CdmaIcCheck;
//      CdmaIcCheckOut &= ~CdmaIcCheck;
      
      CdmaOnOffDir  |=  CdmaOnOff;
      CdmaOnOffOut  |=  CdmaOnOff;
      
      CdmaResetDir  |=  CdmaReset;
      CdmaResetOut  |=  CdmaReset;
   }
  else
  {
    CdmaOnOffDir  |=  CdmaOnOff;
    CdmaOnOffOut  &= ~CdmaOnOff;
    
    CdmaResetDir  |=  CdmaReset;
    CdmaResetOut  &= ~CdmaReset;
      //CDMA RI�˿�
    CdmaRiDir |=  CdmaRi;
    CdmaRiOut &= ~CdmaRi;
  }
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

