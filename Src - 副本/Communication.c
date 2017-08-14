
/******************************************************************************
*
* Copyright (C) 2015, Sunic-ocean
* All Rights Reserved
*
* �� �� ���� CommunicationModule.C
* ��    ���� ͨѶģ�飬����ȡ��GPS������λ��Ϣ�������������ݸ�ʽ
             ��ȡ��ǰCDMA���ź�ǿ�ȣ����ﵽ����ǿ�ȣ�ʹ��CDMAģ�鷢����Ϣ������
             ����ʹ�ñ���ģ�鷢�ͱ��ģ��ص�CDMAģ�顣

//-----------------------------------------------------------------------------
* �޸����ڣ�2015��10��30��17:03:09
* ��    �ߣ�Bob
* �޸�������1���޸ķ��͸�ʽ�����ӱ�γ�Ͷ���ָʾ����֮ǰ�ġ�#����������Ϊ'&'

//-----------------------------------------------------------------------------
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
#include "Rtc.h"
#include "BeiDouModule.h"

SINT8 s8TestDog=0;

/*****************************************************************************
 �� �� ��  : CommunicationInit
 ��������  : �������ݽṹ��ʼ��

 �������  : none
 �� �� ֵ  : none,
*****************************************************************************/
void CommunicationInit(void)
{
//  UINT8 u8Temp=0;

  memset(&sSendInfo.au8GPS_Y[0], '0', 10);
  memset(&sSendInfo.au8GPS_X[0], '0', 11);
  sSendInfo.u8Battery = 0;
//  sSendInfo.u8GapTime =  1;      //Ĭ��Ϊ5���ӷ��ͼ��

  sSendInfo.u8Minute  = (sRecordCb.u8GapTime - '0') * 5;
  sSendInfo.u8GapTime = sRecordCb.u8GapTime;

  sSendInfo.sDog = SwdGet();
  SwdSetName(sSendInfo.sDog, "OpenModuleTimer");
  SwdSetLifeTime(sSendInfo.sDog, 40000);
  SwdEnable(sSendInfo.sDog);     //��������ͨѶģ�鶨ʱ��

  s8TestDog = SwdGet();
  SwdSetName(s8TestDog, "TestTimer");
  SwdSetLifeTime(s8TestDog, 5000);

  u16BdSendNum = 0;  //�������ͱ��Ĵ���
  u16BdRxNum   = 0;    //�������ձ��Ĵ���
  u8BdCentage  = 0;      //�ɹ���
}


/*****************************************************************************
 �� �� ��  : SendInfoToCenter
 ��������  : �������ݵ����ģ�

 �������  : none
 �� �� ֵ  : none,
*****************************************************************************/
#define RtcHex2Bcd(u8Hex) ((((u8Hex % 100) / 10) << 4) + ((u8Hex % 100) % 10))
void SendInfoToCenter(void)
{
	UINT8 *ptr;
  UINT8 i;
//  _tsTimeCb sCurTime;

#if 0
  UINT8 au8Y[10]    = {0x31,0x32,0x33,0x34,0x35,0x31,0x32,0x33,0x34,0x35};
  UINT8 au8X[11]    = {0x35,0x34,0x33,0x32,0x31,0x35,0x34,0x33,0x32,0x31,0x30};

  g_bSendInfo       = TRUE;
  g_bGpsCompleted   = TRUE;
  g_bBdSend         = FALSE;
  g_bCdmaSend       = TRUE;

  memcpy(&sSendInfo.au8GPS_Y[0], &au8Y[0], 10);
  memcpy(&sSendInfo.au8GPS_X[0], &au8X[0], 11);

  StopWatchDog();
#endif

  if(g_bSendInfo == TRUE)
  {
    if(TRUE == g_bGpsCompleted)
    {
      g_bSendInfo     = FALSE;
      g_bGpsCompleted = FALSE;

      //��ȡһ���ڴ�ռ�
      ptr = OSMemGet(pSmallMem, &err);

      //�����������
      i=0;
      ptr[i++] = '#';                        //��Ϣ��ʼ��
      ptr[i++] = ',';

      if((sSendInfo.u8MsgSource & AUTO_COM_MODE) == BD_COM_MODE)
      {
        //���ֻʹ�ñ���ͨѶ��ʽ�����ͱ��Ϊ��������
        memcpy(&ptr[i], &sSendInfo.au8BDIndex[0], 11);
      }
      else
      {
        memcpy(&ptr[i], &sSendInfo.au8CdmaIndex[0], 11);
      }
      i += 11;

      ptr[i++] = ',';
      ptr[i++] = sRecordCb.u8SysType % 100 / 10 + '0';
      ptr[i++] = sRecordCb.u8SysType % 10 + '0';

      ptr[i++] = ',';
      //�˴���ʱ�Ƚϴ󣬶�ȡ��ʱ�䲹��һ��
      GetTimeFromRtc(&sCurTime);
      ptr[i++] = sCurTime.Hour / 10 + '0';
      ptr[i++] = sCurTime.Hour % 10 + '0';
      ptr[i++] = ':';
      ptr[i++] = sCurTime.Minute / 10 + '0';
      ptr[i++] = sCurTime.Minute % 10 + '0';
      ptr[i++] = ':';
      ptr[i++] = sCurTime.Second / 10 + '0';
      ptr[i++] = sCurTime.Second % 10 + '0';

      ptr[i++] = ',';
      memcpy(&ptr[i], &sSendInfo.au8GPS_Y[0], 10);
      i += 10;
      ptr[i++] = ',';
      ptr[i++] = 'N';
      ptr[i++] = ',';
      memcpy(&ptr[i], &sSendInfo.au8GPS_X[0], 11);
      i += 11;
      ptr[i++] = ',';
      ptr[i++] = 'E';
      ptr[i++] = ',';
      ptr[i++] = (sSendInfo.u8Battery%100)/10 + '0'; //��ص�����Ϣ
      ptr[i++] = sSendInfo.u8Battery%10 + '0'; //��ص�����Ϣ
      ptr[i++] = ',';

      if(g_bCdmaSend == TRUE)
      {  
        ptr[i++] = '2';
      }
      else
      {
         if(g_bBdSend == TRUE)  ptr[i++] = '1';
      }

      ptr[i++] = ',';
      ptr[i++] = '&';                       //��Ϣ������, ����ΪĿǰ��

      if(g_bCdmaSend == TRUE)
      {
        AssembleCdmaMsg(&sRecordCb.au8CdmaRevNbr[0], ptr, i);
      }
      else
      {
        if(g_bBdSend == TRUE)
        {
          GetBeiDouIcNum(&sRecordCb.au8BdRevNbr[0]);
//          sRecordCb.au8BdRevNbr[0] = 0x1f;
//          sRecordCb.au8BdRevNbr[1] = 0xf9;
//          sRecordCb.au8BdRevNbr[2] = 0xe8;
          BdSendCmdTXSQ(&sRecordCb.au8BdRevNbr[0], ptr, i);

#ifdef BEIDOU_TEST
          u16BdSendNum++;
          memset(&au8BdSendInfo[0], 0, sizeof(au8BdSendInfo));
          memcpy(&au8BdSendInfo[0], ptr, i);
#endif
        }
      }

      //ʹ����ϣ��ͷŸ��ڴ�
      OSMemPut(pSmallMem, ptr);

       SwdReset(s8TestDog);
       SwdEnable(s8TestDog);
    }
  }

  if(SwdGetLifeTime(s8TestDog) == 0)
  {
    SwdDisable(s8TestDog);
    SwdReset(s8TestDog);

    g_bStandby = TRUE;
  }

  //��ǰһ���ӻ���ͨѶģ�飬��CDMAͨѶʱ���յ�����ֱ�ӻ���
//  if((g_bCommuWake == TRUE) || (g_bCdmaRiEvent == TRUE))
  if(SwdGetLifeTime(sSendInfo.sDog) == 0)
  {
//    g_bCommuWake   = FALSE;
//    g_bCdmaRiEvent = FALSE;

    SwdDisable(sSendInfo.sDog);
    SwdReset(sSendInfo.sDog);

    if(g_bCdmaSend == TRUE)
    {
      CdmaWakeUpCfg();
//      AssembleCdmaMsg("18561795199", "This Is A Test Massage!", 23);
    }
    else
    {
      if(g_bBdSend == TRUE)
      {
        BeiDouPowerOn();
      }
    }
  }
}

