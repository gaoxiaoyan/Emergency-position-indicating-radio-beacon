/******************************************************************************
*											
* Copyright (C) 2014, Sunic-ocean
* All Rights Reserved										
*
* �� �� ���� UartCmd.h
* ��       ����

* �������ڣ� 2014��12��5��11:31:59
* ��    �ߣ� Bob
* ��ǰ�汾�� V1.00

------------------------------------------------------
*******************************************************************************/
#include "msp430x54x.h"

#include "global.h"
#include "DataType.h"
#include "UartCmd.h"
#include "SystemCtl.h"

//�����������ݽṹ����
typedef struct
{
  UINT8 u8Head_1;
  UINT8 u8Head_2;
  UINT8 u8Cmd;
  UINT8 u8Tail_1;
  UINT8 u8Tail_2;
}_tsUartCmdCb sUartCmd;

//����ͨѶ�����б�
typedef void CmdHandler(void);
CmdHandler *apCmdHandlerList[32];

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
  Uart3SendString("#  0�������ʽ��AA 55 cmd CC 33.  \r\n");
  Uart3SendString("#  1��cmd<10> -- GPS������Ϣ�رգ�\r\n");
  Uart3SendString("#  2��cmd<11> -- GPS������Ϣ�򿪣�\r\n");
  Uart3SendString("#  3��cmd<20> -- BeiDou������Ϣ�رգ�\r\n");
  Uart3SendString("#  4��cmd<21> -- BeiDou������Ϣ�򿪣�\r\n");
  Uart3SendString("#  5��cmd<22> -- BeiDou���ͱ��Ĳ��ԣ�\r\n");
  Uart3SendString("#  6��cmd<30> -- CDMA������Ϣ�رգ�\r\n");
  Uart3SendString("#  7��cmd<31> -- CDMA������Ϣ�򿪣�\r\n");
  Uart3SendString("#  8��cmd<32> -- ��ȡCDMAʱ�䣻\r\n");
  Uart3SendString("#  9��cmd<33> -- ��CDMAʱ��У׼RTC��\r\n");
  Uart3SendString("# 10��cmd<34> -- ��ȡ�������룻\r\n");
  Uart3SendString("# 11��cmd<35> -- ���ý��պ��룻\r\n");
  Uart3SendString("# 12��cmd<40> -- ����ͨѶ���ʱ�䣻\r\n");
  Uart3SendString("# 13��cmd<50> -- ��ȡ��ǰ���״̬��\r\n");
  Uart3SendString("###############################################################\r\n");
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
  
  apCmdHandlerList[PRINTF_ALL_CMD]  = PrintfCmdList;       //��ӡ�����б�
//  apCmdHandlerList[GPS_DEBUG_CLOSE] = GpsDebugClose;    //�ر�GPS��debug��ʾ
//  apCmdHandlerList[GPS_DEBUG_OPEN]  = GpsDebugOpen;     //��GPS��debug��ʾ
//  apCmdHandlerList[BD_DEBUG_CLOSE]  = BeiDouDebugClose; //����ͨѶ���ݹر�
//  apCmdHandlerList[BD_DEBUG_OPEN]   = BeiDouDebugOpen;  //����ͨѶ���ݴ�
  apCmdHandlerList[CDMA_TIME_GET]    = GetCdmaTime;       //��ȡCDMAʱ��
  apCmdHandlerList[CDMA_TIME_TO_RTC] = SetCdmaTimeToRtc;  //��CDMAʱ�����õ�rtc��
  apCmdHandlerList[CDMA_LOCAL_NUM]   = SetCdmaLocalPhoneNum;  //���ñ�������
  apCmdHandlerList[CDMA_RECEIVE_NUM] = SetCdmaReceivePhoneNum;  //���ý��ջ�����
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
      sDebugCmdCb.eAssembleState = E_UART_CMD_RECEIVING_HEADER;
      sDebugCmdCb.u8BufDataLen = 0;
    }
  }	
}

/**********************************************************************************************
 �������ƣ�GpsCmdInit
 �������ܣ�GPS cmd parse init

 ������ ��
 ���أ� ��
**********************************************************************************************/
void GpsCmdInit(void)
{
  //��ʼ�������б�
  CmdHandlerInit();
  
  //  sGpsCmdCb.bGpsParse = FALSE;
  sDebugCmdCb.u16BufDataLen = 0;
  sDebugCmdCb.eAssembleState = E_UART_CMD_RECEIVING_HEADER1;
  sDebugCmdCb.pu8AssembleBuf = OSMemGet(pSmallMem, &err);

  //Get One soft timer
  sDebugCmdCb.sGpsDog = SwdGet();   
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

    if(SwdEnable(sDebugCmdCb.sGpsDog) == TRUE)
    {
      SwdDisable(sDebugCmdCb.sGpsDog);
    }
  }
  
  //���ݰ���Ч���Կ��õ�ָ����н���
  if(sGpsCmdCb.eAssembleState == E_UART_CMD_RECEIVING_VALID)
  {
  
  }
}