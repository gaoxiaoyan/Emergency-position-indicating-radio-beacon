
/******************************************************************************
*											
* Copyright (C) 2015, Sunic-ocean
* All Rights Reserved										
*
* �� �� ���� CommunicationModule.C
* ��    ���� ͨѶģ�飬����ȡ��GPS������λ��Ϣ�������������ݸ�ʽ
             ��ȡ��ǰCDMA���ź�ǿ�ȣ����ﵽ����ǿ�ȣ�ʹ��CDMAģ�鷢����Ϣ������
             ����ʹ�ñ���ģ�鷢�ͱ��ģ��ص�CDMAģ�顣

* �������ڣ� 2015��4��27��14:51:26
* ��    �ߣ� Bob
* ��ǰ�汾�� V1.00

------------------------------------------------------
*******************************************************************************/
#define  COMMU_GLOBAL
#include "msp430x54x.h"
#include "global.h"
#include "Communication.h"
#include "DataType.h"
#include "CdmaModule.h"
#include "SystemCtl.h"
#include "string.h"
#include "Timer.h"
#include "SoftTimer.h"
#include "GpsParse.h"
#include "OS_MEM.h"
#include "Uart3.h"
#include "Record.h"

BOOL bCdmaSend=FALSE;
BOOL bBeiDouSend=FALSE;

/*****************************************************************************
 �� �� ��  : CommunicationInit
 ��������  : �������ݽṹ��ʼ��

 �������  : none
 �� �� ֵ  : none,
*****************************************************************************/
void CommunicationInit(void)
{
  sSendInfo.u8Index = 0;
  sSendInfo.pGPS_Y = NULL;
  sSendInfo.pGPS_X = NULL;
  sSendInfo.u8Battery = 0; 
}

/*****************************************************************************
 �� �� ��  : SendInfoToCenter
 ��������  : �������ݵ����ģ�

 �������  : none
 �� �� ֵ  : none,
*****************************************************************************/
void SendInfoToCenter(void)
{
  UINT8 *ptr;
  UINT8 i;
 // UINT8 u8Check=0;

  //if( TRUE == g_bOnTimeSend)
  if( TRUE == bCdmaSend )
  {
    bCdmaSend = FALSE;
    
    ptr = OSMemGet(pSmallMem, &err);
    memset(ptr, 0, BUF_SMALL_SIZE);
    
    if(TRUE == g_bDebug)
    {
      Uart3SendString("CDMA�ź�������׼����������!\r\n");
    }
    
    //�����������
    i=0;
    ptr[i++] = '$';
    
    ptr[i++] = sRecordCb.u16SysNbr/10000 + '0';
    ptr[i++] = sRecordCb.u16SysNbr%10000/1000 + '0';
    ptr[i++] = sRecordCb.u16SysNbr%1000/100 + '0';
    ptr[i++] = sRecordCb.u16SysNbr%100/10 + '0';
    ptr[i++] = sRecordCb.u16SysNbr%10 + '0';
    ptr[i++] = ',';
    memcpy(&ptr[i], sSendInfo.pGPS_Y, 10);
    i += 10;
    ptr[i++] = ',';
    memcpy(&ptr[i], sSendInfo.pGPS_X, 11);
    i += 11;
    ptr[i++] = ',';
    ptr[i++] = '9'; //��ص�����Ϣ
    ptr[i++] = '\r';

    AssembleCdmaMsg(&sRecordCb.au8CdmaRevNbr[0],ptr, i);
//    AssembleCdmaMsg(&sRecordCb.au8CdmaBckNbr[0],ptr, i);  //�����ֻ��Ž��ջ�
    
    OSMemPut(pSmallMem, ptr);
    
    g_bStandby = TRUE;
  }
  else
  {
    if( TRUE == bBeiDouSend )
    { 
       bBeiDouSend = FALSE;
       
       g_bStandby = TRUE;
    }
  }
}


/****************************************************************
 * �������ƣ�ComModuleSwitch
 * ��    �ܣ��ж�CDMAģ���ź�ǿ�ȣ������㷢��Ҫ����ʹ��CDMA���ͣ�
             ����򿪱���ģ��
 *
 * ��    ������
 * ����ֵ  ����
****************************************************************/
void ComModuleSwitch(void)
{
  if(TRUE == g_bGpsCompleted)
  {
    g_bGpsCompleted = FALSE;
    
    if( GetCdmaCSQ() > 16 )
    {
      bCdmaSend = TRUE;
    }
    else
    {
      //�򿪱���ģ���Դ
      BeiDouPowerOn();
      bBeiDouSend = TRUE;
    }
  }
}