
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
#include "msp430x54x.h"

#include "global.h"
#include "DataType.h"
#include "Timer.h"
#include "SystemCtl.h"
#include "string.h"
#include "Uart3.h"
#include "SoftWatchDog.h"

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
 *��������: SystemVersionInfo
 *��������: ��ӡϵͳ�汾��Ϣ

 *����: 
 *����:
*****************************************************************************/
void SystemVersionInfo(void)
{
  UINT8 au8Version[8];

  if(g_bDebug == TRUE)
  {
    Uart3SendString("\r\n #Debug HandShake OK! The Bandrate is 115200bps.\r\n");
    Uart3SendString("/***********************************************************/\r\n");
    Uart3SendString("// Copyright (C) 2015, Sunic-ocean.\r\n");
    Uart3SendString("// All Rights Reserved.\r\n");
    Uart3SendString(" \r\n");
    Uart3SendString("// �������ƣ�Ӧ������ʾλ��.\r\n");
      
    memset(&au8Version[0], 0, 8);
    au8Version[0] = 'V';
    au8Version[1] = VERSION / 100 + '0';
    au8Version[2] = '.';
    au8Version[3] = VERSION % 100 / 10 + '0';
    au8Version[4] = VERSION % 10 + '0';
    au8Version[5] = '\r';
  
    Uart3SendString("// �汾��Ϣ��");
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
 	KeyPowerIeEnable();
 	KeyPowerDir &= ~KeyPower;
 	
 	//RTC�жϣ������ش����ж�
 	RtcIntIesEdge();
 	RtcIntIeEnable();
 	RtcIntDir &= ~RtcInt;
 	
 	//CDMA RI�жϣ��½��ش����ж�
 	CdmaRiIesEdge();
 	CdmaRiIntEnable();
 	CdmaRiDir &= ~CdmaRi;
 	
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
  SwdSetLifeTime(s8Dog, 300);
  SwdEnable(s8Dog);
  
  while(TRUE)
  {
    if(!KeyPowerCheck)
    {
      if(SwdGetLifeTime(s8Dog) == 0)
      {
        SwdDisable(s8Dog);
        SwdFree(s8Dog);
        
        SystemPowerOn();
        GpsPowerOn();
        CdmaPowerOn();
        BeiDouPowerOn();
        
        return;
      }
      else
      {
        SystemPowerOff();
      }
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
  static UINT8 u8Temp=0;
  
  u8Temp++;
}

/****************************************************************
 * �������ƣ�MEM_SET
 * ��    �ܣ��ڴ�����Ϊ�̶�ֵ
 *
 * ��    ����UINT8 *pu8Buf, UINT8 u8Set, UINT8 u8Len
 * ����ֵ  ����
****************************************************************/
void MEM_SET(void *pu8Buf, UINT8 u8Set, UINT8 u8Len)
{
  UINT8 *p = (UINT8 *)pu8Buf;
  
	while(u8Len--)
	{
		*p++ = u8Set;
	}
}

/****************************************************************
 * �������ƣ�MEM_CPY
 * ��    �ܣ���BUF2�����ݸ��Ƶ�BUF1��
 *
 * ��    ����UINT8 *pu8Buf1, UINT8 *pu8Buf2, UINT8 u8Len
 * ����ֵ  ����
****************************************************************/
void MEM_CPY(void *pu8Buf1, const void *pu8Buf2, UINT8 u8Len)
{
  void *dst = pu8Buf1;
  
  while(u8Len-- > 0)
  {
    *(UINT8 *)dst = *(UINT8 *)pu8Buf2;
     dst = (UINT8 *)dst + 1;
     pu8Buf2 = (UINT8 *)pu8Buf2 + 1;
  }

}

/****************************************************************
 * �������ƣ�MEM_CMP
 * ��    �ܣ��ڴ�Ƚϡ�
 *
 * ��    ����const void *cs, const void *ct, UINT8 count
 * ����ֵ  ��SINT8
****************************************************************/
SINT8 MEM_CMP(const void *cs, const void *ct, UINT8 count)
{
  const UINT8 *su1, *su2;
  SINT8 res = 0;
        
  for (su1 = cs, su2 = ct; 0 < count; ++su1, ++su2, count--)
  {
    if ((res = *su1 - *su2) != 0)
    {
      break;
    }
  }
  return res;
}


