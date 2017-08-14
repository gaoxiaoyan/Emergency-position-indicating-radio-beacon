/*****************************************************************************************************
*											
* Copyright (C) 2015, Sunic-ocean
* All Rights Reserved										
*
* �� �� ���� Main.c
* ��    ����

=====================================================================================================
                                    �汾��¼
=====================================================================================================
* �������ڣ� 2015��4��10��14:23:50
* ��    �ߣ� Bob
* ��ǰ�汾�� V1.1
* ��    ���� 1�������°�ʾλ������󣬿�����ʾ�����֣���DTU�����紫��ģʽ��Ϊ�����Ͷ��ŵ�˫ģͨѶ��
             2����ʾ����ʹ��4λLED�ֱ�ָʾϵͳ���У���ѹ�������豸����ָʾ��Χ���� or ���ͻ�����
    

*****************************************************************************************************/
#define  MAIN_GLOBAL
#include "msp430x54x.h"
#include "BatteryDetect.h"
#include "global.h"
#include "DataType.h"
#include "SystemCtl.h"
#include "Timer.h"
#include "SoftTimer.h"
#include "Uart0.h"
#include "Uart1.h"
#include "Uart2.h"
#include "Uart3.h"
#include "GpsParse.h"
#include "OS_MEM.h"
#include "Rtc.h"
#include "CdmaModule.h"
#include "Communication.h"
#include "DebugCmd.h"
#include "Record.h"
#include "BeiDouModule.h"

/*
*********************************************************************************************************
*                                              MAIN FUNCTION
*********************************************************************************************************
*/
void main(void)
{
    
    StopWatchDog();           // �رտ��Ź�
    
    CPU_ENTER_CRITICAL();

    InitClk();                //��ʼ��ʱ�ӣ�DCO��Ƶ��4Mhz
    
    SystemInit();             //�˿ڳ�ʼ��
   
    TimerA_Init();            //��ʱ��A0��ʼ��
    
    Uart3Init();              //UART3��ʼ��

    CPU_EXIT_CRITICAL();
    
    Adc_Init();               //��AD���
    
    WaitSystemStart();        //�ȴ���������
    
    DebugCmdInit();
    
    Uart0Init();              //����0��ʼ��
    CdmaInit();
    
    Uart1Init();              //����1��ʼ��
    BdModuleInit();          
    
    Uart2Init();              //����2��ʼ��
    GpsCmdInit();
    
    CommunicationInit();      //ͨѶ���ݽṹ��ʼ��
    
    RtcInit();                //RTC��ʼ��
    
    RecordInit();             //���ü�¼��ʼ��
    
    SystemIdleInit();         //�������г�ʼ��
    
    while(TRUE)
    {     
      FreeWatchDog();          //�ͷſ��Ź�
 
      GpsCmdRun();            //GPS���ݻ�ȡ
      
      BatteryDetect();        //AD�������
      
      SendInfoToCenter();     //�������ݵ�����
      
      SystemIdleRun();        //ϵͳ��������
      
      BdCmdRun();             //�����������
      
      EnterLowPower();        //����͹��ģ��ȴ�����
      
      DebugCmdRun();          //���Լ�������Ϣ����
          
      RecordRun();            //��������         
    }

}








