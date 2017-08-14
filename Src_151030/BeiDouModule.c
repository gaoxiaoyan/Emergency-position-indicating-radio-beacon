
/******************************************************************************************************
 *
 * Copyright (C) 2015, Sunic-ocean
 * All Rights Reserved
 *
 * �� �� ���� BeiDouModule.c
 * ��    ��������ģ�������������ʼ����IC��⣬��Ϣ����,��������ȡ�
 *
 * �������ڣ� 2015��5��5��16:10:43
 * ��    �ߣ� Bob
 * ��ǰ�汾�� V1.00
 *******************************************************************************************************/
#define  BD_GLOBAL
#include "msp430x54x.h"
#include "BatteryDetect.h"
#include "global.h"
#include "DataType.h"
#include "SystemCtl.h"
#include "Timer.h"
#include "Uart2.h"
#include "Uart3.h"
#include "SoftTimer.h"
#include "OS_MEM.h"
#include "Rtc.h"
#include "CdmaModule.h"
#include "Communication.h"
#include "DebugCmd.h"
#include "Record.h"
#include "BeiDouModule.h"
#include "string.h"
#include "ctype.h"

//���ݰ�����Э��
typedef enum
{
  //����֡ͷ
  E_UART_CMD_RECEIVING_HEADER,

  //����
  E_UART_CMD_RECEIVING_CMD,

  //���������ܳ�
  E_UART_CMD_RECEIVING_LENGHT,

  //������������
  E_UART_CMD_RECEIVING_BODY,

  //������Ч
  E_UART_CMD_RECEIVING_VALID

}teDebugCmdAssembleState;

/* ��������ṹ���� */
typedef struct
{
  UINT8 *pu8AssembleBuf;  //���������ָ��

  UINT8 u8CmdLen;         //�����

  UINT16 u16BufDataLen;     //�Ѿ���������ݳ���

	UINT8 u8Len;						//�������ݳ���Ϊ2byte����¼���ڽ��յĸߵ�λ

  UINT16 u16RevDataLen ;   //ÿ�����ݵĳ���

  teDebugCmdAssembleState eAssembleState;  //���״̬

  SINT8 sBdDog;

}_tsBdCmdCb;

_tsBdCmdCb sBdCmdCb;


//IC�������
UINT8 au8Icjc[12] = {0x24, 0x49, 0x43, 0x4a, 0x43, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x2b};
UINT8 au8LocationAddr[3] = {0};

/****************************************************************************
�������ƣ�BdSendCmdDWSQ()
��    �ܣ�����ģ�鷢�Ͷ�λ����

��    ������
����ֵ  ����
*****************************************************************************/
void BdSendCmdDWSQ(void)
{

}

/****************************************************************************
�������ƣ�BdSendCmdSJSC()
��    �ܣ�����ģ�鷢��ʱ���������

��    ������
����ֵ  ����
*****************************************************************************/
void BdSendCmdSJSC(void)
{

}

/*****************************************************************************
 �� �� ��  : Delay
 ��������  : ��12MHz���� 1T�� ��ʱʱ��Լ����usTime ms
 �������  : USHORT16 usTime
 �������  : NONE
 �� �� ֵ  :
*****************************************************************************/
/*
static void Delay(UINT16 usTime)
{
    UINT16 j;
    UINT32 i;

    for (i = usTime; i > 0; i--)
    {
        for(j = 110; j > 0; j--);
    }
}*/

/****************************************************************************
�������ƣ�BdSendCmdTXSQ()
��    �ܣ�����ģ��ͨѶ����

��    ������
����ֵ  ����
*****************************************************************************/
void BdSendCmdTXSQ(UINT8 *pSendAddr, UINT8 *pMsg, UINT8 u8MsgLen)
{
    UINT8 *pInfo;   //���ͱ����ڴ�ָ��
	UINT8 i, j, u8Check;

    pInfo = OSMemGet(pSmallMem, &err);

	//ͨѶ�����������
	i = 0;
	memcpy(&pInfo[i], "$TXSQ", 5);					//���뷢������
	i += 5;

	pInfo[i++] = (UINT8)(((UINT16)u8MsgLen + 18) >> 8);								//���ĳ��ȼ���18���̶�����ΪͨѶ���ݳ���
	pInfo[i++] = (UINT8)( (UINT16)u8MsgLen + 18);

	memcpy(&pInfo[i], au8LocationAddr, 3);														//��ȡ��������
	i += 3;

	pInfo[i++]  = 0x46;								                                    //ascii Ϊ0x46�����ֱ���Ϊ0x44

	memcpy(&pInfo[i], pSendAddr, 3);																		//���ͷ���ַ
	i += 3;

	pInfo[i++] = (UINT8)((UINT16)u8MsgLen * 8 >> 8);								  //������Ϣ���ȣ�bit��
	pInfo[i++] = (UINT8)((UINT16)u8MsgLen * 8);

	pInfo[i++] = 0;																										//Ӧ��λ

	memcpy(&pInfo[i], pMsg, u8MsgLen);
	i += u8MsgLen;

    u8Check = 0;
	for(j=0; j<i; j++)
	{
		u8Check ^= pInfo[j];
	}

    pInfo[i] = u8Check;

    Uart3EmptyRxBuf();
//  sBdCmdCb.u16BufDataLen = 0;

	Uart3SendBuffer(&pInfo[0], i+1);

    OSMemPut(pSmallMem, pInfo);

  //��ʱ2S��ȷ��������Ϣ
 //   g_u16TimerDelay=0;
//    while(g_u16TimerDelay<200);
}


/****************************************************************************
�������ƣ�BdSendCmdICJC()
��    �ܣ�����IC��⣬����

��    ������
����ֵ  ����
*****************************************************************************/
void BdSendCmdICJC(void)
{
  SINT8 s8Dog;
  UINT8 *pRxBuf;
  UINT8 i;

//���һ���Ƿ����IC��
#if BDIC_DET_EN >0
  if(!BdIcDet())
  {
    Uart32SendString("#BeiDou IC Card Not Found!\r\n ");
  }
#endif

  pRxBuf = OSMemGet(pSmallMem, &err);

  g_u8TimerDelay=0;
  while(g_u8TimerDelay<100);

  Uart3EmptyRxBuf();
  Uart3SendBuffer(au8Icjc, 12);

  s8Dog = SwdGet();
  SwdSetLifeTime(s8Dog, 1000);
  SwdEnable(s8Dog);

  while(TRUE)
  {
    if(Uart3GetRxDataLength()>21)
    {
      Uart3GetRxData(pRxBuf, 22);
      memcpy(&au8LocationAddr[0], &pRxBuf[7],3);

      break;
    }

    //1�볬ʱû���յ�IC��Ϣ���ٴη���һ�Σ�����5�κ���û������ֱ�ӷ���
    if(SwdGetLifeTime(s8Dog) == 0)
    {
      i++;
      if(i>5)
      {
        break;
      }
      else
      {
        SwdReset(s8Dog);
        Uart3EmptyRxBuf();
        Uart3SendBuffer(au8Icjc, 12);
      }
    }
  }

  SwdDisable(s8Dog);
  SwdFree(s8Dog);
  OSMemPut(pSmallMem, pRxBuf);
}

/***********************************************************************************************
  �������ƣ�GetBeiDouIcNum()
  �������ܣ���ȡ�����ı�������

  ���룺��
  �������
***********************************************************************************************/
void GetBeiDouIcNum(UINT8 *ptr)
{
  memcpy(ptr, &au8LocationAddr[0], 3);
}

/***********************************************************************************************
  �������ƣ�GetBeiDouIcNum()
  �������ܣ���ȡ�������ջ�����

  ���룺��
  �������
***********************************************************************************************/
void GetBeiDouSendNum(UINT8 *ptr)
{
  ptr[0] = sRecordCb.au8BdRevNbr[0];
  ptr[1] = sRecordCb.au8BdRevNbr[1];
  ptr[2] = sRecordCb.au8BdRevNbr[2];
}

/***********************************************************************************************
  �������ƣ�GetBeiDouBckSendNum()
  �������ܣ���ȡ�������ý��ջ�����

  ���룺��
  �������
***********************************************************************************************/
void GetBeiDouBckSendNum(UINT8 *ptr)
{
  ptr[0] = sRecordCb.au8BdBckNbr[0];
  ptr[1] = sRecordCb.au8BdBckNbr[1];
  ptr[2] = sRecordCb.au8BdBckNbr[2];
}

/***********************************************************************************************
  �������ƣ�CmdParseTXXX()
  �������ܣ�ͨѶ��Ϣ�������

  ���룺��
  �������
***********************************************************************************************/
#define OFFSET(a, b)   ((UINT8)(&((a *)0)->b))
void CmdParseTXXX(void)
{
  UINT8 *ptr;
  UINT8 u8Temp, u8Field;
  UINT16 u16Len;

  ptr = sBdCmdCb.pu8AssembleBuf;
  u8Field = OFFSET(_tsCmdTxxxCb, au8MsgLen);

  u8Temp = ptr[u8Field];
  u16Len = ((UINT16)u8Temp) << 8;
  u8Temp = ptr[u8Field+1];
  u16Len += u8Temp;
  u8Temp  = (UINT8)(u16Len / 8);

  Uart2SendString("#BeiDou Receiv One Massage: ");
  Uart2SendBuffer(&ptr[u8Field+2], u8Temp);
  Uart2SendString(".\r\n");

#ifdef BEIDOU_TEST
  memset(&au8BdRxInfo[0], 0, sizeof(au8BdRxInfo));
  u8Field = OFFSET(_tsCmdTxxxCb, pMsg);
  memcpy(&au8BdRxInfo[0], &ptr[u8Field], u8Temp);

  if(memcmp(&au8BdRxInfo[0], &au8BdSendInfo[0], sizeof(au8BdRxInfo)) == 0)
  {
     u16BdRxNum++;
  }
#endif
}

/***********************************************************************************************
  �������ƣ�CmdParseDWXX()
  �������ܣ���λ��Ϣ�������

  ���룺��
  �������
***********************************************************************************************/
void CmdParseDWXX(void)
{
  memcpy(&sCmdDwxxCb, &sBdCmdCb.pu8AssembleBuf[0], sizeof(sCmdDwxxCb));
  Uart2SendBuffer(&sCmdDwxxCb.au8BdCmd[0], sizeof(sCmdDwxxCb));
}

/***********************************************************************************************
  �������ƣ�CmdParseFKXX()
  �������ܣ���λ��Ϣ�������

  ���룺��
  �������
***********************************************************************************************/
void CmdParseFKXX(void )
{
   memcpy(&sCmdFkxxCb, &sBdCmdCb.pu8AssembleBuf[0], sizeof(sCmdFkxxCb));

   switch(sCmdFkxxCb.u8BackInfo)
   {
    case 0:
     {
       u16BdSendSucceed++;
       Uart2SendString("#BeiDou Send Massage Successful! \r\n");
       break;
     }

     case 1:
     {
       Uart2SendString("#BeiDou Send Massage Failed! \r\n");
       break;
     }

     case 2:
     {
       Uart2SendString("#BeiDou Signal Is Unlocked! \r\n");
       break;
     }

     case 3:
     {
       Uart2SendString("#BeiDou Battery Is Low! \r\n");
       break;
     }

     case 4:
     {
       Uart2SendString("#BeiDou Send Massage Time Don`t Come! \r\n");
       break;
     }

     case 5:
     {
       Uart2SendString("#BeiDou Encryption And Decryption Is Error! \r\n");
       break;
     }

     case 6:
     {
       Uart2SendString("#BeiDou Crc Check Is Error! \r\n");
       break;
     }

     default: break;
   }
}

/***********************************************************************************************
  �������ƣ�BdCmdAssemble(UINT8 u8InData)
  �������ܣ��ѽ��յ�������ϳ���Ч�����ݰ�

  ���룺u8InData
  �������
***********************************************************************************************/
void BdCmdParse(UINT8 *pCmd)
{
		if(memcmp(pCmd, "$TXXX", 5) == 0)
		{
			CmdParseTXXX();
		}
		else if(memcmp(pCmd, "$DWXX", 5) == 0)
		{
//			CmdParseDWXX();
		}
        else if(memcmp(pCmd, "$FKXX", 5) == 0)
		{
			CmdParseFKXX();
		}
		else if(memcmp(pCmd, "$ICXX", 5) == 0)
		{
//			CmdParseICXX();
		}
		else if(memcmp(pCmd, "$SJXX", 5) == 0)
		{
//			CmdParseSJXX();
		}

}


/***********************************************************************************************
  �������ƣ�BdCmdAssemble(UINT8 u8InData)
  �������ܣ��ѽ��յ�������ϳ���Ч�����ݰ�

  ���룺u8InData
  �������
***********************************************************************************************/
void BdCmdAssemble(UINT8 u8InData)
{
  switch(sBdCmdCb.eAssembleState)
  {
    case E_UART_CMD_RECEIVING_HEADER:
    {
      if(u8InData == '$')
      {
        sBdCmdCb.pu8AssembleBuf[0] = u8InData;
		sBdCmdCb.u16BufDataLen     = 1;
	    sBdCmdCb.eAssembleState    = E_UART_CMD_RECEIVING_CMD;
      }
      break;
    }

    case E_UART_CMD_RECEIVING_CMD:
    {
      if(sBdCmdCb.u8CmdLen < 4)
      {
        sBdCmdCb.pu8AssembleBuf[sBdCmdCb.u16BufDataLen++] = u8InData;
		sBdCmdCb.u8CmdLen++;

		if(sBdCmdCb.u8CmdLen == 4)
		{
			sBdCmdCb.u8CmdLen       = 0;
			sBdCmdCb.u8Len          = 0;
			sBdCmdCb.eAssembleState = E_UART_CMD_RECEIVING_LENGHT;
		}
      }
      break;
    }

    case E_UART_CMD_RECEIVING_LENGHT:
    {
      if(sBdCmdCb.u8Len < 2)
      {
      	sBdCmdCb.pu8AssembleBuf[sBdCmdCb.u16BufDataLen++] = u8InData;
      	sBdCmdCb.u8Len++;
		if(sBdCmdCb.u8Len == 1)
	    {
            sBdCmdCb.u16RevDataLen = (UINT16)u8InData << 8;
        }
	    else
		{
            if(sBdCmdCb.u8Len == 2)
            {
		        sBdCmdCb.u16RevDataLen += u8InData;
			    sBdCmdCb.eAssembleState = E_UART_CMD_RECEIVING_BODY;
		    }
	    }
      }
      break;
    }

    case E_UART_CMD_RECEIVING_BODY:
    {
      if(sBdCmdCb.u16BufDataLen < sBdCmdCb.u16RevDataLen)
      {
	        sBdCmdCb.pu8AssembleBuf[sBdCmdCb.u16BufDataLen++] = u8InData;

			if(sBdCmdCb.u16BufDataLen == sBdCmdCb.u16RevDataLen)
		    {
				sBdCmdCb.eAssembleState = E_UART_CMD_RECEIVING_VALID;
			}
	  }

      break;
    }

    case E_UART_CMD_RECEIVING_VALID:
    {
      break;
    }

    default:
    {
      sBdCmdCb.eAssembleState = E_UART_CMD_RECEIVING_HEADER;
      sBdCmdCb.u16BufDataLen = 0;
    }
  }
}

/**********************************************************************************************
 �������ƣ�DebugCmdInit
 �������ܣ�Debug cmd parse init

 ������ ��
 ���أ� ��
**********************************************************************************************/
void BdModuleInit(void)
{
   //IC���
  BdSendCmdICJC();

  sBdCmdCb.u8CmdLen       = 0;
  sBdCmdCb.u16BufDataLen  = 0;
  sBdCmdCb.u16RevDataLen  = 0;
  sBdCmdCb.eAssembleState = E_UART_CMD_RECEIVING_HEADER;
  sBdCmdCb.pu8AssembleBuf = OSMemGet(pMediumMem, &err);

  //Get One soft timer
  sBdCmdCb.sBdDog = SwdGet();
  SwdSetName(sBdCmdCb.sBdDog, "BeiDouTimer");
  SwdSetLifeTime(sBdCmdCb.sBdDog, 500);
}

#ifdef BEIDOU_TEST
void GetBufBdInfo(void)
{
  UINT8 *ptr;
  UINT8 u8Len;
  UINT8 i,j;

  ptr = OSMemGet(pSmallMem, &err);
  u8Len = Uart3GetRxDataLength();

  if(u8Len > 30)
  {
    Uart3GetRxData(ptr, u8Len);

    i = 0;
    while((ptr[i] != '#') && (i<u8Len))
    {
      i++;
    }

    j = i+1;
    while((ptr[j] != '#') && (j<u8Len))
    {
      j++;
    }

    Uart2SendString("#BeiDou Receiv One Massage: ");
    Uart2SendBuffer(&ptr[i], (j-i+1));
    Uart2SendString(".\r\n");

    memset(&au8BdRxInfo[0], 0, 34);
    memcpy(&au8BdRxInfo[0], &ptr[i], (j-i+1));

    if(memcmp(&au8BdRxInfo[0], &au8BdSendInfo[0], 34) == 0)
       u16BdRxNum++;
  }

  OSMemPut(pSmallMem, ptr);
}
#endif

/***************************************************************************
//�������ƣ�BdCmdRun
//���������������������������������Ϣ��ͨ�ŷ��ء�
//
//��������
//���أ���
***************************************************************************/
void BdCmdRun(void)
{
  UINT8 au8Cmd[5];

  //����ģ�鷢��ʹ��ʱ�������������
  if(g_bBdSend == TRUE)
  {
    while((Uart3GetRxDataLength() > 0) && (sBdCmdCb.eAssembleState != E_UART_CMD_RECEIVING_VALID))
    {
      BdCmdAssemble(Uart3GetByte());

      if(SwdIsEnable(sBdCmdCb.sBdDog) == TRUE)
      {
        SwdDisable(sBdCmdCb.sBdDog);
      }
    }

    //���ݰ���Ч���Կ��õ�ָ����н���
    if(sBdCmdCb.eAssembleState == E_UART_CMD_RECEIVING_VALID)
    {
      memcpy(&au8Cmd[0], &sBdCmdCb.pu8AssembleBuf[0], 5);

      BdCmdParse(&au8Cmd[0]);

      GetBufBdInfo();

      sBdCmdCb.u16BufDataLen = 0;
      sBdCmdCb.u16RevDataLen = 0;
      sBdCmdCb.eAssembleState = E_UART_CMD_RECEIVING_HEADER;
    }
    else
    {
      if(SwdIsEnable(sBdCmdCb.sBdDog) == TRUE)
      {
        if(SwdGetLifeTime(sBdCmdCb.sBdDog) == 0)
        {
          sBdCmdCb.eAssembleState = E_UART_CMD_RECEIVING_HEADER;
          sBdCmdCb.u16BufDataLen = 0;
          sBdCmdCb.u16RevDataLen = 0;

          SwdReset(sBdCmdCb.sBdDog);
          SwdDisable(sBdCmdCb.sBdDog);
        }
      }
      else
      {
        if(sBdCmdCb.u16BufDataLen != 0)
        {
          SwdReset(sBdCmdCb.sBdDog);
          SwdEnable(sBdCmdCb.sBdDog);
        }
      }
    }
  }
}





