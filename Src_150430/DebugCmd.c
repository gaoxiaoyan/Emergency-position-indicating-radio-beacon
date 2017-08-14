/******************************************************************************
*											
* Copyright (C) 2015, Sunic-ocean
* All Rights Reserved										
*
* �� �� ���� DebugCmd.c
* ��       ����

* �������ڣ� 2015��4��29��16:41:25
* ��    �ߣ� Bob
* ��ǰ�汾�� V1.00

------------------------------------------------------
*******************************************************************************/
#include "msp430x54x.h"
#include "BatteryDetect.h"
#include "global.h"
#include "DataType.h"
#include "DebugCmd.h"
#include "SystemCtl.h"
#include "Uart3.h"
#include "string.h"
#include "SoftTimer.h"
#include "CdmaModule.h"
#include "Rtc.h"

//���ݰ�����Э��
typedef enum
{
  //����֡ͷ
  E_UART_CMD_RECEIVING_HEADER1,
  E_UART_CMD_RECEIVING_HEADER2,
  
  //��������
  E_UART_CMD_RECEIVING_CMD,
  
  //����֡β
  E_UART_CMD_RECEIVING_TAIL1,
  E_UART_CMD_RECEIVING_TAIL2,
  
  //������Ч
  E_UART_CMD_RECEIVING_VALID
		
}teDebugCmdAssembleState;

/* ��������ṹ���� */
typedef struct
{
  UINT8 *pu8AssembleBuf;  //���������ָ��
		
  UINT8 u8BufDataLen ;  //��������ݳ���
	
  teDebugCmdAssembleState eAssembleState;  //���״̬
  
  SINT8 sDebugDog;
	
}_tsDebugCmdCb;

_tsDebugCmdCb sDebugCmdCb;

//����ͨѶ�����б�
typedef void CmdHandler(void);
CmdHandler *apCmdHandlerList[88];

/***************************************************************************
//�������ƣ�PrintfCmdList
//������������ӡ�����б�
//
//��������
//���أ���

 ���ͣ� 0xAA + 0x55 + 0x02 + 0xCC + 0x33
***************************************************************************/
void PrintfCmdList(void)
{
  Uart3SendString("###############################################################\r\n");
  Uart3SendString("#            Ӧ������ʾλ���ڲ�����ָ��       \r\n");
  Uart3SendString("#==============================================================\r\n");
  Uart3SendString("#  0�������ʽ��AA + 55 + cmd + CC + 33.  \r\n");
  Uart3SendString("#  1��cmd<00> -- ����ͨѶ�������\r\n");
  Uart3SendString("#  2��cmd<01> -- ��ȡϵͳ����汾��Ϣ��\r\n");
  Uart3SendString("#  3��cmd<02> -- ��ӡ����ָ���б�\r\n");
  Uart3SendString("#  4��cmd<03> -- �����豸RTCʱ�䣻\r\n");
  Uart3SendString("#  5��cmd<04> -- ��ȡ�豸RTCʱ�䣻\r\n");
  Uart3SendString("#  6��cmd<10> -- GPS������Ϣ�رգ�\r\n");
  Uart3SendString("#  7��cmd<11> -- GPS������Ϣ�򿪣�\r\n");
  Uart3SendString("#  8��cmd<20> -- BeiDou������Ϣ�رգ�\r\n");
  Uart3SendString("#  9��cmd<21> -- BeiDou������Ϣ�򿪣�\r\n");
  Uart3SendString("# 10��cmd<22> -- BeiDou���ͱ��Ĳ��ԣ�\r\n");
  Uart3SendString("# 11��cmd<30> -- CDMA������Ϣ�رգ�\r\n");
  Uart3SendString("# 12��cmd<31> -- CDMA������Ϣ�򿪣�\r\n");
  Uart3SendString("# 13��cmd<32> -- ��ȡCDMAʱ�䣻\r\n");
  Uart3SendString("# 14��cmd<33> -- ���ñ������룻\r\n");
  Uart3SendString("# 15��cmd<34> -- ��ȡ�������룻\r\n");
  Uart3SendString("# 16��cmd<35> -- ���ý��պ��룻\r\n");
  Uart3SendString("# 17��cmd<35> -- ��ȡģ���ͺţ�\r\n");
  Uart3SendString("# 18��cmd<40> -- ����ͨѶ���ʱ�䣻\r\n");
  Uart3SendString("# 19��cmd<50> -- ��ȡ��ǰ���ʣ�������\r\n");
  Uart3SendString("###############################################################\r\n");
}

/*****************************************************************************
 *��������: UartHandShake
 *��������: ������������

 *����: 
 *����:

���ͣ� 0xAA + 0x55 + 0x00 + 0xCC + 0x33
���أ� 
*****************************************************************************/
void UartHandShake(void)
{
  Uart3SendString("#Debug HandShake OK! The Bandrate is 115200bps.\r\n");
}

/*****************************************************************************
 *��������: GetSystemVersionInfo
 *��������: ��ӡϵͳ�汾��Ϣ

 *����: 
 *����:

���ͣ� 0xAA + 0x55 + 0x01 + 0xCC + 0x33
*****************************************************************************/
void GetSystemVersionInfo(void)
{
  UINT8 au8Version[8];

//  if(g_bDebug == TRUE)
  {
    Uart3SendString("###############################################################\r\n");
    Uart3SendString("# Copyright (C) 2015, Sunic-ocean.\r\n");
    Uart3SendString("# All Rights Reserved.\r\n");
    Uart3SendString("# \r\n");
    Uart3SendString("# Project Name��Ӧ������ʾλ��.\r\n");
      
    memset(&au8Version[0], 0, 8);
    au8Version[0] = 'V';
    au8Version[1] = VERSION / 100 + '0';
    au8Version[2] = '.';
    au8Version[3] = VERSION % 100 / 10 + '0';
    au8Version[4] = VERSION % 10 + '0';
    au8Version[5] = '\r';
  
    Uart3SendString("# Version��");
    Uart3SendString(&au8Version[0]);
    Uart3SendString("###############################################################\r\n");
    Uart3SendString(" \r\n");
  }
}


/*****************************************************************************
 *��������: SetRtcTime
 *��������: ����RTCʱ��

 *����: 
 *����:

���ͣ� 0xAA + 0x55 + 0x03 + 0xCC + 0x33
*****************************************************************************/
void SetRtcTime(void)
{
  if( SetCdmaTimeToRtc() )
  {
    Uart3SendString("#Set Rtc Time Successful! \r\n");
  }
  else
  {
    Uart3SendString("#Set Rtc Time Failed! \r\n");
  }
}
/*****************************************************************************
 *��������: GetTimeFromRtc
 *��������: ��ȡRTCʱ��

 *����: 
 *����:

���ͣ� 0xAA + 0x55 + 0x04 + 0xCC + 0x33
*****************************************************************************/
void GetRtcTime(void)
{
  _tsTimeCb sTime;
  UINT8 au8Time[24];
  UINT8 i=0;
  
  GetTimeFromRtc(&sTime);
  au8Time[i++] = '2';
  au8Time[i++] = '0';
  au8Time[i++] = sTime.Year / 10 + '0';
  au8Time[i++] = sTime.Year % 10 + '0';
  au8Time[i++] = '/';
  
  au8Time[i++] = sTime.Month / 10 + '0';
  au8Time[i++] = sTime.Month % 10 + '0';
  au8Time[i++] = '/';
  
  au8Time[i++] = sTime.Date / 10 + '0';
  au8Time[i++] = sTime.Date % 10 + '0';
  au8Time[i++] = ',';
  
  au8Time[i++] = sTime.Week % 10 + '0';
  au8Time[i++] = ',';
  
  au8Time[i++] = sTime.Hour / 10 + '0';
  au8Time[i++] = sTime.Hour % 10 + '0';
  au8Time[i++] = ':';
  
  au8Time[i++] = sTime.Minute / 10 + '0';
  au8Time[i++] = sTime.Minute % 10 + '0';
  au8Time[i++] = ':';
  
  au8Time[i++] = sTime.Second / 10 + '0';
  au8Time[i++] = sTime.Second % 10 + '0';
  au8Time[i++] = '\r';
  au8Time[i++] = '\n';
  
  Uart3SendString("#The Current RTC Time Is: ");
  Uart3SendBuffer(&au8Time[0], i);
}

/*****************************************************************************
 *��������: GpsDebugClose
 *��������: �ر�GPS������Ϣ

 *����: 
 *����:

���ͣ� 0xAA + 0x55 + 0x10 + 0xCC + 0x33
*****************************************************************************/
void CmdGpsDebugClose(void)
{
  g_bDebug &= ~GPS_DEBUG;
}

/*****************************************************************************
 *��������: GpsDebugOpen
 *��������: ��GPS������Ϣ

 *����: 
 *����:

���ͣ� 0xAA + 0x55 + 0x11 + 0xCC + 0x33
*****************************************************************************/
void CmdGpsDebugOpen(void)
{
  g_bDebug |=  GPS_DEBUG;
}


/*****************************************************************************
 *��������: CmdBeiDouDebugClose
 *��������: �ر�BeiDou������Ϣ

 *����: 
 *����:

���ͣ� 0xAA + 0x55 + 0x20 + 0xCC + 0x33
*****************************************************************************/
void CmdBeiDouDebugClose(void)
{
  g_bDebug &= ~BD_DEBUG;
}

/*****************************************************************************
 *��������: CmdBeiDouDebugOpen
 *��������: ��BeiDou������Ϣ

 *����: 
 *����:

���ͣ� 0xAA + 0x55 + 0x21 + 0xCC + 0x33
*****************************************************************************/
void CmdBeiDouDebugOpen(void)
{
  g_bDebug |=  BD_DEBUG;
}

/*****************************************************************************
 *��������: CmdCdmaDebugClose
 *��������: �ر�GPS������Ϣ

 *����: 
 *����:

���ͣ� 0xAA + 0x55 + 0x30 + 0xCC + 0x33
*****************************************************************************/
void CmdCdmaDebugClose(void)
{
  g_bDebug &= ~CDMA_DEBUG;
}

/*****************************************************************************
 *��������: CmdCdmaDebugOpen
 *��������: ��GPS������Ϣ

 *����: 
 *����:

���ͣ� 0xAA + 0x55 + 0x31 + 0xCC + 0x33
*****************************************************************************/
void CmdCdmaDebugOpen(void)
{
  g_bDebug |=  CDMA_DEBUG;
}

/*****************************************************************************
 *��������: CmdGetCdmaTime
 *��������: ��ȡCDMAʱ��

 *����: 
 *����:

���ͣ� 0xAA + 0x55 + 0x32 + 0xCC + 0x33
*****************************************************************************/
void CmdGetCdmaTime(void)
{
  UINT8 au8Time[32];
  UINT8 u8Len;
  
  Uart3SendString("#The Current CDMA Time Is: ");
  GetCdmaTime(&au8Time[0], &u8Len);
  Uart3SendBuffer(&au8Time[0], u8Len);
  Uart3SendString("\r\n");
}

/*****************************************************************************
 *��������: CmdSetCdmaLocationNbr
 *��������: ���ñ�������

 *����: 
 *����:

���ͣ� 0xAA + 0x55 + 0x33 + 0xCC + 0x33
*****************************************************************************/
void CmdSetCdmaLocationNbr(void)
{
  SINT8 s8Dog;
  UINT8 i=0;
  UINT8 au8Nbr[20];
  
  s8Dog = SwdGet();
  SwdSetLifeTime(s8Dog, 20000);
  SwdEnable(s8Dog);
  //�رտ��Ź�
  StopWatchDog();
  Uart3SendString("#Please Enter The Location Number: \r\n");
  while(TRUE)
  {
    if(Uart3GetRxDataLength()>10)
    {
      Uart3GetRxData(&au8Nbr[0], 11);
      
      for(i=0; i<11; i++)
      {
        if((au8Nbr[i] < '0') && (au8Nbr[i] > '9'))
        {
          Uart3SendString("#You Input Number Is Error, Please Resend The Cmd! \r\n");
          SwdDisable(s8Dog);
          SwdFree(s8Dog);
          FreeWatchDog();          //�ͷſ��Ź�
          return;
        }
      }
      
      au8Nbr[11] = '\r'; 
      au8Nbr[12] = '\n'; 
      Uart3SendBuffer(&au8Nbr[0], 13);
      
      SwdDisable(s8Dog);
      SwdFree(s8Dog);
      FreeWatchDog();          //�ͷſ��Ź�
      break;
    }
    
    
    if(SwdGetLifeTime(s8Dog) == 0)
    {
      SwdDisable(s8Dog);
      SwdFree(s8Dog);
      FreeWatchDog();          //�ͷſ��Ź�
      Uart3SendString("#No Valid Data, Please Resend The Cmd! \r\n");
      return;
    }
    
  }
  
  if( SetLocationNbr(&au8Nbr[0]) )
  {
    Uart3SendString("#CDMA Location Number Set Successfull! \r\n");
  }
  else
  {
    Uart3SendString("#CDMA Location Number Set Failed! \r\n");
  }
}

/*****************************************************************************
 *��������: CmdGetCdmaLocationNbr
 *��������: ���ñ�������

 *����: 
 *����:

���ͣ� 0xAA + 0x55 + 0x34 + 0xCC + 0x33
*****************************************************************************/
void CmdGetCdmaLocationNbr(void)
{

}

/*****************************************************************************
 *��������: CmdSetCdmaSendNbr
 *��������: ���ñ�������

 *����: 
 *����:

���ͣ� 0xAA + 0x55 + 0x34 + 0xCC + 0x33
*****************************************************************************/
void CmdSetCdmaSendNbr(void)
{
  //�������ý��ջ��ĸ���
  //�����������
  //������ɣ������õĺ��뱣�浽EEPROM
}

/*****************************************************************************
 *��������: CmdGetCdmaCSQ
 *��������: ��ȡCDMA���ź�ǿ��

 *����: 
 *����:

���ͣ� 0xAA + 0x55 + 0x37 + 0xCC + 0x33
*****************************************************************************/
void CmdGetCdmaCSQ(void)
{
  UINT8 u8Temp;
  UINT8 u8Num[4];
  
  u8Temp = GetCdmaCSQ();
  u8Num[0] = u8Temp / 10 + '0';
  u8Num[1] = u8Temp % 10 + '0';
  u8Num[2] = '\r';
  u8Num[3] = '\n';
    
  Uart3SendString("#The CDMA Current CSQ Is: ");
  Uart3SendBuffer(u8Num, 4);
}

/*****************************************************************************
 *��������: CmdGetCdmaCSQ
 *��������: ��ȡCDMA���ź�ǿ��

 *����: 
 *����:

���ͣ� 0xAA + 0x55 + 0x50 + 0xCC + 0x33
*****************************************************************************/
void GetBatteryRemainPower(void)
{
  AdcDetect();
}

/***************************************************************************
//�������ƣ�CmdHandlerInit
//���������������б�����
//
//��������
//���أ���
***************************************************************************/
void CmdHandlerInit(void)
{
  //����ָ������
  memset(apCmdHandlerList, 0, sizeof(apCmdHandlerList));
  
  apCmdHandlerList[CMD_ALL_INFO_PRINT]     = PrintfCmdList;              //��ӡ�����б�
  apCmdHandlerList[CMD_HAND_SHAKE]         = UartHandShake;              //��ӡ�����б�
  apCmdHandlerList[CMD_GET_VERSION]        = GetSystemVersionInfo;       //��ӡ�����б�
  apCmdHandlerList[CMD_SET_RTC_TIME]       = SetRtcTime;
  apCmdHandlerList[CMD_GET_RTC_TIME]       = GetRtcTime;
  apCmdHandlerList[CMD_GPS_INFO_CLOSE]     = CmdGpsDebugClose;           //�ر�GPS��debug��ʾ
  apCmdHandlerList[CMD_GPS_INFO_OPEN]      = CmdGpsDebugOpen;            //��GPS��debug��ʾ
  apCmdHandlerList[CMD_BD_INFO_CLOSE]      = CmdBeiDouDebugClose;        //����ͨѶ���ݹر�
  apCmdHandlerList[CMD_BD_INFO_OPEN]       = CmdBeiDouDebugOpen;         //����ͨѶ���ݴ�
  apCmdHandlerList[CMD_CDMA_GET_TIME]      = CmdGetCdmaTime;             //��ȡCDMAʱ��
  apCmdHandlerList[CMD_CDMA_GET_CSQ]       = CmdGetCdmaCSQ;              //��ȡCDMA�ź�ǿ��
  apCmdHandlerList[CMD_CDMA_SET_LOC_NUM]   = CmdSetCdmaLocationNbr;      //���ñ�������
//  apCmdHandlerList[CDMA_RECEIVE_NUM] = SetCdmaReceivePhoneNum;         //���ý��ջ�����
  apCmdHandlerList[CMD_GET_BAT]            = GetBatteryRemainPower;      //���ý��ջ�����
}

/***********************************************************************************************
  �������ƣ�GpsCmdAssemble(UINT8 u8InData)
  �������ܣ��ѽ��յ�������ϳ���Ч�����ݰ�

  ���룺u8InData
  �������
***********************************************************************************************/
void DebugCmdAssemble(UINT8 u8InData)
{
  switch(sDebugCmdCb.eAssembleState)
  {
    case E_UART_CMD_RECEIVING_HEADER1:
    {
      if(u8InData == 0xAA)
      {
        sDebugCmdCb.pu8AssembleBuf[0] = u8InData;
        sDebugCmdCb.u8BufDataLen = 1;
	      sDebugCmdCb.eAssembleState = E_UART_CMD_RECEIVING_HEADER2;	
      }
      break;
    }	
     
    case E_UART_CMD_RECEIVING_HEADER2:
    {
      if(u8InData == 0x55)
      {
        sDebugCmdCb.pu8AssembleBuf[sDebugCmdCb.u8BufDataLen++] = u8InData;
	      sDebugCmdCb.eAssembleState = E_UART_CMD_RECEIVING_CMD;	
      }
      break;
    }	
		
    case E_UART_CMD_RECEIVING_CMD:
    {
      sDebugCmdCb.pu8AssembleBuf[sDebugCmdCb.u8BufDataLen++] = u8InData;
      sDebugCmdCb.eAssembleState = E_UART_CMD_RECEIVING_TAIL1;
    
      break;
    }
     
    case E_UART_CMD_RECEIVING_TAIL1:
    {
      if(u8InData == 0xCC)
      {
        sDebugCmdCb.pu8AssembleBuf[sDebugCmdCb.u8BufDataLen++] = u8InData;
        sDebugCmdCb.eAssembleState = E_UART_CMD_RECEIVING_TAIL2;
      }
    
      break;
    }
     
    case E_UART_CMD_RECEIVING_TAIL2:
    {
      if(u8InData == 0x33)
      {
        sDebugCmdCb.pu8AssembleBuf[sDebugCmdCb.u8BufDataLen++] = u8InData;
        sDebugCmdCb.eAssembleState = E_UART_CMD_RECEIVING_VALID;
      }
    
      break;
    }
		
    case E_UART_CMD_RECEIVING_VALID:
    { 
      break;
    }
		
    default:
    {
      sDebugCmdCb.eAssembleState = E_UART_CMD_RECEIVING_HEADER1;
      sDebugCmdCb.u8BufDataLen = 0;
    }
  }	
}

/**********************************************************************************************
 �������ƣ�DebugCmdInit
 �������ܣ�Debug cmd parse init

 ������ ��
 ���أ� ��
**********************************************************************************************/
void DebugCmdInit(void)
{
  //��ʼ�������б�
  CmdHandlerInit();
  
  //  sGpsCmdCb.bGpsParse = FALSE;
  sDebugCmdCb.u8BufDataLen = 0;
  sDebugCmdCb.eAssembleState = E_UART_CMD_RECEIVING_HEADER1;
  sDebugCmdCb.pu8AssembleBuf = OSMemGet(pSmallMem, &err);

  //Get One soft timer
  sDebugCmdCb.sDebugDog = SwdGet();   
  SwdSetLifeTime(sDebugCmdCb.sDebugDog, 200);
}

/***************************************************************************
//�������ƣ�UartCmdRun
//�������������ڽ����������
//
//��������
//���أ���
***************************************************************************/
void DebugCmdRun(void)
{
  UINT8 u8Cmd;

  while((Uart3GetRxDataLength() > 0) && (sDebugCmdCb.eAssembleState != E_UART_CMD_RECEIVING_VALID))
  {
    DebugCmdAssemble(Uart3GetByte());

    if(SwdEnable(sDebugCmdCb.sDebugDog) == TRUE)
    {
      SwdDisable(sDebugCmdCb.sDebugDog);
    }
  }
  
  //���ݰ���Ч���Կ��õ�ָ����н���
  if(sDebugCmdCb.eAssembleState == E_UART_CMD_RECEIVING_VALID)
  {
    u8Cmd = sDebugCmdCb.pu8AssembleBuf[2];
    
    if(apCmdHandlerList[u8Cmd] != NULL)
    {
      apCmdHandlerList[u8Cmd]();
    }
    
    sDebugCmdCb.u8BufDataLen = 0;
    sDebugCmdCb.eAssembleState = E_UART_CMD_RECEIVING_HEADER1;
  }
  else    
  {
    if(SwdEnable(sDebugCmdCb.sDebugDog) == TRUE)
    {
      if(SwdGetLifeTime(sDebugCmdCb.sDebugDog) == 0)
      {
        sDebugCmdCb.eAssembleState = E_UART_CMD_RECEIVING_HEADER1;
        sDebugCmdCb.u8BufDataLen = 0;
        
        SwdReset(sDebugCmdCb.sDebugDog);
        SwdDisable(sDebugCmdCb.sDebugDog);
      }
    }
    else
    {
      if(sDebugCmdCb.u8BufDataLen != 0)
      {
        SwdReset(sDebugCmdCb.sDebugDog);
        SwdEnable(sDebugCmdCb.sDebugDog);
      }
    }
  }
}