/******************************************************************************
*											
* Copyright (C) 2015, Sunic-ocean
* All Rights Reserved										
*
* �� �� ���� Main.c
* ��    ����

===============================================================================
                                    �汾��¼
================================================================================
* �������ڣ� 2015��4��10��14:23:50
* ��    �ߣ� Bob
* ��ǰ�汾�� V1.1
* ��    ���� 1�������°�ʾλ������󣬿�����ʾ�����֣���DTU�����紫��ģʽ��Ϊ�����Ͷ��ŵ�˫ģͨѶ��
             2����ʾ����ʹ��4λLED�ֱ�ָʾϵͳ���У���ѹ�������豸����ָʾ��Χ���� or ���ͻ�����
    

*******************************************************************************/

#include "msp430x54x.h"

#include "global.h"
#include "DataType.h"
#include "SystemCtl.h"
#include "Timer.h"
#include "Uart3.h"
#include "SoftTimer.h"
#include "Uart2.h"
#include "GpsParse.h"
#include "OS_MEM.h"

/*
*********************************************************************************************************
*                                              GLOBALS PARAM
*********************************************************************************************************
*/

//������Ϣ����
BOOL g_bDebug=TRUE;

volatile UINT8  g_u8TimerDelay;
volatile UINT16 g_u16TimerDelay;
volatile UINT32 g_u32TimeStamp;

/*========================== ������ ===================================*/
void main(void)
{
  SINT8 s8LedDog=0;
  BOOL bFlag=0;
  
    StopWatchDog();           // �رտ��Ź�
    
    CPU_ENTER_CRITICAL();

    InitClk();                //��ʼ��ʱ�ӣ�DCO��Ƶ��4Mhz
      
    OS_MemInit();             //�ڴ��ʼ��
    
    SystemInit();             //�˿ڳ�ʼ��
   
    TimerA_Init();            //��ʱ��A0��ʼ��
    
    Uart3Init();              //UART3��ʼ��

    CPU_EXIT_CRITICAL();
    
    WaitSystemStart();        //�ȴ���������
    
    Uart2Init();              //����2��ʼ��
    
    GpsCmdInit();
    
    s8LedDog = SwdGet();
    SwdSetLifeTime(s8LedDog, 500);
    SwdEnable(s8LedDog);
    
//    SystemIdleInit();         //ͳ�������ʼ��
      
    while(TRUE)
    {     
      FreeWatchDog();          //�ͷſ��Ź�
 
      if(SwdGetLifeTime(s8LedDog) == 0)
      {
        SwdReset(s8LedDog);
        bFlag = ~bFlag;
        
        if(bFlag == 0)
        {
          SystemRunEnable();
        }
        else
        {
          SystemRunDisable();
        }
      }
      
      GpsCmdRun();
      
    }

}









