
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
#include "SoftTimer.h"
#include "CdmaModule.h"
#include "Rtc.h"

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
 *��������: GetSystemVersionInfo
 *��������: ��ӡϵͳ�汾��Ϣ

 *����: 
 *����:
*****************************************************************************/
void GetSystemVersionInfo(void)
{
  UINT8 au8Version[8];

//  if(g_bDebug == TRUE)
  {
    Uart3SendString("\r\n#Debug HandShake OK! The Bandrate is 115200bps.\r\n");
    Uart3SendString("/***********************************************************/\r\n");
    Uart3SendString("// Copyright (C) 2015, Sunic-ocean.\r\n");
    Uart3SendString("// All Rights Reserved.\r\n");
    Uart3SendString(" \r\n");
    Uart3SendString("// Project Name��Ӧ������ʾλ��.\r\n");
      
    memset(&au8Version[0], 0, 8);
    au8Version[0] = 'V';
    au8Version[1] = VERSION / 100 + '0';
    au8Version[2] = '.';
    au8Version[3] = VERSION % 100 / 10 + '0';
    au8Version[4] = VERSION % 10 + '0';
    au8Version[5] = '\r';
  
    Uart3SendString("// Version��");
    Uart3SendString(&au8Version[0]);
    Uart3SendString("/***********************************************************/\r\n");
    Uart3SendString(" \r\n");
  }
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
 	
 	//������Դ���ƽӿ�
 	SystemPowerOff();
 	SystemPowerDir |= SystemPower;
  
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
          
          SystemPowerOn();
          GpsPowerOn();
          CdmaPowerOn();
          
          
          //�򿪳�������ָʾ�ƣ�˵��ϵͳ�Ѿ�������
          SystemRunEnable();
          
          if(g_bDebug == TRUE)
         {
  //         SystemVersionInfo();
  //         PrintfCmdList();
           Uart3SendString("#System Power On......  \r\n");
           Uart3SendString("#Turn On Gps Power......  \r\n");
           Uart3SendString("#Turn On Cdma Power......  \r\n");
           Uart3SendString("#Turn On BeiDou Power.....  \r\n");
           Uart3SendString("#Initialize System..... \r\n");
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
#pragma vector = PORT1_VECTOR
__interrupt void PORT1_ISR(void)
{
  if( RtcIntCheck() )
  {
    RtcIntFlagClr();
    SetRtcInt1Out();
  }
  
  if(KeyPowerIntCheck())
  {
    KeyPowerIntFlagClr();
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
      g_bStandby = FALSE;
      
      //�������
      //��RTC�����ж�
      
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
BOOL bFlag=FALSE;
SINT8 s8LedDog;
SINT8 s8KeyDog;
void SystemIdleInit(void)
{
  //��ʼ����������ָʾ�ƶ�ʱ��
  s8LedDog = SwdGet();
  SwdSetLifeTime(s8LedDog, 500);
  SwdEnable(s8LedDog);  
  
  //��ʼ���ػ�������ʱ��
  s8KeyDog = SwdGet();
  SwdSetLifeTime(s8KeyDog, 3000);
  SwdDisable(s8KeyDog);  
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
  if(SwdGetLifeTime(s8LedDog) == 0)
  {
    SwdReset(s8LedDog);
    bFlag = ~bFlag;
        
    if(bFlag == 0)
      SystemRunEnable();
    else
      SystemRunDisable();
  }
  
  if( !KeyPowerCheck)
  {
    if(SwdIsEnable(s8KeyDog) == TRUE)
    {
      if(SwdGetLifeTime(s8KeyDog) == 0)
      {
        SwdReset(s8KeyDog);
        
        //�ر�LEDָʾ
        SystemRunDisable();
        
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
//�������ƣ�PrintfCmdList
//������������ӡ�����б�
//
//��������
//���أ���
***************************************************************************/
void PrintfCmdList(void)
{
  Uart3SendString("###############################################################\r\n");
  Uart3SendString("#            Ӧ������ʾλ���ڲ�����ָ��       \r\n");
  Uart3SendString("#==============================================================\r\n");
  Uart3SendString("#  0�������ʽ��AA + 55 + cmd + CC + 33.  \r\n");
  Uart3SendString("#  1��cmd<00> -- ����ͨѶ�������\r\n");
  Uart3SendString("#  2��cmd<01> -- ��ȡϵͳ����汾��Ϣ��\r\n");
  Uart3SendString("#  3��cmd<10> -- GPS������Ϣ�رգ�\r\n");
  Uart3SendString("#  4��cmd<11> -- GPS������Ϣ�򿪣�\r\n");
  Uart3SendString("#  5��cmd<20> -- BeiDou������Ϣ�رգ�\r\n");
  Uart3SendString("#  6��cmd<21> -- BeiDou������Ϣ�򿪣�\r\n");
  Uart3SendString("#  7��cmd<22> -- BeiDou���ͱ��Ĳ��ԣ�\r\n");
  Uart3SendString("#  8��cmd<30> -- CDMA������Ϣ�رգ�\r\n");
  Uart3SendString("#  9��cmd<31> -- CDMA������Ϣ�򿪣�\r\n");
  Uart3SendString("# 10��cmd<32> -- ��ȡCDMAʱ�䣻\r\n");
  Uart3SendString("# 11��cmd<33> -- ��CDMAʱ��У׼RTC��\r\n");
  Uart3SendString("# 12��cmd<34> -- ��ȡ�������룻\r\n");
  Uart3SendString("# 13��cmd<35> -- ���ý��պ��룻\r\n");
  Uart3SendString("# 14��cmd<35> -- ���ý��պ��룻\r\n");
  Uart3SendString("# 15��cmd<40> -- ����ͨѶ���ʱ�䣻\r\n");
  Uart3SendString("# 16��cmd<50> -- ��ȡ��ǰ���ʣ�������\r\n");
  Uart3SendString("###############################################################\r\n");
}

