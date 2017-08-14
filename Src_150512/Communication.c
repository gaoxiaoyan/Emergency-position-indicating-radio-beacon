
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
#include "BeiDouModule.h"

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
  sSendInfo.u8GapTime =  1;      //Ĭ��Ϊ5���ӷ��ͼ��
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
	
  if(TRUE == g_bGpsCompleted)
  {
    g_bGpsCompleted = FALSE;
		
		ptr = OSMemGet(pSmallMem, &err);
    memset(ptr, 0, BUF_SMALL_SIZE);

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
		ptr[i++] = sSendInfo.u8GapTime + '0';		// 1 ~6����5�~30����
		ptr[i++] = ',';
    ptr[i++] = sSendInfo.u8Battery + '0'; //��ص�����Ϣ
/*
    ptr[i++] = 3 + '0';
    ptr[i++] = 2 + '0';
    ptr[i++] = 7 + '0';
    ptr[i++] = 6 + '0';
    ptr[i++] = 8 + '0';
    ptr[i++] = ',';
    memcpy(&ptr[i], "2304.12030", 10);
    i += 10;
    ptr[i++] = ',';
    memcpy(&ptr[i], "12034.65078", 11);
    i += 11;
    ptr[i++] = ',';
		ptr[i++] = 1 + '0';		// 1 ~6����5�~30����
		ptr[i++] = ',';
    ptr[i++] = 5 + '0'; //��ص�����Ϣ
*/
//    g_bCdmaSend = FALSE;
      
		if(g_bCdmaSend == TRUE)
		{
    	AssembleCdmaMsg(&sRecordCb.au8CdmaRevNbr[0], ptr, i);
//    	AssembleCdmaMsg(&sRecordCb.au8CdmaBckNbr[0],ptr, i);  //�����ֻ��Ž��ջ�
  	}
    else
    {
      if(g_bBdSend == TRUE)
      {
        sRecordCb.au8BdRevNbr[0] = 0x1f;
        sRecordCb.au8BdRevNbr[1] = 0xf9;
        sRecordCb.au8BdRevNbr[2] = 0xe8;
				BdSendCmdTXSQ(&sRecordCb.au8BdRevNbr[0], ptr, i);
			}
    }
		
		OSMemPut(pSmallMem, ptr);  
    g_bStandby = TRUE;
  }
}

