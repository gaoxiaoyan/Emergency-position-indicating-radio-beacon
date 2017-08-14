
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

#define CPU_CLK       4194304ul 
#define FLL_FACTOR    (CPU_CLK / 32768ul)


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
  
  g_bCdmaSend = FALSE;
  g_bBdSend   = FALSE;
  
  OS_MemInit();         
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

  s8Dog = SwdGet();
  SwdSetLifeTime(s8Dog, 3000);
//  SwdEnable(s8Dog);

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
          
          //�򿪰����ж�
          KeyPowerIeEnable();
          
          BeiDouPowerOn();
          SystemPowerOn();
          GpsPowerOn();
          CdmaPowerOn();
          
          //�򿪳�������ָʾ�ƣ�˵��ϵͳ�Ѿ�������
          SystemRunEnable();
          
          if(g_bDebug == TRUE)
         {
  //         SystemVersionInfo();
  //         PrintfCmdList();
           Uart3SendString("# System Power On......  \r\n");
           Uart3SendString("# Turn On Gps Power......  \r\n");
           Uart3SendString("# Turn On Cdma Power......  \r\n");
           Uart3SendString("# Turn On BeiDou Power.....  \r\n");
           Uart3SendString("# Initialize System..... \r\n");
         }
          
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
#pragma vector = PORT1_VECTOR
__interrupt void PORT1_ISR(void)
{
  static UINT8 u8KeySet = 1;
  _tsTimeCb sTime;
  
  if( RtcIntCheck() )
  {
    RtcIntFlagClr();
    SetRtcInt1Out();
    //�˴���ʱ�Ƚϴ���Ҫ�Ľ�
    if(g_bStandby == TRUE)
    {
      GetTimeFromRtc(&sTime);
      if(((sTime.Minute + 2) % (sSendInfo.u8GapTime * 5)) == 0) //��ǰ�����ӻ���
      {
         LPM1_EXIT;
      }
    }
  }
  
  if(KeyPowerIntCheck())
  {
    KeyPowerIntFlagClr();
    bKeyFlag = TRUE;
    
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
     
     g_u16TimerDelay=0;
     while(g_u16TimerDelay < 500);  //��ʱ5S, �ȴ���Ϣ��ȫ���͡�
  
      GpsPowerOff();
      CdmaPowerOff();
		  BeiDouPowerOff();
      SystemRunDisable();
      
      WylLedDisable();
      SyjLedDisable();
      
      //�������
      //��RTC�����ж�
      LPM1;
      
      g_bStandby = FALSE;
      
      SystemRunEnable();
      Uart2EnableInterrupt();
      GpsPowerOn();
      CdmaPowerOn();
		  BeiDouPowerOn();
      
      CdmaInit();
      
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
      
      //��������
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


