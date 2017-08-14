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

void UartCmdInit(void)
{
  CmdHandlerInit();
  
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

  while((Uart2GetRxDataLength() > 0) && (sUartCmdCb[USB_TYPE].eAssembleState != E_UART_CMD_RECEIVING_VALID))
  {
    Uart2CmdAssemble(Uart2GetByte());

    if(asTimer[TIMER_UART2].bEnable == TRUE)
    {
      asTimer[TIMER_UART2].bEnable = FALSE;
    }
  }
  
  //���ݰ���Ч���Կ��õ�ָ����н���
    if(sUartCmdCb[USB_TYPE].eAssembleState == E_UART_CMD_RECEIVING_VALID)
    {
      u8Cmd = ConfigPackType(au8UsbBodyBuf, sUartCmdCb[USB_TYPE].u8BufDataLength);
      //����������
      CreatCmdIndex(au8UsbBodyBuf);   
      //���������ָ����buf
      MEM_SET(&au8UsbBodyBuf[0], 0, sizeof(au8UsbBodyBuf));
      sUartCmdCb[USB_TYPE].eAssembleState = E_UART_CMD_RECEIVING_HEADER;
      sUartCmdCb[USB_TYPE].u8BufDataLength = 0;
    }
    else
    {
      //�涨ʱ���������ݣ�ָ�λ��������ȡ��Ч����

    }
}